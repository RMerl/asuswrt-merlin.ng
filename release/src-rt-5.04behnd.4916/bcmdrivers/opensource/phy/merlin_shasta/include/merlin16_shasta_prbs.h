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

/***********************************************************************************
 ***********************************************************************************
 *                                                                                 *
 *  Revision    :        *
 *                                                                                 *
 *  Description :  PRBS test functions provided to IP User                         *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/

/** @file merlin16_shasta_prbs.h
 * PRBS test functions provided to IP User
 */

#ifndef MERLIN16_SHASTA_API_PRBS_H
#define MERLIN16_SHASTA_API_PRBS_H

#include "merlin16_shasta_ipconfig.h"
#include "common/srds_api_enum.h"
#include "common/srds_api_err_code.h"
#include "common/srds_api_types.h"
#include "merlin16_shasta_usr_includes.h"

#define PRBS_VERBOSE 0
/*------------------------------*/
/*  Shared TX Pattern Generator */
/*------------------------------*/
/** Configure Shared TX Pattern API.
 * An input string (hex or binary) and pattern length are taken in as inputs, based on which the Pattern Generator registers
 * are programmed to the appropriate values to generate that pattern.\n
 * eg: For a repeating pattern "0000010110101111", input_pattern = "0000010110101111" or "0x05AF" and patt_length = 16\n\n
 * NOTE: merlin16_shasta_tx_shared_patt_gen_en() API should be called to enable the Pattern generator for that particular lane. \n
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param patt_length Pattern length
 * @param pattern Input Pattern - Can be in hex (eg: "0xB055") or in binary (eg: "011011")
 * @return Error Code generated by invalid input pattern or pattern length (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_config_shared_tx_pattern(srds_access_t *sa__, uint8_t patt_length, const char pattern[]);

/**************************************************/
/* LANE Based APIs - Required to be used per Lane */
/**************************************************/

/*----------------------------*/
/*  Enable Pattern Generator  */
/*----------------------------*/
/** Enable/Disable Shared TX pattern generator.
 * Note: The patt_length input to the function should be the value sent to the merlin16_shasta_config_shared_tx_pattern() function
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param enable Enable shared fixed pattern generator (1 = Enable; 0 = Disable)
 * @param patt_length length of the pattern used in merlin16_shasta_config_shared_tx_pattern()
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_tx_shared_patt_gen_en(srds_access_t *sa__, uint8_t enable, uint8_t patt_length);

/*----------------------------*/
/*  Configure PRBS Functions  */
/*----------------------------*/
/**  Configure PRBS Generator.
 * Once the PRBS generator is configured, to enable PRBS use the merlin16_shasta_tx_prbs_en() API.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param prbs_poly_mode PRBS generator mode select (selects required PRBS polynomial)
 * @param prbs_inv PRBS invert enable
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_config_tx_prbs(srds_access_t *sa__, enum srds_prbs_polynomial_enum prbs_poly_mode, uint8_t prbs_inv);

/**  Get PRBS Generator Configuration.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *prbs_poly_mode PRBS generator mode select (selects required PRBS polynomial)
 * @param *prbs_inv PRBS invert enable
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_get_tx_prbs_config(srds_access_t *sa__, enum srds_prbs_polynomial_enum *prbs_poly_mode, uint8_t *prbs_inv);

/** PRBS Generator Enable/Disable.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param enable Enable PRBS Generator (1 = Enable; 0 = Disable)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_tx_prbs_en(srds_access_t *sa__, uint8_t enable);

/** Get PRBS Generator Enable/Disable.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *enable returns the value of Enable PRBS Generator (1 = Enable; 0 = Disable)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_get_tx_prbs_en(srds_access_t *sa__, uint8_t *enable);

/** PRBS Generator Single Bit Error Injection.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param enable (1 = error is injected; 0 = no error is injected)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_tx_prbs_err_inject(srds_access_t *sa__, uint8_t enable);

/**  Configure PRBS Checker.
 * Once the PRBS checker is configured, use the merlin16_shasta_rx_prbs_en() API to enable the checker.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param prbs_poly_mode PRBS checker mode select (selects required PRBS polynomial)
 * @param prbs_checker_mode Checker Mode to select PRBS LOCK state machine
 * @param prbs_inv PRBS invert enable
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_config_rx_prbs(srds_access_t *sa__, enum srds_prbs_polynomial_enum prbs_poly_mode, enum srds_prbs_checker_mode_enum prbs_checker_mode, uint8_t prbs_inv);

/**  Get PRBS Checker congifuration.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *prbs_poly_mode PRBS checker mode select (selects required PRBS polynomial)
 * @param *prbs_checker_mode Checker Mode to select PRBS LOCK state machine
 * @param *prbs_inv PRBS invert enable
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_get_rx_prbs_config(srds_access_t *sa__, enum srds_prbs_polynomial_enum *prbs_poly_mode, enum srds_prbs_checker_mode_enum *prbs_checker_mode, uint8_t *prbs_inv);

/** PRBS Checker Enable/Disable.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param enable Enable PRBS Checker (1 = Enable; 0 = Disable)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_rx_prbs_en(srds_access_t *sa__, uint8_t enable);

/** Get PRBS Checker Enable/Disable.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *enable returns with the value of Enable PRBS Checker (1 = Enable; 0 = Disable)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_get_rx_prbs_en(srds_access_t *sa__, uint8_t *enable);

/** PRBS Checker LOCK status (live status).
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *chk_lock Live lock status read by API
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_prbs_chk_lock_state(srds_access_t *sa__, uint8_t *chk_lock);

/** PRBS Error Count and Lock Lost status.
 * Error count and lock lost read back as a single 32bit value. Bit 31 is lock lost and [30:0] is error count.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *prbs_err_cnt 32bit value returned by API ([30:0] = Error Count; [31] = Lock lost)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_prbs_err_count_ll(srds_access_t *sa__, uint32_t *prbs_err_cnt);

/** PRBS Error Count and Lock Lost status.
 * Error count and lock lost read back on separate variables
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *prbs_err_cnt 32bit Error count value
 * @param *lock_lost Lock Lost status (1 = if lock was ever lost)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_prbs_err_count_state(srds_access_t *sa__, uint32_t *prbs_err_cnt, uint8_t *lock_lost);

#if defined(MERLIN16_SHASTA_PRBS_CHK_HARDWARE_TIMERS)
/** PRBS checker' hardware configuartion Struct */
struct merlin16_shasta_prbs_chk_hw_timer_ctrl_st {
    /** PRBS checker burst error count mode enable */
    uint8_t prbs_chk_burst_err_cnt_en;
    /** prbs_chk_en timer mode */
    uint8_t prbs_chk_en_timer_mode;
    /** PRBS timer timeout value */
    uint8_t prbs_chk_en_timeout;
};

/** Get PRBS hardware timers configuration.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *prbs_chk_hw_timer_ctrl_bak Structure to store PRBS checker's hardware timers configuration
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_get_prbs_chk_hw_timer_ctrl(srds_access_t *sa__, struct merlin16_shasta_prbs_chk_hw_timer_ctrl_st * const prbs_chk_hw_timer_ctrl_bak);

/** Set PRBS hardware timers configuration.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param *prbs_chk_hw_timer_ctrl_bak Structure to be used to restore PRBS checker's hardware timers configuration
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_set_prbs_chk_hw_timer_ctrl(srds_access_t *sa__, struct merlin16_shasta_prbs_chk_hw_timer_ctrl_st const * const prbs_chk_hw_timer_ctrl_bak);

/** Configure PRBS checker's hardware timers.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param time_ms is the amount of time to delay for BER calculation
 * @param time_ms_adjusted is the amount of time delay that will be used for BER calculation
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_config_prbs_chk_hw_timer(srds_access_t *sa__, uint16_t time_ms, uint16_t *time_ms_adjusted);
#endif /* MERLIN16_SHASTA_PRBS_CHK_HARDWARE_TIMERS */

/** Toggle PRBS checker enable.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_prbs_chk_en_toggle(srds_access_t *sa__);

/** Header display for detailed PRBS display function
 * This can be displayed once and then several cores and lanes after each showing one line
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_display_detailed_prbs_state_hdr(void);

/** Display detailed PRBS data per lane including Burst Error
 * 
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shasta_display_detailed_prbs_state(srds_access_t *sa__);

extern const char* merlin16_shasta_e2s_prbs_mode_enum[8];


#endif
