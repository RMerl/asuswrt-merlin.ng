/*
   Copyright (c) 2015 Broadcom
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

#ifndef _BCM6888_XPORT_MPD_AG_H_
#define _BCM6888_XPORT_MPD_AG_H_

int ag_drv_xport_wol_mpd_config_set(uint8_t xlmac_id, uint8_t psw_en, uint8_t mseq_len);
int ag_drv_xport_wol_mpd_config_get(uint8_t xlmac_id, uint8_t *psw_en, uint8_t *mseq_len);
int ag_drv_xport_wol_mpd_control_set(uint8_t xlmac_id, uint8_t mpd_en);
int ag_drv_xport_wol_mpd_control_get(uint8_t xlmac_id, uint8_t *mpd_en);
int ag_drv_xport_wol_mpd_status_get(uint8_t xlmac_id, uint8_t *mp_detected);
int ag_drv_xport_wol_mpd_mseq_mac_da_low_set(uint8_t xlmac_id, uint32_t mac_da_31_0);
int ag_drv_xport_wol_mpd_mseq_mac_da_low_get(uint8_t xlmac_id, uint32_t *mac_da_31_0);
int ag_drv_xport_wol_mpd_mseq_mac_da_hi_set(uint8_t xlmac_id, uint16_t mac_da_47_32);
int ag_drv_xport_wol_mpd_mseq_mac_da_hi_get(uint8_t xlmac_id, uint16_t *mac_da_47_32);
int ag_drv_xport_wol_mpd_psw_low_set(uint8_t xlmac_id, uint32_t psw_31_0);
int ag_drv_xport_wol_mpd_psw_low_get(uint8_t xlmac_id, uint32_t *psw_31_0);
int ag_drv_xport_wol_mpd_psw_hi_set(uint8_t xlmac_id, uint16_t psw_47_32);
int ag_drv_xport_wol_mpd_psw_hi_get(uint8_t xlmac_id, uint16_t *psw_47_32);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_wol_mpd_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

