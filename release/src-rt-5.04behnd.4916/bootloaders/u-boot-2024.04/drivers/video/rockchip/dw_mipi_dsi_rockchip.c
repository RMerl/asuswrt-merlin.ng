// SPDX-License-Identifier: GPL-2.0+
/*
 * Author(s): Chris Morgan <macromorgan@hotmail.com>
 *
 * This MIPI DSI controller driver is heavily based on the Linux Kernel
 * driver from drivers/gpu/drm/rockchip/dw-mipi-dsi-rockchip.c and the
 * U-Boot driver from drivers/video/stm32/stm32_dsi.c.
 */

#define LOG_CATEGORY UCLASS_VIDEO_BRIDGE

#include <clk.h>
#include <dm.h>
#include <div64.h>
#include <dsi_host.h>
#include <generic-phy.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <phy-mipi-dphy.h>
#include <reset.h>
#include <syscon.h>
#include <video_bridge.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <linux/iopoll.h>

#include <common.h>
#include <log.h>
#include <video.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <linux/bitops.h>
#include <linux/time.h>

#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>

/*
 * DSI wrapper registers & bit definitions
 * Note: registers are named as in the Reference Manual
 */
#define DSI_WCR		0x0404		/* Wrapper Control Reg */
#define WCR_DSIEN	BIT(3)		/* DSI ENable */

#define DSI_PHY_TST_CTRL0		0xb4
#define PHY_TESTCLK			BIT(1)
#define PHY_UNTESTCLK			0
#define PHY_TESTCLR			BIT(0)
#define PHY_UNTESTCLR			0

#define DSI_PHY_TST_CTRL1		0xb8
#define PHY_TESTEN			BIT(16)
#define PHY_UNTESTEN			0
#define PHY_TESTDOUT(n)			(((n) & 0xff) << 8)
#define PHY_TESTDIN(n)			(((n) & 0xff) << 0)

#define BYPASS_VCO_RANGE	BIT(7)
#define VCO_RANGE_CON_SEL(val)	(((val) & 0x7) << 3)
#define VCO_IN_CAP_CON_DEFAULT	(0x0 << 1)
#define VCO_IN_CAP_CON_LOW	(0x1 << 1)
#define VCO_IN_CAP_CON_HIGH	(0x2 << 1)
#define REF_BIAS_CUR_SEL	BIT(0)

#define CP_CURRENT_3UA	0x1
#define CP_CURRENT_4_5UA	0x2
#define CP_CURRENT_7_5UA	0x6
#define CP_CURRENT_6UA	0x9
#define CP_CURRENT_12UA	0xb
#define CP_CURRENT_SEL(val)	((val) & 0xf)
#define CP_PROGRAM_EN		BIT(7)

#define LPF_RESISTORS_15_5KOHM	0x1
#define LPF_RESISTORS_13KOHM	0x2
#define LPF_RESISTORS_11_5KOHM	0x4
#define LPF_RESISTORS_10_5KOHM	0x8
#define LPF_RESISTORS_8KOHM	0x10
#define LPF_PROGRAM_EN		BIT(6)
#define LPF_RESISTORS_SEL(val)	((val) & 0x3f)

#define HSFREQRANGE_SEL(val)	(((val) & 0x3f) << 1)

#define INPUT_DIVIDER(val)	(((val) - 1) & 0x7f)
#define LOW_PROGRAM_EN		0
#define HIGH_PROGRAM_EN		BIT(7)
#define LOOP_DIV_LOW_SEL(val)	(((val) - 1) & 0x1f)
#define LOOP_DIV_HIGH_SEL(val)	((((val) - 1) >> 5) & 0xf)
#define PLL_LOOP_DIV_EN		BIT(5)
#define PLL_INPUT_DIV_EN	BIT(4)

#define POWER_CONTROL		BIT(6)
#define INTERNAL_REG_CURRENT	BIT(3)
#define BIAS_BLOCK_ON		BIT(2)
#define BANDGAP_ON		BIT(0)

#define TER_RESISTOR_HIGH	BIT(7)
#define	TER_RESISTOR_LOW	0
#define LEVEL_SHIFTERS_ON	BIT(6)
#define TER_CAL_DONE		BIT(5)
#define SETRD_MAX		(0x7 << 2)
#define POWER_MANAGE		BIT(1)
#define TER_RESISTORS_ON	BIT(0)

#define BIASEXTR_SEL(val)	((val) & 0x7)
#define BANDGAP_SEL(val)	((val) & 0x7)
#define TLP_PROGRAM_EN		BIT(7)
#define THS_PRE_PROGRAM_EN	BIT(7)
#define THS_ZERO_PROGRAM_EN	BIT(6)

