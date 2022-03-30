# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_ARCH_MTMIPS) += \
	gardena-smart-gateway-mt7688.dtb \
	linkit-smart-7688.dtb
dtb-$(CONFIG_TARGET_AP121) += ap121.dtb
dtb-$(CONFIG_TARGET_AP143) += ap143.dtb
dtb-$(CONFIG_TARGET_AP152) += ap152.dtb
dtb-$(CONFIG_TARGET_BOSTON) += img,boston.dtb
dtb-$(CONFIG_TARGET_MALTA) += mti,malta.dtb
dtb-$(CONFIG_TARGET_PIC32MZDASK) += pic32mzda_sk.dtb
dtb-$(CONFIG_TARGET_XILFPGA) += nexys4ddr.dtb
dtb-$(CONFIG_BOARD_COMTREND_AR5315U) += comtrend,ar-5315u.dtb
dtb-$(CONFIG_BOARD_COMTREND_AR5387UN) += comtrend,ar-5387un.dtb
dtb-$(CONFIG_BOARD_COMTREND_CT5361) += comtrend,ct-5361.dtb
dtb-$(CONFIG_BOARD_COMTREND_VR3032U) += comtrend,vr-3032u.dtb
dtb-$(CONFIG_BOARD_COMTREND_WAP5813N) += comtrend,wap-5813n.dtb
dtb-$(CONFIG_BOARD_HUAWEI_HG556A) += huawei,hg556a.dtb
dtb-$(CONFIG_BOARD_NETGEAR_CG3100D) += netgear,cg3100d.dtb
dtb-$(CONFIG_BOARD_NETGEAR_DGND3700V2) += netgear,dgnd3700v2.dtb
dtb-$(CONFIG_BOARD_SAGEM_FAST1704) += sagem,f@st1704.dtb
dtb-$(CONFIG_BOARD_TPLINK_WDR4300) += tplink_wdr4300.dtb
dtb-$(CONFIG_TARGET_JZ4780_CI20) += ci20.dtb
dtb-$(CONFIG_SOC_BMIPS_BCM6358) += sfr,nb4-ser.dtb
dtb-$(CONFIG_SOC_BMIPS_BCM6838) += brcm,bcm968380gerg.dtb
dtb-$(CONFIG_SOC_LUTON) += luton_pcb090.dtb luton_pcb091.dtb
dtb-$(CONFIG_SOC_OCELOT) += ocelot_pcb120.dtb ocelot_pcb123.dtb
dtb-$(CONFIG_SOC_JR2) += jr2_pcb110.dtb jr2_pcb111.dtb serval2_pcb112.dtb
dtb-$(CONFIG_SOC_SERVALT) += servalt_pcb116.dtb
dtb-$(CONFIG_SOC_SERVAL) += serval_pcb105.dtb serval_pcb106.dtb

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
