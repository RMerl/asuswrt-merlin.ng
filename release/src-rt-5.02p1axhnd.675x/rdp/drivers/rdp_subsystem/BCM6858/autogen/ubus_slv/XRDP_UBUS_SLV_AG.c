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
 * Field: UBUS_SLV_VPB_BASE_BASE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_VPB_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    UBUS_SLV_VPB_BASE_BASE_FIELD_MASK,
    0,
    UBUS_SLV_VPB_BASE_BASE_FIELD_WIDTH,
    UBUS_SLV_VPB_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_VPB_MASK_MASK
 ******************************************************************************/
const ru_field_rec UBUS_SLV_VPB_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    UBUS_SLV_VPB_MASK_MASK_FIELD_MASK,
    0,
    UBUS_SLV_VPB_MASK_MASK_FIELD_WIDTH,
    UBUS_SLV_VPB_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_APB_BASE_BASE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_APB_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    UBUS_SLV_APB_BASE_BASE_FIELD_MASK,
    0,
    UBUS_SLV_APB_BASE_BASE_FIELD_WIDTH,
    UBUS_SLV_APB_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_APB_MASK_MASK
 ******************************************************************************/
const ru_field_rec UBUS_SLV_APB_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    UBUS_SLV_APB_MASK_MASK_FIELD_MASK,
    0,
    UBUS_SLV_APB_MASK_MASK_FIELD_WIDTH,
    UBUS_SLV_APB_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_DQM_BASE_BASE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_DQM_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    UBUS_SLV_DQM_BASE_BASE_FIELD_MASK,
    0,
    UBUS_SLV_DQM_BASE_BASE_FIELD_WIDTH,
    UBUS_SLV_DQM_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_DQM_MASK_MASK
 ******************************************************************************/
const ru_field_rec UBUS_SLV_DQM_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    UBUS_SLV_DQM_MASK_MASK_FIELD_MASK,
    0,
    UBUS_SLV_DQM_MASK_MASK_FIELD_WIDTH,
    UBUS_SLV_DQM_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_RNR_INTR_CTRL_ISR_IST
 ******************************************************************************/
const ru_field_rec UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "ISR",
    "ISR - 32bit RNR INT",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD_MASK,
    0,
    UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD_WIDTH,
    UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_RNR_INTR_CTRL_ISM_ISM
 ******************************************************************************/
const ru_field_rec UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "Interrupt_status_masked",
    "Status Masked of corresponding interrupt source in the ISR",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD_MASK,
    0,
    UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD_WIDTH,
    UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_RNR_INTR_CTRL_IER_IEM
 ******************************************************************************/
const ru_field_rec UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "Interrupt_enable_mask",
    "Each bit in the mask controls the corresponding interrupt source in the IER",
#endif
    UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD_MASK,
    0,
    UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD_WIDTH,
    UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_RNR_INTR_CTRL_ITR_IST
 ******************************************************************************/
const ru_field_rec UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "Interrupt_simulation_test",
    "Each bit in the mask tests the corresponding interrupt source in the ISR",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD_MASK,
    0,
    UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD_WIDTH,
    UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD =
{
    "COUNTER_ENABLE",
#if RU_INCLUDE_DESC
    "COUNTER_ENABLE",
    "Enable free-running counter",
#endif
    UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_PROFILING_START
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD =
{
    "PROFILING_START",
#if RU_INCLUDE_DESC
    "PROFILING_START",
    "Start profiling window.",
#endif
    UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD =
{
    "MANUAL_STOP_MODE",
#if RU_INCLUDE_DESC
    "MANUAL_STOP_MODE",
    "Start profiling window.",
#endif
    UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD =
{
    "DO_MANUAL_STOP",
#if RU_INCLUDE_DESC
    "DO_MANUAL_STOP",
    "Stop window now",
#endif
    UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_STATUS_PROFILING_ON
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD =
{
    "PROFILING_ON",
#if RU_INCLUDE_DESC
    "PROFILING_ON",
    "Profiling is currently on",
#endif
    UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD_WIDTH,
    UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD =
{
    "CYCLES_COUNTER",
#if RU_INCLUDE_DESC
    "CYCLES_COUNTER",
    "Current value of profiling window cycles counter",
#endif
    UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD_WIDTH,
    UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_COUNTER_VAL
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_COUNTER_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Val",
    "Value",
#endif
    UBUS_SLV_PROFILING_COUNTER_VAL_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_COUNTER_VAL_FIELD_WIDTH,
    UBUS_SLV_PROFILING_COUNTER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_START_VALUE_VAL
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Val",
    "Value",
#endif
    UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD_WIDTH,
    UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_STOP_VALUE_VAL
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Val",
    "Value",
#endif
    UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD_WIDTH,
    UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD =
{
    "PROFILING_CYCLES_NUM",
#if RU_INCLUDE_DESC
    "PROFILING_CYCLES_NUM",
    "Length of profiling window in 500MHz clock cycles",
#endif
    UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: UBUS_SLV_VPB_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_VPB_BASE_FIELDS[] =
{
    &UBUS_SLV_VPB_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_VPB_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_VPB_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_VPB_MASK_FIELDS[] =
{
    &UBUS_SLV_VPB_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_VPB_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_APB_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_APB_BASE_FIELDS[] =
{
    &UBUS_SLV_APB_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_APB_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_APB_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_APB_MASK_FIELDS[] =
{
    &UBUS_SLV_APB_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_APB_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_DQM_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_DQM_BASE_FIELDS[] =
{
    &UBUS_SLV_DQM_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_DQM_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_DQM_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_DQM_MASK_FIELDS[] =
{
    &UBUS_SLV_DQM_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_DQM_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ISR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_RNR_INTR_CTRL_ISR_FIELDS[] =
{
    &UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_RNR_INTR_CTRL_ISR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ISM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_RNR_INTR_CTRL_ISM_FIELDS[] =
{
    &UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_RNR_INTR_CTRL_ISM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_IER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_RNR_INTR_CTRL_IER_FIELDS[] =
{
    &UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_RNR_INTR_CTRL_IER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ITR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_RNR_INTR_CTRL_ITR_FIELDS[] =
{
    &UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_RNR_INTR_CTRL_ITR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_CFG_FIELDS[] =
{
    &UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD,
    &UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD,
    &UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD,
    &UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD,
    &UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    UBUS_SLV_PROFILING_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_STATUS_FIELDS[] =
{
    &UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD,
    &UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UBUS_SLV_PROFILING_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_COUNTER_FIELDS[] =
{
    &UBUS_SLV_PROFILING_COUNTER_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_PROFILING_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_START_VALUE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_START_VALUE_FIELDS[] =
{
    &UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_PROFILING_START_VALUE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_STOP_VALUE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_STOP_VALUE_FIELDS[] =
{
    &UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_PROFILING_STOP_VALUE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_CYCLE_NUM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_CYCLE_NUM_FIELDS[] =
{
    &UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_PROFILING_CYCLE_NUM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
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
