/*
<:copyright-BRCM:2012:proprietary:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

/*
 *******************************************************************************
 * File Name  : pktrunner_proto.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the Runner packet Driver.
 *******************************************************************************
 * Todo:
 *   GRE support: fcache.c:_fc_ipproto( ip_protocol ) + rxTupleV4.ports = GRE_PORT
 *   priority and queues
 *   ...
 */

#include <fcachehw.h>
//#include <bcmnet.h> /* SKBMARK_GET_Q_PRIO */
#include <bcmenet_common.h>
#include <linux/blog.h>
#include <linux/bcm_log.h>
#include <linux/blog_rule.h>
#include <linux/if.h>
#include "rdpa_api.h"
#include "fcachehw.h"
#include "rdpa_mw_blog_parse.h"
#include "linux/bcm_skb_defines.h"
#include "rdpa_mw_qos.h"
#ifndef CONFIG_BCM_FTTDP_G9991
#include "pktrunner_wlan.h"
#include "pktrunner_wlan_mcast.h"
#endif
#include "pktrunner_proto.h"
#ifdef XRDP
#include "pktrunner_l2_common.h"
#endif
#include "idx_pool_util.h"
#if !defined(BCM_FCACHE_IP_CLASS_BYPASS)
#include <rdpa_flow_idx_pool.h>
#endif
#include <bcmdpi.h>


// #define MCAST_FORWARD_TO_HOST

#ifndef CONFIG_BCM_FTTDP_G9991
void build_gre_tunnel_header(Blog_t *blog_p, rdpa_tunnel_cfg_t *rdpa_tunnel);
void build_dslite_tunnel_header(Blog_t *blog_p, rdpa_tunnel_cfg_t *rdpa_tunnel);
#endif

typedef union {
    bdmf_number    num;
    rdpa_stat_t    rdpastat;
} pktRunner_flowStat_t;

/* TODO: Get rid of this DLL struct (worst case memory = 8+8+4+4+4 = 28 * 1024 = 28KB )
 * Move this to pktRunner_data_t (similar to UCAST in DSL (worst case memory = 4+4 = 8KB) */
struct mcast_hw_channel_key_entry
{
    DLIST_ENTRY(mcast_hw_channel_key_entry) list;
    uint16_t key;
    uint32_t request_idx; /* Channel index in RDD */
    int ref_cnt[MAX_NUM_VLAN_TAG + 1]; /* ref cnt per tag */
};

#if !defined(BCM_FCACHE_IP_CLASS_BYPASS)
static rdpa_flow_idx_pool_t rdpa_shared_flw_idx_pool_g;  /* shared flow index pool as such has no relation 
                                                          * with pktRunner but needed so can be shared across objects */
#endif

typedef struct mcast_hw_channel_key_entry mcast_hw_channel_key_entry_t;
DLIST_HEAD(mcast_hw_channel_key_list_t, mcast_hw_channel_key_entry);
    
static uint16_t key_last_used = 0;
struct mcast_hw_channel_key_list_t mcast_hw_channel_key_list;

typedef struct
{
    bdmf_ip_t dst_ip;
    bdmf_ip_t src_ip;
    uint32_t prot_key;
} tunnel_key;

struct tunnel_entry
{
    tunnel_key key;
    bdmf_object_handle p_tunnel;
};


static struct tunnel_entry tunnel[MAX_TUNNELS_NUM] = {};

bdmf_object_handle ip_class = NULL;
bdmf_object_handle system_obj = NULL;
static bdmf_object_handle iptv = NULL;
#if defined(XRDP)
static bdmf_object_handle l2_class = NULL;
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
#define MAX_NUM_OF_RADIOS 3
static bdmf_object_handle dhd_helper_obj[MAX_NUM_OF_RADIOS] = {};
#endif  
#endif


typedef struct
{
    uint32_t status;        /* status: Enable=1 or Disable=0         */
    uint32_t activates;     /* number of successful flow activations */
    uint32_t failures;      /* number of flow activation failures    */
    uint32_t deactivates;   /* number of flow deactivations          */
    uint32_t hw_add_fail;   /* number of hash collision              */
    uint32_t no_free_idx;   /* number of no_free_idx errors          */
    uint32_t flushes;       /* number of flow clear calls            */
    uint32_t active;        /* number of currently active flows      */
} pktRunner_state_t;

/* PktRunner Accelerator instance data */
typedef struct {
    uint32_t            max_flow_idxs;      /* Max number of flows supported by this accelerator */
    pktRunner_state_t   state;
    IdxPool_t           idx_pool;           /* pktRunner index pool */
    uint32_t            *rdpa_flow_key_p;   /* mapped RDPA Flow ID (opaque 32bit value) */
    bdmf_number         *flowResetStats_p;  /* Flow Reset Stats */
}pktRunner_data_t;

typedef union {
    struct {
        BE_DECL(
            uint32_t unused:6;
            uint32_t accel:2;
            uint32_t flow_type:4;
            uint32_t flow_idx:20;
        )
        LE_DECL(
            uint32_t flow_idx:20;
            uint32_t flow_type:4;
            uint32_t accel:2;
            uint32_t unused:6;
        )
    };
    uint32_t word;
}PktRunnerFlowKey_t;

static inline int __pktRunnerFlowIdx(uint32_t pktRunner_Key)
{
    return ((PktRunnerFlowKey_t*)&pktRunner_Key)->flow_idx;
}
static inline int __pktRunnerFlowType(uint32_t pktRunner_Key)
{
    return ((PktRunnerFlowKey_t*)&pktRunner_Key)->flow_type;
}
static inline int __pktRunnerAccelIdx(uint32_t pktRunner_Key)
{
    return ((PktRunnerFlowKey_t *)&pktRunner_Key)->accel;
}

static inline uint32_t __pktRunnerBuildKey(uint32_t accel, uint32_t flow_type, uint32_t flow_idx)
{
    PktRunnerFlowKey_t key = {.word = 0 };
    key.accel = accel;
    key.flow_type = flow_type;
    key.flow_idx = flow_idx;

    return key.word;
}

static pktRunner_data_t  pktRunner_data_g[PKTRNR_MAX_FHW_ACCEL];    /* Protocol layer global context */

#define PKTRUNNER_DATA(accel)                   (pktRunner_data_g[accel])
#define PKTRUNNER_STATE(accel)                  (PKTRUNNER_DATA(accel).state)
#define PKTRUNNER_STATS(accel, idx)             (PKTRUNNER_DATA(accel).flowResetStats_p[idx])
#define PKTRUNNER_RDPA_KEY(accel, idx)          (PKTRUNNER_DATA(accel).rdpa_flow_key_p[idx])

static FC_CLEAR_HOOK fc_clear_hook_runner = (FC_CLEAR_HOOK)NULL;

/* XXX: if possible, merge with bcmenet bcmtag code */
#pragma pack(push,1)
typedef struct
{
   uint32_t opcode :3 __PACKING_ATTRIBUTE_FIELD_LEVEL__; //opcode for ingress traffic is allways 1
   uint32_t tc :3 __PACKING_ATTRIBUTE_FIELD_LEVEL__; //traffic class for the ingress traffic
   uint32_t te :2 __PACKING_ATTRIBUTE_FIELD_LEVEL__; //tag enforcement at switch ingress
#define D_BCM_TAG_TE_NO_ENFORCE 0
#define D_BCM_TAG_TE_UNTAG_ENFORCE 1
#define D_BCM_TAG_TE_TAG_ENFORCE 2
   uint32_t ts :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ; //timestamp request    
   uint32_t reserved :14 __PACKING_ATTRIBUTE_FIELD_LEVEL__; //nothing
   uint32_t dst_port :9 __PACKING_ATTRIBUTE_FIELD_LEVEL__; //destination port bitmap
} S_INGRESS_BCM_SWITCH_TAG;
#pragma pack(pop)

static S_INGRESS_BCM_SWITCH_TAG localBcmTag = {.opcode=1,.tc=0,.te=0,.ts=0,.reserved=0,.dst_port=0};

#if defined CC_PKTRUNNER_PROCFS
uint32_t pktRunnerGetState(struct seq_file *sf, uint32_t accel)
{
    uint32_t bytes = 0;

    if (accel == 0)
    {
      bytes += seq_printf(sf, "RUNNER-UCAST:\n");
    }
    else
    {
      bytes += seq_printf(sf, "RUNNER-MCAST:\n");
    }
    bytes += seq_printf(sf, "status         : %u\n",PKTRUNNER_STATE(accel).status);
    bytes += seq_printf(sf, "active         : %u\n",PKTRUNNER_STATE(accel).active);
    bytes += seq_printf(sf, "max_flows      : %u\n",PKTRUNNER_DATA(accel).max_flow_idxs);
    bytes += seq_printf(sf, "activates      : %u\n",PKTRUNNER_STATE(accel).activates);
    bytes += seq_printf(sf, "deactivates    : %u\n",PKTRUNNER_STATE(accel).deactivates);
    bytes += seq_printf(sf, "failures       : %u\n",PKTRUNNER_STATE(accel).failures);
    bytes += seq_printf(sf, "hw_add_fail    : %u\n",PKTRUNNER_STATE(accel).hw_add_fail);
    bytes += seq_printf(sf, "no_free_idx    : %u\n",PKTRUNNER_STATE(accel).no_free_idx);
    bytes += seq_printf(sf, "flushes        : %u\n",PKTRUNNER_STATE(accel).flushes);
    bytes += seq_printf(sf, "Flow_idx_in_use: %u\n",idx_pool_num_in_use(&PKTRUNNER_DATA(accel).idx_pool));
    bytes += seq_printf(sf, "----------------------------\n");
    return bytes;
}
#endif /* CC_PKTRUNNER_PROCFS */

#define REQUEST_KEY_TAG_NUM_VALIDATE(vtag_num, ret) do      \
{                                                           \
    if (vtag_num > MAX_NUM_VLAN_TAG)                        \
    {                                                       \
        protoError("vtag_num<%d> out of range!", vtag_num); \
        return ret;                                         \
    }                                                       \
                                                            \
}while(0)

static inline mcast_hw_channel_key_entry_t *request_key_find(uint16_t key)
{
    mcast_hw_channel_key_entry_t *entry, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &mcast_hw_channel_key_list, list, tmp_entry)
    {
        if (entry->key == key)
            return entry;
    }

    return NULL;
}

/* returns request_idx from key */
static bdmf_index request_key_val_get(uint16_t key)
{
    mcast_hw_channel_key_entry_t *entry = request_key_find(key);
    bdmf_index index = BDMF_INDEX_UNASSIGNED;
    
    if (entry)
    {
        index = entry->request_idx;
    }

    return index;
}

static int request_key_refcnt_get(uint16_t key, uint8_t vtag_num)
{
    mcast_hw_channel_key_entry_t *entry = request_key_find(key);
    int ref_cnt = 0;

    REQUEST_KEY_TAG_NUM_VALIDATE(vtag_num, 0);
    
    if (entry)
    {
        ref_cnt = entry->ref_cnt[vtag_num];
    }

    return ref_cnt;
}

static bdmf_index request_key_del(uint16_t key, uint8_t vtag_num)
{
    mcast_hw_channel_key_entry_t *entry = request_key_find(key);
    bdmf_index index = BDMF_INDEX_UNASSIGNED;
    int i = 0, ref_cnt_total = 0;

    REQUEST_KEY_TAG_NUM_VALIDATE(vtag_num, BDMF_INDEX_UNASSIGNED);

    if (entry)
    {
        index = entry->request_idx;
        entry->ref_cnt[vtag_num]--;

        for (i = 0; i < (MAX_NUM_VLAN_TAG + 1); i++)
        {            
            ref_cnt_total += entry->ref_cnt[i];
        }

        if (ref_cnt_total == 0)
        {
            DLIST_REMOVE(entry, list);
            bdmf_free(entry);
        }
    }

    return index;
}

static uint16_t request_key_add(uint32_t request_idx, uint8_t vtag_num)
{
    mcast_hw_channel_key_entry_t *entry, *tmp_entry;
    uint16_t try_num = 0;

    REQUEST_KEY_TAG_NUM_VALIDATE(vtag_num, BDMF_INDEX_UNASSIGNED);

    /* search if request is already recorded, and return its key */
    DLIST_FOREACH_SAFE(entry, &mcast_hw_channel_key_list, list, tmp_entry)
    {
        if (entry->request_idx == request_idx)
        {
            entry->ref_cnt[vtag_num]++;
            return entry->key;
        }
    }

find_next_free:
    if (try_num++ == PKTRUNNER_MAX_FLOWS_MCAST)
        return BDMF_INDEX_UNASSIGNED;
	
    if (++key_last_used == PKTRUNNER_MAX_FLOWS_MCAST)
        key_last_used = 0;

    /* find next availible key */
    DLIST_FOREACH_SAFE(entry, &mcast_hw_channel_key_list, list, tmp_entry)
    {
        if (entry->key == key_last_used)
	    goto find_next_free;
    }
    
    entry = bdmf_alloc(sizeof(mcast_hw_channel_key_entry_t));
    if (!entry)
        return BDMF_INDEX_UNASSIGNED;

    memset(entry, 0, sizeof(mcast_hw_channel_key_entry_t));
    entry->request_idx = request_idx;
    entry->key = key_last_used;
    entry->ref_cnt[vtag_num] = 1;
    DLIST_INSERT_HEAD(&mcast_hw_channel_key_list, entry, list);

    return key_last_used;
}

static void pktrunner_flow_tunnels_destroy(void)
{
    uint8_t i;

    for (i=0; i < MAX_TUNNELS_NUM; i++)
    {
        if (tunnel[i].p_tunnel)
        {
            bdmf_destroy(tunnel[i].p_tunnel);
            memset(&tunnel[i], 0, sizeof(struct tunnel_entry));
        }
    }
}

static void pktrunner_flow_remove_all(void)
{
    if (ip_class)
    {
        rdpa_ip_class_flush_set(ip_class, 1);
    }
#if defined(XRDP)
    if (l2_class)
    {
        rdpa_l2_class_flush_set(l2_class, 1);
    }
#endif
    pktrunner_flow_tunnels_destroy();
    if (iptv)
        rdpa_iptv_flush_set(iptv, 1);
}

