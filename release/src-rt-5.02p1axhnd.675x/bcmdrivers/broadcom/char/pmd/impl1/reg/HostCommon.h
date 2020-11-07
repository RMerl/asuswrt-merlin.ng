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
#if !defined(HostCommon_h)
#define HostCommon_h
/**
 * \file HostCommon.h
 * \brief Common host/embedded communication protocol objects
 *
 */
#include <linux/types.h>

#if !defined(GET_FIELD) && !defined(SET_FIELD)
#define BRCM_ALIGN(c,r,f)   c##_##r##_##f##_ALIGN
#define BRCM_BITS(c,r,f)    c##_##r##_##f##_BITS
#define BRCM_MASK(c,r,f)    c##_##r##_##f##_MASK
#define BRCM_SHIFT(c,r,f)   c##_##r##_##f##_SHIFT

#define GET_FIELD(m,c,r,f) \
	((((m) & BRCM_MASK(c,r,f)) >> BRCM_SHIFT(c,r,f)) << BRCM_ALIGN(c,r,f))

#define SET_FIELD(m,c,r,f,d) \
	((m) = (((m) & ~BRCM_MASK(c,r,f)) | ((((d) >> BRCM_ALIGN(c,r,f)) << \
	 BRCM_SHIFT(c,r,f)) & BRCM_MASK(c,r,f))) \
	)

#define SET_TYPE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##d)
#define SET_NAME_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##r##_##f##_##d)
#define SET_VALUE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,d)

#endif /* GET & SET */

/* Mailbox register format
 *    [31] Sync bit 
 *    [30] Status bit 
 * [29:24] Sequence number 
 * [23:16] Message ID 
 *  [15:0] Message data 
 */
#define HM_MSG_SYNC_MASK        0x80000000UL
#define HM_MSG_SYNC_SHIFT       31
#define HM_MSG_SYNC_ALIGN       0
#define HM_MSG_STAT_MASK        0x40000000UL
#define HM_MSG_STAT_SHIFT       30
#define HM_MSG_STAT_ALIGN       0
#define HM_MSG_SEQ_MASK         0x3f000000UL
#define HM_MSG_SEQ_SHIFT        24
#define HM_MSG_SEQ_ALIGN        0
#define HM_MSG_ID_MASK          0x00ff0000UL
#define HM_MSG_ID_SHIFT         16
#define HM_MSG_ID_ALIGN         0
#define HM_MSG_DATA_MASK        0x0000ffffUL
#define HM_MSG_DATA_SHIFT       0
#define HM_MSG_DATA_ALIGN       0

typedef uint32_t hm_mailbox;

