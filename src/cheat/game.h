#pragma once
#include "rendering/menu.h"
#include "rendering/esp.h"
#include "rendering/overlay.h"
#include <jni.h>

#define CHECK(a)                                                                                                       \
  do {                                                                                                                 \
    if (!(a)) {                                                                                                        \
      LOGGER_LOG(FATAL, "%s is NULL", #a);                                                                             \
      return false;                                                                                                    \
    }                                                                                                                  \
  } while (0)

#define CHECK_BOOL(a)                                                                                                  \
  do {                                                                                                                 \
    if ((a) == false) {                                                                                                \
      LOGGER_LOG(FATAL, "%s has failed", #a);                                                                          \
      return false;                                                                                                    \
    }                                                                                                                  \
  } while (0)

#define DELETE_REF(env, object)                                                                                        \
  do {                                                                                                                 \
    if (object)                                                                                                        \
      (*env)->DeleteLocalRef(env, object);                                                                             \
  } while (0)

typedef struct {
  JNIEnv *env;
  menu_t *menu;
  overlay_t *overlay;
  rendering_context_t *rctx;
  esp_t *esp;
  bool in_game;
  bool paused;
  float update_time;

  struct {
    jclass class;
    jobject instance;
    jmethodID start_attack;
    jmethodID get_frame_time;
    jobject crosshair_pick_entity;
    jfloat partial_tick;
    jfieldID localplayer;
    jfieldID crosshair_field;
    jfieldID game_renderer_field;
    jfieldID client_level_field;
    jfieldID right_click_delay;

    struct {
      jclass class;
      jobject instance;
      jfieldID fov;
      jfloat fov_val;
      jfieldID gamma;
      jobject key_sprint;
      jobject key_attack;
      jobject key_up;
    } options;

  } minecraft;

  struct {
    jclass class;
    jfieldID is_down;     // bool
    jfieldID click_count; // int -> set it to n number of clicks to register clicks
  } key_mapping;

  struct {
    jclass class;
    jobject instance;
    jfieldID fov;
    jfieldID camera_field;
    jfloat fov_val;
  } game_renderer;

  struct {
    jclass class;
    jobject instance;
    jfieldID x_rot;
    jfieldID y_rot;
    jfieldID position_field;
    jobject position;
  } camera;

  struct {
    jclass class;
    jobject instance;
    jfieldID players_field;
  } client_level;

  struct {
    bool updated;
    jclass super;
    jclass class;
    jobject instance;
    jfieldID abilities_field;
    jfieldID on_ground;
    jfieldID x_rot;
    jfieldID y_rot;
    jfieldID x_rot_old;
    jfieldID y_rot_old;
    jfieldID delta_movement;
    jfieldID falling_distance;
    jfieldID has_impulse;
    jfieldID x_old;
    jfieldID y_old;
    jfieldID z_old;
    jfieldID sprint_trigger_time;
    jfieldID tick_count_field;
    jfieldID input;
    jfieldID is_using_item;
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
    jmethodID tick;
    jmethodID get_height;

    jint tick_count;

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
    jfieldID bounding_box;

    jmethodID get_height;
    jmethodID get_width;
    jmethodID is_local_player;
    jmethodID get_x;
    jmethodID get_y;
    jmethodID get_z;
    jmethodID get_health;
  } players;

  struct {
    jclass class;
    jfieldID left_impulse;
    jfieldID forward_impulse;
  } input;

  struct {
    jclass class;
    jfieldID min_x;
    jfieldID min_y;
    jfieldID min_z;
    jfieldID max_x;
    jfieldID max_y;
    jfieldID max_z;
  } bounding_box;

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
bool set_game_hooks(void);
bool remove_game_hooks(void);
void deinit_game(void);
game_t *get_game(void);