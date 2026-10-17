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
 *  File Name     :  merlin8_phy1_diag.c                                             *
 *  Created On    :  03 Nov 2015                                                   *
 *  Created By    :  Brent Roberts                                                 *
 *  Description   :  Diagnostic APIs for Serdes IPs                                *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/


#ifndef EXCLUDE_STD_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#endif

#include "merlin8_phy1_diag.h"
#include "merlin8_phy1_access.h"
#include "merlin8_phy1_config.h"
#include "merlin8_phy1_enum.h"
#include "merlin8_phy1_debug_functions.h"
#include "merlin8_phy1_functions.h"
#include "merlin8_phy1_internal.h"
#include "merlin8_phy1_internal_error.h"
#include "merlin8_phy1_prbs.h"
#include "merlin8_phy1_select_defns.h"
#include "common/srds_api_uc_common.h"

static uint8_t eyeReportFormat;

/** @file
 *
 */




#ifndef SMALL_FOOTPRINT

/************************************/
/*  Display Eye Scan                */
/************************************/

/* Timestamp check to see if heartbeat timer is programmed correctly for the COMCLK frequency it is running at */
err_code_t merlin8_phy1_test_uc_timestamp(srds_access_t *sa__) {
    return merlin8_phy1_INTERNAL_test_uc_timestamp_with_print_options(sa__, 0xFF);
}

static void _merlin8_phy1_display_eye_scan_ignored_lane(srds_access_t *sa__, uint16_t usr_diag_status) {
    UNUSED(usr_diag_status);
    USR_PRINTF(("         : -"));
}

/* This is best method for terminal ASCII display */
err_code_t merlin8_phy1_display_eye_scan(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint32_t stripe[EYE_SCAN_DISPLAY_SRIPE_SIZE] = {0};
    uint16_t status = 0;
    int16_t  y,y_max=0,y_step=0;
    uint8_t eye_slice;
    uint16_t usr_diag_status;
    err_code_t Exit_Status = ERR_CODE_NONE;

    Exit_Status = merlin8_phy1_INTERNAL_eye_scan_setup(sa__, 0, &y_max, &y_step, &usr_diag_status);
    if((Exit_Status == ERR_CODE_RX_TUNING_NOT_DONE) || (Exit_Status == ERR_CODE_DIAG_SCAN_NO_PMD_LOCK)) {
        Exit_Status = ERR_CODE_NONE;
        goto Exit;
    }

    if (y_step == 0) {
        merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
        Exit_Status = ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
        goto Exit;
    }
    CFUN(merlin8_phy1_display_eye_scan_header(sa__, 1, &usr_diag_status));
    eye_slice = (usr_diag_status & DIAG_EYE_SCAN_RANGE_MASK) == DIAG_EYE_SCAN_SLICE;
    for (y = y_max;y>=-y_max;y=(int16_t)(y-y_step)) {
        const uint8_t stripe_size = EYE_SCAN_DISPLAY_SRIPE_SIZE;
        uint8_t enable_read, buffer_idx;

        if(eye_slice) {
            enable_read = (((y_max-y)/y_step)%stripe_size) == 0;
            buffer_idx = (uint8_t)(((y_max-y)/y_step)%stripe_size);
        } else {
            enable_read = 1;
            buffer_idx = 0;
        }
        if(enable_read) {
            Exit_Status = merlin8_phy1_read_eye_scan_stripe(sa__, &stripe[0], &status);
            if (Exit_Status) {
                EFUN_PRINTF(("ERROR: merlin8_phy1_read_eye_scan_stripe() returned: %s, (status = 0x%x)\n", merlin8_phy1_INTERNAL_e2s_err_code(Exit_Status), status));
                goto Exit;
            }
        }
        CFUN(merlin8_phy1_display_eye_scan_stripe(sa__, y, &stripe[buffer_idx], usr_diag_status));
        USR_PRINTF(("\n"));
    }
    CFUN(merlin8_phy1_display_eye_scan_footer(sa__, 1, &usr_diag_status));
    /* stop acquisition */
    CFUN(merlin8_phy1_meas_eye_scan_done(sa__));
Exit:
    return(merlin8_phy1_error_report(sa__, Exit_Status));
}

err_code_t merlin8_phy1_display_all_eye_scan(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint8_t num_lanes, lane;
    uint8_t lane_orig = merlin8_phy1_acc_get_lane(sa__);
    int16_t i[NUM_LANES_MAX] = {0};
    int16_t i_step[NUM_LANES_MAX] = {0};
    int16_t i_max[NUM_LANES_MAX] = {0};
    uint8_t lane_mask = 0;
    err_code_t err_code;
    uint8_t eye_slice[NUM_LANES_MAX] = {0};
#ifdef __KERNEL__  /* may not be enough stack in linux driver */
    static
#endif
    uint32_t stripe[NUM_LANES_MAX*EYE_SCAN_DISPLAY_SRIPE_SIZE];
    uint16_t max_num_steps = 0;
    uint16_t usr_diag_status[NUM_LANES_MAX] = {0};
    err_code_t Exit_Status = ERR_CODE_NONE;

    ESTM(num_lanes = rdc_revid_multiplicity());
    ENULL_MEMSET(stripe, 0, sizeof(stripe));

    for ( lane=0; lane < num_lanes; lane++) {
        err_code = merlin8_phy1_acc_set_lane(sa__, lane);
        if (err_code) {
            USR_PRINTF(("%s WARN: Unable to switch to lane(%d) and so ignoring lane for for 2D eye scan\n", API_FUNCTION_NAME, lane));
            continue;
        }
        err_code = merlin8_phy1_INTERNAL_eye_scan_setup(sa__, 0, &i_max[lane], &i_step[lane], &usr_diag_status[lane]);
        if (err_code) {
            uint8_t core;
            core = merlin8_phy1_acc_get_core(sa__);
            USR_PRINTF(("core(%d), lane(%d) ignored for 2D eye scan\n", core, lane));
        } else {
            uint16_t num_steps = (uint16_t)(((i_max[lane] << 1) / i_step[lane]) + 1);
            max_num_steps =  (max_num_steps > num_steps) ? max_num_steps : num_steps;
            lane_mask |= (uint8_t)(1<<lane);
            eye_slice[lane] = ((usr_diag_status[lane] & DIAG_EYE_SCAN_RANGE_MASK) == DIAG_EYE_SCAN_SLICE);
            i[lane] = i_max[lane];
        }
    }
    if(lane_mask) {
        CFUN(merlin8_phy1_display_eye_scan_header(sa__, num_lanes, &usr_diag_status[0]));
    }
    else {
        goto Exit;
    }

    while(max_num_steps--) {
        uint16_t status;
        const uint8_t stripe_size = EYE_SCAN_DISPLAY_SRIPE_SIZE;
        uint8_t enable_read, buffer_idx;
        for(lane=0; lane < num_lanes; lane++) {
            if(lane_mask & (1<<lane)) {
                err_code_t local_error = ERR_CODE_NONE;
                if(i[lane] >= -i_max[lane]) {
                    local_error = merlin8_phy1_acc_set_lane(sa__, lane);
                    if (local_error) {
                        lane_mask &= (uint8_t)~(1<<lane);
                        _merlin8_phy1_display_eye_scan_ignored_lane(sa__, usr_diag_status[lane]);
                        continue;
                    }
                    status = 0;
                    if(eye_slice[lane]) {
                        enable_read = (((i_max[lane]-i[lane])/i_step[lane])%stripe_size) == 0;
                        buffer_idx = (uint8_t)(((i_max[lane]-i[lane])/i_step[lane])%stripe_size);
                    } else {
                        enable_read = 1;
                        buffer_idx = 0;
                    }

                    if(enable_read) {
                        local_error = merlin8_phy1_read_eye_scan_stripe(sa__, &stripe[lane*EYE_SCAN_DISPLAY_SRIPE_SIZE], &status);
                        if (local_error) {
                            lane_mask &= (uint8_t)~(1<<lane);
                            _merlin8_phy1_display_eye_scan_ignored_lane(sa__, usr_diag_status[lane]);
                            continue;
                        }
                    }

                    local_error = merlin8_phy1_display_eye_scan_stripe(sa__, i[lane], &stripe[lane*EYE_SCAN_DISPLAY_SRIPE_SIZE+buffer_idx], usr_diag_status[lane]);
                    if (local_error) {
                        lane_mask &= (uint8_t)~(1<<lane);
                        _merlin8_phy1_display_eye_scan_ignored_lane(sa__, usr_diag_status[lane]);
                        continue;
                    }
                    i[lane]=(int16_t)(i[lane]-i_step[lane]);
                } else {
                    _merlin8_phy1_display_eye_scan_ignored_lane(sa__, usr_diag_status[lane]);
                }
            } else {
                _merlin8_phy1_display_eye_scan_ignored_lane(sa__, usr_diag_status[lane]);
            }
        }
        USR_PRINTF(("\n"));
    }

    CFUN(merlin8_phy1_display_eye_scan_footer(sa__, num_lanes, &usr_diag_status[0]));
    for(lane=0;lane < num_lanes;lane++) {
        if(lane_mask & (1<<lane)) {
            CFUN(merlin8_phy1_acc_set_lane(sa__, lane));
            CFUN(merlin8_phy1_meas_eye_scan_done(sa__));
        }
    }
Exit:

    if (merlin8_phy1_acc_set_lane(sa__, lane_orig)) {
        USR_PRINTF(("%s WARN: Unable to switch back to original lane \n", API_FUNCTION_NAME));
        if (Exit_Status == ERR_CODE_NONE) {
            Exit_Status = ERR_CODE_BAD_LANE;
        }
    }
    return(merlin8_phy1_error_report(sa__, Exit_Status));
}


err_code_t merlin8_phy1_meas_eye_scan_start(srds_access_t *sa__, uint8_t direction) {
    INIT_SRDS_ERR_CODE
    uint8_t lock;
    EFUN(merlin8_phy1_pmd_lock_status(sa__,&lock));
    {
        uint8_t sts;

        ESTM(sts = rdv_usr_sts_micro_stopped());
        if((lock == 0) && (sts != DB_STOPPED)) {
            EFUN_PRINTF(("Error: No PMD_RX_LOCK on lane requesting 2D eye scan\n"));
            return(ERR_CODE_DIAG_SCAN_NO_PMD_LOCK);
        }
    }
    if(direction == EYE_SCAN_VERTICAL) {
        EFUN(merlin8_phy1_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_START_VSCAN_EYE,GRACEFUL_STOP_TIME));
    } else if(direction == EYE_SCAN_HORIZ) {
        EFUN(merlin8_phy1_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_START_HSCAN_EYE,GRACEFUL_STOP_TIME));
    } else if(direction == EYE_SCAN_SLICE) {
        EFUN(merlin8_phy1_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_START_EYE_SLICE,GRACEFUL_STOP_TIME));
    } else if(direction == EYE_SCAN_INTR) {
        EFUN(merlin8_phy1_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_START_INTR_EYE,GRACEFUL_STOP_TIME));
    }
    return(ERR_CODE_NONE);
}

/*! Used to pass args for callback functions
 *
 */
typedef struct {
    uint32_t *buffer_ptr;
    } merlin8_phy1_read_eye_scan_stripe_callback_arg_t;

