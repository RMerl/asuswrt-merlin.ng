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

#ifndef __BCM_MACSEC_H
#define __BCM_MACSEC_H

#include <net/macsec.h>

struct bcm_macsec_ops 
{
    /* Device wide */
    int (*bcm_mdo_dev_open)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_dev_stop)(struct macsec_context *ctx, void *priv);
    /* SecY */
    int (*bcm_mdo_add_secy)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_upd_secy)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_del_secy)(struct macsec_context *ctx, void *priv);
    /* Security channels */
    int (*bcm_mdo_add_rxsc)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_upd_rxsc)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_del_rxsc)(struct macsec_context *ctx, void *priv);
    /* Security associations */
    int (*bcm_mdo_add_rxsa)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_upd_rxsa)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_del_rxsa)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_add_txsa)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_upd_txsa)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_del_txsa)(struct macsec_context *ctx, void *priv);
    /* Statistics */
    int (*bcm_mdo_get_dev_stats)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_get_tx_sc_stats)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_get_tx_sa_stats)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_get_rx_sc_stats)(struct macsec_context *ctx, void *priv);
    int (*bcm_mdo_get_rx_sa_stats)(struct macsec_context *ctx, void *priv);
};

struct bcm_macsec_dev
{
    struct bcm_macsec_ops *macsec_ops;
    void *priv;
#ifdef CONFIG_BCM_MACSEC_FIRELIGHT
    union {
        unsigned short svtag[2];
        unsigned int sv_tag;
    };
#endif
};

extern struct bcm_macsec_ops *bcm_macsec_get_ops(void);
extern void bcm_macsec_port_set(int macsec_port, int mac_port);

#endif

