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

#ifndef _IEEE1905_TLV_H_
#define _IEEE1905_TLV_H_

#include "ieee1905_message.h"
#include "ieee1905.h"
/*
 * IEEE1905 TLVs
 */
enum i5TlvRole_Values {
  i5TlvRole_Registrar = 0,
  i5TlvRole_Reserved, /* All values 0x01 through through 0xff are reserved */
};

#define I5_TLV_ASSOC_EVENT_JOIN   0x80  /* 1 if client has joined the BSS else left the BSS */

#define i5TlvMediaSpecificInfoWiFi_Length    10
#define i5TlvMediaSpecificInfo1901_Length    7

#define i5TlvSearchRole_Length    1
#define i5TlvSupportedRole_Length 1

#define i5TlvAutoConfigFreqBand_Length 1
#define i5TlvSupportedFreqBand_Length  1

#define i5TlvPushButtonNotificationMediaCount_Length  1

enum i5TlvLinkMetricNeighbour_Values {
  i5TlvLinkMetricNeighbour_All = 0,
  i5TlvLinkMetricNeighbour_Specify,
  i5TlvLinkMetricNeighbour_Invalid, /* All values 0x02 through through 0xff are reserved */
};
#define i5TlvLinkMetricNeighbour_Length 1

enum i5TlvLinkMetricType_Values {
  i5TlvLinkMetricType_TxOnly = 0,
  i5TlvLinkMetricType_RxOnly,
  i5TlvLinkMetricType_TxAndRx,
  i5TlvLinkMetricType_Invalid, /* All values 0x03 through through 0xff are reserved */
};
#define i5TlvLinkMetricType_Length 1

enum i5TlvLinkMetricResultCode_Values {
  i5TlvLinkMetricResultCode_InvalidNeighbor = 0,
  i5TlvLinkMetricResultCode_Invalid, /* All values 0x01 through through 0xff are reserved */
};

#define i5TlvLinkMetricResultCode_Length 1

#define i5TlvLinkMetricTxOverhead_Length 12
#define i5TlvLinkMetricTxPerLink_Length 29

#define i5TlvLinkMetricRxOverhead_Length 12
#define i5TlvLinkMetricRxPerLink_Length 23

#ifdef MULTIAP
#define i5TlvAPRadioIndentifierType_Length  6
#define i5TlvClientAssociationEvent_Length  13
#define i5TlvClientInfo_Length              12
#define i5TlvAPCapabilities_Length	    1
#define i5TlvAPHTCapabilities_Length	    7
#define i5TlvAPVHTCapabilities_Length	    12
#define i5TlvAPHECapabilities_Min_Length    9
#define i5TlvSteeringPolicyType_Min_Length  3
#define i5TlvMetricReportingPolicyType_Min_Length  2
#define i5TlvMetricReportingPolicyInterface_Min_Length  10
#define i5TlvSteeringRequest_Min_Length 12
#define i5TlvClientAssociationControlRequest_Min_Length 10
#define i5TlvSteeringBTMReport_Min_Length 13
#define i5TlvAPMetrics_Min_Length 13
#define i5TlvAssociatedSTATrafficStats_Min_Length 6
#define i5TlvSTAMACAddress_Length 6
#define i5TlvAssociatedSTALinkMetric_Min_Length 7
#define i5TlvAssociatedSTALinkMetric_BSS_Length 19
#define i5TlvBhSteeringRequest_Length 14
#define i5TlvBhSteeringResponse_Length 13
#define i5TlvBeaconMetricsQuery_Min_Length  18 /* includes only SSIDlen, ChanrepCount and elementIDCount */
#define i5TlvBeaconMetricsRespomse_Min_Length  8
#define i5TlvErrorCode_Length  7
#define i5TlvOperatingChannelReport_Min_Length  9

typedef enum i5TlvClientCapabilityReporResultCode_Values {
  i5TlvClientCapabilityReporResultCode_Success = 0,
  i5TlvClientCapabilityReporResultCode_Failure, /* All values 0x02 through 0xff are reserved */
} i5TlvClientCapabilityReporResultCode_Values_t;

#endif /* MULTIAP */

typedef struct {
  unsigned char type;
  unsigned short length;
} __attribute__((__packed__)) i5_tlv_t;

