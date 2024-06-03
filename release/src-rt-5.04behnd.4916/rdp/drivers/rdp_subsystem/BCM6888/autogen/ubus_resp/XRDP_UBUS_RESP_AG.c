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


#include "XRDP_UBUS_RESP_AG.h"

/******************************************************************************
 * Register: NAME: UBUS_RESP_PROFILING_CFG, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_PROFILING_PROFILING_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_ENABLE *****/
const ru_field_rec UBUS_RESP_PROFILING_CFG_COUNTER_ENABLE_FIELD =
{
    "COUNTER_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable free-running counter\n",
#endif
    { UBUS_RESP_PROFILING_CFG_COUNTER_ENABLE_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_CFG_COUNTER_ENABLE_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_CFG_COUNTER_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROFILING_START *****/
const ru_field_rec UBUS_RESP_PROFILING_CFG_PROFILING_START_FIELD =
{
    "PROFILING_START",
#if RU_INCLUDE_DESC
    "",
    "Start profiling window.\n",
#endif
    { UBUS_RESP_PROFILING_CFG_PROFILING_START_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_CFG_PROFILING_START_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_CFG_PROFILING_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MANUAL_STOP_MODE *****/
const ru_field_rec UBUS_RESP_PROFILING_CFG_MANUAL_STOP_MODE_FIELD =
{
    "MANUAL_STOP_MODE",
#if RU_INCLUDE_DESC
    "",
    "Enable manual stop mode\n",
#endif
    { UBUS_RESP_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DO_MANUAL_STOP *****/
const ru_field_rec UBUS_RESP_PROFILING_CFG_DO_MANUAL_STOP_FIELD =
{
    "DO_MANUAL_STOP",
#if RU_INCLUDE_DESC
    "",
    "Stop window now\n",
#endif
    { UBUS_RESP_PROFILING_CFG_DO_MANUAL_STOP_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_CFG_DO_MANUAL_STOP_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_CFG_DO_MANUAL_STOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_PROFILING_CFG_FIELDS[] =
{
    &UBUS_RESP_PROFILING_CFG_COUNTER_ENABLE_FIELD,
    &UBUS_RESP_PROFILING_CFG_PROFILING_START_FIELD,
    &UBUS_RESP_PROFILING_CFG_MANUAL_STOP_MODE_FIELD,
    &UBUS_RESP_PROFILING_CFG_DO_MANUAL_STOP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_PROFILING_CFG *****/
const ru_reg_rec UBUS_RESP_PROFILING_CFG_REG =
{
    "PROFILING_CFG",
#if RU_INCLUDE_DESC
    "PROFILING_CFG Register",
    "Profiling configuration settings\n",
#endif
    { UBUS_RESP_PROFILING_CFG_REG_OFFSET },
    0,
    0,
    1090,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UBUS_RESP_PROFILING_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_PROFILING_STATUS, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_PROFILING_PROFILING_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PROFILING_ON *****/
const ru_field_rec UBUS_RESP_PROFILING_STATUS_PROFILING_ON_FIELD =
{
    "PROFILING_ON",
#if RU_INCLUDE_DESC
    "",
    "Profiling is currently on\n",
#endif
    { UBUS_RESP_PROFILING_STATUS_PROFILING_ON_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_STATUS_PROFILING_ON_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_STATUS_PROFILING_ON_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CYCLES_COUNTER *****/
const ru_field_rec UBUS_RESP_PROFILING_STATUS_CYCLES_COUNTER_FIELD =
{
    "CYCLES_COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Current value of profiling window cycles counter (bits [30:0]\n",
#endif
    { UBUS_RESP_PROFILING_STATUS_CYCLES_COUNTER_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_STATUS_CYCLES_COUNTER_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_STATUS_CYCLES_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_PROFILING_STATUS_FIELDS[] =
{
    &UBUS_RESP_PROFILING_STATUS_PROFILING_ON_FIELD,
    &UBUS_RESP_PROFILING_STATUS_CYCLES_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_PROFILING_STATUS *****/
const ru_reg_rec UBUS_RESP_PROFILING_STATUS_REG =
{
    "PROFILING_STATUS",
#if RU_INCLUDE_DESC
    "PROFILING_STATUS Register",
    "Profiling status\n",
#endif
    { UBUS_RESP_PROFILING_STATUS_REG_OFFSET },
    0,
    0,
    1091,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UBUS_RESP_PROFILING_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_PROFILING_COUNTER, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_PROFILING_PROFILING_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec UBUS_RESP_PROFILING_COUNTER_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { UBUS_RESP_PROFILING_COUNTER_VAL_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_COUNTER_VAL_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_COUNTER_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_PROFILING_COUNTER_FIELDS[] =
{
    &UBUS_RESP_PROFILING_COUNTER_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_PROFILING_COUNTER *****/
const ru_reg_rec UBUS_RESP_PROFILING_COUNTER_REG =
{
    "PROFILING_COUNTER",
#if RU_INCLUDE_DESC
    "PROFILING_COUNTER Register",
    "Read PROFILING_COUNTER current value\n",
#endif
    { UBUS_RESP_PROFILING_COUNTER_REG_OFFSET },
    0,
    0,
    1092,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_PROFILING_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_PROFILING_START_VALUE, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_PROFILING_PROFILING_START_VALUE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec UBUS_RESP_PROFILING_START_VALUE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { UBUS_RESP_PROFILING_START_VALUE_VAL_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_START_VALUE_VAL_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_START_VALUE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_PROFILING_START_VALUE_FIELDS[] =
{
    &UBUS_RESP_PROFILING_START_VALUE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_PROFILING_START_VALUE *****/
const ru_reg_rec UBUS_RESP_PROFILING_START_VALUE_REG =
{
    "PROFILING_START_VALUE",
#if RU_INCLUDE_DESC
    "PROFILING_START_VALUE Register",
    "Read PROFILING_START_VALUE value\n",
#endif
    { UBUS_RESP_PROFILING_START_VALUE_REG_OFFSET },
    0,
    0,
    1093,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_PROFILING_START_VALUE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_PROFILING_STOP_VALUE, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_PROFILING_PROFILING_STOP_VALUE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec UBUS_RESP_PROFILING_STOP_VALUE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { UBUS_RESP_PROFILING_STOP_VALUE_VAL_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_STOP_VALUE_VAL_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_STOP_VALUE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_PROFILING_STOP_VALUE_FIELDS[] =
{
    &UBUS_RESP_PROFILING_STOP_VALUE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_PROFILING_STOP_VALUE *****/
const ru_reg_rec UBUS_RESP_PROFILING_STOP_VALUE_REG =
{
    "PROFILING_STOP_VALUE",
#if RU_INCLUDE_DESC
    "PROFILING_STOP_VALUE Register",
    "Read PROFILING_STOP_VALUE value\n",
#endif
    { UBUS_RESP_PROFILING_STOP_VALUE_REG_OFFSET },
    0,
    0,
    1094,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_PROFILING_STOP_VALUE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_PROFILING_CYCLE_NUM, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_PROFILING_PROFILING_CYCLE_NUM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PROFILING_CYCLES_NUM *****/
const ru_field_rec UBUS_RESP_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD =
{
    "PROFILING_CYCLES_NUM",
#if RU_INCLUDE_DESC
    "",
    "Length of profiling window in 500MHz clock cycles\n",
#endif
    { UBUS_RESP_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_MASK },
    0,
    { UBUS_RESP_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_WIDTH },
    { UBUS_RESP_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_PROFILING_CYCLE_NUM_FIELDS[] =
{
    &UBUS_RESP_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_PROFILING_CYCLE_NUM *****/
const ru_reg_rec UBUS_RESP_PROFILING_CYCLE_NUM_REG =
{
    "PROFILING_CYCLE_NUM",
#if RU_INCLUDE_DESC
    "PROFILING_CYCLE_NUM Register",
    "Set length of profiling window\n",
#endif
    { UBUS_RESP_PROFILING_CYCLE_NUM_REG_OFFSET },
    0,
    0,
    1095,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_PROFILING_CYCLE_NUM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RNR_INTR_CTRL_ISR, TYPE: Type_XRDP_UBUS_RESPONDER_GLBL_RNR_INTR_CTRL_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec UBUS_RESP_RNR_INTR_CTRL_ISR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "ISR - 32bit RNR INT\n",
#endif
    { UBUS_RESP_RNR_INTR_CTRL_ISR_IST_FIELD_MASK },
    0,
    { UBUS_RESP_RNR_INTR_CTRL_ISR_IST_FIELD_WIDTH },
    { UBUS_RESP_RNR_INTR_CTRL_ISR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RNR_INTR_CTRL_ISR_FIELDS[] =
{
    &UBUS_RESP_RNR_INTR_CTRL_ISR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RNR_INTR_CTRL_ISR *****/
const ru_reg_rec UBUS_RESP_RNR_INTR_CTRL_ISR_REG =
{
    "RNR_INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active QM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.\n",
#endif
    { UBUS_RESP_RNR_INTR_CTRL_ISR_REG_OFFSET },
    0,
    0,
    1096,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RNR_INTR_CTRL_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RNR_INTR_CTRL_ISM, TYPE: Type_XRDP_UBUS_RESPONDER_GLBL_RNR_INTR_CTRL_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ISM *****/
const ru_field_rec UBUS_RESP_RNR_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "",
    "Status Masked of corresponding interrupt source in the ISR\n",
#endif
    { UBUS_RESP_RNR_INTR_CTRL_ISM_ISM_FIELD_MASK },
    0,
    { UBUS_RESP_RNR_INTR_CTRL_ISM_ISM_FIELD_WIDTH },
    { UBUS_RESP_RNR_INTR_CTRL_ISM_ISM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RNR_INTR_CTRL_ISM_FIELDS[] =
{
    &UBUS_RESP_RNR_INTR_CTRL_ISM_ISM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RNR_INTR_CTRL_ISM *****/
const ru_reg_rec UBUS_RESP_RNR_INTR_CTRL_ISM_REG =
{
    "RNR_INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { UBUS_RESP_RNR_INTR_CTRL_ISM_REG_OFFSET },
    0,
    0,
    1097,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RNR_INTR_CTRL_ISM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RNR_INTR_CTRL_IER, TYPE: Type_XRDP_UBUS_RESPONDER_GLBL_RNR_INTR_CTRL_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IEM *****/
const ru_field_rec UBUS_RESP_RNR_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask controls the corresponding interrupt source in the IER\n",
#endif
    { UBUS_RESP_RNR_INTR_CTRL_IER_IEM_FIELD_MASK },
    0,
    { UBUS_RESP_RNR_INTR_CTRL_IER_IEM_FIELD_WIDTH },
    { UBUS_RESP_RNR_INTR_CTRL_IER_IEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RNR_INTR_CTRL_IER_FIELDS[] =
{
    &UBUS_RESP_RNR_INTR_CTRL_IER_IEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RNR_INTR_CTRL_IER *****/
const ru_reg_rec UBUS_RESP_RNR_INTR_CTRL_IER_REG =
{
    "RNR_INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { UBUS_RESP_RNR_INTR_CTRL_IER_REG_OFFSET },
    0,
    0,
    1098,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RNR_INTR_CTRL_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RNR_INTR_CTRL_ITR, TYPE: Type_XRDP_UBUS_RESPONDER_GLBL_RNR_INTR_CTRL_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec UBUS_RESP_RNR_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask tests the corresponding interrupt source in the ISR\n",
#endif
    { UBUS_RESP_RNR_INTR_CTRL_ITR_IST_FIELD_MASK },
    0,
    { UBUS_RESP_RNR_INTR_CTRL_ITR_IST_FIELD_WIDTH },
    { UBUS_RESP_RNR_INTR_CTRL_ITR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RNR_INTR_CTRL_ITR_FIELDS[] =
{
    &UBUS_RESP_RNR_INTR_CTRL_ITR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RNR_INTR_CTRL_ITR *****/
const ru_reg_rec UBUS_RESP_RNR_INTR_CTRL_ITR_REG =
{
    "RNR_INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR\n",
#endif
    { UBUS_RESP_RNR_INTR_CTRL_ITR_REG_OFFSET },
    0,
    0,
    1099,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RNR_INTR_CTRL_ITR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_VPB_START, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_VPB_START
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START *****/
const ru_field_rec UBUS_RESP_VPB_START_START_FIELD =
{
    "START",
#if RU_INCLUDE_DESC
    "",
    "Start address\n",
#endif
    { UBUS_RESP_VPB_START_START_FIELD_MASK },
    0,
    { UBUS_RESP_VPB_START_START_FIELD_WIDTH },
    { UBUS_RESP_VPB_START_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_VPB_START_FIELDS[] =
{
    &UBUS_RESP_VPB_START_START_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_VPB_START *****/
const ru_reg_rec UBUS_RESP_VPB_START_REG =
{
    "VPB_START",
#if RU_INCLUDE_DESC
    "VPB_START Register",
    "VPB start address\n",
#endif
    { UBUS_RESP_VPB_START_REG_OFFSET },
    0,
    0,
    1100,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_VPB_START_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_VPB_END, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_VPB_END
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END *****/
const ru_field_rec UBUS_RESP_VPB_END_END_FIELD =
{
    "END",
#if RU_INCLUDE_DESC
    "",
    "end addresws\n",
#endif
    { UBUS_RESP_VPB_END_END_FIELD_MASK },
    0,
    { UBUS_RESP_VPB_END_END_FIELD_WIDTH },
    { UBUS_RESP_VPB_END_END_FIELD_SHIFT },
    4294967295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_VPB_END_FIELDS[] =
{
    &UBUS_RESP_VPB_END_END_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_VPB_END *****/
const ru_reg_rec UBUS_RESP_VPB_END_REG =
{
    "VPB_END",
#if RU_INCLUDE_DESC
    "VPB_END Register",
    "VPB endaddress\n",
#endif
    { UBUS_RESP_VPB_END_REG_OFFSET },
    0,
    0,
    1101,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_VPB_END_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_APB_START, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_APB_START
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START *****/
const ru_field_rec UBUS_RESP_APB_START_START_FIELD =
{
    "START",
#if RU_INCLUDE_DESC
    "",
    "Start address\n",
#endif
    { UBUS_RESP_APB_START_START_FIELD_MASK },
    0,
    { UBUS_RESP_APB_START_START_FIELD_WIDTH },
    { UBUS_RESP_APB_START_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_APB_START_FIELDS[] =
{
    &UBUS_RESP_APB_START_START_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_APB_START *****/
const ru_reg_rec UBUS_RESP_APB_START_REG =
{
    "APB_START",
#if RU_INCLUDE_DESC
    "APB_START Register",
    "APB start address\n",
#endif
    { UBUS_RESP_APB_START_REG_OFFSET },
    0,
    0,
    1102,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_APB_START_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_APB_END, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_APB_END
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END *****/
const ru_field_rec UBUS_RESP_APB_END_END_FIELD =
{
    "END",
#if RU_INCLUDE_DESC
    "",
    "end addresws\n",
#endif
    { UBUS_RESP_APB_END_END_FIELD_MASK },
    0,
    { UBUS_RESP_APB_END_END_FIELD_WIDTH },
    { UBUS_RESP_APB_END_END_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_APB_END_FIELDS[] =
{
    &UBUS_RESP_APB_END_END_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_APB_END *****/
const ru_reg_rec UBUS_RESP_APB_END_REG =
{
    "APB_END",
#if RU_INCLUDE_DESC
    "APB_END Register",
    "APB end address\n",
#endif
    { UBUS_RESP_APB_END_REG_OFFSET },
    0,
    0,
    1103,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_APB_END_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_DEVICE_0_START, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_DEVICE_0_START
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START *****/
const ru_field_rec UBUS_RESP_DEVICE_0_START_START_FIELD =
{
    "START",
#if RU_INCLUDE_DESC
    "",
    "Start address\n",
#endif
    { UBUS_RESP_DEVICE_0_START_START_FIELD_MASK },
    0,
    { UBUS_RESP_DEVICE_0_START_START_FIELD_WIDTH },
    { UBUS_RESP_DEVICE_0_START_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_DEVICE_0_START_FIELDS[] =
{
    &UBUS_RESP_DEVICE_0_START_START_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_DEVICE_0_START *****/
const ru_reg_rec UBUS_RESP_DEVICE_0_START_REG =
{
    "DEVICE_0_START",
#if RU_INCLUDE_DESC
    "DEVICE_0_START Register",
    "Device 0 start address\n",
#endif
    { UBUS_RESP_DEVICE_0_START_REG_OFFSET },
    0,
    0,
    1104,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_DEVICE_0_START_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_DEVICE_0_END, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_DEVICE_0_END
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END *****/
const ru_field_rec UBUS_RESP_DEVICE_0_END_END_FIELD =
{
    "END",
#if RU_INCLUDE_DESC
    "",
    "end addresws\n",
#endif
    { UBUS_RESP_DEVICE_0_END_END_FIELD_MASK },
    0,
    { UBUS_RESP_DEVICE_0_END_END_FIELD_WIDTH },
    { UBUS_RESP_DEVICE_0_END_END_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_DEVICE_0_END_FIELDS[] =
{
    &UBUS_RESP_DEVICE_0_END_END_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_DEVICE_0_END *****/
const ru_reg_rec UBUS_RESP_DEVICE_0_END_REG =
{
    "DEVICE_0_END",
#if RU_INCLUDE_DESC
    "DEVICE_0_END Register",
    "device end address\n",
#endif
    { UBUS_RESP_DEVICE_0_END_REG_OFFSET },
    0,
    0,
    1105,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_DEVICE_0_END_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_DEVICE_1_START, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_DEVICE_1_START
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START *****/
const ru_field_rec UBUS_RESP_DEVICE_1_START_START_FIELD =
{
    "START",
#if RU_INCLUDE_DESC
    "",
    "Start address\n",
#endif
    { UBUS_RESP_DEVICE_1_START_START_FIELD_MASK },
    0,
    { UBUS_RESP_DEVICE_1_START_START_FIELD_WIDTH },
    { UBUS_RESP_DEVICE_1_START_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_DEVICE_1_START_FIELDS[] =
{
    &UBUS_RESP_DEVICE_1_START_START_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_DEVICE_1_START *****/
const ru_reg_rec UBUS_RESP_DEVICE_1_START_REG =
{
    "DEVICE_1_START",
#if RU_INCLUDE_DESC
    "DEVICE_1_START Register",
    "Device 1 start address\n",
#endif
    { UBUS_RESP_DEVICE_1_START_REG_OFFSET },
    0,
    0,
    1106,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_DEVICE_1_START_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_DEVICE_1_END, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_DEVICE_1_END
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END *****/
const ru_field_rec UBUS_RESP_DEVICE_1_END_END_FIELD =
{
    "END",
#if RU_INCLUDE_DESC
    "",
    "end addresws\n",
#endif
    { UBUS_RESP_DEVICE_1_END_END_FIELD_MASK },
    0,
    { UBUS_RESP_DEVICE_1_END_END_FIELD_WIDTH },
    { UBUS_RESP_DEVICE_1_END_END_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_DEVICE_1_END_FIELDS[] =
{
    &UBUS_RESP_DEVICE_1_END_END_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_DEVICE_1_END *****/
const ru_reg_rec UBUS_RESP_DEVICE_1_END_REG =
{
    "DEVICE_1_END",
#if RU_INCLUDE_DESC
    "DEVICE_1_END Register",
    "device end address\n",
#endif
    { UBUS_RESP_DEVICE_1_END_REG_OFFSET },
    0,
    0,
    1107,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_DEVICE_1_END_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_DEVICE_2_START, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_DEVICE_2_START
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START *****/
const ru_field_rec UBUS_RESP_DEVICE_2_START_START_FIELD =
{
    "START",
#if RU_INCLUDE_DESC
    "",
    "Start address\n",
#endif
    { UBUS_RESP_DEVICE_2_START_START_FIELD_MASK },
    0,
    { UBUS_RESP_DEVICE_2_START_START_FIELD_WIDTH },
    { UBUS_RESP_DEVICE_2_START_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_DEVICE_2_START_FIELDS[] =
{
    &UBUS_RESP_DEVICE_2_START_START_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_DEVICE_2_START *****/
const ru_reg_rec UBUS_RESP_DEVICE_2_START_REG =
{
    "DEVICE_2_START",
#if RU_INCLUDE_DESC
    "DEVICE_2_START Register",
    "Device 2 start address\n",
#endif
    { UBUS_RESP_DEVICE_2_START_REG_OFFSET },
    0,
    0,
    1108,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_DEVICE_2_START_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_DEVICE_2_END, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RSPNDR_DEVICE_2_END
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END *****/
const ru_field_rec UBUS_RESP_DEVICE_2_END_END_FIELD =
{
    "END",
#if RU_INCLUDE_DESC
    "",
    "end addresws\n",
#endif
    { UBUS_RESP_DEVICE_2_END_END_FIELD_MASK },
    0,
    { UBUS_RESP_DEVICE_2_END_END_FIELD_WIDTH },
    { UBUS_RESP_DEVICE_2_END_END_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_DEVICE_2_END_FIELDS[] =
{
    &UBUS_RESP_DEVICE_2_END_END_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_DEVICE_2_END *****/
const ru_reg_rec UBUS_RESP_DEVICE_2_END_REG =
{
    "DEVICE_2_END",
#if RU_INCLUDE_DESC
    "DEVICE_2_END Register",
    "device end address\n",
#endif
    { UBUS_RESP_DEVICE_2_END_REG_OFFSET },
    0,
    0,
    1109,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_DEVICE_2_END_FIELDS,
#endif
};

unsigned long UBUS_RESP_ADDRS[] =
{
    0x828A0000,
};

static const ru_reg_rec *UBUS_RESP_REGS[] =
{
    &UBUS_RESP_VPB_START_REG,
    &UBUS_RESP_VPB_END_REG,
    &UBUS_RESP_APB_START_REG,
    &UBUS_RESP_APB_END_REG,
    &UBUS_RESP_DEVICE_0_START_REG,
    &UBUS_RESP_DEVICE_0_END_REG,
    &UBUS_RESP_DEVICE_1_START_REG,
    &UBUS_RESP_DEVICE_1_END_REG,
    &UBUS_RESP_DEVICE_2_START_REG,
    &UBUS_RESP_DEVICE_2_END_REG,
    &UBUS_RESP_RNR_INTR_CTRL_ISR_REG,
    &UBUS_RESP_RNR_INTR_CTRL_ISM_REG,
    &UBUS_RESP_RNR_INTR_CTRL_IER_REG,
    &UBUS_RESP_RNR_INTR_CTRL_ITR_REG,
    &UBUS_RESP_PROFILING_CFG_REG,
    &UBUS_RESP_PROFILING_STATUS_REG,
    &UBUS_RESP_PROFILING_COUNTER_REG,
    &UBUS_RESP_PROFILING_START_VALUE_REG,
    &UBUS_RESP_PROFILING_STOP_VALUE_REG,
    &UBUS_RESP_PROFILING_CYCLE_NUM_REG,
};

const ru_block_rec UBUS_RESP_BLOCK =
{
    "UBUS_RESP",
    UBUS_RESP_ADDRS,
    1,
    20,
    UBUS_RESP_REGS,
};
