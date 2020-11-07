/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:proprietary:standard 

   Copyright (c) 2013 Broadcom 
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
 ------------------------------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <bcm_OS_Deps.h>
#include <linux/bcm_log.h>
#include "pmd_cal.h"
#include "pmd_op.h"
#include "pmd.h"
#include "pmd_temp_cal.h"


static pmd_calibration_parameters calibration_parameters;
uint16_t cal_index;


static void set_invalid(pmd_calibration_parameters_index cal_param, uint16_t set_index)
{
    switch (cal_param)
    {
        case PMD_FILE_WATERMARK:
            calibration_parameters.watermark.valid = 0;
            break;
         case PMD_CALIBRATION_FILE_VER:
            calibration_parameters.version.valid = 0;
            break;
        case PMD_FAQ_LEVEL0_DAC:
            calibration_parameters.level_0_dac.valid = 0;
            break;
        case PMD_FAQ_LEVEL1_DAC:
            calibration_parameters.level_1_dac.valid = 0;
            break;
        case PMD_BIAS:
            calibration_parameters.bias.valid = 0;
            break;
        case PMD_MOD:
            calibration_parameters.mod.valid = 0;
            break;
        case PMD_APD:
            calibration_parameters.apd.valid = 0;
            break;
        case PMD_MPD_CONFIG:
            calibration_parameters.mpd_config.valid = 0;
            break;
        case PMD_MPD_GAINS:
            calibration_parameters.mpd_gains.valid = 0;
            break;
        case PMD_APDOI_CTRL:
            calibration_parameters.apdoi_ctrl.valid = 0;
            break;
        case PMD_RSSI_A:
            calibration_parameters.rssi_a_cal.valid = 0;
            break;
        case PMD_RSSI_B:
            calibration_parameters.rssi_b_cal.valid = 0;
            break;
        case PMD_RSSI_C:
            calibration_parameters.rssi_c_cal.valid = 0;
            break;
        case PMD_TEMP_0:
            calibration_parameters.temp_0.valid = 0;
            break;
        case PMD_TEMP_COFF_H:
            calibration_parameters.coff_h.valid = 0;
            break;
        case PMD_TEMP_COFF_L:
            calibration_parameters.coff_l.valid = 0;
            break;
        case PMD_ESC_THR:
            calibration_parameters.esc_th.valid = 0;
            break;
        case PMD_ROGUE_THR:
            calibration_parameters.rogue_th.valid = 0;
            break;
        case PMD_LEVEL_0_DAC:
            calibration_parameters.avg_level_0_dac.valid = 0;
            break;
        case PMD_AVG_LEVEL_1_DAC:
            calibration_parameters.avg_level_1_dac.valid = 0;
            break;
        case PMD_DACRANGE:
            calibration_parameters.dacrange.valid = 0;
            break;
        case PMD_LOS_THR:
            calibration_parameters.los_thr.valid = 0;
            break;
        case PMD_SAT_POS:
            calibration_parameters.sat_pos.valid = 0;
            break;
        case PMD_SAT_NEG:
            calibration_parameters.sat_neg.valid = 0;
            break;
        case PMD_EDGE_RATE:
            calibration_parameters.edge_rate.valid[set_index] = 0;
            break;
        case PMD_PREEMPHASIS_WEIGHT:
            calibration_parameters.preemphasis_weight.valid[set_index] = 0;
            break;
        case PMD_PREEMPHASIS_DELAY:
            calibration_parameters.preemphasis_delay.valid[set_index] = 0;
            break;
        case PMD_DUTY_CYCLE:
            calibration_parameters.duty_cycle.valid = 0;
            break;
        case PMD_MPD_CALIBCTRL:
            calibration_parameters.calibctrl.valid = 0;
            break;
        case PMD_TX_POWER:
            calibration_parameters.tx_power.valid = 0;
            break;
        case PMD_BIAS0:
            calibration_parameters.bias0.valid = 0;
            break;
        case PMD_TEMP_OFFSET:
            calibration_parameters.temp_offset.valid = 0;
            break;
        case PMD_BIAS_DELTA_I:
            calibration_parameters.bias_delta_i.valid = 0;
            break;
        case PMD_SLOPE_EFFICIENCY:
            calibration_parameters.slope_efficiency.valid = 0;
            break;
        case PMD_TEMP_TABLE:
            calibration_parameters.temp_table.valid = 0;
            break;
        case PMD_ADF_LOS_THRESHOLDS:
            calibration_parameters.adf_los_thresholds.valid = 0;
            break;
        case PMD_ADF_LOS_LEAKY_BUCKET:
            calibration_parameters.adf_los_leaky_bucket.valid = 0;
            break;
        case PMD_COMPENSATION:
            calibration_parameters.compensation.valid = 0;
            break;
        default:
            BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Invalid calibration param \n");
            break;
    }
}

