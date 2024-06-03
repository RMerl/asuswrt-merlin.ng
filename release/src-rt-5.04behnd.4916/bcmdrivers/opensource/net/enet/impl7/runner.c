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
#include "runner_common.h"
#include <rdpa_api.h>
#include <linux/of.h>
#include <linux/nbuff.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include "enet_dbg.h"
#include "bcmtypes.h"
#ifdef CONFIG_BCM_PTP_1588
#include "ptp_1588.h"
#endif
#include <net_port.h>

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

#if defined(DSL_DEVICES) && defined(CONFIG_BCM_XRDP)
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
#define RDPA_CPU_QUEUE_HW_FIREWALL 0
#define RDPA_CPU_QUEUE_LOW 1
#define RDPA_CPU_QUEUE_HI 2
#else
#define RDPA_CPU_QUEUE_LOW 0
#define RDPA_CPU_QUEUE_HI 1
#endif
#else
#define RDPA_CPU_QUEUE_LOW 3
#define RDPA_CPU_QUEUE_HI 4
#endif

int rdpa_cpu_tc_high;
static int rdpa_cpu_tc_low;
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
int rdpa_cpu_tc_hw_firewall;
#endif

#if defined(CONFIG_BCM_XRDP)
#define DEFAULT_Q_SIZE 1024
#elif defined(DSL_DEVICES)
#define DEFAULT_Q_SIZE 512
#else
#define DEFAULT_Q_SIZE 256
#endif
#if defined(DSL_DEVICES) && defined(CONFIG_BCM_VOICE_SUPPORT)
 /* Reduce budget to avoid latency for voice processing */
#define DEFAULT_NAPI_WEIGHT 16
#elif defined(DSL_DEVICES) || defined(CONFIG_BCM_XRDP)
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

#ifndef G9991_COMMON
const rdpa_filter default_lan_ingress_filters[] = {
        RDPA_FILTER_DHCP,
        RDPA_FILTER_IGMP,
        RDPA_FILTER_MLD,
        RDPA_FILTER_ETYPE_ARP,
        RDPA_FILTER_ETYPE_802_1AG_CFM,
        RDPA_FILTER_BCAST,
        RDPA_FILTER_HDR_ERR,
#ifndef CONFIG_BCM963158
        /* Multicast ingress filter is not needed on 63158 since
           runner traps to host on hitting a ingress filter.
           On DSL XRDPC platforms(i.e 63146, 4912, 6813), runner
           continues to do flow lookup even after hitting a ingress
           filter.
           On DSL RDP platforms(i.e. 63138, 63148, 4908), no
           ingress filters are configured */
        RDPA_FILTER_MCAST,
#endif
        RDPA_FILTER_IP_MCAST_CONTROL,
#if defined(CONFIG_BCM_PTP_1588)
        RDPA_FILTER_ETYPE_PTP_1588,
        RDPA_FILTER_L4_PTP_1588,
#endif
#if defined(XRDP_RGEN6)
        /* Only runner GEN6 platforms has new parser HW to detect DNS */ 
        RDPA_FILTER_DNS,
#endif
};
#else
const rdpa_filter default_lan_ingress_filters[] = {
        RDPA_FILTER_IGMP,
        RDPA_FILTER_ETYPE_UDEF_0,
};
#endif

const rdpa_filter default_wan_filters[] = {
    RDPA_FILTER_ETYPE_ARP,
    RDPA_FILTER_BCAST,
    RDPA_FILTER_HDR_ERR
};
const rdpa_filter gbe_default_extra_filters[] = {
    RDPA_FILTER_DHCP,    
    RDPA_FILTER_ETYPE_PPPOE_D,
};

extern int enetx_weight_budget;
extern void bdmf_sysb_databuf_recycle(void *pBuf);
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

