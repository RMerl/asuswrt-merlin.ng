/*
<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
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
*
* File Name  : ptkrunner_proto.c
*
* Description: This implementation supports the dynamically learnt Flows in
*              xDSL platforms.
*
*******************************************************************************
*/

#if defined(RDP_SIM)
#include "pktrunner_rdpa_sim.h"
#else
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <net/ipv6.h>
#include "fcachehw.h"
#include "bcmenet.h"
#include "clk_rst.h"
#endif

#include <rdpa_api.h>
#include <rdpa_flow_idx_pool.h>
#if !defined(RDP_SIM)
#include <rdpa_int.h>
#else
#include <../rdpa_int.h>
#endif

#include "pktrunner_proto.h"
#include "pktrunner_ucast.h"
#if !defined(RDP_SIM)
#include "pktrunner_host.h"
#endif
#include "pktrunner_mcast.h"
#include "pktrunner_wlan_mcast.h"
#include "pktrunner_l2_ucast.h"

#include "cmdlist_api.h"

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
#include "wfd_dev.h"
#endif

#if defined(XRDP)
#include "pktrunner_mcast_whitelist.h"
#endif

#if !defined(RDP_SIM)
#include "linux/bcm_skb_defines.h"
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))
#endif

/*******************************************************************************
 *
 * Global Variables and Definitions
 *
 *******************************************************************************/

typedef struct {
    uint32_t status;        /* status: Enable=1 or Disable=0    */
    uint32_t activates;     /* number of activate (downcalls)   */
    uint32_t deactivates;   /* number of deactivate (downcalls) */
    uint32_t failures;      /* number of activate failures      */
    uint32_t l3_errors;     /* number of L3 flow errors         */
    uint32_t l2_errors;     /* number of L2 flow errors         */
    uint32_t mc_errors;     /* number of MC flow errors         */
    uint32_t hash_colli;    /* number of hask collision         */
    uint32_t no_free_idx;   /* number of no_free_idx errors     */
    uint32_t flushes;       /* number of clear (upcalls)        */
    uint32_t active;        /* number of active flows           */
    uint32_t ucast_active;  /* number of active unicast flows   */
    uint32_t mcast_active;  /* number of active mcast flows     */
    uint32_t max_flows;     /* Max allowed flows by HW(special/63132)*/
} pktRunner_state_t;

/* PktRunner Accelerator instance data */
typedef struct {
    uint32_t            max_flow_idxs;      /* Max number of flows supported by this accelerator */
    pktRunner_state_t   state;
    IdxPool_t           idx_pool;           /* pktRunner index pool */
    uint32_t            *rdpa_flow_key_p;   /* mapped RDPA Flow ID (opaque 32bit value) */
    int                 *ref_cnt_p;         /* ref_cnt per index */
    bdmf_number         *flowResetStats_p;  /* Flow Reset Stats */
    void*               *cmdlist_buffer_pointers;   /* used by L2 and L3-Ucast for command list overflow */
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
static rdpa_flow_idx_pool_t rdpa_shared_flw_idx_pool_g;  /* shared flow index pool as such has no relation 
                                                          * with pktRunner but needed so can be shared across objects */
static flow_display_info_t *rdpa_shared_flow_disp_pool_p;/* shared flow display pool as such has no relation 
                                                          * with pktRunner but needed so can be shared across objects */

#define PKTRUNNER_DATA(accel)                   (pktRunner_data_g[accel])
#define PKTRUNNER_STATE(accel)                  (PKTRUNNER_DATA(accel).state)
#define PKTRUNNER_STATS(accel, idx)             (PKTRUNNER_DATA(accel).flowResetStats_p[idx])
#define PKTRUNNER_RDPA_KEY(accel, idx)          (PKTRUNNER_DATA(accel).rdpa_flow_key_p[idx])
#define PKTRUNNER_CMDLIST_PTR(accel, idx)       (PKTRUNNER_DATA(accel).cmdlist_buffer_pointers[idx])
#define PKTRUNNER_REF_CNT(accel, idx)           (PKTRUNNER_DATA(accel).ref_cnt_p[idx])

static int __pktRunnerClearRstStats(uint32_t pktRunner_Key);

static void inline __pktRunnerIncErr(uint32_t accel, uint32_t flowType)
{
    if (flowType == PKTRNR_FLOW_TYPE_L3) PKTRUNNER_STATE(accel).l3_errors++;
    else if (flowType == PKTRNR_FLOW_TYPE_L2) PKTRUNNER_STATE(accel).l2_errors++;
    else if (flowType == PKTRNR_FLOW_TYPE_MC) PKTRUNNER_STATE(accel).mc_errors++;
}

typedef union {
    bdmf_number    num;
    rdpa_stat_t    rdpastat;
} pktRunner_flowStat_t;

static int inline __pktRunner_reverse_get_index(uint32_t accel, uint32_t *pktRunner_idx, uint32_t rdpa_key)
{
    uint32_t idx;
    for (idx = 0; idx < PKTRUNNER_DATA(accel).max_flow_idxs; idx++)
    {
        /* TODO : Just matching the rdpa_key may not be sufficient because rpda_key
         * could be same for two different type of RDPA flows 
         * ex ucast/l2_ucast & mcast could have same rdpa_key */
        if (PKTRUNNER_RDPA_KEY(accel,idx) == rdpa_key)
        {
            *pktRunner_idx = idx;
            return 0;
        }
    }
    return -1;
}

#if defined CC_PKTRUNNER_PROCFS
uint32_t pktRunnerGetState(struct seq_file *sf, uint32_t accel)
{
    uint32_t bytes = 0;
    uint32_t max_flows;

    max_flows = PKTRUNNER_STATE(accel).max_flows;
    if (!max_flows)
    {
        max_flows = PKTRUNNER_DATA(accel).max_flow_idxs;
    }
    bytes += seq_printf(sf, "PKT_RUNNER[%u]:\n", accel);
    bytes += seq_printf(sf, "status         : %u\n",PKTRUNNER_STATE(accel).status);
    bytes += seq_printf(sf, "activates      : %u\n",PKTRUNNER_STATE(accel).activates);
    bytes += seq_printf(sf, "deactivates    : %u\n",PKTRUNNER_STATE(accel).deactivates);
    bytes += seq_printf(sf, "failures       : %u\n",PKTRUNNER_STATE(accel).failures);
    bytes += seq_printf(sf, "l3_errors      : %u\n",PKTRUNNER_STATE(accel).l3_errors);
    bytes += seq_printf(sf, "l2_errors      : %u\n",PKTRUNNER_STATE(accel).l2_errors);
    bytes += seq_printf(sf, "mc_errors      : %u\n",PKTRUNNER_STATE(accel).mc_errors);
    bytes += seq_printf(sf, "hash_collision : %u\n",PKTRUNNER_STATE(accel).hash_colli);
    bytes += seq_printf(sf, "no_free_idx    : %u\n",PKTRUNNER_STATE(accel).no_free_idx);
    bytes += seq_printf(sf, "flushes        : %u\n",PKTRUNNER_STATE(accel).flushes);
    bytes += seq_printf(sf, "active         : %u\n",PKTRUNNER_STATE(accel).active);
    bytes += seq_printf(sf, "ucast_active   : %u\n",PKTRUNNER_STATE(accel).ucast_active);
    bytes += seq_printf(sf, "mcast_active   : %u\n",PKTRUNNER_STATE(accel).mcast_active);
    bytes += seq_printf(sf, "max_flows      : %u\n",max_flows);
    bytes += seq_printf(sf, "Flow_idx_in_use: %u\n",idx_pool_num_in_use(&PKTRUNNER_DATA(accel).idx_pool));
    bytes += seq_printf(sf, "----------------------------\n");
    bytes += cmdlist_print_stats(sf);
    return bytes;
}
#endif /* CC_PKTRUNNER_PROCFS */
/*******************************************************************************
 *
 * Public API
 *
 *******************************************************************************/

int __fhwPktRunnerActivate(Blog_t *blog_p, uint32_t key_in, uint32_t accel)
{
    bcmFun_t *bcmFunPrepend = bcmFun_get(BCM_FUN_ID_RUNNER_PREPEND);
    BCM_runnerPrepend_t prepend = { .blog_p = blog_p, .size = 0 };
    int rdpa_key;
    int pktRunner_flow_idx;
    PktRunnerFlowKey_t pktRunner_Key = {.word = key_in};
    int ret_err = BDMF_ERR_OK;
        
    BCM_ASSERT(blog_p != BLOG_NULL);

    __debug("\n::: runner_activate :::\n\n");
    __dumpBlog(blog_p);

//    __debug("\n%s: ************** New Flow **************\n\n", __FUNCTION__);

    if(PKTRUNNER_STATE(accel).max_flows && !blog_p->rx.multicast && PKTRUNNER_STATE(accel).ucast_active >= PKTRUNNER_STATE(accel).max_flows)
    {
        __logInfo("Exceeding max allowed flows - abort");

        goto abort_activate;
    }

    if(bcmFunPrepend)
    {
        bcmFunPrepend(&prepend);

        if(prepend.size > CMDLIST_PREPEND_SIZE_MAX)
        {
            __logError("Invalid prepend data size, aborting flow creation: size %d", prepend.size);

            goto abort_activate;
        }
    }

    /* XXX: add GRE */
    if(blog_p->rx.info.bmap.PLD_L2 )
    {
        void *cmdlist_buffer_p = NULL;

        pktRunner_flow_idx = idx_pool_get_index(&PKTRUNNER_DATA(accel).idx_pool);
        if (pktRunner_flow_idx < 0)
        {
            __logInfo("No free pkt runner indexes for accel <%d>",accel);

            PKTRUNNER_STATE(accel).l2_errors++;
            PKTRUNNER_STATE(accel).no_free_idx++;
            goto abort_activate;
        }

        rdpa_key = runnerL2Ucast_activate(blog_p, prepend.data, prepend.size, &cmdlist_buffer_p, &ret_err);

        if(rdpa_key == FHW_TUPLE_INVALID)
        {
            __logInfo("Could not runnerL2Ucast_activate");

            idx_pool_return_index(&PKTRUNNER_DATA(accel).idx_pool, pktRunner_flow_idx);
            PKTRUNNER_STATE(accel).l2_errors++;
            if( !ret_err )
                PKTRUNNER_STATE(accel).hash_colli++;
            goto abort_activate;
        }

        PKTRUNNER_STATE(accel).activates++;
        PKTRUNNER_STATE(accel).active++;
        PKTRUNNER_STATE(accel).ucast_active++;

        pktRunner_Key.word = __pktRunnerBuildKey(accel, PKTRNR_FLOW_TYPE_L2, pktRunner_flow_idx);

        PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) = rdpa_key;
        PKTRUNNER_CMDLIST_PTR(accel, pktRunner_flow_idx) = cmdlist_buffer_p;

        __debug("::: runnerL2Ucast_activate: flow <%u:%u>, cumm_activates <%u> :::\n\n",
                pktRunner_Key.flow_type, pktRunner_Key.flow_idx, PKTRUNNER_STATE(accel).activates);

        __pktRunnerClearRstStats(pktRunner_Key.word);
        return pktRunner_Key.word;
    } 
    else
    {
        if((blog_p->key.protocol != IPPROTO_UDP) &&
           (blog_p->key.protocol != IPPROTO_TCP) &&
           (blog_p->key.protocol != IPPROTO_IPV6) &&
           (blog_p->key.protocol != IPPROTO_IPIP) &&
           (blog_p->key.protocol != IPPROTO_GRE))
        {
            __logInfo("Flow Type proto<%d> is not supported", blog_p->key.protocol);

            goto abort_activate;
        }

        if(blog_p->rx.multicast)
        {
            int isActivation = 0;

#if defined(CC_PKTRUNNER_MCAST)
            rdpa_key = runnerMcast_activate(blog_p, &isActivation, &ret_err);
#else
            rdpa_key = FHW_TUPLE_INVALID;
            ret_err  = BDMF_ERR_NOT_SUPPORTED;
#endif
            if(rdpa_key == FHW_TUPLE_INVALID)
            {
                __logInfo("Could not runnerMcast_activate");

                PKTRUNNER_STATE(accel).mc_errors++;
                if( !ret_err )
                    PKTRUNNER_STATE(accel).hash_colli++;
                goto abort_activate;
            }

            if(isActivation)
            {
                PKTRUNNER_STATE(accel).activates++;
                PKTRUNNER_STATE(accel).active++;
                PKTRUNNER_STATE(accel).mcast_active++;

                /* New multicast flow -- key_in should be INVALID */
                if (key_in != FHW_TUPLE_INVALID)
                {
                    __logError("New Mcast flow but key_in<0x%08x> accel <%d>", key_in, accel);
                }
                pktRunner_flow_idx = idx_pool_get_index(&PKTRUNNER_DATA(accel).idx_pool);
                if (pktRunner_flow_idx < 0)
                {
                    __logInfo("No free pkt runner indexes for accel <%d>",accel);

                    PKTRUNNER_STATE(accel).mc_errors++;
                    PKTRUNNER_STATE(accel).no_free_idx++;
                    goto abort_activate;
                }
            }
            else
            {
                /* Multicast flow modification -- verify the key_in */
                pktRunner_flow_idx = __pktRunnerFlowIdx(pktRunner_Key.word);
                if (!idx_pool_index_in_use(&PKTRUNNER_DATA(accel).idx_pool, pktRunner_flow_idx))
                {
                    __logError("Mcast modify Invalid key_in <%u:%u>; ",accel,key_in);
                }
                /* make sure HW/RDPA returned the same key as earlier stored */
                else if (rdpa_key != PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx))
                {
                    __logError("Mcast modify <key_in=0x%08x>; stored and returned rdpa key mismatch <%u:%u>",
                               key_in, PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx), rdpa_key);
                }
                if (pktRunner_flow_idx < 0 || pktRunner_flow_idx >= PKTRUNNER_DATA(accel).max_flow_idxs)
                {
                    __logError("Mcast modify Invalid pktRunner_flow_idx <%u:%d>; ",accel,pktRunner_flow_idx);
                    PKTRUNNER_STATE(accel).mc_errors++;
                    goto abort_activate;
                }
            }

            pktRunner_Key.word = __pktRunnerBuildKey(accel, PKTRNR_FLOW_TYPE_MC, pktRunner_flow_idx);

            PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) = rdpa_key;
            /* Should we be clearing the old stats if new client joins ??*/
            __pktRunnerClearRstStats(pktRunner_Key.word);

            __debug("::: runnerMcast_activate: flow <%u:%u>, cumm_activates <%u> :::\n\n",
                    pktRunner_Key.flow_type, pktRunner_Key.flow_idx, PKTRUNNER_STATE(accel).activates);

            return pktRunner_Key.word;
        }
        else
        {
            void *cmdlist_buffer_p = NULL;

            pktRunner_flow_idx = idx_pool_get_index(&PKTRUNNER_DATA(accel).idx_pool);
            if (pktRunner_flow_idx < 0)
            {
                __logInfo("No free pkt runner indexes for accel <%d>",accel);

                PKTRUNNER_STATE(accel).l3_errors++;
                PKTRUNNER_STATE(accel).no_free_idx++;                
                goto abort_activate;
            }

            rdpa_key = runnerUcast_activate(blog_p, prepend.data, prepend.size, &cmdlist_buffer_p, &ret_err);

            if(rdpa_key == FHW_TUPLE_INVALID)
            {
                __logInfo("Could not runnerUcast_activate");

                idx_pool_return_index(&PKTRUNNER_DATA(accel).idx_pool, pktRunner_flow_idx);
                PKTRUNNER_STATE(accel).l3_errors++;
                if (!ret_err)
                    PKTRUNNER_STATE(accel).hash_colli++;                
                goto abort_activate;
            }

            PKTRUNNER_STATE(accel).activates++;
            PKTRUNNER_STATE(accel).active++;
            PKTRUNNER_STATE(accel).ucast_active++;

            pktRunner_Key.word = __pktRunnerBuildKey(accel, PKTRNR_FLOW_TYPE_L3, pktRunner_flow_idx);

            PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) = rdpa_key;
            PKTRUNNER_CMDLIST_PTR(accel, pktRunner_flow_idx) = cmdlist_buffer_p;

        	__pktRunnerClearRstStats(pktRunner_Key.word);

            __debug("::: runnerUcast_activate: flow <%u:%u>, cumm_activates <%u> :::\n\n",
                    pktRunner_Key.flow_type, pktRunner_Key.flow_idx, PKTRUNNER_STATE(accel).activates);

            return pktRunner_Key.word;
        }
    }

