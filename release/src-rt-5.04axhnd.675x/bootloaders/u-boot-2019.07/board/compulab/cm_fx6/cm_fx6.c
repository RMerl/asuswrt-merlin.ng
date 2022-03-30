// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for Compulab CM-FX6 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Nikita Kiryanov <nikita@compulab.co.il>
 */

#include <common.h>
#include <ahci.h>
#include <dm.h>
#include <dwc_ahsata.h>
#include <environment.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <mtd_node.h>
#include <netdev.h>
#include <errno.h>
#include <usb.h>
#include <fdt_support.h>
#include <sata.h>
#include <splash.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/sata.h>
#include <asm/mach-imx/video.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm/platform_data/serial_mxc.h>
#include <dm/device-internal.h>
#include <jffs2/load_kernel.h>
#include "common.h"
#include "../common/eeprom.h"
#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPLASH_SCREEN
static struct splash_location cm_fx6_splash_locations[] = {
	{
		.name = "sf",
		.storage = SPLASH_STORAGE_SF,
		.flags = SPLASH_STORAGE_RAW,
		.offset = 0x100000,
	},
	{
		.name = "mmc_fs",
		.storage = SPLASH_STORAGE_MMC,
		.flags = SPLASH_STORAGE_FS,
		.devpart = "2:1",
	},
	{
		.name = "usb_fs",
		.storage = SPLASH_STORAGE_USB,
		.flags = SPLASH_STORAGE_FS,
		.devpart = "0:1",
	},
	{
		.name = "sata_fs",
		.storage = SPLASH_STORAGE_SATA,
		.flags = SPLASH_STORAGE_FS,
		.devpart = "0:1",
	},
};

int splash_screen_prepare(void)
{
	return splash_source_load(cm_fx6_splash_locations,
				  ARRAY_SIZE(cm_fx6_splash_locations));
}
#endif

#ifdef CONFIG_IMX_HDMI
static void cm_fx6_enable_hdmi(struct display_info_t const *dev)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	imx_setup_hdmi();
	setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_IPU1_IPU_DI0_MASK);
	imx_enable_hdmi_phy();
}

static struct display_info_t preset_hdmi_1024X768 = {
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.enable	= cm_fx6_enable_hdmi,
	.mode	= {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 40385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED,
	}
};

static void cm_fx6_setup_display(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	enable_ipu_clock();
	clrbits_le32(&iomuxc_regs->gpr[3], MXC_CCM_CCGR3_IPU1_IPU_DI0_MASK);
}

int board_video_skip(void)
{
	int ret;
	struct display_info_t *preset;
	char const *panel = env_get("displaytype");

	if (!panel) /* Also accept panel for backward compatibility */
		panel = env_get("panel");

	if (!panel)
		return -ENOENT;

	if (!strcmp(panel, "HDMI"))
		preset = &preset_hdmi_1024X768;
	else
		return -EINVAL;

	ret = ipuv3_fb_init(&preset->mode, 0, preset->pixfmt);
	if (ret) {
		printf("Can't init display %s: %d\n", preset->mode.name, ret);
		return ret;
	}

	preset->enable(preset);
	printf("Display: %s (%ux%u)\n", preset->mode.name, preset->mode.xres,
	       preset->mode.yres);

	return 0;
}
#else
static inline void cm_fx6_setup_display(void) {}
#endif /* CONFIG_VIDEO_IPUV3 */

#ifdef CONFIG_DWC_AHSATA
static int cm_fx6_issd_gpios[] = {
	/* The order of the GPIOs in the array is important! */
	CM_FX6_SATA_LDO_EN,
	CM_FX6_SATA_PHY_SLP,
	CM_FX6_SATA_NRSTDLY,
	CM_FX6_SATA_PWREN,
	CM_FX6_SATA_NSTANDBY1,
	CM_FX6_SATA_NSTANDBY2,
};

static void cm_fx6_sata_power(int on)
{
	int i;

	if (!on) { /* tell the iSSD that the power will be removed */
		gpio_direction_output(CM_FX6_SATA_PWLOSS_INT, 1);
		mdelay(10);
	}

	for (i = 0; i < ARRAY_SIZE(cm_fx6_issd_gpios); i++) {
		gpio_direction_output(cm_fx6_issd_gpios[i], on);
		udelay(100);
	}

	if (!on) /* for compatibility lower the power loss interrupt */
		gpio_direction_output(CM_FX6_SATA_PWLOSS_INT, 0);
}

