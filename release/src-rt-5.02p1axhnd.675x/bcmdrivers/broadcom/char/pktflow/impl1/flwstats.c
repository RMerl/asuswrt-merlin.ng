/*
<:copyright-BRCM:2007:proprietary:standard

   Copyright (c) 2007 Broadcom 
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
 *****************************************************************************
 * File Name  : flwstats.c
 *
 * Description: This file contains the Flow Cache Stats related functions.
 *****************************************************************************
 */

/*----- Includes -----*/

#include <linux/kthread.h>
#include <linux/bcm_realtime.h>
#include "flwstats.h"
#include "fcache.h"
#include <linux/jiffies.h>

/*----- Defines -----*/
static spinlock_t flwstats_lock;

#define FLW_STATS_NODE_IN_USE  0x1
#define FLW_STATS_NODE_ACTIVE  0x2

#define FLW_STATS_END_IDX	MAX_FLW_STATS_QUERIES

typedef struct {
    FlwStatsQueryTuple_t  queryTuple;
    FlwStats_t            flwSt;
    uint8_t               nodeFlag;
    uint16_t              prevIdx;
    uint16_t              nextIdx;
}FlwStatsNode_t;

static FlwStatsNode_t  flwStTbl[MAX_FLW_STATS_QUERIES];

/* Head index for used node linked list*/
static uint16_t flwTblHeadIdx=FLW_STATS_END_IDX;

/* Head index for free node linked list*/
static uint16_t flwTblFreeHeadIdx=FLW_STATS_END_IDX;	

/* Array to hold previous iteration flw stats so that
   Delta Stats can be calculated*/
static BlogFcStats_t *prevSt;
static int prevStSz=0;

/* Stub to initialize the fc_getStats pointer */
int getFlwStatsFnStub(uint32_t param1, 
                      unsigned long param2, 
                      unsigned long param3)
{
    return -1;
}

HOOK3PARM getFlwStatsFn = (HOOK3PARM) getFlwStatsFnStub;

#define FLWSTATS_LOCK()     spin_lock_bh(&flwstats_lock)
#define FLWSTATS_UNLOCK()   spin_unlock_bh(&flwstats_lock)

extern int  fcache_max_ent(void);

#define USER_TASKSET_CMD  "/usr/bin/chrt"

/* Default priority is 0 for all low priority tasks. Exercise 
 * caution while changing this value as it can impact throughput
 */
#define BCM_FLWSTATS_RT_THREAD_PRIO 0 

#define BCM_FLWSTATS_DEFAULT_POLL_INTERVAL_IN_MSEC 1000
#define BCM_FLWSTATS_DEFAULT_GRANULARITY 30

static int flwPollInterval = BCM_FLWSTATS_DEFAULT_POLL_INTERVAL_IN_MSEC;

/* (fcache_max_ent/flwPollGranularity) flows will be checked on every poll */
static int flwPollGranularity = BCM_FLWSTATS_DEFAULT_GRANULARITY;

static int
call_usermodehelper_chrt(int pid, 
                         int threadprio)
{
    int rc;
    char pidbuf[16]={0};
    char priobuf[16]={0};
    char *argv[] = { USER_TASKSET_CMD, "-p", priobuf, pidbuf, NULL };
    char *envp[] = { NULL };
    sprintf(pidbuf, "%d", pid);
    sprintf(priobuf, "%d", threadprio);
    rc = call_usermodehelper(USER_TASKSET_CMD, argv, envp, UMH_WAIT_PROC);
    return rc;
}

