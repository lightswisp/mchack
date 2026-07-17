#include "game.h"
#include "core/minecraft.h"
#include "core/localplayer.h"
#include "jvm.h"
#include "utils/logger.h"

static game_t g_game = {0};

static bool init_collections(void) {
  JNIEnv *env = get_env();
  jclass list_class = (*env)->FindClass(env, "java/util/List");
  if (!list_class) {
    LOGGER_LOG(FATAL, "failed to get list class");
    return false;
  }
  jmethodID list_size = (*env)->GetMethodID(env, list_class, "size", "()I");
  if (!list_size) {
    LOGGER_LOG(FATAL, "failed to get list size method");
    return false;
  }
  jmethodID list_get = (*env)->GetMethodID(env, list_class, "get", "(I)Ljava/lang/Object;");
  if (!list_get) {
    LOGGER_LOG(FATAL, "failed to get list getter");
    return false;
  }

  jclass map_class = (*env)->FindClass(env, "java/util/Map");
  if (!map_class) {
    LOGGER_LOG(FATAL, "failed to get map class");
    return false;
  }
  jmethodID map_clear = (*env)->GetMethodID(env, map_class, "clear", "()V");
  if (!map_clear) {
    LOGGER_LOG(FATAL, "failed to get clear method");
    return false;
  }

  g_game.collections.map_class = map_class;
  g_game.collections.map_clear = map_clear;
  g_game.collections.list_class = list_class;
  g_game.collections.list_size = list_size;
  g_game.collections.list_get = list_get;

  return true;
}

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

  if (!init_collections()) {
    LOGGER_LOG(FATAL, "failed to init collections");
    return false;
  }

  g_game.local_player = local_player;
  g_game.mclass = mclass;
  g_game.minstance = minstance;
  return true;
}

game_t *get_game(void) { return &g_game; }
