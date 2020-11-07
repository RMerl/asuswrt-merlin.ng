/*
* <:copyright-BRCM:2013-2015:proprietary:standard
*
*    Copyright (c) 2013-2015 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
 :>
*/

#ifndef RDP_SIM
#include<linux/kthread.h>
#endif
#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdd_common.h"
#include "rdd_cpu_rx.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_egress_tm_inline.h"
#include "rdp_cpu_ring.h"
#include "rdpa_platform.h"
#include "rdpa_cpu_ex.h"
#include "rdp_drv_qm.h"
#include "rdp_drv_fpm.h"
#include "rdd_cpu_tx.h"
#include "xrdp_drv_ubus_slv_ag.h"
#include "bdmf_sysb_chain.h"
#include "rdp_cpu_ring_inline.h"
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include "rdd_spdsvc.h"
#endif
#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
#include "rdpa_egress_tm_ex.h"
#endif
#ifdef CONFIG_BCM_UDPSPDTEST_SUPPORT
#include "rdpa_spdtest_common_ex.h"
#endif
#if defined(BCM63158)
#include "rdpa_cpu_dsl_inline.h"
#endif

#define RECYCLE_RING_BUDGET 2048
#define RECYCLE_INTERRUPT_THRESHOLD 64
#define FEED_RING_BUDGET 1024
#ifndef RDP_SIM
#define DEF_DATA_RING_SIZE 	1024
#define FEED_RING_MIN_SIZE (8 * 1024)
#define FEED_RING_MID_SIZE (16 * 1024)
#define FEED_RING_MAX_SIZE (32 * 1024)

#else
#define DEF_DATA_RING_SIZE 	128
#define SIM_FEED_RING_SIZE  (4096 + 128)
#endif

#define FEED_RING_LOW_WATERMARK(feed_ring_size) (feed_ring_size - (feed_ring_size >> 2))
#define RECYCLE_RING_HIGH_WATERMARK (RECYCLE_RING_SIZE - 8192)

/* update rate of CPU_TX tasks */
#define CPU_TX_SEND_RATE_FOR_CHAINED 127 

extern struct bdmf_object *cpu_object[rdpa_cpu_port__num_of];
/* cpu_port enum values */
bdmf_attr_enum_table_t cpu_port_enum_table = {
    .type_name = "rdpa_cpu_port",
    .values = {
        {"host",     rdpa_cpu_host},
        {"cpu1",     rdpa_cpu1},
        {"cpu2",     rdpa_cpu2},
        {"cpu3",     rdpa_cpu3},
        {"wlan0",    rdpa_cpu_wlan0},
        {"wlan1",    rdpa_cpu_wlan1},
        {"wlan2",    rdpa_cpu_wlan2},
        {NULL,      0}
    }
};

extern bdmf_attr_enum_table_t cpu_tx_method_enum_table;

static bdmf_boolean cpu_tx_disable;
struct task_struct *recycle_task_s = NULL;
struct task_struct *feed_task_s = NULL;
#ifndef RDP_SIM
static wait_queue_head_t recycle_thread_wqh;
static volatile uint32_t recycle_wakeup;

wait_queue_head_t feed_thread_wqh;
volatile uint32_t feed_wakeup;
#endif

DEFINE_BDMF_FASTLOCK(ier_lock);
DEFINE_BDMF_FASTLOCK(isr_lock);

static bdmf_fastlock cpu_tx_lock[2];
typedef struct {
    rdpa_cpu_port cpu_obj_idx;
    uint8_t rxq_idx;
} rdd_rxq_map_t;

static rdd_rxq_map_t rdd_rxq_map[RING_ID_NUM_OF];
static int num_of_avail_data_queues;
static int feed_ring_low_watermark;

/* Init/exit module. Cater for GPL layer */
#ifdef BDMF_DRIVER_GPL_LAYER
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908) || defined(BCM63158)
extern int (*f_rdpa_cpu_tx_port_enet_or_dsl_wan)(bdmf_sysb sysb,
    uint32_t egress_queue, rdpa_flow wan_flow, rdpa_if wan_if, rdpa_cpu_tx_extra_info_t extra_info);
#endif
#endif

#ifndef CONFIG_BCM_FEED_RING_DYNAMIC
static int get_feed_ring_size(uint32_t *feed_ring_size)
{
#ifndef RDP_SIM
    uint32_t bpm_avail_bufs;
    
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    bpm_avail_bufs = gbpm_get_avail_bufs();

    if (FEED_RING_MIN_SIZE > bpm_avail_bufs)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOMEM, "Not enough bpm buffers=%d for allocating minimum feed ring size=%d\n", bpm_avail_bufs, FEED_RING_MIN_SIZE);
        return -1;
    }
#else  
    bpm_avail_bufs = FEED_RING_MIN_SIZE;
#endif

    if (bpm_avail_bufs > FEED_RING_MAX_SIZE)
        *feed_ring_size = FEED_RING_MAX_SIZE;
    else if (bpm_avail_bufs > FEED_RING_MID_SIZE)
        *feed_ring_size = FEED_RING_MID_SIZE;
    else
        *feed_ring_size = FEED_RING_MIN_SIZE;
#else
    *feed_ring_size = SIM_FEED_RING_SIZE;
#endif

    return BDMF_ERR_OK;
}
#endif /* CONFIG_BCM_FEED_RING_DYNAMIC */

static void dump_list_of_queues(void)
{
    int i;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("RDD queues map:\n");
    bdmf_trace("===============\n");
    for (i = 0; i < RING_ID_NUM_OF; i++)
    {
        bdmf_trace("RDD Q #%d, CPU object %d, Q #%d\n", i, (int)rdd_rxq_map[i].cpu_obj_idx,
            (int)rdd_rxq_map[i].rxq_idx);
    }
    bdmf_trace("\n");
}

static int rdd_rxq_map_alloc(cpu_drv_priv_t *cpu_data)
{
    int i, j;

    if (cpu_data->num_queues > num_of_avail_data_queues)
    {
        bdmf_trace("Cannot allocate requested number of queues (requested %d, available %d)\n",
            cpu_data->num_queues, num_of_avail_data_queues);
        dump_list_of_queues();
        return BDMF_ERR_NORES;
    }

    memset(cpu_data->rxq_to_rdd_rxq, (uint8_t)BDMF_INDEX_UNASSIGNED, RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ);
    for (i = 0, j = 0; j < cpu_data->num_queues; i++)
    {
        if (rdd_rxq_map[i].rxq_idx == (uint8_t)BDMF_INDEX_UNASSIGNED)
        {
            rdd_rxq_map[i].cpu_obj_idx = cpu_data->index;
            rdd_rxq_map[i].rxq_idx = j;
            num_of_avail_data_queues--;
            cpu_data->rxq_to_rdd_rxq[j] = i;
            memset(&(cpu_data->accumulative_rxq_stat[j]), 0, sizeof(rdpa_cpu_rx_stat_t));
            memset(&(cpu_data->rxq_stat[j]), 0, sizeof(rdpa_cpu_rx_stat_t));
            j++;
        }
    }

    return 0;
}

static void rdd_rxq_map_free(cpu_drv_priv_t *cpu_data)
{
    int i;
    uint8_t rdd_rxq_idx;

    for (i = 0; i < cpu_data->num_queues; i++)
    {
        rdd_rxq_idx = cpu_data->rxq_to_rdd_rxq[i];
        rdd_rxq_map[rdd_rxq_idx].cpu_obj_idx = rdpa_cpu_port__num_of;
        rdd_rxq_map[rdd_rxq_idx].rxq_idx = (uint8_t)BDMF_INDEX_UNASSIGNED;
        cpu_data->rxq_to_rdd_rxq[i] = BDMF_INDEX_UNASSIGNED;
        memset(&(cpu_data->accumulative_rxq_stat[i]), 0, sizeof(rdpa_cpu_rx_stat_t));
        memset(&(cpu_data->rxq_stat[i]), 0, sizeof(rdpa_cpu_rx_stat_t));
        num_of_avail_data_queues++;
    }
}

static void rdd_rxq_map_reset(void)
{
    int i;

    num_of_avail_data_queues = DATA_RING_ID_LAST + 1; /* Data rings */
    feed_ring_low_watermark = 0;
    for (i = 0; i < num_of_avail_data_queues; i++)
    {
        rdd_rxq_map[i].cpu_obj_idx = rdpa_cpu_port__num_of;
        rdd_rxq_map[i].rxq_idx = (uint8_t)BDMF_INDEX_UNASSIGNED;
    }
    rdd_rxq_map[FEED_RING_ID].cpu_obj_idx = rdpa_cpu_host;
    rdd_rxq_map[FEED_RING_ID].rxq_idx = FEED_RING_ID;
    rdd_rxq_map[FEED_RCYCLE_RING_ID].cpu_obj_idx = rdpa_cpu_host;
    rdd_rxq_map[FEED_RCYCLE_RING_ID].rxq_idx = FEED_RCYCLE_RING_ID;
    rdd_rxq_map[TX_RCYCLE_RING_ID].cpu_obj_idx = rdpa_cpu_host;
    rdd_rxq_map[TX_RCYCLE_RING_ID].rxq_idx = TX_RCYCLE_RING_ID;
    rdd_rxq_map[TX_HIGH_PRIO_RING_ID].cpu_obj_idx = rdpa_cpu_host;
    rdd_rxq_map[TX_HIGH_PRIO_RING_ID].rxq_idx = TX_HIGH_PRIO_RING_ID;
    rdd_rxq_map[TX_LOW_PRIO_RING_ID].cpu_obj_idx = rdpa_cpu_host;
    rdd_rxq_map[TX_LOW_PRIO_RING_ID].rxq_idx = TX_LOW_PRIO_RING_ID;
}

void rdpa_cpu_tx_disable(bdmf_boolean en)
{
    cpu_tx_disable = en;
}

void cpu_destroy_ex(struct bdmf_object *mo)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    for (i = 0; i < cpu_data->num_queues; i++)
    {
        if (cpu_data->rxq_cfg[i].size)
            rdpa_cpu_int_disconnect_ex(cpu_data, i);
    }
    rdd_rxq_map_free(cpu_data);
}

rdpa_ports rdpa_ports_all_lan(void)
{
    return RDPA_PORT_ALL_LAN;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_ports_all_lan);
