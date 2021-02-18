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
 * File Name  : fcctl_api.c
 * Description: Linux command line utility that controls the Broadcom
 *              Flow Cache.
 ***************************************************************************/

/*** Includes. ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <fcache.h>
#include <fcctl_api.h>
#include <flwstats.h>

static int  fcDevOpen(void);
static int  fcDevIoctl(FcacheIoctl_t ioctl, int arg1, uintptr_t arg2);

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlStatus
 * Description  : Displays flow cache status.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlStatus(FcStatusInfo_t *fcStatusInfo_p)
{
    return fcDevIoctl( FCACHE_IOCTL_STATUS, -1, (uintptr_t)fcStatusInfo_p );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlEnable
 * Description  : Enables flow cache.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlEnable(void)
{
    return fcDevIoctl( FCACHE_IOCTL_ENABLE, 0, 0 );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlDisable
 * Description  : Disables flow cache.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlDisable(void)
{
    return fcDevIoctl( FCACHE_IOCTL_DISABLE, 0, 0 );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlFlush
 * Description  : Flushes one specific or all flows in flow cache.
 *              : flowid=0 to flush all flows; non-0 a specific flow
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlFlush(int flowid)
{
    FcFlushParams_t params = {};
    if ( flowid == 0 )
    {
        params.flags = FCACHE_FLUSH_ALL;
    }
    else
    {
        params.flags = FCACHE_FLUSH_FLOW;
        params.flowid = flowid;
    }
    return fcDevIoctl( FCACHE_IOCTL_FLUSH, -1, (uintptr_t)&params );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlFlushDev
 * Description  : Flushes all flows for a specific device
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlFlushDev(int ifindex)
{
    FcFlushParams_t params = {};
    params.flags = FCACHE_FLUSH_DEV;
    params.devid = ifindex;
    return fcDevIoctl( FCACHE_IOCTL_FLUSH, -1, (uintptr_t)&params );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlFlushParams
 * Description  : Flushes all flows that match given parameters.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlFlushParams(FcFlushParams_t * fcFlushParams_p)
{
    return fcDevIoctl( FCACHE_IOCTL_FLUSH, -1, (uintptr_t)fcFlushParams_p );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlResetStats
 * Description  : Flushes all flows in flow cache.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlResetStats(void)
{
    return fcDevIoctl( FCACHE_IOCTL_RESET_STATS, 0, 0 );
}

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
int fcCtlConfig(int option, int arg1)
{
    int ret = FCACHE_ERROR;

    switch (option)
    {
#if defined(PKTCMF_AVAIL)
        case FCACHE_CONFIG_OPT_DEFER:
            ret = fcDevIoctl( FCACHE_IOCTL_DEFER, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_MONITOR:
            ret = fcDevIoctl( FCACHE_IOCTL_MONITOR, -1, arg1 );
            break;
#endif

        case FCACHE_CONFIG_OPT_SW_DEFER:
            ret = fcDevIoctl( FCACHE_IOCTL_SW_DEFER, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_MCAST:
            ret = fcDevIoctl( FCACHE_IOCTL_MCAST, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_MCAST_LEARN:
            ret = fcDevIoctl( FCACHE_IOCTL_MCAST_LEARN, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_IPV6:
            ret = fcDevIoctl( FCACHE_IOCTL_IPV6, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_TIMER:
            ret = fcDevIoctl( FCACHE_IOCTL_TIMER, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_GRE:
            ret = fcDevIoctl( FCACHE_IOCTL_GRE, -1, arg1 );
            break;
        case FCACHE_CONFIG_OPT_L2TP:
            ret = fcDevIoctl( FCACHE_IOCTL_L2TP, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_ACCEL_MODE:
            ret = fcDevIoctl( FCACHE_IOCTL_ACCEL_MODE, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_TCP_ACK_MFLOWS:
            ret = fcDevIoctl( FCACHE_IOCTL_TCP_ACK_MFLOWS, -1, arg1 );
            break;

        case FCACHE_CONFIG_OPT_HW_ACCEL:
            {
                ret = fcDevIoctl( FCACHE_IOCTL_SET_HW_ACCEL, -1, arg1 );
            }
            break;

        case FCACHE_CONFIG_OPT_4O6_FRAG:
            ret = fcDevIoctl( FCACHE_IOCTL_4O6_FRAG, -1, arg1 );
            break;

        default:
            fprintf( stderr, "invalid config option <%d>\n", option );
            ret = FCACHE_ERROR;
    }

    return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlDump
 * Description  : Dumps the flow information.
 * Parameters   :
 *       option : one of the option to be dumped.
 *         arg1 : parameter value
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlDump(int option, int arg1)
{
    int ret = FCACHE_ERROR;

    switch (option)
    {
        case FCACHE_DUMP_OPT_FLOW_INFO:
            ret = fcDevIoctl( FCACHE_IOCTL_DUMP_FLOW_INFO, -1, arg1 );
            break;

        default:
            fprintf( stderr, "invalid dump option <%d>\n", option );
            ret = FCACHE_ERROR;
    }

    return ret;
}

#if defined(CC_CONFIG_FCACHE_DEBUG)

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlDebug
 * Description  : Sets the debug level for the layer in flow cache.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int  fcCtlDebug(int layer, int level)
{
    if ( layer >= FCACHE_DBG_LAYER_MAX )
        fprintf( stderr, "invalid layer <%d>\n", layer );

    return fcDevIoctl( FCACHE_IOCTL_DEBUG, layer, level );
}
#endif

/*
 *---------------------------------------------------------------------------
 * Function Name: fcCtlCreateFlwStatsQuery
 * Description  : Creates a statistics query for later use.
 * Parameters   :
 *  queryInfo_p : Pointer to an  FlwStatsQueryInfo_t structure where 
 *                the flwStatsQueryTuple_t portion will be used to set
 *                up the query.  The flwStats_t portion will be ignored.
 *                A unique handle would be returned in queryInfo_p for later
 *                use.
 * Returns      : 0 - success, non-0 - error
 *---------------------------------------------------------------------------
 */