/* Interface Structures */
typedef struct {
  unsigned char  localInterface[6];
  unsigned char  neighborInterface[6];
  unsigned short intfType;
  unsigned char  ieee8021BridgeFlag;
  unsigned int   packetErrors;
  unsigned int   transmittedPackets;
  unsigned short macThroughPutCapacity;
  unsigned short linkAvailability;
  unsigned short phyRate;
} i5_tlv_linkMetricTx_t;

typedef struct {
  unsigned char  localInterface[6];
  unsigned char  neighborInterface[6];
  unsigned short intfType;
  unsigned int   packetErrors;
  unsigned int   receivedPackets;
  unsigned char  rcpi;
} i5_tlv_linkMetricRx_t;

typedef struct {
  unsigned char interfaceMac [ETH_ALEN];
  int address;
  int netmask;
  int gateway;
  int dhcpServer;
} i5_tlv_ipv4Type_t;

typedef struct {
  unsigned char interfaceMac [ETH_ALEN];
  /* TBD - kiwin */
} i5_tlv_ipv6Type_t;

int i5_cpy_host16_to_netbuf(unsigned char *dst, unsigned short src);
int i5_cpy_host32_to_netbuf(unsigned char *dst, unsigned int src);
int i5_cpy_netbuf_to_host16(unsigned short *dst, unsigned char *src);
int i5_cpy_netbuf_to_host32(unsigned int *dst, unsigned char *src);

int i5TlvIsEndOfMessageType(int tlvType);
char const *i5TlvGetTlvTypeString(int tlvType);
int i5TlvEndOfMessageTypeInsert(i5_message_type *pmsg);
int i5TlvEndOfMessageTypeExtract(i5_message_type *pmsg);
int i5TlvAlMacAddressTypeInsert(i5_message_type *pmsg);
int i5TlvAlMacAddressTypeExtract(i5_message_type *pmsg, unsigned char *mac_address);
int i5TlvMacAddressTypeInsert(i5_message_type *pmsg, unsigned char *mac_address);
int i5TlvMacAddressTypeExtract(i5_message_type *pmsg, unsigned char *mac_address);
int i5TlvDeviceInformationTypeInsert(i5_message_type *pmsg, unsigned char useLegacyHpav, char* containsGenericPhy);
int i5TlvDeviceInformationTypeExtract(i5_message_type *pmsg, unsigned char *neighbor_al_mac_address, unsigned char *deviceHasGenericPhy);
int i5TlvDeviceInformationTypeExtractAlMac(i5_message_type *pmsg, unsigned char *neighbor_al_mac_address);
int i5TlvGenericPhyTypeInsert (i5_message_type *pmsg);
int i5TlvGenericPhyTypeExtract (i5_message_type *pmsg);
int i5TlvDeviceBridgingCapabilityTypeInsert(i5_message_type *pmsg);
int i5TlvDeviceBridgingCapabilityTypeExtract(i5_message_type *pmsg, unsigned char *pdevid);
int i5TlvLegacyNeighborDeviceTypeInsert(i5_message_type *pmsg);
int i5TlvLegacyNeighborDeviceTypeExtract(i5_message_type *pmsg, unsigned char *pdevid);
int i5Tlv1905NeighborDeviceTypeInsert(i5_message_type *pmsg);
int i5Tlv1905NeighborDeviceTypeExtract(i5_message_type *pmsg, unsigned char *pdevid);
int i5TlvSearchedRoleTypeInsert(i5_message_type *pmsg);
int i5TlvSearchedRoleTypeExtract(i5_message_type *pmsg, unsigned char *searchRole);
int i5TlvDoesMediaTypeMatchLocalFreqBand(unsigned short mediaType);
int i5TlvDoesFreqBandMatchLocalFreqBand(unsigned char *incomingFreqBandValue);
int i5TlvAutoconfigFreqBandTypeInsert(i5_message_type *pmsg, unsigned int freqBand);
int i5TlvAutoconfigFreqBandTypeExtract(i5_message_type *pmsg, unsigned char *autoconfigFreqBand);
int i5TlvSupportedRoleTypeInsert(i5_message_type *pmsg);
int i5TlvSupportedRoleTypeExtract(i5_message_type *pmsg, unsigned char *supportedRole);
int i5TlvSupportedFreqBandTypeInsert(i5_message_type *pmsg, unsigned int freqBand);
int i5TlvSupportedFreqBandTypeExtract(i5_message_type *pmsg, unsigned char *supportedFreqBand);
unsigned int i5TlvGetFreqBandFromMediaType(unsigned short mediaType);
int i5TlvWscTypeInsert(i5_message_type *pmsg, unsigned char const * wscPacket, unsigned wscLength);
int i5TlvWscTypeExtract(i5_message_type *pmsg, unsigned char * wscPacket, unsigned maxWscLength, unsigned *actualWscLength);
int i5TlvWscTypeM2Extract(i5_message_type *pmsg);
int i5TlvVendorGuestSsidExtract(i5_message_type *pmsg, ieee1905_vendor_data *msg_data);
int i5TlvPushButtonEventNotificationTypeInsert(i5_message_type *pmsg, unsigned char* genericPhyIncluded);
int i5TlvPushButtonEventNotificationTypeExtract(i5_message_type * pmsg, unsigned int *pMediaCount, unsigned short **pMediaList);
int i5TlvPushButtonGenericPhyEventNotificationTypeExtract (i5_message_type * pmsg, unsigned int *pMediaCount, unsigned char **pMediaList);
int i5TlvPushButtonEventNotificationTypeExtractFree(unsigned short *pMediaList);
int i5TlvPushButtonGenericPhyEventNotificationTypeExtractFree(unsigned char *pPhyMediaList);
int i5TlvPushButtonGenericPhyEventNotificationTypeInsert(i5_message_type *pmsg);
int i5TlvLinkMetricQueryInsert (i5_message_type * pmsg, enum i5TlvLinkMetricNeighbour_Values specifyAddress,
                                unsigned char const * mac_address, enum i5TlvLinkMetricType_Values metricTypes);
