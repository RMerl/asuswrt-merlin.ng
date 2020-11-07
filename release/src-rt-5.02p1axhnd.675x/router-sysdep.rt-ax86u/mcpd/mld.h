/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
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
 * File Name  : mld.h
 *
 * Description: 
 *              
 ***************************************************************************/
#ifndef __MLD_H__
#define __MLD_H__
#include <netinet/icmp6.h>

/******************************************************************************
*                      Definitions
*******************************************************************************/

#ifndef IN6_IS_ADDR_UNSPECIFIED
#define IN6_IS_ADDR_UNSPECIFIED(a) \
	(((__const UINT32 *) (a))[0] == 0				      \
	 && ((__const UINT32 *) (a))[1] == 0				      \
	 && ((__const UINT32 *) (a))[2] == 0				      \
	 && ((__const UINT32 *) (a))[3] == 0)
#endif

#ifndef IN6_IS_ADDR_LOOPBACK
#define IN6_IS_ADDR_LOOPBACK(a) \
	(((__const UINT32 *) (a))[0] == 0				      \
	 && ((__const UINT32 *) (a))[1] == 0				      \
	 && ((__const UINT32 *) (a))[2] == 0				      \
	 && ((__const UINT32 *) (a))[3] == htonl (1))
#endif

#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(a) (((__const UINT8 *) (a))[0] == 0xff)
#endif

#ifndef IN6_IS_ADDR_LINKLOCAL
#define IN6_IS_ADDR_LINKLOCAL(a) \
	((((__const UINT32 *) (a))[0] & htonl (0xffc00000))		      \
	 == htonl (0xfe800000))
#endif

#ifndef IN6_IS_ADDR_SITELOCAL
#define IN6_IS_ADDR_SITELOCAL(a) \
	((((__const UINT32 *) (a))[0] & htonl (0xffc00000))		      \
	 == htonl (0xfec00000))
#endif

#ifndef IN6_IS_ADDR_V4MAPPED
#define IN6_IS_ADDR_V4MAPPED(a) \
	((((__const UINT32 *) (a))[0] == 0)				      \
	 && (((__const UINT32 *) (a))[1] == 0)			      \
	 && (((__const UINT32 *) (a))[2] == htonl (0xffff)))
#endif

#ifndef IN6_ARE_ADDR_EQUAL
#define IN6_ARE_ADDR_EQUAL(a,b) \
	((((__const UINT32 *) (a))[0] == ((__const UINT32 *) (b))[0])     \
	 && (((__const UINT32 *) (a))[1] == ((__const UINT32 *) (b))[1])  \
	 && (((__const UINT32 *) (a))[2] == ((__const UINT32 *) (b))[2])  \
	 && (((__const UINT32 *) (a))[3] == ((__const UINT32 *) (b))[3]))
#endif

#define IN6_ASSIGN_ADDR(a,b)                                  \
    do {                                                      \
        ((UINT32 *) (a))[0] = ((__const UINT32 *) (b))[0];    \
        ((UINT32 *) (a))[1] = ((__const UINT32 *) (b))[1];    \
        ((UINT32 *) (a))[2] = ((__const UINT32 *) (b))[2];    \
        ((UINT32 *) (a))[3] = ((__const UINT32 *) (b))[3];    \
    } while(0)

#ifndef IN6_IS_ADDR_MC_NODELOCAL
#define IN6_IS_ADDR_MC_NODELOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)   \
	 && ((((__const UINT8 *) (a))[1] & 0xf) == 0x1))
#endif

#ifndef IN6_IS_ADDR_MC_LINKLOCAL
#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)   \
	 && ((((__const UINT8 *) (a))[1] & 0xf) == 0x2))
#endif

#ifndef IN6_IS_ADDR_MC_SITELOCAL
#define IN6_IS_ADDR_MC_SITELOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)   \
	 && ((((__const UINT8 *) (a))[1] & 0xf) == 0x5))
#endif

#ifndef IN6_IS_ADDR_MC_ORGLOCAL
#define IN6_IS_ADDR_MC_ORGLOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)  \
	 && ((((__const UINT8 *) (a))[1] & 0xf) == 0x8))
#endif

#ifndef IN6_IS_ADDR_MC_GLOBAL
#define IN6_IS_ADDR_MC_GLOBAL(a) \
	(IN6_IS_ADDR_MULTICAST(a) \
	 && ((((__const UINT8 *) (a))[1] & 0xf) == 0xe))
#endif

#ifndef IN6_IS_ADDR_MC_SCOPE0
#define IN6_IS_ADDR_MC_SCOPE0(a) \
       (IN6_IS_ADDR_MULTICAST(a) \
    && ((((__const UINT8 *) (a))[1] & 0xf) == 0x0))
#endif

#define MLDV1_QUERY_SIZE            24 /* MLDv1 query size */
#define MLDV2_QUERY_SIZE            28 /* Base MLDv2 query size */

/* Node Local Scope */
#define MLD_NL_ALL_NODES            "ff01::1" /* All Nodes in network */
#define MLD_NL_ALL_ROUTERS          "ff01::2" /* All routers in network */

/* Link Local Scope */
#define MLD_LL_ALL_HOSTS            "ff02::1" /* All Nodes in network */
#define MLD_LL_V1_ALL_ROUTERS       "ff02::2" /* All routers in network */
#define MLD_LL_V2_ALL_ROUTERS       "ff02::16" /* All MLDv2 Routers */

/* Site Local Scope */
#define MLD_SL_ALL_ROUTERS          "ff05::2" /* All routers in network */

/* MLD timer definisions */
#define MLD_TIMER_GROUP_DELAY_MSEC  4000

#endif /* __MLD_H__ */
