#
# (C) Copyright 2016 Nexell
# junghyun kim<jhkim@nexell.co.kr>
#
# SPDX-License-Identifier:      GPL-2.0+
#

SOCDIR=CPUDIR/$(VENDOR)
MACHDIR=$(patsubst %,arch/arm/mach-%,$(machine-y))

LDPPFLAGS += -DMACHDIR=$(MACHDIR) -DSOCDIR=$(SOCDIR)
