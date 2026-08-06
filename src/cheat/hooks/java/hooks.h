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

typedef struct {
  const char *name; // Name of constant (example: "_thread_in_native")
  int32_t value;    // Value of constant
} _VMIntConstantEntry;

// taken from methodFlags.hpp
// for newer jdk versions
enum {
  HAS_MONITOR_BYTECODES = 1 << 0, /* Method contains monitorenter/monitorexit bytecodes */
  HAS_JSRS = 1 << 1,
  IS_OLD = 1 << 2,             /* RedefineClasses() has replaced this method */
  IS_OBSOLETE = 1 << 3,        /* RedefineClasses() has made method obsolete */
  IS_DELETED = 1 << 4,         /* RedefineClasses() has deleted this method */
  IS_PREFIXED_NATIVE = 1 << 5, /* JVMTI has prefixed this native method */
  MONITOR_MATCHING = 1 << 6,   /* True if we know that monitorenter/monitorexit bytecodes match */
  QUEUED_FOR_COMPILATION = 1 << 7,
  IS_NOT_C2_COMPILABLE = 1 << 8,
  IS_NOT_C1_COMPILABLE = 1 << 9,
  IS_NOT_C2_OSR_COMPILABLE = 1 << 10,
  FORCE_INLINE = 1 << 11, /* Annotations but also set/reset at runtime */
  DONT_INLINE = 1 << 12,
  HAS_LOOPS_FLAG = 1 << 13,      /* Method has loops */
  HAS_LOOPS_FLAG_INIT = 1 << 14, /* The loop flag has been initialized */
  ON_STACK_FLAG = 1 << 15,       /* RedefineClasses support to keep Metadata from being cleaned */
  HAS_SCALARIZED_ARGS = 1 << 16,
};

// taken from accessFlags.hpp
enum {
  // HotSpot-specific access flags
  // flags actually put in .class file
  JVM_ACC_WRITTEN_FLAGS = 0x00007FFF,

  // Method* flags
  JVM_ACC_MONITOR_MATCH = 0x10000000,         // True if we know that monitorenter/monitorexit bytecodes match
  JVM_ACC_HAS_MONITOR_BYTECODES = 0x20000000, // Method contains monitorenter/monitorexit bytecodes
  JVM_ACC_HAS_LOOPS = 0x40000000,             // Method has loops
  JVM_ACC_LOOPS_FLAG_INIT = (int)0x80000000,  // The loop flag has been initialized
  JVM_ACC_QUEUED = 0x01000000,                // Queued for compilation
  JVM_ACC_NOT_C2_COMPILABLE = 0x02000000,
  JVM_ACC_NOT_C1_COMPILABLE = 0x04000000,
  JVM_ACC_NOT_C2_OSR_COMPILABLE = 0x08000000,
  JVM_ACC_HAS_LINE_NUMBER_TABLE = 0x00100000,
  JVM_ACC_HAS_CHECKED_EXCEPTIONS = 0x00400000,
  JVM_ACC_HAS_JSRS = 0x00800000,
  JVM_ACC_IS_OLD = 0x00010000,             // RedefineClasses() has replaced this method
  JVM_ACC_IS_OBSOLETE = 0x00020000,        // RedefineClasses() has made method obsolete
  JVM_ACC_IS_PREFIXED_NATIVE = 0x00040000, // JVMTI has prefixed this native method
  JVM_ACC_ON_STACK = 0x00080000,           // RedefinedClasses() is used on the stack

  // Klass* flags
  JVM_ACC_HAS_MIRANDA_METHODS = 0x10000000,     // True if this class has miranda methods in it's vtable
  JVM_ACC_HAS_VANILLA_CONSTRUCTOR = 0x20000000, // True if klass has a vanilla default constructor
  JVM_ACC_HAS_FINALIZER = 0x40000000,           // True if klass has a non-empty finalize() method
  JVM_ACC_IS_CLONEABLE = (int)0x80000000,       // True if klass supports the Clonable interface
  JVM_ACC_HAS_FINAL_METHOD = 0x01000000,        // True if klass has final method

  // Klass* and Method* flags
  JVM_ACC_HAS_LOCAL_VARIABLE_TABLE = 0x00200000,

  JVM_ACC_PROMOTED_FLAGS = 0x00200000, // flags promoted from methods to the holding klass

  // field flags
  // Note: these flags must be defined in the low order 16 bits because
  // InstanceKlass only stores a ushort worth of information from the
  // AccessFlags value.
  // These bits must not conflict with any other field-related access flags
  // (e.g., ACC_ENUM).
  // Note that the class-related ACC_ANNOTATION bit conflicts with these flags.
  JVM_ACC_FIELD_ACCESS_WATCHED = 0x00002000,        // field access is watched by JVMTI
  JVM_ACC_FIELD_MODIFICATION_WATCHED = 0x00008000,  // field modification is watched by JVMTI
  JVM_ACC_FIELD_INTERNAL = 0x00000400,              // internal field, same as JVM_ACC_ABSTRACT
  JVM_ACC_FIELD_STABLE = 0x00000020,                // @Stable field, same as JVM_ACC_SYNCHRONIZED
  JVM_ACC_FIELD_HAS_GENERIC_SIGNATURE = 0x00000800, // field has generic signature

  JVM_ACC_FIELD_INTERNAL_FLAGS = JVM_ACC_FIELD_ACCESS_WATCHED | JVM_ACC_FIELD_MODIFICATION_WATCHED |
                                 JVM_ACC_FIELD_INTERNAL | JVM_ACC_FIELD_STABLE | JVM_ACC_FIELD_HAS_GENERIC_SIGNATURE,

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
  uintptr_t *i2i_entry;
  uintptr_t *fi_entry;
  uintptr_t *fc_entry;
  uint32_t *flags;
  uintptr_t *code;
} entries_t;

typedef struct {
  uintptr_t orig_i2i_entry;
  uintptr_t orig_fi_entry;
  uintptr_t orig_fc_entry;
  uint32_t orig_flags;
  uintptr_t orig_code;
} entries_values_t;

typedef struct {
  bool is_hooked;
  jmethodID method;

  uint8_t *i2i_stub;
  uint8_t *fi_stub;
  uint8_t *fc_stub;
  entries_t pointers;
  entries_values_t backup;

} java_hook_t;

bool java_hook_install(JNIEnv *env, jmethodID mid, JNINativeMethod *detour);
bool java_hook_remove(jmethodID mid);
jmethodID java_get_copy(jmethodID mid);

// used inside hook
void java_unhook(jmethodID mid);
void java_rehook(jmethodID mid);