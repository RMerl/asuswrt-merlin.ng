// SPDX-License-Identifier: GPL-2.0-only
/*
 * ROCKCHIP Type-C PHY driver.
 *
 * Copyright (C) 2020 Amarula Solutions(India)
 * Copyright (C) Fuzhou Rockchip Electronics Co.Ltd
 * Author: Chris Zhong <zyw@rock-chips.com>
 *         Kever Yang <kever.yang@rock-chips.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <generic-phy.h>
#include <reset.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include <asm/arch-rockchip/clock.h>

DECLARE_GLOBAL_DATA_PTR;

#define usleep_range(a, b) udelay((b))

#define CMN_SSM_BANDGAP			(0x21 << 2)
#define CMN_SSM_BIAS			(0x22 << 2)
#define CMN_PLLSM0_PLLEN		(0x29 << 2)
#define CMN_PLLSM0_PLLPRE		(0x2a << 2)
#define CMN_PLLSM0_PLLVREF		(0x2b << 2)
#define CMN_PLLSM0_PLLLOCK		(0x2c << 2)
#define CMN_PLLSM1_PLLEN		(0x31 << 2)
#define CMN_PLLSM1_PLLPRE		(0x32 << 2)
#define CMN_PLLSM1_PLLVREF		(0x33 << 2)
#define CMN_PLLSM1_PLLLOCK		(0x34 << 2)
#define CMN_PLLSM1_USER_DEF_CTRL	(0x37 << 2)
#define CMN_ICAL_OVRD			(0xc1 << 2)
#define CMN_PLL0_VCOCAL_OVRD		(0x83 << 2)
#define CMN_PLL0_VCOCAL_INIT		(0x84 << 2)
#define CMN_PLL0_VCOCAL_ITER		(0x85 << 2)
#define CMN_PLL0_LOCK_REFCNT_START	(0x90 << 2)
#define CMN_PLL0_LOCK_PLLCNT_START	(0x92 << 2)
#define CMN_PLL0_LOCK_PLLCNT_THR	(0x93 << 2)
#define CMN_PLL0_INTDIV			(0x94 << 2)
#define CMN_PLL0_FRACDIV		(0x95 << 2)
#define CMN_PLL0_HIGH_THR		(0x96 << 2)
#define CMN_PLL0_DSM_DIAG		(0x97 << 2)
#define CMN_PLL0_SS_CTRL1		(0x98 << 2)
#define CMN_PLL0_SS_CTRL2		(0x99 << 2)
#define CMN_PLL1_VCOCAL_START		(0xa1 << 2)
#define CMN_PLL1_VCOCAL_OVRD		(0xa3 << 2)
#define CMN_PLL1_VCOCAL_INIT		(0xa4 << 2)
#define CMN_PLL1_VCOCAL_ITER		(0xa5 << 2)
#define CMN_PLL1_LOCK_REFCNT_START	(0xb0 << 2)
#define CMN_PLL1_LOCK_PLLCNT_START	(0xb2 << 2)
#define CMN_PLL1_LOCK_PLLCNT_THR	(0xb3 << 2)
#define CMN_PLL1_INTDIV			(0xb4 << 2)
#define CMN_PLL1_FRACDIV		(0xb5 << 2)
#define CMN_PLL1_HIGH_THR		(0xb6 << 2)
#define CMN_PLL1_DSM_DIAG		(0xb7 << 2)
#define CMN_PLL1_SS_CTRL1		(0xb8 << 2)
#define CMN_PLL1_SS_CTRL2		(0xb9 << 2)
#define CMN_RXCAL_OVRD			(0xd1 << 2)

#define CMN_TXPUCAL_CTRL		(0xe0 << 2)
#define CMN_TXPUCAL_OVRD		(0xe1 << 2)
#define CMN_TXPDCAL_CTRL		(0xf0 << 2)
#define CMN_TXPDCAL_OVRD		(0xf1 << 2)

/* For CMN_TXPUCAL_CTRL, CMN_TXPDCAL_CTRL */
#define CMN_TXPXCAL_START		BIT(15)
#define CMN_TXPXCAL_DONE		BIT(14)
#define CMN_TXPXCAL_NO_RESPONSE		BIT(13)
#define CMN_TXPXCAL_CURRENT_RESPONSE	BIT(12)

#define CMN_TXPU_ADJ_CTRL		(0x108 << 2)
#define CMN_TXPD_ADJ_CTRL		(0x10c << 2)

/*
 * For CMN_TXPUCAL_CTRL, CMN_TXPDCAL_CTRL,
 *     CMN_TXPU_ADJ_CTRL, CMN_TXPDCAL_CTRL
 *
 * NOTE: some of these registers are documented to be 2's complement
 * signed numbers, but then documented to be always positive.  Weird.
 * In such a case, using CMN_CALIB_CODE_POS() avoids the unnecessary
 * sign extension.
 */