#define PLL_BIAS_CUR_SEL_CAP_VCO_CONTROL		0x10
#define PLL_CP_CONTROL_PLL_LOCK_BYPASS			0x11
#define PLL_LPF_AND_CP_CONTROL				0x12
#define PLL_INPUT_DIVIDER_RATIO				0x17
#define PLL_LOOP_DIVIDER_RATIO				0x18
#define PLL_INPUT_AND_LOOP_DIVIDER_RATIOS_CONTROL	0x19
#define BANDGAP_AND_BIAS_CONTROL			0x20
#define TERMINATION_RESISTER_CONTROL			0x21
#define AFE_BIAS_BANDGAP_ANALOG_PROGRAMMABILITY		0x22
#define HS_RX_CONTROL_OF_LANE_CLK			0x34
#define HS_RX_CONTROL_OF_LANE_0				0x44
#define HS_RX_CONTROL_OF_LANE_1				0x54
#define HS_TX_CLOCK_LANE_REQUEST_STATE_TIME_CONTROL	0x60
#define HS_TX_CLOCK_LANE_PREPARE_STATE_TIME_CONTROL	0x61
#define HS_TX_CLOCK_LANE_HS_ZERO_STATE_TIME_CONTROL	0x62
#define HS_TX_CLOCK_LANE_TRAIL_STATE_TIME_CONTROL	0x63
#define HS_TX_CLOCK_LANE_EXIT_STATE_TIME_CONTROL	0x64
#define HS_TX_CLOCK_LANE_POST_TIME_CONTROL		0x65
#define HS_TX_DATA_LANE_REQUEST_STATE_TIME_CONTROL	0x70
#define HS_TX_DATA_LANE_PREPARE_STATE_TIME_CONTROL	0x71
#define HS_TX_DATA_LANE_HS_ZERO_STATE_TIME_CONTROL	0x72
#define HS_TX_DATA_LANE_TRAIL_STATE_TIME_CONTROL	0x73
#define HS_TX_DATA_LANE_EXIT_STATE_TIME_CONTROL		0x74
#define HS_RX_DATA_LANE_THS_SETTLE_CONTROL		0x75
#define HS_RX_CONTROL_OF_LANE_2				0x84
#define HS_RX_CONTROL_OF_LANE_3				0x94

#define DW_MIPI_NEEDS_PHY_CFG_CLK	BIT(0)
#define DW_MIPI_NEEDS_GRF_CLK		BIT(1)

#define RK3399_GRF_SOC_CON20		0x6250
#define RK3399_DSI0_LCDC_SEL		BIT(0)
#define RK3399_DSI1_LCDC_SEL		BIT(4)

#define RK3399_GRF_SOC_CON22		0x6258
#define RK3399_DSI0_TURNREQUEST		(0xf << 12)
#define RK3399_DSI0_TURNDISABLE		(0xf << 8)
#define RK3399_DSI0_FORCETXSTOPMODE	(0xf << 4)
#define RK3399_DSI0_FORCERXMODE		(0xf << 0)

#define RK3399_GRF_SOC_CON23		0x625c
#define RK3399_DSI1_TURNDISABLE		(0xf << 12)
#define RK3399_DSI1_FORCETXSTOPMODE	(0xf << 8)
#define RK3399_DSI1_FORCERXMODE		(0xf << 4)
#define RK3399_DSI1_ENABLE		(0xf << 0)

#define RK3399_GRF_SOC_CON24		0x6260
#define RK3399_TXRX_MASTERSLAVEZ	BIT(7)
#define RK3399_TXRX_ENABLECLK		BIT(6)
#define RK3399_TXRX_BASEDIR		BIT(5)
#define RK3399_TXRX_SRC_SEL_ISP0	BIT(4)
#define RK3399_TXRX_TURNREQUEST		GENMASK(3, 0)

#define RK3568_GRF_VO_CON2		0x0368
#define RK3568_DSI0_SKEWCALHS		(0x1f << 11)
#define RK3568_DSI0_FORCETXSTOPMODE	(0xf << 4)
#define RK3568_DSI0_TURNDISABLE		BIT(2)
#define RK3568_DSI0_FORCERXMODE		BIT(0)

/*
 * Note these registers do not appear in the datasheet, they are
 * however present in the BSP driver which is where these values
 * come from. Name GRF_VO_CON3 is assumed.
 */
#define RK3568_GRF_VO_CON3		0x36c
#define RK3568_DSI1_SKEWCALHS		(0x1f << 11)
#define RK3568_DSI1_FORCETXSTOPMODE	(0xf << 4)
#define RK3568_DSI1_TURNDISABLE		BIT(2)
#define RK3568_DSI1_FORCERXMODE		BIT(0)

#define HIWORD_UPDATE(val, mask)	(val | (mask) << 16)

/* Timeout for regulator on/off, pll lock/unlock & fifo empty */
#define TIMEOUT_US	200000

enum {
	BANDGAP_97_07,
	BANDGAP_98_05,
	BANDGAP_99_02,
	BANDGAP_100_00,
	BANDGAP_93_17,
	BANDGAP_94_15,
	BANDGAP_95_12,
	BANDGAP_96_10,
};

enum {
	BIASEXTR_87_1,
	BIASEXTR_91_5,
	BIASEXTR_95_9,
	BIASEXTR_100,
	BIASEXTR_105_94,
	BIASEXTR_111_88,
	BIASEXTR_118_8,
	BIASEXTR_127_7,
};

struct rockchip_dw_dsi_chip_data {
	u32 reg;

	u32 lcdsel_grf_reg;
	u32 lcdsel_big;
	u32 lcdsel_lit;

	u32 enable_grf_reg;
	u32 enable;

	u32 lanecfg1_grf_reg;
	u32 lanecfg1;
	u32 lanecfg2_grf_reg;
	u32 lanecfg2;

	unsigned int flags;
	unsigned int max_data_lanes;
};

struct dw_rockchip_dsi_priv {
	struct mipi_dsi_device device;
	void __iomem *base;
	struct udevice *panel;
	void __iomem *grf;

	/* Optional external dphy */
	struct phy phy;
	struct phy_configure_opts_mipi_dphy phy_opts;

	struct clk *pclk;
	struct clk *ref;
	struct clk *grf_clk;
	struct clk *phy_cfg_clk;
	struct reset_ctl *rst;
	unsigned int lane_mbps; /* per lane */
	u16 input_div;
	u16 feedback_div;
	const struct rockchip_dw_dsi_chip_data *cdata;
	struct udevice *dsi_host;
};

static inline void dsi_write(struct dw_rockchip_dsi_priv *dsi, u32 reg, u32 val)
{
	writel(val, dsi->base + reg);
}

static inline u32 dsi_read(struct dw_rockchip_dsi_priv *dsi, u32 reg)
{
	return readl(dsi->base + reg);
}

