/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
/*
 * bcm_ru_map.c
 *
 *  Created on: 11 Nov 2015
 *      Author: yonatani
 */
#include <bcm_map_part.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/printk.h>
#include "ru_types.h"

static void remap_ru_block_single_virtbase(ru_block_rec *ru_block, uint64_t virt_base)
{
    uint32_t addr_itr;

    for (addr_itr = 0; addr_itr < ru_block->addr_count; addr_itr++)
    {
        ru_block->addr[addr_itr] += virt_base;
#ifdef DEBUG_RU_IOREMAP
        printk(KERN_INFO "       Remapped Block %s idx = %d Phys Address = 0x%lx to Virtual = 0x%lx\n",
                ru_block->name, addr_itr, (long unsigned int) ru_block->addr[addr_itr], (long unsigned int) ru_block->addr[addr_itr]);
#endif
    }
}

uintptr_t bcm_io_block_virt_base_get(uint32_t block_index, uint32_t phys_base)
{
    return bcm_io_block_address[bcm_io_blocks[block_index].index] - phys_base;
}

/* This function is a utility to fix virtual address of any RU/HAL based drivers when running in Kernel
 * after calling this function all RU addresses will turn virtual and no translation is needed during read/write
 *
 * block_index is defined in {BRCM_CHIP}_map_part.h in BCM_IO_MAP_IDX enum
 * ru_blocks[] is the main ru object of the block
 * */
void remap_ru_block_addrs(uint32_t block_index, ru_block_rec *ru_blocks[])
{
    uint32_t blk;
    /* new_address = old_address + block_virtual_base - block_physical_base */
    uintptr_t virt_base = bcm_io_block_virt_base_get(block_index, bcm_io_blocks[block_index].address);

    for (blk = 0; ru_blocks[blk]; blk++)
    {
        remap_ru_block_single_virtbase(ru_blocks[blk], virt_base);
    }
}

void remap_ru_block_single(uint32_t block_index, ru_block_rec *ru_block, uint32_t phys_base)
{
    uintptr_t virt_base = bcm_io_block_virt_base_get(block_index, phys_base);

    remap_ru_block_single_virtbase(ru_block, virt_base);
}

uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx)
{
    return ru_block->addr[addr_idx] - (bcm_io_block_address[bcm_io_blocks[XRDP_IDX].index] - XRDP_PHYS_BASE);
}

EXPORT_SYMBOL(bcm_io_block_virt_base_get);
EXPORT_SYMBOL(remap_ru_block_addrs);
EXPORT_SYMBOL(remap_ru_block_single);
EXPORT_SYMBOL(xrdp_virt2phys);
