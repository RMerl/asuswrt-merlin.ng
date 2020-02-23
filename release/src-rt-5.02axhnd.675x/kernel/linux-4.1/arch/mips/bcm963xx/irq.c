#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && defined(CONFIG_MIPS_BCM963XX)
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
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
 * Interrupt control functions for Broadcom 963xx MIPS boards
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
#include <linux/linkage.h>

#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/signal.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <boardparms.h>
#include <board.h>
#if defined(CONFIG_BCM_EXT_TIMER)
#include <bcm_ext_timer.h>
#endif

#if defined(CONFIG_SMP)
    #define AFFINITY_OF(d) ((d)->affinity)
    #define AFFINITY_OF_REF(d) (*(d)->affinity)
#else
    #define AFFINITY_OF(d) ((void)(d), CPU_MASK_CPU0)
    #define AFFINITY_OF_REF(d) ((void)(d), CPU_MASK_CPU0)
#endif

#if IRQ_BITS == 64
    #define IRQ_TYPE uint64
#else
    #define IRQ_TYPE uint32
#endif


volatile IrqControl_t * brcm_irq_ctrl[NR_CPUS];
spinlock_t brcm_irqlock;

#if defined(CONFIG_SMP)
extern DEFINE_PER_CPU(unsigned int, ipi_pending);
#endif

static void irq_dispatch_int(void)
{
    int cpu = smp_processor_id();
    IRQ_TYPE pendingIrqs;
    static IRQ_TYPE irqBit[NR_CPUS];

    static uint32 isrNumber[NR_CPUS] = {[0 ... NR_CPUS-1] = (sizeof(IRQ_TYPE) * 8) - 1};

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
    IRQ_TYPE pendingExtIrqs;
    static IRQ_TYPE extIrqBit[NR_CPUS];
    static uint32 extIsrNumber[NR_CPUS] = {[0 ... NR_CPUS-1] = (sizeof(IRQ_TYPE) * 8) - 1};
#endif

    spin_lock(&brcm_irqlock);
#if defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
    pendingIrqs = PERF->IrqStatus & brcm_irq_ctrl[cpu]->IrqMask;
    pendingExtIrqs = PERF->ExtIrqStatus & brcm_irq_ctrl[cpu]->ExtIrqMask;
#else
    pendingIrqs = brcm_irq_ctrl[cpu]->IrqStatus & brcm_irq_ctrl[cpu]->IrqMask;
#if defined(CONFIG_BCM963268)
    pendingExtIrqs = brcm_irq_ctrl[cpu]->ExtIrqStatus & brcm_irq_ctrl[cpu]->ExtIrqMask;
#endif
#if defined(CONFIG_BCM96838)
    pendingExtIrqs = PERFEXT->IrqControl[cpu].IrqStatus & PERFEXT->IrqControl[cpu].IrqMask;
#endif
#endif
    spin_unlock(&brcm_irqlock);

    if (pendingIrqs) 
    {
        while (1) {
            irqBit[cpu] <<= 1;
            isrNumber[cpu]++;
            if (isrNumber[cpu] == (sizeof(IRQ_TYPE) * 8)) {
                isrNumber[cpu] = 0;
                irqBit[cpu] = 0x1;
            }
            if (pendingIrqs & irqBit[cpu]) {
                unsigned int irq = isrNumber[cpu] + INTERNAL_ISR_TABLE_OFFSET;
#if defined(CONFIG_BCM96838)
                if (irq == INTERRUPT_ID_EXTERNAL) 
                {
                    int i;
                    unsigned int reg = PERF->ExtIrqCfg;
                    unsigned int status = (reg & EI_STATUS_MASK) >> EI_STATUS_SHFT;
                    unsigned int mask = (reg & EI_MASK_MASK) >> EI_MASK_SHFT;
                    status &=mask;

                    for(i = 0; i < 6; i++)
                    {
                        if (status & (1 << i))
                        {
                            irq = INTERRUPT_ID_EXTERNAL_0 + i;
                            break;
                        }
                    }
                    spin_lock(&brcm_irqlock);
                    PERF->ExtIrqCfg |= (1 << (i + EI_CLEAR_SHFT));      // Clear
                    spin_unlock(&brcm_irqlock);
                }
#elif !defined(CONFIG_BCM963381)
#if defined(CONFIG_BCM96848)
                if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_7)
#else
                if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3)
#endif
                {   spin_lock(&brcm_irqlock);
                    PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));      // Clear
                    spin_unlock(&brcm_irqlock);
                }
