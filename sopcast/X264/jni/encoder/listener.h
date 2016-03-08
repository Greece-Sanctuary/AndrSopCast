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
	virtual void notifyAVC(pthread_t threadId, jbyteArray arg1, int len,int width, int height, jbyteArray tag);
	virtual void notifyPacket(pthread_t threadId, int msg, jbyteArray arg1);
private:
	JavaVM* mpjVM;
	jclass mClass;
	jobject mObject;
	jmethodID mMethod;
	jmethodID mMethodAVC;
	jmethodID mMethodPacket;
	pthread_t mjVMThread;
	const char* mListenerName;
	const char* mListenerAVCName;
	const char* mListenerPacketName;
	My_Mutex mNotifyLock;
};

#ifdef __cplusplus
 }
#endif

#endif /* LISTENER_H_ */
