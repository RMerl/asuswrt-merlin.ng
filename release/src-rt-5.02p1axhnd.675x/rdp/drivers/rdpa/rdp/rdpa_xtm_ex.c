/*
* <:copyright-BRCM:2015-2017:proprietary:standard
* 
*    Copyright (c) 2015-2017 Broadcom 
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

/*
 * rdpa_xtm_ex.c
 *
 *  Created on: 2017
 *      Author: srinies
 */

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_platform.h"
#include "rdp_drv_bbh.h"
#ifndef BDMF_SYSTEM_SIM
#include "rdd_ih_defs.h"
#include "rdd.h"
#endif
#define RDPA_XTM_FLOW_MAX_INDEX         3

int _cfg_ds_xtm_channel(bdmf_index channel_idx)
{
    return 0;
}

/* "us_cfg" attribute "write" callback. */
int rdpa_us_wan_flow_config(uint32_t      wan_flow,
                            int           wan_channel,
                            uint32_t      hdr_type,
                            uint32_t      wan_port_or_fstat,
                            bdmf_boolean  crc_calc,
                            int           ptm_bonding,
                            uint8_t       pbits_to_queue_table_index,
                            uint8_t       traffic_class_to_queue_table_index,
                            bdmf_boolean  enable)
{
   int rc = 0;

#ifndef BDMF_SYSTEM_SIM
       rc = rdd_us_wan_flow_config(wan_flow, (RDD_WAN_CHANNEL_ID)wan_channel, hdr_type,
                                   wan_port_or_fstat, crc_calc, ptm_bonding, pbits_to_queue_table_index, 
                                   traffic_class_to_queue_table_index);
       if (rc)
          BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "remove us xtmflow %d error\n", wan_flow);
#endif
    return rc;
}

int rdpa_wan_dsl_channel_base(void)
{
   return RDD_WAN0_CHANNEL_BASE;
}

int rdpa_flow_pm_counters_get(int index, rdpa_xtmflow_stat_t *stat)
{
   int rc;

   BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE rdd_flow_counters = {};

   /* US */
   rc = rdd_flow_pm_counters_get(index, BL_LILAC_RDD_FLOW_PM_COUNTERS_BOTH, 0, &rdd_flow_counters);

   if (rc)
      BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read RDD flow counters for xtmflow %d\n", index);

   stat->tx_packets = rdd_flow_counters.good_tx_packet;               
   stat->tx_bytes = rdd_flow_counters.good_tx_bytes;               
   stat->tx_packets_discard = rdd_flow_counters.error_tx_packets_discard;     

   return rc;
}

int rdpa_xtm_xtmchannel_id_to_tm_channel_id(int xtmchannel_id)
{
    return rdpa_wan_dsl_channel_base() + xtmchannel_id;
}

int rdpa_xtmflow_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED; /*not supported in RDP*/
}

