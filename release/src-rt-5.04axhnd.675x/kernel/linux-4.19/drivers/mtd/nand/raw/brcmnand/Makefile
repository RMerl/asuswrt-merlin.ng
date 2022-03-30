# SPDX-License-Identifier: GPL-2.0
# link order matters; don't link the more generic brcmstb_nand.o before the
# more specific iproc_nand.o, for instance
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= iproc_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= bcm63138_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= bcm6368_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= brcmstb_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= brcmnand.o

obj-$(CONFIG_MTD_NAND_BRCMNAND) 	+= bcm63xx_nand.o



EXTRA_CFLAGS   += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
#EXTRA_CFLAGS    += -I $(KERNEL_DIR)/include/asm-generic -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/../flash
EXTRA_CFLAGS  += -I $(KERNEL_DIR)/include/asm-generic -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/../flash -I$(KERNEL_DIR)/drivers/mtd/nand/raw/brcmnand

