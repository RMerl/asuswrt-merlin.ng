# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2015 Google, Inc
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
obj-$(CONFIG_$(SPL_TPL_)DM) += ram-uclass.o
obj-$(CONFIG_MPC83XX_SDRAM) += mpc83xx_sdram.o
obj-$(CONFIG_SANDBOX) += sandbox_ram.o
obj-$(CONFIG_STM32MP1_DDR) += stm32mp1/
obj-$(CONFIG_STM32_SDRAM) += stm32_sdram.o
obj-$(CONFIG_ARCH_BMIPS) += bmips_ram.o

obj-$(CONFIG_ARCH_ROCKCHIP) += rockchip/

obj-$(CONFIG_K3_AM654_DDRSS) += k3-am654-ddrss.o
obj-$(CONFIG_ARCH_MEDIATEK) += mediatek/
obj-$(CONFIG_ARCH_ASPEED) += aspeed/
obj-$(CONFIG_K3_DDRSS) += k3-ddrss/

obj-$(CONFIG_IMXRT_SDRAM) += imxrt_sdram.o

obj-$(CONFIG_RAM_SIFIVE) += sifive/
ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_STARFIVE_DDR) += starfive/
endif

obj-$(CONFIG_DRAM_SUN20I_D1) += sunxi/

obj-$(CONFIG_ARCH_OCTEON) += octeon/

obj-$(CONFIG_ARCH_RENESAS) += renesas/
obj-$(CONFIG_CADENCE_DDR_CTRL) += cadence/
