# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2009 Samsung Electronics
# Minkyu Kang <mk7.kang@samsung.com>

obj-y		+= cpu_info.o
ifndef CONFIG_SPL_BUILD
obj-y		+= timer.o
obj-y		+= sromc.o
obj-$(CONFIG_PWM)	+= pwm.o
endif
