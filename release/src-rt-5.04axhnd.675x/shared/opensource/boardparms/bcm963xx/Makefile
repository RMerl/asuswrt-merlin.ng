ifeq ($(CONFIG_BCM_IN_KERNEL),y)

# Linux
ifeq ($(strip $(CONFIG_DT_SUPPORT_ONLY)),)
obj-y		+=  bp_funcs.o boardparms_chip.o
obj-y		+= pinmux_table_$(BRCM_CHIP).o
obj-y		+= boardparms_$(BRCM_CHIP).o
endif

EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)

ifeq ($(MOCA_LAN_DISCONN),y)
EXTRA_CFLAGS += -DCONFIG_NO_MOCA_LAN
endif

ifeq ($(MOCA_WAN_DISCONN),y)
EXTRA_CFLAGS += -DCONFIG_NO_MOCA_WAN
endif

ifneq ($(CONFIG_BCM_ARCHER),)
EXTRA_CFLAGS += -DCONFIG_BCM_SYSPORT
endif

-include $(TOPDIR)/Rules.make

endif


