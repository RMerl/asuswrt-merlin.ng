/*
* <:copyright-BRCM:2014-2015:proprietary:standard
* 
*    Copyright (c) 2014-2015 Broadcom 
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

#ifndef _RDPA_FILTER_EX_H_
#define _RDPA_FILTER_EX_H_

#include "bdmf_dev.h"
#include "rdpa_api.h"
#ifdef XRDP
#include "rdd_ingress_filter.h"
#endif
#include "rdpa_vlan_ex.h"

#define RDPA_FILTER_ETYPE_UDEFS_QUANT \
    (RDPA_FILTER_ETYPE_UDEF_INDX_MAX - RDPA_FILTER_ETYPE_UDEF_INDX_MIN + 1)
#define RDPA_FILTER_OUI_VALS_QUANT \
    (RDPA_FILTER_OUI_VAL_INDX_MAX - RDPA_FILTER_OUI_VAL_INDX_MIN + 1)

/* Private data */
typedef struct
{
    rdpa_filter_global_cfg_t global_cfg; /* not supported for XRDP */
    uint32_t oui_vals[rdpa_if__number_of][RDPA_FILTER_OUI_VALS_QUANT]; /* not supported for XRDP */
    rdpa_filter_tpid_vals_t tpid_vals; /* Not supported for XRDP */
    
    uint16_t etype_udefs[RDPA_FILTER_ETYPE_UDEFS_QUANT];

    rdpa_filter_ctrl_t entries[RDPA_FILTERS_QUANT][rdpa_if__number_of];

    uint16_t stats[RDPA_FILTERS_QUANT][2]; /* DS / US */
} filter_drv_priv_t;

#define RDPA_FILTER_VAL_DISABLE "disable"

/* LAN 0, LAN 1, LAN 2, LAN 3, LAN 4; WLAN 0, WLAN 1 SWITCH */
#define RDPA_FILTER_OUI_PORTS_QUANT 8 

#define RDPA_FILTER_OUI_VALS_QUANT \
    (RDPA_FILTER_OUI_VAL_INDX_MAX - RDPA_FILTER_OUI_VAL_INDX_MIN + 1)

#define RDPA_FILTER_OUI_VALS_SZ \
    (rdpa_if__number_of * RDPA_FILTER_OUI_VALS_QUANT)
#define RDPA_FILTER_ENTRIES_SZ (RDPA_FILTERS_QUANT * rdpa_if__number_of)
#define RDPA_FILTER_STATS_SZ (RDPA_FILTERS_QUANT * 2) /* US + DS */

#define RDPA_FILTER_ETYPE_DUMMY 0xFFFF
#define RDPA_FILTER_OUI_DUMMY 0xFFFFFF
#define RDPA_FILTER_TPID_DUMMY 0xFFFF

int filter_pre_init_ex(struct bdmf_object *mo);
int filter_post_init_ex(struct bdmf_object *mo);
void filter_destroy_ex(struct bdmf_object *mo);
int filter_attr_global_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int filter_attr_entry_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int filter_attr_stats_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);
int filter_attr_tpid_vals_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int filter_attr_oui_val_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);
int filter_attr_oui_val_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int filter_attr_oui_val_get_next_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index);
int filter_attr_oui_val_s_to_val_ex(struct bdmf_object *mo, struct bdmf_attr *ad, const char *sbuf, void *val,
    uint32_t size);
int filter_attr_etype_udef_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int filter_attr_stats_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
static inline int _filter_verify_oui_val_id(uint8_t val_id)
{
    return (val_id <= RDPA_FILTER_OUI_VAL_INDX_MAX) ? 1 : 0;
}

static inline rdpa_if _filter_map_port_mask_single(rdpa_ports mask)
{
    rdpa_if single = rdpa_if_first;

    while (rdpa_if_id(single) != mask)
        ++single;

    return single;
}

/* Profiles mgmt */
#ifdef RDP
#define NUM_OF_FILTER_PROFILES 6
#else
#define NUM_OF_FILTER_PROFILES 16
#endif
#define INVALID_PROFILE_IDX 0x3f /* 6 bits */

void _rdpa_filter_profiles_pre_init(void);

int ingress_filter_ctrl_cfg_get_next(rdpa_filter_ctrl_t *ingress_filters, bdmf_index *index);
int ingress_filter_ctrl_cfg_read(rdpa_filter_ctrl_t *ingress_filters, bdmf_index index, void *val);
int ingress_filter_ctrl_cfg_validate(bdmf_index index, void *val);
int ingress_filter_entry_set(rdpa_filter filter, struct bdmf_object *owner_obj,
    rdpa_filter_ctrl_t *owner_ctrl_table, rdpa_filter_ctrl_t *ctrl, uint8_t *profile_id);
int ingress_filter_entry_set_ex(filter_drv_priv_t *priv, rdpa_filter filter, struct bdmf_object *ower_obj,
    rdpa_filter_ctrl_t *ctrl_table, rdpa_filter_ctrl_t *ctrl, uint8_t *profile_id);

void rdpa_filter_obj_delete_notify_ex(struct bdmf_object *owner_obj);

#endif /* _RDPA_FILTER_EX_H_ */