#endif

#ifdef RDP_SIM
#define INTERRUPT_ID_XRDP_QUEUE_0 0
#endif

static int _cpu_isr_wrapper(int irq, void *priv)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)priv;
    uint8_t rdd_rxq_idx = irq - INTERRUPT_ID_XRDP_QUEUE_0;
    uint8_t queue_id = rdd_rxq_map[rdd_rxq_idx].rxq_idx;
    rdpa_cpu_rxq_cfg_t *rxq_cfg = &cpu_data->rxq_cfg[queue_id];

    cpu_data->rxq_stat[queue_id].interrupts++;
    rxq_cfg->rx_isr(rxq_cfg->isr_priv);

    bdmf_int_enable(irq);

    return BDMF_IRQ_HANDLED;
}

int rdpa_cpu_int_connect_ex(cpu_drv_priv_t *cpu_data, int queue_id, uint32_t affinity_mask)
{
    uint8_t rdd_rxq_idx;
    int irq, rc;
#ifndef RDP_SIM
    int cpu_num;
    struct cpumask cpus_mask;
#endif

    rdd_rxq_idx = cpu_rdd_rxq_idx_get(cpu_data, queue_id);
    if (rdd_rxq_idx == (uint8_t)BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_INTERNAL;

    irq = INTERRUPT_ID_XRDP_QUEUE_0 + rdd_rxq_idx;

    /* Connect IRQ */
    rc = bdmf_int_connect(irq, cpu_data->cpu_id, BDMF_IRQF_DISABLED,
        _cpu_isr_wrapper, bdmf_attr_get_enum_text_hlp(&cpu_port_enum_table,
        cpu_data->index), cpu_data);

    if (!rc)
        bdmf_int_enable(irq);

    if (!affinity_mask)
        return rc;

#ifndef RDP_SIM
    cpumask_clear(&cpus_mask);
    for (cpu_num = ffs(affinity_mask) - 1; affinity_mask; affinity_mask &= ~(1L << cpu_num), cpu_num = ffs(affinity_mask))
        cpumask_set_cpu(cpu_num, &cpus_mask);
    BcmHalSetIrqAffinity(irq, &cpus_mask);
#endif

    return rc;
}

void rdpa_cpu_int_disconnect_ex(cpu_drv_priv_t *cpu_data, int queue_id)
{
    uint8_t rdd_rxq_idx = cpu_rdd_rxq_idx_get(cpu_data, queue_id);

    if (rdd_rxq_idx == (uint8_t)BDMF_INDEX_UNASSIGNED)
        return;

    bdmf_int_disconnect(INTERRUPT_ID_XRDP_QUEUE_0 + rdd_rxq_idx, cpu_data);
}

static inline void __rdpa_cpu_int_enable(uint8_t rdd_rxq_idx)
{
    uint32_t intr_en_reg;
    unsigned long flags;

    bdmf_fastlock_lock_irq(&ier_lock, flags);

    ag_drv_ubus_slv_rnr_intr_ctrl_ier_get(&intr_en_reg);
    intr_en_reg |= (1 << rdd_rxq_idx);
    ag_drv_ubus_slv_rnr_intr_ctrl_ier_set(intr_en_reg);

    bdmf_fastlock_unlock_irq(&ier_lock, flags);
}

static inline void __rdpa_cpu_int_disable(uint8_t rdd_rxq_idx)
{
    uint32_t intr_en_reg;
    unsigned long flags;

    bdmf_fastlock_lock_irq(&ier_lock, flags);

    ag_drv_ubus_slv_rnr_intr_ctrl_ier_get(&intr_en_reg);
    intr_en_reg &= ~(1 << rdd_rxq_idx);
    ag_drv_ubus_slv_rnr_intr_ctrl_ier_set(intr_en_reg);

    bdmf_fastlock_unlock_irq(&ier_lock, flags);
}

static inline void __rdpa_cpu_int_clear(uint8_t rdd_rxq_idx)
{
    uint32_t mask = (1 << rdd_rxq_idx);
    unsigned long flags;

    bdmf_fastlock_lock_irq(&isr_lock, flags);
    ag_drv_ubus_slv_rnr_intr_ctrl_isr_set(mask);
    bdmf_fastlock_unlock_irq(&isr_lock, flags);
}

void rdpa_rnr_int_enable(uint8_t intr_idx)
{
    __rdpa_cpu_int_enable(intr_idx);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_rnr_int_enable);
#endif

void rdpa_rnr_int_disable(uint8_t intr_idx)
{
    __rdpa_cpu_int_disable(intr_idx);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_rnr_int_disable);
#endif

void rdpa_rnr_int_clear(uint8_t intr_idx)
{
    __rdpa_cpu_int_clear(intr_idx);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_rnr_int_clear);
#endif

#ifndef RDP_SIM

static int good_rcycle_level(ring_id_t rid)
{
    const int queued = rdp_cpu_ring_get_queued(rid);
    const int cur_low = host_ring[rid].num_of_entries - queued;

    if (host_ring[rid].lowest_filling_level > cur_low)
        host_ring[rid].lowest_filling_level = cur_low;
    
    return queued < RECYCLE_INTERRUPT_THRESHOLD;
}

static int _rdpa_recycle_thread_handler(void *data)
{
    int feed_rcycle_cnt, tx_rcycle_cnt;

    while (1)
    {
        wait_event_interruptible(recycle_thread_wqh, recycle_wakeup || kthread_should_stop());
        if (kthread_should_stop())
        {
            BDMF_TRACE_ERR("kthread_should_stop detected in recycle\n");
            break;
        }

        feed_rcycle_cnt = rdp_cpu_ring_recycle_free_host_buf(FEED_RCYCLE_RING_ID, RECYCLE_RING_BUDGET);
        tx_rcycle_cnt = rdp_cpu_ring_recycle_free_host_buf(TX_RCYCLE_RING_ID, RECYCLE_RING_BUDGET);

        if (feed_rcycle_cnt == RECYCLE_RING_BUDGET || tx_rcycle_cnt == RECYCLE_RING_BUDGET)
        {
            /* Budget for one other rings exceeded, it's possible that more buffers remained. Reschedule */
            yield();
            continue;
        }

        /*In Order to be more burst proof we set small threshold before enabling interrupts again*/
        if (good_rcycle_level(FEED_RCYCLE_RING_ID) && good_rcycle_level(TX_RCYCLE_RING_ID))
        {
            recycle_wakeup = 0;
            __rdpa_cpu_int_enable(FEED_RCYCLE_RING_ID);
            __rdpa_cpu_int_enable(TX_RCYCLE_RING_ID);
        }
        else
        {
            /* It's possible that when we reached this moment, one of the queues is full again. Return recycling */
            yield();
        }
    }

    return 0;
}

static void _recycle_isr_wrapper(long priv)
{
    __rdpa_cpu_int_disable((int)priv);
    __rdpa_cpu_int_clear((int)priv);

    recycle_wakeup = 1;
    wake_up_interruptible(&recycle_thread_wqh);
}

#ifdef CONFIG_BCM_FEED_RING_DYNAMIC
int refill_every = CONFIG_BCM_FEED_RING_REFILL_EVERY;
atomic_t allocated_packets;
int max_allocations;
int allocs_count;
int alloc_checks_count;
static uint16_t alloc_start;

static int _rdpa_feed_thread_handler(void *data)
{
    while (1)
    {
        int queued, rc, alp;
        wait_event_interruptible(feed_thread_wqh, feed_wakeup);
        feed_wakeup = 0;
        alloc_checks_count++;

#ifndef RDP_SIM  
        bdmf_fastlock_lock(&feed_ring_lock);
#endif
        queued = rdp_cpu_feed_ring_get_queued();
#ifndef RDP_SIM  
        bdmf_fastlock_unlock(&feed_ring_lock);
#endif
        if (queued > alloc_start)
        {
            if (refill_every < CONFIG_BCM_FEED_RING_REFILL_EVERY)
                refill_every *= 2;
            continue;
        }

        refill_every = 1;
        alp = atomic_read(&allocated_packets);
        if (alp > CONFIG_BCM_FEED_RING_MAX_ALLOCATIONS)
            continue;

        rc = rdp_cpu_fill_feed_ring(CONFIG_BCM_FEED_RING_ALLOC_BATCH);
        atomic_add(rc, &allocated_packets);
        allocs_count++;
        if (alp > max_allocations)
            max_allocations = alp;
    }
    return 0;
}

void rdpa_feed_ring_refill_kick(void)
{
    feed_wakeup = 1;
    wake_up_interruptible(&feed_thread_wqh);
}

#endif /* CONFIG_BCM_FEED_RING_DYNAMIC */

#elif !defined(XRDP_EMULATION)
static void _recycle_isr_wrapper(long priv)
{
    rdp_cpu_ring_recycle_free_host_buf((int)priv, RECYCLE_RING_BUDGET);
}
#endif

extern RING_DESCTIPTOR host_ring[D_NUM_OF_RING_DESCRIPTORS];
extern void (*sysb_recycle_to_feed_cb)(void *datap);
extern void rdp_recycle_buf_to_feed(void *pdata);

static int cpu_rxq_cpu_tx_init(struct bdmf_object *mo)
{
    rdpa_cpu_rxq_cfg_t cpu_tx_high_rxq_cfg = {};
    rdpa_cpu_rxq_cfg_t cpu_tx_low_rxq_cfg = {};
    int rc = 0;

    bdmf_fastlock_init(&cpu_tx_lock[0]);
    bdmf_fastlock_init(&cpu_tx_lock[1]);

    /* CPU TX high Recycle Ring */
    cpu_tx_high_rxq_cfg.size = RDPA_CPU_TX_RING_HIGH_PRIO_SIZE;
    cpu_tx_high_rxq_cfg.type = rdpa_ring_cpu_tx;
    cpu_tx_high_rxq_cfg.isr_priv = TX_HIGH_PRIO_RING_ID;
    cpu_tx_high_rxq_cfg.rx_isr = 0;

    rc = cpu_attr_rxq_cfg_write(mo, NULL, TX_HIGH_PRIO_RING_ID, (void *)&cpu_tx_high_rxq_cfg,
        sizeof(cpu_tx_high_rxq_cfg));
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_NORES, "Recycle ring: can't create\n");
    }

    /* CPU TX low Recycle Ring */
    cpu_tx_low_rxq_cfg.size = RDPA_CPU_TX_RING_LOW_PRIO_SIZE;
    cpu_tx_low_rxq_cfg.type = rdpa_ring_cpu_tx;
    cpu_tx_low_rxq_cfg.isr_priv = TX_LOW_PRIO_RING_ID;
    cpu_tx_low_rxq_cfg.rx_isr = 0;

    rc = cpu_attr_rxq_cfg_write(mo, NULL, TX_LOW_PRIO_RING_ID, (void *)&cpu_tx_low_rxq_cfg,
        sizeof(cpu_tx_low_rxq_cfg));
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_NORES, "Recycle ring: can't create\n");
    }

    /* alocate and init cpu_tx ring indices */
    rdp_cpu_tx_rings_indices_alloc();

    return 0;
}

