#pragma once
#include "../game.h"
#include "../utils/math.h"

jobject localplayer_get(JNIEnv *env, game_t *game);
void localplayer_get_position(JNIEnv *env, game_t *game, vec3_t *out);
void localplayer_get_lerped_position(JNIEnv *env, game_t *game, vec3_t *out);
void localplayer_set_viewangles(JNIEnv *env, game_t *game, vec2_t *in);
void localplayer_set_lerped_viewangles(JNIEnv *env, game_t *game, vec2_t *in);
void localplayer_get_viewangles(JNIEnv *env, game_t *game, vec2_t *out);
void localplayer_get_viewangles_rad(JNIEnv *env, game_t *game, vec2_t *out);
void localplayer_get_delta_movement(JNIEnv *env, game_t *game, vec3_t *out);
void localplayer_set_delta_movement(JNIEnv *env, game_t *game, vec3_t *in);
float localplayer_get_attack_strength(JNIEnv *env, game_t *game, float scale);
float localplayer_get_height(JNIEnv *env, game_t *game);
void localplayer_set_on_ground(JNIEnv *env, game_t *game, bool value);
void localplayer_set_falling_distance(JNIEnv *env, game_t *game, float value);
void localplayer_set_position(JNIEnv *env, game_t *game, vec3_t *in);
bool localplayer_is_moving(JNIEnv *env, game_t *game);
void localplayer_set_sprinting_time(JNIEnv *env, game_t *game, int val);
void localplayer_set_mayfly(JNIEnv *env, game_t *game, bool val);
bool localplayer_is_using_item(JNIEnv *env, game_t *game);

// called from hook
void localplayer_set_mayfly2(JNIEnv *env, jobject this, game_t *game, bool val);
void localplayer_set_on_ground2(JNIEnv *env, jobject this, game_t *game, bool value);
void localplayer_get_delta_movement2(JNIEnv *env, jobject this, game_t *game, vec3_t *out);
void localplayer_set_delta_movement2(JNIEnv *env, jobject this, game_t *game, vec3_t *in);
void localplayer_get_viewangles_rad2(JNIEnv *env, jobject this, game_t *game, vec2_t *out);
void localplayer_set_sprinting2(JNIEnv *env, jobject this, game_t *game);
bool localplayer_get_on_ground2(JNIEnv *env, jobject this, game_t *game);
void localplayer_set_impulse2(JNIEnv *env, jobject this, game_t *game, vec2_t *impulse);