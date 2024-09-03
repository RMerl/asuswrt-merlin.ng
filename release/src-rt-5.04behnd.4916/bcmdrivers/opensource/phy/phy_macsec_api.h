/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

#ifndef PHY_MACSEC_API_H_
#define PHY_MACSEC_API_H_

#include "phy_drv.h"
#include "bcm_macsec.h"

int phy_macsec_pu_init(phy_dev_t *phy_dev);
int phy_macsec_oper(phy_dev_t *phy_dev, void* data);


int phy_macsec_mdo_dev_open(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_dev_stop(struct macsec_context *ctx, void *priv);
/* SecY */
int phy_macsec_mdo_add_secy(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_upd_secy(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_del_secy(struct macsec_context *ctx, void *priv);
/* Security channels */
int phy_macsec_mdo_add_rxsc(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_upd_rxsc(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_del_rxsc(struct macsec_context *ctx, void *priv);
/* Security associations */
int phy_macsec_mdo_add_rxsa(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_upd_rxsa(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_del_rxsa(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_add_txsa(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_upd_txsa(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_del_txsa(struct macsec_context *ctx, void *priv);
/* Statistics */
int phy_macsec_mdo_get_dev_stats(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_get_tx_sc_stats(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_get_tx_sa_stats(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_get_rx_sc_stats(struct macsec_context *ctx, void *priv);
int phy_macsec_mdo_get_rx_sa_stats(struct macsec_context *ctx, void *priv);


#endif