static int cpu_rxq_feed_and_recycle_rings_init(struct bdmf_object *mo)
{
    rdpa_cpu_rxq_cfg_t feed_rcycle_rxq_cfg = {}, tx_rcycle_rxq_cfg = {};
    rdpa_cpu_rxq_cfg_t feedq_cfg = {};
    int rc = 0;

    /* Initialize the recycle rings */
#ifndef RDP_SIM
    init_waitqueue_head(&recycle_thread_wqh);
    recycle_task_s = kthread_create(&_rdpa_recycle_thread_handler, NULL, "recycle_sysb");
#ifndef CONFIG_BCM96846
    kthread_bind(recycle_task_s,  num_online_cpus() - 1);
#endif

    wake_up_process(recycle_task_s);
#ifndef CONFIG_BCM96846
    set_bit(num_online_cpus() - 1, &feed_rcycle_rxq_cfg.irq_affinity_mask);
    set_bit(num_online_cpus() - 1, &tx_rcycle_rxq_cfg.irq_affinity_mask);
#endif
#endif

    /* For platfoms that have single recycle task, the feed recycle settings will be overwritten by tx recycle settings.
     * This should remain this way as both tasks are same except the fact that tx recycle also turns on timer for
     * coalescing. */

    /* Feed Recycle Ring */
    feed_rcycle_rxq_cfg.size = RECYCLE_RING_SIZE;
    feed_rcycle_rxq_cfg.type = rdpa_ring_recycle;
    feed_rcycle_rxq_cfg.isr_priv = FEED_RCYCLE_RING_ID;
#if !defined(XRDP_EMULATION)
    feed_rcycle_rxq_cfg.rx_isr = _recycle_isr_wrapper;
#else
    feed_rcycle_rxq_cfg.rx_isr = 0;
#endif
    rc = cpu_attr_rxq_cfg_write(mo, NULL, FEED_RCYCLE_RING_ID, (void *)&feed_rcycle_rxq_cfg,
        sizeof(feed_rcycle_rxq_cfg));
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_NORES, "Recycle ring: can't create\n");
    }
#ifndef RDP_SIM
    __rdpa_cpu_int_enable(FEED_RCYCLE_RING_ID);
#endif

    /* TX Recycle Ring */
    tx_rcycle_rxq_cfg.size = RECYCLE_RING_SIZE;
    tx_rcycle_rxq_cfg.type = rdpa_ring_recycle;
    tx_rcycle_rxq_cfg.isr_priv = TX_RCYCLE_RING_ID;
#if !defined(XRDP_EMULATION)
    tx_rcycle_rxq_cfg.rx_isr = _recycle_isr_wrapper;
#else
    tx_rcycle_rxq_cfg.rx_isr = 0;
#endif

    rc = cpu_attr_rxq_cfg_write(mo, NULL, TX_RCYCLE_RING_ID, (void *)&tx_rcycle_rxq_cfg, sizeof(tx_rcycle_rxq_cfg));
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_NORES, "Recycle ring: can't create\n");
    }
#ifndef RDP_SIM
    __rdpa_cpu_int_enable(TX_RCYCLE_RING_ID);
#endif

    /* Initialize FEED ring */
#if !defined(RDP_SIM) && defined(CONFIG_BCM_FEED_RING_DYNAMIC)
    alloc_start = CONFIG_BCM_FEED_RING_ALLOC_START + total_feed_reservation(CONFIG_BCM_FEED_RING_SIZE);
    init_waitqueue_head(&feed_thread_wqh);
    feed_task_s = kthread_create(&_rdpa_feed_thread_handler, NULL, "feed_sysb");
#ifndef CONFIG_BCM96846
    kthread_bind(feed_task_s,  num_online_cpus() - 2);
#endif
    wake_up_process(feed_task_s);
#ifndef CONFIG_BCM96846
    set_bit(num_online_cpus() - 2, &feedq_cfg.irq_affinity_mask);
#else
    bdmf_trace("************** Fix me: SMP to fix\n");
    /*set_bit(num_online_cpus() - 2, &feedq_cfg.irq_affinity_mask);*/
#endif
#endif

#ifdef CONFIG_BCM_FEED_RING_DYNAMIC
    feedq_cfg.size = CONFIG_BCM_FEED_RING_SIZE;
#else /* CONFIG_BCM_FEED_RING_DYNAMIC */
    rc = get_feed_ring_size(&feedq_cfg.size);
    if (rc)
    {
        BDMF_TRACE_RET(rc, "FEED ring: can't create, no mem !\n");
    }
#endif /* CONFIG_BCM_FEED_RING_DYNAMIC */
    feed_ring_low_watermark = FEED_RING_LOW_WATERMARK(feedq_cfg.size);
    feedq_cfg.type = rdpa_ring_feed;
    feedq_cfg.ring_prio = RING_HIGH_PRIO;
    feedq_cfg.isr_priv = 0;
    feedq_cfg.rx_isr = 0;
    rc = cpu_attr_rxq_cfg_write(mo, NULL, FEED_RING_ID, (void *)&feedq_cfg, sizeof(feedq_cfg));
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_NORES, "FEED ring: can't create\n");
    }
#ifndef RDP_SIM
#if 0
    __rdpa_cpu_int_enable(FEED_RING_ID);
#endif
#ifdef XRDP
    sysb_recycle_to_feed_cb = rdp_recycle_buf_to_feed;
#endif
#endif
    return 0;
}

/* "int_connect" attribute "write" callback */
int cpu_attr_int_connect_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc = 0;
    static int infra_rings_inited;

    if (!infra_rings_inited)
    {
        /* Configure feed and recycle rings on first CPU object that is added to the system */
        rc = cpu_rxq_feed_and_recycle_rings_init(mo);
        if (rc)
            return rc;

        /* configure CPU_TX descriptors rings*/
        rc = cpu_rxq_cpu_tx_init(mo);
        if (rc)
            return rc;

        infra_rings_inited = 1;
    }
    /* TODO: invoke bdmf_int_connect */
    return rc;
}

/* "int_enabled" attribute "write" callback */
int cpu_attr_int_enabled_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;

    if (index >= cpu_data->num_queues)
        return BDMF_ERR_INTERNAL;

    if (enable)
        rdpa_cpu_int_enable(cpu_data->index, index);
    else
        rdpa_cpu_int_disable(cpu_data->index, index);
    return 0;
}

/* "int_enabled" attribute "read" callback */
int cpu_attr_int_enabled_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *enable = (bdmf_boolean *)val;
    uint32_t mask = 0;

    if (index >= cpu_data->num_queues)
        return BDMF_ERR_INTERNAL;

    ag_drv_ubus_slv_rnr_intr_ctrl_ier_get(&mask);
    *enable = (mask & (1 << cpu_data->rxq_to_rdd_rxq[index])) != 0;

    return 0;
}

/** Enable CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_enable(rdpa_cpu_port port, int queue)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[port]);

    __rdpa_cpu_int_enable(cpu_data->rxq_to_rdd_rxq[queue]);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_int_enable);
#endif

/** Disable CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_disable(rdpa_cpu_port port, int queue)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[port]);

    __rdpa_cpu_int_disable(cpu_data->rxq_to_rdd_rxq[queue]);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_int_disable);
#endif

/** Clear CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_clear(rdpa_cpu_port port, int queue)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[port]);

    __rdpa_cpu_int_clear(cpu_data->rxq_to_rdd_rxq[queue]);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_int_clear);
#endif

void cpu_tx_free_buffer(pbuf_t *pbuf)
{
    void *buf;

    if (!pbuf->abs_flag)
    {
        /* No chaining for FPM buffers */
        drv_fpm_free_buffer(pbuf->length, pbuf->fpm_bn);
        return;
    }

    if (!bdmf_sysb_is_chained(pbuf->sysb))
    {
        bdmf_sysb_free(pbuf->sysb);
        return;
    }

    buf = pbuf->sysb;
    do
    {
        void *next = bdmf_sysb_chain_next(buf);

        bdmf_sysb_free(buf);
        buf = next;
    } while (buf);
}

/* Map wan_flow + queue_id to channel, rc_id, priority */
#define CPU_MAP_US_INFO_TO_RDD(info, priority, buf, free_func) \
    do { \
        int rc, channel, rc_id;\
        rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(info->port, info->x.wan.flow, \
            info->x.wan.queue_id, (int *)&channel, (int *)&rc_id, \
            (int *)&priority, (int *)&info->drop_precedence);\
        if (rc)\
        {\
            ++cpu_object_data->tx_stat.tx_invalid_queue;\
            free_func(buf);\
            if (cpu_object_data->tx_dump.enable) \
            {\
                BDMF_TRACE_ERR("can't map US flow %u, queue %u to RDD. rc=%d\n", \
                    (unsigned)info->x.wan.flow, (unsigned)info->x.wan.queue_id, rc);\
            } \
            return rc;\
        } \
    } while (0)

/* Map DS channel, queue to RDD */
#define CPU_MAP_DS_INFO_TO_RDD(info, priority, buf, free_func) \
    do { \
        int rc, rc_id; \
        if (rdpa_if_is_cpu_port(info->port)) \
        { \
            priority = 0; \
            return 0; \
        } \
        rc = _rdpa_egress_tm_lan_port_queue_to_rdd_tc_check(info->port, \
            info->x.lan.queue_id, (int *)&rc_id, (int *)&priority, \
            (int *)&info->drop_precedence); \
        if (rc) \
        { \
            ++cpu_object_data->tx_stat.tx_invalid_queue; \
            free_func(buf); \
            BDMF_TRACE_ERR("can't map DS port/queue %u/%u P%u to RDD. rc=%d\n", \
                    (unsigned)info->port, (unsigned)info->x.lan.queue_id, priority, rc);\
            return rc; \
        } \
    } while (0)


