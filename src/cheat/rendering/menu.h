#pragma once
#include "draw.h"
#include "../utils/math.h"
#include <GLFW/glfw3.h>

typedef enum { IDLE, ACTIVE } state_t;
typedef enum { BIND_IDLE, BIND_REQUESTED, BIND_PROCESS, BIND_WAITING } bind_state_t;
typedef struct {
  bool set;
  bind_state_t state;
  enum { BIND_MOUSE, BIND_KEY } type;
  int as_code;
  char as_str[128];
} bind_t;

typedef struct {
  bool open;
  char text_buf[512];
  int text_len;
  enum { AIMBOT, ESP, MISC, SCRIPTS } active_tab;

  struct nk_vec2 pos;

  struct {
    float win_width;
    float win_height;

    float panel_height;
    float left_panel_widget_height;

  } resolution;

  struct {
    bool enabled;
    bind_t bind;
    bool trigger;
    bool crits;
    char smooth_time_label[32];
    float smooth_time;
    char distance_label[32];
    float distance;
    char fov_label[32];
    float fov;
    char attack_threshold_label[32];
    float attack_threshold;

    enum { BONE_HEAD, BONE_CHEST, BONE_FEET, BONE_NEAREST, BONE_SIZE } bone;

  } aimbot_settings;

  struct {
    bool enabled;

    struct {
      bool enabled;
      struct nk_colorf color;
      state_t state;
    } glow;

    struct {
      bool enabled;
      struct nk_colorf color;
      int distance;
      char distance_label[32];
      int radius;
      char radius_label[32];
    } arrows;

    struct {
      bool enabled;
      struct nk_colorf color;
    } box;

    struct {
      bool enabled;
      struct nk_colorf color;
    } snap_lines;

    struct {
      bool enabled;
    } health_bar;
  } esp_settings;

  struct {
    bool enabled;

    struct {
      bool enabled;
      state_t state;
    } fly;
    bool sprint;
    bool nofall;
    bool inf_jump;
    char speed_label[32];
    float speed;
    struct {
      bool enabled;
      state_t state;
    } fullbright;
    bool fast_place;
  } misc_settings;

  struct {
    bool enabled;
  } scripts_settings;
} menu_t;

menu_t *get_menu(void);
void init_menu(rendering_context_t *rctx);
void draw_menu(rendering_context_t *rctx);