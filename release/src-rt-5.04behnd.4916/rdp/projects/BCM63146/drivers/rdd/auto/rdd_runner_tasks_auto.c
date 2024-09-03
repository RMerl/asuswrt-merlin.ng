/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/



/* This is an automated file. Do not edit its contents. */


#include "rdd_runner_tasks_auto.h"


char *image_task_names[5][16] = {
	{"IMAGE_0_GENERAL_TIMER", "IMAGE_0_CPU_RX", "IMAGE_0_CPU_RECYCLE", "IMAGE_0_CPU_RX_COPY", "IMAGE_0_SERVICE_QUEUES", "", "", "", "IMAGE_0_PROCESSING0", "IMAGE_0_PROCESSING1", "IMAGE_0_PROCESSING2", "IMAGE_0_PROCESSING3", "IMAGE_0_PROCESSING4", "IMAGE_0_PROCESSING5", "IMAGE_0_PROCESSING6", ""},
	{"IMAGE_1_GENERAL_TIMER", "IMAGE_1_CPU_RECYCLE", "", "", "", "", "IMAGE_1_CPU_TX_0", "IMAGE_1_CPU_TX_1", "IMAGE_1_PROCESSING0", "IMAGE_1_PROCESSING1", "IMAGE_1_PROCESSING2", "IMAGE_1_PROCESSING3", "IMAGE_1_PROCESSING4", "IMAGE_1_PROCESSING5", "IMAGE_1_PROCESSING6", "IMAGE_1_PROCESSING7"},
	{"IMAGE_2_DHD_TIMER", "IMAGE_2_DHD_TX_POST_UPDATE_FIFO", "IMAGE_2_DHD_TX_POST_0", "IMAGE_2_DHD_TX_POST_1", "IMAGE_2_DHD_TX_POST_2", "IMAGE_2_CPU_FPM_RINGS_AND_BUFMNG_REFILL", "", "", "IMAGE_2_PROCESSING0", "IMAGE_2_PROCESSING1", "IMAGE_2_PROCESSING2", "IMAGE_2_PROCESSING3", "IMAGE_2_PROCESSING4", "IMAGE_2_PROCESSING5", "IMAGE_2_PROCESSING6", "IMAGE_2_PROCESSING7"},
	{"", "", "", "IMAGE_4_GENERAL_TIMER", "IMAGE_4_UPDATE_FIFO", "IMAGE_4_TX_TASK_DS_0", "IMAGE_4_BUFFER_CONG_MGT", "", "", "IMAGE_4_COMMON_REPROCESSING", "IMAGE_4_SPDSVC_GEN", "IMAGE_4_TCPSPDTEST", "IMAGE_4_TCPSPDTEST_GEN", "IMAGE_4_SPDSVC_ANALYZER", "", ""},
	{"IMAGE_3_GENERAL_TIMER", "", "IMAGE_3_UPDATE_FIFO", "IMAGE_3_TX_TASK_US_0", "IMAGE_3_TX_TASK_US_1", "IMAGE_3_DHD_TX_COMPLETE_0", "IMAGE_3_DHD_TX_COMPLETE_1", "IMAGE_3_DHD_TX_COMPLETE_2", "IMAGE_3_PROCESSING0", "IMAGE_3_PROCESSING1", "IMAGE_3_PROCESSING2", "IMAGE_3_PROCESSING3", "IMAGE_3_DHD_RX_COMPLETE_0", "IMAGE_3_DHD_RX_COMPLETE_1", "IMAGE_3_DHD_RX_COMPLETE_2", "IMAGE_3_BUFFER_CONG_MGT"}
	};

