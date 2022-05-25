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
	{"IMAGE_0_DHD_TX_COMPLETE_0", "IMAGE_0_DHD_TX_COMPLETE_1", "IMAGE_0_DHD_TX_COMPLETE_2", "IMAGE_0_DHD_RX_COMPLETE_0", "IMAGE_0_DHD_RX_COMPLETE_1", "IMAGE_0_DHD_RX_COMPLETE_2", "IMAGE_0_SPDSVC_ANALYZER", "", "IMAGE_0_PROCESSING0", "IMAGE_0_PROCESSING1", "IMAGE_0_PROCESSING2", "IMAGE_0_PROCESSING3", "IMAGE_0_PROCESSING4", "IMAGE_0_PROCESSING5", "IMAGE_0_PROCESSING6", "IMAGE_0_PROCESSING7"},
	{"IMAGE_1_GENERAL_TIMER", "IMAGE_1_COMMON_REPROCESSING", "IMAGE_1_CPU_RX", "IMAGE_1_CPU_RECYCLE", "IMAGE_1_CPU_RX_COPY", "IMAGE_1_SPDSVC_GEN", "IMAGE_1_TCPSPDTEST", "IMAGE_1_TCPSPDTEST_GEN", "IMAGE_1_PROCESSING0", "IMAGE_1_PROCESSING1", "IMAGE_1_PROCESSING2", "IMAGE_1_PROCESSING3", "IMAGE_1_PROCESSING4", "IMAGE_1_PROCESSING5", "IMAGE_1_PROCESSING6", "IMAGE_1_PROCESSING7"},
	{"IMAGE_2_GENERAL_TIMER", "", "IMAGE_2_CPU_RECYCLE", "IMAGE_2_CPU_TX_0", "IMAGE_2_CPU_TX_1", "IMAGE_2_SERVICE_QUEUES_UPDATE_FIFO", "IMAGE_2_SERVICE_QUEUES_TX", "IMAGE_2_BUDGET_ALLOCATION_SERVICE_QUEUES", "IMAGE_2_PROCESSING0", "IMAGE_2_PROCESSING1", "IMAGE_2_PROCESSING2", "IMAGE_2_PROCESSING3", "IMAGE_2_PROCESSING4", "IMAGE_2_PROCESSING5", "IMAGE_2_PROCESSING6", "IMAGE_2_PROCESSING7"},
	{"IMAGE_3_DHD_TIMER", "IMAGE_3_DHD_TX_POST_UPDATE_FIFO", "IMAGE_3_DHD_TX_POST_0", "IMAGE_3_DHD_TX_POST_1", "IMAGE_3_DHD_TX_POST_2", "", "", "", "IMAGE_3_PROCESSING0", "IMAGE_3_PROCESSING1", "IMAGE_3_PROCESSING2", "IMAGE_3_PROCESSING3", "IMAGE_3_PROCESSING4", "IMAGE_3_PROCESSING5", "IMAGE_3_PROCESSING6", "IMAGE_3_PROCESSING7"},
	{"DS_TM_BUDGET_ALLOCATION", "DS_TM_BUFFER_CONG_MGT", "DS_TM_UPDATE_FIFO", "DS_TM_TX_TASK", "DS_TM_REPORTING", "", "", "", "DS_TM_PROCESSING0", "DS_TM_PROCESSING1", "DS_TM_PROCESSING2", "DS_TM_PROCESSING3", "", "", "", ""},
	{"US_TM_DIRECT_FLOW", "US_TM_BUDGET_ALLOCATION", "US_TM_OVL_BUDGET_ALLOCATION", "US_TM_BUFFER_CONG_MGT", "US_TM_UPDATE_FIFO_EPON", "US_TM_WAN_EPON", "US_TM_UPDATE_FIFO", "US_TM_WAN", "", "", "", "", "", "", "", ""}
	};

