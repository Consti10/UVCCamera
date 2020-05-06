#/*
# * UVCCamera
# * library and sample to access to UVC web camera on non-rooted Android device
# * 
# * Copyright (c) 2015-2017 saki t_saki@serenegiant.com
# * 
# * File name: Android.mk
# * 
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# *  You may obtain a copy of the License at
# * 
# *     http://www.apache.org/licenses/LICENSE-2.0
# * 
# *  Unless required by applicable law or agreed to in writing, software
# *  distributed under the License is distributed on an "AS IS" BASIS,
# *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# *  See the License for the specific language governing permissions and
# *  limitations under the License.
# * 
# * All files in the folder are under this Apache License, Version 2.0.
# * Files in the jni/libjpeg-turbo1500, jni/libusb, jin/libuvc, jni/rapidjson folder may have a different license, see the respective files.
#*/
######################################################################
# libjpeg-turbo1500_static.a
######################################################################
LOCAL_PATH		:= $(call my-dir)
include $(CLEAR_VARS)

# CFLAGS := -Werror

#生成するモジュール名
LOCAL_MODULE    := jpeg-turbo1500_static

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
CONSTI_ARCH_DEPENDENT_SIMD_NAME := simd/arm
else ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
CONSTI_ARCH_DEPENDENT_SIMD_NAME := simd/arm64
endif

#インクルードファイルのパスを指定
LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/ \
        $(LOCAL_PATH)/include \
        $(LOCAL_PATH)/$(CONSTI_ARCH_DEPENDENT_SIMD_NAME) \

LOCAL_EXPORT_C_INCLUDES := \
		$(LOCAL_PATH)/ \
        $(LOCAL_PATH)/include \
        $(LOCAL_PATH)/$(CONSTI_ARCH_DEPENDENT_SIMD_NAME) \

#コンパイラのオプションフラグを指定
LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%)
LOCAL_CFLAGS += -DANDROID_NDK

#リンクするライブラリを指定(静的モジュールにする時は不要)
#LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -ldl	# to avoid NDK issue(no need for static library)

#LOCAL_LDLIBS += -llog
LOCAL_EXPORT_LDLIBS += -llog

#このモジュールを外部モジュールとしてリンクする時のライブラリを指定

LOCAL_ARM_MODE := arm

LOCAL_ASMFLAGS += -DELF

# コンパイル・リンクするソースファイル

LOCAL_SRC_FILES += \
	jcapimin.c \
	jcapistd.c \
	jccoefct.c \
	jccolor.c \
	jcdctmgr.c \
	jchuff.c \
	jcinit.c \
	jcmainct.c \
	jcmarker.c \
	jcmaster.c \
	jcomapi.c \
	jcparam.c \
	jcphuff.c \
	jcprepct.c \
	jcsample.c \
	jctrans.c \
	jdapimin.c \
	jdapistd.c \
	jdatadst.c \
	jdatasrc.c \
	jdcoefct.c \
	jdcolor.c \
	jddctmgr.c \
	jdhuff.c \
	jdinput.c \
	jdmainct.c \
	jdmarker.c \
	jdmaster.c \
	jdmerge.c \
	jdphuff.c \
	jdpostct.c \
	jdsample.c \
	jdtrans.c \
	jerror.c \
	jfdctflt.c \
	jfdctfst.c \
	jfdctint.c \
	jidctflt.c \
	jidctfst.c \
	jidctint.c \
	jidctred.c \
	jquant1.c \
	jquant2.c \
	jutils.c \
	jmemmgr.c \
	jmemnobs.c \

LOCAL_SRC_FILES += \
	jaricom.c \
	jcarith.c \
	jdarith.c \

LOCAL_SRC_FILES += \
	turbojpeg.c \
	transupp.c \
	jdatadst-tj.c \
	jdatasrc-tj.c \

LOCAL_SRC_FILES += \
    cdjpeg.h \
	rdbmp.c \
	wrbmp.c \
	rdppm.c \
	wrppm.c \

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
#NEONを有効にする時
#LOCAL_ARM_NEON := true
LOCAL_SRC_FILES += $(CONSTI_ARCH_DEPENDENT_SIMD_NAME)/jsimd.c  $(CONSTI_ARCH_DEPENDENT_SIMD_NAME)/jsimd_neon.S

LOCAL_CFLAGS += \
	-DSIZEOF_SIZE_T=4 \

else ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
#NEONを有効にする時
#LOCAL_ARM_NEON := true
LOCAL_SRC_FILES +=  $(CONSTI_ARCH_DEPENDENT_SIMD_NAME)/jsimd.c  $(CONSTI_ARCH_DEPENDENT_SIMD_NAME)/jsimd_neon.S

LOCAL_CFLAGS += \
	-DSIZEOF_SIZE_T=8 \

endif


LOCAL_CPPFLAGS += -Wno-incompatible-pointer-types

LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

# 静的ライブラリとしてビルド
include $(BUILD_STATIC_LIBRARY)

######################################################################
# jpeg-turbo1500.so
######################################################################
include $(CLEAR_VARS)
LOCAL_EXPORT_C_INCLUDES := \
		$(LOCAL_PATH)/

LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -ldl	# to avoid NDK issue(no need for static library)

LOCAL_WHOLE_STATIC_LIBRARIES = jpeg-turbo1500_static

LOCAL_MODULE := jpeg-turbo1500
include $(BUILD_SHARED_LIBRARY)

