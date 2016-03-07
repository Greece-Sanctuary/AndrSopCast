/*
 * com_youku_x264_X264Encoder.cpp
 *
 *  Created on: Apr 8, 2015
 *      Author: stainberg
 */

#include "x264encoder.h"
#include "../myLog.h"
#include "../performance.h"
#include <jni.h>
#include <assert.h>
#include <string.h>
#include "../flv/flvPacket.h"
#include "aacencoder.h"
#include <pthread.h>
#include "listener.h"
#include "proto_common.h"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "mutex.h"
#include "com_youku_x264_X264Encoder.h"
#include "watermark.h"
#ifdef TAG
#undef TAG
#endif
#define TAG "com_youku_x264_X264Encoder.cpp"

#include "beautify/bilateral_filter.h"

static const char* mClassPathName = "com/youku/x264/X264Encoder";
static Listener* listener = NULL;
static X264Encoder* x264Encoder = NULL;
static aacEncoder* AACEncoder = NULL;
//static BilateralFilter* bilfilter = NULL;
static FlvPacket* flvPacketHandler = NULL;
static x264_picture_t* inputPicture = NULL;
static int width = 0, height = 0;
static int yuvW = 0, yuvH = 0;
static long timestemp = 0;
static long oldtimestemp = 0;
static uint32_t diff = 0;
static uint8_t* audioBuffer;
static int samples;
static uint8_t* remindAudioBuffer;
static int remindAudioLen;
static JavaVM* jVM = NULL;
static volatile int run = 0;
static int _streamId = 0;
static uint8_t* srcy;
static uint8_t* srcu;
static uint8_t* srcv;
static uint8_t* dsty;
static uint8_t* dstu;
static uint8_t* dstv;
static uint8_t* rgbbuffer;
static pthread_mutex_t mutex;
static int audioInit = 0;
using namespace libyuv;


static int AudioCompress(uint8_t* data, int size, uint8_t* outData);

static long getCurrentTime()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static uint32_t getTimestamp(int type)
{
	timestemp = (long)getCurrentTime();
	if(oldtimestemp == 0)//first in
	{
		oldtimestemp = timestemp;
		diff = 0;
	}
	else //second
	{
		long t = timestemp - oldtimestemp;
		oldtimestemp = timestemp;
		diff += t;
	}
	return diff;
}

static void
com_youku_x264_X264Encoder_init(JNIEnv* env, jobject thiz) {
	if(x264Encoder != NULL) {
		x264Encoder->openX264Encoder();
	}
	x264_picture_alloc(inputPicture, X264_CSP_I420, width, height);
	srcy = (uint8_t*)malloc(sizeof(uint8_t)*1280*720);
	srcu = (uint8_t*)malloc(sizeof(uint8_t)*1280*720/4);
	srcv = (uint8_t*)malloc(sizeof(uint8_t)*1280*720/4);
	dsty = (uint8_t*)malloc(sizeof(uint8_t)*1280*720);
	dstu = (uint8_t*)malloc(sizeof(uint8_t)*1280*720/4);
	dstv = (uint8_t*)malloc(sizeof(uint8_t)*1280*720/4);
	flvPacketHandler->setPPSData(x264Encoder->pps, x264Encoder->ppslen);
	flvPacketHandler->setSPSData(x264Encoder->sps, x264Encoder->spslen);
	flvPacketHandler->setVideoResolution(width, height);
	listener->notify(pthread_self(), 10, NULL, 0, NULL);


}

static void
com_youku_x264_X264Encoder_setup(JNIEnv* env, jobject thiz, jobject weak_thiz) {
	x264Encoder = new X264Encoder();
	AACEncoder = new aacEncoder();
	flvPacketHandler = new FlvPacket();
	inputPicture = new x264_picture_t();
//	bilfilter = new BilateralFilter();
	listener = new Listener(jVM, thiz, weak_thiz);
	pthread_mutex_init(&mutex, NULL);

}

