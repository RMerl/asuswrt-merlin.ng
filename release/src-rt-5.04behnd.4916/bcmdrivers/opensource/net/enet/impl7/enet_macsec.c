/*
   <:copyright-BRCM:2021:DUAL/GPL:standard
   
      Copyright (c) 2021 Broadcom 
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

#include "bcm_macsec.h"
#include "port.h"
#include "enet.h"

int enet_mdo_dev_open(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_dev_open(ctx, macsec_dev->priv);

    return rc;
}

int enet_mdo_dev_stop(struct macsec_context *ctx)
{
    int rc = 0;
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(ctx->netdev))->port;
    struct bcm_macsec_dev *macsec_dev = &(port->p.macsec_dev);

    rc = macsec_dev->macsec_ops->bcm_mdo_dev_stop(ctx, macsec_dev->priv);

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