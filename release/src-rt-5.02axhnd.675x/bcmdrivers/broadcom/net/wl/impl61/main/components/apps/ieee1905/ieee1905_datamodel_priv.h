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

#ifndef _IEEE1905_DATAMODEL_PRIV_H_
#define _IEEE1905_DATAMODEL_PRIV_H_

/*
 * IEEE1905 Data-Model
 */

#include "ieee1905_datamodel.h"
#include "ieee1905_socket.h"
#include "ieee1905_message.h"

extern i5_dm_network_topology_type i5_dm_network_topology;

int i5DmIsMacNull(unsigned char *mac);
int i5DmIsMacWildCard(unsigned char *mac);
int i5DmAnyWirelessInterfaceUp(i5_dm_device_type *deviceToCheck);
int i5DmDeviceIsSelf(unsigned char *device_id);
/* Get the self device pointer */
i5_dm_device_type *i5DmGetSelfDevice();
void i5DmDeviceFree(i5_dm_device_type *device);
/* If STA is Backhaul STA, Remove the device also from Topology */
void i5DmDeviceFreeForBackhaulSTA(i5_dm_device_type const *parent_dev, unsigned char *bSTA_mac);
i5_dm_interface_type *i5DmInterfaceGetLocalPlcInterface(void);
int i5DmIsInterfaceWireless(unsigned short mediaType);
int i5DmIsInterfacePlc(unsigned short mediaType, unsigned char const *netTechOui);
int i5DmIsInterfaceEthernet(unsigned short mediaType);
i5_dm_interface_type *i5DmInterfaceFind(i5_dm_device_type const *parent, unsigned char const *interface_id);
/* Find the Interface from all the device in the network */
i5_dm_interface_type *i5DmFindInterfaceFromNetwork(unsigned char *interface_id);
void i5DmLinkMetricsActivate(void);
void i5DmSetLinkMetricInterval (unsigned int newIntervalMsec);
unsigned char const* i5DmGetNameForMediaType(unsigned short mediaType);
unsigned int i5DmInterfaceStatusGet(unsigned char *device_id, unsigned char *interface_id);
void i5DmInterfaceStatusSet(unsigned char *device_id, unsigned char * interfaceId, int ifindex, unsigned int status);
void i5DmInterfacePending(unsigned char *device_id);
void i5DmInterfaceDone(unsigned char *device_id);
int i5DmInterfaceUpdate(unsigned char *device_id, unsigned char *interface_id, int version, unsigned short media_type,
                        unsigned char const *media_specific_info, unsigned int media_specific_info_size,
                        i5MacAddressDeliveryFunc deliverFunc, char const *ifname, unsigned char status);
int i5DmInterfacePhyUpdate(unsigned char *device_id, unsigned char *interface_id,
                           unsigned char const *pNetTechOui, unsigned char const *pNetTechVariant, unsigned char const *pNetTechName, unsigned char const *url);
int i5DmBridgingTupleUpdate(unsigned char *device_id, int version, char *ifname, unsigned char tuple_num_macaddrs, unsigned char *bridging_tuple_list);
int i5DmBridgingTuplePending(unsigned char *device_id);
int i5DmBridgingTupleDone(unsigned char *device_id);
int i5DmLegacyNeighborPending(unsigned char *device_id);
int i5DmLegacyNeighborDone(unsigned char *device_id);
int i5DmLegacyNeighborUpdate(unsigned char *device_id, unsigned char *local_interface_id, unsigned char *neighbor_interface_id);
i5_dm_legacy_neighbor_type *i5DmLegacyNeighborFind(i5_dm_device_type *parent, unsigned char *local_interface_id, unsigned char *neighbor_interface_id);
int i5Dm1905NeighborPending(unsigned char *device_id);
int i5Dm1905NeighborDone(unsigned char *device_id);
int i5Dm1905NeighborUpdate(unsigned char *device_id, unsigned char *local_interface_id, unsigned char *neighbor1905_al_mac_address, unsigned char *neighbor1905_interface_id,
                           unsigned char *intermediate_legacy_bridge, char *localifName, int localifindex, unsigned char createNeighbor);
