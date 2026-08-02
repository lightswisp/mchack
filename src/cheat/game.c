#include "game.h"
#include "core/localplayer.h"
#include "jvm.h"
#include "utils/logger.h"
#include "hooks/java/hooks.h"
#include <string.h>

static game_t g_game = {0};

static bool init_collections(JNIEnv *env) {
  jclass list_class = (*env)->FindClass(env, "java/util/List");
  CHECK(list_class);

  jmethodID list_size = (*env)->GetMethodID(env, list_class, "size", "()I");
  CHECK(list_size);

  jmethodID list_clear = (*env)->GetMethodID(env, list_class, "clear", "()V");
  CHECK(list_clear);

  jmethodID list_get = (*env)->GetMethodID(env, list_class, "get", "(I)Ljava/lang/Object;");
  CHECK(list_get);

  jclass map_class = (*env)->FindClass(env, "java/util/Map");
  CHECK(map_class);

  jmethodID map_clear = (*env)->GetMethodID(env, map_class, "clear", "()V");
  CHECK(map_clear);

  jclass vec3_class = (*env)->FindClass(env, "net/minecraft/world/phys/Vec3");
  CHECK(vec3_class);

  jfieldID vec3_x = (*env)->GetFieldID(env, vec3_class, "x", "D");
  CHECK(vec3_x);

  jfieldID vec3_y = (*env)->GetFieldID(env, vec3_class, "y", "D");
  CHECK(vec3_y);

  jfieldID vec3_z = (*env)->GetFieldID(env, vec3_class, "z", "D");
  CHECK(vec3_z);

  g_game.collections.vec3.class = vec3_class;
  g_game.collections.vec3.x = vec3_x;
  g_game.collections.vec3.y = vec3_y;
  g_game.collections.vec3.z = vec3_z;

  g_game.collections.map.class = map_class;
  g_game.collections.map.clear = map_clear;
  g_game.collections.list.class = list_class;
  g_game.collections.list.size = list_size;
  g_game.collections.list.get = list_get;
  g_game.collections.list.clear = list_clear;

  return true;
}

static bool init_entity_renderer(JNIEnv *env) {
  jmethodID get_entity_renderer =
      (*env)->GetMethodID(env, g_game.minecraft.class, "getEntityRenderDispatcher",
                          "()Lnet/minecraft/client/renderer/entity/EntityRenderDispatcher;");

  CHECK(get_entity_renderer);
  jobject entity_renderer_instance = (*env)->CallObjectMethod(env, g_game.minecraft.instance, get_entity_renderer);
  CHECK(entity_renderer_instance);

  jclass entity_renderer = (*env)->GetObjectClass(env, entity_renderer_instance);
  CHECK(entity_renderer);

  jmethodID set_render_hitboxes = (*env)->GetMethodID(env, entity_renderer, "setRenderHitBoxes", "(Z)V");
  CHECK(set_render_hitboxes);

  g_game.entity_renderer.class = entity_renderer;
  g_game.entity_renderer.instance = entity_renderer_instance;
  g_game.entity_renderer.set_render_hitboxes = set_render_hitboxes;

  return true;
}