static err_code_t merlin8_phy1_read_eye_scan_stripe_callback(srds_access_t *sa__, void *arg, uint8_t byte_count, uint16_t data) {
    merlin8_phy1_read_eye_scan_stripe_callback_arg_t * const cast_arg = (merlin8_phy1_read_eye_scan_stripe_callback_arg_t *)arg;
    UNUSED(sa__);
    *(cast_arg->buffer_ptr++) = merlin8_phy1_INTERNAL_float8_to_uint32((float8_t)(data & 0xFF));
    if (byte_count > 1) {
        *(cast_arg->buffer_ptr++) = merlin8_phy1_INTERNAL_float8_to_uint32((float8_t)(data >> 8));
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_read_eye_scan_stripe(srds_access_t *sa__, uint32_t *buffer, uint16_t *status) {
    INIT_SRDS_ERR_CODE
    srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
    merlin8_phy1_read_eye_scan_stripe_callback_arg_t arg;
    uint32_t timeout_ms = DIAG_POLL_TIMEOUT;
    uint32_t lane_diag_base;
    const uint8_t lane = merlin8_phy1_acc_get_lane(sa__);
    uint8_t diag_rd_ptr;
    const uint8_t stripe_size = 64;

    if(!buffer || !status) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    SRDS_INFO_PTR_NULL_CHECK;
    arg.buffer_ptr = buffer;

        if(!(merlin8_phy1_info_ptr->lane_count) || (!(merlin8_phy1_info_ptr->diag_mem_ram_size))) {
            return ERR_CODE_INFO_TABLE_ERROR; /* To avoid a modulo by 0 exception */
        }
        lane_diag_base = merlin8_phy1_info_ptr->diag_mem_ram_base + ((lane%merlin8_phy1_info_ptr->lane_count) * merlin8_phy1_info_ptr->diag_mem_ram_size) +
            (uint32_t)(merlin8_phy1_info_ptr->grp_ram_size*merlin8_phy1_INTERNAL_grp_idx_from_lane(merlin8_phy1_acc_get_physical_lane(sa__)));
    EFUN(merlin8_phy1_INTERNAL_poll_diag_data(sa__, merlin8_phy1_info_ptr, status, &diag_rd_ptr, stripe_size, timeout_ms));
    EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                              lane_diag_base,
                                              (uint16_t)merlin8_phy1_info_ptr->diag_mem_ram_size,
                                              diag_rd_ptr,
                                              stripe_size,
                                              &arg,
                                              merlin8_phy1_read_eye_scan_stripe_callback));

    diag_rd_ptr = (uint8_t)(((uint32_t)diag_rd_ptr + stripe_size) % merlin8_phy1_info_ptr->diag_mem_ram_size);
    EFUN(wrv_usr_diag_rd_ptr(diag_rd_ptr));
    ESTM(*status = rdv_usr_diag_status() & 0xFF);
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_eye_scan_stripe(srds_access_t *sa__, int16_t y,uint32_t *buffer, uint16_t usr_diag_status) {
    INIT_SRDS_ERR_CODE

    const uint32_t limits[7] = {917504, 91750, 9175, 917, 91, 9, 1};

    int8_t x;
    uint8_t i;
    uint8_t data_thresh;
    afe_override_slicer_sel_t slicer = INVALID_SLICER;
    int16_t slicer_range_mV;
    int16_t slicer_range;
    int32_t level_mV;
    uint8_t eye_slice;
    uint8_t eye_scan_intr;
    uint8_t open_eye_logber = 7;

    ESTM(data_thresh = rd_p1_thresh_sel());
    slicer_range = (data_thresh ? 29 : 35);

    eye_slice = (usr_diag_status & DIAG_EYE_SCAN_RANGE_MASK) == DIAG_EYE_SCAN_SLICE;
    eye_scan_intr = (usr_diag_status & DIAG_EYE_SCAN_RANGE_MASK) == DIAG_EYE_SCAN_INTR;

    if(eye_scan_intr) {
        open_eye_logber = 5;
    }

    EFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, slicer_range, data_thresh, slicer, &slicer_range_mV));
    level_mV = ( (int16_t)y*slicer_range_mV+slicer_range/2)/slicer_range;

    if(!buffer) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    if (EYE_FORMAT_RAW == eyeReportFormat) {
        merlin8_phy1_report_raw_eye_scan_stripe(sa__, buffer, 64);
        return(ERR_CODE_NONE);
    }

    USR_PRINTF(("%6dmV : ", level_mV));
    for (x=-31;x<32;x++) {
        if((eye_slice) && (x != 0)) {
            continue;
        }

        for (i=0;i<open_eye_logber;i++) {
            uint8_t x_val;
            if(eye_slice) {
                x_val = (uint8_t)x;
            } else {
                x_val = (uint8_t)(x+31);
            }
            if (buffer[x_val]>=limits[i]) {
                USR_PRINTF(("%c", '0'+i+1));
                break;
            }
        }
        if (i==open_eye_logber) {
                if      ((x%5)==0 && (y%5)==0) {USR_PRINTF(("+"));}
                else if ((x%5)!=0 && (y%5)==0) {USR_PRINTF(("-"));}
                else if ((x%5)==0 && (y%5)!=0) {USR_PRINTF((":"));}
                else                           {USR_PRINTF((" "));}
        }
    }
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_eye_scan_header(srds_access_t *sa__, uint8_t i, uint16_t *usr_diag_status) {
    INIT_SRDS_ERR_CODE
    uint8_t x;
    uint8_t stepsize_var[NUM_LANES_MAX];
    uint8_t lane, num_lanes;

    ESTM(num_lanes = rdc_revid_multiplicity());
    if((i != num_lanes) && i != 1) {
        EFUN_PRINTF(("Error: Valid value of number of headers can't be greater than num_lanes = %d\n", num_lanes));
        return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
    }

    /* Initialize stepsize to 64 */
    for(lane=0; lane<i; lane++) {
        stepsize_var[lane] = 64;
    }

    UNUSED(*usr_diag_status);

    EFUN_PRINTF(("EYE DATA FORMAT: %s\n",  (EYE_FORMAT_RAW == eyeReportFormat) ? "RAW" : "PLOT"));
    EFUN_PRINTF(("\n"));
    EFUN_PRINTF((" Each character N represents approximate error rate 1e-N at that location\n"));
    for(x=1;x<=i;x++) {
        {
            EFUN_PRINTF(("  UI/%3d : -30  -25  -20  -15  -10  -5    0    5    10   15   20   25   30", stepsize_var[x-1]));
        }
    }
    EFUN_PRINTF(("\n"));
    for(x=1;x<=i;x++) {
        _merlin8_phy1_display_eye_scan_ignored_lane(sa__, usr_diag_status[x - 1]);
    }
    EFUN_PRINTF(("\n"));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_eye_scan_footer(srds_access_t *sa__, uint8_t i, uint16_t *usr_diag_status) {
    INIT_SRDS_ERR_CODE
    uint8_t x;
    uint8_t stepsize_var[NUM_LANES_MAX];
    uint8_t lane, num_lanes;

    ESTM(num_lanes = rdc_revid_multiplicity());
    if (((uint8_t)i != num_lanes) && ((uint8_t)i != 1)) {
        EFUN_PRINTF(("Error: Valid value of number of header can't be greater than num_lanes = %d\n", num_lanes));
        return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
    }

    /* Initialize stepsize to 64 */
    for(lane=0; lane<i; lane++) {
        stepsize_var[lane] = 64;
    }

    UNUSED(*usr_diag_status);

    EFUN_PRINTF(("\n"));
    for(x=1;x<=i;x++) {
        _merlin8_phy1_display_eye_scan_ignored_lane(sa__, usr_diag_status[x-1]);
    }
    EFUN_PRINTF(("\n"));

    for(x=1;x<=i;x++) {
        {
            EFUN_PRINTF(("  UI/%3d : -30  -25  -20  -15  -10  -5    0    5    10   15   20   25   30", stepsize_var[x-1]));
        }
    }
    EFUN_PRINTF(("\n"));
    return(ERR_CODE_NONE);
}


err_code_t merlin8_phy1_read_eye_scan_status(srds_access_t *sa__, uint16_t *status) {
    INIT_SRDS_ERR_CODE

   if(!status) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

   ESTM(*status=rdv_usr_diag_status());

    return(ERR_CODE_NONE);
}


err_code_t merlin8_phy1_meas_eye_scan_done(srds_access_t *sa__) {
  INIT_SRDS_ERR_CODE
  EFUN(merlin8_phy1_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_DISABLE,GRACEFUL_STOP_TIME));
  return(ERR_CODE_NONE);
}


err_code_t merlin8_phy1_start_ber_scan_test(srds_access_t *sa__, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    INIT_SRDS_ERR_CODE
    uint8_t lock,sts;
    EFUN(merlin8_phy1_pmd_lock_status(sa__,&lock));
    if(lock == 0) {
        EFUN_PRINTF(("Error: No PMD_RX_LOCK on lane requesting BER scan\n"));
        return(ERR_CODE_DIAG_SCAN_NOT_COMPLETE);
    }
    ESTM(sts =rdv_usr_sts_micro_stopped());

    if((sts > 1) && (sts != DB_STOPPED)) {
        EFUN_PRINTF(("Error: Lane is busy (%d) requesting BER scan\n",sts));
        return (ERR_CODE_UC_NOT_STOPPED);
    }
    EFUN(wrcv_diag_max_time_control(timer_control));
    EFUN(wrcv_diag_max_err_control(max_error_control));
    EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_CAPTURE_BER_START, ber_scan_mode,GRACEFUL_STOP_TIME));
    return(ERR_CODE_NONE);
}


err_code_t merlin8_phy1_read_ber_scan_data(srds_access_t *sa__, uint32_t *errors, uint32_t *timer_values, uint8_t *cnt, uint32_t timeout) {
    INIT_SRDS_ERR_CODE
    uint8_t i,prbs_byte,prbs_multi,time_byte,time_multi;
    uint16_t sts,dataword;


    if(!errors || !timer_values || !cnt) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
    /* init data arrays */
    for(i=0;i< DIAG_MAX_SAMPLES;i++) {
        errors[i]=0;
        timer_values[i]=0;
    }
    /* Check for completion read ln.diag_status byte?*/
    ESTM(sts = rdv_usr_diag_status());
    if((sts & 0x8000) == 0) {
        return(merlin8_phy1_error(sa__, ERR_CODE_DATA_NOTAVAIL));
    }
    *cnt = (uint8_t)((sts & 0x00FF)/3);
    for(i=0;i < *cnt;i++) {
        /* Read 2 bytes of data */
        EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_READ_DIAG_DATA_WORD, 0, timeout));
        ESTM(dataword = rd_uc_dsc_data());           /* LSB contains 2 -4bit nibbles */
        time_byte = (uint8_t)(dataword>>8);          /* MSB is time byte */
        prbs_multi = (uint8_t)dataword & 0x0F;       /* split nibbles */
        time_multi = (uint8_t)dataword>>4;
        /* Read 1 bytes of data */
        EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_READ_DIAG_DATA_BYTE, 0, timeout));
        ESTM(prbs_byte = (uint8_t)rd_uc_dsc_data());
        errors[i] = merlin8_phy1_INTERNAL_float12_to_uint32(prbs_byte,prbs_multi); /* convert 12bits to uint32 */
        timer_values[i] = (merlin8_phy1_INTERNAL_float12_to_uint32(time_byte,time_multi)<<3);
    /*  EFUN_PRINTF(("Err=%d (%02x<<%d); Time=%d (%02x<<%d)\n",errors[i],prbs_byte,prbs_multi,timer_values[i],time_byte,time_multi<<3)); */
        /*if(timer_values[i] == 0 && errors[i] == 0) break;*/
    }

  return(ERR_CODE_NONE);
}

/* This is good example function to do BER extrapolation */
err_code_t merlin8_phy1_eye_margin_proj_with_data(srds_access_t *sa__, USR_DOUBLE rate, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    INIT_SRDS_ERR_CODE
    int16_t offset_start = 0;
    err_code_t proj_start_err = ERR_CODE_NONE;

    /* start UC acquisition */
    proj_start_err = merlin8_phy1_INTERNAL_eye_margin_proj_start(sa__, ber_scan_mode, timer_control, max_error_control, &offset_start);
    if(proj_start_err == ERR_CODE_INVALID_RX_PAM_MODE) {
        return ERR_CODE_NONE;
    }
    else if(proj_start_err != ERR_CODE_NONE) {
        return merlin8_phy1_error_report(sa__, proj_start_err);
    }

    EFUN_PRINTF(("Waiting for measurement time approx %d seconds",timer_control+(timer_control>>1)));
    EFUN(merlin8_phy1_INTERNAL_eye_margin_proj_end(sa__, rate, ber_scan_mode, timer_control, offset_start));

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_all_eye_margin_proj(srds_access_t *sa__, USR_DOUBLE rate, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    INIT_SRDS_ERR_CODE
    err_code_t Exit_Status = ERR_CODE_NONE;
    err_code_t proj_start_err = ERR_CODE_NONE;
    int16_t offset_start[NUM_LANES_MAX] = {0};
    uint8_t skip_lane[NUM_LANES_MAX] = {0};
    uint8_t lane = 0, num_lanes = 0;
    uint8_t lane_orig = merlin8_phy1_acc_get_lane(sa__);


    /* Below 'DIAG_VERBOSE' level is intended to be modified only within a debug */
    /* session immediately after a breakpoint, and to retain its state only */
    /* through function exit:  therefore it must be 'volatile' to prevent a */
    /* compiler from eliding code conditioned on it, but NOT 'static'.      */

    CSTM(num_lanes = rdc_revid_multiplicity());

    for(lane = 0; lane < num_lanes; lane++) {
        CFUN(merlin8_phy1_acc_set_lane(sa__, lane));
        /* start UC acquisition */
        proj_start_err = merlin8_phy1_INTERNAL_eye_margin_proj_start(sa__, ber_scan_mode, timer_control, max_error_control, &offset_start[lane]);
        if(proj_start_err == ERR_CODE_INVALID_RX_PAM_MODE) {
            skip_lane[lane] = 1;
        }
        else if(proj_start_err != ERR_CODE_NONE) {
            CFUN(merlin8_phy1_error_report(sa__, proj_start_err));
        }
    }
    EFUN_PRINTF(("Waiting for measurement time approx %d seconds",timer_control+(timer_control>>1)));
    for(lane = 0; lane < num_lanes; lane++) {
        if(skip_lane[lane]) {
            continue;
        }
        CFUN(merlin8_phy1_acc_set_lane(sa__, lane));

        EFUN_PRINTF(("\n********** CORE %d LANE %d: NRZ EYE MARGIN PROJECTION **********\n", merlin8_phy1_acc_get_core(sa__), lane));
        CFUN(merlin8_phy1_INTERNAL_eye_margin_proj_end(sa__, rate, ber_scan_mode, timer_control, offset_start[lane]));
    }

    CFUN(merlin8_phy1_acc_set_lane(sa__, lane_orig));

Exit:
    return(Exit_Status);
}

err_code_t merlin8_phy1_eye_margin_proj(srds_access_t *sa__, USR_DOUBLE rate, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_eye_margin_proj_with_data(sa__, rate, ber_scan_mode, timer_control, max_error_control));
    return(ERR_CODE_NONE);
}



