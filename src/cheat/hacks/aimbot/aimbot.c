#include "../../rendering/gui.h"
#include "aimbot.h"

void aimbot(JNIEnv *env, game_t *game) {

  menu_state_t *menu_state = get_menu_state();
  if (!menu_state->esp_settings.enabled)
    return;
  if (!game->in_game)
    return;
}