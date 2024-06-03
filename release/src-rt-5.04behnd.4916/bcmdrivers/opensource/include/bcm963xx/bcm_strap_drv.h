/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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

#ifndef __BCM_STRAP_DRV_H
#define __BCM_STRAP_DRV_H

#include <linux/of_device.h>

typedef enum 
{
    NAND_2K,
    NAND_4K,
    NAND_8K,
    NAND_512B,
    SPI_NOR,
    EMMC,
    SPI_NAND,
    ETHERNET
}boot_dev_t;

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

int bcm_strap_parse_and_test(struct device_node *np, const char* consumer_name);
uint32_t bcm_strap_get_val(void);
boot_dev_t bcm_get_boot_device(void);
nand_ecc_t bcm_get_nand_ecc(void);
uint32_t bcm_strap_get_field_val(const char* field_name);

#endif