static int getDeltaFlwStats(uint32_t flwIdx, 
                            BlogFcStats_t *flwStFc_p, 
                            FlwStats_t *deltaSt_p)
{

    /* Initialize Delta */
    deltaSt_p->rx_packets = 0;
    deltaSt_p->rx_bytes   = 0;
    deltaSt_p->rx_rtp_packets_lost = 0;

    /* Note that if the last poll was too long ago or there has been heavy 
     * traffic since the last poll, the stats can roll over multiple times 
     * and the delta cannot be calculated. 
     */
    if (flwStFc_p->rx_packets >= prevSt[flwIdx].rx_packets)
    {
        deltaSt_p->rx_packets = flwStFc_p->rx_packets - 
                                prevSt[flwIdx].rx_packets;
    }
    else
    {
        /* This is the roll over scenario. */
        deltaSt_p->rx_packets = (ULONG_MAX - prevSt[flwIdx].rx_packets) + 
                                 flwStFc_p->rx_packets;
    }

    /* Note that if the last poll was too long ago, the stats can roll over
     * multiple times and the delta could be incorrect. 
     */
    if (flwStFc_p->rx_bytes >= prevSt[flwIdx].rx_bytes)
    {
        deltaSt_p->rx_bytes = flwStFc_p->rx_bytes - prevSt[flwIdx].rx_bytes;
    }
    else
    {
        /* This is the roll over scenario. flowcache stores the bytes as 
         * unsigned long. So get the roll over delta using ULONG_MAX
         */
        deltaSt_p->rx_bytes = (ULONG_MAX - prevSt[flwIdx].rx_bytes) + 
                              flwStFc_p->rx_bytes;
    }

    /* CAUTION: Cannot take care of roll over scenario. 
     * RTP packets lost counter can have negative value due to duplicates
     * (refer to RFC 3550 section 6.4.1)
     */
    deltaSt_p->rx_rtp_packets_lost = flwStFc_p->rx_rtp_packets_lost - 
                            prevSt[flwIdx].rx_rtp_packets_lost;

    /* Update the previous iteration flow stats entry as the Delta 
     * has been calculated
     */
    prevSt[flwIdx].rx_packets = flwStFc_p->rx_packets;
    prevSt[flwIdx].rx_bytes   = flwStFc_p->rx_bytes;
    prevSt[flwIdx].rx_rtp_packets_lost = flwStFc_p->rx_rtp_packets_lost;
    prevSt[flwIdx].pollTS_ms  = flwStFc_p->pollTS_ms;


    return 0;
}

static inline bool matchMACAddr(uint8_t *macAddr1, 
                                uint8_t *macAddr2)
{
    int i = 0;
    for (i=0; i < ETH_ALEN; i++)
    {
        if (macAddr1[i] != macAddr2[i])
        {
            return false;
        }
    }
    return true;
}

static inline bool matchV6Addr(uint32_t *V6Addr1, 
                               uint32_t *V6Addr2)
{
    int i = 0;
    for (i=0; i < 4; i++)
    {
        if (V6Addr1[i] != V6Addr2[i])
        {
            return false;
        }
    }
    return true;
}

static inline bool matchPHY(uint8_t *phy1, 
                            void    *dev_p)
{
    uint8_t *devName=NULL;

    if ((devName = (uint8_t *) blog_request(NETDEV_NAME, dev_p, 0, 0)) == 0)
    {
        return false;
    }

    if (strcasecmp(phy1, devName) != 0)
    {
        return false;
    }

    return true;
}

