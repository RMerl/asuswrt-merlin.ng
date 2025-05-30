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


#include "XRDP_SBPM_AG.h"

/******************************************************************************
 * Register: NAME: SBPM_REGS_INIT_FREE_LIST, TYPE: Type_SBPM_SBPM_REGS_INIT_FREE_LIST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_BASE_ADDR *****/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD =
{
    "INIT_BASE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "init_base_addr\n",
#endif
    { SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD_MASK },
    0,
    { SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD_WIDTH },
    { SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_OFFSET *****/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD =
{
    "INIT_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "init_offset\n",
#endif
    { SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD_MASK },
    0,
    { SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD_WIDTH },
    { SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD_SHIFT },
    2047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BSY *****/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "",
    "The bit is used  as busy  indication of buffer allocation request status (busy status)  by CPU.\nBPM asserts this bit on each valid request and de-asserts when request is treated.\n",
#endif
    { SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_MASK },
    0,
    { SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_WIDTH },
    { SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "The bit is used  as ready indication of buffer allocation request status (ready status)  by CPU.\nBPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is READY indication\n",
#endif
    { SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_MASK },
    0,
    { SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_WIDTH },
    { SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_INIT_FREE_LIST_FIELDS[] =
{
    &SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD,
    &SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD,
    &SBPM_REGS_INIT_FREE_LIST_BSY_FIELD,
    &SBPM_REGS_INIT_FREE_LIST_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_INIT_FREE_LIST *****/
const ru_reg_rec SBPM_REGS_INIT_FREE_LIST_REG =
{
    "REGS_INIT_FREE_LIST",
#if RU_INCLUDE_DESC
    "INIT_FREE_LIST Register",
    "request for building the free list using HW accelerator\n",
#endif
    { SBPM_REGS_INIT_FREE_LIST_REG_OFFSET },
    0,
    0,
    998,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_INIT_FREE_LIST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_ALLOC, TYPE: Type_SBPM_SBPM_REGS_BN_ALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SA *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_SA_FIELD =
{
    "SA",
#if RU_INCLUDE_DESC
    "",
    "Source address used by Alloc BN command (may be used for alloc on behalf another user)\n",
#endif
    { SBPM_REGS_BN_ALLOC_SA_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_SA_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_SA_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_ALLOC_FIELDS[] =
{
    &SBPM_REGS_BN_ALLOC_SA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_ALLOC *****/
const ru_reg_rec SBPM_REGS_BN_ALLOC_REG =
{
    "REGS_BN_ALLOC",
#if RU_INCLUDE_DESC
    "BN_ALLOC Register",
    "request for a new buffer\n",
#endif
    { SBPM_REGS_BN_ALLOC_REG_OFFSET },
    0,
    0,
    999,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_BN_ALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_ALLOC_RPLY, TYPE: Type_SBPM_SBPM_REGS_BN_ALLOC_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_BN_VALID *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD =
{
    "ALLOC_BN_VALID",
#if RU_INCLUDE_DESC
    "",
    "alloc_bn_valid\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_BN *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD =
{
    "ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "alloc_bn\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD =
{
    "ACK",
#if RU_INCLUDE_DESC
    "",
    "ack\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NACK *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD =
{
    "NACK",
#if RU_INCLUDE_DESC
    "",
    "nack\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_HIGH *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD =
{
    "EXCL_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Exclusive bit is indication of Exclusive_high status of client with related Alloc request\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_LOW *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD =
{
    "EXCL_LOW",
#if RU_INCLUDE_DESC
    "",
    "Exclusive bit is indication of Exclusive_low status of client with related Alloc request\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "busy\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "rdy\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_MASK },
    0,
    { SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_WIDTH },
    { SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_ALLOC_RPLY_FIELDS[] =
{
    &SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_ALLOC_RPLY *****/
const ru_reg_rec SBPM_REGS_BN_ALLOC_RPLY_REG =
{
    "REGS_BN_ALLOC_RPLY",
#if RU_INCLUDE_DESC
    "BN_ALLOC_RPLY Register",
    "reply for a new buffer alloc\n",
#endif
    { SBPM_REGS_BN_ALLOC_RPLY_REG_OFFSET },
    0,
    0,
    1000,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    SBPM_REGS_BN_ALLOC_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW, TYPE: Type_SBPM_SBPM_REGS_BN_FREE_WITH_CONTXT_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HEAD_BN *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD =
{
    "HEAD_BN",
#if RU_INCLUDE_DESC
    "",
    "head_bn\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SA *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD =
{
    "SA",
#if RU_INCLUDE_DESC
    "",
    "Source addres used for free comand (may be used for freeing BN on behalf another port)\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OFFSET *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD =
{
    "OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Offset (or length) = number of BNs in packet that is going to be freed\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD =
{
    "ACK",
#if RU_INCLUDE_DESC
    "",
    "Ack request\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW *****/
const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_REG =
{
    "REGS_BN_FREE_WITH_CONTXT_LOW",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_LOW Register",
    "Request for freeing buffers of a packet offline with context (lower 32-bit)\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_REG_OFFSET },
    0,
    0,
    1001,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH, TYPE: Type_SBPM_SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: LAST_BN *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD =
{
    "LAST_BN",
#if RU_INCLUDE_DESC
    "",
    "Last BN in packet that is going to be freed\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH *****/
const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_REG =
{
    "REGS_BN_FREE_WITH_CONTXT_HIGH",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_HIGH Register",
    "Request for freeing buffers of a packet offline with context (higher 32-bit)\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_REG_OFFSET },
    0,
    0,
    1002,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_MCST_INC, TYPE: Type_SBPM_SBPM_REGS_MCST_INC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BN *****/
const ru_field_rec SBPM_REGS_MCST_INC_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "",
    "bufer number\n",
#endif
    { SBPM_REGS_MCST_INC_BN_FIELD_MASK },
    0,
    { SBPM_REGS_MCST_INC_BN_FIELD_WIDTH },
    { SBPM_REGS_MCST_INC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MCST_VAL *****/
const ru_field_rec SBPM_REGS_MCST_INC_MCST_VAL_FIELD =
{
    "MCST_VAL",
#if RU_INCLUDE_DESC
    "",
    "MCST value that should be added to current mulicast counter\n",
#endif
    { SBPM_REGS_MCST_INC_MCST_VAL_FIELD_MASK },
    0,
    { SBPM_REGS_MCST_INC_MCST_VAL_FIELD_WIDTH },
    { SBPM_REGS_MCST_INC_MCST_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_REQ *****/
const ru_field_rec SBPM_REGS_MCST_INC_ACK_REQ_FIELD =
{
    "ACK_REQ",
#if RU_INCLUDE_DESC
    "",
    "Acknowledge request\n",
#endif
    { SBPM_REGS_MCST_INC_ACK_REQ_FIELD_MASK },
    0,
    { SBPM_REGS_MCST_INC_ACK_REQ_FIELD_WIDTH },
    { SBPM_REGS_MCST_INC_ACK_REQ_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_MCST_INC_FIELDS[] =
{
    &SBPM_REGS_MCST_INC_BN_FIELD,
    &SBPM_REGS_MCST_INC_MCST_VAL_FIELD,
    &SBPM_REGS_MCST_INC_ACK_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_MCST_INC *****/
const ru_reg_rec SBPM_REGS_MCST_INC_REG =
{
    "REGS_MCST_INC",
#if RU_INCLUDE_DESC
    "MCST_INC Register",
    "Multicast counter increment. Contains the BN, which is head of the packet to be multicast and its counter value\n",
#endif
    { SBPM_REGS_MCST_INC_REG_OFFSET },
    0,
    0,
    1003,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_MCST_INC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_MCST_INC_RPLY, TYPE: Type_SBPM_SBPM_REGS_MCST_INC_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MCST_ACK *****/
const ru_field_rec SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD =
{
    "MCST_ACK",
#if RU_INCLUDE_DESC
    "",
    "Acknowledge reply of MCST command\n",
#endif
    { SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_WIDTH },
    { SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BSY *****/
const ru_field_rec SBPM_REGS_MCST_INC_RPLY_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "",
    "The bit is used  as busy  indication of MCST request status (busy status)  by CPU\nSBPM asserts this bit on each valid request and de-asserts when request is treated:\n1 - request is busy,\n0- request is not busy (ready)\n",
#endif
    { SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_MASK },
    0,
    { SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_WIDTH },
    { SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec SBPM_REGS_MCST_INC_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "The bit is used  as ready indication of MCST request status (ready status)  by CPU.\nSBPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is READY indication:\n1 - request is ready,\n0- request is not ready (busy)\n",
#endif
    { SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_MASK },
    0,
    { SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_WIDTH },
    { SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_MCST_INC_RPLY_FIELDS[] =
{
    &SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD,
    &SBPM_REGS_MCST_INC_RPLY_BSY_FIELD,
    &SBPM_REGS_MCST_INC_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_MCST_INC_RPLY *****/
const ru_reg_rec SBPM_REGS_MCST_INC_RPLY_REG =
{
    "REGS_MCST_INC_RPLY",
#if RU_INCLUDE_DESC
    "MCST_INC_RPLY Register",
    "mcst_inc_rply\n",
#endif
    { SBPM_REGS_MCST_INC_RPLY_REG_OFFSET },
    0,
    0,
    1004,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_MCST_INC_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_CONNECT, TYPE: Type_SBPM_SBPM_REGS_BN_CONNECT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BN *****/
const ru_field_rec SBPM_REGS_BN_CONNECT_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "",
    "bn\n",
#endif
    { SBPM_REGS_BN_CONNECT_BN_FIELD_MASK },
    0,
    { SBPM_REGS_BN_CONNECT_BN_FIELD_WIDTH },
    { SBPM_REGS_BN_CONNECT_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_REQ *****/
const ru_field_rec SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD =
{
    "ACK_REQ",
#if RU_INCLUDE_DESC
    "",
    "ack_req for Connect command (should be always set)\n",
#endif
    { SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_MASK },
    0,
    { SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_WIDTH },
    { SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WR_REQ *****/
const ru_field_rec SBPM_REGS_BN_CONNECT_WR_REQ_FIELD =
{
    "WR_REQ",
#if RU_INCLUDE_DESC
    "",
    "Used for Direct Write (for work arround)\n",
#endif
    { SBPM_REGS_BN_CONNECT_WR_REQ_FIELD_MASK },
    0,
    { SBPM_REGS_BN_CONNECT_WR_REQ_FIELD_WIDTH },
    { SBPM_REGS_BN_CONNECT_WR_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POINTED_BN *****/
const ru_field_rec SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD =
{
    "POINTED_BN",
#if RU_INCLUDE_DESC
    "",
    "pointed_bn\n",
#endif
    { SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_MASK },
    0,
    { SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_WIDTH },
    { SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_CONNECT_FIELDS[] =
{
    &SBPM_REGS_BN_CONNECT_BN_FIELD,
    &SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD,
    &SBPM_REGS_BN_CONNECT_WR_REQ_FIELD,
    &SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_CONNECT *****/
const ru_reg_rec SBPM_REGS_BN_CONNECT_REG =
{
    "REGS_BN_CONNECT",
#if RU_INCLUDE_DESC
    "BN_CONNECT Register",
    "request for connection between two buffers in a linked list. The connection request may be replied with ACK message if the ACK request bit is asserted.\nThis command is used as write command.\n",
#endif
    { SBPM_REGS_BN_CONNECT_REG_OFFSET },
    0,
    0,
    1005,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_BN_CONNECT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_CONNECT_RPLY, TYPE: Type_SBPM_SBPM_REGS_BN_CONNECT_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONNECT_ACK *****/
const ru_field_rec SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD =
{
    "CONNECT_ACK",
#if RU_INCLUDE_DESC
    "",
    "Acknowledge reply on Connect request\n",
#endif
    { SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_WIDTH },
    { SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "busy bit\n",
#endif
    { SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_MASK },
    0,
    { SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_WIDTH },
    { SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "ready bit\n",
#endif
    { SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_MASK },
    0,
    { SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_WIDTH },
    { SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_CONNECT_RPLY_FIELDS[] =
{
    &SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD,
    &SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD,
    &SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_CONNECT_RPLY *****/
const ru_reg_rec SBPM_REGS_BN_CONNECT_RPLY_REG =
{
    "REGS_BN_CONNECT_RPLY",
#if RU_INCLUDE_DESC
    "BN_CONNECT_RPLY Register",
    "bn_connect_rply\n",
#endif
    { SBPM_REGS_BN_CONNECT_RPLY_REG_OFFSET },
    0,
    0,
    1006,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_BN_CONNECT_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_GET_NEXT, TYPE: Type_SBPM_SBPM_REGS_GET_NEXT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BN *****/
const ru_field_rec SBPM_REGS_GET_NEXT_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "",
    "Get Next Buffer of current BN (used in this field)\n",
#endif
    { SBPM_REGS_GET_NEXT_BN_FIELD_MASK },
    0,
    { SBPM_REGS_GET_NEXT_BN_FIELD_WIDTH },
    { SBPM_REGS_GET_NEXT_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_GET_NEXT_FIELDS[] =
{
    &SBPM_REGS_GET_NEXT_BN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_GET_NEXT *****/
const ru_reg_rec SBPM_REGS_GET_NEXT_REG =
{
    "REGS_GET_NEXT",
#if RU_INCLUDE_DESC
    "GET_NEXT Register",
    "a pointer to a buffer in a packet linked list and request for the next buffer in the list\nthis command is used as read command.\n",
#endif
    { SBPM_REGS_GET_NEXT_REG_OFFSET },
    0,
    0,
    1007,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_GET_NEXT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_GET_NEXT_RPLY, TYPE: Type_SBPM_SBPM_REGS_GET_NEXT_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BN_VALID *****/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD =
{
    "BN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Used for validation of Next BN reply\n",
#endif
    { SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_MASK },
    0,
    { SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_WIDTH },
    { SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NEXT_BN *****/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD =
{
    "NEXT_BN",
#if RU_INCLUDE_DESC
    "",
    "Next BN - reply of Get_next command\n",
#endif
    { SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_MASK },
    0,
    { SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_WIDTH },
    { SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BN_NULL *****/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD =
{
    "BN_NULL",
#if RU_INCLUDE_DESC
    "",
    "Next BN is null indication\n",
#endif
    { SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD_MASK },
    0,
    { SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD_WIDTH },
    { SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MCNT_VAL *****/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD =
{
    "MCNT_VAL",
#if RU_INCLUDE_DESC
    "",
    "mcst cnt val\n",
#endif
    { SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD_MASK },
    0,
    { SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD_WIDTH },
    { SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "Get Next command is busy\n",
#endif
    { SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_MASK },
    0,
    { SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_WIDTH },
    { SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "Get Next command is ready\n",
#endif
    { SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_MASK },
    0,
    { SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_WIDTH },
    { SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_GET_NEXT_RPLY_FIELDS[] =
{
    &SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_GET_NEXT_RPLY *****/
const ru_reg_rec SBPM_REGS_GET_NEXT_RPLY_REG =
{
    "REGS_GET_NEXT_RPLY",
#if RU_INCLUDE_DESC
    "GET_NEXT_RPLY Register",
    "get_next_rply\n",
#endif
    { SBPM_REGS_GET_NEXT_RPLY_REG_OFFSET },
    0,
    0,
    1008,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    SBPM_REGS_GET_NEXT_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_CLK_GATE_CNTRL, TYPE: Type_SBPM_SBPM_REGS_SBPM_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\n",
#endif
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\n\n",
#endif
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\n",
#endif
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTERVL *****/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD =
{
    "KEEP_ALIVE_INTERVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\n",
#endif
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_WIDTH },
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\n",
#endif
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_CLK_GATE_CNTRL_FIELDS[] =
{
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_CLK_GATE_CNTRL *****/
const ru_reg_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG =
{
    "REGS_SBPM_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "SBPM_CLK_GATE_CNTRL Register",
    "control for the bl_clk_control module\n",
#endif
    { SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    1009,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_FREE_WITHOUT_CONTXT, TYPE: Type_SBPM_SBPM_REGS_BN_FREE_WITHOUT_CONTXT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HEAD_BN *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD =
{
    "HEAD_BN",
#if RU_INCLUDE_DESC
    "",
    "Head BN = First BN in packet that is going to be freed\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SA *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD =
{
    "SA",
#if RU_INCLUDE_DESC
    "",
    "source address used for command (may be used for performing command on behalf another port)\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_REQ *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD =
{
    "ACK_REQ",
#if RU_INCLUDE_DESC
    "",
    "ACK request - should be always set\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITHOUT_CONTXT_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_FREE_WITHOUT_CONTXT *****/
const ru_reg_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG =
{
    "REGS_BN_FREE_WITHOUT_CONTXT",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT Register",
    "bn_free_without_contxt\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG_OFFSET },
    0,
    0,
    1010,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY, TYPE: Type_SBPM_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_ACK *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD =
{
    "FREE_ACK",
#if RU_INCLUDE_DESC
    "",
    "Acknowledge on Free command\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_STAT *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD =
{
    "ACK_STAT",
#if RU_INCLUDE_DESC
    "",
    "ACK status of CPU\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NACK_STAT *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD =
{
    "NACK_STAT",
#if RU_INCLUDE_DESC
    "",
    "NACK status of CPU\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_HIGH_STAT *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD =
{
    "EXCL_HIGH_STAT",
#if RU_INCLUDE_DESC
    "",
    "Exclusive_high status of CPU\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_LOW_STAT *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD =
{
    "EXCL_LOW_STAT",
#if RU_INCLUDE_DESC
    "",
    "Exclusive_low status of CPU\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BSY *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "",
    "Busy bit of command (command is currently in execution)\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "Ready bit of command (ready for new command execution)\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY *****/
const ru_reg_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG =
{
    "REGS_BN_FREE_WITHOUT_CONTXT_RPLY",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT_RPLY Register",
    "bn_free_without_contxt_rply\n",
#endif
    { SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG_OFFSET },
    0,
    0,
    1011,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY, TYPE: Type_SBPM_SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_ACK *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD =
{
    "FREE_ACK",
#if RU_INCLUDE_DESC
    "",
    "Free command acknowledge\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_STATE *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD =
{
    "ACK_STATE",
#if RU_INCLUDE_DESC
    "",
    "ACK status of CPU\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NACK_STATE *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD =
{
    "NACK_STATE",
#if RU_INCLUDE_DESC
    "",
    "NACK status of CPU\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_HIGH_STATE *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD =
{
    "EXCL_HIGH_STATE",
#if RU_INCLUDE_DESC
    "",
    "Exclusive high status of CPU\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_LOW_STATE *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD =
{
    "EXCL_LOW_STATE",
#if RU_INCLUDE_DESC
    "",
    "Exclusive low status of CPU\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "Busy bit of command\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "Ready bit of command\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD_MASK },
    0,
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD_WIDTH },
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY *****/
const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_REG =
{
    "REGS_BN_FREE_WITH_CONTXT_RPLY",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_RPLY Register",
    "bn_free_with_contxt_rply\n",
#endif
    { SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_REG_OFFSET },
    0,
    0,
    1012,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_GL_TRSH, TYPE: Type_SBPM_SBPM_REGS_SBPM_GL_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GL_BAT *****/
const ru_field_rec SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD =
{
    "GL_BAT",
#if RU_INCLUDE_DESC
    "",
    "Global Threshold for Allocated BN = maximal total number of BNs in SBPM\n",
#endif
    { SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD_WIDTH },
    { SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD_SHIFT },
    2047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GL_BAH *****/
const ru_field_rec SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD =
{
    "GL_BAH",
#if RU_INCLUDE_DESC
    "",
    "Global Hysteresis for Allocated BN = hysteresis value related to maximal total threshold of SRAM BNs\n",
#endif
    { SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_GL_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD,
    &SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_GL_TRSH *****/
const ru_reg_rec SBPM_REGS_SBPM_GL_TRSH_REG =
{
    "REGS_SBPM_GL_TRSH",
#if RU_INCLUDE_DESC
    "GLOBAL_THRESHOLD Register",
    "Global Threshold for Allocated Buffers.\nSBPM will issue BN in the accepted range upon to Global threshold setup.\nThs register also holds global hysteresis value for ACK/NACK transition setting. We cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { SBPM_REGS_SBPM_GL_TRSH_REG_OFFSET },
    0,
    0,
    1013,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_GL_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG0_TRSH, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG0_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_BAT *****/
const ru_field_rec SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD =
{
    "UG_BAT",
#if RU_INCLUDE_DESC
    "",
    "Current UG Threshold for Allocated BN\n",
#endif
    { SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD_SHIFT },
    1024,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_BAH *****/
const ru_field_rec SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD =
{
    "UG_BAH",
#if RU_INCLUDE_DESC
    "",
    "Current UG hysteresis Threshold for Allocated BN\n",
#endif
    { SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG0_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD,
    &SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG0_TRSH *****/
const ru_reg_rec SBPM_REGS_SBPM_UG0_TRSH_REG =
{
    "REGS_SBPM_UG0_TRSH",
#if RU_INCLUDE_DESC
    "UG0_THRESHOLD Register",
    "Threshold for Allocated Buffers of UG0\nThs register also holds UG0 hysteresis value for ACK/NACK transition setting.\nWe cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { SBPM_REGS_SBPM_UG0_TRSH_REG_OFFSET },
    0,
    0,
    1014,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG0_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG1_TRSH, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG1_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_BAT *****/
const ru_field_rec SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD =
{
    "UG_BAT",
#if RU_INCLUDE_DESC
    "",
    "Current UG Threshold for Allocated BN\n",
#endif
    { SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD_SHIFT },
    1024,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_BAH *****/
const ru_field_rec SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD =
{
    "UG_BAH",
#if RU_INCLUDE_DESC
    "",
    "Current UG hysteresis delta Threshold for Allocated BN\n",
#endif
    { SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG1_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD,
    &SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG1_TRSH *****/
const ru_reg_rec SBPM_REGS_SBPM_UG1_TRSH_REG =
{
    "REGS_SBPM_UG1_TRSH",
#if RU_INCLUDE_DESC
    "UG1_THRESHOLD Register",
    "Threshold for Allocated Buffers of UG1\nThs register also holds UG1 hysteresis value for ACK/NACK transition setting.\nWe cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { SBPM_REGS_SBPM_UG1_TRSH_REG_OFFSET },
    0,
    0,
    1015,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG1_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_DBG, TYPE: Type_SBPM_SBPM_REGS_SBPM_DBG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SELECT_BUS *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD =
{
    "SELECT_BUS",
#if RU_INCLUDE_DESC
    "",
    "select bus. the bus index should be mentioned in onehot writting:\nbus0 = 0001\nbus1 = 0010\nbus2 = 0100\nbus3 = 1000\n\n",
#endif
    { SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_DBG_FIELDS[] =
{
    &SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_DBG *****/
const ru_reg_rec SBPM_REGS_SBPM_DBG_REG =
{
    "REGS_SBPM_DBG",
#if RU_INCLUDE_DESC
    "SBPM_DBG Register",
    "SBPM select the debug bus\n",
#endif
    { SBPM_REGS_SBPM_DBG_REG_OFFSET },
    0,
    0,
    1016,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_DBG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG0_BAC, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG0_BAC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UG0BAC *****/
const ru_field_rec SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD =
{
    "UG0BAC",
#if RU_INCLUDE_DESC
    "",
    "UG0 counter for allocated BNs\n",
#endif
    { SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG0_BAC_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG0_BAC *****/
const ru_reg_rec SBPM_REGS_SBPM_UG0_BAC_REG =
{
    "REGS_SBPM_UG0_BAC",
#if RU_INCLUDE_DESC
    "SBPM_UG0_BAC Register",
    "SBPM UG0 allocated BN counter\n",
#endif
    { SBPM_REGS_SBPM_UG0_BAC_REG_OFFSET },
    0,
    0,
    1017,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_UG0_BAC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG1_BAC, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG1_BAC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UG1BAC *****/
const ru_field_rec SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD =
{
    "UG1BAC",
#if RU_INCLUDE_DESC
    "",
    "Baffer Allocated Counter\n",
#endif
    { SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG1_BAC_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG1_BAC *****/
const ru_reg_rec SBPM_REGS_SBPM_UG1_BAC_REG =
{
    "REGS_SBPM_UG1_BAC",
#if RU_INCLUDE_DESC
    "SBPM_UG1_BAC Register",
    "SBPM UG1 allocated BN Counter\n",
#endif
    { SBPM_REGS_SBPM_UG1_BAC_REG_OFFSET },
    0,
    0,
    1018,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_UG1_BAC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_GL_BAC, TYPE: Type_SBPM_SBPM_REGS_SBPM_GL_BAC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAC *****/
const ru_field_rec SBPM_REGS_SBPM_GL_BAC_BAC_FIELD =
{
    "BAC",
#if RU_INCLUDE_DESC
    "",
    "Global BN counter\n",
#endif
    { SBPM_REGS_SBPM_GL_BAC_BAC_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_GL_BAC_BAC_FIELD_WIDTH },
    { SBPM_REGS_SBPM_GL_BAC_BAC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_GL_BAC_FIELDS[] =
{
    &SBPM_REGS_SBPM_GL_BAC_BAC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_GL_BAC *****/
const ru_reg_rec SBPM_REGS_SBPM_GL_BAC_REG =
{
    "REGS_SBPM_GL_BAC",
#if RU_INCLUDE_DESC
    "SBPM_GL_BAC Register",
    "SBPM global BN Counter\n",
#endif
    { SBPM_REGS_SBPM_GL_BAC_REG_OFFSET },
    0,
    0,
    1019,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_GL_BAC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLT *****/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "",
    "exclusive high threshold\n",
#endif
    { SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD_SHIFT },
    960,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLH *****/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "",
    "exclusive histeresis threshold\n",
#endif
    { SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD,
    &SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH *****/
const ru_reg_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_REG =
{
    "REGS_SBPM_UG0_EXCL_HIGH_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG0_EXCLUSIVE_HIGH_THRESHOLD Register",
    "SBPM UG0 Exclusive high and hysteresis threshold.\nWe cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_REG_OFFSET },
    0,
    0,
    1020,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLT *****/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "",
    "exclusive high threshold\n",
#endif
    { SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD_SHIFT },
    960,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLH *****/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "",
    "exclusive histeresis threshold\n",
#endif
    { SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD,
    &SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH *****/
const ru_reg_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_REG =
{
    "REGS_SBPM_UG1_EXCL_HIGH_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG1_EXCLUSIVE_HIGH_THRESHOLD Register",
    "SBPM UG1 Exclusive high and hysteresis threshold.\nWe cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_REG_OFFSET },
    0,
    0,
    1021,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLT *****/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "",
    "exclusive low threshold\n",
#endif
    { SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD_SHIFT },
    896,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLH *****/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "",
    "exclusive histeresis threshold\n",
#endif
    { SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD,
    &SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH *****/
const ru_reg_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_REG =
{
    "REGS_SBPM_UG0_EXCL_LOW_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG0_EXCLUSIVE_LOW_THRESHOLD Register",
    "SBPM UG0 Exclusive low and hysteresis threshold.\nWe cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_REG_OFFSET },
    0,
    0,
    1022,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLT *****/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "",
    "exclusive low threshold\n",
#endif
    { SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD_SHIFT },
    896,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLH *****/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "",
    "exclusive histeresis threshold\n",
#endif
    { SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD,
    &SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH *****/
const ru_reg_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_REG =
{
    "REGS_SBPM_UG1_EXCL_LOW_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG1_EXCLUSIVE_LOW_THRESHOLD Register",
    "SBPM UG1 Exclusive low and hysteresis threshold.\nWe cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_REG_OFFSET },
    0,
    0,
    1023,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG_STATUS, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_ACK_STTS *****/
const ru_field_rec SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD =
{
    "UG_ACK_STTS",
#if RU_INCLUDE_DESC
    "",
    "Ack/Nack status per UG.\n0 - NACK\n1 - ACK\n\nbit [0] in field matches UG0 ACK status,\nbit [1] in field matches UG1 ACK status,\nbit [2] in field matches UG2 ACK status,\nbit [3] in field matches UG3 ACK status,\nbit [4] in field matches UG4 ACK status,\nbit [5] in field matches UG5 ACK status,\nbit [6] in field matches UG6 ACK status,\nbit [7] in field matches UG7 ACK status,\n",
#endif
    { SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_EXCL_HIGH_STTS *****/
const ru_field_rec SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD =
{
    "UG_EXCL_HIGH_STTS",
#if RU_INCLUDE_DESC
    "",
    "High EXCL/Non-Excl status per UG.\n0 - non_exclusive\n1 - exclusive\n\n",
#endif
    { SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_EXCL_LOW_STTS *****/
const ru_field_rec SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD =
{
    "UG_EXCL_LOW_STTS",
#if RU_INCLUDE_DESC
    "",
    "Low EXCL/Non-Excl status per UG.\n0 - non_exclusive\n1 - exclusive\n\n",
#endif
    { SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG_STATUS_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD,
    &SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD,
    &SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG_STATUS *****/
const ru_reg_rec SBPM_REGS_SBPM_UG_STATUS_REG =
{
    "REGS_SBPM_UG_STATUS",
#if RU_INCLUDE_DESC
    "USER_GROUP_STATUS_REGISTER Register",
    "This register is status set of all 8 Ugs: Ack/NACK state and in addition Exclusive state pereach of 8 UGs\n",
#endif
    { SBPM_REGS_SBPM_UG_STATUS_REG_OFFSET },
    0,
    0,
    1024,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_SBPM_UG_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_ERROR_HANDLING_PARAMS, TYPE: Type_SBPM_SBPM_REGS_ERROR_HANDLING_PARAMS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCH_DEPTH *****/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD =
{
    "SEARCH_DEPTH",
#if RU_INCLUDE_DESC
    "",
    "Depth (or maximal threshold) for search during Free without context\n",
#endif
    { SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD_MASK },
    0,
    { SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD_WIDTH },
    { SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_SEARCH_EN *****/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD =
{
    "MAX_SEARCH_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable for max search  during Free without context\n",
#endif
    { SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD_MASK },
    0,
    { SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD_WIDTH },
    { SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHCK_LAST_EN *****/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD =
{
    "CHCK_LAST_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable for Last BN checking  during Free with context\n",
#endif
    { SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD_MASK },
    0,
    { SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD_WIDTH },
    { SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREEZE_IN_ERROR *****/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD =
{
    "FREEZE_IN_ERROR",
#if RU_INCLUDE_DESC
    "",
    "Freeze Ug/Global counters + mask access to SBPM RAM while in ERROR state\n",
#endif
    { SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD_MASK },
    0,
    { SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD_WIDTH },
    { SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_ERROR_HANDLING_PARAMS_FIELDS[] =
{
    &SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_ERROR_HANDLING_PARAMS *****/
const ru_reg_rec SBPM_REGS_ERROR_HANDLING_PARAMS_REG =
{
    "REGS_ERROR_HANDLING_PARAMS",
#if RU_INCLUDE_DESC
    "ERROR_HANDLING_PARAMS Register",
    "Parameters and thresholds used for Error handling: error detection, max search enable and threshold, etc.\n",
#endif
    { SBPM_REGS_ERROR_HANDLING_PARAMS_REG_OFFSET },
    0,
    0,
    1025,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_ERROR_HANDLING_PARAMS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_IIR_ADDR, TYPE: Type_SBPM_SBPM_REGS_SBPM_IIR_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_SA *****/
const ru_field_rec SBPM_REGS_SBPM_IIR_ADDR_CMD_SA_FIELD =
{
    "CMD_SA",
#if RU_INCLUDE_DESC
    "",
    "Source addres of command that caused Interrupt\n",
#endif
    { SBPM_REGS_SBPM_IIR_ADDR_CMD_SA_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_IIR_ADDR_CMD_SA_FIELD_WIDTH },
    { SBPM_REGS_SBPM_IIR_ADDR_CMD_SA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_TA *****/
const ru_field_rec SBPM_REGS_SBPM_IIR_ADDR_CMD_TA_FIELD =
{
    "CMD_TA",
#if RU_INCLUDE_DESC
    "",
    "Target address of command that caused Interrupt\n",
#endif
    { SBPM_REGS_SBPM_IIR_ADDR_CMD_TA_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_IIR_ADDR_CMD_TA_FIELD_WIDTH },
    { SBPM_REGS_SBPM_IIR_ADDR_CMD_TA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_IIR_ADDR_FIELDS[] =
{
    &SBPM_REGS_SBPM_IIR_ADDR_CMD_SA_FIELD,
    &SBPM_REGS_SBPM_IIR_ADDR_CMD_TA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_IIR_ADDR *****/
const ru_reg_rec SBPM_REGS_SBPM_IIR_ADDR_REG =
{
    "REGS_SBPM_IIR_ADDR",
#if RU_INCLUDE_DESC
    "SBPM_IIR_ADDR_REGISTER Register",
    "SBPM SA & TA Address)\n",
#endif
    { SBPM_REGS_SBPM_IIR_ADDR_REG_OFFSET },
    0,
    0,
    1026,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_IIR_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_IIR_LOW, TYPE: Type_SBPM_SBPM_REGS_SBPM_IIR_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_DATA_0TO31 *****/
const ru_field_rec SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD =
{
    "CMD_DATA_0TO31",
#if RU_INCLUDE_DESC
    "",
    "Interrupt command data lowest 32-bit  (latched from BB data[31:0] or CPU request data)\n",
#endif
    { SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD_WIDTH },
    { SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_IIR_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_IIR_LOW *****/
const ru_reg_rec SBPM_REGS_SBPM_IIR_LOW_REG =
{
    "REGS_SBPM_IIR_LOW",
#if RU_INCLUDE_DESC
    "SBPM_IIR_LOW_REGISTER Register",
    "SBPM IIR low (Interrupt information register)\n",
#endif
    { SBPM_REGS_SBPM_IIR_LOW_REG_OFFSET },
    0,
    0,
    1027,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_IIR_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_IIR_HIGH, TYPE: Type_SBPM_SBPM_REGS_SBPM_IIR_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_DATA_32TO63 *****/
const ru_field_rec SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD =
{
    "CMD_DATA_32TO63",
#if RU_INCLUDE_DESC
    "",
    "Data (bits [63:32], with reserved bits) of the command that caused interrupt\n",
#endif
    { SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD_WIDTH },
    { SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_IIR_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_IIR_HIGH *****/
const ru_reg_rec SBPM_REGS_SBPM_IIR_HIGH_REG =
{
    "REGS_SBPM_IIR_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_IIR_HIGH_REGISTER Register",
    "SBPM IIR high (Interrupt information register)\n",
#endif
    { SBPM_REGS_SBPM_IIR_HIGH_REG_OFFSET },
    0,
    0,
    1028,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_IIR_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_DBG_VEC0, TYPE: Type_SBPM_SBPM_REGS_SBPM_DBG_VEC0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD =
{
    "ALLOC_SM",
#if RU_INCLUDE_DESC
    "",
    "Alloc State Machine\n{update, rd_head_cnxt}\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNNCT_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD =
{
    "CNNCT_SM",
#if RU_INCLUDE_DESC
    "",
    "Connect State Machine\n{update}\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MCINT_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD =
{
    "MCINT_SM",
#if RU_INCLUDE_DESC
    "",
    "Multicast incr State Machine\n{read,check,error,update}\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_W_CNXT_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD =
{
    "FREE_W_CNXT_SM",
#if RU_INCLUDE_DESC
    "",
    "Free w cnxt State Machine\n{read,check,update,error}\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_WO_CNXT_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD =
{
    "FREE_WO_CNXT_SM",
#if RU_INCLUDE_DESC
    "",
    "Free w/o cnxt State Machine\n{read,check,update,error}\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GN_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD =
{
    "GN_SM",
#if RU_INCLUDE_DESC
    "",
    "Get next State Machine:\n{read,reply}\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_GN_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD =
{
    "MULTI_GN_SM",
#if RU_INCLUDE_DESC
    "",
    "Those are the 4 Multi get next states:\n{rd_next,error,rd_last,wait}\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_LST_HD *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD =
{
    "FREE_LST_HD",
#if RU_INCLUDE_DESC
    "",
    "the value of the head of FREE list\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_DBG_VEC0_FIELDS[] =
{
    &SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_DBG_VEC0 *****/
const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC0_REG =
{
    "REGS_SBPM_DBG_VEC0",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC0 Register",
    "SBPM debug vector0 includes 21 bit of control/state machine of CMD pipe\n\n\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC0_REG_OFFSET },
    0,
    0,
    1029,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    SBPM_REGS_SBPM_DBG_VEC0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_DBG_VEC1, TYPE: Type_SBPM_SBPM_REGS_SBPM_DBG_VEC1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IN2E_VALID *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD =
{
    "IN2E_VALID",
#if RU_INCLUDE_DESC
    "",
    "sbpm_ingress2egress_valid bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_GN_VALID *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD =
{
    "MULTI_GN_VALID",
#if RU_INCLUDE_DESC
    "",
    "multi_get_next_valid bits\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_ACTIVE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD =
{
    "UG_ACTIVE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_ug_active 2 bits\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_CMD_FULL *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD =
{
    "TX_CMD_FULL",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_cmd_fifo_full bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_FIFO_POP *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD =
{
    "RX_FIFO_POP",
#if RU_INCLUDE_DESC
    "",
    "sbpm_rx_fifo_pop bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_INIT_START *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD =
{
    "RAM_INIT_START",
#if RU_INCLUDE_DESC
    "",
    "sbpm_ram_init_start bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_INIT_DONE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD =
{
    "RAM_INIT_DONE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_ram_init_done bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_FIFO_DATA *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD =
{
    "RX_FIFO_DATA",
#if RU_INCLUDE_DESC
    "",
    "RX FIFO Data in pipe\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD =
{
    "FREE_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_free_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IN2E_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD =
{
    "IN2E_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_in2e_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_WO_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD =
{
    "FREE_WO_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_free_wo_cnxt_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GET_NXT_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD =
{
    "GET_NXT_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_get_next_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_GET_NXT_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD =
{
    "MULTI_GET_NXT_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_multi_get_next_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNCT_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD =
{
    "CNCT_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_cnct_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_W_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD =
{
    "FREE_W_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_free_w_cnxt_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MCIN_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD =
{
    "MCIN_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_mcinc_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_DECODE *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD =
{
    "ALLOC_DECODE",
#if RU_INCLUDE_DESC
    "",
    "sbpm_alloc_rqst_dec\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_DBG_VEC1_FIELDS[] =
{
    &SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_DBG_VEC1 *****/
const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC1_REG =
{
    "REGS_SBPM_DBG_VEC1",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC1 Register",
    "SBPM debug vector1 includes 21 bit of control/state machine of CMD pipe\n\n\n\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC1_REG_OFFSET },
    0,
    0,
    1030,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    17,
    SBPM_REGS_SBPM_DBG_VEC1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_DBG_VEC2, TYPE: Type_SBPM_SBPM_REGS_SBPM_DBG_VEC2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_DATA_FULL *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD =
{
    "TX_DATA_FULL",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_data_fifo_full\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_FIFO_EMPTY *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD =
{
    "TX_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_fifo_empty\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LCL_STTS_FULL *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD =
{
    "LCL_STTS_FULL",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_cmd_local_stts_fifo_full\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LCL_STTS_EMPTY *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD =
{
    "LCL_STTS_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_cmd_local_stts_fifo_empty\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_CMD_FULL *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD =
{
    "TX_CMD_FULL",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_cmd_fifo_full\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_CMD_FIFO_EMPTY *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD =
{
    "TX_CMD_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_cmd_fifo_empty\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BB_DECODER_DEST_ID *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD =
{
    "BB_DECODER_DEST_ID",
#if RU_INCLUDE_DESC
    "",
    "bb_decoder_dest_id\nThis is the ID of the user that will recieve a message from SBPM\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_BBH_SEND_IN_PROGRESS *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD =
{
    "TX_BBH_SEND_IN_PROGRESS",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_bbh_send_in_progress bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SP_2SEND *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD =
{
    "SP_2SEND",
#if RU_INCLUDE_DESC
    "",
    "sbpm_sp_2send - this is the user ID that is about to get stts msg\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX2DATA_FIFO_TADDR *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD =
{
    "TX2DATA_FIFO_TADDR",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx2data_fifo_taddr[2:0] this is the opcode that describe the type of the reply\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CPU_ACCESS *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD =
{
    "CPU_ACCESS",
#if RU_INCLUDE_DESC
    "",
    "sbpm_cpu_access bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBH_ACCESS *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD =
{
    "BBH_ACCESS",
#if RU_INCLUDE_DESC
    "",
    "sbpm_bbh_access bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RNR_ACCESS *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD =
{
    "RNR_ACCESS",
#if RU_INCLUDE_DESC
    "",
    "sbpm_rnr_access bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_DBG_VEC2_FIELDS[] =
{
    &SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_DBG_VEC2 *****/
const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC2_REG =
{
    "REGS_SBPM_DBG_VEC2",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC2 Register",
    "This is one of the TX_handler debug vectors\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC2_REG_OFFSET },
    0,
    0,
    1031,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    13,
    SBPM_REGS_SBPM_DBG_VEC2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_DBG_VEC3, TYPE: Type_SBPM_SBPM_REGS_SBPM_DBG_VEC3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_RPLY *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD =
{
    "ALLOC_RPLY",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_RPLY bit\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BN_RPLY *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD =
{
    "BN_RPLY",
#if RU_INCLUDE_DESC
    "",
    "BN_RPLY value\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_ALLOC_ACK *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD =
{
    "TXFIFO_ALLOC_ACK",
#if RU_INCLUDE_DESC
    "",
    "sbpm_txfifo_alloc_ack\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_FIFO_MCINC_ACK *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD =
{
    "TX_FIFO_MCINC_ACK",
#if RU_INCLUDE_DESC
    "",
    "sbpm_txfifo_mcinc_ack\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_CNCT_ACK *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD =
{
    "TXFIFO_CNCT_ACK",
#if RU_INCLUDE_DESC
    "",
    "sbpm_txfifo_cnct_ack\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_GT_NXT_RPLY *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD =
{
    "TXFIFO_GT_NXT_RPLY",
#if RU_INCLUDE_DESC
    "",
    "sbpm_txfifo_get_next_reply\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_MLTI_GT_NXT_RPLY *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD =
{
    "TXFIFO_MLTI_GT_NXT_RPLY",
#if RU_INCLUDE_DESC
    "",
    "sbpm_txfifo_multi_get_next_reply\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_MSG_PIPE_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD =
{
    "TX_MSG_PIPE_SM",
#if RU_INCLUDE_DESC
    "",
    "sbpm_tx_msg_pipe_cur_sm\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEND_STT_SM *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD =
{
    "SEND_STT_SM",
#if RU_INCLUDE_DESC
    "",
    "sbpm_send_stat_sm_ps\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_IN2ESTTS_CHNG *****/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD =
{
    "TXFIFO_IN2ESTTS_CHNG",
#if RU_INCLUDE_DESC
    "",
    "sbpm_txfifo_ingress2egress_stts_change\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD_WIDTH },
    { SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_DBG_VEC3_FIELDS[] =
{
    &SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD,
    &SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_DBG_VEC3 *****/
const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC3_REG =
{
    "REGS_SBPM_DBG_VEC3",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC3 Register",
    "This is one of TX_handler debug vectors\n",
#endif
    { SBPM_REGS_SBPM_DBG_VEC3_REG_OFFSET },
    0,
    0,
    1032,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    SBPM_REGS_SBPM_DBG_VEC3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_SP_BBH_LOW, TYPE: Type_SBPM_SBPM_REGS_SBPM_SP_BBH_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_SP_BBH_LOW *****/
const ru_field_rec SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD =
{
    "SBPM_SP_BBH_LOW",
#if RU_INCLUDE_DESC
    "",
    "sbpm_sp_bbh_low bit i tells us if SP #i is a BBH (1) or not (0)\n",
#endif
    { SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD_WIDTH },
    { SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD_SHIFT },
    2885615616,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SP_BBH_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_SP_BBH_LOW *****/
const ru_reg_rec SBPM_REGS_SBPM_SP_BBH_LOW_REG =
{
    "REGS_SBPM_SP_BBH_LOW",
#if RU_INCLUDE_DESC
    "SBPM_SP_BBH_LOW Register",
    "This register mark all the SPs which are BBHs.\nEach bit in this register, refers to a SP with the same index\n",
#endif
    { SBPM_REGS_SBPM_SP_BBH_LOW_REG_OFFSET },
    0,
    0,
    1033,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SP_BBH_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_SP_BBH_HIGH, TYPE: Type_SBPM_SBPM_REGS_SBPM_SP_BBH_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_SP_BBH_HIGH *****/
const ru_field_rec SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD =
{
    "SBPM_SP_BBH_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Not in use in 68360!\nsbpm_sp_bbh_high bit i tells us if SP #i is a BBH (1) or not (0)\n",
#endif
    { SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD_SHIFT },
    15379114,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SP_BBH_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_SP_BBH_HIGH *****/
const ru_reg_rec SBPM_REGS_SBPM_SP_BBH_HIGH_REG =
{
    "REGS_SBPM_SP_BBH_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_SP_BBH_HIGH Register",
    "This register mark all the SPs which are BBHs.\nEach bit in this register, refers to a SP with the same index\n",
#endif
    { SBPM_REGS_SBPM_SP_BBH_HIGH_REG_OFFSET },
    0,
    0,
    1034,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SP_BBH_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_SP_RNR_LOW, TYPE: Type_SBPM_SBPM_REGS_SBPM_SP_RNR_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_SP_RNR_LOW *****/
const ru_field_rec SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD =
{
    "SBPM_SP_RNR_LOW",
#if RU_INCLUDE_DESC
    "",
    "sbpm_sp_rnr_low bit i tells us if SP #i is a runner (1) or not (0)\n",
#endif
    { SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD_WIDTH },
    { SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SP_RNR_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_SP_RNR_LOW *****/
const ru_reg_rec SBPM_REGS_SBPM_SP_RNR_LOW_REG =
{
    "REGS_SBPM_SP_RNR_LOW",
#if RU_INCLUDE_DESC
    "SBPM_SP_RNR_LOW Register",
    "This register mark all the SPs which are runners.\nEach bit in this register, refers to a SP with the same index\n",
#endif
    { SBPM_REGS_SBPM_SP_RNR_LOW_REG_OFFSET },
    0,
    0,
    1035,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SP_RNR_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_SP_RNR_HIGH, TYPE: Type_SBPM_SBPM_REGS_SBPM_SP_RNR_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_SP_RNR_HIGH *****/
const ru_field_rec SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD =
{
    "SBPM_SP_RNR_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Not in use in 68360!\nsbpm_sp_rnr_high bit i tells us if SP #i is a runner (1) or not (0)\n",
#endif
    { SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SP_RNR_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_SP_RNR_HIGH *****/
const ru_reg_rec SBPM_REGS_SBPM_SP_RNR_HIGH_REG =
{
    "REGS_SBPM_SP_RNR_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_SP_RNR_HIGH Register",
    "This register mark all the SPs which are runners.\nEach bit in this register, refers to a SP with the same index\n",
#endif
    { SBPM_REGS_SBPM_SP_RNR_HIGH_REG_OFFSET },
    0,
    0,
    1036,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SP_RNR_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG_MAP_LOW, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG_MAP_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_UG_MAP_LOW *****/
const ru_field_rec SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD =
{
    "SBPM_UG_MAP_LOW",
#if RU_INCLUDE_DESC
    "",
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)\n",
#endif
    { SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD_SHIFT },
    22347776,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG_MAP_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG_MAP_LOW *****/
const ru_reg_rec SBPM_REGS_SBPM_UG_MAP_LOW_REG =
{
    "REGS_SBPM_UG_MAP_LOW",
#if RU_INCLUDE_DESC
    "SBPM_UG_MAP_LOW Register",
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)\n",
#endif
    { SBPM_REGS_SBPM_UG_MAP_LOW_REG_OFFSET },
    0,
    0,
    1037,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_UG_MAP_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG_MAP_HIGH, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG_MAP_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_UG_MAP_HIGH *****/
const ru_field_rec SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD =
{
    "SBPM_UG_MAP_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Not in use in 68360!\nbit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)\n",
#endif
    { SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD_SHIFT },
    2160088064,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG_MAP_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG_MAP_HIGH *****/
const ru_reg_rec SBPM_REGS_SBPM_UG_MAP_HIGH_REG =
{
    "REGS_SBPM_UG_MAP_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_UG_MAP_HIGH Register",
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)\n",
#endif
    { SBPM_REGS_SBPM_UG_MAP_HIGH_REG_OFFSET },
    0,
    0,
    1038,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_UG_MAP_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_NACK_MASK_LOW, TYPE: Type_SBPM_SBPM_REGS_SBPM_NACK_MASK_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_NACK_MASK_LOW *****/
const ru_field_rec SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD =
{
    "SBPM_NACK_MASK_LOW",
#if RU_INCLUDE_DESC
    "",
    "bit i value determine if SP number i got nack or not\n",
#endif
    { SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD_WIDTH },
    { SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_NACK_MASK_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_NACK_MASK_LOW *****/
const ru_reg_rec SBPM_REGS_SBPM_NACK_MASK_LOW_REG =
{
    "REGS_SBPM_NACK_MASK_LOW",
#if RU_INCLUDE_DESC
    "SBPM_NACK_MASK_LOW Register",
    "bit i value determine if SP number i got nack or not\n",
#endif
    { SBPM_REGS_SBPM_NACK_MASK_LOW_REG_OFFSET },
    0,
    0,
    1039,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_NACK_MASK_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_NACK_MASK_HIGH, TYPE: Type_SBPM_SBPM_REGS_SBPM_NACK_MASK_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_NACK_MASK_HIGH *****/
const ru_field_rec SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD =
{
    "SBPM_NACK_MASK_HIGH",
#if RU_INCLUDE_DESC
    "",
    "bit i value determine if SP number i got nack or not\n",
#endif
    { SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_NACK_MASK_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_NACK_MASK_HIGH *****/
const ru_reg_rec SBPM_REGS_SBPM_NACK_MASK_HIGH_REG =
{
    "REGS_SBPM_NACK_MASK_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_NACK_MASK_HIGH Register",
    "bit i value determine if SP number i got nack or not\n",
#endif
    { SBPM_REGS_SBPM_NACK_MASK_HIGH_REG_OFFSET },
    0,
    0,
    1040,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_NACK_MASK_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_EXCL_MASK_LOW, TYPE: Type_SBPM_SBPM_REGS_SBPM_EXCL_MASK_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_EXCL_MASK_LOW *****/
const ru_field_rec SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD =
{
    "SBPM_EXCL_MASK_LOW",
#if RU_INCLUDE_DESC
    "",
    "This register mark all the SPs that should get exclusive messages\nyes no\n",
#endif
    { SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD_WIDTH },
    { SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD_SHIFT },
    67108864,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_EXCL_MASK_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_EXCL_MASK_LOW *****/
const ru_reg_rec SBPM_REGS_SBPM_EXCL_MASK_LOW_REG =
{
    "REGS_SBPM_EXCL_MASK_LOW",
#if RU_INCLUDE_DESC
    "SBPM_EXCL_MASK_LOW Register",
    "This register mark all the SPs that should get exclusive messages\n",
#endif
    { SBPM_REGS_SBPM_EXCL_MASK_LOW_REG_OFFSET },
    0,
    0,
    1041,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_EXCL_MASK_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_EXCL_MASK_HIGH, TYPE: Type_SBPM_SBPM_REGS_SBPM_EXCL_MASK_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_EXCL_MASK_HIGH *****/
const ru_field_rec SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD =
{
    "SBPM_EXCL_MASK_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Not in use in 68360!\nThis register mark all the SPs that should get exclusive messages\nyes no\n",
#endif
    { SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD_WIDTH },
    { SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD_SHIFT },
    4096,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_EXCL_MASK_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_EXCL_MASK_HIGH *****/
const ru_reg_rec SBPM_REGS_SBPM_EXCL_MASK_HIGH_REG =
{
    "REGS_SBPM_EXCL_MASK_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_EXCL_MASK_HIGH Register",
    "This register mark all the SPs that should get exclusive messages\n",
#endif
    { SBPM_REGS_SBPM_EXCL_MASK_HIGH_REG_OFFSET },
    0,
    0,
    1042,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_EXCL_MASK_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_RADDR_DECODER, TYPE: Type_SBPM_SBPM_REGS_SBPM_RADDR_DECODER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ID_2OVERWR *****/
const ru_field_rec SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD =
{
    "ID_2OVERWR",
#if RU_INCLUDE_DESC
    "",
    "this field contains the users id that you want to override its default RA\n",
#endif
    { SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD_WIDTH },
    { SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERWR_RA *****/
const ru_field_rec SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD =
{
    "OVERWR_RA",
#if RU_INCLUDE_DESC
    "",
    "The new RA\n",
#endif
    { SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD_WIDTH },
    { SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERWR_VALID *****/
const ru_field_rec SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD =
{
    "OVERWR_VALID",
#if RU_INCLUDE_DESC
    "",
    "the overwr mechanism will be used only if this bit is active (1).\n",
#endif
    { SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD_WIDTH },
    { SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_RADDR_DECODER_FIELDS[] =
{
    &SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD,
    &SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD,
    &SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_RADDR_DECODER *****/
const ru_reg_rec SBPM_REGS_SBPM_RADDR_DECODER_REG =
{
    "REGS_SBPM_RADDR_DECODER",
#if RU_INCLUDE_DESC
    "SBPM_RADDR_DECODER Register",
    "This register let you choose one user that you would like to change its default RA.\n",
#endif
    { SBPM_REGS_SBPM_RADDR_DECODER_REG_OFFSET },
    0,
    0,
    1043,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_SBPM_RADDR_DECODER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_WR_DATA, TYPE: Type_SBPM_SBPM_REGS_SBPM_WR_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_WR_DATA *****/
const ru_field_rec SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD =
{
    "SBPM_WR_DATA",
#if RU_INCLUDE_DESC
    "",
    "If SW want to write a whole word into the SBPMs RAM, it needs first to write the data to this register and then, send connect request with the wr_req bit asserted, with the address (BN field).\n\nIn 68360 the only the 15 LSB are used\n",
#endif
    { SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD_WIDTH },
    { SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_WR_DATA_FIELDS[] =
{
    &SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_WR_DATA *****/
const ru_reg_rec SBPM_REGS_SBPM_WR_DATA_REG =
{
    "REGS_SBPM_WR_DATA",
#if RU_INCLUDE_DESC
    "SBPM_WR_DATA Register",
    "If SW want to write a whole word into the SBPMs RAM, it needs first to write the data to this register and then, send connect request with the wr_req bit asserted, with the address (BN field).\n",
#endif
    { SBPM_REGS_SBPM_WR_DATA_REG_OFFSET },
    0,
    0,
    1044,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_WR_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_UG_BAC_MAX, TYPE: Type_SBPM_SBPM_REGS_SBPM_UG_BAC_MAX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UG0BACMAX *****/
const ru_field_rec SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD =
{
    "UG0BACMAX",
#if RU_INCLUDE_DESC
    "",
    "This is the maximum value that have been recorded on the UG0 counter.\nSW can write to this field in order to change the max record (for example write 0 to reset it)\n",
#endif
    { SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UG1BACMAX *****/
const ru_field_rec SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD =
{
    "UG1BACMAX",
#if RU_INCLUDE_DESC
    "",
    "This is the maximum value that have been recorded on the UG1 counter.\nSW can write to this field in order to change the max record (for example write 0 to reset it)\n",
#endif
    { SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD_WIDTH },
    { SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG_BAC_MAX_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD,
    &SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_UG_BAC_MAX *****/
const ru_reg_rec SBPM_REGS_SBPM_UG_BAC_MAX_REG =
{
    "REGS_SBPM_UG_BAC_MAX",
#if RU_INCLUDE_DESC
    "SBPM_UG_BAC_MAX Register",
    "This register tracks the max values of the UG counters. it can be reset/modified by SW.\n",
#endif
    { SBPM_REGS_SBPM_UG_BAC_MAX_REG_OFFSET },
    0,
    0,
    1045,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG_BAC_MAX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_SPARE, TYPE: Type_SBPM_SBPM_REGS_SBPM_SPARE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GL_BAC_CLEAR_EN *****/
const ru_field_rec SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD =
{
    "GL_BAC_CLEAR_EN",
#if RU_INCLUDE_DESC
    "",
    "sbpm_gl_bac_clear_en\n",
#endif
    { SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD_WIDTH },
    { SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SPARE_FIELDS[] =
{
    &SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_SPARE *****/
const ru_reg_rec SBPM_REGS_SBPM_SPARE_REG =
{
    "REGS_SBPM_SPARE",
#if RU_INCLUDE_DESC
    "SBPM_SPARE Register",
    "sbpm spare register\n",
#endif
    { SBPM_REGS_SBPM_SPARE_REG_OFFSET },
    0,
    0,
    1046,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SPARE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_REGS_SBPM_BAC_UNDERRUN, TYPE: Type_SBPM_SBPM_REGS_SBPM_BAC_UNDERRUN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_CFG_BAC_UNDERRUN_EN *****/
const ru_field_rec SBPM_REGS_SBPM_BAC_UNDERRUN_SBPM_CFG_BAC_UNDERRUN_EN_FIELD =
{
    "SBPM_CFG_BAC_UNDERRUN_EN",
#if RU_INCLUDE_DESC
    "",
    "When=1, enable UG1 BAC Underrun, to overcome scenario of Free is send after in2eg, but free is received at SBPM before In2Eg due to In2Eg Broad-bus leaf stuck\n",
#endif
    { SBPM_REGS_SBPM_BAC_UNDERRUN_SBPM_CFG_BAC_UNDERRUN_EN_FIELD_MASK },
    0,
    { SBPM_REGS_SBPM_BAC_UNDERRUN_SBPM_CFG_BAC_UNDERRUN_EN_FIELD_WIDTH },
    { SBPM_REGS_SBPM_BAC_UNDERRUN_SBPM_CFG_BAC_UNDERRUN_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_BAC_UNDERRUN_FIELDS[] =
{
    &SBPM_REGS_SBPM_BAC_UNDERRUN_SBPM_CFG_BAC_UNDERRUN_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_REGS_SBPM_BAC_UNDERRUN *****/
const ru_reg_rec SBPM_REGS_SBPM_BAC_UNDERRUN_REG =
{
    "REGS_SBPM_BAC_UNDERRUN",
#if RU_INCLUDE_DESC
    "SBPM_BAC_UNDERRUN Register",
    "BAC Underrun cfg\n",
#endif
    { SBPM_REGS_SBPM_BAC_UNDERRUN_REG_OFFSET },
    0,
    0,
    1047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_BAC_UNDERRUN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_INTR_CTRL_ISR, TYPE: Type_SBPM_SBPM_INTR_CTRL_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BAC_UNDERRUN *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD =
{
    "BAC_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "This error bit indicates underrun state of SBPM Buffer Allocated Counter (one of User Groups) due to free operation (w/wo context). Note that ug0 bac is NOT updated/affected, whereas UG1 may be updated, according to config bit bac_underrun_en. SW can clear this bit by writing 1 to this field\n",
#endif
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MCST_OVERFLOW *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD =
{
    "MCST_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "This error bit indicates if the Multi Cast value of a buffer is in overflow as a result of erroneous MCINC command\n",
#endif
    { SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHECK_LAST_ERR *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD =
{
    "CHECK_LAST_ERR",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates error state on Last BN checking during Free with context request. SW can clear this bit by writing 1 to this field.\n",
#endif
    { SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_SEARCH_ERR *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD =
{
    "MAX_SEARCH_ERR",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates error state on maximal search checking during Free without context request. SW can clear this bit by writing 1 to this field.\n",
#endif
    { SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_IN2E *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD =
{
    "INVALID_IN2E",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates invalid ingress2egress command (caused BAC under/overrun). SW can clear this bit by writing 1 to this field.\n",
#endif
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_GET_NEXT_NULL *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD =
{
    "MULTI_GET_NEXT_NULL",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates Null encounter during one of the next BNs. SW can clear this bit by writing 0 to this field.\n",
#endif
    { SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNCT_NULL *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD =
{
    "CNCT_NULL",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates connection of the NULL buffer to another buufer. SW can clear this bit by writing 0 to this field.\n",
#endif
    { SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_NULL *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD =
{
    "ALLOC_NULL",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates allocation of the NULL buffer. SW can clear this bit by writing 0 to this field.\n",
#endif
    { SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_IN2E_OVERFLOW *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_INVALID_IN2E_OVERFLOW_FIELD =
{
    "INVALID_IN2E_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates invalid ingress2egress overflow command (caused BAC over-run). SW can clear this bit by writing 1 to this field.\n",
#endif
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_OVERFLOW_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_OVERFLOW_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_OVERFLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_IN2E_UNDERFLOW *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_INVALID_IN2E_UNDERFLOW_FIELD =
{
    "INVALID_IN2E_UNDERFLOW",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates invalid ingress2egress underflow command (caused BAC under-run). SW can clear this bit by writing 1 to this field.\n",
#endif
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_UNDERFLOW_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_UNDERFLOW_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_INVALID_IN2E_UNDERFLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAC_UNDERRUN_UG0 *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG0_FIELD =
{
    "BAC_UNDERRUN_UG0",
#if RU_INCLUDE_DESC
    "",
    "This error bit indicates underrun state of SBPM Buffer Allocated Counter UG0 due to free operation (w/wo context). Note that ug0 bac is NOT updated/affected. SW can clear this bit by writing 1 to this field\n",
#endif
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG0_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG0_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAC_UNDERRUN_UG1 *****/
const ru_field_rec SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG1_FIELD =
{
    "BAC_UNDERRUN_UG1",
#if RU_INCLUDE_DESC
    "",
    "This error bit indicates underrun state of SBPM Buffer Allocated Counter UG1 due to free operation (w/wo context). Note that ug1 bac May be updated/affected, depending on bac_underrun_en config bit. SW can clear this bit by writing 1 to this field\n",
#endif
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG1_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG1_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_INTR_CTRL_ISR_FIELDS[] =
{
    &SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD,
    &SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD,
    &SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD,
    &SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD,
    &SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD,
    &SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD,
    &SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD,
    &SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD,
    &SBPM_INTR_CTRL_ISR_INVALID_IN2E_OVERFLOW_FIELD,
    &SBPM_INTR_CTRL_ISR_INVALID_IN2E_UNDERFLOW_FIELD,
    &SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG0_FIELD,
    &SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_UG1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_INTR_CTRL_ISR *****/
const ru_reg_rec SBPM_INTR_CTRL_ISR_REG =
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.\n",
#endif
    { SBPM_INTR_CTRL_ISR_REG_OFFSET },
    0,
    0,
    1048,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    SBPM_INTR_CTRL_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_INTR_CTRL_ISM, TYPE: Type_SBPM_SBPM_INTR_CTRL_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ISM *****/
const ru_field_rec SBPM_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "",
    "Status Masked of corresponding interrupt source in the ISR\n",
#endif
    { SBPM_INTR_CTRL_ISM_ISM_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ISM_ISM_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ISM_ISM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_INTR_CTRL_ISM_FIELDS[] =
{
    &SBPM_INTR_CTRL_ISM_ISM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_INTR_CTRL_ISM *****/
const ru_reg_rec SBPM_INTR_CTRL_ISM_REG =
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { SBPM_INTR_CTRL_ISM_REG_OFFSET },
    0,
    0,
    1049,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_INTR_CTRL_ISM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_INTR_CTRL_IER, TYPE: Type_SBPM_SBPM_INTR_CTRL_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IEM *****/
const ru_field_rec SBPM_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask controls the corresponding interrupt source in the IER\n",
#endif
    { SBPM_INTR_CTRL_IER_IEM_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_IER_IEM_FIELD_WIDTH },
    { SBPM_INTR_CTRL_IER_IEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_INTR_CTRL_IER_FIELDS[] =
{
    &SBPM_INTR_CTRL_IER_IEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_INTR_CTRL_IER *****/
const ru_reg_rec SBPM_INTR_CTRL_IER_REG =
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { SBPM_INTR_CTRL_IER_REG_OFFSET },
    0,
    0,
    1050,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_INTR_CTRL_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: SBPM_INTR_CTRL_ITR, TYPE: Type_SBPM_SBPM_INTR_CTRL_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec SBPM_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask tests the corresponding interrupt source in the ISR\n",
#endif
    { SBPM_INTR_CTRL_ITR_IST_FIELD_MASK },
    0,
    { SBPM_INTR_CTRL_ITR_IST_FIELD_WIDTH },
    { SBPM_INTR_CTRL_ITR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_INTR_CTRL_ITR_FIELDS[] =
{
    &SBPM_INTR_CTRL_ITR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: SBPM_INTR_CTRL_ITR *****/
const ru_reg_rec SBPM_INTR_CTRL_ITR_REG =
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR\n",
#endif
    { SBPM_INTR_CTRL_ITR_REG_OFFSET },
    0,
    0,
    1051,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_INTR_CTRL_ITR_FIELDS,
#endif
};

unsigned long SBPM_ADDRS[] =
{
    0x828A1000,
};

static const ru_reg_rec *SBPM_REGS[] =
{
    &SBPM_REGS_INIT_FREE_LIST_REG,
    &SBPM_REGS_BN_ALLOC_REG,
    &SBPM_REGS_BN_ALLOC_RPLY_REG,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_REG,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_REG,
    &SBPM_REGS_MCST_INC_REG,
    &SBPM_REGS_MCST_INC_RPLY_REG,
    &SBPM_REGS_BN_CONNECT_REG,
    &SBPM_REGS_BN_CONNECT_RPLY_REG,
    &SBPM_REGS_GET_NEXT_REG,
    &SBPM_REGS_GET_NEXT_RPLY_REG,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_REG,
    &SBPM_REGS_SBPM_GL_TRSH_REG,
    &SBPM_REGS_SBPM_UG0_TRSH_REG,
    &SBPM_REGS_SBPM_UG1_TRSH_REG,
    &SBPM_REGS_SBPM_DBG_REG,
    &SBPM_REGS_SBPM_UG0_BAC_REG,
    &SBPM_REGS_SBPM_UG1_BAC_REG,
    &SBPM_REGS_SBPM_GL_BAC_REG,
    &SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_REG,
    &SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_REG,
    &SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_REG,
    &SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_REG,
    &SBPM_REGS_SBPM_UG_STATUS_REG,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_REG,
    &SBPM_REGS_SBPM_IIR_ADDR_REG,
    &SBPM_REGS_SBPM_IIR_LOW_REG,
    &SBPM_REGS_SBPM_IIR_HIGH_REG,
    &SBPM_REGS_SBPM_DBG_VEC0_REG,
    &SBPM_REGS_SBPM_DBG_VEC1_REG,
    &SBPM_REGS_SBPM_DBG_VEC2_REG,
    &SBPM_REGS_SBPM_DBG_VEC3_REG,
    &SBPM_REGS_SBPM_SP_BBH_LOW_REG,
    &SBPM_REGS_SBPM_SP_BBH_HIGH_REG,
    &SBPM_REGS_SBPM_SP_RNR_LOW_REG,
    &SBPM_REGS_SBPM_SP_RNR_HIGH_REG,
    &SBPM_REGS_SBPM_UG_MAP_LOW_REG,
    &SBPM_REGS_SBPM_UG_MAP_HIGH_REG,
    &SBPM_REGS_SBPM_NACK_MASK_LOW_REG,
    &SBPM_REGS_SBPM_NACK_MASK_HIGH_REG,
    &SBPM_REGS_SBPM_EXCL_MASK_LOW_REG,
    &SBPM_REGS_SBPM_EXCL_MASK_HIGH_REG,
    &SBPM_REGS_SBPM_RADDR_DECODER_REG,
    &SBPM_REGS_SBPM_WR_DATA_REG,
    &SBPM_REGS_SBPM_UG_BAC_MAX_REG,
    &SBPM_REGS_SBPM_SPARE_REG,
    &SBPM_REGS_SBPM_BAC_UNDERRUN_REG,
    &SBPM_INTR_CTRL_ISR_REG,
    &SBPM_INTR_CTRL_ISM_REG,
    &SBPM_INTR_CTRL_IER_REG,
    &SBPM_INTR_CTRL_ITR_REG,
};

const ru_block_rec SBPM_BLOCK =
{
    "SBPM",
    SBPM_ADDRS,
    1,
    54,
    SBPM_REGS,
};
