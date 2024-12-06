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
 *  Created on: Jan/2023
 *      Author: lode.lathouwers@broadcom.com
 */


#include "enet_xdp.h"
#include "enet.h"
#include "fm_nft.h"
#include "rdpa_types.h"
#include "rdpa_cpu_basic.h"
#include "rdpa_cpu.h"

#ifdef CONFIG_BCM_NFT_OFFLOAD
#include <net/xdp_nft.h>
#endif

#ifdef CONFIG_BCM_XDP
enet_xdp_ingress_result_t enet_xdp_handle_ingress(struct net_device *dev, FkBuff_t *fkb, struct enet_xdp_stats * stats)
{
    enetx_netdev *priv = (enetx_netdev*) netdev_priv(dev);
    struct xdp_buff xdp;
    uint32_t act;
    int err;
    unsigned long cookie;
    unsigned int size;

    xdp.data = fkb->data;
    xdp.data_end = fkb->data + fkb->len;
    xdp.data_hard_start = PFKBUFF_TO_PHEAD(fkb);
    xdp.rxq = &priv->xdp_rxq;
    xdp.frame_sz = SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) + xdp.data_end - xdp.data_hard_start;

    size = xdp.data_end - xdp.data;
    stats->xdp_bytes += size;
    stats->xdp_packets++; 
 
    act = fm_nft_run_xdp_meta(dev, priv->_xdp_prog, &xdp, &cookie);
    // NOTE : xdp.data, xdp.data_end could be changed

    switch (act) 
    {
        case XDP_PASS:
            stats->xdp_pass++;
            break;
        case XDP_TX:
            stats->xdp_tx++;
            break;
        case XDP_REDIRECT:
            err = xdp_do_redirect(dev, &xdp, priv->_xdp_prog);
            if (err) 
            {
                if (printk_ratelimit())
                    enet_err("XDP REDIRECT ERROR %i", err); // redirect packet
                goto dropped;
            }
#ifdef CONFIG_BCM_NFT_OFFLOAD
            if (cookie)
               fm_nft_stats_update(priv->fm_nft_ctx, cookie, 1, size, 0);
#endif
            stats->xdp_redirect++;
            return ENET_XDP_INGRESS_REDIRECT;
        case XDP_ABORTED:
        case XDP_DROP:
            goto dropped;
        default:
            if (printk_ratelimit())
                enet_err("XDP unexpected return code");
            goto dropped;
    }

    return ENET_XDP_INGRESS_PASS;
dropped:
    stats->xdp_drops++;
    _free_fkb(fkb);
#ifdef CONFIG_BCM_NFT_OFFLOAD
    if (cookie)
       fm_nft_stats_update(priv->fm_nft_ctx, cookie, 0, size, 1);
#endif
    return ENET_XDP_INGRESS_DROP;
}


#ifdef CONFIG_BCM_NFT_OFFLOAD
struct bpf_prog *enet_xdp_get_progs_by_dev(struct net_device *dev)
{
    enetx_netdev *priv;

    if (dev == NULL) return NULL;

    priv = (enetx_netdev*) netdev_priv(dev);

    return priv->_xdp_prog;
}
#endif


static int enet_xdp_set(struct net_device *dev, struct bpf_prog *prog,
			struct netlink_ext_ack *extack)
{
    enetx_netdev *priv = (enetx_netdev*) netdev_priv(dev);
    struct bpf_prog *old_prog;
    int err;

    old_prog = priv->_xdp_prog;
    priv->_xdp_prog = prog;

    if (prog) 
    {
        err = xdp_rxq_info_reg(&priv->xdp_rxq, dev, 0, 0 /*rq->xdp_napi.napi_id*/);
        if (err && printk_ratelimit())
            enet_err("xdp_rxq_info_reg error %i\n", err);

        err = xdp_rxq_info_reg_mem_model(&priv->xdp_rxq,                                                                                           
                                         MEM_TYPE_PAGE_SHARED, /* TODO : NEED TO ADD OUR OWN MEMTYPE */
                                         NULL); 

        if (err && printk_ratelimit())
            enet_err("xdp_rxq_info_reg_mem_model error %i\n", err);

    }

    if (old_prog) 
    {
        xdp_rxq_info_unreg(&priv->xdp_rxq);
        bpf_prog_put(old_prog);
    }

    return 0;
}

int enet_xdp_xmit(struct net_device *dev, int n,
                         struct xdp_frame **frames, u32 flags)
{
    int i,rv;
    enetx_netdev *ndev = NETDEV_PRIV(dev);
    enetx_port_t *port = ndev->port;
    rdpa_cpu_tx_info_t info =
    {
        .method = rdpa_cpu_tx_port,
        .port_obj = port->priv,
        .cpu_port = rdpa_cpu_host,
        .bits.no_lock = 0,
        .x.wan.queue_id = 0, //RDPA_OMCI_TCONT_QUEUE_ID,
        .x.wan.flow = 0, //RDPA_OMCI_FLOW_ID,
    };

    if (unlikely(flags & ~XDP_XMIT_FLAGS_MASK))
        return -EINVAL;

    if (!netif_running(dev))
        return -ENETDOWN;

    /* TODO : check flags can have FLUSH set :  XDP_XMIT_FLUSH */

    for (i = 0; i < n; i++) 
    {
        struct xdp_frame * frame = frames[i];
        FkBuff_t * fkb = PHEAD_TO_PFKBUFF(frame);
 
        /* transfer modifications from xdp back to fxb */
        fkb->data = frame->data;
        fkb->len = frame->len;
        rv = rdpa_cpu_send_sysb(FKBUFF_2_PNBUFF(fkb), &info);

        if (rv) {
          if (printk_ratelimit())
            enet_err("enet_xdp_xmit rdpa_cpu_send_sysb returns %i",rv);
          ndev->stats.xdp_xmit_err++;
        }
        else
          ndev->stats.xdp_xmit++;
    }

    return n; 
}


int enet_xdp(struct net_device *dev, struct netdev_bpf *xdp)
{
    switch (xdp->command)
    {
        case XDP_SETUP_PROG:
            return enet_xdp_set(dev, xdp->prog, xdp->extack);
        // case XDP_SETUP_XSK_POOL:// enum name depends on L515
        default:
            return -EINVAL;
    }
}
#endif