static inline void _rdpa_cpu_sysb_flush(void *nbuff, uint8_t *data, int len)
{
#ifndef RDP_SIM
    struct sk_buff *skb;
    uint8_t *dirty, *end;

    if (IS_FKBUFF_PTR(nbuff))
        goto flush_all; /* fkb smart flush is implemented in nbuff_flush */

    skb = PNBUFF_2_SKBUFF(nbuff);
    dirty = skb_shinfo(skb)->dirty_p;

    if (!dirty || dirty < skb->head || dirty > (data + len))
        goto flush_all;

    end = (dirty > data) ? dirty : data;
    bdmf_sysb_inv_headroom_data_flush(nbuff, data, end - data);
    return;

flush_all:
    bdmf_sysb_inv_headroom_data_flush(nbuff, data, len);
#endif
}

static inline void rdpa_cpu_sysb_flush(void *nbuff, uint8_t *data, int len)
{
#ifndef CONFIG_BCM_CACHE_COHERENCY
    _rdpa_cpu_sysb_flush(nbuff, data, len);
#endif
}
static inline void rdpa_cpu_fpm_sysb_flush(void *nbuff, uint8_t *data, int len)
{
#if !defined(CONFIG_BCM_CACHE_COHERENCY) || defined(CONFIG_BCM_FPM_COHERENCY_EXCLUDE)
    _rdpa_cpu_sysb_flush(nbuff, data, len);
#endif
}

#define MIN_PACKET_LENGTH_WITHOUT_CRC  60

/* for speed test */
#if defined(CPU_TX_SPEED_TEST)
int g_test_cpu_send_loop = 0;
int g_debug_wan = 0;
extern int _size;
#endif

/* new CPU_TX_IMPLEMENTATION */
/*int g_dbg_cpu_tx = 1;*/ /* for DBG only should be deleted */
/* cpu_tx_ring debug function: uncomment when required ***************************************
void __dbg_cpu_tx_descriptor_print(const ring_id_t ring_descriptor_idx, uint16_t write_idx, RDD_RING_CPU_TX_DESCRIPTOR_DTS *ring_cpu_tx_descriptor)
{
    bdmf_trace("__dbg_cpu_tx: ring_descriptor_idx %d, write_idx %d\n", ring_descriptor_idx, write_idx);
    bdmf_trace("__dbg_cpu_tx: is_egress %d\n", ring_cpu_tx_descriptor->is_egress);
    bdmf_trace("__dbg_cpu_tx: first_level_q %d\n", ring_cpu_tx_descriptor->first_level_q);
    bdmf_trace("__dbg_cpu_tx: sk_buf_ptr_high 0x%x\n", ring_cpu_tx_descriptor->sk_buf_ptr_high);
    bdmf_trace("__dbg_cpu_tx: sk_buf_ptr_low_or_data_1588 0x%x\n", ring_cpu_tx_descriptor->sk_buf_ptr_low_or_data_1588);
            
    bdmf_trace("__dbg_cpu_tx: color %d\n", ring_cpu_tx_descriptor->color);
    bdmf_trace("__dbg_cpu_tx: do_not_recycle %d\n", ring_cpu_tx_descriptor->do_not_recycle);
    bdmf_trace("__dbg_cpu_tx: flag_1588 %d\n", ring_cpu_tx_descriptor->flag_1588);

    bdmf_trace("__dbg_cpu_tx: lan %d\n", ring_cpu_tx_descriptor->lan);
    bdmf_trace("__dbg_cpu_tx: wan_flow_source_port %d\n", ring_cpu_tx_descriptor->wan_flow_source_port);
    bdmf_trace("__dbg_cpu_tx: fpm_fallback %d\n", ring_cpu_tx_descriptor->fpm_fallback);
    bdmf_trace("__dbg_cpu_tx: sbpm_copy %d\n", ring_cpu_tx_descriptor->sbpm_copy);
    bdmf_trace("__dbg_cpu_tx: target_mem_0 %d\n", ring_cpu_tx_descriptor->target_mem_0);
    bdmf_trace("__dbg_cpu_tx: abs %d\n", ring_cpu_tx_descriptor->abs);
    bdmf_trace("__dbg_cpu_tx: lag_index %d\n", ring_cpu_tx_descriptor->lag_index);
    bdmf_trace("__dbg_cpu_tx: ssid %d\n", ring_cpu_tx_descriptor->ssid);

    bdmf_trace("__dbg_cpu_tx: pkt_buf_ptr_high 0x%x\n", ring_cpu_tx_descriptor->pkt_buf_ptr_high);
    bdmf_trace("__dbg_cpu_tx: pkt_buf_ptr_low_or_fpm_bn0: 0x%x\n", ring_cpu_tx_descriptor->pkt_buf_ptr_low_or_fpm_bn0);
    bdmf_trace("__dbg_cpu_tx: =========================================================\n");
}
******************************************************************************************/

/*******************************************************************************************
*   _rdp_cpu_tx_ring_get_num_of_avail_descriptors - return number of empty slots in ring descriptor
*  Inputs :
*       ring_descriptor_idx - ring index possible values TX_HIGH_PRIO_RING_ID , TX_LOW_PRIO_RING_ID
*       write_idx - current write index
*       read_idx  - current read index
*  Outputs : 
*       empty_slots - number of slots available for writing
********************************************************************************************/
static inline uint16_t _rdp_cpu_tx_ring_get_num_of_avail_descriptors(const ring_id_t ring_descriptor_idx, uint16_t write_idx, uint16_t read_idx)
{
    RING_DESCTIPTOR *cpu_tx_ring_descr = &host_ring[ring_descriptor_idx];
    uint16_t used, available;

    /* we assume that size is power of 2 always */
    used = (write_idx - read_idx) & (cpu_tx_ring_descr->num_of_entries - 1);
    available = cpu_tx_ring_descr->num_of_entries - used;
    
    return available;
}

/******************************************************************************************
 *   __rdp_packet_descriptor_to_cpu_tx_ring - copy CPU_TX descriptor to descriptors ring
 *   Inputs:
 *         ring_descriptor_idx - CPU_TX descriptor idx, possible values TX_HIGH_PRIO_RING_ID , TX_LOW_PRIO_RING_ID
 *         ring_cpu_tx_descriptor - ptr to discriptor to copy to ring. 
 * Output: BDMF_ERR_NO_MORE - ring is full
 *         BDMF_ERR_OK - descriptor is copied. 
 * ***************************************************************************************/
static inline int __rdp_packet_descriptor_to_cpu_tx_ring(const ring_id_t ring_descriptor_idx, RDD_RING_CPU_TX_DESCRIPTOR_DTS *ring_cpu_tx_descriptor,
    int is_exclusive, uint8_t no_lock)
{
    int rc = BDMF_ERR_OK;

    RING_DESCTIPTOR *cpu_tx_ring_descr;
    uint16_t write_idx;
    uint16_t read_idx;
    RDD_RING_CPU_TX_DESCRIPTOR_DTS *cpu_tx_descr_next_ptr = NULL;
    int sram_table_idx;

    if (likely(!no_lock))
    {
        /* We currently assume that there are only two rings for CPU TX: one for high priority traffic and one for low priority; 
         * both rings are defined in the enum sequentially. Hence, it's safe to assume that one index is odd and one is even.
         * If we will extend support for more then 2 CPU TX rings, we should extend the locks array */
        bdmf_fastlock_lock(&cpu_tx_lock[ring_descriptor_idx & 1]);
    }

    cpu_tx_ring_descr = &host_ring[ring_descriptor_idx];
    write_idx = *cpu_tx_ring_descr->write_idx;
    read_idx = *cpu_tx_ring_descr->read_idx;
    sram_table_idx = ring_descriptor_idx == TX_HIGH_PRIO_RING_ID ? 0 : 1;

    /* num of entries should be in power of 2 */
    if (is_exclusive)
    {
        if (((write_idx + 1) & cpu_tx_ring_descr->num_of_entries_mask) == read_idx)
        {
            RDD_CPU_TX_RING_INDICES_READ_IDX_READ_G(*cpu_tx_ring_descr->read_idx, RDD_CPU_TX_RING_INDICES_VALUES_TABLE_ADDRESS_ARR, sram_table_idx);
            if (((write_idx + 1) & cpu_tx_ring_descr->num_of_entries_mask) == *cpu_tx_ring_descr->read_idx)
            {
                rc = BDMF_ERR_NO_MORE;
                goto exit;
            }
        }
    }
    else
    {
        if (_rdp_cpu_tx_ring_get_num_of_avail_descriptors(ring_descriptor_idx, write_idx, read_idx) < RDPA_CPU_TX_RING_EXLUSIVE_RESERVED_SIZE)
        {
            RDD_CPU_TX_RING_INDICES_READ_IDX_READ_G(*cpu_tx_ring_descr->read_idx, RDD_CPU_TX_RING_INDICES_VALUES_TABLE_ADDRESS_ARR, sram_table_idx);
            if (_rdp_cpu_tx_ring_get_num_of_avail_descriptors(ring_descriptor_idx, write_idx, *cpu_tx_ring_descr->read_idx) < RDPA_CPU_TX_RING_EXLUSIVE_RESERVED_SIZE)
            {
                rc = BDMF_ERR_NO_MORE;
                goto exit;
            }
        }
    }


    write_idx = *cpu_tx_ring_descr->write_idx;
    cpu_tx_descr_next_ptr = &((RDD_RING_CPU_TX_DESCRIPTOR_DTS *)cpu_tx_ring_descr->base)[write_idx];
    /*if (g_dbg_cpu_tx)
     *{
     *    bdmf_trace("__dbg_cpu_tx cpu_tx_descr_next_ptr %p, base %p, size %d\n", cpu_tx_descr_next_ptr, cpu_tx_ring_descr->base, (int)sizeof(RDD_RING_CPU_TX_DESCRIPTOR_DTS));
     *    __dbg_cpu_tx_descriptor_print(ring_descriptor_idx, write_idx, ring_cpu_tx_descriptor);
     *}*/

     /* copy in assembler section */
    {
#ifdef CONFIG_ARM64
#ifndef RDP_SIM
        register uint64_t dword0 asm ("x8");
        register uint64_t dword1 asm ("x9");
        __asm__("ldp   %1, %2,[%0]" \
        :  "=r" (ring_cpu_tx_descriptor), "=r" (dword0), "=r" (dword1) \
        : "0" (ring_cpu_tx_descriptor));

        ((uint64_t *)cpu_tx_descr_next_ptr)[0] = swap4bytes64(dword0);
        ((uint64_t *)cpu_tx_descr_next_ptr)[1] = swap4bytes64(dword1);
#else
        ((uint64_t *)cpu_tx_descr_next_ptr)[0] = swap4bytes64(((uint64_t *)ring_cpu_tx_descriptor)[0]);
        ((uint64_t *)cpu_tx_descr_next_ptr)[1] = swap4bytes64(((uint64_t *)ring_cpu_tx_descriptor)[1]);
#endif
#else
        ((uint32_t *)cpu_tx_descr_next_ptr)[0] = swap4bytes(((uint32_t *)ring_cpu_tx_descriptor)[0]);
        ((uint32_t *)cpu_tx_descr_next_ptr)[1] = swap4bytes(((uint32_t *)ring_cpu_tx_descriptor)[1]);
        ((uint32_t *)cpu_tx_descr_next_ptr)[2] = swap4bytes(((uint32_t *)ring_cpu_tx_descriptor)[2]);
        ((uint32_t *)cpu_tx_descr_next_ptr)[3] = swap4bytes(((uint32_t *)ring_cpu_tx_descriptor)[3]);
#endif
    }
#ifndef RDP_SIM 
    bdmf_dcache_flush((unsigned long)cpu_tx_descr_next_ptr, sizeof(RDD_RING_CPU_TX_DESCRIPTOR_DTS));
#endif
    *cpu_tx_ring_descr->write_idx = (write_idx + 1) & cpu_tx_ring_descr->num_of_entries_mask; 
#ifndef RDP_SIM      
    bdmf_dcache_flush((unsigned long)cpu_tx_ring_descr->write_idx, sizeof(uint16_t));
#endif
   
#ifdef CONFIG_BCM_CACHE_COHERENCY
    /* Before accessing the descriptors in FW must do barrier */
    dma_wmb();
#endif
    RDD_CPU_TX_RING_INDICES_WRITE_IDX_WRITE_G((*cpu_tx_ring_descr->write_idx), RDD_CPU_TX_RING_INDICES_VALUES_TABLE_ADDRESS_ARR, sram_table_idx);

exit:
    if (likely(!no_lock))
        bdmf_fastlock_unlock(&cpu_tx_lock[ring_descriptor_idx & 1]);

    return rc;
}

