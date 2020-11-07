#ifndef __FLWSTATS_H_INCLUDED__
#define __FLWSTATS_H_INCLUDED__

/*
*  Copyright 2011, Broadcom Corporation
*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
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

#if defined(__KERNEL__)                 /* Kernel space compilation         */
#include <linux/types.h>                /* LINUX ISO C99 7.18 Integer types */
#else                                   /* User space compilation           */
#include <stdint.h>                     /* C-Lib ISO C99 7.18 Integer types */
#endif
#include <linux/if_ether.h>
#include "bcmtypes.h"


/*
 *****************************************************************************
 * File Name : flwstats.h
 * Description of Flow Stats is CONFIDENTIAL and available ONLY in flwstats.h.
 *
  ****************************************************************************
 */


/*Parameter Tags indicating whether the parameter is an input, output, or input/output argument*/
#ifndef IN
#define IN
#endif /*IN*/

#ifndef OUT
#define OUT
#endif /*OUT*/

#ifndef INOUT
#define INOUT
#endif /*INOUT*/


/* Enums describing flowstat value fields for matching */
typedef enum {
	FLWSTATS_IPVER_V4,					/* IP Version 4 */
	FLWSTATS_IPVER_V6					/* IP Version 6 */
}FlwStatsQueryIpVer_t;


/* Strings matching enum value fields */
#define FLWSTATS_IPVER_V4_VALUENAME			"v4"	/* IP Version 4 */
#define FLWSTATS_IPVER_V6_VALUENAME			"v6"	/* IP Version 6 */


/* srcPhy and dstPhy length must be same as IFNAMSIZ
   in kernel/linux/include/linux/if.h */
#define FLWSTATS_PHY_LEN 16

typedef struct {
    uint8_t         inTxMacSA[ETH_ALEN];                /* Inner Source MAC(L2 GRE) */
#define FLWSTATS_QUERYMASK_INTXMACSRC		0x00080000	/* Source MAC mask(L2 GRE) */
    uint8_t         inTxMacDA[ETH_ALEN];                /* Inner Destination MAC(L2 GRE) */
#define FLWSTATS_QUERYMASK_INTXMACDST		0x00040000	/* Destination MAC mask(L2 GRE) */ 
    uint8_t         inRxMacSA[ETH_ALEN];                /* Inner Source MAC(L2 GRE) */
#define FLWSTATS_QUERYMASK_INRXMACSRC		0x00020000	/* Source MAC mask(L2 GRE) */
    uint8_t         inRxMacDA[ETH_ALEN];                /* Inner Destination MAC(L2 GRE) */
#define FLWSTATS_QUERYMASK_INRXMACDST		0x00010000	/* Destination MAC mask(L2 GRE) */ 
    uint32_t        v4rxdstaddr;                    /* IPV4  RX dst addr */
#define FLWSTATS_QUERYMASK_V4RXDST		0x00008000	/* IPV4  RX dst mask */
	FlwStatsQueryIpVer_t srcipver;					/* src ip version */
#define FLWSTATS_QUERYMASK_SRCIPVER		0x00004000	/* src ip version mask */
	FlwStatsQueryIpVer_t dstipver;					/* dst ip version */
#define FLWSTATS_QUERYMASK_DSTIPVER		0x00002000	/* dst ip version mask */
    uint32_t        v4srcaddr;                      /* IPV4  src addr */
#define FLWSTATS_QUERYMASK_V4SRC		0x00001000	/* IPV4  src mask */
    uint32_t        v4dstaddr;                      /* IPV4  dst addr */
#define FLWSTATS_QUERYMASK_V4DST		0x00000800	/* IPV4  dst mask */
    uint32_t        v6srcaddr[4];                   /* IPV6  src addr */
#define FLWSTATS_QUERYMASK_V6SRC		0x00000400	/* IPV6  src mask */
    uint32_t        v6dstaddr[4];                   /* IPV6  dst addr */
#define FLWSTATS_QUERYMASK_V6DST		0x00000200	/* IPV6  dst mask */
    uint16_t        l4srcport;                      /* L4    src port */
#define FLWSTATS_QUERYMASK_L4SRCPRT		0x00000100	/* L4    src mask */
    uint16_t        l4dstport;                      /* L4    dst port */
#define FLWSTATS_QUERYMASK_L4DSTPRT		0x00000080	/* L4    dst mask */
    uint8_t         ipproto;                        /* ip    protocol */
#define FLWSTATS_QUERYMASK_IPPROTO		0x00000040	/* ip protocol mask */
/* For flows with one VLAN, innervid must be filled in the query */
    uint16_t        innervid;                       /* Inner VLAN */
#define FLWSTATS_QUERYMASK_INVID		0x00000020	/* Inner VLAN mask */
    uint16_t        outervid;                       /* Outer VLAN */
#define FLWSTATS_QUERYMASK_OUTVID		0x00000010	/* Outer VLAN mask */
    uint8_t         macSA[ETH_ALEN];                /* Source MAC */
#define FLWSTATS_QUERYMASK_MACSRC		0x00000008	/* Source MAC mask */
    uint8_t         macDA[ETH_ALEN];                /* Destination MAC */
#define FLWSTATS_QUERYMASK_MACDST		0x00000004	/* Destination MAC mask */ 
    uint8_t         srcphy[FLWSTATS_PHY_LEN];       /* Source Phy e.g. "eth0"*/
#define FLWSTATS_QUERYMASK_SRCPHY		0x00000002	/* Source MAC mask */
    uint8_t         dstphy[FLWSTATS_PHY_LEN];       /* Destination Phy */
#define FLWSTATS_QUERYMASK_DSTPHY		0x00000001	/* Destination MAC mask */ 
    uint32_t        mask;                           /* Mask */
}FlwStatsQueryTuple_t;

