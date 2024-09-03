/*
 * Broadcom IEEE1905 MultiAP-R3 TLV Include file
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: ieee1905_tlv_r3.h 838434 2024-04-01 09:34:42Z $
 */

#ifndef _IEEE1905_TLV_R3_H_
#define _IEEE1905_TLV_R3_H_

#if defined(MULTIAP)

#include "ieee1905_message.h"
#include "ieee1905.h"

#define i5TlvBSSConfigurationReport_Min_Length	1
#define i5TlvDPPCCEIndication_Length  1
#define i5TlvDPPChirpValue_Length     2
#define i5Tlv1905EncapDPP_Length      4
#define i5TlvDPPMessage_Length        8 /* Category [1] + Action ID [1] +
 * OUI [3] + OUIType [1] + CryptoSuite [1] + DPPFrameType [1]
 */
#define i5TlvAKMSuiteCaps_Length      2
#define i5TlvAgentList_Min_Length     1
#define i5TlvAgentList_Length         8
#define i5TlvAssociatedWiFi6STAStatusReport_Min_Length	7
#define i5Tlv1905LayerSecurityCapability_length 3

/* Length of the fixed fields of Encrypted Payload TLV.
 * Encryption Transmission counter [6]
 * Source al_mac [6]
 * Destination al_mac [6]
 * Length of the encrypted data [2]
*/
#define i5TlvEncryptedPayload_Lnegth	20

/* Length of the fields in MIC TLV.
 * 1 byte gtkid
 * 6 bytes integrity transmission counter
 * 6 bytes source al mac address
 * 2 bytes digest length
 * 32 bytes digest
*/
#define i5TlvMicLength			47

/* First 13 bytes of MIC TLV Needed to compute MIC.
 * 1 byte GTK key identifier
 * 6 bytes transmission counter
 * 6 bytes src mac address.
*/
#define i5TlvMicValue			13

/* Minimum len for DPP bootstrap Uri Notification tlv RUID [6] + BSSID[6] + bsta_mac[6] */
#define i5TlvDPPBootstrapURINotification_Min_Length	18

/* Minimum len for valid DPP bootstrap Uri "DPP:" */
#define i5TlvDPPBootstrapURI_Min_Length			4

/* Reserved Lengths in Wi-Fi 7 Agent Capabilities TLV */
#define i5TlvWiFi7AgentCapabilities_Res_Len 13u
#define i5TlvWiFi7AgentCapabilities_Radio_Res_Len 23u

/* Minimum Length for Wi-Fi 7 Agent Capabilities TLV */
#define i5TlvWiFi7AgentCapabilities_Min_Length (4u + i5TlvWiFi7AgentCapabilities_Res_Len)

/* Reserved Lengths in Agent AP MLD Configuration TLV */
#define i5TlvAgentAPMLDConfiguration_Res_Len 20u
#define i5TlvAgentAPMLDAffliatedAP_Res_Len 18u
/* Minimum length of Agent AP MLD Configuration TLV */
#define i5TlvAgentAPMLDConfiguration_Min_Length 1u
#define i5TlvAgentAPMLDAffliatedAP_Length (14u + i5TlvAgentAPMLDAffliatedAP_Res_Len)

/* Reserved Lengths in Backhaul STA MLD Configuration TLV */
#define i5TlvBackhaulSTAMLDConfiguration_Res_Len 17u
#define i5TlvBackhaulSTAMLDAffliatedBSTA_Res_Len 19u
/* Minimum length of Backhaul STA MLD Configuration TLV */
#define i5TlvBackhaulSTAMLDConfiguration_Min_Length (15u + i5TlvBackhaulSTAMLDConfiguration_Res_Len)
#define i5TlvBackhaulSTAMLDAffliatedBSTA_Length (13u + i5TlvBackhaulSTAMLDAffliatedBSTA_Res_Len)

