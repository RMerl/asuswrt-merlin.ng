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

/*
 * IEEE1905 PLC
 */
#if defined(SUPPORT_HOMEPLUG)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include "ieee1905_timer.h"
#include "ieee1905_message.h"
#include "ieee1905_socket.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_interface.h"
#include "ieee1905_trace.h"
#include "ieee1905_tlv.h"
#if defined(BRCM_CMS_BUILD)
#include "ieee1905_cmsutil.h"
#endif // endif
#include "ieee1905_security.h"
#include "ieee1905_interface.h"
#include "ieee1905_flowmanager.h"
#include "ieee1905_plc_fob.h"
#include "i5api.h"
#include "homeplugd.h"

#define I5_TRACE_MODULE   i5TracePlc
#define I5_PLC_BANDWIDTH_UPDATE_INTERVAL_MSEC 1000
#define I5_PLC_UNIX_CONNECT_TIMEOUT_MSEC 2000

static unsigned char plcBandwidthOverridesLeft = 0;
static unsigned int plcOverrideAvailBandwidthMbps = 0;
static unsigned int plcOverrideMacThroughputMbps = 0;

static void i5PlcNotifyReceive(i5_socket_type *psock);

static inline unsigned char *_i5PlcReadMac(unsigned char *mac, unsigned char *p)
{
  memcpy(mac, p, ETH_ALEN);
  return mac;
}

static inline unsigned char _i5PlcReadUnsignedChar(unsigned char *p)
{
  unsigned char * number;

  number = (unsigned char *) p;
  return *number;
}

static inline unsigned short int _i5PlcReadUnsignedShort(unsigned char *p)
{
  unsigned short int * number;

  number = (unsigned short int *) p;
  return ntohs(*number);
}

int i5PlcControlSockReady( void )
{
  if ( NULL == i5_config.plc_control_socket.ptmr ) {
    return 1;
  }
  else {
    return 0;
  }
}

int i5PlcGetDevMacAddress( unsigned char *macAddr )
{
  if ( i5_config.plc_control_socket.macAddrSet ) {
    memcpy(macAddr, &i5_config.plc_control_socket.plcDevMac[0], MAC_ADDR_LEN);
    return 0;
  }
  else {
    return -1;
  }
}

/* Debug function to override Available and Max Throughput bandwidths
 *
 * avail - in Mbps
 * maxThrough - in Mbps
 * numOverrides - 0 - turn off override
 *              - 255 - forever (or until turned off)
 *              - [1,254] - that many times
 */
void i5PlcLinkMetricsOverrideBandwidth (unsigned int avail, unsigned int macThrough, unsigned char numOverrides)
{
  plcBandwidthOverridesLeft = numOverrides;
  if (numOverrides) {
    plcOverrideAvailBandwidthMbps = avail;
    plcOverrideMacThroughputMbps = macThrough;
  }
}