/*******************************************************************************************************
* _rdp_wakeup_cpu_tx_tasks - wake up CPU_TX tasks
*    in case of chips 6846, 6878 it wake up only CPU_TX_0 task
*    for other chips wake up CPU_TX_0 and CPU_TX_1 tasks
********************************************************************************************************/
static inline void _rdp_wakeup_cpu_tx_tasks(void)
{
    static int task_id = CPU_TX_0_THREAD_NUMBER;

#ifndef RDP_SIM   
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(cpu_tx_runner_image), task_id);
#else /* RDP_SIM */
    task_id = CPU_TX_0_THREAD_NUMBER;
    rdp_cpu_runner_wakeup(get_runner_idx(cpu_tx_runner_image), task_id);
#endif /* RDP_SIM */
#if !defined(CPU_TX_SINGLE_TASK)
    /* flip-flop to wake up next time the other cpu_tx task */
    task_id = task_id == CPU_TX_0_THREAD_NUMBER ? (CPU_TX_0_THREAD_NUMBER + 1) : CPU_TX_0_THREAD_NUMBER;
#endif
}

#if defined(CPU_TX_SPEED_TEST)
/*******************************************************************************************************
* _rdp_cpu_tx_run_speed_test - test function for cpu_tx rate estimation
*   send skb packet in loop as defined by g_test_cpu_send_loop counter 
********************************************************************************************************/
static void _rdp_cpu_tx_run_speed_test(pbuf_t *pbuf, const rdpa_cpu_tx_info_t *info, ring_id_t ring_descriptor_idx,
    RDD_RING_CPU_TX_DESCRIPTOR_DTS *ring_cpu_tx_descriptor, int exclusive, uint32_t *pkts_sent)
{
    if (g_debug_wan)
    {
        bdmf_trace("DBG wan_flow %d, queue_id %d\n", info->x.wan.flow, info->x.wan.queue_id);
    }
    if (pbuf->abs_flag && info->method == rdpa_cpu_tx_egress && _size == pbuf->length && g_test_cpu_send_loop)
    {
#if defined KERNEL_64
        uint64_t loop_counter = 0;
        uint64_t falures_num = 0;
        uint64_t time_delta_usec = 0;
        uint64_t time_start, time_end;
#else
        long loop_counter = 0;
        long falures_num = 0;
        long time_delta_usec = 0;
        long time_start, time_end;
#endif
        int rate = 0;
        int rate_bytes = 0;
        int rdd_rc;

        ring_cpu_tx_descriptor->do_not_recycle = 1;
        bdmf_trace("g_test_cpu_send_loop %d, %p\n", g_test_cpu_send_loop, &g_test_cpu_send_loop);
        time_start = bdmf_time_since_epoch_usec();
        while (loop_counter < g_test_cpu_send_loop)
        {
            rdd_rc = __rdp_packet_descriptor_to_cpu_tx_ring(ring_descriptor_idx, ring_cpu_tx_descriptor, exclusive, info->bits.no_lock);
            loop_counter++;
            *pkts_sent = *pkts_sent + 1;
            if (rdd_rc)
                falures_num++;
            if (loop_counter & 7) /* wake up cpu tx task once per 8 packets */
            {
                _rdp_wakeup_cpu_tx_tasks();
            }
        }
        time_end = bdmf_time_since_epoch_usec();
        ring_cpu_tx_descriptor->do_not_recycle = 0;

        /* calculate time_delta */
        time_delta_usec =  time_end - time_start;
        bdmf_trace("Errors %ld, start %ld, end %ld delta %ld, length %d\n", falures_num, time_start, time_end, time_delta_usec, pbuf->length);
        
        /*calculate rate*/
        rate = (g_test_cpu_send_loop - falures_num)*1000/(time_delta_usec/1000);
        rate_bytes = rate * pbuf->length; 
        bdmf_trace("Rate %d, rate_bytes %d\n", rate, rate_bytes);
    }
    else if (_size == pbuf->length)
    {
        bdmf_trace("g_test_cpu_send_loop %d, %p\n", g_test_cpu_send_loop, &g_test_cpu_send_loop);
        bdmf_trace("g_debug_wan %d, %p\n", g_debug_wan, &g_debug_wan);
    }
}
#endif

/********************************************************************************
 * rdpa_cpu_send_pbuf - new implementation send CPU_TX packets bydescriptor  ring 
 * input:
 *       pbuf  - packet buffer structure
 *       info  - deistanation information + flags
 * 
 * ******************************************************************************/
static inline int rdpa_cpu_send_pbuf(pbuf_t *pbuf, const rdpa_cpu_tx_info_t *info)
{
    bdmf_error_t rdd_rc = 0;
    int queue = 0;
    RDD_RING_CPU_TX_DESCRIPTOR_DTS ring_cpu_tx_descriptor = {};
    cpu_drv_priv_t *cpu_object_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[info->cpu_port]);
    uint32_t pkts_sent = 0;
    ring_id_t ring_descriptor_idx; /* possible values: TX_HIGH_PRIO_RING_ID , TX_LOW_PRIO_RING_ID */ 
    int exclusive = 0;

#if !defined(BCM63158)
    cpu_tx_wan_flow_source_port wan_flow_source_port = {};
#endif
    void *buf = NULL;
    int is_chained = 0;

    /* Dump tx data for debugging */
#ifndef XRDP_EMULATION
    CPU_CHECK_DUMP_PBUF(pbuf, info);
