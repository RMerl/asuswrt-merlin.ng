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


#ifndef _XRDP_DRV_XLIF_XRDP1_AG_H_
#define _XRDP_DRV_XLIF_XRDP1_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * disable: 
 *     Disable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_set(uint8_t channel_id, bdmf_boolean disable);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_get(uint8_t channel_id, bdmf_boolean *disable);

/**********************************************************************************************************************
 * oflw: 
 *     Overflow
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_if_oflw_flag_get(uint8_t channel_id, bdmf_boolean *oflw);

/**********************************************************************************************************************
 * err: 
 *     Error
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_if_err_flag_get(uint8_t channel_id, bdmf_boolean *err);

/**********************************************************************************************************************
 * pfc_en: 
 *     PFC_EN
 * llfc_en: 
 *     LLFC_en
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_set(uint8_t channel_id, bdmf_boolean pfc_en, bdmf_boolean llfc_en);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_get(uint8_t channel_id, bdmf_boolean *pfc_en, bdmf_boolean *llfc_en);

/**********************************************************************************************************************
 * value: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_set(uint8_t channel_id, uint16_t value);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_get(uint8_t channel_id, uint16_t *value);

/**********************************************************************************************************************
 * disable_with_credits: 
 *     Disable_With_Credits
 * disable_wo_credits: 
 *     Disable_WO_Credits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_set(uint8_t channel_id, bdmf_boolean disable_with_credits, bdmf_boolean disable_wo_credits);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_get(uint8_t channel_id, bdmf_boolean *disable_with_credits, bdmf_boolean *disable_wo_credits);

/**********************************************************************************************************************
 * value: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_read_credits_get(uint8_t channel_id, uint16_t *value);

/**********************************************************************************************************************
 * value: 
 *     Value
 * en: 
 *     enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_set(uint8_t channel_id, uint16_t value, bdmf_boolean en);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_get(uint8_t channel_id, uint16_t *value, bdmf_boolean *en);

/**********************************************************************************************************************
 * mac_txerr: 
 *     mac_txerr
 * mac_txcrcerr: 
 *     mac_txcrcerr
 * mac_txosts_sinext: 
 *     mac_txosts_sinext
 * mac_txcrcmode: 
 *     mac_txcrcmode
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_set(uint8_t channel_id, bdmf_boolean mac_txerr, bdmf_boolean mac_txcrcerr, bdmf_boolean mac_txosts_sinext, uint8_t mac_txcrcmode);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_get(uint8_t channel_id, bdmf_boolean *mac_txerr, bdmf_boolean *mac_txcrcerr, bdmf_boolean *mac_txosts_sinext, uint8_t *mac_txcrcmode);

/**********************************************************************************************************************
 * enable: 
 *     Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_set(uint8_t channel_id, bdmf_boolean enable);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_get(uint8_t channel_id, bdmf_boolean *enable);

/**********************************************************************************************************************
 * value: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_set(uint8_t channel_id, uint8_t value);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_get(uint8_t channel_id, uint8_t *value);

/**********************************************************************************************************************
 * value: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_set(uint8_t channel_id, bdmf_boolean value);
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_get(uint8_t channel_id, bdmf_boolean *value);

/**********************************************************************************************************************
 * pfc_en: 
 *     PFC_EN
 * llfc_en: 
 *     LLFC_en
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_en_stat_get(uint8_t channel_id, bdmf_boolean *pfc_en, bdmf_boolean *llfc_en);

/**********************************************************************************************************************
 * value: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_stat_get(uint8_t channel_id, uint16_t *value);

/**********************************************************************************************************************
 * select_module: 
 *     RX_TX selection
 *     00 - RX
 *     10 - TX
 * select_lane: 
 *     Select_lane
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_debug_bus_sel_set(uint8_t channel_id, uint8_t select_module, uint8_t select_lane);
bdmf_error_t ag_drv_xlif_xrdp1_channel_debug_bus_sel_get(uint8_t channel_id, uint8_t *select_module, uint8_t *select_lane);

/**********************************************************************************************************************
 * lpi_rx_detect: 
 *     lpi_rx_detect
 * lpi_tx_detect: 
 *     lpi_tx_detect
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_eee_ind_get(uint8_t channel_id, bdmf_boolean *lpi_rx_detect, bdmf_boolean *lpi_tx_detect);

/**********************************************************************************************************************
 * q_off: 
 *     Q_OFF
 * failover_on: 
 *     Failover_on
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xlif_xrdp1_channel_q_off_ind_get(uint8_t channel_id, uint8_t *q_off, bdmf_boolean *failover_on);

#ifdef USE_BDMF_SHELL
enum
{
    cli_xlif_xrdp1_channel_xlif_rx_if_if_dis,
    cli_xlif_xrdp1_channel_xlif_rx_if_oflw_flag,
    cli_xlif_xrdp1_channel_xlif_rx_if_err_flag,
    cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en,
    cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap,
    cli_xlif_xrdp1_channel_xlif_tx_if_if_enable,
    cli_xlif_xrdp1_channel_xlif_tx_if_read_credits,
    cli_xlif_xrdp1_channel_xlif_tx_if_set_credits,
    cli_xlif_xrdp1_channel_xlif_tx_if_out_ctrl,
    cli_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable,
    cli_xlif_xrdp1_channel_xlif_tx_if_tx_threshold,
    cli_xlif_xrdp1_channel_xlif_tx_if_tdm_mode,
    cli_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_en_stat,
    cli_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_stat,
    cli_xlif_xrdp1_channel_debug_bus_sel,
    cli_xlif_xrdp1_channel_xlif_eee_ind,
    cli_xlif_xrdp1_channel_q_off_ind,
};

int bcm_xlif_xrdp1_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_xlif_xrdp1_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
