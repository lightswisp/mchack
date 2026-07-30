#include "trigger.h"
#include "../../core/localplayer.h"
#include "../../core/player.h"
#include "../../utils/logger.h"

void attack(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  float attack_strength = localplayer_get_attack_strength(env, game, 1.0);
  if (attack_strength < menu_state->aimbot_settings.attack_threshold)
    return;

  if (menu_state->aimbot_settings.crits) {
    attack_strength = localplayer_get_attack_strength(env, game, 0.5);
    if (attack_strength <= 0.9f)
      return;

    // set on ground to false
    // set fall distance to > 0
    // localplayer_set_on_ground(env, game, false);
    // localplayer_set_falling_distance(env, game, 0.1);
    vec3_t pos;
    localplayer_get_delta_movement(env, game, &pos);
    pos.y += 0.15;
    localplayer_set_delta_movement(env, game, &pos);
    (*env)->SetBooleanField(env, game->localplayer.instance, game->localplayer.has_impulse, true);

    while (true) {
      bool ground = (*env)->GetBooleanField(env, game->localplayer.instance, game->localplayer.on_ground);
      float dist = (*env)->GetFloatField(env, game->localplayer.instance, game->localplayer.falling_distance);
      if (!ground && dist > 0)
        break;
    }
  }

  (*env)->CallVoidMethod(env, game->minecraft.instance, game->minecraft.start_attack);
}

void attack_with_crit(JNIEnv *env, game_t *game) {}

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

  attack(env, game, menu_state);
}