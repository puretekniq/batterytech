/*
 * boot.cpp
 *
 *  Created on: Jul 29, 2010
 *      Author: rgreen
 */

#ifdef ANDROID_NDK
#include "../../batterytech.h"
#include "importgl.h"
#include <jni.h>
#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#endif

// the current env for the call
JNIEnv* jnienv;
jobject javaBoot;

void Java_com_batterypoweredgames_batterytech_Boot_init(JNIEnv* env, jobject thiz) {
	jnienv = env;
	javaBoot = thiz;
	btInit();
	jnienv = 0;
	javaBoot = 0;
}

void Java_com_batterypoweredgames_batterytech_Boot_update(JNIEnv* env, jobject thiz, jfloat delta) {
	//__android_log_print(ANDROID_LOG_DEBUG, "BatteryTech Boot", "delta is %f", delta);
	jnienv = env;
	javaBoot = thiz;
	btUpdate((F32)delta);
	jnienv = 0;
	javaBoot = 0;
}

void Java_com_batterypoweredgames_batterytech_Boot_draw(JNIEnv* env, jobject thiz) {
	jnienv = env;
	javaBoot = thiz;
	btDraw();
	jnienv = 0;
	javaBoot = 0;
}

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID */