/*
   <:copyright-BRCM:2021:DUAL/GPL:standard
   
      Copyright (c) 2021 Broadcom 
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

#include "bcm_macsec.h"
#include "port.h"
#include "enet.h"
#include "bcmenet_common.h"
#if defined(CONFIG_BCM_MACSEC_FIRELIGHT)
#include <rdpa_api.h>

static int enet_macsec_port_cfg(enetx_port_t *port, int enabled)
{
    int rc, power;
    bdmf_object_handle port_obj = port->priv;
    rdpa_port_dp_cfg_t port_cfg = {};

    rdpa_port_cfg_get(port_obj, &port_cfg);
    port_cfg.bbh_rx_crc_omit_disable = enabled ? 1 : 0;
    rc = rdpa_port_cfg_set(port_obj, &port_cfg);
    
    if (rc)
        return rc;

    phy_dev_power_get(port->p.phy, &power);

    if (power && enabled)
    {
        phy_dev_power_set(port->p.phy, 0);
        phy_dev_power_set(port->p.phy, 1);
    }

    return rc;
}

#define BRCM_SVTAG_LEN     4
#define SVTAG_SIGNATURE    0xEC00
#define SVTAG_OFFSET_ADDER 0x0000
#define SVTAG_PKT_TYPE     0x0400

// based on impl5:bcm63xx_fkb_put_tag()
static inline void enet_fkb_put_svtag(FkBuff_t * fkb_p, struct bcm_macsec_dev *macsec_dev)
{
    int i;
    int tailroom;
    uint16 *to, *from = (uint16*)fkb_p->data;
    BcmEnet_hdr2 *pHdr;

    to = (uint16*)fkb_push(fkb_p, BRCM_SVTAG_LEN);
    pHdr = (BcmEnet_hdr2 *)to;
    for (i = 0; i < ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */

    pHdr->brcm_type = macsec_dev->svtag[0];
    pHdr->brcm_tag = macsec_dev->svtag[1];

    tailroom = ETH_ZLEN + BRCM_SVTAG_LEN - fkb_p->len;
    if (tailroom > 0)
    {
        fkb_pad(fkb_p, tailroom);
        fkb_p->dirty_p = _to_dptr_from_kptr_(fkb_p->data + fkb_p->len);
    }
}