typedef enum
{
    /* Get messages */
    hmid_start_get,
    hmid_test_get = hmid_start_get,
    hmid_software_version_get           = 0x01,
    hmid_alarms_get                     = 0x02,
    hmid_uptime_get                     = 0x03,
    hmid_cur_state_get                  = 0x04,
    hmid_reserved0                      = 0x05,
    hmid_reserved1                      = 0x06,
    hmid_chip_temp_get                  = 0x07,
    hmid_estimated_op_get               = 0x08,
    hmid_offset_cal_get                 = 0x09,     /* Super important */
    hmid_level_0_get                    = 0x0a,     /* Super important */
    hmid_level_1_get                    = 0x0b,     /* Super important */
    hmid_mpd_config_get                 = 0x0c,     /* Super important */
    hmid_memtest_results_get            = 0x0d,
    hmid_rssi_get                       = 0x0e,
    hmid_rssi_sampling_delta_get        = 0x0f,
    hmid_otp_word_get                   = 0x10,
    hmid_temperature_get                = 0x11,
    hmid_esc_alarm_int_get              = 0x12,
    hmid_temp_table_checksum_get        = 0x13,
    hmid_esc_debug_level_get            = 0x14,
    hmid_lrl_los_peaks_get              = 0x15,
    hmid_task_stuck_get                 = 0x16,
    hmid_cal_result_get                 = 0x17,
    hmid_bias_cur_get                   = 0x18,
    hmid_avrg_tx_power_tracking_get     = 0x19,
    hmid_mpd_slow_response_levels_get   = 0x1a,
    hmid_cal_error_get                  = 0x1b,

    /* Set messages */
    hmid_start_set                      = 0x80,
    hmid_test_set                       = 0x80,
    hmid_cmp_avg_set                    = 0x81,
    hmid_bias_set                       = 0x82,
    hmid_mod_set                        = 0x83,
    hmid_burst_en_dep_set               = 0x84,
    hmid_reserved2                      = 0x85,
    hmid_tracking_method_set            = 0x86,   
    hmid_tracking_ldc_machine_set       = 0x87,
    hmid_laser_tracking_set             = 0x88,     /* Super important */
    hmid_apd_step_set                   = 0x89,

    hmid_level_0_dac_ref                = 0x8b,     /* Super important */
    hmid_level_1_dac_ref                = 0x8c,     /* Super important */
    hmid_mpd_config_set                 = 0x8d,     /* Super important */
    hmid_apd_config_set                 = 0x8e,     /* DEPRECATED      */
    hmid_preemphasis_weight_set         = 0x8f,
    hmid_off_sample_set                 = 0x90,
    hmid_preemphasis_delay_set          = 0x91,
    hmid_l0_sample_set                  = 0x92,
    hmid_interation_set                 = 0x93,
    hmid_l1_sample_set                  = 0x94,
    hmid_esc_block_enable_set           = 0x95,
    hmid_lrl_los_thr_set                = 0x96,
    hmid_rssi_c_factor_cal_set          = 0x97,
    hmid_edge_rate_set                  = 0x98,
    hmid_duty_cycle_set                 = 0x99,
    hmid_max_current_set                = 0x9a,
    hmid_mpd_gains_set                  = 0x9b,
    hmid_apdoi_ctrl_set                 = 0x9c,
    hmid_rssi_a_factor_cal_set          = 0x9d,
    hmid_rssi_b_factor_cal_set          = 0x9e,
    hmid_otp_bit_set                    = 0x9f,
    hmid_otp_word_set                   = 0xa0,
    hmid_otp_read_addr_set              = 0xa1,
    hmid_board_type_set                 = 0xa2,
    hmid_esc_thr_set                    = 0xa3,
    hmid_rogue_thr_set                  = 0xa4,
    hmid_temp_0_set                     = 0xa5,
    hmid_temp_coff_h_set                = 0xa6,
    hmid_temp_coff_l_set                = 0xa7,
    hmid_calibctrl_set                  = 0xa8,
    hmid_avg_level_0_dac_ref            = 0xa9,     
    hmid_avg_level_1_dac_ref            = 0xaa,     
    hmid_dacrange_set                   = 0xab,
    hmid_lia_rssi_peakpos_sat_set       = 0xac,
    hmid_lia_rssi_peakneg_sat_set       = 0xad,
    hmid_cal_type_set                   = 0xae,
    hmid_cal_param_set                  = 0xaf,
    hmid_cal_biasctrl_measured_op       = 0xb0,
    hmid_cal_modctrl_measured_op        = 0xb1,
    hmid_cal_rssi_op_set                = 0xb2,
    hmid_cal_avg_tune_measured_op       = 0xb3,
    hmid_cal_meas_tx_power              = 0xb4,
    hmid_cal_meas_bias0                 = 0xb5,
    hmid_cal_measured_er_set            = 0xb6,
    hmid_temp_offset_set                = 0xb7,
    hmid_alarm_clear                    = 0xb8,
    hmid_cal_bias_delta_i               = 0xb9,
    hmid_cal_slope_efficiency           = 0xba,
    hmid_force_enable                   = 0xbb,
    hmid_config_done                    = 0xbc,
    hmid_esc_delay                      = 0xbd,
    hmid_adf_los_thresholds_set         = 0xbe,
    hmid_adf_los_leaky_bucket_set       = 0xbf,
    hmid_compensation_params_set        = 0xc0,

    /* Development debug messages */
    hmid_be_on_off_set                  = 0xE5,
    hmid_cmp_delay_set                  = 0xE6,
    reserved7                           = 0xE9,
    hmid_cid_length_set                 = 0xEA,
    hmid_prbs_set                       = 0xEB,
    hmid_last,
    hmid_invalid                        = 0xff
} hm_msg_id;

