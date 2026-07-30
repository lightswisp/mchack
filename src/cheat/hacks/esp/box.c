#include "box.h"
#include "../../core/player.h"
#include "../../core/camera.h"
#include "../../utils/logger.h"

void box(JNIEnv *env, game_t *game, menu_state_t *menu_state, overlay_t *overlay) {
  if (!menu_state->esp_settings.box.enabled)
    return;

  vec2_t camera_angles;
  vec3_t camera_position;
  vec3_t player_position;
  vec3_t screen;

  size_t size = 0;
  jint max_boxes = clampi(game->players.size, 0, arr_size(overlay->boxes.buffer));

  for (jint i = 0; i < max_boxes; i++) {
    jobject player = (*env)->CallObjectMethod(env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(env, game, player))
      continue;
    if (player_get_health(env, game, player) <= 0)
      continue;

    LOGGER_LOG(LOG, "health: %f", player_get_health(env, game, player));
    float fov = game->minecraft.options.fov_val * game->minecraft.game_renderer.fov_val;
    camera_get_position(env, game, &camera_position);
    camera_get_viewangles2(env, game, &camera_angles);
    player_get_lerped_position(env, game, player, &player_position);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)menu_state->rctx->width,
             (float)menu_state->rctx->height, fov))
      continue;

    overlay->boxes.buffer[size].feet.x = screen.x;
    overlay->boxes.buffer[size].feet.y = screen.y;
    overlay->boxes.buffer[size].feet.z = screen.z;

    player_position.y += player_get_height(env, game, player);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)menu_state->rctx->width,
             (float)menu_state->rctx->height, fov))
      continue;

    overlay->boxes.buffer[size].head.x = screen.x;
    overlay->boxes.buffer[size].head.y = screen.y;
    overlay->boxes.buffer[size].head.z = screen.z;
    size++;
  }
  overlay->boxes.size = size;
}