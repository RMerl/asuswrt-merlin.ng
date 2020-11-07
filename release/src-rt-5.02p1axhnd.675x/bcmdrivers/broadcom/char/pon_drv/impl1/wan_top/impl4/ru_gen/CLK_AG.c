/*
   Copyright (c) 2015 Broadcom Corporation
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
 * Field: CLK_DEJITTER_SAMPLING_CTL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec CLK_DEJITTER_SAMPLING_CTL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLK_DEJITTER_SAMPLING_CTL_0_RESERVED0_FIELD_MASK,
    0,
    CLK_DEJITTER_SAMPLING_CTL_0_RESERVED0_FIELD_WIDTH,
    CLK_DEJITTER_SAMPLING_CTL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_DEJITTER_SAMPLING_CTL_0_CFG_PLL_SMPL_PRD
 ******************************************************************************/
const ru_field_rec CLK_DEJITTER_SAMPLING_CTL_0_CFG_PLL_SMPL_PRD_FIELD =
{
    "CFG_PLL_SMPL_PRD",
#if RU_INCLUDE_DESC
    "",
    "Specifies the sampling period of the sampling counters, running in"
    "the following domains :"
    "PON_SERDES : 10G - 515.625 MHz;  2.5G - 312.5 MHz; 1G - 125 MHz;"
    "100FX - 25 MHz; GPON - 155.5 MHz."
    "LAN_SERDES : 156.25 MHz"
    "SGPHY      : 25 MHz"
    "DSL       : 35.328 MHz"
    ""
    "The sampling counter should be set around 100 ms.  The unit is in"
    "each clock domain's period."
    "Hence, sampling period = X value/frequency.",
#endif
    CLK_DEJITTER_SAMPLING_CTL_0_CFG_PLL_SMPL_PRD_FIELD_MASK,
    0,
    CLK_DEJITTER_SAMPLING_CTL_0_CFG_PLL_SMPL_PRD_FIELD_WIDTH,
    CLK_DEJITTER_SAMPLING_CTL_0_CFG_PLL_SMPL_PRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_DEJITTER_SAMPLING_CTL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec CLK_DEJITTER_SAMPLING_CTL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLK_DEJITTER_SAMPLING_CTL_1_RESERVED0_FIELD_MASK,
    0,
    CLK_DEJITTER_SAMPLING_CTL_1_RESERVED0_FIELD_WIDTH,
    CLK_DEJITTER_SAMPLING_CTL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_DEJITTER_SAMPLING_CTL_1_CFG_EN_PBI_WR_2_SYNCE_PLL
 ******************************************************************************/
const ru_field_rec CLK_DEJITTER_SAMPLING_CTL_1_CFG_EN_PBI_WR_2_SYNCE_PLL_FIELD =
{
    "CFG_EN_PBI_WR_2_SYNCE_PLL",
#if RU_INCLUDE_DESC
    "",
    "Enable PBI write to SyncE_PLL integer/fractional dividers.",
#endif
    CLK_DEJITTER_SAMPLING_CTL_1_CFG_EN_PBI_WR_2_SYNCE_PLL_FIELD_MASK,
    0,
    CLK_DEJITTER_SAMPLING_CTL_1_CFG_EN_PBI_WR_2_SYNCE_PLL_FIELD_WIDTH,
    CLK_DEJITTER_SAMPLING_CTL_1_CFG_EN_PBI_WR_2_SYNCE_PLL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_DEJITTER_SAMPLING_CTL_1_RESERVED1
 ******************************************************************************/
const ru_field_rec CLK_DEJITTER_SAMPLING_CTL_1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLK_DEJITTER_SAMPLING_CTL_1_RESERVED1_FIELD_MASK,
    0,
    CLK_DEJITTER_SAMPLING_CTL_1_RESERVED1_FIELD_WIDTH,
    CLK_DEJITTER_SAMPLING_CTL_1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_DEJITTER_SAMPLING_CTL_1_CFG_CLK_SMPL_SRC
 ******************************************************************************/
const ru_field_rec CLK_DEJITTER_SAMPLING_CTL_1_CFG_CLK_SMPL_SRC_FIELD =
{
    "CFG_CLK_SMPL_SRC",
#if RU_INCLUDE_DESC
    "",
    "Specifies the source of the sample pulse generator : 0 - PON_SERDES;"
    "1 - LAN_SERDES; 2 - SGPHY; 3 - DSL; 4 - NTR.  This samples the"
    "counter running in SyncE_PLL's 250 MHz clock domain.",
#endif
    CLK_DEJITTER_SAMPLING_CTL_1_CFG_CLK_SMPL_SRC_FIELD_MASK,
    0,
    CLK_DEJITTER_SAMPLING_CTL_1_CFG_CLK_SMPL_SRC_FIELD_WIDTH,
    CLK_DEJITTER_SAMPLING_CTL_1_CFG_CLK_SMPL_SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_SAMPLE_COUNTER_PBI_CLK_CNT_SMPL
 ******************************************************************************/
const ru_field_rec CLK_SAMPLE_COUNTER_PBI_CLK_CNT_SMPL_FIELD =
{
    "PBI_CLK_CNT_SMPL",
#if RU_INCLUDE_DESC
    "",
    "Sample clock counter value of SyncE_PLL's dejittering counter."
    "Value should be read upon"
    "receiving interrupt clk_sample_int. Difference in time is obtained"
    "by subtracting current"
    "value from previous.  The value will be different for the different"
    "sampling sources.",
#endif
    CLK_SAMPLE_COUNTER_PBI_CLK_CNT_SMPL_FIELD_MASK,
    0,
    CLK_SAMPLE_COUNTER_PBI_CLK_CNT_SMPL_FIELD_WIDTH,
    CLK_SAMPLE_COUNTER_PBI_CLK_CNT_SMPL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: CLK_DEJITTER_SAMPLING_CTL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLK_DEJITTER_SAMPLING_CTL_0_FIELDS[] =
{
    &CLK_DEJITTER_SAMPLING_CTL_0_RESERVED0_FIELD,
    &CLK_DEJITTER_SAMPLING_CTL_0_CFG_PLL_SMPL_PRD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLK_DEJITTER_SAMPLING_CTL_0_REG = 
{
    "DEJITTER_SAMPLING_CTL_0",
#if RU_INCLUDE_DESC
    "WAN_CLK_DEJITTER_SAMPLING_CTL_0 Register",
    "Clock dejittering control register.",
#endif
    CLK_DEJITTER_SAMPLING_CTL_0_REG_OFFSET,
    0,
    0,
    39,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CLK_DEJITTER_SAMPLING_CTL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: CLK_DEJITTER_SAMPLING_CTL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLK_DEJITTER_SAMPLING_CTL_1_FIELDS[] =
{
    &CLK_DEJITTER_SAMPLING_CTL_1_RESERVED0_FIELD,
    &CLK_DEJITTER_SAMPLING_CTL_1_CFG_EN_PBI_WR_2_SYNCE_PLL_FIELD,
    &CLK_DEJITTER_SAMPLING_CTL_1_RESERVED1_FIELD,
    &CLK_DEJITTER_SAMPLING_CTL_1_CFG_CLK_SMPL_SRC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLK_DEJITTER_SAMPLING_CTL_1_REG = 
{
    "DEJITTER_SAMPLING_CTL_1",
#if RU_INCLUDE_DESC
    "WAN_CLK_DEJITTER_SAMPLING_CTL_1 Register",
    "Clock dejittering control register.",
#endif
    CLK_DEJITTER_SAMPLING_CTL_1_REG_OFFSET,
    0,
    0,
    40,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CLK_DEJITTER_SAMPLING_CTL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: CLK_SAMPLE_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLK_SAMPLE_COUNTER_FIELDS[] =
{
    &CLK_SAMPLE_COUNTER_PBI_CLK_CNT_SMPL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLK_SAMPLE_COUNTER_REG = 
{
    "SAMPLE_COUNTER",
#if RU_INCLUDE_DESC
    "WAN_CLK_SAMPLE_COUNTER Register",
    "Clock counter sample register.",
#endif
    CLK_SAMPLE_COUNTER_REG_OFFSET,
    0,
    0,
    41,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CLK_SAMPLE_COUNTER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: CLK
 ******************************************************************************/
static const ru_reg_rec *CLK_REGS[] =
{
    &CLK_DEJITTER_SAMPLING_CTL_0_REG,
    &CLK_DEJITTER_SAMPLING_CTL_1_REG,
    &CLK_SAMPLE_COUNTER_REG,
};

unsigned long CLK_ADDRS[] =
{
    0x801440a0,
};

const ru_block_rec CLK_BLOCK = 
{
    "CLK",
    CLK_ADDRS,
    1,
    3,
    CLK_REGS
};

/* End of file CLK.c */