int fcCtlCreateFlwStatsQuery(FlwStatsQueryInfo_t *queryInfo_p)
{
	int retVal=0;
	
	/* Issue IOCTL to create a new flowstats query */
    retVal = fcDevIoctl( FCACHE_IOCTL_CREATE_FLWSTATS, -1, (uintptr_t)queryInfo_p);
	
	return(retVal);
}

/*
 *-----------------------------------------------------------------------------
 * Function Name: fcCtlGetFlwStatsQuery
 * Description  : Retrieves statistics for a query previously set up by a 
 *                call to fcCtlCreateFlwStatsQuery().
 * Parameters   :
 *  queryInfo_p : Handle returned by earlier fcCtlCreateFlwStatsQuery() is
 *                passed as input. Results are stored in flwSt field in
 *                queryInfo_p and the query matching the handle
 *                will be filled in queryTuple field
 * Returns      : 0 - success, non-0 - error
 *-----------------------------------------------------------------------------
 */
int fcCtlGetFlwStatsQuery(FlwStatsQueryInfo_t *queryInfo_p)
{
	int retVal = 0;
	
	/* Issue IOCTL to run a flowstats query */
    retVal = fcDevIoctl( FCACHE_IOCTL_GET_FLWSTATS, -1, (uintptr_t)queryInfo_p);
	
	return retVal;
}


/*
 *-----------------------------------------------------------------------------
 * Function Name: fcCtlDeleteFlwStatsQuery
 * Description  : Deletes one or more queries previously set up by a call  
 *                to fcCtlCreateFlwStatsQuery().
 * Parameters   :
 *  queryInfo_p : Handle returned by earlier fcCtlCreateFlwStatsQuery() is
 *                passed as input. queryTuple and flwSt fields in queryInfo_p
 *                will be ignored. The special value 
 *                ALL_STATS_QUERIES_HANDLE may be used to delete all queries.
 * Returns      : 0 - success, non-0 - error
 *-----------------------------------------------------------------------------
 */
