/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#include "port.h"
#include "mux_index.h"
#include "runner.h"
#include <rdpa_api.h>
#include <linux/of.h>
#include <linux/nbuff.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include "enet_dbg.h"
#ifdef CONFIG_BCM_PTP_1588
#include "ptp_1588.h"
#endif
#include <net_port.h>

/* Implemented per platform (dsl/pon) */
int _demux_id_runner_port(enetx_port_t *self);

/* runner API implemented by rdp_ring/enet_ring */
extern int runner_ring_create_delete(enetx_channel *chan, int q_id, int size, rdpa_cpu_rxq_cfg_t *rxq_cfg);
extern int runner_get_pkt_from_ring(int hw_q_id, rdpa_cpu_rx_info_t *info);

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include "linux/bcm_log.h"
#include "spdsvc_defs.h"
static bcmFun_t *enet_spdsvc_transmit;
#endif

#if (defined(CONFIG_BCM_SPDTEST) || defined(CONFIG_BCM_SPDTEST_MODULE))
#include "tcpspdtest_defs.h"
#include <net/sock.h>
#endif
#ifdef CONFIG_BCM_RNR_CPU_RX_DYN_METER
#include "dynamic_meters.h"
#endif

#if defined(DSL_DEVICES)
#define NUM_OF_LAN_QUEUES       8
#else
#define NUM_OF_LAN_QUEUES       4
#endif
#define NUM_OF_WAN_QUEUES       8

static int enet_dump_rx;
static int system_is_read;

/* RDPA queue configuration */

#define RDPA_CPU_QUEUE_LOW 3
#define RDPA_CPU_QUEUE_HI 4

int rdpa_cpu_tc_high;
static int rdpa_cpu_tc_low;

#if defined(CONFIG_BCM_XRDP)
#define DEFAULT_Q_SIZE 1024
#elif defined(CONFIG_BCM96848)
#define DEFAULT_Q_SIZE 64
#elif defined(DSL_DEVICES)
#define DEFAULT_Q_SIZE 512
#else
#define DEFAULT_Q_SIZE 256
#endif
#if defined(DSL_DEVICES) && defined(CONFIG_BCM_VOICE_SUPPORT)
 /* Reduce budget to avoid latency for voice processing */
#define DEFAULT_NAPI_WEIGHT 16
#elif  defined(DSL_DEVICES) || defined(CONFIG_BCM_XRDP)
 /* Set NAPI budget to 32 like enet impl5, otherwise XTM and
    SPU/PDC threads won't have enough CPU cycles to process */
#define DEFAULT_NAPI_WEIGHT 32
#else
#define DEFAULT_NAPI_WEIGHT (DEFAULT_Q_SIZE / 2)
#endif

#define _rdpa_destroy_queue(cpu_obj, channel, q_id) _rdpa_create_queue(cpu_obj, channel, q_id, 0)

#ifdef ENET_CPU_LOW_PR_Q_METER_ENABLE
#define METER_INDEX                 (0)
#define MAX_RECOMENDED_RATE         (10000)
#define MAX_RECOMENDED_BURST_SIZE   (10000)
#endif

#define BC_RATE_LIMIT_METER_INDEX    (1)
#define BC_MAX_RECOMENDED_BURST_SIZE (65535) /* not exceed 16 bit max value */
#define BC_RATE_LIMIT_DISABLE        (0xffffffff)

#ifdef ENET_DEBUG_RX_CPU_TRAFFIC
#include "bcmenet_proc.h"
#endif

#include <bdmf_dev.h>

extern int enetx_weight_budget;
extern void bdmf_sysb_databuf_recycle(void *pBuf, unsigned long context);
extern int chan_thread_handler(void *data);
rdpa_system_init_cfg_t init_cfg = {};

char *port_runner_print_priv(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;

    return port_obj->name;
}

void enetxapi_queue_int_enable(enetx_channel *chan, int q_id)
{
    rdpa_cpu_int_enable(rdpa_cpu_host, chan->rx_q[q_id]);
}

void enetxapi_queue_int_disable(enetx_channel *chan, int q_id)
{
    rdpa_cpu_int_disable(rdpa_cpu_host, chan->rx_q[q_id]);
    rdpa_cpu_int_clear(rdpa_cpu_host, chan->rx_q[q_id]);
}

int link_switch_to_rdpa_port(bdmf_object_handle port_obj)
{
    bdmf_error_t rc = 0;
    bdmf_object_handle switch_port_obj = NULL;

    if ((rc = rdpa_port_get(rdpa_if_switch, &switch_port_obj)))
    {
        enet_err("Failed to get RDPA switch port object. rc=%d\n", rc);
        goto Exit;
    }

    if ((rc = bdmf_link(switch_port_obj, port_obj, NULL)))
    {
        enet_err("Failed to link RDPA port to switch. rc=%d\n", rc);
        goto Exit;
    }

Exit:
    if (switch_port_obj)
        bdmf_put(switch_port_obj);

    return rc;
}

static inline void _rdpa_reason_set_tc_and_queue(rdpa_cpu_reason reason, uint8_t *tc, bdmf_index *queue)
{
    switch (reason)
    {
    case rdpa_cpu_rx_reason_etype_pppoe_d:
    case rdpa_cpu_rx_reason_etype_pppoe_s:
    case rdpa_cpu_rx_reason_etype_arp:
    case rdpa_cpu_rx_reason_etype_802_1ag_cfm:
    case rdpa_cpu_rx_reason_l4_icmp:
    case rdpa_cpu_rx_reason_icmpv6:
    case rdpa_cpu_rx_reason_igmp:
    case rdpa_cpu_rx_reason_dhcp:
    case rdpa_cpu_rx_reason_l4_udef_0:
#if defined(CONFIG_BCM_DSL_XRDP) || defined(CONFIG_BCM_DSL_RDP)
    case rdpa_cpu_rx_reason_hit_trap_high:
    case rdpa_cpu_rx_reason_ingqos:
#endif
#if defined(XRDP) && defined(CONFIG_BCM_FTTDP_G9991)
    case rdpa_cpu_rx_reason_etype_udef_0: /* udef index: INBAND_FILTER_RDPA_INDEX */
#endif
        *queue = RDPA_CPU_QUEUE_HI;
        *tc = rdpa_cpu_tc_high;
        break;
    default:
        *queue = RDPA_CPU_QUEUE_LOW;
        *tc = rdpa_cpu_tc_low;
        break;
    }
}

#ifndef TEST_INGRESS
int _configure_tc_hi_lo(bdmf_object_handle system_obj)
{
    rdpa_cpu_tc rdpa_sys_tc_threshold = rdpa_cpu_tc1;
    
    if (rdpa_system_high_prio_tc_threshold_get(system_obj, &rdpa_sys_tc_threshold))
        return -1;

#ifdef XRDP
    rdpa_cpu_tc_high = rdpa_sys_tc_threshold + 1; 
#else
    rdpa_cpu_tc_high = rdpa_cpu_tc1; 
#endif
    rdpa_cpu_tc_low = rdpa_cpu_tc0; 

    return 0;
}