static void
com_youku_x264_X264Encoder_finalize(JNIEnv* env, jobject thiz)
{
	run = 0;
	if(x264Encoder != NULL)
	{
		x264Encoder->closeX264Encoder();
		delete x264Encoder;
		x264Encoder = NULL;
	}
	if(AACEncoder != NULL)
	{
		AACEncoder->close();
		delete AACEncoder;
		AACEncoder = NULL;
	}
	if(inputPicture != NULL)
	{
		x264_picture_clean(inputPicture);
		delete inputPicture;
		inputPicture = NULL;
	}
	if(listener != NULL)
	{
		delete listener;
		listener = NULL;
	}
	if(srcy != NULL)
	{
		free(srcy);
		srcy = NULL;
	}
	if(srcu != NULL)
	{
		free(srcu);
		srcu = NULL;
	}
	if(srcv != NULL)
	{
		free(srcv);
		srcv = NULL;
	}
	if(audioBuffer != NULL)
	{
		free(audioBuffer);
		audioBuffer = NULL;
	}
	if(remindAudioBuffer != NULL)
	{
		free(remindAudioBuffer);
		remindAudioBuffer = NULL;
	}
//	if (bilfilter != NULL){
//		delete bilfilter;
//		bilfilter = NULL;
//	}
	pthread_mutex_destroy(&mutex);
}

static void
com_youku_x264_X264Encoder_setBitrate(JNIEnv* env, jobject thiz, jint bitrate)
{
	if(x264Encoder != NULL)
	{
		x264Encoder->setBitrate(bitrate);
	}
}

static void
com_youku_x264_X264Encoder_setZerolatencyType(JNIEnv* env, jobject thiz, jint zerotype)
{
	if(x264Encoder != NULL)
	{
		x264Encoder->setZerolatencyType(zerotype);
	}
}

static void
com_youku_x264_X264Encoder_setWaterMarkLogo(JNIEnv* env, jobject thiz, jint zerotype)
{

}


static void
com_youku_x264_X264Encoder_setResolution(JNIEnv* env, jobject thiz, jint w, jint h, jint wd, jint hd)
{
	if(x264Encoder != NULL)
	{
		yuvW = w;//352
		yuvH = h;//288
		width = h;//288
		height = width/wd*hd;
		rgbbuffer = (uint8_t*)malloc(sizeof(uint8_t)*w*h*4);
//		height = w;//352
		x264Encoder->setResolution(width, height);
		LOGI("setResolution, w=%d,h=%d", width, height);

//		bilfilter->InitBilateralFilter(0.09, 10.0/255, 360, 640);
	}
}

static void
com_youku_x264_X264Encoder_setFps(JNIEnv* env, jobject thiz, jint fps)
{
	if(x264Encoder != NULL)
	{
		LOGI("setFps, fps=%d", fps);
		x264Encoder->setFps(fps);
	}
}