err_code_t merlin8_phy1_display_ber_scan_data (srds_access_t *sa__, USR_DOUBLE rate, uint8_t ber_scan_mode, uint32_t *total_errs, uint32_t *measurement_duration, uint8_t max_offset) {

#ifdef SERDES_API_FLOATING_POINT
    /* 'margins_mv[]' vector maps the p1 threshold code with actual mV
        Only relevant when mode=0
        This is not totally linear: for code 0~25 step=6mV; code 25~30 step=18mV; code 30~31 step=12
        'margins_mv[]' is valid only for Merlin. This vector would need to be modified accordingly for different Serdes
    USR_DOUBLE margins_mv[] = {0,6,12,18,24,30,36,42,48,54,60,66,72,78,84,
                           90,96,102,108,114,120,126,132,138,144,150,168,186,204,222,240,252};
    const USR_DOUBLE narrow_margins_mv[] = {0,3.6,7.2,10.8,14.4,18,21.6,25.5,28.8,32.4,36,39.6,43.2,46.8,50.4,
                                        54,57.6,61.2,64.8,68.4,72,75.6,79.2,82.8,86.4,90,100.8,111.6,122.4,133.2,144,151.2}; */
#if  !defined(STANDALONE_EVENT)


    const USR_DOUBLE intrusive_margins_mv[] = {2,6,10,14,18,22,26,30,32,36,40,44,48,52,56,60};
#endif
    INIT_SRDS_ERR_CODE
    const uint32_t HI_CONFIDENCE_ERR_CNT = 100;      /* bit errors */
    const uint32_t HI_CONFIDENCE_MIN_ERR_CNT = 20;   /* bit errors */
    const uint32_t MAX_CLIPPED_ERR_CNT = 8355840;
    const USR_DOUBLE ARTIFICIAL_BER = 0.5;  /* used along ARTIFICIAL_MARGIN(_V/_H) when not enough points to extrapolate */
    const int32_t ARTIFICIAL_MARGIN_V = 500;    /* Used along ARTIFICIAL_BER when not enough points to extrapolate. Unit: mV. Based on the concept of max Vpp of 1 Volt */
    const int32_t ARTIFICIAL_MARGIN_H = 1;  /* Used along ARTIFICIAL_BER when not enough points to extrapolate. Unit: UI. Based on the concept of two consecutive scrambled bits (1 UI appart) being uncorrelated */
    const int32_t MIN_BER_TO_REPORT = -24;   /* we clip the projected BER using this number */
    const USR_DOUBLE MIN_BER_FOR_FIT = -8.0;    /*  all points with BER <= 10^MIN_BER_FOR_FIT will be used for line fit (i.e used for extrapolation) */

    /* BER confidence scale */
    const USR_DOUBLE ber_conf_scale[104] = {
        2.9957,5.5717,3.6123,2.9224,2.5604,2.3337,2.1765,2.0604,1.9704,1.8983,
        1.8391,1.7893,1.7468,1.7100,1.6778,1.6494,1.6239,1.6011,1.5804,1.5616,
        1.5444,1.5286,1.5140,1.5005,1.4879,1.4762,1.4652,1.4550,1.4453,1.4362,
        1.4276,1.4194,1.4117,1.4044,1.3974,1.3908,1.3844,1.3784,1.3726,1.3670,
        1.3617,1.3566,1.3517,1.3470,1.3425,1.3381,1.3339,1.3298,1.3259,1.3221,
        1.3184,1.3148,1.3114,1.3080,1.3048,1.3016,1.2986,1.2956,1.2927,1.2899,
        1.2872,1.2845,1.2820,1.2794,1.2770,1.2746,1.2722,1.2700,1.2677,1.2656,
        1.2634,1.2614,1.2593,1.2573,1.2554,1.2535,1.2516,1.2498,1.2481,1.2463,
        1.2446,1.2429,1.2413,1.2397,1.2381,1.2365,1.2350,1.2335,1.2320,1.2306,
        1.2292,1.2278,1.2264,1.2251,1.2238,1.2225,1.2212,1.2199,1.2187,1.2175,
        1.2163, /* starts in index: 100 for #errors: 100,200,300,400 */
        1.1486, /* 200 */
        1.1198, /* 300 */
        1.1030};    /*400 */


    /* Define variables */
    USR_DOUBLE lbers[DIAG_MAX_SAMPLES] = {0};       /* Internal linear scale sqrt(-log(ber)) */
    USR_DOUBLE margins[DIAG_MAX_SAMPLES] = {0};     /* Eye margin @ each measurement */
    USR_DOUBLE bers[DIAG_MAX_SAMPLES] = {0};        /* computed bit error rate */
    uint32_t i;
    int16_t offset[DIAG_MAX_SAMPLES];
    int8_t mono_flags[DIAG_MAX_SAMPLES];

    int8_t direction;
    uint8_t heye;
    int8_t delta_n=1;
    USR_DOUBLE Exy = 0.0;
    USR_DOUBLE Eyy = 0.0;
    USR_DOUBLE Exx = 0.0;
    USR_DOUBLE Ey  = 0.0;
    USR_DOUBLE Ex  = 0.0;
    USR_DOUBLE alpha = 0.0;
    USR_DOUBLE gauss_noise = -1;
    USR_DOUBLE beta = 0.0;
    USR_DOUBLE sq_r = 0.0, alpha2 = 0.0;
    USR_DOUBLE proj_ber = 0.0, proj_ber_aux = 0.0;
    USR_DOUBLE proj_margin_12 = 0.0;
    USR_DOUBLE proj_margin_15 = 0.0;
    USR_DOUBLE proj_margin_18 = 0.0;
    USR_DOUBLE sq_err1 = 0.0, sq_err2 = 0.0;
    USR_DOUBLE ierr;
    uint8_t start_n;
    uint8_t stop_n;
    uint8_t low_confidence=1;
    uint8_t loop_index;
    uint8_t n_mono = 0;
    uint8_t eye_cnt = 0;
    uint8_t hi_confidence_cnt = 0;
    int8_t first_good_ber_idx = -1;
    int8_t first_small_errcnt_idx = -1;
    int8_t first_non_clipped_errcnt_idx = -1;
    uint8_t range250;
    uint8_t intrusive;
    uint8_t ber_clipped = 0;
    uint8_t last_point_discard;
    uint8_t fit_count;
    int32_t artificial_margin;
    int32_t proj_case = 0;  /* this variable will be used to signal what particular extrapolation case has happened at the end (after discarding invalid points of all sorts). To avoid potential issues: NEVER RE-USE VALUES... new cases should receive brand-new integer value */
    USR_DOUBLE artificial_lber;
    char message[256] = "NO MESSAGE";
    char unit[5];
    err_code_t Exit_Status = ERR_CODE_NONE;


    if(!total_errs || !measurement_duration ) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }


    /* Initialize BER array */
    for (i = 0; i < DIAG_MAX_SAMPLES; i++) {
        bers[i] = 0;
        mono_flags[i] = 0;
    }

    /* Decode mode/direction/etc. */
    heye = (uint8_t)((ber_scan_mode & DIAG_BER_HORZ)>>1);
    direction = (ber_scan_mode & DIAG_BER_NEG) ? -1 : 1 ;
    range250 = (ber_scan_mode & DIAG_BER_P1_NARROW) ? 0 : 1;
    intrusive = (ber_scan_mode & DIAG_BER_INTR) ? 1 : 0;

    /* Prepare artificial points in case they are needed */
    if (heye == 1) {
        artificial_margin = direction*ARTIFICIAL_MARGIN_H;
    } else {
        artificial_margin = direction*ARTIFICIAL_MARGIN_V;
    }
    artificial_lber = (USR_DOUBLE)sqrt(-log10(ARTIFICIAL_BER));

    /* Printing on-screen message */
    if (heye == 1) {
        if (direction==-1) USR_PRINTF(("\n\n********** HORIZONTAL PROJECTION: LEFT SIDE ******************\n"));
        if (direction== 1) USR_PRINTF(("\n\n********** HORIZONTAL PROJECTION: RIGHT SIDE *****************\n"));
    } else {
        if (direction==-1) USR_PRINTF(("\n\n********** VERTICAL PROJECTION: BOTTOM ***********************\n"));
        if (direction== 1) USR_PRINTF(("\n\n********** VERTICAL PROJECTION: TOP **************************\n"));
    }

    /* *******************************************
        * Generate margins[]
        * Generate ber[]
        * Find first and last points for linear fit
        ******************************************* */
    i=0;
    do {
        if(heye == 1) {
            USR_SNPRINTF(unit, sizeof(unit), "%s", "mUI");
            offset[i] = (int16_t)(max_offset-i);
#ifndef STANDALONE_EVENT
            margins[i] = direction*offset[i]*1000.0/64.0;
#else
            margins[i] = info_out->margins[i];
#endif
        } else {
            int16_t nlmv_val = 0;
            afe_override_slicer_sel_t slicer = INVALID_SLICER;
            USR_SNPRINTF(unit, sizeof(unit), "%s", "mV");
            offset[i] = (int16_t)(max_offset-i);
#ifndef STANDALONE_EVENT
            if(intrusive) {
                margins[i] = direction*intrusive_margins_mv[(uint8_t)offset[i]];
            } else {
                CFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, (int16_t)offset[i], range250, slicer, &nlmv_val));
                margins[i] = direction * nlmv_val;
            }
#else
            margins[i] = info_out->margins[i];
