#include "camera.h"

void camera_get_position(JNIEnv *env, game_t *game, vec3_t *out) {
  float x = (*env)->GetDoubleField(env, game->camera.position, game->collections.vec3.x);
  float y = (*env)->GetDoubleField(env, game->camera.position, game->collections.vec3.y);
  float z = (*env)->GetDoubleField(env, game->camera.position, game->collections.vec3.z);

  out->x = x;
  out->y = y;
  out->z = z;
}

void camera_get_viewangles_deg(JNIEnv *env, game_t *game, vec2_t *out) {
  float x_rot = (*env)->GetFloatField(env, game->camera.instance, game->camera.x_rot);
  float y_rot = (*env)->GetFloatField(env, game->camera.instance, game->camera.y_rot);

  out->x = x_rot;
  out->y = y_rot;
}

void camera_get_viewangles_rad(JNIEnv *env, game_t *game, vec2_t *out) {
  float x_rot = (*env)->GetFloatField(env, game->camera.instance, game->camera.x_rot);
  float y_rot = (*env)->GetFloatField(env, game->camera.instance, game->camera.y_rot);

  out->x = deg2rad(x_rot);
  out->y = deg2rad(y_rot);
}

void camera_set_viewangles(JNIEnv *env, game_t *game, vec2_t *in) {
  (*env)->SetFloatField(env, game->camera.instance, game->camera.x_rot, in->x);
  (*env)->SetFloatField(env, game->camera.instance, game->camera.y_rot, in->y);
}