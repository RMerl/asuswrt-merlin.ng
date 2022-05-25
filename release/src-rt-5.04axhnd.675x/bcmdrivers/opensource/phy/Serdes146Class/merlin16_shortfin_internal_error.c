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
/** @file merlin16_shortfin_internal_error.c
 * Implementation of API internal error functions
 */

#include "phy_drv_merlin16.h"
#include "merlin16_shortfin_internal_error.h"
#include "merlin16_shortfin_dependencies.h"
#include "merlin16_shortfin_functions.h"
#include "merlin16_shortfin_debug_functions.h"

const char* merlin16_shortfin_INTERNAL_e2s_err_code(err_code_t err_code)
{
    switch(err_code) {
        case ERR_CODE_NONE: return "ERR_CODE_NONE";
        case ERR_CODE_INVALID_RAM_ADDR: return "ERR_CODE_INVALID_RAM_ADDR";
        case ERR_CODE_SERDES_DELAY: return "ERR_CODE_SERDES_DELAY";
        case ERR_CODE_POLLING_TIMEOUT: return "ERR_CODE_POLLING_TIMEOUT";
        case ERR_CODE_UC_CMD_POLLING_TIMEOUT: return "ERR_CODE_UC_CMD_POLLING_TIMEOUT";
        case ERR_CODE_CFG_PATT_INVALID_PATTERN: return "ERR_CODE_CFG_PATT_INVALID_PATTERN";
        case ERR_CODE_CFG_PATT_INVALID_PATT_LENGTH: return "ERR_CODE_CFG_PATT_INVALID_PATT_LENGTH";
        case ERR_CODE_CFG_PATT_LEN_MISMATCH: return "ERR_CODE_CFG_PATT_LEN_MISMATCH";
        case ERR_CODE_CFG_PATT_PATTERN_BIGGER_THAN_MAXLEN: return "ERR_CODE_CFG_PATT_PATTERN_BIGGER_THAN_MAXLEN";
        case ERR_CODE_CFG_PATT_INVALID_HEX: return "ERR_CODE_CFG_PATT_INVALID_HEX";
        case ERR_CODE_CFG_PATT_INVALID_BIN2HEX: return "ERR_CODE_CFG_PATT_INVALID_BIN2HEX";
        case ERR_CODE_CFG_PATT_INVALID_SEQ_WRITE: return "ERR_CODE_CFG_PATT_INVALID_SEQ_WRITE";
        case ERR_CODE_PATT_GEN_INVALID_MODE_SEL: return "ERR_CODE_PATT_GEN_INVALID_MODE_SEL";
        case ERR_CODE_INVALID_UCODE_LEN: return "ERR_CODE_INVALID_UCODE_LEN";
        case ERR_CODE_MICRO_INIT_NOT_DONE: return "ERR_CODE_MICRO_INIT_NOT_DONE";
        case ERR_CODE_UCODE_LOAD_FAIL: return "ERR_CODE_UCODE_LOAD_FAIL";
        case ERR_CODE_UCODE_VERIFY_FAIL: return "ERR_CODE_UCODE_VERIFY_FAIL";
        case ERR_CODE_INVALID_TEMP_IDX: return "ERR_CODE_INVALID_TEMP_IDX";
        case ERR_CODE_INVALID_PLL_CFG: return "ERR_CODE_INVALID_PLL_CFG";
        case ERR_CODE_TX_HPF_INVALID: return "ERR_CODE_TX_HPF_INVALID";
        case ERR_CODE_VGA_INVALID: return "ERR_CODE_VGA_INVALID";
        case ERR_CODE_PF_INVALID: return "ERR_CODE_PF_INVALID";
        case ERR_CODE_TX_AMP_CTRL_INVALID: return "ERR_CODE_TX_AMP_CTRL_INVALID";
        case ERR_CODE_INVALID_EVENT_LOG_WRITE: return "ERR_CODE_INVALID_EVENT_LOG_WRITE";
        case ERR_CODE_INVALID_EVENT_LOG_READ: return "ERR_CODE_INVALID_EVENT_LOG_READ";
        case ERR_CODE_UC_CMD_RETURN_ERROR: return "ERR_CODE_UC_CMD_RETURN_ERROR";
        case ERR_CODE_DATA_NOTAVAIL: return "ERR_CODE_DATA_NOTAVAIL";
        case ERR_CODE_BAD_PTR_OR_INVALID_INPUT: return "ERR_CODE_BAD_PTR_OR_INVALID_INPUT";
        case ERR_CODE_UC_NOT_STOPPED: return "ERR_CODE_UC_NOT_STOPPED";
        case ERR_CODE_UC_CRC_NOT_MATCH: return "ERR_CODE_UC_CRC_NOT_MATCH";
        case ERR_CODE_CORE_DP_NOT_RESET: return "ERR_CODE_CORE_DP_NOT_RESET";
        case ERR_CODE_LANE_DP_NOT_RESET: return "ERR_CODE_LANE_DP_NOT_RESET";
        case ERR_CODE_EXCEPTION: return "ERR_CODE_EXCEPTION";
        case ERR_CODE_INFO_TABLE_ERROR: return "ERR_CODE_INFO_TABLE_ERROR";
        case ERR_CODE_REFCLK_FREQUENCY_INVALID: return "ERR_CODE_REFCLK_FREQUENCY_INVALID";
        case ERR_CODE_PLL_DIV_INVALID: return "ERR_CODE_PLL_DIV_INVALID";
        case ERR_CODE_VCO_FREQUENCY_INVALID: return "ERR_CODE_VCO_FREQUENCY_INVALID";
        case ERR_CODE_INSUFFICIENT_PARAMETERS: return "ERR_CODE_INSUFFICIENT_PARAMETERS";
        case ERR_CODE_CONFLICTING_PARAMETERS: return "ERR_CODE_CONFLICTING_PARAMETERS";
        case ERR_CODE_BAD_LANE_COUNT: return "ERR_CODE_BAD_LANE_COUNT";
        case ERR_CODE_BAD_LANE: return "ERR_CODE_BAD_LANE";
        case ERR_CODE_UC_NOT_RESET: return "ERR_CODE_UC_NOT_RESET";
        case ERR_CODE_FFE_TAP_INVALID: return "ERR_CODE_FFE_TAP_INVALID";
        case ERR_CODE_FFE_NOT_AVAILABLE: return "ERR_CODE_FFE_NOT_AVAILABLE";
        case ERR_CODE_INVALID_RX_PAM_MODE: return "ERR_CODE_INVALID_RX_PAM_MODE";
        case ERR_CODE_INVALID_PRBS_ERR_ANALYZER_FEC_SIZE: return "ERR_CODE_INVALID_PRBS_ERR_ANALYZER_FEC_SIZE";
        case ERR_CODE_INVALID_PRBS_ERR_ANALYZER_ERR_THRESH: return "ERR_CODE_INVALID_PRBS_ERR_ANALYZER_ERR_THRESH";
        case ERR_CODE_INVALID_PRBS_ERR_ANALYZER_HIST_ERR_THRESH: return "ERR_CODE_INVALID_PRBS_ERR_ANALYZER_HIST_ERR_THRESH";
        case ERR_CODE_CFG_PATT_INVALID_PAM4: return "ERR_CODE_CFG_PATT_INVALID_PAM4";
        case ERR_CODE_RX_PI_DISP_MSB_STATUS_IS_1: return "ERR_CODE_RX_PI_DISP_MSB_STATUS_IS_1";
        case ERR_CODE_IMAGE_SIZE_NOT_SUPPORTED: return "ERR_CODE_IMAGE_SIZE_NOT_SUPPORTED";
        case ERR_CODE_TDT_CLIPPED_WAVEFORM: return "ERR_CODE_TDT_CLIPPED_WAVEFORM";
        case ERR_CODE_DBSTOP_NOT_WORKING: return "ERR_CODE_DBSTOP_NOT_WORKING";
        case ERR_CODE_INVALID_INFO_TABLE_ADDR: return "ERR_CODE_INVALID_INFO_TABLE_ADDR";
        case ERR_CODE_PRBS_CHK_HW_TIMERS_NOT_EXPIRED: return "ERR_CODE_PRBS_CHK_HW_TIMERS_NOT_EXPIRED";
        case ERR_CODE_INVALID_VALUE: return "ERR_CODE_INVALID_VALUE";
        case ERR_TDT_PATTERN_LENGTH_WR_FAILED: return "ERR_TDT_PATTERN_LENGTH_WR_FAILED";
        default:{
            switch(err_code>>8){
            case 1: return "ERR_CODE_TXFIR";
            case 2: return "ERR_CODE_DFE_TAP";
            case 3: return "ERR_CODE_DIAG";
            default: return "Invalid error code";
            }
        }
    }
}

err_code_t merlin16_shortfin_INTERNAL_print_err_msg(srds_access_t *sa__, uint16_t err_code, const char *filename, const char *func_name, uint16_t line_num)
{
    if (err_code != ERR_CODE_NONE) {
        USR_PRINTF(("ERROR:%s->%s() SerDes err_code = %s : %d\n", filename, func_name, merlin16_shortfin_INTERNAL_e2s_err_code(err_code),err_code));
    }
    return err_code;
}

err_code_t merlin16_shortfin_INTERNAL_print_err_msg_and_triage_info(srds_access_t *sa__, uint16_t err_code, const char *filename, const char *func_name, uint16_t line_num)
{
    if (err_code != ERR_CODE_NONE) {
        USR_PRINTF(("ERROR:%s->%s() SerDes err_code = %s : %d\n", filename, func_name, merlin16_shortfin_INTERNAL_e2s_err_code(err_code),err_code));
        merlin16_shortfin_INTERNAL_print_triage_info(sa__, err_code, 1, 1, line_num);
    }
    return err_code;
}
