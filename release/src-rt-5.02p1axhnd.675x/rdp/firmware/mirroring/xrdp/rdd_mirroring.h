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


#ifndef _RDD_MIRRORING_H
#define _RDD_MIRRORING_H

#include "rdd.h"
#include "rdd_common.h"
#include "rdpa_types.h"
#include "rdp_platform.h"

#if defined(BCM63158)
#define RDD_MIRRORING_GET_TM_LAN_TASK(lan_port) IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER
#elif !defined(G9991)
#define RDD_MIRRORING_GET_TM_LAN_TASK(lan_port) (IMAGE_0_DS_TM_LAN0_THREAD_NUMBER + lan_port)
#endif /* G9991 */
#define IS_MIRRORING_CFG(qm_queue) (qm_queue < QM_QUEUE_LAST + 1) 
typedef struct
{
    bbh_id_e src_tx_bbh_id;
    bdmf_boolean lan;
    bdmf_boolean wlan_radio_idx;
    uint16_t rx_dst_queue;
    uint16_t tx_dst_queue;
    rdd_vport_id_t  rx_dst_vport;
    rdd_vport_id_t  tx_dst_vport;
} rdd_mirroring_cfg_t;

/* API to RDPA level */
void rdd_mirroring_set(rdd_mirroring_cfg_t *rdd_mirroring_cfg);
void rdd_mirroring_set_rx(rdd_mirroring_cfg_t *rdd_mirroring_cfg);
void rdd_mirroring_set_tx(rdd_mirroring_cfg_t *rdd_mirroring_cfg);
void rdd_mirroring_set_tx_src_en(bdmf_boolean is_lan, int wlan_radio_idx, bbh_id_e src_tx_bbh_id, bdmf_boolean mirror_enable);
void rdd_mirror_tx_disable(void);

/* Init Function */
void rdd_mirroring_cfg_init(void);

#endif
