/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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


#ifndef _XRDP_DRV_NAT_CACHE_AG_H_
#define _XRDP_DRV_NAT_CACHE_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean natc_reset;
    bdmf_boolean natc_enable;
    bdmf_boolean regfile_fifo_reset;
    bdmf_boolean natc_smem_clear_by_update_disable;
    bdmf_boolean natc_smem_increment_on_reg_lookup;
    uint8_t nat_arb_st;
    bdmf_boolean decr_count_wraparound_enable;
    uint8_t multi_hash_limit;
    uint8_t nat_hash_mode;
    bdmf_boolean ddr_disable_on_reg_lookup;
    bdmf_boolean cache_update_on_ddr_miss;
    bdmf_boolean pending_fifo_entry_check_enable;
    uint8_t unused1;
    bdmf_boolean reg_8bit_in_32bit_swap_control;
    bdmf_boolean reg_32bit_in_64bit_swap_control;
    bdmf_boolean repeated_key_det_en;
    bdmf_boolean ddr_swap_all_control;
    bdmf_boolean smem_8bit_in_32bit_swap_control;
    bdmf_boolean smem_32bit_in_64bit_swap_control;
    bdmf_boolean ddr_64bit_in_128bit_swap_control;
    bdmf_boolean natc_init_done;
    bdmf_boolean unused0;
    bdmf_boolean natc_add_command_speedup_mode;
    bdmf_boolean ddr_enable;
    bdmf_boolean unused2;
    bdmf_boolean ddr_lookup_pending_fifo_mode_disable;
    bdmf_boolean ddr_replace_duplicated_cached_entry_enable;
    bdmf_boolean ddr_hash_swap;
    bdmf_boolean ddr_counter_8bit_in_32bit_swap_control;
    bdmf_boolean cache_update_on_reg_ddr_lookup;
    uint8_t unused3;
    uint8_t unused4;
    uint8_t cache_algo;
    uint8_t age_timer;
    bdmf_boolean age_timer_tick;
    bdmf_boolean cache_lookup_blocking_mode;
    bdmf_boolean ddr_8bit_in_32bit_swap_control;
    bdmf_boolean ddr_32bit_in_64bit_swap_control;
    uint8_t unused5;
} nat_cache_ctrl_status;

typedef struct
{
    uint32_t cache_hit_count;
    uint32_t cache_miss_count;
    uint32_t ddr_request_count;
    uint32_t ddr_evict_count;
    uint32_t ddr_block_count;
} nat_cache_natc_ctrs;

typedef struct
{
    bdmf_boolean non_cacheable_tbl0;
    bdmf_boolean key_len_tbl0;
    bdmf_boolean non_cacheable_tbl1;
    bdmf_boolean key_len_tbl1;
    bdmf_boolean non_cacheable_tbl2;
    bdmf_boolean key_len_tbl2;
    bdmf_boolean non_cacheable_tbl3;
    bdmf_boolean key_len_tbl3;
    bdmf_boolean non_cacheable_tbl4;
    bdmf_boolean key_len_tbl4;
    bdmf_boolean non_cacheable_tbl5;
    bdmf_boolean key_len_tbl5;
    bdmf_boolean non_cacheable_tbl6;
    bdmf_boolean key_len_tbl6;
    bdmf_boolean non_cacheable_tbl7;
    bdmf_boolean key_len_tbl7;
    bdmf_boolean var_context_len_en_tbl0;
    bdmf_boolean var_context_len_en_tbl1;
    bdmf_boolean var_context_len_en_tbl2;
    bdmf_boolean var_context_len_en_tbl3;
    bdmf_boolean var_context_len_en_tbl4;
    bdmf_boolean var_context_len_en_tbl5;
    bdmf_boolean var_context_len_en_tbl6;
    bdmf_boolean var_context_len_en_tbl7;
    bdmf_boolean smem_dis_tbl0;
    bdmf_boolean smem_dis_tbl1;
    bdmf_boolean smem_dis_tbl2;
    bdmf_boolean smem_dis_tbl3;
    bdmf_boolean smem_dis_tbl4;
    bdmf_boolean smem_dis_tbl5;
    bdmf_boolean smem_dis_tbl6;
    bdmf_boolean smem_dis_tbl7;
} nat_cache_natc_table_control;

typedef struct
{
    uint8_t command;
    bdmf_boolean unused0;
    bdmf_boolean busy;
    bdmf_boolean error;
    bdmf_boolean miss;
    bdmf_boolean cache_hit;
    uint8_t multihash_count;
    uint8_t nat_tbl;
    bdmf_boolean decr_count;
    bdmf_boolean cache_flush;
    bdmf_boolean del_cmd_mode;
    bdmf_boolean add_cmd_mode;
    uint8_t del_cmd_ddr_bin;
    uint8_t add_cmd_ddr_bin;
    bdmf_boolean add_cmd_ddr_miss;
} nat_cache_nat0_command_status;

typedef struct
{
    uint32_t data[18];
} nat_cache_nat0_key_result;

typedef struct
{
    uint8_t ddr_size_tbl0;
    uint8_t ddr_size_tbl1;
    uint8_t ddr_size_tbl2;
    uint8_t ddr_size_tbl3;
    uint8_t ddr_size_tbl4;
    uint8_t ddr_size_tbl5;
    uint8_t ddr_size_tbl6;
    uint8_t ddr_size_tbl7;
} nat_cache_natc_ddr_size;

typedef struct
{
    uint8_t total_len_tbl0;
    uint8_t total_len_tbl1;
    uint8_t total_len_tbl2;
    uint8_t total_len_tbl3;
    uint8_t total_len_tbl4;
    uint8_t total_len_tbl5;
    uint8_t total_len_tbl6;
    uint8_t total_len_tbl7;
} nat_cache_natc_total_len;

typedef struct
{
    uint16_t nat_state;
    bdmf_boolean wb_state;
    bdmf_boolean runner_cmd_state;
    uint8_t ddr_rep_state;
    uint8_t ddr_req_state;
    uint8_t apb_state;
    uint8_t debug_sel;
} nat_cache_natc_sm_status;

typedef struct
{
    uint8_t ddr_hash_mode_tbl0;
    uint8_t ddr_hash_mode_tbl1;
    uint8_t ddr_hash_mode_tbl2;
    uint8_t ddr_hash_mode_tbl3;
    uint8_t ddr_hash_mode_tbl4;
    uint8_t ddr_hash_mode_tbl5;
    uint8_t ddr_hash_mode_tbl6;
    uint8_t ddr_hash_mode_tbl7;
} nat_cache_natc_ddr_hash_mode;

typedef struct
{
    uint8_t context_offset;
    bdmf_boolean flow_cntr_en_tbl0;
    bdmf_boolean flow_cntr_en_tbl1;
    bdmf_boolean flow_cntr_en_tbl2;
    bdmf_boolean flow_cntr_en_tbl3;
    bdmf_boolean flow_cntr_en_tbl4;
    bdmf_boolean flow_cntr_en_tbl5;
    bdmf_boolean flow_cntr_en_tbl6;
    bdmf_boolean flow_cntr_en_tbl7;
    bdmf_boolean flow_cntr_clr_on_rd_en;
} nat_cache_natc_flow_cntr_cntl;


