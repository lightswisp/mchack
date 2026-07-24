#pragma once
#include "../game.h"
#include "../utils/math.h"

jobject localplayer_get(JNIEnv *env, game_t *game);
void localplayer_get_position(JNIEnv *env, game_t *game, vec3_t *out);
void localplayer_set_viewangles(JNIEnv *env, game_t *game, vec2_t *in);
void localplayer_get_viewangles(JNIEnv *env, game_t *game, vec2_t *out);
void localplayer_get_delta_movement(JNIEnv *env, game_t *game, vec3_t *out);
void localplayer_set_delta_movement(JNIEnv *env, game_t *game, vec3_t *in);
float localplayer_get_attack_strength(JNIEnv *env, game_t *game);