#define CMN_CALIB_CODE_WIDTH	7
#define CMN_CALIB_CODE_OFFSET	0
#define CMN_CALIB_CODE_MASK	GENMASK(CMN_CALIB_CODE_WIDTH, 0)
#define CMN_CALIB_CODE(x)	\
	sign_extend32((x) >> CMN_CALIB_CODE_OFFSET, CMN_CALIB_CODE_WIDTH)

#define CMN_CALIB_CODE_POS_MASK	GENMASK(CMN_CALIB_CODE_WIDTH - 1, 0)
#define CMN_CALIB_CODE_POS(x)	\
	(((x) >> CMN_CALIB_CODE_OFFSET) & CMN_CALIB_CODE_POS_MASK)

#define CMN_DIAG_PLL0_FBH_OVRD		(0x1c0 << 2)
#define CMN_DIAG_PLL0_FBL_OVRD		(0x1c1 << 2)
#define CMN_DIAG_PLL0_OVRD		(0x1c2 << 2)
#define CMN_DIAG_PLL0_V2I_TUNE		(0x1c5 << 2)
#define CMN_DIAG_PLL0_CP_TUNE		(0x1c6 << 2)
#define CMN_DIAG_PLL0_LF_PROG		(0x1c7 << 2)
#define CMN_DIAG_PLL1_FBH_OVRD		(0x1d0 << 2)
#define CMN_DIAG_PLL1_FBL_OVRD		(0x1d1 << 2)
#define CMN_DIAG_PLL1_OVRD		(0x1d2 << 2)
#define CMN_DIAG_PLL1_V2I_TUNE		(0x1d5 << 2)
#define CMN_DIAG_PLL1_CP_TUNE		(0x1d6 << 2)
#define CMN_DIAG_PLL1_LF_PROG		(0x1d7 << 2)
#define CMN_DIAG_PLL1_PTATIS_TUNE1	(0x1d8 << 2)
#define CMN_DIAG_PLL1_PTATIS_TUNE2	(0x1d9 << 2)
#define CMN_DIAG_PLL1_INCLK_CTRL	(0x1da << 2)
#define CMN_DIAG_HSCLK_SEL		(0x1e0 << 2)

#define XCVR_PSM_RCTRL(n)		((0x4001 | ((n) << 9)) << 2)
#define XCVR_PSM_CAL_TMR(n)		((0x4002 | ((n) << 9)) << 2)
#define XCVR_PSM_A0IN_TMR(n)		((0x4003 | ((n) << 9)) << 2)
#define TX_TXCC_CAL_SCLR_MULT(n)	((0x4047 | ((n) << 9)) << 2)
#define TX_TXCC_CPOST_MULT_00(n)	((0x404c | ((n) << 9)) << 2)
#define TX_TXCC_CPOST_MULT_01(n)	((0x404d | ((n) << 9)) << 2)
#define TX_TXCC_CPOST_MULT_10(n)	((0x404e | ((n) << 9)) << 2)
#define TX_TXCC_CPOST_MULT_11(n)	((0x404f | ((n) << 9)) << 2)
#define TX_TXCC_MGNFS_MULT_000(n)	((0x4050 | ((n) << 9)) << 2)
#define TX_TXCC_MGNFS_MULT_001(n)	((0x4051 | ((n) << 9)) << 2)
#define TX_TXCC_MGNFS_MULT_010(n)	((0x4052 | ((n) << 9)) << 2)
#define TX_TXCC_MGNFS_MULT_011(n)	((0x4053 | ((n) << 9)) << 2)
#define TX_TXCC_MGNFS_MULT_100(n)	((0x4054 | ((n) << 9)) << 2)
#define TX_TXCC_MGNFS_MULT_101(n)	((0x4055 | ((n) << 9)) << 2)
#define TX_TXCC_MGNFS_MULT_110(n)	((0x4056 | ((n) << 9)) << 2)
#define TX_TXCC_MGNFS_MULT_111(n)	((0x4057 | ((n) << 9)) << 2)
#define TX_TXCC_MGNLS_MULT_000(n)	((0x4058 | ((n) << 9)) << 2)
#define TX_TXCC_MGNLS_MULT_001(n)	((0x4059 | ((n) << 9)) << 2)
#define TX_TXCC_MGNLS_MULT_010(n)	((0x405a | ((n) << 9)) << 2)
#define TX_TXCC_MGNLS_MULT_011(n)	((0x405b | ((n) << 9)) << 2)
#define TX_TXCC_MGNLS_MULT_100(n)	((0x405c | ((n) << 9)) << 2)
#define TX_TXCC_MGNLS_MULT_101(n)	((0x405d | ((n) << 9)) << 2)
#define TX_TXCC_MGNLS_MULT_110(n)	((0x405e | ((n) << 9)) << 2)
#define TX_TXCC_MGNLS_MULT_111(n)	((0x405f | ((n) << 9)) << 2)

