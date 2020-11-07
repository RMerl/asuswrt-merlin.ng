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

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR
 ******************************************************************************/
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
    892,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE
 ******************************************************************************/
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
    893,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
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
    894,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_INGFIFO
 ******************************************************************************/
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
    895,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_CMDFIFO
 ******************************************************************************/
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
    896,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO
 ******************************************************************************/
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
    897,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_EGFIFO
 ******************************************************************************/
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
    898,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_FIFOS_RPPRMARR
 ******************************************************************************/
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
    899,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT
 ******************************************************************************/
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
    900,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT
 ******************************************************************************/
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
    901,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT
 ******************************************************************************/
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
    902,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT
 ******************************************************************************/
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
    903,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT
 ******************************************************************************/
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
    904,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT
 ******************************************************************************/
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
    905,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C
 ******************************************************************************/
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
    906,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C
 ******************************************************************************/
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
    907,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C
 ******************************************************************************/
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
    908,
};

/******************************************************************************
 * Register: BAC_IF_BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG
 ******************************************************************************/
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
    909,
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
    0x82e44000,
};

const ru_block_rec BAC_IF_BLOCK = 
{
    "BAC_IF",
    BAC_IF_ADDRS,
    5,
    18,
    BAC_IF_REGS
};

/* End of file XRDP_BAC_IF.c */
