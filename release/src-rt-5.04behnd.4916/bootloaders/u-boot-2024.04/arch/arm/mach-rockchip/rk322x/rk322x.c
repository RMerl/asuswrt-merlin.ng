// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */
#include <init.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/grf_rk322x.h>
#include <asm/arch-rockchip/hardware.h>

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@30020000",
	[BROM_BOOTSOURCE_SD] = "/mmc@30000000",
};

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
#define GRF_BASE	0x11000000
	static struct rk322x_grf * const grf = (void *)GRF_BASE;
	enum {
		GPIO1B2_SHIFT		= 4,
		GPIO1B2_MASK		= 3 << GPIO1B2_SHIFT,
		GPIO1B2_GPIO            = 0,
		GPIO1B2_UART1_SIN,
		GPIO1B2_UART21_SIN,

		GPIO1B1_SHIFT		= 2,
		GPIO1B1_MASK		= 3 << GPIO1B1_SHIFT,
		GPIO1B1_GPIO            = 0,
		GPIO1B1_UART1_SOUT,
		GPIO1B1_UART21_SOUT,
	};
	enum {
		CON_IOMUX_UART2SEL_SHIFT = 8,
		CON_IOMUX_UART2SEL_MASK	= 1 << CON_IOMUX_UART2SEL_SHIFT,
		CON_IOMUX_UART2SEL_2	= 0,
		CON_IOMUX_UART2SEL_21,
	};

	/* Enable early UART2 channel 1 on the RK322x */
	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B1_MASK | GPIO1B2_MASK,
		     GPIO1B2_UART21_SIN << GPIO1B2_SHIFT |
		     GPIO1B1_UART21_SOUT << GPIO1B1_SHIFT);
	/* Set channel C as UART2 input */
	rk_clrsetreg(&grf->con_iomux,
		     CON_IOMUX_UART2SEL_MASK,
		     CON_IOMUX_UART2SEL_21 << CON_IOMUX_UART2SEL_SHIFT);
}
#endif

int arch_cpu_init(void)
{
#ifdef CONFIG_SPL_BUILD
#define SGRF_BASE	0x10150000
	static struct rk322x_sgrf * const sgrf = (void *)SGRF_BASE;

	/* Disable the ddr secure region setting to make it non-secure */
	rk_clrreg(&sgrf->soc_con[0], 0x4000);
#else
#define GRF_BASE	0x11000000
	static struct rk322x_grf * const grf = (void *)GRF_BASE;
	/*
	 * The integrated macphy is enabled by default, disable it
	 * for saving power consuming.
	 */
	rk_clrsetreg(&grf->macphy_con[0],
		     MACPHY_CFG_ENABLE_MASK,
		     0 << MACPHY_CFG_ENABLE_SHIFT);

#endif
	return 0;
}
