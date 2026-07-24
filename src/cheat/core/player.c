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
