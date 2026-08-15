#include "sprint.h"
#include "../../core/localplayer.h"
#include "../../core/keys.h"
#include "../../utils/logger.h"

void sprint(JNIEnv *env, jobject this, game_t *game) {
  if (!game->menu->misc_settings.sprint)
    return;

  // Another way to do sprint
  // vec2_t impulse = {menu_state->misc_settings.speed, menu_state->misc_settings.speed};
  // localplayer_set_impulse2(env, this, game, &impulse);

  if (glfwGetKey(game->rctx->win, GLFW_KEY_W) == GLFW_PRESS) {
    localplayer_set_sprinting2(env, this, game);
  }
}