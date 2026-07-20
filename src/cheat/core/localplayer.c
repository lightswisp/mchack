#include "localplayer.h"
#include "../jvm.h"
#include "../game.h"

jobject localplayer_get(JNIEnv *env, game_t *game) {

  jfieldID player_field =
      (*env)->GetFieldID(env, game->minecraft.class, "player", "Lnet/minecraft/client/player/LocalPlayer;");
  if (player_field == NULL)
    return NULL;
  jobject local_player = (*env)->GetObjectField(env, game->minecraft.instance, player_field);

  if (local_player == NULL)
    return NULL;

  return local_player;
}

void localplayer_get_position(jobject player, vec3_t *out) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  float x = (*env)->CallDoubleMethod(env, player, game->localplayer.get_x);
  float y = (*env)->CallDoubleMethod(env, player, game->localplayer.get_y);
  float z = (*env)->CallDoubleMethod(env, player, game->localplayer.get_z);
  out->x = x;
  out->y = y;
  out->z = z;
}

void localplayer_get_viewangles(jobject player, vec2_t *out) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  out->x = (*env)->GetFloatField(env, player, game->localplayer.x_rot);
  out->y = (*env)->GetFloatField(env, player, game->localplayer.y_rot);
}

void localplayer_set_viewangles(jobject player, vec2_t *angles) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  (*env)->SetFloatField(env, player, game->localplayer.x_rot, angles->x);
  (*env)->SetFloatField(env, player, game->localplayer.y_rot, angles->y);
}