#if defined(XRDP)
static bdmf_boolean is_ecn_remark_set(void)
{
    rdpa_system_cfg_t system_cfg;
    int rc;
    bdmf_boolean is_ecn_remark = 0;

    if ((rc = rdpa_system_cfg_get(system_obj, &system_cfg) == BDMF_ERR_OK))
    {
    	is_ecn_remark = (system_cfg.options == 2);
    }
    return is_ecn_remark;
}
#endif

int runnerResetStats(uint32_t pktrunner_key)
{
    int           rc = -1;
    pktRunner_flowStat_t flow_stats;
    bdmf_index    idx = __pktRunnerFlowIdx(pktrunner_key);
    uint32_t      flow_type = __pktRunnerFlowType(pktrunner_key);
    uint32_t      accel = __pktRunnerAccelIdx(pktrunner_key);

    if (idx < 0 || idx >= PKTRUNNER_DATA(accel).max_flow_idxs)
    {
        return 1;
    }

    switch (flow_type) {
        case PKTRNR_FLOW_TYPE_MC: 
            rc = rdpa_iptv_channel_pm_stats_get(iptv, request_key_val_get(PKTRUNNER_RDPA_KEY(accel,idx)), &flow_stats.rdpastat);
            break;
        case PKTRNR_FLOW_TYPE_IP: 
            if (!ip_class)
            {
                return 1;
            }
            
            rc = rdpa_ip_class_flow_stat_get(ip_class, PKTRUNNER_RDPA_KEY(accel,idx), &flow_stats.rdpastat);
			
            break;
#ifdef XRDP            
        case PKTRNR_FLOW_TYPE_L2:
            if (!l2_class)
            {
                return 1;
            }
            rc = rdpa_l2_class_flow_stat_get(l2_class, PKTRUNNER_RDPA_KEY(accel,idx), &flow_stats.rdpastat);
            break;
#endif            
        default:
            protoError("Failed to match flow_type =%d",flow_type);     
            rc = -1;

    }
    if (rc)
        return 2;

    PKTRUNNER_STATS(accel,idx) = flow_stats.num;

    return 0;
}

/* This function is invoked when all entries pertaining to a entIx in runner need to be cleared */
static int runner_clear(uint32_t pktrunner_key)
{
    return 0;
}

static int runner_refresh_ucast(uint32_t pktrunner_key, uint32_t *pktsCnt_p, uint32_t *octetsCnt_p)
{
    int rc, flowIdx = __pktRunnerFlowIdx(pktrunner_key);
    pktRunner_flowStat_t flowStat, resetFlowStat;
#ifdef XRDP
    uint32_t flow_type = __pktRunnerFlowType(pktrunner_key);
#endif
    uint32_t      accel = __pktRunnerAccelIdx(pktrunner_key);

#ifdef XRDP
    if (flow_type == PKTRNR_FLOW_TYPE_L2)
    {
        if (!l2_class)    
        {
            return 0;
        }
        
        rc = rdpa_l2_class_flow_stat_get(l2_class, PKTRUNNER_RDPA_KEY(accel,flowIdx), &flowStat.rdpastat);
        if (rc < 0)
        {
            protoError("Could not get flowIdx<%d> stats", flowIdx);
            return rc;
        }
    }
    else
#endif        
    {
        if (!ip_class)    
        {
            return 0;
        }
        
        rc = rdpa_ip_class_flow_stat_get(ip_class, PKTRUNNER_RDPA_KEY(accel,flowIdx), &flowStat.rdpastat);
        if (rc < 0)
        {
            protoError("Could not get flowIdx<%d> stats", flowIdx);
            return rc;
        }
    }

    resetFlowStat.num = PKTRUNNER_STATS(accel, flowIdx);

    *pktsCnt_p = flowStat.rdpastat.packets - resetFlowStat.rdpastat.packets; /* cummulative packets */
    *octetsCnt_p = flowStat.rdpastat.bytes - resetFlowStat.rdpastat.bytes;

    protoDebug( "flowIdx<%03u> "
        "cumm_pkt_hits<%u> cumm_octet_hits<%u>\n",
        flowIdx, *pktsCnt_p, *octetsCnt_p );

    return 0; 
}

static int runner_refresh_mcast(uint32_t pktrunner_key, uint32_t *pktsCnt_p, uint32_t *octetsCnt_p)
{
#ifndef MCAST_FORWARD_TO_HOST
    int rc, flowIdx = __pktRunnerFlowIdx(pktrunner_key);
    pktRunner_flowStat_t flowStat, resetFlowStat;
    uint32_t      accel = __pktRunnerAccelIdx(pktrunner_key);

    rc = rdpa_iptv_channel_pm_stats_get(iptv, request_key_val_get(PKTRUNNER_RDPA_KEY(accel,flowIdx)), &flowStat.rdpastat);
    if (rc < 0)
    {
        if (rc == BDMF_ERR_NOENT)
        {
            protoNotice("flowIdx<%d>, channel_index<%ld>, not exist!", flowIdx, request_key_val_get(PKTRUNNER_RDPA_KEY(accel,flowIdx)));
        }
        else
        {
            protoError("Could not get flowIdx<%d> stats", flowIdx);
        }
        
        return rc;
    }

    resetFlowStat.num = PKTRUNNER_STATS(accel, flowIdx);

    *pktsCnt_p = flowStat.rdpastat.packets - resetFlowStat.rdpastat.packets; /* cummulative packets */
    *octetsCnt_p = flowStat.rdpastat.bytes - resetFlowStat.rdpastat.bytes;

    protoDebug( "flowIdx<%03u> "
        "cumm_pkt_hits<%u> cumm_octet_hits<%u>\n",
        flowIdx, *pktsCnt_p, *octetsCnt_p );
#endif

    return 0; 
}



/*
 *------------------------------------------------------------------------------
 * Function   : runner_ip_class_refresh_pathstat
 * Description: This function is invoked to refresh path statistics
 * Parameters :
 *  pathIdx : 8bit index to refer to a path
 * Returns    : Total hits on this connection.
 *------------------------------------------------------------------------------
 */
static int runner_ip_class_refresh_pathstat(int pathIdx, uint32_t *pktsCnt_p, uint32_t *octetsCnt_p)
{
    rdpa_stat_t pathstat;
    int rc;

    if (!ip_class)  
    {
        *pktsCnt_p = 0;
        *octetsCnt_p = 0;
		
        return 0;
    }
    
    rc = rdpa_ip_class_pathstat_get(ip_class, pathIdx, &pathstat);
    if (rc < 0)
    {
        protoError("%s: Could not get pathIdx<%d> stats, rc %d", __FUNCTION__, pathIdx, rc);
        return rc;
    }

    *pktsCnt_p = pathstat.packets; /* collect packets */
    *octetsCnt_p = pathstat.bytes;

    protoDebug( "pathIdx<%03u> "
                "pkt_hits<%u> pkt_bytes<%u>\n",
                pathIdx, *pktsCnt_p, *octetsCnt_p );
    return 0;
}


/*
 *------------------------------------------------------------------------------
 * Function   : runnerRefreshPathStat
 * Description: This function is invoked to collect path statistics
 * Parameters :
 *  pathIdx : 16bit index to refer to a Runner path
 * Returns    : Total hits on this connection.
 *------------------------------------------------------------------------------
 */
static int runnerRefreshPathStat(uint8_t pathIdx, uint32_t *pktsCnt_p, uint32_t *octetsCnt_p)
{
    int rc;
    rdpa_stat_t rawStat;

    rc = runner_ip_class_refresh_pathstat(pathIdx, &rawStat.packets, &rawStat.bytes);
    if (rc < 0)
    {
        protoDebug("%s: Could not get pathIdx<%d> stats, rc %d", __FUNCTION__, pathIdx, rc);
        return rc;
    }

    *pktsCnt_p = rawStat.packets;
    *octetsCnt_p = rawStat.bytes;

    return 0;
}


static inline int runner_deactivate_ucast(uint32_t pktrunner_key, uint32_t *pktsCnt_p,
	uint32_t *octetsCnt_p, struct blog_t *blog_p)
{
    int rc;
    rdpa_ip_flow_info_t flow_info = {};
    uint32_t flowIdx = __pktRunnerFlowIdx(pktrunner_key);
#ifdef XRDP    
    uint32_t flow_type = __pktRunnerFlowType(pktrunner_key);
    rdpa_l2_flow_info_t l2_flow_info = {};
#endif
#ifndef CONFIG_BCM_FTTDP_G9991
    bdmf_object_handle tunnel_obj = NULL;
#endif    
    uint32_t accel = __pktRunnerAccelIdx(pktrunner_key);

    if (accel != PKTRUNNER_ACCEL_UCAST)
    {
        protoError("Failed to remove flow<%03u>, unexpected accel tyoe %d\n", pktrunner_key, accel);
        return -1;
    }
#ifdef XRDP
    if((flow_type == PKTRNR_FLOW_TYPE_L2) && l2_class)
    {
        rc = rdpa_l2_class_flow_get(l2_class, flowIdx, &l2_flow_info);
        if (rc < 0)
        {
            protoError("Failed to get flow<%03u>, rc %d\n", pktrunner_key, rc);
            return rc;
        }
    }
    else if((flow_type == PKTRNR_FLOW_TYPE_IP) && ip_class)
#else
    if(ip_class)
#endif    
    {
        rc = rdpa_ip_class_flow_get(ip_class, flowIdx, &flow_info);
        if (rc < 0)
        {
            protoError("Failed to get flow<%03u>, rc %d\n", pktrunner_key, rc);
            return rc;
        }
    }
    /* Fetch last hit count */
    rc = runner_refresh_ucast(pktrunner_key, pktsCnt_p, octetsCnt_p);
#ifdef XRDP    
    if (flow_type == PKTRNR_FLOW_TYPE_L2)
    {
        if (l2_class)
            rc = rc ? rc : rdpa_l2_class_flow_delete(l2_class, PKTRUNNER_RDPA_KEY(accel,flowIdx));
    }
    else
#endif        
    {
        rc = rc ? rc : rdpa_ip_class_flow_delete(ip_class, PKTRUNNER_RDPA_KEY(accel,flowIdx));
    }

    if (rc < 0)
    {
        protoError("Failed to remove flow<%03u>, rc %d\n", pktrunner_key, rc);
        return rc;
    }
#ifndef CONFIG_BCM_FTTDP_G9991  
#ifdef XRDP
    if (flow_type == PKTRNR_FLOW_TYPE_L2)
        tunnel_obj = l2_flow_info.result.tunnel_obj;
    else
        tunnel_obj = flow_info.result.tunnel_obj;
    if (tunnel_obj != NULL)
#else
    tunnel_obj = flow_info.result.tunnel_obj;
    if (tunnel_obj != NULL)
#endif
    {
        bdmf_number ref_cnt;
        rc = rdpa_tunnel_ref_cnt_get(tunnel_obj, &ref_cnt);
        if (rc < 0)
        {
            protoError("Failed to get ref_count from tunnel, rc %d\n", rc);
            return rc;
        }
        if (!ref_cnt)
        {
            rdpa_tunnel_cfg_t tunnel_cfg;
            uint8_t index;
            rdpa_tunnel_cfg_get(tunnel_obj, &tunnel_cfg);
            if (tunnel_cfg.tunnel_type == rdpa_tunnel_dslite)
                index = DS_LITE_TUNNEL_INDEX;
            else
                index = GRE_TUNNEL_INDEX;
            bdmf_destroy(tunnel[index].p_tunnel);
            memset(&tunnel[index], 0, sizeof(struct tunnel_entry));
        }
    }
#endif    
    rc = idx_pool_return_index(&PKTRUNNER_DATA(accel).idx_pool, flowIdx);
    if (rc < 0)
    {
        protoError("Failed to free flow<%03u>, rc %d\n", pktrunner_key, rc);
        return rc;
    }
    PKTRUNNER_RDPA_KEY(accel, flowIdx) = FHW_TUPLE_INVALID;

    PKTRUNNER_STATS(accel,flowIdx) = 0;

    protoDebug(
        "::: runner_deactivate_ucast flowIx<%03u> hits<%u> bytes<%u> cumm_deactivates<%u> :::\n",
        pktrunner_key, *pktsCnt_p, *octetsCnt_p, PKTRUNNER_STATE(accel).deactivates);

    PKTRUNNER_STATE(accel).deactivates++;
    PKTRUNNER_STATE(accel).active--;

    return 0;
}

