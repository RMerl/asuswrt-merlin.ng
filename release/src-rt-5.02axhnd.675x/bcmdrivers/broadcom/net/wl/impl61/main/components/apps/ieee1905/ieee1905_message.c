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
 * IEEE1905 Message
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <linux/if_ether.h>
#include <wlioctl.h>
#include "ieee1905_timer.h"
#include "ieee1905_tlv.h"
#include "ieee1905_message.h"
#include "ieee1905_socket.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_interface.h"
#include "ieee1905_json.h"
#include "ieee1905_trace.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_security.h"
#include "ieee1905_flowmanager.h"
#include "ieee1905_glue.h"
#include "ieee1905_brutil.h"

#define I5_TRACE_MODULE i5TraceMessage
#define I5_MESSAGE_MAX_TLV_SIZE (I5_PACKET_BUF_LEN - ETH_HLEN - sizeof(i5_message_header_type) )

typedef void i5MessageReceiveFnType(i5_message_type *pmsg);

void i5MessageTopologyDiscoveryReceive(i5_message_type *pmsg);
void i5MessageTopologyNotificationReceive(i5_message_type *pmsg);
void i5MessageTopologyQueryReceive(i5_message_type *pmsg);
void i5MessageTopologyResponseReceive(i5_message_type *pmsg);
void i5MessageVendorSpecificReceive(i5_message_type *pmsg);
void i5MessageLinkMetricQueryReceive(i5_message_type *pmsg);
void i5MessageLinkMetricResponseReceive(i5_message_type *pmsg);
void i5MessageApAutoconfigurationSearchReceive(i5_message_type *pmsg);
void i5MessageApAutoconfigurationResponseReceive(i5_message_type *pmsg);
void i5MessageApAutoconfigurationWscReceive(i5_message_type *pmsg);
void i5MessageApAutoconfigurationRenewReceive(i5_message_type *pmsg);
void i5MessagePushButtonEventNotificationReceive(i5_message_type *pmsg);
void i5MessagePushButtonJoinNotificationReceive(i5_message_type *pmsg);
void i5MessageHigherLayerQueryReceive(i5_message_type *pmsg);
void i5MessageHigherLayerResponseReceive(i5_message_type *pmsg);
void i5MessagePowerChangeRequestReceive(i5_message_type *pmsg);
void i5MessagePowerChangeResponseReceive(i5_message_type *pmsg);
void i5MessageGenericPhyTopologyResponseSend(i5_message_type *pmsg_req);
void i5MessageGenericPhyQueryReceive(i5_message_type *pmsg);
void i5MessageGenericPhyResponseReceive(i5_message_type *pmsg);

void i5MessageBridgeDiscoverySend(i5_socket_type *psock);
void i5MessageTopologyResponseSend(i5_message_type *pmsg_req);
//void i5MessageVendorSpecificSend(i5_socket_type *psock);
void i5MessageLinkMetricQuerySend(i5_socket_type *psock, unsigned char const * destAddr,
                                  unsigned char specifyNeighbor, unsigned char const * neighbor);
//void i5MessageLinkMetricResponseSend(i5_socket_type *psock);
//void i5MessageApAutoconfigurationRenewSend(i5_socket_type *psock);
void i5MessagePushButtonEventNotificationSend();

void i5MessageTopologyQueryTimeout(void *arg);
//void i5MessageLinkMetricQueryTimeout(i5_message_type *pmsg);
//void i5MessageApAutoconfigurationSearchTimeout(i5_message_type *pmsg);

int i5MessageGetVendorSpecificTlvWithCb(i5_message_type *pmsg);
void i5MessageAddVendorSpecificTlvWithCb(i5_message_type *pmsg, unsigned char *neighbor_al_mac,
	unsigned short msg_type_with_tlv);
void i5MessageGetVendorSpecificTlv(i5_message_type *pmsg, i5_message_type **vendorSpecMsg);
void i5MessageAddVendorSpecificTlv(i5_message_type *pmsg, i5_message_type *vendorSpecMsg);

#ifdef MULTIAP
void i5Message1905AckReceive(i5_message_type *pmsg);
void i5MessageAPCapabilityQueryReceive(i5_message_type *pmsg);
void i5MessageAPCapabilityReportReceive(i5_message_type *pmsg);
void i5MessageMultiAPPolicyConfigReceive(i5_message_type *pmsg);
void i5MessageClientCapabilityQueryReceive(i5_message_type *pmsg);
void i5MessageClientCapabilityReportReceive(i5_message_type *pmsg);
void i5MessageClientSteeringRequestReceive(i5_message_type *pmsg);
void i5MessageClientSteeringBTMReportReceive(i5_message_type *pmsg);
void i5MessageClientAssociationControlRequestReceive(i5_message_type *pmsg);
void i5MessageSteeringCompletedReceive(i5_message_type *pmsg);
void i5MessageHigherLayerDataReceive(i5_message_type *pmsg);
void i5MessageChannelPreferenceQueryReceive(i5_message_type *pmsg);
void i5MessageChannelPreferenceReportReceive(i5_message_type *pmsg);
void i5MessageChannelSelectionRequestReceive(i5_message_type *pmsg);
void i5MessageChannelSelectionResponseReceive(i5_message_type *pmsg);
void i5MessageOperatingChannelReportReceive(i5_message_type *pmsg_req);
void i5MessageUnsupportedCommonReceive(i5_message_type *pmsg);
void i5MessageAPMetricsQueryReceive(i5_message_type *pmsg);
void i5MessageAPMetricsResponseReceive(i5_message_type *pmsg);
void i5MessageAssociatedSTALinkMetricsQueryReceive(i5_message_type *pmsg);
void i5MessageAssociatedSTALinkMetricsResponseReceive(i5_message_type *pmsg);
void i5MessageUnAssociatedSTALinkMetricsQueryReceive(i5_message_type *pmsg);
void i5MessageUnAssociatedSTALinkMetricsResponseReceive(i5_message_type *pmsg);
void i5MessageBeaconMetricsQueryReceive(i5_message_type *pmsg);
void i5MessageBeaconMetricsResponseReceive(i5_message_type *pmsg);
void i5MessageBackhaulSteeringRequestReceive(i5_message_type *pmsg);
void i5MessageBackhaulSteeringResponseReceive(i5_message_type *pmsg);
void i5MessageCombinedInfrastructureMetricsReceive(i5_message_type *pmsg);
int i5MessagSendUnAssociatedSTALinkMetricsQueryAck(i5_message_type *pmsg_req,
  ieee1905_unassoc_sta_link_metric_query *query);
#ifdef MULTIAP_PLUGFEST
static void i5MessagRSSIBasedSteering();
#endif /* MULTIAP_PLUGFEST */
#endif /* MULTIAP */

typedef struct i5_message_rceive_types {
	i5MessageReceiveFnType *receiveFn;
	i5_message_types_t messageType;
  char i5MessageNames[50];
} i5_message_rceive_types_t;

static i5_message_rceive_types_t i5_message_process[] = {
  {i5MessageTopologyDiscoveryReceive, i5MessageTopologyDiscoveryValue, "Topology Discovery"},
  {i5MessageTopologyNotificationReceive, i5MessageTopologyNotificationValue, "Topology Notification"},
  {i5MessageTopologyQueryReceive, i5MessageTopologyQueryValue, "Topology Query"},
  {i5MessageTopologyResponseReceive, i5MessageTopologyResponseValue, "Topology Response"},
  {i5MessageVendorSpecificReceive, i5MessageVendorSpecificValue, "Vendor Specific"},
  {i5MessageLinkMetricQueryReceive, i5MessageLinkMetricQueryValue, "Link Metric Query"},
  {i5MessageLinkMetricResponseReceive, i5MessageLinkMetricResponseValue, "Link Metric Response"},
  {i5MessageApAutoconfigurationSearchReceive, i5MessageApAutoconfigurationSearchValue, "ApAutoconfig Search"},
  {i5MessageApAutoconfigurationResponseReceive, i5MessageApAutoconfigurationResponseValue, "ApAutoconfig Response"},
  {i5MessageApAutoconfigurationWscReceive, i5MessageApAutoconfigurationWscValue, "ApAutoconfig Wsc"},
  {i5MessageApAutoconfigurationRenewReceive, i5MessageApAutoconfigurationRenewValue, "ApAutoconfig Renew"},
  {i5MessagePushButtonEventNotificationReceive, i5MessagePushButtonEventNotificationValue, "Push Button Event"},
  {i5MessagePushButtonJoinNotificationReceive, i5MessagePushButtonJoinNotificationValue, "Push Button Join"},
  {i5MessageHigherLayerQueryReceive, i5MessageHigherLayerQueryValue, "Higher Layer Query"},
  {i5MessageHigherLayerResponseReceive, i5MessageHigherLayerResponseValue, "Higher Layer Response"},
  {i5MessagePowerChangeRequestReceive, i5MessagePowerChangeRequestValue, "Power Change Request"},
  {i5MessagePowerChangeResponseReceive, i5MessagePowerChangeResponseValue, "Power Change Response"},
  {i5MessageGenericPhyQueryReceive, i5MessageGenericPhyQueryValue, "Generic PHY Query"},
  {i5MessageGenericPhyResponseReceive, i5MessageGenericPhyResponseValue, "Generic Phy Response"},
#ifdef MULTIAP
  {i5Message1905AckReceive, i5Message1905AckValue, "Acknowledgement"},
  {i5MessageAPCapabilityQueryReceive, i5MessageAPCapabilityQueryValue, "AP Capability Query"},
  {i5MessageAPCapabilityReportReceive, i5MessageAPCapabilityReportValue, "AP Capability Report"},
  {i5MessageMultiAPPolicyConfigReceive, i5MessageMultiAPPolicyConfigRequestValue, "MultiAP Policy Config Request"},
  {i5MessageChannelPreferenceQueryReceive, i5MessageChannelPreferenceQueryValue, "Channel Preference Query"},
  {i5MessageChannelPreferenceReportReceive, i5MessageChannelPreferenceReportValue, "Channel Preference Report"},
  {i5MessageChannelSelectionRequestReceive, i5MessageChannelSelectionRequestValue, "Channel Selection Request"},
  {i5MessageChannelSelectionResponseReceive, i5MessageChannelSelectionResponseValue, "Channel Selection Response"},
  {i5MessageOperatingChannelReportReceive, i5MessageOperatingChannelReportValue, "Operating Channel Report"},
  {i5MessageClientCapabilityQueryReceive, i5MessageClientCapabilityQueryValue, "Client Capability Query"},
  {i5MessageClientCapabilityReportReceive, i5MessageClientCapabilityReportValue, "Client Capability Report"},
  {i5MessageAPMetricsQueryReceive, i5MessageAPMetricsQueryValue, "AP Metrics Query"},
  {i5MessageAPMetricsResponseReceive, i5MessageAPMetricsResponseValue, "AP Metrics Response"},
  {i5MessageAssociatedSTALinkMetricsQueryReceive, i5MessageAssociatedSTALinkMetricsQueryValue, "Associated STA LinkMetrics Query"},
  {i5MessageAssociatedSTALinkMetricsResponseReceive, i5MessageAssociatedSTALinkMetricsResponseValue, "Associated STA LinkMetrics Response"},
  {i5MessageUnAssociatedSTALinkMetricsQueryReceive, i5MessageUnAssociatedSTALinkMetricsQueryValue, "UnAssociated STA LinkMetrics Query"},
  {i5MessageUnAssociatedSTALinkMetricsResponseReceive, i5MessageUnAssociatedSTALinkMetricsResponseValue, "UnAssociated STA LinkMetrics Response"},
  {i5MessageBeaconMetricsQueryReceive, i5MessageBeaconMetricsQueryValue, "Beacon Metrics Query"},
  {i5MessageBeaconMetricsResponseReceive, i5MessageBeaconMetricsResponseValue, "Beacon Metrics Response"},
  {i5MessageCombinedInfrastructureMetricsReceive, i5MessageCombinedInfrastructureMetricsValue, "Combined Infrastructure Metrics"},
  {i5MessageClientSteeringRequestReceive, i5MessageClientSteeringRequestValue, "Client Steering Request"},
  {i5MessageClientSteeringBTMReportReceive, i5MessageClientSteeringBTMReportValue, "Client Steering BTM Report"},
  {i5MessageClientAssociationControlRequestReceive, i5MessageClientAssociationControlRequestValue, "Client Association Control Request"},
  {i5MessageSteeringCompletedReceive, i5MessageSteeringCompletedValue, "Steering Completed"},
  {i5MessageHigherLayerDataReceive, i5MessageHigherLayerDataValue, "HigherLayer Data"},
  {i5MessageBackhaulSteeringRequestReceive, i5MessageBackhaulSteeringRequestValue, "Backhaul Steering Request"},
  {i5MessageBackhaulSteeringResponseReceive, i5MessageBackhaulSteeringResponseValue, "Backhaul Steering Response"},
#endif /* MULTIAP */
};