static iomux_v3_cfg_t const sata_pads[] = {
	/* SATA PWR */
	IOMUX_PADS(PAD_ENET_TX_EN__GPIO1_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A22__GPIO2_IO16    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D20__GPIO3_IO20    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A25__GPIO5_IO02    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	/* SATA CTRL */
	IOMUX_PADS(PAD_ENET_TXD0__GPIO1_IO30  | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D23__GPIO3_IO23    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D29__GPIO3_IO29    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A23__GPIO6_IO06    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_BCLK__GPIO6_IO31   | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static int cm_fx6_setup_issd(void)
{
	int ret, i;

	SETUP_IOMUX_PADS(sata_pads);

	for (i = 0; i < ARRAY_SIZE(cm_fx6_issd_gpios); i++) {
		ret = gpio_request(cm_fx6_issd_gpios[i], "sata");
		if (ret)
			return ret;
	}

	ret = gpio_request(CM_FX6_SATA_PWLOSS_INT, "sata_pwloss_int");
	if (ret)
		return ret;

	return 0;
}

#define CM_FX6_SATA_INIT_RETRIES	10

#else
static int cm_fx6_setup_issd(void) { return 0; }
#endif

#ifdef CONFIG_SYS_I2C_MXC
#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
			PAD_CTL_DSE_40ohm | PAD_CTL_HYS | \
			PAD_CTL_ODE | PAD_CTL_SRE_FAST)

I2C_PADS(i2c0_pads,
	 PAD_EIM_D21__I2C1_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_EIM_D21__GPIO3_IO21 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(3, 21),
	 PAD_EIM_D28__I2C1_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_EIM_D28__GPIO3_IO28 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(3, 28));

I2C_PADS(i2c1_pads,
	 PAD_KEY_COL3__I2C2_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_KEY_COL3__GPIO4_IO12 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(4, 12),
	 PAD_KEY_ROW3__I2C2_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_KEY_ROW3__GPIO4_IO13 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(4, 13));

I2C_PADS(i2c2_pads,
	 PAD_GPIO_3__I2C3_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_GPIO_3__GPIO1_IO03 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(1, 3),
	 PAD_GPIO_6__I2C3_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_GPIO_6__GPIO1_IO06 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(1, 6));


static int cm_fx6_setup_one_i2c(int busnum, struct i2c_pads_info *pads)
{
	int ret;

	ret = setup_i2c(busnum, CONFIG_SYS_I2C_SPEED, 0x7f, pads);
	if (ret)
		printf("Warning: I2C%d setup failed: %d\n", busnum, ret);

	return ret;
}

static int cm_fx6_setup_i2c(void)
{
	int ret = 0, err;

	/* i2c<x>_pads are wierd macro variables; we can't use an array */
	err = cm_fx6_setup_one_i2c(0, I2C_PADS_INFO(i2c0_pads));
	if (err)
		ret = err;
	err = cm_fx6_setup_one_i2c(1, I2C_PADS_INFO(i2c1_pads));
	if (err)
		ret = err;
	err = cm_fx6_setup_one_i2c(2, I2C_PADS_INFO(i2c2_pads));
	if (err)
		ret = err;

	return ret;
}
#else
static int cm_fx6_setup_i2c(void) { return 0; }
#endif

#ifdef CONFIG_USB_EHCI_MX6
#define WEAK_PULLDOWN	(PAD_CTL_PUS_100K_DOWN |		\
			PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
			PAD_CTL_HYS | PAD_CTL_SRE_SLOW)
#define MX6_USBNC_BASEADDR	0x2184800
#define USBNC_USB_H1_PWR_POL	(1 << 9)

static int cm_fx6_setup_usb_host(void)
{
	int err;

	err = gpio_request(CM_FX6_USB_HUB_RST, "usb hub rst");
	if (err)
		return err;

	SETUP_IOMUX_PAD(PAD_GPIO_0__USB_H1_PWR | MUX_PAD_CTRL(NO_PAD_CTRL));
	SETUP_IOMUX_PAD(PAD_SD3_RST__GPIO7_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL));

	return 0;
}

