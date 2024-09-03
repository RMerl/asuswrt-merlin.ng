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

#ifndef _BCM6813_XPORT_TOP_AG_H_
#define _BCM6813_XPORT_TOP_AG_H_

int ag_drv_xport_top_ctrl_set(uint8_t xlmac_id, uint8_t p3_mode, uint8_t p2_mode, uint8_t p1_mode, uint8_t p0_mode);
int ag_drv_xport_top_ctrl_get(uint8_t xlmac_id, uint8_t *p3_mode, uint8_t *p2_mode, uint8_t *p1_mode, uint8_t *p0_mode);
int ag_drv_xport_top_status_get(uint8_t xlmac_id, uint8_t *link_status);
int ag_drv_xport_top_revision_get(uint8_t xlmac_id, uint32_t *xport_rev);
int ag_drv_xport_top_spare_cntrl_set(uint8_t xlmac_id, uint32_t spare_reg);
int ag_drv_xport_top_spare_cntrl_get(uint8_t xlmac_id, uint32_t *spare_reg);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_top_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

