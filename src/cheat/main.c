#include <jni.h>
#include <threads.h>
#include <unistd.h>
#include <signal.h>

#include "core/message.h"
#include "game.h"
#include "hooks/native/hooks.h"
#include "jvm.h"
#include "rendering/draw.h"
#include "rendering/menu.h"
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

bool should_close = false;

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
  if (!remove_hooks())
    LOGGER_LOG(FATAL, "failed to remove hooks! The game will probably crash oops ...");
  else
    LOGGER_LOG(WARNING, "hooks are removed");

  LOGGER_LOG(WARNING, "removing local references...");
  deinit_game();
  LOGGER_LOG(WARNING, "detaching from jvm...");
  detach_jvm();
  should_close = true;
}

/* our main loop */
int mchack_main([[maybe_unused]] void *a) {

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
#define UPDATE_TIME_NS (1000000.0)
  game->update_time = UPDATE_TIME_NS / 1000000000.0f;
  struct timespec loop_ts = {.tv_sec = 0, .tv_nsec = UPDATE_TIME_NS};
  struct timespec update_ts = {.tv_sec = 1, .tv_nsec = 0};

  reset_updates(game);
  while (!should_close) {
    if (!update(game)) {
      LOGGER_LOG(WARNING, "update has failed, waiting...");
      reset_updates(game);
      nanosleep(&update_ts, NULL);
      continue;
    }

    {
      // modules that will be running in main loop
      // misc module is being called from on_tick hook 20 times per second
      esp(game);
      aimbot(game);
    }

    nanosleep(&loop_ts, NULL);
  }

  LOGGER_LOG(WARNING, "bye bye..");
end:
  return 0;
}

__attribute__((destructor)) void onunload() { LOGGER_LOG(WARNING, "module is unloading..."); }

/* run this function on library load */
__attribute__((constructor)) void onload() {
  thrd_t g_thread;
  thrd_create(&g_thread, mchack_main, NULL);
  thrd_detach(g_thread);
}
