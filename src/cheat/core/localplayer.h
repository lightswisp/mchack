#pragma once
#include "../game.h"
#include "../utils/math.h"

jobject localplayer_get(JNIEnv *env, game_t *game);
void localplayer_get_position(JNIEnv *env, game_t *game, vec3_t *out);
void localplayer_get_lerped_position(JNIEnv *env, game_t *game, vec3_t *out);
void localplayer_set_viewangles(JNIEnv *env, game_t *game, vec2_t *in);
void localplayer_get_viewangles(JNIEnv *env, game_t *game, vec2_t *out);
void localplayer_get_viewangles2(JNIEnv *env, game_t *game, vec2_t *out);
void localplayer_get_delta_movement(JNIEnv *env, game_t *game, vec3_t *out);
void localplayer_set_delta_movement(JNIEnv *env, game_t *game, vec3_t *in);
float localplayer_get_attack_strength(JNIEnv *env, game_t *game, float scale);
void localplayer_set_on_ground(JNIEnv *env, game_t *game, bool value);
void localplayer_set_falling_distance(JNIEnv *env, game_t *game, float value);
void localplayer_set_position(JNIEnv *env, game_t *game, vec3_t *in);
bool localplayer_is_moving(JNIEnv *env, game_t *game);
void localplayer_set_sprinting_time(JNIEnv *env, game_t *game, int val);
void localplayer_set_mayfly(JNIEnv *env, game_t *game, bool val);