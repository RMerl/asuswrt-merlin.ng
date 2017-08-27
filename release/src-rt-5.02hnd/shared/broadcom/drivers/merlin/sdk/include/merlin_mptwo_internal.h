 /*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>

*/

/** @file merlin_mptwo_internal.h
 * Internal API functions
 */

#ifndef MERLIN_MPTWO_API_INTERNAL_H
#define MERLIN_MPTWO_API_INTERNAL_H

#define SDK_STATIC static


#include "merlin_mptwo_functions.h"
#include "merlin_mptwo_common.h"
#include "merlin_mptwo_field_access.h"
#include "merlin_mptwo_ipconfig.h"
#include "common/srds_api_err_code.h"
#include "merlin_mptwo_interface.h"
#include "merlin_mptwo_debug_functions.h"

/*------------------------------*/
/** Serdes OSR Mode Structure   */
/*------------------------------*/
typedef struct {
    /** TX OSR Mode */
    uint8_t tx;
    /** RX OSR Mode */
    uint8_t rx;
    /** OSR Mode for TX and RX (used when both TX and RX should have same OSR Mode) */
    uint8_t tx_rx;
}merlin_mptwo_osr_mode_st;

/*------------------------------*/
/** Serdes Lane State Structure */
/*------------------------------*/
typedef struct {
       /** uC lane configuration */
       uint16_t ucv_config;
       /** Frequency offset of local reference clock with respect to RX data in ppm */
       int16_t rx_ppm;
       /** Vertical threshold voltage of p1 slicer (mV) */
       int16_t p1_lvl;
       /** Vertical threshold voltage of m1 slicer (mV) */
       int16_t m1_lvl;
       /** Link time in milliseconds */
       uint16_t link_time;
       /** OSR Mode */
       merlin_mptwo_osr_mode_st osr_mode;
       /** Signal Detect */
       uint8_t sig_det;
       /** Signal Detect Change */
       uint8_t sig_det_chg;
       /** PMD RX Lock */
       uint8_t rx_lock;
       /** PMD RX Lock Change */
       uint8_t rx_lock_chg;
       /** Delay of zero crossing slicer, m1, wrt to data in PI codes */
       int8_t clk90;
       /** Delay of diagnostic/lms slicer, p1, wrt to data in PI codes */
       int8_t clkp1;
       /** Peaking Filter Main Settings */
       int8_t pf_main;
       /** Peaking Filter Hiz mode enable */
       int8_t pf_hiz;
       /** Peaking Filter DC gain adjustment for CTLE */
       int8_t pf_bst;
       /** Low Frequency Peaking filter control */
       int8_t pf2_ctrl;
       /** Variable Gain Amplifier settings */
       int8_t vga;
       /** DC offset DAC control value */
       int8_t dc_offset;
       /** P1 eyediag status */
       int8_t p1_lvl_ctrl;
       /** DFE tap 1 value */
       int8_t dfe1;
       /** DFE tap 2 value */
       int8_t dfe2;
       /** DFE tap 3 value */
       int8_t dfe3;
       /** DFE tap 4 value */
       int8_t dfe4;
       /** DFE tap 5 value */
       int8_t dfe5;
       /** DFE tap 6 value */
       int8_t dfe6;
       /** DFE tap 1 Duty Cycle Distortion */
       int8_t dfe1_dcd;
       /** DFE tap 2 Duty Cycle Distortion */
       int8_t dfe2_dcd;
       /** Slicer calibration control codes (p1 even) */
       int8_t pe;
       /** Slicer calibration control codes (data even) */
       int8_t ze;
       /** Slicer calibration control codes (m1 even) */
       int8_t me;
       /** Slicer calibration control codes (p1 odd) */
       int8_t po;
       /** Slicer calibration control codes (data odd) */
       int8_t zo;
       /** Slicer calibration control codes (m1 odd) */
       int8_t mo;
       /** Frequency offset of local reference clock with respect to TX data in ppm */
       int16_t tx_ppm;
       /** TX equalization FIR pre tap weight */
       int8_t txfir_pre;
       /** TX equalization FIR main tap weight */
       int8_t txfir_main;
       /** TX equalization FIR post1 tap weight */
       int8_t txfir_post1;
       /** TX equalization FIR post2 tap weight */
       int8_t txfir_post2;
       /** TX equalization FIR post3 tap weight */
       int8_t txfir_post3;
       /** Horizontal left eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
       uint16_t heye_left;
       /** Horizontal right eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
       uint16_t heye_right;
       /** Vertical upper eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
       uint16_t veye_upper;
       /** Vertical lower eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
       uint16_t veye_lower;
       /** Baud Rate Phase Detector enable */
       uint8_t br_pd_en;
       /** lane_reset_state **/
       uint8_t reset_state;
       /** uC stopped state **/
       uint8_t stop_state;
       /** TX_AMP **/
       int8_t tx_amp;
       /** TX_Drivermode **/
       uint8_t drivermode;
       /** TX pll select */
       uint8_t tx_pll_select;
       /** RX pll select */
       uint8_t rx_pll_select;
} merlin_mptwo_lane_state_st;

