#include "fullbright.h"
#include "sprint.h"
#include "nofall.h"
#include "fly.h"
#include "infjump.h"
#include "speed.h"
#include "fastplace.h"
#include "misc.h"

void misc(JNIEnv *env, jobject this, game_t *game) {
  if (!game->menu->misc_settings.enabled)
    return;
  if (!game->in_game)
    return;

  sprint(env, this, game);
  nofall(env, this, game);
  fly(env, this, game);
  inf_jump(env, this, game);
  speed(env, this, game);
  fullbright(env, game);
  fastplace(env, game);
}