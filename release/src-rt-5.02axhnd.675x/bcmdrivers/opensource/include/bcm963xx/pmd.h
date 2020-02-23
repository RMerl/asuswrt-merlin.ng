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
#include <net_port.h>

#define PMD_TEMP2APD_FILE_PATH    "/data/pmd_temp2apd"
#define PMD_CALIBRATION_FILE_PATH "/data/pmd_calibration" /*keeps the pmd calibration values */
#define PMD_CALIBRATION_JSON_FILE_PATH "/data/pmd_calibration.json" /*keeps the pmd calibration values */
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
    pmd_dump_data_map,
} pmd_dev_client;

typedef struct {
    uint8_t client;
    uint16_t offset; /* is used as message_id when using the messaging system */
    uint16_t len;
    BCM_IOC_PTR(unsigned char *, buf);
    net_port_t net_port;
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
//#define PMD_IOCTL_RSSI_CAL                      _IOW(LASER_IOC_MAGIC, 17, pmd_params)
#define PMD_IOCTL_TEMP2APD_WRITE                _IOW(LASER_IOC_MAGIC, 18, pmd_params)
#define PMD_IOCTL_DUMP_DATA                     _IOR(LASER_IOC_MAGIC, 19, pmd_params)
#define PMD_IOCTL_RES2TEMP_WRITE                _IOW(LASER_IOC_MAGIC, 20, pmd_params)
#define PMD_IOCTL_RESET_INTO_CALIBRATION_MODE   _IOW(LASER_IOC_MAGIC, 21, pmd_params)
#define PMD_IOCTL_SET_WAN_TYPE                  _IOW(LASER_IOC_MAGIC, 22, pmd_params)

#define PMD_FW_VERSION_GET_MSG         0x1  /* hmid_software_version_get */
#define PMD_ESTIMATED_OP_GET_MSG       0x8  /* hmid_estimated_op_get */
#define PMD_RSSI_GET_MSG               0xe  /* hmid_rssi_non_cal_get */
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
} PMD_ALARM_INDICATION_PARAMETERS_DTE;

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
    PMD_FILE_WATERMARK          = 0,
    PMD_CALIBRATION_FILE_VER    = 1,
    PMD_FAQ_LEVEL0_DAC          = 2,
    PMD_FAQ_LEVEL1_DAC          = 3,
    PMD_BIAS                    = 4,
    PMD_MOD                     = 5,
    PMD_APD                     = 6,
    PMD_MPD_CONFIG              = 7,
    PMD_MPD_GAINS               = 8,
    PMD_APDOI_CTRL              = 9,
    PMD_RSSI_A                  = 10, /* optic_power = a * rssi + b */
    PMD_RSSI_B                  = 11,
    PMD_RSSI_C                  = 12,
    PMD_TEMP_0                  = 13, /* calibration temperature */
    PMD_TEMP_COFF_H             = 14, /* Temperature coefficient high */
    PMD_TEMP_COFF_L             = 15, /* Temperature coefficient low */
    PMD_ESC_THR                 = 16,
    PMD_ROGUE_THR               = 17,
    PMD_LEVEL_0_DAC             = 18,
    PMD_AVG_LEVEL_1_DAC         = 19,
    PMD_DACRANGE                = 20,
    PMD_LOS_THR                 = 21,
    PMD_SAT_POS                 = 22,
    PMD_SAT_NEG                 = 23,
    PMD_EDGE_RATE               = 24,
    PMD_PREEMPHASIS_WEIGHT      = 25,
    PMD_PREEMPHASIS_DELAY       = 26,
    PMD_DUTY_CYCLE              = 27,
    PMD_MPD_CALIBCTRL           = 28,
    PMD_TX_POWER                = 29,
    PMD_BIAS0                   = 30,
    PMD_TEMP_OFFSET             = 31,
    PMD_BIAS_DELTA_I            = 32,
    PMD_SLOPE_EFFICIENCY        = 33,
    PMD_TEMP_TABLE              = 34,
    PMD_ADF_LOS_THRESHOLDS      = 35,
    PMD_ADF_LOS_LEAKY_BUCKET    = 36,
    PMD_COMPENSATION            = 37,

    PMD_NUM_OF_LEGACY_CAL_PARAM,      /* DO NOT CHANGE the legacy calibration parameters below. */
                                      /* These numbers are reserved and must be compatible to   */
                                      /* the old binary 'pmd_calibration' legacy file and to    */
                                      /* the pmd_calibration_legacy structure.                  */
                                      /* The latest legacy calibration file version is 7.       */
                                      /* You may add, remove and alter newer parameters. Newer  */
                                      /* calibration parameters are maintained in a JSON file.  */

    PMD_HOST_SW_CONTROL         = 60, /* backdoor */
    PMD_STATISTICS_COLLECTION   = 61, /* backdoor */
} pmd_calibration_parameters_index;

