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

#ifndef _IEEE1905_MESSAGE_H_
#define _IEEE1905_MESSAGE_H_

/*
 * IEEE1905 Message
 */
#include <asm/byteorder.h>
#include "ieee1905_socket.h"
#include "ieee1905_linkedlist.h"
#include "ieee1905_packet.h"
#include "ieee1905_timer.h"
#include "ieee1905.h"

/* Standard Values */
#define I5_MESSAGE_STANDARD_TOPOLOGY_DISCOVERY_PERIOD_MSEC  60000

/* Not standard, BRCM values.  Do not use for interaction */
#define I5_MESSAGE_TOPOLOGY_DISCOVERY_PERIOD_MSEC     \
                              I5_MESSAGE_STANDARD_TOPOLOGY_DISCOVERY_PERIOD_MSEC
#define I5_MESSAGE_TOPOLOGY_DISCOVERY_RETRY_MSEC      50
#define I5_MESSAGE_FRAGMENT_TIMEOUT_MSEC              20000
#define I5_MESSAGE_TOPOLOGY_QUERY_TIMEOUT_MSEC        5000
#define I5_MESSAGE_TOPOLOGY_QUERY_RETRY_COUNT         10
#define I5_MESSAGE_RELAY_WAIT_TIMEOUT_MSEC            5000
#define I5_MESSAGE_AP_SEARCH_NOT_READY_INTERVAL_MSEC  2000
#define I5_MESSAGE_AP_SEARCH_START_INTERVAL_MSEC      5000
#define I5_MESSAGE_AP_SEARCH_PERIODIC_INTERVAL_MSEC   30000
#define I5_MESSAGE_AP_SEARCH_M1_M2_WAITING_MSEC       5000
#define I5_MESSAGE_AP_SEARCH_START_COUNT              10
#define I5_MESSAGE_START_MESSAGE_DELAY_MSEC           1000

#define I5_MESSAGE_MAX_LINKMETRICS_INTERFACES_PER_NEIGHBOR 3

#define I5_MESSAGE_DIR_RX       0x1
#define I5_MESSAGE_DIR_TX       0x2
/* this will match DIR TX as far as trace enable is concerned but will show "Relayed" instead of "Sent" in packet trace */
#define I5_MESSAGE_DIR_TX_RELAY 0x6

#define I5_SEC_WAIT_TO_REMOVE_UNREACHABLE_DEVICE	\
			(I5_MESSAGE_STANDARD_TOPOLOGY_DISCOVERY_PERIOD_MSEC + 10)

extern unsigned char I5_MULTICAST_MAC[];
extern unsigned char LLDP_MULTICAST_MAC[];

#ifdef MULTIAP
typedef enum i5HigherLayerProtocolField_Values {
  i5TlvHigherLayerProtocol_Reserved = 0,
  i5TlvHigherLayerProtocol_TR181Transport, /* All values 0x02 through 0xff are reserved */
} i5HigherLayerProtocolField_Values;
#endif /* MULTIAP */

typedef struct {
  i5_ll_listitem ll;
  i5_packet_type packet_list;
  i5_packet_type *ppkt; // Current packet (tail when creating)
  i5_socket_type *psock;
  timer_elem_type *ptmr;
  unsigned char fragment_identifier_count;
} i5_message_type;

typedef struct {
  unsigned char message_version;
  unsigned char reserved_field;
  unsigned short message_type;
  unsigned short message_identifier;
  unsigned char fragment_identifier;
  union {
#if defined(__BIG_ENDIAN_BITFIELD)
    struct {
      unsigned char last_fragment_indicator :1;
      unsigned char relay_indicator         :1;
      unsigned char reserved_field_2        :6;
    };
#elif defined(__LITTLE_ENDIAN_BITFIELD)
    struct {
      unsigned char reserved_field_2        :6;
      unsigned char relay_indicator         :1;
      unsigned char last_fragment_indicator :1;
    };
#else
#error "BITFIELD ORDER NOT DEFINED"
#endif // endif
    unsigned char indicators;
  };
} __attribute__((__packed__)) i5_message_header_type;

