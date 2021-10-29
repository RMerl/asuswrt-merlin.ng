/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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
 *  Created on: June 2017
 *      Author: dima.mamut@broadcom.com
 */

#include "phy_drv.h"
#include "phy_bp_parsing.h"
#include "mac_drv.h"
#include "mac_drv_unimac.h"
#include "boardparms.h"
#include "access_macros.h"
#include "bcm_map_part.h"

static bus_type_t bp_parse_bus_type(const EMAC_PORT_INFO *port_info)
{
    bus_type_t bus_type = BUS_TYPE_UNKNOWN;
    uint32_t phy_id = port_info->phy_id;
    uint32_t intf = phy_id & MAC_IFACE;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    case MAC_IF_RGMII:
    {
        bus_type = BUS_TYPE_6846_INT;
        break;
    }
    default:
        break;
    }

    return bus_type;
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
    uint32_t phy_id = port_info->phy_id;
    uint32_t intf = phy_id & MAC_IFACE;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    {
        phy_type = PHY_TYPE_6846_EGPHY;
        break;
    }
    case MAC_IF_RGMII:
    {
        if (phy_id & PHY_EXTERNAL)
            phy_type = PHY_TYPE_EXT1;
        break;
    }
    default:
        break;
    }

    return phy_type;
}

void *bp_parse_phy_priv(const EMAC_PORT_INFO *port_info)
{
    return (void *)(unsigned long)port_info->switch_port;
}

mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    return port < 5 ? MAC_TYPE_UNIMAC : MAC_TYPE_UNKNOWN;
}

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    uint32_t priv = 0;

    /* Set the gmii_direct bit in the unimac cfg register */
    if (port >= 0 && port <= 4)
        priv = UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT;

    return (void *)(unsigned long)priv;
}

#define RGMII_CTRL_REG              RGMII_BASE + 0x0000
#define RGMII_RX_CLOCK_DELAY_CNTRL  RGMII_BASE + 0x0008

void bp_parse_mac_ext(const ETHERNET_MAC_INFO *emac_info, uint32_t port, mac_dev_t *mac_dev)
{
    uint32_t phy_id = emac_info->sw.phy_id[port];
    uint32_t port_flags  = emac_info->sw.port_flags[port];
    uint32_t val;

    if (!IsRGMII(phy_id))
        return;

    READ_32(RGMII_CTRL_REG, val);
    val |= (1 << 0); /* RGMII_MODE_EN=1 */
    val &= ~(7 << 2); /* Clear PORT_MODE */
    val |= (3 << 2); /* RGMII mode */

    if (IsPortTxInternalDelay(port_flags))
        val &= ~(1 << 1); /* ID_MODE_DIS=0 */
    else
        val |= (1 << 1); /* ID_MODE_DIS=1 */

    WRITE_32(RGMII_CTRL_REG, val);

    if (IsPortRxInternalDelay(port_flags))
        val = 0x08;
    else
        val = 0x28;

    WRITE_32(RGMII_RX_CLOCK_DELAY_CNTRL, val);
}
