#pragma once
#include <jni.h>

bool init_jvm();
JavaVM *get_jvm(void);
JNIEnv *get_env(void);
void detach_jvm(void);