int pmd_cal_param_set(pmd_calibration_parameters_index cal_param, int32_t val, uint16_t set_index)
{
    int16_t val_word;

    val_word = val & 0xffff;

    if (CAL_FILE_INVALID_ENTRANCE == val)
    {
        set_invalid(cal_param, set_index);
        return PMD_OPERATION_SUCCESS;
    }

    switch (cal_param)
    {
    case PMD_FAQ_LEVEL0_DAC:
        calibration_parameters.level_0_dac.val = val_word;
        calibration_parameters.level_0_dac.valid = 1;
        break;
    case PMD_FAQ_LEVEL1_DAC:
        calibration_parameters.level_1_dac.val = val_word;
        calibration_parameters.level_1_dac.valid = 1;
        break;
    case PMD_BIAS:
        calibration_parameters.bias.val = val_word;
        calibration_parameters.bias.valid = 1;
        break;
    case PMD_MOD:
        calibration_parameters.mod.val = val_word;
        calibration_parameters.mod.valid = 1;
        break;
    case PMD_APD:
        calibration_parameters.apd.val = val_word;
        calibration_parameters.apd.valid = 1;
        break;
    case PMD_MPD_CONFIG:
        calibration_parameters.mpd_config.val = val_word;
        calibration_parameters.mpd_config.valid = 1;
        break;
    case PMD_MPD_GAINS:
        calibration_parameters.mpd_gains.val = val_word;
        calibration_parameters.mpd_gains.valid = 1;
        break;
    case PMD_APDOI_CTRL:
        calibration_parameters.apdoi_ctrl.val = val_word;
        calibration_parameters.apdoi_ctrl.valid = 1;
        break;
    case PMD_RSSI_A:
        calibration_parameters.rssi_a_cal.val = val;
        calibration_parameters.rssi_a_cal.valid = 1;
        break;
    case PMD_RSSI_B:
        calibration_parameters.rssi_b_cal.val = val;
        calibration_parameters.rssi_b_cal.valid = 1;
        break;
    case PMD_RSSI_C:
        calibration_parameters.rssi_c_cal.val = val;
        calibration_parameters.rssi_c_cal.valid = 1;
        break;
    case PMD_TEMP_0:
        calibration_parameters.temp_0.val = val_word;
        calibration_parameters.temp_0.valid = 1;
        break;
    case PMD_TEMP_COFF_H:
        calibration_parameters.coff_h.val = val_word;
        calibration_parameters.coff_h.valid = 1;
        break;
    case PMD_TEMP_COFF_L:
        calibration_parameters.coff_l.val = val_word;
        calibration_parameters.coff_l.valid = 1;
        break;
    case PMD_ESC_THR:
        calibration_parameters.esc_th.val = val_word;
        calibration_parameters.esc_th.valid = 1;
        break;
    case PMD_ROGUE_THR:
        calibration_parameters.rogue_th.val = val_word;
        calibration_parameters.rogue_th.valid = 1;
        break;
    case PMD_LEVEL_0_DAC:
        calibration_parameters.avg_level_0_dac.val = val_word;
        calibration_parameters.avg_level_0_dac.valid = 1;
        break;
    case PMD_AVG_LEVEL_1_DAC:
        calibration_parameters.avg_level_1_dac.val = val_word;
        calibration_parameters.avg_level_1_dac.valid = 1;
        break;
    case PMD_DACRANGE:
        calibration_parameters.dacrange.val = val_word;
        calibration_parameters.dacrange.valid = 1;
        break;
    case PMD_LOS_THR:
        calibration_parameters.los_thr.val = val_word;
        calibration_parameters.los_thr.valid = 1;
        break;
    case PMD_SAT_POS:		
        calibration_parameters.sat_pos.val = val_word;
        calibration_parameters.sat_pos.valid = 1;
        break;
    case PMD_SAT_NEG:		
        calibration_parameters.sat_neg.val = val_word;
        calibration_parameters.sat_neg.valid = 1;
        break;
    case PMD_EDGE_RATE:
        calibration_parameters.edge_rate.val[set_index] = val_word;
        calibration_parameters.edge_rate.valid[set_index] = 1;
        break;
    case PMD_PREEMPHASIS_WEIGHT:
        calibration_parameters.preemphasis_weight.val[set_index] = val_word;
        calibration_parameters.preemphasis_weight.valid[set_index] = 1;
        break;
    case PMD_PREEMPHASIS_DELAY:
        calibration_parameters.preemphasis_delay.val[set_index] = val;
        calibration_parameters.preemphasis_delay.valid[set_index] = 1;
        break;
    case PMD_DUTY_CYCLE:
    	calibration_parameters.duty_cycle.val = val;
    	calibration_parameters.duty_cycle.valid = 1;
    	break;
    case PMD_MPD_CALIBCTRL:
        calibration_parameters.calibctrl.val = val;
        calibration_parameters.calibctrl.valid = 1;
        break;
    case PMD_TX_POWER:
        calibration_parameters.tx_power.val = val_word;
        calibration_parameters.tx_power.valid = 1;
        break;
    case PMD_BIAS0:
        calibration_parameters.bias0.val = val_word;
        calibration_parameters.bias0.valid = 1;
        break;		
    case PMD_TEMP_OFFSET:
        calibration_parameters.temp_offset.val = val;
        calibration_parameters.temp_offset.valid = 1;
        break;
    case PMD_BIAS_DELTA_I:
        calibration_parameters.bias_delta_i.val = val;
        calibration_parameters.bias_delta_i.valid = 1;
        break;
    case PMD_SLOPE_EFFICIENCY:
        calibration_parameters.slope_efficiency.val = val;
        calibration_parameters.slope_efficiency.valid = 1;
        break;
    case PMD_ADF_LOS_THRESHOLDS:
        calibration_parameters.adf_los_thresholds.val = val;
        calibration_parameters.adf_los_thresholds.valid = 1;
        break;
    case PMD_ADF_LOS_LEAKY_BUCKET:
        calibration_parameters.adf_los_leaky_bucket.val = val_word;
        calibration_parameters.adf_los_leaky_bucket.valid = 1;
        break;
    case PMD_COMPENSATION:
        calibration_parameters.compensation.val = val;
        calibration_parameters.compensation.valid = 1;
        break;
    default:
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Modification of calibration param number %d is not supported \n", cal_param);
        return ERR_CAL_PARAM_INDEX_OUT_OF_RANGE;
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, " calibration_parameters[ %d ] = %x \n", cal_param, val_word);
    return PMD_OPERATION_SUCCESS;
}

