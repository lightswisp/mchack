#include "../rendering/gui.h"
#include "../utils/logger.h"
#include "../game.h"
#include "../jvm.h"
#include "../core/player.h"
#include <unistd.h>

void draw_esp() {
  menu_state_t *menu_state = get_menu_state();
  if (!menu_state->esp_settings.enabled)
    return;

  game_t *game = get_game();
  JNIEnv *env = get_env();
  jmethodID get_entity_renderer =
      (*env)->GetMethodID(env, game->mclass, "getEntityRenderDispatcher",
                          "()Lnet/minecraft/client/renderer/entity/EntityRenderDispatcher;");
  if (!get_entity_renderer) {
    LOGGER_LOG(FATAL, "failed to get entity renderer getter");
    return;
  }
  jobject entity_renderer_instance = (*env)->CallObjectMethod(env, game->minstance, get_entity_renderer);
  if (!entity_renderer_instance) {
    LOGGER_LOG(FATAL, "failed to get enity renderer");
    return;
  }
  jclass entity_renderer = (*env)->GetObjectClass(env, entity_renderer_instance);

  jmethodID set_render_hitboxes = (*env)->GetMethodID(env, entity_renderer, "setRenderHitBoxes", "(Z)V");
  if (!set_render_hitboxes) {
    LOGGER_LOG(FATAL, "failed to get set_render_hitboxes");
    return;
  }
  (*env)->CallVoidMethod(env, entity_renderer_instance, set_render_hitboxes, true);

  jint players_num = 0;
  jobject players = get_players(&players_num);
  if (!players) {
    LOGGER_LOG(FATAL, "failed to get players");
    return;
  }

  for (jint i = 0; i < players_num; i++) {
    jobject player = (*env)->CallObjectMethod(env, players, game->collections.list_get, i);
    LOGGER_LOG(LOG, "player %d: %p", i, player);
    sleep(2);
    (*env)->DeleteLocalRef(env, player);
  }
  (*env)->DeleteLocalRef(env, players);
  (*env)->DeleteLocalRef(env, entity_renderer_instance);
  (*env)->DeleteLocalRef(env, entity_renderer);
}