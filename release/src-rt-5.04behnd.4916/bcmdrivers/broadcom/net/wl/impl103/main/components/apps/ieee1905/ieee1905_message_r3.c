/*
 * Broadcom IEEE1905 MultiAP-R3 Messages Implementation
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
 * $Id: ieee1905_message_r3.c 836368 2024-02-12 07:46:39Z $
 */

#if defined(MULTIAP)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <linux/if_ether.h>
#include <wlioctl.h>
#include "ieee1905_timer.h"
#include "ieee1905_tlv.h"
#include "ieee1905_tlv_r3.h"
#include "ieee1905_message_r3.h"
#include "ieee1905_socket.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_trace.h"
#include "ieee1905_vendor.h"

#define I5_TRACE_MODULE i5TraceMessage

extern void i5VendorInformMessageSend(const unsigned char *dst_al_mac, const void *pmsg,
  const i5_message_types_t message_type, const void *reserved);
extern void i5VendorInformMessageRecieve(void *pmsg, int message_type,
  unsigned char pre_or_post_cb, const unsigned char *src_al_mac);

/* Send BSS configuration request message */
void i5MessageBSSConfigurationRequestSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address)
{
  i5_message_type *pmsg = NULL;
  i5_dm_device_type *pdevice = NULL;
  i5_dm_interface_type *pdmif = NULL;

  if (i5_config.ptmrWSCOrConfigReq) {
    i5TimerFree(i5_config.ptmrWSCOrConfigReq);
    i5_config.ptmrWSCOrConfigReq = NULL;
  }

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  if ((pdevice = i5DmGetSelfDevice()) == NULL) {
    i5TraceError("Local device could not found\n");
    return;
  }

  if ((pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO)) == NULL) {
    i5TraceError("Failed to create bss configuration request message\n");
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending BSS Configuration Request Message to Controller device " I5_MAC_DELIM_FMT ""
    " %04x on %s. \n", I5_MAC_PRM(neighbor_al_mac_address), i5_config.last_message_identifier,
    psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageBSSConfigurationRequestValue, i5_config.last_message_identifier);
  i5TlvMultiAPProfileInsert(pmsg, NULL);
  i5TlvSupportedServiceTypeInsert(pmsg);
  i5TlvAKMSuiteCapabilitiesTypeInsert(pmsg, &pdevice->dev_akm_suites);
  for (pdmif = pdevice->interface_list.ll.next; pdmif; pdmif = pdmif->ll.next) {
    if (!I5_IS_IFR_WIRELESS(pdmif->flags)) {
      continue;
    }
    i5TlvAPRadioBasicCapabilitiesTypeInsert(pmsg, pdmif->InterfaceId,
      pdmif->ApCaps.RadioCaps.maxBSSSupported, pdmif->ApCaps.RadioCaps.List,
      pdmif->ApCaps.RadioCaps.Len);
    if (I5_IS_BSS_STA(pdmif->mapFlags)) {
      i5TlvBackhaulSTARadioCapabilitiesTypeInsert(pmsg, pdmif->InterfaceId,
        I5_TLV_BSTA_RADIO_CAPS_MAC_INCLUDED, pdmif->InterfaceId);
    }
    i5TlvAPRadioAdvancedCapabilitiesTypeInsert(pmsg, pdmif);
    i5TlvAPHTCapabilitiesTypeInsert(pmsg, pdmif->InterfaceId, pdmif->ApCaps.HTCaps);
    if (pdmif->ApCaps.VHTCaps.Valid) {
      i5TlvAPVHTCapabilitiesTypeInsert(pmsg, pdmif->InterfaceId, pdmif->ApCaps.VHTCaps.TxMCSMap,
        pdmif->ApCaps.VHTCaps.RxMCSMap, pdmif->ApCaps.VHTCaps.CapsEx, pdmif->ApCaps.VHTCaps.Caps);
    }
    if (pdmif->ApCaps.HECaps.Valid) {
      i5TlvAPHECapabilitiesTypeInsert(pmsg, pdmif->InterfaceId, &pdmif->ApCaps.HECaps);
      i5TlvAPWiFi6CapabilitiesTypeInsert(pmsg, pdmif->InterfaceId, &pdmif->ApCaps.WiFi6Caps);
    }
  }
  i5TlvAPCapabilitiesTypeInsert(pmsg, pdevice->BasicCaps);
  i5TlvProfile2APCapabilityInsert(pmsg, &pdevice->p2ApCap);

  /* Get JSON encoded dpp config request object and insert it as part of bss config request tlv */
  if (i5_config.dpp_config_req_obj) {
    free(i5_config.dpp_config_req_obj);
    i5_config.dpp_config_req_obj = NULL;
    i5_config.dpp_config_req_obj_len = 0;
  }
  if (i5_config.cbs.get_dpp_config_req_obj) {
    i5_config.cbs.get_dpp_config_req_obj(&i5_config.dpp_config_req_obj,
      &i5_config.dpp_config_req_obj_len);
  }
  if (!i5_config.dpp_config_req_obj || !i5_config.dpp_config_req_obj_len) {
    i5TraceError("Failed to get dpp config request object so skip sending bss configuration req msg\n");
    goto end;
  }

  i5TlvBSSConfigurationRequestTypeInsert(pmsg, i5_config.dpp_config_req_obj,
    i5_config.dpp_config_req_obj_len);
  i5TlvWiFi7AgentCapabilitiesTypeInsert(pmsg, pdevice);

  i5VendorInformMessageSend(neighbor_al_mac_address, pmsg, i5MessageBSSConfigurationRequestValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);
  /* It can process reqconfiguration trigger message now onwards */
  i5_config.flags |= I5_CONFIG_FLAG_ACCEPT_RECONFIG_TRIGGER;
  i5MessageSend(pmsg, 0);