#endif

    if (cpu_tx_disable)
        return BDMF_ERR_STATE;
    
    if (pbuf->abs_flag)
    {
        pbuf->sbpm_copy = 1;
        pbuf->fpm_fallback = 0;
        is_chained = bdmf_sysb_is_chained(pbuf->sysb);
        rdpa_cpu_sysb_flush(pbuf->sysb, pbuf->data, pbuf->length);
    }

    /* packet padding - done after sysb_flush to prevent unnecessary extra length flush */
    if (!rdpa_if_is_wan(info->port) || (rdpa_wan_if_to_wan_type(info->port) == rdpa_wan_gbe))
    {
        /* temporary avoid padding non-gbe wan to avoid padding control messages (corrupts CRC) */
        pbuf->length = pbuf->length > MIN_PACKET_LENGTH_WITHOUT_CRC ? pbuf->length : MIN_PACKET_LENGTH_WITHOUT_CRC;
    }
      
    switch (info->method)
    {
        case rdpa_cpu_tx_egress: /**< Egress port and priority are specified explicitly. This is the most common mode */
        {
            if (rdpa_if_is_wan(info->port))
            {
                /* upstream, egress enqueue, bpm */
                /* Map wan_flow to queue_id */
                CPU_MAP_US_INFO_TO_RDD(info, queue, pbuf, cpu_tx_free_buffer);
#if !defined(BCM63158) && defined(CONFIG_MULTI_WAN_SUPPORT)
                if ((rdpa_wan_if_to_wan_type(info->port) == rdpa_wan_gbe))
                {
                    wan_flow_source_port.flow_or_port_id = rdpa_port_rdpa_if_to_vport(info->port);
                    wan_flow_source_port.is_vport = 1;
                    ring_cpu_tx_descriptor.wan_flow_source_port = wan_flow_source_port.wan_flow_source_port;
                }
                else
                     ring_cpu_tx_descriptor.wan_flow_source_port = (int)info->x.wan.flow;
#else
                ring_cpu_tx_descriptor.wan_flow_source_port = (int)info->x.wan.flow;
#endif
                if (pbuf->abs_flag)
                    pbuf->sbpm_copy = 0;
            }
            else
            {
                /* downstream, egress enqueue, bpm */
                CPU_MAP_DS_INFO_TO_RDD(info, queue, pbuf, cpu_tx_free_buffer);
                /* to_lan bit */
                ring_cpu_tx_descriptor.lan = 1;
#if !defined(BCM63158)
                wan_flow_source_port.flow_or_port_id = rdpa_port_rdpa_if_to_vport(info->port);
                wan_flow_source_port.is_vport = 1;
                ring_cpu_tx_descriptor.wan_flow_source_port = wan_flow_source_port.wan_flow_source_port;
#else
                ring_cpu_tx_descriptor.wan_flow_source_port = rdpa_port_rdpa_if_to_vport(info->port);
#endif
                if (rdpa_if_is_lan(info->port))
                    ring_cpu_tx_descriptor.lag_index = info->lag_index;
                else
                    ring_cpu_tx_descriptor.ssid = info->ssid;
                if (pbuf->abs_flag)
                    pbuf->fpm_fallback = 1;
            }
            ring_cpu_tx_descriptor.first_level_q = queue;
            ring_cpu_tx_descriptor.is_egress = 1;
            ring_descriptor_idx = TX_HIGH_PRIO_RING_ID;
            break;
        }

        case rdpa_cpu_tx_ingress: /**< before bridge forwarding decision, before classification */
        {
            if (!rdpa_if_is_wan(info->port) || (rdpa_wan_if_to_wan_type(info->port) == rdpa_wan_gbe))
            {
                /* From LAN or GBE, full, bpm. Traffic from GBE WAN should simulate same behavior as it comes from BBH_RX. */
                ring_cpu_tx_descriptor.lan = 1;
                ring_cpu_tx_descriptor.wan_flow_source_port = rdpa_port_rx_flow_src_port_get(info->port, 0);
                if (!rdpa_if_is_wan(info->port))
                    ring_cpu_tx_descriptor.ssid = info->ssid;
            }
            else
            {
                /* From WAN, full, bpm */
                ring_cpu_tx_descriptor.lan = 0;
                ring_cpu_tx_descriptor.wan_flow_source_port = info->x.wan.flow;
            }
            ring_cpu_tx_descriptor.is_egress = 0;
            ring_descriptor_idx = TX_LOW_PRIO_RING_ID;
            break;
        }

        default:
        cpu_tx_free_buffer(pbuf);
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "transmit method %d\n", (int)info->method);
    }

    ring_cpu_tx_descriptor.color = !(info->drop_precedence); /* 0 = yellow, 1 = green. if drop_precedence is true so color is yellow */
    rdd_cpu_tx_set_ring_descriptor(info, (void *)pbuf, &ring_cpu_tx_descriptor);

#ifdef CONFIG_BCM_PTP_1588
    if (ring_cpu_tx_descriptor.flag_1588)
        exclusive = 1;
#endif

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE) || defined(CONFIG_BCM_SPDTEST) || defined(CONFIG_BCM_SPDTEST_MODULE)) 
    if (info->bits.is_spdsvc_setup_packet && !rdpa_if_is_wifi(info->port))
    {
        RDD_CPU_TX_DESCRIPTOR_DTS cpu_tx_descriptor = {};

        cpu_tx_descriptor.lan = ring_cpu_tx_descriptor.lan;
        cpu_tx_descriptor.first_level_q = ring_cpu_tx_descriptor.first_level_q;
        cpu_tx_descriptor.wan_flow_source_port = ring_cpu_tx_descriptor.wan_flow_source_port;

#if (defined(CONFIG_BCM_SPDTEST) || defined(CONFIG_BCM_SPDTEST_MODULE))
        if (info->spdt_so_mark)
        {
            ring_cpu_tx_descriptor.egress_dont_drop = 1;
            if ((info->spdt_so_mark  & RDPA_UDPSPDTEST_SO_MARK_PREFIX) == RDPA_UDPSPDTEST_SO_MARK_PREFIX)
                return udpspdtest_tx_start(pbuf, info, &cpu_tx_descriptor);
        }
#endif
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        if (!info->spdt_so_mark)
            return rdd_spdsvc_gen_start(pbuf, info, &cpu_tx_descriptor);
#endif
    }
#endif

    if (pbuf->abs_flag && is_chained)
    {
        buf = bdmf_sysb_chain_next(pbuf->sysb);
        /* unlink packet from chain before send */
        bdmf_sysb_chained_unlink(pbuf->sysb);
    }
#if defined(CPU_TX_SPEED_TEST)
    _rdp_cpu_tx_run_speed_test(pbuf, info, ring_descriptor_idx, &ring_cpu_tx_descriptor, exclusive, &pkts_sent);
#endif
    rdd_rc = __rdp_packet_descriptor_to_cpu_tx_ring(ring_descriptor_idx, &ring_cpu_tx_descriptor, exclusive, info->bits.no_lock);
    pkts_sent++;

    if (pbuf->abs_flag && is_chained && !rdd_rc)
    {
        void *bufnext;

        /* handle chain of sysb */
        while (buf)
        {
            pbuf->sysb = buf;
            pbuf->data = bdmf_sysb_data(buf);
            pbuf->length = bdmf_sysb_length(buf);      
            bufnext = bdmf_sysb_chain_next(buf);
            /* unlink packet from chain before send */
            bdmf_sysb_chained_unlink(buf);
            
            rdpa_cpu_sysb_flush(pbuf->sysb, pbuf->data, pbuf->length);
            
            /* packet padding - done after sysb_flush to prevent unnecessary extra length flush */
            if (!rdpa_if_is_wan(info->port) || (rdpa_wan_if_to_wan_type(info->port) == rdpa_wan_gbe))
            {
                /* temporary avoid padding non-gbe wan to avoid padding control messages (corrupts CRC) */
                pbuf->length = pbuf->length > MIN_PACKET_LENGTH_WITHOUT_CRC ? pbuf->length : MIN_PACKET_LENGTH_WITHOUT_CRC;
            }
    
            rdd_cpu_tx_set_ring_descriptor(info, (void *)pbuf, &ring_cpu_tx_descriptor);
            rdd_rc = __rdp_packet_descriptor_to_cpu_tx_ring(ring_descriptor_idx, &ring_cpu_tx_descriptor, exclusive, info->bits.no_lock);
            if (rdd_rc)
                break;

            if (pkts_sent & CPU_TX_SEND_RATE_FOR_CHAINED) /* wake up cpu tx task once per 128 packets */
            {
                _rdp_wakeup_cpu_tx_tasks();
#if !defined(CPU_TX_SINGLE_TASK)
                _rdp_wakeup_cpu_tx_tasks(); /* to wake up second cpu_tx task */
#endif
            }
            buf = bufnext;
            pkts_sent++;
        }
    }
    _rdp_wakeup_cpu_tx_tasks();

#ifndef XRDP_EMULATION
    CPU_CHECK_DUMP_RDD_RC("pbuf", rdd_rc);
#endif
    if (rdd_rc)
    {
        cpu_tx_free_buffer(pbuf);
        ++cpu_object_data->tx_stat.tx_rdd_error;
        /* BDMF_TRACE_RET(BDMF_ERR_IO, "rdd error %d\n", (int)rdd_rc); */
        return BDMF_ERR_IO;
    }
#ifndef XRDP_EMULATION
    cpu_object_data->tx_stat.tx_ok += pkts_sent;
#endif
    return 0;
}

/** Send raw packet,
 * raw packets are assumed to be control and therefore get exclusive priority on packet buffer allocation
 * mean we ignore low FPM Xoff threshold at the moment
 * if this is not the case, need to pass is_control_packet (already in code for reference)
 *
 * \param[in]   data        Packet data
 * \param[in]   length      Packet length
 * \param[in]   info        Additional transmit info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_raw(void *data, uint32_t length, const rdpa_cpu_tx_info_t *info)
{
    int rc, extra_headroom = 0;
    pbuf_t pbuf;
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();
    fpm_pool_stat fpm_stat;
    uint16_t xon_thr, xoff_thr;
    int is_control_packet = 1;

    if (length > system_cfg->mtu_size - 4)
        return BDMF_ERR_OVERFLOW;

    if (info->method == rdpa_cpu_tx_ingress)
        extra_headroom = RDD_PACKET_HEADROOM_OFFSET;

    if (!is_control_packet)
    {
        rc = ag_drv_fpm_pool_stat_get(&fpm_stat);
        rc = rc ? rc : ag_drv_fpm_pool1_xon_xoff_cfg_get(&xon_thr, &xoff_thr);
        if (rc || (fpm_stat.num_of_tokens_available <= xoff_thr))
            return BDMF_ERR_NORES;
    }
    pbuf.length = length;
    /* When sending FPM to runner processing (cpu_tx_ingress), we should leave enough headroom for addition packet
     * modification (e.g. adding vlan header). */
    pbuf.offset = info->data_offset + extra_headroom;
    pbuf.data = data;
    pbuf.abs_flag = 0;
    pbuf.sysb = NULL;

    rc = drv_fpm_alloc_buffer(pbuf.length + extra_headroom, &(pbuf.fpm_bn));
    if (rc)
        return rc;

    drv_fpm_copy_from_host_buffer(pbuf.data, pbuf.fpm_bn & 0xffff, pbuf.length, pbuf.offset);
    return rdpa_cpu_send_pbuf(&pbuf, info);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_raw);
#endif

/** Send system buffer
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] info Tx info
 * \return 0=OK or int error code\n
 */

int rdpa_cpu_send_sysb(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    pbuf_t pbuf = {};

    pbuf.length = bdmf_sysb_length(sysb);
    pbuf.offset = 0;
    pbuf.data = bdmf_sysb_data(sysb);
    pbuf.abs_flag = 1;
    pbuf.sysb = sysb;
    return rdpa_cpu_send_pbuf(&pbuf, info);
}

#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_sysb);
#endif

