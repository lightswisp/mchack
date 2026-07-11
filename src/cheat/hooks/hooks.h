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
  size_t stolen_bytes;
} hook_t;

bool hook_install(void *addr, void *detour, void **trampoline);
bool hook_remove(void *addr);