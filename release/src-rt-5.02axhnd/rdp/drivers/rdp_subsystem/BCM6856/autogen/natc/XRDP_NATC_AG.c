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
    "(i.e., [127:0] becomes {[63:0], [127:64]})."
    "This bit should be set to 1 in Little Endian mode.",
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
    "Swap all bytes on DDR interface."
    "This bit should be set to 1 in Little Endian mode.",
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
 * Field: NATC_CONTROL_STATUS_UNUSED3
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_UNUSED3_FIELD =
{
    "UNUSED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS_UNUSED3_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_UNUSED3_FIELD_WIDTH,
    NATC_CONTROL_STATUS_UNUSED3_FIELD_SHIFT,
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
 * Field: NATC_CONTROL_STATUS_UNUSED2
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_UNUSED2_FIELD =
{
    "UNUSED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS_UNUSED2_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_UNUSED2_FIELD_WIDTH,
    NATC_CONTROL_STATUS_UNUSED2_FIELD_SHIFT,
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
    "This bit is only valid when CACHE_UPDATE_ON_DDR_MISS bit is set to 1."
    "This bit determines whether pending FIFO entry will be checked to"
    "determine whether cache update on DDR miss will happen or not."
    "1h: Enable; DDR miss entry fetched from DDR will be cached if pending FIFO"
    "contains entries which have the same hash value as DDR miss entry."
    "0h: Disable; DDR miss entry fetched from DDR will always be cached.",
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
    "This bit determines whether DDR lookup will cache the entry for DDR miss entry."
    "1h: Enable; DDR miss entry fetched from DDR will be cached."
    "0h: Disable; DDR miss entry fetched from DDR will not be cached.",
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
    "0h: Enable DDR lookup when cache misses using register interface lookup"
    "1h: Disable DDR lookup when cache misses using register interface lookup",
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
 * Field: NATC_CONTROL_STATUS_REGFILE_FIFO_RESET
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD =
{
    "REGFILE_FIFO_RESET",
#if RU_INCLUDE_DESC
    "",
    "Reset regfile_FIFO.",
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
 * Field: NATC_CONTROL_STATUS_NAT_HASH_MODE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD =
{
    "NAT_HASH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Hash algorithm used for internal caching"
    "0h: 32-bit rolling XOR hash is used as cache hash function."
    "1h: CRC32 hash is used as cache hash function. CRC32 is reduced to 10-bit using"
    "the same method as in 32-bit rolling XOR hash."
    "2h: CRC32 hash is used as cache hash function. CRC32[9:0] is used as hash value"
    "3h: CRC32 hash is used as cache hash function. CRC32[25:16] is used as hash value.",
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
    "Maximum number of multi-hash iterations"
    "Value of 0 is 1 iteration, 1 is 2 iterations, 2 is 3 iterations, etc.",
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
 * Field: NATC_CONTROL_STATUS_NATC_SMEM_DISABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_NATC_SMEM_DISABLE_FIELD =
{
    "NATC_SMEM_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables counters from incrementing when hit",
#endif
    NATC_CONTROL_STATUS_NATC_SMEM_DISABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_NATC_SMEM_DISABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS_NATC_SMEM_DISABLE_FIELD_SHIFT,
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
    "DDR table size is 256K, N = 14.",
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
    "(i.e., [63:0] becomes {[31:0], [63:32]})."
    "This bit should be set to 1 in Little Endian mode.",
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
    "(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})."
    "This bit should be set to 1 in Little Endian mode.",
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
    "the older entry.  The default setting keeps track of 2s age at"
    "~7ms resolution."
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
    "If CACHE_DISABLE or EVICTION_DISABLE is set, HIT_COUNT algorithm"
    "can only keep track of the hit count while the entry is in the cache."
    "When the entry is evicted hit count for that entry is lost."
    "Replacement algorithm prioritizes pseudo-LRU over lowest-hit-count"
    "Replacement algorithm prioritizes lowest-hit-count over pseudo-LRU"
    "Replacement algorithm uses pseudo-LRU"
    "Replacement algorithm uses least-hit-count"
    "Replacement algorithm prioritizes pseudo-LRU over pseudo-random"
    "Replacement algorithm prioritizes lowest-hit-count over pseudo-random"
    "Replacement algorithm uses pseudo-random algorithm"
    "Replacement algorithm prioritizes highest-hit-count over"
    "most-recently-use"
    "Replacement algorithm prioritizes pseudo-LRU over lowest-byte-count"
    "Replacement algorithm prioritizes lowest-byte-count over pseudo-LRU"
    "Replacement algorithm uses least-byte-count"
    "Replacement algorithm prioritizes lowest-byte-count over pseudo-random"
    "Replacement algorithm prioritizes highest-byte-count over"
    "most-recently-use",
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
 * Field: NATC_CONTROL_STATUS2_UNUSED20
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_UNUSED20_FIELD =
{
    "UNUSED20",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS2_UNUSED20_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_UNUSED20_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_UNUSED20_FIELD_SHIFT,
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
    "(debug command) Do not set this bit to 1"
    "Enable replacing existing cache counters with DDR fetched entry",
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
 * Field: NATC_CONTROL_STATUS2_EVICTION_DISABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_EVICTION_DISABLE_FIELD =
{
    "EVICTION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disable counter eviction to DDR; this bit is effective when CACHE_DISABLE is 0"
    "Set this bit when counters are not used; NATC performance will improve due"
    "to reduced DDR accesses; CACHE_ALGO should not use HIT_COUNT and BYTE_COUNT",
#endif
    NATC_CONTROL_STATUS2_EVICTION_DISABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_EVICTION_DISABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_EVICTION_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_TABLE_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_TABLE_CONTROL_RESERVED0_FIELD_MASK,
    0,
    NATC_TABLE_CONTROL_RESERVED0_FIELD_WIDTH,
    NATC_TABLE_CONTROL_RESERVED0_FIELD_SHIFT,
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
    &NATC_CONTROL_STATUS_UNUSED3_FIELD,
    &NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_UNUSED2_FIELD,
    &NATC_CONTROL_STATUS_PENDING_FIFO_ENTRY_CHECK_ENABLE_FIELD,
    &NATC_CONTROL_STATUS_CACHE_UPDATE_ON_DDR_MISS_FIELD,
    &NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD,
    &NATC_CONTROL_STATUS_REGFILE_FIFO_RESET_FIELD,
    &NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD,
    &NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD,
    &NATC_CONTROL_STATUS_DECR_COUNT_WRAPAROUND_ENABLE_FIELD,
    &NATC_CONTROL_STATUS_NAT_ARB_ST_FIELD,
    &NATC_CONTROL_STATUS_NATC_SMEM_INCREMENT_ON_REG_LOOKUP_FIELD,
    &NATC_CONTROL_STATUS_NATC_SMEM_CLEAR_BY_UPDATE_DISABLE_FIELD,
    &NATC_CONTROL_STATUS_NATC_SMEM_DISABLE_FIELD,
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
    1091,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    24,
    NATC_CONTROL_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_CONTROL_STATUS2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CONTROL_STATUS2_FIELDS[] =
{
    &NATC_CONTROL_STATUS2_DDR_HASH_SWAP_FIELD,
    &NATC_CONTROL_STATUS2_DDR_HASH_MODE_FIELD,
    &NATC_CONTROL_STATUS2_DDR_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS2_DDR_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS2_CACHE_LOOKUP_BLOCKING_MODE_FIELD,
    &NATC_CONTROL_STATUS2_AGE_TIMER_TICK_FIELD,
    &NATC_CONTROL_STATUS2_AGE_TIMER_FIELD,
    &NATC_CONTROL_STATUS2_CACHE_ALGO_FIELD,
    &NATC_CONTROL_STATUS2_UNUSED1_FIELD,
    &NATC_CONTROL_STATUS2_UNUSED0_FIELD,
    &NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD,
    &NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS2_UNUSED20_FIELD,
    &NATC_CONTROL_STATUS2_DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE_FIELD,
    &NATC_CONTROL_STATUS2_DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE_FIELD,
    &NATC_CONTROL_STATUS2_EVICTION_DISABLE_FIELD,
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
    1092,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    NATC_CONTROL_STATUS2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_TABLE_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_TABLE_CONTROL_FIELDS[] =
{
    &NATC_TABLE_CONTROL_RESERVED0_FIELD,
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
    1093,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    25,
    NATC_TABLE_CONTROL_FIELDS
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
    3,
    NATC_REGS
};

/* End of file XRDP_NATC.c */
