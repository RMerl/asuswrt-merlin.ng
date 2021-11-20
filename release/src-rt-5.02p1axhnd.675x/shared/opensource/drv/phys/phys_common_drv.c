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

#include "mac_drv.h"
#include "phy_drv.h"
#include "phy_bp_parsing.h"
#include "phys_common_drv.h"
#include "boardparms.h"

static phy_dev_t *phy_devices[BP_MAX_SWITCH_PORTS] = {};
static mac_dev_t *mac_devices[BP_MAX_SWITCH_PORTS] = {};
static int is_initialized = 0;
static const ETHERNET_MAC_INFO *emac_info;

static void copy_port_info(const ETHERNET_MAC_INFO *emac_info, uint32_t port, EMAC_PORT_INFO *port_info)
{
    port_info->switch_port  = port;
    port_info->phy_id       = emac_info->sw.phy_id[port];
    port_info->phyconn      = emac_info->sw.phyconn[port];
    port_info->phy_devName  = emac_info->sw.phy_devName[port];
    memcpy(&(port_info->ledInfo), &(emac_info->sw.ledInfo[port]), sizeof(port_info->ledInfo));
    port_info->phyinit      = emac_info->sw.phyinit[port];
    port_info->port_flags   = emac_info->sw.port_flags[port];
    port_info->phyReset     = emac_info->sw.phyReset[port];
    port_info->oamIndex     = emac_info->sw.oamIndex[port];
    port_info->portMaxRate  = emac_info->sw.portMaxRate[port];
}

static void fill_devices(void)
{
    uint32_t iter;

    if (is_initialized)
        return;

    if ((emac_info = BpGetEthernetMacInfoArrayPtr()) == NULL)
    {
        printk("Error reading Ethernet MAC info from board params\n");
        return;
    }

    mac_drivers_set();
    phy_drivers_set();

    for (iter = 0; iter < BP_MAX_SWITCH_PORTS; iter++)
    {
        EMAC_PORT_INFO port_info = {};
        
        if (emac_info->sw.port_map & (1<<iter))
        {
            copy_port_info(emac_info, iter, &port_info);
            mac_devices[iter] = bp_parse_mac_dev(emac_info, iter);
            phy_devices[iter] = bp_parse_phy_dev(&port_info);
        }
    }

    mac_drivers_init();
    phy_drivers_init();

    for (iter = 0; iter < BP_MAX_SWITCH_PORTS; iter++)
    {
        mac_dev_t *mac_dev = mac_devices[iter];

        if (mac_dev)
        {
            mac_dev_init(mac_dev);
            mac_dev_enable(mac_dev);
        }
    }

    is_initialized  = 1;
}

static inline int port_exist(uint32_t port)
{
    fill_devices();

    if (!phy_devices[port])
        return 0;

    return 1;
}

void port_phy_init(uint32_t port)
{
    if (!port_exist(port))
        return;

    phy_dev_init(phy_devices[port]);
}
EXPORT_SYMBOL(port_phy_init);

phy_rate_t port_get_link_speed(uint32_t port)
{
    phy_rate_t rate = PHY_RATE_ERR;
    int link, speed, duplex;

    if (!port_exist(port))
        return PHY_RATE_ERR;

    if (phy_dev_read_status(phy_devices[port]))
        return PHY_RATE_ERR;

    link = phy_devices[port]->link;
    speed = phy_devices[port]->speed;
    duplex = phy_devices[port]->duplex;

    if (!link)
        return PHY_RATE_LINK_DOWN;

    if (speed == PHY_SPEED_2500)
    {
        rate = PHY_RATE_2500_FULL;
    }
    else if (speed == PHY_SPEED_1000)
    {
        if (duplex ==  PHY_DUPLEX_FULL)
            rate = PHY_RATE_1000_FULL;
        else
            rate = PHY_RATE_1000_HALF;
    }
    else if (speed == PHY_SPEED_100)
    {
        if (duplex ==  PHY_DUPLEX_FULL)
            rate = PHY_RATE_100_FULL;
        else
            rate = PHY_RATE_100_HALF;
    }
    else if (speed == PHY_SPEED_10)
    {
        if (duplex ==  PHY_DUPLEX_FULL)
            rate = PHY_RATE_10_FULL;
        else
            rate = PHY_RATE_10_HALF;
    }

    return rate;
}
EXPORT_SYMBOL(port_get_link_speed);

uint32_t port_get_link_status(uint32_t port)
{
    if (!port_exist(port))
        return 0;

    phy_dev_read_status(phy_devices[port]);

    return phy_devices[port]->link;
}
EXPORT_SYMBOL(port_get_link_status);

static void get_phy_dev_by_addr(uint32_t addr, phy_dev_t **phy_dev)
{
    int iter;

    for (iter = 0; iter < BP_MAX_SWITCH_PORTS; iter++)
    {
        if (phy_devices[iter] == NULL || phy_devices[iter]->phy_drv == NULL)
            continue;

        if (phy_devices[iter]->addr != (addr & BCM_PHY_ID_M))
            continue;
        
        if ((addr & PHY_EXTERNAL) && phy_devices[iter]->phy_drv->phy_type != PHY_TYPE_EXT1);
            continue;

        if (!(addr & PHY_EXTERNAL) && 
            phy_devices[iter]->phy_drv->phy_type != PHY_TYPE_6848_EPHY &&
            phy_devices[iter]->phy_drv->phy_type != PHY_TYPE_6848_EGPHY)
        {
            continue;
        }

        *phy_dev = phy_devices[iter];
    }
}

uint16_t phy_read_register(uint32_t addr, uint16_t reg)
{
    phy_dev_t *phy_dev = NULL;
    uint16_t val;

    get_phy_dev_by_addr(addr, &phy_dev);
    if (phy_dev && phy_dev_read(phy_dev, reg, &val) == 0)
        return val;

    return 0;
}
EXPORT_SYMBOL(phy_read_register);

int32_t phy_write_register(uint32_t addr,uint16_t reg, uint16_t val)
{
    phy_dev_t *phy_dev = NULL;

    get_phy_dev_by_addr(addr, &phy_dev);
    if (phy_dev)
        return phy_dev_write(phy_dev, reg, val);

    return 0;
}
EXPORT_SYMBOL(phy_write_register);
