#include "input.h"
#include "menu.h"
#include <string.h>
#include "../utils/logger.h"

const char *m_buttons[] = {"M1", "M2", "M3", "M4", "M5", "M6", "M7", "M8", "NULL"};
// callbacks
static void cursor_dummy([[maybe_unused]] GLFWwindow *win, [[maybe_unused]] double x, [[maybe_unused]] double y) {
  return;
}

const char *get_mouse_button_name(int button) {
  int idx = clampi(button, 0, arr_size(m_buttons));
  return m_buttons[idx];
}

const char *get_key_name(int key, int scancode) {
  const char *key_name = glfwGetKeyName(key, scancode);
  if (key_name == NULL) {
    return "NULL";
  }
  return key_name;
}

static void aimbot_bind_key_callback(GLFWwindow *win, int key, int scancode, int action, [[maybe_unused]] int mods) {
  LOGGER_LOG(LOG, "key callback");
  menu_t *menu = get_menu();
  if (action == GLFW_RELEASE) {
    if (menu->aimbot_settings.bind.state == BIND_PROCESS) {
      menu->aimbot_settings.bind.state = BIND_WAITING;
      return;
    }
    if (key == GLFW_KEY_ESCAPE) {
      // remove bind
      menu->aimbot_settings.bind.set = false;
    } else {
      menu->aimbot_settings.bind.type = BIND_KEY;
      menu->aimbot_settings.bind.as_code = key;
      menu->aimbot_settings.bind.set = true;
      strcpy(menu->aimbot_settings.bind.as_str, get_key_name(key, scancode));
    }
    // reset state
    menu->aimbot_settings.bind.state = BIND_IDLE;
    // reset callbacks
    glfwSetKeyCallback(win, nk_glfw3_key_callback);
    glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
    return;
  }
}
static void aimbot_bind_mouse_callback(GLFWwindow *win, int button, int action, [[maybe_unused]] int mods) {
  menu_t *menu = get_menu();
  if (action == GLFW_RELEASE) {
    if (menu->aimbot_settings.bind.state == BIND_PROCESS) {
      menu->aimbot_settings.bind.state = BIND_WAITING;
      return;
    }
    LOGGER_LOG(LOG, "mouse callback release");
    menu->aimbot_settings.bind.type = BIND_MOUSE;
    menu->aimbot_settings.bind.as_code = button;
    menu->aimbot_settings.bind.set = true;
    strcpy(menu->aimbot_settings.bind.as_str, get_mouse_button_name(button));
    menu->aimbot_settings.bind.state = BIND_IDLE;
    // reset callbacks
    glfwSetKeyCallback(win, nk_glfw3_key_callback);
    glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);

    return;
  }
}

void restore_input_callbacks(rendering_context_t *rctx) {
  glfwSetKeyCallback(rctx->win, rctx->orig_key_callback);
  glfwSetCharCallback(rctx->win, rctx->orig_char_callback);
  glfwSetCursorPosCallback(rctx->win, rctx->orig_cursor_callback);
  glfwSetMouseButtonCallback(rctx->win, rctx->orig_mouse_callback);
  glfwSetScrollCallback(rctx->win, rctx->orig_scroll_callback);
}

void set_input_callbacks(rendering_context_t *rctx) {
  glfwSetScrollCallback(rctx->win, nk_gflw3_scroll_callback);
  glfwSetKeyCallback(rctx->win, nk_glfw3_key_callback);
  glfwSetCharCallback(rctx->win, nk_glfw3_char_callback);
  glfwSetCursorPosCallback(rctx->win, cursor_dummy);
  glfwSetMouseButtonCallback(rctx->win, nk_glfw3_mouse_button_callback);
}

void init_input_callbacks(rendering_context_t *rctx) {
  rctx->orig_scroll_callback = glfwSetScrollCallback(rctx->win, nk_gflw3_scroll_callback);
  rctx->orig_key_callback = glfwSetKeyCallback(rctx->win, nk_glfw3_key_callback);
  rctx->orig_char_callback = glfwSetCharCallback(rctx->win, nk_glfw3_char_callback);
  rctx->orig_cursor_callback = glfwSetCursorPosCallback(rctx->win, cursor_dummy);
  rctx->orig_mouse_callback = glfwSetMouseButtonCallback(rctx->win, nk_glfw3_mouse_button_callback);
}

static void handle_aimbot_bind(menu_t *menu, rendering_context_t *rctx) {
  LOGGER_LOG(LOG, "bind has been requested!");
  glfwSetKeyCallback(rctx->win, aimbot_bind_key_callback);
  glfwSetMouseButtonCallback(rctx->win, aimbot_bind_mouse_callback);
  menu->aimbot_settings.bind.state = BIND_PROCESS;
}

static void handle_binds(menu_t *menu, rendering_context_t *rctx) {
  if (menu->aimbot_settings.bind.state == BIND_REQUESTED)
    handle_aimbot_bind(menu, rctx);
}

void handle_input(rendering_context_t *rctx) {
  menu_t *menu = get_menu();
  // handle all binds
  if (menu->open)
    handle_binds(menu, rctx);

  static bool was_pressed = false;
  bool is_pressed = (glfwGetKey(rctx->win, GLFW_KEY_DELETE) == GLFW_PRESS);
  if (!is_pressed && was_pressed) {
    menu->open = !menu->open;
    rctx->ctx->style.cursor_visible = menu->open;
    if (menu->open) {
      set_input_callbacks(rctx);
    } else {
      restore_input_callbacks(rctx);
    }
  }
  was_pressed = is_pressed;
}
