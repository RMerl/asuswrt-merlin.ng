LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
scepclient_SOURCES := \
scepclient.c scep.c scep.h

LOCAL_SRC_FILES := $(filter %.c,$(scepclient_SOURCES))

# build scepclient -------------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS) \
	-DPLUGINS='"$(strongswan_SCEPCLIENT_PLUGINS)"'

LOCAL_MODULE := scepclient

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libstrongswan

include $(BUILD_EXECUTABLE)