// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Timesys Corporation
 * Copyright 2015 General Electric Company
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#include <event.h>
#include <image.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <env.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/libfdt.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/video.h>
#include <mmc.h>
#include <fsl_esdhc_imx.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <power/regulator.h>
#include <power/da9063_pmic.h>
#include <power/pmic.h>
#include <input.h>
#include <pwm.h>
#include <version_string.h>
#include <stdlib.h>
#include <dm/root.h>
#include "../common/ge_rtc.h"
#include "../common/vpd_reader.h"
#include "../../../drivers/net/e1000.h"
#include <pci.h>
#include <panel.h>

DECLARE_GLOBAL_DATA_PTR;

#define VPD_PRODUCT_B850 1
#define VPD_PRODUCT_B650 2
#define VPD_PRODUCT_B450 3

#define AR8033_DBG_REG_ADDR		0x1d
#define AR8033_DBG_REG_DATA		0x1e
#define AR8033_SERDES_REG		0x5

static int productid;  /* Default to generic. */
static struct vpd_cache vpd;

#define NC_PAD_CTRL (PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	/*
	 * Set reserved bits to avoid board specific voltage peak issue. The
	 * value is a magic number provided directly by Qualcomm. Note, that
	 * PHY driver will take control of BIT(8) in this register to control
	 * TX clock delay, so we do not initialize that bit here.
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, AR8033_DBG_REG_ADDR, AR8033_SERDES_REG);
	phy_write(phydev, MDIO_DEVAD_NONE, AR8033_DBG_REG_DATA, 0x3c47);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
static void do_enable_backlight(struct display_info_t const *dev)
{
	struct udevice *panel;
	int ret;

	ret = uclass_get_device(UCLASS_PANEL, 0, &panel);
	if (ret) {
		printf("Could not find panel: %d\n", ret);
		return;
	}

	panel_set_backlight(panel, 100);
	panel_enable_backlight(panel);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static int is_b850v3(void)
{
	return productid == VPD_PRODUCT_B850;
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
	.enable	= do_enable_backlight,
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
	int fec_index = 0;
	int i210_index = -1;

	if (!vpd->is_read) {
		printf("VPD wasn't read");
		return;
	}

	if (vpd->has & VPD_HAS_MAC1)
		eth_env_set_enetaddr_by_index("eth", fec_index, vpd->mac1);

	env_set("ethact", "eth0");

	switch (vpd->product_id) {
	case VPD_PRODUCT_B450:
		i210_index = 1;
		break;
	case VPD_PRODUCT_B650:
		i210_index = 1;
		break;
	case VPD_PRODUCT_B850:
		i210_index = 2;
		break;
	}

	if (i210_index >= 0 && (vpd->has & VPD_HAS_MAC2))
		eth_env_set_enetaddr_by_index("eth", i210_index, vpd->mac2);
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
#define PWGIN_IN	IMX_GPIO_NR(4, 14)
#define WIFI_EN	IMX_GPIO_NR(6, 14)

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(misc_pads,
					 ARRAY_SIZE(misc_pads));

#if defined(CONFIG_VIDEO_IPUV3)
	/* Set LDB clock to Video PLL */
	select_ldb_di_clock_source(MXC_PLL5_CLK);
#endif
	return 0;
}

int board_init(void)
{
	if (!read_i2c_vpd(&vpd, vpd_callback)) {
		int ret, rescan;

		vpd.is_read = true;
		productid = vpd.product_id;

		ret = fdtdec_resetup(&rescan);
		if (!ret && rescan) {
			dm_uninit();
			dm_init_and_scan(false);
		}
	}

	gpio_request(SUS_S3_OUT, "sus_s3_out");
	gpio_direction_output(SUS_S3_OUT, 1);

	gpio_request(PWGIN_IN, "pwgin_in");
	gpio_direction_input(PWGIN_IN);

	gpio_request(WIFI_EN, "wifi_en");
	gpio_direction_output(WIFI_EN, 1);

#if defined(CONFIG_VIDEO_IPUV3)
	if (is_b850v3())
		setup_display_b850v3();
	else
		setup_display_bx50v3();
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


/*
 * The SoM used by these boards has XTAL not connected despite datasheet
 * suggesting connecting unused XTAL pins to ground. Without explicitly
 * clearing the CRYSTAL bit the system runs unstable and sometimes reboots
 * unexpectedly.
 */
static void pmic_crystal_fix(void)
{
	struct udevice *pmic;
	static const uint EN_32K_CRYSTAL = (1 << 3);

	if (pmic_get("pmic@58", &pmic)) {
		puts("failed to get device for PMIC\n");
		return;
	}

	if (pmic_clrsetbits(pmic, DA9063_REG_EN_32K, EN_32K_CRYSTAL, 0) < 0) {
		puts("failed to clear CRYSTAL bit\n");
		return;
	}
}

void pmic_init(void)
{
	struct udevice *reg;
	int ret, i;
	static const char * const bucks[] = {
		"bcore1",
		"bcore2",
		"bpro",
		"bmem",
		"bio",
		"bperi",
	};

	pmic_crystal_fix();

	for (i = 0; i < ARRAY_SIZE(bucks); i++) {
		ret = regulator_get_by_devname(bucks[i], &reg);
		if (reg < 0) {
			printf("%s(): Unable to get regulator %s: %d\n",
			       __func__, bucks[i], ret);
			continue;
		}
		regulator_set_mode(reg, DA9063_BUCKMODE_SYNC);
	}
}

static void detect_boot_cause(void)
{
	const char *cause = "POR";

	if (is_b850v3())
		if (!gpio_get_value(PWGIN_IN))
			cause = "PM_WDOG";

	env_set("bootcause", cause);
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

	detect_boot_cause();

	/* board specific pmic init */
	pmic_init();

	check_time();

	pci_init();

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

static int last_stage_init(void)
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
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);

int checkboard(void)
{
	printf("BOARD: General Electric Bx50v3\n");
	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	char *rtc_status = env_get("rtc_status");

	fdt_setprop(blob, 0, "ge,boot-ver", version_string,
		    strlen(version_string) + 1);

	fdt_setprop(blob, 0, "ge,rtc-status", rtc_status,
		    strlen(rtc_status) + 1);
	return 0;
}
#endif

int board_fit_config_name_match(const char *name)
{
	const char *machine = name;

	if (!vpd.is_read)
		return strcmp(name, "imx6q-bx50v3");

	if (!strncmp(machine, "Boot ", 5))
		machine += 5;
	if (!strncmp(machine, "imx6q-", 6))
		machine += 6;

	switch (vpd.product_id) {
	case VPD_PRODUCT_B450:
		return strcasecmp(machine, "b450v3");
	case VPD_PRODUCT_B650:
		return strcasecmp(machine, "b650v3");
	case VPD_PRODUCT_B850:
		return strcasecmp(machine, "b850v3");
	default:
		return -1;
	}
}

int embedded_dtb_select(void)
{
	vpd.is_read = false;
	return fdtdec_setup();
}
