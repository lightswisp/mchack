#include "Zydis/Register.h"
#include <signal.h>
#define _GNU_SOURCE
#include <linux/limits.h>
#include <libgen.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <elf.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>

#include <Zydis/Zydis.h>

typedef struct{
    pid_t tid;
    struct user_regs_struct regs;
} thread_t;

uintptr_t dlclose_addr;
void *handle;

pid_t find_pid(const char *pname){
    DIR *dir;
    struct dirent *ent;
    char path[PATH_MAX];
    char comm[256];
    pid_t target_pid = -1;

    if ((dir = opendir("/proc")) == NULL) {
        perror("Failed to open /proc");
        return -1;
    }

    while ((ent = readdir(dir)) != NULL) {
        int is_pid = 1;
        for (int i = 0; ent->d_name[i] != '\0'; i++) {
            if (!isdigit((unsigned char)ent->d_name[i])) {
                is_pid = 0;
                break;
            }
        }

        if (is_pid) {
            snprintf(path, sizeof(path), "/proc/%s/comm", ent->d_name);
            FILE *fp = fopen(path, "r");
            if (fp != NULL) {
                if (fgets(comm, sizeof(comm), fp) != NULL) {
                    size_t len = strlen(comm);
                    if (len > 0 && comm[len - 1] == '\n') {
                        comm[len - 1] = '\0';
                    }

                    if (strcmp(comm, pname) == 0) {
                        target_pid = (pid_t)atoi(ent->d_name);
                        fclose(fp);
                        break; 
                    }
                }
                fclose(fp);
            }
        }
    }
    
    closedir(dir);
    return target_pid;
}

