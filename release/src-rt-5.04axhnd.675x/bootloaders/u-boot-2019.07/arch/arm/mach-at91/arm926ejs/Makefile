# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_AT91SAM9260)	+= at91sam9260_devices.o
obj-$(CONFIG_AT91SAM9G20)	+= at91sam9260_devices.o
obj-$(CONFIG_AT91SAM9XE)	+= at91sam9260_devices.o
obj-$(CONFIG_AT91SAM9261)	+= at91sam9261_devices.o
obj-$(CONFIG_AT91SAM9G10)	+= at91sam9261_devices.o
obj-$(CONFIG_AT91SAM9263)	+= at91sam9263_devices.o
obj-$(CONFIG_AT91SAM9RL)	+= at91sam9rl_devices.o
obj-$(CONFIG_AT91SAM9M10G45)	+= at91sam9m10g45_devices.o
obj-$(CONFIG_AT91SAM9G45)	+= at91sam9m10g45_devices.o
obj-$(CONFIG_AT91SAM9N12)	+= at91sam9n12_devices.o
obj-$(CONFIG_AT91SAM9X5)	+= at91sam9x5_devices.o
obj-$(CONFIG_AT91_EFLASH)	+= eflash.o
obj-$(CONFIG_AT91_LED)	+= led.o
obj-y += clock.o
obj-y += cpu.o
obj-y	+= reset.o
ifeq ($(CONFIG_ATMEL_PIT_TIMER),)
obj-y	+= timer.o
endif

ifndef CONFIG_SKIP_LOWLEVEL_INIT
ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
obj-y	+= lowlevel_init.o
endif
endif

ifdef CONFIG_$(SPL_)SYS_THUMB_BUILD
ifndef CONFIG_HAS_THUMB2

CFLAGS_cache.o := -marm

endif
endif