/* Convenience bits for flwStatsQueryTuple_t.mask field */	

/* Field names for parsing command lines and dumping data */
#define FLWSTATS_FIELDNAME_V4RXDST		"rxdstv4"	/* IPV4  RX dst addr */
#define FLWSTATS_FIELDNAME_SRCIPVER		"srcipver"	/* src IP4/6 version */
#define FLWSTATS_FIELDNAME_DSTIPVER		"dstipver"	/* dst IP4/6 version */
#define FLWSTATS_FIELDNAME_V4SRC		"srcv4"		/* IPV4  src addr */
#define FLWSTATS_FIELDNAME_V4DST		"dstv4"		/* IPV4  dst addr */
#define FLWSTATS_FIELDNAME_V6SRC		"srcv6"		/* IPV6  src addr */
#define FLWSTATS_FIELDNAME_V6DST		"dstv6"		/* IPV6  dst addr */
#define FLWSTATS_FIELDNAME_L4SRCPRT		"srcport"	/* L4    src port */
#define FLWSTATS_FIELDNAME_L4DSTPRT		"dstport"	/* L4    dst port */
#define FLWSTATS_FIELDNAME_IPPROTO		"proto"		/* ip    protocol */
#define FLWSTATS_FIELDNAME_INVID		"invid"		/* Inner VLAN */
#define FLWSTATS_FIELDNAME_OUTVID		"outvid"	/* Outer VLAN */
#define FLWSTATS_FIELDNAME_MACSRC		"srcmac"	/* Source MAC */
#define FLWSTATS_FIELDNAME_MACDST		"dstmac"	/* Destination MAC */ 
#define FLWSTATS_FIELDNAME_SRCPHY		"srcphy"	/* Source PHY */
#define FLWSTATS_FIELDNAME_DSTPHY		"dstphy"	/* Destination PHY */ 
#define FLWSTATS_FIELDNAME_INRXMACSRC	"inrxsrcmac"	/* inner Source MAC (l2gre) */
#define FLWSTATS_FIELDNAME_INRXMACDST	"inrxdstmac"	/* inner Dest MAC (l2gre) */
#define FLWSTATS_FIELDNAME_INTXMACSRC	"intxsrcmac"	/* inner Source MAC (l2gre) */
#define FLWSTATS_FIELDNAME_INTXMACDST	"intxdstmac"	/* inner Dest MAC (l2gre) */


/* A macro to help parse passed field parameters for integer fields */
#define FILL_TUPLE_FIELD(P,FIELD,KEYTOKEN,MASK,VALUE) \
	if(!strcasecmp(pcField, KEYTOKEN))\
	{\
		P->create.queryTuple.FIELD = VALUE;  		\
		P->create.queryTuple.mask |= MASK;	\
	}

	
#if defined(__KERNEL__)                 /* Kernel space compilation */

/* A macro to dump field parameters for integer fields from kernel space */
#define DUMP_TUPLE_FIELD_INT(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
        if (MASK & (FLWSTATS_QUERYMASK_L4DSTPRT | \
                    FLWSTATS_QUERYMASK_L4SRCPRT | \
                    FLWSTATS_QUERYMASK_INVID | \
                    FLWSTATS_QUERYMASK_OUTVID)) \
		    printk(KEYTOKEN "=%d ", ntohs(P->FIELD) ); \
        else \
            printk(KEYTOKEN "=%d ", P->FIELD ); \
	}

