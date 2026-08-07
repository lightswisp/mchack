#include "../../rendering/gui.h"
#include "../../core/player.h"
#include "../../core/localplayer.h"
#include "../../utils/logger.h"
#include "aimbot.h"
#include "trigger.h"

static jobject find_closest_player(JNIEnv *env, game_t *game, float *dist) {
  vec3_t player_position;
  vec3_t localplayer_position;
  jint players_num = game->players.size;
  float min_distance = 0.0f;
  jobject player = NULL;

  for (jint i = 0; i < players_num; i++) {
    jobject temp_player = (*env)->CallObjectMethod(env, game->players.instance, game->collections.list.get, i);
    if (!temp_player)
      continue;
    if (player_is_localplayer(env, game, temp_player))
      continue;

    localplayer_get_lerped_position(env, game, &localplayer_position);
    player_get_lerped_position(env, game, temp_player, &player_position);
    float temp_distance = calculate_distance_vec3(&localplayer_position, &player_position);

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

static void smooth(JNIEnv *env, game_t *game, vec2_t *delta_angles, vec2_t *local_viewangles, float smooth) {

  local_viewangles->x += delta_angles->x * smooth; // sin(TAU * 0.1 * step);
  local_viewangles->y += delta_angles->y * smooth; // sin(TAU * 0.1 * step);

  localplayer_set_viewangles(env, game, local_viewangles);
}

static void aim(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  if (!menu_state->aimbot_settings.enabled)
    return;

  static int prev_tick;
  vec3_t player_position;
  vec3_t localplayer_position;
  vec3_t delta_position;
  float distance = 0.0f;
  jobject closest_player = find_closest_player(env, game, &distance);

  if (distance > menu_state->aimbot_settings.distance)
    return;

  localplayer_get_lerped_position(env, game, &localplayer_position);
  player_get_lerped_position(env, game, closest_player, &player_position);

  // FEET(
  //     "feet", (vec3, entity)->{ return vec3; }),
  // EYES(
  //     "eyes",
  //     (vec32, entity2)->{ return new Vec3(vec32.x, vec32.y + ((double)entity2.getEyeHeight()), vec32.z);
  //     });

  calculate_delta_pos_vec3(&player_position, &localplayer_position, &delta_position);

  float pitch = rad2deg(-asinf(delta_position.y / distance));
  float yaw = rad2deg(-atan2f(delta_position.x, delta_position.z));
  vec2_t viewangles = {pitch, yaw};
  vec2_t local_viewangles;
  vec2_t delta_angles;
  localplayer_get_viewangles(env, game, &local_viewangles);
  calculate_delta_angle(&local_viewangles, &viewangles, &delta_angles);

  float fov = hypot(delta_angles.x, delta_angles.y);
  if (menu_state->aimbot_settings.fov != 0.0f && fov > menu_state->aimbot_settings.fov)
    return;

  smooth(env, game, &delta_angles, &local_viewangles, menu_state->aimbot_settings.smooth_step);
  prev_tick = game->localplayer.tick_count;
  LOGGER_LOG(LOG, "new tick: %d", prev_tick);

  // autoattack
  if (menu_state->aimbot_settings.auto_attack && distance <= MAX_ATTACK_DISTANCE)
    attack(env, game, menu_state);
}

void aimbot(JNIEnv *env, game_t *game, menu_state_t *menu_state) {

  if (!game->in_game)
    return;
  if (game->players.size < 2)
    return;

  aim(env, game, menu_state);
  trigger(env, game, menu_state);
}