i5_dm_1905_neighbor_type *i5Dm1905GetLocalNeighbor (unsigned char const *neighbor1905_interface_id);
i5_dm_1905_neighbor_type *i5Dm1905NeighborFind(i5_dm_device_type *parent, unsigned char *local_interface_id, unsigned char *neighbor1905_al_mac_address);
int i5Dm1905NeighborBandwidthUpdate (i5_dm_1905_neighbor_type *neighbor, unsigned short MacThroughputCapacity,
                                     unsigned short availableThroughputCapacity, unsigned int rxBytesCumulative, unsigned char* sourceAlMac);
i5_dm_interface_type *i5Dm1905GetLocalInterface(i5_dm_1905_neighbor_type const *neighbor);
void i5DmDeviceQueryStateSet(unsigned char *device_id, unsigned char state);
unsigned char i5DmDeviceQueryStateGet(unsigned char const *device_id);
void i5DmWaitForGenericPhyQuery(i5_dm_device_type *destDev);
unsigned char i5DmAreThereNodesWithVersion(int nodeVersion);
i5_dm_device_type *i5DmDeviceNew(unsigned char *device_id, int version, char const* pDevFriendlyName);
i5_dm_device_type *i5DmDeviceFind(unsigned char const *device_id);
void i5DmDeviceDelete(unsigned char *device_id);
void i5DmDeviceTopologyQuerySendToAllNew(i5_socket_type *psock);
int i5DmDeviceTopologyChangeProcess(unsigned char *device_id);
void i5DmDeviceNewIfNew(unsigned char *neighbor_al_mac_address);
void i5DmTopologyFreeUnreachableDevices(bool idle_check);
void i5DmDeviceFreeUnreachableNeighbors(unsigned char *device_id, int ifindex, unsigned char *neighbor_interface_list, unsigned int length);
void i5DmDeviceRemoveStaleNeighborsTimer(void *arg);
int i5DmIsWifiBandSupported(char *ifname, unsigned int freqBand);
/* Get the local ifname from the neibhors ALID and MAC address based on the media specific info */
int i5DmGetIfnameFromMediaSpecific(char *ifname, unsigned char *alid, unsigned char *mac);

int  i5DmCtlSizeGet(void);
void i5DmCtlAlMacRetrieve(char *pMsgBuf);
void i5DmCtlRetrieve(char * pMsgBuf);

void i5DmRetryPlcRegistry(void);
/* Find Neighbor other than Remote Interface for Same Neighbor AL MAC */
i5_dm_1905_neighbor_type *i5Dm1905FindNeighborOtherThanRemoteInterface(i5_dm_device_type const *parent,
                                  unsigned char *neighbor1905_al_mac_address, unsigned char const *remote_interface_id);
i5_dm_1905_neighbor_type *i5Dm1905FindNeighborByRemoteInterface(i5_dm_device_type const *parent, unsigned char const *remote_interface_id);
int i5DmGetInterfacesWithNeighbor(unsigned char const *neighbor_al_mac,
                                  unsigned char * local_interface_mac_addrs, unsigned char * neighbor_interface_mac_addrs, int maxInterfaces);

void i5Dm1905NeighborUpdateIntermediateBridgeFlag(i5_dm_device_type *device, i5_dm_1905_neighbor_type *neighbor, unsigned char bridgeFlag);
void i5DmRefreshDeviceTimer(unsigned char *alMacAddress, char createFlag);
void i5DmSetFriendlyName(const char * name);
int  i5DmInit(unsigned int supServiceFlag, int isRegistrar, unsigned char device_mode);
void i5DmDeinit(void);
i5_wsc_m2_type *i5DmM2New(unsigned char *m2, unsigned int m2_len);
void i5DmM2ListFree();
/* Update the parent devices for all the devices in the network */
void i5DmUpdateParentDevice();

