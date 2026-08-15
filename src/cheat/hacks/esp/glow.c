#include "glow.h"
#include "../../core/player.h"

void glow(game_t *game) {

  if (!game->menu->esp_settings.glow.enabled) {
    if (game->menu->esp_settings.glow.state == ACTIVE) {
      for (jint i = 0; i < game->players.size; i++) {
        jobject player =
            (*game->env)->CallObjectMethod(game->env, game->players.instance, game->collections.list.get, i);
        if (!player)
          continue;
        if (player_is_localplayer(game->env, game, player))
          continue;

        (*game->env)->SetBooleanField(game->env, player, game->players.glowing, false);
      }
      game->menu->esp_settings.glow.state = IDLE;
    }
    return;
  }

  for (jint i = 0; i < game->players.size; i++) {
    jobject player = (*game->env)->CallObjectMethod(game->env, game->players.instance, game->collections.list.get, i);
    if (!player)
      continue;
    if (player_is_localplayer(game->env, game, player))
      continue;

    (*game->env)->SetBooleanField(game->env, player, game->players.glowing, true);
  }

  game->menu->esp_settings.glow.state = ACTIVE;
}