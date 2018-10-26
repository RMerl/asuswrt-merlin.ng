#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 * BCM63148 SoC main platform file.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/sched.h>
#include <linux/stop_machine.h>
#include <asm/mach/map.h>
#include <asm/clkdev.h>
#include <asm/system_misc.h>
#include <mach/hardware.h>
#include <mach/smp.h>
#include <plat/bsp.h>
#include <plat/b15core.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <pmc_drv.h>
#include <BPCM.h>
#include <pmc_cpu_core.h>
#if defined(CONFIG_BCM_EXT_TIMER) && defined(CONFIG_PLAT_BCM63XX_EXT_TIMER)
#include <plat/bcm63xx_timer.h>
#endif

#define BUS_RANGE_3_DEFAULT_ULIMIT  0x3ffffU
#define BUS_RANGE_4_DEFAULT_ULIMIT 0x1bffffU

static volatile uint32_t core_set_freq_done, core_set_freq_core_1_rdy;

static struct clk ref_clk = {
	.name = "refclk",
	.rate = FREQ_MHZ(25),	/* run-time override */
	.fixed = 1,
	.type = CLK_XTAL,
};

static struct clk_lookup board_clk_lookups[] = {
	{
		.con_id = "refclk",
		.clk = &ref_clk,
	},
};

#define IO_DESC(pa, sz) { \
		.virtual = IO_ADDRESS(pa), \
		.pfn = __phys_to_pfn(pa), \
		.length = sz, \
		.type = MT_DEVICE, \
	}

#define MEM_DESC(pa, sz) { \
		.virtual = IO_ADDRESS(pa), \
		.pfn = __phys_to_pfn(pa), \
		.length = sz, \
		.type = MT_MEMORY_RWX_NONCACHED, \
	}


static struct map_desc bcm63148_io_desc[] __initdata = {
	IO_DESC(USB_CTL_PHYS_BASE, SZ_4K),
	IO_DESC(MEMC_PHYS_BASE, SZ_4K),
	IO_DESC(DDRPHY_PHYS_BASE, SZ_4K),
	IO_DESC(SAR_PHYS_BASE, SZ_16K),
	IO_DESC(SATA_PHYS_BASE, SZ_16K),
	IO_DESC(USBH_PHYS_BASE, SZ_8K),
	IO_DESC(ERROR_PORT_PHYS_BASE, SZ_4K),
//	IO_DESC(L2C_PHYS_BASE, SZ_4K),
	IO_DESC(B15_CTRL_PHYS_BASE, SZ_16K),	// FIXME! once we know the real RDB
	IO_DESC(DECT_PHYS_BASE, SZ_128K),
	IO_DESC(SWITCH_PHYS_BASE, SZ_512K),
	IO_DESC(APM_PHYS_BASE, SZ_128K),
	IO_DESC(RDP_PHYS_BASE, SZ_1M),
	IO_DESC(PMC_PHYS_BASE, SZ_512K),
	IO_DESC(PROC_MON_PHYS_BASE, SZ_4K),
	IO_DESC(DSLPHY_PHYS_BASE, SZ_1M),
	IO_DESC(DSLLMEM_PHYS_BASE, SZ_1M),
	IO_DESC(PERF_PHYS_BASE, SZ_16K),
	IO_DESC(BOOTLUT_PHYS_BASE, SZ_4K),
	IO_DESC(SPIFLASH_PHYS_BASE, SZ_128K),
	IO_DESC(NANDFLASH_PHYS_BASE, SZ_128K),
	/* FIXME!! more!! */
};

/* any fixup that has to be performed in the early stage of
 * kernel booting */
void __init soc_fixup(void)
{
	b15_fixup();
}

/*
 * Map fix-mapped I/O that is needed before full MMU operation
 */
void __init soc_map_io(void)
{
	b15_map_io();

	iotable_init(bcm63148_io_desc, ARRAY_SIZE(bcm63148_io_desc));
}

static int arm_wfi_allowed = 1; // administratively allowed

/* power is significantly reduced by re-enabling interrupts
 * and looping locally until a reschedule is needed.
 * nops would help further but create droops/spikes.
 */