#ifdef MULTIAP
/* Get the regulatory class to channel map data */
const i5_dm_rc_chan_map_type *i5DmGetRegClassChannelMap(void);
/* Count the number of agents in the network */
unsigned int i5DmCountNumOfAgents();
/* Count the number of agents with backhaul BSS */
unsigned int i5DmCountNumOfAgentsWithbBSS();
/* Find the BSS from all the device in the network */
i5_dm_bss_type *i5DmFindBSSFromNetwork(unsigned char *bssid);
/* Find the BSS in a device */
i5_dm_bss_type *i5DmFindBSSFromDevice(i5_dm_device_type *pdmdev, unsigned char *bssid);
/* Find the BSS in a Interface */
i5_dm_bss_type *i5DmFindBSSFromInterface(i5_dm_interface_type *pdmif, unsigned char *bssid);
/* Find the BSS with matching SSID in the device */
i5_dm_bss_type *i5DmFindBSSMatchingSSIDFromDevice(i5_dm_device_type *pdmdev,
  ieee1905_ssid_type *ssid);
/* Find the Client in a BSS */
i5_dm_clients_type *i5DmFindClientInBSS(i5_dm_bss_type *pdmbss, unsigned char *mac);
/* Find the Client in a Device */
i5_dm_clients_type *i5DmFindClientInDevice(i5_dm_device_type *pdmdev, unsigned char *mac);
/* Find the Client in a Interface */
i5_dm_clients_type *i5DmFindClientInInterface(i5_dm_interface_type *pdmif, unsigned char *mac);
/* Create New BSS */
i5_dm_bss_type *i5DmBSSNew(i5_dm_interface_type *parent, unsigned char *bssid, unsigned char *ssid,
  unsigned char ssid_len, unsigned char mapFlags);
/* Remove BBS from the interface */
int i5DmBSSFree(i5_dm_interface_type *parent, i5_dm_bss_type *bss);
/* Create New Client */
i5_dm_clients_type *i5DmClientNew(i5_dm_bss_type *parent, unsigned char *mac, struct timeval assoc_tm);
/* Remove STA from the BSS */
int i5DmClientFree(i5_dm_bss_type *parent, i5_dm_clients_type *client);
/* Add the BSS and update the SSID */
int i5DmBSSUpdate(unsigned char *device_id, unsigned char *interface_id, unsigned char *bssid,
  ieee1905_ssid_type *ssid);
/* Add the Client and update the time */
int i5DmClientUpdate(i5_message_type *pmsg, i5_dm_device_type *pdevice, unsigned char *bssid,
  unsigned char *mac, unsigned short time_elapsed);
/* Update the client capability */
void i5DmUpdateClientCapability(i5_dm_clients_type *pdmclient, unsigned char *assoc_frame,
  unsigned int assoc_frame_len);
/* Associate a client to device */
int i5DmAssociateClient(i5_message_type *pmsg, i5_dm_device_type *pdmdev, unsigned char *bssid,
  unsigned char *mac, unsigned short time_elapsed, unsigned char notify, unsigned char *assoc_frame,
  unsigned int assoc_frame_len);
/* Disassociate a client from device */
int i5DmDisAssociateClient(i5_dm_device_type *pdmdev, unsigned char *bssid, unsigned char *mac, unsigned char notify);
/* Free the memory allocated for STAs and BSSs in steer request structure */
void i5DmSteerRequestInfoFree(ieee1905_steer_req *steer_req);
/* Closes the steering opportunity timer */
void i5DmDeviceSteerOpportunityTimeout(void *arg);
/* Creates Steering Opportunity Timer for the particular device */
void i5DmSteerOpportunityTimer(i5_dm_device_type *destDev, unsigned short opportunity_window);
/* Check if is there any steer opportunity for the device */
int i5DmIsSteerOpportunity(i5_dm_device_type *destDev);
/* Free the memory allocated for block unblock request structure */
void i5DmBlockUnblockInfoFree(ieee1905_block_unblock_sta *block_unblock_sta);
/* Find controller in the network */
i5_dm_device_type *i5DmFindController();
/* Delete all the node from generic list */
void i5DmGlistCleanup(ieee1905_glist_t *list);
/* Free the memory allocated for config list structure */
void i5DmConfigListFree(ieee1905_policy_config *config_list);
/* Find if the MAC is in ieee1905_sta_list list */
int i5DmIsMACInList(ieee1905_glist_t *list, unsigned char *mac);
/* Update the transmitter and reciever link metric. updateFlag is 0 for TX, 1 for Rx and 2 for Both */
int i5Dm1905NeighborLinkMetricUpdate(i5_dm_1905_neighbor_type *neighbor,
  ieee1905_backhaul_link_metric *metric, unsigned int rxBytesCumulative, unsigned char updateFlag);