enum {
  i5MessageApSentM1_no = 0,
  i5MessageApSentM1_yes = 1,
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif // endif

#ifdef MULTIAP
extern void i5WlmGetAssociatedSTALinkMetric(i5_dm_device_type *pDeviceController,
  i5_dm_clients_type *pdmclient, ieee1905_ifr_metricrpt *metricrpt, ieee1905_vendor_data *vndr_data);
extern void i5WlmUpdateMAPMetrics(void *arg);
#endif /* MULTIAP */

static char const * getI5MessageName( unsigned int type)
{
    int i = 0;

    for (i = 0; i < ARRAY_SIZE(i5_message_process); i++) {
      if (type == i5_message_process[i].messageType) {
        return (i5_message_process[i].i5MessageNames);
      }
    }

    return "Unknown Message";
}

static i5MessageReceiveFnType *getI5MessageReceiveFn(unsigned int type)
{
    int i = 0;

    for (i = 0; i < ARRAY_SIZE(i5_message_process); i++) {
      if (type == i5_message_process[i].messageType) {
        return (i5_message_process[i].receiveFn);
      }
    }

    return NULL;
}

unsigned char I5_MULTICAST_MAC[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x13};
unsigned char LLDP_MULTICAST_MAC[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};

i5_message_type i5_message_list;

i5_message_type *i5MessageNew(void)
{
  i5_message_type *pmsg;

  if ((pmsg = (i5_message_type *)malloc(sizeof(i5_message_type))) == NULL) {
    printf("Malloc error\n");
    return NULL;
  }

  memset(pmsg, 0, sizeof(i5_message_type));
  i5LlItemAdd(NULL, &i5_message_list, pmsg);
  pmsg->ppkt = &pmsg->packet_list;

  return pmsg;
}

static void i5MessageDumpHex(i5_packet_type *ppkt, unsigned int dir, i5_socket_type *psock)
{
  int i;
  unsigned long           addr;
  unsigned int            rem;
  unsigned char          *curPtr;
  unsigned int            ifindex = psock->u.sll.sa.sll_ifindex;
  struct ethhdr          *pEthHdr;
  const char             *pMsgName;
  i5_message_header_type *phdr;
  struct timeval          tv;
  time_t                  nowtime;
  struct tm               nowtm;

  if ( 0 == i5TracePacketGetDepth() ) {
    return;
  }

  pEthHdr = (struct ethhdr *)&ppkt->pbuf[0];
  if (pEthHdr->h_proto == htons(LLDP_PROTO)) {
    pMsgName = "Bridge Discovery";
  }
  else {
   phdr = (i5_message_header_type *)&ppkt->pbuf[sizeof(struct ethhdr)];
   pMsgName = getI5MessageName(ntohs(phdr->message_type));
  }

  gettimeofday(&tv, NULL);

  nowtime = tv.tv_sec;
  localtime_r(&nowtime, &nowtm);
  i5TracePacket(dir, ifindex, "%02d:%02d:%02d.%03d %s %s: if:%s-" I5_MAC_DELIM_FMT "\n",
             nowtm.tm_hour, nowtm.tm_min, nowtm.tm_sec, (unsigned int)(tv.tv_usec/1000),
             ((dir == I5_MESSAGE_DIR_RX) ? "Received" : ((dir == I5_MESSAGE_DIR_TX) ? "Sent" : "Relayed")), pMsgName,
             psock->u.sll.ifname,  I5_MAC_PRM(psock->u.sll.mac_address));

  addr   = 0;
  curPtr = &ppkt->pbuf[0];
  while ( addr < ppkt->length )
  {
    i5TracePacket(dir, ifindex, "%08lx  ", addr);

    rem = ((ppkt->length - addr) > 16) ? 16 : (ppkt->length - addr);
    for (i = 0; i < 16; ++i) {
      if ( i < rem ) {
        i5TracePacket(dir, ifindex, "%02x ", curPtr[i]);
      }
      else {
        i5TracePacket(dir, ifindex, "   ");
      }
    }

    for (i = 0; i < rem; ++i) {
      if ( ( curPtr[i] < 32 ) || ( curPtr[i] > 126 ) ) {
        i5TracePacket (dir, ifindex, "%c", '.');
      }
      else {
        i5TracePacket(dir, ifindex, "%c", curPtr[i] );
      }
    }
    addr = addr + 16;
    curPtr = curPtr + 16;
    i5TracePacket(dir, ifindex, "\n");
  }
}

void i5MessageFree(i5_message_type *pmsg)
{
  while (pmsg->packet_list.ll.next != NULL) {
    free(((i5_packet_type *)pmsg->packet_list.ll.next)->pbuf);
    i5LlItemFree(&pmsg->packet_list, pmsg->packet_list.ll.next);
  }

  i5LlItemFree(&i5_message_list, pmsg);
}

unsigned int i5MessageSendLinkQueries(void)
{
   i5_dm_device_type *currDevice = (i5_dm_device_type *)(i5_dm_network_topology.device_list.ll.next);
   unsigned int numQueriesSent = 0;

   i5Trace("\n");

   while(currDevice) {
     /* loop through all devices that aren't self */
     if (!i5DmDeviceIsSelf(currDevice->DeviceId)) {
       /* send a link query message */
       /* TBD - could have multiple bridge sockets */
       //i5_socket_type *bridgeSocket = i5SocketFindDevSocketByType(i5_socket_type_bridge_ll);
       if ( currDevice->psock ) {
         i5MessageLinkMetricQuerySend(currDevice->psock, currDevice->DeviceId, 0, NULL);
         numQueriesSent ++;
       }
     }
     currDevice = (i5_dm_device_type *)(currDevice->ll.next);
   }
   return numQueriesSent;
}

i5_message_type *i5MessageCreate(i5_socket_type *psock, unsigned char const *dst_addr, unsigned short proto)
{
  i5_message_type *pmsg;
  i5_packet_type *ppkt;
  struct ethhdr *peh;

  if ((pmsg = i5MessageNew()) == NULL) {
    return NULL;
  }

  if ((ppkt = i5PacketNew()) == NULL) {
    i5MessageFree(pmsg);
    return NULL;
  }

  i5LlItemAdd(pmsg, pmsg->ppkt, ppkt);
  pmsg->ppkt = ppkt;

  /* set the ethernet header */
  peh = (struct ethhdr *)ppkt->pbuf;
  memcpy((void*)(peh->h_dest),   (void*)dst_addr, ETH_ALEN);
  memcpy((void*)(peh->h_source), (void*)(i5_config.i5_mac_address), ETH_ALEN);
  peh->h_proto = htons(proto);
  ppkt->length = sizeof(struct ethhdr);
  pmsg->psock = psock;

  return pmsg;
}

void i5MessageReset(i5_message_type *pmsg)
{
  pmsg->ppkt = pmsg->packet_list.ll.next;
  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
}

void i5MessageMoveOffset(i5_message_type *pmsg, unsigned int length)
{
  i5_packet_type *ppkt = pmsg->ppkt;

  ppkt->offset += length;
  if (ppkt->offset >= ppkt->length) {
    pmsg->ppkt = (i5_packet_type *)ppkt->ll.next;
    ppkt = pmsg->ppkt;
    if (ppkt) {
      ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
    }
  }
  return;
}

int i5MessageGetNextTlvType(i5_message_type *pmsg)
{
  i5_packet_type *ppkt = pmsg->ppkt;

  if ((ppkt) && (ppkt->length >= ppkt->offset + sizeof(i5_tlv_t))) {
    i5_tlv_t *ptlv_header = (i5_tlv_t *)&ppkt->pbuf[ppkt->offset];
    return ptlv_header->type;
  }
  return 0;
}

/*
 * Scan through the entire message (all packets), searching for the requested "type"
 * return  0 : found
 *        -1 : found type, could not extract value
 *        -2 : not found
 */
int i5MessageTlvExtract(i5_message_type *pmsg, unsigned int type, unsigned int *plength, unsigned char **ppvalue, char withReset)
{
  i5_tlv_t *ptlv_header;
  i5TraceInfo("Looking for type %d (%s)\n", type, i5TlvGetTlvTypeString(type));
  *plength = 0;
  *ppvalue = NULL;

  if (withReset == i5MessageTlvExtractWithReset) {
    i5MessageReset(pmsg);
  }

  while (pmsg->ppkt) {
    if (pmsg->ppkt->length >= pmsg->ppkt->offset + sizeof(i5_tlv_t)) {
      ptlv_header = (i5_tlv_t *)&pmsg->ppkt->pbuf[pmsg->ppkt->offset];
      *plength = ntohs(ptlv_header->length);
      if (i5TlvIsEndOfMessageType(ptlv_header->type)) {
        return -2;
      }
      if (ptlv_header->type == type) {
        if (pmsg->ppkt->length >= (pmsg->ppkt->offset + sizeof(i5_tlv_t) + *plength)) {
          *ppvalue = &pmsg->ppkt->pbuf[pmsg->ppkt->offset + sizeof(i5_tlv_t)];
          /* The TLV was found at full length */
          i5MessageMoveOffset(pmsg, sizeof(i5_tlv_t) + *plength);
          return 0;
        } else {
          /* The TLV *type* was found, but there weren't enough bytes in the packet */
          return -1;
        }
      }
      /* This was the wrong TLV, so move along */
      i5MessageMoveOffset(pmsg, sizeof(i5_tlv_t) + *plength);
    }
    else {
      /* There wasn't enough room in the packet for even a TLV header, so move the offset off the end of the packet */
      i5MessageMoveOffset(pmsg, sizeof(i5_tlv_t));
    }
  }
  /* We searched all the way through and didn't find the TLV */
  return -2;
}

int i5MessageAddFragment(i5_message_type *pmsg)
{
  i5_packet_type *ppkt;
  i5_message_header_type *phdr;

  if ((ppkt = i5PacketNew()) == NULL) {
    return -1;
  }

  memcpy(ppkt->pbuf, pmsg->ppkt->pbuf, sizeof(struct ethhdr) + sizeof(i5_message_header_type));
  ppkt->length = sizeof(struct ethhdr) + sizeof(i5_message_header_type);

  /* change last fragment indicator of previous packet */
  phdr = (i5_message_header_type *)&pmsg->ppkt->pbuf[sizeof(struct ethhdr)];
  phdr->last_fragment_indicator = 0;

  i5LlItemAdd(pmsg, pmsg->ppkt, ppkt);
  pmsg->ppkt = ppkt;

  phdr = (i5_message_header_type *)&ppkt->pbuf[sizeof(struct ethhdr)];
  phdr->fragment_identifier = ++pmsg->fragment_identifier_count;

  return 0;
}

int i5MessageGetPacketSpace(i5_message_type *pmsg, unsigned int *currPacketSpace, unsigned int *nextPacketSpace)
{
  if (NULL == pmsg) {
    return -1;
  }

  if (currPacketSpace) {
    *currPacketSpace = I5_PACKET_BUF_LEN - pmsg->ppkt->length;
  }
  if (nextPacketSpace) {
    *nextPacketSpace = I5_MESSAGE_MAX_TLV_SIZE; // Subtract ETH Header and start of 1905 packet
  }
  return 0;
}

int i5MessageInsertTlv(i5_message_type *pmsg, unsigned char const *buf, unsigned int len)
{
  i5_packet_type *ppkt;

  if (len > I5_PACKET_BUF_LEN) {
    return -1;
  }

  if ((pmsg->ppkt->length > ETH_ZLEN) && ((pmsg->ppkt->length + len) > I5_PACKET_BUF_LEN)) {
    if (i5MessageAddFragment(pmsg) == -1) {
      return -1;
    }
  }

  ppkt = pmsg->ppkt;
  memcpy(&ppkt->pbuf[ppkt->length], buf, len);
  ppkt->length += len;

  return 0;
}

int i5MessageSend(i5_message_type *pmsg, int relay)
{
  int rc = 0;
  i5_packet_type *ppkt = (i5_packet_type *)pmsg->packet_list.ll.next;

#ifdef MULTIAP_PLUGFEST
#ifdef MULTIAP
  /* If the backhaul type is WiFi, it must block outbound 1905 messages on Ethernet interface */
  if ((i5_config.dwds_enabled) && (strcmp(pmsg->psock->u.sll.ifname, "vlan1") == 0)) {
    return rc;
  }
#endif /* MULTIAP */
#endif /* MULTIAP_PLUGFEST */

  while (ppkt != NULL) {
     i5MessageDumpHex(ppkt, relay ? I5_MESSAGE_DIR_TX_RELAY : I5_MESSAGE_DIR_TX, pmsg->psock);
     rc |= i5InterfacePacketSend(pmsg->psock, ppkt);
     ppkt = (i5_packet_type *)ppkt->ll.next;
  }
  return rc;
}

static inline unsigned char* i5MessageGetFirstPbuf(i5_message_type *pmsg)
{
  i5_packet_type *firstPacket =  (i5_packet_type *)(pmsg->packet_list.ll.next);
  return firstPacket->pbuf;
}

unsigned short i5MessageVersionGet(i5_message_type *pmsg) {
  unsigned char* firstPbuf = i5MessageGetFirstPbuf(pmsg);
  i5_message_header_type *phdr = (i5_message_header_type *)&firstPbuf[sizeof(struct ethhdr)];
  return (phdr->message_version);
}

unsigned short i5MessageIdentifierGet(i5_message_type *pmsg) {
  unsigned char* firstPbuf = i5MessageGetFirstPbuf(pmsg);
  i5_message_header_type *phdr = (i5_message_header_type *)&firstPbuf[sizeof(struct ethhdr)];
  return (ntohs(phdr->message_identifier));
}

unsigned short i5MessageTypeGet(i5_message_type *pmsg) {
  unsigned char* firstPbuf = i5MessageGetFirstPbuf(pmsg);
  i5_message_header_type *phdr = (i5_message_header_type *)&firstPbuf[sizeof(struct ethhdr)];
  return (ntohs(phdr->message_type));
}

unsigned char *i5MessageSrcMacAddressGet(i5_message_type *pmsg) {
  unsigned char* firstPbuf = i5MessageGetFirstPbuf(pmsg);
  struct ethhdr *peh = (struct ethhdr *)firstPbuf;
  return (peh->h_source);
}

unsigned char *i5MessageDstMacAddressGet(i5_message_type *pmsg) {
  unsigned char* firstPbuf = i5MessageGetFirstPbuf(pmsg);
  struct ethhdr *peh = (struct ethhdr *)firstPbuf;
  return (peh->h_dest);
}

unsigned short i5MessageProtoGet(i5_message_type *pmsg) {
  unsigned char* firstPbuf = i5MessageGetFirstPbuf(pmsg);
  struct ethhdr *pEthHdr = (struct ethhdr *)&firstPbuf[0];
  return (ntohs(pEthHdr->h_proto));
}

unsigned char i5MessageLastPacketFragmentIdentifierGet(i5_message_type *pmsg) {
  i5_packet_type *lastPacket = (i5_packet_type *)(pmsg->packet_list.ll.next);
  i5_message_header_type *phdr;

  if ( NULL == lastPacket ) {
   return -1;
  }
  else {
    while ( lastPacket->ll.next != NULL ) {
      lastPacket = lastPacket->ll.next;
    }
  }
  phdr = (i5_message_header_type *)&lastPacket->pbuf[sizeof(struct ethhdr)];
  return phdr->fragment_identifier;
}

void i5MessageDumpMessages(void)
{
  i5_message_type *item = (i5_message_type *)i5_message_list.ll.next;

  while (item != NULL) {
    printf("%p type=%d dest: " I5_MAC_DELIM_FMT " Timer : %s \n",
      item->psock, i5MessageTypeGet(item), I5_MAC_PRM(i5MessageDstMacAddressGet(item)),
      (item->ptmr != NULL) ? "YES" : "NO");
    item = (i5_message_type *)item->ll.next;
  }
  return;
}

void i5MessageCancel(i5_socket_type *psock)
{
  unsigned char *neighbor_al_mac_address;
  i5_message_type *item = (i5_message_type *)i5_message_list.ll.next;
  i5_message_type *next;

  i5Trace("\n");
  while (item != NULL) {
    next = (i5_message_type *)item->ll.next;
    if (item->psock == psock) {
      if (item->ptmr) {
        i5TimerFree(item->ptmr);
      }
      if ((i5MessageTypeGet(item) == i5MessageTopologyQueryValue) || (i5MessageTypeGet(item) == i5MessageGenericPhyQueryValue)) {
        neighbor_al_mac_address = i5MessageDstMacAddressGet(item);
        i5DmDeviceQueryStateSet(neighbor_al_mac_address, i5DmStateNew);
      }
      i5MessageFree(item);
    }
    item = next;
  }
  return;
}

i5_message_type *i5MessageMatch(unsigned char *src_mac_addr, unsigned short message_identifier, unsigned short message_type)
{
  i5_message_type *item = (i5_message_type *)i5_message_list.ll.next;

  i5Trace("\n");
  while (item != NULL) {
    unsigned char *pmac = i5MessageSrcMacAddressGet(item);
    if ((i5MessageProtoGet(item) == I5_PROTO) &&
        (i5MessageIdentifierGet(item) == message_identifier) &&
        (i5MessageTypeGet(item) == message_type) &&
        (memcmp(src_mac_addr, pmac, ETH_ALEN) == 0) &&
        (item->ptmr != NULL)) {
      break;
    }
    item = (i5_message_type *)item->ll.next;
  }
  return (item);
}

void i5MessageRelayWaitTimeout(void *arg)
{
  i5_message_type *pmsg = (i5_message_type *)arg;
  if (pmsg) {
    if (pmsg->ptmr) {
      i5TimerFree(pmsg->ptmr);
      pmsg->ptmr = NULL;
    }
    i5MessageFree(pmsg);
  }
}

void i5MessageRelayMulticastSend(i5_message_type *pmsg, i5_socket_type const *butNotThisSocket, unsigned char *pneighbor_al_mac_address)
{
  i5_socket_type *psock;

  for (psock = (i5_socket_type *)i5_config.i5_socket_list.ll.next; psock; psock = psock->ll.next) {
    /* Send it only on ll socket, Do not send it on currently recieved socket(butNotThisSocket) */
    if ((psock->type == i5_socket_type_ll) && (psock != butNotThisSocket)) {
      /* If the butNotThisSocket is not NULL that means its relaying this message which was
       * received from other device. So, in that case do not send it on the loopback interface as
       * the device listening on the same device on loopback will get duplicate copies
       */
      if (butNotThisSocket && I5_SOCKET_IS_LOOPBACK(psock->flags)) {
        i5Trace("Not Sending Relay Multicast Message %x on %s. Flags %02x butNotThisSocket %s\n",
          i5MessageIdentifierGet(pmsg), psock->u.sll.ifname, psock->flags,
          butNotThisSocket->u.sll.ifname);
        continue;
      }
      if (i5BrUtilGetPortStpState(i5SocketGetIfName(psock)) == 0) {
        i5TraceInfo("Port not in forwarding state. Skip send\n");
        continue;
      }

      pmsg->psock = psock;
      i5Trace("Sending Relay Multicast Message %x on %s\n", i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
      i5MessageSend(pmsg, ((NULL == butNotThisSocket) ? 0 : 1));
    }
  }

  /* replace src mac with AL mac so that we can match packets as required by spec
     pneighbor_al_mac_address may be NULL for local messages which means AL MAC is already used and a copy is not needed */
  if ( pneighbor_al_mac_address ) {
    struct ethhdr *peh = (struct ethhdr *)pmsg->ppkt->pbuf;
    memcpy(peh->h_source, pneighbor_al_mac_address, MAC_ADDR_LEN);
  }

  pmsg->ptmr = i5TimerNew(I5_MESSAGE_RELAY_WAIT_TIMEOUT_MSEC, i5MessageRelayWaitTimeout, pmsg);
  if ( NULL == pmsg->ptmr ) {
    i5MessageFree(pmsg);
  }

}

int i5MessageRelayMulticastCheck(i5_message_type *pmsg, unsigned char *pmac)
{
  unsigned short message_identifier = i5MessageIdentifierGet(pmsg);
  unsigned short message_type = i5MessageTypeGet(pmsg);
  i5_message_type *pmsg_match;
  int rc = 0;

  i5Trace("Received Relay Multicast Message %x on %s\n", message_identifier, pmsg->psock->u.sll.ifname);
  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvAlMacAddressTypeExtract(pmsg, pmac);
  if (rc == 0) {
    if ((pmsg_match = i5MessageMatch(pmac, message_identifier, message_type)) != NULL) {
      // Received an existing message, don't relay it and restart the timer since the message continues to be received
      if (pmsg_match->ptmr) {
        i5TimerFree(pmsg_match->ptmr);
        pmsg_match->ptmr = NULL;
      }
      pmsg_match->ptmr = i5TimerNew(I5_MESSAGE_RELAY_WAIT_TIMEOUT_MSEC, i5MessageRelayWaitTimeout, pmsg_match);
      i5Trace("Multicast Message %x discarded on %s\n", message_identifier, pmsg->psock->u.sll.ifname);
      i5MessageFree(pmsg);
      return -1;
    }
  }
  else {
    i5MessageFree(pmsg);
    return -1;
  }

  return 0;
}

int i5MessageRawMessageSend(unsigned char *outputInterfaceMac, unsigned char *msgData, int msgLength)
{
  i5_socket_type *pifSocket = i5SocketFindDevSocketByAddr(outputInterfaceMac, NULL);

  /* there could be multiple interfaces sharing the same address
     this message will be sent to the first one found */
  if ( pifSocket ) {
    i5_packet_type ppkt;
    ppkt.length = msgLength;
    ppkt.offset = 0;
    ppkt.pbuf = msgData;
    i5MessageDumpHex(&ppkt, I5_MESSAGE_DIR_TX, pifSocket);
    i5InterfacePacketSend(pifSocket, &ppkt);
    return 0;
  }
  return -1;
}

/* return 1 if device is already being queried on this socket
 * return 0 if not
 */
int i5MessageCheckForQueryOnDeviceAndSocket(i5_socket_type *srcSock, unsigned char *srcAddr, int queryType)
{
  i5_message_type *item = (i5_message_type *)i5_message_list.ll.next;
  i5_message_type *next;

  i5Trace("Source addr: " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(srcAddr));
  while (item != NULL) {
    next = (i5_message_type *)item->ll.next;
    unsigned char *neighbor_al_mac_address = i5MessageDstMacAddressGet(item);
    if ((item->psock == srcSock) && (i5MessageTypeGet(item) == queryType)&&
        (memcmp(neighbor_al_mac_address, srcAddr, 6) == 0) && (item->ptmr != NULL)) {
      return 1;
    }
    item = next;
  }
  return 0;
}

static void i5MessageUpdateDeviceName (unsigned char * neighbor_al_mac_address, int rcVendName, char * friendlyName)
{
  i5_dm_device_type *device = i5DmDeviceFind(neighbor_al_mac_address);
  if (device != NULL) {
    if (rcVendName == 0) {
      strncpy( device->friendlyName, friendlyName, I5_DEVICE_FRIENDLY_NAME_LEN);
      i5Trace("Received Friendly Name %s\n", friendlyName);
      i5JsonDeviceUpdate(I5_JSON_ALL_CLIENTS, device);
    }
    else {
      i5GlueAssignFriendlyName(neighbor_al_mac_address, (char *)friendlyName, I5_DEVICE_FRIENDLY_NAME_LEN);
      strncpy( (char*)device->friendlyName, (char*)friendlyName, I5_DEVICE_FRIENDLY_NAME_LEN);
      i5Trace("No Friendly Name or IP\n");
    }
  }
}

void i5MessageTopologyDiscoveryReceive(i5_message_type *pmsg)
{
  unsigned char neighbor_al_mac_address[MAC_ADDR_LEN];
  unsigned char neighbor_interface_id[MAC_ADDR_LEN];
  int rc = 0;
  i5_dm_device_type *pDevice;

  i5_message_type *vendorSpecMsg = NULL;
  char friendlyName[I5_DEVICE_FRIENDLY_NAME_LEN] = "";

  i5Trace("Received Topology Discovery Message %04x on %s\n", i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  if (!I5_IS_START_MESSAGE(i5_config.flags)) {
    i5Trace("Messaging not started\n");
    goto end;
  }

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvAlMacAddressTypeExtract(pmsg, neighbor_al_mac_address);
  rc |= i5TlvMacAddressTypeExtract(pmsg, neighbor_interface_id);
  if (rc == 0) {
    if ( i5DmDeviceIsSelf(neighbor_al_mac_address) ) {
      // Received our own topology message
      i5Trace("Loopback Topology Discovery Message %x discarded on %s\n", i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
      i5MessageFree(pmsg);
      return;
    }
    /* This is the only place in the code where we allow the device timer to be created */
    i5DmRefreshDeviceTimer(neighbor_al_mac_address, 1);

    i5MessageGetVendorSpecificTlv(pmsg, &vendorSpecMsg);
    if (vendorSpecMsg != NULL) {
      i5TlvFriendlyNameExtract(vendorSpecMsg, friendlyName, sizeof(friendlyName));
      i5MessageFree(vendorSpecMsg);
    }

    pDevice = i5DmDeviceFind(neighbor_al_mac_address);
    if ( pDevice == NULL ) {
      pDevice = i5DmDeviceNew(neighbor_al_mac_address, i5MessageVersionGet(pmsg), friendlyName);
    }

    if (pDevice) {
      time(&pDevice->active_time);
    }
    i5MessageTopologyQuerySend(pmsg->psock, neighbor_al_mac_address);

    /* if a neighbor entry does not exist for this device send a topology discovery */
    pDevice = i5DmGetSelfDevice();
    if ( pDevice ) {
      if ( NULL == i5Dm1905NeighborFind(pDevice, pmsg->psock->u.sll.mac_address, neighbor_al_mac_address) ) {
        i5MessageTopologyDiscoveryTimeout(pmsg->psock);
      }
    }
    i5Dm1905NeighborUpdate(i5_config.i5_mac_address, pmsg->psock->u.sll.mac_address, neighbor_al_mac_address, neighbor_interface_id,
                           NULL, i5SocketGetIfName(pmsg->psock), i5SocketGetIfIndex(pmsg->psock), 1);
  }

end:
  i5MessageFree(pmsg);
}

void i5MessageTopologyDiscoveryTimeout(void *arg)
{
  i5_socket_type *psock = (i5_socket_type *)arg;

  if (psock->ptmr != NULL) {
    i5TimerFree(psock->ptmr);
  }

  if (!I5_IS_START_MESSAGE(i5_config.flags)) {
    i5TraceInfo("Messaging Not Started Try after %d milliseconds on " I5_MAC_DELIM_FMT "\n",
      I5_MESSAGE_START_MESSAGE_DELAY_MSEC, I5_MAC_PRM(psock->u.sll.mac_address));
    psock->u.sll.discoveryRetryPeriod = I5_MESSAGE_START_MESSAGE_DELAY_MSEC;
    psock->ptmr = i5TimerNew(psock->u.sll.discoveryRetryPeriod, i5MessageTopologyDiscoveryTimeout, psock);
    return;
  }

  if (i5BrUtilGetPortStpState(i5SocketGetIfName(psock)) != 0) {
    i5MessageTopologyDiscoverySend(psock);
    i5MessageBridgeDiscoverySend(psock->u.sll.pLldpProtoSock);
  }
  if (psock->u.sll.discoveryRetryPeriod == 0) {
    psock->u.sll.discoveryRetryPeriod = I5_MESSAGE_TOPOLOGY_DISCOVERY_RETRY_MSEC;
  }
  else if (psock->u.sll.discoveryRetryPeriod < I5_MESSAGE_TOPOLOGY_DISCOVERY_PERIOD_MSEC) {
    psock->u.sll.discoveryRetryPeriod = 4*psock->u.sll.discoveryRetryPeriod;
    if (psock->u.sll.discoveryRetryPeriod > I5_MESSAGE_TOPOLOGY_DISCOVERY_PERIOD_MSEC) {
      psock->u.sll.discoveryRetryPeriod = I5_MESSAGE_TOPOLOGY_DISCOVERY_PERIOD_MSEC;
    }
  }

#ifdef MULTIAP
  if (i5_config.discovery_timeout != 0) {
    psock->u.sll.discoveryRetryPeriod = i5_config.discovery_timeout;
  }
#endif /* MULTIAP */
  psock->ptmr = i5TimerNew(psock->u.sll.discoveryRetryPeriod, i5MessageTopologyDiscoveryTimeout, psock);
}

void i5MessageTopologyDiscoverySend(i5_socket_type *psock)
{
  i5_message_type *pmsg;
  i5_message_type *vendMsg;
  i5_dm_device_type *pdevice;

  pdevice = i5DmGetSelfDevice();
  if ( pdevice == NULL ) {
    i5TraceError("Local device does not exist\n");
  }

  pmsg = i5MessageCreate(psock, I5_MULTICAST_MAC, I5_PROTO);
  if (NULL == pmsg) {
    return;
  }
  vendMsg = i5MessageCreate(psock, I5_MULTICAST_MAC, I5_PROTO);
  if (vendMsg == NULL) {
    i5MessageFree(pmsg);
    return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Topology Discovery Message %04x on %s\n", i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageTopologyDiscoveryValue, i5_config.last_message_identifier);
  i5TlvAlMacAddressTypeInsert(pmsg);
  i5TlvMacAddressTypeInsert(pmsg, psock->u.sll.mac_address);
  i5TlvFriendlyNameInsert(vendMsg, pdevice->friendlyName);
  i5TlvEndOfMessageTypeInsert(vendMsg);
  i5MessageAddVendorSpecificTlv(pmsg, vendMsg);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
  i5MessageFree(vendMsg);
}

void i5MessageBridgeDiscoverySend(i5_socket_type *psock)
{
  i5_message_type *pmsg;

  i5Trace("Sending Bridge Discovery Message %s\n", psock->u.sll.ifname);
  pmsg = i5MessageCreate(psock, LLDP_MULTICAST_MAC, LLDP_PROTO);
  if (pmsg != NULL) {
    i5TlvLldpTypeInsert(pmsg, &i5_config.i5_mac_address[0], &psock->u.sll.mac_address[0]);
    i5MessageSend(pmsg, 0);
    i5MessageFree(pmsg);
  }
}

void i5MessageBridgeDiscoveryReceive(i5_message_type *pmsg)
{
  unsigned char neighbor1905_al_mac_address[MAC_ADDR_LEN];
  unsigned char neighbor1905_interface_id[MAC_ADDR_LEN];
  unsigned char bridgeFlag = 0;
  int rc;

  i5Trace("\n");

  /* parse the packet */
  memset(&neighbor1905_al_mac_address[0], 0, MAC_ADDR_LEN);
  memset(&neighbor1905_interface_id[0], 0, MAC_ADDR_LEN);
  rc = i5TlvLldpTypeExtract(pmsg, &neighbor1905_al_mac_address[0], &neighbor1905_interface_id[0]);
  if ( 0 == rc ) {
    i5Dm1905NeighborUpdate(i5_config.i5_mac_address, pmsg->psock->u.sll.mac_address, neighbor1905_al_mac_address, neighbor1905_interface_id,
                           &bridgeFlag, i5SocketGetIfName(pmsg->psock), i5SocketGetIfIndex(pmsg->psock), 0);
    i5MessageFree(pmsg);
  }
}

void i5MessageUpdateDeviceSock(i5_message_type *pmsg, unsigned char *pmac)
{
  i5_dm_device_type *pdevice = i5DmDeviceFind(pmac);

  if (!pdevice) {
    return;
  }
  if (!pdevice->psock) {
    pdevice->psock = pmsg->psock;
    return;
  }
  if (pdevice->psock != pmsg->psock) {
    pdevice->psock = pmsg->psock;
    if (I5_IS_MULTIAP_CONTROLLER(pdevice->flags)) {
    /* Controller socket changed - Dynamic backhaul. Send search and enable/disable bh sta
     * roaming based on the interface type on which autoconfiguration response is received
     */
      i5WlCfgMultiApControllerSearch(NULL);
    }
  }
}

void i5MessageTopologyNotificationReceive(i5_message_type *pmsg)
{
  unsigned char neighbor_al_mac_address[6];
  int rc = 0;

  i5Trace("Received Topology Notification Message on %s\n", pmsg->psock->u.sll.ifname);
  if (!I5_IS_START_MESSAGE(i5_config.flags)) {
    i5MessageFree(pmsg);
    return;
  }

  rc = i5MessageRelayMulticastCheck(pmsg, neighbor_al_mac_address);
  if (rc == 0) {
    i5MessageUpdateDeviceSock(pmsg, neighbor_al_mac_address);
#ifdef MULTIAP
    i5TlvClientAssociationEventTypeExtract(pmsg, neighbor_al_mac_address);
#endif /* MULTIAP */
    i5DmDeviceQueryStateSet(neighbor_al_mac_address, i5DmStatePending);
    i5MessageTopologyQuerySend(pmsg->psock, neighbor_al_mac_address);
    if (memcmp(i5MessageDstMacAddressGet(pmsg), I5_MULTICAST_MAC, MAC_ADDR_LEN) == 0) {
      i5MessageRelayMulticastSend(pmsg, pmsg->psock /* exclude socket */, neighbor_al_mac_address);
    } else {
      i5MessageFree(pmsg);
    }
  }
}

#ifdef MULTIAP
/* MultiAP reliable multicast sending */
static void i5MesssageReliableMultiCastSend(i5_message_type *pmsg)
{
  i5_dm_device_type *pdevice;
  struct ethhdr *peh;

  i5Trace("Sending Reliable Multicast(UNICAST) Message\n");
  for (pdevice = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
    pdevice != NULL; pdevice = pdevice->ll.next) {
    if (memcmp(pdevice->DeviceId, i5_config.i5_mac_address, MAC_ADDR_LEN) == 0 ||
      !pdevice->psock) {
      continue;
    }

    /* set the destinataion address */
    peh = (struct ethhdr *)pmsg->ppkt->pbuf;
    i5_message_header_type *phdr = (i5_message_header_type *)(peh + 1);
    memcpy((void*)(peh->h_dest), (void*)pdevice->DeviceId, MAC_ADDR_LEN);
    phdr->relay_indicator = 0;
    pmsg->psock = pdevice->psock;
    i5MessageSend(pmsg, 0);
  }
}
#endif /* MULTIAP */

void i5MessageTopologyNotificationSend(unsigned char *bssid, unsigned char *mac, unsigned char isAssoc)
{
  i5_message_type *pmsg;

  /* If the messaging is not started, dont send topology notification */
  if (!I5_IS_START_MESSAGE(i5_config.flags)) {
    return;
  }

  i5Trace("Sending Topology Notification Message\n");

  pmsg = i5MessageCreate((i5_socket_type *)i5_config.i5_socket_list.ll.next, I5_MULTICAST_MAC, I5_PROTO);
  if (NULL != pmsg) {
    i5_config.last_message_identifier++;
    i5PacketHeaderInit(pmsg->ppkt, i5MessageTopologyNotificationValue, i5_config.last_message_identifier);
    i5TlvAlMacAddressTypeInsert(pmsg);

#ifdef MULTIAP
    /* For Assoc and Disassoc notification insert Client Association Event TLV */
    if (bssid != NULL && mac != NULL) {
      i5TlvClientAssociationEventTypeInsert(pmsg, bssid, mac, isAssoc);
    }
#endif /* MULTIAP */

    i5TlvEndOfMessageTypeInsert(pmsg);

    i5MessageRelayMulticastSend(pmsg, NULL /* all sockets */, NULL);

#ifdef MULTIAP
    /* Reliability unicast */
    if (bssid != NULL && mac != NULL) {
      i5MesssageReliableMultiCastSend(pmsg);
    }
#endif /* MULTIAP */
  }
}

void i5MessageTopologyQueryReceive(i5_message_type *pmsg)
{
  i5Trace("Received Topology Query Message %04x on %s\n", i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
  if (!I5_IS_START_MESSAGE(i5_config.flags)) {
    i5MessageFree(pmsg);
    return;
  }
  i5MessageUpdateDeviceSock(pmsg, i5MessageSrcMacAddressGet(pmsg));
  i5MessageTopologyResponseSend(pmsg);
  i5MessageFree(pmsg);
}

void i5MessageTopologyQueryTimeout(void *arg)
{
  unsigned char neighbor_al_mac_address[MAC_ADDR_LEN];
  i5_socket_type *psock = NULL;
  i5_dm_device_type *deviceQueried = NULL;

  i5_message_type *pmsg = (i5_message_type *)arg;
  int queryType;

  if (!pmsg && !pmsg->psock) {
    return;
  }

  queryType = i5MessageTypeGet(pmsg);

  psock = pmsg->psock;
  i5Trace("%s Query Message Timeout %04x\n", (queryType==i5MessageGenericPhyQueryValue) ? "Generic PHY" : "Topology", i5MessageIdentifierGet(pmsg));
  if (pmsg->ptmr) {
    i5TimerFree(pmsg->ptmr);
    pmsg->ptmr = NULL;
  }

  memcpy (neighbor_al_mac_address, i5MessageDstMacAddressGet(pmsg), MAC_ADDR_LEN);
  i5MessageFree(pmsg);

  deviceQueried = i5DmDeviceFind(neighbor_al_mac_address);
  if (NULL == deviceQueried) {
    i5TraceError("%s Query Message Timer for nonexistent " I5_MAC_DELIM_FMT " \n",
                 (queryType==i5MessageGenericPhyQueryValue) ? "Generic PHY" : "Topology",
                 I5_MAC_PRM(neighbor_al_mac_address));
    return;
  }
  if (0 == deviceQueried->validated) {
    deviceQueried->numTopQueryFailures ++;
    if (deviceQueried->numTopQueryFailures >= I5_MESSAGE_TOPOLOGY_QUERY_RETRY_COUNT) {
      i5TraceError("%s Query Message Timers stopped for " I5_MAC_DELIM_FMT " after %d failures \n",
                   (queryType==i5MessageGenericPhyQueryValue) ? "Generic PHY" : "Topology",
                   I5_MAC_PRM(neighbor_al_mac_address), deviceQueried->numTopQueryFailures);
      i5DmDeviceFree(deviceQueried);
      return;
    }
  }
  if (queryType==i5MessageGenericPhyQueryValue) {
    i5MessageGenericPhyTopologyQuerySend(psock, neighbor_al_mac_address);
  }
  else {
    i5MessageTopologyQuerySend(psock, neighbor_al_mac_address);
  }

}

void i5MessageRawTopologyQuerySend (i5_socket_type *psock, unsigned char *neighbor_al_mac_address, int withRetries, int queryType)
{
  i5_message_type *pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);

  if (pmsg != NULL) {
    i5_config.last_message_identifier++;
    i5Trace("Sending Topology Query Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
            I5_MAC_PRM(neighbor_al_mac_address),
            i5_config.last_message_identifier, psock->u.sll.ifname);
    i5DmDeviceQueryStateSet(neighbor_al_mac_address, i5DmStatePending);
    i5PacketHeaderInit(pmsg->ppkt, queryType, i5_config.last_message_identifier);
    i5TlvEndOfMessageTypeInsert(pmsg);
    i5MessageSend(pmsg, 0);
    if (withRetries) {
      pmsg->ptmr = i5TimerNew(I5_MESSAGE_TOPOLOGY_QUERY_TIMEOUT_MSEC, i5MessageTopologyQueryTimeout, pmsg);
      if (!pmsg->ptmr) {
        i5Trace("Failed to create Timer for Topology Query Message to " I5_MAC_DELIM_FMT "\n",
            I5_MAC_PRM(neighbor_al_mac_address));
        i5MessageFree(pmsg);
        i5DmDeviceQueryStateSet(neighbor_al_mac_address, i5DmStateDone);
      }
    }
    else {
      i5MessageFree(pmsg);
    }
  }
}

void i5MessageGenericPhyTopologyQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address)
{
  if (i5DmDeviceQueryStateGet(neighbor_al_mac_address) == i5DmStateDone) {
    i5TraceInfo("Skipping Generic PHY Topology Query Message for device " I5_MAC_DELIM_FMT "\n",
                I5_MAC_PRM(neighbor_al_mac_address));
    return;
  }
  if (i5MessageCheckForQueryOnDeviceAndSocket(psock, neighbor_al_mac_address, i5MessageGenericPhyQueryValue) != 0) {
    i5TraceInfo("Already Doing Generic PHY topology Query Messages for device " I5_MAC_DELIM_FMT "on socket %p \n",
                I5_MAC_PRM(neighbor_al_mac_address), psock);
    return;
  }

  i5MessageRawTopologyQuerySend (psock, neighbor_al_mac_address, 1 /* with retries */, i5MessageGenericPhyQueryValue );
}

void i5MessageTopologyQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address)
{
  if (i5DmDeviceQueryStateGet(neighbor_al_mac_address) == i5DmStateDone) {
    i5TraceInfo("Skipping Topology Query Message for device " I5_MAC_DELIM_FMT "\n",
                I5_MAC_PRM(neighbor_al_mac_address));
    return;
  }
  if (i5MessageCheckForQueryOnDeviceAndSocket(psock, neighbor_al_mac_address, i5MessageTopologyQueryValue) != 0) {
    i5TraceInfo("Already Doing Topology Query Messages for device " I5_MAC_DELIM_FMT "on socket %p \n",
                I5_MAC_PRM(neighbor_al_mac_address), psock);
    return;
  }

  i5MessageRawTopologyQuerySend (psock, neighbor_al_mac_address, 1 /* with retries */, i5MessageTopologyQueryValue);
}

void i5MessageTopologyResponseReceive(i5_message_type *pmsg)
{
  unsigned short message_identifier = i5MessageIdentifierGet(pmsg);
  i5_message_type *pmsg_req;
  unsigned char neighbor_al_mac_address[MAC_ADDR_LEN];
  int rc = 0;

  i5Trace("Received Topology Response Message %04x on %s\n", message_identifier, pmsg->psock->u.sll.ifname);

  if ((pmsg_req = i5MessageMatch(i5_config.i5_mac_address, message_identifier, i5MessageTopologyQueryValue)) != NULL) {
    i5_dm_device_type *pdevice;
    unsigned char genericPhysFound = 0;
#if defined(WIRELESS)
    i5_dm_interface_type *pinterface;
    char prevWlIfPresent[i5MessageFreqBand_Reserved] = { 0 };
    char newWlIfPresent[i5MessageFreqBand_Reserved] = { 0 };
    int i;
#endif // endif
    i5_message_type *vendorSpecMsg = NULL;
    int rcVendName = -1;
    char friendlyName[I5_DEVICE_FRIENDLY_NAME_LEN] = "";

    if (pmsg_req->ptmr) {
      i5TimerFree(pmsg_req->ptmr);
      pmsg_req->ptmr = NULL;
    }
    i5MessageFree(pmsg_req);
    pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
    rc |= i5TlvDeviceInformationTypeExtractAlMac(pmsg, neighbor_al_mac_address);
    if ( 0 == rc )
    {
      i5MessageGetVendorSpecificTlv(pmsg, &vendorSpecMsg);
      if (vendorSpecMsg != NULL) {
        rcVendName = i5TlvFriendlyNameExtract(vendorSpecMsg, friendlyName, sizeof(friendlyName));
        i5MessageFree(vendorSpecMsg);
      }
    }

    pdevice = i5DmDeviceFind(neighbor_al_mac_address);
    if ( pdevice == NULL ) {
      pdevice = i5DmDeviceNew(neighbor_al_mac_address, i5MessageVersionGet(pmsg), friendlyName);
    }

    if ( pdevice == NULL ) {
      return;
    }
    i5MessageUpdateDeviceSock(pmsg, neighbor_al_mac_address);

    time(&pdevice->active_time);
#ifdef MULTIAP
    i5DmBSSClientPending(pdevice);
    /* Send AP capability query only from controller */
    if (I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
      i5MessageAPCapabilityQuerySend(pmsg->psock, neighbor_al_mac_address);
    }
#endif /* MULTIAP */

#if defined(WIRELESS)
    pinterface = pdevice->interface_list.ll.next;
    while( pinterface ) {
      unsigned int index = i5TlvGetFreqBandFromMediaType(pinterface->MediaType);
      if ( index != i5MessageFreqBand_Reserved ) {
        prevWlIfPresent[index]++;
      }
      pinterface = pinterface->ll.next;
    }
#endif // endif
    rc |= i5TlvDeviceInformationTypeExtract(pmsg, neighbor_al_mac_address, &genericPhysFound);
    rc |= i5TlvDeviceBridgingCapabilityTypeExtract(pmsg, neighbor_al_mac_address);
    rc |= i5TlvLegacyNeighborDeviceTypeExtract(pmsg, neighbor_al_mac_address);
    rc |= i5Tlv1905NeighborDeviceTypeExtract(pmsg, neighbor_al_mac_address);

#ifdef MULTIAP
    rc |= i5TlvSupportedServiceTypeExtract(pmsg, &pdevice->flags);
    rc |= i5TlvAPOperationalBSSTypeExtract(pmsg, neighbor_al_mac_address);
    rc |= i5TlvAssocaitedClientsTypeExtract(pmsg, neighbor_al_mac_address);

    /* To remove the BSS and clients not present in the topology response */
    i5DmBSSClientDone(pdevice);
#endif /* MULTIAP */

    if ( 0 == rc ) {
      i5DmDeviceQueryStateSet(neighbor_al_mac_address, i5DmStateDone);
      if ( genericPhysFound != 0 ) {
        // Send the request to the node that told us about it, not the node the message is talking about
        i5TraceInfo("Detected UNKNOWN interface.  Sending Generic Phy Request\n");
        i5DmDeviceQueryStateSet(neighbor_al_mac_address, i5DmStatePending);
        i5MessageGenericPhyTopologyQuerySend(pmsg->psock, neighbor_al_mac_address);
      }
      i5DmDeviceTopologyQuerySendToAllNew(pmsg->psock);

      i5MessageLinkMetricQuerySend(pmsg->psock,
                                   i5MessageSrcMacAddressGet(pmsg),
                                   0, /* specific neighbor */
                                   i5_config.i5_mac_address);
      i5MessageUpdateDeviceName(neighbor_al_mac_address, rcVendName, friendlyName);

      /* In the controller, update the parent device pointers for all the devices */
      if (I5_IS_MULTIAP_CONTROLLER(i5_dm_network_topology.selfDevice->flags)) {
        i5DmUpdateParentDevice();
      }

#if defined(WIRELESS)
      pinterface = pdevice->interface_list.ll.next;
      while( pinterface ) {
        unsigned int index = i5TlvGetFreqBandFromMediaType(pinterface->MediaType);
        if ( index != i5MessageFreqBand_Reserved ) {
          newWlIfPresent[index]++;
        }
        pinterface = pinterface->ll.next;
      }

      /* if a new wireless interface is present then restart AP auto configuration */
      for( i = 0; i < i5MessageFreqBand_Reserved; i++ ) {
        if ( newWlIfPresent[i] > prevWlIfPresent[i] ) {
#ifdef MULTIAP
          /* If there is no search and M1 is in process, start the M1 again */
          if (i5_config.ptmrApSearch == NULL && i5_config.ptmrWSC == NULL) {
            i5WlCfgMultiApWSCTimeout(NULL);
          }
#else
          i5WlcfgApAutoconfigurationStart(NULL);
#endif /* MULTIAP */
          break;
        }
      }
#endif // endif
    }
  }
  else {
    i5Trace("Error: Received Unsolicited Topology Response Message %04x\n", message_identifier);
  }
  i5MessageFree(pmsg);
}

void i5MessageTopologyResponseSend(i5_message_type *pmsg_req)
{
  i5_message_type *pmsg;
  i5_message_type *vendMsg;
  i5_dm_device_type *pdevice;
  char containsGenericPhy = 0;
  i5_dm_device_type *destDev;
  unsigned char useLegacyHpav = 0;

  destDev = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg_req));
  if (destDev && (destDev->nodeVersion == I5_DM_NODE_VERSION_1905) ) {
    useLegacyHpav = 1;
  }

  if (destDev) {
    destDev->psock = pmsg_req->psock;
    time(&destDev->active_time);
  }

  pdevice = i5DmGetSelfDevice();
  if ( pdevice == NULL ) {
    i5TraceError("Local device does not exist\n");
  }

  i5Trace("Sending Topology Response Message %04x on %s\n", i5MessageIdentifierGet(pmsg_req), pmsg_req->psock->u.sll.ifname);
  pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
  if (NULL == pmsg) {
    return;
  }
  vendMsg = i5MessageCreate(pmsg_req->psock, I5_MULTICAST_MAC, I5_PROTO);
  if (vendMsg == NULL) {
    i5MessageFree(pmsg);
    return;
  }

  i5PacketHeaderInit(pmsg->ppkt, i5MessageTopologyResponseValue, i5MessageIdentifierGet(pmsg_req));
  i5TlvDeviceInformationTypeInsert(pmsg, useLegacyHpav, &containsGenericPhy);
  i5TlvDeviceBridgingCapabilityTypeInsert(pmsg);
  i5TlvLegacyNeighborDeviceTypeInsert(pmsg);
  i5Tlv1905NeighborDeviceTypeInsert(pmsg);

#ifdef MULTIAP
  i5TlvSupportedServiceTypeInsert(pmsg);
  i5TlvAPOperationalBSSTypeInsert(pmsg);

  /* If it supports multi AP agent then only send Associated clients TLV */
  if (I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TlvAssocaitedClientsTypeInsert(pmsg);
  }
#endif /* MULTIAP */

  i5TlvFriendlyNameInsert(vendMsg, pdevice->friendlyName);
  i5TlvEndOfMessageTypeInsert(vendMsg);
  i5MessageAddVendorSpecificTlv(pmsg, vendMsg);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
  i5MessageFree(vendMsg);

  if ( (containsGenericPhy) && destDev && (I5_DM_NODE_VERSION_UNKNOWN == destDev->nodeVersion) ) {
    i5DmWaitForGenericPhyQuery(destDev);
  }
}

void i5MessageApAutoconfigurationSearchSend(unsigned int freqBand)
{
#if defined(WIRELESS)
    i5_message_type *pmsg;

#ifdef MULTIAP
    if (I5_IS_REGISTRAR(i5_config.flags) && (I5_IS_MULTIAP_CONTROLLER(i5_config.flags))) {
      i5TraceError("Not sending AP Autoconfiguration Search Message as this device is "
        "Controller and Registrar\n");
      return;
    }
#endif /* MULTIAP */

    i5Trace("Sending AP Autoconfiguration Search Message for Freq Band = %x\n", freqBand);

    pmsg = i5MessageCreate(NULL, I5_MULTICAST_MAC, I5_PROTO);
    if (pmsg != NULL) {
        i5_config.last_message_identifier++;
        i5PacketHeaderInit(pmsg->ppkt, i5MessageApAutoconfigurationSearchValue, i5_config.last_message_identifier);
        i5TlvAlMacAddressTypeInsert (pmsg);
        i5TlvSearchedRoleTypeInsert (pmsg);
        i5TlvAutoconfigFreqBandTypeInsert (pmsg, freqBand);

#ifdef MULTIAP
        i5TlvSupportedServiceTypeInsert(pmsg);
        i5TlvSearchedServiceTypeInsert(pmsg, I5_CONFIG_FLAG_CONTROLLER);
#endif /* MULTIAP */

        i5TlvEndOfMessageTypeInsert (pmsg);
        i5MessageRelayMulticastSend (pmsg, NULL /* all sockets */, NULL);
        if (i5_config.cbs.ap_auto_config_search_sent) {
          i5_config.cbs.ap_auto_config_search_sent();
        }
    }
#endif // endif
}

void i5MessageApAutoconfigurationRenewSend(unsigned int freqBand)
{
  i5_message_type *pmsg;

  i5Trace("Sending AP Autoconfiguration Renew Message for Freq Band = %x\n", freqBand);
  pmsg = i5MessageCreate(NULL, I5_MULTICAST_MAC, I5_PROTO);

  if (pmsg != NULL) {
    i5_config.last_message_identifier++;
    i5PacketHeaderInit(pmsg->ppkt, i5MessageApAutoconfigurationRenewValue, i5_config.last_message_identifier);
    i5TlvAlMacAddressTypeInsert(pmsg);
    i5TlvSupportedRoleTypeInsert(pmsg);
    i5TlvSupportedFreqBandTypeInsert (pmsg, freqBand);
    i5TlvEndOfMessageTypeInsert(pmsg);
    i5MessageRelayMulticastSend(pmsg, NULL /* all sockets */, NULL);
    i5MesssageReliableMultiCastSend(pmsg);
  }
}

void i5MessageApAutoconfigurationResponseSend(i5_message_type *pmsg_req, unsigned int freqBand,
  unsigned char *searcher_al_mac_address)
{
  i5_message_type *pmsg;

  i5Trace("Sending AP Autoconfiguration Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg_req), pmsg_req->psock->u.sll.ifname);
  pmsg = i5MessageCreate(pmsg_req->psock, searcher_al_mac_address, I5_PROTO);
  if (pmsg != NULL) {
    i5PacketHeaderInit(pmsg->ppkt, i5MessageApAutoconfigurationResponseValue,
      i5MessageIdentifierGet(pmsg_req));
    if (I5_IS_REGISTRAR(i5_config.flags)) {
      i5TlvSupportedRoleTypeInsert (pmsg);
    }
    i5TlvSupportedFreqBandTypeInsert (pmsg, freqBand);

#ifdef MULTIAP
    if (I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
      i5TlvSupportedServiceTypeInsert(pmsg);
    }
#endif /* MULTIAP */

    i5TlvEndOfMessageTypeInsert (pmsg);
    i5MessageSend(pmsg, 0);
    i5MessageFree(pmsg);
  }
}

#ifdef MULTIAP
/* Send WSC M1 Message */
void i5MessageApAutoconfigurationWscM1Send(i5_socket_type *psock, unsigned char *macAddr,
  unsigned char const *wscPacket, unsigned wscLen, unsigned char *radioMac)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice = NULL;
  i5_dm_interface_type *pdmif = NULL;

  i5Trace("Sending AP Autoconfiguration Wsc M1 Message on %s\n", psock->u.sll.ifname);

  pdevice = i5DmGetSelfDevice();
  if (pdevice == NULL) {
    i5TraceError("Local device not found for mac["I5_MAC_DELIM_FMT"]\n", I5_MAC_PRM(macAddr));
    return;
  }

  pdmif = i5DmInterfaceFind(pdevice, radioMac);
  if (pdmif == NULL) {
    i5TraceError("Interface["I5_MAC_DELIM_FMT"] not found\n", I5_MAC_PRM(radioMac));
    return;
  }

  pmsg = i5MessageCreate(psock, macAddr, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier ++;
  i5PacketHeaderInit(pmsg->ppkt, i5MessageApAutoconfigurationWscValue,
    i5_config.last_message_identifier);

  if (pdmif != NULL) {
    i5TlvAPRadioBasicCapabilitiesTypeInsert(pmsg, pdmif->InterfaceId,
      pdmif->ApCaps.RadioCaps.maxBSSSupported, pdmif->ApCaps.RadioCaps.List,
      pdmif->ApCaps.RadioCaps.Len);
  }

  i5TlvWscTypeInsert(pmsg, wscPacket, wscLen);
  i5TlvEndOfMessageTypeInsert (pmsg);
  i5MessageSend(pmsg, 0);
  pdmif->flags |= I5_FLAG_IFR_M1_SENT;
  i5MessageFree(pmsg);
}

/* Send WSC M2 Message */
void i5MessageApAutoconfigurationWscM2Send(i5_socket_type *psock, unsigned char *macAddr,
  unsigned char *radioMac)
{
  i5_message_type *pmsg;
  i5_wsc_m2_type *m2s;

  i5Trace("Sending AP Autoconfiguration Wsc M2 Message on %s\n", psock->u.sll.ifname);

  pmsg = i5MessageCreate(psock, macAddr, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier ++;
  i5PacketHeaderInit(pmsg->ppkt, i5MessageApAutoconfigurationWscValue,
    i5_config.last_message_identifier);

  i5TlvAPRadioIndentifierTypeInsert(pmsg, radioMac);

  m2s = (i5_wsc_m2_type*)i5_config.m2_list.ll.next;
  while (m2s != NULL) {
    i5TlvWscTypeInsert(pmsg, m2s->m2, m2s->m2_len);
    m2s = m2s->ll.next;
  }
  i5MessageAddVendorSpecificTlvWithCb(pmsg, NULL, i5MsgMultiAPGuestSsidValue);
  i5TlvEndOfMessageTypeInsert (pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}
#else

void i5MessageApAutoconfigurationWscSend(i5_socket_type *psock, unsigned char *macAddr, unsigned char const * wscPacket,
	unsigned wscLen, unsigned char *radioMac)
{
    i5_message_type *pmsg;

    i5Trace("Sending AP Autoconfiguration Wsc Message on %s\n", psock->u.sll.ifname);
    pmsg = i5MessageCreate(psock, macAddr, I5_PROTO);
    if (pmsg != NULL) {
        i5_config.last_message_identifier ++;
        i5PacketHeaderInit(pmsg->ppkt, i5MessageApAutoconfigurationWscValue, i5_config.last_message_identifier);
        i5TlvWscTypeInsert(pmsg, wscPacket, wscLen);
        i5TlvEndOfMessageTypeInsert (pmsg);
        i5MessageSend(pmsg, 0);
        i5MessageFree(pmsg);
    }
}
#endif /* MULTIAP */

void inline i5MessageGetLinkMetricsWanted(enum i5TlvLinkMetricType_Values metricTypes, char *rxWanted, char *txWanted)
{
   *rxWanted = (metricTypes == i5TlvLinkMetricType_RxOnly) || (metricTypes == i5TlvLinkMetricType_TxAndRx);
   *txWanted = (metricTypes == i5TlvLinkMetricType_TxOnly) || (metricTypes == i5TlvLinkMetricType_TxAndRx);
}

void i5MessageSetTxStats (i5_tlv_linkMetricTx_t* txStats, unsigned char const * local_interface_mac_addrs,
                          unsigned char const * neighbor_interface_mac_addrs, int numLinks)
{
  int linkIndex = 0;
  i5_dm_device_type *selfDevice = i5DmGetSelfDevice();

  i5Trace("\n");

  if (!selfDevice) {
    return;
  }

  for ( ; linkIndex < numLinks ; linkIndex ++) {
    unsigned char const * currNeighborMac = &neighbor_interface_mac_addrs[linkIndex * ETH_ALEN];
    i5_dm_1905_neighbor_type *thisNeighbor = i5Dm1905GetLocalNeighbor(currNeighborMac);
    i5_dm_interface_type *pinterface;
    i5_dm_bss_type *pbss;

    memset(&txStats[linkIndex], 0, sizeof(i5_tlv_linkMetricTx_t));

    if (!thisNeighbor) {
      return;
    }

    pinterface = i5DmInterfaceFind(selfDevice, &local_interface_mac_addrs[linkIndex * ETH_ALEN]);
    /* The interface can be virtual. In that case it will be in the BSS list */
    if (pinterface == NULL) {
      pbss = i5DmFindBSSFromDevice(selfDevice,
        (unsigned char*)&local_interface_mac_addrs[linkIndex * ETH_ALEN]);
      if (pbss) {
        pinterface = (i5_dm_interface_type*)I5LL_PARENT(pbss);
      } else {
        i5TraceInfo("SelfDev["I5_MAC_DELIM_FMT"] Local BSS["I5_MAC_DELIM_FMT"] not found\n",
          I5_MAC_PRM(selfDevice->DeviceId),
          I5_MAC_PRM(&local_interface_mac_addrs[linkIndex * ETH_ALEN]));
      }
    }

    if (pinterface) {
      txStats[linkIndex].intfType = pinterface->MediaType;
    } else {
      i5TraceInfo("SelfDev["I5_MAC_DELIM_FMT"] Local Ifr["I5_MAC_DELIM_FMT"] not found\n",
        I5_MAC_PRM(selfDevice->DeviceId),
        I5_MAC_PRM(&local_interface_mac_addrs[linkIndex * ETH_ALEN]));
    }

    memcpy(&txStats[linkIndex].localInterface, &local_interface_mac_addrs[linkIndex * ETH_ALEN], ETH_ALEN);
    memcpy(&txStats[linkIndex].neighborInterface, &neighbor_interface_mac_addrs[linkIndex * ETH_ALEN], ETH_ALEN);

#ifdef MULTIAP
    txStats[linkIndex].macThroughPutCapacity = thisNeighbor->metric.macThroughPutCapacity;
    txStats[linkIndex].linkAvailability = thisNeighbor->metric.linkAvailability;
    txStats[linkIndex].packetErrors = thisNeighbor->metric.txPacketErrors;
    txStats[linkIndex].transmittedPackets = thisNeighbor->metric.transmittedPackets;
    txStats[linkIndex].phyRate= thisNeighbor->metric.phyRate;
    i5TraceInfo("Local Ifr["I5_MAC_DELIM_FMT"] MediaType[%x] Adding Available[%d] "
      "MacThroughput[%d] Errors[%d] TxPackets[%d] Phyrate[%d]\n",
      I5_MAC_PRM(&local_interface_mac_addrs[linkIndex * ETH_ALEN]),
      pinterface ? pinterface->MediaType : 0,
      thisNeighbor->metric.linkAvailability, thisNeighbor->metric.macThroughPutCapacity,
      thisNeighbor->metric.txPacketErrors, thisNeighbor->metric.transmittedPackets,
      thisNeighbor->metric.phyRate);
#else
    txStats[linkIndex].macThroughPutCapacity = thisNeighbor->MacThroughputCapacity;
    txStats[linkIndex].linkAvailability = thisNeighbor->availableThroughputCapacity;
    i5TraceInfo("Local Ifr["I5_MAC_DELIM_FMT"] MediaType[%x] Adding %d/%d\n",
      I5_MAC_PRM(&local_interface_mac_addrs[linkIndex * ETH_ALEN]),
      pinterface ? pinterface->MediaType : 0,
      thisNeighbor->availableThroughputCapacity, thisNeighbor->MacThroughputCapacity);
#endif /* MULTIAP */

    txStats[linkIndex].ieee8021BridgeFlag = thisNeighbor->IntermediateLegacyBridge;
  }
}

void i5MessageSetRxStats (i5_tlv_linkMetricRx_t* rxStats, unsigned char const * local_interface_mac_addrs,
                          unsigned char const * neighbor_interface_mac_addrs, int numLinks)
{
  int linkIndex = 0;
  i5_dm_device_type *selfDevice = i5DmGetSelfDevice();

  i5Trace("\n");

  if (!selfDevice) {
    return;
  }

  for ( ; linkIndex < numLinks ; linkIndex ++) {
    unsigned char const *currNeighborMac = &neighbor_interface_mac_addrs[linkIndex * ETH_ALEN];
    i5_dm_1905_neighbor_type *thisNeighbor = i5Dm1905GetLocalNeighbor(currNeighborMac);
    i5_dm_interface_type *pinterface;
    i5_dm_bss_type *pbss;

    memset(&rxStats[linkIndex], 0, sizeof(i5_tlv_linkMetricTx_t));

    if (!thisNeighbor) {
      return;
    }

    pinterface = i5DmInterfaceFind(selfDevice, &local_interface_mac_addrs[linkIndex * ETH_ALEN]);
    /* The interface can be virtual. In that case it will be in the BSS list */
    if (pinterface == NULL) {
      pbss = i5DmFindBSSFromDevice(selfDevice,
        (unsigned char*)&local_interface_mac_addrs[linkIndex * ETH_ALEN]);
      if (pbss) {
        pinterface = (i5_dm_interface_type*)I5LL_PARENT(pbss);
      } else {
        i5TraceInfo("SelfDev["I5_MAC_DELIM_FMT"] Local BSS["I5_MAC_DELIM_FMT"] not found\n",
          I5_MAC_PRM(selfDevice->DeviceId),
          I5_MAC_PRM(&local_interface_mac_addrs[linkIndex * ETH_ALEN]));
      }
    }

    if (pinterface) {
      rxStats[linkIndex].intfType = pinterface->MediaType;
    } else {
      i5TraceInfo("SelfDev["I5_MAC_DELIM_FMT"] Local Ifr["I5_MAC_DELIM_FMT"] not found\n",
        I5_MAC_PRM(selfDevice->DeviceId),
        I5_MAC_PRM(&local_interface_mac_addrs[linkIndex * ETH_ALEN]));
    }

    memcpy(&rxStats[linkIndex].localInterface, &local_interface_mac_addrs[linkIndex * ETH_ALEN], ETH_ALEN);
    memcpy(&rxStats[linkIndex].neighborInterface, &neighbor_interface_mac_addrs[linkIndex * ETH_ALEN], ETH_ALEN);
#ifdef MULTIAP
    rxStats[linkIndex].packetErrors = thisNeighbor->metric.rxPacketErrors;
    rxStats[linkIndex].receivedPackets = thisNeighbor->metric.receivedPackets;
    rxStats[linkIndex].rcpi = thisNeighbor->metric.rcpi;
    i5TraceInfo("SelfDev["I5_MAC_DELIM_FMT"] Local Ifr["I5_MAC_DELIM_FMT"] MediaType[%x] "
      "Errors[%d] RxPackets[%d] RCPI[%d]\n",
      I5_MAC_PRM(selfDevice->DeviceId), I5_MAC_PRM(&local_interface_mac_addrs[linkIndex * ETH_ALEN]),
      pinterface ? pinterface->MediaType : 0, thisNeighbor->metric.rxPacketErrors,
      thisNeighbor->metric.receivedPackets, thisNeighbor->metric.rcpi);
#endif /* MULTIAP */
  }
}

/* This should be called if the Querier asked for one specific Mac address */
void i5MessageLinkMetricResponseSendOne(i5_message_type *pmsg_req, unsigned char *macaddr, enum i5TlvLinkMetricType_Values metricTypes)
{
   char rxWanted, txWanted;
   int numLinksFound = 0;
   unsigned char local_interface_mac[I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR][ETH_ALEN];
   unsigned char neighbor_interface_mac[I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR][ETH_ALEN];

   i5_message_type *pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
   if (pmsg == NULL) {
      i5Trace("Unable to allocate pmsg for link metric response");
      return;
   }

   i5MessageGetLinkMetricsWanted(metricTypes, &rxWanted, &txWanted);

   numLinksFound = i5DmGetInterfacesWithNeighbor(macaddr, local_interface_mac[0], neighbor_interface_mac[0], I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR);
   i5TraceInfo("Found %d links to neighbor " I5_MAC_DELIM_FMT "\n", numLinksFound, I5_MAC_PRM(macaddr));

   if (numLinksFound > 0) {
      /* get socket for local_interface_mac */
      i5PacketHeaderInit(pmsg->ppkt, i5MessageLinkMetricResponseValue, i5MessageIdentifierGet(pmsg_req));
      if (txWanted) {
        i5_tlv_linkMetricTx_t txStats[numLinksFound];
        i5MessageSetTxStats (txStats, local_interface_mac[0], neighbor_interface_mac[0], numLinksFound);
        i5TlvLinkMetricTxInsert(pmsg, i5_config.i5_mac_address, macaddr, txStats, numLinksFound);
      }
      if (rxWanted) {
        i5_tlv_linkMetricRx_t rxStats[numLinksFound];
        i5MessageSetRxStats(rxStats, local_interface_mac[0], neighbor_interface_mac[0], numLinksFound);
        i5TlvLinkMetricRxInsert(pmsg, i5_config.i5_mac_address, macaddr, rxStats, numLinksFound);
      }
      i5TlvEndOfMessageTypeInsert (pmsg);
      /* create Link Metric TLV */
   }
   else {
      /* create link metric result code TLV (indicates failure) */
      i5PacketHeaderInit(pmsg->ppkt, i5MessageLinkMetricResponseValue, i5MessageIdentifierGet(pmsg_req));
      i5TlvLinkMetricResultCodeInsert(pmsg);
      i5TlvEndOfMessageTypeInsert (pmsg);
   }

   /* send TLV */
   i5MessageSend(pmsg, 0);
   i5MessageFree(pmsg);
}

/* This should be called if the Querier asked for all available link metrics */
void i5MessageLinkMetricResponseSendAll(i5_message_type *pmsg_req, enum i5TlvLinkMetricType_Values metricTypes)
{
   char rxWanted, txWanted;
   i5_dm_device_type *currDevice = (i5_dm_device_type *)(i5_dm_network_topology.device_list.ll.next);
   i5_message_type *pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
   if (pmsg == NULL) {
      i5Trace("Unable to allocate pmsg for link metric response");
      return;
   }

   i5MessageGetLinkMetricsWanted(metricTypes, &rxWanted, &txWanted);
   i5PacketHeaderInit(pmsg->ppkt, i5MessageLinkMetricResponseValue, i5MessageIdentifierGet(pmsg_req));

   while(currDevice) {

     /* loop through all devices that aren't self */
     if (!i5DmDeviceIsSelf(currDevice->DeviceId)) {
       /* get interfaces for that device */
       unsigned char local_interface_mac[I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR][ETH_ALEN];
       unsigned char neighbor_interface_mac[I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR][ETH_ALEN];

       int numLinksFound = i5DmGetInterfacesWithNeighbor(currDevice->DeviceId, local_interface_mac[0], neighbor_interface_mac[0], I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR);
       i5TraceInfo("Found %d links to neighbor " I5_MAC_DELIM_FMT "\n", numLinksFound, I5_MAC_PRM(currDevice->DeviceId));
       if (txWanted) {
         i5_tlv_linkMetricTx_t txStats[numLinksFound];
         i5MessageSetTxStats (txStats, local_interface_mac[0], neighbor_interface_mac[0], numLinksFound);
         i5TlvLinkMetricTxInsert(pmsg, i5_config.i5_mac_address, currDevice->DeviceId, txStats, numLinksFound);
       }
       if (rxWanted) {
         i5_tlv_linkMetricRx_t rxStats[numLinksFound];
         i5MessageSetRxStats(rxStats, local_interface_mac[0], neighbor_interface_mac[0], numLinksFound);
         i5TlvLinkMetricRxInsert(pmsg, i5_config.i5_mac_address, currDevice->DeviceId, rxStats, numLinksFound);
       }
     }

     currDevice = (i5_dm_device_type *)(currDevice->ll.next);

   }
   i5TlvEndOfMessageTypeInsert (pmsg);

   /* send TLV */
   i5MessageSend(pmsg, 0);
   i5MessageFree(pmsg);
}

void i5MessageLinkMetricQuerySend(i5_socket_type *psock, unsigned char const * destAddr,
                                  unsigned char specifyNeighbor, unsigned char const * neighbor)
{
  i5_message_type *pmsg;

  i5Trace("Send Link Metric Query Message\n");

  if (specifyNeighbor && !neighbor) {
    i5TraceError("Neighbor not specified\n");
    return;
  }

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, destAddr, I5_PROTO);

  if (pmsg != NULL) {
   i5_config.last_message_identifier ++;
   i5PacketHeaderInit(pmsg->ppkt, i5MessageLinkMetricQueryValue, i5_config.last_message_identifier);
   i5TlvLinkMetricQueryInsert(pmsg,
                              specifyNeighbor ? i5TlvLinkMetricNeighbour_Specify : i5TlvLinkMetricNeighbour_All,
                              neighbor,
                              i5TlvLinkMetricType_TxAndRx);
   i5TlvEndOfMessageTypeInsert(pmsg);
   i5Trace("Send Link Query\n");
   i5MessageSend(pmsg, 0);
   i5MessageFree(pmsg);
  } else {
   i5Trace("Unable to allocate pmsg for link metric query.\n");
  }
}

void i5MessageLinkMetricQueryReceive(i5_message_type *pmsg)
{
  int rc = 0;
  unsigned char neighboursQueried;
  unsigned char alMacAddressQueried[MAC_ADDR_LEN];
  enum i5TlvLinkMetricType_Values linkMetricsQueried;

  i5Trace("Received Link Metric Query Message\n");
  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvLinkMetricQueryExtract(pmsg, &neighboursQueried, alMacAddressQueried , &linkMetricsQueried);

  if (rc == 0) {
#ifdef MULTIAP
    /* For multiAP get the neighbor metrics whenever requested */
    i5WlmUpdateMAPMetrics(NULL);
#endif /* MULTIAP */
    if (neighboursQueried == i5TlvLinkMetricNeighbour_All) {
      i5Trace("Asked about: All neighbours for %s\n",
         (linkMetricsQueried == i5TlvLinkMetricType_TxOnly) ? "Tx Metrics" :
         (linkMetricsQueried == i5TlvLinkMetricType_RxOnly) ? "Rx Metrics" :
            "All Metrics");
      i5MessageLinkMetricResponseSendAll(pmsg, linkMetricsQueried);
    } else {
      i5Trace("Asked about:  %02x:%02x:%02x:%02x:%02x:%02x %s\n",
         alMacAddressQueried[0],alMacAddressQueried[1],alMacAddressQueried[2],
         alMacAddressQueried[3],alMacAddressQueried[4],alMacAddressQueried[5],
         (linkMetricsQueried == i5TlvLinkMetricType_TxOnly) ? "Tx Metrics" :
         (linkMetricsQueried == i5TlvLinkMetricType_RxOnly) ? "Rx Metrics" :
            "All Metrics");
      i5MessageLinkMetricResponseSendOne(pmsg, alMacAddressQueried, linkMetricsQueried);
    }

  } else {
    i5Trace("Invalid Link Metric Message\n");
  }
  i5MessageFree(pmsg);
}

void i5MessageLinkMetricResponseReceive(i5_message_type *pmsg)
{
  i5Trace("Received Link Metric Response Message\n");

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  i5TlvLinkMetricResponseExtract(pmsg);

  i5MessageFree(pmsg);
}

void i5MessageApAutoconfigurationSearchReceive(i5_message_type *pmsg)
{
  int            rc = 0;
  unsigned char  searcher_al_mac_address[MAC_ADDR_LEN];
  unsigned short message_identifier = i5MessageIdentifierGet(pmsg);
#if defined(WIRELESS)
  unsigned char  searchedRole[i5TlvSearchRole_Length];
  unsigned char  searchedBand[i5TlvAutoConfigFreqBand_Length];

#ifdef MULTIAP
  unsigned int supportedService = 0, searchedService = 0;
#endif /* MULTIAP */

#endif /* WIRELESS */

  i5Trace("Received AP Autoconfiguration Search Message %04x on %s\n", message_identifier, pmsg->psock->u.sll.ifname);

  rc = i5MessageRelayMulticastCheck(pmsg, searcher_al_mac_address);
  if (0 == rc) {
#if defined(WIRELESS)
    rc |= i5TlvSearchedRoleTypeExtract (pmsg, searchedRole);
    rc |= i5TlvAutoconfigFreqBandTypeExtract (pmsg, searchedBand);

#ifdef MULTIAP
    rc |= i5TlvSupportedServiceTypeExtract(pmsg, &supportedService);
    rc |= i5TlvSearchedServiceTypeExtract(pmsg, &searchedService);
#endif /* MULTIAP */

    if ( (0 == rc)
#ifdef MULTIAP
    && (I5_IS_REGISTRAR(i5_config.flags)) && (I5_IS_MULTIAP_CONTROLLER(i5_config.flags))
#endif /* MULTIAP */
    )
    {
#ifdef MULTIAP
      i5WlCfgProcessAPAutoConfigSearch(pmsg, (unsigned int)searchedBand[0], searcher_al_mac_address);
#else
      i5WlcfgApAutoConfigProcessMessage(pmsg, (unsigned int)searchedBand[0], NULL, 0, NULL);//->psock, i5MessageApAutoconfigurationSearchValue
#endif /* MULTIAP */
    }
#endif /* WIRELESS */
    if (!(I5_IS_REGISTRAR(i5_config.flags) && (I5_IS_MULTIAP_CONTROLLER(i5_config.flags)))) {
      i5MessageRelayMulticastSend(pmsg, pmsg->psock /* exclude socket */, searcher_al_mac_address);
    } else {
      i5MessageFree(pmsg);
    }
  }
}

void i5MessageApAutoconfigurationResponseReceive(i5_message_type *pmsg)
{
#if defined(WIRELESS)
  unsigned char supportedRole[i5TlvSearchRole_Length];
  unsigned char supportedBand[i5TlvAutoConfigFreqBand_Length];
#ifdef MULTIAP
  unsigned int supportedServices = 0;
#endif /* MULTIAP */
  int rc = 0;
  i5_dm_device_type *pdevice;

  i5Trace("Received AP Autoconfiguration Response %04x on %s\n", i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
  pdevice = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg));

  do {
    pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
    rc |= i5TlvSupportedRoleTypeExtract(pmsg, supportedRole);
    rc |= i5TlvSupportedFreqBandTypeExtract(pmsg, supportedBand);
#ifdef MULTIAP
    rc |= i5TlvSupportedServiceTypeExtract(pmsg, &supportedServices);
#endif /* MULTIAP */

    if (rc) {
      i5TraceError("ApAutoconfigurationResponse Tlvs Missing. Try AP Auto Configuration again...\n");
      break;
    }

    if (supportedRole[0] != i5TlvRole_Registrar) {
      i5TraceError("ApAutoconfigurationResponse received from non registar. Try AP Auto Configuration again...\n");
      break;
    }

#ifdef MULTIAP
    if (!I5_IS_MULTIAP_CONTROLLER(supportedServices)) {
      i5TraceError("ApAutoconfigurationResponse received from non controller. Try AP Auto Configuration again...\n");
      break;
    }
#endif /* MULTIAP */

    /* Update registrar and supported service in the device */
    if (pdevice) {
      time(&pdevice->active_time);
      pdevice->flags |= I5_CONFIG_FLAG_REGISTRAR;
#ifdef MULTIAP
      /* Supported Service Flag */
      if (supportedServices & I5_CONFIG_FLAG_CONTROLLER) {
        pdevice->flags |= I5_CONFIG_FLAG_CONTROLLER;
      }
      if (supportedServices & I5_CONFIG_FLAG_AGENT) {
        pdevice->flags |= I5_CONFIG_FLAG_AGENT;
      }
#endif /* MULTIAP */
    } else {
      i5Trace("ApAutoconfigurationResponse received from a non-existent device. Sending topology Query\n");
      i5MessageTopologyQuerySend(pmsg->psock, i5MessageSrcMacAddressGet(pmsg));
      break;
    }

    if (i5_config.cbs.ap_auto_config_resp) {
      i5_config.cbs.ap_auto_config_resp(pdevice);
    }

#ifdef MULTIAP
    rc = i5WlCfgProcessAPAutoConfigSearch(pmsg, (unsigned int)supportedBand[0], pdevice->DeviceId);
#else
    rc = i5WlcfgApAutoConfigProcessMessage(pmsg, (unsigned int)supportedBand[0], NULL, 0, NULL);//->psock, i5MessageApAutoconfigurationResponseValue
#endif /* MULTIAP */
    if ( rc < 0 ) {
      break;
    }
  } while (0);
#endif /* WIRELESS */
  i5MessageFree(pmsg);
}

/* This function should be used if the contents of a the Vendor Specific TLV have been
   constructed using an i5_message_type.
   This function will locate the data and use the TLV code to add it to the real message

   If you are constructing the Vendor Specific TLV contents yourself, just use
   i5TlvVendorSpecificTypeInsert() to add them to your i5_message_type */
void i5MessageAddVendorSpecificTlv(i5_message_type *pmsg, i5_message_type *vendorSpecMsg)
{
   unsigned char *vendorSpec_data = NULL;

   i5Trace("Adding Data from Vendor Specific Msg as a TLV\n");

   if ((NULL == pmsg) || (NULL == vendorSpecMsg)) {
     return;
   }

   vendorSpec_data = vendorSpecMsg->ppkt->pbuf + sizeof(struct ethhdr);

   i5TlvVendorSpecificTypeInsert (pmsg, vendorSpec_data, vendorSpecMsg->ppkt->length - sizeof(struct ethhdr));
}

void i5MessageAddVendorSpecificTlvWithCb(i5_message_type *pmsg, unsigned char *neighbor_al_mac,
	unsigned short msg_type_with_tlv)
{
   ieee1905_vendor_data msg_data;

   /* Initialize Vendor data */
   memset(&msg_data, 0, sizeof(msg_data));

   i5Trace("\n");
   if (NULL == pmsg) {
     return;
   }

   /* Allocate Dynamic mem for Vendor data from Lib */
   msg_data.vendorSpec_msg = (unsigned char *)calloc(1, IEEE1905_MAX_VNDR_DATA_BUF);
   if (!msg_data.vendorSpec_msg) {
     i5TraceDirPrint("calloc error\n");
     return;
   }

   /* Fill neighbor_al_mac in Vendor data, to figure out for which device we need Vendor TLV */
   if (neighbor_al_mac) {
     memcpy(msg_data.neighbor_al_mac, neighbor_al_mac, IEEE1905_MAC_ADDR_LEN);
   }

   /* Get Vendor Specific TLV Data, for "msg_type_with_tlv" Msg, from application */
   if (i5_config.cbs.get_vendor_specific_tlv) {
     i5_config.cbs.get_vendor_specific_tlv(msg_type_with_tlv, &msg_data);
   }

   /* Insert Vendor Specific TLV Data, in Message */
   if (msg_data.vendorSpec_len > 0) {
     i5TlvVendorSpecificTypeInsert(pmsg, msg_data.vendorSpec_msg, msg_data.vendorSpec_len);
   }

   /* Free Dynamic mem for Vendor data */
   if (msg_data.vendorSpec_msg) {
     free(msg_data.vendorSpec_msg);
   }

}

/* If this function returns a pointer to vendorSpecMsg, the caller must free that msg */
void i5MessageGetVendorSpecificTlv(i5_message_type *pmsg, i5_message_type **vendorSpecMsg)
{
   i5Trace("\n");
   if (NULL == pmsg) {
     return;
   }

   /* create a local message to make processing the internal TLVs easier */
   *vendorSpecMsg = i5MessageCreate(pmsg->psock, i5_config.i5_mac_address, I5_PROTO);

   if (*vendorSpecMsg != NULL) {
     unsigned char * vendorSpec_data;
     unsigned int vendorSpec_len;
     i5PacketHeaderInit((*vendorSpecMsg)->ppkt, i5MessageVendorSpecificValue, 0);
     int rc = i5TlvVendorSpecificTypeExtract(pmsg, i5MessageTlvExtractWithReset,
      &vendorSpec_data, &vendorSpec_len);
     if ((rc == 0) && (vendorSpec_data != NULL)) {
       i5MessageInsertTlv(*vendorSpecMsg, vendorSpec_data, vendorSpec_len);
       /* At this point, vendorSpecMsg looks just like a normal message,
          with the internal TLVs placed as if they're external and
          the offset pointed to the beginning of the IEEE1905 data */
     }
     else {
       i5MessageFree(*vendorSpecMsg);
       *vendorSpecMsg = NULL;
     }
   }
}

int i5MessageGetVendorSpecificTlvWithCb(i5_message_type *pmsg)
{
  int rc = 0, count = 1;
  int vndr_tlv_found = 0;
  int vndr_tlv_processed = 0;
  ieee1905_vendor_data msg_data;

  memset(&msg_data, 0, sizeof(msg_data));

  i5Trace("\n");
  if (NULL == pmsg) {
    return -1;
  }

  memcpy(msg_data.neighbor_al_mac, i5MessageSrcMacAddressGet(pmsg), IEEE1905_MAC_ADDR_LEN);

  /* There can be multiple vendor specific TLVs. So first reset the pointer and get the
   * TLVs one by one
   */
  i5MessageReset(pmsg);

  while ((rc = i5TlvVendorSpecificTypeExtract(pmsg, i5MessageTlvExtractWithoutReset,
    &msg_data.vendorSpec_msg, &msg_data.vendorSpec_len)) == 0) {
    vndr_tlv_found++;
    if (msg_data.vendorSpec_msg == NULL) {
      break;
    }

    i5Trace("Index %d Received Vendor Specific TLV on Message %04x on %s\n",
      count, i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

    count++;
    /* Callback to process Vendor Specific Messages at Application layer */
    if (i5_config.cbs.process_vendor_specific_msg) {
      rc = i5_config.cbs.process_vendor_specific_msg(&msg_data);
      if (rc != 0) {
        i5Trace("Error in processing vendor TLV, exit \n");
	/* exit further processing */
	break;
      } else {
	vndr_tlv_processed++;
      }
    }
  }
  i5Trace(" Vendor TLV found[%d] processed[%d] \n", vndr_tlv_found, vndr_tlv_processed);
  if (vndr_tlv_found == 0) {
    /* no vendor tlv found */
    return -1;
  } else if (vndr_tlv_found == vndr_tlv_processed) {
    /* all vndr tlv processed */
    return 0;
  } else {
    return rc;
  }
}

void i5MessageVendorSpecificReceive(i5_message_type *pmsg)
{
  i5Trace("Received Vendor Specific Message\n");
  i5MessageGetVendorSpecificTlvWithCb(pmsg);
  i5MessageFree(pmsg);
}

/* Used when the data model decides it is time to generate a new routing table for client 1905 nodes */
void i5MessageSendRoutingTableMessage(i5_socket_type *psock, unsigned char const * destAddr, i5_routing_table_type *table)
{
   i5_message_type *pmsg = NULL;
   i5_message_type *vendMsg = NULL;

   i5Trace("Send Proprietary Routing Table Message\n");

   if (!psock || !destAddr || !table) {
      i5TraceError("Null pointer(s) %p %p %p!\n", psock, destAddr, table);
      return;
   }

   pmsg = i5MessageCreate(psock, destAddr, I5_PROTO);
   if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Routing Table Send.\n");
     return;
   }

   vendMsg = i5MessageCreate(psock, destAddr, I5_PROTO);
   if (vendMsg != NULL) {
     i5_config.last_message_identifier ++;
     i5PacketHeaderInit(pmsg->ppkt, i5MessageVendorSpecificValue, i5_config.last_message_identifier);

     /* Put internal TLV into fake message */
     i5Tlv_brcm_RoutingTableInsert(vendMsg, table);
     /* Copy fake message into real TLV in real message */
     i5MessageAddVendorSpecificTlv(pmsg, vendMsg);
     i5TlvEndOfMessageTypeInsert(pmsg);

     i5MessageSend(pmsg, 0);
     i5MessageFree(vendMsg);
   } else {
     i5Trace("Unable to allocate pmsg for Routing Table Send.\n");
   }
   i5MessageFree(pmsg);
}

void i5MessageApAutoconfigurationWscReceive(i5_message_type *pmsg)
{
#if defined(WIRELESS)
  unsigned char  mxMsg[2048];
  unsigned char *m2 = NULL;
  unsigned int   mxLength = 0;
  int            rc = 0;
  ieee1905_vendor_data msg_data;
#ifdef MULTIAP
  unsigned char mac[MAC_ADDR_LEN] = {0};
  ieee1905_radio_caps_type RadioCaps;
  i5_dm_device_type *pDevice;
  i5_dm_interface_type *pdmif;
#endif	/* MULTIAP */

  BCM_REFERENCE(m2);
  i5DmM2ListFree();
  memset(&RadioCaps, 0, sizeof(RadioCaps));
  memset(&msg_data, 0, sizeof(msg_data));

  do {
    /* Extract TLV */
    pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
#ifdef MULTIAP
    pDevice = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg));
    if (pDevice == NULL) {
      i5MessageTopologyQuerySend(pmsg->psock, i5MessageSrcMacAddressGet(pmsg));
      i5Trace("Reacieved WSC Message from Unknown Device["I5_MAC_DELIM_FMT"]\n",
        I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg)));
      goto end;
    }

    time(&pDevice->active_time);
    if (I5_IS_REGISTRAR(i5_config.flags)) {
      rc |= i5TlvAPRadioBasicCapabilitiesTypeExtractFromWSCM1(pmsg, mac, &RadioCaps);
      i5TraceDirPrint("Received AP Autoconfiguration WSC M1 from "I5_MAC_DELIM_FMT" for radio"
        I5_MAC_DELIM_FMT"\n",
        I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg)), I5_MAC_PRM(mac));
    } else {
      /* got M2 check if it is from valid controller or not */
      if (!I5_IS_MULTIAP_CONTROLLER(pDevice->flags)) {
        i5TraceDirPrint("Controller not found\n");
        break;
      }
      if (memcmp(pDevice->DeviceId, i5MessageSrcMacAddressGet(pmsg),
        sizeof(pDevice->DeviceId)) != 0) {
        i5TraceDirPrint("M2 got from different controller"I5_MAC_DELIM_FMT". Expected from"
          I5_MAC_DELIM_FMT"\n", I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg)),
          I5_MAC_PRM(pDevice->DeviceId));
        break;
      }
      rc |= i5TlvAPRadioIndentifierTypeExtract(pmsg, mac);
      i5TraceDirPrint("Received AP Autoconfiguration WSC M2 from "I5_MAC_DELIM_FMT" for radio"
        I5_MAC_DELIM_FMT"\n",
        I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg)), I5_MAC_PRM(mac));
    }
