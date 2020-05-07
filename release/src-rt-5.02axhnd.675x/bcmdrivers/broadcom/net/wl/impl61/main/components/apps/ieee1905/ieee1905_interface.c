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
 * IEEE1905 Interface
 */

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
#include "ieee1905_plc.h"
#include "ieee1905_plc_fob.h"
#include "ieee1905_tlv.h"
#include "ieee1905_wlmetric.h"
#if defined(BRCM_CMS_BUILD)
#include "ieee1905_cmsutil.h"
#endif // endif
#include "ieee1905_flowmanager.h"
#include "ieee1905_brutil.h"
#include "ieee1905_glue.h"
#if defined(WIRELESS)
#include "ieee1905_wlcfg.h"
#endif // endif
#include "ieee1905_ethstat.h"

#define I5_TRACE_MODULE                                i5TraceInterface
#define I5_INTERFACE_WIFI_PROMISCUOUS_STP_TIMEOUT_MSEC 5000

typedef void (*i5SocketReceiveFunc)(i5_socket_type *psock);

int i5InterfaceInfoGet(char *ifname, unsigned char *mac_address)
{
  struct ifreq ifr;
  int sockfd;
  int ifindex;
#if defined(SUPPORT_HOMEPLUG)
  unsigned short mediaType;
#endif // endif

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return -1;
  }

  strncpy(ifr.ifr_name, ifname, I5_MAX_IFNAME-1);
  ifr.ifr_name[I5_MAX_IFNAME-1] = '\0';
  if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
    close(sockfd);
    return -1;
  }
  ifindex = ifr.ifr_ifindex;

#if defined(SUPPORT_HOMEPLUG)
  unsigned char netTechOui[I5_PHY_INTERFACE_NETTECHOUI_SIZE];
  mediaType = i5GlueInterfaceGetMediaInfoFromName(ifname, NULL, NULL, netTechOui, NULL, NULL,
    NULL, 0, NULL);
  if ( i5DmIsInterfacePlc(mediaType, netTechOui) ) {
    if ( i5PlcControlSockReady() ){
      if ( i5PlcGetDevMacAddress(mac_address) < 0 ) {
        close(sockfd);
        return -1;
      }
    }
    else {
      close(sockfd);
      return -1;
    }
  }
  else
#endif // endif
  {
    ifr.ifr_addr.sa_family = AF_INET;
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) {
      close(sockfd);
      return -1;
    }
    memcpy(mac_address, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
  }

  close(sockfd);
  return (ifindex);
}

/* Check whether interface is loopback or not */
static bool i5IsInterfaceLoopBack(char *ifname)
{
  struct ifreq ifr;
  int sockfd;
  bool isLoopBack = FALSE;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
     goto end;
  }

  I5STRNCPY(ifr.ifr_name, ifname, I5_MAX_IFNAME);

  if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
     goto end;
  }

  if (ifr.ifr_flags & IFF_LOOPBACK) {
     isLoopBack = TRUE;
  }

end:

  close(sockfd);
  return isLoopBack;
}

int i5InterfacePacketSend(i5_socket_type *psock, i5_packet_type *ppkt)
{
  if (sendto(psock->sd, ppkt->pbuf, ppkt->length, 0, (struct sockaddr*)&(psock->u.sll.sa), sizeof(struct sockaddr_ll)) == -1) {
//    printf("sendto() error\n");
    return IEEE1905_FAIL;
  }
  return IEEE1905_OK;
}

void i5InterfacePacketReceive(i5_socket_type *psock)
{
  int length;
  i5_packet_type *ppkt;

  i5TraceInfo(": received packet from interface %s (socket %d, proto:%04x type:%d)\n", psock->u.sll.ifname, psock->sd, ntohs(psock->u.sll.sa.sll_protocol), psock->type);
  if ((ppkt = i5PacketNew()) != NULL) {
    length = recvfrom(psock->sd, ppkt->pbuf, I5_PACKET_BUF_LEN, 0, NULL, NULL);
    if (psock->type == i5_socket_type_bridge_ll) {
      i5TraceError("Error, received packet on bridge\n");
      i5PacketFree(ppkt);
      return;
    }
    if (length == -1) {
      i5PacketFree(ppkt);
    }
    else {
      ppkt->length = length;
      i5MessagePacketReceive(psock, ppkt);
    }
  }
}

