#include <string.h>
#include "arrows.h"
#include "../../core/localplayer.h"
#include "../../core/player.h"
#include "../../utils/logger.h"
#include "../../core/camera.h"

void arrows(JNIEnv *env, game_t *game, menu_state_t *menu_state, overlay_t *overlay) {
  // it will traverse all players
  // then it will calculate the angle between our crosshair and the angle we need to turn to the enemy
  // this angle will be added to the menu_state->esp_settings.arrows2draw[i]
  // then an arrow will be drawn on the circle.
  // rotation matrix must be applied before altering x,y coordianates
  // X coordinate of an arrow will be calculated as     cos(angle) * R
  // Y coordinate of an arrow will be calculated as     sin(angle) * R
  // where R is the radius of our arrow circle
  if (!menu_state->esp_settings.arrows.enabled)
    return;

  vec3_t localplayer_position;
  vec3_t player_position;
  size_t arrows = 0;
  jint max_arrows = clampi(game->players.size, 0, arr_size(overlay->arrows.buffer));

  for (jint i = 0; i < max_arrows; i++) {
    jobject player = (*env)->CallObjectMethod(env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(env, game, player))
      continue;

    camera_get_position(env, game, &localplayer_position);
    player_get_lerped_position(env, game, player, &player_position);
    float distance = calculate_distance_vec3(&localplayer_position, &player_position);
    if (distance > menu_state->esp_settings.arrows.distance)
      continue;

    float delta_x = player_position.x - localplayer_position.x;
    float delta_z = player_position.z - localplayer_position.z;
    float yaw = rad2deg(-atan2f(delta_x, delta_z));

    vec2_t local_viewangles;
    camera_get_viewangles(env, game, &local_viewangles);
    local_viewangles.y = normalize_angle(local_viewangles.y);
    float delta_yaw = yaw - local_viewangles.y;
    delta_yaw = normalize_angle(delta_yaw);
    float arrow_angle = deg2rad(90.0f - delta_yaw);
    overlay->arrows.buffer[arrows] = arrow_angle;
    arrows++;
  }

  overlay->arrows.size = arrows;
}