#include "../../rendering/gui.h"
#include "../../core/player.h"
#include "../../utils/logger.h"
#include "../../core/localplayer.h"
#include "aimbot.h"

static jobject find_closest_player(JNIEnv *env, game_t *game, double *dist) {
  vec3_t player_position;
  vec3_t localplayer_position;
  jint players_num = game->players.size;
  double min_distance = 0.0f;
  jobject player = NULL;

  for (jint i = 0; i < players_num; i++) {
    jobject temp_player =
        (*env)->CallObjectMethod(env, game->players.instance, game->collections.list.get, i);
    if (player_is_localplayer(temp_player))
      continue;

    localplayer_get_position(game->localplayer.instance, &localplayer_position);
    player_get_position(temp_player, &player_position);
    double temp_distance = calculate_distance_vec3(&localplayer_position, &player_position);

    if (player == NULL) {
      // init
      min_distance = temp_distance;
      player = temp_player;
      continue;
    }
    if (temp_distance < min_distance) {
      min_distance = temp_distance;
      player = temp_player;
    }
  }

  *dist = min_distance;
  return player;
}

void aimbot(JNIEnv *env, game_t *game) {

  menu_state_t *menu_state = get_menu_state();
  if (!menu_state->aimbot_settings.enabled)
    return;
  if (!game->in_game)
    return;
  if (game->players.size < 2)
    return;

  vec3_t player_position;
  vec3_t localplayer_position;
  double distance = 0.0f;
  jobject closest_player = find_closest_player(env, game, &distance);

  if (distance > menu_state->aimbot_settings.distance)
    return;

  localplayer_get_position(game->localplayer.instance, &localplayer_position);
  player_get_position(closest_player, &player_position);

  double delta_x = player_position.x - localplayer_position.x;
  double delta_y = player_position.y - localplayer_position.y;
  double delta_z = player_position.z - localplayer_position.z;

  double pitch = rad2deg(-asin(delta_y / distance));
  double yaw = rad2deg(-atan2(delta_x, delta_z));
  vec2_t viewangles = {pitch, yaw};
  vec2_t local_viewangles;
  localplayer_get_viewangles(game->localplayer.instance, &local_viewangles);

  double delta_pitch = viewangles.x - local_viewangles.x;
  double delta_yaw = viewangles.y - local_viewangles.y;

  delta_yaw = normalize_angle(delta_yaw);

  local_viewangles.x += delta_pitch * menu_state->aimbot_settings.smoothness;
  local_viewangles.y += delta_yaw * menu_state->aimbot_settings.smoothness;

  localplayer_set_viewangles(game->localplayer.instance, &local_viewangles);
}