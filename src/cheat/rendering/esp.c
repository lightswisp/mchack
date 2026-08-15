#include "esp.h"

esp_t g_esp = {0};

static void draw_snap_lines() {
  // nk_stroke_line(canvas, (float)rctx->width / 2, (float)rctx->height / 2,
  //                menu.esp_settings.box.boxes[i].x, menu.esp_settings.box.boxes[i].y, 2.0,
  //                nk_rgb(255, 255, 255));
}

static void draw_health(rendering_context_t *rctx, menu_t *menu) {
  if (!menu->esp_settings.enabled)
    return;
  if (!menu->esp_settings.health_bar.enabled)
    return;
  if (g_esp.health.size == 0)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  for (size_t i = 0; i < g_esp.health.size; i++) {
    float height = fabs(g_esp.health.buffer[i].head.y - g_esp.health.buffer[i].feet.y);
    // offset from the "draw_boxes"
    float box_width = height * 0.35;
    // width of health box
    float health_width = height * 0.04;

    // x offset
    float x = (g_esp.health.buffer[i].head.x - (box_width / 2.0)) - (health_width * 2);
    float y = g_esp.health.buffer[i].head.y;
    float t = 2.5;

    // 20.0 -> max health
    float health_ratio = g_esp.health.buffer[i].health / 20.0f;

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

static void draw_arrows(rendering_context_t *rctx, menu_t *menu) {
  if (!menu->esp_settings.enabled)
    return;
  if (!menu->esp_settings.arrows.enabled)
    return;
  if (g_esp.arrows.size == 0)
    return;

  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_color arrow_color = nk_rgb(249, 37, 114);
  float arrow_size = 20.0f;

  float center_x = rctx->width / 2.0;
  float center_y = rctx->height / 2.0;

  for (size_t i = 0; i < g_esp.arrows.size; i++) {
    float x = center_x;
    float y = center_y;
    float angle = g_esp.arrows.buffer[i];
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
    vec2_t add_factor = {cosf(angle) * menu->esp_settings.arrows.radius,
                         -sinf(angle) * menu->esp_settings.arrows.radius};
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

static void draw_boxes(rendering_context_t *rctx, menu_t *menu) {
  if (!menu->esp_settings.enabled)
    return;
  if (!menu->esp_settings.box.enabled)
    return;
  if (g_esp.boxes.size == 0)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_color box_color = nk_rgb(249, 37, 114);
  for (size_t i = 0; i < g_esp.boxes.size; i++) {
    float height = fabs(g_esp.boxes.buffer[i].head.y - g_esp.boxes.buffer[i].feet.y);
    float width = height * 0.35;
    float t = 2.0;

    struct nk_rect rect = {g_esp.boxes.buffer[i].head.x - (width / 2.0), g_esp.boxes.buffer[i].head.y, width, height};
    draw_outlined_box(canvas, rect, 0, t, box_color);
  }
}

void draw_esp(rendering_context_t *rctx, menu_t *menu) {
  draw_boxes(rctx, menu);
  draw_health(rctx, menu);
  draw_arrows(rctx, menu);
}

esp_t *get_esp(void) { return &g_esp; }