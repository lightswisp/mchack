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

// void player_get_bb(JNIEnv *env, game_t *game, jobject player, vec_bb_t *out) {
//   jobject bb = (*env)->GetObjectField(env, player, game->players.bounding_box);
//   float min_x = (*env)->GetDoubleField(env, bb, game->bounding_box.min_x);
//   float max_x = (*env)->GetDoubleField(env, bb, game->bounding_box.max_x);
//
//   float min_y = (*env)->GetDoubleField(env, bb, game->bounding_box.min_y);
//   float max_y = (*env)->GetDoubleField(env, bb, game->bounding_box.max_y);
//
//   float min_z = (*env)->GetDoubleField(env, bb, game->bounding_box.min_z);
//   float max_z = (*env)->GetDoubleField(env, bb, game->bounding_box.max_z);
//
//   out->min.x = min_x;
//   out->min.y = min_y;
//   out->min.z = min_z;
//
//   out->max.x = max_x;
//   out->max.y = max_y;
//   out->max.z = max_z;
// }

void player_get_bb(JNIEnv *env, game_t *game, jobject player, vec_bb_t *out) {
  float x = (*env)->CallDoubleMethod(env, player, game->players.get_x);
  float y = (*env)->CallDoubleMethod(env, player, game->players.get_y);
  float z = (*env)->CallDoubleMethod(env, player, game->players.get_z);
  float x_old = (*env)->GetDoubleField(env, player, game->players.x_old);
  float y_old = (*env)->GetDoubleField(env, player, game->players.y_old);
  float z_old = (*env)->GetDoubleField(env, player, game->players.z_old);

  float half_width = player_get_width(env, game, player) / 2.0f;
  float height = player_get_height(env, game, player);

  float x_old_min = x_old - half_width;
  float x_min = x - half_width;

  float z_old_min = z_old - half_width;
  float z_min = z - half_width;

  float x_old_max = x_old + half_width;
  float x_max = x + half_width;

  float y_old_max = y_old + height;
  float y_max = y + height;

  float z_old_max = z_old + half_width;
  float z_max = z + half_width;

  out->min.x = x_old_min + (x_min - x_old_min) * game->minecraft.partial_tick;
  out->min.y = y_old + (y - y_old) * game->minecraft.partial_tick;
  out->min.z = z_old_min + (z_min - z_old_min) * game->minecraft.partial_tick;

  out->max.x = x_old_max + (x_max - x_old_max) * game->minecraft.partial_tick;
  out->max.y = y_old_max + (y_max - y_old_max) * game->minecraft.partial_tick;
  out->max.z = z_old_max + (z_max - z_old_max) * game->minecraft.partial_tick;
}

void player_set_team_color(JNIEnv *env, game_t *game, jobject player, int color) {
  jobject team = (*env)->CallObjectMethod(env, player, game->players.get_team);
  jobject chat_formatting = (*env)->CallObjectMethod(env, team, game->team.get_color);

  jobject int_obj = (*env)->NewObject(env, game->collections.integer.class, game->collections.integer.init, color);

  (*env)->SetObjectField(env, chat_formatting, game->chat_formatting.color, int_obj);

  (*env)->DeleteLocalRef(env, int_obj);
  (*env)->DeleteLocalRef(env, chat_formatting);
  (*env)->DeleteLocalRef(env, team);
}

int player_get_team_color(JNIEnv *env, game_t *game, jobject player) {
  return (*env)->CallIntMethod(env, player, game->players.get_team_color);
}

void player_set_glowing(JNIEnv *env, game_t *game, jobject player, bool val) {
  (*env)->SetBooleanField(env, player, game->players.glowing, val);
}