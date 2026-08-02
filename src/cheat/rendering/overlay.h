#pragma once
#include "draw.h"
#include "../utils/math.h"

typedef struct {
  struct {
    float buffer[255];
    size_t size;
  } arrows;

  struct {
    struct {
      vec3_t head;
      vec3_t feet;
    } buffer[255];
    size_t size;
  } boxes;

  struct {
    struct {
      float health;
      vec3_t head;
      vec3_t feet;
    } buffer[255];
    size_t size;
  } health;
} overlay_t;

void draw_overlay(rendering_context_t *rctx);
overlay_t *get_overlay(void);