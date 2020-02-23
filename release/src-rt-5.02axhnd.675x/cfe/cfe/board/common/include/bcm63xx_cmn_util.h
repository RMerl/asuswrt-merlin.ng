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
/*Simple wrap over read/write primitives from flash_api and emmc
  This typically achieved via cfe driver framework which is normally linked to cfe_ram
This a greatly simplified version
*/
#ifndef __BCM63XX_STORAGE_H_
#define __BCM63XX_STORAGE_H_

#include "bcm_storage_dev.h"

typedef struct _cfe_storage_dev_info {
        unsigned int size;
        unsigned int block_size;
} cfe_storage_dev_info_t;

typedef struct _cfe_storage_dev {
        int (*init)(void);
        int (*read_raw)(unsigned int offs, unsigned int sz, void* buf);
        int (*erase)(unsigned int ofs, unsigned int sz);
        int (*get_info)(cfe_storage_dev_info_t** info);
} cfe_storage_dev_t;

int cfe_storage_dev_init(void);

void cfe_storage_dev_reset(void);

cfe_storage_dev_t* cfe_storage_dev_get(void);

#endif
