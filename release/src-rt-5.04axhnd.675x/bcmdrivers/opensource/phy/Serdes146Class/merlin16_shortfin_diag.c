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

#ifndef EXCLUDE_STD_HEADERS
#include <stdio.h>
#include <math.h>
#endif

#include "merlin16_shortfin_diag.h"
#include "merlin16_shortfin_access.h"
#include "merlin16_shortfin_config.h"
#include "merlin16_shortfin_debug_functions.h"
#include "merlin16_shortfin_functions.h"
#include "merlin16_shortfin_internal.h"
#include "merlin16_shortfin_internal_error.h"
#include "merlin16_shortfin_prbs.h"
#include "merlin16_shortfin_select_defns.h"


/************************************/
/*  Display Eye Scan                */
/************************************/


/* Timestamp check to see if heartbeat timer is programmed correctly for the COMCLK frequency it is running at */
err_code_t merlin16_shortfin_test_uc_timestamp(srds_access_t *sa__) {

#ifdef SERDES_API_FLOATING_POINT
    uint16_t time = 0;
    USR_DOUBLE resolution;
    const uint16_t test_delay=400;

    const USR_DOUBLE max_resolution=11.0;
    const USR_DOUBLE min_resolution=9.0;

    EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_UC_DBG,CMD_UC_DBG_TIMESTAMP,100));
    ESTM(time=rd_uc_dsc_data());
    USR_DELAY_MS(test_delay);
    EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_UC_DBG,CMD_UC_DBG_TIMESTAMP,100));
    ESTM(time=(uint16_t)(rd_uc_dsc_data()-time));
    resolution=test_delay*1000.0/(double)time;
    if((resolution<min_resolution)||(resolution>max_resolution)){
        EFUN_PRINTF(("\nERROR : Lane %i:\tuC timestamp: %i\t\tResolution: %.1fms/count. Passing resolution limits:\tMax: %.1fms/count\tMin: %.1fms/count\n",merlin16_shortfin_get_lane(sa__),time,resolution,max_resolution,min_resolution));
        return(ERR_CODE_DIAG_TIMESTAMP_FAIL);
    }
    else {
        EFUN_PRINTF(("\nPassed timestamp check : Lane %i:\tuC timestamp: %i\t\tResolution: %.1fms/count. Passing resolution limits:\tMax: %.1fms/count\tMin: %.1fms/count\n",merlin16_shortfin_get_lane(sa__),time,resolution,max_resolution,min_resolution));
   }
#else
    EFUN_PRINTF(("This function needs SERDES_API_FLOATING_POINT define to operate \n"));
#endif
    return(ERR_CODE_NONE);
}


