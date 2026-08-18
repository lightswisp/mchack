#pragma once
#include "draw.h"
#include "menu.h"

#define MAX_ENTITIES (255)
#define MAX_BB_VERTICIES (8)
enum { BOUND_TOP, BOUND_RIGHT, BOUND_BOTTOM, BOUND_LEFT, BOUNDS_SIZE };

typedef struct {

  struct {

    struct {
      vec2_t camera_angles_deg;
      vec2_t camera_angles_rad;
      vec3_t camera_position;
    } local;

    struct {
      struct {
        // vertices are gonna be useful for 3d boxes
        vec3_t vertices[MAX_BB_VERTICIES];
        vec3_t bottom;
        float bounds[BOUNDS_SIZE];
        float health;
      } buffer[MAX_ENTITIES];
      size_t size;
    } players;
  } core;

  struct {
    struct {
      vec2_t t1[3];
      vec2_t t2[3];
    } buffer[MAX_ENTITIES];
    size_t size;
  } arrows;

  struct {
    struct {
      vec2_t from;
      vec2_t to;
    } buffer[MAX_ENTITIES];
    size_t size;
  } snap_lines;

  struct {
    struct {
      struct nk_rect rect;
    } buffer[MAX_ENTITIES];
    size_t size;
  } boxes;

  struct {
    struct {
      struct nk_color color;
      struct nk_rect inner_rect;
      struct nk_rect outer_rect;
    } buffer[MAX_ENTITIES];
    size_t size;
  } health;

} esp_t;

void draw_esp(rendering_context_t *rctx, menu_t *menu);
esp_t *get_esp(void);