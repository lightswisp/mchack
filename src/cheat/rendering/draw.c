#include <threads.h>
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_INCLUDE_DEFAULT_ALLOCATOR

#include "../hooks/hooks.h"
#include "../utils/logger.h"
#include <dlfcn.h>
#include "draw.h"
#include "gui.h"

struct {
  GLenum last_active_texture;
  GLuint last_sampler;
  GLuint last_program;
  GLuint last_texture;
  GLuint last_array_buffer;
  GLint last_element_array_buffer;
  GLuint last_vertex_array_object;
  GLint last_viewport[4];
  GLint last_scissor_box[4];
  GLenum last_blend_src_rgb;
  GLenum last_blend_dst_rgb;
  GLenum last_blend_src_alpha;
  GLenum last_blend_dst_alpha;
  GLenum last_blend_equation_rgb;
  GLenum last_blend_equation_alpha;
  GLboolean last_enable_blend;
  GLboolean last_enable_cull_face;
  GLboolean last_enable_depth_test;
  GLboolean last_enable_stencil_test;
  GLboolean last_enable_scissor_test;
  GLboolean last_depth_mask;
  GLboolean last_color_mask[4];
  GLint last_polygon_mode[2];
} opengl_state;

rendering_context_t rctx = {.ctx = NULL,
                            .glfw = {0},
                            .atlas = NULL,
                            .width = 0,
                            .height = 0,
                            .initialized = false,
                            .open = false,
                            .orig_key_callback = NULL,
                            .orig_char_callback = NULL,
                            .orig_cursor_callback = NULL,
                            .orig_mouse_callback = NULL};

void *glfwSwapBuffers_addr = NULL;
typedef void (*glfwSwapBuffers_s)(GLFWwindow *);
glfwSwapBuffers_s glfwSwapBuffers_o;

static void cursor_dummy([[maybe_unused]] GLFWwindow *win, [[maybe_unused]] double x,
                         [[maybe_unused]] double y) {
  return;
}

static void restore_callbacks() {
  glfwSetKeyCallback(rctx.win, rctx.orig_key_callback);
  glfwSetCharCallback(rctx.win, rctx.orig_char_callback);
  glfwSetCursorPosCallback(rctx.win, rctx.orig_cursor_callback);
  glfwSetMouseButtonCallback(rctx.win, rctx.orig_mouse_callback);
}

static void handle_input(GLFWwindow *win) {
  static bool was_pressed = false;
  bool is_pressed = (glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
  if (!is_pressed && was_pressed) {
    rctx.open = !rctx.open;
    rctx.ctx->style.cursor_visible = rctx.open;
    if (rctx.open) {
      GLFWkeyfun key_call = glfwSetKeyCallback(win, nk_glfw3_key_callback);
      GLFWcharfun char_call = glfwSetCharCallback(win, nk_glfw3_char_callback);
      GLFWcursorposfun cursor_call = glfwSetCursorPosCallback(win, cursor_dummy);
      GLFWmousebuttonfun mouse_call = glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
      if (!rctx.orig_char_callback && !rctx.orig_key_callback && !rctx.orig_cursor_callback &&
          !rctx.orig_mouse_callback) {
        rctx.orig_char_callback = char_call;
        rctx.orig_key_callback = key_call;
        rctx.orig_cursor_callback = cursor_call;
        rctx.orig_mouse_callback = mouse_call;
      }
    } else {
      restore_callbacks();
    }
  }
  was_pressed = is_pressed;
}

static void save_state() {
  // taken from https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_opengl3.cpp#L454
  glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *)&opengl_state.last_active_texture);
  glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *)&opengl_state.last_program);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *)&opengl_state.last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint *)&opengl_state.last_array_buffer);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &opengl_state.last_element_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint *)&opengl_state.last_vertex_array_object);
  glGetIntegerv(GL_VIEWPORT, opengl_state.last_viewport);
  glGetIntegerv(GL_SCISSOR_BOX, opengl_state.last_scissor_box);
  glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *)&opengl_state.last_blend_src_rgb);
  glGetIntegerv(GL_BLEND_DST_RGB, (GLint *)&opengl_state.last_blend_dst_rgb);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint *)&opengl_state.last_blend_src_alpha);
  glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint *)&opengl_state.last_blend_dst_alpha);
  glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint *)&opengl_state.last_blend_equation_rgb);
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint *)&opengl_state.last_blend_equation_alpha);
  opengl_state.last_enable_blend = glIsEnabled(GL_BLEND);
  opengl_state.last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
  opengl_state.last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
  opengl_state.last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
  opengl_state.last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

  glGetBooleanv(GL_DEPTH_WRITEMASK, &opengl_state.last_depth_mask);
  glGetBooleanv(GL_COLOR_WRITEMASK, opengl_state.last_color_mask);
  glGetIntegerv(GL_POLYGON_MODE, opengl_state.last_polygon_mode);
}

