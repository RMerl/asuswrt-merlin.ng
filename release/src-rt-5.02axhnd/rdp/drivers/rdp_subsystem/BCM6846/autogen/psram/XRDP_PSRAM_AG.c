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
 * Field: PSRAM_CONFIGURATIONS_CTRL_PERM_EN
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD =
{
    "PERM_EN",
#if RU_INCLUDE_DESC
    "permutations_enable",
    "1: enable memory banks permutations"
    "0: disable",
#endif
    PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_COMB_EN
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD =
{
    "COMB_EN",
#if RU_INCLUDE_DESC
    "combinations_enable",
    "1: enable memory banks combinations"
    "0: disable",
#endif
    PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_COMB_FULL
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD =
{
    "COMB_FULL",
#if RU_INCLUDE_DESC
    "combinations_full",
    "1: enable full combinations(also on same 4-banks)"
    "0: disable full combinations(allow only on opposite 4-banks)",
#endif
    PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_BANKS8
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD =
{
    "BANKS8",
#if RU_INCLUDE_DESC
    "banks8",
    "1: all 8 banks are active"
    "0: only 4 banks are active",
#endif
    PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD =
{
    "UB0_REQIN_ESWAP",
#if RU_INCLUDE_DESC
    "ub0_i_reqin_eswap",
    "ub_i_reqin_eswap for ubus slave port - Not connected"
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD =
{
    "UB0_REPOUT_ESWAP",
#if RU_INCLUDE_DESC
    "ub0_i_repout_eswap",
    "ub_i_repout_eswap for ubus slave port - Not connected"
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD =
{
    "UB1_REQIN_ESWAP",
#if RU_INCLUDE_DESC
    "ub1_i_reqin_eswap",
    "ub_i_reqin_eswap for ubus slave port - Not connected"
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD =
{
    "UB1_REPOUT_ESWAP",
#if RU_INCLUDE_DESC
    "ub1_i_repout_eswap",
    "ub_i_repout_eswap for ubus slave port - Not connected"
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD =
{
    "UB2_REQIN_ESWAP",
#if RU_INCLUDE_DESC
    "ub2_i_reqin_eswap",
    "ub_i_reqin_eswap for ubus slave port - Not connected"
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD =
{
    "UB2_REPOUT_ESWAP",
#if RU_INCLUDE_DESC
    "ub2_i_repout_eswap",
    "ub_i_repout_eswap for ubus slave port - Not connected"
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD =
{
    "UB3_REQIN_ESWAP",
#if RU_INCLUDE_DESC
    "ub3_i_reqin_eswap",
    "ub_i_reqin_eswap for ubus slave port - Not connected"
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD =
{
    "UB3_REPOUT_ESWAP",
#if RU_INCLUDE_DESC
    "ub3_i_repout_eswap",
    "ub_i_repout_eswap for ubus slave port - Not connected"
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD =
{
    "COH_EN_EC0",
#if RU_INCLUDE_DESC
    "coherency_check_ec0",
    "1: stall ec client if wants to read same page as one of the pages in the ubus write buffers"
    "0: dont stall",
#endif
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD =
{
    "COH_EN_EC1",
#if RU_INCLUDE_DESC
    "coherency_check_ec1",
    "1: stall ec client if wants to read same page as one of the pages in the ubus write buffers"
    "0: dont stall",
#endif
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD =
{
    "COH_EN_EC2",
#if RU_INCLUDE_DESC
    "coherency_check_ec2",
    "1: stall ec client if wants to read same page as one of the pages in the ubus write buffers"
    "0: dont stall",
#endif
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_WT_0
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD =
{
    "WT_0",
#if RU_INCLUDE_DESC
    "weight_cl0",
    "arbitration weight for client 0 - currently not used.",
#endif
    PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_WT_1
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD =
{
    "WT_1",
#if RU_INCLUDE_DESC
    "weight_cl1",
    "arbitration weight for client 1 - currently not used.",
#endif
    PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_WT_2
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD =
{
    "WT_2",
#if RU_INCLUDE_DESC
    "weight_cl2",
    "arbitration weight for client 2 - currently not used.",
#endif
    PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_ARB_RR
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD =
{
    "ARB_RR",
#if RU_INCLUDE_DESC
    "arb_rr",
    "1: rr between all clients"
    "0: ubus is high priority (def)",
#endif
    PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_CONFIGURATIONS_CTRL_RESERVED0_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CTRL_RESERVED0_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MUEN_CL0MEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD =
{
    "CL0MEN",
#if RU_INCLUDE_DESC
    "cl0_measure_enable",
    "enable monitor for client 0"
    "",
#endif
    PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MUEN_CL1MEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD =
{
    "CL1MEN",
#if RU_INCLUDE_DESC
    "cl1_measure_enable",
    "enable monitor for client 1"
    "",
#endif
    PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MUEN_CL2MEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD =
{
    "CL2MEN",
#if RU_INCLUDE_DESC
    "cl2_measure_enable",
    "enable monitor for client 2"
    "",
#endif
    PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MUEN_CL3MEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD =
{
    "CL3MEN",
#if RU_INCLUDE_DESC
    "cl3_measure_enable",
    "enable monitor for client 3"
    "",
#endif
    PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MUEN_CL4MEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD =
{
    "CL4MEN",
#if RU_INCLUDE_DESC
    "cl4_measure_enable",
    "enable monitor for client 4"
    "",
#endif
    PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MUEN_CL5MEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD =
{
    "CL5MEN",
#if RU_INCLUDE_DESC
    "cl5_measure_enable",
    "enable monitor for client 5",
#endif
    PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MUEN_CL6MEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD =
{
    "CL6MEN",
#if RU_INCLUDE_DESC
    "cl6_measure_enable",
    "enable monitor for client 6",
#endif
    PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MUEN_RESERVED0
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_PM_COUNTERS_MUEN_RESERVED0_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MUEN_RESERVED0_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MUEN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BWCL_TW
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BWCL_TW_FIELD =
{
    "TW",
#if RU_INCLUDE_DESC
    "time_window",
    "measure time window in clock cycles",
#endif
    PSRAM_PM_COUNTERS_BWCL_TW_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BWCL_TW_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BWCL_TW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BWEN_BWCEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD =
{
    "BWCEN",
#if RU_INCLUDE_DESC
    "pm_bw_check_en",
    "start of new monitoring session. zeroes counters on rise.",
#endif
    PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BWEN_RESERVED0
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BWEN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_PM_COUNTERS_BWEN_RESERVED0_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BWEN_RESERVED0_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BWEN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BWEN_CBWCEN
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD =
{
    "CBWCEN",
#if RU_INCLUDE_DESC
    "cyclic_bw_check_en",
    "if this enabled - when the bw period reaches its limit - the counters are reset.",
#endif
    PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BWEN_RESERVED1
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BWEN_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_PM_COUNTERS_BWEN_RESERVED1_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BWEN_RESERVED1_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BWEN_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_MAX_TIME_MAX
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD =
{
    "MAX",
#if RU_INCLUDE_DESC
    "max_time",
    "max wait time",
#endif
    PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_ACC_TIME_MAX
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD =
{
    "MAX",
#if RU_INCLUDE_DESC
    "max_time",
    "max wait time",
#endif
    PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_ACC_REQ_REQ
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD =
{
    "REQ",
#if RU_INCLUDE_DESC
    "number_of_requests",
    "accumulated number of served requests",
#endif
    PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD =
{
    "TIME",
#if RU_INCLUDE_DESC
    "accumulated_time",
    "accumulated wait time",
#endif
    PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD =
{
    "REQ",
#if RU_INCLUDE_DESC
    "Number_of_requests",
    "accumulated number of served requests",
#endif
    PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "Double_word_count",
    "Number of double words that were written to the DDR per client",
#endif
    PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "Double_word_count",
    "Number of double words that were written to the DDR per client",
#endif
    PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BW_WR_CNT_CNT
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "Double_word_count",
    "Number of double words that were written to the DDR per client",
#endif
    PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BW_RD_CNT_CNT
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "Double_word_count",
    "Number of double words that were written to the DDR per client",
#endif
    PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "Double_word_count",
    "Number of double words that were written to the DDR per client",
#endif
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "Double_word_count",
    "Number of double words that were written to the DDR per client",
#endif
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "Double_word_count",
    "Number of double words that were written to the DDR per client",
#endif
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "Double_word_count",
    "Number of double words that were written to the DDR per client",
#endif
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_ARB_REQ_VAL
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_ARB_ARB_VAL
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_ARB_COMB_VAL
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_ARB_COMB_4_VAL
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL
 ******************************************************************************/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD_MASK,
    0,
    PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD_WIDTH,
    PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBGSEL_VS
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBGSEL_VS_FIELD =
{
    "VS",
#if RU_INCLUDE_DESC
    "vec_sel",
    "selects the debug vector",
#endif
    PSRAM_DEBUG_DBGSEL_VS_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBGSEL_VS_FIELD_WIDTH,
    PSRAM_DEBUG_DBGSEL_VS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBGSEL_RESERVED0
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBGSEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_DEBUG_DBGSEL_RESERVED0_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBGSEL_RESERVED0_FIELD_WIDTH,
    PSRAM_DEBUG_DBGSEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBGBUS_VB
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBGBUS_VB_FIELD =
{
    "VB",
#if RU_INCLUDE_DESC
    "dbg_bus",
    "debug vector",
#endif
    PSRAM_DEBUG_DBGBUS_VB_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBGBUS_VB_FIELD_WIDTH,
    PSRAM_DEBUG_DBGBUS_VB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_REQ_VEC_MIPSC_REQ
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD =
{
    "MIPSC_REQ",
#if RU_INCLUDE_DESC
    "mips_c_request",
    "still more commands in the tx fifo",
#endif
    PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD_MASK,
    0,
    PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD_WIDTH,
    PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_REQ_VEC_RNRA_REQ
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD =
{
    "RNRA_REQ",
#if RU_INCLUDE_DESC
    "runner_a_request",
    "still more commands in the tx fifo",
#endif
    PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD_MASK,
    0,
    PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD_WIDTH,
    PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_REQ_VEC_RNRB_REQ
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD =
{
    "RNRB_REQ",
#if RU_INCLUDE_DESC
    "runner_b_request",
    "still more commands in the tx fifo",
#endif
    PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD_MASK,
    0,
    PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD_WIDTH,
    PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_REQ_VEC_SDMA_REQ
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD =
{
    "SDMA_REQ",
#if RU_INCLUDE_DESC
    "sdma_request",
    "still more commands in the tx fifo",
#endif
    PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD_MASK,
    0,
    PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD_WIDTH,
    PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_REQ_VEC_MIPSD_REQ
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD =
{
    "MIPSD_REQ",
#if RU_INCLUDE_DESC
    "mips_d_request",
    "still more commands in the tx fifo",
#endif
    PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD_MASK,
    0,
    PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD_WIDTH,
    PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD =
{
    "MIPSDMA_REQ",
#if RU_INCLUDE_DESC
    "mips_d_dma_request",
    "still more commands in the tx fifo",
#endif
    PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD_MASK,
    0,
    PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD_WIDTH,
    PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_REQ_VEC_RESERVED0
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_DEBUG_REQ_VEC_RESERVED0_FIELD_MASK,
    0,
    PSRAM_DEBUG_REQ_VEC_RESERVED0_FIELD_WIDTH,
    PSRAM_DEBUG_REQ_VEC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD =
{
    "BANK_SEL",
#if RU_INCLUDE_DESC
    "bank_sel",
    "selects bank to capture",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED0
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED0_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED0_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD =
{
    "BANK_ADD_SEL",
#if RU_INCLUDE_DESC
    "bank_addr_sel",
    "selects bank address to capture",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD =
{
    "CAP_WR_EN",
#if RU_INCLUDE_DESC
    "cap_wr_en",
    "capture write enable",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD =
{
    "CAP_RD_EN",
#if RU_INCLUDE_DESC
    "cap_rd_en",
    "capture read enable",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED1
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED1_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED1_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD =
{
    "MAX_WR_CAP",
#if RU_INCLUDE_DESC
    "max_wr_cap",
    "maximum of captures for write."
    "0 means infinite.",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD =
{
    "MAX_RD_CAP",
#if RU_INCLUDE_DESC
    "max_rd_cap",
    "maximum of captures for read."
    "0 means infinite.",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD =
{
    "WR_CAP_CNT_RST",
#if RU_INCLUDE_DESC
    "wr_cap_cnt_reset",
    "reset the counting and start new one."
    "should be asserted, then deasserted, then counting starts again.",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD =
{
    "RD_CAP_CNT_RST",
#if RU_INCLUDE_DESC
    "rd_cap_cnt_reset",
    "reset the counting and start new one."
    "should be asserted, then deasserted, then counting starts again.",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_CFG2_RESERVED0
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG2_RESERVED0_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_CFG2_RESERVED0_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_CFG2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD =
{
    "WR_CAP_NUM_ST",
#if RU_INCLUDE_DESC
    "wr_cap_num_status",
    "actual current capture num for write."
    "max is FFFF (no wrap).",
#endif
    PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD =
{
    "RD_CAP_NUM_ST",
#if RU_INCLUDE_DESC
    "rd_cap_num_status",
    "actual current capture num for read."
    "max is FFFF (no wrap).",
#endif
    PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_W0_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_wr_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_W1_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_wr_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_W2_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_wr_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_W3_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_wr_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_WMSK_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_wr_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_R0_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_rd_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_R1_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_rd_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_R2_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_rd_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PSRAM_DEBUG_DBG_CAP_R3_CV
 ******************************************************************************/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "cap_rd_bus",
    "capture vector",
#endif
    PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD_MASK,
    0,
    PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD_WIDTH,
    PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: PSRAM_CONFIGURATIONS_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_CONFIGURATIONS_CTRL_FIELDS[] =
{
    &PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD,
    &PSRAM_CONFIGURATIONS_CTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_CONFIGURATIONS_CTRL_REG = 
{
    "CONFIGURATIONS_CTRL",
#if RU_INCLUDE_DESC
    "CONTROL Register",
    "control reg",
#endif
    PSRAM_CONFIGURATIONS_CTRL_REG_OFFSET,
    0,
    0,
    807,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    20,
    PSRAM_CONFIGURATIONS_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS[] =
{
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_REG = 
{
    "CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    808,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_MUEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_MUEN_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD,
    &PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD,
    &PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD,
    &PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD,
    &PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD,
    &PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD,
    &PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD,
    &PSRAM_PM_COUNTERS_MUEN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_MUEN_REG = 
{
    "PM_COUNTERS_MUEN",
#if RU_INCLUDE_DESC
    "MON_USER_EN Register",
    "this register contains a bit for enable/disable of the counters. The counters will be reset to zero on the positive edge of the enable bit, and will count until the time window which is decrement counter, will reach zero, or until the enable bit will be de-asserted.",
#endif
    PSRAM_PM_COUNTERS_MUEN_REG_OFFSET,
    0,
    0,
    809,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    PSRAM_PM_COUNTERS_MUEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BWCL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BWCL_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BWCL_TW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BWCL_REG = 
{
    "PM_COUNTERS_BWCL",
#if RU_INCLUDE_DESC
    "BW_COUNTS_CLOCKS Register",
    "determines the time window in which we perform the bandwidth monitoring(on cyclic mode - when cyclic_bw_check_en=1)",
#endif
    PSRAM_PM_COUNTERS_BWCL_REG_OFFSET,
    0,
    0,
    810,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BWCL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BWEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BWEN_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD,
    &PSRAM_PM_COUNTERS_BWEN_RESERVED0_FIELD,
    &PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD,
    &PSRAM_PM_COUNTERS_BWEN_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BWEN_REG = 
{
    "PM_COUNTERS_BWEN",
#if RU_INCLUDE_DESC
    "BW_ENABLE Register",
    "pm_bw_check_en - start of new monitoring session. resets counters on rise."
    "cyclic_bw_check_en - if this enabled - when the bw period reaches its limit - the counters are reet.",
#endif
    PSRAM_PM_COUNTERS_BWEN_REG_OFFSET,
    0,
    0,
    811,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    PSRAM_PM_COUNTERS_BWEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_MAX_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_MAX_TIME_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_MAX_TIME_REG = 
{
    "PM_COUNTERS_MAX_TIME",
#if RU_INCLUDE_DESC
    "MAX_TIME_SERVED %i Register",
    "This array of counters hold the maximum time in clock cycles the client has waited from the moment it had a request pending to the time the request gained arbitration."
    "",
#endif
    PSRAM_PM_COUNTERS_MAX_TIME_REG_OFFSET,
    PSRAM_PM_COUNTERS_MAX_TIME_REG_RAM_CNT,
    4,
    812,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_MAX_TIME_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ACC_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ACC_TIME_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_ACC_TIME_REG = 
{
    "PM_COUNTERS_ACC_TIME",
#if RU_INCLUDE_DESC
    "ACCUMULATE_TIME_SERVED %i Register",
    "This array of counters hold the accumulated time in clock cycles the client has waited from the moment it had a request pending to the time the request gained arbitration. For each access to arbiter, it will be at least 1 cycle."
    "",
#endif
    PSRAM_PM_COUNTERS_ACC_TIME_REG_OFFSET,
    PSRAM_PM_COUNTERS_ACC_TIME_REG_RAM_CNT,
    4,
    813,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ACC_TIME_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ACC_REQ
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ACC_REQ_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_ACC_REQ_REG = 
{
    "PM_COUNTERS_ACC_REQ",
#if RU_INCLUDE_DESC
    "ACCUMULATE_REQ_SERVED %i Register",
    "This array of counters hold the accumulated number of requests that was served per user."
    "",
#endif
    PSRAM_PM_COUNTERS_ACC_REQ_REG_OFFSET,
    PSRAM_PM_COUNTERS_ACC_REQ_REG_RAM_CNT,
    4,
    814,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ACC_REQ_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_LAST_ACC_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_LAST_ACC_TIME_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_LAST_ACC_TIME_REG = 
{
    "PM_COUNTERS_LAST_ACC_TIME",
#if RU_INCLUDE_DESC
    "ACCUMULATE_TIME_LAST %i Register",
    "This array of counters hold the Result of th elast measure of accumulated time in clock cycles the client has waited from the moment it had a request pending to the time the request gained arbitration."
    "",
#endif
    PSRAM_PM_COUNTERS_LAST_ACC_TIME_REG_OFFSET,
    PSRAM_PM_COUNTERS_LAST_ACC_TIME_REG_RAM_CNT,
    4,
    815,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_LAST_ACC_TIME_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_LAST_ACC_REQ
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_LAST_ACC_REQ_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_LAST_ACC_REQ_REG = 
{
    "PM_COUNTERS_LAST_ACC_REQ",
#if RU_INCLUDE_DESC
    "ACCUMULATE_REQ_LAST %i Register",
    "This array of counters hold the last result of accumulated number of requests that was served per user on cyclic measure."
    "",
#endif
    PSRAM_PM_COUNTERS_LAST_ACC_REQ_REG_OFFSET,
    PSRAM_PM_COUNTERS_LAST_ACC_REQ_REG_RAM_CNT,
    4,
    816,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_LAST_ACC_REQ_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_WR_CNT_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_REG = 
{
    "PM_COUNTERS_BW_WR_CNT_ACC",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_WR_ACC Register",
    "This counter holds the sum of the WR_CNT array."
    "It holds the result of the current measure."
    "If the measure is a single measure, the result will be kept until de-assertion and assertion of the SINGLE start bit.",
#endif
    PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_REG_OFFSET,
    0,
    0,
    817,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_RD_CNT_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_REG = 
{
    "PM_COUNTERS_BW_RD_CNT_ACC",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_RD_ACC Register",
    "This counter holds the sum of the RD_CNT array."
    "It holds the result of the current measure."
    "If the measure is a single measure, the result will be kept until de-assertion and assertion of the SINGLE start bit.",
#endif
    PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_REG_OFFSET,
    0,
    0,
    818,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_WR_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_WR_CNT_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BW_WR_CNT_REG = 
{
    "PM_COUNTERS_BW_WR_CNT",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_WR %i Register",
    "This array of counters holds the number of double words written to the psram per client."
    "It holds the result of the current measure."
    "If the measure is a single measure, the result will be kept until de-assertion and assertion of the SINGLE start bit.",
#endif
    PSRAM_PM_COUNTERS_BW_WR_CNT_REG_OFFSET,
    PSRAM_PM_COUNTERS_BW_WR_CNT_REG_RAM_CNT,
    4,
    819,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_WR_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_RD_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_RD_CNT_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BW_RD_CNT_REG = 
{
    "PM_COUNTERS_BW_RD_CNT",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_RD %i Register",
    "This array of counters holds the number of double words read from the psram per client."
    "It holds the result of the current measure."
    "If the measure is a single measure, the result will be kept until de-assertion and assertion of the SINGLE start bit.",
#endif
    PSRAM_PM_COUNTERS_BW_RD_CNT_REG_OFFSET,
    PSRAM_PM_COUNTERS_BW_RD_CNT_REG_RAM_CNT,
    4,
    820,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_RD_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_REG = 
{
    "PM_COUNTERS_BW_WR_CNT_LAST_ACC",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_WR_LAST_ACC Register",
    "This counter is a sum of the WR_CNT_LAST counters, which holds the number of double words written to the psram per client."
    "When the measure is cyclic, it holds the result of the last measure, sampled once every end of a time window.",
#endif
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_REG_OFFSET,
    0,
    0,
    821,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_REG = 
{
    "PM_COUNTERS_BW_RD_CNT_LAST_ACC",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_RD_LAST_ACC Register",
    "This counter is a sum of the RD_CNT_LAST counters, which holds the number of double words written to the psram per client."
    "When the measure is cyclic, it holds the result of the last measure, sampled once every end of a time window.",
#endif
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_REG_OFFSET,
    0,
    0,
    822,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_REG = 
{
    "PM_COUNTERS_BW_WR_CNT_LAST",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_WR_LAST %i Register",
    "This array of counters holds the number of double words written to the psram per client."
    "When the measure is cyclic, it holds the result of the last measure, sampled once every end of a time window.",
#endif
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_REG_OFFSET,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_REG_RAM_CNT,
    4,
    823,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_REG = 
{
    "PM_COUNTERS_BW_RD_CNT_LAST",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_RD_LAST %i Register",
    "This array of counters holds the number of double words read from the psram per client."
    "When the measure is cyclic, it holds the result of the last measure, sampled once every end of a time window.",
#endif
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_REG_OFFSET,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_REG_RAM_CNT,
    4,
    824,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_REQ
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_REQ_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_ARB_REQ_REG = 
{
    "PM_COUNTERS_ARB_REQ",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_REQ_CYCLES Register",
    "Number of cycles there were requests  (even one)"
    "",
#endif
    PSRAM_PM_COUNTERS_ARB_REQ_REG_OFFSET,
    0,
    0,
    825,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_REQ_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_ARB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_ARB_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_ARB_ARB_REG = 
{
    "PM_COUNTERS_ARB_ARB",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_ARB_CYCLES Register",
    "Number of cycles there were more that 1 request for arbitration"
    "",
#endif
    PSRAM_PM_COUNTERS_ARB_ARB_REG_OFFSET,
    0,
    0,
    826,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_ARB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_COMB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_COMB_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_ARB_COMB_REG = 
{
    "PM_COUNTERS_ARB_COMB",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_COMB_CYCLES Register",
    "Number of cycles there were commands combinations"
    "",
#endif
    PSRAM_PM_COUNTERS_ARB_COMB_REG_OFFSET,
    0,
    0,
    827,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_COMB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_COMB_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_COMB_4_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_ARB_COMB_4_REG = 
{
    "PM_COUNTERS_ARB_COMB_4",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_COMB_SAME4_CYCLES Register",
    "Number of cycles there were commands combinations in the same 4 banks"
    "",
#endif
    PSRAM_PM_COUNTERS_ARB_COMB_4_REG_OFFSET,
    0,
    0,
    828,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_COMB_4_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_COMB_BANKS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_COMB_BANKS_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_PM_COUNTERS_ARB_COMB_BANKS_REG = 
{
    "PM_COUNTERS_ARB_COMB_BANKS",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_COMB_BANKS Register",
    "Number of totsl banks that were accessed  during commands combinations cycles",
#endif
    PSRAM_PM_COUNTERS_ARB_COMB_BANKS_REG_OFFSET,
    0,
    0,
    829,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_COMB_BANKS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBGSEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBGSEL_FIELDS[] =
{
    &PSRAM_DEBUG_DBGSEL_VS_FIELD,
    &PSRAM_DEBUG_DBGSEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBGSEL_REG = 
{
    "DEBUG_DBGSEL",
#if RU_INCLUDE_DESC
    "DBG_MUX_SEL Register",
    "selects the debug vecore",
#endif
    PSRAM_DEBUG_DBGSEL_REG_OFFSET,
    0,
    0,
    830,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    PSRAM_DEBUG_DBGSEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBGBUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBGBUS_FIELDS[] =
{
    &PSRAM_DEBUG_DBGBUS_VB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBGBUS_REG = 
{
    "DEBUG_DBGBUS",
#if RU_INCLUDE_DESC
    "DBG_BUS Register",
    "the debug bus",
#endif
    PSRAM_DEBUG_DBGBUS_REG_OFFSET,
    0,
    0,
    831,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBGBUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_REQ_VEC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_REQ_VEC_FIELDS[] =
{
    &PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_REQ_VEC_REG = 
{
    "DEBUG_REQ_VEC",
#if RU_INCLUDE_DESC
    "REQUEST_VECTOR Register",
    "vector of all the requests of the clients",
#endif
    PSRAM_DEBUG_REQ_VEC_REG_OFFSET,
    0,
    0,
    832,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    PSRAM_DEBUG_REQ_VEC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_CFG1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_CFG1_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED0_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG1_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_CFG1_REG = 
{
    "DEBUG_DBG_CAP_CFG1",
#if RU_INCLUDE_DESC
    "DBG_CAP_CFG1 Register",
    "debug capture config",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG1_REG_OFFSET,
    0,
    0,
    833,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    PSRAM_DEBUG_DBG_CAP_CFG1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_CFG2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_CFG2_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_CFG2_REG = 
{
    "DEBUG_DBG_CAP_CFG2",
#if RU_INCLUDE_DESC
    "DBG_CAP_CFG2 Register",
    "debug capture config",
#endif
    PSRAM_DEBUG_DBG_CAP_CFG2_REG_OFFSET,
    0,
    0,
    834,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    PSRAM_DEBUG_DBG_CAP_CFG2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_ST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_ST_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD,
    &PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_ST_REG = 
{
    "DEBUG_DBG_CAP_ST",
#if RU_INCLUDE_DESC
    "DBG_CAP_STAT Register",
    "debug capture status",
#endif
    PSRAM_DEBUG_DBG_CAP_ST_REG_OFFSET,
    0,
    0,
    835,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    PSRAM_DEBUG_DBG_CAP_ST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_W0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_W0_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_W0_REG = 
{
    "DEBUG_DBG_CAP_W0",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA0 Register",
    "debug capture write data0 register [32*1-1:32*0]",
#endif
    PSRAM_DEBUG_DBG_CAP_W0_REG_OFFSET,
    0,
    0,
    836,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_W0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_W1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_W1_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_W1_REG = 
{
    "DEBUG_DBG_CAP_W1",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA1 Register",
    "debug capture write data1 register [32*2-1:32*1]",
#endif
    PSRAM_DEBUG_DBG_CAP_W1_REG_OFFSET,
    0,
    0,
    837,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_W1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_W2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_W2_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_W2_REG = 
{
    "DEBUG_DBG_CAP_W2",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA2 Register",
    "debug capture write data2 register [32*3-1:32*2]",
#endif
    PSRAM_DEBUG_DBG_CAP_W2_REG_OFFSET,
    0,
    0,
    838,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_W2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_W3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_W3_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_W3_REG = 
{
    "DEBUG_DBG_CAP_W3",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA3 Register",
    "debug capture write data3 register [32*4-1:32*3]",
#endif
    PSRAM_DEBUG_DBG_CAP_W3_REG_OFFSET,
    0,
    0,
    839,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_W3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_WMSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_WMSK_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_WMSK_REG = 
{
    "DEBUG_DBG_CAP_WMSK",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA_MASK Register",
    "debug capture write mask register (16b for 16B=128b of data in bank row)",
#endif
    PSRAM_DEBUG_DBG_CAP_WMSK_REG_OFFSET,
    0,
    0,
    840,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_WMSK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_R0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_R0_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_R0_REG = 
{
    "DEBUG_DBG_CAP_R0",
#if RU_INCLUDE_DESC
    "DBG_CAP_RDATA0 Register",
    "debug capture read data0 register [32*1-1:32*0]",
#endif
    PSRAM_DEBUG_DBG_CAP_R0_REG_OFFSET,
    0,
    0,
    841,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_R0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_R1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_R1_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_R1_REG = 
{
    "DEBUG_DBG_CAP_R1",
#if RU_INCLUDE_DESC
    "DBG_CAP_RDATA1 Register",
    "debug capture read data1 register [32*2-1:32*1]",
#endif
    PSRAM_DEBUG_DBG_CAP_R1_REG_OFFSET,
    0,
    0,
    842,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_R1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_R2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_R2_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_R2_REG = 
{
    "DEBUG_DBG_CAP_R2",
#if RU_INCLUDE_DESC
    "DBG_CAP_RDATA2 Register",
    "debug capture read data2 register [32*3-1:32*2]",
#endif
    PSRAM_DEBUG_DBG_CAP_R2_REG_OFFSET,
    0,
    0,
    843,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_R2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_R3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_R3_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PSRAM_DEBUG_DBG_CAP_R3_REG = 
{
    "DEBUG_DBG_CAP_R3",
#if RU_INCLUDE_DESC
    "DBG_CAP_RDATA3 Register",
    "debug capture read data3 register [32*4-1:32*3]",
#endif
    PSRAM_DEBUG_DBG_CAP_R3_REG_OFFSET,
    0,
    0,
    844,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_R3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: PSRAM
 ******************************************************************************/
static const ru_reg_rec *PSRAM_REGS[] =
{
    &PSRAM_CONFIGURATIONS_CTRL_REG,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &PSRAM_PM_COUNTERS_MUEN_REG,
    &PSRAM_PM_COUNTERS_BWCL_REG,
    &PSRAM_PM_COUNTERS_BWEN_REG,
    &PSRAM_PM_COUNTERS_MAX_TIME_REG,
    &PSRAM_PM_COUNTERS_ACC_TIME_REG,
    &PSRAM_PM_COUNTERS_ACC_REQ_REG,
    &PSRAM_PM_COUNTERS_LAST_ACC_TIME_REG,
    &PSRAM_PM_COUNTERS_LAST_ACC_REQ_REG,
    &PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_REG,
    &PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_REG,
    &PSRAM_PM_COUNTERS_BW_WR_CNT_REG,
    &PSRAM_PM_COUNTERS_BW_RD_CNT_REG,
    &PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_REG,
    &PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_REG,
    &PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_REG,
    &PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_REG,
    &PSRAM_PM_COUNTERS_ARB_REQ_REG,
    &PSRAM_PM_COUNTERS_ARB_ARB_REG,
    &PSRAM_PM_COUNTERS_ARB_COMB_REG,
    &PSRAM_PM_COUNTERS_ARB_COMB_4_REG,
    &PSRAM_PM_COUNTERS_ARB_COMB_BANKS_REG,
    &PSRAM_DEBUG_DBGSEL_REG,
    &PSRAM_DEBUG_DBGBUS_REG,
    &PSRAM_DEBUG_REQ_VEC_REG,
    &PSRAM_DEBUG_DBG_CAP_CFG1_REG,
    &PSRAM_DEBUG_DBG_CAP_CFG2_REG,
    &PSRAM_DEBUG_DBG_CAP_ST_REG,
    &PSRAM_DEBUG_DBG_CAP_W0_REG,
    &PSRAM_DEBUG_DBG_CAP_W1_REG,
    &PSRAM_DEBUG_DBG_CAP_W2_REG,
    &PSRAM_DEBUG_DBG_CAP_W3_REG,
    &PSRAM_DEBUG_DBG_CAP_WMSK_REG,
    &PSRAM_DEBUG_DBG_CAP_R0_REG,
    &PSRAM_DEBUG_DBG_CAP_R1_REG,
    &PSRAM_DEBUG_DBG_CAP_R2_REG,
    &PSRAM_DEBUG_DBG_CAP_R3_REG,
};

unsigned long PSRAM_ADDRS[] =
{
    0x82d99000,
};

const ru_block_rec PSRAM_BLOCK = 
{
    "PSRAM",
    PSRAM_ADDRS,
    1,
    38,
    PSRAM_REGS
};

/* End of file XRDP_PSRAM.c */
