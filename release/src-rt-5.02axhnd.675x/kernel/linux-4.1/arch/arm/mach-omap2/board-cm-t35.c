/*
 * CompuLab CM-T35/CM-T3730 modules support
 *
 * Copyright (C) 2009-2011 CompuLab, Ltd.
 * Authors: Mike Rapoport <mike@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input/matrix_keypad.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/omap-gpmc.h>
#include <linux/platform_data/gpio-omap.h>

#include <linux/platform_data/at24.h>
#include <linux/i2c/twl.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/mmc/host.h>
#include <linux/usb/phy.h>

#include <linux/spi/spi.h>
#include <linux/spi/tdo24m.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <linux/platform_data/mtd-nand-omap2.h>
#include <video/omapdss.h>
#include <video/omap-panel-data.h>
#include <linux/platform_data/spi-omap2-mcspi.h>

#include "common.h"
#include "mux.h"
#include "sdram-micron-mt46h32m32lf-6.h"
#include "hsmmc.h"
#include "common-board-devices.h"

#define CM_T35_GPIO_PENDOWN		57
#define SB_T35_USB_HUB_RESET_GPIO	167

#define CM_T35_SMSC911X_CS	5
#define CM_T35_SMSC911X_GPIO	163
#define SB_T35_SMSC911X_CS	4
#define SB_T35_SMSC911X_GPIO	65

#if defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)
#include <linux/smsc911x.h>
#include "gpmc-smsc911x.h"

static struct omap_smsc911x_platform_data cm_t35_smsc911x_cfg = {
	.id		= 0,
	.cs             = CM_T35_SMSC911X_CS,
	.gpio_irq       = CM_T35_SMSC911X_GPIO,
	.gpio_reset     = -EINVAL,
	.flags		= SMSC911X_USE_32BIT | SMSC911X_SAVE_MAC_ADDRESS,
};

static struct omap_smsc911x_platform_data sb_t35_smsc911x_cfg = {
	.id		= 1,
	.cs             = SB_T35_SMSC911X_CS,
	.gpio_irq       = SB_T35_SMSC911X_GPIO,
	.gpio_reset     = -EINVAL,
	.flags		= SMSC911X_USE_32BIT | SMSC911X_SAVE_MAC_ADDRESS,
};

static struct regulator_consumer_supply cm_t35_smsc911x_supplies[] = {
	REGULATOR_SUPPLY("vddvario", "smsc911x.0"),
	REGULATOR_SUPPLY("vdd33a", "smsc911x.0"),
};

static struct regulator_consumer_supply sb_t35_smsc911x_supplies[] = {
	REGULATOR_SUPPLY("vddvario", "smsc911x.1"),
	REGULATOR_SUPPLY("vdd33a", "smsc911x.1"),
};

static void __init cm_t35_init_ethernet(void)
{
	regulator_register_fixed(0, cm_t35_smsc911x_supplies,
				 ARRAY_SIZE(cm_t35_smsc911x_supplies));
	regulator_register_fixed(1, sb_t35_smsc911x_supplies,
				 ARRAY_SIZE(sb_t35_smsc911x_supplies));

	gpmc_smsc911x_init(&cm_t35_smsc911x_cfg);
	gpmc_smsc911x_init(&sb_t35_smsc911x_cfg);
}
#else
static inline void __init cm_t35_init_ethernet(void) { return; }
#endif

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#include <linux/leds.h>

static struct gpio_led cm_t35_leds[] = {
	[0] = {
		.gpio			= 186,
		.name			= "cm-t35:green",
		.default_trigger	= "heartbeat",
		.active_low		= 0,
	},
};

static struct gpio_led_platform_data cm_t35_led_pdata = {
	.num_leds	= ARRAY_SIZE(cm_t35_leds),
	.leds		= cm_t35_leds,
};

static struct platform_device cm_t35_led_device = {
	.name		= "leds-gpio",
	.id		= -1,
	.dev		= {
		.platform_data	= &cm_t35_led_pdata,
	},
};

static void __init cm_t35_init_led(void)
{
	platform_device_register(&cm_t35_led_device);
}
#else
static inline void cm_t35_init_led(void) {}
#endif

#if defined(CONFIG_MTD_NAND_OMAP2) || defined(CONFIG_MTD_NAND_OMAP2_MODULE)
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

static struct mtd_partition cm_t35_nand_partitions[] = {
	{
		.name           = "xloader",
		.offset         = 0,			/* Offset = 0x00000 */
		.size           = 4 * NAND_BLOCK_SIZE,
		.mask_flags     = MTD_WRITEABLE
	},
	{
		.name           = "uboot",
		.offset         = MTDPART_OFS_APPEND,	/* Offset = 0x80000 */
		.size           = 15 * NAND_BLOCK_SIZE,
	},
	{
		.name           = "uboot environment",
		.offset         = MTDPART_OFS_APPEND,	/* Offset = 0x260000 */
		.size           = 2 * NAND_BLOCK_SIZE,
	},
	{
		.name           = "linux",
		.offset         = MTDPART_OFS_APPEND,	/* Offset = 0x2A0000 */
		.size           = 32 * NAND_BLOCK_SIZE,
	},
	{
		.name           = "rootfs",
		.offset         = MTDPART_OFS_APPEND,	/* Offset = 0x6A0000 */
		.size           = MTDPART_SIZ_FULL,
	},
};

