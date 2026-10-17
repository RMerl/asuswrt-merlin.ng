// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Toradex
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>

#include <asm/arch/clock.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/snvs_security_sc.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <env.h>
#include <errno.h>
#include <linux/libfdt.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart3_pads[] = {
	SC_P_FLEXCAN2_RX | MUX_MODE_ALT(2) | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_FLEXCAN2_TX | MUX_MODE_ALT(2) | MUX_PAD_CTRL(UART_PAD_CTRL),
	/* Transceiver FORCEOFF# signal, mux to use pull-up */
	SC_P_QSPI0B_DQS | MUX_MODE_ALT(4) | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
}

static int is_imx8dx(void)
{
	u32 val = 0;
	int sc_err = sc_misc_otp_fuse_read(-1, 6, &val);

	if (!sc_err) {
		/* DX has two A35 cores disabled */
		return (val & 0xf) != 0x0;
	}
	return false;
}

void board_mem_get_layout(u64 *phys_sdram_1_start,
			  u64 *phys_sdram_1_size,
			  u64 *phys_sdram_2_start,
			  u64 *phys_sdram_2_size)
{
	*phys_sdram_1_start = PHYS_SDRAM_1;
	if (is_imx8dx())
		/* Our DX based SKUs only have 1 GB RAM */
		*phys_sdram_1_size = SZ_1G;
	else
		*phys_sdram_1_size = PHYS_SDRAM_1_SIZE;
	*phys_sdram_2_start = PHYS_SDRAM_2;
	*phys_sdram_2_size = PHYS_SDRAM_2_SIZE;
}

int board_early_init_f(void)
{
	sc_pm_clock_rate_t rate;
	int err;

	/*
	 * This works around that having only UART3 up the baudrate is 1.2M
	 * instead of 115.2k. Set UART0 clock root to 80 MHz
	 */
	rate = 80000000;
	err = sc_pm_set_clock_rate(-1, SC_R_UART_0, SC_PM_CLK_PER, &rate);
	if (err)
		return 0;

	/* Set UART3 clock root to 80 MHz and enable it */
	rate = SC_80MHZ;
	err = sc_pm_setup_uart(SC_R_UART_3, rate);
	if (err)
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

static void select_dt_from_module_version(void)
{
	/*
	 * The dtb filename is constructed from ${soc}-colibri-${fdt_board}.dtb.
	 * Set soc depending on the used SoC.
	 */
	if (is_imx8dx())
		env_set("soc", "imx8dx");
	else
		env_set("soc", "imx8qxp");
}

int board_init(void)
{
	board_gpio_init();

	if (IS_ENABLED(CONFIG_IMX_SNVS_SEC_SC_AUTO)) {
		int ret = snvs_security_sc_init();

		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(void)
{
	/* TODO */
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
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
	env_set("board_name", "Colibri iMX8QXP");
	env_set("board_rev", "v1.0");
#endif

	build_info();

	select_dt_from_module_version();

	return 0;
}
