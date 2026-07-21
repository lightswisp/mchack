#include "game.h"
#include "core/localplayer.h"
#include "jvm.h"
#include "utils/logger.h"
#include <string.h>

static game_t g_game = {0};

static bool init_collections(JNIEnv *env) {
  jclass list_class = (*env)->FindClass(env, "java/util/List");
  if (!list_class) {
    LOGGER_LOG(FATAL, "failed to get list class");
    return false;
  }
  jmethodID list_size = (*env)->GetMethodID(env, list_class, "size", "()I");
  if (!list_size) {
    LOGGER_LOG(FATAL, "failed to get list size method");
    return false;
  }
  jmethodID list_clear = (*env)->GetMethodID(env, list_class, "clear", "()V");
  if (!list_clear) {
    LOGGER_LOG(FATAL, "failed to get list clear method");
    return false;
  }
  jmethodID list_get = (*env)->GetMethodID(env, list_class, "get", "(I)Ljava/lang/Object;");
  if (!list_get) {
    LOGGER_LOG(FATAL, "failed to get list getter");
    return false;
  }

  jclass map_class = (*env)->FindClass(env, "java/util/Map");
  if (!map_class) {
    LOGGER_LOG(FATAL, "failed to get map class");
    return false;
  }
  jmethodID map_clear = (*env)->GetMethodID(env, map_class, "clear", "()V");
  if (!map_clear) {
    LOGGER_LOG(FATAL, "failed to get clear method");
    return false;
  }

  jclass vec3_class = (*env)->FindClass(env, "net/minecraft/world/phys/Vec3");
  if (!vec3_class) {
    LOGGER_LOG(FATAL, "failed to get vec3 class");
    return false;
  }
  jfieldID vec3_x = (*env)->GetFieldID(env, vec3_class, "x", "D");
  jfieldID vec3_y = (*env)->GetFieldID(env, vec3_class, "y", "D");
  jfieldID vec3_z = (*env)->GetFieldID(env, vec3_class, "z", "D");

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
  if (!get_entity_renderer) {
    LOGGER_LOG(FATAL, "failed to get entity renderer getter");
    return false;
  }
  jobject entity_renderer_instance =
      (*env)->CallObjectMethod(env, g_game.minecraft.instance, get_entity_renderer);
  if (!entity_renderer_instance) {
    LOGGER_LOG(FATAL, "failed to get enity renderer");
    return false;
  }
  jclass entity_renderer = (*env)->GetObjectClass(env, entity_renderer_instance);

  jmethodID set_render_hitboxes = (*env)->GetMethodID(env, entity_renderer, "setRenderHitBoxes", "(Z)V");
  if (!set_render_hitboxes) {
    LOGGER_LOG(FATAL, "failed to get set_render_hitboxes");
    return false;
  }

  g_game.entity_renderer.class = entity_renderer;
  g_game.entity_renderer.instance = entity_renderer_instance;
  g_game.entity_renderer.set_render_hitboxes = set_render_hitboxes;

  return true;
}

static bool init_minecraft(JNIEnv *env) {

  jclass minecraft = (*env)->FindClass(env, "net/minecraft/client/Minecraft");
  if (minecraft == NULL) {
    LOGGER_LOG(FATAL, "failed to get minecraft class");
    return false;
  }

  jmethodID get_instance =
      (*env)->GetStaticMethodID(env, minecraft, "getInstance", "()Lnet/minecraft/client/Minecraft;");
  jobject minecraft_instance = (*env)->CallStaticObjectMethod(env, minecraft, get_instance);
  if (minecraft_instance == NULL) {
    LOGGER_LOG(FATAL, "failed to get minecraft instance");
    return false;
  }

  jmethodID start_attack = (*env)->GetMethodID(env, minecraft, "startAttack", "()V");
  if (!start_attack) {
    LOGGER_LOG(FATAL, "failed to get start attack method");
    return false;
  }

  jfieldID options_field = (*env)->GetFieldID(env, minecraft, "options", "Lnet/minecraft/client/Options;");
  if (!options_field) {
    LOGGER_LOG(FATAL, "failed to get options field");
    return false;
  }

  jobject options_instance = (*env)->GetObjectField(env, minecraft_instance, options_field);
  if (!options_instance) {
    LOGGER_LOG(FATAL, "failed to get intance of options");
    return false;
  }
  jclass options_class = (*env)->GetObjectClass(env, options_instance);
  jfieldID fov = (*env)->GetFieldID(env, options_class, "fov", "D");
  if (!fov) {
    LOGGER_LOG(FATAL, "failed to get fov field");
    return false;
  }

  g_game.minecraft.options.class = options_class;
  g_game.minecraft.options.instance = options_instance;
  g_game.minecraft.options.fov = fov;

  g_game.minecraft.start_attack = start_attack;

  g_game.minecraft.class = minecraft;
  g_game.minecraft.instance = minecraft_instance;

  return true;
}

static bool init_text_component(JNIEnv *env) {
  jclass text_component_class = (*env)->FindClass(env, "net/minecraft/network/chat/TextComponent");
  if (!text_component_class) {
    LOGGER_LOG(FATAL, "failed to get text component class");
    return false;
  }
  jmethodID text_component_constructor =
      (*env)->GetMethodID(env, text_component_class, "<init>", "(Ljava/lang/String;)V");
  if (!text_component_constructor) {
    LOGGER_LOG(FATAL, "failed to get text constructor");
    return false;
  }

  g_game.text_component.class = text_component_class;
  g_game.text_component.constructor = text_component_constructor;

  return true;
}

bool init_game(void) {
  JNIEnv *env = get_env();

  if (!init_minecraft(env)) {
    LOGGER_LOG(FATAL, "failed to init minecraft");
    return false;
  }

  LOGGER_LOG(LOG, "minecraft is initialized");
  if (!init_collections(env)) {
    LOGGER_LOG(FATAL, "failed to init collections");
    return false;
  }
  LOGGER_LOG(LOG, "collections are initialized");

  if (!init_entity_renderer(env)) {
    LOGGER_LOG(FATAL, "failed to init entity renderer");
    return false;
  }
  LOGGER_LOG(LOG, "entity renderer is initialized");

  if (!init_text_component(env)) {
    LOGGER_LOG(FATAL, "failed to init text component");
    return false;
  }

  return true;
}

game_t *get_game(void) { return &g_game; }