end:
  i5MessageFree(pmsg);
  i5_config.ptmrWSCOrConfigReq = i5TimerNew(I5_MESSAGE_MIN_M1_M2_WAITING_MSEC,
    i5WlCfgMultiApWSCOrBSSConfigReqTimeout, NULL);
}

/* Receive BSS config request message */
void i5MessageBSSConfigurationRequestReceive(i5_message_type *pmsg)
{
  i5_dm_device_type *pdevice = pmsg->pDevice;
  unsigned char *src_mac_addr;
  int rc = 0;
  i5_bss_config_req_attrs_t config_req_attrs;

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);

  /* Create the device pointer if not exists as we get lot of information here */
  if (pdevice == NULL) {
    pdevice = i5DmDeviceNew(src_mac_addr, i5MessageVersionGet(pmsg), NULL);
    if (pdevice == NULL) {
      i5TraceError("Skip processing BSS Configuration Request Message from "
        "device "I5_MAC_DELIM_FMT" as device is not discoverd at the controller.\n",
        I5_MAC_PRM(src_mac_addr));
      goto end;
    }

    pdevice->psock = pmsg->psock;
  }

  pmsg->pDevice = pdevice;

  /* In controller, if the PTK is not present when it recieves encrypted payload TLV, send the
   * reconfiguration trigger message. At the agent side, when it receives, unencrypted
   * reconfiguraiton trigger message, it will start with NIP
   */
  if (I5_IS_MSG_ENCR_PAYLOAD_PRESENT(pmsg->flags) && !I5_DEV_IS_PTK_PRESENT(pdevice)) {
    i5TraceInfo("Encrypted Payload TLV present but the PTK is not present. Send "
      "Reconfiguration Trigger message so that the agent will restart the NIP\n");
    if (pdevice->reconfigTimer) {
      i5TraceInfo("Reconfiguration Trigger Already In Progress\n");
      goto end;
    }
    i5MessageReconfigurationTriggerSend(pdevice);
    goto end;
  }

  if (pdevice->reconfigTimer) {
    i5TimerFree(pdevice->reconfigTimer);
    pdevice->reconfigTimer = NULL;
  }

  i5VendorInformMessageRecieve(pmsg, i5MessageBSSConfigurationRequestValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  i5Trace("Processing BSS Config Request Message ptk[%p] ptk_len[%d] received on %s from device "I5_MAC_DELIM_FMT"\n",
    pdevice->ptk, pdevice->ptk_len, pmsg->psock->u.sll.ifname, I5_MAC_PRM(src_mac_addr));

  rc |= i5TlvMultiAPProfileExtract(pmsg, pdevice);
  rc |= i5TlvSupportedServiceTypeExtract(pmsg, &pdevice->flags);
  rc |= i5TlvAKMSuiteCapabilitiesTypeExtract(pmsg, &pdevice->dev_akm_suites);
  rc |= i5TlvAPRadioBasicCapabilitiesTypeExtract(pmsg, pdevice, 1);
  rc |= i5TlvBackhaulSTARadioCapabilitiesTypeExtract(pmsg);
  rc |= i5TlvAPRadioAdvancedCapabilitiesTypeExtract(pmsg, pdevice);
  rc |= i5TlvProfile2APCapabilityExtract(pmsg, pdevice);
  rc |= i5TlvBSSConfigurationRequestTypeExtract(pmsg, &i5_config.dpp_config_req_obj,
    &i5_config.dpp_config_req_obj_len);
  if (rc != 0) {
    i5TraceError("rc[%d] : Error Processing BSS Config Request Message from device "I5_MAC_DELIM_FMT"\n",
      rc, I5_MAC_PRM(src_mac_addr));
    goto end;
  }

  /* Reset agent teardown device flag to get onboarded if max repeater count not reached */
  pdevice->flags &= ~I5_CONFIG_FLAG_TEARDOWN_AGENT;

  memset(&config_req_attrs, 0, sizeof(config_req_attrs));
  if (i5_config.cbs.parse_dpp_config_req_obj && i5_config.dpp_config_req_obj) {
    i5_config.cbs.parse_dpp_config_req_obj(i5_config.dpp_config_req_obj, &config_req_attrs);
  }

  i5VendorInformMessageRecieve(pmsg, i5MessageBSSConfigurationRequestValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  /* Check agent count to set agent teardown device flag on this agent */
  i5WlcfgCheckForTearDown(pdevice);

  if (i5WlCfgGenerateDPPConfigResponseObjects(pdevice, &config_req_attrs) == 0) {
    i5MessageBSSConfigurationResponseSend(pdevice);
  }

end:
  i5MessageFree(pmsg);
}

/* Send DPP CCE Indication message to a Multi AP Device */
void i5MessageDPPCCEIndicationSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, uint8 enable)
{
  i5_message_type *pmsg = NULL;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending DPP CCE Indication Message to " I5_MAC_DELIM_FMT " %04x on %s. "
    "DPP CCE Indication[%s]\n",
    I5_MAC_PRM(neighbor_al_mac_address),
    i5_config.last_message_identifier, psock->u.sll.ifname, enable ? "Enable" : "Disable");

  i5PacketHeaderInit(pmsg->ppkt, i5MessageDPPCCEIndicationValue,
    i5_config.last_message_identifier);

  i5TlvDPPCCEIndicationInsert(pmsg, enable);

  i5VendorInformMessageSend(neighbor_al_mac_address, pmsg,
    i5MessageDPPCCEIndicationValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive DPP CCE Indication Message */
void i5MessageDPPCCEIndicationReceive(i5_message_type *pmsg)
{
  unsigned char *src_mac_addr;
  uint8 enable = 0;

  /* Dont respond if you are not agent */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    goto end;
  }

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);
  i5VendorInformMessageRecieve(pmsg, i5MessageDPPCCEIndicationValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  i5TlvDPPCCEIndicationExtract(pmsg, &enable);

  i5VendorInformMessageRecieve(pmsg, i5MessageDPPCCEIndicationValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  i5Trace("Received DPP CCE Indication Message %04x on %s. DPP CCE Indication[%s] \n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname, enable ? "Enable" : "Disable");

  if (i5_config.cbs.dpp_cce_indication) {
    i5_config.cbs.dpp_cce_indication(enable);
  }

end:
  if (pmsg != NULL) {
    i5MessageFree(pmsg);
  }
}

/* Send BSS Configuration Response Message */
void i5MessageBSSConfigurationResponseSend(i5_dm_device_type *pdevice)
{
  i5_socket_type *psock = pdevice->psock;
  i5_message_type *pmsg = NULL;
  ieee1905_dpp_config_resp_type_t *config_resp_obj;
  int dpp_config_resp_count = 0;
  unsigned int all_resp_len = 0, idx = 0;
  char *all_resp_str = NULL;
  i5_dpp_attribute_t dpp_attr_hdr = {0};

  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5Trace("Local device is not controller device\n");
    return;
  }

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, pdevice->DeviceId, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending BSS Configuration Response Message to " I5_MAC_DELIM_FMT " %04x on %s. \n",
    I5_MAC_PRM(pdevice->DeviceId), i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageBSSConfigurationResponseValue,
    i5_config.last_message_identifier);

  /* First find the length of all the config response */
  foreach_iglist_item(config_resp_obj, ieee1905_dpp_config_resp_type_t,
    i5_config.dpp_config_resp_str_list) {
    all_resp_len += config_resp_obj->config_resp_len;
    dpp_config_resp_count++;
  }

  if (!dpp_config_resp_count) {
    i5TraceError("Skip sending bss configuration response message to device "I5_MAC_DELIM_FMT" ."
      "As there are no dpp config objects for it.\n", I5_MAC_PRM(pdevice->DeviceId));
    goto end;
  }

  /* For holding response with DPP attributes */
  all_resp_len += (dpp_config_resp_count * sizeof(i5_dpp_attribute_t));

  /* Allocate memory and copy all the response into one buffer with the DPP attribute header */
  all_resp_str = (char*)malloc(all_resp_len + 1);
  if (all_resp_str == NULL) {
    i5TraceDirPrint("malloc failure\n");
    goto end;
  }

  foreach_iglist_item(config_resp_obj, ieee1905_dpp_config_resp_type_t,
    i5_config.dpp_config_resp_str_list) {
    /* Add DPP attribute header 2 bytes of ID and 2 bytes of length */
    dpp_attr_hdr.id = I5_DPP_ATTR_CONFIG_OBJ;
    dpp_attr_hdr.length = config_resp_obj->config_resp_len;
    memcpy(all_resp_str+idx, &dpp_attr_hdr, sizeof(i5_dpp_attribute_t));
    idx += sizeof(i5_dpp_attribute_t);
    memcpy(all_resp_str+idx, config_resp_obj->config_resp_str, config_resp_obj->config_resp_len);
    idx += config_resp_obj->config_resp_len;
    i5Trace("Idx: %u len %u response %s\n", idx, config_resp_obj->config_resp_len,
      config_resp_obj->config_resp_str);
  }

  (void)i5TlvBSSConfigurationResponseTypeInsert(pmsg, all_resp_str, idx);

  i5TlvDefault8021QSettingsTypeInsert(pmsg, &i5_config.policyConfig);
  i5TlvTrafficSeparationPolicyTypeInsert(pmsg, &i5_config.policyConfig);

  i5MessageInsertAPAndbSTAMLDConfigurationTLVs(pmsg, pdevice);

  i5VendorInformMessageSend(pdevice->DeviceId, pmsg,
    i5MessageBSSConfigurationResponseValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);

end:
  if (all_resp_str) {
    free(all_resp_str);
  }
  i5MessageFree(pmsg);
}

