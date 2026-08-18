#include "health.h"
#include "../../core/player.h"
#include "../../core/camera.h"

void health(game_t *game) {
  if (!game->menu->esp_settings.health_bar.enabled)
    return;

#define MAX_HEALTH (20.0f)

  game->esp->health.size = game->esp->core.players.size;
  for (size_t i = 0; i < game->esp->core.players.size; i++) {

    float height = (game->esp->core.players.buffer[i].bounds[BOUND_BOTTOM] - game->esp->core.players.buffer[i].bounds[BOUND_TOP]);
    // width of health box (4%)
    float health_width = height * 0.04;

    // x offset
    float x = game->esp->core.players.buffer[i].bounds[BOUND_LEFT] - (health_width * 2);
    float y = game->esp->core.players.buffer[i].bounds[BOUND_TOP];
    float t = 2.5;

    float health_ratio = game->esp->core.players.buffer[i].health / MAX_HEALTH;

    // it will decrease with the ratio
    float health_height = height * health_ratio;

    struct nk_color health_color =
        nk_rgb((222.0 - (health_ratio * 222.0) + 33.0), 242 * health_ratio, 109 * health_ratio);

    // inner bar
    struct nk_rect inner_rect = {x, y + (height - health_height), health_width, health_height};

    // outline
    struct nk_rect outer_rect = {x - t, y - t, health_width + 2 * t, height + 2 * t};

    game->esp->health.buffer[i].inner_rect = inner_rect;
    game->esp->health.buffer[i].outer_rect = outer_rect;
    game->esp->health.buffer[i].color = health_color;
  }
}