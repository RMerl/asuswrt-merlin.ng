/*
  <:copyright-BRCM:2023:DUAL/GPL:standard
  
     Copyright (c) 2023 Broadcom 
     All Rights Reserved
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2, as published by
  the Free Software Foundation (the "GPL").
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  
  A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
  writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
  
  :>
*/

/*
*******************************************************************************
* File Name  : crossbow_gpl.h
*
*******************************************************************************
*/

#ifndef __CROSSBOW_GPL_H__
#define __CROSSBOW_GPL_H__

#include <linux/nbuff.h>

#if defined(CONFIG_BCM96766)
#define CC_CROSSBOW_TIMER
#endif

#define CROSSBOW_RXQ_MAX  2

#if defined(CONFIG_BCM96765) || defined(CONFIG_BCM96766)
#define CROSSBOW_ACQ_MAX    7
#define CROSSBOW_STQ_MAX    CROSSBOW_ACQ_MAX
#define CROSSBOW_FWQ_MAX    CROSSBOW_ACQ_MAX
#define CROSSBOW_TIMER_MAX  2
#endif

typedef struct {
    void *virt_p;
    phys_addr_t phys_addr;
    unsigned int size;    
} crossbow_gpl_flow_memory_t;

typedef struct {
    uint8_t *pData;
    uint16_t data_len;
    uint8_t ingress_port;
    struct {
        uint8_t rx_csum_verified    : 4;
        uint8_t group_fwd_exception : 4;
    };
} crossbow_gpl_rxq_entry_t;

typedef struct {
    pNBuff_t pNBuff;
    uint16_t egress_port;
    uint16_t egress_queue;
} crossbow_gpl_txq_entry_t;

typedef struct {
    uint32_t size;
    uint32_t level;
    uint32_t discards;
} crossbow_gpl_rxq_stats_t;

typedef struct {
    uint32_t size;
    uint32_t level;
} crossbow_gpl_txq_stats_t;

typedef struct {
    uint32_t size;
    uint32_t level;
    uint32_t bp_count;
} crossbow_gpl_rcq_stats_t;

typedef struct {
    crossbow_gpl_rxq_stats_t rxq[CROSSBOW_RXQ_MAX];
    crossbow_gpl_txq_stats_t txq;
    crossbow_gpl_rcq_stats_t rcq;
} crossbow_gpl_enet_stats_t;

typedef struct {
    // Crossbow Driver
    // RXQ
    int (* rxq_read)(int rxq_index, crossbow_gpl_rxq_entry_t *entry_p);
    int (* rxq_not_empty)(int rxq_index);
    void(* rxq_intr_disable)(int rxq_index);
    void(* rxq_intr_enable)(int rxq_index);
    void(* rxq_intr_clear)(int rxq_index);
    // TXQ
    int (* txq_write)(crossbow_gpl_txq_entry_t *entry_p);
    // RCQ
    int (* rcq_read)(pNBuff_t *pNBuff_p);
    int (* rcq_not_empty)(void);
    void(* rcq_intr_disable)(void);
    void(* rcq_intr_enable)(void);
    void(* rcq_intr_clear)(void);
    void(* rcq_fkb_free)(FkBuff_t *fkb);
    // STATS
    void (* stats_get)(crossbow_gpl_enet_stats_t *stats_p);
    // Ethernet Driver
    irqreturn_t (* rxq_isr)(int irq, void *param);
    int rxq_isr_cpu_id;
    irqreturn_t (* rcq_isr)(int irq, void *param);
    int rcq_isr_cpu_id;
    void (* stats_dump)(void);
    struct task_struct *archer_task_p;
} crossbow_enet_hooks_t;

extern crossbow_enet_hooks_t crossbow_gpl_hooks_g;

#if defined(CC_CROSSBOW_TIMER)

typedef void (*crossbow_timer_handler_t)(unsigned long param);

typedef struct {
    int (* timer_attach)(unsigned int timer_index, crossbow_timer_handler_t handler,
                         unsigned long param);
    int (* timer_detach)(unsigned int timer_index);
    int (* timer_stop)(unsigned int timer_index);
    int (* timer_start)(unsigned int timer_index);
    int (* timer_set_period)(unsigned int timer_index, unsigned int period);
} crossbow_timer_hooks_t;

extern crossbow_timer_hooks_t crossbow_gpl_timer_hooks_g;

#endif /* CC_CROSSBOW_TIMER */

