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

#include "rdd_map_auto.h"
#include "access_macros.h"
#include "rdd_common.h"
#ifdef __KERNEL__
#include "bcm_rsvmem.h"
#endif

#define ECN_IN_TOS_SHIFT 2
extern uintptr_t rdp_runner_core_addr[];

#if !defined(RDP_SIM)
#define RDD_VIRT_TO_PHYS(_addr) virt_to_phys((const volatile void *)(uintptr_t)_addr)
#define RDD_PHYS_TO_VIRT(_addr) phys_to_virt(_addr)
#endif

#ifdef RDP_SIM
#if defined(PHYS_ADDR_64BIT)
#define RDD_IO_VIRT_TO_PHYS(_addr) ((uint64_t)(_addr))
#else
#define RDD_IO_VIRT_TO_PHYS(_addr) ((uint32_t)(_addr))
#endif
#define RDD_IO_PHYS_TO_VIRT(_addr) ((void *)(_addr))

#define RDD_VIRT_TO_PHYS(_addr) rsv_virt_to_phys((volatile void *)(uintptr_t)_addr)
#define RDD_PHYS_TO_VIRT(_addr) rsv_phys_to_virt(_addr)
#define RDD_RSV_VIRT_TO_PHYS(_vbase, _pbase, _addr) RDD_VIRT_TO_PHYS(_addr)
#define RDD_RSV_PHYS_TO_VIRT(_vbase, _pbase, _addr) RDD_PHYS_TO_VIRT(_addr)

#define PHYS_TO_UNCACHED(x)     RDD_PHYS_TO_VIRT(x)
#define CACHED_MALLOC_ATOMIC(size) bdmf_alloc_rsv(size, NULL)
#define NONCACHE_TO_CACHE(ptr)    (ptr)
#define INV_RANGE(ptr, size)       do {} while (0)
#define CACHED_FREE(ptr)           do {} while (0)
#define GFP_ATOMIC                 (0)
#define kmem_cache_free(mem, ptr)  do {} while (0)
#else
#ifdef __KERNEL__
#define RDD_RSV_VIRT_TO_PHYS(_vbase, _pbase, _addr) BcmMemReserveVirtToPhys(_vbase, _pbase, _addr)
#define RDD_RSV_PHYS_TO_VIRT(_vbase, _pbase, _addr) BcmMemReservePhysToVirt(_vbase, _pbase, _addr)
#else
#define RDD_RSV_VIRT_TO_PHYS(_vbase, _pbase, _addr) RDD_VIRT_TO_PHYS((volatile void *)(uintptr_t)_addr)
#define RDD_RSV_PHYS_TO_VIRT(_vbase, _pbase, _addr) RDD_PHYS_TO_VIRT(_addr)
#endif
#endif

#define SCHEDULING
#if  (defined(BCM6888)) 
#define MAX_TM_CORES_PER_DIRECTION 3
#else
#define MAX_TM_CORES_PER_DIRECTION 1
#endif

