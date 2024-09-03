/*
   Copyright (c) 2015 Broadcom
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

#ifndef _BCM4912_XPORT_MAB_AG_H_
#define _BCM4912_XPORT_MAB_AG_H_


/**************************************************************************************************/
/* arb_mode:  - TDM Arbiter/Scheduler Mode.
1'b0 - Fixed Mode. TDM slots allocation is not affect */
/*           ed by the port activity.
1'b1 - Work-Conserving Mode. TDM slots allocation is affect */
/*           ed by the port activity.                                                             */
/* p3_weight:  - P3 weight expressed in TDM time slots.                                           */
/* p2_weight:  - P2 weight expressed in TDM time slots.                                           */
/* p1_weight:  - P1 weight expressed in TDM time slots.                                           */
/* p0_weight:  - P0 weight expressed in TDM time slots.                                           */
/**************************************************************************************************/
typedef struct
{
    uint8_t arb_mode;
    uint8_t p3_weight;
    uint8_t p2_weight;
    uint8_t p1_weight;
    uint8_t p0_weight;
} xport_mab_tx_wrr_ctrl;


/**************************************************************************************************/
/* xgmii3_tx_threshold:  - XGMII P3 asynchronous TX FIFO read depth at which packet dequeue start */
/*                      s.                                                                        */
/* xgmii2_tx_threshold:  - XGMII P2 asynchronous TX FIFO read depth at which packet dequeue start */
/*                      s.                                                                        */
/* xgmii1_tx_threshold:  - XGMII P1 asynchronous TX FIFO read depth at which packet dequeue start */
/*                      s.                                                                        */
/* xgmii0_tx_threshold:  - XGMII P0 asynchronous TX FIFO read depth at which packet dequeue start */
/*                      s.                                                                        */
/* gmii3_tx_threshold:  - GMII P3 asynchronous TX FIFO read depth at which packet dequeue starts. */
/* gmii2_tx_threshold:  - GMII P2 asynchronous TX FIFO read depth at which packet dequeue starts. */
/* gmii1_tx_threshold:  - GMII P1 asynchronous TX FIFO read depth at which packet dequeue starts. */
/* gmii0_tx_threshold:  - GMII P0 asynchronous TX FIFO read depth at which packet dequeue starts. */
/**************************************************************************************************/
typedef struct
{
    uint8_t xgmii3_tx_threshold;
    uint8_t xgmii2_tx_threshold;
    uint8_t xgmii1_tx_threshold;
    uint8_t xgmii0_tx_threshold;
    uint8_t gmii3_tx_threshold;
    uint8_t gmii2_tx_threshold;
    uint8_t gmii1_tx_threshold;
    uint8_t gmii0_tx_threshold;
} xport_mab_tx_threshold;

int ag_drv_xport_mab_ctrl_set(uint8_t xlmac_id, uint8_t tx_credit_disab, uint8_t tx_fifo_rst, uint8_t tx_port_rst, uint8_t rx_port_rst);
int ag_drv_xport_mab_ctrl_get(uint8_t xlmac_id, uint8_t *tx_credit_disab, uint8_t *tx_fifo_rst, uint8_t *tx_port_rst, uint8_t *rx_port_rst);
int ag_drv_xport_mab_tx_wrr_ctrl_set(uint8_t xlmac_id, const xport_mab_tx_wrr_ctrl *tx_wrr_ctrl);
int ag_drv_xport_mab_tx_wrr_ctrl_get(uint8_t xlmac_id, xport_mab_tx_wrr_ctrl *tx_wrr_ctrl);
int ag_drv_xport_mab_tx_threshold_set(uint8_t xlmac_id, const xport_mab_tx_threshold *tx_threshold);
int ag_drv_xport_mab_tx_threshold_get(uint8_t xlmac_id, xport_mab_tx_threshold *tx_threshold);
int ag_drv_xport_mab_status_get(uint8_t xlmac_id, uint8_t *tx_frm_underrun_vect, uint8_t *tx_outstanding_credits_cnt_underrun_vect, uint8_t *tx_fifo_overrun_vect, uint8_t *rx_fifo_overrun_vect);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_mab_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

