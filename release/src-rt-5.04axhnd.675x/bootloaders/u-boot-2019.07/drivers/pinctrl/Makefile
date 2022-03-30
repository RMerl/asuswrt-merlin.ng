# SPDX-License-Identifier: GPL-2.0+

obj-y					+= pinctrl-uclass.o
obj-$(CONFIG_$(SPL_)PINCTRL_GENERIC)	+= pinctrl-generic.o

obj-$(CONFIG_PINCTRL_AT91)		+= pinctrl-at91.o
obj-$(CONFIG_PINCTRL_AT91PIO4)		+= pinctrl-at91-pio4.o
obj-y					+= nxp/
obj-$(CONFIG_$(SPL_)PINCTRL_ROCKCHIP)	+= rockchip/
obj-$(CONFIG_ARCH_ASPEED) += aspeed/
obj-$(CONFIG_ARCH_ATH79) += ath79/
obj-$(CONFIG_ARCH_RMOBILE) += renesas/
obj-$(CONFIG_PINCTRL_SANDBOX)	+= pinctrl-sandbox.o

obj-$(CONFIG_PINCTRL_UNIPHIER)	+= uniphier/
obj-$(CONFIG_PINCTRL_PIC32)	+= pinctrl_pic32.o
obj-$(CONFIG_PINCTRL_EXYNOS)	+= exynos/
obj-$(CONFIG_PINCTRL_MESON)	+= meson/
obj-$(CONFIG_PINCTRL_MTK)	+= mediatek/
obj-$(CONFIG_PINCTRL_MSCC)	+= mscc/
obj-$(CONFIG_ARCH_MVEBU)	+= mvebu/
obj-$(CONFIG_PINCTRL_SINGLE)	+= pinctrl-single.o
obj-$(CONFIG_PINCTRL_STI)	+= pinctrl-sti.o
obj-$(CONFIG_PINCTRL_STM32)	+= pinctrl_stm32.o
obj-$(CONFIG_$(SPL_)PINCTRL_STMFX)	+= pinctrl-stmfx.o
obj-y				+= broadcom/