#endif
                do_IRQ(irq);
                break;
            }
        }
    }

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
    if (pendingExtIrqs) 
    {
        while (1) {
            extIrqBit[cpu] <<= 1;
            extIsrNumber[cpu]++;
            if (extIsrNumber[cpu] == (sizeof(IRQ_TYPE) * 8)) {
                extIsrNumber[cpu] = 0;
                extIrqBit[cpu] = 0x1;
            }
            if (pendingExtIrqs & extIrqBit[cpu]) {
                unsigned int extIrq = extIsrNumber[cpu] + INTERNAL_EXT_ISR_TABLE_OFFSET;
#if defined(CONFIG_BCM963381)
                if (extIrq >= INTERRUPT_ID_EXTERNAL_0 && extIrq <= INTERRUPT_ID_EXTERNAL_7) {
                    spin_lock(&brcm_irqlock);
                    PERF->ExtIrqCfg |= (1 << (extIrq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));      // Clear
                    spin_unlock(&brcm_irqlock);
                }
#endif
                do_IRQ(extIrq);
                break;
            }
        }
    }
#endif
}

#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE
extern void BcmPwrMngtResumeFullSpeed (void);
#endif


asmlinkage void plat_irq_dispatch(void)
{
    u32 cause;

#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE
    BcmPwrMngtResumeFullSpeed();
#endif

    while((cause = (read_c0_cause() & read_c0_status() & CAUSEF_IP))) {
        if (cause & CAUSEF_IP7)
            do_IRQ(MIPS_TIMER_INT);
        else if (cause & CAUSEF_IP0)
            do_IRQ(INTERRUPT_ID_SOFTWARE_0);
        else if (cause & CAUSEF_IP1)
            do_IRQ(INTERRUPT_ID_SOFTWARE_1);
#if defined (CONFIG_SMP)
#if defined(CONFIG_BCM96838)
        else if (cause & (CAUSEF_IP3 | CAUSEF_IP4))
#else
        else if (cause & (CAUSEF_IP2 | CAUSEF_IP3))
#endif
#else 
#if defined(CONFIG_BCM96838)
        else if (cause & CAUSEF_IP3)
#else
        else if (cause & CAUSEF_IP2)
#endif
#endif
            irq_dispatch_int();
    }
}

#if !defined(CONFIG_BCM96838)
// bill
void disable_brcm_irqsave(struct irq_data *data, unsigned long stateSaveArray[])
{
    int cpu;
    unsigned long flags;
    unsigned int irq = data->irq;

    // Test for valid interrupt.
    if ((irq >= INTERNAL_ISR_TABLE_OFFSET ) && (irq <= INTERRUPT_ID_LAST))
    {
        // Disable this processor's interrupts and acquire spinlock.
        spin_lock_irqsave(&brcm_irqlock, flags);

        // Loop thru each processor.
        for_each_cpu(cpu, AFFINITY_OF(data))
        {
            // Save original interrupt's enable state.
            stateSaveArray[cpu] = brcm_irq_ctrl[cpu]->IrqMask & (((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));

            // Clear each cpu's selected interrupt enable.
            brcm_irq_ctrl[cpu]->IrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));

#if defined(CONFIG_BCM963268) || defined (CONFIG_BCM963381) || defined(CONFIG_BCM96848)
            // Save original interrupt's enable state.
            stateSaveArray[cpu] = brcm_irq_ctrl[cpu]->ExtIrqMask & (((IRQ_TYPE)1) << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));

            // Clear each cpu's selected interrupt enable.
            brcm_irq_ctrl[cpu]->ExtIrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#endif
        }

        // Release spinlock and enable this processor's interrupts.
        spin_unlock_irqrestore(&brcm_irqlock, flags);
    }
}


// bill
void restore_brcm_irqsave(struct irq_data *data, unsigned long stateSaveArray[])
{
    int cpu;
    unsigned long flags;

    // Disable this processor's interrupts and acquire spinlock.
    spin_lock_irqsave(&brcm_irqlock, flags);

    // Loop thru each processor.
    for_each_cpu(cpu, AFFINITY_OF(data))
    {
        // Restore cpu's original interrupt enable (off or on).
        brcm_irq_ctrl[cpu]->IrqMask |= stateSaveArray[cpu];
#if defined(CONFIG_BCM963268) || defined (CONFIG_BCM963381) || defined(CONFIG_BCM96848)
        brcm_irq_ctrl[cpu]->ExtIrqMask |= stateSaveArray[cpu];
#endif
    }

    // Release spinlock and enable this processor's interrupts.
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}
#endif //#if !defined(CONFIG_BCM96838)


