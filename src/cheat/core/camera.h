#pragma once
#include "../game.h"
#include "../utils/math.h"

void camera_get_position(JNIEnv *env, game_t *game, vec3_t *out);
void camera_get_viewangles_deg(JNIEnv *env, game_t *game, vec2_t *out);
void camera_get_viewangles_rad(JNIEnv *env, game_t *game, vec2_t *out);
void camera_set_viewangles(JNIEnv *env, game_t *game, vec2_t *in);