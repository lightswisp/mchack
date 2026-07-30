#include "../../rendering/gui.h"
#include "esp.h"
#include <unistd.h>

#include "glow.h"
#include "arrows.h"
#include "box.h"

void esp(JNIEnv *env, game_t *game, menu_state_t *menu_state, overlay_t *overlay) {
  if (!menu_state->esp_settings.enabled)
    return;
  if (!game->in_game)
    return;
  if (game->players.size < 2)
    return;

  glow(env, game, menu_state);
  arrows(env, game, menu_state, overlay);
  box(env, game, menu_state, overlay);
}