static __always_inline void enable_brcm_irq_data_locked(unsigned long irq, cpumask_t affinity)
{
    int cpu;
    unsigned long flags;
    int levelOrEdge = 1;
    int detectSense = 0;
    int bothEdge = 0;

    spin_lock_irqsave(&brcm_irqlock, flags);

    if(( irq >= INTERNAL_ISR_TABLE_OFFSET ) 
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM96838) || defined (CONFIG_BCM963381) || defined(CONFIG_BCM96848)
        && ( irq < (INTERNAL_ISR_TABLE_OFFSET+64) ) 
#endif
        ) 
    {

        for_each_cpu(cpu, &affinity) {
            brcm_irq_ctrl[cpu]->IrqMask |= (((IRQ_TYPE)1)  << (irq - INTERNAL_ISR_TABLE_OFFSET));
        }
    }
#if defined(CONFIG_BCM96838)
    else if((irq >= INTERRUPT_ID_EXTERNAL_0) && (irq <= INTERRUPT_ID_EXTERNAL_5))
    {
        for_each_cpu(cpu, &affinity) {
            brcm_irq_ctrl[cpu]->IrqMask |= (((IRQ_TYPE)1)  << (INTERRUPT_ID_EXTERNAL - INTERNAL_ISR_TABLE_OFFSET));
        }
    }
#endif
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM96838) || defined (CONFIG_BCM963381) || defined(CONFIG_BCM96848)
    else if(( irq >= INTERNAL_EXT_ISR_TABLE_OFFSET ) &&
            ( irq < (INTERNAL_EXT_ISR_TABLE_OFFSET+64) ) ) 
    {
        for_each_cpu(cpu, &affinity) {
#if defined(CONFIG_BCM963268) || defined (CONFIG_BCM963381) || defined(CONFIG_BCM96848)
            brcm_irq_ctrl[cpu]->ExtIrqMask |= (((IRQ_TYPE)1)  << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#else
            PERFEXT->IrqControl[cpu].IrqMask |= (((IRQ_TYPE)1)  << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#endif
        }
    }
#endif
    else if ((irq == INTERRUPT_ID_SOFTWARE_0) || (irq == INTERRUPT_ID_SOFTWARE_1)) {
        set_c0_status(0x1 << (STATUSB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));
    }

    /* Determine the type of IRQ trigger required */
    if ( (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3) 
#if defined(CONFIG_BCM96838)    
     || (irq >= INTERRUPT_ID_EXTERNAL_4 && irq <= INTERRUPT_ID_EXTERNAL_5)
#elif defined (CONFIG_BCM963381) || defined(CONFIG_BCM96848)
     || (irq >= INTERRUPT_ID_EXTERNAL_4 && irq <= INTERRUPT_ID_EXTERNAL_7)
#endif
       )
    {
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
    }
#if defined(CONFIG_BCM96838)
    if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_5) {
#elif defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
    if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_7) {
#else
    if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3) {
#endif
        PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_INSENS_SHFT));    // Edge insesnsitive
        if ( levelOrEdge ) {
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_LEVEL_SHFT));      // Level triggered
        } else {
            PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_LEVEL_SHFT));     // Edge triggered        
        }
        if ( detectSense ) {
            PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_SENSE_SHFT));      // High / Rising triggered
        } else {
            PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_SENSE_SHFT));     // Low / Falling triggered        
        }
        if ( bothEdge ) {
            PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_INSENS_SHFT));      // Both edge triggered 
        } else {
            PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_INSENS_SHFT));     // One edge triggered
        }
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));      // Clear
#if defined (CONFIG_BCM963381) || defined(CONFIG_BCM96848)
        PERF->ExtIrqSts |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_MASK_SHFT));       // Unmask
#else
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_MASK_SHFT));       // Unmask
#endif
    }

    spin_unlock_irqrestore(&brcm_irqlock, flags);
}