static inline void _rdpa_reason_set_tc_and_queue(rdpa_cpu_reason reason, uint8_t *tc, bdmf_index *queue)
{
    switch (reason)
    {
    case rdpa_cpu_rx_reason_etype_pppoe_d:
    case rdpa_cpu_rx_reason_etype_pppoe_s:
    case rdpa_cpu_rx_reason_etype_arp:
    case rdpa_cpu_rx_reason_etype_802_1ag_cfm:
#ifndef XRDP
    case rdpa_cpu_rx_reason_l4_icmp:
    case rdpa_cpu_rx_reason_l4_udef_0:
#else /* XRDP */
    case rdpa_cpu_rx_reason_dns:
#endif
    case rdpa_cpu_rx_reason_icmpv6:
    case rdpa_cpu_rx_reason_igmp:
    case rdpa_cpu_rx_reason_dhcp:
    case rdpa_cpu_rx_reason_ingqos:
#if defined(CONFIG_BCM_DSL_XRDP) || defined(CONFIG_BCM_DSL_RDP)
    case rdpa_cpu_rx_reason_hit_trap_high:
#endif
#if defined(XRDP) && defined(CONFIG_BCM_FTTDP_G9991)
    case rdpa_cpu_rx_reason_etype_udef_0: /* udef index: INBAND_FILTER_RDPA_INDEX */
#endif
        *queue = RDPA_CPU_QUEUE_HI;
        *tc = rdpa_cpu_tc_high;
        break;
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
    case rdpa_cpu_rx_reason_hw_firewall_miss:
        *queue = RDPA_CPU_QUEUE_HW_FIREWALL;
        *tc = rdpa_cpu_tc_hw_firewall;
        break;
#endif
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

#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
    /* XRDP-only feature. HW_FIREWALL should be lowest priority */
    rdpa_cpu_tc_low = rdpa_cpu_tc1; 
    rdpa_cpu_tc_hw_firewall = rdpa_cpu_tc0; 
#endif

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
        if (reason >= rdpa_cpu_rx_reason_udef_0  && reason <= rdpa_cpu_rx_reason_udef_7 && reason != rdpa_cpu_rx_reason_ingqos)
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
#if defined(CONFIG_BCM_DSL_RDP)
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
    bdmf_index idx;
    rdpa_cpu_meter_cfg_t meter_cfg;
    rdpa_cpu_reason_index_t reason_cfg_idx = {};
    rdpa_cpu_reason_cfg_t reason_cfg = {};
    int rc = 0;

    idx = METER_INDEX;
    
    meter_cfg.sir = MAX_RECOMENDED_RATE;
    meter_cfg.burst_size = MAX_RECOMENDED_BURST_SIZE;
        
    rc = rdpa_cpu_meter_cfg_set(rdpa_cpu_obj, idx , &meter_cfg);
    if (rc < 0)
    {
        enet_err("Error(%d) Meter CFG Set\n", rc);
        goto exit;
    }

#ifndef XRDP
    reason_cfg_idx.dir = rdpa_dir_us;
#endif
    reason_cfg_idx.reason = rdpa_cpu_rx_reason_ip_flow_miss;
    
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
    rdpa_cpu_reason_index_t reason_cfg_idx = {};
    rdpa_cpu_reason_cfg_t reason_cfg = {};
    int rc = 0;

#ifndef XRDP
    reason_cfg_idx.dir = rdpa_dir_us;
#endif
    reason_cfg_idx.reason = rdpa_cpu_rx_reason_ip_flow_miss;
    
    reason_cfg.meter = BDMF_INDEX_UNASSIGNED;
    reason_cfg.queue = RDPA_CPU_QUEUE_LOW;

    rc = rdpa_cpu_reason_cfg_set(rdpa_cpu_obj, &reason_cfg_idx, &reason_cfg);
    if (rc < 0)
        enet_err("Error(%d) configuring CPU reason to meter\n", rc);

    return rc;
}

#endif

int configure_bc_rate_limit_meter(void *enet_priv, unsigned int rate_limit)
{
    bdmf_object_handle rdpa_cpu_obj = NULL, rdpa_port_obj = enet_priv;
    bdmf_index idx;
    rdpa_cpu_meter_cfg_t meter_cfg;
    int rc = 0;

    if ((rc = rdpa_cpu_get(rdpa_cpu_host, &rdpa_cpu_obj)) < 0)
        goto exit;

    idx = BC_RATE_LIMIT_METER_INDEX;
    if (BC_RATE_LIMIT_DISABLE != rate_limit)
    {
        meter_cfg.sir = rate_limit;
        meter_cfg.burst_size = BC_MAX_RECOMENDED_BURST_SIZE;
        if ((rc = rdpa_cpu_meter_cfg_set(rdpa_cpu_obj, idx, &meter_cfg)) < 0)
        {
            enet_err("Error Meter CFG Set!\n");
            goto exit;
        }
    }

    rc = rdpa_port_cpu_meter_set(rdpa_port_obj, rdpa_port_meter_bcast, idx);
    if (rc)
    {
        enet_err("Could not set broadcast cpu meter for port object, meter=%d, rc=%d\n",
            (int)idx, rc);
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
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
    chan.rx_q_count = 3;
    chan.rx_q[2] = RDPA_CPU_QUEUE_HW_FIREWALL;
#endif
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
    bdmf_object_handle cpu_obj=NULL, system_obj=NULL;
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
        info.data_offset += sizeof(flow_key);
        info.size -= sizeof(flow_key);
        
        flow_key = *((uint32_t*)info.data);
    }
#endif    

#if defined(CONFIG_BCM_PTP_1588) && defined(XRDP)
    /* In 1588 packets, TS(32 bit) is appended to the begining of the packet by the XRDP,
     * so the data_offset pointer should be forwarded by 32bit and packet length decreased by 32bit */
    if (unlikely(info.reason == rdpa_cpu_rx_reason_etype_ptp_1588))
    {
        info.data_offset += PTP_RX_TS_LEN;
        info.size -= PTP_RX_TS_LEN;
        info.ptp_index = 1;
    }
#endif

    *fkb = fkb_init((uint8_t *)info.data , BCM_PKT_HEADROOM, (uint8_t *)(info.data + info.data_offset), info.size);
    (*fkb)->recycle_hook = (RecycleFuncP)bdmf_sysb_recycle;

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
    if (flow_key) 
        rx_info->fc_ctxt = flow_key;
#endif
    
    /* Parameters required by enet */
    rx_info->src_port = info.handle;
    rx_info->flow_id = info.reason_data;
    rx_info->ptp_index = info.ptp_index;
    rx_info->data_offset = info.data_offset;
    rx_info->reason = info.reason;
    rx_info->extra_skb_flags = 0;
    rx_info->is_exception = info.is_exception;
    rx_info->is_group_fwd_exception = (info.reason == rdpa_cpu_rx_reason_group_exception) ? 1 : 0;

#if defined(CONFIG_BCM_CSO)
    (*fkb)->rx_csum_verified = info.rx_csum_verified;
#endif

    if (unlikely(enet_dump_rx))
        rdpa_cpu_rx_dump_packet("enet", rdpa_cpu_host, hw_q_id, &info, 0);

    return 0;
}

void enetxapi_fkb_databuf_recycle(FkBuff_t *fkb)
{
    ETH_GBPM_TRACK_FKB(fkb, GBPM_VAL_RECYCLE, 0);
    bdmf_sysb_databuf_recycle(fkb);
}

void enetxapi_buf_recycle(struct sk_buff *skb, uint32_t context, uint32_t flags)
{
    ETH_GBPM_TRACK_SKB(skb, GBPM_VAL_RECYCLE, 0);
    bdmf_sysb_recycle(skb, context, flags);
}

inline int _rdpa_cpu_send_sysb(bdmf_sysb sysb, rdpa_cpu_tx_info_t *info)
{
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    spdsvcHook_transmit_t spdsvc_transmit = {};
#endif
    int rc;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    spdsvc_transmit.pNBuff = sysb;
    spdsvc_transmit.dev = NULL;
    spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
    spdsvc_transmit.phy_overhead = BCM_ENET_OVERHEAD;
    spdsvc_transmit.so_mark = 0;
    spdsvc_transmit.egress_type = SPDSVC_EGRESS_TYPE_ENET;
    spdsvc_transmit.transmit_helper = NULL;
    if (enet_spdsvc_transmit)
    {
        rc = enet_spdsvc_transmit(&spdsvc_transmit);
        /* In case of error, NBuff will be free by spdsvc */
        if (unlikely(rc < 0))
            return -1;
        info->bits.is_spdsvc_setup_packet = rc;
    }
    else
        info->bits.is_spdsvc_setup_packet = 0;

    info->spdt_so_mark = spdsvc_transmit.so_mark;
#endif

    rc = rdpa_cpu_send_sysb(sysb, info);

    return rc;
}

#ifdef XRDP
#define QUEUE_THRESHOLD 128*1536 /* Drop threshold in bytes */
#else
#define QUEUE_THRESHOLD 128 /* TODO: platform define */
#endif

#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define rdpa_get_queue_idx(is_wan, qid, q_count)  qid
#else
static int rdpa_get_queue_idx(int is_wan, int qid, int q_count)
{
#if defined(DSL_DEVICES)
    if (is_wan)
    {
        return qid;
    }
    return (q_count - qid -1);
#else
    return qid;
#endif
}
#endif

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
    bdmf_error_t rc = -1;
    BDMF_MATTR_ALLOC(tm_attr, rdpa_egress_tm_drv());
    bdmf_object_handle tm_obj = NULL;
    rdpa_port_tm_cfg_t port_tm_cfg;
    char port_name[IFNAMSIZ];
    bdmf_boolean is_wan;
    rdpa_port_type type;    

    if ((rc = rdpa_port_name_get(port_obj, port_name, IFNAMSIZ)))
    {
        enet_err("Failed to get RDPA port name. rc=%d\n", rc);
        goto Exit;
    }
   
    if ((rc = rdpa_port_is_wan_get(port_obj, &is_wan)))
    {
        enet_err("Failed to get RDPA port is_wan. rc=%d\n", rc);
        goto Exit;
    }

    if ((rc = rdpa_port_type_get(port_obj, &type)))
    {
        enet_err("Failed to get RDPA port is_wan. rc=%d\n", rc);
        goto Exit;
    }

    rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_queue);
    rdpa_egress_tm_mode_set(tm_attr, rdpa_tm_sched_sp);

    if ((rc = bdmf_new_and_set(rdpa_egress_tm_drv(), port_obj, tm_attr, &tm_obj)))
    {
        enet_err("Failed to create RDPA egress tm for port %s. rc=%d\n", port_name, rc);
        goto Exit;
    }

    if ((rc = rdpa_egress_tm_queues_cfg(tm_obj, is_wan))) 
    {
        enet_err("Failed to configure RDPA egress tm queues for port %s. rc=%d\n", port_name, rc);
        goto Exit;
    }

    if ((rc = rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg)))
    {
        enet_err("Failed to get RDPA egress tm for port %s. rc=%d\n", port_name, rc);
        goto Exit;
    }

    port_tm_cfg.sched = tm_obj;

    if ((rc = rdpa_port_tm_cfg_set(port_obj, &port_tm_cfg)))
    {
        enet_err("Failed to set RDPA egress tm for port %s. rc=%d\n", port_name, rc);
        goto Exit;
    }

    enet_dbg("Created RDPA egress tm %s\n", bdmf_object_name(tm_obj));