#endif
        }
        if (total_errs[i] == 0) {
            bers[i] = 1.0/(((USR_DOUBLE)measurement_duration[i])*0.00001*rate);
        } else {
            bers[i] = (USR_DOUBLE)total_errs[i]/(((USR_DOUBLE)measurement_duration[i])*0.00001*rate);
        }

        /* Find the first data point with good BER (BER <= 10^MIN_BER_FOR_FIT)
        NOTE: no need for lower bound on BER, since correction factors will be applied for all total_errs>=0 */
        if ((log10(bers[i]) <= MIN_BER_FOR_FIT) && (first_good_ber_idx == -1)) {
            first_good_ber_idx = (int8_t)i;
        }

        /* Determine high-confidence iterations */
        if (total_errs[i] >= HI_CONFIDENCE_ERR_CNT) {
            hi_confidence_cnt++;
        } else if ((total_errs[i] < HI_CONFIDENCE_MIN_ERR_CNT) && (first_small_errcnt_idx == -1)) {
            /* find the first data point with small error count */
            first_small_errcnt_idx = (int8_t)i;
        }
        /* Determine first NON-clipped error count
            NOTE: Originally this limit was created for post processing of micro-generated data; however, this could be used for PC-generated data as well */
        if ((total_errs[i] < MAX_CLIPPED_ERR_CNT) && (first_non_clipped_errcnt_idx == -1) ) {
            first_non_clipped_errcnt_idx = (int8_t)i;
        }

      i++;

    } while(((total_errs[i] != 0) || (measurement_duration[i] != 0)) && (i<=max_offset));

    eye_cnt = (uint8_t) i;


    /* *******************************************
    Setting up stop_n variable.
    Check if:
        - There is only one point in measurement vector (i.e. eye_cnt = 1)
        - The very last point's measurement time was "too short"
    ******************************************* */

    i = (uint32_t)(eye_cnt - 1);
    if (i>=1) {
        if ((measurement_duration[i] >= 0.5*measurement_duration[i-1]) || (total_errs[i] >= HI_CONFIDENCE_MIN_ERR_CNT) ){
            stop_n = eye_cnt;   /* last point will be included in linear fit */
            last_point_discard = 0;
        } else {
            stop_n = (uint8_t)(eye_cnt - 1);   /* discards the very last point */
            last_point_discard = 1;
        }
    } else {
        stop_n = 1; /* there is ONLY one measurement */
        last_point_discard = 0;
    }


    /* *******************************************
    Print on screen (prints RAW BER data. i.e. conf factors)
    ******************************************* */
    i = 0;
    do {
        if (total_errs[i] == 0) {
           USR_PRINTF(("BER @ %4.0f %s < 1e%-6.2f (%u errors in %0.2f sec)\n", margins[i], unit, log10(bers[i]), total_errs[i], ((USR_DOUBLE)measurement_duration[i])*0.00001));
        } else if (total_errs[i] >= MAX_CLIPPED_ERR_CNT) {
           USR_PRINTF(("BER @ %4.0f %s > 1e%-6.2f (%u errors in %0.2f sec)\n", margins[i], unit, log10(bers[i]), total_errs[i], ((USR_DOUBLE)measurement_duration[i])*0.00001));
        } else {
           USR_PRINTF(("BER @ %4.0f %s = 1e%-6.2f (%u errors in %0.2f sec)\n", margins[i], unit, log10(bers[i]), total_errs[i], ((USR_DOUBLE)measurement_duration[i])*0.00001));
        }
        i++;
    } while (i<stop_n);

    /* *******************************************
    Correcting *all* BER values using confidence factors in 'ber_conf_scale' vector
    This step is done for extrapolation purposes
    ******************************************* */
    for (loop_index=0; loop_index < eye_cnt; loop_index++) {
        if (total_errs[loop_index] <= 100) {
            bers[loop_index] = ber_conf_scale[total_errs[loop_index]] * bers[loop_index];
        } else if (total_errs[loop_index] > 100 && total_errs[loop_index] < 200) {
            bers[loop_index] = ber_conf_scale[100] * bers[loop_index];
        } else if (total_errs[loop_index] >= 200 && total_errs[loop_index] < 300) {
            bers[loop_index] = ber_conf_scale[101] * bers[loop_index];
        } else if (total_errs[loop_index] >= 300 && total_errs[loop_index] < 400) {
            bers[loop_index] = ber_conf_scale[102] * bers[loop_index];
        } else if (total_errs[loop_index] >= 400) {
            bers[loop_index] = ber_conf_scale[103] * bers[loop_index];
        }
    }

    /* *******************************************
    Computes the "linearised" ber vector
    ******************************************* */
    for (loop_index=0; loop_index<eye_cnt; loop_index++) {
        lbers[loop_index] = (USR_DOUBLE)sqrt(-log10(bers[loop_index]));
    }

    /* *******************************************
    Assign highest data point to use
    ******************************************* */
    if (first_good_ber_idx == -1) {
        start_n = stop_n;
    } else {
        start_n = (uint8_t)first_good_ber_idx;
    }



    /* ******************************************************
    ***********  EXTRAPOLATION (START) **********************
    *********************************************************
    Different data set profiles can be received by this code.
    Each case is processed accordingly here (IF-ELSE IF cases)
    ****************************************************** */

    /* ====> Errors encountered all the way to sampling point */
    if (start_n >= eye_cnt) {
        proj_case = 1;
        USR_SNPRINTF(message, sizeof(message), "%s", "No low-BER point measured");

        /* confidence factor of 0.96 is applied in this case to set a LOWER bound and report accordingly.
            This factor corresponds to approximately 3000 errors @95% confidence
            For reference: factors for 900, 2000, 3000, 5000, 20000 and 50000 errors are: 0.96, 0.96, 0.97, 0.99, 0.99, respectively */
        proj_ber = 0.96*log10(bers[eye_cnt-1]);
        proj_ber_aux = proj_ber;
        USR_PRINTF(("BER *worse* than 1e%0.2f\n", proj_ber));
        USR_PRINTF(("No margin @ 1e-12, 1e-15 & 1e-18\n\n\n"));
        fit_count = 1;
    }

    else {

        /* ====> Only ONE measured point. Typically when the eye is wide open.
            Artificial points will be used to make extrapolation possible */
        if (stop_n==1) {
            proj_case = 2;
            USR_SNPRINTF(message, sizeof(message), "%s", "Not enough points (single measured point). Using artificial point");

            low_confidence = 1;
            delta_n = 1;    /* 'delta_n' and 'fit_count' variables were kept for future use in case a new approach to handle low confidence case is adopted */
            fit_count = 2;

            /* Compute covariances and means... but only for two points: artificial and the single measured point */
            Exy = ((margins[0]*lbers[0] + artificial_margin*artificial_lber)/2.0);
            Eyy = ((lbers[0]*lbers[0] + artificial_lber*artificial_lber)/2.0);
            Exx = ((margins[0]*margins[0] + artificial_margin*artificial_margin)/2.0);
            Ey  = ((lbers[0] + artificial_lber)/2.0);
            Ex  = ((margins[0] + artificial_margin)/2.0);
        }

        /* ====> "NORMAL" case (when there are more than 1 measurements) */
        else {

            /* Detect and record nonmonotonic data points */
            for (loop_index=0; loop_index < stop_n; loop_index++) {
                if ((loop_index > start_n) && (log10(bers[loop_index]) > log10(bers[loop_index-1]))) {
                    mono_flags[loop_index] = 1;
                    if (first_good_ber_idx != -1) {
                        n_mono++;
                    }
                }
            }

            /* Finds number of MEASURED points available for extrapolation */
            delta_n = (int8_t)(stop_n-start_n-n_mono);


            /*  HIGH CONFIDENCE case */

            if (delta_n >= 2) { /* there are at least 2 points to trace a line */
                proj_case = 3;
                USR_SNPRINTF(message, sizeof(message), "%s", "Normal case");
                low_confidence = 0;

                /* Compute covariances and means */
                fit_count = 0;
                for (loop_index=start_n; loop_index < stop_n; loop_index++) {
                    if (mono_flags[loop_index] == 0) {
                        Exy += (margins[loop_index]*lbers[loop_index]/(USR_DOUBLE)delta_n);
                        Eyy += (lbers[loop_index]*lbers[loop_index]/(USR_DOUBLE)delta_n);
                        Exx += (margins[loop_index]*margins[loop_index]/(USR_DOUBLE)delta_n);
                        Ey  += (lbers[loop_index]/(USR_DOUBLE)delta_n);
                        Ex  += (margins[loop_index]/(USR_DOUBLE)delta_n);
                        fit_count++;
                    }
                }
            }

            /*  LOW CONFIDENCE case */

            else {  /*  NEW APPROACH (08/28/2014): consider very first point (error count < MAX_CLIPPED_ERR_CNT) and very last point for linear fit. This will give pessimistic/conservative extrapolation */
                low_confidence = 1;
                if ( (first_non_clipped_errcnt_idx>=0) && (first_non_clipped_errcnt_idx < start_n)) {
                    proj_case = 4;
                    USR_SNPRINTF(message, sizeof(message), "%s", "Not enough points. Using first measured point for conservative estimation");
                    fit_count = 2;
                    /* Compute covariances and means... but only for two points: first and last */
                    Exy = ((margins[stop_n-1]*lbers[stop_n-1] + margins[(uint8_t)first_non_clipped_errcnt_idx]*lbers[(uint8_t)first_non_clipped_errcnt_idx])/2.0);
                    Eyy = ((lbers[stop_n-1]*lbers[stop_n-1] + lbers[(uint8_t)first_non_clipped_errcnt_idx]*lbers[(uint8_t)first_non_clipped_errcnt_idx])/2.0);
                    Exx = ((margins[stop_n-1]*margins[stop_n-1] + margins[(uint8_t)first_non_clipped_errcnt_idx]*margins[(uint8_t)first_non_clipped_errcnt_idx])/2.0);
                    Ey  = ((lbers[stop_n-1] + lbers[(uint8_t)first_non_clipped_errcnt_idx])/2.0);
                    Ex  = ((margins[stop_n-1] + margins[(uint8_t)first_non_clipped_errcnt_idx])/2.0);
                } else {
                    proj_case = 5;
                    USR_SNPRINTF(message, sizeof(message), "%s", "Not enough points (cannot use non-clipped ErrorCount point). Using artificial point");
                    /* Compute covariances and means... but only for two points: artificial and the single measured point */
                    Exy = (artificial_margin*artificial_lber)/2.0;
                    Eyy = (artificial_lber*artificial_lber)/2.0;
                    Exx = (artificial_margin*artificial_margin)/2.0;
                    Ey  = (artificial_lber)/2.0;
                    Ex  = (artificial_margin)/2.0;
                    fit_count = 1;
                    /* This FOR loop checks for monotonicity as well */
                    for (loop_index=start_n; loop_index < stop_n; loop_index++) {
                        if (mono_flags[loop_index] == 0) {
                            Exy += (margins[loop_index]*lbers[loop_index]/2.0);
                            Eyy += (lbers[loop_index]*lbers[loop_index]/2.0);
                            Exx += (margins[loop_index]*margins[loop_index]/2.0);
                            Ey  += (lbers[loop_index]/2.0);
                            Ex  += (margins[loop_index]/2.0);
                            fit_count++;
                        }
                    }
                }
            }
        }

        /* Compute fit slope and offset: ber = alpha*margin + beta */
        alpha = (Exy - Ey*Ex) / (Exx - Ex*Ex);
        beta = Ey - Ex*alpha;
        /* Compute alpha2: slope of regression: margin = alpha2*ber + beta2 */
        alpha2 = (Exy - Ey*Ex) / (Eyy - Ey*Ey);
        /* Compute correlation index sq_r */
        sq_r = alpha*alpha2;

        proj_ber = pow(10,(-beta*beta));
        proj_margin_12 = direction*(sqrt(-log10(1e-12))-beta)/alpha;
        proj_margin_15 = direction*(sqrt(-log10(1e-15))-beta)/alpha;
        proj_margin_18 = direction*(sqrt(-log10(1e-18))-beta)/alpha;

        /* Estimate modeled gaussian noise.

            The following is based on the Q-function model and the following table:
                Q    |    log10(BER)
            =======================
              7.033    |    -12
              7.941    |    -15
              8.757    |    -18

        Based on the above, we solve for sigma:
            7.033*sigma = u - proj_margin_12 , and
            7.941*sigma = u - proj_margin_15
        */
        gauss_noise = (proj_margin_12 - proj_margin_15)/0.908;

        sq_err1 = (Eyy + (beta*beta) + (Exx*alpha*alpha) -
                   (2*Ey*beta) - (2*Exy*alpha) + (2*Ex*beta*alpha));
        (void)sq_err1;
        sq_err2 = 0;
        for (loop_index=start_n; loop_index<stop_n; loop_index++) {
            ierr = (lbers[loop_index] - (alpha*margins[loop_index] + beta));
            sq_err2 += (ierr*ierr/(USR_DOUBLE)delta_n);
        }

        proj_ber = log10(proj_ber);
        proj_ber_aux = proj_ber;

        if (proj_ber < MIN_BER_TO_REPORT) {
            proj_ber = MIN_BER_TO_REPORT;
            ber_clipped = 1;
        }

        /* Extrapolated results, low confidence */
        if (low_confidence == 1) {

            USR_PRINTF(("BER(extrapolated) < 1e%0.2f\n", proj_ber));
            USR_PRINTF(("Margin @ 1e-12    > %0.2f %s\n", (proj_ber < -12)? SRDS_ABS(proj_margin_12) : 0, unit));
            USR_PRINTF(("Margin @ 1e-15    > %0.2f %s\n", (proj_ber < -15)? SRDS_ABS(proj_margin_15) : 0, unit));
            USR_PRINTF(("Margin @ 1e-18    > %0.2f %s\n", (proj_ber < -18)? SRDS_ABS(proj_margin_18) : 0, unit));

        /* Extrapolated results, HIGH confidence */
        } else {

            if (ber_clipped == 1) {
                USR_PRINTF(("BER(extrapolated) < 1e%0.2f\n", proj_ber));
            } else {
                USR_PRINTF(("BER(extrapolated) = 1e%0.2f\n", proj_ber));
            }
            USR_PRINTF(("Margin @ 1e-12    = %0.2f %s\n", (proj_ber < -12)? SRDS_ABS(proj_margin_12) : 0, unit));
            USR_PRINTF(("Margin @ 1e-15    = %0.2f %s\n", (proj_ber < -15)? SRDS_ABS(proj_margin_15) : 0, unit));
            USR_PRINTF(("Margin @ 1e-18    = %0.2f %s\n", (proj_ber < -18)? SRDS_ABS(proj_margin_18) : 0, unit));
        }

        USR_PRINTF(("\n\n"));

        /* Print non-monotonic outliers */
        if (n_mono != 0) {
            USR_PRINTF(("Detected non-monotonicity at { "));
            for (loop_index = start_n; loop_index < stop_n; loop_index++) {
                if (mono_flags[loop_index] == 1) {
                    USR_PRINTF(("%0.2f ", margins[loop_index]));
                }
            }
            USR_PRINTF(("} %s\n\n\n",unit));
        }

    }
    /* *******************************************
    ***********  EXTRAPOLATION (END) *************
    ********************************************* */



    /* SUMMARY (for debugging purposes */
    if (DIAG_VERBOSE > 2) USR_PRINTF(("\t=====> DEBUG INFO (start)\n\n"));
    if (DIAG_VERBOSE > 2) {
        USR_PRINTF((" loop   Margin      total_errors   time(sec)  logBER       lber"));
        for (loop_index=0; loop_index < stop_n+last_point_discard; loop_index++) {
            USR_PRINTF(("\n%5d %11.0f %14d %10.3f %8.2f %10.3f", loop_index, margins[loop_index], total_errs[loop_index], ((USR_DOUBLE)measurement_duration[loop_index])*0.00001, log10(bers[loop_index]), lbers[loop_index]));
        }
        USR_PRINTF(("\n\n"));
    }
    if (DIAG_VERBOSE > 2) USR_PRINTF(("Max Offset = %d\n",max_offset));
    if (DIAG_VERBOSE > 2) USR_PRINTF(("ber_clipped: %d, Projected BER (proj_ber_aux) = %.2f\n", ber_clipped, proj_ber_aux));
    if (DIAG_VERBOSE > 2) USR_PRINTF(("first good ber idx at %d, ber = 1e%f\n", first_good_ber_idx, ((first_good_ber_idx>=0) ? log10(bers[(uint8_t)first_good_ber_idx]) : 0.0)));
    {
        char aux_str[20];
        if(first_small_errcnt_idx != -1 ) {
            USR_SNPRINTF(aux_str, sizeof(aux_str), "%d", total_errs[(uint8_t)first_small_errcnt_idx]);
        } else {
            USR_SNPRINTF(aux_str, sizeof(aux_str), "%s", "N/A");
        }
        if (DIAG_VERBOSE > 2) USR_PRINTF(("first small errcnt idx at %d, errors = %s\n", first_small_errcnt_idx, ((first_small_errcnt_idx>=0) ? aux_str : "-1")));
    }
    if (DIAG_VERBOSE > 2) USR_PRINTF(("last point discarded?: %d, low_confidence: %d, first_non_clipped_errcnt_idx: %d, start_n: %d, stop_n: %d, eye_cnt: %d, n_mono: %d, first_good_ber_idx = %d, first_small_errcnt_idx = %d, fit_count: %d, delta_n: %d\n",
                last_point_discard, low_confidence, first_non_clipped_errcnt_idx, start_n, stop_n, eye_cnt, n_mono, first_good_ber_idx, first_small_errcnt_idx, fit_count, delta_n));
    if (DIAG_VERBOSE > 2) USR_PRINTF(("Exy=%.2f, Eyy=%.4f, Exx=%.2f, Ey=%.4f, Ex=%.2f, alpha=%.4f, beta=%.4f, alpha2=%.3f, sq_r=%.3f, sq_err1=%g, sq_err2=%g, gauss_noise=%.3f\n", Exy,Eyy,Exx,Ey,Ex,alpha,beta,alpha2,sq_r,sq_err1,sq_err2,gauss_noise));
    if (DIAG_VERBOSE > 2) USR_PRINTF(("%s\n", message));
    if (DIAG_VERBOSE > 2) USR_PRINTF(("proj_case %d\n",proj_case));
    if (DIAG_VERBOSE > 2) USR_PRINTF(("\n\t=====> DEBUG INFO (end)\n\n"));
    USR_PRINTF(("\n\n\n"));


#else /* SERDES_API_FLOATING_POINT */
    uint8_t jj = 0;
    uint32_t one_u32 = 0x01000000;
    char * ptr = (char *)(&one_u32);
    const uint8_t big_endian = (ptr[0] == 1);

    UNUSED(rate);
    UNUSED(ber_scan_mode);
    UNUSED(max_offset);
    USR_PRINTF(("%s : This function needs SERDES_API_FLOATING_POINT define to operate \n", API_FUNCTION_NAME));

    if(!total_errs || !measurement_duration ) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    USR_PRINTF(("This function needs SERDES_API_FLOATING_POINT define to decode and project errors \n"));
    USR_PRINTF(("\nSERDES BER PROJ DATA DUMP START\n"));
    /*                dprintf ("setting: %d, vco_rate: %gGHz, osr_mode: %d, rate: %gGbps\n", core_config.field.vco_rate,
                        (double)VCO_RATE_TO_MHZ(core_config.field.vco_rate)/1000.0, osr_mode.rx, rate/1e9);
    */
    USR_PRINTF(("%x,%02x,%d,%x,%x\n", DIAG_MAX_SAMPLES, big_endian, rate, ber_scan_mode, max_offset));
    /* printf("%1f, 0x%x, 0x%x, 0x%02x, 0x%x, 0x%x \n", rate, ber_scan_mode, max_offset, big_endian, (DIAG_MAX_SAMPLES), 64); */

    /* Just to address unused variable warning in certain environments where EFUN_PRINTF is not available */
    UNUSED(big_endian);

    USR_PRINTF(("%x", total_errs[0]));
    for (jj=1;jj<DIAG_MAX_SAMPLES;jj++) {
        USR_PRINTF((",%x", total_errs[jj]));
    }
    USR_PRINTF(("\n"));
    USR_PRINTF(("%x,", measurement_duration[0]));
    for (jj=1;jj<DIAG_MAX_SAMPLES;jj++) {
        USR_PRINTF((",%x", measurement_duration[jj]));
    }
    USR_PRINTF(("\n"));

    USR_PRINTF(("SERDES BER PROJ DATA DUMP END\n"));

#endif /* SERDES_API_FLOATING_POINT */
#ifdef SERDES_API_FLOATING_POINT
Exit:
#endif
#ifdef SERDES_API_FLOATING_POINT
    return(merlin8_phy1_error_report(sa__, Exit_Status));
#else
    return(ERR_CODE_NONE);
#endif
}

/*****************************/
/*  Display Lane/Core State  */
/*****************************/

