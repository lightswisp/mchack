#include "esp.h"

esp_t g_esp = {0};

static void draw_snap_lines(rendering_context_t *rctx, menu_t *menu) {
  if (!menu->esp_settings.snap_lines.enabled)
    return;
  if (g_esp.snap_lines.size == 0)
    return;

  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  for (size_t i = 0; i < g_esp.snap_lines.size; i++) {
    nk_stroke_line(canvas, g_esp.snap_lines.buffer[i].from.x, g_esp.snap_lines.buffer[i].from.y,
                   g_esp.snap_lines.buffer[i].to.x, g_esp.snap_lines.buffer[i].to.y, 3.0,
                   nk_rgb_cf(menu->esp_settings.snap_lines.color));
  }
}

static void draw_health(rendering_context_t *rctx, menu_t *menu) {
  if (!menu->esp_settings.health_bar.enabled)
    return;
  if (g_esp.health.size == 0)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  for (size_t i = 0; i < g_esp.health.size; i++) {

    // outline will be black
    nk_stroke_rect(canvas, g_esp.health.buffer[i].outer_rect, 0, 2.5f, nk_rgb(0, 0, 0));
    // inner rect's color is dynamically calculated
    nk_fill_rect(canvas, g_esp.health.buffer[i].inner_rect, 0, g_esp.health.buffer[i].color);
  }
}

static void draw_arrows(rendering_context_t *rctx, menu_t *menu) {
  if (!menu->esp_settings.arrows.enabled)
    return;
  if (g_esp.arrows.size == 0)
    return;

  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_color arrow_color = nk_rgb(249, 37, 114);

  for (size_t i = 0; i < g_esp.arrows.size; i++) {
    nk_fill_triangle(canvas, g_esp.arrows.buffer[i].t1[0].x, g_esp.arrows.buffer[i].t1[0].y,
                     g_esp.arrows.buffer[i].t1[1].x, g_esp.arrows.buffer[i].t1[1].y, g_esp.arrows.buffer[i].t1[2].x,
                     g_esp.arrows.buffer[i].t1[2].y, arrow_color);
    nk_fill_triangle(canvas, g_esp.arrows.buffer[i].t2[0].x, g_esp.arrows.buffer[i].t2[0].y,
                     g_esp.arrows.buffer[i].t2[1].x, g_esp.arrows.buffer[i].t2[1].y, g_esp.arrows.buffer[i].t2[2].x,
                     g_esp.arrows.buffer[i].t2[2].y, arrow_color);
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
  if (!menu->esp_settings.box.enabled)
    return;
  if (g_esp.boxes.size == 0)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  // let it be the default color.
  // i'll save it here for now as unused
  [[maybe_unused]] struct nk_color box_color = nk_rgb(249, 37, 114);
  for (size_t i = 0; i < g_esp.boxes.size; i++) {
    draw_outlined_box(canvas, g_esp.boxes.buffer[i].rect, 0.0, 2.0, nk_rgb_cf(menu->esp_settings.box.color));
  }
}

void draw_esp(rendering_context_t *rctx, menu_t *menu) {
  if (!menu->esp_settings.enabled)
    return;
  draw_boxes(rctx, menu);
  draw_health(rctx, menu);
  draw_arrows(rctx, menu);
  draw_snap_lines(rctx, menu);
}

esp_t *get_esp(void) { return &g_esp; }