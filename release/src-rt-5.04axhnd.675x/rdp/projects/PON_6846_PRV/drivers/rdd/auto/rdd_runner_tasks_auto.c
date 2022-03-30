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


char *image_task_names[3][16] = {
	{"TM_DIRECT_FLOW", "TM_UPDATE_FIFO_US_EPON", "TM_WAN_EPON", "TM_UPDATE_FIFO_US", "TM_WAN", "TM_REPORTING", "TM_BUDGET_ALLOCATION_US", "TM_BUDGET_ALLOCATION_DS", "TM_OVL_BUDGET_ALLOCATION_US", "TM_UPDATE_FIFO_DS", "TM_TX_TASK_DS", "TM_FLUSH", "", "TM_BUDGET_ALLOCATOR", "TM_SERVICE_QUEUES_UPDATE_FIFO", "TM_SERVICE_QUEUES_TX"},
	{"", "PROCESSING_CPU_RX_METER_BUDGET_ALLOCATOR", "PROCESSING_INTERRUPT_COALESCING", "PROCESSING_CPU_RX", "PROCESSING_CPU_RECYCLE", "PROCESSING_CPU_RX_COPY", "", "", "PROCESSING_PROCESSING0", "PROCESSING_PROCESSING1", "PROCESSING_PROCESSING2", "PROCESSING_PROCESSING3", "PROCESSING_PROCESSING4", "PROCESSING_PROCESSING5", "", ""},
	{"IMAGE_2_INTERRUPT_COALESCING", "", "", "IMAGE_2_CPU_TX_0", "IMAGE_2_TIMER_COMMON", "IMAGE_2_CPU_RECYCLE", "", "", "IMAGE_2_PROCESSING0", "IMAGE_2_PROCESSING1", "IMAGE_2_PROCESSING2", "IMAGE_2_PROCESSING3", "IMAGE_2_PROCESSING4", "IMAGE_2_PROCESSING5", "", ""}
	};

