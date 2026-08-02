#include "speed.h"
#include "../../utils/math.h"
#include "../../core/localplayer.h"
#include "../../utils/logger.h"
#include "../../rendering/gui.h"
#include <GLFW/glfw3.h>

void speed(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  // We may also try to modify player's impulse:
  //  this.leftImpulse = (float) (((double) this.leftImpulse) * 0.3d);
  //  this.forwardImpulse = (float)(((double)this.forwardImpulse) * 0.3d);
  float speed = menu_state->misc_settings.speed;
  if (speed == 0.2f)
    return;
  vec3_t delta_movement;
  vec2_t viewangles;
  localplayer_get_delta_movement(env, game, &delta_movement);
  localplayer_get_viewangles(env, game, &viewangles);
  if (delta_movement.x == 0 && delta_movement.z == 0)
    return;

  float z_motion = cosf(deg2rad(viewangles.y)) * speed;
  float x_motion = -sinf(deg2rad(viewangles.y)) * speed;

  delta_movement.x = 0;
  delta_movement.z = 0;

  if (glfwGetKey(menu_state->rctx->win, GLFW_KEY_W) == GLFW_PRESS) {

    delta_movement.x += x_motion;
    delta_movement.z += z_motion;
  }
  if (glfwGetKey(menu_state->rctx->win, GLFW_KEY_S) == GLFW_PRESS) {
    delta_movement.x += -x_motion;
    delta_movement.z += -z_motion;
  }
  if (glfwGetKey(menu_state->rctx->win, GLFW_KEY_A) == GLFW_PRESS) {
    delta_movement.x += z_motion;
    delta_movement.z += -x_motion;
  }
  if (glfwGetKey(menu_state->rctx->win, GLFW_KEY_D) == GLFW_PRESS) {
    delta_movement.x += -z_motion;
    delta_movement.z += x_motion;
  }

  localplayer_set_delta_movement(env, game, &delta_movement);
}