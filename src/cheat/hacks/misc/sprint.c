#include "sprint.h"

void sprint(JNIEnv *env, game_t *game, bool on) {
  if (!on)
    return;

  bool is_moving = (*env)->CallBooleanMethod(env, game->localplayer.instance, game->localplayer.is_moving);
  if (is_moving)
    (*env)->CallNonvirtualVoidMethod(env, game->localplayer.instance, game->localplayer.super,
                                     game->localplayer.set_sprinting, true);
}