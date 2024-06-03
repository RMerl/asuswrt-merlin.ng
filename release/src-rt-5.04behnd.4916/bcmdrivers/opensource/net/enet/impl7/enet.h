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

#ifndef _ENET_H_
#define _ENET_H_

#include "enet_types.h"
#include "port.h"
#include <linux/netdevice.h>
#include <linux/skbuff.h>

#ifdef CONFIG_BCM_XDP
#include "enet_xdp.h"
#endif
 
#if defined(CONFIG_BCM_NFT_OFFLOAD)
#include "fm_nft.h"
#endif

#define ENET_THREAD_NAME        "enet-kthrd"

/** ENET return codes */
#define ENET_FAILURE            (-1)
#define ENET_SUCCESS            (0)

#define ENET_ASSERT(exp)        BCM_ASSERT_A(exp)


#define RXQ_MAX 8
typedef struct _enetx_channel {
    /* Mapping to HW queue ids */
    int rx_q[RXQ_MAX];
    /* Number of queues in channel */
    int rx_q_count;
    /* Number of Linux interfaces using this channel */
    int open_count;
    /* Linked list to next channel */
    struct _enetx_channel *next;
    /*event queue for thread*/
    wait_queue_head_t rxq_wqh;
    volatile unsigned long rxq_cond;
    /*thread handler*/
    struct task_struct *rx_thread;

} enetx_channel;

typedef struct
{
#define BCMENET_PRIV_FEAT_SW_GSO    (1<<0)
    uint32_t     priv_feat;

    enetx_port_t *port;
#ifdef CONFIG_BCM_ENET_TC_OFFLOAD
    struct list_head enet_devs;
    struct list_head ig_chains;
    struct list_head eg_chains;
    struct net_device *enet_netdev;
#endif /* CONFIG_BCM_ENET_TC_OFFLOAD */

#ifdef CONFIG_BCM_XDP
    struct bpf_prog         *_xdp_prog;
    struct xdp_rxq_info     xdp_rxq;
    struct enet_xdp_stats   stats;
#endif
#if defined(CONFIG_BCM_NFT_OFFLOAD)
    fm_nft_ctx_t * fm_nft_ctx;
#endif

} enetx_netdev;

typedef void (*enetx_work_func_t)(enetx_port_t *port);

typedef struct
{
    struct work_struct base_work;
    enetx_port_t *port;
    enetx_work_func_t func;
} enetx_work_t;

int enetx_queue_work(enetx_port_t *port, enetx_work_func_t func);

#define NETDEV_PRIV(dev)  ((enetx_netdev *)netdev_priv(dev))

/* Called from platform ISR implementation */
int enetx_rx_isr(enetx_channel *chan);

/* Called from port init */
int enet_create_netdevice(enetx_port_t *p);
void enet_remove_netdevice(enetx_port_t *p);
void enet_dev_role_update(enetx_port_t *p);

/* Dump ENET system statistics, called from bcmenet_proc */
extern void enet_sys_dump(void);

extern enetx_port_t *root_sw;

/* External per-platform APIs */

#if defined(ARCHER_DEVICE) || defined(RUNNER)

/* read a linked list of fkbs */
int enetxapi_rx_pkt(int queue_id, FkBuff_t **fkb, enetx_rx_info_t *rx_info);
/* Initialize HW queues */
int enetxapi_queues_init(enetx_channel **chan);
/* Uninitialize HW queues */
int enetxapi_queues_uninit(enetx_channel **chan);
/* Disable/clear interrupts on queue */
void enetxapi_queue_int_disable(enetx_channel *chan, int q_id);
/* Enable interrupts on queue */
void enetxapi_queue_int_enable(enetx_channel *chan, int q_id);
/* Return if queue interrupt handler need to be rescheduled */
int enetxapi_queue_need_reschedule(enetx_channel *chan, int q_id);
/* FKB recycle callback */
void enetxapi_fkb_databuf_recycle(FkBuff_t *fkb);
/* SKB recycle callback */
void enetxapi_buf_recycle(struct sk_buff *skb, uint32_t context, uint32_t flags);

/* Check if should dump the packet in case of source port demux error. */
int enetxapi_rx_pkt_dump_on_demux_err(enetx_rx_info_t *rx_info);

#else  // for compiling enet without runner, archer

#define enetxapi_rx_pkt(q, f, r)                0
#define enetxapi_queues_init(c)                 0
#define enetxapi_queues_uninit(c)
#define enetxapi_queue_int_disable(c, q)
#define enetxapi_queue_int_enable(c, q)
#define enetxapi_queue_need_reschedule(c, q)    0
#define enetxapi_fkb_databuf_recycle(f)
#define enetxapi_buf_recycle                    0
#define enetxapi_rx_pkt_dump_on_demux_err(r)    0

#endif

/* Initialize platform dependent configuration */
int enetxapi_post_parse(void);
int enetxapi_post_config(void);
void set_mac_cfg_by_phy(enetx_port_t *p);
void set_mac_eee_by_phy(enetx_port_t *p);
int sw_print_mac_phy_info(enetx_port_t *sw, char **buf, int *sz);
netdev_tx_t enet_xmit(pNBuff_t pNBuff, struct net_device *dev);

int enet_priv_info_get(struct net_device *dev, bcm_netdev_priv_info_type_t info_type
                                    , bcm_netdev_priv_info_out_t *info_out);

#ifdef CONFIG_BCM_BPM_BUF_TRACKING
#include <linux/gbpm.h>
#define ETH_GBPM_TRACK_BUF(buf, value, info)         GBPM_TRACK_BUF(buf, GBPM_DRV_ETH, value, info)
#define ETH_GBPM_TRACK_SKB(skb, value, info)         GBPM_TRACK_SKB(skb, GBPM_DRV_ETH, value, info)
#define ETH_GBPM_TRACK_FKB(fkb, value, info)         GBPM_TRACK_FKB(fkb, GBPM_DRV_ETH, value, info)
#else
#define ETH_GBPM_TRACK_BUF(buf, value, info)         do{}while(0)
#define ETH_GBPM_TRACK_SKB(skb, value, info)         do{}while(0)
#define ETH_GBPM_TRACK_FKB(fkb, value, info)         do{}while(0)
#endif

#ifdef CONFIG_BRCM_QEMU 
int qemu_enet_rx_pkt(int budget);
#endif

#ifdef CONFIG_BLOG
#define BLOG_EGPHY_CHNL_MAX 128
extern enetx_port_t *blog_chnl_to_port[];
void blog_chnl_with_mark_set(int blog_phy, enetx_port_t *p);
void blog_chnl_set(int blog_phy, int blog_channel_rx, int blog_channel_tx, enetx_port_t *p);
void blog_chnl_unit_port_set(enetx_port_t *p);
void blog_chnl_unit_port_unset(enetx_port_t *p);
#else
static inline void blog_chnl_with_mark_set(int blog_phy, enetx_port_t *p) {}
static inline void blog_chnl_set(int blog_phy, int blog_channel_rx, int blog_channel_tx, enetx_port_t *p) {}
static inline void blog_chnl_unit_port_set(enetx_port_t *p) {}
static inline void blog_chnl_unit_port_unset(enetx_port_t *p) {}
#endif

static inline void _free_fkb(FkBuff_t *fkb)
{
    fkb_flush(fkb, fkb->data, fkb->len, FKB_CACHE_FLUSH);
    fkb_release_blog(fkb);
    enetxapi_fkb_databuf_recycle(fkb);
}
#endif /* _ENET_H_ */

