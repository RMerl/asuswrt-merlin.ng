/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

#ifndef __BCM_PHYSICAL_MAP_PART_H
#define __BCM_PHYSICAL_MAP_PART_H

#include <bcm_map_part.h>

#if defined(REG_BASE)
static inline void *bcm_dev_phy2vir(unsigned long phyAddr)
{
    /* Note: The range checking here is just a rough one.
        The caller should guarrantee the physical address of register
        passed in is a valid register based on chip register spec. */
    if (phyAddr < REG_BASE || phyAddr > (REG_BASE + 0x10000000))
        return 0; 
#if defined(BCM_IO_ADDR)
    return (void *)(uintptr_t)BCM_IO_ADDR(phyAddr);
#else
    return (void *)(uintptr_t)(REG_BASE + (phyAddr & 0xffffff));
#endif
}
#else
static inline void *bcm_dev_phy2vir(unsigned long phyAddr)
{
    int idx;
    BCM_IO_BLOCKS *blk = bcm_io_blocks;

    for (idx = 0; idx < LAST_IDX; idx++, blk++)
        if (phyAddr >= blk->address && phyAddr < blk->address + blk->size)
            break;

    if (idx >= LAST_IDX)
        return 0;
    return (void *)(bcm_io_block_address[bcm_io_blocks[idx].index] + phyAddr - blk->address);
}
#endif
#endif /* __BCM_PHYSICAL_MAP_PART_H */


