#pragma once
#include <jni.h>

typedef struct {
  jclass mclass;
  jobject minstance;

  struct {
    jclass map_class;
    jmethodID map_clear;

    jclass list_class;
    jmethodID list_size;
    jmethodID list_get;
  } collections;

  jobject local_player;
} game_t;

bool init_game(void);
game_t *get_game(void);