#endif /* MULTIAP */
    if (I5_IS_REGISTRAR(i5_config.flags)) {
      rc |= i5TlvWscTypeExtract(pmsg, mxMsg, sizeof(mxMsg), &mxLength);
      m2 = &mxMsg[0];
    } else {
      rc |= i5TlvVendorGuestSsidExtract(pmsg, &msg_data);
      rc |= i5TlvWscTypeM2Extract(pmsg);
      if (i5_config.m2_count > 0) {
        m2 = ((i5_wsc_m2_type*)i5_config.m2_list.ll.next)->m2;
        mxLength = ((i5_wsc_m2_type*)i5_config.m2_list.ll.next)->m2_len;
      }
    }
    if (rc) {
      i5TraceError("Unable to extract WSC TLV\n");
      break;
    }

#ifdef MULTIAP
      if (I5_IS_REGISTRAR(i5_config.flags)) {
        i5WlCfgProcessAPAutoConfigWSCM1(pmsg, pDevice, &mxMsg[0], mxLength, mac, &RadioCaps);
        /* If interface is found, copy the radio caps */
        if ((pdmif = i5DmInterfaceFind(pDevice, mac)) != NULL) {
          i5DmCopyRadioCaps(&pdmif->ApCaps.RadioCaps, &RadioCaps);
        }
      } else {
        i5WlCfgProcessAPAutoConfigWSCM2(pmsg, mac, &msg_data);
      }
