# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_M5235EVB) += M5235EVB.dtb \
	M5235EVB_Flash32.dtb
dtb-$(CONFIG_TARGET_COBRA5272) += cobra5272.dtb
dtb-$(CONFIG_TARGET_EB_CPU5282) += eb_cpu5282.dtb \
	eb_cpu5282_internal.dtb
dtb-$(CONFIG_TARGET_M5208EVBE) += M5208EVBE.dtb
dtb-$(CONFIG_TARGET_M5249EVB) += M5249EVB.dtb
dtb-$(CONFIG_TARGET_M5253DEMO) += M5253DEMO.dtb
dtb-$(CONFIG_TARGET_M5272C3) += M5272C3.dtb
dtb-$(CONFIG_TARGET_M5275EVB) += M5275EVB.dtb
dtb-$(CONFIG_TARGET_M5282EVB) += M5282EVB.dtb
dtb-$(CONFIG_TARGET_ASTRO_MCF5373L) += astro_mcf5373l.dtb
dtb-$(CONFIG_TARGET_M53017EVB) += M53017EVB.dtb
dtb-$(CONFIG_TARGET_M5329EVB) += M5329AFEE.dtb M5329BFEE.dtb
dtb-$(CONFIG_TARGET_M5373EVB) += M5373EVB.dtb
dtb-$(CONFIG_TARGET_AMCORE) += amcore.dtb
dtb-$(CONFIG_TARGET_STMARK2) += stmark2.dtb

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
