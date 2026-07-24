#include "fullbright.h"
#include "../../utils/logger.h"

void fullbright(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  if (!menu_state->misc_settings.fullbright.enabled) {
    if (menu_state->misc_settings.fullbright.state == ACTIVE) {
      (*env)->SetDoubleField(env, game->minecraft.options.instance, game->minecraft.options.gamma, 1.0);
      menu_state->misc_settings.fullbright.state = IDLE;
    }
    return;
  }

  (*env)->SetDoubleField(env, game->minecraft.options.instance, game->minecraft.options.gamma, 90.0);
  menu_state->misc_settings.fullbright.state = ACTIVE;
}