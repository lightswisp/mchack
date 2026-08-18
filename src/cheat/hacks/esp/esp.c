#include "../../core/camera.h"
#include "../../core/player.h"
#include "esp.h"

#include "glow.h"
#include "arrows.h"
#include "box.h"
#include "health.h"
#include "snap.h"

#include <unistd.h>
#include <float.h>

static void esp_core(game_t *game) {
  size_t size = 0;
  jint max_calculations = clampi(game->players.size, 0, MAX_ENTITIES);

  for (jint i = 0; i < max_calculations; i++) {
    vec_bb_t player_bb_position = {0};
    float health = 0.0f;

    jobject player = (*game->env)->CallObjectMethod(game->env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(game->env, game, player))
      continue;
    if ((health = player_get_health(game->env, game, player)) <= 0)
      continue;

    float fov = game->minecraft.options.fov_val * game->game_renderer.fov_val;
    camera_get_position(game->env, game, &game->esp->core.local.camera_position);
    camera_get_viewangles_rad(game->env, game, &game->esp->core.local.camera_angles_rad);
    camera_get_viewangles_deg(game->env, game, &game->esp->core.local.camera_angles_deg);
    player_get_bb(game->env, game, player, &player_bb_position);

    float bounds[BOUNDS_SIZE] = {
        FLT_MAX, // top
        FLT_MIN, // right
        FLT_MIN, // bottom
        FLT_MAX, // left
    };

    vec3_t vertices[MAX_BB_VERTICIES] = {
        {player_bb_position.min.x, player_bb_position.min.y, player_bb_position.min.z},
        {player_bb_position.min.x, player_bb_position.min.y, player_bb_position.max.z},
        {player_bb_position.min.x, player_bb_position.max.y, player_bb_position.min.z},
        {player_bb_position.min.x, player_bb_position.max.y, player_bb_position.max.z},
        {player_bb_position.max.x, player_bb_position.min.y, player_bb_position.min.z},
        {player_bb_position.max.x, player_bb_position.min.y, player_bb_position.max.z},
        {player_bb_position.max.x, player_bb_position.max.y, player_bb_position.min.z},
        {player_bb_position.max.x, player_bb_position.max.y, player_bb_position.max.z},
    };

    vec3_t player_bottom = {
        player_bb_position.min.x + (player_bb_position.max.x - player_bb_position.min.x) * 0.5f,
        player_bb_position.min.y,
        player_bb_position.min.z + (player_bb_position.max.z - player_bb_position.min.z) * 0.5f,
    };
    if (!w2s(&player_bottom, &game->esp->core.local.camera_position, &game->esp->core.local.camera_angles_rad,
             &game->esp->core.players.buffer[size].bottom, (float)game->rctx->width, (float)game->rctx->height, fov)) {
      goto end;
    }

    for (size_t j = 0; j < MAX_BB_VERTICIES; j++) {
      if (!w2s(&vertices[j], &game->esp->core.local.camera_position, &game->esp->core.local.camera_angles_rad,
               &game->esp->core.players.buffer[size].vertices[j], (float)game->rctx->width, (float)game->rctx->height,
               fov)) {
        goto end;
      }
      bounds[BOUND_TOP] = fmin(game->esp->core.players.buffer[size].vertices[j].y, bounds[BOUND_TOP]);
      bounds[BOUND_RIGHT] = fmax(game->esp->core.players.buffer[size].vertices[j].x, bounds[BOUND_RIGHT]);
      bounds[BOUND_BOTTOM] = fmax(game->esp->core.players.buffer[size].vertices[j].y, bounds[BOUND_BOTTOM]);
      bounds[BOUND_LEFT] = fmin(game->esp->core.players.buffer[size].vertices[j].x, bounds[BOUND_LEFT]);
    }

    game->esp->core.players.buffer[size].bounds[BOUND_TOP] = bounds[BOUND_TOP];
    game->esp->core.players.buffer[size].bounds[BOUND_RIGHT] = bounds[BOUND_RIGHT];
    game->esp->core.players.buffer[size].bounds[BOUND_BOTTOM] = bounds[BOUND_BOTTOM];
    game->esp->core.players.buffer[size].bounds[BOUND_LEFT] = bounds[BOUND_LEFT];

    game->esp->core.players.buffer[size].health = health;

    size++;

  end:
    // nested break
  }
  game->esp->core.players.size = size;
}

void esp(game_t *game) {
  if (!game->menu->esp_settings.enabled)
    return;
  if (!game->in_game)
    return;
  if (game->players.size < 2)
    return;

  esp_core(game);

  glow(game);
  arrows(game);
  box(game);
  health(game);
  snap(game);
}