static inline void dsi_set(struct dw_rockchip_dsi_priv *dsi, u32 reg, u32 mask)
{
	dsi_write(dsi, reg, dsi_read(dsi, reg) | mask);
}

static inline void dsi_clear(struct dw_rockchip_dsi_priv *dsi, u32 reg, u32 mask)
{
	dsi_write(dsi, reg, dsi_read(dsi, reg) & ~mask);
}

static inline void dsi_update_bits(struct dw_rockchip_dsi_priv *dsi, u32 reg,
				   u32 mask, u32 val)
{
	dsi_write(dsi, reg, (dsi_read(dsi, reg) & ~mask) | val);
}

static void dw_mipi_dsi_phy_write(struct dw_rockchip_dsi_priv *dsi,
				  u8 test_code,
				  u8 test_data)
{
	/*
	 * With the falling edge on TESTCLK, the TESTDIN[7:0] signal content
	 * is latched internally as the current test code. Test data is
	 * programmed internally by rising edge on TESTCLK.
	 */
	dsi_write(dsi, DSI_PHY_TST_CTRL0, PHY_TESTCLK | PHY_UNTESTCLR);

	dsi_write(dsi, DSI_PHY_TST_CTRL1, PHY_TESTEN | PHY_TESTDOUT(0) |
					  PHY_TESTDIN(test_code));

	dsi_write(dsi, DSI_PHY_TST_CTRL0, PHY_UNTESTCLK | PHY_UNTESTCLR);

	dsi_write(dsi, DSI_PHY_TST_CTRL1, PHY_UNTESTEN | PHY_TESTDOUT(0) |
					  PHY_TESTDIN(test_data));

	dsi_write(dsi, DSI_PHY_TST_CTRL0, PHY_TESTCLK | PHY_UNTESTCLR);
}

struct dphy_pll_parameter_map {
	unsigned int max_mbps;
	u8 hsfreqrange;
	u8 icpctrl;
	u8 lpfctrl;
};

/* The table is based on 27MHz DPHY pll reference clock. */
static const struct dphy_pll_parameter_map dppa_map[] = {
	{  89, 0x00, CP_CURRENT_3UA, LPF_RESISTORS_13KOHM },
	{  99, 0x10, CP_CURRENT_3UA, LPF_RESISTORS_13KOHM },
	{ 109, 0x20, CP_CURRENT_3UA, LPF_RESISTORS_13KOHM },
	{ 129, 0x01, CP_CURRENT_3UA, LPF_RESISTORS_15_5KOHM },
	{ 139, 0x11, CP_CURRENT_3UA, LPF_RESISTORS_15_5KOHM },
	{ 149, 0x21, CP_CURRENT_3UA, LPF_RESISTORS_15_5KOHM },
	{ 169, 0x02, CP_CURRENT_6UA, LPF_RESISTORS_13KOHM },
	{ 179, 0x12, CP_CURRENT_6UA, LPF_RESISTORS_13KOHM },
	{ 199, 0x22, CP_CURRENT_6UA, LPF_RESISTORS_13KOHM },
	{ 219, 0x03, CP_CURRENT_4_5UA, LPF_RESISTORS_13KOHM },
	{ 239, 0x13, CP_CURRENT_4_5UA, LPF_RESISTORS_13KOHM },
	{ 249, 0x23, CP_CURRENT_4_5UA, LPF_RESISTORS_13KOHM },
	{ 269, 0x04, CP_CURRENT_6UA, LPF_RESISTORS_11_5KOHM },
	{ 299, 0x14, CP_CURRENT_6UA, LPF_RESISTORS_11_5KOHM },
	{ 329, 0x05, CP_CURRENT_3UA, LPF_RESISTORS_15_5KOHM },
	{ 359, 0x15, CP_CURRENT_3UA, LPF_RESISTORS_15_5KOHM },
	{ 399, 0x25, CP_CURRENT_3UA, LPF_RESISTORS_15_5KOHM },
	{ 449, 0x06, CP_CURRENT_7_5UA, LPF_RESISTORS_11_5KOHM },
	{ 499, 0x16, CP_CURRENT_7_5UA, LPF_RESISTORS_11_5KOHM },
	{ 549, 0x07, CP_CURRENT_7_5UA, LPF_RESISTORS_10_5KOHM },
	{ 599, 0x17, CP_CURRENT_7_5UA, LPF_RESISTORS_10_5KOHM },
	{ 649, 0x08, CP_CURRENT_7_5UA, LPF_RESISTORS_11_5KOHM },
	{ 699, 0x18, CP_CURRENT_7_5UA, LPF_RESISTORS_11_5KOHM },
	{ 749, 0x09, CP_CURRENT_7_5UA, LPF_RESISTORS_11_5KOHM },
	{ 799, 0x19, CP_CURRENT_7_5UA, LPF_RESISTORS_11_5KOHM },
	{ 849, 0x29, CP_CURRENT_7_5UA, LPF_RESISTORS_11_5KOHM },
	{ 899, 0x39, CP_CURRENT_7_5UA, LPF_RESISTORS_11_5KOHM },
	{ 949, 0x0a, CP_CURRENT_12UA, LPF_RESISTORS_8KOHM },
	{ 999, 0x1a, CP_CURRENT_12UA, LPF_RESISTORS_8KOHM },
	{1049, 0x2a, CP_CURRENT_12UA, LPF_RESISTORS_8KOHM },
	{1099, 0x3a, CP_CURRENT_12UA, LPF_RESISTORS_8KOHM },
	{1149, 0x0b, CP_CURRENT_12UA, LPF_RESISTORS_10_5KOHM },
	{1199, 0x1b, CP_CURRENT_12UA, LPF_RESISTORS_10_5KOHM },
	{1249, 0x2b, CP_CURRENT_12UA, LPF_RESISTORS_10_5KOHM },
	{1299, 0x3b, CP_CURRENT_12UA, LPF_RESISTORS_10_5KOHM },
	{1349, 0x0c, CP_CURRENT_12UA, LPF_RESISTORS_10_5KOHM },
	{1399, 0x1c, CP_CURRENT_12UA, LPF_RESISTORS_10_5KOHM },
	{1449, 0x2c, CP_CURRENT_12UA, LPF_RESISTORS_10_5KOHM },
	{1500, 0x3c, CP_CURRENT_12UA, LPF_RESISTORS_10_5KOHM }
};