void hexdump(const void *ptr, size_t buflen) {
    const unsigned char *buf = (const unsigned char *)ptr;
    size_t i, j;
    for (i = 0; i < buflen; i += 16) {
        printf("%08zx: ", i);
        for (j = 0; j < 16; j++) {
            if (i + j < buflen) {
                printf("%02x ", buf[i + j]);
            } else {
                printf("   "); 
            }
        }
        printf(" ");
        for (j = 0; j < 16; j++) {
            if (i + j < buflen) {
                printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        printf("\n");
    }
}

uintptr_t get_base(pid_t pid, const char *lib, bool exact){
    FILE *fd = NULL;
    uintptr_t base_addr = 0;
    char path[PATH_MAX] = {0};
    char maps_buf[1024];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    fd = fopen(path, "r");
    struct {
        uintptr_t a_s, a_e;
        char perms[5];
        uintptr_t offset;
        char dev[255];
        int inode;
        char pathname[PATH_MAX];
        bool has_path;
    } maps;

    while(fgets(maps_buf, sizeof(maps_buf), fd)){
        int n = sscanf(maps_buf, "%lx-%lx %4s %lx %254s %d %4095s", 
            &maps.a_s, 
            &maps.a_e, 
            maps.perms, 
            &maps.offset, 
            maps.dev, 
            &maps.inode, 
            maps.pathname);

        if(exact){
            const char *bname = basename(maps.pathname);
            if(bname == NULL) continue;
            if(strcmp(bname, lib) == 0){
                base_addr = maps.a_s;
                break;
            }
        }
        else{
            if(strstr(maps.pathname, lib) != NULL){
                base_addr = maps.a_s;
                break;
            }
        }
    }

    fclose(fd);
    return base_addr;
} 

// unused
ZyanU64 find_function_ret_addr(ZyanU64 runtime_address, ZyanU8 *instruction_buffer){
    ZyanUSize offset = 0; 
    ZydisDisassembledInstruction instruction; 
    while (ZYAN_SUCCESS(ZydisDisassembleIntel( 
        /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64, 
        /* runtime_address: */ runtime_address, 
        /* buffer:          */ instruction_buffer + offset, 
        /* length:          */ sizeof(instruction_buffer) - offset, 
        /* instruction:     */ &instruction 
    ))) { 
        printf("%016" PRIX64 "  %s\n", runtime_address, instruction.text); 
        if(instruction.info.mnemonic == ZYDIS_MNEMONIC_RET){
            break;
        }
        offset += instruction.info.length; 
        runtime_address += instruction.info.length; 
    } 
    return runtime_address;
}

uint8_t *read_memory(pid_t pid, uintptr_t addr, size_t size){
    uint8_t *buffer = malloc(size);
    uint64_t word = 0;

	for (int i=0; i < size; i+=sizeof(word)) {
        word = 0;
		if ((word = ptrace(PTRACE_PEEKTEXT, pid, addr + i, NULL)) == -1) {;
			fprintf(stderr, "error reading process memory\n");
            return NULL;
		}
        memcpy(buffer + i, &word, sizeof(word));
	}
    return buffer;
}

bool write_memory(pid_t pid, uintptr_t addr, uint8_t *data, size_t size){
    uint64_t word = 0; 
    for(int i = 0; i < size; i+=sizeof(word)){
        memcpy(&word, data+i, sizeof(word));
        if (ptrace(PTRACE_POKETEXT, pid, addr + i, word) == -1) {;
			fprintf(stderr, "error writing to process memory\n");
            return false;
		}
    }
    return true;
}

uintptr_t align(uintptr_t len, size_t alignment) {
    return (len + alignment - 1) & ~(alignment - 1);
}

char *dup_pad(const char *str, size_t pad){
    char *new_buf = calloc(strlen(str) + pad, 1);
    strcpy(new_buf, str);
    return new_buf;
}

ZyanU8 *generate_shellcode(uintptr_t rsp, uintptr_t dlopen_addr, const char *lib, ZyanUSize *shellcode_size){
    typedef struct {
         ZyanU8 encoded_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH]; 
         size_t instruction_size;
    } block_t;

    // mov rbx, 0x6767676767676767 | =====> this part is dynamic
    // push rbx                    |
    // ...
    // mov rdi, rsp 
    // mov rsi, RTLD_NOW
    // mov rax, dlopen_addr
    // sub rsp, stack_mod
    // call rax
    // int3
    ZydisEncoderRequest req; 
    ZyanU8 *shellcode = NULL;
    ZyanUSize temp_size = 0;
    size_t len = strlen(lib);
    // 8 byte alignment to chop the string 
    // because each mov instruction can hold 8 byte at max
    uintptr_t aligned = align(len, sizeof(uint64_t));
    // align it again by 16 bytes, because we need our stack to be aligned
    // x86-64 / ABI requires our stack to be aligned for simd ops
    // otherwise we will get segmentation fault
    // we assume that the stack is already aligned
    aligned = align(aligned, 16);
    fprintf(stdout, "aligned: %lu\n", aligned);

    uintptr_t stack_mod = rsp & 15; // mod(16)
    if(stack_mod != 0){
        fprintf(stdout, "warning! stack is misaligned\n");
    }

    size_t pad = aligned - len;
    char *padded = dup_pad(lib, pad);
    size_t instructions = (aligned/8)*2 + 6;
    // nop sled for ptrace write, because we write in 8 bytes
    size_t nop_sled_size = align(instructions, sizeof(uint64_t)) - instructions; 
    instructions += nop_sled_size;
    fprintf(stdout, "instructions: %lu\n", instructions);
    block_t *blocks = calloc(instructions, sizeof(block_t));
    size_t current_block = 0;


    for(int i = aligned-8; i >= 0; i-=8){

        uint64_t chunk = 0;
        memcpy(&chunk, padded+i, sizeof(chunk));
        memset(&req, 0, sizeof(req)); 
        
        req.mnemonic = ZYDIS_MNEMONIC_MOV; 
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
        req.operand_count = 2; 
        req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER; 
        req.operands[0].reg.value = ZYDIS_REGISTER_RBX; 
        req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE; 
        // force 64bit operand
        req.operands[1].imm.u = 0x1122334455667788; 
        blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
        { 
            fprintf(stderr,"failed to encode mov 1 instruction\n"); 
            goto end; 
        } 
        memcpy(blocks[current_block].encoded_instruction + 2, &chunk, sizeof(uint64_t));
        temp_size+=blocks[current_block].instruction_size;
        current_block++;

        memset(&req, 0, sizeof(req)); 
        req.mnemonic = ZYDIS_MNEMONIC_PUSH; 
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
        req.operand_count = 1; 
        req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER; 
        req.operands[0].reg.value = ZYDIS_REGISTER_RBX; 
        blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
        { 
            fprintf(stderr,"failed to encode push instruction\n"); 
            goto end; 
        } 
        temp_size+=blocks[current_block].instruction_size;
        current_block++;
    }
    
    // mov rdi, rsp
    memset(&req, 0, sizeof(req)); 
    req.mnemonic = ZYDIS_MNEMONIC_MOV; 
    req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
    req.operand_count = 2; 
    req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER; 
    req.operands[0].reg.value = ZYDIS_REGISTER_RDI; 
    req.operands[1].type = ZYDIS_OPERAND_TYPE_REGISTER; 
    req.operands[1].reg.value = ZYDIS_REGISTER_RSP; 
    blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
    { 
        fprintf(stderr,"failed to encode mov instruction\n"); 
        goto end; 
    } 
    temp_size+=blocks[current_block].instruction_size;
    current_block++;

    // mov rsi, RTLD_NOW
    memset(&req, 0, sizeof(req)); 
    req.mnemonic = ZYDIS_MNEMONIC_MOV; 
    req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
    req.operand_count = 2; 
    req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER; 
    req.operands[0].reg.value = ZYDIS_REGISTER_RSI; 
    req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE; 
    req.operands[1].imm.u = RTLD_NOW; 
    blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
    { 
        fprintf(stderr,"failed to encode mov instruction\n"); 
        goto end; 
    } 
    temp_size+=blocks[current_block].instruction_size;
    current_block++;
    
    // mov rax, dlopen
    memset(&req, 0, sizeof(req)); 
    req.mnemonic = ZYDIS_MNEMONIC_MOV; 
    req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
    req.operand_count = 2; 
    req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER; 
    req.operands[0].reg.value = ZYDIS_REGISTER_RAX; 
    req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE; 
    req.operands[1].imm.u = dlopen_addr; 
    blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
    { 
        fprintf(stderr,"failed to encode mov instruction\n"); 
        goto end; 
    } 
    temp_size+=blocks[current_block].instruction_size;
    current_block++;
    
    // sub rsp, stack_mod
    memset(&req, 0, sizeof(req)); 
    req.mnemonic = ZYDIS_MNEMONIC_SUB; 
    req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
    req.operand_count = 2; 
    req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER; 
    req.operands[0].reg.value = ZYDIS_REGISTER_RSP; 
    req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE; 
    req.operands[1].imm.u = stack_mod; 
    blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
    { 
        fprintf(stderr,"failed to encode sub instruction\n"); 
        goto end; 
    } 
    temp_size+=blocks[current_block].instruction_size;
    current_block++;
    
    // call dlopen
    memset(&req, 0, sizeof(req)); 
    req.mnemonic = ZYDIS_MNEMONIC_CALL; 
    req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
    req.operand_count = 1; 
    req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER; 
    req.operands[0].reg.value = ZYDIS_REGISTER_RAX; 
    blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
    { 
        fprintf(stderr,"failed to encode call instruction\n"); 
        goto end; 
    } 
    temp_size+=blocks[current_block].instruction_size;
    current_block++;

    // int3
    memset(&req, 0, sizeof(req)); 
    req.mnemonic = ZYDIS_MNEMONIC_INT3; 
    req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
    req.operand_count = 0; 
    blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
    { 
        fprintf(stderr,"failed to encode int3 instruction\n"); 
        goto end; 
    } 
    temp_size+=blocks[current_block].instruction_size;
    current_block++;


    for(size_t i = 0; i < nop_sled_size; i++){
        memset(&req, 0, sizeof(req)); 
        req.mnemonic = ZYDIS_MNEMONIC_NOP; 
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64; 
        req.operand_count = 0; 
        blocks[current_block].instruction_size = ZYDIS_MAX_INSTRUCTION_LENGTH;
        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, blocks[current_block].encoded_instruction, &blocks[current_block].instruction_size))) 
        { 
            fprintf(stderr,"failed to encode nop instruction\n"); 
            goto end; 
        } 
        temp_size+=blocks[current_block].instruction_size;
        current_block++;
    }

    shellcode = malloc(temp_size);
    *shellcode_size = temp_size;
    size_t shellcode_offset = 0;
    for(size_t i = 0; i < instructions; i++){
        memcpy(shellcode + shellcode_offset, blocks[i].encoded_instruction, blocks[i].instruction_size);
        shellcode_offset += blocks[i].instruction_size;
    }