int fcCtlDeleteFlwStatsQuery(FlwStatsQueryInfo_t *queryInfo_p)
{
	/* Issue IOCTL to delete a flowstats query */
    return fcDevIoctl( FCACHE_IOCTL_DELETE_FLWSTATS, -1, (uintptr_t)queryInfo_p);	
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlGetFlwPollParams/fcCtlSetFlwPollParams
 * Description  : Get/Set flows polling parameters
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlGetFlwPollParams(FlwStatsPollParams_t *pollparams)
{
    return fcDevIoctl(FCACHE_IOCTL_GET_FLOWSTATS_POLL_PARAMS, -1, (int)(pollparams));	
}

int fcCtlSetFlwPollParams(FlwStatsPollParams_t *pollparams)
{
    return fcDevIoctl(FCACHE_IOCTL_SET_FLOWSTATS_POLL_PARAMS, -1, (int)(pollparams));	
}


/*
 *-----------------------------------------------------------------------------
 * Function Name: fcCtlClearFlwStatsQuery
 * Description  : Zeroes out the counters for one or more queries previously  
 *                set up by a call to fcCtlCreateFlwStatsQuery().
 
 * Parameters   :
 *  queryInfo_p : Handle returned by earlier fcCtlCreateFlwStatsQuery() is
 *                passed as input. queryTuple and flwSt fields in queryInfo_p
 *                will be ignored.  The special value
 *                ALL_STATS_QUERIES_HANDLE may be used to clear the 
 *                counters for all queries.
 * Returns      : 0 - success, non-0 - error
 *-----------------------------------------------------------------------------
 */
int fcCtlClearFlwStatsQuery(FlwStatsQueryInfo_t *queryInfo_p)
{
	/* Issue IOCTL to clear a flowstats query */
    return fcDevIoctl(FCACHE_IOCTL_CLEAR_FLWSTATS, -1, (uintptr_t)queryInfo_p);
}


/*
 *-----------------------------------------------------------------------------
 * Function Name: fcCtlGetQueryEntryNumber
 * Description  : Get Query entry number.
 * Returns      : >=0 - number of queries in fc, <0 - error
 *-----------------------------------------------------------------------------
 */
int fcCtlGetQueryEntryNumber(void)
{	
	/* Issue IOCTL to get number of queries in fc */
    return fcDevIoctl( FCACHE_IOCTL_GET_FLWSTATS_NUM, -1, 0);
}


/*
 *-----------------------------------------------------------------------------
 * Function Name: fcCtlDumpFlwStats
 * Description  : Dumps all active queries to console.
 * Returns      : None
 *-----------------------------------------------------------------------------
 */
int fcCtlDumpFlwStats(FlwStatsDumpInfo_t *flwDumpInfo_p)
{	
	/* Issue IOCTL to delete a flowstats query */
    return fcDevIoctl( FCACHE_IOCTL_DUMP_FLWSTATS, -1, (uintptr_t)flwDumpInfo_p);
}


/*
 *------------------------------------------------------------------------------
 * Function Name: fcDevOpen
 * Description  : Opens the flow cache device.
 * Returns      : device handle if successsful or -1 if error
 *------------------------------------------------------------------------------
 */
static int fcDevOpen(void)
{
    int nFd = open( FCACHE_DRV_DEVICE_NAME, O_RDWR );
    if ( nFd == -1 )
    {
        fprintf( stderr, "open <%s> error no %d\n",
                 FCACHE_DRV_DEVICE_NAME, errno );
        return FCACHE_ERROR;
    }
    return nFd;
} /* fcDevOpen */

/*
 *------------------------------------------------------------------------------
 * Function Name: fcDevIoctl
 * Description  : Ioctls into fcache driver passing the IOCTL command, and the
 *                fcache (arg1) and id (arg2) passed as a 16bit tuple.
 *                If arg1 == -1, then arg2 is passed directly.
 *                CAUTION: Display is done in kernel context.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
static int fcDevIoctl(FcacheIoctl_t ioctl_cmd, int arg1, uintptr_t arg2)
{
    int devFd, ret = FCACHE_ERROR;
    uintptr_t arg = 0;

    if ( arg1 == -1 )
        arg = arg2;
    else
        arg = ( ( (arg1 & 0xFF) << 8) | (arg2 & 0xFF) ) ;

    if ( ( devFd = fcDevOpen() ) == FCACHE_ERROR )
        return FCACHE_ERROR;

    if ( (ret = ioctl( devFd, ioctl_cmd, arg )) == FCACHE_ERROR )
        fprintf( stderr, "fcDevIoctl <%d> error\n", ioctl_cmd );

    close( devFd );
    return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fcCtlSetTupleParam
 * Description  : Parses string values into FlwStatsQueryInfo_t struct
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fcCtlSetTupleParam(char *pcField, char *pcValue, FlwStatsQueryInfo_t *pQuery)
{	
	/* Parse integer fields */
	FILL_TUPLE_FIELD(pQuery,
                     v4srcaddr,
                     FLWSTATS_FIELDNAME_V4SRC,
                     FLWSTATS_QUERYMASK_V4SRC,
                     atoi(pcValue))
	FILL_TUPLE_FIELD(pQuery,
                     v4rxdstaddr,
                     FLWSTATS_FIELDNAME_V4RXDST,
                     FLWSTATS_QUERYMASK_V4RXDST,
                     atoi(pcValue))
	FILL_TUPLE_FIELD(pQuery,
                     v4dstaddr,
                     FLWSTATS_FIELDNAME_V4DST,
                     FLWSTATS_QUERYMASK_V4DST,
                     atoi(pcValue))
	FILL_TUPLE_FIELD(pQuery,
                     l4srcport,
                     FLWSTATS_FIELDNAME_L4SRCPRT,
                     FLWSTATS_QUERYMASK_L4SRCPRT,
                     atoi(pcValue))
	FILL_TUPLE_FIELD(pQuery,
                     l4dstport,
                     FLWSTATS_FIELDNAME_L4DSTPRT,
                     FLWSTATS_QUERYMASK_L4DSTPRT,
                     atoi(pcValue))
	FILL_TUPLE_FIELD(pQuery,
                     ipproto,
                     FLWSTATS_FIELDNAME_IPPROTO,
                     FLWSTATS_QUERYMASK_IPPROTO,
                     atoi(pcValue))
	FILL_TUPLE_FIELD(pQuery,
                     innervid,
                     FLWSTATS_FIELDNAME_INVID,
                     FLWSTATS_QUERYMASK_INVID,
                     atoi(pcValue))
	FILL_TUPLE_FIELD(pQuery,
                     outervid,
                     FLWSTATS_FIELDNAME_OUTVID,
                     FLWSTATS_QUERYMASK_OUTVID,
                     atoi(pcValue))
	
	
	/* Parse IPV4 addresses - source address */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_V4SRC))
	{
		struct in_addr result;  /* Address structure to load up */

		/* Is this a valid IPV4 address? */
		if (inet_pton(AF_INET, pcValue, &result) == 1) // success!
		{
			/* Successfully parsed string into "result" */
			memcpy((void *) &(pQuery->create.queryTuple.v4srcaddr), 
				(void *)&result, sizeof(struct in_addr));
		}
		else
		{
			/* Failed, perhaps not a valid representation of IPV4? */
			printf("ERROR:  Please express '%s' as an IPV4 address."
                   "(i.e. '192.168.2.1').\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;
		}	

		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_V4SRC;
	}
	
	/* Parse IPV4 addresses - RX destination address */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_V4RXDST))
	{	
		struct in_addr result;  /* Address structure to load up */

		/* Is this a valid IPV4 address? */
		if (inet_pton(AF_INET, pcValue, &result) == 1) // success!
		{
			/* Successfully parsed string into "result" */
			memcpy((void *) &(pQuery->create.queryTuple.v4rxdstaddr), 
				(void *)&result, sizeof(struct in_addr));
		}
		else
		{
			/* Failed, perhaps not a valid representation of IPV4? */
			printf("ERROR:  Please express '%s' as an IPV4 address."
                   "(i.e. '192.168.2.1').\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;
		}	
		
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_V4RXDST;
	}	
	
		
	/* Parse IPV4 addresses - destination address */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_V4DST))
	{	
		struct in_addr result;  /* Address structure to load up */

		/* Is this a valid IPV4 address? */
		if (inet_pton(AF_INET, pcValue, &result) == 1) // success!
		{
			/* Successfully parsed string into "result" */
			memcpy((void *) &(pQuery->create.queryTuple.v4dstaddr), 
				(void *)&result, sizeof(struct in_addr));
		}
		else
		{
			/* Failed, perhaps not a valid representation of IPV4? */
			printf("ERROR:  Please express '%s' as an IPV4 address."
                   "(i.e. '192.168.2.1').\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;
		}	
		
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_V4DST;
	}	
	
		
	/* Parse IPV6 addresses - source address */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_V6SRC))
	{
		struct in6_addr result;  /* Address struture to load up */

		/* Is this a valid IPV6 address? */
		if (inet_pton(AF_INET6, pcValue, &result) == 1) // success!
		{
			/* Successfully parsed string into "result" */
			memcpy((void *) &(pQuery->create.queryTuple.v6srcaddr), 
				(void *)&result, sizeof(pQuery->create.queryTuple.v6srcaddr));
		}
		else
		{
			/* Failed, perhaps not a valid representation of IPv6? */
			printf("ERROR:  Please express '%s' as an IPV6 address."
                   "(i.e. '2001:0db8:85a3:0000:0000:8a2e:0370:7334' or 'ff02::1').\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;
		}	
	
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_V6SRC;
	}
	
	
	/* Parse IPV6 addresses - destination address */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_V6DST))
	{	
		struct in6_addr result;

		/* Is this a valid IPV6 address? */
		if (inet_pton(AF_INET6, pcValue, &result) == 1) // success!
		{
			/* Successfully parsed string into "result" */
			memcpy((void *)pQuery->create.queryTuple.v6dstaddr, 
				(void *)&result, sizeof(pQuery->create.queryTuple.v6dstaddr));
		}
		else
		{
			/* Failed, perhaps not a valid representation of IPv6? */
			printf("ERROR:  Please express '%s' as an IPV6 address."
                   "(i.e. '2001:0db8:85a3:0000:0000:8a2e:0370:7334' or 'ff02::1').\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;
		}	
	
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_V6DST;
	}	
	
	/* Parse MAC addresses - source address */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_MACSRC))
	{
		unsigned int uiInput[ETH_ALEN];
		int i;
		
		/* Is address in colon delmited notation? */
		i = sscanf(pcValue, "%x:%x:%x:%x:%x:%x", 
			&uiInput[0], &uiInput[1], &uiInput[2], 
			&uiInput[3], &uiInput[4], &uiInput[5]);
		if (i != 6)
		{
			/* Nope. */
			printf("ERROR:  Please express '%s' in MAC address format "
                   "(i.e. '02:10:18:73:81:04').\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;			
		}
		
		/* Populate the tuple structure */
		for(i=0; i<ETH_ALEN; i++)
		{
			/* Set field value for each byte of MAC address */
			pQuery->create.queryTuple.macSA[i] = uiInput[i] & 0xFF;
		}
		
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_MACSRC;		
	}	
	
	
	/* Parse MAC addresses - destination address */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_MACDST))
	{
		unsigned int uiInput[ETH_ALEN];
		int i;
		
		/* Is address in colon delmited notation? */
		i = sscanf(pcValue, "%x:%x:%x:%x:%x:%x", 
			&uiInput[0], &uiInput[1], &uiInput[2], 
			&uiInput[3], &uiInput[4], &uiInput[5]);
		if (i != 6)
		{
			/* Nope. */
			printf("ERROR:  Please express '%s' in MAC address format "
                   "(i.e. '02:10:18:73:81:04').\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;			
		}
		
		/* Populate the tuple structure */
		for(i=0; i<ETH_ALEN; i++)
		{
			/* Set field value for each byte of MAC address */
			pQuery->create.queryTuple.macDA[i] = uiInput[i] & 0xFF;
		}
		
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_MACDST;
	}	

	/* Parse source PHY name */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_SRCPHY))
	{
		/* Populate the tuple structure - source PHY field value. */
		strncpy((char *)pQuery->create.queryTuple.srcphy, pcValue, FLWSTATS_PHY_LEN);
		
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_SRCPHY;
	}	
	
	/* Parse destination PHY name */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_DSTPHY))
	{
		/* Populate the tuple structure - destination PHY field value. */
		strncpy((char *)pQuery->create.queryTuple.dstphy, pcValue, FLWSTATS_PHY_LEN);
		
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_DSTPHY;
	}	
	
	/* Parse IP versions - source IP version */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_SRCIPVER))
	{
		/* Is the IP version IPV4? */
		if(!strcasecmp(pcValue, FLWSTATS_IPVER_V4_VALUENAME))
		{
			/* Yes - set field to IPV4 */
			pQuery->create.queryTuple.srcipver = FLWSTATS_IPVER_V4;
		}
		/* Is the IP version IPV6? */
		else if(!strcasecmp(pcValue, FLWSTATS_IPVER_V6_VALUENAME))
		{
			/* Yes - set field to IPV6 */
			pQuery->create.queryTuple.srcipver = FLWSTATS_IPVER_V6;
		}
		else
		{
			/* It wasn't either - throw an error */
			printf("ERROR:  Please express '%s' as '" 
				FLWSTATS_IPVER_V4_VALUENAME "' or '" 
				FLWSTATS_IPVER_V6_VALUENAME "'.\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;			
		}
				
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_SRCIPVER;
	}	
	
	/* Parse IP versions - destination IP version */
	if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_DSTIPVER))
	{
		/* Is the IP version IPV4? */
		if(!strcasecmp(pcValue, FLWSTATS_IPVER_V4_VALUENAME))
		{
			/* Yes - set field to IPV4 */
			pQuery->create.queryTuple.dstipver = FLWSTATS_IPVER_V4;
		}
		/* Is the IP version IPV6? */
		else if(!strcasecmp(pcValue, FLWSTATS_IPVER_V6_VALUENAME))
		{
			/* Yes - set field to IPV6 */
			pQuery->create.queryTuple.dstipver = FLWSTATS_IPVER_V6;
		}
		else
		{
			/* It wasn't either - throw an error */
			printf("ERROR:  Please express '%s' as '" 
				FLWSTATS_IPVER_V4_VALUENAME "' or '" 
				FLWSTATS_IPVER_V6_VALUENAME "'.\n", pcValue);
			return FLWSTATS_ERR_BAD_PARAMS;			
		}
				
		/* Set mask bit */
		pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_DSTIPVER;
	}	
	
    if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_INRXMACSRC))
    {
        unsigned int uiInput[ETH_ALEN];
        int i;

        /* Is address in colon delmited notation? */
        i = sscanf(pcValue, "%x:%x:%x:%x:%x:%x", 
            &uiInput[0], &uiInput[1], &uiInput[2], 
            &uiInput[3], &uiInput[4], &uiInput[5]);
        if (i != 6)
        {
            /* Nope. */
            printf("ERROR:  Please express '%s' in MAC address format "
                   "(i.e. '02:10:18:73:81:04').\n", pcValue);
            return FLWSTATS_ERR_BAD_PARAMS;			
        }

        /* Populate the tuple structure */
        for(i=0; i<ETH_ALEN; i++)
        {
            /* Set field value for each byte of MAC address */
            pQuery->create.queryTuple.inRxMacSA[i] = uiInput[i] & 0xFF;
        }

        /* Set mask bit */
        pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_INRXMACSRC;		
    }	


    /* Parse MAC addresses - destination address */
    if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_INRXMACDST))
    {
        unsigned int uiInput[ETH_ALEN];
        int i;

        /* Is address in colon delmited notation? */
        i = sscanf(pcValue, "%x:%x:%x:%x:%x:%x", 
            &uiInput[0], &uiInput[1], &uiInput[2], 
            &uiInput[3], &uiInput[4], &uiInput[5]);
        if (i != 6)
        {
            /* Nope. */
            printf("ERROR:  Please express '%s' in MAC address format "
                   "(i.e. '02:10:18:73:81:04').\n", pcValue);
            return FLWSTATS_ERR_BAD_PARAMS;			
        }

        /* Populate the tuple structure */
        for(i=0; i<ETH_ALEN; i++)
        {
            /* Set field value for each byte of MAC address */
            pQuery->create.queryTuple.inRxMacDA[i] = uiInput[i] & 0xFF;
        }

        /* Set mask bit */
        pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_INRXMACDST;
    }   
    	
    if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_INTXMACSRC))
    {
        unsigned int uiInput[ETH_ALEN];
        int i;

        /* Is address in colon delmited notation? */
        i = sscanf(pcValue, "%x:%x:%x:%x:%x:%x", 
            &uiInput[0], &uiInput[1], &uiInput[2], 
            &uiInput[3], &uiInput[4], &uiInput[5]);
        if (i != 6)
        {
            /* Nope. */
            printf("ERROR:  Please express '%s' in MAC address format "
                   "(i.e. '02:10:18:73:81:04').\n", pcValue);
            return FLWSTATS_ERR_BAD_PARAMS;			
        }

        /* Populate the tuple structure */
        for(i=0; i<ETH_ALEN; i++)
        {
            /* Set field value for each byte of MAC address */
            pQuery->create.queryTuple.inTxMacSA[i] = uiInput[i] & 0xFF;
        }

        /* Set mask bit */
        pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_INTXMACSRC;		
    }	


    /* Parse MAC addresses - destination address */
    if(!strcasecmp(pcField, FLWSTATS_FIELDNAME_INTXMACDST))
    {
        unsigned int uiInput[ETH_ALEN];
        int i;

        /* Is address in colon delmited notation? */
        i = sscanf(pcValue, "%x:%x:%x:%x:%x:%x", 
            &uiInput[0], &uiInput[1], &uiInput[2], 
            &uiInput[3], &uiInput[4], &uiInput[5]);
        if (i != 6)
        {
            /* Nope. */
            printf("ERROR:  Please express '%s' in MAC address format "
                   "(i.e. '02:10:18:73:81:04').\n", pcValue);
            return FLWSTATS_ERR_BAD_PARAMS;			
        }

        /* Populate the tuple structure */
        for(i=0; i<ETH_ALEN; i++)
        {
            /* Set field value for each byte of MAC address */
            pQuery->create.queryTuple.inTxMacDA[i] = uiInput[i] & 0xFF;
        }

        /* Set mask bit */
        pQuery->create.queryTuple.mask |= FLWSTATS_QUERYMASK_INTXMACDST;
    }   
    		/* Done parsing - did we match anything? */
	if(pQuery->create.queryTuple.mask == 0)
	{
		/* Nope. Flag an error. */
		printf("ERROR:  Please use a valid field name instead of '%s'\n", pcField);
		return FLWSTATS_ERR_BAD_PARAMS;	
	}
	
	return(0);
}