void i5InterfaceEthernetNotifyReceiveOperStatus(i5_socket_type *psock, unsigned char const operStatus)
{
  unsigned char aggregateOperStatus = operStatus;

  i5Trace("Oper Status: %d\n", aggregateOperStatus);
  if (IF_OPER_UP == aggregateOperStatus || IF_OPER_UNKNOWN == aggregateOperStatus) {
    psock->u.sll.discoveryRetryPeriod = 0;
    i5MessageTopologyDiscoveryTimeout(psock);
    if (i5_config.ptmrApSearch == NULL) {
      i5WlCfgMultiApControllerSearch(NULL);
    }
  }
  else if (IF_OPER_DOWN == aggregateOperStatus) {
#ifndef MULTIAP
    i5_socket_type *searchSock = i5_config.i5_socket_list.ll.next;
    while (searchSock) {
      // The interface is still up if we can find a socket with same MAC
      // but don't count the input psock, since that's going down, and don't count the bridge
      if ((psock->type == i5_socket_type_ll) &&
          (psock->u.sll.sa.sll_ifindex != searchSock->u.sll.sa.sll_ifindex) &&
          (memcmp(psock->u.sll.mac_address, searchSock->u.sll.mac_address, ETH_ALEN) == 0)) {
        aggregateOperStatus = IF_OPER_UP;
        break;
      }
      searchSock = searchSock->ll.next;
    }
#else
      i5_dm_device_type *pDeviceController = i5DmFindController();

      if (pDeviceController && pDeviceController->psock == psock) {
        i5Trace("Ethernet interface %s("I5_MAC_DELIM_FMT"), providing backhaul connectivity to "
          "controller is down. Enable Wi-Fi backhaul STA roaming\n",
          psock->u.sll.ifname, I5_MAC_PRM(psock->u.sll.mac_address));
	if(i5_config.cbs.set_bh_sta_params) {
	  i5_config.cbs.set_bh_sta_params(IEEE1905_BH_STA_ROAM_ENAB_VAP_FOLLOW);
	}
      }
#endif // endif
  }
  i5DmInterfaceStatusSet(i5_config.i5_mac_address, psock->u.sll.mac_address, i5SocketGetIfIndex(psock), aggregateOperStatus);

  if ((IF_OPER_DOWN == operStatus) && (IF_OPER_UP == aggregateOperStatus)) {
    i5DmDeviceFreeUnreachableNeighbors(i5_config.i5_mac_address, i5SocketGetIfIndex(psock), NULL, 0);
  }
}

