#include "updater.h"
#include "localplayer.h"
#include "../utils/logger.h"

static void update_localplayer(JNIEnv *env, game_t *game, jobject local_player) {

  game->localplayer.instance = local_player;
  game->localplayer.class = (*env)->GetObjectClass(env, local_player);

  jclass super = (*env)->GetSuperclass(env, game->localplayer.class);
  jfieldID abilities_field = (*env)->GetFieldID(env, game->localplayer.class, "abilities",
                                                "Lnet/minecraft/world/entity/player/Abilities;");

  jobject abilities = (*env)->GetObjectField(env, game->localplayer.instance, abilities_field);

  jclass abilities_class = (*env)->GetObjectClass(env, abilities);
  jfieldID mayfly = (*env)->GetFieldID(env, abilities_class, "mayfly", "Z");
  jfieldID walking_speed = (*env)->GetFieldID(env, abilities_class, "walkingSpeed", "F");
  jfieldID on_ground = (*env)->GetFieldID(env, game->localplayer.class, "onGround", "Z");
  jmethodID set_sprinting = (*env)->GetMethodID(env, super, "setSprinting", "(Z)V");
  jmethodID is_moving = (*env)->GetMethodID(env, game->localplayer.class, "isMoving", "()Z");
  jmethodID display_client_message = (*env)->GetMethodID(env, game->localplayer.class, "displayClientMessage",
                                                         "(Lnet/minecraft/network/chat/Component;Z)V");
  jmethodID get_x = (*env)->GetMethodID(env, game->localplayer.class, "getX", "()D");
  jmethodID get_y = (*env)->GetMethodID(env, game->localplayer.class, "getY", "()D");
  jmethodID get_z = (*env)->GetMethodID(env, game->localplayer.class, "getZ", "()D");

  jfieldID x_rot = (*env)->GetFieldID(env, game->localplayer.class, "xRot", "F");
  jfieldID y_rot = (*env)->GetFieldID(env, game->localplayer.class, "yRot", "F");
  jfieldID delta_movement =
      (*env)->GetFieldID(env, game->localplayer.class, "deltaMovement", "Lnet/minecraft/world/phys/Vec3;");
  jmethodID set_delta_movement =
      (*env)->GetMethodID(env, game->localplayer.class, "setDeltaMovement", "(DDD)V");
  jmethodID get_delta_movement = (*env)->GetMethodID(env, game->localplayer.class, "getDeltaMovement",
                                                     "()Lnet/minecraft/world/phys/Vec3;");

  game->localplayer.delta_movement = delta_movement;
  game->localplayer.set_delta_movement = set_delta_movement;
  game->localplayer.get_delta_movement = get_delta_movement;
  game->localplayer.abilities.class = abilities_class;
  game->localplayer.abilities.instance = abilities;
  game->localplayer.abilities.mayfly = mayfly;
  game->localplayer.abilities.walking_speed = walking_speed;
  game->localplayer.on_ground = on_ground;
  game->localplayer.super = super;
  game->localplayer.set_sprinting = set_sprinting;
  game->localplayer.is_moving = is_moving;
  game->localplayer.display_client_message = display_client_message;
  game->localplayer.get_x = get_x;
  game->localplayer.get_y = get_y;
  game->localplayer.get_z = get_z;
  game->localplayer.x_rot = x_rot;
  game->localplayer.y_rot = y_rot;
}

static void update_options(JNIEnv *env, game_t *game) {
  jfloat fov = (*env)->GetDoubleField(env, game->minecraft.options.instance, game->minecraft.options.fov);
  game->minecraft.options.fov_val = fov;
}

static void update_players(JNIEnv *env, game_t *game) {

  jfieldID client_level_field = (*env)->GetFieldID(env, game->minecraft.class, "level",
                                                   "Lnet/minecraft/client/multiplayer/ClientLevel;");
  jobject client_level = (*env)->GetObjectField(env, game->minecraft.instance, client_level_field);

  jclass client_level_class = (*env)->GetObjectClass(env, client_level);
  jfieldID players_field = (*env)->GetFieldID(env, client_level_class, "players", "Ljava/util/List;");
  jobject players = (*env)->GetObjectField(env, client_level, players_field);
  jclass players_class = (*env)->FindClass(env, "Lnet/minecraft/world/entity/player/Player;");

  jmethodID is_local_player = (*env)->GetMethodID(env, players_class, "isLocalPlayer", "()Z");
  jmethodID get_x = (*env)->GetMethodID(env, players_class, "getX", "()D");
  jmethodID get_y = (*env)->GetMethodID(env, players_class, "getY", "()D");
  jmethodID get_z = (*env)->GetMethodID(env, players_class, "getZ", "()D");
  jfieldID glowing = (*env)->GetFieldID(env, players_class, "glowing", "Z");

  game->players.class = players_class;
  game->players.instance = players;
  game->players.is_local_player = is_local_player;
  game->players.get_x = get_x;
  game->players.get_y = get_y;
  game->players.get_z = get_z;
  game->players.glowing = glowing;
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
  update_options(env, game);
}