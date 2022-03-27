LOCAL_PATH := external/bluetooth

# Retrieve BlueZ version from configure.ac file
BLUEZ_VERSION := `grep "^AC_INIT" $(LOCAL_PATH)/bluez/configure.ac | sed -e "s/.*,.\(.*\))/\1/"`

ANDROID_VERSION := $(shell echo $(PLATFORM_VERSION) | awk -F. '{ printf "0x%02d%02d%02d",$$1,$$2,$$3 }')

ANDROID_GE_5_0_0 := $(shell test `echo $$(($(ANDROID_VERSION)))` -lt `echo $$((0x050000))`; echo $$?)

# Specify pathmap for glib and sbc
pathmap_INCL += glib:external/bluetooth/glib \
		sbc:external/bluetooth/sbc \

# Specify common compiler flags
BLUEZ_COMMON_CFLAGS := -DVERSION=\"$(BLUEZ_VERSION)\" \
			-DANDROID_VERSION=$(ANDROID_VERSION) \
			-DANDROID_STORAGEDIR=\"/data/misc/bluetooth\" \
			-DHAVE_LINUX_IF_ALG_H \
			-DHAVE_LINUX_TYPES_H \

# Enable warnings enabled in autotools build
BLUEZ_COMMON_CFLAGS += -Wall -Wextra \
			-Wdeclaration-after-statement \
			-Wmissing-declarations \
			-Wredundant-decls \
			-Wcast-align \

# Disable warnings enabled by Android but not enabled in autotools build
BLUEZ_COMMON_CFLAGS += -Wno-pointer-arith \
			-Wno-missing-field-initializers \
			-Wno-unused-parameter \

#
# Android BlueZ daemon (bluetoothd)
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/android/main.c \
	bluez/android/bluetooth.c \
	bluez/profiles/scanparam/scpp.c \
	bluez/profiles/deviceinfo/dis.c \
	bluez/profiles/battery/bas.c \
	bluez/profiles/input/hog-lib.c \
	bluez/android/hidhost.c \
	bluez/android/socket.c \
	bluez/android/ipc.c \
	bluez/android/avdtp.c \
	bluez/android/a2dp.c \
	bluez/android/a2dp-sink.c \
	bluez/android/avctp.c \
	bluez/android/avrcp.c \
	bluez/android/avrcp-lib.c \
	bluez/android/pan.c \
	bluez/android/handsfree.c \
	bluez/android/handsfree-client.c \
	bluez/android/gatt.c \
	bluez/android/health.c \
	bluez/android/sco.c \
	bluez/profiles/health/mcap.c \
	bluez/android/map-client.c \
	bluez/android/log.c \
	bluez/src/shared/mgmt.c \
	bluez/src/shared/util.c \
	bluez/src/shared/queue.c \
	bluez/src/shared/ringbuf.c \
	bluez/src/shared/hfp.c \
	bluez/src/shared/gatt-db.c \
	bluez/src/shared/io-glib.c \
	bluez/src/shared/timeout-glib.c \
	bluez/src/shared/crypto.c \
	bluez/src/shared/uhid.c \
	bluez/src/shared/att.c \
	bluez/src/sdpd-database.c \
	bluez/src/sdpd-service.c \
	bluez/src/sdpd-request.c \
	bluez/src/sdpd-server.c \
	bluez/src/uuid-helper.c \
	bluez/src/eir.c \
	bluez/lib/sdp.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \
	bluez/lib/uuid.c \
	bluez/btio/btio.c \
	bluez/src/sdp-client.c \
	bluez/profiles/network/bnep.c \
	bluez/attrib/gattrib.c \
	bluez/attrib/gatt.c \
	bluez/attrib/att.c

LOCAL_C_INCLUDES := \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib \

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_SHARED_LIBRARIES := \
	libglib \

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_TAGS := optional

# for userdebug/eng this module is bluetoothd-main since bluetoothd is used as
# wrapper to launch bluetooth with Valgrind
ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_MODULE := bluetoothd-main
LOCAL_STRIP_MODULE := false
else
LOCAL_MODULE := bluetoothd
endif

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# bluetooth.default.so HAL
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/android/hal-ipc.c \
	bluez/android/hal-bluetooth.c \
	bluez/android/hal-socket.c \
	bluez/android/hal-hidhost.c \
	bluez/android/hal-pan.c \
	bluez/android/hal-a2dp.c \
	bluez/android/hal-avrcp.c \
	bluez/android/hal-handsfree.c \
	bluez/android/hal-gatt.c \
	bluez/android/hal-utils.c \
	bluez/android/hal-health.c \

ifeq ($(ANDROID_GE_5_0_0), 1)
LOCAL_SRC_FILES += \
	bluez/android/hal-handsfree-client.c \
	bluez/android/hal-map-client.c \
	bluez/android/hal-a2dp-sink.c \
	bluez/android/hal-avrcp-ctrl.c
