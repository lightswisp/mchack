#include "infjump.h"
#include "../../core/localplayer.h"
#include "../../utils/logger.h"
#include <GLFW/glfw3.h>

void inf_jump(JNIEnv *env, jobject this, game_t *game) {
  if (!game->menu->misc_settings.inf_jump)
    return;

  if (glfwGetKey(game->rctx->win, GLFW_KEY_SPACE) == GLFW_PRESS) {
    localplayer_set_on_ground2(env, this, game, true);
  }
}