# SPDX-License-Identifier: GPL-2.0+

dtb-y += $(shell echo $(CONFIG_DEFAULT_DEVICE_TREE)).dtb

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
