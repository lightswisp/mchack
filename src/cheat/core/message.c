#include "message.h"
#include "../game.h"
#include "../jvm.h"

void send_message(const char *msg) {
  JNIEnv *env = get_env();
  game_t *game = get_game();

  jstring jLogText = (*env)->NewStringUTF(env, msg);

  jclass textComponentClass = (*env)->FindClass(env, "net/minecraft/network/chat/TextComponent");
  jmethodID textConstructor = (*env)->GetMethodID(env, textComponentClass, "<init>", "(Ljava/lang/String;)V");
  jobject chatComponent = (*env)->NewObject(env, textComponentClass, textConstructor, jLogText);

  jclass playerClass = (*env)->GetObjectClass(env, game->local_player);
  jmethodID displayChatMethod = (*env)->GetMethodID(env, playerClass, "displayClientMessage",
                                                    "(Lnet/minecraft/network/chat/Component;Z)V");

  if (displayChatMethod != NULL && chatComponent != NULL) {
    (*env)->CallVoidMethod(env, game->local_player, displayChatMethod, chatComponent, JNI_FALSE);
  }
}