static void
com_youku_x264_X264Encoder_CompressBuffer(JNIEnv* env, jobject thiz, jbyteArray in, jint insize, int rotate, int w, int h)
{
	if(!run)
		return;
	if(x264Encoder != NULL)
	{
//		double time = (double)clock()/1000000.0;
		uint8_t tag[5];
		uint32_t timestemp = getTimestamp(0);
		uint8_t srcData[1024*50];
		int len = 0;
		x264_nal_t *p_nals = NULL;
		int nalsCount = 0;
		uint8_t* data = (uint8_t*)env->GetByteArrayElements(in, 0);
		int nPicSize = w * h;
		int destW = 0, destH = 0;
		memcpy(srcy, data, nPicSize);
		for (int i=0;i<nPicSize/4;i++)
		{
			*(srcu+i)=*(data+nPicSize+i*2);
			*(srcv+i)=*(data+nPicSize+i*2+1);
		}
		destW = yuvW ;
		destH = h*yuvW/w;

		int offsetline = (destH-yuvH)/4;

		if(rotate == 90)
		{

//			LOGI("CompressBuffer, w=%d, h=%d, destW=%d, destH=%d, yuvW=%d, yuvH=%d, height=%d", w, h, destW, destH,yuvW,yuvH,height);
			I420Scale(srcy, w,
				srcu, w>>1,
				srcv, w>>1,
				w, h,
				dsty, destW,
				dstu, destW>>1,
				dstv, destW>>1,
				destW, destH,
				kFilterBox);
			for(int i=0; i<yuvH/2; i++)
			{
				memcpy(srcy+(height*i*2), dsty+(destW*(i+offsetline)*2), height);
				memcpy(srcy+(height*(i*2+1)), dsty+(destW*((i+offsetline)*2+1)), height);
				memcpy(srcu+(height/2*i), dstu+(destW/2*(i+offsetline)), height/2);
				memcpy(srcv+(height/2*i), dstv+(destW/2*(i+offsetline)), height/2);
			}

			I420Rotate(srcy, height,
				srcu, height>>1,
				srcv, height>>1,
				inputPicture->img.plane[0], inputPicture->img.i_stride[0],
				inputPicture->img.plane[2], inputPicture->img.i_stride[2],
				inputPicture->img.plane[1], inputPicture->img.i_stride[1], height, width, kRotate90);
/*
			//test yuv->rgb
			I420ToRGB24(inputPicture->img.plane[0], inputPicture->img.i_stride[0],
					inputPicture->img.plane[2], inputPicture->img.i_stride[2],
					inputPicture->img.plane[1], inputPicture->img.i_stride[1],
					rgbbuffer, yuvH*4, yuvH, yuvW);

			//add logo watermark

//			unsigned char* logobuf = new unsigned char[40*40*4];
//			memset(logobuf, 100, 40*40*4);
//			LOGI("CompressBuffer, yuvW=%d, yuvH=%d",yuvW,yuvH);

//			int ret = GenerateLogo(rgbbuffer,yuvH, yuvW, logobuf, 40, 40, 10, 10);

//			for (int i=0; i<360; i++){
//				for (int j=0; j<640; j++)
//				{
//					rgbbuffer[(i*640 + j)*4+1] = 255-rgbbuffer[(i*640 + j)*4+1];
//					rgbbuffer[(i*640 + j)*4+2] = 255-rgbbuffer[(i*640 + j)*4+2];
//					rgbbuffer[(i*640 + j)*4+0] = 255-rgbbuffer[(i*640 + j)*4+0];
//				}
//			}

			//美白cpu代码
//			bilfilter->SetProcessImage(rgbbuffer, 0.09, 0.2*15/255);
//			bilfilter->Process();
//			bilfilter->GetProcessImage(rgbbuffer);

			//test rgb->yuv
			RGB24ToI420(rgbbuffer,yuvH*4,
					inputPicture->img.plane[0], inputPicture->img.i_stride[0],
					inputPicture->img.plane[2], inputPicture->img.i_stride[2],
					inputPicture->img.plane[1], inputPicture->img.i_stride[1],
					yuvH, yuvW);
*/
		}
		else if(rotate == 270)
		{
			I420Scale(srcy, w,
				srcu, w>>1,
				srcv, w>>1,
				w, h,
				dsty, destW,
				dstu, destW>>1,
				dstv, destW>>1,
				destW, destH,
				kFilterBox);
			for(int i=0; i<yuvH/2; i++)
			{
				memcpy(srcy+(height*i*2), dsty+(destW*(i+offsetline)*2)+(destW-height), height);
				memcpy(srcy+(height*(i*2+1)), dsty+(destW*((i+offsetline)*2+1))+(destW-height), height);
				memcpy(srcu+(height/2*i), dstu+(destW/2*(i+offsetline))+(destW-height)/2, height/2);
				memcpy(srcv+(height/2*i), dstv+(destW/2*(i+offsetline))+(destW-height)/2, height/2);
			}
			I420Mirror(srcy, height,
				srcu, height>>1,
				srcv, height>>1,
				dsty, height,
				dstu, height>>1,
				dstv, height>>1,
				height, width);
			I420Rotate(dsty, height,
				dstu, height>>1,
				dstv, height>>1,
				inputPicture->img.plane[0], inputPicture->img.i_stride[0],
				inputPicture->img.plane[2], inputPicture->img.i_stride[2],
				inputPicture->img.plane[1], inputPicture->img.i_stride[1], height, width, kRotate90);
		}

		x264Encoder->x264EncoderProcess(inputPicture, &p_nals, nalsCount);
		if(p_nals)
		{
			for(int i=0; i<nalsCount; i++)
			{
				if (p_nals[i].i_type == NAL_SLICE_IDR)
				{
					tag[0] = 0x01;
					len = x264Encoder->createNalBuffer(srcData, p_nals[i].p_payload, p_nals[i].i_payload);
//					packetSize += flvPacketHandler->writeH264Packet(1, srcData, len, timestemp, outData);//++videoTimestemp*1000/x264Encoder->getFps()
				}
				else if(p_nals[i].i_type == NAL_SLICE)
				{
					tag[0] = 0x02;
					len = x264Encoder->createNalBuffer(srcData, p_nals[i].p_payload, p_nals[i].i_payload);
				}
				else
				{
					len = 0;
				}
				if(len > 0)
				{
					memcpy(tag + 1, &timestemp, sizeof(uint32_t));
					jbyteArray jarrayTag = env->NewByteArray(5);
					env->SetByteArrayRegion(jarrayTag, 0, 5, (jbyte*)tag);
					jbyteArray jarray = env->NewByteArray(len);
					env->SetByteArrayRegion(jarray, 0, len, (jbyte*)srcData);
					listener->notify(pthread_self(), 11, jarray, len, jarrayTag);
				}
			}
		}
		env->ReleaseByteArrayElements(in, (jbyte*)data, JNI_ABORT);
//		double duration = (double)clock()/1000000.0 - time;
//		LOGI("diff time = %f", duration);
	}
}

