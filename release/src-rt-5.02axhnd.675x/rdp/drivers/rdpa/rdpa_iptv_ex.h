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
* :>
*/

#ifndef _RDPA_IPTV_INT_H_
#define _RDPA_IPTV_INT_H_

#include <bdmf_interface.h>
#include "rdd.h"
#ifdef XRDP
#include "rdd_iptv.h"
#endif
#include "rdpa_int.h"
#include "rdpa_iptv.h"
#include "rdpa_ingress_class_int.h"

/***************************************************************************
 * iptv object type
 **************************************************************************/
/* Classification context entry used by IPTV channels. */
struct mcast_result_entry
{
    DLIST_ENTRY(mcast_result_entry) list;
    uint32_t mcast_result_idx; /* Channel result index in RDD */
    rdpa_ic_result_t mcast_result;
    int ref_cnt;
    bdmf_object_handle port_vlan_action[rdpa_if__number_of];
    int port_ref_cnt[rdpa_if__number_of];
    bdmf_boolean port_trap_to_host[rdpa_if__number_of];
};

typedef struct mcast_result_entry mcast_result_entry_t;
DLIST_HEAD(mcast_result_list_t, mcast_result_entry);

struct iptv_channel_list_entry
{
    DLIST_ENTRY(iptv_channel_list_entry) list;
    bdmf_index channel_idx; 
    mcast_result_entry_t *mcast_result;
};

typedef struct iptv_channel_list_entry iptv_channel_list_entry_t;
DLIST_HEAD(iptv_channel_list_t, iptv_channel_list_entry);

/* iptv object private data */
typedef struct
{
    rdpa_iptv_lookup_method lookup_method; /**< IPTV lookup method */
    rdpa_mcast_filter_method mcast_prefix_filter; /**< Multicast prefix filter method,
                        used for GPON mode to apply IPTV lookup method on non-multicast GEM ports. */
    rdpa_forward_action lookup_miss_action; /**< In case of IPTV lookup missed
                        will perform one of the following actions: drop/trap packets */
    int channels_in_use; /**< Number of channels in use */
    int channels_in_ddr; /**< Number of configired channels in DDR. Not in use in XRDP */
#if !defined(BCM63158)
    struct mcast_result_list_t mcast_result_list; /**< Classification contexts list */
#endif
#ifdef XRDP
    bdmf_boolean wlan_to_host; /**< If set, channels that contain WLAN egress port will be trapped to host */
#endif
    struct iptv_channel_list_t iptv_channel_list; /**< List of currently configured IPTV channels */
} iptv_drv_priv_t;

#define IPTV_IS_L2(iptv_cfg) ((iptv_cfg)->lookup_method == iptv_lookup_method_mac || \
    (iptv_cfg)->lookup_method == iptv_lookup_method_mac_vid)
#define IPTV_IS_VID_USED(iptv_cfg) ((iptv_cfg)->lookup_method == iptv_lookup_method_mac_vid || \
    (iptv_cfg)->lookup_method == iptv_lookup_method_group_ip_src_ip_vid)
#define IPTV_IS_SRC_USED(iptv_cfg) ((iptv_cfg)->lookup_method == iptv_lookup_method_group_ip_src_ip || \
    (iptv_cfg)->lookup_method == iptv_lookup_method_group_ip_src_ip_vid)

int rdpa_iptv_cfg_rdd_update_ex(iptv_drv_priv_t *iptv_cfg, iptv_drv_priv_t *new_iptv_cfg, bdmf_boolean post_init);
int rdpa_iptv_post_init_ex(void);
void rdpa_iptv_destroy_ex(void);

int rdpa_iptv_ic_result_add_ex(mcast_result_entry_t *entry);
void rdpa_iptv_ic_result_delete_ex(uint32_t mcast_result_idx, rdpa_traffic_dir dir);

int rdpa_iptv_rdd_entry_get_ex(uint32_t key_idx, rdd_iptv_entry_t *rdd_iptv_entry);
int rdpa_iptv_rdd_entry_add_ex(rdd_iptv_entry_t *rdd_iptv_entry, uint32_t *channel_idx);
int rdpa_iptv_rdd_entry_delete_ex(uint32_t channel_idx);
int rdpa_iptv_rdd_entry_search_ex(rdpa_iptv_channel_key_t *key, uint32_t *index);
#ifdef XRDP
int rdpa_iptv_result_entry_modify(uint32_t channel_idx, rdd_iptv_entry_t *rdd_iptv_entry);
#endif

int rdpa_iptv_stat_read_ex(struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);
int iptv_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
bdmf_error_t rdpa_iptv_channel_rdd_pm_stat_get_ex(bdmf_index channel_index, rdpa_stat_t *pm_stat);
bdmf_error_t rdpa_iptv_channel_mcast_result_val_to_str_ex(const void *val, char *sbuf,
    uint32_t _size, rdd_ic_context_t *rdd_classify_ctx, rdpa_ports ports);
bdmf_error_t rdpa_iptv_lkp_miss_action_write_ex(rdpa_forward_action new_lookup_miss_action);
#endif  /* _RDPA_IPTV_INT_H_ */