static int cm_fx6_setup_usb_otg(void)
{
	int err;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	err = gpio_request(SB_FX6_USB_OTG_PWR, "usb-pwr");
	if (err) {
		printf("USB OTG pwr gpio request failed: %d\n", err);
		return err;
	}

	SETUP_IOMUX_PAD(PAD_EIM_D22__GPIO3_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL));
	SETUP_IOMUX_PAD(PAD_ENET_RX_ER__USB_OTG_ID |
						MUX_PAD_CTRL(WEAK_PULLDOWN));
	clrbits_le32(&iomux->gpr[1], IOMUXC_GPR1_OTG_ID_MASK);
	/* disable ext. charger detect, or it'll affect signal quality at dp. */
	return gpio_direction_output(SB_FX6_USB_OTG_PWR, 0);
}

int board_usb_phy_mode(int port)
{
	return USB_INIT_HOST;
}

int board_ehci_hcd_init(int port)
{
	int ret;
	u32 *usbnc_usb_uh1_ctrl = (u32 *)(MX6_USBNC_BASEADDR + 4);

	/* Only 1 host controller in use. port 0 is OTG & needs no attention */
	if (port != 1)
		return 0;

	/* Set PWR polarity to match power switch's enable polarity */
	setbits_le32(usbnc_usb_uh1_ctrl, USBNC_USB_H1_PWR_POL);
	ret = gpio_direction_output(CM_FX6_USB_HUB_RST, 0);
	if (ret)
		return ret;

	udelay(10);
	ret = gpio_direction_output(CM_FX6_USB_HUB_RST, 1);
	if (ret)
		return ret;

	mdelay(1);

	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port == 0)
		return gpio_direction_output(SB_FX6_USB_OTG_PWR, on);

	return 0;
}
#else
static int cm_fx6_setup_usb_otg(void) { return 0; }
static int cm_fx6_setup_usb_host(void) { return 0; }
#endif

#ifdef CONFIG_FEC_MXC
#define ENET_PAD_CTRL		(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

static int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short val;

	/* Ar8031 phy SmartEEE feature cause link status generates glitch,
	 * which cause ethernet link down/up issue, so disable SmartEEE
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x3);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x805d);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4003);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= ~(0x1 << 8);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		return phydev->drv->config(phydev);

	return 0;
}

static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC   | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_0__CCM_CLKO1    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_3__CCM_CLKO2    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | MUX_PAD_CTRL(0x84)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK  |
						MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL |
						MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL |
						MUX_PAD_CTRL(ENET_PAD_CTRL)),
};

static int handle_mac_address(char *env_var, uint eeprom_bus)
{
	unsigned char enetaddr[6];
	int rc;

	rc = eth_env_get_enetaddr(env_var, enetaddr);
	if (rc)
		return 0;

	rc = cl_eeprom_read_mac_addr(enetaddr, eeprom_bus);
	if (rc)
		return rc;

	if (!is_valid_ethaddr(enetaddr))
		return -1;

	return eth_env_set_enetaddr(env_var, enetaddr);
}

#define SB_FX6_I2C_EEPROM_BUS	0
#define NO_MAC_ADDR		"No MAC address found for %s\n"
int board_eth_init(bd_t *bis)
{
	int err;

	if (handle_mac_address("ethaddr", CONFIG_SYS_I2C_EEPROM_BUS))
		printf(NO_MAC_ADDR, "primary NIC");

	if (handle_mac_address("eth1addr", SB_FX6_I2C_EEPROM_BUS))
		printf(NO_MAC_ADDR, "secondary NIC");

	SETUP_IOMUX_PADS(enet_pads);
	/* phy reset */
	err = gpio_request(CM_FX6_ENET_NRST, "enet_nrst");
	if (err)
		printf("Etnernet NRST gpio request failed: %d\n", err);
	gpio_direction_output(CM_FX6_ENET_NRST, 0);
	udelay(500);
	gpio_set_value(CM_FX6_ENET_NRST, 1);
	enable_enet_clk(1);
	return cpu_eth_init(bis);
}
#endif

