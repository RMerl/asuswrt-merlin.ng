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


#ifndef _RDD_MAP_AUTO_H_
#define _RDD_MAP_AUTO_H_

typedef enum
{
	image_0_runner_image = 0,
	flow_control_runner_image = image_0_runner_image,
	reporting_runner_image = image_0_runner_image,
	ds_tm_runner_image = image_0_runner_image,
	us_tm_runner_image = image_0_runner_image,
	service_queues_runner_image = image_0_runner_image,
	wan_direct_runner_image = image_0_runner_image,
	image_1_runner_image = 1,
	processing0_runner_image = image_1_runner_image,
	cpu_rx_runner_image = image_1_runner_image,
	spdsvc_gen_runner_image = image_1_runner_image,
	pktgen_tx_runner_image = image_1_runner_image,
	tcpspdtest_engine_runner_image = image_1_runner_image,
	common_reprocessing_runner_image = image_1_runner_image,
	image_2_runner_image = 2,
	processing1_runner_image = image_2_runner_image,
	cpu_tx_runner_image = image_2_runner_image,
	dhd_tx_post_runner_image = image_2_runner_image,
	spdsvc_wlan_txpost_runner_image = image_2_runner_image,
	timer_common_runner_image = image_2_runner_image,
	dhd_complete_runner_image = image_2_runner_image,
	spdsvc_wlan_gen_runner_image = image_2_runner_image,
	runner_image_last = 2,
} rdp_runner_image_e;

extern rdp_runner_image_e rdp_core_to_image_map[];
#endif /* _RDD_MAP_AUTO_H_ */
