# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2015 Stefan Roese <sr@denx.de>
# Copyright (C) 2015 Reinhard Pfau <reinhard.pfau@gdsys.cc>
# Copyright (C) 2016 Mario Six <mario.six@gdsys.cc>

obj-$(CONFIG_TARGET_CONTROLCENTERDC) += controlcenterdc.o hre.o keyprogram.o dt_helpers.o

ifeq ($(CONFIG_SPL_BUILD),)
obj-$(CONFIG_TARGET_CONTROLCENTERDC) += hydra.o ihs_phys.o
endif
