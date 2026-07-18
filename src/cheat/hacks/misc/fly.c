#include "fly.h"
#include "../../core/localplayer.h"
#include "../../jvm.h"
#include "../../utils/logger.h"

void fly(JNIEnv *env, game_t *game, bool on) {
  bool current_setting =
      (*env)->GetBooleanField(env, game->localplayer.abilities.instance, game->localplayer.abilities.mayfly);
  if (on && !current_setting) {
    // if it's on but minecraft has changed the flag
    (*env)->SetBooleanField(env, game->localplayer.abilities.instance, game->localplayer.abilities.mayfly,
                            true);
  } else if (!on && current_setting) {
    // if it's off, we need to turn the flag as well
    (*env)->SetBooleanField(env, game->localplayer.abilities.instance, game->localplayer.abilities.mayfly,
                            false);
  }
}