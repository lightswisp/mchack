#pragma once
#include "draw.h"
#include "../utils/math.h"

#define TITLE_SIZE (128)
#define CHEAT_TITLE                                                                                                    \
  "mchack built on "__DATE__                                                                                           \
  " "__TIME__

typedef struct {
  struct {
    bool set;
    char buffer[TITLE_SIZE];
    size_t size;
    size_t iteration;
  } title;

} overlay_t;

void draw_overlay(rendering_context_t *rctx);
overlay_t *get_overlay(void);