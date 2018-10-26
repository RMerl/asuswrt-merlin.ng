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
#include "rdp_drv_bbh.h"
#include "access_macros.h"

static bus_type_t bp_parse_bus_type(const EMAC_PORT_INFO *port_info)
{
    bus_type_t bus_type = BUS_TYPE_UNKNOWN;
    uint32_t phy_id = port_info->phy_id;
    uint32_t intf = phy_id & MAC_IFACE;

    switch (intf)
    {
    case MAC_IF_MII:
    case MAC_IF_GMII:
    {
        bus_type = BUS_TYPE_6838_EGPHY;
        break;
    }
    case MAC_IF_RGMII:
    {
        if (phy_id & PHY_EXTERNAL)
            bus_type = BUS_TYPE_6838_EXT;
        break;
    }
    case MAC_IF_SERDES:
    {
        if (phy_id & PHY_EXTERNAL)
            bus_type = BUS_TYPE_6838_EXT;
        else
            bus_type = BUS_TYPE_6838_AE;
        break;
    }
    default:
        break;
    }

    return bus_type;
}

void bp_parse_phy_ext(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev)
{
#ifndef _CFE_
    phy_dev_t *phy_next;
    uint32_t phy_id = port_info->phy_id;
    uint32_t intf = phy_id & MAC_IFACE;

    if ((phy_id & PHY_EXTERNAL) && (intf == MAC_IF_SERDES))
    {
        phy_next = phy_dev_add(PHY_TYPE_PCS, 2, NULL);
        phy_next->phy_drv->bus_drv = bus_drv_get(BUS_TYPE_6838_AE);
        phy_next->mii_type = PHY_MII_TYPE_SGMII;
        phy_drv_init(phy_next->phy_drv);
        phy_dev_init(phy_next);
        phy_dev->cascade_next = phy_next;
        phy_next->cascade_prev = phy_dev;
    }
#endif
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
        phy_type = PHY_TYPE_6848_EGPHY;
        break;
    }
    case MAC_IF_RGMII:
    {
        if (phy_id & PHY_EXTERNAL)
            phy_type = PHY_TYPE_EXT1;
        break;
    }
    case MAC_IF_SERDES:
    {
#ifndef _CFE_
        if (phy_id & PHY_EXTERNAL)
            phy_type = PHY_TYPE_EXT1;
        else
            phy_type = PHY_TYPE_PCS;
#endif
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
        /* EGPHY ports 0,1,2,3 are connected to EMAC ports 0,1,2,3 */
        if (port_info->switch_port >= 0 && port_info->switch_port <= 3)
            priv = port_info->switch_port;
        break;
    }
    default:
        break;
    }

    return (void *)priv;
}

mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    if (port > 5)
        return MAC_TYPE_UNKNOWN;

    return MAC_TYPE_UNIMAC;
}

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    uint32_t priv = 0;

    /* Set the gmii_direct bit in the unimac cfg register */
    if (port >= 0 && port <= 4)
        priv = UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT;

    return (void *)priv;
}

#define EGPHY_RGMII_OUT_PORT_MODE_MII           2
#define EGPHY_RGMII_OUT_PORT_MODE_RGMII         3
#define EGPHY_RGMII_OUT_PORT_MODE_RVMII         4

#define EGPHY_RGMII_OUT_REF_25_MHZ              0
#define EGPHY_RGMII_OUT_REF_50_MHZ              1
#define EGPHY_RGMII_OUT_REF_OFFSET              3
#define EGPHY_RGMII_OUT_PORT_ID_OFFSET          4

void bp_parse_mac_ext(const ETHERNET_MAC_INFO *emac_info, uint32_t port, mac_dev_t *mac_dev)
{
    uint32_t phy_id = emac_info->sw.phy_id[port];
    uint32_t port_flags  = emac_info->sw.port_flags[port];
    uint32_t rgmii_out_reg = 0;

    if (IsRGMII(phy_id))
    {
        rgmii_out_reg = EGPHY_RGMII_OUT_PORT_MODE_RGMII | (EGPHY_RGMII_OUT_REF_50_MHZ << EGPHY_RGMII_OUT_REF_OFFSET);

        if (!IsPortTxInternalDelay(port_flags))
            rgmii_out_reg |= 1 << EGPHY_RGMII_OUT_PORT_ID_OFFSET;
        printk("Set EMAC4 as RGMII\n");
    }
    else if (IsTMII(phy_id))
    {
        rgmii_out_reg = EGPHY_RGMII_OUT_PORT_MODE_MII | (EGPHY_RGMII_OUT_REF_50_MHZ << EGPHY_RGMII_OUT_REF_OFFSET);
        printk("Set EMAC4 as TMII\n");
    }

    if (rgmii_out_reg)
        WRITE_32(EGPHY_UBUS_MISC_EGPHY_RGMII_OUT, rgmii_out_reg);
}
