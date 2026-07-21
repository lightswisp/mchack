#pragma once
#include <math.h>
typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  float x, y;
} vec2_t;

static inline float calculate_distance_vec3(vec3_t *a, vec3_t *b) {
  return sqrt((b->x - a->x) * (b->x - a->x) + (b->y - a->y) * (b->y - a->y) + (b->z - a->z) * (b->z - a->z));
}

static inline float rad2deg(float a) { return a * 180.0f / M_PI; }
static inline float deg2rad(float a) { return a * M_PI / 180.0f; }

static inline float normalize_angle(float a) {
  a = fmodf(a + 180.0, 360.0);
  if (a < 0.0)
    a += 360.0;
  return a - 180.0;
}