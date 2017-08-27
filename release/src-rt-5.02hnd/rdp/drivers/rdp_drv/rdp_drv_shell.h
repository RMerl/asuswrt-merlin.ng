/*
    <:copyright-BRCM:2015-2016:DUAL/GPL:standard
    
       Copyright (c) 2015-2016 Broadcom 
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
#ifndef _RDP_DRV_SHELL_H_
#define _RDP_DRV_SHELL_H_

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>

#define BBH_RX_ENABLE           1
#define BBH_RX_DISABLE          0
#define CPU_TRAFFIC_DISABLE     1
#define CPU_TRAFFIC_ENABLE      0
#define TRACE_TIMESTAMP_BIT_LEN 12
#define PROFILING_CYCLES_US_DEF 65
#define INVALID_PC              0xaaaa
typedef enum trace_event_type
{
    TRACE_EVENT_CS_ACTIVE_ACTIVE,
    TRACE_EVENT_CS_IDLE_ACTIVE,
    TRACE_EVENT_CS_ACTIVE_IDLE,
    TRACE_EVENT_SCHEDULER
} trace_event_type_t;

#define BBRX_EVENT(acc) (acc == 4 || acc == 5 || acc == 5) /*These are the BB related accelerator bits in the scheduler event*/

#define TASK_INVALID(task_num) (task_num == 16)

#define TASK_EXISTS(task_array, core, task_num) (task_array[rdp_core_to_image_map[core]][task_num] != 0)

void rdp_drv_shell_init(bdmfmon_handle_t driver_dir);
void rdp_drv_shell_exit(bdmfmon_handle_t driver_dir);
#endif
#endif