int pmd_cal_param_get(pmd_calibration_parameters_index cal_param, int32_t *val, uint16_t get_index)
{
    uint8_t valid;

    switch (cal_param)
    {
    case PMD_FILE_WATERMARK:
        *val = calibration_parameters.watermark.val;
        valid = calibration_parameters.watermark.valid;
        break;
    case PMD_CALIBRATION_FILE_VER:
        *val = calibration_parameters.version.val;
        valid = calibration_parameters.version.valid;
        break;
    case PMD_FAQ_LEVEL0_DAC:
        *val = calibration_parameters.level_0_dac.val;
        valid = calibration_parameters.level_0_dac.valid;
        break;
    case PMD_FAQ_LEVEL1_DAC:
        *val = calibration_parameters.level_1_dac.val;
        valid = calibration_parameters.level_1_dac.valid;
        break;
    case PMD_BIAS:
        *val = calibration_parameters.bias.val;
        valid = calibration_parameters.bias.valid;
        break;
    case PMD_MOD:
        *val = calibration_parameters.mod.val;
        valid = calibration_parameters.mod.valid;
        break;
    case PMD_APD:
        *val = calibration_parameters.apd.val;
        valid = calibration_parameters.apd.valid;
        break;
    case PMD_MPD_CONFIG:
        *val = calibration_parameters.mpd_config.val;
        valid = calibration_parameters.mpd_config.valid;
        break;
    case PMD_MPD_GAINS:
        *val = calibration_parameters.mpd_gains.val;
        valid = calibration_parameters.mpd_gains.valid;
        break;
    case PMD_APDOI_CTRL:
        *val = calibration_parameters.apdoi_ctrl.val;
        valid = calibration_parameters.apdoi_ctrl.valid;
        break;
    case PMD_RSSI_A:
        *val = calibration_parameters.rssi_a_cal.val;
        valid = calibration_parameters.rssi_a_cal.valid;
        break;
    case PMD_RSSI_B:
        *val = calibration_parameters.rssi_b_cal.val;
        valid = calibration_parameters.rssi_b_cal.valid;
        break;
    case PMD_RSSI_C:
        *val = calibration_parameters.rssi_c_cal.val;
        valid = calibration_parameters.rssi_c_cal.valid;
        break;
    case PMD_TEMP_0:
        *val = calibration_parameters.temp_0.val;
        valid = calibration_parameters.temp_0.valid;
        break;
    case PMD_TEMP_COFF_H:
        *val = calibration_parameters.coff_h.val;
        valid = calibration_parameters.coff_h.valid;
        break;
    case PMD_TEMP_COFF_L:
        *val = calibration_parameters.coff_l.val;
        valid = calibration_parameters.coff_l.valid;
        break;
    case PMD_ESC_THR:
        *val = calibration_parameters.esc_th.val;
        valid = calibration_parameters.esc_th.valid;
        break;
    case PMD_ROGUE_THR:
        *val = calibration_parameters.rogue_th.val;
        valid = calibration_parameters.rogue_th.valid;
        break;
    case PMD_LEVEL_0_DAC:
        *val = calibration_parameters.avg_level_0_dac.val;
        valid = calibration_parameters.avg_level_0_dac.valid;
        break;
    case PMD_AVG_LEVEL_1_DAC:
        *val = calibration_parameters.avg_level_1_dac.val;
        valid = calibration_parameters.avg_level_1_dac.valid;
        break;
    case PMD_DACRANGE:
        *val = calibration_parameters.dacrange.val;
        valid = calibration_parameters.dacrange.valid;
        break;
    case PMD_LOS_THR:
        *val = calibration_parameters.los_thr.val;
        valid = calibration_parameters.los_thr.valid;
        break;
    case PMD_SAT_POS:
        *val = calibration_parameters.sat_pos.val;
        valid = calibration_parameters.sat_pos.valid;
        break;
    case PMD_SAT_NEG:				
        *val = calibration_parameters.sat_neg.val;
        valid = calibration_parameters.sat_neg.valid;
        break;
    case PMD_EDGE_RATE:
        *val = calibration_parameters.edge_rate.val[get_index];
        valid = calibration_parameters.edge_rate.valid[get_index];
        break;
    case PMD_PREEMPHASIS_WEIGHT:
        *val = calibration_parameters.preemphasis_weight.val[get_index];
        valid = calibration_parameters.preemphasis_weight.valid[get_index];
        break;
    case PMD_PREEMPHASIS_DELAY:
        *val = calibration_parameters.preemphasis_delay.val[get_index];
        valid = calibration_parameters.preemphasis_delay.valid[get_index];
        break;
    case PMD_DUTY_CYCLE:
    	*val = calibration_parameters.duty_cycle.val;
    	valid = calibration_parameters.duty_cycle.valid;
    	break;
    case PMD_MPD_CALIBCTRL:
        *val = calibration_parameters.calibctrl.val;
        valid = calibration_parameters.calibctrl.valid;
        break;
    case PMD_TX_POWER:
        *val = calibration_parameters.tx_power.val;
        valid = calibration_parameters.tx_power.valid;
        break;
    case PMD_BIAS0:
        *val = calibration_parameters.bias0.val;
        valid = calibration_parameters.bias0.valid;
        break;
    case PMD_TEMP_OFFSET:
        *val = calibration_parameters.temp_offset.val;
        valid = calibration_parameters.temp_offset.valid;
        break;
    case PMD_BIAS_DELTA_I:
        *val = calibration_parameters.bias_delta_i.val;
        valid = calibration_parameters.bias_delta_i.valid;
        break;
    case PMD_SLOPE_EFFICIENCY:
        *val = calibration_parameters.slope_efficiency.val;
        valid = calibration_parameters.slope_efficiency.valid;
        break;
    case PMD_ADF_LOS_THRESHOLDS:
        *val = calibration_parameters.adf_los_thresholds.val;
        valid = calibration_parameters.adf_los_thresholds.valid;
        break;
    case PMD_ADF_LOS_LEAKY_BUCKET:
        *val = calibration_parameters.adf_los_leaky_bucket.val;
        valid = calibration_parameters.adf_los_leaky_bucket.valid;
        break;
    case PMD_COMPENSATION:
        *val = calibration_parameters.compensation.val;
        valid = calibration_parameters.compensation.valid;
        break;
    default:
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Read of calibration param number %d is not supported \n", cal_param);
        return ERR_CAL_PARAM_INDEX_OUT_OF_RANGE;
        break;
    }

    if (!valid)
    {
        *val = CAL_FILE_INVALID_ENTRANCE;
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, " calibration param %d invalid \n", cal_param);
        return ERR_CAL_PARAM_VALUE_INVALID;
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, " calibration_parameters[ %d ] = %x \n", cal_param, *val);
    return PMD_OPERATION_SUCCESS;
}