static bool matchQueryAgainstFlow(FlwStatsNode_t *queryNode_p, 
                                  Blog_t *blog_p)
{
    FlwStatsQueryTuple_t *queryTuple = &(queryNode_p->queryTuple);

    if (!queryTuple->mask)
    {
        return false;
    }

    if (   ((queryTuple->mask & FLWSTATS_QUERYMASK_SRCIPVER) &&
            ((RX_IPV4(blog_p) && queryTuple->srcipver != FLWSTATS_IPVER_V4)
            || (RX_IPV6(blog_p) && queryTuple->srcipver != FLWSTATS_IPVER_V6) ) )
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_DSTIPVER) &&
            ((RX_IPV4(blog_p) && queryTuple->dstipver != FLWSTATS_IPVER_V4)
            || (RX_IPV6(blog_p) && queryTuple->dstipver != FLWSTATS_IPVER_V6) ) )
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_V4SRC) && 
            (!RX_IPV4(blog_p) ||
             (RX_IPV4(blog_p) &&
              queryTuple->v4srcaddr != blog_p->rx.tuple.saddr)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_V4RXDST) && 
            (!RX_IPV4(blog_p) ||
             (RX_IPV4(blog_p) &&
              queryTuple->v4rxdstaddr != blog_p->rx.tuple.daddr)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_V4DST) && 
            (!TX_IPV4(blog_p) ||
             (TX_IPV4(blog_p) &&
              queryTuple->v4dstaddr != blog_p->tx.tuple.daddr)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_V6SRC) && 
            (!RX_IPV6(blog_p) ||
             (RX_IPV6(blog_p) &&
              !(matchV6Addr(queryTuple->v6srcaddr, blog_p->tupleV6.saddr.p32)))))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_V6DST) && 
            (!TX_IPV6(blog_p) ||
             (TX_IPV6(blog_p) &&
              !(matchV6Addr(queryTuple->v6dstaddr, blog_p->tupleV6.daddr.p32)))))
           /* Src Port and dst port are not populated by fcache for all 
            * multicast flows. Hence it is not reliable to filter by layer 4 
            * src or dst port for multicast flows. flwstats users are 
            * recommended to filter by Multicast Group address.
            */
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_L4SRCPRT) &&   
             ((RX_IPV4(blog_p) && 
               queryTuple->l4srcport != blog_p->rx.tuple.port.source) || 
              (RX_IPV6(blog_p) &&
               queryTuple->l4srcport != blog_p->tupleV6.port.source)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_L4DSTPRT) && 
            ((TX_IPV4(blog_p) && 
              queryTuple->l4dstport != blog_p->tx.tuple.port.dest) ||
             (TX_IPV6(blog_p) &&
              queryTuple->l4dstport != blog_p->tupleV6.port.dest)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_IPPROTO)  &&
             queryTuple->ipproto   != blog_p->key.protocol)
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_INVID)    && 
             ((blog_p->vtag_num == 0) ||
              (blog_p->vtag_num == 1 &&
               ntohs(queryTuple->innervid) != (ntohl(blog_p->vtag[0]) & 0x0FFF)) || 
              (blog_p->vtag_num == 2 && 
               ntohs(queryTuple->innervid) != (ntohl(blog_p->vtag[1]) & 0x0FFF))))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_OUTVID)   && 
             ((blog_p->vtag_num != 2) ||
              (blog_p->vtag_num == 2 &&
               ntohs(queryTuple->outervid) != (ntohl(blog_p->vtag[0]) & 0x0FFF))))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_MACSRC) && 
            !(matchMACAddr(queryTuple->macSA, 
                           &blog_p->rx.l2hdr[ETH_ALEN])))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_MACDST) && 
            !(matchMACAddr(queryTuple->macDA, 
                           blog_p->tx.l2hdr)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_SRCPHY) && 
            !(matchPHY(queryTuple->srcphy, 
                       blog_p->rx_dev_p)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_DSTPHY) && 
            !(matchPHY(queryTuple->dstphy, 
                       blog_p->tx_dev_p)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_INRXMACSRC) && 
            !(matchMACAddr(queryTuple->inRxMacSA, &blog_p->grerx.l2hdr[ETH_ALEN])))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_INRXMACDST) && 
            !(matchMACAddr(queryTuple->inRxMacDA, blog_p->grerx.l2hdr)))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_INTXMACSRC) && 
            !(matchMACAddr(queryTuple->inTxMacSA, &blog_p->gretx.l2hdr[ETH_ALEN])))
        || ((queryTuple->mask & FLWSTATS_QUERYMASK_INTXMACDST) && 
            !(matchMACAddr(queryTuple->inTxMacDA, blog_p->gretx.l2hdr)))
       )
    {
        return false;
    }

    return true;
}

static int updateStatsTable(uint32_t flwIdx,
                            Blog_t *blog_p,
                            BlogFcStats_t *flwStFc_p)
{
    FlwStats_t deltaStats;   
    uint16_t tempIdx = 0;

    /* Protect against userspace call to get stats */
    FLWSTATS_LOCK();

    if (flwTblHeadIdx == FLW_STATS_END_IDX)
    {
        /* No Query Nodes in table. No updates needed */
        FLWSTATS_UNLOCK();
        return 0;
    }

    /* Get Delta stats and apply Delta on Query Nodes */
    if (getDeltaFlwStats(flwIdx, flwStFc_p, &deltaStats) != 0)
    {
        /* Error, skip this flow */
        FLWSTATS_UNLOCK();
        return -1;
    }

    /* Traverse the Query nodes and apply the Delta stats on matching nodes */
    tempIdx = flwTblHeadIdx;

    while (tempIdx != FLW_STATS_END_IDX)
    {
        if ((flwStTbl[tempIdx].nodeFlag & FLW_STATS_NODE_ACTIVE) &&  matchQueryAgainstFlow(&flwStTbl[tempIdx], blog_p))
        {
            flwStTbl[tempIdx].flwSt.rx_packets += deltaStats.rx_packets;
            flwStTbl[tempIdx].flwSt.rx_bytes   += deltaStats.rx_bytes;
            flwStTbl[tempIdx].flwSt.rx_rtp_packets_lost += deltaStats.rx_rtp_packets_lost;
        }
        tempIdx = flwStTbl[tempIdx].nextIdx;
    }
    FLWSTATS_UNLOCK();

    return 0;
}

