/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
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

#include "rdd.h"
#include "rdd_map_auto.h"
#include "access_macros.h"
#include "rdd_common.h"

extern uintptr_t rdp_runner_core_addr[];

#if defined(RDP_SIM) || defined(_CFE_)
#if defined(PHYS_ADDR_64BIT)
#define RDD_VIRT_TO_PHYS(_addr) ((uint64_t)(_addr))
#else
#define RDD_VIRT_TO_PHYS(_addr) ((uint32_t)(_addr))
#endif
#define RDD_PHYS_TO_VIRT(_addr) ((void *)(_addr))
#else /* RDP_SIM || CFE */
#define RDD_VIRT_TO_PHYS(_addr) virt_to_phys((const volatile void *)(uintptr_t)_addr)
#define RDD_PHYS_TO_VIRT(_addr) phys_to_virt(_addr)
#endif

#ifdef RDP_SIM
#define RDD_RSV_VIRT_TO_PHYS(_addr) rsv_virt_to_phys((volatile void *)(uintptr_t)_addr)
#define RDD_RSV_PHYS_VIRT_TO(_addr) rsv_phys_to_virt(_addr)
#else
#define RDD_RSV_VIRT_TO_PHYS(_addr) RDD_VIRT_TO_PHYS((volatile void *)(uintptr_t)_addr)
#define RDD_RSV_PHYS_VIRT_TO(_addr) RDD_PHYS_TO_VIRT(_addr)
#endif


typedef enum
{
    rdd_size_8,
    rdd_size_16,
    rdd_size_32
} rdd_entry_size_t;

typedef struct rdd_module
{
    int (*init)(const struct rdd_module *);
    uint32_t context_offset;
    uint32_t context_size;
    uint32_t res_offset;
    uint32_t *cfg_ptr;
    void *params;
} rdd_module_t;

static inline void _rdd_module_init(rdd_module_t *module)
{
    if (module->init)
        module->init(module);
}

static inline void _rdd_i_write(uint32_t *addr_arr, uint32_t addr, uint32_t val, uint32_t i,
    rdd_entry_size_t size)
{
    uint32_t *entry;
    int mem_id;

    for (mem_id = 0; mem_id < GROUPED_EN_SEGMENTS_NUM; mem_id++)
    {
        if (addr_arr[mem_id] == INVALID_TABLE_ADDRESS)
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[mem_id] + addr_arr[mem_id]) + addr));
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

static inline uint32_t _rdd_i_read(uint32_t *addr_arr, uint32_t addr, uint32_t i, rdd_entry_size_t size)
{
    uint32_t *entry;
    int mem_id;

    for (mem_id = 0; mem_id < GROUPED_EN_SEGMENTS_NUM; mem_id++)
    {
        if (addr_arr[mem_id] == INVALID_TABLE_ADDRESS)
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[mem_id] + addr_arr[mem_id]) + addr));
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

static inline void _rdd_field_write(uint32_t *addr_arr, uint32_t addr, uint32_t val, uint32_t lsb, uint32_t width,
    rdd_entry_size_t size)
{
    uint32_t *entry;
    int mem_id;

    for (mem_id = 0; mem_id < GROUPED_EN_SEGMENTS_NUM; mem_id++)
    {
        if (addr_arr[mem_id] == INVALID_TABLE_ADDRESS)
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[mem_id] + addr_arr[mem_id]) + addr));
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

static inline uint32_t _rdd_field_read(uint32_t *addr_arr, uint32_t addr, uint32_t lsb, uint32_t width,
    rdd_entry_size_t size)
{
    uint32_t *entry;
    int mem_id;

    for (mem_id = 0; mem_id < GROUPED_EN_SEGMENTS_NUM; mem_id++)
    {
        if (addr_arr[mem_id] == INVALID_TABLE_ADDRESS)
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[mem_id] + addr_arr[mem_id]) + addr));
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
#endif
