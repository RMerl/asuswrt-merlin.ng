#ifndef _MCAST_HAL_API_H_
#define _MCAST_HAL_API_H_
/***********************************************************************
 *
 *  Copyright (c) 2007-2010  Broadcom Corporation
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

/*******************************************************************************
 * File Name  : mcasthal_api.h
 *
 * Description: This file contains mcast IGMP snooping API proto types for HAL
 *
 ******************************************************************************/
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>

typedef struct mcast_hal_snoop_entry 
{
   char                      br_name[IFNAMSIZ]; /* bridge interface name */
   int                       port_no;      /* port number of bridge interface */
   struct                    in_addr grp;  /* mcast group address */
   struct                    in_addr src;  /* mcast source address */
   struct                    in_addr rep;  /* mcast receiver/client address */
   int                       filter_mode;  /* filter mode: MCAST_EXCLUDE or 
                                                       MCAST_INCLUDE from in.h */
} MCAST_HAL_SNOOP_ENTRY_t;

/******************************************************************************
 *
 * Function: mcastHal_addFlow 
 *                                                                             
 * Description: API to add a igmp snooping flow in the HAL/hardware 
 *
 * INPUT  -  *snp_entry MCAST_HAL_SNOOP_ENTRY_t entry
 *
 * Returns -1 on error 0 on success
 *
 ******************************************************************************/
int mcastHal_addFlow(MCAST_HAL_SNOOP_ENTRY_t *snp_entry);

 
/******************************************************************************
 *
 * Function: mcastHal_deleleFlow
 *                                                                             
 * Description: API to remove a igmp snooping flow in the HAL/hardware 
 *
 * INPUT  -  *snp_entry MCAST_HAL_SNOOP_ENTRY_t entry
 *
 * Returns -1 on error 0 on success
 ******************************************************************************/
int mcastHal_deleleFlow(MCAST_HAL_SNOOP_ENTRY_t *snp_entry);


#endif /* _MCAST_HAL_API_H_*/
