# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_SANDBOX64
dtb-$(CONFIG_SANDBOX) += sandbox64.dtb
else
dtb-$(CONFIG_SANDBOX) += sandbox.dtb
endif
dtb-$(CONFIG_UT_DM) += test.dtb other.dtb
dtb-$(CONFIG_CMD_EXTENSION) += overlay0.dtbo overlay1.dtbo

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb *.dtbo