__always_inline void enable_brcm_irq_data(struct irq_data *data)
{
   enable_brcm_irq_data_locked(data->irq, AFFINITY_OF_REF(data));
}

void enable_brcm_irq_irq(unsigned int irq)
{
#if defined(CONFIG_SMP)
    // Note: for performance, no bounds checks are done on the below two lines
    struct irq_desc *desc = irq_desc + irq;
    cpumask_var_t affinity;

    cpumask_copy(affinity, desc->irq_data.affinity);

    // sanity check
    if (affinity->bits[0] == 0)
    {
        //WARN_ONCE(1, "irq %d has no affinity!!!\n", irq);
        cpumask_copy(affinity, &CPU_MASK_CPU0);
    }

    enable_brcm_irq_data_locked(irq, *affinity);
#else
    enable_brcm_irq_data_locked(irq, CPU_MASK_CPU0);
#endif
}

static __always_inline void __disable_ack_brcm_irq(unsigned int irq, cpumask_t affinity)
{
    int cpu;

#if defined(CONFIG_BCM96838)
    if((irq >= INTERRUPT_ID_EXTERNAL_0) && (irq <= INTERRUPT_ID_EXTERNAL_5))
        irq = INTERRUPT_ID_EXTERNAL;
#endif

    if(( irq >= INTERNAL_ISR_TABLE_OFFSET )  
        && ( irq < (INTERNAL_ISR_TABLE_OFFSET+64) ))
    {
        for_each_cpu(cpu, &affinity) {
            brcm_irq_ctrl[cpu]->IrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));
        }
    }
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
    else if(( irq >= INTERNAL_EXT_ISR_TABLE_OFFSET ) &&
            ( irq < (INTERNAL_EXT_ISR_TABLE_OFFSET+64) )) 
    {
        for_each_cpu(cpu, &affinity) {
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
            brcm_irq_ctrl[cpu]->ExtIrqMask &= ~(((IRQ_TYPE)1)  << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#else
            PERFEXT->IrqControl[cpu].IrqMask &= ~(((IRQ_TYPE)1)  << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#endif
        }
    }
#endif
}

static __always_inline void disable_brcm_irq_data_locked(unsigned long irq, cpumask_t affinity)
{
    unsigned long flags;

    spin_lock_irqsave(&brcm_irqlock, flags);
    __disable_ack_brcm_irq(irq, affinity);
    if ((irq == INTERRUPT_ID_SOFTWARE_0) || (irq == INTERRUPT_ID_SOFTWARE_1)) {
        clear_c0_status(0x1 << (STATUSB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));
    }
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}

void disable_brcm_irq_irq(unsigned int irq)
{
#if defined(CONFIG_SMP)
    struct irq_desc *desc = irq_desc + irq;
    cpumask_var_t affinity;

    cpumask_copy(affinity, desc->irq_data.affinity);

    if (unlikely(affinity->bits[0] == 0))
    {
        //WARN_ONCE(1, "irq %d has no affinity!!!\n", irq);
        cpumask_copy(affinity, &CPU_MASK_CPU0);
    }

    disable_brcm_irq_data_locked(irq, *affinity);
#else
    disable_brcm_irq_data_locked(irq, CPU_MASK_CPU0);
#endif
}

void disable_brcm_irq_data(struct irq_data *data)
{
    disable_brcm_irq_data_locked(data->irq, AFFINITY_OF_REF(data));
}

void ack_brcm_irq(struct irq_data *data)
{
    unsigned long flags;
    unsigned int irq = data->irq;

    spin_lock_irqsave(&brcm_irqlock, flags);
    __disable_ack_brcm_irq(irq, AFFINITY_OF_REF(data));

#if defined(CONFIG_SMP)
    if (irq == INTERRUPT_ID_SOFTWARE_0) {
        int this_cpu = smp_processor_id();
        int other_cpu = !this_cpu;
        per_cpu(ipi_pending, this_cpu) = 0;
        mb();
        clear_c0_cause(1<<CAUSEB_IP0);
        if (per_cpu(ipi_pending, other_cpu)) {
            set_c0_cause(1<<CAUSEB_IP0);
        }
    }
#else
    if (irq == INTERRUPT_ID_SOFTWARE_0) {
        clear_c0_cause(1<<CAUSEB_IP0);
    }
#endif

    if (irq == INTERRUPT_ID_SOFTWARE_1) {
        clear_c0_cause(1<<CAUSEB_IP1);
    }

    spin_unlock_irqrestore(&brcm_irqlock, flags);
}


void mask_ack_brcm_irq(struct irq_data *data)
{
    unsigned long flags;
    unsigned int irq = data->irq;

    spin_lock_irqsave(&brcm_irqlock, flags);
    __disable_ack_brcm_irq(irq, AFFINITY_OF_REF(data));

#if defined(CONFIG_SMP)
    if (irq == INTERRUPT_ID_SOFTWARE_0) {
        int this_cpu = smp_processor_id();
        int other_cpu = !this_cpu;
        per_cpu(ipi_pending, this_cpu) = 0;
        mb();
        clear_c0_cause(1<<CAUSEB_IP0);
        if (per_cpu(ipi_pending, other_cpu)) {
            set_c0_cause(1<<CAUSEB_IP0);
        }
        clear_c0_status(1<<STATUSB_IP0);
    }
#else
    if (irq == INTERRUPT_ID_SOFTWARE_0) {
        clear_c0_status(1<<STATUSB_IP0);
        clear_c0_cause(1<<CAUSEB_IP0);
    }
#endif

    if (irq == INTERRUPT_ID_SOFTWARE_1) {
        clear_c0_status(1<<STATUSB_IP1);
        clear_c0_cause(1<<CAUSEB_IP1);
    }

    spin_unlock_irqrestore(&brcm_irqlock, flags);
}


void unmask_brcm_irq_noop(struct irq_data *data)
{
}

int set_brcm_affinity(struct irq_data *data, const struct cpumask *dest, bool force)
{
    int cpu;
    unsigned int irq = data->irq;
    unsigned long flags;
    int ret = 0;

    spin_lock_irqsave(&brcm_irqlock, flags);

#if defined(CONFIG_BCM96838)
    if((irq >= INTERRUPT_ID_EXTERNAL_0) && (irq <= INTERRUPT_ID_EXTERNAL_5))
        irq = INTERRUPT_ID_EXTERNAL;
#endif

    if(( irq >= INTERNAL_ISR_TABLE_OFFSET ) 
        && ( irq < (INTERNAL_ISR_TABLE_OFFSET+64) ) 
        ) 
    {
        for_each_online_cpu(cpu) {
            if (cpumask_test_cpu(cpu, dest) && !(irqd_irq_disabled(data))) {
                brcm_irq_ctrl[cpu]->IrqMask |= (((IRQ_TYPE)1)  << (irq - INTERNAL_ISR_TABLE_OFFSET));
            }
            else {
                brcm_irq_ctrl[cpu]->IrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));
            }
        }
    }

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848) 
    if(( irq >= INTERNAL_EXT_ISR_TABLE_OFFSET ) 
        && ( irq < (INTERNAL_EXT_ISR_TABLE_OFFSET+64) ) 
        )
    {
        for_each_online_cpu(cpu) {
            if (cpumask_test_cpu(cpu, dest) && !(irqd_irq_disabled(data))) {

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
                brcm_irq_ctrl[cpu]->ExtIrqMask |= (((IRQ_TYPE)1)  << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#else
                PERFEXT->IrqControl[cpu].IrqMask |= (((IRQ_TYPE)1)  << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#endif
            }
            else {
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
                brcm_irq_ctrl[cpu]->ExtIrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#else
                PERFEXT->IrqControl[cpu].IrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_EXT_ISR_TABLE_OFFSET));
#endif
            }
        }
    }