typedef enum
{
    hm_err_no_error     = 0,    /*< No error has occured */
    hm_err_bad_param    = 1,    /*< Parameters are out of range */
    hm_err_bad_sync     = 2,    /*< Sync bit toggled unexpectedly */
    hm_err_msg_unknown  = 3,    /*< EP received unknown message if from host */
    hm_err_seq_fail     = 4,    /*< Unexpected sequence number */
    hm_err_overrun      = 5,    /*< Sequence number was higher than expected */
    hm_err_msg_cont     = 6     /*< Long message ID didn't match previous msg */
} hm_error;                     /*< EP to host failure code */

typedef enum
{
    hm_stat_in_prog = 0,        /*< Host/EP: message in progress */
    hm_stat_start   = 1,        /*< Host: start a new message transaction */
    hm_stat_fail    = 1         /*< EP: message has failed */
} hm_status;                    /*< Message transaction status */

typedef struct
{
    uint8_t sync;               /*< Toggling 0,1 syncronizing bit */
    hm_status status;           /*< Message status, start or in progress */
    uint8_t seq;                /*< Long message sequence number */
    hm_msg_id id;               /*< Message id */
    int16_t value;              /*< Message data or request qualifiers */
} hm_message;                   /*< Host message container */

typedef enum
{
    pmd_system_reset_normal,
    pmd_system_reset_watchdog,
    pmd_system_fatal_error,
    pmd_eye_safety,
    pmd_otp_failure,
    pmd_loss_of_signal,
    pmd_calibration_ready,
    pmd_first_burst_complete,
    pmd_alarm_reserved1,
    pmd_mem_data_test_fail,
    pmd_mem_addr_test_fail,
    pmd_mem_cell_test_fail,
    pmd_mpd_pwrup_fail,
    pmd_task_stuck_error,
    pmd_cal_state_over,
    pmd_thermistor_misbehaviour,
    pmd_alarm_num
}pmd_alarm_val;


typedef enum
{
    PMD_RESET_NORMAL = 1 << pmd_system_reset_normal,
    PMD_RESET_WATCHDOD = 1 << pmd_system_reset_watchdog,
    PMD_FATAL_ERROR = 1 << pmd_system_fatal_error,
    PMD_EYE_SAFETY = 1 << pmd_eye_safety,
    PMD_OTP_FAILURE = 1 << pmd_otp_failure,
    PMD_LOSS_SIGNAL = 1 << pmd_loss_of_signal,
    PMD_CALIBRATION_READY = 1 << pmd_calibration_ready,
    PMD_FIRST_BURST_COMPLETE = 1 << pmd_first_burst_complete,
    PMD_MEM_DATA_TEST_FAIL = 1 << pmd_mem_data_test_fail,
    PMD_MEM_ADDR_TEST_FAIL = 1 << pmd_mem_addr_test_fail,
    PMD_MEM_CELL_TEST_FAIL = 1 << pmd_mem_cell_test_fail,
    ALM_MPD_POWER_UP_FAIL = 1 << pmd_mpd_pwrup_fail,
    ALM_TASK_STUCK_ERROR = 1 << pmd_task_stuck_error,
    ALM_CAL_STATE_OVER = 1 << pmd_cal_state_over,
    ALM_THERMISTOR_MISBEHAVIOUR = 1 << pmd_thermistor_misbehaviour
}pmd_alarm_fields;

typedef enum
{
    pmd_esc_eyesafe_alarm,
    pmd_esc_mpd_fault_alarm,
    pmd_esc_bias_overcurrent_alarm,
    pmd_esc_mod_overcurrent_alarm,
    pmd_esc_rogue_alarm,
    pmd_esc_burst_enable_alarm,
}pmd_esc_alarm_val;