static struct omap_nand_platform_data cm_t35_nand_data = {
	.parts			= cm_t35_nand_partitions,
	.nr_parts		= ARRAY_SIZE(cm_t35_nand_partitions),
	.cs			= 0,
};

static void __init cm_t35_init_nand(void)
{
	if (gpmc_nand_init(&cm_t35_nand_data, NULL) < 0)
		pr_err("CM-T35: Unable to register NAND device\n");
}
#else
static inline void cm_t35_init_nand(void) {}
#endif

#define CM_T35_LCD_EN_GPIO 157
#define CM_T35_LCD_BL_GPIO 58
#define CM_T35_DVI_EN_GPIO 54

static const struct display_timing cm_t35_lcd_videomode = {
	.pixelclock	= { 0, 26000000, 0 },

	.hactive = { 0, 480, 0 },
	.hfront_porch = { 0, 104, 0 },
	.hback_porch = { 0, 8, 0 },
	.hsync_len = { 0, 8, 0 },

	.vactive = { 0, 640, 0 },
	.vfront_porch = { 0, 4, 0 },
	.vback_porch = { 0, 2, 0 },
	.vsync_len = { 0, 2, 0 },

	.flags = DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW |
		DISPLAY_FLAGS_DE_HIGH | DISPLAY_FLAGS_PIXDATA_NEGEDGE,
};

static struct panel_dpi_platform_data cm_t35_lcd_pdata = {
	.name                   = "lcd",
	.source                 = "dpi.0",

	.data_lines		= 18,

	.display_timing		= &cm_t35_lcd_videomode,

	.enable_gpio		= -1,
	.backlight_gpio		= CM_T35_LCD_BL_GPIO,
};

static struct platform_device cm_t35_lcd_device = {
	.name                   = "panel-dpi",
	.id                     = 0,
	.dev.platform_data      = &cm_t35_lcd_pdata,
};

static struct connector_dvi_platform_data cm_t35_dvi_connector_pdata = {
	.name                   = "dvi",
	.source                 = "tfp410.0",
	.i2c_bus_num            = -1,
};

static struct platform_device cm_t35_dvi_connector_device = {
	.name                   = "connector-dvi",
	.id                     = 0,
	.dev.platform_data      = &cm_t35_dvi_connector_pdata,
};

static struct encoder_tfp410_platform_data cm_t35_tfp410_pdata = {
	.name                   = "tfp410.0",
	.source                 = "dpi.0",
	.data_lines             = 24,
	.power_down_gpio        = CM_T35_DVI_EN_GPIO,
};

static struct platform_device cm_t35_tfp410_device = {
	.name                   = "tfp410",
	.id                     = 0,
	.dev.platform_data      = &cm_t35_tfp410_pdata,
};

