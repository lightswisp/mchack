#include "arrows.h"

void arrows(JNIEnv *env, game_t *game, menu_state_t *menu_state) {
  // it will traverse all players
  // then it will calculate the angle between our crosshair and the angle we need to turn to the enemy
  // this angle will be added to the menu_state->esp_settings.arrows2draw[i]
  // then an arrow will be drawn on the circle.
  // X coordinate of an arrow will be calculated as     cos(angle) * R
  // Y coordinate of an arrow will be calculated as     sin(angle) * R
  // where R is the radius of our arrow circle
}