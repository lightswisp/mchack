#include "trigger.h"
#include "../../core/localplayer.h"
#include "../../core/player.h"
#include "attack.h"

void trigger(game_t *game) {
  if (!game->menu->aimbot_settings.trigger)
    return;
  if (game->players.size < 2)
    return;
  if (!game->minecraft.crosshair_pick_entity)
    return;
  if (!(*game->env)->IsInstanceOf(game->env, game->minecraft.crosshair_pick_entity, game->players.class))
    return;

  vec3_t localplayer_position;
  vec3_t player_position;

  localplayer_get_position(game->env, game, &localplayer_position);
  player_get_position(game->env, game, game->minecraft.crosshair_pick_entity, &player_position);

  attack(game);
}