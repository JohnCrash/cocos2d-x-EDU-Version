LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

LOCAL_MODULE := json-c_static

LOCAL_MODULE_FILENAME := libjson-c

LOCAL_SRC_FILES := \
	arraylist.c\
	debug.c\
	json_object.c\
	json_tokener.c\
	json_util.c\
	linkhash.c\
	printbuf.c\
	random_seed.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/..

LOCAL_C_INCLUDES := $(LOCAL_PATH)/..
                                 
include $(BUILD_STATIC_LIBRARY)
