LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
libhydra_la_SOURCES := \
hydra.c hydra.h \
attributes/attributes.c attributes/attributes.h \
attributes/attribute_provider.h attributes/attribute_handler.h \
attributes/attribute_manager.c attributes/attribute_manager.h \
attributes/mem_pool.c attributes/mem_pool.h \
kernel/kernel_interface.c kernel/kernel_interface.h \
kernel/kernel_ipsec.c kernel/kernel_ipsec.h \
kernel/kernel_net.c kernel/kernel_net.h \
kernel/kernel_listener.h

LOCAL_SRC_FILES := $(filter %.c,$(libhydra_la_SOURCES))

# adding the plugin source files

LOCAL_SRC_FILES += $(call add_plugin, attr)

LOCAL_SRC_FILES += $(call add_plugin, kernel-pfkey)

LOCAL_SRC_FILES += $(call add_plugin, kernel-netlink)

# build libhydra ---------------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src/include \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS)

LOCAL_MODULE := libhydra

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libstrongswan

include $(BUILD_SHARED_LIBRARY)

