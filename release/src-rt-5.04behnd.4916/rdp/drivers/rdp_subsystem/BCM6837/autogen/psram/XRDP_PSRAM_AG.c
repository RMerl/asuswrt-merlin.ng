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


#include "XRDP_PSRAM_AG.h"

/******************************************************************************
 * Register: NAME: PSRAM_CONFIGURATIONS_CTRL, TYPE: Type_PSRAM_BLOCK_PSRAM_CONFIGURATIONS_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PERM_EN *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD =
{
    "PERM_EN",
#if RU_INCLUDE_DESC
    "",
    "1: enable memory banks permutations\n0: disable\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_PERM_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COMB_EN *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD =
{
    "COMB_EN",
#if RU_INCLUDE_DESC
    "",
    "1: enable memory banks combinations\n0: disable\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_COMB_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COMB_FULL *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD =
{
    "COMB_FULL",
#if RU_INCLUDE_DESC
    "",
    "1: enable full combinations(also on same 4-banks)\n0: disable full combinations(allow only on opposite 4-banks)\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_COMB_FULL_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BANKS8 *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD =
{
    "BANKS8",
#if RU_INCLUDE_DESC
    "",
    "1: all 8 banks are active\n0: only 4 banks are active\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_BANKS8_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UB0_REQIN_ESWAP *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD =
{
    "UB0_REQIN_ESWAP",
#if RU_INCLUDE_DESC
    "",
    "ub_i_reqin_eswap for ubus slave port - Not connected\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_UB0_REQIN_ESWAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UB0_REPOUT_ESWAP *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD =
{
    "UB0_REPOUT_ESWAP",
#if RU_INCLUDE_DESC
    "",
    "ub_i_repout_eswap for ubus slave port - Not connected\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_UB0_REPOUT_ESWAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UB1_REQIN_ESWAP *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD =
{
    "UB1_REQIN_ESWAP",
#if RU_INCLUDE_DESC
    "",
    "ub_i_reqin_eswap for ubus slave port - Not connected\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_UB1_REQIN_ESWAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UB1_REPOUT_ESWAP *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD =
{
    "UB1_REPOUT_ESWAP",
#if RU_INCLUDE_DESC
    "",
    "ub_i_repout_eswap for ubus slave port - Not connected\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_UB1_REPOUT_ESWAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UB2_REQIN_ESWAP *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD =
{
    "UB2_REQIN_ESWAP",
#if RU_INCLUDE_DESC
    "",
    "ub_i_reqin_eswap for ubus slave port - Not connected\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_UB2_REQIN_ESWAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UB2_REPOUT_ESWAP *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD =
{
    "UB2_REPOUT_ESWAP",
#if RU_INCLUDE_DESC
    "",
    "ub_i_repout_eswap for ubus slave port - Not connected\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_UB2_REPOUT_ESWAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UB3_REQIN_ESWAP *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD =
{
    "UB3_REQIN_ESWAP",
#if RU_INCLUDE_DESC
    "",
    "ub_i_reqin_eswap for ubus slave port - Not connected\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_UB3_REQIN_ESWAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UB3_REPOUT_ESWAP *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD =
{
    "UB3_REPOUT_ESWAP",
#if RU_INCLUDE_DESC
    "",
    "ub_i_repout_eswap for ubus slave port - Not connected\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_UB3_REPOUT_ESWAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COH_EN_EC0 *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD =
{
    "COH_EN_EC0",
#if RU_INCLUDE_DESC
    "",
    "1: stall ec client if wants to read same page as one of the pages in the ubus write buffers\n0: dont stall\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC0_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COH_EN_EC1 *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD =
{
    "COH_EN_EC1",
#if RU_INCLUDE_DESC
    "",
    "1: stall ec client if wants to read same page as one of the pages in the ubus write buffers\n0: dont stall\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COH_EN_EC2 *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD =
{
    "COH_EN_EC2",
#if RU_INCLUDE_DESC
    "",
    "1: stall ec client if wants to read same page as one of the pages in the ubus write buffers\n0: dont stall\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_COH_EN_EC2_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WT_0 *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD =
{
    "WT_0",
#if RU_INCLUDE_DESC
    "",
    "arbitration weight for client 0 - currently not used.\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_WT_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WT_1 *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD =
{
    "WT_1",
#if RU_INCLUDE_DESC
    "",
    "arbitration weight for client 1 - currently not used.\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_WT_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WT_2 *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD =
{
    "WT_2",
#if RU_INCLUDE_DESC
    "",
    "arbitration weight for client 2 - currently not used.\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_WT_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ARB_RR *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD =
{
    "ARB_RR",
#if RU_INCLUDE_DESC
    "",
    "1: rr between all clients\n0: ubus is high priority (def)\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_ARB_RR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SCRM_EN *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CTRL_SCRM_EN_FIELD =
{
    "SCRM_EN",
#if RU_INCLUDE_DESC
    "",
    "scrambler enable (def:1)\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_SCRM_EN_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CTRL_SCRM_EN_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CTRL_SCRM_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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
    &PSRAM_CONFIGURATIONS_CTRL_SCRM_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_CONFIGURATIONS_CTRL *****/
