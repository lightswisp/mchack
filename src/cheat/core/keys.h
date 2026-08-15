#pragma once
#include "../game.h"

bool key_down(game_t *game, jobject key);
void key_click(game_t *game, jobject key);
void key_set_down(game_t *game, jobject key, bool val);