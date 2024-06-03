#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <bcm_intr.h>
#include <board.h>

#define INTR_NAME_MAX_LENGTH 16

extern int timer_base_irq, timer_max_irq;

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

    BUG_ON(interruptName == NULL);
    BUG_ON(strlen(interruptName) >= INTR_NAME_MAX_LENGTH);

    if ((devname = kmalloc(INTR_NAME_MAX_LENGTH, GFP_ATOMIC)) == NULL) {
        printk(KERN_ERR "kmalloc(%d, GFP_ATOMIC) failed for intr name\n",
                INTR_NAME_MAX_LENGTH);
        return -1;
    }
    sprintf( devname, "%s", interruptName );

    if ((irq >= timer_base_irq) && (irq <= timer_max_irq))
        irqflags |= IRQF_TIMER;

    retval = request_irq(irq, (void*)pfunc, irqflags, devname,
            (void *)param);
    if (retval != 0) {
        printk(KERN_WARNING "request_irq failed for irq=%d (%s) "
                "retval=%d\n", irq, devname, retval);
        kfree(devname);
        return retval;
    }

#if defined(CONFIG_SMP)
    /* for Timer interrupt, we always use CPU#0 to handle it */
    if ((irq >= timer_base_irq) && (irq <= timer_max_irq)) {
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

    return retval;
}
EXPORT_SYMBOL(BcmHalMapInterruptEx);

/** Broadcom wrapper to linux request_threaded_irq.  This version does more stuff.
 * 
 *  @param pfunc (IN) interrupt handler function
 *  @param param (IN) context/cookie that is passed to interrupt handler
 *  @param irq   (IN) interrupt number
 *  @param interruptName (IN) descriptive name for the interrupt.  15 chars
 *                            or less.  This function will make a copy of
 *                            the name.
 *  @param INTR_REARM_MODE    (IN) See bcm_intr.h, not used in ARM
 *  @param INTR_AFFINITY_MODE (IN) See bcm_intr.h
 *  @param irq_handler_t      (IN) Function called from the irq handler thread If NULL, no irq thread is created
 *  
 *  @return 0 on success.
 *   */
unsigned int BcmHalMapInterruptExThreaded(FN_HANDLER pfunc, void* param,
        unsigned int irq, const char *interruptName,
        INTR_REARM_MODE_ENUM rearmMode,
        INTR_AFFINITY_MODE_ENUM affinMode,
        irq_handler_t thread_fn)
{
    char *devname;
    unsigned long irqflags = 0x00;
    unsigned int retval;
    struct cpumask mask;

    BUG_ON(interruptName == NULL);
    BUG_ON(strlen(interruptName) >= INTR_NAME_MAX_LENGTH);

    if ((devname = kmalloc(INTR_NAME_MAX_LENGTH, GFP_ATOMIC)) == NULL) {
        printk(KERN_ERR "kmalloc(%d, GFP_ATOMIC) failed for intr name\n",
                INTR_NAME_MAX_LENGTH);
        return -1;
    }
    sprintf( devname, "%s", interruptName );

    if ((irq >= timer_base_irq) && (irq <= timer_max_irq))
        irqflags |= IRQF_TIMER;

    retval = request_threaded_irq(irq, (void*)pfunc, thread_fn, irqflags, devname,
            (void *)param);
    if (retval != 0) {
        printk(KERN_WARNING "request_threaded_irq failed for irq=%d (%s) "
                "retval=%d\n", irq, devname, retval);
        kfree(devname);
        return retval;
    }

#if defined(CONFIG_SMP)
    /* for Timer interrupt, we always use CPU#0 to handle it */
    if ((irq >= timer_base_irq) && (irq <= timer_max_irq)) {
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

    return retval;
}
EXPORT_SYMBOL(BcmHalMapInterruptExThreaded);

unsigned int BcmHalMapInterrupt(FN_HANDLER pfunc, void* param, unsigned int irq)
{
    char devname[INTR_NAME_MAX_LENGTH];

    sprintf(devname, "brcm_%d", irq);
    return BcmHalMapInterruptEx(pfunc, param, irq, devname, INTR_REARM_YES,
        INTR_AFFINITY_DEFAULT);
}
EXPORT_SYMBOL(BcmHalMapInterrupt);

unsigned int BcmHalMapInterruptThreaded(FN_HANDLER pfunc, void* param, unsigned int irq, irq_handler_t thread_fn)
{
    char devname[INTR_NAME_MAX_LENGTH];

    sprintf(devname, "brcm_%d", irq);
    return BcmHalMapInterruptExThreaded(pfunc, param, irq, devname, INTR_REARM_YES,
        INTR_AFFINITY_DEFAULT, thread_fn);
}
EXPORT_SYMBOL(BcmHalMapInterruptThreaded);

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

void BcmHalSetIrqAffinity(unsigned int irq, struct cpumask *mask)
{
    irq_set_affinity(irq, mask);
}
EXPORT_SYMBOL(BcmHalSetIrqAffinity);


int BcmHalSetMsiIrqDescOff(unsigned int irq_base, unsigned int irq_offset, struct msi_desc *entry)
{
   return irq_set_msi_desc_off(irq_base, irq_offset, entry);
}
EXPORT_SYMBOL(BcmHalSetMsiIrqDescOff);