static bool init_minecraft(JNIEnv *env) {

  jclass minecraft = (*env)->FindClass(env, "net/minecraft/client/Minecraft");
  CHECK(minecraft);

  jmethodID get_instance =
      (*env)->GetStaticMethodID(env, minecraft, "getInstance", "()Lnet/minecraft/client/Minecraft;");

  CHECK(get_instance);

  jobject minecraft_instance = (*env)->CallStaticObjectMethod(env, minecraft, get_instance);
  CHECK(minecraft_instance);

  jmethodID start_attack = (*env)->GetMethodID(env, minecraft, "startAttack", "()V");
  CHECK(start_attack);

  jfieldID options_field = (*env)->GetFieldID(env, minecraft, "options", "Lnet/minecraft/client/Options;");
  CHECK(options_field);

  jobject options_instance = (*env)->GetObjectField(env, minecraft_instance, options_field);
  CHECK(options_instance);

  jclass options_class = (*env)->GetObjectClass(env, options_instance);
  CHECK(options_class);

  jfieldID fov = (*env)->GetFieldID(env, options_class, "fov", "D");
  CHECK(fov);

  jfieldID key_sprint_field = (*env)->GetFieldID(env, options_class, "keySprint", "Lnet/minecraft/client/KeyMapping;");
  CHECK(key_sprint_field);

  jfieldID key_attack_field = (*env)->GetFieldID(env, options_class, "keyAttack", "Lnet/minecraft/client/KeyMapping;");
  CHECK(key_attack_field);

  jfieldID key_up_field = (*env)->GetFieldID(env, options_class, "keyUp", "Lnet/minecraft/client/KeyMapping;");
  CHECK(key_up_field);

  jobject key_sprint = (*env)->GetObjectField(env, options_instance, key_sprint_field);
  CHECK(key_sprint);

  jobject key_attack = (*env)->GetObjectField(env, options_instance, key_attack_field);
  CHECK(key_attack);

  jobject key_up = (*env)->GetObjectField(env, options_instance, key_up_field);
  CHECK(key_up);

  jfieldID crosshair_field =
      (*env)->GetFieldID(env, minecraft, "crosshairPickEntity", "Lnet/minecraft/world/entity/Entity;");
  CHECK(crosshair_field);

  jfieldID player_field = (*env)->GetFieldID(env, minecraft, "player", "Lnet/minecraft/client/player/LocalPlayer;");
  CHECK(player_field);

  jfieldID gamma_field = (*env)->GetFieldID(env, options_class, "gamma", "D");
  CHECK(gamma_field);

  jfieldID game_renderer_field =
      (*env)->GetFieldID(env, minecraft, "gameRenderer", "Lnet/minecraft/client/renderer/GameRenderer;");
  CHECK(game_renderer_field);

  jmethodID get_frame_time = (*env)->GetMethodID(env, minecraft, "getFrameTime", "()F");
  CHECK(get_frame_time);

  jfieldID client_level_field =
      (*env)->GetFieldID(env, minecraft, "level", "Lnet/minecraft/client/multiplayer/ClientLevel;");
  CHECK(client_level_field);

  g_game.minecraft.options.gamma = gamma_field;
  g_game.minecraft.options.class = options_class;
  g_game.minecraft.options.instance = options_instance;
  g_game.minecraft.options.fov = fov;
  g_game.minecraft.options.key_sprint = key_sprint;
  g_game.minecraft.options.key_attack = key_attack;
  g_game.minecraft.options.key_up = key_up;

  g_game.minecraft.crosshair_field = crosshair_field;
  g_game.minecraft.localplayer = player_field;
  g_game.minecraft.game_renderer_field = game_renderer_field;
  g_game.minecraft.start_attack = start_attack;
  g_game.minecraft.get_frame_time = get_frame_time;
  g_game.minecraft.client_level_field = client_level_field;
  g_game.minecraft.class = minecraft;
  g_game.minecraft.instance = minecraft_instance;

  return true;
}

static bool init_client_level(JNIEnv *env) {
  jclass client_level_class = (*env)->FindClass(env, "Lnet/minecraft/client/multiplayer/ClientLevel;");
  CHECK(client_level_class);

  jfieldID players_field = (*env)->GetFieldID(env, client_level_class, "players", "Ljava/util/List;");
  CHECK(players_field);

  g_game.client_level.class = client_level_class;
  g_game.client_level.players_field = players_field;
  return true;
}

static bool init_key_mapping(JNIEnv *env) {
  jclass key_mapping = (*env)->FindClass(env, "Lnet/minecraft/client/KeyMapping;");
  CHECK(key_mapping);

  jfieldID is_down = (*env)->GetFieldID(env, key_mapping, "isDown", "Z");
  CHECK(is_down);

  jfieldID click_count = (*env)->GetFieldID(env, key_mapping, "clickCount", "I");
  CHECK(click_count);

  return true;
}

