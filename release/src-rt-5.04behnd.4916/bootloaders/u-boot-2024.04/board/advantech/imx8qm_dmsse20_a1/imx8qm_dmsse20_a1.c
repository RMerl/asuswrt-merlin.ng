// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2018 NXP
 * Copyright 2019-2023 Kococonnector GmbH
 */

#include <common.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
/* #include <power-domain.h> */

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart0_pads[] = {
	SC_P_UART0_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART0_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart0_pads, ARRAY_SIZE(uart0_pads));
}

int board_early_init_f(void)
{
	sc_pm_clock_rate_t rate = SC_80MHZ;
	int ret;

	/* Set UART0 clock root to 80 MHz */
	ret = sc_pm_setup_uart(SC_R_UART_0, rate);
	if (ret)
		return ret;

	setup_iomux_uart();

	/* This is needed to because Kernel do not Power Up DC_0 */
	sc_pm_set_resource_power_mode(-1, SC_R_DC_0, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(-1, SC_R_GPIO_5, SC_PM_PW_MODE_ON);

	return 0;
}

#if IS_ENABLED(CONFIG_FEC_MXC)
#include <miiphy.h>

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

#ifdef CONFIG_MXC_GPIO

#define LVDS_ENABLE IMX_GPIO_NR(1, 6)
#define MIPI_ENABLE IMX_GPIO_NR(1, 7)

#define BB_GPIO_3V3_1 IMX_GPIO_NR(4, 20)
#define BB_GPIO_3V3_2 IMX_GPIO_NR(4, 24)
#define BB_GPIO_3V3_3 IMX_GPIO_NR(4, 23)

static void board_gpio_init(void)
{
	/* Enable BB 3V3 */
	gpio_request(BB_GPIO_3V3_1, "bb_3v3_1");
	gpio_direction_output(BB_GPIO_3V3_1, 1);
	gpio_request(BB_GPIO_3V3_2, "bb_3v3_2");
	gpio_direction_output(BB_GPIO_3V3_2, 1);
	gpio_request(BB_GPIO_3V3_3, "bb_3v3_3");
	gpio_direction_output(BB_GPIO_3V3_3, 1);

	/* enable LVDS SAS boards */
	gpio_request(LVDS_ENABLE, "lvds_enable");
	gpio_direction_output(LVDS_ENABLE, 1);

	/* enable MIPI SAS boards */
	gpio_request(MIPI_ENABLE, "mipi_enable");
	gpio_direction_output(MIPI_ENABLE, 1);
}
#endif

int checkboard(void)
{
	puts("Board: DMS-SE20A1 8GB\n");
	build_info();
	print_bootinfo();

	return 0;
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_XEN))
		return 0;

#ifdef CONFIG_MXC_GPIO
	board_gpio_init();
#endif

	return 0;
}

void board_quiesce_devices(void)
{
	if (IS_ENABLED(CONFIG_XEN)) {
		/* Clear magic number to let xen know uboot is over */
		writel(0x0, (void __iomem *)0x80000000);
		return;
	}
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(void)
{
	puts("SCI reboot request");

	while (1)
		putc('.');
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

int board_mmc_get_env_dev(int devno)
{
	/* Use EMMC */
	if (IS_ENABLED(CONFIG_XEN))
		return 0;

	return devno;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	/* Use EMMC */
	if (IS_ENABLED(CONFIG_XEN))
		return 0;

	return dev_no;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "DMS-SE20A1");
	env_set("board_rev", "iMX8QM");
#endif

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

	return 0;
}
