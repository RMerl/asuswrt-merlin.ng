# File: bcmdrivers/char/MoCA/bcm963xx/Makefile
# File: bcmdrivers/char/MoCA/bcm968xx/Makefile
#
# Makefile for the BCM63xx MoCA driver.
# Makefile for the BCM68xx MoCA driver.
#

obj-$(CONFIG_BCM_MoCA) += bcmmoca.o
bcmmoca-objs += bmoca.o

EXTRA_CFLAGS += -DCONFIG_$(BRCM_CHIP) -fno-exceptions -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_ADSLDRV_PATH) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) -I$(BRCMDRIVERS_DIR)/broadcom/net/enet/shared -I$(BRCMDRIVERS_DIR)/opensource/char/board/bcm963xx/impl1/bbsi
EXTRA_CFLAGS += -DMOCA_TRACING -DDSL_MOCA
EXTRA_CFLAGS += -Werror -Wfatal-errors
ifeq ($(BRCM_DRIVER_MoCA_CTP_CANDIDATE),y)
EXTRA_CFLAGS += -DMOCA_CTP_CANDIDATE
endif

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

