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
 * Register: PSRAM_CONFIGURATIONS_CTRL
 ******************************************************************************/
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
    360,
};

/******************************************************************************
 * Register: PSRAM_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
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
    361,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_MUEN
 ******************************************************************************/
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
    362,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BWCL
 ******************************************************************************/
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
    363,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BWEN
 ******************************************************************************/
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
    364,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_MAX_TIME
 ******************************************************************************/
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
    365,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ACC_TIME
 ******************************************************************************/
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
    366,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ACC_REQ
 ******************************************************************************/
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
    367,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_LAST_ACC_TIME
 ******************************************************************************/
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
    368,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_LAST_ACC_REQ
 ******************************************************************************/
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
    369,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_WR_CNT_ACC
 ******************************************************************************/
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
    370,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_RD_CNT_ACC
 ******************************************************************************/
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
    371,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_WR_CNT
 ******************************************************************************/
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
    372,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_RD_CNT
 ******************************************************************************/
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
    373,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST_ACC
 ******************************************************************************/
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
    374,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST_ACC
 ******************************************************************************/
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
    375,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_WR_CNT_LAST
 ******************************************************************************/
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
    376,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_BW_RD_CNT_LAST
 ******************************************************************************/
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
    377,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_REQ
 ******************************************************************************/
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
    378,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_ARB
 ******************************************************************************/
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
    379,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_COMB
 ******************************************************************************/
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
    380,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_COMB_4
 ******************************************************************************/
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
    381,
};

/******************************************************************************
 * Register: PSRAM_PM_COUNTERS_ARB_COMB_BANKS
 ******************************************************************************/
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
    382,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBGSEL
 ******************************************************************************/
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
    383,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBGBUS
 ******************************************************************************/
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
    384,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_REQ_VEC
 ******************************************************************************/
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
    385,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_CFG1
 ******************************************************************************/
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
    386,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_CFG2
 ******************************************************************************/
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
    387,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_ST
 ******************************************************************************/
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
    388,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_W0
 ******************************************************************************/
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
    389,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_W1
 ******************************************************************************/
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
    390,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_W2
 ******************************************************************************/
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
    391,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_W3
 ******************************************************************************/
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
    392,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_WMSK
 ******************************************************************************/
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
    393,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_R0
 ******************************************************************************/
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
    394,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_R1
 ******************************************************************************/
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
    395,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_R2
 ******************************************************************************/
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
    396,
};

/******************************************************************************
 * Register: PSRAM_DEBUG_DBG_CAP_R3
 ******************************************************************************/
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
    397,
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
    0x82c80000,
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