#ifdef CONFIG_NAND_MXS
static iomux_v3_cfg_t const nand_pads[] = {
	IOMUX_PADS(PAD_NANDF_CLE__NAND_CLE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_ALE__NAND_ALE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS0__NAND_CE0_B   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_RB0__NAND_READY_B | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D0__NAND_DATA00   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D1__NAND_DATA01   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D2__NAND_DATA02   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D3__NAND_DATA03   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D4__NAND_DATA04   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D5__NAND_DATA05   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__NAND_DATA06   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D7__NAND_DATA07   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__NAND_RE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__NAND_WE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void cm_fx6_setup_gpmi_nand(void)
{
	SETUP_IOMUX_PADS(nand_pads);
	/* Enable clock roots */
	enable_usdhc_clk(1, 3);
	enable_usdhc_clk(1, 4);

	setup_gpmi_io_clk(MXC_CCM_CS2CDR_ENFC_CLK_PODF(0xf) |
			  MXC_CCM_CS2CDR_ENFC_CLK_PRED(1)   |
			  MXC_CCM_CS2CDR_ENFC_CLK_SEL(0));
}
#else
static void cm_fx6_setup_gpmi_nand(void) {}
#endif

#ifdef CONFIG_MXC_SPI
int cm_fx6_setup_ecspi(void)
{
	cm_fx6_set_ecspi_iomux();
	return gpio_request(CM_FX6_ECSPI_BUS0_CS0, "ecspi_bus0_cs0");
}
#else
int cm_fx6_setup_ecspi(void) { return 0; }
#endif

#ifdef CONFIG_OF_BOARD_SETUP
#define USDHC3_PATH	"/soc/aips-bus@02100000/usdhc@02198000/"

static const struct node_info nodes[] = {
	/*
	 * Both entries target the same flash chip. The st,m25p compatible
	 * is used in the vendor device trees, while upstream uses (the
	 * documented) jedec,spi-nor compatible.
	 */
	{ "st,m25p",	MTD_DEV_TYPE_NOR,	},
	{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR,	},
};

int ft_board_setup(void *blob, bd_t *bd)
{
	u32 baseboard_rev;
	int nodeoffset;
	uint8_t enetaddr[6];
	char baseboard_name[16];
	int err;

	fdt_shrink_to_minimum(blob, 0); /* Make room for new properties */

	/* MAC addr */
	if (eth_env_get_enetaddr("ethaddr", enetaddr)) {
		fdt_find_and_setprop(blob,
				     "/soc/aips-bus@02100000/ethernet@02188000",
				     "local-mac-address", enetaddr, 6, 1);
	}

	if (eth_env_get_enetaddr("eth1addr", enetaddr)) {
		fdt_find_and_setprop(blob, "/eth@pcie", "local-mac-address",
				     enetaddr, 6, 1);
	}

	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));

	baseboard_rev = cl_eeprom_get_board_rev(0);
	err = cl_eeprom_get_product_name((uchar *)baseboard_name, 0);
	if (err || baseboard_rev == 0)
		return 0; /* Assume not an early revision SB-FX6m baseboard */

	if (!strncmp("SB-FX6m", baseboard_name, 7) && baseboard_rev <= 120) {
		nodeoffset = fdt_path_offset(blob, USDHC3_PATH);
		fdt_delprop(blob, nodeoffset, "cd-gpios");
		fdt_find_and_setprop(blob, USDHC3_PATH, "broken-cd",
				     NULL, 0, 1);
		fdt_find_and_setprop(blob, USDHC3_PATH, "keep-power-in-suspend",
				     NULL, 0, 1);
	}

	return 0;
}
#endif

int board_init(void)
{
	int ret;

	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	cm_fx6_setup_gpmi_nand();

	ret = cm_fx6_setup_ecspi();
	if (ret)
		printf("Warning: ECSPI setup failed: %d\n", ret);

	ret = cm_fx6_setup_usb_otg();
	if (ret)
		printf("Warning: USB OTG setup failed: %d\n", ret);

	ret = cm_fx6_setup_usb_host();
	if (ret)
		printf("Warning: USB host setup failed: %d\n", ret);

	/*
	 * cm-fx6 may have iSSD not assembled and in this case it has
	 * bypasses for a (m)SATA socket on the baseboard. The socketed
	 * device is not controlled by those GPIOs. So just print a warning
	 * if the setup fails.
	 */
	ret = cm_fx6_setup_issd();
	if (ret)
		printf("Warning: iSSD setup failed: %d\n", ret);

	/* Warn on failure but do not abort boot */
	ret = cm_fx6_setup_i2c();
	if (ret)
		printf("Warning: I2C setup failed: %d\n", ret);

	cm_fx6_setup_display();

	/* This should be done in the MMC driver when MX6 has a clock driver */
#ifdef CONFIG_FSL_ESDHC
	if (IS_ENABLED(CONFIG_BLK)) {
		int i;

		cm_fx6_set_usdhc_iomux();
		for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++)
			enable_usdhc_clk(1, i);
	}
