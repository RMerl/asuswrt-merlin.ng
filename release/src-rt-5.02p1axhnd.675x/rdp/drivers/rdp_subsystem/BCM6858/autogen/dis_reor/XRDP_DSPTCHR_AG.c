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
 * Field: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "ENABLE",
    "Enable dispatcher reorder block",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED0_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD =
{
    "RDY",
#if RU_INCLUDE_DESC
    "READY",
    "Dispatcher reorder block is RDY",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED1_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED1_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REORDR_PAR_MOD
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REORDR_PAR_MOD_FIELD =
{
    "REORDR_PAR_MOD",
#if RU_INCLUDE_DESC
    "REORDER_SM_PARALLEL_MODE_",
    "Enables parallel operation of Re-Order scheduler to Re-Order SM."
    ""
    "Reduces Re-Order cycle from 16 clocks to 7.",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REORDR_PAR_MOD_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REORDR_PAR_MOD_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REORDR_PAR_MOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD =
{
    "PER_Q_EGRS_CONGST_EN",
#if RU_INCLUDE_DESC
    "EGRESS_PER_Q_CONGESTION_ENALBE",
    "Enable per Q Egress congestion monitoring",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSPTCHR_PER_ENH_POD
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSPTCHR_PER_ENH_POD_FIELD =
{
    "DSPTCHR_PER_ENH_POD",
#if RU_INCLUDE_DESC
    "DISPATCHER_SM_PERFORMANCE_ENH_MODE_",
    "Enables Enhanced performance mode of Dispatcher Load balancing and Dispatcher SM."
    ""
    "This allows Disptach of PD to RNR instead of every 14 clocks, every 11 clocks.",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSPTCHR_PER_ENH_POD_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSPTCHR_PER_ENH_POD_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSPTCHR_PER_ENH_POD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED2
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED2_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED2_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_VQ_EN_EN
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "ENABLE",
    "Enable Virtual Q control - 32 bit vector.",
#endif
    DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD =
{
    "SRC_ID",
#if RU_INCLUDE_DESC
    "SOURCE_ID",
    "Source ID - Dispatcher",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_BB_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED0_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD =
{
    "DST_ID_OVRIDE",
#if RU_INCLUDE_DESC
    "DEST_ID_OVERRIDE",
    "Enable dispatcher reorder block",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_BB_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED1_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED1_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_BB_CFG_ROUTE_OVRIDE
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_ROUTE_OVRIDE_FIELD =
{
    "ROUTE_OVRIDE",
#if RU_INCLUDE_DESC
    "DEST_ROUTE_OVERRIDE",
    "Use this route address instead of pre-configured",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_ROUTE_OVRIDE_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_BB_CFG_ROUTE_OVRIDE_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_BB_CFG_ROUTE_OVRIDE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_BB_CFG_RESERVED2
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED2_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED2_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD =
{
    "OVRIDE_EN",
#if RU_INCLUDE_DESC
    "OVERRIDE_ENABLE",
    "Enable dispatcher reorder block",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_BB_CFG_RESERVED3
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_BB_CFG_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED3_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED3_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_BB_CFG_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD =
{
    "FRST_LVL",
#if RU_INCLUDE_DESC
    "FIRST_LEVEL",
    "First Level congestion threshold.",
#endif
    DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD_WIDTH,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD =
{
    "SCND_LVL",
#if RU_INCLUDE_DESC
    "SECOND_LEVEL",
    "Second Level congestion threshold.",
#endif
    DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD_WIDTH,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "HYST_THRESHOLD",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed",
#endif
    DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD_WIDTH,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD =
{
    "FRST_LVL",
#if RU_INCLUDE_DESC
    "FIRST_LEVEL",
    "First Level congestion threshold.",
#endif
    DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD_WIDTH,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD =
{
    "SCND_LVL",
#if RU_INCLUDE_DESC
    "SECOND_LEVEL",
    "Second Level congestion threshold.",
#endif
    DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD_WIDTH,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "HYST_THRESHOLD",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed",
#endif
    DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD_WIDTH,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD =
{
    "FRST_LVL",
#if RU_INCLUDE_DESC
    "FIRST_LEVEL",
    "First Level congestion threshold.",
#endif
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD_WIDTH,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD =
{
    "SCND_LVL",
#if RU_INCLUDE_DESC
    "SECOND_LEVEL",
    "Second Level congestion threshold.",
#endif
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD_WIDTH,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "HYST_THRESHOLD",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed",
#endif
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD_WIDTH,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD =
{
    "FRST_LVL",
#if RU_INCLUDE_DESC
    "FIRST_LEVEL",
    "First Level congestion threshold.",
#endif
    DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD_WIDTH,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD =
{
    "SCND_LVL",
#if RU_INCLUDE_DESC
    "SECOND_LEVEL",
    "Second Level congestion threshold.",
#endif
    DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD_WIDTH,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD =
{
    "HYST_THRS",
#if RU_INCLUDE_DESC
    "HYST_THRESHOLD",
    "Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the (threshold_level - HYST_TRSH) will the congestion indication be removed",
#endif
    DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD_WIDTH,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD =
{
    "GLBL_CONGSTN",
#if RU_INCLUDE_DESC
    "GLOBAL_CONGESTION",
    "Global congestion levels (according to FLL buffer availability)"
    "",
#endif
    DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD_WIDTH,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED0_FIELD_WIDTH,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD =
{
    "GLBL_EGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "GLOBAL_EGRESS_CONGESTION",
    "Global Egress congestion levels",
#endif
    DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD_WIDTH,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED1_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED1_FIELD_WIDTH,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD =
{
    "SBPM_CONGSTN",
#if RU_INCLUDE_DESC
    "SBPM_CONGESTION",
    "SBPM congestion levels according to SPBM messages",
#endif
    DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD_WIDTH,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED2
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED2_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED2_FIELD_WIDTH,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "CONGESTION_STATE",
    "1 - Passed Threshold"
    "0 - Did not pass threshold",
#endif
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_WIDTH,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "CONGESTION_STATE",
    "1 - Passed Threshold"
    "0 - Did not pass threshold",
#endif
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_WIDTH,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "CONGESTION_STATE",
    "1 - Passed Threshold"
    "0 - Did not pass threshold",
#endif
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_WIDTH,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE
 ******************************************************************************/
const ru_field_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD =
{
    "CONGSTN_STATE",
#if RU_INCLUDE_DESC
    "CONGESTION_STATE",
    "1 - Passed Threshold"
    "0 - Did not pass threshold",
#endif
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_MASK,
    0,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_WIDTH,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT
 ******************************************************************************/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD =
{
    "CMN_CNT",
#if RU_INCLUDE_DESC
    "COMMON",
    "Common number of buffers allocated to this Q.",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD_MASK,
    0,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD_WIDTH,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_RESERVED0_FIELD_WIDTH,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX
 ******************************************************************************/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD =
{
    "CMN_MAX",
#if RU_INCLUDE_DESC
    "COMMON_MAX",
    "Maximum number of buffers allowed to be allocated to the specific VIQ from the common Pool",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD_MASK,
    0,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD_WIDTH,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX
 ******************************************************************************/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD =
{
    "GURNTD_MAX",
#if RU_INCLUDE_DESC
    "GUARANTEED_MAX",
    "Maximum number of buffers allowed to be allocated to the specific VIQ from the guaranteed Pool",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD_MASK,
    0,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD_WIDTH,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT
 ******************************************************************************/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD =
{
    "CREDIT_CNT",
#if RU_INCLUDE_DESC
    "CREDIT_COUNTER",
    "Holds the value of the the accumulated credits. this is sent to the BBH/RNR."
    "BBH disregards the value. RNR uses it to to calculate the amount of available credits.",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD_MASK,
    0,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD_WIDTH,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_CNT
 ******************************************************************************/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_CNT_FIELD =
{
    "CHRNCY_CNT",
#if RU_INCLUDE_DESC
    "COHERENCY",
    "Coherency counter value. BBH sends a coherency message per PD. Coherency messages are counted and only if there is at least 1 coherency message can a PD be forwarded to the RNR for processing.",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_CNT_FIELD_MASK,
    0,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_CNT_FIELD_WIDTH,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_EN
 ******************************************************************************/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_EN_FIELD =
{
    "CHRNCY_EN",
#if RU_INCLUDE_DESC
    "COHERENCY_EN",
    "Enable coherency counting. In case RNR is allocated to a specific VIQ it will not send coherency messages so there is no need to take them into consideration during PD dispatch",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_EN_FIELD_MASK,
    0,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_EN_FIELD_WIDTH,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_RSRV
 ******************************************************************************/
const ru_field_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_RSRV_FIELD =
{
    "RSRV",
#if RU_INCLUDE_DESC
    "reserve",
    "Reserve",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_RSRV_FIELD_MASK,
    0,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_RSRV_FIELD_WIDTH,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_RSRV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD =
{
    "BB_ID",
#if RU_INCLUDE_DESC
    "BroadBus_ID",
    "BroadBud ID: To which BroadBud agent (RNR/BBH) is the current Q associated with",
#endif
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_CRDT_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_CRDT_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_RESERVED0_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD =
{
    "TRGT_ADD",
#if RU_INCLUDE_DESC
    "TARGET_ADDRESS",
    "Target address within the BB agent where the credit message should be written to."
    ""
    "In case of RNR:"
    "27:16 - Ram address"
    "31:28 - Task number to wakeup",
#endif
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_BASE_ADD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_BASE_ADD_FIELD =
{
    "BASE_ADD",
#if RU_INCLUDE_DESC
    "BASE_ADDRESS",
    "Base address within each RNR",
#endif
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_BASE_ADD_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_BASE_ADD_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_BASE_ADD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_OFFSET_ADD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_OFFSET_ADD_FIELD =
{
    "OFFSET_ADD",
#if RU_INCLUDE_DESC
    "OFFSET_ADDRESS",
    "OFFSET address, in conjunction with base address for each task there will be a different address to where to send the PD"
    ""
    "ADD = BASE_ADD + (OFFSET_ADD x TASK)"
    ""
    "PD size is 128bits"
    "",
#endif
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_OFFSET_ADD_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_OFFSET_ADD_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_OFFSET_ADD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "QEUEU_0",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "QEUEU_1",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD =
{
    "Q2",
#if RU_INCLUDE_DESC
    "QEUEU_2",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD =
{
    "Q3",
#if RU_INCLUDE_DESC
    "QEUEU_3",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD =
{
    "Q4",
#if RU_INCLUDE_DESC
    "QEUEU_4",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD =
{
    "Q5",
#if RU_INCLUDE_DESC
    "QEUEU_5",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD =
{
    "Q6",
#if RU_INCLUDE_DESC
    "QEUEU_6",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD =
{
    "Q7",
#if RU_INCLUDE_DESC
    "QEUEU_7",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD =
{
    "Q8",
#if RU_INCLUDE_DESC
    "QEUEU_8",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q9
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q9_FIELD =
{
    "Q9",
#if RU_INCLUDE_DESC
    "QEUEU_9",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q9_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q9_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q10
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q10_FIELD =
{
    "Q10",
#if RU_INCLUDE_DESC
    "QEUEU_10",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q10_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q10_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q11
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q11_FIELD =
{
    "Q11",
#if RU_INCLUDE_DESC
    "QEUEU_11",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q11_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q11_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q12
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q12_FIELD =
{
    "Q12",
#if RU_INCLUDE_DESC
    "QEUEU_12",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q12_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q12_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q13
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q13_FIELD =
{
    "Q13",
#if RU_INCLUDE_DESC
    "QEUEU_13",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q13_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q13_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q14
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q14_FIELD =
{
    "Q14",
#if RU_INCLUDE_DESC
    "QEUEU_14",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q14_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q14_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q15
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q15_FIELD =
{
    "Q15",
#if RU_INCLUDE_DESC
    "QEUEU_15",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q15_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q15_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q16
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q16_FIELD =
{
    "Q16",
#if RU_INCLUDE_DESC
    "QEUEU_16",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q16_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q16_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q16_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q17
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q17_FIELD =
{
    "Q17",
#if RU_INCLUDE_DESC
    "QEUEU_17",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q17_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q17_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q17_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q18
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q18_FIELD =
{
    "Q18",
#if RU_INCLUDE_DESC
    "QEUEU_18",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q18_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q18_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q18_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q19
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q19_FIELD =
{
    "Q19",
#if RU_INCLUDE_DESC
    "QEUEU_19",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q19_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q19_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q19_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q20
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q20_FIELD =
{
    "Q20",
#if RU_INCLUDE_DESC
    "QEUEU_20",
    "0- Dispatcher"
    "1- Reorder"
    "",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q20_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q20_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q20_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q21
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q21_FIELD =
{
    "Q21",
#if RU_INCLUDE_DESC
    "QEUEU_21",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q21_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q21_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q21_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q22
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q22_FIELD =
{
    "Q22",
#if RU_INCLUDE_DESC
    "QEUEU_22",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q22_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q22_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q22_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q23
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q23_FIELD =
{
    "Q23",
#if RU_INCLUDE_DESC
    "QEUEU_23",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q23_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q23_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q23_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q24
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q24_FIELD =
{
    "Q24",
#if RU_INCLUDE_DESC
    "QEUEU_24",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q24_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q24_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q24_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q25
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q25_FIELD =
{
    "Q25",
#if RU_INCLUDE_DESC
    "QEUEU_25",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q25_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q25_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q25_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q26
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q26_FIELD =
{
    "Q26",
#if RU_INCLUDE_DESC
    "QEUEU_26",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q26_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q26_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q26_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q27
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q27_FIELD =
{
    "Q27",
#if RU_INCLUDE_DESC
    "QEUEU_27",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q27_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q27_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q27_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q28
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q28_FIELD =
{
    "Q28",
#if RU_INCLUDE_DESC
    "QEUEU_28",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q28_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q28_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q28_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q29
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q29_FIELD =
{
    "Q29",
#if RU_INCLUDE_DESC
    "QEUEU_29",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q29_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q29_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q29_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q30
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q30_FIELD =
{
    "Q30",
#if RU_INCLUDE_DESC
    "QEUEU_30",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q30_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q30_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q30_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QUEUE_MAPPING_Q_DEST_Q31
 ******************************************************************************/
const ru_field_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_Q31_FIELD =
{
    "Q31",
#if RU_INCLUDE_DESC
    "QEUEU_31",
    "0- Dispatcher"
    "1- Reorder",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q31_FIELD_MASK,
    0,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q31_FIELD_WIDTH,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_Q31_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD =
{
    "POOL_LMT",
#if RU_INCLUDE_DESC
    "POOL_LMT",
    "MAX number of buffers allowed in the pool",
#endif
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_CMN_POOL_LMT_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_CMN_POOL_LMT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "POOL_SIZE",
    "Number of buffers currently in the pool",
#endif
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD =
{
    "POOL_LMT",
#if RU_INCLUDE_DESC
    "POOL_LMT",
    "MAX number of buffers allowed in the pool",
#endif
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "POOL_SIZE",
    "Number of buffers currently in the pool",
#endif
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD =
{
    "POOL_LMT",
#if RU_INCLUDE_DESC
    "POOL_LMT",
    "MAX number of buffers allowed in the pool",
#endif
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "POOL_SIZE",
    "Number of buffers currently in the pool",
#endif
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD =
{
    "POOL_LMT",
#if RU_INCLUDE_DESC
    "POOL_LMT",
    "MAX number of buffers allowed in the pool",
#endif
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_RNR_POOL_LMT_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_RNR_POOL_LMT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "POOL_SIZE",
    "Number of buffers currently in the pool",
#endif
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD =
{
    "POOL_SIZE",
#if RU_INCLUDE_DESC
    "POOL_SIZE",
    "Number of buffers currently in the pool",
#endif
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_RESERVED0_FIELD_WIDTH,
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_MASK_MSK_TSK_255_0_MASK
 ******************************************************************************/
const ru_field_rec DSPTCHR_MASK_MSK_TSK_255_0_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "MASK",
    "MASK"
    "",
#endif
    DSPTCHR_MASK_MSK_TSK_255_0_MASK_FIELD_MASK,
    0,
    DSPTCHR_MASK_MSK_TSK_255_0_MASK_FIELD_WIDTH,
    DSPTCHR_MASK_MSK_TSK_255_0_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_MASK_MSK_Q_MASK
 ******************************************************************************/
const ru_field_rec DSPTCHR_MASK_MSK_Q_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "MASK",
    "MASK",
#endif
    DSPTCHR_MASK_MSK_Q_MASK_FIELD_MASK,
    0,
    DSPTCHR_MASK_MSK_Q_MASK_FIELD_WIDTH,
    DSPTCHR_MASK_MSK_Q_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_MASK_DLY_Q_MASK
 ******************************************************************************/
const ru_field_rec DSPTCHR_MASK_DLY_Q_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "MASK",
    "MASK",
#endif
    DSPTCHR_MASK_DLY_Q_MASK_FIELD_MASK,
    0,
    DSPTCHR_MASK_DLY_Q_MASK_FIELD_WIDTH,
    DSPTCHR_MASK_DLY_Q_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_MASK_NON_DLY_Q_MASK
 ******************************************************************************/
const ru_field_rec DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "MASK",
    "MASK",
#endif
    DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD_MASK,
    0,
    DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD_WIDTH,
    DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT
 ******************************************************************************/
const ru_field_rec DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD =
{
    "DLY_CRDT",
#if RU_INCLUDE_DESC
    "AVIALABLE_CREDIT",
    "The amount of free credits the re-order can utilize to send PDs to the QM",
#endif
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD_MASK,
    0,
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD_WIDTH,
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_RESERVED0_FIELD_WIDTH,
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT
 ******************************************************************************/
const ru_field_rec DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD =
{
    "NON_DLY_CRDT",
#if RU_INCLUDE_DESC
    "AVIALABLE_CREDIT",
    "The amount of free credits the re-order can utilize to send PDs to the QM",
#endif
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD_MASK,
    0,
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD_WIDTH,
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_RESERVED0_FIELD_WIDTH,
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE
 ******************************************************************************/
const ru_field_rec DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD =
{
    "TOTAL_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "TOTAL_EGRESS_SIZE",
    "Accumulates all buffers that are marked as egress (after dispatch and before sending to QM)",
#endif
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD_MASK,
    0,
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD_WIDTH,
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_RESERVED0_FIELD_WIDTH,
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE
 ******************************************************************************/
const ru_field_rec DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD =
{
    "Q_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "Q_EGRESS_SIZE",
    "Accumulates all buffers that are marked as egress (after dispatch and before sending to QM)",
#endif
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD_MASK,
    0,
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD_WIDTH,
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_RESERVED0_FIELD_WIDTH,
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "QEUEU_0",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "QEUEU_1",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD =
{
    "Q2",
#if RU_INCLUDE_DESC
    "QEUEU_2",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD =
{
    "Q3",
#if RU_INCLUDE_DESC
    "QEUEU_3",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD =
{
    "Q4",
#if RU_INCLUDE_DESC
    "QEUEU_4",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD =
{
    "Q5",
#if RU_INCLUDE_DESC
    "QEUEU_5",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD =
{
    "Q6",
#if RU_INCLUDE_DESC
    "QEUEU_6",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD =
{
    "Q7",
#if RU_INCLUDE_DESC
    "QEUEU_7",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD =
{
    "Q8",
#if RU_INCLUDE_DESC
    "QEUEU_8",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q9
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q9_FIELD =
{
    "Q9",
#if RU_INCLUDE_DESC
    "QEUEU_9",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q9_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q9_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q10
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q10_FIELD =
{
    "Q10",
#if RU_INCLUDE_DESC
    "QEUEU_10",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q10_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q10_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q11
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q11_FIELD =
{
    "Q11",
#if RU_INCLUDE_DESC
    "QEUEU_11",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q11_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q11_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q12
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q12_FIELD =
{
    "Q12",
#if RU_INCLUDE_DESC
    "QEUEU_12",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q12_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q12_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q13
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q13_FIELD =
{
    "Q13",
#if RU_INCLUDE_DESC
    "QEUEU_13",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q13_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q13_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q14
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q14_FIELD =
{
    "Q14",
#if RU_INCLUDE_DESC
    "QEUEU_14",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q14_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q14_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q15
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q15_FIELD =
{
    "Q15",
#if RU_INCLUDE_DESC
    "QEUEU_15",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q15_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q15_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q16
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q16_FIELD =
{
    "Q16",
#if RU_INCLUDE_DESC
    "QEUEU_16",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q16_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q16_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q16_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q17
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q17_FIELD =
{
    "Q17",
#if RU_INCLUDE_DESC
    "QEUEU_17",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q17_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q17_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q17_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q18
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q18_FIELD =
{
    "Q18",
#if RU_INCLUDE_DESC
    "QEUEU_18",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q18_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q18_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q18_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q19
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q19_FIELD =
{
    "Q19",
#if RU_INCLUDE_DESC
    "QEUEU_19",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q19_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q19_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q19_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q20
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q20_FIELD =
{
    "Q20",
#if RU_INCLUDE_DESC
    "QEUEU_20",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q20_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q20_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q20_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q21
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q21_FIELD =
{
    "Q21",
#if RU_INCLUDE_DESC
    "QEUEU_21",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q21_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q21_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q21_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q22
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q22_FIELD =
{
    "Q22",
#if RU_INCLUDE_DESC
    "QEUEU_22",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q22_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q22_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q22_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q23
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q23_FIELD =
{
    "Q23",
#if RU_INCLUDE_DESC
    "QEUEU_23",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q23_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q23_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q23_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q24
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q24_FIELD =
{
    "Q24",
#if RU_INCLUDE_DESC
    "QEUEU_24",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q24_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q24_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q24_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q25
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q25_FIELD =
{
    "Q25",
#if RU_INCLUDE_DESC
    "QEUEU_25",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q25_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q25_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q25_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q26
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q26_FIELD =
{
    "Q26",
#if RU_INCLUDE_DESC
    "QEUEU_26",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q26_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q26_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q26_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q27
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q27_FIELD =
{
    "Q27",
#if RU_INCLUDE_DESC
    "QEUEU_27",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q27_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q27_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q27_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q28
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q28_FIELD =
{
    "Q28",
#if RU_INCLUDE_DESC
    "QEUEU_28",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q28_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q28_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q28_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q29
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q29_FIELD =
{
    "Q29",
#if RU_INCLUDE_DESC
    "QEUEU_29",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q29_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q29_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q29_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q30
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q30_FIELD =
{
    "Q30",
#if RU_INCLUDE_DESC
    "QEUEU_30",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q30_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q30_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q30_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q31
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q31_FIELD =
{
    "Q31",
#if RU_INCLUDE_DESC
    "QEUEU_31",
    "wakeup request pending",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q31_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q31_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q31_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD =
{
    "WKUP_THRSHLD",
#if RU_INCLUDE_DESC
    "WAKEUP_THRESHOLD",
    "Wakeup threshold. Once number of Guaranteed buffer count crosses the threshold and there is a pending wakeup request, the dispatcher will issue a wakeup message to the appropriate runner according to a predefind address configuration",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_RESERVED0_FIELD_WIDTH,
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD =
{
    "Q_CRDT",
#if RU_INCLUDE_DESC
    "AVAILABLE_CREDITS",
    "availabe credits in bytes. Q will not be permitted to dispatch PDs if credit levels are below zero",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD =
{
    "NGTV",
#if RU_INCLUDE_DESC
    "NEGATIVE",
    "Bit will be enabled if credit levels are below zero. 2 compliment",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD =
{
    "QUNTUM",
#if RU_INCLUDE_DESC
    "QUANTUM",
    "Quantum size. Should be configured according to Q rate. in Bytes",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD =
{
    "Q0",
#if RU_INCLUDE_DESC
    "QEUEU_0",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD =
{
    "Q1",
#if RU_INCLUDE_DESC
    "QEUEU_1",
    "Valid Credits.",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD =
{
    "Q2",
#if RU_INCLUDE_DESC
    "QEUEU_2",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD =
{
    "Q3",
#if RU_INCLUDE_DESC
    "QEUEU_3",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD =
{
    "Q4",
#if RU_INCLUDE_DESC
    "QEUEU_4",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD =
{
    "Q5",
#if RU_INCLUDE_DESC
    "QEUEU_5",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD =
{
    "Q6",
#if RU_INCLUDE_DESC
    "QEUEU_6",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD =
{
    "Q7",
#if RU_INCLUDE_DESC
    "QEUEU_7",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD =
{
    "Q8",
#if RU_INCLUDE_DESC
    "QEUEU_8",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD =
{
    "Q9",
#if RU_INCLUDE_DESC
    "QEUEU_9",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD =
{
    "Q10",
#if RU_INCLUDE_DESC
    "QEUEU_10",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD =
{
    "Q11",
#if RU_INCLUDE_DESC
    "QEUEU_11",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD =
{
    "Q12",
#if RU_INCLUDE_DESC
    "QEUEU_12",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD =
{
    "Q13",
#if RU_INCLUDE_DESC
    "QEUEU_13",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD =
{
    "Q14",
#if RU_INCLUDE_DESC
    "QEUEU_14",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD =
{
    "Q15",
#if RU_INCLUDE_DESC
    "QEUEU_15",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD =
{
    "Q16",
#if RU_INCLUDE_DESC
    "QEUEU_16",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q16_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD =
{
    "Q17",
#if RU_INCLUDE_DESC
    "QEUEU_17",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q17_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD =
{
    "Q18",
#if RU_INCLUDE_DESC
    "QEUEU_18",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q18_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD =
{
    "Q19",
#if RU_INCLUDE_DESC
    "QEUEU_19",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q19_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD =
{
    "Q20",
#if RU_INCLUDE_DESC
    "QEUEU_20",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q20_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD =
{
    "Q21",
#if RU_INCLUDE_DESC
    "QEUEU_21",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q21_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD =
{
    "Q22",
#if RU_INCLUDE_DESC
    "QEUEU_22",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q22_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD =
{
    "Q23",
#if RU_INCLUDE_DESC
    "QEUEU_23",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q23_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD =
{
    "Q24",
#if RU_INCLUDE_DESC
    "QEUEU_24",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q24_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD =
{
    "Q25",
#if RU_INCLUDE_DESC
    "QEUEU_25",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q25_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD =
{
    "Q26",
#if RU_INCLUDE_DESC
    "QEUEU_26",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q26_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD =
{
    "Q27",
#if RU_INCLUDE_DESC
    "QEUEU_27",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q27_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD =
{
    "Q28",
#if RU_INCLUDE_DESC
    "QEUEU_28",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q28_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD =
{
    "Q29",
#if RU_INCLUDE_DESC
    "QEUEU_29",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q29_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD =
{
    "Q30",
#if RU_INCLUDE_DESC
    "QEUEU_30",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q30_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31
 ******************************************************************************/
const ru_field_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD =
{
    "Q31",
#if RU_INCLUDE_DESC
    "QEUEU_31",
    "Valid Credits",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD_MASK,
    0,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD_WIDTH,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_Q31_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_LB_CFG_LB_MODE
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_LB_CFG_LB_MODE_FIELD =
{
    "LB_MODE",
#if RU_INCLUDE_DESC
    "LOAD_BALANCING_MODE",
    "RoundRobin = 0"
    "StrictPriority = 1"
    "",
#endif
    DSPTCHR_LOAD_BALANCING_LB_CFG_LB_MODE_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_LB_CFG_LB_MODE_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_LB_CFG_LB_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED0_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_LB_CFG_SP_THRSHLD
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_LB_CFG_SP_THRSHLD_FIELD =
{
    "SP_THRSHLD",
#if RU_INCLUDE_DESC
    "SP_THRESHOLD",
    "Configures the threshold in which the LB mechanism opens activates a new RNR",
#endif
    DSPTCHR_LOAD_BALANCING_LB_CFG_SP_THRSHLD_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_LB_CFG_SP_THRSHLD_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_LB_CFG_SP_THRSHLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED1_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED1_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR0
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR0_FIELD =
{
    "RNR0",
#if RU_INCLUDE_DESC
    "RNR_0",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR0_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR0_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR1
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR1_FIELD =
{
    "RNR1",
#if RU_INCLUDE_DESC
    "RNR_1",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR1_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR1_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR2
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR2_FIELD =
{
    "RNR2",
#if RU_INCLUDE_DESC
    "RNR_2",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR2_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR2_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR3
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR3_FIELD =
{
    "RNR3",
#if RU_INCLUDE_DESC
    "RNR_3",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR3_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR3_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR4
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR4_FIELD =
{
    "RNR4",
#if RU_INCLUDE_DESC
    "RNR_4",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR4_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR4_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR5
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR5_FIELD =
{
    "RNR5",
#if RU_INCLUDE_DESC
    "RNR_5",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR5_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR5_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR6
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR6_FIELD =
{
    "RNR6",
#if RU_INCLUDE_DESC
    "RNR_6",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR6_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR6_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR7
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR7_FIELD =
{
    "RNR7",
#if RU_INCLUDE_DESC
    "RNR_7",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR7_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR7_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR8
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR8_FIELD =
{
    "RNR8",
#if RU_INCLUDE_DESC
    "RNR_8",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR8_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR8_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR9
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR9_FIELD =
{
    "RNR9",
#if RU_INCLUDE_DESC
    "RNR_9",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR9_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR9_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR10
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR10_FIELD =
{
    "RNR10",
#if RU_INCLUDE_DESC
    "RNR_10",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR10_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR10_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR11
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR11_FIELD =
{
    "RNR11",
#if RU_INCLUDE_DESC
    "RNR_11",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR11_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR11_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR12
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR12_FIELD =
{
    "RNR12",
#if RU_INCLUDE_DESC
    "RNR_12",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR12_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR12_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR13
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR13_FIELD =
{
    "RNR13",
#if RU_INCLUDE_DESC
    "RNR_13",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR13_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR13_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR14
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR14_FIELD =
{
    "RNR14",
#if RU_INCLUDE_DESC
    "RNR_14",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR14_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR14_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR15
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR15_FIELD =
{
    "RNR15",
#if RU_INCLUDE_DESC
    "RNR_15",
    "Each bit indicats which task is Free for dispatch",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR15_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR15_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK0
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK0_FIELD =
{
    "TSK0",
#if RU_INCLUDE_DESC
    "TSK0_TO_RG_MAP",
    "Can be Task 0/8/16...",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK0_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK0_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK1
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK1_FIELD =
{
    "TSK1",
#if RU_INCLUDE_DESC
    "TSK1_TO_RG_MAP",
    "Can be Task 1/9/17...",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK1_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK1_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK2
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK2_FIELD =
{
    "TSK2",
#if RU_INCLUDE_DESC
    "TSK2_TO_RG_MAP",
    "Can be Task 2/10/18...",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK2_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK2_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK3
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK3_FIELD =
{
    "TSK3",
#if RU_INCLUDE_DESC
    "TSK3_TO_RG_MAP",
    "Can be Task 3/11/19...",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK3_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK3_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK4
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK4_FIELD =
{
    "TSK4",
#if RU_INCLUDE_DESC
    "TSK4_TO_RG_MAP",
    "Can be Task 4/12/20...",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK4_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK4_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK5
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK5_FIELD =
{
    "TSK5",
#if RU_INCLUDE_DESC
    "TSK5_TO_RG_MAP",
    "Can be Task 5/13/21...",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK5_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK5_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK6
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK6_FIELD =
{
    "TSK6",
#if RU_INCLUDE_DESC
    "TSK6_TO_RG_MAP",
    "Can be Task 6/14/22...",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK6_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK6_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK7
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK7_FIELD =
{
    "TSK7",
#if RU_INCLUDE_DESC
    "TSK7_TO_RG_MAP",
    "Can be Task 7/15/23...",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK7_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK7_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_RESERVED0_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_0
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_0_FIELD =
{
    "TSK_CNT_RG_0",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RG0",
    "Counter the amount of available (free) tasks in a RNR Group",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_0_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_0_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_1
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_1_FIELD =
{
    "TSK_CNT_RG_1",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RG1",
    "Counter the amount of available (free) tasks in a RNR Group",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_1_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_1_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_2
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_2_FIELD =
{
    "TSK_CNT_RG_2",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RG2",
    "Counter the amount of available (free) tasks in a RNR Group",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_2_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_2_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_3
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_3_FIELD =
{
    "TSK_CNT_RG_3",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RG3",
    "Counter the amount of available (free) tasks in a RNR Group",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_3_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_3_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_4
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_4_FIELD =
{
    "TSK_CNT_RG_4",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RG4",
    "Counter the amount of available (free) tasks in a RNR Group",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_4_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_4_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_5
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_5_FIELD =
{
    "TSK_CNT_RG_5",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RG5",
    "Counter the amount of available (free) tasks in a RNR Group",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_5_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_5_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_6
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_6_FIELD =
{
    "TSK_CNT_RG_6",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RG6",
    "Counter the amount of available (free) tasks in a RNR Group",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_6_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_6_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_7
 ******************************************************************************/
const ru_field_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_7_FIELD =
{
    "TSK_CNT_RG_7",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RG7",
    "Counter the amount of available (free) tasks in a RNR Group",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_7_FIELD_MASK,
    0,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_7_FIELD_WIDTH,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD =
{
    "FLL_RETURN_BUF",
#if RU_INCLUDE_DESC
    "BUF_RETURNED_TO_FLL",
    "Buffer returned to Fll",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD =
{
    "FLL_CNT_DRP",
#if RU_INCLUDE_DESC
    "FLL_COUNTED_DROP",
    "Drop PD counted",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD =
{
    "UNKNWN_MSG",
#if RU_INCLUDE_DESC
    "UNKNOWN_MESSAGE",
    "Unknown message entered the dispatcher",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD =
{
    "FLL_OVERFLOW",
#if RU_INCLUDE_DESC
    "FLL_OVERFLOW",
    "Number of buffers returned to FLL exceeds the pre-defined allocated buffer amount (due to linked list bug)",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD =
{
    "FLL_NEG",
#if RU_INCLUDE_DESC
    "FLL_NEGATIVE_AMOUNT_OF_BUF",
    "Number of buffers returned to FLL decreased under zero and reached a negative amount (due to linked list bug)",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_RESERVED0_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "Interrupt_status_masked",
    "Status Masked of corresponding interrupt source in the ISR",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "Interrupt_enable_mask",
    "Each bit in the mask controls the corresponding interrupt source in the IER",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "Interrupt_simulation_test",
    "Each bit in the mask tests the corresponding interrupt source in the ISR",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD =
{
    "QDEST0_INT",
#if RU_INCLUDE_DESC
    "QDEST0_INT",
    "New Entry added to Destination queue 0",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST0_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD =
{
    "QDEST1_INT",
#if RU_INCLUDE_DESC
    "QDEST1_INT",
    "New Entry added to Destination queue 1",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST1_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD =
{
    "QDEST2_INT",
#if RU_INCLUDE_DESC
    "QDEST2_INT",
    "New Entry added to Destination queue 2",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST2_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD =
{
    "QDEST3_INT",
#if RU_INCLUDE_DESC
    "QDEST3_INT",
    "New Entry added to Destination queue 3",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST3_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD =
{
    "QDEST4_INT",
#if RU_INCLUDE_DESC
    "QDEST4_INT",
    "New Entry added to Destination queue 4",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST4_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD =
{
    "QDEST5_INT",
#if RU_INCLUDE_DESC
    "QDEST5_INT",
    "New Entry added to Destination queue 5",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST5_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD =
{
    "QDEST6_INT",
#if RU_INCLUDE_DESC
    "QDEST6_INT",
    "New Entry added to Destination queue 6",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST6_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD =
{
    "QDEST7_INT",
#if RU_INCLUDE_DESC
    "QDEST7_INT",
    "New Entry added to Destination queue 7",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST7_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD =
{
    "QDEST8_INT",
#if RU_INCLUDE_DESC
    "QDEST8_INT",
    "New Entry added to Destination queue 8",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST8_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD =
{
    "QDEST9_INT",
#if RU_INCLUDE_DESC
    "QDEST9_INT",
    "New Entry added to Destination queue 9",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST9_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD =
{
    "QDEST10_INT",
#if RU_INCLUDE_DESC
    "QDEST10_INT",
    "New Entry added to Destination queue 10",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST10_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD =
{
    "QDEST11_INT",
#if RU_INCLUDE_DESC
    "QDEST11_INT",
    "New Entry added to Destination queue 11",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST11_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD =
{
    "QDEST12_INT",
#if RU_INCLUDE_DESC
    "QDEST12_INT",
    "New Entry added to Destination queue 12",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST12_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD =
{
    "QDEST13_INT",
#if RU_INCLUDE_DESC
    "QDEST13_INT",
    "New Entry added to Destination queue 13",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST13_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD =
{
    "QDEST14_INT",
#if RU_INCLUDE_DESC
    "QDEST14_INT",
    "New Entry added to Destination queue 14",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST14_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD =
{
    "QDEST15_INT",
#if RU_INCLUDE_DESC
    "QDEST15_INT",
    "New Entry added to Destination queue 15",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST15_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD =
{
    "QDEST16_INT",
#if RU_INCLUDE_DESC
    "QDEST16_INT",
    "New Entry added to Destination queue 16",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST16_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD =
{
    "QDEST17_INT",
#if RU_INCLUDE_DESC
    "QDEST17_INT",
    "New Entry added to Destination queue 17",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST17_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD =
{
    "QDEST18_INT",
#if RU_INCLUDE_DESC
    "QDEST18_INT",
    "New Entry added to Destination queue 18",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST18_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD =
{
    "QDEST19_INT",
#if RU_INCLUDE_DESC
    "QDEST19_INT",
    "New Entry added to Destination queue 19",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST19_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD =
{
    "QDEST20_INT",
#if RU_INCLUDE_DESC
    "QDEST20_INT",
    "New Entry added to Destination queue 20",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST20_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD =
{
    "QDEST21_INT",
#if RU_INCLUDE_DESC
    "QDEST21_INT",
    "New Entry added to Destination queue 21",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST21_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD =
{
    "QDEST22_INT",
#if RU_INCLUDE_DESC
    "QDEST22_INT",
    "New Entry added to Destination queue 22",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST22_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD =
{
    "QDEST23_INT",
#if RU_INCLUDE_DESC
    "QDEST23_INT",
    "New Entry added to Destination queue 23",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST23_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD =
{
    "QDEST24_INT",
#if RU_INCLUDE_DESC
    "QDEST24_INT",
    "New Entry added to Destination queue 24",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST24_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD =
{
    "QDEST25_INT",
#if RU_INCLUDE_DESC
    "QDEST25_INT",
    "New Entry added to Destination queue 25",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST25_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD =
{
    "QDEST26_INT",
#if RU_INCLUDE_DESC
    "QDEST26_INT",
    "New Entry added to Destination queue 26",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST26_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD =
{
    "QDEST27_INT",
#if RU_INCLUDE_DESC
    "QDEST27_INT",
    "New Entry added to Destination queue 27",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST27_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD =
{
    "QDEST28_INT",
#if RU_INCLUDE_DESC
    "QDEST28_INT",
    "New Entry added to Destination queue 28",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST28_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD =
{
    "QDEST29_INT",
#if RU_INCLUDE_DESC
    "QDEST29_INT",
    "New Entry added to Destination queue 29",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST29_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD =
{
    "QDEST30_INT",
#if RU_INCLUDE_DESC
    "QDEST30_INT",
    "New Entry added to Destination queue 30",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST30_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD =
{
    "QDEST31_INT",
#if RU_INCLUDE_DESC
    "QDEST31_INT",
    "New Entry added to Destination queue 31",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_QDEST31_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "Interrupt_status_masked",
    "Status Masked of corresponding interrupt source in the ISR",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "Interrupt_enable_mask",
    "Each bit in the mask controls the corresponding interrupt source in the IER",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST
 ******************************************************************************/
const ru_field_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "Interrupt_simulation_test",
    "Each bit in the mask tests the corresponding interrupt source in the ISR",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD_MASK,
    0,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD_WIDTH,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD =
{
    "EN_BYP",
#if RU_INCLUDE_DESC
    "ENABLE",
    "Enable bypass mode",
#endif
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED0_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD =
{
    "BBID_NON_DLY",
#if RU_INCLUDE_DESC
    "BBID_NON_DELAY_Q",
    "What BBID to use for NON_DELAY Q when in Bypass mode",
#endif
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD =
{
    "BBID_DLY",
#if RU_INCLUDE_DESC
    "BBID_DELAY_Q",
    "What BBID to use for DELAY Q when in Bypass mode",
#endif
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED1_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED1_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_0
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_0_FIELD =
{
    "TSK_CNT_RNR_0",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_0",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_0_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_0_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_1
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_1_FIELD =
{
    "TSK_CNT_RNR_1",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_1",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_1_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_1_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_2
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_2_FIELD =
{
    "TSK_CNT_RNR_2",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_2",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_2_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_2_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_3
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_3_FIELD =
{
    "TSK_CNT_RNR_3",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_3",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_3_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_3_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_4
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_4_FIELD =
{
    "TSK_CNT_RNR_4",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_4",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_4_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_4_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_5
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_5_FIELD =
{
    "TSK_CNT_RNR_5",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_5",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_5_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_5_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_6
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_6_FIELD =
{
    "TSK_CNT_RNR_6",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_6",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_6_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_6_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_7
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_7_FIELD =
{
    "TSK_CNT_RNR_7",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_7",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_7_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_7_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_8
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_8_FIELD =
{
    "TSK_CNT_RNR_8",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_8",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_8_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_8_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_9
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_9_FIELD =
{
    "TSK_CNT_RNR_9",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_9",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_9_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_9_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_9_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_10
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_10_FIELD =
{
    "TSK_CNT_RNR_10",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_10",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_10_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_10_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_11
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_11_FIELD =
{
    "TSK_CNT_RNR_11",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_11",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_11_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_11_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_11_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_12
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_12_FIELD =
{
    "TSK_CNT_RNR_12",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_12",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_12_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_12_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_12_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_13
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_13_FIELD =
{
    "TSK_CNT_RNR_13",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_13",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_13_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_13_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_13_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_14
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_14_FIELD =
{
    "TSK_CNT_RNR_14",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_14",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_14_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_14_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_14_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_15
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_15_FIELD =
{
    "TSK_CNT_RNR_15",
#if RU_INCLUDE_DESC
    "TASK_COUNT_RNR_15",
    "Counter the amount of active tasks",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_15_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_15_FIELD_WIDTH,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_15_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD =
{
    "DBG_SEL",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_SELECT",
    "Selects with vector to output",
#endif
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_BUS_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_BUS_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_RESERVED0_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD =
{
    "DBG_MODE",
#if RU_INCLUDE_DESC
    "DEBUG_MODE",
    "Selects mode to log"
    "2b00 - RSRV"
    "2b11 - RSRV"
    ""
    "2b01 - DBG_CNT_PD_SENT_TO_RNR:"
    "--> CNTR 0-15 : counts PDs per RNR"
    "--> CNTR 16-31: Per config (DBG_RNR_SEL) counts per 16 tasks"
    ""
    "2b10 - DBG_CNT_TOTAL_PD_COUNT"
    "--> CNTR 16: incoming PD"
    "--> CNTR 17: dispatched PD"
    "--> CNTR 18: re-ordered PD"
    "--> CNTR 19: re-ordered PD that are not dropped"
    "--> CNTR 20: re-ordered PD that are dropped"
    "--> CNTR 21: PDs left in the Dispatcher"
    ""
    "",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD_WIDTH,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED0_FIELD_WIDTH,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD =
{
    "EN_CNTRS",
#if RU_INCLUDE_DESC
    "ENABLE",
    "Enable statistics",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD_WIDTH,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD =
{
    "CLR_CNTRS",
#if RU_INCLUDE_DESC
    "CLEAR_COUNTER",
    "Clears all counters",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD_WIDTH,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED1_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED1_FIELD_WIDTH,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD =
{
    "DBG_RNR_SEL",
#if RU_INCLUDE_DESC
    "DEBUG_RNR",
    "Selects RNR to log",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD_WIDTH,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED2_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED2_FIELD_WIDTH,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL
 ******************************************************************************/
const ru_field_rec DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD =
{
    "DBG_VEC_VAL",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR_VALUE",
    "Debug bus vector value",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD_MASK,
    0,
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD_WIDTH,
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_HEAD_HEAD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_HEAD_HEAD_FIELD =
{
    "HEAD",
#if RU_INCLUDE_DESC
    "HEAD",
    "Pointer to the first BD in the link list of this queue.",
#endif
    DSPTCHR_QDES_HEAD_HEAD_FIELD_MASK,
    0,
    DSPTCHR_QDES_HEAD_HEAD_FIELD_WIDTH,
    DSPTCHR_QDES_HEAD_HEAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_BFOUT_BFOUT
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_BFOUT_BFOUT_FIELD =
{
    "BFOUT",
#if RU_INCLUDE_DESC
    "BFOUT",
    "32 bit wrap around counter. Counts number of packets that left this queue since start of queue activity.",
#endif
    DSPTCHR_QDES_BFOUT_BFOUT_FIELD_MASK,
    0,
    DSPTCHR_QDES_BFOUT_BFOUT_FIELD_WIDTH,
    DSPTCHR_QDES_BFOUT_BFOUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_BUFIN_BUFIN
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_BUFIN_BUFIN_FIELD =
{
    "BUFIN",
#if RU_INCLUDE_DESC
    "BUFIN",
    "32 bit wrap around counter. Counts number of packets that entered this queue since start of queue activity.",
#endif
    DSPTCHR_QDES_BUFIN_BUFIN_FIELD_MASK,
    0,
    DSPTCHR_QDES_BUFIN_BUFIN_FIELD_WIDTH,
    DSPTCHR_QDES_BUFIN_BUFIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_TAIL_TAIL
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_TAIL_TAIL_FIELD =
{
    "TAIL",
#if RU_INCLUDE_DESC
    "TAIL",
    "Pointer to the last BD in the linked list of this queue.",
#endif
    DSPTCHR_QDES_TAIL_TAIL_FIELD_MASK,
    0,
    DSPTCHR_QDES_TAIL_TAIL_FIELD_WIDTH,
    DSPTCHR_QDES_TAIL_TAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_FBDNULL_FBDNULL
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD =
{
    "FBDNULL",
#if RU_INCLUDE_DESC
    "FBDNull",
    "If this bit is set then the first BD attached to this Q is a null BD. In this case, its Data Pointer field is not valid, but its Next BD pointer field is valid. When it is set, the NullBD field for this queue is not valid.",
#endif
    DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD_MASK,
    0,
    DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD_WIDTH,
    DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_FBDNULL_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_FBDNULL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_QDES_FBDNULL_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_QDES_FBDNULL_RESERVED0_FIELD_WIDTH,
    DSPTCHR_QDES_FBDNULL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_NULLBD_NULLBD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_NULLBD_NULLBD_FIELD =
{
    "NULLBD",
#if RU_INCLUDE_DESC
    "NullBD",
    "32 bits index of a Null BD that belongs to this queue. Both the data buffer pointer and the next BD field are non valid. The pointer defines a memory allocation for a BD that might be used or not.",
#endif
    DSPTCHR_QDES_NULLBD_NULLBD_FIELD_MASK,
    0,
    DSPTCHR_QDES_NULLBD_NULLBD_FIELD_WIDTH,
    DSPTCHR_QDES_NULLBD_NULLBD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_BUFAVAIL_BUFAVAIL
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD =
{
    "BUFAVAIL",
#if RU_INCLUDE_DESC
    "BUFAVAIL",
    "number of entries available in queue."
    "bufin - bfout",
#endif
    DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD_MASK,
    0,
    DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD_WIDTH,
    DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_REG_Q_HEAD_HEAD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD =
{
    "HEAD",
#if RU_INCLUDE_DESC
    "Q_HEAD",
    "Q HEAD",
#endif
    DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD_MASK,
    0,
    DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD_WIDTH,
    DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_REG_Q_HEAD_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_REG_Q_HEAD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_QDES_REG_Q_HEAD_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_QDES_REG_Q_HEAD_RESERVED0_FIELD_WIDTH,
    DSPTCHR_QDES_REG_Q_HEAD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD =
{
    "VIQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "VIQ_HEAD_VALID",
    "Q head valid. Each bit indicates for a specific VIQ if the head is valid or not",
#endif
    DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_MASK,
    0,
    DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_WIDTH,
    DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_CHRNCY_VLD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_CHRNCY_VLD_FIELD =
{
    "CHRNCY_VLD",
#if RU_INCLUDE_DESC
    "COHERENCY_VALID",
    "Q Coherency counter is valid. Each bit indicates for a specific VIQ if the there is more than one coherency message for that Q. meaning the head of the VIQ can be dispatched"
    "",
#endif
    DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_CHRNCY_VLD_FIELD_MASK,
    0,
    DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_CHRNCY_VLD_FIELD_WIDTH,
    DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_CHRNCY_VLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD =
{
    "VIQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "VIQ_HEAD_VALID",
    "Q head valid. Each bit indicates for a specific VIQ if the head is valid or not",
#endif
    DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_MASK,
    0,
    DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_WIDTH,
    DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD =
{
    "USE_BUF_AVL",
#if RU_INCLUDE_DESC
    "USE_BUFFER_AVAIL",
    "Should buf_avail in the QDES affect poping from head of linked list",
#endif
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD_MASK,
    0,
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD_WIDTH,
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD =
{
    "DEC_BUFOUT_WHEN_MLTCST",
#if RU_INCLUDE_DESC
    "DECREMENT_BUFOUT_WHEN_MULTICAST",
    "Should buf_avail in the QDES affect poping from head of linked list",
#endif
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD_MASK,
    0,
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD_WIDTH,
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_RESERVED0_FIELD_WIDTH,
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_FLLDES_HEAD_HEAD
 ******************************************************************************/
const ru_field_rec DSPTCHR_FLLDES_HEAD_HEAD_FIELD =
{
    "HEAD",
#if RU_INCLUDE_DESC
    "HEAD",
    "Pointer to the first BD in the link list of this queue.",
#endif
    DSPTCHR_FLLDES_HEAD_HEAD_FIELD_MASK,
    0,
    DSPTCHR_FLLDES_HEAD_HEAD_FIELD_WIDTH,
    DSPTCHR_FLLDES_HEAD_HEAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_FLLDES_BFOUT_COUNT
 ******************************************************************************/
const ru_field_rec DSPTCHR_FLLDES_BFOUT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "COUNT",
    "32 bit wrap around counter. Counts number of entries that left this queue since start of queue activity.",
#endif
    DSPTCHR_FLLDES_BFOUT_COUNT_FIELD_MASK,
    0,
    DSPTCHR_FLLDES_BFOUT_COUNT_FIELD_WIDTH,
    DSPTCHR_FLLDES_BFOUT_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_FLLDES_BFIN_BFIN
 ******************************************************************************/
const ru_field_rec DSPTCHR_FLLDES_BFIN_BFIN_FIELD =
{
    "BFIN",
#if RU_INCLUDE_DESC
    "BFIN",
    "32 bit wrap around counter. Counts number of entries that entered this queue since start of queue activity.",
#endif
    DSPTCHR_FLLDES_BFIN_BFIN_FIELD_MASK,
    0,
    DSPTCHR_FLLDES_BFIN_BFIN_FIELD_WIDTH,
    DSPTCHR_FLLDES_BFIN_BFIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_FLLDES_TAIL_TAIL
 ******************************************************************************/
const ru_field_rec DSPTCHR_FLLDES_TAIL_TAIL_FIELD =
{
    "TAIL",
#if RU_INCLUDE_DESC
    "TAIL",
    "Pointer to the last BD in the linked list of this queue.",
#endif
    DSPTCHR_FLLDES_TAIL_TAIL_FIELD_MASK,
    0,
    DSPTCHR_FLLDES_TAIL_TAIL_FIELD_WIDTH,
    DSPTCHR_FLLDES_TAIL_TAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_FLLDES_FLLDROP_DRPCNT
 ******************************************************************************/
const ru_field_rec DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD =
{
    "DRPCNT",
#if RU_INCLUDE_DESC
    "DRPCNT",
    "32 bit counter that counts the number of packets arrived when there is no free BD in the FLL.",
#endif
    DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD_MASK,
    0,
    DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD_WIDTH,
    DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_FLLDES_LTINT_MINBUF
 ******************************************************************************/
const ru_field_rec DSPTCHR_FLLDES_LTINT_MINBUF_FIELD =
{
    "MINBUF",
#if RU_INCLUDE_DESC
    "MINBUF",
    "Low threshold Interrupt. When number of bytes reach this level, then an interrupt is generated to the Host.",
#endif
    DSPTCHR_FLLDES_LTINT_MINBUF_FIELD_MASK,
    0,
    DSPTCHR_FLLDES_LTINT_MINBUF_FIELD_WIDTH,
    DSPTCHR_FLLDES_LTINT_MINBUF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL
 ******************************************************************************/
const ru_field_rec DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD =
{
    "BUFAVAIL",
#if RU_INCLUDE_DESC
    "BUFAVAIL",
    "number of entries available in queue."
    "bufin - bfout",
#endif
    DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD_MASK,
    0,
    DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD_WIDTH,
    DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_FLLDES_FREEMIN_FREEMIN
 ******************************************************************************/
const ru_field_rec DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD =
{
    "FREEMIN",
#if RU_INCLUDE_DESC
    "FREEMIN",
    "minum value of free BD recorded",
#endif
    DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD_MASK,
    0,
    DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD_WIDTH,
    DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DSPTCHR_BDRAM_DATA_RESERVED0
 ******************************************************************************/
const ru_field_rec DSPTCHR_BDRAM_DATA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_BDRAM_DATA_RESERVED0_FIELD_MASK,
    0,
    DSPTCHR_BDRAM_DATA_RESERVED0_FIELD_WIDTH,
    DSPTCHR_BDRAM_DATA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_BDRAM_DATA_DATA
 ******************************************************************************/
const ru_field_rec DSPTCHR_BDRAM_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "Data",
    "Data Buffer entry",
#endif
    DSPTCHR_BDRAM_DATA_DATA_FIELD_MASK,
    0,
    DSPTCHR_BDRAM_DATA_DATA_FIELD_WIDTH,
    DSPTCHR_BDRAM_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_BDRAM_DATA_RESERVED1
 ******************************************************************************/
const ru_field_rec DSPTCHR_BDRAM_DATA_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DSPTCHR_BDRAM_DATA_RESERVED1_FIELD_MASK,
    0,
    DSPTCHR_BDRAM_DATA_RESERVED1_FIELD_WIDTH,
    DSPTCHR_BDRAM_DATA_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DSPTCHR_PDRAM_DATA_DATA
 ******************************************************************************/
const ru_field_rec DSPTCHR_PDRAM_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "Data",
    "Data Buffer entry",
#endif
    DSPTCHR_PDRAM_DATA_DATA_FIELD_MASK,
    0,
    DSPTCHR_PDRAM_DATA_DATA_FIELD_WIDTH,
    DSPTCHR_PDRAM_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_EN_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED0_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RDY_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED1_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REORDR_PAR_MOD_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_PER_Q_EGRS_CONGST_EN_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_DSPTCHR_PER_ENH_POD_FIELD,
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG = 
{
    "REORDER_CFG_DSPTCHR_REORDR_CFG",
#if RU_INCLUDE_DESC
    "DISPATCHER_REORDER_EN Register",
    "Enable of dispatcher reorder",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG_OFFSET,
    0,
    0,
    466,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_REORDER_CFG_VQ_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_VQ_EN_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_VQ_EN_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_REORDER_CFG_VQ_EN_REG = 
{
    "REORDER_CFG_VQ_EN",
#if RU_INCLUDE_DESC
    "VIRTUAL_Q_EN Register",
    "Enable control for each VIQ/VEQ",
#endif
    DSPTCHR_REORDER_CFG_VQ_EN_REG_OFFSET,
    0,
    0,
    467,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_REORDER_CFG_VQ_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_REORDER_CFG_BB_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_BB_CFG_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_BB_CFG_SRC_ID_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_RESERVED0_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_DST_ID_OVRIDE_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_RESERVED1_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_ROUTE_OVRIDE_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_RESERVED2_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_OVRIDE_EN_FIELD,
    &DSPTCHR_REORDER_CFG_BB_CFG_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_REORDER_CFG_BB_CFG_REG = 
{
    "REORDER_CFG_BB_CFG",
#if RU_INCLUDE_DESC
    "BROADBUS_CONFIG Register",
    "Allow override of a specific BB destination with a new Route ADDR",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_REG_OFFSET,
    0,
    0,
    468,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DSPTCHR_REORDER_CFG_BB_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_FIELDS[] =
{
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG = 
{
    "REORDER_CFG_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    469,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_INGRS_CONGSTN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_INGRS_CONGSTN_FIELDS[] =
{
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_FRST_LVL_FIELD,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_SCND_LVL_FIELD,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_HYST_THRS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_REG = 
{
    "CONGESTION_INGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "INGRESS_CONGESTION_THRESHOLD %i Register",
    "Ingress Queues congestion state."
    "",
#endif
    DSPTCHR_CONGESTION_INGRS_CONGSTN_REG_OFFSET,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_REG_RAM_CNT,
    4,
    470,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_EGRS_CONGSTN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_EGRS_CONGSTN_FIELDS[] =
{
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_FRST_LVL_FIELD,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_SCND_LVL_FIELD,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_HYST_THRS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_REG = 
{
    "CONGESTION_EGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "EGRESS_CONGESTION_THRESHOLD %i Register",
    "Egress Queues congestion state per Q."
    "",
#endif
    DSPTCHR_CONGESTION_EGRS_CONGSTN_REG_OFFSET,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_REG_RAM_CNT,
    4,
    471,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FIELDS[] =
{
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FRST_LVL_FIELD,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_SCND_LVL_FIELD,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_HYST_THRS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG = 
{
    "CONGESTION_TOTAL_EGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "TOTAL_EGRESS_CONGESTION_THRESHOLD Register",
    "Egress congestion states (Total Count)",
#endif
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG_OFFSET,
    0,
    0,
    472,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_GLBL_CONGSTN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_GLBL_CONGSTN_FIELDS[] =
{
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_FRST_LVL_FIELD,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_SCND_LVL_FIELD,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_HYST_THRS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_REG = 
{
    "CONGESTION_GLBL_CONGSTN",
#if RU_INCLUDE_DESC
    "GLOBAL_CONGESTION_THRESHOLD Register",
    "Congestion levels of FLL state. Once no mode BDs are availabe congestion indication will be risen on all PDs.",
#endif
    DSPTCHR_CONGESTION_GLBL_CONGSTN_REG_OFFSET,
    0,
    0,
    473,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_CONGESTION_GLBL_CONGSTN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_CONGSTN_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_CONGSTN_STATUS_FIELDS[] =
{
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_CONGSTN_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED0_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_GLBL_EGRS_CONGSTN_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED1_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_SBPM_CONGSTN_FIELD,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_REG = 
{
    "CONGESTION_CONGSTN_STATUS",
#if RU_INCLUDE_DESC
    "CONGESTION_STATUS Register",
    "This register reflects the current congestion levels in the dispatcher.",
#endif
    DSPTCHR_CONGESTION_CONGSTN_STATUS_REG_OFFSET,
    0,
    0,
    474,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    DSPTCHR_CONGESTION_CONGSTN_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG = 
{
    "CONGESTION_PER_Q_INGRS_CONGSTN_LOW",
#if RU_INCLUDE_DESC
    "PER_Q_LOW_INGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage",
#endif
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG_OFFSET,
    0,
    0,
    475,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG = 
{
    "CONGESTION_PER_Q_INGRS_CONGSTN_HIGH",
#if RU_INCLUDE_DESC
    "PER_Q_HIGH_INGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage",
#endif
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG_OFFSET,
    0,
    0,
    476,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_CONGSTN_STATE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG = 
{
    "CONGESTION_PER_Q_EGRS_CONGSTN_LOW",
#if RU_INCLUDE_DESC
    "PER_Q_LOW_EGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage",
#endif
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG_OFFSET,
    0,
    0,
    477,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_FIELDS[] =
{
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_CONGSTN_STATE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG = 
{
    "CONGESTION_PER_Q_EGRS_CONGSTN_HIGH",
#if RU_INCLUDE_DESC
    "PER_Q_HIGH_EGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage",
#endif
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG_OFFSET,
    0,
    0,
    478,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_CMN_CNT_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG = 
{
    "INGRS_QUEUES_Q_INGRS_SIZE",
#if RU_INCLUDE_DESC
    "QUEUE_INGRS_SIZE %i Register",
    "Q Ingress size",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG_OFFSET,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG_RAM_CNT,
    4,
    479,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CMN_MAX_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_GURNTD_MAX_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_CREDIT_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG = 
{
    "INGRS_QUEUES_Q_INGRS_LIMITS",
#if RU_INCLUDE_DESC
    "QUEUE_INGRS_LIMITS %i Register",
    "Q Ingress Limits",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG_OFFSET,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG_RAM_CNT,
    4,
    480,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_FIELDS[] =
{
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_CNT_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_CHRNCY_EN_FIELD,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_RSRV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_REG = 
{
    "INGRS_QUEUES_Q_INGRS_COHRENCY",
#if RU_INCLUDE_DESC
    "QUEUE_INGRS_COHERENCY %i Register",
    "Q Coherency counter",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_REG_OFFSET,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_REG_RAM_CNT,
    4,
    481,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QUEUE_MAPPING_CRDT_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QUEUE_MAPPING_CRDT_CFG_FIELDS[] =
{
    &DSPTCHR_QUEUE_MAPPING_CRDT_CFG_BB_ID_FIELD,
    &DSPTCHR_QUEUE_MAPPING_CRDT_CFG_RESERVED0_FIELD,
    &DSPTCHR_QUEUE_MAPPING_CRDT_CFG_TRGT_ADD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG = 
{
    "QUEUE_MAPPING_CRDT_CFG",
#if RU_INCLUDE_DESC
    "CREDIT_CONFIGURATION %i Register",
    "Configuration for each Q including BB_ID, Target address, valid",
#endif
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG_OFFSET,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG_RAM_CNT,
    4,
    482,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_FIELDS[] =
{
    &DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_BASE_ADD_FIELD,
    &DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_OFFSET_ADD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_REG = 
{
    "QUEUE_MAPPING_PD_DSPTCH_ADD",
#if RU_INCLUDE_DESC
    "DISPATCH_ADDRESS %i Register",
    "Dispatched address will be calculated"
    "ADD= BASE_ADD + (TASK_NUM x OFFSET)",
#endif
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_REG_OFFSET,
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_REG_RAM_CNT,
    4,
    483,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QUEUE_MAPPING_Q_DEST
 ******************************************************************************/
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
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q9_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q10_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q11_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q12_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q13_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q14_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q15_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q16_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q17_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q18_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q19_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q20_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q21_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q22_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q23_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q24_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q25_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q26_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q27_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q28_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q29_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q30_FIELD,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_Q31_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_REG = 
{
    "QUEUE_MAPPING_Q_DEST",
#if RU_INCLUDE_DESC
    "Q_DESTINATION Register",
    "What is the destination of each VIQ. to Dispatcher and from there to Processing RNR or Reorder and from there to the QM",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_REG_OFFSET,
    0,
    0,
    484,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    DSPTCHR_QUEUE_MAPPING_Q_DEST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_CMN_POOL_LMT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_CMN_POOL_LMT_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_CMN_POOL_LMT_POOL_LMT_FIELD,
    &DSPTCHR_POOL_SIZES_CMN_POOL_LMT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_CMN_POOL_LMT_REG = 
{
    "POOL_SIZES_CMN_POOL_LMT",
#if RU_INCLUDE_DESC
    "COMMON_POOL_LIMIT Register",
    "common pool max size",
#endif
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_REG_OFFSET,
    0,
    0,
    485,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_CMN_POOL_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_POOL_SIZE_FIELD,
    &DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_REG = 
{
    "POOL_SIZES_CMN_POOL_SIZE",
#if RU_INCLUDE_DESC
    "COMMON_POOL_SIZE Register",
    "common pool size",
#endif
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_REG_OFFSET,
    0,
    0,
    486,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_POOL_LMT_FIELD,
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_REG = 
{
    "POOL_SIZES_GRNTED_POOL_LMT",
#if RU_INCLUDE_DESC
    "GUARANTEED_POOL_LIMIT Register",
    "Guaranteed pool max size",
#endif
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_REG_OFFSET,
    0,
    0,
    487,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_POOL_SIZE_FIELD,
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_REG = 
{
    "POOL_SIZES_GRNTED_POOL_SIZE",
#if RU_INCLUDE_DESC
    "GUARANTEED_POOL_SIZE Register",
    "Guaranteed pool size",
#endif
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_REG_OFFSET,
    0,
    0,
    488,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_POOL_LMT_FIELD,
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_REG = 
{
    "POOL_SIZES_MULTI_CST_POOL_LMT",
#if RU_INCLUDE_DESC
    "MULTI_CAST_POOL_LIMIT Register",
    "Multi Cast pool max size",
#endif
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_REG_OFFSET,
    0,
    0,
    489,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_POOL_SIZE_FIELD,
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_REG = 
{
    "POOL_SIZES_MULTI_CST_POOL_SIZE",
#if RU_INCLUDE_DESC
    "MULTI_CAST_POOL_SIZE Register",
    "Multi Cast pool size",
#endif
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_REG_OFFSET,
    0,
    0,
    490,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_RNR_POOL_LMT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_RNR_POOL_LMT_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_RNR_POOL_LMT_POOL_LMT_FIELD,
    &DSPTCHR_POOL_SIZES_RNR_POOL_LMT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_RNR_POOL_LMT_REG = 
{
    "POOL_SIZES_RNR_POOL_LMT",
#if RU_INCLUDE_DESC
    "RNR_POOL_LIMIT Register",
    "This counter counts the amount of buffers taken by runner for MultiCast purposes (or any other the requires adding new PDs to a Virtual Egress Queue - VEQ",
#endif
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_REG_OFFSET,
    0,
    0,
    491,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_RNR_POOL_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_POOL_SIZE_FIELD,
    &DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_REG = 
{
    "POOL_SIZES_RNR_POOL_SIZE",
#if RU_INCLUDE_DESC
    "RNR_POOL_SIZE Register",
    "This counter counts the amount of buffers taken by runner for MultiCast purposes (or any other the requires adding new PDs to a Virtual Egress Qeueu - VEQ)",
#endif
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_REG_OFFSET,
    0,
    0,
    492,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_FIELDS[] =
{
    &DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_POOL_SIZE_FIELD,
    &DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_REG = 
{
    "POOL_SIZES_PRCSSING_POOL_SIZE",
#if RU_INCLUDE_DESC
    "PROCESSING_POOL_SIZE Register",
    "This counter counts how many buffers are currenly being handled by all RNRs",
#endif
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_REG_OFFSET,
    0,
    0,
    493,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_MASK_MSK_TSK_255_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_MASK_MSK_TSK_255_0_FIELDS[] =
{
    &DSPTCHR_MASK_MSK_TSK_255_0_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_MASK_MSK_TSK_255_0_REG = 
{
    "MASK_MSK_TSK_255_0",
#if RU_INCLUDE_DESC
    "TASK_MASK %i Register",
    "Address 0 ->  255:224"
    "Address 4 ->  223:192"
    "Address 8 ->  191:160"
    "Address C ->  159:128"
    "Address 10 ->  127:96"
    "Address 14 ->   95:64"
    "Address 18 ->   63:32"
    "Address 1C ->   31: 0"
    ""
    ""
    "8 RG x 8 Regs per RG = 64 registers",
#endif
    DSPTCHR_MASK_MSK_TSK_255_0_REG_OFFSET,
    DSPTCHR_MASK_MSK_TSK_255_0_REG_RAM_CNT,
    4,
    494,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_MASK_MSK_TSK_255_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_MASK_MSK_Q
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_MASK_MSK_Q_FIELDS[] =
{
    &DSPTCHR_MASK_MSK_Q_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_MASK_MSK_Q_REG = 
{
    "MASK_MSK_Q",
#if RU_INCLUDE_DESC
    "QUEUE_MASK %i Register",
    "Queue Mask",
#endif
    DSPTCHR_MASK_MSK_Q_REG_OFFSET,
    DSPTCHR_MASK_MSK_Q_REG_RAM_CNT,
    4,
    495,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_MASK_MSK_Q_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_MASK_DLY_Q
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_MASK_DLY_Q_FIELDS[] =
{
    &DSPTCHR_MASK_DLY_Q_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_MASK_DLY_Q_REG = 
{
    "MASK_DLY_Q",
#if RU_INCLUDE_DESC
    "DELAY_Q Register",
    "Describes which VEQ are part of the Delay Q group.",
#endif
    DSPTCHR_MASK_DLY_Q_REG_OFFSET,
    0,
    0,
    496,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_MASK_DLY_Q_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_MASK_NON_DLY_Q
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_MASK_NON_DLY_Q_FIELDS[] =
{
    &DSPTCHR_MASK_NON_DLY_Q_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_MASK_NON_DLY_Q_REG = 
{
    "MASK_NON_DLY_Q",
#if RU_INCLUDE_DESC
    "NON_DELAY_Q Register",
    "Describes which VEQ are part of the Non-Delay Q group.",
#endif
    DSPTCHR_MASK_NON_DLY_Q_REG_OFFSET,
    0,
    0,
    497,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_MASK_NON_DLY_Q_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_FIELDS[] =
{
    &DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_DLY_CRDT_FIELD,
    &DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG = 
{
    "EGRS_QUEUES_EGRS_DLY_QM_CRDT",
#if RU_INCLUDE_DESC
    "EGRESS_QM_DELAY_CREDIT Register",
    "These registers hold the available credit for the Re-Order to sent PDs to the QM via Delay Q."
    ""
    "",
#endif
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG_OFFSET,
    0,
    0,
    498,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_FIELDS[] =
{
    &DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_NON_DLY_CRDT_FIELD,
    &DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG = 
{
    "EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT",
#if RU_INCLUDE_DESC
    "EGRESS_QM_NON_DELAY_CREDIT Register",
    "These registers hold the available credit for the Re-Order to sent PDs to the QM via Non-Delay Q."
    ""
    "",
#endif
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG_OFFSET,
    0,
    0,
    499,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_FIELDS[] =
{
    &DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_TOTAL_EGRS_SIZE_FIELD,
    &DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG = 
{
    "EGRS_QUEUES_TOTAL_Q_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "TOTAL_EGRESS_SIZE Register",
    "Size of all egress queues. affected from PDs sent to dispatch and from multicast connect"
    "",
#endif
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG_OFFSET,
    0,
    0,
    500,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_FIELDS[] =
{
    &DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_Q_EGRS_SIZE_FIELD,
    &DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG = 
{
    "EGRS_QUEUES_PER_Q_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "Q_EGRESS_SIZE %i Register",
    "Size of all egress queues. affected from PDs sent to dispatch and from multicast connect"
    "",
#endif
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG_OFFSET,
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG_RAM_CNT,
    4,
    501,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ
 ******************************************************************************/
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
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q9_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q10_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q11_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q12_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q13_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q14_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q15_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q16_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q17_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q18_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q19_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q20_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q21_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q22_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q23_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q24_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q25_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q26_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q27_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q28_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q29_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q30_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_Q31_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG = 
{
    "WAKEUP_CONTROL_WKUP_REQ",
#if RU_INCLUDE_DESC
    "WAKEUP_REQUEST Register",
    "Bit per queue, wakeup request from RNR to a specific Q. Once a wakeup request message is sent to dsptchr it will be latched until the amount of credits pass a threshold",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG_OFFSET,
    0,
    0,
    502,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_FIELDS[] =
{
    &DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_WKUP_THRSHLD_FIELD,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG = 
{
    "WAKEUP_CONTROL_WKUP_THRSHLD",
#if RU_INCLUDE_DESC
    "WAKEUP_THRESHOLD Register",
    "Wakeup Thresholds in which to indicate RNR",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG_OFFSET,
    0,
    0,
    503,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_FIELDS[] =
{
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_Q_CRDT_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_NGTV_FIELD,
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_QUNTUM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG = 
{
    "DISPTCH_SCHEDULING_DWRR_INFO",
#if RU_INCLUDE_DESC
    "SCHEDULING_Q_INFO %i Register",
    "DWRR info per Q. including amount of credits per Q. If Q has below zero credits and Quantum size",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG_OFFSET,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG_RAM_CNT,
    4,
    504,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT
 ******************************************************************************/
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

const ru_reg_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG = 
{
    "DISPTCH_SCHEDULING_VLD_CRDT",
#if RU_INCLUDE_DESC
    "VALID_QUEUES Register",
    "Queues with credits above zero. This will allow for the Q to participate in the scheduling round",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG_OFFSET,
    0,
    0,
    505,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_LB_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_LB_CFG_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_LB_CFG_LB_MODE_FIELD,
    &DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED0_FIELD,
    &DSPTCHR_LOAD_BALANCING_LB_CFG_SP_THRSHLD_FIELD,
    &DSPTCHR_LOAD_BALANCING_LB_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_LB_CFG_REG = 
{
    "LOAD_BALANCING_LB_CFG",
#if RU_INCLUDE_DESC
    "LB_CONFIG Register",
    "Selects which Load Balancing mechanism to use",
#endif
    DSPTCHR_LOAD_BALANCING_LB_CFG_REG_OFFSET,
    0,
    0,
    506,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DSPTCHR_LOAD_BALANCING_LB_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR0_FIELD,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_RNR1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_REG = 
{
    "LOAD_BALANCING_FREE_TASK_0_1",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_0_1 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 0"
    "Tasks 16..32 Belong to RNR 1",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_REG_OFFSET,
    0,
    0,
    507,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR2_FIELD,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_RNR3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_REG = 
{
    "LOAD_BALANCING_FREE_TASK_2_3",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_2_3 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 2"
    "Tasks 16..32 Belong to RNR 3",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_REG_OFFSET,
    0,
    0,
    508,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR4_FIELD,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_RNR5_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_REG = 
{
    "LOAD_BALANCING_FREE_TASK_4_5",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_4_5 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 4"
    "Tasks 16..32 Belong to RNR 5",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_REG_OFFSET,
    0,
    0,
    509,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR6_FIELD,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_RNR7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_REG = 
{
    "LOAD_BALANCING_FREE_TASK_6_7",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_6_7 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 6"
    "Tasks 16..32 Belong to RNR 7",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_REG_OFFSET,
    0,
    0,
    510,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR8_FIELD,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_RNR9_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_REG = 
{
    "LOAD_BALANCING_FREE_TASK_8_9",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_8_9 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 8"
    "Tasks 16..32 Belong to RNR 9",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_REG_OFFSET,
    0,
    0,
    511,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR10_FIELD,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_RNR11_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_REG = 
{
    "LOAD_BALANCING_FREE_TASK_10_11",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_10_11 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 10"
    "Tasks 16..32 Belong to RNR 11",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_REG_OFFSET,
    0,
    0,
    512,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR12_FIELD,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_RNR13_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_REG = 
{
    "LOAD_BALANCING_FREE_TASK_12_13",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_12_13 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 12"
    "Tasks 16..32 Belong to RNR 13",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_REG_OFFSET,
    0,
    0,
    513,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR14_FIELD,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_RNR15_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_REG = 
{
    "LOAD_BALANCING_FREE_TASK_14_15",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_14_15 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 14"
    "Tasks 16..32 Belong to RNR 15",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_REG_OFFSET,
    0,
    0,
    514,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK0_FIELD,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK1_FIELD,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK2_FIELD,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK3_FIELD,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK4_FIELD,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK5_FIELD,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK6_FIELD,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_TSK7_FIELD,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_REG = 
{
    "LOAD_BALANCING_TSK_TO_RG_MAPPING",
#if RU_INCLUDE_DESC
    "TASK_TO_RG_MAPPING %i Register",
    "This ram is used to map each task to which group does it belong to.",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_REG_OFFSET,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_REG_RAM_CNT,
    4,
    515,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_0_FIELD,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_1_FIELD,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_2_FIELD,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_TSK_CNT_RG_3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_REG = 
{
    "LOAD_BALANCING_RG_AVLABL_TSK_0_3",
#if RU_INCLUDE_DESC
    "RG_AVAILABLE_TASK_0_3 Register",
    "Available tasks in all runners related to a RNR Group. In case value is zero there are no tasks available for this RNR Group for dispatch hence it should be excluded from the next RNR Group selection",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_REG_OFFSET,
    0,
    0,
    516,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_FIELDS[] =
{
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_4_FIELD,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_5_FIELD,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_6_FIELD,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_TSK_CNT_RG_7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_REG = 
{
    "LOAD_BALANCING_RG_AVLABL_TSK_4_7",
#if RU_INCLUDE_DESC
    "RG_AVAILABLE_TASK_4_7 Register",
    "Available tasks in all runners related to a RNR Group. In case value is zero there are no tasks available for this RNR Group for dispatch hence it should be excluded from the next RNR Group selection",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_REG_OFFSET,
    0,
    0,
    517,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_RETURN_BUF_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_CNT_DRP_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_UNKNWN_MSG_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_OVERFLOW_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FLL_NEG_FIELD,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG_OFFSET,
    0,
    0,
    518,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_ISM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG_OFFSET,
    0,
    0,
    519,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_IEM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG_OFFSET,
    0,
    0,
    520,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_IST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG_OFFSET,
    0,
    0,
    521,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR
 ******************************************************************************/
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

const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG_OFFSET,
    0,
    0,
    522,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    32,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_ISM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG_OFFSET,
    0,
    0,
    523,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_IEM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG_OFFSET,
    0,
    0,
    524,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_FIELDS[] =
{
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_IST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG_OFFSET,
    0,
    0,
    525,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_EN_BYP_FIELD,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED0_FIELD,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_NON_DLY_FIELD,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_BBID_DLY_FIELD,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG = 
{
    "DEBUG_DBG_BYPSS_CNTRL",
#if RU_INCLUDE_DESC
    "DEBUG_BYPASS_CONTROL Register",
    "Debug Bypass control",
#endif
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG_OFFSET,
    0,
    0,
    526,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_FIELDS[] =
{
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_0_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_1_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_2_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_3_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_4_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_5_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_6_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_TSK_CNT_RNR_7_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_REG = 
{
    "DEBUG_GLBL_TSK_CNT_0_7",
#if RU_INCLUDE_DESC
    "TASK_COUNTER_0_7 Register",
    "Counts the amount of active Tasks in RNR",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_REG_OFFSET,
    0,
    0,
    527,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_FIELDS[] =
{
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_8_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_9_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_10_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_11_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_12_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_13_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_14_FIELD,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_TSK_CNT_RNR_15_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_REG = 
{
    "DEBUG_GLBL_TSK_CNT_8_15",
#if RU_INCLUDE_DESC
    "TASK_COUNTER_8_15 Register",
    "Counts the amount of active Tasks in RNR",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_REG_OFFSET,
    0,
    0,
    528,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_BUS_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_BUS_CNTRL_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_BUS_CNTRL_DBG_SEL_FIELD,
    &DSPTCHR_DEBUG_DBG_BUS_CNTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_BUS_CNTRL_REG = 
{
    "DEBUG_DBG_BUS_CNTRL",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_CONTROL Register",
    "Debug bus control which vector to output to the top level",
#endif
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_REG_OFFSET,
    0,
    0,
    529,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_0_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_0_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_0_REG = 
{
    "DEBUG_DBG_VEC_0",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_0 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_0_REG_OFFSET,
    0,
    0,
    530,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_1_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_1_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_1_REG = 
{
    "DEBUG_DBG_VEC_1",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_1 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_1_REG_OFFSET,
    0,
    0,
    531,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_2_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_2_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_2_REG = 
{
    "DEBUG_DBG_VEC_2",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_2 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_2_REG_OFFSET,
    0,
    0,
    532,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_3_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_3_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_3_REG = 
{
    "DEBUG_DBG_VEC_3",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_3 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_3_REG_OFFSET,
    0,
    0,
    533,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_4_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_4_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_4_REG = 
{
    "DEBUG_DBG_VEC_4",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_4 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_4_REG_OFFSET,
    0,
    0,
    534,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_4_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_5_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_5_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_5_REG = 
{
    "DEBUG_DBG_VEC_5",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_5 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_5_REG_OFFSET,
    0,
    0,
    535,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_5_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_6
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_6_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_6_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_6_REG = 
{
    "DEBUG_DBG_VEC_6",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_6 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_6_REG_OFFSET,
    0,
    0,
    536,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_6_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_7_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_7_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_7_REG = 
{
    "DEBUG_DBG_VEC_7",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_7 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_7_REG_OFFSET,
    0,
    0,
    537,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_7_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_8
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_8_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_8_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_8_REG = 
{
    "DEBUG_DBG_VEC_8",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_8 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_8_REG_OFFSET,
    0,
    0,
    538,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_8_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_9
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_9_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_9_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_9_REG = 
{
    "DEBUG_DBG_VEC_9",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_9 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_9_REG_OFFSET,
    0,
    0,
    539,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_9_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_10
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_10_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_10_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_10_REG = 
{
    "DEBUG_DBG_VEC_10",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_10 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_10_REG_OFFSET,
    0,
    0,
    540,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_10_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_11
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_11_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_11_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_11_REG = 
{
    "DEBUG_DBG_VEC_11",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_11 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_11_REG_OFFSET,
    0,
    0,
    541,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_11_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_12
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_12_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_12_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_12_REG = 
{
    "DEBUG_DBG_VEC_12",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_12 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_12_REG_OFFSET,
    0,
    0,
    542,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_12_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_13
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_13_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_13_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_13_REG = 
{
    "DEBUG_DBG_VEC_13",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_13 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_13_REG_OFFSET,
    0,
    0,
    543,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_13_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_14
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_14_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_14_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_14_REG = 
{
    "DEBUG_DBG_VEC_14",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_14 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_14_REG_OFFSET,
    0,
    0,
    544,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_14_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_15
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_15_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_15_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_15_REG = 
{
    "DEBUG_DBG_VEC_15",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_15 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_15_REG_OFFSET,
    0,
    0,
    545,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_15_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_16
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_16_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_16_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_16_REG = 
{
    "DEBUG_DBG_VEC_16",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_16 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_16_REG_OFFSET,
    0,
    0,
    546,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_16_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_17
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_17_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_17_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_17_REG = 
{
    "DEBUG_DBG_VEC_17",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_17 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_17_REG_OFFSET,
    0,
    0,
    547,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_17_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_18
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_18_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_18_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_18_REG = 
{
    "DEBUG_DBG_VEC_18",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_18 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_18_REG_OFFSET,
    0,
    0,
    548,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_18_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_19
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_19_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_19_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_19_REG = 
{
    "DEBUG_DBG_VEC_19",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_19 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_19_REG_OFFSET,
    0,
    0,
    549,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_19_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_20
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_20_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_20_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_20_REG = 
{
    "DEBUG_DBG_VEC_20",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_20 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_20_REG_OFFSET,
    0,
    0,
    550,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_20_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_21
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_21_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_21_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_21_REG = 
{
    "DEBUG_DBG_VEC_21",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_21 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_21_REG_OFFSET,
    0,
    0,
    551,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_21_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_22
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_22_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_22_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_22_REG = 
{
    "DEBUG_DBG_VEC_22",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_22 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_22_REG_OFFSET,
    0,
    0,
    552,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_22_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_23
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_DBG_VEC_23_FIELDS[] =
{
    &DSPTCHR_DEBUG_DBG_VEC_23_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_23_REG = 
{
    "DEBUG_DBG_VEC_23",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_23 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_23_REG_OFFSET,
    0,
    0,
    553,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_DBG_VEC_23_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_FIELDS[] =
{
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_MODE_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED0_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_EN_CNTRS_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_CLR_CNTRS_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED1_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_DBG_RNR_SEL_FIELD,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_REG = 
{
    "DEBUG_STATISTICS_DBG_STTSTCS_CTRL",
#if RU_INCLUDE_DESC
    "DEBUG_STATISTICS_CONTROL Register",
    "Controls which information to log",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_REG_OFFSET,
    0,
    0,
    554,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_STATISTICS_DBG_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_DEBUG_STATISTICS_DBG_CNT_FIELDS[] =
{
    &DSPTCHR_DEBUG_STATISTICS_DBG_CNT_DBG_VEC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG = 
{
    "DEBUG_STATISTICS_DBG_CNT",
#if RU_INCLUDE_DESC
    "DEBUG_COUNT %i Register",
    "Debug counter",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG_OFFSET,
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG_RAM_CNT,
    4,
    555,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_HEAD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_HEAD_FIELDS[] =
{
    &DSPTCHR_QDES_HEAD_HEAD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_HEAD_REG = 
{
    "QDES_HEAD",
#if RU_INCLUDE_DESC
    "HEAD Register",
    "Pointer to the first BD in the link list of this queue.",
#endif
    DSPTCHR_QDES_HEAD_REG_OFFSET,
    DSPTCHR_QDES_HEAD_REG_RAM_CNT,
    32,
    556,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_HEAD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_BFOUT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_BFOUT_FIELDS[] =
{
    &DSPTCHR_QDES_BFOUT_BFOUT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_BFOUT_REG = 
{
    "QDES_BFOUT",
#if RU_INCLUDE_DESC
    "BFOUT Register",
    "32 bit wrap around counter. Counts number of packets that left this queue since start of queue activity.",
#endif
    DSPTCHR_QDES_BFOUT_REG_OFFSET,
    DSPTCHR_QDES_BFOUT_REG_RAM_CNT,
    32,
    557,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_BFOUT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_BUFIN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_BUFIN_FIELDS[] =
{
    &DSPTCHR_QDES_BUFIN_BUFIN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_BUFIN_REG = 
{
    "QDES_BUFIN",
#if RU_INCLUDE_DESC
    "BUFIN Register",
    "32 bit wrap around counter. Counts number of packets that entered this queue since start of queue activity.",
#endif
    DSPTCHR_QDES_BUFIN_REG_OFFSET,
    DSPTCHR_QDES_BUFIN_REG_RAM_CNT,
    32,
    558,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_BUFIN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_TAIL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_TAIL_FIELDS[] =
{
    &DSPTCHR_QDES_TAIL_TAIL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_TAIL_REG = 
{
    "QDES_TAIL",
#if RU_INCLUDE_DESC
    "TAIL Register",
    "Pointer to the last BD in the linked list of this queue.",
#endif
    DSPTCHR_QDES_TAIL_REG_OFFSET,
    DSPTCHR_QDES_TAIL_REG_RAM_CNT,
    32,
    559,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_TAIL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_FBDNULL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_FBDNULL_FIELDS[] =
{
    &DSPTCHR_QDES_FBDNULL_FBDNULL_FIELD,
    &DSPTCHR_QDES_FBDNULL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_FBDNULL_REG = 
{
    "QDES_FBDNULL",
#if RU_INCLUDE_DESC
    "FBDNULL Register",
    "If this bit is set then the first BD attached to this Q is a null BD. In this case, its Data Pointer field is not valid, but its Next BD pointer field is valid. When it is set, the NullBD field for this queue is not valid.",
#endif
    DSPTCHR_QDES_FBDNULL_REG_OFFSET,
    DSPTCHR_QDES_FBDNULL_REG_RAM_CNT,
    32,
    560,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_QDES_FBDNULL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_NULLBD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_NULLBD_FIELDS[] =
{
    &DSPTCHR_QDES_NULLBD_NULLBD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_NULLBD_REG = 
{
    "QDES_NULLBD",
#if RU_INCLUDE_DESC
    "NULLBD Register",
    "32 bits index of a Null BD that belongs to this queue. Both the data buffer pointer and the next BD field are non valid. The pointer defines a memory allocation for a BD that might be used or not.",
#endif
    DSPTCHR_QDES_NULLBD_REG_OFFSET,
    DSPTCHR_QDES_NULLBD_REG_RAM_CNT,
    32,
    561,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_NULLBD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_BUFAVAIL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_BUFAVAIL_FIELDS[] =
{
    &DSPTCHR_QDES_BUFAVAIL_BUFAVAIL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_BUFAVAIL_REG = 
{
    "QDES_BUFAVAIL",
#if RU_INCLUDE_DESC
    "BUFAVAIL Register",
    "number of entries available in queue."
    "bufin - bfout",
#endif
    DSPTCHR_QDES_BUFAVAIL_REG_OFFSET,
    DSPTCHR_QDES_BUFAVAIL_REG_RAM_CNT,
    32,
    562,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_BUFAVAIL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_Q_HEAD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_Q_HEAD_FIELDS[] =
{
    &DSPTCHR_QDES_REG_Q_HEAD_HEAD_FIELD,
    &DSPTCHR_QDES_REG_Q_HEAD_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_REG_Q_HEAD_REG = 
{
    "QDES_REG_Q_HEAD",
#if RU_INCLUDE_DESC
    "QUEUE_HEAD %i Register",
    "Q Head Buffer, Used for the dispatching logic",
#endif
    DSPTCHR_QDES_REG_Q_HEAD_REG_OFFSET,
    DSPTCHR_QDES_REG_Q_HEAD_REG_RAM_CNT,
    4,
    563,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DSPTCHR_QDES_REG_Q_HEAD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_VIQ_HEAD_VLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_VIQ_HEAD_VLD_FIELDS[] =
{
    &DSPTCHR_QDES_REG_VIQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_REG_VIQ_HEAD_VLD_REG = 
{
    "QDES_REG_VIQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "VIQ_HEAD_VALID Register",
    "This register will hold the for each VIQ if the Head of the Q is valid or not."
    "These Queues are for Dispatch"
    "",
#endif
    DSPTCHR_QDES_REG_VIQ_HEAD_VLD_REG_OFFSET,
    0,
    0,
    564,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_REG_VIQ_HEAD_VLD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_FIELDS[] =
{
    &DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_CHRNCY_VLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_REG = 
{
    "QDES_REG_VIQ_CHRNCY_VLD",
#if RU_INCLUDE_DESC
    "VIQ_COHERENCY_VALID Register",
    "This register will hold for each VIQ if the Coherency counter is larger than zero."
    "",
#endif
    DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_REG_OFFSET,
    0,
    0,
    565,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_VEQ_HEAD_VLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_VEQ_HEAD_VLD_FIELDS[] =
{
    &DSPTCHR_QDES_REG_VEQ_HEAD_VLD_VIQ_HEAD_VLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_REG_VEQ_HEAD_VLD_REG = 
{
    "QDES_REG_VEQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "VEQ_HEAD_VALID Register",
    "This register will hold the for each VEQ if the Head of the Q is valid or not"
    "These Queues are for ReOrder",
#endif
    DSPTCHR_QDES_REG_VEQ_HEAD_VLD_REG_OFFSET,
    0,
    0,
    566,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_QDES_REG_VEQ_HEAD_VLD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_FIELDS[] =
{
    &DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_USE_BUF_AVL_FIELD,
    &DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_DEC_BUFOUT_WHEN_MLTCST_FIELD,
    &DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_REG = 
{
    "QDES_REG_QDES_BUF_AVL_CNTRL",
#if RU_INCLUDE_DESC
    "QDES_BUF_AVAIL_CONTROL Register",
    "Todays implementation does not require that QDES available buffer be different than zero. so this register controls whether or not to it should affect poping from the QDES or not",
#endif
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_REG_OFFSET,
    0,
    0,
    567,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_HEAD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_HEAD_FIELDS[] =
{
    &DSPTCHR_FLLDES_HEAD_HEAD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_FLLDES_HEAD_REG = 
{
    "FLLDES_HEAD",
#if RU_INCLUDE_DESC
    "HEAD Register",
    "Pointer to the first BD in the link list of this queue.",
#endif
    DSPTCHR_FLLDES_HEAD_REG_OFFSET,
    0,
    0,
    568,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_HEAD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_BFOUT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_BFOUT_FIELDS[] =
{
    &DSPTCHR_FLLDES_BFOUT_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_FLLDES_BFOUT_REG = 
{
    "FLLDES_BFOUT",
#if RU_INCLUDE_DESC
    "BFOUT Register",
    "32 bit wrap around counter. Counts number of entries that left this queue since start of queue activity.",
#endif
    DSPTCHR_FLLDES_BFOUT_REG_OFFSET,
    0,
    0,
    569,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_BFOUT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_BFIN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_BFIN_FIELDS[] =
{
    &DSPTCHR_FLLDES_BFIN_BFIN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_FLLDES_BFIN_REG = 
{
    "FLLDES_BFIN",
#if RU_INCLUDE_DESC
    "BFIN Register",
    "32 bit wrap around counter. Counts number of entries that entered this queue since start of queue activity.",
#endif
    DSPTCHR_FLLDES_BFIN_REG_OFFSET,
    0,
    0,
    570,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_BFIN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_TAIL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_TAIL_FIELDS[] =
{
    &DSPTCHR_FLLDES_TAIL_TAIL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_FLLDES_TAIL_REG = 
{
    "FLLDES_TAIL",
#if RU_INCLUDE_DESC
    "TAIL Register",
    "Pointer to the last BD in the linked list of this queue.",
#endif
    DSPTCHR_FLLDES_TAIL_REG_OFFSET,
    0,
    0,
    571,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_TAIL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_FLLDROP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_FLLDROP_FIELDS[] =
{
    &DSPTCHR_FLLDES_FLLDROP_DRPCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_FLLDES_FLLDROP_REG = 
{
    "FLLDES_FLLDROP",
#if RU_INCLUDE_DESC
    "FLLDROP Register",
    "32 bit counter that counts the number of packets arrived when there is no free BD in the FLL.",
#endif
    DSPTCHR_FLLDES_FLLDROP_REG_OFFSET,
    0,
    0,
    572,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_FLLDROP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_LTINT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_LTINT_FIELDS[] =
{
    &DSPTCHR_FLLDES_LTINT_MINBUF_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_FLLDES_LTINT_REG = 
{
    "FLLDES_LTINT",
#if RU_INCLUDE_DESC
    "LTINT Register",
    "Low threshold Interrupt. When number of bytes reach this level, then an interrupt is generated to the Host.",
#endif
    DSPTCHR_FLLDES_LTINT_REG_OFFSET,
    0,
    0,
    573,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_LTINT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_BUFAVAIL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_BUFAVAIL_FIELDS[] =
{
    &DSPTCHR_FLLDES_BUFAVAIL_BUFAVAIL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_FLLDES_BUFAVAIL_REG = 
{
    "FLLDES_BUFAVAIL",
#if RU_INCLUDE_DESC
    "BUFAVAIL Register",
    "number of entries available in queue."
    "bufin - bfout",
#endif
    DSPTCHR_FLLDES_BUFAVAIL_REG_OFFSET,
    0,
    0,
    574,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_BUFAVAIL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_FREEMIN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_FLLDES_FREEMIN_FIELDS[] =
{
    &DSPTCHR_FLLDES_FREEMIN_FREEMIN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_FLLDES_FREEMIN_REG = 
{
    "FLLDES_FREEMIN",
#if RU_INCLUDE_DESC
    "FREEMIN Register",
    "Save the MIN size of free BD in the system that has been recorded during work.",
#endif
    DSPTCHR_FLLDES_FREEMIN_REG_OFFSET,
    0,
    0,
    575,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_FLLDES_FREEMIN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_BDRAM_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_BDRAM_DATA_FIELDS[] =
{
    &DSPTCHR_BDRAM_DATA_RESERVED0_FIELD,
    &DSPTCHR_BDRAM_DATA_DATA_FIELD,
    &DSPTCHR_BDRAM_DATA_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_BDRAM_DATA_REG = 
{
    "BDRAM_DATA",
#if RU_INCLUDE_DESC
    "BD %i Register",
    "This Memory holds the Buffer Descriptor (BD) entries.",
#endif
    DSPTCHR_BDRAM_DATA_REG_OFFSET,
    DSPTCHR_BDRAM_DATA_REG_RAM_CNT,
    4,
    576,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DSPTCHR_BDRAM_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DSPTCHR_PDRAM_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DSPTCHR_PDRAM_DATA_FIELDS[] =
{
    &DSPTCHR_PDRAM_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DSPTCHR_PDRAM_DATA_REG = 
{
    "PDRAM_DATA",
#if RU_INCLUDE_DESC
    "PDRAM %i Register",
    "This memory holds the Packet descriptors.",
#endif
    DSPTCHR_PDRAM_DATA_REG_OFFSET,
    DSPTCHR_PDRAM_DATA_REG_RAM_CNT,
    4,
    577,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DSPTCHR_PDRAM_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: DSPTCHR
 ******************************************************************************/
static const ru_reg_rec *DSPTCHR_REGS[] =
{
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG,
    &DSPTCHR_REORDER_CFG_VQ_EN_REG,
    &DSPTCHR_REORDER_CFG_BB_CFG_REG,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_REG,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_REG,
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG,
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG,
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG,
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_REG,
    &DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG,
    &DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_REG,
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
    &DSPTCHR_MASK_MSK_TSK_255_0_REG,
    &DSPTCHR_MASK_MSK_Q_REG,
    &DSPTCHR_MASK_DLY_Q_REG,
    &DSPTCHR_MASK_NON_DLY_Q_REG,
    &DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG,
    &DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG,
    &DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG,
    &DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG,
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG,
    &DSPTCHR_LOAD_BALANCING_LB_CFG_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_REG,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_REG,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_REG,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_REG,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_REG,
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
    &DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_REG,
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
    &DSPTCHR_BDRAM_DATA_REG,
    &DSPTCHR_PDRAM_DATA_REG,
};

unsigned long DSPTCHR_ADDRS[] =
{
    0x82d20000,
};

const ru_block_rec DSPTCHR_BLOCK = 
{
    "DSPTCHR",
    DSPTCHR_ADDRS,
    1,
    112,
    DSPTCHR_REGS
};

/* End of file XRDP_DSPTCHR.c */