int pmd_cal_is_config_valid(pmd_calibration_parameters_index *pmd_cal_param_valid, uint8_t size)
{
    uint8_t index = 0;

    while (index < size)
    {
        switch (pmd_cal_param_valid[index])
        {
            case PMD_FILE_WATERMARK:
                if (!(calibration_parameters.watermark.valid))
                    goto exit_error_print;
                break;
            case PMD_CALIBRATION_FILE_VER:
                if (!(calibration_parameters.version.valid))
                    goto exit_error_print;
                break;
            case PMD_FAQ_LEVEL0_DAC:
                if (!(calibration_parameters.level_0_dac.valid))
                    goto exit_error_print;
                break;
            case PMD_FAQ_LEVEL1_DAC:
                if (!(calibration_parameters.level_1_dac.valid))
                    goto exit_error_print;
                break;
            case PMD_BIAS:
                if (!(calibration_parameters.bias.valid))
                    goto exit_error_print;
                break;
            case PMD_MOD:
                if (!(calibration_parameters.mod.valid))
                    goto exit_error_print;
                break;
            case PMD_APD:
                if (!(calibration_parameters.apd.valid))
                    goto exit_error_print;
                break;
            case PMD_MPD_CONFIG:
                if (!(calibration_parameters.mpd_config.valid))
                    goto exit_error_print;
                break;
            case PMD_MPD_GAINS:
                if (!(calibration_parameters.mpd_gains.valid))
                    goto exit_error_print;
                break;
            case PMD_APDOI_CTRL:
                if (!(calibration_parameters.apdoi_ctrl.valid))
                    goto exit_error_print;
                break;
            case PMD_RSSI_A:
                if (!(calibration_parameters.rssi_a_cal.valid))
                    goto exit_error_print;
                break;
            case PMD_RSSI_B:
                if (!(calibration_parameters.rssi_b_cal.valid))
                    goto exit_error_print;
                break;
            case PMD_RSSI_C:
                if (!(calibration_parameters.rssi_c_cal.valid))
                    goto exit_error_print;
                break;
            case PMD_TEMP_0:
                if (!(calibration_parameters.temp_0.valid))
                    goto exit_error_print;
                break;
            case PMD_TEMP_COFF_H:
                if (!(calibration_parameters.coff_h.valid))
                    goto exit_error_print;
                break;
            case PMD_TEMP_COFF_L:
                if (!(calibration_parameters.coff_l.valid))
                    goto exit_error_print;
                break;
            case PMD_ESC_THR:
                if (!(calibration_parameters.esc_th.valid))
                    goto exit_error_print;
                break;
            case PMD_ROGUE_THR:
                if (!(calibration_parameters.rogue_th.valid))
                    goto exit_error_print;
                break;
            case PMD_LEVEL_0_DAC:
                if (!(calibration_parameters.avg_level_0_dac.valid))
                    goto exit_error_print;
                break;
            case PMD_AVG_LEVEL_1_DAC:
                if (!(calibration_parameters.avg_level_1_dac.valid))
                    goto exit_error_print;
                break;
            case PMD_DACRANGE:
                if (!(calibration_parameters.dacrange.valid))
                    goto exit_error_print;
                break;
            case PMD_LOS_THR:
                if (!(calibration_parameters.los_thr.valid))
                    goto exit_error_print;
                break;
            case PMD_SAT_POS:
                if (!(calibration_parameters.sat_pos.valid))
                    goto exit_error_print;
                break;
            case PMD_SAT_NEG:
                if (!(calibration_parameters.sat_neg.valid))
                    goto exit_error_print;
                break;
            case PMD_EDGE_RATE:
                if (!(calibration_parameters.edge_rate.valid[cal_index]))
                    goto exit_error_print;
                break;
            case PMD_PREEMPHASIS_WEIGHT:
                if (!(calibration_parameters.preemphasis_weight.valid[cal_index]))
                    goto exit_error_print;
                break;
            case PMD_PREEMPHASIS_DELAY:
                if (!(calibration_parameters.preemphasis_delay.valid[cal_index]))
                    goto exit_error_print;
                break;
            case PMD_DUTY_CYCLE:
                if (!(calibration_parameters.duty_cycle.valid))
                    goto exit_error_print;
                break;
            case PMD_MPD_CALIBCTRL:
                if (!(calibration_parameters.calibctrl.valid))
                    goto exit_error_print;
                break;
            case PMD_TX_POWER:
                if (!(calibration_parameters.tx_power.valid))
                    goto exit_error_print;
                break;
            case PMD_BIAS0:
                if (!(calibration_parameters.bias0.valid))
                    goto exit_error_print;
                break;
            case PMD_TEMP_OFFSET:
                if (!(calibration_parameters.temp_offset.valid))
                    goto exit_error_print;
                break;
            case PMD_BIAS_DELTA_I:
                if (!(calibration_parameters.bias_delta_i.valid))
                    goto exit_error_print;
                break;
            case PMD_SLOPE_EFFICIENCY:
                if (!(calibration_parameters.slope_efficiency.valid))
                    goto exit_error_print;
                break;
            case PMD_ADF_LOS_THRESHOLDS:
                if (!(calibration_parameters.adf_los_thresholds.valid))
                    goto exit_error_print;
                break;
            case PMD_ADF_LOS_LEAKY_BUCKET:
                if (!(calibration_parameters.adf_los_leaky_bucket.valid))
                    goto exit_error_print;
                break;
            case PMD_COMPENSATION:
                if (!(calibration_parameters.compensation.valid))
                    goto exit_error_print;
                break;
            default:
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Read of calibration param number %d is not supported \n",
                    pmd_cal_param_valid[index]);
                return ERR_CAL_PARAM_INDEX_OUT_OF_RANGE;
                break;
        }
        index++;
    }

    return PMD_OPERATION_SUCCESS;

exit_error_print:
    BCM_LOG_ERROR(BCM_LOG_ID_PMD, "calibration param %d is not valid\n", pmd_cal_param_valid[index]);
    return ERR_CAL_PARAM_VALUE_INVALID;
}

void pmd_cal_param_init(pmd_calibration_parameters *calibration_parameters_from_json)
{
    switch (pmd_wan_type)
    {
        case PMD_XGPON1_10_2_WAN:
        {
            cal_index = US_2G_INDEX;
            break;
        }
        case PMD_EPON_2_1_WAN:
        case PMD_GPON_2_1_WAN:
        case PMD_EPON_1_1_WAN:
        case PMD_GBE_1_1_WAN:
        case PMD_EPON_10_1_WAN:
        {
            cal_index = US_1G_INDEX;
            break;
        }
        default:
        {
            printk("pmd: unknown wan_type (%d)in cal init", pmd_wan_type);
            break;
        }
    }

    if (calibration_parameters_from_json)
    {
        memcpy(&calibration_parameters, calibration_parameters_from_json, sizeof(pmd_calibration_parameters));
    }
    else
    {
        memset(&calibration_parameters, 0x00, sizeof(pmd_calibration_parameters));
    }
}
