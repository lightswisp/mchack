#pragma once
#include "../game.h"
#include "../rendering/gui.h"

void update(JNIEnv *env, game_t *game, menu_state_t *menu_state);
void reset_updates(game_t *game, menu_state_t *menu_state);