/* A macro to dump field parameters for IPV4 fields from kernel space */
#define DUMP_TUPLE_FIELD_IPV4(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		printk(KEYTOKEN "=%pI4 ", (void *)&P->FIELD );\
	}

/* A macro to dump field parameters for IPV6 fields from kernel space */
#define DUMP_TUPLE_FIELD_IPV6(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		printk(KEYTOKEN "=%pI6c ", (void *)P->FIELD );\
	}

/* A macro to dump field parameters for MAC address fields from kernel space */
#define DUMP_TUPLE_FIELD_MAC(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		printk(KEYTOKEN "=%pM ", (void *)P->FIELD );\
	}
	
/* A macro to dump field parameters for string fields from kernel space */
#define DUMP_TUPLE_FIELD_STR(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		printk(KEYTOKEN "=%s ", \
			(P->FIELD) );\
	}
	
/* A macro to dump field parameters for IP version number enums
  (FlwStatsQueryIpVer_t) from kernel space */
#define DUMP_TUPLE_FIELD_ENUM_IPV(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		if (P->FIELD == FLWSTATS_IPVER_V4)\
		{\
			printk(KEYTOKEN "=" FLWSTATS_IPVER_V4_VALUENAME " ");\
		} else \
		{\
			printk(KEYTOKEN "=" FLWSTATS_IPVER_V6_VALUENAME " ");\
		}\
	}
	
#else                                   /* User space compilation */

/* A macro to dump field parameters for integer fields from user space */
#define DUMP_TUPLE_FIELD_INT(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
        if (MASK & (FLWSTATS_QUERYMASK_L4DSTPRT | \
                    FLWSTATS_QUERYMASK_L4SRCPRT | \
                    FLWSTATS_QUERYMASK_INVID | \
                    FLWSTATS_QUERYMASK_OUTVID)) \
		    printf(KEYTOKEN "=%d ", ntohs(P->FIELD) ); \
        else \
            printf(KEYTOKEN "=%d ", P->FIELD ); \
	}

/* A macro to dump field parameters for IPV4 fields from user space */
#define DUMP_TUPLE_FIELD_IPV4(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		char pcInetAddr[80]; \
		inet_ntop(AF_INET, (void *)&P->FIELD, \
			   pcInetAddr, sizeof(pcInetAddr)); \
		printf(KEYTOKEN "=%s\n", pcInetAddr); \
	}

/* A macro to dump field parameters for IPV6 fields from user space */
#define DUMP_TUPLE_FIELD_IPV6(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		char pcInetAddr[80]; \
		inet_ntop(AF_INET6, (void *)P->FIELD, \
			   pcInetAddr, sizeof(pcInetAddr)); \
		printf(KEYTOKEN "=%s\n", pcInetAddr); \
	}

/* A macro to dump field parameters for MAC address fields from user space */
#define DUMP_TUPLE_FIELD_MAC(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		printf(KEYTOKEN "=%02x:%02x:%02x:%02x:%02x:%02x ", \
			(P->FIELD[0]), (P->FIELD[1]), (P->FIELD[2]), \
			(P->FIELD[3]), (P->FIELD[4]), (P->FIELD[5]) );\
	}
	
/* A macro to dump field parameters for string fields from user space */
#define DUMP_TUPLE_FIELD_STR(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		printf(KEYTOKEN "=%s ", \
			(P->FIELD) );\
	}
	
/* A macro to dump field parameters for IP version number enums
  (FlwStatsQueryIpVer_t) from user space. */
#define DUMP_TUPLE_FIELD_ENUM_IPV(P,FIELD,KEYTOKEN,MASK) \
	if(P->mask & MASK)\
	{\
		if (P->FIELD == FLWSTATS_IPVER_V4)\
		{\
			printf(KEYTOKEN "=" FLWSTATS_IPVER_V4_VALUENAME " ");\
		} else \
		{\
			printf(KEYTOKEN "=" FLWSTATS_IPVER_V6_VALUENAME " ");\
		}\
	}
	
#endif

typedef struct {
    uint32_t rx_packets;
    int32_t  rx_rtp_packets_lost;
    uint64_aligned rx_bytes;
}FlwStats_t;


typedef struct {
    IN  FlwStatsQueryTuple_t  queryTuple;
    OUT uint32_t              handle;
}FlwStatsCreateQueryInfo_t;

typedef struct {
    IN  uint32_t              handle;
    OUT FlwStats_t            flwSt;
}FlwStatsGetQueryInfo_t;

typedef struct {
    IN  uint32_t              handle;
}FlwStatsClearQueryInfo_t;

typedef struct {
    IN  uint32_t              handle;
}FlwStatsDeleteQueryInfo_t;

