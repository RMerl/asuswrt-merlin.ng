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

/*
 * MDIO common functions
 */

#ifndef __MDIO_DRV_COMMON_H__
#define __MDIO_DRV_COMMON_H__

#include "os_dep.h"

#define MDIO_ERROR  -1
#define MDIO_OK      0

int32_t mdio_cmd_read_22(uint32_t *p, uint32_t addr, uint32_t reg, uint16_t *val);
int32_t mdio_cmd_write_22(uint32_t *p, uint32_t addr, uint32_t reg, uint16_t val);
int32_t mdio_cmd_read_45(uint32_t *p, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val);
int32_t mdio_cmd_write_45(uint32_t *p, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val);

#endif
