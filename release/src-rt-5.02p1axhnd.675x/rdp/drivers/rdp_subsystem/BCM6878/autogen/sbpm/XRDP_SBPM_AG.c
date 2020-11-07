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
 * Field: SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR
 ******************************************************************************/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD =
{
    "INIT_BASE_ADDR",
#if RU_INCLUDE_DESC
    "init_head_bn_addr",
    "init_base_addr",
#endif
    SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD_MASK,
    0,
    SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD_WIDTH,
    SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET
 ******************************************************************************/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD =
{
    "INIT_OFFSET",
#if RU_INCLUDE_DESC
    "init_offset",
    "init_offset",
#endif
    SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD_MASK,
    0,
    SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD_WIDTH,
    SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_INIT_FREE_LIST_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_INIT_FREE_LIST_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_INIT_FREE_LIST_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_INIT_FREE_LIST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_INIT_FREE_LIST_BSY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "busy",
    "The bit is used  as busy  indication of buffer allocation request status (busy status)  by CPU."
    "BPM asserts this bit on each valid request and de-asserts when request is treated.",
#endif
    SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_MASK,
    0,
    SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_WIDTH,
    SBPM_REGS_INIT_FREE_LIST_BSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_INIT_FREE_LIST_RDY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_INIT_FREE_LIST_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "ready",
    "The bit is used  as ready indication of buffer allocation request status (ready status)  by CPU."
    "BPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is READY indication",
#endif
    SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_MASK,
    0,
    SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_WIDTH,
    SBPM_REGS_INIT_FREE_LIST_RDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_ALLOC_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_SA
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_SA_FIELD =
{
    "SA",
#if RU_INCLUDE_DESC
    "source_address",
    "Source address used by Alloc BN command (may be used for alloc on behalf another user)",
#endif
    SBPM_REGS_BN_ALLOC_SA_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_SA_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_SA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_ALLOC_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD =
{
    "ALLOC_BN_VALID",
#if RU_INCLUDE_DESC
    "alloc_bn_valid",
    "alloc_bn_valid",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD =
{
    "ALLOC_BN",
#if RU_INCLUDE_DESC
    "alloc_bn",
    "alloc_bn",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD =
{
    "ACK",
#if RU_INCLUDE_DESC
    "ack",
    "ack",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_NACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD =
{
    "NACK",
#if RU_INCLUDE_DESC
    "nack",
    "nack",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD =
{
    "EXCL_HIGH",
#if RU_INCLUDE_DESC
    "excl_high",
    "Exclusive bit is indication of Exclusive_high status of client with related Alloc request",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD =
{
    "EXCL_LOW",
#if RU_INCLUDE_DESC
    "excl_low",
    "Exclusive bit is indication of Exclusive_low status of client with related Alloc request",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_BUSY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "busy",
    "busy",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_ALLOC_RPLY_RDY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "rdy",
    "rdy",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_MASK,
    0,
    SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_WIDTH,
    SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD =
{
    "HEAD_BN",
#if RU_INCLUDE_DESC
    "head_bn",
    "head_bn",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD =
{
    "SA",
#if RU_INCLUDE_DESC
    "source_address",
    "Source addres used for free comand (may be used for freeing BN on behalf another port)",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD =
{
    "OFFSET",
#if RU_INCLUDE_DESC
    "offset",
    "Offset (or length) = number of BNs in packet that is going to be freed",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD =
{
    "ACK",
#if RU_INCLUDE_DESC
    "ack",
    "Ack request",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD =
{
    "LAST_BN",
#if RU_INCLUDE_DESC
    "last_BN",
    "Last BN in packet that is going to be freed",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_MCST_INC_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_MCST_INC_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "bufer_number",
    "bufer number",
#endif
    SBPM_REGS_MCST_INC_BN_FIELD_MASK,
    0,
    SBPM_REGS_MCST_INC_BN_FIELD_WIDTH,
    SBPM_REGS_MCST_INC_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_MCST_INC_MCST_VAL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_MCST_INC_MCST_VAL_FIELD =
{
    "MCST_VAL",
#if RU_INCLUDE_DESC
    "mcst_val",
    "MCST value that should be added to current mulicast counter",
#endif
    SBPM_REGS_MCST_INC_MCST_VAL_FIELD_MASK,
    0,
    SBPM_REGS_MCST_INC_MCST_VAL_FIELD_WIDTH,
    SBPM_REGS_MCST_INC_MCST_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_MCST_INC_ACK_REQ
 ******************************************************************************/
const ru_field_rec SBPM_REGS_MCST_INC_ACK_REQ_FIELD =
{
    "ACK_REQ",
#if RU_INCLUDE_DESC
    "ack_req",
    "Acknowledge request",
#endif
    SBPM_REGS_MCST_INC_ACK_REQ_FIELD_MASK,
    0,
    SBPM_REGS_MCST_INC_ACK_REQ_FIELD_WIDTH,
    SBPM_REGS_MCST_INC_ACK_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_MCST_INC_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_MCST_INC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_MCST_INC_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_MCST_INC_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_MCST_INC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_MCST_INC_RPLY_MCST_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD =
{
    "MCST_ACK",
#if RU_INCLUDE_DESC
    "mcst_ack",
    "Acknowledge reply of MCST command",
#endif
    SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_MASK,
    0,
    SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_WIDTH,
    SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_MCST_INC_RPLY_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_MCST_INC_RPLY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_MCST_INC_RPLY_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_MCST_INC_RPLY_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_MCST_INC_RPLY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_MCST_INC_RPLY_BSY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_MCST_INC_RPLY_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "busy",
    "The bit is used  as busy  indication of MCST request status (busy status)  by CPU"
    "SBPM asserts this bit on each valid request and de-asserts when request is treated:"
    "1 - request is busy,"
    "0- request is not busy (ready)",
#endif
    SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_MASK,
    0,
    SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_WIDTH,
    SBPM_REGS_MCST_INC_RPLY_BSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_MCST_INC_RPLY_RDY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_MCST_INC_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "ready",
    "The bit is used  as ready indication of MCST request status (ready status)  by CPU."
    "SBPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is READY indication:"
    "1 - request is ready,"
    "0- request is not ready (busy)",
#endif
    SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_MASK,
    0,
    SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_WIDTH,
    SBPM_REGS_MCST_INC_RPLY_RDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "bn",
    "bn",
#endif
    SBPM_REGS_BN_CONNECT_BN_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_BN_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_ACK_REQ
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD =
{
    "ACK_REQ",
#if RU_INCLUDE_DESC
    "ack_req",
    "ack_req for Connect command (should be always set)",
#endif
    SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_WR_REQ
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_WR_REQ_FIELD =
{
    "WR_REQ",
#if RU_INCLUDE_DESC
    "wr_req",
    "Used for Direct Write (for work arround)",
#endif
    SBPM_REGS_BN_CONNECT_WR_REQ_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_WR_REQ_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_WR_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_POINTED_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD =
{
    "POINTED_BN",
#if RU_INCLUDE_DESC
    "pointed_bn",
    "pointed_bn",
#endif
    SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_CONNECT_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD =
{
    "CONNECT_ACK",
#if RU_INCLUDE_DESC
    "connect_ack",
    "Acknowledge reply on Connect request",
#endif
    SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_RPLY_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_RPLY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_CONNECT_RPLY_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_RPLY_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_RPLY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_RPLY_BUSY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "busy",
    "busy bit",
#endif
    SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_CONNECT_RPLY_RDY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "rdy",
    "ready bit",
#endif
    SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_MASK,
    0,
    SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_WIDTH,
    SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_BN_FIELD =
{
    "BN",
#if RU_INCLUDE_DESC
    "bufer_number",
    "Get Next Buffer of current BN (used in this field)",
#endif
    SBPM_REGS_GET_NEXT_BN_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_BN_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_GET_NEXT_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_RPLY_BN_VALID
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD =
{
    "BN_VALID",
#if RU_INCLUDE_DESC
    "bn_valid",
    "Used for validation of Next BN reply",
#endif
    SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_RPLY_NEXT_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD =
{
    "NEXT_BN",
#if RU_INCLUDE_DESC
    "next_bn",
    "Next BN - reply of Get_next command",
#endif
    SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_RPLY_BN_NULL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD =
{
    "BN_NULL",
#if RU_INCLUDE_DESC
    "bn_null",
    "Next BN is null indication",
#endif
    SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD =
{
    "MCNT_VAL",
#if RU_INCLUDE_DESC
    "mcnt_val",
    "mcst cnt val",
#endif
    SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_RPLY_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_GET_NEXT_RPLY_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_RPLY_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_RPLY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_RPLY_BUSY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "busy",
    "Get Next command is busy",
#endif
    SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_GET_NEXT_RPLY_RDY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "rdy",
    "Get Next command is ready",
#endif
    SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_MASK,
    0,
    SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_WIDTH,
    SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    "",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD =
{
    "KEEP_ALIVE_INTERVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_WIDTH,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD =
{
    "HEAD_BN",
#if RU_INCLUDE_DESC
    "head_bn",
    "Head BN = First BN in packet that is going to be freed",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD =
{
    "SA",
#if RU_INCLUDE_DESC
    "source_address",
    "source address used for command (may be used for performing command on behalf another port)",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD =
{
    "ACK_REQ",
#if RU_INCLUDE_DESC
    "ack_req",
    "ACK request - should be always set",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD =
{
    "FREE_ACK",
#if RU_INCLUDE_DESC
    "free_ack",
    "Acknowledge on Free command",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD =
{
    "ACK_STAT",
#if RU_INCLUDE_DESC
    "ack_stat",
    "ACK status of CPU",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD =
{
    "NACK_STAT",
#if RU_INCLUDE_DESC
    "nack_stat",
    "NACK status of CPU",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD =
{
    "EXCL_HIGH_STAT",
#if RU_INCLUDE_DESC
    "excl_high_stat",
    "Exclusive_high status of CPU",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD =
{
    "EXCL_LOW_STAT",
#if RU_INCLUDE_DESC
    "excl_low_stat",
    "Exclusive_low status of CPU",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD =
{
    "BSY",
#if RU_INCLUDE_DESC
    "bsy",
    "Busy bit of command (command is currently in execution)",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "rdy",
    "Ready bit of command (ready for new command execution)",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD =
{
    "FREE_ACK",
#if RU_INCLUDE_DESC
    "free_ack",
    "Free command acknowledge",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD =
{
    "ACK_STATE",
#if RU_INCLUDE_DESC
    "ack_state",
    "ACK status of CPU",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD =
{
    "NACK_STATE",
#if RU_INCLUDE_DESC
    "nack_state",
    "NACK status of CPU",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD =
{
    "EXCL_HIGH_STATE",
#if RU_INCLUDE_DESC
    "excl_high_state",
    "Exclusive high status of CPU",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD =
{
    "EXCL_LOW_STATE",
#if RU_INCLUDE_DESC
    "excl_low_state",
    "Exclusive low status of CPU",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD =
{
    "BUSY",
#if RU_INCLUDE_DESC
    "busy",
    "Busy bit of command",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "rdy",
    "Ready bit of command",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD_MASK,
    0,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD_WIDTH,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_GL_TRSH_GL_BAT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD =
{
    "GL_BAT",
#if RU_INCLUDE_DESC
    "GL_BAT",
    "Global Threshold for Allocated BN = maximal total number of BNs in SBPM",
#endif
    SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD_WIDTH,
    SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_GL_TRSH_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_GL_TRSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_GL_TRSH_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_GL_TRSH_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_GL_TRSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_GL_TRSH_GL_BAH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD =
{
    "GL_BAH",
#if RU_INCLUDE_DESC
    "GL_BAH",
    "Global Hysteresis for Allocated BN = hysteresis value related to maximal total threshold of SRAM BNs",
#endif
    SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD_WIDTH,
    SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_GL_TRSH_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_GL_TRSH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_GL_TRSH_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_GL_TRSH_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_GL_TRSH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_TRSH_UG_BAT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD =
{
    "UG_BAT",
#if RU_INCLUDE_DESC
    "UG_BAT",
    "Current UG Threshold for Allocated BN",
#endif
    SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_TRSH_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_TRSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG0_TRSH_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_TRSH_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_TRSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_TRSH_UG_BAH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD =
{
    "UG_BAH",
#if RU_INCLUDE_DESC
    "UG_BAH",
    "Current UG hysteresis Threshold for Allocated BN",
#endif
    SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_TRSH_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_TRSH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG0_TRSH_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_TRSH_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_TRSH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_TRSH_UG_BAT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD =
{
    "UG_BAT",
#if RU_INCLUDE_DESC
    "UG_BAT",
    "Current UG Threshold for Allocated BN",
#endif
    SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_TRSH_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_TRSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG1_TRSH_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_TRSH_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_TRSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_TRSH_UG_BAH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD =
{
    "UG_BAH",
#if RU_INCLUDE_DESC
    "UG_BAH",
    "Current UG hysteresis delta Threshold for Allocated BN",
#endif
    SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_TRSH_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_TRSH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG1_TRSH_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_TRSH_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_TRSH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_SELECT_BUS
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD =
{
    "SELECT_BUS",
#if RU_INCLUDE_DESC
    "select",
    "select bus. the bus index should be mentioned in onehot writting:"
    "bus0 = 0001"
    "bus1 = 0010"
    "bus2 = 0100"
    "bus3 = 1000"
    "",
#endif
    SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_DBG_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_BAC_UG0BAC
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD =
{
    "UG0BAC",
#if RU_INCLUDE_DESC
    "UG0_BAC",
    "UG0 counter for allocated BNs",
#endif
    SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_BAC_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_BAC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG0_BAC_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_BAC_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_BAC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_BAC_UG1BAC
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD =
{
    "UG1BAC",
#if RU_INCLUDE_DESC
    "BAC",
    "Baffer Allocated Counter",
#endif
    SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_BAC_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_BAC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG1_BAC_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_BAC_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_BAC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_GL_BAC_BAC
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_GL_BAC_BAC_FIELD =
{
    "BAC",
#if RU_INCLUDE_DESC
    "BAC",
    "Global BN counter",
#endif
    SBPM_REGS_SBPM_GL_BAC_BAC_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_GL_BAC_BAC_FIELD_WIDTH,
    SBPM_REGS_SBPM_GL_BAC_BAC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_GL_BAC_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_GL_BAC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_GL_BAC_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_GL_BAC_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_GL_BAC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "exclusive_high_threshold",
    "exclusive high threshold",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "exclusive_histeresis_threshold",
    "exclusive histeresis threshold",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "exclusive_high_threshold",
    "exclusive high threshold",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "exclusive_histeresis_threshold",
    "exclusive histeresis threshold",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "exclusive_low_threshold",
    "exclusive low threshold",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "exclusive_histeresis_threshold",
    "exclusive histeresis threshold",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD =
{
    "EXCLT",
#if RU_INCLUDE_DESC
    "exclusive_low_threshold",
    "exclusive low threshold",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD =
{
    "EXCLH",
#if RU_INCLUDE_DESC
    "exclusive_histeresis_threshold",
    "exclusive histeresis threshold",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD =
{
    "UG_ACK_STTS",
#if RU_INCLUDE_DESC
    "UG_ACK_STTS",
    "Ack/Nack status per UG."
    "0 - NACK"
    "1 - ACK"
    ""
    "bit [0] in field matches UG0 ACK status,"
    "bit [1] in field matches UG1 ACK status,"
    "bit [2] in field matches UG2 ACK status,"
    "bit [3] in field matches UG3 ACK status,"
    "bit [4] in field matches UG4 ACK status,"
    "bit [5] in field matches UG5 ACK status,"
    "bit [6] in field matches UG6 ACK status,"
    "bit [7] in field matches UG7 ACK status,",
#endif
    SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG_STATUS_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_STATUS_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD =
{
    "UG_EXCL_HIGH_STTS",
#if RU_INCLUDE_DESC
    "UG_EXCL_HIGH_STTS",
    "High EXCL/Non-Excl status per UG."
    "0 - non_exclusive"
    "1 - exclusive"
    "",
#endif
    SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD =
{
    "UG_EXCL_LOW_STTS",
#if RU_INCLUDE_DESC
    "UG_EXCL_LOW_STTS",
    "Low EXCL/Non-Excl status per UG."
    "0 - non_exclusive"
    "1 - exclusive"
    "",
#endif
    SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG_STATUS_RESERVED1_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_STATUS_RESERVED1_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD =
{
    "SEARCH_DEPTH",
#if RU_INCLUDE_DESC
    "search_depth",
    "Depth (or maximal threshold) for search during Free without context",
#endif
    SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD_MASK,
    0,
    SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD_WIDTH,
    SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD =
{
    "MAX_SEARCH_EN",
#if RU_INCLUDE_DESC
    "max_search_en",
    "Enable for max search  during Free without context",
#endif
    SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD_MASK,
    0,
    SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD_WIDTH,
    SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD =
{
    "CHCK_LAST_EN",
#if RU_INCLUDE_DESC
    "chck_last_en",
    "Enable for Last BN checking  during Free with context",
#endif
    SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD_MASK,
    0,
    SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD_WIDTH,
    SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR
 ******************************************************************************/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD =
{
    "FREEZE_IN_ERROR",
#if RU_INCLUDE_DESC
    "freeze_in_error",
    "Freeze Ug/Global counters + mask access to SBPM RAM while in ERROR state",
#endif
    SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD_MASK,
    0,
    SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD_WIDTH,
    SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_ERROR_HANDLING_PARAMS_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_ERROR_HANDLING_PARAMS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_ERROR_HANDLING_PARAMS_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_ERROR_HANDLING_PARAMS_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_ERROR_HANDLING_PARAMS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_IIR_LOW_CMD_SA
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_IIR_LOW_CMD_SA_FIELD =
{
    "CMD_SA",
#if RU_INCLUDE_DESC
    "cmd_sa",
    "Interrupt command source address (latched from BB SA or CPU code)",
#endif
    SBPM_REGS_SBPM_IIR_LOW_CMD_SA_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_IIR_LOW_CMD_SA_FIELD_WIDTH,
    SBPM_REGS_SBPM_IIR_LOW_CMD_SA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_IIR_LOW_CMD_TA
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_IIR_LOW_CMD_TA_FIELD =
{
    "CMD_TA",
#if RU_INCLUDE_DESC
    "cmd_ta",
    "Interrupt command target address (latched from BB TA or CPU request)",
#endif
    SBPM_REGS_SBPM_IIR_LOW_CMD_TA_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_IIR_LOW_CMD_TA_FIELD_WIDTH,
    SBPM_REGS_SBPM_IIR_LOW_CMD_TA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_22TO0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_22TO0_FIELD =
{
    "CMD_DATA_22TO0",
#if RU_INCLUDE_DESC
    "cmd_data_22to0",
    "Interrupt command data lowest 23-bit  (latched from BB data[22:0] or CPU request data)",
#endif
    SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_22TO0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_22TO0_FIELD_WIDTH,
    SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_22TO0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_23TO63
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_23TO63_FIELD =
{
    "CMD_DATA_23TO63",
#if RU_INCLUDE_DESC
    "cmd_data_23to39",
    "Data (bits [63:23], without reserved bits) of the command that caused interrupt",
#endif
    SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_23TO63_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_23TO63_FIELD_WIDTH,
    SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_23TO63_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD =
{
    "ALLOC_SM",
#if RU_INCLUDE_DESC
    "ALLOC_SM",
    "Alloc State Machine"
    "{update, rd_head_cnxt}",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC0_ALLOC_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD =
{
    "CNNCT_SM",
#if RU_INCLUDE_DESC
    "CNNCT_SM",
    "Connect State Machine"
    "{update}",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC0_CNNCT_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD =
{
    "MCINT_SM",
#if RU_INCLUDE_DESC
    "MCINC_SM",
    "Multicast incr State Machine"
    "{read,check,error,update}",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC0_MCINT_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD =
{
    "FREE_W_CNXT_SM",
#if RU_INCLUDE_DESC
    "FREE_W_CNXT_SM",
    "Free w cnxt State Machine"
    "{read,check,update,error}",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC0_FREE_W_CNXT_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD =
{
    "FREE_WO_CNXT_SM",
#if RU_INCLUDE_DESC
    "FREE_WO_CNXT_SM",
    "Free w/o cnxt State Machine"
    "{read,check,update,error}",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC0_FREE_WO_CNXT_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC0_GN_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD =
{
    "GN_SM",
#if RU_INCLUDE_DESC
    "GN_SM",
    "Get next State Machine:"
    "{read,reply}",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC0_GN_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD =
{
    "MULTI_GN_SM",
#if RU_INCLUDE_DESC
    "MULTI_GN_SM",
    "Those are the 4 Multi get next states:"
    "{rd_next,error,rd_last,wait}",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC0_MULTI_GN_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD =
{
    "FREE_LST_HD",
#if RU_INCLUDE_DESC
    "FREE_LST_HEAD",
    "the value of the head of FREE list",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC0_FREE_LST_HD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD =
{
    "IN2E_VALID",
#if RU_INCLUDE_DESC
    "ingress2egress_valid",
    "sbpm_ingress2egress_valid bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_IN2E_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD =
{
    "MULTI_GN_VALID",
#if RU_INCLUDE_DESC
    "multi_get_next_valid",
    "multi_get_next_valid bits",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_MULTI_GN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD =
{
    "UG_ACTIVE",
#if RU_INCLUDE_DESC
    "sbpm_ug_active",
    "sbpm_ug_active 2 bits",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_UG_ACTIVE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD =
{
    "TX_CMD_FULL",
#if RU_INCLUDE_DESC
    "tx_cmd_fifo_full",
    "sbpm_tx_cmd_fifo_full bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_TX_CMD_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD =
{
    "RX_FIFO_POP",
#if RU_INCLUDE_DESC
    "rx_fifo_pop",
    "sbpm_rx_fifo_pop bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_POP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD =
{
    "RAM_INIT_START",
#if RU_INCLUDE_DESC
    "ram_init_start",
    "sbpm_ram_init_start bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_START_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD =
{
    "RAM_INIT_DONE",
#if RU_INCLUDE_DESC
    "ram_init_done",
    "sbpm_ram_init_done bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_RAM_INIT_DONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD =
{
    "RX_FIFO_DATA",
#if RU_INCLUDE_DESC
    "rx_fifo_data_out",
    "RX FIFO Data in pipe",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_RX_FIFO_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD =
{
    "FREE_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_free_rqst_dec",
    "sbpm_free_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_FREE_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD =
{
    "IN2E_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_in2e_rqst_dec",
    "sbpm_in2e_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_IN2E_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD =
{
    "FREE_WO_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_free_wo_cnxt_rqst_dec",
    "sbpm_free_wo_cnxt_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_FREE_WO_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD =
{
    "GET_NXT_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_get_next_rqst_dec",
    "sbpm_get_next_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_GET_NXT_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD =
{
    "MULTI_GET_NXT_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_multi_get_next_rqst_dec",
    "sbpm_multi_get_next_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_MULTI_GET_NXT_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD =
{
    "CNCT_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_cnct_rqst_dec",
    "sbpm_cnct_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_CNCT_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD =
{
    "FREE_W_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_free_w_cnxt_rqst_dec",
    "sbpm_free_w_cnxt_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_FREE_W_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD =
{
    "MCIN_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_mcinc_rqst_dec",
    "sbpm_mcinc_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_MCIN_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD =
{
    "ALLOC_DECODE",
#if RU_INCLUDE_DESC
    "sbpm_alloc_rqst_dec",
    "sbpm_alloc_rqst_dec",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_ALLOC_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC1_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC1_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD =
{
    "TX_DATA_FULL",
#if RU_INCLUDE_DESC
    "sbpm_tx_data_fifo_full",
    "sbpm_tx_data_fifo_full",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_TX_DATA_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD =
{
    "TX_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "tx_fifo_empty",
    "sbpm_tx_fifo_empty",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_TX_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD =
{
    "LCL_STTS_FULL",
#if RU_INCLUDE_DESC
    "sbpm_tx_cmd_local_stts_fifo_full",
    "sbpm_tx_cmd_local_stts_fifo_full",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD =
{
    "LCL_STTS_EMPTY",
#if RU_INCLUDE_DESC
    "sbpm_tx_cmd_local_stts_fifo_empty",
    "sbpm_tx_cmd_local_stts_fifo_empty",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_LCL_STTS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD =
{
    "TX_CMD_FULL",
#if RU_INCLUDE_DESC
    "sbpm_tx_cmd_fifo_full",
    "sbpm_tx_cmd_fifo_full",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD =
{
    "TX_CMD_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "tx_cmd_fifo_empty",
    "sbpm_tx_cmd_fifo_empty",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_TX_CMD_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD =
{
    "BB_DECODER_DEST_ID",
#if RU_INCLUDE_DESC
    "bb_decoder_dest_id",
    "bb_decoder_dest_id"
    "This is the ID of the user that will recieve a message from SBPM",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_BB_DECODER_DEST_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD =
{
    "TX_BBH_SEND_IN_PROGRESS",
#if RU_INCLUDE_DESC
    "tx_bbh_send_in_progress",
    "sbpm_tx_bbh_send_in_progress bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_TX_BBH_SEND_IN_PROGRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD =
{
    "SP_2SEND",
#if RU_INCLUDE_DESC
    "sp_2send",
    "sbpm_sp_2send - this is the user ID that is about to get stts msg",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_SP_2SEND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD =
{
    "TX2DATA_FIFO_TADDR",
#if RU_INCLUDE_DESC
    "tx2data_fifo_taddr",
    "sbpm_tx2data_fifo_taddr[2:0] this is the opcode that describe the type of the reply",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_TX2DATA_FIFO_TADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD =
{
    "CPU_ACCESS",
#if RU_INCLUDE_DESC
    "cpu_access",
    "sbpm_cpu_access bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_CPU_ACCESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD =
{
    "BBH_ACCESS",
#if RU_INCLUDE_DESC
    "bbh_access",
    "sbpm_bbh_access bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_BBH_ACCESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD =
{
    "RNR_ACCESS",
#if RU_INCLUDE_DESC
    "rnr_access",
    "sbpm_rnr_access bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_RNR_ACCESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC2_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC2_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD =
{
    "ALLOC_RPLY",
#if RU_INCLUDE_DESC
    "alloc_rply",
    "ALLOC_RPLY bit",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_ALLOC_RPLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD =
{
    "BN_RPLY",
#if RU_INCLUDE_DESC
    "bn_rply",
    "BN_RPLY value",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_BN_RPLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD =
{
    "TXFIFO_ALLOC_ACK",
#if RU_INCLUDE_DESC
    "sbpm_txfifo_alloc_ack",
    "sbpm_txfifo_alloc_ack",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_ALLOC_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD =
{
    "TX_FIFO_MCINC_ACK",
#if RU_INCLUDE_DESC
    "sbpm_txfifo_mcinc_ack",
    "sbpm_txfifo_mcinc_ack",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_TX_FIFO_MCINC_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD =
{
    "TXFIFO_CNCT_ACK",
#if RU_INCLUDE_DESC
    "sbpm_txfifo_cnct_ack",
    "sbpm_txfifo_cnct_ack",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_CNCT_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD =
{
    "TXFIFO_GT_NXT_RPLY",
#if RU_INCLUDE_DESC
    "sbpm_txfifo_get_next_reply",
    "sbpm_txfifo_get_next_reply",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_GT_NXT_RPLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD =
{
    "TXFIFO_MLTI_GT_NXT_RPLY",
#if RU_INCLUDE_DESC
    "sbpm_txfifo_multi_get_next_reply",
    "sbpm_txfifo_multi_get_next_reply",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_MLTI_GT_NXT_RPLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD =
{
    "TX_MSG_PIPE_SM",
#if RU_INCLUDE_DESC
    "sbpm_tx_msg_pipe_cur_sm",
    "sbpm_tx_msg_pipe_cur_sm",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_TX_MSG_PIPE_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD =
{
    "SEND_STT_SM",
#if RU_INCLUDE_DESC
    "sbpm_send_stat_sm_ps",
    "sbpm_send_stat_sm_ps",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_SEND_STT_SM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD =
{
    "TXFIFO_IN2ESTTS_CHNG",
#if RU_INCLUDE_DESC
    "sbpm_txfifo_ingress2egress_stts_change",
    "sbpm_txfifo_ingress2egress_stts_change",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_TXFIFO_IN2ESTTS_CHNG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_DBG_VEC3_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_DBG_VEC3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_DBG_VEC3_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_DBG_VEC3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD =
{
    "SBPM_SP_BBH_LOW",
#if RU_INCLUDE_DESC
    "sbpm_sp_bbh_low",
    "sbpm_sp_bbh_low bit i tells us if SP #i is a BBH (1) or not (0)",
#endif
    SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD_WIDTH,
    SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD =
{
    "SBPM_SP_BBH_HIGH",
#if RU_INCLUDE_DESC
    "sbpm_sp_bbh_high",
    "Not in use in 68360!"
    "sbpm_sp_bbh_high bit i tells us if SP #i is a BBH (1) or not (0)",
#endif
    SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD_WIDTH,
    SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD =
{
    "SBPM_SP_RNR_LOW",
#if RU_INCLUDE_DESC
    "sbpm_sp_rnr_low",
    "sbpm_sp_rnr_low bit i tells us if SP #i is a runner (1) or not (0)",
#endif
    SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD_WIDTH,
    SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD =
{
    "SBPM_SP_RNR_HIGH",
#if RU_INCLUDE_DESC
    "sbpm_sp_rnr_high",
    "Not in use in 68360!"
    "sbpm_sp_rnr_high bit i tells us if SP #i is a runner (1) or not (0)",
#endif
    SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD_WIDTH,
    SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD =
{
    "SBPM_UG_MAP_LOW",
#if RU_INCLUDE_DESC
    "sbpm_ug_map_low",
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)",
#endif
    SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD =
{
    "SBPM_UG_MAP_HIGH",
#if RU_INCLUDE_DESC
    "sbpm_ug_map_high",
    "Not in use in 68360!"
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)",
#endif
    SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD =
{
    "SBPM_NACK_MASK_LOW",
#if RU_INCLUDE_DESC
    "sbpm_nack_mask_low",
    "bit i value determine if SP number i got nack or not",
#endif
    SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD_WIDTH,
    SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD =
{
    "SBPM_NACK_MASK_HIGH",
#if RU_INCLUDE_DESC
    "sbpm_nack_mask_high",
    "bit i value determine if SP number i got nack or not",
#endif
    SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD_WIDTH,
    SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD =
{
    "SBPM_EXCL_MASK_LOW",
#if RU_INCLUDE_DESC
    "sbpm_excl_mask_low",
    "This register mark all the SPs that should get exclusive messages"
    "yes no",
#endif
    SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD_WIDTH,
    SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD =
{
    "SBPM_EXCL_MASK_HIGH",
#if RU_INCLUDE_DESC
    "sbpm_excl_mask_high",
    "Not in use in 68360!"
    "This register mark all the SPs that should get exclusive messages"
    "yes no",
#endif
    SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD_WIDTH,
    SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD =
{
    "ID_2OVERWR",
#if RU_INCLUDE_DESC
    "dest_id_to_overwr",
    "this field contains the users id that you want to override its default RA",
#endif
    SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD_WIDTH,
    SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD =
{
    "OVERWR_RA",
#if RU_INCLUDE_DESC
    "overwr_route_addr",
    "The new RA",
#endif
    SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD_WIDTH,
    SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD =
{
    "OVERWR_VALID",
#if RU_INCLUDE_DESC
    "overwr_valid",
    "the overwr mechanism will be used only if this bit is active (1).",
#endif
    SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD_WIDTH,
    SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_RADDR_DECODER_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_RADDR_DECODER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_RADDR_DECODER_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_RADDR_DECODER_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_RADDR_DECODER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD =
{
    "SBPM_WR_DATA",
#if RU_INCLUDE_DESC
    "sbpm_wr_data",
    "If SW want to write a whole word into the SBPMs RAM, it needs first to write the data to this register and then, send connect request with the wr_req bit asserted, with the address (BN field)."
    ""
    "In 68360 the only the 15 LSB are used",
#endif
    SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD_WIDTH,
    SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_WR_DATA_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_WR_DATA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_WR_DATA_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_WR_DATA_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_WR_DATA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD =
{
    "UG0BACMAX",
#if RU_INCLUDE_DESC
    "UG0_BAC_MAX",
    "This is the maximum value that have been recorded on the UG0 counter."
    "SW can write to this field in order to change the max record (for example write 0 to reset it)",
#endif
    SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD =
{
    "UG1BACMAX",
#if RU_INCLUDE_DESC
    "UG1_BAC_MAX",
    "This is the maximum value that have been recorded on the UG1 counter."
    "SW can write to this field in order to change the max record (for example write 0 to reset it)",
#endif
    SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_UG_BAC_MAX_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_UG_BAC_MAX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_UG_BAC_MAX_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_UG_BAC_MAX_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_UG_BAC_MAX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD =
{
    "GL_BAC_CLEAR_EN",
#if RU_INCLUDE_DESC
    "sbpm_gl_bac_clear_en",
    "sbpm_gl_bac_clear_en",
#endif
    SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD_WIDTH,
    SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_REGS_SBPM_SPARE_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_REGS_SBPM_SPARE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_REGS_SBPM_SPARE_RESERVED0_FIELD_MASK,
    0,
    SBPM_REGS_SBPM_SPARE_RESERVED0_FIELD_WIDTH,
    SBPM_REGS_SBPM_SPARE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_BAC_UNDERRUN
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD =
{
    "BAC_UNDERRUN",
#if RU_INCLUDE_DESC
    "bac_underrun",
    "This error bit indicates underrun state of SBPM Buffer Allocated Counter (one of User Groups). SW can clear this bit by writing 1 to this field",
#endif
    SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_BAC_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_MCST_OVERFLOW
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD =
{
    "MCST_OVERFLOW",
#if RU_INCLUDE_DESC
    "mcst_overflow",
    "This error bit indicates if the Multi Cast value of a buffer is in overflow as a result of erroneous MCINC command",
#endif
    SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_MCST_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD =
{
    "CHECK_LAST_ERR",
#if RU_INCLUDE_DESC
    "check_last_err",
    "This bit indicates error state on Last BN checking during Free with context request. SW can clear this bit by writing 1 to this field.",
#endif
    SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_CHECK_LAST_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD =
{
    "MAX_SEARCH_ERR",
#if RU_INCLUDE_DESC
    "max_search_err",
    "This bit indicates error state on maximal search checking during Free without context request. SW can clear this bit by writing 1 to this field.",
#endif
    SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_MAX_SEARCH_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_INVALID_IN2E
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD =
{
    "INVALID_IN2E",
#if RU_INCLUDE_DESC
    "invalid_in2e",
    "This bit indicates invalid ingress2egress command (caused BAC under/overrun). SW can clear this bit by writing 1 to this field.",
#endif
    SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_INVALID_IN2E_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD =
{
    "MULTI_GET_NEXT_NULL",
#if RU_INCLUDE_DESC
    "multi_get_next_null",
    "This bit indicates Null encounter during one of the next BNs. SW can clear this bit by writing 0 to this field.",
#endif
    SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_MULTI_GET_NEXT_NULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_CNCT_NULL
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD =
{
    "CNCT_NULL",
#if RU_INCLUDE_DESC
    "cnct_null",
    "This bit indicates connection of the NULL buffer to another buufer. SW can clear this bit by writing 0 to this field.",
#endif
    SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_CNCT_NULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_ALLOC_NULL
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD =
{
    "ALLOC_NULL",
#if RU_INCLUDE_DESC
    "alloc_null",
    "This bit indicates allocation of the NULL buffer. SW can clear this bit by writing 0 to this field.",
#endif
    SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_ALLOC_NULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISR_RESERVED0
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SBPM_INTR_CTRL_ISR_RESERVED0_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISR_RESERVED0_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ISM_ISM
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "Interrupt_status_masked",
    "Status Masked of corresponding interrupt source in the ISR",
#endif
    SBPM_INTR_CTRL_ISM_ISM_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ISM_ISM_FIELD_WIDTH,
    SBPM_INTR_CTRL_ISM_ISM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_IER_IEM
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "Interrupt_enable_mask",
    "Each bit in the mask controls the corresponding interrupt source in the IER",
#endif
    SBPM_INTR_CTRL_IER_IEM_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_IER_IEM_FIELD_WIDTH,
    SBPM_INTR_CTRL_IER_IEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SBPM_INTR_CTRL_ITR_IST
 ******************************************************************************/
const ru_field_rec SBPM_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "Interrupt_simulation_test",
    "Each bit in the mask tests the corresponding interrupt source in the ISR",
#endif
    SBPM_INTR_CTRL_ITR_IST_FIELD_MASK,
    0,
    SBPM_INTR_CTRL_ITR_IST_FIELD_WIDTH,
    SBPM_INTR_CTRL_ITR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: SBPM_REGS_INIT_FREE_LIST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_INIT_FREE_LIST_FIELDS[] =
{
    &SBPM_REGS_INIT_FREE_LIST_INIT_BASE_ADDR_FIELD,
    &SBPM_REGS_INIT_FREE_LIST_INIT_OFFSET_FIELD,
    &SBPM_REGS_INIT_FREE_LIST_RESERVED0_FIELD,
    &SBPM_REGS_INIT_FREE_LIST_BSY_FIELD,
    &SBPM_REGS_INIT_FREE_LIST_RDY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_INIT_FREE_LIST_REG = 
{
    "REGS_INIT_FREE_LIST",
#if RU_INCLUDE_DESC
    "INIT_FREE_LIST Register",
    "request for building the free list using HW accelerator",
#endif
    SBPM_REGS_INIT_FREE_LIST_REG_OFFSET,
    0,
    0,
    667,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    SBPM_REGS_INIT_FREE_LIST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_ALLOC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_ALLOC_FIELDS[] =
{
    &SBPM_REGS_BN_ALLOC_RESERVED0_FIELD,
    &SBPM_REGS_BN_ALLOC_SA_FIELD,
    &SBPM_REGS_BN_ALLOC_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_ALLOC_REG = 
{
    "REGS_BN_ALLOC",
#if RU_INCLUDE_DESC
    "BN_ALLOC Register",
    "request for a new buffer",
#endif
    SBPM_REGS_BN_ALLOC_REG_OFFSET,
    0,
    0,
    668,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_BN_ALLOC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_ALLOC_RPLY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_ALLOC_RPLY_FIELDS[] =
{
    &SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_VALID_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_ALLOC_BN_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_ACK_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_NACK_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_EXCL_HIGH_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_EXCL_LOW_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_RESERVED0_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_BUSY_FIELD,
    &SBPM_REGS_BN_ALLOC_RPLY_RDY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_ALLOC_RPLY_REG = 
{
    "REGS_BN_ALLOC_RPLY",
#if RU_INCLUDE_DESC
    "BN_ALLOC_RPLY Register",
    "reply for a new buffer alloc",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_REG_OFFSET,
    0,
    0,
    669,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    SBPM_REGS_BN_ALLOC_RPLY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITH_CONTXT_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_HEAD_BN_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_SA_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_RESERVED0_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_OFFSET_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_ACK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_REG = 
{
    "REGS_BN_FREE_WITH_CONTXT_LOW",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_LOW Register",
    "Request for freeing buffers of a packet offline with context (lower 32-bit)",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_REG_OFFSET,
    0,
    0,
    670,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    SBPM_REGS_BN_FREE_WITH_CONTXT_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_LAST_BN_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_REG = 
{
    "REGS_BN_FREE_WITH_CONTXT_HIGH",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_HIGH Register",
    "Request for freeing buffers of a packet offline with context (higher 32-bit)",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_REG_OFFSET,
    0,
    0,
    671,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_BN_FREE_WITH_CONTXT_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_MCST_INC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_MCST_INC_FIELDS[] =
{
    &SBPM_REGS_MCST_INC_BN_FIELD,
    &SBPM_REGS_MCST_INC_MCST_VAL_FIELD,
    &SBPM_REGS_MCST_INC_ACK_REQ_FIELD,
    &SBPM_REGS_MCST_INC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_MCST_INC_REG = 
{
    "REGS_MCST_INC",
#if RU_INCLUDE_DESC
    "MCST_INC Register",
    "Multicast counter increment. Contains the BN, which is head of the packet to be multicast and its counter value",
#endif
    SBPM_REGS_MCST_INC_REG_OFFSET,
    0,
    0,
    672,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_MCST_INC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_MCST_INC_RPLY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_MCST_INC_RPLY_FIELDS[] =
{
    &SBPM_REGS_MCST_INC_RPLY_MCST_ACK_FIELD,
    &SBPM_REGS_MCST_INC_RPLY_RESERVED0_FIELD,
    &SBPM_REGS_MCST_INC_RPLY_BSY_FIELD,
    &SBPM_REGS_MCST_INC_RPLY_RDY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_MCST_INC_RPLY_REG = 
{
    "REGS_MCST_INC_RPLY",
#if RU_INCLUDE_DESC
    "MCST_INC_RPLY Register",
    "mcst_inc_rply",
#endif
    SBPM_REGS_MCST_INC_RPLY_REG_OFFSET,
    0,
    0,
    673,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_MCST_INC_RPLY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_CONNECT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_CONNECT_FIELDS[] =
{
    &SBPM_REGS_BN_CONNECT_BN_FIELD,
    &SBPM_REGS_BN_CONNECT_ACK_REQ_FIELD,
    &SBPM_REGS_BN_CONNECT_WR_REQ_FIELD,
    &SBPM_REGS_BN_CONNECT_POINTED_BN_FIELD,
    &SBPM_REGS_BN_CONNECT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_CONNECT_REG = 
{
    "REGS_BN_CONNECT",
#if RU_INCLUDE_DESC
    "BN_CONNECT Register",
    "request for connection between two buffers in a linked list. The connection request may be replied with ACK message if the ACK request bit is asserted."
    "This command is used as write command.",
#endif
    SBPM_REGS_BN_CONNECT_REG_OFFSET,
    0,
    0,
    674,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    SBPM_REGS_BN_CONNECT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_CONNECT_RPLY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_CONNECT_RPLY_FIELDS[] =
{
    &SBPM_REGS_BN_CONNECT_RPLY_CONNECT_ACK_FIELD,
    &SBPM_REGS_BN_CONNECT_RPLY_RESERVED0_FIELD,
    &SBPM_REGS_BN_CONNECT_RPLY_BUSY_FIELD,
    &SBPM_REGS_BN_CONNECT_RPLY_RDY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_CONNECT_RPLY_REG = 
{
    "REGS_BN_CONNECT_RPLY",
#if RU_INCLUDE_DESC
    "BN_CONNECT_RPLY Register",
    "bn_connect_rply",
#endif
    SBPM_REGS_BN_CONNECT_RPLY_REG_OFFSET,
    0,
    0,
    675,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_BN_CONNECT_RPLY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_GET_NEXT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_GET_NEXT_FIELDS[] =
{
    &SBPM_REGS_GET_NEXT_BN_FIELD,
    &SBPM_REGS_GET_NEXT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_GET_NEXT_REG = 
{
    "REGS_GET_NEXT",
#if RU_INCLUDE_DESC
    "GET_NEXT Register",
    "a pointer to a buffer in a packet linked list and request for the next buffer in the list"
    "this command is used as read command.",
#endif
    SBPM_REGS_GET_NEXT_REG_OFFSET,
    0,
    0,
    676,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_GET_NEXT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_GET_NEXT_RPLY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_GET_NEXT_RPLY_FIELDS[] =
{
    &SBPM_REGS_GET_NEXT_RPLY_BN_VALID_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_NEXT_BN_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_BN_NULL_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_MCNT_VAL_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_RESERVED0_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_BUSY_FIELD,
    &SBPM_REGS_GET_NEXT_RPLY_RDY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_GET_NEXT_RPLY_REG = 
{
    "REGS_GET_NEXT_RPLY",
#if RU_INCLUDE_DESC
    "GET_NEXT_RPLY Register",
    "get_next_rply",
#endif
    SBPM_REGS_GET_NEXT_RPLY_REG_OFFSET,
    0,
    0,
    677,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    SBPM_REGS_GET_NEXT_RPLY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_CLK_GATE_CNTRL_FIELDS[] =
{
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTERVL_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &SBPM_REGS_SBPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG = 
{
    "REGS_SBPM_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "SBPM_CLK_GATE_CNTRL Register",
    "control for the bl_clk_control module",
#endif
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    678,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    SBPM_REGS_SBPM_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITHOUT_CONTXT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITHOUT_CONTXT_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_HEAD_BN_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_SA_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RESERVED0_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_ACK_REQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG = 
{
    "REGS_BN_FREE_WITHOUT_CONTXT",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT Register",
    "bn_free_without_contxt",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG_OFFSET,
    0,
    0,
    679,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FREE_ACK_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED0_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_ACK_STAT_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_NACK_STAT_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_HIGH_STAT_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_EXCL_LOW_STAT_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RESERVED1_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_BSY_FIELD,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_RDY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG = 
{
    "REGS_BN_FREE_WITHOUT_CONTXT_RPLY",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT_RPLY Register",
    "bn_free_without_contxt_rply",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG_OFFSET,
    0,
    0,
    680,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FIELDS[] =
{
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FREE_ACK_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED0_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_ACK_STATE_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_NACK_STATE_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_HIGH_STATE_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_EXCL_LOW_STATE_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RESERVED1_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_BUSY_FIELD,
    &SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_RDY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_REG = 
{
    "REGS_BN_FREE_WITH_CONTXT_RPLY",
#if RU_INCLUDE_DESC
    "BN_FREE_WITH_CONTXT_RPLY Register",
    "bn_free_with_contxt_rply",
#endif
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_REG_OFFSET,
    0,
    0,
    681,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    SBPM_REGS_BN_FREE_WITH_CONTXT_RPLY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_GL_TRSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_GL_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_GL_TRSH_GL_BAT_FIELD,
    &SBPM_REGS_SBPM_GL_TRSH_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_GL_TRSH_GL_BAH_FIELD,
    &SBPM_REGS_SBPM_GL_TRSH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_GL_TRSH_REG = 
{
    "REGS_SBPM_GL_TRSH",
#if RU_INCLUDE_DESC
    "GLOBAL_THRESHOLD Register",
    "Global Threshold for Allocated Buffers."
    "SBPM will issue BN in the accepted range upon to Global threshold setup."
    "Ths register also holds global hysteresis value for ACK/NACK transition setting. We cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_GL_TRSH_REG_OFFSET,
    0,
    0,
    682,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_SBPM_GL_TRSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG0_TRSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG0_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG0_TRSH_UG_BAT_FIELD,
    &SBPM_REGS_SBPM_UG0_TRSH_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_UG0_TRSH_UG_BAH_FIELD,
    &SBPM_REGS_SBPM_UG0_TRSH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG0_TRSH_REG = 
{
    "REGS_SBPM_UG0_TRSH",
#if RU_INCLUDE_DESC
    "UG0_THRESHOLD Register",
    "Threshold for Allocated Buffers of UG0"
    "Ths register also holds UG0 hysteresis value for ACK/NACK transition setting."
    "We cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG0_TRSH_REG_OFFSET,
    0,
    0,
    683,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_SBPM_UG0_TRSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG1_TRSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG1_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG1_TRSH_UG_BAT_FIELD,
    &SBPM_REGS_SBPM_UG1_TRSH_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_UG1_TRSH_UG_BAH_FIELD,
    &SBPM_REGS_SBPM_UG1_TRSH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG1_TRSH_REG = 
{
    "REGS_SBPM_UG1_TRSH",
#if RU_INCLUDE_DESC
    "UG1_THRESHOLD Register",
    "Threshold for Allocated Buffers of UG1"
    "Ths register also holds UG1 hysteresis value for ACK/NACK transition setting."
    "We cross to Nack state if BAC equals the threshold. We cross down to Ack if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG1_TRSH_REG_OFFSET,
    0,
    0,
    684,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_SBPM_UG1_TRSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_DBG_FIELDS[] =
{
    &SBPM_REGS_SBPM_DBG_SELECT_BUS_FIELD,
    &SBPM_REGS_SBPM_DBG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_DBG_REG = 
{
    "REGS_SBPM_DBG",
#if RU_INCLUDE_DESC
    "SBPM_DBG Register",
    "SBPM select the debug bus",
#endif
    SBPM_REGS_SBPM_DBG_REG_OFFSET,
    0,
    0,
    685,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_DBG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG0_BAC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG0_BAC_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG0_BAC_UG0BAC_FIELD,
    &SBPM_REGS_SBPM_UG0_BAC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG0_BAC_REG = 
{
    "REGS_SBPM_UG0_BAC",
#if RU_INCLUDE_DESC
    "SBPM_UG0_BAC Register",
    "SBPM UG0 allocated BN counter",
#endif
    SBPM_REGS_SBPM_UG0_BAC_REG_OFFSET,
    0,
    0,
    686,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG0_BAC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG1_BAC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG1_BAC_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG1_BAC_UG1BAC_FIELD,
    &SBPM_REGS_SBPM_UG1_BAC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG1_BAC_REG = 
{
    "REGS_SBPM_UG1_BAC",
#if RU_INCLUDE_DESC
    "SBPM_UG1_BAC Register",
    "SBPM UG1 allocated BN Counter",
#endif
    SBPM_REGS_SBPM_UG1_BAC_REG_OFFSET,
    0,
    0,
    687,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_UG1_BAC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_GL_BAC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_GL_BAC_FIELDS[] =
{
    &SBPM_REGS_SBPM_GL_BAC_BAC_FIELD,
    &SBPM_REGS_SBPM_GL_BAC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_GL_BAC_REG = 
{
    "REGS_SBPM_GL_BAC",
#if RU_INCLUDE_DESC
    "SBPM_GL_BAC Register",
    "SBPM global BN Counter",
#endif
    SBPM_REGS_SBPM_GL_BAC_REG_OFFSET,
    0,
    0,
    688,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_GL_BAC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLT_FIELD,
    &SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_EXCLH_FIELD,
    &SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_REG = 
{
    "REGS_SBPM_UG0_EXCL_HIGH_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG0_EXCLUSIVE_HIGH_THRESHOLD Register",
    "SBPM UG0 Exclusive high and hysteresis threshold."
    "We cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_REG_OFFSET,
    0,
    0,
    689,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_SBPM_UG0_EXCL_HIGH_TRSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLT_FIELD,
    &SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_EXCLH_FIELD,
    &SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_REG = 
{
    "REGS_SBPM_UG1_EXCL_HIGH_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG1_EXCLUSIVE_HIGH_THRESHOLD Register",
    "SBPM UG1 Exclusive high and hysteresis threshold."
    "We cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_REG_OFFSET,
    0,
    0,
    690,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_SBPM_UG1_EXCL_HIGH_TRSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLT_FIELD,
    &SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_EXCLH_FIELD,
    &SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_REG = 
{
    "REGS_SBPM_UG0_EXCL_LOW_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG0_EXCLUSIVE_LOW_THRESHOLD Register",
    "SBPM UG0 Exclusive low and hysteresis threshold."
    "We cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_REG_OFFSET,
    0,
    0,
    691,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_SBPM_UG0_EXCL_LOW_TRSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLT_FIELD,
    &SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_EXCLH_FIELD,
    &SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_REG = 
{
    "REGS_SBPM_UG1_EXCL_LOW_TRSH",
#if RU_INCLUDE_DESC
    "SBPM_UG1_EXCLUSIVE_LOW_THRESHOLD Register",
    "SBPM UG1 Exclusive low and hysteresis threshold."
    "We cross to Excl state if BAC equals the threshold. We cross down to not Excl if BAC equals the thrshold minus the histeresis value.",
#endif
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_REG_OFFSET,
    0,
    0,
    692,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_SBPM_UG1_EXCL_LOW_TRSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG_STATUS_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG_STATUS_UG_ACK_STTS_FIELD,
    &SBPM_REGS_SBPM_UG_STATUS_RESERVED0_FIELD,
    &SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_HIGH_STTS_FIELD,
    &SBPM_REGS_SBPM_UG_STATUS_UG_EXCL_LOW_STTS_FIELD,
    &SBPM_REGS_SBPM_UG_STATUS_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG_STATUS_REG = 
{
    "REGS_SBPM_UG_STATUS",
#if RU_INCLUDE_DESC
    "USER_GROUP_STATUS_REGISTER Register",
    "This register is status set of all 8 Ugs: Ack/NACK state and in addition Exclusive state pereach of 8 UGs",
#endif
    SBPM_REGS_SBPM_UG_STATUS_REG_OFFSET,
    0,
    0,
    693,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    SBPM_REGS_SBPM_UG_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_ERROR_HANDLING_PARAMS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_ERROR_HANDLING_PARAMS_FIELDS[] =
{
    &SBPM_REGS_ERROR_HANDLING_PARAMS_SEARCH_DEPTH_FIELD,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_MAX_SEARCH_EN_FIELD,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_CHCK_LAST_EN_FIELD,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_FREEZE_IN_ERROR_FIELD,
    &SBPM_REGS_ERROR_HANDLING_PARAMS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_ERROR_HANDLING_PARAMS_REG = 
{
    "REGS_ERROR_HANDLING_PARAMS",
#if RU_INCLUDE_DESC
    "ERROR_HANDLING_PARAMS Register",
    "Parameters and thresholds used for Error handling: error detection, max search enable and threshold, etc.",
#endif
    SBPM_REGS_ERROR_HANDLING_PARAMS_REG_OFFSET,
    0,
    0,
    694,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    SBPM_REGS_ERROR_HANDLING_PARAMS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_IIR_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_IIR_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_IIR_LOW_CMD_SA_FIELD,
    &SBPM_REGS_SBPM_IIR_LOW_CMD_TA_FIELD,
    &SBPM_REGS_SBPM_IIR_LOW_CMD_DATA_22TO0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_IIR_LOW_REG = 
{
    "REGS_SBPM_IIR_LOW",
#if RU_INCLUDE_DESC
    "SBPM_IIR_LOW_REGISTER Register",
    "SBPM IIR low (Interrupt information register)",
#endif
    SBPM_REGS_SBPM_IIR_LOW_REG_OFFSET,
    0,
    0,
    695,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_SBPM_IIR_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_IIR_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_IIR_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_IIR_HIGH_CMD_DATA_23TO63_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_IIR_HIGH_REG = 
{
    "REGS_SBPM_IIR_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_IIR_HIGH_REGISTER Register",
    "SBPM IIR high (Interrupt information register)",
#endif
    SBPM_REGS_SBPM_IIR_HIGH_REG_OFFSET,
    0,
    0,
    696,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_IIR_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG_VEC0
 ******************************************************************************/
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

const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC0_REG = 
{
    "REGS_SBPM_DBG_VEC0",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC0 Register",
    "SBPM debug vector0 includes 21 bit of control/state machine of CMD pipe"
    ""
    "",
#endif
    SBPM_REGS_SBPM_DBG_VEC0_REG_OFFSET,
    0,
    0,
    697,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    SBPM_REGS_SBPM_DBG_VEC0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG_VEC1
 ******************************************************************************/
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
    &SBPM_REGS_SBPM_DBG_VEC1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC1_REG = 
{
    "REGS_SBPM_DBG_VEC1",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC1 Register",
    "SBPM debug vector1 includes 21 bit of control/state machine of CMD pipe"
    ""
    ""
    "",
#endif
    SBPM_REGS_SBPM_DBG_VEC1_REG_OFFSET,
    0,
    0,
    698,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    18,
    SBPM_REGS_SBPM_DBG_VEC1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG_VEC2
 ******************************************************************************/
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
    &SBPM_REGS_SBPM_DBG_VEC2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC2_REG = 
{
    "REGS_SBPM_DBG_VEC2",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC2 Register",
    "This is one of the TX_handler debug vectors",
#endif
    SBPM_REGS_SBPM_DBG_VEC2_REG_OFFSET,
    0,
    0,
    699,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    14,
    SBPM_REGS_SBPM_DBG_VEC2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_DBG_VEC3
 ******************************************************************************/
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
    &SBPM_REGS_SBPM_DBG_VEC3_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_DBG_VEC3_REG = 
{
    "REGS_SBPM_DBG_VEC3",
#if RU_INCLUDE_DESC
    "SBPM_DBG_VEC3 Register",
    "This is one of TX_handler debug vectors",
#endif
    SBPM_REGS_SBPM_DBG_VEC3_REG_OFFSET,
    0,
    0,
    700,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    SBPM_REGS_SBPM_DBG_VEC3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SP_BBH_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SP_BBH_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_SP_BBH_LOW_SBPM_SP_BBH_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_SP_BBH_LOW_REG = 
{
    "REGS_SBPM_SP_BBH_LOW",
#if RU_INCLUDE_DESC
    "SBPM_SP_BBH_LOW Register",
    "This register mark all the SPs which are BBHs."
    "Each bit in this register, refers to a SP with the same index",
#endif
    SBPM_REGS_SBPM_SP_BBH_LOW_REG_OFFSET,
    0,
    0,
    701,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SP_BBH_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SP_BBH_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SP_BBH_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_SP_BBH_HIGH_SBPM_SP_BBH_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_SP_BBH_HIGH_REG = 
{
    "REGS_SBPM_SP_BBH_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_SP_BBH_HIGH Register",
    "This register mark all the SPs which are BBHs."
    "Each bit in this register, refers to a SP with the same index",
#endif
    SBPM_REGS_SBPM_SP_BBH_HIGH_REG_OFFSET,
    0,
    0,
    702,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SP_BBH_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SP_RNR_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SP_RNR_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_SP_RNR_LOW_SBPM_SP_RNR_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_SP_RNR_LOW_REG = 
{
    "REGS_SBPM_SP_RNR_LOW",
#if RU_INCLUDE_DESC
    "SBPM_SP_RNR_LOW Register",
    "This register mark all the SPs which are runners."
    "Each bit in this register, refers to a SP with the same index",
#endif
    SBPM_REGS_SBPM_SP_RNR_LOW_REG_OFFSET,
    0,
    0,
    703,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SP_RNR_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SP_RNR_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SP_RNR_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_SP_RNR_HIGH_SBPM_SP_RNR_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_SP_RNR_HIGH_REG = 
{
    "REGS_SBPM_SP_RNR_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_SP_RNR_HIGH Register",
    "This register mark all the SPs which are runners."
    "Each bit in this register, refers to a SP with the same index",
#endif
    SBPM_REGS_SBPM_SP_RNR_HIGH_REG_OFFSET,
    0,
    0,
    704,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_SP_RNR_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG_MAP_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG_MAP_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG_MAP_LOW_SBPM_UG_MAP_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG_MAP_LOW_REG = 
{
    "REGS_SBPM_UG_MAP_LOW",
#if RU_INCLUDE_DESC
    "SBPM_UG_MAP_LOW Register",
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)",
#endif
    SBPM_REGS_SBPM_UG_MAP_LOW_REG_OFFSET,
    0,
    0,
    705,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_UG_MAP_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG_MAP_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG_MAP_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG_MAP_HIGH_SBPM_UG_MAP_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG_MAP_HIGH_REG = 
{
    "REGS_SBPM_UG_MAP_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_UG_MAP_HIGH Register",
    "bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)",
#endif
    SBPM_REGS_SBPM_UG_MAP_HIGH_REG_OFFSET,
    0,
    0,
    706,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_UG_MAP_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_NACK_MASK_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_NACK_MASK_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_NACK_MASK_LOW_SBPM_NACK_MASK_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_NACK_MASK_LOW_REG = 
{
    "REGS_SBPM_NACK_MASK_LOW",
#if RU_INCLUDE_DESC
    "SBPM_NACK_MASK_LOW Register",
    "bit i value determine if SP number i got nack or not",
#endif
    SBPM_REGS_SBPM_NACK_MASK_LOW_REG_OFFSET,
    0,
    0,
    707,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_NACK_MASK_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_NACK_MASK_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_NACK_MASK_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_NACK_MASK_HIGH_SBPM_NACK_MASK_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_NACK_MASK_HIGH_REG = 
{
    "REGS_SBPM_NACK_MASK_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_NACK_MASK_HIGH Register",
    "bit i value determine if SP number i got nack or not",
#endif
    SBPM_REGS_SBPM_NACK_MASK_HIGH_REG_OFFSET,
    0,
    0,
    708,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_NACK_MASK_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_EXCL_MASK_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_EXCL_MASK_LOW_FIELDS[] =
{
    &SBPM_REGS_SBPM_EXCL_MASK_LOW_SBPM_EXCL_MASK_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_EXCL_MASK_LOW_REG = 
{
    "REGS_SBPM_EXCL_MASK_LOW",
#if RU_INCLUDE_DESC
    "SBPM_EXCL_MASK_LOW Register",
    "This register mark all the SPs that should get exclusive messages",
#endif
    SBPM_REGS_SBPM_EXCL_MASK_LOW_REG_OFFSET,
    0,
    0,
    709,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_EXCL_MASK_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_EXCL_MASK_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_EXCL_MASK_HIGH_FIELDS[] =
{
    &SBPM_REGS_SBPM_EXCL_MASK_HIGH_SBPM_EXCL_MASK_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_EXCL_MASK_HIGH_REG = 
{
    "REGS_SBPM_EXCL_MASK_HIGH",
#if RU_INCLUDE_DESC
    "SBPM_EXCL_MASK_HIGH Register",
    "This register mark all the SPs that should get exclusive messages",
#endif
    SBPM_REGS_SBPM_EXCL_MASK_HIGH_REG_OFFSET,
    0,
    0,
    710,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_REGS_SBPM_EXCL_MASK_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_RADDR_DECODER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_RADDR_DECODER_FIELDS[] =
{
    &SBPM_REGS_SBPM_RADDR_DECODER_ID_2OVERWR_FIELD,
    &SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_RA_FIELD,
    &SBPM_REGS_SBPM_RADDR_DECODER_OVERWR_VALID_FIELD,
    &SBPM_REGS_SBPM_RADDR_DECODER_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_RADDR_DECODER_REG = 
{
    "REGS_SBPM_RADDR_DECODER",
#if RU_INCLUDE_DESC
    "SBPM_RADDR_DECODER Register",
    "This register let you choose one user that you would like to change its default RA.",
#endif
    SBPM_REGS_SBPM_RADDR_DECODER_REG_OFFSET,
    0,
    0,
    711,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    SBPM_REGS_SBPM_RADDR_DECODER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_WR_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_WR_DATA_FIELDS[] =
{
    &SBPM_REGS_SBPM_WR_DATA_SBPM_WR_DATA_FIELD,
    &SBPM_REGS_SBPM_WR_DATA_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_WR_DATA_REG = 
{
    "REGS_SBPM_WR_DATA",
#if RU_INCLUDE_DESC
    "SBPM_WR_DATA Register",
    "If SW want to write a whole word into the SBPMs RAM, it needs first to write the data to this register and then, send connect request with the wr_req bit asserted, with the address (BN field).",
#endif
    SBPM_REGS_SBPM_WR_DATA_REG_OFFSET,
    0,
    0,
    712,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_WR_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_UG_BAC_MAX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_UG_BAC_MAX_FIELDS[] =
{
    &SBPM_REGS_SBPM_UG_BAC_MAX_UG0BACMAX_FIELD,
    &SBPM_REGS_SBPM_UG_BAC_MAX_UG1BACMAX_FIELD,
    &SBPM_REGS_SBPM_UG_BAC_MAX_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_UG_BAC_MAX_REG = 
{
    "REGS_SBPM_UG_BAC_MAX",
#if RU_INCLUDE_DESC
    "SBPM_UG_BAC_MAX Register",
    "This register tracks the max values of the UG counters. it can be reset/modified by SW.",
#endif
    SBPM_REGS_SBPM_UG_BAC_MAX_REG_OFFSET,
    0,
    0,
    713,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    SBPM_REGS_SBPM_UG_BAC_MAX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_REGS_SBPM_SPARE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_REGS_SBPM_SPARE_FIELDS[] =
{
    &SBPM_REGS_SBPM_SPARE_GL_BAC_CLEAR_EN_FIELD,
    &SBPM_REGS_SBPM_SPARE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_REGS_SBPM_SPARE_REG = 
{
    "REGS_SBPM_SPARE",
#if RU_INCLUDE_DESC
    "SBPM_SPARE Register",
    "sbpm spare register",
#endif
    SBPM_REGS_SBPM_SPARE_REG_OFFSET,
    0,
    0,
    714,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SBPM_REGS_SBPM_SPARE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_INTR_CTRL_ISR
 ******************************************************************************/
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
    &SBPM_INTR_CTRL_ISR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_INTR_CTRL_ISR_REG = 
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    SBPM_INTR_CTRL_ISR_REG_OFFSET,
    0,
    0,
    715,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    SBPM_INTR_CTRL_ISR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_INTR_CTRL_ISM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_INTR_CTRL_ISM_FIELDS[] =
{
    &SBPM_INTR_CTRL_ISM_ISM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_INTR_CTRL_ISM_REG = 
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    SBPM_INTR_CTRL_ISM_REG_OFFSET,
    0,
    0,
    716,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_INTR_CTRL_ISM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_INTR_CTRL_IER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_INTR_CTRL_IER_FIELDS[] =
{
    &SBPM_INTR_CTRL_IER_IEM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_INTR_CTRL_IER_REG = 
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    SBPM_INTR_CTRL_IER_REG_OFFSET,
    0,
    0,
    717,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_INTR_CTRL_IER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: SBPM_INTR_CTRL_ITR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SBPM_INTR_CTRL_ITR_FIELDS[] =
{
    &SBPM_INTR_CTRL_ITR_IST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SBPM_INTR_CTRL_ITR_REG = 
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    SBPM_INTR_CTRL_ITR_REG_OFFSET,
    0,
    0,
    718,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SBPM_INTR_CTRL_ITR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: SBPM
 ******************************************************************************/
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
    &SBPM_INTR_CTRL_ISR_REG,
    &SBPM_INTR_CTRL_ISM_REG,
    &SBPM_INTR_CTRL_IER_REG,
    &SBPM_INTR_CTRL_ITR_REG,
};

unsigned long SBPM_ADDRS[] =
{
    0x82d99000,
};

const ru_block_rec SBPM_BLOCK = 
{
    "SBPM",
    SBPM_ADDRS,
    1,
    52,
    SBPM_REGS
};

/* End of file XRDP_SBPM.c */
