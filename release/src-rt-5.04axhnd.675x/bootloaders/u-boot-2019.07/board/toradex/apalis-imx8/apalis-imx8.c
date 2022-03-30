// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Toradex
 */

#include <common.h>

#include <asm/arch/clock.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <environment.h>
#include <errno.h>
#include <linux/libfdt.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart1_pads[] = {
	SC_P_UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

int board_early_init_f(void)
{
	sc_pm_clock_rate_t rate;
	sc_err_t err = 0;

	/* Power up UART1 */
	err = sc_pm_set_resource_power_mode(-1, SC_R_UART_1, SC_PM_PW_MODE_ON);
	if (err != SC_ERR_NONE)
		return 0;

	/* Set UART3 clock root to 80 MHz */
	rate = 80000000;
	err = sc_pm_set_clock_rate(-1, SC_R_UART_1, SC_PM_CLK_PER, &rate);
	if (err != SC_ERR_NONE)
		return 0;

	/* Enable UART1 clock root */
	err = sc_pm_clock_enable(-1, SC_R_UART_1, SC_PM_CLK_PER, true, false);
	if (err != SC_ERR_NONE)
		return 0;

	setup_iomux_uart();

	return 0;
}

#if IS_ENABLED(CONFIG_DM_GPIO)
static void board_gpio_init(void)
{
	/* TODO */
}
#else
static inline void board_gpio_init(void) {}
#endif

#if IS_ENABLED(CONFIG_FEC_MXC)
#include <miiphy.h>

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

void build_info(void)
{
	u32 sc_build = 0, sc_commit = 0;

	/* Get SCFW build and commit id */
	sc_misc_build_info(-1, &sc_build, &sc_commit);
	if (!sc_build) {
		printf("SCFW does not support build info\n");
		sc_commit = 0; /* Display 0 if build info not supported */
	}
	printf("Build: SCFW %x\n", sc_commit);
}

int checkboard(void)
{
	puts("Model: Toradex Apalis iMX8\n");

	build_info();
	print_bootinfo();

	return 0;
}

int board_init(void)
{
	board_gpio_init();

	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
	/* TODO */
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
/* TODO move to common */
	env_set("board_name", "Apalis iMX8QM");
	env_set("board_rev", "v1.0");
#endif

	return 0;
}