/* Receive BSS Configuration Response Message */
void i5MessageBSSConfigurationResponseReceive(i5_message_type *pmsg)
{
  unsigned char *src_mac_addr;
  int rc = 0;
  i5_dm_device_type *pdevice = NULL;
  unsigned char ts_policy_flag = 0;

  i5Trace("Received BSS Configuration Response Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);

  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5Trace("Local device is not agent device\n");
    goto end;
  }

  if ((pdevice = i5DmGetSelfDevice()) == NULL) {
    i5Trace("Local device could not found\n");
    goto end;
  }

  i5VendorInformMessageRecieve(pmsg, i5MessageBSSConfigurationResponseValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  rc = i5TlvBSSConfigurationResponseTypeExtract(pmsg, pdevice);
  if (rc != 0) {
    i5Trace("Failed to extract BSS configuration response tlv\n");
    goto end;
  }

  ts_policy_flag = i5WlcfgGetTrafficSeparationPolicyFlag(pmsg);

  i5MessageExtractAPAndbSTAMLDConfigurationTLVs(pmsg, &i5_config.policyConfig.agent_ap_mld_conf,
    &i5_config.policyConfig.bsta_mld_conf);

  i5VendorInformMessageRecieve(pmsg, i5MessageBSSConfigurationResponseValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  if (rc == 0) {
    (void)i5WlCfgConfigureDevice(pdevice, ts_policy_flag);
  }
  i5MessageCleanupAPAndbSTAMLDConfiguration(&i5_config.policyConfig.agent_ap_mld_conf,
    &i5_config.policyConfig.bsta_mld_conf);

end:
  i5MessageFree(pmsg);
}

/* Send BSS Configuration Result Message */
void i5MessageBSSConfigurationResultSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address)
{
  i5_message_type *pmsg = NULL;
  i5_dm_device_type *pdevice = NULL;

  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5Trace("Local device is not agent device\n");
    return;
  }

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  if ((pdevice = i5DmGetSelfDevice()) == NULL) {
    i5Trace("Local device could not found\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending BSS Configuration Result Message to " I5_MAC_DELIM_FMT " %04x on %s. \n",
    I5_MAC_PRM(neighbor_al_mac_address), i5_config.last_message_identifier, psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageBSSConfigurationResultValue,
    i5_config.last_message_identifier);

  if (i5TlvBSSConfigurationReportTypeInsert(pmsg, pdevice) != 0) {
    goto end;
  }

  (void)i5TlvAgentAPMLDConfigurationInsert(pmsg, &i5_dm_network_topology.agent_ap_mld_conf);
  if (i5_dm_network_topology.bsta_mld_conf) {
    (void)i5TlvBackhaulSTAMLDConfigurationInsert(pmsg, i5_dm_network_topology.bsta_mld_conf);
  }

  i5VendorInformMessageSend(neighbor_al_mac_address, pmsg,
    i5MessageBSSConfigurationResultValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);

end:
  i5MessageFree(pmsg);
}

/* Receive BSS Configuration Result Message */
void i5MessageBSSConfigurationResultReceive(i5_message_type *pmsg)
{
  unsigned char *src_mac_addr;
  i5_dm_device_type *pdevice = pmsg->pDevice;

  i5Trace("Received BSS Configuration Result Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);

  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5Trace("Local device is not controller device\n");
    goto end;
  }

  if (pdevice == NULL) {
    i5Trace("Could not find device ["I5_MAC_DELIM_FMT"] in the controller\n",
      I5_MAC_PRM(src_mac_addr));
    goto end;
  }

  i5VendorInformMessageRecieve(pmsg, i5MessageBSSConfigurationResultValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  if (i5TlvBSSConfigurationReportTypeExtract(pmsg, pdevice, TRUE) != 0) {
    goto end;
  }

  (void)i5MessageExtractAPAndbSTAMLDConfigurationTLVs(pmsg,
    &i5_config.policyConfig.agent_ap_mld_conf, &i5_config.policyConfig.bsta_mld_conf);
  (void)i5DmProcessAPAndbSTAMLDConfigurationFromAgent(pdevice,
    &i5_config.policyConfig.agent_ap_mld_conf, i5_config.policyConfig.bsta_mld_conf);
  (void)i5MessageCleanupAPAndbSTAMLDConfiguration(&i5_config.policyConfig.agent_ap_mld_conf,
    &i5_config.policyConfig.bsta_mld_conf);

  i5VendorInformMessageRecieve(pmsg, i5MessageBSSConfigurationResultValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  /* Send Agent List message for All Agent devices */
  i5WlCfgSendToAllAgentListMessage();

end:
  i5MessageFree(pmsg);
}

/* Send Chirp Notification message */
void i5MessageChirpNotificationSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_dpp_chirp_value_t *dpp_chirp)
{
  i5_message_type *pmsg = NULL;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending Chirp Notification Message to " I5_MAC_DELIM_FMT " %04x on %s. "
    "DPP Chirp Value Fields: Flags[0x%x], EnrolleMAC["I5_MAC_DELIM_FMT"], Hash Length[%d]\n",
    I5_MAC_PRM(neighbor_al_mac_address),
    i5_config.last_message_identifier, psock->u.sll.ifname,
    dpp_chirp->flags, I5_MAC_PRM(dpp_chirp->enrollee_mac), dpp_chirp->hash_len);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageChirpNotificationValue,
    i5_config.last_message_identifier);

  i5TlvDPPChirpValueInsert(pmsg, dpp_chirp);

  i5VendorInformMessageSend(neighbor_al_mac_address, pmsg,
    i5MessageChirpNotificationValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive Chirp Notification Message */
void i5MessageChirpNotificationReceive(i5_message_type *pmsg)
{
  unsigned char *src_mac_addr;

  i5Trace("Received Chirp Notification Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);
  i5VendorInformMessageRecieve(pmsg, i5MessageChirpNotificationValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  /* In this message there can be more than one DPP Chirp TLV, so get it and call the callback
   * inside TLV extract for each DPP chirp
   */
  i5TlvDPPChirpValueExtractAllTlvs(pmsg, src_mac_addr);

  i5VendorInformMessageRecieve(pmsg, i5MessageChirpNotificationValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  i5MessageFree(pmsg);
}

/* Send 1905 Encap EAPOL message */
void i5Message1905EncapEAPOLSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_1905_encap_eapol_t *encap_1905_eapol)
{
  i5_message_type *pmsg = NULL;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending 1905 Encap EAPOL Message to " I5_MAC_DELIM_FMT " %04x on %s. "
    "EAPOL frame payload Length[%u]\n",
    I5_MAC_PRM(neighbor_al_mac_address), i5_config.last_message_identifier, psock->u.sll.ifname,
    encap_1905_eapol->frame_length);

  i5PacketHeaderInit(pmsg->ppkt, i5Message1905EncapEAPOLValue,
    i5_config.last_message_identifier);

  i5Tlv1905EncapEAPOLValueInsert(pmsg, encap_1905_eapol);

  i5VendorInformMessageSend(neighbor_al_mac_address, pmsg,
    i5Message1905EncapEAPOLValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive 1905 Encap EAPOL Message */
void i5Message1905EncapEAPOLReceive(i5_message_type *pmsg)
{
  int rc = 0;
  unsigned char *src_mac_addr;
  i5_1905_encap_eapol_t encap_1905_eapol;

  i5Trace("Received 1905 Encap EAPOL Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);
  i5VendorInformMessageRecieve(pmsg, i5Message1905EncapEAPOLValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  memset(&encap_1905_eapol, 0, sizeof(encap_1905_eapol));
  rc |= i5Tlv1905EncapEAPOLValueExtract(pmsg, &encap_1905_eapol);
  if (rc != 0) {
    goto end;
  }

  i5VendorInformMessageRecieve(pmsg, i5Message1905EncapEAPOLValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  if (i5_config.cbs.encap_1905_eapol) {
    i5_config.cbs.encap_1905_eapol(&encap_1905_eapol, src_mac_addr);
  }

end:
  i5MessageFree(pmsg);
}

/* Send DPP Bootstrapping URI Notification Message */
void i5MessageDPPBootstrappingURINotificationSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_dpp_bootstrap_uri_notification_t *uri_notification)
{
  i5_message_type *pmsg = NULL;

  if (!psock) {
    i5Trace("psock is NULL\n");
    return;
  }

  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5Trace("Local device is not agent device. Skip sending dpp uri notification msg\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
    i5Trace("Failed to create dpp uri notification msg\n");
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending DPP Bootstrapping URI Notification Message to "I5_MAC_DELIM_FMT" "
    "%04x on %s.\n", I5_MAC_PRM(neighbor_al_mac_address), i5_config.last_message_identifier,
    psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageDPPBootstrappingURINotificationValue,
    i5_config.last_message_identifier);

  i5TlvDPPBootstrappingURINotificationTypeInsert(pmsg, uri_notification);

  i5VendorInformMessageSend(neighbor_al_mac_address, pmsg,
    i5MessageDPPBootstrappingURINotificationValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive DPP Bootstrapping URI Notification Message */
void i5MessageDPPBootstrappingURINotificationReceive(i5_message_type *pmsg)
{
  int rc = 0;
  i5_dpp_bootstrap_uri_notification_t uri_notification;
  i5_dm_device_type *pdevice = pmsg->pDevice;

  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5Trace("Local device is not controller device. Skip processing dpp uri notification msg\n");
    goto end;
  }

  if (pdevice == NULL) {
    i5Trace("Could not find device ["I5_MAC_DELIM_FMT"] in the controller."
      "Skip processing dpp uri notification msg\n", I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg)));
    goto end;
  }

  i5Trace("Received DPP Bootstrapping URI Notification Message %04x from "I5_MAC_DELIM_FMT" on "
    "%s\n", i5MessageIdentifierGet(pmsg), I5_MAC_PRM(pdevice->DeviceId), pmsg->psock->u.sll.ifname);

  i5VendorInformMessageRecieve(pmsg, i5MessageDPPBootstrappingURINotificationValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, pdevice->DeviceId);

  memset(&uri_notification, 0, sizeof(uri_notification));
  rc |= i5TlvDPPBootstrappingURINotificationTypeExtract(pmsg, &uri_notification);
  if (rc != 0) {
    i5Trace("Failed to extract DPP Bootstrapping URI tlv from msg %04x on %s\n",
      i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
    goto end;
  }

  i5Trace("Received DPP Uri from device "I5_MAC_DELIM_FMT". Uri notification data  "
    "RUID["I5_MAC_DELIM_FMT"] BSSID["I5_MAC_DELIM_FMT"] bSTA_MAC["I5_MAC_DELIM_FMT"] \n"
    "dpp_uri[%s] len[%d] \n", I5_MAC_PRM(pdevice->DeviceId), I5_MAC_PRM(uri_notification.RUID),
    I5_MAC_PRM(uri_notification.BSSID), I5_MAC_PRM(uri_notification.bSTA_MAC),
    uri_notification.dpp_uri, uri_notification.dpp_uri_len);

  i5VendorInformMessageRecieve(pmsg, i5MessageDPPBootstrappingURINotificationValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, pdevice->DeviceId);

  if (i5_config.cbs.process_dpp_bootstrap_uri_obj) {
    i5_config.cbs.process_dpp_bootstrap_uri_obj(pdevice, &uri_notification);
  }

  free(uri_notification.dpp_uri);

end:
  i5MessageFree(pmsg);
}

/* Send Proxied Encap DPP message */
void i5MessageProxiedEncapDPPSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_1905_encap_dpp_t *dpp_1905_encap,
  i5_dpp_chirp_value_t *dpp_chirp)
{
  i5_message_type *pmsg = NULL;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending Proxied Encap DPP Message to " I5_MAC_DELIM_FMT " %04x on %s. 1905 Encap DPP "
    "Fields: Flags[0x%x], EnrolleMAC["I5_MAC_DELIM_FMT"], Frame Type[%d], Frame Length[%d]\n",
    I5_MAC_PRM(neighbor_al_mac_address), i5_config.last_message_identifier,
    psock->u.sll.ifname, dpp_1905_encap->flags, I5_MAC_PRM(dpp_1905_encap->enrollee_mac),
    dpp_1905_encap->frame_type, dpp_1905_encap->frame_length);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageProxiedEncapDPPValue,
    i5_config.last_message_identifier);

  /* If the message has Authentication Request, we should send reliable multicast message. For
   * multicast message, we should add the AL MAC address type TLV as per IEEE-1905.1-2013 spec
   */
  if (dpp_1905_encap->frame_type == I5_DPP_PUB_AF_AUTHENTICATION_REQ) {
    i5TlvAlMacAddressTypeInsert(pmsg);
  }

  if (dpp_chirp) {
    i5TraceInfo("DPP Chirp Value Fields: Flags[0x%x], EnrolleMAC["I5_MAC_DELIM_FMT"], "
      "Hash Length[%d]\n",
      dpp_chirp->flags, I5_MAC_PRM(dpp_chirp->enrollee_mac), dpp_chirp->hash_len);
  }

  i5Tlv1905EncapDPPInsert(pmsg, dpp_1905_encap);
  if (dpp_chirp) {
    i5TlvDPPChirpValueInsert(pmsg, dpp_chirp);
  }

  i5VendorInformMessageSend(neighbor_al_mac_address, pmsg,
    i5MessageProxiedEncapDPPValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);

  /* if the message has authentication request, then the Multi-AP Controller should send to all the
   * Multi-AP Agents, using the CMDU reliable multicast transmission procedures.
   */
  if (dpp_1905_encap->frame_type == I5_DPP_PUB_AF_AUTHENTICATION_REQ) {
    i5_message_header_type *phdr;

    phdr = (i5_message_header_type *)&pmsg->ppkt->pbuf[sizeof(struct ethhdr)];
    phdr->relay_indicator = 1;

    i5MessageRelayMulticastSend(pmsg, NULL /* all sockets */, NULL);
    i5MesssageReliableMultiCastSend(pmsg);
  } else {
    i5MessageSend(pmsg, 0);
    i5MessageFree(pmsg);
  }
}

/* Receive Proxied Encap DPP Message */
void i5MessageProxiedEncapDPPReceive(i5_message_type *pmsg)
{
  unsigned char *src_mac_addr;
  i5_1905_encap_dpp_t dpp_1905_encap;
  i5_dpp_chirp_value_t dpp_chirp;

  i5Message1905AckSend(pmsg);

  memset(&dpp_1905_encap, 0, sizeof(dpp_1905_encap));
  memset(&dpp_chirp, 0, sizeof(dpp_chirp));

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);
  i5VendorInformMessageRecieve(pmsg, i5MessageProxiedEncapDPPValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  i5Tlv1905EncapDPPExtract(pmsg, &dpp_1905_encap);

  /* In this message there will be only one DPP chirp TLV so get only one TLV */
  i5TlvDPPChirpValueExtractOneTlv(pmsg, &dpp_chirp);

  i5VendorInformMessageRecieve(pmsg, i5MessageProxiedEncapDPPValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  i5Trace("Received Proxied Encap DPP Message %04x on %s. 1905 Encap DPP Fields: Flags[0x%x], "
    "EnrolleMAC["I5_MAC_DELIM_FMT"], Frame Type[%d], Frame Length[%d]\n"
    "DPP Chirp Value Fields: Flags[0x%x], EnrolleMAC["I5_MAC_DELIM_FMT"], Hash Length[%d]\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname,
    dpp_1905_encap.flags, I5_MAC_PRM(dpp_1905_encap.enrollee_mac), dpp_1905_encap.frame_type,
    dpp_1905_encap.frame_length,
    dpp_chirp.flags, I5_MAC_PRM(dpp_chirp.enrollee_mac), dpp_chirp.hash_len);

  if (i5_config.cbs.dpp_proxied_encap) {
    i5_config.cbs.dpp_proxied_encap(&dpp_1905_encap, &dpp_chirp, src_mac_addr);
  }

  if (dpp_1905_encap.frame) {
    free(dpp_1905_encap.frame);
  }
  if (dpp_chirp.hash) {
    free(dpp_chirp.hash);
  }

  i5MessageFree(pmsg);
}

/* Send Direct Encap DPP message */
void i5MessageDirectEncapDPPSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_direct_encap_dpp_t *dpp_direct_encap)
{
  i5_message_type *pmsg = NULL;

  if (!psock) {
    i5TraceError("psock is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(psock, neighbor_al_mac_address, I5_PROTO);
  if (pmsg == NULL) {
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending Direct Encap DPP Message to " I5_MAC_DELIM_FMT " %04x on %s. "
    "Direct Encap DPP Fields: Frame Type[%d], Frame Length[%d]\n",
    I5_MAC_PRM(neighbor_al_mac_address), i5_config.last_message_identifier,
    psock->u.sll.ifname, dpp_direct_encap->frame_type, dpp_direct_encap->frame_length);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageDirectEncapDPPValue,
    i5_config.last_message_identifier);

  i5TlvDPPMessageInsert(pmsg, dpp_direct_encap);

  i5VendorInformMessageSend(neighbor_al_mac_address, pmsg,
    i5MessageDirectEncapDPPValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);

  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive Direct Encap DPP Message */
void i5MessageDirectEncapDPPReceive(i5_message_type *pmsg)
{
  int rc = 0;
  unsigned char *src_mac_addr;
  i5_direct_encap_dpp_t dpp_direct_encap;

  i5Message1905AckSend(pmsg);

  memset(&dpp_direct_encap, 0, sizeof(dpp_direct_encap));

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);
  i5VendorInformMessageRecieve(pmsg, i5MessageDirectEncapDPPValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  rc |= i5TlvDPPMessageExtract(pmsg, &dpp_direct_encap);
  if (rc != 0) {
    return;
  }

  i5VendorInformMessageRecieve(pmsg, i5MessageDirectEncapDPPValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  i5Trace("Received Direct Encap DPP Message from " I5_MAC_DELIM_FMT " %04x on %s. "
    "Direct Encap DPP Fields: Frame Type[%d], Frame Length[%d]\n",
    I5_MAC_PRM(src_mac_addr), i5MessageIdentifierGet(pmsg),
    pmsg->psock->u.sll.ifname, dpp_direct_encap.frame_type, dpp_direct_encap.frame_length);

  if (i5_config.cbs.dpp_direct_encap) {
    i5_config.cbs.dpp_direct_encap(&dpp_direct_encap, src_mac_addr);
  }

  if (dpp_direct_encap.frame) {
    free(dpp_direct_encap.frame);
  }

  i5MessageFree(pmsg);
}

/* Send Agent List message */
void i5MessageAgentListSend(i5_dm_device_type *pdevice)
{
  i5_message_type *pmsg = NULL;

  if (pdevice == NULL) {
    i5TraceError("pdevice is NULL\n");
    return;
  }

  pmsg = i5MessageCreate(pdevice->psock, pdevice->DeviceId, I5_PROTO);
  if (pmsg == NULL) {
    i5TraceError("pmsg is NULL\n");
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending Agent List Message to " I5_MAC_DELIM_FMT " %04x on %s. \n",
    I5_MAC_PRM(pdevice->DeviceId),
    i5_config.last_message_identifier, pmsg->psock->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageAgentListValue,
    i5_config.last_message_identifier);

  i5TlvAgentListTypeInsert(pmsg);

  i5VendorInformMessageSend(pdevice->DeviceId, pmsg,
    i5MessageAgentListValue, NULL);

  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);
  i5MessageFree(pmsg);
}

/* Receive Agent List Message */
void i5MessageAgentListReceive(i5_message_type *pmsg)
{
  unsigned char *src_mac_addr;

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);
  i5VendorInformMessageRecieve(pmsg, i5MessageAgentListValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS, src_mac_addr);

  i5TlvAgentListTypeExtract(pmsg);

  i5VendorInformMessageRecieve(pmsg, i5MessageAgentListValue,
    I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS, src_mac_addr);

  i5Trace("Received Agent List Message %04x on %s. \n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  i5MessageFree(pmsg);
}

/* Send Reconfiguration trigger message */
void i5MessageReconfigurationTriggerSend(i5_dm_device_type *pDevice)
{
  i5_message_type *pmsg = NULL;

  if (!pDevice || !pDevice->psock) {
    i5TraceError("%s is NULL\n", pDevice ? "psock" : "Device");
    return;
  }

  if (pDevice->reconfigTimer) {
    i5Trace("Reconfiguration Trigger is already sent and timer is created to send once again. "
      "So skip here\n");
    return;
  }

  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5Trace("Local device is not controller device\n");
    return;
  }

  if ((pmsg = i5MessageCreate(pDevice->psock, pDevice->DeviceId, I5_PROTO)) == NULL) {
    i5TraceError("Failed to create Reconfiguration Trigger Message\n");
    return;
  }

  i5_config.last_message_identifier++;

  i5Trace("Sending Reconfiguration Trigger Message to " I5_MAC_DELIM_FMT ""
    " %04x on %s. \n", I5_MAC_PRM(pDevice->DeviceId), i5_config.last_message_identifier,
    ((i5_socket_type*)pDevice->psock)->u.sll.ifname);

  i5PacketHeaderInit(pmsg->ppkt, i5MessageReconfigurationTriggerValue, i5_config.last_message_identifier);
  i5TlvEndOfMessageTypeInsert(pmsg);
  i5MessageSend(pmsg, 0);

  i5MessageFree(pmsg);
}

/* Receive Reconfiguration Trigger Message */
void i5MessageReconfigurationTriggerReceive(i5_message_type *pmsg)
{
  i5_dm_device_type *pdevice = pmsg->pDevice;
  unsigned char *src_mac_addr;

  i5Trace("Received Reconfiguration Trigger Message %04x on %s\n",
    i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);

  i5Message1905AckSend(pmsg);

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);

  if (i5_config.ptmrApSearch) {
    i5Trace("Skipping Reconfiguration trigger Message from device "I5_MAC_DELIM_FMT
      " as the agent is already started controller search\n", I5_MAC_PRM(src_mac_addr));
    goto end;
  }

  if (!pdevice || !I5_IS_MULTIAP_CONTROLLER(pdevice->flags)) {
    i5TraceError("Skipping Reconfiguration trigger Message from device "I5_MAC_DELIM_FMT
      " as the device is not controller.\n", I5_MAC_PRM(src_mac_addr));
    goto end;
  }

  if (pdevice->psock && (pdevice->psock != pmsg->psock)) {
    /* make all the interface as not configured */
    i5WlcfgMarkAllInterfacesUnconfigured();

    /* reconfiguration trigger received from different controller socket. Start with search */
    pdevice->psock = pmsg->psock;
    i5WlCfgMultiApControllerSearch(NULL);
  } else if (I5_IS_ACCEPT_RECONFIG_TRIGGER(i5_config.flags)) {
    /* Received a unencrypted message, inform wbd_slave to start the NIP so that the controller
     * and agent can derive a new PTK
     */
    if (!I5_IS_MSG_DECRYPTED(pmsg->flags)) {
      i5TraceInfo("Received unencrypted message. Inform WBD so that it can start the NIP\n");
      if (i5_config.cbs.notify_command && I5_DEV_IS_PTK_PRESENT(pdevice)) {
        i5_config.cbs.notify_command(I5_NOTIFY_CMD_START_NIP, NULL);
        goto end;
      } else {
        /* Do not process unencrypted reconfiguration trigger if the agent doesn't contain
         * PTK. agent might have already started getting it
         */
        goto end;
      }
    }

    /* Do not send BSS configuration request, if it is in progress */
    if (i5_config.ptmrWSCOrConfigReq) {
      i5Trace("Skipping Reconfiguration trigger Message from device "I5_MAC_DELIM_FMT
        " as the agent is already sent BSS configuration Request\n",
        I5_MAC_PRM(src_mac_addr));
      goto end;
    }

    /* make all the interface as not configured */
    i5WlcfgMarkAllInterfacesUnconfigured();

    /* Start the BSS configuration trigger to renew all the interfaces */
    i5MessageBSSConfigurationRequestSend(pdevice->psock, src_mac_addr);
  } else {
    i5TraceError("Skipping Reconfiguration trigger Message from device "I5_MAC_DELIM_FMT
      " as the agent is not ready yet to process it.\n", I5_MAC_PRM(src_mac_addr));
    goto end;
  }

end:
  i5MessageFree(pmsg);
}
#endif /* MULTIAP */
