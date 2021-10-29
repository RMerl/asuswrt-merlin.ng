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

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __PHYS_COMMON_DRV_H
#define __PHYS_COMMON_DRV_H

#define PHY_LINK_ON             1
#define PHY_LINK_OFF            0

typedef enum
{
    PHY_RATE_10_FULL,
    PHY_RATE_10_HALF,
    PHY_RATE_100_FULL,
    PHY_RATE_100_HALF,
    PHY_RATE_1000_FULL,
    PHY_RATE_1000_HALF,
    PHY_RATE_2500_FULL,
    PHY_RATE_LINK_DOWN,
    PHY_RATE_ERR,
} phy_rate_t;

void port_phy_init(uint32_t port);
phy_rate_t port_get_link_speed(uint32_t port);
uint32_t port_get_link_status(uint32_t port);
uint16_t phy_read_register(uint32_t addr, uint16_t reg);
int32_t phy_write_register (uint32_t addr,uint16_t reg, uint16_t val);

#endif
