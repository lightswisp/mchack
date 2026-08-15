#include "localplayer.h"
#include "../jvm.h"
#include "../game.h"
#include "../utils/logger.h"

jobject localplayer_get(JNIEnv *env, game_t *game) {
  jobject local_player = (*env)->GetObjectField(env, game->minecraft.instance, game->minecraft.localplayer);

  if (local_player == NULL)
    return NULL;

  return local_player;
}

void localplayer_get_position(JNIEnv *env, game_t *game, vec3_t *out) {
  float x = (*env)->CallDoubleMethod(env, game->localplayer.instance, game->localplayer.get_x);
  float y = (*env)->CallDoubleMethod(env, game->localplayer.instance, game->localplayer.get_y);
  float z = (*env)->CallDoubleMethod(env, game->localplayer.instance, game->localplayer.get_z);
  out->x = x;
  out->y = y;
  out->z = z;
}

void localplayer_get_lerped_position(JNIEnv *env, game_t *game, vec3_t *out) {
  float x = (*env)->CallDoubleMethod(env, game->localplayer.instance, game->localplayer.get_x);
  float y = (*env)->CallDoubleMethod(env, game->localplayer.instance, game->localplayer.get_y);
  float z = (*env)->CallDoubleMethod(env, game->localplayer.instance, game->localplayer.get_z);
  float x_old = (*env)->GetDoubleField(env, game->localplayer.instance, game->localplayer.x_old);
  float y_old = (*env)->GetDoubleField(env, game->localplayer.instance, game->localplayer.y_old);
  float z_old = (*env)->GetDoubleField(env, game->localplayer.instance, game->localplayer.z_old);
  // old + .... dst * partial tick
  // where partial tick is a value from 0..1 because the integer part is substracted
  out->x = x_old + (x - x_old) * game->minecraft.partial_tick;
  out->y = y_old + (y - y_old) * game->minecraft.partial_tick;
  out->z = z_old + (z - z_old) * game->minecraft.partial_tick;
}

void localplayer_set_position(JNIEnv *env, game_t *game, vec3_t *in) {
  (*env)->CallVoidMethod(env, game->localplayer.instance, game->localplayer.set_pos_raw, in->x, in->y, in->z);
}

void localplayer_get_viewangles(JNIEnv *env, game_t *game, vec2_t *out) {
  out->x = (*env)->GetFloatField(env, game->localplayer.instance, game->localplayer.x_rot);
  out->y = (*env)->GetFloatField(env, game->localplayer.instance, game->localplayer.y_rot);
}

void localplayer_get_viewangles_rad(JNIEnv *env, game_t *game, vec2_t *out) {
  out->x = deg2rad((*env)->GetFloatField(env, game->localplayer.instance, game->localplayer.x_rot));
  out->y = deg2rad((*env)->GetFloatField(env, game->localplayer.instance, game->localplayer.y_rot));
}

void localplayer_set_viewangles(JNIEnv *env, game_t *game, vec2_t *in) {
  (*env)->SetFloatField(env, game->localplayer.instance, game->localplayer.x_rot, in->x);
  (*env)->SetFloatField(env, game->localplayer.instance, game->localplayer.y_rot, in->y);
}

void localplayer_set_lerped_viewangles(JNIEnv *env, game_t *game, vec2_t *in) {
  float x_rot_old = (*env)->GetFloatField(env, game->localplayer.instance, game->localplayer.x_rot_old);
  float y_rot_old = (*env)->GetFloatField(env, game->localplayer.instance, game->localplayer.y_rot_old);
  float x_lerped = x_rot_old + (in->x - x_rot_old) * game->minecraft.partial_tick;
  float y_lerped = y_rot_old + (in->y - y_rot_old) * game->minecraft.partial_tick;
  (*env)->SetFloatField(env, game->localplayer.instance, game->localplayer.x_rot, x_lerped);
  (*env)->SetFloatField(env, game->localplayer.instance, game->localplayer.y_rot, y_lerped);
}

void localplayer_get_delta_movement(JNIEnv *env, game_t *game, vec3_t *out) {

  jobject delta_movement = (*env)->GetObjectField(env, game->localplayer.instance, game->localplayer.delta_movement);
  float x = (*env)->GetDoubleField(env, delta_movement, game->collections.vec3.x);
  float y = (*env)->GetDoubleField(env, delta_movement, game->collections.vec3.y);
  float z = (*env)->GetDoubleField(env, delta_movement, game->collections.vec3.z);
  out->x = x;
  out->y = y;
  out->z = z;
  (*env)->DeleteLocalRef(env, delta_movement);
}

