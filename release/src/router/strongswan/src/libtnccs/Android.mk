LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
libtnccs_la_SOURCES := \
tnc/tnc.h tnc/tnc.c \
tnc/imc/imc.h tnc/imc/imc_manager.h \
tnc/imv/imv.h tnc/imv/imv_manager.h \
tnc/imv/imv_recommendations.h tnc/imv/imv_recommendations.c \
tnc/tnccs/tnccs.h tnc/tnccs/tnccs.c \
tnc/tnccs/tnccs_manager.h tnc/tnccs/tnccs_manager.c

LOCAL_SRC_FILES := $(filter %.c,$(libtnccs_la_SOURCES))

# adding the plugin source files

LOCAL_SRC_FILES += $(call add_plugin, tnc-imc)
ifneq ($(call plugin_enabled, tnc-imc),)
LOCAL_LDLIBS += -ldl
endif

LOCAL_SRC_FILES += $(call add_plugin, tnc-tnccs)

LOCAL_SRC_FILES += $(call add_plugin, tnccs-20)
LOCAL_SRC_FILES += $(call add_plugin_subdirs, tnccs-20, batch messages messages/ietf messages/ita messages/tcg state_machine)
ifneq ($(call plugin_enabled, tnccs-20),)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/plugins/tnccs_20/
endif

ifneq ($(or $(call plugin_enabled, tnc-imc), $(call plugin_enabled, tnc-tnccs), \
			$(call plugin_enabled, tnccs-20)),)
LOCAL_SHARED_LIBRARIES += libtncif
endif

# build libtncif ---------------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src/libtls \
	$(strongswan_PATH)/src/libtncif \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS)

LOCAL_MODULE := libtnccs

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libstrongswan

include $(BUILD_SHARED_LIBRARY)