/* This function is invoked when a flow in the runner needs to be deactivated */
static inline int runner_deactivate_mcast(uint32_t pktrunner_key, uint32_t *pktsCnt_p,
	uint32_t *octetsCnt_p, struct blog_t *blog_p)
{
    uint32_t flowIdx = __pktRunnerFlowIdx(pktrunner_key);
    rdpa_channel_req_key_t request_key;
    int rc;
#ifndef CONFIG_BCM_FTTDP_G9991
    bdmf_index wlan_mcast_idx = RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID;
#endif
    rdpa_iptv_channel_t channel;
    uint32_t accel = __pktRunnerAccelIdx(pktrunner_key);
    int more_ref= 0;

    if (accel != PKTRUNNER_ACCEL_MCAST)
    {
        protoError("Failed to remove flow<%03u>, unexpected accel tyoe %d\n", pktrunner_key, accel);
        return -1;
    }

    rc = runner_refresh_mcast(pktrunner_key, pktsCnt_p, octetsCnt_p);
    if (rc >= 0)
    {
        rc = rdpa_iptv_channel_get(iptv, request_key_val_get(PKTRUNNER_RDPA_KEY(accel, flowIdx)),
            &channel);
#ifndef CONFIG_BCM_FTTDP_G9991
        if (rc >= 0)
            wlan_mcast_idx = channel.wlan_mcast_index;

        request_key.port = blog_parse_egress_port_get(blog_p);
        if (rdpa_if_is_wifi(request_key.port) &&
            wlan_mcast_idx != RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID)
        {
            pktrunner_wlan_mcast_delete(blog_p, &wlan_mcast_idx);
        }

#ifndef XRDP      
        if (wlan_mcast_idx == RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID && rdpa_if_is_wifi(request_key.port))
        {
            /* The SSID ports are stored in the additional table. In case this is the last ssid port the
             * wlan_mcast_index will be equeal to Invalid index and the wlan0/wlan1/wlan2 port should be removed */
            request_key.port = rdpa_if_wlan0 + blog_p->wfd.mcast.wfd_idx; 
        }
#endif
#endif        
        request_key.channel_index = request_key_del(PKTRUNNER_RDPA_KEY(accel, flowIdx), blog_p->vtag_num);
        protoDebug("request_key_val_get; key: %d channel_index: %ld\n", flowIdx, request_key.channel_index);
        rc = rdpa_iptv_channel_request_delete(iptv, &request_key);
    }
    if (rc < 0)
    {
        /* XXX: Temporary work-around: It's possible that we got a blog rule with capabilities that we currently
         * don't handle. In this case we didn't actually added the channel, but assumes that it's ok. */
        if (rc == BDMF_ERR_NOENT)
        {
            /* remove mcast_hw_channel_key_entry */
            request_key.channel_index = request_key_del(PKTRUNNER_RDPA_KEY(accel, flowIdx), blog_p->vtag_num);
            protoNotice("flow<%03u>, channel_index<%ld>, not exist!\n", pktrunner_key, request_key.channel_index);
        }    
        else
        {
            protoError("Failed to remove flow<%03u>, rc %d\n", pktrunner_key, rc);
            return rc;
        }
    }

    more_ref = request_key_refcnt_get(PKTRUNNER_RDPA_KEY(accel, flowIdx), blog_p->vtag_num) ? 1 : 0;

    if (!more_ref)
    {
        rc = idx_pool_return_index(&PKTRUNNER_DATA(accel).idx_pool, flowIdx);
        if (rc < 0)
        {
            protoError("Failed to free flow<%03u>, rc %d\n", pktrunner_key, rc);
            return rc;
        }
        PKTRUNNER_RDPA_KEY(accel, flowIdx) = FHW_TUPLE_INVALID;
    }

    PKTRUNNER_STATS(accel,flowIdx) = 0;

    PKTRUNNER_STATE(accel).deactivates++;
    PKTRUNNER_STATE(accel).active--;

    protoDebug(
        "::: runner_deactivate_mcast flowIx<%03u> hits<%u> bytes<%u> cumm_deactivates<%u> :::\n",
        pktrunner_key, *pktsCnt_p, *octetsCnt_p, PKTRUNNER_STATE(accel).deactivates);

    /* do not actually return number of existing ports used on channel, but only indication whether its still in use */
    return more_ref;
}


static void add_qos_commands(rdpa_traffic_dir dir, rdpa_ip_flow_result_t *ip_flow_result, Blog_t *blog_p)
{
    BOOL enable;
    bdmf_object_handle policer_obj = NULL;

    if (0 == rdpa_mw_pkt_based_qos_get(
        dir, RDPA_MW_QOS_TYPE_FC, &enable))
    {
        ip_flow_result->qos_method = 
            enable? rdpa_qos_method_pbit : rdpa_qos_method_flow;
    }
    
    /* Assign policer */
    blog_parse_policer_get(blog_p, &policer_obj);
    ip_flow_result->policer_obj = policer_obj;

    if (blog_p->dscp2q)
        ip_flow_result->qos_method = rdpa_qos_method_pbit;

    if (blog_p->dscp2pbit)
    {
        ip_flow_result->action_vec |= rdpa_fc_action_opbit_remark;
        ip_flow_result->opbit_action = rdpa_pbit_act_dscp_copy;
    }
}

static int add_fwd_commands(rdpa_ip_flow_info_t *ip_flow, Blog_t *blog_p)
{
    uint16_t src_port = 0, dst_port = 0;

    if (blog_p->tx.info.phyHdrType == BLOG_GPONPHY || blog_p->tx.info.phyHdrType == BLOG_EPONPHY)
    {
        ip_flow->result.wan_flow = blog_p->tx.info.channel;
        ip_flow->key.dir = rdpa_dir_us;
        protoDebug("<port:gpon/epon,gemid/llid:%d>", ip_flow->result.wan_flow);
    }
    /* US GBE, DS and LAN<->WLAN */
    else if ((blog_p->tx.info.phyHdrType == BLOG_ENETPHY || blog_p->tx.info.phyHdrType == BLOG_WLANPHY || 
        blog_p->tx.info.phyHdrType == BLOG_NETXLPHY || blog_p->tx.info.phyHdrType == BLOG_SPDTST))
    {
        if (blog_p->tx.info.phyHdrType == BLOG_NETXLPHY)
        {
            ip_flow->result.port = rdpa_if_wlan0 + blog_p->rnr.radio_idx;
            ip_flow->result.ssid = blog_p->rnr.ssid;
        }
        else if (blog_p->tx.info.phyHdrType == BLOG_SPDTST)
        {
            uint8_t is_hw_mode = (blog_p->spdtst >> 1) & 1;
            uint8_t is_upload = (blog_p->spdtst >> 2) & 1;
            uint8_t stream_idx = (blog_p->spdtst >> 3) & 3;

            ip_flow->result.port = rdpa_if_cpu;
            ip_flow->result.action = rdpa_forward_action_forward;
            ip_flow->result.trap_reason = rdpa_cpu_rx_reason_udef_7;
            ip_flow->result.is_tcpspdtest = is_hw_mode;
            ip_flow->result.tcpspdtest_is_upload = is_upload;
            ip_flow->result.tcpspdtest_stream_id = stream_idx;
            ip_flow->result.action_vec |= is_hw_mode ? 0 : rdpa_fc_action_forward;
        }
        else
        {
            ip_flow->result.port = rdpa_mw_root_dev2rdpa_if((struct net_device *)blog_p->tx_dev_p);
#ifdef XRDP
            if (blog_p->tx.info.phyHdrType == BLOG_WLANPHY)
                ip_flow->result.ssid = rdpa_mw_root_dev2rdpa_ssid((struct net_device *)blog_p->tx_dev_p);
#endif
        }

        if (((struct net_device *)blog_p->rx_dev_p)->priv_flags & IFF_WANDEV)
            ip_flow->key.dir = rdpa_dir_ds;
        else
            ip_flow->key.dir = rdpa_dir_us;
        protoDebug("<dir:%s, port:%d>", ip_flow->key.dir == rdpa_dir_us ? "us" : "ds", ip_flow->result.port);
    }
    else
    {
        protoDebug("Unsupported phy<%d>", blog_p->tx.info.phyHdrType);
        return -1;
    }

#ifdef XRDP
    if (blog_p->rx.info.phyHdrType == BLOG_GPONPHY)
    {
        protoDebug("source.phy GPON\n");
        ip_flow->key.ingress_if = rdpa_wan_type_to_if(rdpa_wan_gpon);
    }
    else if (blog_p->rx.info.phyHdrType == BLOG_EPONPHY)
    {
        protoDebug("source.phy EPON\n");
        ip_flow->key.ingress_if = rdpa_wan_type_to_if(rdpa_wan_epon);
    }
    else if (((struct net_device *)blog_p->rx_dev_p)->priv_flags & IFF_WANDEV)
    {
        protoDebug("source.phy ETH WAN\n");
        ip_flow->key.ingress_if = rdpa_wan_type_to_if(rdpa_wan_gbe);
    }
    else
    {
        /* LAN */ 
        if (blog_p->rx.info.phyHdrType == BLOG_ENETPHY)
        {
            protoDebug("source.phy ETH\n");
            ip_flow->key.ingress_if = rdpa_mw_root_dev2rdpa_if((struct net_device *)blog_p->rx_dev_p);
        }
        else if (blog_p->rx.info.phyHdrType == BLOG_WLANPHY)
        {
            protoDebug("source.phy WLAN\n");
            ip_flow->key.ingress_if = rdpa_mw_root_dev2rdpa_if((struct net_device *)blog_p->rx_dev_p);
            /*ip_flow->key.ingress_if = rdpa_if_ssid0 + ssid;*/  /* for future support with ssid WLAN flow */
        }
        else if (blog_p->rx.info.phyHdrType == BLOG_SPDTST)
        {
            uint8_t is_hw_mode = (blog_p->spdtst >> 1) & 1;

            ip_flow->key.ingress_if = rdpa_if_cpu0;
            ip_flow->key.dir = rdpa_dir_us;
            if (is_hw_mode)
            {
                /* To avoid drop of the packets by QM, mark the packets as generated by speed test. */
                ip_flow->result.action_vec |= rdpa_fc_action_spdt_gen;
            }
        }
        else
        {
            protoError("LAN flow is not supported: Rx %u, Tx %u", blog_p->rx.info.phyHdrType, blog_p->tx.info.phyHdrType);
            return -1;
        }
    }
#endif
    
    /* Assign queue (queue should be configured elsewhere of course),
       XXX: group is alway 0 SP */
    if(blog_p->tx.info.phyHdrType != BLOG_WLANPHY && blog_p->tx.info.phyHdrType != BLOG_NETXLPHY)
    	ip_flow->result.queue_id = SKBMARK_GET_Q_PRIO(blog_p->mark);

    if (CHK4in6(blog_p))
    {
        ip_flow->result.action_vec |= rdpa_fc_action_ttl | rdpa_fc_action_dslite_tunnel;
        if (ip_flow->key.dir == rdpa_dir_us)
        {
#ifndef XRDP            
            memcpy(&ip_flow->result.ds_lite_src, &blog_p->tupleV6.saddr, sizeof(ip6_addr_t));
            memcpy(&ip_flow->result.ds_lite_dst, &blog_p->tupleV6.daddr, sizeof(ip6_addr_t));
#else
#ifndef CONFIG_BCM_FTTDP_G9991 
            tunnel_key flow_tunnel_key = {};
            int rc;
           
            flow_tunnel_key.dst_ip.family = bdmf_ip_family_ipv6;
            memcpy(&flow_tunnel_key.dst_ip.addr.ipv6, &blog_p->tupleV6.daddr, 16);
            flow_tunnel_key.src_ip.family = bdmf_ip_family_ipv6;
            memcpy(&flow_tunnel_key.src_ip.addr.ipv6, &blog_p->tupleV6.saddr, 16);
            flow_tunnel_key.prot_key = 0;

            if (memcmp(&(tunnel[DS_LITE_TUNNEL_INDEX].key), &flow_tunnel_key, sizeof(tunnel_key)) == 0)
                ip_flow->result.tunnel_obj = tunnel[DS_LITE_TUNNEL_INDEX].p_tunnel;
            else
            {
                BDMF_MATTR(tunnel_attrs, rdpa_tunnel_drv());
                rdpa_tunnel_cfg_t rdpa_tunnel;

                if (tunnel[DS_LITE_TUNNEL_INDEX].p_tunnel)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "One tunnel object is already exist\n");
                    return -1;
                }
                /* create new tunnel object */
                rc = bdmf_new_and_set(rdpa_tunnel_drv(), NULL, tunnel_attrs, &(ip_flow->result.tunnel_obj));
                if (rc)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "failed creating tunnel object\n");
                    return -1;
                }

                build_dslite_tunnel_header(blog_p, &rdpa_tunnel);

                rc = rdpa_tunnel_cfg_set(ip_flow->result.tunnel_obj, &rdpa_tunnel);
                if (rc)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "failed updating tunnel object\n");
                    return -1;
                }

                memcpy(&(tunnel[DS_LITE_TUNNEL_INDEX].key), &flow_tunnel_key, sizeof(tunnel_key));
                tunnel[DS_LITE_TUNNEL_INDEX].p_tunnel = ip_flow->result.tunnel_obj;
            }
