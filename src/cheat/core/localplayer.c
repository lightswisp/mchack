#include "localplayer.h"
#include "../jvm.h"
#include "../game.h"

jobject get_localplayer(JNIEnv *env, game_t *game) {

  jfieldID player_field =
      (*env)->GetFieldID(env, game->minecraft.class, "player", "Lnet/minecraft/client/player/LocalPlayer;");
  if (player_field == NULL)
    return NULL;
  jobject local_player = (*env)->GetObjectField(env, game->minecraft.instance, player_field);

  if (local_player == NULL)
    return NULL;

  return local_player;
}