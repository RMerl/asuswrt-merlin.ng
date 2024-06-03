/*
  <:copyright-BRCM:2023:DUAL/GPL:standard

  Copyright (c) 2023 Broadcom 
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
*******************************************************************************
* File Name  : crossbow_gpl.h
*
*******************************************************************************
*/

#ifndef __CROSSBOW_GPL_H__
#define __CROSSBOW_GPL_H__

#include <linux/nbuff.h>

#define CROSSBOW_RXQ_MAX  2

#if defined(CONFIG_BCM96765)
#define CROSSBOW_ACQ_MAX  7
#define CROSSBOW_STQ_MAX  CROSSBOW_ACQ_MAX
#define CROSSBOW_FWQ_MAX  CROSSBOW_ACQ_MAX
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

int crossbow_gpl_flow_memory_get(crossbow_gpl_flow_memory_t *table_mem_p);

int crossbow_gpl_construct(void);

#endif /* __CROSSBOW_GPL_H__ */
