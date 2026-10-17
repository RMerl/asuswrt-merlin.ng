# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y	:= board.o

ifneq ($(CONFIG_XILINX_PS_INIT_FILE),"")
PS_INIT_FILE := $(shell cd $(srctree); readlink -f $(CONFIG_XILINX_PS_INIT_FILE))
init-objs := ps_init_gpl.o
spl/board/xilinx/zynq/ps_init_gpl.o board/xilinx/zynq/ps_init_gpl.o: $(PS_INIT_FILE)
	$(CC) $(c_flags) -I $(srctree)/$(src) -c -o $@ $^
endif

DEVICE_TREE ?= $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%)
ifeq ($(DEVICE_TREE),)
DEVICE_TREE := unset
endif

ifeq ($(init-objs),)
hw-platform-y :=$(shell echo $(DEVICE_TREE))
init-objs := $(if $(wildcard $(srctree)/$(src)/$(hw-platform-y)/ps7_init_gpl.c),\
	$(hw-platform-y)/ps7_init_gpl.o)
endif

ifeq ($(init-objs),)
ifneq ($(wildcard $(srctree)/$(src)/ps7_init_gpl.c),)
init-objs := ps7_init_gpl.o
$(if $(CONFIG_SPL_BUILD),\
$(warning Put custom ps7_init_gpl.c/h to board/xilinx/zynq/custom_hw_platform/))
endif
endif

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CMD_ZYNQ) += cmds.o
obj-$(CONFIG_CMD_ZYNQ_RSA) += bootimg.o
endif

obj-$(CONFIG_SPL_BUILD) += $(init-objs)

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_ps7_init_gpl.o := -Wstrict-prototypes

# To include xil_io.h
CFLAGS_ps7_init_gpl.o := -I$(srctree)/$(src)
