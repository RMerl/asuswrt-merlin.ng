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


char *image_task_names[3][16] = {
	{"DS_TM_BUDGET_ALLOCATION", "DS_TM_UPDATE_FIFO", "DS_TM_FLUSH", "DS_TM_TX_TASK", "", "", "", "", "", "", "", "", "", "", "", ""},
	{"CPU_IF_1_WAN_DIRECT", "", "CPU_IF_1_CPU_RX_UPDATE_FIFO", "CPU_IF_1_INTERRUPT_COALESCING", "CPU_IF_1_CPU_RX_METER_BUDGET_ALLOCATOR", "", "", "", "", "", "", "", "CPU_IF_1_CPU_RECYCLE", "CPU_IF_1_CPU_RX_COPY", "", ""},
	{"CPU_IF_2_CPU_TX_UPDATE_FIFO_EGRESS", "CPU_IF_2_CPU_TX_UPDATE_FIFO_INGRESS", "CPU_IF_2_CPU_RECYCLE", "CPU_IF_2_CPU_TX_EGRESS", "CPU_IF_2_CPU_TX_INGRESS", "CPU_IF_2_REPORTING", "CPU_IF_2_INTERRUPT_COALESCING", "", "", "", "", "", "", "", "CPU_IF_2_TX_ABS_RECYCLE", ""}
	};