#endif            
#endif                
        }
    }
    else if (CHK4to4(blog_p) || PTG4(blog_p) || PTE4(blog_p))
    {
        if (blog_p->rx.tuple.ttl != blog_p->tx.tuple.ttl)
            ip_flow->result.action_vec |= rdpa_fc_action_ttl;
    }
    else if (CHK6to6(blog_p))
    {
        if (blog_p->tupleV6.rx_hop_limit != blog_p->tupleV6.tx_hop_limit)
            ip_flow->result.action_vec |= rdpa_fc_action_ttl;
    }
    else if (MAPT(blog_p))
    {
        ip_flow->result.action_vec |= rdpa_fc_action_mapt;
        ip_flow->result.action_vec |= rdpa_fc_action_ttl;
        ip_flow->result.is_df = blog_p->is_df;         

        if (MAPT_DN(blog_p))
        {
            ip_flow->result.mapt_cfg.tos_tc = blog_p->tx.tuple.tos;
            ip_flow->result.mapt_cfg.proto = blog_p->key.protocol;
            ip_flow->result.mapt_cfg.src_ip.family = bdmf_ip_family_ipv4;
            memcpy(&ip_flow->result.mapt_cfg.src_ip.addr.ipv4, &blog_p->tx.tuple.saddr, sizeof(bdmf_ipv4));
            memcpy(&ip_flow->result.mapt_cfg.dst_ip.addr.ipv4, &blog_p->tx.tuple.daddr, sizeof(bdmf_ipv4));
            ip_flow->result.mapt_cfg.src_port = blog_p->tx.tuple.port.source;
            ip_flow->result.mapt_cfg.dst_port = blog_p->tx.tuple.port.dest;
            ip_flow->result.mapt_cfg.l4csum = blog_p->tx.tuple.check;
            ip_flow->result.mapt_cfg.l3csum = blog_p->rx.tuple.check;
        }
        else
        {
            ip_flow->result.mapt_cfg.tos_tc = PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0);
            ip_flow->result.mapt_cfg.proto = blog_p->key.protocol;        
            ip_flow->result.mapt_cfg.src_ip.family = bdmf_ip_family_ipv6;
            memcpy(&ip_flow->result.mapt_cfg.src_ip.addr.ipv6, &blog_p->tupleV6.saddr, sizeof(bdmf_ipv6_t));
            memcpy(&ip_flow->result.mapt_cfg.dst_ip.addr.ipv6, &blog_p->tupleV6.daddr, sizeof(bdmf_ipv6_t));
            ip_flow->result.mapt_cfg.src_port = blog_p->tupleV6.port.source;
            ip_flow->result.mapt_cfg.dst_port = blog_p->tupleV6.port.dest;
            ip_flow->result.mapt_cfg.l4csum = blog_p->tx.tuple.check;
            ip_flow->result.mapt_cfg.l3csum = 0; /* IPv6 header does not have CSUM */
        }
    }
    else if (RX_GRE(blog_p) ^ TX_GRE(blog_p)) /* The transmit should be different from recieve for perfrom termination */
    {
#ifndef CONFIG_BCM_FTTDP_G9991        
        if TX_GRE(blog_p)
        {
            tunnel_key flow_tunnel_key = {};
            int rc;

            ip_flow->result.action_vec |= rdpa_fc_action_gre_tunnel;
            
            if (TX_GIP46in4(blog_p))
            {
                flow_tunnel_key.dst_ip.family = bdmf_ip_family_ipv4;
                flow_tunnel_key.dst_ip.addr.ipv4 = blog_p->deltx_tuple.daddr;
                flow_tunnel_key.src_ip.family = bdmf_ip_family_ipv4;
                flow_tunnel_key.src_ip.addr.ipv4 = blog_p->deltx_tuple.saddr;
                flow_tunnel_key.prot_key = blog_p->gretx.key;
            }
            else
            {
#ifdef XRDP
                flow_tunnel_key.dst_ip.family = bdmf_ip_family_ipv6;
                memcpy(&flow_tunnel_key.dst_ip.addr.ipv6, &blog_p->del_tupleV6.daddr, 16);
                flow_tunnel_key.src_ip.family = bdmf_ip_family_ipv6;
                memcpy(&flow_tunnel_key.src_ip.addr.ipv6, &blog_p->del_tupleV6.saddr, 16);
                flow_tunnel_key.prot_key = blog_p->gretx.key;
#else
                protoError("GRE Delivery L3 IPv6 is not supported");
                return -1;
#endif                
            }
            if (memcmp(&(tunnel[GRE_TUNNEL_INDEX].key), &flow_tunnel_key, sizeof(tunnel_key)) == 0)
                ip_flow->result.tunnel_obj = tunnel[GRE_TUNNEL_INDEX].p_tunnel;
            else
            {
                BDMF_MATTR(tunnel_attrs, rdpa_tunnel_drv());
                rdpa_tunnel_cfg_t rdpa_tunnel;

                if (tunnel[GRE_TUNNEL_INDEX].p_tunnel)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "One tunnel object is already exist\n");
                    return -1;
                }
                /* create new tunnel object */
                rc = bdmf_new_and_set(rdpa_tunnel_drv(), NULL, tunnel_attrs, &(ip_flow->result.tunnel_obj));
                if (rc)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "failed creating tunnel object\n");
                    return -1;
                }

                build_gre_tunnel_header(blog_p, &rdpa_tunnel);

                rc = rdpa_tunnel_cfg_set(ip_flow->result.tunnel_obj, &rdpa_tunnel);
                if (rc)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "failed updating tunnel object\n");
                    return -1;
                }

                memcpy(&(tunnel[GRE_TUNNEL_INDEX].key), &flow_tunnel_key, sizeof(tunnel_key));
                tunnel[GRE_TUNNEL_INDEX].p_tunnel = ip_flow->result.tunnel_obj;
            }
        }
        else /* RX_GRE */
        {
        }
#endif        
    }
    else
    {
        protoError("Unable to determine if the flow is routed/bridged (%d)", __LINE__);
        return -1;
    }

    ip_flow->key.prot = blog_p->key.protocol;
    ip_flow->key.tcp_pure_ack = blog_p->key.tcp_pure_ack;
    if (blog_p->rx.info.bmap.PLD_IPv6 && !(T4in6DN(blog_p)))
    {
        /* This is a IPv6 flow */
        memcpy(&ip_flow->key.src_ip.addr.ipv6, &blog_p->tupleV6.saddr, sizeof(ip6_addr_t));
        memcpy(&ip_flow->key.dst_ip.addr.ipv6, &blog_p->tupleV6.daddr, sizeof(ip6_addr_t));
        if (blog_p->key.protocol == IPPROTO_TCP || blog_p->key.protocol == IPPROTO_UDP)
        {
            src_port = blog_p->tupleV6.port.source;
            dst_port = blog_p->tupleV6.port.dest;
        }
        ip_flow->key.src_ip.family = bdmf_ip_family_ipv6;
        ip_flow->key.dst_ip.family = bdmf_ip_family_ipv6;
    }
    else
    {
        /* This is a IPv4 flow/tunnel */
        ip_flow->key.src_ip.addr.ipv4 = htonl(blog_p->rx.tuple.saddr);
        ip_flow->key.dst_ip.addr.ipv4 = htonl(blog_p->rx.tuple.daddr);
        if (blog_p->key.protocol == IPPROTO_TCP || blog_p->key.protocol == IPPROTO_UDP)
        {
            src_port = blog_p->rx.tuple.port.source;
            dst_port = blog_p->rx.tuple.port.dest;
        }
        else if (blog_p->key.protocol == IPPROTO_GRE)
            dst_port = blog_p->rx.tuple.gre_callid;
        else if (blog_p->key.protocol == IPPROTO_ESP)
        {
#if defined(CONFIG_CPU_LITTLE_ENDIAN)
            /* XRDP */
            dst_port = blog_p->rx.tuple.esp_spi >> 16;
            src_port = blog_p->rx.tuple.esp_spi & 0xffff;
#else
            /* RDP */
            dst_port = blog_p->rx.tuple.esp_spi & 0xffff;
            src_port = blog_p->rx.tuple.esp_spi >> 16;
#endif
        }

        ip_flow->key.src_ip.family = bdmf_ip_family_ipv4;
        ip_flow->key.dst_ip.family = bdmf_ip_family_ipv4;
    }

    ip_flow->key.src_port = htons(src_port);
    ip_flow->key.dst_port = htons(dst_port);

    return 0;
}

static int _l2_pppoe_passthrou(Blog_t *blog_p)
{
#ifdef XRDP
    if (!l2_class)
        return 0;
    /* Flows for PPPoE termination will be L3 (routed) */
    if (blog_p->rx.info.bmap.PLD_L2)
        return 1;

    return 0;
#else
    /* L2 FC is not supported for RDP */
    return 0;
#endif
}

static int add_l2_commands(rdpa_ip_flow_result_t *ip_flow_result, Blog_t *blog_p)
{
    int ix;
    int is_ext_switch_port = 0;
    int bcmtaglen = 0;
    int bcmtaglocation = 2 * ETH_ALEN;
    int l2_header_offset = 0;

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)    
    fc_class_ctx_t fc_key;
#endif

    if ((((struct net_device *)blog_p->rx_dev_p)->priv_flags & IFF_WANDEV) && IsExternalSwitchPort(blog_p->tx.info.channel))
    {
        is_ext_switch_port = 1;
        bcmtaglen = sizeof(S_INGRESS_BCM_SWITCH_TAG);
    }

    ip_flow_result->ovid_offset = is_ext_switch_port ? rdpa_vlan_offset_16 :
        rdpa_vlan_offset_12;

    BCM_ASSERT((blog_p->tx.length + bcmtaglen <= 32));
    ip_flow_result->l2_header_size = 0;

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
    if (blog_p->fc_hybrid)
    {
        fc_key.word = blog_p->fc_context;  
        fc_key.id.src_port = rdpa_mw_root_dev2rdpa_if(blog_p->rx_dev_p);
        *(uint32_t*)ip_flow_result->l2_header = fc_key.word; 
        ip_flow_result->l2_header_size += L2_FLOW_P_LEN;
        l2_header_offset += L2_FLOW_P_LEN;
    }
#endif    

#if defined(XRDP) && !defined(CONFIG_BCM_FTTDP_G9991)
    if (blog_p->tx.info.phyHdrType == BLOG_WLANPHY && !blog_p->wfd.nic_ucast.is_wfd
        && l2_header_need_llc_snap(blog_p->rnr.radio_idx))
    {
        /* Check if need to add LLC Snap */
        l2_header_offset += L2_DOT11_LLC_SNAP_HDR_LEN;
        ip_flow_result->action_vec |= rdpa_fc_action_llc_snap_set_len;
    }
#endif

    ip_flow_result->pathstat_idx = blog_p->hw_pathstat_idx;
    
    /* get_max_rx_pkt_len including crc according to mtu */
    ip_flow_result->max_pkt_len = blog_getTxMtu(blog_p) + blog_p->rx.length + 4;

    if (TX_GRE(blog_p) && !RX_GRE(blog_p)) /* GRE TX Termination, the L2 should be of the encapsulated packet since the L2 of the tunnel is stored in tunnel object */
    {
        BlogGre_t *gre_p = &blog_p->gretx;

        ip_flow_result->l2_header_offset = blog_p->rx.length - gre_p->l2_hlen - l2_header_offset;
        ip_flow_result->l2_header_size = gre_p->l2_hlen;

        if (ip_flow_result->l2_header_size + l2_header_offset > RDPA_L2_HEADER_SIZE)
        {
            protoError("L2 header size %u exceeded the maximum %d\n", ip_flow_result->l2_header_size, RDPA_L2_HEADER_SIZE);
            return -1;
        }

        memcpy(&ip_flow_result->l2_header[l2_header_offset], gre_p->l2hdr, gre_p->l2_hlen);
    }
    else
    {
        uint8_t rx_length = blog_p->rx.length;
        uint8_t tx_length = blog_p->tx.length;
        uint8_t *l2hdr = blog_p->tx.l2hdr;

        if (RX_GRE(blog_p) && !TX_GRE(blog_p))
        {
            rx_length = blog_p->grerx.l2_hlen;
        }

        ip_flow_result->l2_header_number_of_tags = blog_p->vtag_num;
        ip_flow_result->l2_header_offset = rx_length - tx_length - bcmtaglen - l2_header_offset;
        ip_flow_result->l2_header_size += tx_length + bcmtaglen;

        if (ip_flow_result->l2_header_size + l2_header_offset > RDPA_L2_HEADER_SIZE)
        {
            protoError("L2 header size %u exceeded the maximum %d\n", ip_flow_result->l2_header_size, RDPA_L2_HEADER_SIZE);
            return -1;
        }

        memcpy(&ip_flow_result->l2_header[l2_header_offset], l2hdr, bcmtaglocation);
        memcpy(&ip_flow_result->l2_header[l2_header_offset + bcmtaglocation + bcmtaglen], l2hdr + bcmtaglocation, tx_length - bcmtaglocation);
    }
    
    /* add bcmtag on DS flows on board with external switch */
    if (is_ext_switch_port)
    {
        S_INGRESS_BCM_SWITCH_TAG bcmtag = localBcmTag;
        unsigned int port_map = (1 << blog_p->tx.info.channel);

        bcmtag.dst_port = GET_PORTMAP_FROM_LOGICAL_PORTMAP(port_map, 1);
        memcpy(&ip_flow_result->l2_header[l2_header_offset + bcmtaglocation], &bcmtag, bcmtaglen);

        protoDebug("<bcm chnl %d, bcmtag.dst %x>\n", blog_p->tx.info.channel, bcmtag.dst_port);
    }

#if defined(XRDP) && !defined(CONFIG_BCM_FTTDP_G9991)
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
    if (l2_header_offset - (blog_p->fc_hybrid ? L2_FLOW_P_LEN : 0)) /* Need to add LLC Snap */
#else
    if (l2_header_offset)
#endif    
    {
        l2_header_insert_llc_snap(ip_flow_result->l2_header, blog_p);
        ip_flow_result->l2_header_size += L2_DOT11_LLC_SNAP_HDR_LEN;
        if (ip_flow_result->l2_header_size > RDPA_L2_HEADER_SIZE)
        {
            protoError("L2 header size %u exceeded the maximum %d\n", ip_flow_result->l2_header_size, RDPA_L2_HEADER_SIZE);
            return -1;
        }
    }
