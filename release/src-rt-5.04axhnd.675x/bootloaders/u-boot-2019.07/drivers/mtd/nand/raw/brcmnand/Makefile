# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_NAND_BRCMNAND) += brcmnand_spl.o
else
obj-$(CONFIG_NAND_BRCMNAND_63158) += bcm63158_nand.o
obj-$(CONFIG_NAND_BRCMNAND_6838) += bcm6838_nand.o
obj-$(CONFIG_NAND_BRCMNAND) += brcmnand.o
obj-$(CONFIG_NAND_BRCMNAND) += brcmnand_compat.o
obj-$(CONFIG_NAND_BRCMNAND) += bcmbca_nand.o
endif
