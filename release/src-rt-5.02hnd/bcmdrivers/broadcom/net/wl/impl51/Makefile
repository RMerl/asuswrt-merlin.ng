# Toploevel makefile to build hnd dongle
#
#
#
#

export ALLSRCBASE= $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)
export WLSRCBASE= main/src
export WLCFGDIR = $(ALLSRCBASE)/main/src/wl/config
export WLMAIN = main/src

export PLT = arm
export LINUX_VERSION = 4_1_0
export LINUXDIR = $(KERNEL_DIR)
export 	KBUILD_VERBOSE := 1
SUBMAKE_SETTINGS = SRCBASE=$(SRCBASE) BASEDIR=$(BASEDIR)


.PHONY: oldconfig version pciefd clean version_info loadscript

oldconfig:
	$(MAKE) -C $(WLSRCBASE)/router oldconfig

version:
	$(MAKE) -C $(WLSRCBASE)/router version

pciefd: version
	$(MAKE) -C $(WLSRCBASE)/router pciefd

clean:
	$(MAKE) -C $(WLSRCBASE)/router pciefd-clean

version_info:
	@echo $(shell echo "wl:";)
