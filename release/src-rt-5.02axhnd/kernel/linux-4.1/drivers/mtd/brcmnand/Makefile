ifdef BCM_KF # defined(CONFIG_BCM_KF_MTD_BCMNAND)
#
# Makefile for the Broadcom NAND MTD
#

obj-$(CONFIG_MTD_BRCMNAND)		+=  bcm63xx-nand.o brcmnand_base.o brcmnand_bbt.o
## obj-$(CONFIG_MTD_BRCMNAND_EDU)		+= edu.o
obj-$(CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING)	+= brcmnand_cet.o
EXTRA_CFLAGS	+= -I $(TOPDIR)/include/asm-generic -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
endif # BCM_KF # defined(CONFIG_BCM_KF_MTD_BCMNAND)

