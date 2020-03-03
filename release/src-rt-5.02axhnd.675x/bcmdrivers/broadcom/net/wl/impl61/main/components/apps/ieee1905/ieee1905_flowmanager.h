/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
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
 *
 * $Change: 111969 $
 ***********************************************************************/

#ifndef _IEEE1905_FLOWMANAGER_H_
#define _IEEE1905_FLOWMANAGER_H_

#if defined(SUPPORT_IEEE1905_FM)
#if defined(SUPPORT_FBCTL)
#include <fbctl_api.h>
#endif // endif
#if defined(SUPPORT_FCCTL)
#include <fcctl_api.h>
#endif // endif

#include "ieee1905_datamodel_priv.h"

void i5FlowManagerActivateInterface( i5_socket_type *pif );
void i5FlowManagerDeactivateInterface( i5_socket_type *pif );
void i5FlowManagerCheckNeighborForOverload(i5_dm_1905_neighbor_type *neighbor);
void i5FlowManagerProcessNewNeighbor(i5_dm_1905_neighbor_type *neighbor);
void i5FlowManagerProcessNeighborRemoved(i5_dm_1905_neighbor_type *newNeighbor);
void i5FlowManagerMetricUpdate(int ifindex, unsigned short totalBandwidthMbps, unsigned short availBandwidthMbps);
void i5FlowManagerAddConnectionIndex(unsigned char const *neighborId, unsigned int ifindex);
void i5FlowManagerRemoveConnectionIndex(unsigned char const *neighborId, unsigned int ifindex);
#if defined(SUPPORT_IEEE1905_AUTO_WDS)
void i5FlowManagerProcessLocalWirelessDown(void);
void i5FlowManagerProcessWirelessUp(void);
#endif // endif
void i5FlowManagerShow(void);
void i5FlowManagerDeinit(void);

#endif /* defined(SUPPORT_IEEE1905_FM) */

#endif // endif
