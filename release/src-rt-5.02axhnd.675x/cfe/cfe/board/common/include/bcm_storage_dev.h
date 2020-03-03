/* 
    Copyright 2000-2017 Broadcom Corporation

    <:label-BRCM:2017:DUAL/GPL:standard
    
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
#ifndef __BCM_STOR_DEV_H
#define __BCM_STOR_DEV_H

#include "flash_api.h"

typedef enum _bcm_storage_dev_type {
        STOR_DEV_GEN=0,
        STOR_DEV_NAND,
        STOR_DEV_SNAND,
        STOR_DEV_NOR,
        STOR_DEV_EMMC,
} bcm_storage_dev_type_t;

typedef struct _bcm_storage_dev_info {
        bcm_storage_dev_type_t type;
        flash_device_info_t dev;
} bcm_storage_dev_info_t ;
#endif
