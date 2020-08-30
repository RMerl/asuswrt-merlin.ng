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
#include "bcm6858_lport_srds_ag.h"
#include "serdes_access.h"
#include "lport_drv.h"

static uint32_t enabled_ports = 0xffffffff;

static int _phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    uint32_t port = (uint64_t)phy_dev->priv;

    *enable = enabled_ports & (1 << port);

    return 0;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
    uint32_t port = (uint64_t)phy_dev->priv;

    if (enable)
        enabled_ports |= (1 << port);
    else
        enabled_ports &= ~(1 << port);

    return 0;
}

static int _get_port_status(uint32_t port, uint8_t *sig, uint8_t *lnk)
{
    int rc = LPORT_ERR_OK;
    uint8_t msk = 0x2;
    lport_srds_dual_serdes_0_status srds_0_status = {};
    lport_srds_dual_serdes_1_status srds_1_status = {};

    switch (port)
    {
    case 0:
    case 7:
        msk = 0x1;
    case 1:
    case 4:
        rc = ag_drv_lport_srds_dual_serdes_0_status_get(&srds_0_status);
        *sig = !(srds_0_status.ext_sig_det & msk) ? 1 : 0; 
        *lnk = (srds_0_status.link_status & msk) ? 1 : 0; 
        break;
    case 2:
    case 5:
        msk = 0x1;
    case 3:
    case 6:
        rc = ag_drv_lport_srds_dual_serdes_1_status_get(&srds_1_status);
        *sig = !(srds_1_status.ext_sig_det & msk) ? 1 : 0; 
        *lnk = (srds_1_status.link_status & msk) ? 1 : 0; 
        break;
    }

    return rc;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    uint32_t port = (uint64_t)phy_dev->priv;
    uint8_t sig = 0, lnk = 0;
    int ret;

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

    if (!(enabled_ports & (1 << port)))
        return 0;

    if ((ret = _get_port_status(port, &sig, &lnk)))
        return ret;

    if (!sig)
        return 0;

    phy_dev->link = 1;
    phy_dev->duplex = PHY_DUPLEX_FULL;

    if (phy_dev->mii_type == PHY_MII_TYPE_XFI)
        phy_dev->speed = PHY_SPEED_10000;
    if (phy_dev->mii_type == PHY_MII_TYPE_HSGMII)
        phy_dev->speed = PHY_SPEED_2500;
    if (phy_dev->mii_type == PHY_MII_TYPE_SGMII)
        phy_dev->speed = PHY_SPEED_1000;

    return 0;
}

static int _phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *pcaps)
{
    uint32_t caps = 0;
    uint32_t port = (uint64_t)phy_dev->priv;
    LPORT_PORT_RATE port_rate;

    if (caps_type != CAPS_TYPE_ADVERTISE)
        return 0;

    port_rate = lport_speed_get(port);
    switch (port_rate)
    {
    case LPORT_RATE_10MB: /* 10M */
        caps |= PHY_CAP_10_HALF | PHY_CAP_10_FULL;
        break;
    case LPORT_RATE_100MB: /* 100Mbps */
        caps |= PHY_CAP_100_HALF | PHY_CAP_100_FULL;
        break;
    case LPORT_RATE_1000MB: /* 1000Mbps */
        caps |= PHY_CAP_1000_HALF | PHY_CAP_1000_FULL;
        break;
    case LPORT_RATE_2500MB: /* 2500Mbps */
        caps |= PHY_CAP_2500;
        break;
    case LPORT_RATE_10G: /* 5000/10000Mbps */
        caps |= PHY_CAP_10000;
        break;
    default:
        break;
    }

    if (phy_dev->pause_rx && phy_dev->pause_tx)
        caps |= PHY_CAP_PAUSE;
    else if (phy_dev->pause_rx)
        caps |= PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM;
    else if (phy_dev->pause_tx)
        caps |= PHY_CAP_PAUSE_ASYM;

    *pcaps = caps;
    return 0;
}

static int _phy_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    phy_dev->link = 0;
    phy_dev->pause_rx = 0; 
    phy_dev->pause_tx = 0; 

    if (caps & PHY_CAP_PAUSE)
    {
        phy_dev->pause_rx = 1; 
        phy_dev->pause_tx = (caps & PHY_CAP_PAUSE_ASYM) ? 0 : 1;
    }
    else if (caps & PHY_CAP_PAUSE_ASYM)
    {
        phy_dev->pause_tx = 1; 
    }

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
    .power_get = _phy_power_get,
    .power_set = _phy_power_set,
    .read_status = _phy_read_status,
    .caps_get = _phy_caps_get,
    .caps_set = _phy_caps_set,
    .init = _phy_init,
};
