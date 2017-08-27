/*
    <:copyright-BRCM:2014:DUAL/GPL:standard
    
       Copyright (c) 2014 Broadcom 
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
#ifndef __RDP_NATCACHE_H_
#define __RDP_NATCACHE_H_

#include "rdp_map.h"


#define DEFAULT_AGE_TIMER_32         5
#define DEFAULT_DDR_BINS_PER_BACKET  7
#define DEFAULT_MAX_CACHE_ENTIRES    18

#define NATCACHE_RDP_CONTROL_STATUS                 (NATCACHE_RDP + 0x0)
#define NATCACHE_RDP_CONTROL_STATUS2                (NATCACHE_RDP + 0x4)
#define NATCACHE_RDP_NAT0_KEY_RESULT_0_17           (NATCACHE_RDP + 0x30)
#define NATCACHE_RDP_NAT1_KEY_RESULT_0_17           (NATCACHE_RDP + 0xD0)
#define NATCACHE_RDP_NAT2_KEY_RESULT_0_17           (NATCACHE_RDP + 0x170)
#define NATCACHE_RDP_NAT3_KEY_RESULT_0_17           (NATCACHE_RDP + 0x210)
#define NATCACHE_RDP_NAT0_COMMAND_STATUS            (NATCACHE_RDP + 0x10)
#define NATCACHE_RDP_NAT1_COMMAND_STATUS            (NATCACHE_RDP + 0xB0)
#define NATCACHE_RDP_NAT2_COMMAND_STATUS            (NATCACHE_RDP + 0x150)
#define NATCACHE_RDP_NAT3_COMMAND_STATUS            (NATCACHE_RDP + 0x1f0)
#define NATCACHE_RDP_NAT0_HIT_COUNT                 (NATCACHE_RDP + 0x1C)
#define NATCACHE_RDP_NAT1_HIT_COUNT                 (NATCACHE_RDP + 0xBC)
#define NATCACHE_RDP_NAT2_HIT_COUNT                 (NATCACHE_RDP + 0x15C)
#define NATCACHE_RDP_NAT3_HIT_COUNT                 (NATCACHE_RDP + 0x1FC)
#define NATCACHE_RDP_NAT0_BYTE_COUNT                (NATCACHE_RDP + 0x20)
#define NATCACHE_RDP_NAT1_BYTE_COUNT                (NATCACHE_RDP + 0xC0)
#define NATCACHE_RDP_NAT2_BYTE_COUNT                (NATCACHE_RDP + 0x160)
#define NATCACHE_RDP_NAT3_BYTE_COUNT                (NATCACHE_RDP + 0x200)
#define NATCACHE_RDP_DDR_KEY_BASE_ADDRESS_LOWER     (NATCACHE_RDP + 0x290)
#define NATCACHE_RDP_DDR_RESULT_BASE_ADDRESS_LOWER  (NATCACHE_RDP + 0x298)
#define NATCACHE_RDP_NAT0_KEY_MASK                  (NATCACHE_RDP + 0x14)
#define NATCACHE_RDP_NAT1_KEY_MASK                  (NATCACHE_RDP + 0xB4)
#define NATCACHE_RDP_NAT2_KEY_MASK                  (NATCACHE_RDP + 0x154)
#define NATCACHE_RDP_NAT3_KEY_MASK                  (NATCACHE_RDP + 0x1f4)
#define NATCACHE_RDP_INDIRECT_ADDRESS               (NATCACHE_RDP + 0x400)
#define NATCACHE_RDP_INDIRECT_DATA                  (NATCACHE_RDP + 0x410)


/* NATCACHE_NATC_CONTROL_STATUS  Fields offset */
#define NATCACHE_RDP_CONTROL_STATUS_DDR_ENABLE                             (1<<31)
#define NATCACHE_RDP_CONTROL_STATUS_ADD_COMMAND_SPEEDUP_MODE               (1<<30)
#define NATCACHE_RDP_CONTROL_STATUS_RUNNER_8BIT_RETURN_WORD0_SWAP_CONTROL  (1<<29)
#define NATCACHE_RDP_CONTROL_STATUS_RUNNER_32BIT_IN_64BIT_SWAP_CONTROL     (1<<28)
#define NATCACHE_RDP_CONTROL_STATUS_RUNNER_8BIT_IN_32BIT_SWAP_CONTROL      (1<<27)
#define NATCACHE_RDP_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL       (1<<26)
#define NATCACHE_RDP_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL        (1<<25)
#define NATCACHE_RDP_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL                   (1<<24)
#define NATCACHE_RDP_CONTROL_STATUS_RUNNER_SWAP_ALL_CONTROL                (1<<23)
#define NATCACHE_RDP_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL        (1<<22)
#define NATCACHE_RDP_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL         (1<<21)
#define NATCACHE_RDP_CONTROL_STATUS_CMD1_FIFO_OVERFLOW                     (1<<20)
#define NATCACHE_RDP_CONTROL_STATUS_CMD0_FIFO_OVERFLOW                     (1<<19)
#define NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_OFFSET                       (16)
#define NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_VAL_48_BYTE                  (0)
#define NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_VAL_64_BYTE                  (1)
#define NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_VAL_80_BYTE                  (2)
#define NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_VAL_96_BYTE                  (3)
#define NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_VAL_112_BYTE                 (4)
#define NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_VAL_128_BYTE                 (5)
#define NATCACHE_RDP_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP              (1<<15)
#define NATCACHE_RDP_CONTROL_STATUS_KEY_LEN_OFFSET                         (14)
#define NATCACHE_RDP_CONTROL_STATUS_KEY_LEN_VAL_16_BYTE                    (0)
#define NATCACHE_RDP_CONTROL_STATUS_KEY_LEN_VAL_32_BYTE                    (1)
#define NATCACHE_RDP_CONTROL_STATUS_NAT_RUNNER_CLUSTER_ARB_ST_OFFET        (12)
#define NATCACHE_RDP_CONTROL_STATUS_NAT_RUNNER_CLUSTER_ARB_ST_VAL_RR       (0)
#define NATCACHE_RDP_CONTROL_STATUS_NAT_RUNNER_CLUSTER_ARB_ST_VAL_STRICT   (1)
#define NATCACHE_RDP_CONTROL_STATUS_NAT_RUNNER_CLUSTER_ARB_ST_VAL_FIFO     (2)
#define NATCACHE_RDP_CONTROL_STATUS_MULTI_HASH_LIMIT_OFFSET                (8)
#define NATCACHE_RDP_CONTROL_STATUS_NAT_ARB_ST_OFFSET                      (5)
#define NATCACHE_RDP_CONTROL_STATUS_NAT_ARB_ST_VAL_RR                      (0)
#define NATCACHE_RDP_CONTROL_STATUS_NAT_ARB_ST_VAL_STRICT                  (1)
#define NATCACHE_RDP_CONTROL_STATUS_NAT_ARB_ST_VAL_STRICT_RR               (2)
#define NATCACHE_RDP_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP      (1<<4)
#define NATCACHE_RDP_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE      (1<<3)
#define NATCACHE_RDP_CONTROL_STATUS_NATC_SMEM_DISABLE                      (1<<2)
#define NATCACHE_RDP_CONTROL_STATUS_NATC_ENABLE                            (1<<1)
#define NATCACHE_RDP_CONTROL_STATUS_NATC_RESET                             (1<<0)


