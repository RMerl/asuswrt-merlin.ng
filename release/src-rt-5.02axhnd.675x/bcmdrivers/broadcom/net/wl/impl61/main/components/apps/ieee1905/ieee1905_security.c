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
 * $Change: 116460 $
 ***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "ieee1905_udpsocket.h"
#include "ieee1905_timer.h"
#include "ieee1905_socket.h"
#include "ieee1905_trace.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_plc.h"

#define I5_TRACE_MODULE i5TraceSecurity

#define I5_SECURITY_STATUS_CHECK_INTERVAL_MS   2000
#define I5_SECURITY_STATUS_TIMEOUT_INTERVAL_MS 125000

static void _i5SecurityStatusCheck(void *arg)
{
  secStatusStruct      *pSecTmr = (secStatusStruct * )arg;
  i5_dm_device_type    *pDmDevice;
  i5_dm_interface_type *pDmInterface;
  unsigned int          securityStatus = 0;

  i5Trace("\n");

  if (pSecTmr->timer) {
    i5TimerFree(pSecTmr->timer);
    pSecTmr->timer = NULL;
  }

  pDmDevice = i5DmGetSelfDevice();
  if ( NULL == pDmDevice ) {
    return;
  }

  pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
  while (pDmInterface != NULL) {
    if ( i5DmIsInterfaceWireless(pDmInterface->MediaType) ) {
      pDmInterface->SecurityStatus = 0;
    }

    if ( i5DmIsInterfacePlc(pDmInterface->MediaType, pDmInterface->netTechOui) ) {
      if ( pSecTmr->count >= (I5_SECURITY_STATUS_TIMEOUT_INTERVAL_MS / I5_SECURITY_STATUS_CHECK_INTERVAL_MS)) {
        pDmInterface->SecurityStatus = 0;
      }
      securityStatus |= pDmInterface->SecurityStatus;
    }

    pDmInterface = (i5_dm_interface_type *)pDmInterface->ll.next;
  }

  if ( securityStatus ) {
    pSecTmr->timer = i5TimerNew(I5_SECURITY_STATUS_CHECK_INTERVAL_MS, _i5SecurityStatusCheck, pSecTmr);
    pSecTmr->count++;
  }
}

#ifdef SUPPORT_HOMEPLUG
static int _i5SecurityHaveLowestMac( unsigned short mediaType )
{
  i5_dm_device_type    *pDmDevice;
  i5_dm_interface_type *pDmInterface;

  i5Trace("\n");

  pDmDevice = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  while (pDmDevice != NULL) {
    if ( memcmp(pDmDevice->DeviceId, i5_config.i5_mac_address, MAC_ADDR_LEN) ) {
      pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
      while (pDmInterface != NULL) {
        if ( pDmInterface->MediaType == mediaType ) {
          int cmpRes = memcmp(pDmDevice->DeviceId, i5_config.i5_mac_address, MAC_ADDR_LEN);
          if ( cmpRes < 0 ) {
            i5Trace("Not lowest MAC for mediaType %x\n", mediaType);
            return 0;
          }
          break;
        }
        pDmInterface = pDmInterface->ll.next;
      }
    }
    pDmDevice = pDmDevice->ll.next;
  }
  i5Trace("Have lowest MAC for mediaType %x\n", mediaType);
  return 1;
}
#endif // endif

void i5SecurityInit( void )
{
#if defined(IEEE1905_KERNEL_MODULE_PB_SUPPORT)
  i5UdpSocketSendPushButtonRegistration( 1 );
#endif // endif
}

void i5SecurityProcessExternalPushButtonEvent( unsigned int mediaCount, unsigned short *pMediaList )
{
  i5_dm_device_type    *pDmDevice;
  i5_dm_interface_type *pDmInterface;

  i5Trace("\n");

  pDmDevice = i5DmGetSelfDevice();
  if ( NULL == pDmDevice ) {
    return;
  }

  pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
  while (pDmInterface != NULL) {
#if defined(WIRELESS)
    unsigned int          i;
    if ( i5DmIsInterfaceWireless(pDmInterface->MediaType) ) {
      /* TBD: if ( ap configured ) then start wps */
      for( i = 0; i < mediaCount; i++) {
        if (pDmInterface->MediaType == pMediaList[i]) {
          break;
        }
      }
      if ( i == mediaCount ) {
        i5Trace("starting WPS for mediaType %d\n", pDmInterface->MediaType);
        pDmInterface->SecurityStatus = 1;
#if defined(IEEE1905_KERNEL_MODULE_PB_SUPPORT)
        i5UdpSocketTriggerWirelessPushButtonEvent(0);
#endif // endif
      }
    }
#endif // endif

#ifdef SUPPORT_HOMEPLUG
    if ( i5DmIsInterfacePlc(pDmInterface->MediaType, pDmInterface->netTechOui) ) {
      unsigned int          i;
      for( i = 0; i < mediaCount; i++) {
        if (pDmInterface->MediaType == pMediaList[i]) {
          break;
        }
      }
      if ( i == mediaCount ) {
        if ( _i5SecurityHaveLowestMac(pDmInterface->MediaType) ) {
          i5Trace("starting UKE for mediaType %d\n", pDmInterface->MediaType);
          pDmInterface->SecurityStatus = 1;
          i5PlcUKEStart( );
        }
      }
    }
#endif // endif
    pDmInterface = pDmInterface->ll.next;
  }

  if (NULL == i5_config.ptmrSecStatus.timer) {
    i5_config.ptmrSecStatus.timer = i5TimerNew(I5_SECURITY_STATUS_CHECK_INTERVAL_MS, _i5SecurityStatusCheck, &i5_config.ptmrSecStatus);
    i5_config.ptmrSecStatus.count = 0;
  }

}

