#include "draw.h"
#include "../hooks/hooks.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <dlfcn.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "../utils/logger.h"
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

typedef struct {
  struct nk_context *ctx;
  struct nk_glfw glfw;
  struct nk_font_atlas *atlas;
  int width, height;
  bool initialized;
  bool open;
} rendering_context_t;

rendering_context_t rctx = {
    .ctx = NULL,
    .glfw = {0},
    .atlas = NULL,
    .width = 0,
    .height = 0,
    .initialized = false,
    .open = false,
};

void *glfwSwapBuffers_addr = NULL;
typedef void (*glfwSwapBuffers_s)(GLFWwindow *);
glfwSwapBuffers_s glfwSwapBuffers_o;

static void draw_gui() {
  if (nk_begin(rctx.ctx, "mchack", nk_rect(100, 100, 600, 400),
               NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE |
                   NK_WINDOW_TITLE)) {
    nk_layout_row_dynamic(rctx.ctx, 30, 1);
    if (nk_button_label(rctx.ctx, "button")) {
      LOGGER_LOG(LOG, "button pressed");
    }
  }
  nk_window_show(rctx.ctx, "mchack", rctx.open);
  nk_end(rctx.ctx);
}

void handle_input(GLFWwindow *win) {
  static bool was_pressed = false;
  bool is_pressed = (glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
  if (!is_pressed && was_pressed) {
    rctx.open = !rctx.open;
    rctx.ctx->style.cursor_visible = rctx.open;
  }
  was_pressed = is_pressed;
}

void swap_buffers(GLFWwindow *win) {

  if (!rctx.initialized) {
    LOGGER_LOG(LOG, "setting ctx");
    glewInit();
    rctx.ctx = nk_glfw3_init(&rctx.glfw, win, NK_GLFW3_DEFAULT);
    nk_glfw3_font_stash_begin(&rctx.glfw, &rctx.atlas);
    struct nk_font *monocraft = nk_font_atlas_add_from_file(
        rctx.atlas, "/home/lightswisp/Documents/Nuklear/demo/glfw_opengl3/Monocraft.ttf", 30, 0);
    nk_glfw3_font_stash_end(&rctx.glfw);
    nk_style_set_font(rctx.ctx, &monocraft->handle);
    nk_style_load_all_cursors(rctx.ctx, rctx.atlas->cursors);
    rctx.initialized = true;
  }

  handle_input(win);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  nk_glfw3_new_frame(&rctx.glfw);
  draw_gui();
  nk_glfw3_render(&rctx.glfw, NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
  glPopAttrib();

  glfwSwapBuffers_o(win);
}

bool set_draw_hook(void) {
  glfwSwapBuffers_addr = dlsym(RTLD_NEXT, "glfwSwapBuffers");
  if (glfwSwapBuffers_addr == NULL) {
    LOGGER_LOG(FATAL, "failed to get glfwSwapBuffers address");
    return false;
  }
  LOGGER_LOG(LOG, "glfwSwapBuffers @ %p", glfwSwapBuffers_addr);
  return hook_install(glfwSwapBuffers_addr, (void *)&swap_buffers, (void **)&glfwSwapBuffers_o);
}

bool remove_draw_hook(void) { return hook_remove(glfwSwapBuffers_addr); }