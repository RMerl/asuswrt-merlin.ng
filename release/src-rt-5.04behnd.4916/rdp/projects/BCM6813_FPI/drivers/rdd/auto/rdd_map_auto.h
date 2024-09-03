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


#ifndef _RDD_MAP_AUTO_H_
#define _RDD_MAP_AUTO_H_

typedef enum
{
	image_0_runner_image = 0,
	processing0_runner_image = image_0_runner_image,
	dhd_complete_runner_image = image_0_runner_image,
	service_queues_runner_image = image_0_runner_image,
	image_1_runner_image = 1,
	processing1_runner_image = image_1_runner_image,
	pktgen_tx_runner_image = image_1_runner_image,
	tcpspdtest_engine_runner_image = image_1_runner_image,
	spdsvc_gen_runner_image = image_1_runner_image,
	common_reprocessing_runner_image = image_1_runner_image,
	image_2_runner_image = 2,
	processing2_runner_image = image_2_runner_image,
	cpu_tx_runner_image = image_2_runner_image,
	image_3_runner_image = 3,
	processing3_runner_image = image_3_runner_image,
	cpu_rx_runner_image = image_3_runner_image,
	spu_request_runner_image = image_3_runner_image,
	spdsvc_analyzer_runner_image = image_3_runner_image,
	image_4_runner_image = 4,
	processing4_runner_image = image_4_runner_image,
	dhd_tx_post_runner_image = image_4_runner_image,
	spdsvc_wlan_txpost_runner_image = image_4_runner_image,
	spu_response_runner_image = image_4_runner_image,
	image_5_runner_image = 5,
	processing5_runner_image = image_5_runner_image,
	reporting_runner_image = image_5_runner_image,
	direct_flow_runner_image = image_5_runner_image,
	cpu_tx_mcore_runner_image = image_5_runner_image,
	image_7_runner_image = 7,
	processing7_runner_image = image_7_runner_image,
	flow_control_runner_image = image_7_runner_image,
	ds_tm_runner_image = image_7_runner_image,
	spdsvc_wlan_gen_runner_image = image_7_runner_image,
	image_6_runner_image = 6,
	processing6_runner_image = image_6_runner_image,
	us_tm_runner_image = image_6_runner_image,
	runner_image_last = 7,
} rdp_runner_image_e;

extern rdp_runner_image_e rdp_core_to_image_map[8];

#ifdef DBG_TASK_ENTRY_POINT
extern int rnr_image_first_task[8];
#endif

#endif /* _RDD_MAP_AUTO_H_ */
