# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_T2080QDS) += t2080qds.dtb
dtb-$(CONFIG_TARGET_MCR3000) += mcr3000.dtb
dtb-$(CONFIG_TARGET_GAZERBEAM) += gazerbeam.dtb

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
