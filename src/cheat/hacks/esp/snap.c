#include "snap.h"

void snap(game_t *game) {
  if (!game->menu->esp_settings.snap_lines.enabled)
    return;

  game->esp->snap_lines.size = game->esp->core.players.size;
  for (size_t i = 0; i < game->esp->core.players.size; i++) {

    game->esp->snap_lines.buffer[i].from.x = game->rctx->width / 2.0f;
    game->esp->snap_lines.buffer[i].from.y = game->rctx->height / 2.0f;

    game->esp->snap_lines.buffer[i].to.x = game->esp->core.players.buffer[i].bottom.x;
    game->esp->snap_lines.buffer[i].to.y = game->esp->core.players.buffer[i].bottom.y;
  }
}