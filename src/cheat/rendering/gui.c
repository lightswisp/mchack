#include "gui.h"
#include "draw.h"
#include "../game.h"
#include "../utils/logger.h"
#include <complex.h>
#include <limits.h>
#include <threads.h>
#include <unistd.h>

static menu_state_t menu_state = {.active_tab = 0,
                                  .pos = {100, 100},
                                  .aimbot_settings.enabled = false,
                                  .esp_settings.enabled = false,
                                  .misc_settings.enabled = false,
                                  .scripts_settings.enabled = false};

static void draw_aimbot_group(struct nk_context *ctx, menu_state_t *menu_state) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 2);
  menu_state->aimbot_settings.enabled = nk_check_label(ctx, "aimbot", menu_state->aimbot_settings.enabled);
  menu_state->aimbot_settings.auto_attack =
      nk_check_text_align(ctx, "auto attack", 11, menu_state->aimbot_settings.auto_attack,
                          NK_WIDGET_ALIGN_RIGHT, NK_TEXT_ALIGN_RIGHT);
  struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
  struct nk_rect space;
  nk_layout_row_dynamic(ctx, 20, 1);
  if (nk_widget(&space, ctx)) {
    nk_stroke_line(canvas, space.x, space.y + 4, space.x + space.w, space.y + 4, 2.0f, nk_rgb(49, 49, 49));
  }

  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 2);
  snprintf(menu_state->aimbot_settings.smoothness_label, sizeof(menu_state->aimbot_settings.smoothness_label),
           "smoothness: %.2f", menu_state->aimbot_settings.smoothness);
  nk_label(ctx, menu_state->aimbot_settings.smoothness_label, NK_TEXT_RIGHT);
  nk_slider_float(ctx, 0.0, &menu_state->aimbot_settings.smoothness, 1.0, 0.05);
  snprintf(menu_state->aimbot_settings.distance_label, sizeof(menu_state->aimbot_settings.distance_label),
           "distance: %.2f", menu_state->aimbot_settings.distance);
  nk_label(ctx, menu_state->aimbot_settings.distance_label, NK_TEXT_RIGHT);
  nk_slider_float(ctx, 0.0, &menu_state->aimbot_settings.distance, 10.0, 1.0);
  snprintf(menu_state->aimbot_settings.fov_label, sizeof(menu_state->aimbot_settings.fov_label), "fov: %.2f",
           menu_state->aimbot_settings.fov);
  nk_label(ctx, menu_state->aimbot_settings.fov_label, NK_TEXT_RIGHT);
  nk_slider_float(ctx, 0.0, &menu_state->aimbot_settings.fov, 360.0, 10.0);

  snprintf(menu_state->aimbot_settings.attack_threshold_label,
           sizeof(menu_state->aimbot_settings.attack_threshold_label), "threshold: %.2f",
           menu_state->aimbot_settings.attack_threshold);
  nk_label(ctx, menu_state->aimbot_settings.attack_threshold_label, NK_TEXT_RIGHT);
  nk_slider_float(ctx, 0.0, &menu_state->aimbot_settings.attack_threshold, 1.0, 0.1);

  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->aimbot_settings.trigger =
      nk_check_label(ctx, "trigger bot", menu_state->aimbot_settings.trigger);
}

static void draw_esp_group(struct nk_context *ctx, menu_state_t *menu_state) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->esp_settings.enabled = nk_check_label(ctx, "esp", menu_state->esp_settings.enabled);
  struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
  struct nk_rect space;
  nk_layout_row_dynamic(ctx, 20, 1);
  if (nk_widget(&space, ctx)) {
    nk_stroke_line(canvas, space.x, space.y + 4, space.x + space.w, space.y + 4, 2.0f, nk_rgb(49, 49, 49));
  }
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->esp_settings.arrows.enabled =
      nk_check_label(ctx, "arrows", menu_state->esp_settings.arrows.enabled);
  snprintf(menu_state->esp_settings.arrows.distance_label,
           sizeof(menu_state->esp_settings.arrows.distance_label), "arrows distance: %d",
           menu_state->esp_settings.arrows.distance);
  nk_label(ctx, menu_state->esp_settings.arrows.distance_label, NK_TEXT_LEFT);
  nk_slider_int(ctx, 1, &menu_state->esp_settings.arrows.distance, 20, 1);
  snprintf(menu_state->esp_settings.arrows.radius_label, sizeof(menu_state->esp_settings.arrows.radius_label),
           "arrows radius: %d", menu_state->esp_settings.arrows.radius);
  nk_label(ctx, menu_state->esp_settings.arrows.radius_label, NK_TEXT_LEFT);
  nk_slider_int(ctx, 1, &menu_state->esp_settings.arrows.radius, 300, 1);

  menu_state->esp_settings.glow.enabled = nk_check_label(ctx, "glow", menu_state->esp_settings.glow.enabled);
}

