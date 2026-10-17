# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_XTENSA) += ml605.dtb ml605_nommu.dtb kc705.dtb kc705_nommu.dtb

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