#define XCVR_DIAG_PLLDRC_CTRL(n)	((0x40e0 | ((n) << 9)) << 2)
#define XCVR_DIAG_BIDI_CTRL(n)		((0x40e8 | ((n) << 9)) << 2)
#define XCVR_DIAG_LANE_FCM_EN_MGN(n)	((0x40f2 | ((n) << 9)) << 2)
#define TX_PSC_A0(n)			((0x4100 | ((n) << 9)) << 2)
#define TX_PSC_A1(n)			((0x4101 | ((n) << 9)) << 2)
#define TX_PSC_A2(n)			((0x4102 | ((n) << 9)) << 2)
#define TX_PSC_A3(n)			((0x4103 | ((n) << 9)) << 2)
#define TX_RCVDET_CTRL(n)		((0x4120 | ((n) << 9)) << 2)
#define TX_RCVDET_EN_TMR(n)		((0x4122 | ((n) << 9)) << 2)
#define TX_RCVDET_ST_TMR(n)		((0x4123 | ((n) << 9)) << 2)
#define TX_DIAG_TX_DRV(n)		((0x41e1 | ((n) << 9)) << 2)
#define TX_DIAG_BGREF_PREDRV_DELAY	(0x41e7 << 2)

/* Use this for "n" in macros like "_MULT_XXX" to target the aux channel */
#define AUX_CH_LANE			8

#define TX_ANA_CTRL_REG_1		(0x5020 << 2)

#define TXDA_DP_AUX_EN			BIT(15)
#define AUXDA_SE_EN			BIT(14)
#define TXDA_CAL_LATCH_EN		BIT(13)
#define AUXDA_POLARITY			BIT(12)
#define TXDA_DRV_POWER_ISOLATION_EN	BIT(11)
#define TXDA_DRV_POWER_EN_PH_2_N	BIT(10)
#define TXDA_DRV_POWER_EN_PH_1_N	BIT(9)
#define TXDA_BGREF_EN			BIT(8)
#define TXDA_DRV_LDO_EN			BIT(7)
#define TXDA_DECAP_EN_DEL		BIT(6)
#define TXDA_DECAP_EN			BIT(5)
#define TXDA_UPHY_SUPPLY_EN_DEL		BIT(4)
#define TXDA_UPHY_SUPPLY_EN		BIT(3)
#define TXDA_LOW_LEAKAGE_EN		BIT(2)
#define TXDA_DRV_IDLE_LOWI_EN		BIT(1)
#define TXDA_DRV_CMN_MODE_EN		BIT(0)

#define TX_ANA_CTRL_REG_2		(0x5021 << 2)

#define AUXDA_DEBOUNCING_CLK		BIT(15)
#define TXDA_LPBK_RECOVERED_CLK_EN	BIT(14)
#define TXDA_LPBK_ISI_GEN_EN		BIT(13)
#define TXDA_LPBK_SERIAL_EN		BIT(12)
#define TXDA_LPBK_LINE_EN		BIT(11)
#define TXDA_DRV_LDO_REDC_SINKIQ	BIT(10)
#define XCVR_DECAP_EN_DEL		BIT(9)
#define XCVR_DECAP_EN			BIT(8)
#define TXDA_MPHY_ENABLE_HS_NT		BIT(7)
#define TXDA_MPHY_SA_MODE		BIT(6)
#define TXDA_DRV_LDO_RBYR_FB_EN		BIT(5)
#define TXDA_DRV_RST_PULL_DOWN		BIT(4)
#define TXDA_DRV_LDO_BG_FB_EN		BIT(3)
#define TXDA_DRV_LDO_BG_REF_EN		BIT(2)
#define TXDA_DRV_PREDRV_EN_DEL		BIT(1)
#define TXDA_DRV_PREDRV_EN		BIT(0)

#define TXDA_COEFF_CALC_CTRL		(0x5022 << 2)

#define TX_HIGH_Z			BIT(6)
#define TX_VMARGIN_OFFSET		3
#define TX_VMARGIN_MASK			0x7
#define LOW_POWER_SWING_EN		BIT(2)
#define TX_FCM_DRV_MAIN_EN		BIT(1)
#define TX_FCM_FULL_MARGIN		BIT(0)

#define TX_DIG_CTRL_REG_2		(0x5024 << 2)

#define TX_HIGH_Z_TM_EN			BIT(15)
#define TX_RESCAL_CODE_OFFSET		0
#define TX_RESCAL_CODE_MASK		0x3f

#define TXDA_CYA_AUXDA_CYA		(0x5025 << 2)
#define TX_ANA_CTRL_REG_3		(0x5026 << 2)
#define TX_ANA_CTRL_REG_4		(0x5027 << 2)
#define TX_ANA_CTRL_REG_5		(0x5029 << 2)

