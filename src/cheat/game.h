#pragma once
#include <jni.h>

typedef struct {
  bool in_game;
  bool paused;

  struct {
    jclass class;
    jobject instance;
    jmethodID start_attack;
    jobject crosshair_pick_entity;
    jfloat partial_tick;

    struct {
      jclass class;
      jobject instance;
      jfieldID fov;
      jfloat fov_val;
      jfieldID gamma;
    } options;

    struct {
      jclass class;
      jobject instance;
      jfieldID fov;
      jfloat fov_val;

      struct {
        jclass class;
        jobject instance;
        jfieldID x_rot;
        jfieldID y_rot;
        jobject position;
      } camera;
    } game_renderer;

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
    jfieldID falling_distance;
    jfieldID has_impulse;
    jmethodID set_sprinting;
    jmethodID is_moving;
    jmethodID display_client_message;
    jmethodID get_x;
    jmethodID get_y;
    jmethodID get_z;
    jmethodID set_delta_movement;
    jmethodID get_delta_movement;
    jmethodID get_attack_strength_scale;
    jmethodID set_pos_raw;

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
    jfieldID glowing;
    jfieldID x_old;
    jfieldID y_old;
    jfieldID z_old;
    jfieldID eye_height;

    jmethodID get_height;
    jmethodID get_width;
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