static bool init_players(JNIEnv *env) {
  jclass players_class = (*env)->FindClass(env, "Lnet/minecraft/world/entity/player/Player;");
  CHECK(players_class);

  jmethodID is_local_player = (*env)->GetMethodID(env, players_class, "isLocalPlayer", "()Z");
  CHECK(is_local_player);

  jmethodID get_x = (*env)->GetMethodID(env, players_class, "getX", "()D");
  CHECK(get_x);

  jmethodID get_y = (*env)->GetMethodID(env, players_class, "getY", "()D");
  CHECK(get_y);

  jmethodID get_z = (*env)->GetMethodID(env, players_class, "getZ", "()D");
  CHECK(get_z);

  jfieldID glowing = (*env)->GetFieldID(env, players_class, "glowing", "Z");
  CHECK(glowing);

  jfieldID x_old = (*env)->GetFieldID(env, players_class, "xOld", "D");
  CHECK(x_old);

  jfieldID y_old = (*env)->GetFieldID(env, players_class, "yOld", "D");
  CHECK(y_old);

  jfieldID z_old = (*env)->GetFieldID(env, players_class, "zOld", "D");
  CHECK(z_old);

  jfieldID eye_height = (*env)->GetFieldID(env, players_class, "eyeHeight", "F");
  CHECK(eye_height);

  jmethodID get_height = (*env)->GetMethodID(env, players_class, "getBbHeight", "()F");
  CHECK(get_height);

  jmethodID get_width = (*env)->GetMethodID(env, players_class, "getBbWidth", "()F");
  CHECK(get_width);

  jmethodID get_health = (*env)->GetMethodID(env, players_class, "getHealth", "()F");
  CHECK(get_health);

  jfieldID bounding_box_field = (*env)->GetFieldID(env, players_class, "bb", "Lnet/minecraft/world/phys/AABB;");
  CHECK(bounding_box_field);

  g_game.players.bounding_box = bounding_box_field;
  g_game.players.get_health = get_health;
  g_game.players.get_width = get_width;
  g_game.players.get_height = get_height;
  g_game.players.eye_height = eye_height;
  g_game.players.x_old = x_old;
  g_game.players.y_old = y_old;
  g_game.players.z_old = z_old;
  g_game.players.class = players_class;
  g_game.players.is_local_player = is_local_player;
  g_game.players.get_x = get_x;
  g_game.players.get_y = get_y;
  g_game.players.get_z = get_z;
  g_game.players.glowing = glowing;

  return true;
}

static bool init_game_renderer(JNIEnv *env) {
  jclass game_renderer_class = (*env)->FindClass(env, "Lnet/minecraft/client/renderer/GameRenderer;");
  CHECK(game_renderer_class);

  jfieldID game_renderer_fov = (*env)->GetFieldID(env, game_renderer_class, "fov", "F");
  CHECK(game_renderer_fov);

  jfieldID camera_field = (*env)->GetFieldID(env, game_renderer_class, "mainCamera", "Lnet/minecraft/client/Camera;");
  CHECK(camera_field);

  g_game.game_renderer.class = game_renderer_class;
  g_game.game_renderer.fov = game_renderer_fov;
  g_game.game_renderer.camera_field = camera_field;
  return true;
}

static bool init_camera(JNIEnv *env) {

  jclass camera_class = (*env)->FindClass(env, "Lnet/minecraft/client/Camera;");
  CHECK(camera_class);

  jfieldID x_rot_field = (*env)->GetFieldID(env, camera_class, "xRot", "F");
  CHECK(x_rot_field);

  jfieldID y_rot_field = (*env)->GetFieldID(env, camera_class, "yRot", "F");
  CHECK(y_rot_field);

  jfieldID position_field = (*env)->GetFieldID(env, camera_class, "position", "Lnet/minecraft/world/phys/Vec3;");
  CHECK(position_field);

  g_game.camera.class = camera_class;
  g_game.camera.x_rot = x_rot_field;
  g_game.camera.y_rot = y_rot_field;
  g_game.camera.position_field = position_field;
  return true;
}

static bool init_text_component(JNIEnv *env) {
  jclass text_component_class = (*env)->FindClass(env, "net/minecraft/network/chat/TextComponent");
  CHECK(text_component_class);

  jmethodID text_component_constructor =
      (*env)->GetMethodID(env, text_component_class, "<init>", "(Ljava/lang/String;)V");
  CHECK(text_component_constructor);

  g_game.text_component.class = text_component_class;
  g_game.text_component.constructor = text_component_constructor;

  return true;
}

static bool init_bounding_box(JNIEnv *env) {
  jclass bounding_box = (*env)->FindClass(env, "Lnet/minecraft/world/phys/AABB;");
  CHECK(bounding_box);

  jfieldID min_x = (*env)->GetFieldID(env, bounding_box, "minX", "D");
  CHECK(min_x);

  jfieldID min_y = (*env)->GetFieldID(env, bounding_box, "minY", "D");
  CHECK(min_y);

  jfieldID min_z = (*env)->GetFieldID(env, bounding_box, "minZ", "D");
  CHECK(min_z);

  jfieldID max_x = (*env)->GetFieldID(env, bounding_box, "maxX", "D");
  CHECK(max_x);

  jfieldID max_y = (*env)->GetFieldID(env, bounding_box, "maxY", "D");
  CHECK(max_y);

  jfieldID max_z = (*env)->GetFieldID(env, bounding_box, "maxZ", "D");
  CHECK(max_z);

  g_game.bounding_box.class = bounding_box;
  g_game.bounding_box.min_x = min_x;
  g_game.bounding_box.min_y = min_y;
  g_game.bounding_box.min_z = min_z;
  g_game.bounding_box.max_x = max_x;
  g_game.bounding_box.max_y = max_y;
  g_game.bounding_box.max_z = max_z;
  return true;
}

