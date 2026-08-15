#include <threads.h>
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_INCLUDE_DEFAULT_ALLOCATOR

#include "../hooks/native/hooks.h"
#include "../utils/logger.h"
#include <dlfcn.h>
#include "draw.h"
#include "menu.h"
#include "overlay.h"
#include "input.h"

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

rendering_context_t rctx = {
    .ctx = NULL,
    .glfw = {0},
    .atlas = NULL,
    .width = 0,
    .height = 0,
    .initialized = false,
    .orig_key_callback = NULL,
    .orig_char_callback = NULL,
    .orig_cursor_callback = NULL,
    .orig_mouse_callback = NULL,
};

void *glfwSwapBuffers_addr = NULL;
typedef void (*glfwSwapBuffers_s)(GLFWwindow *);
glfwSwapBuffers_s glfwSwapBuffers_o;

const uint8_t overlay_font_data[] = {
#embed "../fonts/Monocraft.ttf"
};

/* callbacks */

static void on_window_resize(GLFWwindow *window, int width, int height) {
  rctx.width = width;
  rctx.height = height;
  rctx.orig_window_resize_callback(window, width, height);
}

static void on_fb_resize(GLFWwindow *window, int width, int height) {
  rctx.fb_width = width;
  rctx.fb_height = height;
  rctx.orig_fb_resize_callback(window, width, height);
}
/* callbacks end */

static void init_resize_callbacks() {
  rctx.orig_window_resize_callback = glfwSetWindowSizeCallback(rctx.win, on_window_resize);
  rctx.orig_fb_resize_callback = glfwSetFramebufferSizeCallback(rctx.win, on_fb_resize);
}

