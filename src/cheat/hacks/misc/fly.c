#include "fly.h"

void fly(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  if (!menu_state->misc_settings.fly.enabled) {
    if (menu_state->misc_settings.fly.state == ACTIVE) {
      (*env)->SetBooleanField(env, game->localplayer.abilities.instance, game->localplayer.abilities.mayfly,
                              false);
      menu_state->misc_settings.fly.state = IDLE;
    }
    return;
  }

  (*env)->SetBooleanField(env, game->localplayer.abilities.instance, game->localplayer.abilities.mayfly,
                          true);
  menu_state->misc_settings.fly.state = ACTIVE;
}