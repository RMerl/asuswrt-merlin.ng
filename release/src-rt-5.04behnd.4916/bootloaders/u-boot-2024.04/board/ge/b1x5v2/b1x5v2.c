/*
 * GE B105v2, B125v2, B155v2
 *
 * Copyright 2018-2020 GE Inc.
 * Copyright 2018-2020 Collabora Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-imx/video.h>
#include <command.h>
#include <common.h>
#include <i2c.h>
#include <input.h>
#include <ipu_pixfmt.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <malloc.h>
#include <miiphy.h>
#include <micrel.h>
#include <netdev.h>
#include <panel.h>
#include <rtc.h>
#include <spi_flash.h>
#include <version_string.h>

#include "../common/vpd_reader.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SPL_BUILD

#define B1X5V2_GE_VPD_OFFSET	0x0100000
#define B1X5V2_GE_VPD_SIZE	1022

#define VPD_TYPE_INVALID	0x00
#define VPD_BLOCK_NETWORK	0x20
#define VPD_BLOCK_HWID		0x44
#define VPD_MAC_ADDRESS_LENGTH	6

#define VPD_FLAG_VALID_MAC	BIT(1)

#define AR8035_PHY_ID			0x004dd072
#define AR8035_PHY_DEBUG_ADDR_REG	0x1d
#define AR8035_PHY_DEBUG_DATA_REG	0x1e
#define AR8035_HIB_CTRL_REG		0xb
#define AR8035_HIBERNATE_EN		(1 << 15)

static struct vpd_cache {
	bool is_read;
	u8 product_id;
	unsigned char mac[VPD_MAC_ADDRESS_LENGTH];
	u32 flags;
} vpd;

enum product_type {
	PRODUCT_TYPE_B105V2 = 6,
	PRODUCT_TYPE_B105PV2 = 7,
	PRODUCT_TYPE_B125V2 = 8,
	PRODUCT_TYPE_B125PV2 = 9,
	PRODUCT_TYPE_B155V2 = 10,

	PRODUCT_TYPE_INVALID = 0,
};

int dram_init(void) {
	gd->ram_size = imx_ddr_size();
	return 0;
}

int power_init_board(void)
{
	/* all required PMIC configuration happens via DT */
	return 0;
}

static int disable_phy_hibernation(struct phy_device *phydev)
{
	unsigned short val;

	if (phydev->drv->uid == AR8035_PHY_ID) {
		/* Disable hibernation, other configuration has been done by PHY driver */
		phy_write(phydev, MDIO_DEVAD_NONE, AR8035_PHY_DEBUG_ADDR_REG, AR8035_HIB_CTRL_REG);
		val = phy_read(phydev, MDIO_DEVAD_NONE, AR8035_PHY_DEBUG_DATA_REG);
		val &= ~AR8035_HIBERNATE_EN;
		phy_write(phydev, MDIO_DEVAD_NONE, AR8035_PHY_DEBUG_DATA_REG, val);
	} else {
		printf("Unknown PHY: %08x\n", phydev->drv->uid);
	}

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	disable_phy_hibernation(phydev);

	return 0;
}

static int auo_g101evn01_detect(const struct display_info_t *info)
{
	char *dev = env_get("devicetype");
	return !strcmp(dev, "B105v2") || !strcmp(dev, "B105Pv2");
}

static int auo_g121ean01_detect(const struct display_info_t *info)
{
	char *dev = env_get("devicetype");
	return !strcmp(dev, "B125v2") || !strcmp(dev, "B125Pv2");;
}

static int auo_g156xtn01_detect(const struct display_info_t *info)
{
	char *dev = env_get("devicetype");
	return !strcmp(dev, "B155v2");
}

static void b1x5v2_backlight_enable(int percent)
{
	struct udevice *panel;
	int ret;

	ret = uclass_get_device(UCLASS_PANEL, 0, &panel);
	if (ret) {
		printf("Could not find panel: %d\n", ret);
		return;
	}

	panel_set_backlight(panel, percent);
	panel_enable_backlight(panel);

}

static void lcd_enable(const struct display_info_t *info)
{
	printf("Enable backlight...\n");
	b1x5v2_backlight_enable(100);
}

