/*
 * x264encoder.h
 *
 *  Created on: Apr 8, 2015
 *      Author: stainberg
 */

#ifndef X264ENCODER_H_
#define X264ENCODER_H_


extern "C"
{
#include <stdint.h>
#include "x264.h"
#include <assert.h>
#include <stdio.h>

    class X264Encoder
    {
    public:
        X264Encoder();
        ~X264Encoder();

        bool openX264Encoder();
        long x264EncoderProcess(x264_picture_t *pPicture, x264_nal_t **nals, int& nalsCount);
        bool closeX264Encoder();
        int createNalBuffer(uint8_t* pdata, uint8_t* payload, int length);
        void reset();

        void setResolution(unsigned int w, unsigned int h);
        void setBitrate(unsigned int bitrate);
        void setFps(unsigned int fps);
        void setZerolatencyType(unsigned int type);

        void getResolution(int& w, int& h);
        unsigned int getFps();
        void forceIDRFrame();

    private:
        x264_param_t *pParameter;
        x264_t *x264EncoderHandle;
        x264_picture_t *pOutput;

        unsigned int i_bitrate;
        unsigned int i_fps;
        unsigned int width;
        unsigned int height;
        unsigned int zerolatencyType;

        unsigned int current_f_rf_constant;
        unsigned int userSetting_f_rf_constant;

        int64_t frameNo;

        bool isForceIDRFrameEnabled;

    public:
        uint8_t* sps;
		uint8_t* pps;
		int spslen;
		int ppslen;
    };
}

#endif /* X264ENCODER_H_ */
