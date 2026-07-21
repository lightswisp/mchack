#pragma once
#include <jni.h>

typedef struct {
  bool in_game;
  bool paused;

  struct {
    jclass class;
    jobject instance;
    jmethodID start_attack;

    struct {
      jclass class;
      jobject instance;
      jfieldID fov;
      jfloat fov_val;
    } options;

  } minecraft;

  struct {
    bool updated;
    jclass super;
    jclass class;
    jobject instance;
    jfieldID on_ground;
    jfieldID x_rot;
    jfieldID y_rot;
    jfieldID delta_movement;
    jmethodID set_sprinting;
    jmethodID is_moving;
    jmethodID display_client_message;
    jmethodID get_x;
    jmethodID get_y;
    jmethodID get_z;
    jmethodID set_delta_movement;
    jmethodID get_delta_movement;

    struct {
      jclass class;
      jobject instance;
      jfieldID mayfly;
      jfieldID walking_speed;
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

    struct {
      jclass class;
      jfieldID x;
      jfieldID y;
      jfieldID z;
    } vec3;
  } collections;

  struct {
    jclass class;
    jobject instance;
    jmethodID set_render_hitboxes;
  } entity_renderer;

} game_t;

bool init_game(void);
game_t *get_game(void);