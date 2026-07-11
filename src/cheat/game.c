#include "game.h"
#include "core/minecraft.h"
#include "core/player.h"
#include "jvm.h"
#include "utils/logger.h"

static game_t g_game = {0};

bool init_game(void) {
  JNIEnv *env = get_env();
  jclass mclass = get_minecraft_class(env);
  jobject minstance = get_minecraft_instance(env, mclass);
  if (!minstance) {
    LOGGER_LOG(FATAL, "falied to get minecraft");
    return false;
  }
  jobject local_player = get_localplayer(env, mclass, minstance);
  if (!local_player) {
    LOGGER_LOG(FATAL, "falied to get local player");
    return false;
  }
  g_game.local_player = local_player;
  g_game.mclass = mclass;
  g_game.minstance = minstance;
  return true;
}

game_t *get_game(void) { return &g_game; }
