# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_SPL_BUILD

obj-y += boards.o
obj-y += spl_board_init.o
obj-y += memconf.o
obj-y += bcu/
obj-$(CONFIG_SPL_MMC_SUPPORT) += mmc-boot-mode.o

else

obj-$(CONFIG_DISPLAY_CPUINFO) += cpu-info.o
obj-y += dram_init.o
obj-y += board_init.o
obj-$(CONFIG_BOARD_LATE_INIT) += board_late_init.o
ifndef CONFIG_SYSRESET
obj-y += reset.o
endif

obj-$(CONFIG_MICRO_SUPPORT_CARD) += sbc/ micro-support-card.o
obj-y += pinctrl-glue.o
obj-$(CONFIG_MMC) += mmc-first-dev.o
obj-y += fdt-fixup.o

endif

obj-y += soc-info.o
obj-y += boot-device/
obj-y += clk/
obj-y += dram/

obj-$(CONFIG_DEBUG_UART_UNIPHIER) += debug-uart/

obj-$(CONFIG_CPU_V7A) += arm32/
obj-$(CONFIG_ARM64) += arm64/
