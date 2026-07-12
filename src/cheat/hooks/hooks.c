#include "hooks.h"
#include "../utils/logger.h"
#include <Zydis/Zydis.h>
#include <dlfcn.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

hook_t hooks[MAX_HOOKS] = {0};

static bool hook_find_free_slot(size_t *slot) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (hooks[i].addr == NULL) {
      *slot = i;
      return true;
    }
  }
  return false;
}

static bool hook_already_installed(void *addr) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (hooks[i].addr == addr)
      return true;
  }
  return false;
}

static bool hook_protect(void *addr, size_t len) {
  size_t page_size = sysconf(_SC_PAGE_SIZE);
  void *addr_aligned = (void *)((uintptr_t)addr & ~(page_size - 1));
  size_t aligned_len = (((uintptr_t)addr + len - (uintptr_t)addr_aligned) + page_size - 1) & ~(page_size - 1);

  if (mprotect(addr_aligned, aligned_len, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
    LOGGER_LOG(FATAL, "mprotect failed with %s", strerror(errno));
    return false;
  }
  return true;
}

static bool is_relative_instruction(ZydisDecodedInstruction *instruction, ZydisDecodedOperand *operands,
                                    size_t *idx) {
  for (size_t i = 0; i < instruction->operand_count; i++) {
    ZydisDecodedOperand operand = operands[i];
    switch (operand.type) {
    case ZYDIS_OPERAND_TYPE_IMMEDIATE:
      if (operand.imm.is_relative) {
        *idx = i;
        return true;
      }
      break;
    case ZYDIS_OPERAND_TYPE_MEMORY:
      if (operand.mem.base == ZYDIS_REGISTER_RIP || operand.mem.base == ZYDIS_REGISTER_EIP) {
        *idx = i;
        return true;
      }
      break;
    default:
      break;
    }
  }
  return false;
}

static size_t calculate_offset(ZydisDecodedInstruction *instruction, ZydisDecodedOperand *operand) {
  switch (operand->type) {
  case ZYDIS_OPERAND_TYPE_IMMEDIATE:
    if (operand->imm.is_relative)
      return operand->imm.value.u + instruction->length;
    break;
  case ZYDIS_OPERAND_TYPE_MEMORY:
    if (operand->mem.base == ZYDIS_REGISTER_RIP || operand->mem.base == ZYDIS_REGISTER_EIP)
      // TODO: Offset = Base + (Index * Scale) + Displacement
      // For now we don't care about index
      // MOD=00 R/M=101 only right now
      return instruction->length + operand->mem.disp.value;
    break;
  default:
    break;
  }
  return 0;
}

static ZydisMnemonic inverse_mnemonic(ZydisMnemonic mnemonic) {
  switch (mnemonic) {
  case ZYDIS_MNEMONIC_JO:
    return ZYDIS_MNEMONIC_JNO;
  case ZYDIS_MNEMONIC_JNO:
    return ZYDIS_MNEMONIC_JO;
  case ZYDIS_MNEMONIC_JB:
    return ZYDIS_MNEMONIC_JNB;
  case ZYDIS_MNEMONIC_JNB:
    return ZYDIS_MNEMONIC_JB;
  case ZYDIS_MNEMONIC_JZ:
    return ZYDIS_MNEMONIC_JNZ;
  case ZYDIS_MNEMONIC_JNZ:
    return ZYDIS_MNEMONIC_JZ;
  case ZYDIS_MNEMONIC_JBE:
    return ZYDIS_MNEMONIC_JNBE;
  case ZYDIS_MNEMONIC_JNBE:
    return ZYDIS_MNEMONIC_JBE;
  case ZYDIS_MNEMONIC_JS:
    return ZYDIS_MNEMONIC_JNS;
  case ZYDIS_MNEMONIC_JNS:
    return ZYDIS_MNEMONIC_JS;
  case ZYDIS_MNEMONIC_JP:
    return ZYDIS_MNEMONIC_JNP;
  case ZYDIS_MNEMONIC_JNP:
    return ZYDIS_MNEMONIC_JP;
  case ZYDIS_MNEMONIC_JL:
    return ZYDIS_MNEMONIC_JNL;
  case ZYDIS_MNEMONIC_JNL:
    return ZYDIS_MNEMONIC_JL;
  case ZYDIS_MNEMONIC_JLE:
    return ZYDIS_MNEMONIC_JNLE;
  case ZYDIS_MNEMONIC_JNLE:
    return ZYDIS_MNEMONIC_JLE;
  default:
    return ZYDIS_MNEMONIC_INVALID;
  }
  return ZYDIS_MNEMONIC_INVALID;
}

bool hook_install(void *addr, void *detour, void **trampoline) {
  LOGGER_LOG(LOG, "setting hook for %p", addr);
  bool status = false;
  if (hook_already_installed(addr))
    goto end;

  size_t hook_idx = 0;
  if (!hook_find_free_slot(&hook_idx))
    goto end;

  uint8_t prologue_shellcode[] = {
      0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, addr
      0xFF, 0XE0                                                  // jmp rax
  };

  uint8_t trampoline_return_shellcode[] = {
      0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, addr
      0xFF, 0XE0                                                  // jmp rax
  };

  uint8_t relative_mem_shellcode_prologue[] = {
      0x50,                                                       // push rax
      0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, final_addr
  };

  uint8_t relative_mem_shellcode_epilogue[] = {
      0x58 // pop rax
  };

  uint8_t cond_jmp_shellcode[] = {
      0x50,                                                       // push rax
      0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, final_addr
      0x48, 0x87, 0x04, 0x24,                                     // xchg [rsp], rax
      0xC3                                                        // ret
  };

  // set addr
  memcpy(prologue_shellcode + 2, &detour, 8);

  if (!hook_protect(addr, sizeof(prologue_shellcode)))
    goto end;

  ZyanU8 *prologue = (uint8_t *)addr;
  ZyanU64 runtime_address = (ZyanU64)addr;

  ZyanUSize offset = 0;
  ZydisDecodedInstruction instruction;
  ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
  ZydisDecoder decoder;
  ZydisEncoderRequest req;
  ZyanU8 encoded_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH];
  ZyanUSize encoded_length = ZYDIS_MAX_INSTRUCTION_LENGTH;
  ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

#define MAX_INSTRUCTIONS (67)
#define TRAMPOLINE_SIZE (ZYDIS_MAX_INSTRUCTION_LENGTH * MAX_INSTRUCTIONS)
  trampoline_t generated_trampoline = {
      .base = calloc(TRAMPOLINE_SIZE, 1), .size = TRAMPOLINE_SIZE, .edge = 0};

  // loop over the instructions
  while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, &prologue[offset], ZYDIS_MAX_INSTRUCTION_LENGTH,
                                             &instruction, operands)) &&
         offset < sizeof(prologue_shellcode)) {
    size_t opx = 0;
    if (is_relative_instruction(&instruction, operands, &opx)) {

      // FOR MEMORY
      // lea rax, [rip+0x1234] - transforms to:
      //   push rax
      //   mov rax, final_addr
      //   lea rax, [rax]
      //   pop rax
      //
      // FOR COND JUMPS
      // je 0x12 - transforms to:
      //   jne skip
      //   push rax
      //   mov rax, final_addr
      //   xchg [rsp], rax
      //   ret
      //   skip:
      //   ...

      // we need to relocate it
      LOGGER_LOG(LOG, "relocation is needded for current instruction");
      size_t rel_offset = calculate_offset(&instruction, &operands[opx]);
      uintptr_t final_addr = runtime_address + rel_offset;

      memset(&req, 0, sizeof(req));
      memset(encoded_instruction, 0, sizeof(encoded_instruction));
      if (ZYAN_FAILED(ZydisEncoderDecodedInstructionToEncoderRequest(
              &instruction, operands, instruction.operand_count_visible, &req))) {
        LOGGER_LOG(FATAL, "Failed to encode decoded instruction");
        goto end;
      }

      if (operands[opx].type == ZYDIS_OPERAND_TYPE_MEMORY) {

        // set addr
        memcpy(relative_mem_shellcode_prologue + 3, &final_addr, sizeof(final_addr));

        memcpy(generated_trampoline.base + generated_trampoline.edge, relative_mem_shellcode_prologue,
               sizeof(relative_mem_shellcode_prologue));
        generated_trampoline.edge += sizeof(relative_mem_shellcode_prologue);

        encoded_length = ZYDIS_MAX_INSTRUCTION_LENGTH;
        req.operands[opx].mem.displacement = 0;
        req.operands[opx].mem.scale = 0;
        req.operands[opx].mem.index = 0;
        req.operands[opx].mem.base = ZYDIS_REGISTER_RAX;
        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instruction, &encoded_length))) {
          LOGGER_LOG(FATAL, "failed to encode relative instruction");
          goto end;
        }
        memcpy(generated_trampoline.base + generated_trampoline.edge, encoded_instruction, encoded_length);
        generated_trampoline.edge += encoded_length;

        memcpy(generated_trampoline.base + generated_trampoline.edge, relative_mem_shellcode_epilogue,
               sizeof(relative_mem_shellcode_epilogue));
        generated_trampoline.edge += sizeof(relative_mem_shellcode_epilogue);

      } else if (operands[opx].type == ZYDIS_OPERAND_TYPE_IMMEDIATE && operands[opx].imm.is_relative) {
        ZyanI64 skip_size = sizeof(cond_jmp_shellcode);
        encoded_length = ZYDIS_MAX_INSTRUCTION_LENGTH;
        req.mnemonic = inverse_mnemonic(instruction.mnemonic);
        req.operands[opx].imm.s = skip_size;
        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instruction, &encoded_length))) {
          LOGGER_LOG(FATAL, "failed to encode relative jump instruction");
          goto end;
        }
        memcpy(generated_trampoline.base + generated_trampoline.edge, encoded_instruction, encoded_length);
        generated_trampoline.edge += encoded_length;

        // set addr
        memcpy(cond_jmp_shellcode + 3, &final_addr, sizeof(final_addr));

        memcpy(generated_trampoline.base + generated_trampoline.edge, cond_jmp_shellcode,
               sizeof(cond_jmp_shellcode));
        generated_trampoline.edge += sizeof(cond_jmp_shellcode);
      } else {
        LOGGER_LOG(FATAL, "unknown type");
        goto end;
      }

    } else {
      // just blindly copy the instruction without transformation
      memcpy(generated_trampoline.base + generated_trampoline.edge, prologue + offset, instruction.length);
      generated_trampoline.edge += instruction.length;
    }
    offset += instruction.length;
    runtime_address += instruction.length;
  }

  uint8_t *stolen_bytes = malloc(offset);
  if (!stolen_bytes)
    goto end;
  // save stolen bytes
  memcpy(stolen_bytes, prologue, offset);

  // original instructions + jump
  // we need to execute stolen instructions first
  // then we will have to jump to prologue+offset
  if (!hook_protect(generated_trampoline.base, generated_trampoline.size))
    goto end;

  uint8_t *addr_to_jump = prologue + offset;
  // set addr
  memcpy(trampoline_return_shellcode + 2, &addr_to_jump, sizeof(addr_to_jump));

  memcpy(generated_trampoline.base + generated_trampoline.edge, trampoline_return_shellcode,
         sizeof(trampoline_return_shellcode));
  generated_trampoline.edge += sizeof(trampoline_return_shellcode);

  *trampoline = generated_trampoline.base;

  // set jmp hook
  memcpy(prologue, prologue_shellcode, sizeof(prologue_shellcode));

  // save everything
  hooks[hook_idx].addr = addr;
  hooks[hook_idx].trampoline = trampoline;
  hooks[hook_idx].stolen_bytes = stolen_bytes;
  hooks[hook_idx].stolen_bytes_size = offset;

  status = true;
end:
  if (!status) {
    free(generated_trampoline.base);
    free(stolen_bytes);
  }
  return status;
}

bool hook_remove(void *addr) {
  LOGGER_LOG(LOG, "removing hook for %p", addr);
  if (addr == NULL)
    return false;
  // iterate over hooks and look for addr
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (hooks[i].addr == addr) {
      memcpy(addr, hooks[i].stolen_bytes, hooks[i].stolen_bytes_size);
      free(*hooks[i].trampoline);
      free(hooks[i].stolen_bytes);
      return true;
    }
  }
  return false;
}