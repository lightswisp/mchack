#include "menu.h"
#include "draw.h"
#include "../utils/logger.h"
#include <complex.h>
#include <limits.h>
#include <threads.h>
#include <unistd.h>
#include <stdio.h>

static menu_t menu = {
    .open = false,
    .active_tab = 0,
    .pos = {100, 100},
    .aimbot_settings.enabled = false,
    .esp_settings.enabled = false,
    .misc_settings.enabled = false,
    .scripts_settings.enabled = false,
};

static void draw_spacer(struct nk_context *ctx, int margin, int height) {
  nk_layout_row_dynamic(ctx, margin, 1);
  nk_spacing(ctx, 1);
  nk_layout_row_dynamic(ctx, height, 1);
  nk_rule_horizontal(ctx, nk_rgb(49, 49, 49), false);
  nk_layout_row_dynamic(ctx, margin, 1);
  nk_spacing(ctx, 1);
}

static void draw_bind_button(struct nk_context *ctx, bind_t *bind) {
  if (bind->set) {
    if (bind->state == BIND_WAITING) {
      nk_button_label(ctx, "waiting..");
    } else {
      if (nk_button_label(ctx, bind->as_str)) {
        // rebind
        // main logic will be in handle_input()
        // we are just changing the state
        LOGGER_LOG(LOG, "bind request for rebind!");
        bind->state = BIND_REQUESTED;
      }
    }
  } else {
    if (bind->state == BIND_WAITING) {
      nk_button_label(ctx, "waiting..");
    } else {
      if (nk_button_label(ctx, "set bind")) {
        // set bind
        // main logic will be in handle_input()
        // we are just changing the state
        LOGGER_LOG(LOG, "bind request when not set!");
        bind->state = BIND_REQUESTED;
      }
    }
  }
}

static void draw_color_picker(struct nk_context *ctx, struct nk_colorf *color) {
  if (nk_combo_begin_color(ctx, nk_rgb_cf(*color), nk_vec2(400, 400))) {
    nk_layout_row_dynamic(ctx, 300, 1);
    nk_color_pick(ctx, color, NK_RGBA);
    nk_combo_end(ctx);
  }
}

