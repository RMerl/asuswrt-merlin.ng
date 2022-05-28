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

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#include "phy_drv.h"
#include "phy_drv_lport.h"
#include "phy_bp_parsing.h"
#include "boardparms.h"
#include "lport_drv.h"

static bus_type_t bp_parse_bus_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    bus_type_t bus_type = BUS_TYPE_UNKNOWN;
    uint32_t phy_id = emac_info->sw.phy_id[port];
    uint32_t intf = phy_id & MAC_IFACE;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    case MAC_IF_RGMII:
    {
        bus_type = BUS_TYPE_6858_LPORT;
        break;
    }
    case MAC_IF_SGMII:
    case MAC_IF_HSGMII:
    case MAC_IF_XFI:
    {
        if (phy_id & PHY_EXTERNAL)
            bus_type = BUS_TYPE_6858_LPORT;
        break;
    }
    default:
        break;
    }

    return bus_type;
}

void bp_parse_phy_driver(const ETHERNET_MAC_INFO *emac_info, uint32_t port, phy_drv_t *phy_drv)
{
    bus_type_t bus_type;

    if ((bus_type = bp_parse_bus_type(emac_info, port)) != BUS_TYPE_UNKNOWN)
        phy_drv->bus_drv = bus_drv_get(bus_type);

    if (port >= 0 && port <= 7)
    {
#ifndef _CFE_
        phy_drv->link_change_register = lport_link_change_register;
        phy_drv->link_change_unregister = lport_link_change_unregister;
#endif
    }
}

phy_type_t bp_parse_phy_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    phy_type_t phy_type = PHY_TYPE_UNKNOWN;
    uint32_t phy_id = emac_info->sw.phy_id[port];
    uint32_t intf = phy_id & MAC_IFACE;
    uint32_t addr = phy_id & BCM_PHY_ID_M;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    {
        phy_type = PHY_TYPE_6858_EGPHY;
        break;
    }
    case MAC_IF_RGMII:
    {
        if (phy_id & PHY_EXTERNAL)
        {
            if (addr == 0x18 || addr == 0x19)
                phy_type = PHY_TYPE_EXT2;
            else
                phy_type = PHY_TYPE_EXT1;
        }
        break;
    }
    case MAC_IF_SGMII:
    case MAC_IF_HSGMII:
    case MAC_IF_XFI:
    {
        if (phy_id & PHY_EXTERNAL)
            phy_type = PHY_TYPE_EXT3;
        else
            phy_type = PHY_TYPE_LPORT_SERDES;
        break;
    }
    case MAC_IF_SERDES:
    default:
        break;
    }

    return phy_type;
}

void *bp_parse_phy_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    return (void *)(uint64_t)port;
}

mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    return port < 8 ? MAC_TYPE_LPORT : MAC_TYPE_UNKNOWN;
}

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    LPORT_PORT_MUX_SELECT mux_sel = PORT_UNAVAIL;
    uint32_t phy_id = emac_info->sw.phy_id[port];
    uint32_t intf = phy_id & MAC_IFACE;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    {
        mux_sel = PORT_GPHY;
        break;
    }
    case MAC_IF_RGMII:
    {
        mux_sel = PORT_RGMII;
        break;
    }
    case MAC_IF_SGMII:
    {
        if (phy_id & PHY_EXTERNAL)
            mux_sel = PORT_SGMII_SLAVE;
        else
            mux_sel = PORT_SGMII_1000BASE_X;

        break;
    }
    case MAC_IF_HSGMII:
    {
        mux_sel = PORT_HSGMII;
        break;
    }
    case MAC_IF_XFI:
    {
        mux_sel = PORT_XFI;
        break;
    }
    default:
        break;
    }

    return (void *)mux_sel;
}
