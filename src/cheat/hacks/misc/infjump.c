#include "infjump.h"
#include "../../core/localplayer.h"

void inf_jump(JNIEnv *env, game_t *game, bool on) {
  if (!on)
    return;

  localplayer_set_on_ground(env, game, true);
}