static void draw_aimbot_group(struct nk_context *ctx) {
  nk_layout_row(ctx, NK_DYNAMIC, 0, 2, (float[]){0.7, 0.3});
  menu.aimbot_settings.enabled = nk_check_label(ctx, "aimbot", menu.aimbot_settings.enabled);

  draw_bind_button(ctx, &menu.aimbot_settings.bind);

  draw_spacer(ctx, 3, 5);

  nk_layout_row(ctx, NK_DYNAMIC, 0, 2, (float[]){0.2, 0.8});
  nk_label(ctx, "bone", NK_TEXT_LEFT);
  const char *items[] = {"head", "chest", "feet", "nearest"};

  if (nk_combo_begin_label(ctx, items[menu.aimbot_settings.bone], nk_vec2(nk_widget_width(ctx), 200))) {
    nk_layout_row_dynamic(ctx, 0, 1);
    for (int i = 0; i < BONE_SIZE; ++i)
      if (nk_combo_item_label(ctx, items[i], NK_TEXT_CENTERED))
        menu.aimbot_settings.bone = i;
    nk_combo_end(ctx);
  }

  nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[]){0.3, 0.15, 0.55});
  snprintf(menu.aimbot_settings.smooth_time_label, sizeof(menu.aimbot_settings.smooth_time_label), "%.2f",
           menu.aimbot_settings.smooth_time);
  nk_label(ctx, "smooth", NK_TEXT_LEFT);
  nk_label_colored(ctx, menu.aimbot_settings.smooth_time_label, NK_TEXT_ALIGN_MIDDLE, nk_rgb(255, 255, 255));
  nk_slider_float(ctx, 0.0, &menu.aimbot_settings.smooth_time, 5.0, 0.1);

  snprintf(menu.aimbot_settings.distance_label, sizeof(menu.aimbot_settings.distance_label), "%.2f",
           menu.aimbot_settings.distance);
  nk_label(ctx, "distance", NK_TEXT_LEFT);
  nk_label_colored(ctx, menu.aimbot_settings.distance_label, NK_TEXT_ALIGN_MIDDLE, nk_rgb(255, 255, 255));
  nk_slider_float(ctx, 0.0, &menu.aimbot_settings.distance, 10.0, 1.0);

  snprintf(menu.aimbot_settings.fov_label, sizeof(menu.aimbot_settings.fov_label), "%.2f", menu.aimbot_settings.fov);
  nk_label(ctx, "fov", NK_TEXT_LEFT);
  nk_label_colored(ctx, menu.aimbot_settings.fov_label, NK_TEXT_ALIGN_MIDDLE, nk_rgb(255, 255, 255));
  nk_slider_float(ctx, 0.0, &menu.aimbot_settings.fov, 360.0, 5);

  snprintf(menu.aimbot_settings.attack_threshold_label, sizeof(menu.aimbot_settings.attack_threshold_label), "%.2f",
           menu.aimbot_settings.attack_threshold);
  nk_label(ctx, "threshold", NK_TEXT_LEFT);
  nk_label_colored(ctx, menu.aimbot_settings.attack_threshold_label, NK_TEXT_ALIGN_MIDDLE, nk_rgb(255, 255, 255));
  nk_slider_float(ctx, 0.0, &menu.aimbot_settings.attack_threshold, 1.0, 0.1);

  draw_spacer(ctx, 3, 5);

  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 2);
  menu.aimbot_settings.trigger = nk_check_label(ctx, "trigger bot", menu.aimbot_settings.trigger);
  menu.aimbot_settings.crits = nk_check_text_align(ctx, "crits", sizeof("crits") - 1, menu.aimbot_settings.crits,
                                                   NK_WIDGET_ALIGN_RIGHT, NK_TEXT_ALIGN_RIGHT);
}

static void draw_esp_group(struct nk_context *ctx) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu.esp_settings.enabled = nk_check_label(ctx, "esp", menu.esp_settings.enabled);

  draw_spacer(ctx, 3, 5);

  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu.esp_settings.arrows.enabled = nk_check_label(ctx, "arrows", menu.esp_settings.arrows.enabled);
  snprintf(menu.esp_settings.arrows.distance_label, sizeof(menu.esp_settings.arrows.distance_label),
           "arrows distance: %d", menu.esp_settings.arrows.distance);
  nk_label(ctx, menu.esp_settings.arrows.distance_label, NK_TEXT_LEFT);
  nk_slider_int(ctx, 1, &menu.esp_settings.arrows.distance, 20, 1);
  snprintf(menu.esp_settings.arrows.radius_label, sizeof(menu.esp_settings.arrows.radius_label), "arrows radius: %d",
           menu.esp_settings.arrows.radius);
  nk_label(ctx, menu.esp_settings.arrows.radius_label, NK_TEXT_LEFT);
  nk_slider_int(ctx, 1, &menu.esp_settings.arrows.radius, 300, 1);

  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 2);
  menu.esp_settings.glow.enabled = nk_check_text_align(ctx, "glow", sizeof("glow") - 1, menu.esp_settings.glow.enabled,
                                                       NK_WIDGET_ALIGN_RIGHT, NK_TEXT_ALIGN_LEFT);
  menu.esp_settings.box.enabled = nk_check_text_align(ctx, "boxes", sizeof("boxes") - 1, menu.esp_settings.box.enabled,
                                                      NK_WIDGET_ALIGN_RIGHT, NK_TEXT_ALIGN_LEFT);
  draw_color_picker(ctx, &menu.esp_settings.glow.color);
  draw_color_picker(ctx, &menu.esp_settings.box.color);

  menu.esp_settings.health_bar.enabled =
      nk_check_text_align(ctx, "health", sizeof("health") - 1, menu.esp_settings.health_bar.enabled,
                          NK_WIDGET_ALIGN_RIGHT, NK_TEXT_ALIGN_LEFT);
  menu.esp_settings.snap_lines.enabled =
      nk_check_text_align(ctx, "snap lines", sizeof("snap lines") - 1, menu.esp_settings.snap_lines.enabled,
                          NK_WIDGET_ALIGN_RIGHT, NK_TEXT_ALIGN_LEFT);
  nk_spacing(ctx, 1);
  draw_color_picker(ctx, &menu.esp_settings.snap_lines.color);
}

