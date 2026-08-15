#include "health.h"
#include "../../core/player.h"
#include "../../core/camera.h"

void health(game_t *game) {
  if (!game->menu->esp_settings.health_bar.enabled)
    return;

  vec2_t camera_angles;
  vec3_t camera_position;
  vec3_t player_position;
  vec3_t screen;

  size_t size = 0;
  jint max_health_boxes = clampi(game->players.size, 0, arr_size(game->esp->health.buffer));
  for (jint i = 0; i < max_health_boxes; i++) {
    jobject player = (*game->env)->CallObjectMethod(game->env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(game->env, game, player))
      continue;

    float health = player_get_health(game->env, game, player);
    if (health <= 0)
      continue;

    float fov = game->minecraft.options.fov_val * game->game_renderer.fov_val;
    camera_get_position(game->env, game, &camera_position);
    camera_get_viewangles2(game->env, game, &camera_angles);
    player_get_lerped_position(game->env, game, player, &player_position);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)game->rctx->width,
             (float)game->rctx->height, fov))
      continue;

    game->esp->health.buffer[size].feet.x = screen.x;
    game->esp->health.buffer[size].feet.y = screen.y;
    game->esp->health.buffer[size].feet.z = screen.z;

    player_position.y += player_get_height(game->env, game, player);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)game->rctx->width,
             (float)game->rctx->height, fov))
      continue;

    game->esp->health.buffer[size].head.x = screen.x;
    game->esp->health.buffer[size].head.y = screen.y;
    game->esp->health.buffer[size].head.z = screen.z;

    game->esp->health.buffer[size].health = health;
    size++;
  }
  game->esp->health.size = size;
}