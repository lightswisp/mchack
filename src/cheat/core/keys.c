#include "keys.h"

bool key_down(game_t *game, jobject key) {
  return (*game->env)->GetBooleanField(game->env, key, game->key_mapping.is_down);
}

void key_set_down(game_t *game, jobject key, bool val) {
  (*game->env)->SetBooleanField(game->env, key, game->key_mapping.is_down, val);
}

void key_click(game_t *game, jobject key) {
  (*game->env)->SetIntField(game->env, key, game->key_mapping.click_count, 5);
}