/* Reserved Lengths in associated STA MLD Configuration TLV */
#define i5TlvAssociatedSTAMLDConfiguration_Res_Len 18u
#define i5TlvAssociatedSTAMLDAffliatedSTA_Res_Len 19u
/* Minimum length of associated STA MLD Configuration TLV */
#define i5TlvAssociatedSTAMLDConfiguration_Min_Length (14u + i5TlvAssociatedSTAMLDConfiguration_Res_Len)
#define i5TlvAssociatedSTAMLDAffliatedSTA_Length (12u + i5TlvAssociatedSTAMLDAffliatedSTA_Res_Len)

/* Reserved Length in Affiliated STA Metrics TLV */
#define i5TlvAffiliatedSTAMetrics_Res_Len 998u
/* Minimum length of Affiliated STA Metrics TLV */
#define i5TlvAffiliatedSTAMetrics_Min_Len 26u

/* Insert DPP Bootstrap URI Notification TLV */
int i5TlvDPPBootstrappingURINotificationTypeInsert(i5_message_type *pmsg,
  i5_dpp_bootstrap_uri_notification_t *uri_notification);
/* Extract DPP Bootstrap URI Notification TLV */
int i5TlvDPPBootstrappingURINotificationTypeExtract(i5_message_type *pmsg,
  i5_dpp_bootstrap_uri_notification_t *uri_notification);
/* Insert AKM Suites Capabilities TLV */
int i5TlvAKMSuiteCapabilitiesTypeInsert(i5_message_type *pmsg, i5_dm_akm_suite_caps_t *akm_suites);
/* Extract AKM Suites Capabilities TLV */
int i5TlvAKMSuiteCapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_akm_suite_caps_t *akm_suites);
/* Insert BSS config request TLV */
int i5TlvBSSConfigurationRequestTypeInsert(i5_message_type *pmsg, char *config_req_obj,
  unsigned int obj_len);
/* Extract BSS config request TLV */
int i5TlvBSSConfigurationRequestTypeExtract(i5_message_type *pmsg, char **config_req_obj,
  unsigned int *obj_len);
/* Insert BSS Configuration Report TLV */
int i5TlvBSSConfigurationReportTypeInsert(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* Extract BSS Configuration Report TLV */
int i5TlvBSSConfigurationReportTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice,
  bool set_ap_configured);
/* Insert BSS config response TLV */
int i5TlvBSSConfigurationResponseTypeInsert(i5_message_type *pmsg, char *config_resp_obj,
  unsigned int obj_len);
