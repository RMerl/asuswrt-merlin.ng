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

/**********************************************************************************
 **********************************************************************************
 *  File Name     :  srds_api_err_code.h                                          *
 *  Created On    :  18/09/2013                                                   *
 *  Created By    :  Kiran Divakar                                                *
 *  Description   :  Header file with Error Code enums                            *
 *  Revision      :   *
 *                                                                                *
 **********************************************************************************
 **********************************************************************************/

/** @file srds_api_err_code.h
 * Error Code enumerations
 */

#ifndef SRDS_API_ERR_CODE_PUBLIC_H
#define SRDS_API_ERR_CODE_PUBLIC_H

#ifndef EXCLUDE_STD_HEADERS
#include <stdint.h>
#endif

typedef uint16_t err_code_t;

/** ERROR CODE Enum */
enum srds_err_code_enum {
    SRDS_ERR_CODE_NONE = 0,
    SRDS_ERR_CODE_INVALID_RAM_ADDR,
    SRDS_ERR_CODE_SERDES_DELAY,
    SRDS_ERR_CODE_POLLING_TIMEOUT,
    SRDS_ERR_CODE_CFG_PATT_INVALID_PATTERN,
    SRDS_ERR_CODE_CFG_PATT_INVALID_PATT_LENGTH,
    SRDS_ERR_CODE_CFG_PATT_LEN_MISMATCH,
    SRDS_ERR_CODE_CFG_PATT_PATTERN_BIGGER_THAN_MAXLEN,
    SRDS_ERR_CODE_CFG_PATT_INVALID_HEX,
    SRDS_ERR_CODE_CFG_PATT_INVALID_BIN2HEX,
    SRDS_ERR_CODE_CFG_PATT_INVALID_SEQ_WRITE,
    SRDS_ERR_CODE_PATT_GEN_INVALID_MODE_SEL,
    SRDS_ERR_CODE_INVALID_UCODE_LEN,
    SRDS_ERR_CODE_MICRO_INIT_NOT_DONE,
    SRDS_ERR_CODE_UCODE_LOAD_FAIL,
    SRDS_ERR_CODE_UCODE_VERIFY_FAIL,
    SRDS_ERR_CODE_INVALID_TEMP_IDX,
    SRDS_ERR_CODE_INVALID_PLL_CFG,
    SRDS_ERR_CODE_TX_HPF_INVALID,
    SRDS_ERR_CODE_VGA_INVALID,
    SRDS_ERR_CODE_PF_INVALID,
    SRDS_ERR_CODE_TX_AMP_CTRL_INVALID,
    SRDS_ERR_CODE_INVALID_EVENT_LOG_WRITE,
    SRDS_ERR_CODE_INVALID_EVENT_LOG_READ,
    SRDS_ERR_CODE_UC_CMD_RETURN_ERROR,
    SRDS_ERR_CODE_DATA_NOTAVAIL,
    SRDS_ERR_CODE_BAD_PTR_OR_INVALID_INPUT,
    SRDS_ERR_CODE_UC_NOT_STOPPED,
    SRDS_ERR_CODE_UC_CRC_NOT_MATCH,
    SRDS_ERR_CODE_CORE_DP_NOT_RESET,
    SRDS_ERR_CODE_LANE_DP_NOT_RESET,
    SRDS_ERR_CODE_EXCEPTION,
    SRDS_ERR_CODE_INFO_TABLE_ERROR,
    SRDS_ERR_CODE_REFCLK_FREQUENCY_INVALID,
    SRDS_ERR_CODE_PLL_DIV_INVALID,
    SRDS_ERR_CODE_VCO_FREQUENCY_INVALID,
    SRDS_ERR_CODE_INSUFFICIENT_PARAMETERS,
    SRDS_ERR_CODE_CONFLICTING_PARAMETERS,
    SRDS_ERR_CODE_BAD_LANE_COUNT,
    SRDS_ERR_CODE_BAD_LANE,
    SRDS_ERR_CODE_UC_NOT_RESET,
    SRDS_ERR_CODE_FFE_TAP_INVALID,
    SRDS_ERR_CODE_FFE_NOT_AVAILABLE,
    SRDS_ERR_CODE_INVALID_RX_PAM_MODE,
    SRDS_ERR_CODE_INVALID_PRBS_ERR_ANALYZER_FEC_SIZE, 
    SRDS_ERR_CODE_INVALID_PRBS_ERR_ANALYZER_ERR_THRESH, 
    SRDS_ERR_CODE_INVALID_PRBS_ERR_ANALYZER_HIST_ERR_THRESH,
    SRDS_ERR_CODE_CFG_PATT_INVALID_PAM4,
    SRDS_ERR_CODE_INVALID_TDT_PATTERN_FOR_HW_MODE,
    SRDS_ERR_CODE_ODD_PRE_OR_POST_TAP_INPUT,
    SRDS_ERR_CODE_RX_PI_DISP_MSB_STATUS_IS_1,
    SRDS_ERR_CODE_IMAGE_SIZE_NOT_SUPPORTED,
    SRDS_ERR_CODE_TDT_CLIPPED_WAVEFORM,
    SRDS_ERR_CODE_DBSTOP_NOT_WORKING,
    SRDS_ERR_CODE_UC_CMD_POLLING_TIMEOUT,
    SRDS_ERR_CODE_INVALID_INFO_TABLE_ADDR,
    SRDS_ERR_CODE_PRBS_CHK_HW_TIMERS_NOT_EXPIRED,
    SRDS_ERR_CODE_INVALID_VALUE,
    SRDS_ERR_CODE_SRDS_REG_ACCESS_FAIL,
    SRDS_ERR_CODE_TXFIR   = 1 << 8,
    SRDS_ERR_CODE_DFE_TAP = 2 << 8,
    SRDS_ERR_CODE_DIAG    = 3 << 8,
    SRDS_ERR_TDT_PATTERN_LENGTH_WR_FAILED=255
};

