// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/sata.h>
#include <asm/mach-imx/spi.h>
#include <asm/mach-imx/video.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <dm.h>
#include <dm/platform_data/serial_mxc.h>
#include <environment.h>
#include <hwconfig.h>
#include <i2c.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <jffs2/load_kernel.h>
#include <linux/ctype.h>
#include <miiphy.h>
#include <mtd_node.h>
#include <netdev.h>
#include <pci.h>
#include <power/pmic.h>
#include <power/ltc3676_pmic.h>
#include <power/pfuze100_pmic.h>
#include <fdt_support.h>
#include <jffs2/load_kernel.h>
#include <spi_flash.h>

#include "gsc.h"
#include "common.h"

DECLARE_GLOBAL_DATA_PTR;


/*
 * EEPROM board info struct populated by read_eeprom so that we only have to
 * read it once.
 */
struct ventana_board_info ventana_info;

static int board_type;

/* ENET */
static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC    | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL |
		   MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK |
		   MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL |
		   MUX_PAD_CTRL(ENET_PAD_CTRL)),
	/* PHY nRST */
	IOMUX_PADS(PAD_ENET_TXD0__GPIO1_IO30 | DIO_PAD_CFG),
};

#ifdef CONFIG_CMD_NAND
static iomux_v3_cfg_t const nfc_pads[] = {
	IOMUX_PADS(PAD_NANDF_CLE__NAND_CLE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_ALE__NAND_ALE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_WP_B__NAND_WP_B   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_RB0__NAND_READY_B | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS0__NAND_CE0_B   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__NAND_RE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__NAND_WE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D0__NAND_DATA00   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D1__NAND_DATA01   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D2__NAND_DATA02   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D3__NAND_DATA03   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D4__NAND_DATA04   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D5__NAND_DATA05   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__NAND_DATA06   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D7__NAND_DATA07   | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	SETUP_IOMUX_PADS(nfc_pads);

	/* config gpmi and bch clock to 100 MHz */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3));

	/* enable gpmi and bch clock gating */
	setbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_OFFSET);

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

static void setup_iomux_enet(int gpio)
{
	SETUP_IOMUX_PADS(enet_pads);

	/* toggle PHY_RST# */
	gpio_request(gpio, "phy_rst#");
	gpio_direction_output(gpio, 0);
	mdelay(10);
	gpio_set_value(gpio, 1);
	mdelay(100);
}

#ifdef CONFIG_USB_EHCI_MX6
static iomux_v3_cfg_t const usb_pads[] = {
	IOMUX_PADS(PAD_GPIO_1__USB_OTG_ID   | DIO_PAD_CFG),
	IOMUX_PADS(PAD_KEY_COL4__USB_OTG_OC | DIO_PAD_CFG),
	/* OTG PWR */
	IOMUX_PADS(PAD_EIM_D22__GPIO3_IO22  | DIO_PAD_CFG),
};

int board_ehci_hcd_init(int port)
{
	int gpio;

	SETUP_IOMUX_PADS(usb_pads);

	/* Reset USB HUB */
	switch (board_type) {
	case GW53xx:
	case GW552x:
	case GW5906:
		gpio = (IMX_GPIO_NR(1, 9));
		break;
	case GW54proto:
	case GW54xx:
		gpio = (IMX_GPIO_NR(1, 16));
		break;
	default:
		return 0;
	}

	/* request and toggle hub rst */
	gpio_request(gpio, "usb_hub_rst#");
	gpio_direction_output(gpio, 0);
	mdelay(2);
	gpio_set_value(gpio, 1);

	return 0;
}

int board_ehci_power(int port, int on)
{
	/* enable OTG VBUS */
	if (!port && board_type < GW_UNKNOWN) {
		if (gpio_cfg[board_type].otgpwr_en)
			gpio_set_value(gpio_cfg[board_type].otgpwr_en, on);
	}
	return 0;
}
#endif /* CONFIG_USB_EHCI_MX6 */

#ifdef CONFIG_MXC_SPI
iomux_v3_cfg_t const ecspi1_pads[] = {
	/* SS1 */
	IOMUX_PADS(PAD_EIM_D19__GPIO3_IO19  | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
};

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(3, 19)) : -1;
}

static void setup_spi(void)
{
	gpio_request(IMX_GPIO_NR(3, 19), "spi_cs");
	gpio_direction_output(IMX_GPIO_NR(3, 19), 1);
	SETUP_IOMUX_PADS(ecspi1_pads);
}
#endif

