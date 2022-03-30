# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2014 - 2016 Xilinx, Inc.
# Michal Simek <michal.simek@xilinx.com>

obj-y	:= zynqmp.o
obj-y	+= ../common/board.o

ifneq ($(CONFIG_XILINX_PS_INIT_FILE),"")
PS_INIT_FILE := $(shell cd $(srctree); readlink -f $(CONFIG_XILINX_PS_INIT_FILE))
init-objs := ps_init_gpl.o
spl/board/xilinx/zynqmp/ps_init_gpl.o board/xilinx/zynqmp/ps_init_gpl.o: $(PS_INIT_FILE)
	$(CC) $(c_flags) -I $(srctree)/$(src) -c -o $@ $^
endif

ifeq ($(init-objs),)
hw-platform-y :=$(shell echo $(CONFIG_DEFAULT_DEVICE_TREE))
init-objs := $(if $(wildcard $(srctree)/$(src)/$(hw-platform-y)/psu_init_gpl.c),\
	$(hw-platform-y)/psu_init_gpl.o)
endif

ifeq ($(init-objs),)
ifneq ($(wildcard $(srctree)/$(src)/psu_init_gpl.c),)
init-objs := psu_init_gpl.o
$(if $(CONFIG_SPL_BUILD),\
$(warning Put custom psu_init_gpl.c/h to board/xilinx/zynqmp/custom_hw_platform/))
endif
endif

ifdef_any_of = $(filter-out undefined,$(foreach v,$(1),$(origin $(v))))

ifneq ($(call ifdef_any_of, CONFIG_ZYNQMP_PSU_INIT_ENABLED CONFIG_SPL_BUILD),)
obj-y += $(init-objs)
endif

obj-$(CONFIG_MMC_SDHCI_ZYNQ) += tap_delays.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CMD_ZYNQMP) += cmds.o
endif

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_psu_init_gpl.o := -Wstrict-prototypes

# To include xil_io.h
CFLAGS_psu_init_gpl.o := -I$(srctree)/$(src)

# To suppress "warning: cast to pointer from integer of different size"
CFLAGS_psu_init_gpl.o += -Wno-int-to-pointer-cast
