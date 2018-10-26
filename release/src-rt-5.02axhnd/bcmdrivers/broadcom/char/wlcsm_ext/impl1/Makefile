ifneq ($(PREBUILT_EXTRAMOD),1)
wlcsm-objs := src/wlcsm_linux.o
wlcsm-objs += src/wlcsm_netlink.o src/wlcsm_nvram.o
wlcsm-objs += src/bcm_nvram.o
else
wlcsm-objs := wlcsm_preb.o
endif
obj-$(CONFIG_BCM_NVRAM) += wlcsm.o

EXTRA_CFLAGS += -I$(src)/inlcude -I$(BRCMDRIVERS_DIR)/broadcom/char/wlcsm_ext/impl1/include
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)

ifeq ($(strip $(CONFIG_BCM_WLCSM_DEBUG)),y)
EXTRA_CFLAGS += -DWLCSM_DEBUG
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.mod.c *.o *.a *.s *.ko src/*.o src/*.mod.c modules.*
