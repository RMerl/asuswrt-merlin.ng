# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2014 Marek Vasut <marex@denx.de>

ifdef CONFIG_SPL_BUILD
obj-y	:= novena_spl.o
else
obj-y	:= novena.o
obj-$(CONFIG_VIDEO_IPUV3)	+= video.o
endif
