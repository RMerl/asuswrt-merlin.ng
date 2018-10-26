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
 * Field: QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD =
{
    "FPM_PREFETCH_ENABLE",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_ENABLE",
    "FPM Prefetch Enable. Setting this bit to 1 will start filling up the FPM pool prefetch FIFOs."
    "Seeting this bit to 0, will stop FPM prefetches.",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD =
{
    "REORDER_CREDIT_ENABLE",
#if RU_INCLUDE_DESC
    "REORDER_CREDIT_ENABLE",
    "When this bit is set the QM will send credits to the REORDER block."
    "Disabling this bit will stop sending credits to the reorder.",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD =
{
    "DQM_POP_ENABLE",
#if RU_INCLUDE_DESC
    "DQM_POP_ENABLE",
    "When this bit is set the QM will pop PDs from the DQM and place them in the runner SRAM",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD =
{
    "RMT_FIXED_ARB_ENABLE",
#if RU_INCLUDE_DESC
    "RMT_FIXED_ARB_ENABLE",
    "When this bit is set Fixed arbitration will be done in pops from the remote FIFOs (Non delayed highest priority). If this bit is cleared RR arbitration is done",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD =
{
    "DQM_PUSH_FIXED_ARB_ENABLE",
#if RU_INCLUDE_DESC
    "DQM_PUSH_FIXED_ARB_ENABLE",
    "When this bit is set Fixed arbitration will be done in DQM pushes (CPU highest priority, then non-delayed queues and then normal queues. If this bit is cleared RR arbitration is done.",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD =
{
    "FPM_PREFETCH0_SW_RST",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH0_SW_RST",
    "FPM Prefetch FIFO0 SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD =
{
    "FPM_PREFETCH1_SW_RST",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH1_SW_RST",
    "FPM Prefetch FIFO1 SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD =
{
    "FPM_PREFETCH2_SW_RST",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH2_SW_RST",
    "FPM Prefetch FIFO2 SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD =
{
    "FPM_PREFETCH3_SW_RST",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH3_SW_RST",
    "FPM Prefetch FIFO3 SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD =
{
    "NORMAL_RMT_SW_RST",
#if RU_INCLUDE_DESC
    "NORMAL_RMT_SW_RST",
    "Normal Remote FIFO SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD =
{
    "NON_DELAYED_RMT_SW_RST",
#if RU_INCLUDE_DESC
    "NON_DELAYED_RMT_SW_RST",
    "Non-delayed Remote FIFO SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD =
{
    "PRE_CM_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "PRE_CM_FIFO_SW_RST",
    "Pre Copy Machine FIFO SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD =
{
    "CM_RD_PD_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "CM_RD_PD_FIFO_SW_RST",
    "Copy Machine RD PD FIFO SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD =
{
    "CM_WR_PD_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "CM_WR_PD_FIFO_SW_RST",
    "Pre Copy Machine FIFO SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD =
{
    "BB0_OUTPUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "BB0_OUTPUT_FIFO_SW_RST",
    "BB0 OUTPUT FIFO SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD =
{
    "BB1_OUTPUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "BB1_OUTPUT_FIFO_SW_RST",
    "BB1 Output FIFO SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD =
{
    "BB1_INPUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "BB1_INPUT_FIFO_SW_RST",
    "BB1 Input FIFO SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD =
{
    "TM_FIFO_PTR_SW_RST",
#if RU_INCLUDE_DESC
    "TM_FIFO_PTR_SW_RST",
    "TM FIFOs Pointers SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD =
{
    "NON_DELAYED_OUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "NON_DELAYED_OUT_FIFO_SW_RST",
    "Non delayed output FIFO Pointers SW reset.",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_SW_RST_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD =
{
    "DROP_CNT_PKTS_READ_CLEAR_ENABLE",
#if RU_INCLUDE_DESC
    "DROP_CNT_PKTS_READ_CLEAR_ENABLE",
    "Indicates whether the Drop/max_occupancy packets counter is read clear.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD =
{
    "DROP_CNT_BYTES_READ_CLEAR_ENABLE",
#if RU_INCLUDE_DESC
    "DROP_CNT_BYTES_READ_CLEAR_ENABLE",
    "Indicates whether the Drop/max_occupancy bytes counter is read clear.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD =
{
    "DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT",
#if RU_INCLUDE_DESC
    "DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT",
    "This bit defines the functionality of the drop packets counter."
    "0 - Functions as the drop packets counter"
    "1 - Functions as the max packets occupancy holder",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD =
{
    "DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT",
#if RU_INCLUDE_DESC
    "DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT",
    "This bit defines the functionality of the drop bytes counter."
    "0 - Functions as the drop bytes counter"
    "1 - Functions as the max bytes occupancy holder",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD =
{
    "FREE_WITH_CONTEXT_LAST_SEARCH",
#if RU_INCLUDE_DESC
    "FREE_WITH_CONTEXT_LAST_SEARCH",
    "Indicates The value to put in the last_search field of the SBPM free with context message",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD =
{
    "WRED_DISABLE",
#if RU_INCLUDE_DESC
    "WRED_DISABLE",
    "Disables WRED influence on drop condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD =
{
    "DDR_PD_CONGESTION_DISABLE",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_DISABLE",
    "Disables DDR_PD_CONGESTION influence on drop/bp"
    "condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD =
{
    "DDR_BYTE_CONGESTION_DISABLE",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_DISABLE",
    "Disables DDR_BYTE_CONGESTION influence on drop/bp condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD =
{
    "DDR_OCCUPANCY_DISABLE",
#if RU_INCLUDE_DESC
    "DDR_OCCUPANCY_DISABLE",
    "Disables DDR_OCCUPANCY influence on drop/bp condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD =
{
    "DDR_FPM_CONGESTION_DISABLE",
#if RU_INCLUDE_DESC
    "DDR_FPM_CONGESTION_DISABLE",
    "Disables DDR_FPM_CONGESTION influence on drop/bp condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD =
{
    "FPM_UG_DISABLE",
#if RU_INCLUDE_DESC
    "FPM_UG_DISABLE",
    "Disables FPM_UG influence on drop condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD =
{
    "QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE",
#if RU_INCLUDE_DESC
    "QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE",
    "Disables QUEUE_OCCUPANCY_DDR_COPY_DECISION influence on copy condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD =
{
    "PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE",
#if RU_INCLUDE_DESC
    "PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE",
    "Disables PSRAM_OCCUPANCY_DDR_COPY_DECISION influence on copy condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD =
{
    "DONT_SEND_MC_BIT_TO_BBH",
#if RU_INCLUDE_DESC
    "DONT_SEND_MC_BIT_TO_BBH",
    "When set, the multicast bit of the PD will not be sent to BBH TX",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD =
{
    "CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE",
#if RU_INCLUDE_DESC
    "CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE",
    "When set, aggregations are not closed automatically when queue open aggregation time expired.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD =
{
    "FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE",
#if RU_INCLUDE_DESC
    "FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE",
    "When cleared, given that there is an FPM congestion situation and all prefetch FPM buffers are full then a min pool size buffer will be freed each 1us. This is done due to the fact that exclusive indication is received only togeter with buffer allocation reply and if this will not be done then a deadlock could occur."
    "Setting this bit will disable this mechanism.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD =
{
    "FPM_BUFFER_GLOBAL_RES_ENABLE",
#if RU_INCLUDE_DESC
    "FPM_BUFFER_GLOBAL_RES_ENABLE",
    "FPM over subscription mechanism."
    "Each queue will have one out of 8 reserved byte threshold profiles. Each profile defines 8 bit threshold with 512byte resolution."
    "Once the global FPM counter pass configurable threshold the system goes to buffer reservation congestion state. In this state any PD entering a queue which passes the reserved byte threshold will be dropped."
    "",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD =
{
    "QM_PRESERVE_PD_WITH_FPM",
#if RU_INCLUDE_DESC
    "QM_PRESERVE_PD_WITH_FPM",
    "Dont drop pd with fpm allocation.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD =
{
    "QM_RESIDUE_PER_QUEUE",
#if RU_INCLUDE_DESC
    "QM_RESIDUE_PER_QUEUE",
    "0 for 64B/Queue"
    "1 for 128B/Queue",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD =
{
    "GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN",
#if RU_INCLUDE_DESC
    "GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN",
    "Controls the timing of updating the overhead counters with packets which goes through aggregation."
    ""
    "0 - updates when the packets enters QM"
    "1 - updates when aggregation is done.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD =
{
    "FPM_UG_FLOW_CTRL_DISABLE",
#if RU_INCLUDE_DESC
    "FPM_UG_FLOW_CTRL_DISABLE",
    "Disables FPM_UG influence on flow control wake up messages to FW.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD =
{
    "DDR_WRITE_MULTI_SLAVE_EN",
#if RU_INCLUDE_DESC
    "DDR_WRITE_MULTI_SLAVE_EN",
    "Enables to write packet transaction to multiple slave (unlimited), if disable only one ubus slave allowed.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD =
{
    "DDR_PD_CONGESTION_AGG_PRIORITY",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_AGG_PRIORITY",
    "global priority bit to aggregated PDs which go through reprocessing."
    "",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD =
{
    "PSRAM_OCCUPANCY_DROP_DISABLE",
#if RU_INCLUDE_DESC
    "PSRAM_OCCUPANCY_DROP_DISABLE",
    "Disables PSRAM_OCCUPANCY_DROP influence on drop condition",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD =
{
    "QM_DDR_WRITE_ALIGNMENT",
#if RU_INCLUDE_DESC
    "QM_DDR_WRITE_ALIGNMENT",
    "0 According to length"
    "1 8-byte aligned",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD =
{
    "EXCLUSIVE_DONT_DROP",
#if RU_INCLUDE_DESC
    "EXCLUSIVE_DONT_DROP",
    "Controls if the exclusive indication in PD marks the PD as dont drop or as dont drop if the fpm in exclusive state"
    "1 - global dont drop"
    "0 - FPM exclusive state dont drop",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_BP_EN
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_BP_EN_FIELD =
{
    "EXCLUSIVE_DONT_DROP_BP_EN",
#if RU_INCLUDE_DESC
    "EXCLUSIVE_DONT_DROP_BP_EN",
    "when set 1 backpressure will be applied when the DONT_DROP pd should be dropped."
    "for example, 0 fpm buffers available and the PD should be copied to DDR.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_BP_EN_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_BP_EN_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_BP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD =
{
    "GPON_DBR_CEIL",
#if RU_INCLUDE_DESC
    "GPON_DBR_CEIL",
    "If the bit enable, QM round up to 4 every packet length added ghost counters.",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD =
{
    "FPM_POOL_BP_ENABLE",
#if RU_INCLUDE_DESC
    "FPM_POOL_BP_ENABLE",
    "This field indicates whether crossing the per pool FPM buffer prefetch FIFO occupancy thresholds will result in dropping packets or in applying back pressure to the re-order."
    "0 - drop packets"
    "1 - apply back pressure",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD =
{
    "FPM_CONGESTION_BP_ENABLE",
#if RU_INCLUDE_DESC
    "FPM_CONGESTION_BP_ENABLE",
    "This field indicates whether crossing the FPM congestion threshold will result in dropping packets or in applying back pressure to the re-order."
    "0 - drop packets"
    "1 - apply back pressure",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD =
{
    "FPM_PREFETCH_MIN_POOL_SIZE",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_MIN_POOL_SIZE",
    "FPM prefetch minimum pool size."
    "The supported FPM pool sizes are derived from this value:"
    "* FPM_PREFETCH_MIN_POOL_SIZEx1"
    "* FPM_PREFETCH_MIN_POOL_SIZEx2"
    "* FPM_PREFETCH_MIN_POOL_SIZEx4"
    "* FPM_PREFETCH_MIN_POOL_SIZEx8"
    ""
    "The optional values for this field:"
    "0 - 256Byte"
    "1 - 512Byte"
    "2 - 1024Byte"
    "3 - 2048Byte"
    "",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_CONTROL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD =
{
    "FPM_PREFETCH_PENDING_REQ_LIMIT",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_PENDING_REQ_LIMIT",
    "The allowed on the fly FPM prefetch pending Alloc requests to the FPM.",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_CONTROL_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED2_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED2_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_CONTROL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD =
{
    "DDR_BYTE_CONGESTION_DROP_ENABLE",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_DROP_ENABLE",
    "This field indicates whether crossing the DDR bytes thresholds (the number of bytes waiting to be copied to DDR) will result in dropping packets or in applying back pressure to the re-order."
    "0 - apply back pressure"
    "1 - drop packets"
    "",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD =
{
    "DDR_BYTES_LOWER_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTES_LOWER_THR",
    "DDR copy bytes Lower Threshold."
    "When working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:"
    "* If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped."
    "* If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are dropped."
    "* If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped."
    "When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and DDR_BYTES_MID_THR are dont care).",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD =
{
    "DDR_BYTES_MID_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTES_MID_THR",
    "DDR copy bytes Lower Threshold."
    "When working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:"
    "* If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped."
    "* If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are dropped."
    "* If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped."
    "When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and DDR_BYTES_MID_THR are dont care).",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD =
{
    "DDR_BYTES_HIGHER_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTES_HIGHER_THR",
    "DDR copy bytes Lower Threshold."
    "When working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:"
    "* If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped."
    "* If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are dropped."
    "* If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped."
    "When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and DDR_BYTES_MID_THR are dont care).",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD =
{
    "DDR_PD_CONGESTION_DROP_ENABLE",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_DROP_ENABLE",
    "This field indicates whether crossing the DDR Pipe thresholds will result in dropping packets or in applying back pressure to the re-order."
    "0 - apply back pressure"
    "1 - drop packets"
    "",
#endif
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD =
{
    "DDR_PIPE_LOWER_THR",
#if RU_INCLUDE_DESC
    "DDR_PIPE_LOWER_THR",
    "DDR copy Pipe Lower Threshold."
    "When working in packet drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:"
    "* If (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (DDR_PIPE_LOWER_THR) < (DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in low priority are dropped."
    "* If (DDR copy pipe occupancy) <=  (DDR_PIPE_LOWER_THR), then no packets are dropped."
    "When working in backpressure mode (DDR_PD_CONGESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_PIPE_LOWER_THR is dont care).",
#endif
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD =
{
    "DDR_PIPE_HIGHER_THR",
#if RU_INCLUDE_DESC
    "DDR_PIPE_HIGHER_THR",
    "DDR copy Pipe Lower Threshold."
    "When working in packet drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:"
    "* If (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (DDR_PIPE_LOWER_THR) < (DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in low priority are dropped."
    "* If (DDR copy pipe occupancy) <= (DDR_PIPE_LOWER_THR), then no packets are dropped."
    "When working in backpressure mode (DDR_PD_CONGESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_PIPE_LOWER_THR is dont care)."
    "IMPORTANT: recommended maximum value is 0x7B in order to avoid performance degradation when working with aggregation timeout enable",
#endif
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD =
{
    "TOTAL_PD_THR",
#if RU_INCLUDE_DESC
    "TOTAL_PD_THR",
    "If the number of PDs for a certain queue exceeds this value, then PDs will be dropped.",
#endif
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD =
{
    "ABS_DROP_QUEUE",
#if RU_INCLUDE_DESC
    "ABS_DROP_QUEUE",
    "Absolute address drop queue."
    "Absolute address PDs which are dropped will be redirected into this configured queue. FW will be responsible for reclaiming their DDR space.",
#endif
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD_WIDTH,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD =
{
    "ABS_DROP_QUEUE_EN",
#if RU_INCLUDE_DESC
    "ABS_DROP_QUEUE_EN",
    "Absolute address drop queue enable."
    "Enables the mechanism in which absolute address PDs which are dropped are be redirected into this configured queue. FW will be responsible for reclaiming their DDR space.",
#endif
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD_WIDTH,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD =
{
    "MAX_AGG_BYTES",
#if RU_INCLUDE_DESC
    "MAX_AGG_BYTES",
    "This field indicates the maximum number of bytes in an aggregated PD.",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD_WIDTH,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD =
{
    "MAX_AGG_PKTS",
#if RU_INCLUDE_DESC
    "MAX_AGG_PKTS",
    "This field indicates the maximum number of packets in an aggregated PD",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD_WIDTH,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD =
{
    "FPM_BASE_ADDR",
#if RU_INCLUDE_DESC
    "FPM_BASE_ADDR",
    "FPM Base Address. This is the 32-bit MSBs out of the 40-bit address."
    "Multiply this field by 256 to get the 40-bit address."
    "Example:"
    "If desired base address is 0x0080_0000"
    "The FPM_BASE_ADDR field should be configured to: 0x0000_8000.",
#endif
    QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD =
{
    "FPM_BASE_ADDR",
#if RU_INCLUDE_DESC
    "FPM_BASE_ADDR",
    "FPM Base Address. This is the 32-bit MSBs out of the 40-bit address."
    "Multiply this field by 256 to get the 40-bit address."
    "Example:"
    "If desired base address is 0x0080_0000"
    "The FPM_BASE_ADDR field should be configured to: 0x0000_8000.",
#endif
    QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD_WIDTH,
    QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD =
{
    "DDR_SOP_OFFSET0",
#if RU_INCLUDE_DESC
    "DDR_SOP_OFFSET0",
    "DDR SOP Offset option 0"
    "",
#endif
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD =
{
    "DDR_SOP_OFFSET1",
#if RU_INCLUDE_DESC
    "DDR_SOP_OFFSET1",
    "DDR SOP Offset option 1"
    "",
#endif
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD =
{
    "EPON_LINE_RATE",
#if RU_INCLUDE_DESC
    "EPON_LINE_RATE",
    "EPON Line Rate"
    "0 - 1G"
    "1 - 10G",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD_WIDTH,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD =
{
    "EPON_CRC_ADD_DISABLE",
#if RU_INCLUDE_DESC
    "EPON_CRC_ADD_DISABLE",
    "If this bit is not set then 4-bytes will be added to the ghost reporting accumulated bytes and to the byte overhead calculation input",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD =
{
    "MAC_FLOW_OVERWRITE_CRC_EN",
#if RU_INCLUDE_DESC
    "MAC_FLOW_OVERWRITE_CRC_EN",
    "Enables to overwrite CRC addition specified MAC FLOW in the field below.",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD_WIDTH,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD =
{
    "MAC_FLOW_OVERWRITE_CRC",
#if RU_INCLUDE_DESC
    "MAC_FLOW_OVERWRITE_CRC",
    "MAC flow ID to force disable CRC addition",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD_WIDTH,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD =
{
    "FEC_IPG_LENGTH",
#if RU_INCLUDE_DESC
    "FEC_IPG_LENGTH",
    "FEC IPG Length",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD_WIDTH,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DQM_FULL_Q_FULL
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD =
{
    "Q_FULL",
#if RU_INCLUDE_DESC
    "Q_FULL",
    "Queue Full indication."
    "This is a 1-bit indication per queue."
    "This register consists of a batch of 32 queues.",
#endif
    QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD_WIDTH,
    QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD =
{
    "Q_NOT_EMPTY",
#if RU_INCLUDE_DESC
    "Q_NOT_EMPTY",
    "Queue Not empty indication."
    "This is a 1-bit indication per queue."
    "This register consists of a batch of 32 queues.",
#endif
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD_WIDTH,
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_DQM_POP_READY_POP_READY
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD =
{
    "POP_READY",
#if RU_INCLUDE_DESC
    "POP_READY",
    "Queue pop ready indication."
    "This is a 1-bit indication per queue."
    "This register consists of a batch of 32 queues.",
#endif
    QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD_WIDTH,
    QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD =
{
    "CONTEXT_VALID",
#if RU_INCLUDE_DESC
    "CONTEXT_VALID",
    "QM ingress aggregation context valid indication."
    "This is a 1-bit indication per queue."
    "This register consists of a batch of 32 queues.",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD_WIDTH,
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD =
{
    "PRESCALER_GRANULARITY",
#if RU_INCLUDE_DESC
    "PRESCALER_GRANULARITY",
    "defines the granularity of the prescaler counter:"
    "0 = 10bits"
    "1 = 11bits"
    "2 = 12bits"
    "3 = 13bits"
    "4 = 14bits"
    "5 = 15bits"
    "6 = 16bits"
    "debug:"
    "7 = 5bits"
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD =
{
    "AGGREGATION_TIMEOUT_VALUE",
#if RU_INCLUDE_DESC
    "AGGREGATION_TIMEOUT_VALUE",
    "Aggregation timeout value, counted in prescaler counters cycles."
    "valid values = [1..7]"
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_0_FIELD =
{
    "RES_THR_0",
#if RU_INCLUDE_DESC
    "RES_THR_0",
    "fpm buffer reservation threshold 0",
#endif
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_1_FIELD =
{
    "RES_THR_1",
#if RU_INCLUDE_DESC
    "RES_THR_1",
    "fpm buffer reservation threshold 1",
#endif
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_1_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_2
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_2_FIELD =
{
    "RES_THR_2",
#if RU_INCLUDE_DESC
    "RES_THR_2",
    "fpm buffer reservation threshold 2",
#endif
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_2_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_2_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_3
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_3_FIELD =
{
    "RES_THR_3",
#if RU_INCLUDE_DESC
    "RES_THR_3",
    "fpm buffer reservation threshold 3",
#endif
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_3_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_3_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RES_THR_GLOBAL
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RES_THR_GLOBAL_FIELD =
{
    "RES_THR_GLOBAL",
#if RU_INCLUDE_DESC
    "RES_THR_GLOBAL",
    "fpm buffer reservation global threshold",
#endif
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RES_THR_GLOBAL_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RES_THR_GLOBAL_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RES_THR_GLOBAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD =
{
    "RNR_BB_ID",
#if RU_INCLUDE_DESC
    "RNR_BB_ID",
    "Runner BB ID",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_TASK
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD =
{
    "RNR_TASK",
#if RU_INCLUDE_DESC
    "RNR_TASK",
    "Runner task",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD =
{
    "RNR_ENABLE",
#if RU_INCLUDE_DESC
    "RNR_ENABLE",
    "Runner enable."
    "if disable, the lossless flow control is disabled."
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_QM_FLOW_CTRL_INTR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_QM_FLOW_CTRL_INTR_FIELD =
{
    "QM_FLOW_CTRL_INTR",
#if RU_INCLUDE_DESC
    "QM_FLOW_CTRL_INTR",
    "QM lossless flow control interrupts"
    "0-3: UG low threshold was crossed"
    "4:   one flow control enabled WRED low thresholds was crossed",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_QM_FLOW_CTRL_INTR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_QM_FLOW_CTRL_INTR_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_QM_FLOW_CTRL_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD =
{
    "FPM_GBL_CNT",
#if RU_INCLUDE_DESC
    "FPM_GBL_CNT",
    "FPM global counter",
#endif
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD =
{
    "QUEUE_NUM",
#if RU_INCLUDE_DESC
    "QUEUE_NUM",
    "Queue num",
#endif
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD =
{
    "FLUSH_EN",
#if RU_INCLUDE_DESC
    "FLUSH_EN",
    "flush queue enable",
#endif
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_POOLS_THR_FPM_LOWER_THR
 ******************************************************************************/
const ru_field_rec QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD =
{
    "FPM_LOWER_THR",
#if RU_INCLUDE_DESC
    "FPM_LOWER_THR",
    "FPM Lower Threshold."
    "When working in packet drop mode (FPM_BP_ENABLE=0), Then:"
    "* If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (FPM_LOWER_THR) < (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dropped."
    "* If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped."
    "When working in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) < (FPM_LOWER_THR), then backpressure is applied to re-order (in this case FPM_HIGHER_THR is dont care).",
#endif
    QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD_MASK,
    0,
    QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD_WIDTH,
    QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_POOLS_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_POOLS_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_POOLS_THR_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_POOLS_THR_RESERVED0_FIELD_WIDTH,
    QM_FPM_POOLS_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_POOLS_THR_FPM_HIGHER_THR
 ******************************************************************************/
const ru_field_rec QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD =
{
    "FPM_HIGHER_THR",
#if RU_INCLUDE_DESC
    "FPM_HIGHER_THR",
    "FPM Higher Threshold."
    "When working in packet drop mode (FPM_BP_ENABLE=0), Then:"
    "* If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (FPM_LOWER_THR) < (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dropped."
    "* If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped."
    "When working in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) < (FPM_LOWER_THR), then backpressure is applied to re-order (in this case FPM_HIGHER_THR is dont care).",
#endif
    QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD_MASK,
    0,
    QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD_WIDTH,
    QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_POOLS_THR_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_FPM_POOLS_THR_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_POOLS_THR_RESERVED1_FIELD_MASK,
    0,
    QM_FPM_POOLS_THR_RESERVED1_FIELD_WIDTH,
    QM_FPM_POOLS_THR_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR
 ******************************************************************************/
const ru_field_rec QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD =
{
    "FPM_GRP_LOWER_THR",
#if RU_INCLUDE_DESC
    "FPM_GRP_LOWER_THR",
    "FPM group Lower Threshold."
    "* If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped."
    "* If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dropped."
    "* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.",
#endif
    QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD_MASK,
    0,
    QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD_WIDTH,
    QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_USR_GRP_LOWER_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_USR_GRP_LOWER_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_USR_GRP_LOWER_THR_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_USR_GRP_LOWER_THR_RESERVED0_FIELD_WIDTH,
    QM_FPM_USR_GRP_LOWER_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR
 ******************************************************************************/
const ru_field_rec QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD =
{
    "FPM_GRP_MID_THR",
#if RU_INCLUDE_DESC
    "FPM_GRP_MID_THR",
    "FPM group Lower Threshold."
    "* If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped."
    "* If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dropped."
    "* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.",
#endif
    QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD_MASK,
    0,
    QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD_WIDTH,
    QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_USR_GRP_MID_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_USR_GRP_MID_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_USR_GRP_MID_THR_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_USR_GRP_MID_THR_RESERVED0_FIELD_WIDTH,
    QM_FPM_USR_GRP_MID_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR
 ******************************************************************************/
const ru_field_rec QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD =
{
    "FPM_GRP_HIGHER_THR",
#if RU_INCLUDE_DESC
    "FPM_GRP_HIGHER_THR",
    "FPM group Lower Threshold."
    "* If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped."
    "* If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped)."
    "* If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dropped."
    "* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.",
#endif
    QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD_MASK,
    0,
    QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD_WIDTH,
    QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_USR_GRP_HIGHER_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_USR_GRP_HIGHER_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_USR_GRP_HIGHER_THR_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_USR_GRP_HIGHER_THR_RESERVED0_FIELD_WIDTH,
    QM_FPM_USR_GRP_HIGHER_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_USR_GRP_CNT_FPM_UG_CNT
 ******************************************************************************/
const ru_field_rec QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD =
{
    "FPM_UG_CNT",
#if RU_INCLUDE_DESC
    "FPM_UG_CNT",
    "FPM user group counter",
#endif
    QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD_MASK,
    0,
    QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD_WIDTH,
    QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_USR_GRP_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_USR_GRP_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_USR_GRP_CNT_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_USR_GRP_CNT_RESERVED0_FIELD_WIDTH,
    QM_FPM_USR_GRP_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD =
{
    "RNR_BB_ID",
#if RU_INCLUDE_DESC
    "RNR_BB_ID",
    "Runner BB ID associated with this configuration.",
#endif
    QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD_MASK,
    0,
    QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD_WIDTH,
    QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_RNR_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED0_FIELD_MASK,
    0,
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED0_FIELD_WIDTH,
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD =
{
    "RNR_TASK",
#if RU_INCLUDE_DESC
    "RNR_TASK",
    "Runner Task number to be woken up when the update FIFO is written to.",
#endif
    QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD_MASK,
    0,
    QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD_WIDTH,
    QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_RNR_CONFIG_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED1_FIELD_MASK,
    0,
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED1_FIELD_WIDTH,
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD =
{
    "RNR_ENABLE",
#if RU_INCLUDE_DESC
    "RNR_ENABLE",
    "Enable this runner interface",
#endif
    QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD_MASK,
    0,
    QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD_WIDTH,
    QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_RNR_CONFIG_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED2_FIELD_MASK,
    0,
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED2_FIELD_WIDTH,
    QM_RUNNER_GRP_RNR_CONFIG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD =
{
    "START_QUEUE",
#if RU_INCLUDE_DESC
    "START_QUEUE",
    "Indicates the Queue that starts this runner group. Queues belonging to the runner group are defined by the following equation:"
    "START_QUEUE <= runner_queues <= END_QUEUE",
#endif
    QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD_MASK,
    0,
    QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD_WIDTH,
    QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED0_FIELD_MASK,
    0,
    QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED0_FIELD_WIDTH,
    QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD =
{
    "END_QUEUE",
#if RU_INCLUDE_DESC
    "END_QUEUE",
    "Indicates the Queue that ends this runner group."
    "Queues belonging to the runner group are defined by the following equation:"
    "START_QUEUE <= runner_queues <= END_QUEUE",
#endif
    QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD_MASK,
    0,
    QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD_WIDTH,
    QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED1_FIELD_MASK,
    0,
    QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED1_FIELD_WIDTH,
    QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED0_FIELD_MASK,
    0,
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED0_FIELD_WIDTH,
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD =
{
    "BASE_ADDR",
#if RU_INCLUDE_DESC
    "BASE_ADDR",
    "PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_ADDR*8).",
#endif
    QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD_MASK,
    0,
    QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD_WIDTH,
    QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED1_FIELD_MASK,
    0,
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED1_FIELD_WIDTH,
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD =
{
    "SIZE",
#if RU_INCLUDE_DESC
    "SIZE",
    "PD FIFO Size"
    "0 - 2 entries"
    "1 - 4 entries"
    "2 - 8 entries",
#endif
    QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD_MASK,
    0,
    QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD_WIDTH,
    QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED2_FIELD_MASK,
    0,
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED2_FIELD_WIDTH,
    QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED0_FIELD_MASK,
    0,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED0_FIELD_WIDTH,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD =
{
    "BASE_ADDR",
#if RU_INCLUDE_DESC
    "BASE_ADDR",
    "PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_ADDR*8).",
#endif
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD_MASK,
    0,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD_WIDTH,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED1_FIELD_MASK,
    0,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED1_FIELD_WIDTH,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD =
{
    "SIZE",
#if RU_INCLUDE_DESC
    "SIZE",
    "PD FIFO Size"
    "0 - 8 entries"
    "1 - 16 entries"
    "2 - 32 entries"
    "3 - 64 entries"
    "4 - 128 entries"
    "5 - 256 entries",
#endif
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD_MASK,
    0,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD_WIDTH,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED2_FIELD_MASK,
    0,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED2_FIELD_WIDTH,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD =
{
    "QM_DQM_POP_ON_EMPTY",
#if RU_INCLUDE_DESC
    "QM_DQM_POP_ON_EMPTY",
    "HW tried to pop a PD from the DQM of an empty queue.",
#endif
    QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD =
{
    "QM_DQM_PUSH_ON_FULL",
#if RU_INCLUDE_DESC
    "QM_DQM_PUSH_ON_FULL",
    "HW tried to pop a PD into the DQM of a full queue.",
#endif
    QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD =
{
    "QM_CPU_POP_ON_EMPTY",
#if RU_INCLUDE_DESC
    "QM_CPU_POP_ON_EMPTY",
    "CPU tried to pop a PD from the DQM of an empty queue.",
#endif
    QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD =
{
    "QM_CPU_PUSH_ON_FULL",
#if RU_INCLUDE_DESC
    "QM_CPU_PUSH_ON_FULL",
    "CPU tried to push a PD into the DQM of a full queue.",
#endif
    QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD =
{
    "QM_NORMAL_QUEUE_PD_NO_CREDIT",
#if RU_INCLUDE_DESC
    "QM_NORMAL_QUEUE_PD_NO_CREDIT",
    "A PD arrived to the Normal queue without having any credits",
#endif
    QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD =
{
    "QM_NON_DELAYED_QUEUE_PD_NO_CREDIT",
#if RU_INCLUDE_DESC
    "QM_NON_DELAYED_QUEUE_PD_NO_CREDIT",
    "A PD arrived to the NON-delayed queue without having any credits",
#endif
    QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD =
{
    "QM_NON_VALID_QUEUE",
#if RU_INCLUDE_DESC
    "QM_NON_VALID_QUEUE",
    "A PD arrived with a non valid queue number (>287)",
#endif
    QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD =
{
    "QM_AGG_COHERENT_INCONSISTENCY",
#if RU_INCLUDE_DESC
    "QM_AGG_COHERENT_INCONSISTENCY",
    "An aggregation of PDs was done in which the coherent bit of the PD differs between them (The coherent bit of the first aggregated PD was used)",
#endif
    QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD =
{
    "QM_FORCE_COPY_ON_NON_DELAYED",
#if RU_INCLUDE_DESC
    "QM_FORCE_COPY_ON_NON_DELAYED",
    "A PD with force copy bit set was received on the non-delayed queue (in this queue the copy machine is bypassed)",
#endif
    QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD =
{
    "QM_FPM_POOL_SIZE_NONEXISTENT",
#if RU_INCLUDE_DESC
    "QM_FPM_POOL_SIZE_NONEXISTENT",
    "A PD was marked to be copied, but there does not exist an FPM pool buffer large enough to hold it.",
#endif
    QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD =
{
    "QM_TARGET_MEM_ABS_CONTRADICTION",
#if RU_INCLUDE_DESC
    "QM_TARGET_MEM_ABS_CONTRADICTION",
    "A PD was marked with a target_mem=1 (located in PSRAM) and on the other hand, the absolute address indication was set.",
#endif
    QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_1588_DROP
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD =
{
    "QM_1588_DROP",
#if RU_INCLUDE_DESC
    "QM_1588_DROP",
    "1588 Packet is dropped when the QM PD occupancy exceeds threshold (64K)",
#endif
    QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD =
{
    "QM_1588_MULTICAST_CONTRADICTION",
#if RU_INCLUDE_DESC
    "QM_1588_MULTICAST_CONTRADICTION",
    "A PD was marked as a 1588 and multicast together.",
#endif
    QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD =
{
    "QM_BYTE_DROP_CNT_OVERRUN",
#if RU_INCLUDE_DESC
    "QM_BYTE_DROP_CNT_OVERRUN",
    "The byte drop counter of one of the queues reached its maximum value and a new value was pushed.",
#endif
    QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD =
{
    "QM_PKT_DROP_CNT_OVERRUN",
#if RU_INCLUDE_DESC
    "QM_PKT_DROP_CNT_OVERRUN",
    "The Packet drop counter of one of the queues reached its maximum value and a new value was pushed.",
#endif
    QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD =
{
    "QM_TOTAL_BYTE_CNT_UNDERRUN",
#if RU_INCLUDE_DESC
    "QM_TOTAL_BYTE_CNT_UNDERRUN",
    "The Total byte counter was decremented to a negative value.",
#endif
    QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD =
{
    "QM_TOTAL_PKT_CNT_UNDERRUN",
#if RU_INCLUDE_DESC
    "QM_TOTAL_PKT_CNT_UNDERRUN",
    "The Total PD counter was decremented to a negative value.",
#endif
    QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD =
{
    "QM_FPM_UG0_UNDERRUN",
#if RU_INCLUDE_DESC
    "QM_FPM_UG0_UNDERRUN",
    "The UG0 counter was decremented to a negative value.",
#endif
    QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD =
{
    "QM_FPM_UG1_UNDERRUN",
#if RU_INCLUDE_DESC
    "QM_FPM_UG1_UNDERRUN",
    "The UG1 counter was decremented to a negative value.",
#endif
    QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD =
{
    "QM_FPM_UG2_UNDERRUN",
#if RU_INCLUDE_DESC
    "QM_FPM_UG2_UNDERRUN",
    "The UG2 counter was decremented to a negative value.",
#endif
    QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD =
{
    "QM_FPM_UG3_UNDERRUN",
#if RU_INCLUDE_DESC
    "QM_FPM_UG3_UNDERRUN",
    "The UG3 counter was decremented to a negative value.",
#endif
    QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD =
{
    "QM_TIMER_WRAPAROUND",
#if RU_INCLUDE_DESC
    "QM_TIMER_WRAPAROUND",
    "QM aggregation timers wraps around. In this case it isnt guaranteed that the aggregation will be closed on pre-defined timeout expiration. However the aggregation should be closed eventually."
    "",
#endif
    QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_INTR_CTRL_ISR_RESERVED0_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_RESERVED0_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISM_ISM
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "Interrupt_status_masked",
    "Status Masked of corresponding interrupt source in the ISR",
#endif
    QM_INTR_CTRL_ISM_ISM_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISM_ISM_FIELD_WIDTH,
    QM_INTR_CTRL_ISM_ISM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ISM_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISM_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_INTR_CTRL_ISM_RESERVED0_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISM_RESERVED0_FIELD_WIDTH,
    QM_INTR_CTRL_ISM_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_IER_IEM
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "Interrupt_enable_mask",
    "Each bit in the mask controls the corresponding interrupt source in the IER",
#endif
    QM_INTR_CTRL_IER_IEM_FIELD_MASK,
    0,
    QM_INTR_CTRL_IER_IEM_FIELD_WIDTH,
    QM_INTR_CTRL_IER_IEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_IER_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_IER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_INTR_CTRL_IER_RESERVED0_FIELD_MASK,
    0,
    QM_INTR_CTRL_IER_RESERVED0_FIELD_WIDTH,
    QM_INTR_CTRL_IER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ITR_IST
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "Interrupt_simulation_test",
    "Each bit in the mask tests the corresponding interrupt source in the ISR",
#endif
    QM_INTR_CTRL_ITR_IST_FIELD_MASK,
    0,
    QM_INTR_CTRL_ITR_IST_FIELD_WIDTH,
    QM_INTR_CTRL_ITR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_write
#endif
};

/******************************************************************************
 * Field: QM_INTR_CTRL_ITR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ITR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_INTR_CTRL_ITR_RESERVED0_FIELD_MASK,
    0,
    QM_INTR_CTRL_ITR_RESERVED0_FIELD_WIDTH,
    QM_INTR_CTRL_ITR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_write
#endif
};

/******************************************************************************
 * Field: QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD =
{
    "QUEUE_NUM",
#if RU_INCLUDE_DESC
    "QUEUE_NUM",
    "Queue Number",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED0_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED0_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "CMD",
    "Command:"
    "00 - Nothing"
    "01 - Write"
    "10 - Read"
    "11 - Read No commit (entry not popped)"
    ""
    "Will trigger a read/write from the selected RAM"
    ""
    "IMPORTANT: Read is for debug purpose only. shouldnt be used during regular QM work on the requested queue (HW pop)."
    "Popping the same queue both from CPU and HW could cause to race condition which will cause to incorrect data output. It could occur when there is only one entry in the queue which is accessed both from the CPU and the HW.",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED1_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED1_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD =
{
    "DONE",
#if RU_INCLUDE_DESC
    "DONE",
    "Indicates that read/write to DQM is done",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD =
{
    "ERROR",
#if RU_INCLUDE_DESC
    "ERROR",
    "Indicates that that an error occured (write on full or read on empty)",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED2_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED2_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA
 ******************************************************************************/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD_MASK,
    0,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD_WIDTH,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD =
{
    "WRED_PROFILE",
#if RU_INCLUDE_DESC
    "WRED_PROFILE",
    "Defines to which WRED Profile this queue belongs to.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD =
{
    "COPY_DEC_PROFILE",
#if RU_INCLUDE_DESC
    "COPY_DEC_PROFILE",
    "Defines to which Copy Decision Profile this queue belongs to.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_COPY_TO_DDR
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_COPY_TO_DDR_FIELD =
{
    "COPY_TO_DDR",
#if RU_INCLUDE_DESC
    "COPY_TO_DDR",
    "Defines this queue to always copy to DDR.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_COPY_TO_DDR_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_COPY_TO_DDR_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_COPY_TO_DDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD =
{
    "DDR_COPY_DISABLE",
#if RU_INCLUDE_DESC
    "DDR_COPY_DISABLE",
    "Defines this queue never to copy to DDR.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD =
{
    "AGGREGATION_DISABLE",
#if RU_INCLUDE_DESC
    "AGGREGATION_DISABLE",
    "Defines this queue never to aggregated PDs.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_FPM_UG
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_FIELD =
{
    "FPM_UG",
#if RU_INCLUDE_DESC
    "FPM_UG",
    "Defines to which FPM UG this queue belongs to.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD =
{
    "EXCLUSIVE_PRIORITY",
#if RU_INCLUDE_DESC
    "EXCLUSIVE_PRIORITY",
    "Defines this queue with exclusive priority.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD =
{
    "Q_802_1AE",
#if RU_INCLUDE_DESC
    "Q_802_1AE",
    "Defines this queue as 802.1AE for EPON packet overhead calculations.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_SCI
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD =
{
    "SCI",
#if RU_INCLUDE_DESC
    "SCI",
    "Configures SCI for EPON packet overhead calculations.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD =
{
    "FEC_ENABLE",
#if RU_INCLUDE_DESC
    "FEC_ENABLE",
    "FEC enable configuration for EPON packet overhead calculations.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD =
{
    "RES_PROFILE",
#if RU_INCLUDE_DESC
    "RES_PROFILE",
    "FPM reservation profile."
    "Once the QM goes over global FPM reservation threshold."
    "Queue with more bytes the defined in the profile will be dropped."
    "Profile 0 means no drop due to FPM reservation for the queues with this profile.",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QUEUE_CONTEXT_CONTEXT_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_RESERVED0_FIELD_MASK,
    0,
    QM_QUEUE_CONTEXT_CONTEXT_RESERVED0_FIELD_WIDTH,
    QM_QUEUE_CONTEXT_CONTEXT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD =
{
    "MIN_THR",
#if RU_INCLUDE_DESC
    "MIN_THR",
    "WRED Color Min Threshold."
    "This field represents the higher 24-bits of the queue occupancy byte threshold."
    "byte_threshold = THR*64.",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD =
{
    "FLW_CTRL_EN",
#if RU_INCLUDE_DESC
    "FLW_CTRL_EN",
    "0 - flow control disable. regular WRED profile"
    "1 - flow control enable. no WRED drop, wake up appropriate runner task when crossed.",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MIN_THR_0_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_0_RESERVED0_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_RESERVED0_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD =
{
    "MIN_THR",
#if RU_INCLUDE_DESC
    "MIN_THR",
    "WRED Color Min Threshold."
    "This field represents the higher 24-bits of the queue occupancy byte threshold."
    "byte_threshold = THR*64.",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD =
{
    "FLW_CTRL_EN",
#if RU_INCLUDE_DESC
    "FLW_CTRL_EN",
    "0 - flow control disable. regular WRED profile"
    "1 - flow control enable. no WRED drop, wake up appropriate runner task when crossed.",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MIN_THR_1_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_1_RESERVED0_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_RESERVED0_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD =
{
    "MAX_THR",
#if RU_INCLUDE_DESC
    "MAX_THR",
    "WRED Color Max Threshold."
    "This field represents the higher 24-bits of the queue occupancy byte threshold."
    "byte_threshold = THR*64.",
#endif
    QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MAX_THR_0_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MAX_THR_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_WRED_PROFILE_COLOR_MAX_THR_0_RESERVED0_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MAX_THR_0_RESERVED0_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MAX_THR_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD =
{
    "MAX_THR",
#if RU_INCLUDE_DESC
    "MAX_THR",
    "WRED Color Max Threshold."
    "This field represents the higher 24-bits of the queue occupancy byte threshold."
    "byte_threshold = THR*64.",
#endif
    QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_MAX_THR_1_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_MAX_THR_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_WRED_PROFILE_COLOR_MAX_THR_1_RESERVED0_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_MAX_THR_1_RESERVED0_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_MAX_THR_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD =
{
    "SLOPE_MANTISSA",
#if RU_INCLUDE_DESC
    "SLOPE_MANTISSA",
    "WRED Color slope mantissa.",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD =
{
    "SLOPE_EXP",
#if RU_INCLUDE_DESC
    "SLOPE_EXP",
    "WRED Color slope exponent.",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_SLOPE_0_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_0_RESERVED0_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_SLOPE_0_RESERVED0_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_SLOPE_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD =
{
    "SLOPE_MANTISSA",
#if RU_INCLUDE_DESC
    "SLOPE_MANTISSA",
    "WRED Color slope mantissa.",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD =
{
    "SLOPE_EXP",
#if RU_INCLUDE_DESC
    "SLOPE_EXP",
    "WRED Color slope exponent.",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_WRED_PROFILE_COLOR_SLOPE_1_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_1_RESERVED0_FIELD_MASK,
    0,
    QM_WRED_PROFILE_COLOR_SLOPE_1_RESERVED0_FIELD_WIDTH,
    QM_WRED_PROFILE_COLOR_SLOPE_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR
 ******************************************************************************/
const ru_field_rec QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD =
{
    "QUEUE_OCCUPANCY_THR",
#if RU_INCLUDE_DESC
    "QUEUE_OCCUPANCY_THR",
    "Queue Occupancy Threshold."
    "When passing this threhold, packets will be copied to the DDR",
#endif
    QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD_MASK,
    0,
    QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD_WIDTH,
    QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_COPY_DECISION_PROFILE_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_COPY_DECISION_PROFILE_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_COPY_DECISION_PROFILE_THR_RESERVED0_FIELD_MASK,
    0,
    QM_COPY_DECISION_PROFILE_THR_RESERVED0_FIELD_WIDTH,
    QM_COPY_DECISION_PROFILE_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_COPY_DECISION_PROFILE_THR_PSRAM_THR
 ******************************************************************************/
const ru_field_rec QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD =
{
    "PSRAM_THR",
#if RU_INCLUDE_DESC
    "PSRAM_THR",
    "Indicates which of the two PSRAM threshold crossing indications coming from the SBPM will be used for the copy decision. when going over the chosen threshold, packets will be copied to the DDR."
    "0 - Lower threshold"
    "1 - Higher threshold",
#endif
    QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD_MASK,
    0,
    QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD_WIDTH,
    QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_TOTAL_VALID_COUNTER_COUNTER_DATA
 ******************************************************************************/
const ru_field_rec QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD_MASK,
    0,
    QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD_WIDTH,
    QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DQM_VALID_COUNTER_COUNTER_DATA
 ******************************************************************************/
const ru_field_rec QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD_MASK,
    0,
    QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD_WIDTH,
    QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DROP_COUNTER_COUNTER_DATA
 ******************************************************************************/
const ru_field_rec QM_DROP_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_DROP_COUNTER_COUNTER_DATA_FIELD_MASK,
    0,
    QM_DROP_COUNTER_COUNTER_DATA_FIELD_WIDTH,
    QM_DROP_COUNTER_COUNTER_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EPON_RPT_CNT_COUNTER_DATA
 ******************************************************************************/
const ru_field_rec QM_EPON_RPT_CNT_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_EPON_RPT_CNT_COUNTER_DATA_FIELD_MASK,
    0,
    QM_EPON_RPT_CNT_COUNTER_DATA_FIELD_WIDTH,
    QM_EPON_RPT_CNT_COUNTER_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR
 ******************************************************************************/
const ru_field_rec QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD =
{
    "STATUS_BIT_VECTOR",
#if RU_INCLUDE_DESC
    "STATUS_BIT_VECTOR",
    "Status bit vector - a bit per queue indicates if the queue has been updated.",
#endif
    QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD_MASK,
    0,
    QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD_WIDTH,
    QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_RD_DATA_POOL0_DATA
 ******************************************************************************/
const ru_field_rec QM_RD_DATA_POOL0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_RD_DATA_POOL0_DATA_FIELD_MASK,
    0,
    QM_RD_DATA_POOL0_DATA_FIELD_WIDTH,
    QM_RD_DATA_POOL0_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_RD_DATA_POOL1_DATA
 ******************************************************************************/
const ru_field_rec QM_RD_DATA_POOL1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_RD_DATA_POOL1_DATA_FIELD_MASK,
    0,
    QM_RD_DATA_POOL1_DATA_FIELD_WIDTH,
    QM_RD_DATA_POOL1_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_RD_DATA_POOL2_DATA
 ******************************************************************************/
const ru_field_rec QM_RD_DATA_POOL2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_RD_DATA_POOL2_DATA_FIELD_MASK,
    0,
    QM_RD_DATA_POOL2_DATA_FIELD_WIDTH,
    QM_RD_DATA_POOL2_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_RD_DATA_POOL3_DATA
 ******************************************************************************/
const ru_field_rec QM_RD_DATA_POOL3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_RD_DATA_POOL3_DATA_FIELD_MASK,
    0,
    QM_RD_DATA_POOL3_DATA_FIELD_WIDTH,
    QM_RD_DATA_POOL3_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PDFIFO_PTR_WR_PTR
 ******************************************************************************/
const ru_field_rec QM_PDFIFO_PTR_WR_PTR_FIELD =
{
    "WR_PTR",
#if RU_INCLUDE_DESC
    "WR_PTR",
    "PDFIFO WR pointers",
#endif
    QM_PDFIFO_PTR_WR_PTR_FIELD_MASK,
    0,
    QM_PDFIFO_PTR_WR_PTR_FIELD_WIDTH,
    QM_PDFIFO_PTR_WR_PTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PDFIFO_PTR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_PDFIFO_PTR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_PDFIFO_PTR_RESERVED0_FIELD_MASK,
    0,
    QM_PDFIFO_PTR_RESERVED0_FIELD_WIDTH,
    QM_PDFIFO_PTR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PDFIFO_PTR_RD_PTR
 ******************************************************************************/
const ru_field_rec QM_PDFIFO_PTR_RD_PTR_FIELD =
{
    "RD_PTR",
#if RU_INCLUDE_DESC
    "RD_PTR",
    "PDFIFO RD pointers",
#endif
    QM_PDFIFO_PTR_RD_PTR_FIELD_MASK,
    0,
    QM_PDFIFO_PTR_RD_PTR_FIELD_WIDTH,
    QM_PDFIFO_PTR_RD_PTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PDFIFO_PTR_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_PDFIFO_PTR_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_PDFIFO_PTR_RESERVED1_FIELD_MASK,
    0,
    QM_PDFIFO_PTR_RESERVED1_FIELD_WIDTH,
    QM_PDFIFO_PTR_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_UPDATE_FIFO_PTR_WR_PTR
 ******************************************************************************/
const ru_field_rec QM_UPDATE_FIFO_PTR_WR_PTR_FIELD =
{
    "WR_PTR",
#if RU_INCLUDE_DESC
    "WR_PTR",
    "UF WR pointers",
#endif
    QM_UPDATE_FIFO_PTR_WR_PTR_FIELD_MASK,
    0,
    QM_UPDATE_FIFO_PTR_WR_PTR_FIELD_WIDTH,
    QM_UPDATE_FIFO_PTR_WR_PTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_UPDATE_FIFO_PTR_RD_PTR
 ******************************************************************************/
const ru_field_rec QM_UPDATE_FIFO_PTR_RD_PTR_FIELD =
{
    "RD_PTR",
#if RU_INCLUDE_DESC
    "RD_PTR",
    "UF RD pointers",
#endif
    QM_UPDATE_FIFO_PTR_RD_PTR_FIELD_MASK,
    0,
    QM_UPDATE_FIFO_PTR_RD_PTR_FIELD_WIDTH,
    QM_UPDATE_FIFO_PTR_RD_PTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_UPDATE_FIFO_PTR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_UPDATE_FIFO_PTR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_UPDATE_FIFO_PTR_RESERVED0_FIELD_MASK,
    0,
    QM_UPDATE_FIFO_PTR_RESERVED0_FIELD_WIDTH,
    QM_UPDATE_FIFO_PTR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_RD_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_RD_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_RD_DATA_DATA_FIELD_MASK,
    0,
    QM_RD_DATA_DATA_FIELD_WIDTH,
    QM_RD_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_POP_POP
 ******************************************************************************/
const ru_field_rec QM_POP_POP_FIELD =
{
    "POP",
#if RU_INCLUDE_DESC
    "POP",
    "Pop FIFO entry",
#endif
    QM_POP_POP_FIELD_MASK,
    0,
    QM_POP_POP_FIELD_WIDTH,
    QM_POP_POP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_POP_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_POP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_POP_RESERVED0_FIELD_MASK,
    0,
    QM_POP_RESERVED0_FIELD_WIDTH,
    QM_POP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_CM_COMMON_INPUT_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NORMAL_RMT_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_RMT_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_DATA_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_RR_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_EGRESS_RR_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_EGRESS_RR_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_EGRESS_RR_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_EGRESS_RR_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_BB_INPUT_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB_OUTPUT_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_OUT_FIFO_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD_MASK,
    0,
    QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD_WIDTH,
    QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CONTEXT_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_CONTEXT_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_CONTEXT_DATA_DATA_FIELD_MASK,
    0,
    QM_CONTEXT_DATA_DATA_FIELD_WIDTH,
    QM_CONTEXT_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DEBUG_SEL_SELECT
 ******************************************************************************/
const ru_field_rec QM_DEBUG_SEL_SELECT_FIELD =
{
    "SELECT",
#if RU_INCLUDE_DESC
    "SELECT",
    "Counter",
#endif
    QM_DEBUG_SEL_SELECT_FIELD_MASK,
    0,
    QM_DEBUG_SEL_SELECT_FIELD_WIDTH,
    QM_DEBUG_SEL_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DEBUG_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DEBUG_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DEBUG_SEL_RESERVED0_FIELD_MASK,
    0,
    QM_DEBUG_SEL_RESERVED0_FIELD_WIDTH,
    QM_DEBUG_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DEBUG_SEL_ENABLE
 ******************************************************************************/
const ru_field_rec QM_DEBUG_SEL_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "ENABLE",
    "Enable register controlled debug select",
#endif
    QM_DEBUG_SEL_ENABLE_FIELD_MASK,
    0,
    QM_DEBUG_SEL_ENABLE_FIELD_WIDTH,
    QM_DEBUG_SEL_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DEBUG_BUS_LSB_DATA
 ******************************************************************************/
const ru_field_rec QM_DEBUG_BUS_LSB_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_DEBUG_BUS_LSB_DATA_FIELD_MASK,
    0,
    QM_DEBUG_BUS_LSB_DATA_FIELD_WIDTH,
    QM_DEBUG_BUS_LSB_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DEBUG_BUS_MSB_DATA
 ******************************************************************************/
const ru_field_rec QM_DEBUG_BUS_MSB_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_DEBUG_BUS_MSB_DATA_FIELD_MASK,
    0,
    QM_DEBUG_BUS_MSB_DATA_FIELD_WIDTH,
    QM_DEBUG_BUS_MSB_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_SPARE_CONFIG_DATA
 ******************************************************************************/
const ru_field_rec QM_QM_SPARE_CONFIG_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    QM_QM_SPARE_CONFIG_DATA_FIELD_MASK,
    0,
    QM_QM_SPARE_CONFIG_DATA_FIELD_WIDTH,
    QM_QM_SPARE_CONFIG_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GOOD_LVL1_PKTS_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD_MASK,
    0,
    QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD_WIDTH,
    QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GOOD_LVL1_BYTES_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD_MASK,
    0,
    QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD_WIDTH,
    QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GOOD_LVL2_PKTS_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD_MASK,
    0,
    QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD_WIDTH,
    QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GOOD_LVL2_BYTES_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD_MASK,
    0,
    QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD_WIDTH,
    QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_COPIED_PKTS_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_COPIED_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_COPIED_PKTS_CNT_COUNTER_FIELD_MASK,
    0,
    QM_COPIED_PKTS_CNT_COUNTER_FIELD_WIDTH,
    QM_COPIED_PKTS_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_COPIED_BYTES_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_COPIED_BYTES_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_COPIED_BYTES_CNT_COUNTER_FIELD_MASK,
    0,
    QM_COPIED_BYTES_CNT_COUNTER_FIELD_WIDTH,
    QM_COPIED_BYTES_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_AGG_PKTS_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_AGG_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_AGG_PKTS_CNT_COUNTER_FIELD_MASK,
    0,
    QM_AGG_PKTS_CNT_COUNTER_FIELD_WIDTH,
    QM_AGG_PKTS_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_AGG_BYTES_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_AGG_BYTES_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_AGG_BYTES_CNT_COUNTER_FIELD_MASK,
    0,
    QM_AGG_BYTES_CNT_COUNTER_FIELD_WIDTH,
    QM_AGG_BYTES_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_AGG_1_PKTS_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_AGG_1_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_AGG_1_PKTS_CNT_COUNTER_FIELD_MASK,
    0,
    QM_AGG_1_PKTS_CNT_COUNTER_FIELD_WIDTH,
    QM_AGG_1_PKTS_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_AGG_2_PKTS_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_AGG_2_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_AGG_2_PKTS_CNT_COUNTER_FIELD_MASK,
    0,
    QM_AGG_2_PKTS_CNT_COUNTER_FIELD_WIDTH,
    QM_AGG_2_PKTS_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_AGG_3_PKTS_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_AGG_3_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_AGG_3_PKTS_CNT_COUNTER_FIELD_MASK,
    0,
    QM_AGG_3_PKTS_CNT_COUNTER_FIELD_WIDTH,
    QM_AGG_3_PKTS_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_AGG_4_PKTS_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_AGG_4_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_AGG_4_PKTS_CNT_COUNTER_FIELD_MASK,
    0,
    QM_AGG_4_PKTS_CNT_COUNTER_FIELD_WIDTH,
    QM_AGG_4_PKTS_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_WRED_DROP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_WRED_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_WRED_DROP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_WRED_DROP_CNT_COUNTER_FIELD_WIDTH,
    QM_WRED_DROP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_CONGESTION_DROP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD_WIDTH,
    QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_WIDTH,
    QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD_WIDTH,
    QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_PD_CONGESTION_DROP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_WIDTH,
    QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_ABS_REQUEUE_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD_MASK,
    0,
    QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD_WIDTH,
    QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO0_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO0_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO0_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_PREFETCH_FIFO0_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO0_STATUS_RESERVED0_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO0_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO1_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO1_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO1_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_PREFETCH_FIFO1_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO1_STATUS_RESERVED0_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO1_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO2_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO2_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO2_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_PREFETCH_FIFO2_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO2_STATUS_RESERVED0_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO2_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO3_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_PREFETCH_FIFO3_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_PREFETCH_FIFO3_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_PREFETCH_FIFO3_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_PREFETCH_FIFO3_STATUS_RESERVED0_FIELD_WIDTH,
    QM_FPM_PREFETCH_FIFO3_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NORMAL_RMT_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NORMAL_RMT_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NORMAL_RMT_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_NORMAL_RMT_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_NORMAL_RMT_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_NORMAL_RMT_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_NORMAL_RMT_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_RMT_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_RMT_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_NON_DELAYED_RMT_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_NON_DELAYED_RMT_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_NON_DELAYED_RMT_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_OUT_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_NON_DELAYED_OUT_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_NON_DELAYED_OUT_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_NON_DELAYED_OUT_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_NON_DELAYED_OUT_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PRE_CM_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PRE_CM_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PRE_CM_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_PRE_CM_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_PRE_CM_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_PRE_CM_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_PRE_CM_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PRE_CM_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_PRE_CM_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_PRE_CM_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_PRE_CM_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_PRE_CM_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_RD_PD_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_RD_PD_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_RD_PD_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_RD_PD_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_CM_RD_PD_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CM_RD_PD_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_CM_RD_PD_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_CM_RD_PD_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_WR_PD_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_WR_PD_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_WR_PD_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_WR_PD_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_CM_WR_PD_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CM_WR_PD_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_CM_WR_PD_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_CM_WR_PD_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_COMMON_INPUT_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_CM_COMMON_INPUT_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_CM_COMMON_INPUT_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_CM_COMMON_INPUT_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_CM_COMMON_INPUT_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB0_OUTPUT_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB0_OUTPUT_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB0_OUTPUT_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BB0_OUTPUT_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BB0_OUTPUT_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_BB0_OUTPUT_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_BB0_OUTPUT_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB1_OUTPUT_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB1_OUTPUT_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB1_OUTPUT_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BB1_OUTPUT_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BB1_OUTPUT_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_BB1_OUTPUT_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_BB1_OUTPUT_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB1_INPUT_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB1_INPUT_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB1_INPUT_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB1_INPUT_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BB1_INPUT_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BB1_INPUT_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_BB1_INPUT_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_BB1_INPUT_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_DATA_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_DATA_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_DATA_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_EGRESS_DATA_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_EGRESS_DATA_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_EGRESS_DATA_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_EGRESS_DATA_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_RR_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_RR_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "Empty",
#endif
    QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_RR_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "Full",
#endif
    QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD_WIDTH,
    QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_EGRESS_RR_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_EGRESS_RR_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_EGRESS_RR_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    QM_EGRESS_RR_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    QM_EGRESS_RR_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BB_ROUTE_OVR_OVR_EN
 ******************************************************************************/
const ru_field_rec QM_BB_ROUTE_OVR_OVR_EN_FIELD =
{
    "OVR_EN",
#if RU_INCLUDE_DESC
    "OVR_EN",
    "BB rout address decode Override enable",
#endif
    QM_BB_ROUTE_OVR_OVR_EN_FIELD_MASK,
    0,
    QM_BB_ROUTE_OVR_OVR_EN_FIELD_WIDTH,
    QM_BB_ROUTE_OVR_OVR_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BB_ROUTE_OVR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BB_ROUTE_OVR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BB_ROUTE_OVR_RESERVED0_FIELD_MASK,
    0,
    QM_BB_ROUTE_OVR_RESERVED0_FIELD_WIDTH,
    QM_BB_ROUTE_OVR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BB_ROUTE_OVR_DEST_ID
 ******************************************************************************/
const ru_field_rec QM_BB_ROUTE_OVR_DEST_ID_FIELD =
{
    "DEST_ID",
#if RU_INCLUDE_DESC
    "DEST_ID",
    "Destination ID",
#endif
    QM_BB_ROUTE_OVR_DEST_ID_FIELD_MASK,
    0,
    QM_BB_ROUTE_OVR_DEST_ID_FIELD_WIDTH,
    QM_BB_ROUTE_OVR_DEST_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BB_ROUTE_OVR_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BB_ROUTE_OVR_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BB_ROUTE_OVR_RESERVED1_FIELD_MASK,
    0,
    QM_BB_ROUTE_OVR_RESERVED1_FIELD_WIDTH,
    QM_BB_ROUTE_OVR_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BB_ROUTE_OVR_ROUTE_ADDR
 ******************************************************************************/
const ru_field_rec QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD =
{
    "ROUTE_ADDR",
#if RU_INCLUDE_DESC
    "ROUTE_ADDR",
    "Route Address",
#endif
    QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD_MASK,
    0,
    QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD_WIDTH,
    QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BB_ROUTE_OVR_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_BB_ROUTE_OVR_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BB_ROUTE_OVR_RESERVED2_FIELD_MASK,
    0,
    QM_BB_ROUTE_OVR_RESERVED2_FIELD_WIDTH,
    QM_BB_ROUTE_OVR_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_QM_INGRESS_STAT_STAT
 ******************************************************************************/
const ru_field_rec QM_QM_INGRESS_STAT_STAT_FIELD =
{
    "STAT",
#if RU_INCLUDE_DESC
    "STAT",
    "Stat",
#endif
    QM_QM_INGRESS_STAT_STAT_FIELD_MASK,
    0,
    QM_QM_INGRESS_STAT_STAT_FIELD_WIDTH,
    QM_QM_INGRESS_STAT_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_EGRESS_STAT_STAT
 ******************************************************************************/
const ru_field_rec QM_QM_EGRESS_STAT_STAT_FIELD =
{
    "STAT",
#if RU_INCLUDE_DESC
    "STAT",
    "Stat",
#endif
    QM_QM_EGRESS_STAT_STAT_FIELD_MASK,
    0,
    QM_QM_EGRESS_STAT_STAT_FIELD_WIDTH,
    QM_QM_EGRESS_STAT_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_CM_STAT_STAT
 ******************************************************************************/
const ru_field_rec QM_QM_CM_STAT_STAT_FIELD =
{
    "STAT",
#if RU_INCLUDE_DESC
    "STAT",
    "Stat",
#endif
    QM_QM_CM_STAT_STAT_FIELD_MASK,
    0,
    QM_QM_CM_STAT_STAT_FIELD_WIDTH,
    QM_QM_CM_STAT_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_FPM_PREFETCH_STAT_STAT
 ******************************************************************************/
const ru_field_rec QM_QM_FPM_PREFETCH_STAT_STAT_FIELD =
{
    "STAT",
#if RU_INCLUDE_DESC
    "STAT",
    "Stat",
#endif
    QM_QM_FPM_PREFETCH_STAT_STAT_FIELD_MASK,
    0,
    QM_QM_FPM_PREFETCH_STAT_STAT_FIELD_WIDTH,
    QM_QM_FPM_PREFETCH_STAT_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER
 ******************************************************************************/
const ru_field_rec QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD =
{
    "CONNECT_ACK_COUNTER",
#if RU_INCLUDE_DESC
    "CONNECT_ACK_COUNTER",
    "Pending SBPM Connect ACKs counter",
#endif
    QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD_MASK,
    0,
    QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD_WIDTH,
    QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_CONNECT_ACK_COUNTER_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_QM_CONNECT_ACK_COUNTER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_QM_CONNECT_ACK_COUNTER_RESERVED0_FIELD_MASK,
    0,
    QM_QM_CONNECT_ACK_COUNTER_RESERVED0_FIELD_WIDTH,
    QM_QM_CONNECT_ACK_COUNTER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER
 ******************************************************************************/
const ru_field_rec QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD =
{
    "DDR_WR_REPLY_COUNTER",
#if RU_INCLUDE_DESC
    "DDR_WR_REPLY_COUNTER",
    "Pending DDR WR Replies counter",
#endif
    QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD_MASK,
    0,
    QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD_WIDTH,
    QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_DDR_WR_REPLY_COUNTER_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_QM_DDR_WR_REPLY_COUNTER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_QM_DDR_WR_REPLY_COUNTER_RESERVED0_FIELD_MASK,
    0,
    QM_QM_DDR_WR_REPLY_COUNTER_RESERVED0_FIELD_WIDTH,
    QM_QM_DDR_WR_REPLY_COUNTER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER
 ******************************************************************************/
const ru_field_rec QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "DDR_PIPE_BYTE_COUNTER",
    "Pending bytes to be copied to the DDR",
#endif
    QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD_MASK,
    0,
    QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD_WIDTH,
    QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_DDR_PIPE_BYTE_COUNTER_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_QM_DDR_PIPE_BYTE_COUNTER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_QM_DDR_PIPE_BYTE_COUNTER_RESERVED0_FIELD_MASK,
    0,
    QM_QM_DDR_PIPE_BYTE_COUNTER_RESERVED0_FIELD_WIDTH,
    QM_QM_DDR_PIPE_BYTE_COUNTER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER
 ******************************************************************************/
const ru_field_rec QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD_MASK,
    0,
    QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD_WIDTH,
    QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_ABS_REQUEUE_VALID_COUNTER_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_QM_ABS_REQUEUE_VALID_COUNTER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_QM_ABS_REQUEUE_VALID_COUNTER_RESERVED0_FIELD_MASK,
    0,
    QM_QM_ABS_REQUEUE_VALID_COUNTER_RESERVED0_FIELD_WIDTH,
    QM_QM_ABS_REQUEUE_VALID_COUNTER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_ILLEGAL_PD_CAPTURE_PD
 ******************************************************************************/
const ru_field_rec QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD =
{
    "PD",
#if RU_INCLUDE_DESC
    "PD",
    "PD",
#endif
    QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD_MASK,
    0,
    QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD_WIDTH,
    QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD
 ******************************************************************************/
const ru_field_rec QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD =
{
    "PD",
#if RU_INCLUDE_DESC
    "PD",
    "PD",
#endif
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD_MASK,
    0,
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD_WIDTH,
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_QM_CM_PROCESSED_PD_CAPTURE_PD
 ******************************************************************************/
const ru_field_rec QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD =
{
    "PD",
#if RU_INCLUDE_DESC
    "PD",
    "PD",
#endif
    QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD_MASK,
    0,
    QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD_WIDTH,
    QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_POOL_DROP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_FPM_POOL_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_FPM_POOL_DROP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_FPM_POOL_DROP_CNT_COUNTER_FIELD_WIDTH,
    QM_FPM_POOL_DROP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_GRP_DROP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_FPM_GRP_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_FPM_GRP_DROP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_FPM_GRP_DROP_CNT_COUNTER_FIELD_WIDTH,
    QM_FPM_GRP_DROP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FPM_BUFFER_RES_DROP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD_WIDTH,
    QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER
 ******************************************************************************/
const ru_field_rec QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD_MASK,
    0,
    QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD_WIDTH,
    QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_DATA_DATA_FIELD_MASK,
    0,
    QM_DATA_DATA_FIELD_WIDTH,
    QM_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_BASE
 ******************************************************************************/
const ru_field_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_BASE_FIELD_MASK,
    0,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_BASE_FIELD_WIDTH,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_MASK
 ******************************************************************************/
const ru_field_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_MASK_FIELD_MASK,
    0,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_MASK_FIELD_WIDTH,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_BASE
 ******************************************************************************/
const ru_field_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_BASE_FIELD_MASK,
    0,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_BASE_FIELD_WIDTH,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_MASK
 ******************************************************************************/
const ru_field_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_MASK_FIELD_MASK,
    0,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_MASK_FIELD_WIDTH,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_BASE
 ******************************************************************************/
const ru_field_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_BASE_FIELD_MASK,
    0,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_BASE_FIELD_WIDTH,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_MASK
 ******************************************************************************/
const ru_field_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_MASK_FIELD_MASK,
    0,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_MASK_FIELD_WIDTH,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_TYPE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD =
{
    "TYPE",
#if RU_INCLUDE_DESC
    "MAC_type",
    "MAC type",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD =
{
    "DMASRC",
#if RU_INCLUDE_DESC
    "DMA_source_id",
    "source id. This id is used to determine the route to the module.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD =
{
    "SDMASRC",
#if RU_INCLUDE_DESC
    "SDMA_source_id",
    "source id. This id is used to determine the route to the module.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD =
{
    "SBPMSRC",
#if RU_INCLUDE_DESC
    "SBPM_source_id",
    "source id. This id is used to determine the route to the module.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD =
{
    "FPMSRC",
#if RU_INCLUDE_DESC
    "FPM_source_id",
    "source id. This id is used to determine the route to the module.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD =
{
    "PDRNR0SRC",
#if RU_INCLUDE_DESC
    "pd_runner0_source_id",
    "source id. This id is used to determine the route to the 1st (out of possible 2 runners) which are responsible for sending PDs.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD =
{
    "PDRNR1SRC",
#if RU_INCLUDE_DESC
    "pd_runner1_source_id",
    "source id. This id is used to determine the route to the 2nd (out of possible 2 runners) which are responsible for sending PDs.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD =
{
    "STSRNRSRC",
#if RU_INCLUDE_DESC
    "Status_Runner_source_id",
    "source id. This id is used to determine the route to the Runner that is responsible for sending status messages (WAN only).",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD =
{
    "MSGRNRSRC",
#if RU_INCLUDE_DESC
    "Message_Runner_source_id",
    "source id. This id is used to determine the route to the Runner which is responsible for sending DBR/Ghost messages (WAN only).",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD =
{
    "BUFSIZE",
#if RU_INCLUDE_DESC
    "DDR_buffer_size",
    "The data is arranged in the DDR in a fixed size buffers.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD =
{
    "BYTERESUL",
#if RU_INCLUDE_DESC
    "PO_bytes_resulotion",
    "The packet offset byte resulotion.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD =
{
    "DDRTXOFFSET",
#if RU_INCLUDE_DESC
    "DDR_tx_offset",
    "Static offset in 8-bytes resolution for non aggregated packets in DDR",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD =
{
    "HNSIZE0",
#if RU_INCLUDE_DESC
    "HN_size_0",
    "The size of the HN (Header number) in bytes. The BBH decides between size 0 and size 1 according to a bit in the PD",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD =
{
    "HNSIZE1",
#if RU_INCLUDE_DESC
    "HN_size_1",
    "The size of the HN (Header number) in bytes. The BBH decides between size 0 and size 1 according to a bit in the PD",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD =
{
    "TCONTADDR",
#if RU_INCLUDE_DESC
    "TCONT_address",
    "Defines the TCONT address within the Runner address space."
    "The address is in 8 bytes resolution."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD =
{
    "SKBADDR",
#if RU_INCLUDE_DESC
    "SKB_address",
    "Defines the SKB free address within the Runner address space."
    "The address is in 8-bytes resolution."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD =
{
    "PTRADDR",
#if RU_INCLUDE_DESC
    "PTRADDR",
    "This field defins the address in the Runner memory space to which the read pointer is written."
    "The address is in 8-bytes resolution.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "Task_number",
    "The number of the task that is responsible for sending PDs to the BBH",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD =
{
    "DESCBASE",
#if RU_INCLUDE_DESC
    "Descriptor_FIFO_base",
    "Defines the base address of the read request FIFO within the DMA address space."
    "The value should be identical to the relevant configuration in the DMA.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD =
{
    "DESCSIZE",
#if RU_INCLUDE_DESC
    "Descriptor_FIFO_size",
    "The size of the BBH read requests FIFO inside the DMA",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD =
{
    "MAXREQ",
#if RU_INCLUDE_DESC
    "Maximum_number_of_requests",
    "Defines the maximum allowed number of on-the-fly read requests.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD =
{
    "EPNURGNT",
#if RU_INCLUDE_DESC
    "Epon_read_urgent",
    "When asserted, this bit forces urgent priority on the EPON read requests towards the DMA (relevant only for EPON BBH)",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD =
{
    "JUMBOURGNT",
#if RU_INCLUDE_DESC
    "Jumbo_read_urgent",
    "When asserted, this bit forces urgent priority on read requests of a jumbo packet (>2K)",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD =
{
    "DESCBASE",
#if RU_INCLUDE_DESC
    "Descriptor_FIFO_base",
    "Defines the base address of the read request FIFO within the DMA address space."
    "The value should be identical to the relevant configuration in the DMA.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD =
{
    "DESCSIZE",
#if RU_INCLUDE_DESC
    "Descriptor_FIFO_size",
    "The size of the BBH read requests FIFO inside the DMA",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD =
{
    "MAXREQ",
#if RU_INCLUDE_DESC
    "Maximum_number_of_requests",
    "Defines the maximum allowed number of on-the-fly read requests.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD =
{
    "EPNURGNT",
#if RU_INCLUDE_DESC
    "Epon_read_urgent",
    "When asserted, this bit forces urgent priority on the EPON read requests towards the DMA (relevant only for EPON BBH)",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD =
{
    "JUMBOURGNT",
#if RU_INCLUDE_DESC
    "Jumbo_read_urgent",
    "When asserted, this bit forces urgent priority on Jumbo packets (>2k) read requests",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD =
{
    "FREENOCNTXT",
#if RU_INCLUDE_DESC
    "Free_without_context_en",
    "When this bit is enabled, the BBH will use free without context command.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD =
{
    "SPECIALFREE",
#if RU_INCLUDE_DESC
    "Special_free_with_context_en",
    "When this bit is enabled, the BBH will use special free with context command."
    "This bit is relevant only if free without context_en is configured to 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD =
{
    "MAXGN",
#if RU_INCLUDE_DESC
    "max_get_next_on_the_fly",
    "maximum number of pending on the fly get next commands",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD =
{
    "DDRTMBASE",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE",
    "DDR TM base."
    "The address is in bytes resolution."
    "The address should be aligned to 128 bytes.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD =
{
    "DDRTMBASE",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE",
    "MSB of DDR TM base."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD =
{
    "PSRAMSIZE",
#if RU_INCLUDE_DESC
    "PSRAM_FIFO_SIZE",
    "The size of the PSRAM data FIFO in 8 bytes resolution. The BBH uses this information for determining the amount of data that can be ordered from the PSRAM.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD =
{
    "DDRSIZE",
#if RU_INCLUDE_DESC
    "DDR_FIFO_SIZE",
    "The size of the DDR data FIFO in 8 bytes resolution. The BBH uses this information for determining the amount of data that can be ordered from the DDR.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD =
{
    "PSRAMBASE",
#if RU_INCLUDE_DESC
    "PSRAM_FIFO_BASE",
    "the base address of the PSRAM data FIFO in 8 bytes resolution. The DDR data FIFO base address is always 0."
    "In case the whole RAM is to be dedicated to PSRAM data, the base should be 0 as well, and the DDR FIFO size should be configured to 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD =
{
    "HIGHTRXQ",
#if RU_INCLUDE_DESC
    "consider_transmitting_q",
    "this configuration determines whether to give high priority to a current transmitting queue or not.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "route",
    "route address",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_DEST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "dest_id",
    "destination source id",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_EN
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "Q_0",
    "Q0 configuration",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "Q_1",
    "Q1 configuration",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD =
{
    "TASK0",
#if RU_INCLUDE_DESC
    "task_0",
    "task number for queue 0",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD =
{
    "TASK1",
#if RU_INCLUDE_DESC
    "task_1",
    "task number for queue 1",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK2
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD =
{
    "TASK2",
#if RU_INCLUDE_DESC
    "task_2",
    "task number for queue 2",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK3
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD =
{
    "TASK3",
#if RU_INCLUDE_DESC
    "task_3",
    "task number for queue 3",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK4
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD =
{
    "TASK4",
#if RU_INCLUDE_DESC
    "task_4",
    "task number for queue 4",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK5
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD =
{
    "TASK5",
#if RU_INCLUDE_DESC
    "task_5",
    "task number for queue 5",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK6
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD =
{
    "TASK6",
#if RU_INCLUDE_DESC
    "task_6",
    "task number for queue 6",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK7
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD =
{
    "TASK7",
#if RU_INCLUDE_DESC
    "task_7",
    "task number for queue 7",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD =
{
    "CNTXTRST",
#if RU_INCLUDE_DESC
    "Context_reset",
    "Writing 1 to this register will reset the segmentation context table."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD =
{
    "PDFIFORST",
#if RU_INCLUDE_DESC
    "PDs_FIFOs_reset",
    "Writing 1 to this register will reset the PDs FIFOs."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD =
{
    "DMAPTRRST",
#if RU_INCLUDE_DESC
    "DMA_write_pointer_reset",
    "Writing 1 to this register will reset the DMA write pointer."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD =
{
    "SDMAPTRRST",
#if RU_INCLUDE_DESC
    "SDMA_write_pointer_reset",
    "Writing 1 to this register will reset the SDMA write pointer."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD =
{
    "BPMFIFORST",
#if RU_INCLUDE_DESC
    "BPM_FIFO_reset",
    "Writing 1 to this register will reset the BPM FIFO."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD =
{
    "SBPMFIFORST",
#if RU_INCLUDE_DESC
    "SBPM_FIFO_reset",
    "Writing 1 to this register will reset the SBPM FIFO."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD =
{
    "OKFIFORST",
#if RU_INCLUDE_DESC
    "Order_Keeper_FIFO_reset",
    "Writing 1 to this register will reset the order keeper FIFO."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD =
{
    "DDRFIFORST",
#if RU_INCLUDE_DESC
    "DDR_FIFO_reset",
    "Writing 1 to this register will reset the DDR data FIFO."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD =
{
    "SRAMFIFORST",
#if RU_INCLUDE_DESC
    "SRAM_FIFO_reset",
    "Writing 1 to this register will reset the SRAM data FIFO."
    "The reset is done immediately. Reading this register will always return 0."
    "This register is relevalt only for Ethernet.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD =
{
    "SKBPTRRST",
#if RU_INCLUDE_DESC
    "SKB_PTR_reset",
    "Writing 1 to this register will reset the SKB pointers."
    "The reset is done immediately. Reading this register will always return 0."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD =
{
    "STSFIFORST",
#if RU_INCLUDE_DESC
    "STS_FIFOs_reset",
    "Writing 1 to this register will reset the EPON status FIFOs (per queue 32 fifos)."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD =
{
    "REQFIFORST",
#if RU_INCLUDE_DESC
    "REQ_FIFO_reset",
    "Writing 1 to this register will reset the EPON request FIFO (8 entries FIFO that holds the packet requests from the EPON MAC)."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD =
{
    "MSGFIFORST",
#if RU_INCLUDE_DESC
    "MSG_FIFO_reset",
    "Writing 1 to this register will reset the EPON/GPON MSG FIFO"
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD =
{
    "GNXTFIFORST",
#if RU_INCLUDE_DESC
    "GET_NXT_FIFO_reset",
    "Writing 1 to this register will reset the GET NEXT FIFOs"
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD =
{
    "FBNFIFORST",
#if RU_INCLUDE_DESC
    "FIRST_BN_FIFO_reset",
    "Writing 1 to this register will reset the FIRST BN FIFOs"
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD =
{
    "DBGSEL",
#if RU_INCLUDE_DESC
    "debug_select",
    "This register selects 1 of 8 debug vectors."
    "The selected vector is reflected to DBGOUTREG.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_GPR
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD =
{
    "GPR",
#if RU_INCLUDE_DESC
    "general",
    "general purpose register",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "FIFO_base_0",
    "The base of PD FIFO for queue 0.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD =
{
    "FIFOBASE1",
#if RU_INCLUDE_DESC
    "FIFO_base_1",
    "The base of PD FIFO for queue 1.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "FIFO_size_0",
    "The size of PD FIFO for queue 0."
    "A value of n refers to n+1."
    "For GPON, the max value is 0x7"
    "For EPON, the max value is 0xf",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD =
{
    "FIFOSIZE1",
#if RU_INCLUDE_DESC
    "FIFO_size_1",
    "The size of PD FIFO for queue 1."
    "A value of n refers to n+1."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD =
{
    "WKUPTHRESH0",
#if RU_INCLUDE_DESC
    "FIFO_wake_up_threshold_0",
    "The wakeup threshold of the PD FIFO for queue 0."
    "A value of n refers to n+1."
    "Relevant only for EPON BBH.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD =
{
    "WKUPTHRESH1",
#if RU_INCLUDE_DESC
    "FIFO_wake_up_threshold_1",
    "The wakeup threshold of the PD FIFO for queue 1."
    "A value of n refers to n+1."
    "Relevant only for EPON BBH.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD =
{
    "PDLIMIT0",
#if RU_INCLUDE_DESC
    "PD_limit_0",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes."
    "The value is in 8-bytes resolution."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD =
{
    "PDLIMIT1",
#if RU_INCLUDE_DESC
    "PD_limit_1",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes."
    "The value is in 8-bytes resolution."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD =
{
    "PDLIMITEN",
#if RU_INCLUDE_DESC
    "PD_limit_enable",
    "This bit enables the above feature (PDs pre fetch limited according to the total number of bytes).",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "Empty_thershold",
    "EPON PD FIFO empty threshold."
    "A queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD =
{
    "DDRTHRESH",
#if RU_INCLUDE_DESC
    "ddr_tx_threshold",
    "DDR Transmit threshold in 8 bytes resoltion",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD =
{
    "SRAMTHRESH",
#if RU_INCLUDE_DESC
    "sram_tx_threshold",
    "SRAM Transmit threshold in 8 bytes resoltion",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_EN
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable bit",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_EN_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_EN_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_EN
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable bit",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_EN_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_EN_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_SRAMPD
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD =
{
    "SRAMPD",
#if RU_INCLUDE_DESC
    "SRAM_PD",
    "This counter counts the number of packets which were transmitted from the SRAM.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_DDRPD
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD =
{
    "DDRPD",
#if RU_INCLUDE_DESC
    "DDR_PD",
    "This counter counts the number of packets which were transmitted from the DDR.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_PDDROP
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD =
{
    "PDDROP",
#if RU_INCLUDE_DESC
    "PD_DROP",
    "This counter counts the number of PDs which were dropped due to PD FIFO full.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_STSCNT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD =
{
    "STSCNT",
#if RU_INCLUDE_DESC
    "STS_CNT",
    "This counter counts the number of received status messages.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_STSDROP
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD =
{
    "STSDROP",
#if RU_INCLUDE_DESC
    "STS_DROP",
    "This counter counts the number of STS which were dropped due to PD FIFO full.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_MSGCNT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD =
{
    "MSGCNT",
#if RU_INCLUDE_DESC
    "MSG_CNT",
    "This counter counts the number of received DBR/ghost messages.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_MSGDROP
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD =
{
    "MSGDROP",
#if RU_INCLUDE_DESC
    "MSG_DROP",
    "This counter counts the number of MSG which were dropped due to PD FIFO full.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD =
{
    "GETNEXTNULL",
#if RU_INCLUDE_DESC
    "Get_next_is_null",
    "This counter counts the number Get next responses with a null BN.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD =
{
    "FLSHPKTS",
#if RU_INCLUDE_DESC
    "FLSH_PKTS",
    "This counter counts the number of flushed packets",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_LENERR
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_LENERR_FIELD =
{
    "LENERR",
#if RU_INCLUDE_DESC
    "LEN_ERR",
    "This counter counts the number of times a length error occuered",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_LENERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD =
{
    "AGGRLENERR",
#if RU_INCLUDE_DESC
    "AGGR_LEN_ERR",
    "This counter counts the number of times an aggregation length error occuered",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD =
{
    "SRAMPKT",
#if RU_INCLUDE_DESC
    "SRAM_PKT",
    "This counter counts the number of packets which were transmitted from the SRAM.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_DDRPKT
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD =
{
    "DDRPKT",
#if RU_INCLUDE_DESC
    "DDR_PKT",
    "This counter counts the number of packets which were transmitted from the DDR.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD =
{
    "SRAMBYTE",
#if RU_INCLUDE_DESC
    "SRAM_BYTE",
    "This counter counts the number of transmitted bytes from the SRAM.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE",
    "This counter counts the number of transmitted bytes from the DDr.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD =
{
    "PDSEL",
#if RU_INCLUDE_DESC
    "pd_array_sel",
    "rd from the PD FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDVSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD =
{
    "PDVSEL",
#if RU_INCLUDE_DESC
    "pd_valid_array_sel",
    "rd from the PD valid array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD =
{
    "PDEMPTYSEL",
#if RU_INCLUDE_DESC
    "pd_empty_array_sel",
    "rd from the PD empty array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD =
{
    "PDFULLSEL",
#if RU_INCLUDE_DESC
    "pd_full_array_sel",
    "rd from the PD Full array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD =
{
    "PDBEMPTYSEL",
#if RU_INCLUDE_DESC
    "pd_below_empty_array_sel",
    "rd from the PD beliow empty array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD =
{
    "PDFFWKPSEL",
#if RU_INCLUDE_DESC
    "pd_full_for_wakeup_array_sel",
    "rd from the PD full for wakeup empty array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD =
{
    "FBNSEL",
#if RU_INCLUDE_DESC
    "first_BN_array_sel",
    "rd from the first BN array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNVSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD =
{
    "FBNVSEL",
#if RU_INCLUDE_DESC
    "first_BN_valid_array_sel",
    "rd from the first BN valid array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD =
{
    "FBNEMPTYSEL",
#if RU_INCLUDE_DESC
    "first_BN_empty_array_sel",
    "rd from the first BN empty array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD =
{
    "FBNFULLSEL",
#if RU_INCLUDE_DESC
    "first_BN_full_array_sel",
    "rd from the first BN full array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD =
{
    "GETNEXTSEL",
#if RU_INCLUDE_DESC
    "get_next_array_sel",
    "rd from the first Get Next array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD =
{
    "GETNEXTVSEL",
#if RU_INCLUDE_DESC
    "get_next_valid_array_sel",
    "rd from the get_next valid array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD =
{
    "GETNEXTEMPTYSEL",
#if RU_INCLUDE_DESC
    "get_next_empty_array_sel",
    "rd from the get next empty array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD =
{
    "GETNEXTFULLSEL",
#if RU_INCLUDE_DESC
    "get_next_full_array_sel",
    "rd from the get next full array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD =
{
    "GPNCNTXTSEL",
#if RU_INCLUDE_DESC
    "gpon_context_array_sel",
    "rd from the gpon context array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD =
{
    "BPMSEL",
#if RU_INCLUDE_DESC
    "BPM_FIFO_sel",
    "rd from the BPM FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMFSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD =
{
    "BPMFSEL",
#if RU_INCLUDE_DESC
    "BPM_FLUSH_FIFO_sel",
    "rd from the BPM FLUSH FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD =
{
    "SBPMSEL",
#if RU_INCLUDE_DESC
    "SBPM_FIFO_sel",
    "rd from the SBPM FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD =
{
    "SBPMFSEL",
#if RU_INCLUDE_DESC
    "SBPM_FLUSH_FIFO_sel",
    "rd from the SBPM FLUSH FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD =
{
    "STSSEL",
#if RU_INCLUDE_DESC
    "sts_array_sel",
    "rd from the STS FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSVSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD =
{
    "STSVSEL",
#if RU_INCLUDE_DESC
    "sts_valid_array_sel",
    "rd from the STS valid array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD =
{
    "STSEMPTYSEL",
#if RU_INCLUDE_DESC
    "sts_empty_array_sel",
    "rd from the STS empty array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD =
{
    "STSFULLSEL",
#if RU_INCLUDE_DESC
    "sts_full_array_sel",
    "rd from the STS Full array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD =
{
    "STSBEMPTYSEL",
#if RU_INCLUDE_DESC
    "sts_below_empty_array_sel",
    "rd from the STS beliow empty array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD =
{
    "STSFFWKPSEL",
#if RU_INCLUDE_DESC
    "sts_full_for_wakeup_array_sel",
    "rd from the STS full for wakeup empty array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD =
{
    "MSGSEL",
#if RU_INCLUDE_DESC
    "msg_array_sel",
    "rd from the MSG FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGVSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD =
{
    "MSGVSEL",
#if RU_INCLUDE_DESC
    "msg_valid_array_sel",
    "rd from the msg valid array",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD =
{
    "EPNREQSEL",
#if RU_INCLUDE_DESC
    "epon_request_FIFO_sel",
    "rd from the epon request FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_DATASEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD =
{
    "DATASEL",
#if RU_INCLUDE_DESC
    "DATA_FIFO_sel",
    "rd from the DATA FIFO (SRAM and DDR)",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REORDERSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD =
{
    "REORDERSEL",
#if RU_INCLUDE_DESC
    "reorder_FIFO_sel",
    "rd from the reorder FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD =
{
    "TSINFOSEL",
#if RU_INCLUDE_DESC
    "Timestamp_info_FIFO_sel",
    "rd from the Timestamp Info FIFO",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MACTXSEL
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD =
{
    "MACTXSEL",
#if RU_INCLUDE_DESC
    "MAC_TX_FIFO_sel",
    "rd from the MAC TX FIFO."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RDADDR
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD =
{
    "RDADDR",
#if RU_INCLUDE_DESC
    "sw_rd_address",
    "The address inside the array the sw wants to read",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_DATA
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "data",
    "data",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE",
    "This counter counts the number of transmitted bytes from the DDr.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE",
    "This counter counts the number of transmitted bytes from the DDr.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD =
{
    "DBGVEC",
#if RU_INCLUDE_DESC
    "Debug_vector",
    "Selected debug vector.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION
 ******************************************************************************/
const ru_field_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD =
{
    "IN_SEGMENTATION",
#if RU_INCLUDE_DESC
    "in_segmentation",
    "in_segmentation indication",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_MASK,
    0,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_WIDTH,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_DEST
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "dest_id",
    "destination ID",
#endif
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_ROUTE
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "route_override",
    "the route to be used (override the default route)",
#endif
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_OVRD
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD =
{
    "OVRD",
#if RU_INCLUDE_DESC
    "OVRD_EN",
    "override enable",
#endif
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD =
{
    "NUMOFBUFF",
#if RU_INCLUDE_DESC
    "number_of_buffers",
    "the number of 128bytes buffers allocated to the peripheral."
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RR_NUM
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD =
{
    "RR_NUM",
#if RU_INCLUDE_DESC
    "NUM_OF_READ_REQ",
    "number of read requests",
#endif
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_U_THRESH_INTO_U
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_U_THRESH_INTO_U_FIELD =
{
    "INTO_U",
#if RU_INCLUDE_DESC
    "into_urgent_threshold",
    "moving into urgent threshold",
#endif
    QM_DMA_QM_DMA_CONFIG_U_THRESH_INTO_U_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_INTO_U_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_INTO_U_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_U_THRESH_OUT_OF_U
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD =
{
    "OUT_OF_U",
#if RU_INCLUDE_DESC
    "out_of_urgent_threshold",
    "moving out ot urgent threshold",
#endif
    QM_DMA_QM_DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED1_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED1_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PRI_RXPRI
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PRI_RXPRI_FIELD =
{
    "RXPRI",
#if RU_INCLUDE_DESC
    "priority_of_rx_side",
    "priority of rx side (upload) of the peripheral",
#endif
    QM_DMA_QM_DMA_CONFIG_PRI_RXPRI_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PRI_RXPRI_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PRI_RXPRI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PRI_TXPRI
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PRI_TXPRI_FIELD =
{
    "TXPRI",
#if RU_INCLUDE_DESC
    "priority_of_tx_side",
    "priority of tx side (download) of the peripheral",
#endif
    QM_DMA_QM_DMA_CONFIG_PRI_TXPRI_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PRI_TXPRI_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PRI_TXPRI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PRI_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PRI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_PRI_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PRI_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PRI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RXSOURCE
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD =
{
    "RXSOURCE",
#if RU_INCLUDE_DESC
    "bb_source_rx_side",
    "bb source of rx side (upload) of the peripheral",
#endif
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_TXSOURCE
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD =
{
    "TXSOURCE",
#if RU_INCLUDE_DESC
    "bb_source_tx_side",
    "bb source of tx side (download) of the peripheral",
#endif
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_WEIGHT_RXWEIGHT
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD =
{
    "RXWEIGHT",
#if RU_INCLUDE_DESC
    "weight_of_rx_side",
    "weight of rx side (upload) of the peripheral",
#endif
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_WEIGHT_TXWEIGHT
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD =
{
    "TXWEIGHT",
#if RU_INCLUDE_DESC
    "weight_of_tx_side",
    "weight of tx side (download) of the peripheral",
#endif
    QM_DMA_QM_DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED1_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED1_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PTRRST_RSTVEC
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PTRRST_RSTVEC_FIELD =
{
    "RSTVEC",
#if RU_INCLUDE_DESC
    "reset_vector",
    "vector in which each bit represents a peripheral."
    "LSB represent RX peripherals and MSB represent TX peripherals."
    "When asserted, the relevant FIFOS of the selected peripheral will be reset to zero",
#endif
    QM_DMA_QM_DMA_CONFIG_PTRRST_RSTVEC_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PTRRST_RSTVEC_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PTRRST_RSTVEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_PTRRST_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_PTRRST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_PTRRST_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_PTRRST_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_PTRRST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_MAX_OTF_MAX
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_MAX_OTF_MAX_FIELD =
{
    "MAX",
#if RU_INCLUDE_DESC
    "max_on_the_fly",
    "max on the fly",
#endif
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_MAX_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_MAX_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_MAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_MAX_OTF_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_MAX_OTF_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_DBG_SEL_DBGSEL
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_DBG_SEL_DBGSEL_FIELD =
{
    "DBGSEL",
#if RU_INCLUDE_DESC
    "select",
    "select",
#endif
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_DBGSEL_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_DBGSEL_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_DBGSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_CONFIG_DBG_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_CONFIG_DBG_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_NEMPTY_NEMPTY
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_NEMPTY_NEMPTY_FIELD =
{
    "NEMPTY",
#if RU_INCLUDE_DESC
    "not_empty_indications",
    "indication of the queue state",
#endif
    QM_DMA_QM_DMA_DEBUG_NEMPTY_NEMPTY_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_NEMPTY_NEMPTY_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_NEMPTY_NEMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_NEMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_NEMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_NEMPTY_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_NEMPTY_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_NEMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_URGNT_URGNT
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_URGNT_URGNT_FIELD =
{
    "URGNT",
#if RU_INCLUDE_DESC
    "urgent",
    "indication whether the queue is in urgent state or not",
#endif
    QM_DMA_QM_DMA_DEBUG_URGNT_URGNT_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_URGNT_URGNT_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_URGNT_URGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_URGNT_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_URGNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_URGNT_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_URGNT_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_URGNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_SELSRC_SEL_SRC
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_SELSRC_SEL_SRC_FIELD =
{
    "SEL_SRC",
#if RU_INCLUDE_DESC
    "selected_source",
    "the next peripheral to be served by the dma",
#endif
    QM_DMA_QM_DMA_DEBUG_SELSRC_SEL_SRC_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_SELSRC_SEL_SRC_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_SELSRC_SEL_SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_SELSRC_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_SELSRC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_SELSRC_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_SELSRC_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_SELSRC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REQ_CNT
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "write_requests_counter",
    "the number of pending write requests",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REQ_CNT
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "read_requests_counter",
    "the number of pending read requests",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "write_requests_counter",
    "the number of pending write requests",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "write_requests_counter",
    "the number of pending write requests",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDADD_ADDRESS
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDADD_ADDRESS_FIELD =
{
    "ADDRESS",
#if RU_INCLUDE_DESC
    "address",
    "address within the ram",
#endif
    QM_DMA_QM_DMA_DEBUG_RDADD_ADDRESS_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDADD_ADDRESS_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDADD_ADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDADD_DATACS
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDADD_DATACS_FIELD =
{
    "DATACS",
#if RU_INCLUDE_DESC
    "data_ram_cs",
    "chip select for write data ram",
#endif
    QM_DMA_QM_DMA_DEBUG_RDADD_DATACS_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDADD_DATACS_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDADD_DATACS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDADD_CDCS
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDADD_CDCS_FIELD =
{
    "CDCS",
#if RU_INCLUDE_DESC
    "cd_ram_cs",
    "chip select for chunk descriptors ram",
#endif
    QM_DMA_QM_DMA_DEBUG_RDADD_CDCS_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDADD_CDCS_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDADD_CDCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDADD_RRCS
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDADD_RRCS_FIELD =
{
    "RRCS",
#if RU_INCLUDE_DESC
    "rr_ram_cd",
    "chip select for read requests ram",
#endif
    QM_DMA_QM_DMA_DEBUG_RDADD_RRCS_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDADD_RRCS_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDADD_RRCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED1_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED1_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDVALID_VALID
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDVALID_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "valid",
    "indirect read request is valid",
#endif
    QM_DMA_QM_DMA_DEBUG_RDVALID_VALID_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDVALID_VALID_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDVALID_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDVALID_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDVALID_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_RDVALID_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDVALID_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDVALID_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDDATA_DATA
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDDATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "data",
    "read data from ram",
#endif
    QM_DMA_QM_DMA_DEBUG_RDDATA_DATA_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDDATA_DATA_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDDATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDDATARDY_READY
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDDATARDY_READY_FIELD =
{
    "READY",
#if RU_INCLUDE_DESC
    "ready",
    "read data ready",
#endif
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_READY_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_READY_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_READY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_DMA_QM_DMA_DEBUG_RDDATARDY_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_DMA_QM_DMA_DEBUG_RDDATARDY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_RESERVED0_FIELD_MASK,
    0,
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_RESERVED0_FIELD_WIDTH,
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_ENABLE_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_ENABLE_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_REG = 
{
    "GLOBAL_CFG_QM_ENABLE_CTRL",
#if RU_INCLUDE_DESC
    "QM_ENABLE_CTRL Register",
    "QM Enable register",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_REG_OFFSET,
    0,
    0,
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_SW_RST_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_SW_RST_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_REG = 
{
    "GLOBAL_CFG_QM_SW_RST_CTRL",
#if RU_INCLUDE_DESC
    "QM_SW_RST_CTRL Register",
    "QM soft reset register",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_REG_OFFSET,
    0,
    0,
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_GENERAL_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_GENERAL_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_BP_EN_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_REG = 
{
    "GLOBAL_CFG_QM_GENERAL_CTRL",
#if RU_INCLUDE_DESC
    "QM_GENERAL_CTRL Register",
    "QM Enable register",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_REG_OFFSET,
    0,
    0,
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    29,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_FPM_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_CONTROL_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_RESERVED1_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_FPM_CONTROL_REG = 
{
    "GLOBAL_CFG_FPM_CONTROL",
#if RU_INCLUDE_DESC
    "FPM_CONTROL Register",
    "FPM Control Register",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_REG_OFFSET,
    0,
    0,
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    QM_GLOBAL_CFG_FPM_CONTROL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_REG = 
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_CONTROL Register",
    "DDR Byte Congestion Control Register",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_REG_OFFSET,
    0,
    0,
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_REG = 
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_LOWER_THR Register",
    "DDR Byte Congestion Lower Threshold",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_REG_OFFSET,
    0,
    0,
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_REG = 
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_MID_THR Register",
    "DDR Byte Congestion Middle Threshold",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_REG_OFFSET,
    0,
    0,
    6,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_REG = 
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_HIGHER_THR Register",
    "DDR Byte Congestion Higher Threshold",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_REG_OFFSET,
    0,
    0,
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD,
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD,
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_REG = 
{
    "GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_CONTROL Register",
    "DDR PD Congestion Control Register",
#endif
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_REG_OFFSET,
    0,
    0,
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD,
    &QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_REG = 
{
    "GLOBAL_CFG_QM_PD_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "QM_PD_CONGESTION_CONTROL Register",
    "QM PD Congestion Control Register",
#endif
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_REG_OFFSET,
    0,
    0,
    9,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_ABS_DROP_QUEUE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_ABS_DROP_QUEUE_FIELDS[] =
{
    &QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD,
    &QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD,
    &QM_GLOBAL_CFG_ABS_DROP_QUEUE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_REG = 
{
    "GLOBAL_CFG_ABS_DROP_QUEUE",
#if RU_INCLUDE_DESC
    "ABS_DROP_QUEUE Register",
    "Absolute Adress drop queue",
#endif
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_REG_OFFSET,
    0,
    0,
    10,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_AGGREGATION_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_AGGREGATION_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_REG = 
{
    "GLOBAL_CFG_AGGREGATION_CTRL",
#if RU_INCLUDE_DESC
    "AGGREGATION_CTRL Register",
    "Aggregation Control register",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CTRL_REG_OFFSET,
    0,
    0,
    11,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_FPM_BASE_ADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_BASE_ADDR_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_FPM_BASE_ADDR_REG = 
{
    "GLOBAL_CFG_FPM_BASE_ADDR",
#if RU_INCLUDE_DESC
    "FPM_BASE_ADDR Register",
    "FPM Base Address",
#endif
    QM_GLOBAL_CFG_FPM_BASE_ADDR_REG_OFFSET,
    0,
    0,
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_FPM_BASE_ADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG = 
{
    "GLOBAL_CFG_FPM_COHERENT_BASE_ADDR",
#if RU_INCLUDE_DESC
    "FPM_COHERENT_BASE_ADDR Register",
    "FPM Base Address for PDs that have the coherent bit set",
#endif
    QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG_OFFSET,
    0,
    0,
    13,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_SOP_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_SOP_OFFSET_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD,
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD,
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG = 
{
    "GLOBAL_CFG_DDR_SOP_OFFSET",
#if RU_INCLUDE_DESC
    "DDR_SOP_OFFSET Register",
    "DDR SOP Offset options",
#endif
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG_OFFSET,
    0,
    0,
    14,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG = 
{
    "GLOBAL_CFG_EPON_OVERHEAD_CTRL",
#if RU_INCLUDE_DESC
    "EPON_OVERHEAD_CTRL Register",
    "EPON Ghost reporting configuration",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG_OFFSET,
    0,
    0,
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DQM_FULL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DQM_FULL_FIELDS[] =
{
    &QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DQM_FULL_REG = 
{
    "GLOBAL_CFG_DQM_FULL",
#if RU_INCLUDE_DESC
    "DQM_FULL %i Register",
    "Queue Full indication"
    "Each register includes a batch of 32 queues non-empty indication."
    "9 Batches are need for 288 queues."
    "First Batch is for queues 31-0 and so on until the last batch representing queues 287-256.",
#endif
    QM_GLOBAL_CFG_DQM_FULL_REG_OFFSET,
    QM_GLOBAL_CFG_DQM_FULL_REG_RAM_CNT,
    4,
    16,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DQM_FULL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DQM_NOT_EMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DQM_NOT_EMPTY_FIELDS[] =
{
    &QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG = 
{
    "GLOBAL_CFG_DQM_NOT_EMPTY",
#if RU_INCLUDE_DESC
    "DQM_NOT_EMPTY %i Register",
    "Queue Not Empty indication"
    "Each register includes a batch of 32 queues non-empty indication."
    "9 Batches are need for 288 queues."
    "First Batch is for queues 31-0 and so on until the last batch representing queues 287-256.",
#endif
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG_OFFSET,
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG_RAM_CNT,
    4,
    17,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DQM_POP_READY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DQM_POP_READY_FIELDS[] =
{
    &QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_DQM_POP_READY_REG = 
{
    "GLOBAL_CFG_DQM_POP_READY",
#if RU_INCLUDE_DESC
    "DQM_POP_READY %i Register",
    "Queue pop ready indication (Some queues may be non-empty, but due to PD offload they are not immediatly ready to be popped. Pop can be issued, but in this case the result could be delayed)."
    "Each register includes a batch of 32 queues non-empty indication."
    "9 Batches are need for 288 queues."
    "First Batch is for queues 31-0 and so on until the last batch representing queues 287-256.",
#endif
    QM_GLOBAL_CFG_DQM_POP_READY_REG_OFFSET,
    QM_GLOBAL_CFG_DQM_POP_READY_REG_RAM_CNT,
    4,
    18,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DQM_POP_READY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_FIELDS[] =
{
    &QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG = 
{
    "GLOBAL_CFG_AGGREGATION_CONTEXT_VALID",
#if RU_INCLUDE_DESC
    "AGGREGATION_CONTEXT_VALID %i Register",
    "Aggregation context valid."
    "This indicates that the queue is in the process of packet aggregation."
    "Each register includes a batch of 32 queues aggregation valid indication."
    "9 Batches are need for 288 queues."
    "First Batch is for queues 31-0 and so on until the last batch representing queues 287-256.",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG_OFFSET,
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG_RAM_CNT,
    4,
    19,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG = 
{
    "GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL",
#if RU_INCLUDE_DESC
    "QM_AGGREGATION_TIMER_CTRL Register",
    "Open aggregation will be forced to close after internal timer expiration."
    "The first byte (0-7bits) controls the granularity of the internal counter (valid value 0x0-0x3)"
    "The second byte (8-15bits) controls the timout value (valid values 0x0-0x7), which is counted according to granularity cycles."
    "the 16bit is enable for the mechanism",
#endif
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG_OFFSET,
    0,
    0,
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_0_FIELD,
    &QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_1_FIELD,
    &QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_2_FIELD,
    &QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_RES_THR_3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_REG = 
{
    "GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES",
#if RU_INCLUDE_DESC
    "QM_FPM_BUFFER_GRP_RES %i Register",
    "FPM Buffer reservation per queue. Each queue will have at least the defined value of FPM buffers available in the group/profile."
    "512B resolution."
    "8bit per profile. starting from profile 0 to 7."
    ""
    ""
    ""
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_REG_OFFSET,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_REG_RAM_CNT,
    4,
    21,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RES_THR_GLOBAL_FIELD,
    &QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_REG = 
{
    "GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR",
#if RU_INCLUDE_DESC
    "QM_FPM_BUFFER_GBL_THR Register",
    "FPM Buffer reservation global threshold."
    "512B resolution."
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_REG_OFFSET,
    0,
    0,
    22,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD,
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD,
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD,
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD,
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_REG = 
{
    "GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG",
#if RU_INCLUDE_DESC
    "QM_FLOW_CTRL_RNR_CFG Register",
    "lossless flow control configuration"
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_REG_OFFSET,
    0,
    0,
    23,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_QM_FLOW_CTRL_INTR_FIELD,
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_REG = 
{
    "GLOBAL_CFG_QM_FLOW_CTRL_INTR",
#if RU_INCLUDE_DESC
    "QM_FLOW_CTRL_INTR Register",
    "0-3: user groups occupancy low thr. was crossed"
    "4:   WRED low thr was crossed"
    ""
    ""
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_REG_OFFSET,
    0,
    0,
    24,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD,
    &QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG = 
{
    "GLOBAL_CFG_QM_FPM_UG_GBL_CNT",
#if RU_INCLUDE_DESC
    "QM_FPM_UG_GBL_CNT Register",
    "FPM global user group counter:"
    "UG0-3 + UG7"
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG_OFFSET,
    0,
    0,
    25,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD,
    &QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD,
    &QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG = 
{
    "GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE",
#if RU_INCLUDE_DESC
    "QM_EGRESS_FLUSH_QUEUE Register",
    "0-8b: queue to flush"
    "9b:   enable flush"
    ""
    ""
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG_OFFSET,
    0,
    0,
    26,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_POOLS_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_POOLS_THR_FIELDS[] =
{
    &QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD,
    &QM_FPM_POOLS_THR_RESERVED0_FIELD,
    &QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD,
    &QM_FPM_POOLS_THR_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_POOLS_THR_REG = 
{
    "FPM_POOLS_THR",
#if RU_INCLUDE_DESC
    "THR Register",
    "Hold 2 thresholds per FPM pool for priority management",
#endif
    QM_FPM_POOLS_THR_REG_OFFSET,
    QM_FPM_POOLS_THR_REG_RAM_CNT,
    32,
    27,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_FPM_POOLS_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_USR_GRP_LOWER_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_USR_GRP_LOWER_THR_FIELDS[] =
{
    &QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD,
    &QM_FPM_USR_GRP_LOWER_THR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_USR_GRP_LOWER_THR_REG = 
{
    "FPM_USR_GRP_LOWER_THR",
#if RU_INCLUDE_DESC
    "LOWER_THR Register",
    "Holds FPM user group lower threshold.",
#endif
    QM_FPM_USR_GRP_LOWER_THR_REG_OFFSET,
    QM_FPM_USR_GRP_LOWER_THR_REG_RAM_CNT,
    32,
    28,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_FPM_USR_GRP_LOWER_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_USR_GRP_MID_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_USR_GRP_MID_THR_FIELDS[] =
{
    &QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD,
    &QM_FPM_USR_GRP_MID_THR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_USR_GRP_MID_THR_REG = 
{
    "FPM_USR_GRP_MID_THR",
#if RU_INCLUDE_DESC
    "MID_THR Register",
    "Holds FPM user group middle threshold.",
#endif
    QM_FPM_USR_GRP_MID_THR_REG_OFFSET,
    QM_FPM_USR_GRP_MID_THR_REG_RAM_CNT,
    32,
    29,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_FPM_USR_GRP_MID_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_USR_GRP_HIGHER_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_USR_GRP_HIGHER_THR_FIELDS[] =
{
    &QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD,
    &QM_FPM_USR_GRP_HIGHER_THR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_USR_GRP_HIGHER_THR_REG = 
{
    "FPM_USR_GRP_HIGHER_THR",
#if RU_INCLUDE_DESC
    "HIGHER_THR Register",
    "Holds FPM user group higher threshold.",
#endif
    QM_FPM_USR_GRP_HIGHER_THR_REG_OFFSET,
    QM_FPM_USR_GRP_HIGHER_THR_REG_RAM_CNT,
    32,
    30,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_FPM_USR_GRP_HIGHER_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_USR_GRP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_USR_GRP_CNT_FIELDS[] =
{
    &QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD,
    &QM_FPM_USR_GRP_CNT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_USR_GRP_CNT_REG = 
{
    "FPM_USR_GRP_CNT",
#if RU_INCLUDE_DESC
    "CNT Register",
    "FPM user group buffer counter",
#endif
    QM_FPM_USR_GRP_CNT_REG_OFFSET,
    QM_FPM_USR_GRP_CNT_REG_RAM_CNT,
    32,
    31,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_FPM_USR_GRP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RUNNER_GRP_RNR_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RUNNER_GRP_RNR_CONFIG_FIELDS[] =
{
    &QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD,
    &QM_RUNNER_GRP_RNR_CONFIG_RESERVED0_FIELD,
    &QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD,
    &QM_RUNNER_GRP_RNR_CONFIG_RESERVED1_FIELD,
    &QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD,
    &QM_RUNNER_GRP_RNR_CONFIG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RUNNER_GRP_RNR_CONFIG_REG = 
{
    "RUNNER_GRP_RNR_CONFIG",
#if RU_INCLUDE_DESC
    "RNR_CONFIG Register",
    "Runners Configurations",
#endif
    QM_RUNNER_GRP_RNR_CONFIG_REG_OFFSET,
    QM_RUNNER_GRP_RNR_CONFIG_REG_RAM_CNT,
    16,
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    QM_RUNNER_GRP_RNR_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RUNNER_GRP_QUEUE_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RUNNER_GRP_QUEUE_CONFIG_FIELDS[] =
{
    &QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD,
    &QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED0_FIELD,
    &QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD,
    &QM_RUNNER_GRP_QUEUE_CONFIG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RUNNER_GRP_QUEUE_CONFIG_REG = 
{
    "RUNNER_GRP_QUEUE_CONFIG",
#if RU_INCLUDE_DESC
    "QUEUE_CONFIG Register",
    "Consecutive queues which are associated with this runner",
#endif
    QM_RUNNER_GRP_QUEUE_CONFIG_REG_OFFSET,
    QM_RUNNER_GRP_QUEUE_CONFIG_REG_RAM_CNT,
    16,
    33,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_RUNNER_GRP_QUEUE_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RUNNER_GRP_PDFIFO_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RUNNER_GRP_PDFIFO_CONFIG_FIELDS[] =
{
    &QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED0_FIELD,
    &QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD,
    &QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED1_FIELD,
    &QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD,
    &QM_RUNNER_GRP_PDFIFO_CONFIG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RUNNER_GRP_PDFIFO_CONFIG_REG = 
{
    "RUNNER_GRP_PDFIFO_CONFIG",
#if RU_INCLUDE_DESC
    "PDFIFO_CONFIG Register",
    "head of the queue PD FIFO attributes",
#endif
    QM_RUNNER_GRP_PDFIFO_CONFIG_REG_OFFSET,
    QM_RUNNER_GRP_PDFIFO_CONFIG_REG_RAM_CNT,
    16,
    34,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    QM_RUNNER_GRP_PDFIFO_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_FIELDS[] =
{
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED0_FIELD,
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD,
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED1_FIELD,
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD,
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG = 
{
    "RUNNER_GRP_UPDATE_FIFO_CONFIG",
#if RU_INCLUDE_DESC
    "UPDATE_FIFO_CONFIG Register",
    "Update FIFO attributes",
#endif
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG_OFFSET,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG_RAM_CNT,
    16,
    35,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_INTR_CTRL_ISR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_INTR_CTRL_ISR_FIELDS[] =
{
    &QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD,
    &QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD,
    &QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD,
    &QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD,
    &QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD,
    &QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD,
    &QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD,
    &QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD,
    &QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD,
    &QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD,
    &QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD,
    &QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD,
    &QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD,
    &QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD,
    &QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD,
    &QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD,
    &QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD,
    &QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD,
    &QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD,
    &QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD,
    &QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD,
    &QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD,
    &QM_INTR_CTRL_ISR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_INTR_CTRL_ISR_REG = 
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active QM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    QM_INTR_CTRL_ISR_REG_OFFSET,
    0,
    0,
    36,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    23,
    QM_INTR_CTRL_ISR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_INTR_CTRL_ISM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_INTR_CTRL_ISM_FIELDS[] =
{
    &QM_INTR_CTRL_ISM_ISM_FIELD,
    &QM_INTR_CTRL_ISM_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_INTR_CTRL_ISM_REG = 
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    QM_INTR_CTRL_ISM_REG_OFFSET,
    0,
    0,
    37,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_INTR_CTRL_ISM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_INTR_CTRL_IER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_INTR_CTRL_IER_FIELDS[] =
{
    &QM_INTR_CTRL_IER_IEM_FIELD,
    &QM_INTR_CTRL_IER_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_INTR_CTRL_IER_REG = 
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    QM_INTR_CTRL_IER_REG_OFFSET,
    0,
    0,
    38,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_INTR_CTRL_IER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_INTR_CTRL_ITR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_INTR_CTRL_ITR_FIELDS[] =
{
    &QM_INTR_CTRL_ITR_IST_FIELD,
    &QM_INTR_CTRL_ITR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_INTR_CTRL_ITR_REG = 
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    QM_INTR_CTRL_ITR_REG_OFFSET,
    0,
    0,
    39,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_INTR_CTRL_ITR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CLK_GATE_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CLK_GATE_CLK_GATE_CNTRL_FIELDS[] =
{
    &QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CLK_GATE_CLK_GATE_CNTRL_REG = 
{
    "CLK_GATE_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    40,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_CLK_GATE_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED0_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED1_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_CTRL Register",
    "CPU PD Indirect Access Control",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG_RAM_CNT,
    64,
    41,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA %i Register",
    "CPU PD Indirect Write data to DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG_RAM_CNT,
    64,
    42,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA %i Register",
    "CPU PD Indirect Write data to DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG_RAM_CNT,
    64,
    43,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA %i Register",
    "CPU PD Indirect Write data to DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG_RAM_CNT,
    64,
    44,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA %i Register",
    "CPU PD Indirect Write data to DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG_RAM_CNT,
    64,
    45,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA %i Register",
    "CPU PD Indirect Read data from DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG_RAM_CNT,
    64,
    46,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA %i Register",
    "CPU PD Indirect Read data from DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG_RAM_CNT,
    64,
    47,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA %i Register",
    "CPU PD Indirect Read data from DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG_RAM_CNT,
    64,
    48,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA %i Register",
    "CPU PD Indirect Read data from DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG_RAM_CNT,
    64,
    49,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QUEUE_CONTEXT_CONTEXT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QUEUE_CONTEXT_CONTEXT_FIELDS[] =
{
    &QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_COPY_TO_DDR_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QUEUE_CONTEXT_CONTEXT_REG = 
{
    "QUEUE_CONTEXT_CONTEXT",
#if RU_INCLUDE_DESC
    "QUEUE_CONTEXT %i Register",
    "This RAM holds all queue attributes."
    "Not all of the 32-bits in the address space are implemented."
    "WRED Profile            3:0"
    "Copy decision profile 6:4"
    "Copy to DDR         7"
    "DDR copy disable 8"
    "Aggregation Disable 9"
    "FPM User Group         11:10"
    "Exclusive Priority 12"
    "802.1AE                 13"
    "SCI                 14"
    "FEC Enable         15"
    "",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_REG_OFFSET,
    QM_QUEUE_CONTEXT_CONTEXT_REG_RAM_CNT,
    4,
    50,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    QM_QUEUE_CONTEXT_CONTEXT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_MIN_THR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_MIN_THR_0_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD,
    &QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD,
    &QM_WRED_PROFILE_COLOR_MIN_THR_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_WRED_PROFILE_COLOR_MIN_THR_0_REG = 
{
    "WRED_PROFILE_COLOR_MIN_THR_0",
#if RU_INCLUDE_DESC
    "COLOR_MIN_THR %i Register",
    "WRED Color min thresholds",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_0_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_REG_RAM_CNT,
    48,
    51,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_MIN_THR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_MIN_THR_1_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD,
    &QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD,
    &QM_WRED_PROFILE_COLOR_MIN_THR_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_WRED_PROFILE_COLOR_MIN_THR_1_REG = 
{
    "WRED_PROFILE_COLOR_MIN_THR_1",
#if RU_INCLUDE_DESC
    "COLOR_MIN_THR %i Register",
    "WRED Color min thresholds",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_1_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_REG_RAM_CNT,
    48,
    52,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_MAX_THR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_MAX_THR_0_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD,
    &QM_WRED_PROFILE_COLOR_MAX_THR_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_WRED_PROFILE_COLOR_MAX_THR_0_REG = 
{
    "WRED_PROFILE_COLOR_MAX_THR_0",
#if RU_INCLUDE_DESC
    "COLOR_MAX_THR %i Register",
    "WRED Color max thresholds",
#endif
    QM_WRED_PROFILE_COLOR_MAX_THR_0_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_MAX_THR_0_REG_RAM_CNT,
    48,
    53,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_WRED_PROFILE_COLOR_MAX_THR_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_MAX_THR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_MAX_THR_1_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD,
    &QM_WRED_PROFILE_COLOR_MAX_THR_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_WRED_PROFILE_COLOR_MAX_THR_1_REG = 
{
    "WRED_PROFILE_COLOR_MAX_THR_1",
#if RU_INCLUDE_DESC
    "COLOR_MAX_THR %i Register",
    "WRED Color max thresholds",
#endif
    QM_WRED_PROFILE_COLOR_MAX_THR_1_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_MAX_THR_1_REG_RAM_CNT,
    48,
    54,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_WRED_PROFILE_COLOR_MAX_THR_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_SLOPE_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_SLOPE_0_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD,
    &QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD,
    &QM_WRED_PROFILE_COLOR_SLOPE_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_WRED_PROFILE_COLOR_SLOPE_0_REG = 
{
    "WRED_PROFILE_COLOR_SLOPE_0",
#if RU_INCLUDE_DESC
    "COLOR_SLOPE %i Register",
    "WRED Color slopes",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_0_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_SLOPE_0_REG_RAM_CNT,
    48,
    55,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_WRED_PROFILE_COLOR_SLOPE_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_SLOPE_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_SLOPE_1_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD,
    &QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD,
    &QM_WRED_PROFILE_COLOR_SLOPE_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_WRED_PROFILE_COLOR_SLOPE_1_REG = 
{
    "WRED_PROFILE_COLOR_SLOPE_1",
#if RU_INCLUDE_DESC
    "COLOR_SLOPE %i Register",
    "WRED Color slopes",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_1_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_SLOPE_1_REG_RAM_CNT,
    48,
    56,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_WRED_PROFILE_COLOR_SLOPE_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_COPY_DECISION_PROFILE_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_COPY_DECISION_PROFILE_THR_FIELDS[] =
{
    &QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD,
    &QM_COPY_DECISION_PROFILE_THR_RESERVED0_FIELD,
    &QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_COPY_DECISION_PROFILE_THR_REG = 
{
    "COPY_DECISION_PROFILE_THR",
#if RU_INCLUDE_DESC
    "THR Register",
    "DDR Pipe and PSRAM threshold configurations for DDR copy decision logic",
#endif
    QM_COPY_DECISION_PROFILE_THR_REG_OFFSET,
    QM_COPY_DECISION_PROFILE_THR_REG_RAM_CNT,
    32,
    57,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_COPY_DECISION_PROFILE_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_TOTAL_VALID_COUNTER_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_TOTAL_VALID_COUNTER_COUNTER_FIELDS[] =
{
    &QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_TOTAL_VALID_COUNTER_COUNTER_REG = 
{
    "TOTAL_VALID_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER %i Register",
    "Counter."
    "word0:{15'`b0,pkt_cnt[16:0]}"
    "word1:{2'`b0,byte_cnt[29:0]}"
    ""
    "There are two words per queue starting at queue0 up to queue 159/287.",
#endif
    QM_TOTAL_VALID_COUNTER_COUNTER_REG_OFFSET,
    QM_TOTAL_VALID_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    58,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_TOTAL_VALID_COUNTER_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DQM_VALID_COUNTER_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DQM_VALID_COUNTER_COUNTER_FIELDS[] =
{
    &QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DQM_VALID_COUNTER_COUNTER_REG = 
{
    "DQM_VALID_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER %i Register",
    "Counter."
    "word0:{15`b0,pkt_cnt[16:0]}"
    "word1:{2`b0,byte_cnt[29:0]}"
    ""
    "There are two words per queue starting at queue0 up to queue 287.",
#endif
    QM_DQM_VALID_COUNTER_COUNTER_REG_OFFSET,
    QM_DQM_VALID_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    59,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DQM_VALID_COUNTER_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DROP_COUNTER_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DROP_COUNTER_COUNTER_FIELDS[] =
{
    &QM_DROP_COUNTER_COUNTER_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DROP_COUNTER_COUNTER_REG = 
{
    "DROP_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER %i Register",
    "Counter."
    "word0:{6`b0,pkt_cnt[25:0]}"
    "word1:{byte_cnt[31:0]}"
    ""
    "There are two words per queue starting at queue0 up to queue 287.",
#endif
    QM_DROP_COUNTER_COUNTER_REG_OFFSET,
    QM_DROP_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    60,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DROP_COUNTER_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_EPON_RPT_CNT_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EPON_RPT_CNT_COUNTER_FIELDS[] =
{
    &QM_EPON_RPT_CNT_COUNTER_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_EPON_RPT_CNT_COUNTER_REG = 
{
    "EPON_RPT_CNT_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER %i Register",
    "Counter - For each of the 32-queues in a batch, this counter stores a 32-bit accumulated and overhead byte counter per queue."
    "word0: {accumulated_bytes[31:0]}"
    "word1: {accumulated_overhead[31:0}"
    ""
    "There are two words per queue starting at queue0 up to queue 127.",
#endif
    QM_EPON_RPT_CNT_COUNTER_REG_OFFSET,
    QM_EPON_RPT_CNT_COUNTER_REG_RAM_CNT,
    4,
    61,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EPON_RPT_CNT_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_EPON_RPT_CNT_QUEUE_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EPON_RPT_CNT_QUEUE_STATUS_FIELDS[] =
{
    &QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_EPON_RPT_CNT_QUEUE_STATUS_REG = 
{
    "EPON_RPT_CNT_QUEUE_STATUS",
#if RU_INCLUDE_DESC
    "QUEUE_STATUS %i Register",
    "Status bit vector - For each of the 32-queues in a batch, this status indicates which queue counter has been updated.",
#endif
    QM_EPON_RPT_CNT_QUEUE_STATUS_REG_OFFSET,
    QM_EPON_RPT_CNT_QUEUE_STATUS_REG_RAM_CNT,
    4,
    62,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EPON_RPT_CNT_QUEUE_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RD_DATA_POOL0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_POOL0_FIELDS[] =
{
    &QM_RD_DATA_POOL0_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RD_DATA_POOL0_REG = 
{
    "RD_DATA_POOL0",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL0 Register",
    "Read the head of the FIFO",
#endif
    QM_RD_DATA_POOL0_REG_OFFSET,
    0,
    0,
    63,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_POOL0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RD_DATA_POOL1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_POOL1_FIELDS[] =
{
    &QM_RD_DATA_POOL1_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RD_DATA_POOL1_REG = 
{
    "RD_DATA_POOL1",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL1 Register",
    "Read the head of the FIFO",
#endif
    QM_RD_DATA_POOL1_REG_OFFSET,
    0,
    0,
    64,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_POOL1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RD_DATA_POOL2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_POOL2_FIELDS[] =
{
    &QM_RD_DATA_POOL2_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RD_DATA_POOL2_REG = 
{
    "RD_DATA_POOL2",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL2 Register",
    "Read the head of the FIFO",
#endif
    QM_RD_DATA_POOL2_REG_OFFSET,
    0,
    0,
    65,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_POOL2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RD_DATA_POOL3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_POOL3_FIELDS[] =
{
    &QM_RD_DATA_POOL3_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RD_DATA_POOL3_REG = 
{
    "RD_DATA_POOL3",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL3 Register",
    "Read the head of the FIFO",
#endif
    QM_RD_DATA_POOL3_REG_OFFSET,
    0,
    0,
    66,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_POOL3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_PDFIFO_PTR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_PDFIFO_PTR_FIELDS[] =
{
    &QM_PDFIFO_PTR_WR_PTR_FIELD,
    &QM_PDFIFO_PTR_RESERVED0_FIELD,
    &QM_PDFIFO_PTR_RD_PTR_FIELD,
    &QM_PDFIFO_PTR_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_PDFIFO_PTR_REG = 
{
    "PDFIFO_PTR",
#if RU_INCLUDE_DESC
    "PDFIFO_PTR %i Register",
    "PDFIFO per queue rd/wr pointers",
#endif
    QM_PDFIFO_PTR_REG_OFFSET,
    QM_PDFIFO_PTR_REG_RAM_CNT,
    4,
    67,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_PDFIFO_PTR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_UPDATE_FIFO_PTR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_UPDATE_FIFO_PTR_FIELDS[] =
{
    &QM_UPDATE_FIFO_PTR_WR_PTR_FIELD,
    &QM_UPDATE_FIFO_PTR_RD_PTR_FIELD,
    &QM_UPDATE_FIFO_PTR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_UPDATE_FIFO_PTR_REG = 
{
    "UPDATE_FIFO_PTR",
#if RU_INCLUDE_DESC
    "UPDATE_FIFO_PTR %i Register",
    "Update FIFO rd/wr pointers",
#endif
    QM_UPDATE_FIFO_PTR_REG_OFFSET,
    QM_UPDATE_FIFO_PTR_REG_RAM_CNT,
    4,
    68,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_UPDATE_FIFO_PTR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_RD_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_FIELDS[] =
{
    &QM_RD_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_RD_DATA_REG = 
{
    "RD_DATA",
#if RU_INCLUDE_DESC
    "RD_DATA %i Register",
    "Debug - Read the head of the FIFO",
#endif
    QM_RD_DATA_REG_OFFSET,
    QM_RD_DATA_REG_RAM_CNT,
    4,
    69,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_POP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_POP_FIELDS[] =
{
    &QM_POP_POP_FIELD,
    &QM_POP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_POP_REG = 
{
    "POP",
#if RU_INCLUDE_DESC
    "POP Register",
    "Pop an entry in the FIFO",
#endif
    QM_POP_REG_OFFSET,
    0,
    0,
    70,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_POP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CM_COMMON_INPUT_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CM_COMMON_INPUT_FIFO_DATA_FIELDS[] =
{
    &QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CM_COMMON_INPUT_FIFO_DATA_REG = 
{
    "CM_COMMON_INPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "CM Common Input FIFO - debug access",
#endif
    QM_CM_COMMON_INPUT_FIFO_DATA_REG_OFFSET,
    QM_CM_COMMON_INPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    71,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CM_COMMON_INPUT_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_NORMAL_RMT_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NORMAL_RMT_FIFO_DATA_FIELDS[] =
{
    &QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_NORMAL_RMT_FIFO_DATA_REG = 
{
    "NORMAL_RMT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Normal Remote FIFO - debug access",
#endif
    QM_NORMAL_RMT_FIFO_DATA_REG_OFFSET,
    QM_NORMAL_RMT_FIFO_DATA_REG_RAM_CNT,
    4,
    72,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_NORMAL_RMT_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_NON_DELAYED_RMT_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NON_DELAYED_RMT_FIFO_DATA_FIELDS[] =
{
    &QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_NON_DELAYED_RMT_FIFO_DATA_REG = 
{
    "NON_DELAYED_RMT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Non-delayed Remote FIFO - debug access",
#endif
    QM_NON_DELAYED_RMT_FIFO_DATA_REG_OFFSET,
    QM_NON_DELAYED_RMT_FIFO_DATA_REG_RAM_CNT,
    4,
    73,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_NON_DELAYED_RMT_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_EGRESS_DATA_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_DATA_FIFO_DATA_FIELDS[] =
{
    &QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_EGRESS_DATA_FIFO_DATA_REG = 
{
    "EGRESS_DATA_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Egress data FIFO - debug access",
#endif
    QM_EGRESS_DATA_FIFO_DATA_REG_OFFSET,
    QM_EGRESS_DATA_FIFO_DATA_REG_RAM_CNT,
    4,
    74,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EGRESS_DATA_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_EGRESS_RR_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_RR_FIFO_DATA_FIELDS[] =
{
    &QM_EGRESS_RR_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_EGRESS_RR_FIFO_DATA_REG = 
{
    "EGRESS_RR_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Egress RR FIFO - debug access",
#endif
    QM_EGRESS_RR_FIFO_DATA_REG_OFFSET,
    QM_EGRESS_RR_FIFO_DATA_REG_RAM_CNT,
    4,
    75,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EGRESS_RR_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_EGRESS_BB_INPUT_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_BB_INPUT_FIFO_DATA_FIELDS[] =
{
    &QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_EGRESS_BB_INPUT_FIFO_DATA_REG = 
{
    "EGRESS_BB_INPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Egress BB Input FIFO - debug access",
#endif
    QM_EGRESS_BB_INPUT_FIFO_DATA_REG_OFFSET,
    QM_EGRESS_BB_INPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    76,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EGRESS_BB_INPUT_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_EGRESS_BB_OUTPUT_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_BB_OUTPUT_FIFO_DATA_FIELDS[] =
{
    &QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG = 
{
    "EGRESS_BB_OUTPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Egress BB Output FIFO - debug access",
#endif
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG_OFFSET,
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    77,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BB_OUTPUT_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB_OUTPUT_FIFO_DATA_FIELDS[] =
{
    &QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BB_OUTPUT_FIFO_DATA_REG = 
{
    "BB_OUTPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "QM BB Output FIFO - debug access",
#endif
    QM_BB_OUTPUT_FIFO_DATA_REG_OFFSET,
    QM_BB_OUTPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    78,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BB_OUTPUT_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_NON_DELAYED_OUT_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NON_DELAYED_OUT_FIFO_DATA_FIELDS[] =
{
    &QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_NON_DELAYED_OUT_FIFO_DATA_REG = 
{
    "NON_DELAYED_OUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Non delayed output FIFO - debug access",
#endif
    QM_NON_DELAYED_OUT_FIFO_DATA_REG_OFFSET,
    QM_NON_DELAYED_OUT_FIFO_DATA_REG_RAM_CNT,
    4,
    79,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_NON_DELAYED_OUT_FIFO_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CONTEXT_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CONTEXT_DATA_FIELDS[] =
{
    &QM_CONTEXT_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CONTEXT_DATA_REG = 
{
    "CONTEXT_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Aggregation context - debug access",
#endif
    QM_CONTEXT_DATA_REG_OFFSET,
    QM_CONTEXT_DATA_REG_RAM_CNT,
    4,
    80,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CONTEXT_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DEBUG_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DEBUG_SEL_FIELDS[] =
{
    &QM_DEBUG_SEL_SELECT_FIELD,
    &QM_DEBUG_SEL_RESERVED0_FIELD,
    &QM_DEBUG_SEL_ENABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DEBUG_SEL_REG = 
{
    "DEBUG_SEL",
#if RU_INCLUDE_DESC
    "DEBUG_SEL Register",
    "Controls Debug bus select:"
    ""
    "5h1:  qm_dbg_bus = qm_bb_input_dbg_bus;"
    "5h2:  qm_dbg_bus = qm_bb_output_dbg_bus;"
    "5h3:  qm_dbg_bus = qm_cm_dbg_bus;"
    "5h4:  qm_dbg_bus = qm_ddr_write_dbg_bus;"
    "5h5:  qm_dbg_bus = qm_counters_dbg_bus;"
    "5h6:  qm_dbg_bus = qm_cpu_if_dbg_bus;"
    "5h7:  qm_dbg_bus = qm_dqm_push_dbg_bus;"
    "5h8:  qm_dbg_bus = qm_egress_dbg_bus;"
    "5h9:  qm_dbg_bus = qm_fpm_prefetch_dbg_bus;"
    "5ha:  qm_dbg_bus = qm_ingress_dbg_bus;"
    "5hb:  qm_dbg_bus = qm_rmt_fifos_dbg_bus;"
    "5hc:  qm_dbg_bus = {19b0,bbh_debug_0};"
    "5hd:  qm_dbg_bus = {19b0,bbh_debug_1};"
    "5he:  qm_dbg_bus = {19b0,bbh_debug_2};"
    "5hf:  qm_dbg_bus = {19b0,bbh_debug_3};"
    "5h10: qm_dbg_bus = {19b0,bbh_debug_4};"
    "5h11: qm_dbg_bus = {19b0,bbh_debug_5};"
    "5h12: qm_dbg_bus = {19b0,bbh_debug_6};"
    "5h13: qm_dbg_bus = {19b0,bbh_debug_7};"
    "5h14: qm_dbg_bus = {19b0,bbh_debug_8};"
    "5h15: qm_dbg_bus = {19b0,bbh_debug_9};"
    "5h16: qm_dbg_bus = {19b0,dma_debug_vec};"
    "5h17: qm_dbg_bus = {8b0,dqm_diag_r};"
    "",
#endif
    QM_DEBUG_SEL_REG_OFFSET,
    0,
    0,
    81,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_DEBUG_SEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DEBUG_BUS_LSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DEBUG_BUS_LSB_FIELDS[] =
{
    &QM_DEBUG_BUS_LSB_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DEBUG_BUS_LSB_REG = 
{
    "DEBUG_BUS_LSB",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_LSB Register",
    "Debug Bus sampling",
#endif
    QM_DEBUG_BUS_LSB_REG_OFFSET,
    0,
    0,
    82,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DEBUG_BUS_LSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DEBUG_BUS_MSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DEBUG_BUS_MSB_FIELDS[] =
{
    &QM_DEBUG_BUS_MSB_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DEBUG_BUS_MSB_REG = 
{
    "DEBUG_BUS_MSB",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_MSB Register",
    "Debug Bus sampling",
#endif
    QM_DEBUG_BUS_MSB_REG_OFFSET,
    0,
    0,
    83,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DEBUG_BUS_MSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_SPARE_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_SPARE_CONFIG_FIELDS[] =
{
    &QM_QM_SPARE_CONFIG_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_SPARE_CONFIG_REG = 
{
    "QM_SPARE_CONFIG",
#if RU_INCLUDE_DESC
    "QM_SPARE_CONFIG Register",
    "Spare configuration for ECO purposes",
#endif
    QM_QM_SPARE_CONFIG_REG_OFFSET,
    0,
    0,
    84,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_SPARE_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GOOD_LVL1_PKTS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GOOD_LVL1_PKTS_CNT_FIELDS[] =
{
    &QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GOOD_LVL1_PKTS_CNT_REG = 
{
    "GOOD_LVL1_PKTS_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL1_PKTS_CNT Register",
    "Counts the total number of non-dropped and non-reprocessing packets from all queues",
#endif
    QM_GOOD_LVL1_PKTS_CNT_REG_OFFSET,
    0,
    0,
    85,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GOOD_LVL1_PKTS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GOOD_LVL1_BYTES_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GOOD_LVL1_BYTES_CNT_FIELDS[] =
{
    &QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GOOD_LVL1_BYTES_CNT_REG = 
{
    "GOOD_LVL1_BYTES_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL1_BYTES_CNT Register",
    "Counts the total number of non-dropped and non-reprocessing bytes from all queues",
#endif
    QM_GOOD_LVL1_BYTES_CNT_REG_OFFSET,
    0,
    0,
    86,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GOOD_LVL1_BYTES_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GOOD_LVL2_PKTS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GOOD_LVL2_PKTS_CNT_FIELDS[] =
{
    &QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GOOD_LVL2_PKTS_CNT_REG = 
{
    "GOOD_LVL2_PKTS_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL2_PKTS_CNT Register",
    "Counts the total number of non-dropped and reprocessing packets from all queues",
#endif
    QM_GOOD_LVL2_PKTS_CNT_REG_OFFSET,
    0,
    0,
    87,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GOOD_LVL2_PKTS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GOOD_LVL2_BYTES_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GOOD_LVL2_BYTES_CNT_FIELDS[] =
{
    &QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GOOD_LVL2_BYTES_CNT_REG = 
{
    "GOOD_LVL2_BYTES_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL2_BYTES_CNT Register",
    "Counts the total number of non-dropped and reprocessing bytes from all queues",
#endif
    QM_GOOD_LVL2_BYTES_CNT_REG_OFFSET,
    0,
    0,
    88,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GOOD_LVL2_BYTES_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_COPIED_PKTS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_COPIED_PKTS_CNT_FIELDS[] =
{
    &QM_COPIED_PKTS_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_COPIED_PKTS_CNT_REG = 
{
    "COPIED_PKTS_CNT",
#if RU_INCLUDE_DESC
    "COPIED_PKTS_CNT Register",
    "Counts the total number of copied packets to the DDR from all queues",
#endif
    QM_COPIED_PKTS_CNT_REG_OFFSET,
    0,
    0,
    89,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_COPIED_PKTS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_COPIED_BYTES_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_COPIED_BYTES_CNT_FIELDS[] =
{
    &QM_COPIED_BYTES_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_COPIED_BYTES_CNT_REG = 
{
    "COPIED_BYTES_CNT",
#if RU_INCLUDE_DESC
    "COPIED_BYTES_CNT Register",
    "Counts the total number of copied bytes to the DDR from all queues",
#endif
    QM_COPIED_BYTES_CNT_REG_OFFSET,
    0,
    0,
    90,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_COPIED_BYTES_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_AGG_PKTS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_PKTS_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_AGG_PKTS_CNT_REG = 
{
    "AGG_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_PKTS_CNT Register",
    "Counts the total number of aggregated packets from all queues",
#endif
    QM_AGG_PKTS_CNT_REG_OFFSET,
    0,
    0,
    91,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_PKTS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_AGG_BYTES_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_BYTES_CNT_FIELDS[] =
{
    &QM_AGG_BYTES_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_AGG_BYTES_CNT_REG = 
{
    "AGG_BYTES_CNT",
#if RU_INCLUDE_DESC
    "AGG_BYTES_CNT Register",
    "Counts the total number of aggregated bytes from all queues",
#endif
    QM_AGG_BYTES_CNT_REG_OFFSET,
    0,
    0,
    92,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_BYTES_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_AGG_1_PKTS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_1_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_1_PKTS_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_AGG_1_PKTS_CNT_REG = 
{
    "AGG_1_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_1_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 1-packet PD from all queues",
#endif
    QM_AGG_1_PKTS_CNT_REG_OFFSET,
    0,
    0,
    93,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_1_PKTS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_AGG_2_PKTS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_2_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_2_PKTS_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_AGG_2_PKTS_CNT_REG = 
{
    "AGG_2_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_2_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 2-packet PD from all queues",
#endif
    QM_AGG_2_PKTS_CNT_REG_OFFSET,
    0,
    0,
    94,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_2_PKTS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_AGG_3_PKTS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_3_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_3_PKTS_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_AGG_3_PKTS_CNT_REG = 
{
    "AGG_3_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_3_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 3-packet PD from all queues",
#endif
    QM_AGG_3_PKTS_CNT_REG_OFFSET,
    0,
    0,
    95,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_3_PKTS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_AGG_4_PKTS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_4_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_4_PKTS_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_AGG_4_PKTS_CNT_REG = 
{
    "AGG_4_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_4_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 4-packet PD from all queues",
#endif
    QM_AGG_4_PKTS_CNT_REG_OFFSET,
    0,
    0,
    96,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_4_PKTS_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_WRED_DROP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_DROP_CNT_FIELDS[] =
{
    &QM_WRED_DROP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_WRED_DROP_CNT_REG = 
{
    "WRED_DROP_CNT",
#if RU_INCLUDE_DESC
    "WRED_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to WRED",
#endif
    QM_WRED_DROP_CNT_REG_OFFSET,
    0,
    0,
    97,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_WRED_DROP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_CONGESTION_DROP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_CONGESTION_DROP_CNT_FIELDS[] =
{
    &QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_CONGESTION_DROP_CNT_REG = 
{
    "FPM_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to FPM congestion indication",
#endif
    QM_FPM_CONGESTION_DROP_CNT_REG_OFFSET,
    0,
    0,
    98,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_CONGESTION_DROP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DDR_PD_CONGESTION_DROP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DDR_PD_CONGESTION_DROP_CNT_FIELDS[] =
{
    &QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DDR_PD_CONGESTION_DROP_CNT_REG = 
{
    "DDR_PD_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to DDR PD congestion",
#endif
    QM_DDR_PD_CONGESTION_DROP_CNT_REG_OFFSET,
    0,
    0,
    99,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DDR_PD_CONGESTION_DROP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DDR_BYTE_CONGESTION_DROP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DDR_BYTE_CONGESTION_DROP_CNT_FIELDS[] =
{
    &QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DDR_BYTE_CONGESTION_DROP_CNT_REG = 
{
    "DDR_BYTE_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to DDR byte congestion (number of bytes waiting to be copied exceeded the thresholds)",
#endif
    QM_DDR_BYTE_CONGESTION_DROP_CNT_REG_OFFSET,
    0,
    0,
    100,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DDR_BYTE_CONGESTION_DROP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_PD_CONGESTION_DROP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_PD_CONGESTION_DROP_CNT_FIELDS[] =
{
    &QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_PD_CONGESTION_DROP_CNT_REG = 
{
    "QM_PD_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "QM_PD_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to QM PD congestion (this value is limited by the DQM)",
#endif
    QM_QM_PD_CONGESTION_DROP_CNT_REG_OFFSET,
    0,
    0,
    101,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_PD_CONGESTION_DROP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_ABS_REQUEUE_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_ABS_REQUEUE_CNT_FIELDS[] =
{
    &QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_ABS_REQUEUE_CNT_REG = 
{
    "QM_ABS_REQUEUE_CNT",
#if RU_INCLUDE_DESC
    "QM_ABS_REQUEUE_CNT Register",
    "Counts the total number of packets requeued due to absolute address drops from all queues",
#endif
    QM_QM_ABS_REQUEUE_CNT_REG_OFFSET,
    0,
    0,
    102,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_ABS_REQUEUE_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_PREFETCH_FIFO0_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_PREFETCH_FIFO0_STATUS_FIELDS[] =
{
    &QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD,
    &QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD,
    &QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD,
    &QM_FPM_PREFETCH_FIFO0_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_PREFETCH_FIFO0_STATUS_REG = 
{
    "FPM_PREFETCH_FIFO0_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO0_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_FPM_PREFETCH_FIFO0_STATUS_REG_OFFSET,
    0,
    0,
    103,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_FPM_PREFETCH_FIFO0_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_PREFETCH_FIFO1_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_PREFETCH_FIFO1_STATUS_FIELDS[] =
{
    &QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD,
    &QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD,
    &QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD,
    &QM_FPM_PREFETCH_FIFO1_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_PREFETCH_FIFO1_STATUS_REG = 
{
    "FPM_PREFETCH_FIFO1_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO1_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_FPM_PREFETCH_FIFO1_STATUS_REG_OFFSET,
    0,
    0,
    104,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_FPM_PREFETCH_FIFO1_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_PREFETCH_FIFO2_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_PREFETCH_FIFO2_STATUS_FIELDS[] =
{
    &QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD,
    &QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD,
    &QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD,
    &QM_FPM_PREFETCH_FIFO2_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_PREFETCH_FIFO2_STATUS_REG = 
{
    "FPM_PREFETCH_FIFO2_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO2_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_FPM_PREFETCH_FIFO2_STATUS_REG_OFFSET,
    0,
    0,
    105,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_FPM_PREFETCH_FIFO2_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_PREFETCH_FIFO3_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_PREFETCH_FIFO3_STATUS_FIELDS[] =
{
    &QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD,
    &QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD,
    &QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD,
    &QM_FPM_PREFETCH_FIFO3_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_PREFETCH_FIFO3_STATUS_REG = 
{
    "FPM_PREFETCH_FIFO3_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO3_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_FPM_PREFETCH_FIFO3_STATUS_REG_OFFSET,
    0,
    0,
    106,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_FPM_PREFETCH_FIFO3_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_NORMAL_RMT_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NORMAL_RMT_FIFO_STATUS_FIELDS[] =
{
    &QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD,
    &QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD,
    &QM_NORMAL_RMT_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_NORMAL_RMT_FIFO_STATUS_REG = 
{
    "NORMAL_RMT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NORMAL_RMT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_NORMAL_RMT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    107,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_NORMAL_RMT_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_NON_DELAYED_RMT_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NON_DELAYED_RMT_FIFO_STATUS_FIELDS[] =
{
    &QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD,
    &QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD,
    &QM_NON_DELAYED_RMT_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_NON_DELAYED_RMT_FIFO_STATUS_REG = 
{
    "NON_DELAYED_RMT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NON_DELAYED_RMT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_NON_DELAYED_RMT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    108,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_NON_DELAYED_RMT_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_NON_DELAYED_OUT_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NON_DELAYED_OUT_FIFO_STATUS_FIELDS[] =
{
    &QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD,
    &QM_NON_DELAYED_OUT_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_NON_DELAYED_OUT_FIFO_STATUS_REG = 
{
    "NON_DELAYED_OUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NON_DELAYED_OUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_NON_DELAYED_OUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    109,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_NON_DELAYED_OUT_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_PRE_CM_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_PRE_CM_FIFO_STATUS_FIELDS[] =
{
    &QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD,
    &QM_PRE_CM_FIFO_STATUS_FULL_FIELD,
    &QM_PRE_CM_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_PRE_CM_FIFO_STATUS_REG = 
{
    "PRE_CM_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PRE_CM_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_PRE_CM_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    110,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_PRE_CM_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CM_RD_PD_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CM_RD_PD_FIFO_STATUS_FIELDS[] =
{
    &QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD,
    &QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD,
    &QM_CM_RD_PD_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CM_RD_PD_FIFO_STATUS_REG = 
{
    "CM_RD_PD_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_RD_PD_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_CM_RD_PD_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    111,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_CM_RD_PD_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CM_WR_PD_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CM_WR_PD_FIFO_STATUS_FIELDS[] =
{
    &QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD,
    &QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD,
    &QM_CM_WR_PD_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CM_WR_PD_FIFO_STATUS_REG = 
{
    "CM_WR_PD_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_WR_PD_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_CM_WR_PD_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    112,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_CM_WR_PD_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_CM_COMMON_INPUT_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CM_COMMON_INPUT_FIFO_STATUS_FIELDS[] =
{
    &QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD,
    &QM_CM_COMMON_INPUT_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_CM_COMMON_INPUT_FIFO_STATUS_REG = 
{
    "CM_COMMON_INPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_COMMON_INPUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_CM_COMMON_INPUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    113,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_CM_COMMON_INPUT_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BB0_OUTPUT_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB0_OUTPUT_FIFO_STATUS_FIELDS[] =
{
    &QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD,
    &QM_BB0_OUTPUT_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BB0_OUTPUT_FIFO_STATUS_REG = 
{
    "BB0_OUTPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB0_OUTPUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_BB0_OUTPUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    114,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_BB0_OUTPUT_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BB1_OUTPUT_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB1_OUTPUT_FIFO_STATUS_FIELDS[] =
{
    &QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD,
    &QM_BB1_OUTPUT_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BB1_OUTPUT_FIFO_STATUS_REG = 
{
    "BB1_OUTPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB1_OUTPUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_BB1_OUTPUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    115,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_BB1_OUTPUT_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BB1_INPUT_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB1_INPUT_FIFO_STATUS_FIELDS[] =
{
    &QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD,
    &QM_BB1_INPUT_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BB1_INPUT_FIFO_STATUS_REG = 
{
    "BB1_INPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB1_INPUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_BB1_INPUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    116,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_BB1_INPUT_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_EGRESS_DATA_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_DATA_FIFO_STATUS_FIELDS[] =
{
    &QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD,
    &QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD,
    &QM_EGRESS_DATA_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_EGRESS_DATA_FIFO_STATUS_REG = 
{
    "EGRESS_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "EGRESS_DATA_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_EGRESS_DATA_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    117,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_EGRESS_DATA_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_EGRESS_RR_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_RR_FIFO_STATUS_FIELDS[] =
{
    &QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD,
    &QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD,
    &QM_EGRESS_RR_FIFO_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_EGRESS_RR_FIFO_STATUS_REG = 
{
    "EGRESS_RR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "EGRESS_RR_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_EGRESS_RR_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    118,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_EGRESS_RR_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BB_ROUTE_OVR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB_ROUTE_OVR_FIELDS[] =
{
    &QM_BB_ROUTE_OVR_OVR_EN_FIELD,
    &QM_BB_ROUTE_OVR_RESERVED0_FIELD,
    &QM_BB_ROUTE_OVR_DEST_ID_FIELD,
    &QM_BB_ROUTE_OVR_RESERVED1_FIELD,
    &QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD,
    &QM_BB_ROUTE_OVR_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BB_ROUTE_OVR_REG = 
{
    "BB_ROUTE_OVR",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVR %i Register",
    "BB ROUTE Override:"
    "0 - for QM_TOP"
    "1 - for RNR_GRID",
#endif
    QM_BB_ROUTE_OVR_REG_OFFSET,
    QM_BB_ROUTE_OVR_REG_RAM_CNT,
    4,
    119,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    QM_BB_ROUTE_OVR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_INGRESS_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_INGRESS_STAT_FIELDS[] =
{
    &QM_QM_INGRESS_STAT_STAT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_INGRESS_STAT_REG = 
{
    "QM_INGRESS_STAT",
#if RU_INCLUDE_DESC
    "QM_INGRESS_STAT Register",
    "Holds the Ingress Status",
#endif
    QM_QM_INGRESS_STAT_REG_OFFSET,
    0,
    0,
    120,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_INGRESS_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_EGRESS_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_EGRESS_STAT_FIELDS[] =
{
    &QM_QM_EGRESS_STAT_STAT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_EGRESS_STAT_REG = 
{
    "QM_EGRESS_STAT",
#if RU_INCLUDE_DESC
    "QM_EGRESS_STAT Register",
    "Holds the Egress Status",
#endif
    QM_QM_EGRESS_STAT_REG_OFFSET,
    0,
    0,
    121,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_EGRESS_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_CM_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_CM_STAT_FIELDS[] =
{
    &QM_QM_CM_STAT_STAT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_CM_STAT_REG = 
{
    "QM_CM_STAT",
#if RU_INCLUDE_DESC
    "QM_CM_STAT Register",
    "Holds the CM Status",
#endif
    QM_QM_CM_STAT_REG_OFFSET,
    0,
    0,
    122,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_CM_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_FPM_PREFETCH_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_FPM_PREFETCH_STAT_FIELDS[] =
{
    &QM_QM_FPM_PREFETCH_STAT_STAT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_FPM_PREFETCH_STAT_REG = 
{
    "QM_FPM_PREFETCH_STAT",
#if RU_INCLUDE_DESC
    "QM_FPM_PREFETCH_STAT Register",
    "Holds the FPM Prefetch Status",
#endif
    QM_QM_FPM_PREFETCH_STAT_REG_OFFSET,
    0,
    0,
    123,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_FPM_PREFETCH_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_CONNECT_ACK_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_CONNECT_ACK_COUNTER_FIELDS[] =
{
    &QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD,
    &QM_QM_CONNECT_ACK_COUNTER_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_CONNECT_ACK_COUNTER_REG = 
{
    "QM_CONNECT_ACK_COUNTER",
#if RU_INCLUDE_DESC
    "QM_CONNECT_ACK_COUNTER Register",
    "QM connect ack counter",
#endif
    QM_QM_CONNECT_ACK_COUNTER_REG_OFFSET,
    0,
    0,
    124,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_QM_CONNECT_ACK_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_DDR_WR_REPLY_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_DDR_WR_REPLY_COUNTER_FIELDS[] =
{
    &QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD,
    &QM_QM_DDR_WR_REPLY_COUNTER_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_DDR_WR_REPLY_COUNTER_REG = 
{
    "QM_DDR_WR_REPLY_COUNTER",
#if RU_INCLUDE_DESC
    "QM_DDR_WR_REPLY_COUNTER Register",
    "QM DDR WR reply Counter",
#endif
    QM_QM_DDR_WR_REPLY_COUNTER_REG_OFFSET,
    0,
    0,
    125,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_QM_DDR_WR_REPLY_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_DDR_PIPE_BYTE_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_DDR_PIPE_BYTE_COUNTER_FIELDS[] =
{
    &QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD,
    &QM_QM_DDR_PIPE_BYTE_COUNTER_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_DDR_PIPE_BYTE_COUNTER_REG = 
{
    "QM_DDR_PIPE_BYTE_COUNTER",
#if RU_INCLUDE_DESC
    "QM_DDR_PIPE_BYTE_COUNTER Register",
    "QM DDR pipe byte counter",
#endif
    QM_QM_DDR_PIPE_BYTE_COUNTER_REG_OFFSET,
    0,
    0,
    126,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_QM_DDR_PIPE_BYTE_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_ABS_REQUEUE_VALID_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_ABS_REQUEUE_VALID_COUNTER_FIELDS[] =
{
    &QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD,
    &QM_QM_ABS_REQUEUE_VALID_COUNTER_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_ABS_REQUEUE_VALID_COUNTER_REG = 
{
    "QM_ABS_REQUEUE_VALID_COUNTER",
#if RU_INCLUDE_DESC
    "QM_ABS_REQUEUE_VALID_COUNTER Register",
    "Indicates the number of PDs currently in the Absolute address drop queue.",
#endif
    QM_QM_ABS_REQUEUE_VALID_COUNTER_REG_OFFSET,
    0,
    0,
    127,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_QM_ABS_REQUEUE_VALID_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_ILLEGAL_PD_CAPTURE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_ILLEGAL_PD_CAPTURE_FIELDS[] =
{
    &QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_ILLEGAL_PD_CAPTURE_REG = 
{
    "QM_ILLEGAL_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_ILLEGAL_PD_CAPTURE %i Register",
    "PD captured when an illegal PD was detected and the relevant interrupt was generated.",
#endif
    QM_QM_ILLEGAL_PD_CAPTURE_REG_OFFSET,
    QM_QM_ILLEGAL_PD_CAPTURE_REG_RAM_CNT,
    4,
    128,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_ILLEGAL_PD_CAPTURE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_INGRESS_PROCESSED_PD_CAPTURE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_INGRESS_PROCESSED_PD_CAPTURE_FIELDS[] =
{
    &QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG = 
{
    "QM_INGRESS_PROCESSED_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_INGRESS_PROCESSED_PD_CAPTURE %i Register",
    "Last ingress processed PD capture",
#endif
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG_OFFSET,
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG_RAM_CNT,
    4,
    129,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_QM_CM_PROCESSED_PD_CAPTURE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_CM_PROCESSED_PD_CAPTURE_FIELDS[] =
{
    &QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_QM_CM_PROCESSED_PD_CAPTURE_REG = 
{
    "QM_CM_PROCESSED_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_CM_PROCESSED_PD_CAPTURE %i Register",
    "Last copy machine processed PD capture",
#endif
    QM_QM_CM_PROCESSED_PD_CAPTURE_REG_OFFSET,
    QM_QM_CM_PROCESSED_PD_CAPTURE_REG_RAM_CNT,
    4,
    130,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_CM_PROCESSED_PD_CAPTURE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_POOL_DROP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_POOL_DROP_CNT_FIELDS[] =
{
    &QM_FPM_POOL_DROP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_POOL_DROP_CNT_REG = 
{
    "FPM_POOL_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_POOL_DROP_CNT %i Register",
    "Counts the total number of packets dropped for all queues due to FPM pool priority thresholds. Counter per pool",
#endif
    QM_FPM_POOL_DROP_CNT_REG_OFFSET,
    QM_FPM_POOL_DROP_CNT_REG_RAM_CNT,
    4,
    131,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_POOL_DROP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_GRP_DROP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_GRP_DROP_CNT_FIELDS[] =
{
    &QM_FPM_GRP_DROP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_GRP_DROP_CNT_REG = 
{
    "FPM_GRP_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_GRP_DROP_CNT %i Register",
    "Counts the total number of packets dropped from all queues due to FPM user group priority thresholds. Counter per UG (0-3)",
#endif
    QM_FPM_GRP_DROP_CNT_REG_OFFSET,
    QM_FPM_GRP_DROP_CNT_REG_RAM_CNT,
    4,
    132,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_GRP_DROP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_BUFFER_RES_DROP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_BUFFER_RES_DROP_CNT_FIELDS[] =
{
    &QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_BUFFER_RES_DROP_CNT_REG = 
{
    "FPM_BUFFER_RES_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_BUFFER_RES_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to buffer reservation mechanism.",
#endif
    QM_FPM_BUFFER_RES_DROP_CNT_REG_OFFSET,
    0,
    0,
    133,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_BUFFER_RES_DROP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_PSRAM_EGRESS_CONG_DRP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_PSRAM_EGRESS_CONG_DRP_CNT_FIELDS[] =
{
    &QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_PSRAM_EGRESS_CONG_DRP_CNT_REG = 
{
    "PSRAM_EGRESS_CONG_DRP_CNT",
#if RU_INCLUDE_DESC
    "PSRAM_EGRESS_CONG_DRP_CNT Register",
    "Counts the total number of packets dropped from all queues due to psram egress congestion.",
#endif
    QM_PSRAM_EGRESS_CONG_DRP_CNT_REG_OFFSET,
    0,
    0,
    134,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_PSRAM_EGRESS_CONG_DRP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DATA_FIELDS[] =
{
    &QM_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DATA_REG = 
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "CM Residue - debug access",
#endif
    QM_DATA_REG_OFFSET,
    QM_DATA_REG_RAM_CNT,
    4,
    135,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_FIELDS[] =
{
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE",
#if RU_INCLUDE_DESC
    "VPB_BASE Register",
    "VPB Base address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_REG_OFFSET,
    0,
    0,
    136,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_FIELDS[] =
{
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK",
#if RU_INCLUDE_DESC
    "VPB_MASK Register",
    "VPB mask address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_REG_OFFSET,
    0,
    0,
    137,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_FIELDS[] =
{
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE",
#if RU_INCLUDE_DESC
    "APB_BASE Register",
    "APB Base address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_REG_OFFSET,
    0,
    0,
    138,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_FIELDS[] =
{
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK",
#if RU_INCLUDE_DESC
    "APB_MASK Register",
    "APB mask address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_REG_OFFSET,
    0,
    0,
    139,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_FIELDS[] =
{
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE",
#if RU_INCLUDE_DESC
    "DQM_BASE Register",
    "DQM Base address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_REG_OFFSET,
    0,
    0,
    140,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_FIELDS[] =
{
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK",
#if RU_INCLUDE_DESC
    "DQM_MASK Register",
    "DQM mask address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_REG_OFFSET,
    0,
    0,
    141,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE",
#if RU_INCLUDE_DESC
    "MAC_TYPE Register",
    "The BBH supports working with different MAC types. Each MAC requires different interface and features. This register defines the type of MAC the BBH works with.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_REG_OFFSET,
    0,
    0,
    142,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_DMASRC_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SDMASRC_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_SBPMSRC_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED2_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FPMSRC_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_1 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG_OFFSET,
    0,
    0,
    143,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR0SRC_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_PDRNR1SRC_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_STSRNRSRC_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED2_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_MSGRNRSRC_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_2 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG_OFFSET,
    0,
    0,
    144,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_BYTERESUL_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_DDRTXOFFSET_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX",
#if RU_INCLUDE_DESC
    "RD_ADDR_CFG Register",
    "Configurations for determining the address to read from the DDR/PSRAm",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG_OFFSET,
    0,
    0,
    145,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_TCONTADDR_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_SKBADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_1 %i Register",
    "Queue index address:"
    "The BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner."
    "This register defines the queue index address within the Runner address space."
    "SKB address:"
    "When the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0)."
    "This register defines the SKB free base address within the Runner address."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_RAM_CNT,
    4,
    146,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_PTRADDR_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_TASK_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_2 %i Register",
    "PD transfer process:"
    "-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task."
    "-The Runner will push the PDs into the BBH (without any wakeup from the BBH)."
    "-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty)."
    "-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write."
    "-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case."
    "-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_RAM_CNT,
    4,
    147,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCBASE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_DESCSIZE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_MAXREQ_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_EPNURGNT_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_JUMBOURGNT_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX",
#if RU_INCLUDE_DESC
    "DMA_CFG Register",
    "The BBH reads the packet data from the DDR in chunks (with a maximal size of 128 bytes)."
    "For each chunk the BBH writes a read request (descriptor) into the DMA memory space. The read descriptors are arranged in a predefined space in the DMA memory and managed in a cyclic FIFO style."
    "A special configuration limits the maximum number of read requests.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_REG_OFFSET,
    0,
    0,
    148,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCBASE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_DESCSIZE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_MAXREQ_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_EPNURGNT_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_JUMBOURGNT_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX",
#if RU_INCLUDE_DESC
    "SDMA_CFG Register",
    "The BBH reads the packet data from the PSRAM in chunks (with a maximal size of 128 bytes)."
    "For each chunk the BBH writes a read request (descriptor) into the SDMA memory space. The read descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style."
    "A special configuration limits the maximum number of read requests.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG_OFFSET,
    0,
    0,
    149,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_SPECIALFREE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "When packet transmission is done, the BBH releases the SBPM buffers."
    "This register defines which release command is used:"
    "1. Normal free with context"
    "2. Special free with context"
    "3. free without context",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_REG_OFFSET,
    0,
    0,
    150,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_DDRTMBASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_LOW %i Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base."
    ""
    "The DDR TM address space is divided to two - coherent and non coherent."
    ""
    "The first register in this array defines the base address of the non coherent space and the second is for the coherent."
    ""
    "The value of this register should match the relevant registers value in the BBH RX, QM and the Runner."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_RAM_CNT,
    4,
    151,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_DDRTMBASE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_HIGH %i Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base."
    ""
    "The DDR TM address space is divided to two - coherent and non coherent."
    ""
    "The first register in this array defines the base address of the non coherent space and the second is for the coherent."
    ""
    "The value of this register should match the relevant registers value in the BBH RX, QM and the Runner."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_RAM_CNT,
    4,
    152,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMSIZE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_DDRSIZE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_PSRAMBASE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL",
#if RU_INCLUDE_DESC
    "DATA_FIFO_CTRL Register",
    "The BBH orders data both from DDR and PSRAM. The returned data is stored in two FIFOs for reordering. The two FIFOs are implemented in a single RAM. This register defines the division of the RAM to two FIFOs.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG_OFFSET,
    0,
    0,
    153,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_HIGHTRXQ_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG",
#if RU_INCLUDE_DESC
    "ARB_CFG Register",
    "configurations related to different arbitration processes (ordering PDs, ordering data)",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_REG_OFFSET,
    0,
    0,
    154,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_ROUTE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_DEST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_EN_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "override configuration for the route of one of the peripherals (DMA/SDMMA/FPM/SBPM?Runners)",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_REG_OFFSET,
    0,
    0,
    155,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_Q1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR %i Register",
    "configuration which queue is managed by each of the two runners."
    ""
    "Each register in this array configures 2 queues.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_REG_RAM_CNT,
    4,
    156,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK2_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK3_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK4_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK5_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK6_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_TASK7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK",
#if RU_INCLUDE_DESC
    "PER_Q_TASK Register",
    "which task in the runner to wake-up when requesting a PD for a certain q."
    ""
    "This register holds the task number of the first 8 queues."
    ""
    "For queues 8-40 (if they exist) the task that will be waking is the one appearing in the PD_RNR_CFG regs, depending on which runner this queue is associated with.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_REG_OFFSET,
    0,
    0,
    157,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DMAPTRRST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SDMAPTRRST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_OKFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_STSFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REQFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_MSGFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD",
#if RU_INCLUDE_DESC
    "TX_RESET_COMMAND Register",
    "This register enables reset of internal units (for possible WA purposes).",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REG_OFFSET,
    0,
    0,
    158,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL",
#if RU_INCLUDE_DESC
    "DEBUG_SELECT Register",
    "This register selects 1 of 8 debug vectors."
    "The selected vector is reflected to DBGOUTREG.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_REG_OFFSET,
    0,
    0,
    159,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    160,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR",
#if RU_INCLUDE_DESC
    "GENERAL_PURPOSE_REGISTER Register",
    "general purpose register",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_REG_OFFSET,
    0,
    0,
    161,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE0_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIFOBASE1_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE",
#if RU_INCLUDE_DESC
    "PD_FIFO_BASE Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 256 PDs."
    ""
    "The size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_REG_OFFSET,
    0,
    0,
    162,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE0_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIFOSIZE1_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    0,
    0,
    163,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH0_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_WKUPTHRESH1_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "Each register defines the threshold of 2 queues."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    0,
    0,
    164,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT0_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_PDLIMIT1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration for the rest (TCONTs 8-39)."
    "Each register in this array defines the threshold of 2 queues.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    0,
    0,
    165,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_PDLIMITEN_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET,
    0,
    0,
    166,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues)."
    "This configuration is global for all queues."
    "Relevant only for EPON BBH.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_REG_OFFSET,
    0,
    0,
    167,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_DDRTHRESH_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED0_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_SRAMTHRESH_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH",
#if RU_INCLUDE_DESC
    "TX_THRESHOLD Register",
    "Transmit threshold in 8 bytes resolution."
    "The BBH TX will not start to transmit data towards the XLMAC until the amount of data in the TX FIFO is larger than the threshold or if there is a complete packet in the FIFO.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_REG_OFFSET,
    0,
    0,
    168,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_EN_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE",
#if RU_INCLUDE_DESC
    "EEE Register",
    "The BBH is responsible for indicating the XLMAC that no traffic is about to arrive so the XLMAC may try to enter power saving mode."
    ""
    "This register is used to enable this feature.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_REG_OFFSET,
    0,
    0,
    169,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_EN_FIELD,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "The BBH is responsible for indicating the XLMAC that it should and calculate timestamp for the current packet that is being transmitted. The BBH gets the timestamping parameters in the PD and forward it to the XLMAC."
    ""
    "This register is used to enable this feature.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_REG_OFFSET,
    0,
    0,
    170,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_SRAMPD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD",
#if RU_INCLUDE_DESC
    "SRAM_PD_COUNTER Register",
    "This counter counts the number of received PD for packets to be transmitted from the SRAM."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_REG_OFFSET,
    0,
    0,
    171,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_DDRPD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD",
#if RU_INCLUDE_DESC
    "DDR_PD_COUNTER Register",
    "This counter counts the number of received PDs for packets to be transmitted from the DDR."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_REG_OFFSET,
    0,
    0,
    172,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP",
#if RU_INCLUDE_DESC
    "PD_DROP_COUNTER Register",
    "This counter counts the number of PDs which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_REG_OFFSET,
    0,
    0,
    173,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_STSCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT",
#if RU_INCLUDE_DESC
    "STS_COUNTER Register",
    "This counter counts the number of STS messages which were received from Runner."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_REG_OFFSET,
    0,
    0,
    174,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_STSDROP_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP",
#if RU_INCLUDE_DESC
    "STS_DROP_COUNTER Register",
    "This counter counts the number of STS which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_REG_OFFSET,
    0,
    0,
    175,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_MSGCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT",
#if RU_INCLUDE_DESC
    "MSG_COUNTER Register",
    "This counter counts the number of MSG (DBR/Ghost) messages which were received from Runner."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_REG_OFFSET,
    0,
    0,
    176,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_MSGDROP_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP",
#if RU_INCLUDE_DESC
    "MSG_DROP_COUNTER Register",
    "This counter counts the number of MSG which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_REG_OFFSET,
    0,
    0,
    177,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL",
#if RU_INCLUDE_DESC
    "GET_NEXT_IS_NULL_COUNTER Register",
    "This counter counts the number Get next responses with a null BN."
    "It counts the packets for all TCONTs together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "This counter is relevant for Ethernet only.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_REG_OFFSET,
    0,
    0,
    178,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_FLSHPKTS_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS",
#if RU_INCLUDE_DESC
    "FLUSHED_PACKETS_COUNTER Register",
    "This counter counts the number of packets that were flushed (bn was released without sending the data to the EPON MAC) due to flush request."
    "The counter is global for all queues."
    "The counter is read clear.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_REG_OFFSET,
    0,
    0,
    179,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_LENERR_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR",
#if RU_INCLUDE_DESC
    "REQ_LENGTH_ERROR_COUNTER Register",
    "This counter counts the number of times a length error (mismatch between a request from the MAC and a PD from the Runner) occured."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_REG_OFFSET,
    0,
    0,
    180,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_AGGRLENERR_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR",
#if RU_INCLUDE_DESC
    "AGGREGATION_LENGTH_ERROR_COUNTER Register",
    "This counter Counts aggregation length error events."
    "If one or more of the packets in an aggregated PD is shorter than 60 bytes, this counter will be incremented by 1."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_REG_OFFSET,
    0,
    0,
    181,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_SRAMPKT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT",
#if RU_INCLUDE_DESC
    "SRAM_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the SRAM."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_REG_OFFSET,
    0,
    0,
    182,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT",
#if RU_INCLUDE_DESC
    "DDR_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the DDR."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_REG_OFFSET,
    0,
    0,
    183,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_SRAMBYTE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE",
#if RU_INCLUDE_DESC
    "SRAM_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the SRAM."
    "It counts the bytes for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_REG_OFFSET,
    0,
    0,
    184,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_DDRBYTE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the DDR."
    "It counts the bytes for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_REG_OFFSET,
    0,
    0,
    185,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNVSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNEMPTYSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FBNFULLSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTVSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTYSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULLSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_GPNCNTXTSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_BPMFSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_SBPMFSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSVSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSEMPTYSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFULLSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSBEMPTYSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_STSFFWKPSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MSGVSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_EPNREQSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_DATASEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REORDERSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_TSINFOSEL_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_MACTXSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN",
#if RU_INCLUDE_DESC
    "SW_RD_EN Register",
    "writing to this register creates a rd_en pulse to the selected array the SW wants to access."
    ""
    "Each bit in the register represents one of the arrays the SW can access."
    ""
    "The address inside the array is determined in the previous register (sw_rd_address)."
    ""
    "When writing to this register the SW should assert only one bit. If more than one is asserted, The HW will return the value read from the lsb selected array.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REG_OFFSET,
    0,
    0,
    186,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR",
#if RU_INCLUDE_DESC
    "SW_RD_ADDR Register",
    "the address inside the array the SW wants to read",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_REG_OFFSET,
    0,
    0,
    187,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA",
#if RU_INCLUDE_DESC
    "SW_RD_DATA Register",
    "indirect memories and arrays read data",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_REG_OFFSET,
    0,
    0,
    188,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_DDRBYTE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT",
#if RU_INCLUDE_DESC
    "UNIFIED_PKT_COUNTER %i Register",
    "This counter array counts the number of transmitted packets through each interface in the unified BBH."
    ""
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_REG_RAM_CNT,
    4,
    189,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE",
#if RU_INCLUDE_DESC
    "UNIFIED_BYTE_COUNTER %i Register",
    "This counter array counts the number of transmitted bytes through each interface in the unified BBH."
    ""
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_RAM_CNT,
    4,
    190,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG",
#if RU_INCLUDE_DESC
    "DEBUG_OUT_REG %i Register",
    "an array including all the debug vectors of the BBH TX."
    "entries 30 and 31 are DSL debug.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_REG_RAM_CNT,
    4,
    191,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_FIELDS[] =
{
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION",
#if RU_INCLUDE_DESC
    "IN_SEGMENTATION %i Register",
    "40 bit vector in which each bit represents if the segmentation SM is currently handling a PD of a certain TCONT."
    ""
    "first address is for TCONTS [31:0]"
    "second is for TCONTS [39:32]",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_RAM_CNT,
    4,
    192,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_DEST_FIELD,
    &QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD,
    &QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD,
    &QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD,
    &QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD,
    &QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_REG = 
{
    "DMA_QM_DMA_CONFIG_BBROUTEOVRD",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "Broadbus route override",
#endif
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_REG_OFFSET,
    0,
    0,
    193,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD,
    &QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_REG = 
{
    "DMA_QM_DMA_CONFIG_NUM_OF_WRITES",
#if RU_INCLUDE_DESC
    "NUM_OF_WRITE_REQ %i Register",
    "This array of registers defines the memory allocation for the peripherals, for upstream."
    "The allocation is of number of 128byte buffers out of the total 48 buffers for both sdma and dma."
    ""
    "The allocation is done by defining a only the number of allocated buffers. base address is calculated by HW, when base of peripheral 0 is 0."
    "Note that the memory allocation should not contain wrap around."
    "The number of allocated CDs is the same of data buffers."
    "",
#endif
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_REG_RAM_CNT,
    4,
    194,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_NUM_OF_READS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD,
    &QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_REG = 
{
    "DMA_QM_DMA_CONFIG_NUM_OF_READS",
#if RU_INCLUDE_DESC
    "NUM_OF_READ_REQ %i Register",
    "This array of registers controls the number of read requests of each peripheral within the read requests RAM."
    "total of 64 requests are divided between peripherals."
    "Base address of peripheral 0 is 0, base of peripheral 1 is 0 + periph0_num_of_read_requests and so on.",
#endif
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_REG_RAM_CNT,
    4,
    195,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_U_THRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_U_THRESH_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_U_THRESH_INTO_U_FIELD,
    &QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED0_FIELD,
    &QM_DMA_QM_DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD,
    &QM_DMA_QM_DMA_CONFIG_U_THRESH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_U_THRESH_REG = 
{
    "DMA_QM_DMA_CONFIG_U_THRESH",
#if RU_INCLUDE_DESC
    "URGENT_THRESHOLDS %i Register",
    "the in/out of urgent thresholds mark the number of write requests in the queue in which the peripherals priority is changed. The two thresholds should create hysteresis."
    "The moving into urgent threshold must always be greater than the moving out of urgent threshold.",
#endif
    QM_DMA_QM_DMA_CONFIG_U_THRESH_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_REG_RAM_CNT,
    4,
    196,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_PRI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_PRI_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_PRI_RXPRI_FIELD,
    &QM_DMA_QM_DMA_CONFIG_PRI_TXPRI_FIELD,
    &QM_DMA_QM_DMA_CONFIG_PRI_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_PRI_REG = 
{
    "DMA_QM_DMA_CONFIG_PRI",
#if RU_INCLUDE_DESC
    "STRICT_PRIORITY %i Register",
    "The arbitration between the requests of the different peripherals is done in two stages:"
    "1. Strict priority - chooses the peripherals with the highest priority among all perpherals who have a request pending."
    "2. Weighted Round-Robin between all peripherals with the same priority."
    ""
    "This array of registers allow configuration of the priority of each peripheral (both rx and tx) in the following manner:"
    "There are 4 levels of priorities, when each bit in the register represents a different level of priority. One should assert the relevant bit according to the desired priority -"
    "For the lowest  - 0001"
    "For the highest - 1000",
#endif
    QM_DMA_QM_DMA_CONFIG_PRI_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_PRI_REG_RAM_CNT,
    4,
    197,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_DMA_QM_DMA_CONFIG_PRI_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD,
    &QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD,
    &QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD,
    &QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_REG = 
{
    "DMA_QM_DMA_CONFIG_PERIPH_SOURCE",
#if RU_INCLUDE_DESC
    "BB_SOURCE_DMA_PERIPH %i Register",
    "Broadbus source address of the DMA peripherals. Register per peripheral (rx and tx). The source is used to determine the route address to the different peripherals.",
#endif
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_REG_RAM_CNT,
    4,
    198,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_WEIGHT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_WEIGHT_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD,
    &QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED0_FIELD,
    &QM_DMA_QM_DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD,
    &QM_DMA_QM_DMA_CONFIG_WEIGHT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_WEIGHT_REG = 
{
    "DMA_QM_DMA_CONFIG_WEIGHT",
#if RU_INCLUDE_DESC
    "WEIGHT_OF_ROUND_ROBIN %i Register",
    "The second phase of the arbitration between requests is weighted round robin between requests of peripherals with the same priority."
    "This array of registers allow configurtion of the weight of each peripheral (rx and tx). The actual weight will be weight + 1, meaning configuration of 0 is actual weight of 1.",
#endif
    QM_DMA_QM_DMA_CONFIG_WEIGHT_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_REG_RAM_CNT,
    4,
    199,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_PTRRST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_PTRRST_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_PTRRST_RSTVEC_FIELD,
    &QM_DMA_QM_DMA_CONFIG_PTRRST_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_PTRRST_REG = 
{
    "DMA_QM_DMA_CONFIG_PTRRST",
#if RU_INCLUDE_DESC
    "POINTERS_RESET Register",
    "Resets the pointers of the peripherals FIFOs within the DMA. Bit per peripheral side (rx and tx)."
    "For rx side resets the data and CD FIFOs."
    "For tx side resets the read requests FIFO.",
#endif
    QM_DMA_QM_DMA_CONFIG_PTRRST_REG_OFFSET,
    0,
    0,
    200,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_CONFIG_PTRRST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_MAX_OTF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_MAX_OTF_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_MAX_OTF_MAX_FIELD,
    &QM_DMA_QM_DMA_CONFIG_MAX_OTF_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_MAX_OTF_REG = 
{
    "DMA_QM_DMA_CONFIG_MAX_OTF",
#if RU_INCLUDE_DESC
    "MAX_ON_THE_FLY Register",
    "max number of on the fly read commands the DMA may issue to DDR before receiving any data.",
#endif
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_REG_OFFSET,
    0,
    0,
    201,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_REG = 
{
    "DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    202,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_DBG_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_CONFIG_DBG_SEL_FIELDS[] =
{
    &QM_DMA_QM_DMA_CONFIG_DBG_SEL_DBGSEL_FIELD,
    &QM_DMA_QM_DMA_CONFIG_DBG_SEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_CONFIG_DBG_SEL_REG = 
{
    "DMA_QM_DMA_CONFIG_DBG_SEL",
#if RU_INCLUDE_DESC
    "DBG_SEL Register",
    "debug bus select",
#endif
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_REG_OFFSET,
    0,
    0,
    203,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_NEMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_NEMPTY_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_NEMPTY_NEMPTY_FIELD,
    &QM_DMA_QM_DMA_DEBUG_NEMPTY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_NEMPTY_REG = 
{
    "DMA_QM_DMA_DEBUG_NEMPTY",
#if RU_INCLUDE_DESC
    "NOT_EMPTY_VECTOR Register",
    "Each peripheral is represented in a bit on the not empty vector."
    "LSB is for rx peripherals, MSB for tx peripherals."
    "If the bit is asserted, the requests queue of the relevant peripheral is not empty."
    "The not empty vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    QM_DMA_QM_DMA_DEBUG_NEMPTY_REG_OFFSET,
    0,
    0,
    204,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_DEBUG_NEMPTY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_URGNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_URGNT_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_URGNT_URGNT_FIELD,
    &QM_DMA_QM_DMA_DEBUG_URGNT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_URGNT_REG = 
{
    "DMA_QM_DMA_DEBUG_URGNT",
#if RU_INCLUDE_DESC
    "URGENT_VECTOR Register",
    "Each peripheral, a is represented in a bit on the urgent vector. 8 LSB are rx peripherlas, 8 MSB are tx peripherals."
    "If the bit is asserted, the requests queue of the relevant peripheral is in urgent state."
    "The urgent vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    QM_DMA_QM_DMA_DEBUG_URGNT_REG_OFFSET,
    0,
    0,
    205,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_DEBUG_URGNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_SELSRC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_SELSRC_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_SELSRC_SEL_SRC_FIELD,
    &QM_DMA_QM_DMA_DEBUG_SELSRC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_SELSRC_REG = 
{
    "DMA_QM_DMA_DEBUG_SELSRC",
#if RU_INCLUDE_DESC
    "SELECTED_SOURCE_NUM Register",
    "The decision of the dma schedule rand the next peripheral to be served, represented by its source address",
#endif
    QM_DMA_QM_DMA_DEBUG_SELSRC_REG_OFFSET,
    0,
    0,
    206,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_DEBUG_SELSRC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REG = 
{
    "DMA_QM_DMA_DEBUG_REQ_CNT_RX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_RX %i Register",
    "the number of write requests currently pending for each rx peripheral.",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REG_RAM_CNT,
    4,
    207,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REG = 
{
    "DMA_QM_DMA_DEBUG_REQ_CNT_TX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_TX %i Register",
    "the number of read requestscurrently pending for each TX peripheral.",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REG_RAM_CNT,
    4,
    208,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REG = 
{
    "DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_RX %i Register",
    "the accumulated number of write requests served so far for each peripheral. Wrap around on max value, not read clear.",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REG_RAM_CNT,
    4,
    209,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REG = 
{
    "DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_TX %i Register",
    "the accumulated number of read requests served so far for each peripheral. Wrap around on max value, not read clear.",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REG_RAM_CNT,
    4,
    210,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_RDADD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_RDADD_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_RDADD_ADDRESS_FIELD,
    &QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED0_FIELD,
    &QM_DMA_QM_DMA_DEBUG_RDADD_DATACS_FIELD,
    &QM_DMA_QM_DMA_DEBUG_RDADD_CDCS_FIELD,
    &QM_DMA_QM_DMA_DEBUG_RDADD_RRCS_FIELD,
    &QM_DMA_QM_DMA_DEBUG_RDADD_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_RDADD_REG = 
{
    "DMA_QM_DMA_DEBUG_RDADD",
#if RU_INCLUDE_DESC
    "RAM_ADDRES Register",
    "the address and cs of the ram the user wishes to read using the indirect access read mechanism.",
#endif
    QM_DMA_QM_DMA_DEBUG_RDADD_REG_OFFSET,
    0,
    0,
    211,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    QM_DMA_QM_DMA_DEBUG_RDADD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_RDVALID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_RDVALID_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_RDVALID_VALID_FIELD,
    &QM_DMA_QM_DMA_DEBUG_RDVALID_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_RDVALID_REG = 
{
    "DMA_QM_DMA_DEBUG_RDVALID",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_REQUEST_VALID Register",
    "After determining the address and cs, the user should assert this bit for indicating that the address and cs are valid.",
#endif
    QM_DMA_QM_DMA_DEBUG_RDVALID_REG_OFFSET,
    0,
    0,
    212,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_DEBUG_RDVALID_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_RDDATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_RDDATA_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_RDDATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_RDDATA_REG = 
{
    "DMA_QM_DMA_DEBUG_RDDATA",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_DATA %i Register",
    "The returned read data from the selected RAM. Array of 4 registers (128 bits total)."
    "The width of the different memories is as follows:"
    "write data - 128 bits"
    "chunk descriptors - 36 bits"
    "read requests - 42 bits"
    "read data - 64 bits"
    ""
    "The the memories with width smaller than 128, the data will appear in the first registers of the array, for example:"
    "data from the cd RAM will appear in - {reg1[5:0], reg0[31:0]}.",
#endif
    QM_DMA_QM_DMA_DEBUG_RDDATA_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_RDDATA_REG_RAM_CNT,
    4,
    213,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DMA_QM_DMA_DEBUG_RDDATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_RDDATARDY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DMA_QM_DMA_DEBUG_RDDATARDY_FIELDS[] =
{
    &QM_DMA_QM_DMA_DEBUG_RDDATARDY_READY_FIELD,
    &QM_DMA_QM_DMA_DEBUG_RDDATARDY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_DMA_QM_DMA_DEBUG_RDDATARDY_REG = 
{
    "DMA_QM_DMA_DEBUG_RDDATARDY",
#if RU_INCLUDE_DESC
    "READ_DATA_READY Register",
    "When assertd indicats that the data in the previous array is valid.Willremain asserted until the user deasserts the valid bit in regiser RDVALID.",
#endif
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_REG_OFFSET,
    0,
    0,
    214,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: QM
 ******************************************************************************/
static const ru_reg_rec *QM_REGS[] =
{
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_REG,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_REG,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_REG,
    &QM_GLOBAL_CFG_FPM_CONTROL_REG,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_REG,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_REG,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_REG,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_REG,
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_REG,
    &QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_REG,
    &QM_GLOBAL_CFG_ABS_DROP_QUEUE_REG,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_REG,
    &QM_GLOBAL_CFG_FPM_BASE_ADDR_REG,
    &QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG,
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG,
    &QM_GLOBAL_CFG_DQM_FULL_REG,
    &QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG,
    &QM_GLOBAL_CFG_DQM_POP_READY_REG,
    &QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG,
    &QM_GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES_REG,
    &QM_GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR_REG,
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG_REG,
    &QM_GLOBAL_CFG_QM_FLOW_CTRL_INTR_REG,
    &QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG,
    &QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG,
    &QM_FPM_POOLS_THR_REG,
    &QM_FPM_USR_GRP_LOWER_THR_REG,
    &QM_FPM_USR_GRP_MID_THR_REG,
    &QM_FPM_USR_GRP_HIGHER_THR_REG,
    &QM_FPM_USR_GRP_CNT_REG,
    &QM_RUNNER_GRP_RNR_CONFIG_REG,
    &QM_RUNNER_GRP_QUEUE_CONFIG_REG,
    &QM_RUNNER_GRP_PDFIFO_CONFIG_REG,
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG,
    &QM_INTR_CTRL_ISR_REG,
    &QM_INTR_CTRL_ISM_REG,
    &QM_INTR_CTRL_IER_REG,
    &QM_INTR_CTRL_ITR_REG,
    &QM_CLK_GATE_CLK_GATE_CNTRL_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG,
    &QM_QUEUE_CONTEXT_CONTEXT_REG,
    &QM_WRED_PROFILE_COLOR_MIN_THR_0_REG,
    &QM_WRED_PROFILE_COLOR_MIN_THR_1_REG,
    &QM_WRED_PROFILE_COLOR_MAX_THR_0_REG,
    &QM_WRED_PROFILE_COLOR_MAX_THR_1_REG,
    &QM_WRED_PROFILE_COLOR_SLOPE_0_REG,
    &QM_WRED_PROFILE_COLOR_SLOPE_1_REG,
    &QM_COPY_DECISION_PROFILE_THR_REG,
    &QM_TOTAL_VALID_COUNTER_COUNTER_REG,
    &QM_DQM_VALID_COUNTER_COUNTER_REG,
    &QM_DROP_COUNTER_COUNTER_REG,
    &QM_EPON_RPT_CNT_COUNTER_REG,
    &QM_EPON_RPT_CNT_QUEUE_STATUS_REG,
    &QM_RD_DATA_POOL0_REG,
    &QM_RD_DATA_POOL1_REG,
    &QM_RD_DATA_POOL2_REG,
    &QM_RD_DATA_POOL3_REG,
    &QM_PDFIFO_PTR_REG,
    &QM_UPDATE_FIFO_PTR_REG,
    &QM_RD_DATA_REG,
    &QM_POP_REG,
    &QM_CM_COMMON_INPUT_FIFO_DATA_REG,
    &QM_NORMAL_RMT_FIFO_DATA_REG,
    &QM_NON_DELAYED_RMT_FIFO_DATA_REG,
    &QM_EGRESS_DATA_FIFO_DATA_REG,
    &QM_EGRESS_RR_FIFO_DATA_REG,
    &QM_EGRESS_BB_INPUT_FIFO_DATA_REG,
    &QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG,
    &QM_BB_OUTPUT_FIFO_DATA_REG,
    &QM_NON_DELAYED_OUT_FIFO_DATA_REG,
    &QM_CONTEXT_DATA_REG,
    &QM_DEBUG_SEL_REG,
    &QM_DEBUG_BUS_LSB_REG,
    &QM_DEBUG_BUS_MSB_REG,
    &QM_QM_SPARE_CONFIG_REG,
    &QM_GOOD_LVL1_PKTS_CNT_REG,
    &QM_GOOD_LVL1_BYTES_CNT_REG,
    &QM_GOOD_LVL2_PKTS_CNT_REG,
    &QM_GOOD_LVL2_BYTES_CNT_REG,
    &QM_COPIED_PKTS_CNT_REG,
    &QM_COPIED_BYTES_CNT_REG,
    &QM_AGG_PKTS_CNT_REG,
    &QM_AGG_BYTES_CNT_REG,
    &QM_AGG_1_PKTS_CNT_REG,
    &QM_AGG_2_PKTS_CNT_REG,
    &QM_AGG_3_PKTS_CNT_REG,
    &QM_AGG_4_PKTS_CNT_REG,
    &QM_WRED_DROP_CNT_REG,
    &QM_FPM_CONGESTION_DROP_CNT_REG,
    &QM_DDR_PD_CONGESTION_DROP_CNT_REG,
    &QM_DDR_BYTE_CONGESTION_DROP_CNT_REG,
    &QM_QM_PD_CONGESTION_DROP_CNT_REG,
    &QM_QM_ABS_REQUEUE_CNT_REG,
    &QM_FPM_PREFETCH_FIFO0_STATUS_REG,
    &QM_FPM_PREFETCH_FIFO1_STATUS_REG,
    &QM_FPM_PREFETCH_FIFO2_STATUS_REG,
    &QM_FPM_PREFETCH_FIFO3_STATUS_REG,
    &QM_NORMAL_RMT_FIFO_STATUS_REG,
    &QM_NON_DELAYED_RMT_FIFO_STATUS_REG,
    &QM_NON_DELAYED_OUT_FIFO_STATUS_REG,
    &QM_PRE_CM_FIFO_STATUS_REG,
    &QM_CM_RD_PD_FIFO_STATUS_REG,
    &QM_CM_WR_PD_FIFO_STATUS_REG,
    &QM_CM_COMMON_INPUT_FIFO_STATUS_REG,
    &QM_BB0_OUTPUT_FIFO_STATUS_REG,
    &QM_BB1_OUTPUT_FIFO_STATUS_REG,
    &QM_BB1_INPUT_FIFO_STATUS_REG,
    &QM_EGRESS_DATA_FIFO_STATUS_REG,
    &QM_EGRESS_RR_FIFO_STATUS_REG,
    &QM_BB_ROUTE_OVR_REG,
    &QM_QM_INGRESS_STAT_REG,
    &QM_QM_EGRESS_STAT_REG,
    &QM_QM_CM_STAT_REG,
    &QM_QM_FPM_PREFETCH_STAT_REG,
    &QM_QM_CONNECT_ACK_COUNTER_REG,
    &QM_QM_DDR_WR_REPLY_COUNTER_REG,
    &QM_QM_DDR_PIPE_BYTE_COUNTER_REG,
    &QM_QM_ABS_REQUEUE_VALID_COUNTER_REG,
    &QM_QM_ILLEGAL_PD_CAPTURE_REG,
    &QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG,
    &QM_QM_CM_PROCESSED_PD_CAPTURE_REG,
    &QM_FPM_POOL_DROP_CNT_REG,
    &QM_FPM_GRP_DROP_CNT_REG,
    &QM_FPM_BUFFER_RES_DROP_CNT_REG,
    &QM_PSRAM_EGRESS_CONG_DRP_CNT_REG,
    &QM_DATA_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_REG,
    &QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_REG,
    &QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_REG,
    &QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_REG,
    &QM_DMA_QM_DMA_CONFIG_U_THRESH_REG,
    &QM_DMA_QM_DMA_CONFIG_PRI_REG,
    &QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_REG,
    &QM_DMA_QM_DMA_CONFIG_WEIGHT_REG,
    &QM_DMA_QM_DMA_CONFIG_PTRRST_REG,
    &QM_DMA_QM_DMA_CONFIG_MAX_OTF_REG,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_REG,
    &QM_DMA_QM_DMA_CONFIG_DBG_SEL_REG,
    &QM_DMA_QM_DMA_DEBUG_NEMPTY_REG,
    &QM_DMA_QM_DMA_DEBUG_URGNT_REG,
    &QM_DMA_QM_DMA_DEBUG_SELSRC_REG,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REG,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REG,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REG,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REG,
    &QM_DMA_QM_DMA_DEBUG_RDADD_REG,
    &QM_DMA_QM_DMA_DEBUG_RDVALID_REG,
    &QM_DMA_QM_DMA_DEBUG_RDDATA_REG,
    &QM_DMA_QM_DMA_DEBUG_RDDATARDY_REG,
};

unsigned long QM_ADDRS[] =
{
    0x82100000,
};

const ru_block_rec QM_BLOCK = 
{
    "QM",
    QM_ADDRS,
    1,
    215,
    QM_REGS
};

/* End of file XRDP_QM.c */
