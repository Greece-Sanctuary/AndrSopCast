/*
 * Listener.cpp
 *
 *  Created on: 2012-12-26
 *      Author: stainberg
 */

#include "listener.h"
#include <stdlib.h>
#include <jni.h>

#ifdef TAG
#undef TAG
#endif
#define TAG "listener.cpp"

Listener::Listener(JavaVM* vm, jobject thiz, jobject weak_cls) {
	mpjVM = vm;
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	mjVMThread = pthread_self();
	mClass = NULL;
	mMethod = NULL;
	mMethodAVC = NULL;
	mMethodPacket = NULL;
	mObject = NULL;
	mListenerName = "postEventFromNative";
	mListenerAVCName = "postAVCFromNative";
	mListenerPacketName = "postPacketEventFromNative";
	jclass clazz = env->GetObjectClass(thiz);
	if(clazz == NULL) {
		return;
	}
	mClass =(jclass) env->NewGlobalRef(clazz);
	mObject = env->NewGlobalRef(weak_cls);
	mMethod = env->GetStaticMethodID(mClass, mListenerName, "(Ljava/lang/Object;I[BI[B)V");
	mMethodAVC = env->GetStaticMethodID(mClass, mListenerAVCName, "(Ljava/lang/Object;[BIII)V");
	mMethodPacket = env->GetStaticMethodID(mClass, mListenerPacketName, "(Ljava/lang/Object;I[B)V");
}

Listener::~Listener() {
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	env->DeleteGlobalRef(mClass);
	env->DeleteGlobalRef(mObject);
	mClass = NULL;
	mObject = NULL;
	mMethod = NULL;
	mMethodAVC = NULL;
	mMethodPacket = NULL;
}

void Listener::notify(pthread_t threadId, int msg, jbyteArray arg1, int len, jbyteArray tag) {
	AutoMutex l(mNotifyLock);
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	env->CallStaticVoidMethod(mClass, mMethod, mObject, msg, arg1, len, tag);
}

void Listener::notifyAVC(pthread_t threadId, jbyteArray arg1, int len,int width, int height) {
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	env->CallStaticVoidMethod(mClass, mMethodAVC, mObject, arg1, len, width, height);
}

void Listener::notifyPacket(pthread_t threadId, int msg, jbyteArray arg1) {
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	env->CallStaticVoidMethod(mClass, mMethodPacket, mObject, msg, arg1);
}