static void draw_misc_group(struct nk_context *ctx) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu.misc_settings.enabled = nk_check_label(ctx, "misc", menu.misc_settings.enabled);

  draw_spacer(ctx, 3, 5);

  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 2);
  menu.misc_settings.fly.enabled = nk_check_label(ctx, "fly", menu.misc_settings.fly.enabled);
  menu.misc_settings.sprint = nk_check_label(ctx, "sprint", menu.misc_settings.sprint);
  menu.misc_settings.nofall = nk_check_label(ctx, "no fall", menu.misc_settings.nofall);
  menu.misc_settings.inf_jump = nk_check_label(ctx, "infinite jump", menu.misc_settings.inf_jump);
  menu.misc_settings.fullbright.enabled = nk_check_label(ctx, "full bright", menu.misc_settings.fullbright.enabled);
  menu.misc_settings.fast_place = nk_check_label(ctx, "fast place", menu.misc_settings.fast_place);
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 2);
  snprintf(menu.misc_settings.speed_label, sizeof(menu.misc_settings.speed_label), "speed: %.2f",
           menu.misc_settings.speed);
  nk_label(ctx, menu.misc_settings.speed_label, NK_TEXT_LEFT);
  nk_slider_float(ctx, 0.2, &menu.misc_settings.speed, 2.0, 0.1);
}

static void draw_scripts_group(struct nk_context *ctx) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu.scripts_settings.enabled = nk_check_label(ctx, "scripts", menu.scripts_settings.enabled);

  draw_spacer(ctx, 3, 5);

  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  nk_label(ctx, "in progress...", NK_TEXT_LEFT);
}

static void draw_left_panel(struct nk_context *ctx) {
  if (nk_group_begin(ctx, "left", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, menu.resolution.left_panel_widget_height, 1);
    if (nk_button_label(ctx, "aimbot"))
      menu.active_tab = AIMBOT;
    if (nk_button_label(ctx, "esp"))
      menu.active_tab = ESP;
    if (nk_button_label(ctx, "misc"))
      menu.active_tab = MISC;
    if (nk_button_label(ctx, "scripts"))
      menu.active_tab = SCRIPTS;
    nk_edit_string(ctx, NK_EDIT_FIELD, menu.text_buf, &menu.text_len, sizeof(menu.text_buf), nk_filter_default);
    nk_layout_row_dynamic(ctx, menu.resolution.left_panel_widget_height, 2);
    if (nk_button_label(ctx, "save")) {
    }
    if (nk_button_label(ctx, "load")) {
    }

    nk_group_end(ctx);
  }
}

static void draw_right_panel(struct nk_context *ctx) {
  if (nk_group_begin(ctx, "right", NK_WINDOW_BORDER | NK_WINDOW_SCROLL_AUTO_HIDE)) {
    switch (menu.active_tab) {
    case AIMBOT:
      draw_aimbot_group(ctx);
      break;
    case ESP:
      draw_esp_group(ctx);
      break;
    case MISC:
      draw_misc_group(ctx);
      break;
    case SCRIPTS:
      draw_scripts_group(ctx);
      break;
    }
    nk_group_end(ctx);
  }
}

static void setup_menu_theme(rendering_context_t *rctx) {
  struct nk_color background = nk_rgba(26, 26, 28, 200);
  struct nk_color element_bg = nk_rgba(49, 49, 49, 255);
  struct nk_color border_color = nk_rgba(50, 50, 50, 200);
  struct nk_color text_color = nk_rgba(220, 220, 220, 255);

  rctx->ctx->style.window.rounding = 0.0f;
  rctx->ctx->style.window.border = 1.0f;
  rctx->ctx->style.button.rounding = 0.0f;
  rctx->ctx->style.button.border = 1.0f;
  rctx->ctx->style.checkbox.border = 1.0f;

  rctx->ctx->style.window.padding = nk_vec2(16.0f, 16.0f);
  rctx->ctx->style.window.background = background;
  rctx->ctx->style.window.fixed_background = nk_style_item_color(background);
  rctx->ctx->style.window.border_color = border_color;
  rctx->ctx->style.window.group_padding = nk_vec2(16.0f, 16.0f);
  rctx->ctx->style.window.group_border = 2.0f;

  rctx->ctx->style.edit.normal = nk_style_item_color(nk_rgb(26, 26, 26));
  rctx->ctx->style.edit.active = nk_style_item_color(nk_rgb(23, 23, 23));

  rctx->ctx->style.button.normal = nk_style_item_color(element_bg);
  rctx->ctx->style.button.hover = nk_style_item_color(nk_rgba(45, 45, 45, 255));
  rctx->ctx->style.button.active = nk_style_item_color(nk_rgba(55, 55, 55, 255));
  rctx->ctx->style.button.border_color = border_color;
  rctx->ctx->style.button.text_background = background;
  rctx->ctx->style.button.text_normal = text_color;

  rctx->ctx->style.checkbox.normal = nk_style_item_color(element_bg);
  rctx->ctx->style.checkbox.hover = nk_style_item_color(element_bg);
  rctx->ctx->style.checkbox.cursor_normal = nk_style_item_color(nk_rgb(33, 142, 109));
  rctx->ctx->style.checkbox.cursor_hover = nk_style_item_color(nk_rgb(30, 140, 105));
  rctx->ctx->style.checkbox.text_normal = text_color;
  rctx->ctx->style.checkbox.spacing = 4.0f;
  rctx->ctx->style.checkbox.border_color = element_bg;
  rctx->ctx->style.checkbox.border = 0.5f;

  rctx->ctx->style.slider.cursor_size = nk_vec2(30.0, 30.0);
  rctx->ctx->style.slider.padding.x = 15.0f;
  rctx->ctx->style.slider.bar_height = 30.0f;
}

static void setup_resolution(rendering_context_t *rctx) {
  float win_width = 1036.0f; // rctx->width * 0.30f;
  float win_height = 466.0f;

  LOGGER_LOG(LOG, "width: %f | height: %f", win_width, win_height);

  float panel_height = win_height - rctx->ctx->style.window.padding.y * 2;

  float left_panel_widgets = 6.0;
  float left_panel_spacing =
      (rctx->ctx->style.window.spacing.y * (left_panel_widgets - 1)) + (rctx->ctx->style.window.group_padding.y * 2);
  float left_panel_widget_height = ((panel_height - left_panel_spacing) / left_panel_widgets);

  menu.resolution.win_width = win_width;
  menu.resolution.win_height = win_height;
  menu.resolution.panel_height = panel_height;
  menu.resolution.left_panel_widget_height = left_panel_widget_height;
}

void init_menu(rendering_context_t *rctx) {
  setup_menu_theme(rctx);
  setup_resolution(rctx);
}

void draw_menu(rendering_context_t *rctx) {

  if (nk_begin(rctx->ctx, "mchack",
               nk_rect(menu.pos.x, menu.pos.y, menu.resolution.win_width, menu.resolution.win_height),
               NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR)) {
    menu.pos = nk_window_get_position(rctx->ctx);
    float ratio[] = {0.35f, 0.65f};
    nk_layout_row(rctx->ctx, NK_DYNAMIC, menu.resolution.panel_height, 2, ratio);
    draw_left_panel(rctx->ctx);
    draw_right_panel(rctx->ctx);
  }
  nk_window_show(rctx->ctx, "mchack", menu.open);
  if (menu.open) {
    nk_style_show_cursor(rctx->ctx);
  }

  nk_end(rctx->ctx);
}

menu_t *get_menu(void) { return &menu; }