/*------------------------------*/
/** Serdes Core State Structure */
/*------------------------------*/
typedef struct {
       /** Core DP Reset State */
       uint8_t  core_reset;
       /**  PLL Powerdown enable */
       uint8_t  pll_pwrdn;
       /** Micro active enable */
       uint8_t  uc_active;
       /** Comclk Frequency in Mhz */
       uint16_t comclk_mhz;
       /** uCode Major Version number */
       uint16_t ucode_version;
       /** uCode Minor Version number */
       uint8_t  ucode_minor_version;
       /** AFE Hardware version */
       uint8_t  afe_hardware_version;
       /** uC Die Temperature Index */
       uint8_t  temp_idx;
       /** Average Die Temperature (13-bit format) */
       uint16_t avg_tmon;
       /** Analog Resistor Calibration value */
       uint8_t  rescal;
       /** VCO Rate in MHz */
       uint16_t vco_rate_mhz;
       /**  Analog VCO Range */
       uint8_t  analog_vco_range;
       /** PLL Divider value */
       uint8_t  pll_div;
       /** PLL Lock */
       uint8_t  pll_lock;
       /** PLL Lock Change */
       uint8_t  pll_lock_chg;
       /** Live die temperature in Celsius */
       int16_t die_temp;
       /** Core Status Variable */
       uint8_t core_status;
} merlin_mptwo_core_state_st;

/** Print Error messages to screen before returning.
 * @param err_code Error Code input which is returned as well
 * @return Error Code
 */
SDK_STATIC err_code_t _print_err_msg(uint16_t err_code, const char* s, uint32_t n);

/** Print Convert Error code to String.
 * @param err_code Error Code input which is converted to string
 * @return String containing Error code information.
 */
SDK_STATIC char* _e2s_err_code(err_code_t err_code);


/** Read AFE HW Version.
 * @param *afe_hw_version AFE HW Version used
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _get_afe_hw_version(merlin_access_t *ma, uint8_t *afe_hw_version);

/** Check if the micro's operations on that lane are stopped.
 * @return err_code Error Code "ERR_CODE_UC_NOT_STOPPED" returned if micro NOT stopped
 */
SDK_STATIC err_code_t _check_uc_lane_stopped(merlin_access_t *ma);

/** Calculate the mode_sel parameter for tx pattern generator.
 * @param *mode_sel Mode select to be used for generating required pattern
 * @param *zero_pad_len Length of zero padding to be used for generating required pattern
 * @param patt_length Desired Pattern length
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _calc_patt_gen_mode_sel(uint8_t *mode_sel, uint8_t *zero_pad_len, uint8_t patt_length);

/** Convert the PLL_mode to actual PLL divider value and display
 * as part of merlin_mptwo_display_core_state_line()
 * @param val PLL_DIV register value
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _merlin_mptwo_display_pll_to_divider(merlin_access_t *ma, uint8_t val);


/*---------*/
/*  RX_PI  */
/*---------*/

/** Restart TrainingSum
  */
SDK_STATIC err_code_t _trnsum_clear_and_enable(merlin_access_t *ma);

