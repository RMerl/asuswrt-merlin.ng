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
 * Field: NATC_ENG_COMMAND_STATUS_NAT_TBL
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD =
{
    "NAT_TBL",
#if RU_INCLUDE_DESC
    "",
    "NAT Table Number."
    "0h: NAT table 0."
    "1h: NAT Table 1."
    "2h: NAT table 2."
    "3h: NAT Table 3."
    "4h: NAT table 4."
    "5h: NAT Table 5."
    "6h: NAT table 6."
    "7h: NAT Table 7.",
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
    "Multi-hash iteration count"
    "Value of 0 is iteration 1, 1 is iteration 2, 2 is iteration 3, etc.",
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
    "This bit is set when a LOOKUP command misses",
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
    "For Add command, all multi-hash entries are occupied"
    "For Del command, session is not found and cannot be deleted",
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
    "This bit is set when command is issued but the command is still in process.",
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
 * Field: NATC_ENG_COMMAND_STATUS_UNUSED0
 ******************************************************************************/
const ru_field_rec NATC_ENG_COMMAND_STATUS_UNUSED0_FIELD =
{
    "UNUSED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NATC_ENG_COMMAND_STATUS_UNUSED0_FIELD_MASK,
    0,
    NATC_ENG_COMMAND_STATUS_UNUSED0_FIELD_WIDTH,
    NATC_ENG_COMMAND_STATUS_UNUSED0_FIELD_SHIFT,
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
    "This command only operates on the entries in local cache; DDR accesses"
    "The only DDR accesses supported using this register interface is Lookup command when a miss occurs."
    "Writing to COMMAND bits causes BUSY bit to be set;"
    "Note: For all commands, key consists of all 0's means unused entry in h/w"
    "and therefore cannot be used."
    "NOP"
    "Lookup"
    "Add"
    "Del"
    "Hash (debug feature)"
    "Hashes are stored in different set of COMMAND_STATUS register (i.e."
    "Hashes for HASH command issued using NAT0 register are returned"
    "at NAT1 KEY_RESULT registers; hashes for NAT1 HASH command"
    "are returned at NAT0 KEY_RESULT; hashes for NAT2 HASH command are"
    "returned at NAT3 KEY_RESULT; hashes for NAT3 HASH command are"
    "returned at NAT2 KEY_RESULT register)."
    "Internal Cache command (debug feature)"
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
 * Field: NATC_ENG_KEY_MASK_KEY_MASK
 ******************************************************************************/
const ru_field_rec NATC_ENG_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each byte in the key."
    "each bit corresponds to one byte."
    "0 is to enable the compare. 1 is to disable the compare."
    "bit 0 is for byte 0"
    "bit 1 is for byte 1"
    "bit 2 is for byte 2"
    "......................"
    "bit 31 is for byte 31",
#endif
    NATC_ENG_KEY_MASK_KEY_MASK_FIELD_MASK,
    0,
    NATC_ENG_KEY_MASK_KEY_MASK_FIELD_WIDTH,
    NATC_ENG_KEY_MASK_KEY_MASK_FIELD_SHIFT,
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
    "for 1024 entries MODE 10-bit hash value is stored at bit 14 down to bit 5"
    "for 512 enries MODE  9-bit hash value is stored at bit 14 down to bit 6 and"
    "bit 5 is 0."
    "for a miss, this register stores hash value from the first hash.",
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
    "32-bit session hit count value; only valid on a successful lookup command",
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
    "32-bit byte count value; only valid on a successful lookup command",
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
    "16-bit packet length value",
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
    &NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD,
    &NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD,
    &NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD,
    &NATC_ENG_COMMAND_STATUS_MISS_FIELD,
    &NATC_ENG_COMMAND_STATUS_ERROR_FIELD,
    &NATC_ENG_COMMAND_STATUS_BUSY_FIELD,
    &NATC_ENG_COMMAND_STATUS_UNUSED0_FIELD,
    &NATC_ENG_COMMAND_STATUS_COMMAND_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_ENG_COMMAND_STATUS_REG = 
{
    "COMMAND_STATUS",
#if RU_INCLUDE_DESC
    "NAT command & status register",
    "NAT Command and Status Register",
#endif
    NATC_ENG_COMMAND_STATUS_REG_OFFSET,
    0,
    0,
    1108,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    NATC_ENG_COMMAND_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_ENG_KEY_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_KEY_MASK_FIELDS[] =
{
    &NATC_ENG_KEY_MASK_KEY_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_ENG_KEY_MASK_REG = 
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "NAT key mask register",
    "NAT key Mask Register",
#endif
    NATC_ENG_KEY_MASK_REG_OFFSET,
    0,
    0,
    1109,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_KEY_MASK_FIELDS
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
    "NAT Hash Value",
    "NAT Hash Value",
#endif
    NATC_ENG_HASH_REG_OFFSET,
    0,
    0,
    1110,
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
    "NAT Session Hit Count",
    "NAT Hit Count",
#endif
    NATC_ENG_HIT_COUNT_REG_OFFSET,
    0,
    0,
    1111,
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
    "NAT Session Byte Count",
    "NAT BYTE Count",
#endif
    NATC_ENG_BYTE_COUNT_REG_OFFSET,
    0,
    0,
    1112,
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
    "NAT Packet Length",
    "NAT PKT Length",
#endif
    NATC_ENG_PKT_LEN_REG_OFFSET,
    0,
    0,
    1113,
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
    "NAT key & result register",
    "NAT key & result (excluding 4-byte byte count and 4-byte hit-count) register; result is placed after the key;"
    "Key consists of all 0's means unused entry in h/w and therefore should not be"
    "used for lookup/hash/del/add commands; Key should contain an encoded type field"
    "(e.g. Unused=0, IPv4=1, IPv4m=2, IPv6=3, IPv6m=4, etc) to uniquely identify"
    "each key so the keys with shorter length of one type do not get misidentified"
    "as the longer key of another type when the shorter key matches the beginning"
    "of the longer key."
    "For HASH command (debug feature) this register stores the nth multi-hash value;"
    "For 256_BIT_MODE 10-bit hash value is store at bit 14:5.  For 512_BIT_MODE,"
    "9-bit hash value is stored at bit 14:6;"
    "Hashes for HASH command issued using NAT0 register are returned"
    "at NAT1 KEY_RESULT registers; hashes for NAT1 HASH command"
    "are returned at NAT0 KEY_RESULT; hashes for NAT2 HASH command are"
    "returned at NAT3 KEY_RESULT; hashes for NAT3 HASH command are"
    "returned at NAT2 KEY_RESULT register).",
#endif
    NATC_ENG_KEY_RESULT_REG_OFFSET,
    NATC_ENG_KEY_RESULT_REG_RAM_CNT,
    4,
    1114,
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
    &NATC_ENG_KEY_MASK_REG,
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
    7,
    NATC_ENG_REGS
};

/* End of file XRDP_NATC_ENG.c */
