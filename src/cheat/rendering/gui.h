#pragma once
#include "draw.h"
#include "../utils/math.h"
#include <GLFW/glfw3.h>

typedef enum { IDLE, ACTIVE } state_t;

typedef struct {
  GLFWwindow *win;
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
    bool auto_attack;
    char smoothness_label[32];
    float smoothness;
    char distance_label[32];
    float distance;
    char fov_label[32];
    float fov;
  } aimbot_settings;

  struct {
    bool enabled;

    struct {
      bool enabled;
      state_t state;
    } glow;

    struct {
      bool enabled;
      int distance;
      char distance_label[32];
      int radius;
      char radius_label[32];
      float arrows2draw[255];
      size_t size;
    } arrows;
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
  } misc_settings;

  struct {
    bool enabled;
  } scripts_settings;
} menu_state_t;

menu_state_t *get_menu_state(void);
void setup_menu_theme(struct nk_context *ctx);
void setup_resolution(rendering_context_t *rctx);
void draw_gui(rendering_context_t *rctx);