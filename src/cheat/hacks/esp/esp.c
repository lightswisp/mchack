#include "../../rendering/gui.h"
#include "esp.h"
#include <unistd.h>

void esp(JNIEnv *env, game_t *game) {
  menu_state_t *menu_state = get_menu_state();
  if (!menu_state->esp_settings.enabled)
    return;
  if (!game->in_game)
    return;

  //(*env)->CallVoidMethod(env, entity_renderer_instance, set_render_hitboxes, true);

  // jint players_num = 0;
  // jobject players = get_players(&players_num);
  // if (!players) {
  //   LOGGER_LOG(FATAL, "failed to get players");
  //   return;
  // }

  // for (jint i = 0; i < players_num; i++) {
  //   jobject player = (*env)->CallObjectMethod(env, players, game->collections.list_get, i);
  //   LOGGER_LOG(LOG, "player %d: %p", i, player);
  //   sleep(2);
  //   (*env)->DeleteLocalRef(env, player);
  // }
  //(*env)->DeleteLocalRef(env, players);
  //(*env)->DeleteLocalRef(env, entity_renderer_instance);
  //(*env)->DeleteLocalRef(env, entity_renderer);
}