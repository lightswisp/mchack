#include "overlay.h"
#include "menu.h"
#include "esp.h"
#include "math.h"
#include "../game.h"
#include <string.h>

overlay_t g_overlay = {0};

static void draw_title(rendering_context_t *rctx) {
  g_overlay.title.iteration++;
  if (!g_overlay.title.set) {
    strcpy(g_overlay.title.buffer, CHEAT_TITLE);
    g_overlay.title.size = strlen(g_overlay.title.buffer);
    g_overlay.title.set = true;
  }

  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_rect rect = {5, 5, rctx->width, 100};
  float hue = 0.5f * (sinf(M_PI * 0.5 * 0.001 * g_overlay.title.iteration) + 1.0f);
  struct nk_color color = nk_hsv_f(hue, 1.0f, 1.0f);
  nk_draw_text(canvas, rect, g_overlay.title.buffer, g_overlay.title.size, &rctx->overlay_font->handle,
               nk_rgba(0, 0, 0, 100), color);
}

static void draw_dark_background(rendering_context_t *rctx) {
  if (!get_menu()->open)
    return;
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);
  struct nk_rect rect =
      nk_rect(0, 0, rctx->width + rctx->ctx->style.window.padding.x, rctx->height + rctx->ctx->style.window.padding.y);
  nk_fill_rect(canvas, rect, 0.0, nk_rgba(0, 0, 0, 150));
}

static void draw_fov_circle(rendering_context_t *rctx, menu_t *menu) {
  if (!menu->aimbot_settings.enabled)
    return;
  if (menu->aimbot_settings.fov == 0.0f)
    return;

  game_t *game = get_game();
  struct nk_command_buffer *canvas = nk_window_get_canvas(rctx->ctx);

  float aimbot_radius = deg2rad(menu->aimbot_settings.fov / 2.0f);
  // vertical fov
  float game_radius = deg2rad(game->minecraft.options.fov_val / 2.0f);
  float fov_radius_pixels = (rctx->width / 2.0f) * (tanf(aimbot_radius) / tanf(game_radius));

  float diameter = fov_radius_pixels * 2;
  float center_x = rctx->width / 2.0 - (fov_radius_pixels);
  float center_y = rctx->height / 2.0 - (fov_radius_pixels);
  struct nk_rect fov_rect = nk_rect(center_x, center_y, diameter, diameter);
  nk_stroke_circle(canvas, fov_rect, 2.5f, nk_rgb(249, 39, 114));
}

void draw_overlay(rendering_context_t *rctx) {
  // it will be used to draw boxes, text, fov circle, etc
  menu_t *menu = get_menu();
  game_t *game = get_game();
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
    draw_title(rctx);

    if (game->in_game) {
      draw_fov_circle(rctx, menu);
      draw_esp(rctx, menu);
    }
  }

  nk_end(rctx->ctx);
  rctx->ctx->style.window.fixed_background = s1;
  rctx->ctx->style.window.background = s2;
}

overlay_t *get_overlay(void) { return &g_overlay; }