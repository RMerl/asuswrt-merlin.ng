/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2021 Broadcom Ltd.
 */
/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#ifndef __BCM_STRAP_DRV_H
#define __BCM_STRAP_DRV_H

#include <dm.h>
#include <spl.h>

/* broadcom specific boot device definition */
#define BOOT_DEVICE_ETH	0xe0

typedef enum
{
    ECC_NONE,
    ECC_1_BIT,
    ECC_4_BIT,
    ECC_8_BIT,
    ECC_12_BIT,
    ECC_24_BIT,
    ECC_40_BIT,
    ECC_60_BIT
}nand_ecc_t;

int bcm_strap_parse_and_test(ofnode np, const char* consumer_name);
uint32_t bcm_strap_get_val(void);
int bcm_get_boot_device(void);
nand_ecc_t bcm_get_nand_ecc(void);
int bcm_strap_drv_reg(void);
uint32_t bcm_strap_get_field_val(const char* field_name);

#endif