/** Move P1 Slicer (RX_PI) by desired increments/decrements.
 * @param delta Delta change required in P1 slicer
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _move_clkp1_offset(merlin_access_t *ma, int8_t delta);




/*-------------------*/
/*  Ladder controls  */
/*-------------------*/
/** Converts a ladder setting to mV, given the range.
 * @param ctrl is the threshold control (-31..31) maps to -RANGE to RANGE in non-uniform steps
 * @param range_250 determines the range 0 = +/-150mV, 1 = +/-250mV
 * @return ladder threshold voltage in mV
 */
SDK_STATIC int16_t _ladder_setting_to_mV(merlin_access_t *ma, int8_t ctrl, uint8_t range_250);

/** Setup the P1 slicer vertical level.
 * @param threshold Desired threshold level
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _set_p1_threshold(merlin_access_t *ma, int8_t threshold);

/** Get the P1 slicer vertical level.
 * @param *val 8 bit signed value
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _get_p1_threshold(merlin_access_t *ma, int8_t *val);


/*-----------------------*/
/*  TX_PI and ULL Setup  */
/*-----------------------*/
/** Setup the Ultra low latency clk and datapath for TX.
 * @param enable Enable/Disable (1 = Enable; 0 = Disable)
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _ull_tx_mode_setup(merlin_access_t *ma, uint8_t enable);

/** Setup the Ultra low latency for RX.
 * @param enable Enable/Disable (1 = Enable; 0 = Disable)
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _ull_rx_mode_setup(merlin_access_t *ma, uint8_t enable);

/** Safe multiply - multiplies 2 numbers and checks for overflow.
 * @param a First input
 * @param b Second input
 * @param *of Pointer to overflow indicator
 * @return value of a * b
 */
static uint32_t _mult_with_overflow_check(uint32_t a, uint32_t b, uint8_t *of);


/*-----------------------------------------*/
/*  APIs used in Config Shared TX Pattern  */
/*-----------------------------------------*/
/** Compute Binary string for a Hex value ['0' to 'F'].
 * @param var Hex value to be converted to Binary (eg: '6', 'A', ...)
 * @param bin Binary string returned by API (eg: '0110', '1010', ...)
 * @return Error Code generated by invalid hex variable (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _compute_bin(char var, char bin[]);

/** Compute Hex value for a Binary string ['0000' to '1111'].
 * @param bin Binary string to be coverted (eg: '0110', '1010', ...)
 * @param *hex Hex value calculated from the input Binary string
 * @return Error Code generated by invalid Binary string (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _compute_hex(char bin[], uint8_t *hex);


/*-----------------------------------*/
/*  APIs used in Read Event Logger   */
/*-----------------------------------*/

/** Interpret error event code.
 * @param val Event code error value to be interpreted
 * @return Char string to be displayed in event log
 */
static char* _error_val_2_str(uint8_t val);

/** Display event information.
 * @param event_id Event id to be displayed
 * @param entry_len Length of the event entry
 * @param prev_cursor CL72 prev cursor value
 * @param curr_cursor CL72 curr cursor value
 * @param post_cursor CL72 post cursor value
 * @param *supp_info Supplement information
 */
static err_code_t _display_event(uint8_t event_id, uint8_t entry_len, uint8_t prev_cursor, uint8_t curr_cursor, uint8_t post_cursor, uint8_t *supp_info);


/** Convert float8 to usigned int32.
 * uint32 = 1.XXX * 2^Y  where float8 bits are XXXYYYYY
 * @param input Float8 number
 * @return Usigned 32bit number
 */
static uint32_t _float8_to_int32(float8_t input);


#ifdef TO_FLOATS
/*-----------------------------------*/
/*  APIs used in uC data conversion  */
/*-----------------------------------*/

/** Convert usigned int32 to float8.
 * uint32 = 1.XXX * 2^Y  where float8 bits are XXXYYYYY
 * @param input Unsigned int
 * @return Float8 8 bit representations of 32bit number
 */
static float8_t _int32_to_float8(uint32_t input);
#endif

/** Convert float12 to usigned int32.
 * uint32 = XXXXXXXX * 2^YYYY  where float12 bits X=byte and Y=multi
 * @param byte Float8 8bit
 * @param multi 4 bit multipier
 * @return Usigned 32bit number
 */