#else
      i5WlcfgApAutoConfigProcessMessage(pmsg, -1, m2, mxLength, mac);
#endif /* MULTIAP */
  } while (0);
#endif /* WIRELESS */

end:
  i5DmFreeRadioCaps(&RadioCaps);
  i5DmM2ListFree();
  i5MessageFree(pmsg);
}

void i5MessageApAutoconfigurationRenewReceive(i5_message_type *pmsg)
{
#if defined(WIRELESS)
  unsigned char neighbor_al_mac_address[6];
  unsigned char supportedRole[i5TlvSearchRole_Length];
  unsigned char supportedBand[i5TlvAutoConfigFreqBand_Length];
  int rc = 0;

  i5Trace("Received AP Autoconfiguration Renew Message on %s\n", pmsg->psock->u.sll.ifname);

  rc = i5MessageRelayMulticastCheck(pmsg, neighbor_al_mac_address);
  if (rc == 0) {
    pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
    rc |= i5TlvSupportedRoleTypeExtract(pmsg, supportedRole);
    rc |= i5TlvSupportedFreqBandTypeExtract(pmsg, supportedBand);
    if (rc == 0) {
      i5WlcfgApAutoconfigurationRenewProcess(pmsg, (unsigned int)supportedBand[0],
        neighbor_al_mac_address);
    }
    i5MessageRelayMulticastSend(pmsg, pmsg->psock /* exclude socket */, neighbor_al_mac_address);
    /* Return after sending relayed multicast. because we should not do i5MessageFree */
    return;
  }
#endif // endif
  i5MessageFree(pmsg);
}

