LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
libtpmtss_la_SOURCES := \
	tpm_tss.h tpm_tss.c \
	tpm_tss_quote_info.h tpm_tss_quote_info.c \
	tpm_tss_trousers.h tpm_tss_trousers.c \
	tpm_tss_tss2.h tpm_tss_tss2_v1.c tpm_tss_tss2_v2.c \
	tpm_tss_tss2_names.h tpm_tss_tss2_names_v1.c tpm_tss_tss2_names_v2.c

LOCAL_SRC_FILES := $(filter %.c,$(libtpmtss_la_SOURCES))

# build libtpmtss --------------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS)

LOCAL_MODULE := libtpmtss

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libstrongswan

include $(BUILD_SHARED_LIBRARY)

