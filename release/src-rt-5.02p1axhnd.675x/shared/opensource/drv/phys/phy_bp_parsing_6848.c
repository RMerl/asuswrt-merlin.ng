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
#include "mac_drv.h"
#include "mac_drv_unimac.h"
#include "phy_bp_parsing.h"
#include "boardparms.h"

static bus_type_t bp_parse_bus_type(const EMAC_PORT_INFO *port_info)
{
    bus_type_t bus_type = BUS_TYPE_UNKNOWN;
    uint32_t phy_id = port_info->phy_id;
    uint32_t intf = phy_id & MAC_IFACE;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    case MAC_IF_SERDES:
    case MAC_IF_SGMII:
    case MAC_IF_HSGMII:
    {
        bus_type = BUS_TYPE_6848_INT;
        break;
    }
    default:
        break;
    }

    return bus_type;
}

void bp_parse_phy_ext(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev)
{
    phy_dev_t *phy_sgmii;
    uint32_t phy_id = port_info->phy_id;
    uint32_t intf = phy_id & MAC_IFACE;

    if ((phy_id & PHY_EXTERNAL) && (intf == MAC_IF_SGMII || intf == MAC_IF_HSGMII))
    {
        phy_sgmii = phy_dev_add(PHY_TYPE_6848_SGMII, 6, NULL);
        phy_sgmii->phy_drv->bus_drv = bus_drv_get(BUS_TYPE_6848_INT);
        phy_sgmii->mii_type = intf == MAC_IF_SGMII ? PHY_MII_TYPE_SGMII : PHY_MII_TYPE_HSGMII;
        phy_drv_dev_add(phy_sgmii);
        phy_drv_init(phy_sgmii->phy_drv);
        phy_dev_init(phy_sgmii);
        phy_dev->cascade_next = phy_sgmii;
        phy_sgmii->cascade_prev = phy_dev;
    }
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
    uint32_t addr = phy_id & BCM_PHY_ID_M;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    {
        if (addr == 1 || addr == 2)
            phy_type = PHY_TYPE_6848_EGPHY;
        else if (addr == 3 || addr == 4)
            phy_type = PHY_TYPE_6848_EPHY;
        break;
    }
    case MAC_IF_SERDES:
    {
#ifndef _CFE_
        phy_type = PHY_TYPE_PCS;
        break;
#endif
    }
    case MAC_IF_SGMII:
    case MAC_IF_HSGMII:
    {
        if (phy_id & PHY_EXTERNAL)
            phy_type = PHY_TYPE_EXT1;
        else
            phy_type = PHY_TYPE_6848_SGMII;
        break;
    }
    default:
        break;
    }

    return phy_type;
}

void *bp_parse_phy_priv(const EMAC_PORT_INFO *port_info)
{
    uint32_t priv = 0;
    uint32_t phy_id = port_info->phy_id;
    uint32_t intf = phy_id & MAC_IFACE;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    {
        /* EGPHY ports 0,1 are connected to EMAC ports 0,1 */
        if (port_info->switch_port == 0 || port_info->switch_port == 1)
            priv = port_info->switch_port;

        /* EPHY ports 0,1 are connected to EMAC ports 2,3 */
        if (port_info->switch_port == 2 || port_info->switch_port == 3)
            priv = port_info->switch_port - 2;

        break;
    }
    default:
        break;
    }

    return (void *)priv;
}

mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    if (port == 4 || port > 5)
        return MAC_TYPE_UNKNOWN;

    return MAC_TYPE_UNIMAC;
}

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    uint32_t priv = 0;

    /* Set the gmii_direct bit in the unimac cfg register */
    if (port >= 0 && port <= 3)
        priv = UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT;

    return (void *)priv;
}
