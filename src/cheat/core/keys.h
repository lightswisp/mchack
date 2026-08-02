#pragma once
#include "../game.h"

bool key_down(JNIEnv *env, game_t *game, jobject key);
void key_click(JNIEnv *env, game_t *game, jobject key);
void key_set_down(JNIEnv *env, game_t *game, jobject key, bool val);