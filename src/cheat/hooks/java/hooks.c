#include "hooks.h"
#include "../../utils/logger.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "../../game.h"

struct {
  bool initialized;
  void *libjvm;
  bool is_legacy;
  _VMStructEntry *_gHotSpotVMStructs;           // used to retrieve ofsetts
  _VMTypeEntry *_gHotSpotVMTypes;               // used to retrieve size of the class
  _VMIntConstantEntry *_gHotSpotVMIntConstants; // inlining constants
  size_t method_class_size;
  struct {
    uint64_t i2i;
    uint64_t from_compiled;
    uint64_t from_interpreted;
    uint64_t flags;
    uint64_t code;
  } method_offsets;

  // it will contain all natives
  jclass dummy;

#define MAX_HOOKS 512
  java_hook_t hooks[MAX_HOOKS];
} java_hook_ctx = {0};

static __Method java_resolve_jmethod_id(jmethodID mid) { return *((__Method *)mid); }

static bool java_hook_find_free_slot(size_t *slot) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (!java_hook_ctx.hooks[i].is_hooked) {
      *slot = i;
      return true;
    }
  }
  return false;
}

static bool java_hook_already_installed(jmethodID mid) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (java_hook_ctx.hooks[i].method == mid && java_hook_ctx.hooks[i].is_hooked)
      return true;
  }
  return false;
}

void java_unhook(jmethodID mid) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (java_hook_ctx.hooks[i].is_hooked && java_hook_ctx.hooks[i].method == mid) {
      *java_hook_ctx.hooks[i].pointers.i2i_entry = java_hook_ctx.hooks[i].backup.orig_i2i_entry;
      *java_hook_ctx.hooks[i].pointers.fi_entry = java_hook_ctx.hooks[i].backup.orig_fi_entry;
      *java_hook_ctx.hooks[i].pointers.fc_entry = java_hook_ctx.hooks[i].backup.orig_fc_entry;
    }
  }
}

void java_rehook(jmethodID mid) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (java_hook_ctx.hooks[i].is_hooked && java_hook_ctx.hooks[i].method == mid) {
      memcpy(java_hook_ctx.hooks[i].pointers.i2i_entry, &java_hook_ctx.hooks[i].i2i_stub, sizeof(uintptr_t));
      memcpy(java_hook_ctx.hooks[i].pointers.fi_entry, &java_hook_ctx.hooks[i].fi_stub, sizeof(uintptr_t));
      memcpy(java_hook_ctx.hooks[i].pointers.fc_entry, &java_hook_ctx.hooks[i].fc_stub, sizeof(uintptr_t));
    }
  }
}

static bool java_get_class_offset(const char *class, const char *field, uint64_t *offset) {
  for (size_t i = 0; java_hook_ctx._gHotSpotVMStructs[i].fieldName != NULL; i++) {
    const char *type_name = java_hook_ctx._gHotSpotVMStructs[i].typeName;
    const char *field_name = java_hook_ctx._gHotSpotVMStructs[i].fieldName;
    uint64_t _offset = java_hook_ctx._gHotSpotVMStructs[i].offset;
    if (strcmp(type_name, class) == 0 && strcmp(field_name, field) == 0) {
      *offset = _offset;
      return true;
    }
  }
  LOGGER_LOG(FATAL, "failed to find %s", field);
  return false;
}

static bool java_init_structs() {
  java_hook_ctx._gHotSpotVMStructs = *(_VMStructEntry **)dlsym(java_hook_ctx.libjvm, "gHotSpotVMStructs");
  if (!java_hook_ctx._gHotSpotVMStructs) {
    LOGGER_LOG(FATAL, "failed to resolve gHotSpotVMStructs");
    return false;
  }
  LOGGER_LOG(LOG, "gHotSpotVMStructs @ %p", java_hook_ctx._gHotSpotVMStructs);

  java_hook_ctx._gHotSpotVMTypes = *(_VMTypeEntry **)dlsym(java_hook_ctx.libjvm, "gHotSpotVMTypes");
  if (!java_hook_ctx._gHotSpotVMTypes) {
    LOGGER_LOG(FATAL, "failed to resolve gHotSpotVMTypes");
    return false;
  }
  LOGGER_LOG(LOG, "gHotSpotVMTypes @ %p", java_hook_ctx._gHotSpotVMTypes);

  java_hook_ctx._gHotSpotVMIntConstants =
      *(_VMIntConstantEntry **)dlsym(java_hook_ctx.libjvm, "gHotSpotVMIntConstants");
  if (!java_hook_ctx._gHotSpotVMIntConstants) {
    LOGGER_LOG(FATAL, "failed to resolve gHotSpotVMIntConstants");
    return false;
  }
  LOGGER_LOG(LOG, "gHotSpotVMIntConstants @ %p", java_hook_ctx._gHotSpotVMIntConstants);

  return true;
}

