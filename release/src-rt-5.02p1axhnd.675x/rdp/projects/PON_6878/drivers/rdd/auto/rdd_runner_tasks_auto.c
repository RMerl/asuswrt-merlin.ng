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
	{"TM_WAN_DIRECT", "TM_UPDATE_FIFO_US_EPON", "TM_WAN_EPON", "TM_UPDATE_FIFO_US", "TM_WAN", "TM_REPORTING", "TM_BUDGET_ALLOCATION_US", "TM_BUDGET_ALLOCATION_DS", "TM_OVL_BUDGET_ALLOCATION_US", "TM_UPDATE_FIFO_DS", "TM_TX_TASK_DS", "TM_FLUSH", "", "TM_BUDGET_ALLOCATOR", "TM_SERVICE_QUEUES_UPDATE_FIFO", "TM_SERVICE_QUEUES_TX"},
	{"", "PROCESSING_CPU_RX_METER_BUDGET_ALLOCATOR", "PROCESSING_INTERRUPT_COALESCING", "PROCESSING_CPU_RX", "PROCESSING_CPU_RECYCLE", "PROCESSING_CPU_RX_COPY", "PROCESSING_COMMON_REPROCESSING", "PROCESSING_SPDSVC_GEN", "PROCESSING_PROCESSING0", "PROCESSING_PROCESSING1", "PROCESSING_PROCESSING2", "PROCESSING_PROCESSING3", "PROCESSING_PROCESSING4", "PROCESSING_PROCESSING5", "", ""},
	{"IMAGE_2_INTERRUPT_COALESCING", "IMAGE_2_DHD_TX_COMPLETE_0", "IMAGE_2_DHD_RX_COMPLETE_0", "IMAGE_2_CPU_TX_0", "IMAGE_2_TIMER_COMMON", "IMAGE_2_CPU_RECYCLE", "IMAGE_2_DHD_TIMER", "IMAGE_2_DHD_TX_POST_0", "IMAGE_2_PROCESSING0", "IMAGE_2_PROCESSING1", "IMAGE_2_PROCESSING2", "IMAGE_2_PROCESSING3", "IMAGE_2_PROCESSING4", "IMAGE_2_PROCESSING5", "IMAGE_2_DHD_TX_POST_UPDATE_FIFO", "IMAGE_2_DHD_MCAST"}
	};