err_code_t merlin8_phy1_display_lane_state_hdr(srds_access_t *sa__) {
#ifdef PCIEG3_V1
  EFUN_PRINTF(("LN  P STP RATE "));
#else
#if defined(SERDES_TX_RX_SEPARATE)
  EFUN_PRINTF(("LN (P CDRxN  , UC_CFG, UC_STS, RST, STP) "));
#else
  EFUN_PRINTF(("LN (P CDRxN  , UC_CFG, UC_STS, RST, STP) "));
#endif
#endif /* (defined(SERDES_DUAL_PLLS) && !defined(SERDES_NO_PLL_SELECT)) */
#ifndef PCIEG3_V1
  EFUN_PRINTF(("SD LCK RXPPM "));
#else
  EFUN_PRINTF(("DSC0(0,1) SD(D,C) LCK RXPPM "));
#endif
  EFUN_PRINTF(("CLK90 CLKP1 "));
  EFUN_PRINTF(("PF(M,L) "));
  EFUN_PRINTF(("VGA "));
  EFUN_PRINTF(("DCO P1mV "));
  EFUN_PRINTF(("    DFE(1,2,3,4,5,dcd1,dcd2)   SLICER(ze,zo,pe,po,me,mo) "));
#ifndef PCIEG3_V1
  EFUN_PRINTF(("TXPPM   TXEQ(n1,m,p1,p2)    EYE(L,R,U,D)   "));
  EFUN_PRINTF(("  SOC(p,n) "));
#else
  EFUN_PRINTF((" TXPPM "));
#endif
#ifndef PCIEG3_V1
  EFUN_PRINTF(("LINK_TIME    BER"));
#endif
  EFUN_PRINTF(("\n"));
  return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_lane_state_legend(srds_access_t *sa__) {
  EFUN_PRINTF(("\n"));
  EFUN_PRINTF(("**********************************************************************************************\n"));
  EFUN_PRINTF(("****                Legend of Entries in display_lane_state()                             ****\n"));
  EFUN_PRINTF(("**********************************************************************************************\n"));
  EFUN_PRINTF(("LN        : Lane index within IP core\n"));
  EFUN_PRINTF(("P         : Polarity of Tx and Rx. '-' is inverted, '+' is non-inverted.\n"));
#ifndef PCIEG3_V1
  EFUN_PRINTF(("CDRxN     : CDR type x OSR ratio [xT-OSx3p3; x7-OSx7p5; x9-OSx8p25; xA-OSx10]\n"));
  EFUN_PRINTF(("UC_CFG    : Micro lane configuration variable, struct definition can be found in IP specific register html (search for RX_CKRST_CTRL_LANE_UC_CONFIG) \n"));
  EFUN_PRINTF(("UC_STS    : Micro lane status variable, Extended micro lane status variable\n"));
#if defined(SERDES_TX_RX_SEPARATE)
  EFUN_PRINTF(("RST       : TX and RX Reset State{reset_active, reset_occured, reset_held}\n"));
#else
  EFUN_PRINTF(("RST       : Reset State{reset_active, reset_occured, reset_held}\n"));
#endif
#endif /* MERLIN7_PCIEG3 */
  EFUN_PRINTF(("STP       : uC Stopped State\n"));
#ifdef PCIEG3_V1
  EFUN_PRINTF(("RATE      : Rate select\n"));
#endif
#ifndef PCIEG3_V1
  EFUN_PRINTF(("SD        : Signal Detect, a '*' after SD value indicates SD changed since last read\n"));
#else
  EFUN_PRINTF(("DSC0(0,1) : Double read of dsc_one_hot_0 \n"));
  EFUN_PRINTF(("SD(D,C)   : Signal detect and counter\n"));
#endif
  EFUN_PRINTF(("LCK       : pmd_rx_lock, a '*' after LCK value indicates pmd_rx_lock changed since last read\n"));
  EFUN_PRINTF(("RXPPM     : Frequency offset of local reference clock with respect to RX data in ppm\n"));
  EFUN_PRINTF(("CLK90     : Delay of zero crossing slicer, m1, wrt to data in PI codes\n"));


  EFUN_PRINTF(("CLKP1     : Delay of diagnostic/lms slicer, p1, wrt to data in PI codes\n"));
  EFUN_PRINTF(("PF(M,L)   : Peaking Filter Main (0..15) and Low Frequency (0..7) settings\n"));
#if defined(RX_VGA_CTRL_VAL_MIN)
  EFUN_PRINTF(("VGA       : Variable Gain Amplifier settings (%d..%d)\n", RX_VGA_CTRL_VAL_MIN, RX_VGA_CTRL_VAL_MAX));
#else
  EFUN_PRINTF(("VGA       : Variable Gain Amplifier settings (0..%d)\n", RX_VGA_CTRL_VAL_MAX));
#endif
  EFUN_PRINTF(("DCO       : DC offset DAC control value\n"));
  EFUN_PRINTF(("P1mV      : Vertical threshold voltage of p1 slicer\n"));
  EFUN_PRINTF(("DFE taps  : ISI correction taps in units of 2.35mV (for 1 & 2 even values are displayed, dcd = even-odd)\n"));
  EFUN_PRINTF(("SLICER(ze,zo,pe,po,me,mo) : Slicer calibration control codes\n"));
  EFUN_PRINTF(("TXPPM     : Frequency offset of local reference clock with respect to TX data in ppm\n"));
  EFUN_PRINTF(("            a '*' indicates TX PI is enabled\n"));
#ifndef PCIEG3_V1
  EFUN_PRINTF(("TXEQ(n1,m,p1,p2) : TX equalization FIR tap weights in units of 1Vpp/60 units\n"));
  EFUN_PRINTF(("                 : A 'T' preceding the tap weights denotes TX Training is enabled\n"));
  EFUN_PRINTF(("                 : A 'R' preceding the tap weights denotes Remote loopback is enabled\n"));
  EFUN_PRINTF(("                 : A 'L' preceding the tap weights denotes Digital loopback is enabled\n"));
  EFUN_PRINTF(("SOC(p,n)  : Signal Detect Offset Calibration\n"));
  EFUN_PRINTF(("EYE(L,R,U,D) : Eye margin @ 1e-5 (NRZ) or 1e-3 (PAM4), as seen by internal diagnostic slicer in mUI and mV\n"));
  EFUN_PRINTF(("LINK_TIME : Link time in milliseconds\n"));
  EFUN_PRINTF(("BER : Bit Error Rate calculated based on 100ms test time; displayed only if prbs_chk_en=1.\n"));
  EFUN_PRINTF(("    : '!Lock' indicates that the PRBS checker was unable to obtain lock.\n"));
  EFUN_PRINTF(("    : 'Lock lost' indicates that the PRBS checker lost lock after obtaining lock.\n"));
#endif /* MERLIN7_PCIEG3 */
  EFUN_PRINTF(("**********************************************************************************************\n"));
  return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_lane_state(srds_access_t *sa__) {
  err_code_t err_code = merlin8_phy1_INTERNAL_display_lane_state_no_newline(sa__);
  EFUN_PRINTF(("\n"));
  return (err_code);
}

err_code_t merlin8_phy1_display_core_state_hdr(srds_access_t *sa__) {
    char core_type[25] = "merlin8_phy1";
    EFUN_PRINTF(("SerDes type = %s\n",core_type));

#ifdef PCIEG3_V1
  EFUN_PRINTF(("CORE  ST   PLL_PWDN   COM_CLK   UCODE_VER  API_VER  AFE_VER   LIVE_TEMP  RESCAL   RATE ANA_VCO_RANGE PLL_LOCK"));
#else
  EFUN_PRINTF(("CORE RST_ST  PLL_PWDN  UC_ATV   COM_CLK   UCODE_VER  API_VER  AFE_VER   LIVE_TEMP   AVG_TMON   RESCAL   VCO_RATE  ANA_VCO_RANGE  PLL_DIV  PLL_LOCK"));
#endif
  EFUN_PRINTF(("\n"));
  return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_core_state_line(srds_access_t *sa__) {
  err_code_t err_code = merlin8_phy1_INTERNAL_display_core_state_no_newline(sa__);
  EFUN_PRINTF(("\n"));
  return (err_code);
}

err_code_t merlin8_phy1_display_core_state_legend(srds_access_t *sa__) {
  EFUN_PRINTF(("\n"));
  EFUN_PRINTF(("**************************************************************************************************************\n"));
  EFUN_PRINTF(("****                          Legend of Entries in display_core_state()                                   ****\n"));
  EFUN_PRINTF(("**************************************************************************************************************\n"));
#ifndef PCIEG3_V1
  EFUN_PRINTF(("*  RST_ST           : Core DP Reset State{reset_active, reset_occured, reset_held}, Core uC Status byte(hex) *\n"));
#else
  EFUN_PRINTF(("*  ST               : Core uC Status byte(hex)                                                               *\n"));
#endif
  EFUN_PRINTF(("*  PLL_PWDN         : PLL Powerdown Control Bit (active high)                                                *\n"));
#ifndef PCIEG3_V1
  EFUN_PRINTF(("*  UC_ATV           : UC Active bit                                                                          *\n"));
#endif
  EFUN_PRINTF(("*  COM_CLK          : COM Clock frequency in MHz                                                             *\n"));
  EFUN_PRINTF(("*  UCODE_VER        : Microcode Version [majorversion_minorversion]                                          *\n"));
  EFUN_PRINTF(("*  API_VER          : API Version [majorversion_minorversion]                                                *\n"));
  EFUN_PRINTF(("*  AFE_VER          : AFE Hardware Version                                                                   *\n"));
  EFUN_PRINTF(("*  LIVE_TEMP        : Live Die temperature in Celsius                                                        *\n"));
#ifndef PCIEG3_V1
  EFUN_PRINTF(("*  AVG_TMON         : uC Temp_idx, Average temperature in Celsius                                            *\n"));
#endif
  EFUN_PRINTF(("*  RESCAL           : Analog Resistor Calibration value                                                      *\n"));
#ifndef PCIEG3_V1
  EFUN_PRINTF(("*  VCO_RATE         : uC VCO Rate in GHz (approximate)                                                       *\n"));
#else
  EFUN_PRINTF(("*  RATE             : Rate select                                                                            *\n"));
#endif
  EFUN_PRINTF(("*  ANA_VCO_RANGE    : Analog VCO Range                                                                       *\n"));
  EFUN_PRINTF(("*  PLL_DIV          : (Register Value) Actual PLL Divider Value                                              *\n"));
  EFUN_PRINTF(("*  PLL_Lock         : PLL Lock                                                                               *\n"));

  EFUN_PRINTF(("**************************************************************************************************************\n"));
  return (ERR_CODE_NONE);
}

/**********************************************/
/*  Display Lane/Core Config and Debug Status */
/**********************************************/
#ifndef PCIEG3_V1
err_code_t merlin8_phy1_display_core_config(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    EFUN_PRINTF(("\n\n***********************************\n"  ));
    EFUN_PRINTF((    "**** SERDES CORE CONFIGURATION ****\n"  ));
    EFUN_PRINTF((    "***********************************\n\n"));
    {
        struct   merlin8_phy1_uc_core_config_st core_cfg = UC_CORE_CONFIG_INIT;
        {
            uint8_t pll_orig;
            ESTM(pll_orig = merlin8_phy1_acc_get_pll_idx(sa__));
            EFUN(merlin8_phy1_acc_set_pll_idx(sa__, 0));
            EFUN(merlin8_phy1_get_uc_core_config(sa__, &core_cfg));
            EFUN(merlin8_phy1_acc_set_pll_idx(sa__, pll_orig));
        }
        {
            uint16_t  vco_mhz = (uint16_t)core_cfg.vco_rate_in_Mhz;
    EFUN_PRINTF((    "uC Config VCO Rate        = %d (~%d.%03dGHz)\n"                  , core_cfg.field.vco_rate
                                                                                  , (vco_mhz / 1000)
                                                                                  , (vco_mhz % 1000)                ));
    EFUN_PRINTF((    "Core Config from PCS      = %d\n\n"                         , core_cfg.field.core_cfg_from_pcs));
        }
    }
    return(ERR_CODE_NONE);
}


err_code_t merlin8_phy1_display_lane_config(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    struct merlin8_phy1_uc_lane_config_st lane_cfg;

    ENULL_MEMSET(&lane_cfg, 0, sizeof(lane_cfg));

    EFUN_PRINTF(("\n\n*************************************\n"                                          ));
    ESTM_PRINTF((   "**** SERDES LANE %d CONFIGURATION ****\n"  , merlin8_phy1_acc_get_lane(sa__)                 ));
    EFUN_PRINTF((    "*************************************\n\n"                                        ));
    EFUN(                                                         merlin8_phy1_get_uc_lane_cfg(sa__, &lane_cfg));
    EFUN_PRINTF((    "Lane Config from PCS        = %d\n\n"     , lane_cfg.field.lane_cfg_from_pcs      ));
    EFUN_PRINTF((    "Auto-Neg Enabled            = %d\n"       , lane_cfg.field.an_enabled             ));
    EFUN_PRINTF((    "DFE on                      = %d\n"       , lane_cfg.field.dfe_on                 ));
    EFUN_PRINTF((    "Brdfe_on                    = %d\n"       , lane_cfg.field.force_brdfe_on         ));
    EFUN_PRINTF((    "Media Type                  = %d\n"       , lane_cfg.field.media_type             ));
    EFUN_PRINTF((    "Unreliable LOS              = %d\n"       , lane_cfg.field.unreliable_los         ));
    EFUN_PRINTF((    "Scrambling Disable          = %d\n"       , lane_cfg.field.scrambling_dis         ));
    ESTM_PRINTF((    "CL72 Training        Enable = %d\n"       , rd_cl72_ieee_training_enable()        ));
    EFUN_PRINTF((    "CL72 Auto Polarity   Enable = %d\n"       , lane_cfg.field.cl72_auto_polarity_en  ));
    EFUN_PRINTF((    "CL72 Restart timeout Enable = %d\n"       , lane_cfg.field.cl72_restart_timeout_en));


    ESTM_PRINTF((    "TX OSR Mode Force           = %d\n"       , rd_tx_osr_mode_frc()                  ));
    ESTM_PRINTF((    "TX OSR Mode Force Val       = %d\n"       , rd_tx_osr_mode_frc_val()              ));
    ESTM_PRINTF((    "RX OSR Mode Force           = %d\n"       , rd_rx_osr_mode_frc()                  ));
    ESTM_PRINTF((    "RX OSR Mode Force Val       = %d\n"       , rd_rx_osr_mode_frc_val()              ));
    ESTM_PRINTF((    "TX Polarity Invert          = %d\n"       , rd_tx_pmd_dp_invert()                 ));
    ESTM_PRINTF((    "RX Polarity Invert          = %d\n\n"     , rd_rx_pmd_dp_invert()                 ));
    ESTM_PRINTF((    "TXFIR Post2                 = %d\n"       , rd_txfir_post2()                      ));
    ESTM_PRINTF((    "TXFIR Main Override         = %d\n"       , rd_cl72_txfir_main()                  ));
    ESTM_PRINTF((    "TXFIR Pre Override          = %d\n"       , rd_cl72_txfir_pre()                   ));
    ESTM_PRINTF((    "TXFIR Post Override         = %d\n"       , rd_cl72_txfir_post()                  ));
    return(ERR_CODE_NONE);
}
#endif /* MERLIN7_PCIEG3 */

err_code_t merlin8_phy1_display_core_state(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_display_core_state_hdr(sa__));
    EFUN(merlin8_phy1_display_core_state_line(sa__));
    EFUN(merlin8_phy1_display_core_state_legend(sa__));
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_display_all_lane_state(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint8_t lane = 0, num_lanes = 0;
    uint8_t lane_orig = merlin8_phy1_acc_get_lane(sa__);

    ESTM(num_lanes = rdc_revid_multiplicity());
    EFUN(merlin8_phy1_display_lane_state_hdr(sa__));
    for(lane = 0; lane < num_lanes; lane++) {
        EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
        EFUN(merlin8_phy1_display_lane_state(sa__));
    }
    EFUN(merlin8_phy1_acc_set_lane(sa__, lane_orig));

    return ERR_CODE_NONE;
}

#ifndef PCIEG3_V1
err_code_t merlin8_phy1_display_lane_debug_status(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    /* startup */
    struct merlin8_phy1_usr_ctrl_disable_functions_st     ds;
    struct merlin8_phy1_usr_ctrl_disable_dfe_functions_st dsd;
    /* steady state */
    struct merlin8_phy1_usr_ctrl_disable_functions_st     dss;
    struct merlin8_phy1_usr_ctrl_disable_dfe_functions_st dssd;

    uint8_t core;
    uint8_t lane;

    ENULL_MEMSET(&ds  , 0, sizeof(ds  ));
    ENULL_MEMSET(&dsd , 0, sizeof(dsd ));
    ENULL_MEMSET(&dss , 0, sizeof(dss ));
    ENULL_MEMSET(&dssd, 0, sizeof(dssd));
    ESTM(core=merlin8_phy1_acc_get_core(sa__));
    ESTM(lane=merlin8_phy1_acc_get_lane(sa__));
    EFUN_PRINTF(("\n\n*******************************************\n"                                              ));
    ESTM_PRINTF((   "**** SERDES CORE %d LANE %d DEBUG STATUS ****\n", core, lane                                 ));
    EFUN_PRINTF((    "*******************************************\n\n"                                            ));
    ESTM_PRINTF((    "Restart Count                                = %d\n"  , rdv_usr_sts_restart_counter()       ));
    ESTM_PRINTF((    "Reset Count                                  = %d\n"  , rdv_usr_sts_reset_counter()         ));
    ESTM_PRINTF((    "PMD Lock Count                               = %d\n\n", rdv_usr_sts_pmd_lock_counter()      ));
    EFUN(merlin8_phy1_get_usr_ctrl_disable_startup(sa__, &ds));
    EFUN_PRINTF((    "Disable Startup PF Adaptation                = %d\n"  , ds.field.pf_adaptation              ));
    EFUN_PRINTF((    "Disable Startup DC Adaptation                = %d\n"  , ds.field.dc_adaptation              ));
    EFUN_PRINTF((    "Disable Startup Slicer Offset Tuning         = %d\n"  , ds.field.slicer_offset_tuning       ));
    EFUN_PRINTF((    "Disable Startup Clk90 offset Adaptation      = %d\n"  , ds.field.clk90_offset_adaptation    ));
    EFUN_PRINTF((    "Disable Startup P1 level Tuning              = %d\n"  , ds.field.p1_level_tuning            ));
    EFUN_PRINTF((    "Disable Startup Eye Adaptation               = %d\n"  , ds.field.eye_adaptation             ));
    EFUN_PRINTF((    "Disable Startup All Adaptation               = %d\n\n", ds.field.all_adaptation             ));
    EFUN(                                                            merlin8_phy1_get_usr_ctrl_disable_startup_dfe(sa__, &dsd));
    EFUN_PRINTF((    "Disable Startup DFE Tap1 Adaptation          = %d\n"  , dsd.field.dfe_tap1_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap2 Adaptation          = %d\n"  , dsd.field.dfe_tap2_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap3 Adaptation          = %d\n"  , dsd.field.dfe_tap3_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap4 Adaptation          = %d\n"  , dsd.field.dfe_tap4_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap5 Adaptation          = %d\n"  , dsd.field.dfe_tap5_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap1 DCD                 = %d\n"  , dsd.field.dfe_tap1_dcd              ));
    EFUN_PRINTF((    "Disable Startup DFE Tap2 DCD                 = %d\n\n", dsd.field.dfe_tap2_dcd              ));
    EFUN(                                                           merlin8_phy1_get_usr_ctrl_disable_steady_state(sa__, &dss));
    EFUN_PRINTF((    "Disable Steady State PF Adaptation           = %d\n"  , dss.field.pf_adaptation             ));
    EFUN_PRINTF((    "Disable Steady State DC Adaptation           = %d\n"  , dss.field.dc_adaptation             ));
    EFUN_PRINTF((    "Disable Steady State Slicer Offset Tuning    = %d\n"  , dss.field.slicer_offset_tuning      ));
    EFUN_PRINTF((    "Disable Steady State Clk90 offset Adaptation = %d\n"  , dss.field.clk90_offset_adaptation   ));
    EFUN_PRINTF((    "Disable Steady State P1 level Tuning         = %d\n"  , dss.field.p1_level_tuning           ));
    EFUN_PRINTF((    "Disable Steady State Eye Adaptation          = %d\n"  , dss.field.eye_adaptation            ));
    EFUN_PRINTF((    "Disable Steady State All Adaptation          = %d\n\n", dss.field.all_adaptation            ));
    EFUN(                                                      merlin8_phy1_get_usr_ctrl_disable_steady_state_dfe(sa__, &dssd));
    EFUN_PRINTF((    "Disable Steady State DFE Tap1 Adaptation     = %d\n"  , dssd.field.dfe_tap1_adaptation      ));
    EFUN_PRINTF((    "Disable Steady State DFE Tap2 Adaptation     = %d\n"  , dssd.field.dfe_tap2_adaptation      ));
    EFUN_PRINTF((    "Disable Steady State DFE Tap3 Adaptation     = %d\n"  , dssd.field.dfe_tap3_adaptation      ));
    EFUN_PRINTF((    "Disable Steady State DFE Tap4 Adaptation     = %d\n"  , dssd.field.dfe_tap4_adaptation      ));
    EFUN_PRINTF((    "Disable Steady State DFE Tap5 Adaptation     = %d\n"  , dssd.field.dfe_tap5_adaptation      ));
    EFUN_PRINTF((    "Disable Steady State DFE Tap1 DCD            = %d\n"  , dssd.field.dfe_tap1_dcd             ));
    EFUN_PRINTF((    "Disable Steady State DFE Tap2 DCD            = %d\n\n", dssd.field.dfe_tap2_dcd             ));
    ESTM_PRINTF((    "Clk90 offset Adjust                          = %d\n"  , rdv_usr_ctrl_clk90_offset_adjust()  ));
    ESTM_PRINTF((    "Clk90 offset Override                        = %d\n"  , rdv_usr_ctrl_clk90_offset_override()));
    ESTM_PRINTF((    "Lane Event Log Level                         = %d\n"  , rdv_usr_ctrl_lane_event_log_level() ));

    return(ERR_CODE_NONE);
}
#endif /* MERLIN7_PCIEG3 */

static err_code_t _display_ber_scan_data(srds_access_t *sa__, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    INIT_SRDS_ERR_CODE
    uint8_t i,prbs_byte,prbs_multi,time_byte,time_multi;
    uint16_t sts,dataword;
    int16_t offset_start;
    uint8_t cnt;
    uint32_t errors,timer_values;
    int32_t direction;
    int32_t rate;
    uint8_t range250;
    merlin8_phy1_osr_mode_st osr_mode;
    struct merlin8_phy1_uc_core_config_st core_config;
    err_code_t Exit_Status = ERR_CODE_NONE;

    {
        uint8_t dig_lpbk_en;
        ESTM(dig_lpbk_en = rd_dig_lpbk_en());
        if(dig_lpbk_en) {
            EFUN_PRINTF(("Eye scan is not supported while digital loopback is enabled.\n"));
            return (ERR_CODE_INVALID_DIG_LPBK_STATE);
        }
    }

    ENULL_MEMSET(&core_config,0,sizeof(core_config));
    ENULL_MEMSET(&osr_mode,0,sizeof(osr_mode));


#ifndef PCIEG3_V1
    CFUN(merlin8_phy1_get_uc_core_config(sa__, &core_config));
#endif
    CFUN(merlin8_phy1_INTERNAL_get_osr_mode(sa__, &osr_mode));

    if(osr_mode.rx > MERLIN8_PHY1_OSX4)
    {
        USR_PRINTF(("ERROR DIAG display_ber_data: osr mode too high\n"));
        Exit_Status = ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
        goto Exit;
    }

#ifndef PCIEG3_V1
    rate = core_config.vco_rate_in_Mhz/(1<<osr_mode.rx);
#else
    {
        uint8_t rate_sel;
        CSTM(rate_sel = rd_rate_sel());
        switch (rate_sel) {
        case 0: rate = 2500; break;
        case 1: rate = 5000; break;
        case 2: rate = 8000; break;
        default:
                {
                    Exit_Status = ERR_CODE_CONFLICTING_PARAMETERS;
                    goto Exit;
                }
        }
    }
#endif /* MERLIN7_PCIEG3 */

    direction = (ber_scan_mode & DIAG_BER_NEG) ? -1 : 1 ;
    range250 = (ber_scan_mode & DIAG_BER_P1_NARROW) ? 0 : 1;
    USR_PRINTF(("\n****  SERDES CORE %d LANE %d BER DATA    ****\n", merlin8_phy1_acc_get_core(sa__), merlin8_phy1_acc_get_lane(sa__)));
    USR_PRINTF(("BER MODE = %x %d %d\n",ber_scan_mode,timer_control,max_error_control));
    USR_PRINTF(("DATA RATE = %d Mhz\n",(int32_t)rate));
    /* start UC acquisition */
    if(DIAG_VERBOSE > 2) USR_PRINTF(("start begin\n"));
    CFUN(merlin8_phy1_start_ber_scan_test(sa__, ber_scan_mode, timer_control, max_error_control));
    CSTM(offset_start = (int16_t)rd_uc_dsc_data());
    if(ber_scan_mode & DIAG_BER_HORZ) {
        USR_PRINTF(("STARTING OFFSET = %d : %d mUI\n",offset_start,(offset_start*1000)>>6));
    } else {
        int16_t nlmv_val = 0;
        afe_override_slicer_sel_t slicer = INVALID_SLICER;

        CFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, offset_start, range250, slicer, &nlmv_val));
        USR_PRINTF(("STARTING OFFSET = %d : %d mV\n", offset_start, nlmv_val));
    }
    if(DIAG_VERBOSE > 2) USR_PRINTF(("start done\n"));

    USR_PRINTF(("Waiting for measurement time approx %d seconds",timer_control+(timer_control>>1)));
    CFUN(merlin8_phy1_INTERNAL_poll_diag_done(sa__, &sts,(uint32_t)(timer_control*2000)));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("delay done\n"));

    /* Check for completion read ln.diag_status byte?*/
    CSTM(sts = rdv_usr_diag_status());
    if((sts & 0x8000) == 0) {
        Exit_Status = ERR_CODE_DATA_NOTAVAIL;
        merlin8_phy1_error(sa__, Exit_Status);
        goto Exit;
    }
    cnt = (uint8_t)((sts & 0x00FF)/3);
    for(i=0;i < cnt;i++) {
        /* Read 2 bytes of data */
        CFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_READ_DIAG_DATA_WORD, 0, 200));
        CSTM(dataword = rd_uc_dsc_data());           /* LSB contains 2 -4bit nibbles */

        time_byte = (uint8_t)(dataword>>8);          /* MSB is time byte */
        prbs_multi = (uint8_t)dataword & 0x0F;       /* split nibbles */
        time_multi = (uint8_t)dataword>>4;
        /* Read 1 bytes of data */
        CFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_READ_DIAG_DATA_BYTE, 0, 200));
        CSTM(prbs_byte = (uint8_t)rd_uc_dsc_data());
        errors = merlin8_phy1_INTERNAL_float12_to_uint32(prbs_byte,prbs_multi); /* convert 12bits to uint32 */
        timer_values = (merlin8_phy1_INTERNAL_float12_to_uint32(time_byte,time_multi)<<3);
        if(DIAG_VERBOSE < 5) {
            if (!(i % 4))  {
                USR_PRINTF(("\n"));
            }
            if(ber_scan_mode & DIAG_BER_HORZ) {
                USR_PRINTF(("%d %d %d ",direction*(((SRDS_ABS(offset_start)-i)*1000)>>6),errors,timer_values));
            } else {
                int16_t nlmv_val = 0;
                afe_override_slicer_sel_t slicer = INVALID_SLICER;

                CFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, (int16_t)(SRDS_ABS(offset_start)-i), range250, slicer, &nlmv_val));
                USR_PRINTF(("%d %d %d ", direction * nlmv_val, errors, timer_values));
            }

        } else {
            USR_PRINTF(("BER Errors=%d (%02x<<%d): Time=%d (%02x<<%d)\n",errors,prbs_byte,prbs_multi,timer_values,time_byte,time_multi<<3));
        }
        /*if(timer_values == 0 && errors == 0) break;*/
    }
    USR_PRINTF(("\n"));
    CFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_CAPTURE_BER_END,0x00,2000));