#endif

    spin_unlock_irqrestore(&brcm_irqlock, flags);

    return ret;
}


static struct irq_chip brcm_irq_chip = {
    .name = "BCM63xx",
    .irq_enable = enable_brcm_irq_data,
    .irq_disable = disable_brcm_irq_data,
    .irq_ack = ack_brcm_irq,
    .irq_mask = disable_brcm_irq_data,
    .irq_mask_ack = mask_ack_brcm_irq,
    .irq_unmask = enable_brcm_irq_data,
    .irq_set_affinity = set_brcm_affinity
};

static struct irq_chip brcm_irq_chip_no_unmask = {
    .name = "BCM63xx_no_unmask",
    .irq_enable = enable_brcm_irq_data,
    .irq_disable = disable_brcm_irq_data,
    .irq_ack = ack_brcm_irq,
    .irq_mask = disable_brcm_irq_data,
    .irq_mask_ack = mask_ack_brcm_irq,
    .irq_unmask = unmask_brcm_irq_noop,
    .irq_set_affinity = set_brcm_affinity
};


void __init arch_init_irq(void)
{
    int i;

    spin_lock_init(&brcm_irqlock);

    for (i = 0; i < NR_CPUS; i++) {
        brcm_irq_ctrl[i] = &PERF->IrqControl[i];
    }

    for (i = 0; i < NR_IRQS; i++) {
        irq_set_chip_and_handler(i, &brcm_irq_chip, handle_level_irq); 
    }

#if defined(CONFIG_BCM96838)
    PERF->ExtIrqCfg |= EI_CLEAR_MASK;
#endif

    clear_c0_status(ST0_BEV);
#if defined(CONFIG_SMP)
    // make interrupt mask same as TP1, miwang 6/14/10
#if defined(CONFIG_BCM96838)
    change_c0_status(ST0_IM, IE_IRQ1|IE_IRQ2);
#else
    change_c0_status(ST0_IM, IE_IRQ0|IE_IRQ1);
#endif
#else
#if defined(CONFIG_BCM96838)
    change_c0_status(ST0_IM, IE_IRQ1);
#else
    change_c0_status(ST0_IM, IE_IRQ0);
#endif
#endif


#ifdef CONFIG_REMOTE_DEBUG
    rs_kgdb_hook(0);
#endif
}