#endif

	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	char baseboard_name[16];
	int err;

	if (is_mx6dq())
		env_set("board_rev", "MX6Q");
	else if (is_mx6dl())
		env_set("board_rev", "MX6DL");

	err = cl_eeprom_get_product_name((uchar *)baseboard_name, 0);
	if (err)
		return 0;

	if (!strncmp("SB-FX6m", baseboard_name, 7))
		env_set("board_name", "Utilite");
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: CM-FX6\n");
	return 0;
}

int misc_init_r(void)
{
	cl_print_pcb_info();

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;

	switch (gd->ram_size) {
	case 0x10000000: /* DDR_16BIT_256MB */
		gd->bd->bi_dram[0].size = 0x10000000;
		gd->bd->bi_dram[1].size = 0;
		break;
	case 0x20000000: /* DDR_32BIT_512MB */
		gd->bd->bi_dram[0].size = 0x20000000;
		gd->bd->bi_dram[1].size = 0;
		break;
	case 0x40000000:
		if (is_cpu_type(MXC_CPU_MX6SOLO)) { /* DDR_32BIT_1GB */
			gd->bd->bi_dram[0].size = 0x20000000;
			gd->bd->bi_dram[1].size = 0x20000000;
		} else { /* DDR_64BIT_1GB */
			gd->bd->bi_dram[0].size = 0x40000000;
			gd->bd->bi_dram[1].size = 0;
		}
		break;
	case 0x80000000: /* DDR_64BIT_2GB */
		gd->bd->bi_dram[0].size = 0x40000000;
		gd->bd->bi_dram[1].size = 0x40000000;
		break;
	case 0xEFF00000: /* DDR_64BIT_4GB */
		gd->bd->bi_dram[0].size = 0x70000000;
		gd->bd->bi_dram[1].size = 0x7FF00000;
		break;
	}

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	switch (gd->ram_size) {
	case 0x10000000:
	case 0x20000000:
	case 0x40000000:
	case 0x80000000:
		break;
	case 0xF0000000:
		gd->ram_size -= 0x100000;
		break;
	default:
		printf("ERROR: Unsupported DRAM size 0x%lx\n", gd->ram_size);
		return -1;
	}

	return 0;
}

u32 get_board_rev(void)
{
	return cl_eeprom_get_board_rev(CONFIG_SYS_I2C_EEPROM_BUS);
}

static struct mxc_serial_platdata cm_fx6_mxc_serial_plat = {
	.reg = (struct mxc_uart *)UART4_BASE,
};

U_BOOT_DEVICE(cm_fx6_serial) = {
	.name	= "serial_mxc",
	.platdata = &cm_fx6_mxc_serial_plat,
};

#if CONFIG_IS_ENABLED(AHCI)
static int sata_imx_probe(struct udevice *dev)
{
	int i, err;

	/* Make sure this gpio has logical 0 value */
	gpio_direction_output(CM_FX6_SATA_PWLOSS_INT, 0);
	udelay(100);
	cm_fx6_sata_power(1);

	for (i = 0; i < CM_FX6_SATA_INIT_RETRIES; i++) {
		err = setup_sata();
		if (err) {
			printf("SATA setup failed: %d\n", err);
			return err;
		}

		udelay(100);

		err = dwc_ahsata_probe(dev);
		if (!err)
			break;

		/* There is no device on the SATA port */
		if (sata_dm_port_status(0, 0) == 0)
			break;

		/* There's a device, but link not established. Retry */
		device_remove(dev, DM_REMOVE_NORMAL);
	}

	return 0;
}

static int sata_imx_remove(struct udevice *dev)
{
	cm_fx6_sata_power(0);
	mdelay(250);

	return 0;
}

struct ahci_ops sata_imx_ops = {
	.port_status = dwc_ahsata_port_status,
	.reset	= dwc_ahsata_bus_reset,
	.scan	= dwc_ahsata_scan,
};

static const struct udevice_id sata_imx_ids[] = {
	{ .compatible = "fsl,imx6q-ahci" },
	{ }
};

U_BOOT_DRIVER(sata_imx) = {
	.name		= "dwc_ahci",
	.id		= UCLASS_AHCI,
	.of_match	= sata_imx_ids,
	.ops		= &sata_imx_ops,
	.probe		= sata_imx_probe,
	.remove		= sata_imx_remove,  /* reset bus to stop it */
};
#endif /* AHCI */