#endif

    for (ix = 0; ix < blog_p->tx.count; ix++)
    {
        /* fix pppoe length */
        if (blog_p->tx.encap[ix] == PPPoE_2516)
        {
            /* L2_GRE US/DS flows are asymmetric, US flow is l2_class type. */
            /* In such case action should be "pppoe" and not "pppoe_passthrough" */
            if (_l2_pppoe_passthrou(blog_p) && !(TX_GRE(blog_p)))
                ip_flow_result->action_vec |= rdpa_fc_action_pppoe_passthrough;
            else
                ip_flow_result->action_vec |= rdpa_fc_action_pppoe;
        }
    }

    /* set WLAN acceleration info */
    if(blog_p->tx.info.phyHdrType == BLOG_WLANPHY)
    {
        protoDebug("Blog info: is_tx_hw_acc_en %d (need accel), is_wfd %d, is_chain %d\n", blog_p->wfd.nic_ucast.is_tx_hw_acc_en,
            blog_p->wfd.nic_ucast.is_wfd, blog_p->wfd.nic_ucast.is_chain);
        
        /* is_wfd is shared between all unions so it could be filled in one place */
        ip_flow_result->wfd.nic_ucast.is_wfd = blog_p->wfd.nic_ucast.is_wfd;

        if (blog_p->wfd.nic_ucast.is_wfd)
        {
            /* is_chained is shared between all unions so it could be filled in one place */
            ip_flow_result->wfd.nic_ucast.is_chain = blog_p->wfd.nic_ucast.is_chain;

            if (blog_p->wfd.nic_ucast.is_chain)
            {
                protoDebug("WFD Chain info: wfd_prio %d, wfd_idx %d, priority %d, chain_idx %d\n",
                    blog_p->wfd.nic_ucast.wfd_prio, blog_p->wfd.nic_ucast.wfd_idx, blog_p->wfd.nic_ucast.priority,
                    blog_p->wfd.nic_ucast.chain_idx);
                ip_flow_result->wfd.nic_ucast.wfd_prio = blog_p->wfd.nic_ucast.wfd_prio;
                ip_flow_result->wfd.nic_ucast.wfd_idx = blog_p->wfd.nic_ucast.wfd_idx;
                ip_flow_result->wfd.nic_ucast.priority = blog_p->wfd.nic_ucast.priority;
                ip_flow_result->wfd.nic_ucast.chain_idx = blog_p->wfd.nic_ucast.chain_idx;
            }
            else
            {
                protoDebug("WFD DHD info: wfd_prio %d, wfd_idx %d, priority %d, ssid %d, flowring_idx %d\n",
                    blog_p->wfd.dhd_ucast.wfd_prio, blog_p->wfd.dhd_ucast.wfd_idx, blog_p->wfd.dhd_ucast.priority,
                    blog_p->wfd.dhd_ucast.ssid, blog_p->wfd.dhd_ucast.flowring_idx);
                ip_flow_result->wfd.dhd_ucast.wfd_prio = blog_p->wfd.dhd_ucast.wfd_prio;
                ip_flow_result->wfd.dhd_ucast.ssid = blog_p->wfd.dhd_ucast.ssid;
                ip_flow_result->wfd.dhd_ucast.wfd_idx = blog_p->wfd.dhd_ucast.wfd_idx;
                ip_flow_result->wfd.dhd_ucast.priority = blog_p->wfd.dhd_ucast.priority;
                ip_flow_result->wfd.dhd_ucast.flowring_idx = blog_p->wfd.dhd_ucast.flowring_idx;
            }
            
            ip_flow_result->queue_id = blog_p->wfd.nic_ucast.wfd_idx;		
        }
        else
        {
            protoDebug("DHD offload info: radio_idx %d, priority %d, ssid %d, flowring_idx %d\n",
                blog_p->rnr.radio_idx, blog_p->rnr.priority, blog_p->rnr.ssid, blog_p->rnr.flowring_idx);
            ip_flow_result->queue_id = blog_p->rnr.priority;
            ip_flow_result->rnr.radio_idx = blog_p->rnr.radio_idx;
            ip_flow_result->rnr.priority = blog_p->rnr.priority;
            ip_flow_result->rnr.ssid = blog_p->rnr.ssid;
            ip_flow_result->rnr.flowring_idx = blog_p->rnr.flowring_idx;
            ip_flow_result->rnr.flow_prio =  blog_p->rnr.flow_prio;
        }
    }
    else if (blog_p->tx.info.phyHdrType == BLOG_NETXLPHY)
    {
        ip_flow_result->queue_id = 0;
        ip_flow_result->wl_metadata = 0;
        ip_flow_result->wfd.nic_ucast.is_wfd = 1;
    }

    return 0;
}

static int add_l3_commands(rdpa_ip_flow_result_t *ip_flow_result, Blog_t *blog_p)
{
    BlogTuple_t *rxIp_p = &blog_p->rx.tuple;
    BlogTuple_t *txIp_p = &blog_p->tx.tuple;
    rdpa_traffic_dir dir = (((struct net_device *)blog_p->rx_dev_p)->priv_flags & IFF_WANDEV) ? 
                            rdpa_dir_ds: rdpa_dir_us;
    bdmf_boolean is_ecn_enabled = 0;

    if (T4in6DN(blog_p))
    {
        /* XXX: tunneling */
    }
    else if (T4in6UP(blog_p))
    { }
    else if ((blog_p->rx.info.bmap.PLD_IPv6) || (blog_p->l2_ipv6))
    {   /* rx IPv6; tx IPv6 */
#if defined(XRDP)
        is_ecn_enabled = is_ecn_remark_set();
#endif
        if (is_ecn_enabled)
        {
            if (blog_p->rx.tuple.tos !=
                PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0))
            {
                ip_flow_result->action_vec |= rdpa_fc_action_dscp_remark;
                ip_flow_result->dscp_value = PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0);
                protoDebug("Replace DSCPv6<%d>", ip_flow_result->dscp_value);
            }
        }
        else
        {  /* Note: we are not remarking ECN */
            if (blog_p->rx.tuple.tos >> BLOG_RULE_DSCP_IN_TOS_SHIFT !=
                PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0) >> BLOG_RULE_DSCP_IN_TOS_SHIFT)
            {
                ip_flow_result->action_vec |= rdpa_fc_action_dscp_remark;
                ip_flow_result->dscp_value = PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0) >> BLOG_RULE_DSCP_IN_TOS_SHIFT;
                protoDebug("Replace DSCPv6<%d>", ip_flow_result->dscp_value);
            }
        }
    }

    if ((rxIp_p->tos >> BLOG_RULE_DSCP_IN_TOS_SHIFT != txIp_p->tos >> BLOG_RULE_DSCP_IN_TOS_SHIFT) &&
        (!T6in4UP(blog_p)) && (!T6in4DN(blog_p)) && !(blog_p->l2_ipv6) && !(blog_p->rx.info.bmap.PLD_IPv6))
    {
        ip_flow_result->action_vec |= rdpa_fc_action_dscp_remark;
        /* Note: we are not remarking ECN */
        ip_flow_result->dscp_value = *(uint8_t *)(&txIp_p->tos) >> BLOG_RULE_DSCP_IN_TOS_SHIFT;
        protoDebug("Replace DSCP<%d>", ip_flow_result->dscp_value);
    }

    if (blog_p->tx.info.bmap.PLD_IPv4 == 0 && !blog_p->l2_ipv4) /* tx is IPv6  */
        return 0; /* L3 IPv6 fields taken care of. OK to return */

    if (dir == rdpa_dir_us && rxIp_p->saddr != txIp_p->saddr && !(MAPT(blog_p)))
    {
        protoDebug("SNAT");
        ip_flow_result->nat_ip.addr.ipv4 = htonl(txIp_p->saddr);
        ip_flow_result->nat_ip.family = bdmf_ip_family_ipv4;
        if (blog_p->key.protocol == IPPROTO_TCP || blog_p->key.protocol == IPPROTO_UDP)
        {
            ip_flow_result->nat_port = htons(txIp_p->port.source);
            /* XXX: In current FW version NAT for non TCP/UDP traffic is not supported */
            ip_flow_result->action_vec |= rdpa_fc_action_nat;
        }
        else if (blog_p->key.protocol == IPPROTO_GRE || blog_p->key.protocol == IPPROTO_ESP)
            ip_flow_result->action_vec |= rdpa_fc_action_nat;

    }
    else if (dir == rdpa_dir_ds && rxIp_p->daddr != txIp_p->daddr && !(MAPT(blog_p)))
    {
        protoDebug("DNAT");
        ip_flow_result->nat_ip.addr.ipv4 = htonl(txIp_p->daddr);
        ip_flow_result->nat_ip.family = bdmf_ip_family_ipv4;
        if (blog_p->key.protocol == IPPROTO_TCP || blog_p->key.protocol == IPPROTO_UDP)
        {
            ip_flow_result->nat_port = htons(txIp_p->port.dest);
            /* XXX: In current FW version NAT for non TCP/UDP traffic is not supported */
            ip_flow_result->action_vec |= rdpa_fc_action_nat;
        }
        else if (blog_p->key.protocol == IPPROTO_GRE || blog_p->key.protocol == IPPROTO_ESP)
            ip_flow_result->action_vec |= rdpa_fc_action_nat;
    }

    if (blog_p->key.protocol == IPPROTO_GRE && rxIp_p->gre_callid != txIp_p->gre_callid)
    {
        ip_flow_result->nat_port = htons(txIp_p->gre_callid);
        ip_flow_result->action_vec |= rdpa_fc_action_gre_remark;
    }

    return 0;
}

static int add_sq_commands(rdpa_ip_flow_result_t *ip_flow_result, Blog_t *blog_p)
{
    rdpa_traffic_dir dir __maybe_unused =
        (((struct net_device *)blog_p->rx_dev_p)->priv_flags & IFF_WANDEV) ?
                rdpa_dir_ds: rdpa_dir_us;

    if (blog_p->dpi_queue >= RDPA_MAX_SERVICE_QUEUES)
    {
        ip_flow_result->action_vec &= ~rdpa_fc_action_service_q;
        return 0;
    }

#ifdef XRDP
    if (dir == rdpa_dir_us)
    {
        ip_flow_result->queue_id = blog_p->dpi_queue + DPI_XRDP_US_SQ_OFFSET;
    }
    else
#endif /* XRDP */
    {
        ip_flow_result->service_q_id = blog_p->dpi_queue;
        ip_flow_result->action_vec |= rdpa_fc_action_service_q;
    }

    return 0;
}

static int is_ip_proto_acceleration_supported(Blog_t *blog_p)
{
    switch (blog_p->key.protocol)
    {
    case IPPROTO_ICMP:
    case IPPROTO_IGMP:
        return 0;
    case IPPROTO_GRE:
        /* Only GRE v1 to GRE v1 flow is supported */
        return (blog_p->grerx.gre_flags.ver == 1 && blog_p->gretx.gre_flags.ver == 1);
    default:
        break;
    }
    return 1;
}


static bdmf_index runner_activate_l3_flow(Blog_t *blog_p)
{
    rdpa_ip_flow_info_t ip_flow = {};
    bdmf_index index;
    int rc;

    rc = add_fwd_commands(&ip_flow, blog_p);
    if (rc)
    {
        protoInfo("Failed to add FWD commands");
        goto abort_activate;
    }

    add_qos_commands(ip_flow.key.dir, &ip_flow.result, blog_p);

    rc = add_l2_commands(&ip_flow.result, blog_p);
    if (rc)
    {
        protoInfo("Failed to add L2 commands");
        goto abort_activate;
    }

    rc = add_l3_commands(&ip_flow.result, blog_p);
    if (rc)
    {
        protoInfo("Failed to add L3 commands");
        goto abort_activate;
    }

    rc = add_sq_commands(&ip_flow.result, blog_p);
    if (rc)
    {
        protoInfo("Failed to add SQ commands");
        goto abort_activate;
    }

    rc = rdpa_ip_class_flow_add(ip_class, &index, &ip_flow);
    if (rc < 0)
    {
        protoInfo("Failed to activate flow");
        goto abort_activate;
    }

    if (ip_flow.result.policer_obj)
        bdmf_put(ip_flow.result.policer_obj);

    return index;
abort_activate:
    if (ip_flow.result.policer_obj)
        bdmf_put(ip_flow.result.policer_obj);
    return -1;    
}

#ifdef XRDP
static bdmf_index runner_activate_l2_flow(Blog_t *blog_p)
{
    rdpa_l2_flow_info_t l2_flow = {};
    bdmf_index index;
    int rc;
  
    /*Build Key*/
    pktrunner_l2flow_key_construct(blog_p, &l2_flow.key);

    /* Start of result build */
    add_qos_commands(l2_flow.key.dir, &l2_flow.result, blog_p);

    l2_flow.result.port = rdpa_mw_root_dev2rdpa_if((struct net_device *)blog_p->tx_dev_p);

    if (blog_p->tx.info.phyHdrType == BLOG_WLANPHY)
        l2_flow.result.ssid = rdpa_mw_root_dev2rdpa_ssid((struct net_device *)blog_p->tx_dev_p);

    if (blog_p->tx.info.phyHdrType == BLOG_GPONPHY || blog_p->tx.info.phyHdrType == BLOG_EPONPHY)    
    {
        l2_flow.result.wan_flow = blog_p->tx.info.channel;
    }
    
    if(blog_p->tx.info.phyHdrType != BLOG_WLANPHY && blog_p->tx.info.phyHdrType != BLOG_NETXLPHY)
        l2_flow.result.queue_id = SKBMARK_GET_Q_PRIO(blog_p->mark);

    if (RX_GRE(blog_p) ^ TX_GRE(blog_p)) /* The transmit should be different from recieve for perfrom termination */
    {
#ifndef CONFIG_BCM_FTTDP_G9991        
        if TX_GRE(blog_p)
        {
            tunnel_key flow_tunnel_key = {};
            int rc;

           if (TX_GIP46in4(blog_p))
            {
                flow_tunnel_key.dst_ip.family = bdmf_ip_family_ipv4;
                flow_tunnel_key.dst_ip.addr.ipv4 = blog_p->deltx_tuple.daddr;
                flow_tunnel_key.src_ip.family = bdmf_ip_family_ipv4;
                flow_tunnel_key.src_ip.addr.ipv4 = blog_p->deltx_tuple.saddr;
                flow_tunnel_key.prot_key = blog_p->gretx.key;
            }
            else
            {
                flow_tunnel_key.dst_ip.family = bdmf_ip_family_ipv6;
                memcpy(&flow_tunnel_key.dst_ip.addr.ipv6, &blog_p->del_tupleV6.daddr, 16);
                flow_tunnel_key.src_ip.family = bdmf_ip_family_ipv6;
                memcpy(&flow_tunnel_key.src_ip.addr.ipv6, &blog_p->del_tupleV6.saddr, 16);
                flow_tunnel_key.prot_key = blog_p->gretx.key;
            }           
            if (memcmp(&(tunnel[GRE_TUNNEL_INDEX].key), &flow_tunnel_key, sizeof(tunnel_key)) == 0)
                l2_flow.result.tunnel_obj = tunnel[GRE_TUNNEL_INDEX].p_tunnel;
            else
            {
                BDMF_MATTR(tunnel_attrs, rdpa_tunnel_drv());
                rdpa_tunnel_cfg_t rdpa_tunnel;

                l2_flow.result.action_vec |= rdpa_fc_action_gre_tunnel;

                if (tunnel[GRE_TUNNEL_INDEX].p_tunnel)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "One tunnel object is already exist\n");
                    return -1;
                }
                /* create new tunnel object */
                rc = bdmf_new_and_set(rdpa_tunnel_drv(), NULL, tunnel_attrs, &(l2_flow.result.tunnel_obj));
                if (rc)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "failed creating tunnel object\n");
                    return -1;
                }

                build_gre_tunnel_header(blog_p, &rdpa_tunnel);

                rc = rdpa_tunnel_cfg_set(l2_flow.result.tunnel_obj, &rdpa_tunnel);
                if (rc)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "failed updating tunnel object\n");
                    return -1;
                }

                memcpy(&(tunnel[GRE_TUNNEL_INDEX].key), &flow_tunnel_key, sizeof(tunnel_key));
                tunnel[GRE_TUNNEL_INDEX].p_tunnel = l2_flow.result.tunnel_obj;
            }
        }