static void i5PlcLinkMetricsProcessNotification (ieee1905_plc_fob_type *pMsg, int length)
{
   unsigned short lowestTotal = 0xffff;
   unsigned short lowestAvail = 0xffff;
   int            localIfIndex = 0;
   int            dataLen;

   if ( pMsg->macCount > IEEE1905_PLC_FOB_MAX_NODES ) {
      i5TraceError("Invalid number of MACs - %d\n", pMsg->macCount);
      return;
   }

   dataLen = sizeof(unsigned char) + (pMsg->macCount * sizeof(ieee1905_plc_fob_entry_type));
   if ( length < dataLen ) {
      i5TraceError("invalid message length - expected %d, received %d\n", dataLen, length);
      return;
   }

   i5TraceInfo("Number of MACs: %d\n", pMsg->macCount);

   if (pMsg->macCount) {
      ieee1905_plc_fob_entry_type *pFobEntry = pMsg->fobEntry;
      int index = 0;
      for ( ; index < pMsg->macCount; index++, pFobEntry++)
      {
         unsigned short total;
         unsigned short available;
         i5_dm_1905_neighbor_type *neighbor;

         total = ntohs(pFobEntry->totalBw);
         available = ntohs(pFobEntry->availBw);

         if (plcBandwidthOverridesLeft) {
            total = plcOverrideMacThroughputMbps;
            available = plcOverrideAvailBandwidthMbps;
            if (plcBandwidthOverridesLeft < 255) {
              plcBandwidthOverridesLeft --;
            }
            i5TraceInfo("Overriding with %d/%d\n", available, total);
         }

         /* Sanitize */
         if (available > total) {
            available = total;
         }

         neighbor = i5Dm1905GetLocalNeighbor(&pFobEntry->maccAddr[0]);
         if (neighbor) {
            i5Dm1905NeighborBandwidthUpdate(neighbor, total, available, 0, i5_config.i5_mac_address);
            if ( 0 == localIfIndex ) {
               localIfIndex = neighbor->localIfindex;
            }
            if (total < lowestTotal) {
               lowestTotal = total;
            }
            if (available < lowestAvail) {
               lowestAvail = available;
            }
         }
      }

#if defined(SUPPORT_IEEE1905_FM)
      if (localIfIndex != 0) {
        i5FlowManagerMetricUpdate(localIfIndex, lowestTotal, lowestAvail);
      }
#endif /* defined(SUPPORT_IEEE1905_FM) */
   }
}

void i5PlcFOBMessageReceive(i5_socket_type *psock)
{
  unsigned char *pBuf;
  int rc;
  unsigned short int *mmtype;

  i5TraceInfo("\n");

  pBuf = malloc(IEEE1905_PLC_FOB_IND_MAX_SIZE);
  rc = recv(psock->sd, pBuf, IEEE1905_PLC_FOB_IND_MAX_SIZE, 0);
  if (rc > 0) {
    mmtype = (unsigned short int *)&pBuf[IEEE1905_PLC_FOB_IND_MMTYPE_OFFSET];
    if ( IEEE1905_PLC_FOB_IND_MMTYPE == ntohs(*mmtype) )
    {
      int length = rc - IEEE1905_PLC_FOB_IND_PAYLOAD_OFFSET;
      i5PlcLinkMetricsProcessNotification((ieee1905_plc_fob_type *)&pBuf[IEEE1905_PLC_FOB_IND_PAYLOAD_OFFSET], length);
    }
    else {
      i5TraceError("Invalid message received %x\n", ntohs(*mmtype));
    }
  }
  free(pBuf);
}

static int _i5PlcSendMsg( i5_socket_type *psock, unsigned char *pData, unsigned int len )
{
  int sendLen;

  sendLen = sendto(psock->sd, pData, len, 0, NULL, 0);
  if (len != sendLen) {
    printf("%s: sendto failed", __FUNCTION__);
    return -1;
  }

  return 0;
}

void i5PlcConnectUnixSocket(void *arg)
{
  controlSockStruct *pctrl = (controlSockStruct *)arg;
  int                rc;

  if ( pctrl->ptmr != NULL ) {
    i5TimerFree(pctrl->ptmr);
    pctrl->ptmr = NULL;
  }

  if ( NULL == pctrl->psock ) {
    pctrl->psock = i5SocketStreamNew(HOMEPLUGD_MESSAGE_PORT, i5PlcNotifyReceive);
  }

  if ( pctrl->psock )
  {
    if ( 0 == pctrl->macAddrSet ) {
      homeplug_msg_header msghdr;
      msghdr.type = HOMEPLUGD_DEVICE_MAC_GET;
      msghdr.len  = 0;
      rc = _i5PlcSendMsg(i5_config.plc_control_socket.psock, (unsigned char *)&msghdr, sizeof(homeplug_msg_header));
      if ( -1 == rc ) {
        i5Trace("Error sending MAC request\n");
      }
    }
  }

  if ( (NULL == pctrl->psock) || (0 == pctrl->macAddrSet) ) {
      pctrl->ptmr = i5TimerNew(I5_PLC_UNIX_CONNECT_TIMEOUT_MSEC, i5PlcConnectUnixSocket, pctrl);
  }
  else {
    i5_socket_type *pbrsock = i5SocketFindDevSocketByType(i5_socket_type_bridge_ll);
    /* socket is ready and PLC device MAC has been learned so add device */
    i5InterfaceSearchAdd(I5_MATCH_MEDIA_TYPE_PLC);
    /* update bridging tuple */
    if ( pbrsock ) {
      i5_dm_device_type *pdev = i5DmGetSelfDevice();
      if ( (pdev != NULL) && (pdev->BridgingTuplesNumberOfEntries > 0)) {
        i5InterfaceBridgeNotifyReceiveOperStatus(pbrsock, IF_OPER_UP);
      }
    }
  }
}

