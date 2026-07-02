#include <pthread.h>
#include <jni.h>

void      *__main_thread(void *a);
pthread_t g_thread;
JavaVM *vm;
JNIEnv *env;
FILE *log_file = NULL; 

#define LOG_MAX_BUFFER_SIZE      1024
#define LOG_MAX_TIME_BUFFER_SIZE 128
#define COLOR_START(x) "\033["x"m"
#define COLOR_END      "\033[0m"
#define RED            "31"
#define GREEN          "32" 
#define YELLOW         "33"

void logger_fatal(const char *func, const char *fmt, ...){
  char buffer[LOG_MAX_BUFFER_SIZE];
  char time_buffer[LOG_MAX_TIME_BUFFER_SIZE];
  time_t now;
  struct tm* tm;
  va_list args;

  now = time(NULL);
  tm  = localtime(&now);
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  strftime(time_buffer, sizeof(time_buffer), "%F %X", tm);
  fprintf(log_file, COLOR_START(RED)"[FATAL @ %s]"COLOR_START(YELLOW)" %s:"COLOR_END" %s\n", time_buffer, func, buffer);
  va_end(args);
}

void logger_log(const char *func, const char *fmt, ...){
  char buffer[LOG_MAX_BUFFER_SIZE];
  char time_buffer[LOG_MAX_TIME_BUFFER_SIZE];
  time_t now;
  struct tm* tm;
  va_list args;

  now = time(NULL);
  tm  = localtime(&now);
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  strftime(time_buffer, sizeof(time_buffer), "%F %X", tm);
  fprintf(log_file,COLOR_START(GREEN)"[LOG @ %s]"COLOR_START(YELLOW)" %s:"COLOR_END" %s\n", time_buffer, func, buffer);
  va_end(args);
}

/* run this function on library load */
__attribute__((constructor)) void __library_startup(){
  pthread_attr_t attr;

  /* start thread detached */
  if (-1 == pthread_attr_init(&attr)) 
    return;
  if (-1 == pthread_attr_setdetachstate(&attr,
                          PTHREAD_CREATE_DETACHED)) 
    return;

  /* spawn a thread to do the real work */
  pthread_create(&g_thread, NULL, __main_thread, NULL);
}

jobjectArray get_players(){
  jobjectArray players;

  return players;
}

void send_message(jobject localPlayer, const char *msg){
  // 3. Prepare your log message string
  const char* logText = "injected!";
  jstring jLogText = (*env)->NewStringUTF(env, logText);

  jclass textComponentClass = (*env)->FindClass(env, "net/minecraft/network/chat/TextComponent");
  jmethodID textConstructor = (*env)->GetMethodID(env, textComponentClass, "<init>", "(Ljava/lang/String;)V");
  jobject chatComponent = (*env)->NewObject(env, textComponentClass, textConstructor, jLogText);

  jclass playerClass = (*env)->GetObjectClass(env, localPlayer);
  jmethodID displayChatMethod = (*env)->GetMethodID(env, playerClass, "displayClientMessage", "(Lnet/minecraft/network/chat/Component;Z)V");

  if (displayChatMethod != NULL && chatComponent != NULL) {
      (*env)->CallVoidMethod(env, localPlayer, displayChatMethod, chatComponent, JNI_FALSE);
  }
}

/* our main loop */
void* __main_thread(void *a __attribute__((unused)))
{

  log_file = fopen("/tmp/mchack.log", "w");
  // disable buffering
  setvbuf(log_file, NULL, _IONBF, 0);
  logger_log(__func__, "injected");

  jsize jvm_count;
  if(JNI_GetCreatedJavaVMs(&vm, 1, &jvm_count) != JNI_OK){
    logger_fatal(__func__, "unable to get java vms");
    goto end;
  }
  if((*vm)->AttachCurrentThread(vm, (void**)&env, NULL) != JNI_OK){
    logger_fatal(__func__, "unable to attach to current thread");
    goto end;
  }

  jclass minecraft = (*env)->FindClass(env, "net/minecraft/client/Minecraft");
  if(minecraft == NULL){
    logger_fatal(__func__, "failed to get minecraft class");
    goto end;
  }

  logger_log(__func__, "minecraft @ %p", minecraft);
  jmethodID get_instance = (*env)->GetStaticMethodID(env, minecraft, "getInstance", "()Lnet/minecraft/client/Minecraft;");
  jobject minecraft_instance = (*env)->CallStaticObjectMethod(env, minecraft, get_instance);
  if (minecraft_instance == NULL){
    logger_fatal(__func__, "failed to get minecraft instance");
    goto end;
  }

  jfieldID playerField = (*env)->GetFieldID(env, minecraft, "player", "Lnet/minecraft/client/player/LocalPlayer;");
  jobject localPlayer = (*env)->GetObjectField(env, minecraft_instance, playerField);

  if (localPlayer == NULL) {
      (*env)->DeleteLocalRef(env, minecraft_instance);
      (*env)->DeleteLocalRef(env, minecraft);
      goto end;
  }

  send_message(localPlayer, "injected!");

end:
  return NULL;
}

__attribute__((destructor)) void __library_shutdown(){
  //REMOVE_HOOK(readLine);
  //if(g_jvm != NULL) (*g_jvm)->DetachCurrentThread(g_jvm);
}
