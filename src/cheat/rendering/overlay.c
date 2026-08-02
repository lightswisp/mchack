#include "overlay.h"
#include "gui.h"
#include "math.h"
#include "../game.h"

overlay_t overlay = {0};

static void draw_arrows(rendering_context_t *rctx, menu_state_t *menu_state) {
  if (!menu_state->esp_settings.enabled)
    return;
  if (!menu_state->esp_settings.arrows.enabled)
    return;
  if (overlay.arrows.size == 0)
    return;

  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_color arrow_color = nk_rgb(249, 37, 114);
  float arrow_size = 20.0f;

  float center_x = rctx->width / 2.0;
  float center_y = rctx->height / 2.0;

  for (size_t i = 0; i < overlay.arrows.size; i++) {
    float x = center_x;
    float y = center_y;
    float angle = overlay.arrows.buffer[i];
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
    vec2_t add_factor = {cosf(angle) * menu_state->esp_settings.arrows.radius,
                         -sinf(angle) * menu_state->esp_settings.arrows.radius};
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

static void draw_outlined_box(struct nk_command_buffer *canvas, struct nk_rect rect, float r, float t,
                              struct nk_color c) {

  struct nk_rect f1 = rect;
  struct nk_rect f2 = rect;
  f1.x -= t;
  f1.y -= t;
  f1.w += 2 * t;
  f1.h += 2 * t;

  f2.x += t;
  f2.y += t;
  f2.w -= 2 * t;
  f2.h -= 2 * t;

  nk_stroke_rect(canvas, f1, r, t, nk_rgb(0, 0, 0));
  nk_stroke_rect(canvas, rect, r, t, c);
  nk_stroke_rect(canvas, f2, r, t, nk_rgb(0, 0, 0));
}

static void draw_boxes(rendering_context_t *rctx, menu_state_t *menu_state) {
  if (!menu_state->esp_settings.enabled)
    return;
  if (!menu_state->esp_settings.box.enabled)
    return;
  if (overlay.boxes.size == 0)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_color box_color = nk_rgb(249, 37, 114);
  for (size_t i = 0; i < overlay.boxes.size; i++) {
    float height = fabs(overlay.boxes.buffer[i].head.y - overlay.boxes.buffer[i].feet.y);
    float width = height * 0.35;
    float t = 2.0;

    struct nk_rect rect = {overlay.boxes.buffer[i].head.x - (width / 2.0), overlay.boxes.buffer[i].head.y, width,
                           height};
    draw_outlined_box(canvas, rect, 0, t, box_color);
  }
}

static void draw_health(rendering_context_t *rctx, menu_state_t *menu_state) {
  if (!menu_state->esp_settings.enabled)
    return;
  if (!menu_state->esp_settings.health_bar.enabled)
    return;
  if (overlay.health.size == 0)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  for (size_t i = 0; i < overlay.health.size; i++) {
    float height = fabs(overlay.health.buffer[i].head.y - overlay.health.buffer[i].feet.y);
    // offset from the "draw_boxes"
    float box_width = height * 0.35;
    // width of health box
    float health_width = height * 0.04;

    // x offset
    float x = (overlay.health.buffer[i].head.x - (box_width / 2.0)) - (health_width * 2);
    float y = overlay.health.buffer[i].head.y;
    float t = 2.5;

    // 20.0 -> max health
    float health_ratio = overlay.health.buffer[i].health / 20.0f;

    // it will decrease with the ratio
    float health_height = height * health_ratio;

    struct nk_color health_color =
        nk_rgb((222.0 - (health_ratio * 222.0) + 33.0), 242 * health_ratio, 109 * health_ratio);

    struct nk_rect rect = {x, y + (height - health_height), health_width, health_height};

    // outline
    struct nk_rect rect_out = {x - t, y - t, health_width + 2 * t, height + 2 * t};

    nk_stroke_rect(canvas, rect_out, 0, t, nk_rgb(0, 0, 0));
    nk_fill_rect(canvas, rect, 0, health_color);
  }
}

static void draw_snap_lines() {
  // nk_stroke_line(canvas, (float)rctx->width / 2, (float)rctx->height / 2,
  //                menu_state.esp_settings.box.boxes[i].x, menu_state.esp_settings.box.boxes[i].y, 2.0,
  //                nk_rgb(255, 255, 255));
}

static void draw_dark_background(rendering_context_t *rctx) {
  if (!rctx->open)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_rect rect =
      nk_rect(0, 0, rctx->width + rctx->ctx->style.window.padding.x, rctx->height + rctx->ctx->style.window.padding.y);
  nk_fill_rect(canvas, rect, 0.0, nk_rgba(0, 0, 0, 150));
}

static void draw_fov_circle(rendering_context_t *rctx, menu_state_t *menu_state) {
  if (!menu_state->aimbot_settings.enabled)
    return;
  if (menu_state->aimbot_settings.fov == 0.0f)
    return;

  game_t *game = get_game();
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);

  float aimbot_radius = deg2rad(menu_state->aimbot_settings.fov / 2.0f);
  float game_fov = game->minecraft.options.fov_val;
  float game_radius = deg2rad(game_fov / 2.0f);
  float fov_radius_pixels = (rctx->width / 2.0f) * (tanf(aimbot_radius) / tanf(game_radius));

  float diameter = fov_radius_pixels * 2;
  float center_x = rctx->width / 2.0 - (fov_radius_pixels);
  float center_y = rctx->height / 2.0 - (fov_radius_pixels);
  struct nk_rect fov_rect = nk_rect(center_x, center_y, diameter, diameter);
  nk_stroke_circle(canvas, fov_rect, 2.5f, nk_rgb(249, 39, 114));
}

void draw_overlay(rendering_context_t *rctx) {
  // it will be used to draw boxes, text, fov circle, etc
  menu_state_t *menu_state = get_menu_state();
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
    draw_fov_circle(rctx, menu_state);
    draw_boxes(rctx, menu_state);
    draw_health(rctx, menu_state);
    draw_arrows(rctx, menu_state);
  }

  nk_end(rctx->ctx);
  rctx->ctx->style.window.fixed_background = s1;
  rctx->ctx->style.window.background = s2;
}

overlay_t *get_overlay(void) { return &overlay; }