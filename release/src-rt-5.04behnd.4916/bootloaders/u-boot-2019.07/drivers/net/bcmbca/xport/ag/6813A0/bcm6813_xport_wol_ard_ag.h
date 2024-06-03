// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

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