typedef enum
{
    rdd_size_8,
    rdd_size_16,
    rdd_size_32,
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

#if defined(RDP_SIM)

static inline uint8_t read8(uint8_t *addr);
static inline uint16_t read16(uint16_t *addr);
static inline uint32_t read32(uint32_t *addr);
static inline void write8(uint8_t *addr, uint8_t value);
static inline void write16(uint16_t *addr, uint16_t value);
static inline void write32(uint32_t *addr, uint32_t value);

static inline void _rdd_socket_write(uint32_t sim_addr, uint32_t val, rdd_entry_size_t size)
{
    sw2hw_msg write_msg = {};

    write_msg.type = SW2HW_MSG_MEM_WRITE;
    write_msg.mem_write.address = sim_addr;
    switch (size)
    {
    case rdd_size_32:
      	write_msg.mem_write.size = 4;
       	MWRITE_32(write_msg.mem_write.data, val);
        break;
    case rdd_size_16:
        write_msg.mem_write.size = 2;
        MWRITE_16(write_msg.mem_write.data, val);
      	break;
    default:
        write_msg.mem_write.size = 1;
        MWRITE_8(write_msg.mem_write.data, val);
       	break;
    }
    cpu_runner_sim_send_data(cpu_runner_sim_get_msg_length(&write_msg), (char *)&write_msg);
}

static inline uint32_t _rdd_socket_read(uint32_t sim_addr, rdd_entry_size_t size)
{
    sw2hw_msg read_msg = {};
    hw2sw_msg res_msg = {};

    read_msg.type = SW2HW_MSG_MEM_READ;
    read_msg.mem_read.address = sim_addr;
    switch (size)
    {
    case rdd_size_32:
        read_msg.mem_read.size = 4;
        break;
    case rdd_size_16:
        read_msg.mem_read.size = 2;
   	    break;
    default:
        read_msg.mem_read.size = 1;
     	break;
    }
    cpu_runner_sim_send_data(cpu_runner_sim_get_msg_length(&read_msg), (char *)&read_msg);

    /* get response */
    if (cpu_runner_sim_receive_data((char *)&res_msg) > 0)
    {
        switch (size)
        {
        case rdd_size_32:
            return MGET_32(res_msg.read_response.data);
        case rdd_size_16:
            return MGET_16(res_msg.read_response.data);
        default:
            return MGET_8(res_msg.read_response.data);
        }
    }
    return 0;
}

static inline uint32_t sim_addr_calc(int mem_id, uint32_t *addr_arr, uint32_t addr, uint32_t i, rdd_entry_size_t size)
{
    uint32_t sim_addr;

    sim_addr = (rdp_runner_core_addr[mem_id] + addr_arr[mem_id]) + addr;
    switch (size)
    {
    case rdd_size_32:
       	sim_addr += (i * sizeof(uint32_t));
        break;
    case rdd_size_16:
        sim_addr += (i * sizeof(uint16_t));
      	break;
    default:
        sim_addr += (i * sizeof(uint8_t));
       	break;
    }

    return sim_addr;
}

static inline void _rdd_socket_i_write(int mem_id, uint32_t *addr_arr, uint32_t addr,
    uint32_t val, uint32_t i, rdd_entry_size_t size)
{
    uint32_t sim_addr;

    sim_addr = sim_addr_calc(mem_id, addr_arr, addr, i, size);
    return _rdd_socket_write(sim_addr, val, size);
}

static inline uint32_t _rdd_socket_i_read(int mem_id, uint32_t *addr_arr, uint32_t addr, uint32_t i,
    rdd_entry_size_t size)
{
    uint32_t sim_addr;

    sim_addr = sim_addr_calc(mem_id, addr_arr, addr, i, size);
    return _rdd_socket_read(sim_addr, size);
}
#endif

static inline void _rdd_i_write_single_core(uint32_t *addr_arr, uint32_t addr, uint32_t val, uint32_t i,
    rdd_entry_size_t size, int core_id)
{
    uint32_t *entry;

    BUG_ON(addr_arr[core_id] == INVALID_TABLE_ADDRESS);

#if defined(RDP_SIM)
    if (g_runner_sim_connected)
    {
        _rdd_socket_i_write(core_id, addr_arr, addr, val, i, size);
    }
#endif

    entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[core_id] + addr_arr[core_id]) + addr));
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

static inline void _rdd_i_write(uint32_t *addr_arr, uint32_t addr, uint32_t val, uint32_t i,
    rdd_entry_size_t size)
{
    int core_id;

    for (core_id = 0; core_id < NUM_OF_RUNNER_CORES; core_id++)
    {
        if (addr_arr[core_id] == INVALID_TABLE_ADDRESS)
            continue;

        _rdd_i_write_single_core(addr_arr, addr, val, i, size, core_id);
    }
}