typedef enum i5_message_types {
  i5MessageTopologyDiscoveryValue = 0,
  i5MessageTopologyNotificationValue,
  i5MessageTopologyQueryValue,
  i5MessageTopologyResponseValue,
  i5MessageVendorSpecificValue,
  i5MessageLinkMetricQueryValue,
  i5MessageLinkMetricResponseValue,
  i5MessageApAutoconfigurationSearchValue,
  i5MessageApAutoconfigurationResponseValue,
  i5MessageApAutoconfigurationWscValue,
  i5MessageApAutoconfigurationRenewValue,
  i5MessagePushButtonEventNotificationValue,
  i5MessagePushButtonJoinNotificationValue,
  i5MessageHigherLayerQueryValue,            // Unsupported
  i5MessageHigherLayerResponseValue,         // Unsupported
  i5MessagePowerChangeRequestValue,          // Unsupported
  i5MessagePowerChangeResponseValue,         // Unsupported
  i5MessageGenericPhyQueryValue,
  i5MessageGenericPhyResponseValue,
#ifdef MULTIAP
  i5Message1905AckValue = 0x8000,
  i5MessageAPCapabilityQueryValue,
  i5MessageAPCapabilityReportValue,
  i5MessageMultiAPPolicyConfigRequestValue,
  i5MessageChannelPreferenceQueryValue,
  i5MessageChannelPreferenceReportValue,
  i5MessageChannelSelectionRequestValue,
  i5MessageChannelSelectionResponseValue,
  i5MessageOperatingChannelReportValue,
  i5MessageClientCapabilityQueryValue,
  i5MessageClientCapabilityReportValue,
  i5MessageAPMetricsQueryValue,
  i5MessageAPMetricsResponseValue,
  i5MessageAssociatedSTALinkMetricsQueryValue,
  i5MessageAssociatedSTALinkMetricsResponseValue,
  i5MessageUnAssociatedSTALinkMetricsQueryValue,
  i5MessageUnAssociatedSTALinkMetricsResponseValue,
  i5MessageBeaconMetricsQueryValue,
  i5MessageBeaconMetricsResponseValue,
  i5MessageCombinedInfrastructureMetricsValue,
  i5MessageClientSteeringRequestValue,
  i5MessageClientSteeringBTMReportValue,
  i5MessageClientAssociationControlRequestValue,
  i5MessageSteeringCompletedValue,
  i5MessageHigherLayerDataValue,
  i5MessageBackhaulSteeringRequestValue,
  i5MessageBackhaulSteeringResponseValue,
#endif /* MULTIAP */
} i5_message_types_t;

enum {
  i5MessageTlvExtractWithoutReset = 0,
  i5MessageTlvExtractWithReset,
};

enum {
  i5MessageFreqBand_802_11_2_4Ghz = 0,
  i5MessageFreqBand_802_11_5Ghz,
  i5MessageFreqBand_802_11_60Ghz,
  i5MessageFreqBand_Reserved, /* All values 0x03 through through 0xff are reserved */
};

/* Routing structures */
typedef struct {
  i5_ll_listitem ll;
  unsigned char macAddress[MAC_ADDR_LEN];
} i5_routing_destination;

typedef struct {
  i5_ll_listitem ll;
  unsigned char interfaceMac [MAC_ADDR_LEN];
  unsigned char numDestinations;
  i5_routing_destination destinationList;
} i5_routing_table_entry;

typedef struct {
  unsigned char numEntries;
  i5_routing_table_entry entryList;
} i5_routing_table_type;

void i5MessageDumpMessages(void);
void i5MessageCancel(i5_socket_type *psock);
int i5MessageRawMessageSend(unsigned char *outputInterfaceMac, unsigned char *msgData, int msgLength);
void i5MessageReset(i5_message_type *pmsg);
int i5MessageGetNextTlvType(i5_message_type *pmsg);
i5_message_type *i5MessageNew(void);
int i5MessageCheckForQueryOnDeviceAndSocket(i5_socket_type *srcSock, unsigned char *srcAddr, int queryType);
void i5MessageTopologyDiscoverySend(i5_socket_type *psock);
void i5MessageRawTopologyQuerySend (i5_socket_type *psock, unsigned char *neighbor_al_mac_address, int withRetries, int queryType);
void i5MessageGenericPhyTopologyQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address);
void i5MessageTopologyQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address);
void i5MessageLinkMetricQuerySend(i5_socket_type *psock, unsigned char const * destAddr,
                                  unsigned char specifyNeighbor, unsigned char const * neighbor);