#endif        
    }

    rc = add_l2_commands(&l2_flow.result, blog_p);
    if (rc)
    {
        protoInfo("Failed to add L2 commands");
        goto abort_activate;
    }

    rc = add_l3_commands(&l2_flow.result, blog_p);
    if (rc)
    {
        protoInfo("Failed to add L3 commands");
        goto abort_activate;
    }

    rc = add_sq_commands(&l2_flow.result, blog_p);
    if (rc)
    {
        protoInfo("Failed to add SQ commands");
        goto abort_activate;
    }

    rc = rdpa_l2_class_flow_add(l2_class, &index, &l2_flow);
    if (rc < 0)
    {
        protoInfo("Failed to activate flow");
        goto abort_activate;
    }

    if (l2_flow.result.policer_obj)
        bdmf_put(l2_flow.result.policer_obj);
	
    return index;
abort_activate:
    if (l2_flow.result.policer_obj)
        bdmf_put(l2_flow.result.policer_obj);
    return -1; 
}
#endif

static uint32_t runner_activate_ucast(Blog_t *blog_p, uint32_t key_in)
{
    bdmf_index index;
    PktRunnerFlowKey_t pktRunner_key = {};
    int pktRunner_flow_idx;
    uint32_t accel = PKTRUNNER_ACCEL_UCAST;
    
    BCM_ASSERT((blog_p!=BLOG_NULL));

    BCM_LOGCODE(if(protoLogDebug)
        { protoPrint("\n::: runner_activate_ucast :::\n"); blog_dump(blog_p); });

    if (!ip_class)
    {
        protoInfo("No ip_class obj");
        goto abort_activate;
    }
    
    if (!((blog_p->rx.info.bmap.PLD_IPv6 && !T4in6DN(blog_p)) ||
		(blog_p->rx.info.bmap.PLD_IPv4) || (blog_p->rx.info.bmap.PLD_L2)))
    {
        protoInfo("Flow Type is not supported");
        goto abort_activate;
    }

    if (CHK6in4(blog_p))
    {
        protoInfo("6in4 Tunnel not supported");
        goto abort_activate;
    }

    if (!is_ip_proto_acceleration_supported(blog_p))
    {
        protoInfo("Flow Type proto<%d> is not supported", blog_p->key.protocol);
        goto abort_activate;
    }    
    
    pktRunner_flow_idx = idx_pool_get_index(&PKTRUNNER_DATA(accel).idx_pool);
    if (pktRunner_flow_idx < 0)
    {
        PKTRUNNER_STATE(accel).no_free_idx++;
        protoInfo("No free pkt runner indexes for accel %u", accel);
        goto abort_activate;
    }
    /* call provision function per flow type */
#ifdef XRDP    
    if (blog_p->rx.info.bmap.PLD_L2)
    {
        protoInfo("Accelerating L2 Flow");
        index = runner_activate_l2_flow(blog_p);
        pktRunner_key.word = __pktRunnerBuildKey(accel, PKTRNR_FLOW_TYPE_L2, pktRunner_flow_idx);
        PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) = index;
    }
    else
#endif        
    {
        protoInfo("Accelerating L3 Flow");
        index = runner_activate_l3_flow(blog_p);
        pktRunner_key.word = __pktRunnerBuildKey(accel, PKTRNR_FLOW_TYPE_IP, pktRunner_flow_idx);
        PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) = index;
    }

    /* check provision status */
    if (index < 0)
    {
        idx_pool_return_index(&PKTRUNNER_DATA(accel).idx_pool, pktRunner_flow_idx);
        PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) = FHW_TUPLE_INVALID;
        PKTRUNNER_STATE(accel).hw_add_fail++;
        goto abort_activate;
    }
    
    PKTRUNNER_STATS(accel, pktRunner_flow_idx) = 0;

    PKTRUNNER_STATE(accel).activates++;
    PKTRUNNER_STATE(accel).active++;
    protoDebug("::: runner_activate_ucast flowId<%03u> cumm_activates<%u> :::\n\n",
        (int)index, PKTRUNNER_STATE(accel).activates);

    return pktRunner_key.word;

abort_activate:
    PKTRUNNER_STATE(accel).failures++;
    protoInfo("cumm_failures<%u>", PKTRUNNER_STATE(accel).failures);
    return FHW_TUPLE_INVALID;
}

static uint32_t runner_update_ucast(BlogUpdate_t update, uint32_t pktRunner_key, Blog_t *blog_p)
{
    int rc = 0;
    bdmf_index index = __pktRunnerFlowIdx(pktRunner_key);
    uint32_t accel = __pktRunnerAccelIdx(pktRunner_key);

    switch(update)
    {
    case BLOG_UPDATE_DPI_QUEUE:
    case BLOG_UPDATE_DPI_PRIORITY:
#ifdef XRDP
        if (blog_p->rx.info.bmap.PLD_L2)
        {
            rdpa_l2_flow_info_t l2_flow = {};

            rc = rdpa_l2_class_flow_get(l2_class, PKTRUNNER_RDPA_KEY(accel, index), &l2_flow);
            if (rc)
            {
                protoInfo("Failed to get L2 flow");
                return -1;
            }
            /* update flow settings */
            if(update == BLOG_UPDATE_DPI_QUEUE){
               rc = add_sq_commands(&l2_flow.result, blog_p);
            }
#if defined(CONFIG_BCM_DPI_WLAN_QOS)
            else{
               if (blog_p->wfd.nic_ucast.is_chain)
                   l2_flow.result.wfd.nic_ucast.priority = blog_p->wfd.nic_ucast.priority;
               else{
                   if (blog_p->rnr.is_wfd)
                       l2_flow.result.wfd.dhd_ucast.flowring_idx = blog_p->wfd.dhd_ucast.flowring_idx;
                   else{
                       l2_flow.result.rnr.priority = blog_p->rnr.priority;
                       l2_flow.result.rnr.flowring_idx = blog_p->rnr.flowring_idx;
                   }
               }
            }
#endif
            rc = rc ? rc : rdpa_l2_class_flow_set(l2_class, PKTRUNNER_RDPA_KEY(accel, index), &l2_flow);
        }
        else
#endif
        {
            rdpa_ip_flow_info_t ip_flow = {};

            rc = rdpa_ip_class_flow_get(ip_class, PKTRUNNER_RDPA_KEY(accel, index), &ip_flow);
            if (rc)
            {
                protoInfo("Failed to get L2 flow");
                return -1;
            }
            /* update flow settings */
            if(update == BLOG_UPDATE_DPI_QUEUE)
            {
                rc = add_sq_commands(&ip_flow.result, blog_p);
            }
#if defined(CONFIG_BCM_DPI_WLAN_QOS)
            else{
                if (blog_p->wfd.nic_ucast.is_chain)
                    ip_flow.result.wfd.nic_ucast.priority = blog_p->wfd.nic_ucast.priority;
                else {
                    if (blog_p->rnr.is_wfd)
                    {
                        ip_flow.result.wfd.dhd_ucast.priority = blog_p->wfd.dhd_ucast.priority;
                        ip_flow.result.wfd.dhd_ucast.flowring_idx = blog_p->wfd.dhd_ucast.flowring_idx;
                    }
                    else{
                        ip_flow.result.rnr.priority = blog_p->rnr.priority;
                        ip_flow.result.rnr.flowring_idx = blog_p->rnr.flowring_idx;
                    }
                }
            }
#endif
            rc = rc ? rc : rdpa_ip_class_flow_set(ip_class, PKTRUNNER_RDPA_KEY(accel, index), &ip_flow);
        }
        break;
    default:
        rc =-1;
    }

    return rc;
}

static int blog_parse_mcast_request_key(Blog_t *blog_p, rdpa_iptv_channel_key_t *key)
{
    rdpa_iptv_lookup_method lookup_method;

    rdpa_iptv_lookup_method_get(iptv, &lookup_method);

    /* Get VID if necessary */
    if (lookup_method == iptv_lookup_method_group_ip_src_ip_vid ||
        lookup_method == iptv_lookup_method_mac_vid)
    {
        if (blog_p->vtag_num != 1)
        {
            protoDebug("%d tags set in blog_filter. Only a single VID must be specified, required by lookup method\n", blog_p->vtag_num);
            return BDMF_ERR_PARSE;
        }
        /* Get VID from vlan tag 0. note that tag 1 is not supported */
        key->vid = htonl(blog_p->vtag[0]);
    }

    /* L2 multicast */
    if (lookup_method == iptv_lookup_method_mac_vid || 
        lookup_method == iptv_lookup_method_mac)    
    {
        uint32_t is_host = 0; 

        if (blog_p->rx.info.bmap.PLD_IPv4)
        {
            is_host = blog_p->rx.tuple.daddr & htonl(0xff000000);

            if (is_host == 0)
            {
                uint32_t daddr = htonl(blog_p->rx.tuple.daddr);
				
                key->mcast_group.mac.b[0] = 0x1;         
                key->mcast_group.mac.b[1] = 0x0;         
                key->mcast_group.mac.b[2] = 0x5e;         
                key->mcast_group.mac.b[3] = (daddr&0x7f0000)>>16; 
                key->mcast_group.mac.b[4] = (daddr&0xff00)>>8; 
                key->mcast_group.mac.b[5] = daddr&0xff; 
            }
            else
            {
                protoDebug("L2 multicast supported for Host control mode only");
                return BDMF_ERR_NOT_SUPPORTED;
            }
        }
        else if (blog_p->rx.info.bmap.PLD_IPv6)
        {
            is_host = blog_p->tupleV6.daddr.p8[0] & 0xff;

            if (is_host == 0)
            {
                key->mcast_group.mac.b[0] = 0x33;         
                key->mcast_group.mac.b[1] = 0x33;         
                key->mcast_group.mac.b[2] = blog_p->tupleV6.daddr.p8[12];
                key->mcast_group.mac.b[3] = blog_p->tupleV6.daddr.p8[13];
                key->mcast_group.mac.b[4] = blog_p->tupleV6.daddr.p8[14];
                key->mcast_group.mac.b[5] = blog_p->tupleV6.daddr.p8[15];
            }
            else
            {
                protoDebug("L2 multicast supported for Host control mode only");
                return BDMF_ERR_NOT_SUPPORTED;
            }
        }
    }
    else
    {
        /* Retrieve group and source IP addresses. */
        if (blog_p->rx.info.bmap.PLD_IPv4)
        {
            /* IGMP */
            key->mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv4;
            key->mcast_group.l3.gr_ip.addr.ipv4 = htonl(blog_p->rx.tuple.daddr);
            key->mcast_group.l3.src_ip.family = bdmf_ip_family_ipv4;
            key->mcast_group.l3.src_ip.addr.ipv4 = htonl(blog_p->rx.tuple.saddr);
        }
        else
        {
            /* MLD. */
            key->mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv6;
            memcpy(&key->mcast_group.l3.gr_ip.addr.ipv6, &blog_p->tupleV6.daddr, sizeof(bdmf_ipv6_t));
            key->mcast_group.l3.src_ip.family = bdmf_ip_family_ipv6;
            memcpy(&key->mcast_group.l3.src_ip.addr.ipv6, &blog_p->tupleV6.saddr, sizeof(bdmf_ipv6_t));
        }
    }

    return 0;
}

