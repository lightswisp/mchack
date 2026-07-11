#include "jvm.h"
#include "utils/logger.h"

static JavaVM *g_vm = NULL;
static JNIEnv *g_env = NULL;

bool init_jvm() {
  jsize jvm_count;
  if (JNI_GetCreatedJavaVMs(&g_vm, 1, &jvm_count) != JNI_OK) {
    LOGGER_LOG(FATAL, "unable to get java vms");
    return false;
  }
  if ((*g_vm)->AttachCurrentThread(g_vm, (void **)&g_env, NULL) != JNI_OK) {
    LOGGER_LOG(FATAL, "unable to attach to current thread");
    return false;
  }

  return true;
}

JavaVM *get_jvm(void) { return g_vm; }
JNIEnv *get_env(void) { return g_env; }

void detach_jvm() {
  if (g_vm != NULL)
    (*g_vm)->DetachCurrentThread(g_vm);
}