// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 */
#include <common.h>
#include <bootstage.h>
#include <cpu_func.h>
#include <init.h>
#include <malloc.h>
#include <errno.h>
#include <net.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/systimer.h>
#include <asm/arch/sysctrl.h>
#include <asm/arch/wdt.h>

static struct systimer *systimer_base = (struct systimer *)V2M_TIMER01;
static struct sysctrl *sysctrl_base = (struct sysctrl *)SCTL_BASE;

static void flash__init(void);
static void vexpress_timer_init(void);
DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

static inline void delay(ulong loops)
{
	__asm__ volatile ("1:\n"
		"subs %0, %1, #1\n"
		"bne 1b" : "=r" (loops) : "0" (loops));
}

int board_init(void)
{
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;
	gd->bd->bi_arch_number = MACH_TYPE_VEXPRESS;

	icache_enable();
	flash__init();
	vexpress_timer_init();

	return 0;
}

static void flash__init(void)
{
	/* Setup the sytem control register to allow writing to flash */
	writel(readl(&sysctrl_base->scflashctrl) | VEXPRESS_FLASHPROG_FLVPPEN,
	       &sysctrl_base->scflashctrl);
}

int dram_init(void)
{
	gd->ram_size =
		get_ram_size((long *)CFG_SYS_SDRAM_BASE, PHYS_SDRAM_1_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size =
			get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size =
			get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE);

	return 0;
}

/*
 * Start timer:
 *    Setup a 32 bit timer, running at 1KHz
 *    Versatile Express Motherboard provides 1 MHz timer
 */
static void vexpress_timer_init(void)
{
	/*
	 * Set clock frequency in system controller:
	 *   VEXPRESS_REFCLK is 32KHz
	 *   VEXPRESS_TIMCLK is 1MHz
	 */
	writel(SP810_TIMER0_ENSEL | SP810_TIMER1_ENSEL |
	       SP810_TIMER2_ENSEL | SP810_TIMER3_ENSEL |
	       readl(&sysctrl_base->scctrl), &sysctrl_base->scctrl);

	/*
	 * Set Timer0 to be:
	 *   Enabled, free running, no interrupt, 32-bit, wrapping
	 */
	writel(SYSTIMER_RELOAD, &systimer_base->timer0load);
	writel(SYSTIMER_RELOAD, &systimer_base->timer0value);
	writel(SYSTIMER_EN | SYSTIMER_32BIT |
	       readl(&systimer_base->timer0control),
	       &systimer_base->timer0control);
}

int v2m_cfg_write(u32 devfn, u32 data)
{
	/* Configuration interface broken? */
	u32 val;

	devfn |= SYS_CFG_START | SYS_CFG_WRITE;

	val = readl(V2M_SYS_CFGSTAT);
	writel(val & ~SYS_CFG_COMPLETE, V2M_SYS_CFGSTAT);

	writel(data, V2M_SYS_CFGDATA);
	writel(devfn, V2M_SYS_CFGCTRL);

	do {
		val = readl(V2M_SYS_CFGSTAT);
	} while (val == 0);

	return !!(val & SYS_CFG_ERR);
}

/* Use the ARM Watchdog System to cause reset */
void reset_cpu(void)
{
	if (v2m_cfg_write(SYS_CFG_REBOOT | SYS_CFG_SITE_MB, 0))
		printf("Unable to reboot\n");
}

void lowlevel_init(void)
{
}

ulong get_board_rev(void){
	return readl((u32 *)SYS_ID);
}

#ifdef CONFIG_ARMV7_NONSEC
/* Setting the address at which secondary cores start from.
 * Versatile Express uses one address for all cores, so ignore corenr
 */
void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
	/* The SYSFLAGS register on VExpress needs to be cleared first
	 * by writing to the next address, since any writes to the address
	 * at offset 0 will only be ORed in
	 */
	writel(~0, CONFIG_SYSFLAGS_ADDR + 4);
	writel(addr, CONFIG_SYSFLAGS_ADDR);
}
#endif