static bool java_register_dummy_class(JNIEnv *env) {
  // register class with native methods
  const uint8_t class_data[] = {
#embed "../../dummy/classes.class"
  };
  jclass dummy = (*env)->DefineClass(env, "classes", NULL, (jbyte *)class_data, sizeof(class_data));
  if (!dummy) {
    LOGGER_LOG(WARNING, "failed to define dummy class");
    // maybe it was already loaded
    dummy = (*env)->FindClass(env, "classes");
    if (!dummy) {
      LOGGER_LOG(FATAL, "unable to load and find dummy class");
      return false;
    }
  }

  LOGGER_LOG(LOG, "successfully set our dummy class");
  java_hook_ctx.dummy = (*env)->NewGlobalRef(env, dummy);
  return true;
}

static bool java_get_class_size(const char *name, size_t *size) {
  // iterating types
  for (size_t i = 0; java_hook_ctx._gHotSpotVMTypes[i].typeName != NULL; i++) {
    const char *type_name = java_hook_ctx._gHotSpotVMTypes[i].typeName;
    size_t _size = java_hook_ctx._gHotSpotVMTypes[i].size;
    if (strcmp(type_name, name) == 0) {
      *size = _size;
      LOGGER_LOG(LOG, "%s size %lu", name, _size);
      return true;
    }
  }
  return false;
}

[[maybe_unused]] static bool java_get_int_constant(const char *name, int32_t *value) {
  // iterating constants
  for (size_t i = 0; java_hook_ctx._gHotSpotVMIntConstants[i].name != NULL; i++) {
    const char *_name = java_hook_ctx._gHotSpotVMIntConstants[i].name;
    int32_t _value = java_hook_ctx._gHotSpotVMIntConstants[i].value;
    if (strcmp(name, _name) == 0) {
      *value = _value;
      return true;
    }
  }
  return false;
}

static bool java_hook_init(JNIEnv *env) {
  // https://www.javacodegeeks.com/2014/01/mirror-mirror-using-reflection-to-look-inside-the-jvm-at-run-time.html
  java_hook_ctx.libjvm = dlopen("libjvm.so", RTLD_NOW);

  if (!java_hook_ctx.libjvm) {
    LOGGER_LOG(LOG, "failed to open libjvm.so");
    return false;
  }
  LOGGER_LOG(LOG, "libjvm.so @ %p", java_hook_ctx.libjvm);

  if (!java_init_structs()) {
    LOGGER_LOG(FATAL, "failed to init hotspot structs");
    return false;
  }

  if (!(java_get_class_offset("Method", "_i2i_entry", &java_hook_ctx.method_offsets.i2i) &&
        java_get_class_offset("Method", "_from_compiled_entry", &java_hook_ctx.method_offsets.from_compiled) &&
        java_get_class_offset("Method", "_from_interpreted_entry", &java_hook_ctx.method_offsets.from_interpreted) &&
        java_get_class_offset("Method", "_code", &java_hook_ctx.method_offsets.code))) {
    LOGGER_LOG(FATAL, "failed to resolve offsets");
    return false;
  }

  // in java 20, we are looking for _flags field, however in older java versions, this field doesn't exist.
  // so, we will fallback to _access_flags instead
  if (java_get_class_offset("Method", "_flags", &java_hook_ctx.method_offsets.flags)) {
    java_hook_ctx.is_legacy = false;
  } else if (java_get_class_offset("Method", "_access_flags", &java_hook_ctx.method_offsets.flags)) {
    java_hook_ctx.is_legacy = true;
  } else {
    LOGGER_LOG(FATAL, "failed to find flags offset");
    return false;
  }

  // it's not used anywhere in the code for now, but i'll leave it anyways
  if (!java_get_class_size("Method", &java_hook_ctx.method_class_size)) {
    LOGGER_LOG(FATAL, "failed to get Method size");
    return false;
  }

  if (!java_register_dummy_class(env))
    return false;

  java_hook_ctx.initialized = true;
  return true;
}

