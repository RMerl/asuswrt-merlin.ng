// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 NXP Semiconductors
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <usb.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>
#include "../../freescale/common/pfuze.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_DSE_3P3V_49OHM | \
	PAD_CTL_PUS_PU100KOHM | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_PU100KOHM | PAD_CTL_DSE_3P3V_49OHM)
#define ENET_PAD_CTRL_MII  (PAD_CTL_DSE_3P3V_32OHM)

#define ENET_RX_PAD_CTRL  (PAD_CTL_PUS_PU100KOHM | PAD_CTL_DSE_3P3V_49OHM)

#define I2C_PAD_CTRL    (PAD_CTL_DSE_3P3V_32OHM | PAD_CTL_SRE_SLOW | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU100KOHM)


#define LCD_PAD_CTRL    (PAD_CTL_HYS | PAD_CTL_PUS_PU100KOHM | \
			 PAD_CTL_DSE_3P3V_49OHM)

#define LCD_SYNC_PAD_CTRL    (PAD_CTL_HYS | PAD_CTL_PUS_PU100KOHM | \
			      PAD_CTL_DSE_3P3V_196OHM)

#ifdef CONFIG_SYS_I2C_MXC
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

/* I2C4 for PMIC */
static struct i2c_pads_info i2c_pad_info4 = {
	.scl = {
		.i2c_mode = MX7D_PAD_SAI1_RX_SYNC__I2C4_SCL | PC,
		.gpio_mode = MX7D_PAD_SAI1_RX_SYNC__GPIO6_IO16 | PC,
		.gp = IMX_GPIO_NR(6, 16),
	},
	.sda = {
		.i2c_mode = MX7D_PAD_SAI1_RX_BCLK__I2C4_SDA | PC,
		.gpio_mode = MX7D_PAD_SAI1_RX_BCLK__GPIO6_IO17 | PC,
		.gp = IMX_GPIO_NR(6, 17),
	},
};
#endif

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	/* Subtract the defined OPTEE runtime firmware length */
#ifdef CONFIG_OPTEE_TZDRAM_SIZE
		gd->ram_size -= CONFIG_OPTEE_TZDRAM_SIZE;
#endif

	return 0;
}

#ifdef CONFIG_POWER
#define I2C_PMIC	3
int power_init_board(void)
{
	struct pmic *p;
	int ret;
	unsigned int reg, rev_id;

	ret = power_pfuze3000_init(I2C_PMIC);
	if (ret)
		return ret;

	p = pmic_get("PFUZE3000");
	ret = pmic_probe(p);
	if (ret) {
		printf("Warning:  Cannot find PMIC PFUZE3000\n");
		printf("\tPower consumption is not optimized.\n");
		return 0;
	}

	pmic_reg_read(p, PFUZE3000_DEVICEID, &reg);
	pmic_reg_read(p, PFUZE3000_REVID, &rev_id);
	printf("PMIC:  PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n", reg, rev_id);

	/* disable Low Power Mode during standby mode */
	pmic_reg_read(p, PFUZE3000_LDOGCTL, &reg);
	reg |= 0x1;
	pmic_reg_write(p, PFUZE3000_LDOGCTL, reg);

	/* SW1A/1B mode set to APS/APS */
	reg = 0x8;
	pmic_reg_write(p, PFUZE3000_SW1AMODE, reg);
	pmic_reg_write(p, PFUZE3000_SW1BMODE, reg);

	/* SW1A/1B standby voltage set to 1.025V */
	reg = 0xd;
	pmic_reg_write(p, PFUZE3000_SW1ASTBY, reg);
	pmic_reg_write(p, PFUZE3000_SW1BSTBY, reg);

	/* decrease SW1B normal voltage to 0.975V */
	pmic_reg_read(p, PFUZE3000_SW1BVOLT, &reg);
	reg &= ~0x1f;
	reg |= PFUZE3000_SW1AB_SETP(975);
	pmic_reg_write(p, PFUZE3000_SW1BVOLT, reg);

	return 0;
}
#endif

