# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.

# This Makefile builds the internal U-Boot fdt if CONFIG_OF_CONTROL is
# enabled. See doc/README.fdt-control for more details.

DEVICE_TREE ?= $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%)
ifeq ($(DEVICE_TREE),)
DEVICE_TREE := unset
endif

ifneq ($(EXT_DTB),)
DTB := $(EXT_DTB)
else
DTB := arch/$(ARCH)/dts/$(DEVICE_TREE).dtb
endif

$(obj)/dt-spl.dtb: $(DTB) $(objtree)/tools/fdtgrep FORCE
	$(call if_changed,fdtgrep)

$(obj)/dt.dtb: $(DTB) FORCE
	$(call if_changed,shipped)

targets += dt.dtb dt-spl.dtb

$(DTB): arch-dtbs
	$(Q)test -e $@ || (						\
	echo >&2;							\
	echo >&2 "Device Tree Source is not correctly specified.";	\
	echo >&2 "Please define 'CONFIG_DEFAULT_DEVICE_TREE'";		\
	echo >&2 "or build with 'DEVICE_TREE=<device_tree>' argument";	\
	echo >&2;							\
	/bin/false)

PHONY += arch-dtbs
arch-dtbs:
	$(Q)$(MAKE) $(build)=arch/$(ARCH)/dts dtbs

ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_OF_EMBED) := dt-spl.dtb.o
# support "out-of-tree" build for dtb-spl
$(obj)/dt-spl.dtb.o: $(obj)/dt-spl.dtb.S FORCE
	$(call if_changed_dep,as_o_S)
else
obj-$(CONFIG_OF_EMBED) := dt.dtb.o
endif

dtbs: $(obj)/dt.dtb $(obj)/dt-spl.dtb
	@:

clean-files := dt.dtb.S dt-spl.dtb.S

# Let clean descend into dts directories
subdir- += ../arch/arm/dts ../arch/microblaze/dts ../arch/mips/dts ../arch/sandbox/dts ../arch/x86/dts ../arch/powerpc/dts ../arch/riscv/dts
