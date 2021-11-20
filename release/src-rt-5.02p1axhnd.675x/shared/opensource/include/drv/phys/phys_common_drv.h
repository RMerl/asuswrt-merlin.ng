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

#ifndef __PHYS_COMMON_DRV_H
#define __PHYS_COMMON_DRV_H

#define PHY_LINK_ON             1
#define PHY_LINK_OFF            0

typedef enum
{
    PHY_RATE_10_FULL,
    PHY_RATE_10_HALF,
    PHY_RATE_100_FULL,
    PHY_RATE_100_HALF,
    PHY_RATE_1000_FULL,
    PHY_RATE_1000_HALF,
    PHY_RATE_2500_FULL,
    PHY_RATE_LINK_DOWN,
    PHY_RATE_ERR,
} phy_rate_t;

void port_phy_init(uint32_t port);
phy_rate_t port_get_link_speed(uint32_t port);
uint32_t port_get_link_status(uint32_t port);
uint16_t phy_read_register(uint32_t addr, uint16_t reg);
int32_t phy_write_register (uint32_t addr,uint16_t reg, uint16_t val);

#endif
