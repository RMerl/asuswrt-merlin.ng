#if defined(CONFIG_BCM_KF_MIPS_BCM963XX)
/***********************************************************
 *
 * Copyright (c) 2009 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2009:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license 
 * agreement governing use of this software, this software is licensed 
 * to you under the terms of the GNU General Public License version 2 
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php, 
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give 
 *    you permission to link this software with independent modules, and 
 *    to copy and distribute the resulting executable under terms of your 
 *    choice, provided that you also meet, for each linked independent 
 *    module, the terms and conditions of the license of that module. 
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications 
 *    of the software.  
 * 
 * Not withstanding the above, under no circumstances may you combine 
 * this software in any way with any other Broadcom software provided 
 * under a license other than the GPL, without Broadcom's express prior 
 * written consent. 
 * 
 * :>
 *
 ************************************************************/
/***********************************************************
 *
 * This file implements clock events for the Broadcom DSL and GPON CPE
 * when the power management feature is enabled. When the processor
 * is found to be mostly idle, the main CPU clock is slowed down to
 * save power. By slowing down the clock, the C0 counter unfortunately
 * also slows down. This file replaces the (typical) 1 msec clock tick
 * interrupt processing with a reliable timer source which is unaffected
 * by the change in MIPS clock changes.
 *
 * The timer available to replace the C0 timer works differently.
 * The design needs to be adjusted accordingly. The C0 counter is a free
 * running counter which wraps at 0xFFFFFFFF and which runs at different
 * frequencies depending on the MIPS frequency. The C0 compare register
 * requires to be programmed to stay ahead of the C0 counter, to generate
 * an interrupt in the future.
 *
 * The peripheral timers (there are 3 of them) wrap at 0x3fffffff and
 * run at 50 MHz. When the timer reaches a programmed value, it can generate
 * and interrupt and then either stops counting or restarts at 0.
 * This difference in behavior between the C0 counter and the peripheral timers
 * required to use 2 timers for power management. One to generate the periodic
 * interrupts required by the clock events (Timer 0), and one to keep an accurate
 * reference when the clock is slowed down for saving power (Timer 2). Timer 1
 * is planned to be used by the second processor to support SMP.
 *
 ************************************************************/


#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/smp.h>

#include <asm/time.h>
#include <asm/cevt-r4k.h>

#include <bcm_map_part.h>
#include <bcm_intr.h>

extern void BcmPwrMngtCheckWaitCount(void);
extern unsigned int TimerC0Snapshot0;
#if defined(CONFIG_SMP)
extern unsigned int TimerC0Snapshot1;
extern unsigned int C0divider, C0multiplier, C0ratio;
#endif

DEFINE_PER_CPU(struct clock_event_device, bcm_mips_clockevent_device);
int bcm_timer_irq_installed;

static int bcm_mips_next_event0(unsigned long delta,
						struct clock_event_device *evt)
{
	// Timer may be reprogrammed while it is already running, so clear it first
	TIMER->TimerCtl0 = 0;
	TIMER->TimerCnt0 = 0;
	TIMER->TimerCtl0 = TIMERENABLE | RSTCNTCLR | delta;

	return 0;
}

#if defined(CONFIG_SMP)
static int bcm_mips_next_event1(unsigned long delta,
						struct clock_event_device *evt)
{
	// Timer may be reprogrammed while it is already running, so clear it first
	TIMER->TimerCtl1 = 0;
	TIMER->TimerCnt1 = 0;
	TIMER->TimerCtl1 = TIMERENABLE | RSTCNTCLR | delta;

	return 0;
}
#endif

void bcm_mips_set_clock_mode(enum clock_event_mode mode,
						struct clock_event_device *evt)
{
}

void bcm_mips_event_handler(struct clock_event_device *dev)
{
}