static uint32_t _float12_to_uint32(uint8_t byte, uint8_t multi);

#ifdef TO_FLOATS
/** Convert usigned int32 to float12.
 * uint32 = XXXXXXXX * 2^YYYY  where float12 bits X=byte and Y=multi
 * @param input Unsigned int32
 * @param *multi Pointer to byte and the multiplier is returned
 * @return Float12 8 bit representations of 32bit number
 */
static uint8_t _uint32_to_float12(uint32_t input, uint8_t *multi);
#endif

/*-----------------------------*/
/*  Read / Display Core state  */
/*-----------------------------*/
/** Read current merlin_mptwo core status.
 * @param *istate Current merlin_mptwo core status read back and populated by the API
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _merlin_mptwo_read_core_state(merlin_access_t *ma, merlin_mptwo_core_state_st *istate);

/** Display current core state.
 * Reads and displays all important core state values.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _merlin_mptwo_display_core_state_no_newline(merlin_access_t *ma);

/*-----------------------------*/
/*  Read / Display Lane state  */
/*-----------------------------*/
/** Read current merlin_mptwo lane status.
 * @param *istate Current merlin_mptwo lane status read back and populated by the API
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _merlin_mptwo_read_lane_state(merlin_access_t *ma, merlin_mptwo_lane_state_st *istate);


/*-----------------------------------*/
/*  Pll_lock/Sigdet/Pmd_lock status  */
/*-----------------------------------*/
/** Read current PLL Lock and PLL Lock change status of a lane.
 * @param *pll_lock Current merlin_mptwo lane pll_lock status read back and populated by the API
 * @param *pll_lock_chg Current merlin_mptwo lane pll_lock_change status read back and populated by the API
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _merlin_mptwo_pll_lock_status(merlin_access_t *ma, uint8_t *pll_lock, uint8_t *pll_lock_chg);

/** Read current PMD Lock and PMD Lock change status of a lane.
 * @param *pmd_lock Current merlin_mptwo lane pmd_rx_lock status read back and populated by the API
 * @param *pmd_lock_chg Current merlin_mptwo lane pmd_rx_lock_change status read back and populated by the API
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _merlin_mptwo_pmd_lock_status(merlin_access_t *ma, uint8_t *pmd_lock, uint8_t *pmd_lock_chg);

/** Read current Signal_detect and Signal_detect change status of a lane.
 * @param *sig_det Current merlin_mptwo lane signal_detect status read back and populated by the API
 * @param *sig_det_chg Current merlin_mptwo lane signal_detect_change status read back and populated by the API
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _merlin_mptwo_sigdet_status(merlin_access_t *ma, uint8_t *sig_det, uint8_t *sig_det_chg);




/*-----------------*/
/*  Get OSR mode   */
/*-----------------*/
/** Read current merlin_mptwo lane status.
 * @param *mode Returns with the osr mode structure
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _merlin_mptwo_get_osr_mode(merlin_access_t *ma, merlin_mptwo_osr_mode_st *mode);

/** Display current lane state.
 * Reads and displays all important lane state values.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _merlin_mptwo_display_lane_state_no_newline(merlin_access_t *ma);



/** Convert eye margin to mV.
 * Takes in the ladder setting with 3 fractional bits and converts to mV.
 * @param var Ladder setting with 3 fractional bits
 * @param ladder_range Specified if ladder is configured for 150mV or 250mV range
 * @return Eye opening in mV
 */
SDK_STATIC uint16_t _eye_to_mV(merlin_access_t *ma, uint8_t var, uint8_t ladder_range);

/** Convert eye margin to mUI.
 * Takes in a horizontal margin in Phase Interpolator codes and converts it to mUI.
 * @param var Horizontal margin in Phase Interpolator codes with 3 fractional bits
 * @return Eye opening in mV
 */
SDK_STATIC uint16_t _eye_to_mUI(uint8_t var);


