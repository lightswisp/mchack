#include "infjump.h"

void inf_jump(JNIEnv *env, game_t *game, bool on) {
  if (!on)
    return;

  (*env)->SetBooleanField(env, game->localplayer.instance, game->localplayer.on_ground, true);
}