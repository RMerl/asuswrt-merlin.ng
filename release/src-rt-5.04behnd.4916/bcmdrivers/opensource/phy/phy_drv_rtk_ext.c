/*
   <:copyright-BRCM:2024:DUAL/GPL:standard
   
      Copyright (c) 2024 Broadcom 
      All Rights Reserved
   
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
 *  Created on: Jan/2026
 *  
 *  PHY driver for external RTK (Realtek) switch ports
 *  Used for VLAN-tagged external switch ports (e.g., RT-BE58U)
 */

#include "phy_drv.h"
#include "phy_drv_mii.h"
#include <linux/module.h>

/* RTK switch API declarations */
typedef unsigned int rtk_api_ret_t;
typedef unsigned int rtk_uint32;
typedef rtk_uint32 rtk_port_t;

typedef enum rtk_port_linkStatus_e
{
    PORT_LINKDOWN = 0,
    PORT_LINKUP,
    PORT_LINKSTATUS_END
} rtk_port_linkStatus_t;

typedef enum rtk_port_speed_e
{
    PORT_SPEED_10M = 0,
    PORT_SPEED_100M,
    PORT_SPEED_1000M,
    PORT_SPEED_500M,
    PORT_SPEED_2500M,
    PORT_SPEED_END
} rtk_port_speed_t;

typedef enum rtk_port_duplex_e
{
    PORT_HALF_DUPLEX = 0,
    PORT_FULL_DUPLEX,
    PORT_DUPLEX_END
} rtk_port_duplex_t;

#define RT_ERR_OK 0

/* External function from RTK driver - resolved at runtime */
typedef rtk_api_ret_t (*rtk_port_phyStatus_get_fn)(rtk_port_t port,
                                                  rtk_port_linkStatus_t *pLinkStatus,
                                                  rtk_port_speed_t *pSpeed,
                                                  rtk_port_duplex_t *pDuplex);

#define RTK_PHY_STATUS_GET_SYM "rtk_port_phyStatus_get"

/*
 * Map phy_dev->addr to RTK switch port
 * For RT-BE58U: eth1-eth4 = VLAN 1001-1004 = RTK ports p4-p1
 * DTS assigns phy addr 21-24 for phy_m2m0-3
 * Mapping: phy addr 21->RTK p4, 22->RTK p3, 23->RTK p2, 24->RTK p1
 */
static int phy_addr_to_rtk_port(int phy_addr)
{
    switch (phy_addr) {
#if defined(RTBE58U)
        case 21: return 4;  /* eth1/VLAN1001 -> RTK port 4 */
        case 22: return 3;  /* eth2/VLAN1002 -> RTK port 3 */
        case 23: return 2;  /* eth3/VLAN1003 -> RTK port 2 */
        case 24: return 1;  /* eth4/VLAN1004 -> RTK port 1 */
#endif
        default: return -1;
    }
}

static int rtk_ext_phy_read_status(phy_dev_t *phy_dev)
{
    rtk_port_linkStatus_t link_status;
    rtk_port_speed_t rtk_speed;
    rtk_port_duplex_t rtk_duplex;
    rtk_api_ret_t ret;
    rtk_port_phyStatus_get_fn fn = NULL;
    int rtk_port = phy_addr_to_rtk_port(phy_dev->addr);
    int old_link = phy_dev->link;

    if (rtk_port < 0)
        return 0;

    /*
     * Resolve symbol for this call only.
     * __symbol_get() increments the owner module refcount, so we MUST
     * __symbol_put() it when done to avoid refcount leaks.
     */
    fn = (rtk_port_phyStatus_get_fn)__symbol_get(RTK_PHY_STATUS_GET_SYM);
    if (!fn) {
        printk("rtk_ext: Failed to resolve %s symbol\n", RTK_PHY_STATUS_GET_SYM);
        return 0;
    }

    /* Query RTK switch for port link status */
    ret = fn((rtk_port_t)rtk_port, &link_status, &rtk_speed, &rtk_duplex);

    /* Release module ref we acquired via __symbol_get() */
    __symbol_put(RTK_PHY_STATUS_GET_SYM);

    if (ret != RT_ERR_OK) {
        printk("rtk_ext_phy_read_status: rtk_port_phyStatus_get failed, ret=%d\n", ret);
        return 0;
    }

    /* Update PHY device status */
    phy_dev->link = (link_status == PORT_LINKUP) ? 1 : 0;

    /* Convert RTK speed to PHY speed */
    switch (rtk_speed) {
        case PORT_SPEED_10M:
            phy_dev->speed = PHY_SPEED_10;
            break;
        case PORT_SPEED_100M:
            phy_dev->speed = PHY_SPEED_100;
            break;
        case PORT_SPEED_1000M:
            phy_dev->speed = PHY_SPEED_1000;
            break;
        case PORT_SPEED_2500M:
            phy_dev->speed = PHY_SPEED_2500;
            break;
        default:
            phy_dev->speed = PHY_SPEED_UNKNOWN;
            break;
    }

    /* Convert RTK duplex to PHY duplex */
    phy_dev->duplex = (rtk_duplex == PORT_FULL_DUPLEX) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;

    /* Log link state change with detailed port information */
    if (old_link != phy_dev->link) {
        printk("RTK ext port p%d (phy addr %d, eth%d): link %s, speed %d, duplex %s\n",
               rtk_port, phy_dev->addr,
               phy_dev->addr - 20,  /* eth1-eth4 for addr 21-24 */
               phy_dev->link ? "UP" : "DOWN",
               phy_dev->link ? phy_speed_2_mbps(phy_dev->speed) : 0,
               phy_dev->link ? (phy_dev->duplex == PHY_DUPLEX_FULL ? "full" : "half") : "n/a");
    }

    return 0;
}

static int rtk_ext_phy_init(phy_dev_t *phy_dev)
{
    int rtk_port = phy_addr_to_rtk_port(phy_dev->addr);

    printk("rtk_ext_phy_init: phy addr %d -> RTK port p%d (eth%d)\n",
           phy_dev->addr, rtk_port, phy_dev->addr - 20);

    /* Initial link status query */
    rtk_ext_phy_read_status(phy_dev);

    printk("RTK external switch PHY initialized: phy addr %d -> RTK port p%d (eth%d)\n",
           phy_dev->addr, rtk_port, phy_dev->addr - 20);

    return 0;
}

static int rtk_ext_phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    /* External switch ports are always powered */
    *enable = 1;
    return 0;
}

static int rtk_ext_phy_power_set(phy_dev_t *phy_dev, int enable)
{
    /* Cannot control power of external switch ports */
    return 0;
}

phy_drv_t phy_drv_rtk_ext =
{
    .phy_type = PHY_TYPE_EXT1,
    .name = "RTK_EXT",
    .read_status = rtk_ext_phy_read_status,
    .init = rtk_ext_phy_init,
    .power_get = rtk_ext_phy_power_get,
    .power_set = rtk_ext_phy_power_set,
};
