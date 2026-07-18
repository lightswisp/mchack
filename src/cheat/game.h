#pragma once
#include <jni.h>

typedef struct {
  bool in_game;
  struct {
    jclass class;
    jobject instance;
  } minecraft;

  struct {
    bool updated;
    jclass super;
    jclass class;
    jobject instance;
    jfieldID on_ground;
    jmethodID set_sprinting;
    jmethodID is_moving;
    struct {
      jclass class;
      jobject instance;
      jfieldID mayfly;
    } abilities;
  } localplayer;

  struct {
    struct {
      jclass class;
      jmethodID clear;
    } map;

    struct {
      jclass class;
      jmethodID size;
      jmethodID get;
    } list;
  } collections;

  struct {
    jclass class;
    jobject instance;
    jmethodID set_render_hitboxes;
  } entity_renderer;

} game_t;

bool init_game(void);
game_t *get_game(void);