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

void *glfwSwapBuffers_addr = NULL;
typedef void (*glfwSwapBuffers_s)(GLFWwindow *);
glfwSwapBuffers_s glfwSwapBuffers_o;

static void draw_gui() { LOGGER_LOG(LOG, "drawing gui"); }

void swap_buffers(GLFWwindow *win) {
  static struct nk_context *ctx;
  static struct nk_glfw glfw;
  static struct nk_font_atlas *atlas;
  static int width, height;

  if (!ctx) {
    LOGGER_LOG(LOG, "setting ctx");
    glfwGetFramebufferSize(win, &width, &height);
    glewInit();
    ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_DEFAULT);
    nk_glfw3_font_stash_begin(&glfw, &atlas);
    struct nk_font *monocraft = nk_font_atlas_add_from_file(
        atlas, "/home/lightswisp/Documents/Nuklear/demo/glfw_opengl3/Monocraft.ttf", 20, 0);
    nk_glfw3_font_stash_end(&glfw);
    nk_style_set_font(ctx, &monocraft->handle);
    nk_style_load_all_cursors(ctx, atlas->cursors);
  }
  // draw_gui();
  nk_glfw3_new_frame(&glfw);
  if (nk_begin(ctx, "mchack", nk_rect(100, 100, 600, 400),
               NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE |
                   NK_WINDOW_TITLE)) {
    nk_layout_row_static(ctx, 30, 80, 1);
    if (nk_button_label(ctx, "button"))
      fprintf(stdout, "button pressed\n");
  }
  nk_end(ctx);
  LOGGER_LOG(LOG, "rendering...");
  nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
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