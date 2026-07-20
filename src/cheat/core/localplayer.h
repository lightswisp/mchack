#pragma once
#include "../game.h"
#include "../utils/math.h"

jobject localplayer_get(JNIEnv *env, game_t *game);
void localplayer_get_position(jobject player, vec3_t *out);
void localplayer_set_viewangles(jobject player, vec2_t *angles);
void localplayer_get_viewangles(jobject player, vec2_t *out);