#define INTR_NAME_MAX_LENGTH 16

// This is a wrapper to standand Linux request_irq
// Differences are:
//    - The irq won't be renabled after ISR is done and needs to be explicity re-enabled, which is good for NAPI drivers.
//      The change is implemented by filling in an no-op unmask function in brcm_irq_chip_no_unmask and set it as the irq_chip
//    - IRQ flags and interrupt names are automatically set
// Either request_irq or BcmHalMapInterrupt can be used. Just make sure re-enabling IRQ is handled correctly.

unsigned int BcmHalMapInterrupt(FN_HANDLER pfunc, void* param, unsigned int irq)
{
    char devname[INTR_NAME_MAX_LENGTH];

    sprintf(devname, "brcm_%d", irq);
    return BcmHalMapInterruptEx(pfunc, param, irq, devname,
                                INTR_REARM_NO, INTR_AFFINITY_DEFAULT);
}


/** Broadcom wrapper to linux request_irq.  This version does more stuff.
 *
 * @param pfunc (IN) interrupt handler function
 * @param param (IN) context/cookie that is passed to interrupt handler
 * @param irq   (IN) interrupt number
 * @param interruptName (IN) descriptive name for the interrupt.  15 chars
 *                           or less.  This function will make a copy of
 *                           the name.
 * @param INTR_REARM_MODE    (IN) See bcm_intr.h
 * @param INTR_AFFINITY_MODE (IN) See bcm_intr.h
 *
 * @return 0 on success.
 */
