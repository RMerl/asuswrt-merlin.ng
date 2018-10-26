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
 * Field: DQM_FPM_ADDR_FPMADDRESS
 ******************************************************************************/
const ru_field_rec DQM_FPM_ADDR_FPMADDRESS_FIELD =
{
    "FPMADDRESS",
#if RU_INCLUDE_DESC
    "",
    "This is the FPM address to be used by components in this "
    "module. The same address is used to alloc and free a token in the FPM. ",
#endif
    DQM_FPM_ADDR_FPMADDRESS_FIELD_MASK,
    0,
    DQM_FPM_ADDR_FPMADDRESS_FIELD_WIDTH,
    DQM_FPM_ADDR_FPMADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_STS_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_IRQ_STS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_IRQ_STS_RESERVED0_FIELD_MASK,
    0,
    DQM_IRQ_STS_RESERVED0_FIELD_WIDTH,
    DQM_IRQ_STS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_STS_PUSHFULLQ
 ******************************************************************************/
const ru_field_rec DQM_IRQ_STS_PUSHFULLQ_FIELD =
{
    "PUSHFULLQ",
#if RU_INCLUDE_DESC
    "",
    "DQMOL Pushing a Full Queue IRQ Status (RW1C). This is a sticky high bit and needs to be cleared by writing to it. ",
#endif
    DQM_IRQ_STS_PUSHFULLQ_FIELD_MASK,
    0,
    DQM_IRQ_STS_PUSHFULLQ_FIELD_WIDTH,
    DQM_IRQ_STS_PUSHFULLQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_STS_POPEMPTYQ
 ******************************************************************************/
const ru_field_rec DQM_IRQ_STS_POPEMPTYQ_FIELD =
{
    "POPEMPTYQ",
#if RU_INCLUDE_DESC
    "",
    "DQMOL Popping an Empty Queue IRQ Status (RW1C). This is a sticky high bit and needs to be cleared by writing to it. ",
#endif
    DQM_IRQ_STS_POPEMPTYQ_FIELD_MASK,
    0,
    DQM_IRQ_STS_POPEMPTYQ_FIELD_WIDTH,
    DQM_IRQ_STS_POPEMPTYQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_MSK_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_IRQ_MSK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_IRQ_MSK_RESERVED0_FIELD_MASK,
    0,
    DQM_IRQ_MSK_RESERVED0_FIELD_WIDTH,
    DQM_IRQ_MSK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_MSK_PUSHFULLQ
 ******************************************************************************/
const ru_field_rec DQM_IRQ_MSK_PUSHFULLQ_FIELD =
{
    "PUSHFULLQ",
#if RU_INCLUDE_DESC
    "",
    "DQMOL Pushing a Full Queue IRQ Mask ",
#endif
    DQM_IRQ_MSK_PUSHFULLQ_FIELD_MASK,
    0,
    DQM_IRQ_MSK_PUSHFULLQ_FIELD_WIDTH,
    DQM_IRQ_MSK_PUSHFULLQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_MSK_POPEMPTYQ
 ******************************************************************************/
const ru_field_rec DQM_IRQ_MSK_POPEMPTYQ_FIELD =
{
    "POPEMPTYQ",
#if RU_INCLUDE_DESC
    "",
    "DQMOL Popping an Empty Queue IRQ Mask ",
#endif
    DQM_IRQ_MSK_POPEMPTYQ_FIELD_MASK,
    0,
    DQM_IRQ_MSK_POPEMPTYQ_FIELD_WIDTH,
    DQM_IRQ_MSK_POPEMPTYQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_BUF_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_BUF_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_BUF_SIZE_RESERVED0_FIELD_MASK,
    0,
    DQM_BUF_SIZE_RESERVED0_FIELD_WIDTH,
    DQM_BUF_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_BUF_SIZE_POOL_0_SIZE
 ******************************************************************************/
const ru_field_rec DQM_BUF_SIZE_POOL_0_SIZE_FIELD =
{
    "POOL_0_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer Size. This is an encoded value. \n0 =>  256 byte buffer, \n1 =>  512 byte buffer, \n2 => 1024 byte buffer, \n3 => 2048 byte buffer.",
#endif
    DQM_BUF_SIZE_POOL_0_SIZE_FIELD_MASK,
    0,
    DQM_BUF_SIZE_POOL_0_SIZE_FIELD_WIDTH,
    DQM_BUF_SIZE_POOL_0_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_BUF_BASE_BASE
 ******************************************************************************/
const ru_field_rec DQM_BUF_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "",
    "Buffer base address ",
#endif
    DQM_BUF_BASE_BASE_FIELD_MASK,
    0,
    DQM_BUF_BASE_BASE_FIELD_WIDTH,
    DQM_BUF_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_TOKENS_USED_COUNT
 ******************************************************************************/
const ru_field_rec DQM_TOKENS_USED_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Represents the current number of tokens used by the queue data "
    "structure. This count does not include tokens that are prefetched. ",
#endif
    DQM_TOKENS_USED_COUNT_FIELD_MASK,
    0,
    DQM_TOKENS_USED_COUNT_FIELD_WIDTH,
    DQM_TOKENS_USED_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_NUM_PUSHED_COUNT
 ******************************************************************************/
const ru_field_rec DQM_NUM_PUSHED_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Represents the current number of pushed transaction across all queues ",
#endif
    DQM_NUM_PUSHED_COUNT_FIELD_MASK,
    0,
    DQM_NUM_PUSHED_COUNT_FIELD_WIDTH,
    DQM_NUM_PUSHED_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_NUM_POPPED_COUNT
 ******************************************************************************/
const ru_field_rec DQM_NUM_POPPED_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Represents the current number of popped transaction across all queues ",
#endif
    DQM_NUM_POPPED_COUNT_FIELD_MASK,
    0,
    DQM_NUM_POPPED_COUNT_FIELD_WIDTH,
    DQM_NUM_POPPED_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_DIAG_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_DIAG_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_DIAG_SEL_RESERVED0_FIELD_MASK,
    0,
    DQM_DIAG_SEL_RESERVED0_FIELD_WIDTH,
    DQM_DIAG_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_DIAG_SEL_SEL
 ******************************************************************************/
const ru_field_rec DQM_DIAG_SEL_SEL_FIELD =
{
    "SEL",
#if RU_INCLUDE_DESC
    "",
    "MUX Select for routing diag data to the Diag Data Register ",
#endif
    DQM_DIAG_SEL_SEL_FIELD_MASK,
    0,
    DQM_DIAG_SEL_SEL_FIELD_WIDTH,
    DQM_DIAG_SEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_DIAG_DATA_DATA
 ******************************************************************************/
const ru_field_rec DQM_DIAG_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data presented as diag readback data.",
#endif
    DQM_DIAG_DATA_DATA_FIELD_MASK,
    0,
    DQM_DIAG_DATA_DATA_FIELD_WIDTH,
    DQM_DIAG_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_TST_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_IRQ_TST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_IRQ_TST_RESERVED0_FIELD_MASK,
    0,
    DQM_IRQ_TST_RESERVED0_FIELD_WIDTH,
    DQM_IRQ_TST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_TST_PUSHFULLQTST
 ******************************************************************************/
const ru_field_rec DQM_IRQ_TST_PUSHFULLQTST_FIELD =
{
    "PUSHFULLQTST",
#if RU_INCLUDE_DESC
    "",
    "Test the PushFullQ irq",
#endif
    DQM_IRQ_TST_PUSHFULLQTST_FIELD_MASK,
    0,
    DQM_IRQ_TST_PUSHFULLQTST_FIELD_WIDTH,
    DQM_IRQ_TST_PUSHFULLQTST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_IRQ_TST_POPEMPTYQTST
 ******************************************************************************/
const ru_field_rec DQM_IRQ_TST_POPEMPTYQTST_FIELD =
{
    "POPEMPTYQTST",
#if RU_INCLUDE_DESC
    "",
    "Test the PopEmptyQ irq",
#endif
    DQM_IRQ_TST_POPEMPTYQTST_FIELD_MASK,
    0,
    DQM_IRQ_TST_POPEMPTYQTST_FIELD_WIDTH,
    DQM_IRQ_TST_POPEMPTYQTST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_TOKEN_FIFO_TOKEN
 ******************************************************************************/
const ru_field_rec DQM_TOKEN_FIFO_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token value read from the token fifo ",
#endif
    DQM_TOKEN_FIFO_TOKEN_FIELD_MASK,
    0,
    DQM_TOKEN_FIFO_TOKEN_FIELD_WIDTH,
    DQM_TOKEN_FIFO_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_STATUS_RESERVED0_FIELD_MASK,
    0,
    DQM_STATUS_RESERVED0_FIELD_WIDTH,
    DQM_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_STATUS_Q_AVL_TKN_SPACE
 ******************************************************************************/
const ru_field_rec DQM_STATUS_Q_AVL_TKN_SPACE_FIELD =
{
    "Q_AVL_TKN_SPACE",
#if RU_INCLUDE_DESC
    "",
    "Queue Available Unused Token Space ",
#endif
    DQM_STATUS_Q_AVL_TKN_SPACE_FIELD_MASK,
    0,
    DQM_STATUS_Q_AVL_TKN_SPACE_FIELD_WIDTH,
    DQM_STATUS_Q_AVL_TKN_SPACE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_HEAD_PTR_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_HEAD_PTR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_HEAD_PTR_RESERVED0_FIELD_MASK,
    0,
    DQM_HEAD_PTR_RESERVED0_FIELD_WIDTH,
    DQM_HEAD_PTR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_HEAD_PTR_Q_HEAD_PTR
 ******************************************************************************/
const ru_field_rec DQM_HEAD_PTR_Q_HEAD_PTR_FIELD =
{
    "Q_HEAD_PTR",
#if RU_INCLUDE_DESC
    "",
    "Queue Head Pointer (in words). This is a read-only field "
    "and will reset to 0 whenever CNTRL_CFGB is programmed ",
#endif
    DQM_HEAD_PTR_Q_HEAD_PTR_FIELD_MASK,
    0,
    DQM_HEAD_PTR_Q_HEAD_PTR_FIELD_WIDTH,
    DQM_HEAD_PTR_Q_HEAD_PTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_TAIL_PTR_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_TAIL_PTR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_TAIL_PTR_RESERVED0_FIELD_MASK,
    0,
    DQM_TAIL_PTR_RESERVED0_FIELD_WIDTH,
    DQM_TAIL_PTR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_TAIL_PTR_Q_TAIL_PTR
 ******************************************************************************/
const ru_field_rec DQM_TAIL_PTR_Q_TAIL_PTR_FIELD =
{
    "Q_TAIL_PTR",
#if RU_INCLUDE_DESC
    "",
    "Queue Tail Pointer (in words). This is a read-only field "
    "and will reset to 0 whenever CNTRL_CFGB is programmed ",
#endif
    DQM_TAIL_PTR_Q_TAIL_PTR_FIELD_MASK,
    0,
    DQM_TAIL_PTR_Q_TAIL_PTR_FIELD_WIDTH,
    DQM_TAIL_PTR_Q_TAIL_PTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_DQMOL_SIZE_RESERVED0_FIELD_MASK,
    0,
    DQM_DQMOL_SIZE_RESERVED0_FIELD_WIDTH,
    DQM_DQMOL_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_SIZE_MAX_ENTRIES
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD =
{
    "MAX_ENTRIES",
#if RU_INCLUDE_DESC
    "",
    "Maximum number of entries allotted to the queue before it's full",
#endif
    DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD_MASK,
    0,
    DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD_WIDTH,
    DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD =
{
    "Q_DISABLE_OFFLOAD",
#if RU_INCLUDE_DESC
    "",
    "When set, this puts  the DQM OL queue into legacy DQM mode, there's no "
    "offloading of data. All queue data are stored in the QSM memory.",
#endif
    DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD_MASK,
    0,
    DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD_WIDTH,
    DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_SIZE_RESERVED1
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_SIZE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_DQMOL_SIZE_RESERVED1_FIELD_MASK,
    0,
    DQM_DQMOL_SIZE_RESERVED1_FIELD_WIDTH,
    DQM_DQMOL_SIZE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_SIZE_Q_TKN_SIZE
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD =
{
    "Q_TKN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Queue Token Size (in words). This is a base-0 value. A value of 0 means "
    "the token is 1 word long. A value of 1 means the token is 2 words long. "
    "This maxes out at a value of 3 to mean that a token is 4 words long. ",
#endif
    DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD_MASK,
    0,
    DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD_WIDTH,
    DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_CFGA_Q_SIZE
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_CFGA_Q_SIZE_FIELD =
{
    "Q_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Queue Memory Size (in words). It is required that the Queue Memory Size "
    "be whole multiple of the QUEUE_x_CNTRL_SIZE.Q_TKN_SIZE. For example, if "
    "Q_TKN_SIZE == 2 (which represents a 3 word token), then the Queue Memory "
    "Size must be 3, 6, 9, 12, etc. ",
#endif
    DQM_DQMOL_CFGA_Q_SIZE_FIELD_MASK,
    0,
    DQM_DQMOL_CFGA_Q_SIZE_FIELD_WIDTH,
    DQM_DQMOL_CFGA_Q_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_CFGA_Q_START_ADDR
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_CFGA_Q_START_ADDR_FIELD =
{
    "Q_START_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Queue Start Address (word addr). The hardware takes this word address "
    "and adds the base address of the Queue Shared Memory (0x4000 byte addr) "
    "to form the physical address for the Queue. ",
#endif
    DQM_DQMOL_CFGA_Q_START_ADDR_FIELD_MASK,
    0,
    DQM_DQMOL_CFGA_Q_START_ADDR_FIELD_WIDTH,
    DQM_DQMOL_CFGA_Q_START_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_CFGB_ENABLE
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_CFGB_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When set, the DQMOL is enabled and ready for use.",
#endif
    DQM_DQMOL_CFGB_ENABLE_FIELD_MASK,
    0,
    DQM_DQMOL_CFGB_ENABLE_FIELD_WIDTH,
    DQM_DQMOL_CFGB_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_CFGB_RESERVED0
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_CFGB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DQM_DQMOL_CFGB_RESERVED0_FIELD_MASK,
    0,
    DQM_DQMOL_CFGB_RESERVED0_FIELD_WIDTH,
    DQM_DQMOL_CFGB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_PUSHTOKEN_TOKEN
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Queue Token. This is the current token the offload hardware is using for this queue. ",
#endif
    DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD_MASK,
    0,
    DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD_WIDTH,
    DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_PUSHTOKENNEXT_TOKEN
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Queue Token. This is the current token the offload hardware is using for this queue. ",
#endif
    DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD_MASK,
    0,
    DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD_WIDTH,
    DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_POPTOKEN_TOKEN
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_POPTOKEN_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Queue Token. This is the current token the offload hardware is using for this queue. ",
#endif
    DQM_DQMOL_POPTOKEN_TOKEN_FIELD_MASK,
    0,
    DQM_DQMOL_POPTOKEN_TOKEN_FIELD_WIDTH,
    DQM_DQMOL_POPTOKEN_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_DQMOL_POPTOKENNEXT_TOKEN
 ******************************************************************************/
const ru_field_rec DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Queue Token. This is the current token the offload hardware is using for this queue. ",
#endif
    DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD_MASK,
    0,
    DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD_WIDTH,
    DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DQM_QSM_DATA_DATA
 ******************************************************************************/
const ru_field_rec DQM_QSM_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data ",
#endif
    DQM_QSM_DATA_DATA_FIELD_MASK,
    0,
    DQM_QSM_DATA_DATA_FIELD_WIDTH,
    DQM_QSM_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: DQM_FPM_ADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPM_ADDR_FIELDS[] =
{
    &DQM_FPM_ADDR_FPMADDRESS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_FPM_ADDR_REG = 
{
    "FPM_ADDR",
#if RU_INCLUDE_DESC
    "DQMOL FPM Address Register",
    "FPM Address Register",
#endif
    DQM_FPM_ADDR_REG_OFFSET,
    0,
    0,
    213,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPM_ADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_IRQ_STS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_IRQ_STS_FIELDS[] =
{
    &DQM_IRQ_STS_RESERVED0_FIELD,
    &DQM_IRQ_STS_PUSHFULLQ_FIELD,
    &DQM_IRQ_STS_POPEMPTYQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_IRQ_STS_REG = 
{
    "IRQ_STS",
#if RU_INCLUDE_DESC
    "DQMOL IRQ Status Register",
    "DQMOL Interrupt Status Register.",
#endif
    DQM_IRQ_STS_REG_OFFSET,
    0,
    0,
    214,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DQM_IRQ_STS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_IRQ_MSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_IRQ_MSK_FIELDS[] =
{
    &DQM_IRQ_MSK_RESERVED0_FIELD,
    &DQM_IRQ_MSK_PUSHFULLQ_FIELD,
    &DQM_IRQ_MSK_POPEMPTYQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_IRQ_MSK_REG = 
{
    "IRQ_MSK",
#if RU_INCLUDE_DESC
    "DQMOL IRQ Mask Register",
    "DQMOL Interrupt Mask Register.",
#endif
    DQM_IRQ_MSK_REG_OFFSET,
    0,
    0,
    215,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DQM_IRQ_MSK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_BUF_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_BUF_SIZE_FIELDS[] =
{
    &DQM_BUF_SIZE_RESERVED0_FIELD,
    &DQM_BUF_SIZE_POOL_0_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_BUF_SIZE_REG = 
{
    "BUF_SIZE",
#if RU_INCLUDE_DESC
    "DQMOL Token Buffer Size Register",
    "Token buffer size.",
#endif
    DQM_BUF_SIZE_REG_OFFSET,
    0,
    0,
    216,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_BUF_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_BUF_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_BUF_BASE_FIELDS[] =
{
    &DQM_BUF_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_BUF_BASE_REG = 
{
    "BUF_BASE",
#if RU_INCLUDE_DESC
    "DQMOL Token Buffer Base Register",
    "Token buffer base address ",
#endif
    DQM_BUF_BASE_REG_OFFSET,
    0,
    0,
    217,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_BUF_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_TOKENS_USED
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_TOKENS_USED_FIELDS[] =
{
    &DQM_TOKENS_USED_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_TOKENS_USED_REG = 
{
    "TOKENS_USED",
#if RU_INCLUDE_DESC
    "DQMOL Token Used Register",
    "Shows the number of tokens used by DQMOL ",
#endif
    DQM_TOKENS_USED_REG_OFFSET,
    0,
    0,
    218,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_TOKENS_USED_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_NUM_PUSHED
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_NUM_PUSHED_FIELDS[] =
{
    &DQM_NUM_PUSHED_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_NUM_PUSHED_REG = 
{
    "NUM_PUSHED",
#if RU_INCLUDE_DESC
    "DQMOL Token Used Register",
    "counter for number of pushed transactions ",
#endif
    DQM_NUM_PUSHED_REG_OFFSET,
    0,
    0,
    219,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_NUM_PUSHED_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_NUM_POPPED
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_NUM_POPPED_FIELDS[] =
{
    &DQM_NUM_POPPED_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_NUM_POPPED_REG = 
{
    "NUM_POPPED",
#if RU_INCLUDE_DESC
    "DQMOL Diag Select Register",
    "counter for number of popped transactions ",
#endif
    DQM_NUM_POPPED_REG_OFFSET,
    0,
    0,
    220,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_NUM_POPPED_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DIAG_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DIAG_SEL_FIELDS[] =
{
    &DQM_DIAG_SEL_RESERVED0_FIELD,
    &DQM_DIAG_SEL_SEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DIAG_SEL_REG = 
{
    "DIAG_SEL",
#if RU_INCLUDE_DESC
    "DQMOL Diag Readback Register",
    "MUX Select for Diags ",
#endif
    DQM_DIAG_SEL_REG_OFFSET,
    0,
    0,
    221,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_DIAG_SEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DIAG_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DIAG_DATA_FIELDS[] =
{
    &DQM_DIAG_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DIAG_DATA_REG = 
{
    "DIAG_DATA",
#if RU_INCLUDE_DESC
    "DQMOL Token Used Register",
    " ",
#endif
    DQM_DIAG_DATA_REG_OFFSET,
    0,
    0,
    222,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DIAG_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_IRQ_TST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_IRQ_TST_FIELDS[] =
{
    &DQM_IRQ_TST_RESERVED0_FIELD,
    &DQM_IRQ_TST_PUSHFULLQTST_FIELD,
    &DQM_IRQ_TST_POPEMPTYQTST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_IRQ_TST_REG = 
{
    "IRQ_TST",
#if RU_INCLUDE_DESC
    "DQMOL IRQ Test Register",
    "DQMOL Interrupt Test Register.",
#endif
    DQM_IRQ_TST_REG_OFFSET,
    0,
    0,
    223,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DQM_IRQ_TST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_TOKEN_FIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_TOKEN_FIFO_FIELDS[] =
{
    &DQM_TOKEN_FIFO_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_TOKEN_FIFO_REG = 
{
    "TOKEN_FIFO",
#if RU_INCLUDE_DESC
    "DQMOL TokenFifo Register",
    "content from prefetch token fifo ",
#endif
    DQM_TOKEN_FIFO_REG_OFFSET,
    DQM_TOKEN_FIFO_REG_RAM_CNT,
    4,
    224,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_TOKEN_FIFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_STATUS_FIELDS[] =
{
    &DQM_STATUS_RESERVED0_FIELD,
    &DQM_STATUS_Q_AVL_TKN_SPACE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "Queue Status Register",
    "Number of token unused space available on queue.\n"
    "This register is available on the DSPRAM read bus. ",
#endif
    DQM_STATUS_REG_OFFSET,
    DQM_STATUS_REG_RAM_CNT,
    4,
    225,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_HEAD_PTR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_HEAD_PTR_FIELDS[] =
{
    &DQM_HEAD_PTR_RESERVED0_FIELD,
    &DQM_HEAD_PTR_Q_HEAD_PTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_HEAD_PTR_REG = 
{
    "HEAD_PTR",
#if RU_INCLUDE_DESC
    "Queue Head Pointer Register",
    "",
#endif
    DQM_HEAD_PTR_REG_OFFSET,
    DQM_HEAD_PTR_REG_RAM_CNT,
    8,
    226,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_HEAD_PTR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_TAIL_PTR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_TAIL_PTR_FIELDS[] =
{
    &DQM_TAIL_PTR_RESERVED0_FIELD,
    &DQM_TAIL_PTR_Q_TAIL_PTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_TAIL_PTR_REG = 
{
    "TAIL_PTR",
#if RU_INCLUDE_DESC
    "Queue Tail Pointer Register",
    "",
#endif
    DQM_TAIL_PTR_REG_OFFSET,
    DQM_TAIL_PTR_REG_RAM_CNT,
    8,
    227,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_TAIL_PTR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DQMOL_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_SIZE_FIELDS[] =
{
    &DQM_DQMOL_SIZE_RESERVED0_FIELD,
    &DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD,
    &DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD,
    &DQM_DQMOL_SIZE_RESERVED1_FIELD,
    &DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DQMOL_SIZE_REG = 
{
    "DQMOL_SIZE",
#if RU_INCLUDE_DESC
    "Queue Size Register",
    "Number of token space available in Queue ",
#endif
    DQM_DQMOL_SIZE_REG_OFFSET,
    DQM_DQMOL_SIZE_REG_RAM_CNT,
    32,
    228,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    DQM_DQMOL_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DQMOL_CFGA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_CFGA_FIELDS[] =
{
    &DQM_DQMOL_CFGA_Q_SIZE_FIELD,
    &DQM_DQMOL_CFGA_Q_START_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DQMOL_CFGA_REG = 
{
    "DQMOL_CFGA",
#if RU_INCLUDE_DESC
    "Queue Config A Register",
    "Starting queue address and size of memory space ",
#endif
    DQM_DQMOL_CFGA_REG_OFFSET,
    DQM_DQMOL_CFGA_REG_RAM_CNT,
    32,
    229,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_DQMOL_CFGA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DQMOL_CFGB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_CFGB_FIELDS[] =
{
    &DQM_DQMOL_CFGB_ENABLE_FIELD,
    &DQM_DQMOL_CFGB_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DQMOL_CFGB_REG = 
{
    "DQMOL_CFGB",
#if RU_INCLUDE_DESC
    "Queue Config B Register",
    "Number of tokens and low watermark setting ",
#endif
    DQM_DQMOL_CFGB_REG_OFFSET,
    DQM_DQMOL_CFGB_REG_RAM_CNT,
    32,
    230,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_DQMOL_CFGB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DQMOL_PUSHTOKEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_PUSHTOKEN_FIELDS[] =
{
    &DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DQMOL_PUSHTOKEN_REG = 
{
    "DQMOL_PUSHTOKEN",
#if RU_INCLUDE_DESC
    "Queue Next Pop Token Register",
    "Current Token Register ",
#endif
    DQM_DQMOL_PUSHTOKEN_REG_OFFSET,
    DQM_DQMOL_PUSHTOKEN_REG_RAM_CNT,
    32,
    231,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_PUSHTOKEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DQMOL_PUSHTOKENNEXT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_PUSHTOKENNEXT_FIELDS[] =
{
    &DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DQMOL_PUSHTOKENNEXT_REG = 
{
    "DQMOL_PUSHTOKENNEXT",
#if RU_INCLUDE_DESC
    "Queue Next Pop Token Register",
    "Current Token Register ",
#endif
    DQM_DQMOL_PUSHTOKENNEXT_REG_OFFSET,
    DQM_DQMOL_PUSHTOKENNEXT_REG_RAM_CNT,
    32,
    232,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_PUSHTOKENNEXT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DQMOL_POPTOKEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_POPTOKEN_FIELDS[] =
{
    &DQM_DQMOL_POPTOKEN_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DQMOL_POPTOKEN_REG = 
{
    "DQMOL_POPTOKEN",
#if RU_INCLUDE_DESC
    "Queue Next Pop Token Register",
    "Current Token Register ",
#endif
    DQM_DQMOL_POPTOKEN_REG_OFFSET,
    DQM_DQMOL_POPTOKEN_REG_RAM_CNT,
    32,
    233,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_POPTOKEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_DQMOL_POPTOKENNEXT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_POPTOKENNEXT_FIELDS[] =
{
    &DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_DQMOL_POPTOKENNEXT_REG = 
{
    "DQMOL_POPTOKENNEXT",
#if RU_INCLUDE_DESC
    "Queue Next Pop Token Register",
    "Current Token Register ",
#endif
    DQM_DQMOL_POPTOKENNEXT_REG_OFFSET,
    DQM_DQMOL_POPTOKENNEXT_REG_RAM_CNT,
    32,
    234,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_POPTOKENNEXT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DQM_QSM_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_QSM_DATA_FIELDS[] =
{
    &DQM_QSM_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DQM_QSM_DATA_REG = 
{
    "QSM_DATA",
#if RU_INCLUDE_DESC
    "QSM Shared memory space.",
    " "
    "Note that in the UTP has 48KB of memory space. With DFAP/GFAP/DTP,"
    "there are only 16KB of shared memory space. The entire"
    "memory space is carved out here as a placeholder in the DFAP/GFAP/DTP's case."
    "QSM memory",
#endif
    DQM_QSM_DATA_REG_OFFSET,
    DQM_QSM_DATA_REG_RAM_CNT,
    4,
    235,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_QSM_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: DQM
 ******************************************************************************/
static const ru_reg_rec *DQM_REGS[] =
{
    &DQM_FPM_ADDR_REG,
    &DQM_IRQ_STS_REG,
    &DQM_IRQ_MSK_REG,
    &DQM_BUF_SIZE_REG,
    &DQM_BUF_BASE_REG,
    &DQM_TOKENS_USED_REG,
    &DQM_NUM_PUSHED_REG,
    &DQM_NUM_POPPED_REG,
    &DQM_DIAG_SEL_REG,
    &DQM_DIAG_DATA_REG,
    &DQM_IRQ_TST_REG,
    &DQM_TOKEN_FIFO_REG,
    &DQM_STATUS_REG,
    &DQM_HEAD_PTR_REG,
    &DQM_TAIL_PTR_REG,
    &DQM_DQMOL_SIZE_REG,
    &DQM_DQMOL_CFGA_REG,
    &DQM_DQMOL_CFGB_REG,
    &DQM_DQMOL_PUSHTOKEN_REG,
    &DQM_DQMOL_PUSHTOKENNEXT_REG,
    &DQM_DQMOL_POPTOKEN_REG,
    &DQM_DQMOL_POPTOKENNEXT_REG,
    &DQM_QSM_DATA_REG,
};

unsigned long DQM_ADDRS[] =
{
    0x8218004c,
};

const ru_block_rec DQM_BLOCK = 
{
    "DQM",
    DQM_ADDRS,
    1,
    23,
    DQM_REGS
};

/* End of file XRDP_DQM.c */