static iomux_v3_cfg_t const wdog_pads[] = {
	MX7D_PAD_GPIO1_IO00__WDOG1_WDOG_B | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const uart5_pads[] = {
	MX7D_PAD_I2C4_SCL__UART5_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_I2C4_SDA__UART5_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#ifdef CONFIG_FEC_MXC
static iomux_v3_cfg_t const fec1_pads[] = {
	MX7D_PAD_SD2_CD_B__ENET1_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL_MII),
	MX7D_PAD_SD2_WP__ENET1_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL_MII),
	MX7D_PAD_ENET1_RGMII_TXC__ENET1_RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TD0__ENET1_RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TD1__ENET1_RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TD2__ENET1_RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TD3__ENET1_RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TX_CTL__ENET1_RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RXC__ENET1_RGMII_RXC | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RD0__ENET1_RGMII_RD0 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RD1__ENET1_RGMII_RD1 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RD2__ENET1_RGMII_RD2 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RD3__ENET1_RGMII_RD3 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RX_CTL__ENET1_RGMII_RX_CTL | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_SD3_STROBE__GPIO6_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX7D_PAD_SD3_RESET_B__GPIO6_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define FEC1_RST_GPIO	IMX_GPIO_NR(6, 11)

static void setup_iomux_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec1_pads, ARRAY_SIZE(fec1_pads));
	gpio_request(FEC1_RST_GPIO, "phy_rst");
	gpio_direction_output(FEC1_RST_GPIO, 0);
	udelay(500);
	gpio_set_value(FEC1_RST_GPIO, 1);
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_fec();

	return fecmxc_initialize_multi(bis, 0,
		CONFIG_FEC_MXC_PHYADDR, IMX_FEC_BASE);
}

static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, clear gpr1[13], gpr1[17] */
	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			(IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK |
			IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK), 0);

	return set_clk_enet(ENET_125MHZ);
}

int board_phy_config(struct phy_device *phydev)
{
	unsigned short val;

	/* To enable AR8035 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe7;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart5_pads, ARRAY_SIZE(uart5_pads));
}

int board_early_init_f(void)
{
	setup_iomux_uart();

#ifdef CONFIG_SYS_I2C_MXC
	setup_i2c(3, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info4);
#endif

	return 0;
}

#ifdef CONFIG_VIDEO_MXS
static iomux_v3_cfg_t const lcd_pads[] = {
	MX7D_PAD_LCD_CLK__LCD_CLK | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_ENABLE__LCD_ENABLE | MUX_PAD_CTRL(LCD_SYNC_PAD_CTRL),
	MX7D_PAD_LCD_HSYNC__LCD_HSYNC | MUX_PAD_CTRL(LCD_SYNC_PAD_CTRL),
	MX7D_PAD_LCD_VSYNC__LCD_VSYNC | MUX_PAD_CTRL(LCD_SYNC_PAD_CTRL),
	MX7D_PAD_LCD_DATA00__LCD_DATA0 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA01__LCD_DATA1 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA02__LCD_DATA2 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA03__LCD_DATA3 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA04__LCD_DATA4 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA05__LCD_DATA5 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA06__LCD_DATA6 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA07__LCD_DATA7 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA08__LCD_DATA8 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA09__LCD_DATA9 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA10__LCD_DATA10 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA11__LCD_DATA11 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA12__LCD_DATA12 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA13__LCD_DATA13 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA14__LCD_DATA14 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA15__LCD_DATA15 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA16__LCD_DATA16 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA17__LCD_DATA17 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA18__LCD_DATA18 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA19__LCD_DATA19 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA20__LCD_DATA20 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA21__LCD_DATA21 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA22__LCD_DATA22 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_LCD_DATA23__LCD_DATA23 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_GPIO1_IO06__GPIO1_IO6	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX7D_PAD_GPIO1_IO11__GPIO1_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

void setup_lcd(void)
{
	imx_iomux_v3_setup_multiple_pads(lcd_pads, ARRAY_SIZE(lcd_pads));
	gpio_request(IMX_GPIO_NR(1, 11), "lcd_brightness");
	gpio_request(IMX_GPIO_NR(1, 6), "lcd_enable");
	/* Set Brightness to high */
	gpio_direction_output(IMX_GPIO_NR(1, 11) , 1);
	/* Set LCD enable to high */
	gpio_direction_output(IMX_GPIO_NR(1, 6) , 1);
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_VIDEO_MXS
	setup_lcd();
#endif
#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

	return 0;
}

int board_late_init(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	/*
	 * Do not assert internal WDOG_RESET_B_DEB(controlled by bit 4),
	 * since we use PMIC_PWRON to reset the board.
	 */
	clrsetbits_le16(&wdog->wcr, 0, 0x10);

	return 0;
}

int checkboard(void)
{
	puts("Board: i.MX7D PICOSOM\n");

	return 0;
}

static iomux_v3_cfg_t const usb_otg2_pads[] = {
	MX7D_PAD_UART3_CTS_B__USB_OTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		break;
	case 1:
		imx_iomux_v3_setup_multiple_pads(usb_otg2_pads,
						 ARRAY_SIZE(usb_otg2_pads));
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int board_usb_phy_mode(int port)
{
	switch (port) {
	case 0:
		return USB_INIT_DEVICE;
	case 1:
		return USB_INIT_HOST;
	default:
		return -EINVAL;
	}
	return 0;
}
