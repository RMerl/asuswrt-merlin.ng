/*
* <:copyright-BRCM:2018:proprietary:standard
*
*    Copyright (c) 2018 Broadcom
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
 :>
*/

#ifndef _RDPA_SPDTEST_COMMON_EX_H_
#define _RDPA_SPDTEST_COMMON_EX_H_

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_spdsvc.h"
#include "rdd.h"
#include "rdpa_spdtest_common.h"

#ifdef XRDP

#include "rdp_drv_sbpm.h"
#include "rdd_cpu_tx.h"
#include "rdd_cpu_tx.h"
#define MAX_NUM_OF_HDRS_PER_STREAM SBPM_MAX_NUM_OF_MCAST_REPLICATIONS

typedef struct {
    rdpa_spdtest_ref_pkt_t pkt[PKTGEN_TX_NUM_OF_DATA_PKTS];
    uint16_t sbpm_pkt_head_bn[PKTGEN_TX_NUM_OF_DATA_PKTS][MAX_NUM_OF_HDRS_PER_STREAM]; /* Buffer number of SBPM list head */
    uint16_t total_num_of_bns;
} spdt_ref_pkt_container_t;

int udpspdtest_tx_start(pbuf_t *pbuf, const rdpa_cpu_tx_info_t *info, RDD_CPU_TX_DESCRIPTOR_DTS *cpu_tx);

#else /* RDP */

typedef void *spdt_ref_pkt_container_t;

#endif

int flow_object_get(bdmf_object_handle *flow_obj, int *class_created_here);

int spdt_analyzer_flow_add(bdmf_object_handle flow_obj, rdpa_traffic_dir dir, rdpa_spdsvc_analyzer_t *analyzer_p);
int spdt_analyzer_flow_delete(bdmf_object_handle flow_obj, bdmf_index flow_idx);

int spdt_tx_ref_pkt_set(spdt_ref_pkt_container_t *container, bdmf_index index, rdpa_spdtest_ref_pkt_t *ref_pkt);
void spdt_tx_ref_pkts_free(spdt_ref_pkt_container_t *container);
void spdt_tx_defs_init(int thread_num);
void spdt_tx_fpm_ug_budget_set(uint32_t fpm_ug);
void spdt_tx_sbpm_bn_reset(spdt_ref_pkt_container_t *container);

int spdt_so_mark_to_test_type(uint32_t so_mark);

#endif

