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
	mMethodVideoPacket = NULL;
	mMethodAudioPacket = NULL;
	mMethodPacket = NULL;
	mObject = NULL;
	mListenerName = "postEventFromNative";
	mListenerVideoPacketName = "postVideoPacketEventFromNative";
	mListenerAudioPacketName = "postAudioPacketEventFromNative";
	mListenerPacketName = "postPacketEventFromNative";
	jclass clazz = env->GetObjectClass(thiz);
	if(clazz == NULL) {
		return;
	}
	mClass =(jclass) env->NewGlobalRef(clazz);
	mObject = env->NewGlobalRef(weak_cls);
	mMethod = env->GetStaticMethodID(mClass, mListenerName, "(Ljava/lang/Object;I[BI[B)V");
	mMethodVideoPacket = env->GetStaticMethodID(mClass, mListenerVideoPacketName, "(Ljava/lang/Object;I[B)V");
	mMethodAudioPacket = env->GetStaticMethodID(mClass, mListenerAudioPacketName, "(Ljava/lang/Object;I[B)V");
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
	mMethodVideoPacket = NULL;
	mMethodAudioPacket = NULL;
}

void Listener::notify(pthread_t threadId, int msg, jbyteArray arg1, int len, jbyteArray tag) {
	AutoMutex l(mNotifyLock);
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	env->CallStaticVoidMethod(mClass, mMethod, mObject, msg, arg1, len, tag);
//	if(mjVMThread != threadId) {
//		mpjVM->DetachCurrentThread();
//	}
}

void Listener::notifyPacket(pthread_t threadId, int msg, jbyteArray arg1) {
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	env->CallStaticVoidMethod(mClass, mMethodPacket, mObject, msg, arg1);
//	if(mjVMThread != threadId) {
//		mpjVM->DetachCurrentThread();
//	}
}

void Listener::notifyVideoPacket(pthread_t threadId, int msg, jbyteArray arg1) {
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	env->CallStaticVoidMethod(mClass, mMethodVideoPacket, mObject, msg, arg1);
//	if(mjVMThread != threadId) {
//		mpjVM->DetachCurrentThread();
//	}
}

void Listener::notifyAudioPacket(pthread_t threadId, int msg, jbyteArray arg1) {
	JNIEnv* env = NULL;
	mpjVM->AttachCurrentThread((JNIEnv**)&env, NULL);
	env->CallStaticVoidMethod(mClass, mMethodAudioPacket, mObject, msg, arg1);
//	if(mjVMThread != threadId) {
//		mpjVM->DetachCurrentThread();
//	}
}
