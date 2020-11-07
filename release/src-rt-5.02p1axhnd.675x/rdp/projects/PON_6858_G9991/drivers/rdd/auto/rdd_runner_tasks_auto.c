/*
   Copyright (c) 2015 Broadcom
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



/* This is an automated file. Do not edit its contents. */


#include "rdd_runner_tasks_auto.h"


char *image_task_names[5][16] = {
	{"DS_TM_FLOW_CONTROL", "DS_TM_BUDGET_ALLOCATION", "DS_TM_UPDATE_FIFO", "DS_TM_FLUSH", "DS_TM_SCHEDULING_TASK", "DS_TM_FRAG0", "DS_TM_FRAG1", "DS_TM_FRAG2", "DS_TM_FRAG3", "", "DS_TM_SYSTEM_PORT", "", "", "", "", ""},
	{"REASSEMBLY_G9991_0", "REASSEMBLY_G9991_1", "REASSEMBLY_G9991_2", "REASSEMBLY_G9991_3", "", "", "", "", "", "", "", "", "", "", "", ""},
	{"CPU_IF_2_REPORTING", "CPU_IF_2_CPU_RX_METER_BUDGET_ALLOCATOR", "CPU_IF_2_TIMER_COMMON", "CPU_IF_2_INTERRUPT_COALESCING", "CPU_IF_2_CPU_RECYCLE", "CPU_IF_2_CPU_RX_COPY", "CPU_IF_2_CPU_RX", "", "CPU_IF_2_CPU_TX_0", "CPU_IF_2_CPU_TX_1", "", "", "", "", "", ""},
	{"US_TM_WAN_DIRECT", "US_TM_BUDGET_ALLOCATION", "US_TM_OVL_BUDGET_ALLOCATION", "US_TM_UPDATE_FIFO", "US_TM_UPDATE_FIFO_EPON", "US_TM_FLUSH", "US_TM_WAN_0", "US_TM_WAN_1", "US_TM_WAN_EPON", "", "", "", "", "", "", ""},
	{"PROCESSING_0", "PROCESSING_1", "PROCESSING_2", "PROCESSING_3", "PROCESSING_4", "PROCESSING_5", "PROCESSING_6", "PROCESSING_7", "", "", "", "", "", "", "", ""}
	};

