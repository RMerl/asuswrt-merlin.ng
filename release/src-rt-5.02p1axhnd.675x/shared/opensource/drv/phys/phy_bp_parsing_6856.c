/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

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
 *  Created on: Aug 2016
 *      Author: yuval.raviv@broadcom.com
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
    case MAC_IF_SGMII:
    case MAC_IF_HSGMII:
    case MAC_IF_XFI:
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
    phy_dev_t *phy_sgmii;
    uint32_t phy_id = port_info->phy_id;
    uint32_t intf = phy_id & MAC_IFACE;

    if ((port_info->switch_port == 5) && (phy_id & PHY_EXTERNAL) && (intf == MAC_IF_SGMII || intf == MAC_IF_HSGMII))
    {
        phy_sgmii = phy_dev_add(PHY_TYPE_6856_SGMII, 6, NULL);
        phy_sgmii->phy_drv->bus_drv = bus_drv_get(BUS_TYPE_6846_INT);
        phy_sgmii->mii_type = intf == MAC_IF_SGMII ? PHY_MII_TYPE_SGMII : PHY_MII_TYPE_HSGMII;
        phy_drv_dev_add(phy_sgmii);
        phy_drv_init(phy_sgmii->phy_drv);
        phy_dev->cascade_prev = phy_sgmii;
        phy_sgmii->cascade_next = phy_dev;
    }

    if (port_info->switch_port == 8)
        phy_dev->disable_hd = 1;
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
    case MAC_IF_SGMII:
    case MAC_IF_HSGMII:
    {
        if (phy_id & PHY_EXTERNAL)
            phy_type = PHY_TYPE_EXT3;
        else
            phy_type = PHY_TYPE_6856_SGMII;
        break;
    }
    case MAC_IF_XFI:
    {
        if (phy_id & PHY_EXTERNAL)
            phy_type = PHY_TYPE_EXT3;
        break;
    }
    default:
        break;
    }

    return phy_type;
}

void *bp_parse_phy_priv(const EMAC_PORT_INFO *port_info)
{
    return (void *)(uint64_t)port_info->switch_port;
}

mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    return port < 6 ? MAC_TYPE_UNIMAC : MAC_TYPE_UNKNOWN;
}

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    if (port > 5)
        return NULL;

    /* Set the gmii_direct bit in the unimac cfg register */
    return (void *)UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT;
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