static int max_mbps_to_parameter(unsigned int max_mbps)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(dppa_map); i++)
		if (dppa_map[i].max_mbps >= max_mbps)
			return i;

	return -EINVAL;
}

/*
 * ns2bc - Nanoseconds to byte clock cycles
 */
static inline unsigned int ns2bc(struct dw_rockchip_dsi_priv *dsi, int ns)
{
	return DIV_ROUND_UP(ns * dsi->lane_mbps / 8, 1000);
}

/*
 * ns2ui - Nanoseconds to UI time periods
 */
static inline unsigned int ns2ui(struct dw_rockchip_dsi_priv *dsi, int ns)
{
	return DIV_ROUND_UP(ns * dsi->lane_mbps, 1000);
}

static int dsi_phy_init(void *priv_data)
{
	struct mipi_dsi_device *device = priv_data;
	struct udevice *dev = device->dev;
	struct dw_rockchip_dsi_priv *dsi = dev_get_priv(dev);
	int ret, i, vco;

	if (generic_phy_valid(&dsi->phy)) {
		ret = generic_phy_configure(&dsi->phy, &dsi->phy_opts);
		if (ret) {
			dev_err(dsi->dsi_host,
				"Configure external dphy fail %d\n",
				ret);
			return ret;
		}

		ret = generic_phy_power_on(&dsi->phy);
		if (ret) {
			dev_err(dsi->dsi_host,
				"Generic phy power on fail %d\n", ret);
			return ret;
		}

		return 0;
	}

	/*
	 * Get vco from frequency(lane_mbps)
	 * vco	frequency table
	 * 000 - between   80 and  200 MHz
	 * 001 - between  200 and  300 MHz
	 * 010 - between  300 and  500 MHz
	 * 011 - between  500 and  700 MHz
	 * 100 - between  700 and  900 MHz
	 * 101 - between  900 and 1100 MHz
	 * 110 - between 1100 and 1300 MHz
	 * 111 - between 1300 and 1500 MHz
	 */
	vco = (dsi->lane_mbps < 200) ? 0 : (dsi->lane_mbps + 100) / 200;

	i = max_mbps_to_parameter(dsi->lane_mbps);
	if (i < 0) {
		dev_err(dsi->dsi_host,
			"failed to get parameter for %dmbps clock\n",
			dsi->lane_mbps);
		return i;
	}

	dw_mipi_dsi_phy_write(dsi, PLL_BIAS_CUR_SEL_CAP_VCO_CONTROL,
			      BYPASS_VCO_RANGE |
			      VCO_RANGE_CON_SEL(vco) |
			      VCO_IN_CAP_CON_LOW |
			      REF_BIAS_CUR_SEL);

	dw_mipi_dsi_phy_write(dsi, PLL_CP_CONTROL_PLL_LOCK_BYPASS,
			      CP_CURRENT_SEL(dppa_map[i].icpctrl));
	dw_mipi_dsi_phy_write(dsi, PLL_LPF_AND_CP_CONTROL,
			      CP_PROGRAM_EN | LPF_PROGRAM_EN |
			      LPF_RESISTORS_SEL(dppa_map[i].lpfctrl));

	dw_mipi_dsi_phy_write(dsi, HS_RX_CONTROL_OF_LANE_0,
			      HSFREQRANGE_SEL(dppa_map[i].hsfreqrange));

	dw_mipi_dsi_phy_write(dsi, PLL_INPUT_DIVIDER_RATIO,
			      INPUT_DIVIDER(dsi->input_div));
	dw_mipi_dsi_phy_write(dsi, PLL_LOOP_DIVIDER_RATIO,
			      LOOP_DIV_LOW_SEL(dsi->feedback_div) |
			      LOW_PROGRAM_EN);
	/*
	 * We need set PLL_INPUT_AND_LOOP_DIVIDER_RATIOS_CONTROL immediately
	 * to make the configured LSB effective according to IP simulation
	 * and lab test results.
	 * Only in this way can we get correct mipi phy pll frequency.
	 */
	dw_mipi_dsi_phy_write(dsi, PLL_INPUT_AND_LOOP_DIVIDER_RATIOS_CONTROL,
			      PLL_LOOP_DIV_EN | PLL_INPUT_DIV_EN);
	dw_mipi_dsi_phy_write(dsi, PLL_LOOP_DIVIDER_RATIO,
			      LOOP_DIV_HIGH_SEL(dsi->feedback_div) |
			      HIGH_PROGRAM_EN);
	dw_mipi_dsi_phy_write(dsi, PLL_INPUT_AND_LOOP_DIVIDER_RATIOS_CONTROL,
			      PLL_LOOP_DIV_EN | PLL_INPUT_DIV_EN);

	dw_mipi_dsi_phy_write(dsi, AFE_BIAS_BANDGAP_ANALOG_PROGRAMMABILITY,
			      LOW_PROGRAM_EN | BIASEXTR_SEL(BIASEXTR_127_7));
	dw_mipi_dsi_phy_write(dsi, AFE_BIAS_BANDGAP_ANALOG_PROGRAMMABILITY,
			      HIGH_PROGRAM_EN | BANDGAP_SEL(BANDGAP_96_10));

	dw_mipi_dsi_phy_write(dsi, BANDGAP_AND_BIAS_CONTROL,
			      POWER_CONTROL | INTERNAL_REG_CURRENT |
			      BIAS_BLOCK_ON | BANDGAP_ON);

	dw_mipi_dsi_phy_write(dsi, TERMINATION_RESISTER_CONTROL,
			      TER_RESISTOR_LOW | TER_CAL_DONE |
			      SETRD_MAX | TER_RESISTORS_ON);
	dw_mipi_dsi_phy_write(dsi, TERMINATION_RESISTER_CONTROL,
			      TER_RESISTOR_HIGH | LEVEL_SHIFTERS_ON |
			      SETRD_MAX | POWER_MANAGE |
			      TER_RESISTORS_ON);

	dw_mipi_dsi_phy_write(dsi, HS_TX_CLOCK_LANE_REQUEST_STATE_TIME_CONTROL,
			      TLP_PROGRAM_EN | ns2bc(dsi, 500));
	dw_mipi_dsi_phy_write(dsi, HS_TX_CLOCK_LANE_PREPARE_STATE_TIME_CONTROL,
			      THS_PRE_PROGRAM_EN | ns2ui(dsi, 40));
	dw_mipi_dsi_phy_write(dsi, HS_TX_CLOCK_LANE_HS_ZERO_STATE_TIME_CONTROL,
			      THS_ZERO_PROGRAM_EN | ns2bc(dsi, 300));
	dw_mipi_dsi_phy_write(dsi, HS_TX_CLOCK_LANE_TRAIL_STATE_TIME_CONTROL,
			      THS_PRE_PROGRAM_EN | ns2ui(dsi, 100));
	dw_mipi_dsi_phy_write(dsi, HS_TX_CLOCK_LANE_EXIT_STATE_TIME_CONTROL,
			      BIT(5) | ns2bc(dsi, 100));
	dw_mipi_dsi_phy_write(dsi, HS_TX_CLOCK_LANE_POST_TIME_CONTROL,
			      BIT(5) | (ns2bc(dsi, 60) + 7));

	dw_mipi_dsi_phy_write(dsi, HS_TX_DATA_LANE_REQUEST_STATE_TIME_CONTROL,
			      TLP_PROGRAM_EN | ns2bc(dsi, 500));
	dw_mipi_dsi_phy_write(dsi, HS_TX_DATA_LANE_PREPARE_STATE_TIME_CONTROL,
			      THS_PRE_PROGRAM_EN | (ns2ui(dsi, 50) + 20));
	dw_mipi_dsi_phy_write(dsi, HS_TX_DATA_LANE_HS_ZERO_STATE_TIME_CONTROL,
			      THS_ZERO_PROGRAM_EN | (ns2bc(dsi, 140) + 2));
	dw_mipi_dsi_phy_write(dsi, HS_TX_DATA_LANE_TRAIL_STATE_TIME_CONTROL,
			      THS_PRE_PROGRAM_EN | (ns2ui(dsi, 60) + 8));
	dw_mipi_dsi_phy_write(dsi, HS_TX_DATA_LANE_EXIT_STATE_TIME_CONTROL,
			      BIT(5) | ns2bc(dsi, 100));

	return 0;
}

