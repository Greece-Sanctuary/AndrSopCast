/*
 * aacEncoder.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: stainberg
 */

#include "aacencoder.h"
#include <assert.h>
#include "../myLog.h"
#include <string.h>

#ifdef TAG
#undef TAG
#endif
#define TAG "aacencoder.cpp"

aacEncoder::aacEncoder()
{
	this->hEncoder = NULL;
	this->pConfiguration = NULL;
	this->nInputSamples = 0;
	this->nMaxOutputBytes = 0;
	this->audioInfo = NULL;
	this->audioInfoLen = 0;
	this->interrupt = 1;
}

aacEncoder::~aacEncoder()
{
	hEncoder = NULL;
	if(audioInfo != NULL)
	{
		free(audioInfo);
		audioInfo = NULL;
	}
}

void aacEncoder::reset()
{
	interrupt = 0;
}

int aacEncoder::init()
{
	hEncoder = faacEncOpen(nSampleRate, nChannels, &nInputSamples, &nMaxOutputBytes);
	assert(hEncoder != NULL);
	pConfiguration = faacEncGetCurrentConfiguration(hEncoder);
	pConfiguration->allowMidside = 1;
	pConfiguration->inputFormat = FAAC_INPUT_16BIT;
	pConfiguration->aacObjectType = LOW;
	pConfiguration->mpegVersion = MPEG4;
	pConfiguration->useTns = true;
	pConfiguration->useLfe = true;
	pConfiguration->outputFormat = 0;
	pConfiguration->quantqual = 100;
	pConfiguration->bandWidth = 80*200;
	pConfiguration->shortctl = SHORTCTL_NORMAL;
	faacEncSetConfiguration(hEncoder, pConfiguration);
	LOGI("nSampleRate = %ld, nChannels = %u, nInputSamples = %ld, nMaxOutputBytes = %ld", nSampleRate, nChannels, nInputSamples, nMaxOutputBytes);
	faacEncGetDecoderSpecificInfo(hEncoder, &audioInfo, &audioInfoLen);
	interrupt = 0;
	return nInputSamples;
}

uint8_t* aacEncoder::getAudioInfo()
{
	return audioInfo;
}

int aacEncoder::getAudioInfoLength()
{
	return audioInfoLen;
}

unsigned long aacEncoder::getInputSamples()
{
	return nInputSamples;
}

void aacEncoder::setAudio(int c, int pcm, int sr)
{
	this->nChannels = c;
	this->nPCMBitSize = pcm;
	this->nSampleRate = sr;
}

void aacEncoder::close()
{
	interrupt = 1;
}

int aacEncoder::aacEncoderProcess(uint8_t* pdata, int pSize, uint8_t* outputBuffer)
{
	if(interrupt)
	{
		faacEncClose(hEncoder);
		return 0;
	}
	unsigned int bytesWritten;
	unsigned int nBufferSize = pSize / (nPCMBitSize / 8);
	bytesWritten = faacEncEncode(hEncoder, (int32_t*)pdata, nBufferSize, outputBuffer, nMaxOutputBytes);
	return bytesWritten;
}

