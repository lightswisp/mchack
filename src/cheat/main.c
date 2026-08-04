#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "core/message.h"
#include "game.h"
#include "hooks/native/hooks.h"
#include "jvm.h"
#include "rendering/draw.h"
#include "rendering/gui.h"
#include "rendering/overlay.h"
#include "utils/logger.h"
#include "core/updater.h"

/* hacks */
#include "hacks/esp/esp.h"
#include "hacks/misc/misc.h"
#include "hacks/aimbot/aimbot.h"

#ifndef MCHACK_LOG_PATH
#define MCHACK_LOG_PATH ""
#error "log path is undefined"
#endif

void *__main_thread(void *a);
pthread_t g_thread;
bool should_close = false;

/* run this function on library load */
__attribute__((constructor)) void __onload() {
  pthread_attr_t attr;

  /* start thread detached */
  if (-1 == pthread_attr_init(&attr))
    return;
  if (-1 == pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
    return;

  /* spawn a thread to do the real work */
  pthread_create(&g_thread, NULL, __main_thread, NULL);
}

bool set_hooks() {
  LOGGER_LOG(LOG, "setting hooks..");
  return set_draw_hook() && set_game_hooks();
}

bool remove_hooks(void) {
  LOGGER_LOG(LOG, "removing hooks..");
  // send_message("remving hooks..");
  return remove_draw_hook() && remove_game_hooks();
}

void uninject([[maybe_unused]] int sig) {
  LOGGER_LOG(WARNING, "uninjecting!");
  should_close = true;
  remove_hooks();
  LOGGER_LOG(LOG, "hooks are removed");
  LOGGER_LOG(WARNING, "removing local references...");
  deinit_game();
  LOGGER_LOG(WARNING, "detaching from jvm...");
  detach_jvm();
}

/* our main loop */
void *__main_thread([[maybe_unused]] void *a) {

  struct sigaction sa;
  sa.sa_handler = uninject;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGUSR1, &sa, NULL);

  init_logger(MCHACK_LOG_PATH);
  if (!init_jvm()) {
    LOGGER_LOG(FATAL, "failed to init jvm");
    goto end;
  }

  if (!init_game()) {
    LOGGER_LOG(FATAL, "failed to init game");
    goto end;
  }

  LOGGER_LOG(LOG, "injected");
  // send_message("injected!");

  if (!set_hooks()) {
    LOGGER_LOG(FATAL, "failed to set hooks");
    // send_message("failed to set hooks");
    goto end;
  }

  game_t *game = get_game();
  JNIEnv *env = get_env();
  menu_state_t *menu_state = get_menu_state();
  overlay_t *overlay = get_overlay();
  struct timespec loop_ts = {.tv_sec = 0, .tv_nsec = 1000000};
  struct timespec update_ts = {.tv_sec = 1, .tv_nsec = 0};

  reset_updates(game, menu_state, overlay);
  while (!should_close) {
    if (!update(env, game, menu_state, overlay)) {
      LOGGER_LOG(WARNING, "update has failed, waiting...");
      nanosleep(&update_ts, NULL);
      continue;
    }
    esp(env, game, menu_state, overlay);
    misc(env, game, menu_state);
    aimbot(env, game, menu_state);

    nanosleep(&loop_ts, NULL);
  }

  // uninject(SIGUSR1);

end:
  return NULL;
}

__attribute__((destructor)) void __onunload() { LOGGER_LOG(WARNING, "module is unloading..."); }
