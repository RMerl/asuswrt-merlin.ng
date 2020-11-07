#ifndef __FCACHEDRV_CONFIG_H_INCLUDED__
#define __FCACHEDRV_CONFIG_H_INCLUDED__
/*
<:copyright-BRCM:2019:proprietary:standard

   Copyright (c) 2019 Broadcom 
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
 * File Name  : fcachedrv_config.h
 *
 * Description: This file contains fcache driver configuration
 *******************************************************************************
 */

/*----- Includes -----*/


/*----- Defines -----*/

/* Flow cache configured Max number of Flow Entries */
#define FCACHE_CONFIG_MAX_FLOW_ENTRIES     (CONFIG_BCM_PKTFLOW_MAX_FLOWS)

/* Define high flow system if more than 4k flow entries */
#if FCACHE_CONFIG_MAX_FLOW_ENTRIES > (4*1024)
#define CC_FCACHE_HIGH_FLOW_SYS
#endif

/* Flow cache configured Max number of FDB Entries */
#define FCACHE_CONFIG_MAX_FDB_ENTRIES     (CONFIG_BCM_PKTFLOW_MAX_FDB)

/* Flow cache configured Max number of Host Devices */
#define FCACHE_CONFIG_MAX_HOST_DEV_ENTRIES     (CONFIG_BCM_PKTFLOW_MAX_HOST_DEV)

/* Flow cache configured Max number of Host MAC */
#define FCACHE_CONFIG_MAX_HOST_MAC_ENTRIES     (CONFIG_BCM_PKTFLOW_MAX_HOST_MAC)

/* Flow cache system periodic timer */
#if defined(CC_FCACHE_HIGH_FLOW_SYS)
#define FCACHE_REFRESH_MSEC                 ( 10000 )      /* Poll timer interval   */
#define FCACHE_REFRESH_MIN_MSEC             ( 5000 )       /* Min Poll timer interval*/
#else
#define FCACHE_REFRESH_MSEC                 ( 1000 )       /* Poll timer interval   */
#define FCACHE_REFRESH_MIN_MSEC             ( 1000 )       /* Min Poll timer interval*/
#endif
#define FCACHE_REFRESH_MAX_MSEC             ( 0xFFFFFF )   /* Max Poll timer interval - some value for boundary check */

#define FCACHE_SLICE_PERIOD_MIN_MSEC    (5) /* Minimum Slice period in msec */
#define FCACHE_SLICE_MIN_ENT            (8) /* Minimium Entries per slice */
#if defined(CC_FCACHE_HIGH_FLOW_SYS)
#define FCACHE_SLICE_MAX_ENT            (CONFIG_BCM_PKTFLOW_MAX_FLOWS>>9) /* Maximum entries per slice */
#else
#define FCACHE_SLICE_MAX_ENT            (32)/* Maximum entries per slice */
#endif


#endif /* defined(__FCACHEDRV_CONFIG_H_INCLUDED__) */
