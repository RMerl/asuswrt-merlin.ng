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
 * Field: NCO_ADDR_NCO_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_CFG_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_CFG_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_CFG_CFGBYPASS
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_CFG_CFGBYPASS_FIELD =
{
    "CFGBYPASS",
#if RU_INCLUDE_DESC
    "",
    "Bypass the programmable duty cycle when cfgSrcOut is set to 1 or 2"
    "(direct Lif/Xif pass through)."
    "Default : 1",
#endif
    NCO_ADDR_NCO_CFG_CFGBYPASS_FIELD_MASK,
    0,
    NCO_ADDR_NCO_CFG_CFGBYPASS_FIELD_WIDTH,
    NCO_ADDR_NCO_CFG_CFGBYPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_CFG_CFGSRCOUT10MHZ
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_CFG_CFGSRCOUT10MHZ_FIELD =
{
    "CFGSRCOUT10MHZ",
#if RU_INCLUDE_DESC
    "",
    "Selects the 10MHz output source"
    "0: NCO drives 10MHz output"
    "1: Lif 10MHz drives 10MHz output"
    "2: Reserved"
    "3: Reserved",
#endif
    NCO_ADDR_NCO_CFG_CFGSRCOUT10MHZ_FIELD_MASK,
    0,
    NCO_ADDR_NCO_CFG_CFGSRCOUT10MHZ_FIELD_WIDTH,
    NCO_ADDR_NCO_CFG_CFGSRCOUT10MHZ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_CFG_CFGSRCOUT
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_CFG_CFGSRCOUT_FIELD =
{
    "CFGSRCOUT",
#if RU_INCLUDE_DESC
    "",
    "Selects the one PPS output source."
    "0: NCO drives one PPS output"
    "1: lifPpsSig drives one PPS output"
    "2: xifPpsSig drives one PPS output"
    "3: Output is zero"
    "The output source should be set to the Lif or Xif input until the"
    "NCO converges. Only then should the NCO output be selected as the"
    "one PPS source.",
#endif
    NCO_ADDR_NCO_CFG_CFGSRCOUT_FIELD_MASK,
    0,
    NCO_ADDR_NCO_CFG_CFGSRCOUT_FIELD_WIDTH,
    NCO_ADDR_NCO_CFG_CFGSRCOUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_CFG_CFGSRCIN
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_CFG_CFGSRCIN_FIELD =
{
    "CFGSRCIN",
#if RU_INCLUDE_DESC
    "",
    "Selects the input reference source."
    "0: NCO \"free runs\" at 125 MHz core clock."
    "1: NCO tracks lifPpsSig"
    "2: NCO tracks xifPpsSig"
    "3: Reserved",
#endif
    NCO_ADDR_NCO_CFG_CFGSRCIN_FIELD_MASK,
    0,
    NCO_ADDR_NCO_CFG_CFGSRCIN_FIELD_WIDTH,
    NCO_ADDR_NCO_CFG_CFGSRCIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_CFG_CFGNCOCLR
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_CFG_CFGNCOCLR_FIELD =
{
    "CFGNCOCLR",
#if RU_INCLUDE_DESC
    "",
    "Set this bit to reset the NCO logic."
    "Note: This does not reset the NCO configuration registers."
    "0: Normal NCO operation"
    "1: Hold the NCO logic in reset"
    "Default: 1",
#endif
    NCO_ADDR_NCO_CFG_CFGNCOCLR_FIELD_MASK,
    0,
    NCO_ADDR_NCO_CFG_CFGNCOCLR_FIELD_WIDTH,
    NCO_ADDR_NCO_CFG_CFGNCOCLR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_INT_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_INT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_INT_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_INT_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_INT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_INT_INTNONCOSYNC
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_INT_INTNONCOSYNC_FIELD =
{
    "INTNONCOSYNC",
#if RU_INCLUDE_DESC
    "",
    "The NCO has not detected a 1pps input edge within +/-1us of the NCO"
    "generated edge.",
#endif
    NCO_ADDR_NCO_INT_INTNONCOSYNC_FIELD_MASK,
    0,
    NCO_ADDR_NCO_INT_INTNONCOSYNC_FIELD_WIDTH,
    NCO_ADDR_NCO_INT_INTNONCOSYNC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_INT_INTNOXIFPPS
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_INT_INTNOXIFPPS_FIELD =
{
    "INTNOXIFPPS",
#if RU_INCLUDE_DESC
    "",
    "No edges of the Xif 1PPS signal have been detected over 2 NCO"
    "periods.",
#endif
    NCO_ADDR_NCO_INT_INTNOXIFPPS_FIELD_MASK,
    0,
    NCO_ADDR_NCO_INT_INTNOXIFPPS_FIELD_WIDTH,
    NCO_ADDR_NCO_INT_INTNOXIFPPS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_INT_INTNOLIFPPS
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_INT_INTNOLIFPPS_FIELD =
{
    "INTNOLIFPPS",
#if RU_INCLUDE_DESC
    "",
    "No edges of the Lif 1PPS signal have been detected over 2 NCO"
    "periods.",
#endif
    NCO_ADDR_NCO_INT_INTNOLIFPPS_FIELD_MASK,
    0,
    NCO_ADDR_NCO_INT_INTNOLIFPPS_FIELD_WIDTH,
    NCO_ADDR_NCO_INT_INTNOLIFPPS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_MSK_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_MSK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_MSK_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_MSK_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_MSK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_MSK_INTNONCOSYNCMASK
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_MSK_INTNONCOSYNCMASK_FIELD =
{
    "INTNONCOSYNCMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for intNoNcoSync",
#endif
    NCO_ADDR_NCO_MSK_INTNONCOSYNCMASK_FIELD_MASK,
    0,
    NCO_ADDR_NCO_MSK_INTNONCOSYNCMASK_FIELD_WIDTH,
    NCO_ADDR_NCO_MSK_INTNONCOSYNCMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_MSK_INTNOXIFPPSMASK
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_MSK_INTNOXIFPPSMASK_FIELD =
{
    "INTNOXIFPPSMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for intNoXifPps",
#endif
    NCO_ADDR_NCO_MSK_INTNOXIFPPSMASK_FIELD_MASK,
    0,
    NCO_ADDR_NCO_MSK_INTNOXIFPPSMASK_FIELD_WIDTH,
    NCO_ADDR_NCO_MSK_INTNOXIFPPSMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_MSK_INTNOLIFPPSMASK
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_MSK_INTNOLIFPPSMASK_FIELD =
{
    "INTNOLIFPPSMASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for intNoLifPps",
#endif
    NCO_ADDR_NCO_MSK_INTNOLIFPPSMASK_FIELD_MASK,
    0,
    NCO_ADDR_NCO_MSK_INTNOLIFPPSMASK_FIELD_WIDTH,
    NCO_ADDR_NCO_MSK_INTNOLIFPPSMASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_1PPS_PERIOD_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_1PPS_PERIOD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_1PPS_PERIOD_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_1PPS_PERIOD_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_1PPS_PERIOD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_1PPS_PERIOD_CFG1PPSPERIOD
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_1PPS_PERIOD_CFG1PPSPERIOD_FIELD =
{
    "CFG1PPSPERIOD",
#if RU_INCLUDE_DESC
    "",
    "This register sets the period of the 1PPS signal, in units of 100"
    "ns."
    "Default: 10,000,000",
#endif
    NCO_ADDR_NCO_1PPS_PERIOD_CFG1PPSPERIOD_FIELD_MASK,
    0,
    NCO_ADDR_NCO_1PPS_PERIOD_CFG1PPSPERIOD_FIELD_WIDTH,
    NCO_ADDR_NCO_1PPS_PERIOD_CFG1PPSPERIOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_8KHZ_PERIOD_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_8KHZ_PERIOD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_8KHZ_PERIOD_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_8KHZ_PERIOD_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_8KHZ_PERIOD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_8KHZ_PERIOD_CFG8KHZPERIOD
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_8KHZ_PERIOD_CFG8KHZPERIOD_FIELD =
{
    "CFG8KHZPERIOD",
#if RU_INCLUDE_DESC
    "",
    "This register sets the period of the 8 KHz signal, in units of 100"
    "ns."
    "Default: 1,250",
#endif
    NCO_ADDR_NCO_8KHZ_PERIOD_CFG8KHZPERIOD_FIELD_MASK,
    0,
    NCO_ADDR_NCO_8KHZ_PERIOD_CFG8KHZPERIOD_FIELD_WIDTH,
    NCO_ADDR_NCO_8KHZ_PERIOD_CFG8KHZPERIOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_CENTER_FREQUENCY_CFGNCODEFAULT
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_CENTER_FREQUENCY_CFGNCODEFAULT_FIELD =
{
    "CFGNCODEFAULT",
#if RU_INCLUDE_DESC
    "",
    "Initial NCO period integral"
    "Default is 343,597,394.  The NCO Period Count register may be read"
    "to align the input frequency with the default to speed up locking"
    "time.",
#endif
    NCO_ADDR_NCO_CENTER_FREQUENCY_CFGNCODEFAULT_FIELD_MASK,
    0,
    NCO_ADDR_NCO_CENTER_FREQUENCY_CFGNCODEFAULT_FIELD_WIDTH,
    NCO_ADDR_NCO_CENTER_FREQUENCY_CFGNCODEFAULT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_INT_GAIN_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_INT_GAIN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_INT_GAIN_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_INT_GAIN_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_INT_GAIN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_INT_GAIN_CFGNCOGAIN
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_INT_GAIN_CFGNCOGAIN_FIELD =
{
    "CFGNCOGAIN",
#if RU_INCLUDE_DESC
    "",
    "NCO integral gain in number of 0.001 ppb quanta."
    "The gain may be increased for faster convergence, and then decreased"
    "for increased accuracy and holdover."
    "Default is 0x400.",
#endif
    NCO_ADDR_NCO_INT_GAIN_CFGNCOGAIN_FIELD_MASK,
    0,
    NCO_ADDR_NCO_INT_GAIN_CFGNCOGAIN_FIELD_WIDTH,
    NCO_ADDR_NCO_INT_GAIN_CFGNCOGAIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_PRO_GAIN_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_PRO_GAIN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_PRO_GAIN_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_PRO_GAIN_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_PRO_GAIN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_PRO_GAIN_CFGNCOPROPGAIN
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_PRO_GAIN_CFGNCOPROPGAIN_FIELD =
{
    "CFGNCOPROPGAIN",
#if RU_INCLUDE_DESC
    "",
    "NCO proportional gain in number of 0.2 ppb quanta."
    "The gain may be increased for faster convergence, and then decreased"
    "for increased accuracy and holdover."
    "Default is 0x400",
#endif
    NCO_ADDR_NCO_PRO_GAIN_CFGNCOPROPGAIN_FIELD_MASK,
    0,
    NCO_ADDR_NCO_PRO_GAIN_CFGNCOPROPGAIN_FIELD_WIDTH,
    NCO_ADDR_NCO_PRO_GAIN_CFGNCOPROPGAIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_CNT_NCOCNT
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_CNT_NCOCNT_FIELD =
{
    "NCOCNT",
#if RU_INCLUDE_DESC
    "",
    "Current NCO period integral value.",
#endif
    NCO_ADDR_NCO_CNT_NCOCNT_FIELD_MASK,
    0,
    NCO_ADDR_NCO_CNT_NCOCNT_FIELD_WIDTH,
    NCO_ADDR_NCO_CNT_NCOCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_1PPS_HALF_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_1PPS_HALF_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_1PPS_HALF_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_1PPS_HALF_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_1PPS_HALF_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_1PPS_HALF_CFG1PPSHALFPERIOD
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_1PPS_HALF_CFG1PPSHALFPERIOD_FIELD =
{
    "CFG1PPSHALFPERIOD",
#if RU_INCLUDE_DESC
    "",
    "This register sets the portion of the 1PPS period that the signal is"
    "high."
    "This value should be set to the duty cycle high %% times the"
    "cfg1ppsPeriod  (e.g. 10%% * 10,000,000 = 1,000,000)."
    "Default is 5,000,000 (50%% duty cycle).",
#endif
    NCO_ADDR_NCO_1PPS_HALF_CFG1PPSHALFPERIOD_FIELD_MASK,
    0,
    NCO_ADDR_NCO_1PPS_HALF_CFG1PPSHALFPERIOD_FIELD_WIDTH,
    NCO_ADDR_NCO_1PPS_HALF_CFG1PPSHALFPERIOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_8KHZ_HALF_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_8KHZ_HALF_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_8KHZ_HALF_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_8KHZ_HALF_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_8KHZ_HALF_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_8KHZ_HALF_CFG8KHZHALFPERIOD
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_8KHZ_HALF_CFG8KHZHALFPERIOD_FIELD =
{
    "CFG8KHZHALFPERIOD",
#if RU_INCLUDE_DESC
    "",
    "This register sets the portion of the 8 KHz period that the signal"
    "is high."
    "Reset default is 625 (50/50 duty cycle).",
#endif
    NCO_ADDR_NCO_8KHZ_HALF_CFG8KHZHALFPERIOD_FIELD_MASK,
    0,
    NCO_ADDR_NCO_8KHZ_HALF_CFG8KHZHALFPERIOD_FIELD_WIDTH,
    NCO_ADDR_NCO_8KHZ_HALF_CFG8KHZHALFPERIOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_PERIOD_CNT_PERIODCNT
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_PERIOD_CNT_PERIODCNT_FIELD =
{
    "PERIODCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of 8ns clocks in one input period.  This register will"
    "read zero until the first period is complete."
    "It will be updated on each subsequent period.  This value may be"
    "used to program the NCO default period for faster lock time."
    "For a 1PPS, the cfgNcoDefault should equal 343,597,394 *"
    "125M/periodCnt.",
#endif
    NCO_ADDR_NCO_PERIOD_CNT_PERIODCNT_FIELD_MASK,
    0,
    NCO_ADDR_NCO_PERIOD_CNT_PERIODCNT_FIELD_WIDTH,
    NCO_ADDR_NCO_PERIOD_CNT_PERIODCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_PHS_ERR_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_PHS_ERR_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NCO_ADDR_NCO_PHS_ERR_CNT_RESERVED0_FIELD_MASK,
    0,
    NCO_ADDR_NCO_PHS_ERR_CNT_RESERVED0_FIELD_WIDTH,
    NCO_ADDR_NCO_PHS_ERR_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: NCO_ADDR_NCO_PHS_ERR_CNT_NCOPHSERR
 ******************************************************************************/
const ru_field_rec NCO_ADDR_NCO_PHS_ERR_CNT_NCOPHSERR_FIELD =
{
    "NCOPHSERR",
#if RU_INCLUDE_DESC
    "",
    "The number of 8 ns clocks between the ppsNco rising edge and the LIF"
    "PPS input signal rising edge."
    "This is an up/down counter: LIF PPS leading represents a positive"
    "error, and ppsNco input leading represents a negative error."
    "The error will saturate at 0x7ff on a positive error and 0x800 on a"
    "negative error."
    "This register is updated for each rising edge sample following the"
    "first negative edge of ppsNco at start up.",
#endif
    NCO_ADDR_NCO_PHS_ERR_CNT_NCOPHSERR_FIELD_MASK,
    0,
    NCO_ADDR_NCO_PHS_ERR_CNT_NCOPHSERR_FIELD_WIDTH,
    NCO_ADDR_NCO_PHS_ERR_CNT_NCOPHSERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: NCO_ADDR_NCO_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_CFG_FIELDS[] =
{
    &NCO_ADDR_NCO_CFG_RESERVED0_FIELD,
    &NCO_ADDR_NCO_CFG_CFGBYPASS_FIELD,
    &NCO_ADDR_NCO_CFG_CFGSRCOUT10MHZ_FIELD,
    &NCO_ADDR_NCO_CFG_CFGSRCOUT_FIELD,
    &NCO_ADDR_NCO_CFG_CFGSRCIN_FIELD,
    &NCO_ADDR_NCO_CFG_CFGNCOCLR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_CFG_REG = 
{
    "NCO_CFG",
#if RU_INCLUDE_DESC
    "ADDR_NCO_CFG Register",
    "This register is used to provision the Numerically Controlled"
    "Oscillator (NCO) function.",
#endif
    NCO_ADDR_NCO_CFG_REG_OFFSET,
    0,
    0,
    525,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    NCO_ADDR_NCO_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_INT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_INT_FIELDS[] =
{
    &NCO_ADDR_NCO_INT_RESERVED0_FIELD,
    &NCO_ADDR_NCO_INT_INTNONCOSYNC_FIELD,
    &NCO_ADDR_NCO_INT_INTNOXIFPPS_FIELD,
    &NCO_ADDR_NCO_INT_INTNOLIFPPS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_INT_REG = 
{
    "NCO_INT",
#if RU_INCLUDE_DESC
    "ADDR_NCO_INT Register",
    "This register is used to detect the presence and synchronization lock"
    "of the 1PPS signals."
    "These bits are sticky; to clear a bit, write 1 to it.",
#endif
    NCO_ADDR_NCO_INT_REG_OFFSET,
    0,
    0,
    526,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NCO_ADDR_NCO_INT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_MSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_MSK_FIELDS[] =
{
    &NCO_ADDR_NCO_MSK_RESERVED0_FIELD,
    &NCO_ADDR_NCO_MSK_INTNONCOSYNCMASK_FIELD,
    &NCO_ADDR_NCO_MSK_INTNOXIFPPSMASK_FIELD,
    &NCO_ADDR_NCO_MSK_INTNOLIFPPSMASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_MSK_REG = 
{
    "NCO_MSK",
#if RU_INCLUDE_DESC
    "ADDR_NCO_MSK Register",
    "This register is used to mask NCO interrupts.",
#endif
    NCO_ADDR_NCO_MSK_REG_OFFSET,
    0,
    0,
    527,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NCO_ADDR_NCO_MSK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_1PPS_PERIOD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_1PPS_PERIOD_FIELDS[] =
{
    &NCO_ADDR_NCO_1PPS_PERIOD_RESERVED0_FIELD,
    &NCO_ADDR_NCO_1PPS_PERIOD_CFG1PPSPERIOD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_1PPS_PERIOD_REG = 
{
    "NCO_1PPS_PERIOD",
#if RU_INCLUDE_DESC
    "ADDR_NCO_1PPS_PERIOD Register",
    "This register is used to provision the NCO's one pulse per second"
    "(1PPS) period.",
#endif
    NCO_ADDR_NCO_1PPS_PERIOD_REG_OFFSET,
    0,
    0,
    528,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NCO_ADDR_NCO_1PPS_PERIOD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_8KHZ_PERIOD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_8KHZ_PERIOD_FIELDS[] =
{
    &NCO_ADDR_NCO_8KHZ_PERIOD_RESERVED0_FIELD,
    &NCO_ADDR_NCO_8KHZ_PERIOD_CFG8KHZPERIOD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_8KHZ_PERIOD_REG = 
{
    "NCO_8KHZ_PERIOD",
#if RU_INCLUDE_DESC
    "ADDR_NCO_8KHZ_PERIOD Register",
    "This register is used to provision the NCO's 8 KHz period.",
#endif
    NCO_ADDR_NCO_8KHZ_PERIOD_REG_OFFSET,
    0,
    0,
    529,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NCO_ADDR_NCO_8KHZ_PERIOD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_CENTER_FREQUENCY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_CENTER_FREQUENCY_FIELDS[] =
{
    &NCO_ADDR_NCO_CENTER_FREQUENCY_CFGNCODEFAULT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_CENTER_FREQUENCY_REG = 
{
    "NCO_CENTER_FREQUENCY",
#if RU_INCLUDE_DESC
    "ADDR_NCO_CENTER_FREQUENCY Register",
    "This register is used to provision the NCO's initial period integral"
    "value."
    "The reset default is calculated as (8nS/100nS)*(2^32) => 343,597,394",
#endif
    NCO_ADDR_NCO_CENTER_FREQUENCY_REG_OFFSET,
    0,
    0,
    530,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NCO_ADDR_NCO_CENTER_FREQUENCY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_INT_GAIN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_INT_GAIN_FIELDS[] =
{
    &NCO_ADDR_NCO_INT_GAIN_RESERVED0_FIELD,
    &NCO_ADDR_NCO_INT_GAIN_CFGNCOGAIN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_INT_GAIN_REG = 
{
    "NCO_INT_GAIN",
#if RU_INCLUDE_DESC
    "ADDR_NCO_INT_GAIN Register",
    "This register is used to provision the NCO's integral gain value."
    "The provisioned value must be within the range of 15ppm."
    "The value is in 0.001 ppb units",
#endif
    NCO_ADDR_NCO_INT_GAIN_REG_OFFSET,
    0,
    0,
    531,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NCO_ADDR_NCO_INT_GAIN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_PRO_GAIN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_PRO_GAIN_FIELDS[] =
{
    &NCO_ADDR_NCO_PRO_GAIN_RESERVED0_FIELD,
    &NCO_ADDR_NCO_PRO_GAIN_CFGNCOPROPGAIN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_PRO_GAIN_REG = 
{
    "NCO_PRO_GAIN",
#if RU_INCLUDE_DESC
    "ADDR_NCO_PRO_GAIN Register",
    "This register is used to provision the NCO's initial period integral"
    "value."
    "The reset default is calculated as (8nS/100nS)*(2^32) => 343,597,394",
#endif
    NCO_ADDR_NCO_PRO_GAIN_REG_OFFSET,
    0,
    0,
    532,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NCO_ADDR_NCO_PRO_GAIN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_CNT_FIELDS[] =
{
    &NCO_ADDR_NCO_CNT_NCOCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_CNT_REG = 
{
    "NCO_CNT",
#if RU_INCLUDE_DESC
    "ADDR_NCO_CNT Register",
    "The value in this register is the NCO's current period integral."
    "The value is in xxx units."
    "(This value reflects the relationship between the accuracy of the"
    "reference clock frequency and the accuracy of the 125 MHz core clock"
    "frequency."
    "The closer this value is to the ideal value calculated for the NCO"
    "Initial Period Integral Value in register 0x0c5,"
    "the closer the core 125 MHz frequency error matches the reference"
    "clock's frequency error.)"
    ""
    "Note: Once the system has locked to a valid downstream reference and"
    "reached steady state,"
    "the value in this register can be transferred to the \"NCO Initial"
    "Period Integral Value\" in register 0x0c5."
    "This will ensure that the NCO's \"hold-over\" frequency will closely"
    "match the reference frequency.",
#endif
    NCO_ADDR_NCO_CNT_REG_OFFSET,
    0,
    0,
    533,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NCO_ADDR_NCO_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_1PPS_HALF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_1PPS_HALF_FIELDS[] =
{
    &NCO_ADDR_NCO_1PPS_HALF_RESERVED0_FIELD,
    &NCO_ADDR_NCO_1PPS_HALF_CFG1PPSHALFPERIOD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_1PPS_HALF_REG = 
{
    "NCO_1PPS_HALF",
#if RU_INCLUDE_DESC
    "ADDR_NCO_1PPS_HALF Register",
    "This register is used to set the NCO's 1PPS duty cycle."
    "The provisioned value represents the \"high time\" of the 1PPS signal and"
    "is in 100 nS units.",
#endif
    NCO_ADDR_NCO_1PPS_HALF_REG_OFFSET,
    0,
    0,
    534,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NCO_ADDR_NCO_1PPS_HALF_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_8KHZ_HALF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_8KHZ_HALF_FIELDS[] =
{
    &NCO_ADDR_NCO_8KHZ_HALF_RESERVED0_FIELD,
    &NCO_ADDR_NCO_8KHZ_HALF_CFG8KHZHALFPERIOD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_8KHZ_HALF_REG = 
{
    "NCO_8KHZ_HALF",
#if RU_INCLUDE_DESC
    "ADDR_NCO_8KHZ_HALF Register",
    "This register is used to provision the NCO's 8 KHz duty cycle."
    "The provisioned value represents the \"high time\" of the 8 KHz signal"
    "and is in 100 nS units.",
#endif
    NCO_ADDR_NCO_8KHZ_HALF_REG_OFFSET,
    0,
    0,
    535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NCO_ADDR_NCO_8KHZ_HALF_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_PERIOD_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_PERIOD_CNT_FIELDS[] =
{
    &NCO_ADDR_NCO_PERIOD_CNT_PERIODCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_PERIOD_CNT_REG = 
{
    "NCO_PERIOD_CNT",
#if RU_INCLUDE_DESC
    "ADDR_NCO_PERIOD_CNT Register",
    "This register is used to measure the incoming clock period in terms of"
    "the local oscillator.",
#endif
    NCO_ADDR_NCO_PERIOD_CNT_REG_OFFSET,
    0,
    0,
    536,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NCO_ADDR_NCO_PERIOD_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NCO_ADDR_NCO_PHS_ERR_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NCO_ADDR_NCO_PHS_ERR_CNT_FIELDS[] =
{
    &NCO_ADDR_NCO_PHS_ERR_CNT_RESERVED0_FIELD,
    &NCO_ADDR_NCO_PHS_ERR_CNT_NCOPHSERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NCO_ADDR_NCO_PHS_ERR_CNT_REG = 
{
    "NCO_PHS_ERR_CNT",
#if RU_INCLUDE_DESC
    "ADDR_NCO_PHS_ERR_CNT Register",
    "This register is used to measure the incoming clock phase error in 8 ns"
    "units.",
#endif
    NCO_ADDR_NCO_PHS_ERR_CNT_REG_OFFSET,
    0,
    0,
    537,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NCO_ADDR_NCO_PHS_ERR_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: NCO_ADDR
 ******************************************************************************/
static const ru_reg_rec *NCO_ADDR_REGS[] =
{
    &NCO_ADDR_NCO_CFG_REG,
    &NCO_ADDR_NCO_INT_REG,
    &NCO_ADDR_NCO_MSK_REG,
    &NCO_ADDR_NCO_1PPS_PERIOD_REG,
    &NCO_ADDR_NCO_8KHZ_PERIOD_REG,
    &NCO_ADDR_NCO_CENTER_FREQUENCY_REG,
    &NCO_ADDR_NCO_INT_GAIN_REG,
    &NCO_ADDR_NCO_PRO_GAIN_REG,
    &NCO_ADDR_NCO_CNT_REG,
    &NCO_ADDR_NCO_1PPS_HALF_REG,
    &NCO_ADDR_NCO_8KHZ_HALF_REG,
    &NCO_ADDR_NCO_PERIOD_CNT_REG,
    &NCO_ADDR_NCO_PHS_ERR_CNT_REG,
};

static unsigned long NCO_ADDR_ADDRS[] =
{
    0x80142000,
};

const ru_block_rec NCO_ADDR_BLOCK = 
{
    "NCO_ADDR",
    NCO_ADDR_ADDRS,
    1,
    13,
    NCO_ADDR_REGS
};

/* End of file EPON_NCO_ADDR.c */
