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

#include "phy_bp_parsing.h"
#include "phy_drv.h"
#include "boardparms.h"

#if !defined(DSL_DEVICES)
uint32_t bp_parse_phy_addr(const EMAC_PORT_INFO *port_info)
{
    return port_info->phy_id;
}
#endif

static phy_mii_type_t bp_parse_mii_type(const EMAC_PORT_INFO *port_info)
{
    uint32_t phy_id = port_info->phy_id;
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
    case MAC_IF_INVALID:    /* treat as GMII as default, if not specified */
    case MAC_IF_GMII:
        mii_type = PHY_MII_TYPE_GMII;
        break;
    case MAC_IF_RGMII_1P8V:
    case MAC_IF_RGMII_2P5V:
    case MAC_IF_RGMII_3P3V:
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

static void bp_parse_port_flags(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev)
{
    int port_flags = port_info->port_flags;

    phy_dev->delay_rx = !IsPortRxInternalDelay(port_flags);
    phy_dev->delay_tx = !IsPortTxInternalDelay(port_flags);

    phy_dev->swap_pair = IsPortSwapPair(port_flags);
}

static void bp_parse_reset_gpio(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev)
{
    if (port_info->phyReset == BP_GPIO_NONE)
        phy_dev->reset_gpio = -1;
    else {
        phy_dev->reset_gpio = port_info->phyReset & BP_GPIO_NUM_MASK;
        phy_dev->reset_gpio_active_hi = (port_info->phyReset & BP_ACTIVE_MASK) == BP_ACTIVE_HIGH;
    }
}

static void bp_parse_idle_stuffing(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev)
{
    if (port_info->port_flags & PORT_FLAG_WAN_ONLY)
        phy_dev->idle_stuffing = 0;
    else
        phy_dev->idle_stuffing = (phy_dev->mii_type == PHY_MII_TYPE_XFI);
}

phy_dev_t *bp_parse_phy_dev(const EMAC_PORT_INFO *port_info)
{
    phy_dev_t *phy_dev = NULL;
    phy_type_t phy_type;
    uint32_t addr, meta_id;
    void *priv;

    phy_type = bp_parse_phy_type(port_info);

    if (phy_type == PHY_TYPE_UNKNOWN)
        goto Exit;

    meta_id = bp_parse_phy_addr(port_info);
    addr = meta_id & BCM_PHY_ID_M;
    priv = bp_parse_phy_priv(port_info);

    if ((phy_dev = phy_dev_add(phy_type, addr, priv)) == NULL)
        goto Exit;

    phy_dev->meta_id = meta_id;
    phy_dev->mii_type = bp_parse_mii_type(port_info);
    bp_parse_port_flags(port_info, phy_dev);
    bp_parse_phy_driver(port_info, phy_dev->phy_drv);
    bp_parse_phy_ext(port_info, phy_dev);
    bp_parse_idle_stuffing(port_info, phy_dev);
    bp_parse_reset_gpio(port_info, phy_dev);

Exit:
    return phy_dev;
}
EXPORT_SYMBOL(bp_parse_phy_dev);

#if !(defined(CONFIG_BCM96838) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878))
void bp_parse_mac_ext(const ETHERNET_MAC_INFO *emac_info, uint32_t port, mac_dev_t *mac_dev)
{
    /* stub for chips that do not implement this function */
}
#endif

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

    bp_parse_mac_ext(emac_info, port, mac_dev);

Exit:
    return mac_dev;
}
EXPORT_SYMBOL(bp_parse_mac_dev);