// based on impl5:bcm63xx_skb_put_tag()
static inline struct sk_buff *enet_skb_put_svtag(struct sk_buff *skb, struct bcm_macsec_dev *macsec_dev)
{
    int i, headroom;
    int tailroom;

    headroom = BRCM_SVTAG_LEN;
    tailroom = ETH_ZLEN - skb->len;

    if (tailroom < 0)
    {
        tailroom = 0;
    }

#if defined(CONFIG_BCM_USBNET_ACCELERATION)
    if ((skb_writable_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
#else
    if ((skb_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
#endif
    {
        struct sk_buff *oskb = skb;
        skb = skb_copy_expand(oskb, headroom, tailroom, GFP_ATOMIC);
        kfree_skb(oskb);
        if (!skb)
        {
            return NULL;
        }
    }
#if defined(CONFIG_BCM_USBNET_ACCELERATION)
    else if ((headroom != 0) && (skb->bcm_ext.clone_wr_head == NULL))
#else
    else if ((headroom != 0) && !(skb_clone_writable(skb, headroom)))
#endif
    {
        skb = skb_unshare(skb, GFP_ATOMIC);
        if (!skb)
        {
            return NULL;
        }
    }

    if (tailroom > 0)
    {
        if (skb_is_nonlinear(skb))
        {
            /* Non linear skb whose skb->len is < minimum Ethernet Packet Length
               (ETHZLEN or ETH_ZLEN + BroadcomMgmtTag Length) */
            if (skb_linearize(skb))
            {
                return NULL;
            }
        }
        memset(skb->data + skb->len, 0, tailroom);  /* padding to 0 */
        skb_put(skb, tailroom);
    }

    if (headroom != 0)
    {
        uint16 *to, *from;
        BcmEnet_hdr2 *pHdr2 = (BcmEnet_hdr2 *)skb_push(skb, headroom);
        to = (uint16*)pHdr2;
        from = (uint16*)(skb->data + headroom);
        for (i = 0; i < ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
 
        pHdr2->brcm_type = macsec_dev->svtag[0];
        pHdr2->brcm_tag = macsec_dev->svtag[1];

        if (skb_mac_header_was_set(skb))
            skb->mac_header -= headroom;
        /* network_header and transport_header are unchanged */
    }
    return skb;
}

int enet_macsec_port_tx_pkt_mod(enetx_port_t *port, pNBuff_t *pNBuff, uint8_t **data, uint32_t *len, unsigned int port_map)
{
    FkBuff_t *pFkb = 0;
    struct sk_buff *skb = 0;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    if (macsec_dev->sv_tag == 0)
        return 0; /* Nothing to do */

    if (IS_FKBUFF_PTR(*pNBuff))
    {
        FkBuff_t * pFkbOrig = PNBUFF_2_FKBUFF(*pNBuff);

        pFkb = fkb_unshare(pFkbOrig);

        if (pFkb == FKB_NULL)
        {
            fkb_free(pFkbOrig);
            INC_STAT_TX_DROP(port, tx_dropped_no_fkb);
            return -1;
        }
        enet_fkb_put_svtag(pFkb, macsec_dev);
        *data = (void *)pFkb->data;
        *len  = pFkb->len;
        *pNBuff = PBUF_2_PNBUFF((void*)pFkb, FKBUFF_PTR);
    }
    else
    {
        skb = PNBUFF_2_SKBUFF(*pNBuff);
        skb = enet_skb_put_svtag(skb, macsec_dev);
        if (skb == NULL) {
            INC_STAT_TX_DROP(port, tx_dropped_no_skb);
            return -1;
        }
        *data = (void *)skb->data;   /* Re-encode pNBuff for adjusted data and len */
        *len  = skb->len;
        *pNBuff = PBUF_2_PNBUFF((void*)skb, SKBUFF_PTR);
    }
    return 0;
}
#endif

int enet_mdo_dev_open(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_dev_open(ctx, macsec_dev->priv);

#if defined(CONFIG_BCM_MACSEC_FIRELIGHT)
    if (rc)
        return rc;

    rc = enet_macsec_port_cfg(port, 1);
    port->dev->flags |= IFF_NOARP;
    {
        unsigned short port_id = (uintptr_t)macsec_dev->priv;
        macsec_dev->svtag[0] = htons(SVTAG_SIGNATURE | SVTAG_OFFSET_ADDER);
        macsec_dev->svtag[1] = htons(SVTAG_PKT_TYPE | (port_id & 0xFF));
    }
    port->p.ops->tx_pkt_mod = enet_macsec_port_tx_pkt_mod;
#endif
    return rc;
}

int enet_mdo_dev_stop(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_dev_stop(ctx, macsec_dev->priv);

#if defined(CONFIG_BCM_MACSEC_FIRELIGHT)
    macsec_dev->sv_tag = 0;
    port->p.ops->tx_pkt_mod = NULL;

    rc = enet_macsec_port_cfg(port, 0);
    port->dev->flags &= ~IFF_NOARP;
#endif

    return rc;
}

int enet_mdo_add_secy(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_add_secy(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_upd_secy(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_upd_secy(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_del_secy(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_del_secy(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_add_rxsc(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_add_rxsc(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_upd_rxsc(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_upd_rxsc(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_del_rxsc(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_del_rxsc(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_add_rxsa(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_add_rxsa(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_upd_rxsa(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_upd_rxsa(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_del_rxsa(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_del_rxsa(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_add_txsa(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_add_txsa(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_upd_txsa(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_upd_txsa(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_del_txsa(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_del_txsa(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_get_dev_stats(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_get_dev_stats(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_get_tx_sc_stats(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_get_tx_sc_stats(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_get_tx_sa_stats(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_get_tx_sa_stats(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_get_rx_sc_stats(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_get_rx_sc_stats(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_get_rx_sa_stats(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_get_rx_sa_stats(ctx, macsec_dev->priv);

    return rc;
}

const struct macsec_ops enet_macsec_ops =
{
    .mdo_dev_open = enet_mdo_dev_open,
    .mdo_dev_stop = enet_mdo_dev_stop,
    .mdo_add_secy = enet_mdo_add_secy,
    .mdo_upd_secy = enet_mdo_upd_secy,
    .mdo_add_secy = enet_mdo_add_secy,
    .mdo_del_secy = enet_mdo_del_secy,
    .mdo_add_rxsc = enet_mdo_add_rxsc,
    .mdo_upd_rxsc = enet_mdo_upd_rxsc,
    .mdo_del_rxsc = enet_mdo_del_rxsc,
    .mdo_add_rxsa = enet_mdo_add_rxsa,
    .mdo_upd_rxsa = enet_mdo_upd_rxsa,
    .mdo_del_rxsa = enet_mdo_del_rxsa,
    .mdo_add_txsa = enet_mdo_add_txsa,
    .mdo_upd_txsa = enet_mdo_upd_txsa,
    .mdo_del_txsa = enet_mdo_del_txsa,
    .mdo_get_dev_stats = enet_mdo_get_dev_stats,
    .mdo_get_tx_sc_stats = enet_mdo_get_tx_sc_stats,
    .mdo_get_tx_sa_stats = enet_mdo_get_tx_sa_stats,
    .mdo_get_rx_sc_stats = enet_mdo_get_rx_sc_stats,
    .mdo_get_rx_sa_stats = enet_mdo_get_rx_sa_stats,
};
