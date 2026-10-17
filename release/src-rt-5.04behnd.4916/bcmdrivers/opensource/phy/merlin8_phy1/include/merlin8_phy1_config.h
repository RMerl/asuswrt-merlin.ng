/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

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
 *  Revision    :      *
 *                                                                                 *
 *  Description :  Config functions targeted to IP user                            *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MERLIN8_PHY1_API_CONFIG_H
#define MERLIN8_PHY1_API_CONFIG_H

#include "merlin8_phy1_ipconfig.h"
#include "common/srds_api_enum.h"
#include "common/srds_api_err_code.h"
#include "common/srds_api_types.h"
#include "merlin8_phy1_enum.h"
#include "merlin8_phy1_types.h"
#include "merlin8_phy1_select_defns.h"
#include "merlin8_phy1_access.h"

/*! @file
 *  @brief Configuration functions provided to IP User
 */

/*! @addtogroup APITag
 * @{
 */

/*! @defgroup SerdesAPIConfigTag Configuration APIs
 * Serdes API functions which can be used to get and/or set
 * Serdes configuration settings, as well as enable/disable modes.
 * Also includes APIs for loading and verifying uCode.
 */

/*! @addtogroup SerdesAPIConfigTag
 * @{
 */

#define GRACEFUL_STOP_TIME (200)

