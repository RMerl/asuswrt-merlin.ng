// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Timesys Corporation
 * Copyright 2015 General Electric Company
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <linux/errno.h>
#include <linux/libfdt.h>
#include <asm/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <input.h>
#include <pwm.h>
#include <version.h>
#include <stdlib.h>
#include "../common/ge_common.h"
#include "../common/vpd_reader.h"
#include "../../../drivers/net/e1000.h"
DECLARE_GLOBAL_DATA_PTR;

static int confidx = 3;  /* Default to b850v3. */
static struct vpd_cache vpd;

#define NC_PAD_CTRL (PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	PAD_CTL_HYS)

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |	\
	PAD_CTL_SPEED_HIGH | PAD_CTL_DSE_48ohm | PAD_CTL_SRE_FAST)

#define ENET_CLK_PAD_CTRL (PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_120ohm | PAD_CTL_SRE_FAST)

#define ENET_RX_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
	PAD_CTL_SPEED_HIGH   | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define I2C_PAD MUX_PAD_CTRL(I2C_PAD_CTRL)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart3_pads[] = {
	MX6_PAD_EIM_D31__UART3_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D23__UART3_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D24__UART3_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D25__UART3_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC   | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK  | MUX_PAD_CTRL(ENET_CLK_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	/* AR8033 PHY Reset */
	MX6_PAD_ENET_TX_EN__GPIO1_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

	/* Reset AR8033 PHY */
	gpio_request(IMX_GPIO_NR(1, 28), "fec_rst");
	gpio_direction_output(IMX_GPIO_NR(1, 28), 0);
	mdelay(10);
	gpio_set_value(IMX_GPIO_NR(1, 28), 1);
	mdelay(1);
}

static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_CSI0_DAT9__I2C1_SCL | I2C_PAD,
		.gpio_mode = MX6_PAD_CSI0_DAT9__GPIO5_IO27 | I2C_PAD,
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = MX6_PAD_CSI0_DAT8__I2C1_SDA | I2C_PAD,
		.gpio_mode = MX6_PAD_CSI0_DAT8__GPIO5_IO26 | I2C_PAD,
		.gp = IMX_GPIO_NR(5, 26)
	}
};

static struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_KEY_COL3__I2C2_SCL | I2C_PAD,
		.gpio_mode = MX6_PAD_KEY_COL3__GPIO4_IO12 | I2C_PAD,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6_PAD_KEY_ROW3__I2C2_SDA | I2C_PAD,
		.gpio_mode = MX6_PAD_KEY_ROW3__GPIO4_IO13 | I2C_PAD,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

static struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_3__I2C3_SCL | I2C_PAD,
		.gpio_mode = MX6_PAD_GPIO_3__GPIO1_IO03 | I2C_PAD,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA | I2C_PAD,
		.gpio_mode = MX6_PAD_GPIO_6__GPIO1_IO06 | I2C_PAD,
		.gp = IMX_GPIO_NR(1, 6)
	}
};

static iomux_v3_cfg_t const pcie_pads[] = {
	MX6_PAD_GPIO_5__GPIO1_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO_17__GPIO7_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_pcie(void)
{
	imx_iomux_v3_setup_multiple_pads(pcie_pads, ARRAY_SIZE(pcie_pads));
}

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

static int mx6_rgmii_rework(struct phy_device *phydev)
{
	/* Configure AR8033 to ouput a 125MHz clk from CLK_25M */
	/* set device address 0x7 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	/* offset 0x8016: CLK_25M Clock Select */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	/* enable register write, no post increment, address 0x7 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);
	/* set to 125 MHz from local PLL source */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x18);

	/* rgmii tx clock delay enable */
	/* set debug port address: SerDes Test and System Mode Control */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	/* enable rgmii tx clock delay */
	/* set the reserved bits to avoid board specific voltage peak issue*/
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x3D47);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
static iomux_v3_cfg_t const backlight_pads[] = {
	/* Power for LVDS Display */
	MX6_PAD_EIM_D22__GPIO3_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL),
#define LVDS_POWER_GP IMX_GPIO_NR(3, 22)
	/* Backlight enable for LVDS display */
	MX6_PAD_GPIO_0__GPIO1_IO00 | MUX_PAD_CTRL(NO_PAD_CTRL),
#define LVDS_BACKLIGHT_GP IMX_GPIO_NR(1, 0)
	/* backlight PWM brightness control */
	MX6_PAD_SD1_DAT3__PWM1_OUT | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

int board_cfb_skip(void)
{
	gpio_direction_output(LVDS_POWER_GP, 1);

	return 0;
}

static int is_b850v3(void)
{
	return confidx == 3;
}

static int detect_lcd(struct display_info_t const *dev)
{
	return !is_b850v3();
}

