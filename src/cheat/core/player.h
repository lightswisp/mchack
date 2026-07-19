#pragma once
#include <jni.h>
#include "../utils/math.h"

typedef struct {
  jclass player_class;
  jclass client_level_class;
  jfieldID client_level_field;
} player_t;

bool player_is_localplayer(jobject player);
jstring player_get_name(jobject player);
void player_get_position(jobject player, vec3_t *out);