#include "player.h"
#include "../utils/logger.h"
#include "../game.h"
#include "../jvm.h"

jobject get_players(jint *size) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  jfieldID client_level_field = (*env)->GetFieldID(env, game->minecraft.class, "level",
                                                   "Lnet/minecraft/client/multiplayer/ClientLevel;");
  if (!client_level_field) {
    LOGGER_LOG(FATAL, "failed to get client level field");
    return NULL;
  }
  jobject client_level = (*env)->GetObjectField(env, game->minecraft.instance, client_level_field);
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
  *size = (*env)->CallIntMethod(env, players, game->collections.list.size);
  (*env)->DeleteLocalRef(env, client_level_class);
  (*env)->DeleteLocalRef(env, client_level);
  return players;
}

jstring get_name(jobject players, jint index) {}

bool is_localplayer(jobject players, jint index) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  jobject player = (*env)->CallObjectMethod(env, players, game->collections.list.get, index);
  jclass player_class = (*env)->GetObjectClass(env, player);
  jmethodID is_local = (*env)->GetMethodID(env, player_class, "isLocalPlayer", "()Z");
  if (!is_local) {
    // LOGGER_LOG(FATAL, fmt, ...)
  }
}

bool get_position(jobject players, jint index, vec3_t *out) {
  JNIEnv *env = get_env();
  game_t *game = get_game();
  jobject player = (*env)->CallObjectMethod(env, players, game->collections.list.get, index);
  jclass player_class = (*env)->GetObjectClass(env, player);
  jmethodID get_x = (*env)->GetMethodID(env, player_class, "getX", "()D");
  jmethodID get_y = (*env)->GetMethodID(env, player_class, "getY", "()D");
  jmethodID get_z = (*env)->GetMethodID(env, player_class, "getZ", "()D");
  if (!get_x && !get_y && !get_z) {
    LOGGER_LOG(FATAL, "failed to get position methods");
    return false;
  }
  double x = (*env)->CallDoubleMethod(env, player, get_x);
  double y = (*env)->CallDoubleMethod(env, player, get_x);
  double z = (*env)->CallDoubleMethod(env, player, get_x);
  out->x = x;
  out->y = y;
  out->z = z;
  (*env)->DeleteLocalRef(env, player_class);
  (*env)->DeleteLocalRef(env, player);
  return true;
}