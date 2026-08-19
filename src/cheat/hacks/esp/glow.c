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

        player_set_glowing(game->env, game, player, false);
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

    struct nk_color c = nk_rgb_cf(game->menu->esp_settings.glow.color);
    int color = (c.r << 16 | c.g << 8 | c.b);
    if (player_get_team_color(game->env, game, player) != color) {
      player_set_team_color(game->env, game, player, color);
    }
    player_set_glowing(game->env, game, player, true);
  }

  game->menu->esp_settings.glow.state = ACTIVE;
}