/** Send system buffer allocated from FPM
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] info Tx info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_sysb_fpm(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    pbuf_t pbuf = {};

    pbuf.length = bdmf_sysb_length(sysb);
    pbuf.offset = info->data_offset;
    pbuf.data = bdmf_sysb_data(sysb);
    pbuf.abs_flag = 0;
    pbuf.sysb = NULL;
    pbuf.fpm_bn = bdmf_sysb_fpm_num(sysb);

    rdpa_cpu_fpm_sysb_flush(sysb, pbuf.data, pbuf.length);

    return rdpa_cpu_send_pbuf(&pbuf, info);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_sysb_fpm);
#endif

/** Send chained system buffer from WFD
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] info Tx info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_wfd_to_bridge(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *_info, size_t offset_next)
{
    /* TODO: implement */
    return -1;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_wfd_to_bridge);
#endif


/* Send system buffer ptp - similar to rdpa_cpu_send_sysb, but treats only
 * ptp-1588 packets */
int rdpa_cpu_send_sysb_ptp(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    /* TODO */
    return 0;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_sysb_ptp);
#endif

/** Send system buffer - Special function to send EPON Dying
 *  Gasp:
 *  1. reduce "if"
 *  2. use fastlock_irq
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] info Tx info
 * \return 0=OK or int error code\n
 *
 * TBD: Move this function to rdpa_epon.c !
 *  */
int rdpa_cpu_send_epon_dying_gasp(bdmf_sysb sysb,
    const rdpa_cpu_tx_info_t *info)
{
    /* TODO */
    return 0;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_epon_dying_gasp);
#endif

static int _cpu_tc_to_rxq_set(struct bdmf_object *mo, uint8_t tc, uint8_t rxq)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);

/*    bdmf_trace("%s: ============= TC %d, RQX %d (unassidned %d)\n", __FUNCTION__, tc, rxq,
        (uint8_t)BDMF_INDEX_UNASSIGNED);*/
    if ((rxq != (uint8_t)BDMF_INDEX_UNASSIGNED && !cpu_data->rxq_cfg[rxq].size) ||
        rxq == FEED_RING_ID || rxq == FEED_RCYCLE_RING_ID || rxq == TX_RCYCLE_RING_ID)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PERM, mo, "Cannot set TC %d to RXQ %d: queue is not configured\n", tc, rxq);
    }

    cpu_data->tc_to_rxq[tc] = rxq;
    rdd_cpu_tc_to_rxq_set(cpu_data->index, tc,
        rxq == (uint8_t)BDMF_INDEX_UNASSIGNED ? rxq : cpu_data->rxq_to_rdd_rxq[rxq]);
    return 0;
}

int cpu_post_init_ex(struct bdmf_object *mo)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    int i, rc;

    /* Allocate free queues from the pool */
    rc = rdd_rxq_map_alloc(cpu_data);
    if (rc)
        return rc;

    for (i = 0; i < RDPA_CPU_TC_NUM; i++)
    {
        rc = _cpu_tc_to_rxq_set(mo, i, cpu_data->tc_to_rxq[i]);
        if (rc)
            return rc;
    }
    return 0;
}

int cpu_drv_init_ex(struct bdmf_type *drv)
{
    rdd_rxq_map_reset();
#ifdef BDMF_DRIVER_GPL_LAYER
#if defined(BCM63158)
    f_rdpa_cpu_tx_port_enet_or_dsl_wan = rdpa_cpu_tx_port_enet_or_dsl_wan;
#endif
#endif
    return 0;
}

void cpu_drv_exit_ex(struct bdmf_type *drv)
{
#ifndef RDP_SIM
    if (recycle_task_s)
       kthread_stop(recycle_task_s);
    if (feed_task_s)
       kthread_stop(feed_task_s);
#endif
#ifdef BDMF_DRIVER_GPL_LAYER
#if defined(BCM63158)
    f_rdpa_cpu_tx_port_enet_or_dsl_wan = NULL;
#endif
#endif
    rdd_rxq_map_reset();
}

int cpu_attr_tc_to_rxq_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t *rxq = (uint8_t *)val;

    *rxq = cpu_data->tc_to_rxq[index];
    return 0;
}

int cpu_attr_tc_to_rxq_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    uint8_t rxq = *(uint8_t *)val;

    _cpu_tc_to_rxq_set(mo, (uint8_t)index, rxq);
    return 0;
}

int cpu_reason_cfg_validate_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    /* No special validation is required */
    return 0;
}

int cpu_reason_cfg_rdd_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    rdpa_cpu_reason_index_t *rindex = (rdpa_cpu_reason_index_t *)index;
    const rdpa_cpu_reason_cfg_t *reason_cfg = (const rdpa_cpu_reason_cfg_t *)val;

    if (rindex->dir == rdpa_dir_us && reason_cfg->meter_ports)
        return cpu_per_port_reason_meter_cfg(mo, rindex, reason_cfg);
    return cpu_meter_cfg_rdd(mo, rindex, reason_cfg->meter, 0);
}

int rdpa_cpu_loopback_packet_get(rdpa_cpu_loopback_type loopback_type, bdmf_index queue, bdmf_sysb *sysb,
    rdpa_cpu_rx_info_t *info)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

#ifdef RDP_SIM
#define _DUMP_PACKET_EX_GEN_FMT "%s: SRC_PORT-%s QUEUE-%d PACKET_LENGTH-%d COLOR-%d"
#define _DUMP_PACKET_EX_SSID_FMT "SSID-%d"
#define _DUMP_PACKET_EX_REASON_FMT "REASON-%s"
#define _DUMP_PACKET_EX_METADATA_FMT "WL_METADATA-0x%x"
#define _DUMP_PACKET_EX_REDIRECT_FMT "Egress object={%s}, egress queue id=%u, %s wan_flow=%u"
#else
#define _DUMP_PACKET_EX_GEN_FMT "Rx packet on %s: port %s queue %d, %d bytes, color %d,"
#define _DUMP_PACKET_EX_SSID_FMT "CPU Vport ext / SSID %d,"
#define _DUMP_PACKET_EX_REASON_FMT "reason '%s'"
#define _DUMP_PACKET_EX_METADATA_FMT "Metadata 0x%x,"
#define _DUMP_PACKET_EX_REDIRECT_FMT "Egress object={%s}, egress queue id=%u, %s wan_flow=%u"
#endif
void _dump_packet_ex(char *name, rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, uint32_t dst_ssid, rdpa_cpu_rx_ext_info_t *ext_info)
{
#ifdef RDP_SIM
    bdmf_session_handle session = g_cpu_rx_file_session;
#else
    bdmf_session_handle session = NULL;
#endif
    bdmf_session_print(session, _DUMP_PACKET_EX_GEN_FMT " ",
        name, bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->src_port), (int)queue, info->size, info->color);
    if (rdpa_if_is_cpu_port(info->src_port) && (info->is_exception || info->is_ucast))
        bdmf_session_print(session, _DUMP_PACKET_EX_SSID_FMT " ", info->dest_ssid);
    if (info->is_exception)
    {
        bdmf_session_print(session, _DUMP_PACKET_EX_REASON_FMT "\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_cpu_reason_enum_table, info->reason));
    }
    else /* Forwarding to CPU port */
    {
        int ssid_bit;
        bdmf_session_print(session, _DUMP_PACKET_EX_METADATA_FMT " ", info->wl_metadata);
        if (!info->is_ucast)
        {
            bdmf_session_print(session, "Multicast forwarding: ");
            while (dst_ssid)
            {
                ssid_bit = ffs(dst_ssid);
                bdmf_session_print(session, "%s",
                    bdmf_attr_get_enum_text_hlp(&rdpa_wlan_ssid_enum_table,
                    ssid_bit - 1 + rdpa_wlan_ssid0));
                dst_ssid &= ~(1 << (ssid_bit - 1));
                if (dst_ssid)
                    bdmf_session_print(session, "+");
            }
        }
        bdmf_session_print(session, "\n");
    }
#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
    if (ext_info)
    {
        /* Dump additional data first */
        bdmf_session_print(session, _DUMP_PACKET_EX_REDIRECT_FMT "\n",
            ext_info->egress_object ? ext_info->egress_object->name : "Unknown", ext_info->egress_queue_id,
            rdpa_if_is_wan(info->src_port) ? "ingress" : "egress", ext_info->wan_flow);
    }
#endif
    bdmf_session_hexdump(session, (void *)((uint8_t *)info->data + info->data_offset), 0, info->size);
}

void _dump_packet(char *name, rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info,
    uint32_t dst_ssid)
{
    _dump_packet_ex(name, port, queue, info, dst_ssid, NULL);
}

static inline int rdpa_cpu_rx_params_parse(cpu_drv_priv_t *cpu_data, rdpa_cpu_port port, bdmf_index queue, CPU_RX_PARAMS *params, rdpa_cpu_rx_info_t *info)
{
    uint32_t context = 0;
    rdpa_traffic_dir dir;

    info->reason = (rdpa_cpu_reason)params->reason;
    info->src_port = rdpa_port_vport_to_rdpa_if(params->src_bridge_port);
    if (info->reason != rdpa_cpu_rx_reason_oam && info->reason != rdpa_cpu_rx_reason_omci)
    {
        if (info->src_port == rdpa_if_none)
        {
            cpu_data->rxq_stat[queue].dropped++;
            bdmf_sysb_databuf_free(params->data_ptr, context);
            return BDMF_ERR_PERM;
        }
    }

    info->is_exception = params->is_exception;
    info->wl_metadata = params->wl_metadata;
#if defined(BCM63158)
    /* RNR relocates cpu_rx_FWD packet's 8bit wan_flow_id into metadata_1 */
    if (!info->is_exception)
        info->reason_data = params->wl_metadata & 0xFF;
    else
#endif
        info->reason_data = params->flow_id;

    info->dest_ssid = params->dst_ssid;
    info->ptp_index = params->ptp_index;
    info->data = (void *)params->data_ptr;
    info->data_offset = params->data_offset;
    info->size = params->packet_size;
    info->is_rx_offload = params->is_rx_offload;
    info->is_ucast = params->is_ucast;
    info->mcast_tx_prio = params->mcast_tx_prio;
#if defined(CONFIG_RUNNER_CSO)
    info->rx_csum_verified = params->is_csum_verified;
#else
    info->color = params->color;
#endif
    info->omci_encrypted_key_index = params->omci_enc_key_index;

    dir = rdpa_if_is_wan(info->src_port) ? rdpa_dir_ds : rdpa_dir_us;
    if (info->is_exception)
        ++cpu_data->reason_stat[dir][info->reason];
    cpu_data->rxq_stat[queue].received++;
    if (unlikely(cpu_data->rxq_cfg[queue].dump))
    	_dump_packet(cpu_object[port]->name, port, queue, info, info->dest_ssid);

    return 0;
}