__attribute__ (( aligned(16),hot ))
static void bcm63xx_arm_pm_idle(void)
{
	local_irq_enable();
	while (!need_resched());
}

// selective wfi enable/disable based on frequency
static void arm_wfi_enable(unsigned freqHz)
{
	/* enable only if administratively allowed and under 1500MHz */
	if (arm_wfi_allowed && freqHz < FREQ_MHZ(1500)) {
		arm_pm_idle = 0;
	} else {
		arm_pm_idle = bcm63xx_arm_pm_idle;
	}
}

/*
 * clk<n> = Fvco / mdiv<n>
 * where clk0 connects to B15,
 *       clk1 connects to MCP
 *       Fvco is 3GHz
 * clk0 is further scaled by 2^clk_ratio
 */

/* assume multiplier of 60 with 50MHz reference clock */
#define FOSC (60 * FREQ_MHZ(50u))

static unsigned get_arm_core_clk(void)
{
	unsigned ratio = B15CTRL->cpu_ctrl.clock_cfg & 7;
	const unsigned osc = FOSC;
	PLL_CHCFG_REG ch01_cfg;

	ReadBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(ch01_cfg), &ch01_cfg.Reg32);

	return (osc / ch01_cfg.Bits.mdiv0) >> ratio;
}

/*
 * CPU frequency can be changed via the B15 pll or clock-ratio
 *
 * Access to the pll is through bpcm so reads/writes are slow.
 * Access to the clock-ratio is through a fast soc register.
 *
 * To change the frequency from:
 *
 * 1:1 to 1:n
 * - stop all write traffic (i.e. stop all CPUs)
 * - set safe-clock-mode (clock configuration register)
 * - DSB
 * - set clock-divisor (clock configuration register)
 * - DSB
 * - start stopped CPUs
 *
 * 1:n to 1:1
 * - stop all write traffic (i.e. stop all CPUs)
 * - clear clock-divisor (clock configuration register)
 * - DSB
 * - clear safe-clock-mode (clock configuration register)
 * - DSB
 * - start stopped CPUs
 *
 * The configuration changes should be done close together and
 * as quickly as possible to limit the down time for other CPUS.
 * [this makes changing the clock-ratio preferrable to the pll]
 */
