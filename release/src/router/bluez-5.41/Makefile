include ../common.mak
CFLAGS = -Os -Wall $(EXTRACFLAGS)
LIBBLUETOOTH_VER=3.18.13

all:
ifeq ($(wildcard $(SRCBASE)/include/bluetooth),)
ifeq ($(wildcard bluetooth),)
	mkdir bluetooth
	cp -rf lib/* bluetooth
endif
	cp -rf bluetooth $(SRCBASE)/include
endif

install: all
	cp -f prebuild/$(BUILD_NAME)/bccmd				tools/
	cp -f prebuild/$(BUILD_NAME)/bluemoon				tools/
	cp -f prebuild/$(BUILD_NAME)/ciptool				tools/
	cp -f prebuild/$(BUILD_NAME)/hcitool				tools/
	cp -f prebuild/$(BUILD_NAME)/hcidump				tools/
	cp -f prebuild/$(BUILD_NAME)/hciattach				tools/
	cp -f prebuild/$(BUILD_NAME)/hciconfig				tools/
	cp -f prebuild/$(BUILD_NAME)/hid2hci				tools/
	cp -f prebuild/$(BUILD_NAME)/l2ping				tools/
	cp -f prebuild/$(BUILD_NAME)/l2test				tools/
	cp -f prebuild/$(BUILD_NAME)/mpris-proxy			tools/
	cp -f prebuild/$(BUILD_NAME)/rctest				tools/
	cp -f prebuild/$(BUILD_NAME)/rfcomm				tools/
	cp -f prebuild/$(BUILD_NAME)/sdptool				tools/
	cp -f prebuild/$(BUILD_NAME)/btmgmt				tools/
	cp -f prebuild/$(BUILD_NAME)/bluetoothctl			client/
	cp -f prebuild/$(BUILD_NAME)/bluetoothd				src/
	cp -f prebuild/$(BUILD_NAME)/gatttool				attrib/
	mkdir -p lib/.libs
	cp -f prebuild/$(BUILD_NAME)/libbluetooth.so.$(LIBBLUETOOTH_VER)	lib/.libs/

clean:


.PHONY: all clean
