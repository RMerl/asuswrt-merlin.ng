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
 * Field: NATC_ENG_COMMAND_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_ENG_COMMAND_STATUS_RESERVED0_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_RESERVED0_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD =
{
    "DEL_CMD_DDR_BIN",
#if RU_INCLUDE_DESC
    "",
    "This filed specifies the DDR BIN number to be compared for DEL command"
    "when DEL_CMD_MODE is set to 1",
#endif
    NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_ENG_COMMAND_STATUS_RESERVED1_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_RESERVED1_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD =
{
    "DEL_CMD_MODE",
#if RU_INCLUDE_DESC
    "",
    "DEL Command DDR-bin matching mode enable"
    "0h: DEL command deletes the cache entry with matching key"
    "1h: DEL command deletes the cache entry with matching key and matching DDR bin"
    "number specified in DEL_CMD_DDR_BIN field",
#endif
    NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_CACHE_FLUSH
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD =
{
    "CACHE_FLUSH",
#if RU_INCLUDE_DESC
    "",
    "Cache Flush enable"
    "When set, LOOKUP command is used to flush counters from cache into DDR."
    "This command does not use key to lookup the cache entry.  Instead it uses"
    "cache index number located in 10-MSB bits of key specified in NAT_KEY_RESULT register."
    "For 16 bytes key, the cache index will be located in"
    "{NAT_KEY_RESULT[15], NAT_KEY_RESULT[14][7:6]} (15th byte of NAT_KEY_RESULT register and"
    "bits 7:6 of 14th byte of NAT_KEY_RESULT register)."
    "For 32 bytes key, the cache index will be located in"
    "{NAT_KEY_RESULT[31], NAT_KEY_RESULT[30][7:6]} (31th byte of NAT_KEY_RESULT register and"
    "bits 7:6 of 30th byte of NAT_KEY_RESULT register)."
    "0h: LOOKUP command is used as normal lookup command."
    "1h: LOOKUP command is used as cache flush command.",
#endif
    NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_DECR_COUNT
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD =
{
    "DECR_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Decrement-counter mode enable"
    "When set, LOOKUP command will decrement hit counter by 1 and decrement"
    "byte counter by the value specified in PKT_LEN, on a successful lookup."
    "NATC_SMEM_INCREMENT_ON_REG_LOOKUP must be set to 1 for it to be effective"
    "0h: LOOKUP command will increment hit counter and byte counter"
    "1h: LOOKUP command will decrement hit counter and byte counter",
#endif
    NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_NAT_TBL
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD =
{
    "NAT_TBL",
#if RU_INCLUDE_DESC
    "",
    "Select the DDR Table on which the command will operate"
    "0h: DDR table 0"
    "1h: DDR table 1"
    "2h: DDR table 2"
    "3h: DDR table 3"
    "4h: DDR table 4"
    "5h: DDR table 5"
    "6h: DDR table 6"
    "7h: DDR table 7",
#endif
    NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD =
{
    "MULTIHASH_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cache multi-hash iteration count status"
    "Value of 0 is iteration 1, 1 is iteration 2, 2 is iteration 3, etc."
    "cache miss returns 0 count.",
#endif
    NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_CACHE_HIT
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD =
{
    "CACHE_HIT",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when a LOOKUP command has a cache hit",
#endif
    NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_MISS
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_MISS_FIELD =
{
    "MISS",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when a LOOKUP command has a miss",
#endif
    NATC_ENG_COMMAND_STATUS_MISS_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_MISS_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_MISS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_ERROR
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_ERROR_FIELD =
{
    "ERROR",
#if RU_INCLUDE_DESC
    "",
    "This bit is set for the following 2 cases"
    "For ADD command all multi-hash entries are occupied (i.e, no room to ADD)"
    "For DEL command entry is not found and cannot be deleted",
#endif
    NATC_ENG_COMMAND_STATUS_ERROR_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_ERROR_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_ERROR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_BUSY
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "Interface Busy"
    "This bit is set when command is issued but still in progress been processed."
    "When command completes this bit will be cleared.",
#endif
    NATC_ENG_COMMAND_STATUS_BUSY_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_BUSY_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_UNUSED10
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_UNUSED10_FIELD =
{
    "UNUSED10",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_ENG_COMMAND_STATUS_UNUSED10_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_UNUSED10_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_UNUSED10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_COMMAND_STATUS_COMMAND
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_COMMAND_FIELD =
{
    "COMMAND",
#if RU_INCLUDE_DESC
    "",
    "Command to be executed"
    "This command only operates on the entries in cache except LOOKUP command where"
    "entry can be fetched from DDR."
    "Writing to this field causes BUSY bit to be set."
    "Note: For all commands, key consists of all 0's indicates unused entry in h/w"
    "and therefore cannot be used."
    "No-Operation"
    "Lookup"
    "Add (to cache only)"
    "Del (from cache only)"
    "Hash (debug command)"
    "Hashes are stored in different set of COMMAND_STATUS register (i.e."
    "Hashes for HASH command issued using NAT0 register are returned"
    "at NAT1 KEY_RESULT registers; hashes for NAT1 HASH command"
    "are returned at NAT0 KEY_RESULT; hashes for NAT2 HASH command are"
    "returned at NAT3 KEY_RESULT; hashes for NAT3 HASH command are"
    "returned at NAT2 KEY_RESULT register)."
    "Internal Cache command (debug command)"
    "Do not use this command",
#endif
    NATC_ENG_COMMAND_STATUS_COMMAND_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_COMMAND_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_COMMAND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_HASH_HASH
 ******************************************************************************/
const ru_field_rec NATC_ENG_HASH_HASH_FIELD =
{
    "HASH",
#if RU_INCLUDE_DESC
    "",
    "hash value; only valid on a successful lookup/add/del command"
    "For cache hit 10-bit hash value is returned."
    "For cache miss and DDR_ENABLE is 0, first hash value (10-bit) is returned."
    "For cache miss, DDR_ENABLE is 1 and DDR is a hit, 18-bit DDR hash value + DDR bin count is returned."
    "For cache miss, DDR_ENABLE is 1 and DDR is a miss, 18-bit DDR hash value is returned.",
#endif
    NATC_ENG_HASH_HASH_FIELD_MASK,
    0,
    NATC_ENG_HASH_HASH_FIELD_WIDTH,
    NATC_ENG_HASH_HASH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: NATC_ENG_HIT_COUNT_HIT_COUNT
 ******************************************************************************/
const ru_field_rec NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD =
{
    "HIT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "bits 27:0 are 28-bit hit count value."
    "bits 31:28 are 4 lsb of 36-bit byte count value."
    "only valid on a successful lookup or delete command.",
#endif
    NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD_MASK,
    0,
    NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD_WIDTH,
    NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: NATC_ENG_BYTE_COUNT_BYTE_COUNT
 ******************************************************************************/
const ru_field_rec NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD =
{
    "BYTE_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit msb of 36-bit byte count value."
    "{BYTE_COUNT, HIT_COUNT[31:28]} is the 36-bit byte count value."
    "only valid on a successful lookup or delete command",
#endif
    NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD_MASK,
    0,
    NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD_WIDTH,
    NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: NATC_ENG_PKT_LEN_UNUSED
 ******************************************************************************/
const ru_field_rec NATC_ENG_PKT_LEN_UNUSED_FIELD =
{
    "UNUSED",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_ENG_PKT_LEN_UNUSED_FIELD_MASK,
    0,
    NATC_ENG_PKT_LEN_UNUSED_FIELD_WIDTH,
    NATC_ENG_PKT_LEN_UNUSED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_PKT_LEN_PKT_LEN
 ******************************************************************************/
const ru_field_rec NATC_ENG_PKT_LEN_PKT_LEN_FIELD =
{
    "PKT_LEN",
#if RU_INCLUDE_DESC
    "",
    "16-bit packet length value used to increment or decrement byte counter",
#endif
    NATC_ENG_PKT_LEN_PKT_LEN_FIELD_MASK,
    0,
    NATC_ENG_PKT_LEN_PKT_LEN_FIELD_WIDTH,
    NATC_ENG_PKT_LEN_PKT_LEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_ENG_KEY_RESULT_NAT_KEY_RESULT
 ******************************************************************************/
const ru_field_rec NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD =
{
    "NAT_KEY_RESULT",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD_MASK,
    0,
    NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD_WIDTH,
    NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: NATC_ENG_COMMAND_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_COMMAND_STATUS_FIELDS[] =
{
    &NATC_ENG_COMMAND_STATUS_RESERVED0_FIELD,
    &NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD,
    &NATC_ENG_COMMAND_STATUS_RESERVED1_FIELD,
    &NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD,
    &NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD,
    &NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD,
    &NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD,
    &NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD,
    &NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD,
    &NATC_ENG_COMMAND_STATUS_MISS_FIELD,
    &NATC_ENG_COMMAND_STATUS_ERROR_FIELD,
    &NATC_ENG_COMMAND_STATUS_BUSY_FIELD,
    &NATC_ENG_COMMAND_STATUS_UNUSED10_FIELD,
    &NATC_ENG_COMMAND_STATUS_COMMAND_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_ENG_COMMAND_STATUS_REG = 
{
    "COMMAND_STATUS",
#if RU_INCLUDE_DESC
    "NAT3 command & status register",
    "NAT Command and Status Register",
#endif
    NATC_ENG_COMMAND_STATUS_REG_OFFSET,
    0,
    0,
    1094,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    14,
    NATC_ENG_COMMAND_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_ENG_HASH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_HASH_FIELDS[] =
{
    &NATC_ENG_HASH_HASH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_ENG_HASH_REG = 
{
    "HASH",
#if RU_INCLUDE_DESC
    "NAT3 Hash Value",
    "NAT Hash Value",
#endif
    NATC_ENG_HASH_REG_OFFSET,
    0,
    0,
    1095,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_HASH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_ENG_HIT_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_HIT_COUNT_FIELDS[] =
{
    &NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_ENG_HIT_COUNT_REG = 
{
    "HIT_COUNT",
#if RU_INCLUDE_DESC
    "NAT3 Session Hit Count",
    "Hit Count",
#endif
    NATC_ENG_HIT_COUNT_REG_OFFSET,
    0,
    0,
    1096,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_HIT_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_ENG_BYTE_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_BYTE_COUNT_FIELDS[] =
{
    &NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_ENG_BYTE_COUNT_REG = 
{
    "BYTE_COUNT",
#if RU_INCLUDE_DESC
    "NAT3 Session Byte Count",
    "Byte Count",
#endif
    NATC_ENG_BYTE_COUNT_REG_OFFSET,
    0,
    0,
    1097,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_BYTE_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_ENG_PKT_LEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_PKT_LEN_FIELDS[] =
{
    &NATC_ENG_PKT_LEN_UNUSED_FIELD,
    &NATC_ENG_PKT_LEN_PKT_LEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_ENG_PKT_LEN_REG = 
{
    "PKT_LEN",
#if RU_INCLUDE_DESC
    "NAT3 Packet Length",
    "NAT PKT Length",
#endif
    NATC_ENG_PKT_LEN_REG_OFFSET,
    0,
    0,
    1098,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NATC_ENG_PKT_LEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_ENG_KEY_RESULT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_KEY_RESULT_FIELDS[] =
{
    &NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_ENG_KEY_RESULT_REG = 
{
    "KEY_RESULT",
#if RU_INCLUDE_DESC
    "NAT3 key & result register",
    "NAT key & context (excluding 8-byte counters) register; context is placed after the key;"
    "Key consists of all 0's indicates unused entry in hardware and therefore should not be"
    "used for lookup/hash/del/add commands; Key should contain an encoded type field"
    "(e.g. Unused=0, IPv4=1, IPv4m=2, IPv6=3, IPv6m=4, etc) to uniquely identify"
    "each key so the keys with shorter length of one type is not misidentified"
    "as the longer key of another type when the shorter key matches the beginning"
    "of the longer key.  When multiple DDR table mode is used, DDR table number should built"
    "into the key so the same real key in 2 different DDR tables can be distinguished."
    "For HASH debug command, this register stores the nth multi-hash value;"
    "For 256_BIT_MODE 10-bit hash value is store at bit 14:5.  For 512_BIT_MODE,"
    "9-bit hash value is stored at bit 14:6; For HASH debug command,"
    "hashes for HASH command issued using NAT0 register are returned"
    "at NAT1 KEY_RESULT registers; hashes for NAT1 HASH command"
    "are returned at NAT0 KEY_RESULT; hashes for NAT2 HASH command are"
    "returned at NAT3 KEY_RESULT; hashes for NAT3 HASH command are"
    "returned at NAT2 KEY_RESULT register.",
#endif
    NATC_ENG_KEY_RESULT_REG_OFFSET,
    NATC_ENG_KEY_RESULT_REG_RAM_CNT,
    4,
    1099,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_KEY_RESULT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: NATC_ENG
 ******************************************************************************/
static const ru_reg_rec *NATC_ENG_REGS[] =
{
    &NATC_ENG_COMMAND_STATUS_REG,
    &NATC_ENG_HASH_REG,
    &NATC_ENG_HIT_COUNT_REG,
    &NATC_ENG_BYTE_COUNT_REG,
    &NATC_ENG_PKT_LEN_REG,
    &NATC_ENG_KEY_RESULT_REG,
};

unsigned long NATC_ENG_ADDRS[] =
{
    0x82e50010,
    0x82e500b0,
    0x82e50150,
    0x82e501f0,
};

const ru_block_rec NATC_ENG_BLOCK = 
{
    "NATC_ENG",
    NATC_ENG_ADDRS,
    4,
    6,
    NATC_ENG_REGS
};

/* End of file XRDP_NATC_ENG.c */
