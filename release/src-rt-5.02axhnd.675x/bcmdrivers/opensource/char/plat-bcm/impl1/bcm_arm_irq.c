#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>

#define INTR_NAME_MAX_LENGTH 16

#if !defined(CONFIG_BCM947189)
static DEFINE_SPINLOCK(brcm_irqlock);
#endif

/** Broadcom wrapper to linux request_irq.  This version does more stuff.
 * 
 *  @param pfunc (IN) interrupt handler function
 *  @param param (IN) context/cookie that is passed to interrupt handler
 *  @param irq   (IN) interrupt number
 *  @param interruptName (IN) descriptive name for the interrupt.  15 chars
 *                            or less.  This function will make a copy of
 *                            the name.
 *  @param INTR_REARM_MODE    (IN) See bcm_intr.h, not used in ARM
 *  @param INTR_AFFINITY_MODE (IN) See bcm_intr.h
 *  
 *  @return 0 on success.
 *   */
unsigned int BcmHalMapInterruptEx(FN_HANDLER pfunc, void* param,
        unsigned int irq, const char *interruptName,
        INTR_REARM_MODE_ENUM rearmMode,
        INTR_AFFINITY_MODE_ENUM affinMode)
{
    char *devname;
    unsigned long irqflags = 0x00;
    unsigned int retval;
    struct cpumask mask;
#if !defined(CONFIG_BCM947189)
    unsigned long flags;
#endif

    BUG_ON(interruptName == NULL);
    BUG_ON(strlen(interruptName) >= INTR_NAME_MAX_LENGTH);

    if ((devname = kmalloc(INTR_NAME_MAX_LENGTH, GFP_ATOMIC)) == NULL) {
        printk(KERN_ERR "kmalloc(%d, GFP_ATOMIC) failed for intr name\n",
                INTR_NAME_MAX_LENGTH);
        return -1;
    }
    sprintf( devname, "%s", interruptName );

#if !defined(CONFIG_BCM947189)
    if ((irq >= INTERRUPT_ID_TIMER0) && (irq <= INTERRUPT_ID_TIMER_MAX))
        irqflags |= IRQF_TIMER;
#endif

    if(kerSysIsIkosBootSet() == 0) {
        /* For external interrupt, check if it is shared */
        if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_MAX) {
            if (IsExtIntrShared(kerSysGetExtIntInfo(irq)))
                irqflags |= IRQF_SHARED;
        }
    }

    retval = request_irq(irq, (void*)pfunc, irqflags, devname,
            (void *)param);
    if (retval != 0) {
        printk(KERN_WARNING "request_irq failed for irq=%d (%s) "
                "retval=%d\n", irq, devname, retval);
        kfree(devname);
        return retval;
    }

#if defined(CONFIG_SMP) && !defined(CONFIG_BCM947189)
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
#if defined CONFIG_HOTPLUG_CPU
                // static affinity_hint mask with only cpu1
                const cpumask_t *onemask = get_cpu_mask(1);
                // sets both affinity_hint and affinity
                irq_set_affinity_hint(irq, onemask);
#else
                cpumask_set_cpu(1, &mask);
                irq_set_affinity(irq, &mask);
#endif
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

#if !defined(CONFIG_BCM947189)
    if(kerSysIsIkosBootSet() == 0) {
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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858) || (CONFIG_BRCM_CHIP_REV==0x63158A0)
                | (1 << (EI_CLEAR_SHFT + ein))
#endif
                | (bothEdge << (EI_INSENS_SHFT + ein));
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM96878)
            PERF->ExtIrqMaskSet = (1 << ein);
#else
            PERF->ExtIrqStatus |= (1 << (EI_MASK_SHFT + ein));
#endif
            spin_unlock_irqrestore(&brcm_irqlock, flags);
        }
    }
#endif

    return retval;
}
EXPORT_SYMBOL(BcmHalMapInterruptEx);

unsigned int BcmHalMapInterrupt(FN_HANDLER pfunc, void* param, unsigned int irq)
{
    char devname[INTR_NAME_MAX_LENGTH];

    sprintf(devname, "brcm_%d", irq);
    return BcmHalMapInterruptEx(pfunc, param, irq, devname, INTR_REARM_YES,
        INTR_AFFINITY_DEFAULT);
}
EXPORT_SYMBOL(BcmHalMapInterrupt);

void enable_brcm_irq_irq(unsigned int irq)
{
    enable_irq(irq);
}

void disable_brcm_irq_irq(unsigned int irq)
{
    disable_irq(irq);
}
EXPORT_SYMBOL(enable_brcm_irq_irq);
EXPORT_SYMBOL(disable_brcm_irq_irq);

