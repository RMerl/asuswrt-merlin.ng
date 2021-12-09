/*
 <:copyright-BRCM:2013:DUAL/GPL:standard
 
    Copyright (c) 2013 Broadcom 
    All Rights Reserved
 
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

#ifndef _PMD_H_
#define _PMD_H_

#include <linux/ioctl.h>
#include "laser.h"
#include "bcmtypes.h"
#include "wan_types.h"


#define PMD_TEMP2APD_FILE_PATH    "/data/pmd_temp2apd"
#define PMD_CALIBRATION_FILE_PATH "/data/pmd_calibration" /*keeps the pmd calibration values */
#define CAL_FILE_INVALID_ENTRANCE 0xbadbad

#define PMD_I2C_HEADER 6 /*  Consist: 1 byte - CSR address, 1byte - config reg, 4 byte - register address */

#define PMD_DEV_CLASS   "laser_dev"
#define PMD_BUF_MAX_SIZE 300
#define PMD_CAL_MESSAGE_MAX_LEN 100

#define PMD_OPERATION_SUCCESS            ( 0)
#define ERR_CAL_PARAM_VALUE_INVALID      (-1)
#define ERR_CAL_PARAM_INDEX_OUT_OF_RANGE (-2)
#define ERR_CAL_PARAM_CANNOT_OPEN_DEVICE (-3)

typedef enum {
    pmd_reg_map,
    pmd_iram_map,
    pmd_dram_map,
    pmd_cal_map,
    pmd_temp2apd_map,
    pmd_dump_data_map,
} pmd_dev_client;

typedef struct {
    uint8_t client;
    uint16_t offset; /* is used as message_id when using the messaging system */
    uint16_t len;
    BCM_IOC_PTR(unsigned char *, buf);
} pmd_params;

typedef struct
{
    uint16_t error_num;
    char error_message[PMD_CAL_MESSAGE_MAX_LEN];
} pmd_cal_msg_str;

/* IOctl */
#define PMD_IOCTL_SET_PARAMS                    _IOW(LASER_IOC_MAGIC, 11, pmd_params)
#define PMD_IOCTL_GET_PARAMS                    _IOR(LASER_IOC_MAGIC, 12, pmd_params)
#define PMD_IOCTL_CAL_FILE_WRITE                _IOW(LASER_IOC_MAGIC, 13, pmd_params)
#define PMD_IOCTL_CAL_FILE_READ                 _IOR(LASER_IOC_MAGIC, 14, pmd_params)
#define PMD_IOCTL_MSG_WRITE                     _IOW(LASER_IOC_MAGIC, 15, pmd_params)
#define PMD_IOCTL_MSG_READ                      _IOR(LASER_IOC_MAGIC, 16, pmd_params)
#define PMD_IOCTL_RSSI_CAL                      _IOR(LASER_IOC_MAGIC, 17, pmd_params)
#define PMD_IOCTL_TEMP2APD_WRITE                _IOR(LASER_IOC_MAGIC, 18, pmd_params)
#define PMD_IOCTL_DUMP_DATA                     _IOR(LASER_IOC_MAGIC, 19, pmd_params)
#define PMD_IOCTL_RESET_INTO_CALIBRATION_MODE   _IOR(LASER_IOC_MAGIC, 20, pmd_params)


#define PMD_ESTIMATED_OP_GET_MSG       8   /* hmid_estimated_op_get */
#define PMD_RSSI_GET_MSG               0xe /* hmid_rssi_non_cal_get */
#define PMD_RSSI_A_FACTOR_CAL_SET_MSG  0x9d /* hmid_rssi_a_factor_cal_set */
#define PMD_RSSI_B_FACTOR_CAL_SET_MSG  0x9e /* hmid_rssi_b_factor_cal_set */