#ifdef XRDP
static int _rdpa_map_reasons_2_queue(bdmf_object_handle system_obj, bdmf_object_handle cpu_obj)
{
    int rc;
    rdpa_cpu_reason reason;
    uint8_t tc;
    bdmf_index queue;

    if ((rc = _configure_tc_hi_lo(system_obj)))
        return rc;
    
    for (reason = rdpa_cpu_reason_min; reason < rdpa_cpu_reason__num_of; reason++)
    {
        _rdpa_reason_set_tc_and_queue(reason, &tc, &queue);

        /* do not override udef reasons might be set before enet */
#if defined(CONFIG_BCM_DSL_XRDP) || defined(CONFIG_BCM_DSL_RDP)
        if (reason >= rdpa_cpu_rx_reason_udef_0  && reason <= rdpa_cpu_rx_reason_udef_3)
#else
        if (reason >= rdpa_cpu_rx_reason_udef_0  && reason <= rdpa_cpu_rx_reason_udef_7)
#endif
            continue;

        rc = rdpa_system_cpu_reason_to_tc_set(system_obj, reason, tc);
        rc = rc ? rc : rdpa_cpu_tc_to_rxq_set(cpu_obj, tc, queue);
        if (rc < 0)
        {
            enet_err("failed to set Map TC to RXQ, error: %d, RDPA reason %d, TC %d, CPU RXQ %d\n", rc,
                (int)reason, (int)tc, (int)queue);
            return rc;
        }	
    }
    return 0;
}
#else
static int _rdpa_map_reasons_2_queue(bdmf_object_handle system_obj, bdmf_object_handle cpu_obj)
{
    int rc;
    rdpa_cpu_reason_index_t reason_cfg_idx = {BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED};
    rdpa_cpu_reason_cfg_t reason_cfg = {};
    uint8_t tc;

    if ((rc = _configure_tc_hi_lo(system_obj)))
        return rc;

    reason_cfg.meter = BDMF_INDEX_UNASSIGNED;
    while (!rdpa_cpu_reason_cfg_get_next(cpu_obj, &reason_cfg_idx))
    {
#if defined(CONFIG_BCM_DSL_RDP) || (defined(CONFIG_BCM96848) || defined  (CONFIG_BCM96838))
        /* do not override override tcpspeedtest reason */
        if (reason_cfg_idx.reason == rdpa_cpu_rx_reason_tcpspdtst)
            continue;
#endif
        _rdpa_reason_set_tc_and_queue(reason_cfg_idx.reason, &tc, &reason_cfg.queue);
#if defined(DSL_DEVICES)
        /* sf2 based runner has LAN, WAN tables */
        reason_cfg_idx.table_index = (reason_cfg_idx.dir == rdpa_dir_us)?
                CPU_REASON_LAN_TABLE_INDEX : CPU_REASON_WAN1_TABLE_INDEX;
#endif
        rc = rdpa_cpu_reason_cfg_set(cpu_obj, &reason_cfg_idx, &reason_cfg);
        if (rc < 0)
        {
            enet_err("failed to set RDPA reason %d\n", rc);
            return rc;
        }
    }
    return 0;
}
#endif
#else
static int _rdpa_map_reasons_2_queue(bdmf_object_handle system_obj, bdmf_object_handle cpu_obj)
{
#define TEST_INGRESS_TC 1
#define TEST_INGRESS_RXQ 1
    return rdpa_cpu_tc_to_rxq_set(cpu_obj, TEST_INGRESS_TC, TEST_INGRESS_RXQ);
}
#endif

static void rdpa_rx_isr(long priv)
{
    enetx_rx_isr((enetx_channel *)priv);
}

static void rdpa_rx_dump_data_cb(bdmf_index queue, bdmf_boolean enabled)
{
    enet_dump_rx = enabled;
}

#ifdef ENET_CPU_LOW_PR_Q_METER_ENABLE
static int configure_cpu_low_prq_meter(bdmf_object_handle rdpa_cpu_obj)
{
    rdpa_dir_index_t dir_idx;
    rdpa_cpu_meter_cfg_t meter_cfg;
    rdpa_cpu_reason_index_t reason_cfg_idx = {BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED};
    rdpa_cpu_reason_cfg_t reason_cfg = {};
    int rc = 0;

    dir_idx.dir = rdpa_dir_us;
    dir_idx.index = METER_INDEX;
    
    meter_cfg.sir = MAX_RECOMENDED_RATE;
    meter_cfg.burst_size = MAX_RECOMENDED_BURST_SIZE;
        
    rc = rdpa_cpu_meter_cfg_set(rdpa_cpu_obj, &dir_idx , &meter_cfg);
    if (rc < 0)
    {
        enet_err("Error(%d) Meter CFG Set\n", rc);
        goto exit;
    }

    reason_cfg_idx.dir = rdpa_dir_us;
    reason_cfg_idx.reason = rdpa_cpu_rx_reason_ip_flow_miss;
    
    reason_cfg.meter_ports = rdpa_ports_all_lan();
    reason_cfg.meter = METER_INDEX;
    reason_cfg.queue = RDPA_CPU_QUEUE_LOW;

    rc = rdpa_cpu_reason_cfg_set(rdpa_cpu_obj, &reason_cfg_idx, &reason_cfg);
    if (rc < 0)
        enet_err("Error(%d) configuring CPU reason to meter\n", rc);

exit:
    return rc;

}

static int unconfigure_cpu_low_prq_meter(bdmf_object_handle rdpa_cpu_obj)
{
    rdpa_cpu_reason_index_t reason_cfg_idx = {BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED};
    rdpa_cpu_reason_cfg_t reason_cfg = {};
    int rc = 0;

    reason_cfg_idx.dir = rdpa_dir_us;
    reason_cfg_idx.reason = BDMF_INDEX_UNASSIGNED; // disconnect meter from reason
    
    reason_cfg.meter_ports = rdpa_ports_all_lan();
    reason_cfg.meter = METER_INDEX;
    reason_cfg.queue = RDPA_CPU_QUEUE_LOW;

    rc = rdpa_cpu_reason_cfg_set(rdpa_cpu_obj, &reason_cfg_idx, &reason_cfg);
    if (rc < 0)
        enet_err("Error(%d) configuring CPU reason to meter\n", rc);

    return rc;
}

#endif

int configure_bc_rate_limit_meter(int port_id, unsigned int rate_limit)
{
    bdmf_object_handle rdpa_cpu_obj = NULL;
    rdpa_dir_index_t dir_idx;
    rdpa_cpu_meter_cfg_t meter_cfg;
    rdpa_cpu_reason_index_t reason_cfg_idx = {BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED};
    rdpa_cpu_reason_cfg_t reason_cfg = {};
    int rc = 0;

    if ((rc = rdpa_cpu_get(rdpa_cpu_host, &rdpa_cpu_obj)) < 0)
        goto exit;

    dir_idx.dir = rdpa_dir_us;
    dir_idx.index = BC_RATE_LIMIT_METER_INDEX;
    if (BC_RATE_LIMIT_DISABLE != rate_limit)
    {
        meter_cfg.sir = rate_limit;
        meter_cfg.burst_size = BC_MAX_RECOMENDED_BURST_SIZE;
        if ((rc = rdpa_cpu_meter_cfg_set(rdpa_cpu_obj, &dir_idx, &meter_cfg)) < 0)
        {
            enet_err("Error Meter CFG Set!\n");
            goto exit;
        }
    }

    reason_cfg_idx.dir = rdpa_dir_us;
    reason_cfg_idx.reason = rdpa_cpu_rx_reason_bcast;
    reason_cfg_idx.entry_index = BC_RATE_LIMIT_METER_INDEX;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
    reason_cfg_idx.table_index = (reason_cfg_idx.dir == rdpa_dir_us)
        ? CPU_REASON_LAN_TABLE_INDEX : CPU_REASON_WAN1_TABLE_INDEX;
#endif


    if ((rc = rdpa_cpu_reason_cfg_get(rdpa_cpu_obj, &reason_cfg_idx, &reason_cfg)) < 0)
        goto exit;

    if (BC_RATE_LIMIT_DISABLE == rate_limit)
    {
        reason_cfg.meter_ports &= ~(rdpa_if_id(port_id));
    }
    else
    {
        reason_cfg.meter_ports |= rdpa_if_id(port_id);
    }
    reason_cfg.meter = BC_RATE_LIMIT_METER_INDEX;
    reason_cfg.queue = RDPA_CPU_QUEUE_LOW;

    if ((rc = rdpa_cpu_reason_cfg_set(rdpa_cpu_obj, &reason_cfg_idx, &reason_cfg)) < 0)
    {
        enet_err("Error configuring CPU reason to meter!\n");
        goto exit;
    }
exit:
    if (rdpa_cpu_obj)
        bdmf_put(rdpa_cpu_obj);

    return rc;
}

static int _rdpa_create_queue(bdmf_object_handle cpu_obj, enetx_channel *chan, int q_id, int size)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg = {};
    int rc = 0;

    /* Make sure interrupt will not be called during RX queue configuration since interfaces might not be up yet */
    enetxapi_queue_int_disable(chan, q_id);

    rdpa_cpu_rxq_cfg_get(cpu_obj, chan->rx_q[q_id], &rxq_cfg);
    rc = runner_ring_create_delete(chan, q_id, size, &rxq_cfg);
    if (rc)
        goto exit;

    rxq_cfg.size = size;
    rxq_cfg.isr_priv = size ? (long)chan : 0; 
    rxq_cfg.rx_isr = size ? rdpa_rx_isr : NULL;
