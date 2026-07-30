#include "camera.h"

void camera_get_position(JNIEnv *env, game_t *game, vec3_t *out) {
  float x =
      (*env)->GetDoubleField(env, game->minecraft.game_renderer.camera.position, game->collections.vec3.x);
  float y =
      (*env)->GetDoubleField(env, game->minecraft.game_renderer.camera.position, game->collections.vec3.y);
  float z =
      (*env)->GetDoubleField(env, game->minecraft.game_renderer.camera.position, game->collections.vec3.z);
  out->x = x;
  out->y = y;
  out->z = z;
}

void camera_get_viewangles(JNIEnv *env, game_t *game, vec2_t *out) {

  float x_rot = (*env)->GetFloatField(env, game->minecraft.game_renderer.camera.instance,
                                      game->minecraft.game_renderer.camera.x_rot);
  float y_rot = (*env)->GetFloatField(env, game->minecraft.game_renderer.camera.instance,
                                      game->minecraft.game_renderer.camera.y_rot);
  out->x = x_rot;
  out->y = y_rot;
}

void camera_get_viewangles2(JNIEnv *env, game_t *game, vec2_t *out) {

  float x_rot = (*env)->GetFloatField(env, game->minecraft.game_renderer.camera.instance,
                                      game->minecraft.game_renderer.camera.x_rot);
  float y_rot = (*env)->GetFloatField(env, game->minecraft.game_renderer.camera.instance,
                                      game->minecraft.game_renderer.camera.y_rot);
  out->x = deg2rad(x_rot);
  out->y = deg2rad(y_rot);
}