#pragma once
#include "../game.h"
#include "../rendering/gui.h"
#include "../rendering/overlay.h"

void update(JNIEnv *env, game_t *game, menu_state_t *menu_state, overlay_t *overlay);
void reset_updates(game_t *game, menu_state_t *menu_state, overlay_t *overlay);