end:
    free(padded);
    free(blocks);
    return shellcode;
}

void pause_all_threads(pid_t pid, bool just_interrupt){
    DIR *dir;
    struct dirent *ent;
    char path[PATH_MAX] = {0};
    snprintf(path, sizeof(path), "/proc/%d/task", pid);

    if ((dir = opendir(path)) == NULL) {
        perror("Failed to open /proc");
        return;
    }
    while ((ent = readdir(dir)) != NULL) {
        pid_t tid = atoi(ent->d_name);
        if (tid == pid || tid == 0) 
            continue;
        if(just_interrupt){
            ptrace(PTRACE_INTERRUPT, tid, NULL, NULL);
            fprintf(stdout, "stopped thread: %d\n", tid);
        }
        else{
            ptrace(PTRACE_SEIZE, tid, NULL, NULL);
            ptrace(PTRACE_INTERRUPT, tid, NULL, NULL);
            waitpid(tid, NULL, 0);
            fprintf(stdout, "stopped thread: %d\n", tid);
        }
    }
    closedir(dir);
}

void resume_all_threads(pid_t pid){
    DIR *dir;
    struct dirent *ent;
    char path[PATH_MAX] = {0};
    snprintf(path, sizeof(path), "/proc/%d/task", pid);

    if ((dir = opendir(path)) == NULL) {
        perror("Failed to open /proc");
        return;
    }
    while ((ent = readdir(dir)) != NULL) {
        pid_t tid = atoi(ent->d_name);
        ptrace(PTRACE_CONT, tid, NULL, NULL);
    }

    closedir(dir);
}