#ifdef rdc_revid_core
/* revid_core is currently only defined for merlin8_pcieg3 */
#define IF_API_ID_CODES_DONT_MATCH_CORE_ID_CODES                                                                                                            \
uint16_t revid, revid2, revid_core_mac, revid_process;                                                                                                          \
ESTM(revid      = rdc_revid_model());                                                                                                                       \
ESTM(revid_process = rdc_revid_process());                                                                                                                  \
ESTM(revid2     = rdc_revid2());                                                                                                                            \
ESTM(revid_core_mac = rdc_revid_core());                                                                                                                        \
if ((MERLIN8_PHY1_REVID_MODEL_DEFAULT != revid) || (MERLIN8_PHY1_REVID2_DEFAULT != revid2) || (revid_core_mac < 0x10) || (MERLIN8_PHY1_REVID_PROCESS_DEFAULT != revid_process)) { \
    EFUN_PRINTF(("ERROR: IP Hardware ID Mismatch. Expected: 0x%08X and core >= 0x10. Actual: 0x%08X core = 0x%02x.\n",                                      \
                (uint32_t)(((MERLIN8_PHY1_REVID_MODEL_DEFAULT | (MERLIN8_PHY1_REVID_PROCESS_DEFAULT<<6)) << 16) | (MERLIN8_PHY1_REVID2_DEFAULT << 0)),                        \
                (uint32_t)((revid_process<<22) | (revid << 16) | (revid2 << 0))));

#else
/* If revid_core is not defined, read the entire revid2 register and expect unused bits to be 0 */
#define IF_API_ID_CODES_DONT_MATCH_CORE_ID_CODES                                                                                      \
uint16_t revid, revid2, revid_process;                                                                                                \
ESTM(revid         = rdc_revid_model());                                                                                              \
ESTM(revid_process = rdc_revid_process());                                                                                            \
ESTM(revid2        = reg_rdc_DIG_COM_REVID2());                                                                                       \
if ((MERLIN8_PHY1_REVID_MODEL_DEFAULT != revid) || (MERLIN8_PHY1_REVID2_DEFAULT != revid2) || (MERLIN8_PHY1_REVID_PROCESS_DEFAULT != revid_process)) {  \
    EFUN_PRINTF(("ERROR: IP Hardware ID Mismatch. Expected: 0x%08X. Actual: 0x%08X.\n",                                               \
                (uint32_t)(((MERLIN8_PHY1_REVID_MODEL_DEFAULT | (MERLIN8_PHY1_REVID_PROCESS_DEFAULT<<6)) << 16) | (MERLIN8_PHY1_REVID2_DEFAULT << 0)),  \
                (uint32_t)((revid_process<<22) | (revid << 16) | (revid2 << 0))));

#endif


#define RELEASE_LOCK_AND_RETURN(__x__) \
    USR_RELEASE_LOCK; \
    return (__x__);

/****************************************************/
/*  CORE Based APIs - Required to be used per Core  */
/****************************************************/
#ifndef SMALL_FOOTPRINT
/* Returns API Version Number */
/** API Version Number.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] api_version API Version Number returned by the API
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_version(srds_access_t *sa__, uint32_t *api_version);
#endif /* SMALL_FOOTPRINT */

#ifndef PCIEG3_V1
/*------------------------------------------------*/
/*  APIs to Read Core Config variables in uC RAM  */
/*------------------------------------------------*/
/** Read value of core_config uC RAM variable.
 *  Note that various API configuration functions can modify core config.
 *  Since the value returned by this can become stale, re-read after modifying core configuration.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] struct_val core_config RAM variable values read by the API
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_get_uc_core_config(srds_access_t *sa__, struct merlin8_phy1_uc_core_config_st *struct_val);
#endif /* !MERLIN7_PCIEG3 */


/*------------------------------------------------*/
/*  APIs to get number of uc cores  */
/*------------------------------------------------*/
/** Get the number of uc cores.
 *  Note that various API configuration functions can modify core config.
 *  Since the value returned by this can become stale, re-read after modifying core configuration.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] num_micros Value containing the number of uc cores returned by the API.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_get_micro_num_uc_cores(srds_access_t *sa__, uint8_t *num_micros);

/*-----------------------------------*/
/*  Microcode Load/Verify Functions  */
/*-----------------------------------*/
/** Performs the initial register writes which are required before loading microcode.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] pram indicates if the microcode is loaded through the PRAM interface (1 = yes; 0 = no )
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_ucode_load_init(srds_access_t *sa__, uint8_t pram);

/** Writes all or part of the microcode image into Micro.
 * Note: If microcode is being loaded with multiple writes, then ucode_len must be divisble by 4 (32 bit aligned)
 * for all writes, except for the final write, which does not need to be 32 bit aligned.
 * Note: merlin8_phy1_ucode_load_init() must be called before the first call to merlin8_phy1_ucode_load_write(),
 * and merlin8_phy1_ucode_load_close() must be called after the last call to merlin8_phy1_ucode_load_write().
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] ucode_image pointer to the Microcode image organized in bytes.
 * @param[in] ucode_len Length of Microcode Image (number of bytes).
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_ucode_load_write(srds_access_t *sa__, uint8_t *ucode_image, uint32_t ucode_len);

/** Ends Microcode loading state. Call after the last merlin8_phy1_ucode_load_write().
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] pram indicates if the microcode is loaded through the PRAM interface (1 = yes; 0 = no )
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_ucode_load_close(srds_access_t *sa__, uint8_t pram);

/** Load Microcode into current one core through Register (MDIO) Interface.
 * Once the microcode is loaded, de-assert reset to micro to start executing microcode "wrc_micro_core_rstb(0x1)".
 * Note: Micro should be loaded only after issuing a merlin8_phy1_uc_reset(1) followed by asserting and de-asserting
 * core_s_reset. Information table should be intialized with merlin8_phy1_init_merlin8_phy1_info after microcode load.
 * See relevant Programmers guide for more details.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] ucode_image pointer to the Microcode image organized in bytes.
 * @param[in] ucode_len Length of Microcode Image (number of bytes).
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_ucode_load(srds_access_t *sa__, uint8_t *ucode_image, uint32_t ucode_len);


#ifndef SMALL_FOOTPRINT
/** To verify the Microcode image loaded in the Micro.
 * Read back the microcode from Micro and check against expected microcode image.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] ucode_image pointer to the expected Microcode image organized in bytes.
 * @param[in] ucode_len Length of Microcode Image (number of bytes).
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_ucode_load_verify(srds_access_t *sa__, uint8_t *ucode_image, uint32_t ucode_len);

/** To verify the CRC of the microcode loaded in the Micro.
 * Instruct uC to read image and calculate CRC and check against expected CRC.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] ucode_len Length of Microcode Image (number of bytes)
 * @param[in] expected_crc_value Expected CRC value of the microcode
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_ucode_crc_verify(srds_access_t *sa__, uint32_t ucode_len, uint16_t expected_crc_value);

/** To instruct the micro to start calculating the CRC of the microcode.
 * Control is returned after triggering start of CRC calculation (does NOT wait for completion of CRC calculation). \n \n
 * NOTE: No uC commands should be executed between starting the CRC calculaion [merlin8_phy1_ucode_crc_verify()] and verifying the CRC value [merlin8_phy1_start_ucode_crc_calc()].
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] ucode_len Length of Microcode Image (number of bytes).
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_start_ucode_crc_calc(srds_access_t *sa__, uint16_t ucode_len);

/** To check the expected CRC against the CRC calulated by the micro.
 * NOTE: No uC commands should be executed between starting the CRC calculation [merlin8_phy1_ucode_crc_verify()] and verifying the CRC value [merlin8_phy1_start_ucode_crc_calc()].
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] expected_crc_value Expected CRC value of the microcode.
 * @param[in] timeout_ms Time interval in milliseconds inside which the previous command (calculate CRC) should be completed.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_check_ucode_crc(srds_access_t *sa__, uint16_t expected_crc_value, uint32_t timeout_ms);
#endif /* SMALL_FOOTPRINT */

/** Load the Micro through the PRAM bus.
 * NOTE: Information table should be intialized with merlin8_phy1_init_merlin8_phy1_info after microcode load.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] ucode_image Microcode Image to be written.
 * @param[in] ucode_len Length of Microcode Image (number of bytes).
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_ucode_pram_load(srds_access_t *sa__, char const * ucode_image, uint32_t ucode_len);

/** Enable or Disable the uC reset.
 * Dummy function to maintain compatibility with BHK7 APIs [calls merlin8_phy1_uc_reset(..) internally].
 * Note: Micro should be reset using the API everytime before reloading the microcode.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable uC reset (1 = Enable; 0 = Disable).
 * @param[in] ucode_info struct has information regarding stack size.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_uc_reset_with_info(srds_access_t *sa__, uint8_t enable, ucode_info_t ucode_info);

/** Enable or Disable the uC reset.
 * Note: Micro should be reset using the API everytime before reloading the microcode.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable uC reset (1 = Enable; 0 = Disable).
 * @param[in] ucode_info struct has information regarding stack size.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_uc_reset(srds_access_t *sa__, uint8_t enable, ucode_info_t ucode_info);

/** API to assert Serdes core reset.
 * Issues merlin8_phy1_uc_reset followed by merlin8_phy1 core reset (core_s_rstb) after selecting com_clk as micro_clk.
 * NOTE: This API should be used to issue a core reset instead of directly using the core_s_rstb register.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] ucode_info struct has information regarding stack size.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_core_reset(srds_access_t *sa__, ucode_info_t ucode_info);

#ifndef PCIEG3_V1
/** Wait for the uC to become active.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated if uC does not become active (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_wait_uc_active(srds_access_t *sa__);
#endif

#define INFO_TABLE_END INFO_TABLE_END_LEGACY

/** Initialize the merlin8_phy1_info for the uC.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated if uC does not become active (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_init_merlin8_phy1_info(srds_access_t *sa__);

/** Clear the merlin8_phy1_info for the uC.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_clear_merlin8_phy1_info(srds_access_t *sa__);

#ifndef PCIEG3_V1
#ifndef SMALL_FOOTPRINT

/** Set the core_cfg_from_pcs flag in the core configuration.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] core_cfg_from_pcs The value to set.
 * @return Error Code generated if uC does not become active (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_set_core_config_from_pcs(srds_access_t *sa__, uint8_t core_cfg_from_pcs);


#endif /* SMALL_FOOTPRINT */

/*-----------------*/
/*  Configure PLL  */
/*-----------------*/

/** The pll can be configured by providing all four parameters:  refclk frequency, divider value, VCO output frequency and the PLL options.
 *  This is the only option in SMALL_FOOTPRINT
 */
#define merlin8_phy1_configure_pll_refclk_div_vco merlin8_phy1_INTERNAL_configure_pll

#ifndef SMALL_FOOTPRINT
/** PLL Powerdown.
 *
 * Powers down the selected PLL. To power up the PLL, reconfigure the PLL by using
 * one of the "merlin8_phy1_configure_pll_" APIs.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_powerdown(srds_access_t *sa__);

/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this function is based on refclk frequency and divider value.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] refclk Reference clock frequency (enumerated).
 * @param[in] srds_div Divider value (enumerated).
 *
 * @b Example (refclk = 156.25 MHz, Divider = 170, VCO output frequency = 26.5625 GHz)
 * @code
    EFUN(merlin8_phy1_core_dp_reset(sa__, 1));
    EFUN(merlin8_phy1_configure_pll_refclk_div(sa__, MERLIN8_PHY1_PLL_REFCLK_156P25MHZ, MERLIN8_PHY1_PLL_DIV_170));
    EFUN(merlin8_phy1_core_dp_reset(sa__, 0));
 * @endcode
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_refclk_div(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           enum merlin8_phy1_pll_div_enum srds_div);

/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this function is based on refclk frequency and VCO output frequency.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] refclk Reference clock frequency (enumerated).
 * @param[in] vco_freq_khz VCO output frequency, in kHz.
 *
 * @b Example (VCO output frequency = 25 GHz, refclk = 156.25 MHz, Divider = 160)
 * @code
    EFUN(merlin8_phy1_core_dp_reset(sa__, 1));
    EFUN(merlin8_phy1_configure_pll_refclk_vco(sa__, MERLIN8_PHY1_PLL_REFCLK_156P25MHZ, 25000000));
    EFUN(merlin8_phy1_core_dp_reset(sa__, 0));
 * @endcode
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_refclk_vco(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           uint32_t vco_freq_khz);

/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this function is based on divider value and VCO output frequency.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] srds_div Divider value (enumerated).
 * @param[in] vco_freq_khz VCO output frequency, in kHz.
 *
 * @b Example (VCO output frequency = 25 GHz, Divider = 170, refclk = 147.05 MHz)
 * @code
    EFUN(merlin8_phy1_core_dp_reset(sa__, 1));
    EFUN(merlin8_phy1_configure_pll_refclk_vco(sa__, MERLIN8_PHY1_PLL_DIV_170, 25000000));
    EFUN(merlin8_phy1_core_dp_reset(sa__, 0));
 * @endcode
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_div_vco(srds_access_t *sa__,
                                        enum merlin8_phy1_pll_div_enum srds_div,
                                        uint32_t vco_freq_khz);

/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this fucntion is based on input refclk frequency and divider value.
 * but first divides the input refclk by 2 and adjusts parameters accordingly.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] refclk Input Reference clock frequency before refclk divide by 2  (enumerated).
 * @param[in] srds_div Divider value based on input reference clock frequency (enumerated).
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_refclk_div_div2refclk(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           enum merlin8_phy1_pll_div_enum srds_div);

/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this function is based on input refclk frequency and VCO output frequency.
 * but first divides the input refclk by 2 and adjusts parameters accordingly.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] refclk Input Reference clock frequency before refclk divide by 2 (enumerated).
 * @param[in] vco_freq_khz VCO output frequency, in kHz.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_refclk_vco_div2refclk(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           uint32_t vco_freq_khz);

/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this function is based on divider value and VCO output frequency.
 * but first divides the input refclk by 2 and adjusts parameters accordingly.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] srds_div Divider value based on input refclk frequency before refclk divide by 2 (enumerated).
 * @param[in] vco_freq_khz VCO output frequency, in kHz.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_div_vco_div2refclk(srds_access_t *sa__,
                                        enum merlin8_phy1_pll_div_enum srds_div,
                                        uint32_t vco_freq_khz);
/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this fucntion is based on input refclk frequency and divider value.
 * but first divides the input refclk by 4 and adjusts parameters accordingly.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] refclk Input Reference clock frequency before refclk divide by 4  (enumerated).
 * @param[in] srds_div Divider value based on input reference clock frequency (enumerated).
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_refclk_div_div4refclk(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           enum merlin8_phy1_pll_div_enum srds_div);

/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this function is based on input refclk frequency and VCO output frequency.
 * but first divides the input refclk by 4 and adjusts parameters accordingly.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] refclk Input Reference clock frequency before refclk divide by 4 (enumerated).
 * @param[in] vco_freq_khz VCO output frequency, in kHz.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_refclk_vco_div4refclk(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           uint32_t vco_freq_khz);

/** Configure PLL.
 *
 * Use core_dp_s_rstb to re-initialize the PLL to its default configuration before calling this function.
 * For single PLL SerDes, use core_s_rstb instead of core_dp_s_rstb to also re-initialize all registers to default.
 *
 * Configures PLL registers to obtain the required configuration.
 * PLL configuration for this function is based on divider value and VCO output frequency.
 * but first divides the input refclk by 4 and adjusts parameters accordingly.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] srds_div Divider value based on input refclk frequency before refclk divide by 4 (enumerated).
 * @param[in] vco_freq_khz VCO output frequency, in kHz.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_configure_pll_div_vco_div4refclk(srds_access_t *sa__,
                                        enum merlin8_phy1_pll_div_enum srds_div,
                                        uint32_t vco_freq_khz);


/** Get the VCO frequency in kHz, based on the reference clock frequency and divider value.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] refclk_freq_hz Reference clock frequency, in Hz.
 * @param[in] srds_div Divider value, in the same encoding as enum #merlin8_phy1_pll_div_enum.
 * @param[out] vco_freq_khz VCO output frequency, in kHz, obtained based on reference clock frequency and dic value.
 * @param[in] pll_option Select PLL configuration option from enum #merlin8_phy1_pll_option_enum.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_get_vco_from_refclk_div(srds_access_t *sa__, uint32_t refclk_freq_hz, enum merlin8_phy1_pll_div_enum srds_div, uint32_t *vco_freq_khz, enum merlin8_phy1_pll_option_enum pll_option);
#endif /* SMALL_FOOTPRINT */

/**************************************************/
/* LANE Based APIs - Required to be used per Lane */
/**************************************************/

/*------------------------------------------------------------*/
/*  APIs to Write Lane Config and User variables into uC RAM  */
/*------------------------------------------------------------*/
/** Write to lane_config uC RAM variable.
 * Note: This API should be used only during configuration under dp_reset (merlin8_phy1_ln_dp_reset()).
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] struct_val Value to be written into lane_config RAM variable.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_set_uc_lane_cfg(srds_access_t *sa__, struct merlin8_phy1_uc_lane_config_st struct_val);


#ifndef SMALL_FOOTPRINT
/*-----------------------------------------------------------*/
/*  APIs to Read Lane Config and User variables from uC RAM  */
/*-----------------------------------------------------------*/
/** Read value of lane_config uC RAM variable.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] struct_val Value read from lane_config RAM variable.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_get_uc_lane_cfg(srds_access_t *sa__, struct merlin8_phy1_uc_lane_config_st *struct_val);
#endif /* SMALL_FOOTPRINT */
#endif /* MERLIN7_PCIEG3 */

/*--------------------------------------------*/
/*  APIs to Reset Lane to Default             */
/*--------------------------------------------*/

/** Resets TX Lane to Default Configuration.
 * Toggles TX ln_s_rstb and leaves the lane TX in datapath reset.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_reset_tx_lane_to_default(srds_access_t *sa__);

/** Resets RX Lane to Default Configuration.
 * Toggles RX ln_s_rstb and leaves the lane RX in datapath reset.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_reset_rx_lane_to_default(srds_access_t *sa__);

/** Resets Lane to Default Configuration.
 * Toggles ln_s_rstb and leaves the lane in datapath reset.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_reset_lane_to_default(srds_access_t *sa__);

/*--------------------------------------------*/
/*  APIs to Enable or Disable datapath reset  */
/*--------------------------------------------*/

/** Enable or Disable TX datapath reset.
 * Asserts handshake signals upon disable.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable TX datapath reset (1 = Enable; 0 = Disable).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_tx_dp_reset(srds_access_t *sa__, uint8_t enable);

/** Enable or Disable RX datapath reset.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable RX datapath reset (1 = Enable; 0 = Disable).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_rx_dp_reset(srds_access_t *sa__, uint8_t enable);

/** Enable or Disable Core datapath reset.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable Core datapath reset (1 = Enable; 0 = Disable).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_core_dp_reset(srds_access_t *sa__, uint8_t enable);

/** Enable or Disable Lane datapath reset.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable Lane datapath reset (1 = Enable; 0 = Disable).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_ln_dp_reset(srds_access_t *sa__, uint8_t enable);

/*-----------------------------------------*/
/*  APIs for setting and getting OSR mode  */
/*-----------------------------------------*/

/** Configure if the TX and RX osr mode pins should only be used to configure the TX and RX osr modes.
 * Note: Calling this API will overwrite any OSR mode value previously programmed through merlin8_phy1_set_osr_mode(),
 *       merlin8_phy1_set_tx_osr_mode(), or merlin8_phy1_set_rx_osr_mode().
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] use_pins_only Allow the use of TX and RX osr mode pins (1 = Yes; 0 = No).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_set_use_osr_mode_pins_only(srds_access_t *sa__, uint8_t use_pins_only);

/** Get the TX and RX osr mode pins only status.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] tx_use_pins_only Returns whether the TX OSR mode pins only should be used (1 = Yes; 0 = No).
 * @param[out] rx_use_pins_only Returns whether the RX OSR mode pins only should be used (1 = Yes; 0 = No).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_get_use_osr_mode_pins_only(srds_access_t *sa__, uint8_t *tx_use_pins_only, uint8_t *rx_use_pins_only);

/** Configure if the RX osr mode pins should only be used to configure the RX osr mode.
 * Note: Calling this API will overwrite any OSR mode value previously programmed through merlin8_phy1_set_osr_mode()
 *        or merlin8_phy1_set_rx_osr_mode().
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] rx_use_pins_only Allow the use of RX osr mode pins (1 = Yes; 0 = No).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_set_use_rx_osr_mode_pins_only(srds_access_t *sa__, uint8_t rx_use_pins_only);

/** Get the RX osr mode pins only status.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] rx_use_pins_only Returns whether the RX OSR mode pins only should be used (1 = Yes; 0 = No).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_get_use_rx_osr_mode_pins_only(srds_access_t *sa__, uint8_t *rx_use_pins_only);

/** Configure if the RX osr mode pins should only be used to configure the TX osr mode.
 * Note: Calling this API will overwrite any OSR mode value previously programmed through merlin8_phy1_set_osr_mode()
 *        or merlin8_phy1_set_tx_osr_mode().
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] tx_use_pins_only Allow the use of TX osr mode pins (1 = Yes; 0 = No).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_set_use_tx_osr_mode_pins_only(srds_access_t *sa__, uint8_t tx_use_pins_only);

/** Get the TX osr mode pins only status.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] tx_use_pins_only Returns whether the TX OSR mode pins only should be used (1 = Yes; 0 = No).
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_get_use_tx_osr_mode_pins_only(srds_access_t *sa__, uint8_t *tx_use_pins_only);

/** Set both the TX and RX osr mode.
 * Note: Calling this API will overwrite the merlin8_phy1_set_use_osr_mode_pins_only() setting if it was previously set.
 * Note: This API should be used for configuring TX and RX osr modes when pins aren't being used. Do not write directly to the following
 *       register fields: rx_osr_mode_frc, rx_osr_mode_frc_val, tx_osr_mode_frc, tx_osr_mode_frc_val, osr_mode_frc, and osr_mode_frc_val.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] osr_mode The desired osr mode value that will be programmed.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_set_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum osr_mode);

/** Get both the TX and RX osr mode.
 * Note: A return value of 0x3f is uninitialized.
 * Note: This API returns the control status, not the actual hardware status. Use merlin8_phy1_INTERNAL_get_osr_mode()
 *       if the actual hardware status is needed.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] tx_osr_mode The returned TX osr mode.
 * @param[out] rx_osr_mode The returned RX osr mode.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_get_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum *tx_osr_mode, enum merlin8_phy1_osr_mode_enum *rx_osr_mode);

/** Set the RX osr mode.
 * Note: Calling this API will overwrite the merlin8_phy1_set_use_rx_osr_mode_pins_only() setting if it was previously set.
 * Note: This API should be used for configuring TX and RX osr modes when pins aren't being used. Do not write directly to the following
 *       register fields: rx_osr_mode_frc, rx_osr_mode_frc_val, tx_osr_mode_frc, tx_osr_mode_frc_val, osr_mode_frc, and osr_mode_frc_val.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] rx_osr_mode The desired RX osr mode value that will be programmed.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_set_rx_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum rx_osr_mode);

/** Get the RX osr mode.
 * Note: A return value of 0x3f is uninitialized.
 * Note: This API returns the control status, not the actual hardware status. Use merlin8_phy1_INTERNAL_get_osr_mode()
 *       if the actual hardware status is needed.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] rx_osr_mode The returned RX osr mode.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_get_rx_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum *rx_osr_mode);

/** Set the TX osr mode.
 * Note: Calling this API will overwrite the merlin8_phy1_set_use_tx_osr_mode_pins_only() setting if it was previously set.
 * Note: This API should be used for configuring TX and RX osr modes when pins aren't being used. Do not write directly to the following
 *       register fields: rx_osr_mode_frc, rx_osr_mode_frc_val, tx_osr_mode_frc, tx_osr_mode_frc_val, osr_mode_frc, and osr_mode_frc_val.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] tx_osr_mode The desired TX osr mode value that will be programmed.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_set_tx_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum tx_osr_mode);

/** Get the TX osr mode.
 * Note: A return value of 0x3f is uninitialized.
 * Note: This API returns the control status, not the actual hardware status. Use merlin8_phy1_INTERNAL_get_osr_mode()
 *       if the actual hardware status is needed.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] tx_osr_mode The returned TX osr mode.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_get_tx_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum *tx_osr_mode);

#ifndef PCIEG3_V1
/*---------------------------*/
/*  Merlin7 TX Analog APIs  */
/*---------------------------*/

/** Sets TxFIR DCC enable based on combinations of VCO and OSx mode.
 * Returns failcodes if called when lane is in reset.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated by invalid access (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_apply_dcc_en_cfg(srds_access_t *sa__);

#ifndef SMALL_FOOTPRINT
/** Validates Merlin7 TXFIR tap settings.
 * Returns failcodes if the combination of the TXFIR settings are invalid.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] pre      TXFIR pre   tap value (0..10)  in 1/60 Vpp.
 * @param[in] main     TXFIR main  tap value (40..60) in 1/60 Vpp.
 * @param[in] post1    TXFIR post1 tap value (0..23)  in 1/60 Vpp.
 * @param[in] post2    TXFIR post2 tap value (0..5)   in 1/60 Vpp.
 * @return Error Code generated by invalid tap settings (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_validate_txfir_cfg(srds_access_t *sa__, int8_t pre, int8_t main, int8_t post1, int8_t post2);
#endif /* SMALL_FOOTPRINT */

/** Validates and applies Merlin7 TXFIR tap settings.
 * Returns failcodes if TXFIR settings are invalid.
 * NOTE: This function must be called each time TX is reconfigured or powered up.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] pre      TXFIR pre   tap value (0..10) in 1/60 Vpp.
 * @param[in] main     TXFIR main  tap value (40..60) in 1/60 Vpp.
 * @param[in] post1    TXFIR post1 tap value (0..23) in 1/60 Vpp.
 * @param[in] post2    TXFIR post2 tap value (0..5) in 1/60 Vpp.
 * @return Error Code generated by invalid tap settings (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_apply_txfir_cfg(srds_access_t *sa__, int8_t pre, int8_t main, int8_t post1, int8_t post2);

#ifndef SMALL_FOOTPRINT
/** Read TX AFE parameters.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] param selects the parameter to read based on #merlin8_phy1_tx_afe_settings_enum.
 * @param[out] val is the returned signed value of the parameter.
 * @return Error Code generated by invalid tap settings (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_read_tx_afe(srds_access_t *sa__, enum merlin8_phy1_tx_afe_settings_enum param, int8_t *val);
#endif /* SMALL_FOOTPRINT */

/** Validates and applies Merlin7 TX CTLE High Pass Filter (HPF) settings.
 * Returns failcode if CTLE HPF setting is invalid.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] hpf_val High pass filter setting (0..4).
 * @return Error Code generated by invalid settings (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_config_tx_hpf(srds_access_t *sa__, uint8_t hpf_val);

/** Reads Merlin7 TX CTLE High Pass Filter (HPF) settings.
 * Returns failcode if CTLE HPF setting is invalid.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] hpf_val is the returned High pass filter setting (0..4) value.
 * @return Error Code generated by invalid settings (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_rd_tx_hpf_config(srds_access_t *sa__, uint8_t *hpf_val);
#endif /* MERLIN7_PCIEG3 */



/*----------------*/
/*   PMD_RX_LOCK  */
/*----------------*/

/** PMD rx lock status of current lane.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[out] *pmd_rx_lock PMD_RX_LOCK status of current lane returned by API.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_pmd_lock_status(srds_access_t *sa__, uint8_t *pmd_rx_lock);

/*--------------------------------*/
/*  Serdes TX disable/RX Restart  */
/*--------------------------------*/
/** TX Disable.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable TX disable (1 = TX Disable asserted; 0 = TX Disable removed).
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_tx_disable(srds_access_t *sa__, uint8_t enable);

#ifndef PCIEG3_V1
/** Enable/disable Restart RX and hold.
 * (Reset DSC state machine into RESTART State and hold it till disabled)
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable Restart RX and hold (1 = RX restart and hold; 0 = Release hold in restart state)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_rx_restart(srds_access_t *sa__, uint8_t enable);
#endif /* MERLIN7_PCIEG3 */



#ifndef SMALL_FOOTPRINT
/*-----------------------------*/
/*  Stop/Resume RX Adaptation  */
/*-----------------------------*/
/** Stop RX Adaptation on a Lane. Control is returned only after attempting to stop adaptation.
 * RX Adaptation needs to be stopped before modifying any of the VGA, PF or DFE taps.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable RX Adaptation stop (1 = Stop RX Adaptation on lane; 0 = Resume RX Adaptation on lane)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_stop_rx_adaptation(srds_access_t *sa__, uint8_t enable);

/** Request to stop RX Adaptation on a Lane.
 * Control will be returned immediately before adaptaion is completely stopped.
 * RX Adaptation needs to be stopped before modifying any of the VGA, PF or DFE taps.
 * To resume RX adaptation, use the merlin8_phy1_stop_rx_adaptation() API.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_request_stop_rx_adaptation(srds_access_t *sa__);
#endif /* SMALL_FOOTPRINT */

#if !defined(SMALL_FOOTPRINT)
/*------------------------------------*/
/*  Read/Write all RX AFE parameters  */
/*------------------------------------*/

/** Write to RX AFE settings.
 * Note: RX Adaptation needs to be stopped before modifying any of the VGA, PF or DFE taps.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] param Enum (#srds_rx_afe_settings_enum) to select the required RX AFE setting to be modified
 * @param[in] val Value to be written to the selected AFE setting
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_write_rx_afe(srds_access_t *sa__, enum srds_rx_afe_settings_enum param, int8_t val);
#endif /* !SMALL_FOOTPRINT */

#ifndef SMALL_FOOTPRINT
/** Read from RX AFE settings.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] param Enum (#srds_rx_afe_settings_enum) to select the required RX AFE setting to be read.
 * @param[out] val RX AFE value returned from API.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors).
 */
err_code_t merlin8_phy1_read_rx_afe(srds_access_t *sa__, enum srds_rx_afe_settings_enum param, int8_t *val);
#endif /* SMALL_FOOTPRINT */

/*-----------------------------*/
/*  TX_PI Fixed Frequency Mode */
/*-----------------------------*/

/** TX_PI Fixed Frequency Mode.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable/Disable TX_PI (1 = Enable; 0 = Disable)
 * @param[in] freq_override_val Fixed Frequency Override value (freq_override_val = desired_ppm*8192/781.25; Range: -8192 to + 8192);
 * @return Error Code generated by invalid TX_PI settings (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_tx_pi_freq_override(srds_access_t *sa__, uint8_t enable, int16_t freq_override_val);

#ifndef SMALL_FOOTPRINT
/** This function allows user to align TX clocks in selected lanes.
 * The current lane is expected to be the master lane.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable - 0: enable tx_clock_align for selected lanes with current lane as master;
 *                 1: disable tx_clock_align for selected lanes including master lane.
 * @param[in] lane_mask: bit mask for selecting lanes (0x00001111 - lane 0-3 are selected).
 * NOTE: The current lane should be selected as part of the lane_mask; else API errors out.
 * @return Error Code, if generated (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_tx_clock_align_enable(srds_access_t *sa__, uint8_t enable, uint8_t lane_mask);
#endif /* SMALL_FOOTPRINT */

/*--------------------------------------------*/
/*  Loopback and Ultra-Low Latency Functions  */
/*--------------------------------------------*/

#if !defined(PCIEG3_V1)
/** Locks TX_PI to Loop timing.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable TX_PI lock to loop timing (1 = Enable Lock; 0 = Disable Lock)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_loop_timing(srds_access_t *sa__, uint8_t enable);
#endif

#ifndef PCIEG3_V1
/** Enable/Disable Remote Loopback.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] enable Enable Remote Loopback (1 = Enable rmt_lpbk; 0 = Disable rmt_lpbk)
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin8_phy1_rmt_lpbk(srds_access_t *sa__, uint8_t enable);
#endif /* MERLIN7_PCIEG3 */

/*! @} SerdesAPIConfigTag */
/*! @} APITag */
#endif
#ifdef __cplusplus
}
#endif