void i5MessagePushButtonEventNotificationReceive(i5_message_type *pmsg)
{
  int             rc;
  unsigned char   neighbor_al_mac_address[6];
  unsigned int    mediaCount;
  unsigned short *pMediaList = NULL;
  unsigned char  *pPhyMediaList = NULL;

  i5Trace("Received i5Message Push Button Event Notification Message\n");

  rc = i5MessageRelayMulticastCheck(pmsg, neighbor_al_mac_address);
  if ( 0 == rc ) {
    /* extract mediatype information */
    i5TlvPushButtonEventNotificationTypeExtract(pmsg, &mediaCount, &pMediaList);
    if (mediaCount) {
      i5SecurityProcessExternalPushButtonEvent(mediaCount, pMediaList);
    }
    i5TlvPushButtonEventNotificationTypeExtractFree(pMediaList);
    i5TlvPushButtonGenericPhyEventNotificationTypeExtract(pmsg, &mediaCount, &pPhyMediaList);
    if (mediaCount) {
      i5SecurityProcessGenericPhyExternalPushButtonEvent(mediaCount, pPhyMediaList);
    }
    i5TlvPushButtonGenericPhyEventNotificationTypeExtractFree(pPhyMediaList);
    i5MessageRelayMulticastSend(pmsg, pmsg->psock /* exclude socket */, neighbor_al_mac_address);
  }
}

void i5MessagePushButtonEventNotificationSend( )
{
  i5_message_type *pmsg;

  pmsg = i5MessageCreate(NULL, I5_MULTICAST_MAC, I5_PROTO);
  if (pmsg != NULL) {
    unsigned char genericPhyIncluded = 0;
    i5_config.last_message_identifier++;
    i5Trace("Sending i5Message Push Button Event Notification Message %04x\n", i5_config.last_message_identifier);
    i5PacketHeaderInit(pmsg->ppkt, i5MessagePushButtonEventNotificationValue, i5_config.last_message_identifier);
    i5TlvAlMacAddressTypeInsert(pmsg);
    i5TlvPushButtonEventNotificationTypeInsert(pmsg, &genericPhyIncluded);
    if (genericPhyIncluded) {
      i5TlvPushButtonGenericPhyEventNotificationTypeInsert(pmsg);
    }
    i5TlvEndOfMessageTypeInsert (pmsg);
    i5MessageRelayMulticastSend (pmsg, NULL /* all sockets */, NULL);
  }
}

void i5MessagePushButtonJoinNotificationReceive(i5_message_type *pmsg)
{
  (void) pmsg;
  i5Trace("Message not handled\n");
}

void i5MessageHigherLayerQueryReceive(i5_message_type *pmsg)
{
  (void) pmsg;
  i5Trace("Message not handled\n");
}

void i5MessageHigherLayerResponseReceive(i5_message_type *pmsg)
{
  (void) pmsg;
  i5Trace("Message not handled\n");
}

void i5MessagePowerChangeRequestReceive(i5_message_type *pmsg)
{
  (void) pmsg;
  i5Trace("Message not handled\n");
}

void i5MessagePowerChangeResponseReceive(i5_message_type *pmsg)
{
  (void) pmsg;
  i5Trace("Message not handled\n");
}

void i5MessageGenericPhyTopologyResponseSend(i5_message_type *pmsg_req)
{
   i5Trace("Send Generic Phy Topology Response\n");

   i5_message_type *pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
   if (pmsg == NULL) {
      i5Trace("Unable to allocate pmsg for link metric response");
      return;
   }

   i5PacketHeaderInit(pmsg->ppkt, i5MessageGenericPhyResponseValue, i5MessageIdentifierGet(pmsg_req));
   i5TlvGenericPhyTypeInsert (pmsg);
   i5TlvEndOfMessageTypeInsert (pmsg);

   /* send TLV */
   i5MessageSend(pmsg, 0);
   i5MessageFree(pmsg);
}

void i5MessageGenericPhyQueryReceive(i5_message_type *pmsg)
{
  i5_dm_device_type *destDev = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg));
  i5Trace("Rx'd Generic PHY query\n");

  if (destDev) {
    destDev->nodeVersion = I5_DM_NODE_VERSION_19051A;
    if (destDev->nodeVersionTimer) {
      i5TimerFree(destDev->nodeVersionTimer);
    }
  }
  i5MessageGenericPhyTopologyResponseSend(pmsg);
  i5MessageFree(pmsg);
}

void i5MessageGenericPhyResponseReceive(i5_message_type *pmsg)
{
  i5_dm_device_type *destDev = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg));

  i5Trace("Rx'd Generic PHY Response\n");
  if (destDev) {
    destDev->nodeVersion = I5_DM_NODE_VERSION_19051A;
    if (destDev->nodeVersionTimer) {
      i5TimerFree(destDev->nodeVersionTimer);
    }
  }
  i5TlvGenericPhyTypeExtract(pmsg);
  i5MessageFree(pmsg);
}

void i5MessageFragmentTimeout(void *arg)
{
  i5_message_type *pmsg = (i5_message_type *)arg;
  printf("Error: Timeout receiving all fragments\n");
  if (pmsg) {
    if (pmsg->ptmr) {
      i5TimerFree(pmsg->ptmr);
      pmsg->ptmr = NULL;
    }
    i5MessageFree(pmsg);
  }
}

void i5MessagePacketReceive(i5_socket_type *psock, i5_packet_type *ppkt)
{
  struct ethhdr *peh;
  i5_message_header_type *phdr;
  unsigned short message_type, message_identifier;
  i5_message_type *pmsg;
  unsigned char knownPacket = 1;
  i5MessageReceiveFnType *messageReceiveFnPtr = NULL;

  i5TraceInfo("\n");

  if (ppkt->length < sizeof(i5_message_header_type)) {
    printf("Received Invalid Packet Length %d\n", ppkt->length);
    i5PacketFree(ppkt);
    return;
  }

  peh = (struct ethhdr *)ppkt->pbuf;
  if ( i5DmDeviceIsSelf(peh->h_source) ) {
    i5TraceInfo("Received packet using local MAC as source: proto 0x%04x, if %s " I5_MAC_DELIM_FMT "\n"
                 , ntohs(peh->h_proto), psock->u.sll.ifname, I5_MAC_PRM(peh->h_source) );
    i5PacketFree(ppkt);
    return;
  }

  if ((memcmp(peh->h_dest, i5_config.i5_mac_address, MAC_ADDR_LEN) != 0) &&
    (memcmp(peh->h_dest, I5_MULTICAST_MAC, MAC_ADDR_LEN) != 0) &&
    (memcmp(peh->h_dest, LLDP_MULTICAST_MAC, MAC_ADDR_LEN) != 0)) {
    i5TraceInfo("Received packet for different destination : proto 0x%04x, if %s "
      I5_MAC_DELIM_FMT "\n",
      ntohs(peh->h_proto), psock->u.sll.ifname, I5_MAC_PRM(peh->h_dest) );
    i5PacketFree(ppkt);
    return;
  }

#ifdef MULTIAP_PLUGFEST
#ifdef MULTIAP
  /* If the backhaul type is WiFi, it must block inbound 1905 messages on Ethernet interface */
  if ((i5_config.dwds_enabled) && (strcmp(psock->u.sll.ifname, "vlan1") == 0)) {
    i5TraceInfo("Received packet on ethernet source: proto 0x%04x, if %s " I5_MAC_DELIM_FMT "\n"
                 , ntohs(peh->h_proto), psock->u.sll.ifname, I5_MAC_PRM(peh->h_source) );
    return;
  }
#endif /* MULTIAP */
#endif /* MULTIAP_PLUGFEST */

  if (ntohs(peh->h_proto) == LLDP_PROTO) {
    if ((pmsg = i5MessageNew()) == NULL) {
      i5PacketFree(ppkt);
      return;
    }
    i5LlItemAdd(pmsg, pmsg->ppkt, ppkt);
    pmsg->psock = psock;
    pmsg->ppkt = ppkt;
    i5MessageDumpHex(ppkt, I5_MESSAGE_DIR_RX, psock);
    i5MessageBridgeDiscoveryReceive(pmsg);
  }
  else {
    if (ntohs(peh->h_proto) != I5_PROTO) {
      i5TraceInfo("Received Invalid Protocol 0x%04x, if %s\n", ntohs(peh->h_proto), psock->u.sll.ifname);
      i5PacketFree(ppkt);
      return;
    }

    phdr = (i5_message_header_type *)&ppkt->pbuf[sizeof(struct ethhdr)];
    message_type = ntohs(phdr->message_type);
    message_identifier = ntohs(phdr->message_identifier);

    if (phdr->message_version != I5_MESSAGE_VERSION) {
      i5TraceError("Received Invalid Version 0x%02x\n", phdr->message_version);
      knownPacket = 0;
    }
    else if ((messageReceiveFnPtr = getI5MessageReceiveFn(message_type)) == NULL) {
      i5TraceError("Received Invalid Message Type 0x%04x\n", message_type);
      knownPacket = 0;
    }

    if (!knownPacket) {
      if (phdr->relay_indicator) {
        i5TraceInfo("Possibly relaying unknown message\n");
        pmsg = i5MessageNew();
        if (pmsg == NULL) {
          i5PacketFree(ppkt);
          return;
        }
        i5LlItemAdd(pmsg, pmsg->ppkt, ppkt);
        pmsg->psock = psock;
        pmsg->ppkt = ppkt;
        if (i5MessageMatch(peh->h_source, message_identifier, message_type) == NULL) {
          i5TraceInfo("RELAYING!\n");
          i5MessageRelayMulticastSend(pmsg, pmsg->psock, peh->h_source);
        }
        else {
          i5TraceInfo("NOT RELAYING ==-\n");
          i5PacketFree(ppkt);
        }
      }
      else {
        i5TraceInfo("dropping unknown message\n");
        i5PacketFree(ppkt);
      }
      i5MessageDumpHex(ppkt, I5_MESSAGE_DIR_RX, psock);
      return;
    }

    i5TraceInfo("Received I5PROTO - %s on %s (frag%d/%02x)\n", getI5MessageName(message_type), psock->u.sll.ifname, phdr->fragment_identifier, phdr->indicators);

    if (phdr->fragment_identifier == 0) {
      pmsg = i5MessageNew();
      if (pmsg == NULL) {
        i5PacketFree(ppkt);
        return;
      }
      i5LlItemAdd(pmsg, pmsg->ppkt, ppkt);
      pmsg->psock = psock;
      pmsg->ppkt = ppkt;
      i5MessageDumpHex(ppkt, I5_MESSAGE_DIR_RX, psock);
    }
    else {
      if ((pmsg = i5MessageMatch(peh->h_source, message_identifier, message_type)) == NULL) {
        printf("Error: Received a fragment with no previous message identifier 0x%04x\n", message_identifier);
        i5PacketFree(ppkt);
        return;
      }

      // Cancel the timer
      if (pmsg->ptmr) {
        i5TimerFree(pmsg->ptmr);
        pmsg->ptmr = NULL;
      }

      if (i5MessageLastPacketFragmentIdentifierGet(pmsg) != (phdr->fragment_identifier - 1)) {
        i5MessageDumpHex(ppkt, I5_MESSAGE_DIR_RX, psock);
        printf("Error: Received an out-of-order fragment id %d\n", phdr->fragment_identifier);
        i5PacketFree(ppkt);
        i5MessageFree(pmsg);
        return;
      }
      i5LlItemAdd(pmsg, pmsg->ppkt, ppkt);
      pmsg->ppkt = ppkt;
      i5MessageDumpHex(ppkt, I5_MESSAGE_DIR_RX, psock);
    }

    if (phdr->last_fragment_indicator == 1) {
      i5MessageReset(pmsg);
      messageReceiveFnPtr(pmsg);
    } else {
      // Allow a certain time to receive all fragments
      pmsg->ptmr = i5TimerNew(I5_MESSAGE_FRAGMENT_TIMEOUT_MSEC, i5MessageFragmentTimeout, pmsg);
    }
  }

  return;
}

void i5MessageDeinit( void )
{
   i5_message_type *pmsg;
   while (i5_message_list.ll.next != NULL ) {
     pmsg = i5_message_list.ll.next;
     if ( pmsg->ptmr ) {
       i5TimerFree(pmsg->ptmr);
     }
     i5MessageFree(pmsg);
   }
}

#ifdef MULTIAP
/* Send Ap Capability Query message to a Multi AP Devcice */
void i5MessageAPCapabilityQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg != NULL) {
    i5_config.last_message_identifier++;
    i5Trace("Sending AP Capability Query Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
            I5_MAC_PRM(neighbor_al_mac_address),
            i5_config.last_message_identifier, psock->u.sll.ifname);

    i5PacketHeaderInit(pmsg->ppkt, i5MessageAPCapabilityQueryValue, i5_config.last_message_identifier);
    i5TlvEndOfMessageTypeInsert(pmsg);
    i5MessageSend(pmsg, 0);
    i5MessageFree(pmsg);
  }
}

/* Receive AP Capability Query Message */
void i5MessageAPCapabilityQueryReceive(i5_message_type *pmsg)
{
  unsigned short message_identifier = i5MessageIdentifierGet(pmsg);
  i5Trace("Received Client Capability Query Message %04x on %s\n", message_identifier, pmsg->psock->u.sll.ifname);

  /* Dont respond if you are not agent */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    goto end;
  }

  i5MessageAPCapabilityReportSend(pmsg);

end:
  i5MessageFree(pmsg);
}

/* Send AP Capability Report Message */
void i5MessageAPCapabilityReportSend(i5_message_type *pmsg_req)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;

  pdevice = i5DmGetSelfDevice();
  if ( pdevice == NULL ) {
    i5TraceError("Local device does not exist\n");
  }

  i5Trace("Sending AP Capability Report Message %04x on %s\n", i5MessageIdentifierGet(pmsg_req), pmsg_req->psock->u.sll.ifname);
  pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
  if (NULL == pmsg) {
    return;
  }

  i5PacketHeaderInit(pmsg->ppkt, i5MessageAPCapabilityReportValue, i5MessageIdentifierGet(pmsg_req));

  pdmif = (i5_dm_interface_type *)pdevice->interface_list.ll.next;

  i5TlvAPCapabilitiesTypeInsert(pmsg, pdevice->BasicCaps);

  while (pdmif != NULL) {
    if (i5DmIsInterfaceWireless(pdmif->MediaType)) {
      /* Update the AP capability */
      if (pdmif->mapFlags & IEEE1905_MAP_FLAG_STA) {
        /* Update the AP caps from any of the virtual BSS as primary interface is a STA */
        pdmbss = pdmif->bss_list.ll.next;
        if (pdmbss != NULL) {
          i5DmUpdateAPCaps(pdmbss->ifname, pdmif);
        } else {
          /* In case of dedicated backhaul. there wont be virtual BSS */
          i5DmUpdateAPCaps(pdmif->ifname, pdmif);
        }
      } else {
        i5DmUpdateAPCaps(pdmif->ifname, pdmif);
      }

      i5TlvAPRadioBasicCapabilitiesTypeInsert(pmsg, pdmif->InterfaceId,
        pdmif->ApCaps.RadioCaps.maxBSSSupported, pdmif->ApCaps.RadioCaps.List,
        pdmif->ApCaps.RadioCaps.Len);
      i5TlvAPHTCapabilitiesTypeInsert(pmsg, pdmif->InterfaceId, pdmif->ApCaps.HTCaps);
      if (pdmif->ApCaps.VHTCaps.Valid) {
        i5TlvAPVHTCapabilitiesTypeInsert(pmsg, pdmif->InterfaceId, pdmif->ApCaps.VHTCaps.TxMCSMap,
          pdmif->ApCaps.VHTCaps.RxMCSMap, pdmif->ApCaps.VHTCaps.CapsEx, pdmif->ApCaps.VHTCaps.Caps);
      }
      if (pdmif->ApCaps.HECaps.Valid) {
        i5TlvAPHECapabilitiesTypeInsert(pmsg, pdmif->InterfaceId, &pdmif->ApCaps.HECaps);
      }
    }
    pdmif = pdmif->ll.next;
  }

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive AP Capability Report Message */
void i5MessageAPCapabilityReportReceive(i5_message_type *pmsg)
{
  int rc = 0;
  i5_dm_device_type *pdevice;

  i5Trace("Received  AP Capability Report Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  pdevice = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg));
  if (pdevice == NULL) {
    i5TraceError("Neighbour device does not exist\n");
    return;
  }

  rc |= i5TlvAPCapabilitiesTypeExtract(pmsg, pdevice);
  rc |= i5TlvAPRadioBasicCapabilitiesTypeExtract(pmsg, pdevice);
  rc |= i5TlvAPHTCapabilitiesTypeExtract(pmsg, pdevice);
  rc |= i5TlvAPVHTCapabilitiesTypeExtract(pmsg, pdevice);
  rc |= i5TlvAPHECapabilitiesTypeExtract(pmsg, pdevice);
  i5MessageFree(pmsg);
}

/* Send Multi AP policy configuration message to a device */
void i5MessageMultiAPPolicyConfigRequestSend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg != NULL) {
    i5_config.last_message_identifier++;
    i5Trace("Sending Multi AP Policy Config Request Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
            I5_MAC_PRM(neighbor_al_mac_address),
            i5_config.last_message_identifier, psock->u.sll.ifname);

    i5PacketHeaderInit(pmsg->ppkt, i5MessageMultiAPPolicyConfigRequestValue, i5_config.last_message_identifier);
    i5TlvSteeringPolicyTypeInsert(pmsg, neighbor_al_mac_address, &i5_config.policyConfig);
    i5TlvMetricReportingPolicyTypeInsert(pmsg, neighbor_al_mac_address, &i5_config.policyConfig);

    i5Trace("Sending Vendor Specific TLV in AP Policy Config Request Message "
        "to " I5_MAC_DELIM_FMT " %04x on %s\n",
        I5_MAC_PRM(neighbor_al_mac_address), i5_config.last_message_identifier, psock->u.sll.ifname);
    i5MessageAddVendorSpecificTlvWithCb(pmsg, neighbor_al_mac_address, i5MsgMultiAPPolicyConfigRequestValue);

    i5TlvEndOfMessageTypeInsert(pmsg);
    i5MessageSend(pmsg, 0);
    i5MessageFree(pmsg);
  }
}

