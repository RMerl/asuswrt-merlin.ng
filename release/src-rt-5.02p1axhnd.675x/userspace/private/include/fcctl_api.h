#ifndef __PKT_FCCTL_API_H_INCLUDED__
#define __PKT_FCCTL_API_H_INCLUDED__

/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
************************************************************************/
/***************************************************************************
 * File Name  : fcctl_api.h
 * Description: APIs for library that controls the Broadcom Flow Cache.
 ***************************************************************************/

#include <fcache.h>
#include <flwstats.h>

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlStatus
 * Description  : Displays flow cache status.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlStatus(FcStatusInfo_t *fcStatusInfo_p);

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlEnable
 * Description  : Enables flow cache.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlEnable(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlDisable
 * Description  : Disables flow cache.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlDisable(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlFlush
 * Description  : Flush all flows in flow cache or just flows
 *                for a specific device
  * Parameters  :
 *         arg  : ifindex of a specific interface or 0 for all flows
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlFlush(int arg);

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlFlushDev
 * Description  : Flush all flows for a specific interface
  * Parameters  :
 *         arg  : ifindex of a specific interface
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlFlushDev(int arg);

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlFlushParams
 * Description  : Flush flows in flow cache that meet specified parameters.
 * Parameters   : Pointer to FcFlushParams_t. Fields are:
 *        flags : FCACHE_FLUSH_ALL, FCACHE_FLUSH_FLOW
 *              : FCACHE_FLUSH_DEV
 *              : FCACHE_FLUSH_DSTMAC, FCACHE_FLUSH_SRCMAC, FCACHE_FLUSH_MAC
 *          mac : 6 bytes of data
 *        dev_p : pointer to net device
 *      flow_id : flow id
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlFlushParams(FcFlushParams_t * fcFlushParams_p);


/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlResetStats
 * Description  : Reset the stats of all the flows in flow cache.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlResetStats(void);

#define FCACHE_CONFIG_OPT_DEFER             0       /* Set pkt deferral rate value  */
#define FCACHE_CONFIG_OPT_MONITOR           1       /* Monitor idle activates?      */
#define FCACHE_CONFIG_OPT_MCAST             2       /* Mcast enable/disable         */
#define FCACHE_CONFIG_OPT_IPV6              3       /* IPv6 enable/disable          */
#define FCACHE_CONFIG_OPT_TIMER             4       /* Flow Timer values            */
#define FCACHE_CONFIG_OPT_MCAST_LEARN       5       /* Mcast enable/disable         */
#define FCACHE_CONFIG_OPT_L2TP              6       /* L2TP enable/disable values   */
#define FCACHE_CONFIG_OPT_ACCEL_MODE        7       /* Acceleration mode values     */
#define FCACHE_CONFIG_OPT_TCP_ACK_MFLOWS    8       /* TCP Ack Prioritization       */
#define FCACHE_CONFIG_OPT_HW_ACCEL          9       /* HW Acceleration en/disable   */
#define FCACHE_CONFIG_OPT_LOW_PKT_RATE      10      /* Set low pkt rate value       */
#define FCACHE_CONFIG_OPT_NOTIFY_PROC_MODE  11      /* Set notify processing mode   */
#define FCACHE_CONFIG_OPT_SW_DEFER          12      /* Set pkt SW deferral rate value*/
#define FCACHE_CONFIG_OPT_GRE               13      /* GRE enable/disable values    */
#define FCACHE_CONFIG_OPT_4O6_FRAG          14      /* 4over6 frag support          */
#define FCACHE_CONFIG_OPT_MAX               15       

#define FCACHE_DUMP_OPT_FLOW_INFO   1       /* Dump flow info               */

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlConfig
 * Description  : Configures the flow cache parameters.
 * Parameters   :
 *       option : one of the option to be configured.
 *         arg1 : parameter value
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlConfig(int option, int arg1 );


#if defined(CC_CONFIG_FCACHE_DEBUG)
/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlDebug
 * Description  : Sets the debug level for the layer in flow cache.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int  fcCtlDebug(int layer, int level);
#endif

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlCreateFlwStatsQuery
 * Description  : Creates a statistics query for later use.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlCreateFlwStatsQuery(FlwStatsQueryInfo_t *queryInfo_p); /*  */


/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlGetFlwStats
 * Description  : Retrieves statistics for a query previously set up by a 
 *                call to fcCtlCreateFlwStatsQuery().
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlGetFlwStatsQuery(FlwStatsQueryInfo_t *queryInfo_p); /*  */


/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlDeleteFlwStatsQuery
 * Description  : Deletes one or more queries previously set up by a call  
 *                to fcCtlCreateFlwStatsQuery().
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlDeleteFlwStatsQuery(FlwStatsQueryInfo_t *queryInfo_p);


/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlClearFlwStats
 * Description  : Zeroes out the counters for one or more queries previously  
 *                set up by a call to fcCtlCreateFlwStatsQuery().
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlClearFlwStatsQuery(FlwStatsQueryInfo_t *queryInfo_p);


/*
 *-----------------------------------------------------------------------------
 * Function Name: fcCtlGetQueryEntryNumber
 * Description  : Get Query entry number.
 * Returns      : >=0 - number of queries in fc, <0 - error
 *-----------------------------------------------------------------------------
 */
int fcCtlGetQueryEntryNumber(void);


/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlDumpFlwStats
 * Description  : Dumps all active queries.
 * Returns      : >=0 - entry number, <0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlDumpFlwStats(FlwStatsDumpInfo_t *flwDumpInfo_p);

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlSetTupleParam
 * Description  : Parses string values into FlwStatsQueryInfo_t struct
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlSetTupleParam(char *pcField, char *pcValue, FlwStatsQueryInfo_t *pQuery);

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlGetFlwPollParams/fcCtlSetFlwPollParams
 * Description  : Get/Set flows polling parameters
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlGetFlwPollParams(FlwStatsPollParams_t *pollparams);
int fcCtlSetFlwPollParams(FlwStatsPollParams_t *pollparams);

#endif  /* defined(__PKT_FCCTL_API_H_INCLUDED__) */

