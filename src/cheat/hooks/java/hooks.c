#include "hooks.h"
#include "../../utils/logger.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include "../../game.h"

struct {
  bool initialized;
  void *libjvm;
  _VMStructEntry *_gHotSpotVMStructs; // used to retrieve ofsetts
  _VMTypeEntry *_gHotSpotVMTypes;     // used to retrieve size of the class
  size_t method_class_size;
  uintptr_t native_i2i;
  uintptr_t native_from_compiled;
  uintptr_t native_from_interpreted;
  struct {
    uint64_t i2i;
    uint64_t from_compiled;
    uint64_t from_interpreted;
    uint64_t access_flags;
    uint64_t code;
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

static bool java_hook_already_installed(jmethodID mid) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (java_hook_ctx.hooks[i].method == mid && java_hook_ctx.hooks[i].is_hooked)
      return true;
  }
  return false;
}

jmethodID java_get_copy(jmethodID mid) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (java_hook_ctx.hooks[i].method == mid)
      return java_hook_ctx.hooks[i].copy.mid;
  }
  return NULL;
}

static bool java_hook_init(JNIEnv *env) {
  // we also need to obtain i2i entry of some existent method.
  // Let's try java.lang.System -> currentTimeMillis()

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

  java_hook_ctx._gHotSpotVMTypes = *(_VMTypeEntry **)dlsym(java_hook_ctx.libjvm, "gHotSpotVMTypes");
  if (!java_hook_ctx._gHotSpotVMTypes) {
    LOGGER_LOG(FATAL, "failed to resolve gHotSpotVMTypes");
    java_hook_ctx.initialized = false;
    return false;
  }
  LOGGER_LOG(LOG, "gHotSpotVMTypes @ %p", java_hook_ctx._gHotSpotVMTypes);

  java_hook_ctx.initialized = true;

  // iterating structs
  for (size_t i = 0; java_hook_ctx._gHotSpotVMStructs[i].fieldName != NULL; i++) {
    const char *type_name = java_hook_ctx._gHotSpotVMStructs[i].typeName;
    const char *field_name = java_hook_ctx._gHotSpotVMStructs[i].fieldName;
    if (strcmp(type_name, "Method") != 0)
      continue;

    // Method->_i2i_entry
    // Method->_from_compiled_entry
    // Method->_from_interpreted_entry

    LOGGER_LOG(LOG, "%s", field_name);

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

    if (strcmp(field_name, "_access_flags") == 0) {
      java_hook_ctx.method_offsets.access_flags = java_hook_ctx._gHotSpotVMStructs[i].offset;
      LOGGER_LOG(LOG, "_access_flags @ %lx", java_hook_ctx.method_offsets.access_flags);
    }

    if (strcmp(field_name, "_code") == 0) {
      java_hook_ctx.method_offsets.code = java_hook_ctx._gHotSpotVMStructs[i].offset;
      LOGGER_LOG(LOG, "_code @ %lx", java_hook_ctx.method_offsets.code);
    }
  }

  // iterating types
  for (size_t i = 0; java_hook_ctx._gHotSpotVMTypes[i].typeName != NULL; i++) {
    const char *type_name = java_hook_ctx._gHotSpotVMTypes[i].typeName;
    size_t size = java_hook_ctx._gHotSpotVMTypes[i].size;
    if (strcmp(type_name, "Method") == 0) {
      java_hook_ctx.method_class_size = size;
      LOGGER_LOG(LOG, "Method size %lu", size);
      break;
    }
  }

  // get native i2i
  jclass system_class = (*env)->FindClass(env, "java/lang/System");
  CHECK(system_class);
  jmethodID current_time_millis = (*env)->GetStaticMethodID(env, system_class, "currentTimeMillis", "()J");
  CHECK(current_time_millis);
  __Method method = java_resolve_jmethod_id(current_time_millis);
  uintptr_t *i2i_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.i2i);
  uintptr_t *fi_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.from_interpreted);
  uintptr_t *fc_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.from_compiled);
  uint16_t *access_flags = (uint16_t *)(method + java_hook_ctx.method_offsets.access_flags);
  if ((*access_flags & JVM_ACC_NATIVE) != JVM_ACC_NATIVE) {
    LOGGER_LOG(FATAL, "unable to obtain native interpreter");
    return false;
  }
  java_hook_ctx.native_i2i = *i2i_entry;
  java_hook_ctx.native_from_interpreted = *fi_entry;
  java_hook_ctx.native_from_compiled = *fc_entry;
  LOGGER_LOG(LOG, "native i2i @ %lx", java_hook_ctx.native_i2i);

  return true;
}

