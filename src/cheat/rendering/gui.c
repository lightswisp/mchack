#include "gui.h"
#include "draw.h"
#include <limits.h>

static menu_state_t menu_state = {.active_tab = 0,
                                  .aimbot_settings.enabled = false,
                                  .esp_settings.enabled = false,
                                  .misc_settings.enabled = false,
                                  .scripts_settings.enabled = false};

static void draw_aimbot_group(struct nk_context *ctx, menu_state_t *menu_state) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->aimbot_settings.enabled = nk_check_label(ctx, "aimbot", menu_state->aimbot_settings.enabled);
  struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
  struct nk_rect space;
  if (nk_widget(&space, ctx)) {
    nk_stroke_line(canvas, space.x, space.y + 4, space.x + space.w, space.y + 4, 1.0f, nk_rgb(49, 49, 49));
  }
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  nk_label(ctx, "text 2", NK_TEXT_LEFT);
}

static void draw_esp_group(struct nk_context *ctx, menu_state_t *menu_state) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->esp_settings.enabled = nk_check_label(ctx, "esp", menu_state->esp_settings.enabled);
  struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
  struct nk_rect space;
  if (nk_widget(&space, ctx)) {
    nk_stroke_line(canvas, space.x, space.y + 4, space.x + space.w, space.y + 4, 1.0f, nk_rgb(49, 49, 49));
  }
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  nk_label(ctx, "text 2", NK_TEXT_LEFT);
}

static void draw_misc_group(struct nk_context *ctx, menu_state_t *menu_state) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->misc_settings.enabled = nk_check_label(ctx, "misc", menu_state->misc_settings.enabled);
  struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
  struct nk_rect space;
  if (nk_widget(&space, ctx)) {
    nk_stroke_line(canvas, space.x, space.y + 4, space.x + space.w, space.y + 4, 1.0f, nk_rgb(49, 49, 49));
  }
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 3);
  menu_state->misc_settings.fly = nk_check_label(ctx, "fly", menu_state->misc_settings.fly);
  menu_state->misc_settings.sprint = nk_check_label(ctx, "sprint", menu_state->misc_settings.sprint);
  menu_state->misc_settings.nofall = nk_check_label(ctx, "nofall", menu_state->misc_settings.nofall);
  menu_state->misc_settings.inf_jump = nk_check_label(ctx, "infjump", menu_state->misc_settings.inf_jump);
}

static void draw_scripts_group(struct nk_context *ctx, menu_state_t *menu_state) {
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  menu_state->scripts_settings.enabled = nk_check_label(ctx, "scripts", menu_state->scripts_settings.enabled);
  struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
  struct nk_rect space;
  if (nk_widget(&space, ctx)) {
    nk_stroke_line(canvas, space.x, space.y + 4, space.x + space.w, space.y + 4, 1.0f, nk_rgb(49, 49, 49));
  }
  nk_layout_row_dynamic(ctx, NK_DYNAMIC, 1);
  nk_label(ctx, "text 2", NK_TEXT_LEFT);
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
  if (nk_group_begin(ctx, "right", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
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
  ctx->style.window.min_row_height_padding = 0.0f;

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

  ctx->style.slider.normal = nk_style_item_color(element_bg);
  ctx->style.slider.hover = nk_style_item_color(element_bg);
  ctx->style.slider.cursor_normal = nk_style_item_color(nk_rgba(80, 80, 80, 255));
  ctx->style.slider.cursor_hover = nk_style_item_color(nk_rgba(100, 100, 100, 255));
  ctx->style.slider.cursor_size = nk_vec2(4, 0);
}

void setup_resolution(rendering_context_t *rctx) {
  float win_width = rctx->width * 0.30f;
  float win_height = rctx->height * 0.22f;

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

void draw_gui(rendering_context_t *rctx) {

  if (nk_begin(rctx->ctx, "mchack",
               nk_rect(100, 100, menu_state.resolution.win_width, menu_state.resolution.win_height),
               NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR)) {
    float ratio[] = {0.35f, 0.65f};
    nk_layout_row(rctx->ctx, NK_DYNAMIC, menu_state.resolution.panel_height, 2, ratio);
    draw_left_panel(rctx->ctx, &menu_state);
    draw_right_panel(rctx->ctx, &menu_state);
  }
  nk_window_show(rctx->ctx, "mchack", rctx->open);
  nk_end(rctx->ctx);
}

menu_state_t *get_menu_state(void) { return &menu_state; }