static struct connector_atv_platform_data cm_t35_tv_pdata = {
	.name = "tv",
	.source = "venc.0",
	.connector_type = OMAP_DSS_VENC_TYPE_SVIDEO,
	.invert_polarity = false,
};

static struct platform_device cm_t35_tv_connector_device = {
	.name                   = "connector-analog-tv",
	.id                     = 0,
	.dev.platform_data      = &cm_t35_tv_pdata,
};

static struct omap_dss_board_info cm_t35_dss_data = {
	.default_display_name = "dvi",
};

static struct omap2_mcspi_device_config tdo24m_mcspi_config = {
	.turbo_mode	= 0,
};

static struct tdo24m_platform_data tdo24m_config = {
	.model = TDO35S,
};

static struct spi_board_info cm_t35_lcd_spi_board_info[] __initdata = {
	{
		.modalias		= "tdo24m",
		.bus_num		= 4,
		.chip_select		= 0,
		.max_speed_hz		= 1000000,
		.controller_data	= &tdo24m_mcspi_config,
		.platform_data		= &tdo24m_config,
	},
};

static void __init cm_t35_init_display(void)
{
	int err;

	spi_register_board_info(cm_t35_lcd_spi_board_info,
				ARRAY_SIZE(cm_t35_lcd_spi_board_info));


	err = gpio_request_one(CM_T35_LCD_EN_GPIO, GPIOF_OUT_INIT_LOW,
			"lcd bl enable");
	if (err) {
		pr_err("CM-T35: failed to request LCD EN GPIO\n");
		return;
	}

	msleep(50);
	gpio_set_value(CM_T35_LCD_EN_GPIO, 1);

	err = omap_display_init(&cm_t35_dss_data);
	if (err) {
		pr_err("CM-T35: failed to register DSS device\n");
		gpio_free(CM_T35_LCD_EN_GPIO);
	}

	platform_device_register(&cm_t35_tfp410_device);
	platform_device_register(&cm_t35_dvi_connector_device);
	platform_device_register(&cm_t35_lcd_device);
	platform_device_register(&cm_t35_tv_connector_device);
}

static struct regulator_consumer_supply cm_t35_vmmc1_supply[] = {
	REGULATOR_SUPPLY("vmmc", "omap_hsmmc.0"),
};

static struct regulator_consumer_supply cm_t35_vsim_supply[] = {
	REGULATOR_SUPPLY("vmmc_aux", "omap_hsmmc.0"),
};

static struct regulator_consumer_supply cm_t35_vio_supplies[] = {
	REGULATOR_SUPPLY("vcc", "spi1.0"),
	REGULATOR_SUPPLY("vdds_dsi", "omapdss"),
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dpi.0"),
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dsi.0"),
};

/* VMMC1 for MMC1 pins CMD, CLK, DAT0..DAT3 (20 mA, plus card == max 220 mA) */
static struct regulator_init_data cm_t35_vmmc1 = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 3150000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= ARRAY_SIZE(cm_t35_vmmc1_supply),
	.consumer_supplies	= cm_t35_vmmc1_supply,
};

/* VSIM for MMC1 pins DAT4..DAT7 (2 mA, plus card == max 50 mA) */
static struct regulator_init_data cm_t35_vsim = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 3000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= ARRAY_SIZE(cm_t35_vsim_supply),
	.consumer_supplies	= cm_t35_vsim_supply,
};

static struct regulator_init_data cm_t35_vio = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE,
	},
	.num_consumer_supplies	= ARRAY_SIZE(cm_t35_vio_supplies),
	.consumer_supplies	= cm_t35_vio_supplies,
};

static uint32_t cm_t35_keymap[] = {
	KEY(0, 0, KEY_A),	KEY(0, 1, KEY_B),	KEY(0, 2, KEY_LEFT),
	KEY(1, 0, KEY_UP),	KEY(1, 1, KEY_ENTER),	KEY(1, 2, KEY_DOWN),
	KEY(2, 0, KEY_RIGHT),	KEY(2, 1, KEY_C),	KEY(2, 2, KEY_D),
};

static struct matrix_keymap_data cm_t35_keymap_data = {
	.keymap			= cm_t35_keymap,
	.keymap_size		= ARRAY_SIZE(cm_t35_keymap),
};