static void
com_youku_x264_X264Encoder_AudioInit(JNIEnv* env, jobject thiz)
{
	if(AACEncoder != NULL)
	{
		samples = AACEncoder->init()*2;
		flvPacketHandler->setAudioInfo(AACEncoder->getAudioInfo(), AACEncoder->getAudioInfoLength());
//		LOGI("AudioInfo, AudioInfo addr=0x%x, length=%d", AACEncoder->getAudioInfo(), AACEncoder->getAudioInfoLength());
	}
	remindAudioBuffer = (uint8_t*)malloc(sizeof(uint8_t)*samples);
	remindAudioLen = 0;
	audioBuffer = (uint8_t*)malloc(sizeof(uint8_t)*1024*50);
	listener->notify(pthread_self(), 20, NULL, 0, NULL);
}

static void
com_youku_x264_X264Encoder_AudioSet(JNIEnv* env, jobject thiz, jint channels, jint pcmBitRate, jint sampleRate)
{
	if(AACEncoder != NULL)
	{
		AACEncoder->setAudio(channels, pcmBitRate, sampleRate);
	}
}

static void
com_youku_x264_X264Encoder_AudioCompress(JNIEnv* env, jobject thiz, jbyteArray in, jint insize)
{
	if(!run)
		return;
	if(AACEncoder != NULL)
	{
		uint8_t tag[5];
		uint8_t* pAudioBuffer = audioBuffer;
//		int aacLen = 0;
		uint8_t* data = (uint8_t*)env->GetByteArrayElements(in, 0);
		if(remindAudioLen > 0)
		{
			memcpy(pAudioBuffer, remindAudioBuffer, remindAudioLen);
		}
		memcpy(pAudioBuffer + remindAudioLen, data, insize);
		insize += remindAudioLen;
		env->ReleaseByteArrayElements(in, (jbyte*)data, JNI_ABORT);
		while(samples <= insize)
		{
			uint8_t aacData[1024*10];
//			uint8_t* paacData = aacData;
			uint32_t timestemp = getTimestamp(1);
			int len = AACEncoder->aacEncoderProcess(pAudioBuffer, samples, aacData);
			pAudioBuffer += samples;
			insize = insize - samples;
//			paacData += len;
//			aacLen += len;
			if(len > 0)
			{
				tag[0] = 0x00;
				memcpy(tag + 1, &timestemp, sizeof(uint32_t));
				jbyteArray jarrayTag = env->NewByteArray(5);
				env->SetByteArrayRegion(jarrayTag, 0, 5, (jbyte*)tag);
				jbyteArray jarray = env->NewByteArray(len);
				env->SetByteArrayRegion(jarray, 0, len, (jbyte*)aacData);
				listener->notify(pthread_self(), 21, jarray, len, jarrayTag);
			}
		}
		remindAudioLen = insize;
		if(remindAudioLen > 0)
		{
			memcpy(remindAudioBuffer, pAudioBuffer, remindAudioLen);
		}
	}
}

