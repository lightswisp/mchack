#include "updater.h"
#include "localplayer.h"
#include "../utils/logger.h"

// maybe we should start using GlobalRefs instead of always deleting and updating objects?

static bool update_localplayer(game_t *game) {
  jobject local_player = localplayer_get(game->env, game);
  CHECK(local_player);
  DELETE_REF(game->env, game->localplayer.instance);

  jobject abilities = (*game->env)->GetObjectField(game->env, local_player, game->localplayer.abilities_field);
  CHECK(abilities);
  DELETE_REF(game->env, game->localplayer.abilities.instance);

  jint tick_count = (*game->env)->GetIntField(game->env, local_player, game->localplayer.tick_count_field);

  game->localplayer.tick_count = tick_count;
  game->localplayer.abilities.instance = abilities;
  game->localplayer.instance = local_player;

  return true;
}

static bool update_minecraft(game_t *game) {
  jfloat fov = (*game->env)->GetDoubleField(game->env, game->minecraft.options.instance, game->minecraft.options.fov);

  // NO NEED TO CHECK IF IT'S NULL
  jobject crosshair_pick_entity =
      (*game->env)->GetObjectField(game->env, game->minecraft.instance, game->minecraft.crosshair_field);
  DELETE_REF(game->env, game->minecraft.crosshair_pick_entity);
  jfloat partial_tick =
      (*game->env)->CallFloatMethod(game->env, game->minecraft.instance, game->minecraft.get_frame_time);

  game->minecraft.partial_tick = partial_tick;
  game->minecraft.options.fov_val = fov;
  game->minecraft.crosshair_pick_entity = crosshair_pick_entity;

  return true;
}

static bool update_game_renderer(game_t *game) {
  jobject game_renderer =
      (*game->env)->GetObjectField(game->env, game->minecraft.instance, game->minecraft.game_renderer_field);
  CHECK(game_renderer);
  DELETE_REF(game->env, game->game_renderer.instance);
  jfloat game_renderer_fov_val = (*game->env)->GetFloatField(game->env, game_renderer, game->game_renderer.fov);

  game->game_renderer.instance = game_renderer;
  game->game_renderer.fov_val = game_renderer_fov_val;

  return true;
}

static bool update_camera(game_t *game) {
  jobject camera =
      (*game->env)->GetObjectField(game->env, game->game_renderer.instance, game->game_renderer.camera_field);
  CHECK(camera);
  DELETE_REF(game->env, game->camera.instance);

  jobject position = (*game->env)->GetObjectField(game->env, camera, game->camera.position_field);
  CHECK(position);
  DELETE_REF(game->env, game->camera.position);

  game->camera.instance = camera;
  game->camera.position = position;

  return true;
}

static bool update_client_level(game_t *game) {
  jobject client_level =
      (*game->env)->GetObjectField(game->env, game->minecraft.instance, game->minecraft.client_level_field);
  CHECK(client_level);
  DELETE_REF(game->env, game->client_level.instance);

  game->client_level.instance = client_level;

  return true;
}

static bool update_players(game_t *game) {
  if (!game->client_level.instance)
    return false;

  jobject players =
      (*game->env)->GetObjectField(game->env, game->client_level.instance, game->client_level.players_field);
  CHECK(players);
  DELETE_REF(game->env, game->players.instance);

  game->players.instance = players;
  game->players.size = (*game->env)->CallIntMethod(game->env, game->players.instance, game->collections.list.size);

  return true;
}

void reset_updates(game_t *game) {
  game->in_game = false;
  game->esp->arrows.size = 0;
  game->esp->boxes.size = 0;
}

bool update(game_t *game) {
  jobject local_player = localplayer_get(game->env, game);
  if (!local_player) {
    // revert updates for all (local player, players, entities, world, etc)
    reset_updates(game);
    return false;
  }
  // we are in game, so we are safe to update structs
  game->in_game = true;

  CHECK_BOOL(update_localplayer(game));
  CHECK_BOOL(update_client_level(game));
  CHECK_BOOL(update_players(game));
  CHECK_BOOL(update_minecraft(game));
  CHECK_BOOL(update_game_renderer(game));
  CHECK_BOOL(update_camera(game));

  return true;
}