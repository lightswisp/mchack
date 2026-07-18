#pragma once
#include <jni.h>
#include "../utils/math.h"

typedef struct {
  jclass player_class;
  jclass client_level_class;
  jfieldID client_level_field;
} player_t;

jobject get_players(jint *size);
jstring get_name(jobject player, jint index);
bool get_position(jobject players, jint index, vec3_t *out);