endif

LOCAL_C_INCLUDES += \
	$(call include-path-for, system-core) \
	$(call include-path-for, libhardware) \

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_MODULE := bluetooth.default
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_REQUIRED_MODULES := bluetoothd bluetoothd-snoop init.bluetooth.rc

ifeq ($(ANDROID_GE_5_0_0), 1)
LOCAL_MODULE_RELATIVE_PATH := hw
else
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

include $(BUILD_SHARED_LIBRARY)

#
# haltest
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/android/client/haltest.c \
	bluez/android/client/pollhandler.c \
	bluez/android/client/terminal.c \
	bluez/android/client/history.c \
	bluez/android/client/tabcompletion.c \
	bluez/android/client/if-audio.c \
	bluez/android/client/if-sco.c \
	bluez/android/client/if-av.c \
	bluez/android/client/if-rc.c \
	bluez/android/client/if-bt.c \
	bluez/android/client/if-hf.c \
	bluez/android/client/if-hh.c \
	bluez/android/client/if-pan.c \
	bluez/android/client/if-hl.c \
	bluez/android/client/if-sock.c \
	bluez/android/client/if-gatt.c \
	bluez/android/hal-utils.c \

ifeq ($(ANDROID_GE_5_0_0), 1)
LOCAL_SRC_FILES += \
	bluez/android/client/if-hf-client.c \
	bluez/android/client/if-mce.c \
	bluez/android/client/if-av-sink.c \
	bluez/android/client/if-rc-ctrl.c
endif

LOCAL_C_INCLUDES += \
	$(call include-path-for, system-core) \
	$(call include-path-for, libhardware) \

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/bluez/android \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS) -Wno-declaration-after-statement

LOCAL_SHARED_LIBRARIES := \
	libhardware \
	libcutils \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := haltest

include $(BUILD_EXECUTABLE)

#
# mcaptest
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/src/log.c \
	bluez/btio/btio.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \
	bluez/profiles/health/mcap.c \
	bluez/tools/mcaptest.c \

LOCAL_C_INCLUDES := \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib \

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_SHARED_LIBRARIES := \
	libglib \

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := mcaptest

include $(BUILD_EXECUTABLE)

#
# bneptest
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/src/log.c \
	bluez/btio/btio.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \
	bluez/profiles/network/bnep.c \
	bluez/tools/bneptest.c \

LOCAL_C_INCLUDES := \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib \

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_SHARED_LIBRARIES := \
	libglib \

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := bneptest

include $(BUILD_EXECUTABLE)

#
# avdtptest
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/android/avdtptest.c \
	bluez/android/avdtp.c \
	bluez/src/log.c \
	bluez/btio/btio.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \
	bluez/src/shared/util.c \
	bluez/src/shared/queue.c \

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/bluez \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_SHARED_LIBRARIES := \
	libglib \

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := avdtptest

include $(BUILD_EXECUTABLE)

#
# btmon
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/monitor/main.c \
	bluez/monitor/display.c \
	bluez/monitor/hcidump.c \
	bluez/monitor/control.c \
	bluez/monitor/packet.c \
	bluez/monitor/l2cap.c \
	bluez/monitor/avctp.c \
	bluez/monitor/avdtp.c \
	bluez/monitor/a2dp.c \
	bluez/monitor/rfcomm.c \
	bluez/monitor/bnep.c \
	bluez/monitor/uuid.c \
	bluez/monitor/sdp.c \
	bluez/monitor/vendor.c \
	bluez/monitor/lmp.c \
	bluez/monitor/crc.c \
	bluez/monitor/ll.c \
	bluez/monitor/hwdb.c \
	bluez/monitor/keys.c \
	bluez/monitor/ellisys.c \
	bluez/monitor/analyze.c \
	bluez/monitor/intel.c \
	bluez/monitor/broadcom.c \
	bluez/src/shared/util.c \
	bluez/src/shared/queue.c \
	bluez/src/shared/crypto.c \
	bluez/src/shared/btsnoop.c \
	bluez/src/shared/mainloop.c \
	bluez/lib/hci.c \
	bluez/lib/bluetooth.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := btmon

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# btproxy
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/btproxy.c \
	bluez/src/shared/mainloop.c \
	bluez/src/shared/util.c \
	bluez/src/shared/ecc.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := btproxy

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# A2DP audio
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/android/hal-audio.c \
	bluez/android/hal-audio-sbc.c \
	bluez/android/hal-audio-aptx.c \

LOCAL_C_INCLUDES = \
	$(LOCAL_PATH)/bluez \
	$(call include-path-for, system-core) \
	$(call include-path-for, libhardware) \
	$(call include-path-for, sbc) \

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libsbc \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS) -Wno-declaration-after-statement
LOCAL_LDFLAGS := -ldl

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := audio.a2dp.default

