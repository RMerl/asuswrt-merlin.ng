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

#ifndef _EPON_LINK_ARRAY_AG_H_
#define _EPON_LINK_ARRAY_AG_H_
bdmf_error_t ag_drv_epn_burst_cap_set(uint8_t link_idx, uint32_t burstcap);
bdmf_error_t ag_drv_epn_burst_cap_get(uint8_t link_idx, uint32_t *burstcap);
bdmf_error_t ag_drv_epn_queue_llid_map_set(uint8_t que_idx, uint8_t quellidmap);
bdmf_error_t ag_drv_epn_queue_llid_map_get(uint8_t que_idx, uint8_t *quellidmap);
bdmf_error_t ag_drv_epn_unused_tq_cnt_set(uint8_t link_idx, uint32_t unusedtqcnt);
bdmf_error_t ag_drv_epn_unused_tq_cnt_get(uint8_t link_idx, uint32_t *unusedtqcnt);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_set(uint8_t shaper_idx, uint32_t cfgshpmask);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_get(uint8_t shaper_idx, uint32_t *cfgshpmask);
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
#endif
