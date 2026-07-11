#pragma once
#include <jni.h>

jclass get_minecraft_class(JNIEnv *env);
jobject get_minecraft_instance(JNIEnv *env, jclass mclass);