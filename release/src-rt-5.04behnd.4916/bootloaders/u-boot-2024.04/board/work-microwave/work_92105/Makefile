# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2014  DENX Software Engineering GmbH
# Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>

ifdef CONFIG_SPL_BUILD
obj-y	+= work_92105_spl.o
else
obj-y	+= work_92105.o
obj-$(CONFIG_DEPRECATED) += work_92105_display.o
endif
