

######################################################################
#
######################################################################
LOCAL_PATH	:= $(call my-dir)
include $(CLEAR_VARS)

######################################################################
#
######################################################################
CFLAGS := -Werror

LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/ \
		$(LOCAL_PATH)/../ \


LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%)
LOCAL_CFLAGS += -DANDROID_NDK
LOCAL_CFLAGS += -DLOG_NDEBUG
LOCAL_CFLAGS += -DACCESS_RAW_DESCRIPTORS
LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -ldl
LOCAL_LDLIBS += -llog
LOCAL_LDLIBS += -landroid

LOCAL_SHARED_LIBRARIES += usb1.0 uvc
LOCAL_SHARED_LIBRARIES +=libjpeg-turbo

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
		XTest.cpp \

LOCAL_MODULE    := XTest
include $(BUILD_SHARED_LIBRARY)
