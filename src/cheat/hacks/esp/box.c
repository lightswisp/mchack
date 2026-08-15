#include "box.h"
#include "../../core/player.h"
#include "../../core/camera.h"
#include "../../utils/logger.h"

void box(game_t *game) {
  if (!game->menu->esp_settings.box.enabled)
    return;

  vec2_t camera_angles;
  vec3_t camera_position;
  vec3_t player_position;
  vec3_t screen;

  size_t size = 0;
  jint max_boxes = clampi(game->players.size, 0, arr_size(game->esp->boxes.buffer));

  for (jint i = 0; i < max_boxes; i++) {
    jobject player = (*game->env)->CallObjectMethod(game->env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(game->env, game, player))
      continue;
    if (player_get_health(game->env, game, player) <= 0)
      continue;

    float fov = game->minecraft.options.fov_val * game->game_renderer.fov_val;
    camera_get_position(game->env, game, &camera_position);
    camera_get_viewangles2(game->env, game, &camera_angles);
    player_get_lerped_position(game->env, game, player, &player_position);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)game->rctx->width,
             (float)game->rctx->height, fov))
      continue;

    game->esp->boxes.buffer[size].feet.x = screen.x;
    game->esp->boxes.buffer[size].feet.y = screen.y;
    game->esp->boxes.buffer[size].feet.z = screen.z;

    player_position.y += player_get_height(game->env, game, player);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)game->rctx->width,
             (float)game->rctx->height, fov))
      continue;

    game->esp->boxes.buffer[size].head.x = screen.x;
    game->esp->boxes.buffer[size].head.y = screen.y;
    game->esp->boxes.buffer[size].head.z = screen.z;
    size++;
  }
  game->esp->boxes.size = size;
}