static int flwStatsActivateQueries(void)
{
    uint16_t tempIdx = 0;

    FLWSTATS_LOCK();
    tempIdx = flwTblHeadIdx;
    while (tempIdx != FLW_STATS_END_IDX)
    {
        if ((flwStTbl[tempIdx].nodeFlag & FLW_STATS_NODE_IN_USE) && 
            !(flwStTbl[tempIdx].nodeFlag & FLW_STATS_NODE_ACTIVE))
        {
            flwStTbl[tempIdx].nodeFlag |= FLW_STATS_NODE_ACTIVE;
        }
        tempIdx = flwStTbl[tempIdx].nextIdx;
    }   
    FLWSTATS_UNLOCK();
    return 0;
}

static int flwStatsThread(void *thread_data)
{
    Blog_t *blog_p;
    int flwIdx;
    BlogFcStats_t flwStatsFc;   
    int flwIdxStart=1; /*Flow 0 is invalid in fcache*/
    int flwIdxEnd=0; 
    int numFlwsPerPoll=0;
    int maxFlwIdx = 0;

    maxFlwIdx = fcache_max_ent() - 1;
    numFlwsPerPoll = maxFlwIdx / flwPollGranularity;

    flwIdxEnd = numFlwsPerPoll;
    printk("flwStatsThread created. numFlwsPerPoll %d maxFlwIdx %d\n",
            numFlwsPerPoll, maxFlwIdx);

    while(!kthread_should_stop())
    {
        FLWSTATS_LOCK();
        if (flwTblHeadIdx == FLW_STATS_END_IDX)
        {
            /* No Query nodes in the list. Go to sleep */
            FLWSTATS_UNLOCK();
            goto poll_delay;
        }
        FLWSTATS_UNLOCK();

        if (flwIdxEnd < flwIdxStart)
        {
            printk("Critical Error flwIdxEnd %d < flwIdxStart %d\n", 
                    flwIdxEnd, flwIdxStart);
            break;
        }

        for (flwIdx = flwIdxStart; flwIdx <= flwIdxEnd; flwIdx++)
        {
            BLOG_LOCK_BH();
            if (getFlwStatsFn(flwIdx, (unsigned long)&blog_p, (unsigned long)&flwStatsFc) == 0)
            {
                flwStatsFc.pollTS_ms = jiffies_to_msecs(jiffies);
                updateStatsTable(flwIdx, blog_p, &flwStatsFc);
            }
            BLOG_UNLOCK_BH();
        }

        numFlwsPerPoll = maxFlwIdx / flwPollGranularity;
        if (flwIdxEnd != maxFlwIdx)
        {
            flwIdxStart = flwIdxEnd + 1;
            if ((flwIdxStart + numFlwsPerPoll) <= maxFlwIdx)
            {
                flwIdxEnd = flwIdxStart + numFlwsPerPoll;
            }
            else
            {
                flwIdxEnd = maxFlwIdx;
            }
        }
        else
        {
            flwIdxStart = 1; /*Flow 0 is invalid in fcache */
            flwIdxEnd   = numFlwsPerPoll;

            /* One iteration completed. Make all inactive queries active
             * This is done to normalize the query stats so that all
             * Query nodes start counting the stats from the time the
             * queries got created
             */
            flwStatsActivateQueries();
        }

poll_delay:
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(msecs_to_jiffies(flwPollInterval));
    }
    return 0;
}

