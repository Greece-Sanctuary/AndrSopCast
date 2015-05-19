/*
 * flvPacket.c
 *
 *  Created on: Apr 9, 2015
 *      Author: stainberg
 */

#include "flvPacket.h"
#include "../myLog.h"
#ifdef TAG
#undef TAG
#endif
#define TAG "flvPacket.cpp"
#define kTagLength (4)
#define kAVCPacketHeaderSize  (5)
#define kAACPacketHeaderSize (2)
static const byte kAudioDataHeader = 0xAF;
static int stream_buffer_write_offset = 0;
FlvPacket::FlvPacket()
{
	this->_sps = NULL;
	this->_pps = NULL;
	this->_ppslen = 0;
	this->_spslen = 0;
	this->_width = 0;
	this->_height = 0;
	this->writenHeadInfo = 0;

	this->audioHeadinfo = NULL;
	this->audioLen = 0;
	pthread_mutex_init(&mutex, NULL);
}

FlvPacket::~FlvPacket()
{
	free(this->_pps);
	this->_pps = NULL;
	this->_sps = NULL;
	free(this->_sps);
	this->_spslen = 0;
	this->_ppslen = 0;
	this->_width = 0;
	this->_height = 0;
	this->writenHeadInfo = 0;

	this->audioHeadinfo = NULL;
	this->audioLen = 0;
	pthread_mutex_destroy(&mutex);
}

void FlvPacket::resetPacket()
{
	this->writenHeadInfo = 0;
}

void FlvPacket::setSPSData(uint8_t* sps, int spslen)
{
	if(this->_sps == NULL)
	{
		this->_sps = (uint8_t*)malloc(sizeof(uint8_t)*spslen);
	}
	memcpy(this->_sps, sps, spslen);
	this->_spslen = spslen;
}

void FlvPacket::setPPSData(uint8_t* pps, int ppslen)
{
	if(this->_pps == NULL)
		{
			this->_pps = (uint8_t*)malloc(sizeof(uint8_t)*ppslen);
		}
		memcpy(this->_pps, pps, ppslen);
	this->_ppslen = ppslen;
}

void FlvPacket::setVideoResolution(int w, int h)
{
	this->_width = w;
	this->_height = h;
}

void FlvPacket::setAudioInfo(uint8_t* audioInfo, int len)
{
	this->audioHeadinfo = audioInfo;
	this->audioLen = len;
}

size_t FlvPacket::writeFlvHeader(byte* pdata) {
    size_t  index  = 0;
    // 写入 flv header信息
    flv_header header = { };
    memcpy(pdata + index, FLV_SIGNATURE, sizeof(header.signature));
    index += sizeof(header.signature);

    pdata[index++]  =  FLV_VERSION;
    pdata[index++]  =  FLV_FLAG_VIDEO | FLV_FLAG_AUDIO;

    uint32_be offset = swap_uint32(FLV_HEADER_SIZE);
    memcpy(pdata + index, &offset, sizeof(uint32_be));
    index += sizeof(uint32_be);

    byte extend[kTagLength] = { 0x00, 0x00, 0x00, 0x00 };
    memcpy(pdata + index, extend, sizeof(extend));

    return 1 ? FLV_HEADER_SIZE + kTagLength : FLV_HEADER_SIZE;
}

size_t FlvPacket::writeFlvTagHeader(byte* pdata, uint8 type, uint32 size, uint32 time_stamp) {
    flv_tag tag;
    tag.type          =  type;
    tag.body_length   =  uint32_to_uint24_be(size);
    flv_tag_set_timestamp(&tag, time_stamp);
    tag.stream_id     =  uint32_to_uint24_be(0);

    size_t index = 0;

    // tag.type
    pdata[index++] = tag.type;

    // tag.body_length
    memcpy(&pdata[index], &tag.body_length, sizeof(tag.body_length));
    index += sizeof(tag.body_length);

    // tag.timestamp
    memcpy(&pdata[index], &tag.timestamp, sizeof(tag.timestamp));
    index += sizeof(tag.timestamp);

    // tag.timestamp_extended
    memcpy(&pdata[index], &tag.timestamp_extended, sizeof(tag.timestamp_extended));
    index += sizeof(tag.timestamp_extended);

    // tag.stream_id
    memcpy(&pdata[index], &tag.stream_id, sizeof(tag.stream_id));

    return FLV_TAG_SIZE;
}

size_t FlvPacket::writeH264PacketHeader(byte* pdata, int key_frame, int nalu) {
    byte header[kAVCPacketHeaderSize] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    header[0]  = (key_frame ? 0x10 : 0x20) | 0x07;
    header[1]  = nalu ? 0x01 : 0x00;
    memcpy(pdata, header, sizeof(header));
    return sizeof(header);
}

