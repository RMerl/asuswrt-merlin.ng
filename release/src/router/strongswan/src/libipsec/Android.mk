LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
libipsec_la_SOURCES := \
ipsec.c ipsec.h \
esp_context.c esp_context.h \
esp_packet.c esp_packet.h \
ip_packet.c ip_packet.h \
ipsec_event_listener.h \
ipsec_event_relay.c ipsec_event_relay.h \
ipsec_policy.c ipsec_policy.h \
ipsec_policy_mgr.c ipsec_policy_mgr.h \
ipsec_processor.c ipsec_processor.h \
ipsec_sa.c ipsec_sa.h \
ipsec_sa_mgr.c ipsec_sa_mgr.h

LOCAL_SRC_FILES := $(filter %.c,$(libipsec_la_SOURCES))

# build libipsec ---------------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS)

LOCAL_MODULE := libipsec

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libstrongswan

include $(BUILD_SHARED_LIBRARY)