int flwStatsPushCallbk(uint32_t flwIdx, unsigned long ublog_p,
         unsigned long stats_p, unsigned long nReset)
{
    Blog_t *blog_p = (Blog_t *)ublog_p;
    BlogFcStats_t *flwStFc_p = (BlogFcStats_t *)stats_p; 

    /*No BLOG_LOCK needed as this function is called from fcache context*/

    flwStFc_p->pollTS_ms = jiffies_to_msecs(jiffies);
    updateStatsTable(flwIdx, blog_p, flwStFc_p);

    if (nReset)
    {
        /* Clear the previous iteration flow stats entry as the flow 
         * is going to be evicted or the stats are going to be reset
         */
        prevSt[flwIdx].rx_packets = 0;
        prevSt[flwIdx].rx_bytes   = 0;
        prevSt[flwIdx].rx_rtp_packets_lost = 0;
    }

    return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function Name: flwStatsInitTbl
 * Description  : Initialize Flow Stats Table.  Put all entries on free list
 *----------------------------------------------------------------------------
 */
static void flwStatsInitTbl(void)
{
    int i=0;

    /* Protect against flowstats thread call to get stats */
    FLWSTATS_LOCK();	

    for (i = 0; i < MAX_FLW_STATS_QUERIES; i++)
    {
        /* Zero out fields */
        memset(&(flwStTbl[i].queryTuple), 0, sizeof(flwStTbl[i].queryTuple));
        memset(&(flwStTbl[i].flwSt), 0, sizeof(flwStTbl[i].flwSt));

        /* Mark node as free.  Be sure to set first node's prevIdx
           and last node's nextIdx to invalid values. */
        flwStTbl[i].nodeFlag = 0;
        flwStTbl[i].nextIdx  = (i==(MAX_FLW_STATS_QUERIES-1)) ? 
                                FLW_STATS_END_IDX : i+1;
        flwStTbl[i].prevIdx  = (i==0) ? FLW_STATS_END_IDX : i-1;
    }

    /* Set head index for used list to invalid value; for free list,
       to first entry. */
    flwTblHeadIdx = FLW_STATS_END_IDX;
    flwTblFreeHeadIdx=0;

    /* Unlock flowstats table and variables */
    FLWSTATS_UNLOCK();	
}

/*
 *----------------------------------------------------------------------------
 * Function Name: flwStatsInit
 * Description  : Initialize flow stats feature
 *----------------------------------------------------------------------------
 */
int flwStatsInit(void)
{
    struct task_struct *tsk;

    spin_lock_init(&flwstats_lock);

    prevStSz = fcache_max_ent() * sizeof(*prevSt);
    if( (prevSt = kmalloc(prevStSz, GFP_ATOMIC)) == NULL )
    {
        /* release all allocated receive buffers */
        printk("ERROR %s Low memory.\n", __FUNCTION__);
        return -ENOMEM;
    }

    /* Initialize the prevSt array*/
    memset(prevSt, 0, prevStSz);

    tsk = kthread_create(flwStatsThread, NULL, "bcmFlwStatsTask");

    if (IS_ERR(tsk)) {
        printk("flwStatsThread creation failed\n");
        kfree(prevSt);
        return -1;
    }
    wake_up_process(tsk);

    call_usermodehelper_chrt(tsk->pid, BCM_FLWSTATS_RT_THREAD_PRIO);

    /* Initialize Flow Stats Table */
    flwStatsInitTbl();

    fc_flwstats_bind(&getFlwStatsFn, (HOOK4PARM)flwStatsPushCallbk);

    return 0;
}

/*
 *-------------------------------------------------------------------------
 * Function Name: flwStatsGetQueryNum
 * Description  : Get all query entry number.
 * Returns      : active queries number
 *-------------------------------------------------------------------------
 */
int flwStatsGetQueryNum(void)
{
    int queryIdx;
    int query_number = 0;

    /* Protect against flowstats thread call to get stats */
    FLWSTATS_LOCK();

    queryIdx = flwTblHeadIdx;	

    /* Navigate the list for all valid values */
    while(queryIdx != FLW_STATS_END_IDX )
    {
        /* Go to next entry in list */
        queryIdx = flwStTbl[queryIdx].nextIdx;

        query_number++;
    }

    /* Unlock flowstats table and variables */
    FLWSTATS_UNLOCK();	

    return query_number;
}

/*
 *-------------------------------------------------------------------------
 * Function Name: flwStatsDumpToStruct
 * Description  : Dumps all active queries to structure, then copy to user space.
 * Returns      : None
 *-------------------------------------------------------------------------
 */
int flwStatsDumpToStruct(FlwStatsDumpInfo_t *flwStDumpInfo_p, FlwStatsDumpEntryInfo_t *flwStDumpEntry_p)
{
    int queryIdx;
    int i, nRet=0;
    int query_number = 0;
    FlwStatsDumpEntryInfo_t *flwStDumpEntry = flwStDumpEntry_p;

    /* Protect against flowstats thread call to get stats */
    FLWSTATS_LOCK();

    queryIdx = flwTblHeadIdx;	

        /* Navigate the list for all valid values */
        while(queryIdx != FLW_STATS_END_IDX )
        {
        if(query_number == flwStDumpInfo_p->num_entries) 
        {
            nRet = -1;
            break;
        }

            /* Dump statistics */		
        flwStDumpEntry->queryIdx = queryIdx;
        flwStDumpEntry->flwSt.rx_packets = flwStTbl[queryIdx].flwSt.rx_packets;
        flwStDumpEntry->flwSt.rx_bytes = flwStTbl[queryIdx].flwSt.rx_bytes;
        flwStDumpEntry->flwSt.rx_rtp_packets_lost = flwStTbl[queryIdx].flwSt.rx_rtp_packets_lost;
        flwStDumpEntry->nodeFlag = flwStTbl[queryIdx].nodeFlag;
                    
            /* Dump tuple data */
        memcpy(&(flwStDumpEntry->queryTuple), &(flwStTbl[queryIdx].queryTuple), 
           sizeof(FlwStatsQueryTuple_t));
                    
            /* Go to next entry in list */
            queryIdx = flwStTbl[queryIdx].nextIdx;

        query_number++;

        flwStDumpEntry++;
        }
        
    if(nRet)
    {
        /* Unlock flowstats table and variables */
       FLWSTATS_UNLOCK();	

       return nRet;
    }

    for (i = 0; i < PREV_FLW_STATS_DUMP_ENTRIES; i++)
    {
        flwStDumpInfo_p->prevFlwStEntries[i].rx_packets = prevSt[i].rx_packets;
        flwStDumpInfo_p->prevFlwStEntries[i].rx_bytes = prevSt[i].rx_bytes;
        flwStDumpInfo_p->prevFlwStEntries[i].rx_rtp_packets_lost = prevSt[i].rx_rtp_packets_lost;
    }

    /* Unlock flowstats table and variables */
    FLWSTATS_UNLOCK();	

    return 0;
}

/*
 *---------------------------------------------------------------------------
 * Function Name: flwStatsAddNode
 * Description  : Insert a new node into the query table.
 * Returns      : None
 *---------------------------------------------------------------------------
 */
static int flwStatsAddNode(FlwStatsQueryInfo_t *newQuery)
{
    uint16_t newIdx;

    /* Protect against flowstats thread call to get stats */
    FLWSTATS_LOCK();	

    /* Are there free nodes available? */
    if(flwTblFreeHeadIdx==FLW_STATS_END_IDX)
    {
        FLWSTATS_UNLOCK();	
        return(FLWSTATS_ERR_QUERY_NOT_FOUND);
    }

    /* First, pull a free node off the front of the free list */
    newIdx = flwTblFreeHeadIdx;
    flwTblFreeHeadIdx = flwStTbl[flwTblFreeHeadIdx].nextIdx;
    if (flwTblFreeHeadIdx != FLW_STATS_END_IDX) {
       flwStTbl[flwTblFreeHeadIdx].prevIdx = FLW_STATS_END_IDX;
    }
    
    /* Now, insert the new node at the front of the used list */
    if(flwTblHeadIdx!=FLW_STATS_END_IDX)
       flwStTbl[flwTblHeadIdx].prevIdx = newIdx;
    flwStTbl[newIdx].prevIdx = FLW_STATS_END_IDX;
    flwStTbl[newIdx].nextIdx = flwTblHeadIdx;
    flwTblHeadIdx = newIdx;
    
    /* Now copy tuple to the new node in the list and initialize
       all other fields */
    memcpy(&(flwStTbl[newIdx].queryTuple), &(newQuery->create.queryTuple), 
           sizeof(FlwStatsQueryTuple_t));
    memset(&(flwStTbl[newIdx].flwSt), 0, sizeof(FlwStats_t));

    /* Node that the Node is not set to Active
     * Node will be active after 1 iteration of getting the flow stats
     */
    flwStTbl[newIdx].nodeFlag = FLW_STATS_NODE_IN_USE; 

    /* Unlock flowstats table and variables */
    FLWSTATS_UNLOCK();	

    /* Set up return handle so user can use it later */
    newQuery->create.handle = newIdx;

    return 0;
}

/*
 *---------------------------------------------------------------------------
 * Function Name: flwStatsDeleteNode
 * Description  : Delete a node from the query table pointed by queryIdx.
 * Returns      : None
 *---------------------------------------------------------------------------
 */
static int flwStatsDeleteNode(uint16_t queryIdx)
{
    uint16_t prevIdx, nextIdx;

    /* Protect against flowstats thread call to get stats */
    FLWSTATS_LOCK();
        
    /* Does the handle point to a valid (i.e. initialized) query? */
    if(!(flwStTbl[queryIdx].nodeFlag & FLW_STATS_NODE_IN_USE))
    {
        /* Query not in use - return an error */
        FLWSTATS_UNLOCK();	
        return FLWSTATS_ERR_QUERY_NOT_FOUND;
    }		

    /* We have a valid query.  Get indices of previous and next
       nodes in list. */
    prevIdx = flwStTbl[queryIdx].prevIdx;
    nextIdx = flwStTbl[queryIdx].nextIdx;	
        
    /* Delete references to node from previous node. */
    if(prevIdx == FLW_STATS_END_IDX)
    {
        /* This is the head of the list, so there is no previous node.
           Just set head pointer */
        flwTblHeadIdx = nextIdx;
    }
    else
    {
        /* Not the head pointer - fix previous node's pointer */
        flwStTbl[prevIdx].nextIdx = nextIdx;
    }
        
    /* Now, clean up the next node's links. */
    if(nextIdx != FLW_STATS_END_IDX)
    {
        /* Not the tail node - fix the next node's pointer */
        flwStTbl[nextIdx].prevIdx = prevIdx;
    }
        
    /* Node is out of the list now.  Erase the unused node's data
       and mark node as free.  */
    memset(&(flwStTbl[queryIdx].queryTuple), 0, sizeof(FlwStatsQueryTuple_t));
    memset(&(flwStTbl[queryIdx].flwSt), 0, sizeof(FlwStats_t));
    flwStTbl[queryIdx].nodeFlag = 0;	
        
    /* Return node to the head end of the free list */
    if(flwTblFreeHeadIdx != FLW_STATS_END_IDX)
    {
        flwStTbl[flwTblFreeHeadIdx].prevIdx = queryIdx;
    }

    flwStTbl[queryIdx].nextIdx = flwTblFreeHeadIdx;
    flwStTbl[queryIdx].prevIdx = FLW_STATS_END_IDX;
    flwTblFreeHeadIdx = queryIdx;

    /* Unlock flowstats table and variables */
    FLWSTATS_UNLOCK();	

    return 0;
}

/*
 *---------------------------------------------------------------------------
 * Function Name: flwStatsCreateQuery
 * Description  : Insert a new query into the query table.
 * Returns      : None
 *---------------------------------------------------------------------------
 */
int flwStatsCreateQuery(FlwStatsQueryInfo_t *newQuery)
{
    int ret = 0;

    if (newQuery->create.queryTuple.mask & FLWSTATS_QUERYMASK_IPPROTO &&
        newQuery->create.queryTuple.ipproto != FLWSTATS_PROTO_TCP &&
        newQuery->create.queryTuple.ipproto != FLWSTATS_PROTO_UDP)
    {
        /* Only TCP and UDP protocols are supported */
        return(FLWSTATS_ERR_NO_PROTO_SUPPORT);
    }

    // Store L4 port and VID in network order as it is stored in nw order in the blog
    newQuery->create.queryTuple.l4srcport = htons(newQuery->create.queryTuple.l4srcport);
    newQuery->create.queryTuple.l4dstport = htons(newQuery->create.queryTuple.l4dstport);
    newQuery->create.queryTuple.innervid  = htons(newQuery->create.queryTuple.innervid);
    newQuery->create.queryTuple.outervid  = htons(newQuery->create.queryTuple.outervid);

    ret = flwStatsAddNode(newQuery);

    return ret;
}

EXPORT_SYMBOL(flwStatsCreateQuery);

/*
 *---------------------------------------------------------------------------
 * Function Name: flwStatsGetQuery
 * Description  : Return query results to the caller.
 * Returns      : None
 *---------------------------------------------------------------------------
 */
int flwStatsGetQuery(FlwStatsQueryInfo_t *pQueryInfo)
{
    uint16_t queryIdx = pQueryInfo->get.handle;

    /* Is the handle within valid range? */
    if(queryIdx >= MAX_FLW_STATS_QUERIES)
    {
       /* Too big - return an error */
       return FLWSTATS_ERR_BAD_PARAMS;
    }

    /* Protect against flowstats thread call to get stats */
    FLWSTATS_LOCK();

    /* Does the handle point to a valid (i.e. initialized) query? */
    if(!(flwStTbl[queryIdx].nodeFlag & FLW_STATS_NODE_IN_USE))
    {
       /* Query not in use - return an error */
        FLWSTATS_UNLOCK();	
        return FLWSTATS_ERR_QUERY_NOT_FOUND;
    }

    /* We have a valid query.  Copy query info to the return structure */
    pQueryInfo->get.flwSt.rx_packets = flwStTbl[queryIdx].flwSt.rx_packets;
    pQueryInfo->get.flwSt.rx_bytes   = flwStTbl[queryIdx].flwSt.rx_bytes;		
    pQueryInfo->get.flwSt.rx_rtp_packets_lost = flwStTbl[queryIdx].flwSt.rx_rtp_packets_lost;		

    /* Unlock flowstats table and variables */
    FLWSTATS_UNLOCK();	

    /* Our work is done here - ride off into the sunset and return a
       valid error code. */	
    return 0;
}
EXPORT_SYMBOL(flwStatsGetQuery);

/*
 *----------------------------------------------------------------------------
 * Function Name: flwStatsDeleteQuery
 * Description  : Delete indicated query.
 * Returns      : None
 *----------------------------------------------------------------------------
 */
int flwStatsDeleteQuery(FlwStatsQueryInfo_t *pQueryInfo)
{
    uint16_t queryIdx = pQueryInfo->delete.handle;
    int ret = 0;

    /* Is the handle within valid range? */
    if(queryIdx != ALL_STATS_QUERIES_HANDLE &&
       queryIdx >= MAX_FLW_STATS_QUERIES)
    {
       /* Too big - return an error */
       return FLWSTATS_ERR_BAD_PARAMS;
    }

    if (queryIdx == ALL_STATS_QUERIES_HANDLE)
    {
        flwStatsInitTbl();
    }
    else
    {
        ret = flwStatsDeleteNode(queryIdx);
    }

    /* Return a valid error code. */	
    return ret;
}
EXPORT_SYMBOL(flwStatsDeleteQuery);


/*
 *----------------------------------------------------------------------------
 * Function Name: flwStatsClearQuery
 * Description  : Clear the statistics counters of the indicated query.
 * Returns      : None
 *----------------------------------------------------------------------------
 */
int flwStatsClearQuery(FlwStatsQueryInfo_t *pQueryInfo)
{
    uint16_t queryIdx = pQueryInfo->clear.handle;
    int i=0;

    /* Is the handle within valid range? */
    if(queryIdx != ALL_STATS_QUERIES_HANDLE &&
       queryIdx >= MAX_FLW_STATS_QUERIES)
    {
        /* Too big - return an error */
        return FLWSTATS_ERR_BAD_PARAMS;
    }

    /* Protect against flowstats thread call to get stats */
    FLWSTATS_LOCK();		

    if (queryIdx == ALL_STATS_QUERIES_HANDLE)
    {
        for (i = 0; i < MAX_FLW_STATS_QUERIES; i++)
        {
            flwStTbl[i].flwSt.rx_packets = 0;
            flwStTbl[i].flwSt.rx_bytes = 0;	
            flwStTbl[i].flwSt.rx_rtp_packets_lost = 0;	
        }
    }
    else
    {
        /* Does the handle point to a valid (i.e. initialized) query? */
        if(!(flwStTbl[queryIdx].nodeFlag & FLW_STATS_NODE_IN_USE))
        {
            /* Query not in use - return an error */
            FLWSTATS_UNLOCK();	
            return FLWSTATS_ERR_QUERY_NOT_FOUND;
        }
        
        /* We have a valid query.  Clear the counters and return a
           valid error code. */
        flwStTbl[queryIdx].flwSt.rx_packets = 0;
        flwStTbl[queryIdx].flwSt.rx_bytes = 0;	
        flwStTbl[queryIdx].flwSt.rx_rtp_packets_lost = 0;	
    }

    /* Unlock flowstats table and variables */
    FLWSTATS_UNLOCK();	

    return 0;
}
EXPORT_SYMBOL(flwStatsClearQuery);

int flwStatsGetPollParams(FlwStatsPollParams_t *pollParams)
{
    pollParams->intval = flwPollInterval;
    pollParams->granularity = flwPollGranularity;

    return 0;
}

int flwStatsSetPollParams(FlwStatsPollParams_t *pollParams)
{
    flwPollInterval = pollParams->intval;
    flwPollGranularity = pollParams->granularity;

    return 0;
}