static size_t stream_buffer_write(const void * in_buffer, size_t size, void * user_data) {
    memcpy(user_data + stream_buffer_write_offset, in_buffer, size);
    stream_buffer_write_offset += size;
    return size;
}

size_t FlvPacket::writeMetaData(byte* pdata, int width, int height) {
    flv_metadata meta;
    meta.on_metadata_name = amf_str("onMetaData");
    meta.on_metadata = amf_associative_array_new();
    amf_associative_array_add(meta.on_metadata, "width",
                              amf_number_new(width));
    amf_associative_array_add(meta.on_metadata, "height",
                              amf_number_new(height));
    amf_associative_array_add(meta.on_metadata, "framerate",
                                  amf_number_new(15));
    amf_associative_array_add(meta.on_metadata, "videocodecid",
                              amf_number_new((number64)FLV_VIDEO_TAG_CODEC_AVC));
    amf_associative_array_add(meta.on_metadata, "audiosamplerate",
                                      amf_number_new(44100.00));
    amf_associative_array_add(meta.on_metadata, "audiosamplesize",
                                      amf_number_new(16));
    amf_associative_array_add(meta.on_metadata, "stereo", amf_boolean_new(1));
    amf_associative_array_add(meta.on_metadata, "audiocodecid",
                              amf_number_new((number64)FLV_AUDIO_TAG_SOUND_FORMAT_AAC));
    // create the onMetaData tag
    uint32 on_metadata_name_size = (uint32)amf_data_size(meta.on_metadata_name);
    uint32 on_metadata_size = (uint32)amf_data_size(meta.on_metadata);
    uint32 meta_size = on_metadata_name_size + on_metadata_size;

    size_t buffer_size = meta_size + FLV_TAG_SIZE;
    size_t packet_size = 1 ? buffer_size + kTagLength : buffer_size;
    size_t psize = 0;
    psize += writeFlvTagHeader(pdata, FLV_TAG_TYPE_META, meta_size, 0);

    byte metaName[1024] = {0};
    byte metaData[1024] = {0};

    stream_buffer_write_offset = 0;
    size_t metanamelen = amf_data_write(meta.on_metadata_name, stream_buffer_write, metaName);

    stream_buffer_write_offset = 0;
    size_t metalen = amf_data_write(meta.on_metadata, stream_buffer_write, metaData);

    amf_data_free(meta.on_metadata_name);
    amf_data_free(meta.on_metadata);

    memcpy(pdata+psize, metaName, metanamelen);
    psize += metanamelen;
    memcpy(pdata+psize, metaData, metalen);
    psize += metalen;

    uint32 pre_size = swap_uint32(packet_size-4);//为解决第一个pretagsize多了4个而减去4
    memcpy(pdata+psize, &pre_size, sizeof(uint32));
    pdata += sizeof(uint32);
    return packet_size;
}

size_t FlvPacket::writeFlvTagWithVideoHeader(byte* pdata, byte* spsFrame, size_t spsFrameLen, byte* ppsFrame, size_t ppsFrameLen) {
    // 封装AVC sequence header
    const size_t kExtendSize = 11;
    size_t buffer_size = spsFrameLen + ppsFrameLen + kExtendSize;

    // AVCPacket header size
    size_t body_size = kAVCPacketHeaderSize + buffer_size;
    size_t packet_size = body_size + FLV_TAG_SIZE;
    size_t length = 1 ? packet_size + kTagLength : packet_size;

    size_t psize = 0;

    psize += writeFlvTagHeader(pdata + psize, FLV_TAG_TYPE_VIDEO, body_size, 0);
    psize += writeH264PacketHeader(pdata + psize, 1, 0);

    // AVCDecoderConfigurationRecord
    pdata[psize++] = 0x01;
    pdata[psize++] = spsFrame[1];
    pdata[psize++] = spsFrame[2];
    pdata[psize++] = spsFrame[3];
    pdata[psize++] = 0xff;

    // sps
    pdata[psize++]   = 0xe1;
    pdata[psize++]   = (spsFrameLen >> 8) & 0xff;
    pdata[psize++]   = spsFrameLen & 0xff;
    memcpy(&pdata[psize], spsFrame, spsFrameLen);
    psize += spsFrameLen;

    // pps
    pdata[psize++]   = 0x01;
    pdata[psize++]   = (ppsFrameLen >> 8) & 0xff;
    pdata[psize++]   = ppsFrameLen & 0xff;
    memcpy(&pdata[psize], ppsFrame, ppsFrameLen);
    psize += ppsFrameLen;

    uint32 pre_size = swap_uint32(packet_size);
    memcpy(&pdata[psize], &pre_size, sizeof(uint32));

    return length;
}

