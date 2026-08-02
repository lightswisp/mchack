#pragma once
#include <stdint.h>
#include "../../jvm.h"

typedef uintptr_t *__Method;

// taken from jdk source code @ vmStructs.hpp
typedef struct {
  const char *typeName;   // The type name containing the given field (example: "Klass")
  const char *fieldName;  // The field name within the type           (example: "_name")
  const char *typeString; // Quoted name of the type of this field (example: "Symbol*";
                          // parsed in Java to ensure type correctness
  int32_t isStatic;       // Indicates whether following field is an offset or an address
  uint64_t offset;        // Offset of field within structure; only used for nonstatic fields
  void *address;          // Address of field; only used for static fields
                          // ("offset" can not be reused because of apparent solstudio compiler bug
                          // in generation of initializer data)
} _VMStructEntry;

typedef struct {
  bool is_hooked;
  uintptr_t *orig_i2i_entry;
  uintptr_t *orig_fi_entry;
  uintptr_t *orig_fc_entry;
} java_hook_t;

bool java_hook_install(JNIEnv *env, jmethodID mid, void *detour);