/** TXFIR Error Codes Enum */
enum srds_txfir_failcodes {
    SRDS_ERR_CODE_TXFIR_PRE_INVALID         = ERR_CODE_TXFIR +   1,
    SRDS_ERR_CODE_TXFIR_MAIN_INVALID        = ERR_CODE_TXFIR +   2,
    SRDS_ERR_CODE_TXFIR_POST1_INVALID       = ERR_CODE_TXFIR +   4,
    SRDS_ERR_CODE_TXFIR_POST2_INVALID       = ERR_CODE_TXFIR +   8,
    SRDS_ERR_CODE_TXFIR_POST3_INVALID       = ERR_CODE_TXFIR +  16,
    SRDS_ERR_CODE_TXFIR_V2_LIMIT            = ERR_CODE_TXFIR +  32,
    SRDS_ERR_CODE_TXFIR_SUM_LIMIT           = ERR_CODE_TXFIR +  64,
    SRDS_ERR_CODE_TXFIR_PRE_POST1_SUM_LIMIT = ERR_CODE_TXFIR + 128
};

/** DFE Tap Error Codes Enum */
enum srds_dfe_tap_failcodes {
    SRDS_ERR_CODE_DFE1_INVALID        = ERR_CODE_DFE_TAP +  1,
    SRDS_ERR_CODE_DFE2_INVALID        = ERR_CODE_DFE_TAP +  2,
    SRDS_ERR_CODE_DFE3_INVALID        = ERR_CODE_DFE_TAP +  4,
    SRDS_ERR_CODE_DFE4_INVALID        = ERR_CODE_DFE_TAP +  8,
    SRDS_ERR_CODE_DFE5_INVALID        = ERR_CODE_DFE_TAP + 16,
    SRDS_ERR_CODE_DFE_TAP_IDX_INVALID = ERR_CODE_DFE_TAP + 32
};

/** DIAG Error Codes Enum */
enum srds_diag_failcodes {
    SRDS_ERR_CODE_DIAG_TIMEOUT               = ERR_CODE_DIAG + 1,
    SRDS_ERR_CODE_DIAG_INVALID_STATUS_RETURN = ERR_CODE_DIAG + 2,
    SRDS_ERR_CODE_DIAG_SCAN_NOT_COMPLETE     = ERR_CODE_DIAG + 3,
    SRDS_ERR_CODE_DIAG_TIMESTAMP_FAIL        = ERR_CODE_DIAG + 4,
    SRDS_ERR_CODE_DIAG_SCAN_NO_PMD_LOCK      = ERR_CODE_DIAG + 5
};
#endif
