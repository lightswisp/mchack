#pragma once
#include <jni.h>
#include "../utils/math.h"
#include "../game.h"

bool player_is_localplayer(JNIEnv *env, game_t *game, jobject player);
jstring player_get_name(jobject player);
void player_get_position(JNIEnv *env, game_t *game, jobject player, vec3_t *out);
void player_get_lerped_position(JNIEnv *env, game_t *game, jobject player, vec3_t *out);
void player_get_bb(JNIEnv *env, game_t *game, jobject player, vec_bb_t *out);
void player_set_team_color(JNIEnv *env, game_t *game, jobject player, int color);
int player_get_team_color(JNIEnv *env, game_t *game, jobject player);
void player_set_glowing(JNIEnv *env, game_t *game, jobject player, bool val);
float player_get_eye_height(JNIEnv *env, game_t *game, jobject player);
float player_get_height(JNIEnv *env, game_t *game, jobject player);
float player_get_width(JNIEnv *env, game_t *game, jobject player);
float player_get_health(JNIEnv *env, game_t *game, jobject player);