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

#ifndef __PHY_BP_PARSING_H__
#define __PHY_BP_PARSING_H__

#include "bus_drv.h"
#include "phy_drv.h"
#include "mac_drv.h"
#include "boardparms.h"

typedef ETHERNET_CROSSBAR_INFO EMAC_PORT_INFO;

void bp_parse_phy_driver(const EMAC_PORT_INFO *port_info, phy_drv_t *phy_drv);
void bp_parse_phy_ext(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev);
void bp_parse_mac_ext(const ETHERNET_MAC_INFO *emac_info, uint32_t port, mac_dev_t *mac_dev);

phy_type_t bp_parse_phy_type(const EMAC_PORT_INFO *port_info);
mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port);

mac_dev_t *bp_parse_mac_dev(const ETHERNET_MAC_INFO *emac_info, uint32_t port);
phy_dev_t *bp_parse_phy_dev(const EMAC_PORT_INFO *port_info);

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port);
void *bp_parse_phy_priv(const EMAC_PORT_INFO *port_info);

uint32_t bp_parse_phy_addr(const EMAC_PORT_INFO *port_info);

#endif