struct display_info_t const displays[] = {
{
	.di = 0,
	.bus = -1,
	.addr = -1,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = auo_g156xtn01_detect,
	.enable = lcd_enable,
	.mode = {
		.name = "AUO G156XTN01",
		.refresh = 60,
		.xres = 1368, /* because of i.MX6 limitation, actually 1366 */
		.yres = 768,
		.pixclock = 13158, /* 76 MHz in ps */
		.left_margin = 33,
		.right_margin = 67,
		.upper_margin = 4,
		.lower_margin = 4,
		.hsync_len = 94,
		.vsync_len = 30,
		.sync = FB_SYNC_EXT,
		.vmode = FB_VMODE_NONINTERLACED
	}
},
{
	.di = 0,
	.bus = -1,
	.addr = -1,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = auo_g121ean01_detect,
	.enable = lcd_enable,
	.mode = {
		.name = "AUO G121EAN01.4",
		.refresh = 60,
		.xres = 1280,
		.yres = 800,
		.pixclock = 14992, /* 66.7 MHz in ps */
		.left_margin = 8,
		.right_margin = 58,
		.upper_margin = 6,
		.lower_margin = 4,
		.hsync_len = 70,
		.vsync_len = 10,
		.sync = FB_SYNC_EXT,
		.vmode = FB_VMODE_NONINTERLACED
	}
},
{
	.di = 0,
	.bus = -1,
	.addr = -1,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = auo_g101evn01_detect,
	.enable = lcd_enable,
	.mode = {
		.name = "AUO G101EVN01.3",
		.refresh = 60,
		.xres = 1280,
		.yres = 800,
		.pixclock = 14992, /* 66.7 MHz in ps */
		.left_margin = 8,
		.right_margin = 58,
		.upper_margin = 6,
		.lower_margin = 4,
		.hsync_len = 70,
		.vsync_len = 10,
		.sync = FB_SYNC_EXT,
		.vmode = FB_VMODE_NONINTERLACED
	}
}
};
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

static void setup_display(void)
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

	/* CCM_CSCMR2 -> ldb_di0_ipu_div [IMX6SDLRM page 839] */
	/* divide IPU clock by 7 */
	setbits_le32(&mxc_ccm->cscmr2, MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV);

	/* CCM_CHSCCDR -> ipu1_di0_clk_sel [IMX6SDLRM page 849] */
	/* Set LDB_DI0 as clock source for IPU_DI0 */
	clrsetbits_le32(&mxc_ccm->chsccdr,
			MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_MASK,
			(CHSCCDR_CLK_SEL_LDB_DI0 <<
			MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET));

	/* Turn on IPU LDB DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK);

	enable_ipu_clock();

	/* IOMUXC_GPR2 [IMX6SDLRM page 2049] */
	/* Set LDB Channel 0 in SPWG 24 Bit mode */
	writel(IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH |
	       IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG |
	       IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT |
	       IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0,
	       &iomux->gpr[2]);

	/* IOMUXC_GPR3 [IMX6SDLRM page 2051] */
	/* LVDS0 is connected to IPU DI0 */
	clrsetbits_le32(&iomux->gpr[3],
			IOMUXC_GPR3_LVDS0_MUX_CTL_MASK,
		       (IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
			IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET));
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_early_init_f(void)
{
	select_ldb_di_clock_source(MXC_PLL5_CLK);

	return 0;
}

static int eeti_touch_get_model(struct udevice *dev, char *result) {
	u8 query[68] = {0x67, 0x00, 0x42, 0x00, 0x03, 0x01, 'E', 0x00, 0x00, 0x00};
	struct i2c_msg qmsg = {
		.addr = 0x2a,
		.flags = 0,
		.len = sizeof(query),
		.buf = query,
	};
	u8 reply[66] = {0};
	struct i2c_msg rmsg = {
		.addr = 0x2a,
		.flags = I2C_M_RD,
		.len = sizeof(reply),
		.buf = reply,
	};
	int err;

	err = dm_i2c_xfer(dev, &qmsg, 1);
	if (err)
		return err;

	/*
	 * device sends IRQ when its ok to read. To keep the code
	 * simple we just wait an arbitrary, long enough time period.
	 */
	mdelay(10);

	err = dm_i2c_xfer(dev, &rmsg, 1);
	if (err)
		return err;

	if (reply[0] != 0x42 || reply[1] != 0x00 ||
	    reply[2] != 0x03 || reply[4] != 'E')
		return -EPROTO;

	memcpy(result, reply+5, 10);
	return 0;
}

