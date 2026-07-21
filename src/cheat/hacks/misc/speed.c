#include "speed.h"
#include "../../utils/math.h"
#include "../../core/localplayer.h"
#include "../../utils/logger.h"
#include "../../rendering/gui.h"
#include <GLFW/glfw3.h>

void speed(JNIEnv *env, game_t *game, float speed) {
  if (speed == 1.0f)
    return;
  vec3_t delta_movement;
  localplayer_get_delta_movement(env, game, &delta_movement);
  if (delta_movement.x == 0 && delta_movement.z == 0)
    return;

  menu_state_t *menu = get_menu_state();

  if (glfwGetKey(menu->win, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(menu->win, GLFW_KEY_S) == GLFW_PRESS ||
      glfwGetKey(menu->win, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(menu->win, GLFW_KEY_D) == GLFW_PRESS)
    delta_movement.x *= 1.05;
  delta_movement.z *= 1.05;
  localplayer_set_delta_movement(env, game, &delta_movement);
}