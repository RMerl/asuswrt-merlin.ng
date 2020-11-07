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
 * Register: DQM_MAX_ENTRIES_WORDS
 ******************************************************************************/
const ru_reg_rec DQM_MAX_ENTRIES_WORDS_REG = 
{
    "MAX_ENTRIES_WORDS",
#if RU_INCLUDE_DESC
    "DQMOL Max Entries in WORDS Register",
    "Maximum number of entries in words for all the queues. ",
#endif
    DQM_MAX_ENTRIES_WORDS_REG_OFFSET,
    0,
    0,
    216,
};

/******************************************************************************
 * Register: DQM_FPM_ADDR
 ******************************************************************************/
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
    217,
};

/******************************************************************************
 * Register: DQM_IRQ_STS
 ******************************************************************************/
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
    218,
};

/******************************************************************************
 * Register: DQM_IRQ_MSK
 ******************************************************************************/
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
    219,
};

/******************************************************************************
 * Register: DQM_BUF_SIZE
 ******************************************************************************/
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
    220,
};

/******************************************************************************
 * Register: DQM_BUF_BASE
 ******************************************************************************/
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
    221,
};

/******************************************************************************
 * Register: DQM_TOKENS_USED
 ******************************************************************************/
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
    222,
};

/******************************************************************************
 * Register: DQM_NUM_PUSHED
 ******************************************************************************/
const ru_reg_rec DQM_NUM_PUSHED_REG = 
{
    "NUM_PUSHED",
#if RU_INCLUDE_DESC
    "DQMOL Num Pushed Count Register",
    "counter for number of pushed transactions ",
#endif
    DQM_NUM_PUSHED_REG_OFFSET,
    0,
    0,
    223,
};

/******************************************************************************
 * Register: DQM_NUM_POPPED
 ******************************************************************************/
const ru_reg_rec DQM_NUM_POPPED_REG = 
{
    "NUM_POPPED",
#if RU_INCLUDE_DESC
    "DQMOL Num Popped Count Register",
    "counter for number of popped transactions ",
#endif
    DQM_NUM_POPPED_REG_OFFSET,
    0,
    0,
    224,
};

/******************************************************************************
 * Register: DQM_DIAG_SEL
 ******************************************************************************/
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
    225,
};

/******************************************************************************
 * Register: DQM_DIAG_DATA
 ******************************************************************************/
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
    226,
};

/******************************************************************************
 * Register: DQM_IRQ_TST
 ******************************************************************************/
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
    227,
};

/******************************************************************************
 * Register: DQM_TOKEN_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec DQM_TOKEN_FIFO_STATUS_REG = 
{
    "TOKEN_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "DQMOL IRQ Test Register",
    "content from prefetch token fifo ",
#endif
    DQM_TOKEN_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    228,
};

/******************************************************************************
 * Register: DQM_TOKEN_FIFO
 ******************************************************************************/
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
    229,
};

/******************************************************************************
 * Register: DQM_NUM_POPPED_NO_COMMIT
 ******************************************************************************/
const ru_reg_rec DQM_NUM_POPPED_NO_COMMIT_REG = 
{
    "NUM_POPPED_NO_COMMIT",
#if RU_INCLUDE_DESC
    "DQMOL Num Popped No Commit Count Register",
    "counter for number of popped with no commit transactions ",
#endif
    DQM_NUM_POPPED_NO_COMMIT_REG_OFFSET,
    0,
    0,
    230,
};

/******************************************************************************
 * Register: DQM_STATUS
 ******************************************************************************/
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
    231,
};

/******************************************************************************
 * Register: DQM_HEAD_PTR
 ******************************************************************************/
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
    232,
};

/******************************************************************************
 * Register: DQM_TAIL_PTR
 ******************************************************************************/
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
    233,
};

/******************************************************************************
 * Register: DQM_DQMOL_SIZE
 ******************************************************************************/
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
    234,
};

/******************************************************************************
 * Register: DQM_DQMOL_CFGA
 ******************************************************************************/
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
    235,
};

/******************************************************************************
 * Register: DQM_DQMOL_CFGB
 ******************************************************************************/
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
    236,
};

/******************************************************************************
 * Register: DQM_DQMOL_PUSHTOKEN
 ******************************************************************************/
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
    237,
};

/******************************************************************************
 * Register: DQM_DQMOL_PUSHTOKENNEXT
 ******************************************************************************/
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
    238,
};

/******************************************************************************
 * Register: DQM_DQMOL_POPTOKEN
 ******************************************************************************/
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
    239,
};

/******************************************************************************
 * Register: DQM_DQMOL_POPTOKENNEXT
 ******************************************************************************/
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
    240,
};

/******************************************************************************
 * Register: DQM_QueueSharedMem_qsmdata
 ******************************************************************************/
const ru_reg_rec DQM_QueueSharedMem_qsmdata_REG = 
{
    "QueueSharedMem_qsmdata",
#if RU_INCLUDE_DESC
    "QSM Shared memory space.",
    " "
    "Note that in the UTP has 48KB of memory space. With DFAP/GFAP/DTP,"
    "there are only 16KB of shared memory space. The entire"
    "memory space is carved out here as a placeholder in the DFAP/GFAP/DTP's case."
    "QSM memory",
#endif
    DQM_QueueSharedMem_qsmdata_REG_OFFSET,
    DQM_QueueSharedMem_qsmdata_REG_RAM_CNT,
    4,
    241,
};

/******************************************************************************
 * Block: DQM
 ******************************************************************************/
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
    &DQM_TOKEN_FIFO_REG,
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
    &DQM_QueueSharedMem_qsmdata_REG,
};

unsigned long DQM_ADDRS[] =
{
    0x82180034,
};

const ru_block_rec DQM_BLOCK = 
{
    "DQM",
    DQM_ADDRS,
    1,
    26,
    DQM_REGS
};

/* End of file XRDP_DQM.c */
