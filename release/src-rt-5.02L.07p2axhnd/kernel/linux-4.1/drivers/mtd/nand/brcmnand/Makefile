# link order matters; don't link the more generic brcmstb_nand.o before the
# more specific iproc_nand.o, for instance
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= iproc_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= bcm63xx_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= brcmstb_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= brcmnand.o
ifdef BCM_KF # defined (CONFIG_BCM_KF_MTD_BCM963XX)
EXTRA_CFLAGS	+= -I $(TOPDIR)/include/asm-generic -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/../flash
endif # BCM_KF # defined(CONFIG_BCM_KF_MTD_BCM963XX)