extern PMD_WAN_TYPES pmd_wan_type;

typedef struct
{
    uint32_t alarms;
    uint32_t sff;
} pmd_msg_addr;


#define CAL_MULT_LEN 2

typedef enum
{
    US_1G_INDEX,
    US_2G_INDEX,
} us_index;

typedef struct
{
    int32_t val;
    unsigned char valid;
} flash_int;

typedef struct
{
    int32_t val[CAL_MULT_LEN];
    unsigned char valid[CAL_MULT_LEN];
} flash_int_mult;

typedef struct
{
    int16_t val;
    unsigned char valid;
} flash_word;

typedef struct
{
    int16_t val[CAL_MULT_LEN];
    unsigned char valid[CAL_MULT_LEN];
} flash_word_mult;

#define TEMP_TABLE_SIZE 166
typedef struct
{
    uint32_t val[TEMP_TABLE_SIZE];
    unsigned char valid;
} flash_temp_table;

#define APD_TEMP_TABLE_SIZE 160
typedef struct 
{
    uint16_t val[APD_TEMP_TABLE_SIZE];
    unsigned char valid;
} temperature_to_apd_table;

typedef struct
{
    flash_int  watermark;       
    flash_int  version;
    flash_word level_0_dac;     /* used for first burst + CID tracking */
    flash_word level_1_dac;
    flash_word bias;
    flash_word mod;
    flash_word apd;
    flash_word mpd_config;
    flash_word mpd_gains;
    flash_word apdoi_ctrl;
    flash_int rssi_a_cal;      /* optic_power = a * rssi + b */
    flash_int rssi_b_cal;
    flash_int rssi_c_cal;
    flash_word temp_0;
    flash_word coff_h;    /* Temperate coefficient high */
    flash_word coff_l;    /* Temperate coefficient low */
    flash_word esc_th;
    flash_word rogue_th;
    flash_word avg_level_0_dac; /* used for force tracking */
    flash_word avg_level_1_dac;
    flash_word dacrange;
    flash_word los_thr;
    flash_word sat_pos;
    flash_word sat_neg;
    flash_word_mult edge_rate;
    flash_word_mult preemphasis_weight;
    flash_int_mult  preemphasis_delay;
    flash_word duty_cycle;
    flash_word calibctrl;
    flash_word tx_power;
    flash_word bias0;
    flash_word temp_offset;
    flash_word bias_delta_i;
    flash_word slope_efficiency;
    flash_temp_table temp_table;
    flash_int adf_los_thresholds;
    flash_word adf_los_leaky_bucket;
    flash_int compensation;
    temperature_to_apd_table temp2apd_table;
} pmd_calibration_parameters;

typedef struct
{
    /* PMD WAN type auto detection must have a valid file to detetct*/
    int is_calibration_file_valid;
    pmd_calibration_parameters calibration_parameters_from_json;
} pmd_wan_type_auto_detect_settings;


typedef void (*pmd_gpon_isr_callback)(void);
typedef int (*pmd_statistic_first_burst_callback)(uint16_t *bias, uint16_t *mod);
typedef int (*pmd_math_first_burst_model_callback)(uint16_t *bias, uint16_t *mod);
typedef void (*pon_set_pmd_fb_done_callback)(uint8_t state);

int pmd_dev_poll_epon_alarm(void);
void pmd_dev_change_tracking_state(uint32_t old_state, uint32_t new_state);
void pmd_dev_enable_prbs_or_misc_mode(uint16_t enable, uint8_t prbs_mode);
void pmd_dev_assign_pon_stack_callback(pon_set_pmd_fb_done_callback _pon_set_pmd_fb_done);
int pmd_dev_reconfigure_wan_type(PMD_WAN_TYPES new_pmd_wan_type,
    pmd_calibration_parameters *calibration_parameters_from_json);


#endif /* ! _PMD_H_ */
