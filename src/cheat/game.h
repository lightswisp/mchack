#pragma once
#include <jni.h>

typedef struct {
  bool in_game;
  bool paused;

  struct {
    jclass class;
    jobject instance;
    jmethodID pause_game;
    jmethodID set_screen;
    jmethodID start_attack;

    struct {
      jclass class;
      jfieldID buttons;
      jfieldID title;
    } screen;

    struct {
      jclass class;
      jobject instance;
      jmethodID grab_mouse;
    } mouse_handler;

  } minecraft;

  struct {
    bool updated;
    jclass super;
    jclass class;
    jobject instance;
    jfieldID on_ground;
    jfieldID x_rot;
    jfieldID y_rot;
    jmethodID set_sprinting;
    jmethodID is_moving;
    jmethodID display_client_message;
    jmethodID get_x;
    jmethodID get_y;
    jmethodID get_z;
    struct {
      jclass class;
      jobject instance;
      jfieldID mayfly;
    } abilities;
  } localplayer;

  struct {
    bool updated;
    jclass class;
    jobject instance;
    jint size;

    jmethodID is_local_player;
    jmethodID get_x;
    jmethodID get_y;
    jmethodID get_z;
  } players;

  struct {
    jclass class;
    jmethodID constructor;
  } text_component;

  struct {
    struct {
      jclass class;
      jmethodID clear;
    } map;

    struct {
      jclass class;
      jmethodID size;
      jmethodID get;
      jmethodID clear;
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