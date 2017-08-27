ifdef BCM_KF # defined(CONFIG_BCM_KF_MIPS_BCM963XX)
#
# Makefile for generic Broadcom MIPS boards
#
# Copyright (C) 2004 Broadcom Corporation
#

ifdef BCM_KF # defined(CONFIG_BCM_KF_BALOO)
endif # BCM_KF # defined(CONFIG_BCM_KF_BALOO)

ifdef BCM_KF # defined(CONFIG_BCM_KF_BOUNCE)
obj-$(CONFIG_BRCM_BOUNCE) += bounce.o
endif # BCM_KF # defined(CONFIG_BCM_KF_BOUNCE)
ifdef BCM_KF # defined(CONFIG_BCM_KF_PMON)
obj-$(CONFIG_PMON) += pmontool.o
endif # BCM_KF # defined(CONFIG_BCM_KF_PMON)

SRCBASE         := $(TOPDIR)
EXTRA_CFLAGS    += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) -I$(SRCBASE)/include -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
#EXTRA_CFLAGS    += -I$(INC_ADSLDRV_PATH) -DDBG
EXTRA_CFLAGS    += -I$(INC_ADSLDRV_PATH) 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/../shared/broadcom/include/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/include/$(BRCM_BOARD)
cflags-y      += -I$(srctree)/arch/mips/include/asm/bcm963xx
EXTRA_CFLAGS += -g


ifeq "$(ADSL)" "ANNEX_B"
EXTRA_CFLAGS += -DADSL_ANNEXB
endif
ifeq "$(ADSL)" "SADSL"
EXTRA_CFLAGS += -DADSL_SADSL
endif
ifeq "$(ADSL)" "ANNEX_C"
EXTRA_CFLAGS += -DADSL_ANNEXC
endif
endif # BCM_KF # defined(CONFIG_BCM_KF_MIPS_BCM963XX)