static void dsi_phy_post_set_mode(void *priv_data, unsigned long mode_flags)
{
	struct mipi_dsi_device *device = priv_data;
	struct udevice *dev = device->dev;
	struct dw_rockchip_dsi_priv *dsi = dev_get_priv(dev);

	dev_dbg(dev, "Set mode %p enable %ld\n", dsi,
		mode_flags & MIPI_DSI_MODE_VIDEO);

	if (!dsi)
		return;

	/*
	 * DSI wrapper must be enabled in video mode & disabled in command mode.
	 * If wrapper is enabled in command mode, the display controller
	 * register access will hang. Note that this was carried over from the
	 * stm32 dsi driver and is unknown if necessary for Rockchip.
	 */

	if (mode_flags & MIPI_DSI_MODE_VIDEO)
		dsi_set(dsi, DSI_WCR, WCR_DSIEN);
	else
		dsi_clear(dsi, DSI_WCR, WCR_DSIEN);
}

static int
dw_mipi_dsi_get_lane_mbps(void *priv_data, struct display_timing *timings,
			  u32 lanes, u32 format, unsigned int *lane_mbps)
{
	struct mipi_dsi_device *device = priv_data;
	struct udevice *dev = device->dev;
	struct dw_rockchip_dsi_priv *dsi = dev_get_priv(dev);
	int bpp;
	unsigned long mpclk, tmp;
	unsigned int target_mbps = 1000;
	unsigned int max_mbps = dppa_map[ARRAY_SIZE(dppa_map) - 1].max_mbps;
	unsigned long best_freq = 0;
	unsigned long fvco_min, fvco_max, fin, fout;
	unsigned int min_prediv, max_prediv;
	unsigned int _prediv, best_prediv;
	unsigned long _fbdiv, best_fbdiv;
	unsigned long min_delta = ULONG_MAX;

	bpp = mipi_dsi_pixel_format_to_bpp(format);
	if (bpp < 0) {
		dev_err(dsi->dsi_host,
			"failed to get bpp for pixel format %d\n",
			format);
		return bpp;
	}

	mpclk = DIV_ROUND_UP(timings->pixelclock.typ, 1000);
	if (mpclk) {
		/* take 1 / 0.8, since mbps must big than bandwidth of RGB */
		tmp = (mpclk * (bpp / lanes) * 10 / 8) / 1000;
		if (tmp < max_mbps)
			target_mbps = tmp;
		else
			dev_err(dsi->dsi_host,
				"DPHY clock frequency is out of range\n");
	}

	/* for external phy only the mipi_dphy_config is necessary */
	if (generic_phy_valid(&dsi->phy)) {
		phy_mipi_dphy_get_default_config(timings->pixelclock.typ  * 10 / 8,
						 bpp, lanes,
						 &dsi->phy_opts);
		dsi->lane_mbps = target_mbps;
		*lane_mbps = dsi->lane_mbps;

		return 0;
	}

	fin = clk_get_rate(dsi->ref);
	fout = target_mbps * USEC_PER_SEC;

	/* constraint: 5Mhz <= Fref / N <= 40MHz */
	min_prediv = DIV_ROUND_UP(fin, 40 * USEC_PER_SEC);
	max_prediv = fin / (5 * USEC_PER_SEC);

	/* constraint: 80MHz <= Fvco <= 1500Mhz */
	fvco_min = 80 * USEC_PER_SEC;
	fvco_max = 1500 * USEC_PER_SEC;

	for (_prediv = min_prediv; _prediv <= max_prediv; _prediv++) {
		u64 tmp;
		u32 delta;
		/* Fvco = Fref * M / N */
		tmp = (u64)fout * _prediv;
		do_div(tmp, fin);
		_fbdiv = tmp;
		/*
		 * Due to the use of a "by 2 pre-scaler," the range of the
		 * feedback multiplication value M is limited to even division
		 * numbers, and m must be greater than 6, not bigger than 512.
		 */
		if (_fbdiv < 6 || _fbdiv > 512)
			continue;

		_fbdiv += _fbdiv % 2;

		tmp = (u64)_fbdiv * fin;
		do_div(tmp, _prediv);
		if (tmp < fvco_min || tmp > fvco_max)
			continue;

		delta = abs(fout - tmp);
		if (delta < min_delta) {
			best_prediv = _prediv;
			best_fbdiv = _fbdiv;
			min_delta = delta;
			best_freq = tmp;
		}
	}

	if (best_freq) {
		dsi->lane_mbps = DIV_ROUND_UP(best_freq, USEC_PER_SEC);
		*lane_mbps = dsi->lane_mbps;
		dsi->input_div = best_prediv;
		dsi->feedback_div = best_fbdiv;
	} else {
		dev_err(dsi->dsi_host, "Can not find best_freq for DPHY\n");
		return -EINVAL;
	}

	return 0;
}