/** Serdes Core ClockGate.
 * Along with merlin_mptwo_core_clkgate(), all lanes should also be clock gated using merlin_mptwo_lane_clkgate() to complete a Core Clockgate
 * @param enable Enable clockgate (1 = Enable clokgate; 0 = Disable clockgate)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _merlin_mptwo_core_clkgate(merlin_access_t *ma, uint8_t enable);

/** Serdes Lane ClockGate.
 * @param enable Enable lane clockgate (1 = Enable clockgate; 0 = Disable clockgate)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _merlin_mptwo_lane_clkgate(merlin_access_t *ma, uint8_t enable);


/** Set function for PF.
 * @param val Signed input value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _set_rx_pf_main(merlin_access_t *ma, uint8_t val);

/** Get function for PF
 * @param *val PF read value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _get_rx_pf_main(merlin_access_t *ma, int8_t *val);

/** Set function for PF2.
 * @param val signed input value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _set_rx_pf2(merlin_access_t *ma, uint8_t val);

/** Get function for PF2.
 * @param *val PF2 read value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _get_rx_pf2(merlin_access_t *ma, int8_t *val);

/** Set function for VGA.
 * @param val signed input value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _set_rx_vga(merlin_access_t *ma, uint8_t val);

/** Get function for VGA.
 * @param *val VGA read value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _get_rx_vga(merlin_access_t *ma, int8_t *val);


/** Set function for DFE Tap1.
 * @param val signed input value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _set_rx_dfe1(merlin_access_t *ma, int8_t val);

/** Get function for DFE Tap1.
 * @param *val DFE1 read value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _get_rx_dfe1(merlin_access_t *ma, int8_t *val);

/** Set function for DFE Tap2.
 * @param val signed input value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _set_rx_dfe2(merlin_access_t *ma, int8_t val);

/** Get function for DFE Tap2.
 * @param *val DFE2 read value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
SDK_STATIC err_code_t _get_rx_dfe2(merlin_access_t *ma, int8_t *val);

/** Set function for DFE Tap3.
 * @param val signed input value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _set_rx_dfe3(merlin_access_t *ma, int8_t val);

/** Get function for DFE Tap3.
 * @param *val DFE3 read value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _get_rx_dfe3(merlin_access_t *ma, int8_t *val);

/** Set function for DFE Tap4.
 * @param val signed input value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _set_rx_dfe4(merlin_access_t *ma, int8_t val);

/** Get function for DFE Tap4.
 * @param *val DFE4 read value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _get_rx_dfe4(merlin_access_t *ma, int8_t *val);

/** Set function for DFE Tap5.
 * @param val signed input value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _set_rx_dfe5(merlin_access_t *ma, int8_t val);

/** Get function for DFE Tap5
 * @param *val DFE5 read value
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _get_rx_dfe5(merlin_access_t *ma, int8_t *val);


/* Set/Get TX Tap values */
static err_code_t _get_tx_pre(merlin_access_t *ma, int8_t *val);
static err_code_t _get_tx_main(merlin_access_t *ma, int8_t *val);
static err_code_t _get_tx_post1(merlin_access_t *ma, int8_t *val);
static err_code_t _get_tx_post2(merlin_access_t *ma, int8_t *val);

/* Lane Config Struct */
static err_code_t _update_uc_lane_config_st(struct merlin_mptwo_uc_lane_config_st *st);
static err_code_t _update_uc_lane_config_word(struct merlin_mptwo_uc_lane_config_st *st);

/* Lane User Control Disable Startup Function Struct */
static err_code_t _update_usr_ctrl_disable_functions_st(struct merlin_mptwo_usr_ctrl_disable_functions_st *st);
static err_code_t _update_usr_ctrl_disable_functions_byte(struct merlin_mptwo_usr_ctrl_disable_functions_st *st);

/* Lane User Control Disable Startup DFE Function Struct */
static err_code_t _update_usr_ctrl_disable_dfe_functions_st(struct merlin_mptwo_usr_ctrl_disable_dfe_functions_st *st);
static err_code_t _update_usr_ctrl_disable_dfe_functions_byte(struct merlin_mptwo_usr_ctrl_disable_dfe_functions_st *st);

