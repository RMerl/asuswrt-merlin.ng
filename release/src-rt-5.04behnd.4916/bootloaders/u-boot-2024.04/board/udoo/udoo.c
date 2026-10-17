// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <init.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <env.h>
#include <malloc.h>
#include <asm/arch/mx6-pins.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sata.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |                   \
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define WDT_EN		IMX_GPIO_NR(5, 4)
#define WDT_TRG		IMX_GPIO_NR(3, 19)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart2_pads[] = {
	IOMUX_PADS(PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	IOMUX_PADS(PAD_EIM_A24__GPIO5_IO04 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D19__GPIO3_IO19),
};

int mx6_rgmii_rework(struct phy_device *phydev)
{
	/*
	 * Bug: Apparently uDoo does not works with Gigabit switches...
	 * Limiting speed to 10/100Mbps, and setting master mode, seems to
	 * be the only way to have a successfull PHY auto negotiation.
	 * How to fix: Understand why Linux kernel do not have this issue.
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, 0x1c00);

	/* control data pad skew - devaddr = 0x02, register = 0x04 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* rx data pad skew - devaddr = 0x02, register = 0x05 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* tx data pad skew - devaddr = 0x02, register = 0x05 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* gtx and rx clock pad skew - devaddr = 0x02, register = 0x08 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x03FF);
	return 0;
}

static void setup_iomux_enet(void)
{
	gpio_request(IMX_GPIO_NR(2, 31), "eth_power");
	gpio_request(IMX_GPIO_NR(3, 23), "eth_phy_reset");
	gpio_request(IMX_GPIO_NR(6, 24), "strap1");
	gpio_request(IMX_GPIO_NR(6, 25), "strap2");
	gpio_request(IMX_GPIO_NR(6, 27), "strap3");
	gpio_request(IMX_GPIO_NR(6, 28), "strap4");
	gpio_request(IMX_GPIO_NR(6, 29), "strap5");

	gpio_direction_output(IMX_GPIO_NR(2, 31), 1); /* Power supply on */

	gpio_direction_output(IMX_GPIO_NR(3, 23), 0); /* assert PHY rst */

	gpio_direction_output(IMX_GPIO_NR(6, 24), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 25), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 27), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 28), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 29), 1);
	udelay(1000);

	gpio_set_value(IMX_GPIO_NR(3, 23), 1); /* deassert PHY rst */

	/* Need 100ms delay to exit from reset. */
	udelay(1000 * 100);

	gpio_free(IMX_GPIO_NR(6, 24));
	gpio_free(IMX_GPIO_NR(6, 25));
	gpio_free(IMX_GPIO_NR(6, 27));
	gpio_free(IMX_GPIO_NR(6, 28));
	gpio_free(IMX_GPIO_NR(6, 29));
}

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart2_pads);
}

static void setup_iomux_wdog(void)
{
	SETUP_IOMUX_PADS(wdog_pads);
	gpio_direction_output(WDT_TRG, 0);
	gpio_direction_output(WDT_EN, 1);
	gpio_direction_input(WDT_TRG);
}

int board_early_init_f(void)
{
	setup_iomux_wdog();
	setup_iomux_uart();

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (is_cpu_type(MXC_CPU_MX6Q))
		env_set("board_rev", "MX6Q");
	else
		env_set("board_rev", "MX6DL");
#endif
	setup_iomux_enet();

	return 0;
}

int checkboard(void)
{
	if (is_cpu_type(MXC_CPU_MX6Q))
		puts("Board: Udoo Quad\n");
	else
		puts("Board: Udoo DualLite\n");

	return 0;
}
