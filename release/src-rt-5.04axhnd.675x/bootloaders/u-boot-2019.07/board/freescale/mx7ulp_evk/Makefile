# SPDX-License-Identifier: GPL-2.0+
# (C) Copyright 2016 Freescale Semiconductor, Inc.

obj-y  := mx7ulp_evk.o

extra-$(CONFIG_USE_PLUGIN) :=  plugin.bin
$(obj)/plugin.bin: $(obj)/plugin.o
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@
