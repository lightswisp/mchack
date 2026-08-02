#include "fullbright.h"
#include "sprint.h"
#include "nofall.h"
#include "fly.h"
#include "infjump.h"
#include "speed.h"
#include "misc.h"

void misc(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  if (!menu_state->misc_settings.enabled)
    return;
  if (!get_game()->in_game)
    return;

  sprint(env, game, menu_state->misc_settings.sprint);
  nofall(env, game, menu_state->misc_settings.nofall);
  fly(env, game, menu_state);
  inf_jump(env, game, menu_state);
  speed(env, game, menu_state);
  fullbright(env, game, menu_state);
}