/* Receive Multi AP policy configuration message */
void i5MessageMultiAPPolicyConfigReceive(i5_message_type *pmsg)
{
  unsigned short policy_flag = 0;
  int rc = 0;

  ieee1905_vendor_data msg_data;
  memset(&msg_data, 0, sizeof(msg_data));

  i5Trace("Received  Multi AP Policy Config Request Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  i5Message1905AckSend(pmsg);

  rc = i5TlvSteeringPolicyTypeExtract(pmsg);
  if (rc == 0) {
    policy_flag |= MAP_POLICY_RCVD_FLAG_STEER;
  }
  rc = i5TlvMetricReportingPolicyTypeExtract(pmsg);
  if (rc == 0) {
    policy_flag |= MAP_POLICY_RCVD_FLAG_METRIC_REPORT;
  }

  if (policy_flag & MAP_POLICY_RCVD_FLAG_METRIC_REPORT) {

    rc = i5TlvVendorSpecificTypeExtract(pmsg, i5MessageTlvExtractWithReset,
      &msg_data.vendorSpec_msg, &msg_data.vendorSpec_len);
    if ((rc == 0) && (msg_data.vendorSpec_msg != NULL)) {

      i5Trace("Received Vendor Specific TLV in Policy Config Request Message %04x on %s\n",
        i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
    }
  }

#ifdef MULTIAP_PLUGFEST
  /* For Multi-AP agent initiated RSSI based steering */
  if ((policy_flag & MAP_POLICY_RCVD_FLAG_STEER) && I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5MessagRSSIBasedSteering();
  }
#endif /* MULTIAP_PLUGFEST */

  /* Process the AP Metric Reporting Policy */
  if ((policy_flag & MAP_POLICY_RCVD_FLAG_METRIC_REPORT) && I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5DmProcessAPMetricReportingPolicy();
  }

  if (i5_config.cbs.configure_policy) {
    i5_config.cbs.configure_policy(&i5_config.policyConfig, policy_flag, &msg_data);
  }

  i5MessageFree(pmsg);
}

/* Send Client Capability Query message to a Multi AP Devcice */
void i5MessageClientCapabilityQuerySend (i5_socket_type *psock, unsigned char *neighbor_al_mac_address, unsigned char *mac, unsigned char *bssid)
{
  i5_message_type *pmsg;

  if (i5DmIsMacNull(neighbor_al_mac_address) || i5DmIsMacNull(mac) || i5DmIsMacNull(bssid)) {
    i5TraceError("Empty MAC\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg != NULL) {
    i5_config.last_message_identifier++;
    i5Trace("Sending Client Capability Query Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
            I5_MAC_PRM(neighbor_al_mac_address),
            i5_config.last_message_identifier, psock->u.sll.ifname);

    i5PacketHeaderInit(pmsg->ppkt, i5MessageClientCapabilityQueryValue, i5_config.last_message_identifier);
    i5TlvClientInfoTypeInsert(pmsg, mac, bssid);
    i5TlvEndOfMessageTypeInsert(pmsg);
    i5MessageSend(pmsg, 0);
    i5MessageFree(pmsg);
  }
}

void i5MessageClientCapabilityQueryReceive(i5_message_type *pmsg)
{
  unsigned short message_identifier = i5MessageIdentifierGet(pmsg);
  unsigned char mac[MAC_ADDR_LEN] = {0}, bssid[MAC_ADDR_LEN] = {0};
  int rc = 0;

  i5Trace("Received Client Capability Query Message %04x on %s\n", message_identifier, pmsg->psock->u.sll.ifname);

  /* Dont respond if you are not agent */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    goto end;
  }

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvClientInfoTypeExtract(pmsg, mac, bssid);
  if (rc != 0) {
    i5Trace("Failed to parse TLV of Message %04x\n", message_identifier);
    goto end;
  }

  i5MessageClientCapabilityReportSend(pmsg, mac, bssid, rc);

end:
  i5MessageFree(pmsg);
}

/* Send Client Capability Report Message */
void i5MessageClientCapabilityReportSend(i5_message_type *pmsg_req, unsigned char *mac,
  unsigned char *bssid, int rc)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice;
  i5_dm_bss_type *pbss;
  i5_dm_clients_type *pclient;
  i5TlvClientCapabilityReporResultCode_Values_t res = i5TlvClientCapabilityReporResultCode_Success;
  ieee1905_tlv_err_codes_t err = ieee1905_tlv_err_reserved;
  unsigned char *frame = NULL;
  unsigned short frame_len = 0;

  pdevice = i5DmGetSelfDevice();
  if ( pdevice == NULL ) {
    i5TraceError("Local device does not exist\n");
    return;
  }

  i5Trace("Sending Client Capability Report Message %04x on %s\n", i5MessageIdentifierGet(pmsg_req),
    pmsg_req->psock->u.sll.ifname);
  pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
  if (NULL == pmsg) {
    return;
  }

  i5PacketHeaderInit(pmsg->ppkt, i5MessageClientCapabilityReportValue,
    i5MessageIdentifierGet(pmsg_req));
  i5TlvClientInfoTypeInsert(pmsg, mac, bssid);
  if (rc != 0) {
    res = i5TlvClientCapabilityReporResultCode_Failure;
    err = ieee1905_tlv_err_client_cap_report_error;
  } else {
    pbss = i5DmFindBSSFromDevice(pdevice, bssid);
    if (pbss == NULL) {
      res = i5TlvClientCapabilityReporResultCode_Failure;
      err = ieee1905_tlv_err_sta_not_associated;
    } else {
      pclient  = i5DmFindClientInBSS(pbss, mac);
      if (pclient == NULL) {
        res = i5TlvClientCapabilityReporResultCode_Failure;
        err = ieee1905_tlv_err_sta_not_associated;
      } else {
        frame = pclient->assoc_frame;
        frame_len = pclient->assoc_frame_len;
      }
    }
  }
  i5TlvClientCapabilityReportTypeInsert(pmsg, res, frame, frame_len);
  if (err != ieee1905_tlv_err_reserved) {
    i5TlvErrorCodeTypeInsert(pmsg, err, mac);
  }

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

void i5MessageClientCapabilityReportReceive(i5_message_type *pmsg)
{
  int rc = 0;
  ieee1905_tlv_err_codes_t err;
  i5TlvClientCapabilityReporResultCode_Values_t res;
  unsigned char mac[MAC_ADDR_LEN], bssid[MAC_ADDR_LEN], err_sta_mac[MAC_ADDR_LEN];

  i5Trace("Received Client Capability Report Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvClientInfoTypeExtract(pmsg, mac, bssid);
  if (rc == 0) {
    rc |= i5TlvClientCapabilityReportTypeExtract(pmsg, i5MessageSrcMacAddressGet(pmsg), mac, bssid,
      &res);
    if (rc != 0) {
      i5Trace("BSSID["I5_MAC_DELIM_FMT"] STA["I5_MAC_DELIM_FMT"] Failed to parse "
        "Client Capability Report TLV of Message %04x\n",
        I5_MAC_PRM(bssid), I5_MAC_PRM(mac), i5MessageIdentifierGet(pmsg));
    } else if (res != i5TlvClientCapabilityReporResultCode_Success) {
      rc |= i5TlvErrorCodeTypeExtract(pmsg, &err, err_sta_mac);
      if (rc == 0) {
        i5Trace("BSSID["I5_MAC_DELIM_FMT"] STA["I5_MAC_DELIM_FMT"] Error Code %d\n",
          I5_MAC_PRM(bssid), I5_MAC_PRM(mac), err);
      } else {
        i5Trace("BSSID["I5_MAC_DELIM_FMT"] STA["I5_MAC_DELIM_FMT"] Failed to parse "
          "Error Code TLV of Message %04x\n",
          I5_MAC_PRM(bssid), I5_MAC_PRM(mac), i5MessageIdentifierGet(pmsg));
      }
    }
  } else {
    i5Trace("Failed to parse Client Info TLV of Message %04x\n", i5MessageIdentifierGet(pmsg));
  }

  i5MessageFree(pmsg);
}

/* Send Client Steering Rquuest message to a Multi AP Device */
void i5MessageClientSteeringRequestSend(i5_socket_type *psock, unsigned char *neighbor_al,
  ieee1905_steer_req *steer_req, ieee1905_vendor_data *vndr_msg_data)
{
  i5_message_type *pmsg;
  i5_dm_device_type *device = i5DmDeviceFind(neighbor_al);

  if (!device) {
    i5TraceError("Neighbor Device Not Found\n");
    return;
  }

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  /* Check if the steering opportunity is there for the device or not if the request is
   * steering opportunity
   */
  if (IEEE1905_IS_STEER_OPPORTUNITY(steer_req->request_flags)) {
    if (!i5DmIsSteerOpportunity(device)) {
      i5TraceError("Steering Opportunity not there\n");
      return;
    }
  }

  pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Client Steering Request Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Client Steering Request Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageClientSteeringRequestValue, i5_config.last_message_identifier);
  i5TlvSteeringRequestTypeInsert(pmsg, steer_req);
  /* Insert Vendor Specific TLV Data if exists, in Message */
  if (vndr_msg_data && vndr_msg_data->vendorSpec_len > 0) {
    i5Trace("Sending Vendor Specific TLV in Client Steering Request Message "
      "%04x on %s\n", i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

    i5TlvVendorSpecificTypeInsert(pmsg, vndr_msg_data->vendorSpec_msg,
      vndr_msg_data->vendorSpec_len);
  }
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);

  /* Now create the steering opportunity timer if the request is steering opportunity */
  if (IEEE1905_IS_STEER_OPPORTUNITY(steer_req->request_flags)) {
    i5DmSteerOpportunityTimer(device, steer_req->opportunity_window);
  }
}

#ifdef MULTIAP_PLUGFEST

/* Get all the STAs to steer */
static void i5MessageGetAllSTAsToSteer(ieee1905_steer_req *steer_req)
{
  i5_dm_device_type *pdmdev;
  i5_dm_bss_type *pdmbss;
  i5_dm_clients_type *pdmclient;
  ieee1905_sta_list *sta_info;

  pdmdev = i5DmGetSelfDevice();
  if (NULL == pdmdev) {
    return;
  }

  pdmbss = i5DmFindBSSFromDevice(pdmdev, steer_req->source_bssid);
  if (pdmbss == NULL) {
    i5TraceDirPrint("BSS " I5_MAC_DELIM_FMT " Not Found\n", I5_MAC_PRM(steer_req->source_bssid));
    return;
  }

  /* For each cleint in BSS */
  pdmclient = (i5_dm_clients_type *)pdmbss->client_list.ll.next;
  while (pdmclient != NULL) {
    /* Add the STA to sta list */
    sta_info = (ieee1905_sta_list*)malloc(sizeof(*sta_info));
    if (!sta_info) {
      i5TraceDirPrint("malloc error\n");
      return;
    }
    memcpy(sta_info->mac, pdmclient->mac, MAC_ADDR_LEN);
    ieee1905_glist_append(&steer_req->sta_list, (dll_t*)sta_info);
    pdmclient = pdmclient->ll.next;
  }
}

/* Find BSS for a STA */
static i5_dm_bss_type *i5MessageFindBSSForSTA()
{
  i5_dm_device_type *device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;

  while (device != NULL) {
    if (!i5DmDeviceIsSelf(device->DeviceId)) {
      /* Get any BSS from this device */
      /* For each interface in device */
      pdmif = (i5_dm_interface_type *)device->interface_list.ll.next;
      while (pdmif != NULL) {

        /* For each BSS in interface */
        pdmbss = (i5_dm_bss_type *)pdmif->bss_list.ll.next;
        if (pdmbss != NULL) {
          return pdmbss;
        }
        pdmif = pdmif->ll.next;
      }
    }
    device = device->ll.next;
  }

  return NULL;
}

/* Find BSS for all STAs */
static void i5MessageFindBSSForAllSTAs(ieee1905_steer_req *steer_req)
{
  dll_t *item_p, *next_p;
  ieee1905_sta_list *staInfo;
  ieee1905_bss_list *bssInfo;
  i5_dm_bss_type *pdmbss;
  i5_dm_interface_type *pdmif;
  chanspec_t chanspec = 0;
  unsigned short rclass = 0;

  /* For each STA in the STA list */
  for (item_p = dll_head_p(&steer_req->sta_list.head);
    !dll_end(&steer_req->sta_list.head, item_p);
    item_p = next_p) {
    next_p = dll_next_p(item_p);
    staInfo = (ieee1905_sta_list*)item_p;
    pdmbss = i5MessageFindBSSForSTA();
    if (pdmbss == NULL) {
      i5TraceDirPrint("Unable to find BSS for STA " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(staInfo->mac));
      return;
    }

    i5TraceDirPrint("For STA "I5_MAC_DELIM_FMT" BSS Is "I5_MAC_DELIM_FMT"\n",
      I5_MAC_PRM(staInfo->mac), I5_MAC_PRM(pdmbss->BSSID));
    bssInfo = (ieee1905_bss_list*)malloc(sizeof(*bssInfo));
    if (!bssInfo) {
      i5TraceDirPrint("malloc error\n");
      return;
    }
    pdmif = (i5_dm_interface_type*)I5LL_PARENT(pdmbss);
    memcpy(bssInfo->bssid, pdmbss->BSSID, MAC_ADDR_LEN);
    i5WlCfgGetRclass(pdmif->ifname, pdmif->chanspec, &rclass);
    bssInfo->target_op_class = (unsigned char)rclass;
    chanspec = pdmif->chanspec;
    bssInfo->target_channel = wf_chspec_ctlchan(chanspec);
    ieee1905_glist_append(&steer_req->bss_list, (dll_t*)bssInfo);
  }
}

/* To update the steer request structure to find the BSS. Just for test event */
static void i5MessageUpdateSteerReqStructForMAPPlugfest(ieee1905_steer_req *steer_req)
{
  /* If the STA list is empty needs to steer all the STAs */
  if (steer_req->sta_list.count == 0) {
    i5MessageGetAllSTAsToSteer(steer_req);
  }

  /* If the BSS count is 1 and if it is wildcard delete the BSS and find the BSS for STAs */
  if (steer_req->bss_list.count == 1) {
    ieee1905_bss_list *bssInfo = (ieee1905_bss_list*)dll_head_p(&steer_req->bss_list.head);
    if (bssInfo && (i5DmIsMacNull(bssInfo->bssid) || i5DmIsMacWildCard(bssInfo->bssid))) {
      ieee1905_glist_delete(&steer_req->bss_list, (dll_t*)bssInfo);
      free(bssInfo);
    }
  }

  /* If there is no BSS find the BSS for all the STAs */
  if (steer_req->bss_list.count == 0) {
    i5MessageFindBSSForAllSTAs(steer_req);
  }
}

/* Agent initiated RSSI based steering */
static void i5MessagRSSIBasedSteering()
{
  dll_t *item_p, *next_p;
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;
  ieee1905_bss_steer_config *bssCfg;
  ieee1905_steer_req steer_req;

  pdmdev = i5DmGetSelfDevice();
  if (NULL == pdmdev) {
    return;
  }

  /* For each STA in the STA list */
  for (item_p = dll_head_p(&i5_config.policyConfig.steercfg_bss_list.head);
    !dll_end(&i5_config.policyConfig.steercfg_bss_list.head, item_p);
    item_p = next_p) {
    next_p = dll_next_p(item_p);
    bssCfg = (ieee1905_bss_steer_config*)item_p;

    pdmif = i5DmInterfaceFind(pdmdev, bssCfg->mac);
    if (pdmif == NULL) {
      continue;
    }

    /* Agent-initiated Steering Disallowed */
    if (bssCfg->policy == 0) {
      continue;
    }

    /* For all the BSS in the interface issue steer request */
    pdmbss = (i5_dm_bss_type *)pdmif->bss_list.ll.next;
    while (pdmbss != NULL) {
      if (pdmbss->ClientsNumberOfEntries > 0) {
        memset(&steer_req, 0, sizeof(steer_req));
        /* Initialize sta and bss list */
        ieee1905_glist_init(&steer_req.sta_list);
        ieee1905_glist_init(&steer_req.bss_list);
        memcpy(steer_req.source_bssid, pdmbss->BSSID, sizeof(steer_req.source_bssid));
        steer_req.request_flags |= IEEE1905_STEER_FLAGS_RSSI;
        i5MessageUpdateSteerReqStructForMAPPlugfest(&steer_req);
        i5_config.cbs.steer_req(&steer_req);
        i5DmSteerRequestInfoFree(&steer_req);
      }
      pdmbss = pdmbss->ll.next;
    }
  }

}
#endif /* MULTIAP_PLUGFEST */

/* Process Client Steering Request Message */
void i5MessageClientSteeringRequestReceive(i5_message_type *pmsg)
{
  ieee1905_steer_req steer_req;
  int rc = 0;
  ieee1905_vendor_data msg_data;

  memset(&msg_data, 0, sizeof(msg_data));

  i5Trace("Received Client Steering Request Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  memset(&steer_req, 0, sizeof(steer_req));
  memcpy(steer_req.neighbor_al_mac, i5MessageSrcMacAddressGet(pmsg),
    sizeof(steer_req.neighbor_al_mac));

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvSteeringRequestTypeExtract(pmsg, &steer_req);

  if (rc != 0) {
    i5Trace("Failed to parse Steering Request TLV of Message %04x\n", i5MessageIdentifierGet(pmsg));
    goto end;
  }

  rc = i5TlvVendorSpecificTypeExtract(pmsg, i5MessageTlvExtractWithReset,
    &msg_data.vendorSpec_msg, &msg_data.vendorSpec_len);
  if ((rc == 0) && (msg_data.vendorSpec_msg != NULL)) {
    i5Trace("Received Vendor Specific TLV in Steering Request Message %04x on %s\n",
      i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
  }

  /* Send the acknowledgement */
  i5Message1905AckSend(pmsg);

  /* If the steer request callback is registered call it */
  if (i5_config.cbs.steer_req) {
#ifdef MULTIAP_PLUGFEST
    i5MessageUpdateSteerReqStructForMAPPlugfest(&steer_req);
#endif /* MULTIAP_PLUGFEST */
    i5_config.cbs.steer_req(&steer_req, &msg_data);
  }

end:
  /* Free the memory allocated for STAs and BSSs */
  i5DmSteerRequestInfoFree(&steer_req);

  i5MessageFree(pmsg);
}

/* Send Steering completed message */
void i5MessageSteeringCompletedSend(i5_socket_type *psock, unsigned char *neighbor_al)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  /* If it is not agent dont send steering completed message */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Steering Completde Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Steering Completed Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageSteeringCompletedValue,
    i5_config.last_message_identifier);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Process Steering Completed Message */
void i5MessageSteeringCompletedReceive(i5_message_type *pmsg)
{
  i5_dm_device_type *device = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg));

  i5Trace("Received Steering Completed Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  if (!device) {
    i5TraceError("Neighbor Device Not Found\n");
    goto end;
  }

  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Controller functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    goto end;
  }

  /* Close the steering opprotunity timer for this device */
  i5DmDeviceSteerOpportunityTimeout(device);

  /* Send the acknowledgement */
  i5Message1905AckSend(pmsg);

end:
  i5MessageFree(pmsg);
}

/* Send Client Association Control Rquuest message to a Multi AP Device */
void i5MessageClientAssociationControlRequestSend(i5_socket_type *psock, unsigned char *neighbor_al,
  ieee1905_block_unblock_sta *block_unblock_sta)
{
  i5_message_type *pmsg;
  i5_dm_device_type *device = i5DmDeviceFind(neighbor_al);

  if (!device) {
    i5TraceError("Neighbor Device Not Found\n");
    return;
  }

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Client Association Control Request Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Client Association Control Request Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageClientAssociationControlRequestValue,
    i5_config.last_message_identifier);
  i5TlvClientAssociationControlRequestTypeInsert(pmsg, block_unblock_sta);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Send ACK to Client Association Control Request message, loop for all associated clients, add
 * error code TLV for each associated client present in block unblock sta list while sending the
 * Ack message
 */
static int i5MessagSendClientAssociationControlRequestAck(i5_message_type *pmsg_req,
  ieee1905_block_unblock_sta *block_unblock_sta)
{
  i5_message_type* pmsg = NULL;
  i5_dm_device_type *pdevice = NULL;
  i5_dm_bss_type *pbss;
  i5_dm_clients_type *pclient = NULL;
  ieee1905_sta_list *staInfo;
  dll_t *item_p, *next_p;

  pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
  if (!pmsg) {
    i5TraceError(" Memory mallor erroc, exiting \n");
    return -1;
  }

  i5PacketHeaderInit(pmsg->ppkt, i5Message1905AckValue, i5MessageIdentifierGet(pmsg_req));

  pdevice = i5DmGetSelfDevice();
  if (!pdevice) {
    i5TraceError("Local device does not exist\n");
    goto end;
  }

  pbss = i5DmFindBSSFromDevice(pdevice, block_unblock_sta->source_bssid);
  if (pbss == NULL) {
    i5TraceError("Local BSS["I5_MAC_DELIM_FMT"] Not exists in Device["I5_MAC_DELIM_FMT"]\n",
      I5_MAC_PRM(block_unblock_sta->source_bssid), I5_MAC_PRM(pdevice->DeviceId));
    goto end;
  }

  /* Traverse each STA MAC address in list. If the client is associated, then add error code TLV */
  for (item_p = dll_head_p(&block_unblock_sta->sta_list.head);
    !dll_end(&block_unblock_sta->sta_list.head, item_p);
    item_p = next_p) {

    next_p = dll_next_p(item_p);
    staInfo = (ieee1905_sta_list*)item_p;

    pclient = i5DmFindClientInBSS(pbss, staInfo->mac);
    if (pclient) {
      i5TraceInfo("STA["I5_MAC_DELIM_FMT"] present in assoc list of BSS["I5_MAC_DELIM_FMT"] "
        "Device["I5_MAC_DELIM_FMT"]\n",
        I5_MAC_PRM(staInfo->mac), I5_MAC_PRM(block_unblock_sta->source_bssid),
        I5_MAC_PRM(pdevice->DeviceId));
      i5TlvErrorCodeTypeInsert(pmsg, ieee1905_tlv_err_sta_associated, staInfo->mac);
      /* Remove the STA item from list */
      ieee1905_glist_delete(&block_unblock_sta->sta_list, item_p);
      free(item_p);
    }
  }

end:
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
  return 0;
}