/* configure eth0 PHY board-specific LED behavior */
int board_phy_config(struct phy_device *phydev)
{
	unsigned short val;

	/* Marvel 88E1510 */
	if (phydev->phy_id == 0x1410dd1) {
		/*
		 * Page 3, Register 16: LED[2:0] Function Control Register
		 * LED[0] (SPD:Amber) R16_3.3:0 to 0111: on-GbE link
		 * LED[1] (LNK:Green) R16_3.7:4 to 0001: on-link, blink-activity
		 */
		phy_write(phydev, MDIO_DEVAD_NONE, 22, 3);
		val = phy_read(phydev, MDIO_DEVAD_NONE, 16);
		val &= 0xff00;
		val |= 0x0017;
		phy_write(phydev, MDIO_DEVAD_NONE, 16, val);
		phy_write(phydev, MDIO_DEVAD_NONE, 22, 0);
	}

	/* TI DP83867 */
	else if (phydev->phy_id == 0x2000a231) {
		/* configure register 0x170 for ref CLKOUT */
		phy_write(phydev, MDIO_DEVAD_NONE, 13, 0x001f);
		phy_write(phydev, MDIO_DEVAD_NONE, 14, 0x0170);
		phy_write(phydev, MDIO_DEVAD_NONE, 13, 0x401f);
		val = phy_read(phydev, MDIO_DEVAD_NONE, 14);
		val &= ~0x1f00;
		val |= 0x0b00; /* chD tx clock*/
		phy_write(phydev, MDIO_DEVAD_NONE, 14, val);
	}

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#ifdef CONFIG_MV88E61XX_SWITCH
int mv88e61xx_hw_reset(struct phy_device *phydev)
{
	struct mii_dev *bus = phydev->bus;

	/* GPIO[0] output, CLK125 */
	debug("enabling RGMII_REFCLK\n");
	bus->write(bus, 0x1c /*MV_GLOBAL2*/, 0,
		   0x1a /*MV_SCRATCH_MISC*/,
		   (1 << 15) | (0x62 /*MV_GPIO_DIR*/ << 8) | 0xfe);
	bus->write(bus, 0x1c /*MV_GLOBAL2*/, 0,
		   0x1a /*MV_SCRATCH_MISC*/,
		   (1 << 15) | (0x68 /*MV_GPIO01_CNTL*/ << 8) | 7);

	/* RGMII delay - Physical Control register bit[15:14] */
	debug("setting port%d RGMII rx/tx delay\n", CONFIG_MV88E61XX_CPU_PORT);
	/* forced 1000mbps full-duplex link */
	bus->write(bus, 0x10 + CONFIG_MV88E61XX_CPU_PORT, 0, 1, 0xc0fe);
	phydev->autoneg = AUTONEG_DISABLE;
	phydev->speed = SPEED_1000;
	phydev->duplex = DUPLEX_FULL;

	/* LED configuration: 7:4-green (8=Activity)  3:0 amber (8=Link) */
	bus->write(bus, 0x10, 0, 0x16, 0x8088);
	bus->write(bus, 0x11, 0, 0x16, 0x8088);
	bus->write(bus, 0x12, 0, 0x16, 0x8088);
	bus->write(bus, 0x13, 0, 0x16, 0x8088);

	return 0;
}
#endif // CONFIG_MV88E61XX_SWITCH

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FEC_MXC
	struct ventana_board_info *info = &ventana_info;

	if (test_bit(EECONFIG_ETH0, info->config)) {
		setup_iomux_enet(GP_PHY_RST);
		cpu_eth_init(bis);
	}
#endif

#ifdef CONFIG_E1000
	e1000_initialize(bis);
#endif

#ifdef CONFIG_CI_UDC
	/* For otg ethernet*/
	usb_eth_initialize(bis);
#endif

	/* default to the first detected enet dev */
	if (!env_get("ethprime")) {
		struct eth_device *dev = eth_get_dev_by_index(0);
		if (dev) {
			env_set("ethprime", dev->name);
			printf("set ethprime to %s\n", env_get("ethprime"));
		}
	}

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)

static void enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static int detect_i2c(struct display_info_t const *dev)
{
	return i2c_set_bus_num(dev->bus) == 0 &&
		i2c_probe(dev->addr) == 0;
}

static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)
				IOMUXC_BASE_ADDR;

	/* set CH0 data width to 24bit (IOMUXC_GPR2:5 0=18bit, 1=24bit) */
	u32 reg = readl(&iomux->gpr[2]);
	reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT;
	writel(reg, &iomux->gpr[2]);

	/* Enable Backlight */
	gpio_request(IMX_GPIO_NR(1, 10), "bklt_gpio");
	gpio_direction_output(IMX_GPIO_NR(1, 10), 0);
	gpio_request(IMX_GPIO_NR(1, 18), "bklt_en");
	SETUP_IOMUX_PAD(PAD_SD1_CMD__GPIO1_IO18 | DIO_PAD_CFG);
	gpio_direction_output(IMX_GPIO_NR(1, 18), 1);
}

