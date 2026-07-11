#include <jni.h>
#include <pthread.h>
#include <unistd.h>

#include "core/message.h"
#include "game.h"
#include "hooks/hooks.h"
#include "jvm.h"
#include "rendering/draw.h"
#include "utils/logger.h"

#ifndef MCHACK_LOG_PATH
#define MCHACK_LOG_PATH ""
#error "log path is undefined"
#endif

void *__main_thread(void *a);
pthread_t g_thread;

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

bool set_hooks(void) { return set_draw_hook(); }
bool remove_hooks(void) { return remove_draw_hook(); }

/* our main loop */
void *__main_thread(void *a __attribute__((unused))) {

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
  send_message("injected!");
  // 0x7f04bd007ff0

  // stop all threads here

  LOGGER_LOG(LOG, "setting hooks..");
  send_message("setting hooks..");
  if (!set_hooks()) {
    LOGGER_LOG(FATAL, "failed to set hooks");
    send_message("failed to set hooks");
    goto end;
  }

  // resume all threads here

end:
  return NULL;
}

__attribute__((destructor)) void __onunload() {
  LOGGER_LOG(WARNING, "module is unloading...");
  // stop all threads here
  remove_hooks();
  // resume all threads here
  detach_jvm();
}
