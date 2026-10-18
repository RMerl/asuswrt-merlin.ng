# SPDX-License-Identifier: GPL-2.0
# link order matters; don't link the more generic brcmstb_nand.o before the
# more specific iproc_nand.o, for instance
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= iproc_nand.o
ifdef BCM_KF
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= bcmbca_nand.o
else
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= bcm63138_nand.o
endif
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= bcm6368_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= brcmstb_nand.o
obj-$(CONFIG_MTD_NAND_BRCMNAND)		+= brcmnand.o
