#include "nofall.h"

void nofall(JNIEnv *env, jobject this, game_t *game) {
  if (!game->menu->misc_settings.nofall)
    return;

  //  JNIEnv *env = get_env();
  //
  //  jobject local_player = get_localplayer();
  //  if (!local_player) {
  //    LOGGER_LOG(FATAL, "failed to get local player");
  //    return;
  //  }
  //  jclass local_player_class = (*env)->GetObjectClass(env, local_player);
  //  if (!local_player_class) {
  //    LOGGER_LOG(FATAL, "failed to get local player class");
  //    return;
  //  }
  //
  //  jfieldID on_ground = (*env)->GetFieldID(env, local_player_class, "onGround", "Z");
  //  if (!on_ground) {
  //    LOGGER_LOG(FATAL, "failed to get on ground field");
  //    return;
  //  }
  //  jfieldID fall_distance = (*env)->GetFieldID(env, local_player_class, "fallDistance", "F");
  //  float distance = (*env)->GetFloatField(env, local_player, fall_distance);
  //  LOGGER_LOG(LOG, "falling distance %f", distance);
  //  //(*env)->SetBooleanField(env, local_player, on_ground, true);
  //  (*env)->DeleteLocalRef(env, local_player);
  //  (*env)->DeleteLocalRef(env, local_player_class);
}