static void draw_misc_group(struct nk_context *ctx, menu_state_t *menu_state) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->misc_settings.enabled = nk_check_label(ctx, "misc", menu_state->misc_settings.enabled);
  struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
  struct nk_rect space;
  nk_layout_row_dynamic(ctx, 20, 1);
  if (nk_widget(&space, ctx)) {
    nk_stroke_line(canvas, space.x, space.y + 4, space.x + space.w, space.y + 4, 2.0f, nk_rgb(49, 49, 49));
  }
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 2);
  menu_state->misc_settings.fly.enabled = nk_check_label(ctx, "fly", menu_state->misc_settings.fly.enabled);
  menu_state->misc_settings.sprint = nk_check_label(ctx, "sprint", menu_state->misc_settings.sprint);
  menu_state->misc_settings.nofall = nk_check_label(ctx, "no fall", menu_state->misc_settings.nofall);
  menu_state->misc_settings.inf_jump =
      nk_check_label(ctx, "infinite jump", menu_state->misc_settings.inf_jump);
  menu_state->misc_settings.fullbright.enabled =
      nk_check_label(ctx, "full bright", menu_state->misc_settings.fullbright.enabled);
  menu_state->misc_settings.fast_place =
      nk_check_label(ctx, "fast place", menu_state->misc_settings.fast_place);
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 2);
  snprintf(menu_state->misc_settings.speed_label, sizeof(menu_state->misc_settings.speed_label),
           "speed: %.2f", menu_state->misc_settings.speed);
  nk_label(ctx, menu_state->misc_settings.speed_label, NK_TEXT_LEFT);
  nk_slider_float(ctx, 0.2, &menu_state->misc_settings.speed, 2.0, 0.1);
}

static void draw_scripts_group(struct nk_context *ctx, menu_state_t *menu_state) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->scripts_settings.enabled = nk_check_label(ctx, "scripts", menu_state->scripts_settings.enabled);
  struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
  struct nk_rect space;
  nk_layout_row_dynamic(ctx, 20, 1);
  if (nk_widget(&space, ctx)) {
    nk_stroke_line(canvas, space.x, space.y + 4, space.x + space.w, space.y + 4, 2.0f, nk_rgb(49, 49, 49));
  }
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  nk_label(ctx, "in progress...", NK_TEXT_LEFT);
}

static void draw_left_panel(struct nk_context *ctx, menu_state_t *menu_state) {
  if (nk_group_begin(ctx, "left", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, menu_state->resolution.left_panel_widget_height, 1);
    if (nk_button_label(ctx, "aimbot"))
      menu_state->active_tab = AIMBOT;
    if (nk_button_label(ctx, "esp"))
      menu_state->active_tab = ESP;
    if (nk_button_label(ctx, "misc"))
      menu_state->active_tab = MISC;
    if (nk_button_label(ctx, "scripts"))
      menu_state->active_tab = SCRIPTS;
    nk_edit_string(ctx, NK_EDIT_FIELD, menu_state->text_buf, &menu_state->text_len,
                   sizeof(menu_state->text_buf), nk_filter_default);
    nk_layout_row_dynamic(ctx, menu_state->resolution.left_panel_widget_height, 2);
    if (nk_button_label(ctx, "save")) {
    }
    if (nk_button_label(ctx, "load")) {
    }

    nk_group_end(ctx);
  }
}

static void draw_right_panel(struct nk_context *ctx, menu_state_t *menu_state) {
  if (nk_group_begin(ctx, "right", NK_WINDOW_BORDER | NK_WINDOW_SCROLL_AUTO_HIDE)) {
    switch (menu_state->active_tab) {
    case AIMBOT:
      draw_aimbot_group(ctx, menu_state);
      break;
    case ESP:
      draw_esp_group(ctx, menu_state);
      break;
    case MISC:
      draw_misc_group(ctx, menu_state);
      break;
    case SCRIPTS:
      draw_scripts_group(ctx, menu_state);
      break;
    }
    nk_group_end(ctx);
  }
}

