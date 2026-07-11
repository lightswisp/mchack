#include "player.h"
#include "../utils/logger.h"

jobject get_localplayer(JNIEnv *env, jclass mclass, jobject minstance) {
  jfieldID player_field =
      (*env)->GetFieldID(env, mclass, "player", "Lnet/minecraft/client/player/LocalPlayer;");
  if (player_field == NULL)
    return NULL;
  jobject local_player = (*env)->GetObjectField(env, minstance, player_field);

  if (local_player == NULL)
    return NULL;

  LOGGER_LOG(LOG, "local_player @ %p", local_player);
  return local_player;
}