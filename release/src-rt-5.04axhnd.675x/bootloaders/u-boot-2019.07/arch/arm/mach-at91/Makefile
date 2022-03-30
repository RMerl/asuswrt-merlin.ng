# SPDX-License-Identifier: GPL-2.0+

obj-$(CONFIG_AT91_WANTS_COMMON_PHY) += phy.o
ifneq ($(CONFIG_SPL_BUILD),)
obj-$(CONFIG_AT91SAM9260) += sdram.o spl_at91.o
obj-$(CONFIG_AT91SAM9G20) += sdram.o spl_at91.o
obj-$(CONFIG_AT91SAM9M10G45) += mpddrc.o spl_at91.o
obj-$(CONFIG_AT91SAM9N12) += mpddrc.o spl_at91.o
obj-$(CONFIG_AT91SAM9X5) += mpddrc.o spl_at91.o
obj-$(CONFIG_SAMA5D2) += bootparams_atmel.o mpddrc.o spl_atmel.o matrix.o atmel_sfr.o
obj-$(CONFIG_SAMA5D3) += bootparams_atmel.o mpddrc.o spl_atmel.o
obj-$(CONFIG_SAMA5D4) += bootparams_atmel.o mpddrc.o spl_atmel.o matrix.o atmel_sfr.o
obj-y += spl.o
endif

obj-y += clock.o
obj-$(CONFIG_CPU_ARM920T)	+= arm920t/
obj-$(CONFIG_CPU_ARM926EJS)	+= arm926ejs/
obj-$(CONFIG_CPU_V7A)		+= armv7/