Exit:
    if (rc && tm_obj)
        bdmf_destroy(tm_obj);

    BDMF_MATTR_FREE(tm_attr);
    return rc;
}

static int port_ingress_filters_set(bdmf_object_handle port_obj, rdpa_filter *filters, int num_of_filters)
{
    int count, rc;
    rdpa_filter_ctrl_t filter_ctrl;
    uint8_t filter;


    for (filter = RDPA_FILTERS_BEGIN; filter < RDPA_FILTERS_QUANT; filter++)
    {
        filter_ctrl.enabled = 0;
        for (count = 0 ; count < num_of_filters; count++)
        {
            if (filter == filters[count])
            {
                filter_ctrl.enabled = 1;
                break;
            }
        }

        filter_ctrl.action = rdpa_forward_action_host;
        if ((rc = rdpa_port_ingress_filter_set(port_obj, filter, &filter_ctrl)))
        {
            enet_err("Failed to set ingress filter %d. rc=%d\n", filter, rc);
            return rc;
        }
    }

    return 0;
}

int runner_default_filter_init(bdmf_object_handle port_obj, rdpa_filters_group filters_group)
{
    int rc = 0;

    switch (filters_group)
    {
    case RDPA_FILTERS_GROUP_LAN:
        rc = port_ingress_filters_set(port_obj, (rdpa_filter *)default_lan_ingress_filters,
            ARRAY_SIZE(default_lan_ingress_filters));
        break;
    case RDPA_FILTERS_GROUP_WAN_GBE:
        rc = port_ingress_filters_set(port_obj, (rdpa_filter *)gbe_default_extra_filters,
            ARRAY_SIZE(gbe_default_extra_filters));
        if (rc)
            break;
        __attribute__ ((__fallthrough__));
    case RDPA_FILTERS_GROUP_WAN:
        rc = port_ingress_filters_set(port_obj, (rdpa_filter *)default_wan_filters, ARRAY_SIZE(default_wan_filters));
        break;
    default:
        rc = -1;
        break;
    }
    if (rc)
        enet_err("Failed to set default ingress filters for filters group %d. rc=%d\n", filters_group, rc);

    return rc;
}

