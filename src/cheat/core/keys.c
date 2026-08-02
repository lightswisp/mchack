#include "keys.h"

bool key_down(JNIEnv *env, game_t *game, jobject key) {
  return (*env)->GetBooleanField(env, key, game->key_mapping.is_down);
}

void key_set_down(JNIEnv *env, game_t *game, jobject key, bool val) {
  (*env)->SetBooleanField(env, key, game->key_mapping.is_down, val);
}

void key_click(JNIEnv *env, game_t *game, jobject key) {
  (*env)->SetIntField(env, key, game->key_mapping.click_count, 1);
}