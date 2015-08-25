#include <stdlib.h>
#include <string.h>
#include "x264encoder.h"
#include "../flv/flvPacket.h"
#include "../myLog.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "x264encoder.cpp"

// new version for x264 encoder
X264Encoder::X264Encoder()
{
    i_fps = 15;
    i_bitrate = 500;
    width = 352;
    height = 288;
    zerolatencyType = 0;

    frameNo = 0;
    isForceIDRFrameEnabled = false;

    pParameter = NULL;
    x264EncoderHandle = NULL;
    pOutput = NULL;

    sps = (uint8_t*)malloc(sizeof(uint8_t)*1024);
    pps = (uint8_t*)malloc(sizeof(uint8_t)*1024);
    spslen = 0;
    ppslen = 0;
}

X264Encoder::~X264Encoder()
{
    this->closeX264Encoder();
    free(this->sps);
    free(this->pps);
    sps = NULL;
    pps = NULL;
}

void X264Encoder::reset()
{
	frameNo = 0;
	isForceIDRFrameEnabled = false;
}

void X264Encoder::setResolution(unsigned int w, unsigned int h)
{
    width = w;
    height = h;
}

void X264Encoder::setBitrate(unsigned int bitrate)
{
	i_bitrate = bitrate;
}

void X264Encoder::setFps(unsigned int fps)
{
    i_fps = fps;
}

unsigned int X264Encoder::getFps()
{
	return i_fps;
}

void X264Encoder::setZerolatencyType(unsigned int type){
	zerolatencyType = type;
}

void X264Encoder::getResolution(int& w, int & h)
{
	memcpy(&w, &width, sizeof(int));
	memcpy(&h, &height, sizeof(int));
}

bool X264Encoder::openX264Encoder()
{
    this->closeX264Encoder();
    if(!pParameter)
    {
        pParameter = (x264_param_t *)malloc(sizeof(x264_param_t));
        if (!pParameter) {
            this->closeX264Encoder();
            return false;
        }
        memset(pParameter, 0, sizeof(x264_param_t));
    }
    char* strZerolatencyType = "ultrafast";
    if (zerolatencyType == 0)
    	strZerolatencyType = "ultrafast";
    else if (zerolatencyType == 1)
    	strZerolatencyType = "medium";
    int ret = x264_param_default_preset(pParameter, strZerolatencyType, "zerolatency");
    if (ret != 0) {
        this->closeX264Encoder();
        return false;
    }
    pParameter->i_threads = 1;
    pParameter->b_sliced_threads = 0;
    pParameter->i_sync_lookahead = X264_SYNC_LOOKAHEAD_AUTO;
    pParameter->i_width = width;
	pParameter->i_height = height;
	pParameter->i_frame_total = 0;
	pParameter->b_deterministic = 1;
	pParameter->i_frame_reference = 4;
	pParameter->i_bframe = 0;
	pParameter->i_bframe_pyramid = 0;
	pParameter->i_bframe_adaptive = 0;
	pParameter->b_intra_refresh = 0;
	pParameter->i_csp = X264_CSP_I420;
    pParameter->i_level_idc = 9;
    pParameter->i_keyint_min = 10;
    pParameter->i_keyint_max = 30;
    pParameter->b_repeat_headers = 1;
    pParameter->b_interlaced = 0;
    pParameter->i_cqm_preset = X264_CQM_FLAT;
    pParameter->psz_cqm_file = NULL;
    pParameter->b_aud = 0;
	pParameter->i_nal_hrd = X264_NAL_HRD_NONE;
	pParameter->i_scenecut_threshold  = 40;
	pParameter->i_bframe_bias = 0;
    pParameter->i_fps_num = i_fps;
    pParameter->i_fps_den = 1;
    pParameter->i_timebase_num = 1;
    pParameter->i_timebase_den = 1000000;

    pParameter->analyse.i_weighted_pred = 0;
    pParameter->analyse.b_weighted_bipred = 0;
	pParameter->analyse.b_chroma_me = 1;
	pParameter->analyse.i_trellis = 1;
	pParameter->analyse.i_subpel_refine = 4;
	pParameter->analyse.b_transform_8x8 = 1;
	pParameter->analyse.i_me_range = 8;
	pParameter->analyse.i_me_method = X264_ME_UMH; //2
    pParameter->analyse.i_direct_mv_pred = X264_DIRECT_PRED_TEMPORAL; //2
    pParameter->analyse.intra = 0;
    pParameter->analyse.inter = 0;

    pParameter->b_cabac = 0;
    pParameter->b_vfr_input = 0;

    pParameter->rc.i_rc_method = X264_RC_ABR;//X264_RC_CQP;
    pParameter->rc.f_qcompress = 0.6f;  // 0.0 => cbr, 1.0 => constant qp
    pParameter->rc.i_lookahead = 0;
    pParameter->rc.b_mb_tree = 0;
    pParameter->rc.i_qp_min = 10;
    pParameter->rc.i_qp_max = 51;
    pParameter->rc.i_qp_step = 3;
    pParameter->rc.i_qp_constant = 10;
    pParameter->rc.f_rf_constant = 10;		  // 1pass VBR, nominal QP
    pParameter->rc.i_bitrate = i_bitrate;
    pParameter->rc.i_vbv_max_bitrate = i_bitrate;
    pParameter->rc.i_vbv_buffer_size = i_bitrate;
    pParameter->rc.f_vbv_buffer_init = 0.6f;
    pParameter->rc.f_rate_tolerance = 10 / 100.0f; // In CRF mode,maximum CRF as caused by VBV

    if(x264_param_apply_profile(pParameter, "baseline"))
    {
        this->closeX264Encoder();
        return false;
    }

    if (!x264EncoderHandle) {
        x264EncoderHandle = x264_encoder_open(pParameter);
        assert(x264EncoderHandle != NULL);
    }

    int	nal_count = 0;
	x264_nal_t* nals = NULL;
    x264_encoder_headers(x264EncoderHandle, &nals, &nal_count);
    assert(nal_count > 0);

    for (int index = 0; index < nal_count; ++index)
    {
		if (nals[index].i_type == NAL_SPS) {
			spslen = createNalBuffer(sps, nals[index].p_payload, nals[index].i_payload);
		}
		if (nals[index].i_type == NAL_PPS) {
			ppslen = createNalBuffer(pps, nals[index].p_payload, nals[index].i_payload);
		}
	}

    assert(spslen != 0);
    assert(ppslen != 0);

    if (!pOutput) {
        pOutput = (x264_picture_t *)malloc(sizeof(x264_picture_t));
        if (!pOutput) {
            this->closeX264Encoder();
            return false;
        }
    }
    memset(pOutput, 0, sizeof(x264_picture_t));
    return true;
}

