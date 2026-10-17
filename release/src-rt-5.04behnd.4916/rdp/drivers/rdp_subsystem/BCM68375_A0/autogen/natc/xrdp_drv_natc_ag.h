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


#ifndef _XRDP_DRV_NATC_AG_H_
#define _XRDP_DRV_NATC_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean ddr_enable;
    bdmf_boolean natc_add_command_speedup_mode;
    bdmf_boolean unused0;
    bdmf_boolean natc_init_done;
    bdmf_boolean ddr_64bit_in_128bit_swap_control;
    bdmf_boolean smem_32bit_in_64bit_swap_control;
    bdmf_boolean smem_8bit_in_32bit_swap_control;
    bdmf_boolean ddr_swap_all_control;
    bdmf_boolean repeated_key_det_en;
    bdmf_boolean reg_32bit_in_64bit_swap_control;
    bdmf_boolean reg_8bit_in_32bit_swap_control;
    uint8_t ddr_pending_hash_mode;
    bdmf_boolean pending_fifo_entry_check_enable;
    bdmf_boolean cache_update_on_ddr_miss;
    bdmf_boolean ddr_disable_on_reg_lookup;
    uint8_t nat_hash_mode;
    uint8_t multi_hash_limit;
    bdmf_boolean decr_count_wraparound_enable;
    uint8_t nat_arb_st;
    bdmf_boolean natc_smem_increment_on_reg_lookup;
    bdmf_boolean natc_smem_clear_by_update_disable;
    bdmf_boolean regfile_fifo_reset;
    bdmf_boolean natc_enable;
    bdmf_boolean natc_reset;
    uint8_t unused3;
    bdmf_boolean ddr_32bit_in_64bit_swap_control;
    bdmf_boolean ddr_8bit_in_32bit_swap_control;
    bdmf_boolean cache_lookup_blocking_mode;
    bdmf_boolean age_timer_tick;
    uint8_t age_timer;
    uint8_t cache_algo;
    uint8_t unused2;
    uint8_t unused1;
    bdmf_boolean cache_update_on_reg_ddr_lookup;
    bdmf_boolean ddr_counter_8bit_in_32bit_swap_control;
    bdmf_boolean ddr_hash_swap;
    bdmf_boolean ddr_replace_duplicated_cached_entry_enable;
    bdmf_boolean ddr_lookup_pending_fifo_mode_disable;
    bdmf_boolean unused4;
} natc_ctrl_status;

typedef struct
{
    bdmf_boolean smem_dis_tbl7;
    bdmf_boolean smem_dis_tbl6;
    bdmf_boolean smem_dis_tbl5;
    bdmf_boolean smem_dis_tbl4;
    bdmf_boolean smem_dis_tbl3;
    bdmf_boolean smem_dis_tbl2;
    bdmf_boolean smem_dis_tbl1;
    bdmf_boolean smem_dis_tbl0;
    bdmf_boolean var_context_len_en_tbl7;
    bdmf_boolean var_context_len_en_tbl6;
    bdmf_boolean var_context_len_en_tbl5;
    bdmf_boolean var_context_len_en_tbl4;
    bdmf_boolean var_context_len_en_tbl3;
    bdmf_boolean var_context_len_en_tbl2;
    bdmf_boolean var_context_len_en_tbl1;
    bdmf_boolean var_context_len_en_tbl0;
    bdmf_boolean key_len_tbl7;
    bdmf_boolean non_cacheable_tbl7;
    bdmf_boolean key_len_tbl6;
    bdmf_boolean non_cacheable_tbl6;
    bdmf_boolean key_len_tbl5;
    bdmf_boolean non_cacheable_tbl5;
    bdmf_boolean key_len_tbl4;
    bdmf_boolean non_cacheable_tbl4;
    bdmf_boolean key_len_tbl3;
    bdmf_boolean non_cacheable_tbl3;
    bdmf_boolean key_len_tbl2;
    bdmf_boolean non_cacheable_tbl2;
    bdmf_boolean key_len_tbl1;
    bdmf_boolean non_cacheable_tbl1;
    bdmf_boolean key_len_tbl0;
    bdmf_boolean non_cacheable_tbl0;
} natc_table_control;

typedef struct
{
    bdmf_boolean flow_cntr_clr_on_rd_en;
    bdmf_boolean flow_cntr_en_tbl7;
    bdmf_boolean flow_cntr_en_tbl6;
    bdmf_boolean flow_cntr_en_tbl5;
    bdmf_boolean flow_cntr_en_tbl4;
    bdmf_boolean flow_cntr_en_tbl3;
    bdmf_boolean flow_cntr_en_tbl2;
    bdmf_boolean flow_cntr_en_tbl1;
    bdmf_boolean flow_cntr_en_tbl0;
    uint8_t context_offset;
} natc_flow_cntr_cntl;


/**********************************************************************************************************************
 * ddr_enable: 
 *     Enables NAT table offload to DDR functionality.
 *     NATC_CONTROL_STATUS2 register should be configured before enabling this feature.
 * natc_add_command_speedup_mode: 
 *     Default behavior for an ADD command is to do a LOOKUP first to see if the entry
 *     with the same key already exists and replace it; this is to avoid having duplicated
 *     entries in the table for ADD command.  When this bit is set an ADD command will
 *     either replace the entry with the matched key or add an entry to an empty entry
 *     depending on whichever one is encountered first during multi-hash.  Enabling
 *     this bit speeds up the ADD command.
 * unused0: 
 * natc_init_done: 
 *     This bit is set to 1 when NATC cache memories have been initialized to 0's.
 * ddr_64bit_in_128bit_swap_control: 
 *     Swap 64-bit word within 128-bit word for DDR memory read/write accesses
 *     (i.e., [127:0] becomes {[63:0], [127:64]}).
 * smem_32bit_in_64bit_swap_control: 
 *     Swap 32-bit word within 64-bit word for statistics (counter) memory accesses
 *     (i.e., [63:0] becomes {[31:0], [63:32]})
 * smem_8bit_in_32bit_swap_control: 
 *     Reverse bytes within 32-bit word for statistics (counter) memory accesses
 *     (i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})
 * ddr_swap_all_control: 
 *     Swap all bytes on DDR interface.
 * repeated_key_det_en: 
 *     Enable repeated key detection to improve cache lookup performance for repeated key.
 * reg_32bit_in_64bit_swap_control: 
 *     Swap 32-bit word within 64-bit word for key_result register accesses
 *     (i.e., [63:0] becomes {[31:0], [63:32]})
 * reg_8bit_in_32bit_swap_control: 
 *     Reverse bytes within 32-bit word for key_result register accesses
 *     (i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})
 * ddr_pending_hash_mode: 
 *     Hash algorithm used to detect DDR pending operations.
 *     0h: 32-bit rolling XOR hash is used as cache hash function.
 *     1h: CRC32 hash is used as cache hash function.
 *     2h: CRC32 hash is used as cache hash function.
 *     3h: CRC32 hash is used as cache hash function.
 *     4h: RSS hash is used as cache hash function using secret key 0.
 *     5h: RSS hash is used as cache hash function using secret key 1.
 *     6h: RSS hash is used as cache hash function using secret key 2.
 *     7h: RSS hash is used as cache hash function using secret key 3.
 * pending_fifo_entry_check_enable: 
 *     This bit disables caching DDR miss entry function when there is no additional lookup having
 *     the same key in pending fifo (e.g., DDR miss entry is cached only if there are 2 or more lookup
 *     of the same key within a 32 lookup window).
 *     This is to reduce excessive caching of miss entries.
 *     This bit is only valid when CACHE_UPDATE_ON_DDR_MISS bit is set to 1.
 *     1h: Enable; miss entry fetched from DDR will be cached if pending FIFO
 *     contains the same lookup having the same hash value as miss entry.
 *     0h: Disable; miss entry fetched from DDR will always be cached.
 * cache_update_on_ddr_miss: 
 *     This bit enables caching for miss entry
 *     1h: Enable; miss entry in both cache and DDR will be cached.
 *     0h: Disable; miss entry in both cache and DDR will be not cached.
 * ddr_disable_on_reg_lookup: 
 *     This bit prevents register interface lookup to access DDR
 *     0h: Enable register interface lookup in DDR.
 *     1h: Disable register interface lookup in DDR.
 *     Register interface lookup will only return lookup results in Cache.
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
 * multi_hash_limit: 
 *     Maximum number of multi-hash iterations.
 *     This is not used if cache size is 32 cache entries or less.
 *     Value of 0 is 1 iteration, 1 is 2 iterations, 2 is 3 iterations, etc.
 *     This is not used if cache size is 32 entries or less.
 * decr_count_wraparound_enable: 
 *     Decrement Count Wraparound Enable
 *     0h: Do not decrement counters for decrement command when counters reach 0
 *     1h: Always decrement counters for decrement command; will wrap around from 0 to all 1's
 * nat_arb_st: 
 *     NAT Arbitration Mechanism
 * natc_smem_increment_on_reg_lookup: 
 *     Enables incrementing or decrementing hit counter by 1 and byte counter by PKT_LEN
 *     on successful lookups using register interface
 *     BY default, counters only increment on successful lookups on Runner interface
 * natc_smem_clear_by_update_disable: 
 *     Disables clearing counters when an existing entry is replaced by ADD command
 * regfile_fifo_reset: 
 *     Reset regfile_FIFO and ddr pending memory.
 * natc_enable: 
 *     Enables all NATC state machines and input FIFO;
 *     Clearing this bit will halt all state machines gracefully to idle states,
 *     all outstanding transactions in the FIFO will remain in the FIFO and NATC
 *     will stop accepting new commands;  All configuration registers should be
 *     configured before enabling this bit.
 * natc_reset: 
 *     Self Clearing Block Reset (including resetting all registers to default values)
 * unused3: 
 * ddr_32bit_in_64bit_swap_control: 
 *     Swap 32-bit word within 64-bit word for DDR memory read/write accesses
 *     (i.e., [63:0] becomes {[31:0], [63:32]}).
 * ddr_8bit_in_32bit_swap_control: 
 *     Reverse bytes within 32-bit word for DDR memory read/write accesses
 *     (i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]}).
 * cache_lookup_blocking_mode: 
 *     (debug command) Do not set this bit to 1
 * age_timer_tick: 
 *     Timer tick for pseudo-LRU
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
 * cache_algo: 
 *     Replacement algorithm for caching
 *     Lowest-multi-hash-iteration number is used to select the final replacement
 *     entry if multiple entries were chosen by the selected algorithm.  For
 *     instance, if HIT_COUNT algorithm were selected, and 2nd, 3rd and 7th
 *     entry all have the same hit_count values, 2nd entry will be evicted.
 * unused2: 
 * unused1: 
 * cache_update_on_reg_ddr_lookup: 
 *     This bit determines whether register interface lookup will cache the entry from DDR
 *     1h: Enable; entry fetched from DDR will be cached using register interface lookup command
 *     0h: Disable; entry fetched from DDR will not be cached using register interface lookup command
 * ddr_counter_8bit_in_32bit_swap_control: 
 *     Reverse bytes within 32-bit word for DDR counters on read/write accesses.
 *     (i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})
 * ddr_hash_swap: 
 *     Reverse bytes within 18-bit DDR hash value
 * ddr_replace_duplicated_cached_entry_enable: 
 *     (debug command) Do not set this bit to 1
 * ddr_lookup_pending_fifo_mode_disable: 
 *     (debug command) Do not set this bit to 1
 * unused4: 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_ctrl_status_set(const natc_ctrl_status *ctrl_status);
bdmf_error_t ag_drv_natc_ctrl_status_get(natc_ctrl_status *ctrl_status);

/**********************************************************************************************************************
 * smem_dis_tbl7: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 7
 * smem_dis_tbl6: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 6
 * smem_dis_tbl5: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 5
 * smem_dis_tbl4: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 4
 * smem_dis_tbl3: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 3
 * smem_dis_tbl2: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 2
 * smem_dis_tbl1: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 1
 * smem_dis_tbl0: 
 *     Disables cache counters, DDR counters update and eviction for DDR table 0
 * var_context_len_en_tbl7: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 7
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 * var_context_len_en_tbl6: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 6
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 * var_context_len_en_tbl5: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 5
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 * var_context_len_en_tbl4: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 4
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 * var_context_len_en_tbl3: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 3
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 * var_context_len_en_tbl2: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 2
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 * var_context_len_en_tbl1: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 1
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 * var_context_len_en_tbl0: 
 *     Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 0
 *     lowest 4 bits of key[3:0] is used to indicate the context length
 *     0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes
 *     Note that key length is reduced by 4 bit
 *     0h: Disable variable context length
 *     1h: Enable variable context length
 * key_len_tbl7: 
 *     Length of the key for DDR table 7
 *     0h: 16-byte key
 *     1h: 32-byte key
 * non_cacheable_tbl7: 
 *     DDR table 7 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 * key_len_tbl6: 
 *     Length of the key for DDR table 6
 *     0h: 16-byte key
 *     1h: 32-byte key
 * non_cacheable_tbl6: 
 *     DDR table 6 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 * key_len_tbl5: 
 *     Length of the key for DDR table 5
 *     0h: 16-byte key
 *     1h: 32-byte key
 * non_cacheable_tbl5: 
 *     DDR table 5 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 * key_len_tbl4: 
 *     Length of the key for DDR table 4
 *     0h: 16-byte key
 *     1h: 32-byte key
 * non_cacheable_tbl4: 
 *     DDR table 4 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 * key_len_tbl3: 
 *     Length of the key for DDR table 3
 *     0h: 16-byte key
 *     1h: 32-byte key
 * non_cacheable_tbl3: 
 *     DDR table 3 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 * key_len_tbl2: 
 *     Length of the key for DDR table 2
 *     0h: 16-byte key
 *     1h: 32-byte key
 * non_cacheable_tbl2: 
 *     DDR table 2 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 * key_len_tbl1: 
 *     Length of the key for DDR table 1
 *     0h: 16-byte key
 *     1h: 32-byte key
 * non_cacheable_tbl1: 
 *     DDR table 1 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 * key_len_tbl0: 
 *     Length of the key for DDR table 0
 *     0h: 16-byte key
 *     1h: 32-byte key
 * non_cacheable_tbl0: 
 *     DDR table 0 non-cacheable control
 *     0h: DDR table is cached
 *     1h: DDR table is not cached; counters are updated in DDR directly
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_table_control_set(const natc_table_control *table_control);
bdmf_error_t ag_drv_natc_table_control_get(natc_table_control *table_control);

/**********************************************************************************************************************
 * ddr_evict_count_en: 
 *     DDR evict counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable DDR evict counter increment on ddr evict.
 *     1h: Enable DDR evict counter increment on ddr evict.
 * ddr_request_count_en: 
 *     DDR request counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable DDR request counter increment on ddr request.
 *     1h: Enable DDR request counter increment on ddr request.
 * cache_miss_count_en: 
 *     Cache miss counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable cache miss counter increment on cache miss.
 *     1h: Enable cache miss counter increment on cache miss.
 * cache_hit_count_en: 
 *     Cache hit counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable cache hit counter increment on cache hit.
 *     1h: Enable cache hit counter increment on cache hit.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_stat_counter_control_0_set(uint8_t ddr_evict_count_en, uint8_t ddr_request_count_en, uint8_t cache_miss_count_en, uint8_t cache_hit_count_en);
bdmf_error_t ag_drv_natc_stat_counter_control_0_get(uint8_t *ddr_evict_count_en, uint8_t *ddr_request_count_en, uint8_t *cache_miss_count_en, uint8_t *cache_hit_count_en);

/**********************************************************************************************************************
 * counter_wraparound_dis: 
 *     Counter wraparound disable
 *     This applies to all stat counters defined in
 *     NATC_STAT_COUNTER_CONTROL_0 and  NATC_STAT_COUNTER_CONTROL_1.
 *     0h: The counter will wraparound to 0 after it reaches ffffffffh.
 *     1h: The counter will cap at ffffffffh.
 * ddr_block_count_en: 
 *     DDR block counter enable
 *     Each bit enables/disables the counter increment based on a DDR table,
 *     bit 7 for DDR table 7..... bit 0 for DDR table 0.
 *     0h: Disable DDR block counter increment on ddr block.
 *     1h: Enable DDR block counter increment on ddr block.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_stat_counter_control_1_set(bdmf_boolean counter_wraparound_dis, uint8_t ddr_block_count_en);
bdmf_error_t ag_drv_natc_stat_counter_control_1_get(bdmf_boolean *counter_wraparound_dis, uint8_t *ddr_block_count_en);

/**********************************************************************************************************************
 * regfile_fifo_start_addr_3: 
 *     REGFILE FIFO 2 Start Address
 * regfile_fifo_start_addr_2: 
 *     REGFILE FIFO 2 Start Address
 * regfile_fifo_start_addr_1: 
 *     REGFILE FIFO 1 Start Address
 * regfile_fifo_start_addr_0: 
 *     REGFILE FIFO 0 Start Address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_regfile_fifo_start_addr_0_set(uint8_t regfile_fifo_start_addr_3, uint8_t regfile_fifo_start_addr_2, uint8_t regfile_fifo_start_addr_1, uint8_t regfile_fifo_start_addr_0);
bdmf_error_t ag_drv_natc_regfile_fifo_start_addr_0_get(uint8_t *regfile_fifo_start_addr_3, uint8_t *regfile_fifo_start_addr_2, uint8_t *regfile_fifo_start_addr_1, uint8_t *regfile_fifo_start_addr_0);

/**********************************************************************************************************************
 * regfile_fifo_start_addr_7: 
 *     REGFILE FIFO 7 Start Address-- Note that this entry is not used
 * regfile_fifo_start_addr_6: 
 *     REGFILE FIFO 6 Start Address
 * regfile_fifo_start_addr_5: 
 *     REGFILE FIFO 5 Start Address
 * regfile_fifo_start_addr_4: 
 *     REGFILE FIFO 4 Start Address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_regfile_fifo_start_addr_1_set(uint8_t regfile_fifo_start_addr_7, uint8_t regfile_fifo_start_addr_6, uint8_t regfile_fifo_start_addr_5, uint8_t regfile_fifo_start_addr_4);
bdmf_error_t ag_drv_natc_regfile_fifo_start_addr_1_get(uint8_t *regfile_fifo_start_addr_7, uint8_t *regfile_fifo_start_addr_6, uint8_t *regfile_fifo_start_addr_5, uint8_t *regfile_fifo_start_addr_4);

/**********************************************************************************************************************
 * flow_cntr_clr_on_rd_en: 
 *     Enable flow counters clear on read.
 * flow_cntr_en_tbl7: 
 *     Enable flow counters for DDR table 7.
 *     See description of FLOW_CNTR_EN_TBL0.
 * flow_cntr_en_tbl6: 
 *     Enable flow counters for DDR table 6.
 *     See description of FLOW_CNTR_EN_TBL0.
 * flow_cntr_en_tbl5: 
 *     Enable flow counters for DDR table 5.
 *     See description of FLOW_CNTR_EN_TBL0.
 * flow_cntr_en_tbl4: 
 *     Enable flow counters for DDR table 4.
 *     See description of FLOW_CNTR_EN_TBL0.
 * flow_cntr_en_tbl3: 
 *     Enable flow counters for DDR table 3.
 *     See description of FLOW_CNTR_EN_TBL0.
 * flow_cntr_en_tbl2: 
 *     Enable flow counters for DDR table 2.
 *     See description of FLOW_CNTR_EN_TBL0.
 * flow_cntr_en_tbl1: 
 *     Enable flow counters for DDR table 1.
 *     See description of FLOW_CNTR_EN_TBL0.
 * flow_cntr_en_tbl0: 
 *     Enable flow counters for DDR table 0.
 *     If this bit is set to 1 and CONTEXT_OFFSET is less than context length - 8 (byte count and hit cout),
 *     the flow counter is enabled. Otherwsie the flow counter is disabled.
 * context_offset: 
 *     This is the byte offset of the context.
 *     Only bits 6:0 of this byte in the context are used as flow counter address.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_flow_cntr_cntl_set(const natc_flow_cntr_cntl *flow_cntr_cntl);
bdmf_error_t ag_drv_natc_flow_cntr_cntl_get(natc_flow_cntr_cntl *flow_cntr_cntl);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_ctrl_status,
    cli_natc_table_control,
    cli_natc_stat_counter_control_0,
    cli_natc_stat_counter_control_1,
    cli_natc_regfile_fifo_start_addr_0,
    cli_natc_regfile_fifo_start_addr_1,
    cli_natc_flow_cntr_cntl,
};

int bcm_natc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
