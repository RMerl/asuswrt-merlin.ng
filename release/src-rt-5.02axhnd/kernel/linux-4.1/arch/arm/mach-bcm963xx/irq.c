#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
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
*/

/*
 * Interrupt control functions for Broadcom 963xx ARM boards
 */

#include <asm/atomic.h>

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>

#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <linux/bcm_assert.h>
#include <boardparms.h>
#include <board.h>

#ifdef CONFIG_SMP
    #define AFFINITY_OF(d) (*(d)->affinity)
#else
    #define AFFINITY_OF(d) ((void)(d), CPU_MASK_CPU0)
#endif

#define INTR_NAME_MAX_LENGTH 16

#ifdef CONFIG_SMP
extern DEFINE_PER_CPU(unsigned int, ipi_pending);
#endif

static DEFINE_SPINLOCK(brcm_irqlock);

void disable_brcm_irqsave(struct irq_data *data, unsigned long stateSaveArray[])
{
#if 0
	int cpu;
	unsigned long flags;
	unsigned int irq = data->irq;

	/* test for valid interrupt */
	if ((irq >= INTERNAL_ISR_TABLE_OFFSET) && (irq <= INTERRUPT_ID_LAST)) {
		/* Disable this processor's interrupts and acquire spinlock */
		spin_lock_irqsave(&brcm_irqlock, flags);

		/* loop thru each processor */
		for_each_cpu_mask(cpu, AFFINITY_OF(data)) {
			/* save original interrupt's enable state */
			stateSaveArray[cpu] = brcm_irq_ctrl[cpu]->IrqMask & (((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));

			/* clear each cpu's selected interrupt enable */
			brcm_irq_ctrl[cpu]->IrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));

		}

		/* release spinlock and enable this processor's interrupt */
		spin_unlock_irqrestore(&brcm_irqlock, flags);
	}
#endif
}


void restore_brcm_irqsave(struct irq_data *data, unsigned long stateSaveArray[])
{
#if 0
	int cpu;
	unsigned long flags;

	/* Disable this processor's interrupts and acquire spinlock */
	spin_lock_irqsave(&brcm_irqlock, flags);

	/* loop thru each processor */
	for_each_cpu_mask(cpu, AFFINITY_OF(data)) {
		/* restore cpu's original interrupt enable (off or on). */
		brcm_irq_ctrl[cpu]->IrqMask |= stateSaveArray[cpu];
	}

	/* release spinlock and enable this processor's interrupt */
	spin_unlock_irqrestore(&brcm_irqlock, flags);
#endif
}

void enable_brcm_irq_noop(unsigned int irq)
{
}

void enable_brcm_irq_irq(unsigned int irq)
{
	enable_irq(irq);
}

void disable_brcm_irq_irq(unsigned int irq)
{
	disable_irq(irq);
}

/* This is a wrapper to standand Linux request_irq, which automatically sets
 * IRQ flags and interurpt names. 
 * One major difference between IRQ HAL wrapper between ARM vs MIPS is that
 * we DO NOT support REARM_NO mode in ARM.  This means the IRQ is always
 * automatically re-enabled when the ISR is done. */
unsigned int BcmHalMapInterrupt(FN_HANDLER pfunc, void* param, unsigned int irq)
{
	char devname[INTR_NAME_MAX_LENGTH];

	sprintf(devname, "brcm_%d", irq);
	return BcmHalMapInterruptEx(pfunc, param, irq, devname, INTR_REARM_YES,
		INTR_AFFINITY_DEFAULT);
}

/** Broadcom wrapper to linux request_irq.  This version does more stuff.
 *
 * @param pfunc (IN) interrupt handler function
 * @param param (IN) context/cookie that is passed to interrupt handler
 * @param irq   (IN) interrupt number
 * @param interruptName (IN) descriptive name for the interrupt.  15 chars
 *                           or less.  This function will make a copy of
 *                           the name.
 * @param INTR_REARM_MODE    (IN) See bcm_intr.h, not used in ARM
 * @param INTR_AFFINITY_MODE (IN) See bcm_intr.h
 *
 * @return 0 on success.
 */
unsigned int BcmHalMapInterruptEx(FN_HANDLER pfunc, void* param,
		unsigned int irq, const char *interruptName,
		INTR_REARM_MODE_ENUM rearmMode,
		INTR_AFFINITY_MODE_ENUM affinMode)
{
	char *devname;
	unsigned long irqflags = 0x00;
	unsigned int retval;
	struct cpumask mask;
	unsigned long flags;

#if defined(CONFIG_BCM_KF_ASSERT)
	BCM_ASSERT_R(interruptName != NULL, -1);
	BCM_ASSERT_R(strlen(interruptName) < INTR_NAME_MAX_LENGTH, -1);
#endif

	if ((devname = kmalloc(INTR_NAME_MAX_LENGTH, GFP_ATOMIC)) == NULL) {
		printk(KERN_ERR "kmalloc(%d, GFP_ATOMIC) failed for intr name\n",
				INTR_NAME_MAX_LENGTH);
		return -1;
	}
	sprintf( devname, "%s", interruptName );

	if ((irq >= INTERRUPT_ID_TIMER) && (irq <= INTERRUPT_ID_TIMER_MAX))
		irqflags |= IRQF_TIMER;

#if !defined(CONFIG_BRCM_IKOS)
	/* For external interrupt, check if it is shared */
	if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_MAX) {
		if (IsExtIntrShared(kerSysGetExtIntInfo(irq)))
			irqflags |= IRQF_SHARED;
	}