#ifdef ENET_INT_COALESCING_ENABLE
    rxq_cfg.ic_cfg.ic_enable = 1;
    rxq_cfg.ic_cfg.ic_timeout_us = ENET_INTERRUPT_COALESCING_TIMEOUT_US;
    rxq_cfg.ic_cfg.ic_max_pktcnt = ENET_INTERRUPT_COALESCING_MAX_PKT_CNT;
#endif
    rxq_cfg.rx_dump_data_cb = rdpa_rx_dump_data_cb;
    /* XXX: Must make sure rdd_ring is destroyed before removing rdp ring */
    rc = rc ? rc : rdpa_cpu_rxq_cfg_set(cpu_obj, chan->rx_q[q_id], &rxq_cfg);

exit:
    if (rc < 0)
        enet_err("failed to configure RDPA CPU RX q_id - %d chan->rx_q[q_id] - %d \n", q_id, chan->rx_q[q_id]);

    return rc;
}

static enetx_channel *_create_rdpa_queues(void)
{
    static enetx_channel chan;
#ifdef TWO_CHNL_ONE_QUEUE_PER_CHANNEL
    static enetx_channel chan2;
#endif

#ifdef ONE_CHNL_TWO_QUEUE_PER_CHANNEL
    chan.rx_q_count = 2;
    chan.rx_q[0] = RDPA_CPU_QUEUE_HI;
    chan.rx_q[1] = RDPA_CPU_QUEUE_LOW;
#else
#ifdef TWO_CHNL_ONE_QUEUE_PER_CHANNEL
    chan.next = &chan2;

    chan.rx_q_count = 1;
    chan.rx_q[0] = RDPA_CPU_QUEUE_HI;
    chan.next->rx_q_count = 1;
    chan.next->rx_q[0] = RDPA_CPU_QUEUE_LOW;
#else
#if TEST_INGRESS
    chan.rx_q_count = 1;
    chan.rx_q[0] = TEST_INGRESS_RXQ;
#endif
#endif
#endif

    if (chan.rx_q_count == 0)
    {
        enet_err("No RDPA RXQ configuration compiled-in\n");
        return NULL;
    }

    return &chan;
}

int enetxapi_queues_init(enetx_channel **_chan)
{
    bdmf_object_handle cpu_obj, system_obj;
    enetx_channel *chan, *next;
    int i, rc;

    *_chan = chan = _create_rdpa_queues();
    if (!chan)
        goto exit;

    rc = rdpa_system_get(&system_obj);
    rc = rc ? rc : rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);

    init_waitqueue_head(&chan->rxq_wqh);
    chan->rx_thread = kthread_run(chan_thread_handler, chan, "bcmsw_rx");

    while (chan)
    {
        next = chan->next;

        for (i = 0; i < chan->rx_q_count; i++)
            rc = rc ? : _rdpa_create_queue(cpu_obj, chan, i, DEFAULT_Q_SIZE);

        chan = next;
    }

    enetx_weight_budget = DEFAULT_NAPI_WEIGHT;
    
    rc = rc ? : _rdpa_map_reasons_2_queue(system_obj, cpu_obj);

#ifdef ENET_CPU_LOW_PR_Q_METER_ENABLE 
    if (!rc) 
        rc = configure_cpu_low_prq_meter(cpu_obj);
#endif
#ifdef CONFIG_BCM_RNR_CPU_RX_DYN_METER
    dynamic_meters_init(cpu_obj, RDPA_CPU_QUEUE_LOW);
#endif

exit:
    if (!rc)
    {
        enet_dbg("configured RDPA CPU queues\n");
    }
    else
    {
        enet_err("failed to configure RDPA CPU queues\n");
        enetxapi_queues_uninit(_chan);
    }

    if (cpu_obj)
        bdmf_put(cpu_obj);
    if (system_obj)
        bdmf_put(system_obj);

    return rc;
}

int enetxapi_queues_uninit(enetx_channel **_chan)
{
    bdmf_object_handle cpu_obj = NULL;
    int rc, i;
    enetx_channel *next, *chan = *_chan;

#ifdef CONFIG_BCM_RNR_CPU_RX_DYN_METER
    dynamic_meters_uninit(cpu_obj);
#endif
    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
    if (rc)
        goto exit;

    while (chan)
    {

        if (chan->rx_thread)
        {
            kthread_stop(chan->rx_thread);
            chan->rx_thread = NULL;
        }

        next = chan->next;

        for (i = 0; i < chan->rx_q_count; i++)
            rc |= _rdpa_destroy_queue(cpu_obj, chan, i);

        chan = next;
    }

#ifdef ENET_CPU_LOW_PR_Q_METER_ENABLE 
    if (!rc) 
        rc = unconfigure_cpu_low_prq_meter(cpu_obj);

#endif

exit:
    if (rc)
        enet_err("failed to remove RDPA CPU queues\n");
    else
        enet_dbg("removed RDPA CPU queues\n");

    if (cpu_obj)
        bdmf_put(cpu_obj);

    *_chan = NULL;

    return rc;
}

int enetxapi_rx_pkt(int hw_q_id, FkBuff_t **fkb, enetx_rx_info_t *rx_info)
{
    rdpa_cpu_rx_info_t info = {};
    int rc;
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)   
    uint32_t flow_key = 0;
    fc_class_ctx_t fc_key;
#endif

    rc = runner_get_pkt_from_ring(hw_q_id, &info);
    if (unlikely(rc))
    {
        if (unlikely(rc != BDMF_ERR_NO_MORE))
        {
            enet_dbg_rx("enetxapi_rx_pkt error: rc %d\n", rc);
            return rc;
        }

        return -1;
    }
#ifdef ENET_DEBUG_RX_CPU_TRAFFIC
    if (g_debug_mode) 
    {
        *fkb = (FkBuff_t *)info.data;
        
        return 0;
    }
#endif

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
    if (!info.is_exception)
    { 
        info.data_offset += L2_FLOW_P_LEN;
        info.size -= L2_FLOW_P_LEN;
        
        flow_key = *((uint32_t*)info.data);
        fc_key.word = flow_key;
        info.src_port = fc_key.id.src_port;        
        fc_key.id.src_port = 0x0;
        flow_key = fc_key.word;
    }
#endif    

#if defined(CONFIG_BCM_PTP_1588) && defined(XRDP)
    /* In 1588 packets, TS(32 bit) is appended to the begining of the packet by the XRDP,
     * so the data_offset pointer should be forwarded by 32bit and packet length decreased by 32bit */
    if (unlikely(info.reason == rdpa_cpu_rx_reason_etype_ptp_1588))
    {
        info.data_offset += PTP_TS_LEN;
        info.size -= PTP_TS_LEN;
        info.ptp_index = 1;
    }
#endif

    *fkb = fkb_init((uint8_t *)info.data , BCM_PKT_HEADROOM, (uint8_t *)(info.data + info.data_offset), info.size);
    (*fkb)->recycle_hook = (RecycleFuncP)bdmf_sysb_recycle;

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
    if (flow_key) 
        (*fkb)->fc_ctxt = flow_key;
#endif
    
    /* Parameters required by enet */
    rx_info->src_port = info.src_port;
    rx_info->flow_id = info.reason_data;
    rx_info->ptp_index = info.ptp_index;
    rx_info->data_offset = info.data_offset;
    rx_info->reason = info.reason;
    rx_info->extra_skb_flags = 0;
    rx_info->is_exception = info.is_exception;
#if defined(CONFIG_BCM_RUNNER_FLOODING)
    if (unlikely(info.reason == rdpa_cpu_rx_reason_unknown_da))
        rx_info->extra_skb_flags |= SKB_RNR_FLOOD;
#endif

#if defined(CONFIG_BCM_CSO)
    (*fkb)->rx_csum_verified = info.rx_csum_verified;
#endif

    if (unlikely(enet_dump_rx))
        rdpa_cpu_rx_dump_packet("enet", rdpa_cpu_host, hw_q_id, &info, 0);

    return 0;
}

