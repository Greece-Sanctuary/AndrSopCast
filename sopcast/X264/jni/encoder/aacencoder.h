/*
 * aacencoder.h
 *
 *  Created on: Apr 13, 2015
 *      Author: stainberg
 */

#ifndef AACENCODER_H_
#define AACENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "faac.h"
#include "faaccfg.h"
#include "../flv/types.h"

class aacEncoder
{
	public:
		aacEncoder();
		~aacEncoder();

		void setAudio(int c, int pcm, int sr);
		int init();
		int aacEncoderProcess(uint8_t* pdata, int pSize, uint8_t* outputBuffer);

		uint8_t* getAudioInfo();
		int getAudioInfoLength();
		unsigned long getInputSamples();
		void close();
		void reset();

	private:
		unsigned int nChannels;
		int nPCMBitSize;
		unsigned long nSampleRate;
		faacEncHandle hEncoder;
		faacEncConfigurationPtr pConfiguration;

		unsigned long nInputSamples;
		unsigned long nMaxOutputBytes;

		uint8_t* audioInfo;
		unsigned long audioInfoLen;
		int interrupt;
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AACENCODER_H_ */