const ru_reg_rec PSRAM_CONFIGURATIONS_CTRL_REG =
{
    "CONFIGURATIONS_CTRL",
#if RU_INCLUDE_DESC
    "CONTROL Register",
    "control reg\n",
#endif
    { PSRAM_CONFIGURATIONS_CTRL_REG_OFFSET },
    0,
    0,
    633,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    20,
    PSRAM_CONFIGURATIONS_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_CONFIGURATIONS_SCRM_SEED, TYPE: Type_PSRAM_BLOCK_PSRAM_CONFIGURATIONS_SCRM_SEED
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec PSRAM_CONFIGURATIONS_SCRM_SEED_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "programmable seed\n\n\n",
#endif
    { PSRAM_CONFIGURATIONS_SCRM_SEED_VAL_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_SCRM_SEED_VAL_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_SCRM_SEED_VAL_FIELD_SHIFT },
    439041101,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_CONFIGURATIONS_SCRM_SEED_FIELDS[] =
{
    &PSRAM_CONFIGURATIONS_SCRM_SEED_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_CONFIGURATIONS_SCRM_SEED *****/
const ru_reg_rec PSRAM_CONFIGURATIONS_SCRM_SEED_REG =
{
    "CONFIGURATIONS_SCRM_SEED",
#if RU_INCLUDE_DESC
    "SCRM_SEED Register",
    "scrambler seed\n",
#endif
    { PSRAM_CONFIGURATIONS_SCRM_SEED_REG_OFFSET },
    0,
    0,
    634,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_CONFIGURATIONS_SCRM_SEED_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_CONFIGURATIONS_SCRM_ADDR, TYPE: Type_PSRAM_BLOCK_PSRAM_CONFIGURATIONS_SCRM_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec PSRAM_CONFIGURATIONS_SCRM_ADDR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "programmable seed\n\n\n",
#endif
    { PSRAM_CONFIGURATIONS_SCRM_ADDR_VAL_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_SCRM_ADDR_VAL_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_SCRM_ADDR_VAL_FIELD_SHIFT },
    2557891634,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_CONFIGURATIONS_SCRM_ADDR_FIELDS[] =
{
    &PSRAM_CONFIGURATIONS_SCRM_ADDR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_CONFIGURATIONS_SCRM_ADDR *****/
const ru_reg_rec PSRAM_CONFIGURATIONS_SCRM_ADDR_REG =
{
    "CONFIGURATIONS_SCRM_ADDR",
#if RU_INCLUDE_DESC
    "SCRM_ADDR Register",
    "scrambler addr\n4 lsb and 23(1Mb)/22(2Mb)/21(4Mb) msb will replace the real addr bits\n",
#endif
    { PSRAM_CONFIGURATIONS_SCRM_ADDR_REG_OFFSET },
    0,
    0,
    635,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_CONFIGURATIONS_SCRM_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL, TYPE: Type_PSRAM_BLOCK_PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\n",
#endif
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\n\n\n",
#endif
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\n",
#endif
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\n",
#endif
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\n",
#endif
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS[] =
{
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL *****/
const ru_reg_rec PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_REG =
{
    "CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    636,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_MUEN, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_MUEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CL0MEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD =
{
    "CL0MEN",
#if RU_INCLUDE_DESC
    "",
    "enable monitor for client 0\n\n",
#endif
    { PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_MUEN_CL0MEN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CL1MEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD =
{
    "CL1MEN",
#if RU_INCLUDE_DESC
    "",
    "enable monitor for client 1\n\n",
#endif
    { PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_MUEN_CL1MEN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CL2MEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD =
{
    "CL2MEN",
#if RU_INCLUDE_DESC
    "",
    "enable monitor for client 2\n\n",
#endif
    { PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_MUEN_CL2MEN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CL3MEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD =
{
    "CL3MEN",
#if RU_INCLUDE_DESC
    "",
    "enable monitor for client 3\n\n",
#endif
    { PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_MUEN_CL3MEN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CL4MEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD =
{
    "CL4MEN",
#if RU_INCLUDE_DESC
    "",
    "enable monitor for client 4\n\n",
#endif
    { PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_MUEN_CL4MEN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CL5MEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD =
{
    "CL5MEN",
#if RU_INCLUDE_DESC
    "",
    "enable monitor for client 5\n",
#endif
    { PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_MUEN_CL5MEN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CL6MEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD =
{
    "CL6MEN",
#if RU_INCLUDE_DESC
    "",
    "enable monitor for client 6\n",
#endif
    { PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_MUEN_CL6MEN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_MUEN *****/
const ru_reg_rec PSRAM_PM_COUNTERS_MUEN_REG =
{
    "PM_COUNTERS_MUEN",
#if RU_INCLUDE_DESC
    "MON_USER_EN Register",
    "this register contains a bit for enable/disable of the counters. The counters will be reset to zero on the positive edge of the enable bit, and will count until the time window which is decrement counter, will reach zero, or until the enable bit will be de-asserted.\n",
#endif
    { PSRAM_PM_COUNTERS_MUEN_REG_OFFSET },
    0,
    0,
    637,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    PSRAM_PM_COUNTERS_MUEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BWCL, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BWCL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TW *****/
const ru_field_rec PSRAM_PM_COUNTERS_BWCL_TW_FIELD =
{
    "TW",
#if RU_INCLUDE_DESC
    "",
    "measure time window in clock cycles\n",
#endif
    { PSRAM_PM_COUNTERS_BWCL_TW_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BWCL_TW_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BWCL_TW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BWCL_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BWCL_TW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BWCL *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BWCL_REG =
{
    "PM_COUNTERS_BWCL",
#if RU_INCLUDE_DESC
    "BW_COUNTS_CLOCKS Register",
    "determines the time window in which we perform the bandwidth monitoring(on cyclic mode - when cyclic_bw_check_en=1)\n",
#endif
    { PSRAM_PM_COUNTERS_BWCL_REG_OFFSET },
    0,
    0,
    638,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BWCL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BWEN, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BWEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BWCEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD =
{
    "BWCEN",
#if RU_INCLUDE_DESC
    "",
    "start of new monitoring session. zeroes counters on rise.\n",
#endif
    { PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CBWCEN *****/
const ru_field_rec PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD =
{
    "CBWCEN",
#if RU_INCLUDE_DESC
    "",
    "if this enabled - when the bw period reaches its limit - the counters are reset.\n",
#endif
    { PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BWEN_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BWEN_BWCEN_FIELD,
    &PSRAM_PM_COUNTERS_BWEN_CBWCEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BWEN *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BWEN_REG =
{
    "PM_COUNTERS_BWEN",
#if RU_INCLUDE_DESC
    "BW_ENABLE Register",
    "pm_bw_check_en - start of new monitoring session. resets counters on rise.\ncyclic_bw_check_en - if this enabled - when the bw period reaches its limit - the counters are reet.\n",
#endif
    { PSRAM_PM_COUNTERS_BWEN_REG_OFFSET },
    0,
    0,
    639,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    PSRAM_PM_COUNTERS_BWEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_MAX_TIME, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_MAX_TIME
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX *****/
const ru_field_rec PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD =
{
    "MAX",
#if RU_INCLUDE_DESC
    "",
    "max wait time\n",
#endif
    { PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_MAX_TIME_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_MAX_TIME_MAX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_MAX_TIME *****/
const ru_reg_rec PSRAM_PM_COUNTERS_MAX_TIME_REG =
{
    "PM_COUNTERS_MAX_TIME",
#if RU_INCLUDE_DESC
    "MAX_TIME_SERVED 0..6 Register",
    "This array of counters hold the maximum time in clock cycles the client has waited from the moment it had a request pending to the time the request gained arbitration.\n\n",
#endif
    { PSRAM_PM_COUNTERS_MAX_TIME_REG_OFFSET },
    PSRAM_PM_COUNTERS_MAX_TIME_REG_RAM_CNT,
    4,
    640,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_MAX_TIME_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_ACC_TIME, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_ACC_TIME
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX *****/
const ru_field_rec PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD =
{
    "MAX",
#if RU_INCLUDE_DESC
    "",
    "max wait time\n",
#endif
    { PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ACC_TIME_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ACC_TIME_MAX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_ACC_TIME *****/
const ru_reg_rec PSRAM_PM_COUNTERS_ACC_TIME_REG =
{
    "PM_COUNTERS_ACC_TIME",
#if RU_INCLUDE_DESC
    "ACCUMULATE_TIME_SERVED 0..6 Register",
    "This array of counters hold the accumulated time in clock cycles the client has waited from the moment it had a request pending to the time the request gained arbitration. For each access to arbiter, it will be at least 1 cycle.\n\n",
#endif
    { PSRAM_PM_COUNTERS_ACC_TIME_REG_OFFSET },
    PSRAM_PM_COUNTERS_ACC_TIME_REG_RAM_CNT,
    4,
    641,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ACC_TIME_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_ACC_REQ, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_ACC_REQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ *****/
const ru_field_rec PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD =
{
    "REQ",
#if RU_INCLUDE_DESC
    "",
    "accumulated number of served requests\n",
#endif
    { PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ACC_REQ_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ACC_REQ_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_ACC_REQ *****/
const ru_reg_rec PSRAM_PM_COUNTERS_ACC_REQ_REG =
{
    "PM_COUNTERS_ACC_REQ",
#if RU_INCLUDE_DESC
    "ACCUMULATE_REQ_SERVED 0..6 Register",
    "This array of counters hold the accumulated number of requests that was served per user.\n\n",
#endif
    { PSRAM_PM_COUNTERS_ACC_REQ_REG_OFFSET },
    PSRAM_PM_COUNTERS_ACC_REQ_REG_RAM_CNT,
    4,
    642,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ACC_REQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_LAST_ACC_TIME, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_LAST_ACC_TIME
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TIME *****/
const ru_field_rec PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD =
{
    "TIME",
#if RU_INCLUDE_DESC
    "",
    "accumulated wait time\n",
#endif
    { PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_LAST_ACC_TIME_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_LAST_ACC_TIME_TIME_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_LAST_ACC_TIME *****/
const ru_reg_rec PSRAM_PM_COUNTERS_LAST_ACC_TIME_REG =
{
    "PM_COUNTERS_LAST_ACC_TIME",
#if RU_INCLUDE_DESC
    "ACCUMULATE_TIME_LAST 0..6 Register",
    "This array of counters hold the Result of th elast measure of accumulated time in clock cycles the client has waited from the moment it had a request pending to the time the request gained arbitration.\n\n",
#endif
    { PSRAM_PM_COUNTERS_LAST_ACC_TIME_REG_OFFSET },
    PSRAM_PM_COUNTERS_LAST_ACC_TIME_REG_RAM_CNT,
    4,
    643,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_LAST_ACC_TIME_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_LAST_ACC_REQ, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_LAST_ACC_REQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ *****/
const ru_field_rec PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD =
{
    "REQ",
#if RU_INCLUDE_DESC
    "",
    "accumulated number of served requests\n",
#endif
    { PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_LAST_ACC_REQ_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_LAST_ACC_REQ_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_LAST_ACC_REQ *****/
const ru_reg_rec PSRAM_PM_COUNTERS_LAST_ACC_REQ_REG =
{
    "PM_COUNTERS_LAST_ACC_REQ",
#if RU_INCLUDE_DESC
    "ACCUMULATE_REQ_LAST 0..6 Register",
    "This array of counters hold the last result of accumulated number of requests that was served per user on cyclic measure.\n\n",
#endif
    { PSRAM_PM_COUNTERS_LAST_ACC_REQ_REG_OFFSET },
    PSRAM_PM_COUNTERS_LAST_ACC_REQ_REG_RAM_CNT,
    4,
    644,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_LAST_ACC_REQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BW_WR_CNT_ACC, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BW_WR_CNT_ACC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of double words that were written to the DDR per client\n",
#endif
    { PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BW_WR_CNT_ACC *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_REG =
{
    "PM_COUNTERS_BW_WR_CNT_ACC",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_WR_ACC Register",
    "This counter holds the sum of the WR_CNT array.\nIt holds the result of the current measure.\nIf the measure is a single measure, the result will be kept until de-assertion and assertion of the SINGLE start bit.\n",
#endif
    { PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_REG_OFFSET },
    0,
    0,
    645,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_WR_CNT_ACC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BW_RD_CNT_ACC, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BW_RD_CNT_ACC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of double words that were written to the DDR per client\n",
#endif
    { PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BW_RD_CNT_ACC *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_REG =
{
    "PM_COUNTERS_BW_RD_CNT_ACC",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_RD_ACC Register",
    "This counter holds the sum of the RD_CNT array.\nIt holds the result of the current measure.\nIf the measure is a single measure, the result will be kept until de-assertion and assertion of the SINGLE start bit.\n",
#endif
    { PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_REG_OFFSET },
    0,
    0,
    646,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_RD_CNT_ACC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BW_WR_CNT, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BW_WR_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of double words that were written to the DDR per client\n",
#endif
    { PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_WR_CNT_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_WR_CNT_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BW_WR_CNT *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BW_WR_CNT_REG =
{
    "PM_COUNTERS_BW_WR_CNT",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_WR 0..6 Register",
    "This array of counters holds the number of double words written to the psram per client.\nIt holds the result of the current measure.\nIf the measure is a single measure, the result will be kept until de-assertion and assertion of the SINGLE start bit.\n",
#endif
    { PSRAM_PM_COUNTERS_BW_WR_CNT_REG_OFFSET },
    PSRAM_PM_COUNTERS_BW_WR_CNT_REG_RAM_CNT,
    4,
    647,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_WR_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BW_RD_CNT, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BW_RD_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of double words that were written to the DDR per client\n",
#endif
    { PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_RD_CNT_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_RD_CNT_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BW_RD_CNT *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BW_RD_CNT_REG =
{
    "PM_COUNTERS_BW_RD_CNT",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_RD 0..6 Register",
    "This array of counters holds the number of double words read from the psram per client.\nIt holds the result of the current measure.\nIf the measure is a single measure, the result will be kept until de-assertion and assertion of the SINGLE start bit.\n",
#endif
    { PSRAM_PM_COUNTERS_BW_RD_CNT_REG_OFFSET },
    PSRAM_PM_COUNTERS_BW_RD_CNT_REG_RAM_CNT,
    4,
    648,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_RD_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of double words that were written to the DDR per client\n",
#endif
    { PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_REG =
{
    "PM_COUNTERS_BW_WR_CNT_LAST_ACC",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_WR_LAST_ACC Register",
    "This counter is a sum of the WR_CNT_LAST counters, which holds the number of double words written to the psram per client.\nWhen the measure is cyclic, it holds the result of the last measure, sampled once every end of a time window.\n",
#endif
    { PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_REG_OFFSET },
    0,
    0,
    649,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of double words that were written to the DDR per client\n",
#endif
    { PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_REG =
{
    "PM_COUNTERS_BW_RD_CNT_LAST_ACC",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_RD_LAST_ACC Register",
    "This counter is a sum of the RD_CNT_LAST counters, which holds the number of double words written to the psram per client.\nWhen the measure is cyclic, it holds the result of the last measure, sampled once every end of a time window.\n",
#endif
    { PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_REG_OFFSET },
    0,
    0,
    650,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BW_WR_CNT_LAST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of double words that were written to the DDR per client\n",
#endif
    { PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_REG =
{
    "PM_COUNTERS_BW_WR_CNT_LAST",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_WR_LAST 0..6 Register",
    "This array of counters holds the number of double words written to the psram per client.\nWhen the measure is cyclic, it holds the result of the last measure, sampled once every end of a time window.\n",
#endif
    { PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_REG_OFFSET },
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_REG_RAM_CNT,
    4,
    651,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_BW_RD_CNT_LAST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of double words that were written to the DDR per client\n",
#endif
    { PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST *****/
const ru_reg_rec PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_REG =
{
    "PM_COUNTERS_BW_RD_CNT_LAST",
#if RU_INCLUDE_DESC
    "BW_COUNTS_DATA_RD_LAST 0..6 Register",
    "This array of counters holds the number of double words read from the psram per client.\nWhen the measure is cyclic, it holds the result of the last measure, sampled once every end of a time window.\n",
#endif
    { PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_REG_OFFSET },
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_REG_RAM_CNT,
    4,
    652,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_ARB_REQ, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_ARB_REQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_REQ_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_REQ_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_ARB_REQ *****/
const ru_reg_rec PSRAM_PM_COUNTERS_ARB_REQ_REG =
{
    "PM_COUNTERS_ARB_REQ",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_REQ_CYCLES Register",
    "Number of cycles there were requests  (even one)\n\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_REQ_REG_OFFSET },
    0,
    0,
    653,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_REQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_ARB_ARB, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_ARB_ARB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_ARB_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_ARB_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_ARB_ARB *****/
const ru_reg_rec PSRAM_PM_COUNTERS_ARB_ARB_REG =
{
    "PM_COUNTERS_ARB_ARB",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_ARB_CYCLES Register",
    "Number of cycles there were more that 1 request for arbitration\n\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_ARB_REG_OFFSET },
    0,
    0,
    654,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_ARB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_ARB_COMB, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_ARB_COMB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_COMB_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_COMB_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_ARB_COMB *****/
const ru_reg_rec PSRAM_PM_COUNTERS_ARB_COMB_REG =
{
    "PM_COUNTERS_ARB_COMB",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_COMB_CYCLES Register",
    "Number of cycles there were commands combinations\n\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_COMB_REG_OFFSET },
    0,
    0,
    655,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_COMB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_ARB_COMB_4, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_ARB_COMB_4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_COMB_4_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_COMB_4_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_ARB_COMB_4 *****/
const ru_reg_rec PSRAM_PM_COUNTERS_ARB_COMB_4_REG =
{
    "PM_COUNTERS_ARB_COMB_4",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_COMB_SAME4_CYCLES Register",
    "Number of cycles there were commands combinations in the same 4 banks\n\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_COMB_4_REG_OFFSET },
    0,
    0,
    656,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_COMB_4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_PM_COUNTERS_ARB_COMB_BANKS, TYPE: Type_PSRAM_BLOCK_PSRAM_PM_COUNTERS_ARB_COMB_BANKS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD_MASK },
    0,
    { PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD_WIDTH },
    { PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_PM_COUNTERS_ARB_COMB_BANKS_FIELDS[] =
{
    &PSRAM_PM_COUNTERS_ARB_COMB_BANKS_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_PM_COUNTERS_ARB_COMB_BANKS *****/
const ru_reg_rec PSRAM_PM_COUNTERS_ARB_COMB_BANKS_REG =
{
    "PM_COUNTERS_ARB_COMB_BANKS",
#if RU_INCLUDE_DESC
    "ARB_TOTAL_COMB_BANKS Register",
    "Number of totsl banks that were accessed  during commands combinations cycles\n",
#endif
    { PSRAM_PM_COUNTERS_ARB_COMB_BANKS_REG_OFFSET },
    0,
    0,
    657,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_PM_COUNTERS_ARB_COMB_BANKS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBGSEL, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBGSEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VS *****/
const ru_field_rec PSRAM_DEBUG_DBGSEL_VS_FIELD =
{
    "VS",
#if RU_INCLUDE_DESC
    "",
    "selects the debug vector\n",
#endif
    { PSRAM_DEBUG_DBGSEL_VS_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBGSEL_VS_FIELD_WIDTH },
    { PSRAM_DEBUG_DBGSEL_VS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBGSEL_FIELDS[] =
{
    &PSRAM_DEBUG_DBGSEL_VS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBGSEL *****/
const ru_reg_rec PSRAM_DEBUG_DBGSEL_REG =
{
    "DEBUG_DBGSEL",
#if RU_INCLUDE_DESC
    "DBG_MUX_SEL Register",
    "selects the debug vecore\n",
#endif
    { PSRAM_DEBUG_DBGSEL_REG_OFFSET },
    0,
    0,
    658,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBGSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBGBUS, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBGBUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VB *****/
const ru_field_rec PSRAM_DEBUG_DBGBUS_VB_FIELD =
{
    "VB",
#if RU_INCLUDE_DESC
    "",
    "debug vector\n",
#endif
    { PSRAM_DEBUG_DBGBUS_VB_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBGBUS_VB_FIELD_WIDTH },
    { PSRAM_DEBUG_DBGBUS_VB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBGBUS_FIELDS[] =
{
    &PSRAM_DEBUG_DBGBUS_VB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBGBUS *****/
const ru_reg_rec PSRAM_DEBUG_DBGBUS_REG =
{
    "DEBUG_DBGBUS",
#if RU_INCLUDE_DESC
    "DBG_BUS Register",
    "the debug bus\n",
#endif
    { PSRAM_DEBUG_DBGBUS_REG_OFFSET },
    0,
    0,
    659,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBGBUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_REQ_VEC, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_REQ_VEC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MIPSC_REQ *****/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD =
{
    "MIPSC_REQ",
#if RU_INCLUDE_DESC
    "",
    "still more commands in the tx fifo\n",
#endif
    { PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD_MASK },
    0,
    { PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD_WIDTH },
    { PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RNRA_REQ *****/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD =
{
    "RNRA_REQ",
#if RU_INCLUDE_DESC
    "",
    "still more commands in the tx fifo\n",
#endif
    { PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD_MASK },
    0,
    { PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD_WIDTH },
    { PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RNRB_REQ *****/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD =
{
    "RNRB_REQ",
#if RU_INCLUDE_DESC
    "",
    "still more commands in the tx fifo\n",
#endif
    { PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD_MASK },
    0,
    { PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD_WIDTH },
    { PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SDMA_REQ *****/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD =
{
    "SDMA_REQ",
#if RU_INCLUDE_DESC
    "",
    "still more commands in the tx fifo\n",
#endif
    { PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD_MASK },
    0,
    { PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD_WIDTH },
    { PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIPSD_REQ *****/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD =
{
    "MIPSD_REQ",
#if RU_INCLUDE_DESC
    "",
    "still more commands in the tx fifo\n",
#endif
    { PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD_MASK },
    0,
    { PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD_WIDTH },
    { PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIPSDMA_REQ *****/
const ru_field_rec PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD =
{
    "MIPSDMA_REQ",
#if RU_INCLUDE_DESC
    "",
    "still more commands in the tx fifo\n",
#endif
    { PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD_MASK },
    0,
    { PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD_WIDTH },
    { PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_REQ_VEC_FIELDS[] =
{
    &PSRAM_DEBUG_REQ_VEC_MIPSC_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_RNRA_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_RNRB_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_SDMA_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_MIPSD_REQ_FIELD,
    &PSRAM_DEBUG_REQ_VEC_MIPSDMA_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_REQ_VEC *****/
const ru_reg_rec PSRAM_DEBUG_REQ_VEC_REG =
{
    "DEBUG_REQ_VEC",
#if RU_INCLUDE_DESC
    "REQUEST_VECTOR Register",
    "vector of all the requests of the clients\n",
#endif
    { PSRAM_DEBUG_REQ_VEC_REG_OFFSET },
    0,
    0,
    660,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    PSRAM_DEBUG_REQ_VEC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_CFG1, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_CFG1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BANK_SEL *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD =
{
    "BANK_SEL",
#if RU_INCLUDE_DESC
    "",
    "selects bank to capture\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BANK_ADD_SEL *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD =
{
    "BANK_ADD_SEL",
#if RU_INCLUDE_DESC
    "",
    "selects bank address to capture\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CAP_WR_EN *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD =
{
    "CAP_WR_EN",
#if RU_INCLUDE_DESC
    "",
    "capture write enable\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CAP_RD_EN *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD =
{
    "CAP_RD_EN",
#if RU_INCLUDE_DESC
    "",
    "capture read enable\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_CFG1_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_CFG1_BANK_SEL_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG1_BANK_ADD_SEL_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG1_CAP_WR_EN_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG1_CAP_RD_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_CFG1 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_CFG1_REG =
{
    "DEBUG_DBG_CAP_CFG1",
#if RU_INCLUDE_DESC
    "DBG_CAP_CFG1 Register",
    "debug capture config\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG1_REG_OFFSET },
    0,
    0,
    661,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    PSRAM_DEBUG_DBG_CAP_CFG1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_CFG2, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_CFG2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_WR_CAP *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD =
{
    "MAX_WR_CAP",
#if RU_INCLUDE_DESC
    "",
    "maximum of captures for write.\n0 means infinite.\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_RD_CAP *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD =
{
    "MAX_RD_CAP",
#if RU_INCLUDE_DESC
    "",
    "maximum of captures for read.\n0 means infinite.\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WR_CAP_CNT_RST *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD =
{
    "WR_CAP_CNT_RST",
#if RU_INCLUDE_DESC
    "",
    "reset the counting and start new one.\nshould be asserted, then deasserted, then counting starts again.\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_CAP_CNT_RST *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD =
{
    "RD_CAP_CNT_RST",
#if RU_INCLUDE_DESC
    "",
    "reset the counting and start new one.\nshould be asserted, then deasserted, then counting starts again.\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_CFG2_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_CFG2_MAX_WR_CAP_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG2_MAX_RD_CAP_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG2_WR_CAP_CNT_RST_FIELD,
    &PSRAM_DEBUG_DBG_CAP_CFG2_RD_CAP_CNT_RST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_CFG2 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_CFG2_REG =
{
    "DEBUG_DBG_CAP_CFG2",
#if RU_INCLUDE_DESC
    "DBG_CAP_CFG2 Register",
    "debug capture config\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_CFG2_REG_OFFSET },
    0,
    0,
    662,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    PSRAM_DEBUG_DBG_CAP_CFG2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_ST, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_ST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WR_CAP_NUM_ST *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD =
{
    "WR_CAP_NUM_ST",
#if RU_INCLUDE_DESC
    "",
    "actual current capture num for write.\nmax is FFFF (no wrap).\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_CAP_NUM_ST *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD =
{
    "RD_CAP_NUM_ST",
#if RU_INCLUDE_DESC
    "",
    "actual current capture num for read.\nmax is FFFF (no wrap).\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_ST_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_ST_WR_CAP_NUM_ST_FIELD,
    &PSRAM_DEBUG_DBG_CAP_ST_RD_CAP_NUM_ST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_ST *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_ST_REG =
{
    "DEBUG_DBG_CAP_ST",
#if RU_INCLUDE_DESC
    "DBG_CAP_STAT Register",
    "debug capture status\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_ST_REG_OFFSET },
    0,
    0,
    663,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    PSRAM_DEBUG_DBG_CAP_ST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_W0, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_W0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_W0_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_W0_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_W0 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_W0_REG =
{
    "DEBUG_DBG_CAP_W0",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA0 Register",
    "debug capture write data0 register [32*1-1:32*0]\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_W0_REG_OFFSET },
    0,
    0,
    664,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_W0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_W1, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_W1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_W1_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_W1_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_W1 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_W1_REG =
{
    "DEBUG_DBG_CAP_W1",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA1 Register",
    "debug capture write data1 register [32*2-1:32*1]\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_W1_REG_OFFSET },
    0,
    0,
    665,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_W1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_W2, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_W2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_W2_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_W2_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_W2 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_W2_REG =
{
    "DEBUG_DBG_CAP_W2",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA2 Register",
    "debug capture write data2 register [32*3-1:32*2]\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_W2_REG_OFFSET },
    0,
    0,
    666,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_W2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_W3, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_W3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_W3_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_W3_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_W3 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_W3_REG =
{
    "DEBUG_DBG_CAP_W3",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA3 Register",
    "debug capture write data3 register [32*4-1:32*3]\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_W3_REG_OFFSET },
    0,
    0,
    667,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_W3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_WMSK, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_WMSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_WMSK_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_WMSK_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_WMSK *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_WMSK_REG =
{
    "DEBUG_DBG_CAP_WMSK",
#if RU_INCLUDE_DESC
    "DBG_CAP_WDATA_MASK Register",
    "debug capture write mask register (16b for 16B=128b of data in bank row)\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_WMSK_REG_OFFSET },
    0,
    0,
    668,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_WMSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_R0, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_R0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_R0_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_R0_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_R0 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_R0_REG =
{
    "DEBUG_DBG_CAP_R0",
#if RU_INCLUDE_DESC
    "DBG_CAP_RDATA0 Register",
    "debug capture read data0 register [32*1-1:32*0]\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_R0_REG_OFFSET },
    0,
    0,
    669,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_R0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_R1, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_R1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_R1_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_R1_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_R1 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_R1_REG =
{
    "DEBUG_DBG_CAP_R1",
#if RU_INCLUDE_DESC
    "DBG_CAP_RDATA1 Register",
    "debug capture read data1 register [32*2-1:32*1]\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_R1_REG_OFFSET },
    0,
    0,
    670,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_R1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_R2, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_R2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_R2_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_R2_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_R2 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_R2_REG =
{
    "DEBUG_DBG_CAP_R2",
#if RU_INCLUDE_DESC
    "DBG_CAP_RDATA2 Register",
    "debug capture read data2 register [32*3-1:32*2]\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_R2_REG_OFFSET },
    0,
    0,
    671,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_R2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: PSRAM_DEBUG_DBG_CAP_R3, TYPE: Type_PSRAM_BLOCK_PSRAM_DEBUG_DBG_CAP_R3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CV *****/
const ru_field_rec PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD =
{
    "CV",
#if RU_INCLUDE_DESC
    "",
    "capture vector\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD_MASK },
    0,
    { PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD_WIDTH },
    { PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_DEBUG_DBG_CAP_R3_FIELDS[] =
{
    &PSRAM_DEBUG_DBG_CAP_R3_CV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_DEBUG_DBG_CAP_R3 *****/
const ru_reg_rec PSRAM_DEBUG_DBG_CAP_R3_REG =
{
    "DEBUG_DBG_CAP_R3",
#if RU_INCLUDE_DESC
    "DBG_CAP_RDATA3 Register",
    "debug capture read data3 register [32*4-1:32*3]\n",
#endif
    { PSRAM_DEBUG_DBG_CAP_R3_REG_OFFSET },
    0,
    0,
    672,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_DEBUG_DBG_CAP_R3_FIELDS,
#endif
};

unsigned long PSRAM_ADDRS[] =
{
    0x82000000,
};

static const ru_reg_rec *PSRAM_REGS[] =
{
    &PSRAM_CONFIGURATIONS_CTRL_REG,
    &PSRAM_CONFIGURATIONS_SCRM_SEED_REG,
    &PSRAM_CONFIGURATIONS_SCRM_ADDR_REG,
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

const ru_block_rec PSRAM_BLOCK =
{
    "PSRAM",
    PSRAM_ADDRS,
    1,
    40,
    PSRAM_REGS,
};