typedef union {
    FlwStatsCreateQueryInfo_t  create;
    FlwStatsGetQueryInfo_t     get;
    FlwStatsClearQueryInfo_t   clear; 
    FlwStatsDeleteQueryInfo_t  delete;
}FlwStatsQueryInfo_t;

typedef struct {
    IN  uint8_t              queryIdx;
    OUT  uint8_t               nodeFlag;
    OUT  FlwStats_t            flwSt;
    OUT  FlwStatsQueryTuple_t  queryTuple;    
}FlwStatsDumpEntryInfo_t;

#define PREV_FLW_STATS_DUMP_ENTRIES  25

typedef struct {
    uint32_t	num_entries;
    FlwStats_t		prevFlwStEntries[PREV_FLW_STATS_DUMP_ENTRIES];
    BCM_IOC_PTR(FlwStatsDumpEntryInfo_t*, FlwStDumpEntry);
}FlwStatsDumpInfo_t;

typedef struct {
    uint32_t intval;       /* Flow statistics polling interval in milliseconds */
    uint32_t granularity;  /* (max_flows/granularity) flows are checked on every poll  */
}FlwStatsPollParams_t;

/*
 * Portions of user space IOCTL API that are needed in kernel as well
 */

#define MAX_FLW_STATS_QUERIES  128

 /* Error codes */

/* Indicates query identified was not found. */
#define FLWSTATS_ERR_QUERY_NOT_FOUND	-1		

/* Indicates an illegal parameter passed into routine. */
#define FLWSTATS_ERR_BAD_PARAMS 		-2	
	
/* Attempt to create more queries than supported. */	
#define FLWSTATS_ERR_NO_QUERIES_LEFT	-3		

/* Protocol not supported. */
#define FLWSTATS_ERR_NO_PROTO_SUPPORT   -4

/* API Parameter Macros */
/* Indicates action should be taken on all queries */
#define ALL_STATS_QUERIES_HANDLE		MAX_FLW_STATS_QUERIES		

/* Supported Protocols in Queries */
#define FLWSTATS_PROTO_TCP  6
#define FLWSTATS_PROTO_UDP  17

/*
 *---------------------------------------------------------------------------
 * Function Name: flwStatsCreateQuery
 * Description  : Creates a statistics query for later use.
 * Parameters   :
 *  newQuery    : Pointer to an  FlwStatsQueryInfo_t structure where 
 *                the FlwStatsQueryTuple_t portion will be used to set
 *                up the query. 
 *                A unique handle would be returned for later use.
 * Returns      : 0 - success, non-0 - error
 *-----------------------------------------------------------------------------
 */
 
int flwStatsCreateQuery(FlwStatsQueryInfo_t *newQuery);

/*
 *-----------------------------------------------------------------------------
 * Function Name: flwStatsGetQuery
 * Description  : Retrieves statistics for a query previously set up by a 
 *                call to flwStatsCreateQuery().
 * Parameters   :
 *  Query       : Handle returned by earlier flwStatsCreateQuery() is
 *                passed as input. 
 *                Results are stored in flwSt field in FlwStatsQueryInfo_t.
 * Returns      : 0 - success, non-0 - error
 *-----------------------------------------------------------------------------
 */
int flwStatsGetQuery(FlwStatsQueryInfo_t *Query);

/*
 *-----------------------------------------------------------------------------
 * Function Name: flwStatsClearQuery
 * Description  : Zeroes out the counters for one or all queries previously  
 *                set up by a call to flwStatsCreateQuery().
 
 * Parameters   :
 *  Query :       Handle returned by earlier flwStatsCreateQuery() is
 *                passed as input. 
 *                The special value
 *                ALL_STATS_QUERIES_HANDLE may be used to clear the counters for all queries.
 * Returns      : 0 - success, non-0 - error
 *-----------------------------------------------------------------------------
 */

int flwStatsClearQuery(FlwStatsQueryInfo_t *Query);

/*
 *-----------------------------------------------------------------------------
 * Function Name: flwStatsDeleteQuery
 * Description  : Deletes one or all queries previously set up by a call  
 *                to flwStatsCreateQuery().
 * Parameters   :
 *  Query : Handle returned by earlier flwStatsCreateQuery() is
 *                passed as input. 
 *                The special value ALL_STATS_QUERIES_HANDLE may be used to delete all queries.
 * Returns      : 0 - success, non-0 - error
 *-----------------------------------------------------------------------------
 */
int flwStatsDeleteQuery(FlwStatsQueryInfo_t *Query);

#endif  /* defined(__FLWSTATS_H_INCLUDED__) */
