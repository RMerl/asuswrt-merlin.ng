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


#include "XRDP_GSU_REG_AG.h"

/******************************************************************************
 * Register: NAME: GSU_REG_PROFILING_DOWNCNT, TYPE: Type_GSU_REGFILE_CFG_PROFILING_DOWNCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DOWNCNT_VALUE *****/
const ru_field_rec GSU_REG_PROFILING_DOWNCNT_DOWNCNT_VALUE_FIELD =
{
    "DOWNCNT_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Upon write, load value into downcounter and start profiling window.\nRead returns current value of the downcounter.\n",
#endif
    { GSU_REG_PROFILING_DOWNCNT_DOWNCNT_VALUE_FIELD_MASK },
    0,
    { GSU_REG_PROFILING_DOWNCNT_DOWNCNT_VALUE_FIELD_WIDTH },
    { GSU_REG_PROFILING_DOWNCNT_DOWNCNT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PROFILING_DOWNCNT_FIELDS[] =
{
    &GSU_REG_PROFILING_DOWNCNT_DOWNCNT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PROFILING_DOWNCNT *****/
const ru_reg_rec GSU_REG_PROFILING_DOWNCNT_REG =
{
    "PROFILING_DOWNCNT",
#if RU_INCLUDE_DESC
    "PROFILING_DOWNCNT Register",
    "Writing to this counter loads the written value into down counter, which immediately starts decrementing by 1 each clock, until reaching 0. The profiling window is open while downcounter is decrementing, when it reaches 0 the profiling window ends\n",
#endif
    { GSU_REG_PROFILING_DOWNCNT_REG_OFFSET },
    0,
    0,
    1510,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_PROFILING_DOWNCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_PROFILING_COUNTER_A, TYPE: Type_GSU_REGFILE_CFG_PROFILING_COUNTER_A
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec GSU_REG_PROFILING_COUNTER_A_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "Profiling counter A\n",
#endif
    { GSU_REG_PROFILING_COUNTER_A_VALUE_FIELD_MASK },
    0,
    { GSU_REG_PROFILING_COUNTER_A_VALUE_FIELD_WIDTH },
    { GSU_REG_PROFILING_COUNTER_A_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PROFILING_COUNTER_A_FIELDS[] =
{
    &GSU_REG_PROFILING_COUNTER_A_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PROFILING_COUNTER_A *****/
const ru_reg_rec GSU_REG_PROFILING_COUNTER_A_REG =
{
    "PROFILING_COUNTER_A",
#if RU_INCLUDE_DESC
    "PROFILING_COUNTER_A Register",
    "Profiling counter A - read only counter, meaning of the value depends on the selected profiling mode\n",
#endif
    { GSU_REG_PROFILING_COUNTER_A_REG_OFFSET },
    0,
    0,
    1511,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_PROFILING_COUNTER_A_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_PROFILING_COUNTER_B, TYPE: Type_GSU_REGFILE_CFG_PROFILING_COUNTER_B
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec GSU_REG_PROFILING_COUNTER_B_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "Profiling counter B\n",
#endif
    { GSU_REG_PROFILING_COUNTER_B_VALUE_FIELD_MASK },
    0,
    { GSU_REG_PROFILING_COUNTER_B_VALUE_FIELD_WIDTH },
    { GSU_REG_PROFILING_COUNTER_B_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PROFILING_COUNTER_B_FIELDS[] =
{
    &GSU_REG_PROFILING_COUNTER_B_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PROFILING_COUNTER_B *****/
const ru_reg_rec GSU_REG_PROFILING_COUNTER_B_REG =
{
    "PROFILING_COUNTER_B",
#if RU_INCLUDE_DESC
    "PROFILING_COUNTER_B Register",
    "Profiling counter B - read only counter, meaning of the value depends on the selected profiling mode\n",
#endif
    { GSU_REG_PROFILING_COUNTER_B_REG_OFFSET },
    0,
    0,
    1512,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_PROFILING_COUNTER_B_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_PROFILING_MODE, TYPE: Type_GSU_REGFILE_CFG_PROFILING_MODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_A_MODE *****/
const ru_field_rec GSU_REG_PROFILING_MODE_COUNTER_A_MODE_FIELD =
{
    "COUNTER_A_MODE",
#if RU_INCLUDE_DESC
    "",
    "Mode for counter A\n",
#endif
    { GSU_REG_PROFILING_MODE_COUNTER_A_MODE_FIELD_MASK },
    0,
    { GSU_REG_PROFILING_MODE_COUNTER_A_MODE_FIELD_WIDTH },
    { GSU_REG_PROFILING_MODE_COUNTER_A_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_B_MODE *****/
const ru_field_rec GSU_REG_PROFILING_MODE_COUNTER_B_MODE_FIELD =
{
    "COUNTER_B_MODE",
#if RU_INCLUDE_DESC
    "",
    "Mode for counter B\n",
#endif
    { GSU_REG_PROFILING_MODE_COUNTER_B_MODE_FIELD_MASK },
    0,
    { GSU_REG_PROFILING_MODE_COUNTER_B_MODE_FIELD_WIDTH },
    { GSU_REG_PROFILING_MODE_COUNTER_B_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COMMAND_ID *****/
const ru_field_rec GSU_REG_PROFILING_MODE_COMMAND_ID_FIELD =
{
    "COMMAND_ID",
#if RU_INCLUDE_DESC
    "",
    "Command to measure\n",
#endif
    { GSU_REG_PROFILING_MODE_COMMAND_ID_FIELD_MASK },
    0,
    { GSU_REG_PROFILING_MODE_COMMAND_ID_FIELD_WIDTH },
    { GSU_REG_PROFILING_MODE_COMMAND_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PROFILING_MODE_FIELDS[] =
{
    &GSU_REG_PROFILING_MODE_COUNTER_A_MODE_FIELD,
    &GSU_REG_PROFILING_MODE_COUNTER_B_MODE_FIELD,
    &GSU_REG_PROFILING_MODE_COMMAND_ID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PROFILING_MODE *****/
const ru_reg_rec GSU_REG_PROFILING_MODE_REG =
{
    "PROFILING_MODE",
#if RU_INCLUDE_DESC
    "PROFILING_MODE Register",
    "Mode 0 (update FIFO):\n- counter A counts total cycles required for executing UpdateFIFO commands\n- counter B counts processed UpdateFIFO commands\n\nMode 1 (Transmit):\n- counter A counts total cycles required for executing TX commands\n- counter B counts total sent packets (including dropped PDs)\n\nMode 2 (RL Update)\n- counter A counts total cycles required for executing RL update commands\n- counter B counts total processed RL nodes\n\nMode 3 (Average number of PDs per TX)\n- counter A counts total number of PDs sent\n- counter B counts total number of TX commands\n\nMode 4 (Idle counter)\n- counter A counts total number of cycles when GSU is idle\n\nOptional Mode 5 (max updateFIFO)\n\nOptional Mode 6 (max TX)\n\nOptional Mode 7 (max RL)\n",
#endif
    { GSU_REG_PROFILING_MODE_REG_OFFSET },
    0,
    0,
    1513,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    GSU_REG_PROFILING_MODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_QM_CFG_1, TYPE: Type_GSU_REGFILE_CFG_QM_CFG_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_0_BASE_QUEUE *****/
const ru_field_rec GSU_REG_QM_CFG_1_QM_0_BASE_QUEUE_FIELD =
{
    "QM_0_BASE_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "Base for QM0, for converting absolute queue ID from QM 0 to local queue ID.\n",
#endif
    { GSU_REG_QM_CFG_1_QM_0_BASE_QUEUE_FIELD_MASK },
    0,
    { GSU_REG_QM_CFG_1_QM_0_BASE_QUEUE_FIELD_WIDTH },
    { GSU_REG_QM_CFG_1_QM_0_BASE_QUEUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_1_BASE_QUEUE *****/
const ru_field_rec GSU_REG_QM_CFG_1_QM_1_BASE_QUEUE_FIELD =
{
    "QM_1_BASE_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "Base for QM1, for converting absolute queue ID from QM 1 to local queue ID.\n",
#endif
    { GSU_REG_QM_CFG_1_QM_1_BASE_QUEUE_FIELD_MASK },
    0,
    { GSU_REG_QM_CFG_1_QM_1_BASE_QUEUE_FIELD_WIDTH },
    { GSU_REG_QM_CFG_1_QM_1_BASE_QUEUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_QM_CFG_1_FIELDS[] =
{
    &GSU_REG_QM_CFG_1_QM_0_BASE_QUEUE_FIELD,
    &GSU_REG_QM_CFG_1_QM_1_BASE_QUEUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_QM_CFG_1 *****/
const ru_reg_rec GSU_REG_QM_CFG_1_REG =
{
    "QM_CFG_1",
#if RU_INCLUDE_DESC
    "QM_CFG_1 Register",
    "Configure QM0/QM1 bases\n",
#endif
    { GSU_REG_QM_CFG_1_REG_OFFSET },
    0,
    0,
    1514,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    GSU_REG_QM_CFG_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_QM_CFG_2, TYPE: Type_GSU_REGFILE_CFG_QM_CFG_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PD_FIFO_QM0 *****/
const ru_field_rec GSU_REG_QM_CFG_2_PD_FIFO_QM0_FIELD =
{
    "PD_FIFO_QM0",
#if RU_INCLUDE_DESC
    "",
    "Pointer to PDFIFO location where QM0 related queues are located\n",
#endif
    { GSU_REG_QM_CFG_2_PD_FIFO_QM0_FIELD_MASK },
    0,
    { GSU_REG_QM_CFG_2_PD_FIFO_QM0_FIELD_WIDTH },
    { GSU_REG_QM_CFG_2_PD_FIFO_QM0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PD_FIFO_QM1 *****/
const ru_field_rec GSU_REG_QM_CFG_2_PD_FIFO_QM1_FIELD =
{
    "PD_FIFO_QM1",
#if RU_INCLUDE_DESC
    "",
    "Pointer to PDFIFO location where QM1 related queues are located\n",
#endif
    { GSU_REG_QM_CFG_2_PD_FIFO_QM1_FIELD_MASK },
    0,
    { GSU_REG_QM_CFG_2_PD_FIFO_QM1_FIELD_WIDTH },
    { GSU_REG_QM_CFG_2_PD_FIFO_QM1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_QM_CFG_2_FIELDS[] =
{
    &GSU_REG_QM_CFG_2_PD_FIFO_QM0_FIELD,
    &GSU_REG_QM_CFG_2_PD_FIFO_QM1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_QM_CFG_2 *****/
const ru_reg_rec GSU_REG_QM_CFG_2_REG =
{
    "QM_CFG_2",
#if RU_INCLUDE_DESC
    "QM_CFG_2 Register",
    "Configure QM0/QM1 PD FIFO pointers\n",
#endif
    { GSU_REG_QM_CFG_2_REG_OFFSET },
    0,
    0,
    1515,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    GSU_REG_QM_CFG_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_QM_CFG_3, TYPE: Type_GSU_REGFILE_CFG_QM_CFG_3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_QM_0_QUEUES *****/
const ru_field_rec GSU_REG_QM_CFG_3_NUM_OF_QM_0_QUEUES_FIELD =
{
    "NUM_OF_QM_0_QUEUES",
#if RU_INCLUDE_DESC
    "",
    "Hpw many queues are in QM0 - serves for calculationg local Queue ID.\n",
#endif
    { GSU_REG_QM_CFG_3_NUM_OF_QM_0_QUEUES_FIELD_MASK },
    0,
    { GSU_REG_QM_CFG_3_NUM_OF_QM_0_QUEUES_FIELD_WIDTH },
    { GSU_REG_QM_CFG_3_NUM_OF_QM_0_QUEUES_FIELD_SHIFT },
    2047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_QM_CFG_3_FIELDS[] =
{
    &GSU_REG_QM_CFG_3_NUM_OF_QM_0_QUEUES_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_QM_CFG_3 *****/
const ru_reg_rec GSU_REG_QM_CFG_3_REG =
{
    "QM_CFG_3",
#if RU_INCLUDE_DESC
    "QM_CFG_3 Register",
    "Number of QM0 queues\n",
#endif
    { GSU_REG_QM_CFG_3_REG_OFFSET },
    0,
    0,
    1516,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_QM_CFG_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_TRANSMIT_CMD_CFG, TYPE: Type_GSU_REGFILE_CFG_TRANSMIT_CMD_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PD_RETURN_ADDR *****/
const ru_field_rec GSU_REG_TRANSMIT_CMD_CFG_PD_RETURN_ADDR_FIELD =
{
    "PD_RETURN_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address for returning PDs to Runner memory (byte resolution)\n",
#endif
    { GSU_REG_TRANSMIT_CMD_CFG_PD_RETURN_ADDR_FIELD_MASK },
    0,
    { GSU_REG_TRANSMIT_CMD_CFG_PD_RETURN_ADDR_FIELD_WIDTH },
    { GSU_REG_TRANSMIT_CMD_CFG_PD_RETURN_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS_RETURN_ADDR *****/
const ru_field_rec GSU_REG_TRANSMIT_CMD_CFG_STATUS_RETURN_ADDR_FIELD =
{
    "STATUS_RETURN_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address for returning status to Runner memory (byte resolution)\n",
#endif
    { GSU_REG_TRANSMIT_CMD_CFG_STATUS_RETURN_ADDR_FIELD_MASK },
    0,
    { GSU_REG_TRANSMIT_CMD_CFG_STATUS_RETURN_ADDR_FIELD_WIDTH },
    { GSU_REG_TRANSMIT_CMD_CFG_STATUS_RETURN_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_TRANSMIT_CMD_CFG_FIELDS[] =
{
    &GSU_REG_TRANSMIT_CMD_CFG_PD_RETURN_ADDR_FIELD,
    &GSU_REG_TRANSMIT_CMD_CFG_STATUS_RETURN_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_TRANSMIT_CMD_CFG *****/
const ru_reg_rec GSU_REG_TRANSMIT_CMD_CFG_REG =
{
    "TRANSMIT_CMD_CFG",
#if RU_INCLUDE_DESC
    "TRANSMIT_CMD_CFG Register",
    "Configurations related to transmit command\n",
#endif
    { GSU_REG_TRANSMIT_CMD_CFG_REG_OFFSET },
    0,
    0,
    1517,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    GSU_REG_TRANSMIT_CMD_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_RL_STATUS_CFG, TYPE: Type_GSU_REGFILE_CFG_RL_STATUS_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FORCED_INCR_RL_STATUS_ADDR *****/
const ru_field_rec GSU_REG_RL_STATUS_CFG_FORCED_INCR_RL_STATUS_ADDR_FIELD =
{
    "FORCED_INCR_RL_STATUS_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address in Runner memory for returning status for RL force increment command\n",
#endif
    { GSU_REG_RL_STATUS_CFG_FORCED_INCR_RL_STATUS_ADDR_FIELD_MASK },
    0,
    { GSU_REG_RL_STATUS_CFG_FORCED_INCR_RL_STATUS_ADDR_FIELD_WIDTH },
    { GSU_REG_RL_STATUS_CFG_FORCED_INCR_RL_STATUS_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PERIODIC_RL_STATUS_ADDR *****/
const ru_field_rec GSU_REG_RL_STATUS_CFG_PERIODIC_RL_STATUS_ADDR_FIELD =
{
    "PERIODIC_RL_STATUS_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address in Runner memory for returning status for RL periodic update command\n",
#endif
    { GSU_REG_RL_STATUS_CFG_PERIODIC_RL_STATUS_ADDR_FIELD_MASK },
    0,
    { GSU_REG_RL_STATUS_CFG_PERIODIC_RL_STATUS_ADDR_FIELD_WIDTH },
    { GSU_REG_RL_STATUS_CFG_PERIODIC_RL_STATUS_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_RL_STATUS_CFG_FIELDS[] =
{
    &GSU_REG_RL_STATUS_CFG_FORCED_INCR_RL_STATUS_ADDR_FIELD,
    &GSU_REG_RL_STATUS_CFG_PERIODIC_RL_STATUS_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_RL_STATUS_CFG *****/
const ru_reg_rec GSU_REG_RL_STATUS_CFG_REG =
{
    "RL_STATUS_CFG",
#if RU_INCLUDE_DESC
    "RL_STATUS_CFG Register",
    "Configure base addresses for RL status responses\n",
#endif
    { GSU_REG_RL_STATUS_CFG_REG_OFFSET },
    0,
    0,
    1518,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    GSU_REG_RL_STATUS_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_RL_FETCH_CFG, TYPE: Type_GSU_REGFILE_CFG_RL_FETCH_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FECTH_RL_BASE_ADDR *****/
const ru_field_rec GSU_REG_RL_FETCH_CFG_FECTH_RL_BASE_ADDR_FIELD =
{
    "FECTH_RL_BASE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address base for returningresponse to fetchRLcounters command to Runner memory (byte resolution). Target address will be calculated according to the configured base address and the index passed as an argument in the command.\n",
#endif
    { GSU_REG_RL_FETCH_CFG_FECTH_RL_BASE_ADDR_FIELD_MASK },
    0,
    { GSU_REG_RL_FETCH_CFG_FECTH_RL_BASE_ADDR_FIELD_WIDTH },
    { GSU_REG_RL_FETCH_CFG_FECTH_RL_BASE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_RL_FETCH_CFG_FIELDS[] =
{
    &GSU_REG_RL_FETCH_CFG_FECTH_RL_BASE_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_RL_FETCH_CFG *****/
const ru_reg_rec GSU_REG_RL_FETCH_CFG_REG =
{
    "RL_FETCH_CFG",
#if RU_INCLUDE_DESC
    "RL_FETCH_CFG Register",
    "Configurations related to RL counters fetch command\n",
#endif
    { GSU_REG_RL_FETCH_CFG_REG_OFFSET },
    0,
    0,
    1519,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_RL_FETCH_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_MISC_CFG, TYPE: Type_GSU_REGFILE_CFG_MISC_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DONT_TRANSMIT_LAST_PD *****/
const ru_field_rec GSU_REG_MISC_CFG_DONT_TRANSMIT_LAST_PD_FIELD =
{
    "DONT_TRANSMIT_LAST_PD",
#if RU_INCLUDE_DESC
    "",
    "If transmit starts when a queue is full and all transmitted packets come from this queue, dont transmit last packet. This feature prevents unwanted switching between unequal-weight DWRR inputs. Disabled by default\n",
#endif
    { GSU_REG_MISC_CFG_DONT_TRANSMIT_LAST_PD_FIELD_MASK },
    0,
    { GSU_REG_MISC_CFG_DONT_TRANSMIT_LAST_PD_FIELD_WIDTH },
    { GSU_REG_MISC_CFG_DONT_TRANSMIT_LAST_PD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RL_COUNT_DITHERING_EN *****/
const ru_field_rec GSU_REG_MISC_CFG_RL_COUNT_DITHERING_EN_FIELD =
{
    "RL_COUNT_DITHERING_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable dithering for RL counters\n",
#endif
    { GSU_REG_MISC_CFG_RL_COUNT_DITHERING_EN_FIELD_MASK },
    0,
    { GSU_REG_MISC_CFG_RL_COUNT_DITHERING_EN_FIELD_WIDTH },
    { GSU_REG_MISC_CFG_RL_COUNT_DITHERING_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CFG_PCK_OVERHEAD_BYTES_A *****/
const ru_field_rec GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_A_FIELD =
{
    "CFG_PCK_OVERHEAD_BYTES_A",
#if RU_INCLUDE_DESC
    "",
    "Configure number of overhead bytes for per-packet overhead feature in RL calculations - profile A\n",
#endif
    { GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_A_FIELD_MASK },
    0,
    { GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_A_FIELD_WIDTH },
    { GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_A_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CFG_PCK_OVERHEAD_BYTES_B *****/
const ru_field_rec GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_B_FIELD =
{
    "CFG_PCK_OVERHEAD_BYTES_B",
#if RU_INCLUDE_DESC
    "",
    "Configure number of overhead bytes for per-packet overhead feature in RL calculations - profile B\n",
#endif
    { GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_B_FIELD_MASK },
    0,
    { GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_B_FIELD_WIDTH },
    { GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_B_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USE_EXTENDED_MEMORY *****/
const ru_field_rec GSU_REG_MISC_CFG_USE_EXTENDED_MEMORY_FIELD =
{
    "USE_EXTENDED_MEMORY",
#if RU_INCLUDE_DESC
    "",
    "GSU will use extended memory (its own and the memory of adjacent GSU)\n",
#endif
    { GSU_REG_MISC_CFG_USE_EXTENDED_MEMORY_FIELD_MASK },
    0,
    { GSU_REG_MISC_CFG_USE_EXTENDED_MEMORY_FIELD_WIDTH },
    { GSU_REG_MISC_CFG_USE_EXTENDED_MEMORY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CFG_SEND_ACK_TO_RNR *****/
const ru_field_rec GSU_REG_MISC_CFG_CFG_SEND_ACK_TO_RNR_FIELD =
{
    "CFG_SEND_ACK_TO_RNR",
#if RU_INCLUDE_DESC
    "",
    "If set to 1, send ACK message to Runner instead of BBH TX\n",
#endif
    { GSU_REG_MISC_CFG_CFG_SEND_ACK_TO_RNR_FIELD_MASK },
    0,
    { GSU_REG_MISC_CFG_CFG_SEND_ACK_TO_RNR_FIELD_WIDTH },
    { GSU_REG_MISC_CFG_CFG_SEND_ACK_TO_RNR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK_NUM_FOR_RNR_ACK *****/
const ru_field_rec GSU_REG_MISC_CFG_TASK_NUM_FOR_RNR_ACK_FIELD =
{
    "TASK_NUM_FOR_RNR_ACK",
#if RU_INCLUDE_DESC
    "",
    "Task number to wake up when returning ACK in TX status message to Runner - relevant only when CFG_SEND_ACK_TO_RNR=1\n",
#endif
    { GSU_REG_MISC_CFG_TASK_NUM_FOR_RNR_ACK_FIELD_MASK },
    0,
    { GSU_REG_MISC_CFG_TASK_NUM_FOR_RNR_ACK_FIELD_WIDTH },
    { GSU_REG_MISC_CFG_TASK_NUM_FOR_RNR_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CFG_SEND_NACK_TO_BBHTX *****/
const ru_field_rec GSU_REG_MISC_CFG_CFG_SEND_NACK_TO_BBHTX_FIELD =
{
    "CFG_SEND_NACK_TO_BBHTX",
#if RU_INCLUDE_DESC
    "",
    "If set to 1, send NACK message to BBH TX instead of RUNNER\n",
#endif
    { GSU_REG_MISC_CFG_CFG_SEND_NACK_TO_BBHTX_FIELD_MASK },
    0,
    { GSU_REG_MISC_CFG_CFG_SEND_NACK_TO_BBHTX_FIELD_WIDTH },
    { GSU_REG_MISC_CFG_CFG_SEND_NACK_TO_BBHTX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_MISC_CFG_FIELDS[] =
{
    &GSU_REG_MISC_CFG_DONT_TRANSMIT_LAST_PD_FIELD,
    &GSU_REG_MISC_CFG_RL_COUNT_DITHERING_EN_FIELD,
    &GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_A_FIELD,
    &GSU_REG_MISC_CFG_CFG_PCK_OVERHEAD_BYTES_B_FIELD,
    &GSU_REG_MISC_CFG_USE_EXTENDED_MEMORY_FIELD,
    &GSU_REG_MISC_CFG_CFG_SEND_ACK_TO_RNR_FIELD,
    &GSU_REG_MISC_CFG_TASK_NUM_FOR_RNR_ACK_FIELD,
    &GSU_REG_MISC_CFG_CFG_SEND_NACK_TO_BBHTX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_MISC_CFG *****/
const ru_reg_rec GSU_REG_MISC_CFG_REG =
{
    "MISC_CFG",
#if RU_INCLUDE_DESC
    "MISC_CFG Register",
    "Miscellaneous configurations\n",
#endif
    { GSU_REG_MISC_CFG_REG_OFFSET },
    0,
    0,
    1520,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    GSU_REG_MISC_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_PAUSE_CFG, TYPE: Type_GSU_REGFILE_CFG_PAUSE_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PAUSE *****/
const ru_field_rec GSU_REG_PAUSE_CFG_PAUSE_FIELD =
{
    "PAUSE",
#if RU_INCLUDE_DESC
    "",
    "Write 1 to pause GSU operation\n",
#endif
    { GSU_REG_PAUSE_CFG_PAUSE_FIELD_MASK },
    0,
    { GSU_REG_PAUSE_CFG_PAUSE_FIELD_WIDTH },
    { GSU_REG_PAUSE_CFG_PAUSE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STEP_MODE *****/
const ru_field_rec GSU_REG_PAUSE_CFG_STEP_MODE_FIELD =
{
    "STEP_MODE",
#if RU_INCLUDE_DESC
    "",
    "Switch to step mode execution\n",
#endif
    { GSU_REG_PAUSE_CFG_STEP_MODE_FIELD_MASK },
    0,
    { GSU_REG_PAUSE_CFG_STEP_MODE_FIELD_WIDTH },
    { GSU_REG_PAUSE_CFG_STEP_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DO_SINGLE_COMMAND *****/
const ru_field_rec GSU_REG_PAUSE_CFG_DO_SINGLE_COMMAND_FIELD =
{
    "DO_SINGLE_COMMAND",
#if RU_INCLUDE_DESC
    "",
    "In step mode, do single command\n",
#endif
    { GSU_REG_PAUSE_CFG_DO_SINGLE_COMMAND_FIELD_MASK },
    0,
    { GSU_REG_PAUSE_CFG_DO_SINGLE_COMMAND_FIELD_WIDTH },
    { GSU_REG_PAUSE_CFG_DO_SINGLE_COMMAND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PAUSE_CFG_FIELDS[] =
{
    &GSU_REG_PAUSE_CFG_PAUSE_FIELD,
    &GSU_REG_PAUSE_CFG_STEP_MODE_FIELD,
    &GSU_REG_PAUSE_CFG_DO_SINGLE_COMMAND_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PAUSE_CFG *****/
const ru_reg_rec GSU_REG_PAUSE_CFG_REG =
{
    "PAUSE_CFG",
#if RU_INCLUDE_DESC
    "PAUSE_CFG Register",
    "Configurations to control pause/active mechanism\n",
#endif
    { GSU_REG_PAUSE_CFG_REG_OFFSET },
    0,
    0,
    1521,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    GSU_REG_PAUSE_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_AQM_CMD_CFG, TYPE: Type_GSU_REGFILE_CFG_AQM_CMD_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: AQM_RETURN_ADDR *****/
const ru_field_rec GSU_REG_AQM_CMD_CFG_AQM_RETURN_ADDR_FIELD =
{
    "AQM_RETURN_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address for returning PAQM timestamps to Runner memory (byte resolution)\n",
#endif
    { GSU_REG_AQM_CMD_CFG_AQM_RETURN_ADDR_FIELD_MASK },
    0,
    { GSU_REG_AQM_CMD_CFG_AQM_RETURN_ADDR_FIELD_WIDTH },
    { GSU_REG_AQM_CMD_CFG_AQM_RETURN_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_AQM_CMD_CFG_FIELDS[] =
{
    &GSU_REG_AQM_CMD_CFG_AQM_RETURN_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_AQM_CMD_CFG *****/
const ru_reg_rec GSU_REG_AQM_CMD_CFG_REG =
{
    "AQM_CMD_CFG",
#if RU_INCLUDE_DESC
    "AQM_CMD_CFG Register",
    "Configurations related to AQM command\n",
#endif
    { GSU_REG_AQM_CMD_CFG_REG_OFFSET },
    0,
    0,
    1522,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_AQM_CMD_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_FLUSH_CMD_CFG, TYPE: Type_GSU_REGFILE_CFG_FLUSH_CMD_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS_RETURN_ADDR *****/
const ru_field_rec GSU_REG_FLUSH_CMD_CFG_STATUS_RETURN_ADDR_FIELD =
{
    "STATUS_RETURN_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address for returning status to Runner memory (byte resolution)\n",
#endif
    { GSU_REG_FLUSH_CMD_CFG_STATUS_RETURN_ADDR_FIELD_MASK },
    0,
    { GSU_REG_FLUSH_CMD_CFG_STATUS_RETURN_ADDR_FIELD_WIDTH },
    { GSU_REG_FLUSH_CMD_CFG_STATUS_RETURN_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_FLUSH_CMD_CFG_FIELDS[] =
{
    &GSU_REG_FLUSH_CMD_CFG_STATUS_RETURN_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_FLUSH_CMD_CFG *****/
const ru_reg_rec GSU_REG_FLUSH_CMD_CFG_REG =
{
    "FLUSH_CMD_CFG",
#if RU_INCLUDE_DESC
    "FLUSH_CMD_CFG Register",
    "Configurations related to Flush command\n",
#endif
    { GSU_REG_FLUSH_CMD_CFG_REG_OFFSET },
    0,
    0,
    1523,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_FLUSH_CMD_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_RL_UPDATE_CMD_CFG, TYPE: Type_GSU_REGFILE_CFG_RL_UPDATE_CMD_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: LAST_VALID_NODE *****/
const ru_field_rec GSU_REG_RL_UPDATE_CMD_CFG_LAST_VALID_NODE_FIELD =
{
    "LAST_VALID_NODE",
#if RU_INCLUDE_DESC
    "",
    "Set last valid node for RL update. GSU will traverse node mask vector searching for eligible RL nodes until hitting the last node number - then it will stop and report next node 0 in response to Runner.\n",
#endif
    { GSU_REG_RL_UPDATE_CMD_CFG_LAST_VALID_NODE_FIELD_MASK },
    0,
    { GSU_REG_RL_UPDATE_CMD_CFG_LAST_VALID_NODE_FIELD_WIDTH },
    { GSU_REG_RL_UPDATE_CMD_CFG_LAST_VALID_NODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_RL_UPDATE_CMD_CFG_FIELDS[] =
{
    &GSU_REG_RL_UPDATE_CMD_CFG_LAST_VALID_NODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_RL_UPDATE_CMD_CFG *****/
const ru_reg_rec GSU_REG_RL_UPDATE_CMD_CFG_REG =
{
    "RL_UPDATE_CMD_CFG",
#if RU_INCLUDE_DESC
    "RL_UPDATE_CMD_CFG Register",
    "Configurations related to periodic RL update command\n",
#endif
    { GSU_REG_RL_UPDATE_CMD_CFG_REG_OFFSET },
    0,
    0,
    1524,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_RL_UPDATE_CMD_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_PFC_MODIFY_CMD_CFG, TYPE: Type_GSU_REGFILE_CFG_PFC_MODIFY_CMD_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: LAST_VALID_NODE *****/
const ru_field_rec GSU_REG_PFC_MODIFY_CMD_CFG_LAST_VALID_NODE_FIELD =
{
    "LAST_VALID_NODE",
#if RU_INCLUDE_DESC
    "",
    "Set last valid node for PFC reschedule. GSU will traverse node mask vector searching for eligible PFC nodes until hitting the last node number - then it will stop.\n",
#endif
    { GSU_REG_PFC_MODIFY_CMD_CFG_LAST_VALID_NODE_FIELD_MASK },
    0,
    { GSU_REG_PFC_MODIFY_CMD_CFG_LAST_VALID_NODE_FIELD_WIDTH },
    { GSU_REG_PFC_MODIFY_CMD_CFG_LAST_VALID_NODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PFC_MODIFY_CMD_CFG_FIELDS[] =
{
    &GSU_REG_PFC_MODIFY_CMD_CFG_LAST_VALID_NODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PFC_MODIFY_CMD_CFG *****/
const ru_reg_rec GSU_REG_PFC_MODIFY_CMD_CFG_REG =
{
    "PFC_MODIFY_CMD_CFG",
#if RU_INCLUDE_DESC
    "PFC_MODIFY_CMD_CFG Register",
    "Configurations related to PFC modify command\n",
#endif
    { GSU_REG_PFC_MODIFY_CMD_CFG_REG_OFFSET },
    0,
    0,
    1525,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_PFC_MODIFY_CMD_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_PD_FIFO_CFG, TYPE: Type_GSU_REGFILE_CFG_PD_FIFO_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDS_PER_QUEUE *****/
const ru_field_rec GSU_REG_PD_FIFO_CFG_PDS_PER_QUEUE_FIELD =
{
    "PDS_PER_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "PD FIFO Size\n0 - 4  entries\n1 - 8  entries\n2 - 16 entries\n3 - 32 entries\n4 - 64 entries\n",
#endif
    { GSU_REG_PD_FIFO_CFG_PDS_PER_QUEUE_FIELD_MASK },
    0,
    { GSU_REG_PD_FIFO_CFG_PDS_PER_QUEUE_FIELD_WIDTH },
    { GSU_REG_PD_FIFO_CFG_PDS_PER_QUEUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PD_FIFO_CFG_FIELDS[] =
{
    &GSU_REG_PD_FIFO_CFG_PDS_PER_QUEUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PD_FIFO_CFG *****/
const ru_reg_rec GSU_REG_PD_FIFO_CFG_REG =
{
    "PD_FIFO_CFG",
#if RU_INCLUDE_DESC
    "PD_FIFO_CFG Register",
    "PD FIFO related configs\n",
#endif
    { GSU_REG_PD_FIFO_CFG_REG_OFFSET },
    0,
    0,
    1526,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_PD_FIFO_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_PFC_STATUS_CFG, TYPE: Type_GSU_REGFILE_CFG_PFC_STATUS_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS_RETURN_ADDR *****/
const ru_field_rec GSU_REG_PFC_STATUS_CFG_STATUS_RETURN_ADDR_FIELD =
{
    "STATUS_RETURN_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address for returning status to Runner memory (byte resolution)\n",
#endif
    { GSU_REG_PFC_STATUS_CFG_STATUS_RETURN_ADDR_FIELD_MASK },
    0,
    { GSU_REG_PFC_STATUS_CFG_STATUS_RETURN_ADDR_FIELD_WIDTH },
    { GSU_REG_PFC_STATUS_CFG_STATUS_RETURN_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PFC_STATUS_CFG_FIELDS[] =
{
    &GSU_REG_PFC_STATUS_CFG_STATUS_RETURN_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PFC_STATUS_CFG *****/
const ru_reg_rec GSU_REG_PFC_STATUS_CFG_REG =
{
    "PFC_STATUS_CFG",
#if RU_INCLUDE_DESC
    "PFC_STATUS_CFG Register",
    "Configure PFC status return address\n",
#endif
    { GSU_REG_PFC_STATUS_CFG_REG_OFFSET },
    0,
    0,
    1527,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_PFC_STATUS_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_GSU_STATUS, TYPE: Type_GSU_REGFILE_CFG_GSU_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACTIVE *****/
const ru_field_rec GSU_REG_GSU_STATUS_ACTIVE_FIELD =
{
    "ACTIVE",
#if RU_INCLUDE_DESC
    "",
    "Status bit indicating GSU is in active state\n",
#endif
    { GSU_REG_GSU_STATUS_ACTIVE_FIELD_MASK },
    0,
    { GSU_REG_GSU_STATUS_ACTIVE_FIELD_WIDTH },
    { GSU_REG_GSU_STATUS_ACTIVE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_GSU_STATUS_FIELDS[] =
{
    &GSU_REG_GSU_STATUS_ACTIVE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_GSU_STATUS *****/
const ru_reg_rec GSU_REG_GSU_STATUS_REG =
{
    "GSU_STATUS",
#if RU_INCLUDE_DESC
    "GSU_STATUS Register",
    "Status register\n",
#endif
    { GSU_REG_GSU_STATUS_REG_OFFSET },
    0,
    0,
    1528,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_REG_GSU_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_CLK_GATE_CNTRL, TYPE: Type_GSU_REGFILE_CFG_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec GSU_REG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\nTRIGGERS_HW\n\n",
#endif
    { GSU_REG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { GSU_REG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { GSU_REG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec GSU_REG_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\nTRIGGERS_HW\n\n",
#endif
    { GSU_REG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { GSU_REG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { GSU_REG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\nTRIGGERS_HW\n\n",
#endif
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\nTRIGGERS_HW\n\n",
#endif
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\nTRIGGERS_HW\n\n",
#endif
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_CLK_GATE_CNTRL_FIELDS[] =
{
    &GSU_REG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &GSU_REG_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &GSU_REG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_CLK_GATE_CNTRL *****/
const ru_reg_rec GSU_REG_CLK_GATE_CNTRL_REG =
{
    "CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { GSU_REG_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    1529,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    GSU_REG_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_REG_PD_QUEUE_ID_EN, TYPE: Type_GSU_REGFILE_CFG_PD_QUEUE_ID_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_SEND_QUEUE_ID *****/
const ru_field_rec GSU_REG_PD_QUEUE_ID_EN_EN_SEND_QUEUE_ID_FIELD =
{
    "EN_SEND_QUEUE_ID",
#if RU_INCLUDE_DESC
    "",
    "Enable/disable send PD QUEUE ID debug feature\n",
#endif
    { GSU_REG_PD_QUEUE_ID_EN_EN_SEND_QUEUE_ID_FIELD_MASK },
    0,
    { GSU_REG_PD_QUEUE_ID_EN_EN_SEND_QUEUE_ID_FIELD_WIDTH },
    { GSU_REG_PD_QUEUE_ID_EN_EN_SEND_QUEUE_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QUEUE_ID_RETURN_ADDR *****/
const ru_field_rec GSU_REG_PD_QUEUE_ID_EN_QUEUE_ID_RETURN_ADDR_FIELD =
{
    "QUEUE_ID_RETURN_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address in Runner memory for returning queue IDs\n",
#endif
    { GSU_REG_PD_QUEUE_ID_EN_QUEUE_ID_RETURN_ADDR_FIELD_MASK },
    0,
    { GSU_REG_PD_QUEUE_ID_EN_QUEUE_ID_RETURN_ADDR_FIELD_WIDTH },
    { GSU_REG_PD_QUEUE_ID_EN_QUEUE_ID_RETURN_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_REG_PD_QUEUE_ID_EN_FIELDS[] =
{
    &GSU_REG_PD_QUEUE_ID_EN_EN_SEND_QUEUE_ID_FIELD,
    &GSU_REG_PD_QUEUE_ID_EN_QUEUE_ID_RETURN_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_REG_PD_QUEUE_ID_EN *****/
const ru_reg_rec GSU_REG_PD_QUEUE_ID_EN_REG =
{
    "PD_QUEUE_ID_EN",
#if RU_INCLUDE_DESC
    "PD_QUEUE_ID_EN Register",
    "Configuration for PD_QUEUE_ID debug feature\n",
#endif
    { GSU_REG_PD_QUEUE_ID_EN_REG_OFFSET },
    0,
    0,
    1530,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    GSU_REG_PD_QUEUE_ID_EN_FIELDS,
#endif
};

unsigned long GSU_REG_ADDRS[] =
{
    0xB23F0000,
    0xB23F1000,
    0xB23F2000,
    0xB23F3000,
    0xB25F0000,
    0xB25F1000,
    0xB25F2000,
    0xB25F3000,
};

static const ru_reg_rec *GSU_REG_REGS[] =
{
    &GSU_REG_PROFILING_DOWNCNT_REG,
    &GSU_REG_PROFILING_COUNTER_A_REG,
    &GSU_REG_PROFILING_COUNTER_B_REG,
    &GSU_REG_PROFILING_MODE_REG,
    &GSU_REG_QM_CFG_1_REG,
    &GSU_REG_QM_CFG_2_REG,
    &GSU_REG_QM_CFG_3_REG,
    &GSU_REG_TRANSMIT_CMD_CFG_REG,
    &GSU_REG_RL_STATUS_CFG_REG,
    &GSU_REG_RL_FETCH_CFG_REG,
    &GSU_REG_MISC_CFG_REG,
    &GSU_REG_PAUSE_CFG_REG,
    &GSU_REG_AQM_CMD_CFG_REG,
    &GSU_REG_FLUSH_CMD_CFG_REG,
    &GSU_REG_RL_UPDATE_CMD_CFG_REG,
    &GSU_REG_PFC_MODIFY_CMD_CFG_REG,
    &GSU_REG_PD_FIFO_CFG_REG,
    &GSU_REG_PFC_STATUS_CFG_REG,
    &GSU_REG_GSU_STATUS_REG,
    &GSU_REG_CLK_GATE_CNTRL_REG,
    &GSU_REG_PD_QUEUE_ID_EN_REG,
};

const ru_block_rec GSU_REG_BLOCK =
{
    "GSU_REG",
#if RU_INCLUDE_SIZE
    { 0x00000058 },
#endif
    GSU_REG_ADDRS,
    8,
    21,
    GSU_REG_REGS,
};
