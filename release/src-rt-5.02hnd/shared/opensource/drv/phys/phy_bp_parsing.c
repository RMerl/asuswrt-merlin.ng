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

#include "phy_bp_parsing.h"
#include "phy_drv.h"
#include "boardparms.h"

static uint32_t bp_parse_phy_addr(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    return emac_info->sw.phy_id[port] & BCM_PHY_ID_M;
}

static phy_mii_type_t bp_parse_mii_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    uint32_t phy_id = emac_info->sw.phy_id[port];
    uint32_t intf = phy_id & MAC_IFACE;
    phy_mii_type_t mii_type = PHY_MII_TYPE_UNKNOWN;

    switch (intf)
    {
    case MAC_IF_MII:
        mii_type = PHY_MII_TYPE_MII;
        break;
    case MAC_IF_TMII:
        mii_type = PHY_MII_TYPE_TMII;
        break;
    case MAC_IF_GMII:
        mii_type = PHY_MII_TYPE_GMII;
        break;
    case MAC_IF_RGMII:
        mii_type = PHY_MII_TYPE_RGMII;
        break;
    case MAC_IF_SGMII:
        mii_type = PHY_MII_TYPE_SGMII;
        break;
    case MAC_IF_HSGMII:
        mii_type = PHY_MII_TYPE_HSGMII;
        break;
    case MAC_IF_SERDES:
        mii_type = PHY_MII_TYPE_SERDES;
        break;
    case MAC_IF_XFI:
        mii_type = PHY_MII_TYPE_XFI;
        break;
    default:
        mii_type = PHY_MII_TYPE_UNKNOWN;
        break;
    }

    return mii_type;
}

phy_dev_t *bp_parse_phy_dev(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    phy_dev_t *phy_dev = NULL;
    phy_type_t phy_type;
    uint32_t addr;
    void *priv;

    if ((phy_type = bp_parse_phy_type(emac_info, port)) == PHY_TYPE_UNKNOWN)
        goto Exit;

    if ((addr = bp_parse_phy_addr(emac_info, port)) == 0)
        goto Exit;

    priv = bp_parse_phy_priv(emac_info, port);

    if ((phy_dev = phy_dev_add(phy_type, addr, priv)) == NULL)
        goto Exit;

    phy_dev->mii_type = bp_parse_mii_type(emac_info, port);

    bp_parse_phy_driver(emac_info, port, phy_dev->phy_drv);

Exit:
    return phy_dev;
}
EXPORT_SYMBOL(bp_parse_phy_dev);

mac_dev_t *bp_parse_mac_dev(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    mac_dev_t *mac_dev = NULL;
    mac_type_t mac_type;
    void *priv;

    if ((mac_type = bp_parse_mac_type(emac_info, port)) == MAC_TYPE_UNKNOWN)
        goto Exit;

    priv = bp_parse_mac_priv(emac_info, port);

    if ((mac_dev = mac_dev_add(mac_type, port, priv)) == NULL)
        goto Exit;

Exit:
    return mac_dev;
}
EXPORT_SYMBOL(bp_parse_mac_dev);
