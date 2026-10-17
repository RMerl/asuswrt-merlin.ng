# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_NAND_BRCMNAND) += brcmnand_spl.o
else
obj-$(CONFIG_NAND_BRCMNAND_6368) += bcm6368_nand.o
obj-$(CONFIG_NAND_BRCMNAND_6838) += bcm6838_nand.o
obj-$(CONFIG_NAND_BRCMNAND_BCMBCA) += bcmbca_nand.o
obj-$(CONFIG_NAND_BRCMNAND_IPROC) += iproc_nand.o
obj-$(CONFIG_NAND_BRCMNAND) += brcmnand.o
obj-$(CONFIG_NAND_BRCMNAND) += brcmnand_compat.o
endif
