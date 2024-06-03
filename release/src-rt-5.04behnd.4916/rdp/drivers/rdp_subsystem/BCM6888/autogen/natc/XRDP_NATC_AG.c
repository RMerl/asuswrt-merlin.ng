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


#include "XRDP_NATC_AG.h"

/******************************************************************************
 * Register: NAME: NATC_CONTROL_STATUS, TYPE: Type_NATC_CONTROL_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_ENABLE *****/
const ru_field_rec NATC_CONTROL_STATUS_DDR_ENABLE_FIELD =
{
    "DDR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enables NAT table offload to DDR functionality.\nNATC_CONTROL_STATUS2 register should be configured before enabling this feature.\n",
#endif
    { NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_ADD_COMMAND_SPEEDUP_MODE *****/
const ru_field_rec NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD =
{
    "NATC_ADD_COMMAND_SPEEDUP_MODE",
#if RU_INCLUDE_DESC
    "",
    "Default behavior for an ADD command is to do a LOOKUP first to see if the entry\nwith the same key already exists and replace it; this is to avoid having duplicated\nentries in the table for ADD command.  When this bit is set an ADD command will\neither replace the entry with the matched key or add an entry to an empty entry\ndepending on whichever one is encountered first during multi-hash.  Enabling\nthis bit speeds up the ADD command.\n",
#endif
    { NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED0 *****/
const ru_field_rec NATC_CONTROL_STATUS_UNUSED0_FIELD =
{
    "UNUSED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { NATC_CONTROL_STATUS_UNUSED0_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_UNUSED0_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_UNUSED0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_INIT_DONE *****/
const ru_field_rec NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD =
{
    "NATC_INIT_DONE",
#if RU_INCLUDE_DESC
    "",
    "This bit is set to 1 when NATC cache memories have been initialized to 0's.\n",
#endif
    { NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_64BIT_IN_128BIT_SWAP_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD =
{
    "DDR_64BIT_IN_128BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 64-bit word within 128-bit word for DDR memory read/write accesses\n(i.e., [127:0] becomes {[63:0], [127:64]}).\n",
#endif
    { NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_32BIT_IN_64BIT_SWAP_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "SMEM_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for statistics (counter) memory accesses\n(i.e., [63:0] becomes {[31:0], [63:32]})\n",
#endif
    { NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_8BIT_IN_32BIT_SWAP_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "SMEM_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for statistics (counter) memory accesses\n(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})\n",
#endif
    { NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SWAP_ALL_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD =
{
    "DDR_SWAP_ALL_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap all bytes on DDR interface.\n",
#endif
    { NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REPEATED_KEY_DET_EN *****/
const ru_field_rec NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD =
{
    "REPEATED_KEY_DET_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable repeated key detection to improve cache lookup performance for repeated key.\n",
#endif
    { NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REG_32BIT_IN_64BIT_SWAP_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "REG_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for key_result register accesses\n(i.e., [63:0] becomes {[31:0], [63:32]})\n",
#endif
    { NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REG_8BIT_IN_32BIT_SWAP_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "REG_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for key_result register accesses\n(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})\n",
#endif
    { NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_PENDING_HASH_MODE *****/
const ru_field_rec NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD =
{
    "DDR_PENDING_HASH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used to detect DDR pending operations.\n0h: 32-bit rolling XOR hash is used as cache hash function.\n1h: CRC32 hash is used as cache hash function.\n2h: CRC32 hash is used as cache hash function.\n3h: CRC32 hash is used as cache hash function.\n4h: RSS hash is used as cache hash function using secret key 0.\n5h: RSS hash is used as cache hash function using secret key 1.\n6h: RSS hash is used as cache hash function using secret key 2.\n7h: RSS hash is used as cache hash function using secret key 3.\n",
#endif
    { NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PENDING_FIFO_ENTRY_CHECK_ENABLE *****/
const ru_field_rec NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD =
{
    "PENDING_FIFO_ENTRY_CHECK_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "This bit disables caching DDR miss entry function when there is no additional lookup having\nthe same key in pending fifo (e.g., DDR miss entry is cached only if there are 2 or more lookup\nof the same key within a 32 lookup window).\nThis is to reduce excessive caching of miss entries.\nThis bit is only valid when CACHE_UPDATE_ON_DDR_MISS bit is set to 1.\n1h: Enable; miss entry fetched from DDR will be cached if pending FIFO\ncontains the same lookup having the same hash value as miss entry.\n0h: Disable; miss entry fetched from DDR will always be cached.\n",
#endif
    { NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_UPDATE_ON_DDR_MISS *****/
const ru_field_rec NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD =
{
    "CACHE_UPDATE_ON_DDR_MISS",
#if RU_INCLUDE_DESC
    "",
    "This bit enables caching for miss entry\n1h: Enable; miss entry in both cache and DDR will be cached.\n0h: Disable; miss entry in both cache and DDR will be not cached.\n",
#endif
    { NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_DISABLE_ON_REG_LOOKUP *****/
const ru_field_rec NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD =
{
    "DDR_DISABLE_ON_REG_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "This bit prevents register interface lookup to access DDR\n0h: Enable register interface lookup in DDR.\n1h: Disable register interface lookup in DDR.\nRegister interface lookup will only return lookup results in Cache.\n",
#endif
    { NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_HASH_MODE *****/
const ru_field_rec NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD =
{
    "NAT_HASH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for internal caching\n0h: 32-bit rolling XOR hash is used as cache hash function.\n1h: CRC32 hash is used as cache hash function. CRC32 is reduced to N-bit using\nthe same method as in 32-bit rolling XOR hash.\n2h: CRC32 hash is used as cache hash function. CRC32[N:0] is used as hash value.\n3h: CRC32 hash is used as cache hash function. CRC32[31:N] is used as hash value.\n4h: RSS hash is used as cache hash function using secret key 0. RSS[N:0] is used as hash value.\n5h: RSS hash is used as cache hash function using secret key 1. RSS[N:0] is used as hash value.\n6h: RSS hash is used as cache hash function using secret key 2. RSS[N:0] is used as hash value.\n7h: RSS hash is used as cache hash function using secret key 3. RSS[N:0] is used as hash value.\n",
#endif
    { NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_HASH_LIMIT *****/
const ru_field_rec NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD =
{
    "MULTI_HASH_LIMIT",
#if RU_INCLUDE_DESC
    "",
    "Maximum number of multi-hash iterations.\nThis is not used if cache size is 32 cache entries or less.\nValue of 0 is 1 iteration, 1 is 2 iterations, 2 is 3 iterations, etc.\nThis is not used if cache size is 32 entries or less.\n",
#endif
    { NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_SHIFT },
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DECR_COUNT_WRAPAROUND_ENABLE *****/
const ru_field_rec NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD =
{
    "DECR_COUNT_WRAPAROUND_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Decrement Count Wraparound Enable\n0h: Do not decrement counters for decrement command when counters reach 0\n1h: Always decrement counters for decrement command; will wrap around from 0 to all 1's\n",
#endif
    { NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_ARB_ST *****/
const ru_field_rec NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD =
{
    "NAT_ARB_ST",
#if RU_INCLUDE_DESC
    "",
    "NAT Arbitration Mechanism\n",
#endif
    { NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_SMEM_INCREMENT_ON_REG_LOOKUP *****/
const ru_field_rec NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD =
{
    "NATC_SMEM_INCREMENT_ON_REG_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "Enables incrementing or decrementing hit counter by 1 and byte counter by PKT_LEN\non successful lookups using register interface\nBY default, counters only increment on successful lookups on Runner interface\n",
#endif
    { NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_SMEM_CLEAR_BY_UPDATE_DISABLE *****/
const ru_field_rec NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD =
{
    "NATC_SMEM_CLEAR_BY_UPDATE_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables clearing counters when an existing entry is replaced by ADD command\n",
#endif
    { NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_RESET *****/
const ru_field_rec NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD =
{
    "REGFILE_FIFO_RESET",
#if RU_INCLUDE_DESC
    "",
    "Reset regfile_FIFO and ddr pending memory.\n",
#endif
    { NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_ENABLE *****/
const ru_field_rec NATC_CONTROL_STATUS_NATC_ENABLE_FIELD =
{
    "NATC_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enables all NATC state machines and input FIFO;\nClearing this bit will halt all state machines gracefully to idle states,\nall outstanding transactions in the FIFO will remain in the FIFO and NATC\nwill stop accepting new commands;  All configuration registers should be\nconfigured before enabling this bit.\n",
#endif
    { NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_RESET *****/
const ru_field_rec NATC_CONTROL_STATUS_NATC_RESET_FIELD =
{
    "NATC_RESET",
#if RU_INCLUDE_DESC
    "",
    "Self Clearing Block Reset (including resetting all registers to default values)\n",
#endif
    { NATC_CONTROL_STATUS_NATC_RESET_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS_NATC_RESET_FIELD_WIDTH },
    { NATC_CONTROL_STATUS_NATC_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CONTROL_STATUS_FIELDS[] =
{
    &NATC_CONTROL_STATUS_DDR_ENABLE_FIELD,
    &NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD,
    &NATC_CONTROL_STATUS_UNUSED0_FIELD,
    &NATC_CONTROL_STATUS_NATC_INIT_DONE_FIELD,
    &NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD,
    &NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD,
    &NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD,
    &NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD,
    &NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD,
    &NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD,
    &NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD,
    &NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD,
    &NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD,
    &NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD,
    &NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD,
    &NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD,
    &NATC_CONTROL_STATUS_NATC_ENABLE_FIELD,
    &NATC_CONTROL_STATUS_NATC_RESET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_CONTROL_STATUS *****/
const ru_reg_rec NATC_CONTROL_STATUS_REG =
{
    "CONTROL_STATUS",
#if RU_INCLUDE_DESC
    "NAT Cache Control and Status Register",
    "NAT Cache Control and Status Register.\n",
#endif
    { NATC_CONTROL_STATUS_REG_OFFSET },
    0,
    0,
    618,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    24,
    NATC_CONTROL_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_CONTROL_STATUS2, TYPE: Type_NATC_CONTROL_STATUS2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED3 *****/
const ru_field_rec NATC_CONTROL_STATUS2_UNUSED3_FIELD =
{
    "UNUSED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { NATC_CONTROL_STATUS2_UNUSED3_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_UNUSED3_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_UNUSED3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_32BIT_IN_64BIT_SWAP_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "DDR_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for DDR memory read/write accesses\n(i.e., [63:0] becomes {[31:0], [63:32]}).\n",
#endif
    { NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_8BIT_IN_32BIT_SWAP_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "DDR_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for DDR memory read/write accesses\n(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]}).\n",
#endif
    { NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_LOOKUP_BLOCKING_MODE *****/
const ru_field_rec NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD =
{
    "CACHE_LOOKUP_BLOCKING_MODE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1\n",
#endif
    { NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGE_TIMER_TICK *****/
const ru_field_rec NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD =
{
    "AGE_TIMER_TICK",
#if RU_INCLUDE_DESC
    "",
    "Timer tick for pseudo-LRU\n",
#endif
    { NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGE_TIMER *****/
const ru_field_rec NATC_CONTROL_STATUS2_AGE_TIMER_FIELD =
{
    "AGE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timer value used for pseudo-LRU;\nWhen timer fires the 8-bit age value of every entry in the cache is\ndecremented (cap at 0).  The entry with lower value is\nthe older entry.  The default setting keeps track of ~0.26s age at\n~1ms resolution.\n0: 1 tick\n1: 2 ticks\n2: 4 ticks\n3: 8 ticks\n4: 16 ticks\n..\n..\n31: 2^31 TICKS\n",
#endif
    { NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_SHIFT },
    19,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_ALGO *****/
const ru_field_rec NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD =
{
    "CACHE_ALGO",
#if RU_INCLUDE_DESC
    "",
    "Replacement algorithm for caching\nLowest-multi-hash-iteration number is used to select the final replacement\nentry if multiple entries were chosen by the selected algorithm.  For\ninstance, if HIT_COUNT algorithm were selected, and 2nd, 3rd and 7th\nentry all have the same hit_count values, 2nd entry will be evicted.\n",
#endif
    { NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED2 *****/
const ru_field_rec NATC_CONTROL_STATUS2_UNUSED2_FIELD =
{
    "UNUSED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { NATC_CONTROL_STATUS2_UNUSED2_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_UNUSED2_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_UNUSED2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED1 *****/
const ru_field_rec NATC_CONTROL_STATUS2_UNUSED1_FIELD =
{
    "UNUSED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { NATC_CONTROL_STATUS2_UNUSED1_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_UNUSED1_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_UNUSED1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_UPDATE_ON_REG_DDR_LOOKUP *****/
const ru_field_rec NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD =
{
    "CACHE_UPDATE_ON_REG_DDR_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "This bit determines whether register interface lookup will cache the entry from DDR\n1h: Enable; entry fetched from DDR will be cached using register interface lookup command\n0h: Disable; entry fetched from DDR will not be cached using register interface lookup command\n",
#endif
    { NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL *****/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for DDR counters on read/write accesses.\n(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})\n",
#endif
    { NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HASH_SWAP *****/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD =
{
    "DDR_HASH_SWAP",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 18-bit DDR hash value\n",
#endif
    { NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE *****/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD =
{
    "DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1\n",
#endif
    { NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE *****/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD =
{
    "DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1\n",
#endif
    { NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED4 *****/
const ru_field_rec NATC_CONTROL_STATUS2_UNUSED4_FIELD =
{
    "UNUSED4",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { NATC_CONTROL_STATUS2_UNUSED4_FIELD_MASK },
    0,
    { NATC_CONTROL_STATUS2_UNUSED4_FIELD_WIDTH },
    { NATC_CONTROL_STATUS2_UNUSED4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CONTROL_STATUS2_FIELDS[] =
{
    &NATC_CONTROL_STATUS2_UNUSED3_FIELD,
    &NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD,
    &NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD,
    &NATC_CONTROL_STATUS2_AGE_TIMER_FIELD,
    &NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD,
    &NATC_CONTROL_STATUS2_UNUSED2_FIELD,
    &NATC_CONTROL_STATUS2_UNUSED1_FIELD,
    &NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD,
    &NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD,
    &NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD,
    &NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD,
    &NATC_CONTROL_STATUS2_UNUSED4_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_CONTROL_STATUS2 *****/
const ru_reg_rec NATC_CONTROL_STATUS2_REG =
{
    "CONTROL_STATUS2",
#if RU_INCLUDE_DESC
    "NAT Cache Control and Status Register2",
    "NAT Cache Control and Status Register\n",
#endif
    { NATC_CONTROL_STATUS2_REG_OFFSET },
    0,
    0,
    619,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    NATC_CONTROL_STATUS2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_TABLE_CONTROL, TYPE: Type_NATC_TABLE_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL7 *****/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD =
{
    "SMEM_DIS_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 7\n",
#endif
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL6 *****/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD =
{
    "SMEM_DIS_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 6\n",
#endif
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL5 *****/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD =
{
    "SMEM_DIS_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 5\n",
#endif
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL4 *****/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD =
{
    "SMEM_DIS_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 4\n",
#endif
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL3 *****/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD =
{
    "SMEM_DIS_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 3\n",
#endif
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL2 *****/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD =
{
    "SMEM_DIS_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 2\n",
#endif
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL1 *****/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD =
{
    "SMEM_DIS_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 1\n",
#endif
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMEM_DIS_TBL0 *****/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD =
{
    "SMEM_DIS_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 0\n",
#endif
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL7 *****/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 7\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n",
#endif
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL6 *****/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 6\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n",
#endif
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL5 *****/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 5\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n",
#endif
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL4 *****/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 4\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n",
#endif
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL3 *****/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 3\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n",
#endif
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL2 *****/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 2\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n",
#endif
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL1 *****/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 1\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n",
#endif
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAR_CONTEXT_LEN_EN_TBL0 *****/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 0\nlowest 4 bits of key[3:0] is used to indicate the context length\n0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes\nNote that key length is reduced by 4 bit\n0h: Disable variable context length\n1h: Enable variable context length\n",
#endif
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL7 *****/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD =
{
    "KEY_LEN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 7\n0h: 16-byte key\n1h: 32-byte key\n",
#endif
    { NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL7 *****/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD =
{
    "NON_CACHEABLE_TBL7",
#if RU_INCLUDE_DESC
    "",
    "DDR table 7 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n",
#endif
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL6 *****/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD =
{
    "KEY_LEN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 6\n0h: 16-byte key\n1h: 32-byte key\n",
#endif
    { NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL6 *****/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD =
{
    "NON_CACHEABLE_TBL6",
#if RU_INCLUDE_DESC
    "",
    "DDR table 6 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n",
#endif
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL5 *****/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD =
{
    "KEY_LEN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 5\n0h: 16-byte key\n1h: 32-byte key\n",
#endif
    { NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL5 *****/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD =
{
    "NON_CACHEABLE_TBL5",
#if RU_INCLUDE_DESC
    "",
    "DDR table 5 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n",
#endif
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL4 *****/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD =
{
    "KEY_LEN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 4\n0h: 16-byte key\n1h: 32-byte key\n",
#endif
    { NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL4 *****/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD =
{
    "NON_CACHEABLE_TBL4",
#if RU_INCLUDE_DESC
    "",
    "DDR table 4 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n",
#endif
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL3 *****/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD =
{
    "KEY_LEN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 3\n0h: 16-byte key\n1h: 32-byte key\n",
#endif
    { NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL3 *****/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD =
{
    "NON_CACHEABLE_TBL3",
#if RU_INCLUDE_DESC
    "",
    "DDR table 3 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n",
#endif
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL2 *****/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD =
{
    "KEY_LEN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 2\n0h: 16-byte key\n1h: 32-byte key\n",
#endif
    { NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL2 *****/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD =
{
    "NON_CACHEABLE_TBL2",
#if RU_INCLUDE_DESC
    "",
    "DDR table 2 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n",
#endif
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL1 *****/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD =
{
    "KEY_LEN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 1\n0h: 16-byte key\n1h: 32-byte key\n",
#endif
    { NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL1 *****/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD =
{
    "NON_CACHEABLE_TBL1",
#if RU_INCLUDE_DESC
    "",
    "DDR table 1 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n",
#endif
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_LEN_TBL0 *****/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD =
{
    "KEY_LEN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 0\n0h: 16-byte key\n1h: 32-byte key\n",
#endif
    { NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_CACHEABLE_TBL0 *****/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD =
{
    "NON_CACHEABLE_TBL0",
#if RU_INCLUDE_DESC
    "",
    "DDR table 0 non-cacheable control\n0h: DDR table is cached\n1h: DDR table is not cached; counters are updated in DDR directly\n",
#endif
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_MASK },
    0,
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_WIDTH },
    { NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_TABLE_CONTROL_FIELDS[] =
{
    &NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD,
    &NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD,
    &NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD,
    &NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD,
    &NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD,
    &NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD,
    &NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD,
    &NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD,
    &NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD,
    &NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD,
    &NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD,
    &NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD,
    &NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD,
    &NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD,
    &NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD,
    &NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD,
    &NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD,
    &NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD,
    &NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD,
    &NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD,
    &NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD,
    &NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD,
    &NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD,
    &NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD,
    &NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD,
    &NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD,
    &NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD,
    &NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD,
    &NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD,
    &NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD,
    &NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD,
    &NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_TABLE_CONTROL *****/
const ru_reg_rec NATC_TABLE_CONTROL_REG =
{
    "TABLE_CONTROL",
#if RU_INCLUDE_DESC
    "NAT Cache Table Control Register",
    "NAT Cache Table Control Register\n",
#endif
    { NATC_TABLE_CONTROL_REG_OFFSET },
    0,
    0,
    620,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    NATC_TABLE_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_STAT_COUNTER_CONTROL_0, TYPE: Type_NATC_STAT_COUNTER_CONTROL_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_EVICT_COUNT_EN *****/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD =
{
    "DDR_EVICT_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR evict counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable DDR evict counter increment on ddr evict.\n1h: Enable DDR evict counter increment on ddr evict.\n",
#endif
    { NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_MASK },
    0,
    { NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_WIDTH },
    { NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REQUEST_COUNT_EN *****/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD =
{
    "DDR_REQUEST_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR request counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable DDR request counter increment on ddr request.\n1h: Enable DDR request counter increment on ddr request.\n",
#endif
    { NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_MASK },
    0,
    { NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_WIDTH },
    { NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_MISS_COUNT_EN *****/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD =
{
    "CACHE_MISS_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "Cache miss counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable cache miss counter increment on cache miss.\n1h: Enable cache miss counter increment on cache miss.\n",
#endif
    { NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_MASK },
    0,
    { NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_WIDTH },
    { NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_HIT_COUNT_EN *****/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD =
{
    "CACHE_HIT_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "Cache hit counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable cache hit counter increment on cache hit.\n1h: Enable cache hit counter increment on cache hit.\n",
#endif
    { NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_MASK },
    0,
    { NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_WIDTH },
    { NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_STAT_COUNTER_CONTROL_0_FIELDS[] =
{
    &NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD,
    &NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD,
    &NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD,
    &NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_STAT_COUNTER_CONTROL_0 *****/
const ru_reg_rec NATC_STAT_COUNTER_CONTROL_0_REG =
{
    "STAT_COUNTER_CONTROL_0",
#if RU_INCLUDE_DESC
    "NAT cache stat counter control register 0",
    "NAT cache stat counter control register 0\n",
#endif
    { NATC_STAT_COUNTER_CONTROL_0_REG_OFFSET },
    0,
    0,
    621,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NATC_STAT_COUNTER_CONTROL_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_STAT_COUNTER_CONTROL_1, TYPE: Type_NATC_STAT_COUNTER_CONTROL_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_WRAPAROUND_DIS *****/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD =
{
    "COUNTER_WRAPAROUND_DIS",
#if RU_INCLUDE_DESC
    "",
    "Counter wraparound disable\nThis applies to all stat counters defined in\nNATC_STAT_COUNTER_CONTROL_0 and  NATC_STAT_COUNTER_CONTROL_1.\n0h: The counter will wraparound to 0 after it reaches ffffffffh.\n1h: The counter will cap at ffffffffh.\n",
#endif
    { NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_MASK },
    0,
    { NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_WIDTH },
    { NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BLOCK_COUNT_EN *****/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD =
{
    "DDR_BLOCK_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR block counter enable\nEach bit enables/disables the counter increment based on a DDR table,\nbit 7 for DDR table 7..... bit 0 for DDR table 0.\n0h: Disable DDR block counter increment on ddr block.\n1h: Enable DDR block counter increment on ddr block.\n",
#endif
    { NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_MASK },
    0,
    { NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_WIDTH },
    { NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_STAT_COUNTER_CONTROL_1_FIELDS[] =
{
    &NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD,
    &NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_STAT_COUNTER_CONTROL_1 *****/
const ru_reg_rec NATC_STAT_COUNTER_CONTROL_1_REG =
{
    "STAT_COUNTER_CONTROL_1",
#if RU_INCLUDE_DESC
    "NAT cache stat counter control register 1",
    "NAT cache stat counter control register 1\n",
#endif
    { NATC_STAT_COUNTER_CONTROL_1_REG_OFFSET },
    0,
    0,
    622,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NATC_STAT_COUNTER_CONTROL_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_REGFILE_FIFO_START_ADDR_0, TYPE: Type_NATC_REGFILE_FIFO_START_ADDR_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_3 *****/
const ru_field_rec NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD =
{
    "REGFILE_FIFO_START_ADDR_3",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 2 Start Address\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD_MASK },
    0,
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD_WIDTH },
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD_SHIFT },
    26,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_2 *****/
const ru_field_rec NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD =
{
    "REGFILE_FIFO_START_ADDR_2",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 2 Start Address\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD_MASK },
    0,
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD_WIDTH },
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD_SHIFT },
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_1 *****/
const ru_field_rec NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD =
{
    "REGFILE_FIFO_START_ADDR_1",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 1 Start Address\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD_MASK },
    0,
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD_WIDTH },
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD_SHIFT },
    6,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_0 *****/
const ru_field_rec NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD =
{
    "REGFILE_FIFO_START_ADDR_0",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 0 Start Address\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD_MASK },
    0,
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD_WIDTH },
    { NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_REGFILE_FIFO_START_ADDR_0_FIELDS[] =
{
    &NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_3_FIELD,
    &NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_2_FIELD,
    &NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_1_FIELD,
    &NATC_REGFILE_FIFO_START_ADDR_0_REGFILE_FIFO_START_ADDR_0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_REGFILE_FIFO_START_ADDR_0 *****/
const ru_reg_rec NATC_REGFILE_FIFO_START_ADDR_0_REG =
{
    "REGFILE_FIFO_START_ADDR_0",
#if RU_INCLUDE_DESC
    "REGFILE FIFO Start Address register 0",
    "REGFILE FIFO Start Address register 0\nActual FIFO size is 2 more than the number programmed in\nthis register due to input and output holder registers\nwhich account for 2 additional depth.\nThe actual FIFO 0 (DDR_KEY_REQ_FIFO) size is\nREGFILE_FIFO_START_ADDR_1 -  REGFILE_FIFO_START_ADDR_0 + 2.\nThe actual FIFO 1 (DDR_RESULT_REQ_FIFO) size is\nREGFILE_FIFO_START_ADDR_2 -  REGFILE_FIFO_START_ADDR_1 + 2.\nThe actual FIFO 2 (DDR_KEY_REQ_PIPE) size is\nREGFILE_FIFO_START_ADDR_3 -  REGFILE_FIFO_START_ADDR_2 + 2.\nThe actual FIFO 3 (BLOCKING_PENDING_FIFO) size is\nREGFILE_FIFO_START_ADDR_4 -  REGFILE_FIFO_START_ADDR_3 + 2.\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_0_REG_OFFSET },
    0,
    0,
    623,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NATC_REGFILE_FIFO_START_ADDR_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_REGFILE_FIFO_START_ADDR_1, TYPE: Type_NATC_REGFILE_FIFO_START_ADDR_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_7 *****/
const ru_field_rec NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD =
{
    "REGFILE_FIFO_START_ADDR_7",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 7 Start Address-- Note that this entry is not used\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD_MASK },
    0,
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD_WIDTH },
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD_SHIFT },
    74,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_6 *****/
const ru_field_rec NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD =
{
    "REGFILE_FIFO_START_ADDR_6",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 6 Start Address\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD_MASK },
    0,
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD_WIDTH },
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD_SHIFT },
    68,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_5 *****/
const ru_field_rec NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD =
{
    "REGFILE_FIFO_START_ADDR_5",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 5 Start Address\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD_MASK },
    0,
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD_WIDTH },
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD_SHIFT },
    62,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REGFILE_FIFO_START_ADDR_4 *****/
const ru_field_rec NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD =
{
    "REGFILE_FIFO_START_ADDR_4",
#if RU_INCLUDE_DESC
    "",
    "REGFILE FIFO 4 Start Address\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD_MASK },
    0,
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD_WIDTH },
    { NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD_SHIFT },
    56,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_REGFILE_FIFO_START_ADDR_1_FIELDS[] =
{
    &NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_7_FIELD,
    &NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_6_FIELD,
    &NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_5_FIELD,
    &NATC_REGFILE_FIFO_START_ADDR_1_REGFILE_FIFO_START_ADDR_4_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_REGFILE_FIFO_START_ADDR_1 *****/
const ru_reg_rec NATC_REGFILE_FIFO_START_ADDR_1_REG =
{
    "REGFILE_FIFO_START_ADDR_1",
#if RU_INCLUDE_DESC
    "REGFILE FIFO Start Address register 1",
    "REGFILE FIFO Start Address register 1\nActual FIFO size is 2 more than the number programmed in\nthis register due to input and output holder registers\nwhich account for 2 additional depth.\nThe delta between REGFILE_FIFO_START_ADDR_4 and REGFILE_FIFO_START_ADDR_5,\nREGFILE_FIFO_START_ADDR_5 and REGFILE_FIFO_START_ADDR_6,\nREGFILE_FIFO_START_ADDR_6 and REGFILE_FIFO_START_ADDR_7\nneed to be identical since these are used for the same wide FIFO.\nThe actual FIFO 4 (DDR_WRITE_RESULT_FIFO) size is\nREGFILE_FIFO_START_ADDR_5 -  REGFILE_FIFO_START_ADDR_4 + 2.\nThe actual FIFO 5 (DDR_WRITE_RESULT_FIFO) size is\nREGFILE_FIFO_START_ADDR_6 -  REGFILE_FIFO_START_ADDR_5 + 2.\nThe actual FIFO 6 (DDR_WRITE_RESULT_FIFO) size is\nREGFILE_FIFO_START_ADDR_7 -  REGFILE_FIFO_START_ADDR_6 + 2.\nThe actual FIFO 7 size is the same as FIFO 4, 5, 6\n",
#endif
    { NATC_REGFILE_FIFO_START_ADDR_1_REG_OFFSET },
    0,
    0,
    624,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NATC_REGFILE_FIFO_START_ADDR_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_FLOW_CNTR_CNTL, TYPE: Type_NATC_FLOW_CNTR_CNTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL7 *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD =
{
    "FLOW_CNTR_EN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 7.\nSee description of FLOW_CNTR_EN_TBL0.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL6 *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD =
{
    "FLOW_CNTR_EN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 6.\nSee description of FLOW_CNTR_EN_TBL0.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL5 *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD =
{
    "FLOW_CNTR_EN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 5.\nSee description of FLOW_CNTR_EN_TBL0.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL4 *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD =
{
    "FLOW_CNTR_EN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 4.\nSee description of FLOW_CNTR_EN_TBL0.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL3 *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD =
{
    "FLOW_CNTR_EN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 3.\nSee description of FLOW_CNTR_EN_TBL0.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL2 *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD =
{
    "FLOW_CNTR_EN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 2.\nSee description of FLOW_CNTR_EN_TBL0.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL1 *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD =
{
    "FLOW_CNTR_EN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 1.\nSee description of FLOW_CNTR_EN_TBL0.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_EN_TBL0 *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD =
{
    "FLOW_CNTR_EN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Enable flow counters for DDR table 0.\nIf this bit is set to 1 and CONTEXT_OFFSET is less than context length - 8 (byte count and hit cout),\nthe flow counter is enabled. Otherwsie the flow counter is disabled.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT_OFFSET *****/
const ru_field_rec NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD =
{
    "CONTEXT_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "This is the byte offset of the context.\nOnly bits 6:0 of this byte in the context are used as flow counter address.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD_MASK },
    0,
    { NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD_WIDTH },
    { NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_FLOW_CNTR_CNTL_FIELDS[] =
{
    &NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL7_FIELD,
    &NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL6_FIELD,
    &NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL5_FIELD,
    &NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL4_FIELD,
    &NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL3_FIELD,
    &NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL2_FIELD,
    &NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL1_FIELD,
    &NATC_FLOW_CNTR_CNTL_FLOW_CNTR_EN_TBL0_FIELD,
    &NATC_FLOW_CNTR_CNTL_CONTEXT_OFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_FLOW_CNTR_CNTL *****/
const ru_reg_rec NATC_FLOW_CNTR_CNTL_REG =
{
    "FLOW_CNTR_CNTL",
#if RU_INCLUDE_DESC
    "NAT Cache Flow Counter Control  Register",
    "NAT Cache Flow Counter Control Register.\n",
#endif
    { NATC_FLOW_CNTR_CNTL_REG_OFFSET },
    0,
    0,
    625,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    NATC_FLOW_CNTR_CNTL_FIELDS,
#endif
};

unsigned long NATC_ADDRS[] =
{
    0x82950000,
};

static const ru_reg_rec *NATC_REGS[] =
{
    &NATC_CONTROL_STATUS_REG,
    &NATC_CONTROL_STATUS2_REG,
    &NATC_TABLE_CONTROL_REG,
    &NATC_STAT_COUNTER_CONTROL_0_REG,
    &NATC_STAT_COUNTER_CONTROL_1_REG,
    &NATC_REGFILE_FIFO_START_ADDR_0_REG,
    &NATC_REGFILE_FIFO_START_ADDR_1_REG,
    &NATC_FLOW_CNTR_CNTL_REG,
};

const ru_block_rec NATC_BLOCK =
{
    "NATC",
    NATC_ADDRS,
    1,
    8,
    NATC_REGS,
};
