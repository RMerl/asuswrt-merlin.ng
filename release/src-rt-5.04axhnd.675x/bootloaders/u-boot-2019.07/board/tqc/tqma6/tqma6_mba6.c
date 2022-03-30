// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013, 2014 TQ Systems (ported SabreSD to TQMa6x)
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 */

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>

#include <common.h>
#include <fsl_esdhc.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <i2c.h>
#include <micrel.h>
#include <miiphy.h>
#include <mmc.h>
#include <netdev.h>

#include "tqma6_bb.h"

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#if defined(CONFIG_TQMA6Q)

#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII	0x02e0790
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM	0x02e07ac

#elif defined(CONFIG_TQMA6S) || defined(CONFIG_TQMA6DL)

#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII	0x02e0768
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM	0x02e0788

#else

#error "need to select module"

#endif

#define ENET_RX_PAD_CTRL	(PAD_CTL_DSE_34ohm)
#define ENET_TX_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_DSE_34ohm)
#define ENET_CLK_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_HIGH | \
				 PAD_CTL_DSE_34ohm)
#define ENET_MDIO_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_60ohm)

/* disable on die termination for RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE	0x00000000
/* optimised drive strength for 1.0 .. 1.3 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P2V	0x00080000
/* optimised drive strength for 1.3 .. 2.5 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V	0x000C0000

#define ENET_PHY_RESET_GPIO IMX_GPIO_NR(1, 25)

static iomux_v3_cfg_t const mba6_enet_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_ENET_MDIO__ENET_MDIO,	ENET_MDIO_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_MDC__ENET_MDC,	ENET_MDIO_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_RGMII_TXC__RGMII_TXC,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD0__RGMII_TD0,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD1__RGMII_TD1,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD2__RGMII_TD2,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD3__RGMII_TD3,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL,
		     ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_REF_CLK__ENET_TX_CLK,	ENET_CLK_PAD_CTRL),
	/*
	 * these pins are also used for config strapping by phy
	 */
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD0__RGMII_RD0,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD1__RGMII_RD1,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD2__RGMII_RD2,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD3__RGMII_RD3,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RXC__RGMII_RXC,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL,
		     ENET_RX_PAD_CTRL),
	/* KSZ9031 PHY Reset */
	NEW_PAD_CTRL(MX6_PAD_ENET_CRS_DV__GPIO1_IO25,	GPIO_OUT_PAD_CTRL),
};

static void mba6_setup_iomuxc_enet(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* clear gpr1[ENET_CLK_SEL] for externel clock */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE,
		     (void *)IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V,
		     (void *)IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);

	imx_iomux_v3_setup_multiple_pads(mba6_enet_pads,
					 ARRAY_SIZE(mba6_enet_pads));

	/* Reset PHY */
	gpio_direction_output(ENET_PHY_RESET_GPIO , 0);
	/* Need delay 10ms after power on according to KSZ9031 spec */
	mdelay(10);
	gpio_set_value(ENET_PHY_RESET_GPIO, 1);
	/*
	 * KSZ9031 manual: 100 usec wait time after reset before communication
	 * over MDIO
	 * BUGBUG: hardware has an RC const that needs > 10 msec from 0->1 on
	 * reset before the phy sees a high level
	 */
	mdelay(15);
}

static iomux_v3_cfg_t const mba6_uart2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD4_DAT4__UART2_RX_DATA, UART_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD4_DAT7__UART2_TX_DATA, UART_PAD_CTRL),
};

static void mba6_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba6_uart2_pads,
					 ARRAY_SIZE(mba6_uart2_pads));
}

#define USDHC2_CD_GPIO	IMX_GPIO_NR(1, 4)
#define USDHC2_WP_GPIO	IMX_GPIO_NR(1, 2)

int tqma6_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = !gpio_get_value(USDHC2_CD_GPIO);

	return ret;
}

int tqma6_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = gpio_get_value(USDHC2_WP_GPIO);

	return ret;
}

static struct fsl_esdhc_cfg mba6_usdhc_cfg = {
	.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 4,
};

