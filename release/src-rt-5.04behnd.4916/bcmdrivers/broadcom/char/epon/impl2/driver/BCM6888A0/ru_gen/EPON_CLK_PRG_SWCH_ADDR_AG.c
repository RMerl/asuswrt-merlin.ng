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
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKSEL
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKSEL_FIELD =
{
    "CFGPRGCLKSEL",
#if RU_INCLUDE_DESC
    "",
    "Input select value for programmable clock output."
    "0: epnClk125    (125 MHz MAC clock, default)"
    "1: epnClkRbc125 (125 MHz receive clock)"
    "2: main_clk    (500 MHz from RDP)"
    "3: dev_clk    (250 MHz from UBUS)"
    "4. clk50_cnt_g  ( 50 MHz from XTAL)"
    "5 - 7: reserved",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKSEL_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKSEL_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_RESERVED0_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_RESERVED0_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKDIVIDE
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKDIVIDE_FIELD =
{
    "CFGPRGCLKDIVIDE",
#if RU_INCLUDE_DESC
    "",
    "Divider value for programmable clock output. The input for the"
    "programmable clock is selected by cfgPrgClkSel, below. The output"
    "frequency of the programmable clock is as follows:"
    "0: Pass through selected clock"
    "n >= cfgPrgClkDenom : numerator of the fractional divider"
    "The divide ratio = cfgPrgClkDivide/ cfgPrgClkDenom",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKDIVIDE_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKDIVIDE_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKDIVIDE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_RESERVED0
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_RESERVED0_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_RESERVED0_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_CFGPRGCLKDENOM
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_CFGPRGCLKDENOM_FIELD =
{
    "CFGPRGCLKDENOM",
#if RU_INCLUDE_DESC
    "",
    "This value serves as the denominator of the factional divide.  The"
    "default is 0x1000 which provides a 4096 value.",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_CFGPRGCLKDENOM_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_CFGPRGCLKDENOM_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_CFGPRGCLKDENOM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKSEL_1
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKSEL_1_FIELD =
{
    "CFGPRGCLKSEL_1",
#if RU_INCLUDE_DESC
    "",
    "Input select value for programmable clock output."
    "0: epnClk125    (125 MHz MAC clock, default)"
    "1: epnClkRbc125 (125 MHz receive clock)"
    "2: main_clk    (500 MHz from RDP)"
    "3: dev_clk    (250 MHz from UBUS)"
    "4. clk50_cnt_g  ( 50 MHz from XTAL)"
    "5 - 7: reserved",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKSEL_1_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKSEL_1_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKSEL_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_RESERVED0
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_RESERVED0_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_RESERVED0_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKDIVIDE_1
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKDIVIDE_1_FIELD =
{
    "CFGPRGCLKDIVIDE_1",
#if RU_INCLUDE_DESC
    "",
    "Divider value for programmable clock output. The input for the"
    "programmable clock is selected by cfgPrgClkSel, below. The output"
    "frequency of the programmable clock is as follows:"
    "0: Pass through selected clock"
    "n >= cfgPrgClkDenom : numerator of the fractional divider"
    "The divide ratio = cfgPrgClkDivide/ cfgPrgClkDenom",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKDIVIDE_1_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKDIVIDE_1_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKDIVIDE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_RESERVED0
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_RESERVED0_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_RESERVED0_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_CFGPRGCLKDENOM_1
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_CFGPRGCLKDENOM_1_FIELD =
{
    "CFGPRGCLKDENOM_1",
#if RU_INCLUDE_DESC
    "",
    "This value serves as the denominator of the factional divide.  The"
    "default is 0x1000 which provides a 4096 value.",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_CFGPRGCLKDENOM_1_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_CFGPRGCLKDENOM_1_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_CFGPRGCLKDENOM_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSOFFSET
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSOFFSET_FIELD =
{
    "CFGPPSOFFSET",
#if RU_INCLUDE_DESC
    "",
    "This value adjusts the phase offset of the 1PPS to the programmable"
    "clock output.",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSOFFSET_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSOFFSET_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKGAIN
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKGAIN_FIELD =
{
    "CFGPPSTRACKGAIN",
#if RU_INCLUDE_DESC
    "",
    "This value adjusts step size of the programmable clock phase"
    "adjustment. If this is programmed too low the tracking may not be"
    "able to keep up.  If it is set too high, the clock may have too much"
    "jitter.  The value of 2^gain should not be greater than the divided"
    "clock period.",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKGAIN_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKGAIN_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKGAIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_RESERVED0
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_RESERVED0_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_RESERVED0_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGOUTCLKINV
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGOUTCLKINV_FIELD =
{
    "CFGOUTCLKINV",
#if RU_INCLUDE_DESC
    "",
    "This value inverts the programmable clock output.",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGOUTCLKINV_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGOUTCLKINV_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGOUTCLKINV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKEN
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKEN_FIELD =
{
    "CFGPPSTRACKEN",
#if RU_INCLUDE_DESC
    "",
    "This value adjusts the period of the programmable clock to track the"
    "1PPS.  This provides a step to the programmable clock phase to track"
    "the 1PPS.  This reduces the output jitter of the clock by making"
    "small adjustments.",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKEN_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKEN_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSALIGNEN
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSALIGNEN_FIELD =
{
    "CFGPPSALIGNEN",
#if RU_INCLUDE_DESC
    "",
    "This value will reset the programmable clock on the rising edge of"
    "1PPS. This should be done intialy to align the clock to the 1PPS."
    "Setting this all the time will increase the jitter of the output"
    "clock, but maintain a constanct output phase of the 1PPS relative to"
    "the clock edge.",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSALIGNEN_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSALIGNEN_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSALIGNEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSSELECT
 ******************************************************************************/
const ru_field_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSSELECT_FIELD =
{
    "CFGPPSSELECT",
#if RU_INCLUDE_DESC
    "",
    "This value selects the source of the 1PPS input 0-1Gbps, 1-10Gbps",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSSELECT_FIELD_MASK,
    0,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSSELECT_FIELD_WIDTH,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSSELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_FIELDS[] =
{
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKSEL_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_RESERVED0_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_CFGPRGCLKDIVIDE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_REG = 
{
    "CLK_PRG_CONFIG",
#if RU_INCLUDE_DESC
    "ADDR_CLK_PRG_CONFIG Register",
    "The register controls the Programmable Divider (ProgDiv) function."
    "Note: Writing this register has the side-effect of clearing the"
    "programmable clock divider circuit."
    "This allows for near-instantaneous updates of the clock divider output"
    "frequency."
    ""
    "Table of Useful Divider Values"
    "Input Clock Freq. (MHz) Desired Output Frequency Divider"
    "cfgPrgClkDivide value (hex) cfgPrgClkDenom value (hex)"
    "125      10 MHz   12.5"
    "0xC800       0x1000"
    "125      8 KHz    15625"
    "0x3D09000       0x1000",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_REG_OFFSET,
    0,
    0,
    14,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_FIELDS[] =
{
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_RESERVED0_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_CFGPRGCLKDENOM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_REG = 
{
    "CLK_PRG_CONFIG2",
#if RU_INCLUDE_DESC
    "ADDR_CLK_PRG_CONFIG2 Register",
    "",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_REG_OFFSET,
    0,
    0,
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_FIELDS[] =
{
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKSEL_1_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_RESERVED0_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_CFGPRGCLKDIVIDE_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_REG = 
{
    "CLK_PRG_CONFIG_1",
#if RU_INCLUDE_DESC
    "ADDR_CLK_PRG_CONFIG_1 Register",
    "The register controls the Programmable Divider (ProgDiv) function."
    "Note: Writing this register has the side-effect of clearing the"
    "programmable clock divider circuit."
    "This allows for near-instantaneous updates of the clock divider output"
    "frequency."
    ""
    "Table of Useful Divider Values"
    "Input Clock Freq. (MHz) Desired Output Frequency Divider"
    "cfgPrgClkDivide value (hex) cfgPrgClkDenom value (hex)"
    "125      10 MHz   12.5"
    "0xC800       0x1000"
    "125      8 KHz    15625"
    "0x3D09000       0x1000",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_REG_OFFSET,
    0,
    0,
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_FIELDS[] =
{
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_RESERVED0_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_CFGPRGCLKDENOM_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_REG = 
{
    "CLK_PRG_CONFIG2_1",
#if RU_INCLUDE_DESC
    "ADDR_CLK_PRG_CONFIG2_1 Register",
    "",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_REG_OFFSET,
    0,
    0,
    17,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_FIELDS[] =
{
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSOFFSET_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKGAIN_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_RESERVED0_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGOUTCLKINV_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSTRACKEN_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSALIGNEN_FIELD,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_CFGPPSSELECT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_REG = 
{
    "CLK_PRG_CONFIG3",
#if RU_INCLUDE_DESC
    "ADDR_CLK_PRG_CONFIG3 Register",
    "",
#endif
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_REG_OFFSET,
    0,
    0,
    18,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: CLK_PRG_SWCH_ADDR
 ******************************************************************************/
static const ru_reg_rec *CLK_PRG_SWCH_ADDR_REGS[] =
{
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_REG,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_REG,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG_1_REG,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG2_1_REG,
    &CLK_PRG_SWCH_ADDR_CLK_PRG_CONFIG3_REG,
};

static unsigned long CLK_PRG_SWCH_ADDR_ADDRS[] =
{
    0x80100800,
};

const ru_block_rec CLK_PRG_SWCH_ADDR_BLOCK = 
{
    "CLK_PRG_SWCH_ADDR",
    CLK_PRG_SWCH_ADDR_ADDRS,
    1,
    5,
    CLK_PRG_SWCH_ADDR_REGS
};

/* End of file EPON_CLK_PRG_SWCH_ADDR.c */
