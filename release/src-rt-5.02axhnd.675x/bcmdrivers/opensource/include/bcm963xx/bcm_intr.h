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

#ifndef __BCM_INTR_H
#define __BCM_INTR_H

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#if !defined(FAP_4KE)
#include <linux/irq.h>
#endif
#endif

#ifdef __cplusplus
    extern "C" {
#endif

#if defined(CONFIG_BCM963268)
#include <63268_intr.h>
#endif
#if defined(CONFIG_BCM96838)
#include <6838_intr.h>
#endif
#if defined(CONFIG_BCM963138)
#include <63138_intr.h>
#endif
#if defined(CONFIG_BCM960333)
#include <60333_intr.h>
#endif
#if defined(CONFIG_BCM963381)
#include <63381_intr.h>
#endif
#if defined(CONFIG_BCM963148)
#include <63148_intr.h>
#endif
#if defined(CONFIG_BCM96848)
#include <6848_intr.h>
#endif
#if defined(CONFIG_BCM94908)
#include <4908_intr.h>
#endif
#if defined(CONFIG_BCM96858)
#include <6858_intr.h>
#endif
#if defined(CONFIG_BCM947189)
#include <47189_intr.h>
#endif
#if defined(CONFIG_BCM963158)
#include <63158_intr.h>
#endif
#if defined(CONFIG_BCM96846)
#include <6846_intr.h>
#endif
#if defined(CONFIG_BCM947622)
#include <47622_intr.h>
#endif
#if defined(CONFIG_BCM963178)
#include <63178_intr.h>
#endif
#if defined(CONFIG_BCM96856)
#include <6856_intr.h>
#endif
#if defined(CONFIG_BCM96878)
#include <6878_intr.h>
#endif

/* defines */
/* The following definitions must match the definitions in linux/interrupt.h for 
   irqreturn_t and irq_handler_t */

typedef enum {  
    BCM_IRQ_NONE = 0,
    BCM_IRQ_HANDLED = 1,
    BCM_IRQ_WAKE_THREAD = 2
} FN_HANDLER_RT;
typedef FN_HANDLER_RT (*FN_HANDLER) (int, void *);


/** used by BcmHalMapInterruptEx */
/* REARM_MODE option is not supported for ARM GIC.  Re-arm is always yes for ARM */
typedef enum {
   INTR_REARM_NO=0,
   INTR_REARM_YES=1
} INTR_REARM_MODE_ENUM;

/** used by BcmHalMapInterruptEx */
typedef enum {
   INTR_AFFINITY_DEFAULT=0,
   INTR_AFFINITY_TP1_ONLY=1,  /**< set affinity to TP1, complain if no TP1 */
   INTR_AFFINITY_TP1_IF_POSSIBLE=2, /**< set affinity to TP1, silently use
                                         TP0 if TP1 not available */
   INTR_AFFINITY_BOTH_IF_POSSIBLE=3 /**< set affinity to both TP, silently
                                         use TP0 if TP1 not available */
} INTR_AFFINITY_MODE_ENUM;


/* prototypes */
/* NOTE: The enable/disable functions do not appear symmetric, allowing enabling but not disabling of external interrupts. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#if !defined(FAP_4KE)
extern void enable_brcm_irq_irq(unsigned int irq);
extern void disable_brcm_irq_irq(unsigned int irq);
extern void enable_brcm_irq_noop(unsigned int irq);
extern void disable_brcm_irq_data(struct irq_data *data);
extern void enable_brcm_irq_data(struct irq_data *data);
#endif  /* FAP4KE */
#else /* KERNEL_VERSION */
extern void enable_brcm_irq(unsigned int irq);
extern void disable_brcm_irq(unsigned int irq);
#endif /* KERNEL_VERSION */

extern unsigned int BcmHalMapInterrupt(FN_HANDLER isr, void* param, unsigned int interruptId);
extern unsigned int BcmHalMapInterruptEx(FN_HANDLER isr,
                                         void* param,
                                         unsigned int interruptNum,
                                         const char *interruptName,
                                         INTR_REARM_MODE_ENUM rearmMode,
                                         INTR_AFFINITY_MODE_ENUM affinMode);
extern void BcmHalExternalIrqClear(unsigned int irq);
extern void BcmHalExternalIrqMask(unsigned int irq);
extern void BcmHalExternalIrqUnmask(unsigned int irq);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#if !defined(FAP_4KE)
extern void BcmHalSetIrqAffinity(unsigned int irq, struct cpumask *mask);
#endif
#endif
int ext_irq_connect(int irq, void* param, FN_HANDLER isr);
void ext_irq_disconnect(int irq, void* param);
void ext_irq_enable(int irq, int enable);
extern void dump_intr_regs(void);

#if !defined(FAP_4KE)
/* bill BCM interrupt control prototypes */
/* NOTE: These routines only operate on HW interrupts between INTERRUPT_ID_TIMER and INTERRUPT_ID_I2C. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
extern void disable_brcm_irqsave(struct irq_data *data, unsigned long stateSaveArray[]);
extern void restore_brcm_irqsave(struct irq_data *data, unsigned long stateSaveArray[]);
#else
extern void disable_brcm_irqsave(unsigned int irq, unsigned long stateSaveArray[]);
extern void restore_brcm_irqsave(unsigned int irq, unsigned long stateSaveArray[]);
#endif // Kernel version
#endif

/* compatibility definitions */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#define BcmHalInterruptEnable(irq)      enable_brcm_irq_irq( irq )
#define BcmHalInterruptDisable(irq)     disable_brcm_irq_irq( irq )
#else
#define BcmHalInterruptEnable(irq)      enable_brcm_irq( irq )
#define BcmHalInterruptDisable(irq)     disable_brcm_irq( irq )
#endif

#ifdef CONFIG_HOTPLUG_CPU
#include <linux/cpu.h>
extern void bcm63xx_rehint_one_irq(struct irq_desc *desc, int cpu);
extern int bcm63xx_rehint_irqaffinity(struct notifier_block *nb, unsigned long action, void *hcpu);
#endif
struct cpumask;
int bcm_irq_set_affinity_hint(int irq, const struct cpumask *mask);
int bcm_set_affinity(int irq, int cpu_id, int force);

#ifdef __cplusplus
    }
#endif

#endif
