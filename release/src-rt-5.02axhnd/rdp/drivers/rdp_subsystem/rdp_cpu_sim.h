/*
    <:copyright-BRCM:2013-2016:DUAL/GPL:standard
    
       Copyright (c) 2013-2016 Broadcom 
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

#ifndef _RDP_CPU_SIM_H_
#define _RDP_CPU_SIM_H_

#ifdef RDP_SIM
#include <stdio.h>
#include "rdpa_types.h"
#include "rdd_defs.h"
#include "bdmf_session.h"
#include "bdmf_shell.h"
#include "rdp_cpu_sim_if_auto.h"

extern FILE *g_cpu_rx_file;
extern bdmf_session_handle g_cpu_rx_file_session;

void cpu_runner_sim_send_data(uint32_t length, char *buffer);
uint32_t cpu_runner_sim_receive_data(char *buffer);
uint32_t cpu_runner_sim_get_msg_length(sw2hw_msg *msg);
int cpu_runner_sim_connect(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void rdp_cpu_qm_req(uint32_t *cpu_tx_descriptor, uint16_t qm_queue_num);
int rdp_cpu_fpm_alloc(uint32_t packet_len, uint32_t *buff_num);
void rdp_cpu_fpm_free(uint32_t buffer_num);
void rdp_cpu_runner_wakeup(uint32_t runner_id, uint32_t task_id);
int rdp_cpu_counter_read(uint32_t group_id, uint32_t start_counter, uint32_t* cntr_arr, uint32_t num_of_counters, bdmf_boolean* cntr_double, uint8_t* cn0_bytes);
int cpu_sim_rx_file_write(bdmf_session_handle session, const char *buf, uint32_t size);
#endif
#endif