/* This is best method for terminal ASCII display */
err_code_t merlin16_shortfin_display_eye_scan(srds_access_t *sa__) {
    uint32_t   stripe[64] = {0};
    uint16_t   status = 0;
    int8_t     y,y_max,y_step;
    uint8_t    core,lane;

    y_max  = EYE_SCAN_NRZ_VERTICAL_IDX_MAX;
    y_step = EYE_SCAN_NRZ_VERTICAL_STEP;
    ESTM(core=merlin16_shortfin_get_core(sa__));
    ESTM(lane=merlin16_shortfin_get_lane(sa__));
    EFUN_PRINTF(("\n\n****  SERDES EYE SCAN CORE %d LANE %d   ****\n",core,lane));
    EFUN(merlin16_shortfin_display_eye_scan_header(1));

    /* start horizontal acquisition */
    {   err_code_t err_code = merlin16_shortfin_meas_eye_scan_start(sa__, EYE_SCAN_HORIZ);
        if (err_code)
        {
            if (err_code == ERR_CODE_DIAG_SCAN_NO_PMD_LOCK) {
                return(ERR_CODE_NONE);
            }
            else
            {
                EFUN((merlin16_shortfin_meas_eye_scan_done(sa__), err_code));
            }
        }
    }

    for (y = y_max;y>=-y_max;y=(int8_t)(y-y_step))
    {
        {   err_code_t err_code = merlin16_shortfin_read_eye_scan_stripe(sa__, &stripe[0], &status);
            if (err_code) {
                EFUN((merlin16_shortfin_meas_eye_scan_done(sa__), err_code));
            }
        }
        EFUN(merlin16_shortfin_display_eye_scan_stripe(sa__, y,&stripe[0]));
        EFUN_PRINTF(("\n"));
    }
    /* stop acquisition */
    EFUN(merlin16_shortfin_meas_eye_scan_done(sa__));
    EFUN(merlin16_shortfin_display_eye_scan_footer(1));

    return(ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_meas_eye_scan_start(srds_access_t *sa__, uint8_t direction) {
    uint8_t lock;

    ESTM(lock = rd_pmd_rx_lock());
    { 
        uint8_t sts;

        ESTM((sts = rdv_usr_sts_micro_stopped()));
        if((lock == 0) && (sts != DB_STOPPED)) {
              EFUN_PRINTF(("Error: No PMD_RX_LOCK on lane requesting 2D eye scan\n"));
              return(ERR_CODE_DIAG_SCAN_NO_PMD_LOCK);
        }
    }
    if(direction == EYE_SCAN_VERTICAL) {
        EFUN(merlin16_shortfin_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_START_VSCAN_EYE,GRACEFUL_STOP_TIME));
    } else if(direction == EYE_SCAN_HORIZ) {
        EFUN(merlin16_shortfin_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_START_HSCAN_EYE,GRACEFUL_STOP_TIME));
    } else if(direction == EYE_SCAN_SLICE) {
        EFUN(merlin16_shortfin_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_START_EYE_SLICE,GRACEFUL_STOP_TIME));
    }
    return(ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_INTERNAL_poll_diag_data(srds_access_t *sa__, srds_info_t *merlin16_shortfin_info_ptr, uint16_t *status, uint8_t *diag_rd_ptr, uint8_t byte_count, uint32_t timeout_ms) {
    uint32_t lane_diag_size;
    uint16_t loop;

    CHECK_AND_RETURN_IF_NULL(merlin16_shortfin_info_ptr, ERR_CODE_INVALID_INFO_TABLE_ADDR);
    lane_diag_size = merlin16_shortfin_info_ptr->diag_mem_ram_size;
    *diag_rd_ptr = 0;

    if(!status) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    /** If the byte_count is too high, then there might be problems not updating
     *  the read pointer fast enough.
     */
    if (byte_count > (lane_diag_size / 2)) {
        ESTM_PRINTF(("\nERROR : merlin16_shortfin_INTERNAL_poll_diag_data() has excessive byte count of %d.\n", byte_count));
        return (merlin16_shortfin_error(sa__, ERR_CODE_DIAG_TIMEOUT));
    }

    ESTM(*diag_rd_ptr = rdv_usr_diag_rd_ptr());

    /* Wait until byte_count bytes are available to be read in the diagnostic memory. */
    loop = 0;
    while (1) {
        uint8_t diag_wr_ptr, full_count;

        ESTM(diag_wr_ptr = rdv_usr_diag_wr_ptr());
        if (diag_wr_ptr >= *diag_rd_ptr) {
            full_count = (uint8_t)(diag_wr_ptr - *diag_rd_ptr);
        } else {
            full_count = (uint8_t)((uint16_t)diag_wr_ptr + lane_diag_size - *diag_rd_ptr);
        }
        if (full_count >= byte_count) {
            break;
        }

        ++loop;
        if (loop > 10) {
            EFUN(USR_DELAY_US(10*timeout_ms));
        }
        if (loop > 1000) {
            return(merlin16_shortfin_error(sa__, ERR_CODE_DIAG_TIMEOUT));
        }
    }
    ESTM(*status = rdv_usr_diag_status() & 0xFF);
    return(ERR_CODE_NONE);
}

typedef struct {
    uint32_t *buffer_ptr;
    } merlin16_shortfin_read_eye_scan_stripe_callback_arg_t;

static err_code_t merlin16_shortfin_read_eye_scan_stripe_callback(void *arg, uint8_t byte_count, uint16_t data) {
    merlin16_shortfin_read_eye_scan_stripe_callback_arg_t * const cast_arg = (merlin16_shortfin_read_eye_scan_stripe_callback_arg_t *)arg;
    *(cast_arg->buffer_ptr++) = merlin16_shortfin_INTERNAL_float8_to_int32((float8_t)(data & 0xFF));
    if (byte_count > 1) {
        *(cast_arg->buffer_ptr++) = merlin16_shortfin_INTERNAL_float8_to_int32((float8_t)(data >> 8));
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_read_eye_scan_stripe(srds_access_t *sa__, uint32_t *buffer, uint16_t *status) {
    srds_info_t * merlin16_shortfin_info_ptr = merlin16_shortfin_INTERNAL_get_merlin16_shortfin_info_ptr_with_check(sa__);
    const uint8_t lane = merlin16_shortfin_get_lane(sa__);
    const uint8_t stripe_size = 64;

    merlin16_shortfin_read_eye_scan_stripe_callback_arg_t arg;
    uint32_t lane_diag_base;
    uint8_t diag_rd_ptr;

    CHECK_AND_RETURN_IF_NULL(merlin16_shortfin_info_ptr, ERR_CODE_INVALID_INFO_TABLE_ADDR);
    if(!buffer || !status) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    EFUN(merlin16_shortfin_INTERNAL_verify_merlin16_shortfin_info(merlin16_shortfin_info_ptr, sa__));

        lane_diag_base = merlin16_shortfin_info_ptr->diag_mem_ram_base + ((lane%merlin16_shortfin_info_ptr->lane_count) * merlin16_shortfin_info_ptr->diag_mem_ram_size) +
            (uint32_t)(merlin16_shortfin_info_ptr->grp_ram_size*merlin16_shortfin_INTERNAL_grp_idx_from_lane(merlin16_shortfin_get_physical_lane(sa__)));

    EFUN(merlin16_shortfin_INTERNAL_poll_diag_data(sa__, merlin16_shortfin_info_ptr, status, &diag_rd_ptr, stripe_size, 400));

    arg.buffer_ptr = buffer;
    EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__,
                                              lane_diag_base,
                                              (uint16_t)merlin16_shortfin_info_ptr->diag_mem_ram_size,
                                              diag_rd_ptr,
                                              stripe_size,
                                              &arg,
                                              merlin16_shortfin_read_eye_scan_stripe_callback));

    diag_rd_ptr = (uint8_t)(((uint32_t)diag_rd_ptr + stripe_size) % merlin16_shortfin_info_ptr->diag_mem_ram_size);
    EFUN(wrv_usr_diag_rd_ptr(diag_rd_ptr));
    ESTM(*status = rdv_usr_diag_status() & 0xFF);
    return(ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_display_eye_scan_stripe(srds_access_t *sa__, int8_t y,uint32_t *buffer) {

    const uint32_t limits[7] = {917504, 91750, 9175, 917, 91, 9, 1};

    int8_t x,i;
    uint8_t data_thresh;
    int16_t level;


    ESTM(data_thresh = rd_p1_thresh_sel());
    level = merlin16_shortfin_INTERNAL_ladder_setting_to_mV(sa__, y,data_thresh);

    if(!buffer) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    EFUN_PRINTF(("%6dmV : ", level));

    for (x=-31;x<32;x++) {
        for (i=0;i<7;i++) {
            if (buffer[x+31]>=limits[i]) {
                EFUN_PRINTF(("%c", '0'+i+1));
                break;
            }
        }
        if (i==7) {
            if      ((x%5)==0 && (y%5)==0) {EFUN_PRINTF(("+"));}
            else if ((x%5)!=0 && (y%5)==0) {EFUN_PRINTF(("-"));}
            else if ((x%5)==0 && (y%5)!=0) {EFUN_PRINTF((":"));}
            else                           {EFUN_PRINTF((" "));}
        }
    }
    return(ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_display_eye_scan_header(int8_t i) {
int8_t x;
    EFUN_PRINTF(("\n"));
    EFUN_PRINTF((" Each character N represents approximate error rate 1e-N at that location\n"));
    for(x=1;x<=i;x++) {
        EFUN_PRINTF(("  UI/64  : -30  -25  -20  -15  -10  -5    0    5    10   15   20   25   30"));
    }
    EFUN_PRINTF(("\n"));
    for(x=1;x<=i;x++) {
        EFUN_PRINTF(("         : -|----|----|----|----|----|----|----|----|----|----|----|----|-"));
    }
    EFUN_PRINTF(("\n"));
    return(ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_display_eye_scan_footer(int8_t i) {
int8_t x;
    for(x=1;x<=i;x++) {
        EFUN_PRINTF(("         : -|----|----|----|----|----|----|----|----|----|----|----|----|-"));
    }
    EFUN_PRINTF(("\n"));
    for(x=1;x<=i;x++) {
        EFUN_PRINTF(("  UI/64  : -30  -25  -20  -15  -10  -5    0    5    10   15   20   25   30"));
    }
    EFUN_PRINTF(("\n"));
    return(ERR_CODE_NONE);
}


err_code_t merlin16_shortfin_read_eye_scan_status(srds_access_t *sa__, uint16_t *status) {

   if(!status) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

   ESTM(*status=rdv_usr_diag_status());

    return(ERR_CODE_NONE);
}


err_code_t merlin16_shortfin_meas_eye_scan_done(srds_access_t *sa__) {
  EFUN(merlin16_shortfin_pmd_uc_diag_cmd(sa__, CMD_UC_DIAG_DISABLE,GRACEFUL_STOP_TIME));
  return(ERR_CODE_NONE);
}


err_code_t merlin16_shortfin_start_ber_scan_test(srds_access_t *sa__, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    uint8_t lock,sts;
    ESTM(lock = rd_pmd_rx_lock());
    if(lock == 0) {
        EFUN_PRINTF(("Error: No PMD_RX_LOCK on lane requesting BER scan\n"));
        return(ERR_CODE_DIAG_SCAN_NOT_COMPLETE);
    }
    ESTM(sts =rdv_usr_sts_micro_stopped());
    if(sts > 1) {
       EFUN_PRINTF(("Error: Lane is busy (%d) requesting BER scan\n",sts));
        return(ERR_CODE_DIAG_SCAN_NOT_COMPLETE);
    }

    EFUN(wrcv_diag_max_time_control(timer_control));
    EFUN(wrcv_diag_max_err_control(max_error_control));
    EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_CAPTURE_BER_START, ber_scan_mode,GRACEFUL_STOP_TIME));
    return(ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_read_ber_scan_data(srds_access_t *sa__, uint32_t *errors, uint32_t *timer_values, uint8_t *cnt, uint32_t timeout) {
    uint8_t i,prbs_byte,prbs_multi,time_byte,time_multi;
    uint16_t sts,dataword;


    if(!errors || !timer_values || !cnt) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
    /* init data arrays */
    for(i=0;i< DIAG_MAX_SAMPLES;i++) {
        errors[i]=0;
        timer_values[i]=0;
    }
    /* Check for completion read ln.diag_status byte?*/
 ESTM(sts = rdv_usr_diag_status());
    if((sts & 0x8000) == 0) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_DATA_NOTAVAIL));
    }
    *cnt = (uint8_t)((sts & 0x00FF)/3);
    for(i=0;i < *cnt;i++) {
        /* Read 2 bytes of data */
        EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_READ_DIAG_DATA_WORD, 0, timeout));
        ESTM(dataword = rd_uc_dsc_data());           /* LSB contains 2 -4bit nibbles */
        time_byte = (uint8_t)(dataword>>8);          /* MSB is time byte */
        prbs_multi = (uint8_t)dataword & 0x0F;       /* split nibbles */
        time_multi = (uint8_t)dataword>>4;
        /* Read 1 bytes of data */
        EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_READ_DIAG_DATA_BYTE, 0, timeout));
        ESTM(prbs_byte = (uint8_t)rd_uc_dsc_data());
        errors[i] = merlin16_shortfin_INTERNAL_float12_to_uint32(prbs_byte,prbs_multi); /* convert 12bits to uint32 */
        timer_values[i] = (merlin16_shortfin_INTERNAL_float12_to_uint32(time_byte,time_multi)<<3);
    /*  EFUN_PRINTF(("Err=%d (%02x<<%d); Time=%d (%02x<<%d)\n",errors[i],prbs_byte,prbs_multi,timer_values[i],time_byte,time_multi<<3)); */
        /*if(timer_values[i] == 0 && errors[i] == 0) break;*/
    }

  return(ERR_CODE_NONE);
}


/* This is good example function to do BER extrapolation */
err_code_t merlin16_shortfin_eye_margin_proj(srds_access_t *sa__, USR_DOUBLE rate, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    uint32_t errs[DIAG_MAX_SAMPLES];
    uint32_t time[DIAG_MAX_SAMPLES];
    uint8_t i,cnt=0;
    uint16_t sts;
    int16_t offset_start;
    /* Below 'DIAG_VERBOSE' level is intended to be modified only within a debug */
    /* session immediately after a breakpoint, and to retain its state only */
    /* through function exit:  therefore it must be 'volatile' to prevent a */
    /* compiler from eliding code conditioned on it, but NOT 'static'.      */


    for(i=0;i<DIAG_MAX_SAMPLES;i++) {
        errs[i]=0;
        time[i]=0;
    }
    /* start UC acquisition */
    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("start begin\n"));
    EFUN(merlin16_shortfin_start_ber_scan_test(sa__, ber_scan_mode, timer_control, max_error_control));
    ESTM(offset_start = (int16_t)rd_uc_dsc_data());
    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("offset_start = %d:%dmV\n",offset_start,merlin16_shortfin_INTERNAL_ladder_setting_to_mV(sa__, (int8_t)offset_start,0)));
    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("start done\n"));

    EFUN_PRINTF(("Waiting for measurement time approx %d seconds",timer_control+(timer_control>>1)));
    EFUN(merlin16_shortfin_INTERNAL_poll_diag_done(sa__, &sts,(uint32_t)(timer_control*2000)));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("delay done\n"));

    EFUN(merlin16_shortfin_read_ber_scan_data(sa__, &errs[0], &time[0], &cnt, 2000));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("read done cnt=%d\n",cnt));

    EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_CAPTURE_BER_END,0x00,200));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("end function done\n"));
 /* if(cnt == 1) {                                                     */
 /*     EFUN_PRINTF(("Not enough points found to extrapolate BER\n")); */
 /*     return(ERR_CODE_NONE);                                         */
 /* }                                                                  */

    EFUN(merlin16_shortfin_display_ber_scan_data(sa__, rate, ber_scan_mode, &errs[0], &time[0],(uint8_t)SRDS_ABS(offset_start)));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("display done\n"));

    return(ERR_CODE_NONE);
}