struct display_info_t const displays[] = {{
	.bus	= -1,
	.addr	= -1,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_lcd,
	.enable	= NULL,
	.mode	= {
		.name           = "G121X1-L03",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 20,
		.right_margin   = 300,
		.upper_margin   = 30,
		.lower_margin   = 8,
		.hsync_len      = 1,
		.vsync_len      = 1,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= -1,
	.addr	= 3,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= do_enable_hdmi,
	.mode	= {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

static void enable_videopll(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	s32 timeout = 100000;

	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	/* PLL_VIDEO  455MHz (24MHz * (37+11/12) / 2)
	 *   |
	 * PLL5
	 *   |
	 * CS2CDR[LDB_DI0_CLK_SEL]
	 *   |
	 *   +----> LDB_DI0_SERIAL_CLK_ROOT
	 *   |
	 *   +--> CSCMR2[LDB_DI0_IPU_DIV] --> LDB_DI0_IPU  455 / 7 = 65 MHz
	 */

	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_DIV_SELECT |
			BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT,
			BF_ANADIG_PLL_VIDEO_DIV_SELECT(37) |
			BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(1));

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(11), &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(12), &ccm->analog_pll_video_denom);

	clrbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;

	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_BYPASS,
			BM_ANADIG_PLL_VIDEO_ENABLE);
}

static void setup_display_b850v3(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	enable_videopll();

	/* IPU1 DI0 clock is 455MHz / 7 = 65MHz */
	setbits_le32(&mxc_ccm->cscmr2, MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV);

	imx_setup_hdmi();

	/* Set LDB_DI0 as clock source for IPU_DI0 */
	clrsetbits_le32(&mxc_ccm->chsccdr,
			MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_MASK,
			(CHSCCDR_CLK_SEL_LDB_DI0 <<
			 MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET));

	/* Turn on IPU LDB DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK);

	enable_ipu_clock();

	writel(IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES |
	       IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW |
	       IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW |
	       IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG |
	       IOMUXC_GPR2_DATA_WIDTH_CH1_24BIT |
	       IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG |
	       IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT |
	       IOMUXC_GPR2_SPLIT_MODE_EN_MASK |
	       IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0 |
	       IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0,
	       &iomux->gpr[2]);

	clrbits_le32(&iomux->gpr[3],
		     IOMUXC_GPR3_LVDS0_MUX_CTL_MASK |
		     IOMUXC_GPR3_LVDS1_MUX_CTL_MASK |
		     IOMUXC_GPR3_HDMI_MUX_CTL_MASK);
}

static void setup_display_bx50v3(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	enable_videopll();

	/* When a reset/reboot is performed the display power needs to be turned
	 * off for atleast 500ms. The boot time is ~300ms, we need to wait for
	 * an additional 200ms here. Unfortunately we use external PMIC for
	 * doing the reset, so can not differentiate between POR vs soft reset
	 */
	mdelay(200);

	/* IPU1 DI0 clock is 455MHz / 7 = 65MHz */
	setbits_le32(&mxc_ccm->cscmr2, MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV);

	/* Set LDB_DI0 as clock source for IPU_DI0 */
	clrsetbits_le32(&mxc_ccm->chsccdr,
			MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_MASK,
			(CHSCCDR_CLK_SEL_LDB_DI0 <<
			MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET));

	/* Turn on IPU LDB DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK);

	enable_ipu_clock();

	writel(IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES |
	       IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW |
	       IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG |
	       IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT |
	       IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0,
	       &iomux->gpr[2]);

	clrsetbits_le32(&iomux->gpr[3],
			IOMUXC_GPR3_LVDS0_MUX_CTL_MASK,
		       (IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
			IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET));

	/* backlights off until needed */
	imx_iomux_v3_setup_multiple_pads(backlight_pads,
					 ARRAY_SIZE(backlight_pads));
	gpio_request(LVDS_POWER_GP, "lvds_power");
	gpio_direction_input(LVDS_POWER_GP);
}
#endif /* CONFIG_VIDEO_IPUV3 */

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

#define VPD_TYPE_INVALID 0x00
#define VPD_BLOCK_NETWORK 0x20
#define VPD_BLOCK_HWID 0x44
#define VPD_PRODUCT_B850 1
#define VPD_PRODUCT_B650 2
#define VPD_PRODUCT_B450 3
#define VPD_HAS_MAC1 0x1
#define VPD_HAS_MAC2 0x2
#define VPD_MAC_ADDRESS_LENGTH 6

struct vpd_cache {
	bool is_read;
	u8 product_id;
	u8 has;
	unsigned char mac1[VPD_MAC_ADDRESS_LENGTH];
	unsigned char mac2[VPD_MAC_ADDRESS_LENGTH];
};

/*
 * Extracts MAC and product information from the VPD.
 */
static int vpd_callback(struct vpd_cache *vpd, u8 id, u8 version, u8 type,
			size_t size, u8 const *data)
{
	if (id == VPD_BLOCK_HWID && version == 1 && type != VPD_TYPE_INVALID &&
	    size >= 1) {
		vpd->product_id = data[0];
	} else if (id == VPD_BLOCK_NETWORK && version == 1 &&
		   type != VPD_TYPE_INVALID) {
		if (size >= 6) {
			vpd->has |= VPD_HAS_MAC1;
			memcpy(vpd->mac1, data, VPD_MAC_ADDRESS_LENGTH);
		}
		if (size >= 12) {
			vpd->has |= VPD_HAS_MAC2;
			memcpy(vpd->mac2, data + 6, VPD_MAC_ADDRESS_LENGTH);
		}
	}

	return 0;
}

static void process_vpd(struct vpd_cache *vpd)
{
	int fec_index = -1;
	int i210_index = -1;

	if (!vpd->is_read) {
		printf("VPD wasn't read");
		return;
	}

	switch (vpd->product_id) {
	case VPD_PRODUCT_B450:
		env_set("confidx", "1");
		i210_index = 0;
		fec_index = 1;
		break;
	case VPD_PRODUCT_B650:
		env_set("confidx", "2");
		i210_index = 0;
		fec_index = 1;
		break;
	case VPD_PRODUCT_B850:
		env_set("confidx", "3");
		i210_index = 1;
		fec_index = 2;
		break;
	}

	if (fec_index >= 0 && (vpd->has & VPD_HAS_MAC1))
		eth_env_set_enetaddr_by_index("eth", fec_index, vpd->mac1);

	if (i210_index >= 0 && (vpd->has & VPD_HAS_MAC2))
		eth_env_set_enetaddr_by_index("eth", i210_index, vpd->mac2);
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();
	setup_pcie();

	e1000_initialize(bis);

	return cpu_eth_init(bis);
}

static iomux_v3_cfg_t const misc_pads[] = {
	MX6_PAD_KEY_ROW2__GPIO4_IO11	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_A25__GPIO5_IO02	| MUX_PAD_CTRL(NC_PAD_CTRL),
	MX6_PAD_EIM_CS0__GPIO2_IO23	| MUX_PAD_CTRL(NC_PAD_CTRL),
	MX6_PAD_EIM_CS1__GPIO2_IO24	| MUX_PAD_CTRL(NC_PAD_CTRL),
	MX6_PAD_EIM_OE__GPIO2_IO25	| MUX_PAD_CTRL(NC_PAD_CTRL),
	MX6_PAD_EIM_BCLK__GPIO6_IO31	| MUX_PAD_CTRL(NC_PAD_CTRL),
	MX6_PAD_GPIO_1__GPIO1_IO01	| MUX_PAD_CTRL(NC_PAD_CTRL),
	MX6_PAD_GPIO_9__WDOG1_B         | MUX_PAD_CTRL(NC_PAD_CTRL),
};
#define SUS_S3_OUT	IMX_GPIO_NR(4, 11)
#define WIFI_EN	IMX_GPIO_NR(6, 14)

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(misc_pads,
					 ARRAY_SIZE(misc_pads));

