#include "esp.h"
#include <unistd.h>

#include "glow.h"
#include "arrows.h"
#include "box.h"
#include "health.h"

void esp(game_t *game) {
  if (!game->menu->esp_settings.enabled)
    return;
  if (!game->in_game)
    return;
  if (game->players.size < 2)
    return;

  glow(game);
  arrows(game);
  box(game);
  health(game);
}