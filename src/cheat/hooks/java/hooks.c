#include "hooks.h"
#include "../../utils/logger.h"
#include <dlfcn.h>
#include <string.h>

struct {
  bool initialized;
  void *libjvm;
  _VMStructEntry *_gHotSpotVMStructs;
  struct {
    uint64_t i2i;
    uint64_t from_compiled;
    uint64_t from_interpreted;
  } method_offsets;

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

static bool java_hook_init() {
  // https://www.javacodegeeks.com/2014/01/mirror-mirror-using-reflection-to-look-inside-the-jvm-at-run-time.html
  java_hook_ctx.libjvm = dlopen("libjvm.so", RTLD_NOW);

  if (!java_hook_ctx.libjvm) {
    LOGGER_LOG(LOG, "failed to open libjvm.so");
    java_hook_ctx.initialized = false;
    return false;
  }

  LOGGER_LOG(LOG, "libjvm.so @ %p", java_hook_ctx.libjvm);
  java_hook_ctx._gHotSpotVMStructs = *(_VMStructEntry **)dlsym(java_hook_ctx.libjvm, "gHotSpotVMStructs");
  if (!java_hook_ctx._gHotSpotVMStructs) {
    LOGGER_LOG(FATAL, "failed to resolve gHotSpotVMStructs");
    java_hook_ctx.initialized = false;
    return false;
  }
  LOGGER_LOG(LOG, "gHotSpotVMStructs @ %p", java_hook_ctx._gHotSpotVMStructs);
  java_hook_ctx.initialized = true;

  for (size_t i = 0; java_hook_ctx._gHotSpotVMStructs[i].fieldName != NULL; i++) {
    const char *type_name = java_hook_ctx._gHotSpotVMStructs[i].typeName;
    const char *field_name = java_hook_ctx._gHotSpotVMStructs[i].fieldName;
    if (strcmp(type_name, "Method") != 0)
      continue;

    // Method->_i2i_entry
    // Method->_from_compiled_entry
    // Method->_from_interpreted_entry

    if (strcmp(field_name, "_i2i_entry") == 0) {
      java_hook_ctx.method_offsets.i2i = java_hook_ctx._gHotSpotVMStructs[i].offset;
      LOGGER_LOG(LOG, "i2i @ %lx", java_hook_ctx.method_offsets.i2i);
    }

    if (strcmp(field_name, "_from_compiled_entry") == 0) {
      java_hook_ctx.method_offsets.from_compiled = java_hook_ctx._gHotSpotVMStructs[i].offset;
      LOGGER_LOG(LOG, "_from_compiled_entry @ %lx", java_hook_ctx.method_offsets.from_compiled);
    }

    if (strcmp(field_name, "_from_interpreted_entry") == 0) {
      java_hook_ctx.method_offsets.from_interpreted = java_hook_ctx._gHotSpotVMStructs[i].offset;
      LOGGER_LOG(LOG, "_from_interpreted_entry @ %lx", java_hook_ctx.method_offsets.from_interpreted);
    }
  }

  return true;
}

bool java_hook_install(JNIEnv *env, jmethodID mid, void *detour) {
  if (!mid)
    return false;

  if (!java_hook_ctx.initialized) {
    if (!java_hook_init())
      return false;
  }

  __Method method = java_resolve_jmethod_id(mid);
  uintptr_t **i2i_entry = (uintptr_t **)(method + java_hook_ctx.method_offsets.i2i);
  uintptr_t **fi_entry = (uintptr_t **)(method + java_hook_ctx.method_offsets.from_interpreted);
  uintptr_t **fc_entry = (uintptr_t **)(method + java_hook_ctx.method_offsets.from_compiled);

  size_t free_slot;
  if (!java_hook_find_free_slot(&free_slot)) {
    LOGGER_LOG(FATAL, "hooks are full!");
    return false;
  }

  // backup
  java_hook_ctx.hooks[free_slot].orig_i2i_entry = *i2i_entry;
  java_hook_ctx.hooks[free_slot].orig_fi_entry = *fi_entry;
  java_hook_ctx.hooks[free_slot].orig_fc_entry = *fc_entry;

  *i2i_entry = detour;
  *fi_entry = detour;
  *fc_entry = detour;

  java_hook_ctx.hooks[free_slot].is_hooked = true;
  return true;
}