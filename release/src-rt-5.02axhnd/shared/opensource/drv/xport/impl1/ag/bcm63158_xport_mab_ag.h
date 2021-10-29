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

#ifndef _BCM63158_XPORT_MAB_AG_H_
#define _BCM63158_XPORT_MAB_AG_H_

//#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t link_down_rst_en;
    uint8_t xgmii_tx_rst;
    uint8_t gmii_tx_rst;
    uint8_t xgmii_rx_rst;
    uint8_t gmii_rx_rst;
} xport_mab_ctrl;

typedef struct
{
    uint8_t arb_mode;
    uint8_t p3_weight;
    uint8_t p2_weight;
    uint8_t p1_weight;
    uint8_t p0_weight;
} xport_mab_tx_wrr_ctrl;

typedef struct
{
    uint8_t xgmii0_tx_threshold;
    uint8_t gmii3_tx_threshold;
    uint8_t gmii2_tx_threshold;
    uint8_t gmii1_tx_threshold;
    uint8_t gmii0_tx_threshold;
} xport_mab_tx_threshold;

typedef struct
{
    uint8_t xgmii_rx_afifo_overrun;
    uint8_t gmii_rx_afifo_overrun_vect;
    uint8_t xgmii_tx_frm_underrun;
    uint8_t xgmii_outstanding_credits_cnt_underrun;
    uint8_t gmii_outstanding_credits_cnt_underrun_vect;
    uint8_t xgmii_tx_afifo_overrun;
    uint8_t gmii_tx_afifo_overrun_vect;
} xport_mab_status;

int ag_drv_xport_mab_ctrl_set(const xport_mab_ctrl *ctrl);
int ag_drv_xport_mab_ctrl_get(xport_mab_ctrl *ctrl);
int ag_drv_xport_mab_tx_wrr_ctrl_set(const xport_mab_tx_wrr_ctrl *tx_wrr_ctrl);
int ag_drv_xport_mab_tx_wrr_ctrl_get(xport_mab_tx_wrr_ctrl *tx_wrr_ctrl);
int ag_drv_xport_mab_tx_threshold_set(const xport_mab_tx_threshold *tx_threshold);
int ag_drv_xport_mab_tx_threshold_get(xport_mab_tx_threshold *tx_threshold);
int ag_drv_xport_mab_link_down_tx_data_set(uint8_t txctl, uint8_t txd);
int ag_drv_xport_mab_link_down_tx_data_get(uint8_t *txctl, uint8_t *txd);
int ag_drv_xport_mab_status_get(xport_mab_status *status);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_mab_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

