#include "updater.h"
#include "localplayer.h"
#include "../utils/logger.h"

static void update_localplayer(JNIEnv *env, game_t *game, jobject local_player) {
  if (game->localplayer.updated)
    return;

  game->localplayer.instance = local_player;
  game->localplayer.class = (*env)->GetObjectClass(env, local_player);

  jclass super = (*env)->GetSuperclass(env, game->localplayer.class);
  jfieldID abilities_field = (*env)->GetFieldID(env, game->localplayer.class, "abilities",
                                                "Lnet/minecraft/world/entity/player/Abilities;");

  jobject abilities = (*env)->GetObjectField(env, game->localplayer.instance, abilities_field);

  jclass abilities_class = (*env)->GetObjectClass(env, abilities);
  jfieldID mayfly = (*env)->GetFieldID(env, abilities_class, "mayfly", "Z");
  jfieldID on_ground = (*env)->GetFieldID(env, game->localplayer.class, "onGround", "Z");
  jmethodID set_sprinting = (*env)->GetMethodID(env, super, "setSprinting", "(Z)V");
  jmethodID is_moving = (*env)->GetMethodID(env, game->localplayer.class, "isMoving", "()Z");
  jmethodID display_client_message = (*env)->GetMethodID(env, game->localplayer.class, "displayClientMessage",
                                                         "(Lnet/minecraft/network/chat/Component;Z)V");

  game->localplayer.abilities.class = abilities_class;
  game->localplayer.abilities.instance = abilities;
  game->localplayer.abilities.mayfly = mayfly;
  game->localplayer.on_ground = on_ground;
  game->localplayer.super = super;
  game->localplayer.set_sprinting = set_sprinting;
  game->localplayer.is_moving = is_moving;
  game->localplayer.display_client_message = display_client_message;
  game->localplayer.updated = true;
  LOGGER_LOG(LOG, "local player is updated!");
}

static void update_players(JNIEnv *env, game_t *game) {
  if (game->players.updated)
    return;

  jfieldID client_level_field = (*env)->GetFieldID(env, game->minecraft.class, "level",
                                                   "Lnet/minecraft/client/multiplayer/ClientLevel;");
  jobject client_level = (*env)->GetObjectField(env, game->minecraft.instance, client_level_field);

  jclass client_level_class = (*env)->GetObjectClass(env, client_level);
  jfieldID players_field = (*env)->GetFieldID(env, client_level_class, "players", "Ljava/util/List;");
  jobject players = (*env)->GetObjectField(env, client_level, players_field);
  jclass players_class = (*env)->GetObjectClass(env, players);

  jmethodID is_local_player = (*env)->GetMethodID(env, players_class, "isLocalPlayer", "()Z");
  jmethodID get_x = (*env)->GetMethodID(env, players_class, "getX", "()D");
  jmethodID get_y = (*env)->GetMethodID(env, players_class, "getY", "()D");
  jmethodID get_z = (*env)->GetMethodID(env, players_class, "getZ", "()D");

  game->players.class = players_class;
  game->players.instance = players;
  game->players.is_local_player = is_local_player;
  game->players.get_x = get_x;
  game->players.get_y = get_y;
  game->players.get_z = get_z;

  (*env)->DeleteLocalRef(env, client_level);
  (*env)->DeleteLocalRef(env, client_level_class);

  LOGGER_LOG(LOG, "players are updated!");
  game->players.updated = true;
}

void update_players_size(JNIEnv *env, game_t *game) {
  // it will constantly update player's size, because it is dynamic
  game->players.size = (*env)->CallIntMethod(env, game->players.instance, game->collections.list.size);
}


void reset_updates(game_t *game) {
  game->in_game = false;
  game->localplayer.updated = false;
  game->players.updated = false;
}

void update(JNIEnv *env, game_t *game) {
  jobject local_player = localplayer_get(env, game);
  if (!local_player) {
    // revert updates for all (local player, players, entities, world, etc)
    reset_updates(game);
    return;
  }
  // we are in game, so we are safe to update structs
  game->in_game = true;

  update_localplayer(env, game, local_player);
  update_players(env, game);
  update_players_size(env, game);
}