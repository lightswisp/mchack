#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION

#include "../hooks/hooks.h"
#include "../utils/logger.h"
#include <dlfcn.h>
#include "draw.h"
#include "gui.h"

rendering_context_t rctx = {.ctx = NULL,
                            .glfw = {0},
                            .atlas = NULL,
                            .width = 0,
                            .height = 0,
                            .initialized = false,
                            .open = false,
                            .orig_key_callback = NULL,
                            .orig_char_callback = NULL};

void *glfwSwapBuffers_addr = NULL;
typedef void (*glfwSwapBuffers_s)(GLFWwindow *);
glfwSwapBuffers_s glfwSwapBuffers_o;

void handle_input(GLFWwindow *win) {
  static bool was_pressed = false;
  bool is_pressed = (glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
  if (!is_pressed && was_pressed) {
    rctx.open = !rctx.open;
    rctx.ctx->style.cursor_visible = rctx.open;
    if (rctx.open) {
      GLFWkeyfun key_call = glfwSetKeyCallback(win, nk_glfw3_key_callback);
      GLFWcharfun char_call = glfwSetCharCallback(win, nk_glfw3_char_callback);
      if (!rctx.orig_char_callback && !rctx.orig_key_callback) {
        rctx.orig_char_callback = char_call;
        rctx.orig_key_callback = key_call;
      }
    } else {
      glfwSetKeyCallback(win, rctx.orig_key_callback);
      glfwSetCharCallback(win, rctx.orig_char_callback);
    }
  }
  was_pressed = is_pressed;
}

void swap_buffers(GLFWwindow *win) {
  if (!rctx.initialized) {
    LOGGER_LOG(LOG, "setting ctx");
    rctx.win = win;
    glewInit();
    glfwGetFramebufferSize(win, &rctx.width, &rctx.height);
    glViewport(0, 0, rctx.width, rctx.height);

    // saving the state
    GLint bound_buffer, bound_element_array_buffer, bound_vao = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bound_buffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &bound_element_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &bound_vao);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    rctx.ctx = nk_glfw3_init(&rctx.glfw, win, NK_GLFW3_DEFAULT);

    nk_glfw3_font_stash_begin(&rctx.glfw, &rctx.atlas);
    struct nk_font *default_font = nk_font_atlas_add_default(rctx.atlas, 33.0f, NULL);
    nk_glfw3_font_stash_end(&rctx.glfw);
    nk_style_set_font(rctx.ctx, &default_font->handle);
    nk_style_load_all_cursors(rctx.ctx, rctx.atlas->cursors);
    setup_menu_theme(rctx.ctx);
    setup_resolution(&rctx);
    rctx.initialized = true;

    // restoring the state
    glPopAttrib();
    glBindBuffer(GL_ARRAY_BUFFER, bound_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bound_element_array_buffer);
    glBindVertexArray(bound_vao);
  }

  // saving the state
  GLint bound_buffer, bound_element_array_buffer, bound_vao = 0;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bound_buffer);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &bound_element_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &bound_vao);
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  // rendering
  handle_input(win);
  nk_glfw3_new_frame(&rctx.glfw);
  draw_gui(&rctx);
  nk_glfw3_render(&rctx.glfw, NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

  // restoring the state
  glPopAttrib();
  glBindBuffer(GL_ARRAY_BUFFER, bound_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bound_element_array_buffer);
  glBindVertexArray(bound_vao);

  glfwSwapBuffers_o(win);
}

bool set_draw_hook(void) {
  LOGGER_LOG(LOG, "setting draw hook...");
  glfwSwapBuffers_addr = dlsym(RTLD_NEXT, "glfwSwapBuffers");
  if (glfwSwapBuffers_addr == NULL) {
    LOGGER_LOG(FATAL, "failed to get glfwSwapBuffers address");
    return false;
  }
  LOGGER_LOG(LOG, "glfwSwapBuffers @ %p", glfwSwapBuffers_addr);
  return hook_install(glfwSwapBuffers_addr, (void *)&swap_buffers, (void **)&glfwSwapBuffers_o);
}

bool remove_draw_hook(void) {
  LOGGER_LOG(LOG, "restoring callbacks...");
  glfwSetKeyCallback(rctx.win, rctx.orig_key_callback);
  glfwSetCharCallback(rctx.win, rctx.orig_char_callback);
  LOGGER_LOG(LOG, "removing draw hook...");
  if (!hook_remove(glfwSwapBuffers_addr)) {
    LOGGER_LOG(FATAL, "failed to remove draw hook");
    return false;
  }
  LOGGER_LOG(LOG, "shutting down nuklear...");
  nk_glfw3_shutdown(&rctx.glfw);
  return true;
}