static bool init_localplayer(JNIEnv *env) {

  jclass local_player_class = (*env)->FindClass(env, "Lnet/minecraft/client/player/LocalPlayer;");
  CHECK(local_player_class);

  jclass super = (*env)->GetSuperclass(env, local_player_class);
  CHECK(super);

  jfieldID abilities_field =
      (*env)->GetFieldID(env, local_player_class, "abilities", "Lnet/minecraft/world/entity/player/Abilities;");
  CHECK(abilities_field);

  jclass abilities_class = (*env)->FindClass(env, "Lnet/minecraft/world/entity/player/Abilities;");
  CHECK(abilities_class);

  jfieldID mayfly = (*env)->GetFieldID(env, abilities_class, "mayfly", "Z");
  CHECK(mayfly);

  jfieldID walking_speed = (*env)->GetFieldID(env, abilities_class, "walkingSpeed", "F");
  CHECK(walking_speed);

  jfieldID on_ground = (*env)->GetFieldID(env, local_player_class, "onGround", "Z");
  CHECK(on_ground);

  jmethodID set_sprinting = (*env)->GetMethodID(env, super, "setSprinting", "(Z)V");
  CHECK(set_sprinting);

  jmethodID is_moving = (*env)->GetMethodID(env, local_player_class, "isMoving", "()Z");
  CHECK(is_moving);

  jmethodID display_client_message = (*env)->GetMethodID(env, local_player_class, "displayClientMessage",
                                                         "(Lnet/minecraft/network/chat/Component;Z)V");
  CHECK(display_client_message);

  jmethodID get_x = (*env)->GetMethodID(env, local_player_class, "getX", "()D");
  CHECK(get_x);

  jmethodID get_y = (*env)->GetMethodID(env, local_player_class, "getY", "()D");
  CHECK(get_y);

  jmethodID get_z = (*env)->GetMethodID(env, local_player_class, "getZ", "()D");
  CHECK(get_z);

  jfieldID x_rot = (*env)->GetFieldID(env, local_player_class, "xRot", "F");
  CHECK(x_rot);

  jfieldID y_rot = (*env)->GetFieldID(env, local_player_class, "yRot", "F");
  CHECK(y_rot);

  jfieldID delta_movement =
      (*env)->GetFieldID(env, local_player_class, "deltaMovement", "Lnet/minecraft/world/phys/Vec3;");
  CHECK(delta_movement);
  jmethodID set_delta_movement = (*env)->GetMethodID(env, local_player_class, "setDeltaMovement", "(DDD)V");
  CHECK(set_delta_movement);

  jmethodID get_delta_movement =
      (*env)->GetMethodID(env, local_player_class, "getDeltaMovement", "()Lnet/minecraft/world/phys/Vec3;");
  CHECK(get_delta_movement);

  jmethodID get_attack_strength_scale = (*env)->GetMethodID(env, local_player_class, "getAttackStrengthScale", "(F)F");
  CHECK(get_attack_strength_scale);

  jfieldID falling_distance = (*env)->GetFieldID(env, local_player_class, "fallDistance", "F");
  CHECK(falling_distance);

  jmethodID set_pos_raw = (*env)->GetMethodID(env, local_player_class, "setPosRaw", "(DDD)V");
  CHECK(set_pos_raw);

  jfieldID has_impulse = (*env)->GetFieldID(env, local_player_class, "hasImpulse", "Z");
  CHECK(has_impulse);

  jfieldID x_old = (*env)->GetFieldID(env, local_player_class, "xOld", "D");
  CHECK(x_old);

  jfieldID y_old = (*env)->GetFieldID(env, local_player_class, "yOld", "D");
  CHECK(y_old);

  jfieldID z_old = (*env)->GetFieldID(env, local_player_class, "zOld", "D");
  CHECK(z_old);

  jfieldID sprint_trigger_time = (*env)->GetFieldID(env, local_player_class, "sprintTriggerTime", "I");
  CHECK(sprint_trigger_time);

  jfieldID tick_count_field = (*env)->GetFieldID(env, local_player_class, "tickCount", "I");
  CHECK(tick_count_field);

  jmethodID tick = (*env)->GetMethodID(env, local_player_class, "tick", "()V");
  CHECK(tick);

  g_game.localplayer.tick = tick;
  g_game.localplayer.tick_count_field = tick_count_field;
  g_game.localplayer.sprint_trigger_time = sprint_trigger_time;
  g_game.localplayer.class = local_player_class;
  g_game.localplayer.abilities_field = abilities_field;
  g_game.localplayer.x_old = x_old;
  g_game.localplayer.y_old = y_old;
  g_game.localplayer.z_old = z_old;
  g_game.localplayer.has_impulse = has_impulse;
  g_game.localplayer.set_pos_raw = set_pos_raw;
  g_game.localplayer.falling_distance = falling_distance;
  g_game.localplayer.get_attack_strength_scale = get_attack_strength_scale;
  g_game.localplayer.delta_movement = delta_movement;
  g_game.localplayer.set_delta_movement = set_delta_movement;
  g_game.localplayer.get_delta_movement = get_delta_movement;
  g_game.localplayer.abilities.class = abilities_class;
  g_game.localplayer.abilities.mayfly = mayfly;
  g_game.localplayer.abilities.walking_speed = walking_speed;
  g_game.localplayer.on_ground = on_ground;
  g_game.localplayer.super = super;
  g_game.localplayer.set_sprinting = set_sprinting;
  g_game.localplayer.is_moving = is_moving;
  g_game.localplayer.display_client_message = display_client_message;
  g_game.localplayer.get_x = get_x;
  g_game.localplayer.get_y = get_y;
  g_game.localplayer.get_z = get_z;
  g_game.localplayer.x_rot = x_rot;
  g_game.localplayer.y_rot = y_rot;

  return true;
}

