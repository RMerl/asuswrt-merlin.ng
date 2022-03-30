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


char *image_task_names[8][16] = {
	{"DS_TM_BUDGET_ALLOCATION", "DS_TM_BUFFER_CONG_MGT", "DS_TM_UPDATE_FIFO", "DS_TM_TX_TASK", "DS_TM_FLUSH", "DS_TM_REPORTING", "", "", "", "", "", "", "", "", "", ""},
	{"", "IMAGE_1_CPU_RX_METER_BUDGET_ALLOCATOR", "IMAGE_1_INTERRUPT_COALESCING", "IMAGE_1_CPU_RX", "IMAGE_1_CPU_RECYCLE", "IMAGE_1_CPU_RX_COPY", "IMAGE_1_COMMON_REPROCESSING", "IMAGE_1_SPDSVC_GEN", "IMAGE_1_PROCESSING0", "IMAGE_1_PROCESSING1", "IMAGE_1_PROCESSING2", "IMAGE_1_PROCESSING3", "IMAGE_1_PROCESSING4", "IMAGE_1_PROCESSING5", "IMAGE_1_PROCESSING6", "IMAGE_1_PROCESSING7"},
	{"IMAGE_2_INTERRUPT_COALESCING", "IMAGE_2_CPU_RECYCLE", "IMAGE_2_TIMER_COMMON", "", "", "", "IMAGE_2_CPU_TX_0", "IMAGE_2_CPU_TX_1", "IMAGE_2_PROCESSING0", "IMAGE_2_PROCESSING1", "IMAGE_2_PROCESSING2", "IMAGE_2_PROCESSING3", "IMAGE_2_PROCESSING4", "IMAGE_2_PROCESSING5", "IMAGE_2_PROCESSING6", "IMAGE_2_PROCESSING7"},
	{"US_TM_DIRECT_FLOW", "US_TM_BUDGET_ALLOCATION", "US_TM_OVL_BUDGET_ALLOCATION", "US_TM_BUFFER_CONG_MGT", "US_TM_UPDATE_FIFO_EPON", "US_TM_WAN_EPON", "US_TM_UPDATE_FIFO", "US_TM_WAN", "US_TM_FLUSH", "", "", "", "", "", "", ""},
	{"IMAGE_4_DHD_TX_COMPLETE_0", "IMAGE_4_DHD_TX_COMPLETE_1", "IMAGE_4_DHD_TX_COMPLETE_2", "IMAGE_4_DHD_RX_COMPLETE_0", "IMAGE_4_DHD_RX_COMPLETE_1", "IMAGE_4_DHD_RX_COMPLETE_2", "", "", "IMAGE_4_0", "IMAGE_4_1", "IMAGE_4_2", "IMAGE_4_3", "IMAGE_4_4", "IMAGE_4_5", "IMAGE_4_6", "IMAGE_4_7"},
	{"PROCESSING_SERVICE_QUEUES_UPDATE_FIFO", "PROCESSING_SERVICE_QUEUES_TX", "PROCESSING_BUDGET_ALLOCATION_SERVICE_QUEUES", "", "", "", "", "", "PROCESSING_0", "PROCESSING_1", "PROCESSING_2", "PROCESSING_3", "PROCESSING_4", "PROCESSING_5", "PROCESSING_6", "PROCESSING_7"},
	{"IMAGE_6_DHD_TIMER", "IMAGE_6_DHD_TX_POST_UPDATE_FIFO", "IMAGE_6_DHD_TX_POST_0", "IMAGE_6_DHD_TX_POST_1", "IMAGE_6_DHD_TX_POST_2", "", "", "", "IMAGE_6_0", "IMAGE_6_1", "IMAGE_6_2", "IMAGE_6_3", "IMAGE_6_4", "IMAGE_6_5", "IMAGE_6_6", "IMAGE_6_7"},
	{"", "", "", "", "", "", "", "", "IMAGE_7_0", "IMAGE_7_1", "IMAGE_7_2", "IMAGE_7_3", "IMAGE_7_4", "IMAGE_7_5", "IMAGE_7_6", "IMAGE_7_7"}
	};

