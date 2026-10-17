# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_AP121) += ap121.dtb
dtb-$(CONFIG_TARGET_AP143) += ap143.dtb
dtb-$(CONFIG_TARGET_AP152) += ap152.dtb
dtb-$(CONFIG_TARGET_BOSTON) += img,boston.dtb
dtb-$(CONFIG_TARGET_MALTA) += mti,malta.dtb
dtb-$(CONFIG_TARGET_PIC32MZDASK) += pic32mzda_sk.dtb
dtb-$(CONFIG_TARGET_XILFPGA) += nexys4ddr.dtb
dtb-$(CONFIG_BOARD_BROADCOM_BCM968380GERG) += brcm,bcm968380gerg.dtb
dtb-$(CONFIG_BOARD_COMTREND_AR5315U) += comtrend,ar-5315u.dtb
dtb-$(CONFIG_BOARD_COMTREND_AR5387UN) += comtrend,ar-5387un.dtb
dtb-$(CONFIG_BOARD_COMTREND_CT5361) += comtrend,ct-5361.dtb
dtb-$(CONFIG_BOARD_COMTREND_VR3032U) += comtrend,vr-3032u.dtb
dtb-$(CONFIG_BOARD_COMTREND_WAP5813N) += comtrend,wap-5813n.dtb
dtb-$(CONFIG_BOARD_HUAWEI_HG556A) += huawei,hg556a.dtb
dtb-$(CONFIG_BOARD_INTENO_XG6846) += inteno,xg6846.dtb
dtb-$(CONFIG_BOARD_MT7620_RFB) += mediatek,mt7620-rfb.dtb
dtb-$(CONFIG_BOARD_MT7620_MT7530_RFB) += mediatek,mt7620-mt7530-rfb.dtb
dtb-$(CONFIG_BOARD_MT7621_RFB) += mediatek,mt7621-rfb.dtb
dtb-$(CONFIG_BOARD_MT7621_NAND_RFB) += mediatek,mt7621-nand-rfb.dtb
dtb-$(CONFIG_BOARD_MT7628_RFB) += mediatek,mt7628-rfb.dtb
dtb-$(CONFIG_BOARD_GARDENA_SMART_GATEWAY_MT7688) += gardena-smart-gateway-mt7688.dtb
dtb-$(CONFIG_BOARD_LINKIT_SMART_7688) += linkit-smart-7688.dtb
dtb-$(CONFIG_TARGET_OCTEON_EBB7304) += mrvl,octeon-ebb7304.dtb
dtb-$(CONFIG_TARGET_OCTEON_NIC23) += mrvl,octeon-nic23.dtb
dtb-$(CONFIG_BOARD_NETGEAR_CG3100D) += netgear,cg3100d.dtb
dtb-$(CONFIG_BOARD_NETGEAR_DGND3700V2) += netgear,dgnd3700v2.dtb
dtb-$(CONFIG_BOARD_SAGEM_FAST1704) += sagem,f@st1704.dtb
dtb-$(CONFIG_BOARD_SFR_NB4_SER) += sfr,nb4-ser.dtb
dtb-$(CONFIG_BOARD_TPLINK_WDR4300) += tplink_wdr4300.dtb
dtb-$(CONFIG_BOARD_VOCORE2) += vocore_vocore2.dtb
dtb-$(CONFIG_TARGET_JZ4780_CI20) += ci20.dtb
dtb-$(CONFIG_SOC_LUTON) += luton_pcb090.dtb luton_pcb091.dtb
dtb-$(CONFIG_SOC_OCELOT) += ocelot_pcb120.dtb ocelot_pcb123.dtb
dtb-$(CONFIG_SOC_JR2) += jr2_pcb110.dtb jr2_pcb111.dtb serval2_pcb112.dtb
dtb-$(CONFIG_SOC_SERVALT) += servalt_pcb116.dtb
dtb-$(CONFIG_SOC_SERVAL) += serval_pcb105.dtb serval_pcb106.dtb

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
