# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y	+= cache.o
obj-y	+= cache_init.o
obj-y	+= genex.o
obj-y	+= reloc.o
obj-y	+= stack.o
obj-y	+= traps.o

obj-$(CONFIG_CMD_BOOTM) += bootm.o

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += ashldi3.o ashrdi3.o lshrdi3.o
