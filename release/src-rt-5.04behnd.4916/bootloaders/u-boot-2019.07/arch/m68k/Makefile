# SPDX-License-Identifier: GPL-2.0+

head-y := arch/m68k/cpu/$(CPU)/start.o

libs-y += arch/m68k/cpu/$(CPU)/
libs-y += arch/m68k/lib/

cpuflags-$(CONFIG_M5208)	:= -mcpu=5208
cpuflags-$(CONFIG_M5235)	:= -mcpu=5235 -fPIC
cpuflags-$(CONFIG_M52277)	:= -mcpu=52277 -fPIC
cpuflags-$(CONFIG_M5249)	:= -mcpu=5249
cpuflags-$(CONFIG_M5253)	:= -mcpu=5253
cpuflags-$(CONFIG_M5271)	:= -mcpu=5271
cpuflags-$(CONFIG_M5272)	:= -mcpu=5272
cpuflags-$(CONFIG_M5275)	:= -mcpu=5275
cpuflags-$(CONFIG_M5282)	:= -mcpu=5282
cpuflags-$(CONFIG_M5307)	:= -mcpu=5307
cpuflags-$(CONFIG_MCF5301x)	:= -mcpu=53015 -fPIC
cpuflags-$(CONFIG_MCF532x)	:= -mcpu=5329 -fPIC
cpuflags-$(CONFIG_MCF5441x)	:= -mcpu=54418 -fPIC
cpuflags-$(CONFIG_MCF5445x)	:= -mcpu=54455 -fPIC
cpuflags-$(CONFIG_MCF547x_8x)	:= -mcpu=5485 -fPIC

PLATFORM_CPPFLAGS += $(cpuflags-y)


ldflags-$(CONFIG_MCF5441x)	:= --got=single
ldflags-$(CONFIG_MCF5445x)	:= --got=single
ldflags-$(CONFIG_MCF547x_8x)	:= --got=single

ifneq (,$(findstring -linux-,$(shell $(CC) --version)))
ifneq (,$(findstring GOT,$(shell $(LD) --help)))
PLATFORM_LDFLAGS += $(ldflags-y)
endif
endif
