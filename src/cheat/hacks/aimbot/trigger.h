#pragma once
#include "../../game.h"
#include "../../rendering/gui.h"

#define MAX_ATTACK_DISTANCE (3.34F)

void trigger(JNIEnv *env, game_t *game, menu_state_t *menu_state);
void attack(JNIEnv *env, game_t *game);