static void
com_youku_x264_X264Encoder_writeFLV(JNIEnv* env, jobject thiz, jbyteArray in, jint insize, jbyteArray tag, int withHeader)
{
	pthread_mutex_lock(&mutex);
	size_t head_size = sizeof(proto_header) + sizeof(u2r_streaming);
	uint8_t type;
	uint32_t timestemp;
	int packetSize = 0;
	size_t packet_size = 0;
	uint8_t packetData[1024*50];
	uint8_t header[head_size];
	uint8_t* _in = (uint8_t*)env->GetByteArrayElements(in, 0);
	uint8_t* _tag = (uint8_t*)env->GetByteArrayElements(tag, 0);
	memcpy(&type, _tag, sizeof(uint8_t));
	memcpy(&timestemp, _tag + 1, sizeof(uint32_t));
	if(insize > 0)
	{
		if(type == 1 || type == 2)
		{
			if(audioInit == 0)
			{
				env->ReleaseByteArrayElements(in, (jbyte*)_in, JNI_ABORT);
				env->ReleaseByteArrayElements(tag, (jbyte*)_tag, JNI_ABORT);
				pthread_mutex_unlock(&mutex);
				return;
			}
			if(type == 1)
				packetSize = flvPacketHandler->writeH264Packet(1, _in, insize, timestemp, packetData);
			if(type == 2)
				packetSize = flvPacketHandler->writeH264Packet(0, _in, insize, timestemp, packetData);
		}
		else if(type == 0)
		{
			packetSize = flvPacketHandler->writeAACPacket(_in, insize, timestemp, packetData);
			audioInit = 1;
		}
		//head
		if(withHeader)
		{
			packet_size =  head_size + packetSize;
			proto_header hdr;
			memset(&hdr, 0, sizeof(hdr));
			hdr.magic     =  255;
			hdr.cmd		  =  htons(CMD_U2R_STREAMING);
			hdr.size      =  htonl(packet_size);
			hdr.version	  =  1;

			u2r_streaming req;
			memset(&req, 0, sizeof(req));
			req.streamid		= htonl(_streamId);
			req.payload_type	= PAYLOAD_TYPE_FLV;
			req.payload_size	= htonl(packetSize);
			memcpy(header, &hdr, sizeof(hdr));
			memcpy(header+sizeof(hdr), &req, sizeof(req));
		}
		else
		{
			packet_size = packetSize;
			head_size = 0;
		}
	}
	env->ReleaseByteArrayElements(in, (jbyte*)_in, JNI_ABORT);
	env->ReleaseByteArrayElements(tag, (jbyte*)_tag, JNI_ABORT);
	jbyteArray jarray = env->NewByteArray(packet_size);
	if(head_size > 0)
	{
		env->SetByteArrayRegion(jarray, 0, head_size, (jbyte*)header);
	}
	env->SetByteArrayRegion(jarray, head_size, packetSize, (jbyte*)packetData);
	pthread_mutex_unlock(&mutex);
	listener->notifyPacket(pthread_self(), 30, jarray);
}

static jbyteArray
com_youku_x264_X264Encoder_handShake(JNIEnv* env, jobject thiz, jint streamId, jint userID, jstring token)
{
	const char *_token = env->GetStringUTFChars(token, NULL);
	size_t packet_size = sizeof(proto_header) +	sizeof(u2r_req_state);
	proto_header hdr;
	memset(&hdr, 0, sizeof(hdr));
	hdr.magic     =  255;
	hdr.cmd		  =  htons(CMD_U2R_REQ_STATE);
	hdr.size      =  htonl(packet_size);
	hdr.version	  =  1;

	u2r_req_state req;
	memset(&req, 0, sizeof(req));
	req.version	  = htonl(1);
	req.streamid  = htonl(streamId);
	req.user_id	  = htonl(userID);
	req.payload_type = PAYLOAD_TYPE_FLV;
	memcpy(req.token, _token, strlen(_token));
	uint8_t sendBuffer [1024];
	memcpy(sendBuffer, &hdr, sizeof(hdr));
	memcpy(sendBuffer+sizeof(hdr), &req, sizeof(req));
	env->ReleaseStringUTFChars(token, _token);
	jbyteArray jarray = env->NewByteArray(packet_size);
	env->SetByteArrayRegion(jarray, 0, packet_size, (jbyte*)sendBuffer);
	return jarray;
}