static inline uint32_t _rdd_i_read_single_core(uint32_t *addr_arr, uint32_t addr, uint32_t i, rdd_entry_size_t size, int core_id)
{
    uint32_t ret = 0;
    uint32_t *entry;

    BUG_ON(addr_arr[core_id] == INVALID_TABLE_ADDRESS);

#if defined(RDP_SIM)
    if (g_runner_sim_connected)
    {
        ret = _rdd_socket_i_read(core_id, addr_arr, addr, i, size);
    }
    else
#endif
    {
        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[core_id] + addr_arr[core_id]) + addr));
        switch (size)
        {
        case rdd_size_32:
            ret = MGET_I_32(entry, i);
            break;
        case rdd_size_16:
            ret = MGET_I_16(entry, i);
            break;
        default:
            ret = MGET_I_8(entry, i);
            break;
        }
    }

    return ret;
}

static inline uint32_t _rdd_i_read(uint32_t *addr_arr, uint32_t addr, uint32_t i, rdd_entry_size_t size)
{
    uint32_t ret = 0;
    int core_id;

    for (core_id = 0; core_id < NUM_OF_RUNNER_CORES; core_id++)
    {
        if (addr_arr[core_id] == INVALID_TABLE_ADDRESS)
            continue;

        ret = _rdd_i_read_single_core(addr_arr, addr, i, size, core_id);
    }
    return ret;
}

static inline void _rdd_field_write_single_core(uint32_t *addr_arr, uint32_t addr, uint32_t val, uint32_t lsb, uint32_t width,
    rdd_entry_size_t size, int core_id)
{
    uint32_t *entry;

    BUG_ON(addr_arr[core_id] == INVALID_TABLE_ADDRESS);

#if defined(RDP_SIM)
    if (g_runner_sim_connected)
    {
        uint32_t current_value = _rdd_socket_i_read(core_id, addr_arr, addr, 0, size);
        RDP_FIELD_SET(current_value, lsb, width, val);
        _rdd_socket_i_write(core_id, addr_arr, addr, current_value, 0, size);
    }
#endif
    entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[core_id] + addr_arr[core_id]) + addr));
    switch (size)
    {
    case rdd_size_32:
        RDP_FIELD_MWRITE_32(entry, lsb, width, val);
        break;
    case rdd_size_16:
        RDP_FIELD_MWRITE_16(entry, lsb, width, val);
        break;
    default:
        RDP_FIELD_MWRITE_8(entry, lsb, width, val);
        break;
    }
}

static inline void _rdd_field_write(uint32_t *addr_arr, uint32_t addr, uint32_t val, uint32_t lsb, uint32_t width,
    rdd_entry_size_t size)
{
    int core_id;

    for (core_id = 0; core_id < NUM_OF_RUNNER_CORES; core_id++)
    {
        if (addr_arr[core_id] == INVALID_TABLE_ADDRESS)
            continue;

        _rdd_field_write_single_core(addr_arr, addr, val, lsb, width, size, core_id);
    }
}


static inline uint32_t _rdd_field_read_single_core(uint32_t *addr_arr, uint32_t addr, uint32_t lsb, uint32_t width,
    rdd_entry_size_t size, int core_id)
{
    uint32_t ret = 0;
    uint32_t *entry;

    BUG_ON(addr_arr[core_id] == INVALID_TABLE_ADDRESS);

#if defined(RDP_SIM)
    if (g_runner_sim_connected)
    {
        uint32_t current_value = _rdd_socket_i_read(core_id, addr_arr, addr, 0, size);
        ret = RDP_FIELD_GET(current_value, lsb, width);
    }
    else
#endif
    {
        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[core_id] + addr_arr[core_id]) + addr));
        switch (size)
        {
            case rdd_size_32:
                ret = RDP_FIELD_MGET_32(entry, lsb, width);
                break;
            case rdd_size_16:
                ret = RDP_FIELD_MGET_16(entry, lsb, width);
                break;
            default:
                ret = RDP_FIELD_MGET_8(entry, lsb, width);
                break;
        }
    }

    return ret;
}

