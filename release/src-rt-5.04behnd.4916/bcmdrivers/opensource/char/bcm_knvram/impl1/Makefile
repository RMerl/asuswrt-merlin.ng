bcm_knvram-objs := src/wlcsm_linux.o
bcm_knvram-objs += src/wlcsm_netlink.o src/wlcsm_nvram.o
bcm_knvram-objs += src/bcm_nvram.o
obj-$(CONFIG_BCM_NVRAM) += bcm_knvram.o

EXTRA_CFLAGS += -I$(src)/inlcude -I$(BRCMDRIVERS_DIR)/opensource/char/bcm_knvram/impl1/include
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)

ifeq ($(strip $(CONFIG_BCM_WLCSM_DEBUG)),y)
EXTRA_CFLAGS += -DWLCSM_DEBUG
endif

EXTRA_CFLAGS += -DWL_DEFAULT_NUM_SSID=$(CONFIG_BCM_DEFAULT_NUM_MBSS)

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.mod.c *.o *.a *.s *.ko src/*.o src/*.mod.c modules.*

bcm_headers_install:
	echo BCMLIBS installing header
	echo $(INSTALL_HEADERS_WITH_CP) ./include/wl_common_defs.h $(INC_BCMDRIVER_PATH)/include/
	$(INSTALL_HEADERS_WITH_CP) ./include/wl_common_defs.h $(INC_BCMDRIVER_PATH)/include/

.PHONY: bcm_headers_install