typedef struct
{
    uint32_t esc_be                :1 ;
    uint32_t esc_rogue             :1 ;
    uint32_t esc_mod_over_current  :1 ;
    uint32_t esc_bias_over_current :1 ;
    uint32_t esc_mpd_fault         :1 ;
    uint32_t esc_eye_safety        :1 ;
}PMD_ALARM_INDICATION_PARAMETERS_DTE;

#define PMD_GPON_STATE_INIT_O1                  0
#define PMD_GPON_STATE_STANDBY_O2               1
#define PMD_GPON_STATE_SERIAL_NUMBER_O3         2
#define PMD_GPON_STATE_RANGING_O4               3
#define PMD_GPON_STATE_OPERATION_O5             4
#define PMD_GPON_STATE_POPUP_O6                 5
#define PMD_GPON_STATE_EMERGENCY_STOP_O7        6

#define PMD_CALIBRATION_UNCALIBRATED    0x0    
#define PMD_CALIBRATION_RX_DONE         0x1
#define PMD_CALIBRATION_TX_DONE         0x2
#define PMD_CALIBRATION_RXTX_DONE       0x3   

typedef enum
{
    pmd_file_watermark          = 0,
    pmd_file_version            = 1,
    pmd_level_0_dac             = 2,
    pmd_level_1_dac             = 3,
    pmd_bias                    = 4,
    pmd_mod                     = 5,
    pmd_apd                     = 6,
    pmd_mpd_config              = 7,
    pmd_mpd_gains               = 8,
    pmd_apdoi_ctrl              = 9,
    pmd_rssi_a                  = 10, /* optic_power = a * rssi + b */
    pmd_rssi_b                  = 11,
    pmd_rssi_c                  = 12,
    pmd_temp_0                  = 13, /* calibration temperature */
    pmd_temp_coff_h             = 14, /* Temperature coefficient high */
    pmd_temp_coff_l             = 15, /* Temperature coefficient low */
    pmd_esc_thr                 = 16,
    pmd_rogue_thr               = 17,
    pmd_avg_level_0_dac         = 18,
    pmd_avg_level_1_dac         = 19,
    pmd_dacrange	            = 20,
    pmd_los_thr                 = 21,
    pmd_sat_pos                 = 22,
    pmd_sat_neg                 = 23,
    pmd_edge_rate 	            = 24,
    pmd_preemphasis_weight      = 25,
    pmd_preemphasis_delay       = 26,
    pmd_duty_cycle              = 27,
    pmd_mpd_calibctrl           = 28,
    pmd_tx_power                = 29,
    pmd_bias0                   = 30,
    pmd_temp_offset             = 31,
    pmd_bias_delta_i            = 32,
    pmd_slope_efficiency        = 33,
    pmd_temp_table              = 34,
    pmd_adf_los_thresholds      = 35,
    pmd_adf_los_leaky_bucket    = 36,
    pmd_compensation            = 37,

    pmd_num_of_cal_param,

    pmd_host_sw_control         = 60, /* backdoor */
    pmd_statistics_collection   = 61, /* backdoor */
}pmd_calibration_param;

extern PMD_WAN_TYPES pmd_wan_type;

typedef struct
{
    uint32_t alarms;
    uint32_t sff;
}pmd_msg_addr;

typedef void (*pmd_gpon_isr_callback)(void);
typedef int (*pmd_statistic_first_burst_callback)(uint16_t *bias, uint16_t *mod);
typedef int (*pmd_math_first_burst_model_callback)(uint16_t *bias, uint16_t *mod);
typedef void (*pon_set_pmd_fb_done_callback)(uint8_t state);

int pmd_dev_poll_epon_alarm(void);
void pmd_dev_change_tracking_state(uint32_t old_state, uint32_t new_state);
void pmd_dev_enable_prbs_or_misc_mode(uint16_t enable, uint8_t prbs_mode);
void pmd_dev_assign_pon_stack_callback(pon_set_pmd_fb_done_callback _pon_set_pmd_fb_done);
int pmd_dev_reconfigure_wan_type(PMD_WAN_TYPES new_pmd_wan_type);

#endif /* ! _PMD_H_ */
