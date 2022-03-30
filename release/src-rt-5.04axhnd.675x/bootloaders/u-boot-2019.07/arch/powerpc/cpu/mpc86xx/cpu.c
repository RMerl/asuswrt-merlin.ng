// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2006,2009-2010 Freescale Semiconductor, Inc.
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <asm/mmu.h>
#include <mpc86xx.h>
#include <asm/fsl_law.h>
#include <asm/ppc.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Default board reset function
 */
static void
__board_reset(void)
{
	/* Do nothing */
}
void board_reset(void) __attribute__((weak, alias("__board_reset")));


int
checkcpu(void)
{
	sys_info_t sysinfo;
	uint pvr, svr;
	uint major, minor;
	char buf1[32], buf2[32];
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	struct cpu_type *cpu;
	uint msscr0 = mfspr(MSSCR0);

	svr = get_svr();
	major = SVR_MAJ(svr);
	minor = SVR_MIN(svr);

	if (cpu_numcores() > 1) {
#ifndef CONFIG_MP
		puts("Unicore software on multiprocessor system!!\n"
		     "To enable mutlticore build define CONFIG_MP\n");
#endif
	}
	puts("CPU:   ");

	cpu = gd->arch.cpu;

	puts(cpu->name);

	printf(", Version: %d.%d, (0x%08x)\n", major, minor, svr);
	puts("Core:  ");

	pvr = get_pvr();
	major = PVR_E600_MAJ(pvr);
	minor = PVR_E600_MIN(pvr);

	printf("e600 Core %d", (msscr0 & 0x20) ? 1 : 0);
	if (gur->pordevsr & MPC86xx_PORDEVSR_CORE1TE)
		puts("\n    Core1Translation Enabled");
	debug(" (MSSCR0=%x, PORDEVSR=%x)", msscr0, gur->pordevsr);

	printf(", Version: %d.%d, (0x%08x)\n", major, minor, pvr);

	get_sys_info(&sysinfo);

	puts("Clock Configuration:\n");
	printf("       CPU:%-4s MHz, ", strmhz(buf1, sysinfo.freq_processor));
	printf("MPX:%-4s MHz\n", strmhz(buf1, sysinfo.freq_systembus));
	printf("       DDR:%-4s MHz (%s MT/s data rate), ",
		strmhz(buf1, sysinfo.freq_systembus / 2),
		strmhz(buf2, sysinfo.freq_systembus));

	if (sysinfo.freq_localbus > LCRR_CLKDIV) {
		printf("LBC:%-4s MHz\n", strmhz(buf1, sysinfo.freq_localbus));
	} else {
		printf("LBC: unknown (LCRR[CLKDIV] = 0x%02lx)\n",
		       sysinfo.freq_localbus);
	}

	puts("L1:    D-cache 32 KiB enabled\n");
	puts("       I-cache 32 KiB enabled\n");

	puts("L2:    ");
	if (get_l2cr() & 0x80000000) {
#if defined(CONFIG_ARCH_MPC8610)
		puts("256");
#elif defined(CONFIG_ARCH_MPC8641)
		puts("512");
#endif
		puts(" KiB enabled\n");
	} else {
		puts("Disabled\n");
	}

	return 0;
}


int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;

	/* Attempt board-specific reset */
	board_reset();

	/* Next try asserting HRESET_REQ */
	out_be32(&gur->rstcr, MPC86xx_RSTCR_HRST_REQ);

	while (1)
		;

	return 1;
}


/*
 * Get timebase clock frequency
 */
unsigned long
get_tbclk(void)
{
	sys_info_t sys_info;

	get_sys_info(&sys_info);
	return (sys_info.freq_systembus + 3L) / 4L;
}


#if defined(CONFIG_WATCHDOG)
void
watchdog_reset(void)
{
#if defined(CONFIG_ARCH_MPC8610)
	/*
	 * This actually feed the hard enabled watchdog.
	 */
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_wdt_t *wdt = &immap->im_wdt;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	u32 tmp = gur->pordevsr;

	if (tmp & 0x4000) {
		wdt->swsrr = 0x556c;
		wdt->swsrr = 0xaa39;
	}
#endif
}
#endif	/* CONFIG_WATCHDOG */

/*
 * Print out the state of various machine registers.
 * Currently prints out LAWs, BR0/OR0, and BATs
 */
void print_reginfo(void)
{
	print_bats();
	print_laws();
	print_lbc_regs();
}

/*
 * Set the DDR BATs to reflect the actual size of DDR.
 *
 * dram_size is the actual size of DDR, in bytes
 *
 * Note: we assume that CONFIG_MAX_MEM_MAPPED is 2G or smaller as we only
 * are using a single BAT to cover DDR.
 *
 * If this is not true, (e.g. CONFIG_MAX_MEM_MAPPED is 2GB but HID0_XBSEN
 * is not defined) then we might have a situation where U-Boot will attempt
 * to relocated itself outside of the region mapped by DBAT0.
 * This will cause a machine check.
 *
 * Currently we are limited to power of two sized DDR since we only use a
 * single bat.  If a non-power of two size is used that is less than
 * CONFIG_MAX_MEM_MAPPED u-boot will crash.
 *
 */
void setup_ddr_bat(phys_addr_t dram_size)
{
	unsigned long batu, bl;

	bl = TO_BATU_BL(min(dram_size, CONFIG_MAX_MEM_MAPPED));

	if (BATU_SIZE(bl) != dram_size) {
		u64 sz = (u64)dram_size - BATU_SIZE(bl);
		print_size(sz, " left unmapped\n");
	}

	batu = bl | BATU_VS | BATU_VP;
	write_bat(DBAT0, batu, CONFIG_SYS_DBAT0L);
	write_bat(IBAT0, batu, CONFIG_SYS_IBAT0L);
}
