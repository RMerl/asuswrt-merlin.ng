# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2007
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>

extra-y	+= start.o

obj-y	+= board.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-y	+= time.o
obj-$(CONFIG_CMD_SH_ZIMAGEBOOT) += zimageboot.o

udivsi3-y			:= udivsi3_i4i-Os.o

ifeq ($(CONFIG_CC_OPTIMIZE_FOR_SPEED),y)
udivsi3-$(CONFIG_CPU_SH4)	:= udivsi3_i4i.o
endif
udivsi3-y			+= udivsi3.o

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += movmem.o ashldi3.o ashrdi3.o lshrdi3.o \
				    ashlsi3.o ashrsi3.o ashiftrt.o lshrsi3.o \
				    udiv_qrnnd.o $(udivsi3-y)
