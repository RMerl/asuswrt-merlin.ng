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
 * Field: NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD =
{
    "SMEM_32BIT_IN_64BIT_SWAP_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Swap 32-bit word within 64-bit word for NAT statistics (counter) memory accesses"
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
    "Reverse bytes within 32-bit word for NAT statistics (counter) memory accesses"
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
 * Field: NATC_CONTROL_STATUS_TOTAL_LEN
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_TOTAL_LEN_FIELD =
{
    "TOTAL_LEN",
#if RU_INCLUDE_DESC
    "",
    "Total length of the lookup key + result (including 4-byte byte count and 4-byte hit count)."
    "The result length  (including 4-byte byte count and 4-byte hit count)"
    "should be TOTAL_LEN - KEY_LEN."
    "The default value of this register will be from harware DEFINE which defines the"
    "total length of the nat cache memory and Statitics Memory (4-byte byte count and 4-byte hit count)."
    "0h: 48-byte"
    "1h: 64-byte"
    "2h: 80-byte"
    "3h: 96-byte"
    "4h: 112-byte"
    "5h: 128-byte"
    "6h: Not used"
    "7h: Not used",
#endif
    NATC_CONTROL_STATUS_TOTAL_LEN_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_TOTAL_LEN_FIELD_WIDTH,
    NATC_CONTROL_STATUS_TOTAL_LEN_FIELD_SHIFT,
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
    "0h: Enable DDR lookup on register lookup miss"
    "1h: Disable DDR lookup on register lookup cache miss",
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
 * Field: NATC_CONTROL_STATUS_UNUSED1
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_UNUSED1_FIELD =
{
    "UNUSED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS_UNUSED1_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_UNUSED1_FIELD_WIDTH,
    NATC_CONTROL_STATUS_UNUSED1_FIELD_SHIFT,
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
    "0h: 32-bit rolling XOR hash is used as nat hash function."
    "1h: CRC32 hash is used as NAT hash function. CRC32 is reduced to 16-bit using"
    "the same method as the one used in 32-bit rolling XOR hash."
    "2h: CRC32 hash is used as NAT hash function. CRC32[15:0] is used as hash value"
    "3h: CRC32 hash is used as NAT hash function. CRC32[31:16] is used as hash value.",
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
    "Value of 0 is 1 time, 1 is 2 times, 2 is 3 times, etc.",
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
 * Field: NATC_CONTROL_STATUS_UNUSED0
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS_UNUSED0_FIELD =
{
    "UNUSED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_CONTROL_STATUS_UNUSED0_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS_UNUSED0_FIELD_WIDTH,
    NATC_CONTROL_STATUS_UNUSED0_FIELD_SHIFT,
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
    "Enables incrementing NAT hit counter by 1 and"
    "NAT byte counter by PKT_LEN defined in NAT_PKT_LEN register"
    "on successful lookups using register interface"
    "BY default, NAT counter memory only increments on successful lookups by Runner.",
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
    "Disables clearing NAT counter (hit count and byte count) memory when an existing entry is replaced by ADD command",
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
    "Disables NAT counter memory from incrementing for a lookup hit.",
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
    "Enables all NATC state machines and input fifo;"
    "Clearing this bit from a 1 will halt all state machines gracefully to idle states,"
    "all outstanding transactions in the fifo will remain in the fifo and input"
    "fifo will also be disable;  All NATC configuration registers should be configured"
    "before enabling this bit.",
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
    "Block Reset (including resetting all registers to default values)"
    "Self clear.",
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
    "Reverse bytes within 16-bit DDR hash value",
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
    "0h: 32-bit rolling XOR hash is used as DDR hash function."
    "1h: CRC32 hash is used as DDR hash function. CRC32 is reduced to 16-bit using"
    "the same method as the one used in 32-bit rolling XOR hash."
    "2h: CRC32 hash is used as DDR hash function. CRC32[15:0] is used as hash value"
    "3h: CRC32 hash is used as DDR hash function. CRC32[31:16] is used as hash value.",
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
    "Swap 32-bit word within 64-bit word for NAT DDR memory read/write accesses"
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
    "Reverse bytes within 32-bit word for NAT DDR memory read/write accesses"
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
    "1h: Local cache lookup will be blocked if the lookup keys have the same hash value"
    "as the oustanding DDR searches. This is used when the ordering whthin the same"
    "flow is to be preserved."
    "0h: Local cache will not be blocked during multiple outstanding DDR searches."
    "This is used where search ordering preservation is not required."
    "The hit count and byte count in local cache are not accurate in this mode."
    "The scenario is as follows when the same key is used."
    "1. First key lookup, it is not in nat cache, it proceeds with ddr lookup."
    "2. Second key lookup, it is still not in nat cache. It proceeds with another ddr lookup."
    "3. The key/result/hit count/byte count from #1 ddr lookup returns and writes to local cache."
    "4. Other lookup causes local cache enrty from #3 to be evicted."
    "5. The key/results/hit count/byte count (hit count and byte count value in ddr before overridden by eviction) from #2 ddr lookup returns"
    "and it writes the hit count/byte count into local cache."
    "This causes the hit count in local cache to be one less than correct hit count."
    "This also causes the byte count in local cache to be one packet length less than correct byte count.",
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
    "can only keep track of the hit count while the session is in the cache."
    "When the session is evicted hit count for that session is lost."
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
 * Field: NATC_CONTROL_STATUS2_DDR_BINS_PER_BUCKET
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_BINS_PER_BUCKET_FIELD =
{
    "DDR_BINS_PER_BUCKET",
#if RU_INCLUDE_DESC
    "",
    "Number of entries per bucket - 1.  Number of entries within a bucket is"
    "limited by bus max burst size.  For instance, if UBUS supports max burst"
    "size of 512 bytes, key length is 16 bytes, maximum DDR_BINS_PER_BUCKET that can be programmed"
    "is 512 bytes / 16-bytes (bytes per bin) = 32 entries."
    "0h: 1 entry"
    "1h: 2 entries"
    "2h: 3 entries"
    "3h: 4 entries"
    "4h: 5 entries"
    ".............",
#endif
    NATC_CONTROL_STATUS2_DDR_BINS_PER_BUCKET_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_BINS_PER_BUCKET_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_BINS_PER_BUCKET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CONTROL_STATUS2_DDR_SIZE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_SIZE_FIELD =
{
    "DDR_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Number of NAT entries in DDR NAT table."
    "Add DDR_BINS_PER_BUCKET field to the table size selection below to compute"
    "the actual size of the table."
    "For instance, if DDR_BINS_PER_BUCKET is 3 (4 bins per bucket)"
    "and DDR_size is 3 (64k entries), the size of the table in DDR is"
    "(64*1024+3) multiply by total length of key and results in bytes"
    "(defined by TOTAL_LEN in NATC_CONTROL_STATUS register)."
    "Extra 3 entries are used to store"
    "collided entries of the last entry of the desire table size."
    "64k entries"
    "32k entries"
    "16k entries"
    "8k entries",
#endif
    NATC_CONTROL_STATUS2_DDR_SIZE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_SIZE_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_SIZE_FIELD_SHIFT,
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
    "1h: On a register interface Lookup command,"
    "if it is a cache miss, NAT local cache will be updated with value"
    "fetched from a successful DDR lookup."
    "The hit count and byte count returned back to the register interface will be"
    "the hit count and byte count in SMEM (counter memory) after the update."
    "0h: On a register interface Lookup command,"
    "if it is a cache miss, NAT local cache will not be updated with value"
    "fetched from a successful DDR lookup."
    "If it is a cache hit, the hit count and byte count returned back to the register interface"
    "will be the hit count and byte count in SMEM (counter memory) after the update."
    "If it is a cache miss, the hit count and byte count returned back to the register interface"
    "will be the hit count and byte count returned from DDR.",
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
    "Reverse bytes within 32-bit word for NAT DDR counters (hit count and byte count) read/write accesses."
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
 * Field: NATC_CONTROL_STATUS2_DDR_WRITE_W_ACK_ENABLE
 ******************************************************************************/
const ru_field_rec NATC_CONTROL_STATUS2_DDR_WRITE_W_ACK_ENABLE_FIELD =
{
    "DDR_WRITE_W_ACK_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable write w/ack for DDR accesses"
    "Default is to use write w/o ack",
#endif
    NATC_CONTROL_STATUS2_DDR_WRITE_W_ACK_ENABLE_FIELD_MASK,
    0,
    NATC_CONTROL_STATUS2_DDR_WRITE_W_ACK_ENABLE_FIELD_WIDTH,
    NATC_CONTROL_STATUS2_DDR_WRITE_W_ACK_ENABLE_FIELD_SHIFT,
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
    "Enable replacing local hit count and byte count with DDR fetched entry when the entry"
    "already existed in cache"
    "This is a debug feature, do not set this bit",
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
    "0h: If DDR lookup keys have the same hash value as an outstanding DDR searches,"
    "DDR search request will be fordwarded to a pending lookup FIFO"
    "(instead of forwarding to DDR control). The request"
    "in pending lookup FIFO will be read to perform local cache lookup after the"
    "pending DDR search with the same key hash value has returned from DDR control"
    "and completes the local cache lookup."
    "1h: DDR lookup pending FIFO mode is disabled. All DDR lookup search requests"
    "are forwarded to DDR control regardless of the key hash value.",
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
    "Disable session hit count and byte count eviction back to DDR."
    "Set this bit when session hit count and byte count values are not used and the accesses"
    "to DDR will be reduced by 1/2 to speed up overall NAT operation; this bit"
    "is effective when CACHE_DISABLE is 0; this bit also affects CACHE_ALGO"
    "if HIT_COUNT and BYTE_COUNT are used.",
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
 * Field: NATC_TABLE_CONTROL_KEY_LEN_TBL7
 ******************************************************************************/
const ru_field_rec NATC_TABLE_CONTROL_KEY_LEN_TBL7_FIELD =
{
    "KEY_LEN_TBL7",
#if RU_INCLUDE_DESC
    "",
    "Table 7 length of the key."
    "0h: 16-byte key;"
    "1h: 32-byte key;",
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
    "Table 7 Non Cacheable."
    "0h: Cacheable."
    "lookup is fetched from DDR and cached (i.e., internal NAT table is updated)."
    "1h: Non Cacheable."
    "lookup is fetched from DDR and not cached (i.e., internal NAT table is not"
    "updated) and session hit count and byte count values are only updated in DDR;",
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
    "Table 6 length of the key."
    "0h: 16-byte key;"
    "1h: 32-byte key;",
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
    "Table 6 Non Cacheable."
    "0h: Cacheable."
    "lookup is fetched from DDR and cached (i.e., internal NAT table is updated)."
    "1h: Non Cacheable."
    "lookup is fetched from DDR and not cached (i.e., internal NAT table is not"
    "updated) and session hit count and byte count values are only updated in DDR;",
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
    "Table 5 length of the key."
    "0h: 16-byte key;"
    "1h: 32-byte key;",
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
    "Table 5 Non Cacheable."
    "0h: Cacheable."
    "lookup is fetched from DDR and cached (i.e., internal NAT table is updated)."
    "1h: Non Cacheable."
    "lookup is fetched from DDR and not cached (i.e., internal NAT table is not"
    "updated) and session hit count and byte count values are only updated in DDR;",
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
    "Table 4 length of the key."
    "0h: 16-byte key;"
    "1h: 32-byte key;",
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
    "Table 4 Non Cacheable."
    "0h: Cacheable."
    "lookup is fetched from DDR and cached (i.e., internal NAT table is updated)."
    "1h: Non Cacheable."
    "lookup is fetched from DDR and not cached (i.e., internal NAT table is not"
    "updated) and session hit count and byte count values are only updated in DDR;",
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
    "Table 3 length of the key."
    "0h: 16-byte key;"
    "1h: 32-byte key;",
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
    "Table 3 Non Cacheable."
    "0h: Cacheable."
    "lookup is fetched from DDR and cached (i.e., internal NAT table is updated)."
    "1h: Non Cacheable."
    "lookup is fetched from DDR and not cached (i.e., internal NAT table is not"
    "updated) and session hit count and byte count values are only updated in DDR;",
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
    "Table 2 length of the key."
    "0h: 16-byte key;"
    "1h: 32-byte key;",
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
    "Table 2 Non Cacheable."
    "0h: Cacheable."
    "lookup is fetched from DDR and cached (i.e., internal NAT table is updated)."
    "1h: Non Cacheable."
    "lookup is fetched from DDR and not cached (i.e., internal NAT table is not"
    "updated) and session hit count and byte count values are only updated in DDR;",
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
    "Table 1 length of the key."
    "0h: 16-byte key;"
    "1h: 32-byte key;",
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
    "Table 1 Non Cacheable."
    "0h: Cacheable."
    "lookup is fetched from DDR and cached (i.e., internal NAT table is updated)."
    "1h: Non Cacheable."
    "lookup is fetched from DDR and not cached (i.e., internal NAT table is not"
    "updated) and session hit count and byte count values are only updated in DDR;",
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
    "Table 0 length of the key."
    "0h: 16-byte key;"
    "1h: 32-byte key;",
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
    "Table 0 Non Cacheable."
    "0h: Cacheable."
    "lookup is fetched from DDR and cached (i.e., internal NAT table is updated)."
    "1h: Non Cacheable."
    "lookup is fetched from DDR and not cached (i.e., internal NAT table is not"
    "updated) and session hit count and byte count values are only updated in DDR;",
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
    &NATC_CONTROL_STATUS_SMEM_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_SMEM_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_DDR_SWAP_ALL_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_UNUSED3_FIELD,
    &NATC_CONTROL_STATUS_REG_32BIT_IN_64BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_REG_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS_UNUSED2_FIELD,
    &NATC_CONTROL_STATUS_TOTAL_LEN_FIELD,
    &NATC_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP_FIELD,
    &NATC_CONTROL_STATUS_UNUSED1_FIELD,
    &NATC_CONTROL_STATUS_NAT_HASH_MODE_FIELD,
    &NATC_CONTROL_STATUS_MULTI_HASH_LIMIT_FIELD,
    &NATC_CONTROL_STATUS_UNUSED0_FIELD,
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
    1105,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    22,
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
    &NATC_CONTROL_STATUS2_DDR_BINS_PER_BUCKET_FIELD,
    &NATC_CONTROL_STATUS2_DDR_SIZE_FIELD,
    &NATC_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP_FIELD,
    &NATC_CONTROL_STATUS2_DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL_FIELD,
    &NATC_CONTROL_STATUS2_DDR_WRITE_W_ACK_ENABLE_FIELD,
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
    1106,
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
    1107,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    17,
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
