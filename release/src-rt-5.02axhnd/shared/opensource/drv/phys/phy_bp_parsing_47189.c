/*
   <:copyright-BRCM:2016:DUAL/GPL:standard
   
      Copyright (c) 2016 Broadcom 
      All Rights Reserved
   
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
 * BCM947189-specific boardparms parsing for enet driver impl7
 */

#include "phy_drv.h"
#include "phy_drv_lport.h"
#include "phy_bp_parsing.h"
#include "boardparms.h"
#include "lport_drv.h"


static bus_type_t bp_parse_bus_type(const EMAC_PORT_INFO *port_info)
{
    if (port_info->switch_port == 0) {
        return BUS_TYPE_47189_GMAC0;
    } else if (port_info->switch_port == 1) {
        return BUS_TYPE_47189_GMAC1;
    } else {
        printk("[bp_parse_bus_type] ERROR: port number %d doesn't have a "
               "matching bus type\n", port_info->switch_port);
    }

    return 0;
}

void bp_parse_phy_ext(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev)
{
}

void bp_parse_phy_driver(const EMAC_PORT_INFO *port_info, phy_drv_t *phy_drv)
{
    bus_type_t bus_type;

    if ((bus_type = bp_parse_bus_type(port_info)) != BUS_TYPE_UNKNOWN)
        phy_drv->bus_drv = bus_drv_get(bus_type);
}

phy_type_t bp_parse_phy_type(const EMAC_PORT_INFO *port_info)
{
    phy_type_t phy_type = PHY_TYPE_UNKNOWN;

    switch (port_info->phyconn) {
    case PHY_CONN_TYPE_EXT_SW:
        phy_type = PHY_TYPE_53125;
        break;
    case PHY_CONN_TYPE_EXT_PHY:
        phy_type = PHY_TYPE_EXT1;
        break;
    case PHY_CONN_TYPE_PLC:
        break;
    default:
        printk("[bp_parse_phy_type] PhyConnType unknown or not implemented (port %d)", port_info->switch_port);
    }

    return phy_type;
}

void *bp_parse_phy_priv(const EMAC_PORT_INFO *port_info)
{
    return (void *)port_info->switch_port;
}

mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    /* Assume GMAC always */
    return MAC_TYPE_GMAC;
}

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    /* TODO: What to use as the mac private field? */
    return 0;
}
