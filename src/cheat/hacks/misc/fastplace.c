#include "fastplace.h"

void fastplace(JNIEnv *env, game_t *game) {
  if (!game->menu->misc_settings.fast_place)
    return;

  (*env)->SetIntField(env, game->minecraft.instance, game->minecraft.right_click_delay, 0);
}