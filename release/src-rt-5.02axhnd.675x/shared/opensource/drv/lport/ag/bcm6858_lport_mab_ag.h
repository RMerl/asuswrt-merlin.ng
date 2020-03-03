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

#ifndef _BCM6858_LPORT_MAB_AG_H_
#define _BCM6858_LPORT_MAB_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t link_down_rst_en;
    uint8_t xgmii_tx_rst;
    uint8_t gmii_tx_rst;
    uint8_t xgmii_rx_rst;
    uint8_t gmii_rx_rst;
} lport_mab_cntrl;

typedef struct
{
    uint8_t arb_mode;
    uint8_t p7_weight;
    uint8_t p6_weight;
    uint8_t p5_weight;
    uint8_t p4_weight;
} lport_mab_tx_wrr_ctrl;

typedef struct
{
    uint8_t xgmii1_tx_threshold;
    uint8_t gmii7_tx_threshold;
    uint8_t gmii6_tx_threshold;
    uint8_t gmii5_tx_threshold;
    uint8_t gmii4_tx_threshold;
} lport_mab_tx_threshold;

typedef struct
{
    uint8_t xgmii_rx_afifo_overrun;
    uint8_t gmii_rx_afifo_overrun_vect;
    uint8_t xgmii_tx_frm_underrun;
    uint8_t xgmii_outstanding_credits_cnt_underrun;
    uint8_t gmii_outstanding_credits_cnt_underrun_vect;
    uint8_t xgmii_tx_afifo_overrun;
    uint8_t gmii_tx_afifo_overrun_vect;
} lport_mab_status;

int ag_drv_lport_mab_cntrl_set(uint8_t xlmac_id, const lport_mab_cntrl *cntrl);
int ag_drv_lport_mab_cntrl_get(uint8_t xlmac_id, lport_mab_cntrl *cntrl);
int ag_drv_lport_mab_tx_wrr_ctrl_set(uint8_t xlmac_id, const lport_mab_tx_wrr_ctrl *tx_wrr_ctrl);
int ag_drv_lport_mab_tx_wrr_ctrl_get(uint8_t xlmac_id, lport_mab_tx_wrr_ctrl *tx_wrr_ctrl);
int ag_drv_lport_mab_tx_threshold_set(uint8_t xlmac_id, const lport_mab_tx_threshold *tx_threshold);
int ag_drv_lport_mab_tx_threshold_get(uint8_t xlmac_id, lport_mab_tx_threshold *tx_threshold);
int ag_drv_lport_mab_link_down_tx_data_set(uint8_t xlmac_id, uint8_t txctl, uint8_t txd);
int ag_drv_lport_mab_link_down_tx_data_get(uint8_t xlmac_id, uint8_t *txctl, uint8_t *txd);
int ag_drv_lport_mab_status_get(uint8_t xlmac_id, lport_mab_status *status);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lport_mab_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

