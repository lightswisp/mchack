#include "box.h"
#include <float.h>
#include <string.h>

void box(game_t *game) {
  if (!game->menu->esp_settings.box.enabled)
    return;

  game->esp->boxes.size = game->esp->core.players.size;

  for (size_t i = 0; i < game->esp->core.players.size; i++) {
    struct nk_rect rect = {
        .x = game->esp->core.players.buffer[i].bounds[BOUND_LEFT],
        .y = game->esp->core.players.buffer[i].bounds[BOUND_TOP],
        .w = (game->esp->core.players.buffer[i].bounds[BOUND_RIGHT] - game->esp->core.players.buffer[i].bounds[BOUND_LEFT]),
        .h = (game->esp->core.players.buffer[i].bounds[BOUND_BOTTOM] - game->esp->core.players.buffer[i].bounds[BOUND_TOP])};
    game->esp->boxes.buffer[i].rect = rect;
  }
}