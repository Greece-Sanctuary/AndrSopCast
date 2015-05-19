/*
 * Listener.h
 *
 *  Created on: 2012-12-26
 *      Author: stainberg
 */

#ifndef LISTENER_H_
#define LISTENER_H_

#include <jni.h>
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

class Listener {
public:
	Listener();
	Listener(JavaVM* vm, jobject cls, jobject weak_cls);
	virtual ~Listener();
	virtual void notify(pthread_t threadId, int msg, jbyteArray arg1, int len, jbyteArray tag);
	virtual void notifyVideoPacket(pthread_t threadId, int msg, jbyteArray arg1);
	virtual void notifyAudioPacket(pthread_t threadId, int msg, jbyteArray arg1);
	virtual void notifyPacket(pthread_t threadId, int msg, jbyteArray arg1);
private:
	JavaVM* mpjVM;
	jclass mClass;
	jobject mObject;
	jmethodID mMethod;
	jmethodID mMethodVideoPacket;
	jmethodID mMethodAudioPacket;
	jmethodID mMethodPacket;
	pthread_t mjVMThread;
	const char* mListenerName;
	const char* mListenerVideoPacketName;
	const char* mListenerAudioPacketName;
	const char* mListenerPacketName;
	My_Mutex mNotifyLock;
};

#ifdef __cplusplus
 }
#endif

#endif /* LISTENER_H_ */
