#include "sprint.h"
#include "../../core/localplayer.h"
#include "../../core/keys.h"
#include "../../utils/logger.h"

void sprint(JNIEnv *env, game_t *game, bool on) {
  if (!on)
    return;

  localplayer_set_sprinting_time(env, game, 2);
  //(*env)->CallNonvirtualVoidMethod(env, game->localplayer.instance, game->localplayer.super,
  //                                  game->localplayer.set_sprinting, true);
}