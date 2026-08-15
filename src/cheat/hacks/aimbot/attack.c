#include "../../core/localplayer.h"

void attack(game_t *game) {
  float attack_strength = localplayer_get_attack_strength(game->env, game, 1.0);
  if (attack_strength < game->menu->aimbot_settings.attack_threshold)
    return;

  if (game->menu->aimbot_settings.crits) {
    attack_strength = localplayer_get_attack_strength(game->env, game, 0.5);
    if (attack_strength <= 0.9f)
      return;

    // set on ground to false
    // set fall distance to > 0
    // localplayer_set_on_ground(env, game, false);
    // localplayer_set_falling_distance(env, game, 0.1);
    vec3_t pos;
    localplayer_get_delta_movement(game->env, game, &pos);
    pos.y += 0.15;
    localplayer_set_delta_movement(game->env, game, &pos);
    (*game->env)->SetBooleanField(game->env, game->localplayer.instance, game->localplayer.has_impulse, true);

    while (true) {
      bool ground = (*game->env)->GetBooleanField(game->env, game->localplayer.instance, game->localplayer.on_ground);
      float dist =
          (*game->env)->GetFloatField(game->env, game->localplayer.instance, game->localplayer.falling_distance);
      if (!ground && dist > 0)
        break;
    }
  }

  // autoclick
  // n = 1/cps -> we have to click every n
  // if cps is 2, then n = 1/2 = 0.5, every 0.5 second we need to click in order to acheive this cps
  // n2 = n / dt -> every n2 tick we have to click

  (*game->env)->CallVoidMethod(game->env, game->minecraft.instance, game->minecraft.start_attack);
}