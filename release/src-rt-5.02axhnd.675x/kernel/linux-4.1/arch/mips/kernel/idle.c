/*
 * MIPS idle loop and WAIT instruction support.
 *
 * Copyright (C) xxxx  the Anonymous
 * Copyright (C) 1994 - 2006 Ralf Baechle
 * Copyright (C) 2003, 2004  Maciej W. Rozycki
 * Copyright (C) 2001, 2004, 2011, 2012	 MIPS Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <linux/export.h>
#include <linux/init.h>
#include <linux/irqflags.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <asm/cpu.h>
#include <asm/cpu-info.h>
#include <asm/cpu-type.h>
#include <asm/idle.h>
#include <asm/mipsregs.h>

#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && defined(CONFIG_MIPS_BCM963XX)
/* Bcm version minimizes the chance of an irq sneaking in between checking
need_resched and wait instruction, or eliminates it completely (depending on 
pipeline design). This avoids delayed processing of softirq. (The delayed 
softirq problem can happen when preemption is disabled and softirq runs in 
process context.) */

extern void BcmPwrMngtReduceCpuSpeed(void);
extern void BcmPwrMngtResumeFullSpeed(void);

static void bcm_r4k_wait(void)
{
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE) || defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
	BcmPwrMngtReduceCpuSpeed();
#endif

	/* Always try to treat the segment below as an atomic entity and try not 
	to insert code or move code around */
	/* Begin fixed safe code pattern for the particular MIPS pipleline*/
	raw_local_irq_disable();
	if (!need_resched() &&  !(read_c0_cause() & read_c0_status())) {
		/* Perform SYNC, enable interrupts, then WAIT */
		__asm__ __volatile__ (
			".set push\n"
			".set noreorder\n"
			".set noat\n"
			"sync\n"
			"mfc0	$1, $12\n"
			"ori $1, $1, 0x1f\n"
			"xori	$1, $1, 0x1e\n"
			"mtc0	$1, $12\n"
			"nop\n"  // Recommended by MIPS team
			"wait\n"
			"nop\n"  // Needed to ensure next instruction is safe
			"nop\n"  // When speed is reduced to 1/8, need one more to get DG interrupt
			"nop\n"  // Safety net...
			".set pop\n");
	}
	else {
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE) || defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
		BcmPwrMngtResumeFullSpeed();
#endif
		raw_local_irq_enable();
	}
	/* End fixed code pattern */
}
#endif /* CONFIG_BCM_KF_MIPS_BCM963XX */


/*
 * Not all of the MIPS CPUs have the "wait" instruction available. Moreover,
 * the implementation of the "wait" feature differs between CPU families. This
 * points to the function that implements CPU specific wait.
 * The wait instruction stops the pipeline and reduces the power consumption of
 * the CPU very much.
 */
void (*cpu_wait)(void);
EXPORT_SYMBOL(cpu_wait);

static void r3081_wait(void)
{
	unsigned long cfg = read_c0_conf();
	write_c0_conf(cfg | R30XX_CONF_HALT);
	local_irq_enable();
}

static void r39xx_wait(void)
{
	if (!need_resched())
		write_c0_conf(read_c0_conf() | TX39_CONF_HALT);
	local_irq_enable();
}

void r4k_wait(void)
{
	local_irq_enable();
	__r4k_wait();
}

/*
 * This variant is preferable as it allows testing need_resched and going to
 * sleep depending on the outcome atomically.  Unfortunately the "It is
 * implementation-dependent whether the pipeline restarts when a non-enabled
 * interrupt is requested" restriction in the MIPS32/MIPS64 architecture makes
 * using this version a gamble.
 */
void r4k_wait_irqoff(void)
{
	if (!need_resched())
		__asm__(
		"	.set	push		\n"
		"	.set	arch=r4000	\n"
		"	wait			\n"
		"	.set	pop		\n");
	local_irq_enable();
}

/*
 * The RM7000 variant has to handle erratum 38.	 The workaround is to not
 * have any pending stores when the WAIT instruction is executed.
 */
static void rm7k_wait_irqoff(void)
{
	if (!need_resched())
		__asm__(
		"	.set	push					\n"
		"	.set	arch=r4000				\n"
		"	.set	noat					\n"
		"	mfc0	$1, $12					\n"
		"	sync						\n"
		"	mtc0	$1, $12		# stalls until W stage	\n"
		"	wait						\n"
		"	mtc0	$1, $12		# stalls until W stage	\n"
		"	.set	pop					\n");
	local_irq_enable();
}

/*
 * Au1 'wait' is only useful when the 32kHz counter is used as timer,
 * since coreclock (and the cp0 counter) stops upon executing it. Only an
 * interrupt can wake it, so they must be enabled before entering idle modes.
 */
static void au1k_wait(void)
{
	unsigned long c0status = read_c0_status() | 1;	/* irqs on */

	__asm__(
	"	.set	arch=r4000			\n"
	"	cache	0x14, 0(%0)		\n"
	"	cache	0x14, 32(%0)		\n"
	"	sync				\n"
	"	mtc0	%1, $12			\n" /* wr c0status */
	"	wait				\n"
	"	nop				\n"
	"	nop				\n"
	"	nop				\n"
	"	nop				\n"
	"	.set	mips0			\n"
	: : "r" (au1k_wait), "r" (c0status));
}