struct hstt {
	unsigned int maxfreq;
	struct mipi_dsi_phy_timing timing;
};

#define HSTT(_maxfreq, _c_lp2hs, _c_hs2lp, _d_lp2hs, _d_hs2lp)	\
{					\
	.maxfreq = _maxfreq,		\
	.timing = {			\
		.clk_lp2hs = _c_lp2hs,	\
		.clk_hs2lp = _c_hs2lp,	\
		.data_lp2hs = _d_lp2hs,	\
		.data_hs2lp = _d_hs2lp,	\
	}				\
}

/*
 * Table A-3 High-Speed Transition Times
 * (Note spacing is deliberate for readability).
 */
static struct hstt hstt_table[] = {
	HSTT(  90,  32, 20,  26, 13),
	HSTT( 100,  35, 23,  28, 14),
	HSTT( 110,  32, 22,  26, 13),
	HSTT( 130,  31, 20,  27, 13),
	HSTT( 140,  33, 22,  26, 14),
	HSTT( 150,  33, 21,  26, 14),
	HSTT( 170,  32, 20,  27, 13),
	HSTT( 180,  36, 23,  30, 15),
	HSTT( 200,  40, 22,  33, 15),
	HSTT( 220,  40, 22,  33, 15),
	HSTT( 240,  44, 24,  36, 16),
	HSTT( 250,  48, 24,  38, 17),
	HSTT( 270,  48, 24,  38, 17),
	HSTT( 300,  50, 27,  41, 18),
	HSTT( 330,  56, 28,  45, 18),
	HSTT( 360,  59, 28,  48, 19),
	HSTT( 400,  61, 30,  50, 20),
	HSTT( 450,  67, 31,  55, 21),
	HSTT( 500,  73, 31,  59, 22),
	HSTT( 550,  79, 36,  63, 24),
	HSTT( 600,  83, 37,  68, 25),
	HSTT( 650,  90, 38,  73, 27),
	HSTT( 700,  95, 40,  77, 28),
	HSTT( 750, 102, 40,  84, 28),
	HSTT( 800, 106, 42,  87, 30),
	HSTT( 850, 113, 44,  93, 31),
	HSTT( 900, 118, 47,  98, 32),
	HSTT( 950, 124, 47, 102, 34),
	HSTT(1000, 130, 49, 107, 35),
	HSTT(1050, 135, 51, 111, 37),
	HSTT(1100, 139, 51, 114, 38),
	HSTT(1150, 146, 54, 120, 40),
	HSTT(1200, 153, 57, 125, 41),
	HSTT(1250, 158, 58, 130, 42),
	HSTT(1300, 163, 58, 135, 44),
	HSTT(1350, 168, 60, 140, 45),
	HSTT(1400, 172, 64, 144, 47),
	HSTT(1450, 176, 65, 148, 48),
	HSTT(1500, 181, 66, 153, 50)
};

static int dw_mipi_dsi_rockchip_get_timing(void *priv_data,
					   unsigned int lane_mbps,
					   struct mipi_dsi_phy_timing *timing)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hstt_table); i++)
		if (lane_mbps < hstt_table[i].maxfreq)
			break;

	if (i == ARRAY_SIZE(hstt_table))
		i--;

	*timing = hstt_table[i].timing;

	return 0;
}

