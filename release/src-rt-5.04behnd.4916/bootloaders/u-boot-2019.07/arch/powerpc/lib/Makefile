# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

## Build a couple of necessary functions into a private libgcc
## if the user asked for it
lib-$(CONFIG_USE_PRIVATE_LIBGCC) += _ashldi3.o _ashrdi3.o _lshrdi3.o

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL
obj-y += cache.o time.o
ifndef CONFIG_TIMER
obj-y += ticks.o
endif
else

obj-y	+= ppcstring.o

obj-y	+= ppccache.o
ifndef CONFIG_TIMER
obj-y	+= ticks.o
endif
obj-y	+= reloc.o

obj-$(CONFIG_BAT_RW) += bat_rw.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-y	+= cache.o
obj-y	+= extable.o
obj-y	+= interrupts.o
obj-$(CONFIG_CMD_KGDB) += kgdb.o
obj-y	+= stack.o
obj-y	+= time.o
endif # not minimal

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_FRAMEWORK) += spl.o
endif
