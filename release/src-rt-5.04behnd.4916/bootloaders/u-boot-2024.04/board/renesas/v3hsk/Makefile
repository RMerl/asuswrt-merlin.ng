#
# board/renesas/v3hsk/Makefile
#
# Copyright (C) 2019 Renesas Electronics Corporation
# Copyright (C) 2019 Cogent Embedded, Inc.
#
# SPDX-License-Identifier: GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y	:= ../rcar-common/gen3-spl.o
else
obj-y	:= ../rcar-common/v3-common.o ../rcar-common/common.o
obj-$(CONFIG_SYSRESET)	+= cpld.o
endif
