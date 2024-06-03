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


#include "XRDP_BUFMNG_AG.h"

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_ORDR, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_ORDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NXTLVL *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_ORDR_NXTLVL_FIELD =
{
    "NXTLVL",
#if RU_INCLUDE_DESC
    "",
    "the address is counter\nbits[ 5:0] are next-level after this counter.\nIf msb=1, then nxt_level is not valid\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_ORDR_NXTLVL_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_ORDR_NXTLVL_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_ORDR_NXTLVL_FIELD_SHIFT },
    63,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_ORDR_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_ORDR_NXTLVL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_ORDR *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_ORDR_REG =
{
    "COUNTERS_CFG_STAT_ORDR",
#if RU_INCLUDE_DESC
    "CNTRS_ORDER 0..31 Register",
    "order of counters for buffer management engine.\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_ORDR_REG_OFFSET },
    BUFMNG_COUNTERS_CFG_STAT_ORDR_REG_RAM_CNT,
    4,
    208,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_ORDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_RSRV_THR, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_RSRV_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THR *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_THR_FIELD =
{
    "THR",
#if RU_INCLUDE_DESC
    "",
    "threshold\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_THR_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_THR_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_THR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_RSRV_THR *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_REG =
{
    "COUNTERS_CFG_STAT_RSRV_THR",
#if RU_INCLUDE_DESC
    "RSRV_THR 0..31 Register",
    "reserved threshold\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_REG_OFFSET },
    BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_REG_RAM_CNT,
    4,
    209,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_HIPRI_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THR *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_THR_FIELD =
{
    "THR",
#if RU_INCLUDE_DESC
    "",
    "threshold\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_THR_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_THR_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_THR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_REG =
{
    "COUNTERS_CFG_STAT_HIPRI_THR",
#if RU_INCLUDE_DESC
    "HIPRI_THR 0..31 Register",
    "high priority threshold\n* Note - Similar to configuration of UG thresholds in flow control mode, when using BufMgr based FC, the thr value should be:\nbmgr_high_thr - Hysteresis  >= max_tokens_per_packet * 2\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_REG_OFFSET },
    BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_REG_RAM_CNT,
    4,
    210,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_MAX_THR, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_MAX_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THR *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_MAX_THR_THR_FIELD =
{
    "THR",
#if RU_INCLUDE_DESC
    "",
    "threshold\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_MAX_THR_THR_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_MAX_THR_THR_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_MAX_THR_THR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_MAX_THR_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_MAX_THR_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_MAX_THR *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_MAX_THR_REG =
{
    "COUNTERS_CFG_STAT_MAX_THR",
#if RU_INCLUDE_DESC
    "MNG_MAX_THR 0..31 Register",
    "max threshold\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_MAX_THR_REG_OFFSET },
    BUFMNG_COUNTERS_CFG_STAT_MAX_THR_REG_RAM_CNT,
    4,
    211,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_MAX_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_HI_WMRK_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CTR0 *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR0_FIELD =
{
    "CTR0",
#if RU_INCLUDE_DESC
    "",
    "counter to get its high-watermark\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR0_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR0_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CTR1 *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR1_FIELD =
{
    "CTR1",
#if RU_INCLUDE_DESC
    "",
    "counter to get its high-watermark\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR1_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR1_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CTR2 *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR2_FIELD =
{
    "CTR2",
#if RU_INCLUDE_DESC
    "",
    "counter to get its high-watermark\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR2_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR2_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR2_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR0_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR1_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_CTR2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_REG =
{
    "COUNTERS_CFG_STAT_HI_WMRK_CFG",
#if RU_INCLUDE_DESC
    "HI_WATERMARK_CFG Register",
    "3 counter numbers to get the high watermark of.\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_REG_OFFSET },
    0,
    0,
    212,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_HI_WMRK_VAL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_VAL_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_VAL_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_REG =
{
    "COUNTERS_CFG_STAT_HI_WMRK_VAL",
#if RU_INCLUDE_DESC
    "HI_WATERMARK_VAL 0..2 Register",
    "hi-watermark counters  values\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_REG_OFFSET },
    BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_REG_RAM_CNT,
    4,
    213,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_CNTR_INIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IDX *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_IDX_FIELD =
{
    "IDX",
#if RU_INCLUDE_DESC
    "",
    "counter index\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_IDX_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_IDX_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_IDX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "counter value\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_VAL_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_VAL_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_IDX_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_REG =
{
    "COUNTERS_CFG_STAT_CNTR_INIT",
#if RU_INCLUDE_DESC
    "CNTR_INIT Register",
    "initialize counter with a value, and also the hi_wmr to the same value (if the index in amongst the 3 indexes of hi_wmr)\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_REG_OFFSET },
    0,
    0,
    214,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_CNTR_NEG_ST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "If bit=1, the corresponding counter got a wrong decrement command that was supposed to turn it negative.\nSticky, until cleared with register cntr_neg_stat_clr.\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_VAL_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_VAL_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_REG =
{
    "COUNTERS_CFG_STAT_CNTR_NEG_ST",
#if RU_INCLUDE_DESC
    "CNTR_NEG_STAT Register",
    "32b vector.\nIf bit=1, the corresponding counter got a wrong decrement command that was supposed to turn it negative.\nSticky, until cleared with register cntr_neg_stat_clr\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_REG_OFFSET },
    0,
    0,
    215,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "if bit=1, it clears corresponding bit in cntr_neg_st register\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_VAL_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_VAL_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_REG =
{
    "COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR",
#if RU_INCLUDE_DESC
    "CNTR_NEG_STAT_CLR Register",
    "32b vector.\nIf bit=1, it clears corresponding bit in cntr_neg_stat register\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_REG_OFFSET },
    0,
    0,
    216,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_CAPT_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MOD *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_MOD_FIELD =
{
    "MOD",
#if RU_INCLUDE_DESC
    "",
    "mode\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_MOD_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_MOD_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_MOD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_MOD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_REG =
{
    "COUNTERS_CFG_STAT_CAPT_CFG",
#if RU_INCLUDE_DESC
    "CAPTURE_CONFIG Register",
    "capture configuration\n0: capture last negative cmd/cntrs\n1: capture last cmd/cntrs\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_REG_OFFSET },
    0,
    0,
    217,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IDX *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_IDX_FIELD =
{
    "IDX",
#if RU_INCLUDE_DESC
    "",
    "counter index\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_IDX_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_IDX_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_IDX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_VAL_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_VAL_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_IDX_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_REG =
{
    "COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT",
#if RU_INCLUDE_DESC
    "CNTR_NEG_CAPT_CNTR 0..2 Register",
    "3 counters capturing values of  counters before last corrupt command (supposed to turn one of these counters negative).\norder:\n0:lvl0\n4:lvl1\n8:lvl2\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_REG_OFFSET },
    BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_REG_RAM_CNT,
    4,
    218,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IDX *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_IDX_FIELD =
{
    "IDX",
#if RU_INCLUDE_DESC
    "",
    "command index inside cnpl (pointer to bacif array)\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_IDX_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_IDX_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_IDX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "",
    "copy of command as seen on bb_data[31:9].\nThis can be compare to bacif debug info.\nbit 63:60 are always 4h4 for this command, other params[59:41] should reflect bb_data[27:9] (bit 8 is reserved).\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_CMD_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_CMD_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_CMD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_IDX_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_CMD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_REG =
{
    "COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD",
#if RU_INCLUDE_DESC
    "CNTR_NEG_CAPT_CMD Register",
    "last corrupt command (supposed to turn one of the counters negative) or last command. Selection between the 2 modes is done through configuration register - capture_config.\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_REG_OFFSET },
    0,
    0,
    219,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_POOLS_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL0_SIZE *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL0_SIZE_FIELD =
{
    "POOL0_SIZE",
#if RU_INCLUDE_DESC
    "",
    "size of pool0 in tokens.\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL0_SIZE_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL0_SIZE_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL0_SIZE_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL1_SIZE *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL1_SIZE_FIELD =
{
    "POOL1_SIZE",
#if RU_INCLUDE_DESC
    "",
    "size of pool1 in tokens.\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL1_SIZE_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL1_SIZE_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL1_SIZE_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL2_SIZE *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL2_SIZE_FIELD =
{
    "POOL2_SIZE",
#if RU_INCLUDE_DESC
    "",
    "size of pool2 in tokens.\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL2_SIZE_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL2_SIZE_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL2_SIZE_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL3_SIZE *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL3_SIZE_FIELD =
{
    "POOL3_SIZE",
#if RU_INCLUDE_DESC
    "",
    "size of pool3 in tokens.\n\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL3_SIZE_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL3_SIZE_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL3_SIZE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL0_SIZE_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL1_SIZE_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL2_SIZE_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_POOL3_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_REG =
{
    "COUNTERS_CFG_STAT_POOLS_SIZE",
#if RU_INCLUDE_DESC
    "POOLS_SIZE Register",
    "4 cfgs to indicate how many tokens to inc/dec for each of the 4 pools. (enabled by 1b in bb message)\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_REG_OFFSET },
    0,
    0,
    220,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_MISC, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_MISC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NEG_EN *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_MISC_NEG_EN_FIELD =
{
    "NEG_EN",
#if RU_INCLUDE_DESC
    "",
    "negative value enable:\n1: decrement can reach negative values of counter(bit 19 in counter[19:0])\n0: decrement not allowed to reach negative values of counter\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_MISC_NEG_EN_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_MISC_NEG_EN_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_MISC_NEG_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FC_HYST *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_MISC_FC_HYST_FIELD =
{
    "FC_HYST",
#if RU_INCLUDE_DESC
    "",
    "fc hysteresis (resolution of 16).\nused as configuration for following feature:\nfc vector of 32 bits (1 per counter) with the following definition:\n1.\tIf  0:\nFor each INC or DEC, the HW checks if the counter >= high_prio_thr (unique for each counter).\nIf yes, asserts the relevant bit in the vector\n2.\tIf 1:\nFor each INC or DEC, the HW checks if the counter <= high_prio_thr - 16*fc_hyst.\nIf yes, de-asserts the relevant bit\n\nfc_hyst must be > 0.\n* And also, when using BufMgr based FC it should comply with:\nbmgr_high_thr - Hysteresis  >= max_tokens_per_packet * 2\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_MISC_FC_HYST_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_MISC_FC_HYST_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_MISC_FC_HYST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_MISC_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_MISC_NEG_EN_FIELD,
    &BUFMNG_COUNTERS_CFG_STAT_MISC_FC_HYST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_MISC *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_MISC_REG =
{
    "COUNTERS_CFG_STAT_MISC",
#if RU_INCLUDE_DESC
    "MISC Register",
    "general configurations:\nneg_en, fc_hyst\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_MISC_REG_OFFSET },
    0,
    0,
    221,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BUFMNG_COUNTERS_CFG_STAT_MISC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_FC_ST_VEC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_VAL_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_VAL_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_REG =
{
    "COUNTERS_CFG_STAT_FC_ST_VEC",
#if RU_INCLUDE_DESC
    "FC_STATUS_VEC Register",
    "32b flow control status vector\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_REG_OFFSET },
    0,
    0,
    222,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL, TYPE: Type_BUFMNG_BLOCK_COUNTERS_CFG_STAT_CTRS_VAL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_VAL_FIELD_MASK },
    0,
    { BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_VAL_FIELD_WIDTH },
    { BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_FIELDS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL *****/
const ru_reg_rec BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_REG =
{
    "COUNTERS_CFG_STAT_CTRS_VAL",
#if RU_INCLUDE_DESC
    "CNTRS_VAL 0..31 Register",
    "counters current values\n",
#endif
    { BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_REG_OFFSET },
    BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_REG_RAM_CNT,
    4,
    223,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_FIELDS,
#endif
};

unsigned long BUFMNG_ADDRS[] =
{
    0x82C60000,
};

static const ru_reg_rec *BUFMNG_REGS[] =
{
    &BUFMNG_COUNTERS_CFG_STAT_ORDR_REG,
    &BUFMNG_COUNTERS_CFG_STAT_RSRV_THR_REG,
    &BUFMNG_COUNTERS_CFG_STAT_HIPRI_THR_REG,
    &BUFMNG_COUNTERS_CFG_STAT_MAX_THR_REG,
    &BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_CFG_REG,
    &BUFMNG_COUNTERS_CFG_STAT_HI_WMRK_VAL_REG,
    &BUFMNG_COUNTERS_CFG_STAT_CNTR_INIT_REG,
    &BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_REG,
    &BUFMNG_COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR_REG,
    &BUFMNG_COUNTERS_CFG_STAT_CAPT_CFG_REG,
    &BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT_REG,
    &BUFMNG_COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD_REG,
    &BUFMNG_COUNTERS_CFG_STAT_POOLS_SIZE_REG,
    &BUFMNG_COUNTERS_CFG_STAT_MISC_REG,
    &BUFMNG_COUNTERS_CFG_STAT_FC_ST_VEC_REG,
    &BUFMNG_COUNTERS_CFG_STAT_CTRS_VAL_REG,
};

const ru_block_rec BUFMNG_BLOCK =
{
    "BUFMNG",
    BUFMNG_ADDRS,
    1,
    16,
    BUFMNG_REGS,
};