ifeq ($(ANDROID_GE_5_0_0), 1)
LOCAL_MODULE_RELATIVE_PATH := hw
else
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

include $(BUILD_SHARED_LIBRARY)

#
# SCO audio
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := bluez/android/hal-sco.c \
	bluez/android/hal-utils.c

LOCAL_C_INCLUDES = \
	$(call include-path-for, system-core) \
	$(call include-path-for, libhardware) \
	$(call include-path-for, audio-utils) \

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libaudioutils \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS) -Wno-declaration-after-statement

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := audio.sco.default

ifeq ($(ANDROID_GE_5_0_0), 1)
LOCAL_MODULE_RELATIVE_PATH := hw
else
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

include $(BUILD_SHARED_LIBRARY)

#
# l2cap-test
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/l2test.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := l2test

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# bluetoothd-snoop
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/android/bluetoothd-snoop.c \
	bluez/src/shared/mainloop.c \
	bluez/src/shared/btsnoop.c \
	bluez/android/log.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \
	$(LOCAL_PATH)/bluez/lib \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := bluetoothd-snoop

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# init.bluetooth.rc
#

include $(CLEAR_VARS)

LOCAL_MODULE := init.bluetooth.rc
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := bluez/android/$(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)

include $(BUILD_PREBUILT)

#
# btmgmt
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/btmgmt.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \
	bluez/lib/sdp.c \
	bluez/src/shared/mainloop.c \
	bluez/src/shared/io-mainloop.c \
	bluez/src/shared/mgmt.c \
	bluez/src/shared/queue.c \
	bluez/src/shared/util.c \
	bluez/src/shared/gap.c \
	bluez/src/uuid-helper.c \
	bluez/client/display.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \
	$(LOCAL_PATH)/bluez/android/compat \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := btmgmt

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# hcitool
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/hcitool.c \
	bluez/src/oui.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := hcitool

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# hciconfig
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	bluez/tools/hciconfig.c \
	bluez/tools/csr.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := hciconfig

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# l2ping
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/l2ping.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := l2ping

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# avtest
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/avtest.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := avtest

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# hciattach
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/hciattach.c \
	bluez/tools/hciattach_st.c \
	bluez/tools/hciattach_ti.c \
	bluez/tools/hciattach_tialt.c \
	bluez/tools/hciattach_ath3k.c \
	bluez/tools/hciattach_qualcomm.c \
	bluez/tools/hciattach_intel.c \
	bluez/tools/hciattach_bcm43xx.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := hciattach

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# libsbc
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	sbc/sbc/sbc.c \
	sbc/sbc/sbc_primitives.c \
	sbc/sbc/sbc_primitives_mmx.c \
	sbc/sbc/sbc_primitives_neon.c \
	sbc/sbc/sbc_primitives_armv6.c \
	sbc/sbc/sbc_primitives_iwmmxt.c \

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/sbc \

LOCAL_CFLAGS:= \
	-Os \
	-Wno-sign-compare \
	-Wno-missing-field-initializers \
	-Wno-unused-parameter \
	-Wno-type-limits \
	-Wno-empty-body \

LOCAL_MODULE := libsbc

include $(BUILD_SHARED_LIBRARY)

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))

#
# bluetoothd (debug)
# this is just a wrapper used in userdebug/eng to launch bluetoothd-main
# with/without Valgrind
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/android/bluetoothd-wrapper.c \
	bluez/android/hal-utils.c

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := bluetoothd

LOCAL_REQUIRED_MODULES := \
	bluetoothd-main \
	valgrind \
	memcheck-$(TARGET_ARCH)-linux \
	vgpreload_core-$(TARGET_ARCH)-linux \
	vgpreload_memcheck-$(TARGET_ARCH)-linux \
	default.supp

include $(BUILD_EXECUTABLE)

endif

#
# bluetooth-headers
#

include $(CLEAR_VARS)

LOCAL_MODULE := bluetooth-headers
LOCAL_NODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

include_path := $(local-intermediates-dir)/include
include_files := $(wildcard $(LOCAL_PATH)/bluez/lib/*.h)
$(shell mkdir -p $(include_path)/bluetooth)
$(foreach file,$(include_files),$(shell cp -u $(file) $(include_path)/bluetooth))

LOCAL_EXPORT_C_INCLUDE_DIRS := $(include_path)

include $(BUILD_STATIC_LIBRARY)

#
# avtest
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/avinfo.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := avinfo

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)

#
# rctest
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluez/tools/rctest.c \
	bluez/lib/bluetooth.c \
	bluez/lib/hci.c \
	bluez/lib/sdp.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/bluez \

LOCAL_CFLAGS := $(BLUEZ_COMMON_CFLAGS)

LOCAL_STATIC_LIBRARIES := \
	bluetooth-headers \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := rctest

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/bluez/configure.ac

include $(BUILD_EXECUTABLE)