static void restore_state() {
  // taken from https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_opengl3.cpp#L454
  if (opengl_state.last_program == 0 || glIsProgram(opengl_state.last_program))
    glUseProgram(opengl_state.last_program);

  glGetIntegerv(GL_SAMPLER_BINDING, (GLint *)&opengl_state.last_sampler);
  glActiveTexture(opengl_state.last_active_texture);
  glBindTexture(GL_TEXTURE_2D, opengl_state.last_texture);
  glBindSampler(0, opengl_state.last_sampler);
  glBindVertexArray(opengl_state.last_vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, opengl_state.last_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opengl_state.last_element_array_buffer);
  glBlendEquationSeparate(opengl_state.last_blend_equation_rgb, opengl_state.last_blend_equation_alpha);
  glBlendFuncSeparate(opengl_state.last_blend_src_rgb, opengl_state.last_blend_dst_rgb,
                      opengl_state.last_blend_src_alpha, opengl_state.last_blend_dst_alpha);
  if (opengl_state.last_enable_blend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  if (opengl_state.last_enable_cull_face)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  if (opengl_state.last_enable_depth_test)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  if (opengl_state.last_enable_stencil_test)
    glEnable(GL_STENCIL_TEST);
  else
    glDisable(GL_STENCIL_TEST);
  if (opengl_state.last_enable_scissor_test)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);

  glDepthMask(opengl_state.last_depth_mask);
  glColorMask(opengl_state.last_color_mask[0], opengl_state.last_color_mask[1],
              opengl_state.last_color_mask[2], opengl_state.last_color_mask[3]);
  glPolygonMode(GL_FRONT_AND_BACK, (GLenum)opengl_state.last_polygon_mode[0]);

  glViewport(opengl_state.last_viewport[0], opengl_state.last_viewport[1],
             (GLsizei)opengl_state.last_viewport[2], (GLsizei)opengl_state.last_viewport[3]);
  glScissor(opengl_state.last_scissor_box[0], opengl_state.last_scissor_box[1],
            (GLsizei)opengl_state.last_scissor_box[2], (GLsizei)opengl_state.last_scissor_box[3]);
}

void swap_buffers(GLFWwindow *win) {
  if (atomic_load(&rctx.shutting_down)) {
    LOGGER_LOG(LOG, "no more execution for us");
    glfwSwapBuffers_o(win);
    return;
  }
  // for synchronization between the rendering thread and our thread
  atomic_store(&rctx.executing, true);

  if (!rctx.initialized) {
    LOGGER_LOG(LOG, "setting ctx");
    rctx.win = win;
    glewInit();
    glfwGetWindowSize(win, &rctx.width, &rctx.height);
    glfwGetFramebufferSize(win, &rctx.fb_width, &rctx.fb_height);

    save_state();

    glViewport(0, 0, rctx.fb_width, rctx.fb_height);

    glActiveTexture(GL_TEXTURE0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    rctx.ctx = nk_glfw3_init(&rctx.glfw, win, NK_GLFW3_DEFAULT);

    nk_glfw3_font_stash_begin(&rctx.glfw, &rctx.atlas);
    struct nk_font *default_font = nk_font_atlas_add_default(rctx.atlas, 33.0f, NULL);
    nk_glfw3_font_stash_end(&rctx.glfw);
    nk_style_set_font(rctx.ctx, &default_font->handle);
    nk_style_load_all_cursors(rctx.ctx, rctx.atlas->cursors);
    setup_menu_theme(rctx.ctx);
    setup_resolution(&rctx);
    rctx.initialized = true;

    restore_state();
  }

  save_state();

  glfwGetWindowSize(win, &rctx.width, &rctx.height);
  glfwGetFramebufferSize(win, &rctx.fb_width, &rctx.fb_height);
  glViewport(0, 0, rctx.fb_width, rctx.fb_height);
  glActiveTexture(GL_TEXTURE0);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_FALSE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  // rendering
  handle_input(win);
  nk_glfw3_new_frame(&rctx.glfw);
  draw_gui(&rctx);
  nk_glfw3_render(&rctx.glfw, NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

  restore_state();
  atomic_store(&rctx.executing, false);
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
  atomic_store(&rctx.shutting_down, true);
  LOGGER_LOG(LOG, "now waiting for threads to synchronize...");
  while (atomic_load(&rctx.executing) == true) {
    thrd_yield();
  }
  LOGGER_LOG(LOG, "removing draw hook...");
  if (!hook_remove(glfwSwapBuffers_addr)) {
    LOGGER_LOG(FATAL, "failed to remove draw hook");
    return false;
  }
  LOGGER_LOG(LOG, "shutting down nuklear...");
  nk_glfw3_shutdown(&rctx.glfw);
  LOGGER_LOG(LOG, "restoring callbacks...");
  restore_callbacks();
  return true;
}