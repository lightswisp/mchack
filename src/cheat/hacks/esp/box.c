#include "box.h"
#include "../../core/player.h"
#include "../../core/camera.h"

void box(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  if (!menu_state->esp_settings.box.enabled)
    return;

  vec2_t camera_angles;
  vec3_t camera_position;
  vec3_t player_position;
  vec3_t screen;

  size_t size = 0;

  for (jint i = 0; i < game->players.size; i++) {
    jobject player = (*env)->CallObjectMethod(env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(env, game, player))
      continue;

    float fov = game->minecraft.options.fov_val * game->minecraft.game_renderer.fov_val;
    camera_get_position(env, game, &camera_position);
    camera_get_viewangles2(env, game, &camera_angles);
    player_get_lerped_position(env, game, player, &player_position);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)menu_state->rctx->width,
             (float)menu_state->rctx->height, fov))
      continue;

    menu_state->esp_settings.box.boxes[size].feet.x = screen.x;
    menu_state->esp_settings.box.boxes[size].feet.y = screen.y;
    menu_state->esp_settings.box.boxes[size].feet.z = screen.z;

    player_position.y += player_get_height(env, game, player);
    if (!w2s(&player_position, &camera_position, &camera_angles, &screen, (float)menu_state->rctx->width,
             (float)menu_state->rctx->height, fov))
      continue;

    menu_state->esp_settings.box.boxes[size].head.x = screen.x;
    menu_state->esp_settings.box.boxes[size].head.y = screen.y;
    menu_state->esp_settings.box.boxes[size].head.z = screen.z;
    size++;
  }
  menu_state->esp_settings.box.size = size;
}