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
#include <linux/bcm_log.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include "blocks.h"
#include "HostCommon.h"
#include "clkrst_testif.h"
#include "pmd_op.h"
#include "pmd_msg.h"

#define INGRESS_MAILBOX CRT_ADDR + CRT_CPU_INGRESS_MAIL_BOX
#define EGRESS_MAILBOX  CRT_ADDR + CRT_CPU_EGRESS_MAIL_BOX
#define MAX_RETRIES_NUM 3

static uint8_t sync_bit = 1;
static int msg_state;
static int retries_counter;
static int retries_set_flag;

extern uint16_t last_hmid_cal_set_argument;

static DEFINE_SPINLOCK(pmd_msg_list_lock);

typedef struct
{
	hm_message host_msg;
	hm_message pmd_msg;
    uint16_t len;
    int16_t *buf;
} msg_params;

/* holds the num of transaction sequence need for each message type */
uint8_t msg_seq_num[hmid_last];

typedef enum
{
    initiate_msg_state = 0,
    read_msg_state,
    send_msg_state,
    finish_msg_op
} pmd_msg_state;

char * msg_state_string[4] = {"init", "read", "send", "finish"};

static int initiate_msg(msg_params *param);
static int read_msg(msg_params *param);
static int send_msg(msg_params *param);
static void reset_retries(void);

typedef int (* msg_fun) (msg_params *params);

static msg_fun msg_state_machine[finish_msg_op] =
{
    initiate_msg,
    read_msg,
    send_msg,
};

void msg_system_init(void)
{
    sync_bit = 1;
    reset_retries();

    /* get */
    msg_seq_num[hmid_test_get] = 8;
    msg_seq_num[hmid_software_version_get] = 2;
    msg_seq_num[hmid_alarms_get] = 1;
    msg_seq_num[hmid_uptime_get] = 4;
    msg_seq_num[hmid_cur_state_get] = 10;
    msg_seq_num[hmid_chip_temp_get] =1;
    msg_seq_num[hmid_estimated_op_get] = 1;
    msg_seq_num[hmid_offset_cal_get] = 2;
    msg_seq_num[hmid_level_0_get] = 1;
    msg_seq_num[hmid_level_1_get] = 1;
    msg_seq_num[hmid_mpd_config_get] = 1;
    msg_seq_num[hmid_memtest_results_get] = 1;
    msg_seq_num[hmid_rssi_get] = 2;
    msg_seq_num[hmid_rssi_sampling_delta_get] = 2;
    msg_seq_num[hmid_otp_word_get] = 2;
    msg_seq_num[hmid_temperature_get] = 1;
    msg_seq_num[hmid_esc_alarm_int_get] = 1;
    msg_seq_num[hmid_temp_table_checksum_get] = 2;
    msg_seq_num[hmid_esc_debug_level_get] = 1;
    msg_seq_num[hmid_lrl_los_peaks_get] = 1;
    msg_seq_num[hmid_task_stuck_get] = 1;
    msg_seq_num[hmid_esc_block_enable_set] = 1;
    msg_seq_num[hmid_cal_result_get] = 25;
    msg_seq_num[hmid_bias_cur_get] = 1;
    msg_seq_num[hmid_avrg_tx_power_tracking_get] = 1;
    msg_seq_num[hmid_mpd_slow_response_levels_get] = 5;
    msg_seq_num[hmid_cal_error_get] = 1;

    /* set */
    msg_seq_num[hmid_test_set] = 8;
    msg_seq_num[hmid_cmp_avg_set] = 1;
    msg_seq_num[hmid_bias_set] = 1;
    msg_seq_num[hmid_mod_set] = 1;
    msg_seq_num[hmid_burst_en_dep_set] = 1;
    msg_seq_num[hmid_tracking_method_set] = 1;
    msg_seq_num[hmid_tracking_ldc_machine_set] = 1;
    msg_seq_num[hmid_laser_tracking_set] = 1;
    msg_seq_num[hmid_apd_step_set] = 1;
    msg_seq_num[hmid_level_0_dac_ref] = 1;
    msg_seq_num[hmid_level_1_dac_ref] = 1;
    msg_seq_num[hmid_mpd_config_set] = 1;
    msg_seq_num[hmid_apd_config_set] = 1; /* DEPRECATED */
    msg_seq_num[hmid_preemphasis_weight_set] = 1;
    msg_seq_num[hmid_off_sample_set] = 1;
    msg_seq_num[hmid_preemphasis_delay_set] = 2;
    msg_seq_num[hmid_l0_sample_set] = 1;
    msg_seq_num[hmid_interation_set] = 1;
    msg_seq_num[hmid_l1_sample_set] = 1;
    msg_seq_num[hmid_lrl_los_thr_set] = 1;   
    msg_seq_num[hmid_edge_rate_set] = 1;
    msg_seq_num[hmid_duty_cycle_set] = 1;
    msg_seq_num[hmid_max_current_set] = 1;
    msg_seq_num[hmid_mpd_gains_set] = 1;
    msg_seq_num[hmid_apdoi_ctrl_set] = 1;
    msg_seq_num[hmid_rssi_a_factor_cal_set] = 2;
    msg_seq_num[hmid_rssi_b_factor_cal_set] = 2;
    msg_seq_num[hmid_rssi_c_factor_cal_set] = 2;
    msg_seq_num[hmid_otp_bit_set] = 1;    
    msg_seq_num[hmid_otp_word_set] = 3;
    msg_seq_num[hmid_otp_read_addr_set] = 1;
    msg_seq_num[hmid_esc_thr_set] = 1;
    msg_seq_num[hmid_rogue_thr_set] = 1;
    msg_seq_num[hmid_board_type_set] = 1;
    msg_seq_num[hmid_esc_thr_set ] = 1;
    msg_seq_num[hmid_rogue_thr_set] = 1;
    msg_seq_num[hmid_temp_0_set] = 1;
    msg_seq_num[hmid_temp_coff_h_set] = 1;
    msg_seq_num[hmid_temp_coff_l_set] = 1;
    msg_seq_num[hmid_calibctrl_set] = 1;
    msg_seq_num[hmid_avg_level_0_dac_ref] = 1;
    msg_seq_num[hmid_avg_level_1_dac_ref] = 1;
    msg_seq_num[hmid_dacrange_set] = 1;
    msg_seq_num[hmid_lia_rssi_peakpos_sat_set] = 1;
    msg_seq_num[hmid_lia_rssi_peakneg_sat_set] = 1;
    msg_seq_num[hmid_cal_type_set] = 1;
    msg_seq_num[hmid_cal_param_set] = 13;
    msg_seq_num[hmid_cal_biasctrl_measured_op] = 1;
    msg_seq_num[hmid_cal_modctrl_measured_op] = 1;
    msg_seq_num[hmid_cal_rssi_op_set] = 1;
    msg_seq_num[hmid_cal_avg_tune_measured_op] = 1;
    msg_seq_num[hmid_cal_meas_tx_power] = 1;
    msg_seq_num[hmid_cal_meas_bias0] = 1;
    msg_seq_num[hmid_cal_measured_er_set] = 1;
    msg_seq_num[hmid_temp_offset_set] = 1;
    msg_seq_num[hmid_alarm_clear] = 1;
    msg_seq_num[hmid_cal_bias_delta_i] = 1;
    msg_seq_num[hmid_cal_slope_efficiency] = 1;
    msg_seq_num[hmid_force_enable] = 1;
    msg_seq_num[hmid_config_done] = 1;
    msg_seq_num[hmid_esc_delay] = 1;
    msg_seq_num[hmid_adf_los_thresholds_set] = 2;
    msg_seq_num[hmid_adf_los_leaky_bucket_set] = 1;
    msg_seq_num[hmid_compensation_params_set] = 2;

};

