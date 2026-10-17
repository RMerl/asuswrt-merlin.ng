# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2014 - 2016 Xilinx, Inc.
# Michal Simek <michal.simek@amd.com>

obj-y	:= zynqmp.o

ifneq ($(CONFIG_XILINX_PS_INIT_FILE),"")
PS_INIT_FILE := $(shell cd $(srctree); readlink -f $(CONFIG_XILINX_PS_INIT_FILE))
init-objs := ps_init_gpl.o
spl/board/xilinx/zynqmp/ps_init_gpl.o board/xilinx/zynqmp/ps_init_gpl.o: $(PS_INIT_FILE)
	$(CC) $(c_flags) -I $(srctree)/$(src) -c -o $@ $^
endif

DEVICE_TREE ?= $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%)
ifeq ($(DEVICE_TREE),)
DEVICE_TREE := unset
endif

ifeq ($(init-objs),)
hw-platform-y :=$(shell echo $(DEVICE_TREE))
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

obj-$(CONFIG_$(SPL_)ZYNQMP_PSU_INIT_ENABLED) += $(init-objs)

ifdef CONFIG_SPL_BUILD
ifneq ($(CONFIG_ZYNQMP_SPL_PM_CFG_OBJ_FILE),"")
obj-$(CONFIG_SPL_BUILD) += pm_cfg_obj.o
$(obj)/pm_cfg_obj.o: $(shell cd $(srctree); readlink -f $(CONFIG_ZYNQMP_SPL_PM_CFG_OBJ_FILE)) FORCE
endif
endif

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CMD_ZYNQMP) += cmds.o
endif

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_psu_init_gpl.o := -Wstrict-prototypes

# To include xil_io.h
CFLAGS_psu_init_gpl.o := -I$(srctree)/$(src)

# To suppress "warning: cast to pointer from integer of different size"
CFLAGS_psu_init_gpl.o += -Wno-int-to-pointer-cast
