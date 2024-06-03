# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_XTFPGA) += ml605.dtb ml605_nommu.dtb kc705.dtb kc705_nommu.dtb

targets += $(dtb-y)

DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
