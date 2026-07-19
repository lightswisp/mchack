#include "message.h"
#include "../jvm.h"
#include "../utils/logger.h"
#include "localplayer.h"

bool message_send(const char *msg) {
  bool status = false;
  JNIEnv *env = get_env();
  game_t *game = get_game();
  jstring log_text = NULL;
  jobject chat_component = NULL;

  if (!game->in_game)
    return false;

  log_text = (*env)->NewStringUTF(env, msg);
  if (!log_text) {
    LOGGER_LOG(FATAL, "failed to allocate memory for text");
    goto end;
  }
  chat_component =
      (*env)->NewObject(env, game->text_component.class, game->text_component.constructor, log_text);
  if (!chat_component) {
    LOGGER_LOG(FATAL, "failed to get chat component");
    goto end;
  }

  (*env)->CallVoidMethod(env, game->localplayer.instance, game->localplayer.display_client_message,
                         chat_component, JNI_FALSE);
  status = true;

end:
  (*env)->DeleteLocalRef(env, log_text);
  (*env)->DeleteLocalRef(env, chat_component);
  return status;
}