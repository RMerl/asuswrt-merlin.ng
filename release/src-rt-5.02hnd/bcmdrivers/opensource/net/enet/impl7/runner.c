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
#include "enet.h"
#include "runner.h"
#include <rdpa_api.h>
#include <linux/of.h>
#include <linux/nbuff.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include "enet_dbg.h"
#include "runner_wifi.h"
#include "ptp_1588.h"

static int enet_dump_rx;
port_ops_t port_runner_port_wan_gbe;
static int system_is_read;

/* RDPA queue configuration */

#define RDPA_CPU_QUEUE_LOW 3
#define RDPA_CPU_QUEUE_HI 4
#define RDPA_CPU_TC_LOW 0
#define RDPA_CPU_TC_HIGH 1
#ifdef CONFIG_BCM96858
#define DEFAULT_Q_SIZE 1024
#else
#define DEFAULT_Q_SIZE 256
#endif

#define _rdpa_destroy_queue(cpu_obj, channel, q_id) _rdpa_create_queue(cpu_obj, channel, q_id, 0)

extern void bdmf_sysb_databuf_recycle(void *pBuf, unsigned context);
extern int chan_thread_handler(void *data);
rdpa_system_init_cfg_t init_cfg = {};
int wan_port_id = -1; /* initialize as invalid WAN port */

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
    case rdpa_cpu_rx_reason_etype_801_1ag_cfm:
    case rdpa_cpu_rx_reason_l4_icmp:
    case rdpa_cpu_rx_reason_icmpv6:
    case rdpa_cpu_rx_reason_igmp:
    case rdpa_cpu_rx_reason_dhcp:
    case rdpa_cpu_rx_reason_l4_udef_0:
        *queue = RDPA_CPU_QUEUE_HI;
        *tc = RDPA_CPU_TC_HIGH;
        break;
    default:
        *queue = RDPA_CPU_QUEUE_LOW;
        *tc = RDPA_CPU_TC_LOW;
        break;
    }
}