static struct twl4030_keypad_data cm_t35_kp_data = {
	.keymap_data	= &cm_t35_keymap_data,
	.rows		= 3,
	.cols		= 3,
	.rep		= 1,
};

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.deferred	= true,
	},
	{
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA,
		.transceiver	= 1,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.ocr_mask	= 0x00100000,	/* 3.3V */
	},
	{}	/* Terminator */
};

static struct usbhs_phy_data phy_data[] __initdata = {
	{
		.port = 1,
		.reset_gpio = OMAP_MAX_GPIO_LINES + 6,
		.vcc_gpio = -EINVAL,
	},
	{
		.port = 2,
		.reset_gpio = OMAP_MAX_GPIO_LINES + 7,
		.vcc_gpio = -EINVAL,
	},
};

static struct usbhs_omap_platform_data usbhs_bdata __initdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
};

static void  __init cm_t35_init_usbh(void)
{
	int err;

	err = gpio_request_one(SB_T35_USB_HUB_RESET_GPIO,
			       GPIOF_OUT_INIT_LOW, "usb hub rst");
	if (err) {
		pr_err("SB-T35: usb hub rst gpio request failed: %d\n", err);
	} else {
		udelay(10);
		gpio_set_value(SB_T35_USB_HUB_RESET_GPIO, 1);
		msleep(1);
	}

	usbhs_init_phys(phy_data, ARRAY_SIZE(phy_data));
	usbhs_init(&usbhs_bdata);
}

static int cm_t35_twl_gpio_setup(struct device *dev, unsigned gpio,
				 unsigned ngpio)
{
	int wlan_rst = gpio + 2;

	if (gpio_request_one(wlan_rst, GPIOF_OUT_INIT_HIGH, "WLAN RST") == 0) {
		gpio_export(wlan_rst, 0);
		udelay(10);
		gpio_set_value_cansleep(wlan_rst, 0);
		udelay(10);
		gpio_set_value_cansleep(wlan_rst, 1);
	} else {
		pr_err("CM-T35: could not obtain gpio for WiFi reset\n");
	}

	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
	mmc[0].gpio_cd = gpio + 0;
	omap_hsmmc_late_init(mmc);

	return 0;
}

static struct twl4030_gpio_platform_data cm_t35_gpio_data = {
	.setup          = cm_t35_twl_gpio_setup,
};

static struct twl4030_power_data cm_t35_power_data = {
	.use_poweroff	= true,
};

static struct twl4030_platform_data cm_t35_twldata = {
	/* platform_data for children goes here */
	.keypad		= &cm_t35_kp_data,
	.gpio		= &cm_t35_gpio_data,
	.vmmc1		= &cm_t35_vmmc1,
	.vsim		= &cm_t35_vsim,
	.vio		= &cm_t35_vio,
	.power		= &cm_t35_power_data,
};

#if defined(CONFIG_VIDEO_OMAP3) || defined(CONFIG_VIDEO_OMAP3_MODULE)
#include <media/omap3isp.h>
#include "devices.h"

static struct isp_platform_subdev cm_t35_isp_subdevs[] = {
	{
		.board_info = &(struct i2c_board_info){
			I2C_BOARD_INFO("mt9t001", 0x5d)
		},
		.i2c_adapter_id = 3,
		.bus = &(struct isp_bus_cfg){
			.interface = ISP_INTERFACE_PARALLEL,
			.bus = {
				.parallel = {
					.clk_pol = 1,
				},
			},
		},
	},
	{
		.board_info = &(struct i2c_board_info){
			I2C_BOARD_INFO("tvp5150", 0x5c),
		},
		.i2c_adapter_id = 3,
		.bus = &(struct isp_bus_cfg){
			.interface = ISP_INTERFACE_PARALLEL,
			.bus = {
				.parallel = {
					.clk_pol = 0,
				},
			},
		},
	},
	{ 0 },
};

static struct isp_platform_data cm_t35_isp_pdata = {
	.subdevs = cm_t35_isp_subdevs,
};

