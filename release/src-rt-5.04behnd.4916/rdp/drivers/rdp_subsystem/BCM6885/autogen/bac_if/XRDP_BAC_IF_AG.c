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


#include "XRDP_BAC_IF_AG.h"

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST, TYPE: Type_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_SIZE_FIELD =
{
    "SIZE",
#if RU_INCLUDE_DESC
    "",
    "size\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_SIZE_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_SIZE_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_SIZE_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_REG =
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST",
#if RU_INCLUDE_DESC
    "BB_OUT_BURST Register",
    "0: no split, result is transmitted in full\n2: split results into 2 cycles bursts\n4(def): split results into 4 cycles bursts\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_REG_OFFSET },
    0,
    0,
    184,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP, TYPE: Type_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OP *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_OP_FIELD =
{
    "OP",
#if RU_INCLUDE_DESC
    "",
    "opcode\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_OP_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_OP_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_OP_FIELD_SHIFT },
    134217761,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_OP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_REG =
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP",
#if RU_INCLUDE_DESC
    "PROG_RTRN_OP Register",
    "program return opcode\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_REG_OFFSET },
    0,
    0,
    185,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM, TYPE: Type_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BA *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BA_FIELD =
{
    "BA",
#if RU_INCLUDE_DESC
    "",
    "base_address (in 8B resolution).\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BA_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BA_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BT *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BT_FIELD =
{
    "BT",
#if RU_INCLUDE_DESC
    "",
    "first task the base address refers to.\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BT_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BT_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OFST *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_OFST_FIELD =
{
    "OFST",
#if RU_INCLUDE_DESC
    "",
    "offset jump for each task (in 8B resolution).\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_OFST_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_OFST_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_OFST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "enable bit for the data/prog feature in xpe/natc.\nneeded to be set to enable the feature, otherwise all data will be written to data memory.\n\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_EN_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_EN_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BA_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_BT_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_OFST_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_REG =
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM",
#if RU_INCLUDE_DESC
    "PROGRAM_MEM_PARAMS Register",
    "base address and jump offset for program memory. All in 8B resolution.\nAddress = base + (task_number - base_task) * offset\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_REG_OFFSET },
    0,
    0,
    186,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, TYPE: Type_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\nTRIGGERS_HW\n\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\nTRIGGERS_HW\n\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\nTRIGGERS_HW\n\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\nTRIGGERS_HW\n\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\nTRIGGERS_HW\n\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_REG =
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    187,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE, TYPE: Type_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "",
    "route address\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_REG =
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE",
#if RU_INCLUDE_DESC
    "BB_OVERRIDE_ROUTE Register",
    "override configuration for the route of one of the peripherals\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_REG_OFFSET },
    0,
    0,
    188,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL, TYPE: Type_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "",
    "destination source id\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "enable\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_REG =
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL",
#if RU_INCLUDE_DESC
    "BB_OVERRIDE_CTRL Register",
    "override configuration for the route of one of the peripherals\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_REG_OFFSET },
    0,
    0,
    189,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO, TYPE: Type_BACIF_BLOCK_BACIF_FIFOS_INGFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "",
    "lower 31b of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "valid bit of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_REG =
{
    "BACIF_BLOCK_BACIF_FIFOS_INGFIFO",
#if RU_INCLUDE_DESC
    "INGRS_FIFO 0..127 Register",
    "ingress fifo debug\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_REG_OFFSET },
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_REG_RAM_CNT,
    4,
    190,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO, TYPE: Type_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "",
    "lower 31b of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "valid bit of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_REG =
{
    "BACIF_BLOCK_BACIF_FIFOS_CMDFIFO",
#if RU_INCLUDE_DESC
    "CMD_FIFO 0..31 Register",
    "cmd fifo debug\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_REG_OFFSET },
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_REG_RAM_CNT,
    4,
    191,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO, TYPE: Type_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "",
    "lower 31b of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "valid bit of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_REG =
{
    "BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO",
#if RU_INCLUDE_DESC
    "RSLT_FIFO 0..31 Register",
    "result fifo debug\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_REG_OFFSET },
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_REG_RAM_CNT,
    4,
    192,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO, TYPE: Type_BACIF_BLOCK_BACIF_FIFOS_EGFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "",
    "lower 31b of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "valid bit of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_REG =
{
    "BACIF_BLOCK_BACIF_FIFOS_EGFIFO",
#if RU_INCLUDE_DESC
    "EGRS_FIFO 0..7 Register",
    "egress fifo debug\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_REG_OFFSET },
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_REG_RAM_CNT,
    4,
    193,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR, TYPE: Type_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "",
    "lower 31b of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "valid bit of entry\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_REG =
{
    "BACIF_BLOCK_BACIF_FIFOS_RPPRMARR",
#if RU_INCLUDE_DESC
    "PRLY_PARAMS_ARR_FIFO 0..7 Register",
    "reply params array debug\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_REG_OFFSET },
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_REG_RAM_CNT,
    4,
    194,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT",
#if RU_INCLUDE_DESC
    "ING_F_CNTR Register",
    "number of bb transactions that enter the ingress fifo of accl_if\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_REG_OFFSET },
    0,
    0,
    195,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT",
#if RU_INCLUDE_DESC
    "CMD_F_CNTR Register",
    "number of commands (eob) that enter the command fifo of accl_if\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_REG_OFFSET },
    0,
    0,
    196,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT",
#if RU_INCLUDE_DESC
    "ENG_CMD_CNTR Register",
    "number of commands (eob) that enter the engine from the accl_if\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_REG_OFFSET },
    0,
    0,
    197,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT",
#if RU_INCLUDE_DESC
    "ENG_RSLT_CNTR Register",
    "number of results (eob) that enter the result fifo of accl_if from the engine\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_REG_OFFSET },
    0,
    0,
    198,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT",
#if RU_INCLUDE_DESC
    "RSLT_F_CNTR Register",
    "number of results (eob) that leave the result fifo of accl_if\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_REG_OFFSET },
    0,
    0,
    199,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT",
#if RU_INCLUDE_DESC
    "EGR_F_CNTR Register",
    "number of bb transactions that leave the egress fifo of accl_if\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_REG_OFFSET },
    0,
    0,
    200,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C",
#if RU_INCLUDE_DESC
    "ERR_CMD_LONG_CNTR Register",
    "number of commands that entered and were longer than the max command size for the accelerator configured in HW parameter\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_REG_OFFSET },
    0,
    0,
    201,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C",
#if RU_INCLUDE_DESC
    "ERR_PARAMS_OVERFLOW_CNTR Register",
    "reply params array is full (no free entries), and a new command has arrived\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_REG_OFFSET },
    0,
    0,
    202,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C",
#if RU_INCLUDE_DESC
    "ERR_PARAMS_UNDERFLOW_CNTR Register",
    "reply params array is empty, and a new result has arrived\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_REG_OFFSET },
    0,
    0,
    203,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "",
    "value of cntr\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_CNTR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_CNTR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C",
#if RU_INCLUDE_DESC
    "ERR_EOB_DATA_PROG_CNTR Register",
    "eob from natc-engine rslt, when still writing rslt to rnr-data mem, before writing rest of rslt to prog mem, so no wakeup\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_REG_OFFSET },
    0,
    0,
    204,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG, TYPE: Type_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_CLR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD =
{
    "RD_CLR",
#if RU_INCLUDE_DESC
    "",
    "read clear bit\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRAP *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD =
{
    "WRAP",
#if RU_INCLUDE_DESC
    "",
    "read clear bit\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_REG =
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "bits rd_clr and wrap for the counters\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_REG_OFFSET },
    0,
    0,
    205,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0, TYPE: Type_BACIF_BLOCK_BACIF_DEBUG_DBG0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value of debug reg\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_VAL_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_VAL_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0 *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_REG =
{
    "BACIF_BLOCK_BACIF_DEBUG_DBG0",
#if RU_INCLUDE_DESC
    "DEBUG0 Register",
    "debug register.\nshould be checked during reg test only for access, not data, since data is unexpected during the test.\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_REG_OFFSET },
    0,
    0,
    206,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS, TYPE: Type_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RTRN_RNR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_RTRN_RNR_FIELD =
{
    "RTRN_RNR",
#if RU_INCLUDE_DESC
    "",
    "return runner number\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_RTRN_RNR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_RTRN_RNR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_RTRN_RNR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TSK_RNR *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_TSK_RNR_FIELD =
{
    "TSK_RNR",
#if RU_INCLUDE_DESC
    "",
    "task number\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_TSK_RNR_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_TSK_RNR_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_TSK_RNR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VLD *****/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_VLD_FIELD =
{
    "VLD",
#if RU_INCLUDE_DESC
    "",
    "capture value(for 1st error) is valid\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_VLD_FIELD_MASK },
    0,
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_VLD_FIELD_WIDTH },
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_VLD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_RTRN_RNR_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_TSK_RNR_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_VLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS *****/
const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_REG =
{
    "BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS",
#if RU_INCLUDE_DESC
    "CAPT_EOB_ERR_PARAMS Register",
    "capture rtrn_rnr_num, task_num from first time eob_data_prog error occurred\n",
#endif
    { BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_REG_OFFSET },
    0,
    0,
    207,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_FIELDS,
#endif
};

unsigned long BAC_IF_ADDRS[] =
{
    0xB4B60000,
    0xB4B61000,
    0xB4B62000,
    0xB4B63000,
    0xB4B64000,
    0xB4B65000,
    0xB4B66000,
    0xB4B67000,
    0xB4B68000,
    0xB4B69000,
    0xB4B6A000,
};

static const ru_reg_rec *BAC_IF_REGS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BB_OUT_BRST_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PROG_RTRN_OP_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_PRGRM_M_PRM_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDROUTE_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_BBOVRDCTRL_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_EOB_DATA_PROG_C_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_DEBUG_DBG0_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_DEBUG_CAPT_EOB_ERR_PRMS_REG,
};

const ru_block_rec BAC_IF_BLOCK =
{
    "BAC_IF",
#if RU_INCLUDE_SIZE
    { 0x00000E14 },
#endif
    BAC_IF_ADDRS,
    11,
    24,
    BAC_IF_REGS,
};