host_msg_type get_msg_type(hm_msg_id id)
{
    if (id < hmid_start_set)
        return pmd_get_msg;
    return pmd_set_msg;
}

/* returns msg len in multipels of uint16_t */
uint16_t get_msg_seq_num(hm_msg_id id)
{
    return msg_seq_num[id];
}

static int set_ingress_mailbox(hm_message *msg)
{
    int rc;
    uint32_t mailbox = 0;

    hm_message_to_mailbox(msg, &mailbox);

#ifdef __LITTLE_ENDIAN
    mailbox = swab32(mailbox);
#endif

    rc = pmd_op_i2c(pmd_reg_map, INGRESS_MAILBOX, (unsigned char *)&mailbox, 4, PMD_WRITE_OP);
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "mailbox reg = 0x%x (sync %d status %d seq %d id 0x%x value 0x%x). error = %d\n",
    		mailbox, msg->sync, msg->status, msg->seq, msg->id, msg->value, rc);
#endif
    return rc;
}

static int get_egress_mailbox(hm_message *msg)
{
    int rc;
    uint32_t mailbox;

    rc = pmd_op_i2c(pmd_reg_map, EGRESS_MAILBOX, (unsigned char *)&mailbox, 4, PMD_READ_OP);
    if (rc)
        return rc;

#ifdef __LITTLE_ENDIAN
    mailbox = swab32(mailbox);
#endif

    hm_mailbox_to_message(mailbox, msg);

#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "mailbox = 0x%x (sync %d status %d seq %d id 0x%x value 0x%x). error = %d\n", mailbox,
        msg->sync, msg->status, msg->seq, msg->id, msg->value, rc);
#endif
    return 0;
}

static void reset_retries(void)
{
    retries_set_flag = 0;
    retries_counter = 0;
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, " \n");
#endif
}

static void abort_msg(void)
{
    /* Reset sync bit */
    sync_bit = 1;
    reset_retries();
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, " Msg transaction failed \n");
}

static int set_retries(int count)
{
    if (retries_set_flag)
    {
        /* operation failed abort */
        if (!retries_counter)
        {
            reset_retries();
            abort_msg();
            return -1;
        }
        retries_counter--;
        /* sleep ms */
        mdelay(10);
    }
    else
    {
        retries_set_flag = 1;
        retries_counter = count;
    }
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, " retries_set_flag %d retries_counter %d\n", retries_set_flag,  retries_counter);
#endif
    return 0;
}

