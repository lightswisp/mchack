#include "updater.h"
#include "localplayer.h"
#include "../utils/logger.h"

// maybe we should start using GlobalRefs instead of always deleting and updating objects?

static bool update_localplayer(JNIEnv *env, game_t *game) {
  jobject local_player = localplayer_get(env, game);
  CHECK(local_player);
  DELETE_REF(env, game->localplayer.instance);

  jobject abilities = (*env)->GetObjectField(env, local_player, game->localplayer.abilities_field);
  CHECK(abilities);
  DELETE_REF(env, game->localplayer.abilities.instance);

  jint tick_count = (*env)->GetIntField(env, local_player, game->localplayer.tick_count_field);

  game->localplayer.tick_count = tick_count;
  game->localplayer.abilities.instance = abilities;
  game->localplayer.instance = local_player;

  return true;
}

static bool update_minecraft(JNIEnv *env, game_t *game) {
  jfloat fov = (*env)->GetDoubleField(env, game->minecraft.options.instance, game->minecraft.options.fov);

  // NO NEED TO CHECK IF IT'S NULL
  jobject crosshair_pick_entity =
      (*env)->GetObjectField(env, game->minecraft.instance, game->minecraft.crosshair_field);
  DELETE_REF(env, game->minecraft.crosshair_pick_entity);
  jfloat partial_tick = (*env)->CallFloatMethod(env, game->minecraft.instance, game->minecraft.get_frame_time);

  game->minecraft.partial_tick = partial_tick;
  game->minecraft.options.fov_val = fov;
  game->minecraft.crosshair_pick_entity = crosshair_pick_entity;

  return true;
}

static bool update_game_renderer(JNIEnv *env, game_t *game) {
  jobject game_renderer = (*env)->GetObjectField(env, game->minecraft.instance, game->minecraft.game_renderer_field);
  CHECK(game_renderer);
  DELETE_REF(env, game->game_renderer.instance);
  jfloat game_renderer_fov_val = (*env)->GetFloatField(env, game_renderer, game->game_renderer.fov);

  game->game_renderer.instance = game_renderer;
  game->game_renderer.fov_val = game_renderer_fov_val;

  return true;
}

static bool update_camera(JNIEnv *env, game_t *game) {
  jobject camera = (*env)->GetObjectField(env, game->game_renderer.instance, game->game_renderer.camera_field);
  CHECK(camera);
  DELETE_REF(env, game->camera.instance);

  jobject position = (*env)->GetObjectField(env, camera, game->camera.position_field);
  CHECK(position);
  DELETE_REF(env, game->camera.position);

  game->camera.instance = camera;
  game->camera.position = position;

  return true;
}

static bool update_client_level(JNIEnv *env, game_t *game) {
  jobject client_level = (*env)->GetObjectField(env, game->minecraft.instance, game->minecraft.client_level_field);
  CHECK(client_level);
  DELETE_REF(env, game->client_level.instance);

  game->client_level.instance = client_level;

  return true;
}

static bool update_players(JNIEnv *env, game_t *game) {
  if (!game->client_level.instance)
    return false;

  jobject players = (*env)->GetObjectField(env, game->client_level.instance, game->client_level.players_field);
  CHECK(players);
  DELETE_REF(env, game->players.instance);

  game->players.instance = players;
  game->players.size = (*env)->CallIntMethod(env, game->players.instance, game->collections.list.size);

  return true;
}

void reset_updates(game_t *game, menu_state_t *menu_state, overlay_t *overlay) {
  game->in_game = false;
  overlay->arrows.size = 0;
  overlay->boxes.size = 0;
}

bool update(JNIEnv *env, game_t *game, menu_state_t *menu_state, overlay_t *overlay) {
  jobject local_player = localplayer_get(env, game);
  if (!local_player) {
    // revert updates for all (local player, players, entities, world, etc)
    reset_updates(game, menu_state, overlay);
    return false;
  }
  // we are in game, so we are safe to update structs
  game->in_game = true;

  CHECK_BOOL(update_localplayer(env, game));
  CHECK_BOOL(update_client_level(env, game));
  CHECK_BOOL(update_players(env, game));
  CHECK_BOOL(update_minecraft(env, game));
  CHECK_BOOL(update_game_renderer(env, game));
  CHECK_BOOL(update_camera(env, game));

  return true;
}