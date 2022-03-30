# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>

obj-y				+= cpu.o misc.o timer.o psc.o pinmux.o reset.o
obj-$(CONFIG_DA850_LOWLEVEL)	+= da850_lowlevel.o
obj-$(CONFIG_SOC_DM355)	+= dm355.o
obj-$(CONFIG_SOC_DM365)	+= dm365.o
obj-$(CONFIG_SOC_DM644X)	+= dm644x.o
obj-$(CONFIG_SOC_DM646X)	+= dm646x.o
obj-$(CONFIG_SOC_DA850)	+= da850_pinmux.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_FRAMEWORK)	+= spl.o
obj-$(CONFIG_SOC_DM365)	+= dm365_lowlevel.o
obj-$(CONFIG_SOC_DA8XX)	+= da850_lowlevel.o
endif