int X264Encoder::createNalBuffer(uint8_t* pdata, uint8_t* payload, int length) {
    // ������������������("00 00 00 01"���"00 00 01")
    if ((payload[0] == 0x00) && (payload[1] == 0x00)
        && (payload[2] == 0x00) && (payload[3] == 0x01)) {  // 00 00 00 01
        payload += 4; length -= 4;
    } else if ((payload[0] == 0x00) && (payload[1] == 0x00)
               && (payload[2] == 0x01)) { // 00 00 01
        payload += 3; length -= 3;
    }

    memcpy(pdata, payload, length);
    return length;
}

void X264Encoder::forceIDRFrame()
{
    isForceIDRFrameEnabled = true;
}

long X264Encoder::x264EncoderProcess(x264_picture_t *pPicture, x264_nal_t **nals, int& nalsCount)
{
    pPicture->i_pts = (int64_t)(frameNo * pParameter->i_fps_den);
    pPicture->i_type = X264_TYPE_AUTO;
    pPicture->i_qpplus1 = 0;//X264_QP_AUTO;

    if (isForceIDRFrameEnabled) {
        pPicture->i_type = X264_TYPE_IDR;
        isForceIDRFrameEnabled = false;
    }

    int32_t framesize = -1;

    framesize = x264_encoder_encode(x264EncoderHandle, nals, &nalsCount, pPicture, pOutput);

    if (framesize>0) {
        frameNo++;
    }

    return framesize;
}

bool X264Encoder::closeX264Encoder()
{
    if (pOutput) {
        free(pOutput);
        pOutput = NULL;
    }
    if (pParameter) {
        free(pParameter);
        pParameter = NULL;
    }
    if (x264EncoderHandle) {
        x264_encoder_close(x264EncoderHandle);
        x264EncoderHandle = NULL;
    }
    return true;
}
