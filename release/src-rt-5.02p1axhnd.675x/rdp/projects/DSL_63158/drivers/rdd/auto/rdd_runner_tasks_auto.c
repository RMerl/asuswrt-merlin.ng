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


char *image_task_names[6][16] = {
	{"", "DS_TM_BUDGET_ALLOCATION", "DS_TM_UPDATE_FIFO", "DS_TM_TX_TASK", "DS_TM_FLUSH", "DS_TM_REPORTING", "DS_TM_SERVICE_QUEUES_BUDGET_ALLOCATOR", "DS_TM_SERVICE_QUEUES_UPDATE_FIFO", "DS_TM_SERVICE_QUEUES_TX", "", "", "", "", "", "", ""},
	{"", "IMAGE_1_CPU_RX_METER_BUDGET_ALLOCATOR", "IMAGE_1_INTERRUPT_COALESCING", "IMAGE_1_CPU_RX", "IMAGE_1_COMMON_REPROCESSING", "IMAGE_1_CPU_RECYCLE", "IMAGE_1_CPU_RX_COPY", "IMAGE_1_SPDSVC_GEN", "IMAGE_1_PROCESSING0", "IMAGE_1_PROCESSING1", "IMAGE_1_PROCESSING2", "IMAGE_1_PROCESSING3", "IMAGE_1_PROCESSING4", "IMAGE_1_PROCESSING5", "IMAGE_1_PROCESSING6", "IMAGE_1_PROCESSING7"},
	{"IMAGE_2_INTERRUPT_COALESCING", "IMAGE_2_CPU_RECYCLE", "", "", "", "", "IMAGE_2_CPU_TX_0", "IMAGE_2_CPU_TX_1", "IMAGE_2_PROCESSING0", "IMAGE_2_PROCESSING1", "IMAGE_2_PROCESSING2", "IMAGE_2_PROCESSING3", "IMAGE_2_PROCESSING4", "IMAGE_2_PROCESSING5", "IMAGE_2_PROCESSING6", "IMAGE_2_PROCESSING7"},
	{"US_TM_WAN_DIRECT", "US_TM_BUDGET_ALLOCATION", "US_TM_OVL_BUDGET_ALLOCATION", "US_TM_UPDATE_FIFO", "US_TM_FLUSH", "US_TM_EPON_UPDATE_FIFO", "", "US_TM_WAN_AE2P5", "US_TM_WAN_AE10", "US_TM_WAN_DSL", "US_TM_WAN_PON", "US_TM_WAN_EPON", "", "", "US_TM_WAN_TX_DDR_READ", "US_TM_WAN_TX_PSRAM_WRITE"},
	{"IMAGE_4_DHD_TIMER", "IMAGE_4_DHD_TX_POST_0", "IMAGE_4_DHD_TX_POST_1", "IMAGE_4_DHD_TX_POST_2", "IMAGE_4_DHD_TX_POST_UPDATE_FIFO", "IMAGE_4_DHD_MCAST", "", "", "IMAGE_4_0", "IMAGE_4_1", "IMAGE_4_2", "IMAGE_4_3", "IMAGE_4_4", "IMAGE_4_5", "IMAGE_4_6", "IMAGE_4_7"},
	{"PROCESSING_DHD_RX_COMPLETE_0", "PROCESSING_DHD_RX_COMPLETE_1", "PROCESSING_DHD_RX_COMPLETE_2", "PROCESSING_DHD_TX_COMPLETE_0", "PROCESSING_DHD_TX_COMPLETE_1", "PROCESSING_DHD_TX_COMPLETE_2", "", "", "PROCESSING_0", "PROCESSING_1", "PROCESSING_2", "PROCESSING_3", "PROCESSING_4", "PROCESSING_5", "PROCESSING_6", "PROCESSING_7"}
	};

