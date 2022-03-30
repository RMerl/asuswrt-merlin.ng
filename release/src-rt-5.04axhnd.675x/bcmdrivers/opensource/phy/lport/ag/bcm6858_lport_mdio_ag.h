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

#ifndef _BCM6858_LPORT_MDIO_AG_H_
#define _BCM6858_LPORT_MDIO_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t start_busy;
    uint8_t fail;
    uint8_t op_code;
    uint8_t phy_prt_addr;
    uint8_t reg_dev_addr;
    uint16_t data_addr;
} lport_mdio_control;

int ag_drv_lport_mdio_control_set(const lport_mdio_control *control);
int ag_drv_lport_mdio_control_get(lport_mdio_control *control);
int ag_drv_lport_mdio_cfg_set(uint8_t free_run_clk_enable, uint8_t supress_preamble, uint8_t mdio_clk_divider, uint8_t mdio_clause);
int ag_drv_lport_mdio_cfg_get(uint8_t *free_run_clk_enable, uint8_t *supress_preamble, uint8_t *mdio_clk_divider, uint8_t *mdio_clause);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lport_mdio_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