void on_tick() { LOGGER_LOG(LOG, "hooked !!!!!!!!!!!!!!!"); }

bool set_game_hooks() {
  JNIEnv *env = get_env();
  LOGGER_LOG(LOG, "setting onTick hook");
  CHECK_BOOL(java_hook_install(env, g_game.localplayer.tick, (void *)on_tick));

  return true;
}

bool init_game(void) {
  JNIEnv *env = get_env();

  CHECK_BOOL(init_minecraft(env));
  CHECK_BOOL(init_collections(env));
  CHECK_BOOL(init_entity_renderer(env));
  CHECK_BOOL(init_game_renderer(env));
  CHECK_BOOL(init_key_mapping(env));
  CHECK_BOOL(init_text_component(env));
  CHECK_BOOL(init_bounding_box(env));
  CHECK_BOOL(init_localplayer(env));
  CHECK_BOOL(init_client_level(env));
  CHECK_BOOL(init_players(env));
  CHECK_BOOL(init_camera(env));

  return true;
}

void deinit_game() {
  JNIEnv *env = get_env();
  // DELETE_REF(env, g_game.minecraft.class);
  DELETE_REF(env, g_game.minecraft.instance);
  DELETE_REF(env, g_game.minecraft.crosshair_pick_entity);
  DELETE_REF(env, g_game.minecraft.options.class);
  DELETE_REF(env, g_game.minecraft.options.instance);
  DELETE_REF(env, g_game.game_renderer.class);
  DELETE_REF(env, g_game.game_renderer.instance);
  DELETE_REF(env, g_game.camera.class);
  DELETE_REF(env, g_game.camera.instance);
  DELETE_REF(env, g_game.camera.position);
  DELETE_REF(env, g_game.client_level.class);
  DELETE_REF(env, g_game.client_level.instance);
  DELETE_REF(env, g_game.localplayer.super);
  DELETE_REF(env, g_game.localplayer.class);
  DELETE_REF(env, g_game.localplayer.instance);
  DELETE_REF(env, g_game.localplayer.abilities.class);
  DELETE_REF(env, g_game.localplayer.abilities.instance);
  DELETE_REF(env, g_game.players.class);
  DELETE_REF(env, g_game.players.instance);
  DELETE_REF(env, g_game.bounding_box.class);
  DELETE_REF(env, g_game.text_component.class);
  DELETE_REF(env, g_game.collections.map.class);
  DELETE_REF(env, g_game.collections.list.class);
  DELETE_REF(env, g_game.collections.vec3.class);
  DELETE_REF(env, g_game.entity_renderer.class);
  DELETE_REF(env, g_game.entity_renderer.instance);
  DELETE_REF(env, g_game.key_mapping.class);
}

game_t *get_game(void) { return &g_game; }