void setup_menu_theme(struct nk_context *ctx) {
  struct nk_color background = nk_rgba(26, 26, 28, 200);
  struct nk_color element_bg = nk_rgba(49, 49, 49, 255);
  struct nk_color border_color = nk_rgba(50, 50, 50, 200);
  struct nk_color text_color = nk_rgba(220, 220, 220, 255);

  ctx->style.window.rounding = 0.0f;
  ctx->style.window.border = 1.0f;
  ctx->style.button.rounding = 0.0f;
  ctx->style.button.border = 1.0f;
  ctx->style.checkbox.border = 1.0f;

  ctx->style.window.padding = nk_vec2(16.0f, 16.0f);
  ctx->style.window.background = background;
  ctx->style.window.fixed_background = nk_style_item_color(background);
  ctx->style.window.border_color = border_color;
  ctx->style.window.group_padding = nk_vec2(16.0f, 16.0f);
  ctx->style.window.group_border = 2.0f;

  ctx->style.edit.normal = nk_style_item_color(nk_rgb(26, 26, 26));
  ctx->style.edit.active = nk_style_item_color(nk_rgb(23, 23, 23));

  ctx->style.button.normal = nk_style_item_color(element_bg);
  ctx->style.button.hover = nk_style_item_color(nk_rgba(45, 45, 45, 255));
  ctx->style.button.active = nk_style_item_color(nk_rgba(55, 55, 55, 255));
  ctx->style.button.border_color = border_color;
  ctx->style.button.text_background = background;
  ctx->style.button.text_normal = text_color;

  ctx->style.checkbox.normal = nk_style_item_color(element_bg);
  ctx->style.checkbox.hover = nk_style_item_color(element_bg);
  ctx->style.checkbox.cursor_normal = nk_style_item_color(nk_rgb(33, 142, 109));
  ctx->style.checkbox.cursor_hover = nk_style_item_color(nk_rgb(30, 140, 105));
  ctx->style.checkbox.text_normal = text_color;
  ctx->style.checkbox.spacing = 4.0f;
  ctx->style.checkbox.border_color = element_bg;
  ctx->style.checkbox.border = 0.5f;

  ctx->style.slider.cursor_size = nk_vec2(30.0, 30.0);
  ctx->style.slider.padding.x = 15.0f;
  ctx->style.slider.bar_height = 30.0f;
}

void setup_resolution(rendering_context_t *rctx) {
  float win_width = 1036.0f; // rctx->width * 0.30f;
  float win_height = 466.0f;

  LOGGER_LOG(LOG, "width: %f | height: %f", win_width, win_height);

  float panel_height = win_height - rctx->ctx->style.window.padding.y * 2;

  float left_panel_widgets = 6.0;
  float left_panel_spacing = (rctx->ctx->style.window.spacing.y * (left_panel_widgets - 1)) +
                             (rctx->ctx->style.window.group_padding.y * 2);
  float left_panel_widget_height = ((panel_height - left_panel_spacing) / left_panel_widgets);

  menu_state.resolution.win_width = win_width;
  menu_state.resolution.win_height = win_height;
  menu_state.resolution.panel_height = panel_height;
  menu_state.resolution.left_panel_widget_height = left_panel_widget_height;
}

static void draw_menu(rendering_context_t *rctx) {
  if (nk_begin(rctx->ctx, "mchack",
               nk_rect(menu_state.pos.x, menu_state.pos.y, menu_state.resolution.win_width,
                       menu_state.resolution.win_height),
               NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR)) {
    menu_state.pos = nk_window_get_position(rctx->ctx);
    float ratio[] = {0.35f, 0.65f};
    nk_layout_row(rctx->ctx, NK_DYNAMIC, menu_state.resolution.panel_height, 2, ratio);
    draw_left_panel(rctx->ctx, &menu_state);
    draw_right_panel(rctx->ctx, &menu_state);
  }
  nk_window_show(rctx->ctx, "mchack", rctx->open);
  nk_end(rctx->ctx);
}

