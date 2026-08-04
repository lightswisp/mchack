#pragma once
#include <stdint.h>
#include "../../jvm.h"

typedef uint8_t *__Method;

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
  const char *typeName;       // Type name (example: "Method")
  const char *superclassName; // Superclass name, or null if none (example: "oopDesc")
  int32_t isOopType;          // Does this type represent an oop typedef? (i.e., "Method*" or
                              // "Klass*", but NOT "Method")
  int32_t isIntegerType;      // Does this type represent an integer type (of arbitrary size)?
  int32_t isUnsigned;         // If so, is it unsigned?
  uint64_t size;              // Size, in bytes, of the type
} _VMTypeEntry;

enum {
  JVM_ACC_PUBLIC = 0x0001,
  JVM_ACC_PRIVATE = 0x0002,
  JVM_ACC_PROTECTED = 0x0004,
  JVM_ACC_STATIC = 0x0008,
  JVM_ACC_FINAL = 0x0010,
  JVM_ACC_SYNCHRONIZED = 0x0020,
  JVM_ACC_SUPER = 0x0020,
  JVM_ACC_IDENTITY = 0x0020,
  JVM_ACC_VOLATILE = 0x0040,
  JVM_ACC_BRIDGE = 0x0040,
  JVM_ACC_TRANSIENT = 0x0080,
  JVM_ACC_VARARGS = 0x0080,
  JVM_ACC_NATIVE = 0x0100,
  JVM_ACC_INTERFACE = 0x0200,
  JVM_ACC_ABSTRACT = 0x0400,
  JVM_ACC_STRICT = 0x0800,
  JVM_ACC_STRICT_INIT = 0x0800,
  JVM_ACC_SYNTHETIC = 0x1000,
  JVM_ACC_ANNOTATION = 0x2000,
  JVM_ACC_ENUM = 0x4000,
  JVM_ACC_MODULE = 0x8000
};

typedef struct {
  bool is_hooked;
  jmethodID method;
  jclass class;

  struct {
    __Method raw;
    jmethodID mid;
  } copy;

  struct {
    uintptr_t *i2i_entry;
    uintptr_t *fi_entry;
    uintptr_t *fc_entry;
    uintptr_t *code;
    uint16_t *access_flags;
  } pointers;

  struct {
    uintptr_t orig_i2i_entry;
    uintptr_t orig_fi_entry;
    uintptr_t orig_fc_entry;
    uintptr_t orig_code;
    uint16_t orig_access_flags;
  } backup;

} java_hook_t;

bool java_hook_install(JNIEnv *env, jclass class, jmethodID mid, JNINativeMethod *detour);
bool java_hook_remove(JNIEnv *env, jmethodID mid);
jmethodID java_get_copy(jmethodID mid);