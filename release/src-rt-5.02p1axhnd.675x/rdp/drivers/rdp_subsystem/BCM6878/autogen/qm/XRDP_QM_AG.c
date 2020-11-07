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
    "6878:"
    "1 for 32B/Queue"
    "0 debug - no residue"
    ""
    "Other projects:"
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
 * Field: QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD =
{
    "DROP_CNT_WRED_DROPS",
#if RU_INCLUDE_DESC
    "DROP_CNT_WRED_DROPS",
    "Drop counter counts WRED drops by color per queue."
    "In order to enable this feature the drop counter should be configured to count drops. if the drop counter is configured count max occupancy per queue, it will override WRED drops count."
    "color 0 - is written in dropped bytes field (word0)"
    "color 1 - is written in dropped pkts field (word1)",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD_SHIFT,
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
 * Field: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD =
{
    "PD_OCCUPANCY_EN",
#if RU_INCLUDE_DESC
    "PD_OCCUPANCY_EN",
    "If set, aggregation of queues with PD occupancy more than encoded on PD_OCCUPANCY_VALUE arent closed even if the timer is expired."
    "If not set, then aggregation is closed after queues timer expires",
#endif
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD =
{
    "PD_OCCUPANCY_VALUE",
#if RU_INCLUDE_DESC
    "PD_OCCUPANCY_VALUE",
    "if PD_OCCUPANCY_EN == 1 then"
    "Aggregations of queues with more than byte_occupacny of (PD_OCCUPNACY > 0) ?2 ^ (PD_OCCUPANCY + 5):0 are not closed on timeout.",
#endif
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD_MASK,
    0,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD_WIDTH,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD_SHIFT,
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
 * Field: QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO
 ******************************************************************************/
const ru_field_rec QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD =
{
    "QM_COPY_PLEN_ZERO",
#if RU_INCLUDE_DESC
    "QM_COPY_PLEN_ZERO",
    "Packet with length = 0 is copied to DDR. FW/SW should take care that zero length packets arent copied to DDR."
    "",
#endif
    QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD_MASK,
    0,
    QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD_WIDTH,
    QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD_SHIFT,
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
    ru_access_rw
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
    ru_access_rw
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
 * Field: QM_FPM_BUFFER_RESERVATION_DATA_DATA
 ******************************************************************************/
const ru_field_rec QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "DATA",
#endif
    QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD_MASK,
    0,
    QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD_WIDTH,
    QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FPM_BUFFER_RESERVATION_DATA_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FPM_BUFFER_RESERVATION_DATA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FPM_BUFFER_RESERVATION_DATA_RESERVED0_FIELD_MASK,
    0,
    QM_FPM_BUFFER_RESERVATION_DATA_RESERVED0_FIELD_WIDTH,
    QM_FPM_BUFFER_RESERVATION_DATA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG0_EN
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG0_EN_FIELD =
{
    "FLOW_CTRL_UG0_EN",
#if RU_INCLUDE_DESC
    "FLOW_CTRL_UG0_EN",
    "Flow control enable",
#endif
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG0_EN_FIELD_MASK,
    0,
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG0_EN_FIELD_WIDTH,
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG0_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG1_EN
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG1_EN_FIELD =
{
    "FLOW_CTRL_UG1_EN",
#if RU_INCLUDE_DESC
    "FLOW_CTRL_UG1_EN",
    "Flow control enable",
#endif
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG1_EN_FIELD_MASK,
    0,
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG1_EN_FIELD_WIDTH,
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG1_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG2_EN
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG2_EN_FIELD =
{
    "FLOW_CTRL_UG2_EN",
#if RU_INCLUDE_DESC
    "FLOW_CTRL_UG2_EN",
    "Flow control enable",
#endif
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG2_EN_FIELD_MASK,
    0,
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG2_EN_FIELD_WIDTH,
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG2_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG3_EN
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG3_EN_FIELD =
{
    "FLOW_CTRL_UG3_EN",
#if RU_INCLUDE_DESC
    "FLOW_CTRL_UG3_EN",
    "Flow control enable",
#endif
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG3_EN_FIELD_MASK,
    0,
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG3_EN_FIELD_WIDTH,
    QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG3_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_UG_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_UG_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FLOW_CTRL_UG_CTRL_RESERVED0_FIELD_MASK,
    0,
    QM_FLOW_CTRL_UG_CTRL_RESERVED0_FIELD_WIDTH,
    QM_FLOW_CTRL_UG_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_STATUS_UG0
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_STATUS_UG0_FIELD =
{
    "UG0",
#if RU_INCLUDE_DESC
    "UG0",
    "User group 0 status",
#endif
    QM_FLOW_CTRL_STATUS_UG0_FIELD_MASK,
    0,
    QM_FLOW_CTRL_STATUS_UG0_FIELD_WIDTH,
    QM_FLOW_CTRL_STATUS_UG0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_STATUS_UG1
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_STATUS_UG1_FIELD =
{
    "UG1",
#if RU_INCLUDE_DESC
    "UG1",
    "User group 1 status",
#endif
    QM_FLOW_CTRL_STATUS_UG1_FIELD_MASK,
    0,
    QM_FLOW_CTRL_STATUS_UG1_FIELD_WIDTH,
    QM_FLOW_CTRL_STATUS_UG1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_STATUS_UG2
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_STATUS_UG2_FIELD =
{
    "UG2",
#if RU_INCLUDE_DESC
    "UG2",
    "User group 2 status",
#endif
    QM_FLOW_CTRL_STATUS_UG2_FIELD_MASK,
    0,
    QM_FLOW_CTRL_STATUS_UG2_FIELD_WIDTH,
    QM_FLOW_CTRL_STATUS_UG2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_STATUS_UG3
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_STATUS_UG3_FIELD =
{
    "UG3",
#if RU_INCLUDE_DESC
    "UG3",
    "User group 3 status",
#endif
    QM_FLOW_CTRL_STATUS_UG3_FIELD_MASK,
    0,
    QM_FLOW_CTRL_STATUS_UG3_FIELD_WIDTH,
    QM_FLOW_CTRL_STATUS_UG3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_STATUS_WRED
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_STATUS_WRED_FIELD =
{
    "WRED",
#if RU_INCLUDE_DESC
    "WRED",
    "OR on all wred flow control queues",
#endif
    QM_FLOW_CTRL_STATUS_WRED_FIELD_MASK,
    0,
    QM_FLOW_CTRL_STATUS_WRED_FIELD_WIDTH,
    QM_FLOW_CTRL_STATUS_WRED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_STATUS_R0
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_STATUS_R0_FIELD =
{
    "R0",
#if RU_INCLUDE_DESC
    "R0",
    "reserved",
#endif
    QM_FLOW_CTRL_STATUS_R0_FIELD_MASK,
    0,
    QM_FLOW_CTRL_STATUS_R0_FIELD_WIDTH,
    QM_FLOW_CTRL_STATUS_R0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_WRED_SOURCE_SRC
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_WRED_SOURCE_SRC_FIELD =
{
    "SRC",
#if RU_INCLUDE_DESC
    "SRC",
    "each bit represents queue",
#endif
    QM_FLOW_CTRL_WRED_SOURCE_SRC_FIELD_MASK,
    0,
    QM_FLOW_CTRL_WRED_SOURCE_SRC_FIELD_WIDTH,
    QM_FLOW_CTRL_WRED_SOURCE_SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD =
{
    "RNR_BB_ID",
#if RU_INCLUDE_DESC
    "RNR_BB_ID",
    "Runner BB ID",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD_MASK,
    0,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD_WIDTH,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD_MASK,
    0,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD_WIDTH,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_TASK
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD =
{
    "RNR_TASK",
#if RU_INCLUDE_DESC
    "RNR_TASK",
    "Runner task",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD_MASK,
    0,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD_WIDTH,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD_MASK,
    0,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD_WIDTH,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD =
{
    "RNR_ENABLE",
#if RU_INCLUDE_DESC
    "RNR_ENABLE",
    "Runner enable."
    "if disable, the lossless flow control is disabled."
    ""
    "",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD_MASK,
    0,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD_WIDTH,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_EN
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_EN_FIELD =
{
    "SRAM_WR_EN",
#if RU_INCLUDE_DESC
    "SRAM_WR_EN",
    "If set, the wake up messages data is written to SRAM_ADDR",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_EN_FIELD_MASK,
    0,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_EN_FIELD_WIDTH,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_ADDR
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_ADDR_FIELD =
{
    "SRAM_WR_ADDR",
#if RU_INCLUDE_DESC
    "SRAM_WR_ADDR",
    "Sram address to write the vector",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_ADDR_FIELD_MASK,
    0,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_ADDR_FIELD_WIDTH,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED2
 ******************************************************************************/
const ru_field_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD_MASK,
    0,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD_WIDTH,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
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
 * Field: QM_BACKPRESSURE_COUNTER
 ******************************************************************************/
const ru_field_rec QM_BACKPRESSURE_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER",
    "Counter",
#endif
    QM_BACKPRESSURE_COUNTER_FIELD_MASK,
    0,
    QM_BACKPRESSURE_COUNTER_FIELD_WIDTH,
    QM_BACKPRESSURE_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_ADDR
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "ADDR",
#endif
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_ADDR_FIELD_MASK,
    0,
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_ADDR_FIELD_WIDTH,
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD =
{
    "BBHTX_REQ_OTF",
#if RU_INCLUDE_DESC
    "BBHTX_REQ_OTF",
    "BBHTX_REQ_OTF",
#endif
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD_MASK,
    0,
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD_WIDTH,
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_RESERVED0
 ******************************************************************************/
const ru_field_rec QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_RESERVED0_FIELD_MASK,
    0,
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_RESERVED0_FIELD_WIDTH,
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
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
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD,
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
    30,
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
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_RESERVED0_FIELD,
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
    5,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_FIELDS
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
    21,
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
    22,
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
    23,
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
    24,
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
    "Holds FPM user group middle threshold."
    "*IMPORTANT* if buffer reservations is enabled, the following should be honored:"
    "HIGHER_THR-MID_THR > 16",
#endif
    QM_FPM_USR_GRP_MID_THR_REG_OFFSET,
    QM_FPM_USR_GRP_MID_THR_REG_RAM_CNT,
    32,
    25,
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
    "Holds FPM user group higher threshold."
    "*IMPORTANT* if buffer reservations is enabled, the following should be honored:"
    "HIGHER_THR-MID_THR > 16",
#endif
    QM_FPM_USR_GRP_HIGHER_THR_REG_OFFSET,
    QM_FPM_USR_GRP_HIGHER_THR_REG_RAM_CNT,
    32,
    26,
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
    27,
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
    28,
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
    29,
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
    30,
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
    31,
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
    &QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD,
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
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    24,
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
    33,
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
    34,
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
    35,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
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
    36,
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
    37,
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
    38,
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
    39,
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
    40,
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
    41,
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
    42,
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
    43,
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
    44,
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
    45,
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
    46,
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
    47,
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
    48,
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
    49,
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
    50,
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
    51,
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
    52,
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
    53,
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
    "word0:{15`b0,pkt_cnt[16:0]}"
    "word1:{2`b0,byte_cnt[29:0]}"
    "word2:{15b0,res_cnt[16:0]}"
    "word3: reserved"
    ""
    "There are three words per queue starting at queue0 up to queue 159/287.",
#endif
    QM_TOTAL_VALID_COUNTER_COUNTER_REG_OFFSET,
    QM_TOTAL_VALID_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    54,
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
    55,
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
    "in WRED drop mode:"
    "word0 - color1 dropped packets"
    "word1 - color0 dropped packets"
    ""
    ""
    "There are two words per queue starting at queue0 up to queue 287.",
#endif
    QM_DROP_COUNTER_COUNTER_REG_OFFSET,
    QM_DROP_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    56,
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
    57,
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
    58,
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
    59,
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
    60,
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
    61,
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
    62,
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
    63,
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
    64,
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
    65,
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
    66,
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
    67,
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
    68,
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
    69,
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
    70,
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
    71,
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
    72,
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
    73,
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
    74,
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
    75,
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
    76,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CONTEXT_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FPM_BUFFER_RESERVATION_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_BUFFER_RESERVATION_DATA_FIELDS[] =
{
    &QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD,
    &QM_FPM_BUFFER_RESERVATION_DATA_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FPM_BUFFER_RESERVATION_DATA_REG = 
{
    "FPM_BUFFER_RESERVATION_DATA",
#if RU_INCLUDE_DESC
    "PROFILE %i Register",
    "Reserved FPM buffers in units of min. FPM buffer."
    "entry0 -> profile0"
    "..."
    "entry7 -> profile7",
#endif
    QM_FPM_BUFFER_RESERVATION_DATA_REG_OFFSET,
    QM_FPM_BUFFER_RESERVATION_DATA_REG_RAM_CNT,
    4,
    77,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_FPM_BUFFER_RESERVATION_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FLOW_CTRL_UG_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FLOW_CTRL_UG_CTRL_FIELDS[] =
{
    &QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG0_EN_FIELD,
    &QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG1_EN_FIELD,
    &QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG2_EN_FIELD,
    &QM_FLOW_CTRL_UG_CTRL_FLOW_CTRL_UG3_EN_FIELD,
    &QM_FLOW_CTRL_UG_CTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FLOW_CTRL_UG_CTRL_REG = 
{
    "FLOW_CTRL_UG_CTRL",
#if RU_INCLUDE_DESC
    "UG_CTRL Register",
    "FPM user group ctrl",
#endif
    QM_FLOW_CTRL_UG_CTRL_REG_OFFSET,
    0,
    0,
    78,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    QM_FLOW_CTRL_UG_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FLOW_CTRL_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FLOW_CTRL_STATUS_FIELDS[] =
{
    &QM_FLOW_CTRL_STATUS_UG0_FIELD,
    &QM_FLOW_CTRL_STATUS_UG1_FIELD,
    &QM_FLOW_CTRL_STATUS_UG2_FIELD,
    &QM_FLOW_CTRL_STATUS_UG3_FIELD,
    &QM_FLOW_CTRL_STATUS_WRED_FIELD,
    &QM_FLOW_CTRL_STATUS_R0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FLOW_CTRL_STATUS_REG = 
{
    "FLOW_CTRL_STATUS",
#if RU_INCLUDE_DESC
    "STATUS Register",
    "Keeps status vector of user group + wred are under flow control."
    "3:0 - {ug3,ug2,ug1,ug0}, 4 - OR on wred_source"
    ""
    "for UG -"
    "queues which passed the mid. thr. is set to 1. the user groups indication is de-asserted when the occupancy reaches the low thr."
    "4bits - bit for each user group."
    ""
    "for WRED/occupancy -"
    "If one of the queues which pass color1 min occupancy marked as 1. when the occupancy is reduced to below than color0 min occupancy in all queues the bit is set to 0."
    "FW can set/reset the value, it will updated when flow control which is relevant to the corresponding bit takes place.",
#endif
    QM_FLOW_CTRL_STATUS_REG_OFFSET,
    0,
    0,
    79,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    QM_FLOW_CTRL_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FLOW_CTRL_WRED_SOURCE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FLOW_CTRL_WRED_SOURCE_FIELDS[] =
{
    &QM_FLOW_CTRL_WRED_SOURCE_SRC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FLOW_CTRL_WRED_SOURCE_REG = 
{
    "FLOW_CTRL_WRED_SOURCE",
#if RU_INCLUDE_DESC
    "WRED_SOURCE %i Register",
    "Keeps status vector of queues which are under flow control:"
    "queues which passed the color1 low threshold is set to 1. the queue indication is de-asserted when the queue byte occupancy reaches the color0 low threshold."
    "320bits - bit for each queue number (up to 320 queues).",
#endif
    QM_FLOW_CTRL_WRED_SOURCE_REG_OFFSET,
    QM_FLOW_CTRL_WRED_SOURCE_REG_RAM_CNT,
    4,
    80,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FLOW_CTRL_WRED_SOURCE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_FIELDS[] =
{
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_BB_ID_FIELD,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED0_FIELD,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_TASK_FIELD,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED1_FIELD,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RNR_ENABLE_FIELD,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_EN_FIELD,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_SRAM_WR_ADDR_FIELD,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_REG = 
{
    "FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG",
#if RU_INCLUDE_DESC
    "QM_FLOW_CTRL_RNR_CFG Register",
    "lossless flow control configuration"
    ""
    "",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_REG_OFFSET,
    0,
    0,
    81,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_FIELDS
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
    82,
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
    83,
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
    84,
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
    85,
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
    86,
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
    87,
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
    88,
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
    89,
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
    90,
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
    91,
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
    92,
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
    93,
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
    94,
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
    95,
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
    96,
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
    97,
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
    98,
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
    99,
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
    100,
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
    101,
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
    102,
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
    103,
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
    104,
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
    105,
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
    106,
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
    107,
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
    108,
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
    109,
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
    110,
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
    111,
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
    112,
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
    113,
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
    114,
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
    115,
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
    116,
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
    117,
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
    118,
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
    119,
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
    120,
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
    121,
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
    122,
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
    123,
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
    124,
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
    125,
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
    126,
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
    127,
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
    128,
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
    129,
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
    130,
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
    131,
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
    132,
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
    133,
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
    134,
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
    135,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_PSRAM_EGRESS_CONG_DRP_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_BACKPRESSURE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BACKPRESSURE_FIELDS[] =
{
    &QM_BACKPRESSURE_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_BACKPRESSURE_REG = 
{
    "BACKPRESSURE",
#if RU_INCLUDE_DESC
    "BACKPRESSURE Register",
    "Back pressure sets the relevant register per bp reason. SW should unset.",
#endif
    QM_BACKPRESSURE_REG_OFFSET,
    0,
    0,
    136,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BACKPRESSURE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_FIELDS[] =
{
    &QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_ADDR_FIELD,
    &QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD,
    &QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_REG = 
{
    "GLOBAL_CFG2_BBHTX_FIFO_ADDR",
#if RU_INCLUDE_DESC
    "BBHTX_FIFO_ADDR Register",
    "QMs BBH TX FIFO address."
    "bit 10:5 of BBs target address to QM. relevant only for project where the BBHTX_SDMA are external to QM",
#endif
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_REG_OFFSET,
    0,
    0,
    137,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_FIELDS
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
    138,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DATA_FIELDS
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
    &QM_FPM_BUFFER_RESERVATION_DATA_REG,
    &QM_FLOW_CTRL_UG_CTRL_REG,
    &QM_FLOW_CTRL_STATUS_REG,
    &QM_FLOW_CTRL_WRED_SOURCE_REG,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_REG,
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
    &QM_BACKPRESSURE_REG,
    &QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_REG,
    &QM_DATA_REG,
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
    139,
    QM_REGS
};

/* End of file XRDP_QM.c */
