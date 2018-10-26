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
 * Register: UBUS_SLV_VPB_BASE
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_VPB_BASE_REG = 
{
    "VPB_BASE",
#if RU_INCLUDE_DESC
    "VPB_BASE Register",
    "VPB Base address",
#endif
    UBUS_SLV_VPB_BASE_REG_OFFSET,
    0,
    0,
    584,
};

/******************************************************************************
 * Register: UBUS_SLV_VPB_MASK
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_VPB_MASK_REG = 
{
    "VPB_MASK",
#if RU_INCLUDE_DESC
    "VPB_MASK Register",
    "VPB mask address",
#endif
    UBUS_SLV_VPB_MASK_REG_OFFSET,
    0,
    0,
    585,
};

/******************************************************************************
 * Register: UBUS_SLV_APB_BASE
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_APB_BASE_REG = 
{
    "APB_BASE",
#if RU_INCLUDE_DESC
    "APB_BASE Register",
    "APB Base address",
#endif
    UBUS_SLV_APB_BASE_REG_OFFSET,
    0,
    0,
    586,
};

/******************************************************************************
 * Register: UBUS_SLV_APB_MASK
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_APB_MASK_REG = 
{
    "APB_MASK",
#if RU_INCLUDE_DESC
    "APB_MASK Register",
    "APB mask address",
#endif
    UBUS_SLV_APB_MASK_REG_OFFSET,
    0,
    0,
    587,
};

/******************************************************************************
 * Register: UBUS_SLV_DQM_BASE
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_DQM_BASE_REG = 
{
    "DQM_BASE",
#if RU_INCLUDE_DESC
    "DQM_BASE Register",
    "DQM Base address",
#endif
    UBUS_SLV_DQM_BASE_REG_OFFSET,
    0,
    0,
    588,
};

/******************************************************************************
 * Register: UBUS_SLV_DQM_MASK
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_DQM_MASK_REG = 
{
    "DQM_MASK",
#if RU_INCLUDE_DESC
    "DQM_MASK Register",
    "DQM mask address",
#endif
    UBUS_SLV_DQM_MASK_REG_OFFSET,
    0,
    0,
    589,
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ISR
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_RNR_INTR_CTRL_ISR_REG = 
{
    "RNR_INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active QM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ISR_REG_OFFSET,
    0,
    0,
    590,
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ISM
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_RNR_INTR_CTRL_ISM_REG = 
{
    "RNR_INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ISM_REG_OFFSET,
    0,
    0,
    591,
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_IER
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_RNR_INTR_CTRL_IER_REG = 
{
    "RNR_INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    UBUS_SLV_RNR_INTR_CTRL_IER_REG_OFFSET,
    0,
    0,
    592,
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ITR
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_RNR_INTR_CTRL_ITR_REG = 
{
    "RNR_INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ITR_REG_OFFSET,
    0,
    0,
    593,
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_CFG
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_PROFILING_CFG_REG = 
{
    "PROFILING_CFG",
#if RU_INCLUDE_DESC
    "PROFILING_CFG Register",
    "Profiling configuration settings",
#endif
    UBUS_SLV_PROFILING_CFG_REG_OFFSET,
    0,
    0,
    594,
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_STATUS
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_PROFILING_STATUS_REG = 
{
    "PROFILING_STATUS",
#if RU_INCLUDE_DESC
    "PROFILING_STATUS Register",
    "Profiling status",
#endif
    UBUS_SLV_PROFILING_STATUS_REG_OFFSET,
    0,
    0,
    595,
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_COUNTER
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_PROFILING_COUNTER_REG = 
{
    "PROFILING_COUNTER",
#if RU_INCLUDE_DESC
    "PROFILING_COUNTER Register",
    "Read PROFILING_COUNTER current value",
#endif
    UBUS_SLV_PROFILING_COUNTER_REG_OFFSET,
    0,
    0,
    596,
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_START_VALUE
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_PROFILING_START_VALUE_REG = 
{
    "PROFILING_START_VALUE",
#if RU_INCLUDE_DESC
    "PROFILING_START_VALUE Register",
    "Read PROFILING_START_VALUE value",
#endif
    UBUS_SLV_PROFILING_START_VALUE_REG_OFFSET,
    0,
    0,
    597,
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_STOP_VALUE
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_PROFILING_STOP_VALUE_REG = 
{
    "PROFILING_STOP_VALUE",
#if RU_INCLUDE_DESC
    "PROFILING_STOP_VALUE Register",
    "Read PROFILING_STOP_VALUE value",
#endif
    UBUS_SLV_PROFILING_STOP_VALUE_REG_OFFSET,
    0,
    0,
    598,
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_CYCLE_NUM
 ******************************************************************************/
const ru_reg_rec UBUS_SLV_PROFILING_CYCLE_NUM_REG = 
{
    "PROFILING_CYCLE_NUM",
#if RU_INCLUDE_DESC
    "PROFILING_CYCLE_NUM Register",
    "Set length of profiling window",
#endif
    UBUS_SLV_PROFILING_CYCLE_NUM_REG_OFFSET,
    0,
    0,
    599,
};

/******************************************************************************
 * Block: UBUS_SLV
 ******************************************************************************/
static const ru_reg_rec *UBUS_SLV_REGS[] =
{
    &UBUS_SLV_VPB_BASE_REG,
    &UBUS_SLV_VPB_MASK_REG,
    &UBUS_SLV_APB_BASE_REG,
    &UBUS_SLV_APB_MASK_REG,
    &UBUS_SLV_DQM_BASE_REG,
    &UBUS_SLV_DQM_MASK_REG,
    &UBUS_SLV_RNR_INTR_CTRL_ISR_REG,
    &UBUS_SLV_RNR_INTR_CTRL_ISM_REG,
    &UBUS_SLV_RNR_INTR_CTRL_IER_REG,
    &UBUS_SLV_RNR_INTR_CTRL_ITR_REG,
    &UBUS_SLV_PROFILING_CFG_REG,
    &UBUS_SLV_PROFILING_STATUS_REG,
    &UBUS_SLV_PROFILING_COUNTER_REG,
    &UBUS_SLV_PROFILING_START_VALUE_REG,
    &UBUS_SLV_PROFILING_STOP_VALUE_REG,
    &UBUS_SLV_PROFILING_CYCLE_NUM_REG,
};

unsigned long UBUS_SLV_ADDRS[] =
{
    0x82d2a000,
};

const ru_block_rec UBUS_SLV_BLOCK = 
{
    "UBUS_SLV",
    UBUS_SLV_ADDRS,
    1,
    16,
    UBUS_SLV_REGS
};

/* End of file XRDP_UBUS_SLV.c */
