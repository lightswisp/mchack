#pragma once
#include "draw.h"
#include "menu.h"

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

} esp_t;

void draw_esp(rendering_context_t *rctx, menu_t *menu);
esp_t *get_esp(void);