static iomux_v3_cfg_t const mba6_usdhc2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD2_CLK__SD2_CLK,		USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_CMD__SD2_CMD,		USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT0__SD2_DATA0,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT1__SD2_DATA1,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT2__SD2_DATA2,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT3__SD2_DATA3,	USDHC_PAD_CTRL),
	/* CD */
	NEW_PAD_CTRL(MX6_PAD_GPIO_4__GPIO1_IO04,	GPIO_IN_PAD_CTRL),
	/* WP */
	NEW_PAD_CTRL(MX6_PAD_GPIO_2__GPIO1_IO02,	GPIO_IN_PAD_CTRL),
};

int tqma6_bb_board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(mba6_usdhc2_pads,
					 ARRAY_SIZE(mba6_usdhc2_pads));
	gpio_direction_input(USDHC2_CD_GPIO);
	gpio_direction_input(USDHC2_WP_GPIO);

	mba6_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	if (fsl_esdhc_initialize(bis, &mba6_usdhc_cfg))
		puts("Warning: failed to initialize SD\n");

	return 0;
}

static struct i2c_pads_info mba6_i2c1_pads = {
/* I2C1: MBa6x */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT9__I2C1_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT9__GPIO5_IO27,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT8__I2C1_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT8__GPIO5_IO26,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 26)
	}
};

static void mba6_setup_i2c(void)
{
	int ret;
	/*
	 * use logical index for bus, e.g. I2C1 -> 0
	 * warn on error
	 */
	ret = setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mba6_i2c1_pads);
	if (ret)
		printf("setup I2C1 failed: %d\n", ret);
}

int board_phy_config(struct phy_device *phydev)
{
/*
 * optimized pad skew values depends on CPU variant on the TQMa6x module:
 * CONFIG_TQMA6Q: i.MX6Q/D
 * CONFIG_TQMA6S: i.MX6S
 * CONFIG_TQMA6DL: i.MX6DL
 */
#if defined(CONFIG_TQMA6Q)
#define MBA6X_KSZ9031_CTRL_SKEW	0x0032
#define MBA6X_KSZ9031_CLK_SKEW	0x03ff
#define MBA6X_KSZ9031_RX_SKEW	0x3333
#define MBA6X_KSZ9031_TX_SKEW	0x2036
#elif defined(CONFIG_TQMA6S) || defined(CONFIG_TQMA6DL)
#define MBA6X_KSZ9031_CTRL_SKEW	0x0030
#define MBA6X_KSZ9031_CLK_SKEW	0x03ff
#define MBA6X_KSZ9031_RX_SKEW	0x3333
#define MBA6X_KSZ9031_TX_SKEW	0x2052
#else
#error
#endif
	/* min rx/tx ctrl delay */
	ksz9031_phy_extended_write(phydev, 2,
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MBA6X_KSZ9031_CTRL_SKEW);
	/* min rx delay */
	ksz9031_phy_extended_write(phydev, 2,
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MBA6X_KSZ9031_RX_SKEW);
	/* max tx delay */
	ksz9031_phy_extended_write(phydev, 2,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MBA6X_KSZ9031_TX_SKEW);
	/* rx/tx clk skew */
	ksz9031_phy_extended_write(phydev, 2,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MBA6X_KSZ9031_CLK_SKEW);

	phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	bus = fec_get_miibus(base, -1);
	if (!bus)
		return -EINVAL;
	/* scan phy */
	phydev = phy_find_by_mask(bus, (0xf << CONFIG_FEC_MXC_PHYADDR),
					PHY_INTERFACE_MODE_RGMII);

	if (!phydev) {
		ret = -EINVAL;
		goto free_bus;
	}
	ret  = fec_probe(bis, -1, base, bus, phydev);
	if (ret)
		goto free_phydev;

	return 0;

free_phydev:
	free(phydev);
free_bus:
	free(bus);
	return ret;
}

int tqma6_bb_board_early_init_f(void)
{
	mba6_setup_iomuxc_uart();

	return 0;
}

int tqma6_bb_board_init(void)
{
	mba6_setup_i2c();
	/* do it here - to have reset completed */
	mba6_setup_iomuxc_enet();

	return 0;
}

int tqma6_bb_board_late_init(void)
{
	return 0;
}

const char *tqma6_bb_get_boardname(void)
{
	return "MBa6x";
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void tqma6_bb_ft_board_setup(void *blob, bd_t *bd)
{
 /* TBD */
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
