// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2009 Magnus Lilja <lilja.magnus@gmail.com>
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 */


#include <common.h>
#include <netdev.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <watchdog.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong bootflag)
{
	/*
	 * copy ourselves from where we are running to where we were
	 * linked at. Use ulong pointers as all addresses involved
	 * are 4-byte-aligned.
	 */
	ulong *start_ptr, *end_ptr, *link_ptr, *run_ptr, *dst;
	asm volatile ("ldr %0, =_start" : "=r"(start_ptr));
	asm volatile ("ldr %0, =_end" : "=r"(end_ptr));
	asm volatile ("ldr %0, =board_init_f" : "=r"(link_ptr));
	asm volatile ("adr %0, board_init_f" : "=r"(run_ptr));
	for (dst = start_ptr; dst < end_ptr; dst++)
		*dst = *(dst+(run_ptr-link_ptr));
	/*
	 * branch to nand_boot's link-time address.
	 */
	asm volatile("ldr pc, =nand_boot");
}
#endif

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

int board_early_init_f(void)
{
	/* CS5: CPLD incl. network controller */
	static const struct mxc_weimcs cs5 = {
		/*    sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  3, 24, 0,  4,  3),
		/*   oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(2,  2,   2,   5,  2,  0,  5,  2,  0,  0,   0,   1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(2,   2,  2,  2,  0,  0,  2,  2,  0,  0,  0,  0,   0,  0)
	};

	mxc_setup_weimcs(5, &cs5);

	/* Setup UART1 and SPI2 pins */
	mx31_uart1_hw_init();
	mx31_spi2_hw_init();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int board_late_init(void)
{
	u32 val;
	struct pmic *p;
	int ret;

	ret = pmic_init(CONFIG_FSL_PMIC_BUS);
	if (ret)
		return ret;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return -ENODEV;
	/* Enable RTC battery */
	pmic_reg_read(p, REG_POWER_CTL0, &val);
	pmic_reg_write(p, REG_POWER_CTL0, val | COINCHEN);
	pmic_reg_write(p, REG_INT_STATUS1, RTCRSTI);
#ifdef CONFIG_HW_WATCHDOG
	hw_watchdog_init();
#endif
	return 0;
}

int checkboard(void)
{
	printf("Board: MX31PDK\n");
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
