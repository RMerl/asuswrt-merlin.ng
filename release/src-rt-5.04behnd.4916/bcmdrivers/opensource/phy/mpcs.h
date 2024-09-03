/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard
    
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

/*
 *  Created on: October 2021
 *      Author: ido.brezel@broadcom.com
 */

#ifndef _PHY_DRV_MPCS_H_
#define _PHY_DRV_MPCS_H_

typedef enum
{
    eSPEED_1_1,
    eSPEED_10_10,
    eSPEED_2_2,
    eSPEED_100m_100m,
    eSPEED_5_5,
} espeed_t;

struct mpcs_cfg_s
{
    espeed_t cfg_port_speed;

    int cfg_5g5g_mode;
    int cfg_5g5g_mode_vcoDiv2;
    int cfg_2p5g2p5g_mode;
    int cfg_2p5g2p5g_mode_vcoDiv4;

    int cfg_usxgmii_mode;
    int cfg_usxgmii_5g;
    int cfg_usxgmii_2p5g;
    int cfg_usxgmii_1g;
    int cfg_usxgmii_100m;
    int cfg_usxgmii_10m;
    int sgmiiMode_1000;
    int cfg_sgmii_1000m;
    int cfg_sgmii_100m;
    int cfg_sgmii_10m;
    int clk_mode_312;
    int cfg_2p5g_is_xgmii;
};

int mpcs_read(uint16_t reg, uint16_t *val);
int mpcs_write(uint16_t reg, uint16_t val);
void mpcs_init(struct mpcs_cfg_s *m);
int is_pcs_locked(void);
int is_pcs_state(void);
void mpcs_rx_reset(void);
void mpcs_wantop_force_lbe(void);

#endif
