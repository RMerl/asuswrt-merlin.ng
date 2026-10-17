# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_SPL_BUILD

obj-$(CONFIG_ARCH_UNIPHIER_LD4)		+= umc-ld4.o \
					   ddrphy-training.o ddrphy-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO4)	+= umc-pro4.o \
					   ddrphy-training.o ddrphy-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_SLD8)	+= umc-sld8.o \
					   ddrphy-training.o ddrphy-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO5)	+= umc-pro5.o
obj-$(CONFIG_ARCH_UNIPHIER_PXS2)	+= umc-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD6B)	+= umc-pxs2.o

else

obj-$(CONFIG_CMD_DDRPHY_DUMP) += cmd_ddrphy.o
obj-$(CONFIG_CMD_DDRMPHY_DUMP) += cmd_ddrmphy.o

endif
