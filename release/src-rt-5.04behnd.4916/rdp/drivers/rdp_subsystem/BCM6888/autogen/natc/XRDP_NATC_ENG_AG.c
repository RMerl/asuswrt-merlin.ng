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


#include "XRDP_NATC_ENG_AG.h"

/******************************************************************************
 * Register: NAME: NATC_ENG_COMMAND_STATUS, TYPE: Type_NAT_COMMAND_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COMMAND *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_COMMAND_FIELD =
{
    "COMMAND",
#if RU_INCLUDE_DESC
    "",
    "Command to be executed\nThis command only operates on the entries in cache except LOOKUP command where\nentry can be fetched from DDR.\nWriting to this field causes BUSY bit to be set.\nNote: For all commands, key consists of all 0's indicates unused entry in h/w\nand therefore cannot be used.\n",
#endif
    { NATC_ENG_COMMAND_STATUS_COMMAND_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_COMMAND_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_COMMAND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED5 *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_UNUSED5_FIELD =
{
    "UNUSED5",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { NATC_ENG_COMMAND_STATUS_UNUSED5_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_UNUSED5_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_UNUSED5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "Interface Busy\nThis bit is set when command is issued but still in progress been processed.\nWhen command completes this bit will be cleared.\n",
#endif
    { NATC_ENG_COMMAND_STATUS_BUSY_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_BUSY_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ERROR *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_ERROR_FIELD =
{
    "ERROR",
#if RU_INCLUDE_DESC
    "",
    "This bit is set for the following 2 cases\nFor ADD command all multi-hash entries are occupied (i.e, no room to ADD)\nFor DEL command entry is not found and cannot be deleted\n",
#endif
    { NATC_ENG_COMMAND_STATUS_ERROR_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_ERROR_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_ERROR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MISS *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_MISS_FIELD =
{
    "MISS",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when a LOOKUP command has a miss\n",
#endif
    { NATC_ENG_COMMAND_STATUS_MISS_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_MISS_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_MISS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_HIT *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD =
{
    "CACHE_HIT",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when a LOOKUP command has a cache hit\n",
#endif
    { NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTIHASH_COUNT *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD =
{
    "MULTIHASH_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cache multi-hash iteration count status\nValue of 0 is iteration 1, 1 is iteration 2, 2 is iteration 3, etc.\ncache miss returns 0 count.\n",
#endif
    { NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_TBL *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD =
{
    "NAT_TBL",
#if RU_INCLUDE_DESC
    "",
    "Select the DDR Table on which the command will operate\n0h: DDR table 0\n1h: DDR table 1\n2h: DDR table 2\n3h: DDR table 3\n4h: DDR table 4\n5h: DDR table 5\n6h: DDR table 6\n7h: DDR table 7\n",
#endif
    { NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DECR_COUNT *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD =
{
    "DECR_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Decrement-counter mode enable\nWhen set, LOOKUP command will decrement hit counter by 1 and decrement\nbyte counter by the value specified in PKT_LEN, on a successful lookup.\nNATC_SMEM_INCREMENT_ON_REG_LOOKUP must be set to 1 for it to be effective\n0h: LOOKUP command will increment hit counter and byte counter\n1h: LOOKUP command will decrement hit counter and byte counter\n",
#endif
    { NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_FLUSH *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD =
{
    "CACHE_FLUSH",
#if RU_INCLUDE_DESC
    "",
    "Cache Flush enable\nWhen set, LOOKUP command is used to flush counters from cache into DDR.\nThis command does not use key to lookup the cache entry.  Instead it uses\ncache index number located in 10-MSB bits of key specified in NAT_KEY_RESULT register.\nFor 16 bytes key, the cache index will be located in\n{NAT_KEY_RESULT[15], NAT_KEY_RESULT[14][7:6]} (15th byte of NAT_KEY_RESULT register and\nbits 7:6 of 14th byte of NAT_KEY_RESULT register).\nFor 32 bytes key, the cache index will be located in\n{NAT_KEY_RESULT[31], NAT_KEY_RESULT[30][7:6]} (31th byte of NAT_KEY_RESULT register and\nbits 7:6 of 30th byte of NAT_KEY_RESULT register).\n0h: LOOKUP command is used as normal lookup command.\n1h: LOOKUP command is used as cache flush command.\n",
#endif
    { NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEL_CMD_MODE *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD =
{
    "DEL_CMD_MODE",
#if RU_INCLUDE_DESC
    "",
    "DEL Command DDR-bin matching mode enable\n0h: DEL command deletes the cache entry with matching key\n1h: DEL command deletes the cache entry with matching key and matching DDR bin\nnumber specified in DEL_CMD_DDR_BIN field\n",
#endif
    { NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADD_CMD_MODE *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_ADD_CMD_MODE_FIELD =
{
    "ADD_CMD_MODE",
#if RU_INCLUDE_DESC
    "",
    "ADD Command mode\n0h: ADD command writes 0 (DDR bin number and DDR miss flag) to cache.\n1h: ADD command writes DDR bin number and DDR miss flag to cache\nspecified in ADD_CMD_DDR_BIN and ADD_CMD_DDR_MISS fields.\n",
#endif
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_MODE_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_MODE_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEL_CMD_DDR_BIN *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD =
{
    "DEL_CMD_DDR_BIN",
#if RU_INCLUDE_DESC
    "",
    "This filed specifies the DDR BIN number to be compared for DEL command\nwhen DEL_CMD_MODE is set to 1\n",
#endif
    { NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADD_CMD_DDR_BIN *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD =
{
    "ADD_CMD_DDR_BIN",
#if RU_INCLUDE_DESC
    "",
    "This filed specifies the DDR BIN number to be written to cache for ADD command\nwhen ADD_CMD_MODE is set to 1.\n",
#endif
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADD_CMD_DDR_MISS *****/
const ru_field_rec NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD =
{
    "ADD_CMD_DDR_MISS",
#if RU_INCLUDE_DESC
    "",
    "This filed specifies the DDR MISS flag to be written to cache for ADD command\nwhen ADD_CMD_MODE is set to 1.\n",
#endif
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD_MASK },
    0,
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD_WIDTH },
    { NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_COMMAND_STATUS_FIELDS[] =
{
    &NATC_ENG_COMMAND_STATUS_COMMAND_FIELD,
    &NATC_ENG_COMMAND_STATUS_UNUSED5_FIELD,
    &NATC_ENG_COMMAND_STATUS_BUSY_FIELD,
    &NATC_ENG_COMMAND_STATUS_ERROR_FIELD,
    &NATC_ENG_COMMAND_STATUS_MISS_FIELD,
    &NATC_ENG_COMMAND_STATUS_CACHE_HIT_FIELD,
    &NATC_ENG_COMMAND_STATUS_MULTIHASH_COUNT_FIELD,
    &NATC_ENG_COMMAND_STATUS_NAT_TBL_FIELD,
    &NATC_ENG_COMMAND_STATUS_DECR_COUNT_FIELD,
    &NATC_ENG_COMMAND_STATUS_CACHE_FLUSH_FIELD,
    &NATC_ENG_COMMAND_STATUS_DEL_CMD_MODE_FIELD,
    &NATC_ENG_COMMAND_STATUS_ADD_CMD_MODE_FIELD,
    &NATC_ENG_COMMAND_STATUS_DEL_CMD_DDR_BIN_FIELD,
    &NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_BIN_FIELD,
    &NATC_ENG_COMMAND_STATUS_ADD_CMD_DDR_MISS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_ENG_COMMAND_STATUS *****/
const ru_reg_rec NATC_ENG_COMMAND_STATUS_REG =
{
    "COMMAND_STATUS",
#if RU_INCLUDE_DESC
    "NAT0 command & status register",
    "NAT Command and Status Register\n",
#endif
    { NATC_ENG_COMMAND_STATUS_REG_OFFSET },
    0,
    0,
    642,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    NATC_ENG_COMMAND_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_ENG_HASH, TYPE: Type_NAT_HASH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH *****/
const ru_field_rec NATC_ENG_HASH_HASH_FIELD =
{
    "HASH",
#if RU_INCLUDE_DESC
    "",
    "hash value; only valid on a successful lookup/add/del command\nFor cache hit 10-bit hash value is returned.\nFor cache miss and DDR_ENABLE is 0, first hash value (10-bit) is returned.\nFor cache miss, DDR_ENABLE is 1 and DDR is a hit, 18-bit DDR hash value + DDR bin count is returned.\nFor cache miss, DDR_ENABLE is 1 and DDR is a miss, 18-bit DDR hash value is returned.\n",
#endif
    { NATC_ENG_HASH_HASH_FIELD_MASK },
    0,
    { NATC_ENG_HASH_HASH_FIELD_WIDTH },
    { NATC_ENG_HASH_HASH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_HASH_FIELDS[] =
{
    &NATC_ENG_HASH_HASH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_ENG_HASH *****/
const ru_reg_rec NATC_ENG_HASH_REG =
{
    "HASH",
#if RU_INCLUDE_DESC
    "NAT0 Hash Value",
    "NAT Hash Value\n",
#endif
    { NATC_ENG_HASH_REG_OFFSET },
    0,
    0,
    643,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_HASH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_ENG_HIT_COUNT, TYPE: Type_NAT_HIT_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HIT_COUNT *****/
const ru_field_rec NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD =
{
    "HIT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "bits 27:0 are 28-bit hit count value.\nbits 31:28 are 4 lsb of 36-bit byte count value.\nonly valid on a successful lookup or delete command.\n",
#endif
    { NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD_MASK },
    0,
    { NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD_WIDTH },
    { NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_HIT_COUNT_FIELDS[] =
{
    &NATC_ENG_HIT_COUNT_HIT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_ENG_HIT_COUNT *****/
const ru_reg_rec NATC_ENG_HIT_COUNT_REG =
{
    "HIT_COUNT",
#if RU_INCLUDE_DESC
    "NAT0 Session Hit Count",
    "Hit Count\n",
#endif
    { NATC_ENG_HIT_COUNT_REG_OFFSET },
    0,
    0,
    644,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_HIT_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_ENG_BYTE_COUNT, TYPE: Type_NAT_BYTE_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYTE_COUNT *****/
const ru_field_rec NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD =
{
    "BYTE_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit msb of 36-bit byte count value.\n{BYTE_COUNT, HIT_COUNT[31:28]} is the 36-bit byte count value.\nonly valid on a successful lookup or delete command\n",
#endif
    { NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD_MASK },
    0,
    { NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD_WIDTH },
    { NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_BYTE_COUNT_FIELDS[] =
{
    &NATC_ENG_BYTE_COUNT_BYTE_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_ENG_BYTE_COUNT *****/
const ru_reg_rec NATC_ENG_BYTE_COUNT_REG =
{
    "BYTE_COUNT",
#if RU_INCLUDE_DESC
    "NAT0 Session Byte Count",
    "Byte Count\n",
#endif
    { NATC_ENG_BYTE_COUNT_REG_OFFSET },
    0,
    0,
    645,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_BYTE_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_ENG_PKT_LEN, TYPE: Type_NAT_PKT_LEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PKT_LEN *****/
const ru_field_rec NATC_ENG_PKT_LEN_PKT_LEN_FIELD =
{
    "PKT_LEN",
#if RU_INCLUDE_DESC
    "",
    "16-bit packet length value used to increment or decrement byte counter\n",
#endif
    { NATC_ENG_PKT_LEN_PKT_LEN_FIELD_MASK },
    0,
    { NATC_ENG_PKT_LEN_PKT_LEN_FIELD_WIDTH },
    { NATC_ENG_PKT_LEN_PKT_LEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNUSED *****/
const ru_field_rec NATC_ENG_PKT_LEN_UNUSED_FIELD =
{
    "UNUSED",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { NATC_ENG_PKT_LEN_UNUSED_FIELD_MASK },
    0,
    { NATC_ENG_PKT_LEN_UNUSED_FIELD_WIDTH },
    { NATC_ENG_PKT_LEN_UNUSED_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_PKT_LEN_FIELDS[] =
{
    &NATC_ENG_PKT_LEN_PKT_LEN_FIELD,
    &NATC_ENG_PKT_LEN_UNUSED_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_ENG_PKT_LEN *****/
const ru_reg_rec NATC_ENG_PKT_LEN_REG =
{
    "PKT_LEN",
#if RU_INCLUDE_DESC
    "NAT0 Packet Length",
    "NAT PKT Length\n",
#endif
    { NATC_ENG_PKT_LEN_REG_OFFSET },
    0,
    0,
    646,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NATC_ENG_PKT_LEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_ENG_KEY_RESULT, TYPE: Type_NAT_KEY_RESULT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NAT_KEY_RESULT *****/
const ru_field_rec NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD =
{
    "NAT_KEY_RESULT",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD_MASK },
    0,
    { NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD_WIDTH },
    { NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_ENG_KEY_RESULT_FIELDS[] =
{
    &NATC_ENG_KEY_RESULT_NAT_KEY_RESULT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_ENG_KEY_RESULT *****/
const ru_reg_rec NATC_ENG_KEY_RESULT_REG =
{
    "KEY_RESULT",
#if RU_INCLUDE_DESC
    "NAT0 key & result register",
    "NAT key & context (excluding 8-byte counters) register; context is placed after the key;\nKey consists of all 0's indicates unused entry in hardware and therefore should not be\nused for lookup/hash/del/add commands; Key should contain an encoded type field\n(e.g. Unused=0, IPv4=1, IPv4m=2, IPv6=3, IPv6m=4, etc) to uniquely identify\neach key so the keys with shorter length of one type is not misidentified\nas the longer key of another type when the shorter key matches the beginning\nof the longer key.  When multiple DDR table mode is used, DDR table number should built\ninto the key so the same real key in 2 different DDR tables can be distinguished.\n",
#endif
    { NATC_ENG_KEY_RESULT_REG_OFFSET },
    NATC_ENG_KEY_RESULT_REG_RAM_CNT,
    4,
    647,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_ENG_KEY_RESULT_FIELDS,
#endif
};

unsigned long NATC_ENG_ADDRS[] =
{
    0x82950000,
};

static const ru_reg_rec *NATC_ENG_REGS[] =
{
    &NATC_ENG_COMMAND_STATUS_REG,
    &NATC_ENG_HASH_REG,
    &NATC_ENG_HIT_COUNT_REG,
    &NATC_ENG_BYTE_COUNT_REG,
    &NATC_ENG_PKT_LEN_REG,
    &NATC_ENG_KEY_RESULT_REG,
};

const ru_block_rec NATC_ENG_BLOCK =
{
    "NATC_ENG",
    NATC_ENG_ADDRS,
    1,
    6,
    NATC_ENG_REGS,
};