static uint32_t runner_activate_mcast(Blog_t *blog_p, uint32_t key_in)
{
    int rc;
    rdpa_iptv_channel_request_t request;
    uint16_t channel_map_idx;
    rdpa_channel_req_key_t request_key;
#if !defined(CONFIG_BCM_FTTDP_G9991)
    bdmf_index wlan_mcast_idx = RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID;
    rdpa_iptv_channel_t channel;
    bdmf_index tmp;
#endif
    PktRunnerFlowKey_t pktRunner_key = {};
    uint32_t accel = PKTRUNNER_ACCEL_MCAST;

    BCM_ASSERT((blog_p!=BLOG_NULL));

    BCM_LOGCODE(if(protoLogDebug)
        { protoPrint("\n::: runner_activate_mcast :::\n"); blog_dump(blog_p); });

    /* Get the multicast group, multicast source and vid */
    memset(&request, 0, sizeof(rdpa_iptv_channel_request_t));
#ifndef CONFIG_BCM_FTTDP_G9991
    request.wlan_mcast_index = RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID;
#endif
    rc = blog_parse_mcast_request_key(blog_p, &request.key);
    if (rc < 0)
    {
        if (rc == BDMF_ERR_PARSE)
        {
            /* BDMF_ERR_PARSE is returned if the number of tags doesn't comply with current iptv lookup method.
               this is not an error, but a way to handle a unexpected blogs with different number of tags */
            return FHW_TUPLE_INVALID;
        }
        goto exit;
    }

    /* Get mcast ckassification result from blog commands */
    rc = blog_parse_mcast_result_get(blog_p, &request.mcast_result);
    if (rc < 0)
        goto exit;

#ifdef MCAST_FORWARD_TO_HOST
    request.mcast_result.action = rdpa_forward_action_host;
#endif

#if !defined(CONFIG_BCM_FTTDP_G9991)
    channel.key = request.key;
    rc = rdpa_iptv_channel_find(iptv, &tmp, &channel);
    if (!rc)
    {
        /* In case channel already exist use existing wlan_mcast entry */
        wlan_mcast_idx = channel.wlan_mcast_index;
    }
    if (blog_p->rx.info.phyHdrType == BLOG_WLANPHY ||
        blog_p->rx.info.phyHdrType == BLOG_NETXLPHY ||
        blog_p->tx.info.phyHdrType == BLOG_WLANPHY || 
        blog_p->tx.info.phyHdrType == BLOG_NETXLPHY)
    {
        rc = pktrunner_wlan_mcast_add(blog_p, &wlan_mcast_idx);
        if (rc < 0)
        {
            protoError("Failed to add wlan_mcast entry %ld\n", wlan_mcast_idx);
            goto exit;
        }
    }
    request.wlan_mcast_index = wlan_mcast_idx;
#else
#ifdef XRDP
    /* Update RX_IF for multi wan */
    if (blog_p->rx.info.phyHdrType == BLOG_GPONPHY)
    {
        protoDebug("source.phy GPON\n");
        request.key.rx_if = rdpa_wan_type_to_if(rdpa_wan_gpon);
    }
    else if (blog_p->rx.info.phyHdrType == BLOG_EPONPHY)
    {
        protoDebug("source.phy EPON\n");
        request.key.rx_if = rdpa_wan_type_to_if(rdpa_wan_epon);
    }
    else if (((struct net_device *)blog_p->rx_dev_p)->priv_flags & IFF_WANDEV)
    {
        protoDebug("source.phy ETH WAN\n");
        request.key.rx_if = rdpa_wan_type_to_if(rdpa_wan_gbe);
    }
    else
    {
        /* LAN */ 
        protoError("LAN flow is not supported: Rx %u, Tx %u", blog_p->rx.info.phyHdrType, blog_p->tx.info.phyHdrType);
    }
#endif
#endif
    rc = rdpa_iptv_channel_request_add(iptv, &request_key, &request);

    if (rc < 0)
        return FHW_TUPLE_INVALID;

    channel_map_idx = request_key_add(request_key.channel_index, blog_p->vtag_num);
    protoDebug("request_key_add; key: %d channel_index: %ld\n", channel_map_idx, request_key.channel_index);

exit:
    blog_parse_mcast_result_put(&request.mcast_result);
    if (rc < 0)
    {
#if !defined(CONFIG_BCM_FTTDP_G9991)
        if (request.wlan_mcast_index != RDPA_WLAN_MCAST_FWD_TABLE_INDEX_INVALID)
        {
            wlan_mcast_idx = request.wlan_mcast_index;
            pktrunner_wlan_mcast_delete(blog_p, &wlan_mcast_idx);
        }
#endif
        protoError("Failed to add channel request, error %d\n", rc);
        PKTRUNNER_STATE(accel).hw_add_fail++;
        PKTRUNNER_STATE(accel).failures++;
        protoInfo("cumm_failures<%u>", PKTRUNNER_STATE(accel).failures);
        return FHW_TUPLE_INVALID;
    }
    else
    {
        int pktRunner_flow_idx;
        if (key_in != FHW_TUPLE_INVALID)
        {
            pktRunner_flow_idx = __pktRunnerFlowIdx(key_in);
            /* Mulitcast Modification */
            if (pktRunner_flow_idx < 0 || pktRunner_flow_idx >= PKTRUNNER_DATA(accel).max_flow_idxs)
            {
                protoError("Mcast modify; Invalid key_in 0x%08x pktRunner_flow_idx %d new 0x%08x\n", key_in, pktRunner_flow_idx, channel_map_idx);
                return FHW_TUPLE_INVALID;
            }
            if (__pktRunnerAccelIdx(key_in) != PKTRUNNER_ACCEL_MCAST)
            {
                protoError("Mcast modify; Invalid key_in 0x%08x accel %u new 0x%08x\n", key_in, __pktRunnerAccelIdx(key_in), channel_map_idx);
                return FHW_TUPLE_INVALID;
            }
            /* Verify that already stored rdpa_key is same as the new key */
            if (PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) != channel_map_idx)
            {
                protoError("Mcast modify; key_in 0x%08x stored 0x%08x new 0x%08x \n", 
                           key_in, PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx), channel_map_idx);
                return FHW_TUPLE_INVALID;
            }
        }
        else
        {
            pktRunner_flow_idx = idx_pool_get_index(&PKTRUNNER_DATA(accel).idx_pool);
            if (pktRunner_flow_idx < 0 )
            {
                PKTRUNNER_STATE(accel).no_free_idx++;
                protoError("Mcast activate; No free indexes\n");
                return FHW_TUPLE_INVALID;
            }
        }
        pktRunner_key.word = __pktRunnerBuildKey(accel, PKTRNR_FLOW_TYPE_MC, pktRunner_flow_idx);
        PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) = channel_map_idx;
        PKTRUNNER_STATS(accel, pktRunner_flow_idx) = 0;
        PKTRUNNER_STATE(accel).activates++;
        PKTRUNNER_STATE(accel).active++;
        protoDebug("::: runner_activate_mcast flowId<%03u> cumm_activates<%u> :::\n\n",
            channel_map_idx, PKTRUNNER_STATE(accel).activates);
    }
    return pktRunner_key.word;
}


/* Clears FlowCache association(s) to pktrunner entries. This local function MUST be called
   with the Protocol Layer Lock taken. */
static void __clear_fcache(const FlowScope_t scope)
{
    /* For now use UCAST -- this clear_fcache will be removed in future */
    uint32_t accel = PKTRUNNER_ACCEL_UCAST;

    /* Upcall into FlowCache */
    if(fc_clear_hook_runner != (FC_CLEAR_HOOK)NULL)
    {
        PKTRUNNER_STATE(accel).flushes += fc_clear_hook_runner(0, scope);
    }

    pktrunner_flow_remove_all();

    protoDebug("scope<%s> cumm_flushes<%u>",
               (scope == System_e) ? "System" : "Match",
               PKTRUNNER_STATE(accel).flushes);
}

#ifndef CONFIG_BCM_FTTDP_G9991  
void build_gre_tunnel_header(Blog_t *blog_p, rdpa_tunnel_cfg_t *rdpa_tunnel)
{
    uint8_t     *data_p = rdpa_tunnel->tunnel_header;
    BlogTuple_t *deltx_p = &blog_p->deltx_tuple;
    BlogTupleV6_t  *deltxV6_p = &blog_p->del_tupleV6;
    BlogGre_t   *gre_p = &blog_p->gretx;
    uint16_t    proto;
    uint16_t    icsum16, ip_hdr_len;
    uint16_t    l3_total_length, csum;

    if (blog_p->tx.info.bmap.GRE_ETH)
        proto = BLOG_ETH_P_ETH_BRIDGING;
    else
        proto = BLOG_ETH_P_IPV4;

    /* Copy tunnel L2 */                    
    memcpy(data_p, blog_p->tx.l2hdr, blog_p->tx.length);   
    data_p += blog_p->tx.length;
    rdpa_tunnel->layer3_offset = rdpa_tunnel->tunnel_header_length = blog_p->tx.length;

    if (blog_p->tx.info.bmap.DEL_IPv4)
    {
        ip_hdr_len = BLOG_IPV4_HDR_LEN;
        l3_total_length = ip_hdr_len + gre_p->hlen;
        /* compute incremental checksum for ipid, total length and tos fields */
        icsum16 = _compute_icsum16( 0, 0, htons(l3_total_length) );
        csum = _apply_icsum( deltx_p->check, (__force uint32_t)icsum16 );

        /* Insert IPV4 Hdr */
        *((uint16_t *)data_p + 0) = htons(0x4500 | deltx_p->tos);
        *((uint16_t *)data_p + 1) = htons(l3_total_length);
        *((uint16_t *)data_p + 2) = gre_p->ipid - htons(1); 
        *((uint16_t *)data_p + 3) = (gre_p->fragflag ? htons(BLOG_IP_FLAG_DF) : 0);
        *((uint16_t *)data_p + 4) = htons((deltx_p->ttl << 8) | (BLOG_IPPROTO_GRE & 0xFF));
        *((uint16_t *)data_p + 5) = csum;
        memcpy((uint8_t*)&((BlogIpv4Hdr_t *)data_p)->sAddr, (uint8_t*)&deltx_p->saddr, sizeof(deltx_p->saddr) + sizeof(deltx_p->daddr));

        rdpa_tunnel->local_ip.family = bdmf_ip_family_ipv4;
        rdpa_tunnel->local_ip.addr.ipv4 = htonl(deltx_p->saddr);
    }
    else
    {
        ip_hdr_len = BLOG_IPV6_HDR_LEN;

        /* Insert IPV6 Hdr */
        *((uint16_t *)data_p + 0) = htons(0x6000);
        *((uint16_t *)data_p + 1) = htons(0x0000);
        *((uint16_t *)data_p + 2) = gre_p->hlen;
        *((uint16_t *)data_p + 3) = htons(((BLOG_IPPROTO_GRE & 0xFF) << 8) | (deltxV6_p->rx_hop_limit));
        memcpy((uint8_t*)&((BlogIpv6Hdr_t *)data_p)->sAddr, (uint8_t*)&deltxV6_p->saddr, sizeof(deltxV6_p->saddr) + sizeof(deltxV6_p->daddr));

        rdpa_tunnel->local_ip.family = bdmf_ip_family_ipv6;
        memcpy(&rdpa_tunnel->local_ip.addr, deltxV6_p->saddr.p8, sizeof(deltxV6_p->saddr));
    }
    data_p += ip_hdr_len;

    rdpa_tunnel->gre_proto_offset =  rdpa_tunnel->layer3_offset + ip_hdr_len + sizeof(gre_p->gre_flags.u16);

    *((uint16_t *)data_p + 0) = htons(gre_p->gre_flags.u16);
    *((uint16_t *)data_p + 1) = htons(proto);
    if(gre_p->gre_flags.keyIe)
    {
        *((uint32_t *)data_p + 1) = htonl(gre_p->key);
    }

    rdpa_tunnel->tunnel_header_length += ip_hdr_len + gre_p->hlen;

    if (rdpa_tunnel->tunnel_header_length > RDPA_MAX_TUNNEL_HEADER_LEN)
        protoError("tunnel header length(%d) > RDPA_MAX_TUNNEL_HEADER_LEN(%d)", rdpa_tunnel->tunnel_header_length, RDPA_MAX_TUNNEL_HEADER_LEN);

    rdpa_tunnel->tunnel_type = (proto == BLOG_ETH_P_ETH_BRIDGING ? rdpa_tunnel_l2gre : rdpa_tunnel_l3gre);
}

void build_dslite_tunnel_header(Blog_t *blog_p, rdpa_tunnel_cfg_t *rdpa_tunnel)
{
    uint8_t     *data_p = rdpa_tunnel->tunnel_header;
    uint16_t    ip_hdr_len;

    /* Copy tunnel L2 */                    
    rdpa_tunnel->layer3_offset = 0;

    ip_hdr_len = BLOG_IPV6_HDR_LEN;

    /* Insert IPV6 Hdr */
    *((uint16_t *)data_p + 0) = htons(0x6000);
    *((uint16_t *)data_p + 1) = htons(0x0000);
    *((uint16_t *)data_p + 2) = 0;
    *((uint16_t *)data_p + 3) = htons(((BLOG_IPPROTO_IPIP & 0xFF) << 8) | (blog_p->tupleV6.rx_hop_limit));
    memcpy((uint8_t*)&((BlogIpv6Hdr_t *)data_p)->sAddr, (uint8_t*)&blog_p->tupleV6.saddr, sizeof(blog_p->tupleV6.saddr) + sizeof(blog_p->tupleV6.daddr));

    rdpa_tunnel->local_ip.family = bdmf_ip_family_ipv6;
    memcpy(&rdpa_tunnel->local_ip.addr, blog_p->tupleV6.saddr.p8, sizeof(blog_p->tupleV6.saddr));
    
    data_p += ip_hdr_len;

    rdpa_tunnel->tunnel_header_length = ip_hdr_len;

    if (rdpa_tunnel->tunnel_header_length > RDPA_MAX_TUNNEL_HEADER_LEN)
        protoError("tunnel header length(%d) > RDPA_MAX_TUNNEL_HEADER_LEN(%d)", rdpa_tunnel->tunnel_header_length, RDPA_MAX_TUNNEL_HEADER_LEN);

    rdpa_tunnel->tunnel_type = rdpa_tunnel_dslite;
}
#endif


static void pktrunner_bind(int enable)
{
    FhwBindHwHooks_t no_hw_hooks = {};
    FhwBindHwHooks_t hw_hooks_ucast = {};
    FhwBindHwHooks_t hw_hooks_mcast = {};

    /* Initialize the clear function pointer */
    no_hw_hooks.fhw_clear_fn = &fc_clear_hook_runner;

    /* Initialize UCAST --- START */
    hw_hooks_ucast.activate_fn = (HOOKP32)runner_activate_ucast;
    hw_hooks_ucast.deactivate_fn = (HOOK4PARM)runner_deactivate_ucast;
    hw_hooks_ucast.update_fn = (HOOK3PARM)runner_update_ucast;
    hw_hooks_ucast.refresh_fn = (HOOK3PARM)runner_refresh_ucast;
    hw_hooks_ucast.refresh_pathstat_fn = (HOOK3PARM)runnerRefreshPathStat;     
    hw_hooks_ucast.fhw_clear_fn = &fc_clear_hook_runner;
    hw_hooks_ucast.reset_stats_fn =(HOOK32)runnerResetStats;
    hw_hooks_ucast.cap = (1<<HW_CAP_IPV4_UCAST) | (1<<HW_CAP_IPV6_UCAST) | (1<<HW_CAP_IPV6_TUNNEL);
#ifdef XRDP 
    hw_hooks_ucast.cap |= (1<<HW_CAP_L2_UCAST);
#endif    
       
    hw_hooks_ucast.clear_fn = (HOOK32)runner_clear;
    hw_hooks_ucast.max_ent = PKTRUNNER_MAX_FLOWS;
#if defined(XRDP) && !defined(CONFIG_BCM_FTTDP_G9991)
    /* Number of HW_pathstat needs to match with HWACC counter group assignment */
    hw_hooks_ucast.max_hw_pathstat = RDPA_IP_CLASS_MAX_PATHS;
#else
    hw_hooks_ucast.max_hw_pathstat = 0;
#endif
    /* Initialize UCAST --- END */
                
    /* Initialize MCAST --- START */
    hw_hooks_mcast.activate_fn = (HOOKP32)runner_activate_mcast;
    hw_hooks_mcast.deactivate_fn = (HOOK4PARM)runner_deactivate_mcast;
    hw_hooks_mcast.update_fn = (HOOK3PARM)NULL;
    hw_hooks_mcast.refresh_fn = (HOOK3PARM)runner_refresh_mcast;
    hw_hooks_mcast.refresh_pathstat_fn = (HOOK3PARM)NULL;
    hw_hooks_mcast.fhw_clear_fn = &fc_clear_hook_runner;
    hw_hooks_mcast.reset_stats_fn =(HOOK32)runnerResetStats;
    hw_hooks_mcast.cap = (1<<HW_CAP_IPV4_MCAST) | (1<<HW_CAP_IPV6_MCAST);
    hw_hooks_mcast.clear_fn = (HOOK32)runner_clear;
    hw_hooks_mcast.max_ent = PKTRUNNER_MAX_FLOWS_MCAST;
    /* pathstat not supported in Mcast */
    hw_hooks_mcast.max_hw_pathstat = 0;
    /* Initialize MCAST --- START */

    /* Block flow-cache from packet processing and try to push the flows */
    blog_lock(); 

    fhw_bind_hw(FHW_PRIO_0, enable ? &hw_hooks_ucast : &no_hw_hooks);
    fhw_bind_hw(FHW_PRIO_1, enable ? &hw_hooks_mcast : &no_hw_hooks);
    
    protoNotice("%s runner binding to Flow Cache", enable ? "Enabled" :
        "Disabled");

    PKTRUNNER_STATE(PKTRUNNER_ACCEL_UCAST).status = enable;
    PKTRUNNER_STATE(PKTRUNNER_ACCEL_MCAST).status = enable;

    blog_unlock();
}

