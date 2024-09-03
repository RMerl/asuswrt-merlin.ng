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

#ifndef _EPON_LINK_ARRAY_AG_H_
#define _EPON_LINK_ARRAY_AG_H_
bdmf_error_t ag_drv_epn_burst_cap_set(uint8_t link_idx, uint32_t burstcap);
bdmf_error_t ag_drv_epn_burst_cap_get(uint8_t link_idx, uint32_t *burstcap);
bdmf_error_t ag_drv_epn_queue_llid_map_set(uint8_t que_idx, uint8_t quellidmap);
bdmf_error_t ag_drv_epn_queue_llid_map_get(uint8_t que_idx, uint8_t *quellidmap);
bdmf_error_t ag_drv_epn_unused_tq_cnt_set(uint8_t link_idx, uint32_t unusedtqcnt);
bdmf_error_t ag_drv_epn_unused_tq_cnt_get(uint8_t link_idx, uint32_t *unusedtqcnt);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_set(uint8_t shaper_idx, uint16_t cfgshpmask);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_get(uint8_t shaper_idx, uint16_t *cfgshpmask);
bdmf_error_t ag_drv_epn_tx_l2s_queue_config_set(uint8_t que_idx, uint16_t cfgl2squeend, uint16_t cfgl2squestart);
bdmf_error_t ag_drv_epn_tx_l2s_queue_config_get(uint8_t que_idx, uint16_t *cfgl2squeend, uint16_t *cfgl2squestart);
bdmf_error_t ag_drv_epn_tx_ctc_burst_limit_set(uint8_t l2_queue_idx, uint32_t prvburstlimit);
bdmf_error_t ag_drv_epn_tx_ctc_burst_limit_get(uint8_t l2_queue_idx, uint32_t *prvburstlimit);
bdmf_error_t ag_drv_lif_llid_set(uint8_t llid_index, uint32_t cfgllid);
bdmf_error_t ag_drv_lif_llid_get(uint8_t llid_index, uint32_t *cfgllid);
bdmf_error_t ag_drv_xif_llid__set(uint8_t llid_index, uint32_t cfgonullid);
bdmf_error_t ag_drv_xif_llid__get(uint8_t llid_index, uint32_t *cfgonullid);
bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_set(uint8_t llid_index, uint32_t cfgp2psci_lo);
bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_get(uint8_t llid_index, uint32_t *cfgp2psci_lo);
bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_set(uint8_t llid_index, uint32_t cfgp2psci_hi);
bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_get(uint8_t llid_index, uint32_t *cfgp2psci_hi);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_set(uint8_t llid_index, uint32_t cfgp2psci_lo);
bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_get(uint8_t llid_index, uint32_t *cfgp2psci_lo);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_set(uint8_t llid_index, uint32_t cfgp2psci_hi);
bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_get(uint8_t llid_index, uint32_t *cfgp2psci_hi);
bdmf_error_t ag_drv_lif_data_port_data_set(uint8_t portidx, uint32_t pbiportdata);
bdmf_error_t ag_drv_lif_data_port_data_get(uint8_t portidx, uint32_t *pbiportdata);
bdmf_error_t ag_drv_xif_port_data__set(uint8_t portidx, uint32_t portdata);
bdmf_error_t ag_drv_xif_port_data__get(uint8_t portidx, uint32_t *portdata);
#endif