void i5SecurityProcessGenericPhyExternalPushButtonEvent( unsigned int mediaCount, unsigned char *pMediaList )
{
  i5_dm_device_type    *pDmDevice;
  i5_dm_interface_type *pDmInterface;

  i5Trace("\n");

  pDmDevice = i5DmGetSelfDevice();
  if ( NULL == pDmDevice ) {
    return;
  }

  pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
  while (pDmInterface != NULL) {

#ifdef SUPPORT_HOMEPLUG
    if ( i5DmIsInterfacePlc(pDmInterface->MediaType, pDmInterface->netTechOui) ) {
      unsigned int i;
      for( i = 0; i < mediaCount; i++) {
        if ( i5DmIsInterfacePlc(I5_MEDIA_TYPE_UNKNOWN, &pMediaList[4*i]) &&
             (pMediaList[4*i]+3 == pDmInterface->netTechVariant) ) {
          break;
        }
      }
      if ( i == mediaCount ) {
        if ( _i5SecurityHaveLowestMac(pDmInterface->MediaType) ) {
          i5Trace("starting UKE for mediaType %d\n", pDmInterface->MediaType);
          pDmInterface->SecurityStatus = 1;
          i5PlcUKEStart( );
        }
      }
    }
#endif // endif
    pDmInterface = pDmInterface->ll.next;
  }

  if (NULL == i5_config.ptmrSecStatus.timer) {
    i5_config.ptmrSecStatus.timer = i5TimerNew(I5_SECURITY_STATUS_CHECK_INTERVAL_MS, _i5SecurityStatusCheck, &i5_config.ptmrSecStatus);
    i5_config.ptmrSecStatus.count = 0;
  }

}

void i5SecurityProcessLocalPushButtonEvent( void )
{
  i5_dm_device_type    *pDmDevice;
  i5_dm_interface_type *pDmInterface;
  unsigned int          securityStarted = 0;

  pDmDevice = i5DmGetSelfDevice();
  if ( NULL == pDmDevice ) {
    return;
  }

  pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
  while (pDmInterface != NULL) {
    if ( i5DmIsInterfaceWireless(pDmInterface->MediaType) ) {
      securityStarted |= pDmInterface->SecurityStatus;
      i5Trace("Is Wifi securityStarted? %d\n", pDmInterface->SecurityStatus);
    }
    if ( i5DmIsInterfacePlc(pDmInterface->MediaType, pDmInterface->netTechOui) ) {
      securityStarted |= pDmInterface->SecurityStatus;
      i5Trace("Is Plc securityStarted? %d\n", pDmInterface->SecurityStatus);
    }
    pDmInterface = pDmInterface->ll.next;
  }

  if ( securityStarted ) {
    return;
  }

  pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
  while (pDmInterface != NULL) {
#if defined(IEEE1905_KERNEL_MODULE_PB_SUPPORT)
    if ( i5DmIsInterfaceWireless(pDmInterface->MediaType) ) {
      /* TBD: if ( ap configured ) then start wps */
      i5Trace("starting WPS for mediaType %d\n", pDmInterface->MediaType);
      pDmInterface->SecurityStatus = 1;

      i5UdpSocketTriggerWirelessPushButtonEvent(0);
    }
#endif // endif

#ifdef SUPPORT_HOMEPLUG
    if ( i5DmIsInterfacePlc(pDmInterface->MediaType, pDmInterface->netTechOui) ) {
      i5Trace("starting UKE for mediaType %d\n", pDmInterface->MediaType);
      pDmInterface->SecurityStatus = 1;
      i5PlcUKEStart( );
    }
#endif // endif
    pDmInterface = pDmInterface->ll.next;
  }

  if (NULL == i5_config.ptmrSecStatus.timer) {
    i5_config.ptmrSecStatus.timer = i5TimerNew(I5_SECURITY_STATUS_CHECK_INTERVAL_MS, _i5SecurityStatusCheck, &i5_config.ptmrSecStatus);
    i5_config.ptmrSecStatus.count = 0;
  }

  i5MessagePushButtonEventNotificationSend();

}

void i5SecuritySesCompleteNotify( void )
{
  i5_dm_device_type    *pDmDevice;
  i5_dm_interface_type *pDmInterface;

  i5Trace("\n");

  pDmDevice = i5DmGetSelfDevice();
  if ( NULL == pDmDevice ) {
    return;
  }

  pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
  while (pDmInterface != NULL) {
    if ( i5DmIsInterfaceWireless(pDmInterface->MediaType) ) {
      pDmInterface->SecurityStatus = 0;
    }
    pDmInterface = pDmInterface->ll.next;
  }
}

void i5SecurityStatusUpdate(unsigned short mediaType)
{
  i5_dm_device_type    *pDmDevice;
  i5_dm_interface_type *pDmInterface;

  i5Trace("\n");

  pDmDevice = i5DmGetSelfDevice();
  if ( NULL == pDmDevice ) {
    return;
  }

  pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
  while (pDmInterface != NULL) {
    if ( (I5_MEDIA_TYPE_WIFI_B == mediaType) &&
         i5DmIsInterfaceWireless(pDmInterface->MediaType) ) {
      pDmInterface->SecurityStatus = 0;
      i5Trace("clear sec status for Wifi\n");
    }

    if ( (I5_MEDIA_TYPE_1901_WAVELET == mediaType) &&
         i5DmIsInterfacePlc(pDmInterface->MediaType, pDmInterface->netTechOui) ) {
      pDmInterface->SecurityStatus = 0;
      i5Trace("clear sec status for 1901\n");
    }

    pDmInterface = pDmInterface->ll.next;
  }
}
