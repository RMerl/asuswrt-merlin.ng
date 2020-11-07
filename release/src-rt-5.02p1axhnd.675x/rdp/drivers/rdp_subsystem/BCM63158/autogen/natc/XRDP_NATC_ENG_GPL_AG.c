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

/******************************************************************************
 * Register: NATC_ENG_COMMAND_STATUS
 ******************************************************************************/
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
    936,
};

/******************************************************************************
 * Register: NATC_ENG_HASH
 ******************************************************************************/
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
    937,
};

/******************************************************************************
 * Register: NATC_ENG_HIT_COUNT
 ******************************************************************************/
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
    938,
};

/******************************************************************************
 * Register: NATC_ENG_BYTE_COUNT
 ******************************************************************************/
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
    939,
};

/******************************************************************************
 * Register: NATC_ENG_PKT_LEN
 ******************************************************************************/
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
    940,
};

/******************************************************************************
 * Register: NATC_ENG_KEY_RESULT
 ******************************************************************************/
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
    941,
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
    0x82e500c0,
    0x82e50170,
    0x82e50220,
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
