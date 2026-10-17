/*
   Copyright (c) 2025 Broadcom
   All Rights Reserved

    <:label-BRCM:2025:DUAL/GPL:standard

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


char *image_task_names[11][16] = {
	{"", "", "", "", "", "", "", "", "IMAGE_0_PROCESSING0", "IMAGE_0_PROCESSING1", "IMAGE_0_PROCESSING2", "IMAGE_0_PROCESSING3", "IMAGE_0_PROCESSING4", "IMAGE_0_PROCESSING5", "IMAGE_0_PROCESSING6", "IMAGE_0_PROCESSING7"},
	{"", "", "", "", "", "", "", "IMAGE_1_DIRECT_FLOW", "IMAGE_1_PROCESSING0", "IMAGE_1_PROCESSING1", "IMAGE_1_PROCESSING2", "IMAGE_1_PROCESSING3", "IMAGE_1_PROCESSING4", "IMAGE_1_PROCESSING5", "IMAGE_1_PROCESSING6", "IMAGE_1_PROCESSING7"},
	{"", "", "", "", "", "", "", "IMAGE_2_SCORE_DATA_TRACE", "IMAGE_2_PROCESSING0", "IMAGE_2_PROCESSING1", "IMAGE_2_PROCESSING2", "IMAGE_2_PROCESSING3", "IMAGE_2_PROCESSING4", "IMAGE_2_PROCESSING5", "IMAGE_2_PROCESSING6", "IMAGE_2_PROCESSING7"},
	{"", "", "", "IMAGE_3_SPDSVC_ANALYZER", "IMAGE_3_SPDSVC_GEN", "IMAGE_3_TCPSPDTEST_DOWNLOAD", "IMAGE_3_TCPSPDTEST_UPLOAD", "IMAGE_3_TCPSPDTEST_UPLOAD_TIMER", "IMAGE_3_PROCESSING0", "IMAGE_3_PROCESSING1", "IMAGE_3_PROCESSING2", "IMAGE_3_PROCESSING3", "IMAGE_3_PROCESSING4", "IMAGE_3_PROCESSING5", "IMAGE_3_PROCESSING6", "IMAGE_3_PROCESSING7"},
	{"IMAGE_4_GENERAL_TIMER", "IMAGE_4_CPU_RECYCLE", "IMAGE_4_CPU_TX_0", "IMAGE_4_CPU_TX_1", "", "", "", "", "IMAGE_4_PROCESSING0", "IMAGE_4_PROCESSING1", "IMAGE_4_PROCESSING2", "IMAGE_4_PROCESSING3", "IMAGE_4_PROCESSING4", "IMAGE_4_PROCESSING5", "IMAGE_4_PROCESSING6", "IMAGE_4_PROCESSING7"},
	{"", "", "IMAGE_5_DHD_TX_COMPLETE_0", "IMAGE_5_DHD_TX_COMPLETE_1", "IMAGE_5_DHD_TX_COMPLETE_2", "IMAGE_5_DHD_RX_COMPLETE_0", "IMAGE_5_DHD_RX_COMPLETE_1", "IMAGE_5_DHD_RX_COMPLETE_2", "IMAGE_5_PROCESSING0", "IMAGE_5_PROCESSING1", "IMAGE_5_PROCESSING2", "IMAGE_5_PROCESSING3", "IMAGE_5_PROCESSING4", "IMAGE_5_PROCESSING5", "IMAGE_5_PROCESSING6", "IMAGE_5_PROCESSING7"},
	{"", "", "", "", "", "", "IMAGE_6_MULTICAST0", "IMAGE_6_MULTICAST1", "IMAGE_6_PROCESSING0", "IMAGE_6_PROCESSING1", "IMAGE_6_PROCESSING2", "IMAGE_6_PROCESSING3", "IMAGE_6_PROCESSING4", "IMAGE_6_PROCESSING5", "IMAGE_6_PROCESSING6", "IMAGE_6_PROCESSING7"},
	{"TM_LAN_UPDATE_RL", "TM_LAN_UPDATE_AQM", "", "", "TM_LAN_UPDATE_FIFO_GSU0", "TM_LAN_UPDATE_FIFO_GSU1", "TM_LAN_UPDATE_FIFO_GSU2", "TM_LAN_UPDATE_FIFO_GSU3", "TM_LAN_UPDATE_FIFO_GSU4", "TM_LAN_TX_GSU0", "TM_LAN_TX_GSU1", "TM_LAN_TX_GSU2", "TM_LAN_TX_GSU3", "TM_LAN_TX_GSU4", "", ""},
	{"TM_WAN_UPDATE_RL", "TM_WAN_UPDATE_AQM", "TM_WAN_OVL_BUDGET_ALLOCATION", "", "TM_WAN_UPDATE_FIFO_GSU0", "TM_WAN_TX_GSU0", "", "TM_WAN_WAN", "", "TM_WAN_CPU_FPM_BUFMNG_RELEASE", "", "", "", "", "", ""},
	{"TM_REPROC_UPDATE_RL", "TM_REPROC_UPDATE_AQM", "TM_REPROC_DHD_TIMER", "TM_REPROC_CPU_RX", "TM_REPROC_UPDATE_FIFO_GSU0", "TM_REPROC_UPDATE_FIFO_GSU1", "TM_REPROC_UPDATE_FIFO_GSU2", "TM_REPROC_CPU_RX_CSO", "TM_REPROC_DHD_TX_POST_UPDATE_FIFO", "TM_REPROC_TX_GSU0", "TM_REPROC_TX_GSU1", "TM_REPROC_TX_GSU2", "TM_REPROC_DHD_TX_POST_0", "TM_REPROC_DHD_TX_POST_1", "TM_REPROC_DHD_TX_POST_3", ""},
	{"TM_SERVICE_UPDATE_RL", "TM_SERVICE_UPDATE_AQM", "TM_SERVICE_SERVICE_QUEUES", "", "TM_SERVICE_UPDATE_FIFO_GSU0", "", "", "TM_SERVICE_TX_GSU0", "", "", "", "", "", "", "", ""}
	};

