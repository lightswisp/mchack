#include "minecraft.h"
#include "../utils/logger.h"

jclass get_minecraft_class(JNIEnv *env) {
  jclass minecraft = (*env)->FindClass(env, "net/minecraft/client/Minecraft");
  if (minecraft == NULL) {
    LOGGER_LOG(FATAL, "failed to get minecraft class");
    return NULL;
  }
  return minecraft;
}

jobject get_minecraft_instance(JNIEnv *env, jclass mclass) {
  jmethodID get_instance =
      (*env)->GetStaticMethodID(env, mclass, "getInstance", "()Lnet/minecraft/client/Minecraft;");
  jobject minecraft_instance = (*env)->CallStaticObjectMethod(env, mclass, get_instance);
  if (minecraft_instance == NULL) {
    LOGGER_LOG(FATAL, "failed to get minecraft instance");
    return NULL;
  }
  LOGGER_LOG(LOG, "minecraft @ %p", minecraft_instance);
  return minecraft_instance;
}