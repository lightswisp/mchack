#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"
#include <stdatomic.h>

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

typedef struct {
  struct nk_context *ctx;
  struct nk_glfw glfw;
  struct nk_font_atlas *atlas;
  struct nk_font *overlay_font;
  int width, height;
  int fb_width, fb_height;
  bool initialized;

  _Atomic bool executing;
  _Atomic bool shutting_down;

  GLFWkeyfun orig_key_callback;
  GLFWcharfun orig_char_callback;
  GLFWcursorposfun orig_cursor_callback;
  GLFWmousebuttonfun orig_mouse_callback;
  GLFWwindowsizefun orig_window_resize_callback;
  GLFWframebuffersizefun orig_fb_resize_callback;
  GLFWscrollfun orig_scroll_callback;
  GLFWwindow *win;

} rendering_context_t;

bool set_draw_hook(void);
bool remove_draw_hook(void);
rendering_context_t *get_rendering_context(void);