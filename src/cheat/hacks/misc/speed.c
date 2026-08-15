#include "speed.h"
#include "../../utils/math.h"
#include "../../core/localplayer.h"
#include "../../utils/logger.h"
#include "../../rendering/menu.h"
#include <GLFW/glfw3.h>

void speed(JNIEnv *env, jobject this, game_t *game) {
  float speed = game->menu->misc_settings.speed;
  if (speed == 0.2f)
    return;

  vec3_t delta_movement;
  vec2_t viewangles;
  localplayer_get_delta_movement2(env, this, game, &delta_movement);
  localplayer_get_viewangles_rad2(env, this, game, &viewangles);
  if (delta_movement.x == 0 && delta_movement.z == 0)
    return;

  float z_motion = cosf(viewangles.y) * speed;
  float x_motion = -sinf(viewangles.y) * speed;

  delta_movement.x = 0;
  delta_movement.z = 0;

  if (glfwGetKey(game->rctx->win, GLFW_KEY_W) == GLFW_PRESS) {

    delta_movement.x += x_motion;
    delta_movement.z += z_motion;
  }
  if (glfwGetKey(game->rctx->win, GLFW_KEY_S) == GLFW_PRESS) {
    delta_movement.x += -x_motion;
    delta_movement.z += -z_motion;
  }
  if (glfwGetKey(game->rctx->win, GLFW_KEY_A) == GLFW_PRESS) {
    delta_movement.x += z_motion;
    delta_movement.z += -x_motion;
  }
  if (glfwGetKey(game->rctx->win, GLFW_KEY_D) == GLFW_PRESS) {
    delta_movement.x += -z_motion;
    delta_movement.z += x_motion;
  }

  localplayer_set_delta_movement2(env, this, game, &delta_movement);
}