	setup_iomux_uart();

#if defined(CONFIG_VIDEO_IPUV3)
	/* Set LDB clock to Video PLL */
	select_ldb_di_clock_source(MXC_PLL5_CLK);
#endif
	return 0;
}

static void set_confidx(const struct vpd_cache* vpd)
{
	switch (vpd->product_id) {
	case VPD_PRODUCT_B450:
		confidx = 1;
		break;
	case VPD_PRODUCT_B650:
		confidx = 2;
		break;
	case VPD_PRODUCT_B850:
		confidx = 3;
		break;
	}
}

int board_init(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info3);

	if (!read_vpd(&vpd, vpd_callback)) {
		vpd.is_read = true;
		set_confidx(&vpd);
	}

	gpio_request(SUS_S3_OUT, "sus_s3_out");
	gpio_direction_output(SUS_S3_OUT, 1);

	gpio_request(WIFI_EN, "wifi_en");
	gpio_direction_output(WIFI_EN, 1);

#if defined(CONFIG_VIDEO_IPUV3)
	if (is_b850v3())
		setup_display_b850v3();
	else
		setup_display_bx50v3();

	gpio_request(LVDS_BACKLIGHT_GP, "lvds_backlight");
	gpio_direction_input(LVDS_BACKLIGHT_GP);
#endif

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd2",	 MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"sd3",	 MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

