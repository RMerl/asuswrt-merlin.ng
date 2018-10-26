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
 *  Created on: Nov 2016
 *      Author: steven.hsieh@broadcom.com
 */

#include "phy_drv.h"
#include "mac_drv.h"
#include "mac_drv_unimac.h"
#include "phy_bp_parsing.h"
#include "boardparms.h"
#include "bcm_gpio.h"

static bus_type_t bp_parse_bus_type(const EMAC_PORT_INFO *port_info)
{
    return BUS_TYPE_DSL_ETHSW;
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
    uint32_t phy_id;
    uint32_t intf;

    phy_id = port_info->phy_id;

    intf = phy_id & MAC_IFACE;

    // no phy connection
#ifdef EXT_BCM53134
    if ((phy_id & BCM_PHY_ID_M) == 0 && IsMacToMac(phy_id))
	return IsPortConnectedToExternalSwitch(phy_id) ? PHY_TYPE_UNKNOWN : PHY_TYPE_MAC2MAC;
    if (phy_id == BP_PHY_ID_NOT_SPECIFIED) return PHY_TYPE_UNKNOWN;
#else
    if (IsMacToMac(phy_id)) 
        return IsPortConnectedToExternalSwitch(phy_id) ? PHY_TYPE_UNKNOWN : PHY_TYPE_MAC2MAC;
    if (phy_id == BP_PHY_ID_NOT_SPECIFIED) return PHY_TYPE_UNKNOWN;
#endif
    switch (intf)
    {
#if EXT_BCM53134
    	case MAC_IF_RGMII:
            phy_type = PHY_TYPE_53125;
            printk("%s MAC_IF_RGMII intf, phy type PHY_TYPE_53125\n", __FUNCTION__);
            break;
#endif
        case MAC_IF_RGMII_1P8V:
        case MAC_IF_RGMII_3P3V:
        case MAC_IF_INVALID:    /* treat as GMII as default, if not specified */
        case MAC_IF_GMII:
            phy_type = PHY_TYPE_SF2_GPHY;
            break;
        case PHY_TYPE_CL45GPHY:
            phy_type = PHY_TYPE_EXT3;
            break;
        case MAC_IF_SERDES:
            phy_type = PHY_TYPE_SF2_SERDES;
            break;
    }

    return phy_type;
}

void *bp_parse_phy_priv(const EMAC_PORT_INFO *port_info)
{
    void *priv = 0;

    return priv;
}

mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    switch (emac_info->ucPhyType)
    {
    case BP_ENET_NO_PHY: // runner
        if (port < 4) return MAC_TYPE_UNIMAC;
        break;
    case BP_ENET_EXTERNAL_SWITCH:   // sf2
        if (port < 9) return MAC_TYPE_SF2;
        break;
    }
    return MAC_TYPE_UNKNOWN;
}

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port)
{
    switch (emac_info->ucPhyType)
    {
    case BP_ENET_NO_PHY: // runner
        // unimac
        if (port < 3) return (void *)(UNIMAC_DRV_PRIV_FLAG_EXTSW_CONNECTED|UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT);
        return (void *)UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT;
    }
    return (void *) 0;
}

static void bp_phy_external_reset(uint16_t resetGpio)
{
    /* keep reset over 200 us */
    if (resetGpio & BP_ACTIVE_LOW)
        bcm_gpio_set_data((resetGpio & BP_GPIO_NUM_MASK), 0);
    else
        bcm_gpio_set_data((resetGpio & BP_GPIO_NUM_MASK), 1);
    bcm_gpio_set_dir(resetGpio, 1);
    udelay(250);

    /* release reset */
    if (resetGpio & BP_ACTIVE_LOW)
        bcm_gpio_set_data((resetGpio & BP_GPIO_NUM_MASK), 1);
    else
        bcm_gpio_set_data((resetGpio & BP_GPIO_NUM_MASK), 0);
    udelay(10);
}

void bp_parse_phy_ext(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev)
{
    EMAC_PORT_INFO ext_info;
    phy_dev_t *phy_ext_dev;

    if (port_info->phy_id_ext > 0)
    {
        memcpy(&ext_info, port_info, sizeof(ext_info));
        ext_info.phy_id = port_info->phy_id_ext;
        // assuming the cascaded phy has only one more member
        ext_info.phy_id_ext = 0;
        if ( (phy_ext_dev = bp_parse_phy_dev(&ext_info)) == NULL)
        {
            printk("Failed to create cascaded phy devices\n");
            return;
        }
        phy_dev->cascade_next = phy_ext_dev;
        phy_ext_dev->cascade_prev = phy_dev;

        /* perform reset if reset gpio pin is defined */
        if (port_info->phyReset != BP_GPIO_NONE)
            bp_phy_external_reset(port_info->phyReset);
    }
}

uint32_t bp_parse_phy_addr(const EMAC_PORT_INFO *port_info)
{
#ifdef EXT_BCM53134
	return port_info->phy_id;
#else
    uint32_t meta_id = port_info->phy_id;
    
    // for mac2mac, use port number as address
    if (IsMacToMac(meta_id))
    {
        meta_id &= ~BP_PHY_ID_NOT_SPECIFIED;
        meta_id |= port_info->switch_port;
    }
    return meta_id;
#endif
}
