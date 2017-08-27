#ifndef __XRDP_WFD_INLINE_H_INCLUDED__
#define __XRDP_WFD_INLINE_H_INCLUDED__

/*
<:copyright-BRCM:2016:DUAL/GPL:standard 

   Copyright (c) 2016 Broadcom 
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

Author: kosta.sopov@broadcom.com
*/

#include "rdpa_api.h"
#include "rdpa_mw_blog_parse.h"
#include "rdp_cpu_ring_defs.h"
#include "rdp_mm.h"
#include "linux/prefetch.h"
#include "rdp_cpu_ring.h"
#include "rdpa_port_int.h"

typedef struct {
    int qid;
    int wfd_idx;
} wfd_rx_isr_context_t;

typedef union {
    struct {
        uint16_t flowring_idx:10;
        uint16_t tx_prio:3;
        uint16_t reserved:3;
    };
    uint16_t hword;
} wl_metadata_dongle_t;

#define WFD_WLAN_QUEUE_MAX_SIZE (RDPA_CPU_WLAN_QUEUE_MAX_SIZE)

static inline void map_ssid_vector_to_ssid_index(uint16_t *bridge_port_ssid_vector, uint32_t *wifi_drv_ssid_index)
{
   *wifi_drv_ssid_index = __ffs(*bridge_port_ssid_vector);
}

static inline void wfd_dev_rx_isr_callback(long priv)
{
    wfd_rx_isr_context_t *ctx = (wfd_rx_isr_context_t *)priv;

    /* Disable PCI interrupt */
    rdpa_cpu_int_disable(rdpa_cpu_wlan0 + wfd_objects[ctx->wfd_idx].wl_radio_idx, ctx->qid);
    rdpa_cpu_int_clear(rdpa_cpu_wlan0 +  wfd_objects[ctx->wfd_idx].wl_radio_idx, ctx->qid);

    /*Atomically set the queue bit on*/
    set_bit(ctx->qid, &wfd_objects[ctx->wfd_idx].wfd_rx_work_avail);

    /* Call the RDPA receiving packets handler (thread or tasklet) */
    WFD_WAKEUP_RXWORKER(ctx->wfd_idx);
}

static inline int wfd_get_minQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }

    return 0;
}

static inline int wfd_get_maxQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }
    return WFD_NUM_QUEUES_PER_WFD_INST - 1;
}

static inline int wfd_config_rx_queue(int wfd_idx, int qid, uint32_t qsize,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg;
    bdmf_object_handle rdpa_cpu_obj;
    uint32_t *ring_base = NULL;
    int rc = 0;
    bdmf_sysb_type qsysb_type = bdmf_sysb_skb;

    if (eFwdHookType == WFD_WL_FWD_HOOKTYPE_FKB)
    {
        qsysb_type = bdmf_sysb_fkb;
    }

    if (rdpa_cpu_get(rdpa_cpu_wlan0 + wfd_objects[wfd_idx].wl_radio_idx, &rdpa_cpu_obj))
        return -1;

    /* Read current configuration, set new drop threshold and ISR and write back. */
    bdmf_lock();
    rc = rdpa_cpu_rxq_cfg_get(rdpa_cpu_obj, qid, &rxq_cfg);
    if (rc)
        goto unlock_exit;

    if (qsize)
    {
        wfd_rx_isr_context_t *isr_ctx = (wfd_rx_isr_context_t *)kmalloc(GFP_KERNEL, sizeof(wfd_rx_isr_context_t));

        isr_ctx->qid = qid;
        isr_ctx->wfd_idx = wfd_idx;
        rxq_cfg.isr_priv = (long)isr_ctx;
    }
    else
    {
        kfree((wfd_rx_isr_context_t *)rxq_cfg.isr_priv);
        rxq_cfg.isr_priv = 0;
    }

    rxq_cfg.size = qsize ? WFD_WLAN_QUEUE_MAX_SIZE : 0;
    rxq_cfg.rx_isr = qsize ? wfd_dev_rx_isr_callback : 0;
    rxq_cfg.ring_head = ring_base;
    rxq_cfg.ic_cfg.ic_enable = qsize ? true : false;
    rxq_cfg.ic_cfg.ic_timeout_us = WFD_INTERRUPT_COALESCING_TIMEOUT_US;
    rxq_cfg.ic_cfg.ic_max_pktcnt = WFD_INTERRUPT_COALESCING_MAX_PKT_CNT;
    rxq_cfg.rxq_stat = NULL;
    rc = rdpa_cpu_rxq_cfg_set(rdpa_cpu_obj, qid, &rxq_cfg);

    if (qsize)
        rdpa_cpu_tc_to_rxq_set(rdpa_cpu_obj, qid, qid);

unlock_exit:
    bdmf_put(rdpa_cpu_obj);
    bdmf_unlock();
    return rc;
}

static void release_wfd_interfaces(void)
{
    /* TODO: implement */
}

