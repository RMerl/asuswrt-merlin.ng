/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#ifndef _RDD_PLATFORM_H
#define _RDD_PLATFORM_H

#include "rdp_map.h"
#include "access_macros.h"

typedef enum
{
    rdd_runner_0, /* 3rd GEN RDP - MAIN A */
    rdd_runner_1, /* 3rd GEN RDP - MAIN B */
    rdd_runner_2, /* 3rd GEN RDP - PICO A */
    rdd_runner_3, /* 3rd GEN RDP - PICO B */
    rdd_runner_last
} rdd_runner_t;

typedef enum
{
    rdd_mem_0, /* 3rd GEN RDP - PRIVATE A */
    rdd_mem_1, /* 3rd GEN RDP - PRIVATE B */
    rdd_mem_last
} rdd_mem_t;

#if defined(WL4908) && !defined(BDMF_SYSTEM_SIM)
#define RDD_VIRT_TO_PHYS(_addr) virt_to_phys((const volatile void *)_addr)
#else
#define RDD_VIRT_TO_PHYS(v) ((uint32_t)(v))
#endif

typedef struct
{
    int runner_vector; /* runners pertaining to group */
    int mem_vector; /* memories pertaining to group */
} rdd_runner_group_t;

typedef enum
{
    rdd_size_8,
    rdd_size_16,
    rdd_size_32
} rdd_entry_size_t;

typedef struct rdd_module
{
    rdd_runner_group_t *group;
    int (*init)(const struct rdd_module *);
    uint32_t context_offset;
    uint32_t context_size;
    uint32_t res_offset;
    uint32_t cfg_ptr;
    void *params;
} rdd_module_t;

static inline void _rdd_module_init(rdd_module_t *module)
{
    if (module->init)
        module->init(module);
}

/* TODO: REMOVE */
#define MWRITE_GROUP_BLOCK_32(group, addr, block, size) _rdd_block_write(group, (addr), block, rdd_size_32, size)
#define MWRITE_GROUP_BLOCK_16(group, addr, block, size) _rdd_block_write(group, (addr), block, rdd_size_16, size)

static inline uint32_t _rdd_mem_to_base_addr(rdd_mem_t mem)
{
    switch (mem)
    {
    case rdd_mem_0:
        return RUNNER_PRIVATE_0_OFFSET;
    case rdd_mem_1:
        return RUNNER_PRIVATE_1_OFFSET;
    default:
        return 0;
    }
}

static inline void _rdd_i_write(rdd_runner_group_t *group, uint32_t addr, uint32_t val, uint32_t i,
    rdd_entry_size_t size)
{
    uint32_t *entry;
    int mem;

    for (mem = rdd_mem_0; mem < rdd_mem_last; mem++)
    {
        if (!(group->mem_vector & 1 << mem))
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS(_rdd_mem_to_base_addr(mem)) + addr);
        switch (size)
        {
        case rdd_size_32:
            MWRITE_I_32(entry, i, val);
            break;
        case rdd_size_16:
            MWRITE_I_16(entry, i, val);
            break;
        default:
            MWRITE_I_8(entry, i, val);
            break;
        }
    }
}

static inline uint32_t _rdd_i_read(rdd_runner_group_t *group, uint32_t addr, uint32_t i, rdd_entry_size_t size)
{
    uint32_t *entry;
    int mem;

    for (mem = rdd_mem_0; mem < rdd_mem_last; mem++)
    {
        if (!(group->mem_vector & 1 << mem))
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS(_rdd_mem_to_base_addr(mem)) + addr);
        switch (size)
        {
        case rdd_size_32:
            return MGET_I_32(entry, i);
        case rdd_size_16:
            return MGET_I_16(entry, i);
        default:
            return MGET_I_8(entry, i);
        }
    }
    return 0;
}

static inline void _rdd_field_write(rdd_runner_group_t *group, uint32_t addr, uint32_t val, uint32_t lsb, uint32_t width,
    rdd_entry_size_t size)
{
    uint32_t *entry;
    int mem;

    for (mem = rdd_mem_0; mem < rdd_mem_last; mem++)
    {
        if (!(group->mem_vector & 1 << mem))
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS(_rdd_mem_to_base_addr(mem)) + addr);
        switch (size)
        {
        case rdd_size_32:
            FIELD_MWRITE_32(entry, lsb, width, val);
            break;
        case rdd_size_16:
            FIELD_MWRITE_16(entry, lsb, width, val);
            break;
        default:
            FIELD_MWRITE_8(entry, lsb, width, val);
            break;
        }
    }
}

static inline uint32_t _rdd_field_read(rdd_runner_group_t *group, uint32_t addr, uint32_t lsb, uint32_t width,
    rdd_entry_size_t size)
{
    uint32_t *entry;
    int mem;

    for (mem = rdd_mem_0; mem < rdd_mem_last; mem++)
    {
        if (!(group->mem_vector & 1 << mem))
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS(_rdd_mem_to_base_addr(mem)) + addr);
        switch (size)
        {
        case rdd_size_32:
            return FIELD_MGET_32(entry, lsb, width);
        case rdd_size_16:
            return FIELD_MGET_16(entry, lsb, width);
        default:
            return FIELD_MGET_8(entry, lsb, width);
        }
    }
    return 0;
}

static inline void _rdd_block_write(rdd_runner_group_t *group, const uint32_t addr, uint32_t *block,
    uint32_t block_size, uint32_t size)
{
    int mem;

    for (mem = rdd_mem_0; mem < rdd_mem_last; mem++)
    {
        if (!(group->mem_vector & 1 << mem))
            continue;
        switch (block_size)
        {
        case rdd_size_32:
            MWRITE_BLK_32((uint32_t *)(DEVICE_ADDRESS(_rdd_mem_to_base_addr(mem)) + addr), block, size);
            break;
        case rdd_size_16:
            MWRITE_BLK_16((uint16_t *)(DEVICE_ADDRESS(_rdd_mem_to_base_addr(mem)) + addr), block, size);
            break;
        }
    }
}
#endif