#ifdef CONFIG_HOTPLUG_CPU
void bcm63xx_rehint_one_irq(struct irq_desc *desc, int cpu)
{
	struct irq_data *d = irq_desc_get_irq_data(desc);
	struct cpumask *affinity_dst;
	const struct cpumask *affinity = affinity_dst = 
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
		d->affinity;
#else
		d->common->affinity;
#endif
	struct irq_chip *c;

	/*
	 * If this is a per-CPU interrupt then we have nothing to do.
	 */
	if (irqd_is_per_cpu(d))
		return;

	/*
	 * If there is no affinity_hint then we have nothing to do.
	 */
	if (!desc->affinity_hint)
		return;

	/*
	 * If affinity matches affinity_hint then we have nothing to do.
	 */
	if (cpumask_equal(affinity, desc->affinity_hint))
		return;

	affinity = desc->affinity_hint;

	// ignore irq_set_affinity failures due to affinity_hint
	// not intersecting cpu_online_mask
	c = irq_data_get_irq_chip(d);
	if (!c->irq_set_affinity)
		pr_warn("IRQ%u: unable to set affinity\n", d->irq);
	else if (c->irq_set_affinity(d, affinity, false) == IRQ_SET_MASK_OK) {
		pr_info("IRQ%d: affinity change from %32pbl to %32pbl\n",
				d->irq, affinity_dst, affinity);
		cpumask_copy(affinity_dst, affinity);
	}
}
EXPORT_SYMBOL(bcm63xx_rehint_one_irq);

/*
 * A new CPU has come online.  Migrate IRQs that have an affinity_hint
 * that doesn't match their active affinity
 */
int bcm63xx_rehint_irqaffinity(struct notifier_block *nb, unsigned long action, void *hcpu)
{
	int cpu = (uintptr_t) hcpu;
	struct irq_desc *desc;
	unsigned long flags;
	unsigned int i;

	if (action != CPU_ONLINE)
		return NOTIFY_OK;

	local_irq_save(flags);

	for_each_irq_desc(i, desc) {
		raw_spin_lock(&desc->lock);
		bcm63xx_rehint_one_irq(desc, cpu);
		raw_spin_unlock(&desc->lock);
	}

	local_irq_restore(flags);

	return NOTIFY_OK;
}
EXPORT_SYMBOL(bcm63xx_rehint_irqaffinity);
#endif // #ifdef CONFIG_HOTPLUG_CPU

int bcm_irq_set_affinity_hint(int irq, const struct cpumask *mask)
{
    return irq_set_affinity_hint(irq, mask);
}
EXPORT_SYMBOL(bcm_irq_set_affinity_hint);

int bcm_set_affinity(int irq, int cpu_id, int force)
{
    int ret = 0;
#if defined(CONFIG_SMP)
    const struct cpumask *mask;

    if (cpu_id < 0 || cpu_id >= NR_CPUS)
        return -1;

    mask = get_cpu_mask(cpu_id);
    ret = bcm_irq_set_affinity_hint(irq, mask);

    /* force parameter allow the set irq affinity on the cpu that has not power up yet */
    if (!ret && force && !cpu_online(cpu_id))
        ret = irq_force_affinity(irq, mask);
#endif

    return ret;
}
EXPORT_SYMBOL(bcm_set_affinity);

#if !defined(CONFIG_BCM947189)
void BcmHalExternalIrqClear(unsigned int irq)
{
    unsigned long flags;
    spin_lock_irqsave(&brcm_irqlock, flags);
#if defined(CONFIG_BCM96858) || (CONFIG_BRCM_CHIP_REV==0x63158A0)
    do {
        PERF->ExtIrqCtrl |= (0xff << EI_CLEAR_SHFT);
        PERF->ExtIrqClear = (1 << (irq - INTERRUPT_ID_EXTERNAL_0));
    } while ((PERF->ExtIrqStatus & (1 << (irq - INTERRUPT_ID_EXTERNAL_0))) && EI_STATUS_MASK);
#else
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963138)
    PERF->ExtIrqCtrl |=  (1 << (EI_CLEAR_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
    PERF->ExtIrqCtrl &= ~(1 << (EI_CLEAR_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
#else
    PERF->ExtIrqClear = (1 << (irq - INTERRUPT_ID_EXTERNAL_0));
    PERF->ExtIrqClear = ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0));
#endif
#endif
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}

void BcmHalExternalIrqMask(unsigned int irq)
{
    unsigned long flags;
    spin_lock_irqsave(&brcm_irqlock, flags);
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM96878)
    PERF->ExtIrqMaskClear = (1 << (irq - INTERRUPT_ID_EXTERNAL_0));
#else
    PERF->ExtIrqStatus &= ~(1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
#endif
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}

void BcmHalExternalIrqUnmask(unsigned int irq)
{
    unsigned long flags;
    spin_lock_irqsave(&brcm_irqlock, flags);
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM96878)
    PERF->ExtIrqMaskSet = (1 << (irq - INTERRUPT_ID_EXTERNAL_0));
#else
    PERF->ExtIrqStatus |= (1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
#endif
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}

void BcmHalSetIrqAffinity(unsigned int irq, struct cpumask *mask)
{
    irq_set_affinity(irq, mask);
}
EXPORT_SYMBOL(BcmHalSetIrqAffinity);
EXPORT_SYMBOL(BcmHalExternalIrqClear);
EXPORT_SYMBOL(BcmHalExternalIrqMask);
EXPORT_SYMBOL(BcmHalExternalIrqUnmask);
#endif