void localplayer_set_delta_movement(JNIEnv *env, game_t *game, vec3_t *in) {
  jobject delta_movement = (*env)->GetObjectField(env, game->localplayer.instance, game->localplayer.delta_movement);
  (*env)->SetDoubleField(env, delta_movement, game->collections.vec3.x, in->x);
  (*env)->SetDoubleField(env, delta_movement, game->collections.vec3.y, in->y);
  (*env)->SetDoubleField(env, delta_movement, game->collections.vec3.z, in->z);
  (*env)->DeleteLocalRef(env, delta_movement);
}

float localplayer_get_attack_strength(JNIEnv *env, game_t *game, float scale) {
  return (*env)->CallFloatMethod(env, game->localplayer.instance, game->localplayer.get_attack_strength_scale, scale);
}

void localplayer_set_on_ground(JNIEnv *env, game_t *game, bool value) {
  (*env)->SetBooleanField(env, game->localplayer.instance, game->localplayer.on_ground, value);
}

void localplayer_set_falling_distance(JNIEnv *env, game_t *game, float value) {
  (*env)->SetFloatField(env, game->localplayer.instance, game->localplayer.falling_distance, value);
}

float localplayer_get_height(JNIEnv *env, game_t *game) {
  return (*env)->CallFloatMethod(env, game->localplayer.instance, game->localplayer.get_height);
}

bool localplayer_is_using_item(JNIEnv *env, game_t *game) {
  return (*env)->GetBooleanField(env, game->localplayer.instance, game->localplayer.is_using_item);
}

bool localplayer_is_moving(JNIEnv *env, game_t *game) {
  return (*env)->CallBooleanMethod(env, game->localplayer.instance, game->localplayer.is_moving);
}

void localplayer_set_sprinting_time(JNIEnv *env, game_t *game, int val) {
  (*env)->SetIntField(env, game->localplayer.instance, game->localplayer.sprint_trigger_time, val);
}

void localplayer_set_mayfly(JNIEnv *env, game_t *game, bool val) {
  (*env)->SetBooleanField(env, game->localplayer.abilities.instance, game->localplayer.abilities.mayfly, val);
}

void localplayer_set_sprinting2(JNIEnv *env, jobject this, game_t *game) {
  (*env)->CallNonvirtualVoidMethod(env, this, game->localplayer.super, game->localplayer.set_sprinting, true);
}

void localplayer_set_mayfly2(JNIEnv *env, jobject this, game_t *game, bool val) {
  jobject abilities = (*env)->GetObjectField(env, this, game->localplayer.abilities_field);
  (*env)->SetBooleanField(env, abilities, game->localplayer.abilities.mayfly, val);
  (*env)->DeleteLocalRef(env, abilities);
}

void localplayer_set_on_ground2(JNIEnv *env, jobject this, game_t *game, bool value) {
  (*env)->SetBooleanField(env, this, game->localplayer.on_ground, value);
}

void localplayer_get_delta_movement2(JNIEnv *env, jobject this, game_t *game, vec3_t *out) {
  jobject delta_movement = (*env)->GetObjectField(env, this, game->localplayer.delta_movement);
  float x = (*env)->GetDoubleField(env, delta_movement, game->collections.vec3.x);
  float y = (*env)->GetDoubleField(env, delta_movement, game->collections.vec3.y);
  float z = (*env)->GetDoubleField(env, delta_movement, game->collections.vec3.z);
  out->x = x;
  out->y = y;
  out->z = z;
  (*env)->DeleteLocalRef(env, delta_movement);
}

void localplayer_set_delta_movement2(JNIEnv *env, jobject this, game_t *game, vec3_t *in) {
  jobject delta_movement = (*env)->GetObjectField(env, this, game->localplayer.delta_movement);
  (*env)->SetDoubleField(env, delta_movement, game->collections.vec3.x, in->x);
  (*env)->SetDoubleField(env, delta_movement, game->collections.vec3.y, in->y);
  (*env)->SetDoubleField(env, delta_movement, game->collections.vec3.z, in->z);
  (*env)->DeleteLocalRef(env, delta_movement);
}

void localplayer_get_viewangles_rad2(JNIEnv *env, jobject this, game_t *game, vec2_t *out) {
  out->x = deg2rad((*env)->GetFloatField(env, this, game->localplayer.x_rot));
  out->y = deg2rad((*env)->GetFloatField(env, this, game->localplayer.y_rot));
}

bool localplayer_get_on_ground2(JNIEnv *env, jobject this, game_t *game) {
  return (*env)->GetBooleanField(env, this, game->localplayer.on_ground);
}

void localplayer_set_impulse2(JNIEnv *env, jobject this, game_t *game, vec2_t *impulse) {
  jobject input = (*env)->GetObjectField(env, this, game->localplayer.input);
  (*env)->SetFloatField(env, input, game->input.forward_impulse, impulse->x);
  (*env)->SetFloatField(env, input, game->input.left_impulse, impulse->y);
}