void enetxapi_fkb_databuf_recycle(FkBuff_t *fkb, void *context)
{
    ETH_GBPM_TRACK_FKB(fkb, GBPM_VAL_RECYCLE, 0);
    bdmf_sysb_databuf_recycle(fkb, (unsigned long)context);
}

void enetxapi_buf_recycle(struct sk_buff *skb, uint32_t context, uint32_t flags)
{
    ETH_GBPM_TRACK_SKB(skb, GBPM_VAL_RECYCLE, 0);
    bdmf_sysb_recycle(skb, context, flags);
}

#ifdef XRDP
#if (defined(CONFIG_BCM_SPDTEST) || defined(CONFIG_BCM_SPDTEST_MODULE))
void enet_spdtest_test_connect(bdmf_sysb sysb, rdpa_cpu_tx_info_t *info)
{
    void *pBuf = PNBUFF_2_PBUF((pNBuff_t)sysb);

    if (pBuf == (void *)NULL)
        return;

    if (IS_SKBUFF_PTR((pNBuff_t)sysb))
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF((pNBuff_t)sysb);

        if (skb->recycle_and_rnr_flags & SKB_RNR_UDPSPDT_BASIC)
        {
            info->bits.is_spdsvc_setup_packet = 1;
            info->spdt_so_mark = RDPA_UDPSPDTEST_SO_MARK_BASIC;
        }
        else if (skb->recycle_and_rnr_flags & SKB_RNR_UDPSPDT_IPERF3)
        {
            info->bits.is_spdsvc_setup_packet = 1;
            info->spdt_so_mark = RDPA_UDPSPDTEST_SO_MARK_IPERF3;
        }
    }
    else if (IS_FKBUFF_PTR((pNBuff_t)sysb))
    {
        FkBuff_t* fkb = PNBUFF_2_FKBUFF((pNBuff_t)sysb);

        if (fkb->spdtst)
        {
            info->bits.is_spdsvc_setup_packet = 1;
            info->spdt_so_mark = RDPA_TCPSPDTEST_SO_MARK;
        }
    }
}
#endif
#endif

inline int _rdpa_cpu_send_sysb(bdmf_sysb sysb, rdpa_cpu_tx_info_t *info)
{
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    spdsvcHook_transmit_t spdsvc_transmit;
#endif
    int rc;

#if defined(RDP) && defined(CONFIG_BCM_FTTDP_G9991)
    /* FTTDP FW does not support sending from sysb, so we need to copy to bpm */
    rc = rdpa_cpu_send_raw(bdmf_sysb_data(sysb), bdmf_sysb_length(sysb), info);
    /* rdpa_cpu_send_raw copies to bpm but does not free buffer */
    nbuff_flushfree(sysb);
#else
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    spdsvc_transmit.pNBuff = sysb;
    spdsvc_transmit.dev = NULL;
    spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
    spdsvc_transmit.phy_overhead = BCM_ENET_OVERHEAD;
    info->bits.is_spdsvc_setup_packet = enet_spdsvc_transmit(&spdsvc_transmit);
    if (info->bits.is_spdsvc_setup_packet < 0)
    {
        /* In case of error, NBuff will be free by spdsvc */
        return -1;
    }
#endif

#if (defined(CONFIG_BCM_SPDTEST) || defined(CONFIG_BCM_SPDTEST_MODULE))
#ifdef XRDP
    /* XXX: Temporary solution - like for Speed Service */
    enet_spdtest_test_connect(sysb, info);
#endif
#endif

    rc = rdpa_cpu_send_sysb(sysb, info);
#endif

    return rc;
}

#ifdef XRDP
#define QUEUE_THRESHOLD 128*1536 /* Drop threshold in bytes */
#else
#define QUEUE_THRESHOLD 128 /* TODO: platform define */
#endif
static int rdpa_get_queue_idx(int is_wan, int qid, int q_count)
{
#if defined(DSL_RUNNER_DEVICE) && !defined(XRDP) /* DSL RDP Platforms */
    if (is_wan)
    {
        return qid;
    }
    return (q_count - qid -1);
#else
    return qid;
#endif
}
static int rdpa_egress_tm_queues_cfg(bdmf_object_handle tm_obj, int is_wan)
{
    bdmf_error_t rc = 0;
    rdpa_tm_queue_cfg_t queue_cfg = {};
    int qid;
    int qidx;
    int q_count =  is_wan ? NUM_OF_WAN_QUEUES : NUM_OF_LAN_QUEUES;

    queue_cfg.drop_alg = rdpa_tm_drop_alg_dt;
    queue_cfg.drop_threshold = QUEUE_THRESHOLD;

    for (qid = 0; qid < q_count; qid++)
    {
        queue_cfg.queue_id = qid;
        qidx = rdpa_get_queue_idx(is_wan, qid, q_count);
        if ((rc = rdpa_egress_tm_queue_cfg_set(tm_obj, qidx, &queue_cfg)))
        {
            enet_err("Failed to configure RDPA egress tm queue %d. rc=%d\n", qid, rc);
            break;
        }
    }

    return rc;
}

