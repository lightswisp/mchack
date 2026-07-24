#include "trigger.h"
#include "../../core/localplayer.h"
#include "../../core/player.h"
#include "../../utils/logger.h"

void attack(JNIEnv *env, game_t *game) {
  (*env)->CallVoidMethod(env, game->minecraft.instance, game->minecraft.start_attack);
}

void trigger(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  if (!menu_state->aimbot_settings.trigger)
    return;
  if (!game->minecraft.crosshair_pick_entity)
    return;
  if (!(*env)->IsInstanceOf(env, game->minecraft.crosshair_pick_entity, game->players.class))
    return;

  vec3_t localplayer_position;
  vec3_t player_position;

  localplayer_get_position(env, game, &localplayer_position);
  player_get_position(env, game, game->minecraft.crosshair_pick_entity, &player_position);

  attack(env, game);
}