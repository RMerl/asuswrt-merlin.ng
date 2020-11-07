/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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
#ifndef _RDP_DRV_CNPL_H_
#define _RDP_DRV_CNPL_H_

#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_cnpl_ag.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CNPL_PERIOD_CYCLES                      8192
/* Quanta for 500MHZ (XRDP broadbus) -> 16382ns */
/* Quanta for 466MHZ (XRDP broadbus) -> 17579ns */
#define CNPL_PERIODIC_UPDATE_QUANTA_NS          (CNPL_PERIOD_CYCLES * 1000 / UBUS_SLV_FREQ_IN_MHZ) 
#define CNPL_PERIODIC_UPDATE_HALF_QUANTA_NS     (CNPL_PERIODIC_UPDATE_QUANTA_NS / 2)
#define CNPL_PERIODIC_UPDATE_MINIMUM            1

#define CNPL_SECOND_TO_US                       1000000
#define POLICER_TIMER_PERIOD                    (CNPL_SECOND_TO_US / CNPL_PERIODIC_UPDATE_US)
#define BUCKET_SIZE_RATE_MULT_MAX               15 

#define CNPL_READ_DONE                          0
#define CNPL_READ_PROCESS                       1
#define CNPL_READ_TIMEOUT                       0xffff
#define CNPL_READ_COUNTER_BUFFER                8
#define CNPL_READ_POLICER_BUFFER                2

#define CNPL_MAX_COUNTER_GROUPS                 16
#define CNPL_MAX_COUNTER_INDEX                  16384
#define CNPL_MAX_POLICER_INDEX                  96
#define CNPL_COMMAND_OFFSET                     30
#define CNPL_COUNTER_READ_COMMAND               1
#define CNPL_COUNTER_READ_COMMAND_GROUP_OFFSET  26
#define CNPL_COUNTER_READ_COMMAND_START_OFFSET  12
#define CNPL_COUNTER_READ_COMMAND_SIZE_OFFSET   4
#define CNPL_MAX_POLICER_GROUPS                 2
#define CNPL_POLICER_POLICE_COMMAND             2
#define CNPL_POLICER_COMMAND_GROUP_OFFSET       28
#define CNPL_POLICER_COMMAND_START_OFFSET       20
#define CNPL_POLICER_COMMAND_PACKET_LEN_OFFSET  4
#define CNPL_POLICER_READ_COMMAND               3
#define CNPL_POLICER_COMMAND_CLEAR_OFFSET       19

uint8_t drv_cnpl_periodic_update_us_to_n_get(uint32_t microseconds);
bdmf_error_t drv_cnpl_memory_data_init(void);
bdmf_error_t drv_cnpl_counter_read_command_set(uint8_t group, uint16_t start_counter, uint8_t num_of_counters);
bdmf_error_t drv_cnpl_counter_read_command_get(uint32_t *counters, uint8_t num_of_counters, bdmf_boolean cn_double, uint8_t cn0_byts, bdmf_boolean is_start_counter_odd);
bdmf_error_t drv_cnpl_policer_police_command_set(uint8_t group, uint8_t policer_num, uint16_t packet_len);
bdmf_error_t drv_cnpl_policer_read_command_get(void *policers, uint8_t num_of_policers, bdmf_boolean single);
bdmf_error_t drv_cnpl_policer_police(uint8_t *result, uint8_t group, uint8_t policer_num, uint16_t packet_len);
bdmf_error_t drv_cnpl_policer_read_command_set(uint8_t group, uint8_t policer_num, bdmf_boolean reset_after_read);
bdmf_error_t drv_cnpl_policer_read(void *policers, uint8_t group, uint8_t num_of_policers, bdmf_boolean reset);
bdmf_error_t drv_cnpl_counter_read(void *counters, uint8_t group, uint16_t start_counter, uint8_t num_of_counters);
bdmf_error_t drv_cnpl_counter_clr(uint8_t group, uint32_t cntr_id);
bdmf_error_t drv_cnpl_counter_set(uint8_t group, uint32_t cntr_id, uint8_t value);

#ifdef USE_BDMF_SHELL
int drv_cnpl_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_cnpl_cli_init(bdmfmon_handle_t driver_dir);
void drv_cnpl_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#ifdef __cplusplus
}
#endif

#endif