int i5PlcUKEStart( void )
{
  homeplug_msg_header msghdr;
  int               rc;

  i5Trace("\n");

  if ( 0 == i5PlcControlSockReady() ) {
    return -1;
  }

  msghdr.type = HOMEPLUGD_UKE_CMD_START;
  msghdr.len  = 0;
  rc = _i5PlcSendMsg(i5_config.plc_control_socket.psock, (unsigned char *)&msghdr, sizeof(homeplug_msg_header));
  if ( -1 == rc ) {
    i5Trace("Error sending UKE Start\n");
  }

  return rc;
}

int i5PlcUKERandomize( void )
{
  homeplug_msg_header msghdr;
  int               rc;

  i5Trace("\n");

  if ( 0 == i5PlcControlSockReady() ) {
    return -1;
  }

  msghdr.type = HOMEPLUGD_UKE_CMD_RANDOMIZE;
  msghdr.len  = 0;
  rc = _i5PlcSendMsg(i5_config.plc_control_socket.psock, (unsigned char *)&msghdr, sizeof(homeplug_msg_header));
  if ( -1 == rc ) {
    i5Trace("Error sending UKE Randomize\n");
  }

  return rc;
}

static void i5PlcNotifyReceive(i5_socket_type *psock)
{
  i5_socket_type    *pifsock;
  unsigned char     *pBuf;
  unsigned char     *pRead;
  homeplug_msg_type *pMsg;
  int                len;

  i5Trace("\n");

  pBuf = malloc(HOMEPLUGD_MESSAGE_MAX_SIZE);
  if ( pBuf == NULL ) {
    return;
  }
  pRead = pBuf;
  len = recv(psock->sd, pBuf, HOMEPLUGD_MESSAGE_MAX_SIZE, 0);
  if ( len < 0 ) {
    free(pBuf);
    return;
  }
  else if ( len == 0 ) {
    i5Trace("connection was closed\n");
    free(pBuf);
    i5_config.plc_control_socket.psock = NULL;
    i5SocketClose(psock);
    i5_config.plc_control_socket.macAddrSet = 0;
    i5PlcConnectUnixSocket(&i5_config.plc_control_socket);
    return;
  }

  while ((int)(pRead - pBuf + sizeof(homeplug_msg_header)) <= len) {
    pMsg = (homeplug_msg_type *)pRead;
    pRead += sizeof(homeplug_msg_header);

    if ( 0 == i5_config.plc_control_socket.macAddrSet ) {
      /* only HOMEPLUGD_DEVICE_MAC_GET_RSP can be processed at this time */
      if ( HOMEPLUGD_DEVICE_MAC_GET_RSP == pMsg->hdr.type ) {
        memcpy(&i5_config.plc_control_socket.plcDevMac[0], pRead, MAC_ADDR_LEN);
        i5_config.plc_control_socket.macAddrSet = 1;
        i5Trace("MAC address set\n");
        break;
      }
      else {
        i5TraceError("Received notification %d before MAC address was set\n", pMsg->hdr.type);
      }
    }
    else {
      switch ( pMsg->hdr.type ) {
        case HOMEPLUGD_DEVICES_ADDED:
          i5Trace("Homeplug device(s) added\n");
          pifsock = i5SocketFindDevSocketByAddr(&i5_config.plc_control_socket.plcDevMac[0], NULL);
          if ( pifsock ) {
            i5MessageTopologyDiscoveryTimeout(pifsock);
          }
          break;

        case HOMEPLUGD_DEVICES_REMOVED:
          i5Trace("Homeplug device(s) removed\n");
          if (pMsg->hdr.len > 0 && (pRead - pBuf + pMsg->hdr.len <= len)) {
            i5_socket_type *pif = i5SocketFindDevSocketByAddr(i5_config.plc_control_socket.plcDevMac, NULL);
            i5DmDeviceFreeUnreachableNeighbors(i5_config.i5_mac_address, i5SocketGetIfIndex(pif), &pMsg->data, pMsg->hdr.len);
          }
          break;

        case HOMEPLUGD_DEVICES_NONE:
          i5Trace("HOMEPLUGD_DEVICES_NONE\n");
          break;

        case HOMEPLUGD_UKE_RSP_FAILED:
          i5Trace("UKE failed\n");
          i5SecurityStatusUpdate( I5_MEDIA_TYPE_1901_WAVELET );
          break;

        case HOMEPLUGD_UKE_RSP_SUCCESS:
          i5Trace("UKE success\n");
          i5SecurityStatusUpdate( I5_MEDIA_TYPE_1901_WAVELET );
          break;

        case HOMEPLUGD_UKE_RSP_TIMEOUT:
          i5Trace("UKE timeout\n");
          i5SecurityStatusUpdate( I5_MEDIA_TYPE_1901_WAVELET );
          break;

        /* FOB messages */
        case HOMEPLUGD_IEEE_1905_FOB_REGISTER_RSP_OK:
          i5Trace("FOB registration success\n");
          break;

        case HOMEPLUGD_IEEE_1905_FOB_REGISTER_RSP_ERROR:
          i5TraceError("FOB registration failed - retrying.\n");
          i5DmRetryPlcRegistry();
          break;

        case HOMEPLUGD_SET_PASS_NMK_SUCCESS:
          i5Trace("NMK Set Successful.\n");
          break;

        case HOMEPLUGD_SET_PASS_NMK_FAILURE:
          i5TraceError("NMK Set Failure.\n");
          break;

        case HOMEPLUGD_DEVICE_MAC_GET_RSP:
           i5Trace("MAC response ignored\n");
           break;

        default:
         i5Trace("received unknown message type %d\n", pMsg->hdr.type);
      }
    }
    pRead += pMsg->hdr.len;
  }
  free(pBuf);
}

