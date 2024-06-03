/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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