/* Extract BSS config response TLV */
int i5TlvBSSConfigurationResponseTypeExtract(i5_message_type *pmsg, i5_dm_device_type *device);
/* TLV to Insert DPP CCE Indication */
int i5TlvDPPCCEIndicationInsert(i5_message_type *pmsg, uint8 enable);
/* Extract DPP CCE Indication */
int i5TlvDPPCCEIndicationExtract(i5_message_type *pmsg, uint8 *enable);
/* TLV to Insert DPP Chirp Value */
int i5TlvDPPChirpValueInsert(i5_message_type *pmsg, i5_dpp_chirp_value_t *dpp_chirp);
/* Extract one DPP Chirp Value TLV */
int i5TlvDPPChirpValueExtractOneTlv(i5_message_type *pmsg, i5_dpp_chirp_value_t *dpp_chirp);
/* Extract All DPP Chirp Value TLVs in the message and call the callback for each TLV */
int i5TlvDPPChirpValueExtractAllTlvs(i5_message_type *pmsg, unsigned char *src_mac_addr);
/* TLV to Insert 1905 Encap EAPOL Value */
int i5Tlv1905EncapEAPOLValueInsert(i5_message_type *pmsg, i5_1905_encap_eapol_t *encap_1905_eapol);
/* Extract one 1905 Encap EAPOL Value TLV */
int i5Tlv1905EncapEAPOLValueExtract(i5_message_type *pmsg, i5_1905_encap_eapol_t *encap_1905_eapol);
/* TLV to Insert 1905 Encap DPP */
int i5Tlv1905EncapDPPInsert(i5_message_type *pmsg, i5_1905_encap_dpp_t *dpp_1905_encap);
/* Extract 1905 Encap DPP */
int i5Tlv1905EncapDPPExtract(i5_message_type *pmsg, i5_1905_encap_dpp_t *dpp_1905_encap);
/* Insert DPP Message TLV */
int i5TlvDPPMessageInsert(i5_message_type *pmsg, i5_direct_encap_dpp_t *dpp_direct_encap);
/* Extract DPP Message TLV */
int i5TlvDPPMessageExtract(i5_message_type *pmsg, i5_direct_encap_dpp_t *dpp_direct_encap);
/* TLV to report the AP WiFi6 Capabilities */
int i5TlvAPWiFi6CapabilitiesTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  ieee1905_wifi6_caps_type* WiFiCaps);
/* Extract AP WiFi6 Capabilities TLV */
int i5TlvAPWiFi6CapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* TLV to Insert Agent List info */
int i5TlvAgentListTypeInsert(i5_message_type *pmsg);
/* Extract Agent List info */
int i5TlvAgentListTypeExtract(i5_message_type *pmsg);
/* Check if TLV is Encrypted Payload TLV */
int i5TlvIsEncryptedPayloadType(int tlvType);
/* Insert Encrypted pyload TLV */
i5_message_type *i5TlvEncryptedPayloadTypeInsert(i5_message_type *pmsg, i5_dm_device_type *dstdev);
/* Extract Encrypted pyload TLV */
int i5TlvEncryptedPayloadTypeExtract(i5_message_type *pmsg);
/* Insert Associated WiFi6 STA Status Report TLV */
int i5TlvAssociatedWiFi6STAStatusReportTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  ieee1905_wifi6_sta_status *wifi6_sta_status);
/* Extract Associated WiFi6 STA Status Report TLV */
int i5TlvAssociatedWiFi6STAStatusReportTypeExtract(i5_message_type *pmsg);
/* TLV to report the 1905 Layer Security Capability */
int i5Tlv1905LayerSecurityCapabilityTypeInsert(i5_message_type *pmsg);
/* Extract 1905 Layer Security Capability TLV */
int i5Tlv1905LayerSecurityCapabilityTypeExtract(i5_message_type *pmsg);
/* TLV to Insert the MIC */
int i5TlvMICTypeInsert(i5_message_type *pmsg);
/* Extract MIC TLV */
int i5MessageCheckMIC(i5_message_type *pmsg);
/* Insert WiFi7 Agent Capabilities TLV */
int i5TlvWiFi7AgentCapabilitiesTypeInsert(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* Extract WiFi7 Agent Capabilities TLV */
int i5TlvWiFi7AgentCapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice);
/* TLV to Insert Agent AP MLD Configuration */
int i5TlvAgentAPMLDConfigurationInsert(i5_message_type *pmsg, ieee1905_glist_t *agent_ap_mld_conf);
/* Extract Agent AP MLD Configuration TLV */
int i5TlvAgentAPMLDConfigurationExtract(i5_message_type *pmsg, ieee1905_glist_t *agent_ap_mld_conf);
/* TLV to Insert Backhaul STA MLD Configuration */
int i5TlvBackhaulSTAMLDConfigurationInsert(i5_message_type *pmsg,
  i5_bsta_mld_conf_t *bsta_mld_conf);
/* Extract Backhaul STA MLD Configuration TLV */
int i5TlvBackhaulSTAMLDConfigurationExtract(i5_message_type *pmsg,
  i5_bsta_mld_conf_t *bsta_mld_conf);
/* TLV to Insert associated STA MLD Configuration */
int i5TlvAssociatedSTAMLDConfigurationInsert(i5_message_type *pmsg, i5_dm_clients_type *pdmclient);
/* Extract Associated STA MLD Configuration TLV */
int i5TlvAssociatedSTAMLDConfigurationExtract(unsigned char *pvalue, unsigned int length,
  i5_dm_clients_type *pdmclient);
/* TLV to add Affiliated STA Traffic Stats TLV */
int i5TlvAffiliatedSTAMetricsTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  ieee1905_affiliated_sta_metric *affiliated_metric, i5_dm_device_type *pDestDevice);
/* Extract Affiliated STA Traffic Stats TLV */
int i5TlvAffiliatedSTAMetricsTypeExtract(i5_message_type *pmsg);

#endif /* MULTIAP */
#endif /* _IEEE1905_TLV_R3_H_ */
