#pragma once
#include <stddef.h>
#include <stdint.h>

#define MAX_HOOKS 512

typedef struct {
  uint8_t *base;
  size_t size;
  size_t edge;
} trampoline_t;

typedef struct {
  void *addr;        // addr of func
  void **trampoline; // dynamically allocated trampoline
  uint8_t *stolen_bytes;
  size_t stolen_bytes_size;
} hook_t;

bool native_hook_install(void *addr, void *detour, void **trampoline);
bool native_hook_remove(void *addr);