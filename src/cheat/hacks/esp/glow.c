#include "glow.h"
#include "../../core/player.h"

void glow(JNIEnv *env, game_t *game, menu_state_t *menu_state) {

  if (!menu_state->esp_settings.glow.enabled) {
    if (menu_state->esp_settings.glow.state == ACTIVE) {
      for (jint i = 0; i < game->players.size; i++) {
        jobject player = (*env)->CallObjectMethod(env, game->players.instance, game->collections.list.get, i);
        if (player_is_localplayer(player))
          continue;

        (*env)->SetBooleanField(env, player, game->players.glowing, false);
      }
      menu_state->esp_settings.glow.state = IDLE;
    }
    return;
  }

  for (jint i = 0; i < game->players.size; i++) {
    jobject player = (*env)->CallObjectMethod(env, game->players.instance, game->collections.list.get, i);
    if (player_is_localplayer(player))
      continue;

    (*env)->SetBooleanField(env, player, game->players.glowing, true);
  }

  menu_state->esp_settings.glow.state = ACTIVE;
}