/* NATCACHE_NATC_CONTROL_STATUS2  Fields offset */
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_SWAP                               (1<<31)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_OFFSET                        (29)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_VAL_XOR                       (0)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_VAL_CRC32_16B_REDUCED         (1)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_VAL_CRC32_16B_LSB             (2)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_VAL_CRC32_16B_MSB             (2)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL             (1<<28)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL              (1<<27)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE                  (1<<26)
#define NATCACHE_RDP_CONTROL_STATUS2_AGE_TIMER_TICK                              (1<<25)
#define NATCACHE_RDP_CONTROL_STATUS2_AGE_TIMER_OFFSET                            (20)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_OFFSET                           (16)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_LRU_THEN_HIT_COUNT               (0)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_HIT_COUNT_THEN_LRU               (1)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_LRU                              (2)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_HIT_COUNT                        (3)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_LRU_THEN_RANDOM                  (4)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_HIT_COUNT_THEN_RANDOM            (5)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_RANDOM                           (6)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_HIGHEST_HIT_COUNT_THEN_LRU       (7)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_LRU_THEN_BYTE_COUNT              (8)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_BYTE_COUNT_THEN_LRU              (9)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_BYTE_COUNT                       (10)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_BYTE_COUNT_THEN_RANDOM           (11)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_ALGO_HIGHEST_BYTE_COUNT_THEN_LRU      (12)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_BINS_PER_BUCKET_OFFSET                  (8)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_OFFSET                             (06)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_VAL_8K                             (0)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_VAL_16K                            (1)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_VAL_32K                            (2)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_VAL_64K                            (3)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP              (1<<5)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL      (1<<4)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_WRITE_W_ACK_ENABLE                      (1<<3)
#define NATCACHE_RDP_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE  (1<<2)
#define NATCACHE_RDP_CONTROL_STATUS2_CACHE_DISABLE                               (1<<1)
#define NATCACHE_RDP_CONTROL_STATUS2_EVICTION_DISABLE                            (1<<0)

#endif