void detach_all_threads(pid_t pid){
    DIR *dir;
    struct dirent *ent;
    char path[PATH_MAX] = {0};
    snprintf(path, sizeof(path), "/proc/%d/task", pid);

    if ((dir = opendir(path)) == NULL) {
        perror("Failed to open /proc");
        return;
    }
    while ((ent = readdir(dir)) != NULL) {
        pid_t tid = atoi(ent->d_name);
        ptrace(PTRACE_DETACH, tid, NULL, NULL);
    }

    closedir(dir);
}

thread_t* save_threads_state(pid_t pid, size_t *thread_num){
    DIR *dir;
    thread_t *states = NULL;
    size_t entries = 0;
    struct dirent *ent;
    char path[PATH_MAX] = {0};
    snprintf(path, sizeof(path), "/proc/%d/task", pid);

    if ((dir = opendir(path)) == NULL) {
        perror("Failed to open /proc");
        return NULL;
    }
    // count
    while ((ent = readdir(dir)) != NULL) entries++;
    rewinddir(dir);
    // allocate
    states = calloc(entries, sizeof(thread_t));
    size_t idx = 0;
    while ((ent = readdir(dir)) != NULL){
        pid_t tid = atoi(ent->d_name);
        states[idx].tid = tid;
        ptrace(PTRACE_GETREGS, tid, NULL, &states[idx].regs);
        idx++;
    }

    *thread_num = entries;
    closedir(dir);
    return states;
}

void handle_trap(pid_t pid, struct user_regs_struct *old_regs, uintptr_t old_entry, uint8_t *backup_buffer, size_t size){
    struct user_regs_struct regs;
    while(true){
        int status;
        pid_t tid = waitpid(-1, &status, __WALL);
        if(tid == -1){
            fprintf(stderr, "waitpid err\n");
            break;
        }

        if(WIFSTOPPED(status)){
            int signal = WSTOPSIG(status);
            if(signal == SIGTRAP){
                fprintf(stdout, "trap at %d!\n", tid);
                if(tid != pid){
                    continue;
                } 
                ptrace(PTRACE_GETREGS, tid, NULL, &regs);

                // debug
                fprintf(stdout, "rsp on interrupt: 0x%llx\n", regs.rsp); 

                handle = (void*)regs.rax;
                //pause_all_threads(pid, true);
                // restore instructions
                write_memory(tid, old_entry, backup_buffer, size);
                fprintf(stdout, "restoring registers state\n");
                ptrace(PTRACE_SETREGS, tid, NULL, old_regs);
                ptrace(PTRACE_CONT, tid, NULL, NULL);
                break;
            }
            else{
                fprintf(stdout, "skip the signal %d for %d\n", signal, tid);
                ptrace(PTRACE_CONT, tid, NULL, signal);
            }
        }

    }
}

