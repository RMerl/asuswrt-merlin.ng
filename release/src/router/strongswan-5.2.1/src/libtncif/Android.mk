LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
libtncif_la_SOURCES := \
tncif.h tncifimc.h tncifimv.h tncif_names.h tncif_names.c \
tncif_identity.h tncif_identity.c \
tncif_pa_subtypes.h tncif_pa_subtypes.c \
tncif_policy.h tncif_policy.c

LOCAL_SRC_FILES := $(filter %.c,$(libtncif_la_SOURCES))

# build libtncif ---------------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS)

LOCAL_MODULE := libtncif

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libstrongswan

include $(BUILD_SHARED_LIBRARY)

