/*
   Copyright (c) 2016 Broadcom Corporation
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

#ifndef __MDIO_DRV_IMPL3_H
#define __MDIO_DRV_IMPL3_H

#include "os_dep.h"

#define MDIO_ERROR     -1
#define MDIO_OK        0

extern int32_t mdio_read_c22_register(uint8_t core_num, uint32_t phy_addr, uint32_t reg_addr, uint16_t *data_read);
extern int32_t mdio_write_c22_register(uint8_t core_num, uint32_t phy_addr, uint32_t reg_addr, uint16_t data_write);

extern int32_t mdio_read_c45_register(uint8_t core_num, uint32_t port_addr, uint32_t dev_addr, uint16_t dev_offset, uint16_t *data_read);
extern int32_t mdio_write_c45_register(uint8_t core_num, uint32_t port_addr, uint32_t dev_addr,uint16_t dev_offset, uint16_t data_write);


#endif