int _rdpa_port_def_flow_trap_set(bdmf_object_handle port_obj, int port_id, rdpa_port_type port_type)
{
    rdpa_ic_result_t def_flow = { .action = rdpa_forward_action_host, .disable_stat = 1, };
    int rc;

    /* Trap to host default traffic, not use stat counter for this flow */
    rc = rdpa_port_def_flow_set(port_obj, &def_flow);
    if (rc)
        enet_err("Failed to set RDPA port default flow for port (id %d, type %d). rc=%d\n", port_id, port_type, rc);

    return rc;
}

bdmf_object_handle create_rdpa_port(enetx_port_t *self, net_device_handle_t handle, 
    bdmf_object_handle owner,
    int create_egress_tm, int enable_set, int create_ingress_filters)
{
    bdmf_error_t rc;
    BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());
    bdmf_object_handle cpu_obj = NULL;
    bdmf_object_handle port_obj = NULL;
    rdpa_port_dp_cfg_t port_cfg = {};
    rdpa_filters_group filters_group = RDPA_FILTERS_GROUP_LAN;
    rdpa_port_interface_t rdpa_port_interface = {};

    rdpa_port_interface.index = self->p.port_id;
    rdpa_port_interface.type = port_runner_port_type_mapping(self->port_type);
    if (rdpa_port_interface.type == rdpa_port_type_none)
    {
        BDMF_MATTR_FREE(rdpa_port_attrs);
        return NULL;
    }
    rdpa_port_interface.is_wan = self->port_info.is_wan;    
    rdpa_port_interface.handle = handle;

    enet_dbgv("(type=%x, index=%d, owner=%px)\n", rdpa_port_interface.type, rdpa_port_interface.index, (void *)owner);

    if ((rc = rdpa_port_type_set(rdpa_port_attrs, rdpa_port_interface.type)))
    {
        enet_err("Failed to set RDPA port type %d. rc=%d\n", rdpa_port_interface.type, rc);
        port_obj = NULL;
        goto Exit;
    }

    if ((rc = rdpa_port_index_set(rdpa_port_attrs, rdpa_port_interface.index)))
     {
        enet_err("Failed to set RDPA port index %d. rc=%d\n", rdpa_port_interface.index, rc);
        port_obj = NULL;
        goto Exit;
    }

    if ((rc = rdpa_port_is_wan_set(rdpa_port_attrs, rdpa_port_interface.is_wan)))
     {
        enet_err("Failed to set RDPA port is_wan %d. rc=%d\n", rdpa_port_interface.is_wan, rc);
        port_obj = NULL;
        goto Exit;
    }

    if ((rc = rdpa_port_handle_set(rdpa_port_attrs, rdpa_port_interface.handle)))
    {
        enet_err("Failed to set RDPA port handle %d. rc=%d\n", rdpa_port_interface.handle, rc);
        port_obj = NULL;
        goto Exit;
    }

    if (rdpa_port_interface.type != rdpa_port_emac)
        goto skip_emac;

#if defined(CONFIG_BCM_PON_XRDP) && !defined(CONFIG_ONU_TYPE_SFU) 
    if (rdpa_port_interface.is_wan)
    {
        filters_group = RDPA_FILTERS_GROUP_WAN_GBE;
        port_cfg.sal_enable = 0;
        port_cfg.dal_enable = 0;
    }
    else
#endif
    {
#ifndef CONFIG_BCM_FTTDP_G9991
        port_cfg.sal_enable = 1;
        port_cfg.dal_enable = 1;
        port_cfg.sal_miss_action = rdpa_forward_action_host;
        port_cfg.dal_miss_action = rdpa_forward_action_host;
#endif
    }

    if ((rc = rdpa_port_cfg_set(rdpa_port_attrs, &port_cfg)))
    {
        enet_err("Failed to set configuration for RDPA port %d. rc=%d\n", rdpa_port_interface.index, rc);
        goto Exit;
    }

skip_emac:
    if (enable_set)
        rdpa_port_enable_set(rdpa_port_attrs, 0);

    if ((rc = bdmf_new_and_set(rdpa_port_drv(), owner, rdpa_port_attrs, &port_obj)))
    {
        enet_err("Failed to create RDPA port %d. rc=%d\n", rdpa_port_interface.index, rc);
        goto Exit;
    }

    if (create_egress_tm && (rc = create_rdpa_egress_tm(port_obj)))
    {
        enet_err("Failed to create ergress_tm : %d\n", create_egress_tm);

        enet_err("Failed to create ergress_tm for RDPA port %d. rc=%d\n", rdpa_port_interface.index, rc);
        goto Exit;
    }

#ifdef G9991_PRV 
    if (rdpa_port_interface.type == rdpa_port_sys_port)
    {
        rc = _rdpa_port_def_flow_trap_set(port_obj, rdpa_port_interface.index, rdpa_port_interface.type);
        if (rc)
        {
            enet_err("Failed to set default flow to trap for system port, rc=%d\n", rc);
            goto Exit;
        }
    }
