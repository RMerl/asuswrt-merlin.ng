/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#ifndef _RDD_INIT_H
#define _RDD_INIT_H

typedef struct
{
    uint8_t *ddr0_runner_base_ptr;
    int is_basic;
#ifdef CONFIG_DHD_RUNNER
    RDD_DHD_HW_CONFIGURATION_DTS dhd_hw_config;
#endif    
    uint32_t fw_clang_dis;
} rdd_init_params_t;

int rdd_init(void);
void rdd_exit(void);
int rdd_data_structures_init(rdd_init_params_t *init_params, RDD_HW_IPTV_CONFIGURATION_DTS *iptv_hw_config);
int rdd_egress_port_to_broadcom_switch_port_init(rdd_vport_id_t vport, uint8_t broadcom_switch_port);
int rdd_broadcom_switch_ports_mapping_table_config(rdd_vport_id_t vport, uint8_t broadcom_switch_port);
int rdd_lookup_ports_mapping_table_config(rdd_vport_id_t vport, uint8_t lookup_port);
int rdd_lookup_ports_mapping_table_init(rdd_vport_id_t vport, uint8_t lookup_port);
int rdd_lookup_ports_mapping_table_get(rdd_vport_id_t vport, uint8_t *lookup_port);
int rdd_lookup_ports_mapping_table_restore(rdd_vport_id_t vport);
int rdd_us_queue_end_update_context(void);

#define DHD_TX_COMPLETE_0_THREAD_NUMBER   IMAGE_5_PROCESSING_DHD_TX_COMPLETE_0_THREAD_NUMBER
#define DHD_RX_COMPLETE_0_THREAD_NUMBER   IMAGE_5_PROCESSING_DHD_RX_COMPLETE_0_THREAD_NUMBER
#define DHD_TIMER_THREAD_NUMBER           IMAGE_4_IMAGE_4_DHD_TIMER_THREAD_NUMBER

#endif /* _RDD_INIT_H */