struct display_info_t const displays[] = {{
	/* HDMI Output */
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= enable_hdmi,
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
} }, {
	/* Freescale MXC-LVDS1: HannStar HSD100PXN1-A00 w/ egalx_ts cont */
	.bus	= 2,
	.addr	= 0x4,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.detect	= detect_i2c,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "Hannstar-XGA",
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
} }, {
	/* DLC700JMG-T-4 */
	.bus	= 2,
	.addr	= 0x38,
	.detect	= NULL,
	.enable	= enable_lvds,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.mode	= {
		.name           = "DLC700JMGT4",
		.refresh        = 60,
		.xres           = 1024,		/* 1024x600active pixels */
		.yres           = 600,
		.pixclock       = 15385,	/* 64MHz */
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	/* DLC800FIG-T-3 */
	.bus	= 2,
	.addr	= 0x14,
	.detect	= NULL,
	.enable	= enable_lvds,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.mode	= {
		.name           = "DLC800FIGT3",
		.refresh        = 60,
		.xres           = 1024,		/* 1024x768 active pixels */
		.yres           = 768,
		.pixclock       = 15385,	/* 64MHz */
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 2,
	.addr	= 0x5d,
	.detect	= detect_i2c,
	.enable	= enable_lvds,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.mode	= {
		.name           = "Z101WX01",
		.refresh        = 60,
		.xres           = 1280,
		.yres           = 800,
		.pixclock       = 15385,	/* 64MHz */
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
	}
},
};
size_t display_count = ARRAY_SIZE(displays);

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();
	/* Turn on LDB0,IPU,IPU DI0 clocks */
	reg = __raw_readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_LDB_DI0_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 |MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3<<MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
	      |(3<<MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<<MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     |IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	     |IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     |IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     |IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
	     |IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	     |IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~IOMUXC_GPR3_LVDS0_MUX_CTL_MASK)
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       <<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	/* LVDS Backlight GPIO on LVDS connector - output low */
	SETUP_IOMUX_PAD(PAD_SD2_CLK__GPIO1_IO10 | DIO_PAD_CFG);
	gpio_direction_output(IMX_GPIO_NR(1, 10), 0);
}
#endif /* CONFIG_VIDEO_IPUV3 */

/* setup board specific PMIC */
int power_init_board(void)
{
	setup_pmic();
	return 0;
}

#if defined(CONFIG_CMD_PCI)
int imx6_pcie_toggle_reset(void)
{
	if (board_type < GW_UNKNOWN) {
		uint pin = gpio_cfg[board_type].pcie_rst;
		gpio_request(pin, "pci_rst#");
		gpio_direction_output(pin, 0);
		mdelay(50);
		gpio_direction_output(pin, 1);
	}
	return 0;
}

/*
 * Most Ventana boards have a PLX PEX860x PCIe switch onboard and use its
 * GPIO's as PERST# signals for its downstream ports - configure the GPIO's
 * properly and assert reset for 100ms.
 */
#define MAX_PCI_DEVS	32
struct pci_dev {
	pci_dev_t devfn;
	unsigned short vendor;
	unsigned short device;
	unsigned short class;
	unsigned short busno; /* subbordinate busno */
	struct pci_dev *ppar;
};
struct pci_dev pci_devs[MAX_PCI_DEVS];
int pci_devno;
int pci_bridgeno;

void board_pci_fixup_dev(struct pci_controller *hose, pci_dev_t dev,
			 unsigned short vendor, unsigned short device,
			 unsigned short class)
{
	int i;
	u32 dw;
	struct pci_dev *pdev = &pci_devs[pci_devno++];

	debug("%s: %02d:%02d.%02d: %04x:%04x\n", __func__,
	      PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev), vendor, device);

	/* store array of devs for later use in device-tree fixup */
	pdev->devfn = dev;
	pdev->vendor = vendor;
	pdev->device = device;
	pdev->class = class;
	pdev->ppar = NULL;
	if (class == PCI_CLASS_BRIDGE_PCI)
		pdev->busno = ++pci_bridgeno;
	else
		pdev->busno = 0;

	/* fixup RC - it should be 00:00.0 not 00:01.0 */
	if (PCI_BUS(dev) == 0)
		pdev->devfn = 0;

	/* find dev's parent */
	for (i = 0; i < pci_devno; i++) {
		if (pci_devs[i].busno == PCI_BUS(pdev->devfn)) {
			pdev->ppar = &pci_devs[i];
			break;
		}
	}

	/* assert downstream PERST# */
	if (vendor == PCI_VENDOR_ID_PLX &&
	    (device & 0xfff0) == 0x8600 &&
	    PCI_DEV(dev) == 0 && PCI_FUNC(dev) == 0) {
		debug("configuring PLX 860X downstream PERST#\n");
		pci_hose_read_config_dword(hose, dev, 0x62c, &dw);
		dw |= 0xaaa8; /* GPIO1-7 outputs */
		pci_hose_write_config_dword(hose, dev, 0x62c, dw);

		pci_hose_read_config_dword(hose, dev, 0x644, &dw);
		dw |= 0xfe;   /* GPIO1-7 output high */
		pci_hose_write_config_dword(hose, dev, 0x644, dw);

		mdelay(100);
	}
}
#endif /* CONFIG_CMD_PCI */

#ifdef CONFIG_SERIAL_TAG
/*
 * called when setting up ATAGS before booting kernel
 * populate serialnum from the following (in order of priority):
 *   serial# env var
 *   eeprom
 */
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial = env_get("serial#");

	if (serial) {
		serialnr->high = 0;
		serialnr->low = simple_strtoul(serial, NULL, 10);
	} else if (ventana_info.model[0]) {
		serialnr->high = 0;
		serialnr->low = ventana_info.serial;
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}
#endif

/*
 * Board Support
 */

