// SPDX-License-Identifier: GPL-2.0+
/*
 * K+P iMX6Q KP_IMX6Q_TPC board configuration
 *
 * Copyright (C) 2018 Lukasz Majewski <lukma@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <errno.h>
#include <fsl_esdhc.h>
#include <fuse.h>
#include <i2c.h>
#include <miiphy.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <usb.h>
#include <usb/ehci-ci.h>

DECLARE_GLOBAL_DATA_PTR;

#define ENET_PAD_CTRL							\
	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	 PAD_CTL_HYS)

#define I2C_PAD_CTRL							\
	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	PAD_CTL_HYS | PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define PC			MUX_PAD_CTRL(I2C_PAD_CTRL)

static struct i2c_pads_info kp_imx6q_tpc_i2c_pad_info0 = {
	.scl = {
		.i2c_mode  = MX6Q_PAD_CSI0_DAT9__I2C1_SCL | PC,
		.gpio_mode = MX6Q_PAD_CSI0_DAT9__GPIO5_IO27 | PC,
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		 .i2c_mode = MX6Q_PAD_CSI0_DAT8__I2C1_SDA | PC,
		 .gpio_mode = MX6Q_PAD_CSI0_DAT8__GPIO5_IO26 | PC,
		 .gp = IMX_GPIO_NR(5, 26)
	}
};

static struct i2c_pads_info kp_imx6q_tpc_i2c_pad_info1 = {
	.scl = {
		.i2c_mode  = MX6Q_PAD_KEY_COL3__I2C2_SCL | PC,
		.gpio_mode = MX6Q_PAD_KEY_COL3__GPIO4_IO12 | PC,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		 .i2c_mode = MX6Q_PAD_KEY_ROW3__I2C2_SDA | PC,
		 .gpio_mode = MX6Q_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		 .gp = IMX_GPIO_NR(4, 13)
	}
};

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

#ifdef CONFIG_FEC_MXC
static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL |
		   MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL |
		   MUX_PAD_CTRL(ENET_PAD_CTRL)),
	/* AR8031 PHY Reset */
	IOMUX_PADS(PAD_ENET_CRS_DV__GPIO1_IO25	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void eth_phy_reset(void)
{
	/* Reset AR8031 PHY */
	gpio_direction_output(IMX_GPIO_NR(1, 25), 0);
	mdelay(10);
	gpio_set_value(IMX_GPIO_NR(1, 25), 1);
	udelay(100);
}

static int setup_fec_clock(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* set gpr1[21] to select anatop clock */
	clrsetbits_le32(&iomuxc_regs->gpr[1], 0x1 << 21, 0x1 << 21);

	return enable_fec_anatop_clock(0, ENET_50MHZ);
}

int board_eth_init(bd_t *bis)
{
	SETUP_IOMUX_PADS(enet_pads);
	setup_fec_clock();
	eth_phy_reset();

	return cpu_eth_init(bis);
}

static int ar8031_phy_fixup(struct phy_device *phydev)
{
	unsigned short val;

	/* To enable AR8031 output a 125MHz clk from CLK_25M */
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
	ar8031_phy_fixup(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC

#define USDHC2_CD_GPIO	IMX_GPIO_NR(1, 4)
static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{ USDHC2_BASE_ADDR },
	{ USDHC4_BASE_ADDR },
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		return !gpio_get_value(USDHC2_CD_GPIO);
	case USDHC4_BASE_ADDR:
		return 1; /* eMMC/uSDHC4 is always present */
	}

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int i, ret;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    micro SD
	 * mmc2                    eMMC
	 */
	gpio_direction_input(USDHC2_CD_GPIO);

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
}
#endif

#ifdef CONFIG_USB_EHCI_MX6
static void setup_usb(void)
{
	/*
	 * Set daisy chain for otg_pin_id on MX6Q.
	 * For MX6DL, this bit is reserved.
	 */
	imx_iomux_set_gpr_register(1, 13, 1, 0);
}

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return USB_INIT_DEVICE;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		break;
	case 1:
		gpio_direction_output(IMX_GPIO_NR(3, 31), !!on);
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return -EINVAL;
	}

	return 0;
}
#endif

int board_early_init_f(void)
{
#ifdef CONFIG_USB_EHCI_MX6
	setup_usb();
#endif

	return 0;
}

int board_init(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/* Enable eim_slow clocks */
	setbits_le32(&mxc_ccm->CCGR6, 0x1 << MXC_CCM_CCGR6_EMI_SLOW_OFFSET);

	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &kp_imx6q_tpc_i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &kp_imx6q_tpc_i2c_pad_info1);

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd2",  MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc", MAKE_CFGVAL(0x60, 0x58, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	env_set("boardname", "kp-tpc");
	env_set("boardsoc", "imx6q");
	return 0;
}

int checkboard(void)
{
	puts("Board: K+P KP_IMX6Q_TPC i.MX6Q\n");
	return 0;
}