int i5TlvLinkMetricQueryExtract(i5_message_type * pmsg,
                                unsigned char * neighbours,
                                unsigned char * alMacAddress,
                                enum i5TlvLinkMetricType_Values * metricsRequested);
int i5TlvLinkMetricResultCodeInsert (i5_message_type * pmsg);
void i5TlvLinkMetricResponseExtract(i5_message_type * pmsg);
int i5TlvLinkMetricTxInsert (i5_message_type * pmsg,
                             unsigned char const * local_al_mac, unsigned char const * neighbor_al_mac,
                             i5_tlv_linkMetricTx_t const * txStats, int numLinks);
int i5TlvLinkMetricTxExtract (i5_message_type * pmsg,
                              unsigned char * reporter_al_mac, unsigned char * neighbor_al_mac,
                              i5_tlv_linkMetricTx_t * txStats, int maxLinks, int *numLinksReturned);
int i5TlvLinkMetricRxInsert (i5_message_type * pmsg,
                             unsigned char const * local_al_mac, unsigned char const * neighbor_al_mac,
                             i5_tlv_linkMetricRx_t const * rxStats, int numLinks);
int i5TlvLinkMetricRxExtract (i5_message_type * pmsg,
                              unsigned char * reporter_al_mac, unsigned char * neighbor_al_mac,
                              i5_tlv_linkMetricRx_t * rxStats, int maxLinks, int *numLinksReturned);

int i5TlvLldpTypeInsert(i5_message_type *pmsg, const unsigned char *chassis_mac, const unsigned char *portid_mac);
int i5TlvLldpTypeExtract(i5_message_type *pmsg, unsigned char *neighbor_al_mac, unsigned char *neighbor_interface_mac);
int i5TlvVendorSpecificTypeInsert(i5_message_type *pmsg, unsigned char *vendorSpec_msg, unsigned int vendorSpec_len);
int i5TlvVendorSpecificTypeExtract(i5_message_type *pmsg, char withReset,
  unsigned char **vendorSpec_data, unsigned int * vendorSpec_len);