#define RX_PSC_A0(n)			((0x8000 | ((n) << 9)) << 2)
#define RX_PSC_A1(n)			((0x8001 | ((n) << 9)) << 2)
#define RX_PSC_A2(n)			((0x8002 | ((n) << 9)) << 2)
#define RX_PSC_A3(n)			((0x8003 | ((n) << 9)) << 2)
#define RX_PSC_CAL(n)			((0x8006 | ((n) << 9)) << 2)
#define RX_PSC_RDY(n)			((0x8007 | ((n) << 9)) << 2)
#define RX_IQPI_ILL_CAL_OVRD		(0x8023 << 2)
#define RX_EPI_ILL_CAL_OVRD		(0x8033 << 2)
#define RX_SDCAL0_OVRD			(0x8041 << 2)
#define RX_SDCAL1_OVRD			(0x8049 << 2)
#define RX_SLC_INIT			(0x806d << 2)
#define RX_SLC_RUN			(0x806e << 2)
#define RX_CDRLF_CNFG2			(0x8081 << 2)
#define RX_SIGDET_HL_FILT_TMR(n)	((0x8090 | ((n) << 9)) << 2)
#define RX_SLC_IOP0_OVRD		(0x8101 << 2)
#define RX_SLC_IOP1_OVRD		(0x8105 << 2)
#define RX_SLC_QOP0_OVRD		(0x8109 << 2)
#define RX_SLC_QOP1_OVRD		(0x810d << 2)
#define RX_SLC_EOP0_OVRD		(0x8111 << 2)
#define RX_SLC_EOP1_OVRD		(0x8115 << 2)
#define RX_SLC_ION0_OVRD		(0x8119 << 2)
#define RX_SLC_ION1_OVRD		(0x811d << 2)
#define RX_SLC_QON0_OVRD		(0x8121 << 2)
#define RX_SLC_QON1_OVRD		(0x8125 << 2)
#define RX_SLC_EON0_OVRD		(0x8129 << 2)
#define RX_SLC_EON1_OVRD		(0x812d << 2)
#define RX_SLC_IEP0_OVRD		(0x8131 << 2)
#define RX_SLC_IEP1_OVRD		(0x8135 << 2)
#define RX_SLC_QEP0_OVRD		(0x8139 << 2)
#define RX_SLC_QEP1_OVRD		(0x813d << 2)
#define RX_SLC_EEP0_OVRD		(0x8141 << 2)
#define RX_SLC_EEP1_OVRD		(0x8145 << 2)
#define RX_SLC_IEN0_OVRD		(0x8149 << 2)
#define RX_SLC_IEN1_OVRD		(0x814d << 2)
#define RX_SLC_QEN0_OVRD		(0x8151 << 2)
#define RX_SLC_QEN1_OVRD		(0x8155 << 2)
#define RX_SLC_EEN0_OVRD		(0x8159 << 2)
#define RX_SLC_EEN1_OVRD		(0x815d << 2)
#define RX_REE_CTRL_DATA_MASK(n)	((0x81bb | ((n) << 9)) << 2)
#define RX_DIAG_SIGDET_TUNE(n)		((0x81dc | ((n) << 9)) << 2)
#define RX_DIAG_SC2C_DELAY		(0x81e1 << 2)

#define PMA_LANE_CFG			(0xc000 << 2)
#define PIPE_CMN_CTRL1			(0xc001 << 2)
#define PIPE_CMN_CTRL2			(0xc002 << 2)
#define PIPE_COM_LOCK_CFG1		(0xc003 << 2)
#define PIPE_COM_LOCK_CFG2		(0xc004 << 2)
#define PIPE_RCV_DET_INH		(0xc005 << 2)
#define DP_MODE_CTL			(0xc008 << 2)
#define DP_CLK_CTL			(0xc009 << 2)
#define STS				(0xc00F << 2)
#define PHY_ISO_CMN_CTRL		(0xc010 << 2)
#define PHY_DP_TX_CTL			(0xc408 << 2)
#define PMA_CMN_CTRL1			(0xc800 << 2)
#define PHY_PMA_ISO_CMN_CTRL		(0xc810 << 2)
#define PHY_ISOLATION_CTRL		(0xc81f << 2)
#define PHY_PMA_ISO_XCVR_CTRL(n)	((0xcc11 | ((n) << 6)) << 2)
#define PHY_PMA_ISO_LINK_MODE(n)	((0xcc12 | ((n) << 6)) << 2)
#define PHY_PMA_ISO_PWRST_CTRL(n)	((0xcc13 | ((n) << 6)) << 2)
#define PHY_PMA_ISO_TX_DATA_LO(n)	((0xcc14 | ((n) << 6)) << 2)
#define PHY_PMA_ISO_TX_DATA_HI(n)	((0xcc15 | ((n) << 6)) << 2)
#define PHY_PMA_ISO_RX_DATA_LO(n)	((0xcc16 | ((n) << 6)) << 2)
#define PHY_PMA_ISO_RX_DATA_HI(n)	((0xcc17 | ((n) << 6)) << 2)
#define TX_BIST_CTRL(n)			((0x4140 | ((n) << 9)) << 2)
#define TX_BIST_UDDWR(n)		((0x4141 | ((n) << 9)) << 2)

