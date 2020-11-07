/*
* <:copyright-BRCM:2007:proprietary:standard 
* 
*    Copyright (c) 2007 Broadcom 
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

#ifndef BCM_OMCI_API_COMMON_H
#define BCM_OMCI_API_COMMON_H

#include "bcm_gpon_api_common.h"

/**
 * OMCI Driver user API
 **/

#define BCM_OMCI_DEVICE_NAME "bcm_omci"

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

#define BCM_OMCI_RX_MSG_MAX_SIZE_BYTES 2040
#define BCM_OMCI_TX_MSG_MAX_SIZE_BYTES 2040

/**
 * OMCI Driver ioctl command IDs
 **/

/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
#define BCM_OMCI_IOC_GET_COUNTERS         100  /*Return codes: 0*/
#define BCM_OMCI_IOC_GET_DRIVER_VERSION   101  /*Return codes: 0*/
#define BCM_OMCI_IOC_CFG_MIRROR           102  /*Return codes: 0*/

/*Type of ioctl argument pointer for command BCM_PLOAM_IOC_GET_COUNTERS*/
typedef struct {
  IN UINT32 reset;
  OUT UINT32 rxBytes;
  OUT UINT32 rxFragments;
  OUT UINT32 rxFragmentsDropped;
  OUT UINT32 rxMessagesTotal;
  OUT UINT32 rxMessagesDiscarded;
  OUT UINT32 txBytes;
  OUT UINT32 txFragments;
  OUT UINT32 txMessages;
  OUT UINT32 rxBaseLinePackets;
  OUT UINT32 rxExtendedPackets;
  OUT UINT32 rxMicCrcErrCnt;
} BCM_Omci_Counters;

typedef struct {
  IN UBOOL8 enable;
  IN UINT8  portIndex;
} BCM_Omci_PortMirror;


#endif /*BCM_OMCI_API_COMMON_H*/