bool inject(pid_t pid, const char *process_name, const char *lib){
    #define MAX_FUNCTION_SIZE 512
    int success = false;
    uint8_t instruction_buffer[MAX_FUNCTION_SIZE];
    struct user_regs_struct old_regs, regs;
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        fprintf(stderr, "attach failed\n");
        return false;
    }
    waitpid(pid, NULL, 0);
    ptrace(PTRACE_GETREGS, pid, NULL, &regs); 
    ptrace(PTRACE_GETREGS, pid, NULL, &old_regs); 
    printf("attached to %d\n", pid);

    uintptr_t dlopen_offset, dlclose_offset;
    uintptr_t external_process_base = get_base(pid, process_name, true);
    Elf64_Ehdr *external_elf_header = (Elf64_Ehdr*)read_memory(pid, external_process_base, sizeof(Elf64_Ehdr));
    uintptr_t external_module_base = get_base(pid, "libc.so", false);
    uintptr_t local_module_base = get_base(getpid(), "libc.so", false);

    dlopen_offset  = (uintptr_t)dlopen  - local_module_base;
    dlclose_offset = (uintptr_t)dlclose - local_module_base;

    uintptr_t dlopen_addr = external_module_base + dlopen_offset;

    // saving for later use
    dlclose_addr = external_module_base + dlclose_offset;

    fprintf(stdout, "dlopen memory address: %p\n", (void*)dlopen_addr);
    fprintf(stdout, "dlclose memory address: %p\n", (void*)dlclose_addr);
    
    // now let's read the entry point addr
    uintptr_t external_entry = external_elf_header->e_entry + external_process_base;
    fprintf(stdout, "base: 0x%lx\n", external_process_base);
    fprintf(stdout, "sig: %s\n", external_elf_header->e_ident);
    fprintf(stdout, "entry: 0x%lx\n", external_entry);

    ZyanUSize shellcode_size = 0;
    ZyanU8 *shellcode = generate_shellcode(regs.rsp, dlopen_addr, lib, &shellcode_size);

    fprintf(stdout, "generated shellcode: %p\n", shellcode);
    hexdump((void*)shellcode, shellcode_size);


    // preserving instructions, reading from entry point
    uint8_t *backup_buffer = read_memory(pid, external_entry, shellcode_size);
    fprintf(stdout, "regs.rip = %llx | oldregs.rip = %llx\n", regs.rip, old_regs.rip);
    hexdump(backup_buffer, shellcode_size);

    // write shellcode
    if(!write_memory(pid, external_entry, shellcode, shellcode_size)) return false;
    fprintf(stdout, "rsp = 0x%llx\n", regs.rsp);

    regs.rip = external_entry;
    // sometimes rip is substracted by 2 bytes which can cause segfaults
    // https://gist.github.com/zhangyoufu/86910448ec92c7eca7e39186b79c98c2
    regs.rax = -1;
    fprintf(stdout, "set rip to 0x%llx\n", regs.rip);
    ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    ptrace(PTRACE_CONT, pid, NULL, NULL);
    handle_trap(pid, &old_regs, external_entry, backup_buffer, shellcode_size);

    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    success = true;

end:
    free(backup_buffer);
    free(shellcode);
    return success;
}

int main(void){

    #define LIB_NAME "libmchack.so"
    #define PROCESS_NAME "java"

    char *absolute_path = realpath(LIB_NAME, NULL);
    fprintf(stdout,"absolute path: %s\n", absolute_path);

    if(geteuid() != 0){
        fprintf(stderr, "run as root please\n");
        goto end;
    }

    pid_t pid = find_pid(PROCESS_NAME);
    if(pid == -1){
        fprintf(stderr, "unable to find pid of java\n");
        goto end;
    }

    printf("java pid: %d\n", pid);
    if(inject(pid, PROCESS_NAME, absolute_path) == false) goto end;
    fprintf(stdout,"handle: %p\n", handle);

    /* tail the log file output */

end:
    return 0;
}