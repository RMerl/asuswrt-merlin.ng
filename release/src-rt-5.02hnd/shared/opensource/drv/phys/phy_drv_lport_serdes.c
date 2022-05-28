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

/*
 * Phy driver for lport SerDes: SGMII/HSGMII/XFI
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "lport_drv.h"

static int _phy_read_status(phy_dev_t *phy_dev)
{
    lport_port_status_s port_status;
    uint32_t port = (uint64_t)phy_dev->priv;
    int ret;

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

    if ((ret = lport_get_port_status(port, &port_status)) != 0)
        return ret;

    phy_dev->link = port_status.port_up ? 1 : 0;

    if (!phy_dev->link)
        return 0;

    if (port_status.rate == LPORT_RATE_10G)
        phy_dev->speed = PHY_SPEED_10000;
    else if (port_status.rate == LPORT_RATE_2500MB)
        phy_dev->speed = PHY_SPEED_2500;
    else if (port_status.rate == LPORT_RATE_1000MB)
        phy_dev->speed = PHY_SPEED_1000;
    else if (port_status.rate == LPORT_RATE_100MB)
        phy_dev->speed = PHY_SPEED_100;
    else if (port_status.rate == LPORT_RATE_10MB)
        phy_dev->speed = PHY_SPEED_10;

    phy_dev->duplex = (port_status.duplex == LPORT_FULL_DUPLEX) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;

    return 0;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    return 0;
}

phy_drv_t phy_drv_lport_serdes =
{
    .phy_type = PHY_TYPE_LPORT_SERDES,
    .name = "SERDES",
    .read_status = _phy_read_status,
    .init = _phy_init,
};