static const struct mipi_dsi_phy_ops dsi_rockchip_phy_ops = {
	.init = dsi_phy_init,
	.get_lane_mbps = dw_mipi_dsi_get_lane_mbps,
	.get_timing = dw_mipi_dsi_rockchip_get_timing,
	.post_set_mode = dsi_phy_post_set_mode,
};

static int dw_mipi_dsi_rockchip_attach(struct udevice *dev)
{
	struct dw_rockchip_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct mipi_dsi_panel_plat *mplat;
	struct display_timing timings;
	int ret;

	ret = uclass_first_device_err(UCLASS_PANEL, &priv->panel);
	if (ret) {
		dev_err(dev, "panel device error %d\n", ret);
		return ret;
	}

	mplat = dev_get_plat(priv->panel);
	mplat->device = &priv->device;
	device->lanes = mplat->lanes;
	device->format = mplat->format;
	device->mode_flags = mplat->mode_flags;

	ret = panel_get_display_timing(priv->panel, &timings);
	if (ret) {
		ret = ofnode_decode_display_timing(dev_ofnode(priv->panel),
						   0, &timings);
		if (ret) {
			dev_err(dev, "decode display timing error %d\n", ret);
			return ret;
		}
	}

	ret = uclass_get_device(UCLASS_DSI_HOST, 0, &priv->dsi_host);
	if (ret) {
		dev_err(dev, "No video dsi host detected %d\n", ret);
		return ret;
	}

	ret = dsi_host_init(priv->dsi_host, device, &timings, 4,
			    &dsi_rockchip_phy_ops);
	if (ret) {
		dev_err(dev, "failed to initialize mipi dsi host\n");
		return ret;
	}

	return 0;
}

static int dw_mipi_dsi_rockchip_set_bl(struct udevice *dev, int percent)
{
	struct dw_rockchip_dsi_priv *priv = dev_get_priv(dev);
	int ret;

	/*
	 * Allow backlight to be optional, since this driver may be
	 * used to simply detect a panel rather than bring one up.
	 */
	ret = panel_enable_backlight(priv->panel);
	if ((ret) && (ret != -ENOSYS)) {
		dev_err(dev, "panel %s enable backlight error %d\n",
			priv->panel->name, ret);
		return ret;
	}

	ret = dsi_host_enable(priv->dsi_host);
	if (ret) {
		dev_err(dev, "failed to enable mipi dsi host\n");
		return ret;
	}

	return 0;
}

static void dw_mipi_dsi_rockchip_config(struct dw_rockchip_dsi_priv *dsi)
{
	if (dsi->cdata->lanecfg1_grf_reg)
		rk_setreg(dsi->grf + dsi->cdata->lanecfg1_grf_reg, dsi->cdata->lanecfg1);

	if (dsi->cdata->lanecfg2_grf_reg)
		rk_setreg(dsi->grf + dsi->cdata->lanecfg2_grf_reg, dsi->cdata->lanecfg2);

	if (dsi->cdata->enable_grf_reg)
		rk_setreg(dsi->grf + dsi->cdata->enable_grf_reg, dsi->cdata->enable);
}

static int dw_mipi_dsi_rockchip_bind(struct udevice *dev)
{
	int ret;

	ret = device_bind_driver_to_node(dev, "dw_mipi_dsi", "dsihost",
					 dev_ofnode(dev), NULL);
	if (ret) {
		dev_err(dev, "failed to bind driver to node\n");
		return ret;
	}

	return dm_scan_fdt_dev(dev);
}

static int dw_mipi_dsi_rockchip_probe(struct udevice *dev)
{
	struct dw_rockchip_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	int ret, i;
	const struct rockchip_dw_dsi_chip_data *cdata =
			(const struct rockchip_dw_dsi_chip_data *)dev_get_driver_data(dev);

	device->dev = dev;

	priv->base = (void *)dev_read_addr(dev);
	if ((fdt_addr_t)priv->base == FDT_ADDR_T_NONE) {
		dev_err(dev, "dsi dt register address error\n");
		return -EINVAL;
	}

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	i = 0;
	while (cdata[i].reg) {
		if (cdata[i].reg == (fdt_addr_t)priv->base) {
			priv->cdata = &cdata[i];
			break;
		}

		i++;
	}

	if (!priv->cdata) {
		dev_err(dev, "no dsi-config for %s node\n", dev->name);
		return -EINVAL;
	}

	/*
	 * Get an optional external dphy. The external dphy stays as
	 * NULL if it's not initialized.
	 */
	ret = generic_phy_get_by_name(dev, "dphy", &priv->phy);
	if (ret && ret != -ENODATA) {
		dev_err(dev, "failed to get mipi dphy: %d\n", ret);
		return ret;
	}

	priv->pclk = devm_clk_get(dev, "pclk");
	if (IS_ERR(priv->pclk)) {
		ret = PTR_ERR(priv->pclk);
		dev_err(dev, "peripheral clock get error %d\n", ret);
		return ret;
	}

	/* Get a ref clock only if not using an external phy. */
	if (generic_phy_valid(&priv->phy)) {
		dev_dbg(dev, "setting priv->ref to NULL\n");
		priv->ref = NULL;

	} else {
		priv->ref = devm_clk_get(dev, "ref");
		if (IS_ERR(priv->ref)) {
			ret = PTR_ERR(priv->ref);
			dev_err(dev, "pll reference clock get error %d\n", ret);
			return ret;
		}
	}

	if (cdata->flags & DW_MIPI_NEEDS_PHY_CFG_CLK) {
		priv->phy_cfg_clk = devm_clk_get(dev, "phy_cfg");
		if (IS_ERR(priv->phy_cfg_clk)) {
			ret = PTR_ERR(priv->phy_cfg_clk);
			dev_err(dev, "phy_cfg_clk clock get error %d\n", ret);
			return ret;
		}

		clk_enable(priv->phy_cfg_clk);
	}

	if (cdata->flags & DW_MIPI_NEEDS_GRF_CLK) {
		priv->grf_clk = devm_clk_get(dev, "grf");
		if (IS_ERR(priv->grf_clk)) {
			ret = PTR_ERR(priv->grf_clk);
			dev_err(dev, "grf_clk clock get error %d\n", ret);
			return ret;
		}

		clk_enable(priv->grf_clk);
	}

	priv->rst = devm_reset_control_get_by_index(device->dev, 0);
	if (IS_ERR(priv->rst)) {
		ret = PTR_ERR(priv->rst);
		dev_err(dev, "missing dsi hardware reset %d\n", ret);
		return ret;
	}

	/* Reset */
	reset_deassert(priv->rst);

	dw_mipi_dsi_rockchip_config(priv);

	return 0;
}