abort_activate:
    PKTRUNNER_STATE(accel).failures++;
    __logInfo("cumm_failures<%u>", PKTRUNNER_STATE(accel).failures);

    return FHW_TUPLE_INVALID;
}

int fhwPktRunnerActivate(Blog_t *blog_p, uint32_t key_in)
{
    return __fhwPktRunnerActivate(blog_p, key_in, PKTRUNNER_ACCEL_FLOW);
}

#if !defined(RDP_SIM)
int fhwPktRunnerAddHostMac(char *mac_p)
{
    return runnerUcast_add_host_mac(mac_p);
}

int fhwPktRunnerDelHostMac(char *mac_p)
{
    return runnerUcast_delete_host_mac(mac_p);
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function   : __runnerClearRstStats
 * Description: This function is invoked to clear the reset stats of a flow
 * Parameters :
 *  pktRunner_Key : 32bit valyue to refer to a PktRunner flow
 * Returns    : Success(zero)/Failure(non-zero).
 *------------------------------------------------------------------------------
 */
 
static int __pktRunnerClearRstStats(uint32_t pktRunner_Key) 
{
    int flowIdx = __pktRunnerFlowIdx(pktRunner_Key);
    int accel = __pktRunnerAccelIdx(pktRunner_Key);
    if (flowIdx < 0 || flowIdx >= PKTRUNNER_DATA(accel).max_flow_idxs)
        return -1;
    PKTRUNNER_STATS(accel, flowIdx) = 0; 
    return 0;
}

#if !defined(RDP_SIM)
/*
 *------------------------------------------------------------------------------
 * Function   : fhwPktRunnerRefresh
 * Description: This function is invoked to collect flow statistics
 * Parameters :
 *  tuple : 16bit index to refer to a Runner flow
 * Returns    : Total hits on this connection.
 *------------------------------------------------------------------------------
 */
static int fhwPktRunnerRefresh(uint32_t pktRunner_Key, uint32_t *pktsCnt_p, uint32_t *octetsCnt_p)
{
    int flowIdx = __pktRunnerFlowIdx(pktRunner_Key);
    e_PKTRNR_FLOW_TYPE flowType = __pktRunnerFlowType(pktRunner_Key);
    uint32_t accel = __pktRunnerAccelIdx(pktRunner_Key);
    int rc = -1;

    pktRunner_flowStat_t rawStat;
    pktRunner_flowStat_t resetStat;

    if (flowIdx < 0 || flowIdx >= PKTRUNNER_DATA(accel).max_flow_idxs)
        return rc;

    switch (flowType)
    {
        case PKTRNR_FLOW_TYPE_L3:
            rc = runnerUcast_refresh(PKTRUNNER_RDPA_KEY(accel, flowIdx), &rawStat.rdpastat.packets, &rawStat.rdpastat.bytes);
            break;
        case PKTRNR_FLOW_TYPE_L2:
            rc = runnerL2Ucast_refresh(PKTRUNNER_RDPA_KEY(accel, flowIdx), &rawStat.rdpastat.packets, &rawStat.rdpastat.bytes);
            break;
#if defined(CC_PKTRUNNER_MCAST)
        case PKTRNR_FLOW_TYPE_MC:
            rc = runnerMcast_refresh(PKTRUNNER_RDPA_KEY(accel, flowIdx), &rawStat.rdpastat.packets, &rawStat.rdpastat.bytes);
            break;
#endif
        default:
            break;
    }
    if (rc < 0)
    {
        __logError("Could not get pktRunner_Key <0x%x> flow <%u:%u> rdpa_key<%u> stats, rc %d", pktRunner_Key, flowType, flowIdx, PKTRUNNER_RDPA_KEY(accel, flowIdx), rc);
        
        __pktRunnerIncErr(accel, flowType);        
        return rc;
    }

    resetStat.num = PKTRUNNER_STATS(accel, flowIdx);

    *pktsCnt_p = rawStat.rdpastat.packets-resetStat.rdpastat.packets;
    *octetsCnt_p = rawStat.rdpastat.bytes-resetStat.rdpastat.bytes;

    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhwPktRunnerRefreshPathStat
 * Description: This function is invoked to collect path statistics
 * Parameters :
 *  pathIdx : 16bit index to refer to a Runner path
 * Returns    : Total hits on this connection.
 *------------------------------------------------------------------------------
 */
static int fhwPktRunnerRefreshPathStat(uint8_t pathIdx, uint32_t *pktsCnt_p, uint32_t *octetsCnt_p)
{
    int rc;
    rdpa_stat_t rawStat;

    rc = runnerL2Ucast_refresh_pathstat(pathIdx, &rawStat.packets, &rawStat.bytes);
    if (rc < 0)
    {
        __logError("Could not get pathIdx<%d> stats, rc %d", pathIdx, rc);
        return rc;
    }

    *pktsCnt_p = rawStat.packets;
    *octetsCnt_p = rawStat.bytes;

    return 0;
}


/*
 *------------------------------------------------------------------------------
 * Function   : fhwPktRunnerDeactivate
 * Description: This function is invoked when a Runner flow needs to be
 *              deactivated.
 * Parameters :
 *  tuple     : 16bit index to refer to a flow in HW
 *  blog_p    : pointer to a blog object (for multicast only)
 * Returns    : Remaining number of active ports (for multicast only)
 *------------------------------------------------------------------------------
 */
#if !defined(CONFIG_BCM_CMDLIST_SIM)
static int fhwPktRunnerDeactivate(uint32_t pktRunner_Key, uint32_t *pktsCnt_p,
                                  uint32_t *octetsCnt_p, struct blog_t *blog_p)
{
    int isDeactivation = 1;
    int rc = -1;
    int flowIdx = __pktRunnerFlowIdx(pktRunner_Key);
    e_PKTRNR_FLOW_TYPE flowType = __pktRunnerFlowType(pktRunner_Key);
    uint32_t accel = __pktRunnerAccelIdx(pktRunner_Key);

    __debug("\n::: runnerDeactivate :::\n\n");
    __dumpBlog(blog_p);

    /* Fetch last hit count */
    rc = fhwPktRunnerRefresh(pktRunner_Key, pktsCnt_p, octetsCnt_p);
    if (!rc)
    {
        switch (flowType)
        {
            case PKTRNR_FLOW_TYPE_L3:
                rc = runnerUcast_deactivate(PKTRUNNER_RDPA_KEY(accel, flowIdx), PKTRUNNER_CMDLIST_PTR(accel, flowIdx));
                if(!rc)
                {
                    PKTRUNNER_CMDLIST_PTR(accel, flowIdx) = NULL;
                }
                break;
            case PKTRNR_FLOW_TYPE_L2:
                rc = runnerL2Ucast_deactivate(PKTRUNNER_RDPA_KEY(accel, flowIdx), PKTRUNNER_CMDLIST_PTR(accel, flowIdx));
                break;
    #if defined(CC_PKTRUNNER_MCAST)
            case PKTRNR_FLOW_TYPE_MC:
                rc = runnerMcast_deactivate(blog_p, &isDeactivation);
                break;
    #endif
            default:
                break;
        }
    }

    if(rc < 0) /* Explicitly check for -ve return; +ve return is pending client for mcast */
    {
        __pktRunnerIncErr(accel, flowType);        
        goto abort_deactivate;
    }

    if(isDeactivation)
    {
        __pktRunnerClearRstStats(pktRunner_Key);
        PKTRUNNER_STATE(accel).deactivates++;
        PKTRUNNER_STATE(accel).active--;
        if(blog_p->rx.multicast)
        {
            PKTRUNNER_STATE(accel).mcast_active--;
        }
        else
        {
            PKTRUNNER_STATE(accel).ucast_active--;
        }
        if (idx_pool_return_index(&PKTRUNNER_DATA(accel).idx_pool, flowIdx) < 0)
        {
            __logInfo("Failed to free pkt runner index <%d:%d> for rdpa_key<%u>",
                      accel,flowIdx,PKTRUNNER_RDPA_KEY(accel, flowIdx));

            __pktRunnerIncErr(accel, flowType);        
            goto abort_deactivate;
        }
        PKTRUNNER_RDPA_KEY(accel, flowIdx) = FHW_TUPLE_INVALID;
    }

    __logDebug("::: runnerDeactivate flow<%u:%u> hits<%u> bytes<%u> cumm_deactivates<%u> :::\n",
               flowType, flowIdx, *pktsCnt_p, *octetsCnt_p, PKTRUNNER_STATE(accel).deactivates);

    return rc;

abort_deactivate:
    PKTRUNNER_STATE(accel).failures++;

    return rc;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fhwPktRunnerUpdate
 * Description: This function is invoked when a Runner flow needs to be
 *              updated.
 * Parameters :
 *  blog_p    : pointer to a blog object
 *  tuple     : 16bit index to refer to a flow in HW
 * Returns    : 0 on success.
 *------------------------------------------------------------------------------
 */
int fhwPktRunnerUpdate(BlogUpdate_t update, uint32_t pktRunner_Key, Blog_t *blog_p)
{
    int flowIdx = __pktRunnerFlowIdx(pktRunner_Key);
    e_PKTRNR_FLOW_TYPE flowType = __pktRunnerFlowType(pktRunner_Key);
    uint32_t accel = __pktRunnerAccelIdx(pktRunner_Key);
    int rc = -1;

    switch (flowType)
    {
        case PKTRNR_FLOW_TYPE_L3:
            rc = runnerUcast_update(update, PKTRUNNER_RDPA_KEY(accel, flowIdx), blog_p);
            break;
        case PKTRNR_FLOW_TYPE_L2:
            rc = runnerL2Ucast_update(update, PKTRUNNER_RDPA_KEY(accel, flowIdx), blog_p);
            break;
#if defined(CC_PKTRUNNER_MCAST)
        case PKTRNR_FLOW_TYPE_MC:
            __logError("Multicast flow<%d:%d> updates are not supported",flowType, flowIdx);
            break;
#endif
        default:
            break;
    }

    if (rc)
    {
        __pktRunnerIncErr(accel, flowType);        
    }

    return rc;
}
#else
static int fhwPktRunnerDeactivate(uint32_t pktRunner_Key, uint32_t *pktsCnt_p,
                                  uint32_t *octetsCnt_p, struct blog_t *blog_p)
{
    return 0;
}

int fhwPktRunnerUpdate(BlogUpdate_t update, uint32_t pktRunner_Key, Blog_t *blog_p)
{
    return 0;
}
#endif /* CONFIG_BCM_CMDLIST_SIM */
#endif /* !defined(RDP_SIM) */

/*
 *------------------------------------------------------------------------------
 * Function   : fhwPktRunnerResetStats
 * Description: This function is invoked to reset stats for a flow
 * Parameters :
 *  hwTuple: 29bit index to refer to a Runner flow
 * Returns    : 0 on success.
 * 
 * Assumption: bdmf_number lines up in size and offsets with rdpa_stat_t 
 *------------------------------------------------------------------------------
 */
int fhwPktRunnerResetStats(uint32_t pktRunner_Key)
{
    int           rc = -1;
    pktRunner_flowStat_t flowStat;
    int idx = __pktRunnerFlowIdx(pktRunner_Key);
    e_PKTRNR_FLOW_TYPE flowType = __pktRunnerFlowType(pktRunner_Key);
    uint32_t accel = __pktRunnerAccelIdx(pktRunner_Key);

    if (idx < 0 || idx >= PKTRUNNER_DATA(accel).max_flow_idxs) {
        return 1;
    }
    switch (flowType)
    {
        case PKTRNR_FLOW_TYPE_L3:
            rc = runnerUcast_refresh(PKTRUNNER_RDPA_KEY(accel, idx), &flowStat.rdpastat.packets, &flowStat.rdpastat.bytes);
            break;
        case PKTRNR_FLOW_TYPE_L2:
            rc = runnerL2Ucast_refresh(PKTRUNNER_RDPA_KEY(accel, idx), &flowStat.rdpastat.packets, &flowStat.rdpastat.bytes);
            break;
#if defined(CC_PKTRUNNER_MCAST)
        case PKTRNR_FLOW_TYPE_MC:
            rc = runnerMcast_refresh(PKTRUNNER_RDPA_KEY(accel, idx), &flowStat.rdpastat.packets, &flowStat.rdpastat.bytes);
            break;
#endif
        default:
            break;
    }

    if (rc == 0)
    {
        PKTRUNNER_STATS(accel, idx) = flowStat.num;
    }
    else
    {
        __pktRunnerIncErr(accel, flowType);        
    }

    return rc < 0;
}

#if !defined(RDP_SIM)
#ifdef XRDP
int fhwPktRunnerMcastWhitelistActivate(Blog_t *blog_p, uint32_t key_in)
{
    uint32_t rdpa_key;
    int accel = PKTRUNNER_ACCEL_MCAST_WHITELIST;
    int rc, pktRunner_flow_idx;
    PktRunnerFlowKey_t pktRunner_Key = {.word = key_in};

    rc = runnerMcastWhitelist_add(blog_p, &rdpa_key);
    if (rc < 0)
    {
        __logInfo("Could not runnerMcastWhitelist_add");

        goto abort_activate;
    }
    else if ((rc > 0) && (rdpa_key != FHW_TUPLE_INVALID))
    {
        /* duplicate entry case */
        rc  = __pktRunner_reverse_get_index(accel, (uint32_t *)&pktRunner_flow_idx,
                                            rdpa_key);
        if (unlikely(rc))
            goto abort_activate;
    }
    else
    {
        pktRunner_flow_idx = idx_pool_get_index(&PKTRUNNER_DATA(accel).idx_pool);
        if (pktRunner_flow_idx < 0)
        {
            __logInfo("No free pkt runner indexes for accel <%d>", accel);
            PKTRUNNER_STATE(accel).no_free_idx++;
            runnerMcastWhitelist_delete(rdpa_key);
            goto abort_activate;
        }
        PKTRUNNER_STATE(accel).active++;
        PKTRUNNER_RDPA_KEY(accel, pktRunner_flow_idx) = rdpa_key;
    }

    pktRunner_Key.word = __pktRunnerBuildKey(accel, 0, pktRunner_flow_idx);
    PKTRUNNER_STATE(accel).activates++;
    if (PKTRUNNER_DATA(accel).ref_cnt_p != NULL)
        PKTRUNNER_REF_CNT(accel, pktRunner_flow_idx)++;

    return pktRunner_Key.word;

abort_activate:

    PKTRUNNER_STATE(accel).failures++;
    __logInfo("cumm_failures<%u>", PKTRUNNER_STATE(accel).failures);

    return FHW_TUPLE_INVALID;
}

int fhwPktRunnerMcastWhitelistDeactivate(uint32_t pktRunner_Key, uint32_t *pktsCnt_p,
                                         uint32_t *octetsCnt_p, Blog_t *blog_p)
{
    uint32_t accel = __pktRunnerAccelIdx(pktRunner_Key);
    int flowIdx = __pktRunnerFlowIdx(pktRunner_Key);
    int rc;

    if ((PKTRUNNER_DATA(accel).ref_cnt_p != NULL) &&
        ((--PKTRUNNER_REF_CNT(accel, flowIdx)) > 0))
        return 0;

    rc = runnerMcastWhitelist_delete(PKTRUNNER_RDPA_KEY(accel, flowIdx));
    if (rc)
    {
        PKTRUNNER_STATE(accel).failures++;
        return rc;
    }

    PKTRUNNER_STATE(accel).deactivates++;
    PKTRUNNER_STATE(accel).active--;

    if (idx_pool_return_index(&PKTRUNNER_DATA(accel).idx_pool, flowIdx) < 0)
    {
        __logInfo("Failed to free pkt runner index <%d:%d> for rdpa_key<%u>",
                  accel,flowIdx,PKTRUNNER_RDPA_KEY(accel, flowIdx));
        PKTRUNNER_STATE(accel).failures++;
    }
    else
        PKTRUNNER_RDPA_KEY(accel, flowIdx) = FHW_TUPLE_INVALID;

    return rc;
}
#endif /* XRDP */
#endif /* !defined(RDP_SIM) */

/******************************************************************
 *
 * Flow Cache Binding
 *
 *****************************************************************/

#if defined(CONFIG_BCM_FHW)

static FC_CLEAR_HOOK fhw_clear_hook_fp = NULL;

/*
 *------------------------------------------------------------------------------
 * Function   : __clearFCache
 * Description: Clears FlowCache association(s) to Runner entries.
 *              This local function MUST be called with the Protocol Layer
 *              Lock taken.
 *------------------------------------------------------------------------------
 */
static int __clearFCache(uint32_t accel, uint32_t key, const FlowScope_t scope)
{
    int count = 0;

    /* Upcall into FlowCache */
    if(fhw_clear_hook_fp != NULL)
    {
        PKTRUNNER_STATE(accel).flushes += fhw_clear_hook_fp(key, scope);
    }


    // FIXME

//    count = fapPkt_deactivateAll();


    __debug("key<%03u> scope<%s> cumm_flushes<%u>",
            key,
            (scope == System_e) ? "System" : "Match",
            PKTRUNNER_STATE(accel).flushes);

    return count;
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function   : runnerClear
 * Description: This function is invoked when all entries pertaining to
 *              a tuple in Runner need to be cleared.
 * Parameters :
 *  tuple: FHW Engine instance and match index
 * Returns    : success
 *------------------------------------------------------------------------------
 */
int runnerClear(uint32_t tuple)
{
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function   : runnerEnable
 * Description: Binds the Runner Protocol Layer handler functions to the
 *              Flow Cache hooks.
 *------------------------------------------------------------------------------
 */
void runnerEnable(void)
{
#if defined(CONFIG_BCM_FHW)
    FhwBindHwHooks_t hwHooks_flow = {};
#if !defined(RDP_SIM) && defined(XRDP)
    FhwBindHwHooks_t hwHooks_mc_wlist = {};
#endif
    /* Initialize HW Hooks -- Start */
    hwHooks_flow.activate_fn = (HOOKP32)fhwPktRunnerActivate;
    hwHooks_flow.deactivate_fn = (HOOK4PARM)fhwPktRunnerDeactivate;
    hwHooks_flow.update_fn = (HOOK3PARM)fhwPktRunnerUpdate;
    hwHooks_flow.refresh_fn = (HOOK3PARM)fhwPktRunnerRefresh;
    hwHooks_flow.refresh_pathstat_fn = (HOOK3PARM)fhwPktRunnerRefreshPathStat;
#if !defined(RDP_SIM)
    hwHooks_flow.add_host_mac_fn = (HOOKP)fhwPktRunnerAddHostMac;
    hwHooks_flow.del_host_mac_fn = (HOOKP)fhwPktRunnerDelHostMac;
#endif
    hwHooks_flow.fhw_clear_fn = &fhw_clear_hook_fp;
    hwHooks_flow.reset_stats_fn =(HOOK32)fhwPktRunnerResetStats;
    hwHooks_flow.cap = (1<<HW_CAP_IPV4_UCAST) | (1<<HW_CAP_L2_UCAST) | (1<<HW_CAP_PATH_STATS);
    hwHooks_flow.max_ent = PKTRUNNER_MAX_L2L3_FLOWS + PKTRUNNER_MAX_MCAST_FLOWS;
    /* Number of HW_pathstat needs to match with HWACC counter group assignment */
    hwHooks_flow.max_hw_pathstat = 64;
    /* Bind to fc HW layer for learning connection configurations dynamically */
    hwHooks_flow.clear_fn = (HOOK32)runnerClear;
    /* Initialize HW Hooks -- End */

#if !defined(RDP_SIM) && defined(XRDP)
    /* Initialize HW Hooks -- Start */
    hwHooks_mc_wlist.activate_fn = (HOOKP32)fhwPktRunnerMcastWhitelistActivate;
    hwHooks_mc_wlist.deactivate_fn = (HOOK4PARM)fhwPktRunnerMcastWhitelistDeactivate;
    hwHooks_mc_wlist.fhw_clear_fn = &fhw_clear_hook_fp;
    hwHooks_mc_wlist.cap = 1 << HW_CAP_MCAST_WHITELIST;
    hwHooks_mc_wlist.max_ent = PKTRUNNER_MAX_MCAST_FLOWS;
    hwHooks_mc_wlist.max_hw_pathstat = 0;
    /* Initialize HW Hooks -- End */
#endif

#if defined(CC_PKTRUNNER_MCAST)
    hwHooks_flow.cap |= (1<<HW_CAP_IPV4_MCAST);
#endif

#if defined(CC_PKTRUNNER_IPV6)
    hwHooks_flow.cap |= ((1<<HW_CAP_IPV6_UCAST) | (1<<HW_CAP_IPV6_TUNNEL));
#if defined(CC_PKTRUNNER_MCAST)
    hwHooks_flow.cap |= (1<<HW_CAP_IPV6_MCAST);
#endif
#endif

    /* Block flow-cache from packet processing and try to push the flows */
    blog_lock();

    fhw_bind_hw(FHW_PRIO_0, &hwHooks_flow);

    BCM_ASSERT(fhw_clear_hook_fp != NULL);

    PKTRUNNER_STATE(PKTRUNNER_ACCEL_FLOW).status = 1;

#ifdef XRDP
    fhw_bind_hw(FHW_PRIO_1, &hwHooks_mc_wlist);
    PKTRUNNER_STATE(PKTRUNNER_ACCEL_MCAST_WHITELIST).status = 1;
#endif

    blog_unlock();

    bcm_print("Enabled Runner binding to Flow Cache\n");
#else
    bcm_print("Flow Cache is not built\n");
#endif
}


/*
 *------------------------------------------------------------------------------
 * Function   : runnerDisable
 * Description: Clears all active Flow Cache associations with Runner.
 *              Unbind all flow cache to Runner hooks.
 *------------------------------------------------------------------------------
 */
void runnerDisable(void)
{
#if defined(CONFIG_BCM_FHW)
    FhwBindHwHooks_t hwHooks = {};
    FhwHwAccPrio_t prioIx = FHW_PRIO_0;

    /* Block flow-cache from packet processing and try to push the flows */
    blog_lock(); 

    /* Clear system wide active FlowCache associations, and disable learning. */

    __clearFCache(0, 0, System_e);

    hwHooks.fhw_clear_fn = &fhw_clear_hook_fp;
    hwHooks.reset_stats_fn =(HOOK32) fhwPktRunnerResetStats; 

    fhw_bind_hw(prioIx, &hwHooks);

    fhw_clear_hook_fp = (FC_CLEAR_HOOK)NULL;

    PKTRUNNER_STATE(PKTRUNNER_ACCEL_FLOW).status = 0;

    blog_unlock();

    bcm_print("Disabled Runner binding to Flow Cache\n");
#else
    bcm_print("Flow Cache is not built\n");
#endif
}

#if !defined(RDP_SIM)
extern unsigned int UtilGetChipIsLP(void);
#endif
int __init _pktRunnerAccelInit(uint32_t accel, uint32_t num_flows, uint32_t cmd_list_ovrflw, uint32_t ref_count)
{
    int ret, idx;   
    pktRunner_data_t *pktRunner_p;
#if !defined(RDP_SIM)
    pktRunner_state_t *pktRunner_state_p;
#endif
    char owner_name[16];

    snprintf(owner_name, sizeof(owner_name), "PktRnr[%d]",accel);

    pktRunner_p = &pktRunner_data_g[accel];

    pktRunner_p->max_flow_idxs = num_flows;
    /* TODO : Need to use _wrap because rdpa_standalone/UT aren't including CE include file */
    ret = idx_pool_init_wrap(&pktRunner_p->idx_pool, num_flows, owner_name);
    if (ret)
    {
        return ret;
    }

    pktRunner_p->rdpa_flow_key_p = kmalloc(sizeof(*pktRunner_p->rdpa_flow_key_p) * num_flows, GFP_KERNEL);

    if (!pktRunner_p->rdpa_flow_key_p)
    {
        /* TODO : Need to use _wrap because rdpa_standalone/UT aren't including CE include file */
        idx_pool_exit_wrap(&pktRunner_p->idx_pool);
        return -1;
    }
    for (idx=0; idx < num_flows; idx++)
    {
        PKTRUNNER_RDPA_KEY(accel, idx) = FHW_TUPLE_INVALID;
    }


    pktRunner_p->flowResetStats_p = kmalloc(sizeof(*pktRunner_p->flowResetStats_p)* num_flows, GFP_KERNEL);

    if (!pktRunner_p->flowResetStats_p)
    {
        /* TODO : Need to use _wrap because rdpa_standalone/UT aren't including CE include file */
        idx_pool_exit_wrap(&pktRunner_p->idx_pool);
        kfree(pktRunner_p->rdpa_flow_key_p);
        return -1;
    }
    memset(pktRunner_p->flowResetStats_p, 0x0, sizeof(*pktRunner_p->flowResetStats_p) * num_flows);

    if (ref_count)
    {
        pktRunner_p->ref_cnt_p = kmalloc(sizeof(*pktRunner_p->ref_cnt_p) * num_flows, GFP_KERNEL);
        if (!pktRunner_p->ref_cnt_p)
        {
            idx_pool_exit_wrap(&pktRunner_p->idx_pool);
            kfree(pktRunner_p->rdpa_flow_key_p);
            kfree(pktRunner_p->flowResetStats_p);
            return -1;
        }
        memset(pktRunner_p->ref_cnt_p, 0x0, sizeof(*pktRunner_p->ref_cnt_p) * num_flows);
    }

    if (cmd_list_ovrflw)
    {
        pktRunner_p->cmdlist_buffer_pointers = kmalloc(sizeof(*pktRunner_p->cmdlist_buffer_pointers) * num_flows, GFP_KERNEL);

        if (!pktRunner_p->cmdlist_buffer_pointers)
        {
            /* TODO : Need to use _wrap because rdpa_standalone/UT aren't including CE include file */
            idx_pool_exit_wrap(&pktRunner_p->idx_pool);
            kfree(pktRunner_p->rdpa_flow_key_p);
            kfree(pktRunner_p->flowResetStats_p);
            if (pktRunner_p->ref_cnt_p)
                kfree(pktRunner_p->ref_cnt_p);
            return -1;
        }
        memset(pktRunner_p->cmdlist_buffer_pointers, 0x0, sizeof(*pktRunner_p->cmdlist_buffer_pointers) * num_flows);
    }

#if !defined(RDP_SIM)
    if (UtilGetChipIsLP())
        pktRunner_state_p->max_flows = 704; /* 63132 Runner HW supports only 704 max flows */
#endif
    return ret;
}

int __exit _pktRunnerAccelExit(uint32_t accel)
{
    int ret;   
    pktRunner_data_t *pktRunner_p;

    pktRunner_p = &pktRunner_data_g[accel];


    ret = idx_pool_exit_wrap(&pktRunner_p->idx_pool);
    if (ret)
    {
        return ret;
    }

    kfree(pktRunner_p->rdpa_flow_key_p);
    kfree(pktRunner_p->flowResetStats_p);
    if (pktRunner_p->ref_cnt_p)
        kfree(pktRunner_p->ref_cnt_p);
    if (pktRunner_p->cmdlist_buffer_pointers)
    {
        kfree(pktRunner_p->cmdlist_buffer_pointers);
    }

    return ret;
}

/*
*******************************************************************************
* Function   : runnerProto_construct
* Description: Constructs the Runner Protocol layer
*******************************************************************************
*/
int __init runnerProto_construct(void)
{
    int ret;
    
    memset(&pktRunner_data_g, 0, sizeof(pktRunner_data_g));

    /* Initialize Accelerator#0 */
    ret = _pktRunnerAccelInit(PKTRUNNER_ACCEL_FLOW, PKTRUNNER_MAX_L2L3_FLOWS+PKTRUNNER_MAX_MCAST_FLOWS, 1/*cmd_list_ovrflw*/, 0 /*ref_count */);
    if (ret)
    {
        return ret;
    }

    /* Only create for L2+L3; Mcast needs separate pool */
    memset(&rdpa_shared_flw_idx_pool_g, 0, sizeof(rdpa_shared_flw_idx_pool_g));
    ret = rdpa_flow_idx_pool_init(&rdpa_shared_flw_idx_pool_g, PKTRUNNER_MAX_L2L3_FLOWS, "L2L3-ucast");
    if (ret)
    {
        return ret;
    }

    /* Only create for L2+L3; Mcast needs separate pool */
    rdpa_shared_flow_disp_pool_p = kmalloc(sizeof(*rdpa_shared_flow_disp_pool_p)*PKTRUNNER_MAX_L2L3_FLOWS, GFP_KERNEL);
    if (!rdpa_shared_flow_disp_pool_p)
    {
        rdpa_flow_idx_pool_exit(&rdpa_shared_flw_idx_pool_g);
        return -1;
    }
    memset(rdpa_shared_flow_disp_pool_p, 0, sizeof(*rdpa_shared_flow_disp_pool_p)*PKTRUNNER_MAX_L2L3_FLOWS);

    ret = runnerUcast_construct(&rdpa_shared_flw_idx_pool_g, rdpa_shared_flow_disp_pool_p);
    if (ret)
    {
        rdpa_flow_idx_pool_exit(&rdpa_shared_flw_idx_pool_g);
        kfree(rdpa_shared_flow_disp_pool_p);
        return ret;
    }

    ret = runnerL2Ucast_construct(&rdpa_shared_flw_idx_pool_g, rdpa_shared_flow_disp_pool_p);
    if (ret)
    {
        rdpa_flow_idx_pool_exit(&rdpa_shared_flw_idx_pool_g);
        kfree(rdpa_shared_flow_disp_pool_p);
        runnerUcast_destruct();
        return ret;
    }

#if defined(CC_PKTRUNNER_MCAST)
    /* Mcast will create its own pool */
    ret = runnerMcast_construct(NULL, NULL);
    if (ret)
    {
        rdpa_flow_idx_pool_exit(&rdpa_shared_flw_idx_pool_g);
        runnerL2Ucast_destruct();
        runnerUcast_destruct();

        return ret;
    }

#if !defined(CONFIG_BCM_CMDLIST_SIM)
    ret = pktrunner_wlan_mcast_construct();
    if (ret)
    {
        rdpa_flow_idx_pool_exit(&rdpa_shared_flw_idx_pool_g);
        runnerMcast_destruct();
        runnerL2Ucast_destruct();
        runnerUcast_destruct();

        return ret;
    }
#endif
#endif

#if !defined(RDP_SIM) && defined(XRDP)
    ret = runnerMcastWhitelist_construct(NULL);
    if (ret)
    {
        rdpa_flow_idx_pool_exit(&rdpa_shared_flw_idx_pool_g);
        pktrunner_wlan_mcast_destruct();
        runnerMcast_destruct();
        runnerL2Ucast_destruct();
        runnerUcast_destruct();

        return ret;
    }
#endif

#if !defined(CONFIG_BCM_CMDLIST_SIM)
    {
        cmdlist_hooks_t cmdlist_hooks;

        cmdlist_hooks.ipv6_addresses_table_add = runnerUcast_ipv6_addresses_table_add;
        cmdlist_hooks.ipv4_addresses_table_add = runnerUcast_ipv4_addresses_table_add;
        cmdlist_hooks.brcm_tag_info = NULL;

        cmdlist_bind(&cmdlist_hooks);
    }
#endif

    /* Initialize subsequent Accelerators.. if any, before runnerEnable */

#if !defined(RDP_SIM) && defined(XRDP)
    /* Initialize Accelerator#1 for Mcast_whitelist */
    ret = _pktRunnerAccelInit(PKTRUNNER_ACCEL_MCAST_WHITELIST, PKTRUNNER_MAX_MCAST_FLOWS, 0, 1 /* ref_count */);
    if (ret)
    {
        return ret;
    }
#endif

    runnerEnable();

#if !defined(CONFIG_BCM_CMDLIST_SIM) && !defined(RDP_SIM)
    {
        unsigned int rdp_freq;

        get_rdp_freq(&rdp_freq);

        bcm_print("Initialized Runner Protocol Layer (%u)\n", rdp_freq);
    }
#else
    /* Override default log level to DEBUG */
    bcmLog_setLogLevel(BCM_LOG_ID_PKTRUNNER, BCM_LOG_LEVEL_DEBUG);

    bcm_print("Initialized Runner Protocol Layer in SIMULATION MODE\n");
#endif /* CONFIG_BCM_CMDLIST_SIM */

    return 0;
}

/*
*******************************************************************************
* Function   : runnerProto_destruct
* Description: Destructs the Runner Protocol layer
*******************************************************************************
*/
void __exit runnerProto_destruct(void)
{
#if !defined(CONFIG_BCM_CMDLIST_SIM)
    runnerDisable();

    cmdlist_unbind();

#if defined(CC_PKTRUNNER_MCAST)
    runnerMcast_destruct();

    pktrunner_wlan_mcast_destruct();
#endif

    runnerL2Ucast_destruct();

    runnerUcast_destruct();

    _pktRunnerAccelExit(PKTRUNNER_ACCEL_FLOW);

#if !defined(RDP_SIM) && defined(XRDP)
    runnerMcastWhitelist_destruct();
    _pktRunnerAccelExit(PKTRUNNER_ACCEL_MCAST_WHITELIST);
#endif

#endif
}


/*******************************************************************************
 *
 * Auxiliary Functions
 *
 *******************************************************************************/

rdpa_mcast_flow_t *__mcastFlowMalloc(void)
{
    return kmalloc(sizeof(rdpa_mcast_flow_t), GFP_ATOMIC);
}

void __mcastFlowFree(rdpa_mcast_flow_t *mcastFlow_p)
{
    kfree(mcastFlow_p);
}

uint32_t __enetLogicalPortToPhysicalPort(uint32_t logicalPort)
{
    return LOGICAL_PORT_TO_PHYSICAL_PORT(logicalPort);
}

uint32_t __skbMarkToQueuePriority(uint32_t skbMark)
{
    return SKBMARK_GET_Q_PRIO(skbMark);
}

uint32_t __skbMarkToTrafficClass(uint32_t skbMark)
{
    return SKBMARK_GET_TC_ID(skbMark);
}

int __isEnetWanPort(uint32_t logicalPort)
{
   int isWanPort = FALSE ;

#if defined(RDP_SIM)
   isWanPort = rdpa_blog_is_wan_port(logicalPort);
#else   
   bcmFun_t *enetIsWanPortFun = bcmFun_get(BCM_FUN_ID_ENET_IS_WAN_PORT);

   BCM_ASSERT(enetIsWanPortFun != NULL);

   isWanPort = enetIsWanPortFun(&logicalPort);
#endif

   return (isWanPort);
}

int __lagPortGet(Blog_t *blog_p)
{
#if defined(RDP_SIM)
    return blog_p->lag_port;
#else   
    bcmFun_t *lagPortGetFun = bcmFun_get(BCM_FUN_ID_ENET_LAG_PORT_GET);

    if(lagPortGetFun != NULL)
    {
        return lagPortGetFun(blog_p->tx_dev_p);
    }

    return 0;
#endif
}

/* Returns TRUE if LAN/SF2-Port is bonded with Runner WAN port */
int __isEnetBondedLanWanPort(uint32_t logicalPort)
{
   int ret_val = FALSE ;

#if !defined(CONFIG_BCM963158) && !defined(RDP_SIM)
  bcmFun_t *enetFunc = bcmFun_get(BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT);

  BCM_ASSERT(enetFunc != NULL);

  ret_val = enetFunc(&logicalPort);
#endif

   return (ret_val);
}

int __isWlanPhy(Blog_t *blog_p)
{
    return (blog_p->rx.info.phyHdrType == BLOG_WLANPHY ||
            blog_p->tx.info.phyHdrType == BLOG_WLANPHY);
}

int __isTxWlanPhy(Blog_t *blog_p)
{
    return (blog_p->tx.info.phyHdrType == BLOG_WLANPHY);
}

int __isTxEthPhy(Blog_t *blog_p)
{
    return (blog_p->tx.info.phyHdrType == BLOG_ENETPHY);
}

static inline int __isRxIntfWan(Blog_t *blog_p)
{
    if ((blog_p->rx.info.phyHdrType == BLOG_ENETPHY &&
         (__isEnetWanPort(blog_p->rx.info.channel) || /* from ENET-WAN */
          __isEnetBondedLanWanPort(blog_p->rx.info.channel))) || /* from Enet-Bonded-LAN-as-WAN */
        (blog_p->rx.info.bmap.BCM_XPHY) || /* from XTM-WAN */
        (blog_p->rx.info.phyHdrType == BLOG_EPONPHY) || /* from EPON */
        (blog_p->rx.info.phyHdrType == BLOG_GPONPHY) /* from GPON */
       )
        return 1;
    else
        return 0;
}

typedef struct {
    struct {
        rdpa_traffic_dir dir;       /**< Traffic direction */
        rdpa_if ingress_if;         /**< Ingress interface */
        uint16_t wan_flow;          /**< WAN Flow, used f ingress port is wan (e.g. gem_flow), ignored otherwise \XRDP_LIMITED */
    }key;
    struct {
        rdpa_if egress_if;          /**< RDPA Egress Interface */
        uint8_t is_hit_trap;        /**< 1: Trap to cpu; 0: forwarding */
        uint8_t cpu_reason;                                    /**< CPU_rx trap reason */    
        int wan_flow;               /**< DSL ATM/PTM US channel */
        int wan_flow_mode;          /**< DSL ATM/PTM US channel */
        union {
            uint32_t wl_metadata;   /**< WL metadata */
            rdpa_wfd_t wfd;
            rdpa_rnr_t rnr;
        };
        uint32_t queue_id;          /**< Egress queue id */
        uint8_t is_tcpspdtest;      /**< 1: TCPSPD_test; 0: No TCPSPD_test */
#if defined(XRDP)
        uint8_t tcpspdtest_stream_id;  /* tcpspdtest stream_id */    
        uint8_t tcpspdtest_is_upload;  /* tcpspdtest action download/upload */
#endif
    }result;
}flowParams_t;

int __L2L3ParseBlogFlowParams(Blog_t *blog_p, flowParams_t *params_p)
{
    if (blog_p->tx.info.bmap.BCM_XPHY) /* XTM-WAN Upstream */
    {
        __debug("dest.phy XTM\n");

        if(__isRxIntfWan(blog_p))
        {
            params_p->key.dir = rdpa_dir_ds;
        }
        else
        {
            params_p->key.dir = rdpa_dir_us;
        }

        params_p->result.egress_if = rdpa_wan_type_to_if(rdpa_wan_dsl);

        params_p->result.wan_flow = blog_p->tx.info.channel ; /* WAN FLOW table index */
        params_p->result.wan_flow_mode = blog_p->ptm_us_bond ; /* WAN FLOW bonded/single */
    }
    else if ((blog_p->tx.info.phyHdrType == BLOG_ENETPHY) &&
             __isEnetWanPort(blog_p->tx.info.channel)) /* ENET-WAN Upstream */
    {
        __debug("dest.phy ETH WAN\n");

        if(__isRxIntfWan(blog_p))
        {
            params_p->key.dir = rdpa_dir_ds;
        }
        else
        {
            params_p->key.dir = rdpa_dir_us;
        }

        params_p->result.egress_if = rdpa_wan_type_to_if(rdpa_wan_gbe);

        params_p->result.wan_flow = GBE_WAN_FLOW_ID ; /* WAN FLOW table index */
        params_p->result.wan_flow_mode = 0 ; /* WAN FLOW bonded/single */
    }
    else if ((blog_p->tx.info.phyHdrType == BLOG_GPONPHY) ||
             (blog_p->tx.info.phyHdrType == BLOG_EPONPHY)) /* E/GPON Upstream */
    {
        __debug("dest.phy E/GPON\n");

        if(__isRxIntfWan(blog_p))
        {
            params_p->key.dir = rdpa_dir_ds;
        }
        else
        {
            params_p->key.dir = rdpa_dir_us;
        }

        /* egress_if is the same for rdpa_wan_gpon/rdpa_wan_epon */
        params_p->result.egress_if = rdpa_wan_type_to_if(rdpa_wan_gpon); 
        params_p->result.wan_flow = blog_p->tx.info.channel ; /* GEM index */
        params_p->result.wan_flow_mode = 0; 
    }
    else if(blog_p->tx.info.phyHdrType == BLOG_SPDTST)    /* SPDTST flow */
    {
#if defined(XRDP)        
        uint8_t is_hw_mode = (blog_p->spdtst >> 1) & 1;
        uint8_t is_upload = (blog_p->spdtst >> 2) & 1;
        uint8_t stream_idx = (blog_p->spdtst >> 3) & 3;

        params_p->result.is_tcpspdtest = is_hw_mode;
        params_p->result.tcpspdtest_is_upload = is_upload;
        params_p->result.tcpspdtest_stream_id = stream_idx;
#endif
        __debug("dest.phy SPDTST\n");

        /* WAN-to-SPDT is DS, WLAN or ETH_LAN-to-SPDT is US */ 
        if(__isRxIntfWan(blog_p))
        {
            params_p->key.dir = rdpa_dir_ds;
        }
        else
        {
            params_p->key.dir = rdpa_dir_us;
        }

        params_p->result.egress_if = rdpa_if_cpu;
        params_p->result.cpu_reason = rdpa_cpu_rx_reason_tcpspdtst;
    }
    else if ((blog_p->tx.info.phyHdrType == BLOG_ENETPHY) &&
             __isEnetBondedLanWanPort(blog_p->tx.info.channel) )  /* LAN/WLAN to Enet-Bonded-LAN-as-WAN */
    {
        __debug("dest.phy ETH Bonded-LAN-WAN\n");

        params_p->key.dir = rdpa_dir_us;

        params_p->result.egress_if = rdpa_physical_port_to_rdpa_if(LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel));
    }
    else if (blog_p->rx.info.bmap.BCM_XPHY) /* XTM-WAN to LAN/WLAN */
    {
        params_p->key.dir = rdpa_dir_ds; 
        if(blog_p->tx.info.phyHdrType == BLOG_ENETPHY) 
        { 
            __debug("dest.phy ETHn"); 
            params_p->result.egress_if = rdpa_physical_port_to_rdpa_if(LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel)); 
        } 
        else if(BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(blog_p->tx.info.phyHdrType)) 
        { 
            params_p->result.egress_if = rdpa_if_wlan0;  /* WLAN flow */ 
        } 
        else 
        { 
            __logError("%sWAN-to-LAN flows are not supported", 
             (blog_p->rx.info.phyHdrType == BLOG_XTMPHY)?
                 "DSL":(blog_p->rx.info.phyHdrType == BLOG_ENETPHY)?
                 "ENET":(blog_p->rx.info.phyHdrType == BLOG_GPONPHY)?
                 "GPON":(blog_p->rx.info.phyHdrType == BLOG_EPONPHY)?
                 "EPON":"UNKNOWN");
            return -1;
        } 
    }
    else if (blog_p->rx.info.phyHdrType == BLOG_ENETPHY &&
            (__isEnetWanPort(blog_p->rx.info.channel) || /* ENET-WAN to LAN/WLAN */
             __isEnetBondedLanWanPort(blog_p->rx.info.channel)) ) /* Enet-Bonded-LAN-as-WAN to LAN/WLAN : Unlikely to hit this case */
    {
        /* Traffic from Runner-WAN or SF2-LAN-as-WAN are both handled by DS cluster */
        params_p->key.dir = rdpa_dir_ds; 
        if(blog_p->tx.info.phyHdrType == BLOG_ENETPHY) 
        { 
            __debug("dest.phy ETHn"); 
            params_p->result.egress_if = rdpa_physical_port_to_rdpa_if(LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel)); 
        } 
        else if(BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(blog_p->tx.info.phyHdrType)) 
        { 
            params_p->result.egress_if = rdpa_if_wlan0;  /* WLAN flow */ 
        } 
        else 
        { 
            __logError("%sWAN-to-LAN flows are not supported", 
             (blog_p->rx.info.phyHdrType == BLOG_XTMPHY)?
                 "DSL":(blog_p->rx.info.phyHdrType == BLOG_ENETPHY)?
                 "ENET":(blog_p->rx.info.phyHdrType == BLOG_GPONPHY)?
                 "GPON":(blog_p->rx.info.phyHdrType == BLOG_EPONPHY)?
                 "EPON":"UNKNOWN");
               
            return -1;
        } 
    }
    else if ((blog_p->rx.info.phyHdrType == BLOG_GPONPHY) ||
             (blog_p->rx.info.phyHdrType == BLOG_EPONPHY)) /* GPON/EPON downstream */
    {
        params_p->key.dir = rdpa_dir_ds; 
        if(blog_p->tx.info.phyHdrType == BLOG_ENETPHY) 
        { 
            __debug("dest.phy ETHn"); 
            params_p->result.egress_if = rdpa_physical_port_to_rdpa_if(LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel)); 
        } 
        else if(BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(blog_p->tx.info.phyHdrType)) 
        { 
            params_p->result.egress_if = rdpa_if_wlan0;  /* WLAN flow */ 
        } 
        else 
        { 
            __logError("%sWAN-to-LAN flows are not supported", 
             (blog_p->rx.info.phyHdrType == BLOG_XTMPHY)?
                 "DSL":(blog_p->rx.info.phyHdrType == BLOG_ENETPHY)?
                 "ENET":(blog_p->rx.info.phyHdrType == BLOG_GPONPHY)?
                 "GPON":(blog_p->rx.info.phyHdrType == BLOG_EPONPHY)?
                 "EPON":"UNKNOWN");
            return -1;
        } 
    }
    else if(blog_p->rx.info.phyHdrType == BLOG_SPDTST)
    {
        /* SPDT runner FW set is_LAN bit in common_repo task */
        params_p->key.dir = rdpa_dir_us;
            
        if(blog_p->tx.info.phyHdrType == BLOG_ENETPHY)  /* SPDTST to LAN */
        {
            __debug("dest.phy ETH-LAN\n");
            params_p->result.egress_if = rdpa_physical_port_to_rdpa_if(LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel));
        }
        else if(BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(blog_p->tx.info.phyHdrType))  /* SPDTST to WLAN */
        {
            __debug("dest.phy WLAN\n");
            params_p->result.egress_if = rdpa_if_wlan0;  
        }
    }    
    else
    {
        /* LAN-to-LAN (not supported) */ 
        /* LAN-to-WLAN or WLAN-to-LAN or WLAN-to-WLAN */

        if( BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(blog_p->tx.info.phyHdrType) && 
            ( blog_p->rx.info.phyHdrType == BLOG_ENETPHY   ||  /* LAN to WLAN */
              blog_p->rx.info.phyHdrType == BLOG_WLANPHY )     /* WLAN to WLAN */
          )
        {
            params_p->key.dir = rdpa_dir_us;
            params_p->result.egress_if = rdpa_if_wlan0;  /* WLAN flow */
        }
        else if(blog_p->rx.info.phyHdrType == BLOG_WLANPHY &&
                blog_p->tx.info.phyHdrType == BLOG_ENETPHY) /* WLAN to LAN */
        {
            __debug("dest.phy ETH\n");

            params_p->key.dir = rdpa_dir_us; /* put in us connection table, fw will send ds */

            params_p->result.egress_if = rdpa_physical_port_to_rdpa_if(LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel));
        }
        else
        {
            __logError("LAN-to-LAN flow is not supported: Rx %u, Tx %u", blog_p->rx.info.phyHdrType, blog_p->tx.info.phyHdrType);

            return -1;
        }
    }

    if(blog_p->rx.info.bmap.BCM_XPHY) /* XTM-WAN to LAN/WLAN */
    {
        __debug("source.phy XTM\n");

        params_p->key.ingress_if = rdpa_wan_type_to_if(rdpa_wan_dsl);
    }
    else if(blog_p->rx.info.phyHdrType == BLOG_ENETPHY &&
            __isEnetWanPort(blog_p->rx.info.channel)) /* ENET-WAN to LAN/WLAN */
    {
        __debug("source.phy ETH WAN\n");

        params_p->key.ingress_if = rdpa_wan_type_to_if(rdpa_wan_gbe);
    }
    else if(blog_p->rx.info.phyHdrType == BLOG_GPONPHY) 
    {
        __debug("source.phy GPON WAN\n");

        params_p->key.ingress_if = rdpa_wan_type_to_if(rdpa_wan_gpon);
        /* GEM index */
        params_p->key.wan_flow = blog_p->rx.info.channel;
    } 
    else if(blog_p->rx.info.phyHdrType == BLOG_EPONPHY) 
    {
        __debug("source.phy EPON WAN\n");

        params_p->key.ingress_if = rdpa_wan_type_to_if(rdpa_wan_epon);
        /* FIXME!! do we have flow id? */
    } 
    else
    {
        /* LAN */ 
        if (blog_p->rx.info.phyHdrType == BLOG_ENETPHY)
        {
            __debug("source.phy ETH\n");

            params_p->key.ingress_if = rdpa_physical_port_to_rdpa_if(LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->rx.info.channel));
        }
        else if (blog_p->rx.info.phyHdrType == BLOG_WLANPHY)
        {
            __debug("source.phy WLAN\n");

            params_p->key.ingress_if = rdpa_if_wlan0;  /* WLAN flow */
        }
        else if(blog_p->rx.info.phyHdrType == BLOG_SPDTST)  /* Speed Test flow */
        {
#if defined(XRDP)            
            uint8_t is_upload = (blog_p->spdtst >> 2) & 1;
            uint8_t stream_idx = (blog_p->spdtst >> 3) & 3;

            params_p->result.tcpspdtest_is_upload = is_upload;
            params_p->result.tcpspdtest_stream_id = stream_idx;
#endif
            __debug("source.phy SPD_TEST\n");           
            params_p->key.ingress_if = rdpa_if_cpu;
            params_p->key.dir = rdpa_dir_us;
            params_p->result.cpu_reason = rdpa_cpu_rx_reason_tcpspdtst;
        }
        else
        {
            __logError("LAN flow is not supported: Rx %u, Tx %u", blog_p->rx.info.phyHdrType, blog_p->tx.info.phyHdrType);

            return -1;
        }
    }

    if(BLOG_IS_TX_HWACC_ENABLED_WLAN_PHY(blog_p->tx.info.phyHdrType))
    {
        params_p->result.queue_id  = 0;     /* queue_id not used for WLAN */
        params_p->result.rnr.is_wfd = blog_p->rnr.is_wfd; /* union with wfd nic and wfd dhd */
        if (blog_p->rnr.is_wfd)
        {
            params_p->result.wfd.nic_ucast.is_chain = blog_p->wfd.nic_ucast.is_chain;
            if (blog_p->wfd.nic_ucast.is_chain)
            {
                params_p->result.wfd.nic_ucast.wfd_prio = blog_p->wfd.nic_ucast.wfd_prio;
                params_p->result.wfd.nic_ucast.wfd_idx = blog_p->wfd.nic_ucast.wfd_idx;
                params_p->result.wfd.nic_ucast.priority = blog_p->wfd.nic_ucast.priority;
                params_p->result.wfd.nic_ucast.chain_idx = blog_p->wfd.nic_ucast.chain_idx;
            }
            else
            {
                params_p->result.wfd.dhd_ucast.wfd_prio = blog_p->wfd.dhd_ucast.wfd_prio;
                params_p->result.wfd.dhd_ucast.ssid = blog_p->wfd.dhd_ucast.ssid;
                params_p->result.wfd.dhd_ucast.wfd_idx = blog_p->wfd.dhd_ucast.wfd_idx;
                params_p->result.wfd.dhd_ucast.priority = blog_p->wfd.dhd_ucast.priority;
                params_p->result.wfd.dhd_ucast.flowring_idx = blog_p->wfd.dhd_ucast.flowring_idx;
            }
        }
        else /* Flow through DHD-RNR */
        {
            params_p->result.rnr.radio_idx = blog_p->rnr.radio_idx;
            params_p->result.rnr.priority = blog_p->rnr.priority;
            params_p->result.rnr.ssid = blog_p->rnr.ssid;
            params_p->result.rnr.flowring_idx = blog_p->rnr.flowring_idx;
            params_p->result.rnr.llcsnap_flag = blog_p->rnr.llcsnap_flag;
            params_p->result.rnr.flow_prio = blog_p->rnr.flow_prio;
        }
    }
    else if (blog_p->tx.info.bmap.BCM_XPHY) 
    {
       params_p->result.queue_id = params_p->result.wan_flow / RDPA_MAX_XTMCHANNEL;
    }
    else
    {
        params_p->result.queue_id = SKBMARK_GET_Q_PRIO(blog_p->mark);
    }
    __debug("source.channel %u\n", blog_p->rx.info.channel);

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
    if (blog_p->fc_hybrid)
    {
        fc_class_ctx_t fc_key;

        fc_key.word = blog_p->fc_context;
        fc_key.id.src_port = params_p->key.ingress_if; 
        blog_p->fc_context = fc_key.word;

        params_p->result.is_hit_trap = 1;

        if (blog_p->iq_prio)
            params_p->result.cpu_reason = rdpa_cpu_rx_reason_hit_trap_high;
        else
            params_p->result.cpu_reason = rdpa_cpu_rx_reason_hit_trap_low;
        
        __debug("egress_if=%d, src_port=%d, blog_fc_ctx=0x%x, cpu_reason=%d\n", 
                params_p->result.egress_if, fc_key.id.src_port, blog_p->fc_context, params_p->result.cpu_reason);
    }