bool java_hook_remove(JNIEnv *env, jmethodID mid) {
  for (size_t i = 0; i < MAX_HOOKS; i++) {
    if (java_hook_ctx.hooks[i].is_hooked && java_hook_ctx.hooks[i].method == mid) {
      *java_hook_ctx.hooks[i].pointers.i2i_entry = java_hook_ctx.hooks[i].backup.orig_i2i_entry;
      *java_hook_ctx.hooks[i].pointers.fi_entry = java_hook_ctx.hooks[i].backup.orig_fi_entry;
      *java_hook_ctx.hooks[i].pointers.fc_entry = java_hook_ctx.hooks[i].backup.orig_fc_entry;

      (*env)->UnregisterNatives(env, java_hook_ctx.hooks[i].class);
      *java_hook_ctx.hooks[i].pointers.access_flags = java_hook_ctx.hooks[i].backup.orig_access_flags;

      free(java_hook_ctx.hooks[i].copy.raw);
      free(java_hook_ctx.hooks[i].copy.mid);
      (*env)->DeleteGlobalRef(env, java_hook_ctx.hooks[i].class);
      java_hook_ctx.hooks[i].is_hooked = false;
      return true;
    }
  }
  return false;
}

bool java_hook_install(JNIEnv *env, jclass class, jmethodID mid, JNINativeMethod *detour) {
  if (!mid)
    return false;

  if (!java_hook_ctx.initialized) {
    if (!java_hook_init(env))
      return false;
  }

  if (java_hook_already_installed(mid)) {
    LOGGER_LOG(WARNING, "hook is already installed");
    return false;
  }

  __Method method = java_resolve_jmethod_id(mid);
  uintptr_t *i2i_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.i2i);
  uintptr_t *fi_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.from_interpreted);
  uintptr_t *fc_entry = (uintptr_t *)(method + java_hook_ctx.method_offsets.from_compiled);
  uintptr_t *code = (uintptr_t *)(method + java_hook_ctx.method_offsets.code);
  uint16_t *access_flags = (uint16_t *)(method + java_hook_ctx.method_offsets.access_flags);

  size_t free_slot;
  if (!java_hook_find_free_slot(&free_slot)) {
    LOGGER_LOG(FATAL, "hooks are full!");
    return false;
  }

  // backup
  java_hook_ctx.hooks[free_slot].method = mid;
  java_hook_ctx.hooks[free_slot].class = (*env)->NewGlobalRef(env, class);
  java_hook_ctx.hooks[free_slot].copy.raw = (__Method)malloc(java_hook_ctx.method_class_size);
  if (!java_hook_ctx.hooks[free_slot].copy.raw) {
    LOGGER_LOG(FATAL, "failed to allocate memory");
    return false;
  }

  // creating a copy of Method, so that we can call it using JNI
  memcpy(java_hook_ctx.hooks[free_slot].copy.raw, method, java_hook_ctx.method_class_size);
  __Method *m_slot = (__Method *)malloc(sizeof(__Method *));
  if (!m_slot) {
    LOGGER_LOG(FATAL, "failed to allocate memory");
    return false;
  }
  memcpy(m_slot, &java_hook_ctx.hooks[free_slot].copy.raw, sizeof(__Method));
  java_hook_ctx.hooks[free_slot].copy.mid = (jmethodID)m_slot;

  LOGGER_LOG(LOG, "current access flags: %u", *access_flags);

  // setting entries
  java_hook_ctx.hooks[free_slot].pointers.i2i_entry = i2i_entry;
  java_hook_ctx.hooks[free_slot].pointers.fi_entry = fi_entry;
  java_hook_ctx.hooks[free_slot].pointers.fc_entry = fc_entry;
  java_hook_ctx.hooks[free_slot].pointers.code = code;
  java_hook_ctx.hooks[free_slot].pointers.access_flags = access_flags;

  // backup
  java_hook_ctx.hooks[free_slot].backup.orig_i2i_entry = *i2i_entry;
  java_hook_ctx.hooks[free_slot].backup.orig_fi_entry = *fi_entry;
  java_hook_ctx.hooks[free_slot].backup.orig_fc_entry = *fc_entry;
  java_hook_ctx.hooks[free_slot].backup.orig_code = *code;
  java_hook_ctx.hooks[free_slot].backup.orig_access_flags = *access_flags;

  // set hook
  *access_flags |= JVM_ACC_NATIVE;
  LOGGER_LOG(LOG, "new access flags: %d", *access_flags);
  if ((*env)->RegisterNatives(env, class, detour, 1) != 0) {
    LOGGER_LOG(FATAL, "failed to register native method");
    return false;
  }

  LOGGER_LOG(LOG, "registered native method!");

  // setting native interpreter
  //*code = 0;
  memcpy(i2i_entry, &java_hook_ctx.native_i2i, sizeof(uintptr_t));
  LOGGER_LOG(LOG, "new i2i %lx", *i2i_entry);
  memcpy(fi_entry, &java_hook_ctx.native_from_interpreted, sizeof(uintptr_t));
  LOGGER_LOG(LOG, "new fi %lx", *fi_entry);
  memcpy(fc_entry, &java_hook_ctx.native_from_compiled, sizeof(uintptr_t));
  LOGGER_LOG(LOG, "all entries are set");

  // hooked
  java_hook_ctx.hooks[free_slot].is_hooked = true;
  return true;
}