/* Process Client Steering Request Message */
void i5MessageClientAssociationControlRequestReceive(i5_message_type *pmsg)
{
  ieee1905_block_unblock_sta block_unblock_sta;
  int rc = 0;

  i5Trace("Received Client Association Control Request Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  memset(&block_unblock_sta, 0, sizeof(block_unblock_sta));

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvClientAssociationControlRequestTypeExtract(pmsg, &block_unblock_sta);

  /* Send the acknowledgement */
  i5MessagSendClientAssociationControlRequestAck(pmsg, &block_unblock_sta);

  /* If the steer request callback is registered call it */
  if (i5_config.cbs.block_unblock_sta_req) {
    i5_config.cbs.block_unblock_sta_req(&block_unblock_sta);
  }

  i5DmBlockUnblockInfoFree(&block_unblock_sta);

  i5MessageFree(pmsg);
}

/* Send Client steering BTM Report to the controller */
void i5MessageClientSteeringBTMReportSend(i5_socket_type *psock, unsigned char *neighbor_al,
  ieee1905_btm_report *btm_report)
{
  i5_message_type *pmsg;
  i5_dm_device_type *device = i5DmDeviceFind(neighbor_al);

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  if (!device) {
    i5TraceError("Neighbor Device Not Found\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Client Steering BTM Report Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Client Steering BTM Report Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageClientSteeringBTMReportValue,
    i5_config.last_message_identifier);
  i5TlvSteeringBTMReportTypeInsert(pmsg, btm_report);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Process Client Steering BTM report Message */
void i5MessageClientSteeringBTMReportReceive(i5_message_type *pmsg)
{
  ieee1905_btm_report btm_report;
  int rc = 0;

  i5Trace("Received Client Steering BTM Report Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  memset(&btm_report, 0, sizeof(btm_report));

  /* Send the acknowledgement */
  i5Message1905AckSend(pmsg);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvSteeringBTMReportTypeExtract(pmsg, &btm_report);

  if (rc == 0 && i5_config.cbs.steering_btm_rpt) {
     i5_config.cbs.steering_btm_rpt(&btm_report);
  }

  i5MessageFree(pmsg);
}

/* Send Higher Layer Data Payload to a Multi AP Devcice */
void i5MessageHigherLayerDataMessageSend (i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5HigherLayerProtocolField_Values protocol,
  unsigned char *data, unsigned int data_len)
{
  i5_message_type *pmsg;

  if (i5DmIsMacNull(neighbor_al_mac_address)) {
    i5TraceError("Empty Neighbor AL MAC\n");
    return;
  }

  if (data_len <= 0) {
    i5TraceError("No data to send\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Higher Layer Data Payload Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Higher Layer Data Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageHigherLayerDataValue, i5_config.last_message_identifier);
  i5TlvHigherLayerDataTypeInsert(pmsg, protocol, data, data_len);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive Higher Layer Data Payload */
void i5MessageHigherLayerDataReceive(i5_message_type *pmsg)
{
  int rc = 0;

  i5Trace("Received Higher Layer Data Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  /* Send the acknowledgement */
  i5Message1905AckSend(pmsg);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvHigherLayerDataTypeExtract(pmsg);

  i5MessageFree(pmsg);
}

/* Send 1905 ACK Message */
void i5Message1905AckSend(i5_message_type *pmsg_req)
{
  i5_dm_device_type *pdevice;
  i5_message_type *pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req),
    I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for ACK Message");
     return;
  }

   i5Trace("Sending 1905 Ack Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg_req)),
          i5MessageIdentifierGet(pmsg_req), pmsg_req->psock->u.sll.ifname);

  pdevice = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg_req));
  if (pdevice) {
    pdevice->psock = pmsg_req->psock;
    time(&pdevice->active_time);
  }

  /* get socket for local_interface_mac */
  i5PacketHeaderInit(pmsg->ppkt, i5Message1905AckValue, i5MessageIdentifierGet(pmsg_req));
  i5TlvEndOfMessageTypeInsert (pmsg);

  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive 1905 Ack Message */
void i5Message1905AckReceive(i5_message_type *pmsg)
{
  i5Trace("Received 1905 Ack Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  i5MessageFree(pmsg);
}

void i5MessageUnsupportedCommonReceive(i5_message_type *pmsg)
{
  i5TraceError("Received Unsupported Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  i5MessageFree(pmsg);
}

/* Send Channel Preference Query message to a Multi AP Devcice */
void i5MessageChannelPreferenceQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg != NULL) {
    i5_config.last_message_identifier++;
    i5Trace("Sending Channel Preference Query Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
      I5_MAC_PRM(neighbor_al_mac_address),
      i5_config.last_message_identifier, psock->u.sll.ifname);

    i5PacketHeaderInit(pmsg->ppkt, i5MessageChannelPreferenceQueryValue, i5_config.last_message_identifier);
    i5TlvEndOfMessageTypeInsert(pmsg);
    i5MessageSend(pmsg, 0);
    i5MessageFree(pmsg);
  }
}

/* Receive Channel Preference Query Message */
void i5MessageChannelPreferenceQueryReceive(i5_message_type *pmsg)
{
  unsigned short message_identifier = i5MessageIdentifierGet(pmsg);
  i5Trace("Received Channel Preference Query Message %04x on %s\n",
    message_identifier, pmsg->psock->u.sll.ifname);

  /* Dont respond if you are not agent */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    goto end;
  }
  /* send solicited channel preference report */
  i5MessageChannelPreferenceReportSend(pmsg->psock, i5MessageSrcMacAddressGet(pmsg),
    i5MessageIdentifierGet(pmsg), TRUE);

end:
  i5MessageFree(pmsg);
}

unsigned char i5MessagePrepareChannelPreferencebuffer(unsigned char reg_class_count,
  ieee1905_chan_pref_rc_map *rc_chan_pref, unsigned char* chan_pref_buf)
{
  int i = 0, j, k;

  if (!chan_pref_buf) {
    return 0;
  }

  /* Add preference data to a character buffer */
  chan_pref_buf[i++] = reg_class_count;
  for(k = 0;
    (k < reg_class_count) && ((i + 2 + rc_chan_pref[k].count * 2) < I5_RADIO_CAP_SIZE);
    k++) {
    chan_pref_buf[i++] = rc_chan_pref[k].regclass;
    chan_pref_buf[i++] = rc_chan_pref[k].count;
    for (j = 0; j < rc_chan_pref[k].count; j++) {
      chan_pref_buf[i++] = rc_chan_pref[k].channel[j];
    }
    chan_pref_buf[i++] = rc_chan_pref[k].pref << 4 | rc_chan_pref[k].reason;
  }
  /* return length of the buffer */
  return i;
}
/* Send Channel Preference Report Message */
void i5MessageChannelPreferenceReportSend(i5_socket_type *psock, unsigned char *dst_mac,
  unsigned short msg_token, bool solicited_msg)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice;
  i5_dm_interface_type *pdmif;
  unsigned short pkt_token;

  if (!psock || !dst_mac) {
    i5TraceError("Invalid argument to send channel prefernce report to controller \n");
    return;
  }

  pdevice = i5DmGetSelfDevice();
  if ( pdevice == NULL ) {
    i5TraceError("Local device does not exist\n");
  }

  pmsg = i5MessageCreate(psock, dst_mac, I5_PROTO);
  if (NULL == pmsg) {
    return;
  }

  pkt_token = solicited_msg ? msg_token : ++i5_config.last_message_identifier;
  i5Trace("Sending Channel Preference Report Message %04x on %s\n", pkt_token, psock->u.sll.ifname);
  i5PacketHeaderInit(pmsg->ppkt, i5MessageChannelPreferenceReportValue, pkt_token);

  for (pdmif = (i5_dm_interface_type *)pdevice->interface_list.ll.next;
    pdmif != NULL; pdmif = pdmif->ll.next) {
    unsigned char *chan_pref_buf;
    unsigned char len = 0;
    ieee1905_chan_pref_rc_map_array LocalChanPrefs;

    if (!i5DmIsInterfaceWireless(pdmif->MediaType)) {
      continue;
    }

    memset(&LocalChanPrefs, 0, sizeof(LocalChanPrefs));

    chan_pref_buf = (unsigned char *)malloc(I5_RADIO_CAP_SIZE);
    if (NULL == chan_pref_buf) {
      i5TraceError("Malloc failed channel preference buffer\n");
      goto end;
    }

    if (i5_config.cbs.prepare_channel_pref) {
      i5_config.cbs.prepare_channel_pref(pdmif, &LocalChanPrefs);
    }
    if (LocalChanPrefs.rc_count >= 0) {
      len = i5MessagePrepareChannelPreferencebuffer(LocalChanPrefs.rc_count,
        LocalChanPrefs.rc_map, chan_pref_buf);
      i5TlvChannelPreferenceTypeInsert(pmsg, pdmif->InterfaceId, chan_pref_buf, len);
    }
    if (LocalChanPrefs.rc_map) {
      free(LocalChanPrefs.rc_map);
    }
    free(chan_pref_buf);
  }

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);

end:
  i5MessageFree(pmsg);
}

/* Receive AP Capability Report Message */
void i5MessageChannelPreferenceReportReceive(i5_message_type *pmsg)
{
  int rc = 0;

  i5Trace("Received  Channel Preference Report Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  rc |= i5TlvChannelPreferenceTypeExtract(pmsg, 0);
  i5Message1905AckSend(pmsg);
  i5MessageFree(pmsg);
}

void i5MessageChannelPreferenceReportSendUnsolicited(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, unsigned char *data, unsigned short data_len)
{
  i5_message_type *pmsg;
  unsigned char *buf;
  int count;
  i5_dm_device_type *pdevice;

  if (i5DmIsMacNull(neighbor_al_mac_address)) {
    i5TraceError("Empty Neighbor AL MAC\n");
    return;
  }

  pdevice = i5DmDeviceFind(neighbor_al_mac_address);
  if ( pdevice == NULL ) {
    i5TraceError("Neighbor device does not exist\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for channel preference Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Channel Preference Report Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageChannelPreferenceReportValue, i5_config.last_message_identifier);

  if (data && data_len > 0) {
    buf = malloc(data_len/2);
    for (count = 0; count < (data_len / 2); count++) {
      unsigned int tmp;
      sscanf((char *)(data + 2 * count), "%02x", &tmp);
      buf[count] =  (unsigned char)tmp;
    }
    i5MessageInsertTlv(pmsg, buf, data_len/2);
    free(buf);
  }

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

void i5MessageChannelSelectionRequestSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, unsigned char *data, unsigned short data_len)
{
  i5_message_type *pmsg;
  unsigned char *buf;
  int count;
  i5_dm_device_type *pdevice;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  if (i5DmIsMacNull(neighbor_al_mac_address)) {
    i5TraceError("Empty Neighbor AL MAC\n");
    return;
  }

  pdevice = i5DmDeviceFind(neighbor_al_mac_address);
  if ( pdevice == NULL ) {
    i5TraceError("Neighbor device does not exist\n");
    return;
  }
  pdevice->psock = psock;

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for channel selection Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Channel selection request Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageChannelSelectionRequestValue, i5_config.last_message_identifier);

  if (data && data_len > 0) {
    buf = malloc(data_len/2);
    for (count = 0; count < (data_len / 2); count++) {
      unsigned int tmp;
      sscanf((char *)(data + 2 * count), "%02x", &tmp);
      buf[count] =  (unsigned char)tmp;
    }
    i5MessageInsertTlv(pmsg, buf, data_len/2);
    free(buf);
  } else {
       i5Trace("Sending channel selection request without preference TLV\n");
  }

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

i5_message_type *i5MessageChannelSelectionRequestCreate(i5_socket_type *psock, unsigned char *neighbor_al_mac_address)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return NULL;
  }

  if (i5DmIsMacNull(neighbor_al_mac_address)) {
    i5TraceError("Empty Neighbor AL MAC\n");
    return NULL;
  }

  pdevice = i5DmDeviceFind(neighbor_al_mac_address);
  if ( pdevice == NULL ) {
    i5TraceError("Neighbor device does not exist\n");
    return NULL;
  }
  pdevice->psock = psock;

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for channel selection Message");
     return NULL;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Channel selection request Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageChannelSelectionRequestValue, i5_config.last_message_identifier);

  return pmsg;
}

/* Receive Channel Selection Request Message */
void i5MessageChannelSelectionRequestReceive(i5_message_type *pmsg)
{
  int rc = 0;
  uint8 resp_code = 0;

  i5Trace("Received  Channel Selecetion Request Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  /* Dont respond if you are not agent */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    goto end;
  }

  /* If present, process channel set vendor TLV and skip channel preference
   * for the interface to which the vendor TLV is received
   */
  i5MessageGetVendorSpecificTlvWithCb(pmsg);

  i5MessageReset(pmsg);
  rc |= i5TlvChannelPreferenceTypeExtract(pmsg, 1);
  i5Trace(" rc[%d] after parsing channel preference TLV \n", rc);
  rc |= i5TlvTransmitPowerLimitTypeExtract(pmsg);

  if (rc != 0) {
    resp_code = 2;
  }
  i5MessageChannelSelectionResponseSend(pmsg, resp_code);
  if (i5_config.cbs.send_opchannel_rpt) {
    i5_config.cbs.send_opchannel_rpt();
  }
end:
  i5MessageFree(pmsg);
}

/* Send Channel Selection Response Message */
void i5MessageChannelSelectionResponseSend(i5_message_type *pmsg_req, uint8 resp_code)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice;
  i5_dm_interface_type *pdmif;

  pdevice = i5DmGetSelfDevice();
  if ( pdevice == NULL ) {
    i5TraceError("Local device does not exist\n");
  }

  i5Trace("Sending Channel Selection Response Message %04x on %s\n", i5MessageIdentifierGet(pmsg_req), pmsg_req->psock->u.sll.ifname);
  pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
  if (NULL == pmsg) {
    return;
  }

  i5PacketHeaderInit(pmsg->ppkt, i5MessageChannelSelectionResponseValue, i5MessageIdentifierGet(pmsg_req));

  pdmif = (i5_dm_interface_type *)pdevice->interface_list.ll.next;
  while (pdmif != NULL) {
   /* For all the operational BSS send the Channel Preference information */
    if (pdmif->msg_stat_flag == I5_MSG_RECEIVED) {
      if ((pdmif->bss_list.ll.next == NULL) || !pdmif->isConfigured) {
	/* override resp_code with error as interface is not operational Yet */
	resp_code = 2;
      }
      i5TlvChannelSelectionResponseTypeInsert(pmsg, pdmif->InterfaceId, resp_code);
      pdmif->msg_stat_flag = I5_MSG_RESP_SENT;
    }
    pdmif = pdmif->ll.next;
  }

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive Channel Selection Response Message */
void i5MessageChannelSelectionResponseReceive(i5_message_type *pmsg)
{
  int rc = 0;

  i5Trace("Received  Channel Selecetion Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  rc |= i5TlvChannelSelectionResponseTypeExtract(pmsg);
  i5MessageFree(pmsg);
}

/* Receive Operating Channel Report Message */
void i5MessageOperatingChannelReportReceive(i5_message_type *pmsg)
{
  int rc = 0;

  i5Trace("Received Operating Channel Report Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  rc |= i5TlvOperatingChannelReportTypeExtract(pmsg);
  /* Send the acknowledgement */
  i5Message1905AckSend(pmsg);
  i5MessageFree(pmsg);
}

/* Prepare the client association control request for all the BSS except the source and target */
int i5MessagePrepareandSendClientAssociationControl(ieee1905_client_assoc_cntrl_info *assoc_cntrl)
{
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;
  i5_dm_device_type *pdevice = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  ieee1905_block_unblock_sta block_unblock_sta;
  ieee1905_sta_list *sta_info;

  /* Prepare the block/unblock STA structure to send client association control message */
  memset(&block_unblock_sta, 0, sizeof(block_unblock_sta));
  block_unblock_sta.unblock = assoc_cntrl->unblock;
  block_unblock_sta.time_period = assoc_cntrl->time_period;
  ieee1905_glist_init(&block_unblock_sta.sta_list);
  sta_info = (ieee1905_sta_list*)malloc(sizeof(*sta_info));
  if (!sta_info) {
    i5TraceDirPrint("malloc error\n");
    goto end;
  }

  memcpy(sta_info->mac, assoc_cntrl->sta_mac, MAC_ADDR_LEN);
  ieee1905_glist_append(&block_unblock_sta.sta_list, (dll_t*)sta_info);

  /* For each multiap device in the network */
  while (pdevice != NULL) {

    /* For each interface in the device */
    pdmif = (i5_dm_interface_type *)pdevice->interface_list.ll.next;
    while (pdmif != NULL) {

      /* For each BSS in the interface */
      pdmbss = (i5_dm_bss_type *)pdmif->bss_list.ll.next;
      while (pdmbss != NULL) {

        /* If the BSSID is not source BSS and target BSS send client association control message */
        if (memcmp(pdmbss->BSSID, assoc_cntrl->source_bssid, sizeof(pdmbss->BSSID)) &&
          memcmp(pdmbss->BSSID, assoc_cntrl->trgt_bssid, sizeof(pdmbss->BSSID))) {
            /* Copy the BSSID */
            memcpy(block_unblock_sta.source_bssid, pdmbss->BSSID,
              sizeof(block_unblock_sta.source_bssid));

            /* For self device call the callback */
            if (i5DmDeviceIsSelf(pdevice->DeviceId)) {
              i5_config.cbs.block_unblock_sta_req(&block_unblock_sta);
            } else {
              i5MessageClientAssociationControlRequestSend(pdevice->psock, pdevice->DeviceId,
                &block_unblock_sta);
            }
        }
        pdmbss = pdmbss->ll.next;
      }
      pdmif = pdmif->ll.next;
    }
    pdevice = pdevice->ll.next;
  }

end:
  i5DmBlockUnblockInfoFree(&block_unblock_sta);
  return 0;
}

/* Send AP Metrics Query Message to an agent. All the BSSIDs are stored in the linear array */
void i5MessageAPMetricsQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address,
  unsigned char *bssids, unsigned char count)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for AP Metrics Query Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Ap Metrics Query Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageAPMetricsQueryValue, i5_config.last_message_identifier);
  i5TlvAPMetricQueryTypeInsert(pmsg, bssids, count);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive AP Metrics Query */
void i5MessageAPMetricsQueryReceive(i5_message_type *pmsg)
{
  int rc = 0;
  unsigned char count = 0;
  unsigned char *bssids = NULL;

  i5Trace("Received AP Metrics Query Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvAPMetricQueryTypeExtract(pmsg, &bssids, &count);

  if (rc != 0) {
    goto end;
  }

  i5MessageAPMetricsResponseSend(pmsg->psock, i5MessageSrcMacAddressGet(pmsg),
    i5MessageIdentifierGet(pmsg), bssids, count, NULL, 1);

end:
  if (bssids) {
    free(bssids);
  }
  i5MessageFree(pmsg);
}

static int i5MessageReportAPMetricBSS(i5_message_type *pmsg, i5_dm_bss_type *pbss,
  ieee1905_ifr_metricrpt *metricrpt, unsigned char isGetStats)
{
  int rc = 0;
  i5_dm_clients_type *pdmclient;
  i5_dm_interface_type *pdmif = (i5_dm_interface_type*)I5LL_PARENT(pbss);
  unsigned char incl_trfc_stat = 0, incl_lnk_mtrc = 0;
  ieee1905_vendor_data msg_data;

  /* get the latest stats from WBD */
  if (isGetStats) {
    if (i5_config.cbs.interface_metric) {
      i5_config.cbs.interface_metric(pdmif->ifname, pdmif->InterfaceId, &pdmif->ifrMetric);
    }

    if (i5_config.cbs.ap_metric) {
      i5_config.cbs.ap_metric(pbss->ifname, pbss->BSSID, &pbss->APMetric);
    }
  }

  rc |= i5TlvAPMetricsTypeInsert(pmsg, pbss->BSSID, pbss->ClientsNumberOfEntries,
    &pbss->APMetric, &pdmif->ifrMetric);

  /* if the Metric Reporting Policy TLV has Associated STA Traffic Stats Inclusion Policy set to 1
   * for a specified radio, include Associated STA Traffic Stats.
   * if the Metric Reporting Policy TLV has Associated STA Link Metrics Inclusion Policy set to 1
   * for a specified radio, include Associated STA Link Metrics.
   */
  if (!metricrpt) {
    return rc;
  }

  incl_trfc_stat = metricrpt->sta_mtrc_policy_flag & MAP_STA_MTRC_TRAFFIC_STAT;
  incl_lnk_mtrc = metricrpt->sta_mtrc_policy_flag & MAP_STA_MTRC_LINK_MTRC;
  if (incl_trfc_stat || incl_lnk_mtrc) {
    /* For all the associated clients */
    pdmclient = (i5_dm_clients_type *)pbss->client_list.ll.next;
    while (pdmclient != NULL) {
      memset(&msg_data, 0, sizeof(msg_data));
      /* Allocate Dynamic mem for Vendor data from Lib */
      msg_data.vendorSpec_msg = (unsigned char *)calloc(1, IEEE1905_MAX_VNDR_DATA_BUF);
      if (!msg_data.vendorSpec_msg) {
        i5TraceDirPrint("calloc error\n");
        rc = -1;
        goto end;
      }
      /* If the get stats is enabled. get it */
      if (isGetStats) {
        i5WlmGetAssociatedSTALinkMetric(NULL, pdmclient, NULL, &msg_data);
      }

      /* include Associated STA Traffic Stats */
      if (incl_trfc_stat) {
        rc |= i5TlvAssociatedSTATrafficStatsTypeInsert(pmsg, pdmclient->mac,
          &pdmclient->traffic_stats);
      }
      /* include Associated STA Link Metrics */
      if (incl_lnk_mtrc) {
        rc |= i5TlvAssociatedSTALinkMetricsTypeInsert(pmsg, pdmclient->mac, pbss->BSSID,
          &pdmclient->link_metric);

        /* Insert Vendor Specific TLV Data, in Message */
        if (msg_data.vendorSpec_len > 0) {
          i5Trace("Sending Vendor Specific TLV in AP Metrics Response Message "
            "%04x on %s\n", i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

          i5TlvVendorSpecificTypeInsert(pmsg, msg_data.vendorSpec_msg, msg_data.vendorSpec_len);
        }
      }
      /* Free Dynamic mem for Vendor data */
      if (msg_data.vendorSpec_msg) {
        free(msg_data.vendorSpec_msg);
      }
      pdmclient = pdmclient->ll.next;
    }
  }

end:
  return rc;
}

/* Send AP Metrics Response Message. All the BSSIDs are stored in the linear array */
void i5MessageAPMetricsResponseSend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address,
  unsigned short msgIndentifier, unsigned char *bssids, unsigned char count,
  unsigned char *ifrMAC, unsigned char isGetStats)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice;
  i5_dm_bss_type *pbss;
  i5_dm_interface_type *pdmif = NULL;
  ieee1905_ifr_metricrpt *metricrpt = NULL;
  int i;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pdevice = i5DmGetSelfDevice();
  if ( pdevice == NULL ) {
    i5TraceError("Local device does not exist\n");
    return;
  }

  i5Trace("Sending AP Metrics Response Message %04x on %s\n", msgIndentifier, psock->u.sll.ifname);
  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (NULL == pmsg) {
    return;
  }

  i5PacketHeaderInit(pmsg->ppkt, i5MessageAPMetricsResponseValue, msgIndentifier);

  /* If bssids provided send for only the BSS which is requested */
  if (count > 0 && bssids) {
    for (i = 0; i < count; i++) {
      pbss = i5DmFindBSSFromDevice(pdevice, &bssids[i * MAC_ADDR_LEN]);
      if (pbss == NULL)
        continue;

      metricrpt = i5DmFindMetricReportPolicy(((i5_dm_interface_type*)I5LL_PARENT(pbss))->InterfaceId);
      if (i5MessageReportAPMetricBSS(pmsg, pbss, metricrpt, isGetStats) != 0) {
        goto end;
      }
    }
  } else if (ifrMAC != NULL) {
    /* If interface is provided */
    pdmif = i5DmInterfaceFind(pdevice, ifrMAC);
    if (pdmif == NULL) {
      goto end;
    }
    metricrpt = i5DmFindMetricReportPolicy(pdmif->InterfaceId);

    pbss = pdmif->bss_list.ll.next;
    while (pbss) {
      /* Insert AP Metric TLV */
      if (i5MessageReportAPMetricBSS(pmsg, pbss, metricrpt, isGetStats) != 0) {
        goto end;
      }
      pbss = pbss->ll.next;
    }
  } else {
    /* Send for all the BSS in the device */
    pdmif = pdevice->interface_list.ll.next;
    while (pdmif) {
      metricrpt = i5DmFindMetricReportPolicy(pdmif->InterfaceId);
      /* For all the BSS */
      pbss = pdmif->bss_list.ll.next;
      while (pbss) {
        /* Insert AP Metric TLV */
        if (i5MessageReportAPMetricBSS(pmsg, pbss, metricrpt, isGetStats) != 0) {
          goto end;
        }
        pbss = pbss->ll.next;
      }
      pdmif = pdmif->ll.next;
    }
  }
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);

end:
  i5MessageFree(pmsg);
}

/* Receive AP Metrics Response */
void i5MessageAPMetricsResponseReceive(i5_message_type *pmsg)
{
  int sta_found = 0;

  i5Trace("Received AP Metrics Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);

  i5MessageGetVendorSpecificTlvWithCb(pmsg);

  i5TlvAPMetricsTypeExtract(pmsg, 0);
  i5TlvAssociatedSTATrafficStatsTypeExtract(pmsg);
  i5TlvAssociatedSTALinkMetricsTypeExtract(pmsg, &sta_found);

  i5MessageFree(pmsg);
}

/* Send Associated STA Link Metrics query to an agent */
void i5MessageAssociatedSTALinkMetricsQuerySend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, unsigned char *mac)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Associated STA Link Metrics Query Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Associated STA Link Metrics Query Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageAssociatedSTALinkMetricsQueryValue,
    i5_config.last_message_identifier);
  i5TlvSTAMACAddressTypeInsert(pmsg, mac);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive Associated STA Link Metrics query */