int __init _pktRunnerAccelInit(uint32_t accel, uint32_t num_flows)
{
    int ret;   
    pktRunner_data_t *pktRunner_p;
    char owner_name[16];

    snprintf(owner_name, sizeof(owner_name), "PktRnr[%d]",accel);

    pktRunner_p = &pktRunner_data_g[accel];

    pktRunner_p->max_flow_idxs = num_flows;

    ret = idx_pool_init(&pktRunner_p->idx_pool, num_flows, owner_name);
    if (ret)
    {
        return ret;
    }

    pktRunner_p->rdpa_flow_key_p = kmalloc(sizeof(*pktRunner_p->rdpa_flow_key_p) * num_flows, GFP_KERNEL);

    if (!pktRunner_p->rdpa_flow_key_p)
    {
        idx_pool_exit(&pktRunner_p->idx_pool);
        return -1;
    }

    pktRunner_p->flowResetStats_p = kmalloc(sizeof(*pktRunner_p->flowResetStats_p)* num_flows, GFP_KERNEL);

    if (!pktRunner_p->flowResetStats_p)
    {
        idx_pool_exit(&pktRunner_p->idx_pool);
        kfree(pktRunner_p->rdpa_flow_key_p);
        return -1;
    }

    return ret;
}

int __exit _pktRunnerAccelExit(uint32_t accel)
{
    int ret;   
    pktRunner_data_t *pktRunner_p;

    pktRunner_p = &pktRunner_data_g[accel];


    ret = idx_pool_exit(&pktRunner_p->idx_pool);
    if (ret)
    {
        return ret;
    }

    kfree(pktRunner_p->rdpa_flow_key_p);
    kfree(pktRunner_p->flowResetStats_p);

    return ret;
}

/* Binds the Runner Protocol Layer handler functions to Flow Cache hooks. */
int pktrunner_enable(void)
{
    int ret = 0;
    memset(&pktRunner_data_g, 0, sizeof(pktRunner_data_g));
    /* Initialize UCAST accelerator */
    ret = _pktRunnerAccelInit(PKTRUNNER_ACCEL_UCAST, PKTRUNNER_MAX_FLOWS);
    /* Initialize MCAST accelerator */
    ret = ret ? ret : _pktRunnerAccelInit(PKTRUNNER_ACCEL_MCAST, PKTRUNNER_MAX_FLOWS_MCAST);

    pktrunner_bind(1);
    BCM_ASSERT((fc_clear_hook_runner != (FC_CLEAR_HOOK)NULL));

    return ret;
}

/* Clears all active Flow Cache associations with Runner.
 * Unbind all flow cache to Runner hooks. */
void pktrunner_disable(void)
{
    pktrunner_bind(0);

    /* Clear system wide active FlowCache associations, and disable learning. */
    __clear_fcache(System_e);

    _pktRunnerAccelExit(PKTRUNNER_ACCEL_UCAST);
    _pktRunnerAccelExit(PKTRUNNER_ACCEL_MCAST);

}

int pktrunner_debug(int log_level)
{
    if(log_level >= 0 && log_level < BCM_LOG_LEVEL_MAX)
    {
        bcmLog_setLogLevel(BCM_LOG_ID_PKTRUNNER, log_level);
    }
    else
    {
        protoError("Invalid Log level %d (max %d)", log_level, BCM_LOG_LEVEL_MAX);
        return -1;
    }

    return 0;
}



#ifdef XRDP
static int l2_class_created_here;
#endif
static int ip_class_created_here;

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
static void dhd_helper_destruct(void)
{
    int i;

    for (i = 0; i < MAX_NUM_OF_RADIOS; i++)
    {
        if (dhd_helper_obj[i])
            bdmf_put(dhd_helper_obj[i]);
    }
}
#endif

static void cleanup_rdpa_objects(void)
{
    if (iptv)
    {
#ifndef CONFIG_BCM_FTTDP_G9991
        pktrunner_wlan_mcast_destruct();
#endif
        bdmf_destroy(iptv);
    }
    if (ip_class)
    {
        if (ip_class_created_here)
            bdmf_destroy(ip_class);
        else
            bdmf_put(ip_class);
    }
#ifdef XRDP    
    if (l2_class)
    {
        if (l2_class_created_here)
            bdmf_destroy(l2_class);
        else
            bdmf_put(l2_class);
    }
#endif
#ifndef CONFIG_BCM_FTTDP_G9991
    pktrunner_wlan_destruct();
#endif

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) 
    dhd_helper_destruct();
#endif

    bdmf_put(system_obj);
}

#if defined(XRDP) && !defined(CONFIG_BCM_FTTDP_G9991)
static int pktrunner_fc_accel_mode_set( uint32_t accel_mode )
{
    int rc = 0;
    
    if (accel_mode == BLOG_ACCEL_MODE_L23)
    {
        protoNotice("Fcache change acceleration mode to MODE_L2+L3");
        
        if (l2_class_created_here)
            return 0;

        rc = rdpa_l2_class_get(&l2_class);
        if (rc)
        {
            BDMF_MATTR(l2_class_attrs, rdpa_l2_class_drv());
            
            protoNotice("l2_class does not exists, Creating...");
         
            rdpa_l2_class_flow_idx_pool_ptr_set(l2_class_attrs, &rdpa_shared_flw_idx_pool_g);
            rc = bdmf_new_and_set(rdpa_l2_class_drv(), NULL, l2_class_attrs, &l2_class);
            if (rc)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "rdpa l2_class object does not exist and can't be created.\n");
                goto go_out;
            }
            l2_class_created_here = 1;
        }
    }
    else
    {
        protoNotice("Fcache change acceleration mode to MODE_L3");

        if (l2_class)
        {
            protoNotice("Deleting l2_class object");
            if (l2_class_created_here)
                bdmf_destroy(l2_class);
            else
                bdmf_put(l2_class);

            l2_class = NULL;
            l2_class_created_here = 0;
        }
    }
    
go_out:
    return rc;
}
#endif

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
static void dhd_helper_construct(void)
{
    int i = 0;
    for ( ;i < MAX_NUM_OF_RADIOS; i++)
    {
        dhd_helper_obj[i] = NULL;
    }
}
#endif

int __init runnerProto_construct(void)
{
    int rc;
    bdmf_object_handle port_obj;
    rdpa_wan_type wan_type = rdpa_wan_none;
    BDMF_MATTR(iptv_attrs, rdpa_iptv_drv());
#if !defined(BCM_FCACHE_IP_CLASS_BYPASS)
    rdpa_l4_filter_cfg_t l4_filter_cfg = {
        .action = rdpa_forward_action_forward,
    };
    rdpa_fc_bypass fc_bypass_mask;
    BDMF_MATTR(ip_class_attrs, rdpa_ip_class_drv());
#endif

#if (BLOG_HDRSZ_MAX > 32)
#error Runner FW does not support L2 header bigger then 32
#endif

#if !defined(BCM_FCACHE_IP_CLASS_BYPASS)
    /* Only create for L2+L3; Mcast needs separate pool */
    memset(&rdpa_shared_flw_idx_pool_g, 0, sizeof(rdpa_shared_flw_idx_pool_g));
    rc = rdpa_flow_idx_pool_init(&rdpa_shared_flw_idx_pool_g, PKTRUNNER_MAX_FLOWS, "L2L3-class");
    if (rc)
    {
        return rc;
    }
#endif

    rdpa_system_get(&system_obj);

    DLIST_INIT(&mcast_hw_channel_key_list);

    bcmLog_setLogLevel(BCM_LOG_ID_PKTRUNNER, BCM_LOG_LEVEL_ERROR);

#if !defined(BCM_FCACHE_IP_CLASS_BYPASS)
    rc = rdpa_ip_class_get(&ip_class);
    if (rc)
    {
        rdpa_ip_class_flow_idx_pool_ptr_set(ip_class_attrs, &rdpa_shared_flw_idx_pool_g);
        rc = bdmf_new_and_set(rdpa_ip_class_drv(), NULL, ip_class_attrs, &ip_class);
        if (rc)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "rdpa ip_class object does not exist and can't be created.\n");
            goto error;
        }
        ip_class_created_here = 1;
    }

    rc = rc ? rc : rdpa_ip_class_fc_bypass_get(ip_class, &fc_bypass_mask);
    if (rc)
    	BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "ip_class can't get fc_bypass attribute\n");

    fc_bypass_mask &= ~RDPA_IP_CLASS_MASK_US_WLAN;

#ifndef XRDP
    rc = rc ? rc : rdpa_ip_class_fc_bypass_set(ip_class, fc_bypass_mask);
    if (rc)
    	BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "ip_class can't set fc_bypass attribute\n");
#endif

#ifdef XRDP
    rc = rc ? rc : rdpa_ip_class_key_type_set(ip_class, RDPA_IP_CLASS_6TUPLE);
    if (rc)
    	BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "ip_class can't set key_type attribute\n");
#endif

#endif /* !BCM_FCACHE_IP_CLASS_BYPASS */

    {
        rdpa_system_init_cfg_t init_cfg;
        rdpa_iptv_lookup_method method = iptv_lookup_method_group_ip_src_ip_vid;

        rdpa_system_init_cfg_get(system_obj, &init_cfg);
        
        rc = rdpa_port_get(rdpa_if_wan0, &port_obj); /* FIXME : Multi-WAN XPON */
        if (rc)
        {
           BCM_LOG_INFO(BCM_LOG_ID_PKTRUNNER, "faild to get port rdpa_if_wan0");
        }
        else
        {
            rdpa_port_wan_type_get(port_obj, &wan_type);
            bdmf_put(port_obj);
        }
        
        /* CMS MCPD is expected to work with group_ip_src_ip only in GPON SFU */
        if ((wan_type == rdpa_wan_gpon || wan_type == rdpa_wan_xgpon) &&
            init_cfg.ip_class_method == rdpa_method_none)
        {
            method = iptv_lookup_method_group_ip_src_ip;
        }

        rdpa_iptv_lookup_method_set(iptv_attrs, method);
        rc = bdmf_new_and_set(rdpa_iptv_drv(), NULL, iptv_attrs, &iptv);
        if (rc)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "rdpa iptv object does not exist and can't be created\n");
            goto error;
        }
    }

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
    dhd_helper_construct();
#endif  

#if !defined(BCM_FCACHE_IP_CLASS_BYPASS)
    l4_filter_cfg.protocol = IPPROTO_GRE;
#ifndef XRDP
    rc = rdpa_ip_class_l4_filter_set(ip_class, rdpa_l4_filter_gre, &l4_filter_cfg);
    if (rc)
    	BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "ip_class can't set l4_filter[gre] attribute\n");
#endif

    /* enable rdpa l4 esp filter */
    l4_filter_cfg.protocol = IPPROTO_ESP;
    rc = rdpa_ip_class_l4_filter_set(ip_class, rdpa_l4_filter_esp, &l4_filter_cfg);
    if (rc)
    	BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "ip_class can't set l4_filter[esp] attribute\n");
#endif /* !BCM_FCACHE_IP_CLASS_BYPASS */

    protoNotice("Reset and initialized pktrunner protocol Layer");
#if !defined(CONFIG_BCM_FTTDP_G9991)  
    rc = pktrunner_wlan_mcast_construct();
    if (rc)
    	BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, "Can't create wlan_mcast object\n");
#endif
    
#ifdef XRDP
    runnerHost_construct();

#if !defined(CONFIG_BCM_FTTDP_G9991)
    /* bind to acceleration mode function hook used by blog/flow_cache */
    blog_accel_mode_set_fn = (blog_accel_mode_set_t) pktrunner_fc_accel_mode_set;

    /* Set the Runner acceleration mode to be in sync with blog/flow cache */
    pktrunner_fc_accel_mode_set( blog_support_get_accel_mode() );
#endif /* CONFIG_BCM_FTTDP_G9991 */
#endif /* XRDP */

    pktrunner_enable();


#ifndef CONFIG_BCM_FTTDP_G9991
    pktrunner_wlan_construct();
#endif
    return 0;

error:

    cleanup_rdpa_objects();
#if !defined(BCM_FCACHE_IP_CLASS_BYPASS)
    rdpa_flow_idx_pool_exit(&rdpa_shared_flw_idx_pool_g);
#endif
    return rc;
}

void __exit runnerProto_destruct(void)
{
    mcast_hw_channel_key_entry_t *key_entry, *tmp_entry;

    pktrunner_disable();
    cleanup_rdpa_objects();

#if !defined(BCM_FCACHE_IP_CLASS_BYPASS)
    rdpa_flow_idx_pool_exit(&rdpa_shared_flw_idx_pool_g);
#endif

    DLIST_FOREACH_SAFE(key_entry, &mcast_hw_channel_key_list, list, tmp_entry)
    {
        DLIST_REMOVE(key_entry, list);
        bdmf_free(key_entry);
    }

#ifdef XRDP    
    runnerHost_destruct();
#endif    
}


