LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

LOCAL_MODULE := amrnb_static

LOCAL_MODULE_FILENAME := libamrnb

LOCAL_SRC_FILES := \
	interf_dec.c\
	interf_enc.c\
	sp_dec.c\
	sp_enc.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/..

LOCAL_C_INCLUDES := $(LOCAL_PATH)/..
                                 
include $(BUILD_STATIC_LIBRARY)
