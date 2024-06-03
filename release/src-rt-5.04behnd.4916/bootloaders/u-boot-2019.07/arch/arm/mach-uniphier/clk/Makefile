# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_SPL_BUILD

obj-$(CONFIG_ARCH_UNIPHIER_LD4)		+= clk-early-ld4.o clk-dram-ld4.o dpll-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO4)	+= clk-early-ld4.o clk-dram-ld4.o dpll-pro4.o
obj-$(CONFIG_ARCH_UNIPHIER_SLD8)	+= clk-early-ld4.o clk-dram-ld4.o dpll-sld8.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO5)	+= clk-early-ld4.o clk-dram-pro5.o dpll-pro5.o
obj-$(CONFIG_ARCH_UNIPHIER_PXS2)	+= clk-early-ld4.o clk-dram-pxs2.o dpll-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD6B)	+= clk-early-ld4.o clk-dram-pxs2.o dpll-pxs2.o

else

obj-$(CONFIG_ARCH_UNIPHIER_LD4)		+= clk-ld4.o pll-ld4.o dpll-tail.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO4)	+= clk-pro4.o pll-pro4.o dpll-tail.o
obj-$(CONFIG_ARCH_UNIPHIER_SLD8)	+= clk-ld4.o pll-ld4.o dpll-tail.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO5)	+= clk-pro5.o
obj-$(CONFIG_ARCH_UNIPHIER_PXS2)	+= clk-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD6B)	+= clk-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD11)	+= clk-ld11.o pll-ld11.o
obj-$(CONFIG_ARCH_UNIPHIER_LD20)	+= clk-ld20.o pll-ld20.o
obj-$(CONFIG_ARCH_UNIPHIER_PXS3)	+= clk-pxs3.o pll-pxs3.o

endif

obj-$(CONFIG_ARCH_UNIPHIER_LD11)	+= pll-base-ld20.o
obj-$(CONFIG_ARCH_UNIPHIER_LD20)	+= pll-base-ld20.o
obj-$(CONFIG_ARCH_UNIPHIER_PXS3)	+= pll-base-ld20.o
