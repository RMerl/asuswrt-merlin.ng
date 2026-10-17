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


#include "XRDP_DSPTCHR_AG.h"

/******************************************************************************
 * Register: NAME: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_REORDER_CFG_DSPTCHR_REORDR_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "Enable dispatcher reorder block\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AUTO_INIT_EN *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_EN_FIELD =
{
    "AUTO_INIT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable auto init of several block inside the Dispatcher.\nCurrently includes Prev and Next BD rams\n\nOnce set it will init the BD Ram memories. It will clear when finished\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_EN_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_EN_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AUTO_INIT_SIZE *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_SIZE_FIELD =
{
    "AUTO_INIT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Limits configuration of Prev and Next BD rams according to their size\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_SIZE_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_SIZE_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RDY *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "",
    "Dispatcher reorder block is RDY\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHN_MODE *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CHN_MODE_FIELD =
{
    "CHN_MODE",
#if RU_INCLUDE_DESC
    "",
    "Channel mode\n00 - 100G\n01 - 200G\n10 - 400G\n11 - Reserved\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CHN_MODE_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CHN_MODE_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CHN_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DSP_ID *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_ID_FIELD =
{
    "DSP_ID",
#if RU_INCLUDE_DESC
    "",
    "0 - Dispatchers 0-4\n1 - Dispatchers 5-9\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_ID_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_ID_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CRDT_INIT_PORT *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_INIT_PORT_FIELD =
{
    "CRDT_INIT_PORT",
#if RU_INCLUDE_DESC
    "",
    "Send Initial credits to specific port\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_INIT_PORT_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_INIT_PORT_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_INIT_PORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CRDT_PORT *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_PORT_FIELD =
{
    "CRDT_PORT",
#if RU_INCLUDE_DESC
    "",
    "Port to send credits to\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_PORT_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_PORT_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_PORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PER_Q_EGRS_CONGST_EN *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD =
{
    "PER_Q_EGRS_CONGST_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable per Q Egress congestion monitoring\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INGRS_PIPE_DLY_EN *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_EN_FIELD =
{
    "INGRS_PIPE_DLY_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable delay added to the ingress pipe to\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_EN_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_EN_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INGRS_PIPE_DLY_CNT *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_CNT_FIELD =
{
    "INGRS_PIPE_DLY_CNT",
#if RU_INCLUDE_DESC
    "",
    "Ingress delay count.\nAdds delay to INGRESS PIPE\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_CNT_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_CNT_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_CNT_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EGRS_DROP_ONLY *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EGRS_DROP_ONLY_FIELD =
{
    "EGRS_DROP_ONLY",
#if RU_INCLUDE_DESC
    "",
    "Disables new Ingress drop mech and only allow drop from the re-order\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EGRS_DROP_ONLY_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EGRS_DROP_ONLY_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EGRS_DROP_ONLY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CRDT_EFF_REP *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_EFF_REP_FIELD =
{
    "CRDT_EFF_REP",
#if RU_INCLUDE_DESC
    "",
    "Will allow de-assert common_buf_empty when re-order is returning a buffer at the same time msgdec is requesting a common buffer.\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_EFF_REP_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_EFF_REP_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_EFF_REP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FRZ_ON_ERR *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FRZ_ON_ERR_FIELD =
{
    "FRZ_ON_ERR",
#if RU_INCLUDE_DESC
    "",
    "Freeze Dispatcher when there is an error indication (unknown message on BB or writing reorder PD to an empty VIQ)\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FRZ_ON_ERR_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FRZ_ON_ERR_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FRZ_ON_ERR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DSP_HALF *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_HALF_FIELD =
{
    "DSP_HALF",
#if RU_INCLUDE_DESC
    "",
    "To which half the Dispatcher belongs to\n0 - Half 0\n1 - Half 1\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_HALF_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_HALF_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_HALF_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DSP_NUM *****/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_NUM_FIELD =
{
    "DSP_NUM",
#if RU_INCLUDE_DESC
    "",
    "Number of Dispatcher 100G instance (0-8)\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_NUM_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_NUM_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_EN_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_AUTO_INIT_SIZE_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CHN_MODE_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_ID_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_INIT_PORT_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_PORT_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_EN_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_INGRS_PIPE_DLY_CNT_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EGRS_DROP_ONLY_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_CRDT_EFF_REP_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FRZ_ON_ERR_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_HALF_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSP_NUM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG *****/
const ru_reg_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG =
{
    "REORDER_CFG_DSPTCHR_REORDR_CFG",
#if RU_INCLUDE_DESC
    "DISPATCHER_REORDER_EN Register",
    "Enable of dispatcher reorder\n",
#endif
    { DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG_OFFSET },
    0,
    0,
    1280,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_REORDER_CFG_VQ_EN, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_REORDER_CFG_VQ_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "Enable Virtual Q control - 32 bit vector.\n",
#endif
    { DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_VQ_EN_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_REORDER_CFG_VQ_EN *****/
const ru_reg_rec DSPTCHR_REORDER_CFG_VQ_EN_REG =
{
    "REORDER_CFG_VQ_EN",
#if RU_INCLUDE_DESC
    "VIRTUAL_Q_EN Register",
    "Enable control for each VIQ/VEQ\n",
#endif
    { DSPTCHR_REORDER_CFG_VQ_EN_REG_OFFSET },
    0,
    0,
    1281,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_REORDER_CFG_VQ_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_REORDER_CFG_BB_CFG, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_REORDER_CFG_BB_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRC_ID *****/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD =
{
    "SRC_ID",
#if RU_INCLUDE_DESC
    "",
    "Source ID - Dispatcher\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD_SHIFT },
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DST_ID_OVRIDE *****/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD =
{
    "DST_ID_OVRIDE",
#if RU_INCLUDE_DESC
    "",
    "Enable dispatcher reorder block\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVRIDE_EN *****/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD =
{
    "OVRIDE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable dispatcher reorder block\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_BB_CFG_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_REORDER_CFG_BB_CFG *****/
const ru_reg_rec DSPTCHR_REORDER_CFG_BB_CFG_REG =
{
    "REORDER_CFG_BB_CFG",
#if RU_INCLUDE_DESC
    "BROADBUS_CONFIG Register",
    "Allow override of a specific BB destination with a new Route ADDR\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CFG_REG_OFFSET },
    0,
    0,
    1282,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_REORDER_CFG_BB_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_REORDER_CFG_BB_CFG_RT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_REORDER_CFG_BB_CFG_RT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE_OVERRIDE *****/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_RT_ROUTE_OVERRIDE_FIELD =
{
    "ROUTE_OVERRIDE",
#if RU_INCLUDE_DESC
    "",
    "BB Route address override\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CFG_RT_ROUTE_OVERRIDE_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_BB_CFG_RT_ROUTE_OVERRIDE_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_BB_CFG_RT_ROUTE_OVERRIDE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_BB_CFG_RT_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_BB_CFG_RT_ROUTE_OVERRIDE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_REORDER_CFG_BB_CFG_RT *****/
const ru_reg_rec DSPTCHR_REORDER_CFG_BB_CFG_RT_REG =
{
    "REORDER_CFG_BB_CFG_RT",
#if RU_INCLUDE_DESC
    "BROADBUS_CONFIG_RT Register",
    "BB Route address override\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CFG_RT_REG_OFFSET },
    0,
    0,
    1283,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_REORDER_CFG_BB_CFG_RT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_REORDER_CFG_BB_CMN_CFG, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_REORDER_CFG_BB_CMN_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRC_ID *****/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CMN_CFG_SRC_ID_FIELD =
{
    "SRC_ID",
#if RU_INCLUDE_DESC
    "",
    "Source ID - Dispatcher\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_SRC_ID_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_SRC_ID_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_SRC_ID_FIELD_SHIFT },
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DST_ID_OVRIDE *****/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CMN_CFG_DST_ID_OVRIDE_FIELD =
{
    "DST_ID_OVRIDE",
#if RU_INCLUDE_DESC
    "",
    "Enable dispatcher reorder block\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_DST_ID_OVRIDE_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_DST_ID_OVRIDE_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_DST_ID_OVRIDE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVRIDE_EN *****/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CMN_CFG_OVRIDE_EN_FIELD =
{
    "OVRIDE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable dispatcher reorder block\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_OVRIDE_EN_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_OVRIDE_EN_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_OVRIDE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_BB_CMN_CFG_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_BB_CMN_CFG_SRC_ID_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CMN_CFG_DST_ID_OVRIDE_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CMN_CFG_OVRIDE_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_REORDER_CFG_BB_CMN_CFG *****/
const ru_reg_rec DSPTCHR_REORDER_CFG_BB_CMN_CFG_REG =
{
    "REORDER_CFG_BB_CMN_CFG",
#if RU_INCLUDE_DESC
    "BROADBUS_CMN_CONFIG Register",
    "Allow override of a specific BB destination with a new Route ADDR\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_REG_OFFSET },
    0,
    0,
    1284,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_REORDER_CFG_BB_CMN_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_REORDER_CFG_BB_CMN_CFG_RT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE_OVERRIDE *****/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_ROUTE_OVERRIDE_FIELD =
{
    "ROUTE_OVERRIDE",
#if RU_INCLUDE_DESC
    "",
    "BB Route address override\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_ROUTE_OVERRIDE_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_ROUTE_OVERRIDE_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_ROUTE_OVERRIDE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_ROUTE_OVERRIDE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT *****/
const ru_reg_rec DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_REG =
{
    "REORDER_CFG_BB_CMN_CFG_RT",
#if RU_INCLUDE_DESC
    "BROADBUS_CMN_CONFIG_RT Register",
    "BB Route address override\n",
#endif
    { DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_REG_OFFSET },
    0,
    0,
    1285,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_REORDER_CFG_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\n\nMin value for Dispatcher is 0x14\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL *****/
const ru_reg_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG =
{
    "REORDER_CFG_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    1286,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_INGRS_CONGSTN, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_INGRS_CONGSTN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FRST_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD =
{
    "FRST_LVL",
#if RU_INCLUDE_DESC
    "",
    "First Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SCND_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD =
{
    "SCND_LVL",
#if RU_INCLUDE_DESC
    "",
    "Second Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HYST_THRS *****/
const ru_field_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed\n",
#endif
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_INGRS_CONGSTN_FIELDS[] =
{
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_INGRS_CONGSTN *****/
const ru_reg_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_REG =
{
    "CONGESTION_INGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "INGRESS_CONGESTION_THRESHOLD 0..8 Register",
    "Ingress Queues congestion state.\n\n",
#endif
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_REG_OFFSET },
    DSPTCHR_CONGESTION_INGRS_CONGSTN_REG_RAM_CNT,
    4,
    1287,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_INGRS_CONGSTN_2ND
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THRD_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_THRD_LVL_FIELD =
{
    "THRD_LVL",
#if RU_INCLUDE_DESC
    "",
    "Third Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_THRD_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_THRD_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_THRD_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HYST_THRS *****/
const ru_field_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed\n",
#endif
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_HYST_THRS_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_HYST_THRS_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_HYST_THRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_FIELDS[] =
{
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_THRD_LVL_FIELD,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_HYST_THRS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND *****/
const ru_reg_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_REG =
{
    "CONGESTION_INGRS_CONGSTN_2ND",
#if RU_INCLUDE_DESC
    "INGRESS_CONGESTION_THRESHOLD_2ND 0..8 Register",
    "Ingress Queues congestion state.\n\n",
#endif
    { DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_REG_OFFSET },
    DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_REG_RAM_CNT,
    4,
    1288,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_EGRS_CONGSTN, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_EGRS_CONGSTN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FRST_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD =
{
    "FRST_LVL",
#if RU_INCLUDE_DESC
    "",
    "First Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SCND_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD =
{
    "SCND_LVL",
#if RU_INCLUDE_DESC
    "",
    "Second Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HYST_THRS *****/
const ru_field_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed\n",
#endif
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_EGRS_CONGSTN_FIELDS[] =
{
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_EGRS_CONGSTN *****/
const ru_reg_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_REG =
{
    "CONGESTION_EGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "EGRESS_CONGESTION_THRESHOLD 0..8 Register",
    "Egress Queues congestion state per Q.\n\n",
#endif
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_REG_OFFSET },
    DSPTCHR_CONGESTION_EGRS_CONGSTN_REG_RAM_CNT,
    4,
    1289,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_EGRS_CONGSTN_2ND
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THRD_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_THRD_LVL_FIELD =
{
    "THRD_LVL",
#if RU_INCLUDE_DESC
    "",
    "THIRD Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_THRD_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_THRD_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_THRD_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HYST_THRS *****/
const ru_field_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed\n",
#endif
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_HYST_THRS_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_HYST_THRS_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_HYST_THRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_FIELDS[] =
{
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_THRD_LVL_FIELD,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_HYST_THRS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND *****/
const ru_reg_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_REG =
{
    "CONGESTION_EGRS_CONGSTN_2ND",
#if RU_INCLUDE_DESC
    "EGRESS_CONGESTION_THRESHOLD_2ND 0..8 Register",
    "Egress Queues congestion state per Q.\n\n",
#endif
    { DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_REG_OFFSET },
    DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_REG_RAM_CNT,
    4,
    1290,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_TOTAL_EGRS_CONGSTN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FRST_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD =
{
    "FRST_LVL",
#if RU_INCLUDE_DESC
    "",
    "First Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SCND_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD =
{
    "SCND_LVL",
#if RU_INCLUDE_DESC
    "",
    "Second Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HYST_THRS *****/
const ru_field_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed\n",
#endif
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FIELDS[] =
{
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN *****/
const ru_reg_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG =
{
    "CONGESTION_TOTAL_EGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "TOTAL_EGRESS_CONGESTION_THRESHOLD Register",
    "Egress congestion states (Total Count)\n",
#endif
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG_OFFSET },
    0,
    0,
    1291,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_GLBL_CONGSTN, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_GLBL_CONGSTN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FRST_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD =
{
    "FRST_LVL",
#if RU_INCLUDE_DESC
    "",
    "First Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SCND_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD =
{
    "SCND_LVL",
#if RU_INCLUDE_DESC
    "",
    "Second Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HYST_THRS *****/
const ru_field_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed\n",
#endif
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_GLBL_CONGSTN_FIELDS[] =
{
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_GLBL_CONGSTN *****/
const ru_reg_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_REG =
{
    "CONGESTION_GLBL_CONGSTN",
#if RU_INCLUDE_DESC
    "GLOBAL_CONGESTION_THRESHOLD Register",
    "Congestion levels of FLL state. Once no mode BDs are availabe congestion indication will be risen on all PDs.\n",
#endif
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_REG_OFFSET },
    0,
    0,
    1292,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_TOTAL_EGRS_CONGSTN_2ND
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THRD_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_THRD_LVL_FIELD =
{
    "THRD_LVL",
#if RU_INCLUDE_DESC
    "",
    "Third Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_THRD_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_THRD_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_THRD_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HYST_THRS *****/
const ru_field_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed\n",
#endif
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_HYST_THRS_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_HYST_THRS_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_HYST_THRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_FIELDS[] =
{
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_THRD_LVL_FIELD,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_HYST_THRS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND *****/
const ru_reg_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_REG =
{
    "CONGESTION_TOTAL_EGRS_CONGSTN_2ND",
#if RU_INCLUDE_DESC
    "TOTAL_EGRESS_CONGESTION_THRESHOLD_2ND Register",
    "Egress congestion states (Total Count) 2nd register\n",
#endif
    { DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_REG_OFFSET },
    0,
    0,
    1293,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_GLBL_CONGSTN_2ND
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THRD_LVL *****/
const ru_field_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_THRD_LVL_FIELD =
{
    "THRD_LVL",
#if RU_INCLUDE_DESC
    "",
    "Third Level congestion threshold.\n",
#endif
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_THRD_LVL_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_THRD_LVL_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_THRD_LVL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HYST_THRS *****/
const ru_field_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed\n",
#endif
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_HYST_THRS_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_HYST_THRS_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_HYST_THRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_FIELDS[] =
{
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_THRD_LVL_FIELD,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_HYST_THRS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND *****/
const ru_reg_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_REG =
{
    "CONGESTION_GLBL_CONGSTN_2ND",
#if RU_INCLUDE_DESC
    "GLOBAL_CONGESTION_THRESHOLD_2ND Register",
    "Congestion levels of FLL state. Once no mode BDs are available congestion indication will be risen on all PDs. 2nd register\n\n",
#endif
    { DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_REG_OFFSET },
    0,
    0,
    1294,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_CONGESTN_THRSHLD_ACT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THRSHLD_1_ACT *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_1_ACT_FIELD =
{
    "THRSHLD_1_ACT",
#if RU_INCLUDE_DESC
    "",
    "How to treat congestion when we cross 1st threshold per VIQ\n0 - Send to FW (inside PD)\n1 - Send FC message\n",
#endif
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_1_ACT_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_1_ACT_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_1_ACT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THRSHLD_2_ACT *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_2_ACT_FIELD =
{
    "THRSHLD_2_ACT",
#if RU_INCLUDE_DESC
    "",
    "How to treat congestion when we cross 2nd threshold per VIQ\n0 - Send to FW (inside PD)\n1 - Send FC message\n",
#endif
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_2_ACT_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_2_ACT_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_2_ACT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THRSHLD_3_ACT *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_3_ACT_FIELD =
{
    "THRSHLD_3_ACT",
#if RU_INCLUDE_DESC
    "",
    "How to treat congestion when we cross 3rd threshold per VIQ\n0 - Send to FW (inside PD)\n1 - Send FC message\n",
#endif
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_3_ACT_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_3_ACT_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_3_ACT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_FIELDS[] =
{
    &DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_1_ACT_FIELD,
    &DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_2_ACT_FIELD,
    &DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_THRSHLD_3_ACT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT *****/
const ru_reg_rec DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_REG =
{
    "CONGESTION_CONGESTN_THRSHLD_ACT",
#if RU_INCLUDE_DESC
    "CONGESTION_THRESHOLD_ACTION Register",
    "How to treat congestion in case we crossed a threshold\n0 - Send to FW (inside PD)\n1 - Send FC message\n",
#endif
    { DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_REG_OFFSET },
    0,
    0,
    1295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_CONGSTN_STATUS, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_CONGSTN_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GLBL_CONGSTN *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD =
{
    "GLBL_CONGSTN",
#if RU_INCLUDE_DESC
    "",
    "Global congestion levels (according to FLL buffer availability)\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GLBL_EGRS_CONGSTN *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD =
{
    "GLBL_EGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "",
    "Global Egress congestion levels\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_CONGSTN *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD =
{
    "SBPM_CONGSTN",
#if RU_INCLUDE_DESC
    "",
    "SBPM congestion levels according to SPBM messages\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GLBL_CONGSTN_VH *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_VH_FIELD =
{
    "GLBL_CONGSTN_VH",
#if RU_INCLUDE_DESC
    "",
    "Global Ingress congestion Very High state\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_VH_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_VH_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_VH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GLBL_EGRS_CONGSTN_VH *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_VH_FIELD =
{
    "GLBL_EGRS_CONGSTN_VH",
#if RU_INCLUDE_DESC
    "",
    "Global Egress congestion Very High state\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_VH_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_VH_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_VH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GLBL_CONGSTN_STCKY *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_STCKY_FIELD =
{
    "GLBL_CONGSTN_STCKY",
#if RU_INCLUDE_DESC
    "",
    "Global congestion levels (according to FLL buffer availability)\nSticky Value\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_STCKY_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_STCKY_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_STCKY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GLBL_EGRS_CONGSTN_STCKY *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_STCKY_FIELD =
{
    "GLBL_EGRS_CONGSTN_STCKY",
#if RU_INCLUDE_DESC
    "",
    "Global Egress congestion levels\nSticky value\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_STCKY_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_STCKY_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_STCKY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_CONGSTN_STCKY *****/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_STCKY_FIELD =
{
    "SBPM_CONGSTN_STCKY",
#if RU_INCLUDE_DESC
    "",
    "SBPM congestion levels according to SPBM messages\nSticky value\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_STCKY_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_STCKY_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_STCKY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_CONGSTN_STATUS_FIELDS[] =
{
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_VH_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_VH_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_STCKY_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_STCKY_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_STCKY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_CONGSTN_STATUS *****/
const ru_reg_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_REG =
{
    "CONGESTION_CONGSTN_STATUS",
#if RU_INCLUDE_DESC
    "CONGESTION_STATUS Register",
    "This register reflects the current congestion levels in the dispatcher.\n",
#endif
    { DSPTCHR_CONGESTION_CONGSTN_STATUS_REG_OFFSET },
    0,
    0,
    1296,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_PER_Q_INGRS_CONGSTN_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONGSTN_STATE *****/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "",
    "1 - Passed Threshold\n0 - Did not pass threshold\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW *****/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG =
{
    "CONGESTION_PER_Q_INGRS_CONGSTN_LOW",
#if RU_INCLUDE_DESC
    "PER_Q_LOW_INGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG_OFFSET },
    0,
    0,
    1297,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONGSTN_STATE *****/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "",
    "1 - Passed Threshold\n0 - Did not pass threshold\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH *****/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG =
{
    "CONGESTION_PER_Q_INGRS_CONGSTN_HIGH",
#if RU_INCLUDE_DESC
    "PER_Q_HIGH_INGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG_OFFSET },
    0,
    0,
    1298,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONGSTN_STATE *****/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "",
    "1 - Passed Threshold\n0 - Did not pass threshold\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH *****/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_REG =
{
    "CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH",
#if RU_INCLUDE_DESC
    "PER_Q_VERY_HIGH_INGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_REG_OFFSET },
    0,
    0,
    1299,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_PER_Q_INGRS_CONGSTN_FC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONGSTN_STATE *****/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "",
    "1 - Passed Threshold\n0 - Did not pass threshold\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_CONGSTN_STATE_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_CONGSTN_STATE_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_CONGSTN_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_CONGSTN_STATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC *****/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_REG =
{
    "CONGESTION_PER_Q_INGRS_CONGSTN_FC",
#if RU_INCLUDE_DESC
    "PER_Q_FC_INGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_REG_OFFSET },
    0,
    0,
    1300,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_PER_Q_EGRS_CONGSTN_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONGSTN_STATE *****/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "",
    "1 - Passed Threshold\n0 - Did not pass threshold\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW *****/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG =
{
    "CONGESTION_PER_Q_EGRS_CONGSTN_LOW",
#if RU_INCLUDE_DESC
    "PER_Q_LOW_EGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG_OFFSET },
    0,
    0,
    1301,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONGSTN_STATE *****/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "",
    "1 - Passed Threshold\n0 - Did not pass threshold\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH *****/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG =
{
    "CONGESTION_PER_Q_EGRS_CONGSTN_HIGH",
#if RU_INCLUDE_DESC
    "PER_Q_HIGH_EGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG_OFFSET },
    0,
    0,
    1302,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONGSTN_STATE *****/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "",
    "1 - Passed Threshold\n0 - Did not pass threshold\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_CONGSTN_STATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH *****/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_REG =
{
    "CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH",
#if RU_INCLUDE_DESC
    "PER_Q_VERY_HIGH_EGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_REG_OFFSET },
    0,
    0,
    1303,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_CONGESTION_PER_Q_EGRS_CONGSTN_FC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONGSTN_STATE *****/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "",
    "1 - Passed Threshold\n0 - Did not pass threshold\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_CONGSTN_STATE_FIELD_MASK },
    0,
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_CONGSTN_STATE_FIELD_WIDTH },
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_CONGSTN_STATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_CONGSTN_STATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC *****/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_REG =
{
    "CONGESTION_PER_Q_EGRS_CONGSTN_FC",
#if RU_INCLUDE_DESC
    "PER_Q_FC_EGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage\n",
#endif
    { DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_REG_OFFSET },
    0,
    0,
    1304,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_INGRS_QUEUES_Q_INGRS_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMN_CNT *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD =
{
    "CMN_CNT",
#if RU_INCLUDE_DESC
    "",
    "Common number of buffers allocated to this Q.\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE *****/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG =
{
    "INGRS_QUEUES_Q_INGRS_SIZE",
#if RU_INCLUDE_DESC
    "QUEUE_INGRS_SIZE 0..8 Register",
    "Q Ingress size\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG_OFFSET },
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG_RAM_CNT,
    4,
    1305,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_INGRS_QUEUES_Q_INGRS_LIMITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMN_MAX *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD =
{
    "CMN_MAX",
#if RU_INCLUDE_DESC
    "",
    "Maximum number of buffers allowed to be allocated to the specific VIQ from the common Pool\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GURNTD_MAX *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD =
{
    "GURNTD_MAX",
#if RU_INCLUDE_DESC
    "",
    "Maximum number of buffers allowed to be allocated to the specific VIQ from the guaranteed Pool\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDIT_CNT *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD =
{
    "CREDIT_CNT",
#if RU_INCLUDE_DESC
    "",
    "Holds the value of the the accumulated credits. this is sent to the BBH/RNR.\nBBH disregards the value. RNR uses it to to calculate the amount of available credits.\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS *****/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG =
{
    "INGRS_QUEUES_Q_INGRS_LIMITS",
#if RU_INCLUDE_DESC
    "QUEUE_INGRS_LIMITS 0..8 Register",
    "Q Ingress Limits\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG_OFFSET },
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG_RAM_CNT,
    4,
    1306,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_INGRS_QUEUES_Q_ING_WRR_WGHT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_0 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_0_FIELD =
{
    "WRR_Q_0",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 0\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_0_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_0_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_1 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_1_FIELD =
{
    "WRR_Q_1",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 1\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_1_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_1_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_2 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_2_FIELD =
{
    "WRR_Q_2",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 2\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_2_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_2_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_3 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_3_FIELD =
{
    "WRR_Q_3",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 3\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_3_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_3_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_4 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_4_FIELD =
{
    "WRR_Q_4",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 4\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_4_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_4_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_5 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_5_FIELD =
{
    "WRR_Q_5",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 5\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_5_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_5_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_6 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_6_FIELD =
{
    "WRR_Q_6",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 6\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_6_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_6_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_7 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_7_FIELD =
{
    "WRR_Q_7",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 7\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_7_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_7_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_0_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_1_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_2_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_3_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_4_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_5_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_6_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_WRR_Q_7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1 *****/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_REG =
{
    "INGRS_QUEUES_Q_ING_WRR_WGHT1",
#if RU_INCLUDE_DESC
    "QUEUE_ING_WRR_WEIGHT_1 Register",
    "Queue WRR weight for ingress queue head arbitration\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_REG_OFFSET },
    0,
    0,
    1307,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_INGRS_QUEUES_Q_ING_WRR_WGHT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_8 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_WRR_Q_8_FIELD =
{
    "WRR_Q_8",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 8\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_WRR_Q_8_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_WRR_Q_8_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_WRR_Q_8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_WRR_Q_8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2 *****/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_REG =
{
    "INGRS_QUEUES_Q_ING_WRR_WGHT2",
#if RU_INCLUDE_DESC
    "QUEUE_ING_WRR_WEIGHT_2 Register",
    "Queue WRR weight for ingress queue head arbitration\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_REG_OFFSET },
    0,
    0,
    1308,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_INGRS_QUEUES_Q_ORD_WRR_WGHT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_0 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_0_FIELD =
{
    "WRR_Q_0",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 0\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_0_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_0_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_1 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_1_FIELD =
{
    "WRR_Q_1",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 1\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_1_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_1_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_2 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_2_FIELD =
{
    "WRR_Q_2",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 2\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_2_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_2_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_3 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_3_FIELD =
{
    "WRR_Q_3",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 3\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_3_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_3_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_4 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_4_FIELD =
{
    "WRR_Q_4",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 4\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_4_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_4_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_5 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_5_FIELD =
{
    "WRR_Q_5",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 5\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_5_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_5_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_6 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_6_FIELD =
{
    "WRR_Q_6",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 6\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_6_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_6_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_7 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_7_FIELD =
{
    "WRR_Q_7",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 7\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_7_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_7_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_0_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_1_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_2_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_3_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_4_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_5_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_6_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_WRR_Q_7_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1 *****/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_REG =
{
    "INGRS_QUEUES_Q_ORD_WRR_WGHT1",
#if RU_INCLUDE_DESC
    "QUEUE_ORD_WRR_WEIGHT_1 Register",
    "Queue WRR weight for Reorder queue veq arbitration\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_REG_OFFSET },
    0,
    0,
    1309,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_INGRS_QUEUES_Q_ORD_WRR_WGHT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_Q_8 *****/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_WRR_Q_8_FIELD =
{
    "WRR_Q_8",
#if RU_INCLUDE_DESC
    "",
    "WRR Weight for Queue 8\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_WRR_Q_8_FIELD_MASK },
    0,
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_WRR_Q_8_FIELD_WIDTH },
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_WRR_Q_8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_WRR_Q_8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2 *****/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_REG =
{
    "INGRS_QUEUES_Q_ORD_WRR_WGHT2",
#if RU_INCLUDE_DESC
    "QUEUE_ORD_WRR_WEIGHT_2 Register",
    "Queue WRR weight for Reorder queue veq arbitration\n",
#endif
    { DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_REG_OFFSET },
    0,
    0,
    1310,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QUEUE_MAPPING_CRDT_CFG, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QUEUE_MAPPING_CRDT_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BB_ID *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD =
{
    "BB_ID",
#if RU_INCLUDE_DESC
    "",
    "BroadBud ID: To which BroadBud agent (RNR/BBH) is the current Q associated with\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TRGT_ADD *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD =
{
    "TRGT_ADD",
#if RU_INCLUDE_DESC
    "",
    "Target address within the BB agent where the credit message should be written to.\n\nIn case of BBH:\n31:23 - VIQ (one hot)\n22:18 - 5b0\n17:16 - 2b10\n\nIn case of RNR:\n27:16 - Ram address\n31:28 - Task number to wakeup\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QUEUE_MAPPING_CRDT_CFG_FIELDS[] =
{
    &DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD,
    &DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QUEUE_MAPPING_CRDT_CFG *****/
const ru_reg_rec DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG =
{
    "QUEUE_MAPPING_CRDT_CFG",
#if RU_INCLUDE_DESC
    "CREDIT_CONFIGURATION 0..8 Register",
    "Configuration for each Q including BB_ID, Target address, valid\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG_OFFSET },
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG_RAM_CNT,
    4,
    1311,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QUEUE_MAPPING_Q_DEST, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QUEUE_MAPPING_Q_DEST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q2 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD =
{
    "Q2",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q3 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD =
{
    "Q3",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q4 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD =
{
    "Q4",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q5 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD =
{
    "Q5",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q6 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD =
{
    "Q6",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q7 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD =
{
    "Q7",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q8 *****/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD =
{
    "Q8",
#if RU_INCLUDE_DESC
    "",
    "0- Dispatcher\n1- Reorder\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD_MASK },
    0,
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD_WIDTH },
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QUEUE_MAPPING_Q_DEST_FIELDS[] =
{
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QUEUE_MAPPING_Q_DEST *****/
const ru_reg_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_REG =
{
    "QUEUE_MAPPING_Q_DEST",
#if RU_INCLUDE_DESC
    "Q_DESTINATION Register",
    "What is the destination of each VIQ. to Dispatcher and from there to Processing RNR or Reorder and from there to the QM\n",
#endif
    { DSPTCHR_QUEUE_MAPPING_Q_DEST_REG_OFFSET },
    0,
    0,
    1312,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_CMN_POOL_LMT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_CMN_POOL_LMT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_LMT *****/
const ru_field_rec DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD =
{
    "POOL_LMT",
#if RU_INCLUDE_DESC
    "",
    "MAX number of buffers allowed in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_CMN_POOL_LMT_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_CMN_POOL_LMT *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_CMN_POOL_LMT_REG =
{
    "POOL_SIZES_CMN_POOL_LMT",
#if RU_INCLUDE_DESC
    "COMMON_POOL_LIMIT Register",
    "common pool max size\n",
#endif
    { DSPTCHR_POOL_SIZES_CMN_POOL_LMT_REG_OFFSET },
    0,
    0,
    1313,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_CMN_POOL_SIZE, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_CMN_POOL_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_SIZE *****/
const ru_field_rec DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Number of buffers currently in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_CMN_POOL_SIZE *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_REG =
{
    "POOL_SIZES_CMN_POOL_SIZE",
#if RU_INCLUDE_DESC
    "COMMON_POOL_SIZE Register",
    "common pool size\n",
#endif
    { DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_REG_OFFSET },
    0,
    0,
    1314,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_GRNTED_POOL_LMT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_LMT *****/
const ru_field_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD =
{
    "POOL_LMT",
#if RU_INCLUDE_DESC
    "",
    "MAX number of buffers allowed in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_REG =
{
    "POOL_SIZES_GRNTED_POOL_LMT",
#if RU_INCLUDE_DESC
    "GUARANTEED_POOL_LIMIT Register",
    "Guaranteed pool max size\n",
#endif
    { DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_REG_OFFSET },
    0,
    0,
    1315,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_GRNTED_POOL_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_SIZE *****/
const ru_field_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Number of buffers currently in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_REG =
{
    "POOL_SIZES_GRNTED_POOL_SIZE",
#if RU_INCLUDE_DESC
    "GUARANTEED_POOL_SIZE Register",
    "Guaranteed pool size\n",
#endif
    { DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_REG_OFFSET },
    0,
    0,
    1316,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_MULTI_CST_POOL_LMT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_LMT *****/
const ru_field_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD =
{
    "POOL_LMT",
#if RU_INCLUDE_DESC
    "",
    "MAX number of buffers allowed in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_REG =
{
    "POOL_SIZES_MULTI_CST_POOL_LMT",
#if RU_INCLUDE_DESC
    "MULTI_CAST_POOL_LIMIT Register",
    "Multi Cast pool max size\n",
#endif
    { DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_REG_OFFSET },
    0,
    0,
    1317,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_MULTI_CST_POOL_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_SIZE *****/
const ru_field_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Number of buffers currently in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_REG =
{
    "POOL_SIZES_MULTI_CST_POOL_SIZE",
#if RU_INCLUDE_DESC
    "MULTI_CAST_POOL_SIZE Register",
    "Multi Cast pool size\n",
#endif
    { DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_REG_OFFSET },
    0,
    0,
    1318,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_RNR_POOL_LMT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_RNR_POOL_LMT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_LMT *****/
const ru_field_rec DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD =
{
    "POOL_LMT",
#if RU_INCLUDE_DESC
    "",
    "MAX number of buffers allowed in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_RNR_POOL_LMT_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_RNR_POOL_LMT *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_RNR_POOL_LMT_REG =
{
    "POOL_SIZES_RNR_POOL_LMT",
#if RU_INCLUDE_DESC
    "RNR_POOL_LIMIT Register",
    "This counter counts the amount of buffers taken by runner for MultiCast purposes (or any other the requires adding new PDs to a Virtual Egress Queue - VEQ\n",
#endif
    { DSPTCHR_POOL_SIZES_RNR_POOL_LMT_REG_OFFSET },
    0,
    0,
    1319,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_RNR_POOL_SIZE, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_RNR_POOL_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_SIZE *****/
const ru_field_rec DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Number of buffers currently in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_RNR_POOL_SIZE *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_REG =
{
    "POOL_SIZES_RNR_POOL_SIZE",
#if RU_INCLUDE_DESC
    "RNR_POOL_SIZE Register",
    "This counter counts the amount of buffers taken by runner for MultiCast purposes (or any other the requires adding new PDs to a Virtual Egress Qeueu - VEQ)\n",
#endif
    { DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_REG_OFFSET },
    0,
    0,
    1320,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_POOL_SIZES_PRCSSING_POOL_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_SIZE *****/
const ru_field_rec DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Number of buffers currently in the pool\n",
#endif
    { DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD_MASK },
    0,
    { DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD_WIDTH },
    { DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE *****/
const ru_reg_rec DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_REG =
{
    "POOL_SIZES_PRCSSING_POOL_SIZE",
#if RU_INCLUDE_DESC
    "PROCESSING_POOL_SIZE Register",
    "This counter counts how many buffers are currenly being handled by all RNRs\n",
#endif
    { DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_REG_OFFSET },
    0,
    0,
    1321,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_MASK_MSK_Q, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_MASK_MSK_Q
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec DSPTCHR_MASK_MSK_Q_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "MASK\n",
#endif
    { DSPTCHR_MASK_MSK_Q_MASK_FIELD_MASK },
    0,
    { DSPTCHR_MASK_MSK_Q_MASK_FIELD_WIDTH },
    { DSPTCHR_MASK_MSK_Q_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_MASK_MSK_Q_FIELDS[] =
{
    &DSPTCHR_MASK_MSK_Q_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_MASK_MSK_Q *****/
const ru_reg_rec DSPTCHR_MASK_MSK_Q_REG =
{
    "MASK_MSK_Q",
#if RU_INCLUDE_DESC
    "QUEUE_MASK 0..7 Register",
    "Queue Mask: Per RNR group holds a vector of which tasks are related to the group\n",
#endif
    { DSPTCHR_MASK_MSK_Q_REG_OFFSET },
    DSPTCHR_MASK_MSK_Q_REG_RAM_CNT,
    4,
    1322,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_MASK_MSK_Q_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_MASK_DLY_Q, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_MASK_DLY_Q
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec DSPTCHR_MASK_DLY_Q_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "MASK\n",
#endif
    { DSPTCHR_MASK_DLY_Q_MASK_FIELD_MASK },
    0,
    { DSPTCHR_MASK_DLY_Q_MASK_FIELD_WIDTH },
    { DSPTCHR_MASK_DLY_Q_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_MASK_DLY_Q_FIELDS[] =
{
    &DSPTCHR_MASK_DLY_Q_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_MASK_DLY_Q *****/
const ru_reg_rec DSPTCHR_MASK_DLY_Q_REG =
{
    "MASK_DLY_Q",
#if RU_INCLUDE_DESC
    "DELAY_Q Register",
    "Describes which VEQ are part of the Delay Q group.\n",
#endif
    { DSPTCHR_MASK_DLY_Q_REG_OFFSET },
    0,
    0,
    1323,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_MASK_DLY_Q_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_MASK_NON_DLY_Q, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_MASK_NON_DLY_Q
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "MASK\n",
#endif
    { DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD_MASK },
    0,
    { DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD_WIDTH },
    { DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_MASK_NON_DLY_Q_FIELDS[] =
{
    &DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_MASK_NON_DLY_Q *****/
const ru_reg_rec DSPTCHR_MASK_NON_DLY_Q_REG =
{
    "MASK_NON_DLY_Q",
#if RU_INCLUDE_DESC
    "NON_DELAY_Q Register",
    "Describes which VEQ are part of the Non-Delay Q group.\n",
#endif
    { DSPTCHR_MASK_NON_DLY_Q_REG_OFFSET },
    0,
    0,
    1324,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_MASK_NON_DLY_Q_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_EGRS_QUEUES_EGRS_DLY_QM_CRDT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DLY_CRDT *****/
const ru_field_rec DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD =
{
    "DLY_CRDT",
#if RU_INCLUDE_DESC
    "",
    "The amount of free credits the re-order can utilize to send PDs to the QM\n",
#endif
    { DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD_MASK },
    0,
    { DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD_WIDTH },
    { DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_FIELDS[] =
{
    &DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT *****/
const ru_reg_rec DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG =
{
    "EGRS_QUEUES_EGRS_DLY_QM_CRDT",
#if RU_INCLUDE_DESC
    "EGRESS_QM_DELAY_CREDIT Register",
    "These registers hold the available credit for the Re-Order to sent PDs to the QM via Delay Q.\n\n\n",
#endif
    { DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG_OFFSET },
    0,
    0,
    1325,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NON_DLY_CRDT *****/
const ru_field_rec DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD =
{
    "NON_DLY_CRDT",
#if RU_INCLUDE_DESC
    "",
    "The amount of free credits the re-order can utilize to send PDs to the QM\n",
#endif
    { DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD_MASK },
    0,
    { DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD_WIDTH },
    { DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_FIELDS[] =
{
    &DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT *****/
const ru_reg_rec DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG =
{
    "EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT",
#if RU_INCLUDE_DESC
    "EGRESS_QM_NON_DELAY_CREDIT Register",
    "These registers hold the available credit for the Re-Order to sent PDs to the QM via Non-Delay Q.\n\n\n",
#endif
    { DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG_OFFSET },
    0,
    0,
    1326,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOTAL_EGRS_SIZE *****/
const ru_field_rec DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD =
{
    "TOTAL_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Accumulates all buffers that are marked as egress (after dispatch and before sending to QM)\n",
#endif
    { DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD_MASK },
    0,
    { DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD_WIDTH },
    { DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_FIELDS[] =
{
    &DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE *****/
const ru_reg_rec DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG =
{
    "EGRS_QUEUES_TOTAL_Q_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "TOTAL_EGRESS_SIZE Register",
    "Size of all egress queues. affected from PDs sent to dispatch and from multicast connect\n\n",
#endif
    { DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG_OFFSET },
    0,
    0,
    1327,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_EGRS_QUEUES_PER_Q_EGRS_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_EGRS_SIZE *****/
const ru_field_rec DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD =
{
    "Q_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Accumulates all buffers that are marked as egress (after dispatch and before sending to QM)\n",
#endif
    { DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD_MASK },
    0,
    { DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD_WIDTH },
    { DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_FIELDS[] =
{
    &DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE *****/
const ru_reg_rec DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG =
{
    "EGRS_QUEUES_PER_Q_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "Q_EGRESS_SIZE 0..8 Register",
    "Size of all egress queues. affected from PDs sent to dispatch and from multicast connect\n\n",
#endif
    { DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG_OFFSET },
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG_RAM_CNT,
    4,
    1328,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_WAKEUP_CONTROL_WKUP_REQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q2 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD =
{
    "Q2",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q3 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD =
{
    "Q3",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q4 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD =
{
    "Q4",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q5 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD =
{
    "Q5",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q6 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD =
{
    "Q6",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q7 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD =
{
    "Q7",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q8 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD =
{
    "Q8",
#if RU_INCLUDE_DESC
    "",
    "wakeup request pending\nTRIGGERS_HW\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_FIELDS[] =
{
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ *****/
const ru_reg_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG =
{
    "WAKEUP_CONTROL_WKUP_REQ",
#if RU_INCLUDE_DESC
    "WAKEUP_REQUEST Register",
    "Bit per queue, wakeup request from RNR to a specific Q. Once a wakeup request message is sent to dsptchr it will be latched until the amount of credits pass a threshold\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG_OFFSET },
    0,
    0,
    1329,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_WAKEUP_CONTROL_WKUP_THRSHLD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKUP_THRSHLD *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD =
{
    "WKUP_THRSHLD",
#if RU_INCLUDE_DESC
    "",
    "Wakeup threshold. Once number of Guaranteed buffer count crosses the threshold and there is a pending wakeup request, the dispatcher will issue a wakeup message to the appropriate runner according to a predefind address configuration\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_FIELDS[] =
{
    &DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD *****/
const ru_reg_rec DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG =
{
    "WAKEUP_CONTROL_WKUP_THRSHLD",
#if RU_INCLUDE_DESC
    "WAKEUP_THRESHOLD Register",
    "Wakeup Thresholds in which to indicate RNR\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG_OFFSET },
    0,
    0,
    1330,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_WAKEUP_CONTROL_WKUP_CRDT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Queue 0\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q0_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q0_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q0_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Queue 1\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q1_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q1_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q2 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q2_FIELD =
{
    "Q2",
#if RU_INCLUDE_DESC
    "",
    "Queue 2\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q2_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q2_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q2_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q3 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q3_FIELD =
{
    "Q3",
#if RU_INCLUDE_DESC
    "",
    "Queue 3\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q3_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q3_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q3_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q4 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q4_FIELD =
{
    "Q4",
#if RU_INCLUDE_DESC
    "",
    "Queue 4\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q4_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q4_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q4_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q5 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q5_FIELD =
{
    "Q5",
#if RU_INCLUDE_DESC
    "",
    "Queue 5\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q5_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q5_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q5_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q6 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q6_FIELD =
{
    "Q6",
#if RU_INCLUDE_DESC
    "",
    "Queue 6\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q6_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q6_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q6_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q7 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q7_FIELD =
{
    "Q7",
#if RU_INCLUDE_DESC
    "",
    "Queue 7\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q7_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q7_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q7_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q8 *****/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q8_FIELD =
{
    "Q8",
#if RU_INCLUDE_DESC
    "",
    "Queue 8\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q8_FIELD_MASK },
    0,
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q8_FIELD_WIDTH },
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q8_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_FIELDS[] =
{
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q0_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q1_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q2_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q3_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q4_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q5_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q6_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q7_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_Q8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT *****/
const ru_reg_rec DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_REG =
{
    "WAKEUP_CONTROL_WKUP_CRDT",
#if RU_INCLUDE_DESC
    "WAKEUP_CRDT Register",
    "Wakeup send credit upon RNR request\n0 - Send credit only upon RNR Wakeup request\n1 - Send credit when possible\n\n",
#endif
    { DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_REG_OFFSET },
    0,
    0,
    1331,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DISPTCH_SCHEDULING_DWRR_INFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_CRDT *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD =
{
    "Q_CRDT",
#if RU_INCLUDE_DESC
    "",
    "availabe credits in bytes. Q will not be permitted to dispatch PDs if credit levels are below zero\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NGTV *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD =
{
    "NGTV",
#if RU_INCLUDE_DESC
    "",
    "Bit will be enabled if credit levels are below zero. 2 compliment\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QUNTUM *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD =
{
    "QUNTUM",
#if RU_INCLUDE_DESC
    "",
    "Quantum size. Should be configured according to Q rate. in Bytes\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_FIELDS[] =
{
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO *****/
const ru_reg_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG =
{
    "DISPTCH_SCHEDULING_DWRR_INFO",
#if RU_INCLUDE_DESC
    "SCHEDULING_Q_INFO 0..8 Register",
    "DWRR info per Q. including amount of credits per Q. If Q has below zero credits and Quantum size\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG_OFFSET },
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG_RAM_CNT,
    4,
    1332,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DISPTCH_SCHEDULING_VLD_CRDT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q0 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q1 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits.\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q2 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD =
{
    "Q2",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q3 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD =
{
    "Q3",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q4 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD =
{
    "Q4",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q5 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD =
{
    "Q5",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q6 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD =
{
    "Q6",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q7 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD =
{
    "Q7",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q8 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD =
{
    "Q8",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q9 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD =
{
    "Q9",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q10 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD =
{
    "Q10",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q11 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD =
{
    "Q11",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q12 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD =
{
    "Q12",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q13 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD =
{
    "Q13",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q14 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD =
{
    "Q14",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q15 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD =
{
    "Q15",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q16 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD =
{
    "Q16",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q17 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD =
{
    "Q17",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q18 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD =
{
    "Q18",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q19 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD =
{
    "Q19",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q20 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD =
{
    "Q20",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q21 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD =
{
    "Q21",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q22 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD =
{
    "Q22",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q23 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD =
{
    "Q23",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q24 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD =
{
    "Q24",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q25 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD =
{
    "Q25",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q26 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD =
{
    "Q26",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q27 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD =
{
    "Q27",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q28 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD =
{
    "Q28",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q29 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD =
{
    "Q29",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q30 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD =
{
    "Q30",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: Q31 *****/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD =
{
    "Q31",
#if RU_INCLUDE_DESC
    "",
    "Valid Credits\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD_MASK },
    0,
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD_WIDTH },
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_FIELDS[] =
{
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT *****/
const ru_reg_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG =
{
    "DISPTCH_SCHEDULING_VLD_CRDT",
#if RU_INCLUDE_DESC
    "VALID_QUEUES Register",
    "Queues with credits above zero. This will allow for the Q to participate in the scheduling round\n",
#endif
    { DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG_OFFSET },
    0,
    0,
    1333,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, TYPE: Type_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLL_RETURN_BUF *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD =
{
    "FLL_RETURN_BUF",
#if RU_INCLUDE_DESC
    "",
    "Buffer returned to Fll\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLL_CNT_DRP *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD =
{
    "FLL_CNT_DRP",
#if RU_INCLUDE_DESC
    "",
    "Drop PD counted\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UNKNWN_MSG *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD =
{
    "UNKNWN_MSG",
#if RU_INCLUDE_DESC
    "",
    "Unknown message entered the dispatcher\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLL_OVERFLOW *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD =
{
    "FLL_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "Number of buffers returned to FLL exceeds the pre-defined allocated buffer amount (due to linked list bug)\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLL_NEG *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD =
{
    "FLL_NEG",
#if RU_INCLUDE_DESC
    "",
    "Number of buffers returned to FLL decreased under zero and reached a negative amount (due to linked list bug)\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REORDR_EMPTY_VIQ *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REORDR_EMPTY_VIQ_FIELD =
{
    "REORDR_EMPTY_VIQ",
#if RU_INCLUDE_DESC
    "",
    "Returning PD to an empty VIQ\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REORDR_EMPTY_VIQ_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REORDR_EMPTY_VIQ_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REORDR_EMPTY_VIQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REORDR_EMPTY_VIQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR *****/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG =
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG_OFFSET },
    0,
    0,
    1334,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM, TYPE: Type_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ISM *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "",
    "Status Masked of corresponding interrupt source in the ISR\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM *****/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG =
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG_OFFSET },
    0,
    0,
    1335,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER, TYPE: Type_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IEM *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask controls the corresponding interrupt source in the IER\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER *****/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG =
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG_OFFSET },
    0,
    0,
    1336,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR, TYPE: Type_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask tests the corresponding interrupt source in the ISR\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR *****/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG =
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG_OFFSET },
    0,
    0,
    1337,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, TYPE: Type_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST0_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD =
{
    "QDEST0_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 0\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST1_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD =
{
    "QDEST1_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 1\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST2_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD =
{
    "QDEST2_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 2\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST3_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD =
{
    "QDEST3_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 3\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST4_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD =
{
    "QDEST4_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 4\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST5_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD =
{
    "QDEST5_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 5\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST6_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD =
{
    "QDEST6_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 6\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST7_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD =
{
    "QDEST7_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 7\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST8_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD =
{
    "QDEST8_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 8\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST9_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD =
{
    "QDEST9_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 9\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST10_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD =
{
    "QDEST10_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 10\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST11_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD =
{
    "QDEST11_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 11\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST12_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD =
{
    "QDEST12_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 12\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST13_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD =
{
    "QDEST13_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 13\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST14_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD =
{
    "QDEST14_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 14\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST15_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD =
{
    "QDEST15_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 15\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST16_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD =
{
    "QDEST16_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 16\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST17_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD =
{
    "QDEST17_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 17\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST18_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD =
{
    "QDEST18_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 18\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST19_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD =
{
    "QDEST19_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 19\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST20_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD =
{
    "QDEST20_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 20\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST21_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD =
{
    "QDEST21_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 21\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST22_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD =
{
    "QDEST22_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 22\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST23_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD =
{
    "QDEST23_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 23\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST24_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD =
{
    "QDEST24_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 24\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST25_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD =
{
    "QDEST25_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 25\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST26_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD =
{
    "QDEST26_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 26\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST27_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD =
{
    "QDEST27_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 27\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST28_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD =
{
    "QDEST28_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 28\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST29_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD =
{
    "QDEST29_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 29\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST30_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD =
{
    "QDEST30_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 30\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QDEST31_INT *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD =
{
    "QDEST31_INT",
#if RU_INCLUDE_DESC
    "",
    "New Entry added to Destination queue 31\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR *****/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG =
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG_OFFSET },
    0,
    0,
    1338,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM, TYPE: Type_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ISM *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "",
    "Status Masked of corresponding interrupt source in the ISR\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM *****/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG =
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG_OFFSET },
    0,
    0,
    1339,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER, TYPE: Type_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IEM *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask controls the corresponding interrupt source in the IER\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER *****/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG =
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG_OFFSET },
    0,
    0,
    1340,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR, TYPE: Type_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask tests the corresponding interrupt source in the ISR\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD_MASK },
    0,
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD_WIDTH },
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR *****/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG =
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR\n",
#endif
    { DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG_OFFSET },
    0,
    0,
    1341,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_BYPSS_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_BYP *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD =
{
    "EN_BYP",
#if RU_INCLUDE_DESC
    "",
    "Enable bypass mode\n",
#endif
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBID_NON_DLY *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD =
{
    "BBID_NON_DLY",
#if RU_INCLUDE_DESC
    "",
    "What BBID to use for NON_DELAY Q when in Bypass mode\n",
#endif
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBID_DLY *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD =
{
    "BBID_DLY",
#if RU_INCLUDE_DESC
    "",
    "What BBID to use for DELAY Q when in Bypass mode\n",
#endif
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG =
{
    "DEBUG_DBG_BYPSS_CNTRL",
#if RU_INCLUDE_DESC
    "DEBUG_BYPASS_CONTROL Register",
    "Debug Bypass control\n",
#endif
    { DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG_OFFSET },
    0,
    0,
    1342,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_BUS_CNTRL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_BUS_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_SEL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD =
{
    "DBG_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects which vector to output\n",
#endif
    { DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_BUS_CNTRL_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_BUS_CNTRL *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_BUS_CNTRL_REG =
{
    "DEBUG_DBG_BUS_CNTRL",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_CONTROL Register",
    "Debug bus control which vector to output to the top level\n",
#endif
    { DSPTCHR_DEBUG_DBG_BUS_CNTRL_REG_OFFSET },
    0,
    0,
    1343,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_0, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_0_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_0 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_0_REG =
{
    "DEBUG_DBG_VEC_0",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_0 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_0_REG_OFFSET },
    0,
    0,
    1344,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_1, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_1_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_1 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_1_REG =
{
    "DEBUG_DBG_VEC_1",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_1 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_1_REG_OFFSET },
    0,
    0,
    1345,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_2, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_2_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_2 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_2_REG =
{
    "DEBUG_DBG_VEC_2",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_2 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_2_REG_OFFSET },
    0,
    0,
    1346,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_3, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_3_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_3 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_3_REG =
{
    "DEBUG_DBG_VEC_3",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_3 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_3_REG_OFFSET },
    0,
    0,
    1347,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_4, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_4_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_4 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_4_REG =
{
    "DEBUG_DBG_VEC_4",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_4 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_4_REG_OFFSET },
    0,
    0,
    1348,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_5, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_5
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_5_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_5 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_5_REG =
{
    "DEBUG_DBG_VEC_5",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_5 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_5_REG_OFFSET },
    0,
    0,
    1349,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_6, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_6
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_6_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_6 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_6_REG =
{
    "DEBUG_DBG_VEC_6",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_6 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_6_REG_OFFSET },
    0,
    0,
    1350,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_7, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_7
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_7_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_7 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_7_REG =
{
    "DEBUG_DBG_VEC_7",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_7 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_7_REG_OFFSET },
    0,
    0,
    1351,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_8, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_8
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_8_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_8 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_8_REG =
{
    "DEBUG_DBG_VEC_8",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_8 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_8_REG_OFFSET },
    0,
    0,
    1352,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_8_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_9, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_9
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_9_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_9 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_9_REG =
{
    "DEBUG_DBG_VEC_9",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_9 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_9_REG_OFFSET },
    0,
    0,
    1353,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_9_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_10, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_10
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_10_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_10 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_10_REG =
{
    "DEBUG_DBG_VEC_10",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_10 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_10_REG_OFFSET },
    0,
    0,
    1354,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_10_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_11, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_11
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_11_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_11 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_11_REG =
{
    "DEBUG_DBG_VEC_11",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_11 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_11_REG_OFFSET },
    0,
    0,
    1355,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_11_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_12, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_12
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_12_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_12 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_12_REG =
{
    "DEBUG_DBG_VEC_12",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_12 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_12_REG_OFFSET },
    0,
    0,
    1356,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_12_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_13, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_13
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_13_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_13 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_13_REG =
{
    "DEBUG_DBG_VEC_13",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_13 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_13_REG_OFFSET },
    0,
    0,
    1357,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_13_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_14, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_14
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_14_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_14 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_14_REG =
{
    "DEBUG_DBG_VEC_14",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_14 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_14_REG_OFFSET },
    0,
    0,
    1358,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_14_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_15, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_15
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_15_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_15 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_15_REG =
{
    "DEBUG_DBG_VEC_15",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_15 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_15_REG_OFFSET },
    0,
    0,
    1359,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_15_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_16, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_16
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_16_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_16 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_16_REG =
{
    "DEBUG_DBG_VEC_16",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_16 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_16_REG_OFFSET },
    0,
    0,
    1360,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_16_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_17, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_17
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_17_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_17 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_17_REG =
{
    "DEBUG_DBG_VEC_17",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_17 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_17_REG_OFFSET },
    0,
    0,
    1361,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_17_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_18, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_18
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_18_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_18 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_18_REG =
{
    "DEBUG_DBG_VEC_18",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_18 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_18_REG_OFFSET },
    0,
    0,
    1362,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_18_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_19, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_19
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_19_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_19 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_19_REG =
{
    "DEBUG_DBG_VEC_19",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_19 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_19_REG_OFFSET },
    0,
    0,
    1363,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_19_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_20, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_20
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_20_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_20 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_20_REG =
{
    "DEBUG_DBG_VEC_20",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_20 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_20_REG_OFFSET },
    0,
    0,
    1364,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_20_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_21, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_21
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_21_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_21 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_21_REG =
{
    "DEBUG_DBG_VEC_21",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_21 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_21_REG_OFFSET },
    0,
    0,
    1365,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_21_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_22, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_22
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_22_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_22 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_22_REG =
{
    "DEBUG_DBG_VEC_22",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_22 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_22_REG_OFFSET },
    0,
    0,
    1366,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_22_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_23, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_23
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_23_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_23 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_23_REG =
{
    "DEBUG_DBG_VEC_23",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_23 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_23_REG_OFFSET },
    0,
    0,
    1367,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_23_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_24, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_24
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_24_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_24_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_24_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_24_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_24_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_24_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_24 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_24_REG =
{
    "DEBUG_DBG_VEC_24",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_24 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_24_REG_OFFSET },
    0,
    0,
    1368,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_24_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_DBG_VEC_25, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_DBG_VEC_25
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_25_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_25_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_DBG_VEC_25_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_DBG_VEC_25_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_25_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_25_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_DBG_VEC_25 *****/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_25_REG =
{
    "DEBUG_DBG_VEC_25",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_25 Register",
    "Debug vector mapped to registers\n",
#endif
    { DSPTCHR_DEBUG_DBG_VEC_25_REG_OFFSET },
    0,
    0,
    1369,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_25_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_INTR_VEC_0, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_INTR_VEC_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INTR_VEC_0 *****/
const ru_field_rec DSPTCHR_DEBUG_INTR_VEC_0_INTR_VEC_0_FIELD =
{
    "INTR_VEC_0",
#if RU_INCLUDE_DESC
    "",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\nDATA bits 127:96\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_0_INTR_VEC_0_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_INTR_VEC_0_INTR_VEC_0_FIELD_WIDTH },
    { DSPTCHR_DEBUG_INTR_VEC_0_INTR_VEC_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_INTR_VEC_0_FIELDS[] =
{
    &DSPTCHR_DEBUG_INTR_VEC_0_INTR_VEC_0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_INTR_VEC_0 *****/
const ru_reg_rec DSPTCHR_DEBUG_INTR_VEC_0_REG =
{
    "DEBUG_INTR_VEC_0",
#if RU_INCLUDE_DESC
    "INTR_VEC_0 Register",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_0_REG_OFFSET },
    0,
    0,
    1370,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_INTR_VEC_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_INTR_VEC_1, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_INTR_VEC_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INTR_VEC_1 *****/
const ru_field_rec DSPTCHR_DEBUG_INTR_VEC_1_INTR_VEC_1_FIELD =
{
    "INTR_VEC_1",
#if RU_INCLUDE_DESC
    "",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\nDATA bits 95:64\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_1_INTR_VEC_1_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_INTR_VEC_1_INTR_VEC_1_FIELD_WIDTH },
    { DSPTCHR_DEBUG_INTR_VEC_1_INTR_VEC_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_INTR_VEC_1_FIELDS[] =
{
    &DSPTCHR_DEBUG_INTR_VEC_1_INTR_VEC_1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_INTR_VEC_1 *****/
const ru_reg_rec DSPTCHR_DEBUG_INTR_VEC_1_REG =
{
    "DEBUG_INTR_VEC_1",
#if RU_INCLUDE_DESC
    "INTR_VEC_1 Register",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_1_REG_OFFSET },
    0,
    0,
    1371,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_INTR_VEC_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_INTR_VEC_2, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_INTR_VEC_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INTR_VEC_2 *****/
const ru_field_rec DSPTCHR_DEBUG_INTR_VEC_2_INTR_VEC_2_FIELD =
{
    "INTR_VEC_2",
#if RU_INCLUDE_DESC
    "",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\nDATA bits 63:32\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_2_INTR_VEC_2_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_INTR_VEC_2_INTR_VEC_2_FIELD_WIDTH },
    { DSPTCHR_DEBUG_INTR_VEC_2_INTR_VEC_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_INTR_VEC_2_FIELDS[] =
{
    &DSPTCHR_DEBUG_INTR_VEC_2_INTR_VEC_2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_INTR_VEC_2 *****/
const ru_reg_rec DSPTCHR_DEBUG_INTR_VEC_2_REG =
{
    "DEBUG_INTR_VEC_2",
#if RU_INCLUDE_DESC
    "INTR_VEC_2 Register",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_2_REG_OFFSET },
    0,
    0,
    1372,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_INTR_VEC_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_INTR_VEC_3, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_INTR_VEC_3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INTR_VEC_3 *****/
const ru_field_rec DSPTCHR_DEBUG_INTR_VEC_3_INTR_VEC_3_FIELD =
{
    "INTR_VEC_3",
#if RU_INCLUDE_DESC
    "",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\nDATA bits 31:0\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_3_INTR_VEC_3_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_INTR_VEC_3_INTR_VEC_3_FIELD_WIDTH },
    { DSPTCHR_DEBUG_INTR_VEC_3_INTR_VEC_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_INTR_VEC_3_FIELDS[] =
{
    &DSPTCHR_DEBUG_INTR_VEC_3_INTR_VEC_3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_INTR_VEC_3 *****/
const ru_reg_rec DSPTCHR_DEBUG_INTR_VEC_3_REG =
{
    "DEBUG_INTR_VEC_3",
#if RU_INCLUDE_DESC
    "INTR_VEC_3 Register",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_3_REG_OFFSET },
    0,
    0,
    1373,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_INTR_VEC_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_INTR_VEC_4, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_INTR_VEC_4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INTR_VEC_4 *****/
const ru_field_rec DSPTCHR_DEBUG_INTR_VEC_4_INTR_VEC_4_FIELD =
{
    "INTR_VEC_4",
#if RU_INCLUDE_DESC
    "",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\nTarget_addr, 2b0,Source_addr,6b0,empty_viq_intr,msgdec_intr\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_4_INTR_VEC_4_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_INTR_VEC_4_INTR_VEC_4_FIELD_WIDTH },
    { DSPTCHR_DEBUG_INTR_VEC_4_INTR_VEC_4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_INTR_VEC_4_FIELDS[] =
{
    &DSPTCHR_DEBUG_INTR_VEC_4_INTR_VEC_4_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_INTR_VEC_4 *****/
const ru_reg_rec DSPTCHR_DEBUG_INTR_VEC_4_REG =
{
    "DEBUG_INTR_VEC_4",
#if RU_INCLUDE_DESC
    "INTR_VEC_4 Register",
    "Captures BB data and address in case of error (unknown message or empty VIQ)\nTarget_addr, 2b0,Source_addr,6b0,empty_viq_intr,msgdec_intr\n",
#endif
    { DSPTCHR_DEBUG_INTR_VEC_4_REG_OFFSET },
    0,
    0,
    1374,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_INTR_VEC_4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_STATISTICS_DBG_STTSTCS_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_MODE *****/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD =
{
    "DBG_MODE",
#if RU_INCLUDE_DESC
    "",
    "Selects mode to log\n",
#endif
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD_WIDTH },
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_CNTRS *****/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD =
{
    "EN_CNTRS",
#if RU_INCLUDE_DESC
    "",
    "Enable statistics\n",
#endif
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD_WIDTH },
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CLR_CNTRS *****/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD =
{
    "CLR_CNTRS",
#if RU_INCLUDE_DESC
    "",
    "Clears all counters\n",
#endif
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD_WIDTH },
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_RNR_BASE *****/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_BASE_FIELD =
{
    "DBG_RNR_BASE",
#if RU_INCLUDE_DESC
    "",
    "Selects start base for RNR to log\n",
#endif
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_BASE_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_BASE_FIELD_WIDTH },
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_BASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_RNR_SEL *****/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD =
{
    "DBG_RNR_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects RNR to log\n",
#endif
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_FIELDS[] =
{
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_BASE_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL *****/
const ru_reg_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_REG =
{
    "DEBUG_STATISTICS_DBG_STTSTCS_CTRL",
#if RU_INCLUDE_DESC
    "DEBUG_STATISTICS_CONTROL Register",
    "Controls which information to log\n",
#endif
    { DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_REG_OFFSET },
    0,
    0,
    1375,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_DEBUG_STATISTICS_DBG_CNT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_DEBUG_STATISTICS_DBG_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBG_VEC_VAL *****/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "",
    "Debug bus vector value\n",
#endif
    { DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD_MASK },
    0,
    { DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD_WIDTH },
    { DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_STATISTICS_DBG_CNT_FIELDS[] =
{
    &DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_DEBUG_STATISTICS_DBG_CNT *****/
const ru_reg_rec DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG =
{
    "DEBUG_STATISTICS_DBG_CNT",
#if RU_INCLUDE_DESC
    "DEBUG_COUNT 0..31 Register",
    "Debug counter\n",
#endif
    { DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG_OFFSET },
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG_RAM_CNT,
    4,
    1376,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_HEAD, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_HEAD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HEAD *****/
const ru_field_rec DSPTCHR_QDES_HEAD_HEAD_FIELD =
{
    "HEAD",
#if RU_INCLUDE_DESC
    "",
    "Pointer to the first BD in the link list of this queue.\n",
#endif
    { DSPTCHR_QDES_HEAD_HEAD_FIELD_MASK },
    0,
    { DSPTCHR_QDES_HEAD_HEAD_FIELD_WIDTH },
    { DSPTCHR_QDES_HEAD_HEAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_HEAD_FIELDS[] =
{
    &DSPTCHR_QDES_HEAD_HEAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_HEAD *****/
const ru_reg_rec DSPTCHR_QDES_HEAD_REG =
{
    "QDES_HEAD",
#if RU_INCLUDE_DESC
    "HEAD Register",
    "Pointer to the first BD in the link list of this queue.\n",
#endif
    { DSPTCHR_QDES_HEAD_REG_OFFSET },
    DSPTCHR_QDES_HEAD_REG_RAM_CNT,
    32,
    1377,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_HEAD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_BFOUT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_BFOUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BFOUT *****/
const ru_field_rec DSPTCHR_QDES_BFOUT_BFOUT_FIELD =
{
    "BFOUT",
#if RU_INCLUDE_DESC
    "",
    "32 bit wrap around counter. Counts number of packets that left this queue since start of queue activity.\n",
#endif
    { DSPTCHR_QDES_BFOUT_BFOUT_FIELD_MASK },
    0,
    { DSPTCHR_QDES_BFOUT_BFOUT_FIELD_WIDTH },
    { DSPTCHR_QDES_BFOUT_BFOUT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_BFOUT_FIELDS[] =
{
    &DSPTCHR_QDES_BFOUT_BFOUT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_BFOUT *****/
const ru_reg_rec DSPTCHR_QDES_BFOUT_REG =
{
    "QDES_BFOUT",
#if RU_INCLUDE_DESC
    "BFOUT Register",
    "32 bit wrap around counter. Counts number of packets that left this queue since start of queue activity.\n",
#endif
    { DSPTCHR_QDES_BFOUT_REG_OFFSET },
    DSPTCHR_QDES_BFOUT_REG_RAM_CNT,
    32,
    1378,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_BFOUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_BUFIN, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_BUFIN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BUFIN *****/
const ru_field_rec DSPTCHR_QDES_BUFIN_BUFIN_FIELD =
{
    "BUFIN",
#if RU_INCLUDE_DESC
    "",
    "32 bit wrap around counter. Counts number of packets that entered this queue since start of queue activity.\n",
#endif
    { DSPTCHR_QDES_BUFIN_BUFIN_FIELD_MASK },
    0,
    { DSPTCHR_QDES_BUFIN_BUFIN_FIELD_WIDTH },
    { DSPTCHR_QDES_BUFIN_BUFIN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_BUFIN_FIELDS[] =
{
    &DSPTCHR_QDES_BUFIN_BUFIN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_BUFIN *****/
const ru_reg_rec DSPTCHR_QDES_BUFIN_REG =
{
    "QDES_BUFIN",
#if RU_INCLUDE_DESC
    "BUFIN Register",
    "32 bit wrap around counter. Counts number of packets that entered this queue since start of queue activity.\n",
#endif
    { DSPTCHR_QDES_BUFIN_REG_OFFSET },
    DSPTCHR_QDES_BUFIN_REG_RAM_CNT,
    32,
    1379,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_BUFIN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_TAIL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_TAIL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TAIL *****/
const ru_field_rec DSPTCHR_QDES_TAIL_TAIL_FIELD =
{
    "TAIL",
#if RU_INCLUDE_DESC
    "",
    "Pointer to the last BD in the linked list of this queue.\n",
#endif
    { DSPTCHR_QDES_TAIL_TAIL_FIELD_MASK },
    0,
    { DSPTCHR_QDES_TAIL_TAIL_FIELD_WIDTH },
    { DSPTCHR_QDES_TAIL_TAIL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_TAIL_FIELDS[] =
{
    &DSPTCHR_QDES_TAIL_TAIL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_TAIL *****/
const ru_reg_rec DSPTCHR_QDES_TAIL_REG =
{
    "QDES_TAIL",
#if RU_INCLUDE_DESC
    "TAIL Register",
    "Pointer to the last BD in the linked list of this queue.\n",
#endif
    { DSPTCHR_QDES_TAIL_REG_OFFSET },
    DSPTCHR_QDES_TAIL_REG_RAM_CNT,
    32,
    1380,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_TAIL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_FBDNULL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_FBDNULL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FBDNULL *****/
const ru_field_rec DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD =
{
    "FBDNULL",
#if RU_INCLUDE_DESC
    "",
    "If this bit is set then the first BD attached to this Q is a null BD. In this case, its Data Pointer field is not valid, but its Next BD pointer field is valid. When it is set, the NullBD field for this queue is not valid.\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD_MASK },
    0,
    { DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD_WIDTH },
    { DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_FBDNULL_FIELDS[] =
{
    &DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_FBDNULL *****/
const ru_reg_rec DSPTCHR_QDES_FBDNULL_REG =
{
    "QDES_FBDNULL",
#if RU_INCLUDE_DESC
    "FBDNULL Register",
    "If this bit is set then the first BD attached to this Q is a null BD. In this case, its Data Pointer field is not valid, but its Next BD pointer field is valid. When it is set, the NullBD field for this queue is not valid.\n",
#endif
    { DSPTCHR_QDES_FBDNULL_REG_OFFSET },
    DSPTCHR_QDES_FBDNULL_REG_RAM_CNT,
    32,
    1381,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_FBDNULL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_NULLBD, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_NULLBD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NULLBD *****/
const ru_field_rec DSPTCHR_QDES_NULLBD_NULLBD_FIELD =
{
    "NULLBD",
#if RU_INCLUDE_DESC
    "",
    "32 bits index of a Null BD that belongs to this queue. Both the data buffer pointer and the next BD field are non valid. The pointer defines a memory allocation for a BD that might be used or not.\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_QDES_NULLBD_NULLBD_FIELD_MASK },
    0,
    { DSPTCHR_QDES_NULLBD_NULLBD_FIELD_WIDTH },
    { DSPTCHR_QDES_NULLBD_NULLBD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_NULLBD_FIELDS[] =
{
    &DSPTCHR_QDES_NULLBD_NULLBD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_NULLBD *****/
const ru_reg_rec DSPTCHR_QDES_NULLBD_REG =
{
    "QDES_NULLBD",
#if RU_INCLUDE_DESC
    "NULLBD Register",
    "32 bits index of a Null BD that belongs to this queue. Both the data buffer pointer and the next BD field are non valid. The pointer defines a memory allocation for a BD that might be used or not.\n",
#endif
    { DSPTCHR_QDES_NULLBD_REG_OFFSET },
    DSPTCHR_QDES_NULLBD_REG_RAM_CNT,
    32,
    1382,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_NULLBD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_BUFAVAIL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_BUFAVAIL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BUFAVAIL *****/
const ru_field_rec DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD =
{
    "BUFAVAIL",
#if RU_INCLUDE_DESC
    "",
    "number of entries available in queue.\nbufin - bfout\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD_MASK },
    0,
    { DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD_WIDTH },
    { DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_BUFAVAIL_FIELDS[] =
{
    &DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_BUFAVAIL *****/
const ru_reg_rec DSPTCHR_QDES_BUFAVAIL_REG =
{
    "QDES_BUFAVAIL",
#if RU_INCLUDE_DESC
    "BUFAVAIL Register",
    "number of entries available in queue.\nbufin - bfout\n",
#endif
    { DSPTCHR_QDES_BUFAVAIL_REG_OFFSET },
    DSPTCHR_QDES_BUFAVAIL_REG_RAM_CNT,
    32,
    1383,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_BUFAVAIL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_REG_Q_HEAD, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_REG_Q_HEAD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HEAD *****/
const ru_field_rec DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD =
{
    "HEAD",
#if RU_INCLUDE_DESC
    "",
    "Q HEAD\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD_MASK },
    0,
    { DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD_WIDTH },
    { DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_Q_HEAD_FIELDS[] =
{
    &DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_REG_Q_HEAD *****/
const ru_reg_rec DSPTCHR_QDES_REG_Q_HEAD_REG =
{
    "QDES_REG_Q_HEAD",
#if RU_INCLUDE_DESC
    "QUEUE_HEAD 0..8 Register",
    "Q Head Buffer, Used for the dispatching logic\n",
#endif
    { DSPTCHR_QDES_REG_Q_HEAD_REG_OFFSET },
    DSPTCHR_QDES_REG_Q_HEAD_REG_RAM_CNT,
    4,
    1384,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_REG_Q_HEAD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_REG_VIQ_HEAD_VLD, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_REG_VIQ_HEAD_VLD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VIQ_HEAD_VLD *****/
const ru_field_rec DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD =
{
    "VIQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "",
    "Q head valid. Each bit indicates for a specific VIQ if the head is valid or not\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_MASK },
    0,
    { DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_WIDTH },
    { DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_VIQ_HEAD_VLD_FIELDS[] =
{
    &DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_REG_VIQ_HEAD_VLD *****/
const ru_reg_rec DSPTCHR_QDES_REG_VIQ_HEAD_VLD_REG =
{
    "QDES_REG_VIQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "VIQ_HEAD_VALID Register",
    "This register will hold the for each VIQ if the Head of the Q is valid or not.\nThese Queues are for Dispatch\n\n",
#endif
    { DSPTCHR_QDES_REG_VIQ_HEAD_VLD_REG_OFFSET },
    0,
    0,
    1385,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_REG_VIQ_HEAD_VLD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_REG_VEQ_HEAD_VLD, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_REG_VEQ_HEAD_VLD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VIQ_HEAD_VLD *****/
const ru_field_rec DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD =
{
    "VIQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "",
    "Q head valid. Each bit indicates for a specific VIQ if the head is valid or not\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_MASK },
    0,
    { DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_WIDTH },
    { DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_VEQ_HEAD_VLD_FIELDS[] =
{
    &DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_REG_VEQ_HEAD_VLD *****/
const ru_reg_rec DSPTCHR_QDES_REG_VEQ_HEAD_VLD_REG =
{
    "QDES_REG_VEQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "VEQ_HEAD_VALID Register",
    "This register will hold the for each VEQ if the Head of the Q is valid or not\nThese Queues are for ReOrder\n",
#endif
    { DSPTCHR_QDES_REG_VEQ_HEAD_VLD_REG_OFFSET },
    0,
    0,
    1386,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_REG_VEQ_HEAD_VLD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_QDES_REG_QDES_BUF_AVL_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USE_BUF_AVL *****/
const ru_field_rec DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD =
{
    "USE_BUF_AVL",
#if RU_INCLUDE_DESC
    "",
    "Should buf_avail in the QDES affect poping from head of linked list\n",
#endif
    { DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD_MASK },
    0,
    { DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD_WIDTH },
    { DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEC_BUFOUT_WHEN_MLTCST *****/
const ru_field_rec DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD =
{
    "DEC_BUFOUT_WHEN_MLTCST",
#if RU_INCLUDE_DESC
    "",
    "Should buf_avail in the QDES affect poping from head of linked list\n",
#endif
    { DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD_MASK },
    0,
    { DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD_WIDTH },
    { DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_FIELDS[] =
{
    &DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD,
    &DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL *****/
const ru_reg_rec DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_REG =
{
    "QDES_REG_QDES_BUF_AVL_CNTRL",
#if RU_INCLUDE_DESC
    "QDES_BUF_AVAIL_CONTROL Register",
    "Todays implementation does not require that QDES available buffer be different than zero. so this register controls whether or not to it should affect poping from the QDES or not\n",
#endif
    { DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_REG_OFFSET },
    0,
    0,
    1387,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_FLLDES_HEAD, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_FLLDES_HEAD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HEAD *****/
const ru_field_rec DSPTCHR_FLLDES_HEAD_HEAD_FIELD =
{
    "HEAD",
#if RU_INCLUDE_DESC
    "",
    "Pointer to the first BD in the link list of this queue.\n",
#endif
    { DSPTCHR_FLLDES_HEAD_HEAD_FIELD_MASK },
    0,
    { DSPTCHR_FLLDES_HEAD_HEAD_FIELD_WIDTH },
    { DSPTCHR_FLLDES_HEAD_HEAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_HEAD_FIELDS[] =
{
    &DSPTCHR_FLLDES_HEAD_HEAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_FLLDES_HEAD *****/
const ru_reg_rec DSPTCHR_FLLDES_HEAD_REG =
{
    "FLLDES_HEAD",
#if RU_INCLUDE_DESC
    "HEAD Register",
    "Pointer to the first BD in the link list of this queue.\n",
#endif
    { DSPTCHR_FLLDES_HEAD_REG_OFFSET },
    0,
    0,
    1388,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_HEAD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_FLLDES_BFOUT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_FLLDES_BFOUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec DSPTCHR_FLLDES_BFOUT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "32 bit wrap around counter. Counts number of entries that left this queue since start of queue activity.\n",
#endif
    { DSPTCHR_FLLDES_BFOUT_COUNT_FIELD_MASK },
    0,
    { DSPTCHR_FLLDES_BFOUT_COUNT_FIELD_WIDTH },
    { DSPTCHR_FLLDES_BFOUT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_BFOUT_FIELDS[] =
{
    &DSPTCHR_FLLDES_BFOUT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_FLLDES_BFOUT *****/
const ru_reg_rec DSPTCHR_FLLDES_BFOUT_REG =
{
    "FLLDES_BFOUT",
#if RU_INCLUDE_DESC
    "BFOUT Register",
    "32 bit wrap around counter. Counts number of entries that left this queue since start of queue activity.\n",
#endif
    { DSPTCHR_FLLDES_BFOUT_REG_OFFSET },
    0,
    0,
    1389,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_BFOUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_FLLDES_BFIN, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_FLLDES_BFIN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BFIN *****/
const ru_field_rec DSPTCHR_FLLDES_BFIN_BFIN_FIELD =
{
    "BFIN",
#if RU_INCLUDE_DESC
    "",
    "32 bit wrap around counter. Counts number of entries that entered this queue since start of queue activity.\n",
#endif
    { DSPTCHR_FLLDES_BFIN_BFIN_FIELD_MASK },
    0,
    { DSPTCHR_FLLDES_BFIN_BFIN_FIELD_WIDTH },
    { DSPTCHR_FLLDES_BFIN_BFIN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_BFIN_FIELDS[] =
{
    &DSPTCHR_FLLDES_BFIN_BFIN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_FLLDES_BFIN *****/
const ru_reg_rec DSPTCHR_FLLDES_BFIN_REG =
{
    "FLLDES_BFIN",
#if RU_INCLUDE_DESC
    "BFIN Register",
    "32 bit wrap around counter. Counts number of entries that entered this queue since start of queue activity.\n",
#endif
    { DSPTCHR_FLLDES_BFIN_REG_OFFSET },
    0,
    0,
    1390,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_BFIN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_FLLDES_TAIL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_FLLDES_TAIL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TAIL *****/
const ru_field_rec DSPTCHR_FLLDES_TAIL_TAIL_FIELD =
{
    "TAIL",
#if RU_INCLUDE_DESC
    "",
    "Pointer to the last BD in the linked list of this queue.\n",
#endif
    { DSPTCHR_FLLDES_TAIL_TAIL_FIELD_MASK },
    0,
    { DSPTCHR_FLLDES_TAIL_TAIL_FIELD_WIDTH },
    { DSPTCHR_FLLDES_TAIL_TAIL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_TAIL_FIELDS[] =
{
    &DSPTCHR_FLLDES_TAIL_TAIL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_FLLDES_TAIL *****/
const ru_reg_rec DSPTCHR_FLLDES_TAIL_REG =
{
    "FLLDES_TAIL",
#if RU_INCLUDE_DESC
    "TAIL Register",
    "Pointer to the last BD in the linked list of this queue.\n",
#endif
    { DSPTCHR_FLLDES_TAIL_REG_OFFSET },
    0,
    0,
    1391,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_TAIL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_FLLDES_FLLDROP, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_FLLDES_FLLDROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DRPCNT *****/
const ru_field_rec DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD =
{
    "DRPCNT",
#if RU_INCLUDE_DESC
    "",
    "32 bit counter that counts the number of packets arrived when there is no free BD in the FLL.\n",
#endif
    { DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD_MASK },
    0,
    { DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD_WIDTH },
    { DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_FLLDROP_FIELDS[] =
{
    &DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_FLLDES_FLLDROP *****/
const ru_reg_rec DSPTCHR_FLLDES_FLLDROP_REG =
{
    "FLLDES_FLLDROP",
#if RU_INCLUDE_DESC
    "FLLDROP Register",
    "32 bit counter that counts the number of packets arrived when there is no free BD in the FLL.\n",
#endif
    { DSPTCHR_FLLDES_FLLDROP_REG_OFFSET },
    0,
    0,
    1392,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_FLLDROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_FLLDES_LTINT, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_FLLDES_LTINT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MINBUF *****/
const ru_field_rec DSPTCHR_FLLDES_LTINT_MINBUF_FIELD =
{
    "MINBUF",
#if RU_INCLUDE_DESC
    "",
    "Low threshold Interrupt. When number of bytes reach this level, then an interrupt is generated to the Host.\n",
#endif
    { DSPTCHR_FLLDES_LTINT_MINBUF_FIELD_MASK },
    0,
    { DSPTCHR_FLLDES_LTINT_MINBUF_FIELD_WIDTH },
    { DSPTCHR_FLLDES_LTINT_MINBUF_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_LTINT_FIELDS[] =
{
    &DSPTCHR_FLLDES_LTINT_MINBUF_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_FLLDES_LTINT *****/
const ru_reg_rec DSPTCHR_FLLDES_LTINT_REG =
{
    "FLLDES_LTINT",
#if RU_INCLUDE_DESC
    "LTINT Register",
    "Low threshold Interrupt. When number of bytes reach this level, then an interrupt is generated to the Host.\n",
#endif
    { DSPTCHR_FLLDES_LTINT_REG_OFFSET },
    0,
    0,
    1393,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_LTINT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_FLLDES_BUFAVAIL, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_FLLDES_BUFAVAIL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BUFAVAIL *****/
const ru_field_rec DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD =
{
    "BUFAVAIL",
#if RU_INCLUDE_DESC
    "",
    "number of entries available in queue.\nbufin - bfout\nCHANGED_BY_HW\n\n",
#endif
    { DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD_MASK },
    0,
    { DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD_WIDTH },
    { DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_BUFAVAIL_FIELDS[] =
{
    &DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_FLLDES_BUFAVAIL *****/
const ru_reg_rec DSPTCHR_FLLDES_BUFAVAIL_REG =
{
    "FLLDES_BUFAVAIL",
#if RU_INCLUDE_DESC
    "BUFAVAIL Register",
    "number of entries available in queue.\nbufin - bfout\n",
#endif
    { DSPTCHR_FLLDES_BUFAVAIL_REG_OFFSET },
    0,
    0,
    1394,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_BUFAVAIL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_FLLDES_FREEMIN, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_FLLDES_FREEMIN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FREEMIN *****/
const ru_field_rec DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD =
{
    "FREEMIN",
#if RU_INCLUDE_DESC
    "",
    "minum value of free BD recorded\n",
#endif
    { DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD_MASK },
    0,
    { DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD_WIDTH },
    { DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_FREEMIN_FIELDS[] =
{
    &DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_FLLDES_FREEMIN *****/
const ru_reg_rec DSPTCHR_FLLDES_FREEMIN_REG =
{
    "FLLDES_FREEMIN",
#if RU_INCLUDE_DESC
    "FREEMIN Register",
    "Save the MIN size of free BD in the system that has been recorded during work.\n",
#endif
    { DSPTCHR_FLLDES_FREEMIN_REG_OFFSET },
    0,
    0,
    1395,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_FREEMIN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_BDRAM_NEXT_DATA, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_BDRAM_NEXT_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DSPTCHR_BDRAM_NEXT_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data Buffer entry\n",
#endif
    { DSPTCHR_BDRAM_NEXT_DATA_DATA_FIELD_MASK },
    0,
    { DSPTCHR_BDRAM_NEXT_DATA_DATA_FIELD_WIDTH },
    { DSPTCHR_BDRAM_NEXT_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_BDRAM_NEXT_DATA_FIELDS[] =
{
    &DSPTCHR_BDRAM_NEXT_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_BDRAM_NEXT_DATA *****/
const ru_reg_rec DSPTCHR_BDRAM_NEXT_DATA_REG =
{
    "BDRAM_NEXT_DATA",
#if RU_INCLUDE_DESC
    "BD 0..2047 Register",
    "This Memory holds the Buffer Descriptor (BD) entries.\n",
#endif
    { DSPTCHR_BDRAM_NEXT_DATA_REG_OFFSET },
    DSPTCHR_BDRAM_NEXT_DATA_REG_RAM_CNT,
    4,
    1396,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_BDRAM_NEXT_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_BDRAM_PREV_DATA, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_BDRAM_PREV_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DSPTCHR_BDRAM_PREV_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data Buffer entry\n",
#endif
    { DSPTCHR_BDRAM_PREV_DATA_DATA_FIELD_MASK },
    0,
    { DSPTCHR_BDRAM_PREV_DATA_DATA_FIELD_WIDTH },
    { DSPTCHR_BDRAM_PREV_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_BDRAM_PREV_DATA_FIELDS[] =
{
    &DSPTCHR_BDRAM_PREV_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_BDRAM_PREV_DATA *****/
const ru_reg_rec DSPTCHR_BDRAM_PREV_DATA_REG =
{
    "BDRAM_PREV_DATA",
#if RU_INCLUDE_DESC
    "BD 0..2047 Register",
    "This Memory holds the Buffer Descriptor (BD) entries.\n",
#endif
    { DSPTCHR_BDRAM_PREV_DATA_REG_OFFSET },
    DSPTCHR_BDRAM_PREV_DATA_REG_RAM_CNT,
    4,
    1397,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_BDRAM_PREV_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: DSPTCHR_PDRAM_DATA, TYPE: Type_DSPTCHER_REORDR_TOP_DISPATCHER_PDRAM_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec DSPTCHR_PDRAM_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data Buffer entry\n",
#endif
    { DSPTCHR_PDRAM_DATA_DATA_FIELD_MASK },
    0,
    { DSPTCHR_PDRAM_DATA_DATA_FIELD_WIDTH },
    { DSPTCHR_PDRAM_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_PDRAM_DATA_FIELDS[] =
{
    &DSPTCHR_PDRAM_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: DSPTCHR_PDRAM_DATA *****/
const ru_reg_rec DSPTCHR_PDRAM_DATA_REG =
{
    "PDRAM_DATA",
#if RU_INCLUDE_DESC
    "PDRAM 0..8191 Register",
    "This memory holds the Packet descriptors.\n",
#endif
    { DSPTCHR_PDRAM_DATA_REG_OFFSET },
    DSPTCHR_PDRAM_DATA_REG_RAM_CNT,
    4,
    1398,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_PDRAM_DATA_FIELDS,
#endif
};

unsigned long DSPTCHR_ADDRS[] =
{
    0xA2000000,
    0xA2010000,
    0xA2020000,
};

static const ru_reg_rec *DSPTCHR_REGS[] =
{
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG,
    &DSPTCHR_REORDER_CFG_VQ_EN_REG,
    &DSPTCHR_REORDER_CFG_BB_CFG_REG,
    &DSPTCHR_REORDER_CFG_BB_CFG_RT_REG,
    &DSPTCHR_REORDER_CFG_BB_CMN_CFG_REG,
    &DSPTCHR_REORDER_CFG_BB_CMN_CFG_RT_REG,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_2ND_REG,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_2ND_REG,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_REG,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_2ND_REG,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_2ND_REG,
    &DSPTCHR_CONGESTION_CONGESTN_THRSHLD_ACT_REG,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_REG,
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG,
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG,
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_VHIGH_REG,
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_FC_REG,
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG,
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG,
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_VHIGH_REG,
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_FC_REG,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT1_REG,
    &DSPTCHR_INGRS_QUEUES_Q_ING_WRR_WGHT2_REG,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT1_REG,
    &DSPTCHR_INGRS_QUEUES_Q_ORD_WRR_WGHT2_REG,
    &DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_REG,
    &DSPTCHR_POOL_SIZES_CMN_POOL_LMT_REG,
    &DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_REG,
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_REG,
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_REG,
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_REG,
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_REG,
    &DSPTCHR_POOL_SIZES_RNR_POOL_LMT_REG,
    &DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_REG,
    &DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_REG,
    &DSPTCHR_MASK_MSK_Q_REG,
    &DSPTCHR_MASK_DLY_Q_REG,
    &DSPTCHR_MASK_NON_DLY_Q_REG,
    &DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG,
    &DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG,
    &DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG,
    &DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_CRDT_REG,
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG,
    &DSPTCHR_DEBUG_DBG_BUS_CNTRL_REG,
    &DSPTCHR_DEBUG_DBG_VEC_0_REG,
    &DSPTCHR_DEBUG_DBG_VEC_1_REG,
    &DSPTCHR_DEBUG_DBG_VEC_2_REG,
    &DSPTCHR_DEBUG_DBG_VEC_3_REG,
    &DSPTCHR_DEBUG_DBG_VEC_4_REG,
    &DSPTCHR_DEBUG_DBG_VEC_5_REG,
    &DSPTCHR_DEBUG_DBG_VEC_6_REG,
    &DSPTCHR_DEBUG_DBG_VEC_7_REG,
    &DSPTCHR_DEBUG_DBG_VEC_8_REG,
    &DSPTCHR_DEBUG_DBG_VEC_9_REG,
    &DSPTCHR_DEBUG_DBG_VEC_10_REG,
    &DSPTCHR_DEBUG_DBG_VEC_11_REG,
    &DSPTCHR_DEBUG_DBG_VEC_12_REG,
    &DSPTCHR_DEBUG_DBG_VEC_13_REG,
    &DSPTCHR_DEBUG_DBG_VEC_14_REG,
    &DSPTCHR_DEBUG_DBG_VEC_15_REG,
    &DSPTCHR_DEBUG_DBG_VEC_16_REG,
    &DSPTCHR_DEBUG_DBG_VEC_17_REG,
    &DSPTCHR_DEBUG_DBG_VEC_18_REG,
    &DSPTCHR_DEBUG_DBG_VEC_19_REG,
    &DSPTCHR_DEBUG_DBG_VEC_20_REG,
    &DSPTCHR_DEBUG_DBG_VEC_21_REG,
    &DSPTCHR_DEBUG_DBG_VEC_22_REG,
    &DSPTCHR_DEBUG_DBG_VEC_23_REG,
    &DSPTCHR_DEBUG_DBG_VEC_24_REG,
    &DSPTCHR_DEBUG_DBG_VEC_25_REG,
    &DSPTCHR_DEBUG_INTR_VEC_0_REG,
    &DSPTCHR_DEBUG_INTR_VEC_1_REG,
    &DSPTCHR_DEBUG_INTR_VEC_2_REG,
    &DSPTCHR_DEBUG_INTR_VEC_3_REG,
    &DSPTCHR_DEBUG_INTR_VEC_4_REG,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_REG,
    &DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG,
    &DSPTCHR_QDES_HEAD_REG,
    &DSPTCHR_QDES_BFOUT_REG,
    &DSPTCHR_QDES_BUFIN_REG,
    &DSPTCHR_QDES_TAIL_REG,
    &DSPTCHR_QDES_FBDNULL_REG,
    &DSPTCHR_QDES_NULLBD_REG,
    &DSPTCHR_QDES_BUFAVAIL_REG,
    &DSPTCHR_QDES_REG_Q_HEAD_REG,
    &DSPTCHR_QDES_REG_VIQ_HEAD_VLD_REG,
    &DSPTCHR_QDES_REG_VEQ_HEAD_VLD_REG,
    &DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_REG,
    &DSPTCHR_FLLDES_HEAD_REG,
    &DSPTCHR_FLLDES_BFOUT_REG,
    &DSPTCHR_FLLDES_BFIN_REG,
    &DSPTCHR_FLLDES_TAIL_REG,
    &DSPTCHR_FLLDES_FLLDROP_REG,
    &DSPTCHR_FLLDES_LTINT_REG,
    &DSPTCHR_FLLDES_BUFAVAIL_REG,
    &DSPTCHR_FLLDES_FREEMIN_REG,
    &DSPTCHR_BDRAM_NEXT_DATA_REG,
    &DSPTCHR_BDRAM_PREV_DATA_REG,
    &DSPTCHR_PDRAM_DATA_REG,
};

const ru_block_rec DSPTCHR_BLOCK =
{
    "DSPTCHR",
#if RU_INCLUDE_SIZE
    { 0x00010000 },
#endif
    DSPTCHR_ADDRS,
    3,
    119,
    DSPTCHR_REGS,
};