#ifdef CONFIG_BCM96858
static int _rdpa_map_reasons_2_queue(bdmf_object_handle system_obj, bdmf_object_handle cpu_obj)
{
    int rc;
    rdpa_cpu_reason reason;
    uint8_t tc;
    bdmf_index queue;
    
    for (reason = rdpa_cpu_reason_min; reason < rdpa_cpu_reason__num_of; reason++)
    {
        _rdpa_reason_set_tc_and_queue(reason, &tc, &queue);
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
    int rc = 0;
    rdpa_cpu_reason_index_t reason_cfg_idx = {BDMF_INDEX_UNASSIGNED, BDMF_INDEX_UNASSIGNED};
    rdpa_cpu_reason_cfg_t reason_cfg = {};
    uint8_t tc;
        
    reason_cfg.meter = BDMF_INDEX_UNASSIGNED;
    while (!rdpa_cpu_reason_cfg_get_next(cpu_obj, &reason_cfg_idx))
    {
        _rdpa_reason_set_tc_and_queue(reason_cfg_idx.reason, &tc, &reason_cfg.queue);
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

static void rdpa_rx_isr(long priv)
{
    enetx_rx_isr((enetx_channel *)priv);
}

static void rdpa_rx_dump_data_cb(bdmf_index queue, bdmf_boolean enabled)
{
    enet_dump_rx = enabled;
}

static int _rdpa_create_queue(bdmf_object_handle cpu_obj, enetx_channel *chan, int q_id, int size)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg;
    int rc;

    /* Make sure interrupt will not be called during RX queue configuration since interfaces might not be up yet */
    enetxapi_queue_int_disable(chan, q_id);

    bdmf_lock();
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
    bdmf_unlock();
    if (rc < 0)
        enet_err("failed to configure RDPA CPU RX q_id - %d chan->rx_q[q_id] - %d \n", q_id, chan->rx_q[q_id]);

    return rc;
}

static enetx_channel *_create_rdpa_queues(void)
{
    enetx_channel *chan;

    chan = kzalloc(sizeof(enetx_channel), GFP_KERNEL);
    if (!chan)
        return NULL;

#ifndef ONE_QUEUE_PER_CHANNEL
    chan->rx_q_count = 2;
    chan->rx_q[0] = RDPA_CPU_QUEUE_HI;
    chan->rx_q[1] = RDPA_CPU_QUEUE_LOW;
#else
    chan->next = kzalloc(sizeof(enetx_channel), GFP_KERNEL);
    if (!chan->next)
        return NULL;

    chan->rx_q_count = 1;
    chan->rx_q[0] = RDPA_CPU_QUEUE_HI;
    chan->next->rx_q_count = 1;
    chan->next->rx_q[0] = RDPA_CPU_QUEUE_LOW;
#endif

    return chan;
}

/* This implementation configures 2 queues (HI/LOW) on 1 channel */
int enetxapi_queues_init(enetx_channel **_chan)
{
    bdmf_object_handle cpu_obj = NULL;
    bdmf_object_handle system_obj = NULL;
    enetx_channel *chan, *next;
    int i, rc = 0;

    *_chan = chan = _create_rdpa_queues();
    if (!chan)
        goto exit;

    rc = rdpa_system_get(&system_obj);
    rc = rc ? rc : rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
    

    chan->rx_thread = kthread_run(chan_thread_handler, chan, "bcmsw_rx");
    init_waitqueue_head(&chan->rxq_wqh);

    while (chan)
    {
        next = chan->next;

        for (i = 0; i < chan->rx_q_count; i++)
            rc = rc ? : _rdpa_create_queue(cpu_obj, chan, i, DEFAULT_Q_SIZE);

        chan = next;
    }
    
    rc = rc ? : _rdpa_map_reasons_2_queue(system_obj, cpu_obj);

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

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
    if (rc)
        goto exit;

    while (chan)
    {

        if (chan->rx_thread)
        {
                kthread_stop(&chan->rx_thread);
                chan->rx_thread = NULL;
        }

        next = chan->next;

        for (i = 0; i < chan->rx_q_count; i++)
            rc |= _rdpa_destroy_queue(cpu_obj, chan, i);

        kfree(chan);
        chan = next;
    }

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
    rdpa_cpu_rx_info_t info;
    int rc;

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

    *fkb = fkb_init((uint8_t *)info.data , BCM_PKT_HEADROOM, (uint8_t *)(info.data + info.data_offset), info.size);
    (*fkb)->recycle_hook = (RecycleFuncP)bdmf_sysb_recycle;

    /* Parameters required by enet */
    rx_info->src_port = info.src_port;
    rx_info->flow_id = info.reason_data;
    rx_info->ptp_index = info.ptp_index;
    rx_info->data_offset = info.data_offset;
    if (unlikely(enet_dump_rx))
        rdpa_cpu_rx_dump_packet("enet", rdpa_cpu_host, hw_q_id, &info, 0);

    return 0;
}

void enetxapi_fkb_databuf_recycle(FkBuff_t *fkb, void *context)
{
    bdmf_sysb_databuf_recycle(fkb, context);
}

void enetxapi_buf_recycle(struct sk_buff *skb, uint32_t context, uint32_t flags)
{
    bdmf_sysb_recycle(skb, context, flags);
}

static inline int _rdpa_cpu_send_sysb(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    int rc;

#ifdef BRCM_FTTDP
    /* FTTDP FW does not support sending from sysb, so we need to copy to bpm */
    rc = rdpa_cpu_send_raw(bdmf_sysb_data(sysb), bdmf_sysb_length(sysb), info);
    /* rdpa_cpu_send_raw copies to bpm but does not free buffer */
    nbuff_flushfree(sysb);
#else
    rc = rdpa_cpu_send_sysb(sysb, info);
#endif

    return rc;
}

static int dispatch_pkt_lan(pNBuff_t pNBuff, enetx_port_t *port, int channel, int egress_queue)
{
    rdpa_cpu_tx_info_t info = {};
#ifdef CONFIG_BCM_PTP_1588
    char *ptp_offset;
#endif

    info.method = rdpa_cpu_tx_port;
    info.port = port->p.port_id;
    info.cpu_port = rdpa_cpu_host;
    info.x.lan.queue_id = egress_queue;

    enet_dbg_tx("rdpa_cpu_send: port %d queue %d\n", info.port, egress_queue);

#ifdef CONFIG_BCM_PTP_1588
    if (unlikely(is_pkt_ptp_1588((bdmf_sysb)pNBuff, &ptp_offset)))
        return ptp_1588_cpu_send_sysb((bdmf_sysb)pNBuff, &info, ptp_offset);
    else
#endif
    return _rdpa_cpu_send_sysb((bdmf_sysb)pNBuff, &info);
}

static int dispatch_pkt_gbe_wan(pNBuff_t pNBuff, enetx_port_t *port, int channel, int egress_queue)
{
    rdpa_cpu_tx_info_t info = {};

    info.method = rdpa_cpu_tx_port;
    info.port = port->p.port_id;
    info.cpu_port = rdpa_cpu_host;
    info.x.wan.queue_id = egress_queue;

    enet_dbg_tx("rdpa_cpu_send: port %d queue %d\n", info.port, egress_queue);

    return _rdpa_cpu_send_sysb((bdmf_sysb)pNBuff, &info);
}


#ifdef CONFIG_BCM96858
#define QUEUE_THRESHOLD 128*1536 /* Drop threshold in bytes */
#else
#define QUEUE_THRESHOLD 128 /* TODO: platform define */
#endif

static int rdpa_egress_tm_queues_cfg(bdmf_object_handle tm_obj, int q_count)
{
    bdmf_error_t rc = 0;
    rdpa_tm_queue_cfg_t queue_cfg = {};
    int qid;

    queue_cfg.drop_alg = rdpa_tm_drop_alg_dt;
    queue_cfg.drop_threshold = QUEUE_THRESHOLD;

    for (qid = 0; qid < q_count; qid++)
    {
        queue_cfg.queue_id = qid;
        if ((rc = rdpa_egress_tm_queue_cfg_set(tm_obj, qid, &queue_cfg)))
        {
            enet_err("Failed to configure RDPA egress tm queue %d. rc=%d\n", qid, rc);
            break;
        }
    }

    return rc;
}

static int create_rdpa_egress_tm(bdmf_object_handle port_obj)
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

    rdpa_egress_tm_index_set(tm_attr, is_wan ? rdpaif - rdpa_if_wan0 : rdpaif - rdpa_if_lan0);
    rdpa_egress_tm_dir_set(tm_attr, is_wan ? rdpa_dir_us : rdpa_dir_ds);
    rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_queue);
    rdpa_egress_tm_mode_set(tm_attr, rdpa_tm_sched_sp);

    if ((rc = bdmf_new_and_set(rdpa_egress_tm_drv(), port_obj, tm_attr, &tm_obj)))
    {
        enet_err("Failed to create RDPA egress tm for port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    if ((rc = rdpa_egress_tm_queues_cfg(tm_obj, is_wan ? 8 : 4)))
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

    enet_dbg("Created RDPA egress tm %s\n", tm_obj->name);

Exit:
    if (rc && tm_obj)
        bdmf_destroy(tm_obj);

    return rc;
}

bdmf_object_handle create_rdpa_port(rdpa_if rdpaif, rdpa_emac emac, bdmf_object_handle owner)
{
    bdmf_error_t rc;
    BDMF_MATTR(rdpa_port_attrs, rdpa_port_drv());
    bdmf_object_handle cpu_obj = NULL;
    bdmf_object_handle port_obj = NULL;
    rdpa_port_dp_cfg_t port_cfg = {};

    if ((rc = rdpa_port_index_set(rdpa_port_attrs, rdpaif)))
    {
        enet_err("Failed to set RDPA port index %d. rc=%d\n", rdpaif, rc);
        return NULL;
    }

#ifdef CONFIG_BCM96858
    port_cfg.ls_fc_enable = 1;
#endif
    port_cfg.emac = emac;

    if (rdpa_if_is_wan(rdpaif))
    {
        rdpa_port_wan_type_set(rdpa_port_attrs, rdpa_wan_gbe);
    }
    else if (rdpaif != rdpa_if_switch)
    {
#ifndef BRCM_FTTDP
        port_cfg.sal_enable = 1;
        port_cfg.dal_enable = 1;
        port_cfg.sal_miss_action = rdpa_forward_action_host;
        port_cfg.dal_miss_action = rdpa_forward_action_host;
#endif
    }

    if ((rc = rdpa_port_cfg_set(rdpa_port_attrs, &port_cfg)))
    {
        enet_err("Failed to set configuration for RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    if ((rc = bdmf_new_and_set(rdpa_port_drv(), owner, rdpa_port_attrs, &port_obj)))
    {
        enet_err("Failed to create RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    if (!rdpa_if_is_lag_and_switch(rdpaif) && (rc = create_rdpa_egress_tm(port_obj)))
    {
        enet_err("Failed to create ergress_tm for RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
#ifdef CONFIG_BCM96858
    rc = rc ? rc : rdpa_port_cpu_obj_set(port_obj, cpu_obj);
    if (rc)
    {
        enet_err("Failed to set CPU object for port %s, error %d\n", port_obj->name, rc);
        goto Exit;
    }
#else
    rc = 0;
#endif

    enet_dbg("Created RDPA port: %s\n", port_obj->name);

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

static int link_switch_to_rdpa_port(bdmf_object_handle port_obj)
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

static int link_tc_to_q_to_rdpa_lan_port(bdmf_object_handle port_obj)
{
    bdmf_error_t rc = 0;
    bdmf_object_handle rdpa_tc_to_queue_obj = NULL;
    rdpa_tc_to_queue_key_t t2q_key;

    t2q_key.dir = rdpa_dir_ds; 
    t2q_key.table = 0; 

    if ((rc = rdpa_tc_to_queue_get(&t2q_key, &rdpa_tc_to_queue_obj)))
    {
        enet_err("Failed to get RDPA switch port object. rc=%d\n", rc);
        goto Exit;
    }
   
    if ((rc = bdmf_link(rdpa_tc_to_queue_obj, port_obj, NULL)))
    {
        enet_err("Failed to link tc_to_q table to RDPA port rc=%d\n", rc);
        goto Exit;
    }

Exit:
    if (rdpa_tc_to_queue_obj)
        bdmf_put(rdpa_tc_to_queue_obj);

    return rc;
}

static int _demux_id_runner_port(enetx_port_t *self)
{
    rdpa_if demuxif = self->p.port_id;

#ifndef BRCM_FTTDP
    /* XXX non-G9991 FW wan gbe src port is lan0 + mac_id, not wan0 */
    if (rdpa_if_is_wan(demuxif))
    {
#ifndef CONFIG_BCM96858
        demuxif = rdpa_if_lan0 + self->p.mac->mac_id;

        if (self->p.mac->mac_id == 5)
#endif
            demuxif = rdpa_if_wan0;
    }
#endif

    return demuxif;
}

static int port_runner_port_init(enetx_port_t *self)
{
    bdmf_error_t rc;
    rdpa_if rdpaif = self->p.port_id;
    rdpa_emac emac = rdpa_emac_none;

    if (demux_on_sw(self->p.parent_sw, _demux_id_runner_port(self), self))
        return -1;

    if (self->p.mac)
        emac = rdpa_emac0 + self->p.mac->mac_id;

#ifdef CONFIG_BLOG
    self->n.blog_phy = BLOG_ENETPHY;
    self->n.blog_chnl = emac;
#endif

    if (!(self->priv = create_rdpa_port(rdpaif, emac, NULL)))
    {
        enet_err("Failed to create RDPA port object for %s\n", self->obj_name);
        return -1;
    }
        
    /* Override bp_parser settings, since once a rdpa port object is created, port role cannot change */
    self->p.port_cap = rdpa_if_is_wan(rdpaif) ? PORT_CAP_WAN_ONLY : PORT_CAP_LAN_ONLY;
    self->n.port_netdev_role = rdpa_if_is_wan(rdpaif) ? PORT_NETDEV_ROLE_WAN : PORT_NETDEV_ROLE_LAN;

    if (rdpa_if_is_wan(rdpaif))
        self->p.ops = &port_runner_port_wan_gbe; /* use ops with correct dispatch_pkt */
    else
    {
        rc = link_tc_to_q_to_rdpa_lan_port(self->priv);
        if (rc)
            return rc;
    }

    if (rdpa_if_is_lag_and_switch(rdpaif) && (rc = link_switch_to_rdpa_port(self->priv)))
    {
        enet_err("Failed to link RDPA switch to port object %s. rc =%d\n", self->obj_name, rc);
        return rc;
    }

    enet_dbg("Initialized runner port %s\n", self->obj_name);

    return 0;
}

static int port_runner_port_uninit(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_tm_cfg_t port_tm_cfg;

    if (!port_obj)
        return 0;

    rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg);
    if (port_tm_cfg.sched)
        bdmf_destroy(port_tm_cfg.sched);

    bdmf_destroy(port_obj);
    self->priv = 0;

    demux_on_sw(self->p.parent_sw, _demux_id_runner_port(self), NULL);

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

    if (init_cfg.gbe_wan_emac == rdpa_emac_none)
        return 0;

    wan_port_id = init_cfg.gbe_wan_emac; /* gbe wan port will always be emac number */

    enet_dbg("system init_cfg: wan_port_id=%d\n", wan_port_id);

    system_is_read = 1;

    return 0;
}

static int port_runner_sw_init(enetx_port_t *self)
{
#ifdef ENET_RUNNER_WIFI
    if (register_wifi_dev_forwarder())
        return -1;
#endif

#ifdef CONFIG_BCM_PTP_1588
    if (ptp_1588_init())
        return -1;
#endif

    if (!init_cfg.runner_ext_sw_cfg.enabled)
        goto Exit;

    if (!(self->priv = create_rdpa_port(rdpa_if_switch, rdpa_emac_none, NULL)))
    {
        enet_err("Failed to create RDPA switch object for %s\n", self->obj_name);
        return -1;
    }

Exit:
    enet_dbg("Initialized runner switch %s\n", self->obj_name);

    return 0;
}

static int port_runner_sw_uninit(enetx_port_t *self)
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

static void port_runner_port_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    port_generic_stats_get(self, net_stats);
    
    /* TODO: Add FW dropped packets */
}

static void port_runner_port_stats_clear(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_stat_t stat = {};

    rdpa_port_stat_set(port_obj, &stat);
    port_generic_stats_clear(self);
}
    
static int port_runner_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    int rc;

    if ((rc = read_init_cfg()))
        return rc;

    if (port_info->is_gpon)
    {
        *port_type = PORT_TYPE_RUNNER_GPON;
        *port_id = rdpa_if_wan0;
        return 0;
    }
    
    if (port_info->is_epon)
    {
        *port_type = PORT_TYPE_RUNNER_EPON;
        *port_id = rdpa_if_wan0;
        return 0;
    }

    if (port_info->is_detect)
    {
        *port_type = PORT_TYPE_RUNNER_DETECT;
        *port_id = rdpa_if_wan0;
        return 0;
    }

#ifdef ENET_RUNNER_WIFI
    if (port_info->port >= rdpa_if_ssid0 && port_info->port <= rdpa_if_ssid15)
    {
        *port_type = PORT_TYPE_RUNNER_WIFI;
        *port_id = port_info->port;
        return 0;
    }
#endif
    
    *port_type = PORT_TYPE_RUNNER_PORT;
    if (port_info->port == wan_port_id)
    {
        *port_id = rdpa_if_wan0;
        return 0;
    }

    if (port_info->is_attached)
        *port_id = rdpa_if_lag0 + port_info->port;
    else if (port_info->is_management)
        *port_id = rdpa_if_lan0 + 29;
    else
        *port_id = rdpa_if_lan0 + port_info->port;

    return 0;
}

#ifdef CONFIG_BCM_PTP_1588
static int port_runner_sw_demux(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port)
{
    uint32_t reason = rx_info->flow_id; /* reason is stored in flow_id */

    if (unlikely(reason == rdpa_cpu_rx_reason_etype_ptp_1588))
        ptp_1588_rx_pkt_store_timestamp(fkb->data, fkb->len, rx_info->ptp_index);
    
    return port_generic_sw_demux(sw, rx_info, fkb, out_port);
}
#endif

sw_ops_t port_runner_sw =
{
    .init = port_runner_sw_init,
    .uninit = port_runner_sw_uninit,
#ifdef CONFIG_BCM_PTP_1588
    .port_demux = port_runner_sw_demux,
#else
    .port_demux = port_generic_sw_demux,
#endif
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_runner_sw_port_id_on_sw,
};

static int port_runner_pause_get(enetx_port_t *self, int *rx_enable, int *tx_enable)
{
    if (!self->p.mac)
    {
        enet_err("missing mac device in port %s\n", self->obj_name);
        return -1;
    }

    return mac_dev_pause_get(self->p.mac, rx_enable, tx_enable);
}

static int port_runner_pause_set(enetx_port_t *self, int rx_enable, int tx_enable)
{
    if (!self->p.mac || !self->dev)
    {
        enet_err("missing mac or net device in port %s\n", self->obj_name);
        return -1;
    }

    return mac_dev_pause_set(self->p.mac, rx_enable, tx_enable, self->dev->dev_addr);
}

int port_runner_generic_mtu_set(enetx_port_t *self, int mtu)
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

port_ops_t port_runner_port =
{
    .init = port_runner_port_init,
    .uninit = port_runner_port_uninit,
    .dispatch_pkt = dispatch_pkt_lan,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .pause_get = port_runner_pause_get,
    .pause_set = port_runner_pause_set,
    .mtu_set = port_runner_generic_mtu_set,
};

port_ops_t port_runner_port_wan_gbe =
{
    .init = port_runner_port_init,
    .uninit = port_runner_port_uninit,
    .dispatch_pkt = dispatch_pkt_gbe_wan,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .pause_get = port_runner_pause_get,
    .pause_set = port_runner_pause_set,
    .mtu_set = port_runner_generic_mtu_set,
};

#if defined(GPON) || defined(EPON)
enetx_port_t *pon_port;

void enet_pon_drv_link_change(int up)
{
    if (pon_port)
    {
        if (up)
            netif_carrier_on(pon_port->dev);
        else
            netif_carrier_off(pon_port->dev);
    }
}
EXPORT_SYMBOL(enet_pon_drv_link_change);
#endif

int port_runner_empty_uninit(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;

    demux_on_sw(self->p.parent_sw, rdpa_if_wan0, NULL);

    bdmf_put(port_obj);
    self->priv = 0;

#if defined(GPON) || defined(EPON)
    pon_port = NULL;
#endif

    return 0;
}

#ifdef GPON

static void port_runner_gpon_stats(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
   rdpa_gem_stat_t gem_stat;
   rdpa_iptv_stat_t iptv_stat;
   bdmf_object_handle gem = NULL;
   bdmf_object_handle iptv = NULL;
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

iptv_exit:
    if (iptv)
        bdmf_put(iptv);
}

static int port_runner_gpon_init(enetx_port_t *self)
{
    self->p.port_cap = PORT_CAP_WAN_ONLY;
    self->n.port_netdev_role = PORT_NETDEV_ROLE_WAN;

    pon_port = self;

#ifdef CONFIG_BLOG
    self->n.blog_phy = BLOG_GPONPHY;
    self->n.set_channel_in_mark = 1; /* blog_chnl will be set to/from gem */
#endif
    if (demux_on_sw(self->p.parent_sw, rdpa_if_wan0, self))
        return -1;
    
    if (rdpa_port_get(rdpa_if_wan0, (bdmf_object_handle *)&self->priv))
        return -1;

    return 0;
}

static int dispatch_pkt_gpon(pNBuff_t pNBuff, enetx_port_t *port, int channel, int egress_queue)
{
    int rc;
    rdpa_cpu_tx_info_t info = {};

    info.method = rdpa_cpu_tx_port;
    info.port = rdpa_if_wan0;
    info.cpu_port = rdpa_cpu_host;
    info.x.wan.queue_id = egress_queue;
    info.x.wan.flow = channel;

    /* FIXME: BU6858 - omci channel should be ignored */
    if (channel == 0)  return 0;

    rc = _rdpa_cpu_send_sysb((bdmf_sysb)pNBuff, &info);
    if (unlikely(rc < 0))
    {
        rdpa_gem_flow_us_cfg_t us_cfg = {};
        bdmf_object_handle gem = NULL;

        rdpa_gem_get(channel, &gem);
        if (gem)
        {
            rdpa_gem_us_cfg_get(gem, &us_cfg);
            bdmf_put(gem);

            if (!us_cfg.tcont)
            {
                enet_err("can't send sysb - no tcont for gem (%d) \n", channel);
                return rc;
            }
        }

        enet_err("_rdpa_cpu_send_sysb() rc %d (wan_flow: %d queue_id: %u)\n", rc, channel, egress_queue);
    }

    return rc;
}

port_ops_t port_runner_gpon =
{
    .init = port_runner_gpon_init,
    .uninit = port_runner_empty_uninit,
    .dispatch_pkt = dispatch_pkt_gpon,
    .stats_get = port_runner_gpon_stats,
    /* TODO: stats_clear */
};
#endif

#ifdef EPON
static void port_runner_epon_stats(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    /* TODO: return mac stats */
}

static int port_runner_epon_init(enetx_port_t *self)
{
    self->p.port_cap = PORT_CAP_WAN_ONLY;
    self->n.port_netdev_role = PORT_NETDEV_ROLE_WAN;
    pon_port = self;
#ifdef CONFIG_BLOG
    self->n.blog_phy = BLOG_EPONPHY;
    self->n.set_channel_in_mark = 1; /* blog_chnl will be set to/from gem */
#endif
    if (demux_on_sw(self->p.parent_sw, rdpa_if_wan0, self))
        return -1;

    if (rdpa_port_get(rdpa_if_wan0, (bdmf_object_handle *)&self->priv))
        return -1;

    return 0;
}

static int dispatch_pkt_epon(pNBuff_t pNBuff, enetx_port_t *port, int channel, int egress_queue)
{
    int rc;
    rdpa_cpu_tx_info_t info = {};

    /* TODO: queue/channel mapping from EponFrame.c:EponTrafficSend() */
    info.method = rdpa_cpu_tx_port;
    info.port = rdpa_if_wan0;
    info.cpu_port = rdpa_cpu_host;
    info.x.wan.queue_id = egress_queue;
    info.x.wan.flow = channel;

    rc = _rdpa_cpu_send_sysb((bdmf_sysb)pNBuff, &info);

    return rc;
}

port_ops_t port_runner_epon =
{
    .init = port_runner_epon_init,
    .uninit = port_runner_empty_uninit,
    .dispatch_pkt = dispatch_pkt_epon,
    .stats_get = port_runner_epon_stats,
    /* TODO: stats_clear */
};
#endif