#endif
    return 0;
}

int __ucastSetFwdAndFilters(Blog_t *blog_p, rdpa_ip_flow_info_t *ip_flow_p)
{
// FIXME!! need to fix rdpa_ip_flow_info_t from 6858 to support what we have for other platforms */
    flowParams_t flow_params = {};
    int err = 0;

    err = __L2L3ParseBlogFlowParams(blog_p, &flow_params);
    if (err)
    {
        return err;
    }

    ip_flow_p->key.ingress_if = flow_params.key.ingress_if;
    ip_flow_p->key.dir = flow_params.key.dir;
    // wan_flow is not part of L3 flow key
    //ip_flow_p->key.wan_flow = params_p->key.wan_flow;
    ip_flow_p->result.cpu_reason = flow_params.result.cpu_reason;
    ip_flow_p->result.egress_if = flow_params.result.egress_if;
    ip_flow_p->result.is_hit_trap = flow_params.result.is_hit_trap;
    ip_flow_p->result.queue_id = flow_params.result.queue_id;
    ip_flow_p->result.wl_metadata = flow_params.result.wl_metadata;
    ip_flow_p->result.wan_flow = flow_params.result.wan_flow;
    ip_flow_p->result.wan_flow_mode = flow_params.result.wan_flow_mode;
#if defined(XRDP)    
    ip_flow_p->result.is_tcpspdtest = flow_params.result.is_tcpspdtest;    
    ip_flow_p->result.tcpspdtest_stream_id = flow_params.result.tcpspdtest_stream_id;
    ip_flow_p->result.tcpspdtest_is_upload = flow_params.result.tcpspdtest_is_upload;
#endif
    
    __debug("src.rdpa_if %u\n", ip_flow_p->key.ingress_if);
    __debug("dest.channel %u\n", ip_flow_p->result.egress_if);
    __debug("dest.queue %u\n", ip_flow_p->result.queue_id);
    __debug("direction %s\n", ip_flow_p->key.dir == rdpa_dir_us ? "US" : "DS");

    /* L4 protocol is set in the Blog Key for both IPv4 and IPv6 */
    ip_flow_p->key.tcp_pure_ack = blog_p->key.tcp_pure_ack;
    ip_flow_p->key.prot = blog_p->key.protocol;
    ip_flow_p->result.lag_port = blog_p->lag_port;

    __debug("protocol %u\n", ip_flow_p->key.prot);

#if defined(CC_PKTRUNNER_IPV6)
    if(blog_p->rx.info.bmap.PLD_IPv6 && !(T4in6DN(blog_p)))
    {
        ip_flow_p->key.src_ip.family = bdmf_ip_family_ipv6;
        memcpy(ip_flow_p->key.src_ip.addr.ipv6.data, blog_p->tupleV6.saddr.p8, 16);
        ip_flow_p->key.dst_ip.family = bdmf_ip_family_ipv6;
        memcpy(ip_flow_p->key.dst_ip.addr.ipv6.data, blog_p->tupleV6.daddr.p8, 16);
        ip_flow_p->key.src_port = ntohs(blog_p->tupleV6.port.source);
        ip_flow_p->key.dst_port = ntohs(blog_p->tupleV6.port.dest);

        __debug("IPv6 Src " IP6PHEX "\n", IP6(ip_flow_p->key.src_ip.addr.ipv6.data), ip_flow_p->key.src_port);
        __debug("IPv6 Dst " IP6PHEX "\n", IP6(ip_flow_p->key.dst_ip.addr.ipv6.data), ip_flow_p->key.dst_port);
    }
    else
#endif
    {
        ip_flow_p->key.src_ip.family = bdmf_ip_family_ipv4;
        ip_flow_p->key.src_ip.addr.ipv4 = ntohl(blog_p->rx.tuple.saddr);
        ip_flow_p->key.dst_ip.family = bdmf_ip_family_ipv4;
        ip_flow_p->key.dst_ip.addr.ipv4 = ntohl(blog_p->rx.tuple.daddr);
        ip_flow_p->key.src_port = ntohs(blog_p->rx.tuple.port.source);
        ip_flow_p->key.dst_port = ntohs(blog_p->rx.tuple.port.dest);

        __debug("IPv4 Src <%pI4:%u>\n", &blog_p->rx.tuple.saddr, ip_flow_p->key.src_port);
        __debug("IPv4 Dst <%pI4:%u>\n", &blog_p->rx.tuple.daddr, ip_flow_p->key.dst_port);
    }

    __debug("\n");

    /* Check if the flow is routed or bridged */

    ip_flow_p->result.is_routed = 0;

#if defined(CC_PKTRUNNER_IPV6)
    if (MAPT(blog_p))
    {
        ip_flow_p->result.is_routed = 1;
        ip_flow_p->result.is_df = blog_p->is_df;

        if (MAPT_UP(blog_p))
            ip_flow_p->result.is_mapt_us = 1;
    }
    else if(CHK4in6(blog_p) || CHK6in4(blog_p) || MAPT(blog_p))
    {
        ip_flow_p->result.is_routed = 1;
    }
    else if(CHK6to6(blog_p))
    {
        if(blog_p->tupleV6.rx_hop_limit != blog_p->tupleV6.tx_hop_limit)
        {
            ip_flow_p->result.is_routed = 1;
        }
    }
    else
#endif
    {
        if(CHK4to4(blog_p))
        {
            if(blog_p->rx.tuple.ttl != blog_p->tx.tuple.ttl)
            {
                ip_flow_p->result.is_routed = 1;
            }
        }
        else if(RX_GRE(blog_p) || TX_GRE(blog_p))
        {
            if(blog_p->rx.tuple.ttl != blog_p->tx.tuple.ttl)
            {
                ip_flow_p->result.is_routed = 1;
            }            
        }
        else
        {
            __logError("Unable to determine if the flow is routed or bridged");
            return -1;
        }
    }

    ip_flow_p->result.tc = SKBMARK_GET_TC_ID(blog_p->mark);
    ip_flow_p->result.mtu = blog_getTxMtu(blog_p);
    ip_flow_p->result.is_ingqos_high_prio = blog_p->iq_prio;

    ip_flow_p->result.tos = blog_p->rx.tuple.tos;
    ip_flow_p->result.is_l2_accel = 0;

    return 0;
}

