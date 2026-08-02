#include "infjump.h"
#include "../../core/localplayer.h"
#include <GLFW/glfw3.h>

void inf_jump(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  if (!menu_state->misc_settings.inf_jump)
    return;

  if (glfwGetKey(menu_state->rctx->win, GLFW_KEY_W) == GLFW_PRESS) {
    localplayer_set_on_ground(env, game, true);
  }
}