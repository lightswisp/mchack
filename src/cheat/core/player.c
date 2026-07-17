#include "player.h"
#include "../utils/logger.h"
#include "../game.h"
#include "../jvm.h"

jobject get_players(jint *size) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  jfieldID client_level_field =
      (*env)->GetFieldID(env, game->mclass, "level", "Lnet/minecraft/client/multiplayer/ClientLevel;");
  if (!client_level_field) {
    LOGGER_LOG(FATAL, "failed to get client level field");
    return NULL;
  }
  jobject client_level = (*env)->GetObjectField(env, game->minstance, client_level_field);
  if (!client_level) {
    LOGGER_LOG(FATAL, "failed to get client level");
    return NULL;
  }

  jclass client_level_class = (*env)->GetObjectClass(env, client_level);
  if (!client_level_class) {
    LOGGER_LOG(FATAL, "failed to get client level class");
    return NULL;
  }
  jfieldID players_field = (*env)->GetFieldID(env, client_level_class, "players", "Ljava/util/List;");
  if (!players_field) {
    LOGGER_LOG(FATAL, "failed to get players field");
    return NULL;
  }
  jobject players = (*env)->GetObjectField(env, client_level, players_field);
  if (!players) {
    LOGGER_LOG(FATAL, "failed to get players");
    return NULL;
  }
  *size = (*env)->CallIntMethod(env, players, game->collections.list_size);
  (*env)->DeleteLocalRef(env, client_level_class);
  (*env)->DeleteLocalRef(env, client_level);
  return players;
}

jstring get_name(jobject player, jint index) {}