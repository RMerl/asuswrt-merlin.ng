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
 * Field: RNR_REGS_CFG_GLOBAL_CTRL_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "Runner enable. When reset runner pipe is halted, instruction memory and context memory can be accessed by the CPU. The CPU can reset or set this bit"
    "The firmware can reset this bit by writing to the disable bit at the runner I/O control register.",
#endif
    RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD_WIDTH,
    RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD =
{
    "DMA_ILLEGAL_STATUS",
#if RU_INCLUDE_DESC
    "dma_illegal",
    "Notifies about DMA illegal access (>16 cycles on UBUS). Sticky bit. cleared by writing 1 to this bit.",
#endif
    RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD_MASK,
    0,
    RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD_WIDTH,
    RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD =
{
    "PREDICTION_OVERRUN_STATUS",
#if RU_INCLUDE_DESC
    "prediction_overrun_status",
    "Notifies about prediction FIFO overwrite status. Sticky bit. cleared by writing 1 to this bit.",
#endif
    RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD_MASK,
    0,
    RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD_WIDTH,
    RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_GLOBAL_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_GLOBAL_CTRL_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_GLOBAL_CTRL_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_GLOBAL_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD =
{
    "MICRO_SEC_VAL",
#if RU_INCLUDE_DESC
    "Micro_sec_val",
    "",
#endif
    RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD_MASK,
    0,
    RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD_WIDTH,
    RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_GLOBAL_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GLOBAL_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_GLOBAL_CTRL_RESERVED1_FIELD_MASK,
    0,
    RNR_REGS_CFG_GLOBAL_CTRL_RESERVED1_FIELD_WIDTH,
    RNR_REGS_CFG_GLOBAL_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD =
{
    "THREAD_NUM",
#if RU_INCLUDE_DESC
    "Thread_Number",
    "The thread number to be invoked by the CPU.",
#endif
    RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD_MASK,
    0,
    RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD_WIDTH,
    RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_CPU_WAKEUP_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_CPU_WAKEUP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_CPU_WAKEUP_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_CPU_WAKEUP_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_CPU_WAKEUP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT0_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD =
{
    "INT0_STS",
#if RU_INCLUDE_DESC
    "Interrupt_0_status",
    "While any of this field bits is set interrupt line 0 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT0_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT1_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD =
{
    "INT1_STS",
#if RU_INCLUDE_DESC
    "Interrupt_1_status",
    "While any of this field bits is set interrupt line 0 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT1_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT2_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD =
{
    "INT2_STS",
#if RU_INCLUDE_DESC
    "Interrupt2_status",
    "While this bit is set interrupt line 2 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT2_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT3_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD =
{
    "INT3_STS",
#if RU_INCLUDE_DESC
    "Interrupt3_status",
    "While this bit is set interrupt line 3 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT3_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT4_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD =
{
    "INT4_STS",
#if RU_INCLUDE_DESC
    "Interrupt4_status",
    "While this bit is set interrupt line 4 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT4_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT5_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD =
{
    "INT5_STS",
#if RU_INCLUDE_DESC
    "Interrupt5_status",
    "While this bit is set interrupt line 5 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT5_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT6_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD =
{
    "INT6_STS",
#if RU_INCLUDE_DESC
    "Interrupt6_status",
    "While this bit is set interrupt line 6 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT6_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT7_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD =
{
    "INT7_STS",
#if RU_INCLUDE_DESC
    "Interrupt7_status",
    "While this bit is set interrupt line 6 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT7_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT8_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD =
{
    "INT8_STS",
#if RU_INCLUDE_DESC
    "Interrupt8_status",
    "While this bit is set interrupt line 8 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT8_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_INT9_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD =
{
    "INT9_STS",
#if RU_INCLUDE_DESC
    "Interrupt9_status",
    "While this bit is set interrupt line 9 is set. SW can write '1' to clear any bit. Write of '0' is ignored.",
#endif
    RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_INT9_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_INT_CTRL_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD =
{
    "FIT_FAIL_STS",
#if RU_INCLUDE_DESC
    "Fit_fail_status",
    "",
#endif
    RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD_WIDTH,
    RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT0_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD =
{
    "INT0_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_0_mask",
    "Mask INT0 causes",
#endif
    RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT0_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT1_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD =
{
    "INT1_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_1_mask",
    "INT1 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT1_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT2_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD =
{
    "INT2_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_2_mask",
    "INT2 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT2_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT3_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD =
{
    "INT3_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_3_mask",
    "INT3 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT3_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT4_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD =
{
    "INT4_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_4_mask",
    "INT4 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT4_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT5_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD =
{
    "INT5_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_5_mask",
    "INT5 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT5_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT6_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD =
{
    "INT6_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_6_mask",
    "INT6 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT6_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT7_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD =
{
    "INT7_MASK",
#if RU_INCLUDE_DESC
    "Inerrupt_7_mask",
    "INT7 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT7_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT8_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD =
{
    "INT8_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_8_mask",
    "INT8 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT8_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_INT9_MASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD =
{
    "INT9_MASK",
#if RU_INCLUDE_DESC
    "Interrupt_9_mask",
    "INT9 mask cause",
#endif
    RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_INT9_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_INT_MASK_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_INT_MASK_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD =
{
    "DISABLE_DMA_OLD_FLOW_CONTROL",
#if RU_INCLUDE_DESC
    "DISABLE_DMA_OLD_FLOW_CONTROL",
    "Disable DMA old flow control. When set to 1, DMA will not check read FIFO occupancy when issuing READ requests, relying instead on DMA backpressure mechanism vs read dispatcher block.",
#endif
    RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD_MASK,
    0,
    RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD_WIDTH,
    RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD =
{
    "TEST_FIT_FAIL",
#if RU_INCLUDE_DESC
    "TEST_FIT_FAIL",
    "set to 1 to test fit fail interrupt.",
#endif
    RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD_MASK,
    0,
    RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD_WIDTH,
    RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_GEN_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_GEN_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_GEN_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_GEN_CFG_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_GEN_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_CAM_CFG_STOP_VALUE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD =
{
    "STOP_VALUE",
#if RU_INCLUDE_DESC
    "Stop_Value",
    "CAM operation is stopped when reaching an entry with a value matching this field."
    "For a 32-bit or 64-bit CAM entries, this value is concatenated.",
#endif
    RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD_MASK,
    0,
    RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD_WIDTH,
    RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_CAM_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_CAM_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_CAM_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_CAM_CFG_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_CAM_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_DDR_CFG_DMA_BASE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD =
{
    "DMA_BASE",
#if RU_INCLUDE_DESC
    "DMA_base_address",
    "DMA base address for ADDR_CALC",
#endif
    RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD_MASK,
    0,
    RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD_WIDTH,
    RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD =
{
    "DMA_BUF_SIZE",
#if RU_INCLUDE_DESC
    "DMA_buffer_size",
    "3 bits indicating buffer size",
#endif
    RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD_MASK,
    0,
    RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD_WIDTH,
    RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_DDR_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_DDR_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_DDR_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_DDR_CFG_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_DDR_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD =
{
    "DMA_STATIC_OFFSET",
#if RU_INCLUDE_DESC
    "DMA_static_offset",
    "DMA static offset",
#endif
    RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD_MASK,
    0,
    RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD_WIDTH,
    RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PSRAM_CFG_DMA_BASE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD =
{
    "DMA_BASE",
#if RU_INCLUDE_DESC
    "DMA_base_address",
    "DMA base address for ADDR_CALC",
#endif
    RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD_MASK,
    0,
    RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD_WIDTH,
    RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD =
{
    "DMA_BUF_SIZE",
#if RU_INCLUDE_DESC
    "DMA_buffer_size",
    "3 bits indicating buffer size",
#endif
    RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD_MASK,
    0,
    RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD_WIDTH,
    RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PSRAM_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PSRAM_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_PSRAM_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_PSRAM_CFG_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_PSRAM_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD =
{
    "DMA_STATIC_OFFSET",
#if RU_INCLUDE_DESC
    "DMA_static_offset",
    "DMA static offset",
#endif
    RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD_MASK,
    0,
    RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD_WIDTH,
    RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD =
{
    "MASK0",
#if RU_INCLUDE_DESC
    "MASK0",
    "Mask 0 for range serach. according to the number of 1 in the mask the cam machine can differ between the Key and TAG",
#endif
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD_MASK,
    0,
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD_WIDTH,
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD =
{
    "MASK1",
#if RU_INCLUDE_DESC
    "MASK1",
    "Mask 0 for range serach. according to the number of 1 in the mask the cam machine can differ between the Key and TAG",
#endif
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD_MASK,
    0,
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD_WIDTH,
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD =
{
    "SCHEDULER_MODE",
#if RU_INCLUDE_DESC
    "MODE",
    "Configure priority mode for scheduler operation",
#endif
    RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD_MASK,
    0,
    RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD_WIDTH,
    RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_SCH_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_SCH_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_SCH_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_SCH_CFG_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_SCH_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD =
{
    "BKPT_0_EN",
#if RU_INCLUDE_DESC
    "BKPT_0_EN",
    "Enable breakpoint 0",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_0_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD =
{
    "BKPT_0_USE_THREAD",
#if RU_INCLUDE_DESC
    "BKPT_0_USE_THREAD",
    "Enable breakpoint for given thread only",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_0_USE_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD =
{
    "BKPT_1_EN",
#if RU_INCLUDE_DESC
    "BKPT_1_EN",
    "Enable breakpoint 1",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_1_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD =
{
    "BKPT_1_USE_THREAD",
#if RU_INCLUDE_DESC
    "BKPT_1_USE_THREAD",
    "Enable breakpoint for given thread only",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_1_USE_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD =
{
    "BKPT_2_EN",
#if RU_INCLUDE_DESC
    "BKPT_2_EN",
    "Enable breakpoint 2",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_2_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD =
{
    "BKPT_2_USE_THREAD",
#if RU_INCLUDE_DESC
    "BKPT_2_USE_THREAD",
    "Enable breakpoint for given thread only",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_2_USE_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD =
{
    "BKPT_3_EN",
#if RU_INCLUDE_DESC
    "BKPT_3_EN",
    "Enable breakpoint 3",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_3_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD =
{
    "BKPT_3_USE_THREAD",
#if RU_INCLUDE_DESC
    "BKPT_3_USE_THREAD",
    "Enable breakpoint for given thread only",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_3_USE_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD =
{
    "BKPT_4_EN",
#if RU_INCLUDE_DESC
    "BKPT_4_EN",
    "Enable breakpoint 4",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_4_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD =
{
    "BKPT_4_USE_THREAD",
#if RU_INCLUDE_DESC
    "BKPT_4_USE_THREAD",
    "Enable breakpoint for given thread only",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_4_USE_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD =
{
    "BKPT_5_EN",
#if RU_INCLUDE_DESC
    "BKPT_5_EN",
    "Enable breakpoint 5",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_5_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD =
{
    "BKPT_5_USE_THREAD",
#if RU_INCLUDE_DESC
    "BKPT_5_USE_THREAD",
    "Enable breakpoint for given thread only",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_5_USE_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD =
{
    "BKPT_6_EN",
#if RU_INCLUDE_DESC
    "BKPT_6_EN",
    "Enable breakpoint 6",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_6_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD =
{
    "BKPT_6_USE_THREAD",
#if RU_INCLUDE_DESC
    "BKPT_6_USE_THREAD",
    "Enable breakpoint for given thread only",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_6_USE_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD =
{
    "BKPT_7_EN",
#if RU_INCLUDE_DESC
    "BKPT_7_EN",
    "Enable breakpoint 7",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_7_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD =
{
    "BKPT_7_USE_THREAD",
#if RU_INCLUDE_DESC
    "BKPT_7_USE_THREAD",
    "Enable breakpoint for given thread only",
#endif
    RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_BKPT_7_USE_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_STEP_MODE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD =
{
    "STEP_MODE",
#if RU_INCLUDE_DESC
    "STEP_MODE",
    "Configure step mode",
#endif
    RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_STEP_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_BKPT_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD =
{
    "NEW_FLAGS_VAL",
#if RU_INCLUDE_DESC
    "NEW_FLAGS_VAL",
    "Value for new flags",
#endif
    RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_BKPT_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_CFG_RESERVED1_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_IMM_ENABLE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "ENABLE_IMM",
    "Enable immediate breakpoint",
#endif
    RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_IMM_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_IMM_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_BKPT_IMM_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_IMM_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_IMM_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_STS_BKPT_ADDR
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD =
{
    "BKPT_ADDR",
#if RU_INCLUDE_DESC
    "BKPT_ADDR",
    "Breakpoint address",
#endif
    RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_STS_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_STS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_BKPT_STS_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_STS_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_STS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_STS_ACTIVE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD =
{
    "ACTIVE",
#if RU_INCLUDE_DESC
    "ACTIVE",
    "Breakpoint active indication",
#endif
    RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_BKPT_STS_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_BKPT_STS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_BKPT_STS_RESERVED1_FIELD_MASK,
    0,
    RNR_REGS_CFG_BKPT_STS_RESERVED1_FIELD_WIDTH,
    RNR_REGS_CFG_BKPT_STS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD =
{
    "CURRENT_PC_ADDR",
#if RU_INCLUDE_DESC
    "CURRENT_PC",
    "Current program counter address",
#endif
    RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD_MASK,
    0,
    RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD_WIDTH,
    RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PC_STS_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PC_STS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_PC_STS_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_PC_STS_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_PC_STS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PC_STS_PC_RET
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PC_STS_PC_RET_FIELD =
{
    "PC_RET",
#if RU_INCLUDE_DESC
    "PC_RET",
    "Call stack return address",
#endif
    RNR_REGS_CFG_PC_STS_PC_RET_FIELD_MASK,
    0,
    RNR_REGS_CFG_PC_STS_PC_RET_FIELD_WIDTH,
    RNR_REGS_CFG_PC_STS_PC_RET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PC_STS_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PC_STS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_PC_STS_RESERVED1_FIELD_MASK,
    0,
    RNR_REGS_CFG_PC_STS_RESERVED1_FIELD_WIDTH,
    RNR_REGS_CFG_PC_STS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD =
{
    "ADDR_BASE",
#if RU_INCLUDE_DESC
    "address_base",
    "address base for calculation",
#endif
    RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD_MASK,
    0,
    RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD_WIDTH,
    RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_EXT_ACC_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_EXT_ACC_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_EXT_ACC_CFG_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_EXT_ACC_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_FIELD =
{
    "ADDR_STEP",
#if RU_INCLUDE_DESC
    "address_step",
    "address step for calculation",
#endif
    RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_FIELD_MASK,
    0,
    RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_FIELD_WIDTH,
    RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD =
{
    "START_THREAD",
#if RU_INCLUDE_DESC
    "start_thread",
    "start thread for calculation",
#endif
    RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD_MASK,
    0,
    RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD_WIDTH,
    RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_EXT_ACC_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_EXT_ACC_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_EXT_ACC_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_REGS_CFG_EXT_ACC_CFG_RESERVED1_FIELD_WIDTH,
    RNR_REGS_CFG_EXT_ACC_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_STALL_CNT1_STALL_ON_JMP_FULL_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_STALL_CNT1_STALL_ON_JMP_FULL_CNT_FIELD =
{
    "STALL_ON_JMP_FULL_CNT",
#if RU_INCLUDE_DESC
    "STALL_ON_JMP_FULL_CNT",
    "Count stalls due to branch FIFO full",
#endif
    RNR_REGS_CFG_STALL_CNT1_STALL_ON_JMP_FULL_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_STALL_CNT1_STALL_ON_JMP_FULL_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_STALL_CNT1_STALL_ON_JMP_FULL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD =
{
    "TOTAL_STALL_CNT",
#if RU_INCLUDE_DESC
    "TOTAL_STALL_CNT",
    "Count total stall cycles in profiling window",
#endif
    RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD =
{
    "STALL_ON_ALU_B_FULL_CNT",
#if RU_INCLUDE_DESC
    "STALL_ON_ALU_B_FULL_CNT",
    "Count stalls due to ALU B FIFO full",
#endif
    RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD =
{
    "STALL_ON_ALU_A_FULL_CNT",
#if RU_INCLUDE_DESC
    "STALL_ON_ALU_A_FULL_CNT",
    "Count stalls due to ALU A FIFO full",
#endif
    RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD =
{
    "STALL_ON_JMPREG",
#if RU_INCLUDE_DESC
    "STALL_ON_JMPREG",
    "Count stalls due to jump on address from register",
#endif
    RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD_MASK,
    0,
    RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD_WIDTH,
    RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD =
{
    "STALL_ON_MEMIO_FULL_CNT",
#if RU_INCLUDE_DESC
    "STALL_ON_MEMIO_FULL_CNT",
    "Count stalls due to MEMIO FIFO full",
#endif
    RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD =
{
    "STALL_ON_WAW_CNT",
#if RU_INCLUDE_DESC
    "STALL_ON_WAW_CNT",
    "Count stalls due to WAW on conditional",
#endif
    RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_STALL_CNT4_ACTIVE_CYCLES_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_STALL_CNT4_ACTIVE_CYCLES_CNT_FIELD =
{
    "ACTIVE_CYCLES_CNT",
#if RU_INCLUDE_DESC
    "ACTIVE_CYCLES_CNT",
    "Count active cycles of a task",
#endif
    RNR_REGS_CFG_STALL_CNT4_ACTIVE_CYCLES_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_STALL_CNT4_ACTIVE_CYCLES_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_STALL_CNT4_ACTIVE_CYCLES_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD =
{
    "TRACE_WRITE_PNT",
#if RU_INCLUDE_DESC
    "Trace_write_pointer",
    "Trace write pointer",
#endif
    RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD =
{
    "IDLE_NO_ACTIVE_TASK",
#if RU_INCLUDE_DESC
    "IDLE_no_active_task",
    "No active task",
#endif
    RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD =
{
    "CURR_THREAD_NUM",
#if RU_INCLUDE_DESC
    "CURR_thread_num",
    "Current thread num",
#endif
    RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD =
{
    "PROFILING_ACTIVE",
#if RU_INCLUDE_DESC
    "Profiling_active",
    "Status of profiling ON/OFF",
#endif
    RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD =
{
    "TRACE_FIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "TRACE_FIFO_OVERRUN",
    "Sticky bit, indicating trace event FIFO overrun. Cleared by writing bit [31] of PROFILING_CFG_1 register",
#endif
    RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_STS_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_STS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_PROFILING_STS_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_STS_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_STS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD =
{
    "TRACE_BASE_ADDR",
#if RU_INCLUDE_DESC
    "TRACE_BASE_ADDR",
    "Base address for trace buffer",
#endif
    RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_0_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_PROFILING_CFG_0_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_0_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD =
{
    "TRACE_MAX_ADDR",
#if RU_INCLUDE_DESC
    "TRACE_MAX_ADDR",
    "Trace buffer MAX address",
#endif
    RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_0_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_PROFILING_CFG_0_RESERVED1_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_0_RESERVED1_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD =
{
    "TRACE_WRAPAROUND",
#if RU_INCLUDE_DESC
    "TRACE_WRAPAROUND",
    "Wraparound when writing trace buffer",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_WRAPAROUND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD =
{
    "TRACE_MODE",
#if RU_INCLUDE_DESC
    "TRACE_MODE",
    "Select all tasks or single task mode",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD =
{
    "TRACE_DISABLE_IDLE_IN",
#if RU_INCLUDE_DESC
    "TRACE_DISABLE_IDLE_IN",
    "Select whether to log IDLE in context swap events",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_IDLE_IN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD =
{
    "TRACE_DISABLE_WAKEUP_LOG",
#if RU_INCLUDE_DESC
    "TRACE_DISABLE_WAKEUP_LOG",
    "Enable/disable logging of scheduler events (wakeups). Relevant only for single task mode",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_DISABLE_WAKEUP_LOG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD =
{
    "TRACE_TASK",
#if RU_INCLUDE_DESC
    "TRACE_TASK",
    "Select task for single task operation (tracer)",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD =
{
    "IDLE_COUNTER_SOURCE_SEL",
#if RU_INCLUDE_DESC
    "IDLE_COUNTER_SOURCE_SEL",
    "Select mode for IDLE counter",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_IDLE_COUNTER_SOURCE_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD =
{
    "COUNTERS_SELECTED_TASK_MODE",
#if RU_INCLUDE_DESC
    "COUNTERS_SELECTED_TASK_MODE",
    "Enable single selected task mode (counters)",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_SELECTED_TASK_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD =
{
    "COUNTERS_TASK",
#if RU_INCLUDE_DESC
    "COUNTERS_TASK",
    "Select task for single task operation (counters)",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_COUNTERS_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD =
{
    "TRACE_RESET_EVENT_FIFO",
#if RU_INCLUDE_DESC
    "TRACE_RESET_EVENT_FIFO",
    "Apply software reset to event FIFO",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD =
{
    "TRACE_CLEAR_FIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "TRACE_CLEAR_FIFO_OVERRUN",
    "Write 1 to clear event FIFO overrun sticky bit",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_COUNTER_VAL
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "Current 32-bit value of counter",
#endif
    RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_2_EN_PROF_ON_SELECTED_PC
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_EN_PROF_ON_SELECTED_PC_FIELD =
{
    "EN_PROF_ON_SELECTED_PC",
#if RU_INCLUDE_DESC
    "EN_PROF_ON_SELECTED_PC",
    "Enable profiling window between start PC and end PC",
#endif
    RNR_REGS_CFG_PROFILING_CFG_2_EN_PROF_ON_SELECTED_PC_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_2_EN_PROF_ON_SELECTED_PC_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_2_EN_PROF_ON_SELECTED_PC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD =
{
    "TRIGGER_ON_SECOND",
#if RU_INCLUDE_DESC
    "TRIGGER_ON_SECOND",
    "Dont start profiling window when encountering start PC for first time",
#endif
    RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_2_PC_START
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD =
{
    "PC_START",
#if RU_INCLUDE_DESC
    "PC_START",
    "Configure PC value to start profiling",
#endif
    RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_FIELD =
{
    "PC_STOP",
#if RU_INCLUDE_DESC
    "PC_STOP",
    "Configure PC value to stop profiling",
#endif
    RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_2_DISABLE_NOPS_AND_UNCOND
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_DISABLE_NOPS_AND_UNCOND_FIELD =
{
    "DISABLE_NOPS_AND_UNCOND",
#if RU_INCLUDE_DESC
    "DISABLE_NOPS_AND_UNCOND",
    "For executed commands counter, dont count NOPs and UNCONDITIONAL JUMP commands.",
#endif
    RNR_REGS_CFG_PROFILING_CFG_2_DISABLE_NOPS_AND_UNCOND_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_2_DISABLE_NOPS_AND_UNCOND_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_2_DISABLE_NOPS_AND_UNCOND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_PROFILING_CFG_2_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_PROFILING_CFG_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_REGS_CFG_PROFILING_CFG_2_RESERVED0_FIELD_MASK,
    0,
    RNR_REGS_CFG_PROFILING_CFG_2_RESERVED0_FIELD_WIDTH,
    RNR_REGS_CFG_PROFILING_CFG_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD =
{
    "EXEC_COUNTER",
#if RU_INCLUDE_DESC
    "EXEC_COUNTER",
    "Count executed commands",
#endif
    RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD_MASK,
    0,
    RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD_WIDTH,
    RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD =
{
    "IDLE_CNT",
#if RU_INCLUDE_DESC
    "IDLE",
    "IDLE countReserved",
#endif
    RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD =
{
    "UNTAKEN_JMP_CNT",
#if RU_INCLUDE_DESC
    "UNTAKEN_JMP_CNT",
    "Counts jumps with prediction miss, when prediction was dont jump",
#endif
    RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD =
{
    "TAKEN_JMP_CNT",
#if RU_INCLUDE_DESC
    "TAKEN_JMP_CNT",
    "Counts jumps with prediction miss, when prediction was jump",
#endif
    RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD_MASK,
    0,
    RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD_WIDTH,
    RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX
 ******************************************************************************/
const ru_field_rec RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD =
{
    "METAL_FIX",
#if RU_INCLUDE_DESC
    "Metal_fix",
    "32 bit register for metal fix",
#endif
    RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD_MASK,
    0,
    RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD_WIDTH,
    RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: RNR_REGS_CFG_GLOBAL_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_GLOBAL_CTRL_FIELDS[] =
{
    &RNR_REGS_CFG_GLOBAL_CTRL_EN_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_DMA_ILLEGAL_STATUS_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_PREDICTION_OVERRUN_STATUS_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_RESERVED0_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_MICRO_SEC_VAL_FIELD,
    &RNR_REGS_CFG_GLOBAL_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_GLOBAL_CTRL_REG = 
{
    "CFG_GLOBAL_CTRL",
#if RU_INCLUDE_DESC
    "GLOBAL_CONTROL Register",
    "Global control",
#endif
    RNR_REGS_CFG_GLOBAL_CTRL_REG_OFFSET,
    0,
    0,
    237,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_REGS_CFG_GLOBAL_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_CPU_WAKEUP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_CPU_WAKEUP_FIELDS[] =
{
    &RNR_REGS_CFG_CPU_WAKEUP_THREAD_NUM_FIELD,
    &RNR_REGS_CFG_CPU_WAKEUP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_CPU_WAKEUP_REG = 
{
    "CFG_CPU_WAKEUP",
#if RU_INCLUDE_DESC
    "CPU_WAKEUP Register",
    "Writing to this register generates a request towards the runner scheduler.",
#endif
    RNR_REGS_CFG_CPU_WAKEUP_REG_OFFSET,
    0,
    0,
    238,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_CPU_WAKEUP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_INT_CTRL
 ******************************************************************************/
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
    &RNR_REGS_CFG_INT_CTRL_RESERVED0_FIELD,
    &RNR_REGS_CFG_INT_CTRL_FIT_FAIL_STS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_INT_CTRL_REG = 
{
    "CFG_INT_CTRL",
#if RU_INCLUDE_DESC
    "INTERRUPT_CONTROL Register",
    "Interrupt control - UNUSED in 6858",
#endif
    RNR_REGS_CFG_INT_CTRL_REG_OFFSET,
    0,
    0,
    239,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    RNR_REGS_CFG_INT_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_INT_MASK
 ******************************************************************************/
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
    &RNR_REGS_CFG_INT_MASK_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_INT_MASK_REG = 
{
    "CFG_INT_MASK",
#if RU_INCLUDE_DESC
    "INTERRUPT_MASK Register",
    "Interrupt mask -  UNUSED in 6858",
#endif
    RNR_REGS_CFG_INT_MASK_REG_OFFSET,
    0,
    0,
    240,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    RNR_REGS_CFG_INT_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_GEN_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_GEN_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_GEN_CFG_DISABLE_DMA_OLD_FLOW_CONTROL_FIELD,
    &RNR_REGS_CFG_GEN_CFG_TEST_FIT_FAIL_FIELD,
    &RNR_REGS_CFG_GEN_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_GEN_CFG_REG = 
{
    "CFG_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIGURATION Register",
    "General configuration",
#endif
    RNR_REGS_CFG_GEN_CFG_REG_OFFSET,
    0,
    0,
    241,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    RNR_REGS_CFG_GEN_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_CAM_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_CAM_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_CAM_CFG_STOP_VALUE_FIELD,
    &RNR_REGS_CFG_CAM_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_CAM_CFG_REG = 
{
    "CFG_CAM_CFG",
#if RU_INCLUDE_DESC
    "CAM_CONFIGURATION Register",
    "CAM configuration",
#endif
    RNR_REGS_CFG_CAM_CFG_REG_OFFSET,
    0,
    0,
    242,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_CAM_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_DDR_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_DDR_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_DDR_CFG_DMA_BASE_FIELD,
    &RNR_REGS_CFG_DDR_CFG_DMA_BUF_SIZE_FIELD,
    &RNR_REGS_CFG_DDR_CFG_RESERVED0_FIELD,
    &RNR_REGS_CFG_DDR_CFG_DMA_STATIC_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_DDR_CFG_REG = 
{
    "CFG_DDR_CFG",
#if RU_INCLUDE_DESC
    "DMA_DDR_CONFIG Register",
    "DMA DDR config Register. Contains configurations such as buffer size and ddr base address that are used for DDR address calculations (from buffer number) when DMA instruction addr_calc flag is set.",
#endif
    RNR_REGS_CFG_DDR_CFG_REG_OFFSET,
    0,
    0,
    243,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_DDR_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PSRAM_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PSRAM_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_PSRAM_CFG_DMA_BASE_FIELD,
    &RNR_REGS_CFG_PSRAM_CFG_DMA_BUF_SIZE_FIELD,
    &RNR_REGS_CFG_PSRAM_CFG_RESERVED0_FIELD,
    &RNR_REGS_CFG_PSRAM_CFG_DMA_STATIC_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_PSRAM_CFG_REG = 
{
    "CFG_PSRAM_CFG",
#if RU_INCLUDE_DESC
    "DMA_PSRAM_CONFIG Register",
    "DMA PSRAM config Register. Contains configurations such as buffer size and ddr base address that are used for DDR address calculations (from buffer number) when DMA instruction addr_calc flag is set.",
#endif
    RNR_REGS_CFG_PSRAM_CFG_REG_OFFSET,
    0,
    0,
    244,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_PSRAM_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK0_FIELD,
    &RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_MASK1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG = 
{
    "CFG_RAMRD_RANGE_MASK_CFG",
#if RU_INCLUDE_DESC
    "RAMRD_MASK_CONFIG Register",
    "Ramrd mask for range search. The register holds 2 mask that can be chosen by runner core for range seraches.",
#endif
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG_OFFSET,
    0,
    0,
    245,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_SCH_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_SCH_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_SCH_CFG_SCHEDULER_MODE_FIELD,
    &RNR_REGS_CFG_SCH_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_SCH_CFG_REG = 
{
    "CFG_SCH_CFG",
#if RU_INCLUDE_DESC
    "SCHEDULER_CONFIG Register",
    "scheduler configuration",
#endif
    RNR_REGS_CFG_SCH_CFG_REG_OFFSET,
    0,
    0,
    246,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_SCH_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_BKPT_CFG
 ******************************************************************************/
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
    &RNR_REGS_CFG_BKPT_CFG_RESERVED0_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_NEW_FLAGS_VAL_FIELD,
    &RNR_REGS_CFG_BKPT_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_BKPT_CFG_REG = 
{
    "CFG_BKPT_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG Register",
    "breakpoint configuration",
#endif
    RNR_REGS_CFG_BKPT_CFG_REG_OFFSET,
    0,
    0,
    247,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    20,
    RNR_REGS_CFG_BKPT_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_BKPT_IMM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_BKPT_IMM_FIELDS[] =
{
    &RNR_REGS_CFG_BKPT_IMM_ENABLE_FIELD,
    &RNR_REGS_CFG_BKPT_IMM_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_BKPT_IMM_REG = 
{
    "CFG_BKPT_IMM",
#if RU_INCLUDE_DESC
    "BKPT_IMMEDIATE Register",
    "break point immediate",
#endif
    RNR_REGS_CFG_BKPT_IMM_REG_OFFSET,
    0,
    0,
    248,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_BKPT_IMM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_BKPT_STS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_BKPT_STS_FIELDS[] =
{
    &RNR_REGS_CFG_BKPT_STS_BKPT_ADDR_FIELD,
    &RNR_REGS_CFG_BKPT_STS_RESERVED0_FIELD,
    &RNR_REGS_CFG_BKPT_STS_ACTIVE_FIELD,
    &RNR_REGS_CFG_BKPT_STS_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_BKPT_STS_REG = 
{
    "CFG_BKPT_STS",
#if RU_INCLUDE_DESC
    "BKPT_STS Register",
    "breakpoint status",
#endif
    RNR_REGS_CFG_BKPT_STS_REG_OFFSET,
    0,
    0,
    249,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_BKPT_STS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PC_STS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PC_STS_FIELDS[] =
{
    &RNR_REGS_CFG_PC_STS_CURRENT_PC_ADDR_FIELD,
    &RNR_REGS_CFG_PC_STS_RESERVED0_FIELD,
    &RNR_REGS_CFG_PC_STS_PC_RET_FIELD,
    &RNR_REGS_CFG_PC_STS_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_PC_STS_REG = 
{
    "CFG_PC_STS",
#if RU_INCLUDE_DESC
    "PC_STS Register",
    "Program counterstatus",
#endif
    RNR_REGS_CFG_PC_STS_REG_OFFSET,
    0,
    0,
    250,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_PC_STS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_EXT_ACC_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_EXT_ACC_CFG_FIELDS[] =
{
    &RNR_REGS_CFG_EXT_ACC_CFG_ADDR_BASE_FIELD,
    &RNR_REGS_CFG_EXT_ACC_CFG_RESERVED0_FIELD,
    &RNR_REGS_CFG_EXT_ACC_CFG_ADDR_STEP_FIELD,
    &RNR_REGS_CFG_EXT_ACC_CFG_START_THREAD_FIELD,
    &RNR_REGS_CFG_EXT_ACC_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_EXT_ACC_CFG_REG = 
{
    "CFG_EXT_ACC_CFG",
#if RU_INCLUDE_DESC
    "EXTERNAL_ACC_CONFIG Register",
    "Configure parameters for address calculation in external accelerator command",
#endif
    RNR_REGS_CFG_EXT_ACC_CFG_REG_OFFSET,
    0,
    0,
    251,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    RNR_REGS_CFG_EXT_ACC_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_STALL_CNT1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT1_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT1_STALL_ON_JMP_FULL_CNT_FIELD,
    &RNR_REGS_CFG_STALL_CNT1_TOTAL_STALL_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_STALL_CNT1_REG = 
{
    "CFG_STALL_CNT1",
#if RU_INCLUDE_DESC
    "STALL_CNT1 Register",
    "Stall counters",
#endif
    RNR_REGS_CFG_STALL_CNT1_REG_OFFSET,
    0,
    0,
    252,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_STALL_CNT2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT2_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_B_FULL_CNT_FIELD,
    &RNR_REGS_CFG_STALL_CNT2_STALL_ON_ALU_A_FULL_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_STALL_CNT2_REG = 
{
    "CFG_STALL_CNT2",
#if RU_INCLUDE_DESC
    "STALL_CNT2 Register",
    "Stall counters",
#endif
    RNR_REGS_CFG_STALL_CNT2_REG_OFFSET,
    0,
    0,
    253,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_STALL_CNT3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT3_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT3_STALL_ON_JMPREG_FIELD,
    &RNR_REGS_CFG_STALL_CNT3_STALL_ON_MEMIO_FULL_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_STALL_CNT3_REG = 
{
    "CFG_STALL_CNT3",
#if RU_INCLUDE_DESC
    "STALL_CNT3 Register",
    "Stall counters",
#endif
    RNR_REGS_CFG_STALL_CNT3_REG_OFFSET,
    0,
    0,
    254,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_STALL_CNT4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_STALL_CNT4_FIELDS[] =
{
    &RNR_REGS_CFG_STALL_CNT4_STALL_ON_WAW_CNT_FIELD,
    &RNR_REGS_CFG_STALL_CNT4_ACTIVE_CYCLES_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_STALL_CNT4_REG = 
{
    "CFG_STALL_CNT4",
#if RU_INCLUDE_DESC
    "STALL_CNT4 Register",
    "Stall counters",
#endif
    RNR_REGS_CFG_STALL_CNT4_REG_OFFSET,
    0,
    0,
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_STALL_CNT4_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PROFILING_STS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_STS_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_STS_TRACE_WRITE_PNT_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_IDLE_NO_ACTIVE_TASK_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_CURR_THREAD_NUM_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_PROFILING_ACTIVE_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_TRACE_FIFO_OVERRUN_FIELD,
    &RNR_REGS_CFG_PROFILING_STS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_PROFILING_STS_REG = 
{
    "CFG_PROFILING_STS",
#if RU_INCLUDE_DESC
    "PROFILING_STS Register",
    "profiling status",
#endif
    RNR_REGS_CFG_PROFILING_STS_REG_OFFSET,
    0,
    0,
    256,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_REGS_CFG_PROFILING_STS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PROFILING_CFG_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_CFG_0_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_CFG_0_TRACE_BASE_ADDR_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_0_RESERVED0_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_0_TRACE_MAX_ADDR_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_0_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_PROFILING_CFG_0_REG = 
{
    "CFG_PROFILING_CFG_0",
#if RU_INCLUDE_DESC
    "PROFILING_CFG_0 Register",
    "profiling confuguration 0",
#endif
    RNR_REGS_CFG_PROFILING_CFG_0_REG_OFFSET,
    0,
    0,
    257,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_REGS_CFG_PROFILING_CFG_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PROFILING_CFG_1
 ******************************************************************************/
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
    &RNR_REGS_CFG_PROFILING_CFG_1_RESERVED0_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_RESET_EVENT_FIFO_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_1_TRACE_CLEAR_FIFO_OVERRUN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_PROFILING_CFG_1_REG = 
{
    "CFG_PROFILING_CFG_1",
#if RU_INCLUDE_DESC
    "PROFILING_CFG_1 Register",
    "profiling confuguration 1",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_REG_OFFSET,
    0,
    0,
    258,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    RNR_REGS_CFG_PROFILING_CFG_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PROFILING_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_COUNTER_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_COUNTER_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_PROFILING_COUNTER_REG = 
{
    "CFG_PROFILING_COUNTER",
#if RU_INCLUDE_DESC
    "PROFILING_COUNTER Register",
    "Display profiling counter value",
#endif
    RNR_REGS_CFG_PROFILING_COUNTER_REG_OFFSET,
    0,
    0,
    259,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_PROFILING_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PROFILING_CFG_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_PROFILING_CFG_2_FIELDS[] =
{
    &RNR_REGS_CFG_PROFILING_CFG_2_EN_PROF_ON_SELECTED_PC_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_2_TRIGGER_ON_SECOND_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_2_PC_START_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_2_PC_STOP_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_2_DISABLE_NOPS_AND_UNCOND_FIELD,
    &RNR_REGS_CFG_PROFILING_CFG_2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_PROFILING_CFG_2_REG = 
{
    "CFG_PROFILING_CFG_2",
#if RU_INCLUDE_DESC
    "PROFILING_CFG_2 Register",
    "profiling confuguration 2",
#endif
    RNR_REGS_CFG_PROFILING_CFG_2_REG_OFFSET,
    0,
    0,
    260,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_REGS_CFG_PROFILING_CFG_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_EXEC_CMDS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_EXEC_CMDS_CNT_FIELDS[] =
{
    &RNR_REGS_CFG_EXEC_CMDS_CNT_EXEC_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_EXEC_CMDS_CNT_REG = 
{
    "CFG_EXEC_CMDS_CNT",
#if RU_INCLUDE_DESC
    "EXEC_CMDS_CNT Register",
    "Count all executed commands in profiling window",
#endif
    RNR_REGS_CFG_EXEC_CMDS_CNT_REG_OFFSET,
    0,
    0,
    261,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_EXEC_CMDS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_IDLE_CNT1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_IDLE_CNT1_FIELDS[] =
{
    &RNR_REGS_CFG_IDLE_CNT1_IDLE_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_IDLE_CNT1_REG = 
{
    "CFG_IDLE_CNT1",
#if RU_INCLUDE_DESC
    "IDLE_CNT1 Register",
    "idle count",
#endif
    RNR_REGS_CFG_IDLE_CNT1_REG_OFFSET,
    0,
    0,
    262,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_IDLE_CNT1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_JMP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_JMP_CNT_FIELDS[] =
{
    &RNR_REGS_CFG_JMP_CNT_UNTAKEN_JMP_CNT_FIELD,
    &RNR_REGS_CFG_JMP_CNT_TAKEN_JMP_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_JMP_CNT_REG = 
{
    "CFG_JMP_CNT",
#if RU_INCLUDE_DESC
    "JUMP_CNT Register",
    "Mispredicted jumps count",
#endif
    RNR_REGS_CFG_JMP_CNT_REG_OFFSET,
    0,
    0,
    263,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_REGS_CFG_JMP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_REGS_CFG_METAL_FIX_REG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_REGS_CFG_METAL_FIX_REG_FIELDS[] =
{
    &RNR_REGS_CFG_METAL_FIX_REG_METAL_FIX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_REGS_CFG_METAL_FIX_REG_REG = 
{
    "CFG_METAL_FIX_REG",
#if RU_INCLUDE_DESC
    "METAL_FIX Register",
    "32 bit register for metal fixes.",
#endif
    RNR_REGS_CFG_METAL_FIX_REG_REG_OFFSET,
    0,
    0,
    264,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_REGS_CFG_METAL_FIX_REG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: RNR_REGS
 ******************************************************************************/
static const ru_reg_rec *RNR_REGS_REGS[] =
{
    &RNR_REGS_CFG_GLOBAL_CTRL_REG,
    &RNR_REGS_CFG_CPU_WAKEUP_REG,
    &RNR_REGS_CFG_INT_CTRL_REG,
    &RNR_REGS_CFG_INT_MASK_REG,
    &RNR_REGS_CFG_GEN_CFG_REG,
    &RNR_REGS_CFG_CAM_CFG_REG,
    &RNR_REGS_CFG_DDR_CFG_REG,
    &RNR_REGS_CFG_PSRAM_CFG_REG,
    &RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG,
    &RNR_REGS_CFG_SCH_CFG_REG,
    &RNR_REGS_CFG_BKPT_CFG_REG,
    &RNR_REGS_CFG_BKPT_IMM_REG,
    &RNR_REGS_CFG_BKPT_STS_REG,
    &RNR_REGS_CFG_PC_STS_REG,
    &RNR_REGS_CFG_EXT_ACC_CFG_REG,
    &RNR_REGS_CFG_STALL_CNT1_REG,
    &RNR_REGS_CFG_STALL_CNT2_REG,
    &RNR_REGS_CFG_STALL_CNT3_REG,
    &RNR_REGS_CFG_STALL_CNT4_REG,
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

unsigned long RNR_REGS_ADDRS[] =
{
    0x82d00000,
    0x82d01000,
    0x82d02000,
};

const ru_block_rec RNR_REGS_BLOCK = 
{
    "RNR_REGS",
    RNR_REGS_ADDRS,
    3,
    28,
    RNR_REGS_REGS
};

/* End of file XRDP_RNR_REGS.c */