/*
 * Selects which PLL clock will be driven on the analog high speed
 * clock 0: PLL 0 div 1
 * clock 1: PLL 1 div 2
 */
#define CLK_PLL_CONFIG			0X30
#define CLK_PLL_MASK			0x33

#define CMN_READY			BIT(0)

#define DP_PLL_CLOCK_ENABLE		BIT(2)
#define DP_PLL_ENABLE			BIT(0)
#define DP_PLL_DATA_RATE_RBR		((2 << 12) | (4 << 8))
#define DP_PLL_DATA_RATE_HBR		((2 << 12) | (4 << 8))
#define DP_PLL_DATA_RATE_HBR2		((1 << 12) | (2 << 8))

#define DP_MODE_A0			BIT(4)
#define DP_MODE_A2			BIT(6)
#define DP_MODE_ENTER_A0		0xc101
#define DP_MODE_ENTER_A2		0xc104

#define PHY_MODE_SET_TIMEOUT		100000

#define PIN_ASSIGN_C_E			0x51d9
#define PIN_ASSIGN_D_F			0x5100

#define MODE_DISCONNECT			0
#define MODE_UFP_USB			BIT(0)
#define MODE_DFP_USB			BIT(1)
#define MODE_DFP_DP			BIT(2)

struct usb3phy_reg {
	u32 offset;
	u32 enable_bit;
	u32 write_enable;
};

/**
 * struct rockchip_usb3phy_port_cfg: usb3-phy port configuration.
 * @reg: the base address for usb3-phy config.
 * @typec_conn_dir: the register of type-c connector direction.
 * @usb3tousb2_en: the register of type-c force usb2 to usb2 enable.
 * @external_psm: the register of type-c phy external psm clock.
 * @pipe_status: the register of type-c phy pipe status.
 * @usb3_host_disable: the register of type-c usb3 host disable.
 * @usb3_host_port: the register of type-c usb3 host port.
 * @uphy_dp_sel: the register of type-c phy DP select control.
 */
struct rockchip_usb3phy_port_cfg {
	unsigned int reg;
	struct usb3phy_reg typec_conn_dir;
	struct usb3phy_reg usb3tousb2_en;
	struct usb3phy_reg external_psm;
	struct usb3phy_reg pipe_status;
	struct usb3phy_reg usb3_host_disable;
	struct usb3phy_reg usb3_host_port;
	struct usb3phy_reg uphy_dp_sel;
};

struct rockchip_tcphy {
	void __iomem *reg_base;
	void __iomem *grf_base;
	struct clk clk_core;
	struct clk clk_ref;
	struct reset_ctl uphy_rst;
	struct reset_ctl pipe_rst;
	struct reset_ctl tcphy_rst;
	const struct rockchip_usb3phy_port_cfg *port_cfgs;
	u8 mode;
};

struct phy_reg {
	u16 value;
	u32 addr;
};

static struct phy_reg usb3_pll_cfg[] = {
	{ 0xf0,		CMN_PLL0_VCOCAL_INIT },
	{ 0x18,		CMN_PLL0_VCOCAL_ITER },
	{ 0xd0,		CMN_PLL0_INTDIV },
	{ 0x4a4a,	CMN_PLL0_FRACDIV },
	{ 0x34,		CMN_PLL0_HIGH_THR },
	{ 0x1ee,	CMN_PLL0_SS_CTRL1 },
	{ 0x7f03,	CMN_PLL0_SS_CTRL2 },
	{ 0x20,		CMN_PLL0_DSM_DIAG },
	{ 0,		CMN_DIAG_PLL0_OVRD },
	{ 0,		CMN_DIAG_PLL0_FBH_OVRD },
	{ 0,		CMN_DIAG_PLL0_FBL_OVRD },
	{ 0x7,		CMN_DIAG_PLL0_V2I_TUNE },
	{ 0x45,		CMN_DIAG_PLL0_CP_TUNE },
	{ 0x8,		CMN_DIAG_PLL0_LF_PROG },
};

static inline int property_enable(struct rockchip_tcphy *priv,
				  const struct usb3phy_reg *reg, bool en)
{
	u32 mask = 1 << reg->write_enable;
	u32 val = en << reg->enable_bit;

	return writel(val | mask, priv->grf_base + reg->offset);
}