int create_rdpa_egress_tm(bdmf_object_handle port_obj)
{
    bdmf_error_t rc;
    BDMF_MATTR(tm_attr, rdpa_egress_tm_drv());
    bdmf_object_handle tm_obj = NULL;
    rdpa_port_tm_cfg_t port_tm_cfg;
    rdpa_if rdpaif;
    int is_wan;

    if ((rc = rdpa_port_index_get(port_obj, &rdpaif)))
    {
        enet_err("Failed to get RDPA port index. rc=%d\n", rc);
        goto Exit;
    }

    is_wan = rdpa_if_is_wan(rdpaif);

    rdpa_egress_tm_dir_set(tm_attr, is_wan ? rdpa_dir_us : rdpa_dir_ds);
    rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_queue);
    rdpa_egress_tm_mode_set(tm_attr, rdpa_tm_sched_sp);

    if ((rc = bdmf_new_and_set(rdpa_egress_tm_drv(), port_obj, tm_attr, &tm_obj)))
    {
        enet_err("Failed to create RDPA egress tm for port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    if ((rc = rdpa_egress_tm_queues_cfg(tm_obj, is_wan))) 
    {
        enet_err("Failed to configure RDPA egress tm queues for port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    if ((rc = rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg)))
    {
        enet_err("Failed to get RDPA egress tm for port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    port_tm_cfg.sched = tm_obj;

    if ((rc = rdpa_port_tm_cfg_set(port_obj, &port_tm_cfg)))
    {
        enet_err("Failed to set RDPA egress tm for port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    enet_dbg("Created RDPA egress tm %s\n", bdmf_object_name(tm_obj));

Exit:
    if (rc && tm_obj)
        bdmf_destroy(tm_obj);

    return rc;
}

#if defined(CONFIG_BCM_DSL_XRDP)
int default_filter_init(bdmf_object_handle port_obj)
{
    rdpa_filter_ctrl_t filter_ctrl;
    int rc;

    filter_ctrl.enabled = TRUE;
    filter_ctrl.action = rdpa_forward_action_host;

    /* adding default entry to always trap IP_FRAG packet */
    rc = rdpa_port_ingress_filter_set(port_obj, RDPA_FILTER_IP_FRAG, &filter_ctrl);
    enet_dbg("Adding filter '%u' for port %s, rc=%d\n", RDPA_FILTER_IP_FRAG,
             bdmf_object_name(port_obj), rc);
               
    return rc;
}
#endif

bdmf_object_handle _create_rdpa_port(rdpa_if rdpaif, rdpa_emac emac, bdmf_object_handle owner, rdpa_if control_sid, int create_egress_tm, int enable_set)
{
    bdmf_error_t rc;
    BDMF_MATTR(rdpa_port_attrs, rdpa_port_drv());
    bdmf_object_handle cpu_obj = NULL;
    bdmf_object_handle port_obj = NULL;
    rdpa_port_dp_cfg_t port_cfg = {};

    enet_dbgv("(if=%x, emac=%x, owner=%px)\n", rdpaif, emac, (void *)owner);
    if ((rc = rdpa_port_index_set(rdpa_port_attrs, rdpaif)))
     {
        enet_err("Failed to set RDPA port index %d. rc=%d\n", rdpaif, rc);
        return NULL;
    }

    if (emac == rdpa_emac_none)
        goto skip_emac;

    port_cfg.emac = emac;

    if (rdpa_if_is_wan(rdpaif))
        rdpa_port_wan_type_set(rdpa_port_attrs, rdpa_wan_gbe);
#if defined(CONFIG_BCM_PON_XRDP) && !defined(CONFIG_GPON_SFU) && !defined(CONFIG_EPON_SFU)
    else 
#endif
        if (rdpaif != rdpa_if_switch)
    {
#ifndef CONFIG_BCM_FTTDP_G9991
        port_cfg.sal_enable = 1;
        port_cfg.dal_enable = 1;
        port_cfg.sal_miss_action = rdpa_forward_action_host;
        port_cfg.dal_miss_action = rdpa_forward_action_host;
#if defined(CONFIG_BCM96878)
        port_cfg.ls_fc_enable = !rdpa_if_is_wan(rdpaif);
#else
        port_cfg.ls_fc_enable = 0;
#endif
#endif
    }
        
#if defined(CONFIG_BCM_FTTDP_G9991) && defined(XRDP)
    /* Trap to host default traffic for system port, not use stat counter for this flow */
    if (rdpaif == rdpa_if_lan29)
    {
        rdpa_ic_result_t def_flow = { .action = rdpa_forward_action_host, .disable_stat = 1, };

        if ((rc = rdpa_port_def_flow_set(rdpa_port_attrs, &def_flow)))
        {
            enet_err("Failed to set RDPA port default flow for system port %d. rc=%d\n", rdpaif, rc);
            goto Exit;
        }
    }
#endif

    if (control_sid != rdpa_if_none)
        port_cfg.control_sid = control_sid;

    if ((rc = rdpa_port_cfg_set(rdpa_port_attrs, &port_cfg)))
    {
        enet_err("Failed to set configuration for RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

skip_emac:
    if (enable_set)
        rdpa_port_enable_set(rdpa_port_attrs, 0);

    if ((rc = bdmf_new_and_set(rdpa_port_drv(), owner, rdpa_port_attrs, &port_obj)))
    {
        enet_err("Failed to create RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    if (create_egress_tm && !rdpa_if_is_lag_and_switch(rdpaif) && (rc = create_rdpa_egress_tm(port_obj)))
    {
        enet_err("Failed to create ergress_tm for RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

#ifdef XRDP
    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
    rc = rc ? rc : rdpa_port_cpu_obj_set(port_obj, cpu_obj);
    if (rc)
    {
        enet_err("Failed to set CPU object for port %s, error %d\n", bdmf_object_name(port_obj), rc);
        goto Exit;
    }
#else
    rc = 0;
#endif

#if defined(CONFIG_BCM_DSL_XRDP)
    if ((rc = default_filter_init(port_obj)))
    {
        enet_err("Failed to set up default filter for RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }
#endif

    enet_dbg("Created RDPA port: %s\n", bdmf_object_name(port_obj));

Exit:
    if (cpu_obj)
        bdmf_put(cpu_obj);
    if (rc && port_obj)
    {
        bdmf_destroy(port_obj);
        port_obj = NULL;
    }

    return port_obj;
}

bdmf_object_handle create_rdpa_port(rdpa_if rdpaif, rdpa_emac emac, bdmf_object_handle owner, rdpa_if control_sid)
{
    return _create_rdpa_port(rdpaif, emac, owner, control_sid, 1, 0);
}

int port_runner_port_uninit(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_tm_cfg_t port_tm_cfg;

    if (!port_obj)
        return 0;

    rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg);
    if (port_tm_cfg.sched)
        bdmf_destroy(port_tm_cfg.sched);

    rdpa_port_uninit_set(port_obj, 1);
    bdmf_destroy(port_obj);
    self->priv = 0;

    mux_set_rx_index(self->p.parent_sw, _demux_id_runner_port(self), NULL);

    return 0;
}

int read_init_cfg(void)
{
    int rc;
    bdmf_object_handle system_obj = NULL;

    if (system_is_read)
        return 0;

    if ((rc = rdpa_system_get(&system_obj)))
    {
        enet_err("Failed to get RDPA System object\n");
        return rc;
    }
    
    rdpa_system_init_cfg_get(system_obj, &init_cfg);
    bdmf_put(system_obj);
    enet_dbgv("init_cfg: runner_ext_sw_cfg.enabled=%d\n", init_cfg.runner_ext_sw_cfg.enabled);

    system_is_read = 1;

    return 0;
}

#include "bcm_chip_arch.h"

int port_runner_sw_init(enetx_port_t *self)
{
#ifdef CONFIG_BCM_PTP_1588
    if (ptp_1588_init())
        return -1;
#endif

#ifdef SF2_DEVICE
    /* for SF2 based runner, only register external switch */
    if (init_cfg.runner_ext_sw_cfg.enabled && (self == root_sw))
        goto Exit;
#endif

    if (!(self->priv = create_rdpa_port(rdpa_if_switch, init_cfg.runner_ext_sw_cfg.emac_id, NULL, rdpa_if_none)))
    {
        enet_err("Failed to create RDPA switch object for %s\n", self->obj_name);
        return -1;
    }

#ifdef SF2_DEVICE
Exit:
#endif
    enet_dbg("Initialized runner switch %s\n", self->obj_name);

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    enet_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
    BCM_ASSERT(enet_spdsvc_transmit != NULL);
#endif

    return 0;
}

int port_runner_sw_uninit(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;

    if (port_obj)
    {
        bdmf_destroy(port_obj);
        self->priv = 0;
    }

#ifdef CONFIG_BCM_PTP_1588
    ptp_1588_uninit();
#endif
    return 0;
}

static void port_runner_rdp_drop_stats_get(enetx_port_t *self, uint32_t *rxDropped, uint32_t *txDropped)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_stat_t stat;
    int rc;

    if (!port_obj)
        return;

    /* Read RDPA statistic */
    rc = rdpa_port_stat_get(port_obj, &stat);
    if (rc)
    {
        enet_err("can not get stat port %s %s\n", self->obj_name, self->dev->name);
    }
    else
    {
        /* Add up the TX discards */
        *txDropped += stat.tx_discard;
        *rxDropped += stat.discard_pkt;
    }
}

void port_runner_port_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    uint32_t rxDropped = 0, txDropped = 0;

    port_generic_stats_get(self, net_stats);

    /* Add FW dropped packets */
    net_stats->rx_dropped += self->n.port_stats.rx_dropped;
    net_stats->tx_dropped += self->n.port_stats.tx_dropped;

    /* Add rdp dropped count */
    port_runner_rdp_drop_stats_get(self, &rxDropped, &txDropped);
    net_stats->rx_dropped += rxDropped;
    net_stats->tx_dropped += txDropped;
}

void port_runner_port_stats_clear(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_stat_t stat = {};

    rdpa_port_stat_set(port_obj, &stat);
    port_generic_stats_clear(self);
}
    
/* mib dump for ports on internal runner switch */
int port_runner_mib_dump(enetx_port_t *self, int all)
{
    /* based on impl5\bcmsw_runner.c bcmeapi_ethsw_dump_mib() */
    mac_stats_t         mac_stats;
    int                 port = self->p.mac->mac_id;
    uint64_t            errcnt = 0;

    mac_dev_stats_get(self->p.mac, &mac_stats);

    printk("\nRunner Stats : Port# %d\n",port);

    /* Display Tx statistics */
     /* Display Tx statistics */
    printk("\n");
    printk("TxUnicastPkts:          %10llu \n", mac_stats.tx_unicast_packet);
    printk("TxMulticastPkts:        %10llu \n", mac_stats.tx_multicast_packet);
    printk("TxBroadcastPkts:        %10llu \n", mac_stats.tx_broadcast_packet);
    printk("TxDropPkts:             %10llu \n", mac_stats.tx_error);

    /* Display remaining tx stats only if requested */
    if (all) {
        printk("TxBytes:                %10llu \n", mac_stats.tx_byte);
        printk("TxFragments:            %10llu \n", mac_stats.tx_fragments_frame);
        printk("TxCol:                  %10llu \n", mac_stats.tx_total_collision);
        printk("TxSingleCol:            %10llu \n", mac_stats.tx_single_collision);
        printk("TxMultipleCol:          %10llu \n", mac_stats.tx_multiple_collision);
        printk("TxDeferredTx:           %10llu \n", mac_stats.tx_deferral_packet);
        printk("TxLateCol:              %10llu \n", mac_stats.tx_late_collision);
        printk("TxExcessiveCol:         %10llu \n", mac_stats.tx_excessive_collision);
        printk("TxPausePkts:            %10llu \n", mac_stats.tx_pause_control_frame);
        printk("TxExcessivePkts:        %10llu \n", mac_stats.tx_excessive_deferral_packet);
        printk("TxJabberFrames:         %10llu \n", mac_stats.tx_jabber_frame);
        printk("TxFcsError:             %10llu \n", mac_stats.tx_fcs_error);
        printk("TxCtrlFrames:           %10llu \n", mac_stats.tx_control_frame);
        printk("TxOverSzFrames:         %10llu \n", mac_stats.tx_oversize_frame);
        printk("TxUnderSzFrames:        %10llu \n", mac_stats.tx_undersize_frame);
        printk("TxUnderrun:             %10llu \n", mac_stats.tx_underrun);
        printk("TxPkts64Octets:         %10llu \n", mac_stats.tx_frame_64);
        printk("TxPkts65to127Octets:    %10llu \n", mac_stats.tx_frame_65_127);
        printk("TxPkts128to255Octets:   %10llu \n", mac_stats.tx_frame_128_255);
        printk("TxPkts256to511Octets:   %10llu \n", mac_stats.tx_frame_256_511);
        printk("TxPkts512to1023Octets:  %10llu \n", mac_stats.tx_frame_512_1023);
        printk("TxPkts1024to1518Octets: %10llu \n", mac_stats.tx_frame_1024_1518);
        printk("TxPkts1519toMTUOctets:  %10llu \n", mac_stats.tx_frame_1519_mtu);
    }
    else {
        errcnt = 0;
        errcnt += mac_stats.tx_total_collision;
        errcnt += mac_stats.tx_single_collision;
        errcnt += mac_stats.tx_multiple_collision;
        errcnt += mac_stats.tx_deferral_packet;
        errcnt += mac_stats.tx_late_collision;
        errcnt += mac_stats.tx_excessive_collision;
        errcnt += mac_stats.tx_excessive_deferral_packet;
        errcnt += mac_stats.tx_jabber_frame;
        errcnt += mac_stats.tx_fcs_error;
        errcnt += mac_stats.tx_undersize_frame;
        errcnt += mac_stats.tx_underrun;
        printk("TxOtherErrors:          %10llu \n", errcnt);
    }

    /* Display Rx statistics */
    printk("\n");
    printk("RxUnicastPkts:          %10llu \n", mac_stats.rx_unicast_packet);
    printk("RxMulticastPkts:        %10llu \n", mac_stats.rx_multicast_packet);
    printk("RxBroadcastPkts:        %10llu \n", mac_stats.rx_broadcast_packet);

    /* Display remaining rx stats only if requested */
    if (all) {
        printk("RxBytes:                %10llu \n", mac_stats.rx_byte);
        printk("RxJabbers:              %10llu \n", mac_stats.rx_jabber);
        printk("RxAlignErrs:            %10llu \n", mac_stats.rx_alignment_error);
        printk("RxFCSErrs:              %10llu \n", mac_stats.rx_fcs_error);
        printk("RxFragments:            %10llu \n", mac_stats.rx_fragments);
        printk("RxOversizePkts:         %10llu \n", mac_stats.rx_oversize_packet);
        printk("RxUndersizePkts:        %10llu \n", mac_stats.rx_undersize_packet);
        printk("RxPausePkts:            %10llu \n", mac_stats.rx_pause_control_frame);
        printk("RxOverflow:             %10llu \n", mac_stats.rx_overflow);
        printk("RxCtrlPkts:             %10llu \n", mac_stats.rx_control_frame);
        printk("RxUnknownOp:            %10llu \n", mac_stats.rx_unknown_opcode);
        printk("RxLenError:             %10llu \n", mac_stats.rx_frame_length_error);
        printk("RxCodeError:            %10llu \n", mac_stats.rx_code_error);
        printk("RxCarrierSenseErr:      %10llu \n", mac_stats.rx_carrier_sense_error);
        printk("RxPkts64Octets:         %10llu \n", mac_stats.rx_frame_64);
        printk("RxPkts65to127Octets:    %10llu \n", mac_stats.rx_frame_65_127);
        printk("RxPkts128to255Octets:   %10llu \n", mac_stats.rx_frame_128_255);
        printk("RxPkts256to511Octets:   %10llu \n", mac_stats.rx_frame_256_511);
        printk("RxPkts512to1023Octets:  %10llu \n", mac_stats.rx_frame_512_1023);
        printk("RxPkts1024to1522Octets: %10llu \n", mac_stats.rx_frame_1024_1518);
        printk("RxPkts1523toMTU:        %10llu \n", mac_stats.rx_frame_1519_mtu);
    }
    else {
        errcnt = 0;
        errcnt += mac_stats.rx_jabber;
        errcnt += mac_stats.rx_alignment_error;
        errcnt += mac_stats.rx_fcs_error;
        errcnt += mac_stats.rx_oversize_packet;
        errcnt += mac_stats.rx_undersize_packet;
        errcnt += mac_stats.rx_overflow;
        errcnt += mac_stats.rx_unknown_opcode;
        errcnt += mac_stats.rx_frame_length_error;
        errcnt += mac_stats.rx_code_error;
        errcnt += mac_stats.rx_carrier_sense_error;
        printk("RxOtherErrors:          %10llu \n", errcnt);
    }
    return 0;
}

// add by Andrew
/* mib dump for ports on internal runner switch */
int port_runner_mib_dump_us(enetx_port_t *self, void *ethswctl)
{
    /* based on impl5\bcmsw_runner.c bcmeapi_ethsw_dump_mib() */
    mac_stats_t         mac_stats;
    struct ethswctl_data *e = (struct ethswctl_data *)ethswctl;

    mac_dev_stats_get(self->p.mac, &mac_stats);

    /* Calculate Tx statistics */
    e->port_stats.txPackets = mac_stats.tx_unicast_packet + 
                              mac_stats.tx_multicast_packet + 
                              mac_stats.tx_broadcast_packet;
    e->port_stats.txDrops = mac_stats.tx_error;
    e->port_stats.txBytes = mac_stats.tx_byte;

    /* Calculate Rx statistics */
    e->port_stats.rxPackets = mac_stats.rx_unicast_packet + 
                              mac_stats.rx_multicast_packet + 
                              mac_stats.rx_broadcast_packet;
    e->port_stats.rxBytes = mac_stats.rx_byte;
    e->port_stats.rxDrops = 0;
    e->port_stats.rxDiscards = 0;

    return 0;
}
// end of add

int port_runner_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    int rc;

    if ((rc = read_init_cfg()))
        return rc;

#ifdef GPON
    if (port_info->is_gpon)
    {
        *port_type = PORT_TYPE_RUNNER_GPON;
        return 0;
    }
#endif
    
#ifdef EPON
    if (port_info->is_epon)
    {
        *port_type = PORT_TYPE_RUNNER_EPON;
        *port_id = rdpa_wan_type_to_if(rdpa_wan_epon);
        return 0;
    }
#endif

#ifdef ENET_RUNNER_WIFI
    if (port_info->port >= rdpa_if_ssid0 && port_info->port <= rdpa_if_ssid15)
    {
        *port_type = PORT_TYPE_RUNNER_WIFI;
        *port_id = port_info->port;
        return 0;
    }
#endif
    
    *port_type = PORT_TYPE_RUNNER_PORT;
    if (port_info->is_undef)
    {
        *port_type = PORT_TYPE_RUNNER_MAC;
        *port_id = rdpa_if_none;
        return 0;
    }

    if (port_info->port == init_cfg.gbe_wan_emac && init_cfg.gbe_wan_emac != rdpa_emac_none)
        port_info->is_wan = 1;

    if (port_info->is_wan)
    {
        *port_id = rdpa_wan_type_to_if(rdpa_wan_gbe);
        return 0;
    }

    if (port_info->is_attached)
        *port_id = rdpa_if_lag0 + port_info->port;
    else if (port_info->is_management)
        *port_id = rdpa_if_lan0 + 29;
    else if (init_cfg.runner_ext_sw_cfg.enabled)
        /* port in board param is not being used (for example CONFIG_BCM_ETHWAN not defined)
           don't create unspecified port during bp parsing */
        return 1;
    else
        *port_id = rdpa_if_lan0 + port_info->port;

    return 0;
}

int port_runner_mtu_set(enetx_port_t *self, int mtu)
{
    bdmf_object_handle port_obj = self->priv;
    bdmf_number old_rdpa_mtu;
    int rc;

    /* XXX: Is this needed for rdpa, or only mac ? */
    mtu += ENET_MAX_MTU_EXTRA_SIZE;

    rdpa_port_mtu_size_get(port_obj, &old_rdpa_mtu);
    if ((rc = rdpa_port_mtu_size_set(port_obj, mtu)))
    {
        enet_err("failed to set rdpa mtu size %d on %s\n", mtu, self->obj_name);
        return -1;
    }

    if (port_generic_mtu_set(self, mtu))
    {
        /* Rollback */
        rdpa_port_mtu_size_set(port_obj, old_rdpa_mtu);
        return -1;
    }

    return 0;
}

void port_runner_print_status(enetx_port_t *self)
{
    printk("%s ", self->dev->name);
    phy_dev_print_status(self->p.phy);
}

int port_runner_link_change(enetx_port_t *self, int up)
{
    return rdpa_port_enable_set(self->priv, up);
}

int enetxapi_rx_pkt_dump_on_demux_err(enetx_rx_info_t *rx_info)
{
    if (rdpa_if_is_cpu_not_wlan(rx_info->src_port))
    {
        /* rdpa_if_cpuXXX is not linked to any network device in enet, but packets can come if generated by CPU/Runner
         * pktgen and there is no available flow. In that case, we don't want to dump an error. */
        return 0;
    }
    return 1;
}

#ifdef GPON
#if !defined(CONFIG_BCM963158) 
static void port_runner_gpon_stats_clear(enetx_port_t *self)
{
   rdpa_gem_stat_t gem_stat = {};
   bdmf_object_handle gem = NULL;
   rdpa_iptv_stat_t iptv_stat = {};
   bdmf_object_handle iptv = NULL;
   int rc;

    while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
    {
        rc = rdpa_gem_stat_set(gem, &gem_stat);
        if (rc)
            goto gem_exit;
    }

gem_exit:
    if (gem)
        bdmf_put(gem);

    rc = rdpa_iptv_get(&iptv);
    if (rc)
        goto iptv_exit;

    rc = rdpa_iptv_iptv_stat_set(iptv, &iptv_stat);
    if (rc)
        goto iptv_exit;

iptv_exit:
    if (iptv)
        bdmf_put(iptv);
}

static void port_runner_gpon_stats(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
   rdpa_gem_stat_t gem_stat;
   bdmf_object_handle gem = NULL;
   rdpa_iptv_stat_t iptv_stat;
   bdmf_object_handle iptv = NULL;
#ifdef CONFIG_BCM_FTTDP_G9991   
   rdpa_port_stat_t   wan_port_stat;
#endif
   
   int rc;

    while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
    {
        rc = rdpa_gem_stat_get(gem, &gem_stat);
        if (rc)
            goto gem_exit;

        net_stats->rx_bytes += gem_stat.rx_bytes;
        net_stats->rx_packets += gem_stat.rx_packets;
        net_stats->rx_dropped += gem_stat.rx_packets_discard;
        net_stats->tx_bytes += gem_stat.tx_bytes;
        net_stats->tx_packets += gem_stat.tx_packets;
        net_stats->tx_dropped += gem_stat.tx_packets_discard;
    }

gem_exit:
    if (gem)
        bdmf_put(gem);

    rc = rdpa_iptv_get(&iptv);
    if (rc)
        goto iptv_exit;

    rc = rdpa_iptv_iptv_stat_get(iptv, &iptv_stat);
    if (rc)
        goto iptv_exit;

    net_stats->multicast = iptv_stat.rx_valid_pkt;
#ifdef CONFIG_BCM_KF_EXTSTATS
    net_stats->rx_multicast_bytes = iptv_stat.rx_valid_bytes;
#endif

#ifdef CONFIG_BCM_FTTDP_G9991
    rc = rdpa_port_stat_get(self->priv, &wan_port_stat);
    if (rc)
        goto iptv_exit;

    net_stats->multicast = wan_port_stat.rx_multicast_pkt;
#ifdef CONFIG_BCM_KF_EXTSTATS
    net_stats->rx_broadcast_packets = wan_port_stat.rx_broadcast_pkt;
    net_stats->tx_multicast_packets = wan_port_stat.tx_multicast_pkt;
    net_stats->tx_broadcast_packets = wan_port_stat.tx_broadcast_pkt;
#endif /* CONFIG_BCM_KF_EXTSTATS */    

#endif /* CONFIG_BCM_FTTDP_G9991 */

iptv_exit:
    if (iptv)
        bdmf_put(iptv);
}
#endif //!63158

static int port_runner_gpon_init(enetx_port_t *self)
{
    BDMF_MATTR(port_attrs, rdpa_port_drv());
    rdpa_wan_type wan_type;
    int rc;
#if (defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM_DSL_XRDP)) && !defined(CONFIG_GPON_SFU) && !defined(CONFIG_EPON_SFU)
    rdpa_port_dp_cfg_t port_cfg;
#endif
    
    self->p.port_cap = PORT_CAP_WAN_ONLY;
    self->n.port_netdev_role = PORT_NETDEV_ROLE_WAN;

#ifdef CONFIG_BLOG
    self->n.blog_phy = BLOG_GPONPHY;
    self->n.set_channel_in_mark = 1; /* blog_chnl will be set to/from gem */
#endif

    if (!self->p.mac || self->p.mac->mac_drv->mac_type != MAC_TYPE_xGPON)
        return -1;

    wan_type = self->p.mac->mac_id == NET_PORT_SUBTYPE_GPON ? rdpa_wan_gpon : rdpa_wan_xgpon;
    self->p.port_id = rdpa_wan_type_to_if(wan_type);
    
    rdpa_port_index_set(port_attrs, self->p.port_id);
    /* wan_type MUST come after index */
    rdpa_port_wan_type_set(port_attrs, wan_type);
    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, port_attrs, (bdmf_object_handle *)&self->priv);
    if (rc < 0)
    {
        enet_err("Problem creating gpon wan port object\n");
        return rc;
    }
#if (defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM_DSL_XRDP)) && !defined(CONFIG_GPON_SFU) && !defined(CONFIG_EPON_SFU)
    rdpa_port_cfg_get(*(bdmf_object_handle *)&self->priv, &port_cfg);
    /* XRDP: For WAN port, SAL and DAL should be disabled by default */
    port_cfg.sal_enable = 0;
    port_cfg.dal_enable = 0;
    rdpa_port_cfg_set(*(bdmf_object_handle *)&self->priv, &port_cfg);
#endif
    
    if (mux_set_rx_index(self->p.parent_sw, self->p.port_id, self))
        return -1;

    return 0;
}

static int port_runner_gpon_uninit(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;

    mux_set_rx_index(self->p.parent_sw, self->p.port_id, NULL);

    bdmf_destroy(port_obj);
    self->priv = 0;

    return 0;
}

static int dispatch_pkt_gpon(dispatch_info_t *dispatch_info)
{
    int rc;
    rdpa_cpu_tx_info_t info = {};

    info.method = rdpa_cpu_tx_port;
    info.port = rdpa_wan_type_to_if(rdpa_wan_gpon);
    info.cpu_port = rdpa_cpu_host;
    info.x.wan.queue_id = dispatch_info->egress_queue;
    info.x.wan.flow = dispatch_info->channel;
    info.drop_precedence = dispatch_info->drop_eligible;
    info.flags = 0;

    if (dispatch_info->channel == 0)
    {
        nbuff_flushfree(dispatch_info->pNBuff);
        return 0;
    }

    rc = _rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
    if (unlikely(rc != 0))
    {
        rdpa_gem_flow_us_cfg_t us_cfg = {};
        bdmf_object_handle gem = NULL;

        rdpa_gem_get(dispatch_info->channel, &gem);
        if (gem)
        {
            rdpa_gem_us_cfg_get(gem, &us_cfg);
            bdmf_put(gem);

            if (!us_cfg.tcont)
            {
                enet_err("can't send sysb - no tcont for gem (%d) \n", dispatch_info->channel);
                return rc;
            }
        }

        enet_err("_rdpa_cpu_send_sysb() rc %d (wan_flow: %d queue_id: %u)\n",
          rc, dispatch_info->channel, dispatch_info->egress_queue);
    }

    return rc;
}

port_ops_t port_runner_gpon =
{
    .init = port_runner_gpon_init,
    .uninit = port_runner_gpon_uninit,
    .dispatch_pkt = dispatch_pkt_gpon,
#if defined(CONFIG_BCM963158)
    .stats_get = port_generic_sw_stats_get,
#else
    .stats_get = port_runner_gpon_stats,
    .stats_clear = port_runner_gpon_stats_clear,
#endif
    .mtu_set = port_runner_mtu_set,
    /* TODO: stats_clear */
    .mib_dump = port_runner_mib_dump,
    .print_status = port_runner_print_status,
    .print_priv = port_runner_print_priv,
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};
#endif /* GPON */

port_ops_t port_runner_port_mac =
{
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_runner_mtu_set,
    .mib_dump = port_runner_mib_dump,
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

#ifdef EPON
static void port_runner_epon_stats(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    port_generic_stats_get(self, net_stats);
}

int create_rdpa_egress_tm(bdmf_object_handle port_obj);

bdmf_object_handle create_epon_rdpa_port(mac_dev_t *mac_dev)
{
    BDMF_MATTR(port_attrs, rdpa_port_drv());
    bdmf_object_handle port_obj = NULL;
    bdmf_object_handle cpu_obj = NULL;
    rdpa_wan_type wan_type = rdpa_wan_xepon;
    rdpa_port_dp_cfg_t port_cfg = {0};
    mac_cfg_t mac_cfg = {};
    bdmf_error_t rc = 0;

    if (!mac_dev || !mac_dev->mac_drv)
        return NULL;
    
#if (defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM_DSL_XRDP)) && !defined(CONFIG_GPON_SFU) && !defined(CONFIG_EPON_SFU)
    /* XRDP: For WAN port, SAL and DAL should be disabled by default */
    port_cfg.sal_enable = 0;
    port_cfg.dal_enable = 0;
#else
    port_cfg.sal_enable = 1;
    port_cfg.dal_enable = 1;
#endif
    port_cfg.sal_miss_action = rdpa_forward_action_host;
    port_cfg.dal_miss_action = rdpa_forward_action_host;
    port_cfg.control_sid = rdpa_if_none;
    port_cfg.emac = rdpa_emac_none;
    port_cfg.physical_port = rdpa_physical_none;
    port_cfg.ls_fc_enable = 0;
    port_cfg.ae_enable = (mac_dev->mac_drv->mac_type == MAC_TYPE_EPON_AE);
        
    mac_dev_cfg_get(mac_dev, &mac_cfg);
    if ((!port_cfg.ae_enable) && (mac_cfg.speed != MAC_SPEED_10000))
    {
        wan_type = rdpa_wan_epon;
    }
    rdpa_port_index_set(port_attrs, rdpa_wan_type_to_if(wan_type));
    rdpa_port_wan_type_set(port_attrs, wan_type);
    rdpa_port_cfg_set(port_attrs, &port_cfg);
    if (port_cfg.ae_enable)
    {
        if (mac_cfg.speed == MAC_SPEED_100)
            rdpa_port_speed_set(port_attrs,rdpa_speed_100m);
        else if (mac_cfg.speed == MAC_SPEED_1000)
            rdpa_port_speed_set(port_attrs,rdpa_speed_1g);
        else if (mac_cfg.speed == MAC_SPEED_2500)
            rdpa_port_speed_set(port_attrs,rdpa_speed_2_5g);
        else if (mac_cfg.speed == MAC_SPEED_5000)
            rdpa_port_speed_set(port_attrs,rdpa_speed_5g);
        else if (mac_cfg.speed == MAC_SPEED_10000)
            rdpa_port_speed_set(port_attrs,rdpa_speed_10g);
        else
        {
            enet_err("AE speed is not supported");
            goto Exit;
        }
    }

    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, port_attrs, &port_obj);
    if (rc < 0)
    {
        enet_err("Failed to create wan port");
        goto Exit;
    }

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
    if (rc)
    {
        enet_err("Failed to get CPU object, rc(%d)", rc);
        goto Exit;
    }
#if defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM_DSL_XRDP)
    rc = rdpa_port_cpu_obj_set(port_obj, cpu_obj);
    if (rc)
    {
        enet_err("Failed to set CPU object to WAN port, rc(%d)", rc);
        goto Exit;
    }
#endif

Exit:
    if (cpu_obj)
        bdmf_put(cpu_obj);
    
    if (rc && port_obj)
    {
        bdmf_destroy(port_obj);
        port_obj = NULL;
    }

    return port_obj;
}

static int port_runner_epon_init(enetx_port_t *self)
{
    rdpa_port_dp_cfg_t cfg;
    bdmf_error_t rc = 0;

    self->p.port_cap = PORT_CAP_WAN_ONLY;
    self->n.port_netdev_role = PORT_NETDEV_ROLE_WAN;
#ifdef CONFIG_BLOG
    self->n.blog_phy = BLOG_EPONPHY;
#endif
    if (mux_set_rx_index(self->p.parent_sw, rdpa_wan_type_to_if(rdpa_wan_epon), self))
        return -1;

    if (!(self->priv = create_epon_rdpa_port(self->p.mac)))
    {
        enet_err("Failed to create EPON rdpa port object for %s\n", self->obj_name);
        return -1;
    }
    
    rc = rdpa_port_get(rdpa_wan_type_to_if(rdpa_wan_epon), (bdmf_object_handle *)&self->priv);
    if (rc)
        goto Exit;

    rc = rdpa_port_cfg_get(self->priv, &cfg);
    if (rc)
        goto Exit;

    if (cfg.ae_enable && (rc = create_rdpa_egress_tm(self->priv)))
        goto Exit;
    
    if (!cfg.ae_enable)
    {
        self->n.set_channel_in_mark = 1; /*Epon mode: channel will be set to/from gem */
    }
#ifdef CONFIG_BLOG    
    else
    {
        self->n.blog_chnl = self->n.blog_chnl_rx = 0; /*AE mode: set blog_chnl=0 */
    }
#endif

Exit:
    if (self->priv)
        bdmf_put(self->priv);
    return rc;
}

static int port_runner_epon_uninit(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_tm_cfg_t port_tm_cfg;

    if (!port_obj)
        return 0;

    rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg);
    if (port_tm_cfg.sched)
        bdmf_destroy(port_tm_cfg.sched);

    rdpa_port_uninit_set(port_obj, 1);
    bdmf_destroy(port_obj);
    self->priv = 0;

    mux_set_rx_index(self->p.parent_sw, rdpa_wan_type_to_if(rdpa_wan_epon), NULL);

    return 0;
}

static int dispatch_pkt_epon(dispatch_info_t *dispatch_info)
{
    int rc;
    rdpa_cpu_tx_info_t info = {};

    /* TODO: queue/channel mapping from EponFrame.c:EponTrafficSend() */
    info.method = rdpa_cpu_tx_port;
    info.port = rdpa_wan_type_to_if(rdpa_wan_epon);
    info.cpu_port = rdpa_cpu_host;
    info.x.wan.queue_id = dispatch_info->egress_queue;
    if (dispatch_info->port->n.set_channel_in_mark)
        info.x.wan.flow = dispatch_info->channel;
        
    info.drop_precedence = dispatch_info->drop_eligible;

    rc = _rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);

    return rc;
}

port_ops_t port_runner_epon =
{
    .init = port_runner_epon_init,
    .uninit = port_runner_epon_uninit,
    .dispatch_pkt = dispatch_pkt_epon,
    .stats_get = port_runner_epon_stats,
    .mtu_set = port_runner_mtu_set,
    /* TODO: stats_clear */
    .mib_dump = port_runner_mib_dump,
    .print_status = port_runner_print_status,
    .print_priv = port_runner_print_priv,
    .link_change = port_runner_link_change,
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};
#endif /* EPON */

