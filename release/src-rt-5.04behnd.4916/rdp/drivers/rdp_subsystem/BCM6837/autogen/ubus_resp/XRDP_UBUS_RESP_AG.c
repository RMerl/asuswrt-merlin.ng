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
    1073,
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
    1074,
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
    1075,
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
    1076,
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
    1077,
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
    1078,
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
    1079,
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
    1080,
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
    1081,
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
    1082,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RNR_INTR_CTRL_ITR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_LOCK, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_LOCK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RCHK_LOCK *****/
const ru_field_rec UBUS_RESP_RCHK_LOCK_RCHK_LOCK_FIELD =
{
    "RCHK_LOCK",
#if RU_INCLUDE_DESC
    "",
    "Range checker cfg lock bit\n",
#endif
    { UBUS_RESP_RCHK_LOCK_RCHK_LOCK_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_LOCK_RCHK_LOCK_FIELD_WIDTH },
    { UBUS_RESP_RCHK_LOCK_RCHK_LOCK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_LOCK_FIELDS[] =
{
    &UBUS_RESP_RCHK_LOCK_RCHK_LOCK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_LOCK *****/
const ru_reg_rec UBUS_RESP_RCHK_LOCK_REG =
{
    "RCHK_LOCK",
#if RU_INCLUDE_DESC
    "RCHK_LOCK Register",
    "Range checker configuration lock bit\n",
#endif
    { UBUS_RESP_RCHK_LOCK_REG_OFFSET },
    0,
    0,
    1083,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RCHK_LOCK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_ENG_CTRL, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_ENG_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WR_ABORT *****/
const ru_field_rec UBUS_RESP_RCHK_ENG_CTRL_WR_ABORT_FIELD =
{
    "WR_ABORT",
#if RU_INCLUDE_DESC
    "",
    "Abort write command if engine params match\n",
#endif
    { UBUS_RESP_RCHK_ENG_CTRL_WR_ABORT_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ENG_CTRL_WR_ABORT_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ENG_CTRL_WR_ABORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_ABORT *****/
const ru_field_rec UBUS_RESP_RCHK_ENG_CTRL_RD_ABORT_FIELD =
{
    "RD_ABORT",
#if RU_INCLUDE_DESC
    "",
    "Abort read command if engine params match\n",
#endif
    { UBUS_RESP_RCHK_ENG_CTRL_RD_ABORT_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ENG_CTRL_RD_ABORT_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ENG_CTRL_RD_ABORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROT *****/
const ru_field_rec UBUS_RESP_RCHK_ENG_CTRL_PROT_FIELD =
{
    "PROT",
#if RU_INCLUDE_DESC
    "",
    "PROT matching field\n",
#endif
    { UBUS_RESP_RCHK_ENG_CTRL_PROT_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ENG_CTRL_PROT_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ENG_CTRL_PROT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROT_MSK *****/
const ru_field_rec UBUS_RESP_RCHK_ENG_CTRL_PROT_MSK_FIELD =
{
    "PROT_MSK",
#if RU_INCLUDE_DESC
    "",
    "PROT Mask field to be compared with PROT field\n",
#endif
    { UBUS_RESP_RCHK_ENG_CTRL_PROT_MSK_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ENG_CTRL_PROT_MSK_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ENG_CTRL_PROT_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_ENG_CTRL_FIELDS[] =
{
    &UBUS_RESP_RCHK_ENG_CTRL_WR_ABORT_FIELD,
    &UBUS_RESP_RCHK_ENG_CTRL_RD_ABORT_FIELD,
    &UBUS_RESP_RCHK_ENG_CTRL_PROT_FIELD,
    &UBUS_RESP_RCHK_ENG_CTRL_PROT_MSK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_ENG_CTRL *****/
const ru_reg_rec UBUS_RESP_RCHK_ENG_CTRL_REG =
{
    "RCHK_ENG_CTRL",
#if RU_INCLUDE_DESC
    "RCHK_ENG_CTRL 0..7 Register",
    "Range Checker engine ctrl\n",
#endif
    { UBUS_RESP_RCHK_ENG_CTRL_REG_OFFSET },
    UBUS_RESP_RCHK_ENG_CTRL_REG_RAM_CNT,
    4,
    1084,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UBUS_RESP_RCHK_ENG_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_ENG_START_ADD, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_ENG_START_ADD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START_ADD *****/
const ru_field_rec UBUS_RESP_RCHK_ENG_START_ADD_START_ADD_FIELD =
{
    "START_ADD",
#if RU_INCLUDE_DESC
    "",
    "Range checker engine start address\n",
#endif
    { UBUS_RESP_RCHK_ENG_START_ADD_START_ADD_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ENG_START_ADD_START_ADD_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ENG_START_ADD_START_ADD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_ENG_START_ADD_FIELDS[] =
{
    &UBUS_RESP_RCHK_ENG_START_ADD_START_ADD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_ENG_START_ADD *****/
const ru_reg_rec UBUS_RESP_RCHK_ENG_START_ADD_REG =
{
    "RCHK_ENG_START_ADD",
#if RU_INCLUDE_DESC
    "RCHK_ENG_START_ADD 0..7 Register",
    "Range Checker eng Start Address\n",
#endif
    { UBUS_RESP_RCHK_ENG_START_ADD_REG_OFFSET },
    UBUS_RESP_RCHK_ENG_START_ADD_REG_RAM_CNT,
    4,
    1085,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RCHK_ENG_START_ADD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_ENG_END_ADD, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_ENG_END_ADD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END_ADD *****/
const ru_field_rec UBUS_RESP_RCHK_ENG_END_ADD_END_ADD_FIELD =
{
    "END_ADD",
#if RU_INCLUDE_DESC
    "",
    "Range checker engine end address\n",
#endif
    { UBUS_RESP_RCHK_ENG_END_ADD_END_ADD_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ENG_END_ADD_END_ADD_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ENG_END_ADD_END_ADD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_ENG_END_ADD_FIELDS[] =
{
    &UBUS_RESP_RCHK_ENG_END_ADD_END_ADD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_ENG_END_ADD *****/
const ru_reg_rec UBUS_RESP_RCHK_ENG_END_ADD_REG =
{
    "RCHK_ENG_END_ADD",
#if RU_INCLUDE_DESC
    "RCHK_ENG_END_ADD 0..7 Register",
    "Range Checker engine End Address\n",
#endif
    { UBUS_RESP_RCHK_ENG_END_ADD_REG_OFFSET },
    UBUS_RESP_RCHK_ENG_END_ADD_REG_RAM_CNT,
    4,
    1086,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RCHK_ENG_END_ADD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_ENG_SECLEV_EN, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_ENG_SECLEV_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SECLEV_EN *****/
const ru_field_rec UBUS_RESP_RCHK_ENG_SECLEV_EN_SECLEV_EN_FIELD =
{
    "SECLEV_EN",
#if RU_INCLUDE_DESC
    "",
    "Ranbge checker engine seclev enable\n",
#endif
    { UBUS_RESP_RCHK_ENG_SECLEV_EN_SECLEV_EN_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ENG_SECLEV_EN_SECLEV_EN_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ENG_SECLEV_EN_SECLEV_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_ENG_SECLEV_EN_FIELDS[] =
{
    &UBUS_RESP_RCHK_ENG_SECLEV_EN_SECLEV_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_ENG_SECLEV_EN *****/
const ru_reg_rec UBUS_RESP_RCHK_ENG_SECLEV_EN_REG =
{
    "RCHK_ENG_SECLEV_EN",
#if RU_INCLUDE_DESC
    "RCHK_ENG_SECLEV 0..7 Register",
    "Range Checker eng Seclev enable\n",
#endif
    { UBUS_RESP_RCHK_ENG_SECLEV_EN_REG_OFFSET },
    UBUS_RESP_RCHK_ENG_SECLEV_EN_REG_RAM_CNT,
    4,
    1087,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RCHK_ENG_SECLEV_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_RNR_EN, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_RNR_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RCHK_RNR_EN *****/
const ru_field_rec UBUS_RESP_RCHK_RNR_EN_RCHK_RNR_EN_FIELD =
{
    "RCHK_RNR_EN",
#if RU_INCLUDE_DESC
    "",
    "Range Checker Runner enable mapping\n",
#endif
    { UBUS_RESP_RCHK_RNR_EN_RCHK_RNR_EN_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_RNR_EN_RCHK_RNR_EN_FIELD_WIDTH },
    { UBUS_RESP_RCHK_RNR_EN_RCHK_RNR_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_RNR_EN_FIELDS[] =
{
    &UBUS_RESP_RCHK_RNR_EN_RCHK_RNR_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_RNR_EN *****/
const ru_reg_rec UBUS_RESP_RCHK_RNR_EN_REG =
{
    "RCHK_RNR_EN",
#if RU_INCLUDE_DESC
    "RCHK_RNR 0..7 Register",
    "Range Checker Runner enable mapping\n",
#endif
    { UBUS_RESP_RCHK_RNR_EN_REG_OFFSET },
    UBUS_RESP_RCHK_RNR_EN_REG_RAM_CNT,
    4,
    1088,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RCHK_RNR_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_ABORT_CAPT0, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_ABORT_CAPT0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RCHK_ABORT_CAPT0 *****/
const ru_field_rec UBUS_RESP_RCHK_ABORT_CAPT0_RCHK_ABORT_CAPT0_FIELD =
{
    "RCHK_ABORT_CAPT0",
#if RU_INCLUDE_DESC
    "",
    "Controls capture during abort:\n[31:31] - Abort indication\n[21:21] - Trans rwb\n[20:16] - runner accessed\n[15:13] - Trans prot\n[12:8]  - Trans seclev\n[7:0]   - Trans srcid\n",
#endif
    { UBUS_RESP_RCHK_ABORT_CAPT0_RCHK_ABORT_CAPT0_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ABORT_CAPT0_RCHK_ABORT_CAPT0_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ABORT_CAPT0_RCHK_ABORT_CAPT0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_ABORT_CAPT0_FIELDS[] =
{
    &UBUS_RESP_RCHK_ABORT_CAPT0_RCHK_ABORT_CAPT0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_ABORT_CAPT0 *****/
const ru_reg_rec UBUS_RESP_RCHK_ABORT_CAPT0_REG =
{
    "RCHK_ABORT_CAPT0",
#if RU_INCLUDE_DESC
    "RCHK_ABORT_CAPT0 Register",
    "Range Checker Abort capture status\n",
#endif
    { UBUS_RESP_RCHK_ABORT_CAPT0_REG_OFFSET },
    0,
    0,
    1089,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RCHK_ABORT_CAPT0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_ABORT_CAPT1, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_ABORT_CAPT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RCHK_ABORT_CAPT1 *****/
const ru_field_rec UBUS_RESP_RCHK_ABORT_CAPT1_RCHK_ABORT_CAPT1_FIELD =
{
    "RCHK_ABORT_CAPT1",
#if RU_INCLUDE_DESC
    "",
    "Address high bits [31:0] capture during abort\n",
#endif
    { UBUS_RESP_RCHK_ABORT_CAPT1_RCHK_ABORT_CAPT1_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ABORT_CAPT1_RCHK_ABORT_CAPT1_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ABORT_CAPT1_RCHK_ABORT_CAPT1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_ABORT_CAPT1_FIELDS[] =
{
    &UBUS_RESP_RCHK_ABORT_CAPT1_RCHK_ABORT_CAPT1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_ABORT_CAPT1 *****/
const ru_reg_rec UBUS_RESP_RCHK_ABORT_CAPT1_REG =
{
    "RCHK_ABORT_CAPT1",
#if RU_INCLUDE_DESC
    "RCHK_ABORT_CAPT1 Register",
    "Range Checker Abort capture status\n",
#endif
    { UBUS_RESP_RCHK_ABORT_CAPT1_REG_OFFSET },
    0,
    0,
    1090,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RCHK_ABORT_CAPT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_RESP_RCHK_ABORT_CAPT2, TYPE: Type_XRDP_UBUS_RESPONDER_XRDP_UBUS_RCHK_RCHK_ABORT_CAPT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RCHK_ABORT_CAPT2 *****/
const ru_field_rec UBUS_RESP_RCHK_ABORT_CAPT2_RCHK_ABORT_CAPT2_FIELD =
{
    "RCHK_ABORT_CAPT2",
#if RU_INCLUDE_DESC
    "",
    "Address low bits [39:32] capture during abort\n",
#endif
    { UBUS_RESP_RCHK_ABORT_CAPT2_RCHK_ABORT_CAPT2_FIELD_MASK },
    0,
    { UBUS_RESP_RCHK_ABORT_CAPT2_RCHK_ABORT_CAPT2_FIELD_WIDTH },
    { UBUS_RESP_RCHK_ABORT_CAPT2_RCHK_ABORT_CAPT2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_RESP_RCHK_ABORT_CAPT2_FIELDS[] =
{
    &UBUS_RESP_RCHK_ABORT_CAPT2_RCHK_ABORT_CAPT2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_RESP_RCHK_ABORT_CAPT2 *****/
const ru_reg_rec UBUS_RESP_RCHK_ABORT_CAPT2_REG =
{
    "RCHK_ABORT_CAPT2",
#if RU_INCLUDE_DESC
    "RCHK_ABORT_CAPT2 Register",
    "Range Checker Abort capture status\n",
#endif
    { UBUS_RESP_RCHK_ABORT_CAPT2_REG_OFFSET },
    0,
    0,
    1091,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_RESP_RCHK_ABORT_CAPT2_FIELDS,
#endif
};

unsigned long UBUS_RESP_ADDRS[] =
{
    0x828A0000,
};

static const ru_reg_rec *UBUS_RESP_REGS[] =
{
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
    &UBUS_RESP_RCHK_LOCK_REG,
    &UBUS_RESP_RCHK_ENG_CTRL_REG,
    &UBUS_RESP_RCHK_ENG_START_ADD_REG,
    &UBUS_RESP_RCHK_ENG_END_ADD_REG,
    &UBUS_RESP_RCHK_ENG_SECLEV_EN_REG,
    &UBUS_RESP_RCHK_RNR_EN_REG,
    &UBUS_RESP_RCHK_ABORT_CAPT0_REG,
    &UBUS_RESP_RCHK_ABORT_CAPT1_REG,
    &UBUS_RESP_RCHK_ABORT_CAPT2_REG,
};

const ru_block_rec UBUS_RESP_BLOCK =
{
    "UBUS_RESP",
    UBUS_RESP_ADDRS,
    1,
    19,
    UBUS_RESP_REGS,
};