void pmic_init(void)
{
#define I2C_PMIC                0x2
#define DA9063_I2C_ADDR         0x58
#define DA9063_REG_BCORE2_CFG   0x9D
#define DA9063_REG_BCORE1_CFG   0x9E
#define DA9063_REG_BPRO_CFG     0x9F
#define DA9063_REG_BIO_CFG      0xA0
#define DA9063_REG_BMEM_CFG     0xA1
#define DA9063_REG_BPERI_CFG    0xA2
#define DA9063_BUCK_MODE_MASK   0xC0
#define DA9063_BUCK_MODE_MANUAL 0x00
#define DA9063_BUCK_MODE_SLEEP  0x40
#define DA9063_BUCK_MODE_SYNC   0x80
#define DA9063_BUCK_MODE_AUTO   0xC0

	uchar val;

	i2c_set_bus_num(I2C_PMIC);

	i2c_read(DA9063_I2C_ADDR, DA9063_REG_BCORE2_CFG, 1, &val, 1);
	val &= ~DA9063_BUCK_MODE_MASK;
	val |= DA9063_BUCK_MODE_SYNC;
	i2c_write(DA9063_I2C_ADDR, DA9063_REG_BCORE2_CFG, 1, &val, 1);

	i2c_read(DA9063_I2C_ADDR, DA9063_REG_BCORE1_CFG, 1, &val, 1);
	val &= ~DA9063_BUCK_MODE_MASK;
	val |= DA9063_BUCK_MODE_SYNC;
	i2c_write(DA9063_I2C_ADDR, DA9063_REG_BCORE1_CFG, 1, &val, 1);

	i2c_read(DA9063_I2C_ADDR, DA9063_REG_BPRO_CFG, 1, &val, 1);
	val &= ~DA9063_BUCK_MODE_MASK;
	val |= DA9063_BUCK_MODE_SYNC;
	i2c_write(DA9063_I2C_ADDR, DA9063_REG_BPRO_CFG, 1, &val, 1);

	i2c_read(DA9063_I2C_ADDR, DA9063_REG_BIO_CFG, 1, &val, 1);
	val &= ~DA9063_BUCK_MODE_MASK;
	val |= DA9063_BUCK_MODE_SYNC;
	i2c_write(DA9063_I2C_ADDR, DA9063_REG_BIO_CFG, 1, &val, 1);

	i2c_read(DA9063_I2C_ADDR, DA9063_REG_BMEM_CFG, 1, &val, 1);
	val &= ~DA9063_BUCK_MODE_MASK;
	val |= DA9063_BUCK_MODE_SYNC;
	i2c_write(DA9063_I2C_ADDR, DA9063_REG_BMEM_CFG, 1, &val, 1);

	i2c_read(DA9063_I2C_ADDR, DA9063_REG_BPERI_CFG, 1, &val, 1);
	val &= ~DA9063_BUCK_MODE_MASK;
	val |= DA9063_BUCK_MODE_SYNC;
	i2c_write(DA9063_I2C_ADDR, DA9063_REG_BPERI_CFG, 1, &val, 1);
}

int board_late_init(void)
{
	process_vpd(&vpd);

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	if (is_b850v3())
		env_set("videoargs", "video=DP-1:1024x768@60 video=HDMI-A-1:1024x768@60");
	else
		env_set("videoargs", "video=LVDS-1:1024x768@65");

	/* board specific pmic init */
	pmic_init();

	check_time();

	return 0;
}

/*
 * Removes the 'eth[0-9]*addr' environment variable with the given index
 *
 * @param index [in] the index of the eth_device whose variable is to be removed
 */
static void remove_ethaddr_env_var(int index)
{
	char env_var_name[9];

	sprintf(env_var_name, index == 0 ? "ethaddr" : "eth%daddr", index);
	env_set(env_var_name, NULL);
}

int last_stage_init(void)
{
	int i;

	/*
	 * Remove first three ethaddr which may have been created by
	 * function process_vpd().
	 */
	for (i = 0; i < 3; ++i)
		remove_ethaddr_env_var(i);

	return 0;
}

int checkboard(void)
{
	printf("BOARD: %s\n", CONFIG_BOARD_NAME);
	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	fdt_setprop(blob, 0, "ge,boot-ver", version_string,
	                                    strlen(version_string) + 1);
	return 0;
}
#endif

static int do_backlight_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_VIDEO_IPUV3
	/* We need at least 200ms between power on and backlight on
	 * as per specifications from CHI MEI */
	mdelay(250);

	/* enable backlight PWM 1 */
	pwm_init(0, 0, 0);

	/* duty cycle 5000000ns, period: 5000000ns */
	pwm_config(0, 5000000, 5000000);

	/* Backlight Power */
	gpio_direction_output(LVDS_BACKLIGHT_GP, 1);

	pwm_enable(0);
#endif

	return 0;
}

U_BOOT_CMD(
       bx50_backlight_enable, 1,      1,      do_backlight_enable,
       "enable Bx50 backlight",
       ""
);