#endif
#ifdef XRDP
    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
    rc = rc ? rc : rdpa_port_cpu_obj_set(port_obj, cpu_obj);
    if (rc)
    {
        enet_err("Failed to set CPU object for port index %d, type %d, error %d\n", rdpa_port_interface.type,
            rdpa_port_interface.index, rc);
        goto Exit;
    }
#else
    rc = 0;
#endif

    if (create_ingress_filters)
    {
        if ((rc = runner_default_filter_init(port_obj, filters_group)))
        {
            enet_err("Failed to set up default filters for RDPA port index %d, type %d. rc=%d\n",
                rdpa_port_interface.index, rdpa_port_interface.type, rc);
            goto Exit;
        }
    }

    enet_dbgv("Created RDPA object for port %s, object type=%d, index=%d\n", self->name,
        (int)rdpa_port_interface.type, (int)rdpa_port_interface.index); 

Exit:
    if (cpu_obj)
        bdmf_put(cpu_obj);
    if (rc && port_obj)
    {
        bdmf_destroy(port_obj);
        port_obj = NULL;
    }

    BDMF_MATTR_FREE(rdpa_port_attrs);
    return port_obj;
}

int port_runner_port_uninit(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_tm_cfg_t port_tm_cfg;

    if (!port_obj)
        return 0;

    if (!rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg))
        if (port_tm_cfg.sched)
            bdmf_destroy(port_tm_cfg.sched);

    rdpa_port_uninit_set(port_obj, 1);
    bdmf_destroy(port_obj);
    self->priv = 0;

    mux_set_rx_index(self->p.parent_sw, self->p.port_id, NULL);

    blog_chnl_unit_port_unset(self);

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

int port_runner_sw_init(enetx_port_t *self)
{
#ifdef CONFIG_BCM_PTP_1588
    if (ptp_1588_init())
        return -1;
#endif

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    enet_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
    if (NULL == enet_spdsvc_transmit)
        enet_err("no speed test tx function available\n");
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

int port_runner_sw_config_trunk(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    /* based on impl5\bcmsw_runner.c:bcm_enet_rdp_config_bond() */
    bdmf_object_handle port_obj = NULL;
    bdmf_object_handle bond_obj = NULL;
    char bond_name[IFNAMSIZ] = {};
    int rc;

    rc = rdpa_port_get(port->name, &port_obj);
    if (rc)
    {
        enet_err("NO rdpa port for %s\n\n", port->obj_name);
        return -1;
    }

    snprintf(bond_name, IFNAMSIZ, "bond%d", grp_no);

    /* get the rdpa bond port in order to link to lan ports */
    rc = rdpa_port_get(bond_name, &bond_obj);


    if (rc)
    {
        if (add)
        {
            /* Bond object does not exist - Create one */
            BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());
            rdpa_port_name_set(rdpa_port_attrs, bond_name);
            rdpa_port_type_set(rdpa_port_attrs, rdpa_port_bond);
            rdpa_port_index_set(rdpa_port_attrs, grp_no);
            rdpa_port_is_wan_set(rdpa_port_attrs, 0);

            rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &bond_obj);
            BDMF_MATTR_FREE(rdpa_port_attrs);
            if (rc)
            {
                enet_err("Failed to create bond port rc(%d)\n",  rc);
                goto error_exit;
            }
        }
        else
        {
            enet_err("No rdpa bond for grp_no=%d %s\n\n", grp_no, port->obj_name);
            goto error_exit;
        }
    }

    if (add)
    {
        /* Link the port with bond object */
        rc = bdmf_link(bond_obj, port_obj, NULL);
        if (rc)
        {
            enet_err("Failed to link bond port for grp_no=%d %s\n", grp_no, port->obj_name);
            goto error_exit;
        }
    }
    else
    {
        /* UnLink the port from bond object */
        rc = bdmf_unlink(bond_obj, port_obj);
        if (rc)
        {
            enet_err("Failed to unlink bond port for grp_no=%d %s\n", grp_no, port->obj_name);
            goto error_exit;
        }

        if (bdmf_get_next_us_link(bond_obj, NULL) == NULL)
        {
            /* No More linked objects to this bond object - destroy */
            if ( bdmf_destroy(bond_obj) )
            {
                enet_err("Failed to destroy bond port for grp_no=%d %s\n", grp_no, port->obj_name);
            }
            else
            {
                bond_obj = NULL;
            }
        }
    }

error_exit:
    if (port_obj)
    {
        bdmf_put(port_obj);
    }
    if (bond_obj)
    {
        bdmf_put(bond_obj);
    }
    return rc;
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
        /* Supress error - error valid when TM is not configured for the port */
        enet_dbg("can not get stat port %s %s\n", self->obj_name, self->dev->name);
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

int port_runner_wan_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    int rc, is_wan;
#if defined(CONFIG_BCM_PON)
    rdpa_port_dp_cfg_t cfg;
#endif

    self->n.port_netdev_role = role;
    is_wan = self->n.port_netdev_role == PORT_NETDEV_ROLE_WAN;

    if ((rc = rdpa_port_is_wan_set(self->priv, is_wan)))
    {
        enet_err("Failed to set RDPA port is_wan for port %s. rc=%d\n", self->obj_name, rc);
        goto Exit;
    }
