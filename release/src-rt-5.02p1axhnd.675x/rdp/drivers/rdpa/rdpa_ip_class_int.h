/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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


#ifndef _RDPA_IP_CLASS_INT_H_
#define _RDPA_IP_CLASS_INT_H_

#include <bdmf_interface.h>
#include "rdpa_ip_class_basic.h"
#include "rdpa_cpu_basic.h"
#include "rdpa_ip_class.h"
#ifdef XRDP
#include "rdd_tuple_lkp.h"
#endif
#include "rdpa_flow_idx_pool.h"

#ifndef XRDP
struct rdp_v6_subnets
{
    bdmf_ipv6_t src;
    bdmf_ipv6_t dst;
    uint32_t refcnt; /* zero means subnet is not in use */
};
#endif

/* ip_class object private data */
typedef struct {
    uint32_t num_flows;     /**< Number of configured IP flows */
    rdpa_ip_class_method op_method; /** Operational method of the IP class */
    rdpa_l4_filter_cfg_t l4_filters[RDPA_MAX_L4_FILTERS]; /**< L4 filters configuration */
    uint32_t l4_filter_stat[RDPA_MAX_L4_FILTERS]; /**< L4 filter statistics shadow */
    bdmf_mac_t routed_mac[RDPA_MAX_ROUTED_MAC]; /**< Router mac address */
    rdpa_fc_bypass fc_bypass_mask;/**< mac mode bitmask */
    rdpa_key_type ip_key_type;/**< ip flow key type */
    bdmf_boolean tcp_ack_prio; /* TCP Ack prioritization (enable/disable) */
    rdpa_flow_idx_pool_t *flow_idx_pool_p;
    uint32_t *ctx_ext_idx;
} ip_class_drv_priv_t;

#define ECN_IN_TOS_SHIFT 2

int ip_class_attr_flow_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size);
int ip_class_attr_flow_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index,
    void *val, uint32_t size);
int ip_class_attr_flow_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index);
int ip_class_attr_flow_add_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size, bdmf_index *ctx_ext_index);
int ip_class_attr_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);
int ip_class_attr_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size);
int ip_class_attr_flow_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index);

int ip_class_attr_pathstat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);

int ip_class_attr_tcp_ack_prio_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);

#endif /* _RDPA_IP_CLASS_INT_H_ */