static bool b1x5v2_board_is_p_model(void)
{
	struct udevice *bus = NULL;
	struct udevice *dev = NULL;
	int err;

	err = uclass_get_device_by_name(UCLASS_I2C, "i2c@21a0000", &bus);
	if (err || !bus) {
		printf("Could not get I2C bus: %d\n", err);
		return true;
	}

	/* The P models do not have this port expander */
	err = dm_i2c_probe(bus, 0x21, 0, &dev);
	if (err || !dev) {
		return true;
	}

	return false;
}

static enum product_type b1x5v2_board_type(void)
{
	struct udevice *bus = NULL;
	struct udevice *dev = NULL;
	char model[11] = {0};
	int err;
	int retry;

	err = uclass_get_device_by_name(UCLASS_I2C, "i2c@21a8000", &bus);
	if (err) {
		printf("Could not get I2C bus: %d\n", err);
		return PRODUCT_TYPE_INVALID;
	}

	err = dm_i2c_probe(bus, 0x41, 0, &dev);
	if (!err && dev) { /* Ilitek Touchscreen */
		if (b1x5v2_board_is_p_model()) {
			return PRODUCT_TYPE_B105PV2;
		} else {
			return PRODUCT_TYPE_B105V2;
		}
	}

	err = dm_i2c_probe(bus, 0x2a, 0, &dev);
	if (err || !dev) {
		printf("Could not find touchscreen: %d\n", err);
		return PRODUCT_TYPE_INVALID;
	}

	for (retry = 0; retry < 3; ++retry) {
		err = eeti_touch_get_model(dev, model);
		if (!err)
			break;
		printf("Retry %d read EETI touchscreen model: %d\n", retry + 1, err);
	}
	if (err) {
		printf("Could not read EETI touchscreen model: %d\n", err);
		return PRODUCT_TYPE_INVALID;
	}

	if (!strcmp(model, "Orion_1320")) { /* EETI EXC80H60 */
		if (b1x5v2_board_is_p_model()) {
			return PRODUCT_TYPE_B125PV2;
		} else {
			return PRODUCT_TYPE_B125V2;
		}
	} else if (!strcmp(model, "Orion_1343")) { /* EETI EXC80H84 */
		return PRODUCT_TYPE_B155V2;
	}

	printf("Unknown EETI touchscreen model: %s\n", model);
	return PRODUCT_TYPE_INVALID;
}

static void set_env_per_board_type(enum product_type type)
{
	switch (type) {
	case PRODUCT_TYPE_B105V2:
		env_set("resolution", "1280x800");
		env_set("devicetype", "B105v2");
		env_set("fdtfile", "imx6dl-b105v2.dtb");
		break;
	case PRODUCT_TYPE_B105PV2:
		env_set("resolution", "1280x800");
		env_set("devicetype", "B105Pv2");
		env_set("fdtfile", "imx6dl-b105pv2.dtb");
		break;
	case PRODUCT_TYPE_B125V2:
		env_set("resolution", "1280x800");
		env_set("devicetype", "B125v2");
		env_set("fdtfile", "imx6dl-b125v2.dtb");
		break;
	case PRODUCT_TYPE_B125PV2:
		env_set("resolution", "1280x800");
		env_set("devicetype", "B125Pv2");
		env_set("fdtfile", "imx6dl-b125pv2.dtb");
		break;
	case PRODUCT_TYPE_B155V2:
		env_set("resolution", "1366x768");
		env_set("devicetype", "B155v2");
		env_set("fdtfile", "imx6dl-b155v2.dtb");
		break;
	default:
		break;
	}
}

static int b1x5v2_board_type_autodetect(void)
{
	enum product_type product = b1x5v2_board_type();
	if (product != PRODUCT_TYPE_INVALID) {
		set_env_per_board_type(product);
		return 0;
	}
	return -1;
}

/*
 * Extracts MAC and product information from the VPD.
 */
static int vpd_callback(struct vpd_cache *vpd, u8 id, u8 version, u8 type,
			size_t size, u8 const *data)
{
	if (type == VPD_TYPE_INVALID)
		return 0;

	if (id == VPD_BLOCK_HWID && version == 1 && size >= 1) {
		vpd->product_id = data[0];
	} else if (id == VPD_BLOCK_NETWORK && version == 1) {
		if (size >= VPD_MAC_ADDRESS_LENGTH) {
			memcpy(vpd->mac, data, VPD_MAC_ADDRESS_LENGTH);
			vpd->flags |= VPD_FLAG_VALID_MAC;
		}
	}

	return 0;
}