err_code_t merlin16_shortfin_display_ber_scan_data (srds_access_t *sa__, USR_DOUBLE rate, uint8_t ber_scan_mode, uint32_t *total_errs, uint32_t *total_time, uint8_t max_offset) {

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

    const unsigned int HI_CONFIDENCE_ERR_CNT = 100;      /* bit errors */
    const unsigned int HI_CONFIDENCE_MIN_ERR_CNT = 20;   /* bit errors */
    const unsigned int MAX_CLIPPED_ERR_CNT = 8355840;
    const USR_DOUBLE ARTIFICIAL_BER = 0.5;  /* used along ARTIFICIAL_MARGIN(_V/_H) when not enough points to extrapolate */
    const int ARTIFICIAL_MARGIN_V = 500;    /* Used along ARTIFICIAL_BER when not enough points to extrapolate. Unit: mV. Based on the concept of max Vpp of 1 Volt */
    const int ARTIFICIAL_MARGIN_H = 1;  /* Used along ARTIFICIAL_BER when not enough points to extrapolate. Unit: UI. Based on the concept of two consecutive scrambled bits (1 UI appart) being uncorrelated */
    const int MIN_BER_TO_REPORT = -24;   /* we clip the projected BER using this number */
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
    int8_t offset[DIAG_MAX_SAMPLES];
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
    uint8_t eye_cnt;
    uint8_t hi_confidence_cnt = 0;
    int8_t first_good_ber_idx = -1;
    int8_t first_small_errcnt_idx = -1;
    int8_t first_non_clipped_errcnt_idx = -1;
    uint8_t range250;
    uint8_t intrusive;
    uint8_t ber_clipped = 0;
    uint8_t last_point_discard;
    uint8_t fit_count;
    int artificial_margin;
    int proj_case = 0;  /* this variable will be used to signal what particular extrapolation case has happened at the end (after discarding invalid points of all sorts). To avoid potential issues: NEVER RE-USE VALUES... new cases should receive brand-new integer value */
    USR_DOUBLE artificial_lber;
    char message[256] = "NO MESSAGE";
    char unit[5];

    if(!total_errs || !total_time ) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
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
        if (direction==-1) EFUN_PRINTF(("\n\n********** HORIZONTAL PROJECTION: LEFT SIDE ******************\n"));
        if (direction== 1) EFUN_PRINTF(("\n\n********** HORIZONTAL PROJECTION: RIGHT SIDE *****************\n"));
    } else {
        if (direction==-1) EFUN_PRINTF(("\n\n********** VERTICAL PROJECTION: BOTTOM ***********************\n"));
        if (direction== 1) EFUN_PRINTF(("\n\n********** VERTICAL PROJECTION: TOP **************************\n"));
    }

    /* *******************************************
        * Generate margins[]
        * Generate ber[]
        * Find first and last points for linear fit
        ******************************************* */
    i=0;
    do {
        if(heye == 1) {
            ENULL_STRCPY(unit,"mUI");
            offset[i] = (int8_t)(max_offset-i);
#ifndef STANDALONE_EVENT
            margins[i] = direction*offset[i]*1000.0/64.0;
#else
            margins[i] = info_out->margins[i];
#endif
        } else {
            ENULL_STRCPY(unit,"mV");
            offset[i] = (int8_t)(max_offset-i);
#ifndef STANDALONE_EVENT
            if(intrusive) {
                margins[i] = direction*intrusive_margins_mv[offset[i]];
            } else {
                margins[i] = direction*merlin16_shortfin_INTERNAL_ladder_setting_to_mV(sa__, (int16_t)offset[i], range250);
            }
#else
            margins[i] = info_out->margins[i];
#endif
        }
        if (total_errs[i] == 0) {
            bers[i] = 1.0/(((USR_DOUBLE)total_time[i])*0.00001*rate);
        } else {
            bers[i] = (USR_DOUBLE)total_errs[i]/(((USR_DOUBLE)total_time[i])*0.00001*rate);
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

    } while(((total_errs[i] != 0) || (total_time[i] != 0)) && (i<=max_offset));

    eye_cnt = (uint8_t) i;


    /* *******************************************
    Setting up stop_n variable.
    Check if:
        - There is only one point in measurement vector (i.e. eye_cnt = 1)
        - The very last point's measurement time was "too short"
    ******************************************* */

    i = (uint32_t)(eye_cnt - 1);
    if (i>=1) {
        if ((total_time[i] >= 0.5*total_time[i-1]) || (total_errs[i] >= HI_CONFIDENCE_MIN_ERR_CNT) ){
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
           EFUN_PRINTF(("BER @ %4.0f %s < 1e%-6.2f (%u errors in %0.2f sec)\n", margins[i], unit, log10(bers[i]), total_errs[i], ((USR_DOUBLE)total_time[i])*0.00001));
        } else if (total_errs[i] >= MAX_CLIPPED_ERR_CNT) {
           EFUN_PRINTF(("BER @ %4.0f %s > 1e%-6.2f (%u errors in %0.2f sec)\n", margins[i], unit, log10(bers[i]), total_errs[i], ((USR_DOUBLE)total_time[i])*0.00001));
        } else {
           EFUN_PRINTF(("BER @ %4.0f %s = 1e%-6.2f (%u errors in %0.2f sec)\n", margins[i], unit, log10(bers[i]), total_errs[i], ((USR_DOUBLE)total_time[i])*0.00001));
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
        ENULL_STRCPY(message,"No low-BER point measured");

        /* confidence factor of 0.96 is applied in this case to set a LOWER bound and report accordingly.
            This factor corresponds to approximately 3000 errors @95% confidence
            For reference: factors for 900, 2000, 3000, 5000, 20000 and 50000 errors are: 0.96, 0.96, 0.97, 0.99, 0.99, respectively */
        proj_ber = 0.96*log10(bers[eye_cnt-1]);
        proj_ber_aux = proj_ber;
        EFUN_PRINTF(("BER *worse* than 1e%0.2f\n", proj_ber));
        EFUN_PRINTF(("No margin @ 1e-12, 1e-15 & 1e-18\n\n\n"));
        fit_count = 1;
    }

    else {

        /* ====> Only ONE measured point. Typically when the eye is wide open.
            Artificial points will be used to make extrapolation possible */
        if (stop_n==1) {
            proj_case = 2;
            ENULL_STRCPY(message,"Not enough points (single measured point). Using artificial point");

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
                ENULL_STRCPY(message,"Normal case");
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
                    ENULL_STRCPY(message,"Not enough points. Using first measured point for conservative estimation");
                    fit_count = 2;
                    /* Compute covariances and means... but only for two points: first and last */
                    Exy = ((margins[stop_n-1]*lbers[stop_n-1] + margins[first_non_clipped_errcnt_idx]*lbers[first_non_clipped_errcnt_idx])/2.0);
                    Eyy = ((lbers[stop_n-1]*lbers[stop_n-1] + lbers[first_non_clipped_errcnt_idx]*lbers[first_non_clipped_errcnt_idx])/2.0);
                    Exx = ((margins[stop_n-1]*margins[stop_n-1] + margins[first_non_clipped_errcnt_idx]*margins[first_non_clipped_errcnt_idx])/2.0);
                    Ey  = ((lbers[stop_n-1] + lbers[first_non_clipped_errcnt_idx])/2.0);
                    Ex  = ((margins[stop_n-1] + margins[first_non_clipped_errcnt_idx])/2.0);
                } else {
                    proj_case = 5;
                    ENULL_STRCPY(message,"Not enough points (cannot use non-clipped ErrorCount point). Using artificial point");
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

            EFUN_PRINTF(("BER(extrapolated) < 1e%0.2f\n", proj_ber));
            EFUN_PRINTF(("Margin @ 1e-12    > %0.2f %s\n", (proj_ber < -12)? SRDS_ABS(proj_margin_12) : 0, unit));
            EFUN_PRINTF(("Margin @ 1e-15    > %0.2f %s\n", (proj_ber < -15)? SRDS_ABS(proj_margin_15) : 0, unit));
            EFUN_PRINTF(("Margin @ 1e-18    > %0.2f %s\n", (proj_ber < -18)? SRDS_ABS(proj_margin_18) : 0, unit));

        /* Extrapolated results, HIGH confidence */
        } else {

            if (ber_clipped == 1) {
                EFUN_PRINTF(("BER(extrapolated) < 1e%0.2f\n", proj_ber));
            } else {
                EFUN_PRINTF(("BER(extrapolated) = 1e%0.2f\n", proj_ber));
            }
            EFUN_PRINTF(("Margin @ 1e-12    = %0.2f %s\n", (proj_ber < -12)? SRDS_ABS(proj_margin_12) : 0, unit));
            EFUN_PRINTF(("Margin @ 1e-15    = %0.2f %s\n", (proj_ber < -15)? SRDS_ABS(proj_margin_15) : 0, unit));
            EFUN_PRINTF(("Margin @ 1e-18    = %0.2f %s\n", (proj_ber < -18)? SRDS_ABS(proj_margin_18) : 0, unit));
        }

        EFUN_PRINTF(("\n\n"));

        /* Print non-monotonic outliers */
        if (n_mono != 0) {
            EFUN_PRINTF(("Detected non-monotonicity at { "));
            for (loop_index = start_n; loop_index < stop_n; loop_index++) {
                if (mono_flags[loop_index] == 1) {
                    EFUN_PRINTF(("%0.2f ", margins[loop_index]));
                }
            }
            EFUN_PRINTF(("} %s\n\n\n",unit));
        }

    }
    /* *******************************************
    ***********  EXTRAPOLATION (END) *************
    ********************************************* */



    /* SUMMARY (for debugging purposes */
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("\t=====> DEBUG INFO (start)\n\n"));
    if (DIAG_VERBOSE > 2) {
        EFUN_PRINTF((" loop   Margin      total_errors   time(sec)  logBER       lber"));
        for (loop_index=0; loop_index < stop_n+last_point_discard; loop_index++) {
            EFUN_PRINTF(("\n%5d %11.0f %14d %10.3f %8.2f %10.3f", loop_index, margins[loop_index], total_errs[loop_index], ((USR_DOUBLE)total_time[loop_index])*0.00001, log10(bers[loop_index]), lbers[loop_index]));
        }
        EFUN_PRINTF(("\n\n"));
    }
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("Max Offset = %d\n",max_offset));
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("ber_clipped: %d, Projected BER (proj_ber_aux) = %.2f\n", ber_clipped, proj_ber_aux));
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("first good ber idx at %d, ber = 1e%f\n", first_good_ber_idx, ((first_good_ber_idx>=0) ? log10(bers[first_good_ber_idx]) : 0.0)));
    {
        char aux_str[20];
        USR_SPRINTF(aux_str, "%d", total_errs[first_small_errcnt_idx]);
        if (DIAG_VERBOSE > 2) EFUN_PRINTF(("first small errcnt idx at %d, errors = %s\n", first_small_errcnt_idx, ((first_small_errcnt_idx>=0) ? aux_str : "-1")));
    }
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("last point discarded?: %d, low_confidence: %d, first_non_clipped_errcnt_idx: %d, start_n: %d, stop_n: %d, eye_cnt: %d, n_mono: %d, first_good_ber_idx = %d, first_small_errcnt_idx = %d, fit_count: %d, delta_n: %d\n",
                last_point_discard, low_confidence, first_non_clipped_errcnt_idx, start_n, stop_n, eye_cnt, n_mono, first_good_ber_idx, first_small_errcnt_idx, fit_count, delta_n));
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("Exy=%.2f, Eyy=%.4f, Exx=%.2f, Ey=%.4f, Ex=%.2f, alpha=%.4f, beta=%.4f, alpha2=%.3f, sq_r=%.3f, sq_err1=%g, sq_err2=%g, gauss_noise=%.3f\n", Exy,Eyy,Exx,Ey,Ex,alpha,beta,alpha2,sq_r,sq_err1,sq_err2,gauss_noise));
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("%s\n", message));
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("proj_case %d\n",proj_case));
    if (DIAG_VERBOSE > 2) EFUN_PRINTF(("\n\t=====> DEBUG INFO (end)\n\n"));
    EFUN_PRINTF(("\n\n\n"));