static void draw_fov_circle(rendering_context_t *rctx) {
  if (!menu_state.aimbot_settings.enabled)
    return;
  if (menu_state.aimbot_settings.fov == 0.0f)
    return;

  game_t *game = get_game();
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);

  float aimbot_radius = deg2rad(menu_state.aimbot_settings.fov / 2.0f);
  float game_fov = game->minecraft.options.fov_val;
  float game_radius = deg2rad(game_fov / 2.0f);
  float fov_radius_pixels = (rctx->width / 2.0f) * (tanf(aimbot_radius) / tanf(game_radius));

  float diameter = fov_radius_pixels * 2;
  float center_x = rctx->width / 2.0 - (fov_radius_pixels);
  float center_y = rctx->height / 2.0 - (fov_radius_pixels);
  struct nk_rect fov_rect = nk_rect(center_x, center_y, diameter, diameter);
  nk_stroke_circle(canvas, fov_rect, 2.5f, nk_rgb(249, 39, 114));
}

static void draw_arrows(rendering_context_t *rctx) {
  if (!menu_state.esp_settings.arrows.enabled)
    return;
  if (menu_state.esp_settings.arrows.size == 0)
    return;

  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_color arrow_color = nk_rgb(249, 37, 114);
  float arrow_size = 20.0f;

  float center_x = rctx->width / 2.0;
  float center_y = rctx->height / 2.0;

  for (size_t i = 0; i < menu_state.esp_settings.arrows.size; i++) {
    float x = center_x;
    float y = center_y;
    float angle = menu_state.esp_settings.arrows.arrows2draw[i];
    float angle90 = angle - 1.5707963267948966;
    vec2_t t1[3] = {{x, y}, {x, y + arrow_size}, {x - arrow_size, y + arrow_size * 2}};
    vec2_t t2[3] = {{x, y}, {x, y + arrow_size}, {x + arrow_size, y + arrow_size * 2}};

    // first rotate
    transform_rotate(&t1[0], rctx->width, rctx->height, angle90);
    transform_rotate(&t1[1], rctx->width, rctx->height, angle90);
    transform_rotate(&t1[2], rctx->width, rctx->height, angle90);
    transform_rotate(&t2[0], rctx->width, rctx->height, angle90);
    transform_rotate(&t2[1], rctx->width, rctx->height, angle90);
    transform_rotate(&t2[2], rctx->width, rctx->height, angle90);

    // then move
    vec2_t add_factor = {cosf(angle) * menu_state.esp_settings.arrows.radius,
                         -sinf(angle) * menu_state.esp_settings.arrows.radius};
    vec2add(&t1[0], &add_factor);
    vec2add(&t1[1], &add_factor);
    vec2add(&t1[2], &add_factor);
    vec2add(&t2[0], &add_factor);
    vec2add(&t2[1], &add_factor);
    vec2add(&t2[2], &add_factor);

    nk_fill_triangle(canvas, t1[0].x, t1[0].y, t1[1].x, t1[1].y, t1[2].x, t1[2].y, arrow_color);
    nk_fill_triangle(canvas, t2[0].x, t2[0].y, t2[1].x, t2[1].y, t2[2].x, t2[2].y, arrow_color);
  }
}

static void draw_dark_background(rendering_context_t *rctx) {
  if (!rctx->open)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_rect rect = nk_rect(0, 0, rctx->width + rctx->ctx->style.window.padding.x,
                                rctx->height + rctx->ctx->style.window.padding.y);
  nk_fill_rect(canvas, rect, 0.0, nk_rgba(0, 0, 0, 150));
}

static void draw_overlay(rendering_context_t *rctx) {
  // it will be used to draw boxes, text, fov circle, etc
  struct nk_color transparent = nk_rgba(0, 0, 0, 0);
  struct nk_style_item transparent_item = nk_style_item_color(transparent);
  struct nk_style_item s1 = rctx->ctx->style.window.fixed_background;
  struct nk_color s2 = rctx->ctx->style.window.background;

  rctx->ctx->style.window.fixed_background = transparent_item;
  rctx->ctx->style.window.background = transparent;
  if (nk_begin(rctx->ctx, "overlay",
               nk_rect(-17, 0, rctx->width + rctx->ctx->style.window.padding.x * 2,
                       rctx->height + rctx->ctx->style.window.padding.y),
               NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND)) {

    draw_dark_background(rctx);
    draw_fov_circle(rctx);
    draw_arrows(rctx);
  }

  nk_end(rctx->ctx);
  rctx->ctx->style.window.fixed_background = s1;
  rctx->ctx->style.window.background = s2;
}

void draw_gui(rendering_context_t *rctx) {
  menu_state.win = rctx->win;
  draw_overlay(rctx);
  draw_menu(rctx);
}

menu_state_t *get_menu_state(void) { return &menu_state; }