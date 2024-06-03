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


#include "XRDP_RNR_REGS_AG.h"

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_GLOBAL_CTRL, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_GLOBAL_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "Runner enable. When reset runner pipe is halted, instruction memory and context memory can be accessed by the CPU. The CPU can reset or set this bit\nThe firmware can reset this bit by writing to the disable bit at the runner I/O control register.\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_ILLEGAL_STATUS *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD =
{
    "DMA_ILLEGAL_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Notifies about DMA illegal access (>16 cycles on UBUS). Sticky bit. cleared by writing 1 to this bit.\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PREDICTION_OVERRUN_STATUS *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD =
{
    "PREDICTION_OVERRUN_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Notifies about prediction FIFO overwrite status. Sticky bit. cleared by writing 1 to this bit.\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GDMA_CHKSUM_DEST_ILLEGAL *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_DEST_ILLEGAL_FIELD =
{
    "GDMA_CHKSUM_DEST_ILLEGAL",
#if RU_INCLUDE_DESC
    "",
    "GDMA checksum write back destination if out of bounds. checksum desc is ignored.\nSticky bit. cleared by writing 1 to this bit.\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_DEST_ILLEGAL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_DEST_ILLEGAL_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_DEST_ILLEGAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GDMA_CHKSUM_LEN_ILLEGAL *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_LEN_ILLEGAL_FIELD =
{
    "GDMA_CHKSUM_LEN_ILLEGAL",
#if RU_INCLUDE_DESC
    "",
    "GDMA checksum length is out of bounds. Checksum desc is ignored.\nSticky bit. cleared by writing 1 to this bit.\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_LEN_ILLEGAL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_LEN_ILLEGAL_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_LEN_ILLEGAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GDMA_TOTAL_DESCS_ILLEGAL *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_GDMA_TOTAL_DESCS_ILLEGAL_FIELD =
{
    "GDMA_TOTAL_DESCS_ILLEGAL",
#if RU_INCLUDE_DESC
    "",
    "Total length of GDMA descriptors exceeds configured value of 32B or 64B.\nThe data is written according to all descs which are bounded in 32B or 64B. The crossing descriptor is ignored.\nSticky bit. cleared by writing 1 to this bit.\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_TOTAL_DESCS_ILLEGAL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_TOTAL_DESCS_ILLEGAL_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_TOTAL_DESCS_ILLEGAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GDMA_INCOMPLETE_ILLEGAL *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_GDMA_INCOMPLETE_ILLEGAL_FIELD =
{
    "GDMA_INCOMPLETE_ILLEGAL",
#if RU_INCLUDE_DESC
    "",
    "Total byte count according to all read descs is less than GDMA command BC.\nGDMA writes all the data which is described by the descriptors.\nSticky bit. cleared by writing 1 to this bit.\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_INCOMPLETE_ILLEGAL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_INCOMPLETE_ILLEGAL_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_INCOMPLETE_ILLEGAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GDMA_CHKSUM_OVERLAP_ILLEGAL *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_OVERLAP_ILLEGAL_FIELD =
{
    "GDMA_CHKSUM_OVERLAP_ILLEGAL",
#if RU_INCLUDE_DESC
    "",
    "GDMA checksum length or destination overlaps with another checksum descriptor. The second checksum desc is ignored.\nSticky bit. cleared by writing 1 to this bit.\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_OVERLAP_ILLEGAL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_OVERLAP_ILLEGAL_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_OVERLAP_ILLEGAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MICRO_SEC_VAL *****/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD =
{
    "MICRO_SEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD_WIDTH },
    { RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD_SHIFT },
    350,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_GLOBAL_CTRL_FIELDS[] =
{
    &RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_DEST_ILLEGAL_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_LEN_ILLEGAL_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_GDMA_TOTAL_DESCS_ILLEGAL_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_GDMA_INCOMPLETE_ILLEGAL_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_GDMA_CHKSUM_OVERLAP_ILLEGAL_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_GLOBAL_CTRL *****/
const ru_reg_rec RNR_REGS_CFG_GLOBAL_CTRL_REG =
{
    "CFG_GLOBAL_CTRL",
#if RU_INCLUDE_DESC
    "GLOBAL_CONTROL Register",
    "Global control\n",
#endif
    { RNR_REGS_CFG_GLOBAL_CTRL_REG_OFFSET },
    0,
    0,
    963,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_REGS_CFG_GLOBAL_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_CPU_WAKEUP, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_CPU_WAKEUP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD_NUM *****/
const ru_field_rec RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD =
{
    "THREAD_NUM",
#if RU_INCLUDE_DESC
    "",
    "The thread number to be invoked by the CPU.\n",
#endif
    { RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD_MASK },
    0,
    { RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD_WIDTH },
    { RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_CPU_WAKEUP_FIELDS[] =
{
    &RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_CPU_WAKEUP *****/
const ru_reg_rec RNR_REGS_CFG_CPU_WAKEUP_REG =
{
    "CFG_CPU_WAKEUP",
#if RU_INCLUDE_DESC
    "CPU_WAKEUP Register",
    "Writing to this register generates a request towards the runner scheduler.\n",
#endif
    { RNR_REGS_CFG_CPU_WAKEUP_REG_OFFSET },
    0,
    0,
    964,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_CPU_WAKEUP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_INT_CTRL, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_INT_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INT0_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD =
{
    "INT0_STS",
#if RU_INCLUDE_DESC
    "",
    "While any of this field bits is set interrupt line 0 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT1_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD =
{
    "INT1_STS",
#if RU_INCLUDE_DESC
    "",
    "While any of this field bits is set interrupt line 0 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT2_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD =
{
    "INT2_STS",
#if RU_INCLUDE_DESC
    "",
    "While this bit is set interrupt line 2 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT3_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD =
{
    "INT3_STS",
#if RU_INCLUDE_DESC
    "",
    "While this bit is set interrupt line 3 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT4_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD =
{
    "INT4_STS",
#if RU_INCLUDE_DESC
    "",
    "While this bit is set interrupt line 4 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT5_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD =
{
    "INT5_STS",
#if RU_INCLUDE_DESC
    "",
    "While this bit is set interrupt line 5 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT6_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD =
{
    "INT6_STS",
#if RU_INCLUDE_DESC
    "",
    "While this bit is set interrupt line 6 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT7_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD =
{
    "INT7_STS",
#if RU_INCLUDE_DESC
    "",
    "While this bit is set interrupt line 6 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT8_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD =
{
    "INT8_STS",
#if RU_INCLUDE_DESC
    "",
    "While this bit is set interrupt line 8 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT9_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD =
{
    "INT9_STS",
#if RU_INCLUDE_DESC
    "",
    "While this bit is set interrupt line 9 is set. SW can write '1' to clear any bit. Write of '0' is ignored.\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIT_FAIL_STS *****/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD =
{
    "FIT_FAIL_STS",
#if RU_INCLUDE_DESC
    "",
    "\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_INT_CTRL_FIELDS[] =
{
    &RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD,
    &RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_INT_CTRL *****/
const ru_reg_rec RNR_REGS_CFG_INT_CTRL_REG =
{
    "CFG_INT_CTRL",
#if RU_INCLUDE_DESC
    "INTERRUPT_CONTROL Register",
    "Interrupt control - UNUSED in 6858\n",
#endif
    { RNR_REGS_CFG_INT_CTRL_REG_OFFSET },
    0,
    0,
    965,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    RNR_REGS_CFG_INT_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_INT_MASK, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_INT_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INT0_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD =
{
    "INT0_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask INT0 causes\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT1_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD =
{
    "INT1_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT1 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT2_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD =
{
    "INT2_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT2 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT3_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD =
{
    "INT3_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT3 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT4_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD =
{
    "INT4_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT4 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT5_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD =
{
    "INT5_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT5 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT6_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD =
{
    "INT6_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT6 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT7_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD =
{
    "INT7_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT7 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT8_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD =
{
    "INT8_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT8 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT9_MASK *****/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD =
{
    "INT9_MASK",
#if RU_INCLUDE_DESC
    "",
    "INT9 mask cause\n",
#endif
    { RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD_WIDTH },
    { RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_INT_MASK_FIELDS[] =
{
    &RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD,
    &RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_INT_MASK *****/
const ru_reg_rec RNR_REGS_CFG_INT_MASK_REG =
{
    "CFG_INT_MASK",
#if RU_INCLUDE_DESC
    "INTERRUPT_MASK Register",
    "Interrupt mask -  UNUSED in 6858\n",
#endif
    { RNR_REGS_CFG_INT_MASK_REG_OFFSET },
    0,
    0,
    966,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    RNR_REGS_CFG_INT_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_GEN_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_GEN_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DISABLE_DMA_OLD_FLOW_CONTROL *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD =
{
    "DISABLE_DMA_OLD_FLOW_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "Disable DMA old flow control. When set to 1, DMA will not check read FIFO occupancy when issuing READ requests, relying instead on DMA backpressure mechanism vs read dispatcher block.\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TEST_FIT_FAIL *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD =
{
    "TEST_FIT_FAIL",
#if RU_INCLUDE_DESC
    "",
    "set to 1 to test fit fail interrupt.\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZERO_DATA_MEM *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_FIELD =
{
    "ZERO_DATA_MEM",
#if RU_INCLUDE_DESC
    "",
    "Trigger self-zeroing mechanism for data memory.\n\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZERO_CONTEXT_MEM *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_FIELD =
{
    "ZERO_CONTEXT_MEM",
#if RU_INCLUDE_DESC
    "",
    "Trigger self-zeroing mechanism for context memory.\n\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZERO_DATA_MEM_DONE *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_DONE_FIELD =
{
    "ZERO_DATA_MEM_DONE",
#if RU_INCLUDE_DESC
    "",
    "Goes high when zeroing is done. Reset to 0 when config ZERO_DATA_MEM is set to 1\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_DONE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_DONE_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ZERO_CONTEXT_MEM_DONE *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_DONE_FIELD =
{
    "ZERO_CONTEXT_MEM_DONE",
#if RU_INCLUDE_DESC
    "",
    "Goes high when zeroing is done. Reset to 0 when config ZERO_CONTEXT_MEM is set to 1\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_DONE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_DONE_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHICKEN_DISABLE_SKIP_JMP *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_SKIP_JMP_FIELD =
{
    "CHICKEN_DISABLE_SKIP_JMP",
#if RU_INCLUDE_DESC
    "",
    "When set to 1, skip jump functionality is disabled\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_SKIP_JMP_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_SKIP_JMP_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_SKIP_JMP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHICKEN_DISABLE_ALU_LOAD_BALANCING *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_ALU_LOAD_BALANCING_FIELD =
{
    "CHICKEN_DISABLE_ALU_LOAD_BALANCING",
#if RU_INCLUDE_DESC
    "",
    "When set to 1, ALU load balancing functionality is disabled\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_ALU_LOAD_BALANCING_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_ALU_LOAD_BALANCING_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_ALU_LOAD_BALANCING_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GDMA_DESC_OFFSET *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_GDMA_DESC_OFFSET_FIELD =
{
    "GDMA_DESC_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Configure descriptor offset for GATHER DMA command\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_GDMA_DESC_OFFSET_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_GDMA_DESC_OFFSET_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_GDMA_DESC_OFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBTX_TCAM_DEST_SEL *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_BBTX_TCAM_DEST_SEL_FIELD =
{
    "BBTX_TCAM_DEST_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select destination TCAM for Runner\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_BBTX_TCAM_DEST_SEL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_BBTX_TCAM_DEST_SEL_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_BBTX_TCAM_DEST_SEL_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBTX_HASH_DEST_SEL *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_BBTX_HASH_DEST_SEL_FIELD =
{
    "BBTX_HASH_DEST_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select destination HASH for Runner\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_BBTX_HASH_DEST_SEL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_BBTX_HASH_DEST_SEL_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_BBTX_HASH_DEST_SEL_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBTX_NATC_DEST_SEL *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_BBTX_NATC_DEST_SEL_FIELD =
{
    "BBTX_NATC_DEST_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select destination NATC for Runner\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_BBTX_NATC_DEST_SEL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_BBTX_NATC_DEST_SEL_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_BBTX_NATC_DEST_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBTX_CNPL_DEST_SEL *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_BBTX_CNPL_DEST_SEL_FIELD =
{
    "BBTX_CNPL_DEST_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select destination CNPL for Runner\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_BBTX_CNPL_DEST_SEL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_BBTX_CNPL_DEST_SEL_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_BBTX_CNPL_DEST_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GDMA_GDESC_BUFFER_SIZE *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_GDMA_GDESC_BUFFER_SIZE_FIELD =
{
    "GDMA_GDESC_BUFFER_SIZE",
#if RU_INCLUDE_DESC
    "",
    "0 - GDMA prefetches 32B when reading descriptors from the data memory\n1 - GDMA prefetches 64B when reading descriptors from the data memory\n\nif 0 is set then total number of gather descriptors per GDMA command isnt allowed to exceed 32B.\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_GDMA_GDESC_BUFFER_SIZE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_GDMA_GDESC_BUFFER_SIZE_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_GDMA_GDESC_BUFFER_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE_FIELD =
{
    "CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE",
#if RU_INCLUDE_DESC
    "",
    "When set to 1, use old mode for unique ID assignment\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHICKEN_ENABLE_DMA_OLD_MODE *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_DMA_OLD_MODE_FIELD =
{
    "CHICKEN_ENABLE_DMA_OLD_MODE",
#if RU_INCLUDE_DESC
    "",
    "When set to 1, use old mode for 40 bit address calculation\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_DMA_OLD_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_DMA_OLD_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_DMA_OLD_MODE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PREVENT_CS_TILL_STORES_DONE *****/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_PREVENT_CS_TILL_STORES_DONE_FIELD =
{
    "PREVENT_CS_TILL_STORES_DONE",
#if RU_INCLUDE_DESC
    "",
    "When set to 1, prevents context swap until all STOREs reach meeory\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_PREVENT_CS_TILL_STORES_DONE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_GEN_CFG_PREVENT_CS_TILL_STORES_DONE_FIELD_WIDTH },
    { RNR_REGS_CFG_GEN_CFG_PREVENT_CS_TILL_STORES_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_GEN_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD,
    &RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD,
    &RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_FIELD,
    &RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_FIELD,
    &RNR_REGS_CFG_GEN_CFG_ZERO_DATA_MEM_DONE_FIELD,
    &RNR_REGS_CFG_GEN_CFG_ZERO_CONTEXT_MEM_DONE_FIELD,
    &RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_SKIP_JMP_FIELD,
    &RNR_REGS_CFG_GEN_CFG_CHICKEN_DISABLE_ALU_LOAD_BALANCING_FIELD,
    &RNR_REGS_CFG_GEN_CFG_GDMA_DESC_OFFSET_FIELD,
    &RNR_REGS_CFG_GEN_CFG_BBTX_TCAM_DEST_SEL_FIELD,
    &RNR_REGS_CFG_GEN_CFG_BBTX_HASH_DEST_SEL_FIELD,
    &RNR_REGS_CFG_GEN_CFG_BBTX_NATC_DEST_SEL_FIELD,
    &RNR_REGS_CFG_GEN_CFG_BBTX_CNPL_DEST_SEL_FIELD,
    &RNR_REGS_CFG_GEN_CFG_GDMA_GDESC_BUFFER_SIZE_FIELD,
    &RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE_FIELD,
    &RNR_REGS_CFG_GEN_CFG_CHICKEN_ENABLE_DMA_OLD_MODE_FIELD,
    &RNR_REGS_CFG_GEN_CFG_PREVENT_CS_TILL_STORES_DONE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_GEN_CFG *****/
const ru_reg_rec RNR_REGS_CFG_GEN_CFG_REG =
{
    "CFG_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIGURATION Register",
    "General configuration\n",
#endif
    { RNR_REGS_CFG_GEN_CFG_REG_OFFSET },
    0,
    0,
    967,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    17,
    RNR_REGS_CFG_GEN_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_CAM_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_CAM_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STOP_VALUE *****/
const ru_field_rec RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD =
{
    "STOP_VALUE",
#if RU_INCLUDE_DESC
    "",
    "CAM operation is stopped when reaching an entry with a value matching this field.\nFor a 32-bit or 64-bit CAM entries, this value is concatenated.\n",
#endif
    { RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD_WIDTH },
    { RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_CAM_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_CAM_CFG *****/
const ru_reg_rec RNR_REGS_CFG_CAM_CFG_REG =
{
    "CFG_CAM_CFG",
#if RU_INCLUDE_DESC
    "CAM_CONFIGURATION Register",
    "CAM configuration\n",
#endif
    { RNR_REGS_CFG_CAM_CFG_REG_OFFSET },
    0,
    0,
    968,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_CAM_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_FPM_MINI_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_FPM_MINI_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_BASE *****/
const ru_field_rec RNR_REGS_CFG_FPM_MINI_CFG_DMA_BASE_FIELD =
{
    "DMA_BASE",
#if RU_INCLUDE_DESC
    "",
    "DMA base address for ADDR_CALC\n",
#endif
    { RNR_REGS_CFG_FPM_MINI_CFG_DMA_BASE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_FPM_MINI_CFG_DMA_BASE_FIELD_WIDTH },
    { RNR_REGS_CFG_FPM_MINI_CFG_DMA_BASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_STATIC_OFFSET *****/
const ru_field_rec RNR_REGS_CFG_FPM_MINI_CFG_DMA_STATIC_OFFSET_FIELD =
{
    "DMA_STATIC_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "DMA static offset\n",
#endif
    { RNR_REGS_CFG_FPM_MINI_CFG_DMA_STATIC_OFFSET_FIELD_MASK },
    0,
    { RNR_REGS_CFG_FPM_MINI_CFG_DMA_STATIC_OFFSET_FIELD_WIDTH },
    { RNR_REGS_CFG_FPM_MINI_CFG_DMA_STATIC_OFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_FPM_MINI_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_FPM_MINI_CFG_DMA_BASE_FIELD,
    &RNR_REGS_CFG_FPM_MINI_CFG_DMA_STATIC_OFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_FPM_MINI_CFG *****/
const ru_reg_rec RNR_REGS_CFG_FPM_MINI_CFG_REG =
{
    "CFG_FPM_MINI_CFG",
#if RU_INCLUDE_DESC
    "DMA_FPM_MINI_CONFIG Register",
    "DMA FPM mini config Register. Contains configurations such as buffer size and base address that are used for DDR address calculations (from buffer number) when DMA instruction addr_calc flag is set.\n",
#endif
    { RNR_REGS_CFG_FPM_MINI_CFG_REG_OFFSET },
    0,
    0,
    969,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_FPM_MINI_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_DDR_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_DDR_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_BASE *****/
const ru_field_rec RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD =
{
    "DMA_BASE",
#if RU_INCLUDE_DESC
    "",
    "DMA base address for ADDR_CALC\n",
#endif
    { RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD_WIDTH },
    { RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_BUF_SIZE *****/
const ru_field_rec RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD =
{
    "DMA_BUF_SIZE",
#if RU_INCLUDE_DESC
    "",
    "3 bits indicating buffer size\n",
#endif
    { RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD_WIDTH },
    { RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_BUF_SIZE_MODE *****/
const ru_field_rec RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_MODE_FIELD =
{
    "DMA_BUF_SIZE_MODE",
#if RU_INCLUDE_DESC
    "",
    "Determine old buffer  mode or new mode.\nOld mode: 128, 256, 512, 1024, 2048\nNew mode: 128, 320, 640, 1280, 2560\n",
#endif
    { RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_STATIC_OFFSET *****/
const ru_field_rec RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD =
{
    "DMA_STATIC_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "DMA static offset\n",
#endif
    { RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD_WIDTH },
    { RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_DDR_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD,
    &RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD,
    &RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_MODE_FIELD,
    &RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_DDR_CFG *****/
const ru_reg_rec RNR_REGS_CFG_DDR_CFG_REG =
{
    "CFG_DDR_CFG",
#if RU_INCLUDE_DESC
    "DMA_DDR_CONFIG Register",
    "DMA DDR config Register. Contains configurations such as buffer size and ddr base address that are used for DDR address calculations (from buffer number) when DMA instruction addr_calc flag is set.\n",
#endif
    { RNR_REGS_CFG_DDR_CFG_REG_OFFSET },
    0,
    0,
    970,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_DDR_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_PSRAM_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_PSRAM_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_BASE *****/
const ru_field_rec RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD =
{
    "DMA_BASE",
#if RU_INCLUDE_DESC
    "",
    "DMA base address for ADDR_CALC\n",
#endif
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD_WIDTH },
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_BUF_SIZE *****/
const ru_field_rec RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD =
{
    "DMA_BUF_SIZE",
#if RU_INCLUDE_DESC
    "",
    "3 bits indicating buffer size\n",
#endif
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD_WIDTH },
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_BUF_SIZE_MODE *****/
const ru_field_rec RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_MODE_FIELD =
{
    "DMA_BUF_SIZE_MODE",
#if RU_INCLUDE_DESC
    "",
    "Determine old buffer  mode or new mode\nOld mode: 128, 256, 512, 1024, 2048\nNew mode: 128, 320, 640, 1280, 2560\n",
#endif
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DMA_STATIC_OFFSET *****/
const ru_field_rec RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD =
{
    "DMA_STATIC_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "DMA static offset\n",
#endif
    { RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD_WIDTH },
    { RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PSRAM_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD,
    &RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD,
    &RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_MODE_FIELD,
    &RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_PSRAM_CFG *****/
const ru_reg_rec RNR_REGS_CFG_PSRAM_CFG_REG =
{
    "CFG_PSRAM_CFG",
#if RU_INCLUDE_DESC
    "DMA_PSRAM_CONFIG Register",
    "DMA PSRAM config Register. Contains configurations such as buffer size and ddr base address that are used for DDR address calculations (from buffer number) when DMA instruction addr_calc flag is set.\n",
#endif
    { RNR_REGS_CFG_PSRAM_CFG_REG_OFFSET },
    0,
    0,
    971,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_PSRAM_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_RAMRD_RANGE_MASK_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK0 *****/
const ru_field_rec RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD =
{
    "MASK0",
#if RU_INCLUDE_DESC
    "",
    "Mask 0 for range serach. according to the number of 1 in the mask the cam machine can differ between the Key and TAG\n",
#endif
    { RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD_MASK },
    0,
    { RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD_WIDTH },
    { RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK1 *****/
const ru_field_rec RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD =
{
    "MASK1",
#if RU_INCLUDE_DESC
    "",
    "Mask 0 for range serach. according to the number of 1 in the mask the cam machine can differ between the Key and TAG\n",
#endif
    { RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD_MASK },
    0,
    { RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD_WIDTH },
    { RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD,
    &RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG *****/
const ru_reg_rec RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG =
{
    "CFG_RAMRD_RANGE_MASK_CFG",
#if RU_INCLUDE_DESC
    "RAMRD_MASK_CONFIG Register",
    "Ramrd mask for range search. The register holds 2 mask that can be chosen by runner core for range seraches.\n",
#endif
    { RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG_OFFSET },
    0,
    0,
    972,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_SCH_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_SCH_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SCHEDULER_MODE *****/
const ru_field_rec RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD =
{
    "SCHEDULER_MODE",
#if RU_INCLUDE_DESC
    "",
    "Configure priority mode for scheduler operation\n",
#endif
    { RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_SCH_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_SCH_CFG *****/
const ru_reg_rec RNR_REGS_CFG_SCH_CFG_REG =
{
    "CFG_SCH_CFG",
#if RU_INCLUDE_DESC
    "SCHEDULER_CONFIG Register",
    "scheduler configuration\n",
#endif
    { RNR_REGS_CFG_SCH_CFG_REG_OFFSET },
    0,
    0,
    973,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_SCH_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_BKPT_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_BKPT_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_0_EN *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD =
{
    "BKPT_0_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 0\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_0_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD =
{
    "BKPT_0_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_1_EN *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD =
{
    "BKPT_1_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 1\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_1_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD =
{
    "BKPT_1_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_2_EN *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD =
{
    "BKPT_2_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 2\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_2_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD =
{
    "BKPT_2_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_3_EN *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD =
{
    "BKPT_3_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 3\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_3_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD =
{
    "BKPT_3_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_4_EN *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD =
{
    "BKPT_4_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 4\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_4_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD =
{
    "BKPT_4_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_5_EN *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD =
{
    "BKPT_5_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 5\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_5_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD =
{
    "BKPT_5_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_6_EN *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD =
{
    "BKPT_6_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 6\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_6_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD =
{
    "BKPT_6_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_7_EN *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD =
{
    "BKPT_7_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 7\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_7_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD =
{
    "BKPT_7_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STEP_MODE *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD =
{
    "STEP_MODE",
#if RU_INCLUDE_DESC
    "",
    "Configure step mode\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NEW_FLAGS_VAL *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD =
{
    "NEW_FLAGS_VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for new flags\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_BREAKPOINT_ON_FIT_FAIL *****/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_ENABLE_BREAKPOINT_ON_FIT_FAIL_FIELD =
{
    "ENABLE_BREAKPOINT_ON_FIT_FAIL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1, Runner will break on fit_fail\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_ENABLE_BREAKPOINT_ON_FIT_FAIL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_CFG_ENABLE_BREAKPOINT_ON_FIT_FAIL_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_CFG_ENABLE_BREAKPOINT_ON_FIT_FAIL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_BKPT_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_ENABLE_BREAKPOINT_ON_FIT_FAIL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_BKPT_CFG *****/
const ru_reg_rec RNR_REGS_CFG_BKPT_CFG_REG =
{
    "CFG_BKPT_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG Register",
    "breakpoint configuration\n",
#endif
    { RNR_REGS_CFG_BKPT_CFG_REG_OFFSET },
    0,
    0,
    974,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    19,
    RNR_REGS_CFG_BKPT_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_BKPT_IMM, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_BKPT_IMM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable immediate breakpoint\n",
#endif
    { RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_BKPT_IMM_FIELDS[] =
{
    &RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_BKPT_IMM *****/
const ru_reg_rec RNR_REGS_CFG_BKPT_IMM_REG =
{
    "CFG_BKPT_IMM",
#if RU_INCLUDE_DESC
    "BKPT_IMMEDIATE Register",
    "break point immediate\n",
#endif
    { RNR_REGS_CFG_BKPT_IMM_REG_OFFSET },
    0,
    0,
    975,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_BKPT_IMM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_BKPT_STS, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_BKPT_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_ADDR *****/
const ru_field_rec RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD =
{
    "BKPT_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACTIVE *****/
const ru_field_rec RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD =
{
    "ACTIVE",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint active indication\n",
#endif
    { RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_BKPT_ADDR *****/
const ru_field_rec RNR_REGS_CFG_BKPT_STS_DATA_BKPT_ADDR_FIELD =
{
    "DATA_BKPT_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Data address when triggered by breakpoint\n",
#endif
    { RNR_REGS_CFG_BKPT_STS_DATA_BKPT_ADDR_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_STS_DATA_BKPT_ADDR_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_STS_DATA_BKPT_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_REASON *****/
const ru_field_rec RNR_REGS_CFG_BKPT_STS_BKPT_REASON_FIELD =
{
    "BKPT_REASON",
#if RU_INCLUDE_DESC
    "",
    "Display reason for breakpoint\n",
#endif
    { RNR_REGS_CFG_BKPT_STS_BKPT_REASON_FIELD_MASK },
    0,
    { RNR_REGS_CFG_BKPT_STS_BKPT_REASON_FIELD_WIDTH },
    { RNR_REGS_CFG_BKPT_STS_BKPT_REASON_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_BKPT_STS_FIELDS[] =
{
    &RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD,
    &RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD,
    &RNR_REGS_CFG_BKPT_STS_DATA_BKPT_ADDR_FIELD,
    &RNR_REGS_CFG_BKPT_STS_BKPT_REASON_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_BKPT_STS *****/
const ru_reg_rec RNR_REGS_CFG_BKPT_STS_REG =
{
    "CFG_BKPT_STS",
#if RU_INCLUDE_DESC
    "BKPT_STS Register",
    "breakpoint status\n",
#endif
    { RNR_REGS_CFG_BKPT_STS_REG_OFFSET },
    0,
    0,
    976,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_BKPT_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_PC_STS, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_PC_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CURRENT_PC_ADDR *****/
const ru_field_rec RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD =
{
    "CURRENT_PC_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Current program counter address\n",
#endif
    { RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD_WIDTH },
    { RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PC_RET *****/
const ru_field_rec RNR_REGS_CFG_PC_STS_PC_RET_FIELD =
{
    "PC_RET",
#if RU_INCLUDE_DESC
    "",
    "Call stack return address\n",
#endif
    { RNR_REGS_CFG_PC_STS_PC_RET_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PC_STS_PC_RET_FIELD_WIDTH },
    { RNR_REGS_CFG_PC_STS_PC_RET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PC_STS_FIELDS[] =
{
    &RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD,
    &RNR_REGS_CFG_PC_STS_PC_RET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_PC_STS *****/
const ru_reg_rec RNR_REGS_CFG_PC_STS_REG =
{
    "CFG_PC_STS",
#if RU_INCLUDE_DESC
    "PC_STS Register",
    "Program counterstatus\n",
#endif
    { RNR_REGS_CFG_PC_STS_REG_OFFSET },
    0,
    0,
    977,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_PC_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_EXT_ACC_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_EXT_ACC_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR_BASE *****/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD =
{
    "ADDR_BASE",
#if RU_INCLUDE_DESC
    "",
    "address base for calculation.\nSee description under start_thread field\n",
#endif
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD_WIDTH },
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR_STEP_0 *****/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_0_FIELD =
{
    "ADDR_STEP_0",
#if RU_INCLUDE_DESC
    "",
    "address step 0 for thread base address calculation. See description under start_thread field.\n",
#endif
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_0_FIELD_MASK },
    0,
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_0_FIELD_WIDTH },
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR_STEP_1 *****/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_1_FIELD =
{
    "ADDR_STEP_1",
#if RU_INCLUDE_DESC
    "",
    "address step 1 for thread base address calculation. See description under start_thread field.\n",
#endif
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_1_FIELD_MASK },
    0,
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_1_FIELD_WIDTH },
    { RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: START_THREAD *****/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD =
{
    "START_THREAD",
#if RU_INCLUDE_DESC
    "",
    "start thread for calculation.\n\nAddress for external accelerator will be calculated as follows:\n<adjusted_thread>=<current_thread>-<start_thread>\n\nacc_thread_address = address_base + adjusted_thread*(2^step_0) + adjusted_thread*(2^step_1)\n\nNote that if step_0 or step_1 are set to 0, the operands of the equation will still be non-zero.\nSo if total_step is required to be an exact power N of 2, then both step_0 and step_1 should be set to (N-1).\nFor example if total step is 256 (2^8), then both step_0 and step_1 should be set to 7.\n\nIn any case, the resulting address should be always aligned to 8-bytes (64-bits)\n",
#endif
    { RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_EXT_ACC_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD,
    &RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_0_FIELD,
    &RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_1_FIELD,
    &RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_EXT_ACC_CFG *****/
const ru_reg_rec RNR_REGS_CFG_EXT_ACC_CFG_REG =
{
    "CFG_EXT_ACC_CFG",
#if RU_INCLUDE_DESC
    "EXTERNAL_ACC_CONFIG Register",
    "Configure parameters for address calculation in external accelerator command\n",
#endif
    { RNR_REGS_CFG_EXT_ACC_CFG_REG_OFFSET },
    0,
    0,
    978,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_EXT_ACC_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_FIT_FAIL_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_FIT_FAIL_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START_ADDR *****/
const ru_field_rec RNR_REGS_CFG_FIT_FAIL_CFG_START_ADDR_FIELD =
{
    "START_ADDR",
#if RU_INCLUDE_DESC
    "",
    "If (current PC >= start_addr) AND (current PC <= stop_addr), fit fail will not be checked. START_ADDR value should be even.\n",
#endif
    { RNR_REGS_CFG_FIT_FAIL_CFG_START_ADDR_FIELD_MASK },
    0,
    { RNR_REGS_CFG_FIT_FAIL_CFG_START_ADDR_FIELD_WIDTH },
    { RNR_REGS_CFG_FIT_FAIL_CFG_START_ADDR_FIELD_SHIFT },
    8191,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STOP_ADDR *****/
const ru_field_rec RNR_REGS_CFG_FIT_FAIL_CFG_STOP_ADDR_FIELD =
{
    "STOP_ADDR",
#if RU_INCLUDE_DESC
    "",
    "If (current PC >= start_addr) AND (current PC <= stop_addr), fit fail will not be checked. STOP_ADDR value should be even.\n",
#endif
    { RNR_REGS_CFG_FIT_FAIL_CFG_STOP_ADDR_FIELD_MASK },
    0,
    { RNR_REGS_CFG_FIT_FAIL_CFG_STOP_ADDR_FIELD_WIDTH },
    { RNR_REGS_CFG_FIT_FAIL_CFG_STOP_ADDR_FIELD_SHIFT },
    8191,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_FIT_FAIL_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_FIT_FAIL_CFG_START_ADDR_FIELD,
    &RNR_REGS_CFG_FIT_FAIL_CFG_STOP_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_FIT_FAIL_CFG *****/
const ru_reg_rec RNR_REGS_CFG_FIT_FAIL_CFG_REG =
{
    "CFG_FIT_FAIL_CFG",
#if RU_INCLUDE_DESC
    "FIT_FAIL_CONFIG Register",
    "Configure exclusion addresses for fit fail. Fit fail will not be triggered if program counter is between two configured addresses.\n",
#endif
    { RNR_REGS_CFG_FIT_FAIL_CFG_REG_OFFSET },
    0,
    0,
    979,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_FIT_FAIL_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_DATA_BKPT_CFG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_DATA_BKPT_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_0_EN *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_EN_FIELD =
{
    "BKPT_0_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 0\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_0_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_USE_THREAD_FIELD =
{
    "BKPT_0_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_1_EN *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_EN_FIELD =
{
    "BKPT_1_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 1\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_1_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_USE_THREAD_FIELD =
{
    "BKPT_1_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_2_EN *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_EN_FIELD =
{
    "BKPT_2_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 2\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_2_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_USE_THREAD_FIELD =
{
    "BKPT_2_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_3_EN *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_EN_FIELD =
{
    "BKPT_3_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint 3\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_EN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_EN_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BKPT_3_USE_THREAD *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_USE_THREAD_FIELD =
{
    "BKPT_3_USE_THREAD",
#if RU_INCLUDE_DESC
    "",
    "Enable breakpoint for given thread only\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RESET_DATA_BKPT *****/
const ru_field_rec RNR_REGS_CFG_DATA_BKPT_CFG_RESET_DATA_BKPT_FIELD =
{
    "RESET_DATA_BKPT",
#if RU_INCLUDE_DESC
    "",
    "Reset data breakpoint mechanism. Write 1 and then 0\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_RESET_DATA_BKPT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_DATA_BKPT_CFG_RESET_DATA_BKPT_FIELD_WIDTH },
    { RNR_REGS_CFG_DATA_BKPT_CFG_RESET_DATA_BKPT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_DATA_BKPT_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_EN_FIELD,
    &RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_0_USE_THREAD_FIELD,
    &RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_EN_FIELD,
    &RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_1_USE_THREAD_FIELD,
    &RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_EN_FIELD,
    &RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_2_USE_THREAD_FIELD,
    &RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_EN_FIELD,
    &RNR_REGS_CFG_DATA_BKPT_CFG_BKPT_3_USE_THREAD_FIELD,
    &RNR_REGS_CFG_DATA_BKPT_CFG_RESET_DATA_BKPT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_DATA_BKPT_CFG *****/
const ru_reg_rec RNR_REGS_CFG_DATA_BKPT_CFG_REG =
{
    "CFG_DATA_BKPT_CFG",
#if RU_INCLUDE_DESC
    "DATA_BKPT_CFG Register",
    "Data breakpoint configuration\n",
#endif
    { RNR_REGS_CFG_DATA_BKPT_CFG_REG_OFFSET },
    0,
    0,
    980,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_REGS_CFG_DATA_BKPT_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_AQM_COUNTER_VAL, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_AQM_COUNTER_VAL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: AQM_COUNTER_VALUE *****/
const ru_field_rec RNR_REGS_CFG_AQM_COUNTER_VAL_AQM_COUNTER_VALUE_FIELD =
{
    "AQM_COUNTER_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Counter value\n",
#endif
    { RNR_REGS_CFG_AQM_COUNTER_VAL_AQM_COUNTER_VALUE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_AQM_COUNTER_VAL_AQM_COUNTER_VALUE_FIELD_WIDTH },
    { RNR_REGS_CFG_AQM_COUNTER_VAL_AQM_COUNTER_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_AQM_COUNTER_VAL_FIELDS[] =
{
    &RNR_REGS_CFG_AQM_COUNTER_VAL_AQM_COUNTER_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_AQM_COUNTER_VAL *****/
const ru_reg_rec RNR_REGS_CFG_AQM_COUNTER_VAL_REG =
{
    "CFG_AQM_COUNTER_VAL",
#if RU_INCLUDE_DESC
    "AQM_COUNTER_VAL Register",
    "Read current AQM counter value\n",
#endif
    { RNR_REGS_CFG_AQM_COUNTER_VAL_REG_OFFSET },
    0,
    0,
    981,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_AQM_COUNTER_VAL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_STALL_CNT1, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_STALL_CNT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_STALL_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD =
{
    "TOTAL_STALL_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count total stall cycles in profiling window\n",
#endif
    { RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT1_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_STALL_CNT1 *****/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT1_REG =
{
    "CFG_STALL_CNT1",
#if RU_INCLUDE_DESC
    "STALL_CNT1 Register",
    "Stall counters\n",
#endif
    { RNR_REGS_CFG_STALL_CNT1_REG_OFFSET },
    0,
    0,
    982,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_STALL_CNT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_STALL_CNT2, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_STALL_CNT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_ALU_B_FULL_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD =
{
    "STALL_ON_ALU_B_FULL_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due to ALU B FIFO full\n",
#endif
    { RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_ALU_A_FULL_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD =
{
    "STALL_ON_ALU_A_FULL_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due to ALU A FIFO full\n",
#endif
    { RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT2_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD,
    &RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_STALL_CNT2 *****/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT2_REG =
{
    "CFG_STALL_CNT2",
#if RU_INCLUDE_DESC
    "STALL_CNT2 Register",
    "Stall counters\n",
#endif
    { RNR_REGS_CFG_STALL_CNT2_REG_OFFSET },
    0,
    0,
    983,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_STALL_CNT3, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_STALL_CNT3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_JMPREG *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD =
{
    "STALL_ON_JMPREG",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due to jump on address from register\n",
#endif
    { RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_MEMIO_FULL_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD =
{
    "STALL_ON_MEMIO_FULL_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due to MEMIO FIFO full\n",
#endif
    { RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT3_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD,
    &RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_STALL_CNT3 *****/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT3_REG =
{
    "CFG_STALL_CNT3",
#if RU_INCLUDE_DESC
    "STALL_CNT3 Register",
    "Stall counters\n",
#endif
    { RNR_REGS_CFG_STALL_CNT3_REG_OFFSET },
    0,
    0,
    984,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_STALL_CNT4, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_STALL_CNT4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_WAW_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD =
{
    "STALL_ON_WAW_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due to WAW on conditional\n",
#endif
    { RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_SUPER_CMD *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT4_STALL_ON_SUPER_CMD_FIELD =
{
    "STALL_ON_SUPER_CMD",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due to super command\n",
#endif
    { RNR_REGS_CFG_STALL_CNT4_STALL_ON_SUPER_CMD_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT4_STALL_ON_SUPER_CMD_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT4_STALL_ON_SUPER_CMD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT4_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD,
    &RNR_REGS_CFG_STALL_CNT4_STALL_ON_SUPER_CMD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_STALL_CNT4 *****/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT4_REG =
{
    "CFG_STALL_CNT4",
#if RU_INCLUDE_DESC
    "STALL_CNT4 Register",
    "Stall counters\n",
#endif
    { RNR_REGS_CFG_STALL_CNT4_REG_OFFSET },
    0,
    0,
    985,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_STALL_CNT5, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_STALL_CNT5
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_SUPER_CMD_WHEN_FULL *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT5_STALL_ON_SUPER_CMD_WHEN_FULL_FIELD =
{
    "STALL_ON_SUPER_CMD_WHEN_FULL",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due to super command full stall\n",
#endif
    { RNR_REGS_CFG_STALL_CNT5_STALL_ON_SUPER_CMD_WHEN_FULL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT5_STALL_ON_SUPER_CMD_WHEN_FULL_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT5_STALL_ON_SUPER_CMD_WHEN_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_CS_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT5_STALL_ON_CS_CNT_FIELD =
{
    "STALL_ON_CS_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count stall cycles due to CS event\n",
#endif
    { RNR_REGS_CFG_STALL_CNT5_STALL_ON_CS_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT5_STALL_ON_CS_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT5_STALL_ON_CS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT5_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT5_STALL_ON_SUPER_CMD_WHEN_FULL_FIELD,
    &RNR_REGS_CFG_STALL_CNT5_STALL_ON_CS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_STALL_CNT5 *****/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT5_REG =
{
    "CFG_STALL_CNT5",
#if RU_INCLUDE_DESC
    "STALL_CNT5 Register",
    "Stall counters\n",
#endif
    { RNR_REGS_CFG_STALL_CNT5_REG_OFFSET },
    0,
    0,
    986,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_STALL_CNT6, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_STALL_CNT6
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACTIVE_CYCLES_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT6_ACTIVE_CYCLES_CNT_FIELD =
{
    "ACTIVE_CYCLES_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count active cycles of a task\n",
#endif
    { RNR_REGS_CFG_STALL_CNT6_ACTIVE_CYCLES_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT6_ACTIVE_CYCLES_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT6_ACTIVE_CYCLES_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT6_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT6_ACTIVE_CYCLES_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_STALL_CNT6 *****/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT6_REG =
{
    "CFG_STALL_CNT6",
#if RU_INCLUDE_DESC
    "STALL_CNT6 Register",
    "Stall counters\n",
#endif
    { RNR_REGS_CFG_STALL_CNT6_REG_OFFSET },
    0,
    0,
    987,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_STALL_CNT6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_STALL_CNT7, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_STALL_CNT7
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_JMP_FULL_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT7_STALL_ON_JMP_FULL_CNT_FIELD =
{
    "STALL_ON_JMP_FULL_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due to branch FIFO full\n",
#endif
    { RNR_REGS_CFG_STALL_CNT7_STALL_ON_JMP_FULL_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT7_STALL_ON_JMP_FULL_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT7_STALL_ON_JMP_FULL_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STALL_ON_SKIP_JMP_CNT *****/
const ru_field_rec RNR_REGS_CFG_STALL_CNT7_STALL_ON_SKIP_JMP_CNT_FIELD =
{
    "STALL_ON_SKIP_JMP_CNT",
#if RU_INCLUDE_DESC
    "",
    "Count stalls due wit conditional on skip jump\n",
#endif
    { RNR_REGS_CFG_STALL_CNT7_STALL_ON_SKIP_JMP_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_STALL_CNT7_STALL_ON_SKIP_JMP_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_STALL_CNT7_STALL_ON_SKIP_JMP_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT7_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT7_STALL_ON_JMP_FULL_CNT_FIELD,
    &RNR_REGS_CFG_STALL_CNT7_STALL_ON_SKIP_JMP_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_STALL_CNT7 *****/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT7_REG =
{
    "CFG_STALL_CNT7",
#if RU_INCLUDE_DESC
    "STALL_CNT7 Register",
    "Stall counters\n",
#endif
    { RNR_REGS_CFG_STALL_CNT7_REG_OFFSET },
    0,
    0,
    988,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_PROFILING_STS, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_PROFILING_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_WRITE_PNT *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD =
{
    "TRACE_WRITE_PNT",
#if RU_INCLUDE_DESC
    "",
    "Trace write pointer\n",
#endif
    { RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IDLE_NO_ACTIVE_TASK *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD =
{
    "IDLE_NO_ACTIVE_TASK",
#if RU_INCLUDE_DESC
    "",
    "No active task\n",
#endif
    { RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CURR_THREAD_NUM *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD =
{
    "CURR_THREAD_NUM",
#if RU_INCLUDE_DESC
    "",
    "Current thread num\n",
#endif
    { RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROFILING_ACTIVE *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD =
{
    "PROFILING_ACTIVE",
#if RU_INCLUDE_DESC
    "",
    "Status of profiling ON/OFF\n",
#endif
    { RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_FIFO_OVERRUN *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD =
{
    "TRACE_FIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "Sticky bit, indicating trace event FIFO overrun. Cleared by writing bit [31] of PROFILING_CFG_1 register\n",
#endif
    { RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SINGLE_MODE_PROFILING_STATUS *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_SINGLE_MODE_PROFILING_STATUS_FIELD =
{
    "SINGLE_MODE_PROFILING_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Shows the status of profiling window in single mode\n",
#endif
    { RNR_REGS_CFG_PROFILING_STS_SINGLE_MODE_PROFILING_STATUS_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_STS_SINGLE_MODE_PROFILING_STATUS_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_STS_SINGLE_MODE_PROFILING_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_STS_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_SINGLE_MODE_PROFILING_STATUS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_PROFILING_STS *****/
const ru_reg_rec RNR_REGS_CFG_PROFILING_STS_REG =
{
    "CFG_PROFILING_STS",
#if RU_INCLUDE_DESC
    "PROFILING_STS Register",
    "profiling status\n",
#endif
    { RNR_REGS_CFG_PROFILING_STS_REG_OFFSET },
    0,
    0,
    989,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_REGS_CFG_PROFILING_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_PROFILING_CFG_0, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_PROFILING_CFG_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_BASE_ADDR *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD =
{
    "TRACE_BASE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Base address for trace buffer\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_MAX_ADDR *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD =
{
    "TRACE_MAX_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Trace buffer MAX address\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_CFG_0_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_PROFILING_CFG_0 *****/
const ru_reg_rec RNR_REGS_CFG_PROFILING_CFG_0_REG =
{
    "CFG_PROFILING_CFG_0",
#if RU_INCLUDE_DESC
    "PROFILING_CFG_0 Register",
    "profiling confuguration 0\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_0_REG_OFFSET },
    0,
    0,
    990,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_PROFILING_CFG_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_PROFILING_CFG_1, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_PROFILING_CFG_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_WRAPAROUND *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD =
{
    "TRACE_WRAPAROUND",
#if RU_INCLUDE_DESC
    "",
    "Wraparound when writing trace buffer\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_MODE *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD =
{
    "TRACE_MODE",
#if RU_INCLUDE_DESC
    "",
    "Select all tasks or single task mode\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_DISABLE_IDLE_IN *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD =
{
    "TRACE_DISABLE_IDLE_IN",
#if RU_INCLUDE_DESC
    "",
    "Select whether to log IDLE in context swap events\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_DISABLE_WAKEUP_LOG *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD =
{
    "TRACE_DISABLE_WAKEUP_LOG",
#if RU_INCLUDE_DESC
    "",
    "Enable/disable logging of scheduler events (wakeups). Relevant only for single task mode\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_TASK *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD =
{
    "TRACE_TASK",
#if RU_INCLUDE_DESC
    "",
    "Select task for single task operation (tracer)\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IDLE_COUNTER_SOURCE_SEL *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD =
{
    "IDLE_COUNTER_SOURCE_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select mode for IDLE counter\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTERS_SELECTED_TASK_MODE *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD =
{
    "COUNTERS_SELECTED_TASK_MODE",
#if RU_INCLUDE_DESC
    "",
    "Enable single selected task mode (counters)\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTERS_TASK *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD =
{
    "COUNTERS_TASK",
#if RU_INCLUDE_DESC
    "",
    "Select task for single task operation (counters)\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROFILING_WINDOW_MODE *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_MODE_FIELD =
{
    "PROFILING_WINDOW_MODE",
#if RU_INCLUDE_DESC
    "",
    "Choose profiling window mode\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_MODE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_MODE_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SINGLE_MODE_START_OPTION *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_START_OPTION_FIELD =
{
    "SINGLE_MODE_START_OPTION",
#if RU_INCLUDE_DESC
    "",
    "Choose start window option in case of single runner profiling window mode\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_START_OPTION_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_START_OPTION_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_START_OPTION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SINGLE_MODE_STOP_OPTION *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_STOP_OPTION_FIELD =
{
    "SINGLE_MODE_STOP_OPTION",
#if RU_INCLUDE_DESC
    "",
    "Choose stop window option in case of single runner profiling window mode\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_STOP_OPTION_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_STOP_OPTION_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_STOP_OPTION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WINDOW_MANUAL_START *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_START_FIELD =
{
    "WINDOW_MANUAL_START",
#if RU_INCLUDE_DESC
    "",
    "write 1 to start profiling window manually (if appropriate option is configured in SINGLE_MODE_START_OPTION)\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_START_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_START_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WINDOW_MANUAL_STOP *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_STOP_FIELD =
{
    "WINDOW_MANUAL_STOP",
#if RU_INCLUDE_DESC
    "",
    "write 1 to stop profiling window manually (if appropriate option is configured in SINGLE_MODE_START_OPTION)\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_STOP_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_STOP_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_STOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACER_ENABLE *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACER_ENABLE_FIELD =
{
    "TRACER_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable tracer logic (writing trace to memory)\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACER_ENABLE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACER_ENABLE_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACER_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROFILING_WINDOW_RESET *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_RESET_FIELD =
{
    "PROFILING_WINDOW_RESET",
#if RU_INCLUDE_DESC
    "",
    "write 1 to reset profiling window\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_RESET_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_RESET_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROFILING_WINDOW_ENABLE *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_ENABLE_FIELD =
{
    "PROFILING_WINDOW_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable single profiling window. If enabled, will start looking for start condition as specified by START_OPTION field\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_ENABLE_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_ENABLE_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_RESET_EVENT_FIFO *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD =
{
    "TRACE_RESET_EVENT_FIFO",
#if RU_INCLUDE_DESC
    "",
    "Apply software reset to event FIFO\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRACE_CLEAR_FIFO_OVERRUN *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD =
{
    "TRACE_CLEAR_FIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "Write 1 to clear event FIFO overrun sticky bit\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_CFG_1_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_MODE_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_START_OPTION_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_SINGLE_MODE_STOP_OPTION_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_START_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_WINDOW_MANUAL_STOP_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACER_ENABLE_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_RESET_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_PROFILING_WINDOW_ENABLE_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_PROFILING_CFG_1 *****/
const ru_reg_rec RNR_REGS_CFG_PROFILING_CFG_1_REG =
{
    "CFG_PROFILING_CFG_1",
#if RU_INCLUDE_DESC
    "PROFILING_CFG_1 Register",
    "profiling confuguration 1\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_1_REG_OFFSET },
    0,
    0,
    991,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    18,
    RNR_REGS_CFG_PROFILING_CFG_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_PROFILING_COUNTER, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_PROFILING_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Current 32-bit value of counter\n",
#endif
    { RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_COUNTER_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_PROFILING_COUNTER *****/
const ru_reg_rec RNR_REGS_CFG_PROFILING_COUNTER_REG =
{
    "CFG_PROFILING_COUNTER",
#if RU_INCLUDE_DESC
    "PROFILING_COUNTER Register",
    "Display profiling counter value\n",
#endif
    { RNR_REGS_CFG_PROFILING_COUNTER_REG_OFFSET },
    0,
    0,
    992,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_PROFILING_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_PROFILING_CFG_2, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_PROFILING_CFG_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TRIGGER_ON_SECOND *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD =
{
    "TRIGGER_ON_SECOND",
#if RU_INCLUDE_DESC
    "",
    "Dont start profiling window when encountering start PC for first time\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PC_START *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD =
{
    "PC_START",
#if RU_INCLUDE_DESC
    "",
    "Configure PC value to start profiling\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PC_STOP_OR_CYCLE_COUNT *****/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_OR_CYCLE_COUNT_FIELD =
{
    "PC_STOP_OR_CYCLE_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Configure PC value to stop profiling, or number of cycles to count - as selected by STOP_OPTION field in PROGFILING_CFG_1 register\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_OR_CYCLE_COUNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_OR_CYCLE_COUNT_FIELD_WIDTH },
    { RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_OR_CYCLE_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_CFG_2_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_OR_CYCLE_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_PROFILING_CFG_2 *****/
const ru_reg_rec RNR_REGS_CFG_PROFILING_CFG_2_REG =
{
    "CFG_PROFILING_CFG_2",
#if RU_INCLUDE_DESC
    "PROFILING_CFG_2 Register",
    "profiling confuguration 2\n",
#endif
    { RNR_REGS_CFG_PROFILING_CFG_2_REG_OFFSET },
    0,
    0,
    993,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    RNR_REGS_CFG_PROFILING_CFG_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_EXEC_CMDS_CNT, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_EXEC_CMDS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXEC_COUNTER *****/
const ru_field_rec RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD =
{
    "EXEC_COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Count executed commands\n",
#endif
    { RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD_MASK },
    0,
    { RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD_WIDTH },
    { RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_EXEC_CMDS_CNT_FIELDS[] =
{
    &RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_EXEC_CMDS_CNT *****/
const ru_reg_rec RNR_REGS_CFG_EXEC_CMDS_CNT_REG =
{
    "CFG_EXEC_CMDS_CNT",
#if RU_INCLUDE_DESC
    "EXEC_CMDS_CNT Register",
    "Count all executed commands in profiling window\n",
#endif
    { RNR_REGS_CFG_EXEC_CMDS_CNT_REG_OFFSET },
    0,
    0,
    994,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_EXEC_CMDS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_IDLE_CNT1, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_IDLE_CNT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IDLE_CNT *****/
const ru_field_rec RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD =
{
    "IDLE_CNT",
#if RU_INCLUDE_DESC
    "",
    "IDLE countReserved\n",
#endif
    { RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_IDLE_CNT1_FIELDS[] =
{
    &RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_IDLE_CNT1 *****/
const ru_reg_rec RNR_REGS_CFG_IDLE_CNT1_REG =
{
    "CFG_IDLE_CNT1",
#if RU_INCLUDE_DESC
    "IDLE_CNT1 Register",
    "idle count\n",
#endif
    { RNR_REGS_CFG_IDLE_CNT1_REG_OFFSET },
    0,
    0,
    995,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_IDLE_CNT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_JMP_CNT, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_JMP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNTAKEN_JMP_CNT *****/
const ru_field_rec RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD =
{
    "UNTAKEN_JMP_CNT",
#if RU_INCLUDE_DESC
    "",
    "Counts jumps with prediction miss, when prediction was dont jump\n",
#endif
    { RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TAKEN_JMP_CNT *****/
const ru_field_rec RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD =
{
    "TAKEN_JMP_CNT",
#if RU_INCLUDE_DESC
    "",
    "Counts jumps with prediction miss, when prediction was jump\n",
#endif
    { RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD_MASK },
    0,
    { RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD_WIDTH },
    { RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_JMP_CNT_FIELDS[] =
{
    &RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD,
    &RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_JMP_CNT *****/
const ru_reg_rec RNR_REGS_CFG_JMP_CNT_REG =
{
    "CFG_JMP_CNT",
#if RU_INCLUDE_DESC
    "JUMP_CNT Register",
    "Mispredicted jumps count\n",
#endif
    { RNR_REGS_CFG_JMP_CNT_REG_OFFSET },
    0,
    0,
    996,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_JMP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_REGS_CFG_METAL_FIX_REG, TYPE: Type_RCQ_CORE_REGS_RCQ_CFG_METAL_FIX_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: METAL_FIX *****/
const ru_field_rec RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD =
{
    "METAL_FIX",
#if RU_INCLUDE_DESC
    "",
    "32 bit register for metal fix\n",
#endif
    { RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD_MASK },
    0,
    { RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD_WIDTH },
    { RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_METAL_FIX_REG_FIELDS[] =
{
    &RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_REGS_CFG_METAL_FIX_REG *****/
const ru_reg_rec RNR_REGS_CFG_METAL_FIX_REG_REG =
{
    "CFG_METAL_FIX_REG",
#if RU_INCLUDE_DESC
    "METAL_FIX Register",
    "32 bit register for metal fixes.\n",
#endif
    { RNR_REGS_CFG_METAL_FIX_REG_REG_OFFSET },
    0,
    0,
    997,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_METAL_FIX_REG_FIELDS,
#endif
};

unsigned long RNR_REGS_ADDRS[] =
{
    0x82800000,
    0x82801000,
    0x82802000,
    0x82803000,
    0x82804000,
    0x82805000,
    0x82806000,
};

static const ru_reg_rec *RNR_REGS_REGS[] =
{
    &RNR_REGS_CFG_GLOBAL_CTRL_REG,
    &RNR_REGS_CFG_CPU_WAKEUP_REG,
    &RNR_REGS_CFG_INT_CTRL_REG,
    &RNR_REGS_CFG_INT_MASK_REG,
    &RNR_REGS_CFG_GEN_CFG_REG,
    &RNR_REGS_CFG_CAM_CFG_REG,
    &RNR_REGS_CFG_FPM_MINI_CFG_REG,
    &RNR_REGS_CFG_DDR_CFG_REG,
    &RNR_REGS_CFG_PSRAM_CFG_REG,
    &RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG,
    &RNR_REGS_CFG_SCH_CFG_REG,
    &RNR_REGS_CFG_BKPT_CFG_REG,
    &RNR_REGS_CFG_BKPT_IMM_REG,
    &RNR_REGS_CFG_BKPT_STS_REG,
    &RNR_REGS_CFG_PC_STS_REG,
    &RNR_REGS_CFG_EXT_ACC_CFG_REG,
    &RNR_REGS_CFG_FIT_FAIL_CFG_REG,
    &RNR_REGS_CFG_DATA_BKPT_CFG_REG,
    &RNR_REGS_CFG_AQM_COUNTER_VAL_REG,
    &RNR_REGS_CFG_STALL_CNT1_REG,
    &RNR_REGS_CFG_STALL_CNT2_REG,
    &RNR_REGS_CFG_STALL_CNT3_REG,
    &RNR_REGS_CFG_STALL_CNT4_REG,
    &RNR_REGS_CFG_STALL_CNT5_REG,
    &RNR_REGS_CFG_STALL_CNT6_REG,
    &RNR_REGS_CFG_STALL_CNT7_REG,
    &RNR_REGS_CFG_PROFILING_STS_REG,
    &RNR_REGS_CFG_PROFILING_CFG_0_REG,
    &RNR_REGS_CFG_PROFILING_CFG_1_REG,
    &RNR_REGS_CFG_PROFILING_COUNTER_REG,
    &RNR_REGS_CFG_PROFILING_CFG_2_REG,
    &RNR_REGS_CFG_EXEC_CMDS_CNT_REG,
    &RNR_REGS_CFG_IDLE_CNT1_REG,
    &RNR_REGS_CFG_JMP_CNT_REG,
    &RNR_REGS_CFG_METAL_FIX_REG_REG,
};

const ru_block_rec RNR_REGS_BLOCK =
{
    "RNR_REGS",
    RNR_REGS_ADDRS,
    7,
    35,
    RNR_REGS_REGS,
};