static int rockchip_tcphy_get_mode(struct rockchip_tcphy *priv)
{
	/* TODO: Add proper logic to find DP or USB3 mode */
	return MODE_DFP_USB | MODE_UFP_USB;
}

static void rockchip_tcphy_cfg_24m(struct rockchip_tcphy *priv)
{
	u32 i, rdata;

	/*
	 * cmn_ref_clk_sel = 3, select the 24Mhz for clk parent
	 * cmn_psm_clk_dig_div = 2, set the clk division to 2
	 */
	writel(0x830, priv->reg_base + PMA_CMN_CTRL1);
	for (i = 0; i < 4; i++) {
		/*
		 * The following PHY configuration assumes a 24 MHz reference
		 * clock.
		 */
		writel(0x90, priv->reg_base + XCVR_DIAG_LANE_FCM_EN_MGN(i));
		writel(0x960, priv->reg_base + TX_RCVDET_EN_TMR(i));
		writel(0x30, priv->reg_base + TX_RCVDET_ST_TMR(i));
	}

	rdata = readl(priv->reg_base + CMN_DIAG_HSCLK_SEL);
	rdata &= ~CLK_PLL_MASK;
	rdata |= CLK_PLL_CONFIG;
	writel(rdata, priv->reg_base + CMN_DIAG_HSCLK_SEL);
}

static void rockchip_tcphy_cfg_usb3_pll(struct rockchip_tcphy *priv)
{
	u32 i;

	/* load the configuration of PLL0 */
	for (i = 0; i < ARRAY_SIZE(usb3_pll_cfg); i++)
		writel(usb3_pll_cfg[i].value,
		       priv->reg_base + usb3_pll_cfg[i].addr);
}

static void rockchip_tcphy_tx_usb3_cfg_lane(struct rockchip_tcphy *priv,
					    u32 lane)
{
	writel(0x7799, priv->reg_base + TX_PSC_A0(lane));
	writel(0x7798, priv->reg_base + TX_PSC_A1(lane));
	writel(0x5098, priv->reg_base + TX_PSC_A2(lane));
	writel(0x5098, priv->reg_base + TX_PSC_A3(lane));
	writel(0, priv->reg_base + TX_TXCC_MGNFS_MULT_000(lane));
	writel(0xbf, priv->reg_base + XCVR_DIAG_BIDI_CTRL(lane));
}

static void rockchip_tcphy_rx_usb3_cfg_lane(struct rockchip_tcphy *priv,
					    u32 lane)
{
	writel(0xa6fd, priv->reg_base + RX_PSC_A0(lane));
	writel(0xa6fd, priv->reg_base + RX_PSC_A1(lane));
	writel(0xa410, priv->reg_base + RX_PSC_A2(lane));
	writel(0x2410, priv->reg_base + RX_PSC_A3(lane));
	writel(0x23ff, priv->reg_base + RX_PSC_CAL(lane));
	writel(0x13, priv->reg_base + RX_SIGDET_HL_FILT_TMR(lane));
	writel(0x03e7, priv->reg_base + RX_REE_CTRL_DATA_MASK(lane));
	writel(0x1004, priv->reg_base + RX_DIAG_SIGDET_TUNE(lane));
	writel(0x2010, priv->reg_base + RX_PSC_RDY(lane));
	writel(0xfb, priv->reg_base + XCVR_DIAG_BIDI_CTRL(lane));
}

static int rockchip_tcphy_init(struct phy *phy, struct rockchip_tcphy *priv)
{
	const struct rockchip_usb3phy_port_cfg *cfg = priv->port_cfgs;
	u32 val;
	int ret;

	ret = clk_enable(&priv->clk_core);
	if (ret) {
		dev_err(phy->dev, "failed to enable core clk (ret=%d)\n", ret);
		return ret;
	}

	ret = clk_enable(&priv->clk_ref);
	if (ret) {
		dev_err(phy->dev, "failed to enable ref clk (ret=%d)\n", ret);
		goto err_clk_core;
	}

	ret = reset_deassert(&priv->tcphy_rst);
	if (ret) {
		dev_err(phy->dev, "failed to deassert uphy-tcphy reset (ret=%d)\n",
			ret);
		goto err_clk_ref;
	}

	property_enable(priv, &cfg->typec_conn_dir, 0);

	rockchip_tcphy_cfg_24m(priv);

	rockchip_tcphy_cfg_usb3_pll(priv);

	rockchip_tcphy_tx_usb3_cfg_lane(priv, 0);
	rockchip_tcphy_rx_usb3_cfg_lane(priv, 1);

	ret = reset_deassert(&priv->uphy_rst);
	if (ret) {
		dev_err(phy->dev, "failed to deassert uphy rst (ret=%d)\n",
			ret);
		goto err_tcphy_rst;
	}

	ret = readl_poll_sleep_timeout(priv->reg_base + PMA_CMN_CTRL1,
				       val, val & CMN_READY, 10,
				       PHY_MODE_SET_TIMEOUT);
	if (ret < 0) {
		dev_err(phy->dev, "PMA Timeout!\n");
		ret = -ETIMEDOUT;
		goto err_uphy_rst;
	}

	ret = reset_deassert(&priv->pipe_rst);
	if (ret) {
		dev_err(phy->dev, "failed to deassert pipe rst (ret=%d)\n",
			ret);
		goto err_uphy_rst;
	}

	return 0;

err_uphy_rst:
	reset_assert(&priv->uphy_rst);
err_tcphy_rst:
	reset_assert(&priv->tcphy_rst);
err_clk_ref:
	clk_disable(&priv->clk_ref);
err_clk_core:
	clk_disable(&priv->clk_core);
	return ret;
}