static struct regulator_consumer_supply cm_t35_camera_supplies[] = {
	REGULATOR_SUPPLY("vaa", "3-005d"),
	REGULATOR_SUPPLY("vdd", "3-005d"),
};

static void __init cm_t35_init_camera(void)
{
	struct clk *clk;

	clk = clk_register_fixed_rate(NULL, "mt9t001-clkin", NULL, CLK_IS_ROOT,
				      48000000);
	clk_register_clkdev(clk, NULL, "3-005d");

	regulator_register_fixed(2, cm_t35_camera_supplies,
				 ARRAY_SIZE(cm_t35_camera_supplies));

	if (omap3_init_camera(&cm_t35_isp_pdata) < 0)
		pr_warn("CM-T3x: Failed registering camera device!\n");
}

#else
static inline void cm_t35_init_camera(void) {}
#endif /* CONFIG_VIDEO_OMAP3 */

static void __init cm_t35_init_i2c(void)
{
	omap3_pmic_get_config(&cm_t35_twldata, TWL_COMMON_PDATA_USB,
			      TWL_COMMON_REGULATOR_VDAC |
			      TWL_COMMON_PDATA_AUDIO);

	omap3_pmic_init("tps65930", &cm_t35_twldata);

	omap_register_i2c_bus(3, 400, NULL, 0);
}

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	/* nCS and IRQ for CM-T35 ethernet */
	OMAP3_MUX(GPMC_NCS5, OMAP_MUX_MODE0),
	OMAP3_MUX(UART3_CTS_RCTX, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),

	/* nCS and IRQ for SB-T35 ethernet */
	OMAP3_MUX(GPMC_NCS4, OMAP_MUX_MODE0),
	OMAP3_MUX(GPMC_WAIT3, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),

	/* PENDOWN GPIO */
	OMAP3_MUX(GPMC_NCS6, OMAP_MUX_MODE4 | OMAP_PIN_INPUT),

	/* mUSB */
	OMAP3_MUX(HSUSB0_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_STP, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(HSUSB0_DIR, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_NXT, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_DATA0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_DATA1, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_DATA2, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_DATA3, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_DATA4, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_DATA5, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_DATA6, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(HSUSB0_DATA7, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),

	/* MMC 2 */
	OMAP3_MUX(SDMMC2_DAT4, OMAP_MUX_MODE1 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(SDMMC2_DAT5, OMAP_MUX_MODE1 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(SDMMC2_DAT6, OMAP_MUX_MODE1 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(SDMMC2_DAT7, OMAP_MUX_MODE1 | OMAP_PIN_INPUT),

	/* McSPI 1 */
	OMAP3_MUX(MCSPI1_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCSPI1_SIMO, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCSPI1_SOMI, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCSPI1_CS0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),

	/* McSPI 4 */
	OMAP3_MUX(MCBSP1_CLKR, OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP1_DX, OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP1_DR, OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP1_FSX, OMAP_MUX_MODE1 | OMAP_PIN_INPUT_PULLUP),

	/* McBSP 2 */
	OMAP3_MUX(MCBSP2_FSX, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP2_CLKX, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP2_DR, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP2_DX, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),

	/* serial ports */
	OMAP3_MUX(MCBSP3_CLKX, OMAP_MUX_MODE1 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(MCBSP3_FSX, OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(UART1_TX, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(UART1_RX, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),

	/* common DSS */
	OMAP3_MUX(DSS_PCLK, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_HSYNC, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_VSYNC, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_ACBIAS, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA6, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA7, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA8, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA9, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA10, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA11, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA12, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA13, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA14, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA15, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA16, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(DSS_DATA17, OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),

	/* Camera */
	OMAP3_MUX(CAM_HS, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_VS, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_XCLKA, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_PCLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_FLD, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D1, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D2, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D3, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D4, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D5, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D6, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D7, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(CAM_D8, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(CAM_D9, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(CAM_STROBE, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),

	OMAP3_MUX(CAM_D10, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(CAM_D11, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),

	/* display controls */
	OMAP3_MUX(MCBSP1_FSR, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(GPMC_NCS7, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(GPMC_NCS3, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),

	/* TPS IRQ */
	OMAP3_MUX(SYS_NIRQ, OMAP_MUX_MODE0 | OMAP_WAKEUP_EN | \
		  OMAP_PIN_INPUT_PULLUP),

	{ .reg_offset = OMAP_MUX_TERMINATOR },
};

static void __init cm_t3x_common_dss_mux_init(int mux_mode)
{
	omap_mux_init_signal("dss_data18", mux_mode);
	omap_mux_init_signal("dss_data19", mux_mode);
	omap_mux_init_signal("dss_data20", mux_mode);
	omap_mux_init_signal("dss_data21", mux_mode);
	omap_mux_init_signal("dss_data22", mux_mode);
	omap_mux_init_signal("dss_data23", mux_mode);
}

static void __init cm_t35_init_mux(void)
{
	int mux_mode = OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT;

	omap_mux_init_signal("dss_data0.dss_data0", mux_mode);
	omap_mux_init_signal("dss_data1.dss_data1", mux_mode);
	omap_mux_init_signal("dss_data2.dss_data2", mux_mode);
	omap_mux_init_signal("dss_data3.dss_data3", mux_mode);
	omap_mux_init_signal("dss_data4.dss_data4", mux_mode);
	omap_mux_init_signal("dss_data5.dss_data5", mux_mode);
	cm_t3x_common_dss_mux_init(mux_mode);
}

static void __init cm_t3730_init_mux(void)
{
	int mux_mode = OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT;

	omap_mux_init_signal("sys_boot0", mux_mode);
	omap_mux_init_signal("sys_boot1", mux_mode);
	omap_mux_init_signal("sys_boot3", mux_mode);
	omap_mux_init_signal("sys_boot4", mux_mode);
	omap_mux_init_signal("sys_boot5", mux_mode);
	omap_mux_init_signal("sys_boot6", mux_mode);
	cm_t3x_common_dss_mux_init(mux_mode);
}
#else
static inline void cm_t35_init_mux(void) {}
static inline void cm_t3730_init_mux(void) {}
#endif

static void __init cm_t3x_common_init(void)
{
	omap3_mux_init(board_mux, OMAP_PACKAGE_CUS);
	omap_serial_init();
	omap_sdrc_init(mt46h32m32lf6_sdrc_params,
			     mt46h32m32lf6_sdrc_params);
	omap_hsmmc_init(mmc);
	cm_t35_init_i2c();
	omap_ads7846_init(1, CM_T35_GPIO_PENDOWN, 0, NULL);
	cm_t35_init_ethernet();
	cm_t35_init_led();
	cm_t35_init_display();
	omap_twl4030_audio_init("cm-t3x", NULL);

	usb_bind_phy("musb-hdrc.0.auto", 0, "twl4030_usb");
	usb_musb_init(NULL);
	cm_t35_init_usbh();
	cm_t35_init_camera();
}

static void __init cm_t35_init(void)
{
	cm_t3x_common_init();
	cm_t35_init_mux();
	cm_t35_init_nand();
}

static void __init cm_t3730_init(void)
{
	cm_t3x_common_init();
	cm_t3730_init_mux();
}

MACHINE_START(CM_T35, "Compulab CM-T35")
	.atag_offset	= 0x100,
	.reserve	= omap_reserve,
	.map_io		= omap3_map_io,
	.init_early	= omap35xx_init_early,
	.init_irq	= omap3_init_irq,
	.init_machine	= cm_t35_init,
	.init_late	= omap35xx_init_late,
	.init_time	= omap3_sync32k_timer_init,
	.restart	= omap3xxx_restart,
MACHINE_END

MACHINE_START(CM_T3730, "Compulab CM-T3730")
	.atag_offset	= 0x100,
	.reserve	= omap_reserve,
	.map_io		= omap3_map_io,
	.init_early	= omap3630_init_early,
	.init_irq	= omap3_init_irq,
	.init_machine	= cm_t3730_init,
	.init_late     = omap3630_init_late,
	.init_time	= omap3_sync32k_timer_init,
	.restart	= omap3xxx_restart,
MACHINE_END
