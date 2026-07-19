#include "player.h"
#include "../utils/logger.h"
#include "../game.h"
#include "../jvm.h"

jstring player_get_name(jobject player) {}

bool player_is_localplayer(jobject player) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  bool is_local = (*env)->CallBooleanMethod(env, player, game->players.is_local_player);
  return is_local;
}

void player_get_position(jobject player, vec3_t *out) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  double x = (*env)->CallDoubleMethod(env, player, game->players.get_x);
  double y = (*env)->CallDoubleMethod(env, player, game->players.get_y);
  double z = (*env)->CallDoubleMethod(env, player, game->players.get_z);
  out->x = x;
  out->y = y;
  out->z = z;
}