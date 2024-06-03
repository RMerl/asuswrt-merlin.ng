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


#include "XRDP_DQM_FPMINI_AG.h"

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2, TYPE: Type_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_DATA_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_DATA_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2 *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_REG =
{
    "FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2",
#if RU_INCLUDE_DESC
    "MEM_ENTRY 0..511 Register",
    "mem_entry\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_REG_OFFSET },
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_REG_RAM_CNT,
    4,
    315,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1, TYPE: Type_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: L1 *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_L1_FIELD =
{
    "L1",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_L1_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_L1_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_L1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_L1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1 *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_REG =
{
    "FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1",
#if RU_INCLUDE_DESC
    "MEM_ENTRY 0..15 Register",
    "mem_entry\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_REG_OFFSET },
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_REG_RAM_CNT,
    4,
    316,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0, TYPE: Type_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_DATA_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_DATA_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0 *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_REG =
{
    "FPMINI_BLOCK_FPMINI_LVL_0_REG_L0",
#if RU_INCLUDE_DESC
    "MEM_ENTRY Register",
    "mem_entry\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_REG_OFFSET },
    0,
    0,
    317,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT, TYPE: Type_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "init\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_INIT_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_INIT_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_INIT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_INIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_REG =
{
    "FPMINI_BLOCK_FPMINI_CFG0_L2_INIT",
#if RU_INCLUDE_DESC
    "L2_MEM_INIT Register",
    "initialization for l2 memory.\nwr 1 to start init, poll for 0 for done.\n\nL0/L1 have reset value, that will propagate to L2 when enabling l2_mem_init bit\nChange in configuration value of L0 will automatically change also value of L1, and then this will propagate to L2 when enabling l2_mem_init bit\n\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_REG_OFFSET },
    0,
    0,
    318,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK, TYPE: Type_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "0: dis\n1: en\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_EN_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_EN_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_REG =
{
    "FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK",
#if RU_INCLUDE_DESC
    "ALLC_FAST_ACK Register",
    "alloc fast ack.\n0: nack when L0 indicates no tokens\n1: nack when no tokens in prealloc fifo\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_REG_OFFSET },
    0,
    0,
    319,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS, TYPE: Type_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_VAL_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_VAL_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_VAL_FIELD_SHIFT },
    16384,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_REG =
{
    "FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS",
#if RU_INCLUDE_DESC
    "NUM_AVAIL_TOKENS Register",
    "number of available tokens.\ndecrement when allocating by client (not pre-fetch fifo),\nincrement when freed by client.\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_REG_OFFSET },
    0,
    0,
    320,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM, TYPE: Type_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_VAL_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_VAL_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_VAL_FIELD_SHIFT },
    16384,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_REG =
{
    "FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM",
#if RU_INCLUDE_DESC
    "NUM_AVAIL_TOKENS_LOW_WM Register",
    "number of available tokens low watermark.\nstart from max value, and decremented when getting lower value of available tokens.\nThis register allows software to read the lowest value the NUM_AVAIL_TOKENS count reached since the last time it was cleared. Any write to this register will reset the value back to the maximum number of tokens.\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_REG_OFFSET },
    0,
    0,
    321,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR, TYPE: Type_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_VAL_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_VAL_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_REG =
{
    "FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR",
#if RU_INCLUDE_DESC
    "FREE_ERR_CNTR Register",
    "free cmd for unallocated token\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_REG_OFFSET },
    0,
    0,
    322,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG, TYPE: Type_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_CLR *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_RD_CLR_FIELD =
{
    "RD_CLR",
#if RU_INCLUDE_DESC
    "",
    "read clear bit\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_RD_CLR_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_RD_CLR_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_RD_CLR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRAP *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_WRAP_FIELD =
{
    "WRAP",
#if RU_INCLUDE_DESC
    "",
    "read clear bit\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_WRAP_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_WRAP_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_WRAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MSK_FREE_ERR *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_FREE_ERR_FIELD =
{
    "MSK_FREE_ERR",
#if RU_INCLUDE_DESC
    "",
    "mask_fpmini_free error\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_FREE_ERR_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_FREE_ERR_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_FREE_ERR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MSK_MC_INC_ERR *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_INC_ERR_FIELD =
{
    "MSK_MC_INC_ERR",
#if RU_INCLUDE_DESC
    "",
    "mask_fpmcast_mc_inc error\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_INC_ERR_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_INC_ERR_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_INC_ERR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MSK_MC_DEC_ERR *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_DEC_ERR_FIELD =
{
    "MSK_MC_DEC_ERR",
#if RU_INCLUDE_DESC
    "",
    "mask_fpmcast_mc_dec error\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_DEC_ERR_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_DEC_ERR_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_DEC_ERR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_RD_CLR_FIELD,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_WRAP_FIELD,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_FREE_ERR_FIELD,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_INC_ERR_FIELD,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_MSK_MC_DEC_ERR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_REG =
{
    "FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "bits rd_clr and wrap for the pm counters(above), masks for error counters.\nRelevant also for fpmcast performance counters.\n\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_REG_OFFSET },
    0,
    0,
    323,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL, TYPE: Type_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VS *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_VS_FIELD =
{
    "VS",
#if RU_INCLUDE_DESC
    "",
    "selects th debug vector\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_VS_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_VS_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_VS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_VS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_REG =
{
    "FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL",
#if RU_INCLUDE_DESC
    "DBG_MUX_SEL Register",
    "selects the debug vector.\nFirst 32 addresses dedicated to cmd fifo, next 32 addresses dedicated to alloc fifo, and from 0x40 debug of internal signals.\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_REG_OFFSET },
    0,
    0,
    324,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS, TYPE: Type_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VB *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_VB_FIELD =
{
    "VB",
#if RU_INCLUDE_DESC
    "",
    "debug vector\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_VB_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_VB_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_VB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_VB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_REG =
{
    "FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS",
#if RU_INCLUDE_DESC
    "DBG_BUS Register",
    "the debug bus\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_REG_OFFSET },
    0,
    0,
    325,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC, TYPE: Type_FPMINI_BLOCK_FPMCAST_MC_MEM_MC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_DATA_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_DATA_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_REG =
{
    "FPMINI_BLOCK_FPMCAST_MC_MEM_MC",
#if RU_INCLUDE_DESC
    "MEM_ENTRY 0..2047 Register",
    "mem_entry\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_REG_OFFSET },
    DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_REG_RAM_CNT,
    4,
    326,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT, TYPE: Type_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "init\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_INIT_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_INIT_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_INIT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_INIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_REG =
{
    "FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT",
#if RU_INCLUDE_DESC
    "MC_MEM_INIT Register",
    "initialization for mc memory.\nwr 1 to start init, poll for 0 for done.\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_REG_OFFSET },
    0,
    0,
    327,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC, TYPE: Type_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "0: dis\n1: en\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_EN_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_EN_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_REG =
{
    "FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC",
#if RU_INCLUDE_DESC
    "FREE_BYPASS_MC Register",
    "free commands bypass mc logic.\n0: free first accesses mc mem, and if needed frees the token\n1: dqm debug mode: free directly accesses free token logic. no access to mc mem.\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_REG_OFFSET },
    0,
    0,
    328,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR, TYPE: Type_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_VAL_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_VAL_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_REG =
{
    "FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR",
#if RU_INCLUDE_DESC
    "MC_INC_ERR_CNTR Register",
    "mc inc cmd new value is above 15.\nnew value will not be written.\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_REG_OFFSET },
    0,
    0,
    329,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR, TYPE: Type_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_VAL_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_VAL_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_REG =
{
    "FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR",
#if RU_INCLUDE_DESC
    "MC_DEC_ERR_CNTR Register",
    "mc dec cmd new value is below 0.\nnew value will not be written.\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_REG_OFFSET },
    0,
    0,
    330,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL, TYPE: Type_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VS *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_VS_FIELD =
{
    "VS",
#if RU_INCLUDE_DESC
    "",
    "selects th debug vector\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_VS_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_VS_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_VS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_VS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_REG =
{
    "FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL",
#if RU_INCLUDE_DESC
    "DBG_MUX_SEL Register",
    "selects the debug vector.\nFirst 32 addresses dedicated to cmd fifo, and from 0x40 debug of internal signals.\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_REG_OFFSET },
    0,
    0,
    331,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS, TYPE: Type_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VB *****/
const ru_field_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_VB_FIELD =
{
    "VB",
#if RU_INCLUDE_DESC
    "",
    "debug vector\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_VB_FIELD_MASK },
    0,
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_VB_FIELD_WIDTH },
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_VB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_FIELDS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_VB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS *****/
const ru_reg_rec DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_REG =
{
    "FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS",
#if RU_INCLUDE_DESC
    "DBG_BUS Register",
    "the debug bus\n",
#endif
    { DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_REG_OFFSET },
    0,
    0,
    332,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_FIELDS,
#endif
};

unsigned long DQM_FPMINI_ADDRS[] =
{
    0x82C40000,
};

static const ru_reg_rec *DQM_FPMINI_REGS[] =
{
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_2_MEM_L2_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_1_REGS_L1_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_LVL_0_REG_L0_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_L2_INIT_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_CFG0_ALLC_FAST_ACK_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_FREE_ERR_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_PRFM_CNTRS_GEN_CFG_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGSEL_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMINI_DEBUG_DBGBUS_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_MC_MEM_MC_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_MC_INIT_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_CFG0_FREE_BP_MC_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_INC_ERR_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_PRFM_CNTRS_MC_DEC_ERR_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGSEL_REG,
    &DQM_FPMINI_FPMINI_BLOCK_FPMCAST_DEBUG_DBGBUS_REG,
};

const ru_block_rec DQM_FPMINI_BLOCK =
{
    "DQM_FPMINI",
    DQM_FPMINI_ADDRS,
    1,
    18,
    DQM_FPMINI_REGS,
};
