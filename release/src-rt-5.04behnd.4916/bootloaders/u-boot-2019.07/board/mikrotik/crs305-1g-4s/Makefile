# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2015 Stefan Roese <sr@denx.de>

obj-y	:= crs305-1g-4s.o
extra-y	:= kwbimage.cfg

quiet_cmd_sed = SED     $@
      cmd_sed = sed $(SEDFLAGS_$(@F)) $< >$(dir $<)$(@F)

SEDFLAGS_kwbimage.cfg =-e "s|^BINARY.*|BINARY $(srctree)/$(@D)/binary.0 0000005b 00000068|"
$(src)/kwbimage.cfg: $(src)/kwbimage.cfg.in include/autoconf.mk \
		include/config/auto.conf
	  $(call if_changed,sed)
