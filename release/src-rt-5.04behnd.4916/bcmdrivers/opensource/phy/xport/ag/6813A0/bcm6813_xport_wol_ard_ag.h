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

#ifndef _BCM6813_XPORT_WOL_ARD_AG_H_
#define _BCM6813_XPORT_WOL_ARD_AG_H_

int ag_drv_xport_wol_ard_config_set(uint8_t xlmac_id, uint8_t has_brcm_tag);
int ag_drv_xport_wol_ard_config_get(uint8_t xlmac_id, uint8_t *has_brcm_tag);
int ag_drv_xport_wol_ard_control_set(uint8_t xlmac_id, uint8_t ard_en);
int ag_drv_xport_wol_ard_control_get(uint8_t xlmac_id, uint8_t *ard_en);
int ag_drv_xport_wol_ard_status_get(uint8_t xlmac_id, uint8_t *ar_detected);
int ag_drv_xport_wol_ard_custom_tag_cfg_set(uint8_t xlmac_id, uint16_t ethertype1, uint16_t ethertype2);
int ag_drv_xport_wol_ard_custom_tag_cfg_get(uint8_t xlmac_id, uint16_t *ethertype1, uint16_t *ethertype2);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_wol_ard_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