int i5PlcSetPasswordNmk(unsigned char *password)
{
  int msgLen = sizeof(homeplug_msg_header) + I5_PASSWORD_MAX_LENGTH + 1;
  homeplug_msg_type *msg;

  if ( 0 == i5PlcControlSockReady() ) {
    return -1;
  }

  msg = malloc(msgLen);

  if (NULL == msg) {
    return -1;
  }

  msg->hdr.type = HOMEPLUGD_SET_PASS_NMK;
  msg->hdr.len  = I5_PASSWORD_MAX_LENGTH + 1;

  memset (&msg->data, 0, I5_SSID_MAX_LENGTH + 1);
  strncpy ((char *)&msg->data, (char *)password, I5_SSID_MAX_LENGTH);

  if ( -1 == _i5PlcSendMsg(i5_config.plc_control_socket.psock, (unsigned char *)msg, msgLen) ) {
    i5Trace("Error sending registry call msg\n");
    free(msg);
    return -1;
  }

  free(msg);

  return 0;
}

int i5PlcFOBRegisterMACs(unsigned short int interval, unsigned char n_macs, unsigned char const * macs)
{
  unsigned char     *pBuf;
  homeplug_msg_type *msg;
  char              *p;
  int               rc;
  int               payload_len;
  int               msg_len;

  i5Trace("\n");

  if ( 0 == i5PlcControlSockReady() ) {
    return -1;
  }

  payload_len = sizeof(interval) +  sizeof(n_macs) + ETH_ALEN * n_macs;
  msg_len = sizeof(homeplug_msg_header) +  payload_len;

  pBuf = malloc(msg_len);
  if (NULL == pBuf) {
    return -1;
  }

  msg = (homeplug_msg_type *) pBuf;

  /* Fill message header */
  msg->hdr.type = HOMEPLUGD_IEEE_1905_FOB_REGISTER_CMD;
  msg->hdr.len  = payload_len;

  /* Fill message payload */
  p = (char * ) &msg->data;

  unsigned short int * ushort_p = (unsigned short int *) p;
  *ushort_p = htons(interval);
  p += sizeof(unsigned short int);

  *p = n_macs;
  p++;

  memcpy(p, macs, ETH_ALEN * n_macs);

  /* Send message */
  rc = _i5PlcSendMsg(i5_config.plc_control_socket.psock, (unsigned char *)msg, msg_len);
  if ( -1 == rc ) {
    i5Trace("Error sending registry call msg\n");
  }

  free(pBuf);

  return rc;
}

