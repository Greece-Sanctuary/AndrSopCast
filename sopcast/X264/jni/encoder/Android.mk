LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../x264 \
					$(LOCAL_PATH)/beautify \
					$(LOCAL_PATH)/../flv \
    				$(LOCAL_PATH)
    
LOCAL_MODULE    := x264Encoder
LOCAL_SRC_FILES :=  beautify/bilateral_filter.cpp \
					x264encoder.cpp \
					aacencoder.cpp \
					com_youku_x264_X264Encoder.cpp \
					../flv/flv.c \
					../flv/amf.c \
					../flv/avc.c \
					../flv/info.c \
					../flv/types.c \
					../flv/flvPacket.cpp \
					listener.cpp

#LOCAL_SHARED_LIBRARIES := libfaac libyuv
LOCAL_LDLIBS := -L$(LOCAL_PATH)/../x264 -lx264 -llog -L$(LOCAL_PATH)/../prebuilt -lfaac -L$(LOCAL_PATH)/../prebuilt -lyuv

include $(BUILD_SHARED_LIBRARY)
