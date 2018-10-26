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
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_THR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_THR_FIELD =
{
    "THR",
#if RU_INCLUDE_DESC
    "thr",
    "threshold",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_THR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_THR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_THR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_RESERVED0
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_RESERVED0_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_RESERVED0_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_EN
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "en",
    "en override route address",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_EN_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_EN_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED0
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED0_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED0_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ID
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ID_FIELD =
{
    "ID",
#if RU_INCLUDE_DESC
    "id",
    "id to override route address",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ID_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ID_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED1
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED1_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED1_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ADDR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "addr",
    "addr to override route address",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ADDR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ADDR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED2
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED2_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED2_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "entry",
    "lower 31b of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "valid",
    "valid bit of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "entry",
    "lower 31b of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "valid",
    "valid bit of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "entry",
    "lower 31b of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "valid",
    "valid bit of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "entry",
    "lower 31b of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "valid",
    "valid bit of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "entry",
    "lower 31b of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "valid",
    "valid bit of entry",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD =
{
    "CNTR",
#if RU_INCLUDE_DESC
    "cntr",
    "value of cntr",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD =
{
    "RD_CLR",
#if RU_INCLUDE_DESC
    "rd_clr",
    "read clear bit",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD =
{
    "WRAP",
#if RU_INCLUDE_DESC
    "wrap",
    "read clear bit",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_MASK,
    0,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_WIDTH,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_THR_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_REG = 
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR",
#if RU_INCLUDE_DESC
    "RSLT_FIFO_FULL_THR Register",
    "FULL threshold of result fifo for rdy indication to engine:  If there are less words than thr left - there will be !rdy indication to engine, even if there is antry empty, and result will not be pushed into fifo."
    "- NOT USED ANYMORE!",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_REG_OFFSET,
    0,
    0,
    1047,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_EN_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED0_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ID_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED1_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_ADDR_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_REG = 
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE",
#if RU_INCLUDE_DESC
    "DEC_ROUTE_OVERIDE Register",
    "route override info for the route address decoder",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_REG_OFFSET,
    0,
    0,
    1048,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_REG = 
{
    "BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    1049,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_REG = 
{
    "BACIF_BLOCK_BACIF_FIFOS_INGFIFO",
#if RU_INCLUDE_DESC
    "INGRS_FIFO %i Register",
    "ingress fifo debug",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_REG_OFFSET,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_REG_RAM_CNT,
    4,
    1050,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_REG = 
{
    "BACIF_BLOCK_BACIF_FIFOS_CMDFIFO",
#if RU_INCLUDE_DESC
    "CMD_FIFO %i Register",
    "cmd fifo debug",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_REG_OFFSET,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_REG_RAM_CNT,
    4,
    1051,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_REG = 
{
    "BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO",
#if RU_INCLUDE_DESC
    "RSLT_FIFO %i Register",
    "result fifo debug",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_REG_OFFSET,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_REG_RAM_CNT,
    4,
    1052,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_REG = 
{
    "BACIF_BLOCK_BACIF_FIFOS_EGFIFO",
#if RU_INCLUDE_DESC
    "EGRS_FIFO %i Register",
    "egress fifo debug",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_REG_OFFSET,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_REG_RAM_CNT,
    4,
    1053,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_ENTRY_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_REG = 
{
    "BACIF_BLOCK_BACIF_FIFOS_RPPRMARR",
#if RU_INCLUDE_DESC
    "PRLY_PARAMS_ARR_FIFO %i Register",
    "reply params array debug",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_REG_OFFSET,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_REG_RAM_CNT,
    4,
    1054,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT",
#if RU_INCLUDE_DESC
    "ING_F_CNTR Register",
    "number of bb transactions that enter the ingress fifo of accl_if",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_REG_OFFSET,
    0,
    0,
    1055,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT",
#if RU_INCLUDE_DESC
    "CMD_F_CNTR Register",
    "number of commands (eob) that enter the command fifo of accl_if",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_REG_OFFSET,
    0,
    0,
    1056,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT",
#if RU_INCLUDE_DESC
    "ENG_CMD_CNTR Register",
    "number of commands (eob) that enter the engine from the accl_if",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_REG_OFFSET,
    0,
    0,
    1057,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT",
#if RU_INCLUDE_DESC
    "ENG_RSLT_CNTR Register",
    "number of results (eob) that enter the result fifo of accl_if from the engine",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_REG_OFFSET,
    0,
    0,
    1058,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT",
#if RU_INCLUDE_DESC
    "RSLT_F_CNTR Register",
    "number of results (eob) that leave the result fifo of accl_if",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_REG_OFFSET,
    0,
    0,
    1059,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT",
#if RU_INCLUDE_DESC
    "EGR_F_CNTR Register",
    "number of bb transactions that leave the egress fifo of accl_if",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_REG_OFFSET,
    0,
    0,
    1060,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C",
#if RU_INCLUDE_DESC
    "ERR_CMD_LONG_CNTR Register",
    "number of commands that entered and were longer than the max command size for the accelerator configured in HW parameter",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_REG_OFFSET,
    0,
    0,
    1061,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C",
#if RU_INCLUDE_DESC
    "ERR_PARAMS_OVERFLOW_CNTR Register",
    "reply params array is full (no free entries), and a new command has arrived",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_REG_OFFSET,
    0,
    0,
    1062,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_CNTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C",
#if RU_INCLUDE_DESC
    "ERR_PARAMS_UNDERFLOW_CNTR Register",
    "reply params array is empty, and a new result has arrived",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_REG_OFFSET,
    0,
    0,
    1063,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_FIELDS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD,
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_REG = 
{
    "BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "bits rd_clr and wrap for the counters",
#endif
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_REG_OFFSET,
    0,
    0,
    1064,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: BAC_IF
 ******************************************************************************/
static const ru_reg_rec *BAC_IF_REGS[] =
{
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE_REG,
    &BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
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
    &BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG_REG,
};

unsigned long BAC_IF_ADDRS[] =
{
    0x82e40000,
    0x82e41000,
    0x82e42000,
    0x82e43000,
};

const ru_block_rec BAC_IF_BLOCK = 
{
    "BAC_IF",
    BAC_IF_ADDRS,
    4,
    18,
    BAC_IF_REGS
};

/* End of file XRDP_BAC_IF.c */