int board_early_init_f(void)
{
	setup_iomux_uart();

#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

int board_init(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	clrsetbits_le32(&iomuxc_regs->gpr[1],
			IOMUXC_GPR1_OTG_ID_MASK,
			IOMUXC_GPR1_OTG_ID_GPIO1);

	/* address of linux boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/* read Gateworks EEPROM into global struct (used later) */
	setup_ventana_i2c(0);
	board_type = read_eeprom(CONFIG_I2C_GSC, &ventana_info);

#ifdef CONFIG_CMD_NAND
	if (gpio_cfg[board_type].nand)
		setup_gpmi_nand();
#endif
#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif
	setup_ventana_i2c(1);
	setup_ventana_i2c(2);

#ifdef CONFIG_SATA
	setup_sata();
#endif

	setup_iomux_gpio(board_type, &ventana_info);

	return 0;
}

#if defined(CONFIG_DISPLAY_BOARDINFO_LATE)
/*
 * called during late init (after relocation and after board_init())
 * by virtue of CONFIG_DISPLAY_BOARDINFO_LATE as we needed i2c initialized and
 * EEPROM read.
 */
int checkboard(void)
{
	struct ventana_board_info *info = &ventana_info;
	unsigned char buf[4];
	const char *p;
	int quiet; /* Quiet or minimal output mode */

	quiet = 0;
	p = env_get("quiet");
	if (p)
		quiet = simple_strtol(p, NULL, 10);
	else
		env_set("quiet", "0");

	puts("\nGateworks Corporation Copyright 2014\n");
	if (info->model[0]) {
		printf("Model: %s\n", info->model);
		printf("MFGDate: %02x-%02x-%02x%02x\n",
		       info->mfgdate[0], info->mfgdate[1],
		       info->mfgdate[2], info->mfgdate[3]);
		printf("Serial:%d\n", info->serial);
	} else {
		puts("Invalid EEPROM - board will not function fully\n");
	}
	if (quiet)
		return 0;

	/* Display GSC firmware revision/CRC/status */
	gsc_info(0);

	/* Display RTC */
	if (!gsc_i2c_read(GSC_RTC_ADDR, 0x00, 1, buf, 4)) {
		printf("RTC:   %d\n",
		       buf[0] | buf[1]<<8 | buf[2]<<16 | buf[3]<<24);
	}

	return 0;
}
#endif

#ifdef CONFIG_CMD_BMODE
/*
 * BOOT_CFG1, BOOT_CFG2, BOOT_CFG3, BOOT_CFG4
 * see Table 8-11 and Table 5-9
 *  BOOT_CFG1[7] = 1 (boot from NAND)
 *  BOOT_CFG1[5] = 0 - raw NAND
 *  BOOT_CFG1[4] = 0 - default pad settings
 *  BOOT_CFG1[3:2] = 00 - devices = 1
 *  BOOT_CFG1[1:0] = 00 - Row Address Cycles = 3
 *  BOOT_CFG2[4:3] = 00 - Boot Search Count = 2
 *  BOOT_CFG2[2:1] = 01 - Pages In Block = 64
 *  BOOT_CFG2[0] = 0 - Reset time 12ms
 */
static const struct boot_mode board_boot_modes[] = {
	/* NAND: 64pages per block, 3 row addr cycles, 2 copies of FCB/DBBT */
	{ "nand", MAKE_CFGVAL(0x80, 0x02, 0x00, 0x00) },
	{ "emmc2", MAKE_CFGVAL(0x60, 0x48, 0x00, 0x00) }, /* GW5600 */
	{ "emmc3", MAKE_CFGVAL(0x60, 0x50, 0x00, 0x00) }, /* GW5903/4/5 */
	{ NULL, 0 },
};
#endif

/* late init */
int misc_init_r(void)
{
	struct ventana_board_info *info = &ventana_info;
	char buf[256];
	int i;

	/* set env vars based on EEPROM data */
	if (ventana_info.model[0]) {
		char str[16], fdt[36];
		char *p;
		const char *cputype = "";

		/*
		 * FDT name will be prefixed with CPU type.  Three versions
		 * will be created each increasingly generic and bootloader
		 * env scripts will try loading each from most specific to
		 * least.
		 */
		if (is_cpu_type(MXC_CPU_MX6Q) ||
		    is_cpu_type(MXC_CPU_MX6D))
			cputype = "imx6q";
		else if (is_cpu_type(MXC_CPU_MX6DL) ||
			 is_cpu_type(MXC_CPU_MX6SOLO))
			cputype = "imx6dl";
		env_set("soctype", cputype);
		if (8 << (ventana_info.nand_flash_size-1) >= 2048)
			env_set("flash_layout", "large");
		else
			env_set("flash_layout", "normal");
		memset(str, 0, sizeof(str));
		for (i = 0; i < (sizeof(str)-1) && info->model[i]; i++)
			str[i] = tolower(info->model[i]);
		env_set("model", str);
		if (!env_get("fdt_file")) {
			sprintf(fdt, "%s-%s.dtb", cputype, str);
			env_set("fdt_file", fdt);
		}
		p = strchr(str, '-');
		if (p) {
			*p++ = 0;

			env_set("model_base", str);
			sprintf(fdt, "%s-%s.dtb", cputype, str);
			env_set("fdt_file1", fdt);
			if (board_type != GW551x &&
			    board_type != GW552x &&
			    board_type != GW553x &&
			    board_type != GW560x)
				str[4] = 'x';
			str[5] = 'x';
			str[6] = 0;
			sprintf(fdt, "%s-%s.dtb", cputype, str);
			env_set("fdt_file2", fdt);
		}

		/* initialize env from EEPROM */
		if (test_bit(EECONFIG_ETH0, info->config) &&
		    !env_get("ethaddr")) {
			eth_env_set_enetaddr("ethaddr", info->mac0);
		}
		if (test_bit(EECONFIG_ETH1, info->config) &&
		    !env_get("eth1addr")) {
			eth_env_set_enetaddr("eth1addr", info->mac1);
		}

		/* board serial-number */
		sprintf(str, "%6d", info->serial);
		env_set("serial#", str);

		/* memory MB */
		sprintf(str, "%d", (int) (gd->ram_size >> 20));
		env_set("mem_mb", str);
	}

	/* Set a non-initialized hwconfig based on board configuration */
	if (!strcmp(env_get("hwconfig"), "_UNKNOWN_")) {
		buf[0] = 0;
		if (gpio_cfg[board_type].rs232_en)
			strcat(buf, "rs232;");
		for (i = 0; i < gpio_cfg[board_type].dio_num; i++) {
			char buf1[32];
			sprintf(buf1, "dio%d:mode=gpio;", i);
			if (strlen(buf) + strlen(buf1) < sizeof(buf))
				strcat(buf, buf1);
		}
		env_set("hwconfig", buf);
	}

	/* setup baseboard specific GPIO based on board and env */
	setup_board_gpio(board_type, info);

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	/* disable boot watchdog */
	gsc_boot_wd_disable();

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP

static int ft_sethdmiinfmt(void *blob, char *mode)
{
	int off;

	if (!mode)
		return -EINVAL;

	off = fdt_node_offset_by_compatible(blob, -1, "nxp,tda1997x");
	if (off < 0)
		return off;

	if (0 == strcasecmp(mode, "yuv422bt656")) {
		u8 cfg[] = { 0x00, 0x00, 0x00, 0x82, 0x81, 0x00,
			     0x00, 0x00, 0x00 };
		mode = "422_ccir";
		fdt_setprop(blob, off, "vidout_fmt", mode, strlen(mode) + 1);
		fdt_setprop_u32(blob, off, "vidout_trc", 1);
		fdt_setprop_u32(blob, off, "vidout_blc", 1);
		fdt_setprop(blob, off, "vidout_portcfg", cfg, sizeof(cfg));
		printf("   set HDMI input mode to %s\n", mode);
	} else if (0 == strcasecmp(mode, "yuv422smp")) {
		u8 cfg[] = { 0x00, 0x00, 0x00, 0x88, 0x87, 0x00,
			     0x82, 0x81, 0x00 };
		mode = "422_smp";
		fdt_setprop(blob, off, "vidout_fmt", mode, strlen(mode) + 1);
		fdt_setprop_u32(blob, off, "vidout_trc", 0);
		fdt_setprop_u32(blob, off, "vidout_blc", 0);
		fdt_setprop(blob, off, "vidout_portcfg", cfg, sizeof(cfg));
		printf("   set HDMI input mode to %s\n", mode);
	} else {
		return -EINVAL;
	}

	return 0;
}

#if defined(CONFIG_CMD_PCI)
#define PCI_ID(x) ( \
	(PCI_BUS(x->devfn)<<16)| \
	(PCI_DEV(x->devfn)<<11)| \
	(PCI_FUNC(x->devfn)<<8) \
	)
int fdt_add_pci_node(void *blob, int par, struct pci_dev *dev)
{
	uint32_t reg[5];
	char node[32];
	int np;

	sprintf(node, "pcie@%d,%d,%d", PCI_BUS(dev->devfn),
		PCI_DEV(dev->devfn), PCI_FUNC(dev->devfn));

	np = fdt_subnode_offset(blob, par, node);
	if (np >= 0)
		return np;
	np = fdt_add_subnode(blob, par, node);
	if (np < 0) {
		printf("   %s failed: no space\n", __func__);
		return np;
	}

	memset(reg, 0, sizeof(reg));
	reg[0] = cpu_to_fdt32(PCI_ID(dev));
	fdt_setprop(blob, np, "reg", reg, sizeof(reg));

	return np;
}

/* build a path of nested PCI devs for all bridges passed through */
int fdt_add_pci_path(void *blob, struct pci_dev *dev)
{
	struct pci_dev *bridges[MAX_PCI_DEVS];
	int k, np;

	/* build list of parents */
	np = fdt_node_offset_by_compatible(blob, -1, "fsl,imx6q-pcie");
	if (np < 0)
		return np;

	k = 0;
	while (dev) {
		bridges[k++] = dev;
		dev = dev->ppar;
	};

	/* now add them the to DT in reverse order */
	while (k--) {
		np = fdt_add_pci_node(blob, np, bridges[k]);
		if (np < 0)
			break;
	}

	return np;
}

/*
 * The GW16082 has a hardware errata errata such that it's
 * INTA/B/C/D are mis-mapped to its four slots (slot12-15). Because
 * of this normal PCI interrupt swizzling will not work so we will
 * provide an irq-map via device-tree.
 */
int fdt_fixup_gw16082(void *blob, int np, struct pci_dev *dev)
{
	int len;
	int host;
	uint32_t imap_new[8*4*4];
	const uint32_t *imap;
	uint32_t irq[4];
	uint32_t reg[4];
	int i;

	/* build irq-map based on host controllers map */
	host = fdt_node_offset_by_compatible(blob, -1, "fsl,imx6q-pcie");
	if (host < 0) {
		printf("   %s failed: missing host\n", __func__);
		return host;
	}

	/* use interrupt data from root complex's node */
	imap = fdt_getprop(blob, host, "interrupt-map", &len);
	if (!imap || len != 128) {
		printf("   %s failed: invalid interrupt-map\n",
		       __func__);
		return -FDT_ERR_NOTFOUND;
	}

	/* obtain irq's of host controller in pin order */
	for (i = 0; i < 4; i++)
		irq[(fdt32_to_cpu(imap[(i*8)+3])-1)%4] = imap[(i*8)+6];

	/*
	 * determine number of swizzles necessary:
	 *   For each bridge we pass through we need to swizzle
	 *   the number of the slot we are on.
	 */
	struct pci_dev *d;
	int b;
	b = 0;
	d = dev->ppar;
	while(d && d->ppar) {
		b += PCI_DEV(d->devfn);
		d = d->ppar;
	}

	/* create new irq mappings for slots12-15
	 * <skt> <idsel> <slot> <skt-inta> <skt-intb>
	 * J3    AD28    12     INTD      INTA
	 * J4    AD29    13     INTC      INTD
	 * J5    AD30    14     INTB      INTC
	 * J2    AD31    15     INTA      INTB
	 */
	for (i = 0; i < 4; i++) {
		/* addr matches bus:dev:func */
		u32 addr = dev->busno << 16 | (12+i) << 11;

		/* default cells from root complex */
		memcpy(&imap_new[i*32], imap, 128);
		/* first cell is PCI device address (BDF) */
		imap_new[(i*32)+(0*8)+0] = cpu_to_fdt32(addr);
		imap_new[(i*32)+(1*8)+0] = cpu_to_fdt32(addr);
		imap_new[(i*32)+(2*8)+0] = cpu_to_fdt32(addr);
		imap_new[(i*32)+(3*8)+0] = cpu_to_fdt32(addr);
		/* third cell is pin */
		imap_new[(i*32)+(0*8)+3] = cpu_to_fdt32(1);
		imap_new[(i*32)+(1*8)+3] = cpu_to_fdt32(2);
		imap_new[(i*32)+(2*8)+3] = cpu_to_fdt32(3);
		imap_new[(i*32)+(3*8)+3] = cpu_to_fdt32(4);
		/* sixth cell is relative interrupt */
		imap_new[(i*32)+(0*8)+6] = irq[(15-(12+i)+b+0)%4];
		imap_new[(i*32)+(1*8)+6] = irq[(15-(12+i)+b+1)%4];
		imap_new[(i*32)+(2*8)+6] = irq[(15-(12+i)+b+2)%4];
		imap_new[(i*32)+(3*8)+6] = irq[(15-(12+i)+b+3)%4];
	}
	fdt_setprop(blob, np, "interrupt-map", imap_new,
		    sizeof(imap_new));
	reg[0] = cpu_to_fdt32(0xfff00);
	reg[1] = 0;
	reg[2] = 0;
	reg[3] = cpu_to_fdt32(0x7);
	fdt_setprop(blob, np, "interrupt-map-mask", reg, sizeof(reg));
	fdt_setprop_cell(blob, np, "#interrupt-cells", 1);
	fdt_setprop_string(blob, np, "device_type", "pci");
	fdt_setprop_cell(blob, np, "#address-cells", 3);
	fdt_setprop_cell(blob, np, "#size-cells", 2);
	printf("   Added custom interrupt-map for GW16082\n");

	return 0;
}

/* The sky2 GigE MAC obtains it's MAC addr from device-tree by default */
int fdt_fixup_sky2(void *blob, int np, struct pci_dev *dev)
{
	char *tmp, *end;
	char mac[16];
	unsigned char mac_addr[6];
	int j;

	sprintf(mac, "eth1addr");
	tmp = env_get(mac);
	if (tmp) {
		for (j = 0; j < 6; j++) {
			mac_addr[j] = tmp ?
				      simple_strtoul(tmp, &end,16) : 0;
			if (tmp)
				tmp = (*end) ? end+1 : end;
		}
		fdt_setprop(blob, np, "local-mac-address", mac_addr,
			    sizeof(mac_addr));
		printf("   Added mac addr for eth1\n");
		return 0;
	}

	return -1;
}

/*
 * PCI DT nodes must be nested therefore if we need to apply a DT fixup
 * we will walk the PCI bus and add bridge nodes up to the device receiving
 * the fixup.
 */
void ft_board_pci_fixup(void *blob, bd_t *bd)
{
	int i, np;
	struct pci_dev *dev;

	for (i = 0; i < pci_devno; i++) {
		dev = &pci_devs[i];

		/*
		 * The GW16082 consists of a TI XIO2001 PCIe-to-PCI bridge and
		 * an EEPROM at i2c1-0x50.
		 */
		if ((dev->vendor == PCI_VENDOR_ID_TI) &&
		    (dev->device == 0x8240) &&
		    (i2c_set_bus_num(1) == 0) &&
		    (i2c_probe(0x50) == 0))
		{
			np = fdt_add_pci_path(blob, dev);
			if (np > 0)
				fdt_fixup_gw16082(blob, np, dev);
		}

		/* ethernet1 mac address */
		else if ((dev->vendor == PCI_VENDOR_ID_MARVELL) &&
		         (dev->device == 0x4380))
		{
			np = fdt_add_pci_path(blob, dev);
			if (np > 0)
				fdt_fixup_sky2(blob, np, dev);
		}
	}
}
#endif /* if defined(CONFIG_CMD_PCI) */

void ft_board_wdog_fixup(void *blob, phys_addr_t addr)
{
	int off = fdt_node_offset_by_compat_reg(blob, "fsl,imx6q-wdt", addr);

	if (off) {
		fdt_delprop(blob, off, "ext-reset-output");
		fdt_delprop(blob, off, "fsl,ext-reset-output");
	}
}

/*
 * called prior to booting kernel or by 'fdt boardsetup' command
 *
 * unless 'fdt_noauto' env var is set we will update the following in the DTB:
 *  - mtd partitions based on mtdparts/mtdids env
 *  - system-serial (board serial num from EEPROM)
 *  - board (full model from EEPROM)
 *  - peripherals removed from DTB if not loaded on board (per EEPROM config)
 */
#define WDOG1_ADDR	0x20bc000
#define WDOG2_ADDR	0x20c0000
#define GPIO3_ADDR	0x20a4000
#define USDHC3_ADDR	0x2198000
#define PWM0_ADDR	0x2080000
int ft_board_setup(void *blob, bd_t *bd)
{
	struct ventana_board_info *info = &ventana_info;
	struct ventana_eeprom_config *cfg;
	static const struct node_info nodes[] = {
		{ "sst,w25q256",          MTD_DEV_TYPE_NOR, },  /* SPI flash */
		{ "fsl,imx6q-gpmi-nand",  MTD_DEV_TYPE_NAND, }, /* NAND flash */
	};
	const char *model = env_get("model");
	const char *display = env_get("display");
	int i;
	char rev = 0;

	/* determine board revision */
	for (i = sizeof(ventana_info.model) - 1; i > 0; i--) {
		if (ventana_info.model[i] >= 'A') {
			rev = ventana_info.model[i];
			break;
		}
	}

	if (env_get("fdt_noauto")) {
		puts("   Skiping ft_board_setup (fdt_noauto defined)\n");
		return 0;
	}

	if (test_bit(EECONFIG_NAND, info->config)) {
		/* Update partition nodes using info from mtdparts env var */
		puts("   Updating MTD partitions...\n");
		fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
	}

	/* Update display timings from display env var */
	if (display) {
		if (fdt_fixup_display(blob, fdt_get_alias(blob, "lvds0"),
				      display) >= 0)
			printf("   Set display timings for %s...\n", display);
	}

	printf("   Adjusting FDT per EEPROM for %s...\n", model);

	/* board serial number */
	fdt_setprop(blob, 0, "system-serial", env_get("serial#"),
		    strlen(env_get("serial#")) + 1);

	/* board (model contains model from device-tree) */
	fdt_setprop(blob, 0, "board", info->model,
		    strlen((const char *)info->model) + 1);

	/* set desired digital video capture format */
	ft_sethdmiinfmt(blob, env_get("hdmiinfmt"));

	/*
	 * Board model specific fixups
	 */
	switch (board_type) {
	case GW51xx:
		/*
		 * disable wdog node for GW51xx-A/B to work around
		 * errata causing wdog timer to be unreliable.
		 */
		if (rev >= 'A' && rev < 'C') {
			i = fdt_node_offset_by_compat_reg(blob, "fsl,imx6q-wdt",
							  WDOG1_ADDR);
			if (i)
				fdt_status_disabled(blob, i);
		}

		/* GW51xx-E adds WDOG1_B external reset */
		if (rev < 'E')
			ft_board_wdog_fixup(blob, WDOG1_ADDR);
		break;

	case GW52xx:
		/* GW522x Uses GPIO3_IO23 instead of GPIO1_IO29 */
		if (info->model[4] == '2') {
			u32 handle = 0;
			u32 *range = NULL;

			i = fdt_node_offset_by_compatible(blob, -1,
							  "fsl,imx6q-pcie");
			if (i)
				range = (u32 *)fdt_getprop(blob, i,
							   "reset-gpio", NULL);

			if (range) {
				i = fdt_node_offset_by_compat_reg(blob,
					"fsl,imx6q-gpio", GPIO3_ADDR);
				if (i)
					handle = fdt_get_phandle(blob, i);
				if (handle) {
					range[0] = cpu_to_fdt32(handle);
					range[1] = cpu_to_fdt32(23);
				}
			}

			/* these have broken usd_vsel */
			if (strstr((const char *)info->model, "SP318-B") ||
			    strstr((const char *)info->model, "SP331-B"))
				gpio_cfg[board_type].usd_vsel = 0;

			/* GW522x-B adds WDOG1_B external reset */
			if (rev < 'B')
				ft_board_wdog_fixup(blob, WDOG1_ADDR);
		}

		/* GW520x-E adds WDOG1_B external reset */
		else if (info->model[4] == '0' && rev < 'E')
			ft_board_wdog_fixup(blob, WDOG1_ADDR);
		break;

	case GW53xx:
		/* GW53xx-E adds WDOG1_B external reset */
		if (rev < 'E')
			ft_board_wdog_fixup(blob, WDOG1_ADDR);
		break;

	case GW54xx:
		/*
		 * disable serial2 node for GW54xx for compatibility with older
		 * 3.10.x kernel that improperly had this node enabled in the DT
		 */
		fdt_set_status_by_alias(blob, "serial2", FDT_STATUS_DISABLED,
					0);

		/* GW54xx-E adds WDOG2_B external reset */
		if (rev < 'E')
			ft_board_wdog_fixup(blob, WDOG2_ADDR);
		break;

	case GW551x:
		/*
		 * isolate CSI0_DATA_EN for GW551x-A to work around errata
		 * causing non functional digital video in (it is not hooked up)
		 */
		if (rev == 'A') {
			u32 *range = NULL;
			int len;
			const u32 *handle = NULL;

			i = fdt_node_offset_by_compatible(blob, -1,
						"fsl,imx-tda1997x-video");
			if (i)
				handle = fdt_getprop(blob, i, "pinctrl-0",
						     NULL);
			if (handle)
				i = fdt_node_offset_by_phandle(blob,
							fdt32_to_cpu(*handle));
			if (i)
				range = (u32 *)fdt_getprop(blob, i, "fsl,pins",
							   &len);
			if (range) {
				len /= sizeof(u32);
				for (i = 0; i < len; i += 6) {
					u32 mux_reg = fdt32_to_cpu(range[i+0]);
					u32 conf_reg = fdt32_to_cpu(range[i+1]);
					/* mux PAD_CSI0_DATA_EN to GPIO */
					if (is_cpu_type(MXC_CPU_MX6Q) &&
					    mux_reg == 0x260 &&
					    conf_reg == 0x630)
						range[i+3] = cpu_to_fdt32(0x5);
					else if (!is_cpu_type(MXC_CPU_MX6Q) &&
						 mux_reg == 0x08c &&
						 conf_reg == 0x3a0)
						range[i+3] = cpu_to_fdt32(0x5);
				}
				fdt_setprop_inplace(blob, i, "fsl,pins", range,
						    len);
			}

			/* set BT656 video format */
			ft_sethdmiinfmt(blob, "yuv422bt656");
		}

		/* GW551x-C adds WDOG1_B external reset */
		if (rev < 'C')
			ft_board_wdog_fixup(blob, WDOG1_ADDR);
		break;
	case GW5901:
	case GW5902:
		/* GW5901/GW5901 revB adds WDOG1_B as an external reset */
		if (rev < 'B')
			ft_board_wdog_fixup(blob, WDOG1_ADDR);
		break;
	}

	/* Configure DIO */
	for (i = 0; i < gpio_cfg[board_type].dio_num; i++) {
		struct dio_cfg *cfg = &gpio_cfg[board_type].dio_cfg[i];
		char arg[10];

		sprintf(arg, "dio%d", i);
		if (!hwconfig(arg))
			continue;
		if (hwconfig_subarg_cmp(arg, "mode", "pwm") && cfg->pwm_param)
		{
			phys_addr_t addr;
			int off;

			printf("   Enabling pwm%d for DIO%d\n",
			       cfg->pwm_param, i);
			addr = PWM0_ADDR + (0x4000 * (cfg->pwm_param - 1));
			off = fdt_node_offset_by_compat_reg(blob,
							    "fsl,imx6q-pwm",
							    addr);
			if (off)
				fdt_status_okay(blob, off);
		}
	}

	/* remove no-1-8-v if UHS-I support is present */
	if (gpio_cfg[board_type].usd_vsel) {
		debug("Enabling UHS-I support\n");
		i = fdt_node_offset_by_compat_reg(blob, "fsl,imx6q-usdhc",
						  USDHC3_ADDR);
		if (i)
			fdt_delprop(blob, i, "no-1-8-v");
	}

#if defined(CONFIG_CMD_PCI)
	if (!env_get("nopcifixup"))
		ft_board_pci_fixup(blob, bd);
#endif

	/*
	 * Peripheral Config:
	 *  remove nodes by alias path if EEPROM config tells us the
	 *  peripheral is not loaded on the board.
	 */
	if (env_get("fdt_noconfig")) {
		puts("   Skiping periperhal config (fdt_noconfig defined)\n");
		return 0;
	}
	cfg = econfig;
	while (cfg->name) {
		if (!test_bit(cfg->bit, info->config)) {
			fdt_del_node_and_alias(blob, cfg->dtalias ?
					       cfg->dtalias : cfg->name);
		}
		cfg++;
	}

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

static struct mxc_serial_platdata ventana_mxc_serial_plat = {
	.reg = (struct mxc_uart *)UART2_BASE,
};

U_BOOT_DEVICE(ventana_serial) = {
	.name   = "serial_mxc",
	.platdata = &ventana_mxc_serial_plat,
};