void i5MessageAssociatedSTALinkMetricsQueryReceive(i5_message_type *pmsg)
{
  int rc = 0;
  unsigned char mac[MAC_ADDR_LEN];

  i5Trace("Received Associated STA Link Metrics Query Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvSTAMACAddressTypeExtract(pmsg, mac);

  if (rc != 0)
    goto end;

  i5MessageAssociatedSTALinkMetricsResponseSend(pmsg->psock, i5MessageSrcMacAddressGet(pmsg),
    i5MessageIdentifierGet(pmsg), mac, 1);

end:
  i5MessageFree(pmsg);
}

/* Send Associated STA Link Metrics Response. All the STA MAC are stored in the linear array */
void i5MessageAssociatedSTALinkMetricsResponseSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, unsigned short msgIndentifier, unsigned char *macs,
  unsigned char count)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice;
  i5_dm_clients_type *pclient;
  unsigned char found = 0;
  int i;

  /* Initialize Vendor data */
  ieee1905_vendor_data msg_data;
  memset(&msg_data, 0, sizeof(msg_data));

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pdevice = i5DmGetSelfDevice();
  if ( pdevice == NULL ) {
    i5TraceError("Local device does not exist\n");
    return;
  }

  i5Trace("Sending Associated STA Link Metrics Response Message %04x on %s\n",
    msgIndentifier, psock->u.sll.ifname);
  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (NULL == pmsg) {
    return;
  }

  i5PacketHeaderInit(pmsg->ppkt, i5MessageAssociatedSTALinkMetricsResponseValue, msgIndentifier);

  for (i = 0; i < count; i++) {
    pclient = i5DmFindClientInDevice(pdevice, &macs[i * MAC_ADDR_LEN]);
    if (pclient == NULL)
      continue;

    found = 1;

    /* Allocate Dynamic mem for Vendor data from Lib */
    msg_data.vendorSpec_msg = (unsigned char *)calloc(1, IEEE1905_MAX_VNDR_DATA_BUF);
    if (!msg_data.vendorSpec_msg) {
      i5TraceDirPrint("calloc error\n");
      goto end;
    }

    /* Get Vendor Specific Associated STA Link Metrics data from application */
    i5WlmGetAssociatedSTALinkMetric(NULL, pclient, NULL, &msg_data);
    if (i5TlvAssociatedSTALinkMetricsTypeInsert(pmsg, pclient->mac,
      ((i5_dm_bss_type*)I5LL_PARENT(pclient))->BSSID, &pclient->link_metric) != 0) {
      goto end;
    }

    /* Insert Vendor Specific TLV Data, in Message */
    if (msg_data.vendorSpec_len > 0) {
      i5Trace("Sending Vendor Specific TLV in Associated STA Link Metrics Response Message "
        "%04x on %s\n", msgIndentifier, psock->u.sll.ifname);

      i5TlvVendorSpecificTypeInsert(pmsg, msg_data.vendorSpec_msg, msg_data.vendorSpec_len);
    }
    /* Free Dynamic mem for Vendor data */
    if (msg_data.vendorSpec_msg) {
      free(msg_data.vendorSpec_msg);
    }
  }

  /* If the specified STA is not associated with any of the BSS operated by the Multi-AP Agent,
   * the Multi-AP shall set the number of BSSIDs reported field to 0
   */
  if (!found && macs) {
    if (i5TlvAssociatedSTALinkMetricsTypeInsert(pmsg, &macs[0], NULL, NULL) != 0) {
      goto end;
    }
    i5TlvErrorCodeTypeInsert(pmsg, ieee1905_tlv_err_sta_not_associated, &macs[0]);
  }

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);

end:
  i5MessageFree(pmsg);
}

/* Receive Associated STA Link Metrics Response */
void i5MessageAssociatedSTALinkMetricsResponseReceive(i5_message_type *pmsg)
{
  int rc, sta_found = 0;
  unsigned char sta_mac[MAC_ADDR_LEN];
  ieee1905_tlv_err_codes_t err = ieee1905_tlv_err_reserved;

  i5Trace("Received Associated STA Link Metrics Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  i5Trace("Received Vendor Specific TLV in Associated STA Link Metrics Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
  i5MessageGetVendorSpecificTlvWithCb(pmsg);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc = i5TlvAssociatedSTALinkMetricsTypeExtract(pmsg, &sta_found);

  /* If STA not found, extract error code TLV */
  if (rc == 0 && sta_found == 0) {
    rc = i5TlvErrorCodeTypeExtract(pmsg, &err, sta_mac);
    if (rc == 0) {
      i5TraceInfo("STA["I5_MAC_DELIM_FMT"] Not found err[%d]\n", I5_MAC_PRM(sta_mac), err);
    } else {
      i5TraceInfo("Failed to parse error code TLV in Message %04x on %s\n",
        i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
    }
  }

  i5MessageFree(pmsg);
}

/* Send UnAssociated STA Link Metrics query to an agent */
void i5MessageUnAssociatedSTALinkMetricsQuerySend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, ieee1905_unassoc_sta_link_metric_query *query)
{
  i5_message_type *pmsg;
  i5_dm_device_type *device = i5DmDeviceFind(neighbor_al_mac_address);

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  if (!device) {
    i5TraceError("Neighbor Device["I5_MAC_DELIM_FMT"] Device Not Found\n",
      I5_MAC_PRM(neighbor_al_mac_address));
    return;
  }

#ifndef MULTIAP_PLUGFEST
  /* Check if it supports unassociated STA link metrics or not */
  if (!(device->BasicCaps & IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT)) {
    i5TraceError("Neighbor Device["I5_MAC_DELIM_FMT"] Does not supports Unassociated STA "
      "link metric reporting\n", I5_MAC_PRM(neighbor_al_mac_address));
    return;
  }
#endif /* MULTIAP_PLUGFEST */

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for UnAssociated STA Link Metrics Query Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending UnAssociated STA Link Metrics Query Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageUnAssociatedSTALinkMetricsQueryValue,
    i5_config.last_message_identifier);
  i5TlvUnAssociatedSTALinkMetricsQueryTypeInsert(pmsg, query);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Send ACK to Unassociated link query, loop for all associated clients, send error
 * for each associated client present in unassoc sta link query's mac list
 */
int i5MessagSendUnAssociatedSTALinkMetricsQueryAck(i5_message_type *pmsg_req,
  ieee1905_unassoc_sta_link_metric_query *query)
{
  i5_message_type* pmsg = NULL;
  i5_dm_device_type *pdevice = NULL;
  i5_dm_clients_type *pclient = NULL;
  unassoc_query_per_chan_rqst *data = NULL;
  unsigned char *mac = NULL;
  int i = 0, count = 0;

  pdevice = i5DmGetSelfDevice();
  if (!pdevice) {
    i5TraceError("Local device does not exist\n");
    return -1;
  }

  pmsg = i5MessageCreate(pmsg_req->psock, i5MessageSrcMacAddressGet(pmsg_req), I5_PROTO);
  if (!pmsg) {
    i5TraceError(" Memory mallor erroc, exiting \n");
    return -1;
  }

  i5PacketHeaderInit(pmsg->ppkt, i5Message1905AckValue, i5MessageIdentifierGet(pmsg_req));
  if (!query || !query->chCount || !query->data) {
    /* send ack */
    goto finish;
  }
  data = query->data;
  for(i = 0; i < query->chCount; i++) {
    mac = data[i].mac_list;
    for (count = 0; count < data[i].n_sta; count++) {
      pclient = i5DmFindClientInDevice(pdevice, &mac[count * MAC_ADDR_LEN]);
      if (pclient == NULL) {
        continue;
      } else {
        i5TlvErrorCodeTypeInsert(pmsg, ieee1905_tlv_err_sta_associated,
	  &mac[count * MAC_ADDR_LEN]);
      }
    }
  }
finish:
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
  return 0;
}

/* Receive UnAssociated STA Link Metrics query */
void i5MessageUnAssociatedSTALinkMetricsQueryReceive(i5_message_type *pmsg)
{
  int rc = 0;
  int i;
  ieee1905_unassoc_sta_link_metric_query *query = NULL;
  unassoc_query_per_chan_rqst *plist = NULL;
  i5_dm_device_type *pdevice = NULL;

  i5Trace("Received UnAssociated STA Link Metrics Query Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  pdevice = i5DmGetSelfDevice();
  /* Check if it supports unassociated STA link metrics or not */
  if (!(pdevice->BasicCaps & IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT)) {
    i5TraceError("Device["I5_MAC_DELIM_FMT"] Does not supports Unassociated STA "
      "link metric reporting\n", I5_MAC_PRM(i5_config.i5_mac_address));
    goto end;
  }
  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);

  rc |= i5TlvUnAssociatedSTALinkMetricsQueryTypeExtract(pmsg, &query);
  /* send ack with error code = 0x01 for associated sta present in query */
  if ((i5MessagSendUnAssociatedSTALinkMetricsQueryAck(pmsg, query)) == -1) {
    i5TraceError(" error in sending ack for unassoc link metric query \n");
    goto end;
  }

  if (rc != 0) {
    i5TraceError(" error in extracting unassoc link metric query, rc= %d \n", rc);
    goto end;
  }
  memcpy(query->neighbor_al_mac, i5MessageSrcMacAddressGet(pmsg), sizeof(query->neighbor_al_mac));

  i5TraceInfo("Unassoc link metric query from AL_ID=" I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(query->neighbor_al_mac));

  if (i5_config.cbs.unassoc_sta_metric) {
    rc = i5_config.cbs.unassoc_sta_metric(query);
  }

end:
  if (query) {
    plist = query->data;
    if (plist) {
      for(i = 0; i < query->chCount; i++) {
	  if (plist[i].mac_list) {
            free(plist[i].mac_list);
	  }
      }
      free(plist);
    }
    free(query);
   }

  i5MessageFree(pmsg);
}

/* Send UnAssociated STA Link Metrics Response */
void i5MessageUnAssociatedSTALinkMetricsResponseSend(i5_socket_type *psock,
  unsigned char *neighbor_al, ieee1905_unassoc_sta_link_metric *metric)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
  if (NULL == pmsg) {
    return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending UnAssociated STA Link Metrics Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageUnAssociatedSTALinkMetricsResponseValue,
    i5_config.last_message_identifier);

  i5TlvUnAssociatedSTALinkMetricsResponseTypeInsert(pmsg, metric);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive UnAssociated STA Link Metrics Response */
void i5MessageUnAssociatedSTALinkMetricsResponseReceive(i5_message_type *pmsg)
{
  int rc = 0;
  ieee1905_unassoc_sta_link_metric metric;

  i5Trace("Received UnAssociated STA Link Metrics Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  memset(&metric, 0, sizeof(metric));
  ieee1905_glist_init(&metric.sta_list);
  rc = i5TlvUnAssociatedSTALinkMetricsResponseTypeExtract(pmsg, &metric);

  if (rc == 0 && i5_config.cbs.unassoc_sta_metric_resp) {
    i5_config.cbs.unassoc_sta_metric_resp(i5MessageSrcMacAddressGet(pmsg), &metric);
  }

  i5DmGlistCleanup(&metric.sta_list);
  i5MessageFree(pmsg);
}

/* Sned Beacon Metrics Query Message */
void i5MessageBeaconMetricsQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address,
  ieee1905_beacon_request *query)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Beacon Metrics Query Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Beacon Metrics Query Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageBeaconMetricsQueryValue,
    i5_config.last_message_identifier);
  i5TlvBeaconMetricsQueryTypeInsert(pmsg, query);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive Beacon Metrics Query */
void i5MessageBeaconMetricsQueryReceive(i5_message_type *pmsg)
{
  int rc = 0;
  ieee1905_beacon_request query;
  i5_dm_device_type *pdevice;
  i5_dm_bss_type *pbss;
  i5_dm_clients_type *pclient;

  i5Trace("Received Beacon Metrics Query Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
  memset(&query, 0, sizeof(query));
  memcpy(query.neighbor_al_mac, i5MessageSrcMacAddressGet(pmsg),
    sizeof(query.neighbor_al_mac));

  /* Send the acknowledgment */
  i5Message1905AckSend(pmsg);

  pdevice = i5DmGetSelfDevice();
  if (pdevice == NULL) {
    i5TraceError("Local device does not exist\n");
    rc = -1;
    goto end;
  }

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvBeaconMetricsQueryTypeExtract(pmsg, &query);
  if (rc != 0) {
    goto end;
  }
  if (i5_config.cbs.beacon_metrics_query) {
    pclient = i5DmFindClientInDevice(pdevice, query.sta_mac);
    if (pclient == NULL) {
      i5TraceError("STA " I5_MAC_DELIM_FMT " does not exist in device " I5_MAC_DELIM_FMT "\n",
        I5_MAC_PRM(query.sta_mac), I5_MAC_PRM(pdevice->DeviceId));
      goto end;
    }
    pbss = (i5_dm_bss_type*)I5LL_PARENT(pclient);
    i5_config.cbs.beacon_metrics_query(pbss->ifname, pbss->BSSID, &query);
  }

end:
  i5MessageFree(pmsg);

  if (query.ap_chan_report) {
    free(query.ap_chan_report);
  }
  if (query.element_list) {
    free(query.element_list);
  }
}

/* Sned Beacon Metrics response Message */
void i5MessageBeaconMetricsResponseSend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address,
  ieee1905_beacon_report *report)
{
  i5_message_type *pmsg;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
     i5Trace("Unable to allocate pmsg for Beacon Metrics Response Message");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Beacon Metrics Response Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al_mac_address),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageBeaconMetricsResponseValue,
    i5_config.last_message_identifier);
  i5TlvBeaconMetricsResponseTypeInsert(pmsg, report);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive Beacon Metrics Response */
void i5MessageBeaconMetricsResponseReceive(i5_message_type *pmsg)
{
  int rc = 0;
  ieee1905_beacon_report report;

  i5Trace("Received Beacon Metrics response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
  memset(&report, 0, sizeof(report));

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvBeaconMetricsResponseTypeExtract(pmsg, &report);
  if (rc != 0) {
    i5Trace("Failed to extract becon report\n");
    goto end;
  }

  if (i5_config.cbs.beacon_metric_resp) {
    i5_config.cbs.beacon_metric_resp(i5MessageSrcMacAddressGet(pmsg), &report);
  }

end:
  i5MessageFree(pmsg);

  if (report.report_element) {
    free(report.report_element);
  }
}

/* Used when application using 1905 lib, wants to send a Vendor Specific Message to other 1905 entity */
int i5MessageVendorSpecificMessageSend(i5_socket_type *psock,
   unsigned char *neighbor_al, ieee1905_vendor_data *msg_data)
{
   int rc = 0;
   i5_message_type *pmsg = NULL;

   if (!psock) {
      i5TraceError("Null pointer(s) %p!\n", psock);
      return rc;
   }

   pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
   if (NULL == pmsg) {
     i5Trace("Unable to allocate pmsg for Vendor Specific Message Send\n");
     return rc;
   }

   i5_config.last_message_identifier++;
   i5Trace("Sending Proprietary Vendor Specific Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
     I5_MAC_PRM(neighbor_al),
     i5_config.last_message_identifier, psock->u.sll.ifname);

   i5PacketHeaderInit(pmsg->ppkt, i5MessageVendorSpecificValue,
     i5_config.last_message_identifier);

   i5TlvVendorSpecificTypeInsert(pmsg, msg_data->vendorSpec_msg, msg_data->vendorSpec_len);
   i5TlvEndOfMessageTypeInsert(pmsg);
   rc = i5MessageSend(pmsg, 0);
   i5MessageFree(pmsg);
   return rc;
}

/* Send Backhaul Steering Request message to a Multi AP Device */
int i5MessageBackhaulSteeringRequestSend(i5_socket_type *psock,
  unsigned char *neighbor_al, ieee1905_backhaul_steer_msg *bh_steer_req)
{
  i5_message_type *pmsg;
  i5_dm_device_type *device = i5DmDeviceFind(neighbor_al);

  if (!device) {
    i5TraceError("Neighbor Device Not Found\n");
    return -1;
  }

  pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
  if (pmsg == NULL) {
     i5TraceDirPrint("Unable to allocate pmsg for Backhaul Steering Request Message\n");
     return -1;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Backhaul Steering Request Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageBackhaulSteeringRequestValue, i5_config.last_message_identifier);
  i5TlvBhSteeringRequestTypeInsert(pmsg, bh_steer_req);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);

  return 0;
}

/* Process Backhaul Steering Request Message */
void i5MessageBackhaulSteeringRequestReceive(i5_message_type *pmsg)
{
  ieee1905_backhaul_steer_msg *bh_steer_req;
  int rc = 0;

  i5Trace("Received Backhaul Steering Request Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  if ((bh_steer_req = (ieee1905_backhaul_steer_msg *)malloc(sizeof(*bh_steer_req))) == NULL) {
    i5TraceError("Malloc error\n");
    goto end;
  }
  memset(bh_steer_req, 0, sizeof(*bh_steer_req));
  memcpy(bh_steer_req->neighbor_al_mac, i5MessageSrcMacAddressGet(pmsg),
    sizeof(bh_steer_req->neighbor_al_mac));

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvBhSteeringRequestTypeExtract(pmsg, bh_steer_req);

  if (rc != 0) {
    i5Trace("Failed to parse Steering Request TLV of Message %04x\n", i5MessageIdentifierGet(pmsg));
    free(bh_steer_req);
    goto end;
  }

  /* Send the acknowledgement */
  i5Message1905AckSend(pmsg);

  /* If the steer request callback is registered call it */
  if (i5_config.cbs.backhaul_steer_req) {
    char ifname[I5_MAX_IFNAME];

    i5GetIfnameFromMacAdress(bh_steer_req->bh_sta_mac, ifname);
    i5_config.cbs.backhaul_steer_req(ifname, bh_steer_req);
  } else {
    free(bh_steer_req);
  }
end:
  i5MessageFree(pmsg);
}

/* Send Backhaul Steering Response message to a Multi AP Device */
void i5MessageBackhaulSteeringResponseSend(i5_socket_type *psock,
  unsigned char *neighbor_al, ieee1905_backhaul_steer_msg *bh_steer_resp)
{
  i5_message_type *pmsg;
  i5_dm_device_type *device = i5DmDeviceFind(neighbor_al);

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  if (!device) {
    i5TraceError("Neighbor Device Not Found\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
  if (pmsg == NULL) {
     i5TraceDirPrint("Unable to allocate pmsg for Backhaul Steering Request Message\n");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Backhaul Steering Response Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
          I5_MAC_PRM(neighbor_al),
          i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageBackhaulSteeringResponseValue, i5_config.last_message_identifier);
  i5TlvBhSteeringResponseTypeInsert(pmsg, bh_steer_resp);
  /* Insert Error Code TLV if Response is Reject */
  if (bh_steer_resp->resp_status_code) {
    i5TlvErrorCodeTypeInsert(pmsg, ieee1905_tlv_err_backhaul_steer_reject_low_signal, bh_steer_resp->bh_sta_mac);
  }
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Process Backhaul Steering Response Message */
void i5MessageBackhaulSteeringResponseReceive(i5_message_type *pmsg)
{
  ieee1905_backhaul_steer_msg bh_steer_resp;
  int rc = 0;

  i5Trace("Received Backhaul Steering Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  memset(&bh_steer_resp, 0, sizeof(bh_steer_resp));
  memcpy(bh_steer_resp.neighbor_al_mac, i5MessageSrcMacAddressGet(pmsg),
    sizeof(bh_steer_resp.neighbor_al_mac));

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  rc |= i5TlvBhSteeringResponseTypeExtract(pmsg, &bh_steer_resp);

  if (rc != 0) {
    i5Trace("Failed to parse Steering Response TLV of Message %04x\n", i5MessageIdentifierGet(pmsg));
    goto end;
  }

  /* Send the acknowledgement */
  i5Message1905AckSend(pmsg);

end:
  i5MessageFree(pmsg);
}

/* Send Combined Infrastructure Metrics Message to a MultiAP Agent */
void i5MessageCombinedInfrastructureMetricsSend(i5_socket_type *psock, unsigned char *neighbor_al)
{
  i5_message_type *pmsg;
  i5_dm_device_type *pdevice;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pbss;
  char rxWanted = 1, txWanted = 1;

  pmsg = i5MessageCreate(psock, neighbor_al, I5_PROTO);
  if (pmsg == NULL) {
     i5TraceDirPrint("Unable to allocate pmsg for Combined Infrastructure Metrics Message\n");
     return;
  }

  i5_config.last_message_identifier++;
  i5Trace("Sending Combined Infrastructure Metrics Message to " I5_MAC_DELIM_FMT " %04x on %s\n",
    I5_MAC_PRM(neighbor_al), i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageCombinedInfrastructureMetricsValue,
    i5_config.last_message_identifier);

  /* Include AP Metrics TLV for all the devices controller identifies */
  pdevice = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  while (pdevice != NULL) {
    /* Send for all the BSS in the device */
    pdmif = pdevice->interface_list.ll.next;
    while (pdmif) {
      /* For all the BSS */
      pbss = pdmif->bss_list.ll.next;
      while (pbss) {
        i5TlvAPMetricsTypeInsert(pmsg, pbss->BSSID, pbss->ClientsNumberOfEntries,
          &pbss->APMetric, &pdmif->ifrMetric);
        pbss = pbss->ll.next;
      }
      pdmif = pdmif->ll.next;
    }
    pdevice = pdevice->ll.next;
  }

  /* Include trasmitter and receiver link metric for backhaul */
  pdevice = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  while (pdevice != NULL) {
    /* loop through all devices that aren't self */
    if (!i5DmDeviceIsSelf(pdevice->DeviceId)) {
      /* get interfaces for that device */
      unsigned char local_interface_mac[I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR][ETH_ALEN];
      unsigned char neighbor_interface_mac[I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR][ETH_ALEN];
      int numLinksFound = i5DmGetInterfacesWithNeighbor(pdevice->DeviceId, local_interface_mac[0],
        neighbor_interface_mac[0], I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR);
      i5TraceInfo("Found %d links to neighbor " I5_MAC_DELIM_FMT "\n", numLinksFound,
        I5_MAC_PRM(pdevice->DeviceId));
      if (txWanted) {
        i5_tlv_linkMetricTx_t txStats[numLinksFound];

        i5MessageSetTxStats (txStats, local_interface_mac[0], neighbor_interface_mac[0],
          numLinksFound);
        i5TlvLinkMetricTxInsert(pmsg, i5_config.i5_mac_address, pdevice->DeviceId, txStats,
          numLinksFound);
      }
      if (rxWanted) {
        i5_tlv_linkMetricRx_t rxStats[numLinksFound];

        i5MessageSetRxStats(rxStats, local_interface_mac[0], neighbor_interface_mac[0],
          numLinksFound);
        i5TlvLinkMetricRxInsert(pmsg, i5_config.i5_mac_address, pdevice->DeviceId, rxStats,
          numLinksFound);
      }
    }
    pdevice = pdevice->ll.next;
  }

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Process Combined Infrastructure Metrics Message */
void i5MessageCombinedInfrastructureMetricsReceive(i5_message_type *pmsg)
{
  i5Trace("Received Combined Infrastructure Metrics Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  /* Send the acknowledgement */
  i5Message1905AckSend(pmsg);

  pmsg->ppkt->offset = sizeof(struct ethhdr) + sizeof(i5_message_header_type);
  i5TlvAPMetricsTypeExtract(pmsg, 1);
  i5TlvLinkMetricResponseExtract(pmsg);

  i5MessageFree(pmsg);
}

/* Send operating channel report */
int i5MessageOperatingChanReportSend(i5_socket_type *psock, unsigned char *dst_mac,
  ieee1905_operating_chan_report *chan_rpt)
{
  i5_message_type* pmsg = NULL;

  if (!psock || !dst_mac || !chan_rpt) {
    i5TraceError("Invalid argument to send operating channel report to controller \n");
    return -1;
  }
  pmsg = i5MessageCreate(psock, dst_mac, I5_PROTO);
  if (!pmsg) {
    i5TraceError(" Memory mallor erroc, exiting \n");
    return -1;
  }

  i5_config.last_message_identifier++;
  i5PacketHeaderInit(pmsg->ppkt, i5MessageOperatingChannelReportValue,
    i5_config.last_message_identifier);

  if (chan_rpt->list) {
    i5TlvOperatingChannelReportTypeInsert(pmsg, chan_rpt->radio_mac,
      chan_rpt->list->chan, chan_rpt->list->op_class, chan_rpt->tx_pwr);

    i5TlvEndOfMessageTypeInsert(pmsg);
    i5MessageSend(pmsg, 0);
    free(chan_rpt->list);
  }
  i5MessageFree(pmsg);
  return 0;
}

#endif /* MULTIAP */