Exit:

  return(merlin8_phy1_error_report(sa__, Exit_Status));
}

/*******************************************/
/*  Diagnostic Functions Required for SDK  */
/*******************************************/

/* Required Diagnostic Functions */

err_code_t merlin8_phy1_display_all_diag_data(srds_access_t *sa__, uint16_t  diag_level) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_display_diag_data_core(sa__, diag_level, 0xFF));
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_display_diag_data(srds_access_t *sa__, uint16_t  diag_level) {
    INIT_SRDS_ERR_CODE
    uint8_t lane = merlin8_phy1_acc_get_lane(sa__), lane_select=0;
    uint8_t num_lanes = 0;
    ESTM(num_lanes = rdc_revid_multiplicity());
    if(num_lanes > NUM_LANES_MAX || num_lanes == 0) {
        USR_PRINTF(("ERROR : rdc_revid_multiplicity returned unexpected value, check register reads are still functional\n"));
        return ERR_CODE_BAD_LANE_COUNT;
    }
    if (lane < num_lanes)
    {
        lane_select = (uint8_t)(0x1<<lane);
        EFUN(merlin8_phy1_display_diag_data_core(sa__, diag_level, lane_select));
    }
    else {
        return (ERR_CODE_BAD_LANE_COUNT);
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_diag_data_core(srds_access_t *sa__, uint16_t  diag_level, uint16_t lane_select) {
    INIT_SRDS_ERR_CODE
    uint8_t rx_lock, lane, num_lanes=0, save_lane=0;
    uint8_t micro_stop[NUM_LANES_MAX] = {0};
    uint8_t core;
    char core_type[25] = "merlin8_phy1";
    uint16_t data_cnt=0;
    uint32_t api_version;
    err_code_t diag_data_err = ERR_CODE_NONE;
    uint16_t diag_level_error = 0;
    srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
    uint8_t saveEyeReportFormat;
    EFUN(merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr));
    EFUN_PRINTF(("\n**** SERDES DISPLAY DIAG DATA ****\n"));
    ESTM_PRINTF(("Rev ID Letter        = %02X\n", rdc_revid_rev_letter()));
    ESTM_PRINTF(("Rev ID Process       = %02X\n", rdc_revid_process()));
    ESTM_PRINTF(("Rev ID Model         = %02X\n", rdc_revid_model()));
    ESTM_PRINTF(("Rev ID 2             = %02X\n", rdc_revid2()));
    ESTM_PRINTF(("Rev ID # Lanes       = %d\n"  ,rdc_revid_multiplicity()));
    ESTM_PRINTF(("Core  = %d;    LANE  = %d\n",merlin8_phy1_acc_get_core(sa__),merlin8_phy1_acc_get_lane(sa__)));
    EFUN(merlin8_phy1_version(sa__, &api_version));
    EFUN_PRINTF(("SERDES API Version   = %06X\n",api_version));
    ESTM_PRINTF(("Common Ucode Version = %04X", rdcv_common_ucode_version()));
    ESTM_PRINTF(("_%02X\n", rdcv_common_ucode_minor_version()));
    ESTM_PRINTF(("AFE Hardware Version = 0x%X", rdcv_afe_hardware_version()));
    EFUN_PRINTF(("\n"));
    EFUN_PRINTF(("SerDes type          = %s\n",core_type));
    EFUN_PRINTF(("\n"));
    ESTM(num_lanes = rdc_revid_multiplicity());
    save_lane = merlin8_phy1_acc_get_lane(sa__);
    saveEyeReportFormat = eyeReportFormat;
    eyeReportFormat = (uint8_t) (lane_select >> 8);
    /* stop micro tuning per lane, so all accesses are consistent */
    for (lane=0; lane<num_lanes; lane++) {
        if (lane_select & (0x1<<lane)) {
            err_code_t err_code=ERR_CODE_NONE;
            EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
            EFUN(merlin8_phy1_pmd_lock_status(sa__,&rx_lock));
            micro_stop[lane] = merlin8_phy1_INTERNAL_stop_micro(sa__,rx_lock,&err_code);
            if(err_code) USR_PRINTF(("Unable to stop lane %d micro,  following data is suspect\n", lane));
        }
    }
    EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
    if((diag_level & SRDS_DIAG_TIMESTAMP) || (diag_level & SRDS_DIAG_CORE)) {
        diag_data_err = merlin8_phy1_test_uc_timestamp(sa__);
        if(diag_data_err != ERR_CODE_NONE) {
            EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_TIMESTAMP returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
            diag_level_error |= SRDS_DIAG_TIMESTAMP;
        }
    }
    if(diag_level & SRDS_DIAG_CORE) {
        EFUN(merlin8_phy1_display_core_state_hdr(sa__));
        diag_data_err = merlin8_phy1_display_core_state_line(sa__);
        if(diag_data_err != ERR_CODE_NONE) {
            EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_CORE returned error %s.\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
            diag_level_error |= SRDS_DIAG_CORE;
        }
        EFUN_PRINTF(("\n"));
    }

    EFUN(merlin8_phy1_display_lane_state_hdr(sa__));
    save_lane = merlin8_phy1_acc_get_lane(sa__);
    for (lane=0; lane<num_lanes; lane++) {
        if (lane_select & (0x1<<lane)) {
            EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
            diag_data_err = merlin8_phy1_display_lane_state(sa__);
            if(diag_data_err != ERR_CODE_NONE) {
                EFUN_PRINTF(("\n**** WARNING: merlin8_phy1_display_lane_state() returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
            }
        }
    }
    EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));

#ifndef PCIEG3_V1
    if(diag_level & SRDS_DIAG_LANE) {

        struct merlin8_phy1_detailed_lane_status_st *lane_st;
        err_code_t Exit_Status = ERR_CODE_NONE;
        lane_st = (struct merlin8_phy1_detailed_lane_status_st *)USR_CALLOC(1, sizeof(struct merlin8_phy1_detailed_lane_status_st));
        if(!lane_st) {
#ifdef use_malloc
#endif
            return (ERR_CODE_MEM_ALLOC_FAIL);
        }
        save_lane = merlin8_phy1_acc_get_lane(sa__);
        for (lane=0; lane<num_lanes; lane++) {
            if (lane_select & (0x1<<lane)) {
                CFUN(merlin8_phy1_acc_set_lane(sa__, lane));
                CFUN(merlin8_phy1_log_full_pmd_state(sa__, lane_st));
                CFUN(merlin8_phy1_disp_full_pmd_state(sa__, lane_st, 1));
            }
        }
        CFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
Exit:
        USR_FREE(lane_st);
        if(Exit_Status != ERR_CODE_NONE) {
            EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_LANE returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(Exit_Status)));
            diag_level_error |= SRDS_DIAG_LANE;
        }
    }