static void restore_resize_callbacks() {
  glfwSetWindowSizeCallback(rctx.win, rctx.orig_window_resize_callback);
  glfwSetFramebufferSizeCallback(rctx.win, rctx.orig_fb_resize_callback);
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

static void log_state() {
  LOGGER_LOG(LOG, "Active Texture: %d", opengl_state.last_active_texture);
  LOGGER_LOG(LOG, "Program: %u", opengl_state.last_program);
  LOGGER_LOG(LOG, "Texture 2D Binding: %u", opengl_state.last_texture);
  LOGGER_LOG(LOG, "Array Buffer Binding: %u", opengl_state.last_array_buffer);
  LOGGER_LOG(LOG, "Element Array Buffer Binding: %u", opengl_state.last_element_array_buffer);
  LOGGER_LOG(LOG, "Vertex Array Object: %u", opengl_state.last_vertex_array_object);

  LOGGER_LOG(LOG, "Viewport [x, y, w, h]: [%d, %d, %d, %d]", opengl_state.last_viewport[0],
             opengl_state.last_viewport[1], opengl_state.last_viewport[2], opengl_state.last_viewport[3]);
  LOGGER_LOG(LOG, "Scissor Box [x, y, w, h]: [%d, %d, %d, %d]", opengl_state.last_scissor_box[0],
             opengl_state.last_scissor_box[1], opengl_state.last_scissor_box[2], opengl_state.last_scissor_box[3]);

  LOGGER_LOG(LOG, "Blend Src RGB / Dst RGB: 0x%X / 0x%X", opengl_state.last_blend_src_rgb,
             opengl_state.last_blend_dst_rgb);
  LOGGER_LOG(LOG, "Blend Src Alpha / Dst Alpha: 0x%X / 0x%X", opengl_state.last_blend_src_alpha,
             opengl_state.last_blend_dst_alpha);
  LOGGER_LOG(LOG, "Blend Eq RGB / Eq Alpha: 0x%X / 0x%X", opengl_state.last_blend_equation_rgb,
             opengl_state.last_blend_equation_alpha);

  LOGGER_LOG(LOG, "Enable Blend: %s", opengl_state.last_enable_blend ? "true" : "false");
  LOGGER_LOG(LOG, "Enable Cull Face: %s", opengl_state.last_enable_cull_face ? "true" : "false");
  LOGGER_LOG(LOG, "Enable Depth Test: %s", opengl_state.last_enable_depth_test ? "true" : "false");
  LOGGER_LOG(LOG, "Enable Stencil Test: %s", opengl_state.last_enable_stencil_test ? "true" : "false");
  LOGGER_LOG(LOG, "Enable Scissor Test: %s", opengl_state.last_enable_scissor_test ? "true" : "false");

  LOGGER_LOG(LOG, "Depth Mask: %s", opengl_state.last_depth_mask ? "true" : "false");
  LOGGER_LOG(LOG, "Color Mask [R, G, B, A]: [%s, %s, %s, %s]", opengl_state.last_color_mask[0] ? "true" : "false",
             opengl_state.last_color_mask[1] ? "true" : "false", opengl_state.last_color_mask[2] ? "true" : "false",
             opengl_state.last_color_mask[3] ? "true" : "false");
  LOGGER_LOG(LOG, "Polygon Mode [Front, Back]: [0x%X, 0x%X]", opengl_state.last_polygon_mode[0],
             opengl_state.last_polygon_mode[1]);
}

static void restore_state() {
  // taken from https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_opengl3.cpp#L454
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
  glColorMask(opengl_state.last_color_mask[0], opengl_state.last_color_mask[1], opengl_state.last_color_mask[2],
              opengl_state.last_color_mask[3]);
  glPolygonMode(GL_FRONT_AND_BACK, (GLenum)opengl_state.last_polygon_mode[0]);

  glViewport(opengl_state.last_viewport[0], opengl_state.last_viewport[1], (GLsizei)opengl_state.last_viewport[2],
             (GLsizei)opengl_state.last_viewport[3]);
  glScissor(opengl_state.last_scissor_box[0], opengl_state.last_scissor_box[1],
            (GLsizei)opengl_state.last_scissor_box[2], (GLsizei)opengl_state.last_scissor_box[3]);
}

static void init_callbacks() {
  // getting callbacks

  init_resize_callbacks();
  init_input_callbacks(&rctx);

  // restoring them back
  restore_input_callbacks(&rctx);
}

void swap_buffers(GLFWwindow *win) {
  if (atomic_load(&rctx.shutting_down)) {
    LOGGER_LOG(LOG, "no more execution for us");
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

    init_callbacks();
    save_state();
    log_state();

    glViewport(0, 0, rctx.fb_width, rctx.fb_height);

    //    glActiveTexture(GL_TEXTURE0);
    //    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //    glDepthMask(GL_FALSE);
    //    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    rctx.ctx = nk_glfw3_init(&rctx.glfw, win, NK_GLFW3_DEFAULT);

    nk_glfw3_font_stash_begin(&rctx.glfw, &rctx.atlas);
    struct nk_font *default_font = nk_font_atlas_add_default(rctx.atlas, 33.0f, NULL);
    rctx.overlay_font =
        nk_font_atlas_add_from_memory(rctx.atlas, (void *)overlay_font_data, sizeof(overlay_font_data), 33, NULL);
    nk_glfw3_font_stash_end(&rctx.glfw);
    nk_style_set_font(rctx.ctx, &default_font->handle);
    // resizing cursors
    for (size_t i = 0; i < NK_CURSOR_COUNT; ++i) {
      rctx.atlas->cursors[i].size.x *= 2;
      rctx.atlas->cursors[i].size.y *= 2;
    }
    nk_style_load_all_cursors(rctx.ctx, rctx.atlas->cursors);
    nk_style_hide_cursor(rctx.ctx);
    init_menu(&rctx);
    rctx.initialized = true;
    restore_state();
  }

  if (win != rctx.win) {
    LOGGER_LOG(FATAL, "win ptr mismatch!");
    asm("int3");
  }

  save_state();

  glViewport(0, 0, rctx.fb_width, rctx.fb_height);
  // glActiveTexture(GL_TEXTURE0);
  // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  // glDepthMask(GL_FALSE);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  // rendering
  handle_input(&rctx);
  nk_glfw3_new_frame(&rctx.glfw);
  {
    draw_overlay(&rctx);
    draw_menu(&rctx);
  }
  nk_glfw3_render(&rctx.glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

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
  return native_hook_install(glfwSwapBuffers_addr, (void *)&swap_buffers, (void **)&glfwSwapBuffers_o);
}

bool remove_draw_hook(void) {
  atomic_store(&rctx.shutting_down, true);
  LOGGER_LOG(LOG, "now waiting for threads to synchronize...");
  while (atomic_load(&rctx.executing) == true) {
    thrd_yield();
  }
  LOGGER_LOG(LOG, "restoring callbacks...");
  restore_input_callbacks(&rctx);
  restore_resize_callbacks();

  LOGGER_LOG(LOG, "removing draw hook...");
  if (!native_hook_remove(glfwSwapBuffers_addr)) {
    LOGGER_LOG(FATAL, "failed to remove draw hook");
    return false;
  }
  LOGGER_LOG(LOG, "shutting down nuklear...");
  nk_glfw3_shutdown(&rctx.glfw);
  return true;
}

rendering_context_t *get_rendering_context(void) { return &rctx; }