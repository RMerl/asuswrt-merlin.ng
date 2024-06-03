# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifdef CONFIG_SPL_BUILD
# necessary to create built-in.o
obj- := __dummy__.o
else
obj-y	:= fpga.o x600.o
endif
