# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

objs-before-objcopy := 20001122-1.o 20010114-2.o 20010226-1.o 980619-1.o \
	acc1.o compare-fp-1.o fpu.o mul-subnormal-single-1.o darwin-ldouble.o
targets += $(objs-before-objcopy)

# remove -msoft-float flag
$(foreach m, $(objs-before-objcopy), $(eval CFLAGS_REMOVE_$m := -msoft-float))
ccflags-y := -mhard-float -fkeep-inline-functions

obj-y := $(objs-before-objcopy:.o=_.o)

OBJCOPYFLAGS := -R .gnu.attributes
$(obj)/%_.o: $(obj)/%.o FORCE
	$(call if_changed,objcopy)
