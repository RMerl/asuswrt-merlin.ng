# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_ADP_AG101P) += ag101p.dtb
dtb-$(CONFIG_TARGET_ADP_AE3XX) += ae3xx.dtb
targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
