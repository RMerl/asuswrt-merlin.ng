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

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: NATC_CONTROL_STATUS_DDR_ENABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_DDR_ENABLE_FIELD =
{
    "DDR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enables NAT table offload to DDR functionality."
    "NATC_CONTROL_STATUS2 register should be configured before enabling this feature.",
#endif
    NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_DDR_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD =
{
    "NATC_ADD_COMMAND_SPEEDUP_MODE",
#if RU_INCLUDE_DESC
    "",
    "Default behavior for an ADD command is to do a LOOKUP first to see if the entry"
    "with the same key already exists and replace it; this is to avoid having duplicated"
    "entries in the table for ADD command.  When this bit is set an ADD command will"
    "either replace the entry with the matched key or add an entry to an empty entry"
    "depending on whichever one is encountered first during multi-hash.  Enabling"
    "this bit speeds up the ADD command.",
#endif
    NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_UNUSED4
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_UNUSED4_FIELD =
{
    "UNUSED4",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS_UNUSED4_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_UNUSED4_FIELD_WIDTH,
    NATC_CONTROL_STATUS_UNUSED4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD =
{
    "DDR_64BIT_IN_128BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 64-bit word within 128-bit word for DDR memory read/write accesses"
    "(i.e., [127:0] becomes {[63:0], [127:64]}).",
#endif
    NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS_DDR_64BIT_IN_128BIT_SWAP_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "SMEM_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for statistics (counter) memory accesses"
    "(i.e., [63:0] becomes {[31:0], [63:32]})",
#endif
    NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "SMEM_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for statistics (counter) memory accesses"
    "(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})",
#endif
    NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD =
{
    "DDR_SWAP_ALL_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap all bytes on DDR interface.",
#endif
    NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD =
{
    "REPEATED_KEY_DET_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable repeated key detection to improve cache lookup performance for repeated key.",
#endif
    NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_WIDTH,
    NATC_CONTROL_STATUS_REPEATED_KEY_DET_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "REG_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for key_result register accesses"
    "(i.e., [63:0] becomes {[31:0], [63:32]})",
#endif
    NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "REG_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for key_result register accesses"
    "(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})",
#endif
    NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD =
{
    "DDR_PENDING_HASH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used to detect DDR pending operations."
    "0h: 32-bit rolling XOR hash is used as cache hash function."
    "1h: CRC32 hash is used as cache hash function."
    "2h: CRC32 hash is used as cache hash function."
    "3h: CRC32 hash is used as cache hash function."
    "4h: RSS hash is used as cache hash function using secret key 0."
    "5h: RSS hash is used as cache hash function using secret key 1."
    "6h: RSS hash is used as cache hash function using secret key 2."
    "7h: RSS hash is used as cache hash function using secret key 3.",
#endif
    NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_DDR_PENDING_HASH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD =
{
    "PENDING_FIFO_ENTRY_CHECK_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "This bit disables caching DDR miss entry function when there is no additional lookup having"
    "the same key in pending fifo (e.g., DDR miss entry is cached only if there are 2 or more lookup"
    "of the same key within a 32 lookup window)."
    "This is to reduce excessive caching of miss entries."
    "This bit is only valid when CACHE_UPDATE_ON_DDR_MISS bit is set to 1."
    "1h: Enable; miss entry fetched from DDR will be cached if pending FIFO"
    "contains the same lookup having the same hash value as miss entry."
    "0h: Disable; miss entry fetched from DDR will always be cached.",
#endif
    NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD =
{
    "CACHE_UPDATE_ON_DDR_MISS",
#if RU_INCLUDE_DESC
    "",
    "This bit enables caching for miss entry"
    "1h: Enable; miss entry in both cache and DDR will be cached."
    "0h: Disable; miss entry in both cache and DDR will be not cached.",
#endif
    NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_WIDTH,
    NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD =
{
    "DDR_DISABLE_ON_REG_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "This bit prevents register interface lookup to access DDR"
    "0h: Enable register interface lookup in DDR."
    "1h: Disable register interface lookup in DDR."
    "Register interface lookup will only return lookup results in Cache.",
#endif
    NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_WIDTH,
    NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_NAT_HASH_MODE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD =
{
    "NAT_HASH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for internal caching"
    "0h: 32-bit rolling XOR hash is used as cache hash function."
    "1h: CRC32 hash is used as cache hash function. CRC32 is reduced to N-bit using"
    "the same method as in 32-bit rolling XOR hash."
    "2h: CRC32 hash is used as cache hash function. CRC32[N:0] is used as hash value."
    "3h: CRC32 hash is used as cache hash function. CRC32[31:N] is used as hash value."
    "4h: RSS hash is used as cache hash function using secret key 0. RSS[N:0] is used as hash value."
    "5h: RSS hash is used as cache hash function using secret key 1. RSS[N:0] is used as hash value."
    "6h: RSS hash is used as cache hash function using secret key 2. RSS[N:0] is used as hash value."
    "7h: RSS hash is used as cache hash function using secret key 3. RSS[N:0] is used as hash value.",
#endif
    NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_MULTI_HASH_LIMIT
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD =
{
    "MULTI_HASH_LIMIT",
#if RU_INCLUDE_DESC
    "",
    "Maximum number of multi-hash iterations."
    "This is not used if cache size is 32 cache entries or less."
    "Value of 0 is 1 iteration, 1 is 2 iterations, 2 is 3 iterations, etc."
    "This is not used if cache size is 32 entries or less.",
#endif
    NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_WIDTH,
    NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD =
{
    "DECR_COUNT_WRAPAROUND_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Decrement Count Wraparound Enable"
    "0h: Do not decrement counters for decrement command when counters reach 0"
    "1h: Always decrement counters for decrement command; will wrap around from 0 to all 1's",
#endif
    NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_NAT_ARB_ST
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD =
{
    "NAT_ARB_ST",
#if RU_INCLUDE_DESC
    "",
    "NAT Arbitration Mechanism"
    "Round-robin arbitration"
    "Strict priority arbitration"
    "listed from highest to lowest priority --  NAT0, NAT1, NAT2, NAT3, Runner"
    "Strict priority arbitration (priority reversed from above)"
    "listed from highest to lowest priority --  Runner, NAT3, NAT2, NAT1, NAT0",
#endif
    NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_WIDTH,
    NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD =
{
    "NATC_SMEM_INCREMENT_ON_REG_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "Enables incrementing or decrementing hit counter by 1 and byte counter by PKT_LEN"
    "on successful lookups using register interface"
    "BY default, counters only increment on successful lookups on Runner interface",
#endif
    NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_WIDTH,
    NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD =
{
    "NATC_SMEM_CLEAR_BY_UPDATE_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables clearing counters when an existing entry is replaced by ADD command",
#endif
    NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_REGFILE_FIFO_RESET
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD =
{
    "REGFILE_FIFO_RESET",
#if RU_INCLUDE_DESC
    "",
    "Reset regfile_FIFO and ddr pending memory.",
#endif
    NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_WIDTH,
    NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_NATC_ENABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NATC_ENABLE_FIELD =
{
    "NATC_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enables all NATC state machines and input FIFO;"
    "Clearing this bit will halt all state machines gracefully to idle states,"
    "all outstanding transactions in the FIFO will remain in the FIFO and NATC"
    "will stop accepting new commands;  All configuration registers should be"
    "configured before enabling this bit.",
#endif
    NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_NATC_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS_NATC_RESET
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NATC_RESET_FIELD =
{
    "NATC_RESET",
#if RU_INCLUDE_DESC
    "",
    "Self Clearing Block Reset (including resetting all registers to default values)",
#endif
    NATC_CONTROL_STATUS_NATC_RESET_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_NATC_RESET_FIELD_WIDTH,
    NATC_CONTROL_STATUS_NATC_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_DDR_HASH_MODE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_HASH_MODE_FIELD =
{
    "DDR_HASH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for DDR lookup"
    "Hash value is DDR table size dependent."
    "0h: 32-bit rolling XOR hash is used as DDR hash function. It is reduced to N-bit"
    "DDR table size is 8K,   N = 13."
    "DDR table size is 16K,  N = 14."
    "DDR table size is 32K,  N = 15."
    "DDR table size is 64K,  N = 16."
    "DDR table size is 128K, N = 17."
    "DDR table size is 256K, N = 18."
    "1h: CRC32 hash is used as DDR hash function. CRC32 is reduced to N-bit using"
    "the same method as in 32-bit rolling XOR hash."
    "DDR table size is 8K,   N = 13."
    "DDR table size is 16K,  N = 14."
    "DDR table size is 32K,  N = 15."
    "DDR table size is 64K,  N = 16."
    "DDR table size is 128K, N = 17."
    "DDR table size is 256K, N = 18."
    "2h: CRC32 hash is used as DDR hash function. CRC32[N:0] is used as hash value"
    "DDR table size is 8K,   N = 12."
    "DDR table size is 16K,  N = 13."
    "DDR table size is 32K,  N = 14."
    "DDR table size is 64K,  N = 15."
    "DDR table size is 128K, N = 16."
    "DDR table size is 256K, N = 17."
    "3h: CRC32 hash is used as DDR hash function. CRC32[31:N] is used as hash value"
    "DDR table size is 8K,   N = 19."
    "DDR table size is 16K,  N = 18."
    "DDR table size is 32K,  N = 17."
    "DDR table size is 64K,  N = 16."
    "DDR table size is 128K, N = 15."
    "DDR table size is 256K, N = 14."
    "4h: RSS hash is used as DDR hash function using secret key 0. RSS[N:0] is used as hash value."
    "DDR table size is 8K,   N = 13."
    "DDR table size is 16K,  N = 14."
    "DDR table size is 32K,  N = 15."
    "DDR table size is 64K,  N = 16."
    "DDR table size is 128K, N = 17."
    "DDR table size is 256K, N = 18."
    "5h: RSS hash is used as DDR hash function using secret key 1. RSS[N:0] is used as hash value."
    "DDR table size is 8K,   N = 13."
    "DDR table size is 16K,  N = 14."
    "DDR table size is 32K,  N = 15."
    "DDR table size is 64K,  N = 16."
    "DDR table size is 128K, N = 17."
    "DDR table size is 256K, N = 18."
    "6h: RSS hash is used as DDR hash function using secret key 2. RSS[N:0] is used as hash value."
    "DDR table size is 8K,   N = 13."
    "DDR table size is 16K,  N = 14."
    "DDR table size is 32K,  N = 15."
    "DDR table size is 64K,  N = 16."
    "DDR table size is 128K, N = 17."
    "DDR table size is 256K, N = 18."
    "7h: RSS hash is used as DDR hash function using secret key 3. RSS[N:0] is used as hash value."
    "DDR table size is 8K,   N = 13."
    "DDR table size is 16K,  N = 14."
    "DDR table size is 32K,  N = 15."
    "DDR table size is 64K,  N = 16."
    "DDR table size is 128K, N = 17."
    "DDR table size is 256K, N = 18.",
#endif
    NATC_CONTROL_STATUS2_DDR_HASH_MODE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_HASH_MODE_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_HASH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "DDR_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for DDR memory read/write accesses"
    "(i.e., [63:0] becomes {[31:0], [63:32]}).",
#endif
    NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "DDR_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for DDR memory read/write accesses"
    "(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]}).",
#endif
    NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD =
{
    "CACHE_LOOKUP_BLOCKING_MODE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1",
#endif
    NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_AGE_TIMER_TICK
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD =
{
    "AGE_TIMER_TICK",
#if RU_INCLUDE_DESC
    "",
    "Timer tick for pseudo-LRU"
    "Timer is incremented on every system clock cycle"
    "Timer is incremented on every packet arrival to NAT block",
#endif
    NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_AGE_TIMER
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_AGE_TIMER_FIELD =
{
    "AGE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timer value used for pseudo-LRU;"
    "When timer fires the 8-bit age value of every entry in the cache is"
    "decremented (cap at 0).  The entry with lower value is"
    "the older entry.  The default setting keeps track of ~0.26s age at"
    "~1ms resolution."
    "0: 1 tick"
    "1: 2 ticks"
    "2: 4 ticks"
    "3: 8 ticks"
    "4: 16 ticks"
    ".."
    ".."
    "31: 2^31 TICKS",
#endif
    NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_AGE_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_CACHE_ALGO
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD =
{
    "CACHE_ALGO",
#if RU_INCLUDE_DESC
    "",
    "Replacement algorithm for caching"
    "Lowest-multi-hash-iteration number is used to select the final replacement"
    "entry if multiple entries were chosen by the selected algorithm.  For"
    "instance, if HIT_COUNT algorithm were selected, and 2nd, 3rd and 7th"
    "entry all have the same hit_count values, 2nd entry will be evicted."
    "Replacement algorithm prioritizes pseudo-LRU over lowest-hit-count."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm prioritizes lowest-hit-count over pseudo-LRU."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm uses pseudo-LRU."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm uses least-hit-count."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm prioritizes pseudo-LRU over pseudo-random."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm prioritizes lowest-hit-count over pseudo-random."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm uses pseudo-random algorithm."
    "Replacement algorithm prioritizes highest-hit-count over"
    "most-recently-use."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm prioritizes pseudo-LRU over lowest-byte-count."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm prioritizes lowest-byte-count over pseudo-LRU."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm uses least-byte-count."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm prioritizes lowest-byte-count over pseudo-random."
    "Replacement algorithm is LRU if cache size is 32 entries or less."
    "Replacement algorithm prioritizes highest-byte-count over"
    "most-recently-use."
    "Replacement algorithm is LRU if cache size is 32 entries or less.",
#endif
    NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_UNUSED2
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_UNUSED2_FIELD =
{
    "UNUSED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS2_UNUSED2_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_UNUSED2_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_UNUSED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_UNUSED1
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_UNUSED1_FIELD =
{
    "UNUSED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS2_UNUSED1_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_UNUSED1_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_UNUSED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD =
{
    "CACHE_UPDATE_ON_REG_DDR_LOOKUP",
#if RU_INCLUDE_DESC
    "",
    "This bit determines whether register interface lookup will cache the entry from DDR"
    "1h: Enable; entry fetched from DDR will be cached using register interface lookup command"
    "0h: Disable; entry fetched from DDR will not be cached using register interface lookup command",
#endif
    NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD =
{
    "DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 32-bit word for DDR counters on read/write accesses."
    "(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})",
#endif
    NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_DDR_HASH_SWAP
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD =
{
    "DDR_HASH_SWAP",
#if RU_INCLUDE_DESC
    "",
    "Reverse bytes within 18-bit DDR hash value",
#endif
    NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD =
{
    "DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1",
#endif
    NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD =
{
    "DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "(debug command) Do not set this bit to 1",
#endif
    NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_UNUSED0
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_UNUSED0_FIELD =
{
    "UNUSED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS2_UNUSED0_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_UNUSED0_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_UNUSED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_SMEM_DIS_TBL7
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD =
{
    "SMEM_DIS_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 7",
#endif
    NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_WIDTH,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_SMEM_DIS_TBL6
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD =
{
    "SMEM_DIS_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 6",
#endif
    NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_WIDTH,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_SMEM_DIS_TBL5
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD =
{
    "SMEM_DIS_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 5",
#endif
    NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_WIDTH,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_SMEM_DIS_TBL4
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD =
{
    "SMEM_DIS_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 4",
#endif
    NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_WIDTH,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_SMEM_DIS_TBL3
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD =
{
    "SMEM_DIS_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 3",
#endif
    NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_WIDTH,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_SMEM_DIS_TBL2
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD =
{
    "SMEM_DIS_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 2",
#endif
    NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_WIDTH,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_SMEM_DIS_TBL1
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD =
{
    "SMEM_DIS_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 1",
#endif
    NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_WIDTH,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_SMEM_DIS_TBL0
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD =
{
    "SMEM_DIS_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Disables cache counters, DDR counters update and eviction for DDR table 0",
#endif
    NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_WIDTH,
    NATC_TABLE_CONTROL_SMEM_DIS_TBL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 7"
    "lowest 4 bits of key[3:0] is used to indicate the context length"
    "0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes"
    "Note that key length is reduced by 4 bit"
    "0h: Disable variable context length"
    "1h: Enable variable context length",
#endif
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_WIDTH,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 6"
    "lowest 4 bits of key[3:0] is used to indicate the context length"
    "0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes"
    "Note that key length is reduced by 4 bit"
    "0h: Disable variable context length"
    "1h: Enable variable context length",
#endif
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_WIDTH,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 5"
    "lowest 4 bits of key[3:0] is used to indicate the context length"
    "0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes"
    "Note that key length is reduced by 4 bit"
    "0h: Disable variable context length"
    "1h: Enable variable context length",
#endif
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_WIDTH,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 4"
    "lowest 4 bits of key[3:0] is used to indicate the context length"
    "0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes"
    "Note that key length is reduced by 4 bit"
    "0h: Disable variable context length"
    "1h: Enable variable context length",
#endif
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_WIDTH,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 3"
    "lowest 4 bits of key[3:0] is used to indicate the context length"
    "0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes"
    "Note that key length is reduced by 4 bit"
    "0h: Disable variable context length"
    "1h: Enable variable context length",
#endif
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_WIDTH,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 2"
    "lowest 4 bits of key[3:0] is used to indicate the context length"
    "0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes"
    "Note that key length is reduced by 4 bit"
    "0h: Disable variable context length"
    "1h: Enable variable context length",
#endif
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_WIDTH,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 1"
    "lowest 4 bits of key[3:0] is used to indicate the context length"
    "0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes"
    "Note that key length is reduced by 4 bit"
    "0h: Disable variable context length"
    "1h: Enable variable context length",
#endif
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_WIDTH,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD =
{
    "VAR_CONTEXT_LEN_EN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Controls the amount of context to fetch from DDR in unit of 8 bytes for DDR table 0"
    "lowest 4 bits of key[3:0] is used to indicate the context length"
    "0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytes"
    "Note that key length is reduced by 4 bit"
    "0h: Disable variable context length"
    "1h: Enable variable context length",
#endif
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_WIDTH,
    NATC_TABLE_CONTROL_VAR_CONTEXT_LEN_EN_TBL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL7
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD =
{
    "KEY_LEN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 7"
    "0h: 16-byte key"
    "1h: 32-byte key",
#endif
    NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_WIDTH,
    NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD =
{
    "NON_CACHEABLE_TBL7",
#if RU_INCLUDE_DESC
    "",
    "DDR table 7 non-cacheable control"
    "0h: DDR table is cached"
    "1h: DDR table is not cached; counters are updated in DDR directly",
#endif
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_WIDTH,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL6
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD =
{
    "KEY_LEN_TBL6",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 6"
    "0h: 16-byte key"
    "1h: 32-byte key",
#endif
    NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_WIDTH,
    NATC_TABLE_CONTROL_KEY_LEN_TBL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD =
{
    "NON_CACHEABLE_TBL6",
#if RU_INCLUDE_DESC
    "",
    "DDR table 6 non-cacheable control"
    "0h: DDR table is cached"
    "1h: DDR table is not cached; counters are updated in DDR directly",
#endif
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_WIDTH,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL5
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD =
{
    "KEY_LEN_TBL5",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 5"
    "0h: 16-byte key"
    "1h: 32-byte key",
#endif
    NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_WIDTH,
    NATC_TABLE_CONTROL_KEY_LEN_TBL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD =
{
    "NON_CACHEABLE_TBL5",
#if RU_INCLUDE_DESC
    "",
    "DDR table 5 non-cacheable control"
    "0h: DDR table is cached"
    "1h: DDR table is not cached; counters are updated in DDR directly",
#endif
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_WIDTH,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL4
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD =
{
    "KEY_LEN_TBL4",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 4"
    "0h: 16-byte key"
    "1h: 32-byte key",
#endif
    NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_WIDTH,
    NATC_TABLE_CONTROL_KEY_LEN_TBL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD =
{
    "NON_CACHEABLE_TBL4",
#if RU_INCLUDE_DESC
    "",
    "DDR table 4 non-cacheable control"
    "0h: DDR table is cached"
    "1h: DDR table is not cached; counters are updated in DDR directly",
#endif
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_WIDTH,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL3
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD =
{
    "KEY_LEN_TBL3",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 3"
    "0h: 16-byte key"
    "1h: 32-byte key",
#endif
    NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_WIDTH,
    NATC_TABLE_CONTROL_KEY_LEN_TBL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD =
{
    "NON_CACHEABLE_TBL3",
#if RU_INCLUDE_DESC
    "",
    "DDR table 3 non-cacheable control"
    "0h: DDR table is cached"
    "1h: DDR table is not cached; counters are updated in DDR directly",
#endif
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_WIDTH,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL2
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD =
{
    "KEY_LEN_TBL2",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 2"
    "0h: 16-byte key"
    "1h: 32-byte key",
#endif
    NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_WIDTH,
    NATC_TABLE_CONTROL_KEY_LEN_TBL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD =
{
    "NON_CACHEABLE_TBL2",
#if RU_INCLUDE_DESC
    "",
    "DDR table 2 non-cacheable control"
    "0h: DDR table is cached"
    "1h: DDR table is not cached; counters are updated in DDR directly",
#endif
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_WIDTH,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL1
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD =
{
    "KEY_LEN_TBL1",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 1"
    "0h: 16-byte key"
    "1h: 32-byte key",
#endif
    NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_WIDTH,
    NATC_TABLE_CONTROL_KEY_LEN_TBL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD =
{
    "NON_CACHEABLE_TBL1",
#if RU_INCLUDE_DESC
    "",
    "DDR table 1 non-cacheable control"
    "0h: DDR table is cached"
    "1h: DDR table is not cached; counters are updated in DDR directly",
#endif
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_WIDTH,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL0
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD =
{
    "KEY_LEN_TBL0",
#if RU_INCLUDE_DESC
    "",
    "Length of the key for DDR table 0"
    "0h: 16-byte key"
    "1h: 32-byte key",
#endif
    NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_WIDTH,
    NATC_TABLE_CONTROL_KEY_LEN_TBL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD =
{
    "NON_CACHEABLE_TBL0",
#if RU_INCLUDE_DESC
    "",
    "DDR table 0 non-cacheable control"
    "0h: DDR table is cached"
    "1h: DDR table is not cached; counters are updated in DDR directly",
#endif
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_WIDTH,
    NATC_TABLE_CONTROL_NON_CACHEABLE_TBL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN
 ******************************************************************************/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD =
{
    "DDR_EVICT_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR evict counter enable"
    "Each bit enables/disables the counter increment based on a DDR table,"
    "bit 7 for DDR table 7..... bit 0 for DDR table 0."
    "0h: Disable DDR evict counter increment on ddr evict."
    "1h: Enable DDR evict counter increment on ddr evict.",
#endif
    NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_MASK,
    0,
    NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_WIDTH,
    NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN
 ******************************************************************************/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD =
{
    "DDR_REQUEST_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR request counter enable"
    "Each bit enables/disables the counter increment based on a DDR table,"
    "bit 7 for DDR table 7..... bit 0 for DDR table 0."
    "0h: Disable DDR request counter increment on ddr request."
    "1h: Enable DDR request counter increment on ddr request.",
#endif
    NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_MASK,
    0,
    NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_WIDTH,
    NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN
 ******************************************************************************/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD =
{
    "CACHE_MISS_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "Cache miss counter enable"
    "Each bit enables/disables the counter increment based on a DDR table,"
    "bit 7 for DDR table 7..... bit 0 for DDR table 0."
    "0h: Disable cache miss counter increment on cache miss."
    "1h: Enable cache miss counter increment on cache miss.",
#endif
    NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_MASK,
    0,
    NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_WIDTH,
    NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN
 ******************************************************************************/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD =
{
    "CACHE_HIT_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "Cache hit counter enable"
    "Each bit enables/disables the counter increment based on a DDR table,"
    "bit 7 for DDR table 7..... bit 0 for DDR table 0."
    "0h: Disable cache hit counter increment on cache hit."
    "1h: Enable cache hit counter increment on cache hit.",
#endif
    NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_MASK,
    0,
    NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_WIDTH,
    NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_STAT_COUNTER_CONTROL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_STAT_COUNTER_CONTROL_1_RESERVED0_FIELD_MASK,
    0,
    NATC_STAT_COUNTER_CONTROL_1_RESERVED0_FIELD_WIDTH,
    NATC_STAT_COUNTER_CONTROL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS
 ******************************************************************************/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD =
{
    "COUNTER_WRAPAROUND_DIS",
#if RU_INCLUDE_DESC
    "",
    "Counter wraparound disable"
    "This applies to all stat counters defined in"
    "NATC_STAT_COUNTER_CONTROL_0 and  NATC_STAT_COUNTER_CONTROL_1."
    "0h: The counter will wraparound to 0 after it reaches ffffffffh."
    "1h: The counter will cap at ffffffffh.",
#endif
    NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_MASK,
    0,
    NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_WIDTH,
    NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN
 ******************************************************************************/
const ru_field_rec NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD =
{
    "DDR_BLOCK_COUNT_EN",
#if RU_INCLUDE_DESC
    "",
    "DDR block counter enable"
    "Each bit enables/disables the counter increment based on a DDR table,"
    "bit 7 for DDR table 7..... bit 0 for DDR table 0."
    "0h: Disable DDR block counter increment on ddr block."
    "1h: Enable DDR block counter increment on ddr block.",
#endif
    NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_MASK,
    0,
    NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_WIDTH,
    NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: NATC_CONTROL_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CONTROL_STATUS_FIELDS[] =
{
    &NATC_CONTROL_STATUS_DDR_ENABLE_FIELD,
    &NATC_CONTROL_STATUS_NATC_ADD_COMMAND_SPEEDUP_MODE_FIELD,
    &NATC_CONTROL_STATUS_UNUSED4_FIELD,
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

const ru_reg_rec NATC_CONTROL_STATUS_REG = 
{
    "CONTROL_STATUS",
#if RU_INCLUDE_DESC
    "NAT Cache Control and Status Register",
    "NAT Cache Control and Status Register.",
#endif
    NATC_CONTROL_STATUS_REG_OFFSET,
    0,
    0,
    1043,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    23,
    NATC_CONTROL_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_CONTROL_STATUS2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CONTROL_STATUS2_FIELDS[] =
{
    &NATC_CONTROL_STATUS2_DDR_HASH_MODE_FIELD,
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
    &NATC_CONTROL_STATUS2_UNUSED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_CONTROL_STATUS2_REG = 
{
    "CONTROL_STATUS2",
#if RU_INCLUDE_DESC
    "NAT Cache Control and Status Register2",
    "NAT Cache Control and Status Register",
#endif
    NATC_CONTROL_STATUS2_REG_OFFSET,
    0,
    0,
    1044,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    NATC_CONTROL_STATUS2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_TABLE_CONTROL
 ******************************************************************************/
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

const ru_reg_rec NATC_TABLE_CONTROL_REG = 
{
    "TABLE_CONTROL",
#if RU_INCLUDE_DESC
    "NAT Cache Table Control Register",
    "NAT Cache Table Control Register",
#endif
    NATC_TABLE_CONTROL_REG_OFFSET,
    0,
    0,
    1045,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    NATC_TABLE_CONTROL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_STAT_COUNTER_CONTROL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_STAT_COUNTER_CONTROL_0_FIELDS[] =
{
    &NATC_STAT_COUNTER_CONTROL_0_DDR_EVICT_COUNT_EN_FIELD,
    &NATC_STAT_COUNTER_CONTROL_0_DDR_REQUEST_COUNT_EN_FIELD,
    &NATC_STAT_COUNTER_CONTROL_0_CACHE_MISS_COUNT_EN_FIELD,
    &NATC_STAT_COUNTER_CONTROL_0_CACHE_HIT_COUNT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_STAT_COUNTER_CONTROL_0_REG = 
{
    "STAT_COUNTER_CONTROL_0",
#if RU_INCLUDE_DESC
    "NAT cache stat counter control register 0",
    "NAT cache stat counter control register 0",
#endif
    NATC_STAT_COUNTER_CONTROL_0_REG_OFFSET,
    0,
    0,
    1046,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NATC_STAT_COUNTER_CONTROL_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_STAT_COUNTER_CONTROL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_STAT_COUNTER_CONTROL_1_FIELDS[] =
{
    &NATC_STAT_COUNTER_CONTROL_1_RESERVED0_FIELD,
    &NATC_STAT_COUNTER_CONTROL_1_COUNTER_WRAPAROUND_DIS_FIELD,
    &NATC_STAT_COUNTER_CONTROL_1_DDR_BLOCK_COUNT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_STAT_COUNTER_CONTROL_1_REG = 
{
    "STAT_COUNTER_CONTROL_1",
#if RU_INCLUDE_DESC
    "NAT cache stat counter control register 1",
    "NAT cache stat counter control register 1",
#endif
    NATC_STAT_COUNTER_CONTROL_1_REG_OFFSET,
    0,
    0,
    1047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    NATC_STAT_COUNTER_CONTROL_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: NATC
 ******************************************************************************/
static const ru_reg_rec *NATC_REGS[] =
{
    &NATC_CONTROL_STATUS_REG,
    &NATC_CONTROL_STATUS2_REG,
    &NATC_TABLE_CONTROL_REG,
    &NATC_STAT_COUNTER_CONTROL_0_REG,
    &NATC_STAT_COUNTER_CONTROL_1_REG,
};

unsigned long NATC_ADDRS[] =
{
    0x82e50000,
};

const ru_block_rec NATC_BLOCK = 
{
    "NATC",
    NATC_ADDRS,
    1,
    5,
    NATC_REGS
};

/* End of file XRDP_NATC.c */
