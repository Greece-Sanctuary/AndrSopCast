/*
 * flvPacket.h
 *
 *  Created on: Apr 9, 2015
 *      Author: stainberg
 */

#ifndef FLVPACKET_H_
#define FLVPACKET_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "flv.h"
#include "info.h"
#include "mutex.h"

	class FlvPacket
	{
	public:
		FlvPacket();
		~FlvPacket();

		int writeH264Packet(int isKeyFrame, uint8_t* data, int len, uint32_t timestamp, uint8_t* out);
		void setSPSData(uint8_t* sps, int spslen);
		void setPPSData(uint8_t* pps, int ppslen);
		void setVideoResolution(int w, int h);
		void setAudioInfo(uint8_t* audioInfo, int len);
		int writeAACPacket(uint8_t* data, int len, uint32_t timestamp, uint8_t* out);
		void resetPacket();

	private:
		size_t writeFlvHeader(byte* pdata);
		size_t writeFlvTagHeader(byte* pdata, uint8 type, uint32 size, uint32 time_stamp);
		size_t writeH264PacketHeader(byte* pdata, int key_frame, int nalu);
		size_t writeMetaData(byte* pdata, int width, int height);
		size_t writeFlvTagWithVideoHeader(byte* pdata, byte* spsFrame, size_t spsFrameLen, byte* ppsFrame, size_t ppsFrameLen);
		size_t writeFlvTagWithAudioHeader(byte* pdata, const byte* audioInfo, size_t audioInfoSize, uint32 time_stamp);
		size_t writeFlvHeads(byte* data);

	private:
		int _width;
		int _height;
		uint8_t* _pps;
		uint8_t* _sps;
		int _spslen;
		int _ppslen;
		volatile int writenHeadInfo;
		pthread_mutex_t mutex;
		uint8_t* audioHeadinfo;
		int audioLen;
	};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FLVPACKET_H_ */