static jint
com_youku_x264_X264Encoder_checkKey(JNIEnv* env, jobject thiz, jbyteArray in, jint insize, jint streamId)
{
	proto_header header;
	u2r_rsp_state rsp;
	uint8_t* _in = (uint8_t*)env->GetByteArrayElements(in, 0);
	memcpy(&header, _in, sizeof(proto_header));
	memcpy(&rsp, _in+sizeof(proto_header), sizeof(rsp));
	env->ReleaseByteArrayElements(in, (jbyte*)_in, JNI_ABORT);
	header.version = header.version;
	header.cmd = NTOHS(header.cmd);
	header.magic = header.magic;
	header.size = NTOHL(header.size);

	rsp.result = NTOHS(rsp.result );
	rsp.streamid = NTOHL(rsp.streamid);
	if(streamId == rsp.streamid && rsp.result == 0 && CMD_U2R_RSP_STATE == header.cmd)
	{
		_streamId = streamId;
		return 1;
	}
	else
	{
		return 0;
	}

}

int jniRegisterNativeMethods(JNIEnv* env, const JNINativeMethod* methods, int count) {
	jclass clazz;
	int ret = -1;
	clazz = env->FindClass(mClassPathName);
	if(clazz == NULL) {
		return ret;
	}
	if((ret = env->RegisterNatives(clazz,methods, count)) < 0) {
		return ret;
	}
	return ret;
}

static void
com_youku_x264_X264Encoder_stop(JNIEnv* env, jobject thiz)
{
	run = 0;
	oldtimestemp = 0;
	timestemp = 0;
	_streamId = 0;
	remindAudioLen = 0;
	audioInit = 0;
	diff = 0;
	x264Encoder->reset();
	AACEncoder->reset();
	flvPacketHandler->resetPacket();
}

static void
com_youku_x264_X264Encoder_prepare(JNIEnv* env, jobject thiz)
{
	run = 1;
}

static JNINativeMethod mMethods[] = {//method for JAVA. use this to register native method
		{"native_init", "()V", (void*) com_youku_x264_X264Encoder_init},
		{"native_setup", "(Ljava/lang/Object;)V", (void*) com_youku_x264_X264Encoder_setup},
		{"native_finalize", "()V", (void*) com_youku_x264_X264Encoder_finalize},
		{"native_setBitrate", "(I)V", (void*)com_youku_x264_X264Encoder_setBitrate},
		{"native_setZerolatencyType", "(I)V", (void*)com_youku_x264_X264Encoder_setZerolatencyType},
		{"native_setResolution", "(IIII)V", (void*)com_youku_x264_X264Encoder_setResolution},
		{"native_setFps", "(I)V", (void*)com_youku_x264_X264Encoder_setFps},
		{"native_compress", "([BIIII)V", (void*)com_youku_x264_X264Encoder_CompressBuffer},
		{"native_audioInit", "()V", (void*) com_youku_x264_X264Encoder_AudioInit},
		{"native_audioSet", "(III)V", (void*)com_youku_x264_X264Encoder_AudioSet},//Channels, PCMBitSize, SampleRate
		{"native_audioCompress", "([BI)V", (void*)com_youku_x264_X264Encoder_AudioCompress},
		{"native_writeFLV", "([BI[BI)V", (void*)com_youku_x264_X264Encoder_writeFLV},
		{"native_handshake", "(IILjava/lang/String;)[B", (void*)com_youku_x264_X264Encoder_handShake},
		{"native_checkKey", "([BII)I", (void*)com_youku_x264_X264Encoder_checkKey},
		{"native_stop", "()V", (void*)com_youku_x264_X264Encoder_stop},
		{"native_prepare", "()V", (void*)com_youku_x264_X264Encoder_prepare}
};

int registerMethods(JNIEnv* env) {
	JavaVM* jVM;
	env->GetJavaVM((JavaVM**)&jVM);
	return jniRegisterNativeMethods(env, mMethods, sizeof(mMethods)/sizeof(mMethods[0]));
}

int JNI_OnLoad(JavaVM* vm, void* reserved)
{//JNI main
	jVM = vm;
	JNIEnv* env = NULL;
	jint ret = JNI_ERR;
	if(vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("GetEnv failed!");
		return ret;
	}
	assert(env != NULL);
	if(registerMethods(env) != JNI_OK) {
		LOGE("can not load methods!");
		return ret;
	}
	ret = JNI_VERSION_1_4;
	LOGI("Loaded!");
	return ret;
}