static int __initdata nowait;

static int __init wait_disable(char *s)
{
	nowait = 1;

	return 1;
}

__setup("nowait", wait_disable);

void __init check_wait(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

	if (nowait) {
		printk("Wait instruction disabled.\n");
		return;
	}

	switch (current_cpu_type()) {
	case CPU_R3081:
	case CPU_R3081E:
		cpu_wait = r3081_wait;
		break;
	case CPU_TX3927:
		cpu_wait = r39xx_wait;
		break;
	case CPU_R4200:
/*	case CPU_R4300: */
	case CPU_R4600:
	case CPU_R4640:
	case CPU_R4650:
	case CPU_R4700:
	case CPU_R5000:
	case CPU_R5500:
	case CPU_NEVADA:
	case CPU_4KC:
	case CPU_4KEC:
	case CPU_4KSC:
	case CPU_5KC:
	case CPU_25KF:
	case CPU_PR4450:
	case CPU_BMIPS3300:
#if !defined(CONFIG_BCM_KF_MIPS_BCM963XX)
	case CPU_BMIPS4350:
#endif
	case CPU_BMIPS4380:
	case CPU_BMIPS5000:
	case CPU_CAVIUM_OCTEON:
	case CPU_CAVIUM_OCTEON_PLUS:
	case CPU_CAVIUM_OCTEON2:
	case CPU_CAVIUM_OCTEON3:
	case CPU_JZRISC:
	case CPU_LOONGSON1:
	case CPU_XLR:
	case CPU_XLP:
		cpu_wait = r4k_wait;
		break;

#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && defined(CONFIG_MIPS_BCM963XX)
	case CPU_BMIPS4350:
		cpu_wait = bcm_r4k_wait;
		break;
#endif
	case CPU_RM7000:
		cpu_wait = rm7k_wait_irqoff;
		break;

	case CPU_PROAPTIV:
	case CPU_P5600:
		/*
		 * Incoming Fast Debug Channel (FDC) data during a wait
		 * instruction causes the wait never to resume, even if an
		 * interrupt is received. Avoid using wait at all if FDC data is
		 * likely to be received.
		 */
		if (IS_ENABLED(CONFIG_MIPS_EJTAG_FDC_TTY))
			break;
		/* fall through */
	case CPU_M14KC:
	case CPU_M14KEC:
	case CPU_24K:
	case CPU_34K:
	case CPU_1004K:
	case CPU_1074K:
	case CPU_INTERAPTIV:
	case CPU_M5150:
	case CPU_QEMU_GENERIC:
		cpu_wait = r4k_wait;
		if (read_c0_config7() & MIPS_CONF7_WII)
			cpu_wait = r4k_wait_irqoff;
		break;

	case CPU_74K:
		cpu_wait = r4k_wait;
		if ((c->processor_id & 0xff) >= PRID_REV_ENCODE_332(2, 1, 0))
			cpu_wait = r4k_wait_irqoff;
		break;

	case CPU_TX49XX:
		cpu_wait = r4k_wait_irqoff;
		break;
	case CPU_ALCHEMY:
		cpu_wait = au1k_wait;
		break;
	case CPU_20KC:
		/*
		 * WAIT on Rev1.0 has E1, E2, E3 and E16.
		 * WAIT on Rev2.0 and Rev3.0 has E16.
		 * Rev3.1 WAIT is nop, why bother
		 */
		if ((c->processor_id & 0xff) <= 0x64)
			break;

		/*
		 * Another rev is incremeting c0_count at a reduced clock
		 * rate while in WAIT mode.  So we basically have the choice
		 * between using the cp0 timer as clocksource or avoiding
		 * the WAIT instruction.  Until more details are known,
		 * disable the use of WAIT for 20Kc entirely.
		   cpu_wait = r4k_wait;
		 */
		break;
	default:
		break;
	}
}

#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && defined(CONFIG_MIPS_BCM963XX) && defined(CONFIG_BCM_KF_POWER_SAVE)
/* for power management */
static void set_cpu_r4k_wait(int enable)
{
	if(enable) {
		cpu_wait = bcm_r4k_wait;
		printk("wait instruction: enabled\n");
    }
	else {
		cpu_wait = NULL;
		printk("wait instruction: disabled\n");
    }
}

static int get_cpu_r4k_wait(void)
{
	if(cpu_wait == bcm_r4k_wait)
		return 1;
	else
		return 0;
}

#include <linux/module.h> // just for EXPORT_SYMBOL
EXPORT_SYMBOL(set_cpu_r4k_wait);
EXPORT_SYMBOL(get_cpu_r4k_wait);
#endif 

void arch_cpu_idle(void)
{
	if (cpu_wait)
		cpu_wait();
	else
		local_irq_enable();
}

#ifdef CONFIG_CPU_IDLE

int mips_cpuidle_wait_enter(struct cpuidle_device *dev,
			    struct cpuidle_driver *drv, int index)
{
	arch_cpu_idle();
	return index;
}

#endif