#else
    UNUSED(rate);
    UNUSED(ber_scan_mode);
    UNUSED(max_offset);

    EFUN_PRINTF(("This function needs SERDES_API_FLOATING_POINT define to operate \n"));

    if(!total_errs || !total_time ) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
#endif
    return(ERR_CODE_NONE);
}

/*****************************/
/*  Display Lane/Core State  */
/*****************************/

err_code_t merlin16_shortfin_display_lane_state_hdr(void) {
  EFUN_PRINTF(("LN (CDRxN  , UC_CFG,UC_STS, RST, STP) "));
  EFUN_PRINTF(("SD LCK RXPPM "));
  EFUN_PRINTF(("CLK90 CLKP1 "));
  EFUN_PRINTF(("PF(M,L)  "));
  EFUN_PRINTF(("VGA "));
  EFUN_PRINTF(("DCO P1mV "));
  EFUN_PRINTF((" DFE(1,2,3,4,5,dcd1,dcd2)   SLICER(ze,zo,pe,po,me,mo) "));
  EFUN_PRINTF(("TXPPM TXEQ(n1,m,p1,p2)    EYE(L,R,U,D)  "));
  EFUN_PRINTF(("SOC(p,n) "));
  EFUN_PRINTF(("LINK_TIME   BER"));
  EFUN_PRINTF(("\n"));
  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_display_lane_state_legend(void) {
  EFUN_PRINTF(("\n"));
  EFUN_PRINTF(("**********************************************************************************************\n"));
  EFUN_PRINTF(("****                Legend of Entries in display_lane_state()                             ****\n"));
  EFUN_PRINTF(("**********************************************************************************************\n"));
  EFUN_PRINTF(("LN        : Lane index within IP core\n"));
  EFUN_PRINTF(("CDRxN     : CDR type x OSR ratio [xT-OS3p3; x7-OS7p5; x9-OSx8p25; xA-OSx10]\n"));
  EFUN_PRINTF(("UC_CFG    : Micro lane configuration variable\n"));
  EFUN_PRINTF(("UC_STS    : Micro lane status variable\n"));
#if defined(SERDES_TX_RS_SEPARATE)
  EFUN_PRINTF(("RST       : TX and RX Reset State{reset_active, reset_occured, reset_held}\n"));
#else
  EFUN_PRINTF(("RST       : Reset State{reset_active, reset_occured, reset_held}\n"));
#endif
  EFUN_PRINTF(("STP       : uC Stopped State\n"));
  EFUN_PRINTF(("SD        : Signal Detect\n"));
  EFUN_PRINTF(("LCK       : pmd_rx_lock\n"));
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
  EFUN_PRINTF(("SLICER(ze,zo,pe,po,me,mo): Slicer calibration control codes\n"));
  EFUN_PRINTF(("TXPPM     : Frequency offset of local reference clock with respect to TX data in ppm\n"));
  EFUN_PRINTF(("TXEQ(n1,m,p1,p2) : TX equalization FIR tap weights in units of 1Vpp/60 units\n"));
  EFUN_PRINTF(("SOC(p,n)  : Signal Detect Offset Calibration\n"));
  EFUN_PRINTF(("EYE(L,R,U,D) : Eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV\n"));
  EFUN_PRINTF(("LINK_TIME : Link time in milliseconds\n"));
  EFUN_PRINTF(("BER : Bit Error Rate calculated based on 100ms test time; displayed only if prbs_chk_en=1.\n"));
  EFUN_PRINTF(("**********************************************************************************************\n"));
  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_display_lane_state(srds_access_t *sa__) {
  err_code_t err_code = merlin16_shortfin_INTERNAL_display_lane_state_no_newline(sa__);
  EFUN_PRINTF(("\n"));
  return (err_code);
}

err_code_t merlin16_shortfin_display_core_state_hdr(void) {
    char core_type[20] = "merlin16_shortfin";
    EFUN_PRINTF(("SerDes type = %s\n",core_type));

  EFUN_PRINTF(("CORE RST_ST  PLL_PWDN  UC_ATV   COM_CLK   UCODE_VER  API_VER  AFE_VER   LIVE_TEMP   AVG_TMON   RESCAL   VCO_RATE  ANA_VCO_RANGE  PLL_DIV  PLL_LOCK"));
  EFUN_PRINTF(("\n"));
  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_display_core_state_line(srds_access_t *sa__) {
  err_code_t err_code = merlin16_shortfin_INTERNAL_display_core_state_no_newline(sa__);
  EFUN_PRINTF(("\n"));
  return (err_code);
}

err_code_t merlin16_shortfin_display_core_state_legend(void) {
  EFUN_PRINTF(("\n"));
  EFUN_PRINTF(("**************************************************************************************************************\n"));
  EFUN_PRINTF(("****                          Legend of Entries in display_core_state()                                   ****\n"));
  EFUN_PRINTF(("**************************************************************************************************************\n"));
  EFUN_PRINTF(("*  RST_ST           : Core DP Reset State{reset_active, reset_occured, reset_held}, Core uC Status byte(hex) *\n"));
  EFUN_PRINTF(("*  PLL_PWDN         : PLL Powerdown Control Bit (active high)                                                *\n"));
  EFUN_PRINTF(("*  UC_ATV           : UC Active bit                                                                          *\n"));
  EFUN_PRINTF(("*  COM_CLK          : COM Clock frequency in MHz                                                             *\n"));
  EFUN_PRINTF(("*  UCODE_VER        : Microcode Version [majorversion_minorversion]                                          *\n"));
  EFUN_PRINTF(("*  API_VER          : API Version                                                                            *\n"));
  EFUN_PRINTF(("*  AFE_VER          : AFE Hardware Version                                                                   *\n"));
  EFUN_PRINTF(("*  LIVE_TEMP        : Live Die temperature in Celsius                                                        *\n"));
  EFUN_PRINTF(("*  AVG_TMON         : uC Temp_idx, Average temperature in Celsius                                            *\n"));
  EFUN_PRINTF(("*  RESCAL           : Analog Resistor Calibration value                                                      *\n"));
  EFUN_PRINTF(("*  VCO_RATE         : uC VCO Rate in GHz (approximate)                                                       *\n"));
  EFUN_PRINTF(("*  ANA_VCO_RANGE    : Analog VCO Range                                                                       *\n"));
  EFUN_PRINTF(("*  PLL_Lock         : PLL Lock                                                                               *\n"));

  EFUN_PRINTF(("**************************************************************************************************************\n"));
  return (ERR_CODE_NONE);
}

/**********************************************/
/*  Display Lane/Core Config and Debug Status */
/**********************************************/
err_code_t merlin16_shortfin_display_core_config(srds_access_t *sa__) {
    EFUN_PRINTF(("\n\n***********************************\n"  ));
    EFUN_PRINTF((    "**** SERDES CORE CONFIGURATION ****\n"  ));
    EFUN_PRINTF((    "***********************************\n\n"));
    {
        struct   merlin16_shortfin_uc_core_config_st core_cfg;
        ENULL_MEMSET(&core_cfg, 0, sizeof(core_cfg));
            EFUN(merlin16_shortfin_get_uc_core_config(sa__, &core_cfg));
        {
            uint16_t  vco_mhz = (uint16_t)core_cfg.vco_rate_in_Mhz;
    EFUN_PRINTF((    "uC Config VCO Rate   = %d (~%d.%03dGHz)\n"                  , core_cfg.field.vco_rate
                                                                                  , (vco_mhz / 1000)
                                                                                  , (vco_mhz % 1000)                ));
    EFUN_PRINTF((    "Core Config from PCS = %d\n\n"                              , core_cfg.field.core_cfg_from_pcs));
        }
    }
#ifdef rdc_tx_lane_addr_4
    ESTM_PRINTF((    "Tx Lane Addr 4       = %d\n"                                , rdc_tx_lane_addr_4()));
    ESTM_PRINTF((    "Rx Lane Addr 4       = %d\n"                                , rdc_rx_lane_addr_4()));
#endif
#ifdef rdc_tx_lane_addr_5
    ESTM_PRINTF((    "Tx Lane Addr 5       = %d\n"                                , rdc_tx_lane_addr_5()));
    ESTM_PRINTF((    "Rx Lane Addr 5       = %d\n"                                , rdc_rx_lane_addr_5()));
#endif
#ifdef rdc_tx_lane_addr_6
    ESTM_PRINTF((    "Tx Lane Addr 6       = %d\n"                                , rdc_tx_lane_addr_6()));
    ESTM_PRINTF((    "Rx Lane Addr 6       = %d\n"                                , rdc_rx_lane_addr_6()));
#endif
#ifdef rdc_tx_lane_addr_7
    ESTM_PRINTF((    "Tx Lane Addr 7       = %d\n"                                , rdc_tx_lane_addr_7()));
    ESTM_PRINTF((    "Rx Lane Addr 7       = %d\n"                                , rdc_rx_lane_addr_7()));
#endif
    return(ERR_CODE_NONE);
}


err_code_t merlin16_shortfin_display_lane_config(srds_access_t *sa__) {
    struct merlin16_shortfin_uc_lane_config_st lane_cfg;

    ENULL_MEMSET(&lane_cfg, 0, sizeof(lane_cfg));

    EFUN_PRINTF(("\n\n*************************************\n"                                          ));
    ESTM_PRINTF((   "**** SERDES LANE %d CONFIGURATION ****\n"  , merlin16_shortfin_get_lane(sa__)                 ));
    EFUN_PRINTF((    "*************************************\n\n"                                        ));
    EFUN(                                                         merlin16_shortfin_get_uc_lane_cfg(sa__, &lane_cfg));
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


err_code_t merlin16_shortfin_display_core_state(srds_access_t *sa__) {
    EFUN(merlin16_shortfin_display_core_state_hdr());
    EFUN(merlin16_shortfin_display_core_state_line(sa__));
    EFUN(merlin16_shortfin_display_core_state_legend());
    return ERR_CODE_NONE;
}


err_code_t merlin16_shortfin_display_lane_debug_status(srds_access_t *sa__) {
    /* startup */
    struct merlin16_shortfin_usr_ctrl_disable_functions_st     ds;
    struct merlin16_shortfin_usr_ctrl_disable_dfe_functions_st dsd;
    /* steady state */
    struct merlin16_shortfin_usr_ctrl_disable_functions_st     dss;
    struct merlin16_shortfin_usr_ctrl_disable_dfe_functions_st dssd;
    uint8_t core,lane;

    ENULL_MEMSET(&ds  , 0, sizeof(ds  ));
    ENULL_MEMSET(&dsd , 0, sizeof(dsd ));
    ENULL_MEMSET(&dss , 0, sizeof(dss ));
    ENULL_MEMSET(&dssd, 0, sizeof(dssd));
    ESTM(core=merlin16_shortfin_get_core(sa__));
    ESTM(lane=merlin16_shortfin_get_lane(sa__));
    EFUN_PRINTF(("\n\n*******************************************\n"                                              ));
    ESTM_PRINTF((   "**** SERDES CORE %d LANE %d DEBUG STATUS ****\n", core, lane                                 ));
    EFUN_PRINTF((    "*******************************************\n\n"                                            ));
    ESTM_PRINTF((    "Restart Count                                = %d\n"  , rdv_usr_sts_restart_counter()       ));
    ESTM_PRINTF((    "Reset Count                                  = %d\n"  , rdv_usr_sts_reset_counter()         ));
    ESTM_PRINTF((    "PMD Lock Count                               = %d\n\n", rdv_usr_sts_pmd_lock_counter()      ));
    EFUN(merlin16_shortfin_get_usr_ctrl_disable_startup(sa__, &ds));
    EFUN_PRINTF((    "Disable Startup PF Adaptation                = %d\n"  , ds.field.pf_adaptation              ));
    EFUN_PRINTF((    "Disable Startup DC Adaptation                = %d\n"  , ds.field.dc_adaptation              ));
    EFUN_PRINTF((    "Disable Startup Slicer Offset Tuning         = %d\n"  , ds.field.slicer_offset_tuning       ));
    EFUN_PRINTF((    "Disable Startup Clk90 offset Adaptation      = %d\n"  , ds.field.clk90_offset_adaptation    ));
    EFUN_PRINTF((    "Disable Startup P1 level Tuning              = %d\n"  , ds.field.p1_level_tuning            ));
    EFUN_PRINTF((    "Disable Startup Eye Adaptation               = %d\n"  , ds.field.eye_adaptation             ));
    EFUN_PRINTF((    "Disable Startup All Adaptation               = %d\n\n", ds.field.all_adaptation             ));
    EFUN(                                                            merlin16_shortfin_get_usr_ctrl_disable_startup_dfe(sa__, &dsd));
    EFUN_PRINTF((    "Disable Startup DFE Tap1 Adaptation          = %d\n"  , dsd.field.dfe_tap1_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap2 Adaptation          = %d\n"  , dsd.field.dfe_tap2_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap3 Adaptation          = %d\n"  , dsd.field.dfe_tap3_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap4 Adaptation          = %d\n"  , dsd.field.dfe_tap4_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap5 Adaptation          = %d\n"  , dsd.field.dfe_tap5_adaptation       ));
    EFUN_PRINTF((    "Disable Startup DFE Tap1 DCD                 = %d\n"  , dsd.field.dfe_tap1_dcd              ));
    EFUN_PRINTF((    "Disable Startup DFE Tap2 DCD                 = %d\n\n", dsd.field.dfe_tap2_dcd              ));
    EFUN(                                                           merlin16_shortfin_get_usr_ctrl_disable_steady_state(sa__, &dss));
    EFUN_PRINTF((    "Disable Steady State PF Adaptation           = %d\n"  , dss.field.pf_adaptation             ));
    EFUN_PRINTF((    "Disable Steady State DC Adaptation           = %d\n"  , dss.field.dc_adaptation             ));
    EFUN_PRINTF((    "Disable Steady State Slicer Offset Tuning    = %d\n"  , dss.field.slicer_offset_tuning      ));
    EFUN_PRINTF((    "Disable Steady State Clk90 offset Adaptation = %d\n"  , dss.field.clk90_offset_adaptation   ));
    EFUN_PRINTF((    "Disable Steady State P1 level Tuning         = %d\n"  , dss.field.p1_level_tuning           ));
    EFUN_PRINTF((    "Disable Steady State Eye Adaptation          = %d\n"  , dss.field.eye_adaptation            ));
    EFUN_PRINTF((    "Disable Steady State All Adaptation          = %d\n\n", dss.field.all_adaptation            ));
    EFUN(                                                      merlin16_shortfin_get_usr_ctrl_disable_steady_state_dfe(sa__, &dssd));
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

static err_code_t _display_ber_scan_data(srds_access_t *sa__, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    uint8_t i,prbs_byte,prbs_multi,time_byte,time_multi;
    uint16_t sts,dataword;
    int16_t offset_start;
    uint8_t cnt;
    uint32_t errors,timer_values;
    int rate,direction;
    uint8_t range250;
    merlin16_shortfin_osr_mode_st osr_mode;
    struct merlin16_shortfin_uc_core_config_st core_config;

    ENULL_MEMSET(&core_config,0,sizeof(core_config));
    ENULL_MEMSET(&osr_mode,0,sizeof(osr_mode));

    EFUN(merlin16_shortfin_get_uc_core_config(sa__, &core_config));
    EFUN(merlin16_shortfin_INTERNAL_get_osr_mode(sa__, &osr_mode));

#ifdef wr_ams_rx_rxclk_div2p5
    if((osr_mode.rx > MERLIN16_SHORTFIN_OSX4) && (osr_mode.rx != MERLIN16_SHORTFIN_OSX2P5)) {
#else
    if(osr_mode.rx > MERLIN16_SHORTFIN_OSX4) {
#endif
        EFUN_PRINTF(("ERROR DIAG display_ber_data: osr mode too high\n"));
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    if (osr_mode.rx > MERLIN16_SHORTFIN_OSX4) { /* BER supported for only MERLIN16_SHORTFIN_OSX2P5 mode when osr_mode > osx4 */
        rate = (int)(core_config.vco_rate_in_Mhz/2.5);
    }
    else {
        rate = core_config.vco_rate_in_Mhz/(1<<osr_mode.rx);
    }
    direction = (ber_scan_mode & DIAG_BER_NEG) ? -1 : 1 ;
    range250 = (ber_scan_mode & DIAG_BER_P1_NARROW) ? 0 : 1;

    EFUN_PRINTF(("\n****  SERDES BER DATA    ****\n"));
    EFUN_PRINTF(("BER MODE = %x %d %d\n",ber_scan_mode,timer_control,max_error_control));
    EFUN_PRINTF(("DATA RATE = %d Mhz\n",rate));
    /* start UC acquisition */
    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("start begin\n"));
    EFUN(merlin16_shortfin_start_ber_scan_test(sa__, ber_scan_mode, timer_control, max_error_control));
    ESTM(offset_start = (int16_t)rd_uc_dsc_data());
    if(ber_scan_mode & DIAG_BER_HORZ) {
        EFUN_PRINTF(("STARTING OFFSET = %d : %d mUI\n",offset_start,(offset_start*1000)>>6));
    } else {
        EFUN_PRINTF(("STARTING OFFSET = %d : %d mV\n",offset_start,merlin16_shortfin_INTERNAL_ladder_setting_to_mV(sa__, (int8_t)offset_start, range250)));
    }
    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("start done\n"));


    EFUN_PRINTF(("Waiting for measurement time approx %d seconds",timer_control+(timer_control>>1)));
    EFUN(merlin16_shortfin_INTERNAL_poll_diag_done(sa__, &sts,(uint32_t)(timer_control*2000)));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("delay done\n"));

    /* Check for completion read ln.diag_status byte?*/
    ESTM(sts = rdv_usr_diag_status());
    if((sts & 0x8000) == 0) {
        return(merlin16_shortfin_error(sa__, ERR_CODE_DATA_NOTAVAIL));
    }
    cnt = (uint8_t)((sts & 0x00FF)/3);
    for(i=0;i < cnt;i++) {
        /* Read 2 bytes of data */
        EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_READ_DIAG_DATA_WORD, 0, 200));
        ESTM(dataword = rd_uc_dsc_data());           /* LSB contains 2 -4bit nibbles */
        time_byte = (uint8_t)(dataword>>8);          /* MSB is time byte */
        prbs_multi = (uint8_t)dataword & 0x0F;       /* split nibbles */
        time_multi = (uint8_t)dataword>>4;
        /* Read 1 bytes of data */
        EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_READ_DIAG_DATA_BYTE, 0, 200));
        ESTM(prbs_byte = (uint8_t)rd_uc_dsc_data());
        errors = merlin16_shortfin_INTERNAL_float12_to_uint32(prbs_byte,prbs_multi); /* convert 12bits to uint32 */
        timer_values = (merlin16_shortfin_INTERNAL_float12_to_uint32(time_byte,time_multi)<<3);
        if(DIAG_VERBOSE < 5) {
            if (!(i % 4))  {
                EFUN_PRINTF(("\n"));
            }
            if(ber_scan_mode & DIAG_BER_HORZ) {
                EFUN_PRINTF(("%d %d %d ",direction*(((SRDS_ABS(offset_start)-i)*1000)>>6),errors,timer_values));
            } else {
                EFUN_PRINTF(("%d %d %d ",direction*merlin16_shortfin_INTERNAL_ladder_setting_to_mV(sa__, (int8_t)(SRDS_ABS(offset_start)-i), range250),errors,timer_values));
            }

        } else {
            EFUN_PRINTF(("BER Errors=%d (%02x<<%d): Time=%d (%02x<<%d)\n",errors,prbs_byte,prbs_multi,timer_values,time_byte,time_multi<<3));
        }
        /*if(timer_values == 0 && errors == 0) break;*/
    }
    EFUN_PRINTF(("\n"));
    EFUN(merlin16_shortfin_pmd_uc_cmd(sa__, CMD_CAPTURE_BER_END,0x00,2000));

  return(ERR_CODE_NONE);
}