static inline uint32_t _rdd_field_read(uint32_t *addr_arr, uint32_t addr, uint32_t lsb, uint32_t width,
    rdd_entry_size_t size)
{
    int core_id;
    uint32_t ret = 0;


    for (core_id = 0; core_id < NUM_OF_RUNNER_CORES; core_id++)
    {
        if (addr_arr[core_id] == INVALID_TABLE_ADDRESS)
            continue;

        ret = _rdd_field_read_single_core(addr_arr, addr, lsb, width, size, core_id);
        break;
    }
    return ret;
}

#if defined(RDP_SIM)

#if defined(__LP64__) || defined(_LP64)
/* 64 bit*/
#define GET_ADDR_LOW(lsb_addr, phys_ring_address) (lsb_addr = phys_ring_address & 0xFFFFFFFF);
#else
/* 32 bit */
#define GET_ADDR_LOW(lsb_addr, phys_ring_address) (lsb_addr = ((uint32_t)phys_ring_address & 0xFFFFFFFF));
#endif

static int is_rdp_block_addr(void *addr)
{
    return (uint8_t *)addr >= soc_base_address && (uint8_t *)addr <= soc_base_address + SIM_MEM_SIZE;
}

static inline uint8_t read8(uint8_t *addr)
{
    if (g_runner_sim_connected && is_rdp_block_addr(addr))
    {
        uint32_t addr_low;

        GET_ADDR_LOW(addr_low, (bdmf_phys_addr_t)addr);
        return (uint8_t)_rdd_socket_read(addr_low, rdd_size_8);
    }
    else
        return *addr;
}

static inline uint16_t read16(uint16_t *addr)
{
    uint16_t val;

    if (g_runner_sim_connected && is_rdp_block_addr(addr))
    {
        uint32_t addr_low;

        GET_ADDR_LOW(addr_low, (bdmf_phys_addr_t)addr);
        val = (uint16_t)_rdd_socket_read(addr_low, rdd_size_16);
    }
    else
        val = *addr;
    return swap2bytes(val);
}

static inline uint32_t read32(uint32_t *addr)
{
    uint32_t val;

    if (g_runner_sim_connected && is_rdp_block_addr(addr))
    {
        uint32_t addr_low;

        GET_ADDR_LOW(addr_low, (bdmf_phys_addr_t)addr);
        val = (uint32_t)_rdd_socket_read(addr_low, rdd_size_32);
    }
    else
        val = *addr;
    return swap4bytes(val);
}

static inline void write8(uint8_t *addr, uint8_t value)
{
    if (g_runner_sim_connected && is_rdp_block_addr(addr))
    {
        uint32_t addr_low;

        GET_ADDR_LOW(addr_low, (bdmf_phys_addr_t)addr);
        _rdd_socket_write(addr_low, value, rdd_size_8);
    }
    else
        *addr = value;
}

static inline void write16(uint16_t *addr, uint16_t value)
{
    if (g_runner_sim_connected && is_rdp_block_addr(addr))
    {
        uint32_t addr_low;

        GET_ADDR_LOW(addr_low, (bdmf_phys_addr_t)addr);
        _rdd_socket_write(addr_low, value, rdd_size_16);
    }
    else
        *addr = swap2bytes(value);
}

static inline void write32(uint32_t *addr, uint32_t value)
{
    if (g_runner_sim_connected && is_rdp_block_addr(addr))
    {
        uint32_t addr_low;

        GET_ADDR_LOW(addr_low, (bdmf_phys_addr_t)addr);
        _rdd_socket_write(addr_low, value, rdd_size_32);
    }
    else
        *addr = swap4bytes(value);
}

#endif
#endif
