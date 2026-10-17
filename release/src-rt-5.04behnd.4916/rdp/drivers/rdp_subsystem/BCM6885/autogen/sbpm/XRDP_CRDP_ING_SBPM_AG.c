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


#include "XRDP_CRDP_ING_SBPM_AG.h"

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_INIT_FREE_LIST, TYPE: Type_SBPM_SBPM_REGS_INIT_FREE_LIST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_MAX_BN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_INIT_FREE_LIST_INIT_MAX_BN_FIELD =
{
    "INIT_MAX_BN",
#if RU_INCLUDE_DESC
    "",
    "Maximum BN value.\n",
#endif
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_INIT_MAX_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_INIT_MAX_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_INIT_MAX_BN_FIELD_SHIFT },
    2047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_INIT_START *****/
const ru_field_rec CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RAM_INIT_START_FIELD =
{
    "RAM_INIT_START",
#if RU_INCLUDE_DESC
    "",
    "Start SBPM initialization for free BNs memory and MC memory (if it has MC).\n\n",
#endif
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RAM_INIT_START_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RAM_INIT_START_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RAM_INIT_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BSY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_INIT_FREE_LIST_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "",
    "The bit is used  as busy  indication of buffer allocation request status (busy status)  by CPU.\nBPM asserts this bit on each valid request and de-asserts when request is treated.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "The bit is used  as ready indication of buffer allocation request status (ready status)  by CPU.\nBPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is READY indication\nTRIGGERS_HW\n",
#endif
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_INIT_FREE_LIST_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_INIT_FREE_LIST_INIT_MAX_BN_FIELD,
    &CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RAM_INIT_START_FIELD,
    &CRDP_ING_SBPM_REGS_INIT_FREE_LIST_BSY_FIELD,
    &CRDP_ING_SBPM_REGS_INIT_FREE_LIST_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_INIT_FREE_LIST *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_INIT_FREE_LIST_REG =
{
    "REGS_INIT_FREE_LIST",
#if RU_INCLUDE_DESC
    "INIT_FREE_LIST Register",
    "request for building the free list using HW accelerator\n",
#endif
    { CRDP_ING_SBPM_REGS_INIT_FREE_LIST_REG_OFFSET },
    0,
    0,
    1019,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_ING_SBPM_REGS_INIT_FREE_LIST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_BN_ALLOC, TYPE: Type_SBPM_SBPM_REGS_BN_ALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_REQ_FIELD =
{
    "REQ",
#if RU_INCLUDE_DESC
    "",
    "rsv\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_REQ_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_REQ_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_BN_ALLOC_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_BN_ALLOC_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_BN_ALLOC *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_BN_ALLOC_REG =
{
    "REGS_BN_ALLOC",
#if RU_INCLUDE_DESC
    "BN_ALLOC Register",
    "request for a new buffer\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_REG_OFFSET },
    0,
    0,
    1020,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_BN_ALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY, TYPE: Type_SBPM_SBPM_REGS_BN_ALLOC_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_BN_VALID *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD =
{
    "ALLOC_BN_VALID",
#if RU_INCLUDE_DESC
    "",
    "alloc_bn_valid\nCHANGED_BY_HW\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD =
{
    "ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "alloc_bn\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD =
{
    "ACK",
#if RU_INCLUDE_DESC
    "",
    "ack\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NACK *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD =
{
    "NACK",
#if RU_INCLUDE_DESC
    "",
    "nack\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_HIGH *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD =
{
    "EXCL_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Exclusive bit is indication of Exclusive_high status of client with related Alloc request\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_MED *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_MED_FIELD =
{
    "EXCL_MED",
#if RU_INCLUDE_DESC
    "",
    "Exclusive bit is indication of Exclusive_med status of client with related Alloc request\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_MED_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_MED_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_MED_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_LOW *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD =
{
    "EXCL_LOW",
#if RU_INCLUDE_DESC
    "",
    "exclusive low\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "busy\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "rdy\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_MED_FIELD,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_REG =
{
    "REGS_BN_ALLOC_RPLY",
#if RU_INCLUDE_DESC
    "BN_ALLOC_RPLY Register",
    "reply for a new buffer alloc\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_REG_OFFSET },
    0,
    0,
    1021,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_MCST_INC, TYPE: Type_SBPM_SBPM_REGS_MCST_INC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_MCST_INC_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "",
    "bufer number\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_MCST_INC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_MCST_INC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MCST_VAL *****/
const ru_field_rec CRDP_ING_SBPM_REGS_MCST_INC_MCST_VAL_FIELD =
{
    "MCST_VAL",
#if RU_INCLUDE_DESC
    "",
    "MCST value that should be added to current mulicast counter\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_MCST_VAL_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_MCST_INC_MCST_VAL_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_MCST_INC_MCST_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_REQ *****/
const ru_field_rec CRDP_ING_SBPM_REGS_MCST_INC_ACK_REQ_FIELD =
{
    "ACK_REQ",
#if RU_INCLUDE_DESC
    "",
    "Acknowledge request\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_ACK_REQ_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_MCST_INC_ACK_REQ_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_MCST_INC_ACK_REQ_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_MCST_INC_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_MCST_INC_BN_FIELD,
    &CRDP_ING_SBPM_REGS_MCST_INC_MCST_VAL_FIELD,
    &CRDP_ING_SBPM_REGS_MCST_INC_ACK_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_MCST_INC *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_MCST_INC_REG =
{
    "REGS_MCST_INC",
#if RU_INCLUDE_DESC
    "MCST_INC Register",
    "Multicast counter increment. Contains the BN, which is head of the packet to be multicast and its counter value\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_REG_OFFSET },
    0,
    0,
    1022,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_ING_SBPM_REGS_MCST_INC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_MCST_INC_RPLY, TYPE: Type_SBPM_SBPM_REGS_MCST_INC_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MCST_ACK *****/
const ru_field_rec CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD =
{
    "MCST_ACK",
#if RU_INCLUDE_DESC
    "",
    "Acknowledge reply of MCST command\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MC_RES *****/
const ru_field_rec CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MC_RES_FIELD =
{
    "MC_RES",
#if RU_INCLUDE_DESC
    "",
    "MC increment result\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MC_RES_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MC_RES_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MC_RES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BSY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_MCST_INC_RPLY_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "",
    "The bit is used  as busy  indication of MCST request status (busy status)  by CPU\nSBPM asserts this bit on each valid request and de-asserts when request is treated:\n1 - request is busy,\n0- request is not busy (ready)\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_MCST_INC_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "The bit is used  as ready indication of MCST request status (ready status)  by CPU.\nSBPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is READY indication:\n1 - request is ready,\n0- request is not ready (busy)\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_MCST_INC_RPLY_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD,
    &CRDP_ING_SBPM_REGS_MCST_INC_RPLY_MC_RES_FIELD,
    &CRDP_ING_SBPM_REGS_MCST_INC_RPLY_BSY_FIELD,
    &CRDP_ING_SBPM_REGS_MCST_INC_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_MCST_INC_RPLY *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_MCST_INC_RPLY_REG =
{
    "REGS_MCST_INC_RPLY",
#if RU_INCLUDE_DESC
    "MCST_INC_RPLY Register",
    "mcst_inc_rply\n",
#endif
    { CRDP_ING_SBPM_REGS_MCST_INC_RPLY_REG_OFFSET },
    0,
    0,
    1023,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_ING_SBPM_REGS_MCST_INC_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_BN_CONNECT, TYPE: Type_SBPM_SBPM_REGS_BN_CONNECT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_CONNECT_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "",
    "bn\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_CONNECT_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_CONNECT_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_CONNECT_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_REQ *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD =
{
    "ACK_REQ",
#if RU_INCLUDE_DESC
    "",
    "ack_req for Connect command (should be always set)\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POINTED_BN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD =
{
    "POINTED_BN",
#if RU_INCLUDE_DESC
    "",
    "pointed_bn\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_BN_CONNECT_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_BN_CONNECT_BN_FIELD,
    &CRDP_ING_SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD,
    &CRDP_ING_SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_BN_CONNECT *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_BN_CONNECT_REG =
{
    "REGS_BN_CONNECT",
#if RU_INCLUDE_DESC
    "BN_CONNECT Register",
    "request for connection between two buffers in a linked list. The connection request may be replied with ACK message if the ACK request bit is asserted.\nThis command is used as write command.\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_CONNECT_REG_OFFSET },
    0,
    0,
    1024,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_ING_SBPM_REGS_BN_CONNECT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY, TYPE: Type_SBPM_SBPM_REGS_BN_CONNECT_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONNECT_ACK *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD =
{
    "CONNECT_ACK",
#if RU_INCLUDE_DESC
    "",
    "Acknowledge reply on Connect request\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "busy bit\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "ready bit\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD,
    &CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD,
    &CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_REG =
{
    "REGS_BN_CONNECT_RPLY",
#if RU_INCLUDE_DESC
    "BN_CONNECT_RPLY Register",
    "bn_connect_rply\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_REG_OFFSET },
    0,
    0,
    1025,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_GET_NEXT, TYPE: Type_SBPM_SBPM_REGS_GET_NEXT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GET_NEXT_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "",
    "Get Next Buffer of current BN (used in this field)\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_GET_NEXT_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GET_NEXT_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GET_NEXT_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_GET_NEXT_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_GET_NEXT_BN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_GET_NEXT *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_GET_NEXT_REG =
{
    "REGS_GET_NEXT",
#if RU_INCLUDE_DESC
    "GET_NEXT Register",
    "a pointer to a buffer in a packet linked list and request for the next buffer in the list\nthis command is used as read command.\n",
#endif
    { CRDP_ING_SBPM_REGS_GET_NEXT_REG_OFFSET },
    0,
    0,
    1026,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_GET_NEXT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_GET_NEXT_RPLY, TYPE: Type_SBPM_SBPM_REGS_GET_NEXT_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BN_VALID *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD =
{
    "BN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Used for validation of Next BN reply\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NEXT_BN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD =
{
    "NEXT_BN",
#if RU_INCLUDE_DESC
    "",
    "Next BN - reply of Get_next command\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUSY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "",
    "Get Next command is busy\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "Get Next command is ready\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD,
    &CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD,
    &CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD,
    &CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_GET_NEXT_RPLY *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_REG =
{
    "REGS_GET_NEXT_RPLY",
#if RU_INCLUDE_DESC
    "GET_NEXT_RPLY Register",
    "get_next_rply\n",
#endif
    { CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_REG_OFFSET },
    0,
    0,
    1027,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL, TYPE: Type_SBPM_SBPM_REGS_SBPM_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTERVL *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD =
{
    "KEEP_ALIVE_INTERVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG =
{
    "REGS_SBPM_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "SBPM_CLK_GATE_CNTRL Register",
    "control for the bl_clk_control module\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    1028,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1, TYPE: Type_SBPM_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PLEN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_PLEN_FIELD =
{
    "PLEN",
#if RU_INCLUDE_DESC
    "",
    "plen\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_PLEN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_PLEN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_PLEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SOP_OFFSET *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_SOP_OFFSET_FIELD =
{
    "SOP_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "number of packets\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_SOP_OFFSET_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_SOP_OFFSET_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_SOP_OFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CALC *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_CALC_FIELD =
{
    "CALC",
#if RU_INCLUDE_DESC
    "",
    "rsv\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_CALC_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_CALC_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_CALC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_PLEN_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_SOP_OFFSET_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_CALC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1 *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_REG =
{
    "REGS_BN_FREE_WITHOUT_CONTXT_REG1",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT_1 Register",
    "bn_free_without_contxt\n\n1. First write to BN_FREE_WITHOUT_CONTXT_REG1.\n2. Then write to BN_FREE_WITHOUT_CONTXT_REG2.\n\nAfter writing BN_FREE_WITHOUT_CONTXT_REG2 The free command executes.\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_REG_OFFSET },
    0,
    0,
    1029,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2, TYPE: Type_SBPM_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIRST_BN *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_FIRST_BN_FIELD =
{
    "FIRST_BN",
#if RU_INCLUDE_DESC
    "",
    "First BN\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_FIRST_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_FIRST_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_FIRST_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_PACKET *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_NUM_PACKET_FIELD =
{
    "NUM_PACKET",
#if RU_INCLUDE_DESC
    "",
    "number of packets\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_NUM_PACKET_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_NUM_PACKET_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_NUM_PACKET_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_FIRST_BN_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_NUM_PACKET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2 *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_REG =
{
    "REGS_BN_FREE_WITHOUT_CONTXT_REG2",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT_2 Register",
    "bn_free_without_contxt\n\n1. First write to BN_FREE_WITHOUT_CONTXT_REG1.\n2. Then write to BN_FREE_WITHOUT_CONTXT_REG2.\n\nAfter writing BN_FREE_WITHOUT_CONTXT_REG2 The free command executes\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_REG_OFFSET },
    0,
    0,
    1030,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY, TYPE: Type_SBPM_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_ACK *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD =
{
    "FREE_ACK",
#if RU_INCLUDE_DESC
    "",
    "Acknowledge on Free command\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_STAT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD =
{
    "ACK_STAT",
#if RU_INCLUDE_DESC
    "",
    "ACK status of CPU\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NACK_STAT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD =
{
    "NACK_STAT",
#if RU_INCLUDE_DESC
    "",
    "NACK status of CPU\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_HIGH_STAT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD =
{
    "EXCL_HIGH_STAT",
#if RU_INCLUDE_DESC
    "",
    "Exclusive_high status of CPU\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_MED_STAT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_MED_STAT_FIELD =
{
    "EXCL_MED_STAT",
#if RU_INCLUDE_DESC
    "",
    "Exclusive_med status of CPU\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_MED_STAT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_MED_STAT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_MED_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_LOW_STAT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD =
{
    "EXCL_LOW_STAT",
#if RU_INCLUDE_DESC
    "",
    "Exclusive_low status of CPU\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BSY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "",
    "Busy bit of command (command is currently in execution)\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "Ready bit of command (ready for new command execution)\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_MED_STAT_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG =
{
    "REGS_BN_FREE_WITHOUT_CONTXT_RPLY",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT_RPLY Register",
    "bn_free_without_contxt_rply\n",
#endif
    { CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG_OFFSET },
    0,
    0,
    1031,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_GL_TRSH, TYPE: Type_SBPM_SBPM_REGS_GL_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GL_BAT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAT_FIELD =
{
    "GL_BAT",
#if RU_INCLUDE_DESC
    "",
    "Global Threshold for Allocated BN = maximal total number of BNs in SBPM\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAT_FIELD_SHIFT },
    2047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GL_BAH *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAH_FIELD =
{
    "GL_BAH",
#if RU_INCLUDE_DESC
    "",
    "Global Hysteresis for Allocated BN = hysteresis value related to maximal total threshold of SRAM BNs\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAH_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAH_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_GL_TRSH_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAT_FIELD,
    &CRDP_ING_SBPM_REGS_GL_TRSH_GL_BAH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_GL_TRSH *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_GL_TRSH_REG =
{
    "REGS_GL_TRSH",
#if RU_INCLUDE_DESC
    "GLOBAL_THRESHOLD Register",
    "Global Threshold for Allocated Buffers.\nSBPM will issue BN in the accepted range upon to Global threshold setup.\nThs register also holds global hysteresis value for ACK/NACK transition setting. We cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_TRSH_REG_OFFSET },
    0,
    0,
    1032,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_GL_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_DBG, TYPE: Type_SBPM_SBPM_REGS_SBPM_DBG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SELECT_BUS *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD =
{
    "SELECT_BUS",
#if RU_INCLUDE_DESC
    "",
    "select bus. the bus index should be mentioned in onehot writting:\nbus0 = 0001\nbus1 = 0010\nbus2 = 0100\nbus3 = 1000\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WINDOW_START *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_DBG_WINDOW_START_FIELD =
{
    "WINDOW_START",
#if RU_INCLUDE_DESC
    "",
    "This field set the start of the bn allocation range.\n\nVector bit register dbg_bn_alloc[MAX_BN-1:0]. Each bit in the register is corresponding to a BN allocation.\nFor simulation the design will support 2K (ingress) and 16K (egress) bits vector. For synthesis the design will support only 2K bits vector with start BN window allocation and. It also affects the interrupts.\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_DBG_WINDOW_START_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_DBG_WINDOW_START_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_DBG_WINDOW_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_DBG_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_DBG_WINDOW_START_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_DBG *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_DBG_REG =
{
    "REGS_SBPM_DBG",
#if RU_INCLUDE_DESC
    "SBPM_DBG Register",
    "SBPM select the debug bus\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_DBG_REG_OFFSET },
    0,
    0,
    1033,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_SBPM_DBG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_GL_BAC, TYPE: Type_SBPM_SBPM_REGS_GL_BAC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_BAC_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Global BN counter\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_BAC_VAL_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_BAC_VAL_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_BAC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_GL_BAC_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_GL_BAC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_GL_BAC *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_GL_BAC_REG =
{
    "REGS_GL_BAC",
#if RU_INCLUDE_DESC
    "GL_BAC Register",
    "SBPM global BN Counter\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_BAC_REG_OFFSET },
    0,
    0,
    1034,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_GL_BAC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH, TYPE: Type_SBPM_SBPM_REGS_GL_EXCL_HIGH_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "",
    "exclusive high threshold\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLT_FIELD_SHIFT },
    1920,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLH *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "",
    "exclusive histeresis threshold\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLH_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLH_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLT_FIELD,
    &CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_EXCLH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_REG =
{
    "REGS_GL_EXCL_HIGH_TRSH",
#if RU_INCLUDE_DESC
    "GL_EXCLUSIVE_HIGH_THRESHOLD Register",
    "SBPM GL Exclusive high and hysteresis threshold.\nWe cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_REG_OFFSET },
    0,
    0,
    1035,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH, TYPE: Type_SBPM_SBPM_REGS_GL_EXCL_LOW_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "",
    "exclusive low threshold\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLT_FIELD_SHIFT },
    1408,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLH *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "",
    "exclusive histeresis threshold\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLH_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLH_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLT_FIELD,
    &CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_EXCLH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_REG =
{
    "REGS_GL_EXCL_LOW_TRSH",
#if RU_INCLUDE_DESC
    "GL_EXCLUSIVE_LOW_THRESHOLD Register",
    "SBPM GL Exclusive low and hysteresis threshold.\nWe cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_REG_OFFSET },
    0,
    0,
    1036,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH, TYPE: Type_SBPM_SBPM_REGS_GL_EXCL_MED_TRSH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLT *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "",
    "exclusive low threshold\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLT_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLT_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLT_FIELD_SHIFT },
    1024,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLH *****/
const ru_field_rec CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "",
    "exclusive histeresis threshold\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLH_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLH_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLH_FIELD_SHIFT },
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLT_FIELD,
    &CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_EXCLH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_REG =
{
    "REGS_GL_EXCL_MED_TRSH",
#if RU_INCLUDE_DESC
    "GL_EXCLUSIVE_MED_THRESHOLD Register",
    "SBPM GL Exclusive medium and hysteresis threshold.\nWe cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.\n",
#endif
    { CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_REG_OFFSET },
    0,
    0,
    1037,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_GL_STATUS, TYPE: Type_SBPM_SBPM_REGS_SBPM_GL_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACK_STTS *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_ACK_STTS_FIELD =
{
    "ACK_STTS",
#if RU_INCLUDE_DESC
    "",
    "Ack/Nack status\n0 - NACK\n1 - ACK\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_ACK_STTS_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_ACK_STTS_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_ACK_STTS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_HIGH_STTS *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_HIGH_STTS_FIELD =
{
    "EXCL_HIGH_STTS",
#if RU_INCLUDE_DESC
    "",
    "High EXCL/Non-Excl\n0 - non_exclusive\n1 - exclusive\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_HIGH_STTS_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_HIGH_STTS_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_HIGH_STTS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_MED_STTS *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_MED_STTS_FIELD =
{
    "EXCL_MED_STTS",
#if RU_INCLUDE_DESC
    "",
    "Med EXCL/Non-Excl status.\n0 - non_exclusive\n1 - exclusive\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_MED_STTS_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_MED_STTS_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_MED_STTS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_LOW_STTS *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_LOW_STTS_FIELD =
{
    "EXCL_LOW_STTS",
#if RU_INCLUDE_DESC
    "",
    "Low EXCL/Non-Excl status\n0 - non_exclusive\n1 - exclusive\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_LOW_STTS_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_LOW_STTS_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_LOW_STTS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_ACK_STTS_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_HIGH_STTS_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_MED_STTS_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_EXCL_LOW_STTS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_GL_STATUS *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_REG =
{
    "REGS_SBPM_GL_STATUS",
#if RU_INCLUDE_DESC
    "GLOBAL_STATUS Register",
    "ACK/NACK and EXCLUSIVE Status\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_REG_OFFSET },
    0,
    0,
    1038,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_IIR_ADDR, TYPE: Type_SBPM_SBPM_REGS_IIR_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_SA *****/
const ru_field_rec CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_SA_FIELD =
{
    "CMD_SA",
#if RU_INCLUDE_DESC
    "",
    "Source addres of command that caused Interrupt\n",
#endif
    { CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_SA_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_SA_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_SA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_TA *****/
const ru_field_rec CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_TA_FIELD =
{
    "CMD_TA",
#if RU_INCLUDE_DESC
    "",
    "Target address of command that caused Interrupt\n",
#endif
    { CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_TA_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_TA_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_TA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BN_SRC *****/
const ru_field_rec CRDP_ING_SBPM_REGS_IIR_ADDR_BN_SRC_FIELD =
{
    "BN_SRC",
#if RU_INCLUDE_DESC
    "",
    "The non-allocated BN that caused the interrupt (non_alloc interrupt).\n",
#endif
    { CRDP_ING_SBPM_REGS_IIR_ADDR_BN_SRC_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_IIR_ADDR_BN_SRC_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_IIR_ADDR_BN_SRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_IIR_ADDR_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_SA_FIELD,
    &CRDP_ING_SBPM_REGS_IIR_ADDR_CMD_TA_FIELD,
    &CRDP_ING_SBPM_REGS_IIR_ADDR_BN_SRC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_IIR_ADDR *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_IIR_ADDR_REG =
{
    "REGS_IIR_ADDR",
#if RU_INCLUDE_DESC
    "IIR_ADDR_REGISTER Register",
    "SBPM SA & TA Address\n\nThe first error source is sampled in to the registers IIR_ADDR, sbpm_iir_low and sbpm_iir_high and the registers are locked so no new error source could be sampled. In order to unlock them you need to read the register sbpm_iir_high.\n",
#endif
    { CRDP_ING_SBPM_REGS_IIR_ADDR_REG_OFFSET },
    0,
    0,
    1039,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_ING_SBPM_REGS_IIR_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_IIR_LOW, TYPE: Type_SBPM_SBPM_REGS_SBPM_IIR_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_DATA_0TO31 *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD =
{
    "CMD_DATA_0TO31",
#if RU_INCLUDE_DESC
    "",
    "Interrupt command data lowest 32-bit  (latched from BB data[31:0] or CPU request data)\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_0TO31_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_IIR_LOW *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_REG =
{
    "REGS_SBPM_IIR_LOW",
#if RU_INCLUDE_DESC
    "SBPM_IIR_LOW_REGISTER Register",
    "SBPM IIR low (Interrupt information register)\n\nThe first error source is sampled in to the registers IIR_ADDR, sbpm_iir_low and sbpm_iir_high and the registers are locked so no new error source could be sampled. In order to unlock them you need to read the register sbpm_iir_high.\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_REG_OFFSET },
    0,
    0,
    1040,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH, TYPE: Type_SBPM_SBPM_REGS_SBPM_IIR_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_DATA_32TO63 *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD =
{
    "CMD_DATA_32TO63",
#if RU_INCLUDE_DESC
    "",
    "Data (bits [63:32], with reserved bits) of the command that caused interrupt\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_32TO63_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_REG =
{
    "REGS_SBPM_IIR_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_IIR_HIGH_REGISTER Register",
    "SBPM IIR high (Interrupt information register)\n\n\nThe first error source is sampled in to the registers IIR_ADDR, sbpm_iir_low and sbpm_iir_high and the registers are locked so no new error source could be sampled. In order to unlock them you need to read the register sbpm_iir_high.\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_REG_OFFSET },
    0,
    0,
    1041,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0, TYPE: Type_SBPM_SBPM_REGS_SBPM_DBG_VEC0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_FIFO_EMPTY *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_SBPM_FIFO_EMPTY_FIELD =
{
    "SBPM_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "SBPM FIFO empty status\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_SBPM_FIFO_EMPTY_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_SBPM_FIFO_EMPTY_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_SBPM_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_SBPM_FIFO_EMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0 *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_REG =
{
    "REGS_SBPM_DBG_VEC0",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC0 Register",
    "SBPM debug vector0 includes FIFO empty status bits\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_REG_OFFSET },
    0,
    0,
    1042,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1, TYPE: Type_SBPM_SBPM_REGS_SBPM_DBG_VEC1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_FIFO_FULL *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_SBPM_FIFO_FULL_FIELD =
{
    "SBPM_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "SBPM FIFO full status\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_SBPM_FIFO_FULL_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_SBPM_FIFO_FULL_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_SBPM_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_SBPM_FIFO_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1 *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_REG =
{
    "REGS_SBPM_DBG_VEC1",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC1 Register",
    "SBPM debug vector1 includes FIFO full status bits\n\n\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_REG_OFFSET },
    0,
    0,
    1043,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1, TYPE: Type_SBPM_SBPM_REGS_SBPM_RADDR_DECODER_REG1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERWR_RA *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_OVERWR_RA_FIELD =
{
    "OVERWR_RA",
#if RU_INCLUDE_DESC
    "",
    "The new RA\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_OVERWR_RA_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_OVERWR_RA_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_OVERWR_RA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_OVERWR_RA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1 *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_REG =
{
    "REGS_SBPM_RADDR_DECODER_REG1",
#if RU_INCLUDE_DESC
    "SBPM_RADDR_DECODER1 Register",
    "This register let you choose one user that you would like to change its default RA.\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_REG_OFFSET },
    0,
    0,
    1044,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2, TYPE: Type_SBPM_SBPM_REGS_SBPM_RADDR_DECODER_REG2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ID_2OVERWR *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_ID_2OVERWR_FIELD =
{
    "ID_2OVERWR",
#if RU_INCLUDE_DESC
    "",
    "this field contains the users id that you want to override its default RA\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_ID_2OVERWR_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_ID_2OVERWR_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_ID_2OVERWR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERWR_VALID *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_OVERWR_VALID_FIELD =
{
    "OVERWR_VALID",
#if RU_INCLUDE_DESC
    "",
    "the overwr mechanism will be used only if this bit is active (1).\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_OVERWR_VALID_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_OVERWR_VALID_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_OVERWR_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_ID_2OVERWR_FIELD,
    &CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_OVERWR_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2 *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_REG =
{
    "REGS_SBPM_RADDR_DECODER_REG2",
#if RU_INCLUDE_DESC
    "SBPM_RADDR_DECODER2 Register",
    "This register let you choose one user that you would like to change its default RA.\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_REG_OFFSET },
    0,
    0,
    1045,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_BAC_MAX, TYPE: Type_SBPM_SBPM_REGS_BAC_MAX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BAC_MAX_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "This is the maximum BN value that have been recorded.\n\n",
#endif
    { CRDP_ING_SBPM_REGS_BAC_MAX_VAL_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BAC_MAX_VAL_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BAC_MAX_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CLR *****/
const ru_field_rec CRDP_ING_SBPM_REGS_BAC_MAX_CLR_FIELD =
{
    "CLR",
#if RU_INCLUDE_DESC
    "",
    "max value clear\n",
#endif
    { CRDP_ING_SBPM_REGS_BAC_MAX_CLR_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_BAC_MAX_CLR_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_BAC_MAX_CLR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_BAC_MAX_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_BAC_MAX_VAL_FIELD,
    &CRDP_ING_SBPM_REGS_BAC_MAX_CLR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_BAC_MAX *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_BAC_MAX_REG =
{
    "REGS_BAC_MAX",
#if RU_INCLUDE_DESC
    "BAC_MAX Register",
    "SBPM BAC max value\n",
#endif
    { CRDP_ING_SBPM_REGS_BAC_MAX_REG_OFFSET },
    0,
    0,
    1046,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_BAC_MAX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SBPM_SPARE, TYPE: Type_SBPM_SBPM_REGS_SBPM_SPARE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SPR *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SBPM_SPARE_SPR_FIELD =
{
    "SPR",
#if RU_INCLUDE_DESC
    "",
    "reserved bits\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_SPARE_SPR_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SBPM_SPARE_SPR_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SBPM_SPARE_SPR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SBPM_SPARE_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SBPM_SPARE_SPR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SBPM_SPARE *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SBPM_SPARE_REG =
{
    "REGS_SBPM_SPARE",
#if RU_INCLUDE_DESC
    "SBPM_SPARE Register",
    "sbpm spare register\n",
#endif
    { CRDP_ING_SBPM_REGS_SBPM_SPARE_REG_OFFSET },
    0,
    0,
    1047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_SBPM_SPARE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_CFG_BN_SIZE, TYPE: Type_SBPM_SBPM_REGS_CFG_BN_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE *****/
const ru_field_rec CRDP_ING_SBPM_REGS_CFG_BN_SIZE_SIZE_FIELD =
{
    "SIZE",
#if RU_INCLUDE_DESC
    "",
    "Configure the buffer number (BN) size for BN free without context calculation.\n\n0: BN is 128 bytes\n1: BN is 256 bytes\n2: BN is 512 bytes\n3: reserved\n",
#endif
    { CRDP_ING_SBPM_REGS_CFG_BN_SIZE_SIZE_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_CFG_BN_SIZE_SIZE_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_CFG_BN_SIZE_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_CFG_BN_SIZE_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_CFG_BN_SIZE_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_CFG_BN_SIZE *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_CFG_BN_SIZE_REG =
{
    "REGS_CFG_BN_SIZE",
#if RU_INCLUDE_DESC
    "CFG_BN_SIZE Register",
    "Configure the buffer number (BN) Size for BN free without context calculation.\n\n0: BN is 128 bytes\n1: BN is 256 bytes\n2: BN is 512 bytes\n3: reserved\n",
#endif
    { CRDP_ING_SBPM_REGS_CFG_BN_SIZE_REG_OFFSET },
    0,
    0,
    1048,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_CFG_BN_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB, TYPE: Type_SBPM_SBPM_REGS_LL_WEIGHT_ARB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "bits 0-2  : weights for connect (wr access)\nbits 3-5  : get fifo a (rd access)\nbits 6-8  : get fifo b (rd access)\nbits 9-11 : mc2get fifo (rd access)\nbits 12-14: rsv\nbits 15-17: rsv\n",
#endif
    { CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_VAL_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_VAL_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_REG =
{
    "REGS_LL_WEIGHT_ARB",
#if RU_INCLUDE_DESC
    "LL_WEIGHT_ARB Register",
    "Weights for arbitration access to link list memory.\nLink list memory has 6 clients and each of them has 3 bits weight configuration.\n",
#endif
    { CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_REG_OFFSET },
    0,
    0,
    1049,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SP_RNR_G0, TYPE: Type_SBPM_SBPM_REGS_SP_RNR_G0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SP_RNR_G0_START_FIELD =
{
    "START",
#if RU_INCLUDE_DESC
    "",
    "runner start number\n",
#endif
    { CRDP_ING_SBPM_REGS_SP_RNR_G0_START_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SP_RNR_G0_START_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SP_RNR_G0_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: END *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SP_RNR_G0_END_FIELD =
{
    "END",
#if RU_INCLUDE_DESC
    "",
    "runner end number\n",
#endif
    { CRDP_ING_SBPM_REGS_SP_RNR_G0_END_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SP_RNR_G0_END_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SP_RNR_G0_END_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SP_RNR_G0_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SP_RNR_G0_START_FIELD,
    &CRDP_ING_SBPM_REGS_SP_RNR_G0_END_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SP_RNR_G0 *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SP_RNR_G0_REG =
{
    "REGS_SP_RNR_G0",
#if RU_INCLUDE_DESC
    "SP_RNR_G0 Register",
    "The runner client numbers are consecutive.\nThis register holds the runner client start number and end number for group 0.\n",
#endif
    { CRDP_ING_SBPM_REGS_SP_RNR_G0_REG_OFFSET },
    0,
    0,
    1050,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_SP_RNR_G0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_SP_RNR_G1, TYPE: Type_SBPM_SBPM_REGS_SP_RNR_G1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SP_RNR_G1_START_FIELD =
{
    "START",
#if RU_INCLUDE_DESC
    "",
    "runner start number\n",
#endif
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_START_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_START_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_START_FIELD_SHIFT },
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: END *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SP_RNR_G1_END_FIELD =
{
    "END",
#if RU_INCLUDE_DESC
    "",
    "runner end number\n",
#endif
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_END_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_END_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_END_FIELD_SHIFT },
    319,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_ING_SBPM_REGS_SP_RNR_G1_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "rsv\n",
#endif
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_VALID_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_VALID_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_VALID_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_SP_RNR_G1_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_SP_RNR_G1_START_FIELD,
    &CRDP_ING_SBPM_REGS_SP_RNR_G1_END_FIELD,
    &CRDP_ING_SBPM_REGS_SP_RNR_G1_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_SP_RNR_G1 *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_SP_RNR_G1_REG =
{
    "REGS_SP_RNR_G1",
#if RU_INCLUDE_DESC
    "SP_RNR_G1 Register",
    "The runner SP/client numbers are consecutive.\nThis register holds the runner client start number and end number for group 1.\n\n",
#endif
    { CRDP_ING_SBPM_REGS_SP_RNR_G1_REG_OFFSET },
    0,
    0,
    1051,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_ING_SBPM_REGS_SP_RNR_G1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_NACK_MASK, TYPE: Type_SBPM_SBPM_REGS_NACK_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NACK_MASK *****/
const ru_field_rec CRDP_ING_SBPM_REGS_NACK_MASK_NACK_MASK_FIELD =
{
    "NACK_MASK",
#if RU_INCLUDE_DESC
    "",
    "non-runner client number/ID\n",
#endif
    { CRDP_ING_SBPM_REGS_NACK_MASK_NACK_MASK_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_NACK_MASK_NACK_MASK_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_NACK_MASK_NACK_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_ING_SBPM_REGS_NACK_MASK_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_ING_SBPM_REGS_NACK_MASK_VALID_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_NACK_MASK_VALID_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_NACK_MASK_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_NACK_MASK_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_NACK_MASK_NACK_MASK_FIELD,
    &CRDP_ING_SBPM_REGS_NACK_MASK_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_NACK_MASK *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_NACK_MASK_REG =
{
    "REGS_NACK_MASK",
#if RU_INCLUDE_DESC
    "NACK_MASK Register",
    "sbpm_nack_mask register\n",
#endif
    { CRDP_ING_SBPM_REGS_NACK_MASK_REG_OFFSET },
    0,
    0,
    1052,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_SBPM_REGS_NACK_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_REGS_EXCL_MASK, TYPE: Type_SBPM_SBPM_REGS_EXCL_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCL_MASK *****/
const ru_field_rec CRDP_ING_SBPM_REGS_EXCL_MASK_EXCL_MASK_FIELD =
{
    "EXCL_MASK",
#if RU_INCLUDE_DESC
    "",
    "This register holds the number of the client that should receive congestion message.\n",
#endif
    { CRDP_ING_SBPM_REGS_EXCL_MASK_EXCL_MASK_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_REGS_EXCL_MASK_EXCL_MASK_FIELD_WIDTH },
    { CRDP_ING_SBPM_REGS_EXCL_MASK_EXCL_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_REGS_EXCL_MASK_FIELDS[] =
{
    &CRDP_ING_SBPM_REGS_EXCL_MASK_EXCL_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_REGS_EXCL_MASK *****/
const ru_reg_rec CRDP_ING_SBPM_REGS_EXCL_MASK_REG =
{
    "REGS_EXCL_MASK",
#if RU_INCLUDE_DESC
    "EXCL_MASK Register",
    "This register hold the client number that should receive congestion message.\n",
#endif
    { CRDP_ING_SBPM_REGS_EXCL_MASK_REG_OFFSET },
    0,
    0,
    1053,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_REGS_EXCL_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_INTR_CTRL_ISR, TYPE: Type_SBPM_SBPM_INTR_CTRL_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_LIST_NON_ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_LIST_NON_ALLOC_BN_FIELD =
{
    "FREE_LIST_NON_ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "This error bit is set when a non allocated BN is free by free list command.\n\nOnly the lower part of the free list command is captured (the first 4 BNs).\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_LIST_NON_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_LIST_NON_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_LIST_NON_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GET2FREE_NON_ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_GET2FREE_NON_ALLOC_BN_FIELD =
{
    "GET2FREE_NON_ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "This error bit is set when a non allocated BN is free by free w/o context command.\n\nThe command source is not captured.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET2FREE_NON_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET2FREE_NON_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET2FREE_NON_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MC2FREE_NON_ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_MC2FREE_NON_ALLOC_BN_FIELD =
{
    "MC2FREE_NON_ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "This error bit is set when a non allocated BN is free by free w/o context command. This BN is the head BN or the third BN when mcst_srch == 4.\n\nThe command source is not captured.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MC2FREE_NON_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MC2FREE_NON_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MC2FREE_NON_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_ALRDY_ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_ALLOC_ALRDY_ALLOC_BN_FIELD =
{
    "ALLOC_ALRDY_ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates that an already allocated BN was allocated again.\n\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_ALLOC_ALRDY_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_ALLOC_ALRDY_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_ALLOC_ALRDY_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GET_NON_ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_GET_NON_ALLOC_BN_FIELD =
{
    "GET_NON_ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "This bit error indicates that a get request is executed on non allocated BN\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET_NON_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET_NON_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET_NON_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GET_RES_NON_ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_GET_RES_NON_ALLOC_BN_FIELD =
{
    "GET_RES_NON_ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "This bit error indicates that a get result is not allocated.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET_RES_NON_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET_RES_NON_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_GET_RES_NON_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CONNECT_NON_ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_NON_ALLOC_BN_FIELD =
{
    "CONNECT_NON_ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates a connect request was set for non allocated BN\n\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_NON_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_NON_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_NON_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CONNECT_TO_NON_ALLOC_POINTED_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_TO_NON_ALLOC_POINTED_BN_FIELD =
{
    "CONNECT_TO_NON_ALLOC_POINTED_BN",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates a connect request is set to non allocated pointed BN.\n\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_TO_NON_ALLOC_POINTED_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_TO_NON_ALLOC_POINTED_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_TO_NON_ALLOC_POINTED_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MC_INC_NON_ALLOC_BN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_MC_INC_NON_ALLOC_BN_FIELD =
{
    "MC_INC_NON_ALLOC_BN",
#if RU_INCLUDE_DESC
    "",
    "This bit error indicates that a mc increment request is for non allocated BN or If the command is free w/o context 1st/2nd level BNs are not allocated.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MC_INC_NON_ALLOC_BN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MC_INC_NON_ALLOC_BN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MC_INC_NON_ALLOC_BN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_WO_CONTEXT_COMB *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_WO_CONTEXT_COMB_FIELD =
{
    "FREE_WO_CONTEXT_COMB",
#if RU_INCLUDE_DESC
    "",
    "This bits indicate that the free w/o context command was sent with illegal combinations.\n\nbit [0]: Number of BNs == 1 and 2nd BN valid == 1.\n\nbit [1]: Number of BNs is lower than decrements value (which could be 1, 2 or 3).\n\nbit [2]: mcst_srch == 0 and 2nd BN valid == 1.\n\nbit [3]: Number of BNs == 0.\n\nbit [4]: mcst_srch != 0 and 2nd BN valid == 0 and Number of BNs > 1.\n\nbit [5]: mcst_srch == 4 and Number of BNs < 3.\n\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_WO_CONTEXT_COMB_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_WO_CONTEXT_COMB_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_WO_CONTEXT_COMB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BAC_UNDERRUN *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD =
{
    "BAC_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "This error bit indicates under-run state of SBPM Buffer Allocated Counter.\n\nThe source command is not captured.\n\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MCST_OVERFLOW *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD =
{
    "MCST_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "This error bit indicates if the Multi Cast result is overflow.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY_FREE_LIST *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_EMPTY_FREE_LIST_FIELD =
{
    "EMPTY_FREE_LIST",
#if RU_INCLUDE_DESC
    "",
    "This error bit indicates the bn free list command is empty. No valid bit was set.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_EMPTY_FREE_LIST_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_EMPTY_FREE_LIST_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_EMPTY_FREE_LIST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ERROR_BB_CMD_MSG *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISR_ERROR_BB_CMD_MSG_FIELD =
{
    "ERROR_BB_CMD_MSG",
#if RU_INCLUDE_DESC
    "",
    "This error bit indicates undefined BB command message was sent.\nThe cmd == 101b or cmd == 111b are undefined.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_ERROR_BB_CMD_MSG_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISR_ERROR_BB_CMD_MSG_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISR_ERROR_BB_CMD_MSG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_INTR_CTRL_ISR_FIELDS[] =
{
    &CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_LIST_NON_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_GET2FREE_NON_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_MC2FREE_NON_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_ALLOC_ALRDY_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_GET_NON_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_GET_RES_NON_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_NON_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_CONNECT_TO_NON_ALLOC_POINTED_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_MC_INC_NON_ALLOC_BN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_FREE_WO_CONTEXT_COMB_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_EMPTY_FREE_LIST_FIELD,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_ERROR_BB_CMD_MSG_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_INTR_CTRL_ISR *****/
const ru_reg_rec CRDP_ING_SBPM_INTR_CTRL_ISR_REG =
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.\n\nWhen an error interrupt is set the source command that caused the interrupt is captured to following registers:\n\nIIR_ADDR, SBPM_IIR_LOW, SBPM_IIR_HIGH\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISR_REG_OFFSET },
    0,
    0,
    1054,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    14,
    CRDP_ING_SBPM_INTR_CTRL_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_INTR_CTRL_ISM, TYPE: Type_SBPM_SBPM_INTR_CTRL_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ISM *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "",
    "Status Masked of corresponding interrupt source in the ISR\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISM_ISM_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ISM_ISM_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ISM_ISM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_INTR_CTRL_ISM_FIELDS[] =
{
    &CRDP_ING_SBPM_INTR_CTRL_ISM_ISM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_INTR_CTRL_ISM *****/
const ru_reg_rec CRDP_ING_SBPM_INTR_CTRL_ISM_REG =
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ISM_REG_OFFSET },
    0,
    0,
    1055,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_INTR_CTRL_ISM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_INTR_CTRL_IER, TYPE: Type_SBPM_SBPM_INTR_CTRL_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IEM *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask controls the corresponding interrupt source in the IER\nTRIGGERED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_IER_IEM_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_IER_IEM_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_IER_IEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_INTR_CTRL_IER_FIELDS[] =
{
    &CRDP_ING_SBPM_INTR_CTRL_IER_IEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_INTR_CTRL_IER *****/
const ru_reg_rec CRDP_ING_SBPM_INTR_CTRL_IER_REG =
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_IER_REG_OFFSET },
    0,
    0,
    1056,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_INTR_CTRL_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_SBPM_INTR_CTRL_ITR, TYPE: Type_SBPM_SBPM_INTR_CTRL_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec CRDP_ING_SBPM_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask tests the corresponding interrupt source in the ISR\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ITR_IST_FIELD_MASK },
    0,
    { CRDP_ING_SBPM_INTR_CTRL_ITR_IST_FIELD_WIDTH },
    { CRDP_ING_SBPM_INTR_CTRL_ITR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_SBPM_INTR_CTRL_ITR_FIELDS[] =
{
    &CRDP_ING_SBPM_INTR_CTRL_ITR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_SBPM_INTR_CTRL_ITR *****/
const ru_reg_rec CRDP_ING_SBPM_INTR_CTRL_ITR_REG =
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR\n",
#endif
    { CRDP_ING_SBPM_INTR_CTRL_ITR_REG_OFFSET },
    0,
    0,
    1057,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_SBPM_INTR_CTRL_ITR_FIELDS,
#endif
};

unsigned long CRDP_ING_SBPM_ADDRS[] =
{
    0xA20A0000,
    0xA20B0000,
    0xA20C0000,
};

static const ru_reg_rec *CRDP_ING_SBPM_REGS[] =
{
    &CRDP_ING_SBPM_REGS_INIT_FREE_LIST_REG,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_REG,
    &CRDP_ING_SBPM_REGS_BN_ALLOC_RPLY_REG,
    &CRDP_ING_SBPM_REGS_MCST_INC_REG,
    &CRDP_ING_SBPM_REGS_MCST_INC_RPLY_REG,
    &CRDP_ING_SBPM_REGS_BN_CONNECT_REG,
    &CRDP_ING_SBPM_REGS_BN_CONNECT_RPLY_REG,
    &CRDP_ING_SBPM_REGS_GET_NEXT_REG,
    &CRDP_ING_SBPM_REGS_GET_NEXT_RPLY_REG,
    &CRDP_ING_SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG1_REG,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG2_REG,
    &CRDP_ING_SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG,
    &CRDP_ING_SBPM_REGS_GL_TRSH_REG,
    &CRDP_ING_SBPM_REGS_SBPM_DBG_REG,
    &CRDP_ING_SBPM_REGS_GL_BAC_REG,
    &CRDP_ING_SBPM_REGS_GL_EXCL_HIGH_TRSH_REG,
    &CRDP_ING_SBPM_REGS_GL_EXCL_LOW_TRSH_REG,
    &CRDP_ING_SBPM_REGS_GL_EXCL_MED_TRSH_REG,
    &CRDP_ING_SBPM_REGS_SBPM_GL_STATUS_REG,
    &CRDP_ING_SBPM_REGS_IIR_ADDR_REG,
    &CRDP_ING_SBPM_REGS_SBPM_IIR_LOW_REG,
    &CRDP_ING_SBPM_REGS_SBPM_IIR_HIGH_REG,
    &CRDP_ING_SBPM_REGS_SBPM_DBG_VEC0_REG,
    &CRDP_ING_SBPM_REGS_SBPM_DBG_VEC1_REG,
    &CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG1_REG,
    &CRDP_ING_SBPM_REGS_SBPM_RADDR_DECODER_REG2_REG,
    &CRDP_ING_SBPM_REGS_BAC_MAX_REG,
    &CRDP_ING_SBPM_REGS_SBPM_SPARE_REG,
    &CRDP_ING_SBPM_REGS_CFG_BN_SIZE_REG,
    &CRDP_ING_SBPM_REGS_LL_WEIGHT_ARB_REG,
    &CRDP_ING_SBPM_REGS_SP_RNR_G0_REG,
    &CRDP_ING_SBPM_REGS_SP_RNR_G1_REG,
    &CRDP_ING_SBPM_REGS_NACK_MASK_REG,
    &CRDP_ING_SBPM_REGS_EXCL_MASK_REG,
    &CRDP_ING_SBPM_INTR_CTRL_ISR_REG,
    &CRDP_ING_SBPM_INTR_CTRL_ISM_REG,
    &CRDP_ING_SBPM_INTR_CTRL_IER_REG,
    &CRDP_ING_SBPM_INTR_CTRL_ITR_REG,
};

const ru_block_rec CRDP_ING_SBPM_BLOCK =
{
    "CRDP_ING_SBPM",
#if RU_INCLUDE_SIZE
    { 0x00000810 },
#endif
    CRDP_ING_SBPM_ADDRS,
    3,
    39,
    CRDP_ING_SBPM_REGS,
};
