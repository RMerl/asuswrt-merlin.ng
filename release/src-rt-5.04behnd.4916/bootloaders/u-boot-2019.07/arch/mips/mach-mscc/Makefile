# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

CFLAGS_cpu.o += -finline-limit=64000

obj-y += cpu.o dram.o reset.o phy.o lowlevel_init.o
obj-$(CONFIG_SOC_LUTON) += lowlevel_init_luton.o gpio.o
obj-$(CONFIG_SOC_OCELOT) += gpio.o
obj-$(CONFIG_SOC_SERVAL) += gpio.o
