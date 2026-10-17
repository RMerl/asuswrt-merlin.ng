MODNAME := bcm_memc
$(MODNAME)-objs := bcm_memc.o

ifeq ($(strip $(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)), y)
$(MODNAME)-objs += bcm_ddr_sr.o 
endif

EXTRA_CFLAGS += -Wall -Werror -Wfatal-errors
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx

obj-$(CONFIG_BCM_MEMC) += $(MODNAME).o

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean