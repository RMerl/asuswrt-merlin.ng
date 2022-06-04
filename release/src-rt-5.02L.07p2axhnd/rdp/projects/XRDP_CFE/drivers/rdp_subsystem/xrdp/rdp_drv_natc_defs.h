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
#ifndef _RDP_DRV_NATC_DEFS_H_
#define _RDP_DRV_NATC_DEFS_H_

#define HASH_FUNCTION_INIT_VAL   0x4899b351
#define NATC_TABLE_KEY_16B       16

typedef enum age_timer_tick_e
{
    timer_tick_clock = 0,
    timer_tick_packet,
    timer_tick_def = timer_tick_clock
} age_timer_tick_t;

typedef enum age_timer_e
{
    age_timer_1_tick = 0,
    age_timer_2_ticks,
    age_timer_4_ticks,
    age_timer_8_ticks,
    age_timer_16_ticks,
    age_timer_32_ticks,
    age_timer_64_ticks,
    age_timer_128_ticks,
    age_timer_256_ticks,
    age_timer_512_ticks,
    age_timer_ticks_def = 15,
    age_timer_ticks_last=32
} age_timer_t;

typedef enum ddr_bins_per_bucket_e
{
    ddr_1_bin_per_bucket = 0,
    ddr_2_bins_per_bucket,
    ddr_3_bins_per_bucket,
    ddr_4_bins_per_bucket,
    ddr_5_bins_per_bucket,
    ddr_6_bins_per_bucket,
    ddr_7_bins_per_bucket,
    ddr_8_bins_per_bucket,
    ddr_9_bins_per_bucket,
    ddr_10_bins_per_bucket,
    bins_per_bucket_def=ddr_1_bin_per_bucket,
    bins_per_bucket_last=255
} ddr_bins_per_bucket_t;

typedef enum ddr_nat_table_size_e
{
    ddr_size_8k = 0,
    ddr_size_16k,
    ddr_size_32k,
    ddr_size_64k,
    ddr_size_def = ddr_size_8k
} ddr_nat_table_size_t;

typedef enum total_length_e
{
    total_len_48B = 0,
    total_len_64B,
    total_len_80B,
    total_len_96B,
    total_len_112B,
    total_len_128B,
#if defined(BCM63158)
    total_len_144B,
    total_len_160B,
#endif
    total_len_def = total_len_80B
} total_length_t;

typedef enum cache_algo_e
{
    lru_then_hit_count = 0,
    hit_count_then_lru,
    lru,
    hit_count,
    lru_then_random,
    hit_count_then_random,
    random_algo,
    highest_hit_count_then_lru,
    lru_then_byte_count,
    byte_count_then_lru,
    byte_count,
    byte_count_then_random,
    highest_byte_count_then_lru,
    cache_algo_def = lru_then_hit_count
} cache_algo_t;

typedef enum nat_arb_e
{
    nat_arb_rr = 0,
    nat_arb_strict,
    nat_arb_strict_reserved,
    nat_arb_def = nat_arb_rr
} nat_arb_t;

typedef enum key_len_e
{
    key_len_16B = 0,
    key_len_32B,
    key_len_def = key_len_16B
} key_len_t;

typedef struct tbl_control
{
    key_len_t key_len;
    uint8_t   non_cacheable;
    uint8_t   var_context_len_en;
} tbl_control_t;

typedef enum nat_command_e{
    natc_cmd_nop = 0,
    natc_cmd_lookup,
    natc_cmd_add,
    natc_cmd_del
}nat_command_t;

typedef enum hash_mode_e{
    hash_mode_rolling_xor = 0,
    hash_mode_crc32,
    hash_mode_crc32_low,
    hash_mode_crc32_high
}hash_mode_t;

typedef enum multi_hash_limit_e
{
    multi_hash_limit_def = 0xf
}multi_hash_limit_t;

#endif