static int core_set_freq(unsigned shift)
{
	unsigned ratio = B15CTRL->cpu_ctrl.clock_cfg;
	const unsigned safe_mode = 16;

	// only one core running, no idlers;
	// enable/disable wfi for idlers
	arm_wfi_enable(FOSC/2 >> shift);

	if (shift != 0) {
		//A barrier here to ensure there are no pending memory accesses
		//when entering safe mode.
		smp_wmb();
		//Switching ARM DDR access over to UBUS temporarily. We need to make sure there's no
		//MCP activity when we enter Safe mode.
		B15CTRL->cpu_ctrl.bus_range[3].ulimit = (BUS_RANGE_3_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_UBUS;
		B15CTRL->cpu_ctrl.bus_range[4].ulimit = (BUS_RANGE_4_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_UBUS;
		//Read back to make sure the setting has taken effect before moving on.
		(void)B15CTRL->cpu_ctrl.bus_range[3].ulimit;
		(void)B15CTRL->cpu_ctrl.bus_range[4].ulimit;
		dsb();
		// set safe_clk_mode if < 1000MHz (2x 500MHz MCP)
		ratio |= safe_mode;
		B15CTRL->cpu_ctrl.clock_cfg = ratio; // set safe-mode
		//UBUS fast-ack makes above write operation a posted write.
		//Counter fast-ack by reading back the register. We want to
		//be sure the clock_cfg change has taken effect before
		//moving on.
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();

		ratio = (ratio & ~7) | shift;
		B15CTRL->cpu_ctrl.clock_cfg = ratio; // new divisor
		//Counter fast-ack
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();
		//Switching ARM DDR access back to MCP
		B15CTRL->cpu_ctrl.bus_range[3].ulimit = (BUS_RANGE_3_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_MCP0;
		B15CTRL->cpu_ctrl.bus_range[4].ulimit = (BUS_RANGE_4_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_MCP0;
		//Read back to make sure the setting has taken effect before moving on.
		(void)B15CTRL->cpu_ctrl.bus_range[3].ulimit;
		(void)B15CTRL->cpu_ctrl.bus_range[4].ulimit;
		dsb();
	} else {
		shift = ratio & 7;
		while (shift--) {
			// frequency doubling one step at a time
			ratio = (ratio & ~7) | shift;
			B15CTRL->cpu_ctrl.clock_cfg = ratio;
			//Counter fast-ack
			B15CTRL->cpu_ctrl.clock_cfg;
			if (shift <= 1) {
				// 50us spike mitigation at 750 & 1500MHz
				// tmrctl = enable | microseconds | 50
				PMC->ctrl.gpTmr0Ctl = (1 << 31) | (1 << 29) | 50;
				while (PMC->ctrl.gpTmr0Ctl & (1 << 31));
			}
		}

		//A barrier here to ensure there are no pending memory accesses
		//when exiting safe mode.
		smp_wmb();
		// clear safe_clk_mode if >= 1000MHz (2x 500MHz MCP)
		B15CTRL->cpu_ctrl.clock_cfg = ratio & ~safe_mode; // clear safe-mode
		//Counter fast-ack
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();
	}

	return 0;
}

static int core_set_freq_sync(void *p) {
	//Load variables used into cache. We don't want DDR accesses
	//in the code sequence below.
	(void)core_set_freq_core_1_rdy;
	(void)core_set_freq_done;

	if (smp_processor_id()==0) {
		//Core0 is doing the frequency change. Wait until core1
		//is ready for it. We have to make sure core1 is not
		//doing any memory accesses while core0 is changing
		//CPU frequency.
		//Deliberately using cached variables for inter-core
		//synchronization instead of atomic variables.
		//Atomic variable primitives would generate a memory
		//access because MegaBarriers are used.
		// check if the remote cpu is online
		if(cpumask_test_cpu(1, cpu_online_mask))
			while(!core_set_freq_core_1_rdy);
		
		core_set_freq(*(unsigned*)p);
		core_set_freq_done=1;
	}
	else {

		core_set_freq_core_1_rdy=1;
		//Wait until core0 is done changing frequency before moving on.
		while(!core_set_freq_done);

	}

	return 0;
}

/* freq in unit of Hz */
int soc_set_arm_core_clock(struct clk *cur_clk, unsigned long freqHz)
{
	unsigned shift;

	// change frequency through cpu ratio register
	// find power-of-2 divisor
	for (shift = 0; shift <= 4; shift++)
		/* default pll shift 2 */
		if ((FOSC/2 >> shift) == freqHz)
			break;
	if (shift > 4) {
		printk("Invalid cpu frequency %luMHz\n", freqHz / FREQ_MHZ(1));
		return -EINVAL;
	}

	cur_clk->rate = freqHz;
	smp_mb();

	core_set_freq_done=0;
	core_set_freq_core_1_rdy=0;
	__stop_machine(core_set_freq_sync, &shift, cpu_online_mask);

	return 0;
}

static struct clk_ops arm_clk_ops = {
	.enable = NULL,
	.disable = NULL,
	.round = NULL,
	.setrate = soc_set_arm_core_clock,
	.status = NULL,
};

void __init soc_init_clock(void)
{
	unsigned arm_periph_clk;

	pmc_init();

	arm_periph_clk = get_arm_core_clk();

	/* install clock source into the lookup table */
	clkdev_add_table(board_clk_lookups,
			ARRAY_SIZE(board_clk_lookups));

	if (arm_periph_clk != 0) {
		/* install the clock source for ARM PLL */
		static struct clk arm_pclk = {
			.name = "arm_pclk",
			.fixed = 1,
			.type = CLK_PLL,
			.ops = &arm_clk_ops,
		};
		static struct clk_lookup arm_clk_lookups[] = {
			/* ARM CPU clock */
			CLKDEV_INIT("cpu", "arm_pclk", &arm_pclk),
		};

		arm_pclk.rate = arm_periph_clk;
		clkdev_add_table(arm_clk_lookups,
				ARRAY_SIZE(arm_clk_lookups));
	} else {
		static struct clk dummy_arm_pclk;
		static struct clk_lookup arm_clk_lookups[] = {
			/* ARM CPU clock */
			CLKDEV_INIT("cpu", "arm_pclk", &dummy_arm_pclk),
		};

		clkdev_add_table(arm_clk_lookups,
				ARRAY_SIZE(arm_clk_lookups));
	}
}

#if 0
static int soc_abort_handler(unsigned long addr, unsigned int fsr,
		struct pt_regs *regs)
{
	/*
	 * These happen for no good reason
	 * possibly left over from CFE
	 */
	printk(KERN_WARNING "External imprecise Data abort at "
			"addr=%#lx, fsr=%#x ignored.\n", addr, fsr);

	/* Returning non-zero causes fault display and panic */
	return 0;
}
#endif

static void soc_aborts_enable(void)
{
#if 0
	u32 x;

	/* Install our hook */
	hook_fault_code(16 + 6, soc_abort_handler, SIGBUS, 0,
			"imprecise external abort");

	/* Enable external aborts - clear "A" bit in CPSR */

	/* Read CPSR */
	asm( "mrs	%0,cpsr": "=&r" (x) : : );

	x &= ~ PSR_A_BIT;

	/* Update CPSR, affect bits 8-15 */
	asm( "msr	cpsr_x,%0; nop; nop": : "r" (x) : "cc" );
#endif
}

/*
 * This SoC relies on MPCORE GIC interrupt controller
 */
void __init soc_init_irq(void)
{
	b15_init_gic();
	soc_aborts_enable();
}

#ifdef CONFIG_CACHE_L2X0
/* 63148 integrate l2 cache controller, no need to init */
void  __init soc_l2_cache_init(void)
{
	return;
}
#endif

/*
 * SoC initialization that need to be done early,
 * e.g. L2 cache, clock, I/O pin mux, power management
 */
void __init soc_init_early(void)
{
	b15_init_early();

	/*
	 * DMA memory
	 *
	 * The PCIe-to-AXI mapping (PAX) has a window of 128 MB alighed at 1MB
	 * we should make the DMA-able DRAM at least this large.
	 * Will need to use CONSISTENT_BASE and CONSISTENT_SIZE macros
	 * to program the PAX inbound mapping registers.
	 */
	// FIXME!!!
	//init_consistent_dma_size(SZ_128M);
}

/*
 * Initialize SoC timers
 */
void __init soc_init_timer(void)
{
	/* in BCM63148, we provide 2 ways to initialize timers.
	 * One is based on PERIPH Timer, and the other is using
	 * Cortex B15 MPcore own GTIMER */
#if defined(CONFIG_BCM_EXT_TIMER) && defined(CONFIG_PLAT_BCM63XX_EXT_TIMER)
	bcm63xx_timer_init();
#endif

	// FIXME!! the timer needs to be implemented!
#if 0
#ifdef CONFIG_PLAT_B15_MPCORE_TIMER
#define GTIMER_CLK_FREQ		FREQ_MHZ(25)
	b15_init_timer(GTIMER_CLK_FREQ);
#endif
#endif
}

/*
 * Install all other SoC device drivers
 * that are not automatically discoverable.
 */

void __init soc_add_devices(void)
{
	/* if there is soc specific device */

	/* to ensure RAC is disabled, due to some known issues with RAC */
	B15CTRL->cpu_ctrl.rac_cfg0 = 0;
}

/*
 * Functions to allow enabling/disabling WAIT instruction
 */
void set_cpu_arm_wait(int enable)
{
	arm_wfi_allowed = enable;
	printk("wait instruction: %s\n", enable ? "enabled" : "disabled");
	arm_wfi_enable(get_arm_core_clk());
	kick_all_cpus_sync();
}
EXPORT_SYMBOL(set_cpu_arm_wait);

int get_cpu_arm_wait(void)
{
	return arm_wfi_allowed;
}
EXPORT_SYMBOL(get_cpu_arm_wait);

static int __init bcm963xx_idle_init(void)
{
	arm_wfi_enable(get_arm_core_clk());
	return 0;
}
arch_initcall(bcm963xx_idle_init);
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
