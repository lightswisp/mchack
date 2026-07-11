#pragma once
#include <jni.h>

typedef struct {
  jclass mclass;
  jobject minstance;
  jobject local_player;
} game_t;

bool init_game(void);
game_t *get_game(void);