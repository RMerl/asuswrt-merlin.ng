// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 DENX
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * Derived from code written by Robert Aigner (ra@spiid.net)
 *
 * Itself derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 */
#include <common.h>
#include <dm.h>
#include <netdev.h>
#include <ns16550.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <asm/mach-types.h>
#include <asm/omap_mmc.h>
#include "cairo.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);
	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_CAIRO();
}

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#ifdef CONFIG_SPL_BUILD
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on the first bank.  This
 * provides the timing values back to the function that configures
 * the memory.
 *
 * The Cairo board uses SAMSUNG DDR - K4X51163PG-FGC6
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	timings->sharing = SAMSUNG_SHARING;
	timings->mcfg = SAMSUNG_V_MCFG_165(128 << 20);
	timings->ctrla = SAMSUNG_V_ACTIMA_165;
	timings->ctrlb = SAMSUNG_V_ACTIMB_165;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	timings->mr = SAMSUNG_V_MR_165;
}
#endif

static const struct ns16550_platdata cairo_serial = {
	.base = OMAP34XX_UART2,
	.reg_shift = 2,
	.clock = V_NS16550_CLK,
	.fcr = UART_FCR_DEFVAL,
};

U_BOOT_DEVICE(cairo_uart) = {
	"ns16550_serial",
	&cairo_serial
};

/* force SPL booting into U-Boot, not Linux */
#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	return 1;
}
#endif
