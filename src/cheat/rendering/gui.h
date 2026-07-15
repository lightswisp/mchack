#pragma once
#include "draw.h"

typedef struct {
  char text_buf[512];
  int text_len;
  enum { AIMBOT, ESP, MISC, SCRIPTS } active_tab;

  struct {
    float win_width;
    float win_height;

    float panel_height;
    float left_panel_widget_height;

  } resolution;

  struct {
    bool enabled;
  } aimbot_settings;

  struct {
    bool enabled;
  } esp_settings;

  struct {
    bool enabled;
  } misc_settings;

  struct {
    bool enabled;
  } scripts_settings;
} menu_state_t;

void setup_menu_theme(struct nk_context *ctx);
void setup_resolution(rendering_context_t *rctx);
void draw_gui(rendering_context_t *rctx);