static inline int
_wfd_bulk_fkb_get(uint32_t qid, uint32_t budget, wfd_object_t *wfd_p, void **rx_pkts, uint32_t *rx_pktcnt)
{
    FkBuff_t *fkb;
    rdpa_cpu_rx_info_t info = {};
    wl_metadata_dongle_t wl_dongle;
    int rc = 0;
    uint32_t count = 0;

    while (budget)
    {
        rc = rdpa_cpu_packet_get(rdpa_cpu_wlan0 + wfd_p->wl_radio_idx, qid, &info);
        if (unlikely(rc))
            break;

        fkb = fkb_init((uint8_t *)info.data, BCM_PKT_HEADROOM, (uint8_t*)(info.data + info.data_offset), 
                info.size);
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
        //     fkb->dirty_p = _to_dptr_from_kptr_(data + ETH_HLEN);
#endif
        fkb->recycle_hook = bdmf_sysb_recycle;
        fkb->recycle_context = 0;
        fkb->wl.ucast.dhd.is_ucast = 1;
        fkb->wl.ucast.dhd.ssid = info.dest_ssid;

        wl_dongle.hword = info.wl_metadata;
        fkb->wl.ucast.dhd.flowring_idx = wl_dongle.flowring_idx;
        fkb->wl.ucast.dhd.wl_prio = wl_dongle.tx_prio;

        rx_pkts[count] = (void *)fkb;
        count++;
        budget--;
    }
    *rx_pktcnt = count;
    return rc;
}

static inline uint32_t
wfd_bulk_fkb_get(unsigned long qid, unsigned long budget, void *priv, void **rx_pkts)
{
    int rc;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    uint32_t rx_pktcnt = 0;

    rc = _wfd_bulk_fkb_get(qid, budget, wfd_p, rx_pkts, &rx_pktcnt);

    /* First handle the fastpath bulk packets */
    if (rc && rc != BDMF_ERR_NO_MORE)
        printk("%s:%d _wfd_bulk_fkb_get failed; rc %d\n", __func__, __LINE__, rc);

    if (!rx_pktcnt)
        return 0;

    (void) wfd_p->wfd_fwdHook(rx_pktcnt, (unsigned long)rx_pkts, wfd_p->wl_radio_idx, 0);
    wfd_p->wl_chained_packets += rx_pktcnt;
    wfd_p->count_rx_queue_packets += rx_pktcnt;

    return rx_pktcnt;
}

static uint32_t
wfd_bulk_skb_get(unsigned long qid, unsigned long budget, void *priv, void **rx_pkts)
{
    /* Not supported yet */
    return 0;
}

static int wfd_accelerator_init(void)
{
    return 0;
}

static int wfd_rdpa_init(int wl_radio_idx)
{
    int rc;
    bdmf_object_handle rdpa_cpu_obj, rdpa_port_obj;
    BDMF_MATTR(cpu_wlan_attrs, rdpa_cpu_drv());
    BDMF_MATTR(rdpa_port_attrs, rdpa_port_drv());

    /* create cpu */
    rdpa_cpu_index_set(cpu_wlan_attrs, rdpa_cpu_wlan0 + wl_radio_idx);
    rdpa_cpu_num_queues_set(cpu_wlan_attrs, WFD_NUM_QUEUES_PER_WFD_INST);

    if ((rc = bdmf_new_and_set(rdpa_cpu_drv(), NULL, cpu_wlan_attrs, &rdpa_cpu_obj)))
    {
        printk("%s:%s Failed to create cpu wlan%d object rc(%d)\n", __FILE__, __FUNCTION__, rdpa_cpu_wlan0 + wl_radio_idx, rc);
        return -1;
    }

    if ((rc = rdpa_cpu_int_connect_set(rdpa_cpu_obj, true)) && rc != BDMF_ERR_ALREADY)
    {
        printk("%s:%s Failed to connect cpu interrupts rc(%d)\n", __FILE__, __FUNCTION__, rc);
        return -1;
    }

    rdpa_port_index_set(rdpa_port_attrs, rdpa_if_wlan0 + wl_radio_idx);
    rdpa_port_cpu_obj_set(rdpa_port_attrs, rdpa_cpu_obj);
    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_port_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa port object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        return -1;
    }

    return 0;
}

static void wfd_rdpa_uninit(int wl_radio_idx)
{
    bdmf_object_handle rdpa_cpu_obj, rdpa_port_obj;

    if (!rdpa_port_get(rdpa_if_wlan0 + wl_radio_idx, &rdpa_port_obj))
    {
        bdmf_put(rdpa_port_obj);
        bdmf_destroy(rdpa_port_obj);
    }

    if (!rdpa_cpu_get(rdpa_cpu_wlan0 + wl_radio_idx, &rdpa_cpu_obj))
    {
        bdmf_put(rdpa_cpu_obj);
        bdmf_destroy(rdpa_cpu_obj);
    }
}

static inline int wfd_queue_not_empty(int wl_radio_idx, long qid, int qidx)
{
    return rdpa_cpu_queue_not_empty(rdpa_cpu_wlan0 + wl_radio_idx, qid);
}

static inline void wfd_int_enable(int wl_radio_idx, long qid, int qidx)
{
    rdpa_cpu_int_enable(rdpa_cpu_wlan0 + wl_radio_idx, qid);
}

static inline void wfd_int_disable(int wl_radio_idx, long qid, int qidx)
{
    rdpa_cpu_int_disable(rdpa_cpu_wlan0 + wl_radio_idx, qid);
}

static inline void *wfd_acc_info_get(int wl_radio_idx)
{
    return rdpa_cpu_data_get(rdpa_cpu_wlan0 + wl_radio_idx);
}

static inline int wfd_get_qid(int qidx)
{
    return qidx;
}

static inline int wfd_get_objidx(int qid, int qidx)
{
    return qidx;
}
#endif /* __XRDP_WFD_INLINE_H_INCLUDED__ */