void i5MessageTopologyDiscoveryTimeout(void *arg);
unsigned char *i5MessageSrcMacAddressGet(i5_message_type *pmsg);
unsigned char i5MessageLastPacketFragmentIdentifierGet(i5_message_type *pmsg);
int i5MessageGetPacketSpace(i5_message_type *pmsg, unsigned int *currPacketSpace, unsigned int *nextPacketSpace);
int i5MessageInsertTlv(i5_message_type *pmsg, unsigned char const *buf, unsigned int len);
int i5MessageTlvExtract(i5_message_type *pmsg, unsigned int type, unsigned int *plength, unsigned char **ppvalue, char withReset);
unsigned short i5MessageVersionGet(i5_message_type *pmsg);
unsigned short i5MessageTypeGet(i5_message_type *pmsg);
void i5MessagePacketReceive(i5_socket_type *psock, i5_packet_type *ppkt);
void i5MessageTopologyQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address);
void i5MessageTopologyNotificationSend(unsigned char *bssid, unsigned char *mac, unsigned char isAssoc);
void i5MessageApAutoconfigurationWscSend(i5_socket_type * psock, unsigned char * macAddr, unsigned char const * wscPacket,
unsigned wscLen, unsigned char *radioMac);
void i5MessageApAutoconfigurationSearchSend(unsigned int freqBand);
void i5MessageApAutoconfigurationResponseSend(i5_message_type * pmsg_req, unsigned int freqBand,
  unsigned char *searcher_al_mac_address);
void i5MessageApAutoconfigurationRenewSend(unsigned int freqBand);
void i5MessagePushButtonEventNotificationSend(void);
unsigned int i5MessageSendLinkQueries(void);
void i5MessageSendRoutingTableMessage(i5_socket_type *psock, unsigned char const * destAddr, i5_routing_table_type *table);
void i5MessageDeinit( void );

#ifdef MULTIAP
int i5MessageSend(i5_message_type *pmsg, int relay);
void i5MessageFree(i5_message_type *pmsg);
/* Send AP Capability Query message to a Multi AP Devcice */
void i5MessageAPCapabilityQuerySend (i5_socket_type *psock, unsigned char *neighbor_al_mac_address);
/* Send AP Capability Report Message */
void i5MessageAPCapabilityReportSend(i5_message_type *pmsg_req);
/* Send MultiAP policy config message */
void i5MessageMultiAPPolicyConfigRequestSend (i5_socket_type *psock, unsigned char *neighbor_al_mac_address);
/* Send Client Capability Query message to a Multi AP Devcice */
void i5MessageClientCapabilityQuerySend (i5_socket_type *psock, unsigned char *neighbor_al_mac_address, unsigned char *mac, unsigned char *bssid);
/* Send Client Capability Report Message */
void i5MessageClientCapabilityReportSend(i5_message_type *pmsg_req, unsigned char *mac, unsigned char *bssid, int rc);
/* Send Client Steering Rquuest message to a Multi AP Device */
void i5MessageClientSteeringRequestSend(i5_socket_type *psock, unsigned char *neighbor_al,
  ieee1905_steer_req *steer_req, ieee1905_vendor_data *vndr_msg_data);
/* Send Steering completed message */
void i5MessageSteeringCompletedSend(i5_socket_type *psock, unsigned char *neighbor_al);
/* Send Client Association Control Rquuest message to a Multi AP Device */
void i5MessageClientAssociationControlRequestSend(i5_socket_type *psock, unsigned char *neighbor_al,
  ieee1905_block_unblock_sta *block_unblock_sta);
/* Send BTM Report to the controller */
void i5MessageClientSteeringBTMReportSend(i5_socket_type *psock, unsigned char *neighbor_al,
  ieee1905_btm_report *btm_report);
/* Send Higher Layer Data Payload to a Multi AP Devcice */
void i5MessageHigherLayerDataMessageSend (i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5HigherLayerProtocolField_Values protocol,
  unsigned char *data, unsigned int data_len);
