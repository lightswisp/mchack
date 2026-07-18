#include "message.h"
#include "../jvm.h"
#include "../utils/logger.h"
#include "localplayer.h"

bool send_message(const char *msg) {
  bool status = false;
  JNIEnv *env = get_env();
  game_t *game = get_game();

  jstring jLogText = (*env)->NewStringUTF(env, msg);
  if (!jLogText) {
    LOGGER_LOG(FATAL, "failed to allocate memory for text");
    goto end;
  }

  jclass textComponentClass = (*env)->FindClass(env, "net/minecraft/network/chat/TextComponent");
  if (!textComponentClass) {
    LOGGER_LOG(FATAL, "failed to get text component class");
    goto end;
  }
  jmethodID textConstructor = (*env)->GetMethodID(env, textComponentClass, "<init>", "(Ljava/lang/String;)V");
  if (!textConstructor) {
    LOGGER_LOG(FATAL, "failed to get text constructor");
    goto end;
  }
  jobject chatComponent = (*env)->NewObject(env, textComponentClass, textConstructor, jLogText);
  if (!chatComponent) {
    LOGGER_LOG(FATAL, "failed to get chat component");
    goto end;
  }

  jobject local_player = get_localplayer(env, game);
  jclass playerClass = (*env)->GetObjectClass(env, local_player);
  if (!playerClass) {
    LOGGER_LOG(FATAL, "failed to get player class");
    goto end;
  }
  jmethodID displayChatMethod = (*env)->GetMethodID(env, playerClass, "displayClientMessage",

                                                    "(Lnet/minecraft/network/chat/Component;Z)V");

  if (!displayChatMethod) {
    LOGGER_LOG(FATAL, "failed to get display chat method");
    goto end;
  }
  (*env)->CallVoidMethod(env, local_player, displayChatMethod, chatComponent, JNI_FALSE);
  status = true;

end:
  (*env)->DeleteLocalRef(env, jLogText);
  return status;
}