/**********************************************************************************************************************
 * natc_reset: 
 *     Self Clearing Block Reset (including resetting all registers to default values)
 *     
 * natc_enable: 
 *     Enables all NATC state machines and input FIFO;
 *     Clearing this bit will halt all state machines gracefully to idle states,
 *     all outstanding transactions in the FIFO will remain in the FIFO and NATC
 *     will stop accepting new commands;  All configuration registers should be
 *     configured before enabling this bit.
 *     
 * regfile_fifo_reset: 
 *     Reset regfile_FIFO and ddr pending memory.
 *     
 * natc_smem_clear_by_update_disable: 
 *     Disables clearing counters when an existing entry is replaced by ADD command
 *     
 * natc_smem_increment_on_reg_lookup: 
 *     Enables incrementing or decrementing hit counter by 1 and byte counter by PKT_LEN
 *     on successful lookups using register interface
 *     BY default, counters only increment on successful lookups on Runner interface
 *     
 * nat_arb_st: 
 *     NAT Arbitration Mechanism
 *     
 * decr_count_wraparound_enable: 
 *     Decrement Count Wraparound Enable
 *     0h: Do not decrement counters for decrement command when counters reach 0
 *     1h: Always decrement counters for decrement command; will wrap around from 0 to all 1's
 *     
 * multi_hash_limit: 
 *     Maximum number of multi-hash iterations.
 *     This is not used if cache size is 32 cache entries or less.
 *     Value of 0 is 1 iteration, 1 is 2 iterations, 2 is 3 iterations, etc.
 *     This is not used if cache size is 32 entries or less.
 *     
 * nat_hash_mode: 
 *     Hash algorithm used for internal caching
 *     0h: 32-bit rolling XOR hash is used as cache hash function.
 *     1h: CRC32 hash is used as cache hash function. CRC32 is reduced to N-bit using
 *     the same method as in 32-bit rolling XOR hash.
 *     2h: CRC32 hash is used as cache hash function. CRC32[N:0] is used as hash value.
 *     3h: CRC32 hash is used as cache hash function. CRC32[31:N] is used as hash value.
 *     4h: RSS hash is used as cache hash function using secret key 0. RSS[N:0] is used as hash value.
 *     5h: RSS hash is used as cache hash function using secret key 1. RSS[N:0] is used as hash value.
 *     6h: RSS hash is used as cache hash function using secret key 2. RSS[N:0] is used as hash value.
 *     7h: RSS hash is used as cache hash function using secret key 3. RSS[N:0] is used as hash value.
 *     
 * ddr_disable_on_reg_lookup: 
 *     This bit prevents register interface lookup to access DDR
 *     0h: Enable register interface lookup in DDR.
 *     1h: Disable register interface lookup in DDR.
 *     Register interface lookup will only return lookup results in Cache.
 *     
 * cache_update_on_ddr_miss: 
 *     This bit enables caching for miss entry
 *     1h: Enable; miss entry in both cache and DDR will be cached.
 *     0h: Disable; miss entry in both cache and DDR will be not cached.
 *     
 * pending_fifo_entry_check_enable: 
 *     This bit disables caching DDR miss entry function when there is no additional lookup having
 *     the same key in pending fifo (e.g., DDR miss entry is cached only if there are 2 or more lookup
 *     of the same key within a 32 lookup window).
 *     This is to reduce excessive caching of miss entries.
 *     This bit is only valid when CACHE_UPDATE_ON_DDR_MISS bit is set to 1.
 *     1h: Enable; miss entry fetched from DDR will be cached if pending FIFO
 *     contains the same lookup having the same hash value as miss entry.
 *     0h: Disable; miss entry fetched from DDR will always be cached.
 * unused1: 
 *     Unused field since BCM68880B0/BCM68370A0.
 *     
 *     In past those bits were used as Hash algorithm used to detect DDR pending operations with default =1h: CRC32
 *     hash is used as cache hash function.
 *     Now design has hard coded CRC32 hash mode used for pending_tables (not cache or ddr).
 * reg_8bit_in_32bit_swap_control: 
 *     Reverse bytes within 32-bit word for key_result register accesses
 *     (i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})
 *     
 * reg_32bit_in_64bit_swap_control: 
 *     Swap 32-bit word within 64-bit word for key_result register accesses
 *     (i.e., [63:0] becomes {[31:0], [63:32]})
 *     
 * repeated_key_det_en: 
 *     Enable repeated key detection to improve cache lookup performance for repeated key.
 *     
 * ddr_swap_all_control: 
 *     Swap all bytes on DDR interface.
 *     
 * smem_8bit_in_32bit_swap_control: 
 *     Reverse bytes within 32-bit word for statistics (counter) memory accesses
 *     (i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})
 *     
 * smem_32bit_in_64bit_swap_control: 
 *     Swap 32-bit word within 64-bit word for statistics (counter) memory accesses
 *     (i.e., [63:0] becomes {[31:0], [63:32]})
 *     
 * ddr_64bit_in_128bit_swap_control: 
 *     Swap 64-bit word within 128-bit word for DDR memory read/write accesses
 *     (i.e., [127:0] becomes {[63:0], [127:64]}).
 *     
 * natc_init_done: 
 *     This bit is set to 1 when NATC cache memories have been initialized to 0's.
 *     
 * unused0: 
 *     No Descriotion
 * natc_add_command_speedup_mode: 
 *     Default behavior for an ADD command is to do a LOOKUP first to see if the entry
 *     with the same key already exists and replace it; this is to avoid having duplicated
 *     entries in the table for ADD command.  When this bit is set an ADD command will
 *     either replace the entry with the matched key or add an entry to an empty entry
 *     depending on whichever one is encountered first during multi-hash.  Enabling
 *     this bit speeds up the ADD command.
 *     
 * ddr_enable: 
 *     Enables NAT table offload to DDR functionality.
 *     NATC_CONTROL_STATUS2 register should be configured before enabling this feature.
 *     
 * unused2: 
 *     No Descriotion
 * ddr_lookup_pending_fifo_mode_disable: 
 *     (debug command) Do not set this bit to 1
 *     
 * ddr_replace_duplicated_cached_entry_enable: 
 *     (debug command) Do not set this bit to 1
 *     
 * ddr_hash_swap: 
 *     Reverse bytes within 18-bit DDR hash value
 *     
 * ddr_counter_8bit_in_32bit_swap_control: 
 *     Reverse bytes within 32-bit word for DDR counters on read/write accesses.
 *     (i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})
 *     
 * cache_update_on_reg_ddr_lookup: 
 *     This bit determines whether register interface lookup will cache the entry from DDR
 *     1h: Enable; entry fetched from DDR will be cached using register interface lookup command
 *     0h: Disable; entry fetched from DDR will not be cached using register interface lookup command
 *     
 * unused3: 
 *     No Descriotion
 * unused4: 
 *     No Descriotion
 * cache_algo: 
 *     Replacement algorithm for caching
 *     Lowest-multi-hash-iteration number is used to select the final replacement
 *     entry if multiple entries were chosen by the selected algorithm.  For
 *     instance, if HIT_COUNT algorithm were selected, and 2nd, 3rd and 7th
 *     entry all have the same hit_count values, 2nd entry will be evicted.
 *     
 * age_timer: 
 *     Timer value used for pseudo-LRU;
 *     When timer fires the 8-bit age value of every entry in the cache is
 *     decremented (cap at 0).  The entry with lower value is
 *     the older entry.  The default setting keeps track of ~0.26s age at
 *     ~1ms resolution.
 *     0: 1 tick
 *     1: 2 ticks
 *     2: 4 ticks
 *     3: 8 ticks
 *     4: 16 ticks
 *     ..
 *     ..
 *     31: 2^31 TICKS
 *     
 * age_timer_tick: 
 *     Timer tick for pseudo-LRU
 *     
 * cache_lookup_blocking_mode: 
 *     (debug command) Do not set this bit to 1
 *     
 * ddr_8bit_in_32bit_swap_control: 
 *     Reverse bytes within 32-bit word for DDR memory read/write accesses
 *     (i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]}).
 *     
 * ddr_32bit_in_64bit_swap_control: 
 *     Swap 32-bit word within 64-bit word for DDR memory read/write accesses
 *     (i.e., [63:0] becomes {[31:0], [63:32]}).
 *     
 * unused5: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_ctrl_status_set(const nat_cache_ctrl_status *ctrl_status);
bdmf_error_t ag_drv_nat_cache_ctrl_status_get(nat_cache_ctrl_status *ctrl_status);

/**********************************************************************************************************************
 * cache_hit_count: 
 *     32-bit total cache hit count value for statistics collection
 *     
 * cache_miss_count: 
 *     32-bit total cache miss count value for statistics collection
 *     
 * ddr_request_count: 
 *     32-bit total DDR request count value for statistics collection
 *     
 * ddr_evict_count: 
 *     32-bit total DDR evict count value for statistics collection.
 *     It does not include the flush command evict count.
 *     
 * ddr_block_count: 
 *     32-bit total DDR blocked access count value for statistics collection
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_ctrs_set(const nat_cache_natc_ctrs *natc_ctrs);
bdmf_error_t ag_drv_nat_cache_natc_ctrs_get(nat_cache_natc_ctrs *natc_ctrs);

/**********************************************************************************************************************
 * non_cacheable_tbl0: 
 *     DDR table 0 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 *     
 * key_len_tbl0: 
 *     Length of the key for DDR table 0
 *     0h: 16-byte key
 *     1h: 32-byte key
 *     
 * non_cacheable_tbl1: 
 *     DDR table 1 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 *     
 * key_len_tbl1: 
 *     Length of the key for DDR table 1
 *     0h: 16-byte key
 *     1h: 32-byte key
 *     
 * non_cacheable_tbl2: 
 *     DDR table 2 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 *     
 * key_len_tbl2: 
 *     Length of the key for DDR table 2
 *     0h: 16-byte key
 *     1h: 32-byte key
 *     
 * non_cacheable_tbl3: 
 *     DDR table 3 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 *     
 * key_len_tbl3: 
 *     Length of the key for DDR table 3
 *     0h: 16-byte key
 *     1h: 32-byte key
 *     
 * non_cacheable_tbl4: 
 *     DDR table 4 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 *     
 * key_len_tbl4: 
 *     Length of the key for DDR table 4
 *     0h: 16-byte key
 *     1h: 32-byte key
 *     
 * non_cacheable_tbl5: 
 *     DDR table 5 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 *     
 * key_len_tbl5: 
 *     Length of the key for DDR table 5
 *     0h: 16-byte key
 *     1h: 32-byte key
 *     
 * non_cacheable_tbl6: 
 *     DDR table 6 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 *     
 * key_len_tbl6: 
 *     Length of the key for DDR table 6
 *     0h: 16-byte key
 *     1h: 32-byte key
 *     
 * non_cacheable_tbl7: 
 *     DDR table 7 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 *     
 * key_len_tbl7: 
 *     Length of the key for DDR table 7
 *     0h: 16-byte key
 *     1h: 32-byte key
 *     
 * var_context_len_en_tbl0: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 0
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 *     
 * var_context_len_en_tbl1: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 1
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 *     
 * var_context_len_en_tbl2: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 2
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 *     
 * var_context_len_en_tbl3: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 3
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 *     
 * var_context_len_en_tbl4: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 4
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 *     
 * var_context_len_en_tbl5: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 5
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 *     
 * var_context_len_en_tbl6: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 6
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 *     
 * var_context_len_en_tbl7: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 7
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 *     
 * smem_dis_tbl0: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 0
 *     
 * smem_dis_tbl1: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 1
 *     
 * smem_dis_tbl2: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 2
 *     
 * smem_dis_tbl3: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 3
 *     
 * smem_dis_tbl4: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 4
 *     
 * smem_dis_tbl5: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 5
 *     
 * smem_dis_tbl6: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 6
 *     
 * smem_dis_tbl7: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 7
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_table_control_set(const nat_cache_natc_table_control *natc_table_control);
bdmf_error_t ag_drv_nat_cache_natc_table_control_get(nat_cache_natc_table_control *natc_table_control);

/**********************************************************************************************************************
 * command: 
 *     Command to be executed
 *     This command only operates on the entries in cache except LOOKUP command where
 *     entry can be fetched from DDR.
 *     Writing to this field causes BUSY bit to be set.
 *     Note: For all commands, key consists of all 0's indicates unused entry in h/w
 *     and therefore cannot be used.
 *     
 * unused0: 
 *     No Descriotion
 * busy: 
 *     Interface Busy
 *     This bit is set when command is issued but still in progress been processed.
 *     When command completes this bit will be cleared.
 *     
 * error: 
 *     This bit is set for the following 2 cases
 *     For ADD command all multi-hash entries are occupied (i.e, no room to ADD)
 *     For DEL command entry is not found and cannot be deleted
 *     
 * miss: 
 *     This bit is set when a LOOKUP command has a miss
 *     
 * cache_hit: 
 *     This bit is set when a LOOKUP command has a cache hit
 *     
 * multihash_count: 
 *     Cache multi-hash iteration count status
 *     Value of 0 is iteration 1, 1 is iteration 2, 2 is iteration 3, etc.
 *     cache miss returns 0 count.
 *     
 * nat_tbl: 
 *     Select the DDR Table on which the command will operate
 *     0h: DDR table 0
 *     1h: DDR table 1
 *     2h: DDR table 2
 *     3h: DDR table 3
 *     4h: DDR table 4
 *     5h: DDR table 5
 *     6h: DDR table 6
 *     7h: DDR table 7
 *     
 * decr_count: 
 *     Decrement-counter mode enable
 *     When set, LOOKUP command will decrement hit counter by 1 and decrement
 *     byte counter by the value specified in PKT_LEN, on a successful lookup.
 *     NATC_SMEM_INCREMENT_ON_REG_LOOKUP must be set to 1 for it to be effective
 *     0h: LOOKUP command will increment hit counter and byte counter
 *     1h: LOOKUP command will decrement hit counter and byte counter
 *     
 * cache_flush: 
 *     Cache Flush enable
 *     When set, LOOKUP command is used to flush counters from cache into DDR.
 *     This command does not use key to lookup the cache entry.  Instead it uses
 *     cache index number located in 10-MSB bits of key specified in NAT_KEY_RESULT register.
 *     For 16 bytes key, the cache index will be located in
 *     {NAT_KEY_RESULT[15], NAT_KEY_RESULT[14][7:6]} (15th byte of NAT_KEY_RESULT register and
 *     bits 7:6 of 14th byte of NAT_KEY_RESULT register).
 *     For 32 bytes key, the cache index will be located in
 *     {NAT_KEY_RESULT[31], NAT_KEY_RESULT[30][7:6]} (31th byte of NAT_KEY_RESULT register and
 *     bits 7:6 of 30th byte of NAT_KEY_RESULT register).
 *     0h: LOOKUP command is used as normal lookup command.
 *     1h: LOOKUP command is used as cache flush command.
 *     
 * del_cmd_mode: 
 *     DEL Command DDR-bin matching mode enable
 *     0h: DEL command deletes the cache entry with matching key
 *     1h: DEL command deletes the cache entry with matching key and matching DDR bin
 *     number specified in DEL_CMD_DDR_BIN field
 *     
 * add_cmd_mode: 
 *     ADD Command mode
 *     0h: ADD command writes 0 (DDR bin number and DDR miss flag) to cache.
 *     1h: ADD command writes DDR bin number and DDR miss flag to cache
 *     specified in ADD_CMD_DDR_BIN and ADD_CMD_DDR_MISS fields.
 *     
 * del_cmd_ddr_bin: 
 *     This filed specifies the DDR BIN number to be compared for DEL command
 *     when DEL_CMD_MODE is set to 1
 *     
 * add_cmd_ddr_bin: 
 *     This filed specifies the DDR BIN number to be written to cache for ADD command
 *     when ADD_CMD_MODE is set to 1.
 *     
 * add_cmd_ddr_miss: 
 *     This filed specifies the DDR MISS flag to be written to cache for ADD command
 *     when ADD_CMD_MODE is set to 1.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_nat0_command_status_set(const nat_cache_nat0_command_status *nat0_command_status);
bdmf_error_t ag_drv_nat_cache_nat0_command_status_get(nat_cache_nat0_command_status *nat0_command_status);

/**********************************************************************************************************************
 * hash: 
 *     hash value; only valid on a successful lookup/add/del command
 *     For cache hit 10-bit hash value is returned.
 *     For cache miss and DDR_ENABLE is 0, first hash value (10-bit) is returned.
 *     For cache miss, DDR_ENABLE is 1 and DDR is a hit, 18-bit DDR hash value + DDR bin count is returned.
 *     For cache miss, DDR_ENABLE is 1 and DDR is a miss, 18-bit DDR hash value is returned.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_nat0_hash_get(uint32_t *hash);

/**********************************************************************************************************************
 * hit_count: 
 *     bits 27:0 are 28-bit hit count value.
 *     bits 31:28 are 4 lsb of 36-bit byte count value.
 *     only valid on a successful lookup or delete command.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_nat0_hit_count_get(uint32_t *hit_count);

/**********************************************************************************************************************
 * byte_count: 
 *     32-bit msb of 36-bit byte count value.
 *     {BYTE_COUNT, HIT_COUNT[31:28]} is the 36-bit byte count value.
 *     only valid on a successful lookup or delete command
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_nat0_byte_count_get(uint32_t *byte_count);

/**********************************************************************************************************************
 * pkt_len: 
 *     16-bit packet length value used to increment or decrement byte counter
 *     
 * unused: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_nat0_pkt_len_set(uint16_t pkt_len, uint16_t unused);
bdmf_error_t ag_drv_nat_cache_nat0_pkt_len_get(uint16_t *pkt_len, uint16_t *unused);

/**********************************************************************************************************************
 * nat_key_result: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_nat0_key_result_set(uint8_t zero, const nat_cache_nat0_key_result *nat0_key_result);
bdmf_error_t ag_drv_nat_cache_nat0_key_result_get(uint8_t zero, nat_cache_nat0_key_result *nat0_key_result);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * zeros: 
 *     No Descriotion
 * bar: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar);
bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar);

/**********************************************************************************************************************
 * bar: 
 *     No Descriotion
 * zeros: 
 *     No Descriotion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros);
bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros);

/**********************************************************************************************************************
 * cache_hit_count_en: 
 *     Cache hit counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable cache hit counter increment on cache hit.
 *     1h: Enable cache hit counter increment on cache hit.
 *     
 * cache_miss_count_en: 
 *     Cache miss counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable cache miss counter increment on cache miss.
 *     1h: Enable cache miss counter increment on cache miss.
 *     
 * ddr_request_count_en: 
 *     DDR request counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable DDR request counter increment on ddr request.
 *     1h: Enable DDR request counter increment on ddr request.
 *     
 * ddr_evict_count_en: 
 *     DDR evict counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable DDR evict counter increment on ddr evict.
 *     1h: Enable DDR evict counter increment on ddr evict.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_stat_counter_control_0_set(uint8_t cache_hit_count_en, uint8_t cache_miss_count_en, uint8_t ddr_request_count_en, uint8_t ddr_evict_count_en);
bdmf_error_t ag_drv_nat_cache_natc_stat_counter_control_0_get(uint8_t *cache_hit_count_en, uint8_t *cache_miss_count_en, uint8_t *ddr_request_count_en, uint8_t *ddr_evict_count_en);

/**********************************************************************************************************************
 * ddr_block_count_en: 
 *     DDR block counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable DDR block counter increment on ddr block.
 *     1h: Enable DDR block counter increment on ddr block.
 *     
 * counter_wraparound_dis: 
 *     Counter wraparound disable
 *     This applies to all stat counters defined in
 *     NATC_STAT_COUNTER_CONTROL_0 and  NATC_STAT_COUNTER_CONTROL_1.
 *     0h: The counter will wraparound to 0 after it reaches ffffffffh.
 *     1h: The counter will cap at ffffffffh.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_stat_counter_control_1_set(uint8_t ddr_block_count_en, bdmf_boolean counter_wraparound_dis);
bdmf_error_t ag_drv_nat_cache_natc_stat_counter_control_1_get(uint8_t *ddr_block_count_en, bdmf_boolean *counter_wraparound_dis);

/**********************************************************************************************************************
 * ddr_size_tbl0: 
 *     Number of entries in DDR table 0
 *     Value of 6 or above is invalid
 *     To compute the actual size of the table, add DDR_BINS_PER_BUCKET field
 *     to the table size selection;
 *     For instance, if DDR_BINS_PER_BUCKET is 3 (4 bins per bucket)
 *     and DDR_size is 3 (64k entries), the actual size of the table in DDR is
 *     (64*1024+3) multiply by total length (TOTAL_LEN) of key and context in bytes
 *     Extra 3 entries are used to store collided entries of the last entry
 *     
 * ddr_size_tbl1: 
 *     Number of entries in DDR table 1
 *     0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid
 *     See description of DDR_SIZE_TBL0
 *     
 * ddr_size_tbl2: 
 *     Number of entries in DDR table 2
 *     See description of DDR_SIZE_TBL0
 *     0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid
 *     
 * ddr_size_tbl3: 
 *     Number of entries in DDR table 3
 *     See description of DDR_SIZE_TBL0
 *     0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid
 *     
 * ddr_size_tbl4: 
 *     Number of entries in DDR table 4
 *     See description of DDR_SIZE_TBL0
 *     0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid
 *     
 * ddr_size_tbl5: 
 *     Number of entries in DDR table 5
 *     See description of DDR_SIZE_TBL0
 *     0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid
 *     
 * ddr_size_tbl6: 
 *     Number of entries in DDR table 6
 *     See description of DDR_SIZE_TBL0
 *     0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid
 *     
 * ddr_size_tbl7: 
 *     Number of entries in DDR table 7
 *     See description of DDR_SIZE_TBL0
 *     0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_ddr_size_set(const nat_cache_natc_ddr_size *natc_ddr_size);
bdmf_error_t ag_drv_nat_cache_natc_ddr_size_get(nat_cache_natc_ddr_size *natc_ddr_size);

/**********************************************************************************************************************
 * ddr_bins_per_bucket_tbl0: 
 *     Number of entries per bucket in DDR table 0
 *     This is limited by bus max burst size.  For instance, if
 *     UBUS supports max burst size of 128 bytes, key length is 16
 *     bytes, maximum DDR_BINS_PER_BUCKET that can be programmed
 *     is 128 bytes / 16-bytes (bytes per bin) = 8 entries
 *     0h: 1 entry
 *     1h: 2 entries
 *     2h: 3 entries
 *     3h: 4 entries
 *     4h: 5 entries
 *     5h: 6 entries
 *     6h: 7 entries
 *     7h: 8 entries
 *     .............
 *     
 * ddr_bins_per_bucket_tbl1: 
 *     Number of entries per bucket in DDR table 1
 *     See description of DDR_BINS_PER_BUCKET_TBL0
 *     
 * ddr_bins_per_bucket_tbl2: 
 *     Number of entries per bucket in DDR table 2
 *     See description of DDR_BINS_PER_BUCKET_TBL0
 *     
 * ddr_bins_per_bucket_tbl3: 
 *     Number of entries per bucket in DDR table 3
 *     See description of DDR_BINS_PER_BUCKET_TBL0
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_set(uint8_t ddr_bins_per_bucket_tbl0, uint8_t ddr_bins_per_bucket_tbl1, uint8_t ddr_bins_per_bucket_tbl2, uint8_t ddr_bins_per_bucket_tbl3);
bdmf_error_t ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_get(uint8_t *ddr_bins_per_bucket_tbl0, uint8_t *ddr_bins_per_bucket_tbl1, uint8_t *ddr_bins_per_bucket_tbl2, uint8_t *ddr_bins_per_bucket_tbl3);

/**********************************************************************************************************************
 * ddr_bins_per_bucket_tbl4: 
 *     Number of entries per bucket in DDR table 4
 *     See description of DDR_BINS_PER_BUCKET_TBL0
 *     
 * ddr_bins_per_bucket_tbl5: 
 *     Number of entries per bucket in DDR table 5
 *     See description of DDR_BINS_PER_BUCKET_TBL0
 *     
 * ddr_bins_per_bucket_tbl6: 
 *     Number of entries per bucket in DDR table 6
 *     See description of DDR_BINS_PER_BUCKET_TBL0
 *     
 * ddr_bins_per_bucket_tbl7: 
 *     Number of entries per bucket in DDR table 7
 *     See description of DDR_BINS_PER_BUCKET_TBL0
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_set(uint8_t ddr_bins_per_bucket_tbl4, uint8_t ddr_bins_per_bucket_tbl5, uint8_t ddr_bins_per_bucket_tbl6, uint8_t ddr_bins_per_bucket_tbl7);
bdmf_error_t ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_get(uint8_t *ddr_bins_per_bucket_tbl4, uint8_t *ddr_bins_per_bucket_tbl5, uint8_t *ddr_bins_per_bucket_tbl6, uint8_t *ddr_bins_per_bucket_tbl7);

/**********************************************************************************************************************
 * total_len_tbl0: 
 *     Length of the lookup key plus context (including 8-byte counters) in DDR table 0
 *     The context length (including 8-byte counters) is calculated by TOTAL_LEN minus KEY_LEN
 *     The maximum value should not exceed hardware capability.
 *     For instance, most projects have max of 80-bytes and BCM63158 has max value of 144-byte.
 *     0h: 48-byte
 *     1h: 64-byte
 *     2h: 80-byte
 *     3h: 96-byte
 *     4h: 112-byte
 *     5h: 128-byte
 *     6h: 144-byte
 *     7h: 160-byte
 *     
 * total_len_tbl1: 
 *     Length of the lookup key plus context (including 8-byte counters) in DDR table 1
 *     See description of TOTAL_LEN_TBL0.
 *     
 * total_len_tbl2: 
 *     Length of the lookup key plus context (including 8-byte counters) in DDR table 2
 *     See description of TOTAL_LEN_TBL0.
 *     
 * total_len_tbl3: 
 *     Length of the lookup key plus context (including 8-byte counters) in DDR table 3
 *     See description of TOTAL_LEN_TBL0.
 *     
 * total_len_tbl4: 
 *     Length of the lookup key plus context (including 8-byte counters) in DDR table 4
 *     See description of TOTAL_LEN_TBL0.
 *     
 * total_len_tbl5: 
 *     Length of the lookup key plus context (including 8-byte counters) in DDR table 5
 *     See description of TOTAL_LEN_TBL0.
 *     
 * total_len_tbl6: 
 *     Length of the lookup key plus context (including 8-byte counters) in DDR table 6
 *     See description of TOTAL_LEN_TBL0.
 *     
 * total_len_tbl7: 
 *     Length of the lookup key plus context (including 8-byte counters) in DDR table 7
 *     See description of TOTAL_LEN_TBL0.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_total_len_set(const nat_cache_natc_total_len *natc_total_len);
bdmf_error_t ag_drv_nat_cache_natc_total_len_get(nat_cache_natc_total_len *natc_total_len);

/**********************************************************************************************************************
 * nat_state: 
 *     Nat state machine.
 *     15'b000000000000000: NAT_ST_IDLE.
 *     15'b000000000000001: NAT_ST_IDLE_WRITE_SMEM.
 *     15'b000000000000010: NAT_ST_IDLE_DDR_PENDING.
 *     15'b000000000000100: NAT_ST_HASH.
 *     15'b000000000001000: NAT_ST_NAT_MEM_READ_REQ.
 *     15'b000000000010000: NAT_ST_NAT_MEM_WRITE_REQ.
 *     15'b000000000100000: NAT_ST_READ_SMEM.
 *     15'b000000001000000: NAT_ST_UPDATE_DDR.
 *     15'b000000010000000: NAT_ST_IDLE_BLOCKING_PENDING.
 *     15'b000000100000000: NAT_ST_EVICT_WAIT.
 *     15'b000001000000000: NAT_ST_CHECK_NON_CACHEABLE.
 *     15'b000010000000000: NAT_ST_WAIT.
 *     15'b000100000000000: NAT_ST_WAIT_NATC_MEM_REQ_DONE.
 *     15'b001000000000000: NAT_ST_CACHE_FLUSH.
 *     15'b010000000000000: NAT_ST_DDR_MISS_0.
 *     15'b100000000000000: NAT_ST_DDR_MISS_1.
 *     
 * wb_state: 
 *     Write-back state machine.
 *     1'b0: WB_ST_IDLE.
 *     1'b1: WB_ST_WRITE_BACIF.
 *     
 * runner_cmd_state: 
 *     Runner command state machine.
 *     1'b0: RUNNER_CMD_ST_IDLE.
 *     1'b1: RUNNER_CMD_ST_WRITE_RUNNER_FIFO.
 *     
 * ddr_rep_state: 
 *     DDR reply state machine.
 *     3'b000: DDR_REP_ST_IDLE.
 *     3'b001: DDR_REP_ST_READ_DATA.
 *     3'b010: DDR_REP_ST_READ_RESULT.
 *     3'b011: DDR_REP_ST_READ_WAIT.
 *     3'b100: DDR_REP_ST_EVICT_WR_NON_CACHEABLE.
 *     
 * ddr_req_state: 
 *     DDR request state machine.
 *     2'b00: DDR_REQ_ST_IDLE.
 *     2'b01: DDR_REQ_ST_WRITE_HEADER.
 *     2'b10: DDR_REQ_ST_WRITE_HEADER_DELAY.
 *     
 * apb_state: 
 *     APB to RBUS bridge state machine.
 *     2'b00: APB_ST_IDLE.
 *     2'b01: APB_ST_RW.
 *     2'b10: AOB_ST_END.
 *     
 * debug_sel: 
 *     Debug bus select.
 *     2'b00: prb_nat_control.
 *     2'b01: prb_cmd_control.
 *     2'b10: prb_wb_control.
 *     2'b11: prb_ddr_control.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_sm_status_set(const nat_cache_natc_sm_status *natc_sm_status);
bdmf_error_t ag_drv_nat_cache_natc_sm_status_get(nat_cache_natc_sm_status *natc_sm_status);

/**********************************************************************************************************************
 * regfile_fifo_start_addr_0: 
 *     REGFILE FIFO 0 Start Address
 *     
 * regfile_fifo_start_addr_1: 
 *     REGFILE FIFO 1 Start Address
 *     
 * regfile_fifo_start_addr_2: 
 *     REGFILE FIFO 2 Start Address
 *     
 * regfile_fifo_start_addr_3: 
 *     REGFILE FIFO 2 Start Address
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_set(uint8_t regfile_fifo_start_addr_0, uint8_t regfile_fifo_start_addr_1, uint8_t regfile_fifo_start_addr_2, uint8_t regfile_fifo_start_addr_3);
bdmf_error_t ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_get(uint8_t *regfile_fifo_start_addr_0, uint8_t *regfile_fifo_start_addr_1, uint8_t *regfile_fifo_start_addr_2, uint8_t *regfile_fifo_start_addr_3);

/**********************************************************************************************************************
 * regfile_fifo_start_addr_4: 
 *     REGFILE FIFO 4 Start Address
 *     
 * regfile_fifo_start_addr_5: 
 *     REGFILE FIFO 5 Start Address
 *     
 * regfile_fifo_start_addr_6: 
 *     REGFILE FIFO 6 Start Address
 *     
 * regfile_fifo_start_addr_7: 
 *     REGFILE FIFO 7 Start Address-- Note that this entry is not used
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_set(uint8_t regfile_fifo_start_addr_4, uint8_t regfile_fifo_start_addr_5, uint8_t regfile_fifo_start_addr_6, uint8_t regfile_fifo_start_addr_7);
bdmf_error_t ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_get(uint8_t *regfile_fifo_start_addr_4, uint8_t *regfile_fifo_start_addr_5, uint8_t *regfile_fifo_start_addr_6, uint8_t *regfile_fifo_start_addr_7);

/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl0_key_mask_set(uint32_t index, uint32_t key_mask);
bdmf_error_t ag_drv_nat_cache_natc_tbl0_key_mask_get(uint32_t index, uint32_t *key_mask);

/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl1_key_mask_set(uint32_t index, uint32_t key_mask);
bdmf_error_t ag_drv_nat_cache_natc_tbl1_key_mask_get(uint32_t index, uint32_t *key_mask);

/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl2_key_mask_set(uint32_t index, uint32_t key_mask);
bdmf_error_t ag_drv_nat_cache_natc_tbl2_key_mask_get(uint32_t index, uint32_t *key_mask);

/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl3_key_mask_set(uint32_t index, uint32_t key_mask);
bdmf_error_t ag_drv_nat_cache_natc_tbl3_key_mask_get(uint32_t index, uint32_t *key_mask);

/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl4_key_mask_set(uint32_t index, uint32_t key_mask);
bdmf_error_t ag_drv_nat_cache_natc_tbl4_key_mask_get(uint32_t index, uint32_t *key_mask);

/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl5_key_mask_set(uint32_t index, uint32_t key_mask);
bdmf_error_t ag_drv_nat_cache_natc_tbl5_key_mask_get(uint32_t index, uint32_t *key_mask);

/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl6_key_mask_set(uint32_t index, uint32_t key_mask);
bdmf_error_t ag_drv_nat_cache_natc_tbl6_key_mask_get(uint32_t index, uint32_t *key_mask);

/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_tbl7_key_mask_set(uint32_t index, uint32_t key_mask);
bdmf_error_t ag_drv_nat_cache_natc_tbl7_key_mask_get(uint32_t index, uint32_t *key_mask);

/**********************************************************************************************************************
 * ddr_hash_mode_tbl0: 
 *     Hash algorithm used for DDR table 0 lookup.
 *     Hash value is DDR table size dependent.
 *     0h: 32-bit rolling XOR hash is used as DDR hash function. It is reduced to N-bit
 *     DDR table size is 8K,   N = 13.
 *     DDR table size is 16K,  N = 14.
 *     DDR table size is 32K,  N = 15.
 *     DDR table size is 64K,  N = 16.
 *     DDR table size is 128K, N = 17.
 *     DDR table size is 256K, N = 18.
 *     1h: CRC32 hash is used as DDR hash function. CRC32 is reduced to N-bit using
 *     the same method as in 32-bit rolling XOR hash.
 *     DDR table size is 8K,   N = 13.
 *     DDR table size is 16K,  N = 14.
 *     DDR table size is 32K,  N = 15.
 *     DDR table size is 64K,  N = 16.
 *     DDR table size is 128K, N = 17.
 *     DDR table size is 256K, N = 18.
 *     2h: CRC32 hash is used as DDR hash function. CRC32[N:0] is used as hash value
 *     DDR table size is 8K,   N = 12.
 *     DDR table size is 16K,  N = 13.
 *     DDR table size is 32K,  N = 14.
 *     DDR table size is 64K,  N = 15.
 *     DDR table size is 128K, N = 16.
 *     DDR table size is 256K, N = 17.
 *     3h: CRC32 hash is used as DDR hash function. CRC32[31:N] is used as hash value
 *     DDR table size is 8K,   N = 19.
 *     DDR table size is 16K,  N = 18.
 *     DDR table size is 32K,  N = 17.
 *     DDR table size is 64K,  N = 16.
 *     DDR table size is 128K, N = 15.
 *     DDR table size is 256K, N = 14.
 *     4h: RSS hash is used as DDR hash function using secret key 0. RSS[N:0] is used as hash value.
 *     DDR table size is 8K,   N = 13.
 *     DDR table size is 16K,  N = 14.
 *     DDR table size is 32K,  N = 15.
 *     DDR table size is 64K,  N = 16.
 *     DDR table size is 128K, N = 17.
 *     DDR table size is 256K, N = 18.
 *     5h: RSS hash is used as DDR hash function using secret key 1. RSS[N:0] is used as hash value.
 *     DDR table size is 8K,   N = 13.
 *     DDR table size is 16K,  N = 14.
 *     DDR table size is 32K,  N = 15.
 *     DDR table size is 64K,  N = 16.
 *     DDR table size is 128K, N = 17.
 *     DDR table size is 256K, N = 18.
 *     6h: RSS hash is used as DDR hash function using secret key 2. RSS[N:0] is used as hash value.
 *     DDR table size is 8K,   N = 13.
 *     DDR table size is 16K,  N = 14.
 *     DDR table size is 32K,  N = 15.
 *     DDR table size is 64K,  N = 16.
 *     DDR table size is 128K, N = 17.
 *     DDR table size is 256K, N = 18.
 *     7h: RSS hash is used as DDR hash function using secret key 3. RSS[N:0] is used as hash value.
 *     DDR table size is 8K,   N = 13.
 *     DDR table size is 16K,  N = 14.
 *     DDR table size is 32K,  N = 15.
 *     DDR table size is 64K,  N = 16.
 *     DDR table size is 128K, N = 17.
 *     DDR table size is 256K, N = 18.
 *     8h: Key[N:4] is used as hash value.
 *     DDR table size is 8K,   N = 16.
 *     DDR table size is 16K,  N = 17.
 *     DDR table size is 32K,  N = 18.
 *     DDR table size is 64K,  N = 19.
 *     DDR table size is 128K, N = 20.
 *     DDR table size is 256K, N = 21.
 *     
 * ddr_hash_mode_tbl1: 
 *     Hash algorithm used for DDR table 1 lookup.
 *     See description of DDR_HASH_MODE_TBL0.
 *     
 * ddr_hash_mode_tbl2: 
 *     Hash algorithm used for DDR table 2 lookup.
 *     See description of DDR_HASH_MODE_TBL0.
 *     
 * ddr_hash_mode_tbl3: 
 *     Hash algorithm used for DDR table 3 lookup.
 *     See description of DDR_HASH_MODE_TBL0.
 *     
 * ddr_hash_mode_tbl4: 
 *     Hash algorithm used for DDR table 4 lookup.
 *     See description of DDR_HASH_MODE_TBL0.
 *     
 * ddr_hash_mode_tbl5: 
 *     Hash algorithm used for DDR table 5 lookup.
 *     See description of DDR_HASH_MODE_TBL0.
 *     
 * ddr_hash_mode_tbl6: 
 *     Hash algorithm used for DDR table 6 lookup.
 *     See description of DDR_HASH_MODE_TBL0.
 *     
 * ddr_hash_mode_tbl7: 
 *     Hash algorithm used for DDR table 7 lookup.
 *     See description of DDR_HASH_MODE_TBL0.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_ddr_hash_mode_set(const nat_cache_natc_ddr_hash_mode *natc_ddr_hash_mode);
bdmf_error_t ag_drv_nat_cache_natc_ddr_hash_mode_get(nat_cache_natc_ddr_hash_mode *natc_ddr_hash_mode);

/**********************************************************************************************************************
 * context_offset: 
 *     This is the byte offset of the context.
 *     Only bits 6:0 of this byte in the context are used as flow counter address.
 *     
 * flow_cntr_en_tbl0: 
 *     Enable flow counters for DDR table 0.
 *     If this bit is set to 1 and CONTEXT_OFFSET is less than context length - 8 (byte count and hit cout),
 *     the flow counter is enabled. Otherwsie the flow counter is disabled.
 *     
 * flow_cntr_en_tbl1: 
 *     Enable flow counters for DDR table 1.
 *     See description of FLOW_CNTR_EN_TBL0.
 *     
 * flow_cntr_en_tbl2: 
 *     Enable flow counters for DDR table 2.
 *     See description of FLOW_CNTR_EN_TBL0.
 *     
 * flow_cntr_en_tbl3: 
 *     Enable flow counters for DDR table 3.
 *     See description of FLOW_CNTR_EN_TBL0.
 *     
 * flow_cntr_en_tbl4: 
 *     Enable flow counters for DDR table 4.
 *     See description of FLOW_CNTR_EN_TBL0.
 *     
 * flow_cntr_en_tbl5: 
 *     Enable flow counters for DDR table 5.
 *     See description of FLOW_CNTR_EN_TBL0.
 *     
 * flow_cntr_en_tbl6: 
 *     Enable flow counters for DDR table 6.
 *     See description of FLOW_CNTR_EN_TBL0.
 *     
 * flow_cntr_en_tbl7: 
 *     Enable flow counters for DDR table 7.
 *     See description of FLOW_CNTR_EN_TBL0.
 *     
 * flow_cntr_clr_on_rd_en: 
 *     Enable flow counters clear on read.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_natc_flow_cntr_cntl_set(const nat_cache_natc_flow_cntr_cntl *natc_flow_cntr_cntl);
bdmf_error_t ag_drv_nat_cache_natc_flow_cntr_cntl_get(nat_cache_natc_flow_cntr_cntl *natc_flow_cntr_cntl);

/**********************************************************************************************************************
 * natc_entry: 
 *     NAT Cache Entry number.
 *     
 * w_r: 
 *     NAT Cache Memory and Statics Memory Transaction.1 : NAT Cache Memory and Statics Memory Write.0 : NAT Cache
 *     Memory and Statics Memory Read.
 *     
 *     Note that after setting NAT_CACHE.NATC_BLOCK.NATCACHE_INDIR_RW.NATC_INDIR_ADDR_REG.w_r = 1
 *     for the write, the FW needs to set this field to 0x0!
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_set(uint16_t natc_entry, bdmf_boolean w_r);
bdmf_error_t ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_get(uint16_t *natc_entry, bdmf_boolean *w_r);

/**********************************************************************************************************************
 * flow_cntr_entry: 
 *     Flow counter address.
 *     
 * w_r: 
 *     Flow counter Transaction.1 : Flow counter write.0 : Flow counter read.
 *     
 *     Note that after setting NAT_CACHE.NATC_BLOCK.NATCACHE_INDIR_RW.NATC_FLOW_CNTR_INDIR_ADDR_REG.w_r = 1
 *     for the write, the FW needs to set this field to 0x0!
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_set(uint8_t flow_cntr_entry, bdmf_boolean w_r);
bdmf_error_t ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_get(uint8_t *flow_cntr_entry, bdmf_boolean *w_r);

/**********************************************************************************************************************
 * data: 
 *     Flow Counter indirect register access data register,
 *     bits[31:0].-------------------------------------------------------For Flow Counter write operation,first, write
 *     all the data to Flow Counter Indirect Data Registers[1:0],Flow Counter Indirect Data register[0] is for flow
 *     Counter hit count and 4 lsb of Flow Counter byte count.Flow Counter Indirect Data register[0] bits 27:0 are
 *     28-bit Flow Counter hit count.Flow Counter Indirect Data register[0] bits 31:28 are 4 lsb of 36-bit Flow
 *     Counter byte count.Flow Counter Indirect Data Register[1] is for 32 msb of 36-bit Flow Counter byte count.{Flow
 *     Counter Indirect Data Register[1], Flow Counter Indirect Data register[0][31:28]} is the 36-bit Flow Counter
 *     byte count.then followed by a write to Flow Counter Indirect Address Register to set upFlow Counter adress and
 *     W_R bit to 1, this will initiate the write
 *     operation.--------------------------------------------------------For Flow Counter read operation,first, write
 *     to Flow Counter Indirect Address Register to set upFlow Counter address and W_R bit to 0, this will initiate
 *     the read operation.the read data from Flow Counter will be loaded into Flow Counter Indirect Data
 *     Registers[1:0].then followed by read from Flow Counter Indirect Data Registers[1:0] for all data.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set(uint32_t index, uint32_t data);
bdmf_error_t ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_get(uint32_t index, uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_nat_cache_ctrl_status,
    cli_nat_cache_natc_ctrs,
    cli_nat_cache_natc_table_control,
    cli_nat_cache_nat0_command_status,
    cli_nat_cache_nat0_hash,
    cli_nat_cache_nat0_hit_count,
    cli_nat_cache_nat0_byte_count,
    cli_nat_cache_nat0_pkt_len,
    cli_nat_cache_nat0_key_result,
    cli_nat_cache_natc_tbl0_ddr_key_base_address_lower,
    cli_nat_cache_natc_tbl0_ddr_key_base_address_upper,
    cli_nat_cache_natc_tbl0_ddr_result_base_address_lower,
    cli_nat_cache_natc_tbl0_ddr_result_base_address_upper,
    cli_nat_cache_natc_tbl1_ddr_key_base_address_lower,
    cli_nat_cache_natc_tbl1_ddr_key_base_address_upper,
    cli_nat_cache_natc_tbl1_ddr_result_base_address_lower,
    cli_nat_cache_natc_tbl1_ddr_result_base_address_upper,
    cli_nat_cache_natc_tbl2_ddr_key_base_address_lower,
    cli_nat_cache_natc_tbl2_ddr_key_base_address_upper,
    cli_nat_cache_natc_tbl2_ddr_result_base_address_lower,
    cli_nat_cache_natc_tbl2_ddr_result_base_address_upper,
    cli_nat_cache_natc_tbl3_ddr_key_base_address_lower,
    cli_nat_cache_natc_tbl3_ddr_key_base_address_upper,
    cli_nat_cache_natc_tbl3_ddr_result_base_address_lower,
    cli_nat_cache_natc_tbl3_ddr_result_base_address_upper,
    cli_nat_cache_natc_tbl4_ddr_key_base_address_lower,
    cli_nat_cache_natc_tbl4_ddr_key_base_address_upper,
    cli_nat_cache_natc_tbl4_ddr_result_base_address_lower,
    cli_nat_cache_natc_tbl4_ddr_result_base_address_upper,
    cli_nat_cache_natc_tbl5_ddr_key_base_address_lower,
    cli_nat_cache_natc_tbl5_ddr_key_base_address_upper,
    cli_nat_cache_natc_tbl5_ddr_result_base_address_lower,
    cli_nat_cache_natc_tbl5_ddr_result_base_address_upper,
    cli_nat_cache_natc_tbl6_ddr_key_base_address_lower,
    cli_nat_cache_natc_tbl6_ddr_key_base_address_upper,
    cli_nat_cache_natc_tbl6_ddr_result_base_address_lower,
    cli_nat_cache_natc_tbl6_ddr_result_base_address_upper,
    cli_nat_cache_natc_tbl7_ddr_key_base_address_lower,
    cli_nat_cache_natc_tbl7_ddr_key_base_address_upper,
    cli_nat_cache_natc_tbl7_ddr_result_base_address_lower,
    cli_nat_cache_natc_tbl7_ddr_result_base_address_upper,
    cli_nat_cache_natc_stat_counter_control_0,
    cli_nat_cache_natc_stat_counter_control_1,
    cli_nat_cache_natc_ddr_size,
    cli_nat_cache_natc_ddr_bins_per_bucket_0,
    cli_nat_cache_natc_ddr_bins_per_bucket_1,
    cli_nat_cache_natc_total_len,
    cli_nat_cache_natc_sm_status,
    cli_nat_cache_natc_regfile_fifo_start_addr_0,
    cli_nat_cache_natc_regfile_fifo_start_addr_1,
    cli_nat_cache_natc_tbl0_key_mask,
    cli_nat_cache_natc_tbl1_key_mask,
    cli_nat_cache_natc_tbl2_key_mask,
    cli_nat_cache_natc_tbl3_key_mask,
    cli_nat_cache_natc_tbl4_key_mask,
    cli_nat_cache_natc_tbl5_key_mask,
    cli_nat_cache_natc_tbl6_key_mask,
    cli_nat_cache_natc_tbl7_key_mask,
    cli_nat_cache_natc_ddr_hash_mode,
    cli_nat_cache_natc_flow_cntr_cntl,
    cli_nat_cache_indir_rw_natc_indir_addr_reg,
    cli_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg,
    cli_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg,
};

int bcm_nat_cache_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_nat_cache_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