/*******************************************/
/*  Diagnostic Functions Required for SDK  */
/*******************************************/

/* Required Diagnostic Functions */
err_code_t merlin16_shortfin_display_diag_data(srds_access_t *sa__, uint16_t  diag_level) {
    uint8_t rx_lock, micro_stop, core, lane;
    uint32_t api_version;
    EFUN_PRINTF(("\n**** SERDES DISPLAY DIAG DATA ****\n"));
    ESTM_PRINTF(("Rev ID Letter  = %02X\n", rdc_revid_rev_letter()));
    ESTM_PRINTF(("Rev ID Process = %02X\n", rdc_revid_process()));
    ESTM_PRINTF(("Rev ID Model   = %02X\n", rdc_revid_model()));
    ESTM_PRINTF(("Rev ID Model   = %02X\n", rdc_revid2()));
    ESTM_PRINTF(("Rev ID # Lanes = %d\n"  ,rdc_revid_multiplicity()));
    ESTM_PRINTF(("Core = %d; LANE = %d\n",merlin16_shortfin_get_core(sa__),merlin16_shortfin_get_lane(sa__)));
    EFUN(merlin16_shortfin_version(sa__, &api_version));
    EFUN_PRINTF(("SERDES API Version         = %06X\n",api_version));
    ESTM_PRINTF(("Common Ucode Version       = %04X", rdcv_common_ucode_version()));
    ESTM_PRINTF(("_%02X\n", rdcv_common_ucode_minor_version()));
    ESTM_PRINTF(("AFE Hardware Version       = 0x%X\n\n", rdcv_afe_hardware_version()));

    /* stop micro so all accesses are consistent */
    ESTM(rx_lock = rd_pmd_rx_lock());
    {
        err_code_t err_code=ERR_CODE_NONE;
        micro_stop = merlin16_shortfin_INTERNAL_stop_micro(sa__,rx_lock,&err_code);
        if(err_code) USR_PRINTF(("Unable to stop microcontroller,  following data is suspect\n"));
    }

    if(diag_level & SRDS_DIAG_CORE) {
        EFUN(merlin16_shortfin_display_core_state_hdr());
        EFUN(merlin16_shortfin_display_core_state_line(sa__));
        EFUN_PRINTF(("\n"));
    }

    EFUN(merlin16_shortfin_display_lane_state_hdr());
    EFUN(merlin16_shortfin_display_lane_state(sa__));


    if(diag_level & SRDS_DIAG_TIMESTAMP) {
        EFUN(merlin16_shortfin_test_uc_timestamp(sa__));
    }
    if(diag_level & SRDS_DIAG_EVENT_SAFE) {
        merlin16_shortfin_INTERNAL_event_log_dump_state_t state;
        uint8_t micro_num = 0;
        {
            state.index = 0;
            state.line_start_index = 0;
            EFUN(merlin16_shortfin_INTERNAL_read_event_log_with_callback(sa__, micro_num, 1, &state, merlin16_shortfin_INTERNAL_event_log_dump_callback));
            EFUN(merlin16_shortfin_INTERNAL_event_log_dump_callback(&state, 0, 0));
        }
    }
    if(diag_level & SRDS_DIAG_EVENT) {
        EFUN(merlin16_shortfin_read_event_log(sa__));
    }
    /* currently REG_CORE and REG_LANE dump same data. */
    core = merlin16_shortfin_get_core(sa__);
    lane = merlin16_shortfin_get_lane(sa__);

    if((diag_level & SRDS_DIAG_REG_CORE)
       || (diag_level & SRDS_DIAG_REG_LANE)) {
        EFUN(merlin16_shortfin_reg_dump(sa__, core, lane, NULL));
    }
    if(diag_level & SRDS_DIAG_UC_CORE) {
        EFUN(merlin16_shortfin_uc_core_var_dump(sa__, core, NULL));

    }
    if(diag_level & SRDS_DIAG_UC_LANE) {

        EFUN(merlin16_shortfin_uc_lane_var_dump(sa__, core, lane, NULL));
    }
    if(diag_level & SRDS_DIAG_EYE) {
        EFUN(merlin16_shortfin_display_eye_scan(sa__));
    }
    if(diag_level & SRDS_DIAG_LANE_DEBUG) {
        EFUN(merlin16_shortfin_display_lane_debug_status(sa__));
    }
    if(diag_level & SRDS_DIAG_BER_VERT) {
        /* display ber projections for all channels */
        uint8_t ber_mode = DIAG_BER_VERT | DIAG_BER_POS;
        uint8_t timer_control = 23;       /* 30 seconds */
        uint8_t err_threshold = 100 / 16; /* 100 errors */
        EFUN(_display_ber_scan_data(sa__, ber_mode, timer_control,  err_threshold));
        ber_mode = DIAG_BER_VERT | DIAG_BER_NEG;
        EFUN(_display_ber_scan_data(sa__, ber_mode, timer_control,  err_threshold));
    }
    if(diag_level & SRDS_DIAG_BER_HORZ) {
        /* display ber projections for all channels */
        uint8_t ber_mode = DIAG_BER_HORZ | DIAG_BER_POS;
        uint8_t timer_control = 23;       /* 30 seconds */
        uint8_t err_threshold = 100 / 16; /* 100 errors */
        EFUN(_display_ber_scan_data(sa__, ber_mode, timer_control,  err_threshold));
        ber_mode = DIAG_BER_HORZ | DIAG_BER_NEG;
        EFUN(_display_ber_scan_data(sa__, ber_mode, timer_control,  err_threshold));
    }

    if(diag_level & SRDS_DIAG_INFO_TABLE) {
        EFUN(merlin16_shortfin_INTERNAL_display_info_table(sa__));
    }

    /* re enable micro */
    if (!micro_stop) {
        EFUN(merlin16_shortfin_stop_rx_adaptation(sa__, 0));
    }
    EFUN_PRINTF(("\n**** SERDES DISPLAY DIAG DATA END ****\n"));
    EFUN_PRINTF(("\n\n"));
    return (ERR_CODE_NONE);

}

