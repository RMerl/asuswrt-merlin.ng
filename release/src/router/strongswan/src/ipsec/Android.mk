LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# build ipsec ------------------------------------------------------------------

LOCAL_MODULE := ipsec

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := EXECUTABLES

GEN := $(local-intermediates-dir)/ipsec

$(GEN) : PRIVATE_PATH := $(LOCAL_PATH)
$(GEN) : PRIVATE_CUSTOM_TOOL = sed \
	-e "s:@IPSEC_SHELL@:/system/bin/sh:" \
	-e "s:@IPSEC_VERSION@:$(strongswan_VERSION):" \
	-e "s:@IPSEC_NAME@:strongSwan:" \
	-e "s:@IPSEC_DISTRO@::" \
	-e "s:@IPSEC_DIR@:$(strongswan_DIR):" \
	-e "s:@IPSEC_SCRIPT@:ipsec:" \
	-e "s:@IPSEC_BINDIR@:$(strongswan_DIR):" \
	-e "s:@IPSEC_SBINDIR@:$(strongswan_SBINDIR):" \
	-e "s:@IPSEC_CONFDIR@:$(strongswan_CONFDIR):" \
	-e "s:@IPSEC_PIDDIR@:$(strongswan_PIDDIR):" \
	$< > $@ && chmod +x $@

$(GEN) : $(strongswan_PATH)/Android.mk
$(GEN) : $(LOCAL_PATH)/_ipsec.in
	$(transform-generated-source)

LOCAL_GENERATED_SOURCES := $(GEN)

include $(BUILD_SYSTEM)/base_rules.mk

