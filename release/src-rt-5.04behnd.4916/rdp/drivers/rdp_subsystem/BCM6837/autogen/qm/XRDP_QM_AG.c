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


#include "XRDP_QM_AG.h"

/******************************************************************************
 * Register: NAME: QM_DATA, TYPE: Type_QM_TOP_QM_CM_RESIDUE_MEM_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_DATA_DATA_FIELD_MASK },
    0,
    { QM_DATA_DATA_FIELD_WIDTH },
    { QM_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DATA_FIELDS[] =
{
    &QM_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DATA *****/
const ru_reg_rec QM_DATA_REG =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA 0..1023 Register",
    "CM Residue - debug access\n",
#endif
    { QM_DATA_REG_OFFSET },
    QM_DATA_REG_RAM_CNT,
    4,
    674,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CONTEXT_DATA, TYPE: Type_QM_TOP_QM_AGGREGATION_CONTEXT_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CONTEXT_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_CONTEXT_DATA_DATA_FIELD_MASK },
    0,
    { QM_CONTEXT_DATA_DATA_FIELD_WIDTH },
    { QM_CONTEXT_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CONTEXT_DATA_FIELDS[] =
{
    &QM_CONTEXT_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CONTEXT_DATA *****/
const ru_reg_rec QM_CONTEXT_DATA_REG =
{
    "CONTEXT_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..639 Register",
    "Aggregation context - debug access\n4 words per queue --> 160x4\nnot designed to be accessed on the fly,\nshould not be read during traffic/timers_en\n",
#endif
    { QM_CONTEXT_DATA_REG_OFFSET },
    QM_CONTEXT_DATA_REG_RAM_CNT,
    4,
    675,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CONTEXT_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_BUFFER_RESERVATION_DATA, TYPE: Type_QM_TOP_QM_FPM_BUFFER_RESERVATION_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD_MASK },
    0,
    { QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD_WIDTH },
    { QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_BUFFER_RESERVATION_DATA_FIELDS[] =
{
    &QM_FPM_BUFFER_RESERVATION_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_BUFFER_RESERVATION_DATA *****/
const ru_reg_rec QM_FPM_BUFFER_RESERVATION_DATA_REG =
{
    "FPM_BUFFER_RESERVATION_DATA",
#if RU_INCLUDE_DESC
    "PROFILE 0..7 Register",
    "Reserved FPM buffers in units of min. FPM buffer.\nentry0 -> profile0\n...\nentry7 -> profile7\n",
#endif
    { QM_FPM_BUFFER_RESERVATION_DATA_REG_OFFSET },
    QM_FPM_BUFFER_RESERVATION_DATA_REG_RAM_CNT,
    4,
    676,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_BUFFER_RESERVATION_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_PORT_CFG, TYPE: Type_QM_TOP_QM_FLOW_CTRL_PORT_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_BYTE *****/
const ru_field_rec QM_PORT_CFG_EN_BYTE_FIELD =
{
    "EN_BYTE",
#if RU_INCLUDE_DESC
    "",
    "Enable flow control on byte occupancy\n",
#endif
    { QM_PORT_CFG_EN_BYTE_FIELD_MASK },
    0,
    { QM_PORT_CFG_EN_BYTE_FIELD_WIDTH },
    { QM_PORT_CFG_EN_BYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_UG *****/
const ru_field_rec QM_PORT_CFG_EN_UG_FIELD =
{
    "EN_UG",
#if RU_INCLUDE_DESC
    "",
    "Enable flow control according to byte occupancy\n",
#endif
    { QM_PORT_CFG_EN_UG_FIELD_MASK },
    0,
    { QM_PORT_CFG_EN_UG_FIELD_WIDTH },
    { QM_PORT_CFG_EN_UG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBH_RX_BB_ID *****/
const ru_field_rec QM_PORT_CFG_BBH_RX_BB_ID_FIELD =
{
    "BBH_RX_BB_ID",
#if RU_INCLUDE_DESC
    "",
    "BB ID to which Xoff/Xon is sent.\nDesign assumption:\n",
#endif
    { QM_PORT_CFG_BBH_RX_BB_ID_FIELD_MASK },
    0,
    { QM_PORT_CFG_BBH_RX_BB_ID_FIELD_WIDTH },
    { QM_PORT_CFG_BBH_RX_BB_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FW_PORT_ID *****/
const ru_field_rec QM_PORT_CFG_FW_PORT_ID_FIELD =
{
    "FW_PORT_ID",
#if RU_INCLUDE_DESC
    "",
    "FW_ID compared to PD to decide whether to send Xoff/Xon.\n",
#endif
    { QM_PORT_CFG_FW_PORT_ID_FIELD_MASK },
    0,
    { QM_PORT_CFG_FW_PORT_ID_FIELD_WIDTH },
    { QM_PORT_CFG_FW_PORT_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_PORT_CFG_FIELDS[] =
{
    &QM_PORT_CFG_EN_BYTE_FIELD,
    &QM_PORT_CFG_EN_UG_FIELD,
    &QM_PORT_CFG_BBH_RX_BB_ID_FIELD,
    &QM_PORT_CFG_FW_PORT_ID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_PORT_CFG *****/
const ru_reg_rec QM_PORT_CFG_REG =
{
    "PORT_CFG",
#if RU_INCLUDE_DESC
    "PORT_CFG 0..10 Register",
    "En -enable\nWAN or LAN\nBB_ID\n",
#endif
    { QM_PORT_CFG_REG_OFFSET },
    QM_PORT_CFG_REG_RAM_CNT,
    4,
    677,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_PORT_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FC_UG_MASK_UG_EN, TYPE: Type_QM_TOP_QM_FLOW_CTRL_FC_UG_MASK_UG_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UG_EN *****/
const ru_field_rec QM_FC_UG_MASK_UG_EN_UG_EN_FIELD =
{
    "UG_EN",
#if RU_INCLUDE_DESC
    "",
    "UG/BUFMNG participates in FC (0-31)\n",
#endif
    { QM_FC_UG_MASK_UG_EN_UG_EN_FIELD_MASK },
    0,
    { QM_FC_UG_MASK_UG_EN_UG_EN_FIELD_WIDTH },
    { QM_FC_UG_MASK_UG_EN_UG_EN_FIELD_SHIFT },
    4294967295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FC_UG_MASK_UG_EN_FIELDS[] =
{
    &QM_FC_UG_MASK_UG_EN_UG_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FC_UG_MASK_UG_EN *****/
const ru_reg_rec QM_FC_UG_MASK_UG_EN_REG =
{
    "FC_UG_MASK_UG_EN",
#if RU_INCLUDE_DESC
    "FC_UG_MASK_UG_EN Register",
    "Bit per UG/BUFMNG:\n0 - UG is ignored during FC eval. drop enabled.\n1 - FC is sent according to UG thresholds\n",
#endif
    { QM_FC_UG_MASK_UG_EN_REG_OFFSET },
    0,
    0,
    678,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FC_UG_MASK_UG_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FC_QUEUE_MASK, TYPE: Type_QM_TOP_QM_FLOW_CTRL_FC_QUEUE_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: QUEUE_VEC *****/
const ru_field_rec QM_FC_QUEUE_MASK_QUEUE_VEC_FIELD =
{
    "QUEUE_VEC",
#if RU_INCLUDE_DESC
    "",
    "each bit represents queue.\n1 - fc is enabled for this queue (unmasked)\n0 - fc is disabled for this queue (masked)\n",
#endif
    { QM_FC_QUEUE_MASK_QUEUE_VEC_FIELD_MASK },
    0,
    { QM_FC_QUEUE_MASK_QUEUE_VEC_FIELD_WIDTH },
    { QM_FC_QUEUE_MASK_QUEUE_VEC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FC_QUEUE_MASK_FIELDS[] =
{
    &QM_FC_QUEUE_MASK_QUEUE_VEC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FC_QUEUE_MASK *****/
const ru_reg_rec QM_FC_QUEUE_MASK_REG =
{
    "FC_QUEUE_MASK",
#if RU_INCLUDE_DESC
    "FC_QUEUE_MASK 0..4 Register",
    "FC queue mask. 1 bit per queue.\nif 1 - FC is enable (unmasked)\nif 0 - FC is disable (masked)\n",
#endif
    { QM_FC_QUEUE_MASK_REG_OFFSET },
    QM_FC_QUEUE_MASK_REG_RAM_CNT,
    4,
    679,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FC_QUEUE_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FC_QUEUE_RANGE1_START, TYPE: Type_QM_TOP_QM_FLOW_CTRL_FC_QUEUE_RANGE1_START
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START_QUEUE *****/
const ru_field_rec QM_FC_QUEUE_RANGE1_START_START_QUEUE_FIELD =
{
    "START_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "First queue of range1 for which FC will be applied.\n",
#endif
    { QM_FC_QUEUE_RANGE1_START_START_QUEUE_FIELD_MASK },
    0,
    { QM_FC_QUEUE_RANGE1_START_START_QUEUE_FIELD_WIDTH },
    { QM_FC_QUEUE_RANGE1_START_START_QUEUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FC_QUEUE_RANGE1_START_FIELDS[] =
{
    &QM_FC_QUEUE_RANGE1_START_START_QUEUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FC_QUEUE_RANGE1_START *****/
const ru_reg_rec QM_FC_QUEUE_RANGE1_START_REG =
{
    "FC_QUEUE_RANGE1_START",
#if RU_INCLUDE_DESC
    "FC_QUEUE_RANGE1_START Register",
    "FC will be applied for 128 queues, starting at this queue.\nFor example, if set to 0, FC range1 will be between 0-127.\nAny queue in this range that needs to be excluded, will be masked using the FC_QUEUE mask register.\n",
#endif
    { QM_FC_QUEUE_RANGE1_START_REG_OFFSET },
    0,
    0,
    680,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FC_QUEUE_RANGE1_START_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FC_QUEUE_RANGE2_START, TYPE: Type_QM_TOP_QM_FLOW_CTRL_FC_QUEUE_RANGE2_START
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START_QUEUE *****/
const ru_field_rec QM_FC_QUEUE_RANGE2_START_START_QUEUE_FIELD =
{
    "START_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "First queue of range1 for which FC will be applied.\n",
#endif
    { QM_FC_QUEUE_RANGE2_START_START_QUEUE_FIELD_MASK },
    0,
    { QM_FC_QUEUE_RANGE2_START_START_QUEUE_FIELD_WIDTH },
    { QM_FC_QUEUE_RANGE2_START_START_QUEUE_FIELD_SHIFT },
    128,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FC_QUEUE_RANGE2_START_FIELDS[] =
{
    &QM_FC_QUEUE_RANGE2_START_START_QUEUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FC_QUEUE_RANGE2_START *****/
const ru_reg_rec QM_FC_QUEUE_RANGE2_START_REG =
{
    "FC_QUEUE_RANGE2_START",
#if RU_INCLUDE_DESC
    "FC_QUEUE_RANGE2_START Register",
    "FC will be applied for 128 queues, starting at this queue.\nFor example, if set to 128, FC range2 will be between 128-255.\nAny queue in this range that needs to be excluded, will be masked using the FC_QUEUE mask register.\n",
#endif
    { QM_FC_QUEUE_RANGE2_START_REG_OFFSET },
    0,
    0,
    681,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FC_QUEUE_RANGE2_START_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_DBG, TYPE: Type_QM_TOP_QM_FLOW_CTRL_DBG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS *****/
const ru_field_rec QM_DBG_STATUS_FIELD =
{
    "STATUS",
#if RU_INCLUDE_DESC
    "",
    "1 - exceeds thr\n0 - below thr\n",
#endif
    { QM_DBG_STATUS_FIELD_MASK },
    0,
    { QM_DBG_STATUS_FIELD_WIDTH },
    { QM_DBG_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DBG_FIELDS[] =
{
    &QM_DBG_STATUS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DBG *****/
const ru_reg_rec QM_DBG_REG =
{
    "DBG",
#if RU_INCLUDE_DESC
    "DBG Register",
    "Includes several debug signals from Flow control logic:\n[10:0]  - fc port state\n[21:11] - curr port state - virtual wire\n[22]    - qm_bb_fc_valid_out\n[23]    - arbiter fc_rr_resource_valid\n",
#endif
    { QM_DBG_REG_OFFSET },
    0,
    0,
    682,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DBG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_UG_OCCUPANCY_STATUS, TYPE: Type_QM_TOP_QM_FLOW_CTRL_UG_OCCUPANCY_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS *****/
const ru_field_rec QM_UG_OCCUPANCY_STATUS_STATUS_FIELD =
{
    "STATUS",
#if RU_INCLUDE_DESC
    "",
    "queue\n",
#endif
    { QM_UG_OCCUPANCY_STATUS_STATUS_FIELD_MASK },
    0,
    { QM_UG_OCCUPANCY_STATUS_STATUS_FIELD_WIDTH },
    { QM_UG_OCCUPANCY_STATUS_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_UG_OCCUPANCY_STATUS_FIELDS[] =
{
    &QM_UG_OCCUPANCY_STATUS_STATUS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_UG_OCCUPANCY_STATUS *****/
const ru_reg_rec QM_UG_OCCUPANCY_STATUS_REG =
{
    "UG_OCCUPANCY_STATUS",
#if RU_INCLUDE_DESC
    "UG_OCCUPANCY_STATUS 0..10 Register",
    "Working with UG:\n4b per port (bit per UG)\nOr working with BUFMNG:\n32b per port (bit per BUFMNG)\n\n",
#endif
    { QM_UG_OCCUPANCY_STATUS_REG_OFFSET },
    QM_UG_OCCUPANCY_STATUS_REG_RAM_CNT,
    4,
    683,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_UG_OCCUPANCY_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QUEUE_RANGE1_OCCUPANCY_STATUS, TYPE: Type_QM_TOP_QM_FLOW_CTRL_QUEUE_RANGE1_OCCUPANCY_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS *****/
const ru_field_rec QM_QUEUE_RANGE1_OCCUPANCY_STATUS_STATUS_FIELD =
{
    "STATUS",
#if RU_INCLUDE_DESC
    "",
    "1 - exceeds thr\n0 - below thr\n",
#endif
    { QM_QUEUE_RANGE1_OCCUPANCY_STATUS_STATUS_FIELD_MASK },
    0,
    { QM_QUEUE_RANGE1_OCCUPANCY_STATUS_STATUS_FIELD_WIDTH },
    { QM_QUEUE_RANGE1_OCCUPANCY_STATUS_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QUEUE_RANGE1_OCCUPANCY_STATUS_FIELDS[] =
{
    &QM_QUEUE_RANGE1_OCCUPANCY_STATUS_STATUS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QUEUE_RANGE1_OCCUPANCY_STATUS *****/
const ru_reg_rec QM_QUEUE_RANGE1_OCCUPANCY_STATUS_REG =
{
    "QUEUE_RANGE1_OCCUPANCY_STATUS",
#if RU_INCLUDE_DESC
    "QUEUE_RANGE1_OCCUPANCY_STATUS 0..43 Register",
    "Holds the flow control status for queues in range1. Each port holds a bit per queue.\n",
#endif
    { QM_QUEUE_RANGE1_OCCUPANCY_STATUS_REG_OFFSET },
    QM_QUEUE_RANGE1_OCCUPANCY_STATUS_REG_RAM_CNT,
    4,
    684,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QUEUE_RANGE1_OCCUPANCY_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QUEUE_RANGE2_OCCUPANCY_STATUS, TYPE: Type_QM_TOP_QM_FLOW_CTRL_QUEUE_RANGE2_OCCUPANCY_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS *****/
const ru_field_rec QM_QUEUE_RANGE2_OCCUPANCY_STATUS_STATUS_FIELD =
{
    "STATUS",
#if RU_INCLUDE_DESC
    "",
    "1 - exceeds thr\n0 - below thr\n",
#endif
    { QM_QUEUE_RANGE2_OCCUPANCY_STATUS_STATUS_FIELD_MASK },
    0,
    { QM_QUEUE_RANGE2_OCCUPANCY_STATUS_STATUS_FIELD_WIDTH },
    { QM_QUEUE_RANGE2_OCCUPANCY_STATUS_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QUEUE_RANGE2_OCCUPANCY_STATUS_FIELDS[] =
{
    &QM_QUEUE_RANGE2_OCCUPANCY_STATUS_STATUS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QUEUE_RANGE2_OCCUPANCY_STATUS *****/
const ru_reg_rec QM_QUEUE_RANGE2_OCCUPANCY_STATUS_REG =
{
    "QUEUE_RANGE2_OCCUPANCY_STATUS",
#if RU_INCLUDE_DESC
    "QUEUE_RANGE2_OCCUPANCY_STATUS 0..43 Register",
    "Holds the flow control status for queues in range2. Each port holds a bit per queue.\n",
#endif
    { QM_QUEUE_RANGE2_OCCUPANCY_STATUS_REG_OFFSET },
    QM_QUEUE_RANGE2_OCCUPANCY_STATUS_REG_RAM_CNT,
    4,
    685,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QUEUE_RANGE2_OCCUPANCY_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RD_DATA, TYPE: Type_QM_TOP_QM_CM_WR_PD_FIFO_RD_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_RD_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_RD_DATA_DATA_FIELD_MASK },
    0,
    { QM_RD_DATA_DATA_FIELD_WIDTH },
    { QM_RD_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_FIELDS[] =
{
    &QM_RD_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RD_DATA *****/
const ru_reg_rec QM_RD_DATA_REG =
{
    "RD_DATA",
#if RU_INCLUDE_DESC
    "RD_DATA 0..4 Register",
    "Debug - Read the head of the FIFO\n",
#endif
    { QM_RD_DATA_REG_OFFSET },
    QM_RD_DATA_REG_RAM_CNT,
    4,
    686,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_POP, TYPE: Type_QM_TOP_QM_CM_WR_PD_FIFO_POP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POP *****/
const ru_field_rec QM_POP_POP_FIELD =
{
    "POP",
#if RU_INCLUDE_DESC
    "",
    "Pop FIFO entry\n",
#endif
    { QM_POP_POP_FIELD_MASK },
    0,
    { QM_POP_POP_FIELD_WIDTH },
    { QM_POP_POP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_POP_FIELDS[] =
{
    &QM_POP_POP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_POP *****/
const ru_reg_rec QM_POP_REG =
{
    "POP",
#if RU_INCLUDE_DESC
    "POP Register",
    "Pop an entry in the FIFO\n",
#endif
    { QM_POP_REG_OFFSET },
    0,
    0,
    687,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_POP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RD_DATA_1, TYPE: Type_QM_TOP_QM_CM_RD_PD_FIFO_RD_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_RD_DATA_1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_RD_DATA_1_DATA_FIELD_MASK },
    0,
    { QM_RD_DATA_1_DATA_FIELD_WIDTH },
    { QM_RD_DATA_1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_1_FIELDS[] =
{
    &QM_RD_DATA_1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RD_DATA_1 *****/
const ru_reg_rec QM_RD_DATA_1_REG =
{
    "RD_DATA_1",
#if RU_INCLUDE_DESC
    "RD_DATA 0..4 Register",
    "Debug - Read the head of the FIFO\n",
#endif
    { QM_RD_DATA_1_REG_OFFSET },
    QM_RD_DATA_1_REG_RAM_CNT,
    4,
    688,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_POP_1, TYPE: Type_QM_TOP_QM_CM_RD_PD_FIFO_POP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POP *****/
const ru_field_rec QM_POP_1_POP_FIELD =
{
    "POP",
#if RU_INCLUDE_DESC
    "",
    "Pop FIFO entry\n",
#endif
    { QM_POP_1_POP_FIELD_MASK },
    0,
    { QM_POP_1_POP_FIELD_WIDTH },
    { QM_POP_1_POP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_POP_1_FIELDS[] =
{
    &QM_POP_1_POP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_POP_1 *****/
const ru_reg_rec QM_POP_1_REG =
{
    "POP_1",
#if RU_INCLUDE_DESC
    "POP Register",
    "Pop an entry in the FIFO\n",
#endif
    { QM_POP_1_REG_OFFSET },
    0,
    0,
    689,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_POP_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RD_DATA_2, TYPE: Type_QM_TOP_QM_CM_INPUT_FIFO_RD_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_RD_DATA_2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_RD_DATA_2_DATA_FIELD_MASK },
    0,
    { QM_RD_DATA_2_DATA_FIELD_WIDTH },
    { QM_RD_DATA_2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_2_FIELDS[] =
{
    &QM_RD_DATA_2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RD_DATA_2 *****/
const ru_reg_rec QM_RD_DATA_2_REG =
{
    "RD_DATA_2",
#if RU_INCLUDE_DESC
    "RD_DATA 0..4 Register",
    "Debug - Read the head of the FIFO\n",
#endif
    { QM_RD_DATA_2_REG_OFFSET },
    QM_RD_DATA_2_REG_RAM_CNT,
    4,
    690,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_POP_2, TYPE: Type_QM_TOP_QM_CM_INPUT_FIFO_POP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POP *****/
const ru_field_rec QM_POP_2_POP_FIELD =
{
    "POP",
#if RU_INCLUDE_DESC
    "",
    "Pop FIFO entry\n",
#endif
    { QM_POP_2_POP_FIELD_MASK },
    0,
    { QM_POP_2_POP_FIELD_WIDTH },
    { QM_POP_2_POP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_POP_2_FIELDS[] =
{
    &QM_POP_2_POP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_POP_2 *****/
const ru_reg_rec QM_POP_2_REG =
{
    "POP_2",
#if RU_INCLUDE_DESC
    "POP Register",
    "Pop an entry in the FIFO\n",
#endif
    { QM_POP_2_REG_OFFSET },
    0,
    0,
    691,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_POP_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_PDFIFO_PTR, TYPE: Type_QM_TOP_QM_TM_FIFO_PTR_PDFIFO_PTR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WR_PTR *****/
const ru_field_rec QM_PDFIFO_PTR_WR_PTR_FIELD =
{
    "WR_PTR",
#if RU_INCLUDE_DESC
    "",
    "PDFIFO WR pointers\n",
#endif
    { QM_PDFIFO_PTR_WR_PTR_FIELD_MASK },
    0,
    { QM_PDFIFO_PTR_WR_PTR_FIELD_WIDTH },
    { QM_PDFIFO_PTR_WR_PTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_PTR *****/
const ru_field_rec QM_PDFIFO_PTR_RD_PTR_FIELD =
{
    "RD_PTR",
#if RU_INCLUDE_DESC
    "",
    "PDFIFO RD pointers\n",
#endif
    { QM_PDFIFO_PTR_RD_PTR_FIELD_MASK },
    0,
    { QM_PDFIFO_PTR_RD_PTR_FIELD_WIDTH },
    { QM_PDFIFO_PTR_RD_PTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_PDFIFO_PTR_FIELDS[] =
{
    &QM_PDFIFO_PTR_WR_PTR_FIELD,
    &QM_PDFIFO_PTR_RD_PTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_PDFIFO_PTR *****/
const ru_reg_rec QM_PDFIFO_PTR_REG =
{
    "PDFIFO_PTR",
#if RU_INCLUDE_DESC
    "PDFIFO_PTR 0..159 Register",
    "PDFIFO per queue rd/wr pointers\n",
#endif
    { QM_PDFIFO_PTR_REG_OFFSET },
    QM_PDFIFO_PTR_REG_RAM_CNT,
    4,
    692,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_PDFIFO_PTR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_UPDATE_FIFO_PTR, TYPE: Type_QM_TOP_QM_TM_FIFO_PTR_UPDATE_FIFO_PTR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WR_PTR *****/
const ru_field_rec QM_UPDATE_FIFO_PTR_WR_PTR_FIELD =
{
    "WR_PTR",
#if RU_INCLUDE_DESC
    "",
    "UF WR pointers\n",
#endif
    { QM_UPDATE_FIFO_PTR_WR_PTR_FIELD_MASK },
    0,
    { QM_UPDATE_FIFO_PTR_WR_PTR_FIELD_WIDTH },
    { QM_UPDATE_FIFO_PTR_WR_PTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_PTR *****/
const ru_field_rec QM_UPDATE_FIFO_PTR_RD_PTR_FIELD =
{
    "RD_PTR",
#if RU_INCLUDE_DESC
    "",
    "UF RD pointers\n",
#endif
    { QM_UPDATE_FIFO_PTR_RD_PTR_FIELD_MASK },
    0,
    { QM_UPDATE_FIFO_PTR_RD_PTR_FIELD_WIDTH },
    { QM_UPDATE_FIFO_PTR_RD_PTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_UPDATE_FIFO_PTR_FIELDS[] =
{
    &QM_UPDATE_FIFO_PTR_WR_PTR_FIELD,
    &QM_UPDATE_FIFO_PTR_RD_PTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_UPDATE_FIFO_PTR *****/
const ru_reg_rec QM_UPDATE_FIFO_PTR_REG =
{
    "UPDATE_FIFO_PTR",
#if RU_INCLUDE_DESC
    "UPDATE_FIFO_PTR 0..15 Register",
    "Update FIFO rd/wr pointers\n",
#endif
    { QM_UPDATE_FIFO_PTR_REG_OFFSET },
    QM_UPDATE_FIFO_PTR_REG_RAM_CNT,
    4,
    693,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_UPDATE_FIFO_PTR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RD_DATA_POOL0, TYPE: Type_QM_TOP_QM_FPM_PREFETCH_FIFO_RD_DATA_POOL0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_RD_DATA_POOL0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_RD_DATA_POOL0_DATA_FIELD_MASK },
    0,
    { QM_RD_DATA_POOL0_DATA_FIELD_WIDTH },
    { QM_RD_DATA_POOL0_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_POOL0_FIELDS[] =
{
    &QM_RD_DATA_POOL0_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RD_DATA_POOL0 *****/
const ru_reg_rec QM_RD_DATA_POOL0_REG =
{
    "RD_DATA_POOL0",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL0 Register",
    "Read the head of the FIFO\n",
#endif
    { QM_RD_DATA_POOL0_REG_OFFSET },
    0,
    0,
    694,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_POOL0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RD_DATA_POOL1, TYPE: Type_QM_TOP_QM_FPM_PREFETCH_FIFO_RD_DATA_POOL1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_RD_DATA_POOL1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_RD_DATA_POOL1_DATA_FIELD_MASK },
    0,
    { QM_RD_DATA_POOL1_DATA_FIELD_WIDTH },
    { QM_RD_DATA_POOL1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_POOL1_FIELDS[] =
{
    &QM_RD_DATA_POOL1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RD_DATA_POOL1 *****/
const ru_reg_rec QM_RD_DATA_POOL1_REG =
{
    "RD_DATA_POOL1",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL1 Register",
    "Read the head of the FIFO\n",
#endif
    { QM_RD_DATA_POOL1_REG_OFFSET },
    0,
    0,
    695,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_POOL1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RD_DATA_POOL2, TYPE: Type_QM_TOP_QM_FPM_PREFETCH_FIFO_RD_DATA_POOL2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_RD_DATA_POOL2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_RD_DATA_POOL2_DATA_FIELD_MASK },
    0,
    { QM_RD_DATA_POOL2_DATA_FIELD_WIDTH },
    { QM_RD_DATA_POOL2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_POOL2_FIELDS[] =
{
    &QM_RD_DATA_POOL2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RD_DATA_POOL2 *****/
const ru_reg_rec QM_RD_DATA_POOL2_REG =
{
    "RD_DATA_POOL2",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL2 Register",
    "Read the head of the FIFO\n",
#endif
    { QM_RD_DATA_POOL2_REG_OFFSET },
    0,
    0,
    696,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_POOL2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RD_DATA_POOL3, TYPE: Type_QM_TOP_QM_FPM_PREFETCH_FIFO_RD_DATA_POOL3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_RD_DATA_POOL3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_RD_DATA_POOL3_DATA_FIELD_MASK },
    0,
    { QM_RD_DATA_POOL3_DATA_FIELD_WIDTH },
    { QM_RD_DATA_POOL3_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RD_DATA_POOL3_FIELDS[] =
{
    &QM_RD_DATA_POOL3_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RD_DATA_POOL3 *****/
const ru_reg_rec QM_RD_DATA_POOL3_REG =
{
    "RD_DATA_POOL3",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL3 Register",
    "Read the head of the FIFO\n",
#endif
    { QM_RD_DATA_POOL3_REG_OFFSET },
    0,
    0,
    697,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_RD_DATA_POOL3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_POP_3, TYPE: Type_QM_TOP_QM_FPM_PREFETCH_FIFO_POP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POP_POOL0 *****/
const ru_field_rec QM_POP_3_POP_POOL0_FIELD =
{
    "POP_POOL0",
#if RU_INCLUDE_DESC
    "",
    "Pop FIFO entry\n",
#endif
    { QM_POP_3_POP_POOL0_FIELD_MASK },
    0,
    { QM_POP_3_POP_POOL0_FIELD_WIDTH },
    { QM_POP_3_POP_POOL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POP_POOL1 *****/
const ru_field_rec QM_POP_3_POP_POOL1_FIELD =
{
    "POP_POOL1",
#if RU_INCLUDE_DESC
    "",
    "Pop FIFO entry\n",
#endif
    { QM_POP_3_POP_POOL1_FIELD_MASK },
    0,
    { QM_POP_3_POP_POOL1_FIELD_WIDTH },
    { QM_POP_3_POP_POOL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POP_POOL2 *****/
const ru_field_rec QM_POP_3_POP_POOL2_FIELD =
{
    "POP_POOL2",
#if RU_INCLUDE_DESC
    "",
    "Pop FIFO entry\n",
#endif
    { QM_POP_3_POP_POOL2_FIELD_MASK },
    0,
    { QM_POP_3_POP_POOL2_FIELD_WIDTH },
    { QM_POP_3_POP_POOL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POP_POOL3 *****/
const ru_field_rec QM_POP_3_POP_POOL3_FIELD =
{
    "POP_POOL3",
#if RU_INCLUDE_DESC
    "",
    "Pop FIFO entry\n",
#endif
    { QM_POP_3_POP_POOL3_FIELD_MASK },
    0,
    { QM_POP_3_POP_POOL3_FIELD_WIDTH },
    { QM_POP_3_POP_POOL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_POP_3_FIELDS[] =
{
    &QM_POP_3_POP_POOL0_FIELD,
    &QM_POP_3_POP_POOL1_FIELD,
    &QM_POP_3_POP_POOL2_FIELD,
    &QM_POP_3_POP_POOL3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_POP_3 *****/
const ru_reg_rec QM_POP_3_REG =
{
    "POP_3",
#if RU_INCLUDE_DESC
    "POP Register",
    "Pop an entry in the FIFO\n",
#endif
    { QM_POP_3_REG_OFFSET },
    0,
    0,
    698,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_POP_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_EPON_RPT_CNT_COUNTER, TYPE: Type_QM_TOP_QM_EPON_RPT_CNT_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_EPON_RPT_CNT_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_EPON_RPT_CNT_COUNTER_DATA_FIELD_MASK },
    0,
    { QM_EPON_RPT_CNT_COUNTER_DATA_FIELD_WIDTH },
    { QM_EPON_RPT_CNT_COUNTER_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EPON_RPT_CNT_COUNTER_FIELDS[] =
{
    &QM_EPON_RPT_CNT_COUNTER_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_EPON_RPT_CNT_COUNTER *****/
const ru_reg_rec QM_EPON_RPT_CNT_COUNTER_REG =
{
    "EPON_RPT_CNT_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER 0..319 Register",
    "Counter - For each of the 32-queues in a batch, this counter stores a 32-bit accumulated and overhead byte counter per queue.\nword0: {accumulated_bytes[31:0]}\nword1: {accumulated_overhead[31:0}\n\nThere are two words per queue starting at queue0 up to queue 160.\n",
#endif
    { QM_EPON_RPT_CNT_COUNTER_REG_OFFSET },
    QM_EPON_RPT_CNT_COUNTER_REG_RAM_CNT,
    4,
    699,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EPON_RPT_CNT_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_EPON_RPT_CNT_QUEUE_STATUS, TYPE: Type_QM_TOP_QM_EPON_RPT_CNT_QUEUE_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS_BIT_VECTOR *****/
const ru_field_rec QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD =
{
    "STATUS_BIT_VECTOR",
#if RU_INCLUDE_DESC
    "",
    "Status bit vector - a bit per queue indicates if the queue has been updated.\n",
#endif
    { QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD_MASK },
    0,
    { QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD_WIDTH },
    { QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EPON_RPT_CNT_QUEUE_STATUS_FIELDS[] =
{
    &QM_EPON_RPT_CNT_QUEUE_STATUS_STATUS_BIT_VECTOR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_EPON_RPT_CNT_QUEUE_STATUS *****/
const ru_reg_rec QM_EPON_RPT_CNT_QUEUE_STATUS_REG =
{
    "EPON_RPT_CNT_QUEUE_STATUS",
#if RU_INCLUDE_DESC
    "QUEUE_STATUS 0..4 Register",
    "Status bit vector - For each of the 32-queues in a batch, this status indicates which queue counter has been updated.\n",
#endif
    { QM_EPON_RPT_CNT_QUEUE_STATUS_REG_OFFSET },
    QM_EPON_RPT_CNT_QUEUE_STATUS_REG_RAM_CNT,
    4,
    700,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EPON_RPT_CNT_QUEUE_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_DROP_COUNTER_COUNTER, TYPE: Type_QM_TOP_QM_DROP_COUNTER_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_DROP_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_DROP_COUNTER_COUNTER_DATA_FIELD_MASK },
    0,
    { QM_DROP_COUNTER_COUNTER_DATA_FIELD_WIDTH },
    { QM_DROP_COUNTER_COUNTER_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DROP_COUNTER_COUNTER_FIELDS[] =
{
    &QM_DROP_COUNTER_COUNTER_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DROP_COUNTER_COUNTER *****/
const ru_reg_rec QM_DROP_COUNTER_COUNTER_REG =
{
    "DROP_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER 0..319 Register",
    "Counter.\nword0 - 0x0:{6`b0,pkt_cnt[25:0]}\nword1 - 0x4:{byte_cnt[31:0]}\n\nin WRED drop mode:\nword0 - 0x0: color1 dropped packets\nword1 - 0x4: color0 dropped packets\n\n\nThere are two words per queue starting at queue0 up to queue 160.\n",
#endif
    { QM_DROP_COUNTER_COUNTER_REG_OFFSET },
    QM_DROP_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    701,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DROP_COUNTER_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER, TYPE: Type_QM_TOP_QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_DATA_FIELD_MASK },
    0,
    { QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_DATA_FIELD_WIDTH },
    { QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_FIELDS[] =
{
    &QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER *****/
const ru_reg_rec QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_REG =
{
    "TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER 0..319 Register",
    "Counter.\nword0 - 0x0:{byte_cnt[3:0],pkt_cnt[27:0]}\nword1 - 0x4:{byte_cnt[35:4]}\n\nThere are two words per queue starting at queue0 up to queue 160.\n",
#endif
    { QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_REG_OFFSET },
    QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    702,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_DQM_VALID_COUNTER_COUNTER, TYPE: Type_QM_TOP_QM_DQM_VALID_COUNTER_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD_MASK },
    0,
    { QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD_WIDTH },
    { QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DQM_VALID_COUNTER_COUNTER_FIELDS[] =
{
    &QM_DQM_VALID_COUNTER_COUNTER_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DQM_VALID_COUNTER_COUNTER *****/
const ru_reg_rec QM_DQM_VALID_COUNTER_COUNTER_REG =
{
    "DQM_VALID_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER 0..319 Register",
    "Counter.\nword0:{15`b0,pkt_cnt[16:0]}\nword1:{2`b0,byte_cnt[29:0]}\n\nThere are two words per queue starting at queue0 up to queue 160.\n",
#endif
    { QM_DQM_VALID_COUNTER_COUNTER_REG_OFFSET },
    QM_DQM_VALID_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    703,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DQM_VALID_COUNTER_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_TOTAL_VALID_COUNTER_COUNTER, TYPE: Type_QM_TOP_QM_TOTAL_VALID_COUNTER_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD_MASK },
    0,
    { QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD_WIDTH },
    { QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_TOTAL_VALID_COUNTER_COUNTER_FIELDS[] =
{
    &QM_TOTAL_VALID_COUNTER_COUNTER_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_TOTAL_VALID_COUNTER_COUNTER *****/
const ru_reg_rec QM_TOTAL_VALID_COUNTER_COUNTER_REG =
{
    "TOTAL_VALID_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER 0..639 Register",
    "Counter.\nword0:{15b0,pkt_cnt[16:0]}\nword1:{2b0,byte_cnt[29:0]}\nword2:{14b0,res_cnt[17:0]}\nword3: reserved\n\nThere are three (+one reserved) words per queue starting at queue0 up to queue 160.\n",
#endif
    { QM_TOTAL_VALID_COUNTER_COUNTER_REG_OFFSET },
    QM_TOTAL_VALID_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    704,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_TOTAL_VALID_COUNTER_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_WRED_PROFILE_COLOR_MIN_THR_0, TYPE: Type_QM_TOP_QM_WRED_PROFILE_COLOR_MIN_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_THR *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD =
{
    "MIN_THR",
#if RU_INCLUDE_DESC
    "",
    "WRED Color Min Threshold.\nThis field represents the higher 24-bits of the queue occupancy byte threshold.\nbyte_threshold = THR*64.\n",
#endif
    { QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD_SHIFT },
    131072,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLW_CTRL_EN *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD =
{
    "FLW_CTRL_EN",
#if RU_INCLUDE_DESC
    "",
    "0 - flow control disable. regular WRED profile\n1 - flow control enable. no WRED drop, wake up appropriate runner task when crossed.\n",
#endif
    { QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_MIN_THR_0_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_MIN_THR_0_MIN_THR_FIELD,
    &QM_WRED_PROFILE_COLOR_MIN_THR_0_FLW_CTRL_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_WRED_PROFILE_COLOR_MIN_THR_0 *****/
const ru_reg_rec QM_WRED_PROFILE_COLOR_MIN_THR_0_REG =
{
    "WRED_PROFILE_COLOR_MIN_THR_0",
#if RU_INCLUDE_DESC
    "COLOR_MIN_THR 0..1 Register",
    "WRED Color min thresholds\n",
#endif
    { QM_WRED_PROFILE_COLOR_MIN_THR_0_REG_OFFSET },
    QM_WRED_PROFILE_COLOR_MIN_THR_0_REG_RAM_CNT,
    48,
    705,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_WRED_PROFILE_COLOR_MIN_THR_1, TYPE: Type_QM_TOP_QM_WRED_PROFILE_COLOR_MIN_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_THR *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD =
{
    "MIN_THR",
#if RU_INCLUDE_DESC
    "",
    "WRED Color Min Threshold.\nThis field represents the higher 24-bits of the queue occupancy byte threshold.\nbyte_threshold = THR*64.\n",
#endif
    { QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD_SHIFT },
    131072,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLW_CTRL_EN *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD =
{
    "FLW_CTRL_EN",
#if RU_INCLUDE_DESC
    "",
    "0 - flow control disable. regular WRED profile\n1 - flow control enable. no WRED drop, wake up appropriate runner task when crossed.\n",
#endif
    { QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_MIN_THR_1_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_MIN_THR_1_MIN_THR_FIELD,
    &QM_WRED_PROFILE_COLOR_MIN_THR_1_FLW_CTRL_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_WRED_PROFILE_COLOR_MIN_THR_1 *****/
const ru_reg_rec QM_WRED_PROFILE_COLOR_MIN_THR_1_REG =
{
    "WRED_PROFILE_COLOR_MIN_THR_1",
#if RU_INCLUDE_DESC
    "COLOR_MIN_THR 0..1 Register",
    "WRED Color min thresholds\n",
#endif
    { QM_WRED_PROFILE_COLOR_MIN_THR_1_REG_OFFSET },
    QM_WRED_PROFILE_COLOR_MIN_THR_1_REG_RAM_CNT,
    48,
    706,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_WRED_PROFILE_COLOR_MAX_THR_0, TYPE: Type_QM_TOP_QM_WRED_PROFILE_COLOR_MAX_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_THR *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD =
{
    "MAX_THR",
#if RU_INCLUDE_DESC
    "",
    "WRED Color Max Threshold.\nThis field represents the higher 24-bits of the queue occupancy byte threshold.\nbyte_threshold = THR*64.\n",
#endif
    { QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD_SHIFT },
    131072,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_MAX_THR_0_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_MAX_THR_0_MAX_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_WRED_PROFILE_COLOR_MAX_THR_0 *****/
const ru_reg_rec QM_WRED_PROFILE_COLOR_MAX_THR_0_REG =
{
    "WRED_PROFILE_COLOR_MAX_THR_0",
#if RU_INCLUDE_DESC
    "COLOR_MAX_THR 0..1 Register",
    "WRED Color max thresholds\n",
#endif
    { QM_WRED_PROFILE_COLOR_MAX_THR_0_REG_OFFSET },
    QM_WRED_PROFILE_COLOR_MAX_THR_0_REG_RAM_CNT,
    48,
    707,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_WRED_PROFILE_COLOR_MAX_THR_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_WRED_PROFILE_COLOR_MAX_THR_1, TYPE: Type_QM_TOP_QM_WRED_PROFILE_COLOR_MAX_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_THR *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD =
{
    "MAX_THR",
#if RU_INCLUDE_DESC
    "",
    "WRED Color Max Threshold.\nThis field represents the higher 24-bits of the queue occupancy byte threshold.\nbyte_threshold = THR*64.\n",
#endif
    { QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD_SHIFT },
    131072,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_MAX_THR_1_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_MAX_THR_1_MAX_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_WRED_PROFILE_COLOR_MAX_THR_1 *****/
const ru_reg_rec QM_WRED_PROFILE_COLOR_MAX_THR_1_REG =
{
    "WRED_PROFILE_COLOR_MAX_THR_1",
#if RU_INCLUDE_DESC
    "COLOR_MAX_THR 0..1 Register",
    "WRED Color max thresholds\n",
#endif
    { QM_WRED_PROFILE_COLOR_MAX_THR_1_REG_OFFSET },
    QM_WRED_PROFILE_COLOR_MAX_THR_1_REG_RAM_CNT,
    48,
    708,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_WRED_PROFILE_COLOR_MAX_THR_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_WRED_PROFILE_COLOR_SLOPE_0, TYPE: Type_QM_TOP_QM_WRED_PROFILE_COLOR_SLOPE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SLOPE_MANTISSA *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD =
{
    "SLOPE_MANTISSA",
#if RU_INCLUDE_DESC
    "",
    "WRED Color slope mantissa.\n",
#endif
    { QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SLOPE_EXP *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD =
{
    "SLOPE_EXP",
#if RU_INCLUDE_DESC
    "",
    "WRED Color slope exponent.\n",
#endif
    { QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_SLOPE_0_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_MANTISSA_FIELD,
    &QM_WRED_PROFILE_COLOR_SLOPE_0_SLOPE_EXP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_WRED_PROFILE_COLOR_SLOPE_0 *****/
const ru_reg_rec QM_WRED_PROFILE_COLOR_SLOPE_0_REG =
{
    "WRED_PROFILE_COLOR_SLOPE_0",
#if RU_INCLUDE_DESC
    "COLOR_SLOPE 0..1 Register",
    "WRED Color slopes\n",
#endif
    { QM_WRED_PROFILE_COLOR_SLOPE_0_REG_OFFSET },
    QM_WRED_PROFILE_COLOR_SLOPE_0_REG_RAM_CNT,
    48,
    709,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_WRED_PROFILE_COLOR_SLOPE_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_WRED_PROFILE_COLOR_SLOPE_1, TYPE: Type_QM_TOP_QM_WRED_PROFILE_COLOR_SLOPE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SLOPE_MANTISSA *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD =
{
    "SLOPE_MANTISSA",
#if RU_INCLUDE_DESC
    "",
    "WRED Color slope mantissa.\n",
#endif
    { QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SLOPE_EXP *****/
const ru_field_rec QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD =
{
    "SLOPE_EXP",
#if RU_INCLUDE_DESC
    "",
    "WRED Color slope exponent.\n",
#endif
    { QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD_MASK },
    0,
    { QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD_WIDTH },
    { QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_PROFILE_COLOR_SLOPE_1_FIELDS[] =
{
    &QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_MANTISSA_FIELD,
    &QM_WRED_PROFILE_COLOR_SLOPE_1_SLOPE_EXP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_WRED_PROFILE_COLOR_SLOPE_1 *****/
const ru_reg_rec QM_WRED_PROFILE_COLOR_SLOPE_1_REG =
{
    "WRED_PROFILE_COLOR_SLOPE_1",
#if RU_INCLUDE_DESC
    "COLOR_SLOPE 0..1 Register",
    "WRED Color slopes\n",
#endif
    { QM_WRED_PROFILE_COLOR_SLOPE_1_REG_OFFSET },
    QM_WRED_PROFILE_COLOR_SLOPE_1_REG_RAM_CNT,
    48,
    710,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_WRED_PROFILE_COLOR_SLOPE_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QUEUE_CONTEXT_CONTEXT, TYPE: Type_QM_TOP_QM_QUEUE_CONTEXT_CONTEXT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WRED_PROFILE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD =
{
    "WRED_PROFILE",
#if RU_INCLUDE_DESC
    "",
    "Defines to which WRED Profile this queue belongs to.\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COPY_DEC_PROFILE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD =
{
    "COPY_DEC_PROFILE",
#if RU_INCLUDE_DESC
    "",
    "Defines to which Copy Decision Profile this queue belongs.\nprofile 3d7 is copy_to_ddr: always copy to DDR\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_COPY_DISABLE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD =
{
    "DDR_COPY_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Defines this queue never to copy to DDR.\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGGREGATION_DISABLE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD =
{
    "AGGREGATION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Defines this queue never to aggregated PDs.\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_UG_OR_BUFMNG *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_OR_BUFMNG_FIELD =
{
    "FPM_UG_OR_BUFMNG",
#if RU_INCLUDE_DESC
    "",
    "Defines to which FPM UG or BUFMNG this queue belongs.\nUse BUFMNG or UG according to the config bit: BUFMNG_EN_OR_UG_CNTR in QM_GLOBAL_CFG.QM_GENERAL_CTRL2\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_OR_BUFMNG_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_OR_BUFMNG_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_OR_BUFMNG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLUSIVE_PRIORITY *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD =
{
    "EXCLUSIVE_PRIORITY",
#if RU_INCLUDE_DESC
    "",
    "Defines this queue with exclusive priority.\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_802_1AE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD =
{
    "Q_802_1AE",
#if RU_INCLUDE_DESC
    "",
    "Defines this queue as 802.1AE for EPON packet overhead calculations.\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SCI *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD =
{
    "SCI",
#if RU_INCLUDE_DESC
    "",
    "Configures SCI for EPON packet overhead calculations.\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FEC_ENABLE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD =
{
    "FEC_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "FEC enable configuration for EPON packet overhead calculations.\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RES_PROFILE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD =
{
    "RES_PROFILE",
#if RU_INCLUDE_DESC
    "",
    "FPM reservation profile.\nOnce the QM goes over global FPM reservation threshold.\nQueue with more bytes the defined in the profile will be dropped.\nProfile 0 means no drop due to FPM reservation for the queues with this profile.\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SPARE_ROOM_0 *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_0_FIELD =
{
    "SPARE_ROOM_0",
#if RU_INCLUDE_DESC
    "",
    "Select 1 out of 4 headroom and tailroom pairs. if PDs SOP select is set to 0. This profile is chosen\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_0_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_0_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SPARE_ROOM_1 *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_1_FIELD =
{
    "SPARE_ROOM_1",
#if RU_INCLUDE_DESC
    "",
    "Select 1 out of 4 headroom and tailroom pairs. if PDs SOP select is set to 1. This profile is chosen\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_1_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_1_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SERVICE_QUEUE_PROFILE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_SERVICE_QUEUE_PROFILE_FIELD =
{
    "SERVICE_QUEUE_PROFILE",
#if RU_INCLUDE_DESC
    "",
    "17-options, 16 for the service queue and an extra one for using its own queue (sort of a service-queue disable bit)\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_SERVICE_QUEUE_PROFILE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_SERVICE_QUEUE_PROFILE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_SERVICE_QUEUE_PROFILE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMESTAMP_RES_PROFILE *****/
const ru_field_rec QM_QUEUE_CONTEXT_CONTEXT_TIMESTAMP_RES_PROFILE_FIELD =
{
    "TIMESTAMP_RES_PROFILE",
#if RU_INCLUDE_DESC
    "",
    "Select which profile this queue belongs to. 1 out of 2 resolutions profiles, for 10b out of 32b timestamp resolution in the PD:\n0d = profile0\n1d = profile1\n2d = profile2\n3d = profile3\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_TIMESTAMP_RES_PROFILE_FIELD_MASK },
    0,
    { QM_QUEUE_CONTEXT_CONTEXT_TIMESTAMP_RES_PROFILE_FIELD_WIDTH },
    { QM_QUEUE_CONTEXT_CONTEXT_TIMESTAMP_RES_PROFILE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QUEUE_CONTEXT_CONTEXT_FIELDS[] =
{
    &QM_QUEUE_CONTEXT_CONTEXT_WRED_PROFILE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_COPY_DEC_PROFILE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_DDR_COPY_DISABLE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_AGGREGATION_DISABLE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_FPM_UG_OR_BUFMNG_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_EXCLUSIVE_PRIORITY_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_Q_802_1AE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_SCI_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_FEC_ENABLE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_RES_PROFILE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_0_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_SPARE_ROOM_1_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_SERVICE_QUEUE_PROFILE_FIELD,
    &QM_QUEUE_CONTEXT_CONTEXT_TIMESTAMP_RES_PROFILE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QUEUE_CONTEXT_CONTEXT *****/
const ru_reg_rec QM_QUEUE_CONTEXT_CONTEXT_REG =
{
    "QUEUE_CONTEXT_CONTEXT",
#if RU_INCLUDE_DESC
    "QUEUE_CONTEXT 0..159 Register",
    "This RAM holds all queue attributes.\nWRED Profile            3:0\nCopy decision profile\t6:4\nDDR copy disable\t7\nAggregation Disable\t8\nFPM UGroup or BUFMNG\t13:9\nExclusive Priority\t14\n802.1AE\t                15\nSCI\t                16\nFEC Enable\t        17\nReservation Profile     20:18\nHeadroom1 Profile       22:21\nHeadroom2 Profile       24:23\nService Queue Profile   29:25\nTimestamp Profile       31:30\n",
#endif
    { QM_QUEUE_CONTEXT_CONTEXT_REG_OFFSET },
    QM_QUEUE_CONTEXT_CONTEXT_REG_RAM_CNT,
    4,
    711,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    14,
    QM_QUEUE_CONTEXT_CONTEXT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: QUEUE_NUM *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD =
{
    "QUEUE_NUM",
#if RU_INCLUDE_DESC
    "",
    "Queue Number\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "",
    "Command:\n00 - Nothing\n01 - Write\n10 - Read\n11 - Read No commit (entry not popped)\n\nWill trigger a read/write from the selected RAM\n\nIMPORTANT: Read is for debug purpose only. shouldnt be used during regular QM work on the requested queue (HW pop).\nPopping the same queue both from CPU and HW could cause to race condition which will cause to incorrect data output. It could occur when there is only one entry in the queue which is accessed both from the CPU and the HW.\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DONE *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD =
{
    "DONE",
#if RU_INCLUDE_DESC
    "",
    "Indicates that read/write to DQM is done\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ERROR *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD =
{
    "ERROR",
#if RU_INCLUDE_DESC
    "",
    "Indicates that that an error occured (write on full or read on empty)\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_QUEUE_NUM_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_CMD_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_DONE_FIELD,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_ERROR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_CTRL Register",
    "CPU PD Indirect Access Control\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG_RAM_CNT,
    64,
    712,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0 *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA 0..3 Register",
    "CPU PD Indirect Write data to DQM.\nFirst entry represents PD[127:96] and so on until the last entry representing PD[31:0].\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG_RAM_CNT,
    64,
    713,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1 *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA 0..3 Register",
    "CPU PD Indirect Write data to DQM.\nFirst entry represents PD[127:96] and so on until the last entry representing PD[31:0].\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG_RAM_CNT,
    64,
    714,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2 *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA 0..3 Register",
    "CPU PD Indirect Write data to DQM.\nFirst entry represents PD[127:96] and so on until the last entry representing PD[31:0].\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG_RAM_CNT,
    64,
    715,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3 *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA 0..3 Register",
    "CPU PD Indirect Write data to DQM.\nFirst entry represents PD[127:96] and so on until the last entry representing PD[31:0].\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG_RAM_CNT,
    64,
    716,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0 *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA 0..3 Register",
    "CPU PD Indirect Read data from DQM.\nFirst entry represents PD[127:96] and so on until the last entry representing PD[31:0].\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG_RAM_CNT,
    64,
    717,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1 *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA 0..3 Register",
    "CPU PD Indirect Read data from DQM.\nFirst entry represents PD[127:96] and so on until the last entry representing PD[31:0].\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG_RAM_CNT,
    64,
    718,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2 *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA 0..3 Register",
    "CPU PD Indirect Read data from DQM.\nFirst entry represents PD[127:96] and so on until the last entry representing PD[31:0].\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG_RAM_CNT,
    64,
    719,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3, TYPE: Type_QM_TOP_QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD_MASK },
    0,
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD_WIDTH },
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_FIELDS[] =
{
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3 *****/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG =
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA 0..3 Register",
    "CPU PD Indirect Read data from DQM.\nFirst entry represents PD[127:96] and so on until the last entry representing PD[31:0].\n",
#endif
    { QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG_OFFSET },
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG_RAM_CNT,
    64,
    720,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RUNNER_GRP_RNR_CONFIG, TYPE: Type_QM_TOP_QM_RUNNER_GRP_RNR_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RNR_BB_ID *****/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD =
{
    "RNR_BB_ID",
#if RU_INCLUDE_DESC
    "",
    "Runner BB ID associated with this configuration.\n",
#endif
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD_WIDTH },
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RNR_TASK *****/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD =
{
    "RNR_TASK",
#if RU_INCLUDE_DESC
    "",
    "Runner Task number to be woken up when the update FIFO is written to.\n",
#endif
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD_WIDTH },
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RNR_ENABLE *****/
const ru_field_rec QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD =
{
    "RNR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable this runner interface\n",
#endif
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD_WIDTH },
    { QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RUNNER_GRP_RNR_CONFIG_FIELDS[] =
{
    &QM_RUNNER_GRP_RNR_CONFIG_RNR_BB_ID_FIELD,
    &QM_RUNNER_GRP_RNR_CONFIG_RNR_TASK_FIELD,
    &QM_RUNNER_GRP_RNR_CONFIG_RNR_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RUNNER_GRP_RNR_CONFIG *****/
const ru_reg_rec QM_RUNNER_GRP_RNR_CONFIG_REG =
{
    "RUNNER_GRP_RNR_CONFIG",
#if RU_INCLUDE_DESC
    "RNR_CONFIG Register",
    "Runners Configurations\n",
#endif
    { QM_RUNNER_GRP_RNR_CONFIG_REG_OFFSET },
    QM_RUNNER_GRP_RNR_CONFIG_REG_RAM_CNT,
    16,
    721,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_RUNNER_GRP_RNR_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RUNNER_GRP_QUEUE_CONFIG, TYPE: Type_QM_TOP_QM_RUNNER_GRP_QUEUE_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START_QUEUE *****/
const ru_field_rec QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD =
{
    "START_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "Indicates the Queue that starts this runner group. Queues belonging to the runner group are defined by the following equation:\nSTART_QUEUE <= runner_queues <= END_QUEUE\n",
#endif
    { QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD_WIDTH },
    { QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: END_QUEUE *****/
const ru_field_rec QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD =
{
    "END_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "Indicates the Queue that ends this runner group.\nQueues belonging to the runner group are defined by the following equation:\nSTART_QUEUE <= runner_queues <= END_QUEUE\n",
#endif
    { QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD_WIDTH },
    { QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RUNNER_GRP_QUEUE_CONFIG_FIELDS[] =
{
    &QM_RUNNER_GRP_QUEUE_CONFIG_START_QUEUE_FIELD,
    &QM_RUNNER_GRP_QUEUE_CONFIG_END_QUEUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RUNNER_GRP_QUEUE_CONFIG *****/
const ru_reg_rec QM_RUNNER_GRP_QUEUE_CONFIG_REG =
{
    "RUNNER_GRP_QUEUE_CONFIG",
#if RU_INCLUDE_DESC
    "QUEUE_CONFIG Register",
    "Consecutive queues which are associated with this runner\n",
#endif
    { QM_RUNNER_GRP_QUEUE_CONFIG_REG_OFFSET },
    QM_RUNNER_GRP_QUEUE_CONFIG_REG_RAM_CNT,
    16,
    722,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_RUNNER_GRP_QUEUE_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RUNNER_GRP_PDFIFO_CONFIG, TYPE: Type_QM_TOP_QM_RUNNER_GRP_PDFIFO_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BASE_ADDR *****/
const ru_field_rec QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD =
{
    "BASE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_ADDR*8).\n",
#endif
    { QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD_WIDTH },
    { QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE *****/
const ru_field_rec QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD =
{
    "SIZE",
#if RU_INCLUDE_DESC
    "",
    "PD FIFO Size\n0 - 2 entries\n1 - 4 entries\n2 - 8 entries\n",
#endif
    { QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD_WIDTH },
    { QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RUNNER_GRP_PDFIFO_CONFIG_FIELDS[] =
{
    &QM_RUNNER_GRP_PDFIFO_CONFIG_BASE_ADDR_FIELD,
    &QM_RUNNER_GRP_PDFIFO_CONFIG_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RUNNER_GRP_PDFIFO_CONFIG *****/
const ru_reg_rec QM_RUNNER_GRP_PDFIFO_CONFIG_REG =
{
    "RUNNER_GRP_PDFIFO_CONFIG",
#if RU_INCLUDE_DESC
    "PDFIFO_CONFIG Register",
    "head of the queue PD FIFO attributes\n",
#endif
    { QM_RUNNER_GRP_PDFIFO_CONFIG_REG_OFFSET },
    QM_RUNNER_GRP_PDFIFO_CONFIG_REG_RAM_CNT,
    16,
    723,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_RUNNER_GRP_PDFIFO_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG, TYPE: Type_QM_TOP_QM_RUNNER_GRP_UPDATE_FIFO_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BASE_ADDR *****/
const ru_field_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD =
{
    "BASE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_ADDR*8).\n",
#endif
    { QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD_WIDTH },
    { QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE *****/
const ru_field_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD =
{
    "SIZE",
#if RU_INCLUDE_DESC
    "",
    "PD FIFO Size\n0 - 8 entries\n1 - 16 entries\n2 - 32 entries\n3 - 64 entries\n4 - 128 entries\n5 - 256 entries\n",
#endif
    { QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD_MASK },
    0,
    { QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD_WIDTH },
    { QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_FIELDS[] =
{
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_BASE_ADDR_FIELD,
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG *****/
const ru_reg_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG =
{
    "RUNNER_GRP_UPDATE_FIFO_CONFIG",
#if RU_INCLUDE_DESC
    "UPDATE_FIFO_CONFIG Register",
    "Update FIFO attributes\n",
#endif
    { QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG_OFFSET },
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG_RAM_CNT,
    16,
    724,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_POOLS_THR, TYPE: Type_QM_TOP_QM_FPM_POOLS_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_LOWER_THR *****/
const ru_field_rec QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD =
{
    "FPM_LOWER_THR",
#if RU_INCLUDE_DESC
    "",
    "FPM Lower Threshold.\nWhen working in packet drop mode (FPM_BP_ENABLE=0), Then:\n* If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (FPM_LOWER_THR) < (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dropped.\n* If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped.\nWhen working in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) < (FPM_LOWER_THR), then backpressure is applied to re-order (in this case FPM_HIGHER_THR is dont care).\n",
#endif
    { QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD_MASK },
    0,
    { QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD_WIDTH },
    { QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_HIGHER_THR *****/
const ru_field_rec QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD =
{
    "FPM_HIGHER_THR",
#if RU_INCLUDE_DESC
    "",
    "FPM Higher Threshold.\nWhen working in packet drop mode (FPM_BP_ENABLE=0), Then:\n* If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (FPM_LOWER_THR) < (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dropped.\n* If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped.\nWhen working in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) < (FPM_LOWER_THR), then backpressure is applied to re-order (in this case FPM_HIGHER_THR is dont care).\n",
#endif
    { QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD_MASK },
    0,
    { QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD_WIDTH },
    { QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_POOLS_THR_FIELDS[] =
{
    &QM_FPM_POOLS_THR_FPM_LOWER_THR_FIELD,
    &QM_FPM_POOLS_THR_FPM_HIGHER_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_POOLS_THR *****/
const ru_reg_rec QM_FPM_POOLS_THR_REG =
{
    "FPM_POOLS_THR",
#if RU_INCLUDE_DESC
    "THR Register",
    "Hold 2 thresholds per FPM pool for priority management\n",
#endif
    { QM_FPM_POOLS_THR_REG_OFFSET },
    QM_FPM_POOLS_THR_REG_RAM_CNT,
    32,
    725,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_FPM_POOLS_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_USR_GRP_LOWER_THR, TYPE: Type_QM_TOP_QM_FPM_USR_GRP_LOWER_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_GRP_LOWER_THR *****/
const ru_field_rec QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD =
{
    "FPM_GRP_LOWER_THR",
#if RU_INCLUDE_DESC
    "",
    "FPM group Lower Threshold.\n* If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped.\n* If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dropped.\n* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.\n",
#endif
    { QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD_MASK },
    0,
    { QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD_WIDTH },
    { QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD_SHIFT },
    65401,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_USR_GRP_LOWER_THR_FIELDS[] =
{
    &QM_FPM_USR_GRP_LOWER_THR_FPM_GRP_LOWER_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_USR_GRP_LOWER_THR *****/
const ru_reg_rec QM_FPM_USR_GRP_LOWER_THR_REG =
{
    "FPM_USR_GRP_LOWER_THR",
#if RU_INCLUDE_DESC
    "LOWER_THR Register",
    "Holds FPM user group lower threshold.\n",
#endif
    { QM_FPM_USR_GRP_LOWER_THR_REG_OFFSET },
    QM_FPM_USR_GRP_LOWER_THR_REG_RAM_CNT,
    32,
    726,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_USR_GRP_LOWER_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_USR_GRP_MID_THR, TYPE: Type_QM_TOP_QM_FPM_USR_GRP_MID_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_GRP_MID_THR *****/
const ru_field_rec QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD =
{
    "FPM_GRP_MID_THR",
#if RU_INCLUDE_DESC
    "",
    "FPM group Lower Threshold.\n* If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped.\n* If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dropped.\n* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.\n",
#endif
    { QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD_MASK },
    0,
    { QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD_WIDTH },
    { QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD_SHIFT },
    65401,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_USR_GRP_MID_THR_FIELDS[] =
{
    &QM_FPM_USR_GRP_MID_THR_FPM_GRP_MID_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_USR_GRP_MID_THR *****/
const ru_reg_rec QM_FPM_USR_GRP_MID_THR_REG =
{
    "FPM_USR_GRP_MID_THR",
#if RU_INCLUDE_DESC
    "MID_THR Register",
    "Holds FPM user group middle threshold.\n*IMPORTANT* if buffer reservations is enabled, the following should be honored:\nHIGHER_THR-MID_THR > 16\n",
#endif
    { QM_FPM_USR_GRP_MID_THR_REG_OFFSET },
    QM_FPM_USR_GRP_MID_THR_REG_RAM_CNT,
    32,
    727,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_USR_GRP_MID_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_USR_GRP_HIGHER_THR, TYPE: Type_QM_TOP_QM_FPM_USR_GRP_HIGHER_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_GRP_HIGHER_THR *****/
const ru_field_rec QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD =
{
    "FPM_GRP_HIGHER_THR",
#if RU_INCLUDE_DESC
    "",
    "FPM group Lower Threshold.\n* If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped.\n* If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dropped.\n* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.\n",
#endif
    { QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD_MASK },
    0,
    { QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD_WIDTH },
    { QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_USR_GRP_HIGHER_THR_FIELDS[] =
{
    &QM_FPM_USR_GRP_HIGHER_THR_FPM_GRP_HIGHER_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_USR_GRP_HIGHER_THR *****/
const ru_reg_rec QM_FPM_USR_GRP_HIGHER_THR_REG =
{
    "FPM_USR_GRP_HIGHER_THR",
#if RU_INCLUDE_DESC
    "HIGHER_THR Register",
    "Holds FPM user group higher threshold.\n*IMPORTANT* if buffer reservations is enabled, the following should be honored:\nHIGHER_THR-MID_THR > 16\n",
#endif
    { QM_FPM_USR_GRP_HIGHER_THR_REG_OFFSET },
    QM_FPM_USR_GRP_HIGHER_THR_REG_RAM_CNT,
    32,
    728,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_USR_GRP_HIGHER_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_USR_GRP_CNT, TYPE: Type_QM_TOP_QM_FPM_USR_GRP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_UG_CNT *****/
const ru_field_rec QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD =
{
    "FPM_UG_CNT",
#if RU_INCLUDE_DESC
    "",
    "FPM user group counter\n",
#endif
    { QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD_MASK },
    0,
    { QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD_WIDTH },
    { QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_USR_GRP_CNT_FIELDS[] =
{
    &QM_FPM_USR_GRP_CNT_FPM_UG_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_USR_GRP_CNT *****/
const ru_reg_rec QM_FPM_USR_GRP_CNT_REG =
{
    "FPM_USR_GRP_CNT",
#if RU_INCLUDE_DESC
    "CNT Register",
    "FPM user group buffer counter\n",
#endif
    { QM_FPM_USR_GRP_CNT_REG_OFFSET },
    QM_FPM_USR_GRP_CNT_REG_RAM_CNT,
    32,
    729,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_USR_GRP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_DEBUG_SEL, TYPE: Type_QM_TOP_QM_DEBUG_DEBUG_SEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SELECT *****/
const ru_field_rec QM_DEBUG_SEL_SELECT_FIELD =
{
    "SELECT",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_DEBUG_SEL_SELECT_FIELD_MASK },
    0,
    { QM_DEBUG_SEL_SELECT_FIELD_WIDTH },
    { QM_DEBUG_SEL_SELECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec QM_DEBUG_SEL_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable register controlled debug select\n",
#endif
    { QM_DEBUG_SEL_ENABLE_FIELD_MASK },
    0,
    { QM_DEBUG_SEL_ENABLE_FIELD_WIDTH },
    { QM_DEBUG_SEL_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DEBUG_SEL_FIELDS[] =
{
    &QM_DEBUG_SEL_SELECT_FIELD,
    &QM_DEBUG_SEL_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DEBUG_SEL *****/
const ru_reg_rec QM_DEBUG_SEL_REG =
{
    "DEBUG_SEL",
#if RU_INCLUDE_DESC
    "DEBUG_SEL Register",
    "Controls Debug bus select:\n\n5h1:  qm_dbg_bus = qm_bb_input_dbg_bus;\n5h2:  qm_dbg_bus = qm_bb_output_dbg_bus;\n5h3:  qm_dbg_bus = qm_cm_dbg_bus;\n5h4:  qm_dbg_bus = qm_ddr_write_dbg_bus;\n5h5:  qm_dbg_bus = qm_counters_dbg_bus;\n5h6:  qm_dbg_bus = qm_cpu_if_dbg_bus;\n5h7:  qm_dbg_bus = qm_dqm_push_dbg_bus;\n5h8:  qm_dbg_bus = qm_egress_dbg_bus;\n5h9:  qm_dbg_bus = qm_fpm_prefetch_dbg_bus;\n5ha:  qm_dbg_bus = qm_ingress_dbg_bus;\n5hb:  qm_dbg_bus = qm_rmt_fifos_dbg_bus;\n5hc:  qm_dbg_bus = {19b0,bbh_debug_0};\n5hd:  qm_dbg_bus = {19b0,bbh_debug_1};\n5he:  qm_dbg_bus = {19b0,bbh_debug_2};\n5hf:  qm_dbg_bus = {19b0,bbh_debug_3};\n5h10: qm_dbg_bus = {19b0,bbh_debug_4};\n5h11: qm_dbg_bus = {19b0,bbh_debug_5};\n5h12: qm_dbg_bus = {19b0,bbh_debug_6};\n5h13: qm_dbg_bus = {19b0,bbh_debug_7};\n5h14: qm_dbg_bus = {19b0,bbh_debug_8};\n5h15: qm_dbg_bus = {19b0,bbh_debug_9};\n5h16: qm_dbg_bus = {19b0,dma_debug_vec};\n5h17: qm_dbg_bus = {8b0,dqm_diag_r};\n\n",
#endif
    { QM_DEBUG_SEL_REG_OFFSET },
    0,
    0,
    730,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_DEBUG_SEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_DEBUG_BUS_LSB, TYPE: Type_QM_TOP_QM_DEBUG_DEBUG_BUS_LSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_DEBUG_BUS_LSB_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_DEBUG_BUS_LSB_DATA_FIELD_MASK },
    0,
    { QM_DEBUG_BUS_LSB_DATA_FIELD_WIDTH },
    { QM_DEBUG_BUS_LSB_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DEBUG_BUS_LSB_FIELDS[] =
{
    &QM_DEBUG_BUS_LSB_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DEBUG_BUS_LSB *****/
const ru_reg_rec QM_DEBUG_BUS_LSB_REG =
{
    "DEBUG_BUS_LSB",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_LSB Register",
    "Debug Bus sampling\n",
#endif
    { QM_DEBUG_BUS_LSB_REG_OFFSET },
    0,
    0,
    731,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DEBUG_BUS_LSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_DEBUG_BUS_MSB, TYPE: Type_QM_TOP_QM_DEBUG_DEBUG_BUS_MSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_DEBUG_BUS_MSB_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_DEBUG_BUS_MSB_DATA_FIELD_MASK },
    0,
    { QM_DEBUG_BUS_MSB_DATA_FIELD_WIDTH },
    { QM_DEBUG_BUS_MSB_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DEBUG_BUS_MSB_FIELDS[] =
{
    &QM_DEBUG_BUS_MSB_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DEBUG_BUS_MSB *****/
const ru_reg_rec QM_DEBUG_BUS_MSB_REG =
{
    "DEBUG_BUS_MSB",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_MSB Register",
    "Debug Bus sampling\n",
#endif
    { QM_DEBUG_BUS_MSB_REG_OFFSET },
    0,
    0,
    732,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DEBUG_BUS_MSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_SPARE_CONFIG, TYPE: Type_QM_TOP_QM_DEBUG_QM_SPARE_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_QM_SPARE_CONFIG_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { QM_QM_SPARE_CONFIG_DATA_FIELD_MASK },
    0,
    { QM_QM_SPARE_CONFIG_DATA_FIELD_WIDTH },
    { QM_QM_SPARE_CONFIG_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_SPARE_CONFIG_FIELDS[] =
{
    &QM_QM_SPARE_CONFIG_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_SPARE_CONFIG *****/
const ru_reg_rec QM_QM_SPARE_CONFIG_REG =
{
    "QM_SPARE_CONFIG",
#if RU_INCLUDE_DESC
    "QM_SPARE_CONFIG Register",
    "Spare configuration for ECO purposes\n",
#endif
    { QM_QM_SPARE_CONFIG_REG_OFFSET },
    0,
    0,
    733,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_SPARE_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GOOD_LVL1_PKTS_CNT, TYPE: Type_QM_TOP_QM_DEBUG_GOOD_LVL1_PKTS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD_WIDTH },
    { QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GOOD_LVL1_PKTS_CNT_FIELDS[] =
{
    &QM_GOOD_LVL1_PKTS_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GOOD_LVL1_PKTS_CNT *****/
const ru_reg_rec QM_GOOD_LVL1_PKTS_CNT_REG =
{
    "GOOD_LVL1_PKTS_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL1_PKTS_CNT Register",
    "Counts the total number of non-dropped and non-reprocessing packets from all queues\n",
#endif
    { QM_GOOD_LVL1_PKTS_CNT_REG_OFFSET },
    0,
    0,
    734,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GOOD_LVL1_PKTS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GOOD_LVL1_BYTES_CNT, TYPE: Type_QM_TOP_QM_DEBUG_GOOD_LVL1_BYTES_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD_WIDTH },
    { QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GOOD_LVL1_BYTES_CNT_FIELDS[] =
{
    &QM_GOOD_LVL1_BYTES_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GOOD_LVL1_BYTES_CNT *****/
const ru_reg_rec QM_GOOD_LVL1_BYTES_CNT_REG =
{
    "GOOD_LVL1_BYTES_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL1_BYTES_CNT Register",
    "Counts the total number of non-dropped and non-reprocessing bytes from all queues\n",
#endif
    { QM_GOOD_LVL1_BYTES_CNT_REG_OFFSET },
    0,
    0,
    735,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GOOD_LVL1_BYTES_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GOOD_LVL2_PKTS_CNT, TYPE: Type_QM_TOP_QM_DEBUG_GOOD_LVL2_PKTS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD_WIDTH },
    { QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GOOD_LVL2_PKTS_CNT_FIELDS[] =
{
    &QM_GOOD_LVL2_PKTS_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GOOD_LVL2_PKTS_CNT *****/
const ru_reg_rec QM_GOOD_LVL2_PKTS_CNT_REG =
{
    "GOOD_LVL2_PKTS_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL2_PKTS_CNT Register",
    "Counts the total number of non-dropped and reprocessing packets from all queues\n",
#endif
    { QM_GOOD_LVL2_PKTS_CNT_REG_OFFSET },
    0,
    0,
    736,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GOOD_LVL2_PKTS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GOOD_LVL2_BYTES_CNT, TYPE: Type_QM_TOP_QM_DEBUG_GOOD_LVL2_BYTES_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD_WIDTH },
    { QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GOOD_LVL2_BYTES_CNT_FIELDS[] =
{
    &QM_GOOD_LVL2_BYTES_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GOOD_LVL2_BYTES_CNT *****/
const ru_reg_rec QM_GOOD_LVL2_BYTES_CNT_REG =
{
    "GOOD_LVL2_BYTES_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL2_BYTES_CNT Register",
    "Counts the total number of non-dropped and reprocessing bytes from all queues\n",
#endif
    { QM_GOOD_LVL2_BYTES_CNT_REG_OFFSET },
    0,
    0,
    737,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GOOD_LVL2_BYTES_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_COPIED_PKTS_CNT, TYPE: Type_QM_TOP_QM_DEBUG_COPIED_PKTS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_COPIED_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_COPIED_PKTS_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_COPIED_PKTS_CNT_COUNTER_FIELD_WIDTH },
    { QM_COPIED_PKTS_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_COPIED_PKTS_CNT_FIELDS[] =
{
    &QM_COPIED_PKTS_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_COPIED_PKTS_CNT *****/
const ru_reg_rec QM_COPIED_PKTS_CNT_REG =
{
    "COPIED_PKTS_CNT",
#if RU_INCLUDE_DESC
    "COPIED_PKTS_CNT Register",
    "Counts the total number of copied packets to the DDR from all queues\n",
#endif
    { QM_COPIED_PKTS_CNT_REG_OFFSET },
    0,
    0,
    738,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_COPIED_PKTS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_COPIED_BYTES_CNT, TYPE: Type_QM_TOP_QM_DEBUG_COPIED_BYTES_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_COPIED_BYTES_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_COPIED_BYTES_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_COPIED_BYTES_CNT_COUNTER_FIELD_WIDTH },
    { QM_COPIED_BYTES_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_COPIED_BYTES_CNT_FIELDS[] =
{
    &QM_COPIED_BYTES_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_COPIED_BYTES_CNT *****/
const ru_reg_rec QM_COPIED_BYTES_CNT_REG =
{
    "COPIED_BYTES_CNT",
#if RU_INCLUDE_DESC
    "COPIED_BYTES_CNT Register",
    "Counts the total number of copied bytes to the DDR from all queues\n",
#endif
    { QM_COPIED_BYTES_CNT_REG_OFFSET },
    0,
    0,
    739,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_COPIED_BYTES_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_AGG_PKTS_CNT, TYPE: Type_QM_TOP_QM_DEBUG_AGG_PKTS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_AGG_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_AGG_PKTS_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_AGG_PKTS_CNT_COUNTER_FIELD_WIDTH },
    { QM_AGG_PKTS_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_PKTS_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_AGG_PKTS_CNT *****/
const ru_reg_rec QM_AGG_PKTS_CNT_REG =
{
    "AGG_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_PKTS_CNT Register",
    "Counts the total number of aggregated packets from all queues\n",
#endif
    { QM_AGG_PKTS_CNT_REG_OFFSET },
    0,
    0,
    740,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_PKTS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_AGG_BYTES_CNT, TYPE: Type_QM_TOP_QM_DEBUG_AGG_BYTES_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_AGG_BYTES_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_AGG_BYTES_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_AGG_BYTES_CNT_COUNTER_FIELD_WIDTH },
    { QM_AGG_BYTES_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_BYTES_CNT_FIELDS[] =
{
    &QM_AGG_BYTES_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_AGG_BYTES_CNT *****/
const ru_reg_rec QM_AGG_BYTES_CNT_REG =
{
    "AGG_BYTES_CNT",
#if RU_INCLUDE_DESC
    "AGG_BYTES_CNT Register",
    "Counts the total number of aggregated bytes from all queues\n",
#endif
    { QM_AGG_BYTES_CNT_REG_OFFSET },
    0,
    0,
    741,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_BYTES_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_AGG_1_PKTS_CNT, TYPE: Type_QM_TOP_QM_DEBUG_AGG_1_PKTS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_AGG_1_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_AGG_1_PKTS_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_AGG_1_PKTS_CNT_COUNTER_FIELD_WIDTH },
    { QM_AGG_1_PKTS_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_1_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_1_PKTS_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_AGG_1_PKTS_CNT *****/
const ru_reg_rec QM_AGG_1_PKTS_CNT_REG =
{
    "AGG_1_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_1_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 1-packet PD from all queues\n",
#endif
    { QM_AGG_1_PKTS_CNT_REG_OFFSET },
    0,
    0,
    742,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_1_PKTS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_AGG_2_PKTS_CNT, TYPE: Type_QM_TOP_QM_DEBUG_AGG_2_PKTS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_AGG_2_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_AGG_2_PKTS_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_AGG_2_PKTS_CNT_COUNTER_FIELD_WIDTH },
    { QM_AGG_2_PKTS_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_2_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_2_PKTS_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_AGG_2_PKTS_CNT *****/
const ru_reg_rec QM_AGG_2_PKTS_CNT_REG =
{
    "AGG_2_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_2_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 2-packet PD from all queues\n",
#endif
    { QM_AGG_2_PKTS_CNT_REG_OFFSET },
    0,
    0,
    743,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_2_PKTS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_AGG_3_PKTS_CNT, TYPE: Type_QM_TOP_QM_DEBUG_AGG_3_PKTS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_AGG_3_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_AGG_3_PKTS_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_AGG_3_PKTS_CNT_COUNTER_FIELD_WIDTH },
    { QM_AGG_3_PKTS_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_3_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_3_PKTS_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_AGG_3_PKTS_CNT *****/
const ru_reg_rec QM_AGG_3_PKTS_CNT_REG =
{
    "AGG_3_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_3_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 3-packet PD from all queues\n",
#endif
    { QM_AGG_3_PKTS_CNT_REG_OFFSET },
    0,
    0,
    744,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_3_PKTS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_AGG_4_PKTS_CNT, TYPE: Type_QM_TOP_QM_DEBUG_AGG_4_PKTS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_AGG_4_PKTS_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_AGG_4_PKTS_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_AGG_4_PKTS_CNT_COUNTER_FIELD_WIDTH },
    { QM_AGG_4_PKTS_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AGG_4_PKTS_CNT_FIELDS[] =
{
    &QM_AGG_4_PKTS_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_AGG_4_PKTS_CNT *****/
const ru_reg_rec QM_AGG_4_PKTS_CNT_REG =
{
    "AGG_4_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_4_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 4-packet PD from all queues\n",
#endif
    { QM_AGG_4_PKTS_CNT_REG_OFFSET },
    0,
    0,
    745,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AGG_4_PKTS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_WRED_DROP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_WRED_DROP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_WRED_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_WRED_DROP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_WRED_DROP_CNT_COUNTER_FIELD_WIDTH },
    { QM_WRED_DROP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_WRED_DROP_CNT_FIELDS[] =
{
    &QM_WRED_DROP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_WRED_DROP_CNT *****/
const ru_reg_rec QM_WRED_DROP_CNT_REG =
{
    "WRED_DROP_CNT",
#if RU_INCLUDE_DESC
    "WRED_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to WRED\n",
#endif
    { QM_WRED_DROP_CNT_REG_OFFSET },
    0,
    0,
    746,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_WRED_DROP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_CONGESTION_DROP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_FPM_CONGESTION_DROP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD_WIDTH },
    { QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_CONGESTION_DROP_CNT_FIELDS[] =
{
    &QM_FPM_CONGESTION_DROP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_CONGESTION_DROP_CNT *****/
const ru_reg_rec QM_FPM_CONGESTION_DROP_CNT_REG =
{
    "FPM_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to FPM congestion indication\n",
#endif
    { QM_FPM_CONGESTION_DROP_CNT_REG_OFFSET },
    0,
    0,
    747,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_CONGESTION_DROP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_DDR_PD_CONGESTION_DROP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_DDR_PD_CONGESTION_DROP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_WIDTH },
    { QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DDR_PD_CONGESTION_DROP_CNT_FIELDS[] =
{
    &QM_DDR_PD_CONGESTION_DROP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DDR_PD_CONGESTION_DROP_CNT *****/
const ru_reg_rec QM_DDR_PD_CONGESTION_DROP_CNT_REG =
{
    "DDR_PD_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to DDR PD congestion\n",
#endif
    { QM_DDR_PD_CONGESTION_DROP_CNT_REG_OFFSET },
    0,
    0,
    748,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DDR_PD_CONGESTION_DROP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_DDR_BYTE_CONGESTION_DROP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_DDR_BYTE_CONGESTION_DROP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD_WIDTH },
    { QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_DDR_BYTE_CONGESTION_DROP_CNT_FIELDS[] =
{
    &QM_DDR_BYTE_CONGESTION_DROP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_DDR_BYTE_CONGESTION_DROP_CNT *****/
const ru_reg_rec QM_DDR_BYTE_CONGESTION_DROP_CNT_REG =
{
    "DDR_BYTE_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to DDR byte congestion (number of bytes waiting to be copied exceeded the thresholds)\n",
#endif
    { QM_DDR_BYTE_CONGESTION_DROP_CNT_REG_OFFSET },
    0,
    0,
    749,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_DDR_BYTE_CONGESTION_DROP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_PD_CONGESTION_DROP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_QM_PD_CONGESTION_DROP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_WIDTH },
    { QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_PD_CONGESTION_DROP_CNT_FIELDS[] =
{
    &QM_QM_PD_CONGESTION_DROP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_PD_CONGESTION_DROP_CNT *****/
const ru_reg_rec QM_QM_PD_CONGESTION_DROP_CNT_REG =
{
    "QM_PD_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "QM_PD_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to QM PD congestion (this value is limited by the DQM)\n",
#endif
    { QM_QM_PD_CONGESTION_DROP_CNT_REG_OFFSET },
    0,
    0,
    750,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_PD_CONGESTION_DROP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_ABS_REQUEUE_CNT, TYPE: Type_QM_TOP_QM_DEBUG_QM_ABS_REQUEUE_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD_WIDTH },
    { QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_ABS_REQUEUE_CNT_FIELDS[] =
{
    &QM_QM_ABS_REQUEUE_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_ABS_REQUEUE_CNT *****/
const ru_reg_rec QM_QM_ABS_REQUEUE_CNT_REG =
{
    "QM_ABS_REQUEUE_CNT",
#if RU_INCLUDE_DESC
    "QM_ABS_REQUEUE_CNT Register",
    "Counts the total number of packets requeued due to absolute address drops from all queues\n",
#endif
    { QM_QM_ABS_REQUEUE_CNT_REG_OFFSET },
    0,
    0,
    751,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_ABS_REQUEUE_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_PREFETCH_FIFO0_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_FPM_PREFETCH_FIFO0_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_PREFETCH_FIFO0_STATUS_FIELDS[] =
{
    &QM_FPM_PREFETCH_FIFO0_STATUS_USED_WORDS_FIELD,
    &QM_FPM_PREFETCH_FIFO0_STATUS_EMPTY_FIELD,
    &QM_FPM_PREFETCH_FIFO0_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_PREFETCH_FIFO0_STATUS *****/
const ru_reg_rec QM_FPM_PREFETCH_FIFO0_STATUS_REG =
{
    "FPM_PREFETCH_FIFO0_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO0_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_FPM_PREFETCH_FIFO0_STATUS_REG_OFFSET },
    0,
    0,
    752,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_FPM_PREFETCH_FIFO0_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_PREFETCH_FIFO1_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_FPM_PREFETCH_FIFO1_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_PREFETCH_FIFO1_STATUS_FIELDS[] =
{
    &QM_FPM_PREFETCH_FIFO1_STATUS_USED_WORDS_FIELD,
    &QM_FPM_PREFETCH_FIFO1_STATUS_EMPTY_FIELD,
    &QM_FPM_PREFETCH_FIFO1_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_PREFETCH_FIFO1_STATUS *****/
const ru_reg_rec QM_FPM_PREFETCH_FIFO1_STATUS_REG =
{
    "FPM_PREFETCH_FIFO1_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO1_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_FPM_PREFETCH_FIFO1_STATUS_REG_OFFSET },
    0,
    0,
    753,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_FPM_PREFETCH_FIFO1_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_PREFETCH_FIFO2_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_FPM_PREFETCH_FIFO2_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_PREFETCH_FIFO2_STATUS_FIELDS[] =
{
    &QM_FPM_PREFETCH_FIFO2_STATUS_USED_WORDS_FIELD,
    &QM_FPM_PREFETCH_FIFO2_STATUS_EMPTY_FIELD,
    &QM_FPM_PREFETCH_FIFO2_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_PREFETCH_FIFO2_STATUS *****/
const ru_reg_rec QM_FPM_PREFETCH_FIFO2_STATUS_REG =
{
    "FPM_PREFETCH_FIFO2_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO2_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_FPM_PREFETCH_FIFO2_STATUS_REG_OFFSET },
    0,
    0,
    754,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_FPM_PREFETCH_FIFO2_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_PREFETCH_FIFO3_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_FPM_PREFETCH_FIFO3_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD_MASK },
    0,
    { QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD_WIDTH },
    { QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_PREFETCH_FIFO3_STATUS_FIELDS[] =
{
    &QM_FPM_PREFETCH_FIFO3_STATUS_USED_WORDS_FIELD,
    &QM_FPM_PREFETCH_FIFO3_STATUS_EMPTY_FIELD,
    &QM_FPM_PREFETCH_FIFO3_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_PREFETCH_FIFO3_STATUS *****/
const ru_reg_rec QM_FPM_PREFETCH_FIFO3_STATUS_REG =
{
    "FPM_PREFETCH_FIFO3_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO3_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_FPM_PREFETCH_FIFO3_STATUS_REG_OFFSET },
    0,
    0,
    755,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_FPM_PREFETCH_FIFO3_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_NORMAL_RMT_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_NORMAL_RMT_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NORMAL_RMT_FIFO_STATUS_FIELDS[] =
{
    &QM_NORMAL_RMT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_NORMAL_RMT_FIFO_STATUS_EMPTY_FIELD,
    &QM_NORMAL_RMT_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_NORMAL_RMT_FIFO_STATUS *****/
const ru_reg_rec QM_NORMAL_RMT_FIFO_STATUS_REG =
{
    "NORMAL_RMT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NORMAL_RMT_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_NORMAL_RMT_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    756,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_NORMAL_RMT_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_NON_DELAYED_RMT_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_NON_DELAYED_RMT_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NON_DELAYED_RMT_FIFO_STATUS_FIELDS[] =
{
    &QM_NON_DELAYED_RMT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_NON_DELAYED_RMT_FIFO_STATUS_EMPTY_FIELD,
    &QM_NON_DELAYED_RMT_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_NON_DELAYED_RMT_FIFO_STATUS *****/
const ru_reg_rec QM_NON_DELAYED_RMT_FIFO_STATUS_REG =
{
    "NON_DELAYED_RMT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NON_DELAYED_RMT_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_NON_DELAYED_RMT_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    757,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_NON_DELAYED_RMT_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_NON_DELAYED_OUT_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_NON_DELAYED_OUT_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NON_DELAYED_OUT_FIFO_STATUS_FIELDS[] =
{
    &QM_NON_DELAYED_OUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_NON_DELAYED_OUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_NON_DELAYED_OUT_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_NON_DELAYED_OUT_FIFO_STATUS *****/
const ru_reg_rec QM_NON_DELAYED_OUT_FIFO_STATUS_REG =
{
    "NON_DELAYED_OUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NON_DELAYED_OUT_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_NON_DELAYED_OUT_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    758,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_NON_DELAYED_OUT_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_PRE_CM_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_PRE_CM_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_PRE_CM_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_PRE_CM_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_PRE_CM_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_PRE_CM_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_PRE_CM_FIFO_STATUS_FIELDS[] =
{
    &QM_PRE_CM_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_PRE_CM_FIFO_STATUS_EMPTY_FIELD,
    &QM_PRE_CM_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_PRE_CM_FIFO_STATUS *****/
const ru_reg_rec QM_PRE_CM_FIFO_STATUS_REG =
{
    "PRE_CM_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PRE_CM_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_PRE_CM_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    759,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_PRE_CM_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CM_RD_PD_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_CM_RD_PD_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CM_RD_PD_FIFO_STATUS_FIELDS[] =
{
    &QM_CM_RD_PD_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_CM_RD_PD_FIFO_STATUS_EMPTY_FIELD,
    &QM_CM_RD_PD_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CM_RD_PD_FIFO_STATUS *****/
const ru_reg_rec QM_CM_RD_PD_FIFO_STATUS_REG =
{
    "CM_RD_PD_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_RD_PD_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_CM_RD_PD_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    760,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_CM_RD_PD_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CM_WR_PD_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_CM_WR_PD_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CM_WR_PD_FIFO_STATUS_FIELDS[] =
{
    &QM_CM_WR_PD_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_CM_WR_PD_FIFO_STATUS_EMPTY_FIELD,
    &QM_CM_WR_PD_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CM_WR_PD_FIFO_STATUS *****/
const ru_reg_rec QM_CM_WR_PD_FIFO_STATUS_REG =
{
    "CM_WR_PD_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_WR_PD_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_CM_WR_PD_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    761,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_CM_WR_PD_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CM_COMMON_INPUT_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_CM_COMMON_INPUT_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CM_COMMON_INPUT_FIFO_STATUS_FIELDS[] =
{
    &QM_CM_COMMON_INPUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_CM_COMMON_INPUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_CM_COMMON_INPUT_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CM_COMMON_INPUT_FIFO_STATUS *****/
const ru_reg_rec QM_CM_COMMON_INPUT_FIFO_STATUS_REG =
{
    "CM_COMMON_INPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_COMMON_INPUT_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_CM_COMMON_INPUT_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    762,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_CM_COMMON_INPUT_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_BB0_OUTPUT_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_BB0_OUTPUT_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB0_OUTPUT_FIFO_STATUS_FIELDS[] =
{
    &QM_BB0_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_BB0_OUTPUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_BB0_OUTPUT_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_BB0_OUTPUT_FIFO_STATUS *****/
const ru_reg_rec QM_BB0_OUTPUT_FIFO_STATUS_REG =
{
    "BB0_OUTPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB0_OUTPUT_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_BB0_OUTPUT_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    763,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_BB0_OUTPUT_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_BB1_OUTPUT_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_BB1_OUTPUT_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB1_OUTPUT_FIFO_STATUS_FIELDS[] =
{
    &QM_BB1_OUTPUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_BB1_OUTPUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_BB1_OUTPUT_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_BB1_OUTPUT_FIFO_STATUS *****/
const ru_reg_rec QM_BB1_OUTPUT_FIFO_STATUS_REG =
{
    "BB1_OUTPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB1_OUTPUT_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_BB1_OUTPUT_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    764,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_BB1_OUTPUT_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_BB1_INPUT_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_BB1_INPUT_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB1_INPUT_FIFO_STATUS_FIELDS[] =
{
    &QM_BB1_INPUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_BB1_INPUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_BB1_INPUT_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_BB1_INPUT_FIFO_STATUS *****/
const ru_reg_rec QM_BB1_INPUT_FIFO_STATUS_REG =
{
    "BB1_INPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB1_INPUT_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_BB1_INPUT_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    765,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_BB1_INPUT_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_EGRESS_DATA_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_EGRESS_DATA_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_DATA_FIFO_STATUS_FIELDS[] =
{
    &QM_EGRESS_DATA_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_EGRESS_DATA_FIFO_STATUS_EMPTY_FIELD,
    &QM_EGRESS_DATA_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_EGRESS_DATA_FIFO_STATUS *****/
const ru_reg_rec QM_EGRESS_DATA_FIFO_STATUS_REG =
{
    "EGRESS_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "EGRESS_DATA_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_EGRESS_DATA_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    766,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_EGRESS_DATA_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_EGRESS_RR_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_EGRESS_RR_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_RR_FIFO_STATUS_FIELDS[] =
{
    &QM_EGRESS_RR_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_EGRESS_RR_FIFO_STATUS_EMPTY_FIELD,
    &QM_EGRESS_RR_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_EGRESS_RR_FIFO_STATUS *****/
const ru_reg_rec QM_EGRESS_RR_FIFO_STATUS_REG =
{
    "EGRESS_RR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "EGRESS_RR_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_EGRESS_RR_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    767,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_EGRESS_RR_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_BB_ROUTE_OVR, TYPE: Type_QM_TOP_QM_DEBUG_BB_ROUTE_OVR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OVR_EN *****/
const ru_field_rec QM_BB_ROUTE_OVR_OVR_EN_FIELD =
{
    "OVR_EN",
#if RU_INCLUDE_DESC
    "",
    "BB rout address decode Override enable\n",
#endif
    { QM_BB_ROUTE_OVR_OVR_EN_FIELD_MASK },
    0,
    { QM_BB_ROUTE_OVR_OVR_EN_FIELD_WIDTH },
    { QM_BB_ROUTE_OVR_OVR_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST_ID *****/
const ru_field_rec QM_BB_ROUTE_OVR_DEST_ID_FIELD =
{
    "DEST_ID",
#if RU_INCLUDE_DESC
    "",
    "Destination ID\n",
#endif
    { QM_BB_ROUTE_OVR_DEST_ID_FIELD_MASK },
    0,
    { QM_BB_ROUTE_OVR_DEST_ID_FIELD_WIDTH },
    { QM_BB_ROUTE_OVR_DEST_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE_ADDR *****/
const ru_field_rec QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD =
{
    "ROUTE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Route Address\n",
#endif
    { QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD_MASK },
    0,
    { QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD_WIDTH },
    { QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB_ROUTE_OVR_FIELDS[] =
{
    &QM_BB_ROUTE_OVR_OVR_EN_FIELD,
    &QM_BB_ROUTE_OVR_DEST_ID_FIELD,
    &QM_BB_ROUTE_OVR_ROUTE_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_BB_ROUTE_OVR *****/
const ru_reg_rec QM_BB_ROUTE_OVR_REG =
{
    "BB_ROUTE_OVR",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVR 0..2 Register",
    "BB ROUTE Override:\n0 - for QM_TOP\n1 - for RNR_GRID\n",
#endif
    { QM_BB_ROUTE_OVR_REG_OFFSET },
    QM_BB_ROUTE_OVR_REG_RAM_CNT,
    4,
    768,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_BB_ROUTE_OVR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_INGRESS_STAT, TYPE: Type_QM_TOP_QM_DEBUG_QM_INGRESS_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STAT *****/
const ru_field_rec QM_QM_INGRESS_STAT_STAT_FIELD =
{
    "STAT",
#if RU_INCLUDE_DESC
    "",
    "Stat\n",
#endif
    { QM_QM_INGRESS_STAT_STAT_FIELD_MASK },
    0,
    { QM_QM_INGRESS_STAT_STAT_FIELD_WIDTH },
    { QM_QM_INGRESS_STAT_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_INGRESS_STAT_FIELDS[] =
{
    &QM_QM_INGRESS_STAT_STAT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_INGRESS_STAT *****/
const ru_reg_rec QM_QM_INGRESS_STAT_REG =
{
    "QM_INGRESS_STAT",
#if RU_INCLUDE_DESC
    "QM_INGRESS_STAT Register",
    "Holds the Ingress Status\n",
#endif
    { QM_QM_INGRESS_STAT_REG_OFFSET },
    0,
    0,
    769,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_INGRESS_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_EGRESS_STAT, TYPE: Type_QM_TOP_QM_DEBUG_QM_EGRESS_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STAT *****/
const ru_field_rec QM_QM_EGRESS_STAT_STAT_FIELD =
{
    "STAT",
#if RU_INCLUDE_DESC
    "",
    "Stat\n",
#endif
    { QM_QM_EGRESS_STAT_STAT_FIELD_MASK },
    0,
    { QM_QM_EGRESS_STAT_STAT_FIELD_WIDTH },
    { QM_QM_EGRESS_STAT_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_EGRESS_STAT_FIELDS[] =
{
    &QM_QM_EGRESS_STAT_STAT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_EGRESS_STAT *****/
const ru_reg_rec QM_QM_EGRESS_STAT_REG =
{
    "QM_EGRESS_STAT",
#if RU_INCLUDE_DESC
    "QM_EGRESS_STAT Register",
    "Holds the Egress Status\n",
#endif
    { QM_QM_EGRESS_STAT_REG_OFFSET },
    0,
    0,
    770,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_EGRESS_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_CM_STAT, TYPE: Type_QM_TOP_QM_DEBUG_QM_CM_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STAT *****/
const ru_field_rec QM_QM_CM_STAT_STAT_FIELD =
{
    "STAT",
#if RU_INCLUDE_DESC
    "",
    "Stat\n",
#endif
    { QM_QM_CM_STAT_STAT_FIELD_MASK },
    0,
    { QM_QM_CM_STAT_STAT_FIELD_WIDTH },
    { QM_QM_CM_STAT_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_CM_STAT_FIELDS[] =
{
    &QM_QM_CM_STAT_STAT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_CM_STAT *****/
const ru_reg_rec QM_QM_CM_STAT_REG =
{
    "QM_CM_STAT",
#if RU_INCLUDE_DESC
    "QM_CM_STAT Register",
    "Holds the CM Status\n",
#endif
    { QM_QM_CM_STAT_REG_OFFSET },
    0,
    0,
    771,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_CM_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_FPM_PREFETCH_STAT, TYPE: Type_QM_TOP_QM_DEBUG_QM_FPM_PREFETCH_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STAT *****/
const ru_field_rec QM_QM_FPM_PREFETCH_STAT_STAT_FIELD =
{
    "STAT",
#if RU_INCLUDE_DESC
    "",
    "Stat\n",
#endif
    { QM_QM_FPM_PREFETCH_STAT_STAT_FIELD_MASK },
    0,
    { QM_QM_FPM_PREFETCH_STAT_STAT_FIELD_WIDTH },
    { QM_QM_FPM_PREFETCH_STAT_STAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_FPM_PREFETCH_STAT_FIELDS[] =
{
    &QM_QM_FPM_PREFETCH_STAT_STAT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_FPM_PREFETCH_STAT *****/
const ru_reg_rec QM_QM_FPM_PREFETCH_STAT_REG =
{
    "QM_FPM_PREFETCH_STAT",
#if RU_INCLUDE_DESC
    "QM_FPM_PREFETCH_STAT Register",
    "Holds the FPM Prefetch Status\n",
#endif
    { QM_QM_FPM_PREFETCH_STAT_REG_OFFSET },
    0,
    0,
    772,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_FPM_PREFETCH_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_CONNECT_ACK_COUNTER, TYPE: Type_QM_TOP_QM_DEBUG_QM_CONNECT_ACK_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONNECT_ACK_COUNTER *****/
const ru_field_rec QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD =
{
    "CONNECT_ACK_COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Pending SBPM Connect ACKs counter\n",
#endif
    { QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD_MASK },
    0,
    { QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD_WIDTH },
    { QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_CONNECT_ACK_COUNTER_FIELDS[] =
{
    &QM_QM_CONNECT_ACK_COUNTER_CONNECT_ACK_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_CONNECT_ACK_COUNTER *****/
const ru_reg_rec QM_QM_CONNECT_ACK_COUNTER_REG =
{
    "QM_CONNECT_ACK_COUNTER",
#if RU_INCLUDE_DESC
    "QM_CONNECT_ACK_COUNTER Register",
    "QM connect ack counter\n",
#endif
    { QM_QM_CONNECT_ACK_COUNTER_REG_OFFSET },
    0,
    0,
    773,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_CONNECT_ACK_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_DDR_WR_REPLY_COUNTER, TYPE: Type_QM_TOP_QM_DEBUG_QM_DDR_WR_REPLY_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_WR_REPLY_COUNTER *****/
const ru_field_rec QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD =
{
    "DDR_WR_REPLY_COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Pending DDR WR Replies counter\n",
#endif
    { QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD_MASK },
    0,
    { QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD_WIDTH },
    { QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_DDR_WR_REPLY_COUNTER_FIELDS[] =
{
    &QM_QM_DDR_WR_REPLY_COUNTER_DDR_WR_REPLY_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_DDR_WR_REPLY_COUNTER *****/
const ru_reg_rec QM_QM_DDR_WR_REPLY_COUNTER_REG =
{
    "QM_DDR_WR_REPLY_COUNTER",
#if RU_INCLUDE_DESC
    "QM_DDR_WR_REPLY_COUNTER Register",
    "QM DDR WR reply Counter\n",
#endif
    { QM_QM_DDR_WR_REPLY_COUNTER_REG_OFFSET },
    0,
    0,
    774,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_DDR_WR_REPLY_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_DDR_PIPE_BYTE_COUNTER, TYPE: Type_QM_TOP_QM_DEBUG_QM_DDR_PIPE_BYTE_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Pending bytes to be copied to the DDR\n",
#endif
    { QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD_MASK },
    0,
    { QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD_WIDTH },
    { QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_DDR_PIPE_BYTE_COUNTER_FIELDS[] =
{
    &QM_QM_DDR_PIPE_BYTE_COUNTER_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_DDR_PIPE_BYTE_COUNTER *****/
const ru_reg_rec QM_QM_DDR_PIPE_BYTE_COUNTER_REG =
{
    "QM_DDR_PIPE_BYTE_COUNTER",
#if RU_INCLUDE_DESC
    "QM_DDR_PIPE_BYTE_COUNTER Register",
    "QM DDR pipe byte counter\n",
#endif
    { QM_QM_DDR_PIPE_BYTE_COUNTER_REG_OFFSET },
    0,
    0,
    775,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_DDR_PIPE_BYTE_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_ABS_REQUEUE_VALID_COUNTER, TYPE: Type_QM_TOP_QM_DEBUG_QM_ABS_REQUEUE_VALID_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD_MASK },
    0,
    { QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD_WIDTH },
    { QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_ABS_REQUEUE_VALID_COUNTER_FIELDS[] =
{
    &QM_QM_ABS_REQUEUE_VALID_COUNTER_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_ABS_REQUEUE_VALID_COUNTER *****/
const ru_reg_rec QM_QM_ABS_REQUEUE_VALID_COUNTER_REG =
{
    "QM_ABS_REQUEUE_VALID_COUNTER",
#if RU_INCLUDE_DESC
    "QM_ABS_REQUEUE_VALID_COUNTER Register",
    "Indicates the number of PDs currently in the Absolute address drop queue.\n",
#endif
    { QM_QM_ABS_REQUEUE_VALID_COUNTER_REG_OFFSET },
    0,
    0,
    776,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_ABS_REQUEUE_VALID_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_ILLEGAL_PD_CAPTURE, TYPE: Type_QM_TOP_QM_DEBUG_QM_ILLEGAL_PD_CAPTURE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PD *****/
const ru_field_rec QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD =
{
    "PD",
#if RU_INCLUDE_DESC
    "",
    "PD\n",
#endif
    { QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD_MASK },
    0,
    { QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD_WIDTH },
    { QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_ILLEGAL_PD_CAPTURE_FIELDS[] =
{
    &QM_QM_ILLEGAL_PD_CAPTURE_PD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_ILLEGAL_PD_CAPTURE *****/
const ru_reg_rec QM_QM_ILLEGAL_PD_CAPTURE_REG =
{
    "QM_ILLEGAL_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_ILLEGAL_PD_CAPTURE 0..3 Register",
    "PD captured when an illegal PD was detected and the relevant interrupt was generated.\n",
#endif
    { QM_QM_ILLEGAL_PD_CAPTURE_REG_OFFSET },
    QM_QM_ILLEGAL_PD_CAPTURE_REG_RAM_CNT,
    4,
    777,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_ILLEGAL_PD_CAPTURE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_INGRESS_PROCESSED_PD_CAPTURE, TYPE: Type_QM_TOP_QM_DEBUG_QM_INGRESS_PROCESSED_PD_CAPTURE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PD *****/
const ru_field_rec QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD =
{
    "PD",
#if RU_INCLUDE_DESC
    "",
    "PD\n",
#endif
    { QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD_MASK },
    0,
    { QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD_WIDTH },
    { QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_INGRESS_PROCESSED_PD_CAPTURE_FIELDS[] =
{
    &QM_QM_INGRESS_PROCESSED_PD_CAPTURE_PD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_INGRESS_PROCESSED_PD_CAPTURE *****/
const ru_reg_rec QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG =
{
    "QM_INGRESS_PROCESSED_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_INGRESS_PROCESSED_PD_CAPTURE 0..3 Register",
    "Last ingress processed PD capture\n",
#endif
    { QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG_OFFSET },
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG_RAM_CNT,
    4,
    778,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_QM_CM_PROCESSED_PD_CAPTURE, TYPE: Type_QM_TOP_QM_DEBUG_QM_CM_PROCESSED_PD_CAPTURE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PD *****/
const ru_field_rec QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD =
{
    "PD",
#if RU_INCLUDE_DESC
    "",
    "PD\n",
#endif
    { QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD_MASK },
    0,
    { QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD_WIDTH },
    { QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_QM_CM_PROCESSED_PD_CAPTURE_FIELDS[] =
{
    &QM_QM_CM_PROCESSED_PD_CAPTURE_PD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_QM_CM_PROCESSED_PD_CAPTURE *****/
const ru_reg_rec QM_QM_CM_PROCESSED_PD_CAPTURE_REG =
{
    "QM_CM_PROCESSED_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_CM_PROCESSED_PD_CAPTURE 0..3 Register",
    "Last copy machine processed PD capture\n",
#endif
    { QM_QM_CM_PROCESSED_PD_CAPTURE_REG_OFFSET },
    QM_QM_CM_PROCESSED_PD_CAPTURE_REG_RAM_CNT,
    4,
    779,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_QM_CM_PROCESSED_PD_CAPTURE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_GRP_DROP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_FPM_GRP_DROP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_FPM_GRP_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_FPM_GRP_DROP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_FPM_GRP_DROP_CNT_COUNTER_FIELD_WIDTH },
    { QM_FPM_GRP_DROP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_GRP_DROP_CNT_FIELDS[] =
{
    &QM_FPM_GRP_DROP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_GRP_DROP_CNT *****/
const ru_reg_rec QM_FPM_GRP_DROP_CNT_REG =
{
    "FPM_GRP_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_GRP_DROP_CNT 0..31 Register",
    "Counts the total number of packets dropped from all queues due to FPM user group priority thresholds. Counter per UG/BUFMNG (0-31)\n",
#endif
    { QM_FPM_GRP_DROP_CNT_REG_OFFSET },
    QM_FPM_GRP_DROP_CNT_REG_RAM_CNT,
    4,
    780,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_GRP_DROP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_POOL_DROP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_FPM_POOL_DROP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_FPM_POOL_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_FPM_POOL_DROP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_FPM_POOL_DROP_CNT_COUNTER_FIELD_WIDTH },
    { QM_FPM_POOL_DROP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_POOL_DROP_CNT_FIELDS[] =
{
    &QM_FPM_POOL_DROP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_POOL_DROP_CNT *****/
const ru_reg_rec QM_FPM_POOL_DROP_CNT_REG =
{
    "FPM_POOL_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_POOL_DROP_CNT 0..3 Register",
    "Counts the total number of packets dropped for all queues due to FPM pool priority thresholds. Counter per pool\n",
#endif
    { QM_FPM_POOL_DROP_CNT_REG_OFFSET },
    QM_FPM_POOL_DROP_CNT_REG_RAM_CNT,
    4,
    781,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_POOL_DROP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_FPM_BUFFER_RES_DROP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_FPM_BUFFER_RES_DROP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD_WIDTH },
    { QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_FPM_BUFFER_RES_DROP_CNT_FIELDS[] =
{
    &QM_FPM_BUFFER_RES_DROP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_FPM_BUFFER_RES_DROP_CNT *****/
const ru_reg_rec QM_FPM_BUFFER_RES_DROP_CNT_REG =
{
    "FPM_BUFFER_RES_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_BUFFER_RES_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to buffer reservation mechanism.\n",
#endif
    { QM_FPM_BUFFER_RES_DROP_CNT_REG_OFFSET },
    0,
    0,
    782,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_FPM_BUFFER_RES_DROP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_PSRAM_EGRESS_CONG_DRP_CNT, TYPE: Type_QM_TOP_QM_DEBUG_PSRAM_EGRESS_CONG_DRP_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER *****/
const ru_field_rec QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD =
{
    "COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter\n",
#endif
    { QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD_MASK },
    0,
    { QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD_WIDTH },
    { QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_PSRAM_EGRESS_CONG_DRP_CNT_FIELDS[] =
{
    &QM_PSRAM_EGRESS_CONG_DRP_CNT_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_PSRAM_EGRESS_CONG_DRP_CNT *****/
const ru_reg_rec QM_PSRAM_EGRESS_CONG_DRP_CNT_REG =
{
    "PSRAM_EGRESS_CONG_DRP_CNT",
#if RU_INCLUDE_DESC
    "PSRAM_EGRESS_CONG_DRP_CNT Register",
    "Counts the total number of packets dropped from all queues due to psram egress congestion.\n",
#endif
    { QM_PSRAM_EGRESS_CONG_DRP_CNT_REG_OFFSET },
    0,
    0,
    783,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_PSRAM_EGRESS_CONG_DRP_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_BACKPRESSURE, TYPE: Type_QM_TOP_QM_DEBUG_BACKPRESSURE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STATUS *****/
const ru_field_rec QM_BACKPRESSURE_STATUS_FIELD =
{
    "STATUS",
#if RU_INCLUDE_DESC
    "",
    "STATUS:\n\nBack pressure sets the relevant register per bp reason. SW should write-clear in order to unset:\n\n0x1 - fpm exclusive threshold\n0x2 - fpm prefetch occupancy\n0x4 - DDR byte on the fly byte count threshold is exceeded\n0x8 - PD count in copy machine is exceeded\n\nAny permutation of the above may occur. especially if the value isnt cleared every time.\n\n\nA write of 0xFFFF_FFFF in order at reset this indication.\n\n\n",
#endif
    { QM_BACKPRESSURE_STATUS_FIELD_MASK },
    0,
    { QM_BACKPRESSURE_STATUS_FIELD_WIDTH },
    { QM_BACKPRESSURE_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec QM_BACKPRESSURE_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "reserved\n",
#endif
    { QM_BACKPRESSURE_R1_FIELD_MASK },
    0,
    { QM_BACKPRESSURE_R1_FIELD_WIDTH },
    { QM_BACKPRESSURE_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BACKPRESSURE_FIELDS[] =
{
    &QM_BACKPRESSURE_STATUS_FIELD,
    &QM_BACKPRESSURE_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_BACKPRESSURE *****/
const ru_reg_rec QM_BACKPRESSURE_REG =
{
    "BACKPRESSURE",
#if RU_INCLUDE_DESC
    "BACKPRESSURE Register",
    "Back pressure sets the relevant register per bp reason. SW should unset by write clear the corresponding bit:\n\n0x1 - fpm exclusive threshold\n0x2 - fpm prefetch occupancy\n0x4 - DDR byte on the fly byte count threshold is exceeded\n0x8 - PD count in copy machine is exceeded\n\nA write of 0xFFFF_FFFF in order at reset this indication.\n\n\n",
#endif
    { QM_BACKPRESSURE_REG_OFFSET },
    0,
    0,
    784,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_BACKPRESSURE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_AQM_TIMESTAMP_CURR_COUNTER, TYPE: Type_QM_TOP_QM_DEBUG_AQM_TIMESTAMP_CURR_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec QM_AQM_TIMESTAMP_CURR_COUNTER_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "AQM timestamp current 32bit counter value for debug\n",
#endif
    { QM_AQM_TIMESTAMP_CURR_COUNTER_VALUE_FIELD_MASK },
    0,
    { QM_AQM_TIMESTAMP_CURR_COUNTER_VALUE_FIELD_WIDTH },
    { QM_AQM_TIMESTAMP_CURR_COUNTER_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_AQM_TIMESTAMP_CURR_COUNTER_FIELDS[] =
{
    &QM_AQM_TIMESTAMP_CURR_COUNTER_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_AQM_TIMESTAMP_CURR_COUNTER *****/
const ru_reg_rec QM_AQM_TIMESTAMP_CURR_COUNTER_REG =
{
    "AQM_TIMESTAMP_CURR_COUNTER",
#if RU_INCLUDE_DESC
    "AQM_TIMESTAMP_CURR_COUNTER Register",
    "AQM timestamp current 32bit counter value for debug\n",
#endif
    { QM_AQM_TIMESTAMP_CURR_COUNTER_REG_OFFSET },
    0,
    0,
    785,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_AQM_TIMESTAMP_CURR_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_BB0_EGR_MSG_OUT_FIFO_STATUS, TYPE: Type_QM_TOP_QM_DEBUG_BB0_EGR_MSG_OUT_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec QM_BB0_EGR_MSG_OUT_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec QM_BB0_EGR_MSG_OUT_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Empty\n",
#endif
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec QM_BB0_EGR_MSG_OUT_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "Full\n",
#endif
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_FULL_FIELD_WIDTH },
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB0_EGR_MSG_OUT_FIFO_STATUS_FIELDS[] =
{
    &QM_BB0_EGR_MSG_OUT_FIFO_STATUS_USED_WORDS_FIELD,
    &QM_BB0_EGR_MSG_OUT_FIFO_STATUS_EMPTY_FIELD,
    &QM_BB0_EGR_MSG_OUT_FIFO_STATUS_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_BB0_EGR_MSG_OUT_FIFO_STATUS *****/
const ru_reg_rec QM_BB0_EGR_MSG_OUT_FIFO_STATUS_REG =
{
    "BB0_EGR_MSG_OUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB0_EGR_MSG_OUT_FIFO_STATUS Register",
    "Holds the FIFO Status\n",
#endif
    { QM_BB0_EGR_MSG_OUT_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    786,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_BB0_EGR_MSG_OUT_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_COUNT_PKT_NOT_PD_MODE_BITS, TYPE: Type_QM_TOP_QM_DEBUG_COUNT_PKT_NOT_PD_MODE_BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_EGRESS_ACCUM_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_TOTAL_EGRESS_ACCUM_CNT_PKT_FIELD =
{
    "TOTAL_EGRESS_ACCUM_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nTotal egress accumulated counters j1211\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_TOTAL_EGRESS_ACCUM_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_TOTAL_EGRESS_ACCUM_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_TOTAL_EGRESS_ACCUM_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GLOBAL_EGRESS_DROP_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_GLOBAL_EGRESS_DROP_CNT_PKT_FIELD =
{
    "GLOBAL_EGRESS_DROP_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nQM Global egress drop counters j1224\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GLOBAL_EGRESS_DROP_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GLOBAL_EGRESS_DROP_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GLOBAL_EGRESS_DROP_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DROP_ING_EGR_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_DROP_ING_EGR_CNT_PKT_FIELD =
{
    "DROP_ING_EGR_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nDrop counters (ingress and egress) j1226\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DROP_ING_EGR_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DROP_ING_EGR_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DROP_ING_EGR_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_GRP_DROP_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_FPM_GRP_DROP_CNT_PKT_FIELD =
{
    "FPM_GRP_DROP_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nFPM_GRP_DROP_CNT (UG or BMGR drop)\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_FPM_GRP_DROP_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_FPM_GRP_DROP_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_FPM_GRP_DROP_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_PD_CONGESTION_DROP_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_PD_CONGESTION_DROP_CNT_PKT_FIELD =
{
    "QM_PD_CONGESTION_DROP_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nQM_PD_CONGESTION_DROP_CNT (crossing max pds per queue - totatl_pd_thr)\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_PD_CONGESTION_DROP_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_PD_CONGESTION_DROP_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_PD_CONGESTION_DROP_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_PD_CONGESTION_DROP_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_DDR_PD_CONGESTION_DROP_CNT_PKT_FIELD =
{
    "DDR_PD_CONGESTION_DROP_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nDDR_PD_CONGESTION_DROP_CNT (copy machine fifo used entries)\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DDR_PD_CONGESTION_DROP_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DDR_PD_CONGESTION_DROP_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DDR_PD_CONGESTION_DROP_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRED_DROP_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_WRED_DROP_CNT_PKT_FIELD =
{
    "WRED_DROP_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nWRED_DROP_CNT_PKT\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_WRED_DROP_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_WRED_DROP_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_WRED_DROP_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GOOD_LVL2_PKTS_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL2_PKTS_CNT_PKT_FIELD =
{
    "GOOD_LVL2_PKTS_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nGOOD_LVL2_PKTS_CNT will count ingress agg pd as packets (if reprocessing bit set)\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL2_PKTS_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL2_PKTS_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL2_PKTS_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GOOD_LVL1_PKTS_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL1_PKTS_CNT_PKT_FIELD =
{
    "GOOD_LVL1_PKTS_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nGOOD_LVL1_PKTS_CNT will count ingress agg pd as packets (if reprocessing bit not set)\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL1_PKTS_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL1_PKTS_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL1_PKTS_CNT_PKT_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_TOTAL_VALID_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_TOTAL_VALID_CNT_PKT_FIELD =
{
    "QM_TOTAL_VALID_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nQM_TOTAL_VALID_CNT_PKT will count agg pd as packets\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_TOTAL_VALID_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_TOTAL_VALID_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_TOTAL_VALID_CNT_PKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DQM_VALID_CNT_PKT *****/
const ru_field_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_DQM_VALID_CNT_PKT_FIELD =
{
    "DQM_VALID_CNT_PKT",
#if RU_INCLUDE_DESC
    "",
    "Mode/Chicken bits for count pkts instead of PDs jira fix:\nDQM_VALID_CNT_PKT will count agg pd as packets\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DQM_VALID_CNT_PKT_FIELD_MASK },
    0,
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DQM_VALID_CNT_PKT_FIELD_WIDTH },
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_DQM_VALID_CNT_PKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_COUNT_PKT_NOT_PD_MODE_BITS_FIELDS[] =
{
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_TOTAL_EGRESS_ACCUM_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_GLOBAL_EGRESS_DROP_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_DROP_ING_EGR_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_FPM_GRP_DROP_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_PD_CONGESTION_DROP_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_DDR_PD_CONGESTION_DROP_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_WRED_DROP_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL2_PKTS_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_GOOD_LVL1_PKTS_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_QM_TOTAL_VALID_CNT_PKT_FIELD,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_DQM_VALID_CNT_PKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_COUNT_PKT_NOT_PD_MODE_BITS *****/
const ru_reg_rec QM_COUNT_PKT_NOT_PD_MODE_BITS_REG =
{
    "COUNT_PKT_NOT_PD_MODE_BITS",
#if RU_INCLUDE_DESC
    "COUNT_PKT_NOT_PD_MODE_BITS Register",
    "Mode/Chicken bits for count pkts instead of PDs jira fixes:\n[0] - Total egress accumulated counters j1211\n[1] - QM Global egress drop counters j1224\n[2] - Drop counters (ingress and egress) j1226\n[3] - FPM_GRP_DROP_CNT (UG or BMGR drop)\n[4] - QM_PD_CONGESTION_DROP_CNT (crossing max pds per queue - totatl_pd_thr)\n[5] - DDR_PD_CONGESTION_DROP_CNT (copy machine fifo used entries)\n[6] - WRED_DROP_CNT\n[7] - GOOD_LVL2_PKTS_CNT will count ingress agg pd as 1 packet (if reprocessing bit set)\n[8] - GOOD_LVL1_PKTS_CNT will count ingress agg pd as 1 packet (if reprocessing bit not set)\n",
#endif
    { QM_COUNT_PKT_NOT_PD_MODE_BITS_REG_OFFSET },
    0,
    0,
    787,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    QM_COUNT_PKT_NOT_PD_MODE_BITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_QM_ENABLE_CTRL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_QM_ENABLE_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_PREFETCH_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD =
{
    "FPM_PREFETCH_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "FPM Prefetch Enable. Setting this bit to 1 will start filling up the FPM pool prefetch FIFOs.\nSeeting this bit to 0, will stop FPM prefetches.\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REORDER_CREDIT_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD =
{
    "REORDER_CREDIT_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set the QM will send credits to the REORDER block.\nDisabling this bit will stop sending credits to the reorder.\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DQM_POP_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD =
{
    "DQM_POP_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set the QM will pop PDs from the DQM and place them in the runner SRAM\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RMT_FIXED_ARB_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD =
{
    "RMT_FIXED_ARB_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set Fixed arbitration will be done in pops from the remote FIFOs (Non delayed highest priority). If this bit is cleared RR arbitration is done\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DQM_PUSH_FIXED_ARB_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD =
{
    "DQM_PUSH_FIXED_ARB_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set Fixed arbitration will be done in DQM pushes (CPU highest priority, then non-delayed queues and then normal queues. If this bit is cleared RR arbitration is done.\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AQM_CLK_COUNTER_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_CLK_COUNTER_ENABLE_FIELD =
{
    "AQM_CLK_COUNTER_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set it enables the AQM internal global counter that counts clocks to uSec. If this bit is cleared the counter is disabled and zeroed.\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_CLK_COUNTER_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_CLK_COUNTER_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_CLK_COUNTER_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AQM_TIMESTAMP_COUNTER_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_COUNTER_ENABLE_FIELD =
{
    "AQM_TIMESTAMP_COUNTER_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set it enables the AQM timestamp global counter that counts timestamp in uSec resolution. If this bit is cleared the counter is disabled and zeroed.\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_COUNTER_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_COUNTER_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_COUNTER_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AQM_TIMESTAMP_WRITE_TO_PD_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_WRITE_TO_PD_ENABLE_FIELD =
{
    "AQM_TIMESTAMP_WRITE_TO_PD_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set it enables the AQM timestamp 10bit write to PD. If this bit is cleared the timestamp is not written to PD.\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_WRITE_TO_PD_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_WRITE_TO_PD_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_WRITE_TO_PD_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_ENABLE_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_FPM_PREFETCH_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_REORDER_CREDIT_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_POP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_RMT_FIXED_ARB_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_DQM_PUSH_FIXED_ARB_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_CLK_COUNTER_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_COUNTER_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_AQM_TIMESTAMP_WRITE_TO_PD_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_QM_ENABLE_CTRL *****/
const ru_reg_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_REG =
{
    "GLOBAL_CFG_QM_ENABLE_CTRL",
#if RU_INCLUDE_DESC
    "QM_ENABLE_CTRL Register",
    "QM Enable register\n",
#endif
    { QM_GLOBAL_CFG_QM_ENABLE_CTRL_REG_OFFSET },
    0,
    0,
    788,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_QM_SW_RST_CTRL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_QM_SW_RST_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_PREFETCH0_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD =
{
    "FPM_PREFETCH0_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "FPM Prefetch FIFO0 SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH0_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_PREFETCH1_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD =
{
    "FPM_PREFETCH1_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "FPM Prefetch FIFO1 SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH1_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_PREFETCH2_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD =
{
    "FPM_PREFETCH2_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "FPM Prefetch FIFO2 SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH2_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_PREFETCH3_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD =
{
    "FPM_PREFETCH3_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "FPM Prefetch FIFO3 SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_FPM_PREFETCH3_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NORMAL_RMT_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD =
{
    "NORMAL_RMT_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "Normal Remote FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NORMAL_RMT_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_DELAYED_RMT_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD =
{
    "NON_DELAYED_RMT_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "Non-delayed Remote FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_RMT_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRE_CM_FIFO_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD =
{
    "PRE_CM_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "Pre Copy Machine FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_PRE_CM_FIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CM_RD_PD_FIFO_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD =
{
    "CM_RD_PD_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "Copy Machine RD PD FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_RD_PD_FIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CM_WR_PD_FIFO_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD =
{
    "CM_WR_PD_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "Pre Copy Machine FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_CM_WR_PD_FIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BB0_OUTPUT_FIFO_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD =
{
    "BB0_OUTPUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "BB0 OUTPUT FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_OUTPUT_FIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BB1_OUTPUT_FIFO_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD =
{
    "BB1_OUTPUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "BB1 Output FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_OUTPUT_FIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BB1_INPUT_FIFO_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD =
{
    "BB1_INPUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "BB1 Input FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB1_INPUT_FIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TM_FIFO_PTR_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD =
{
    "TM_FIFO_PTR_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "TM FIFOs Pointers SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_TM_FIFO_PTR_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_DELAYED_OUT_FIFO_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD =
{
    "NON_DELAYED_OUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "Non delayed output FIFO Pointers SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_NON_DELAYED_OUT_FIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BB0_EGR_MSG_OUT_FIFO_SW_RST *****/
const ru_field_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_EGR_MSG_OUT_FIFO_SW_RST_FIELD =
{
    "BB0_EGR_MSG_OUT_FIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "BB0 free messages from egress OUTPUT FIFO SW reset.\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_EGR_MSG_OUT_FIFO_SW_RST_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_EGR_MSG_OUT_FIFO_SW_RST_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_EGR_MSG_OUT_FIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_BB0_EGR_MSG_OUT_FIFO_SW_RST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_QM_SW_RST_CTRL *****/
const ru_reg_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_REG =
{
    "GLOBAL_CFG_QM_SW_RST_CTRL",
#if RU_INCLUDE_DESC
    "QM_SW_RST_CTRL Register",
    "QM soft reset register\n",
#endif
    { QM_GLOBAL_CFG_QM_SW_RST_CTRL_REG_OFFSET },
    0,
    0,
    789,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_QM_GENERAL_CTRL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_QM_GENERAL_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DROP_CNT_PKTS_READ_CLEAR_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD =
{
    "DROP_CNT_PKTS_READ_CLEAR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Indicates whether the Drop/max_occupancy packets counter is read clear.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DROP_CNT_BYTES_READ_CLEAR_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD =
{
    "DROP_CNT_BYTES_READ_CLEAR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Indicates whether the Drop/max_occupancy bytes counter is read clear.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD =
{
    "DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT",
#if RU_INCLUDE_DESC
    "",
    "This bit defines the functionality of the drop packets counter.\n0 - Functions as the drop packets counter\n1 - Functions as the max packets occupancy holder\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD =
{
    "DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT",
#if RU_INCLUDE_DESC
    "",
    "This bit defines the functionality of the drop bytes counter.\n0 - Functions as the drop bytes counter\n1 - Functions as the max bytes occupancy holder\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_WITH_CONTEXT_LAST_SEARCH *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD =
{
    "FREE_WITH_CONTEXT_LAST_SEARCH",
#if RU_INCLUDE_DESC
    "",
    "Indicates The value to put in the last_search field of the SBPM free with context message\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FREE_WITH_CONTEXT_LAST_SEARCH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRED_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD =
{
    "WRED_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables WRED influence on drop condition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_WRED_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_PD_CONGESTION_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD =
{
    "DDR_PD_CONGESTION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables DDR_PD_CONGESTION influence on drop/bp\ncondition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BYTE_CONGESTION_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD =
{
    "DDR_BYTE_CONGESTION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables DDR_BYTE_CONGESTION influence on drop/bp condition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_BYTE_CONGESTION_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_OCCUPANCY_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD =
{
    "DDR_OCCUPANCY_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables DDR_OCCUPANCY influence on drop/bp condition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_OCCUPANCY_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_FPM_CONGESTION_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD =
{
    "DDR_FPM_CONGESTION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables DDR_FPM_CONGESTION influence on drop/bp condition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_FPM_CONGESTION_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_UG_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD =
{
    "FPM_UG_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables FPM_UG influence on drop condition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD =
{
    "QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables QUEUE_OCCUPANCY_DDR_COPY_DECISION influence on copy condition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD =
{
    "PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables PSRAM_OCCUPANCY_DDR_COPY_DECISION influence on copy condition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DONT_SEND_MC_BIT_TO_BBH *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD =
{
    "DONT_SEND_MC_BIT_TO_BBH",
#if RU_INCLUDE_DESC
    "",
    "When set, the multicast bit of the PD will not be sent to BBH TX\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DONT_SEND_MC_BIT_TO_BBH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD =
{
    "CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "When set, aggregations are not closed automatically when queue open aggregation time expired.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD =
{
    "FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "When cleared, given that there is an FPM congestion situation and all prefetch FPM buffers are full then a min pool size buffer will be freed each 1us. This is done due to the fact that exclusive indication is received only togeter with buffer allocation reply and if this will not be done then a deadlock could occur.\nSetting this bit will disable this mechanism.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_BUFFER_GLOBAL_RES_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD =
{
    "FPM_BUFFER_GLOBAL_RES_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "FPM over subscription mechanism.\nEach queue will have one out of 8 reserved byte threshold profiles. Each profile defines 8 bit threshold with 512byte resolution.\nOnce the global FPM counter pass configurable threshold the system goes to buffer reservation congestion state. In this state any PD entering a queue which passes the reserved byte threshold will be dropped.\n\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_BUFFER_GLOBAL_RES_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_PRESERVE_PD_WITH_FPM *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD =
{
    "QM_PRESERVE_PD_WITH_FPM",
#if RU_INCLUDE_DESC
    "",
    "Dont drop pd with fpm allocation.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_PRESERVE_PD_WITH_FPM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_RESIDUE_PER_QUEUE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD =
{
    "QM_RESIDUE_PER_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "Updated definition:\n1 - Use full residue memory - reset value\n0 - Debug only - use half size or no residue in projects where the full residue is 32B/q\n\n\nOLD Definition (perior 63146A0)\n6878:\n1 for 32B/Queue\n0 debug - no residue\n\nOther projects:\n0 for 64B/Queue\n1 for 128B/Queue\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_RESIDUE_PER_QUEUE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD =
{
    "GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN",
#if RU_INCLUDE_DESC
    "",
    "Controls the timing of updating the overhead counters with packets which goes through aggregation.\n\n0 - updates when the packets enters QM\n1 - updates when aggregation is done.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_UG_FLOW_CTRL_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD =
{
    "FPM_UG_FLOW_CTRL_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables FPM_UG influence on flow control wake up messages to FW.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_FPM_UG_FLOW_CTRL_DISABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_WRITE_MULTI_SLAVE_EN *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD =
{
    "DDR_WRITE_MULTI_SLAVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables to write packet transaction to multiple slave (unlimited), if disable only one ubus slave allowed.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_WRITE_MULTI_SLAVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_PD_CONGESTION_AGG_PRIORITY *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD =
{
    "DDR_PD_CONGESTION_AGG_PRIORITY",
#if RU_INCLUDE_DESC
    "",
    "global priority bit to aggregated PDs which go through reprocessing.\n\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DDR_PD_CONGESTION_AGG_PRIORITY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_OCCUPANCY_DROP_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD =
{
    "PSRAM_OCCUPANCY_DROP_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables PSRAM_OCCUPANCY_DROP influence on drop condition\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_PSRAM_OCCUPANCY_DROP_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_DDR_WRITE_ALIGNMENT *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD =
{
    "QM_DDR_WRITE_ALIGNMENT",
#if RU_INCLUDE_DESC
    "",
    "0 According to length\n1 8-byte aligned\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_QM_DDR_WRITE_ALIGNMENT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLUSIVE_DONT_DROP *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD =
{
    "EXCLUSIVE_DONT_DROP",
#if RU_INCLUDE_DESC
    "",
    "Controls if the exclusive indication in PD marks the PD as dont drop or as dont drop if the fpm in exclusive state\n1 - global dont drop\n0 - FPM exclusive state dont drop\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_EXCLUSIVE_DONT_DROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DQMOL_JIRA_973_FIX_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DQMOL_JIRA_973_FIX_ENABLE_FIELD =
{
    "DQMOL_JIRA_973_FIX_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "chicken bit for DQMOL bug:\nREPIN_D is full, QSM is locked by hw_push, prefetch locks UBUS.\nprefetch waits to QSM to unload repin_d but hw_push locks UBUS but cant since adjusted reqout_h is full because HOL is read req(offload or FPM req).\n\npreviously - EXCLUSIVE_DONT_DROP_BP_EN\n\nwhen set 1 backpressure will be applied when the DONT_DROP pd should be dropped.\nfor example, 0 fpm buffers available and the PD should be copied to DDR.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DQMOL_JIRA_973_FIX_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DQMOL_JIRA_973_FIX_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DQMOL_JIRA_973_FIX_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GPON_DBR_CEIL *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD =
{
    "GPON_DBR_CEIL",
#if RU_INCLUDE_DESC
    "",
    "If the bit enable, QM round up to 4 every packet length added ghost counters.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DROP_CNT_WRED_DROPS *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD =
{
    "DROP_CNT_WRED_DROPS",
#if RU_INCLUDE_DESC
    "",
    "Drop counter counts WRED drops by color per queue.\nIn order to enable this feature the drop counter should be configured to count drops. if the drop counter is configured count max occupancy per queue, it will override WRED drops count.\ncolor 0 - is written in dropped bytes field (word0)\ncolor 1 - is written in dropped pkts field (word1)\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SAME_SEC_LVL_BIT_AGG_EN *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_SAME_SEC_LVL_BIT_AGG_EN_FIELD =
{
    "SAME_SEC_LVL_BIT_AGG_EN",
#if RU_INCLUDE_DESC
    "",
    "Uses bit 126 in aggregated PD to mark aggregation where all packets have the same second level queue.\nIf set to 0, then aggregated PDs in DRAM bit 126 is the 9th bit of the fourth second level queue\nif set to 1, then aggregated PDs in DRAM bit 126 is same second level queue.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_SAME_SEC_LVL_BIT_AGG_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_SAME_SEC_LVL_BIT_AGG_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_SAME_SEC_LVL_BIT_AGG_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE_FIELD =
{
    "GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Indicates whether the QM_GLOBAL_EGRESS_DROP_COUNTER counter is read clear.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE_FIELD =
{
    "GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Indicates whether the QM_GLOBAL_EGRESS_AQM_DROP_COUNTER counter is read clear.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DQMOL_JIRA_973_FIX_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_GPON_DBR_CEIL_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_DROP_CNT_WRED_DROPS_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_SAME_SEC_LVL_BIT_AGG_EN_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_QM_GENERAL_CTRL *****/
const ru_reg_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_REG =
{
    "GLOBAL_CFG_QM_GENERAL_CTRL",
#if RU_INCLUDE_DESC
    "QM_GENERAL_CTRL Register",
    "QM General Control register\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL_REG_OFFSET },
    0,
    0,
    790,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_FPM_CONTROL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_FPM_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_POOL_BP_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD =
{
    "FPM_POOL_BP_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "This field indicates whether crossing the per pool FPM buffer prefetch FIFO occupancy thresholds will result in dropping packets or in applying back pressure to the re-order.\n0 - drop packets\n1 - apply back pressure\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_CONGESTION_BP_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD =
{
    "FPM_CONGESTION_BP_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "This field indicates whether crossing the FPM congestion threshold will result in dropping packets or in applying back pressure to the re-order.\n0 - drop packets\n1 - apply back pressure\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_FORCE_BP_LVL *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_FORCE_BP_LVL_FIELD =
{
    "FPM_FORCE_BP_LVL",
#if RU_INCLUDE_DESC
    "",
    "Min pool occupancy which forces BP even if QM is working in drop mode. The purpose of this reg is to solve cases when exclusive_dont_drop packets exist in QM + pools are shallow. BP_EN = MIN(POOL0,POOL1,POOL2,POOL3) < FPM_FORCE_BP_LVL.\nWhen set to 0, BP isnt applied. It isnt recommended since a packet of an exclusive dont drop queue can receive dirty/used FPM buffer.\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_FORCE_BP_LVL_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_FORCE_BP_LVL_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_FORCE_BP_LVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_PREFETCH_GRANULARITY *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_GRANULARITY_FIELD =
{
    "FPM_PREFETCH_GRANULARITY",
#if RU_INCLUDE_DESC
    "",
    "FPM_PREFETCH_MIN_POOL_SIZE granularity\n\n0 - 256B\n1 - 320B\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_GRANULARITY_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_GRANULARITY_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_GRANULARITY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_PREFETCH_MIN_POOL_SIZE *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD =
{
    "FPM_PREFETCH_MIN_POOL_SIZE",
#if RU_INCLUDE_DESC
    "",
    "FPM prefetch minimum pool size.\nThe supported FPM pool sizes are derived from this value:\n* FPM_PREFETCH_MIN_POOL_SIZEx1\n* FPM_PREFETCH_MIN_POOL_SIZEx2\n* FPM_PREFETCH_MIN_POOL_SIZEx4\n* FPM_PREFETCH_MIN_POOL_SIZEx8\n\nThe optional values for this field (also depend on FPM_PREFETCH_GRANULARITY value) :\n0 - 256Byte  or 320Byte\n1 - 512Byte  or 640Byte\n2 - 1024Byte or 1280Byte\n3 - 2048Byte or 2560Byte\n\n\n\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_PREFETCH_PENDING_REQ_LIMIT *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD =
{
    "FPM_PREFETCH_PENDING_REQ_LIMIT",
#if RU_INCLUDE_DESC
    "",
    "The allowed on the fly FPM prefetch pending Alloc requests to the FPM.\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_OVERRIDE_BB_ID_EN *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_EN_FIELD =
{
    "FPM_OVERRIDE_BB_ID_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable FPM BB ID override with non default value\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_OVERRIDE_BB_ID_VALUE *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_VALUE_FIELD =
{
    "FPM_OVERRIDE_BB_ID_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value to override the default FPM BB ID.\n\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_VALUE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_VALUE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_CONTROL_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_POOL_BP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_CONGESTION_BP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_FORCE_BP_LVL_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_GRANULARITY_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_MIN_POOL_SIZE_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_PREFETCH_PENDING_REQ_LIMIT_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_EN_FIELD,
    &QM_GLOBAL_CFG_FPM_CONTROL_FPM_OVERRIDE_BB_ID_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_FPM_CONTROL *****/
const ru_reg_rec QM_GLOBAL_CFG_FPM_CONTROL_REG =
{
    "GLOBAL_CFG_FPM_CONTROL",
#if RU_INCLUDE_DESC
    "FPM_CONTROL Register",
    "FPM Control Register\n\nAdditonal FPM registers can be found below - Enhancement registers\n",
#endif
    { QM_GLOBAL_CFG_FPM_CONTROL_REG_OFFSET },
    0,
    0,
    791,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    QM_GLOBAL_CFG_FPM_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BYTE_CONGESTION_DROP_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD =
{
    "DDR_BYTE_CONGESTION_DROP_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "This field indicates whether crossing the DDR bytes thresholds (the number of bytes waiting to be copied to DDR) will result in dropping packets or in applying back pressure to the re-order.\n0 - apply back pressure\n1 - drop packets\n\n",
#endif
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_DDR_BYTE_CONGESTION_DROP_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL *****/
const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_REG =
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_CONTROL Register",
    "DDR Byte Congestion Control Register\n",
#endif
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_REG_OFFSET },
    0,
    0,
    792,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BYTES_LOWER_THR *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD =
{
    "DDR_BYTES_LOWER_THR",
#if RU_INCLUDE_DESC
    "",
    "DDR copy bytes Lower Threshold.\nWhen working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:\n* If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.\n* If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are dropped.\n* If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped.\nWhen working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and DDR_BYTES_MID_THR are dont care).\n",
#endif
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD_SHIFT },
    98304,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_DDR_BYTES_LOWER_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR *****/
const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_REG =
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_LOWER_THR Register",
    "DDR Byte Congestion Lower Threshold\n",
#endif
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_REG_OFFSET },
    0,
    0,
    793,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BYTES_MID_THR *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD =
{
    "DDR_BYTES_MID_THR",
#if RU_INCLUDE_DESC
    "",
    "DDR copy bytes Lower Threshold.\nWhen working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:\n* If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.\n* If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are dropped.\n* If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped.\nWhen working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and DDR_BYTES_MID_THR are dont care).\n",
#endif
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD_SHIFT },
    114688,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_DDR_BYTES_MID_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR *****/
const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_REG =
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_MID_THR Register",
    "DDR Byte Congestion Middle Threshold\n",
#endif
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_REG_OFFSET },
    0,
    0,
    794,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BYTES_HIGHER_THR *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD =
{
    "DDR_BYTES_HIGHER_THR",
#if RU_INCLUDE_DESC
    "",
    "DDR copy bytes Lower Threshold.\nWhen working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:\n* If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.\n* If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are dropped.\n* If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped.\nWhen working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and DDR_BYTES_MID_THR are dont care).\n",
#endif
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD_SHIFT },
    131072,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_DDR_BYTES_HIGHER_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR *****/
const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_REG =
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_HIGHER_THR Register",
    "DDR Byte Congestion Higher Threshold\n",
#endif
    { QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_REG_OFFSET },
    0,
    0,
    795,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_PD_CONGESTION_DROP_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD =
{
    "DDR_PD_CONGESTION_DROP_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "This field indicates whether crossing the DDR Pipe thresholds will result in dropping packets or in applying back pressure to the re-order.\n0 - apply back pressure\n1 - drop packets\n\n",
#endif
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_PIPE_LOWER_THR *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD =
{
    "DDR_PIPE_LOWER_THR",
#if RU_INCLUDE_DESC
    "",
    "DDR copy Pipe Lower Threshold.\nWhen working in packet drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:\n* If (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (DDR_PIPE_LOWER_THR) < (DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in low priority are dropped.\n* If (DDR copy pipe occupancy) <=  (DDR_PIPE_LOWER_THR), then no packets are dropped.\nWhen working in backpressure mode (DDR_PD_CONGESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_PIPE_LOWER_THR is dont care).\n",
#endif
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD_SHIFT },
    96,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_PIPE_HIGHER_THR *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD =
{
    "DDR_PIPE_HIGHER_THR",
#if RU_INCLUDE_DESC
    "",
    "DDR copy Pipe Lower Threshold.\nWhen working in packet drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:\n* If (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are dropped (only exclusive packets are not dropped).\n* If (DDR_PIPE_LOWER_THR) < (DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in low priority are dropped.\n* If (DDR copy pipe occupancy) <= (DDR_PIPE_LOWER_THR), then no packets are dropped.\nWhen working in backpressure mode (DDR_PD_CONGESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_PIPE_LOWER_THR is dont care).\nIMPORTANT: recommended maximum value is 0x7B in order to avoid performance degradation when working with aggregation timeout enable\n",
#endif
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD_SHIFT },
    120,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PD_CONGESTION_DROP_ENABLE_FIELD,
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_LOWER_THR_FIELD,
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_DDR_PIPE_HIGHER_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL *****/
const ru_reg_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_REG =
{
    "GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_CONTROL Register",
    "DDR PD Congestion Control Register\n",
#endif
    { QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_REG_OFFSET },
    0,
    0,
    796,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_PD_THR *****/
const ru_field_rec QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD =
{
    "TOTAL_PD_THR",
#if RU_INCLUDE_DESC
    "",
    "If the number of PDs for a certain queue exceeds this value, then PDs will be dropped.\n",
#endif
    { QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD_SHIFT },
    65536,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_TOTAL_PD_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL *****/
const ru_reg_rec QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_REG =
{
    "GLOBAL_CFG_QM_PD_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "QM_PD_CONGESTION_CONTROL Register",
    "QM PD Congestion Control Register\n",
#endif
    { QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_REG_OFFSET },
    0,
    0,
    797,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_ABS_DROP_QUEUE, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_ABS_DROP_QUEUE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ABS_DROP_QUEUE *****/
const ru_field_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD =
{
    "ABS_DROP_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "Absolute address drop queue.\nAbsolute address PDs which are dropped will be redirected into this configured queue. FW will be responsible for reclaiming their DDR space.\n",
#endif
    { QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ABS_DROP_QUEUE_EN *****/
const ru_field_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD =
{
    "ABS_DROP_QUEUE_EN",
#if RU_INCLUDE_DESC
    "",
    "Absolute address drop queue enable.\nEnables the mechanism in which absolute address PDs which are dropped are be redirected into this configured queue. FW will be responsible for reclaiming their DDR space.\n",
#endif
    { QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_ABS_DROP_QUEUE_FIELDS[] =
{
    &QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_FIELD,
    &QM_GLOBAL_CFG_ABS_DROP_QUEUE_ABS_DROP_QUEUE_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_ABS_DROP_QUEUE *****/
const ru_reg_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_REG =
{
    "GLOBAL_CFG_ABS_DROP_QUEUE",
#if RU_INCLUDE_DESC
    "ABS_DROP_QUEUE Register",
    "Absolute Adress drop queue\n",
#endif
    { QM_GLOBAL_CFG_ABS_DROP_QUEUE_REG_OFFSET },
    0,
    0,
    798,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_AGGREGATION_CTRL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_AGGREGATION_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_AGG_BYTES *****/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD =
{
    "MAX_AGG_BYTES",
#if RU_INCLUDE_DESC
    "",
    "This field indicates the maximum number of bytes in an aggregated PD.\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD_SHIFT },
    512,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_AGG_PKTS *****/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD =
{
    "MAX_AGG_PKTS",
#if RU_INCLUDE_DESC
    "",
    "This field indicates the maximum number of packets in an aggregated PD\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGG_OVR_512B_EN *****/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_AGG_OVR_512B_EN_FIELD =
{
    "AGG_OVR_512B_EN",
#if RU_INCLUDE_DESC
    "",
    "This feature, when enabled, allows QM to aggregate more than 512 in each aggregation.\nMax PD size allowed to be added to an aggregation will still remain 512 due to limitation in the PD struct.\nNote that the default value once this feature is enabled in 640Byte. This needs to be configured in the MAX_AGG_BYTES.\nMAX_PACKET_SIZE will still be 512\n\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_AGG_OVR_512B_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_AGG_OVR_512B_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_AGG_OVR_512B_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_AGG_PKT_SIZE *****/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKT_SIZE_FIELD =
{
    "MAX_AGG_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "This indicates the maximum Packet size that can be aggregated.\nWith current PD limitation max Packet can be 512.\nThis is true even if the AGG_OVER_512BYTE_ENABLE is set to 1b1.\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKT_SIZE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKT_SIZE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKT_SIZE_FIELD_SHIFT },
    511,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_AGG_PKT_SIZE *****/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_MIN_AGG_PKT_SIZE_FIELD =
{
    "MIN_AGG_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "This indicates the minimum Packet size that can be aggregated.\nThis is for the design to understand if the current accumulated agg size has enough space for another Packet. if it hasnt, it will close the agg. if it has it will agg and leave the aggregation open.\n\n\nNote!!\nYou must config this value aligned to be to 8 (e.g for 60 byte min packet configure 64)\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MIN_AGG_PKT_SIZE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MIN_AGG_PKT_SIZE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_MIN_AGG_PKT_SIZE_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_AGGREGATION_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_BYTES_FIELD,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKTS_FIELD,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_AGG_OVR_512B_EN_FIELD,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_MAX_AGG_PKT_SIZE_FIELD,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_MIN_AGG_PKT_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_AGGREGATION_CTRL *****/
const ru_reg_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_REG =
{
    "GLOBAL_CFG_AGGREGATION_CTRL",
#if RU_INCLUDE_DESC
    "AGGREGATION_CTRL Register",
    "Aggregation Control register\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL_REG_OFFSET },
    0,
    0,
    799,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    QM_GLOBAL_CFG_AGGREGATION_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_AGGREGATION_CTRL2, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_AGGREGATION_CTRL2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: AGG_POOL_SEL_EN *****/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_EN_FIELD =
{
    "AGG_POOL_SEL_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable pool selection for aggregarion according to AGG_POOL_SEL configuration and not according to min_buffer_size config\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGG_POOL_SEL *****/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_FIELD =
{
    "AGG_POOL_SEL",
#if RU_INCLUDE_DESC
    "",
    "This register sets the the size of the pool to use for Agg Packets.\nSince the design can now support max AGG of any size up to 640 (and even till 1024) there should be an explicit configuration to which pool to use.\n\n-> POOL0 - Use 8 buffers\n-> POOL1 - Use 4 buffers\n-> POOL2 - Use 2 buffers\n-> POOL3 - Use 1 buffers\n\nThis configuration needs to take into account the min_buffer_size and max_agg_size in order to make sure that the correct pool is chosen for agg\n\ne.g. min buffer size is 256 and max_agg_byte is 1016. will require pool of 4 buffers\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_AGGREGATION_CTRL2_FIELDS[] =
{
    &QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_EN_FIELD,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL2_AGG_POOL_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_AGGREGATION_CTRL2 *****/
const ru_reg_rec QM_GLOBAL_CFG_AGGREGATION_CTRL2_REG =
{
    "GLOBAL_CFG_AGGREGATION_CTRL2",
#if RU_INCLUDE_DESC
    "AGGREGATION_CTRL_2 Register",
    "Aggregation Control 2nd register\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CTRL2_REG_OFFSET },
    0,
    0,
    800,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_AGGREGATION_CTRL2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_FPM_BASE_ADDR, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_FPM_BASE_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_BASE_ADDR *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD =
{
    "FPM_BASE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "FPM Base Address. This is the 32-bit MSBs out of the 40-bit address.\nMultiply this field by 256 to get the 40-bit address.\nExample:\nIf desired base address is 0x0080_0000\nThe FPM_BASE_ADDR field should be configured to: 0x0000_8000.\n",
#endif
    { QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_BASE_ADDR_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_BASE_ADDR_FPM_BASE_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_FPM_BASE_ADDR *****/
const ru_reg_rec QM_GLOBAL_CFG_FPM_BASE_ADDR_REG =
{
    "GLOBAL_CFG_FPM_BASE_ADDR",
#if RU_INCLUDE_DESC
    "FPM_BASE_ADDR Register",
    "FPM Base Address\n",
#endif
    { QM_GLOBAL_CFG_FPM_BASE_ADDR_REG_OFFSET },
    0,
    0,
    801,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_FPM_BASE_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_BASE_ADDR *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD =
{
    "FPM_BASE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "FPM Base Address. This is the 32-bit MSBs out of the 40-bit address.\nMultiply this field by 256 to get the 40-bit address.\nExample:\nIf desired base address is 0x0080_0000\nThe FPM_BASE_ADDR field should be configured to: 0x0000_8000.\n",
#endif
    { QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FPM_BASE_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR *****/
const ru_reg_rec QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG =
{
    "GLOBAL_CFG_FPM_COHERENT_BASE_ADDR",
#if RU_INCLUDE_DESC
    "FPM_COHERENT_BASE_ADDR Register",
    "FPM Base Address for PDs that have the coherent bit set\n",
#endif
    { QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG_OFFSET },
    0,
    0,
    802,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DDR_SOP_OFFSET, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DDR_SOP_OFFSET
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SOP_OFFSET0 *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD =
{
    "DDR_SOP_OFFSET0",
#if RU_INCLUDE_DESC
    "",
    "DDR SOP Offset option 0\n\n",
#endif
    { QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_SOP_OFFSET1 *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD =
{
    "DDR_SOP_OFFSET1",
#if RU_INCLUDE_DESC
    "",
    "DDR SOP Offset option 1\n\n",
#endif
    { QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD_SHIFT },
    98,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_SOP_OFFSET_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET0_FIELD,
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_DDR_SOP_OFFSET1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DDR_SOP_OFFSET *****/
const ru_reg_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG =
{
    "GLOBAL_CFG_DDR_SOP_OFFSET",
#if RU_INCLUDE_DESC
    "DDR_SOP_OFFSET Register",
    "DDR SOP Offset options\n",
#endif
    { QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG_OFFSET },
    0,
    0,
    803,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EPON_LINE_RATE *****/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD =
{
    "EPON_LINE_RATE",
#if RU_INCLUDE_DESC
    "",
    "EPON Line Rate\n0 - 1G\n1 - 10G\n",
#endif
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EPON_CRC_ADD_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD =
{
    "EPON_CRC_ADD_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "If this bit is not set then 4-bytes will be added to the ghost reporting accumulated bytes and to the byte overhead calculation input\n",
#endif
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_FLOW_OVERWRITE_CRC_EN *****/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD =
{
    "MAC_FLOW_OVERWRITE_CRC_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables to overwrite CRC addition specified MAC FLOW in the field below.\n",
#endif
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_FLOW_OVERWRITE_CRC *****/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD =
{
    "MAC_FLOW_OVERWRITE_CRC",
#if RU_INCLUDE_DESC
    "",
    "MAC flow ID to force disable CRC addition\n",
#endif
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD_WIDTH },
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FEC_IPG_LENGTH *****/
const ru_field_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD =
{
    "FEC_IPG_LENGTH",
#if RU_INCLUDE_DESC
    "",
    "FEC IPG Length\n",
#endif
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD_WIDTH },
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD_SHIFT },
    10,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_LINE_RATE_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_EPON_CRC_ADD_DISABLE_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_EN_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_MAC_FLOW_OVERWRITE_CRC_FIELD,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FEC_IPG_LENGTH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL *****/
const ru_reg_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG =
{
    "GLOBAL_CFG_EPON_OVERHEAD_CTRL",
#if RU_INCLUDE_DESC
    "EPON_OVERHEAD_CTRL Register",
    "EPON Ghost reporting configuration\n",
#endif
    { QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG_OFFSET },
    0,
    0,
    804,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_BBHTX_FIFO_ADDR, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_BBHTX_FIFO_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "ADDR\n",
#endif
    { QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_ADDR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_ADDR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_ADDR_FIELD_SHIFT },
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBHTX_REQ_OTF *****/
const ru_field_rec QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD =
{
    "BBHTX_REQ_OTF",
#if RU_INCLUDE_DESC
    "",
    "BBHTX_REQ_OTF\n",
#endif
    { QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD_WIDTH },
    { QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_FIELDS[] =
{
    &QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_ADDR_FIELD,
    &QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_BBHTX_REQ_OTF_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_BBHTX_FIFO_ADDR *****/
const ru_reg_rec QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_REG =
{
    "GLOBAL_CFG_BBHTX_FIFO_ADDR",
#if RU_INCLUDE_DESC
    "BBHTX_FIFO_ADDR Register",
    "bit 10:5 of BBs target address to QM. relevant only for project where the BBHTX_SDMA are external to QM\n",
#endif
    { QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_REG_OFFSET },
    0,
    0,
    805,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DQM_FULL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DQM_FULL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_FULL *****/
const ru_field_rec QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD =
{
    "Q_FULL",
#if RU_INCLUDE_DESC
    "",
    "Queue Full indication.\nThis is a 1-bit indication per queue.\nThis register consists of a batch of 32 queues.\n",
#endif
    { QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DQM_FULL_FIELDS[] =
{
    &QM_GLOBAL_CFG_DQM_FULL_Q_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DQM_FULL *****/
const ru_reg_rec QM_GLOBAL_CFG_DQM_FULL_REG =
{
    "GLOBAL_CFG_DQM_FULL",
#if RU_INCLUDE_DESC
    "DQM_FULL 0..4 Register",
    "Queue Full indication\nEach register includes a batch of 32 queues non-empty indication.\n5 Batches are needed for 160 queues.\nFirst Batch is for queues 31-0 and so on until the last batch representing queues 159-128.\n",
#endif
    { QM_GLOBAL_CFG_DQM_FULL_REG_OFFSET },
    QM_GLOBAL_CFG_DQM_FULL_REG_RAM_CNT,
    4,
    806,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DQM_FULL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DQM_NOT_EMPTY, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DQM_NOT_EMPTY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_NOT_EMPTY *****/
const ru_field_rec QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD =
{
    "Q_NOT_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Queue Not empty indication.\nThis is a 1-bit indication per queue.\nThis register consists of a batch of 32 queues.\n",
#endif
    { QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DQM_NOT_EMPTY_FIELDS[] =
{
    &QM_GLOBAL_CFG_DQM_NOT_EMPTY_Q_NOT_EMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DQM_NOT_EMPTY *****/
const ru_reg_rec QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG =
{
    "GLOBAL_CFG_DQM_NOT_EMPTY",
#if RU_INCLUDE_DESC
    "DQM_NOT_EMPTY 0..4 Register",
    "Queue Not Empty indication\nEach register includes a batch of 32 queues non-empty indication.\n5 Batches are need for 160 queues.\nFirst Batch is for queues 31-0 and so on until the last batch representing queues 159-128.\n",
#endif
    { QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG_OFFSET },
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG_RAM_CNT,
    4,
    807,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DQM_POP_READY, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DQM_POP_READY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POP_READY *****/
const ru_field_rec QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD =
{
    "POP_READY",
#if RU_INCLUDE_DESC
    "",
    "Queue pop ready indication.\nThis is a 1-bit indication per queue.\nThis register consists of a batch of 32 queues.\n",
#endif
    { QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DQM_POP_READY_FIELDS[] =
{
    &QM_GLOBAL_CFG_DQM_POP_READY_POP_READY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DQM_POP_READY *****/
const ru_reg_rec QM_GLOBAL_CFG_DQM_POP_READY_REG =
{
    "GLOBAL_CFG_DQM_POP_READY",
#if RU_INCLUDE_DESC
    "DQM_POP_READY 0..4 Register",
    "Queue pop ready indication (Some queues may be non-empty, but due to PD offload they are not immediatly ready to be popped. Pop can be issued, but in this case the result could be delayed).\nEach register includes a batch of 32 queues non-empty indication.\n5 Batches are need for 160 queues.\nFirst Batch is for queues 31-0 and so on until the last batch representing queues 159-128.\n",
#endif
    { QM_GLOBAL_CFG_DQM_POP_READY_REG_OFFSET },
    QM_GLOBAL_CFG_DQM_POP_READY_REG_RAM_CNT,
    4,
    808,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_DQM_POP_READY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT_VALID *****/
const ru_field_rec QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD =
{
    "CONTEXT_VALID",
#if RU_INCLUDE_DESC
    "",
    "QM ingress aggregation context valid indication.\nThis is a 1-bit indication per queue.\nThis register consists of a batch of 32 queues.\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_FIELDS[] =
{
    &QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_CONTEXT_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID *****/
const ru_reg_rec QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG =
{
    "GLOBAL_CFG_AGGREGATION_CONTEXT_VALID",
#if RU_INCLUDE_DESC
    "AGGREGATION_CONTEXT_VALID 0..4 Register",
    "Aggregation context valid.\nThis indicates that the queue is in the process of packet aggregation.\nEach register includes a batch of 32 queues aggregation valid indication.\n5 Batches are need for 160 queues.\nFirst Batch is for queues 31-0 and so on until the last batch representing queues 159-128.\n",
#endif
    { QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG_OFFSET },
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG_RAM_CNT,
    4,
    809,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: QUEUE_NUM *****/
const ru_field_rec QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD =
{
    "QUEUE_NUM",
#if RU_INCLUDE_DESC
    "",
    "Queue num\n",
#endif
    { QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH_EN *****/
const ru_field_rec QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD =
{
    "FLUSH_EN",
#if RU_INCLUDE_DESC
    "",
    "flush queue enable\n",
#endif
    { QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_QUEUE_NUM_FIELD,
    &QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FLUSH_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE *****/
const ru_reg_rec QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG =
{
    "GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE",
#if RU_INCLUDE_DESC
    "QM_EGRESS_FLUSH_QUEUE Register",
    "0-8b: queue to flush\n9b:   enable flush\n\n\n\n\n",
#endif
    { QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG_OFFSET },
    0,
    0,
    810,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PRESCALER_GRANULARITY *****/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD =
{
    "PRESCALER_GRANULARITY",
#if RU_INCLUDE_DESC
    "",
    "defines the granularity of the prescaler counter:\n0 = 10bits\n1 = 11bits\n2 = 12bits\n3 = 13bits\n4 = 14bits\n5 = 15bits\n6 = 16bits\n7 = 17bits (to support 1GHz clk)\n\n\n",
#endif
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGGREGATION_TIMEOUT_VALUE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD =
{
    "AGGREGATION_TIMEOUT_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Aggregation timeout value, counted in prescaler counters cycles.\nvalid values = [1..7]\n0 - isnt supported\n\n\n",
#endif
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PD_OCCUPANCY_EN *****/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD =
{
    "PD_OCCUPANCY_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, aggregation of queues with PD occupancy more than encoded on PD_OCCUPANCY_VALUE arent closed even if the timer is expired.\nIf not set, then aggregation is closed after queues timer expires\n",
#endif
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PD_OCCUPANCY_VALUE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD =
{
    "PD_OCCUPANCY_VALUE",
#if RU_INCLUDE_DESC
    "",
    "if PD_OCCUPANCY_EN == 1 then\nAggregations of queues with more than byte_occupacny of (PD_OCCUPNACY > 0) ?2 ^ (PD_OCCUPANCY + 5):0 are not closed on timeout.\n",
#endif
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PRESCALER_GRANULARITY_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_AGGREGATION_TIMEOUT_VALUE_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_EN_FIELD,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_PD_OCCUPANCY_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL *****/
const ru_reg_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG =
{
    "GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL",
#if RU_INCLUDE_DESC
    "QM_AGGREGATION_TIMER_CTRL Register",
    "Open aggregation will be forced to close after internal timer expiration.\nThe first byte (0-7bits) controls the granularity of the internal counter (valid value 0x0-0x3)\nThe second byte (8-15bits) controls the timout value (valid values 0x0-0x7), which is counted according to granularity cycles.\nthe 16bit is enable for the mechanism\n",
#endif
    { QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG_OFFSET },
    0,
    0,
    811,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_GBL_CNT *****/
const ru_field_rec QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD =
{
    "FPM_GBL_CNT",
#if RU_INCLUDE_DESC
    "",
    "FPM global counter\n",
#endif
    { QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FPM_GBL_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT *****/
const ru_reg_rec QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG =
{
    "GLOBAL_CFG_QM_FPM_UG_GBL_CNT",
#if RU_INCLUDE_DESC
    "QM_FPM_UG_GBL_CNT Register",
    "FPM global user group counter:\nUG0-3 + UG7\n\n\n",
#endif
    { QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG_OFFSET },
    0,
    0,
    812,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DDR_SPARE_ROOM, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DDR_SPARE_ROOM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HEADROOM *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_HEADROOM_FIELD =
{
    "DDR_HEADROOM",
#if RU_INCLUDE_DESC
    "",
    "DDR headroom space\n",
#endif
    { QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_HEADROOM_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_HEADROOM_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_HEADROOM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_TAILROOM *****/
const ru_field_rec QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_TAILROOM_FIELD =
{
    "DDR_TAILROOM",
#if RU_INCLUDE_DESC
    "",
    "DDR tailroom in bytes\n\n",
#endif
    { QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_TAILROOM_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_TAILROOM_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_TAILROOM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DDR_SPARE_ROOM_FIELDS[] =
{
    &QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_HEADROOM_FIELD,
    &QM_GLOBAL_CFG_DDR_SPARE_ROOM_DDR_TAILROOM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DDR_SPARE_ROOM *****/
const ru_reg_rec QM_GLOBAL_CFG_DDR_SPARE_ROOM_REG =
{
    "GLOBAL_CFG_DDR_SPARE_ROOM",
#if RU_INCLUDE_DESC
    "DDR_SPARE_ROOM 0..3 Register",
    "DDR_SPARE_ROOM\n",
#endif
    { QM_GLOBAL_CFG_DDR_SPARE_ROOM_REG_OFFSET },
    QM_GLOBAL_CFG_DDR_SPARE_ROOM_REG_RAM_CNT,
    4,
    813,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_DDR_SPARE_ROOM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DUMMY_PROFILE_0 *****/
const ru_field_rec QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_0_FIELD =
{
    "DUMMY_PROFILE_0",
#if RU_INCLUDE_DESC
    "",
    "DDR dummy spare room profile 0\n",
#endif
    { QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_0_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_0_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DUMMY_PROFILE_1 *****/
const ru_field_rec QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_1_FIELD =
{
    "DUMMY_PROFILE_1",
#if RU_INCLUDE_DESC
    "",
    "DDR dummy spare room profile 1\n",
#endif
    { QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_1_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_1_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_FIELDS[] =
{
    &QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_0_FIELD,
    &QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_DUMMY_PROFILE_1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID *****/
const ru_reg_rec QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_REG =
{
    "GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID",
#if RU_INCLUDE_DESC
    "DUMMY_SPARE_ROOM_PROFILE_ID Register",
    "DUMMY_SPARE_ROOM_PROFILE_ID\n",
#endif
    { QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_REG_OFFSET },
    0,
    0,
    814,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_DQM_UBUS_CTRL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_DQM_UBUS_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TKN_REQOUT_H *****/
const ru_field_rec QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_H_FIELD =
{
    "TKN_REQOUT_H",
#if RU_INCLUDE_DESC
    "",
    "token reqout hspace. less then this values FSM stays at idle\n",
#endif
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_H_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_H_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_H_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TKN_REQOUT_D *****/
const ru_field_rec QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_D_FIELD =
{
    "TKN_REQOUT_D",
#if RU_INCLUDE_DESC
    "",
    "token reqout dspace. less then this values FSM stays at idle\n",
#endif
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_D_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_D_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_D_FIELD_SHIFT },
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OFFLOAD_REQOUT_H *****/
const ru_field_rec QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_H_FIELD =
{
    "OFFLOAD_REQOUT_H",
#if RU_INCLUDE_DESC
    "",
    "offload reqout hspace. less then this values FSM stays at idle\n* From Jira 1218- Note that the credit thresholds should be configured to support the FPM mini FIFO size.\n",
#endif
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_H_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_H_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_H_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OFFLOAD_REQOUT_D *****/
const ru_field_rec QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_D_FIELD =
{
    "OFFLOAD_REQOUT_D",
#if RU_INCLUDE_DESC
    "",
    "offload reqout dspace. less then this values FSM stays at idle\n\nIMPORTANT: RDPDEVEL-1057\nThe correct value should be 4 however the reset value is 5. The correct value must to be written by SW.\n\n* From Jira 1218- Note that the credit thresholds should be configured to support the FPM mini FIFO size.\n",
#endif
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_D_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_D_FIELD_WIDTH },
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_D_FIELD_SHIFT },
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_DQM_UBUS_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_H_FIELD,
    &QM_GLOBAL_CFG_DQM_UBUS_CTRL_TKN_REQOUT_D_FIELD,
    &QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_H_FIELD,
    &QM_GLOBAL_CFG_DQM_UBUS_CTRL_OFFLOAD_REQOUT_D_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_DQM_UBUS_CTRL *****/
const ru_reg_rec QM_GLOBAL_CFG_DQM_UBUS_CTRL_REG =
{
    "GLOBAL_CFG_DQM_UBUS_CTRL",
#if RU_INCLUDE_DESC
    "DQM_UBUS_CTRL Register",
    "Dont TOUCH.\n",
#endif
    { QM_GLOBAL_CFG_DQM_UBUS_CTRL_REG_OFFSET },
    0,
    0,
    815,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_GLOBAL_CFG_DQM_UBUS_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_MEM_AUTO_INIT, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_MEM_AUTO_INIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_INIT_EN *****/
const ru_field_rec QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_INIT_EN_FIELD =
{
    "MEM_INIT_EN",
#if RU_INCLUDE_DESC
    "",
    "Memory auto init enable\n",
#endif
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_INIT_EN_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_INIT_EN_FIELD_WIDTH },
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_INIT_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_SEL_INIT *****/
const ru_field_rec QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SEL_INIT_FIELD =
{
    "MEM_SEL_INIT",
#if RU_INCLUDE_DESC
    "",
    "Select which memory to AUTO INIT\n\n3b000: qm_total_valid_counter\n3b001: qm_drop_counter\n3b010: qm_dqm_valid_counter\n3b011: qm_epon_rpt_cnt_counter\n3b111: All memoires\n",
#endif
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SEL_INIT_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SEL_INIT_FIELD_WIDTH },
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SEL_INIT_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_SIZE_INIT *****/
const ru_field_rec QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SIZE_INIT_FIELD =
{
    "MEM_SIZE_INIT",
#if RU_INCLUDE_DESC
    "",
    "What is the size of the memory (according to the Number of Qs)\n\n3b000: 96\n3b001: 128\n3b010: 160\n3b011: 288\n3b100: 448\n",
#endif
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SIZE_INIT_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SIZE_INIT_FIELD_WIDTH },
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SIZE_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_MEM_AUTO_INIT_FIELDS[] =
{
    &QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_INIT_EN_FIELD,
    &QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SEL_INIT_FIELD,
    &QM_GLOBAL_CFG_MEM_AUTO_INIT_MEM_SIZE_INIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_MEM_AUTO_INIT *****/
const ru_reg_rec QM_GLOBAL_CFG_MEM_AUTO_INIT_REG =
{
    "GLOBAL_CFG_MEM_AUTO_INIT",
#if RU_INCLUDE_DESC
    "MEMORY_AUTO_INIT Register",
    "Memory init mechanism:\n2b00: qm_total_valid_counter\n2b01: qm_drop_counter\n2b10: qm_dqm_valid_counter\n2b11: qm_epon_rpt_cnt_counter\n\n",
#endif
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_REG_OFFSET },
    0,
    0,
    816,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    QM_GLOBAL_CFG_MEM_AUTO_INIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_MEM_AUTO_INIT_STS, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_MEM_AUTO_INIT_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_INIT_DONE *****/
const ru_field_rec QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_MEM_INIT_DONE_FIELD =
{
    "MEM_INIT_DONE",
#if RU_INCLUDE_DESC
    "",
    "Memory auto init done\nBit is asserted when init is done.\nBit is de-asserted when init starts\n",
#endif
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_MEM_INIT_DONE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_MEM_INIT_DONE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_MEM_INIT_DONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_FIELDS[] =
{
    &QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_MEM_INIT_DONE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_MEM_AUTO_INIT_STS *****/
const ru_reg_rec QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_REG =
{
    "GLOBAL_CFG_MEM_AUTO_INIT_STS",
#if RU_INCLUDE_DESC
    "MEMORY_AUTO_INIT_STATUS Register",
    "Memory init status\n\n\n",
#endif
    { QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_REG_OFFSET },
    0,
    0,
    817,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_0_NUM_OF_TKNS *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_0_NUM_OF_TKNS_FIELD =
{
    "POOL_0_NUM_OF_TKNS",
#if RU_INCLUDE_DESC
    "",
    "Number of tokens used for each buffer in pool0\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_0_NUM_OF_TKNS_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_0_NUM_OF_TKNS_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_0_NUM_OF_TKNS_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_1_NUM_OF_TKNS *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_1_NUM_OF_TKNS_FIELD =
{
    "POOL_1_NUM_OF_TKNS",
#if RU_INCLUDE_DESC
    "",
    "Number of tokens used for each buffer in pool1\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_1_NUM_OF_TKNS_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_1_NUM_OF_TKNS_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_1_NUM_OF_TKNS_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_2_NUM_OF_TKNS *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_2_NUM_OF_TKNS_FIELD =
{
    "POOL_2_NUM_OF_TKNS",
#if RU_INCLUDE_DESC
    "",
    "Number of tokens used for each buffer in pool2\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_2_NUM_OF_TKNS_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_2_NUM_OF_TKNS_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_2_NUM_OF_TKNS_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_3_NUM_OF_TKNS *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_3_NUM_OF_TKNS_FIELD =
{
    "POOL_3_NUM_OF_TKNS",
#if RU_INCLUDE_DESC
    "",
    "Number of tokens used for each buffer in pool3\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_3_NUM_OF_TKNS_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_3_NUM_OF_TKNS_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_3_NUM_OF_TKNS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_0_NUM_OF_TKNS_FIELD,
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_1_NUM_OF_TKNS_FIELD,
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_2_NUM_OF_TKNS_FIELD,
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_POOL_3_NUM_OF_TKNS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS *****/
const ru_reg_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_REG =
{
    "GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS",
#if RU_INCLUDE_DESC
    "FPM_MPM_ENHANCEMENT_POOL_SIZES_TOKENS Register",
    "Enhancement support for FPM and MPM\nConfigures the amount of Tokens per\npool\nFPM:\npool 0: 8\npool 1: 4\npool 2: 2\npool 3: 1\n\nMPM:\nValues can be: 1,2,3,4,5,8,10,20,40\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_REG_OFFSET },
    0,
    0,
    818,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_0_NUM_OF_BYTES *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_0_NUM_OF_BYTES_FIELD =
{
    "POOL_0_NUM_OF_BYTES",
#if RU_INCLUDE_DESC
    "",
    "Number of bytes used for each buffer in pool0 - Up to 16K\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_0_NUM_OF_BYTES_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_0_NUM_OF_BYTES_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_0_NUM_OF_BYTES_FIELD_SHIFT },
    2048,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_1_NUM_OF_BYTES *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_1_NUM_OF_BYTES_FIELD =
{
    "POOL_1_NUM_OF_BYTES",
#if RU_INCLUDE_DESC
    "",
    "Number of Bytes used for each buffer in pool1\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_1_NUM_OF_BYTES_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_1_NUM_OF_BYTES_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_1_NUM_OF_BYTES_FIELD_SHIFT },
    1024,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_0_NUM_OF_BYTES_FIELD,
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_POOL_1_NUM_OF_BYTES_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE *****/
const ru_reg_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_REG =
{
    "GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE",
#if RU_INCLUDE_DESC
    "FPM_MPM_ENHANCEMENT_POOL_0_1_SIZES_BYTES Register",
    "Enhancement support for FPM and MPM\nConfigures the amount of Bytes per pool\n\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_REG_OFFSET },
    0,
    0,
    819,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_2_NUM_OF_BYTES *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_2_NUM_OF_BYTES_FIELD =
{
    "POOL_2_NUM_OF_BYTES",
#if RU_INCLUDE_DESC
    "",
    "Number of bytes used for each buffer in pool0 - Up to 16K\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_2_NUM_OF_BYTES_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_2_NUM_OF_BYTES_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_2_NUM_OF_BYTES_FIELD_SHIFT },
    512,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_3_NUM_OF_BYTES *****/
const ru_field_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_3_NUM_OF_BYTES_FIELD =
{
    "POOL_3_NUM_OF_BYTES",
#if RU_INCLUDE_DESC
    "",
    "Number of Bytes used for each buffer in pool1\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_3_NUM_OF_BYTES_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_3_NUM_OF_BYTES_FIELD_WIDTH },
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_3_NUM_OF_BYTES_FIELD_SHIFT },
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_FIELDS[] =
{
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_2_NUM_OF_BYTES_FIELD,
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_POOL_3_NUM_OF_BYTES_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE *****/
const ru_reg_rec QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_REG =
{
    "GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE",
#if RU_INCLUDE_DESC
    "FPM_MPM_ENHANCEMENT_POOL_2_3_SIZES_BYTES Register",
    "Enhancement support for FPM and MPM\nConfigures the amount of Bytes per pool\n\n",
#endif
    { QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_REG_OFFSET },
    0,
    0,
    820,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_MC_CTRL, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_MC_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MC_HEADERS_POOL_SEL *****/
const ru_field_rec QM_GLOBAL_CFG_MC_CTRL_MC_HEADERS_POOL_SEL_FIELD =
{
    "MC_HEADERS_POOL_SEL",
#if RU_INCLUDE_DESC
    "",
    "This register sets the size of the pool to use for MC headers.\nSince the 256K FPM PD Format does not have a field indicating the pool size, there should be an explicit configuration for which pool to use.\n\n-> POOL0 - Use 8 buffers\n-> POOL1 - Use 4 buffers\n-> POOL2 - Use 2 buffers\n-> POOL3 - Use 1 buffers\n\nThis configuration needs to take into account the min_buffer_size in order to make sure that the correct pool is chosen for multicast headers.\n",
#endif
    { QM_GLOBAL_CFG_MC_CTRL_MC_HEADERS_POOL_SEL_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_MC_CTRL_MC_HEADERS_POOL_SEL_FIELD_WIDTH },
    { QM_GLOBAL_CFG_MC_CTRL_MC_HEADERS_POOL_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_MC_CTRL_FIELDS[] =
{
    &QM_GLOBAL_CFG_MC_CTRL_MC_HEADERS_POOL_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_MC_CTRL *****/
const ru_reg_rec QM_GLOBAL_CFG_MC_CTRL_REG =
{
    "GLOBAL_CFG_MC_CTRL",
#if RU_INCLUDE_DESC
    "MC_CTRL Register",
    "Multicast Control register\n",
#endif
    { QM_GLOBAL_CFG_MC_CTRL_REG_OFFSET },
    0,
    0,
    821,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_MC_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "Clock counter cycle 16bit value. default is according to the clk cycle.\nSet 1000d for 1uSec @1GHz clk.\nSet 880d for 1usec @880MHz clk.\nSet 750d for 1usec @750MHz clk.\n",
#endif
    { QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_VALUE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_VALUE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_VALUE_FIELD_SHIFT },
    750,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_FIELDS[] =
{
    &QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE *****/
const ru_reg_rec QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_REG =
{
    "GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE",
#if RU_INCLUDE_DESC
    "AQM_CLK_COUNTER_CYCLE Register",
    "Clock counter cycle 16bit value. Sets the cycles for the timestamp counter.\nDefault is according to the clk cycle:\nSet 1000d for 1uSec @1GHz clk.\nSet 880d for 1usec @880MHz clk.\nSet 750d for 1usec @750MHz clk.\n",
#endif
    { QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_REG_OFFSET },
    0,
    0,
    822,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "Global threshold value of queue fill level (ingress occupancy in bytes) to mark the PD Push to Empty bit\n",
#endif
    { QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_VALUE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_VALUE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_VALUE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_FIELDS[] =
{
    &QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR *****/
const ru_reg_rec QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_REG =
{
    "GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR",
#if RU_INCLUDE_DESC
    "AQM_PUSH_TO_EMPTY_THR Register",
    "Global threshold value of queue fill level (ingress occupancy in bytes) to mark the PD Push to Empty bit\n",
#endif
    { QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_REG_OFFSET },
    0,
    0,
    823,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_CFG_QM_GENERAL_CTRL2, TYPE: Type_QM_TOP_QM_GLOBAL_CFG_QM_GENERAL_CTRL2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE_FIELD =
{
    "EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Indicates whether the egress accumulated total packets counter is read clear.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE_FIELD =
{
    "EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Indicates whether the egress accumulated total bytes counter is read clear.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGG_CLOSURE_SUSPEND_ON_BP *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_BP_FIELD =
{
    "AGG_CLOSURE_SUSPEND_ON_BP",
#if RU_INCLUDE_DESC
    "",
    "When set, closure due to aggregation timers will be suspended when BP is applied from CM pipe.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_BP_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_BP_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_BP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUFMNG_EN_OR_UG_CNTR *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL2_BUFMNG_EN_OR_UG_CNTR_FIELD =
{
    "BUFMNG_EN_OR_UG_CNTR",
#if RU_INCLUDE_DESC
    "",
    "When set, QM use BUFMNG instead of user-group counters - direct interface (no BB).\nOld mechanism remain for backup.\n1= enable bufmng\n0= use ug counters\ndefault is 1=bufmng\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_BUFMNG_EN_OR_UG_CNTR_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_BUFMNG_EN_OR_UG_CNTR_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_BUFMNG_EN_OR_UG_CNTR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DQM_TO_FPM_UBUS_OR_FPMINI *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL2_DQM_TO_FPM_UBUS_OR_FPMINI_FIELD =
{
    "DQM_TO_FPM_UBUS_OR_FPMINI",
#if RU_INCLUDE_DESC
    "",
    "1b configuration, selecting between access of dqm to fpm through ubus (old mode), and access to fpmini (new).\nDefault is 1 (fpmini new mode).\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_DQM_TO_FPM_UBUS_OR_FPMINI_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_DQM_TO_FPM_UBUS_OR_FPMINI_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_DQM_TO_FPM_UBUS_OR_FPMINI_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE *****/
const ru_field_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE_FIELD =
{
    "AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Relevant when AGG_CLOSURE_SUSPEND_ON_BP is set.\nWhen 1,  ingress_ddr_fpm_congestion_bp does not suspend aggregation timers closure.\nWhen 0, ingress_ddr_fpm_congestion_bp suspend aggregation timers closure.\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE_FIELD_MASK },
    0,
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE_FIELD_WIDTH },
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_CFG_QM_GENERAL_CTRL2_FIELDS[] =
{
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL2_EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_BP_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL2_BUFMNG_EN_OR_UG_CNTR_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL2_DQM_TO_FPM_UBUS_OR_FPMINI_FIELD,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL2_AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_CFG_QM_GENERAL_CTRL2 *****/
const ru_reg_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL2_REG =
{
    "GLOBAL_CFG_QM_GENERAL_CTRL2",
#if RU_INCLUDE_DESC
    "QM_GENERAL_CTRL2 Register",
    "QM General Control2 register\n",
#endif
    { QM_GLOBAL_CFG_QM_GENERAL_CTRL2_REG_OFFSET },
    0,
    0,
    824,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    QM_GLOBAL_CFG_QM_GENERAL_CTRL2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_COPY_DECISION_PROFILE_THR, TYPE: Type_QM_TOP_QM_COPY_DECISION_PROFILE_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: QUEUE_OCCUPANCY_THR *****/
const ru_field_rec QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD =
{
    "QUEUE_OCCUPANCY_THR",
#if RU_INCLUDE_DESC
    "",
    "Queue Occupancy Threshold.\nWhen passing this threhold, packets will be copied to the DDR\n",
#endif
    { QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD_MASK },
    0,
    { QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD_WIDTH },
    { QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD_SHIFT },
    10240,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_THR *****/
const ru_field_rec QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD =
{
    "PSRAM_THR",
#if RU_INCLUDE_DESC
    "",
    "Indicates which of the two PSRAM threshold crossing indications coming from the SBPM will be used for the copy decision. when going over the chosen threshold, packets will be copied to the DDR.\n0 - Lower threshold\n1 - Higher threshold\n",
#endif
    { QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD_MASK },
    0,
    { QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD_WIDTH },
    { QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_COPY_DECISION_PROFILE_THR_FIELDS[] =
{
    &QM_COPY_DECISION_PROFILE_THR_QUEUE_OCCUPANCY_THR_FIELD,
    &QM_COPY_DECISION_PROFILE_THR_PSRAM_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_COPY_DECISION_PROFILE_THR *****/
const ru_reg_rec QM_COPY_DECISION_PROFILE_THR_REG =
{
    "COPY_DECISION_PROFILE_THR",
#if RU_INCLUDE_DESC
    "THR Register",
    "DDR Pipe and PSRAM threshold configurations for DDR copy decision logic\n",
#endif
    { QM_COPY_DECISION_PROFILE_THR_REG_OFFSET },
    QM_COPY_DECISION_PROFILE_THR_REG_RAM_CNT,
    32,
    825,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    QM_COPY_DECISION_PROFILE_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_INTR_CTRL_ISR, TYPE: Type_QM_TOP_QM_INTR_CTRL_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_DQM_POP_ON_EMPTY *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD =
{
    "QM_DQM_POP_ON_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "HW tried to pop a PD from the DQM of an empty queue.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_DQM_POP_ON_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_DQM_PUSH_ON_FULL *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD =
{
    "QM_DQM_PUSH_ON_FULL",
#if RU_INCLUDE_DESC
    "",
    "HW tried to pop a PD into the DQM of a full queue.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_DQM_PUSH_ON_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_CPU_POP_ON_EMPTY *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD =
{
    "QM_CPU_POP_ON_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "CPU tried to pop a PD from the DQM of an empty queue.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_CPU_POP_ON_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_CPU_PUSH_ON_FULL *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD =
{
    "QM_CPU_PUSH_ON_FULL",
#if RU_INCLUDE_DESC
    "",
    "CPU tried to push a PD into the DQM of a full queue.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_CPU_PUSH_ON_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_NORMAL_QUEUE_PD_NO_CREDIT *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD =
{
    "QM_NORMAL_QUEUE_PD_NO_CREDIT",
#if RU_INCLUDE_DESC
    "",
    "A PD arrived to the Normal queue without having any credits\n",
#endif
    { QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_NORMAL_QUEUE_PD_NO_CREDIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_NON_DELAYED_QUEUE_PD_NO_CREDIT *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD =
{
    "QM_NON_DELAYED_QUEUE_PD_NO_CREDIT",
#if RU_INCLUDE_DESC
    "",
    "A PD arrived to the NON-delayed queue without having any credits\n",
#endif
    { QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_NON_DELAYED_QUEUE_PD_NO_CREDIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_NON_VALID_QUEUE *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD =
{
    "QM_NON_VALID_QUEUE",
#if RU_INCLUDE_DESC
    "",
    "A PD arrived with a non valid queue number (>287)\n",
#endif
    { QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_NON_VALID_QUEUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_AGG_COHERENT_INCONSISTENCY *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD =
{
    "QM_AGG_COHERENT_INCONSISTENCY",
#if RU_INCLUDE_DESC
    "",
    "An aggregation of PDs was done in which the coherent bit of the PD differs between them (The coherent bit of the first aggregated PD was used)\n",
#endif
    { QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_AGG_COHERENT_INCONSISTENCY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_FORCE_COPY_ON_NON_DELAYED *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD =
{
    "QM_FORCE_COPY_ON_NON_DELAYED",
#if RU_INCLUDE_DESC
    "",
    "A PD with force copy bit set was received on the non-delayed queue (in this queue the copy machine is bypassed)\n",
#endif
    { QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_FORCE_COPY_ON_NON_DELAYED_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_FPM_POOL_SIZE_NONEXISTENT *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD =
{
    "QM_FPM_POOL_SIZE_NONEXISTENT",
#if RU_INCLUDE_DESC
    "",
    "A PD was marked to be copied, but there does not exist an FPM pool buffer large enough to hold it.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_FPM_POOL_SIZE_NONEXISTENT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_TARGET_MEM_ABS_CONTRADICTION *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD =
{
    "QM_TARGET_MEM_ABS_CONTRADICTION",
#if RU_INCLUDE_DESC
    "",
    "A PD was marked with a target_mem=1 (located in PSRAM) and on the other hand, the absolute address indication was set.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_TARGET_MEM_ABS_CONTRADICTION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_1588_DROP *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD =
{
    "QM_1588_DROP",
#if RU_INCLUDE_DESC
    "",
    "1588 Packet is dropped when the QM PD occupancy exceeds threshold (64K)\n",
#endif
    { QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_1588_DROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_1588_MULTICAST_CONTRADICTION *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD =
{
    "QM_1588_MULTICAST_CONTRADICTION",
#if RU_INCLUDE_DESC
    "",
    "A PD was marked as a 1588 and multicast together.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_1588_MULTICAST_CONTRADICTION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_BYTE_DROP_CNT_OVERRUN *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD =
{
    "QM_BYTE_DROP_CNT_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "The byte drop counter of one of the queues reached its maximum value and a new value was pushed.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_BYTE_DROP_CNT_OVERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_PKT_DROP_CNT_OVERRUN *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD =
{
    "QM_PKT_DROP_CNT_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "The Packet drop counter of one of the queues reached its maximum value and a new value was pushed.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_PKT_DROP_CNT_OVERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_TOTAL_BYTE_CNT_UNDERRUN *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD =
{
    "QM_TOTAL_BYTE_CNT_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "The Total byte counter was decremented to a negative value.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_TOTAL_BYTE_CNT_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_TOTAL_PKT_CNT_UNDERRUN *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD =
{
    "QM_TOTAL_PKT_CNT_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "The Total PD counter was decremented to a negative value.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_TOTAL_PKT_CNT_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_FPM_UG0_UNDERRUN *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD =
{
    "QM_FPM_UG0_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "The UG0 counter was decremented to a negative value.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_FPM_UG0_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_FPM_UG1_UNDERRUN *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD =
{
    "QM_FPM_UG1_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "The UG1 counter was decremented to a negative value.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_FPM_UG1_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_FPM_UG2_UNDERRUN *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD =
{
    "QM_FPM_UG2_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "The UG2 counter was decremented to a negative value.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_FPM_UG2_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_FPM_UG3_UNDERRUN *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD =
{
    "QM_FPM_UG3_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "The UG3 counter was decremented to a negative value.\n",
#endif
    { QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_FPM_UG3_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_TIMER_WRAPAROUND *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD =
{
    "QM_TIMER_WRAPAROUND",
#if RU_INCLUDE_DESC
    "",
    "QM aggregation timers wraps around. In this case it isnt guaranteed that the aggregation will be closed on pre-defined timeout expiration. However the aggregation should be closed eventually.\n\n",
#endif
    { QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_TIMER_WRAPAROUND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_COPY_PLEN_ZERO *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD =
{
    "QM_COPY_PLEN_ZERO",
#if RU_INCLUDE_DESC
    "",
    "Packet with length = 0 is copied to DDR. FW/SW should take care that zero length packets arent copied to DDR.\n\n",
#endif
    { QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_COPY_PLEN_ZERO_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_INGRESS_BB_UNEXPECTED_MSG *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_INGRESS_BB_UNEXPECTED_MSG_FIELD =
{
    "QM_INGRESS_BB_UNEXPECTED_MSG",
#if RU_INCLUDE_DESC
    "",
    "Unexpected Message arrived at QM ingress BB.\n\n",
#endif
    { QM_INTR_CTRL_ISR_QM_INGRESS_BB_UNEXPECTED_MSG_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_INGRESS_BB_UNEXPECTED_MSG_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_INGRESS_BB_UNEXPECTED_MSG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_EGRESS_BB_UNEXPECTED_MSG *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_EGRESS_BB_UNEXPECTED_MSG_FIELD =
{
    "QM_EGRESS_BB_UNEXPECTED_MSG",
#if RU_INCLUDE_DESC
    "",
    "Unexpected Message arrived at QM egress BB.\n\n",
#endif
    { QM_INTR_CTRL_ISR_QM_EGRESS_BB_UNEXPECTED_MSG_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_EGRESS_BB_UNEXPECTED_MSG_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_EGRESS_BB_UNEXPECTED_MSG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DQM_REACHED_FULL *****/
const ru_field_rec QM_INTR_CTRL_ISR_DQM_REACHED_FULL_FIELD =
{
    "DQM_REACHED_FULL",
#if RU_INCLUDE_DESC
    "",
    "DQM reached a full condition (used for debug).\n",
#endif
    { QM_INTR_CTRL_ISR_DQM_REACHED_FULL_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_DQM_REACHED_FULL_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_DQM_REACHED_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QM_FPMINI_INTR *****/
const ru_field_rec QM_INTR_CTRL_ISR_QM_FPMINI_INTR_FIELD =
{
    "QM_FPMINI_INTR",
#if RU_INCLUDE_DESC
    "",
    "FPMINI interrupt\n",
#endif
    { QM_INTR_CTRL_ISR_QM_FPMINI_INTR_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISR_QM_FPMINI_INTR_FIELD_WIDTH },
    { QM_INTR_CTRL_ISR_QM_FPMINI_INTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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
    &QM_INTR_CTRL_ISR_QM_INGRESS_BB_UNEXPECTED_MSG_FIELD,
    &QM_INTR_CTRL_ISR_QM_EGRESS_BB_UNEXPECTED_MSG_FIELD,
    &QM_INTR_CTRL_ISR_DQM_REACHED_FULL_FIELD,
    &QM_INTR_CTRL_ISR_QM_FPMINI_INTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_INTR_CTRL_ISR *****/
const ru_reg_rec QM_INTR_CTRL_ISR_REG =
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active QM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.\n",
#endif
    { QM_INTR_CTRL_ISR_REG_OFFSET },
    0,
    0,
    826,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    27,
    QM_INTR_CTRL_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_INTR_CTRL_ISM, TYPE: Type_QM_TOP_QM_INTR_CTRL_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ISM *****/
const ru_field_rec QM_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "",
    "Status Masked of corresponding interrupt source in the ISR\n",
#endif
    { QM_INTR_CTRL_ISM_ISM_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ISM_ISM_FIELD_WIDTH },
    { QM_INTR_CTRL_ISM_ISM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_INTR_CTRL_ISM_FIELDS[] =
{
    &QM_INTR_CTRL_ISM_ISM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_INTR_CTRL_ISM *****/
const ru_reg_rec QM_INTR_CTRL_ISM_REG =
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { QM_INTR_CTRL_ISM_REG_OFFSET },
    0,
    0,
    827,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_INTR_CTRL_ISM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_INTR_CTRL_IER, TYPE: Type_QM_TOP_QM_INTR_CTRL_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IEM *****/
const ru_field_rec QM_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask controls the corresponding interrupt source in the IER\n",
#endif
    { QM_INTR_CTRL_IER_IEM_FIELD_MASK },
    0,
    { QM_INTR_CTRL_IER_IEM_FIELD_WIDTH },
    { QM_INTR_CTRL_IER_IEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_INTR_CTRL_IER_FIELDS[] =
{
    &QM_INTR_CTRL_IER_IEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_INTR_CTRL_IER *****/
const ru_reg_rec QM_INTR_CTRL_IER_REG =
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { QM_INTR_CTRL_IER_REG_OFFSET },
    0,
    0,
    828,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_INTR_CTRL_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_INTR_CTRL_ITR, TYPE: Type_QM_TOP_QM_INTR_CTRL_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec QM_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask tests the corresponding interrupt source in the ISR\n",
#endif
    { QM_INTR_CTRL_ITR_IST_FIELD_MASK },
    0,
    { QM_INTR_CTRL_ITR_IST_FIELD_WIDTH },
    { QM_INTR_CTRL_ITR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_INTR_CTRL_ITR_FIELDS[] =
{
    &QM_INTR_CTRL_ITR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_INTR_CTRL_ITR *****/
const ru_reg_rec QM_INTR_CTRL_ITR_REG =
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR\n",
#endif
    { QM_INTR_CTRL_ITR_REG_OFFSET },
    0,
    0,
    829,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_INTR_CTRL_ITR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_TIMESTAMP_RES_PROFILE_VALUE, TYPE: Type_QM_TOP_QM_TIMESTAMP_RES_PROFILE_VALUE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: START *****/
const ru_field_rec QM_TIMESTAMP_RES_PROFILE_VALUE_START_FIELD =
{
    "START",
#if RU_INCLUDE_DESC
    "",
    "AQM Timestamp resolution Profile, 4 profiles to select which 10bits of the timestamp counter 32bit will be written to the PD.\nValue represents the start of the 10bit offset in the 32bit:\n0d = [9:0], 1d = [10:1], 2d = [11:2] , 22d = [31:22]\ntotal 23 values (0d-22d).\n",
#endif
    { QM_TIMESTAMP_RES_PROFILE_VALUE_START_FIELD_MASK },
    0,
    { QM_TIMESTAMP_RES_PROFILE_VALUE_START_FIELD_WIDTH },
    { QM_TIMESTAMP_RES_PROFILE_VALUE_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_TIMESTAMP_RES_PROFILE_VALUE_FIELDS[] =
{
    &QM_TIMESTAMP_RES_PROFILE_VALUE_START_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_TIMESTAMP_RES_PROFILE_VALUE *****/
const ru_reg_rec QM_TIMESTAMP_RES_PROFILE_VALUE_REG =
{
    "TIMESTAMP_RES_PROFILE_VALUE",
#if RU_INCLUDE_DESC
    "VALUE Register",
    "AQM Timestamp resolution Profile, 4 profiles to select which 10bits of the timestamp counter 32bit will be written to the PD.\nValue represents the start of the 10bit offset in the 32bit:\n0d = [9:0], 1d = [10:1], 2d = [11:2] , 22d = [31:22]\ntotal 23 values (0d-22d).\n",
#endif
    { QM_TIMESTAMP_RES_PROFILE_VALUE_REG_OFFSET },
    QM_TIMESTAMP_RES_PROFILE_VALUE_REG_RAM_CNT,
    16,
    830,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_TIMESTAMP_RES_PROFILE_VALUE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CM_COMMON_INPUT_FIFO_DATA, TYPE: Type_QM_TOP_QM_CM_COMMON_INPUT_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CM_COMMON_INPUT_FIFO_DATA_FIELDS[] =
{
    &QM_CM_COMMON_INPUT_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CM_COMMON_INPUT_FIFO_DATA *****/
const ru_reg_rec QM_CM_COMMON_INPUT_FIFO_DATA_REG =
{
    "CM_COMMON_INPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..7 Register",
    "CM Common Input FIFO - debug access\n",
#endif
    { QM_CM_COMMON_INPUT_FIFO_DATA_REG_OFFSET },
    QM_CM_COMMON_INPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    831,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_CM_COMMON_INPUT_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_NORMAL_RMT_FIFO_DATA, TYPE: Type_QM_TOP_QM_NORMAL_RMT_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NORMAL_RMT_FIFO_DATA_FIELDS[] =
{
    &QM_NORMAL_RMT_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_NORMAL_RMT_FIFO_DATA *****/
const ru_reg_rec QM_NORMAL_RMT_FIFO_DATA_REG =
{
    "NORMAL_RMT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..31 Register",
    "Normal Remote FIFO - debug access\n",
#endif
    { QM_NORMAL_RMT_FIFO_DATA_REG_OFFSET },
    QM_NORMAL_RMT_FIFO_DATA_REG_RAM_CNT,
    4,
    832,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_NORMAL_RMT_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_NON_DELAYED_RMT_FIFO_DATA, TYPE: Type_QM_TOP_QM_NON_DELAYED_RMT_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NON_DELAYED_RMT_FIFO_DATA_FIELDS[] =
{
    &QM_NON_DELAYED_RMT_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_NON_DELAYED_RMT_FIFO_DATA *****/
const ru_reg_rec QM_NON_DELAYED_RMT_FIFO_DATA_REG =
{
    "NON_DELAYED_RMT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..31 Register",
    "Non-delayed Remote FIFO - debug access\n",
#endif
    { QM_NON_DELAYED_RMT_FIFO_DATA_REG_OFFSET },
    QM_NON_DELAYED_RMT_FIFO_DATA_REG_RAM_CNT,
    4,
    833,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_NON_DELAYED_RMT_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_EGRESS_DATA_FIFO_DATA, TYPE: Type_QM_TOP_QM_EGRESS_DATA_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_DATA_FIFO_DATA_FIELDS[] =
{
    &QM_EGRESS_DATA_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_EGRESS_DATA_FIFO_DATA *****/
const ru_reg_rec QM_EGRESS_DATA_FIFO_DATA_REG =
{
    "EGRESS_DATA_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..7 Register",
    "Egress data FIFO - debug access\n",
#endif
    { QM_EGRESS_DATA_FIFO_DATA_REG_OFFSET },
    QM_EGRESS_DATA_FIFO_DATA_REG_RAM_CNT,
    4,
    834,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EGRESS_DATA_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_EGRESS_RR_FIFO_DATA, TYPE: Type_QM_TOP_QM_EGRESS_RR_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_EGRESS_RR_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_EGRESS_RR_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_EGRESS_RR_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_EGRESS_RR_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_RR_FIFO_DATA_FIELDS[] =
{
    &QM_EGRESS_RR_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_EGRESS_RR_FIFO_DATA *****/
const ru_reg_rec QM_EGRESS_RR_FIFO_DATA_REG =
{
    "EGRESS_RR_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..1 Register",
    "Egress RR FIFO - debug access\n",
#endif
    { QM_EGRESS_RR_FIFO_DATA_REG_OFFSET },
    QM_EGRESS_RR_FIFO_DATA_REG_RAM_CNT,
    4,
    835,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EGRESS_RR_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_EGRESS_BB_INPUT_FIFO_DATA, TYPE: Type_QM_TOP_QM_EGRESS_BB_INPUT_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_BB_INPUT_FIFO_DATA_FIELDS[] =
{
    &QM_EGRESS_BB_INPUT_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_EGRESS_BB_INPUT_FIFO_DATA *****/
const ru_reg_rec QM_EGRESS_BB_INPUT_FIFO_DATA_REG =
{
    "EGRESS_BB_INPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..7 Register",
    "Egress BB Input FIFO - debug access\n",
#endif
    { QM_EGRESS_BB_INPUT_FIFO_DATA_REG_OFFSET },
    QM_EGRESS_BB_INPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    836,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EGRESS_BB_INPUT_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_EGRESS_BB_OUTPUT_FIFO_DATA, TYPE: Type_QM_TOP_QM_EGRESS_BB_OUTPUT_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_EGRESS_BB_OUTPUT_FIFO_DATA_FIELDS[] =
{
    &QM_EGRESS_BB_OUTPUT_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_EGRESS_BB_OUTPUT_FIFO_DATA *****/
const ru_reg_rec QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG =
{
    "EGRESS_BB_OUTPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..31 Register",
    "Egress BB Output FIFO - debug access\n",
#endif
    { QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG_OFFSET },
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    837,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_BB_OUTPUT_FIFO_DATA, TYPE: Type_QM_TOP_QM_BB_OUTPUT_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB_OUTPUT_FIFO_DATA_FIELDS[] =
{
    &QM_BB_OUTPUT_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_BB_OUTPUT_FIFO_DATA *****/
const ru_reg_rec QM_BB_OUTPUT_FIFO_DATA_REG =
{
    "BB_OUTPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..31 Register",
    "QM BB Output FIFO - debug access\n",
#endif
    { QM_BB_OUTPUT_FIFO_DATA_REG_OFFSET },
    QM_BB_OUTPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    838,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BB_OUTPUT_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_NON_DELAYED_OUT_FIFO_DATA, TYPE: Type_QM_TOP_QM_NON_DELAYED_OUT_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_NON_DELAYED_OUT_FIFO_DATA_FIELDS[] =
{
    &QM_NON_DELAYED_OUT_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_NON_DELAYED_OUT_FIFO_DATA *****/
const ru_reg_rec QM_NON_DELAYED_OUT_FIFO_DATA_REG =
{
    "NON_DELAYED_OUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..31 Register",
    "Non delayed output FIFO - debug access\n",
#endif
    { QM_NON_DELAYED_OUT_FIFO_DATA_REG_OFFSET },
    QM_NON_DELAYED_OUT_FIFO_DATA_REG_RAM_CNT,
    4,
    839,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_NON_DELAYED_OUT_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_BB0_EGR_MSG_OUT_FIFO_DATA, TYPE: Type_QM_TOP_QM_BB0_EGR_MSG_OUT_FIFO_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_BB0_EGR_MSG_OUT_FIFO_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_BB0_EGR_MSG_OUT_FIFO_DATA_DATA_FIELD_MASK },
    0,
    { QM_BB0_EGR_MSG_OUT_FIFO_DATA_DATA_FIELD_WIDTH },
    { QM_BB0_EGR_MSG_OUT_FIFO_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_BB0_EGR_MSG_OUT_FIFO_DATA_FIELDS[] =
{
    &QM_BB0_EGR_MSG_OUT_FIFO_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_BB0_EGR_MSG_OUT_FIFO_DATA *****/
const ru_reg_rec QM_BB0_EGR_MSG_OUT_FIFO_DATA_REG =
{
    "BB0_EGR_MSG_OUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA 0..3 Register",
    "QM free messages from egress to BB0 Output FIFO - debug access\n",
#endif
    { QM_BB0_EGR_MSG_OUT_FIFO_DATA_REG_OFFSET },
    QM_BB0_EGR_MSG_OUT_FIFO_DATA_REG_RAM_CNT,
    4,
    840,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_BB0_EGR_MSG_OUT_FIFO_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_CLK_GATE_CLK_GATE_CNTRL, TYPE: Type_QM_TOP_QM_CLK_GATE_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\n",
#endif
    { QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\n\n\n",
#endif
    { QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\n",
#endif
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\n",
#endif
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\n",
#endif
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_CLK_GATE_CLK_GATE_CNTRL_FIELDS[] =
{
    &QM_CLK_GATE_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &QM_CLK_GATE_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_CLK_GATE_CLK_GATE_CNTRL *****/
const ru_reg_rec QM_CLK_GATE_CLK_GATE_CNTRL_REG =
{
    "CLK_GATE_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { QM_CLK_GATE_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    841,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    QM_CLK_GATE_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER, TYPE: Type_QM_TOP_QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_DATA_FIELD_MASK },
    0,
    { QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_DATA_FIELD_WIDTH },
    { QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_FIELDS[] =
{
    &QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER *****/
const ru_reg_rec QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_REG =
{
    "GLOBAL_EGRESS_DROP_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER Register",
    "Counter for non AQM drop, not per queue. Increment if Drop = 1 and AQM_drop = 0.\n\n",
#endif
    { QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_REG_OFFSET },
    0,
    0,
    842,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER, TYPE: Type_QM_TOP_QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "DATA\n",
#endif
    { QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_DATA_FIELD_MASK },
    0,
    { QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_DATA_FIELD_WIDTH },
    { QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_FIELDS[] =
{
    &QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER *****/
const ru_reg_rec QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_REG =
{
    "GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER Register",
    "Global egress counter for AQM egress drop message, not per queue. Increment if Drop = 1 and AQM_drop = 1.\n",
#endif
    { QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_REG_OFFSET },
    0,
    0,
    843,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_FIELDS,
#endif
};

unsigned long QM_ADDRS[] =
{
    0x82C00000,
};

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
    &QM_GLOBAL_CFG_AGGREGATION_CTRL2_REG,
    &QM_GLOBAL_CFG_FPM_BASE_ADDR_REG,
    &QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG,
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG,
    &QM_GLOBAL_CFG_BBHTX_FIFO_ADDR_REG,
    &QM_GLOBAL_CFG_DQM_FULL_REG,
    &QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG,
    &QM_GLOBAL_CFG_DQM_POP_READY_REG,
    &QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG,
    &QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG,
    &QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG,
    &QM_GLOBAL_CFG_DDR_SPARE_ROOM_REG,
    &QM_GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID_REG,
    &QM_GLOBAL_CFG_DQM_UBUS_CTRL_REG,
    &QM_GLOBAL_CFG_MEM_AUTO_INIT_REG,
    &QM_GLOBAL_CFG_MEM_AUTO_INIT_STS_REG,
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS_REG,
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE_REG,
    &QM_GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE_REG,
    &QM_GLOBAL_CFG_MC_CTRL_REG,
    &QM_GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE_REG,
    &QM_GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR_REG,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL2_REG,
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
    &QM_TIMESTAMP_RES_PROFILE_VALUE_REG,
    &QM_GLOBAL_EGRESS_DROP_COUNTER_COUNTER_REG,
    &QM_GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER_REG,
    &QM_TOTAL_VALID_COUNTER_COUNTER_REG,
    &QM_DQM_VALID_COUNTER_COUNTER_REG,
    &QM_DROP_COUNTER_COUNTER_REG,
    &QM_TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER_REG,
    &QM_EPON_RPT_CNT_COUNTER_REG,
    &QM_EPON_RPT_CNT_QUEUE_STATUS_REG,
    &QM_RD_DATA_POOL0_REG,
    &QM_RD_DATA_POOL1_REG,
    &QM_RD_DATA_POOL2_REG,
    &QM_RD_DATA_POOL3_REG,
    &QM_POP_3_REG,
    &QM_PDFIFO_PTR_REG,
    &QM_UPDATE_FIFO_PTR_REG,
    &QM_RD_DATA_2_REG,
    &QM_POP_2_REG,
    &QM_RD_DATA_1_REG,
    &QM_POP_1_REG,
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
    &QM_BB0_EGR_MSG_OUT_FIFO_DATA_REG,
    &QM_FPM_BUFFER_RESERVATION_DATA_REG,
    &QM_PORT_CFG_REG,
    &QM_FC_UG_MASK_UG_EN_REG,
    &QM_FC_QUEUE_MASK_REG,
    &QM_FC_QUEUE_RANGE1_START_REG,
    &QM_FC_QUEUE_RANGE2_START_REG,
    &QM_DBG_REG,
    &QM_UG_OCCUPANCY_STATUS_REG,
    &QM_QUEUE_RANGE1_OCCUPANCY_STATUS_REG,
    &QM_QUEUE_RANGE2_OCCUPANCY_STATUS_REG,
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
    &QM_FPM_GRP_DROP_CNT_REG,
    &QM_FPM_POOL_DROP_CNT_REG,
    &QM_FPM_BUFFER_RES_DROP_CNT_REG,
    &QM_PSRAM_EGRESS_CONG_DRP_CNT_REG,
    &QM_BACKPRESSURE_REG,
    &QM_AQM_TIMESTAMP_CURR_COUNTER_REG,
    &QM_BB0_EGR_MSG_OUT_FIFO_STATUS_REG,
    &QM_COUNT_PKT_NOT_PD_MODE_BITS_REG,
    &QM_DATA_REG,
};

const ru_block_rec QM_BLOCK =
{
    "QM",
    QM_ADDRS,
    1,
    170,
    QM_REGS,
};
