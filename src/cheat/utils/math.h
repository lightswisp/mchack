#pragma once
#include <math.h>
#include <stddef.h>

#define TAU 6.283185307179586F
#define arr_size(a) (sizeof(a) / sizeof(a[0]))

typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  float x, y;
} vec2_t;

typedef struct {
  vec3_t min;
  vec3_t max;
} vec_bb_t;

typedef struct {
  size_t left;
  size_t right;
} node_t;

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

static inline float normalize_angle2(float a) {
  a = fmodf(a + M_PI, TAU);
  if (a < 0.0)
    a += TAU;
  return a - M_PI;
}

static inline float normalize_angle360(float a) { return a >= 0.0f ? a : a + 360.0f; }

static inline float clampf(float a, float min, float max) {
  if (a < min)
    return min;
  if (a > max)
    return max;
  return a;
}

static inline float clampfd(float a, float min, float max, float def) {
  if (a < min)
    return min;
  if (a > max)
    return max;
  return def;
}

static inline void get_neighbours(int i, int m, node_t *out) {
  if (i == 0) {
    out->left = i + 1;
    out->right = m - 1;
  } else if (i == m - 1) {
    out->left = 0;
    out->right = i - 1;
  } else {
    out->left = i + 1;
    out->right = i - 1;
  }
}

static inline int clampi(int a, int min, int max) {
  if (a < min)
    return min;
  if (a > max)
    return max;
  return a;
}

static inline void calculate_delta_angle(vec2_t *from, vec2_t *to, vec2_t *out) {
  out->x = clampf(to->x - from->x, -90.0f, 90.0f);
  out->y = normalize_angle(to->y - from->y);
}

static inline void calculate_delta_pos_vec3(vec3_t *a, vec3_t *b, vec3_t *out) {
  out->x = a->x - b->x;
  out->y = a->y - b->y;
  out->z = a->z - b->z;
}

static inline float minf(float a, float b) {
  if (a < b)
    return a;
  return b;
}

static inline float maxf(float a, float b) {
  if (a > b)
    return a;
  return b;
}

static inline bool within_range(float value, float from, float to) {
  if (value >= minf(to, from) && value <= maxf(to, from))
    return true;
  return false;
}

static inline bool w2s(vec3_t *entity, vec3_t *camera, /* must be in radians */ vec2_t *camera_angles, vec3_t *out,
                       float width, float height, float fov) {
  float fovy_rad = deg2rad(fov);
  float dx = entity->x - camera->x;
  float dy = entity->y - camera->y;
  float dz = entity->z - camera->z;

  float rad_yaw = camera_angles->y;
  float rad_pitch = camera_angles->x;

  float sin_yaw = sinf(rad_yaw);
  float cos_yaw = cosf(rad_yaw);
  float sin_pitch = sinf(rad_pitch);
  float cos_pitch = cosf(rad_pitch);

  float x_cam = -dx * cos_yaw - dz * sin_yaw;
  float y_tmp = -dx * sin_yaw + dz * cos_yaw;

  float y_cam = dy * cos_pitch + y_tmp * sin_pitch;
  float z_cam = y_tmp * cos_pitch - dy * sin_pitch;

  if (z_cam <= 0.1f) {
    return false;
  }

  float aspect_ratio = width / height;
  float tan_half_fovy = tanf(fovy_rad * 0.5f);

  float x_ndc = x_cam / (z_cam * tan_half_fovy * aspect_ratio);
  float y_ndc = y_cam / (z_cam * tan_half_fovy);

  float width_half = width * 0.5f;
  float height_half = height * 0.5f;

  out->x = width_half + (x_ndc * width_half);
  out->y = height_half - (y_ndc * height_half);
  out->z = z_cam;

  if (isnan(out->x) || isnan(out->y) || isinf(out->x) || isinf(out->y)) {
    return false;
  }

  return true;

  //  float width_half = (float)width / 2.0;
  //  float height_half = (float)height / 2.0;
  //
  //  // fov argument is a vertical fov, we also need to calculate the horizontal fov
  //  float fovy_rad = deg2rad(fov);
  //  float d = height_half / tanf(fovy_rad / 2);
  //  float fovx_rad = atanf(width_half / d) * 2.0;
  //
  //  float distance = calculate_distance_vec3(entity, local);
  //  float delta_x = entity->x - local->x;
  //  float delta_y = entity->y - local->y;
  //  float delta_z = entity->z - local->z;
  //
  //  float pitch = -asinf(delta_y / distance);
  //  float yaw = -atan2f(delta_x, delta_z);
  //
  //  float pitch_delta = pitch - local_angles->x;
  //  float yaw_delta = normalize_angle2(yaw - local_angles->y);
  //  // if it is out of our field of view, just skip
  //  if (fabs(yaw_delta) > fovx_rad / 2.0)
  //    return false;
  //
  //  // X
  //  float xz_distance = sqrtf(delta_x * delta_x + delta_z * delta_z);
  //  float dz = sinf(yaw_delta) * xz_distance;
  //  float dx = cosf(yaw_delta) * xz_distance;
  //  float dfx = tanf(fovx_rad / 2) * dx;
  //  float x = width_half + ((dz / dfx) * width_half);
  //
  //  // Y
  //  float dy = sinf(pitch_delta) * distance;
  //  float dd = cosf(pitch_delta) * distance;
  //  float dfy = tanf(fovy_rad / 2) * dd;
  //  float y = height_half + ((dy / dfy) * height_half);
  //
  //  // out of screen checks
  //  if (x < 0 || x > width || y < 0 || y > height || isnan(x) || isnan(y)) {
  //    return false;
  //  }
  //
  //  out->x = x;
  //  out->y = y;
  //
  //  return true;
}