#if defined(CONFIG_BCM_PON)
    if (is_wan)
    {
        if ((rc = rdpa_port_cfg_get(self->priv , &cfg)))
        {
            enet_err("Failed to get RDPA port configuration for port %s. rc=%d\n", self->obj_name, rc);
            goto Exit;
        }
        
        cfg.dal_enable = 0;
        cfg.sal_enable = 0;

        if ((rc = rdpa_port_cfg_set(self->priv , &cfg)))
        {
            enet_err("Failed to set RDPA port configuration for port %s. rc=%d\n", self->obj_name, rc);
            goto Exit;
        }
    }
    else
    {
#ifndef CONFIG_BCM_FTTDP_G9991
        if ((rc = rdpa_port_cfg_get(self->priv , &cfg)))
        {
            enet_err("Failed to get RDPA port configuration for port %s. rc=%d\n", self->obj_name, rc);
            goto Exit;
        }

        cfg.sal_enable = 1;
        cfg.dal_enable = 1;

        if ((rc = rdpa_port_cfg_set(self->priv , &cfg)))
        {
            enet_err("Failed to set RDPA port configuration for port %s. rc=%d\n", self->obj_name, rc);
            goto Exit;
        }
#endif        
    }
    if ((rc = runner_default_filter_init(self->priv, is_wan ? RDPA_FILTERS_GROUP_WAN_GBE : RDPA_FILTERS_GROUP_LAN)))
        enet_err("Error settings default filters for port %s. rc=%d\n", self->obj_name, rc);
#endif

Exit:
    return rc;
}

