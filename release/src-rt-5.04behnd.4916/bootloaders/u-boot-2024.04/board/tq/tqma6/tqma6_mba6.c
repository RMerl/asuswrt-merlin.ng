// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013, 2014 TQ-Systems (ported SabreSD to TQMa6x)
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 */

#include <init.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>

#include <common.h>
#include <fsl_esdhc_imx.h>
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

/* disable on die termination for RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE	0x00000000
/* optimised drive strength for 1.0 .. 1.3 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P2V	0x00080000
/* optimised drive strength for 1.3 .. 2.5 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V	0x000C0000

static void mba6_setup_iomuxc_enet(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* clear gpr1[ENET_CLK_SEL] for externel clock */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE,
		     (void *)IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V,
		     (void *)IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);
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

int board_mmc_get_env_dev(int devno)
{
	/*
	 * This assumes that the baseboard registered
	 * the boot device first ...
	 * Note: SDHC3 == idx2
	 */
	return (2 == devno) ? 0 : 1;
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

int tqma6_bb_board_early_init_f(void)
{
	mba6_setup_iomuxc_uart();

	return 0;
}

int tqma6_bb_board_init(void)
{
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
void tqma6_bb_ft_board_setup(void *blob, struct bd_info *bd)
{
 /* TBD */
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