void i5InterfaceSocketPromiscuousMulticastSet(i5_socket_type *psock, unsigned char *multicast_address)
{
  struct packet_mreq mr;

  memset(&mr,0,sizeof(mr));
  mr.mr_ifindex = psock->u.sll.sa.sll_ifindex;
  mr.mr_type = PACKET_MR_MULTICAST;
  mr.mr_alen = ETH_ALEN;
  memcpy(mr.mr_address, multicast_address, ETH_ALEN);

  if(setsockopt(psock->sd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0)
  {
    printf("%s: setsockopt error (%s)\n", __func__, strerror(errno));
    printf("multicast_addr: %02x %02x %02x %02x %02x %02x\n",
           multicast_address[0],multicast_address[1],multicast_address[2],
           multicast_address[3],multicast_address[4],multicast_address[5]);
  }
}

void i5InterfaceWifiPromiscuousStp(void *arg)
{
  unsigned char stp_mc_address[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
  i5_socket_type *psock = (i5_socket_type *)arg;

  i5TimerFree(psock->u.sll.pnltmr);
  psock->u.sll.pnltmr = NULL;
  i5InterfaceSocketPromiscuousMulticastSet(psock, stp_mc_address);

  i5MessageTopologyDiscoveryTimeout(psock);
}

void i5InterfaceWifiNotifyReceiveOperStatus(i5_socket_type *psock, unsigned char oper_status)
{
  i5Trace("Interface: %s Oper Status: %d\n",psock->u.sll.ifname, oper_status);
  /* If oper status is unknown, assume bridge port state 'forwarding' as UP and all other state as down */
  if (IF_OPER_UNKNOWN == oper_status) {
    if (i5BrUtilGetPortStpState(psock->u.sll.ifname) == 0) {
      oper_status = IF_OPER_DOWN;
    } else {
      oper_status = IF_OPER_UP;
    }
    i5Trace("Oper Status: %d based on bridge port state\n", oper_status);
  }
  if (IF_OPER_UP == oper_status) {
    /* A delay is needed by the wlan interface, unfortunately, before we can configure it */
    if (psock->u.sll.pnltmr) {
      i5TimerFree(psock->u.sll.pnltmr);
    }
    psock->u.sll.pnltmr = i5TimerNew(I5_INTERFACE_WIFI_PROMISCUOUS_STP_TIMEOUT_MSEC, i5InterfaceWifiPromiscuousStp, psock);
#if defined(SUPPORT_IEEE1905_FM) && defined(SUPPORT_IEEE1905_AUTO_WDS)
    i5FlowManagerProcessWirelessUp();
#endif // endif
#if defined(WIRELESS)
#ifdef MULTIAP
    if (i5_config.ptmrApSearch == NULL && !i5WlCfgIsVirtualInterface(psock->u.sll.ifname)) {
      i5WlCfgMultiApControllerSearch(psock->u.sll.mac_address);
    }
#else
    i5WlcfgApAutoconfigurationStart(psock->u.sll.ifname);
#endif /* MULTIAP */
#endif // endif
  }
  else if (IF_OPER_DOWN == oper_status) {
#if defined(SUPPORT_IEEE1905_FM) && defined(SUPPORT_IEEE1905_AUTO_WDS)
    i5FlowManagerProcessLocalWirelessDown();
#endif // endif
#if defined(WIRELESS)
    i5WlcfgApAutoconfigurationStop(psock->u.sll.ifname);
#endif // endif
  }
  i5DmInterfaceStatusSet(i5_config.i5_mac_address, psock->u.sll.mac_address, i5SocketGetIfIndex(psock), oper_status);
}

void i5InterfacePlcNotifyReceiveOperStatus(i5_socket_type *psock, unsigned char oper_status)
{
  i5Trace("Oper Status: %d\n", oper_status);
  if (IF_OPER_UP == oper_status || IF_OPER_DOWN == oper_status) {
    i5DmInterfaceStatusSet(i5_config.i5_mac_address, psock->u.sll.mac_address, i5SocketGetIfIndex(psock), oper_status);
  }
}

i5_socket_type *i5InterfaceProtoSocketCreate(char *ifname, unsigned short protocol, unsigned int socket_type, i5SocketReceiveFunc pRcvFunc)
{
  i5_socket_type *psock;
  int sd;
  int flags;

  if ((sd = socket(AF_PACKET, SOCK_RAW, htons(protocol))) == -1) {
    printf("socket() error - ensure that this process is running as root\n");
    return NULL;
  }

  flags = fcntl(sd, F_GETFL, 0);
  if(flags < 0) {
     printf("cannot retrieve socket flags. errno=%d", errno);
  }
  if ( fcntl(sd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
     printf("cannot set socket to non-blocking. errno=%d", errno);
  }

  psock = i5SocketNew(sd, socket_type, pRcvFunc);
  if ( psock == NULL ) {
    close(sd);
    return NULL;
  }
  psock->u.sll.sa.sll_family   = PF_PACKET;
  psock->u.sll.sa.sll_protocol = htons(protocol); // Set to receive only these packets
  psock->u.sll.sa.sll_halen    = ETH_ALEN; // Some versions of linux can't send without this
  strncpy(psock->u.sll.ifname, ifname, I5_MAX_IFNAME-1);
  psock->u.sll.ifname[I5_MAX_IFNAME-1] = '\0';

  if ((psock->u.sll.sa.sll_ifindex = i5InterfaceInfoGet(ifname, psock->u.sll.mac_address)) == -1) {
    printf("i5InterfaceInfoGet(%s) error\n", ifname);
    i5SocketClose(psock);
    return NULL;
  }

  if((bind(sd, (struct sockaddr *)&(psock->u.sll.sa), sizeof(struct sockaddr_ll)))== -1) {
    printf("bind() error\n");
    i5SocketClose(psock);
    return NULL;
  }

  i5Trace("new socket: sd=%d, protocol=%04hx, %s\n", sd, protocol, ifname);

  return(psock);
}

i5_socket_type *i5InterfaceSocketSet(char *ifname, unsigned short media_type, unsigned char const *pNetTechOui)
{
  i5_socket_type *pifsock;
  i5_socket_type *psock;

  /* create 1905 protocol socket */
  pifsock = i5InterfaceProtoSocketCreate(ifname, I5_PROTO, i5_socket_type_ll, i5InterfacePacketReceive);
  if ( NULL == pifsock ) {
    return NULL;
  }
  i5InterfaceSocketPromiscuousMulticastSet(pifsock, I5_MULTICAST_MAC);
  if ( i5DmIsInterfaceWireless(media_type) ) {
    pifsock->u.sll.options = USE_IF_MAC_AS_SRC;
    pifsock->u.sll.notify  = i5InterfaceWifiNotifyReceiveOperStatus;
  }
  else if (i5DmIsInterfaceEthernet(media_type) ) {
    pifsock->u.sll.options = USE_AL_MAC_AS_SRC;
    pifsock->u.sll.notify  = i5InterfaceEthernetNotifyReceiveOperStatus;
    pifsock->u.sll.pInterfaceCtx = i5EthStatGetCtx(ifname);
  }
  else if (i5DmIsInterfacePlc(media_type, pNetTechOui) ) {
    pifsock->u.sll.options = USE_AL_MAC_AS_SRC;
    pifsock->u.sll.notify = i5InterfacePlcNotifyReceiveOperStatus;
#if defined(SUPPORT_HOMEPLUG)
    psock = i5InterfaceProtoSocketCreate(ifname, IEEE1905_PLC_FOB_IND_ETHERTYPE, i5_socket_type_raweth, i5PlcFOBMessageReceive);
    if ( NULL == psock ) {
      i5SocketClose(pifsock);
      return NULL;
    }
    psock->u.sll.options = pifsock->u.sll.options;
    psock->u.sll.notify = NULL;
    pifsock->u.sll.pMetricSock = psock;
#endif // endif
  }
  else {
    pifsock->u.sll.options = USE_AL_MAC_AS_SRC;
    pifsock->u.sll.notify = NULL;
  }

  /* create LLDP socket */
  psock = i5InterfaceProtoSocketCreate(ifname, LLDP_PROTO, i5_socket_type_raweth, i5InterfacePacketReceive);
  if ( NULL == psock ) {
    i5SocketClose(pifsock);
    return NULL;
  }
  psock->u.sll.options = pifsock->u.sll.options;
  psock->u.sll.notify = NULL;
  pifsock->u.sll.pLldpProtoSock = psock;
  i5InterfaceSocketPromiscuousMulticastSet(psock, LLDP_MULTICAST_MAC);

  return(pifsock);
}

void i5InterfaceNew(char *ifname, unsigned short media_type, unsigned char const *media_specific_info,
                    unsigned int media_specific_info_size, unsigned char const *pNetTechOui,
                    unsigned char const *pNetTechVariant,  unsigned char const *pNetTechName,
                    unsigned char const *url, int sizeUrl, i5MacAddressDeliveryFunc deliverFunc,
                    char *real_ifname)
{
  i5_socket_type *psock = NULL;
  unsigned char   macAddr[MAC_ADDR_LEN];
  unsigned char  *interfaceId = NULL;
  int             ifindex;
  int             bCreateSocket;
  unsigned char   status;
  unsigned char   isVirtual = 0, isLoopBack = 0;
#if defined(SUPPORT_BRIDGE_DEDICATED_PORT) && defined(WIRELESS) && \
	defined(SUPPORT_IEEE1905_FM)
  char            parentIfnameBuf[I5_MAX_IFNAME];
  char            bridgeIfnameBuf[I5_MAX_IFNAME];
  char *          brName;
  char *          wlParentName;
#endif // endif

  i5Trace("called for interface %s media type = %04x\n", ifname, media_type);

#if defined(SUPPORT_BRIDGE_DEDICATED_PORT) && defined(WIRELESS) && \
	defined(SUPPORT_IEEE1905_FM)
  if (real_ifname && strlen(real_ifname) > 0) {
    wlParentName = i5WlcfgGetWlParentInterface(real_ifname, parentIfnameBuf);
  } else {
    wlParentName = i5WlcfgGetWlParentInterface(ifname, parentIfnameBuf);
  }
  if (  wlParentName != NULL && wlParentName != ifname) {
      // we are the child interface of another interface.  Assume it's dedicated.
      brName = i5BrUtilGetBridgeName(ifname, bridgeIfnameBuf);
      i5TraceAssert(brName != NULL);
      if (brName) {
          printf("Marking %s as dedicated (bridge: %s)\n", ifname, brName);
          i5BrUtilMarkDedicatedStpPort(brName, ifname, 1);
      }
  }
#endif // endif

  isLoopBack = i5IsInterfaceLoopBack(ifname);

  bCreateSocket = i5GlueInterfaceIsSocketRequired(ifname, media_type, real_ifname);
  if ( 0 == bCreateSocket ) {
    i5TraceInfo("create dm interface but not sockets for %s\n", ifname);
    ifindex = i5InterfaceInfoGet(ifname, &macAddr[0]);
    if ( ifindex <= 0 ) {
      i5TraceError("Unable to read interface information for %s\n", ifname);
      return;
    }
    interfaceId = &macAddr[0];
    status = i5GlueIsOperStateUP(ifname) ? IF_OPER_UP : IF_OPER_DOWN;
  }
  else {
    i5TraceInfo("create dm interface and sockets for %s\n", ifname);

    /* Remove the socket created for primary interface if virtual VLAN interface is getting
     * created.
     */
    if (I5_IS_GUEST_ENABLED(i5_config.flags)) {
      i5GlueRemovePrimarySocketOnVirtualSocketCreate(ifname);
    }

    psock = i5InterfaceSocketSet(ifname, media_type, pNetTechOui);
    if ( psock == NULL ) {
      i5TraceError("Unable to create sockets for %s\n", ifname);
      return;
    }
    interfaceId = &psock->u.sll.mac_address[0];
    status = IF_OPER_UP;

    /* If it is loopback socket, set the flag */
    if (isLoopBack) {
      psock->flags |= I5_SOCKET_FLAG_LOOPBACK;
    }

    /* Create VLAN interfaces whenever creating a IEEE1905 socket */
    if (i5GlueIsCreateVLANS() && !isLoopBack) {
      /* Create VLAN if the scoket created is non virtual VLAN. Because after creating a virtual
       * VLAN interface, the flow will come here to create the socket for virtual VLAN. So, we
       * should skip creating VLAN for virtual VLAN interface
       */
      if (!i5GlueIsIfnameVLAN(ifname, NULL, NULL)) {
        i5TraceInfo("Socket Created For[%s]. So, create VLANs\n", ifname);
        i5GlueCreateVLAN(ifname, (i5DmIsInterfaceWireless(media_type) ? 1 : 0));
      }
    }

#if defined(MULTIAP) && defined(__CONFIG_DHDAP__) && defined(__CONFIG_GMAC3__)
    /* Set IEEE1905 AL MAC and Multicast MAC, so that the driver will send the packet to socket
     * created on the interface directly instead of sending it to bridge socket
     */
    if (i5DmIsInterfaceWireless(media_type)) {
      i5WlCfgSetIEEE1905MACInDHD(ifname, i5_config.i5_mac_address, 1);
      i5WlCfgSetIEEE1905MACInDHD(ifname, I5_MULTICAST_MAC, 0);
    }
#endif /* MULTIAP && __CONFIG_DHDAP__ &&  __CONFIG_GMAC3__ */
  }

#ifdef MULTIAP
  if (i5DmIsInterfaceWireless(media_type) &&
    i5WlCfgIsVirtualInterface(ifname)) {
    isVirtual = 1;
  }
#endif /* MULTIAP */

  /* Do not add loopback interface in the interface list. Add virtual interfaces only in
   * controller. In controller it is required while filling neighbor interface ID in the
   * neighbor list. In the dual band case, if the backhaul is virtual we need it for filling
   * neighbor interface ID as the controller will not have BSSIDs.
   */
  if (!isLoopBack &&
    (I5_IS_MULTIAP_CONTROLLER(i5_config.flags) || !isVirtual)) {
    i5DmInterfaceUpdate(i5_config.i5_mac_address, interfaceId, I5_MESSAGE_VERSION, media_type,
                      media_specific_info, media_specific_info_size,
                      deliverFunc, ifname, status);
  } else {
    i5TraceInfo("Not Updating Interface for ifname %s, beacuase its %s interface\n",
      ifname, isVirtual ? "Virtual" : "Loopback");
  }

  if ((media_type == 0xffff) && (pNetTechOui || pNetTechVariant || pNetTechName || url)) {
    i5DmInterfacePhyUpdate(i5_config.i5_mac_address, interfaceId, pNetTechOui, pNetTechVariant, pNetTechName, url);
  }

  if (psock) {
#if defined(SUPPORT_IEEE1905_FM)
    i5FlowManagerActivateInterface(psock);
#endif /* defined(SUPPORT_IEEE1905_FM) */

    i5MessageTopologyDiscoveryTimeout(psock);
  }
}

void i5InterfaceAddDefaultBrouteEntries(char *ifname)
{
  int ret;
  char *cmd;
  unsigned char macAddr[6];
#ifdef MULTIAP
  char *psz;
  char lib_path[] = "/lib:/lib/aarch64/gpl:/lib/aarch64:/lib64/gpl:/lib64";
#endif /* MULTIAP */

  ret = i5InterfaceInfoGet(ifname, &macAddr[0]);
  if ( ret < 0 ) {
    i5TraceError("Unable to retrieve interface information\n");
  }

  cmd = (char *)malloc(256);

#ifdef MULTIAP
  psz = getenv("LD_LIBRARY_PATH");
  memset(cmd, 0, 256);
  if (psz != NULL && strlen(psz) > 0) {
    memcpy(cmd, psz, strlen(psz));
    strcat(cmd, ":");
  }

  if (strstr(cmd, lib_path) == NULL) {
    strcat(cmd, lib_path);
    setenv("LD_LIBRARY_PATH", cmd, 1);
  }
#endif /* MULTIAP */

  snprintf(cmd, 256,
               "ebtables -t broute -D BROUTING -d " I5_MAC_DELIM_FMT " -p 0x%x -j DROP;"
               "ebtables -t broute -I BROUTING 1 -d " I5_MAC_DELIM_FMT " -p 0x%x -j DROP;",
               I5_MAC_PRM(I5_MULTICAST_MAC), I5_PROTO,
               I5_MAC_PRM(I5_MULTICAST_MAC), I5_PROTO);
  system(cmd);

  snprintf(cmd, 256,
               "ebtables -t broute -D BROUTING -d " I5_MAC_DELIM_FMT " -p 0x%x -j DROP;"
               "ebtables -t broute -I BROUTING 1 -d " I5_MAC_DELIM_FMT " -p 0x%x -j DROP;",
               I5_MAC_PRM(i5_config.i5_mac_address), I5_PROTO,
               I5_MAC_PRM(i5_config.i5_mac_address), I5_PROTO);
  system(cmd);

  snprintf(cmd, 256,
               "ebtables -t broute -D BROUTING -d " I5_MAC_DELIM_FMT " -p 0x%x -j DROP;"
               "ebtables -t broute -I BROUTING 1 -d " I5_MAC_DELIM_FMT " -p 0x%x -j DROP;",
               I5_MAC_PRM(macAddr), I5_PROTO,
               I5_MAC_PRM(macAddr), I5_PROTO);
  system(cmd);
  free(cmd);
}

void i5InterfaceBridgeNotifyReceiveOperStatus( i5_socket_type *psock, unsigned char oper_status )
{
  unsigned char  brIfMacs[I5_DM_BRIDGE_TUPLE_MAX_INTERFACES][MAC_ADDR_LEN];
  char           portList[I5_DM_BRIDGE_TUPLE_MAX_INTERFACES][I5_MAX_IFNAME];
  int            index;
  int            prt, j;
  int            portCnt;
  int            found;
  int            ret;
#if defined(SUPPORT_HOMEPLUG)
  int            hasPlc = 0;
#endif // endif

  i5Trace("\n");

  if ( (oper_status != IF_OPER_UP) && (oper_status != IF_OPER_UNKNOWN) ) {
    /* remove bridging tuple entry */
    i5DmBridgingTupleUpdate(i5_config.i5_mac_address, I5_MESSAGE_VERSION, psock->u.sll.ifname, 0, NULL);
  }
  else {
    memset(portList, 0, I5_DM_BRIDGE_TUPLE_MAX_INTERFACES * I5_MAX_IFNAME);
    portCnt = i5BrUtilGetPortList(psock->u.sll.ifname, I5_DM_BRIDGE_TUPLE_MAX_INTERFACES, &portList[0][0]);
    if ( portCnt == 0 ) {
      /* bridging tuple entry */
      i5DmBridgingTupleUpdate(i5_config.i5_mac_address, I5_MESSAGE_VERSION, psock->u.sll.ifname, 0, NULL);
    }
    else {
      int sockfd;
      struct ifreq ifr;

      if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return;
      }

      index = 0;
      for( prt = 0; prt < portCnt; prt++) {
#if defined(SUPPORT_HOMEPLUG)
        unsigned char netTechOui[I5_PHY_INTERFACE_NETTECHOUI_SIZE];
        unsigned short mediaType = i5GlueInterfaceGetMediaInfoFromName(portList[prt], NULL, NULL,
          netTechOui, NULL, NULL, NULL, 0, NULL);
        if ( 1 == i5DmIsInterfacePlc(mediaType, netTechOui) ) {
          hasPlc = 1;
        }
#endif // endif
        strncpy(ifr.ifr_name, portList[prt], I5_MAX_IFNAME-1);
        ifr.ifr_name[I5_MAX_IFNAME-1] = '\0';
        ifr.ifr_addr.sa_family = AF_INET;
        ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
        if ( ret >= 0 ) {
          memcpy(&brIfMacs[index], ifr.ifr_hwaddr.sa_data, ETH_ALEN);
          if ( index == 0 ) {
            index++;
            continue;
          }
          found = 0;
          for ( j = 0; j < index; j++) {
            if (0 == memcmp(&brIfMacs[j], &brIfMacs[index], MAC_ADDR_LEN)) {
              /* already in list */
              found = 1;
              break;
            }
          }
          if ( 0 == found ) {
            index++;
            if ( I5_DM_BRIDGE_TUPLE_MAX_INTERFACES == index) {
               i5Trace(" maximum entries reached\n");
            }
          }
        }
      }

      close(sockfd);
      if ( index ) {
#if defined(SUPPORT_HOMEPLUG)
         if ((1 == hasPlc) && (index < I5_DM_BRIDGE_TUPLE_MAX_INTERFACES)) {
           if ( i5PlcControlSockReady() ) {
             if ( 0 == i5PlcGetDevMacAddress(brIfMacs[index]) ) {
               index++;
             }
           }
         }
#endif // endif
        i5DmBridgingTupleUpdate(i5_config.i5_mac_address, I5_MESSAGE_VERSION, psock->u.sll.ifname, index, &brIfMacs[0][0]);
      }
    }
  }
}

void i5InterfaceAdd(char *ifname, unsigned short matchMediaType)
{
  unsigned char  mediaSpecificInfo[i5TlvMediaSpecificInfoWiFi_Length] = {0};
  unsigned short mediaType;
  int            mediaLen = i5TlvMediaSpecificInfoWiFi_Length, is_loopback_listen;
  unsigned char  netTechOui[I5_PHY_INTERFACE_NETTECHOUI_SIZE];
  unsigned char  netTechVariant;
  unsigned char  netTechName[I5_PHY_INTERFACE_NETTECHNAME_SIZE];
  unsigned char  url[I5_PHY_INTERFACE_URL_MAX_SIZE];
  char real_ifname[I5_MAX_IFNAME], vlan_ifname[I5_MAX_IFNAME];

  /* skip interfaces that have already been learned */
  if ( i5SocketFindDevSocketByName(ifname) != NULL ) {
    return;
  }

  /* Skip primary VLAN interfaces, if the scoket is already created for virtual primary VLAN */
  if (I5_IS_GUEST_ENABLED(i5_config.flags) && i5_config.prim_vlan_id > 0) {
    snprintf(vlan_ifname, sizeof(vlan_ifname), "%s.%d", ifname, i5_config.prim_vlan_id);
    if (i5SocketFindDevSocketByName(vlan_ifname) != NULL) {
      i5Trace("ifname[%s] vlan_ifname[%s]. Socket already created\n", ifname, vlan_ifname);
      return;
    }
  }

  memset(real_ifname, 0, sizeof(real_ifname));

  mediaType = i5GlueInterfaceGetMediaInfoFromName(ifname, mediaSpecificInfo, &mediaLen, netTechOui,
    &netTechVariant, netTechName, url, I5_PHY_INTERFACE_URL_MAX_SIZE, real_ifname);
  i5Trace("called for interface %s, mediaType %x, matchType %x real_ifname %s\n",
    ifname, mediaType, matchMediaType, real_ifname);
  if ( I5_MATCH_MEDIA_TYPE_WL == matchMediaType ) {
    if (!i5DmIsInterfaceWireless(mediaType) ) {
      return;
    }
  }
  else if ( I5_MATCH_MEDIA_TYPE_PLC == matchMediaType ) {
    i5Trace("I5_MATCH_MEDIA_TYPE_PLC Checking\n");
    if (!i5DmIsInterfacePlc(mediaType, mediaType == 0xffff ? netTechOui : NULL) ) {
      i5Trace("I5_MATCH_MEDIA_TYPE_PLC Check failed\n");
      return;
    }
    i5Trace("I5_MATCH_MEDIA_TYPE_PLC Check passed\n");
  }
  else if ( I5_MATCH_MEDIA_TYPE_ETH == matchMediaType ) {
    if (!i5DmIsInterfaceEthernet(mediaType) ) {
      return;
    }
  }
  else if ( (matchMediaType != I5_MATCH_MEDIA_TYPE_ANY) && (matchMediaType != mediaType) ) {
    return;
  }

  if ( I5_MEDIA_TYPE_BRIDGE == mediaType) {
    i5_socket_type *pifsock = i5InterfaceProtoSocketCreate(ifname, I5_PROTO, i5_socket_type_bridge_ll, i5InterfacePacketReceive);
    char cmd[50];
    if (NULL == pifsock) {
      exit(-1);
    }
    /* Turn on STP on the bridge to support dynamic backhaul */
    snprintf(cmd, sizeof(cmd), "brctl stp %s on", ifname);
    system(cmd);
    pifsock->u.sll.options = USE_AL_MAC_AS_SRC;
    pifsock->u.sll.notify  = i5InterfaceBridgeNotifyReceiveOperStatus;
    i5InterfaceBridgeNotifyReceiveOperStatus(pifsock, IF_OPER_UP);
    i5InterfaceAddDefaultBrouteEntries(ifname);
    return;
  }
#if defined(SUPPORT_HOMEPLUG)
  else if ( i5DmIsInterfacePlc(mediaType, netTechOui) ) {
    i5Trace("SUPPORT HOMEPLUG\n");
    if ( i5PlcControlSockReady() ) {
      i5Trace("SUPPORT HOMEPLUG socket ready\n");
      i5InterfaceNew(ifname, mediaType, mediaSpecificInfo, mediaLen,
                     netTechOui, &netTechVariant, netTechName, url, I5_PHY_INTERFACE_URL_MAX_SIZE,
                     i5PlcLinkMetricsUpdateMacList, NULL);
    }
    else {
      i5Trace("SUPPORT HOMEPLUG socket not ready\n");
    }
  }
#endif // endif
#if defined(WIRELESS)
  else if ( i5DmIsInterfaceWireless(mediaType)) {
    i5InterfaceNew(ifname, mediaType, NULL, 0,
                   NULL, NULL, NULL, NULL, I5_PHY_INTERFACE_URL_MAX_SIZE,
                   NULL, real_ifname);
  }
#endif // endif
  else if (I5_MEDIA_TYPE_UNKNOWN != mediaType) {
    i5InterfaceNew(ifname, mediaType, mediaSpecificInfo, mediaLen,
                   NULL, NULL, NULL, NULL, I5_PHY_INTERFACE_URL_MAX_SIZE,
                   NULL, NULL);
  }
#ifdef MULTIAP
  else if (i5IsInterfaceLoopBack(ifname)) {
    is_loopback_listen = (int)strtoul(i5WlCfgGetNvVal(NULL, I5_WLCFG_NVRAM_LOOPBACK), NULL, 0);
    i5Trace("ifname[%s] is loopback is_loopback_listen[%s=%d] and i5_config.device_mode[0x%x]\n",
      ifname, I5_WLCFG_NVRAM_LOOPBACK, is_loopback_listen, i5_config.device_mode);
    /* Create loopback if the device has both controller and agent */
    if ((I5_IS_MULTIAP_CONTROLLER(i5_config.device_mode) &&
      I5_IS_MULTIAP_AGENT(i5_config.device_mode)) ||
      is_loopback_listen) {
      i5InterfaceNew(ifname, mediaType, mediaSpecificInfo, mediaLen,
                     NULL, NULL, NULL, NULL, I5_PHY_INTERFACE_URL_MAX_SIZE,
                     NULL, NULL);
    }
  }
#endif /* MULTIAP */

  return;
}

void i5InterfaceSearchAdd(unsigned short matchMediaType)
{
  DIR *dp;
  struct dirent *ep;
  char brif_path[MAX_PATH_NAME];

  snprintf(brif_path, MAX_PATH_NAME, "/sys/class/net");
  dp = opendir (brif_path);
  if (dp != NULL)
  {
    while ((ep = readdir(dp)) != NULL) {
      i5InterfaceAdd(ep->d_name, matchMediaType);
    }
    closedir (dp);
  }
  else {
    printf("Warning can't read interface list\n");
    return;
  }
}

/* return of zero indicates the searchString is not present *
 * any other value returned is the 1-based line number      *
 *
 * Notes: "wl wds" puts MACs in REVERSE order               */
int i5InterfaceSearchFileForString(char const *file, char const *searchString)
{
  char line[32] = "";
  unsigned int lineNumber = 0;
  int found = 0;

  if ((file == NULL) || (searchString == NULL)) {
    return 0;
  }

  i5Trace("Search for %s in %s\n", searchString, file);

  FILE* fp = fopen(file,"r");
  if (fp == NULL) {
    i5Trace("No such file\n");
    return 0;
  }

  while (fgets(line, sizeof(line)-1, fp) != NULL) {
    i5TraceInfo("Line: %s\n", line);
    if (!found) {
      if (strstr(line, searchString) != NULL ) {
        found = 1;
        lineNumber = 1; /* if this is last line in the "wl wds" list, we will return "1" */
        i5TraceInfo("Found! index=%d\n", lineNumber);
      }
    }
    else {
      /* For each line after the line we wanted, add 1 to the count */
      i5TraceInfo("Incrementing: index=%d\n", lineNumber);
      lineNumber ++;
    }
  }
  fclose(fp);
  i5Trace("returning index=%d\n", lineNumber);
  return lineNumber;
}

/* return of a pointer to the string in the index'th position *
 * return 0 on success
 * return 1 on failure
 *
 * Notes: "wl wds" puts MACs in REVERSE order by index        */
int i5InterfaceSearchFileForIndex(char const *file, unsigned int wdsIndex, char* macString, unsigned int size)
{
  unsigned int totalLines = 0;
  char line[32] = "";

  if ((file == NULL) || (wdsIndex == 0) || (macString == NULL) || (size == 0)) {
    return -1;
  }

  i5Trace("Search for index %d in %s\n", wdsIndex, file);

  FILE* fp = fopen(file,"r");
  if (fp == NULL) {
    i5Trace("No such file\n");
    return -1;
  }

  while (fgets(line, sizeof(line)-1, fp) != NULL) {
    totalLines ++;
  }

  if (wdsIndex > totalLines) {
    i5TraceError("ERROR: only %d WDS bridges, can't find bridge #%d\n", totalLines, wdsIndex);
    fclose(fp);
    return -1;
  }
  rewind(fp);
  while (totalLines >= wdsIndex) {
    fgets(line, sizeof(line)-1, fp);
    totalLines --;
  }
  strncpy(macString, &line[4], size); /* line[] looks like "wds xx:xx:xx:xx:xx:xx" */
  macString[size-1] = '\0';
  fclose(fp);
  return 0;
}

void i5InterfaceInit()
{
  srand((unsigned)time(NULL));
  i5_config.last_message_identifier = rand()%0xFFFF;
  i5InterfaceSearchAdd(I5_MEDIA_TYPE_BRIDGE);
  i5InterfaceSearchAdd(I5_MATCH_MEDIA_TYPE_ANY);
}

void i5GetInterfaceIDFromIfname(char *ifname, unsigned char *mac)
{
  i5_dm_device_type  *pdevice = i5DmGetSelfDevice();
  i5_dm_interface_type *pinterface = pdevice->interface_list.ll.next;

  while ( pinterface != NULL )
  {
      if (0 == strcmp(pinterface->wlParentName, ifname))
      {
          memcpy(mac, pinterface->InterfaceId, MAC_ADDR_LEN);
          break;
      }
      pinterface = pinterface->ll.next;
  }
}

void i5GetIfnameFromMacAdress(unsigned char *mac, char *ifname)
{
    i5_dm_device_type  *pdevice = i5DmGetSelfDevice();
    i5_dm_interface_type *pinterface = pdevice->interface_list.ll.next;

    if (!ifname || !mac)
        return;

    while (pinterface != NULL) {
        if (0 == memcmp(pinterface->InterfaceId, mac, MAC_ADDR_LEN)) {
            memcpy(ifname, pinterface->ifname, I5_MAX_IFNAME);
	    return;
        }
        pinterface = pinterface->ll.next;
    }
}
