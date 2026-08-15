#include "fly.h"
#include "../../core/localplayer.h"

void fly(JNIEnv *env, jobject this, game_t *game) {
  if (!game->menu->misc_settings.fly.enabled) {
    if (game->menu->misc_settings.fly.state == ACTIVE) {
      localplayer_set_mayfly2(env, this, game, false);
      game->menu->misc_settings.fly.state = IDLE;
    }
    return;
  }

  localplayer_set_mayfly2(env, this, game, true);
  game->menu->misc_settings.fly.state = ACTIVE;
}