#if defined(CONFIG_SMP)
extern struct plat_smp_ops *mp_ops;
#endif
irqreturn_t bcm_timer_interrupt_handler_TP0(int irq, void *dev_id)
{
	struct clock_event_device *cd;
	irqreturn_t rc = IRQ_NONE;
	byte timer_ints = TIMER->TimerInts & (TIMER0|TIMER1);

	if (timer_ints & TIMER0) {
		TIMER->TimerCtl0 = 0;
	}
	if (timer_ints & TIMER1) {
		TIMER->TimerCtl1 = 0;
	}
	TIMER->TimerInts = timer_ints;

	if (timer_ints & TIMER0) {
		// Turn off timer
		TIMER->TimerCtl0 = 0;

		cd = &per_cpu(bcm_mips_clockevent_device, 0);
		cd->event_handler(cd);

		BcmPwrMngtCheckWaitCount();

		rc = IRQ_HANDLED;
	}
#if defined(CONFIG_SMP)
	if (timer_ints & TIMER1) {
		// Turn off timer
		TIMER->TimerCtl1 = 0;
		mp_ops->send_ipi_single(1, SMP_BCM_PWRSAVE_TIMER);

		rc = IRQ_HANDLED;
	}
#endif

	return rc;
}

struct irqaction perf_timer_irqaction = {
	.handler = bcm_timer_interrupt_handler_TP0,
	.flags = IRQF_SHARED,
	.name = "Periph Timer",
};

#if defined(CONFIG_SMP)
void bcm_timer_interrupt_handler_TP1(void)
{
	struct clock_event_device *cd;

	cd = &per_cpu(bcm_mips_clockevent_device, 1);
	cd->event_handler(cd);

	BcmPwrMngtCheckWaitCount();

	return;
}
#endif

int __cpuinit r4k_clockevent_init(void)
{
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *cd;

	cd = &per_cpu(bcm_mips_clockevent_device, cpu);

	cd->name		= "BCM Periph Timer";
	cd->features		= CLOCK_EVT_FEAT_ONESHOT;

	clockevent_set_clock(cd, mips_hpt_frequency);

	/* Calculate the min / max delta */
	cd->max_delta_ns	= clockevent_delta2ns(0x3fffffff, cd);
	cd->min_delta_ns	= clockevent_delta2ns(0x300, cd);

	cd->rating		= 300;
	cd->irq			= INTERRUPT_ID_TIMER;
	cd->cpumask		= cpumask_of(cpu);
	if (cpu == 0)
		cd->set_next_event	= bcm_mips_next_event0;
#if defined(CONFIG_SMP)
	else
		cd->set_next_event	= bcm_mips_next_event1;
#endif

	cd->set_mode		= bcm_mips_set_clock_mode;
	cd->event_handler	= bcm_mips_event_handler;

	clockevents_register_device(cd);

	if (cpu == 0) {
		// Start the BCM Timer interrupt
		irq_set_affinity(INTERRUPT_ID_TIMER, cpumask_of(0));
		setup_irq(INTERRUPT_ID_TIMER, &perf_timer_irqaction);

		// Start the BCM Timer0 - keep accurate 1 msec tick count
		TIMER->TimerCtl0 = TIMERENABLE | RSTCNTCLR | (50000-1);
		TIMER->TimerMask |= TIMER0EN;

		// Take a snapshot of the C0 timer when Timer2 was started
		// This will be needed later when having to make adjustments
		TimerC0Snapshot0 = read_c0_count();

		// Start the BCM Timer2
		// to keep an accurate free running high precision counter
		// Count up to its maximum value so it can be used by csrc-r4k-bcm-pwr.c
		TIMER->TimerCtl2 = TIMERENABLE | 0x3fffffff;
	}
#if defined(CONFIG_SMP)
	else {
		unsigned int newTimerCnt, mult, rem, result;
		// Start the BCM Timer1 - keep accurate 1 msec tick count
		TIMER->TimerCtl1 = TIMERENABLE | RSTCNTCLR | (50000-1);
		TIMER->TimerMask |= TIMER1EN;

		// Take a snapshot of the C0 timer when Timer1 was started
		// This will be needed later when having to make adjustments
		TimerC0Snapshot1 = read_c0_count();
		newTimerCnt = TIMER->TimerCnt2 & 0x3fffffff;
		mult = newTimerCnt/C0divider;
		rem  = newTimerCnt%C0divider;
		result  = mult*C0multiplier + ((rem*C0ratio)>>10);
		TimerC0Snapshot1 -= result;
	}
#endif

	return 0;
}
#endif