static void rockchip_tcphy_exit(struct rockchip_tcphy *priv)
{
	reset_assert(&priv->tcphy_rst);
	reset_assert(&priv->uphy_rst);
	reset_assert(&priv->pipe_rst);
	clk_disable(&priv->clk_core);
	clk_disable(&priv->clk_ref);
}

static int tcphy_cfg_usb3_to_usb2_only(struct rockchip_tcphy *priv,
				       bool value)
{
	const struct rockchip_usb3phy_port_cfg *cfg = priv->port_cfgs;

	property_enable(priv, &cfg->usb3tousb2_en, value);
	property_enable(priv, &cfg->usb3_host_disable, value);
	property_enable(priv, &cfg->usb3_host_port, !value);

	return 0;
}

static int rockchip_usb3_phy_power_on(struct phy *phy)
{
	struct udevice *parent = dev_get_parent(phy->dev);
	struct rockchip_tcphy *priv = dev_get_priv(parent);
	const struct rockchip_usb3phy_port_cfg *cfg = priv->port_cfgs;
	const struct usb3phy_reg *reg = &cfg->pipe_status;
	int timeout, new_mode;
	u32 val;
	int ret;

	new_mode = rockchip_tcphy_get_mode(priv);
	if (new_mode < 0) {
		dev_err(phy->dev, "invalid mode %d\n", new_mode);
		return new_mode;
	}

	if (priv->mode == new_mode)
		return 0;

	if (priv->mode == MODE_DISCONNECT) {
		ret = rockchip_tcphy_init(phy, priv);
		if (ret) {
			dev_err(phy->dev, "failed to init tcphy (ret=%d)\n", ret);
			return ret;
		}
	}

	/* wait TCPHY for pipe ready */
	for (timeout = 0; timeout < 100; timeout++) {
		val = readl(priv->grf_base + reg->offset);
		if (!(val & BIT(reg->enable_bit))) {
			priv->mode |= new_mode & (MODE_DFP_USB | MODE_UFP_USB);

			/* enable usb3 host */
			tcphy_cfg_usb3_to_usb2_only(priv, false);
			return 0;
		}
		usleep_range(10, 20);
	}

	if (priv->mode == MODE_DISCONNECT)
		rockchip_tcphy_exit(priv);

	return -ETIMEDOUT;
}

static int rockchip_usb3_phy_power_off(struct phy *phy)
{
	struct udevice *parent = dev_get_parent(phy->dev);
	struct rockchip_tcphy *priv = dev_get_priv(parent);

	tcphy_cfg_usb3_to_usb2_only(priv, false);

	if (priv->mode == MODE_DISCONNECT)
		goto exit;

	priv->mode &= ~(MODE_UFP_USB | MODE_DFP_USB);
	if (priv->mode == MODE_DISCONNECT)
		rockchip_tcphy_exit(priv);

exit:
	return 0;
}

static struct phy_ops rockchip_tcphy_usb3_ops = {
	.power_on = rockchip_usb3_phy_power_on,
	.power_off = rockchip_usb3_phy_power_off,
};

static void rockchip_tcphy_pre_init(struct udevice *dev)
{
	struct rockchip_tcphy *priv = dev_get_priv(dev);
	const struct rockchip_usb3phy_port_cfg *cfg = priv->port_cfgs;

	reset_assert(&priv->tcphy_rst);
	reset_assert(&priv->uphy_rst);
	reset_assert(&priv->pipe_rst);

	/* select external psm clock */
	property_enable(priv, &cfg->external_psm, 1);
	property_enable(priv, &cfg->usb3tousb2_en, 0);

	priv->mode = MODE_DISCONNECT;
}

