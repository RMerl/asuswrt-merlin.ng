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

#ifndef __PHY_BP_PARSING_H__
#define __PHY_BP_PARSING_H__

#include "bus_drv.h"
#include "phy_drv.h"
#include "mac_drv.h"
#include "boardparms.h"

typedef ETHERNET_CROSSBAR_INFO EMAC_PORT_INFO;

void bp_parse_phy_driver(const EMAC_PORT_INFO *port_info, phy_drv_t *phy_drv);
void bp_parse_phy_ext(const EMAC_PORT_INFO *port_info, phy_dev_t *phy_dev);
void bp_parse_mac_ext(const ETHERNET_MAC_INFO *emac_info, uint32_t port, mac_dev_t *mac_dev);

phy_type_t bp_parse_phy_type(const EMAC_PORT_INFO *port_info);
mac_type_t bp_parse_mac_type(const ETHERNET_MAC_INFO *emac_info, uint32_t port);

mac_dev_t *bp_parse_mac_dev(const ETHERNET_MAC_INFO *emac_info, uint32_t port);
phy_dev_t *bp_parse_phy_dev(const EMAC_PORT_INFO *port_info);

void *bp_parse_mac_priv(const ETHERNET_MAC_INFO *emac_info, uint32_t port);
void *bp_parse_phy_priv(const EMAC_PORT_INFO *port_info);

uint32_t bp_parse_phy_addr(const EMAC_PORT_INFO *port_info);

#endif