/* Broadcom vendor specific TLVs */
int i5Tlv_brcm_RoutingTableInsert (i5_message_type * pmsg, i5_routing_table_type *table);
int i5TlvFriendlyNameInsert(i5_message_type *pmsg, char const *friendlyName);
int i5TlvFriendlyNameExtract(i5_message_type *pmsg, char *friendlyName, int maxFriendlyNameSize);
int i5TlvFriendlyUrlInsert(i5_message_type *pmsg, unsigned char const *controlUrl);
int i5TlvFriendlyUrlExtract(i5_message_type *pmsg, unsigned char *controlUrl, int maxControlUrlSize);

#ifdef MULTIAP
/* Multi AP Specific TLV's */
int i5TlvSupportedServiceTypeInsert(i5_message_type *pmsg);
int i5TlvSupportedServiceTypeExtract(i5_message_type *pmsg, unsigned int *supportedService);
int i5TlvSearchedServiceTypeInsert(i5_message_type *pmsg, unsigned int searchService);
int i5TlvSearchedServiceTypeExtract(i5_message_type *pmsg, unsigned int *searchedService);
int i5TlvAPRadioIndentifierTypeInsert(i5_message_type *pmsg, unsigned char *mac);
int i5TlvAPRadioIndentifierTypeExtract(i5_message_type *pmsg, unsigned char *out_mac);
/* TLV to indicate all BSS(s) it is currently operating on each of its radios */
int i5TlvAPOperationalBSSTypeInsert(i5_message_type *pmsg);
/* Extract information from AP operational BSS TLV */
int i5TlvAPOperationalBSSTypeExtract(i5_message_type *pmsg, unsigned char *pdevid);
/* TLV to indicate all the 802.11 clients that are directly associated with each of the BSS(s)
 * that is operated by the Multi-AP Agent
 */
int i5TlvAssocaitedClientsTypeInsert(i5_message_type *pmsg);
/* Extract clients associated to the BSS */
int i5TlvAssocaitedClientsTypeExtract(i5_message_type *pmsg, unsigned char *pdevid);
/* TLV to indicate a 802.11 client joins or leaves a BSS */
int i5TlvClientAssociationEventTypeInsert(i5_message_type *pmsg, unsigned char *bssid, unsigned char *mac, unsigned char isAssoc);
/* Extract Client Association Event TLV */
int i5TlvClientAssociationEventTypeExtract(i5_message_type *pmsg, unsigned char *neighbor_al_mac_address);
/* TLV to get the client info */
int i5TlvClientInfoTypeInsert(i5_message_type *pmsg, unsigned char *mac, unsigned char *bssid);
/* Extract Client Info TLV */
int i5TlvClientInfoTypeExtract(i5_message_type *pmsg, unsigned char *mac, unsigned char *bssid);
/* TLV to report the client capability */
int i5TlvClientCapabilityReportTypeInsert(i5_message_type *pmsg, unsigned char result, unsigned char *frame, unsigned int frame_len);
/* Extract Client capability TLV. Free the frame variable once used */
int i5TlvClientCapabilityReportTypeExtract(i5_message_type *pmsg,
  unsigned char *neighbor_al_mac_address, unsigned char *mac, unsigned char *bssid,
  i5TlvClientCapabilityReporResultCode_Values_t *res);
/* TLV to report the AP basic Capabilities */
int i5TlvAPCapabilitiesTypeInsert(i5_message_type *pmsg, unsigned char cap);
/* Extract AP basic Capabilities TLV */
int i5TlvAPCapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* TLV to report the AP Radios basic Capabilities */
int i5TlvAPRadioBasicCapabilitiesTypeInsert(i5_message_type *pmsg, unsigned char *mac, unsigned char bssCount, unsigned char *data, unsigned char len);
/* Extract Multiple AP Radios basic Capabilities TLV */
int i5TlvAPRadioBasicCapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* Extract AP Radios basic Capabilities TLV got in WSC M1 message */
int i5TlvAPRadioBasicCapabilitiesTypeExtractFromWSCM1(i5_message_type *pmsg, unsigned char *outMac,
  ieee1905_radio_caps_type *RadioCaps);
