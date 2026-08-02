#include "health.h"
#include "../../core/player.h"
#include "../../core/camera.h"

void health(JNIEnv *env, game_t *game, menu_state_t *menu_state, overlay_t *overlay) {
  if (!menu_state->esp_settings.health_bar.enabled)
    return;

  vec2_t camera_angles;
  vec3_t camera_position;
  vec3_t player_position;
  vec3_t screen;

  size_t size = 0;
  jint max_health_boxes = clampi(game->players.size, 0, arr_size(overlay->health.buffer));
  for (jint i = 0; i < max_health_boxes; i++) {
    jobject player = (*env)->CallObjectMethod(env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(env, game, player))
      continue;

    float health = player_get_health(env, game, player);
    if (health <= 0)
      continue;

    float fov = game->minecraft.options.fov_val * game->game_renderer.fov_val;
    camera_get_position(env, game, &camera_position);
    camera_get_viewangles2(env, game, &camera_angles);
    player_get_lerped_position(env, game, player, &player_position);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)menu_state->rctx->width,
             (float)menu_state->rctx->height, fov))
      continue;

    overlay->health.buffer[size].feet.x = screen.x;
    overlay->health.buffer[size].feet.y = screen.y;
    overlay->health.buffer[size].feet.z = screen.z;

    player_position.y += player_get_height(env, game, player);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)menu_state->rctx->width,
             (float)menu_state->rctx->height, fov))
      continue;

    overlay->health.buffer[size].head.x = screen.x;
    overlay->health.buffer[size].head.y = screen.y;
    overlay->health.buffer[size].head.z = screen.z;

    overlay->health.buffer[size].health = health;
    size++;
  }
  overlay->health.size = size;
}