unsigned int BcmHalMapInterruptEx(FN_HANDLER pfunc,
                                  void* param,
                                  unsigned int irq,
                                  const char *interruptName,
                                  INTR_REARM_MODE_ENUM rearmMode,
                                  INTR_AFFINITY_MODE_ENUM affinMode)
{
    char *devname;
    unsigned long irqflags;
    struct irq_chip *chip;
    unsigned int retval;

    BUG_ON(interruptName == NULL);
    BUG_ON(strlen(interruptName) >= INTR_NAME_MAX_LENGTH);

    if ((devname = kmalloc(INTR_NAME_MAX_LENGTH, GFP_ATOMIC)) == NULL)
    {
        printk(KERN_ERR "kmalloc(%d, GFP_ATOMIC) failed for intr name\n",
                        INTR_NAME_MAX_LENGTH);      
        return -1;
    }
    sprintf( devname, "%s", interruptName );

    /* If this is for the timer interrupt, do not invoke the following code
       because doing so kills the timer interrupts that may already be running */
    if (irq != INTERRUPT_ID_TIMER) {
        chip = (rearmMode == INTR_REARM_NO) ? &brcm_irq_chip_no_unmask :
                                              &brcm_irq_chip;
        irq_set_chip_and_handler(irq, chip, handle_level_irq);
    }

    if (rearmMode == INTR_REARM_YES)
    {
        irq_modify_status(irq, IRQ_NOAUTOEN, 0);
    }


    irqflags = 0;
#if defined(CONFIG_BCM_EXT_TIMER)
    /* There are 3 timers with individual control, so the interrupt can be shared */
    if ( (irq >= INTERRUPT_ID_TIMER) && (irq < (INTERRUPT_ID_TIMER+EXT_TIMER_NUM)) )
         irqflags |= IRQF_SHARED;
#endif
    /* For external interrupt, check if it is shared */
    if ( (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3)
#if defined(CONFIG_BCM96838)
     || (irq >= INTERRUPT_ID_EXTERNAL_4 && irq <= INTERRUPT_ID_EXTERNAL_5)
#elif defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
     || (irq >= INTERRUPT_ID_EXTERNAL_4 && irq <= INTERRUPT_ID_EXTERNAL_7)
#endif
       )
    {
       if( IsExtIntrShared(kerSysGetExtIntInfo(irq)) )
    	   irqflags |= IRQF_SHARED;
    }

    retval = request_irq(irq, (void*)pfunc, irqflags, devname, (void *) param);
    if (retval != 0)
    {
        printk(KERN_WARNING "request_irq failed for irq=%d (%s) retval=%d\n",
               irq, devname, retval);
        kfree(devname);
        return retval;
    }

    // now deal with interrupt affinity requests
    if (affinMode != INTR_AFFINITY_DEFAULT)
    {
        struct cpumask mask;

        cpumask_clear(&mask);

        if (affinMode == INTR_AFFINITY_TP1_ONLY ||
            affinMode == INTR_AFFINITY_TP1_IF_POSSIBLE)
        {
            if (cpu_online(1))
            {
                cpumask_set_cpu(1, &mask);
                irq_set_affinity(irq, &mask);
            }
            else
            {
                // TP1 is not on-line but caller insisted on it
                if (affinMode == INTR_AFFINITY_TP1_ONLY)
                {
                    printk(KERN_WARNING
                           "cannot assign intr %d to TP1, not online\n", irq);
                    retval = request_irq(irq, NULL, 0, NULL, NULL);
                    kfree(devname);
                    retval = -1;
                }
            }
        }
        else
        {
            // INTR_AFFINITY_BOTH_IF_POSSIBLE
            cpumask_set_cpu(0, &mask);
            if (cpu_online(1))
            {
                cpumask_set_cpu(1, &mask);
                irq_set_affinity(irq, &mask);
            }
        }
    }

    return retval;
}
EXPORT_SYMBOL(BcmHalMapInterruptEx);


//***************************************************************************
//  void  BcmHalGenerateSoftInterrupt
//
//   Triggers a software interrupt.
//
//***************************************************************************
void BcmHalGenerateSoftInterrupt( unsigned int irq )
{
    unsigned long flags;

    local_irq_save(flags);

    set_c0_cause(0x1 << (CAUSEB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));

    local_irq_restore(flags);
}

void BcmHalExternalIrqMask(unsigned int irq)
{
    unsigned long flags;
    spin_lock_irqsave(&brcm_irqlock, flags);
#if defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
    PERF->ExtIrqSts &= ~(1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
#else
    PERF->ExtIrqCfg &= ~(1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
#endif
    spin_unlock_irqrestore(&brcm_irqlock, flags); 
}

void BcmHalExternalIrqUnmask(unsigned int irq)
{
    unsigned long flags;
    spin_lock_irqsave(&brcm_irqlock, flags);
#if defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
    PERF->ExtIrqSts |= (1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
#else
    PERF->ExtIrqCfg |= (1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
#endif
    spin_unlock_irqrestore(&brcm_irqlock, flags); 
}

EXPORT_SYMBOL(enable_brcm_irq_irq);
EXPORT_SYMBOL(disable_brcm_irq_irq);
EXPORT_SYMBOL(BcmHalMapInterrupt);
EXPORT_SYMBOL(BcmHalGenerateSoftInterrupt);
EXPORT_SYMBOL(BcmHalExternalIrqMask);
EXPORT_SYMBOL(BcmHalExternalIrqUnmask);

#if !defined(CONFIG_BCM96838)
// bill
EXPORT_SYMBOL(disable_brcm_irqsave);
EXPORT_SYMBOL(restore_brcm_irqsave);
#endif

#endif //defined(CONFIG_BCM_KF_MIPS_BCM963XX)
