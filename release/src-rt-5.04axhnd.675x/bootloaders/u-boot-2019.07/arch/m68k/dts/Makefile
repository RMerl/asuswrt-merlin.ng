# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_M52277EVB) += M52277EVB.dtb \
	M52277EVB_stmicro.dtb
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
dtb-$(CONFIG_TARGET_M54418TWR) += M54418TWR.dtb \
	M54418TWR_nand_mii.dtb \
	M54418TWR_nand_rmii.dtb \
	M54418TWR_serial_mii.dtb \
	M54418TWR_serial_rmii.dtb \
	M54418TWR_nand_rmii_lowfreq.dtb
dtb-$(CONFIG_TARGET_M54451EVB) += M54451EVB.dtb \
	M54451EVB_stmicro.dtb
dtb-$(CONFIG_TARGET_M54455EVB) += M54455EVB.dtb \
	M54455EVB_intel.dtb \
	M54455EVB_stm33.dtb \
	M54455EVB_a66.dtb \
	M54455EVB_i66.dtb
dtb-$(CONFIG_TARGET_AMCORE) += amcore.dtb
dtb-$(CONFIG_TARGET_STMARK2) += stmark2.dtb
dtb-$(CONFIG_TARGET_M5475EVB) += M5475AFE.dtb \
	M5475BFE.dtb \
	M5475CFE.dtb \
	M5475DFE.dtb \
	M5475EFE.dtb \
	M5475FFE.dtb \
	M5475GFE.dtb
dtb-$(CONFIG_TARGET_M5485EVB) += M5485AFE.dtb \
	M5485BFE.dtb \
	M5485CFE.dtb \
	M5485DFE.dtb \
	M5485EFE.dtb \
	M5485FFE.dtb \
	M5485GFE.dtb \
	M5485HFE.dtb

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
