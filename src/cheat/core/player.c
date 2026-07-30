#include "player.h"
#include "../jvm.h"

jstring player_get_name([[maybe_unused]] jobject player) { return NULL; }

bool player_is_localplayer(JNIEnv *env, game_t *game, jobject player) {
  bool is_local = (*env)->CallBooleanMethod(env, player, game->players.is_local_player);
  return is_local;
}

void player_get_position(JNIEnv *env, game_t *game, jobject player, vec3_t *out) {
  float x = (*env)->CallDoubleMethod(env, player, game->players.get_x);
  float y = (*env)->CallDoubleMethod(env, player, game->players.get_y);
  float z = (*env)->CallDoubleMethod(env, player, game->players.get_z);
  out->x = x;
  out->y = y;
  out->z = z;
}

float player_get_health(JNIEnv *env, game_t *game, jobject player) {
  return (*env)->CallFloatMethod(env, player, game->players.get_health);
}

float player_get_eye_height(JNIEnv *env, game_t *game, jobject player) {
  return (*env)->GetFloatField(env, player, game->players.eye_height);
}

float player_get_height(JNIEnv *env, game_t *game, jobject player) {
  return (*env)->CallFloatMethod(env, player, game->players.get_height);
}

float player_get_width(JNIEnv *env, game_t *game, jobject player) {
  return (*env)->CallFloatMethod(env, player, game->players.get_width);
}

void player_get_lerped_position(JNIEnv *env, game_t *game, jobject player, vec3_t *out) {
  float x = (*env)->CallDoubleMethod(env, player, game->players.get_x);
  float y = (*env)->CallDoubleMethod(env, player, game->players.get_y);
  float z = (*env)->CallDoubleMethod(env, player, game->players.get_z);
  float x_old = (*env)->GetDoubleField(env, player, game->players.x_old);
  float y_old = (*env)->GetDoubleField(env, player, game->players.y_old);
  float z_old = (*env)->GetDoubleField(env, player, game->players.z_old);
  out->x = x_old + (x - x_old) * game->minecraft.partial_tick;
  out->y = y_old + (y - y_old) * game->minecraft.partial_tick;
  out->z = z_old + (z - z_old) * game->minecraft.partial_tick;
}