/* Core Config Struct */
static err_code_t _update_uc_core_config_st(struct  merlin_mptwo_uc_core_config_st *st);
static err_code_t _update_uc_core_config_word(struct  merlin_mptwo_uc_core_config_st *st);

static uint8_t _merlin_mptwo_rdb_uc_var(merlin_access_t *ma, err_code_t *err_code_p, uint16_t addr);
static uint16_t _merlin_mptwo_rdw_uc_var(merlin_access_t *ma, err_code_t *err_code_p, uint16_t addr);
static err_code_t _merlin_mptwo_wrb_uc_var(merlin_access_t *ma, uint16_t addr, uint8_t wr_val);
static err_code_t _merlin_mptwo_wrw_uc_var(merlin_access_t *ma, uint16_t addr, uint16_t wr_val);
static err_code_t _merlin_mptwo_display_ber_scan_data(merlin_access_t *ma, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control);


/*--------------------------*/
/*  Register field polling  */
/*--------------------------*/

/** Polls lane variable "usr_diag_status" to verify data is available in uC diag buffer.
 * It then reads a WORD of data wich is 2 float8_t samples, it converts them to uint32_t
 * and returns them in the memory pointed to by *data.
 * Define macro CUSTOM_REG_POLLING to replace the default implementation provided in merlin_mptwo_functions.c.
 *
 * @param *data is pointer to 2 element array of uint32_tpassed from uC through dsc_data
 * @param *status returns a status byte           \n
 *    bit 15 - indicates the ey scan is complete \n
 *    bit 14 - indicates uC is slower than read access \n
 *    bit 13 - indicates uC is faster than read access \n
 *    bit 7-0 - indicates amount of data in the uC buffer
 *
 * @param timeout_ms Maximum time interval in milliseconds for which the polling is done
 * @return Error code generated by polling function (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin_mptwo_poll_diag_eye_data(merlin_access_t *ma, uint32_t *data,uint16_t *status, uint32_t timeout_ms);

/** Polls lane variable "usr_diag_status" to verify data is available in uC diag buffer.
 * Define macro CUSTOM_REG_POLLING to replace the default implementation provided in merlin_mptwo_functions.c.
 *
 * @param *status returns a status byte \n
 *    bit 15 - indicates the ey scan is complete \n
 *    bit 14-0 - reserved for debug
 *
 * @param timeout_ms Maximum time interval in milliseconds for which the polling is done
 * @return Error code generated by polling function (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin_mptwo_poll_diag_done(merlin_access_t *ma, uint16_t *status, uint32_t timeout_ms);

/** Polls for register field "uc_dsc_ready_for_cmd" to be 1 within the time interval specified by timeout_ms.
 * Function returns 0 if polling passes, else it returns error code.
 * Define macro CUSTOM_REG_POLLING to replace the default implementation provided in merlin_mptwo_functions.c.
 * @param timeout_ms Maximum time interval in milliseconds for which the polling is done
 * @return Error code generated by polling function (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin_mptwo_poll_uc_dsc_ready_for_cmd_equals_1(merlin_access_t *ma, uint32_t timeout_ms);

/** Polls for register field "dsc_state" to be "DSC_STATE_UC_TUNE"
 * within the time interval specified by timeout_ms.
 * Function returns 0 if polling passes, else it returns error code.
 * Define macro CUSTOM_REG_POLLING to replace the default implementation provided in
 * merlin_mptwo_functions.c.
 * @param timeout_ms Maximum time interval in milliseconds for which the polling is done
 * @return Error code generated by polling function (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin_mptwo_poll_dsc_state_equals_uc_tune(merlin_access_t *ma, uint32_t timeout_ms);

/** Polls for register field "st_afe_tx_fifo_resetb" to be 1 within the time interval specified by timeout_ms.
 * Function returns 0 if polling passes, else it returns error code.
 * Define macro CUSTOM_REG_POLLING to replace the default implementation provided in merlin_mptwo_functions.c.
 * @param timeout_ms Maximum time interval in milliseconds for which the polling is done
 * @return Error code generated by polling function (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin_mptwo_poll_st_afe_tx_fifo_resetb_equals_1(merlin_access_t *ma, uint32_t timeout_ms);



#endif