static int read_spi_vpd(struct vpd_cache *cache,
		 int (*process_block)(struct vpd_cache *, u8 id, u8 version,
				      u8 type, size_t size, u8 const *data))
{
	static const int size = B1X5V2_GE_VPD_SIZE;
	struct udevice *dev;
	int ret;
	u8 *data;

	ret = uclass_get_device_by_name(UCLASS_SPI_FLASH, "m25p80@0", &dev);
	if (ret)
		return ret;

	data = malloc(size);
	if (!data)
		return -ENOMEM;

	ret = spi_flash_read_dm(dev, B1X5V2_GE_VPD_OFFSET, size, data);
	if (ret) {
		free(data);
		return ret;
	}

	ret = vpd_reader(size, data, cache, process_block);

	free(data);

	return ret;
}

int board_init(void)
{
	if (!read_spi_vpd(&vpd, vpd_callback)) {
		vpd.is_read = true;
	}

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	setup_display();

	return 0;
}

static void init_bootcause(void)
{
	const char *cause;

	/* We care about WDOG only, treating everything else as
	 * a power-on-reset.
	 */
	if (get_imx_reset_cause() & 0x0010)
		cause = "WDOG";
	else
		cause = "POR";

	env_set("bootcause", cause);
}

int misc_init_r(void)
{
	init_bootcause();

	return 0;
}

#define M41T62_REG_FLAGS	0xf
#define M41T62_FLAGS_OF		(1 << 2)
static void check_time(void)
{
	struct udevice *rtc = NULL;
	struct rtc_time tm;
	u8 val;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_RTC, "m41t62@68", &rtc);
	if (ret) {
		printf("Could not get RTC: %d\n", ret);
		env_set("rtc_status", "FAIL");
		return;
	}

	ret = dm_i2c_read(rtc, M41T62_REG_FLAGS, &val, sizeof(val));
	if (ret) {
		printf("Could not read RTC register: %d\n", ret);
		env_set("rtc_status", "FAIL");
		return;
	}

	ret = dm_rtc_reset(rtc);
	if (ret) {
		printf("Could not reset RTC: %d\n", ret);
		env_set("rtc_status", "FAIL");
		return;
	}

	if (val & M41T62_FLAGS_OF) {
		env_set("rtc_status", "STOP");
		return;
	}

	ret = dm_rtc_get(rtc, &tm);
	if (ret) {
		printf("Could not read RTC: %d\n", ret);
		env_set("rtc_status", "FAIL");
		return;
	}

	if (tm.tm_year > 2037) {
		tm.tm_sec  = 0;
		tm.tm_min  = 0;
		tm.tm_hour = 0;
		tm.tm_mday = 1;
		tm.tm_wday = 2;
		tm.tm_mon  = 1;
		tm.tm_year = 2036;

		ret = dm_rtc_set(rtc, &tm);
		if (ret) {
			printf("Could not update RTC: %d\n", ret);
			env_set("rtc_status", "FAIL");
			return;
		}

		printf("RTC behind 2037, capped to 2036 for userspace handling\n");
		env_set("rtc_status", "2038");
		return;
	}

	env_set("rtc_status", "OK");
}

static void process_vpd(struct vpd_cache *vpd)
{
	if (!vpd->is_read) {
		printf("VPD wasn't read\n");
		return;
	}

	if (vpd->flags & VPD_FLAG_VALID_MAC) {
		eth_env_set_enetaddr_by_index("eth", 0, vpd->mac);
		env_set("ethact", "eth0");
	}
}

int board_late_init(void)
{
	process_vpd(&vpd);

	if (vpd.product_id >= PRODUCT_TYPE_B105V2 &&
	    vpd.product_id <= PRODUCT_TYPE_B155V2) {
		set_env_per_board_type((enum product_type)vpd.product_id);
	} else {
		b1x5v2_board_type_autodetect();
	}

	printf("Board: GE %s\n", env_get("devicetype"));

	check_time();

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

static int do_b1x5v2_autodetect(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int err;

	err = b1x5v2_board_type_autodetect();
	if (!err)
		printf("Identified %s\n", env_get("devicetype"));

	return 0;
}

U_BOOT_CMD(
       autodetect_devtype, 1,      1,      do_b1x5v2_autodetect,
       "autodetect b1x5v2 device type",
       ""
);

#endif // CONFIG_SPL_BUILD