static int rockchip_tcphy_parse_dt(struct udevice *dev)
{
	struct rockchip_tcphy *priv = dev_get_priv(dev);
	int ret;

	priv->grf_base = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(priv->grf_base))
		return PTR_ERR(priv->grf_base);

	ret = clk_get_by_name(dev, "tcpdcore", &priv->clk_core);
	if (ret) {
		dev_err(dev, "failed to get tcpdcore clk (ret=%d)\n", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "tcpdphy-ref", &priv->clk_ref);
	if (ret) {
		dev_err(dev, "failed to get tcpdphy-ref clk (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "uphy", &priv->uphy_rst);
	if (ret) {
		dev_err(dev, "failed to get uphy reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "uphy-pipe", &priv->pipe_rst);
	if (ret) {
		dev_err(dev, "failed to get uphy-pipe reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "uphy-tcphy", &priv->tcphy_rst);
	if (ret) {
		dev_err(dev, "failed to get uphy-tcphy reset (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}

static int rockchip_tcphy_probe(struct udevice *dev)
{
	struct rockchip_tcphy *priv = dev_get_priv(dev);
	const struct rockchip_usb3phy_port_cfg *phy_cfgs;
	unsigned int reg;
	int index, ret;

	priv->reg_base = dev_read_addr_ptr(dev);
	if (!priv->reg_base)
		return -EINVAL;

	ret = dev_read_u32_index(dev, "reg", 1, &reg);
	if (ret) {
		dev_err(dev, "failed to read reg property (ret = %d)\n", ret);
		return ret;
	}

	phy_cfgs = (const struct rockchip_usb3phy_port_cfg *)
					dev_get_driver_data(dev);
	if (!phy_cfgs)
		return -EINVAL;

	/* find out a proper config which can be matched with dt. */
	index = 0;
	while (phy_cfgs[index].reg) {
		if (phy_cfgs[index].reg == reg) {
			priv->port_cfgs = &phy_cfgs[index];
			break;
		}

		++index;
	}

	if (!priv->port_cfgs) {
		dev_err(dev, "failed find proper phy-cfg\n");
		return -EINVAL;
	}

	ret = rockchip_tcphy_parse_dt(dev);
	if (ret)
		return ret;

	rockchip_tcphy_pre_init(dev);

	return 0;
}

static int rockchip_tcphy_bind(struct udevice *dev)
{
	struct udevice *tcphy_dev;
	ofnode node;
	const char *name;
	int ret = 0;

	dev_for_each_subnode(node, dev) {
		if (!ofnode_valid(node)) {
			dev_info(dev, "subnode %s not found\n", dev->name);
			return -ENXIO;
		}

		name = ofnode_get_name(node);
		dev_dbg(dev, "subnode %s\n", name);

		if (!strcasecmp(name, "dp-port")) {
			dev_dbg(dev, "Warning: dp-port not supported yet!\n");
			continue;
		} else if (!strcasecmp(name, "usb3-port")) {
			ret = device_bind_driver_to_node(dev,
							 "rockchip_tcphy_usb3_port",
							 name, node, &tcphy_dev);
			if (ret) {
				dev_err(dev,
					"'%s' cannot bind 'rockchip_tcphy_usb3_port'\n",
					name);
				return ret;
			}
		}
	}

	return ret;
}

static const struct rockchip_usb3phy_port_cfg rk3399_typec_phy_cfgs[] = {
	{
		.reg			= 0xff7c0000,
		.typec_conn_dir		= { 0xe580, 0, 16 },
		.usb3tousb2_en		= { 0xe580, 3, 19 },
		.external_psm		= { 0xe588, 14, 30 },
		.pipe_status		= { 0xe5c0, 0, 0 },
		.usb3_host_disable	= { 0x2434, 0, 16 },
		.usb3_host_port		= { 0x2434, 12, 28 },
		.uphy_dp_sel		= { 0x6268, 19, 19 },
	},
	{
		.reg			= 0xff800000,
		.typec_conn_dir		= { 0xe58c, 0, 16 },
		.usb3tousb2_en		= { 0xe58c, 3, 19 },
		.external_psm		= { 0xe594, 14, 30 },
		.pipe_status		= { 0xe5c0, 16, 16 },
		.usb3_host_disable	= { 0x2444, 0, 16 },
		.usb3_host_port		= { 0x2444, 12, 28 },
		.uphy_dp_sel		= { 0x6268, 3, 19 },
	},
	{ /* sentinel */ }
};

static const struct udevice_id rockchip_typec_phy_ids[] = {
	{
		.compatible = "rockchip,rk3399-typec-phy",
		.data = (ulong)&rk3399_typec_phy_cfgs,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(rockchip_tcphy_usb3_port) = {
	.name		= "rockchip_tcphy_usb3_port",
	.id		= UCLASS_PHY,
	.ops		= &rockchip_tcphy_usb3_ops,
};

U_BOOT_DRIVER(rockchip_typec_phy) = {
	.name	= "rockchip_typec_phy",
	.id	= UCLASS_PHY,
	.of_match = rockchip_typec_phy_ids,
	.probe = rockchip_tcphy_probe,
	.bind = rockchip_tcphy_bind,
	.priv_auto	= sizeof(struct rockchip_tcphy),
};
