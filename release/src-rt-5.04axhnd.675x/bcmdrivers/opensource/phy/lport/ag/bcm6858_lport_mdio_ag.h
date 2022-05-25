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

