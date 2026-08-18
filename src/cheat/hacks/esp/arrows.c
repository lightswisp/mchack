#include <string.h>
#include "arrows.h"
#include "../../core/localplayer.h"
#include "../../core/player.h"
#include "../../utils/logger.h"
#include "../../core/camera.h"

void arrows(game_t *game) {
  // it will traverse all players
  // then it will calculate the angle between our crosshair and the angle we need to turn to the enemy
  // this angle will be added to the menu_state->esp_settings.arrows2draw[i]
  // then an arrow will be drawn on the circle.
  // rotation matrix must be applied before altering x,y coordianates
  // X coordinate of an arrow will be calculated as     cos(angle) * R
  // Y coordinate of an arrow will be calculated as     sin(angle) * R
  // where R is the radius of our arrow circle
  if (!game->menu->esp_settings.arrows.enabled)
    return;

  vec3_t player_position;
  size_t arrows = 0;
  float arrow_size = 20.0f;
  jint max_arrows = clampi(game->players.size, 0, MAX_ENTITIES);

  for (jint i = 0; i < max_arrows; i++) {
    jobject player = (*game->env)->CallObjectMethod(game->env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(game->env, game, player))
      continue;

    player_get_lerped_position(game->env, game, player, &player_position);
    float distance = calculate_distance_vec3(&game->esp->core.local.camera_position, &player_position);
    if (distance > game->menu->esp_settings.arrows.distance)
      continue;

    float delta_x = player_position.x - game->esp->core.local.camera_position.x;
    float delta_z = player_position.z - game->esp->core.local.camera_position.z;
    float yaw = rad2deg(-atan2f(delta_x, delta_z));

    float delta_yaw = yaw - normalize_angle(game->esp->core.local.camera_angles_deg.y);
    delta_yaw = normalize_angle(delta_yaw);
    float arrow_angle = deg2rad(90.0f - delta_yaw);

    float x = game->rctx->width / 2.0;
    float y = game->rctx->height / 2.0;
    float angle90 = arrow_angle - HALF_PI;
    vec2_t t1[3] = {{x, y}, {x, y + arrow_size}, {x - arrow_size, y + arrow_size * 2}};
    vec2_t t2[3] = {{x, y}, {x, y + arrow_size}, {x + arrow_size, y + arrow_size * 2}};

    // first rotate
    transform_rotate(&t1[0], game->rctx->width, game->rctx->height, angle90);
    transform_rotate(&t1[1], game->rctx->width, game->rctx->height, angle90);
    transform_rotate(&t1[2], game->rctx->width, game->rctx->height, angle90);
    transform_rotate(&t2[0], game->rctx->width, game->rctx->height, angle90);
    transform_rotate(&t2[1], game->rctx->width, game->rctx->height, angle90);
    transform_rotate(&t2[2], game->rctx->width, game->rctx->height, angle90);

    // then move
    vec2_t add_factor = {cosf(arrow_angle) * game->menu->esp_settings.arrows.radius,
                         -sinf(arrow_angle) * game->menu->esp_settings.arrows.radius};
    vec2add(&t1[0], &add_factor);
    vec2add(&t1[1], &add_factor);
    vec2add(&t1[2], &add_factor);
    vec2add(&t2[0], &add_factor);
    vec2add(&t2[1], &add_factor);
    vec2add(&t2[2], &add_factor);

    // setting vertices
    game->esp->arrows.buffer[arrows].t1[0] = t1[0];
    game->esp->arrows.buffer[arrows].t1[1] = t1[1];
    game->esp->arrows.buffer[arrows].t1[2] = t1[2];
    game->esp->arrows.buffer[arrows].t2[0] = t2[0];
    game->esp->arrows.buffer[arrows].t2[1] = t2[1];
    game->esp->arrows.buffer[arrows].t2[2] = t2[2];
    arrows++;
  }

  game->esp->arrows.size = arrows;
}