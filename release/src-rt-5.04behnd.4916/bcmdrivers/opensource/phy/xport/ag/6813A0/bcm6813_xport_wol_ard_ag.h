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

