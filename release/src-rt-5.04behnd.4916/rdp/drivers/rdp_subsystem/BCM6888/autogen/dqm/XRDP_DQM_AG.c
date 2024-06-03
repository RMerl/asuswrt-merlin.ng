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


#include "XRDP_DQM_AG.h"

/******************************************************************************
 * Register: NAME: DQM_TOKEN_FIFO_VALUE, TYPE: Type_dqmol_token_fifo
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN *****/
const ru_field_rec DQM_TOKEN_FIFO_VALUE_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token value read from the token fifo \n",
#endif
    { DQM_TOKEN_FIFO_VALUE_TOKEN_FIELD_MASK },
    0,
    { DQM_TOKEN_FIFO_VALUE_TOKEN_FIELD_WIDTH },
    { DQM_TOKEN_FIFO_VALUE_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_TOKEN_FIFO_VALUE_FIELDS[] =
{
    &DQM_TOKEN_FIFO_VALUE_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_TOKEN_FIFO_VALUE *****/
const ru_reg_rec DQM_TOKEN_FIFO_VALUE_REG =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "DQMOL TokenFifo[ 0] Register",
    "content from prefetch token fifo \n",
#endif
    { DQM_TOKEN_FIFO_VALUE_REG_OFFSET },
    0,
    0,
    281,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_TOKEN_FIFO_VALUE_FIELDS,
#endif
};

unsigned long DQM_TOKEN_FIFO_ADDRS[] =
{
    0x82C80000,
    0x82C80004,
    0x82C80008,
    0x82C8000C,
    0x82C80010,
    0x82C80014,
    0x82C80018,
    0x82C8001C,
    0x82C80020,
    0x82C80024,
    0x82C80028,
    0x82C8002C,
    0x82C80030,
    0x82C80034,
    0x82C80038,
    0x82C8003C,
};

static const ru_reg_rec *DQM_TOKEN_FIFO_REGS[] =
{
    &DQM_TOKEN_FIFO_VALUE_REG,
};

const ru_block_rec DQM_TOKEN_FIFO_BLOCK =
{
    "DQM_TOKEN_FIFO",
    DQM_TOKEN_FIFO_ADDRS,
    16,
    1,
    DQM_TOKEN_FIFO_REGS,
};
/******************************************************************************
 * Register: NAME: DQM_MAX_ENTRIES_WORDS, TYPE: Type_max_entries_words
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX *****/
const ru_field_rec DQM_MAX_ENTRIES_WORDS_MAX_FIELD =
{
    "MAX",
#if RU_INCLUDE_DESC
    "",
    "Represents the maximum number of entries the queue can hold (in words). This is a global settings.\n",
#endif
    { DQM_MAX_ENTRIES_WORDS_MAX_FIELD_MASK },
    0,
    { DQM_MAX_ENTRIES_WORDS_MAX_FIELD_WIDTH },
    { DQM_MAX_ENTRIES_WORDS_MAX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_MAX_ENTRIES_WORDS_FIELDS[] =
{
    &DQM_MAX_ENTRIES_WORDS_MAX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_MAX_ENTRIES_WORDS *****/
const ru_reg_rec DQM_MAX_ENTRIES_WORDS_REG =
{
    "MAX_ENTRIES_WORDS",
#if RU_INCLUDE_DESC
    "DQMOL Max Entries in WORDS Register",
    "Maximum number of entries in words for all the queues. \n",
#endif
    { DQM_MAX_ENTRIES_WORDS_REG_OFFSET },
    0,
    0,
    282,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_MAX_ENTRIES_WORDS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPM_ADDR, TYPE: Type_fpm_addr
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPMADDRESS *****/
const ru_field_rec DQM_FPM_ADDR_FPMADDRESS_FIELD =
{
    "FPMADDRESS",
#if RU_INCLUDE_DESC
    "",
    "This is the FPM address to be used by components in this module. The same address is used to alloc and free a token in the FPM. \n",
#endif
    { DQM_FPM_ADDR_FPMADDRESS_FIELD_MASK },
    0,
    { DQM_FPM_ADDR_FPMADDRESS_FIELD_WIDTH },
    { DQM_FPM_ADDR_FPMADDRESS_FIELD_SHIFT },
    3550478848,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPM_ADDR_FIELDS[] =
{
    &DQM_FPM_ADDR_FPMADDRESS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPM_ADDR *****/
const ru_reg_rec DQM_FPM_ADDR_REG =
{
    "FPM_ADDR",
#if RU_INCLUDE_DESC
    "DQMOL FPM Address Register",
    "FPM Address Register\n",
#endif
    { DQM_FPM_ADDR_REG_OFFSET },
    0,
    0,
    283,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPM_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_IRQ_STS, TYPE: Type_dqmol_irq_sts
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POPEMPTYQ *****/
const ru_field_rec DQM_IRQ_STS_POPEMPTYQ_FIELD =
{
    "POPEMPTYQ",
#if RU_INCLUDE_DESC
    "",
    "DQMOL Popping an Empty Queue IRQ Status (RW1C). This is a sticky high bit and needs to be cleared by writing to it. \n",
#endif
    { DQM_IRQ_STS_POPEMPTYQ_FIELD_MASK },
    0,
    { DQM_IRQ_STS_POPEMPTYQ_FIELD_WIDTH },
    { DQM_IRQ_STS_POPEMPTYQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PUSHFULLQ *****/
const ru_field_rec DQM_IRQ_STS_PUSHFULLQ_FIELD =
{
    "PUSHFULLQ",
#if RU_INCLUDE_DESC
    "",
    "DQMOL Pushing a Full Queue IRQ Status (RW1C). This is a sticky high bit and needs to be cleared by writing to it. \n",
#endif
    { DQM_IRQ_STS_PUSHFULLQ_FIELD_MASK },
    0,
    { DQM_IRQ_STS_PUSHFULLQ_FIELD_WIDTH },
    { DQM_IRQ_STS_PUSHFULLQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_IRQ_STS_FIELDS[] =
{
    &DQM_IRQ_STS_POPEMPTYQ_FIELD,
    &DQM_IRQ_STS_PUSHFULLQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_IRQ_STS *****/
const ru_reg_rec DQM_IRQ_STS_REG =
{
    "IRQ_STS",
#if RU_INCLUDE_DESC
    "DQMOL IRQ Status Register",
    "DQMOL Interrupt Status Register.\n",
#endif
    { DQM_IRQ_STS_REG_OFFSET },
    0,
    0,
    284,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_IRQ_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_IRQ_MSK, TYPE: Type_dqmol_irq_msk
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POPEMPTYQ *****/
const ru_field_rec DQM_IRQ_MSK_POPEMPTYQ_FIELD =
{
    "POPEMPTYQ",
#if RU_INCLUDE_DESC
    "",
    "DQMOL Popping an Empty Queue IRQ Mask \n",
#endif
    { DQM_IRQ_MSK_POPEMPTYQ_FIELD_MASK },
    0,
    { DQM_IRQ_MSK_POPEMPTYQ_FIELD_WIDTH },
    { DQM_IRQ_MSK_POPEMPTYQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PUSHFULLQ *****/
const ru_field_rec DQM_IRQ_MSK_PUSHFULLQ_FIELD =
{
    "PUSHFULLQ",
#if RU_INCLUDE_DESC
    "",
    "DQMOL Pushing a Full Queue IRQ Mask \n",
#endif
    { DQM_IRQ_MSK_PUSHFULLQ_FIELD_MASK },
    0,
    { DQM_IRQ_MSK_PUSHFULLQ_FIELD_WIDTH },
    { DQM_IRQ_MSK_PUSHFULLQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_IRQ_MSK_FIELDS[] =
{
    &DQM_IRQ_MSK_POPEMPTYQ_FIELD,
    &DQM_IRQ_MSK_PUSHFULLQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_IRQ_MSK *****/
const ru_reg_rec DQM_IRQ_MSK_REG =
{
    "IRQ_MSK",
#if RU_INCLUDE_DESC
    "DQMOL IRQ Mask Register",
    "DQMOL Interrupt Mask Register.\n",
#endif
    { DQM_IRQ_MSK_REG_OFFSET },
    0,
    0,
    285,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_IRQ_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_BUF_SIZE, TYPE: Type_dqmol_tok_buf_size
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_0_SIZE *****/
const ru_field_rec DQM_BUF_SIZE_POOL_0_SIZE_FIELD =
{
    "POOL_0_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer Size. This is an encoded value. \n0 =>  256 byte buffer, \n1 =>  512 byte buffer, \n2 => 1024 byte buffer, \n3 => 2048 byte buffer.\n",
#endif
    { DQM_BUF_SIZE_POOL_0_SIZE_FIELD_MASK },
    0,
    { DQM_BUF_SIZE_POOL_0_SIZE_FIELD_WIDTH },
    { DQM_BUF_SIZE_POOL_0_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_BUF_SIZE_FIELDS[] =
{
    &DQM_BUF_SIZE_POOL_0_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_BUF_SIZE *****/
const ru_reg_rec DQM_BUF_SIZE_REG =
{
    "BUF_SIZE",
#if RU_INCLUDE_DESC
    "DQMOL Token Buffer Size Register",
    "Token buffer size.\n",
#endif
    { DQM_BUF_SIZE_REG_OFFSET },
    0,
    0,
    286,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_BUF_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_BUF_BASE, TYPE: Type_dqmol_tok_buf_base
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BASE *****/
const ru_field_rec DQM_BUF_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "",
    "Buffer base address for bits[39:8]. Address bits [7:0] is always assumed to be 0. \n",
#endif
    { DQM_BUF_BASE_BASE_FIELD_MASK },
    0,
    { DQM_BUF_BASE_BASE_FIELD_WIDTH },
    { DQM_BUF_BASE_BASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_BUF_BASE_FIELDS[] =
{
    &DQM_BUF_BASE_BASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_BUF_BASE *****/
const ru_reg_rec DQM_BUF_BASE_REG =
{
    "BUF_BASE",
#if RU_INCLUDE_DESC
    "DQMOL Token Buffer Base Register",
    "Token buffer base address \n",
#endif
    { DQM_BUF_BASE_REG_OFFSET },
    0,
    0,
    287,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_BUF_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_TOKENS_USED, TYPE: Type_dqmol_tokens_used
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec DQM_TOKENS_USED_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Represents the current number of tokens used by the queue data structure. This count does not include tokens that are prefetched. \n",
#endif
    { DQM_TOKENS_USED_COUNT_FIELD_MASK },
    0,
    { DQM_TOKENS_USED_COUNT_FIELD_WIDTH },
    { DQM_TOKENS_USED_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_TOKENS_USED_FIELDS[] =
{
    &DQM_TOKENS_USED_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_TOKENS_USED *****/
const ru_reg_rec DQM_TOKENS_USED_REG =
{
    "TOKENS_USED",
#if RU_INCLUDE_DESC
    "DQMOL Token Used Register",
    "Shows the number of tokens used by DQMOL \n",
#endif
    { DQM_TOKENS_USED_REG_OFFSET },
    0,
    0,
    288,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_TOKENS_USED_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_NUM_PUSHED, TYPE: Type_dqmol_num_pushed
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec DQM_NUM_PUSHED_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Represents the current number of pushed transaction across all queues \n",
#endif
    { DQM_NUM_PUSHED_COUNT_FIELD_MASK },
    0,
    { DQM_NUM_PUSHED_COUNT_FIELD_WIDTH },
    { DQM_NUM_PUSHED_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_NUM_PUSHED_FIELDS[] =
{
    &DQM_NUM_PUSHED_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_NUM_PUSHED *****/
const ru_reg_rec DQM_NUM_PUSHED_REG =
{
    "NUM_PUSHED",
#if RU_INCLUDE_DESC
    "DQMOL Num Pushed Count Register",
    "counter for number of pushed transactions \n",
#endif
    { DQM_NUM_PUSHED_REG_OFFSET },
    0,
    0,
    289,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_NUM_PUSHED_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_NUM_POPPED, TYPE: Type_dqmol_num_popped
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec DQM_NUM_POPPED_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Represents the current number of popped transaction across all queues \n",
#endif
    { DQM_NUM_POPPED_COUNT_FIELD_MASK },
    0,
    { DQM_NUM_POPPED_COUNT_FIELD_WIDTH },
    { DQM_NUM_POPPED_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_NUM_POPPED_FIELDS[] =
{
    &DQM_NUM_POPPED_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_NUM_POPPED *****/
const ru_reg_rec DQM_NUM_POPPED_REG =
{
    "NUM_POPPED",
#if RU_INCLUDE_DESC
    "DQMOL Num Popped Count Register",
    "counter for number of popped transactions \n",
#endif
    { DQM_NUM_POPPED_REG_OFFSET },
    0,
    0,
    290,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_NUM_POPPED_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DIAG_SEL, TYPE: Type_dqmol_diag_sel
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SEL *****/
const ru_field_rec DQM_DIAG_SEL_SEL_FIELD =
{
    "SEL",
#if RU_INCLUDE_DESC
    "",
    "MUX Select for routing diag data to the Diag Data Register \n",
#endif
    { DQM_DIAG_SEL_SEL_FIELD_MASK },
    0,
    { DQM_DIAG_SEL_SEL_FIELD_WIDTH },
    { DQM_DIAG_SEL_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DIAG_SEL_FIELDS[] =
{
    &DQM_DIAG_SEL_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DIAG_SEL *****/
const ru_reg_rec DQM_DIAG_SEL_REG =
{
    "DIAG_SEL",
#if RU_INCLUDE_DESC
    "DQMOL Diag Readback Register",
    "MUX Select for Diags \n",
#endif
    { DQM_DIAG_SEL_REG_OFFSET },
    0,
    0,
    291,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DIAG_SEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DIAG_DATA, TYPE: Type_dqmol_diag_data
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DQM_DIAG_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data presented as diag readback data.\n",
#endif
    { DQM_DIAG_DATA_DATA_FIELD_MASK },
    0,
    { DQM_DIAG_DATA_DATA_FIELD_WIDTH },
    { DQM_DIAG_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DIAG_DATA_FIELDS[] =
{
    &DQM_DIAG_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DIAG_DATA *****/
const ru_reg_rec DQM_DIAG_DATA_REG =
{
    "DIAG_DATA",
#if RU_INCLUDE_DESC
    "DQMOL Token Used Register",
    " \n",
#endif
    { DQM_DIAG_DATA_REG_OFFSET },
    0,
    0,
    292,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DIAG_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_IRQ_TST, TYPE: Type_dqmol_irq_tst
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POPEMPTYQTST *****/
const ru_field_rec DQM_IRQ_TST_POPEMPTYQTST_FIELD =
{
    "POPEMPTYQTST",
#if RU_INCLUDE_DESC
    "",
    "Test the PopEmptyQ irq\n",
#endif
    { DQM_IRQ_TST_POPEMPTYQTST_FIELD_MASK },
    0,
    { DQM_IRQ_TST_POPEMPTYQTST_FIELD_WIDTH },
    { DQM_IRQ_TST_POPEMPTYQTST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PUSHFULLQTST *****/
const ru_field_rec DQM_IRQ_TST_PUSHFULLQTST_FIELD =
{
    "PUSHFULLQTST",
#if RU_INCLUDE_DESC
    "",
    "Test the PushFullQ irq\n",
#endif
    { DQM_IRQ_TST_PUSHFULLQTST_FIELD_MASK },
    0,
    { DQM_IRQ_TST_PUSHFULLQTST_FIELD_WIDTH },
    { DQM_IRQ_TST_PUSHFULLQTST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_IRQ_TST_FIELDS[] =
{
    &DQM_IRQ_TST_POPEMPTYQTST_FIELD,
    &DQM_IRQ_TST_PUSHFULLQTST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_IRQ_TST *****/
const ru_reg_rec DQM_IRQ_TST_REG =
{
    "IRQ_TST",
#if RU_INCLUDE_DESC
    "DQMOL IRQ Test Register",
    "DQMOL Interrupt Test Register.\n",
#endif
    { DQM_IRQ_TST_REG_OFFSET },
    0,
    0,
    293,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_IRQ_TST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_TOKEN_FIFO_STATUS, TYPE: Type_dqmol_token_fifo_status
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_LOC *****/
const ru_field_rec DQM_TOKEN_FIFO_STATUS_RD_LOC_FIELD =
{
    "RD_LOC",
#if RU_INCLUDE_DESC
    "",
    "token fifo read pointer \n",
#endif
    { DQM_TOKEN_FIFO_STATUS_RD_LOC_FIELD_MASK },
    0,
    { DQM_TOKEN_FIFO_STATUS_RD_LOC_FIELD_WIDTH },
    { DQM_TOKEN_FIFO_STATUS_RD_LOC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LEVEL *****/
const ru_field_rec DQM_TOKEN_FIFO_STATUS_LEVEL_FIELD =
{
    "LEVEL",
#if RU_INCLUDE_DESC
    "",
    "token fifo depth count \n",
#endif
    { DQM_TOKEN_FIFO_STATUS_LEVEL_FIELD_MASK },
    0,
    { DQM_TOKEN_FIFO_STATUS_LEVEL_FIELD_WIDTH },
    { DQM_TOKEN_FIFO_STATUS_LEVEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec DQM_TOKEN_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "token fifo empty \n",
#endif
    { DQM_TOKEN_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { DQM_TOKEN_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { DQM_TOKEN_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec DQM_TOKEN_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "token fifo full\n",
#endif
    { DQM_TOKEN_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { DQM_TOKEN_FIFO_STATUS_FULL_FIELD_WIDTH },
    { DQM_TOKEN_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_TOKEN_FIFO_STATUS_FIELDS[] =
{
    &DQM_TOKEN_FIFO_STATUS_RD_LOC_FIELD,
    &DQM_TOKEN_FIFO_STATUS_LEVEL_FIELD,
    &DQM_TOKEN_FIFO_STATUS_EMPTY_FIELD,
    &DQM_TOKEN_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_TOKEN_FIFO_STATUS *****/
const ru_reg_rec DQM_TOKEN_FIFO_STATUS_REG =
{
    "TOKEN_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "DQMOL IRQ Test Register",
    "content from prefetch token fifo \n",
#endif
    { DQM_TOKEN_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    294,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DQM_TOKEN_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_NUM_POPPED_NO_COMMIT, TYPE: Type_dqmol_num_popped_no_commit
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec DQM_NUM_POPPED_NO_COMMIT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Represents the current number of popped with no-commit transaction across all queues \n",
#endif
    { DQM_NUM_POPPED_NO_COMMIT_COUNT_FIELD_MASK },
    0,
    { DQM_NUM_POPPED_NO_COMMIT_COUNT_FIELD_WIDTH },
    { DQM_NUM_POPPED_NO_COMMIT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_NUM_POPPED_NO_COMMIT_FIELDS[] =
{
    &DQM_NUM_POPPED_NO_COMMIT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_NUM_POPPED_NO_COMMIT *****/
const ru_reg_rec DQM_NUM_POPPED_NO_COMMIT_REG =
{
    "NUM_POPPED_NO_COMMIT",
#if RU_INCLUDE_DESC
    "DQMOL Num Popped No Commit Count Register",
    "counter for number of popped with no commit transactions \n",
#endif
    { DQM_NUM_POPPED_NO_COMMIT_REG_OFFSET },
    0,
    0,
    295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_NUM_POPPED_NO_COMMIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_QSMDATA, TYPE: Type_data
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DQM_QSMDATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data \n",
#endif
    { DQM_QSMDATA_DATA_FIELD_MASK },
    0,
    { DQM_QSMDATA_DATA_FIELD_WIDTH },
    { DQM_QSMDATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_QSMDATA_FIELDS[] =
{
    &DQM_QSMDATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_QSMDATA *****/
const ru_reg_rec DQM_QSMDATA_REG =
{
    "QSMDATA",
#if RU_INCLUDE_DESC
    "QSM Shared memory space.",
    " \n",
#endif
    { DQM_QSMDATA_REG_OFFSET },
    DQM_QSMDATA_REG_RAM_CNT,
    4,
    296,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_QSMDATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_STATUS, TYPE: Type_dqmol_q_status
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_AVL_TKN_SPACE *****/
const ru_field_rec DQM_STATUS_Q_AVL_TKN_SPACE_FIELD =
{
    "Q_AVL_TKN_SPACE",
#if RU_INCLUDE_DESC
    "",
    "Queue Available Unused Token Space (in words). \n",
#endif
    { DQM_STATUS_Q_AVL_TKN_SPACE_FIELD_MASK },
    0,
    { DQM_STATUS_Q_AVL_TKN_SPACE_FIELD_WIDTH },
    { DQM_STATUS_Q_AVL_TKN_SPACE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NEXT_LINE_DATA_IS_LOCAL *****/
const ru_field_rec DQM_STATUS_NEXT_LINE_DATA_IS_LOCAL_FIELD =
{
    "NEXT_LINE_DATA_IS_LOCAL",
#if RU_INCLUDE_DESC
    "",
    "Queue data for the next Line is stored locally in the QSM. \n",
#endif
    { DQM_STATUS_NEXT_LINE_DATA_IS_LOCAL_FIELD_MASK },
    0,
    { DQM_STATUS_NEXT_LINE_DATA_IS_LOCAL_FIELD_WIDTH },
    { DQM_STATUS_NEXT_LINE_DATA_IS_LOCAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CURR_LINE_DATA_IS_LOCAL *****/
const ru_field_rec DQM_STATUS_CURR_LINE_DATA_IS_LOCAL_FIELD =
{
    "CURR_LINE_DATA_IS_LOCAL",
#if RU_INCLUDE_DESC
    "",
    "Queue data for the current Line is stored locally in the QSM. \n",
#endif
    { DQM_STATUS_CURR_LINE_DATA_IS_LOCAL_FIELD_MASK },
    0,
    { DQM_STATUS_CURR_LINE_DATA_IS_LOCAL_FIELD_WIDTH },
    { DQM_STATUS_CURR_LINE_DATA_IS_LOCAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_STATUS_FIELDS[] =
{
    &DQM_STATUS_Q_AVL_TKN_SPACE_FIELD,
    &DQM_STATUS_NEXT_LINE_DATA_IS_LOCAL_FIELD,
    &DQM_STATUS_CURR_LINE_DATA_IS_LOCAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_STATUS *****/
const ru_reg_rec DQM_STATUS_REG =
{
    "STATUS",
#if RU_INCLUDE_DESC
    "Queue Status Register",
    "Number of token unused space available on queue.\nThis register is available on the DSPRAM read bus. \n",
#endif
    { DQM_STATUS_REG_OFFSET },
    DQM_STATUS_REG_RAM_CNT,
    4,
    297,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DQM_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_HEAD_PTR, TYPE: Type_ioproc_dqmol_q_head_ptr
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_HEAD_PTR *****/
const ru_field_rec DQM_HEAD_PTR_Q_HEAD_PTR_FIELD =
{
    "Q_HEAD_PTR",
#if RU_INCLUDE_DESC
    "",
    "Queue Head Pointer (in words). This is a read-only field and will reset to 0 whenever CNTRL_CFGB is programmed \n",
#endif
    { DQM_HEAD_PTR_Q_HEAD_PTR_FIELD_MASK },
    0,
    { DQM_HEAD_PTR_Q_HEAD_PTR_FIELD_WIDTH },
    { DQM_HEAD_PTR_Q_HEAD_PTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_HEAD_PTR_FIELDS[] =
{
    &DQM_HEAD_PTR_Q_HEAD_PTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_HEAD_PTR *****/
const ru_reg_rec DQM_HEAD_PTR_REG =
{
    "HEAD_PTR",
#if RU_INCLUDE_DESC
    "Queue Head Pointer Register",
    "",
#endif
    { DQM_HEAD_PTR_REG_OFFSET },
    DQM_HEAD_PTR_REG_RAM_CNT,
    8,
    298,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_HEAD_PTR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_TAIL_PTR, TYPE: Type_ioproc_dqmol_q_tail_ptr
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_TAIL_PTR *****/
const ru_field_rec DQM_TAIL_PTR_Q_TAIL_PTR_FIELD =
{
    "Q_TAIL_PTR",
#if RU_INCLUDE_DESC
    "",
    "Queue Tail Pointer (in words). This is a read-only field and will reset to 0 whenever CNTRL_CFGB is programmed \n",
#endif
    { DQM_TAIL_PTR_Q_TAIL_PTR_FIELD_MASK },
    0,
    { DQM_TAIL_PTR_Q_TAIL_PTR_FIELD_WIDTH },
    { DQM_TAIL_PTR_Q_TAIL_PTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_TAIL_PTR_FIELDS[] =
{
    &DQM_TAIL_PTR_Q_TAIL_PTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_TAIL_PTR *****/
const ru_reg_rec DQM_TAIL_PTR_REG =
{
    "TAIL_PTR",
#if RU_INCLUDE_DESC
    "Queue Tail Pointer Register",
    "",
#endif
    { DQM_TAIL_PTR_REG_OFFSET },
    DQM_TAIL_PTR_REG_RAM_CNT,
    8,
    299,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_TAIL_PTR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_WORD0, TYPE: Type_ioproc_dqmol_q_tag_word0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DQM_WORD0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { DQM_WORD0_DATA_FIELD_MASK },
    0,
    { DQM_WORD0_DATA_FIELD_WIDTH },
    { DQM_WORD0_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_WORD0_FIELDS[] =
{
    &DQM_WORD0_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_WORD0 *****/
const ru_reg_rec DQM_WORD0_REG =
{
    "WORD0",
#if RU_INCLUDE_DESC
    "Queue Tag Word0 Register",
    "",
#endif
    { DQM_WORD0_REG_OFFSET },
    DQM_WORD0_REG_RAM_CNT,
    12,
    300,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_WORD0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_WORD1, TYPE: Type_ioproc_dqmol_q_tag_word1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DQM_WORD1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { DQM_WORD1_DATA_FIELD_MASK },
    0,
    { DQM_WORD1_DATA_FIELD_WIDTH },
    { DQM_WORD1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_WORD1_FIELDS[] =
{
    &DQM_WORD1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_WORD1 *****/
const ru_reg_rec DQM_WORD1_REG =
{
    "WORD1",
#if RU_INCLUDE_DESC
    "Queue Tag Word1 Register",
    "",
#endif
    { DQM_WORD1_REG_OFFSET },
    DQM_WORD1_REG_RAM_CNT,
    12,
    301,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_WORD1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_WORD2, TYPE: Type_ioproc_dqmol_q_tag_word2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DQM_WORD2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    { DQM_WORD2_DATA_FIELD_MASK },
    0,
    { DQM_WORD2_DATA_FIELD_WIDTH },
    { DQM_WORD2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_WORD2_FIELDS[] =
{
    &DQM_WORD2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_WORD2 *****/
const ru_reg_rec DQM_WORD2_REG =
{
    "WORD2",
#if RU_INCLUDE_DESC
    "Queue Tag Word2 Register",
    "",
#endif
    { DQM_WORD2_REG_OFFSET },
    DQM_WORD2_REG_RAM_CNT,
    12,
    302,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_WORD2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DQMOL_SIZE, TYPE: Type_dqmol_q_size
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_TKN_SIZE *****/
const ru_field_rec DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD =
{
    "Q_TKN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Queue Token Size (in words). This is a base-0 value. A value of 0 means the token is 1 word long. A value of 1 means the token is 2 words long. This maxes out at a value of 3 to mean that a token is 4 words long. \n",
#endif
    { DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD_MASK },
    0,
    { DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD_WIDTH },
    { DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_DISABLE_OFFLOAD *****/
const ru_field_rec DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD =
{
    "Q_DISABLE_OFFLOAD",
#if RU_INCLUDE_DESC
    "",
    "When set, this puts  the DQM OL queue into legacy DQM mode, there's no offloading of data. All queue data are stored in the QSM memory.\n",
#endif
    { DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD_MASK },
    0,
    { DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD_WIDTH },
    { DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_ENTRIES *****/
const ru_field_rec DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD =
{
    "MAX_ENTRIES",
#if RU_INCLUDE_DESC
    "",
    "Maximum number of entries allotted to the queue before it's full\n",
#endif
    { DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD_MASK },
    0,
    { DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD_WIDTH },
    { DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD_SHIFT },
    16384,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_SIZE_FIELDS[] =
{
    &DQM_DQMOL_SIZE_Q_TKN_SIZE_FIELD,
    &DQM_DQMOL_SIZE_Q_DISABLE_OFFLOAD_FIELD,
    &DQM_DQMOL_SIZE_MAX_ENTRIES_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DQMOL_SIZE *****/
const ru_reg_rec DQM_DQMOL_SIZE_REG =
{
    "DQMOL_SIZE",
#if RU_INCLUDE_DESC
    "Queue Size Register",
    "Number of token space available in Queue \n",
#endif
    { DQM_DQMOL_SIZE_REG_OFFSET },
    DQM_DQMOL_SIZE_REG_RAM_CNT,
    32,
    303,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DQM_DQMOL_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DQMOL_CFGA, TYPE: Type_dqmol_queue_cfga
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_START_ADDR *****/
const ru_field_rec DQM_DQMOL_CFGA_Q_START_ADDR_FIELD =
{
    "Q_START_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Queue Start Address (word addr). The hardware takes this word address and adds the base address of the Queue Shared Memory (0x4000 byte addr) to form the physical address for the Queue. \n",
#endif
    { DQM_DQMOL_CFGA_Q_START_ADDR_FIELD_MASK },
    0,
    { DQM_DQMOL_CFGA_Q_START_ADDR_FIELD_WIDTH },
    { DQM_DQMOL_CFGA_Q_START_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_SIZE *****/
const ru_field_rec DQM_DQMOL_CFGA_Q_SIZE_FIELD =
{
    "Q_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Queue Memory Size (in words). It is required that the Queue Memory Size be whole multiple of the QUEUE_x_CNTRL_SIZE.Q_TKN_SIZE. For example, if Q_TKN_SIZE == 2 (which represents a 3 word token), then the Queue Memory Size must be 3, 6, 9, 12, etc. \n",
#endif
    { DQM_DQMOL_CFGA_Q_SIZE_FIELD_MASK },
    0,
    { DQM_DQMOL_CFGA_Q_SIZE_FIELD_WIDTH },
    { DQM_DQMOL_CFGA_Q_SIZE_FIELD_SHIFT },
    128,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_CFGA_FIELDS[] =
{
    &DQM_DQMOL_CFGA_Q_START_ADDR_FIELD,
    &DQM_DQMOL_CFGA_Q_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DQMOL_CFGA *****/
const ru_reg_rec DQM_DQMOL_CFGA_REG =
{
    "DQMOL_CFGA",
#if RU_INCLUDE_DESC
    "Queue Config A Register",
    "Starting queue address and size of memory space \n",
#endif
    { DQM_DQMOL_CFGA_REG_OFFSET },
    DQM_DQMOL_CFGA_REG_RAM_CNT,
    32,
    304,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DQM_DQMOL_CFGA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DQMOL_CFGB, TYPE: Type_dqmol_queue_cfgb
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec DQM_DQMOL_CFGB_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When set, the DQMOL is enabled and ready for use.\n",
#endif
    { DQM_DQMOL_CFGB_ENABLE_FIELD_MASK },
    0,
    { DQM_DQMOL_CFGB_ENABLE_FIELD_WIDTH },
    { DQM_DQMOL_CFGB_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_CFGB_FIELDS[] =
{
    &DQM_DQMOL_CFGB_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DQMOL_CFGB *****/
const ru_reg_rec DQM_DQMOL_CFGB_REG =
{
    "DQMOL_CFGB",
#if RU_INCLUDE_DESC
    "Queue Config B Register",
    "Number of tokens and low watermark setting \n",
#endif
    { DQM_DQMOL_CFGB_REG_OFFSET },
    DQM_DQMOL_CFGB_REG_RAM_CNT,
    32,
    305,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_CFGB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DQMOL_PUSHTOKEN, TYPE: Type_ioproc_dqm_queue_token
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN *****/
const ru_field_rec DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Queue Token. This is the current token the offload hardware is using for this queue. \n",
#endif
    { DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD_MASK },
    0,
    { DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD_WIDTH },
    { DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_PUSHTOKEN_FIELDS[] =
{
    &DQM_DQMOL_PUSHTOKEN_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DQMOL_PUSHTOKEN *****/
const ru_reg_rec DQM_DQMOL_PUSHTOKEN_REG =
{
    "DQMOL_PUSHTOKEN",
#if RU_INCLUDE_DESC
    "Queue Push Token Register",
    "Current Token Register \n",
#endif
    { DQM_DQMOL_PUSHTOKEN_REG_OFFSET },
    DQM_DQMOL_PUSHTOKEN_REG_RAM_CNT,
    32,
    306,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_PUSHTOKEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DQMOL_PUSHTOKENNEXT, TYPE: Type_ioproc_dqm_queue_token
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN *****/
const ru_field_rec DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Queue Token. This is the current token the offload hardware is using for this queue. \n",
#endif
    { DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD_MASK },
    0,
    { DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD_WIDTH },
    { DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_PUSHTOKENNEXT_FIELDS[] =
{
    &DQM_DQMOL_PUSHTOKENNEXT_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DQMOL_PUSHTOKENNEXT *****/
const ru_reg_rec DQM_DQMOL_PUSHTOKENNEXT_REG =
{
    "DQMOL_PUSHTOKENNEXT",
#if RU_INCLUDE_DESC
    "Queue Next Push Token Register",
    "Current Token Register \n",
#endif
    { DQM_DQMOL_PUSHTOKENNEXT_REG_OFFSET },
    DQM_DQMOL_PUSHTOKENNEXT_REG_RAM_CNT,
    32,
    307,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_PUSHTOKENNEXT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DQMOL_POPTOKEN, TYPE: Type_ioproc_dqm_queue_token
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN *****/
const ru_field_rec DQM_DQMOL_POPTOKEN_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Queue Token. This is the current token the offload hardware is using for this queue. \n",
#endif
    { DQM_DQMOL_POPTOKEN_TOKEN_FIELD_MASK },
    0,
    { DQM_DQMOL_POPTOKEN_TOKEN_FIELD_WIDTH },
    { DQM_DQMOL_POPTOKEN_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_POPTOKEN_FIELDS[] =
{
    &DQM_DQMOL_POPTOKEN_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DQMOL_POPTOKEN *****/
const ru_reg_rec DQM_DQMOL_POPTOKEN_REG =
{
    "DQMOL_POPTOKEN",
#if RU_INCLUDE_DESC
    "Queue Pop Token Register",
    "Current Token Register \n",
#endif
    { DQM_DQMOL_POPTOKEN_REG_OFFSET },
    DQM_DQMOL_POPTOKEN_REG_RAM_CNT,
    32,
    308,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_POPTOKEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_DQMOL_POPTOKENNEXT, TYPE: Type_ioproc_dqm_queue_token
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN *****/
const ru_field_rec DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD =
{
    "TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Queue Token. This is the current token the offload hardware is using for this queue. \n",
#endif
    { DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD_MASK },
    0,
    { DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD_WIDTH },
    { DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_DQMOL_POPTOKENNEXT_FIELDS[] =
{
    &DQM_DQMOL_POPTOKENNEXT_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_DQMOL_POPTOKENNEXT *****/
const ru_reg_rec DQM_DQMOL_POPTOKENNEXT_REG =
{
    "DQMOL_POPTOKENNEXT",
#if RU_INCLUDE_DESC
    "Queue Next Pop Token Register",
    "Current Token Register \n",
#endif
    { DQM_DQMOL_POPTOKENNEXT_REG_OFFSET },
    DQM_DQMOL_POPTOKENNEXT_REG_RAM_CNT,
    32,
    309,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_DQMOL_POPTOKENNEXT_FIELDS,
#endif
};

unsigned long DQM_ADDRS[] =
{
    0x82C80000,
};

static const ru_reg_rec *DQM_REGS[] =
{
    &DQM_MAX_ENTRIES_WORDS_REG,
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
    &DQM_TOKEN_FIFO_STATUS_REG,
    &DQM_NUM_POPPED_NO_COMMIT_REG,
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
    &DQM_WORD0_REG,
    &DQM_WORD1_REG,
    &DQM_WORD2_REG,
    &DQM_QSMDATA_REG,
};

const ru_block_rec DQM_BLOCK =
{
    "DQM",
    DQM_ADDRS,
    1,
    28,
    DQM_REGS,
};