typedef struct {
    uint16_t addr;
    uint8_t  line_byte_counter;
} merlin16_shortfin_diag_access_callback_state_t;

static err_code_t merlin16_shortfin_diag_access_read_byte_callback(void *arg, uint8_t byte_count, uint16_t data) {
    merlin16_shortfin_diag_access_callback_state_t * const state_ptr = (merlin16_shortfin_diag_access_callback_state_t *)arg;
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

static err_code_t merlin16_shortfin_diag_access_read_word_callback(void *arg, uint8_t byte_count, uint16_t data) {
    merlin16_shortfin_diag_access_callback_state_t * const state_ptr = (merlin16_shortfin_diag_access_callback_state_t *)arg;
    UNUSED(byte_count);

    if (state_ptr->line_byte_counter == 0) {
        USR_PRINTF(("\n%04x  ", state_ptr->addr));
    }
    USR_PRINTF(("%04x ", data));
    state_ptr->addr = (uint16_t)(state_ptr->addr + 2);
    state_ptr->line_byte_counter = (uint8_t)((state_ptr->line_byte_counter+2) % 16);
    return (ERR_CODE_NONE);
}

/* Required Diagnostic Functions */
err_code_t merlin16_shortfin_diag_access(srds_access_t *sa__, enum srds_diag_access_enum type, uint16_t addr, uint16_t data, uint16_t param) {
    srds_info_t * merlin16_shortfin_info_ptr = merlin16_shortfin_INTERNAL_get_merlin16_shortfin_info_ptr_with_check(sa__);
    merlin16_shortfin_diag_access_callback_state_t state;

    CHECK_AND_RETURN_IF_NULL(merlin16_shortfin_info_ptr, ERR_CODE_INVALID_INFO_TABLE_ADDR);
    EFUN(merlin16_shortfin_INTERNAL_verify_merlin16_shortfin_info(merlin16_shortfin_info_ptr, sa__));
    state.addr = addr;
    state.line_byte_counter = 0;


    switch(type) {
    case SRDS_REG_READ:      {
                             uint16_t tmp = 0;
                             if(data > 1) {
                                 uint16_t i;
                                 EFUN_PRINTF(("\n****  SERDES BLK REGISTER READ    ****"));
                                 for (i = 0; i < data; i++) {
                                     if (!(i % 16))  {
                                         ESTM_PRINTF(("\n%04x ",i+addr));
                                     }
                                     EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, (uint16_t)(i+addr),&tmp));
                                     ESTM_PRINTF(("%04x ", tmp));
                                 }
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr,&tmp));
                                 EFUN_PRINTF(("Register Read: x%04x = x%04x\n",addr,tmp));
                             }
                             } break;
    case SRDS_REG_RMW:       {
                             uint16_t tmp = 0, tmp2 = 0;
                             EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr,&tmp));
                             tmp2 = (uint16_t)((tmp & ~param) | (param & data));
                             EFUN(merlin16_shortfin_pmd_wr_reg(sa__, addr,tmp2));
                             EFUN_PRINTF(("Register RMW: x%04x = x%04x -> x%04x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_CORE_RAM_READ_BYTE: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK CORE RAM READ BYTE   ****"));
                                 EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           addr + merlin16_shortfin_info_ptr->core_var_ram_base,
                                                                           data,
                                                                           0,
                                                                           data,
                                                                           &state,
                                                                           merlin16_shortfin_diag_access_read_byte_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Core RAM Read byte: x%04x = x%02x\n",(uint8_t)addr,merlin16_shortfin_rdbc_uc_var(sa__, __ERR, (uint8_t)addr)));
                             }
                             } break;
    case SRDS_LANE_RAM_READ_BYTE: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK LANE RAM READ BYTE   ****"));
                                 EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__, (addr + merlin16_shortfin_info_ptr->lane_var_ram_base + (merlin16_shortfin_get_physical_lane(sa__) * merlin16_shortfin_info_ptr->lane_var_ram_size)),
                                                                           data,
                                                                           0,
                                                                           data,
                                                                           &state,
                                                                           merlin16_shortfin_diag_access_read_byte_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Lane RAM Read byte: x%04x = x%02x\n",addr,merlin16_shortfin_rdbl_uc_var(sa__, __ERR,addr)));
                             }
                             } break;
    case SRDS_CORE_RAM_READ_WORD: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK CORE RAM READ WORD   ****"));
                                 EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           addr + merlin16_shortfin_info_ptr->core_var_ram_base,
                                                                           (uint16_t)(((data+1)>>1)<<1), /* Round up to nearest word count */
                                                                           0,
                                                                           (uint16_t)(((data+1)>>1)<<1), /* Round up to nearest word count */
                                                                           &state,
                                                                           merlin16_shortfin_diag_access_read_word_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Core RAM Read word: x%04x = x%04x\n",addr,merlin16_shortfin_rdwc_uc_var(sa__, __ERR, (uint8_t)addr)));
                             }
                             } break;
    case SRDS_LANE_RAM_READ_WORD: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK LANE RAM READ WORD   ****"));
                                 EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__, (addr + merlin16_shortfin_info_ptr->lane_var_ram_base + (merlin16_shortfin_get_physical_lane(sa__) * merlin16_shortfin_info_ptr->lane_var_ram_size)),
                                                                           (uint16_t)(((data+1)>>1)<<1), /* Round up to nearest word count */
                                                                           0,
                                                                           (uint16_t)(((data+1)>>1)<<1), /* Round up to nearest word count */
                                                                           &state,
                                                                           merlin16_shortfin_diag_access_read_word_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Lane RAM Read word: x%04x = x%04x\n",addr,merlin16_shortfin_rdwl_uc_var(sa__, __ERR, addr)));
                             }
                             } break;
    case SRDS_CORE_RAM_RMW_BYTE:  {
                             uint8_t tmp, tmp2;
                             ESTM(tmp = merlin16_shortfin_rdbc_uc_var(sa__, __ERR, (uint8_t)addr));
                             tmp2 = (uint8_t)((tmp & (uint8_t)~param) | ((uint8_t)param & data));
                             EFUN(merlin16_shortfin_wrbc_uc_var(sa__, (uint8_t)addr,tmp2));
                             EFUN_PRINTF(("Core RAM RMW byte: x%04x = x%02x -> x%02x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_LANE_RAM_RMW_BYTE:  {
                             uint8_t tmp, tmp2;
                             ESTM(tmp = merlin16_shortfin_rdbl_uc_var(sa__, __ERR,addr));
                             tmp2 = (uint8_t)((tmp & (uint8_t)~param) | ((uint8_t)param & data));
                             EFUN(merlin16_shortfin_wrbl_uc_var(sa__, addr,tmp2));
                             EFUN_PRINTF(("Lane RAM RMW byte: x%04x = x%02x -> x%02x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_CORE_RAM_RMW_WORD:  {
                             uint16_t tmp, tmp2;
                             ESTM(tmp = merlin16_shortfin_rdwc_uc_var(sa__, __ERR, (uint8_t)addr));
                             tmp2 = (uint16_t)((tmp & ~param) | (param & data));
                             EFUN(merlin16_shortfin_wrwc_uc_var(sa__, (uint8_t)addr,tmp2));
                             EFUN_PRINTF(("Core RAM RMW word: x%04x = x%04x -> x%04x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_LANE_RAM_RMW_WORD:  {
                             uint16_t tmp, tmp2;
                             ESTM(tmp = merlin16_shortfin_rdwl_uc_var(sa__, __ERR,addr));
                             tmp2 = (uint16_t)((tmp & ~param) | (param & data));
                             EFUN(merlin16_shortfin_wrwl_uc_var(sa__, addr,tmp2));
                             EFUN_PRINTF(("Lane RAM RMW word: x%04x = x%04x -> x%04x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_GLOB_RAM_READ_BYTE: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK GLOB RAM READ BYTE   ****"));
                                 EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           0x20000000 | (uint32_t)addr,
                                                                           data,
                                                                           0,
                                                                           data,
                                                                           &state,
                                                                           merlin16_shortfin_diag_access_read_byte_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Glob RAM Read byte: x%04x = x%02x\n",addr,merlin16_shortfin_INTERNAL_rdb_uc_var(sa__, __ERR,addr)));
                             }
                             } break;
    case SRDS_GLOB_RAM_RMW_BYTE:  {
                             uint8_t tmp, tmp2;
                             ESTM(tmp = merlin16_shortfin_INTERNAL_rdb_uc_var(sa__, __ERR,addr));
                             tmp2 = (uint8_t)((tmp & (uint8_t)~param) | ((uint8_t)param & data));
                             EFUN(merlin16_shortfin_INTERNAL_wrb_uc_var(sa__, addr,tmp2));
                             EFUN_PRINTF(("Glob RAM RMW byte: x%04x = x%02x -> x%02x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_GLOB_RAM_READ_WORD: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK GLOB RAM READ WORD   ****"));
                                 EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           0x20000000 | (uint32_t)addr,
                                                                           (uint16_t)(((data+1)>>1)<<1), /* Round up to nearest word count */
                                                                           0,
                                                                           (uint16_t)(((data+1)>>1)<<1), /* Round up to nearest word count */
                                                                           &state,
                                                                           merlin16_shortfin_diag_access_read_word_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Glob RAM Read word: x%04x = x%04x\n",addr,merlin16_shortfin_INTERNAL_rdw_uc_var(sa__, __ERR,addr)));
                             }
                             } break;
    case SRDS_GLOB_RAM_RMW_WORD:  {
                             uint16_t tmp, tmp2;
                             ESTM(tmp = merlin16_shortfin_INTERNAL_rdw_uc_var(sa__, __ERR,addr));
                             tmp2 = (uint16_t)((tmp & ~param) | (param & data));
                             EFUN(merlin16_shortfin_INTERNAL_wrw_uc_var(sa__, addr,tmp2));
                             EFUN_PRINTF(("Glob RAM RMW word: x%04x = x%04x -> x%04x\n",addr,tmp,tmp2));
                             } break;
    case SRDS_UC_CMD:        {
                             uint16_t tmp;
                             EFUN(merlin16_shortfin_pmd_uc_cmd_with_data(sa__, (enum srds_pmd_uc_cmd_enum)addr, (uint8_t)param, data, GRACEFUL_STOP_TIME));
                             ESTM(tmp = rd_uc_dsc_data());
                             EFUN_PRINTF(("uC Command: cmd=x%02x supp=x%02x data=x%04x returned=x%04x\n",addr,param,data,tmp));
                             } break;
    case SRDS_PROG_RAM_READ_BYTE: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK PROG RAM READ BYTE   ****"));
                                 EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           addr,
                                                                           data,
                                                                           0,
                                                                           data,
                                                                           &state,
                                                                           merlin16_shortfin_diag_access_read_byte_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Prog RAM Read byte: x%04x = x%02x\n",addr,merlin16_shortfin_INTERNAL_rdb_uc_var(sa__, __ERR,addr)));
                             }
                             } break;
    case SRDS_PROG_RAM_READ_WORD: {
                             if(data > 1) {
                                 EFUN_PRINTF(("\n****  SERDES BLK PROG RAM READ WORD   ****"));
                                 EFUN(merlin16_shortfin_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                                           addr,
                                                                           (uint16_t)(((data+1)>>1)<<1), /* Round up to nearest word count */
                                                                           0,
                                                                           (uint16_t)(((data+1)>>1)<<1), /* Round up to nearest word count */
                                                                           &state,
                                                                           merlin16_shortfin_diag_access_read_word_callback));
                                 EFUN_PRINTF(("\n"));
                             } else {
                                 ESTM_PRINTF(("Prog RAM Read word: x%04x = x%04x\n",addr,merlin16_shortfin_INTERNAL_rdw_uc_var(sa__, __ERR,addr)));
                             }
                             } break;
    case SRDS_BER_PROJ_DATA: {
                             /* display ber projections for all channels */
                             EFUN(_display_ber_scan_data(sa__, (uint8_t)addr, (uint8_t)data,  (uint8_t)(param>>4)));
                             } break;
    case SRDS_EN_BREAKPOINT: /* FIXME:  Add feature when available */
    case SRDS_GOTO_BREAKPOINT:
    case SRDS_RD_BREAKPOINT:
    case SRDS_DIS_BREAKPOINT:
    default: EFUN_PRINTF(("Invalid request type merlin16_shortfin_diag_access\n"));
    }

    return(ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_display_state (srds_access_t *sa__) {
    EFUN(merlin16_shortfin_display_core_state(sa__));
    EFUN(merlin16_shortfin_display_lane_state_hdr());
    EFUN(merlin16_shortfin_display_lane_state(sa__));
    EFUN(merlin16_shortfin_display_lane_state_legend());
    return(ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_display_config (srds_access_t *sa__) {
    EFUN(merlin16_shortfin_display_core_config(sa__));
    EFUN(merlin16_shortfin_display_lane_config(sa__));
    return(ERR_CODE_NONE);
}

/*************************/
/*  Temperature forcing  */
/*************************/

err_code_t merlin16_shortfin_force_die_temperature (srds_access_t *sa__, int16_t die_temp) {
    /* disable force */
    if(die_temp == -255) {
        EFUN(wrc_micro_pvt_tempdata_frc(0));
        return(ERR_CODE_NONE);
    }
    EFUN_PRINTF(("\n\n\t>>>> FORCING TEMPERATURE!!!!\n\n"));
    /* enable force */
    if (die_temp>130)
        die_temp = 130;
    if (die_temp<-45)
        die_temp = -45;

    EFUN(wrc_micro_pvt_tempdata_frcval((uint16_t)(_degC_to_bin(die_temp))));
    EFUN(wrc_micro_pvt_tempdata_frc(1));

    return(ERR_CODE_NONE);
}

/**********************/
/*  CL72/CL93 Status  */
/**********************/


err_code_t merlin16_shortfin_display_cl72_status(srds_access_t *sa__) {
  EFUN_PRINTF(("\n\n************************\n"                                                                                                                              ));
  ESTM_PRINTF((   "** LANE %d CL72 Status **\n"                                                                                        , merlin16_shortfin_get_lane(sa__)               ));
  EFUN_PRINTF((    "************************\n"                                                                                                                              ));
  ESTM_PRINTF((    "cl72_signal_detect         = %d   (1 = CL72 training FSM in SEND_DATA state;  0 = CL72 in training state)\n"       , rd_cl72_training_fsm_signal_detect()));
  ESTM_PRINTF((    "cl72_ieee_training_failure = %d   (1 = Training failure detected;             0 = Training failure not detected)\n", rd_cl72_ieee_training_failure()     ));
  ESTM_PRINTF((    "cl72_ieee_training_status  = %d   (1 = Start-up protocol in progress;         0 = Start-up protocol complete)\n"   , rd_cl72_ieee_training_status()      ));
  ESTM_PRINTF((    "cl72_ieee_receiver_status  = %d   (1 = Receiver trained and ready to receive; 0 = Receiver training)\n\n"          , rd_cl72_ieee_receiver_status()      ));
  return(ERR_CODE_NONE);
}