/* Get the Channel to Regulatory class mapping array */
i5_dm_chan_rc_map_type *i5DmGetChannelRCMap(unsigned int *rc_map_count);
/* Get the Regulatory class to Channel mapping array */
i5_dm_rc_chan_map_type *i5DmGetRCChannelMap(unsigned int *reg_class_count);
/* Check whether the channel is valid in the regulatory class */
int i5DmIsChannelValidInRC(unsigned char channel, unsigned char rc);
/* Update the DFS status of the current operating channel from channel preference report */
void i5DmUpdateDFSStatusFromChannelPreference(i5_dm_interface_type *ifr);
/* Copy AP Capability */
int i5DmCopyAPCaps(ieee1905_ap_caps_type *ToApCaps, ieee1905_ap_caps_type *FromApCaps);
/* Copy Radio Capability */
int i5DmCopyRadioCaps(ieee1905_radio_caps_type *ToRadioCaps,
  ieee1905_radio_caps_type *FromRadioCaps);
/* Check if all the wireless interfaces configured */
int i5DmIsAllInterfacesConfigured();
/* Check if M1 sent to all the wireless interfaces configured */
int i5DmIsM1SentToAllWirelessInterfaces();
/* Check if M2 received by all the wireless interfaces configured */
int i5DmIsM2ReceivedByAllWirelessInterfaces();
/* Pre configure all virtual radios */
void i5DmPreConfigureVirtualInterfaces();
/* Process AP Metric Reporting Policy */
void i5DmProcessAPMetricReportingPolicy();
/* Find the interface metric policy */
ieee1905_ifr_metricrpt *i5DmFindMetricReportPolicy(unsigned char *ifrMAC);
/* Helper function to derive Bandwidth from Global operating class */
uint i5DmGetBandWidthFromOpClass(unsigned char opClass);
/* Is this neighbor device running in the self device */
int i5DmIsNeighborDeviceOnSameDevice(i5_dm_device_type *neighbor_device);
/* Unset the state of all BSS and clients */
void i5DmBSSClientPending(i5_dm_device_type *device);
/* Remove all the BSS and clients with state not set */
void i5DmBSSClientDone(i5_dm_device_type *device);
/* Update AP Caps */
void i5DmUpdateAPCaps(char *ifname, i5_dm_interface_type *pdmif);
/* Free Radio Caps */
void i5DmFreeRadioCaps(ieee1905_radio_caps_type *RadioCaps);
/* Add the SSID to the BSS Info table */
ieee1905_client_bssinfo_type *i5DmAddSSIDToBSSTable(ieee1905_ssid_type *ssid,
  unsigned char mapFlags);
/* Find the SSID in BSS info table */
ieee1905_client_bssinfo_type *i5DmFindSSIDInBSSTable(ieee1905_ssid_type *ssid);
/* Check if the STA is backhaul STA */
int i5DmIsSTABackhaulSTA(i5_dm_bss_type *bss, i5_dm_clients_type *sta);
/* Update the MAP Flags in all the BSS of a interface from Controllers BSS Table */
void i5DmUpdateMAPFlagsFromControllerBSSTable(i5_dm_device_type *pdevice,
  i5_dm_interface_type *pdmif);
/* Get the printable data model buffer for etxtended data model CLI command */
char *i5DmGetExtendedDataModelBuffer(unsigned int *MsgBuf_len);
#endif /* MULTIAP */

#endif /* _IEEE1905_DATAMODEL_PRIV_H_ */