/* Send ACK Message */
void i5Message1905AckSend(i5_message_type *pmsg_req);
void i5MessageChannelPreferenceQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address);
/* Send Channel Preference Report Message */
void i5MessageChannelPreferenceReportSend(i5_socket_type *psock, unsigned char *dst_mac,
  unsigned short msg_token, bool solicited_msg);
void i5MessageChannelPreferenceReportSendUnsolicited(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, unsigned char *data, unsigned short data_len);
void i5MessageChannelSelectionRequestSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, unsigned char *data, unsigned short data_len);
void i5MessageChannelSelectionResponseSend(i5_message_type *pmsg_req, uint8 resp_code);
i5_message_type *i5MessageChannelSelectionRequestCreate(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address);
/* Prepare the client association control request for all the BSS except the source and target */
int i5MessagePrepareandSendClientAssociationControl(ieee1905_client_assoc_cntrl_info *assoc_cntrl);
/* Send AP Metrics Query Message to an agent. All the BSSIDs are stored in the linear array */
void i5MessageAPMetricsQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address,
  unsigned char *bssids, unsigned char count);
/* Send AP Metrics Response Message. All the BSSIDs are stored in the linear array */
void i5MessageAPMetricsResponseSend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address,
  unsigned short msgIndentifier, unsigned char *bssids, unsigned char count,
  unsigned char *ifrMAC, unsigned char isGetStats);
/* Send Associated STA Link Metrics query to an agent */
void i5MessageAssociatedSTALinkMetricsQuerySend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, unsigned char *mac);
/* Send Associated STA Link Metrics Response. All the STA MAC are stored in the linear array */
void i5MessageAssociatedSTALinkMetricsResponseSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, unsigned short msgIndentifier, unsigned char *macs,
  unsigned char count);
/* Send UnAssociated STA Link Metrics query to an agent */
void i5MessageUnAssociatedSTALinkMetricsQuerySend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, ieee1905_unassoc_sta_link_metric_query *query);
/* Send UnAssociated STA Link Metrics Response */
void i5MessageUnAssociatedSTALinkMetricsResponseSend(i5_socket_type *psock,
  unsigned char *neighbor_al, ieee1905_unassoc_sta_link_metric *metric);
/* Sned Beacon Metrics Query Message */
void i5MessageBeaconMetricsQuerySend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address,
  ieee1905_beacon_request *query);
/* Sned Beacon Metrics response Message */
void i5MessageBeaconMetricsResponseSend(i5_socket_type *psock, unsigned char *neighbor_al_mac_address,
  ieee1905_beacon_report *report);
/* Used when application using 1905 lib, wants to send a Vendor Specific Message to other 1905 entity */
int i5MessageVendorSpecificMessageSend(i5_socket_type *psock,
   unsigned char *neighbor_al, ieee1905_vendor_data *msg_data);
/* Send WSC M1 Message */
void i5MessageApAutoconfigurationWscM1Send(i5_socket_type *psock, unsigned char *macAddr,
  unsigned char const *wscPacket, unsigned wscLen, unsigned char *radioMac);
/* Send WSC M2 Message */
void i5MessageApAutoconfigurationWscM2Send(i5_socket_type *psock, unsigned char *macAddr,
  unsigned char *radioMac);
/* Send Backhaul Steering Rquuest message to a Multi AP Device */
int i5MessageBackhaulSteeringRequestSend(i5_socket_type *psock, unsigned char *neighbor_al,
  ieee1905_backhaul_steer_msg *bh_steer_req);
/* Send Backhaul Steering Response message to a Multi AP Device */
void i5MessageBackhaulSteeringResponseSend(i5_socket_type *psock,
  unsigned char *neighbor_al, ieee1905_backhaul_steer_msg *bh_steer_resp);
/* Send Combined Infrastructure Metrics Message to a MultiAP Agent */
void i5MessageCombinedInfrastructureMetricsSend(i5_socket_type *psock, unsigned char *neighbor_al);
/* Send operating channel report from Multi AP agent to Multi AP controller */
int i5MessageOperatingChanReportSend(i5_socket_type *psock, unsigned char *dst_mac,
  ieee1905_operating_chan_report *chan_report);
#endif /* MULTIAP */

#endif // endif
