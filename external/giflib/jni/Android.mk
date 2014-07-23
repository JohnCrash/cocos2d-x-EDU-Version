LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

LOCAL_MODULE := gif_static

LOCAL_MODULE_FILENAME := libgif

LOCAL_SRC_FILES := \
	dgif_lib.c\
	gif.c\
	gif_hash.c\
	gifalloc.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/..

LOCAL_C_INCLUDES := $(LOCAL_PATH)/..
                                 
include $(BUILD_STATIC_LIBRARY)