static void finish_msg(void)
{
    /* Done with the msg, flip the sync bit */
    sync_bit = ~sync_bit & 1;
    msg_state = finish_msg_op;
    reset_retries();
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Next sync_bit = %d\n", sync_bit);
#endif
}

static int process_pmd_msg(msg_params *params, int rc)
{
    /* i2c error, stay in the same state and retry */
    if (rc)
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "i2c transaction failed \n");
        return set_retries(MAX_RETRIES_NUM);
    }
    /* pmd has yet moved to the new message, stay in the same state and retry */
    if (params->pmd_msg.sync != sync_bit)
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Sync mismatch. Expected = %d\n", sync_bit);
        return set_retries(MAX_RETRIES_NUM);
    }
    /* msg error, abort the current message and reset the sync bit to 1 */
    if (unlikely(params->pmd_msg.status != hm_stat_in_prog))
    {
        switch (params->pmd_msg.value)
        {
        case hm_err_bad_param:
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "bad parameters error \n");
            break;
        case hm_err_bad_sync:
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "sync mismatch error \n");
            break;
        case hm_err_msg_unknown:
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "unknown msg error \n");
            break;
        case hm_err_seq_fail:
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Wrong Sequence number received \n");
            break;
        case hm_err_overrun:
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Sequence number higher than expected \n");
            break;
        case hm_err_msg_cont:
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "msg ID mismatch \n");
            break;
        default:
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "msg error is out of range \n");
            break;
        }
        abort_msg();
        return -1;
    }
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Seq %d len %d\n", params->pmd_msg.seq, params->len);
#endif
    /* pmd has yet updated the egress mailbox, stay in the same state and retry */
    if (params->pmd_msg.seq != params->host_msg.seq)
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Seq mismatch. Expected = %d received %d\n",
                params->host_msg.seq, params->pmd_msg.seq);
        return set_retries(MAX_RETRIES_NUM);
    }
    /* If we reach here, there are no errors */

    /* all transactions done*/
    if (params->pmd_msg.seq == params->len - 1)
    {
        finish_msg();
        return 0;
    }
    /* move to the next state */
    msg_state = send_msg_state;
    params->host_msg.seq++;
    reset_retries();
    return 0;
}

static int process_host_msg(int rc)
{
    /* no errors, go to next state */
    if (!rc)
    {
        msg_state = read_msg_state;
        reset_retries();
        return 0;
    }
    /* i2c error, stay in the same state and retry */
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "i2c transaction failed \n");
    return set_retries(MAX_RETRIES_NUM);
}

static int initiate_msg(msg_params *param)
{
    int rc;

    param->host_msg.sync = sync_bit;
    param->host_msg.status = hm_stat_start;
    param->host_msg.seq = 0;

    if (get_msg_type(param->host_msg.id) == pmd_set_msg)
        param->host_msg.value = param->buf[0];

    rc = set_ingress_mailbox(&param->host_msg);
    return process_host_msg(rc);
}

static int read_msg(msg_params *param)
{
    int rc;

    rc = get_egress_mailbox(&param->pmd_msg);
    if (get_msg_type(param->host_msg.id) == pmd_get_msg)
        param->buf[param->host_msg.seq] = param->pmd_msg.value;
    return process_pmd_msg(param, rc);
}

static int send_msg(msg_params *param)
{
    int rc;

    param->host_msg.status = hm_stat_in_prog;
    if (get_msg_type(param->host_msg.id) == pmd_set_msg)
        param->host_msg.value = param->buf[param->host_msg.seq];

    rc = set_ingress_mailbox(&param->host_msg);
    return process_host_msg(rc);
}

int pmd_msg_handler(hm_msg_id id, int16_t *buf, uint16_t len)
{
    msg_params param = { };
    int rc;
    uint16_t msg_seq;

    spin_lock_bh(&pmd_msg_list_lock);

    if (hmid_cal_type_set == id)
    {
        last_hmid_cal_set_argument = *buf;
    }

    msg_seq = get_msg_seq_num(id);
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "msg id 0x%x msg transactions %d sync %d\n", (int)id, msg_seq, sync_bit);
#endif
    if (id > hmid_last || msg_seq == 0)
    {
        spin_unlock_bh(&pmd_msg_list_lock);
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "msg id 0x%x, msg_seq %d issue\n", (int)id, msg_seq);
        return -1;
    }

    if (len != (msg_seq * 2))
    {
        spin_unlock_bh(&pmd_msg_list_lock);
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Wrong len: msg 0x%x expecting %d\n" ,(int)id, (msg_seq *2));
        return -1;
    }

    param.host_msg.id = id;
    param.buf = buf;
    param.len = msg_seq;

    while (msg_state != finish_msg_op)
    {
#ifdef PMD_FULL_DEBUG
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "msg state %s\n", msg_state_string[msg_state]);
#endif
        rc = msg_state_machine[msg_state](&param);
        if (rc)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Failed in msg 0x%x state %d\n", id, msg_state);
            break;
        }
    }

    msg_state = initiate_msg_state;

    spin_unlock_bh(&pmd_msg_list_lock);

    return rc;
}


