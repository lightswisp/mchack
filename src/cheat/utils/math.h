#pragma once
#include <math.h>

#define arr_size(a) (sizeof(a) / sizeof(a[0]))

typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  float x, y;
} vec2_t;

static inline float calculate_distance_vec3(vec3_t *a, vec3_t *b) {
  return sqrt((b->x - a->x) * (b->x - a->x) + (b->y - a->y) * (b->y - a->y) + (b->z - a->z) * (b->z - a->z));
}

static inline void vec2add(vec2_t *a, vec2_t *b) {
  a->x += b->x;
  a->y += b->y;
}

static inline void s2p(vec2_t *a, float width, float height) {
  a->x = a->x - width / 2.0f;
  a->y = height / 2.0f - a->y;
}

static inline void p2s(vec2_t *a, float width, float height) {
  a->x = a->x + width / 2.0f;
  a->y = height / 2.0f - a->y;
}

static inline void transform_rotate(vec2_t *a, int width, int height, float angle) {
  s2p(a, width, height);
  float xt = a->x * cosf(angle) - a->y * sinf(angle);
  float yt = a->x * sinf(angle) + a->y * cosf(angle);
  a->x = xt;
  a->y = yt;
  p2s(a, width, height);
}

static inline void transform_scale(vec2_t *a, float factor) {
  a->x *= factor;
  a->y *= factor;
}

static inline void transform_add(vec2_t *a, float factor) {
  a->x += factor;
  a->y += factor;
}

static inline float rad2deg(float a) { return a * 180.0f / M_PI; }
static inline float deg2rad(float a) { return a * M_PI / 180.0f; }

static inline float normalize_angle(float a) {
  a = fmodf(a + 180.0, 360.0);
  if (a < 0.0)
    a += 360.0;
  return a - 180.0;
}

static inline float clamp(float a, float min, float max) {
  if (a < min)
    return min;
  if (a > max)
    return max;
  return a;
}