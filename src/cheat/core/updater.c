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

  game->localplayer.abilities.class = abilities_class;
  game->localplayer.abilities.instance = abilities;
  game->localplayer.abilities.mayfly = mayfly;
  game->localplayer.on_ground = on_ground;
  game->localplayer.super = super;
  game->localplayer.set_sprinting = set_sprinting;
  game->localplayer.is_moving = is_moving;
  game->localplayer.updated = true;
  LOGGER_LOG(LOG, "local player is updated!");
}

void reset_updates(game_t *game) {
  game->in_game = false;
  game->localplayer.updated = false;
}

void update(JNIEnv *env, game_t *game) {
  jobject local_player = get_localplayer(env, game);
  if (!local_player) {
    // revert updates for all (local player, players, entities, world, etc)
    reset_updates(game);
    return;
  }
  // we are in game, so we are safe to update structs
  game->in_game = true;

  update_localplayer(env, game, local_player);
}