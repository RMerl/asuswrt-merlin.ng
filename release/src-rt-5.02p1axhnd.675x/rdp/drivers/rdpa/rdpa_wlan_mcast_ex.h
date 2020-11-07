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

#ifndef _RDPA_WLAN_MCAST_EX_H_
#define _RDPA_WLAN_MCAST_EX_H_

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdpa_wlan_mcast.h"
#include "rdd_wlan_mcast_common.h"

/* mcast object private data */
typedef struct {
    uint32_t fwd_table_entries;
    uint32_t dhd_station_entries;
    uint32_t ssid_mac_address_entries;
    wlan_mcast_dhd_list_table_t dhd_list_table;
} wlan_mcast_drv_priv_t;

void wlan_mcast_destroy_ex(struct bdmf_object *mo);
int wlan_mcast_attr_fwd_table_read_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table);
int wlan_mcast_attr_fwd_table_write_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table);
int wlan_mcast_attr_fwd_table_add_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table);
int wlan_mcast_attr_fwd_table_delete_ex(struct bdmf_object *mo, bdmf_index index);

#ifdef XRDP
rdpa_wlan_mcast_fwd_table_t *__wlan_mcast_fwd_table_get(int idx);
#endif

#ifdef CONFIG_DHD_RUNNER

#define WLAN_MCAST_DHD_LIST_TABLE_SIZE sizeof(RDD_WLAN_MCAST_DHD_LIST_TABLE_DTS)

int wlan_mcast_attr_dhd_station_read_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station);
int wlan_mcast_attr_dhd_station_add_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station);
int wlan_mcast_attr_dhd_station_delete_ex(struct bdmf_object *mo, bdmf_index index);
int wlan_mcast_attr_dhd_station_find_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station);

int wlan_mcast_dhd_station_per_radio_cnt_read_ex(bdmf_index rdpa_fwd_table_index, uint8_t radio_index);
void __dhd_station_entry_set(RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *rdd_dhd_station,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station);
#endif

#endif /* _RDPA_WLAN_MCAST_EX_H_ */