typedef enum
{
    PMD_ESC_ALARM_EYESAFE = 1 << pmd_esc_eyesafe_alarm,
    PMD_ESC_ALARM_MPD_FAULT = 1 << pmd_esc_mpd_fault_alarm,
    PMD_ESC_BIAS_OVR_CRNT = 1 << pmd_esc_bias_overcurrent_alarm,
    PMD_ESC_MOD_OVR_CRNT = 1 << pmd_esc_mod_overcurrent_alarm,
    PMD_ESC_ROGUE_ALARM = 1 << pmd_esc_rogue_alarm,
    PMD_ESC_BE_ALARM = 1<< pmd_esc_burst_enable_alarm,
}pmd_esc_alarm_field;

typedef enum
{
    pmd_tracking_state_disabled,        /* Don't execute any tracking */
    pmd_tracking_state_tune,            /* Auto-tune monitor photo diode */
    pmd_tracking_state_first_burst,     /* One shot fire of fire burst */
    pmd_tracking_state_tracking,        /* Full tracking with collection */
} pmd_tracking_state;

typedef enum
{
    pmd_idle_task_stuck_alarm,
    pmd_laser_task_stuck_alarm,
    pmd_rssi_task_stuck_alarm,
    pmd_timer_task_stuck_alarm
}pmd_task_stuck_error_val;

typedef enum
{
    pmd_cal_cant_gurantee_clear_los_alarm,
    pmd_cal_modctrl_clipped_alarm,
    pmd_cal_no_optical_power_measure_alarm,
    pmd_cal_mpd_channel_error,
    pmd_cal_biasctl_slop_not_found_alarm,
}pmd_cal_error_val;

typedef enum
{
     PMD_IDLE_TASK_STUCK_ALARM = 1 << pmd_idle_task_stuck_alarm,
     PMD_LASER_TASK_STUCK_ALARM = 1 << pmd_laser_task_stuck_alarm,
     PMD_RSSI_TASK_STUCK_ALARM  = 1 << pmd_rssi_task_stuck_alarm,
     PMD_TIMER_TASK_STUCK_ALARM = 1 << pmd_timer_task_stuck_alarm
 } pmd_task_stuck_error_msg;

 typedef enum
 {
     PMD_CAL_CANT_GUARANTEE_CLEAR_LOS_ALARM = 1 << pmd_cal_cant_gurantee_clear_los_alarm,
     PMD_CAL_MODCTRL_CLIPPED_ALARM = 1 << pmd_cal_modctrl_clipped_alarm,
     PMD_CAL_NO_OPTICAL_POWER_MEASURE_ALARM = 1 << pmd_cal_no_optical_power_measure_alarm,
     PMD_CAL_MPD_CHANNEL_ERROR = 1 << pmd_cal_mpd_channel_error,
     PMD_CAL_BIASCTL_SLOP_ALARM = 1 << pmd_cal_biasctl_slop_not_found_alarm,
 } pmd_cal_error_msg;


/*******************************************************************************
 * Conversion functions
 ******************************************************************************/

/**
 * \brief Convert mailbox format to host message format
 *
 * This function takes a message as it was received from the mailbox registers 
 * and parsed it into a host message format for software to use. 
 *
 * \param mailbox Mailbox formatted register 
 * \param msg Host message format container 
 *
 * \return
 * None
 */
void hm_mailbox_to_message(hm_mailbox mailbox, hm_message *msg);


/**
 * \brief Convert host message format to mailbox format
 *
 * This function takes a message as it is used in software and converts it to a 
 * format compatible with the mailbox interface. 
 *
 * \param msg Host message format message 
 * \param mailbox Mailbox formatted container
 *
 * \return
 * None
 */
void hm_message_to_mailbox(const hm_message *msg, hm_mailbox *mailbox);


#endif /* End of file pmd_host_common.h */