#endif


    if(diag_level & SRDS_DIAG_EVENT_SAFE) {
        merlin8_phy1_INTERNAL_event_log_dump_state_t state;
        uint8_t micro_num = 0;
        {
            state.index = 0;
            state.line_start_index = 0;
            state.zero_cnt = 0;
            save_lane = merlin8_phy1_acc_get_lane(sa__);
            diag_data_err = merlin8_phy1_INTERNAL_read_event_log_with_callback(sa__, micro_num, 1, &state, merlin8_phy1_INTERNAL_event_log_dump_callback);
            EFUN(merlin8_phy1_acc_set_lane(sa__,save_lane));
            if(diag_data_err != ERR_CODE_NONE) {
                EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_EVENT_SAFE returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                diag_level_error |= SRDS_DIAG_EVENT_SAFE;
            }
            /* No register/ram reads or writes, local error handling not required */
            EFUN(merlin8_phy1_INTERNAL_event_log_dump_callback(sa__, &state, 0, 0));
        }
    }
    if(diag_level & SRDS_DIAG_EVENT) {
        save_lane = merlin8_phy1_acc_get_lane(sa__);
        diag_data_err = merlin8_phy1_read_event_log(sa__);
        EFUN(merlin8_phy1_acc_set_lane(sa__,save_lane));
        if(diag_data_err != ERR_CODE_NONE) {
            EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_EVENT returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
            diag_level_error |= SRDS_DIAG_EVENT;
        }

    }
    core = merlin8_phy1_acc_get_core(sa__);

    if (diag_level & SRDS_DIAG_REG_CORE) {
        diag_data_err = merlin8_phy1_reg_select_dump(sa__, SRDS_CORE_REG_ONLY, NULL);
        if(diag_data_err != ERR_CODE_NONE) {
            EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_REG_CORE returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
            diag_level_error = (uint16_t)(diag_level_error | (diag_level & SRDS_DIAG_REG_CORE));
        }
    }
    if (diag_level & SRDS_DIAG_REG_LANE) {
        save_lane = merlin8_phy1_acc_get_lane(sa__);
        for (lane=0; lane<num_lanes; lane++) {
            if (lane_select & (0x1<<lane)) {
                EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
                diag_data_err = merlin8_phy1_reg_select_dump(sa__, SRDS_LANE_REG_ONLY, NULL);
                if(diag_data_err != ERR_CODE_NONE) {
                    EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_REG_LANE returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                    diag_level_error = (uint16_t)(diag_level_error | (diag_level & SRDS_DIAG_REG_LANE));
                }
            }
        }
            EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
    }

    if(diag_level & SRDS_DIAG_UC_CORE) {
        data_cnt = (uint16_t)merlin8_phy1_info_ptr->core_var_ram_size;

        diag_data_err = merlin8_phy1_uc_core_var_blk_dump(sa__, core, data_cnt, NULL);
        if(diag_data_err != ERR_CODE_NONE) {
            EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_UC_CORE returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
            diag_level_error |= SRDS_DIAG_UC_CORE;
        }

    }
    if(diag_level & SRDS_DIAG_UC_LANE) {
        save_lane = merlin8_phy1_acc_get_lane(sa__);
        for (lane=0; lane<num_lanes; lane++) {
            if (lane_select & (0x1<<lane)) {
                EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
                diag_data_err = merlin8_phy1_uc_lane_var_blk_dump(sa__, core, lane, data_cnt, NULL);
                if(diag_data_err != ERR_CODE_NONE) {
                    EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_UC_LANE returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                    diag_level_error |= SRDS_DIAG_UC_LANE;
                }
            }
        }
        EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
    }
    if(diag_level & SRDS_DIAG_EYE) {
        save_lane = merlin8_phy1_acc_get_lane(sa__);
        for (lane=0; lane<num_lanes; lane++) {
            if (lane_select & (0x1<<lane)) {
                EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
                diag_data_err = merlin8_phy1_display_eye_scan(sa__);
                if(diag_data_err != ERR_CODE_NONE) {
                    EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_EYE returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                    diag_level_error |= SRDS_DIAG_EYE;
                }
            }
        }
        EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
    }
#ifndef PCIEG3_V1
    if(diag_level & SRDS_DIAG_LANE_DEBUG) {
        save_lane = merlin8_phy1_acc_get_lane(sa__);
        for (lane=0; lane<num_lanes; lane++) {
            if (lane_select & (0x1<<lane)) {
                EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
                diag_data_err = merlin8_phy1_display_lane_debug_status(sa__);
                if(diag_data_err != ERR_CODE_NONE) {
                    EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_LANE_DEBUG returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                    diag_level_error |= SRDS_DIAG_LANE_DEBUG;
                }
            }
        }
        EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
    }
#endif
    if(diag_level & SRDS_DIAG_BER_VERT) {
        save_lane = merlin8_phy1_acc_get_lane(sa__);
        for (lane=0; lane<num_lanes; lane++) {
            if (lane_select & (0x1<<lane)) {
                /* display ber projections for all channels */
                uint8_t ber_mode = DIAG_BER_VERT | DIAG_BER_POS;
                uint8_t timer_control = 23;       /* 30 seconds */
                uint8_t err_threshold = 100 / 16; /* 100 errors */
                diag_data_err = _display_ber_scan_data(sa__, ber_mode, timer_control,  err_threshold);
                if(diag_data_err != ERR_CODE_NONE) {
                    EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_BER_VERT (POS) returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                    diag_level_error |= SRDS_DIAG_BER_VERT;
                }
                ber_mode = DIAG_BER_VERT | DIAG_BER_NEG;
                diag_data_err = _display_ber_scan_data(sa__, ber_mode, timer_control,  err_threshold);
                if(diag_data_err != ERR_CODE_NONE) {
                    EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_BER_VERT (NEG) returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                    diag_level_error |= SRDS_DIAG_BER_VERT;
                }
            }
        }
        EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
    }
    if(diag_level & SRDS_DIAG_BER_HORZ) {
        save_lane = merlin8_phy1_acc_get_lane(sa__);
        for (lane=0; lane<num_lanes; lane++) {
            if (lane_select & (0x1<<lane)) {
                /* display ber projections for all channels */
                uint8_t ber_mode = DIAG_BER_HORZ | DIAG_BER_POS;
                uint8_t timer_control = 23;       /* 30 seconds */
                uint8_t err_threshold = 100 / 16; /* 100 errors */
                diag_data_err = _display_ber_scan_data(sa__, ber_mode, timer_control,  err_threshold);
                if(diag_data_err != ERR_CODE_NONE) {
                    EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_BER_HORZ (POS) returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                    diag_level_error |= SRDS_DIAG_BER_HORZ;
                }
                ber_mode = DIAG_BER_HORZ | DIAG_BER_NEG;
                diag_data_err = _display_ber_scan_data(sa__, ber_mode, timer_control,  err_threshold);
                if(diag_data_err != ERR_CODE_NONE) {
                    EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_BER_HORZ (NEG) returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
                    diag_level_error |= SRDS_DIAG_BER_HORZ;
                }
            }
        }
        EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
    }


    if(diag_level & SRDS_DIAG_INFO_TABLE) {
        diag_data_err = merlin8_phy1_INTERNAL_display_info_table(sa__);
        if(diag_data_err != ERR_CODE_NONE) {
            EFUN_PRINTF(("\n**** WARNING: SRDS_DIAG_INFO_TABLE returned error %s ****\n", merlin8_phy1_INTERNAL_e2s_err_code(diag_data_err)));
            diag_level_error |= SRDS_DIAG_INFO_TABLE;
        }

    }
    save_lane = merlin8_phy1_acc_get_lane(sa__);
    /* re enable micro tuning per lane*/
    for (lane=0; lane<num_lanes; lane++) {
        if ((lane_select & (0x1<<lane)) && (!micro_stop[lane])) {
            EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
            EFUN(merlin8_phy1_stop_rx_adaptation(sa__, 0));
        }
    }
    EFUN(merlin8_phy1_acc_set_lane(sa__, save_lane));
    if(diag_level_error) {
        EFUN_PRINTF(("\n**** ERROR: Diag level error reported: 0x%x ****\n", diag_level_error));
    }
    EFUN_PRINTF(("\n**** SERDES DISPLAY DIAG DATA END ****\n"));
    EFUN_PRINTF(("\n\n"));
#ifdef use_malloc
#endif
    eyeReportFormat = saveEyeReportFormat;

    return (ERR_CODE_NONE);

}

/*! Used for passing arguments in the below
 *  callback functions.
 */
typedef struct {
    uint32_t addr;
    uint8_t  line_byte_counter;
} merlin8_phy1_diag_access_callback_state_t;

static err_code_t merlin8_phy1_diag_access_read_byte_callback(srds_access_t *sa__, void *arg, uint8_t byte_count, uint16_t data) {
    merlin8_phy1_diag_access_callback_state_t * const state_ptr = (merlin8_phy1_diag_access_callback_state_t *)arg;
    while (byte_count > 0) {
        if (state_ptr->line_byte_counter == 0) {
            USR_PRINTF(("\n%04x  ", state_ptr->addr));
        }
        USR_PRINTF(("%02x ", data & 0xFF));
        ++state_ptr->addr;
        state_ptr->line_byte_counter = (uint8_t)((state_ptr->line_byte_counter+1) % 16);
        data >>= 8;
        --byte_count;
    }
    return (ERR_CODE_NONE);
}

static err_code_t merlin8_phy1_diag_access_read_word_callback(srds_access_t *sa__, void *arg, uint8_t byte_count, uint16_t data) {
    merlin8_phy1_diag_access_callback_state_t * const state_ptr = (merlin8_phy1_diag_access_callback_state_t *)arg;
    UNUSED(byte_count);

    if (state_ptr->line_byte_counter == 0) {
        USR_PRINTF(("\n%04x  ", state_ptr->addr));
    }
    USR_PRINTF(("%04x ", data));
    state_ptr->addr = state_ptr->addr + 2;
    state_ptr->line_byte_counter = (uint8_t)((state_ptr->line_byte_counter+2) % 16);
    return (ERR_CODE_NONE);
}