size_t FlvPacket::writeFlvTagWithAudioHeader(byte* pdata, const byte* audioInfo, size_t audioInfoSize, uint32 time_stamp) {

    size_t body_size = kAACPacketHeaderSize + audioInfoSize;
    size_t packet_size = body_size + FLV_TAG_SIZE;
    size_t length = 1 ? packet_size + kTagLength : packet_size;
    size_t psize = 0;

    psize += writeFlvTagHeader(pdata + psize, FLV_TAG_TYPE_AUDIO, body_size, time_stamp);

    byte format[kAACPacketHeaderSize] = { kAudioDataHeader, 0x01};
    format[1] = 0x00;
    memcpy(pdata + psize, format, sizeof(format));
    psize += sizeof(format);

    memcpy(pdata + psize, audioInfo, audioInfoSize);
    psize += audioInfoSize;

    uint32 pre_size = swap_uint32(packet_size);
    memcpy(&pdata[psize], &pre_size, sizeof(uint32));

    return length;
}

// 写FLV头
size_t FlvPacket::writeFlvHeads(byte* data) {
    byte* phead = data;
    size_t headlen = 0;

    // 写FLV头
    size_t flvHeadLen = writeFlvHeader(phead);
    phead += flvHeadLen;
    headlen += flvHeadLen;

    // 写 Meta 相关信息
    size_t flvMetaDataLen = writeMetaData(phead, this->_width, this->_height);
    phead += flvMetaDataLen;
    headlen += flvMetaDataLen;

    // 写视频编码头信息
    size_t flvVideoLen = writeFlvTagWithVideoHeader(phead, this->_sps, this->_spslen, this->_pps, this->_ppslen);
	phead += flvVideoLen;
	headlen += flvVideoLen;
	// 写音频编码头信息
	size_t flvAudioLen = writeFlvTagWithAudioHeader(phead, this->audioHeadinfo, this->audioLen, 0);
	phead += flvAudioLen;
	headlen += flvAudioLen;
    // 返回Flv头长度
    return headlen;
}

int FlvPacket::writeAACPacket(uint8_t* data, int len, uint32_t timestamp, uint8_t* out)
{
//	pthread_mutex_lock(&mutex);
	size_t hl = 0;
	size_t written = 0;
	if(!this->writenHeadInfo)
	{
		hl = this->writeFlvHeads(out);
		LOGI("flv head length = %d", hl);
		written += hl;
		this->writenHeadInfo = 1;
	}
	const size_t kAACPacketSize = 2;
	size_t buffer_size = kAACPacketSize + len + FLV_TAG_SIZE;//11 + 2 + len
	written += writeFlvTagHeader(out + written, FLV_TAG_TYPE_AUDIO, len + kAACPacketSize, timestamp);//flv tag 11
	byte format[kAACPacketSize] = { kAudioDataHeader, 0x01};
	memcpy(out + written, format, sizeof(format));
	written += sizeof(format);
	memcpy(out + written, data, len);
	written += len;
	uint32 pre_size = swap_uint32(buffer_size);
	memcpy(out + written, &pre_size, sizeof(uint32));
	written += sizeof(uint32);//11 + 2 + len + pre size
//	pthread_mutex_unlock(&mutex);
	return written;
}

int FlvPacket::writeH264Packet(int isKeyFrame, uint8_t* data, int len, uint32_t timestamp, uint8_t* out)
{
//	pthread_mutex_lock(&mutex);
	size_t hl = 0;
	size_t written = 0;
	if(!this->writenHeadInfo)
	{
		hl = this->writeFlvHeads(out);
		LOGI("flv head length = %d", hl);
		written += hl;
		this->writenHeadInfo = 1;
	}
	const size_t kAVCPacketSize = kAVCPacketHeaderSize + 4;//5 + 4
	size_t buffer_size = kAVCPacketSize + len + FLV_TAG_SIZE;//11 + 5 + 4 + len
	written += writeFlvTagHeader(out + written, FLV_TAG_TYPE_VIDEO, len + kAVCPacketSize, timestamp);//flv tag 11
	written += writeH264PacketHeader(out + written, isKeyFrame, 1);//264 packet tag 11 + 5
	// write length
	size_t size = len;
	byte length[4] = { 0x00, 0x00, 0x00, 0x00 };
	length[0]  = (size >> 24) & 0xff;
	length[1]  = (size >> 16) & 0xff;
	length[2]  = (size >>  8) & 0xff;
	length[3]  = (size >>  0) & 0xff;
	memcpy(out + written, length, sizeof(length));//11 + 5 + 4
	written += sizeof(length);
	// write tag data
	memcpy(out + written, data, size);//11 + 5 + 4 +len
	written += size;
	uint32 pre_size = swap_uint32(buffer_size);
	memcpy(out + written, &pre_size, sizeof(uint32));
	written += sizeof(uint32);//11 + 5 + 4 + len + pre size
//	pthread_mutex_unlock(&mutex);
	return written;
}













