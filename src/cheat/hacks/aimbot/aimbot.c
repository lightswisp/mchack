#include "../../core/player.h"
#include "../../core/camera.h"
#include "../../core/localplayer.h"
#include "../../utils/logger.h"
#include "aimbot.h"
#include "trigger.h"
#include "attack.h"
#include <float.h>

static jobject find_closest_player(game_t *game, float *dist) {
  vec3_t player_position;
  vec3_t localplayer_position;
  jint players_num = game->players.size;
  float min_distance = 0.0f;
  jobject player = NULL;

  for (jint i = 0; i < players_num; i++) {
    jobject temp_player =
        (*game->env)->CallObjectMethod(game->env, game->players.instance, game->collections.list.get, i);
    if (!temp_player)
      continue;
    if (player_is_localplayer(game->env, game, temp_player))
      continue;

    localplayer_get_lerped_position(game->env, game, &localplayer_position);
    player_get_lerped_position(game->env, game, temp_player, &player_position);
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

static float calc_smoothing_coefficient(float d, float dt, float epsilon) {
  float exp = dt / d;
  float c = 1.0f - powf(epsilon, exp);
  return c;
}

static void exp_smooth(game_t *game, vec2_t *delta, vec2_t *local, float smooth, float dt) {

  float c = calc_smoothing_coefficient(smooth, dt, 0.0001f);
  local->x += delta->x * c; // sin(TAU * 0.1 * step);
  local->y += delta->y * c; // sin(TAU * 0.1 * step);

  localplayer_set_viewangles(game->env, game, local);
}

static void aim(game_t *game) {
  if (!game->menu->aimbot_settings.enabled)
    return;

  if (game->players.size < 2)
    return;

  vec3_t player_position = {0};
  vec3_t localplayer_position = {0};
  vec2_t local_viewangles = {0};
  vec2_t viewangles[BONE_SIZE] = {0};
  vec2_t delta_angles[BONE_SIZE] = {0};
  vec_bb_t bb_position = {0};
  float distance = 0.0f;
  jobject closest_player = find_closest_player(game, &distance);
  if (distance > game->menu->aimbot_settings.distance)
    return;

  localplayer_get_viewangles(game->env, game, &local_viewangles);
  local_viewangles.y = normalize_angle(local_viewangles.y);
  localplayer_get_lerped_position(game->env, game, &localplayer_position);
  localplayer_position.y += localplayer_get_height(game->env, game);
  player_get_lerped_position(game->env, game, closest_player, &player_position);
  player_get_bb(game->env, game, closest_player, &bb_position);

#define MAX_EDGES (4)
  /*
             edge indexes
     Z
     |       1         2
     |        +-------+
     |        |       |
     |        |       |
     |        |       |
     |        +-------+
     |       0         3
     |
     +---------------------- X
  */
  vec3_t edges[MAX_EDGES] = {{bb_position.min.x, 0, bb_position.min.z},
                             {bb_position.min.x, 0, bb_position.max.z},
                             {bb_position.max.x, 0, bb_position.max.z},
                             {bb_position.max.x, 0, bb_position.min.z}};
  struct {
    size_t idx;
    float distance;
  } closest_edge = {0, FLT_MAX};
  vec2_t edges_delta_angles[MAX_EDGES];
  vec2_t edges_viewangles[MAX_EDGES];

  for (size_t i = 0; i < BONE_SIZE; i++) {
    vec3_t tmp_pos = player_position;
    vec3_t delta_position = {0};
    node_t neighbours;
    switch (i) {
    case BONE_FEET:
      // calculating delta angle to the feet
      tmp_pos.y += 0;
      calculate_delta_pos_vec3(&tmp_pos, &localplayer_position, &delta_position);
      viewangles[i].x = rad2deg(-atan2f(delta_position.y, hypot(delta_position.x, delta_position.z)));
      viewangles[i].y = rad2deg(-atan2f(delta_position.x, delta_position.z));
      calculate_delta_angle(&local_viewangles, &viewangles[i], &delta_angles[i]);
      break;
    case BONE_NEAREST:
      // calculating delta angles to all parts of the hitbox and then finding the nearest point
      // to our viewangles.
      // if we are already looking at the enemy - delta angles are at 0 degrees.

      // fisrt calculate edges
      for (size_t j = 0; j < MAX_EDGES; j++) {
        float edge_distance = calculate_distance_vec3(&edges[j], &localplayer_position);
        calculate_delta_pos_vec3(&edges[j], &localplayer_position, &delta_position);
        edges_viewangles[j].x = rad2deg(-atan2f(delta_position.y, hypot(delta_position.x, delta_position.z)));
        edges_viewangles[j].y = rad2deg(-atan2f(delta_position.x, delta_position.z));
        calculate_delta_angle(&local_viewangles, &edges_viewangles[j], &edges_delta_angles[j]);
        // LOGGER_LOG(LOG, "[%lu] angle: %f | delta: %f", j, edges_viewangles[j].y, edges_delta_angles[j].y);
        //  getting the min angle
        if (edge_distance < closest_edge.distance) {
          closest_edge.idx = j;
          closest_edge.distance = edge_distance;
        }
      }
      // get neighbours based on the closest edge
      get_neighbours(closest_edge.idx, MAX_EDGES, &neighbours);

      float left = normalize_angle360(edges_viewangles[neighbours.left].y);
      float center = normalize_angle360(edges_viewangles[closest_edge.idx].y);
      float right = normalize_angle360(edges_viewangles[neighbours.right].y);
      float local = normalize_angle360(local_viewangles.y);

#define MAX_NEIGHBOURS (3)
      float neighbours_delta_angles[MAX_NEIGHBOURS] = {edges_delta_angles[closest_edge.idx].y,
                                                       edges_delta_angles[neighbours.left].y,
                                                       edges_delta_angles[neighbours.right].y

      };

      // check if our yaw is within two ranges:
      // 1. (left .. center)
      // 2. (center .. right)
      // if it is, then set nearest yaw to current yaw, else choose center (min_edge_delta_angle.y)

      // if we are looking over the head, then it will clamp to the head,
      // if we are looking below the feet, then it will clamp to the feet,
      // else it keeps our pitch as is
      float nearest_pitch = clampf(local_viewangles.x, viewangles[BONE_HEAD].x, viewangles[BONE_FEET].x);
      float nearest_yaw = FLT_MAX;

      // within range is failing. We probably need to normalize angles to [0, 360]
      if (within_range(local, left, center) || within_range(local, center, right)) {
        nearest_yaw = local_viewangles.y;
      } else {
        for (size_t j = 0; j < MAX_NEIGHBOURS; j++) {
          if (fabs(neighbours_delta_angles[j]) < fabs(nearest_yaw)) {
            nearest_yaw = edges_viewangles[j].y;
          }
        }
      }
      vec2_t nearest_viewangles = {nearest_pitch, nearest_yaw};
      calculate_delta_angle(&local_viewangles, &nearest_viewangles, &delta_angles[i]);
      break;
    case BONE_HEAD:
      // same as feet, but we are adding height to the base
      tmp_pos.y += player_get_eye_height(game->env, game, closest_player);
      calculate_delta_pos_vec3(&tmp_pos, &localplayer_position, &delta_position);
      viewangles[i].x = rad2deg(-atan2f(delta_position.y, hypot(delta_position.x, delta_position.z)));
      viewangles[i].y = rad2deg(-atan2f(delta_position.x, delta_position.z));
      calculate_delta_angle(&local_viewangles, &viewangles[i], &delta_angles[i]);
      break;
    case BONE_CHEST:
      // just multiplying the height to get the chest position
      tmp_pos.y += player_get_height(game->env, game, closest_player) * 0.67;
      calculate_delta_pos_vec3(&tmp_pos, &localplayer_position, &delta_position);
      viewangles[i].x = rad2deg(-atan2f(delta_position.y, hypot(delta_position.x, delta_position.z)));
      viewangles[i].y = rad2deg(-atan2f(delta_position.x, delta_position.z));
      calculate_delta_angle(&local_viewangles, &viewangles[i], &delta_angles[i]);
      break;
    default:
      break;
    }
  }

  // fov should be working by the nearest bone
  float fov = hypot(delta_angles[BONE_NEAREST].x, delta_angles[BONE_NEAREST].y);
  if (game->menu->aimbot_settings.fov != 0.0f && fov > game->menu->aimbot_settings.fov)
    return;

  if (game->menu->aimbot_settings.bind.set) {
    if (game->menu->open)
      return;

    switch (game->menu->aimbot_settings.bind.type) {
    case BIND_MOUSE:
      if (glfwGetMouseButton(game->rctx->win, game->menu->aimbot_settings.bind.as_code) == GLFW_PRESS) {
        exp_smooth(game, &delta_angles[game->menu->aimbot_settings.bone], &local_viewangles,
                   game->menu->aimbot_settings.smooth_time, game->update_time);
      }
      break;
    case BIND_KEY:
      if (glfwGetKey(game->rctx->win, game->menu->aimbot_settings.bind.as_code) == GLFW_PRESS) {
        exp_smooth(game, &delta_angles[game->menu->aimbot_settings.bone], &local_viewangles,
                   game->menu->aimbot_settings.smooth_time, game->update_time);
      }
      break;
    }
  } else {
    exp_smooth(game, &delta_angles[game->menu->aimbot_settings.bone], &local_viewangles,
               game->menu->aimbot_settings.smooth_time, game->update_time);
  }
}

void aimbot(game_t *game) {

  if (!game->in_game)
    return;

  aim(game);
  trigger(game);
}