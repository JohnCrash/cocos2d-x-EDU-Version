LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_static
LOCAL_MODULE_FILENAME := avdevice
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavdevice.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/. $(LOCAL_PATH)/../../include
LOCAL_STATIC_LIBRARIES += avcodec_static
LOCAL_STATIC_LIBRARIES += avfilter_static
LOCAL_STATIC_LIBRARIES += avformat_static
LOCAL_STATIC_LIBRARIES += swresample_static
LOCAL_STATIC_LIBRARIES += swscale_static
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := avcodec_static
LOCAL_MODULE_FILENAME := avcodec
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavcodec.a
LOCAL_STATIC_LIBRARIES += avutil_static
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := avfilter_static
LOCAL_MODULE_FILENAME := avfilter
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavfilter.a
LOCAL_STATIC_LIBRARIES += avutil_static
LOCAL_STATIC_LIBRARIES += avcodec_static
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := avformat_static
LOCAL_MODULE_FILENAME := avformat
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavformat.a
LOCAL_STATIC_LIBRARIES += avutil_static
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := avutil_static
LOCAL_MODULE_FILENAME := avutil
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := swresample_static
LOCAL_MODULE_FILENAME := swresample
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := swscale_static
LOCAL_MODULE_FILENAME := swscale
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)