#endif

	retval = request_irq(irq, (void*)pfunc, irqflags, devname,
			(void *)param);
	if (retval != 0) {
		printk(KERN_WARNING "request_irq failed for irq=%d (%s) "
				"retval=%d\n", irq, devname, retval);
		kfree(devname);
		return retval;
	}

#ifdef CONFIG_SMP
	/* for Timer interrupt, we always use CPU#0 to handle it */
	if ((irq >= INTERRUPT_ID_TIMER) && (irq <= INTERRUPT_ID_TIMER_MAX)) {
		cpumask_clear(&mask);
		cpumask_set_cpu(0, &mask);
		irq_set_affinity(irq, &mask);
	}
#endif

	/* now deal with interrupt affinity requests */
	if (affinMode != INTR_AFFINITY_DEFAULT) {
		cpumask_clear(&mask);

		if (affinMode == INTR_AFFINITY_TP1_ONLY ||
				affinMode == INTR_AFFINITY_TP1_IF_POSSIBLE) {
			if (cpu_online(1)) {
				cpumask_set_cpu(1, &mask);
				irq_set_affinity(irq, &mask);
			} else {
				/* TP1 is not on-line but caller insisted on it */
				if (affinMode == INTR_AFFINITY_TP1_ONLY) {
					printk(KERN_WARNING "cannot assign "
							"intr %d to TP1, not "
							"online\n", irq);
					retval = request_irq(irq, NULL, 0,
							NULL, NULL);
					kfree(devname);
					retval = -1;
				}
			}
		} else {
			/* INTR_AFFINITY_BOTH_IF_POSSIBLE */
			cpumask_set_cpu(0, &mask);
			if (cpu_online(1)) {
				cpumask_set_cpu(1, &mask);
				irq_set_affinity(irq, &mask);
			}
		}
	}

#if !defined(CONFIG_BRCM_IKOS)
	if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_MAX)
	{
		int levelOrEdge, detectSense, bothEdge;
		int ein = irq - INTERRUPT_ID_EXTERNAL_0;

		if( IsExtIntrTypeActHigh(kerSysGetExtIntInfo(irq)) )
			detectSense = 1;
		else
			detectSense = 0;

		if( IsExtIntrTypeSenseLevel(kerSysGetExtIntInfo(irq)) )
			levelOrEdge = 1;
		else
			levelOrEdge = 0;

		if( IsExtIntrTypeBothEdge(kerSysGetExtIntInfo(irq)) )
			bothEdge = 1;
		else
			bothEdge = 0;

		spin_lock_irqsave(&brcm_irqlock, flags);
		PERF->ExtIrqCtrl |= (levelOrEdge << (EI_LEVEL_SHFT + ein)) 
			| (detectSense << (EI_SENSE_SHFT + ein)) 
			| (bothEdge << (EI_INSENS_SHFT + ein))
			| (1 << (EI_CLEAR_SHFT + ein));
		PERF->ExtIrqStatus |= (1 << (EI_MASK_SHFT + ein));
		spin_unlock_irqrestore(&brcm_irqlock, flags);
	}
#endif

	return retval;
}
EXPORT_SYMBOL(BcmHalMapInterruptEx);


//***************************************************************************
//  void  BcmHalGenerateSoftInterrupt
//
//   Triggers a software interrupt.
//
//***************************************************************************
void BcmHalGenerateSoftInterrupt(unsigned int irq)
{
#if 0
	unsigned long flags;

	local_irq_save(flags);
	set_c0_cause(0x1 << (CAUSEB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));
	local_irq_restore(flags);
#endif
}

void BcmHalExternalIrqClear(unsigned int irq)
{
	// clear interrupt (write 1 then 0)
	unsigned long flags;
	spin_lock_irqsave(&brcm_irqlock, flags);
	PERF->ExtIrqCtrl |=  (1 << (EI_CLEAR_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
	PERF->ExtIrqCtrl &= ~(1 << (EI_CLEAR_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
	spin_unlock_irqrestore(&brcm_irqlock, flags); 
}

void BcmHalExternalIrqMask(unsigned int irq)
{
	unsigned long flags;
	spin_lock_irqsave(&brcm_irqlock, flags);
	PERF->ExtIrqStatus &= ~(1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
	spin_unlock_irqrestore(&brcm_irqlock, flags); 
}

void BcmHalExternalIrqUnmask(unsigned int irq)
{
	unsigned long flags;
	spin_lock_irqsave(&brcm_irqlock, flags);
	PERF->ExtIrqStatus |= (1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
	spin_unlock_irqrestore(&brcm_irqlock, flags); 
}


EXPORT_SYMBOL(enable_brcm_irq_noop);
EXPORT_SYMBOL(enable_brcm_irq_irq);
EXPORT_SYMBOL(disable_brcm_irq_irq);
EXPORT_SYMBOL(BcmHalMapInterrupt);
EXPORT_SYMBOL(BcmHalGenerateSoftInterrupt);
EXPORT_SYMBOL(BcmHalExternalIrqClear);
EXPORT_SYMBOL(BcmHalExternalIrqMask);
EXPORT_SYMBOL(BcmHalExternalIrqUnmask);

EXPORT_SYMBOL(disable_brcm_irqsave);
EXPORT_SYMBOL(restore_brcm_irqsave);

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