/* TLV to report the AP HT Capabilities */
int i5TlvAPHTCapabilitiesTypeInsert(i5_message_type *pmsg, unsigned char *mac, unsigned char cap);
/* Extract AP HT Capabilities TLV */
int i5TlvAPHTCapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* TLV to report the AP VHT Capabilities */
int i5TlvAPVHTCapabilitiesTypeInsert(i5_message_type *pmsg, unsigned char *mac, unsigned short txMCSMap, unsigned short rxMCSmap, unsigned char capsEx, unsigned char caps);
/* Extract AP VHT Capabilities TLV */
int i5TlvAPVHTCapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* TLV to report the AP HE Capabilities */
int i5TlvAPHECapabilitiesTypeInsert(i5_message_type *pmsg, unsigned char *mac, ieee1905_he_caps_type* HECaps);
/* Extract AP HE Capabilities TLV */
int i5TlvAPHECapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* TLV to insert steering policy */
int i5TlvSteeringPolicyTypeInsert(i5_message_type *pmsg, unsigned char *neighbor_al_mac_address,
  ieee1905_policy_config *list);
/* TLV to extract steering policy */
int i5TlvSteeringPolicyTypeExtract(i5_message_type *pmsg);
/* TLV to insert metric reporting policy */
int i5TlvMetricReportingPolicyTypeInsert(i5_message_type *pmsg, unsigned char *neighbor_al_mac_address,
  ieee1905_policy_config *configlist);
/* TLV to extract metric reporting policy */
int i5TlvMetricReportingPolicyTypeExtract(i5_message_type *pmsg);
/* TLV to Send the Client Steering Request */
int i5TlvSteeringRequestTypeInsert(i5_message_type *pmsg, ieee1905_steer_req *steer_req);
/* Extract Client Steering Request TLV */
int i5TlvSteeringRequestTypeExtract(i5_message_type *pmsg, ieee1905_steer_req *steer_req);
/* TLV to Send the Client Association Control Request */
int i5TlvClientAssociationControlRequestTypeInsert(i5_message_type *pmsg,
  ieee1905_block_unblock_sta *block_unblock_sta);
/* Extract Client Association Control Request TLV */
int i5TlvClientAssociationControlRequestTypeExtract(i5_message_type *pmsg,
  ieee1905_block_unblock_sta *block_unblock_sta);
/* TLV to add BTM Report */
int i5TlvSteeringBTMReportTypeInsert(i5_message_type *pmsg, ieee1905_btm_report *btm_report);
/* Extract BTM Report TLV */
int i5TlvSteeringBTMReportTypeExtract(i5_message_type *pmsg, ieee1905_btm_report *btm_report);
/* TLV to report the Higher layer data TLV */
int i5TlvHigherLayerDataTypeInsert(i5_message_type *pmsg,
  i5HigherLayerProtocolField_Values protocol, unsigned char *data, unsigned int data_len);
/* Extract Higher layer data TLV */
int i5TlvHigherLayerDataTypeExtract(i5_message_type *pmsg);
int i5TlvChannelPreferenceTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  unsigned char *data, unsigned char len);
int i5TlvChannelPreferenceTypeInsert_Stored(i5_message_type *pmsg, unsigned char *mac,
  ieee1905_chan_pref_rc_map_array *cp);
int i5TlvChannelPreferenceTypeExtract(i5_message_type *pmsg, int isAgent);
int i5TlvTransmitPowerLimitTypeExtract(i5_message_type *pmsg);
int i5TlvChannelSelectionResponseTypeInsert(i5_message_type *pmsg, unsigned char *mac, uint8 resp_code);
int i5TlvChannelSelectionResponseTypeExtract(i5_message_type *pmsg);
int i5TlvOperatingChannelReportTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  uint8 channel, uint8 rclass, int8 tx_pwr);
/* Extract Operating Channel report TLV */
int i5TlvOperatingChannelReportTypeExtract(i5_message_type *pmsg);
/* TLV to add Ap Metric Query. All the BSSIDs are stored in the linear array */
int i5TlvAPMetricQueryTypeInsert(i5_message_type *pmsg, unsigned char *bssids, unsigned char count);
/* Extract Ap Metric Query TLV */
int i5TlvAPMetricQueryTypeExtract(i5_message_type *pmsg, unsigned char **bssids_out,
  unsigned char *count);
/* TLV to add Ap Metrics */
int i5TlvAPMetricsTypeInsert(i5_message_type *pmsg, unsigned char *bssid, unsigned short sta_count,
  ieee1905_ap_metric *apMetric, ieee1905_interface_metric *ifrMetric);
