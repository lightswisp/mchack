#include "fly.h"
#include "../../core/localplayer.h"

void fly(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  if (!menu_state->misc_settings.fly.enabled) {
    if (menu_state->misc_settings.fly.state == ACTIVE) {
      localplayer_set_mayfly(env, game, false);
      menu_state->misc_settings.fly.state = IDLE;
    }
    return;
  }

  localplayer_set_mayfly(env, game, true);
  menu_state->misc_settings.fly.state = ACTIVE;
}