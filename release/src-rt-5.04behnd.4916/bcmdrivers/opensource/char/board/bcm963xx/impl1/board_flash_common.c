/*
    Copyright 2000-2010 Broadcom Corporation

   <:label-BRCM:2012:DUAL/GPL:standard
   
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

/*!\file flash_common.c
 * \brief This file contains NOR flash related functions  *
 */

/** Includes. */
#include <linux/kernel.h>
#include <linux/string.h>

#include "bcmtypes.h"
#include "flash_api.h"
#include "flash_common.h"

// #define DEBUG_FLASH

FLASH_PARTITION_INFO fAuxFsInfo;


void kerSysFlashPartInfoGet(PFLASH_PARTITION_INFO pflash_partition_info)
{
    memcpy((void*)pflash_partition_info,(const void*)(&fAuxFsInfo), sizeof(FLASH_PARTITION_INFO));
}


unsigned int flash_get_reserved_bytes_auxfs()
{
    int flash_type = flash_get_flash_type();

    if ((flash_type == FLASH_IFC_NAND) || (flash_type == FLASH_IFC_SPINAND))
    {
        return 0;
    }
    else
    {
        return fAuxFsInfo.total_len;
    }
}

unsigned int flash_get_reserved_bytes_at_end(const FLASH_ADDR_INFO *fInfo)
{
    unsigned int reserved=0;
    int i = fInfo->flash_meta_start_blk;
    int totalBlks = flash_get_numsectors();

    while (i < totalBlks)
    {
        reserved += flash_get_sector_size((unsigned short) i);
        i++;
    }

#if defined(DEBUG_FLASH)
    printk("reserved at bottom=%dKB\n", reserved/1024);
#endif

    return reserved;
}