/* Extract Ap Metrics TLV */
int i5TlvAPMetricsTypeExtract(i5_message_type *pmsg, unsigned char isCombined);
/* TLV to add Associated STA Traffic Stats TLV */
int i5TlvAssociatedSTATrafficStatsTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  ieee1905_sta_traffic_stats *traffic_stat);
/* Extract Associated STA Traffic Stats TLV */
int i5TlvAssociatedSTATrafficStatsTypeExtract(i5_message_type *pmsg);
/* TLV to add STA MAC Address Type TLV */
int i5TlvSTAMACAddressTypeInsert(i5_message_type *pmsg, unsigned char *mac);
/* Extract STA MAC Address Type TLV */
int i5TlvSTAMACAddressTypeExtract(i5_message_type *pmsg, unsigned char *mac);
/* TLV to add Associated STA Link Metrics TLV */
int i5TlvAssociatedSTALinkMetricsTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  unsigned char *bssid, ieee1905_sta_link_metric *link_metric);
/* Extract Associated STA Link Metrics TLV */
int i5TlvAssociatedSTALinkMetricsTypeExtract(i5_message_type *pmsg, int *sta_found);
/* TLV to add UnAssociated STA Link Metrics Query TLV */
int i5TlvUnAssociatedSTALinkMetricsQueryTypeInsert(i5_message_type *pmsg,
  ieee1905_unassoc_sta_link_metric_query *query);
/* Extract UnAssociated STA Link Metrics Query TLV */
int i5TlvUnAssociatedSTALinkMetricsQueryTypeExtract(i5_message_type *pmsg,
  ieee1905_unassoc_sta_link_metric_query **query);
/* TLV to add UnAssociated STA Link Metrics Response TLV */
int i5TlvUnAssociatedSTALinkMetricsResponseTypeInsert(i5_message_type *pmsg,
  ieee1905_unassoc_sta_link_metric *metric);
/* Extract UnAssociated STA Link Metrics Response TLV */
int i5TlvUnAssociatedSTALinkMetricsResponseTypeExtract(i5_message_type *pmsg,
  ieee1905_unassoc_sta_link_metric *metric);
/* TLV to add Beacons metrics query TLV */
int i5TlvBeaconMetricsQueryTypeInsert(i5_message_type *pmsg, ieee1905_beacon_request *query);
/* Extract Beacons metrics query TLV */
int i5TlvBeaconMetricsQueryTypeExtract(i5_message_type *pmsg, ieee1905_beacon_request *query);
/* TLV to add Beacons metrics response TLV */
int i5TlvBeaconMetricsResponseTypeInsert(i5_message_type *pmsg, ieee1905_beacon_report *report);
/* Extract Beacons metrics response TLV */
int i5TlvBeaconMetricsResponseTypeExtract(i5_message_type *pmsg, ieee1905_beacon_report *report);
/* TLV to insert the Backhaul Steering Request */
int i5TlvBhSteeringRequestTypeInsert(i5_message_type *pmsg, ieee1905_backhaul_steer_msg *bh_steer_req);
/* Extract Backhaul Steering Request */
int i5TlvBhSteeringRequestTypeExtract(i5_message_type *pmsg, ieee1905_backhaul_steer_msg *bh_steer_req);
/* TLV to insert the Backhaul Steering Response */
int i5TlvBhSteeringResponseTypeInsert(i5_message_type *pmsg, ieee1905_backhaul_steer_msg *bh_steer_resp);
/* Extract Backhaul Steering Response */
int i5TlvBhSteeringResponseTypeExtract(i5_message_type *pmsg, ieee1905_backhaul_steer_msg *bh_steer_resp);
/* TLV to insert the Error Codes */
int i5TlvErrorCodeTypeInsert(i5_message_type *pmsg, ieee1905_tlv_err_codes_t err,
  unsigned char *sta_mac);
/* Extract Error Codes */
int i5TlvErrorCodeTypeExtract(i5_message_type *pmsg, ieee1905_tlv_err_codes_t *err,
  unsigned char *sta_mac);
#endif /* MULTIAP */

#endif // endif