bool java_hook_remove(jmethodID mid) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (java_hook_ctx.hooks[i].is_hooked && java_hook_ctx.hooks[i].method == mid) {
      *java_hook_ctx.hooks[i].pointers.i2i_entry = java_hook_ctx.hooks[i].backup.orig_i2i_entry;
      *java_hook_ctx.hooks[i].pointers.fi_entry = java_hook_ctx.hooks[i].backup.orig_fi_entry;
      *java_hook_ctx.hooks[i].pointers.fc_entry = java_hook_ctx.hooks[i].backup.orig_fc_entry;
      *java_hook_ctx.hooks[i].pointers.flags = java_hook_ctx.hooks[i].backup.orig_flags;

      free(java_hook_ctx.hooks[i].i2i_stub);
      free(java_hook_ctx.hooks[i].fi_stub);
      free(java_hook_ctx.hooks[i].fc_stub);
      java_hook_ctx.hooks[i].is_hooked = false;
      return true;
    }
  }
  return false;
}

static void java_get_entries(__Method method, entries_t *entries) {
  entries->i2i_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.i2i);
  entries->fi_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.from_interpreted);
  entries->fc_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.from_compiled);
  entries->flags = (uint32_t *)(method + java_hook_ctx.method_offsets.flags);
  entries->code = (uintptr_t *)(method + java_hook_ctx.method_offsets.code);
}

static void java_get_entries_values(entries_t *entries, entries_values_t *entries_values) {
  entries_values->orig_i2i_entry = *entries->i2i_entry;
  entries_values->orig_fi_entry = *entries->fi_entry;
  entries_values->orig_fc_entry = *entries->fc_entry;
  entries_values->orig_flags = *entries->flags;
  entries_values->orig_code = *entries->code;
}

static void java_set_method_noinline(__Method method) {
  uint32_t mask = 0;
  entries_t entries;
  java_get_entries(method, &entries);
  if (java_hook_ctx.is_legacy) {
    mask = JVM_ACC_NOT_C1_COMPILABLE | JVM_ACC_NOT_C2_COMPILABLE | JVM_ACC_NOT_C2_OSR_COMPILABLE;
    LOGGER_LOG(LOG, "legacy noinline flags: %u", mask);
  } else {
    mask = DONT_INLINE | IS_NOT_C2_COMPILABLE | IS_NOT_C1_COMPILABLE | IS_NOT_C2_OSR_COMPILABLE;
    LOGGER_LOG(LOG, "modern noinline flags: %u", mask);
  }
  *entries.flags |= mask;
}

