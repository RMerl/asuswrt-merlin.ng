/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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