static inline int crossbow_gpl_rxq_read(int rxq_index,
                                        crossbow_gpl_rxq_entry_t *entry_p)
{
    return crossbow_gpl_hooks_g.rxq_read(rxq_index, entry_p);
}

static inline int crossbow_gpl_rxq_not_empty(int rxq_index)
{
    return crossbow_gpl_hooks_g.rxq_not_empty(rxq_index);
}

static inline void crossbow_gpl_rxq_intr_disable(int rxq_index)
{
    crossbow_gpl_hooks_g.rxq_intr_disable(rxq_index);
}

static inline void crossbow_gpl_rxq_intr_enable(int rxq_index)
{
    crossbow_gpl_hooks_g.rxq_intr_enable(rxq_index);
}

static inline void crossbow_gpl_rxq_intr_clear(int rxq_index)
{
    crossbow_gpl_hooks_g.rxq_intr_clear(rxq_index);
}

static inline int crossbow_gpl_txq_write(crossbow_gpl_txq_entry_t *entry_p)
{
    return crossbow_gpl_hooks_g.txq_write(entry_p);
}

static inline int crossbow_gpl_rcq_read(pNBuff_t *pNBuff_p)
{
    return crossbow_gpl_hooks_g.rcq_read(pNBuff_p);
}

static inline int crossbow_gpl_rcq_not_empty(void)
{
    return crossbow_gpl_hooks_g.rcq_not_empty();
}

static inline void crossbow_gpl_rcq_intr_disable(void)
{
    crossbow_gpl_hooks_g.rcq_intr_disable();
}

static inline void crossbow_gpl_rcq_intr_enable(void)
{
    crossbow_gpl_hooks_g.rcq_intr_enable();
}

static inline void crossbow_gpl_rcq_intr_clear(void)
{
    crossbow_gpl_hooks_g.rcq_intr_clear();
}

static inline void crossbow_gpl_enet_stats_get(crossbow_gpl_enet_stats_t *stats_p)
{
    crossbow_gpl_hooks_g.stats_get(stats_p);
}

static inline irqreturn_t crossbow_gpl_rxq_isr(int irq, void *param)
{
    return crossbow_gpl_hooks_g.rxq_isr(irq, param);
}

static inline int crossbow_gpl_rxq_isr_cpu_id(void)
{
    return crossbow_gpl_hooks_g.rxq_isr_cpu_id;
}

static inline irqreturn_t crossbow_gpl_rcq_isr(int irq, void *param)
{
    return crossbow_gpl_hooks_g.rcq_isr(irq, param);
}

static inline int crossbow_gpl_rcq_isr_cpu_id(void)
{
    return crossbow_gpl_hooks_g.rcq_isr_cpu_id;
}

static inline void crossbow_gpl_rcq_fkb_free(FkBuff_t *fkb)
{
    crossbow_gpl_hooks_g.rcq_fkb_free(fkb);
}

static inline void crossbow_gpl_enet_stats_dump(void)
{
    crossbow_gpl_hooks_g.stats_dump();
}

int crossbow_gpl_enet_bind(crossbow_enet_hooks_t *hooks_p);

// Timers

#if defined(CC_CROSSBOW_TIMER)

static inline int crossbow_gpl_timer_attach(unsigned int timer_index,
                                            crossbow_timer_handler_t handler,
                                            unsigned long param)
{
    return crossbow_gpl_timer_hooks_g.timer_attach(timer_index, handler, param);
}

static inline int crossbow_gpl_timer_detach(unsigned int timer_index)
{
    return crossbow_gpl_timer_hooks_g.timer_detach(timer_index);
}

static inline int crossbow_gpl_timer_stop(unsigned int timer_index)
{
    return crossbow_gpl_timer_hooks_g.timer_stop(timer_index);
}

static inline int crossbow_gpl_timer_start(unsigned int timer_index)
{
    return crossbow_gpl_timer_hooks_g.timer_start(timer_index);
}

static inline int crossbow_gpl_timer_set_period(unsigned int timer_index,
                                                unsigned int period)
{
    return crossbow_gpl_timer_hooks_g.timer_set_period(timer_index, period);
}

int crossbow_gpl_timer_bind(crossbow_timer_hooks_t *hooks_p);

#endif /* CC_CROSSBOW_TIMER */

// Miscellaneous

int crossbow_gpl_flow_memory_get(crossbow_gpl_flow_memory_t *table_mem_p);

int crossbow_gpl_construct(void);

#endif /* __CROSSBOW_GPL_H__ */
