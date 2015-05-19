MY_LOCAL_PATH := $(call my-dir)

#-----FAAC-----
include $(CLEAR_VARS)

LOCAL_PATH := $(MY_LOCAL_PATH)

LOCAL_MODULE := faac

LOCAL_SRC_FILES := libfaac/aacquant.c \
	libfaac/backpred.c \
	libfaac/bitstream.c \
	libfaac/channels.c \
	libfaac/fft.c \
	libfaac/filtbank.c \
	libfaac/frame.c \
	libfaac/huffman.c \
	libfaac/ltp.c \
	libfaac/midside.c \
	libfaac/psychkni.c \
	libfaac/tns.c \
	libfaac/util.c

LOCAL_C_INCLUDES := $(MY_LOCAL_PATH)include \
	$(MY_LOCAL_PATH)/include

include $(BUILD_STATIC_LIBRARY)