int __l2ucastSetFwdAndFilters(Blog_t *blog_p, rdpa_l2_flow_info_t *l2_flow_p)
{
    flowParams_t flow_params = {};
    int err = 0;

    err = __L2L3ParseBlogFlowParams(blog_p, &flow_params);
    if (err)
    {
        return err;
    }

    l2_flow_p->key.ingress_if = flow_params.key.ingress_if;
    l2_flow_p->key.dir = flow_params.key.dir;
    l2_flow_p->key.wan_flow = flow_params.key.wan_flow;
    l2_flow_p->result.cpu_reason = flow_params.result.cpu_reason;
    l2_flow_p->result.egress_if = flow_params.result.egress_if;
    l2_flow_p->result.is_hit_trap = flow_params.result.is_hit_trap;
    l2_flow_p->result.queue_id = flow_params.result.queue_id;
    l2_flow_p->result.wl_metadata = flow_params.result.wl_metadata;
    l2_flow_p->result.wan_flow = flow_params.result.wan_flow;
    l2_flow_p->result.wan_flow_mode = flow_params.result.wan_flow_mode;


    memcpy( &l2_flow_p->key.dst_mac.b[0], &blog_p->rx.l2hdr[0], BLOG_ETH_ADDR_LEN );
    memcpy( &l2_flow_p->key.src_mac.b[0], &blog_p->rx.l2hdr[6], BLOG_ETH_ADDR_LEN );
    l2_flow_p->key.eth_type = ntohs(blog_p->eth_type);
    l2_flow_p->key.vtag0 = ntohl(blog_p->vtag[0]);
    l2_flow_p->key.vtag1 = ntohl(blog_p->vtag[1]);
    l2_flow_p->key.vtag_num = blog_p->vtag_num;
    l2_flow_p->key.tos = blog_p->rx.tuple.tos;
    l2_flow_p->key.tcp_pure_ack = blog_p->key.tcp_pure_ack;

    __debug("Dst MAC <%02x:%02x:%02x:%02x:%02x:%02x>\n", 
        blog_p->rx.l2hdr[0], blog_p->rx.l2hdr[1], blog_p->rx.l2hdr[2],
        blog_p->rx.l2hdr[3], blog_p->rx.l2hdr[4], blog_p->rx.l2hdr[5] );
    __debug("Src MAC <%02x:%02x:%02x:%02x:%02x:%02x>\n", 
        blog_p->rx.l2hdr[6], blog_p->rx.l2hdr[7], blog_p->rx.l2hdr[8],
        blog_p->rx.l2hdr[9], blog_p->rx.l2hdr[10], blog_p->rx.l2hdr[11] );
    __debug("vtag_num = %u, vtag[0] = 0x%08x, vtag[1] = 0x%08x, eth_type = 0x%04x tos= 0x%02x\n", 
            blog_p->vtag_num, ntohl(blog_p->vtag[0]), ntohl(blog_p->vtag[1]), ntohs(blog_p->eth_type), 
            blog_p->rx.tuple.tos) ;
    __debug("\n");

    /* Check if the flow is routed or bridged */
    l2_flow_p->result.is_routed = 0;
    l2_flow_p->result.is_l2_accel = 1;
    l2_flow_p->result.tc = SKBMARK_GET_TC_ID(blog_p->mark);
    l2_flow_p->result.mtu = blog_getTxMtu(blog_p);
    l2_flow_p->result.is_tos_mangle = 0;
    l2_flow_p->result.tos = blog_p->tx.tuple.tos;
    l2_flow_p->result.lag_port = blog_p->lag_port;
    l2_flow_p->result.is_ingqos_high_prio = blog_p->iq_prio;

    /* L2 accel: tos field holds tos value for both IPv4 and IPv6 */
    if(blog_p->rx.tuple.tos != blog_p->tx.tuple.tos)
    {
        l2_flow_p->result.is_tos_mangle = 1;
        l2_flow_p->result.tos = blog_p->tx.tuple.tos;
    }

    return 0;
}