#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
static inline int rdpa_cpu_rx_params_parse_redirected(cpu_drv_priv_t *cpu_data, rdpa_cpu_port port, bdmf_index queue,
    CPU_RX_PARAMS *params, rdpa_cpu_rx_info_t *info, rdpa_cpu_rx_ext_info_t *ext_info)
{
    int rc;
    int dump = cpu_data->rxq_cfg[queue].dump;

    /* Disable dump temporarily if redirected packet. It will be dumped in slightly
     * different format
     */
    if ((rdpa_cpu_reason)params->reason == rdpa_cpu_rx_reason_cpu_redirect)
        cpu_data->rxq_cfg[queue].dump = 0;

    rc = rdpa_cpu_rx_params_parse(cpu_data, rdpa_cpu_host, queue, params, info);
    if (rc)
        return rc;

    /* Parse extended info if trap reason is REDIRECT */
    ext_info->valid = 0;
    if (info->reason == rdpa_cpu_rx_reason_cpu_redirect)
    {
        ext_info->egress_object = NULL;
        ext_info->egress_queue_id = (uint32_t)BDMF_INDEX_UNASSIGNED;
        rc = rdpa_rdd_tx_queue_info_get(params->cpu_redirect_egress_queue, &ext_info->egress_object,
            &ext_info->egress_queue_id);
        if (rc)
            return rc;
        ext_info->wan_flow = params->cpu_redirect_wan_flow;
        ext_info->valid = 1;
        if (unlikely(dump))
        {
            _dump_packet_ex(cpu_object[port]->name, port, queue, info, info->dest_ssid, ext_info);
        }
        cpu_data->rxq_cfg[queue].dump = dump;
    }
    return rc;
}
#endif

int rdpa_cpu_packets_bulk_get(rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info, int max_count, int *count)
{
    cpu_drv_priv_t *cpu_data;
    CPU_RX_PARAMS params[CPU_RX_PACKETS_BULK_SIZE] = {};
    int rc, i, parsed_count;

    if ((unsigned)port >= rdpa_cpu_port__num_of || !cpu_object[port])
        return BDMF_ERR_PARM;

    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[port]);

    if (_check_queue_range(cpu_data, (unsigned)queue))
        return BDMF_ERR_RANGE;

    rc = rdp_cpu_ring_read_bulk(cpu_data->rxq_to_rdd_rxq[queue], params, max_count, count);
    for (parsed_count = 0, i = 0; i < *count; i++)
    {
        if (!rdpa_cpu_rx_params_parse(cpu_data, port, queue, &params[i], info))
        {
            parsed_count++;
            info++;
        }
    }

    *count = parsed_count;
    return rc;
}
#if !defined(BDMF_DRIVER_GPL_LAYER) && !defined(RUNNER_CPU_DQM_RX)
EXPORT_SYMBOL(rdpa_cpu_packets_bulk_get);
#endif

int rdpa_cpu_packet_get(rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info)
{
    cpu_drv_priv_t *cpu_data;
    CPU_RX_PARAMS params = {};
    int rc;

    if ((unsigned)port >= rdpa_cpu_port__num_of || !cpu_object[port])
        return BDMF_ERR_PARM;

    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[port]);

    if (_check_queue_range(cpu_data, (unsigned)queue))
        return BDMF_ERR_RANGE;

    rc = rdp_cpu_ring_read_packet_refill(cpu_data->rxq_to_rdd_rxq[queue], &params);
    if (rc)
    {
        if (rc == BDMF_ERR_NO_MORE)
            return rc;
        return BDMF_ERR_INTERNAL;
    }

    return rdpa_cpu_rx_params_parse(cpu_data, port, queue, &params, info);
}

int rdpa_cpu_packet_get_redirected(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, rdpa_cpu_rx_ext_info_t *ext_info)
{
#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
    cpu_drv_priv_t *cpu_data;
    CPU_RX_PARAMS params = {};
    int rc;

    if (!cpu_object[rdpa_cpu_host])
        return BDMF_ERR_PARM;

    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[port]);

    if (_check_queue_range(cpu_data, (unsigned)queue))
        return BDMF_ERR_RANGE;

    rc = rdp_cpu_ring_read_packet_refill(cpu_data->rxq_to_rdd_rxq[queue], &params);
    if (rc)
    {
        if (rc == BDMF_ERR_NO_MORE)
            return rc;
        return BDMF_ERR_INTERNAL;
    }

    return rdpa_cpu_rx_params_parse_redirected(cpu_data, port, queue, &params, info, ext_info);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

void cpu_rxq_cfg_params_init_ex(cpu_drv_priv_t *cpu_data, rdpa_cpu_rxq_cfg_t *rxq_cfg, uint32_t *entry_size,
    uint32_t *init_write_idx)
{
    if (entry_size)
    {
        /* same entry size for data and recycle rings */
        *entry_size = rxq_cfg->type == rdpa_ring_feed ?
            sizeof(RDD_CPU_FEED_DESCRIPTOR_DTS) : sizeof(RDD_CPU_RX_DESCRIPTOR_DTS);
    }
    if (init_write_idx)
    {
        *init_write_idx = rxq_cfg->type == rdpa_ring_feed ? rxq_cfg->size - 1 : 0;
    }
}

int _check_queue_range(cpu_drv_priv_t *cpu_data, uint32_t rxq_idx)
{
    if (rxq_idx >= cpu_data->num_queues && rxq_idx != FEED_RING_ID && rxq_idx != FEED_RCYCLE_RING_ID &&
        rxq_idx != TX_RCYCLE_RING_ID && rxq_idx != TX_HIGH_PRIO_RING_ID && rxq_idx != TX_LOW_PRIO_RING_ID)
    {
        return BDMF_ERR_PARM;
    }

    return 0;
}

void cpu_rxq_cfg_indecies_get(cpu_drv_priv_t *cpu_data, uint8_t *first_rxq_idx, uint8_t *last_rxq_idx)
{
    *first_rxq_idx = 0;
    *last_rxq_idx = cpu_data->num_queues - 1;
}

#if defined(BCM63158)
#define RDPA_CPU_MAX_WLAN_RSV_QUEUES 11 /* 3 radios, 3 queues per radio (2 for WFD, 1 for DHD) + 2 for XTM*/
#else
#define RDPA_CPU_MAX_WLAN_RSV_QUEUES 9 /* 3 radios, 3 queues per radio (2 for WFD, 1 for DHD) */
#endif
int cpu_rxq_cfg_max_num_set(cpu_drv_priv_t *cpu_data)
{
    if (!num_of_avail_data_queues)
        return BDMF_ERR_NORES;
    if (cpu_data->index == rdpa_cpu_host) /* Created by system object at init time */
    {
        /* We must guarantee enough queues for WLAN support */
        cpu_data->num_queues = MIN(num_of_avail_data_queues - RDPA_CPU_MAX_WLAN_RSV_QUEUES, RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ);
    }
    else
    {
        cpu_data->num_queues = MIN(num_of_avail_data_queues, RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ);
    }
    return 0;
}

uint8_t cpu_rdd_rxq_idx_get(cpu_drv_priv_t *cpu_data, bdmf_index rxq_idx)
{
    uint8_t rdd_idx;

    switch (rxq_idx)
    {
        case FEED_RING_ID:
        case FEED_RCYCLE_RING_ID:
        case TX_RCYCLE_RING_ID:
        case TX_HIGH_PRIO_RING_ID:
        case TX_LOW_PRIO_RING_ID:
        case BDMF_INDEX_UNASSIGNED:
            rdd_idx = (uint8_t)rxq_idx;
            break;
        default:
            rdd_idx = (uint8_t)cpu_data->rxq_to_rdd_rxq[rxq_idx];
            break;
    }

    return rdd_idx;
}

int cpu_rxq_cfg_size_validate_ex(cpu_drv_priv_t *cpu_data, rdpa_cpu_rxq_cfg_t *rxq_cfg)
{
    /* same size for feed and recycle rings */
    int max_rxq_size = 0;
    
    if (rxq_cfg->type == rdpa_ring_data)
        max_rxq_size = RDPA_CPU_QUEUE_MAX_SIZE; 
    else if (rxq_cfg->type == rdpa_ring_feed || rxq_cfg->type == rdpa_ring_recycle)
        max_rxq_size = RDPA_FEED_QUEUE_MAX_SIZE;
    else if (rxq_cfg->type == rdpa_ring_cpu_tx)
        max_rxq_size = RDPA_CPU_TX_RING_SIZE;

    /* Check te MAX queue size */
    if (rxq_cfg->size > max_rxq_size)
    {
        BDMF_TRACE_RET(BDMF_ERR_RANGE, "Queue size %u is too big\n"
            "Maximum allowed is %u\n", rxq_cfg->size, max_rxq_size);
    }

    /* Check the MIN queue size. Zero is allowed*/
    if (rxq_cfg->size && rxq_cfg->size < RDPA_CPU_QUEUE_MIN_SIZE)
    {
        BDMF_TRACE_RET(BDMF_ERR_RANGE, "Queue size %u is too small."
            " Minimum allowed is %u\n", rxq_cfg->size, RDPA_CPU_QUEUE_MIN_SIZE);
    }

    /* Check is queue size is multiple of 32 */
    if (rxq_cfg->size & 0x1F)
    {
        BDMF_TRACE_RET(BDMF_ERR_RANGE, "Queue size %u have "
            "to be multiple of 32.\n", rxq_cfg->size);
    }
    return 0;
}

void rdpa_cpu_ring_read_idx_sync(rdpa_cpu_port port, bdmf_index queue)
{
    cpu_drv_priv_t *cpu_data;
    uint32_t ring_id;

    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[port]);
    ring_id = cpu_data->rxq_to_rdd_rxq[queue];
    rdp_cpu_ring_read_idx_ddr_sync(ring_id);
}

void rdpa_cpu_rx_meter_clean_stat_ex(bdmf_index counter_id)
{
    rdd_cpu_rx_meter_clean_stat(counter_id);
}