int i5PlcLinkMetricsUpdateMacList (unsigned char const *macAddresses, unsigned char numMacs)
{
    int counter = 0;
    unsigned char const *saveMacPointer = macAddresses;

    /* Library call to send MAC addresses to PLC SoC */
    i5Trace("rx'd %d Address%s \n", numMacs, (numMacs != 1) ? "es" : "");

    for ( ; counter < numMacs ; counter ++) {
        i5Trace("   Mac address #%d  %02x:%02x:%02x:%02x:%02x:%02x \n", counter,
           macAddresses[0],macAddresses[1],macAddresses[2],
           macAddresses[3],macAddresses[4],macAddresses[5]);
        macAddresses += 6;
    }

    i5PlcFOBRegisterMACs(I5_PLC_BANDWIDTH_UPDATE_INTERVAL_MSEC, numMacs, saveMacPointer);

  return 0;
}

unsigned short i5PlcFetchIfInfo(char const *ifname, unsigned char *pMediaInfo, int *pMediaLen,
                                unsigned char *pNetTechOui, unsigned char *pNetTechVariant, unsigned char *pNetTechName,
                                unsigned char *url, int sizeUrl)
{
  if ( pMediaInfo ) {
    if ( *pMediaLen < i5TlvMediaSpecificInfo1901_Length) {
      i5TraceError("invalid media info length\n");
      return I5_MEDIA_TYPE_UNKNOWN;
    }
    else {
      memset(pMediaInfo, 0, i5TlvMediaSpecificInfo1901_Length);
      *pMediaLen = i5TlvMediaSpecificInfo1901_Length;
    }
  }

  if (pNetTechOui) {
    pNetTechOui[0] = I5_GEN_PHY_HPAV2_NETTECHOUI_01;
    pNetTechOui[1] = I5_GEN_PHY_HPAV2_NETTECHOUI_02;
    pNetTechOui[2] = I5_GEN_PHY_HPAV2_NETTECHOUI_03;
  }

  // TBD - set to actual value when they are defined
  if (pNetTechVariant) {
    *pNetTechVariant = I5_GEN_PHY_HPAV2_NETTECHVARIANT;
  }

  // TBD - set to actual value when they are defined
  if (pNetTechName) {
    strcpy ((char *)pNetTechName, "HomePlug AV 2");
  }

  // TBD - set to actual value when they are defined
  if (url) {
    strncpy ((char *)url, "http://www.HomePlug.org/", sizeUrl);
  }

  return I5_MEDIA_TYPE_UNKNOWN;
}

#endif /* SUPPORT_HOMEPLUG */

void i5PlcInitialize( void )
{
#if defined(SUPPORT_HOMEPLUG)
  i5PlcConnectUnixSocket(&i5_config.plc_control_socket);
#endif // endif
}