/* mib dump for ports on internal runner switch */
int port_runner_mib_dump(enetx_port_t *self, int all)
{
    /* based on impl5\bcmsw_runner.c bcmeapi_ethsw_dump_mib() */
    mac_stats_t         *mac_stats = &(self->p.mac->stats);
    int                 port = self->port_info.port;
    uint64_t            errcnt = 0;

    if (mac_dev_stats_get(self->p.mac, mac_stats))
        return -1;

    printk("\nRunner Stats : Port# %d\n",port);

    /* Display Tx statistics */
     /* Display Tx statistics */
    printk("\n");
    printk("TxUnicastPkts:          %10llu \n", mac_stats->tx_unicast_packet);
    printk("TxMulticastPkts:        %10llu \n", mac_stats->tx_multicast_packet);
    printk("TxBroadcastPkts:        %10llu \n", mac_stats->tx_broadcast_packet);
    printk("TxDropPkts:             %10llu \n", mac_stats->tx_error);

    /* Display remaining tx stats only if requested */
    if (all) {
        printk("TxBytes:                %10llu \n", mac_stats->tx_byte);
        printk("TxFragments:            %10llu \n", mac_stats->tx_fragments_frame);
        printk("TxCol:                  %10llu \n", mac_stats->tx_total_collision);
        printk("TxSingleCol:            %10llu \n", mac_stats->tx_single_collision);
        printk("TxMultipleCol:          %10llu \n", mac_stats->tx_multiple_collision);
        printk("TxDeferredTx:           %10llu \n", mac_stats->tx_deferral_packet);
        printk("TxLateCol:              %10llu \n", mac_stats->tx_late_collision);
        printk("TxExcessiveCol:         %10llu \n", mac_stats->tx_excessive_collision);
        printk("TxPausePkts:            %10llu \n", mac_stats->tx_pause_control_frame);
        printk("TxExcessivePkts:        %10llu \n", mac_stats->tx_excessive_deferral_packet);
        printk("TxJabberFrames:         %10llu \n", mac_stats->tx_jabber_frame);
        printk("TxFcsError:             %10llu \n", mac_stats->tx_fcs_error);
        printk("TxCtrlFrames:           %10llu \n", mac_stats->tx_control_frame);
        printk("TxOverSzFrames:         %10llu \n", mac_stats->tx_oversize_frame);
        printk("TxUnderSzFrames:        %10llu \n", mac_stats->tx_undersize_frame);
        printk("TxUnderrun:             %10llu \n", mac_stats->tx_underrun);
        printk("TxPkts64Octets:         %10llu \n", mac_stats->tx_frame_64);
        printk("TxPkts65to127Octets:    %10llu \n", mac_stats->tx_frame_65_127);
        printk("TxPkts128to255Octets:   %10llu \n", mac_stats->tx_frame_128_255);
        printk("TxPkts256to511Octets:   %10llu \n", mac_stats->tx_frame_256_511);
        printk("TxPkts512to1023Octets:  %10llu \n", mac_stats->tx_frame_512_1023);
        printk("TxPkts1024to1518Octets: %10llu \n", mac_stats->tx_frame_1024_1518);
        if (self->p.mac->mac_drv->mac_type == MAC_TYPE_UNIMAC)
            printk("TxPkts1519to9216Octets: %10llu \n", mac_stats->tx_frame_1519_mtu);
        else
            printk("TxPkts1519toMTU:        %10llu \n", mac_stats->tx_frame_1519_mtu);
    }
    else {
        errcnt = 0;
        errcnt += mac_stats->tx_total_collision;
        errcnt += mac_stats->tx_single_collision;
        errcnt += mac_stats->tx_multiple_collision;
        errcnt += mac_stats->tx_deferral_packet;
        errcnt += mac_stats->tx_late_collision;
        errcnt += mac_stats->tx_excessive_collision;
        errcnt += mac_stats->tx_excessive_deferral_packet;
        errcnt += mac_stats->tx_jabber_frame;
        errcnt += mac_stats->tx_fcs_error;
        errcnt += mac_stats->tx_undersize_frame;
        errcnt += mac_stats->tx_underrun;
        printk("TxOtherErrors:          %10llu \n", errcnt);
    }

    /* Display Rx statistics */
    printk("\n");
    printk("RxUnicastPkts:          %10llu \n", mac_stats->rx_unicast_packet);
    printk("RxMulticastPkts:        %10llu \n", mac_stats->rx_multicast_packet);
    printk("RxBroadcastPkts:        %10llu \n", mac_stats->rx_broadcast_packet);

    /* Display remaining rx stats only if requested */
    if (all) {
        printk("RxBytes:                %10llu \n", mac_stats->rx_byte);
        printk("RxJabbers:              %10llu \n", mac_stats->rx_jabber);
        printk("RxAlignErrs:            %10llu \n", mac_stats->rx_alignment_error);
        printk("RxFCSErrs:              %10llu \n", mac_stats->rx_fcs_error);
        printk("RxFragments:            %10llu \n", mac_stats->rx_fragments);
        printk("RxOversizePkts:         %10llu \n", mac_stats->rx_oversize_packet);
        printk("RxUndersizePkts:        %10llu \n", mac_stats->rx_undersize_packet);
        printk("RxPausePkts:            %10llu \n", mac_stats->rx_pause_control_frame);
        printk("RxOverflow:             %10llu \n", mac_stats->rx_overflow);
        printk("RxCtrlPkts:             %10llu \n", mac_stats->rx_control_frame);
        printk("RxUnknownOp:            %10llu \n", mac_stats->rx_unknown_opcode);
        printk("RxLenError:             %10llu \n", mac_stats->rx_frame_length_error);
        printk("RxCodeError:            %10llu \n", mac_stats->rx_code_error);
        printk("RxCarrierSenseErr:      %10llu \n", mac_stats->rx_carrier_sense_error);
        printk("RxPkts64Octets:         %10llu \n", mac_stats->rx_frame_64);
        printk("RxPkts65to127Octets:    %10llu \n", mac_stats->rx_frame_65_127);
        printk("RxPkts128to255Octets:   %10llu \n", mac_stats->rx_frame_128_255);
        printk("RxPkts256to511Octets:   %10llu \n", mac_stats->rx_frame_256_511);
        printk("RxPkts512to1023Octets:  %10llu \n", mac_stats->rx_frame_512_1023);
        printk("RxPkts1024to1518Octets: %10llu \n", mac_stats->rx_frame_1024_1518);
        if (self->p.mac->mac_drv->mac_type == MAC_TYPE_UNIMAC)
            printk("RxPkts1519to9216Octets: %10llu \n", mac_stats->rx_frame_1519_mtu);
        else
            printk("RxPkts1519toMTU:        %10llu \n", mac_stats->rx_frame_1519_mtu);
    }
    else {
        errcnt = 0;
        errcnt += mac_stats->rx_jabber;
        errcnt += mac_stats->rx_alignment_error;
        errcnt += mac_stats->rx_fcs_error;
        errcnt += mac_stats->rx_oversize_packet;
        errcnt += mac_stats->rx_undersize_packet;
        errcnt += mac_stats->rx_overflow;
        errcnt += mac_stats->rx_unknown_opcode;
        errcnt += mac_stats->rx_frame_length_error;
        errcnt += mac_stats->rx_code_error;
        errcnt += mac_stats->rx_carrier_sense_error;
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

    *port_id = port_info->port;

    if (port_info->is_gpon)
    {
        *port_type = PORT_TYPE_RUNNER_GPON;
        return 0;
    }

    if (port_info->is_epon || port_info->is_epon_ae)
    {
        *port_type = PORT_TYPE_RUNNER_EPON;
        return 0;
    }

    *port_type = PORT_TYPE_RUNNER_PORT;
    if (port_info->is_undef)
    {
        *port_type = PORT_TYPE_RUNNER_MAC;
        *port_id = 0;
        return 0;
    }

    if (port_info->is_management)
    {
        *port_type = PORT_TYPE_G9991_SYS_PORT;
        return 0;
    }

    if (port_info->is_attached)
    {
        /* Is this an emac to which SIDs are attached? */
#if !defined(CONFIG_BCM_FTTDP_G9991)
        *port_type = PORT_TYPE_RUNNER_MAC;
        port_info->is_undef = 1;
#else
        *port_type = PORT_TYPE_RUNNER_PORT;
#endif
    }

    return 0;
}

int port_runner_mtu_set(enetx_port_t *self, int mtu)
{
    bdmf_object_handle port_obj = self->priv;
    bdmf_number old_max_pkt_size;
    int rc;

    rdpa_port_max_pkt_size_get(port_obj, &old_max_pkt_size);
    if ((rc = rdpa_port_max_pkt_size_set(port_obj, mtu + ENET_MAX_MTU_EXTRA_SIZE)))
    {
        enet_err("failed to set rdpa max packet size %d on %s\n", mtu, self->obj_name);
        return -1;
    }

    if (port_generic_mtu_set(self, mtu))
    {
        /* Rollback */
        rdpa_port_max_pkt_size_set(port_obj, old_max_pkt_size);
        return -1;
    }

    return 0;
}

rdpa_port_type port_runner_port_type_mapping(port_type_t port_type)
{
    switch(port_type)
    {
        case PORT_TYPE_RUNNER_PORT:
            return rdpa_port_emac;
        case PORT_TYPE_G9991_PORT:
        case PORT_TYPE_G9991_ES_PORT:
            return rdpa_port_sid;
        case PORT_TYPE_G9991_CTRL_PORT:
            return rdpa_port_ctrl_sid;
        case PORT_TYPE_G9991_SYS_PORT:
            return rdpa_port_sys_port;
        case PORT_TYPE_SF2_PORT:
            return rdpa_port_sf2_emac;
        default:
            enet_err("[%s] Illegal port type %d\n", __FUNCTION__, port_type);
            break;
    }
    return rdpa_port_type_none; 
}

int _rdpa_port_name_set(enetx_port_t *self, char *ovr_name)
{
    int rc;
    bdmf_object_handle port_obj = self->priv;
    char *name = ovr_name;
   
    if (!port_obj)
        return 0;

    if (!name)
        name = self->dev ? self->name : self->obj_name;

    if ((rc = rdpa_port_name_set(port_obj, name)))
        enet_err("Failed to set RDPA port name %s. rc=%d\n", name, rc);

    return rc;
}

int port_runner_port_post_init(enetx_port_t *self)
{
    return _rdpa_port_name_set(self, NULL);
}

void port_runner_print_status(enetx_port_t *self)
{
    pr_cont("%s ", self->dev->name);
    phy_dev_print_status(self->p.phy);
}

int port_runner_link_change(enetx_port_t *self, int up)
{
    return rdpa_port_enable_set(self->priv, up);
}

int enetxapi_rx_pkt_dump_on_demux_err(enetx_rx_info_t *rx_info)
{
    if (rx_info->src_port == RDPA_PORT_CPU_HANDLE)
    {
        /* rdpa port/name=cpuXXX is not linked to any network device in enet, but packets can come if generated by
         * CPU/Runner pktgen and there is no available flow. In that case, we don't want to dump an error. */
        return 0;
    }
    return 1;
}

#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96855)
// ----------- SIOCETHSWCTLOPS ETHSWDOSCTRL functions ---
int _runner_rdpa_dos_ctrl(struct ethswctl_data *e)
{
    bdmf_object_handle system_obj;
    rdpa_parser_cfg_t parser_cfg;
    bdmf_number dos_attack_reason;
    int rc;

    enet_dbg("_runner_rdpa_dos_ctrl e->type=%s\n", (e->type == TYPE_SET) ? "SET" : "GET");

    if ((rc = rdpa_system_get(&system_obj)))
    {
        enet_err("Failed to get RDPA System object\n");
        goto exit;
    }
    if ((rc = rdpa_system_parser_cfg_get(system_obj, &parser_cfg)))
    {
        enet_err("Failed to get RDPA System parser configure, rc=%d\n", rc);
        goto exit;
    }
    if ((rc = rdpa_system_dos_attack_reason_get(system_obj, &dos_attack_reason)))
    {
        enet_err("Failed to get RDPA System dos attack reason configure, rc=%d\n", rc);
        goto exit;
    }    

    if (e->type == TYPE_GET)
    {
        if (dos_attack_reason & (1 << rdpa_dos_reason_mac_sa_eq_da))  e->dosCtrl.da_eq_sa_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_ip_land))  e->dosCtrl.ip_lan_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_tcp_blat))  e->dosCtrl.tcp_blat_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_udp_blat))  e->dosCtrl.udp_blat_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_tcp_null_scan))  e->dosCtrl.tcp_null_scan_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_tcp_xmas_scan))  e->dosCtrl.tcp_xmas_scan_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_tcp_synfin_scan))  e->dosCtrl.tcp_synfin_scan_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_tcp_syn_error))  e->dosCtrl.tcp_synerr_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_tcp_short_hdr))  e->dosCtrl.tcp_shorthdr_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_tcp_frag_error))  e->dosCtrl.tcp_fragerr_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_icmpv4_fragment))  e->dosCtrl.icmpv4_frag_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_icmpv6_fragment))  e->dosCtrl.icmpv6_frag_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_icmpv4_long_ping))  e->dosCtrl.icmpv4_longping_drop_en = 1;
        if (dos_attack_reason & (1 << rdpa_dos_reason_icmpv6_long_ping))  e->dosCtrl.icmpv6_longping_drop_en = 1;
    }
    else if (e->type == TYPE_SET)
    {
        dos_attack_reason = 0;
        if (e->dosCtrl.da_eq_sa_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_mac_sa_eq_da);
        if (e->dosCtrl.ip_lan_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_ip_land);
        if (e->dosCtrl.tcp_blat_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_tcp_blat);
        if (e->dosCtrl.udp_blat_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_udp_blat);
        if (e->dosCtrl.tcp_null_scan_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_tcp_null_scan);
        if (e->dosCtrl.tcp_xmas_scan_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_tcp_xmas_scan);
        if (e->dosCtrl.tcp_synfin_scan_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_tcp_synfin_scan);
        if (e->dosCtrl.tcp_synerr_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_tcp_syn_error);
        if (e->dosCtrl.tcp_shorthdr_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_tcp_short_hdr);
        if (e->dosCtrl.tcp_fragerr_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_tcp_frag_error);
        if (e->dosCtrl.icmpv4_frag_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_icmpv4_fragment);
        if (e->dosCtrl.icmpv6_frag_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_icmpv6_fragment);
        if (e->dosCtrl.icmpv4_longping_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_icmpv4_long_ping);
        if (e->dosCtrl.icmpv6_longping_drop_en) dos_attack_reason |= (1 << rdpa_dos_reason_icmpv6_long_ping);

        printk("SET dos_val 0x%04x \n", (uint16_t)dos_attack_reason);
        rc = rdpa_system_dos_attack_reason_set(system_obj, dos_attack_reason);
        if (rc)
        {   
            enet_err("Failed to set dos attack reason, v=0x%04x, rc=%d\n", (uint16_t)dos_attack_reason, rc);
            goto exit;
        }
    }

exit:
    if (system_obj)
        bdmf_put(system_obj);
    
    return rc ? -EFAULT : BCM_E_NONE;
}
#endif


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