struct video_bridge_ops dw_mipi_dsi_rockchip_ops = {
	.attach = dw_mipi_dsi_rockchip_attach,
	.set_backlight = dw_mipi_dsi_rockchip_set_bl,
};

static const struct rockchip_dw_dsi_chip_data rk3399_chip_data[] = {
	{
		.reg = 0xff960000,
		.lcdsel_grf_reg = RK3399_GRF_SOC_CON20,
		.lcdsel_big = HIWORD_UPDATE(0, RK3399_DSI0_LCDC_SEL),
		.lcdsel_lit = HIWORD_UPDATE(RK3399_DSI0_LCDC_SEL,
					    RK3399_DSI0_LCDC_SEL),

		.lanecfg1_grf_reg = RK3399_GRF_SOC_CON22,
		.lanecfg1 = HIWORD_UPDATE(0, RK3399_DSI0_TURNREQUEST |
					     RK3399_DSI0_TURNDISABLE |
					     RK3399_DSI0_FORCETXSTOPMODE |
					     RK3399_DSI0_FORCERXMODE),

		.flags = DW_MIPI_NEEDS_PHY_CFG_CLK | DW_MIPI_NEEDS_GRF_CLK,
		.max_data_lanes = 4,
	},
	{
		.reg = 0xff968000,
		.lcdsel_grf_reg = RK3399_GRF_SOC_CON20,
		.lcdsel_big = HIWORD_UPDATE(0, RK3399_DSI1_LCDC_SEL),
		.lcdsel_lit = HIWORD_UPDATE(RK3399_DSI1_LCDC_SEL,
					    RK3399_DSI1_LCDC_SEL),

		.lanecfg1_grf_reg = RK3399_GRF_SOC_CON23,
		.lanecfg1 = HIWORD_UPDATE(0, RK3399_DSI1_TURNDISABLE |
					     RK3399_DSI1_FORCETXSTOPMODE |
					     RK3399_DSI1_FORCERXMODE |
					     RK3399_DSI1_ENABLE),

		.lanecfg2_grf_reg = RK3399_GRF_SOC_CON24,
		.lanecfg2 = HIWORD_UPDATE(RK3399_TXRX_MASTERSLAVEZ |
					  RK3399_TXRX_ENABLECLK,
					  RK3399_TXRX_MASTERSLAVEZ |
					  RK3399_TXRX_ENABLECLK |
					  RK3399_TXRX_BASEDIR),

		.enable_grf_reg = RK3399_GRF_SOC_CON23,
		.enable = HIWORD_UPDATE(RK3399_DSI1_ENABLE, RK3399_DSI1_ENABLE),

		.flags = DW_MIPI_NEEDS_PHY_CFG_CLK | DW_MIPI_NEEDS_GRF_CLK,
		.max_data_lanes = 4,
	},
	{ /* sentinel */ }
};

static const struct rockchip_dw_dsi_chip_data rk3568_chip_data[] = {
	{
		.reg = 0xfe060000,
		.lanecfg1_grf_reg = RK3568_GRF_VO_CON2,
		.lanecfg1 = HIWORD_UPDATE(0, RK3568_DSI0_SKEWCALHS |
					  RK3568_DSI0_FORCETXSTOPMODE |
					  RK3568_DSI0_TURNDISABLE |
					  RK3568_DSI0_FORCERXMODE),
		.max_data_lanes = 4,
	},
	{
		.reg = 0xfe070000,
		.lanecfg1_grf_reg = RK3568_GRF_VO_CON3,
		.lanecfg1 = HIWORD_UPDATE(0, RK3568_DSI1_SKEWCALHS |
					  RK3568_DSI1_FORCETXSTOPMODE |
					  RK3568_DSI1_TURNDISABLE |
					  RK3568_DSI1_FORCERXMODE),
		.max_data_lanes = 4,
	},
	{ /* sentinel */ }
};

static const struct udevice_id dw_mipi_dsi_rockchip_dt_ids[] = {
	{ .compatible = "rockchip,rk3399-mipi-dsi",
	  .data = (long)&rk3399_chip_data,
	},
	{ .compatible = "rockchip,rk3568-mipi-dsi",
	  .data = (long)&rk3568_chip_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(dw_mipi_dsi_rockchip) = {
	.name			= "dw-mipi-dsi-rockchip",
	.id			= UCLASS_VIDEO_BRIDGE,
	.of_match		= dw_mipi_dsi_rockchip_dt_ids,
	.bind			= dw_mipi_dsi_rockchip_bind,
	.probe			= dw_mipi_dsi_rockchip_probe,
	.ops			= &dw_mipi_dsi_rockchip_ops,
	.priv_auto		= sizeof(struct dw_rockchip_dsi_priv),
};