static bool java_hook_protect(void *addr, size_t len) {
  size_t page_size = sysconf(_SC_PAGE_SIZE);
  void *addr_aligned = (void *)((uintptr_t)addr & ~(page_size - 1));
  size_t aligned_len = (((uintptr_t)addr + len - (uintptr_t)addr_aligned) + page_size - 1) & ~(page_size - 1);

  if (mprotect(addr_aligned, aligned_len, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
    LOGGER_LOG(FATAL, "mprotect failed with %s", strerror(errno));
    return false;
  }
  return true;
}

static uint8_t *generate_i2i_stub(__Method native_method, entries_values_t *native_entries_values) {
  uint8_t i2i_stub[] = {
      0x48, 0xBB, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
      0x11,                                           // mov rbx, Method
      0x48, 0x83, 0xEC, 0x08,                         // sub rsp, 8
      0xC7, 0x04, 0x24, 0x78, 0x56, 0x34, 0x12,       // mov [rsp], lower_bits
      0xC7, 0x44, 0x24, 0x04, 0xFF, 0x7F, 0x00, 0x00, // mov [rsp+4], upper_bits
      0xC3                                            // ret
  };

  uint32_t i2i_lower = native_entries_values->orig_i2i_entry & 0xffffffff;
  uint32_t i2i_upper = native_entries_values->orig_i2i_entry >> 32;
  memcpy(i2i_stub + 2, &native_method, sizeof(native_method));
  memcpy(i2i_stub + 17, &i2i_lower, sizeof(i2i_lower));
  memcpy(i2i_stub + 25, &i2i_upper, sizeof(i2i_upper));

  uint8_t *stub = malloc(sizeof(i2i_stub));

  if (!stub) {
    LOGGER_LOG(FATAL, "failed to allocate mempory");
    return NULL;
  }
  memcpy(stub, i2i_stub, sizeof(i2i_stub));
  if (!java_hook_protect(stub, sizeof(i2i_stub))) {
    LOGGER_LOG(FATAL, "protection failed");
    free(stub);
    return NULL;
  }

  return stub;
}

static uint8_t *generate_fi_stub(__Method native_method, entries_values_t *native_entries_values) {
  uint8_t from_interpreted_stub[] = {
      0x48, 0xBB, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
      0x11,                                           // mov rbx, Method
      0x48, 0x83, 0xEC, 0x08,                         // sub rsp, 8
      0xC7, 0x04, 0x24, 0x78, 0x56, 0x34, 0x12,       // mov [rsp], lower_bits
      0xC7, 0x44, 0x24, 0x04, 0xFF, 0x7F, 0x00, 0x00, // mov [rsp+4], upper_bits
      0xC3                                            // ret
  };

  uint32_t fi_lower = native_entries_values->orig_fi_entry & 0xffffffff;
  uint32_t fi_upper = native_entries_values->orig_fi_entry >> 32;
  memcpy(from_interpreted_stub + 2, &native_method, sizeof(native_method));
  memcpy(from_interpreted_stub + 17, &fi_lower, sizeof(fi_lower));
  memcpy(from_interpreted_stub + 25, &fi_upper, sizeof(fi_upper));

  uint8_t *stub = malloc(sizeof(from_interpreted_stub));

  if (!stub) {
    LOGGER_LOG(FATAL, "failed to allocate mempory");
    return NULL;
  }
  memcpy(stub, from_interpreted_stub, sizeof(from_interpreted_stub));
  if (!java_hook_protect(stub, sizeof(from_interpreted_stub))) {
    LOGGER_LOG(FATAL, "protection failed");
    free(stub);
    return NULL;
  }

  return stub;
}

static uint8_t *generate_fc_stub(__Method native_method, entries_values_t *native_entries_values) {
  uint8_t fc_stub[] = {
      0x48, 0xBB, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
      0x11,                                           // mov rbx, Method
      0x48, 0x83, 0xEC, 0x08,                         // sub rsp, 8
      0xC7, 0x04, 0x24, 0x78, 0x56, 0x34, 0x12,       // mov [rsp], lower_bits
      0xC7, 0x44, 0x24, 0x04, 0xFF, 0x7F, 0x00, 0x00, // mov [rsp+4], upper_bits
      0xC3                                            // ret
  };

  uint32_t fc_lower = native_entries_values->orig_fc_entry & 0xffffffff;
  uint32_t fc_upper = native_entries_values->orig_fc_entry >> 32;
  memcpy(fc_stub + 2, &native_method, sizeof(native_method));
  memcpy(fc_stub + 17, &fc_lower, sizeof(fc_lower));
  memcpy(fc_stub + 25, &fc_upper, sizeof(fc_upper));

  uint8_t *stub = malloc(sizeof(fc_stub));

  if (!stub) {
    LOGGER_LOG(FATAL, "failed to allocate mempory");
    return NULL;
  }
  memcpy(stub, fc_stub, sizeof(fc_stub));
  if (!java_hook_protect(stub, sizeof(fc_stub))) {
    LOGGER_LOG(FATAL, "protection failed");
    free(stub);
    return NULL;
  }

  return stub;
}

bool java_hook_install(JNIEnv *env, jmethodID mid, JNINativeMethod *detour) {
  int status = false;

  if (!mid)
    goto end;

  if (!java_hook_ctx.initialized) {
    if (!java_hook_init(env))
      goto end;
  }

  if (java_hook_already_installed(mid)) {
    LOGGER_LOG(WARNING, "hook is already installed");
    goto end;
  }

  __Method method = java_resolve_jmethod_id(mid);
  entries_t entries;
  entries_values_t entries_values;
  java_get_entries(method, &entries);
  java_get_entries_values(&entries, &entries_values);

  size_t free_slot;
  if (!java_hook_find_free_slot(&free_slot)) {
    LOGGER_LOG(FATAL, "hooks are full!");
    goto end;
  }

  // backup
  java_hook_ctx.hooks[free_slot].method = mid;

  // setting entries
  memcpy(&java_hook_ctx.hooks[free_slot].pointers, &entries, sizeof(entries_t));
  // backup
  memcpy(&java_hook_ctx.hooks[free_slot].backup, &entries_values, sizeof(entries_values_t));

  // register our dummy method
  if ((*env)->RegisterNatives(env, java_hook_ctx.dummy, detour, 1) != 0) {
    LOGGER_LOG(FATAL, "failed to register native method");
    goto end;
  }

  // get Method ptr
  jmethodID native_mid = (*env)->GetMethodID(env, java_hook_ctx.dummy, detour->name, detour->signature);
  __Method native_method = java_resolve_jmethod_id(native_mid);
  entries_t native_entries;
  entries_values_t native_entries_values;
  java_get_entries(native_method, &native_entries);
  java_get_entries_values(&native_entries, &native_entries_values);

  // generate stubs
  uint8_t *i2i_stub = generate_i2i_stub(native_method, &native_entries_values);
  if (!i2i_stub) {
    LOGGER_LOG(FATAL, "failed to generate i2i stub");
    goto end;
  }
  java_hook_ctx.hooks[free_slot].i2i_stub = i2i_stub;
  LOGGER_LOG(LOG, "i2i stub @ %p", i2i_stub);

  uint8_t *fi_stub = generate_fi_stub(native_method, &native_entries_values);
  if (!fi_stub) {
    LOGGER_LOG(FATAL, "failed to generate fi stub");
    goto end;
  }
  java_hook_ctx.hooks[free_slot].fi_stub = fi_stub;
  LOGGER_LOG(LOG, "fi stub @ %p", fi_stub);

  uint8_t *fc_stub = generate_fc_stub(native_method, &native_entries_values);
  if (!fc_stub) {
    LOGGER_LOG(FATAL, "failed to generate fc stub");
    goto end;
  }
  java_hook_ctx.hooks[free_slot].fc_stub = fc_stub;
  LOGGER_LOG(LOG, "fc stub @ %p", fc_stub);

  java_set_method_noinline(method);

  memcpy(entries.i2i_entry, &java_hook_ctx.hooks[free_slot].i2i_stub, sizeof(uintptr_t));
  LOGGER_LOG(LOG, "new i2i %lx", *entries.i2i_entry);
  memcpy(entries.fi_entry, &java_hook_ctx.hooks[free_slot].fi_stub, sizeof(uintptr_t));
  LOGGER_LOG(LOG, "new fi %lx", *entries.fi_entry);
  memcpy(entries.fc_entry, &java_hook_ctx.hooks[free_slot].fc_stub, sizeof(uintptr_t));
  LOGGER_LOG(LOG, "new fc %lx", *entries.fc_entry);
  LOGGER_LOG(LOG, "all entries are set");

  // hooked
  status = true;
  java_hook_ctx.hooks[free_slot].is_hooked = true;

end:
  if (!status) {
    free(java_hook_ctx.hooks[free_slot].i2i_stub);
    free(java_hook_ctx.hooks[free_slot].fi_stub);
    free(java_hook_ctx.hooks[free_slot].fc_stub);
  }
  return status;
}