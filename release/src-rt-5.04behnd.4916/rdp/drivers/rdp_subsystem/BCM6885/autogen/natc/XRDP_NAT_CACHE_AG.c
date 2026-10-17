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


#include "XRDP_NAT_CACHE_AG.h"

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_CONTROL_STATUS, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_CONTROL_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_RESET *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_NATC_RESET_FIELD =
{
    "NATC_RESET",
#if RU_INCLUDE_DESC
    "",
    "Self Clearing Block Reset (including resetting all registers to default values)\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_RESET_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_RESET_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_ENABLE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_NATC_ENABLE_FIELD =
{
    "NATC_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enables all NATC state machines and input FIFO;\nClearing this bit will halt all state machines gracefully to idle states,\nall outstanding transactions in the FIFO will remain in the FIFO and NATC\nwill stop accepting new commands;  All configuration registers should be\nconfigured before enabling this bit.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_RESET *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD =
{
    "REGFILE_FIFO_RESET",
#if RU_INCLUDE_DESC
    "",
    "Reset regfile_FIFO and ddr pending memory.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_SMEM_CLEAR_BY_UPDATE_DISABLE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD =
{
    "NATC_SMEM_CLEAR_BY_UPDATE_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables clearing counters when an existing entry is replaced by ADD command\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_SMEM_INCREMENT_ON_REG_LOOKUP *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD =
{
    "NATC_SMEM_INCREMENT_ON_REG_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "Enables incrementing or decrementing hit counter by 1 and byte counter by PKT_LEN\non successful lookups using register interface\nBY default, counters only increment on successful lookups on Runner interface\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_ARB_ST *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD =
{
    "NAT_ARB_ST",
#if RU_INCLUDE_DESC
    "",
    "NAT Arbitration Mechanism\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DECR_COUNT_WRAPAROUND_ENABLE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD =
{
    "DECR_COUNT_WRAPAROUND_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Decrement Count Wraparound Enable\n0h: Do not decrement counters for decrement command when counters reach 0\n1h: Always decrement counters for decrement command; will wrap around from 0 to all 1's\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_HASH_LIMIT *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD =
{
    "MULTI_HASH_LIMIT",
#if RU_INCLUDE_DESC
    "",
    "Maximum number of multi-hash iterations.\nThis is not used if cache size is 32 cache entries or less.\nValue of 0 is 1 iteration, 1 is 2 iterations, 2 is 3 iterations, etc.\nThis is not used if cache size is 32 entries or less.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_SHIFT },
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_HASH_MODE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD =
{
    "NAT_HASH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for internal caching\n0h: 32-bit rolling XOR hash is used as cache hash function.\n1h: CRC32 hash is used as cache hash function. CRC32 is reduced to N-bit using\nthe same method as in 32-bit rolling XOR hash.\n2h: CRC32 hash is used as cache hash function. CRC32[N:0] is used as hash value.\n3h: CRC32 hash is used as cache hash function. CRC32[31:N] is used as hash value.\n4h: RSS hash is used as cache hash function using secret key 0. RSS[N:0] is used as hash value.\n5h: RSS hash is used as cache hash function using secret key 1. RSS[N:0] is used as hash value.\n6h: RSS hash is used as cache hash function using secret key 2. RSS[N:0] is used as hash value.\n7h: RSS hash is used as cache hash function using secret key 3. RSS[N:0] is used as hash value.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_DISABLE_ON_REG_LOOKUP *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD =
{
    "DDR_DISABLE_ON_REG_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "This bit prevents register interface lookup to access DDR\n0h: Enable register interface lookup in DDR.\n1h: Disable register interface lookup in DDR.\nRegister interface lookup will only return lookup results in Cache.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_UPDATE_ON_DDR_MISS *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD =
{
    "CACHE_UPDATE_ON_DDR_MISS",
#if RU_INCLUDE_DESC
    "",
    "This bit enables caching for miss entry\n1h: Enable; miss entry in both cache and DDR will be cached.\n0h: Disable; miss entry in both cache and DDR will be not cached.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PENDING_FIFO_ENTRY_CHECK_ENABLE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD =
{
    "PENDING_FIFO_ENTRY_CHECK_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "This bit disables caching DDR miss entry function when there is no additional lookup having\nthe same key in pending fifo (e.g., DDR miss entry is cached only if there are 2 or more lookup\nof the same key within a 32 lookup window).\nThis is to reduce excessive caching of miss entries.\nThis bit is only valid when CACHE_UPDATE_ON_DDR_MISS bit is set to 1.\n1h: Enable; miss entry fetched from DDR will be cached if pending FIFO\ncontains the same lookup having the same hash value as miss entry.\n0h: Disable; miss entry fetched from DDR will always be cached.\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED1 *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_UNUSED1_FIELD =
{
    "UNUSED1",
#if RU_INCLUDE_DESC
    "",
    "Unused field since BCM68880B0/BCM68370A0.\n\nIn past those bits were used as Hash algorithm used to detect DDR pending operations with default =1h: CRC32 hash is used as cache hash function.\nNow design has hard coded CRC32 hash mode used for pending_tables (not cache or ddr).\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_UNUSED1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_UNUSED1_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_UNUSED1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REG_8BIT_IN_32BIT_SWAP_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "REG_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for key_result register accesses\n(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REG_32BIT_IN_64BIT_SWAP_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "REG_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for key_result register accesses\n(i.e., [63:0] becomes {[31:0], [63:32]})\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REPEATED_KEY_DET_EN *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD =
{
    "REPEATED_KEY_DET_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable repeated key detection to improve cache lookup performance for repeated key.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SWAP_ALL_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD =
{
    "DDR_SWAP_ALL_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap all bytes on DDR interface.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_8BIT_IN_32BIT_SWAP_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "SMEM_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for statistics (counter) memory accesses\n(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_32BIT_IN_64BIT_SWAP_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "SMEM_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for statistics (counter) memory accesses\n(i.e., [63:0] becomes {[31:0], [63:32]})\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_64BIT_IN_128BIT_SWAP_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD =
{
    "DDR_64BIT_IN_128BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 64-bit word within 128-bit word for DDR memory read/write accesses\n(i.e., [127:0] becomes {[63:0], [127:64]}).\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_INIT_DONE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD =
{
    "NATC_INIT_DONE",
#if RU_INCLUDE_DESC
    "",
    "This bit is set to 1 when NATC cache memories have been initialized to 0's.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED0 *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_UNUSED0_FIELD =
{
    "UNUSED0",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_UNUSED0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_UNUSED0_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_UNUSED0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_ADD_COMMAND_SPEEDUP_MODE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD =
{
    "NATC_ADD_COMMAND_SPEEDUP_MODE",
#if RU_INCLUDE_DESC
    "",
    "Default behavior for an ADD command is to do a LOOKUP first to see if the entry\nwith the same key already exists and replace it; this is to avoid having duplicated\nentries in the table for ADD command.  When this bit is set an ADD command will\neither replace the entry with the matched key or add an entry to an empty entry\ndepending on whichever one is encountered first during multi-hash.  Enabling\nthis bit speeds up the ADD command.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_ENABLE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS_DDR_ENABLE_FIELD =
{
    "DDR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enables NAT table offload to DDR functionality.\nNATC_CONTROL_STATUS2 register should be configured before enabling this feature.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_CONTROL_STATUS_FIELDS[] =
{
    &NAT_CACHE_NATC_CONTROL_STATUS_NATC_RESET_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_NATC_ENABLE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_UNUSED1_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_UNUSED0_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS_DDR_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_CONTROL_STATUS *****/
const ru_reg_rec NAT_CACHE_NATC_CONTROL_STATUS_REG =
{
    "NATC_CONTROL_STATUS",
#if RU_INCLUDE_DESC
    "NATC_CONTROL_STATUS Register",
    "NAT Cache Control and Status Register.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS_REG_OFFSET },
    0,
    0,
    1592,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    24,
    NAT_CACHE_NATC_CONTROL_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_CONTROL_STATUS2, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_CONTROL_STATUS2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED2 *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED2_FIELD =
{
    "UNUSED2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED2_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD =
{
    "DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD =
{
    "DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_SWAP *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD =
{
    "DDR_HASH_SWAP",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 18-bit DDR hash value\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for DDR counters on read/write accesses.\n(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_UPDATE_ON_REG_DDR_LOOKUP *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD =
{
    "CACHE_UPDATE_ON_REG_DDR_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "This bit determines whether register interface lookup will cache the entry from DDR\n1h: Enable; entry fetched from DDR will be cached using register interface lookup command\n0h: Disable; entry fetched from DDR will not be cached using register interface lookup command\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED3 *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED3_FIELD =
{
    "UNUSED3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED3_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED4 *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED4_FIELD =
{
    "UNUSED4",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED4_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_ALGO *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD =
{
    "CACHE_ALGO",
#if RU_INCLUDE_DESC
    "",
    "Replacement algorithm for caching\nLowest-multi-hash-iteration number is used to select the final replacement\nentry if multiple entries were chosen by the selected algorithm.  For\ninstance, if HIT_COUNT algorithm were selected, and 2nd, 3rd and 7th\nentry all have the same hit_count values, 2nd entry will be evicted.\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGE_TIMER *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_FIELD =
{
    "AGE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timer value used for pseudo-LRU;\nWhen timer fires the 8-bit age value of every entry in the cache is\ndecremented (cap at 0).  The entry with lower value is\nthe older entry.  The default setting keeps track of ~0.26s age at\n~1ms resolution.\n0: 1 tick\n1: 2 ticks\n2: 4 ticks\n3: 8 ticks\n4: 16 ticks\n..\n..\n31: 2^31 TICKS\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_SHIFT },
    19,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGE_TIMER_TICK *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD =
{
    "AGE_TIMER_TICK",
#if RU_INCLUDE_DESC
    "",
    "Timer tick for pseudo-LRU\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_LOOKUP_BLOCKING_MODE *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD =
{
    "CACHE_LOOKUP_BLOCKING_MODE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_8BIT_IN_32BIT_SWAP_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "DDR_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for DDR memory read/write accesses\n(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]}).\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_32BIT_IN_64BIT_SWAP_CONTROL *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "DDR_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for DDR memory read/write accesses\n(i.e., [63:0] becomes {[31:0], [63:32]}).\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED5 *****/
const ru_field_rec NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED5_FIELD =
{
    "UNUSED5",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED5_FIELD_WIDTH },
    { NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_CONTROL_STATUS2_FIELDS[] =
{
    &NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED2_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED3_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED4_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NAT_CACHE_NATC_CONTROL_STATUS2_UNUSED5_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_CONTROL_STATUS2 *****/
const ru_reg_rec NAT_CACHE_NATC_CONTROL_STATUS2_REG =
{
    "NATC_CONTROL_STATUS2",
#if RU_INCLUDE_DESC
    "NATC_CONTROL_STATUS2 Register",
    "NAT Cache Control and Status Register\n\n",
#endif
    { NAT_CACHE_NATC_CONTROL_STATUS2_REG_OFFSET },
    0,
    0,
    1593,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    NAT_CACHE_NATC_CONTROL_STATUS2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TABLE_CONTROL, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TABLE_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD =
{
    "NON_CACHEABLE_TBL0",
#if RU_INCLUDE_DESC
    "",
    "DDR table 0 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD =
{
    "KEY_LEN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 0\n0h: 16-byte key\n1h: 32-byte key\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD =
{
    "NON_CACHEABLE_TBL1",
#if RU_INCLUDE_DESC
    "",
    "DDR table 1 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD =
{
    "KEY_LEN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 1\n0h: 16-byte key\n1h: 32-byte key\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD =
{
    "NON_CACHEABLE_TBL2",
#if RU_INCLUDE_DESC
    "",
    "DDR table 2 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD =
{
    "KEY_LEN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 2\n0h: 16-byte key\n1h: 32-byte key\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD =
{
    "NON_CACHEABLE_TBL3",
#if RU_INCLUDE_DESC
    "",
    "DDR table 3 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD =
{
    "KEY_LEN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 3\n0h: 16-byte key\n1h: 32-byte key\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD =
{
    "NON_CACHEABLE_TBL4",
#if RU_INCLUDE_DESC
    "",
    "DDR table 4 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD =
{
    "KEY_LEN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 4\n0h: 16-byte key\n1h: 32-byte key\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD =
{
    "NON_CACHEABLE_TBL5",
#if RU_INCLUDE_DESC
    "",
    "DDR table 5 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD =
{
    "KEY_LEN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 5\n0h: 16-byte key\n1h: 32-byte key\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD =
{
    "NON_CACHEABLE_TBL6",
#if RU_INCLUDE_DESC
    "",
    "DDR table 6 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD =
{
    "KEY_LEN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 6\n0h: 16-byte key\n1h: 32-byte key\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD =
{
    "NON_CACHEABLE_TBL7",
#if RU_INCLUDE_DESC
    "",
    "DDR table 7 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD =
{
    "KEY_LEN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 7\n0h: 16-byte key\n1h: 32-byte key\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 0\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 1\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 2\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 3\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 4\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 5\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 6\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 7\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD =
{
    "SMEM_DIS_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 0\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD =
{
    "SMEM_DIS_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 1\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD =
{
    "SMEM_DIS_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 2\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD =
{
    "SMEM_DIS_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 3\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD =
{
    "SMEM_DIS_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 4\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD =
{
    "SMEM_DIS_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 5\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD =
{
    "SMEM_DIS_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 6\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD =
{
    "SMEM_DIS_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 7\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TABLE_CONTROL_FIELDS[] =
{
    &NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD,
    &NAT_CACHE_NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TABLE_CONTROL *****/
const ru_reg_rec NAT_CACHE_NATC_TABLE_CONTROL_REG =
{
    "NATC_TABLE_CONTROL",
#if RU_INCLUDE_DESC
    "NATC_TABLE_CONTROL Register",
    "NAT Cache Table Control Register\n\n",
#endif
    { NAT_CACHE_NATC_TABLE_CONTROL_REG_OFFSET },
    0,
    0,
    1594,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    NAT_CACHE_NATC_TABLE_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NAT0_COMMAND_STATUS, TYPE: Type_NATC_BLOCK_NATCACHE_NAT0_COMMAND_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COMMAND *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_COMMAND_FIELD =
{
    "COMMAND",
#if RU_INCLUDE_DESC
    "",
    "Command to be executed\nThis command only operates on the entries in cache except LOOKUP command where\nentry can be fetched from DDR.\nWriting to this field causes BUSY bit to be set.\nNote: For all commands, key consists of all 0's indicates unused entry in h/w\nand therefore cannot be used.\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_COMMAND_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_COMMAND_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_COMMAND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED0 *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_UNUSED0_FIELD =
{
    "UNUSED0",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_UNUSED0_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_UNUSED0_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_UNUSED0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "Interface Busy\nThis bit is set when command is issued but still in progress been processed.\nWhen command completes this bit will be cleared.\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_BUSY_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_BUSY_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ERROR *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_ERROR_FIELD =
{
    "ERROR",
#if RU_INCLUDE_DESC
    "",
    "This bit is set for the following 2 cases\nFor ADD command all multi-hash entries are occupied (i.e, no room to ADD)\nFor DEL command entry is not found and cannot be deleted\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_ERROR_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_ERROR_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_ERROR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MISS *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_MISS_FIELD =
{
    "MISS",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when a LOOKUP command has a miss\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_MISS_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_MISS_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_MISS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_HIT *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_HIT_FIELD =
{
    "CACHE_HIT",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when a LOOKUP command has a cache hit\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_HIT_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_HIT_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_HIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTIHASH_COUNT *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_MULTIHASH_COUNT_FIELD =
{
    "MULTIHASH_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cache multi-hash iteration count status\nValue of 0 is iteration 1, 1 is iteration 2, 2 is iteration 3, etc.\ncache miss returns 0 count.\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_TBL *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_NAT_TBL_FIELD =
{
    "NAT_TBL",
#if RU_INCLUDE_DESC
    "",
    "Select the DDR Table on which the command will operate\n0h: DDR table 0\n1h: DDR table 1\n2h: DDR table 2\n3h: DDR table 3\n4h: DDR table 4\n5h: DDR table 5\n6h: DDR table 6\n7h: DDR table 7\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_NAT_TBL_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_NAT_TBL_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_NAT_TBL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DECR_COUNT *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_DECR_COUNT_FIELD =
{
    "DECR_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Decrement-counter mode enable\nWhen set, LOOKUP command will decrement hit counter by 1 and decrement\nbyte counter by the value specified in PKT_LEN, on a successful lookup.\nNATC_SMEM_INCREMENT_ON_REG_LOOKUP must be set to 1 for it to be effective\n0h: LOOKUP command will increment hit counter and byte counter\n1h: LOOKUP command will decrement hit counter and byte counter\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_DECR_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_DECR_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_DECR_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_FLUSH *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_FLUSH_FIELD =
{
    "CACHE_FLUSH",
#if RU_INCLUDE_DESC
    "",
    "Cache Flush enable\nWhen set, LOOKUP command is used to flush counters from cache into DDR.\nThis command does not use key to lookup the cache entry.  Instead it uses\ncache index number located in 10-MSB bits of key specified in NAT_KEY_RESULT register.\nFor 16 bytes key, the cache index will be located in\n{NAT_KEY_RESULT[15], NAT_KEY_RESULT[14][7:6]} (15th byte of NAT_KEY_RESULT register and\nbits 7:6 of 14th byte of NAT_KEY_RESULT register).\nFor 32 bytes key, the cache index will be located in\n{NAT_KEY_RESULT[31], NAT_KEY_RESULT[30][7:6]} (31th byte of NAT_KEY_RESULT register and\nbits 7:6 of 30th byte of NAT_KEY_RESULT register).\n0h: LOOKUP command is used as normal lookup command.\n1h: LOOKUP command is used as cache flush command.\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_FLUSH_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_FLUSH_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_FLUSH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEL_CMD_MODE *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_MODE_FIELD =
{
    "DEL_CMD_MODE",
#if RU_INCLUDE_DESC
    "",
    "DEL Command DDR-bin matching mode enable\n0h: DEL command deletes the cache entry with matching key\n1h: DEL command deletes the cache entry with matching key and matching DDR bin\nnumber specified in DEL_CMD_DDR_BIN field\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_MODE_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_MODE_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADD_CMD_MODE *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_MODE_FIELD =
{
    "ADD_CMD_MODE",
#if RU_INCLUDE_DESC
    "",
    "ADD Command mode\n0h: ADD command writes 0 (DDR bin number and DDR miss flag) to cache.\n1h: ADD command writes DDR bin number and DDR miss flag to cache\nspecified in ADD_CMD_DDR_BIN and ADD_CMD_DDR_MISS fields.\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_MODE_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_MODE_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_R1_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_R1_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEL_CMD_DDR_BIN *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD =
{
    "DEL_CMD_DDR_BIN",
#if RU_INCLUDE_DESC
    "",
    "This filed specifies the DDR BIN number to be compared for DEL command\nwhen DEL_CMD_MODE is set to 1\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADD_CMD_DDR_BIN *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD =
{
    "ADD_CMD_DDR_BIN",
#if RU_INCLUDE_DESC
    "",
    "This filed specifies the DDR BIN number to be written to cache for ADD command\nwhen ADD_CMD_MODE is set to 1.\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADD_CMD_DDR_MISS *****/
const ru_field_rec NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD =
{
    "ADD_CMD_DDR_MISS",
#if RU_INCLUDE_DESC
    "",
    "This filed specifies the DDR MISS flag to be written to cache for ADD command\nwhen ADD_CMD_MODE is set to 1.\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD_WIDTH },
    { NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NAT0_COMMAND_STATUS_FIELDS[] =
{
    &NAT_CACHE_NAT0_COMMAND_STATUS_COMMAND_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_UNUSED0_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_BUSY_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_ERROR_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_MISS_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_HIT_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_MULTIHASH_COUNT_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_NAT_TBL_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_DECR_COUNT_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_CACHE_FLUSH_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_MODE_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_MODE_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_R1_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD,
    &NAT_CACHE_NAT0_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NAT0_COMMAND_STATUS *****/
const ru_reg_rec NAT_CACHE_NAT0_COMMAND_STATUS_REG =
{
    "NAT0_COMMAND_STATUS",
#if RU_INCLUDE_DESC
    "NAT0_COMMAND_STATUS Register",
    "NAT Command and Status Register\n\n",
#endif
    { NAT_CACHE_NAT0_COMMAND_STATUS_REG_OFFSET },
    0,
    0,
    1595,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    NAT_CACHE_NAT0_COMMAND_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NAT0_HASH, TYPE: Type_NATC_BLOCK_NATCACHE_NAT0_HASH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH *****/
const ru_field_rec NAT_CACHE_NAT0_HASH_HASH_FIELD =
{
    "HASH",
#if RU_INCLUDE_DESC
    "",
    "hash value; only valid on a successful lookup/add/del command\nFor cache hit 10-bit hash value is returned.\nFor cache miss and DDR_ENABLE is 0, first hash value (10-bit) is returned.\nFor cache miss, DDR_ENABLE is 1 and DDR is a hit, 18-bit DDR hash value + DDR bin count is returned.\nFor cache miss, DDR_ENABLE is 1 and DDR is a miss, 18-bit DDR hash value is returned.\n\n",
#endif
    { NAT_CACHE_NAT0_HASH_HASH_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_HASH_HASH_FIELD_WIDTH },
    { NAT_CACHE_NAT0_HASH_HASH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NAT0_HASH_FIELDS[] =
{
    &NAT_CACHE_NAT0_HASH_HASH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NAT0_HASH *****/
const ru_reg_rec NAT_CACHE_NAT0_HASH_REG =
{
    "NAT0_HASH",
#if RU_INCLUDE_DESC
    "NAT0_HASH Register",
    "NAT Hash Value\n\n",
#endif
    { NAT_CACHE_NAT0_HASH_REG_OFFSET },
    0,
    0,
    1596,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NAT0_HASH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NAT0_HIT_COUNT, TYPE: Type_NATC_BLOCK_NATCACHE_NAT0_HIT_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HIT_COUNT *****/
const ru_field_rec NAT_CACHE_NAT0_HIT_COUNT_HIT_COUNT_FIELD =
{
    "HIT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "bits 27:0 are 28-bit hit count value.\nbits 31:28 are 4 lsb of 36-bit byte count value.\nonly valid on a successful lookup or delete command.\n\n",
#endif
    { NAT_CACHE_NAT0_HIT_COUNT_HIT_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_HIT_COUNT_HIT_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NAT0_HIT_COUNT_HIT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NAT0_HIT_COUNT_FIELDS[] =
{
    &NAT_CACHE_NAT0_HIT_COUNT_HIT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NAT0_HIT_COUNT *****/
const ru_reg_rec NAT_CACHE_NAT0_HIT_COUNT_REG =
{
    "NAT0_HIT_COUNT",
#if RU_INCLUDE_DESC
    "NAT0_HIT_COUNT Register",
    "Hit Count\n\n",
#endif
    { NAT_CACHE_NAT0_HIT_COUNT_REG_OFFSET },
    0,
    0,
    1597,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NAT0_HIT_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NAT0_BYTE_COUNT, TYPE: Type_NATC_BLOCK_NATCACHE_NAT0_BYTE_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYTE_COUNT *****/
const ru_field_rec NAT_CACHE_NAT0_BYTE_COUNT_BYTE_COUNT_FIELD =
{
    "BYTE_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit msb of 36-bit byte count value.\n{BYTE_COUNT, HIT_COUNT[31:28]} is the 36-bit byte count value.\nonly valid on a successful lookup or delete command\n\n",
#endif
    { NAT_CACHE_NAT0_BYTE_COUNT_BYTE_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_BYTE_COUNT_BYTE_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NAT0_BYTE_COUNT_BYTE_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NAT0_BYTE_COUNT_FIELDS[] =
{
    &NAT_CACHE_NAT0_BYTE_COUNT_BYTE_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NAT0_BYTE_COUNT *****/
const ru_reg_rec NAT_CACHE_NAT0_BYTE_COUNT_REG =
{
    "NAT0_BYTE_COUNT",
#if RU_INCLUDE_DESC
    "NAT0_BYTE_COUNT Register",
    "Byte Count\n\n",
#endif
    { NAT_CACHE_NAT0_BYTE_COUNT_REG_OFFSET },
    0,
    0,
    1598,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NAT0_BYTE_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NAT0_PKT_LEN, TYPE: Type_NATC_BLOCK_NATCACHE_NAT0_PKT_LEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PKT_LEN *****/
const ru_field_rec NAT_CACHE_NAT0_PKT_LEN_PKT_LEN_FIELD =
{
    "PKT_LEN",
#if RU_INCLUDE_DESC
    "",
    "16-bit packet length value used to increment or decrement byte counter\n\n",
#endif
    { NAT_CACHE_NAT0_PKT_LEN_PKT_LEN_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_PKT_LEN_PKT_LEN_FIELD_WIDTH },
    { NAT_CACHE_NAT0_PKT_LEN_PKT_LEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED *****/
const ru_field_rec NAT_CACHE_NAT0_PKT_LEN_UNUSED_FIELD =
{
    "UNUSED",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NAT0_PKT_LEN_UNUSED_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_PKT_LEN_UNUSED_FIELD_WIDTH },
    { NAT_CACHE_NAT0_PKT_LEN_UNUSED_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NAT0_PKT_LEN_FIELDS[] =
{
    &NAT_CACHE_NAT0_PKT_LEN_PKT_LEN_FIELD,
    &NAT_CACHE_NAT0_PKT_LEN_UNUSED_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NAT0_PKT_LEN *****/
const ru_reg_rec NAT_CACHE_NAT0_PKT_LEN_REG =
{
    "NAT0_PKT_LEN",
#if RU_INCLUDE_DESC
    "NAT0_PKT_LEN Register",
    "NAT PKT Length\n\n",
#endif
    { NAT_CACHE_NAT0_PKT_LEN_REG_OFFSET },
    0,
    0,
    1599,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NAT0_PKT_LEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NAT0_KEY_RESULT, TYPE: Type_NATC_BLOCK_NATCACHE_NAT0_KEY_RESULT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_KEY_RESULT *****/
const ru_field_rec NAT_CACHE_NAT0_KEY_RESULT_NAT_KEY_RESULT_FIELD =
{
    "NAT_KEY_RESULT",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NAT0_KEY_RESULT_NAT_KEY_RESULT_FIELD_MASK },
    0,
    { NAT_CACHE_NAT0_KEY_RESULT_NAT_KEY_RESULT_FIELD_WIDTH },
    { NAT_CACHE_NAT0_KEY_RESULT_NAT_KEY_RESULT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NAT0_KEY_RESULT_FIELDS[] =
{
    &NAT_CACHE_NAT0_KEY_RESULT_NAT_KEY_RESULT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NAT0_KEY_RESULT *****/
const ru_reg_rec NAT_CACHE_NAT0_KEY_RESULT_REG =
{
    "NAT0_KEY_RESULT",
#if RU_INCLUDE_DESC
    "NAT0_KEY_RESULT 0..37 Register",
    "NAT key & context (excluding 8-byte counters) register; context is placed after the key;\nKey consists of all 0's indicates unused entry in hardware and therefore should not be\nused for lookup/hash/del/add commands; Key should contain an encoded type field\n(e.g. Unused=0, IPv4=1, IPv4m=2, IPv6=3, IPv6m=4, etc) to uniquely identify\neach key so the keys with shorter length of one type is not misidentified\nas the longer key of another type when the shorter key matches the beginning\nof the longer key.  When multiple DDR table mode is used, DDR table number should built\ninto the key so the same real key in 2 different DDR tables can be distinguished.\n\n",
#endif
    { NAT_CACHE_NAT0_KEY_RESULT_REG_OFFSET },
    NAT_CACHE_NAT0_KEY_RESULT_REG_RAM_CNT,
    4,
    1600,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NAT0_KEY_RESULT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR key table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\nIn order maximum number of key fetches,\nfor 16-byte key, bit 3 should be 0 to align at 16 byte boundary\nfor 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary\n\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1601,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR key table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1602,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR context table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\n\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1603,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR context table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1604,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR key table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\nIn order maximum number of key fetches,\nfor 16-byte key, bit 3 should be 0 to align at 16 byte boundary\nfor 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary\n\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1605,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR key table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1606,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR context table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\n\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1607,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR context table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1608,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR key table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\nIn order maximum number of key fetches,\nfor 16-byte key, bit 3 should be 0 to align at 16 byte boundary\nfor 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary\n\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1609,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR key table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1610,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR context table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\n\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1611,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR context table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1612,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR key table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\nIn order maximum number of key fetches,\nfor 16-byte key, bit 3 should be 0 to align at 16 byte boundary\nfor 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary\n\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1613,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR key table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1614,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR context table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\n\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1615,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR context table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1616,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR key table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\nIn order maximum number of key fetches,\nfor 16-byte key, bit 3 should be 0 to align at 16 byte boundary\nfor 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary\n\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1617,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR key table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1618,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR context table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\n\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1619,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR context table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1620,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR key table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\nIn order maximum number of key fetches,\nfor 16-byte key, bit 3 should be 0 to align at 16 byte boundary\nfor 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary\n\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1621,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR key table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1622,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR context table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\n\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1623,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR context table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1624,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR key table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\nIn order maximum number of key fetches,\nfor 16-byte key, bit 3 should be 0 to align at 16 byte boundary\nfor 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary\n\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1625,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR key table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1626,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR context table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\n\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1627,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR context table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1628,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR key table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\nIn order maximum number of key fetches,\nfor 16-byte key, bit 3 should be 0 to align at 16 byte boundary\nfor 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary\n\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1629,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR key table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1630,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD,
    &NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_REG =
{
    "NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER Register",
    "Lower 32-bit of the base address of DDR context table\nAddress must be 64-bit aligned (bit 2 through 0 are zero's)\n\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET },
    0,
    0,
    1631,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAR *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD =
{
    "BAR",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZEROS *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD =
{
    "ZEROS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD,
    &NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER *****/
const ru_reg_rec NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_REG =
{
    "NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER Register",
    "Upper 8-bit of the base address of DDR context table\nFor 32-bit system this field should be left as 0\n\n",
#endif
    { NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET },
    0,
    0,
    1632,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_STAT_COUNTER_CONTROL_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_HIT_COUNT_EN *****/
const ru_field_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD =
{
    "CACHE_HIT_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "Cache hit counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable cache hit counter increment on cache hit.\n1h: Enable cache hit counter increment on cache hit.\n\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_WIDTH },
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_MISS_COUNT_EN *****/
const ru_field_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD =
{
    "CACHE_MISS_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "Cache miss counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable cache miss counter increment on cache miss.\n1h: Enable cache miss counter increment on cache miss.\n\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_WIDTH },
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REQUEST_COUNT_EN *****/
const ru_field_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD =
{
    "DDR_REQUEST_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR request counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable DDR request counter increment on ddr request.\n1h: Enable DDR request counter increment on ddr request.\n\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_WIDTH },
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_EVICT_COUNT_EN *****/
const ru_field_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD =
{
    "DDR_EVICT_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR evict counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable DDR evict counter increment on ddr evict.\n1h: Enable DDR evict counter increment on ddr evict.\n\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_WIDTH },
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_FIELDS[] =
{
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD,
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD,
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD,
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0 *****/
const ru_reg_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_REG =
{
    "NATC_STAT_COUNTER_CONTROL_0",
#if RU_INCLUDE_DESC
    "NATC_STAT_COUNTER_CONTROL_0 Register",
    "NAT cache stat counter control register 0\n\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_REG_OFFSET },
    0,
    0,
    1633,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_STAT_COUNTER_CONTROL_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BLOCK_COUNT_EN *****/
const ru_field_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD =
{
    "DDR_BLOCK_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR block counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable DDR block counter increment on ddr block.\n1h: Enable DDR block counter increment on ddr block.\n\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_WIDTH },
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_WRAPAROUND_DIS *****/
const ru_field_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD =
{
    "COUNTER_WRAPAROUND_DIS",
#if RU_INCLUDE_DESC
    "",
    "Counter wraparound disable\nThis applies to all stat counters defined in\nNATC_STAT_COUNTER_CONTROL_0 and  NATC_STAT_COUNTER_CONTROL_1.\n0h: The counter will wraparound to 0 after it reaches ffffffffh.\n1h: The counter will cap at ffffffffh.\n\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_WIDTH },
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_R1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_R1_FIELD_WIDTH },
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_FIELDS[] =
{
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD,
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD,
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1 *****/
const ru_reg_rec NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_REG =
{
    "NATC_STAT_COUNTER_CONTROL_1",
#if RU_INCLUDE_DESC
    "NATC_STAT_COUNTER_CONTROL_1 Register",
    "NAT cache stat counter control register 1\n\n",
#endif
    { NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_REG_OFFSET },
    0,
    0,
    1634,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_CACHE_HIT_COUNT, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_CACHE_HIT_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_HIT_COUNT *****/
const ru_field_rec NAT_CACHE_NATC_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD =
{
    "CACHE_HIT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total cache hit count value for statistics collection\n\n",
#endif
    { NAT_CACHE_NATC_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NATC_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_CACHE_HIT_COUNT_FIELDS[] =
{
    &NAT_CACHE_NATC_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_CACHE_HIT_COUNT *****/
const ru_reg_rec NAT_CACHE_NATC_CACHE_HIT_COUNT_REG =
{
    "NATC_CACHE_HIT_COUNT",
#if RU_INCLUDE_DESC
    "NATC_CACHE_HIT_COUNT Register",
    "NATC CACHE HIT COUNT\n\n",
#endif
    { NAT_CACHE_NATC_CACHE_HIT_COUNT_REG_OFFSET },
    0,
    0,
    1635,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_CACHE_HIT_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_CACHE_MISS_COUNT, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_CACHE_MISS_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_MISS_COUNT *****/
const ru_field_rec NAT_CACHE_NATC_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD =
{
    "CACHE_MISS_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total cache miss count value for statistics collection\n\n",
#endif
    { NAT_CACHE_NATC_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NATC_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_CACHE_MISS_COUNT_FIELDS[] =
{
    &NAT_CACHE_NATC_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_CACHE_MISS_COUNT *****/
const ru_reg_rec NAT_CACHE_NATC_CACHE_MISS_COUNT_REG =
{
    "NATC_CACHE_MISS_COUNT",
#if RU_INCLUDE_DESC
    "NATC_CACHE_MISS_COUNT Register",
    "NATC CACHE MISS COUNT\n\n",
#endif
    { NAT_CACHE_NATC_CACHE_MISS_COUNT_REG_OFFSET },
    0,
    0,
    1636,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_CACHE_MISS_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_DDR_REQUEST_COUNT, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_DDR_REQUEST_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REQUEST_COUNT *****/
const ru_field_rec NAT_CACHE_NATC_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD =
{
    "DDR_REQUEST_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR request count value for statistics collection\n\n",
#endif
    { NAT_CACHE_NATC_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_DDR_REQUEST_COUNT_FIELDS[] =
{
    &NAT_CACHE_NATC_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_DDR_REQUEST_COUNT *****/
const ru_reg_rec NAT_CACHE_NATC_DDR_REQUEST_COUNT_REG =
{
    "NATC_DDR_REQUEST_COUNT",
#if RU_INCLUDE_DESC
    "NATC_DDR_REQUEST_COUNT Register",
    "NATC DDR REQUEST COUNT\n\n",
#endif
    { NAT_CACHE_NATC_DDR_REQUEST_COUNT_REG_OFFSET },
    0,
    0,
    1637,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_DDR_REQUEST_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_DDR_EVICT_COUNT, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_DDR_EVICT_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_EVICT_COUNT *****/
const ru_field_rec NAT_CACHE_NATC_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD =
{
    "DDR_EVICT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR evict count value for statistics collection.\nIt does not include the flush command evict count.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_DDR_EVICT_COUNT_FIELDS[] =
{
    &NAT_CACHE_NATC_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_DDR_EVICT_COUNT *****/
const ru_reg_rec NAT_CACHE_NATC_DDR_EVICT_COUNT_REG =
{
    "NATC_DDR_EVICT_COUNT",
#if RU_INCLUDE_DESC
    "NATC_DDR_EVICT_COUNT Register",
    "NATC DDR EVICT COUNT\n\n",
#endif
    { NAT_CACHE_NATC_DDR_EVICT_COUNT_REG_OFFSET },
    0,
    0,
    1638,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_DDR_EVICT_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_DDR_BLOCK_COUNT, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_DDR_BLOCK_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BLOCK_COUNT *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD =
{
    "DDR_BLOCK_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR blocked access count value for statistics collection\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_DDR_BLOCK_COUNT_FIELDS[] =
{
    &NAT_CACHE_NATC_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_DDR_BLOCK_COUNT *****/
const ru_reg_rec NAT_CACHE_NATC_DDR_BLOCK_COUNT_REG =
{
    "NATC_DDR_BLOCK_COUNT",
#if RU_INCLUDE_DESC
    "NATC_DDR_BLOCK_COUNT Register",
    "DDR BLOCK COUNT\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BLOCK_COUNT_REG_OFFSET },
    0,
    0,
    1639,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_DDR_BLOCK_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_DDR_SIZE, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_DDR_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL0_FIELD =
{
    "DDR_SIZE_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 0\nValue of 6 or above is invalid\nTo compute the actual size of the table, add DDR_BINS_PER_BUCKET field\nto the table size selection;\nFor instance, if DDR_BINS_PER_BUCKET is 3 (4 bins per bucket)\nand DDR_size is 3 (64k entries), the actual size of the table in DDR is\n(64*1024+3) multiply by total length (TOTAL_LEN) of key and context in bytes\nExtra 3 entries are used to store collided entries of the last entry\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL1_FIELD =
{
    "DDR_SIZE_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 1\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\nSee description of DDR_SIZE_TBL0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL2_FIELD =
{
    "DDR_SIZE_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 2\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL3_FIELD =
{
    "DDR_SIZE_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 3\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL4_FIELD =
{
    "DDR_SIZE_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 4\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL5_FIELD =
{
    "DDR_SIZE_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 5\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL6_FIELD =
{
    "DDR_SIZE_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 6\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SIZE_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL7_FIELD =
{
    "DDR_SIZE_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in DDR table 7\nSee description of DDR_SIZE_TBL0\n0=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_SIZE_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_R1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_SIZE_R1_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_SIZE_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_DDR_SIZE_FIELDS[] =
{
    &NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL0_FIELD,
    &NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL1_FIELD,
    &NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL2_FIELD,
    &NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL3_FIELD,
    &NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL4_FIELD,
    &NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL5_FIELD,
    &NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL6_FIELD,
    &NAT_CACHE_NATC_DDR_SIZE_DDR_SIZE_TBL7_FIELD,
    &NAT_CACHE_NATC_DDR_SIZE_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_DDR_SIZE *****/
const ru_reg_rec NAT_CACHE_NATC_DDR_SIZE_REG =
{
    "NATC_DDR_SIZE",
#if RU_INCLUDE_DESC
    "NATC_DDR_SIZE Register",
    "DDR Size Register\n\n",
#endif
    { NAT_CACHE_NATC_DDR_SIZE_REG_OFFSET },
    0,
    0,
    1640,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    NAT_CACHE_NATC_DDR_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_DDR_BINS_PER_BUCKET_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 0\nThis is limited by bus max burst size.  For instance, if\nUBUS supports max burst size of 128 bytes, key length is 16\nbytes, maximum DDR_BINS_PER_BUCKET that can be programmed\nis 128 bytes / 16-bytes (bytes per bin) = 8 entries\n0h: 1 entry\n1h: 2 entries\n2h: 3 entries\n3h: 4 entries\n4h: 5 entries\n5h: 6 entries\n6h: 7 entries\n7h: 8 entries\n.............\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 1\nSee description of DDR_BINS_PER_BUCKET_TBL0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 2\nSee description of DDR_BINS_PER_BUCKET_TBL0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 3\nSee description of DDR_BINS_PER_BUCKET_TBL0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_FIELDS[] =
{
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL0_FIELD,
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL1_FIELD,
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL2_FIELD,
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_DDR_BINS_PER_BUCKET_TBL3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0 *****/
const ru_reg_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_REG =
{
    "NATC_DDR_BINS_PER_BUCKET_0",
#if RU_INCLUDE_DESC
    "NATC_DDR_BINS_PER_BUCKET_0 Register",
    "DDR Bins Per Bucket Register 0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_REG_OFFSET },
    0,
    0,
    1641,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_DDR_BINS_PER_BUCKET_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 4\nSee description of DDR_BINS_PER_BUCKET_TBL0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 5\nSee description of DDR_BINS_PER_BUCKET_TBL0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 6\nSee description of DDR_BINS_PER_BUCKET_TBL0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BINS_PER_BUCKET_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD =
{
    "DDR_BINS_PER_BUCKET_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket in DDR table 7\nSee description of DDR_BINS_PER_BUCKET_TBL0\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_FIELDS[] =
{
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL4_FIELD,
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL5_FIELD,
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL6_FIELD,
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_DDR_BINS_PER_BUCKET_TBL7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1 *****/
const ru_reg_rec NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_REG =
{
    "NATC_DDR_BINS_PER_BUCKET_1",
#if RU_INCLUDE_DESC
    "NATC_DDR_BINS_PER_BUCKET_1 Register",
    "DDR Bins Per Bucket Register 1\n\n",
#endif
    { NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_REG_OFFSET },
    0,
    0,
    1642,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TOTAL_LEN, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TOTAL_LEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD =
{
    "TOTAL_LEN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 0\nThe context length (including 8-byte counters) is calculated by TOTAL_LEN minus KEY_LEN\nThe maximum value should not exceed hardware capability.\nFor instance, most projects have max of 80-bytes and BCM63158 has max value of 144-byte.\n0h: 48-byte\n1h: 64-byte\n2h: 80-byte\n3h: 96-byte\n4h: 112-byte\n5h: 128-byte\n6h: 144-byte\n7h: 160-byte\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD =
{
    "TOTAL_LEN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 1\nSee description of TOTAL_LEN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD =
{
    "TOTAL_LEN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 2\nSee description of TOTAL_LEN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD =
{
    "TOTAL_LEN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 3\nSee description of TOTAL_LEN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD =
{
    "TOTAL_LEN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 4\nSee description of TOTAL_LEN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD =
{
    "TOTAL_LEN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 5\nSee description of TOTAL_LEN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD =
{
    "TOTAL_LEN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 6\nSee description of TOTAL_LEN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_LEN_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD =
{
    "TOTAL_LEN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Length of the lookup key plus context (including 8-byte counters) in DDR table 7\nSee description of TOTAL_LEN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec NAT_CACHE_NATC_TOTAL_LEN_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_R1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TOTAL_LEN_R1_FIELD_WIDTH },
    { NAT_CACHE_NATC_TOTAL_LEN_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TOTAL_LEN_FIELDS[] =
{
    &NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL0_FIELD,
    &NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL1_FIELD,
    &NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL2_FIELD,
    &NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL3_FIELD,
    &NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL4_FIELD,
    &NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL5_FIELD,
    &NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL6_FIELD,
    &NAT_CACHE_NATC_TOTAL_LEN_TOTAL_LEN_TBL7_FIELD,
    &NAT_CACHE_NATC_TOTAL_LEN_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TOTAL_LEN *****/
const ru_reg_rec NAT_CACHE_NATC_TOTAL_LEN_REG =
{
    "NATC_TOTAL_LEN",
#if RU_INCLUDE_DESC
    "NATC_TOTAL_LEN Register",
    "DDR TABLE Total Length Register\n\n",
#endif
    { NAT_CACHE_NATC_TOTAL_LEN_REG_OFFSET },
    0,
    0,
    1643,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    NAT_CACHE_NATC_TOTAL_LEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_SM_STATUS, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_SM_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_STATE *****/
const ru_field_rec NAT_CACHE_NATC_SM_STATUS_NAT_STATE_FIELD =
{
    "NAT_STATE",
#if RU_INCLUDE_DESC
    "",
    "Nat state machine.\n15'b000000000000000: NAT_ST_IDLE.\n15'b000000000000001: NAT_ST_IDLE_WRITE_SMEM.\n15'b000000000000010: NAT_ST_IDLE_DDR_PENDING.\n15'b000000000000100: NAT_ST_HASH.\n15'b000000000001000: NAT_ST_NAT_MEM_READ_REQ.\n15'b000000000010000: NAT_ST_NAT_MEM_WRITE_REQ.\n15'b000000000100000: NAT_ST_READ_SMEM.\n15'b000000001000000: NAT_ST_UPDATE_DDR.\n15'b000000010000000: NAT_ST_IDLE_BLOCKING_PENDING.\n15'b000000100000000: NAT_ST_EVICT_WAIT.\n15'b000001000000000: NAT_ST_CHECK_NON_CACHEABLE.\n15'b000010000000000: NAT_ST_WAIT.\n15'b000100000000000: NAT_ST_WAIT_NATC_MEM_REQ_DONE.\n15'b001000000000000: NAT_ST_CACHE_FLUSH.\n15'b010000000000000: NAT_ST_DDR_MISS_0.\n15'b100000000000000: NAT_ST_DDR_MISS_1.\n\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_NAT_STATE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_SM_STATUS_NAT_STATE_FIELD_WIDTH },
    { NAT_CACHE_NATC_SM_STATUS_NAT_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WB_STATE *****/
const ru_field_rec NAT_CACHE_NATC_SM_STATUS_WB_STATE_FIELD =
{
    "WB_STATE",
#if RU_INCLUDE_DESC
    "",
    "Write-back state machine.\n1'b0: WB_ST_IDLE.\n1'b1: WB_ST_WRITE_BACIF.\n\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_WB_STATE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_SM_STATUS_WB_STATE_FIELD_WIDTH },
    { NAT_CACHE_NATC_SM_STATUS_WB_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RUNNER_CMD_STATE *****/
const ru_field_rec NAT_CACHE_NATC_SM_STATUS_RUNNER_CMD_STATE_FIELD =
{
    "RUNNER_CMD_STATE",
#if RU_INCLUDE_DESC
    "",
    "Runner command state machine.\n1'b0: RUNNER_CMD_ST_IDLE.\n1'b1: RUNNER_CMD_ST_WRITE_RUNNER_FIFO.\n\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_RUNNER_CMD_STATE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_SM_STATUS_RUNNER_CMD_STATE_FIELD_WIDTH },
    { NAT_CACHE_NATC_SM_STATUS_RUNNER_CMD_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REP_STATE *****/
const ru_field_rec NAT_CACHE_NATC_SM_STATUS_DDR_REP_STATE_FIELD =
{
    "DDR_REP_STATE",
#if RU_INCLUDE_DESC
    "",
    "DDR reply state machine.\n3'b000: DDR_REP_ST_IDLE.\n3'b001: DDR_REP_ST_READ_DATA.\n3'b010: DDR_REP_ST_READ_RESULT.\n3'b011: DDR_REP_ST_READ_WAIT.\n3'b100: DDR_REP_ST_EVICT_WR_NON_CACHEABLE.\n\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_DDR_REP_STATE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_SM_STATUS_DDR_REP_STATE_FIELD_WIDTH },
    { NAT_CACHE_NATC_SM_STATUS_DDR_REP_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REQ_STATE *****/
const ru_field_rec NAT_CACHE_NATC_SM_STATUS_DDR_REQ_STATE_FIELD =
{
    "DDR_REQ_STATE",
#if RU_INCLUDE_DESC
    "",
    "DDR request state machine.\n2'b00: DDR_REQ_ST_IDLE.\n2'b01: DDR_REQ_ST_WRITE_HEADER.\n2'b10: DDR_REQ_ST_WRITE_HEADER_DELAY.\n\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_DDR_REQ_STATE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_SM_STATUS_DDR_REQ_STATE_FIELD_WIDTH },
    { NAT_CACHE_NATC_SM_STATUS_DDR_REQ_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: APB_STATE *****/
const ru_field_rec NAT_CACHE_NATC_SM_STATUS_APB_STATE_FIELD =
{
    "APB_STATE",
#if RU_INCLUDE_DESC
    "",
    "APB to RBUS bridge state machine.\n2'b00: APB_ST_IDLE.\n2'b01: APB_ST_RW.\n2'b10: AOB_ST_END.\n\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_APB_STATE_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_SM_STATUS_APB_STATE_FIELD_WIDTH },
    { NAT_CACHE_NATC_SM_STATUS_APB_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEBUG_SEL *****/
const ru_field_rec NAT_CACHE_NATC_SM_STATUS_DEBUG_SEL_FIELD =
{
    "DEBUG_SEL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus select.\n2'b00: prb_nat_control.\n2'b01: prb_cmd_control.\n2'b10: prb_wb_control.\n2'b11: prb_ddr_control.\n\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_DEBUG_SEL_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_SM_STATUS_DEBUG_SEL_FIELD_WIDTH },
    { NAT_CACHE_NATC_SM_STATUS_DEBUG_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec NAT_CACHE_NATC_SM_STATUS_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_R1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_SM_STATUS_R1_FIELD_WIDTH },
    { NAT_CACHE_NATC_SM_STATUS_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_SM_STATUS_FIELDS[] =
{
    &NAT_CACHE_NATC_SM_STATUS_NAT_STATE_FIELD,
    &NAT_CACHE_NATC_SM_STATUS_WB_STATE_FIELD,
    &NAT_CACHE_NATC_SM_STATUS_RUNNER_CMD_STATE_FIELD,
    &NAT_CACHE_NATC_SM_STATUS_DDR_REP_STATE_FIELD,
    &NAT_CACHE_NATC_SM_STATUS_DDR_REQ_STATE_FIELD,
    &NAT_CACHE_NATC_SM_STATUS_APB_STATE_FIELD,
    &NAT_CACHE_NATC_SM_STATUS_DEBUG_SEL_FIELD,
    &NAT_CACHE_NATC_SM_STATUS_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_SM_STATUS *****/
const ru_reg_rec NAT_CACHE_NATC_SM_STATUS_REG =
{
    "NATC_SM_STATUS",
#if RU_INCLUDE_DESC
    "NATC_SM_STATUS Register",
    "NAT State Machine Status Register\n\n",
#endif
    { NAT_CACHE_NATC_SM_STATUS_REG_OFFSET },
    0,
    0,
    1644,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    NAT_CACHE_NATC_SM_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_REGFILE_FIFO_START_ADDR_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_0 *****/
const ru_field_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD =
{
    "REGFILE_FIFO_START_ADDR_0",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 0 Start Address\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD_WIDTH },
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_1 *****/
const ru_field_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD =
{
    "REGFILE_FIFO_START_ADDR_1",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 1 Start Address\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD_WIDTH },
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD_SHIFT },
    6,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_2 *****/
const ru_field_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD =
{
    "REGFILE_FIFO_START_ADDR_2",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 2 Start Address\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD_WIDTH },
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD_SHIFT },
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_3 *****/
const ru_field_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD =
{
    "REGFILE_FIFO_START_ADDR_3",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 2 Start Address\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD_WIDTH },
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD_SHIFT },
    26,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_FIELDS[] =
{
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD,
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD,
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD,
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0 *****/
const ru_reg_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REG =
{
    "NATC_REGFILE_FIFO_START_ADDR_0",
#if RU_INCLUDE_DESC
    "NATC_REGFILE_FIFO_START_ADDR_0 Register",
    "REGFILE FIFO Start Address register 0\nActual FIFO size is 2 more than the number programmed in\nthis register due to input and output holder registers\nwhich account for 2 additional depth.\nThe actual FIFO 0 (DDR_KEY_REQ_FIFO) size is\nREGFILE_FIFO_START_ADDR_1 -  REGFILE_FIFO_START_ADDR_0 + 2.\nThe actual FIFO 1 (DDR_RESULT_REQ_FIFO) size is\nREGFILE_FIFO_START_ADDR_2 -  REGFILE_FIFO_START_ADDR_1 + 2.\nThe actual FIFO 2 (DDR_KEY_REQ_PIPE) size is\nREGFILE_FIFO_START_ADDR_3 -  REGFILE_FIFO_START_ADDR_2 + 2.\nThe actual FIFO 3 (BLOCKING_PENDING_FIFO) size is\nREGFILE_FIFO_START_ADDR_4 -  REGFILE_FIFO_START_ADDR_3 + 2.\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REG_OFFSET },
    0,
    0,
    1645,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_REGFILE_FIFO_START_ADDR_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_4 *****/
const ru_field_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD =
{
    "REGFILE_FIFO_START_ADDR_4",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 4 Start Address\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD_WIDTH },
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD_SHIFT },
    56,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_5 *****/
const ru_field_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD =
{
    "REGFILE_FIFO_START_ADDR_5",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 5 Start Address\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD_WIDTH },
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD_SHIFT },
    62,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_6 *****/
const ru_field_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD =
{
    "REGFILE_FIFO_START_ADDR_6",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 6 Start Address\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD_WIDTH },
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD_SHIFT },
    68,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_7 *****/
const ru_field_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD =
{
    "REGFILE_FIFO_START_ADDR_7",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 7 Start Address-- Note that this entry is not used\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD_WIDTH },
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD_SHIFT },
    74,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_FIELDS[] =
{
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD,
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD,
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD,
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1 *****/
const ru_reg_rec NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REG =
{
    "NATC_REGFILE_FIFO_START_ADDR_1",
#if RU_INCLUDE_DESC
    "NATC_REGFILE_FIFO_START_ADDR_1 Register",
    "REGFILE FIFO Start Address register 1\nActual FIFO size is 2 more than the number programmed in\nthis register due to input and output holder registers\nwhich account for 2 additional depth.\nThe delta between REGFILE_FIFO_START_ADDR_4 and REGFILE_FIFO_START_ADDR_5,\nREGFILE_FIFO_START_ADDR_5 and REGFILE_FIFO_START_ADDR_6,\nREGFILE_FIFO_START_ADDR_6 and REGFILE_FIFO_START_ADDR_7\nneed to be identical since these are used for the same wide FIFO.\nThe actual FIFO 4 (DDR_WRITE_RESULT_FIFO) size is\nREGFILE_FIFO_START_ADDR_5 -  REGFILE_FIFO_START_ADDR_4 + 2.\nThe actual FIFO 5 (DDR_WRITE_RESULT_FIFO) size is\nREGFILE_FIFO_START_ADDR_6 -  REGFILE_FIFO_START_ADDR_5 + 2.\nThe actual FIFO 6 (DDR_WRITE_RESULT_FIFO) size is\nREGFILE_FIFO_START_ADDR_7 -  REGFILE_FIFO_START_ADDR_6 + 2.\nThe actual FIFO 7 size is the same as FIFO 4, 5, 6\n\n",
#endif
    { NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REG_OFFSET },
    0,
    0,
    1646,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL0_KEY_MASK, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL0_KEY_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_MASK *****/
const ru_field_rec NAT_CACHE_NATC_TBL0_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each bit in the key.\nFor 16-byte key, there are corresponding 4 KEY_MASK registers.\nFor 32-byte key, there are corresponding 8 KEY_MASK registers.\nEach bit in each KEY_MASK register corresponds to one bit in the key.\n0 enables the compare and 1 disables the compare.\nbit 0 of KEY_MASK register[0] corresponds to key bit 0.\nbit 1 of KEY_MASK register[0] corresponds to key bit 1.\nbit 2 of KEY_MASK register[0] corresponds to key bit 2.\n......................\nbit 31 of KEY_MASK register [0] corresponds to key bit 31.\nbit 0 of KEY_MASK register[1] corresponds to key bit 32.\nbit 1 of KEY_MASK register[1] corresponds to key bit 33.\nbit 2 of KEY_MASK register[1] corresponds to key bit 34.\n......................\nbit 31 of KEY_MASK register [1] corresponds to key bit 63.\n......................\nbit 0 of KEY_MASK register[7] corresponds to key bit 224.\nbit 1 of KEY_MASK register[7] corresponds to key bit 225.\nbit 2 of KEY_MASK register[7] corresponds to key bit 226.\n......................\nbit 31 of KEY_MASK register [7] corresponds to key bit 255.\n\n",
#endif
    { NAT_CACHE_NATC_TBL0_KEY_MASK_KEY_MASK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL0_KEY_MASK_KEY_MASK_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL0_KEY_MASK_KEY_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL0_KEY_MASK_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL0_KEY_MASK_KEY_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL0_KEY_MASK *****/
const ru_reg_rec NAT_CACHE_NATC_TBL0_KEY_MASK_REG =
{
    "NATC_TBL0_KEY_MASK",
#if RU_INCLUDE_DESC
    "NATC_TBL0_KEY_MASK 0..7 Register",
    "NAT Cache key Mask Register\n\n",
#endif
    { NAT_CACHE_NATC_TBL0_KEY_MASK_REG_OFFSET },
    NAT_CACHE_NATC_TBL0_KEY_MASK_REG_RAM_CNT,
    4,
    1647,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_TBL0_KEY_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL1_KEY_MASK, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL1_KEY_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_MASK *****/
const ru_field_rec NAT_CACHE_NATC_TBL1_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each bit in the key.\nFor 16-byte key, there are corresponding 4 KEY_MASK registers.\nFor 32-byte key, there are corresponding 8 KEY_MASK registers.\nEach bit in each KEY_MASK register corresponds to one bit in the key.\n0 enables the compare and 1 disables the compare.\nbit 0 of KEY_MASK register[0] corresponds to key bit 0.\nbit 1 of KEY_MASK register[0] corresponds to key bit 1.\nbit 2 of KEY_MASK register[0] corresponds to key bit 2.\n......................\nbit 31 of KEY_MASK register [0] corresponds to key bit 31.\nbit 0 of KEY_MASK register[1] corresponds to key bit 32.\nbit 1 of KEY_MASK register[1] corresponds to key bit 33.\nbit 2 of KEY_MASK register[1] corresponds to key bit 34.\n......................\nbit 31 of KEY_MASK register [1] corresponds to key bit 63.\n......................\nbit 0 of KEY_MASK register[7] corresponds to key bit 224.\nbit 1 of KEY_MASK register[7] corresponds to key bit 225.\nbit 2 of KEY_MASK register[7] corresponds to key bit 226.\n......................\nbit 31 of KEY_MASK register [7] corresponds to key bit 255.\n\n",
#endif
    { NAT_CACHE_NATC_TBL1_KEY_MASK_KEY_MASK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL1_KEY_MASK_KEY_MASK_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL1_KEY_MASK_KEY_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL1_KEY_MASK_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL1_KEY_MASK_KEY_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL1_KEY_MASK *****/
const ru_reg_rec NAT_CACHE_NATC_TBL1_KEY_MASK_REG =
{
    "NATC_TBL1_KEY_MASK",
#if RU_INCLUDE_DESC
    "NATC_TBL1_KEY_MASK 0..7 Register",
    "NAT Cache key Mask Register\n\n",
#endif
    { NAT_CACHE_NATC_TBL1_KEY_MASK_REG_OFFSET },
    NAT_CACHE_NATC_TBL1_KEY_MASK_REG_RAM_CNT,
    4,
    1648,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_TBL1_KEY_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL2_KEY_MASK, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL2_KEY_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_MASK *****/
const ru_field_rec NAT_CACHE_NATC_TBL2_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each bit in the key.\nFor 16-byte key, there are corresponding 4 KEY_MASK registers.\nFor 32-byte key, there are corresponding 8 KEY_MASK registers.\nEach bit in each KEY_MASK register corresponds to one bit in the key.\n0 enables the compare and 1 disables the compare.\nbit 0 of KEY_MASK register[0] corresponds to key bit 0.\nbit 1 of KEY_MASK register[0] corresponds to key bit 1.\nbit 2 of KEY_MASK register[0] corresponds to key bit 2.\n......................\nbit 31 of KEY_MASK register [0] corresponds to key bit 31.\nbit 0 of KEY_MASK register[1] corresponds to key bit 32.\nbit 1 of KEY_MASK register[1] corresponds to key bit 33.\nbit 2 of KEY_MASK register[1] corresponds to key bit 34.\n......................\nbit 31 of KEY_MASK register [1] corresponds to key bit 63.\n......................\nbit 0 of KEY_MASK register[7] corresponds to key bit 224.\nbit 1 of KEY_MASK register[7] corresponds to key bit 225.\nbit 2 of KEY_MASK register[7] corresponds to key bit 226.\n......................\nbit 31 of KEY_MASK register [7] corresponds to key bit 255.\n\n",
#endif
    { NAT_CACHE_NATC_TBL2_KEY_MASK_KEY_MASK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL2_KEY_MASK_KEY_MASK_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL2_KEY_MASK_KEY_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL2_KEY_MASK_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL2_KEY_MASK_KEY_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL2_KEY_MASK *****/
const ru_reg_rec NAT_CACHE_NATC_TBL2_KEY_MASK_REG =
{
    "NATC_TBL2_KEY_MASK",
#if RU_INCLUDE_DESC
    "NATC_TBL2_KEY_MASK 0..7 Register",
    "NAT Cache key Mask Register\n\n",
#endif
    { NAT_CACHE_NATC_TBL2_KEY_MASK_REG_OFFSET },
    NAT_CACHE_NATC_TBL2_KEY_MASK_REG_RAM_CNT,
    4,
    1649,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_TBL2_KEY_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL3_KEY_MASK, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL3_KEY_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_MASK *****/
const ru_field_rec NAT_CACHE_NATC_TBL3_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each bit in the key.\nFor 16-byte key, there are corresponding 4 KEY_MASK registers.\nFor 32-byte key, there are corresponding 8 KEY_MASK registers.\nEach bit in each KEY_MASK register corresponds to one bit in the key.\n0 enables the compare and 1 disables the compare.\nbit 0 of KEY_MASK register[0] corresponds to key bit 0.\nbit 1 of KEY_MASK register[0] corresponds to key bit 1.\nbit 2 of KEY_MASK register[0] corresponds to key bit 2.\n......................\nbit 31 of KEY_MASK register [0] corresponds to key bit 31.\nbit 0 of KEY_MASK register[1] corresponds to key bit 32.\nbit 1 of KEY_MASK register[1] corresponds to key bit 33.\nbit 2 of KEY_MASK register[1] corresponds to key bit 34.\n......................\nbit 31 of KEY_MASK register [1] corresponds to key bit 63.\n......................\nbit 0 of KEY_MASK register[7] corresponds to key bit 224.\nbit 1 of KEY_MASK register[7] corresponds to key bit 225.\nbit 2 of KEY_MASK register[7] corresponds to key bit 226.\n......................\nbit 31 of KEY_MASK register [7] corresponds to key bit 255.\n\n",
#endif
    { NAT_CACHE_NATC_TBL3_KEY_MASK_KEY_MASK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL3_KEY_MASK_KEY_MASK_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL3_KEY_MASK_KEY_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL3_KEY_MASK_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL3_KEY_MASK_KEY_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL3_KEY_MASK *****/
const ru_reg_rec NAT_CACHE_NATC_TBL3_KEY_MASK_REG =
{
    "NATC_TBL3_KEY_MASK",
#if RU_INCLUDE_DESC
    "NATC_TBL3_KEY_MASK 0..7 Register",
    "NAT Cache key Mask Register\n\n",
#endif
    { NAT_CACHE_NATC_TBL3_KEY_MASK_REG_OFFSET },
    NAT_CACHE_NATC_TBL3_KEY_MASK_REG_RAM_CNT,
    4,
    1650,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_TBL3_KEY_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL4_KEY_MASK, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL4_KEY_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_MASK *****/
const ru_field_rec NAT_CACHE_NATC_TBL4_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each bit in the key.\nFor 16-byte key, there are corresponding 4 KEY_MASK registers.\nFor 32-byte key, there are corresponding 8 KEY_MASK registers.\nEach bit in each KEY_MASK register corresponds to one bit in the key.\n0 enables the compare and 1 disables the compare.\nbit 0 of KEY_MASK register[0] corresponds to key bit 0.\nbit 1 of KEY_MASK register[0] corresponds to key bit 1.\nbit 2 of KEY_MASK register[0] corresponds to key bit 2.\n......................\nbit 31 of KEY_MASK register [0] corresponds to key bit 31.\nbit 0 of KEY_MASK register[1] corresponds to key bit 32.\nbit 1 of KEY_MASK register[1] corresponds to key bit 33.\nbit 2 of KEY_MASK register[1] corresponds to key bit 34.\n......................\nbit 31 of KEY_MASK register [1] corresponds to key bit 63.\n......................\nbit 0 of KEY_MASK register[7] corresponds to key bit 224.\nbit 1 of KEY_MASK register[7] corresponds to key bit 225.\nbit 2 of KEY_MASK register[7] corresponds to key bit 226.\n......................\nbit 31 of KEY_MASK register [7] corresponds to key bit 255.\n\n",
#endif
    { NAT_CACHE_NATC_TBL4_KEY_MASK_KEY_MASK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL4_KEY_MASK_KEY_MASK_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL4_KEY_MASK_KEY_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL4_KEY_MASK_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL4_KEY_MASK_KEY_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL4_KEY_MASK *****/
const ru_reg_rec NAT_CACHE_NATC_TBL4_KEY_MASK_REG =
{
    "NATC_TBL4_KEY_MASK",
#if RU_INCLUDE_DESC
    "NATC_TBL4_KEY_MASK 0..7 Register",
    "NAT Cache key Mask Register\n\n",
#endif
    { NAT_CACHE_NATC_TBL4_KEY_MASK_REG_OFFSET },
    NAT_CACHE_NATC_TBL4_KEY_MASK_REG_RAM_CNT,
    4,
    1651,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_TBL4_KEY_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL5_KEY_MASK, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL5_KEY_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_MASK *****/
const ru_field_rec NAT_CACHE_NATC_TBL5_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each bit in the key.\nFor 16-byte key, there are corresponding 4 KEY_MASK registers.\nFor 32-byte key, there are corresponding 8 KEY_MASK registers.\nEach bit in each KEY_MASK register corresponds to one bit in the key.\n0 enables the compare and 1 disables the compare.\nbit 0 of KEY_MASK register[0] corresponds to key bit 0.\nbit 1 of KEY_MASK register[0] corresponds to key bit 1.\nbit 2 of KEY_MASK register[0] corresponds to key bit 2.\n......................\nbit 31 of KEY_MASK register [0] corresponds to key bit 31.\nbit 0 of KEY_MASK register[1] corresponds to key bit 32.\nbit 1 of KEY_MASK register[1] corresponds to key bit 33.\nbit 2 of KEY_MASK register[1] corresponds to key bit 34.\n......................\nbit 31 of KEY_MASK register [1] corresponds to key bit 63.\n......................\nbit 0 of KEY_MASK register[7] corresponds to key bit 224.\nbit 1 of KEY_MASK register[7] corresponds to key bit 225.\nbit 2 of KEY_MASK register[7] corresponds to key bit 226.\n......................\nbit 31 of KEY_MASK register [7] corresponds to key bit 255.\n\n",
#endif
    { NAT_CACHE_NATC_TBL5_KEY_MASK_KEY_MASK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL5_KEY_MASK_KEY_MASK_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL5_KEY_MASK_KEY_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL5_KEY_MASK_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL5_KEY_MASK_KEY_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL5_KEY_MASK *****/
const ru_reg_rec NAT_CACHE_NATC_TBL5_KEY_MASK_REG =
{
    "NATC_TBL5_KEY_MASK",
#if RU_INCLUDE_DESC
    "NATC_TBL5_KEY_MASK 0..7 Register",
    "NAT Cache key Mask Register\n\n",
#endif
    { NAT_CACHE_NATC_TBL5_KEY_MASK_REG_OFFSET },
    NAT_CACHE_NATC_TBL5_KEY_MASK_REG_RAM_CNT,
    4,
    1652,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_TBL5_KEY_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL6_KEY_MASK, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL6_KEY_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_MASK *****/
const ru_field_rec NAT_CACHE_NATC_TBL6_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each bit in the key.\nFor 16-byte key, there are corresponding 4 KEY_MASK registers.\nFor 32-byte key, there are corresponding 8 KEY_MASK registers.\nEach bit in each KEY_MASK register corresponds to one bit in the key.\n0 enables the compare and 1 disables the compare.\nbit 0 of KEY_MASK register[0] corresponds to key bit 0.\nbit 1 of KEY_MASK register[0] corresponds to key bit 1.\nbit 2 of KEY_MASK register[0] corresponds to key bit 2.\n......................\nbit 31 of KEY_MASK register [0] corresponds to key bit 31.\nbit 0 of KEY_MASK register[1] corresponds to key bit 32.\nbit 1 of KEY_MASK register[1] corresponds to key bit 33.\nbit 2 of KEY_MASK register[1] corresponds to key bit 34.\n......................\nbit 31 of KEY_MASK register [1] corresponds to key bit 63.\n......................\nbit 0 of KEY_MASK register[7] corresponds to key bit 224.\nbit 1 of KEY_MASK register[7] corresponds to key bit 225.\nbit 2 of KEY_MASK register[7] corresponds to key bit 226.\n......................\nbit 31 of KEY_MASK register [7] corresponds to key bit 255.\n\n",
#endif
    { NAT_CACHE_NATC_TBL6_KEY_MASK_KEY_MASK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL6_KEY_MASK_KEY_MASK_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL6_KEY_MASK_KEY_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL6_KEY_MASK_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL6_KEY_MASK_KEY_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL6_KEY_MASK *****/
const ru_reg_rec NAT_CACHE_NATC_TBL6_KEY_MASK_REG =
{
    "NATC_TBL6_KEY_MASK",
#if RU_INCLUDE_DESC
    "NATC_TBL6_KEY_MASK 0..7 Register",
    "NAT Cache key Mask Register\n\n",
#endif
    { NAT_CACHE_NATC_TBL6_KEY_MASK_REG_OFFSET },
    NAT_CACHE_NATC_TBL6_KEY_MASK_REG_RAM_CNT,
    4,
    1653,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_TBL6_KEY_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_TBL7_KEY_MASK, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_TBL7_KEY_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_MASK *****/
const ru_field_rec NAT_CACHE_NATC_TBL7_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each bit in the key.\nFor 16-byte key, there are corresponding 4 KEY_MASK registers.\nFor 32-byte key, there are corresponding 8 KEY_MASK registers.\nEach bit in each KEY_MASK register corresponds to one bit in the key.\n0 enables the compare and 1 disables the compare.\nbit 0 of KEY_MASK register[0] corresponds to key bit 0.\nbit 1 of KEY_MASK register[0] corresponds to key bit 1.\nbit 2 of KEY_MASK register[0] corresponds to key bit 2.\n......................\nbit 31 of KEY_MASK register [0] corresponds to key bit 31.\nbit 0 of KEY_MASK register[1] corresponds to key bit 32.\nbit 1 of KEY_MASK register[1] corresponds to key bit 33.\nbit 2 of KEY_MASK register[1] corresponds to key bit 34.\n......................\nbit 31 of KEY_MASK register [1] corresponds to key bit 63.\n......................\nbit 0 of KEY_MASK register[7] corresponds to key bit 224.\nbit 1 of KEY_MASK register[7] corresponds to key bit 225.\nbit 2 of KEY_MASK register[7] corresponds to key bit 226.\n......................\nbit 31 of KEY_MASK register [7] corresponds to key bit 255.\n\n",
#endif
    { NAT_CACHE_NATC_TBL7_KEY_MASK_KEY_MASK_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_TBL7_KEY_MASK_KEY_MASK_FIELD_WIDTH },
    { NAT_CACHE_NATC_TBL7_KEY_MASK_KEY_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_TBL7_KEY_MASK_FIELDS[] =
{
    &NAT_CACHE_NATC_TBL7_KEY_MASK_KEY_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_TBL7_KEY_MASK *****/
const ru_reg_rec NAT_CACHE_NATC_TBL7_KEY_MASK_REG =
{
    "NATC_TBL7_KEY_MASK",
#if RU_INCLUDE_DESC
    "NATC_TBL7_KEY_MASK 0..7 Register",
    "NAT Cache key Mask Register\n\n",
#endif
    { NAT_CACHE_NATC_TBL7_KEY_MASK_REG_OFFSET },
    NAT_CACHE_NATC_TBL7_KEY_MASK_REG_RAM_CNT,
    4,
    1654,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_NATC_TBL7_KEY_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_DDR_HASH_MODE, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_DDR_HASH_MODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD =
{
    "DDR_HASH_MODE_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 0 lookup.\nHash value is DDR table size dependent.\n0h: 32-bit rolling XOR hash is used as DDR hash function. It is reduced to N-bit\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n1h: CRC32 hash is used as DDR hash function. CRC32 is reduced to N-bit using\nthe same method as in 32-bit rolling XOR hash.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n2h: CRC32 hash is used as DDR hash function. CRC32[N:0] is used as hash value\nDDR table size is 8K,   N = 12.\nDDR table size is 16K,  N = 13.\nDDR table size is 32K,  N = 14.\nDDR table size is 64K,  N = 15.\nDDR table size is 128K, N = 16.\nDDR table size is 256K, N = 17.\n3h: CRC32 hash is used as DDR hash function. CRC32[31:N] is used as hash value\nDDR table size is 8K,   N = 19.\nDDR table size is 16K,  N = 18.\nDDR table size is 32K,  N = 17.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 15.\nDDR table size is 256K, N = 14.\n4h: RSS hash is used as DDR hash function using secret key 0. RSS[N:0] is used as hash value.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n5h: RSS hash is used as DDR hash function using secret key 1. RSS[N:0] is used as hash value.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n6h: RSS hash is used as DDR hash function using secret key 2. RSS[N:0] is used as hash value.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n7h: RSS hash is used as DDR hash function using secret key 3. RSS[N:0] is used as hash value.\nDDR table size is 8K,   N = 13.\nDDR table size is 16K,  N = 14.\nDDR table size is 32K,  N = 15.\nDDR table size is 64K,  N = 16.\nDDR table size is 128K, N = 17.\nDDR table size is 256K, N = 18.\n8h: Key[N:4] is used as hash value.\nDDR table size is 8K,   N = 16.\nDDR table size is 16K,  N = 17.\nDDR table size is 32K,  N = 18.\nDDR table size is 64K,  N = 19.\nDDR table size is 128K, N = 20.\nDDR table size is 256K, N = 21.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD =
{
    "DDR_HASH_MODE_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 1 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD =
{
    "DDR_HASH_MODE_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 2 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD =
{
    "DDR_HASH_MODE_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 3 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD =
{
    "DDR_HASH_MODE_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 4 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD =
{
    "DDR_HASH_MODE_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 5 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD =
{
    "DDR_HASH_MODE_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 6 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_MODE_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD =
{
    "DDR_HASH_MODE_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR table 7 lookup.\nSee description of DDR_HASH_MODE_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_DDR_HASH_MODE_FIELDS[] =
{
    &NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL0_FIELD,
    &NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL1_FIELD,
    &NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL2_FIELD,
    &NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL3_FIELD,
    &NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL4_FIELD,
    &NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL5_FIELD,
    &NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL6_FIELD,
    &NAT_CACHE_NATC_DDR_HASH_MODE_DDR_HASH_MODE_TBL7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_DDR_HASH_MODE *****/
const ru_reg_rec NAT_CACHE_NATC_DDR_HASH_MODE_REG =
{
    "NATC_DDR_HASH_MODE",
#if RU_INCLUDE_DESC
    "NATC_DDR_HASH_MODE Register",
    "NAT Cache DDR Hash Mode Register\n\n",
#endif
    { NAT_CACHE_NATC_DDR_HASH_MODE_REG_OFFSET },
    0,
    0,
    1655,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    NAT_CACHE_NATC_DDR_HASH_MODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_NATC_FLOW_CNTR_CNTL, TYPE: Type_NATC_BLOCK_NATCACHE_NATC_FLOW_CNTR_CNTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT_OFFSET *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD =
{
    "CONTEXT_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "This is the byte offset of the context.\nOnly bits 6:0 of this byte in the context are used as flow counter address.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_R2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_R2_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL0 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD =
{
    "FLOW_CNTR_EN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 0.\nIf this bit is set to 1 and CONTEXT_OFFSET is less than context length - 8 (byte count and hit cout),\nthe flow counter is enabled. Otherwsie the flow counter is disabled.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL1 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD =
{
    "FLOW_CNTR_EN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 1.\nSee description of FLOW_CNTR_EN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL2 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD =
{
    "FLOW_CNTR_EN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 2.\nSee description of FLOW_CNTR_EN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL3 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD =
{
    "FLOW_CNTR_EN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 3.\nSee description of FLOW_CNTR_EN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL4 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD =
{
    "FLOW_CNTR_EN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 4.\nSee description of FLOW_CNTR_EN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL5 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD =
{
    "FLOW_CNTR_EN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 5.\nSee description of FLOW_CNTR_EN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL6 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD =
{
    "FLOW_CNTR_EN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 6.\nSee description of FLOW_CNTR_EN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL7 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD =
{
    "FLOW_CNTR_EN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 7.\nSee description of FLOW_CNTR_EN_TBL0.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_CLR_ON_RD_EN *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_CLR_ON_RD_EN_FIELD =
{
    "FLOW_CNTR_CLR_ON_RD_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters clear on read.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_CLR_ON_RD_EN_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_CLR_ON_RD_EN_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_CLR_ON_RD_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_R1_FIELD_MASK },
    0,
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_R1_FIELD_WIDTH },
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_NATC_FLOW_CNTR_CNTL_FIELDS[] =
{
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_R2_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_FLOW_CNTR_CLR_ON_RD_EN_FIELD,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_NATC_FLOW_CNTR_CNTL *****/
const ru_reg_rec NAT_CACHE_NATC_FLOW_CNTR_CNTL_REG =
{
    "NATC_FLOW_CNTR_CNTL",
#if RU_INCLUDE_DESC
    "NATC_FLOW_CNTR_CNTL Register",
    "NAT Cache Flow Counter Control Register.\n\n",
#endif
    { NAT_CACHE_NATC_FLOW_CNTR_CNTL_REG_OFFSET },
    0,
    0,
    1656,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    NAT_CACHE_NATC_FLOW_CNTR_CNTL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG, TYPE: Type_NATC_BLOCK_NATCACHE_INDIR_RW_NATC_INDIR_ADDR_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_ENTRY *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD =
{
    "NATC_ENTRY",
#if RU_INCLUDE_DESC
    "",
    "NAT Cache Entry number.\n\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: W_R *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_W_R_FIELD =
{
    "W_R",
#if RU_INCLUDE_DESC
    "",
    "NAT Cache Memory and Statics Memory Transaction.1 : NAT Cache Memory and Statics Memory Write.0 : NAT Cache Memory and Statics Memory Read.\n\nNote that after setting NAT_CACHE.NATC_BLOCK.NATCACHE_INDIR_RW.NATC_INDIR_ADDR_REG.w_r = 1\nfor the write, the FW needs to set this field to 0x0!\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_W_R_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_W_R_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_W_R_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_R1_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_R1_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_FIELDS[] =
{
    &NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD,
    &NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_W_R_FIELD,
    &NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG *****/
const ru_reg_rec NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_REG =
{
    "INDIR_RW_NATC_INDIR_ADDR_REG",
#if RU_INCLUDE_DESC
    "NATC_INDIR_ADDR_REG Register",
    "No Description\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_REG_OFFSET },
    0,
    0,
    1657,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG, TYPE: Type_NATC_BLOCK_NATCACHE_INDIR_RW_NATC_INDIR_DATA_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits[31:0].-------------------------------------------------------For NAT Cache nd Statics Memory write operation,first, write all the data to Indirect Data Registers[N-1:0],N is number of words including key, result, hit count and byte count.Indirect Data Register[1:0] are for Statics Memory.Indirect Data register[0] is for hit count and 4 lsb of byte count.Indirect Data register[0] bits 27:0 are 28-bit hit count.Indirect Data register[0] bits 31:28 are 4 lsb of 36-bit byte count.Indirect Data Register[1] is for 32 msb of 36-bit byte count.{Indirect Data Register[1], Indirect Data register[0][31:28]} is the 36-bit byte count.indirect Data register [N-1:2] are for NAT Cache Memory (key and result), key is first,followed by result, followed by {ddr_miss, nat_ddr_bin, nat_tbl}then followed by a write to Indirect Address Register to set upNAT Cache Entry Number and W_R bit to 1, this will initiate the write operation.--------------------------------------------------------For NAT Cache Memory and statics Memory read operation,first, write to Indirect Address Register to set upNAT Cache Entry Number and W_R bit to 0, this will initiate the read operation.the read data from NAT Cache Memory and statics Memory will be loaded into Indirect Data Registers[N-1:0].then followed by read from Indirect Data Registers[N-1:0] for all data.\n\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_DATA_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_DATA_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_FIELDS[] =
{
    &NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG *****/
const ru_reg_rec NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_REG =
{
    "INDIR_RW_NATC_INDIR_DATA_REG",
#if RU_INCLUDE_DESC
    "NATC_INDIR_DATA_REG 0..40 Register",
    "No Description\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_REG_OFFSET },
    NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_REG_RAM_CNT,
    4,
    1658,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG, TYPE: Type_NATC_BLOCK_NATCACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_ENTRY *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD =
{
    "FLOW_CNTR_ENTRY",
#if RU_INCLUDE_DESC
    "",
    "Flow counter address.\n\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R2_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R2_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: W_R *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD =
{
    "W_R",
#if RU_INCLUDE_DESC
    "",
    "Flow counter Transaction.1 : Flow counter write.0 : Flow counter read.\n\nNote that after setting NAT_CACHE.NATC_BLOCK.NATCACHE_INDIR_RW.NATC_FLOW_CNTR_INDIR_ADDR_REG.w_r = 1\nfor the write, the FW needs to set this field to 0x0!\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R1_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R1_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_FIELDS[] =
{
    &NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD,
    &NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R2_FIELD,
    &NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD,
    &NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG *****/
const ru_reg_rec NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_REG =
{
    "INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG",
#if RU_INCLUDE_DESC
    "NATC_FLOW_CNTR_INDIR_ADDR_REG Register",
    "No Description\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_REG_OFFSET },
    0,
    0,
    1659,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG, TYPE: Type_NATC_BLOCK_NATCACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Flow Counter indirect register access data register, bits[31:0].-------------------------------------------------------For Flow Counter write operation,first, write all the data to Flow Counter Indirect Data Registers[1:0],Flow Counter Indirect Data register[0] is for flow Counter hit count and 4 lsb of Flow Counter byte count.Flow Counter Indirect Data register[0] bits 27:0 are 28-bit Flow Counter hit count.Flow Counter Indirect Data register[0] bits 31:28 are 4 lsb of 36-bit Flow Counter byte count.Flow Counter Indirect Data Register[1] is for 32 msb of 36-bit Flow Counter byte count.{Flow Counter Indirect Data Register[1], Flow Counter Indirect Data register[0][31:28]} is the 36-bit Flow Counter byte count.then followed by a write to Flow Counter Indirect Address Register to set upFlow Counter adress and W_R bit to 1, this will initiate the write operation.--------------------------------------------------------For Flow Counter read operation,first, write to Flow Counter Indirect Address Register to set upFlow Counter address and W_R bit to 0, this will initiate the read operation.the read data from Flow Counter will be loaded into Flow Counter Indirect Data Registers[1:0].then followed by read from Flow Counter Indirect Data Registers[1:0] for all data.\n\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_MASK },
    0,
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_WIDTH },
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_FIELDS[] =
{
    &NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG *****/
const ru_reg_rec NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_REG =
{
    "INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG",
#if RU_INCLUDE_DESC
    "NATC_FLOW_CNTR_INDIR_DATA_REG 0..1 Register",
    "No Description\n",
#endif
    { NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_REG_OFFSET },
    NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_REG_RAM_CNT,
    4,
    1660,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_FIELDS,
#endif
};

unsigned long NAT_CACHE_ADDRS[] =
{
    0xB4B00000,
    0xB4B20000,
};

static const ru_reg_rec *NAT_CACHE_REGS[] =
{
    &NAT_CACHE_NATC_CONTROL_STATUS_REG,
    &NAT_CACHE_NATC_CONTROL_STATUS2_REG,
    &NAT_CACHE_NATC_TABLE_CONTROL_REG,
    &NAT_CACHE_NAT0_COMMAND_STATUS_REG,
    &NAT_CACHE_NAT0_HASH_REG,
    &NAT_CACHE_NAT0_HIT_COUNT_REG,
    &NAT_CACHE_NAT0_BYTE_COUNT_REG,
    &NAT_CACHE_NAT0_PKT_LEN_REG,
    &NAT_CACHE_NAT0_KEY_RESULT_REG,
    &NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NAT_CACHE_NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_0_REG,
    &NAT_CACHE_NATC_STAT_COUNTER_CONTROL_1_REG,
    &NAT_CACHE_NATC_CACHE_HIT_COUNT_REG,
    &NAT_CACHE_NATC_CACHE_MISS_COUNT_REG,
    &NAT_CACHE_NATC_DDR_REQUEST_COUNT_REG,
    &NAT_CACHE_NATC_DDR_EVICT_COUNT_REG,
    &NAT_CACHE_NATC_DDR_BLOCK_COUNT_REG,
    &NAT_CACHE_NATC_DDR_SIZE_REG,
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_0_REG,
    &NAT_CACHE_NATC_DDR_BINS_PER_BUCKET_1_REG,
    &NAT_CACHE_NATC_TOTAL_LEN_REG,
    &NAT_CACHE_NATC_SM_STATUS_REG,
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_0_REG,
    &NAT_CACHE_NATC_REGFILE_FIFO_START_ADDR_1_REG,
    &NAT_CACHE_NATC_TBL0_KEY_MASK_REG,
    &NAT_CACHE_NATC_TBL1_KEY_MASK_REG,
    &NAT_CACHE_NATC_TBL2_KEY_MASK_REG,
    &NAT_CACHE_NATC_TBL3_KEY_MASK_REG,
    &NAT_CACHE_NATC_TBL4_KEY_MASK_REG,
    &NAT_CACHE_NATC_TBL5_KEY_MASK_REG,
    &NAT_CACHE_NATC_TBL6_KEY_MASK_REG,
    &NAT_CACHE_NATC_TBL7_KEY_MASK_REG,
    &NAT_CACHE_NATC_DDR_HASH_MODE_REG,
    &NAT_CACHE_NATC_FLOW_CNTR_CNTL_REG,
    &NAT_CACHE_INDIR_RW_NATC_INDIR_ADDR_REG_REG,
    &NAT_CACHE_INDIR_RW_NATC_INDIR_DATA_REG_REG,
    &NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG_REG,
    &NAT_CACHE_INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG_REG,
};

const ru_block_rec NAT_CACHE_BLOCK =
{
    "NAT_CACHE",
#if RU_INCLUDE_SIZE
    { 0x000007D8 },
#endif
    NAT_CACHE_ADDRS,
    2,
    69,
    NAT_CACHE_REGS,
};