/* Required Diagnostic Functions */
err_code_t merlin8_phy1_diag_access(srds_access_t *sa__, enum srds_diag_access_enum type, uint32_t addr, uint32_t data, uint16_t param) {
    INIT_SRDS_ERR_CODE
    srds_info_t * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
    merlin8_phy1_diag_access_callback_state_t state;

    EFUN(merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr));
    state.addr = addr;
    state.line_byte_counter = 0;


    switch(type) {
    case SRDS_REG_READ:      {
                             uint16_t tmp = 0;
                             if(data > 1) {
                                 uint32_t i;
                                 EFUN_PRINTF(("\n****  SERDES BLK REGISTER READ    ****"));
                                 for (i = 0; i < data; i++) {
                                     if (!(i % 16))  {
                                         ESTM_PRINTF(("\n%04x ",i+addr));
                                     }
                                     EFUN(merlin8_phy1_acc_rdt_reg(sa__, (uint16_t)(i+addr),&tmp));
                                     ESTM_PRINTF(("%04x ", tmp));
                                 }
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 EFUN(merlin8_phy1_acc_rdt_reg(sa__, (uint16_t)addr,&tmp));
                                 EFUN_PRINTF(("Register Read: x%04x = x%04x\n",addr,tmp));
                             }
                             } break;
    case SRDS_REG_RMW:       {
                             uint16_t tmp = 0, tmp2 = 0;
                             EFUN(merlin8_phy1_acc_rdt_reg(sa__, (uint16_t)addr,&tmp));
                             tmp2 = (uint16_t)((tmp & ~param) | (param & (uint16_t)data));
                             EFUN(merlin8_phy1_acc_wr_reg(sa__, (uint16_t)addr,tmp2));
                             EFUN_PRINTF(("Register RMW: x%04x = x%04x -> x%04x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_CORE_RAM_READ_BYTE: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK CORE RAM READ BYTE   ****"));
                                 EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           addr + merlin8_phy1_info_ptr->core_var_ram_base,
                                                                           data,
                                                                           0,
                                                                           data,
                                                                           &state,
                                                                           merlin8_phy1_diag_access_read_byte_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Core RAM Read byte: x%04x = x%02x\n",(uint8_t)addr,merlin8_phy1_rdbc_uc_var(sa__, __ERR, (uint8_t)addr)));
                             }
                             } break;
    case SRDS_LANE_RAM_READ_BYTE: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK LANE RAM READ BYTE   ****"));
                                 EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__, (addr + merlin8_phy1_info_ptr->lane_var_ram_base + (merlin8_phy1_acc_get_physical_lane(sa__) * merlin8_phy1_info_ptr->lane_var_ram_size)),
                                                                           data,
                                                                           0,
                                                                           data,
                                                                           &state,
                                                                           merlin8_phy1_diag_access_read_byte_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Lane RAM Read byte: x%04x = x%02x\n",addr,merlin8_phy1_rdbl_uc_var(sa__, __ERR, (uint16_t)addr)));
                             }
                             } break;
    case SRDS_CORE_RAM_READ_WORD: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK CORE RAM READ WORD   ****"));
                                 EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           addr + merlin8_phy1_info_ptr->core_var_ram_base,
                                                                           ((data+1)>>1)<<1, /* Round up to nearest word count */
                                                                           0,
                                                                           ((data+1)>>1)<<1, /* Round up to nearest word count */
                                                                           &state,
                                                                           merlin8_phy1_diag_access_read_word_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Core RAM Read word: x%04x = x%04x\n",addr,merlin8_phy1_rdwc_uc_var(sa__, __ERR, (uint8_t)addr)));
                             }
                             } break;
    case SRDS_LANE_RAM_READ_WORD: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK LANE RAM READ WORD   ****"));
                                 EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__, (addr + merlin8_phy1_info_ptr->lane_var_ram_base + (merlin8_phy1_acc_get_physical_lane(sa__) * merlin8_phy1_info_ptr->lane_var_ram_size)),
                                                                           ((data+1)>>1)<<1, /* Round up to nearest word count */
                                                                           0,
                                                                           ((data+1)>>1)<<1, /* Round up to nearest word count */
                                                                           &state,
                                                                           merlin8_phy1_diag_access_read_word_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Lane RAM Read word: x%04x = x%04x\n",addr,merlin8_phy1_rdwl_uc_var(sa__, __ERR, (uint16_t)addr)));
                             }
                             } break;
    case SRDS_CORE_RAM_RMW_BYTE:  {
                             uint8_t tmp, tmp2;
                             ESTM(tmp = merlin8_phy1_rdbc_uc_var(sa__, __ERR, (uint8_t)addr));
                             tmp2 = (uint8_t)((tmp & (uint8_t)~param) | ((uint8_t)param & data));
                             EFUN(merlin8_phy1_wrbc_uc_var(sa__, (uint8_t)addr,tmp2));
                             EFUN_PRINTF(("Core RAM RMW byte: x%04x = x%02x -> x%02x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_LANE_RAM_RMW_BYTE:  {
                             uint8_t tmp, tmp2;
                             ESTM(tmp = merlin8_phy1_rdbl_uc_var(sa__, __ERR, (uint16_t)addr));
                             tmp2 = (uint8_t)((tmp & (uint8_t)~param) | ((uint8_t)param & data));
                             EFUN(merlin8_phy1_wrbl_uc_var(sa__, (uint16_t)addr,tmp2));
                             EFUN_PRINTF(("Lane RAM RMW byte: x%04x = x%02x -> x%02x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_CORE_RAM_RMW_WORD:  {
                             uint16_t tmp, tmp2;
                             ESTM(tmp = merlin8_phy1_rdwc_uc_var(sa__, __ERR, (uint8_t)addr));
                             tmp2 = (uint16_t)((tmp & ~param) | (param & (uint16_t)data));
                             EFUN(merlin8_phy1_wrwc_uc_var(sa__, (uint8_t)addr,tmp2));
                             EFUN_PRINTF(("Core RAM RMW word: x%04x = x%04x -> x%04x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_LANE_RAM_RMW_WORD:  {
                             uint16_t tmp, tmp2;
                             ESTM(tmp = merlin8_phy1_rdwl_uc_var(sa__, __ERR,(uint16_t)addr));
                             tmp2 = (uint16_t)((tmp & ~param) | (param & (uint16_t)data));
                             EFUN(merlin8_phy1_wrwl_uc_var(sa__, (uint16_t)addr,tmp2));
                             EFUN_PRINTF(("Lane RAM RMW word: x%04x = x%04x -> x%04x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_GLOB_RAM_READ_BYTE: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK GLOB RAM READ BYTE   ****"));
                                 EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           DATA_RAM_BASE | addr,
                                                                           data,
                                                                           0,
                                                                           data,
                                                                           &state,
                                                                           merlin8_phy1_diag_access_read_byte_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Glob RAM Read byte: x%04x = x%02x\n",addr,merlin8_phy1_INTERNAL_rdb_uc_var(sa__, __ERR, DATA_RAM_BASE | addr)));
                             }
                             } break;
    case SRDS_GLOB_RAM_RMW_BYTE:  {
                             uint8_t tmp, tmp2;
                             ESTM(tmp = merlin8_phy1_INTERNAL_rdb_uc_var(sa__, __ERR, DATA_RAM_BASE | addr));
                             tmp2 = (uint8_t)((tmp & (uint8_t)~param) | ((uint8_t)param & data));
                             EFUN(merlin8_phy1_INTERNAL_wrb_uc_var(sa__, DATA_RAM_BASE | addr, tmp2));
                             EFUN_PRINTF(("Glob RAM RMW byte: x%04x = x%02x -> x%02x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_GLOB_RAM_READ_WORD: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK GLOB RAM READ WORD   ****"));
                                 EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           DATA_RAM_BASE | addr,
                                                                           ((data+1)>>1)<<1, /* Round up to nearest word count */
                                                                           0,
                                                                           ((data+1)>>1)<<1, /* Round up to nearest word count */
                                                                           &state,
                                                                           merlin8_phy1_diag_access_read_word_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Glob RAM Read word: x%04x = x%04x\n",addr,merlin8_phy1_INTERNAL_rdw_uc_var(sa__, __ERR, DATA_RAM_BASE | addr)));
                             }
                             } break;
    case SRDS_GLOB_RAM_RMW_WORD:  {
                             uint16_t tmp, tmp2;
                             ESTM(tmp = merlin8_phy1_INTERNAL_rdw_uc_var(sa__, __ERR, DATA_RAM_BASE | addr));
                             tmp2 = (uint16_t)((tmp & ~param) | (param & (uint16_t)data));
                             EFUN(merlin8_phy1_INTERNAL_wrw_uc_var(sa__, DATA_RAM_BASE | addr, tmp2));
                             EFUN_PRINTF(("Glob RAM RMW word: x%04x = x%04x -> x%04x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_UC_CMD:        {
                             uint16_t tmp;
                             EFUN(merlin8_phy1_pmd_uc_cmd_with_data(sa__, (enum srds_pmd_uc_cmd_enum)addr, (uint8_t)param, (uint16_t)data, GRACEFUL_STOP_TIME));
                             ESTM(tmp = rd_uc_dsc_data());
                             EFUN_PRINTF(("uC Command: cmd=x%02x supp=x%02x data=x%04x returned=x%04x\n",addr,param,data,tmp));
                             } break;
    case SRDS_PROG_RAM_READ_BYTE: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK PROG RAM READ BYTE   ****"));
                                 EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           addr,
                                                                           data,
                                                                           0,
                                                                           data,
                                                                           &state,
                                                                           merlin8_phy1_diag_access_read_byte_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Prog RAM Read byte: x%04x = x%02x\n",addr,merlin8_phy1_INTERNAL_rdb_uc_var(sa__, __ERR, addr)));
                             }
                             } break;
    case SRDS_PROG_RAM_READ_WORD: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK PROG RAM READ WORD   ****"));
                                 EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           addr,
                                                                           ((data+1)>>1)<<1, /* Round up to nearest word count */
                                                                           0,
                                                                           ((data+1)>>1)<<1, /* Round up to nearest word count */
                                                                           &state,
                                                                           merlin8_phy1_diag_access_read_word_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Prog RAM Read word: x%04x = x%04x\n",addr,merlin8_phy1_INTERNAL_rdw_uc_var(sa__, __ERR, addr)));
                             }
                             } break;
    case SRDS_EN_BREAKPOINT: {
        EFUN(merlin8_phy1_INTERNAL_en_breakpoint(sa__, (uint8_t)addr));
                             } break;
    case SRDS_GOTO_BREAKPOINT: {
        EFUN(merlin8_phy1_INTERNAL_goto_breakpoint(sa__, (uint8_t)addr));
                             } break;
    case SRDS_RD_BREAKPOINT: {
        EFUN(merlin8_phy1_INTERNAL_rd_breakpoint(sa__));
                             } break;
    case SRDS_DIS_BREAKPOINT: {
        EFUN(merlin8_phy1_INTERNAL_dis_breakpoint(sa__));
                             } break;
    case SRDS_BER_PROJ_DATA: {
                             /* display ber projections for all channels */
                             EFUN(_display_ber_scan_data(sa__, (uint8_t)addr, (uint8_t)data,  (uint8_t)(param>>4)));
                             } break;

    default: EFUN_PRINTF(("Invalid request type merlin8_phy1_diag_access\n"));
    }

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_display_state (srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_display_core_state(sa__));
    EFUN(merlin8_phy1_display_lane_state_hdr(sa__));
    EFUN(merlin8_phy1_display_lane_state(sa__));
    EFUN(merlin8_phy1_display_lane_state_legend(sa__));
    return(ERR_CODE_NONE);
}


#ifndef PCIEG3_V1
err_code_t merlin8_phy1_display_config (srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_display_core_config(sa__));
    EFUN(merlin8_phy1_display_lane_config(sa__));
    return(ERR_CODE_NONE);
}
#endif

/*************************/
/*  Temperature forcing  */
/*************************/

err_code_t merlin8_phy1_force_die_temperature (srds_access_t *sa__, int16_t die_temp) {
    INIT_SRDS_ERR_CODE
    /* disable force */
    if(die_temp == -255) {
        EFUN(wrc_micro_pvt_tempdata_frc(0));
        return(ERR_CODE_NONE);
    }

    /* enable force */
    if (die_temp>130)
        die_temp = 130;
    if (die_temp<-45)
        die_temp = -45;

    EFUN(wrc_micro_pvt_tempdata_frcval((uint16_t)(_degC_to_bin(die_temp))));
    EFUN(wrc_micro_pvt_tempdata_frc(1));

    return(ERR_CODE_NONE);
}

/**************************/
/*  Temperature reading   */
/**************************/

err_code_t merlin8_phy1_read_die_temperature (srds_access_t *sa__, int16_t *die_temp) {
    INIT_SRDS_ERR_CODE
    uint16_t die_temp_sensor_reading;

    ESTM(die_temp_sensor_reading=rdc_micro_pvt_tempdata_rmi());
    *die_temp = (int16_t)(_bin_to_degC(die_temp_sensor_reading));

    return(ERR_CODE_NONE);
}


err_code_t merlin8_phy1_read_die_temperature_double (srds_access_t *sa__, USR_DOUBLE *die_temp) {
#ifdef SERDES_API_FLOATING_POINT
    uint16_t die_temp_sensor_reading;
    INIT_SRDS_ERR_CODE

    ESTM(die_temp_sensor_reading=rdc_micro_pvt_tempdata_rmi());

    *die_temp = _bin_to_degC_double(die_temp_sensor_reading);

    return(ERR_CODE_NONE);
#else
    *die_temp = 0;
    EFUN_PRINTF((" Function 'merlin8_phy1_read_die_temperature_double' needs SERDES_API_FLOATING_POINT defined to operate \n"));
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
#endif
}

#ifndef MERLIN8_PHY1_NO_CL72
/**********************/
/*  CL72/CL93 Status  */
/**********************/


err_code_t merlin8_phy1_display_cl72_status(srds_access_t *sa__) {
  INIT_SRDS_ERR_CODE
  EFUN_PRINTF(("\n\n************************\n"                                                                                                                              ));
  ESTM_PRINTF((   "** LANE %d CL72 Status **\n"                                                                                        , merlin8_phy1_acc_get_lane(sa__)               ));
  EFUN_PRINTF((    "************************\n"                                                                                                                              ));
  ESTM_PRINTF((    "cl72_signal_detect         = %d   (1 = CL72 training FSM in SEND_DATA state;  0 = CL72 in training state)\n"       , rd_cl72_training_fsm_signal_detect()));
  ESTM_PRINTF((    "cl72_ieee_training_failure = %d   (1 = Training failure detected;             0 = Training failure not detected)\n", rd_cl72_ieee_training_failure()     ));
  ESTM_PRINTF((    "cl72_ieee_training_status  = %d   (1 = Start-up protocol in progress;         0 = Start-up protocol complete)\n"   , rd_cl72_ieee_training_status()      ));
  ESTM_PRINTF((    "cl72_ieee_receiver_status  = %d   (1 = Receiver trained and ready to receive; 0 = Receiver training)\n\n"          , rd_cl72_ieee_receiver_status()      ));
  return(ERR_CODE_NONE);
}



#endif /* MERLIN7_PCIEG3 */
#endif /* SMALL_FOOTPRINT */
