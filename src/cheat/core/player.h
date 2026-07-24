#pragma once
#include <jni.h>
#include "../utils/math.h"
#include "../game.h"

typedef struct {
  jclass player_class;
  jclass client_level_class;
  jfieldID client_level_field;
} player_t;

bool player_is_localplayer(JNIEnv *env, game_t *game, jobject player);
jstring player_get_name(jobject player);
void player_get_position(JNIEnv *env, game_t *game, jobject player, vec3_t *out);