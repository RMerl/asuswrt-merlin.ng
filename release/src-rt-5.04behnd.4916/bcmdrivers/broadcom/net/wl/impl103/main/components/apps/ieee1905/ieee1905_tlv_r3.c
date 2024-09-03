/*
 * Broadcom IEEE1905 MultiAP-R3 TLV Implementation
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
 * $Id: ieee1905_tlv_r3.c 838434 2024-04-01 09:34:42Z $
 */

#if defined(MULTIAP)

#define __USE_XOPEN
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "ieee1905_message.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_tlv.h"
#include "ieee1905_tlv_r3.h"
#include "ieee1905_trace.h"
#include "ieee1905_aessiv.h"
#include <sha256.h>
#include <hmac_sha256.h>
#include <bcmendian.h>
#include <security_ipc.h>

#define I5_TRACE_MODULE i5TraceTlv

/* Insert DPP Bootstrap URI Notification TLV */
int i5TlvDPPBootstrappingURINotificationTypeInsert(i5_message_type *pmsg,
  i5_dpp_bootstrap_uri_notification_t *uri_notification)
{
  unsigned char *pbuf, *pmem;
  i5_tlv_t *ptlv;
  int rc = 0;

  i5Trace("\n");

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  memcpy(pbuf, uri_notification->RUID, MAC_ADDR_LEN);
  pbuf += MAC_ADDR_LEN;

  memcpy(pbuf, uri_notification->BSSID, MAC_ADDR_LEN);
  pbuf += MAC_ADDR_LEN;

  memcpy(pbuf, uri_notification->bSTA_MAC, MAC_ADDR_LEN);
  pbuf += MAC_ADDR_LEN;

  memcpy(pbuf, uri_notification->dpp_uri, uri_notification->dpp_uri_len);
  pbuf += uri_notification->dpp_uri_len;

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvDPPBootstrappingURINotificationType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);

  return rc;
}

/* Extract DPP Bootstrap URI Notification TLV */
int i5TlvDPPBootstrappingURINotificationTypeExtract(i5_message_type *pmsg,
  i5_dpp_bootstrap_uri_notification_t *uri_notification)
{
  int rc;
  unsigned char *pvalue;
  unsigned int length;

  rc = i5MessageTlvExtract(pmsg, i5TlvDPPBootstrappingURINotificationType, &length,
    &pvalue, i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }
  if (length < i5TlvDPPBootstrapURINotification_Min_Length) {
    i5TraceError("DPP Bootstrap Notification tlv's length %d is less than min expected len %d "
      "skip processing\n", length, i5TlvDPPBootstrapURINotification_Min_Length);
    goto end;
  }

  uri_notification->dpp_uri_len = length - i5TlvDPPBootstrapURINotification_Min_Length;
  if (uri_notification->dpp_uri_len < i5TlvDPPBootstrapURI_Min_Length) {
    i5TraceError("DPP Bootstrap URI length %d is less than min expected len %d "
      "skip processing\n", uri_notification->dpp_uri_len, i5TlvDPPBootstrapURI_Min_Length);
    goto end;
  }

  memcpy(uri_notification->RUID, pvalue, MAC_ADDR_LEN);
  pvalue += MAC_ADDR_LEN;

  memcpy(uri_notification->BSSID, pvalue, MAC_ADDR_LEN);
  pvalue += MAC_ADDR_LEN;

  memcpy(uri_notification->bSTA_MAC, pvalue, MAC_ADDR_LEN);
  pvalue += MAC_ADDR_LEN;

  uri_notification->dpp_uri = (char*)malloc(uri_notification->dpp_uri_len + 1 /* '\0' */);
  if (!uri_notification->dpp_uri) {
    i5TraceDirPrint("malloc error\n");
    goto end;
  }
  memset(uri_notification->dpp_uri, 0, uri_notification->dpp_uri_len + 1);
  memcpy(uri_notification->dpp_uri, pvalue, uri_notification->dpp_uri_len);

  return rc;

end:
  return -1;
}

/* Insert AKM Suites Capabilities TLV */
int i5TlvAKMSuiteCapabilitiesTypeInsert(i5_message_type *pmsg, i5_dm_akm_suite_caps_t *akm_suites)
{
  unsigned char *pbuf, *pmem;
  i5_tlv_t *ptlv;
  int rc = 0;
  i5_dm_bss_akm_suite_caps_t *bss_akm_suite;

  i5Trace("\n");

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  *pbuf = (uint8)akm_suites->bh_akm_suites.count;
  pbuf++;
  foreach_iglist_item(bss_akm_suite, i5_dm_bss_akm_suite_caps_t, akm_suites->bh_akm_suites) {
    memcpy(pbuf, bss_akm_suite->akm_suite.oui, WFA_OUI_LEN);
    pbuf += WFA_OUI_LEN;
    *pbuf = bss_akm_suite->akm_suite.akm_suite_type;
    pbuf++;
  }

  *pbuf = (uint8)akm_suites->fh_akm_suites.count;
  pbuf++;
  foreach_iglist_item(bss_akm_suite, i5_dm_bss_akm_suite_caps_t, akm_suites->fh_akm_suites) {
    memcpy(pbuf, bss_akm_suite->akm_suite.oui, WFA_OUI_LEN);
    pbuf += WFA_OUI_LEN;
    *pbuf = bss_akm_suite->akm_suite.akm_suite_type;
    pbuf++;
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvAKMSuiteCapabilitiesType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);

  return rc;
}

/* Extract AKM Suites Capabilities TLV */
int i5TlvAKMSuiteCapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_akm_suite_caps_t *akm_suites)
{
  int rc, count, idx;
  unsigned char *pvalue;
  unsigned int length, extracted_len = i5TlvAKMSuiteCaps_Length;
  i5_dm_bss_akm_suite_caps_t *bss_akm_suite;

  i5Trace("\n");

  i5DmAKMSuitesCleanup(akm_suites);
  ieee1905_glist_init(&akm_suites->bh_akm_suites);
  ieee1905_glist_init(&akm_suites->fh_akm_suites);

  rc = i5MessageTlvExtract(pmsg, i5TlvAKMSuiteCapabilitiesType, &length,
    &pvalue, i5MessageTlvExtractWithReset);
  if (rc != 0) {
    i5Trace("i5TlvAKMSuiteCapabilitiesType Doesn't exist\n");
    goto end;
  }
  if (length < extracted_len) {
    goto end;
  }

  count = *pvalue++;
  if (length < extracted_len + (count * I5_DM_BSS_AKM_SUITE_LEN)) {
    goto end;
  }

  for (idx = 0; idx < count; idx++) {
    bss_akm_suite = (i5_dm_bss_akm_suite_caps_t*)malloc(sizeof(*bss_akm_suite));
    if (!bss_akm_suite) {
      i5TraceDirPrint("malloc error \n");
      goto end;
    }
    memcpy(bss_akm_suite->akm_suite.oui, pvalue, WFA_OUI_LEN);
    pvalue += WFA_OUI_LEN;
    bss_akm_suite->akm_suite.akm_suite_type = *pvalue;
    pvalue++;
    extracted_len += I5_DM_BSS_AKM_SUITE_LEN;
    ieee1905_glist_append(&akm_suites->bh_akm_suites, (dll_t*)bss_akm_suite);
  }

  count = *pvalue++;
  if (length < extracted_len + (count * I5_DM_BSS_AKM_SUITE_LEN)) {
    goto end;
  }

  for (idx = 0; idx < count; idx++) {
    bss_akm_suite = (i5_dm_bss_akm_suite_caps_t*)malloc(sizeof(*bss_akm_suite));
    if (!bss_akm_suite) {
      i5TraceDirPrint("malloc error \n");
      goto end;
    }
    memcpy(bss_akm_suite->akm_suite.oui, pvalue, WFA_OUI_LEN);
    pvalue += WFA_OUI_LEN;
    bss_akm_suite->akm_suite.akm_suite_type = *pvalue;
    pvalue++;
    ieee1905_glist_append(&akm_suites->fh_akm_suites, (dll_t*)bss_akm_suite);
  }

  return 0;

end:
  return -1;
}

/* Insert BSS config request TLV */
int i5TlvBSSConfigurationRequestTypeInsert(i5_message_type *pmsg, char *config_req_obj,
  unsigned int obj_len)
{
  unsigned int len = sizeof(i5_tlv_t) + sizeof(i5_dpp_attribute_t) + obj_len;
  i5_tlv_t *ptlv;
  uint8 *buf;
  int rc;
  i5_dpp_attribute_t dpp_attr_hdr = {0};

  if ((buf = (uint8 *)malloc(len)) == NULL) {
    i5TraceDirPrint("malloc failure\n");
    return -1;
  }

  ptlv = (i5_tlv_t *)buf;
  ptlv->type = i5TlvBSSConfigurationRequestType;
  ptlv->length = htons(obj_len + sizeof(i5_dpp_attribute_t));
  /* Add DPP attribute header 2 bytes of ID and 2 bytes of length */
  dpp_attr_hdr.id = I5_DPP_ATTR_CONFIG_REQ_OBJ;
  dpp_attr_hdr.length = obj_len;
  memcpy(&buf[sizeof(i5_tlv_t)], &dpp_attr_hdr, sizeof(i5_dpp_attribute_t));
  memcpy(&buf[sizeof(i5_tlv_t) + sizeof(i5_dpp_attribute_t)], config_req_obj, obj_len);
  i5Trace("len %u data %s\n", obj_len, config_req_obj);

  rc = i5MessageInsertTlv(pmsg, buf, len);

  free(buf);

  return rc;
}

/* Extract BSS config request TLV */
int i5TlvBSSConfigurationRequestTypeExtract(i5_message_type *pmsg, char **config_req_obj,
  unsigned int *obj_len)
{
  uint8 *pvalue;
  uint8 *pos, *end;
  unsigned int length;
  i5_dpp_attribute_t *pdpp_attr_hdr;

  if (i5MessageTlvExtract(pmsg, i5TlvBSSConfigurationRequestType, &length, &pvalue,
    i5MessageTlvExtractWithReset) != 0) {
    i5Trace("i5TlvBSSConfigurationRequestType Doesn't exist\n");
    goto end;
  }

  pos = pvalue;
  end = (pvalue + length);

  for (; (end - pos) >= sizeof(i5_dpp_attribute_t); pos += pdpp_attr_hdr->length) {
    pdpp_attr_hdr = (i5_dpp_attribute_t*)pos;
    pos += sizeof(i5_dpp_attribute_t);

    if (pdpp_attr_hdr->length > (end - pos)) {
      i5TraceError("For DPP Attribute 0x%x Attribute Length %u is greater than the remaining "
        "length %u\n", pdpp_attr_hdr->id, pdpp_attr_hdr->length, (unsigned int)(end - pos));
      goto end;
    }

    if (pdpp_attr_hdr->id != I5_DPP_ATTR_CONFIG_REQ_OBJ) {
      i5TraceError("Unsupported DPP attribute ID 0x%x of length %u\n", pdpp_attr_hdr->id,
        pdpp_attr_hdr->length);
      continue;
    }

    *config_req_obj = (char*)malloc(pdpp_attr_hdr->length + 1);
    if ((*config_req_obj) == NULL) {
      i5TraceDirPrint("malloc error\n");
      goto end;
    }

    memset(*config_req_obj, 0, (pdpp_attr_hdr->length + 1));
    memcpy(*config_req_obj, pos, pdpp_attr_hdr->length);
    *obj_len = pdpp_attr_hdr->length;
    i5Trace("BSS configuration request %s\n", *config_req_obj);
    break;
  }
  return 0;

end:
  return -1;
}

/* Insert BSS Configuration Report TLV */
int i5TlvBSSConfigurationReportTypeInsert(i5_message_type *pmsg, i5_dm_device_type *pdevice)
{
  unsigned char *pInterfaceCount, *pbuf, *pmem, count = 0;
  i5_tlv_t *ptlv;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pbss;
  int rc = 0;

  if ((i5_config.map_profile == ieee1905_map_profile0) ||
    (i5_config.map_profile == ieee1905_map_profile2)) {
    i5Trace("Our MAP Profile is %d. Not inserting i5TlvBSSConfigurationReportType TLV(0x%X)\n",
      i5_config.map_profile, i5TlvBSSConfigurationReportType);
    return 0;
  }

  i5Trace("\n");
  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	/* Header filled at the end */
  pInterfaceCount = pbuf;		/* Interface count filled at the end */
  pbuf++;
  foreach_i5glist_item(pdmif, i5_dm_interface_type, pdevice->interface_list) {
    if (!I5_IS_IFR_WIRELESS(pdmif->flags)) {
      continue;
    }
    eacopy(pdmif->InterfaceId, pbuf);
    pbuf += MAC_ADDR_LEN;
    *pbuf = (unsigned char)pdmif->BSSNumberOfEntries;
    pbuf++;
    foreach_i5glist_item(pbss, i5_dm_bss_type, pdmif->bss_list) {
      uint8 caps = 0x0;

      eacopy(pbss->BSSID, pbuf);
      pbuf += MAC_ADDR_LEN;
      if (I5_IS_BSS_BACKHAUL(pbss->mapFlags)) {
        setbit(&caps, 7);
      }
      if (I5_IS_BSS_FRONTHAUL(pbss->mapFlags)) {
        setbit(&caps, 6);
      }
      if (I5_IS_BSS_PROF1_DISALLOWED(pbss->mapFlags)) {
        setbit(&caps, 5);
      }
      if (I5_IS_BSS_PROF2_DISALLOWED(pbss->mapFlags)) {
        setbit(&caps, 4);
      }
      if (I5_BSS_IS_MBSSID_SET(pbss)) {
        setbit(&caps, 3);
      }
      if (I5_BSS_IS_TRANSMITTED_BSS(pbss)) {
        setbit(&caps, 2);
      }
      *pbuf = caps;
      pbuf++;

      *pbuf = 0;	/* Reserved bits 0-7 */
      pbuf++;

      *pbuf = (unsigned char)pbss->ssid.SSID_len;
      pbuf++;
      memcpy(pbuf, pbss->ssid.SSID, pbss->ssid.SSID_len);
      pbuf += pbss->ssid.SSID_len;
    }
    count++;
  }

  *pInterfaceCount = count;

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvBSSConfigurationReportType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);
  return rc;
}

/* Extract BSS Configuration Report TLV */
int i5TlvBSSConfigurationReportTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice,
  bool set_ap_configured)
{
  int rc, ifr_count, idx, jdx;
  unsigned char *pvalue;
  unsigned int length, pos = 0;
  const unsigned int ifr_info_min_len = 7; /* RUID(6) + number of bss(1) */
  const unsigned int bss_info_min_len = 9; /* BSSID(6) + caps(1) + reserved(1) + ssid len(1) */
  i5_dm_interface_type *pinterface;

  i5Trace("\n");
  rc = i5MessageTlvExtract(pmsg, i5TlvBSSConfigurationReportType, &length,
    &pvalue, i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }

  if (length < i5TlvBSSConfigurationReport_Min_Length) {
    i5TraceError("BSS Congiguration results tlv's length %d is less than min expected len %d "
      "skip processing\n", length, i5TlvBSSConfigurationReport_Min_Length);
    goto end;
  }

  ifr_count = pvalue[pos];
  pos++;
  if ((length - pos) < (ifr_count * ifr_info_min_len)) {
    i5TraceError("BSS Congiguration results tlv's remaining length %d for interface count %d is "
      "less than expected len %d skip processing \n",
      (length - pos), ifr_count, (ifr_count * ifr_info_min_len));
    goto end;
  }

  for (idx = 0; idx < ifr_count; idx++) {
    unsigned char interface_mac[MAC_ADDR_LEN];
    unsigned char bss_count;

    memcpy(interface_mac, &pvalue[pos], MAC_ADDR_LEN);
    pos += MAC_ADDR_LEN;
    bss_count = pvalue[pos];
    pos++;
    if ((length - pos)  < (bss_count * bss_info_min_len)) {
      i5TraceError("BSS Congiguration results tlv's remaining length %d for bss count %d is "
        "less than expected len %d skip processing \n",
	(length - pos), bss_count, (bss_count * bss_info_min_len));
      goto end;
    }
    for (jdx = 0; jdx < bss_count; jdx++) {
      unsigned char bssid[MAC_ADDR_LEN];
      ieee1905_ssid_type ssid;
      uint8 caps = 0;

      memset(&ssid, 0, sizeof(ssid));
      memcpy(bssid, &pvalue[pos], MAC_ADDR_LEN);
      pos += MAC_ADDR_LEN;
      caps = pvalue[pos];
      pos++;
      pos++; /* Reserved */
      ssid.SSID_len = pvalue[pos];
      pos++;
      if ((length - pos) < ssid.SSID_len) {
        i5TraceError("BSS Congiguration results tlv's remaining length %d is less than "
          "ssid len %d skip processing\n", (length - pos), ssid.SSID_len);
        goto end;
      }
      memcpy(ssid.SSID, &pvalue[pos], ssid.SSID_len);
      pos += ssid.SSID_len;

      i5TraceInfo("Ifr[%d]["I5_MAC_DELIM_FMT"] BSS[%d]["I5_MAC_DELIM_FMT"] cap [0x%x] "
        "ssid_len %d ssid[%s] set_ap_configured[%d]\n", idx, I5_MAC_PRM(interface_mac), jdx,
        I5_MAC_PRM(bssid), caps, ssid.SSID_len, (char*)ssid.SSID, set_ap_configured);
      i5DmBSSUpdate(pdevice, interface_mac, bssid, &ssid, caps);
    }

    if (set_ap_configured && ((pinterface = i5DmInterfaceFind(pdevice, interface_mac)) != NULL)) {
      ieee1905_radio_caps_type *RadioCaps = &pinterface->ApCaps.RadioCaps;
      int if_band = ieee1905_get_band_from_radiocaps(RadioCaps);
      pinterface->flags |= I5_FLAG_IFR_WIRELESS;
      pinterface->flags |= I5_FLAG_IFR_MAIN_WIRELESS;
      if (if_band != BAND_INV && i5_config.cbs.ap_configured) {
        i5_config.cbs.ap_configured(pdevice, pinterface, if_band);
      }
    }
  }

  return rc;

end:
  return -1;
}

/* Insert BSS Configuraion Response TLV */
int i5TlvBSSConfigurationResponseTypeInsert(i5_message_type *pmsg, char *config_resp_obj,
  unsigned int obj_len)
{
  unsigned int len = sizeof(i5_tlv_t) + obj_len;
  i5_tlv_t *ptlv;
  uint8 *buf;
  int rc;

  i5Trace("\n");
  if ((buf = (uint8 *)malloc(len)) == NULL) {
    i5TraceDirPrint("malloc failure\n");
    return -1;
  }

  ptlv = (i5_tlv_t *)buf;
  ptlv->type = i5TlvBSSConfigurationResponseType;
  ptlv->length = htons(obj_len);

  memcpy(&buf[sizeof(i5_tlv_t)], config_resp_obj, obj_len);
  i5Trace("len %u\n", obj_len);

  rc = i5MessageInsertTlv(pmsg, buf, len);

  free(buf);

  return rc;
}

int i5TlvProcessBSSConfigurationResponseAttributes(i5_dm_device_type *pdevice,
  uint8 *tlvBody, unsigned int tlvBodyLen)
{
  char *config_resp_obj;
  uint8 *pos = tlvBody, *end = tlvBody + tlvBodyLen;
  i5_dpp_attribute_t *pdpp_attr_hdr;
  ieee1905_client_bssinfo_type *bss_info;
  ieee1905_ssid_list_type *ssid_type;
  unsigned short vlan_id;

  for (; (end - pos) >= sizeof(i5_dpp_attribute_t); pos += pdpp_attr_hdr->length) {
    pdpp_attr_hdr = (i5_dpp_attribute_t*)pos;
    pos += sizeof(i5_dpp_attribute_t);

    if (pdpp_attr_hdr->length > (end - pos)) {
      i5TraceError("For DPP Attribute 0x%x Attribute Length %u is greater than the remaining "
        "length %u\n", pdpp_attr_hdr->id, pdpp_attr_hdr->length, (unsigned int)(end - pos));
      goto end;
    }

    if (pdpp_attr_hdr->id != I5_DPP_ATTR_CONFIG_OBJ) {
      i5TraceError("Unsupported DPP attribute ID 0x%x of length %u\n", pdpp_attr_hdr->id,
        pdpp_attr_hdr->length);
      continue;
    }

    config_resp_obj = (char*)malloc(pdpp_attr_hdr->length + 1);
    if (config_resp_obj == NULL) {
      i5TraceDirPrint("malloc failure\n");
      goto end;
    }
    memset(config_resp_obj, 0, pdpp_attr_hdr->length + 1);

    bss_info = (ieee1905_client_bssinfo_type *)malloc(sizeof(*bss_info));
    if (!bss_info) {
      i5TraceDirPrint("malloc failure\n");
      free(config_resp_obj);
      goto end;
    }
    memset(bss_info, 0, sizeof(*bss_info));

    memcpy(config_resp_obj, pos, pdpp_attr_hdr->length);
    if (i5_config.cbs.parse_dpp_config_resp_obj) {
      i5Trace("Parsing dpp config response obj %s\n", config_resp_obj);
      i5_config.cbs.parse_dpp_config_resp_obj(config_resp_obj, bss_info);
      i5TraceInfo("ssid len: %d ssid: %s bss_info.map_flag[%d]\n", bss_info->ssid.SSID_len,
	bss_info->ssid.SSID, bss_info->map_flag);

      /* If the traffic separation policy has SSID whose VLAN ID is not primary label it as Guest */
      ssid_type = i5DmTSPolicyFindSSID(&i5_config.policyConfig.ts_policy_list, bss_info->ssid.SSID,
        &vlan_id);
      i5Trace("SSID[%s] VLAN ID[%d] Primary VLAN ID[%d]. SSID %s\n", bss_info->ssid.SSID,
        vlan_id, i5_config.policyConfig.prim_vlan_id, ssid_type ? "Present" : "Not Present");
      if (ssid_type && vlan_id != i5_config.policyConfig.prim_vlan_id) {
        bss_info->map_flag |= IEEE1905_MAP_FLAG_GUEST;
      }

      if (i5DmInterfaceFind(pdevice, bss_info->RUID) == NULL) {
        i5TraceError("RUID["MACF"] which was in DPP config response is not found in this device\n",
          ETHERP_TO_MACF(bss_info->RUID));
        free(bss_info);
      } else {
        (void)i5DmAddClientBssInfoToDppConfigObjList(bss_info);
        i5Trace("Added dpp config response obj for RUID["MACF"]\n", ETHERP_TO_MACF(bss_info->RUID));
      }
    } else {
      free(bss_info);
    }

    free(config_resp_obj);
  }

  return 0;

end:
  return -1;
}

/* Extract BSS config response TLV. Parse JSON dpp config response data and store it to device's
 * interface matching the RUID
 */
int i5TlvBSSConfigurationResponseTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice)
{
  uint8 *pvalue;
  unsigned int obj_len = 0;
  int rc = -1;

  i5Trace("\n");

  i5DmDppConfigObjsListFree();

  i5MessageReset(pmsg);
  while (i5MessageTlvExtract(pmsg, i5TlvBSSConfigurationResponseType, &obj_len, &pvalue,
    i5MessageTlvExtractWithoutReset) == 0) {
    if (obj_len <= 0) {
      goto end;
    }

    if (i5TlvProcessBSSConfigurationResponseAttributes(pdevice, pvalue, obj_len) != 0) {
      goto end;
    }
  }

  rc = 0;

end:

  if (rc) {
    i5TraceDirPrint("Error while processing bss configuration response tlv \n");
    i5DmDppConfigObjsListFree();
  }

  return rc;
}

/* TLV to Insert DPP CCE Indication */
int i5TlvDPPCCEIndicationInsert(i5_message_type *pmsg, uint8 enable)
{
  unsigned char buf[sizeof(i5_tlv_t) + i5TlvDPPCCEIndication_Length];
  i5_tlv_t *ptlv = (i5_tlv_t *)buf;
  int len = 0;

  i5Trace("\n");
  ptlv->type = i5TlvDPPCCEIndicationType;
  ptlv->length = htons(i5TlvDPPCCEIndication_Length);
  len += sizeof(i5_tlv_t);

  buf[len] = (unsigned char)enable;
  len++;

  i5Trace("DPP CCE Indication[%s] \n", enable ? "Enable" : "Disable");
  return (i5MessageInsertTlv(pmsg, buf, len));
}

/* Extract DPP CCE Indication */
int i5TlvDPPCCEIndicationExtract(i5_message_type *pmsg, uint8 *enable)
{
  unsigned char *pvalue;
  unsigned int length;

  i5Trace("\n");

  if (i5MessageTlvExtract(pmsg, i5TlvDPPCCEIndicationType, &length, &pvalue,
    i5MessageTlvExtractWithReset) == 0) {
    if (length >= i5TlvDPPCCEIndication_Length) {
      *enable = *pvalue;
      pvalue++;
    } else {
      goto end;
    }
  } else {
    goto end;
  }

  i5Trace("DPP CCE Indication[%s] \n", *enable ? "Enable" : "Disable");
  return 0;

end:
  return -1;
}

/* TLV to Insert DPP Chirp Value */
int i5TlvDPPChirpValueInsert(i5_message_type *pmsg, i5_dpp_chirp_value_t *dpp_chirp)
{
  unsigned char *pbuf, *pmem;
  i5_tlv_t *ptlv;
  int rc = 0;

  if ((i5_config.map_profile == ieee1905_map_profile0) ||
    (i5_config.map_profile == ieee1905_map_profile2)) {
    i5Trace("Our MAP Profile is %d. Not inserting i5TlvDPPChirpValueType TLV(0x%X)\n",
      i5_config.map_profile, i5TlvDPPChirpValueType);
    return 0;
  }

  i5Trace("\n");

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  *pbuf = dpp_chirp->flags;
  pbuf++;

  /* if the Enrollee MAC Address present bit is set, add STA MAC address */
  if (dpp_chirp->flags & I5_DPP_CHIRP_FLAGS_ENROLLE_MAC_PRESENT) {
    eacopy(dpp_chirp->enrollee_mac, pbuf);
    pbuf += MAC_ADDR_LEN;
  }

  /* Insert hash length and hash */
  *pbuf = dpp_chirp->hash_len;
  pbuf++;
  if (dpp_chirp->hash_len) {
    memcpy(pbuf, dpp_chirp->hash, dpp_chirp->hash_len);
    pbuf += dpp_chirp->hash_len;
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvDPPChirpValueType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);
  return rc;
}

/* Extract DPP Chirp Value */
int i5TlvDPPChirpValueExtract(i5_dpp_chirp_value_t *dpp_chirp, unsigned char *pvalue,
  unsigned int length)
{
  unsigned int extracted_len = i5TlvDPPChirpValue_Length;

  i5Trace("\n");

  if (length < i5TlvDPPChirpValue_Length) {
    goto end;
  }

  dpp_chirp->flags = *pvalue;
  pvalue++;

  if (dpp_chirp->flags & I5_DPP_CHIRP_FLAGS_ENROLLE_MAC_PRESENT) {
    if (length < (extracted_len + MAC_ADDR_LEN)) {
      goto end;
    }
    eacopy(pvalue , dpp_chirp->enrollee_mac);
    pvalue += MAC_ADDR_LEN;
    extracted_len += MAC_ADDR_LEN;
  }

  dpp_chirp->hash_len = *pvalue;
  pvalue++;

  if (length < (extracted_len + dpp_chirp->hash_len)) {
    goto end;
  }

  if (dpp_chirp->hash_len) {
    if ((dpp_chirp->hash = (unsigned char *)malloc(dpp_chirp->hash_len)) == NULL) {
      i5TraceDirPrint("malloc error\n");
      goto end;
    }
    memcpy(dpp_chirp->hash, pvalue, dpp_chirp->hash_len);
    pvalue += dpp_chirp->hash_len;
  }

  return 0;

end:
  return -1;
}

/* Extract one DPP Chirp Value TLV */
int i5TlvDPPChirpValueExtractOneTlv(i5_message_type *pmsg, i5_dpp_chirp_value_t *dpp_chirp)
{
  int rc;
  unsigned char *pvalue;
  unsigned int length;

  i5Trace("\n");

  rc = i5MessageTlvExtract(pmsg, i5TlvDPPChirpValueType, &length, &pvalue,
    i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }

  rc = i5TlvDPPChirpValueExtract(dpp_chirp, pvalue, length);

  return rc;

end:
  return -1;
}

/* Extract All DPP Chirp Value TLVs in the message and call the callback for each TLV */
int i5TlvDPPChirpValueExtractAllTlvs(i5_message_type *pmsg, unsigned char *src_mac_addr)
{
  int rc;
  unsigned char *pvalue;
  unsigned int length;
  i5_dpp_chirp_value_t dpp_chirp;

  i5Trace("\n");

  i5MessageReset(pmsg);
  while (i5MessageTlvExtract(pmsg, i5TlvDPPChirpValueType, &length, &pvalue,
    i5MessageTlvExtractWithoutReset) == 0) {

    memset(&dpp_chirp, 0, sizeof(dpp_chirp));

    rc = i5TlvDPPChirpValueExtract(&dpp_chirp, pvalue, length);
    if (rc != 0) {
      goto end;
    }

    i5TraceInfo("DPP Chirp Value Fields: Flags[0x%x], "
      "EnrolleMAC["I5_MAC_DELIM_FMT"], Hash Length[%d]\n",
      dpp_chirp.flags, I5_MAC_PRM(dpp_chirp.enrollee_mac), dpp_chirp.hash_len);

    if (i5_config.cbs.dpp_chirp_notification) {
      i5_config.cbs.dpp_chirp_notification(&dpp_chirp, src_mac_addr);
    }

    if (dpp_chirp.hash) {
      free(dpp_chirp.hash);
      dpp_chirp.hash = NULL;
    }
  }

  return 0;

end:
  return -1;
}

/* TLV to Insert 1905 Encap EAPOL Value */
int i5Tlv1905EncapEAPOLValueInsert(i5_message_type *pmsg, i5_1905_encap_eapol_t *encap_1905_eapol)
{
  unsigned char *pbuf, *pmem;
  i5_tlv_t *ptlv;
  int rc = 0;

  i5Trace("\n");

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  if (encap_1905_eapol->frame_length) {
    memcpy(pbuf, encap_1905_eapol->frame, encap_1905_eapol->frame_length);
    pbuf += encap_1905_eapol->frame_length;
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5Tlv1905EncapEAPOLType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);
  return rc;
}

/* Extract one 1905 Encap EAPOL Value TLV */
int i5Tlv1905EncapEAPOLValueExtract(i5_message_type *pmsg, i5_1905_encap_eapol_t *encap_1905_eapol)
{
  int rc;
  unsigned char *pvalue;
  unsigned int length;

  i5Trace("\n");

  rc = i5MessageTlvExtract(pmsg, i5Tlv1905EncapEAPOLType, &length, &pvalue,
    i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }

  encap_1905_eapol->frame_length = length;
  if (encap_1905_eapol->frame_length == 0) {
    goto end;
  }

  if ((encap_1905_eapol->frame = (unsigned char *)malloc(encap_1905_eapol->frame_length)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    goto end;
  }
  memcpy(encap_1905_eapol->frame, pvalue, encap_1905_eapol->frame_length);
  pvalue += encap_1905_eapol->frame_length;

  return 0;

end:
  return -1;
}

/* TLV to Insert 1905 Encap DPP */
int i5Tlv1905EncapDPPInsert(i5_message_type *pmsg, i5_1905_encap_dpp_t *dpp_1905_encap)
{
  unsigned char *pbuf, *pmem;
  i5_tlv_t *ptlv;
  int rc = 0;
  unsigned short tlv_len = (dpp_1905_encap->frame_length + sizeof(i5_tlv_t) + MAC_ADDR_LEN +
    i5Tlv1905EncapDPP_Length);

  i5Trace("\n");

  if ((pmem = (unsigned char *)malloc(tlv_len)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  *pbuf = dpp_1905_encap->flags;
  pbuf++;

  /* if the Enrollee MAC Address present bit is set, add STA MAC address */
  if (dpp_1905_encap->flags & I5_1905_ENCAP_DPP_FLAGS_ENROLLE_MAC_PRESENT) {
    eacopy(dpp_1905_encap->enrollee_mac, pbuf);
    pbuf += MAC_ADDR_LEN;
  }

  *pbuf = dpp_1905_encap->frame_type;
  pbuf++;

  *((unsigned short *)pbuf) = htons(dpp_1905_encap->frame_length);
  pbuf += 2;

  if (dpp_1905_encap->frame_length) {
    memcpy(pbuf, dpp_1905_encap->frame, dpp_1905_encap->frame_length);
    pbuf += dpp_1905_encap->frame_length;
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5Tlv1905EncapDPPType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);
  return rc;
}

/* Extract 1905 Encap DPP */
int i5Tlv1905EncapDPPExtract(i5_message_type *pmsg, i5_1905_encap_dpp_t *dpp_1905_encap)
{
  int rc;
  unsigned char *pvalue;
  unsigned int length, extracted_len = i5Tlv1905EncapDPP_Length;

  i5Trace("\n");

  rc = i5MessageTlvExtract(pmsg, i5Tlv1905EncapDPPType, &length, &pvalue,
    i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }

  if (length < i5Tlv1905EncapDPP_Length) {
    goto end;
  }

  dpp_1905_encap->flags = *pvalue;
  pvalue++;

  if (dpp_1905_encap->flags & I5_1905_ENCAP_DPP_FLAGS_ENROLLE_MAC_PRESENT) {
    if (length < (extracted_len + MAC_ADDR_LEN)) {
      goto end;
    }
    eacopy(pvalue , dpp_1905_encap->enrollee_mac);
    pvalue += MAC_ADDR_LEN;
    extracted_len += MAC_ADDR_LEN;
  }

  dpp_1905_encap->frame_type = *pvalue;
  pvalue++;

  dpp_1905_encap->frame_length = ntohs(*((unsigned short *)pvalue));
  pvalue += 2;

  if (length < (extracted_len + dpp_1905_encap->frame_length)) {
    goto end;
  }

  if (dpp_1905_encap->frame_length) {
    if ((dpp_1905_encap->frame = (unsigned char *)malloc(dpp_1905_encap->frame_length)) == NULL) {
      i5TraceDirPrint("malloc error\n");
      goto end;
    }
    memcpy(dpp_1905_encap->frame, pvalue, dpp_1905_encap->frame_length);
    pvalue += dpp_1905_encap->frame_length;
  }

  return 0;

end:
  return -1;
}

/* Insert DPP Message TLV */
int i5TlvDPPMessageInsert(i5_message_type *pmsg, i5_direct_encap_dpp_t *dpp_direct_encap)
{
  unsigned char *pbuf, *pmem;
  i5_tlv_t *ptlv;
  int rc = 0;
  unsigned short tlv_len = (dpp_direct_encap->frame_length + sizeof(i5_tlv_t));

  i5Trace("\n");

  if ((pmem = (unsigned char *)malloc(tlv_len)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  if (dpp_direct_encap->frame_length) {
    memcpy(pbuf, dpp_direct_encap->frame, dpp_direct_encap->frame_length);
    pbuf += dpp_direct_encap->frame_length;
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvDPPMessageType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);
  return rc;
}

/* Extract DPP Message TLV */
int i5TlvDPPMessageExtract(i5_message_type *pmsg, i5_direct_encap_dpp_t *dpp_direct_encap)
{
  int rc;
  unsigned char *pvalue;
  unsigned int length;

  i5Trace("\n");

  rc = i5MessageTlvExtract(pmsg, i5TlvDPPMessageType, &length, &pvalue,
    i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }

  if (length <= i5TlvDPPMessage_Length) {
    goto end;
  }

  /* Extract DPP Action Frame Length */
  dpp_direct_encap->frame_length = length;

  /* Extract DPP Action Frame Data */
  dpp_direct_encap->frame = (unsigned char *)calloc(1, dpp_direct_encap->frame_length);
  if (dpp_direct_encap->frame == NULL) {
    i5TraceDirPrint("malloc error\n");
    rc = -1;
    goto end;
  }
  memcpy(dpp_direct_encap->frame, pvalue, dpp_direct_encap->frame_length);
  pvalue += dpp_direct_encap->frame_length;

end:
  return rc;
}

/*Insert and report AP WiFi6 Capabilities TLV */
int i5TlvAPWiFi6CapabilitiesTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  ieee1905_wifi6_caps_type *WiFi6Caps)
{
  unsigned char *pbuf, *pmem ;
  i5_tlv_t *ptlv;
  int rc = 0;
  uint8 noroles = 0;

  if (i5_config.map_profile == ieee1905_map_profile0) {
    i5Trace("Our MAP Profile is %d. Not inserting i5TlvAPWiFi6CapabilitiesType TLV(0x%X)\n",
      i5_config.map_profile, i5TlvAPWiFi6CapabilitiesType);
    return 0;
  }

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    printf("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t); // Header filled at the end
  memcpy(pbuf, mac, MAC_ADDR_LEN);
  pbuf += MAC_ADDR_LEN;

  /* Fill No of Roles */
  *pbuf = WiFi6Caps->Noroles;
  noroles = WiFi6Caps->Noroles;

  if (WiFi6Caps->Noroles <= 0 ) {
    pbuf++;
    goto end;
  }
  pbuf++;

  /* HE features don't change based on AP/STA so we Add
   * same values for both AP and STA
   */
  while (noroles) {
    /* Fill Agent Roles flag */
    if (noroles > 1) {
	/* default 1st will support AP alone */
      *pbuf = WiFi6Caps->Roleflag ^ IEEE1905_AP_WiFi6CAP_BSTA_AGENTROLE;
    } else {
      *pbuf = WiFi6Caps->Roleflag;
    }
    pbuf++;

    /* Add mcs map of 80MHZ */
    *((unsigned short *)pbuf) = htons(WiFi6Caps->TxBW80MCSMap);
    pbuf += 2;

    *((unsigned short *)pbuf) = htons(WiFi6Caps->RxBW80MCSMap);
    pbuf += 2;

    /* Add mcs map of 160 MHz */
    if (WiFi6Caps->Roleflag & IEEE1905_AP_WiFi6CAP_160MHZ) {
      *((unsigned short *)pbuf) = htons(WiFi6Caps->TxBW160MCSMap);
      pbuf += 2;
      *((unsigned short *)pbuf) = htons(WiFi6Caps->RxBW160MCSMap);
      pbuf += 2;
    }

    /* Add mcs map of 80p80 MHz */
    if (WiFi6Caps->Roleflag & IEEE1905_AP_WiFi6CAP_80P80MHZ) {
      *((unsigned short *)pbuf) = htons(WiFi6Caps->TxBW80p80MCSMap);
      pbuf += 2;
      *((unsigned short *)pbuf) = htons(WiFi6Caps->RxBW80p80MCSMap);
      pbuf += 2;
    }

    /* Fill HE support flag */
    *pbuf =  WiFi6Caps->HESupportflag;
    pbuf++;
    /* Fill MIMOMax user flag */
    *pbuf =  WiFi6Caps->MIMOMaxUserflag;
    pbuf++;
    /* Fill Max DLOFDMATX  flag */
    *pbuf =  WiFi6Caps->DLOFDMATX;
    pbuf++;
    /* Fill Max ULOFDMATX  flag */
    *pbuf =   WiFi6Caps->ULOFDMARX;
    pbuf++;
    /* Fill General flags */
    *pbuf = WiFi6Caps->Generalflags;
    pbuf++;
    noroles--;
  }

end:
  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvAPWiFi6CapabilitiesType;
  ptlv->length = htons(pbuf-pmem-sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf-pmem);

  free(pmem);
  return (rc);
}

/* Extract AP WiFi6 Capabilities TLV */
int i5TlvAPWiFi6CapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice)
{
  i5_dm_interface_type *pdmif;
  unsigned char *pvalue, mac[MAC_ADDR_LEN];
  unsigned int length;
  unsigned char *hemcsmap;
  int rc = 0;

  i5MessageReset(pmsg);
  while((rc = i5MessageTlvExtract(pmsg, i5TlvAPWiFi6CapabilitiesType,
    &length, &pvalue, i5MessageTlvExtractWithoutReset)) == 0) {
    if (length < i5TlvAPWiFi6Capabilities_Min_Length) {
      rc = -1;
      goto end;
    }

    memset(mac, 0, MAC_ADDR_LEN);
    memcpy(mac, pvalue, MAC_ADDR_LEN);
    pvalue += MAC_ADDR_LEN;
    pdmif = i5DmInterfaceFind(pdevice, mac);

    if (pdmif == NULL) {
      i5TraceError("Interface " I5_MAC_DELIM_FMT " does not exist\n", I5_MAC_PRM(mac));
      continue;
    }
    pdmif->flags |= I5_FLAG_IFR_WIRELESS;
    pdmif->flags |= I5_FLAG_IFR_MAIN_WIRELESS;

    pdmif->ApCaps.WiFi6Caps.Noroles = *pvalue;
    if (pdmif->ApCaps.WiFi6Caps.Noroles == 0 ) {
      pvalue++;
      goto end;
    }
    pvalue++;

    /*  TODO: Currently we are parsing for one role only. */
    pdmif->ApCaps.WiFi6Caps.Roleflag = *pvalue;
    pvalue++;
    hemcsmap = pvalue;
    pvalue += 4;

    /* Extract 80Mhz mcs map */
    pdmif->ApCaps.WiFi6Caps.TxBW80MCSMap = ntohs(*((unsigned short *)hemcsmap));
    hemcsmap += 2;
    pdmif->ApCaps.WiFi6Caps.RxBW80MCSMap = ntohs(*((unsigned short *)hemcsmap));
    hemcsmap += 2;

    /* Extract 160 Mhz mcs map */
    if (pdmif->ApCaps.WiFi6Caps.Roleflag & IEEE1905_AP_WiFi6CAP_160MHZ) {
      hemcsmap = pvalue;
      pvalue += 4;
      pdmif->ApCaps.WiFi6Caps.TxBW160MCSMap = ntohs(*((unsigned short *)hemcsmap));
      hemcsmap += 2;
      pdmif->ApCaps.WiFi6Caps.RxBW160MCSMap = ntohs(*((unsigned short *)hemcsmap));
      hemcsmap += 2;
    }

    /* Extract 80p80 mcs map */
    if (pdmif->ApCaps.WiFi6Caps.Roleflag & IEEE1905_AP_WiFi6CAP_80P80MHZ) {
      hemcsmap = pvalue;
      pvalue += 4;
      pdmif->ApCaps.WiFi6Caps.TxBW80p80MCSMap = ntohs(*((unsigned short *)hemcsmap));
      hemcsmap += 2;
      pdmif->ApCaps.WiFi6Caps.RxBW80p80MCSMap = ntohs(*((unsigned short *)hemcsmap));
      hemcsmap += 2;
    }

    pdmif->ApCaps.WiFi6Caps.HESupportflag = *pvalue;
    pvalue++;
    pdmif->ApCaps.WiFi6Caps.MIMOMaxUserflag = *pvalue;
    pvalue++;
    pdmif->ApCaps.WiFi6Caps.DLOFDMATX = *pvalue;
    pvalue++;
    pdmif->ApCaps.WiFi6Caps.ULOFDMARX = *pvalue;
    pvalue++;
    pdmif->ApCaps.WiFi6Caps.Generalflags = *pvalue;
    pvalue++;
    pdmif->ApCaps.WiFi6Caps.Valid = 1;
  }

end:
  if (rc == -2) {
    rc = 0;
  }
  return rc;
}

/* TLV to Insert Agent List */
int i5TlvAgentListTypeInsert(i5_message_type *pmsg)
{
  i5_dm_device_type *device;
  unsigned char *pbuf, *pmem, *tmpPtrLoc;
  i5_tlv_t *ptlv;
  int rc = 0;
  unsigned int count = 0;

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }
  pbuf = pmem + sizeof(i5_tlv_t); // Header filled at the end

  tmpPtrLoc = pbuf; /* Fill number of Agent List field later */
  pbuf++;

  foreach_i5glist_item(device, i5_dm_device_type, i5_dm_network_topology.device_list) {

    if (I5_IS_MULTIAP_CONTROLLER(device->flags)) {
      continue;
    }

    /* Insert ALMAC Unique Identifier of the Multi-AP Agent */
    memcpy(pbuf, device->DeviceId, MAC_ADDR_LEN);
    pbuf += MAC_ADDR_LEN;

    /* Insert MAP profile of the Multi-AP Agent. set the Multi-AP Profile field to the value of the
    * Multi-AP Profile field of the Multi-AP Profile TLV received from each Multi-AP Agent
    * (If the Multi-AP Profile field is not received, set to Profile-1)
    */
    if (device->profile == ieee1905_map_profile0) {
      *pbuf = ieee1905_map_profile1;
    } else {
      *pbuf = device->profile;
    }
    pbuf++;

    /* Insert 1905 security enabled/disabled of the Multi-AP Agent */
    if (device->ptk_len > 0) {
      *pbuf = (unsigned char)MAP_1905_SEC_ENABLED;
    } else {
      *pbuf = (unsigned char)MAP_1905_SEC_DISABLED;
    }
    i5TraceInfo("Agent List info ALMAC " I5_MAC_DELIM_FMT " profile [%d] sec [0x%x] \n",
      I5_MAC_PRM(device->DeviceId), device->profile, *pbuf);
    pbuf++;

    count++;
  }
  /* Now fill the number of agents field */
  *tmpPtrLoc = (unsigned char)count;

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvAgentListValueType;
  ptlv->length = htons(pbuf-pmem-sizeof(i5_tlv_t));

  rc = i5MessageInsertTlv(pmsg, pmem, pbuf-pmem);

  free(pmem);
  return (rc);
}

/* Extract Agent List */
int i5TlvAgentListTypeExtract(i5_message_type *pmsg)
{
  i5_dm_device_type *pdevice = pmsg->pDevice;
  unsigned char *pvalue, *src_mac_addr;
  unsigned int length, num_of_agents = 0, idx_agent;
  int rc = 0;
  ieee1905_glist_t agent_list; /* List of i5_agentlist_t nodes */
  i5_agentlist_t *agentinfo = NULL; /* List node */

  ieee1905_glist_init(&agent_list);

  src_mac_addr = i5MessageSrcMacAddressGet(pmsg);
  if (pdevice == NULL) {
    i5TraceError("Agent device does not found " I5_MAC_DELIM_FMT " \n",
      I5_MAC_PRM(src_mac_addr));
    rc = -1;
    goto end;
  }

  rc = i5MessageTlvExtract(pmsg, i5TlvAgentListValueType, &length, &pvalue, i5MessageTlvExtractWithReset);

  /* If TLV length is not proper then the TLV is corrupted */
  if (rc != 0 || (length < i5TlvAgentList_Min_Length)) {
    i5TraceError("Agent List TLV/length is not proper.\n");
    rc = -1;
    goto end;
  }

  /* Extract Total Number of agents */
  num_of_agents = *pvalue;
  pvalue++;
  if (num_of_agents <= 0 || (length < (num_of_agents * i5TlvAgentList_Length))) {
    i5TraceError("Agent List count not matching the TLV length \n");
    rc = -1;
    goto end;
  }

  /* Extract details for each Agent details */
  for (idx_agent = 0; idx_agent < num_of_agents; idx_agent++) {

    agentinfo = (i5_agentlist_t *)malloc(sizeof(*agentinfo));

    if (!agentinfo) {
      i5TraceDirPrint("Malloc Failed\n");
      rc = -1;
      goto end;
    }
    memset(agentinfo, 0, sizeof(*agentinfo));

    /* Extract agent ALMAC Unique Identifier of the Multi-AP Agent */
    eacopy(pvalue, agentinfo->ALID);
    pvalue += MAC_ADDR_LEN;

    /* Extract agent profile of the Multi-AP Agent */
    agentinfo->map_profile = *pvalue;
    pvalue++;

    /* Extract agent 1905 security enabled/disabled of the Multi-AP Agent */
    agentinfo->SecType = *pvalue;
    pvalue++;

    i5TraceInfo("Agent List info ALMAC " I5_MAC_DELIM_FMT " profile [%d] sec [0x%x] \n",
      I5_MAC_PRM(agentinfo->ALID), agentinfo->map_profile, agentinfo->SecType);

    /* If the device is not found create it. So that while processing agent list it can check the
     * profile
     */
    if (!(agentinfo->pDevice = i5DmDeviceFind(agentinfo->ALID))) {
      agentinfo->pDevice = i5DmDeviceNew(agentinfo->ALID, 0, NULL);
      if (agentinfo->pDevice) {
        agentinfo->pDevice->profile = agentinfo->map_profile;
      }
    }

    ieee1905_glist_append(&agent_list, (dll_t*)agentinfo);
  }

  if (i5_config.cbs.process_agentlist) {
    i5_config.cbs.process_agentlist(&agent_list, src_mac_addr);
  }

  rc = 0;

end:

  i5DmGlistCleanup(&agent_list);

  return rc;
}

/* Insert Encrypted pyload TLV */
i5_message_type *
i5TlvEncryptedPayloadTypeInsert(i5_message_type *pmsg, i5_dm_device_type *dst_dev)
{
  unsigned char *pbuf, *pmem;
  unsigned char *src_mac, *cmdu, *data, *siv;
  i5_tlv_t *ptlv;
  i5_packet_type *ppkt = (i5_packet_type *)pmsg->packet_list.ll.next;
  i5_message_type *enc_pmsg;
  int rc = 0;
  unsigned int data_len = 0, pmem_len;
  unsigned int pkt_data_len, copied_bytes = 0, number_of_bytes_rem;
  i5_uint48_t enc_tx_counter;
  uint64 tmp_counter;

  /* pmsg here has full message including end of message TLV.
   * We dont need to encrypt EOM TLV, but add it after encrypting
   * the other TLV's
  */

  i5Trace("\n");

  if (ppkt == NULL) {
    i5TraceError("There are no packets in this message\n");
    return NULL;
  }

  /* Calculate data length */
  do {
    /* For dumping decrypted packets */
    i5MessageDumpUnencryptedHex(ppkt, I5_MESSAGE_DIR_TX, pmsg->psock);
    data_len += (ppkt->length - I5_MESSAGE_TLV_OFFSET);
    ppkt = (i5_packet_type *)ppkt->ll.next;
  } while (ppkt != NULL);

  /* pmsg here has full message including end of message TLV.
   * We dont need to encrypt EOM TLV, but add it after encrypting
   * the other TLV's
  */
  data_len -= sizeof(i5_tlv_t);

  /* Calculate the buffer size needed for encrypted payload TLV */
  pmem_len = data_len + i5TlvEncryptedPayload_Lnegth + SIV_BLOCK_SZ + sizeof(i5_tlv_t);

  if ((pmem = (unsigned char *)malloc(pmem_len)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return NULL;
  }

  memset(pmem, 0, pmem_len);

  /* Point data to the place where encrypted data will be in the encrypted payload TLV */
  data = pmem + sizeof(i5_tlv_t) + i5TlvEncryptedPayload_Lnegth + SIV_BLOCK_SZ;

  /* point ppkt to first pkt in pmsg */
  ppkt = (i5_packet_type *)pmsg->packet_list.ll.next;

  number_of_bytes_rem = data_len;
  /* Copy the data */
  while (ppkt != NULL && number_of_bytes_rem) {
    pkt_data_len = ppkt->length - I5_MESSAGE_TLV_OFFSET;
    if (pkt_data_len > number_of_bytes_rem) {
      pkt_data_len = number_of_bytes_rem;
    }
    memcpy(data + copied_bytes, ppkt->pbuf + I5_MESSAGE_TLV_OFFSET, pkt_data_len);
    /* Update the number of bytes read */
    copied_bytes += pkt_data_len;
    /* Update number of bytes still remaining */
    number_of_bytes_rem -= pkt_data_len;
    ppkt = (i5_packet_type *)ppkt->ll.next;
  }

  pbuf = pmem + sizeof(i5_tlv_t); /* Header filled at the end */

  /* Get src mac addr */
  src_mac = i5MessageSrcMacAddressGet(pmsg);

  /* Change the endianness of the counter */
  tmp_counter = (uint64)dst_dev->enc_tx_counter.val;
  tmp_counter = hton64(tmp_counter);
  enc_tx_counter.val = tmp_counter >> 16;

  /* 6 bytes encryption transmission counter */
  memcpy(pbuf, &(enc_tx_counter), I5_SEC_COUNTER_LEN);
  pbuf += I5_SEC_COUNTER_LEN;

  /* src mac addr */
  memcpy(pbuf, src_mac, ETHER_ADDR_LEN);
  pbuf += ETHER_ADDR_LEN;

  /* dst mac addr */
  memcpy(pbuf, dst_dev->DeviceId, ETHER_ADDR_LEN);
  pbuf += ETHER_ADDR_LEN;

  /* point ppkt to first pkt in pmsg */
  ppkt = (i5_packet_type *)pmsg->packet_list.ll.next;

  cmdu = ppkt->pbuf + I5_MESSAGE_CMDU_OFFSET;
  /* Point siv to the place where siv output of encryption will be in encrypted payload TLV */
  siv = pbuf + 2;

  rc = ieee1905_encrypt_packet(data, data_len, siv, cmdu, (unsigned char *)&enc_tx_counter,
    src_mac, dst_dev);
  if (rc) {
    i5TraceError("Encryption Failure rc[%d]\n", rc);
    free(pmem);
    return NULL;
  }

  /* Length of AES-SIV encrypted data.
   * It will be encrypted data length + 16 bytes of SIV
  */
  *((unsigned short *)pbuf) = htons((unsigned short)(data_len + SIV_BLOCK_SZ));
  pbuf += 2;

  /* SIV is already at this point. just increment the pointer */
  pbuf += SIV_BLOCK_SZ;

  /* Encrypted data is already at this point. Just increment the ponter.*/
  pbuf += data_len;

  /* Encrypted Payload TLV is in pbuf now. Create new message with same
   * destination, message_type and message id and insert this TLV.
   * This messge will be freed by i5MessageSend()
  */
  enc_pmsg = i5MessageCreate(pmsg->psock, dst_dev->DeviceId, I5_PROTO);
  if (enc_pmsg == NULL) {
    free(pmem);
    return NULL;
  }

  i5PacketHeaderInit(enc_pmsg->ppkt, i5MessageTypeGet(pmsg), i5MessageIdentifierGet(pmsg));

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvEncryptedPayloadType;
  ptlv->length = htons(pbuf-pmem-sizeof(i5_tlv_t));
  i5MessageInsertTlv(enc_pmsg, pmem, pbuf - pmem);
  free(pmem);
  /* Now insert end of message type TLV */
  i5TlvEndOfMessageTypeInsert(enc_pmsg);

  /* Increment the encryption tx counter */
  dst_dev->enc_tx_counter.val++;

  return enc_pmsg;
}

/* Extract Encrypted pyload TLV */
int i5TlvEncryptedPayloadTypeExtract(i5_message_type *pmsg)
{
  i5_dm_device_type *src_dev = pmsg->pDevice;
  unsigned char *pvalue, *pmem;
  unsigned char *dst_mac, *cmdu, *data, *siv;
  i5_packet_type *ppkt = (i5_packet_type *)pmsg->ppkt;
  int rc = 0, data_len;
  unsigned int length, pmem_len;
  i5_uint48_t enc_rx_counter;
  uint64 tmp_counter;
  i5_tlv_t *ptlv;

  /* pmsg here has encrypted payload TLV.
   * Extract it in pmsg only and add End of message TLV at the end.
  */

  i5Trace("\n");

  /* Check for Source Device */
  if (src_dev == NULL) {
    i5TraceError("Source device not present\n");
    return -1;
  }

  /* Check for PTK */
  if (!I5_DEV_IS_PTK_PRESENT(src_dev)) {
    i5TraceError("Source device is present. PTK[%p] PTK_LENGTH[%d]\n",
      src_dev->ptk, src_dev->ptk_len);
    return -1;
  }

  rc = i5MessageTlvExtract(pmsg, i5TlvEncryptedPayloadType, &length, &pvalue,
        i5MessageTlvExtractWithReset);
  if (rc != 0) {
    return rc;
  }

  /* Allocate buffer for decrypted payload data and EOM TLV */
  pmem_len = length - i5TlvEncryptedPayload_Lnegth - SIV_BLOCK_SZ + sizeof(i5_tlv_t);
  if ((pmem = (unsigned char *)malloc(pmem_len)) == NULL) {
    i5TraceError("malloc error\n");
    return -1;
  }

  bzero(pmem, pmem_len);
  enc_rx_counter.val = 0;
  cmdu = ppkt->pbuf + I5_MESSAGE_CMDU_OFFSET;
  memcpy(&(enc_rx_counter), pvalue, I5_SEC_COUNTER_LEN);
  pvalue += I5_SEC_COUNTER_LEN;
  /* Change the endianness of the counter */
  tmp_counter = (uint64)enc_rx_counter.val;
  tmp_counter = hton64(tmp_counter);
  tmp_counter = tmp_counter >> 16;

  pvalue += ETHER_ADDR_LEN;
  dst_mac = pvalue;
  pvalue += ETHER_ADDR_LEN;
  data_len = ntohs(*((unsigned short *)pvalue)) - SIV_BLOCK_SZ;
  pvalue += 2;
  siv = pvalue;
  pvalue += SIV_BLOCK_SZ;
  data = pmem;
  /* Copy the data in pbuf */
  memcpy(pmem, pvalue, data_len);

  /* Check if enc_tx_counter is greater than last received enc_tx_counter */
  if (tmp_counter <= src_dev->enc_rx_counter.val) {
    i5TraceError("enc_rx_counter[%llu] is greater than last received enc_rx_counter[%llu]\n",
      (uint64)enc_rx_counter.val, (uint64)src_dev->enc_rx_counter.val);
    free(pmem);
    return -1;
  }

  rc = ieee1905_decrypt_packet(data, data_len, siv, cmdu, (unsigned char *)&enc_rx_counter,
      dst_mac, src_dev);
  if (rc) {
    i5TraceError("Decryption Failure rc[%d]\n", rc);
    free(pmem);
    src_dev->decrypt_fail_counter++;
    return rc;
  }

  /* Store Curr encryption rx counter values */
  src_dev->enc_rx_counter.val = tmp_counter;

  /* pmem has decrypted tlv's, store this in decrypted_tlvs in pmsg and
   * set the flag I5_FLAG_MSG_DECRYPTED. While extracting TLV's this will
   * be used instead of ppkt->pbuf.
  */
  pmsg->decrypted_data = pmem;
  pmsg->flags |= I5_FLAG_MSG_DECRYPTED;
  pmsg->decrypted_data_len = data_len;

  /* Now insert end of message type TLV */
  ptlv = (i5_tlv_t *)(pmem + data_len);
  ptlv->type = i5TlvEndOfMessageType;
  ptlv->length = htons(0);
  pmsg->decrypted_data_len += sizeof(i5_tlv_t);

  return rc;
}

/* Insert Associated WiFi6 STA Status Report TLV */
int i5TlvAssociatedWiFi6STAStatusReportTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  ieee1905_wifi6_sta_status *wifi6_sta_status)
{
  unsigned char *pmem, *pbuf;
  i5_tlv_t *ptlv;
  int rc = 0;
  unsigned int i;

  if ((i5_config.map_profile == ieee1905_map_profile0) ||
      ((i5_config.map_profile == ieee1905_map_profile2) &&
       (I5_IS_R2_CERT_COMPATIBLE(i5_config.flags)))) {
    i5Trace("Our MAP Profile is %d. Not inserting AssociatedWiFi6STAStatusReportType TLV\n",
      i5_config.map_profile);
    return 0;
  }

  if (!wifi6_sta_status->number_of_tid) {
    i5TraceInfo("empty TID, Queue Size pairs\n");
  }

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  i5TraceInfo("\n");
  pbuf = pmem + sizeof(i5_tlv_t); // Header filled at the end

  memcpy(pbuf, mac, MAC_ADDR_LEN);
  pbuf += MAC_ADDR_LEN;

  *pbuf = wifi6_sta_status->number_of_tid;
  pbuf++;

  for (i = 0; i < wifi6_sta_status->number_of_tid; i++) {
    *pbuf = wifi6_sta_status->tid[i];
    pbuf++;
    *pbuf = wifi6_sta_status->queue_size[i];
    pbuf++;
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvAssociatedWiFi6STAStatusReportType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));

  rc = i5MessageInsertTlv(pmsg, pmem, pbuf-pmem);

  free(pmem);
  return (rc);
}

/* Extract Associated WiFi6 STA Status Report TLV */
int i5TlvAssociatedWiFi6STAStatusReportTypeExtract(i5_message_type *pmsg)
{
  i5_dm_device_type *pdevice = pmsg->pDevice;
  int rc = 0;
  unsigned int length;
  unsigned char *pvalue, mac[MAC_ADDR_LEN];
  i5_dm_clients_type *pdmclient;

  if (pdevice == NULL) {
    i5TraceError("Neighbour device does not exist\n");
    rc = -1;
    goto end;
  }

  i5MessageReset(pmsg);
  while ((rc = i5MessageTlvExtract(pmsg, i5TlvAssociatedWiFi6STAStatusReportType, &length, &pvalue,
    i5MessageTlvExtractWithoutReset)) == 0) {
    int pos = 0;
    unsigned char n, i;

    if (length < i5TlvAssociatedWiFi6STAStatusReport_Min_Length) {
      i5TraceError("length is too small %u\n", length);
      continue;
    }

    memcpy(mac, &pvalue[pos], MAC_ADDR_LEN);
    pos += MAC_ADDR_LEN;

    n = pvalue[pos];
    pos++;
    if (!n) {
      i5TraceInfo("empty status report\n");
      continue;
    }

    if (length != (pos + n * IEEE1905_TID_QUEUE_SIZE_PAIR)) {
      i5TraceError("length(%u) doesn't match with n(%u)\n", length, n);
      continue;
    }

    pdmclient = i5DmFindClientInDevice(pdevice, mac);
    if (!pdmclient) {
      i5TraceError("STA " I5_MAC_DELIM_FMT " does not exist in device " I5_MAC_DELIM_FMT "\n",
        I5_MAC_PRM(mac), I5_MAC_PRM(pdevice->DeviceId));
      continue;
    }

    pdmclient->wifi6_sta_status.number_of_tid = n;
    i5Trace("n %u\n", pdmclient->wifi6_sta_status.number_of_tid);
    for (i = 0; i < n; i++) {
        pdmclient->wifi6_sta_status.tid[i] = pvalue[pos];
        pos++;
        pdmclient->wifi6_sta_status.queue_size[i] = pvalue[pos];
        pos++;
        i5Trace("TID %u Queue Size 0x%x\n",
          pdmclient->wifi6_sta_status.tid[i], pdmclient->wifi6_sta_status.queue_size[i]);
    }
  }
end:
  return rc;
}

/* TLV to report the 1905 Layer Security Capability */
int i5Tlv1905LayerSecurityCapabilityTypeInsert(i5_message_type *pmsg)
{
  unsigned char buf[sizeof(i5_tlv_t) + i5Tlv1905LayerSecurityCapability_length];
  i5_tlv_t *ptlv = (i5_tlv_t *)buf;
  int len = 0;
  i5_dm_device_type *pSelfDevice = i5DmGetSelfDevice();

  if ((i5_config.map_profile == ieee1905_map_profile0) ||
      (i5_config.map_profile == ieee1905_map_profile2)) {
    i5Trace("Our MAP Profile is %d. Not inserting i5Tlv1905LayerSecurityCapabilityType TLV\n",
      i5_config.map_profile);
    return 0;
  }

  if (!I5_IS_P2_AP_CAP_DPP_ONBOARD(pSelfDevice->p2ApCap.flags)) {
    i5Trace("DPP Onboarding is not supported. Not inserting i5Tlv1905LayerSecurityCapabilityType "
      "TLV\n");
    return 0;
  }

  ptlv->type = i5Tlv1905LayerSecurityCapabilityType;
  ptlv->length = htons(i5Tlv1905LayerSecurityCapability_length);
  len += sizeof(i5_tlv_t);

  buf[len++] = (unsigned char)MAP_ONBOARDING_PROTO_DPP;
  buf[len++] = (unsigned char)MAP_MSG_INTEGRITY_HMAC_SHA256;
  buf[len++] = (unsigned char)MAP_ENCR_ALGO_AES_SIV;

  return (i5MessageInsertTlv(pmsg, buf, len));
}

/* Extract 1905 Layer Security Capability TLV */
int i5Tlv1905LayerSecurityCapabilityTypeExtract(i5_message_type *pmsg)
{
  i5_dm_device_type *pdevice = pmsg->pDevice;
  unsigned char *pvalue;
  unsigned int length;

  if (pdevice == NULL) {
    i5TraceError("Neighbour device does not exist\n");
    goto end;
  }

  if (i5MessageTlvExtract(pmsg, i5Tlv1905LayerSecurityCapabilityType, &length, &pvalue,
    i5MessageTlvExtractWithReset) != 0) {
    goto end;
  }

  if (length < i5Tlv1905LayerSecurityCapability_length) {
    i5TraceError("Invalid Length[%d] for 1905 Layer Security Capability from Neighbour "
      "device["I5_MAC_DELIM_FMT"]\n", length, I5_MAC_PRM(pdevice->DeviceId));
    goto end;
  }

  /* If DPP onboarding is supported. Set the P2P AP cap value */
  if (*pvalue == MAP_ONBOARDING_PROTO_DPP) {
    pdevice->p2ApCap.flags |= I5_P2_AP_CAP_DPP_ONBOARD;
  }
  pvalue++;

  return 0;

end:
  return -1;
}

/* TLV to Insert the MIC */
int i5TlvMICTypeInsert(i5_message_type *pmsg)
{
  unsigned char *pmem, *data, *pbuf, digest[SHA256_DIGEST_LENGTH] = {0};
  unsigned char *src_mac;
  i5_tlv_t *ptlv;
  int rc = 0;
  i5_packet_type *ppkt = (i5_packet_type *)pmsg->packet_list.ll.next;
  unsigned int data_len = 0, pmem_len = 0;
  unsigned int pkt_data_len;
  unsigned char gtkid;
  i5_uint48_t mic_tx_counter;
  uint64 tmp_counter;

  i5Trace("\n");

  if (ppkt == NULL) {
    i5TraceError("ppkt is NULL \n");
    return -1;
  }
  /* Calculate data length */
  pmem_len += I5_SEC_CMDU_LEN;	/* First six bytes of cmdu */
  pmem_len += i5TlvMicValue;	/* 13 bytes of MIC TLV */
  do {
    pmem_len += (ppkt->length - I5_MESSAGE_TLV_OFFSET);
    ppkt = (i5_packet_type *)ppkt->ll.next;
  } while (ppkt != NULL);

  if ((pmem = (unsigned char *)malloc(pmem_len)) == NULL) {
    i5TraceError("malloc error\n");
    return -1;
  }

  data = pmem;

  /* point ppkt to first pkt in pmsg */
  ppkt = (i5_packet_type *)pmsg->packet_list.ll.next;

  /* Copy first 6 bytes of cmdu */
  memcpy(data + data_len, ppkt->pbuf + I5_MESSAGE_CMDU_OFFSET, I5_SEC_CMDU_LEN);
  data_len += I5_SEC_CMDU_LEN;

  /* Copy gtk key id */
  gtkid = 1;
  memcpy(data + data_len, &gtkid, sizeof(gtkid));
  data_len += sizeof(gtkid);

  /* Change the endianness of the counter */
  tmp_counter = (uint64)i5_dm_network_topology.mic_tx_counter.val;
  tmp_counter = hton64(tmp_counter);
  mic_tx_counter.val = tmp_counter >> 16;

  /* Copy Integrity transmission counter */
  memcpy(data + data_len, &(mic_tx_counter), I5_SEC_COUNTER_LEN);
  data_len += I5_SEC_COUNTER_LEN;

  /* Copy Src MAC */
  src_mac = i5MessageSrcMacAddressGet(pmsg);
  memcpy(data + data_len, src_mac, ETHER_ADDR_LEN);
  data_len += ETHER_ADDR_LEN;

  /* Copy the data */
  do {
    pkt_data_len = ppkt->length - I5_MESSAGE_TLV_OFFSET;
    memcpy(data + data_len, ppkt->pbuf + I5_MESSAGE_TLV_OFFSET, pkt_data_len);
    /* Update the number of bytes read */
    data_len += pkt_data_len;
    /* Update number of bytes still remaining */
    ppkt = (i5_packet_type *)ppkt->ll.next;
  } while (ppkt != NULL);

  /* Calculate digest */
  hmac_sha256(i5_dm_network_topology.gtk, i5_dm_network_topology.gtk_len, data, data_len,
      digest, NULL);

  free(pmem);

  /* Insert MIC type TLV */
  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceError("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t); // Header filled at the end

  *pbuf = gtkid;
  pbuf++;

  /* 6 bytes Integrity transmission counter */
  memcpy(pbuf, &(mic_tx_counter), I5_SEC_COUNTER_LEN);
  pbuf += I5_SEC_COUNTER_LEN;

  /* src mac addr */
  memcpy(pbuf, src_mac, ETHER_ADDR_LEN);
  pbuf += ETHER_ADDR_LEN;

  /* MIC Length */
  *((unsigned short*)pbuf) = htons(SHA256_DIGEST_LENGTH);
  pbuf += 2;

  /* MIC */
  memcpy(pbuf, digest, SHA256_DIGEST_LENGTH);
  pbuf += SHA256_DIGEST_LENGTH;

  i5TraceInfo("GTKIdentifier[%d] integrity transmission counter[%llu] \n",
    gtkid, (uint64)i5_dm_network_topology.mic_tx_counter.val);

  /* Since MIC TLV has to be inserted befor EOM TLV, adjust the packet length */
  pmsg->ppkt->length -= sizeof(i5_tlv_t);
  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvMICType;
  ptlv->length = htons(pbuf-pmem-sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, (pbuf-pmem));

  /* Now Insert EOM type TLv */
  rc = i5TlvEndOfMessageTypeInsert(pmsg);
  pmsg->flags |= I5_FLAG_MIC_TLV_PRESENT;

  /* Increament Integrity Transmission Counter */
  i5_dm_network_topology.mic_tx_counter.val++;

  free(pmem);
  return (rc);
}

/* Extract MIC TLV */
int i5TlvMICTypeExtract(i5_message_type *pmsg, unsigned char *gtkid, i5_uint48_t *mic_rx_counter,
  unsigned char *src_mac, unsigned char *digest)
{
  unsigned char *pvalue;
  unsigned int length, pos = 0;
  int rc = 0;

  rc = i5MessageTlvExtract(pmsg, i5TlvMICType, &length, &pvalue, i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }

  if (length < i5TlvMicLength) {
    i5TraceError("Invalid Length[%d] for MIC from Neighbour device["I5_MAC_DELIM_FMT"]\n",
      length, I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg)));
    goto end;
  }

  *gtkid = pvalue[pos++];

  memcpy(mic_rx_counter, &pvalue[pos], I5_SEC_COUNTER_LEN);
  pos += I5_SEC_COUNTER_LEN;
  memcpy(src_mac, &pvalue[pos], ETHER_ADDR_LEN);
  pos += ETHER_ADDR_LEN;
  pos += 2;

  memcpy(digest, &pvalue[pos], SHA256_DIGEST_LENGTH);

  return 0;

end:
  return -1;
}

/* Generate the MIC and compare it with the recieved MIC */
static int i5MessageCompareMIC(i5_message_type *pmsg, unsigned char gtkid,
  i5_uint48_t *mic_rx_counter, unsigned char *src_mac, unsigned char *digest)
{
  unsigned char *pmem, *data, computed_digest[SHA256_DIGEST_LENGTH] = {0};
  i5_packet_type *ppkt = (i5_packet_type *)pmsg->packet_list.ll.next;
  unsigned int pkt_data_len, data_len = 0, pmem_len = 0, pmsg_bytes_rem = 0;
  i5_tlv_t *ptlv;

  i5Trace("\n");

  if (ppkt == NULL) {
    i5TraceError("ppkt is NULL \n");
    return -1;
  }
  /* Calculate data length */
  do {
    pmem_len += (ppkt->length - I5_MESSAGE_TLV_OFFSET);
    ppkt = (i5_packet_type *)ppkt->ll.next;
  } while (ppkt != NULL);

  /* From pmsg we dont need to copy MIC TLV */
  pmsg_bytes_rem = pmem_len;
  pmsg_bytes_rem -= sizeof(i5_tlv_t);
  pmsg_bytes_rem -= i5TlvMicLength;
  pmsg_bytes_rem -= sizeof(i5_tlv_t);

  pmem_len += I5_SEC_CMDU_LEN;	/* First six bytes of cmdu */
  pmem_len += i5TlvMicValue;	/* 13 bytes of MIC TLV */
  if ((pmem = (unsigned char *)malloc(pmem_len)) == NULL) {
    i5TraceError("malloc error\n");
    return -1;
  }

  data = pmem;

  /* point ppkt to first pkt in pmsg */
  ppkt = (i5_packet_type *)pmsg->packet_list.ll.next;

  /* Copy first 6 bytes of cmdu */
  memcpy(data + data_len, ppkt->pbuf + I5_MESSAGE_CMDU_OFFSET, I5_SEC_CMDU_LEN);
  data_len += I5_SEC_CMDU_LEN;

  /* Copy gtk key id */
  memcpy(data + data_len, &gtkid, sizeof(gtkid));
  data_len += sizeof(gtkid);

  /* Copy Integrity transmission counter */
  memcpy(data + data_len, mic_rx_counter, I5_SEC_COUNTER_LEN);
  data_len += I5_SEC_COUNTER_LEN;

  /* Copy Src MAC */
  memcpy(data + data_len, src_mac, ETHER_ADDR_LEN);
  data_len += ETHER_ADDR_LEN;

  /* Copy the data */
  while (ppkt != NULL && pmsg_bytes_rem) {
    pkt_data_len = ppkt->length - I5_MESSAGE_TLV_OFFSET;
    if (pkt_data_len > pmsg_bytes_rem) {
      pkt_data_len = pmsg_bytes_rem;
    }
    memcpy(data + data_len, ppkt->pbuf + I5_MESSAGE_TLV_OFFSET, pkt_data_len);
    /* Update the number of bytes read */
    data_len += pkt_data_len;
    /* Update number of bytes still remaining */
    pmsg_bytes_rem -= pkt_data_len;
    ppkt = (i5_packet_type *)ppkt->ll.next;
  }

  /* Now insert end of message type TLV */
  ptlv = (i5_tlv_t *)(data + data_len);
  ptlv->type = i5TlvEndOfMessageType;
  ptlv->length = htons(0);
  data_len += sizeof(i5_tlv_t);

  /* Compute MIC */
  hmac_sha256(i5_dm_network_topology.gtk, i5_dm_network_topology.gtk_len, data, data_len,
    computed_digest, NULL);

  free(pmem);
  return (memcmp(digest, computed_digest, SHA256_DIGEST_LENGTH));
}

/* Go through all the fragments in the message and check the MIC.
 * Returns 0 if all fine
 * -1 If MIC is not matching
 * -2 If Transmission counter not greater than reception counter
 */
int i5MessageCheckMIC(i5_message_type *pmsg)
{
  int ret = 0, rc;
  unsigned char src_mac[MAC_ADDR_LEN], digest[SHA256_DIGEST_LENGTH];
  unsigned char gtkid;
  i5_uint48_t mic_rx_counter;
  uint64 tmp_counter;

  /* If GIK present then only check the MIC */
  if (!i5_dm_network_topology.gtk || !i5_dm_network_topology.gtk_len) {
    i5TraceInfo("GIK not present to check the MIC on the message %04x on %s\n",
      i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
    goto end;
  }

  memset(src_mac, 0, MAC_ADDR_LEN);
  memset(digest, 0, SHA256_DIGEST_LENGTH);
  mic_rx_counter.val = 0;

  rc = i5TlvMICTypeExtract(pmsg, &gtkid, &mic_rx_counter, src_mac, digest);
  /* If MIC TLV is not present return */
  if (rc != 0) {
    i5TraceInfo("MIC TLV not present in the message %04x on %s from "
      "device["I5_MAC_DELIM_FMT"]\n",
      i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname,
      I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg)));
    goto end;
  }

  if (pmsg->pDevice == NULL) {
    i5TraceInfo("Device["I5_MAC_DELIM_FMT"] Not found for the message %04x on %s\n",
      I5_MAC_PRM(src_mac), i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname);
    goto end;
  }

  /* Change the endianness of the counter */
  tmp_counter = (uint64)mic_rx_counter.val;
  tmp_counter = hton64(tmp_counter);
  tmp_counter = tmp_counter >> 16;

  i5TraceInfo("GTKIdentifier[%d] integrity reception counter[%llu] \n",
    gtkid, tmp_counter);

  /* If the Integrity Transmission Counter value received in the message is not greater than the
   * integrity reception counter of the Multi-AP device, the receiver shall ignore and discard
   * the message
   */
  if (tmp_counter <= pmsg->pDevice->mic_rx_counter.val) {
    ret = -2;
    i5TraceError("Integrity Transmission Counter[%llu] is not greater than the "
      "integrity reception counter[%llu] in the message %04x on %s from "
      "device["I5_MAC_DELIM_FMT"]\n",
      tmp_counter, (uint64)pmsg->pDevice->mic_rx_counter.val,
      i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname,
      I5_MAC_PRM(i5MessageSrcMacAddressGet(pmsg)));
    goto end;
  }

  if (i5MessageCompareMIC(pmsg, gtkid, &mic_rx_counter, src_mac, digest) != 0) {
    ret = -1;
    i5TraceError("MIC not matching for message %04x on %s from device["I5_MAC_DELIM_FMT"]\n",
      i5MessageIdentifierGet(pmsg), pmsg->psock->u.sll.ifname, I5_MAC_PRM(src_mac));
    goto end;
  }

  /* Store the MIC rx counter value */
  pmsg->pDevice->mic_rx_counter.val = tmp_counter;

end:
  return ret;
}

/*Insert WiFi7 Agent Capabilities TLV */
int i5TlvWiFi7AgentCapabilitiesTypeInsert(i5_message_type *pmsg, i5_dm_device_type *pdevice)
{
  unsigned char *pbuf, *pmem, *nradios;
  i5_tlv_t *ptlv;
  int rc = 0;
  i5_dm_interface_type *pdmif;
  int i;

  if (i5_config.map_profile == ieee1905_map_profile0 ||
    i5_config.map_profile == ieee1905_map_profile2) {
    i5Trace("Our MAP Profile is %u. Not inserting i5TlvWiFi7AgentCapabilitiesType TLV(0x%X)\n",
      i5_config.map_profile, i5TlvWiFi7AgentCapabilitiesType);
    return 0;
  }

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    printf("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t); // Header filled at the end

  i5Trace("Wi-Fi 7 Agent capabilities of self Device[" I5_MAC_DELIM_FMT "] "
    "MaxNumMLDs %u MaxLinks [0x%x] Caps [0x%x]\n",
    I5_MAC_PRM(pdevice->DeviceId), pdevice->WiFi7AgentCaps.MaxNumMLDs,
    pdevice->WiFi7AgentCaps.MaxLinks, pdevice->WiFi7AgentCaps.Caps);
  *pbuf = pdevice->WiFi7AgentCaps.MaxNumMLDs;
  pbuf++;
  *pbuf = pdevice->WiFi7AgentCaps.MaxLinks;
  pbuf++;
  *pbuf = pdevice->WiFi7AgentCaps.Caps;
  pbuf++;

  /* 13 octets are reserved here */
  bzero(pbuf, i5TlvWiFi7AgentCapabilities_Res_Len);
  pbuf += i5TlvWiFi7AgentCapabilities_Res_Len;

  nradios = pbuf;
  *nradios = 0;
  pbuf++;
  foreach_i5glist_item(pdmif, i5_dm_interface_type, pdevice->interface_list) {
    ieee1905_wifi7_radio_caps_type *RadioMLDCaps = &pdmif->RadioMLDCaps;

    if (!I5_IS_IFR_WIRELESS(pdmif->flags)) {
      continue;
    }
    i5Trace("Wi-Fi 7 Agent capabilities of radio " I5_MAC_DELIM_FMT " APMLDModeFlags[0x%x] "
      "bSTAMLDModeFlags [0x%x] AP records [%u] bSTA records[%u] caps [0x%x]\n",
      I5_MAC_PRM(pdmif->InterfaceId), RadioMLDCaps->APMLDModeFlags, RadioMLDCaps->bSTAMLDModeFlags,
      RadioMLDCaps->NumAPSTRRecords, RadioMLDCaps->NumbSTASTRRecords,RadioMLDCaps->Caps);

    eacopy(pdmif->InterfaceId, pbuf);
    pbuf += MAC_ADDR_LEN;
    *pbuf = RadioMLDCaps->Caps;
    pbuf++;

    /* 23 octets are reserved here */
    bzero(pbuf, i5TlvWiFi7AgentCapabilities_Radio_Res_Len);
    pbuf += i5TlvWiFi7AgentCapabilities_Radio_Res_Len;

    *pbuf = RadioMLDCaps->APMLDModeFlags;
    pbuf++;
    *pbuf = RadioMLDCaps->bSTAMLDModeFlags;
    pbuf++;
    *pbuf = RadioMLDCaps->NumAPSTRRecords;
    pbuf++;
    for(i = 0; i < RadioMLDCaps->NumAPSTRRecords; i++) {
      mld_records_type *APSTRRecords = &RadioMLDCaps->APSTRRecords[i];

      i5Trace("APSTRRecord[%d] mac[" I5_MAC_DELIM_FMT "] freqsep [%u]\n", i + 1,
        I5_MAC_PRM(APSTRRecords->mac), APSTRRecords->FreqSep);
      eacopy(APSTRRecords->mac, pbuf);
      pbuf += MAC_ADDR_LEN;
      *pbuf = APSTRRecords->FreqSep;
      pbuf++;
    }
    *pbuf = 0; /* Num_AP_EMLSR_Records */
    pbuf++;
    *pbuf = 0; /* Num_AP_EMLMR_Records */
    pbuf++;
    *pbuf = RadioMLDCaps->NumbSTASTRRecords;
    pbuf++;
    for(i = 0; i < RadioMLDCaps->NumbSTASTRRecords; i++) {
      mld_records_type *bSTASTRRecords= &RadioMLDCaps->bSTASTRRecords[i];

      i5Trace("bSTASTRRecord[%d] mac[" I5_MAC_DELIM_FMT "] freqsep [%u]\n", i + 1,
        I5_MAC_PRM(bSTASTRRecords->mac), bSTASTRRecords->FreqSep);
      eacopy(bSTASTRRecords->mac, pbuf);
      pbuf += MAC_ADDR_LEN;
      *pbuf = bSTASTRRecords->FreqSep;
      pbuf++;
    }
    *pbuf = 0; /* Num_bSTA_EMLSR_Records */
    pbuf++;
    *pbuf = 0; /* Num_bSTA_EMLMR_Records */
    pbuf++;
    (*nradios)++;
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvWiFi7AgentCapabilitiesType;
  ptlv->length = htons(pbuf-pmem-sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf-pmem);

  free(pmem);
  return (rc);
}

/* Extract WiFi7 Agent Capabilities TLV */
int i5TlvWiFi7AgentCapabilitiesTypeExtract(i5_message_type *pmsg, i5_dm_device_type *pdevice)
{
  i5_dm_interface_type *pdmif;
  unsigned char *pvalue, mac[MAC_ADDR_LEN];
  unsigned int length, minlen, radio_cap_min_len, extracted_len;
  int rc = 0;
  ieee1905_wifi7_agent_caps_type *WiFi7AgentCaps;
  int nradios, i, j;

 extracted_len = minlen = i5TlvWiFi7AgentCapabilities_Min_Length;
  i5MessageReset(pmsg);
  rc = i5MessageTlvExtract(pmsg, i5TlvWiFi7AgentCapabilitiesType,
    &length, &pvalue, i5MessageTlvExtractWithoutReset);
  if (length < minlen) {
    i5TraceError("Length %u is less than Minimum required length %u\n", length, minlen);
    rc = -1;
    goto end;
  }

  WiFi7AgentCaps = &pdevice->WiFi7AgentCaps;
  bzero(WiFi7AgentCaps, sizeof(*WiFi7AgentCaps));

  WiFi7AgentCaps->MaxNumMLDs = *pvalue;
  pvalue++;
  WiFi7AgentCaps->MaxLinks = *pvalue;
  pvalue++;
  WiFi7AgentCaps->Caps = *pvalue;
  pvalue++;
  /* 13 octets are reserved here */
  bzero(pvalue, i5TlvWiFi7AgentCapabilities_Res_Len);
  pvalue += i5TlvWiFi7AgentCapabilities_Res_Len;
  nradios = *pvalue;
  pvalue++;

  i5Trace("Wi-Fi 7 Agent capabilities of Device[" I5_MAC_DELIM_FMT "] "
    "MaxNumMLDs %u MaxLinks [0x%x] Caps [0x%x] radios[%d]\n",
    I5_MAC_PRM(pdevice->DeviceId), WiFi7AgentCaps->MaxNumMLDs,
    WiFi7AgentCaps->MaxLinks, WiFi7AgentCaps->Caps, nradios);

  radio_cap_min_len = MAC_ADDR_LEN + i5TlvWiFi7AgentCapabilities_Radio_Res_Len + 9;
  minlen += nradios * radio_cap_min_len;
  if (length < minlen) {
    i5TraceError("Length %u is less than Minimum required length %u. extraced len %u\n",
      length, minlen, extracted_len);
    goto end;
  }

  for (i = 0; i < nradios; i++) {
    ieee1905_wifi7_radio_caps_type *RadioMLDCaps;

    bzero(mac, MAC_ADDR_LEN);
    eacopy(pvalue, mac);

    pvalue += MAC_ADDR_LEN;
    pdmif = i5DmInterfaceFind(pdevice, mac);
    if (pdmif == NULL) {
      i5TraceError("Interface " I5_MAC_DELIM_FMT " does not exist\n", I5_MAC_PRM(mac));
      rc = -1;
      goto end;
    }

    pdmif->flags |= I5_FLAG_IFR_WIRELESS;
    pdmif->flags |= I5_FLAG_IFR_MAIN_WIRELESS;

    RadioMLDCaps = &pdmif->RadioMLDCaps;
    bzero(RadioMLDCaps, sizeof(*RadioMLDCaps));

    RadioMLDCaps->Caps = *pvalue;
    pvalue++;

    /* 23 octets are reserved here */
    bzero(pvalue, i5TlvWiFi7AgentCapabilities_Radio_Res_Len);
    pvalue += i5TlvWiFi7AgentCapabilities_Radio_Res_Len;

    RadioMLDCaps->APMLDModeFlags = *pvalue;
    pvalue++;
    RadioMLDCaps->bSTAMLDModeFlags = *pvalue;
    pvalue++;
    RadioMLDCaps->NumAPSTRRecords = *pvalue;
    pvalue++;

    i5Trace("Wi-Fi 7 Agent capabilities of radio["I5_MAC_DELIM_FMT"] "
      "Caps [0x%x] AP MLDmode[0x%x] Num of AP STR records[%u]\n", I5_MAC_PRM(mac),
      RadioMLDCaps->Caps, RadioMLDCaps->APMLDModeFlags, RadioMLDCaps->NumAPSTRRecords);

    extracted_len += radio_cap_min_len;
    minlen += RadioMLDCaps->NumAPSTRRecords * sizeof(mld_records_type);
    if (length < minlen) {
      i5TraceError("Length %u is less than Minimum required length %u. extraced len %u"
        "nradios [%d] current radio [%d]\n", length, minlen, extracted_len, nradios, (i + 1));
      goto end;
    }

    for (j = 0; j < RadioMLDCaps->NumAPSTRRecords; j++) {
      mld_records_type *APSTRRecords = &RadioMLDCaps->APSTRRecords[j];

      eacopy(pvalue, APSTRRecords->mac);
      pvalue += MAC_ADDR_LEN;
      APSTRRecords->FreqSep = *pvalue;
      pvalue++;
      i5Trace("APSTRRecord[%d]: mac ["I5_MAC_DELIM_FMT"] freqsep [%u]\n",
        j+1, I5_MAC_PRM(APSTRRecords->mac), APSTRRecords->FreqSep);
    }

    pvalue++; /* Num of AP EMLSR records */
    pvalue++; /* Num of AP EMLMR records */
    RadioMLDCaps->NumbSTASTRRecords = *pvalue;
    pvalue++;
    i5Trace("bSTA MLDmode[0x%x] Num of bSTA STR records[%u]\n",
      RadioMLDCaps->bSTAMLDModeFlags, RadioMLDCaps->NumbSTASTRRecords);

    extracted_len += RadioMLDCaps->NumAPSTRRecords * sizeof(mld_records_type);
    minlen += RadioMLDCaps->NumbSTASTRRecords * sizeof(mld_records_type);
    if (length < minlen) {
      i5TraceError("Length %u is less than Minimum required length %u. extraced len %u"
        "nradios [%d] current radio [%d]\n", length, minlen, extracted_len, nradios, (i + 1));
      goto end;
    }
    for (j = 0; j < RadioMLDCaps->NumbSTASTRRecords; j++) {
      mld_records_type *bSTASTRRecords = &RadioMLDCaps->bSTASTRRecords[j];

      eacopy(pvalue, bSTASTRRecords->mac);
      pvalue += MAC_ADDR_LEN;
      bSTASTRRecords->FreqSep = *pvalue;
      pvalue++;
      i5Trace("bSTASTRRecord[%d]: mac ["I5_MAC_DELIM_FMT"] freqsep [%u]\n",
        j+1, I5_MAC_PRM(bSTASTRRecords->mac), bSTASTRRecords->FreqSep);
    }
    pvalue++; /* Num of bSTA EMLSR records */
    pvalue++; /* Num of bSTA EMLMR records */
  }

end:
  return rc;
}

/* TLV to Insert Agent AP MLD Configuration */
int i5TlvAgentAPMLDConfigurationInsert(i5_message_type *pmsg, ieee1905_glist_t *agent_ap_mld_conf)
{
  unsigned char *pbuf, *pmem, *tmpPtrLoc, num_ap_mlds = 0;
  i5_tlv_t *ptlv;
  int rc = 0;
  i5_agent_ap_mld_conf_t *agent_ap_mld;
  i5_ap_mld_affliated_ap_conf_t *affliated_ap;

  i5Trace("\n");

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  /* Number of AP MLDs for which configuration information is included for this Agent. */
  tmpPtrLoc = pbuf; /* Fill this count at the end */
  pbuf++;

  foreach_iglist_item(agent_ap_mld, i5_agent_ap_mld_conf_t, *agent_ap_mld_conf) {

    /* Skip sending Pseudo MLO Configurations as AP MLDs */
    if (agent_ap_mld->affliated_aps.count < 2) {
      i5TraceInfo("Skip sending Pseudo MLO Configuration as AP MLD : SSID Len %u SSID %s "
        "AP MLD MAC["I5_MAC_DELIM_FMT"] MLD Mode 0x%x Number of Affiliated APs %u\n",
	agent_ap_mld->ssid.SSID_len, agent_ap_mld->ssid.SSID,
        I5_MAC_PRM(agent_ap_mld->ap_mld_mac), agent_ap_mld->mld_mode,
        agent_ap_mld->affliated_aps.count);
      continue;
    }
    num_ap_mlds++;

    /* Indicates whether the AP_MLD_MAC_Addr field is valid */
    *pbuf = agent_ap_mld->ap_mld_flag;
    pbuf++;

    /* Length of the SSID field. */
    *pbuf = (unsigned char)agent_ap_mld->ssid.SSID_len;
    pbuf++;

    /* SSID of the APs Affiliated to the MLD */
    memcpy(pbuf, agent_ap_mld->ssid.SSID, agent_ap_mld->ssid.SSID_len);
    pbuf += agent_ap_mld->ssid.SSID_len;

    /* AP MLD MAC Address */
    eacopy(agent_ap_mld->ap_mld_mac, pbuf);
    pbuf += MAC_ADDR_LEN;

    /* MLD Mode */
    *pbuf = agent_ap_mld->mld_mode;
    pbuf++;

    /* 20 octets are reserved here */
    bzero(pbuf, i5TlvAgentAPMLDConfiguration_Res_Len);
    pbuf += i5TlvAgentAPMLDConfiguration_Res_Len;

    /* Number of Affiliated APs belonging to this MLD */
    *pbuf = (unsigned char)agent_ap_mld->affliated_aps.count;
    pbuf++;
    i5TraceInfo("AP MLD Flag 0x%x SSID Len %u SSID %s AP MLD MAC["I5_MAC_DELIM_FMT"] "
      "MLD Mode 0x%x Number of Affiliated APs %u\n",
      agent_ap_mld->ap_mld_flag, agent_ap_mld->ssid.SSID_len, agent_ap_mld->ssid.SSID,
      I5_MAC_PRM(agent_ap_mld->ap_mld_mac), agent_ap_mld->mld_mode,
      agent_ap_mld->affliated_aps.count);

    foreach_iglist_item(affliated_ap, i5_ap_mld_affliated_ap_conf_t,
      agent_ap_mld->affliated_aps) {

      /* Indicates whether the Affiliated_AP_MAC_Addr field is valid */
      *pbuf = affliated_ap->ap_flag;
      pbuf++;

      /* RUID of radio on which the Affiliated AP operates */
      eacopy(affliated_ap->ruid, pbuf);
      pbuf += MAC_ADDR_LEN;

      /* MAC Address of Affiliated  AP of the MLD */
      eacopy(affliated_ap->ap_mac, pbuf);
      pbuf += MAC_ADDR_LEN;

      /* The LinkID of this link */
      *pbuf = affliated_ap->link_id;
      pbuf++;

      /* 18 octets are reserved here */
      bzero(pbuf, i5TlvAgentAPMLDAffliatedAP_Res_Len);
      pbuf += i5TlvAgentAPMLDAffliatedAP_Res_Len;

      i5TraceInfo("Affliated AP Flag 0x%x RUID["I5_MAC_DELIM_FMT"] AP MAC["I5_MAC_DELIM_FMT"] "
        "Link ID %u\n",
        affliated_ap->ap_flag, I5_MAC_PRM(affliated_ap->ruid),
        I5_MAC_PRM(affliated_ap->ap_mac), affliated_ap->link_id);
    }
  }

  /* Now fill the Number of AP MLDs */
  *tmpPtrLoc = (unsigned char)num_ap_mlds;
  i5TraceInfo("Number of AP MLDs %u\n", num_ap_mlds);

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvAgentAPMLDConfigurationValueType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);
  return rc;
}

/* Extract Agent AP MLD Configuration TLV */
int i5TlvAgentAPMLDConfigurationExtract(i5_message_type *pmsg, ieee1905_glist_t *agent_ap_mld_conf)
{
  int rc;
  unsigned char num_ap_mlds, num_affiliated_aps, i, j;
  unsigned char *pvalue;
  unsigned int length, extracted_len = i5TlvAgentAPMLDConfiguration_Min_Length, per_ap_mld_conf_len;
  i5_agent_ap_mld_conf_t *agent_ap_mld = NULL;
  i5_ap_mld_affliated_ap_conf_t *affliated_ap;

  i5Trace("\n");

  rc = i5MessageTlvExtract(pmsg, i5TlvAgentAPMLDConfigurationValueType, &length, &pvalue,
    i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }

  if (length < i5TlvAgentAPMLDConfiguration_Min_Length) {
    i5TraceError("Length %u is less than Minimum required length %u\n",
      length, i5TlvAgentAPMLDConfiguration_Min_Length);
    goto end;
  }

  /* Number of AP MLDs for which configuration information is included for this Agent. */
  num_ap_mlds = *pvalue;
  pvalue++;
  i5TraceInfo("Number of AP MLDs %u\n", num_ap_mlds);

  for (i = 0; i < num_ap_mlds; i++) {
    agent_ap_mld = (i5_agent_ap_mld_conf_t*)calloc(1, sizeof(*agent_ap_mld));
    if (!agent_ap_mld) {
      i5TraceDirPrint("Malloc Failed for Agent AP MLD Configuration\n");
      goto end;
    }

    ieee1905_glist_init(&agent_ap_mld->affliated_aps);

    if (length < (extracted_len + 2)) {
      i5TraceError("Length %u is less than Minimum required length %u. extracted_len %u\n",
        length, (extracted_len + 2), extracted_len);
      goto end;
    }

    /* Indicates whether the AP_MLD_MAC_Addr field is valid */
    agent_ap_mld->ap_mld_flag = *pvalue;
    pvalue++;

    /* Length of the SSID field. */
    agent_ap_mld->ssid.SSID_len = *pvalue;
    pvalue++;
    extracted_len += 2;

    per_ap_mld_conf_len = (agent_ap_mld->ssid.SSID_len + MAC_ADDR_LEN +
      i5TlvAgentAPMLDConfiguration_Res_Len + 2);
    if (length < (extracted_len + per_ap_mld_conf_len)) {
      i5TraceError("Length %u is less than Minimum required length %u. extracted_len %u\n",
        length, (extracted_len + per_ap_mld_conf_len), extracted_len);
      goto end;
    }

    /* SSID of the APs Affiliated to the MLD */
    memcpy(agent_ap_mld->ssid.SSID, pvalue, agent_ap_mld->ssid.SSID_len);
    pvalue += agent_ap_mld->ssid.SSID_len;

    /* AP MLD MAC Address. This field is valid only if AP_MLD_MAC_Addr_Valid=1 */
    if (agent_ap_mld->ap_mld_flag & I5_AP_MLD_FLAG_MAC_ADDR_VALID) {
      eacopy(pvalue, agent_ap_mld->ap_mld_mac);
    }
    pvalue += MAC_ADDR_LEN;

    /* MLD Mode */
    agent_ap_mld->mld_mode = *pvalue;
    pvalue++;

    /* 20 octets are reserved here */
    bzero(pvalue, i5TlvAgentAPMLDConfiguration_Res_Len);
    pvalue += i5TlvAgentAPMLDConfiguration_Res_Len;

    /* Number of Affiliated APs belonging to this MLD */
    num_affiliated_aps = *pvalue;
    pvalue++;
    extracted_len += per_ap_mld_conf_len;

    if (length < (extracted_len + (num_affiliated_aps * i5TlvAgentAPMLDAffliatedAP_Length))) {
      i5TraceError("Length %u is less than Minimum required length %u. extracted_len %u\n",
        length, (extracted_len + (num_affiliated_aps * i5TlvAgentAPMLDAffliatedAP_Length)),
        extracted_len);
      goto end;
    }
    i5TraceInfo("AP MLD Flag 0x%x SSID Len %u SSID %s AP MLD MAC["I5_MAC_DELIM_FMT"] "
      "MLD Mode 0x%x Number of Affiliated APs %u\n",
      agent_ap_mld->ap_mld_flag, agent_ap_mld->ssid.SSID_len, agent_ap_mld->ssid.SSID,
      I5_MAC_PRM(agent_ap_mld->ap_mld_mac), agent_ap_mld->mld_mode, num_affiliated_aps);

    for (j = 0; j < num_affiliated_aps; j++) {
      affliated_ap = (i5_ap_mld_affliated_ap_conf_t*)calloc(1, sizeof(*affliated_ap));
      if (!affliated_ap) {
        i5TraceDirPrint("Malloc Failed for Affliated AP Configuration for a AP MLD\n");
        goto end;
      }

      /* Indicates whether the Affiliated_AP_MAC_Addr field is valid */
      affliated_ap->ap_flag = *pvalue;
      pvalue++;

      /* RUID of radio on which the Affiliated AP operates */
      eacopy(pvalue, affliated_ap->ruid);
      pvalue += MAC_ADDR_LEN;

      /* MAC Address of Affiliated  AP of the MLD. This field is valid only if
       * Affiliated_AP_MAC_Addr_Valid=1
       */
      if (affliated_ap->ap_flag & I5_AFFILIATED_AP_FLAG_MAC_ADDR_VALID) {
        eacopy(pvalue, affliated_ap->ap_mac);
      }
      pvalue += MAC_ADDR_LEN;

      /* The LinkID of this link */
      affliated_ap->link_id = *pvalue;
      pvalue++;

      /* 18 octets are reserved here */
      bzero(pvalue, i5TlvAgentAPMLDAffliatedAP_Res_Len);
      pvalue += i5TlvAgentAPMLDAffliatedAP_Res_Len;

      ieee1905_glist_append(&agent_ap_mld->affliated_aps, (dll_t*)affliated_ap);
      i5TraceInfo("Affliated AP Flag 0x%x RUID["I5_MAC_DELIM_FMT"] AP MAC["I5_MAC_DELIM_FMT"] "
        "Link ID %u\n",
        affliated_ap->ap_flag, I5_MAC_PRM(affliated_ap->ruid),
        I5_MAC_PRM(affliated_ap->ap_mac), affliated_ap->link_id);
    }
    extracted_len += (num_affiliated_aps * i5TlvAgentAPMLDAffliatedAP_Length);
    ieee1905_glist_append(agent_ap_mld_conf, (dll_t*)agent_ap_mld);
    agent_ap_mld = NULL;
  }

  return 0;

end:
  if (agent_ap_mld) {
    i5DmGlistCleanup(&agent_ap_mld->affliated_aps);
    free(agent_ap_mld);
  }
  i5DmAgentAPMLDConfigListFree(agent_ap_mld_conf);
  return -1;
}

/* TLV to Insert Backhaul STA MLD Configuration */
int i5TlvBackhaulSTAMLDConfigurationInsert(i5_message_type *pmsg, i5_bsta_mld_conf_t *bsta_mld_conf)
{
  unsigned char *pbuf, *pmem;
  i5_tlv_t *ptlv;
  int rc = 0;
  i5_bsta_mld_affliated_bsta_conf_t *affliated_bsta;

  i5Trace("\n");

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  /* Indicates whether the BSTA_MLD_MAC_Addr and AP_MLD_MAC_Addr field is valid */
  *pbuf = bsta_mld_conf->bsta_mld_flag;
  pbuf++;

  /* MAC Address of bSTA MLD */
  eacopy(bsta_mld_conf->bsta_mld_mac, pbuf);
  pbuf += MAC_ADDR_LEN;

  /* MAC Address of the AP MLD to which the bSTA MLD is associated */
  eacopy(bsta_mld_conf->ap_mld_mac, pbuf);
  pbuf += MAC_ADDR_LEN;

  /* MLD Mode */
  *pbuf = bsta_mld_conf->mld_mode;
  pbuf++;

  /* 17 octets are reserved here */
  bzero(pbuf, i5TlvBackhaulSTAMLDConfiguration_Res_Len);
  pbuf += i5TlvBackhaulSTAMLDConfiguration_Res_Len;

  /* Number of Affiliated bSTAs in the bSTA non-AP MLD */
  *pbuf = (unsigned char)bsta_mld_conf->affliated_bstas.count;
  pbuf++;
  i5TraceInfo("bSTA MLD Flag 0x%x  bSTA MLD MAC["I5_MAC_DELIM_FMT"] "
    "AP MLD MAC["I5_MAC_DELIM_FMT"] MLD Mode 0x%x Number of Affiliated bSTAs %u\n",
    bsta_mld_conf->bsta_mld_flag, I5_MAC_PRM(bsta_mld_conf->bsta_mld_mac),
    I5_MAC_PRM(bsta_mld_conf->ap_mld_mac), bsta_mld_conf->mld_mode,
    bsta_mld_conf->affliated_bstas.count);

  foreach_iglist_item(affliated_bsta, i5_bsta_mld_affliated_bsta_conf_t,
    bsta_mld_conf->affliated_bstas) {

    /* Indicates whether the Affiliated bSTA MAC Address field is valid */
    *pbuf = affliated_bsta->bsta_flag;
    pbuf++;

    /* RUID of radio on which Affiliated bSTA operates */
    eacopy(affliated_bsta->ruid, pbuf);
    pbuf += MAC_ADDR_LEN;

    /* MAC Address of Affiliated bSTA */
    eacopy(affliated_bsta->bsta_mac, pbuf);
    pbuf += MAC_ADDR_LEN;

    /* 19 octets are reserved here */
    bzero(pbuf, i5TlvBackhaulSTAMLDAffliatedBSTA_Res_Len);
    pbuf += i5TlvBackhaulSTAMLDAffliatedBSTA_Res_Len;

    i5TraceInfo("Affliated bSTA Flag 0x%x RUID["I5_MAC_DELIM_FMT"] "
      "bSTA MAC["I5_MAC_DELIM_FMT"]\n",
      affliated_bsta->bsta_flag, I5_MAC_PRM(affliated_bsta->ruid),
      I5_MAC_PRM(affliated_bsta->bsta_mac));
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvBackhaulSTAMLDConfigurationValueType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);
  return rc;
}

/* Extract Backhaul STA MLD Configuration TLV */
int i5TlvBackhaulSTAMLDConfigurationExtract(i5_message_type *pmsg,
  i5_bsta_mld_conf_t *bsta_mld_conf)
{
  int rc;
  unsigned char num_affiliated_bstas, j;
  unsigned char *pvalue;
  unsigned int length, extracted_len = i5TlvBackhaulSTAMLDConfiguration_Min_Length;
  i5_bsta_mld_affliated_bsta_conf_t *affliated_bsta;

  i5Trace("\n");

  ieee1905_glist_init(&bsta_mld_conf->affliated_bstas);

  rc = i5MessageTlvExtract(pmsg, i5TlvBackhaulSTAMLDConfigurationValueType, &length, &pvalue,
    i5MessageTlvExtractWithReset);
  if (rc != 0) {
    goto end;
  }

  if (length < i5TlvBackhaulSTAMLDConfiguration_Min_Length) {
    i5TraceError("Length %u is less than Minimum required length %u\n",
      length, i5TlvBackhaulSTAMLDConfiguration_Min_Length);
    goto end;
  }

  /* Indicates whether the BSTA_MLD_MAC_Addr and AP_MLD_MAC_Addr field is valid */
  bsta_mld_conf->bsta_mld_flag = *pvalue;
  pvalue++;

  /* bSTA MLD MAC Address. This field is valid only if bSTA_MLD_MAC_Addr=1 */
  if (bsta_mld_conf->bsta_mld_flag & I5_BSTA_MLD_FLAG_BSTA_MAC_ADDR_VALID) {
    eacopy(pvalue, bsta_mld_conf->bsta_mld_mac);
  }
  pvalue += MAC_ADDR_LEN;

  /* AP MLD MAC Address. This field is valid only if AP_MLD_MAC_Addr_Valid=1 */
  if (bsta_mld_conf->bsta_mld_flag & I5_BSTA_MLD_FLAG_AP_MAC_ADDR_VALID) {
    eacopy(pvalue, bsta_mld_conf->ap_mld_mac);
  }
  pvalue += MAC_ADDR_LEN;

  /* MLD Mode */
  bsta_mld_conf->mld_mode = *pvalue;
  pvalue++;

  /* 17 octets reserved here */
  bzero(pvalue, i5TlvBackhaulSTAMLDConfiguration_Res_Len);
  pvalue += i5TlvBackhaulSTAMLDConfiguration_Res_Len;

  /* Number of Affiliated bSTAs in the bSTA non-AP MLD */
  num_affiliated_bstas = *pvalue;
  pvalue++;

  if (length < (extracted_len + (num_affiliated_bstas * i5TlvBackhaulSTAMLDAffliatedBSTA_Length))) {
    i5TraceError("Length %u is less than Minimum required length %u. extracted_len %u\n",
      length, (extracted_len + (num_affiliated_bstas * i5TlvBackhaulSTAMLDAffliatedBSTA_Length)),
      extracted_len);
    goto end;
  }
  i5TraceInfo("bSTA MLD Flag 0x%x  bSTA MLD MAC["I5_MAC_DELIM_FMT"] "
    "AP MLD MAC["I5_MAC_DELIM_FMT"] MLD Mode 0x%x Number of Affiliated bSTAs %u\n",
    bsta_mld_conf->bsta_mld_flag, I5_MAC_PRM(bsta_mld_conf->bsta_mld_mac),
    I5_MAC_PRM(bsta_mld_conf->ap_mld_mac), bsta_mld_conf->mld_mode, num_affiliated_bstas);

  for (j = 0; j < num_affiliated_bstas; j++) {
    affliated_bsta = (i5_bsta_mld_affliated_bsta_conf_t*)calloc(1, sizeof(*affliated_bsta));
    if (!affliated_bsta) {
      i5TraceDirPrint("Malloc Failed for Affliated bSTA Configuration for a bSTA MLD\n");
      i5DmGlistCleanup(&bsta_mld_conf->affliated_bstas);
      ieee1905_glist_init(&bsta_mld_conf->affliated_bstas);
      goto end;
    }

    /* Indicates whether the Affiliated bSTA MAC Address field is valid */
    affliated_bsta->bsta_flag = *pvalue;
    pvalue++;

    /* RUID of radio on which Affiliated bSTA operates */
    eacopy(pvalue, affliated_bsta->ruid);
    pvalue += MAC_ADDR_LEN;

    /* MAC Address of Affiliated bSTA of the bSTA MLD. This field is valid only if
     * Affiliated_bSTA_MAC_Addr_Valid=1
     */
    if (affliated_bsta->bsta_flag & I5_AFFILIATED_BSTA_FLAG_MAC_ADDR_VALID) {
      eacopy(pvalue, affliated_bsta->bsta_mac);
    }
    pvalue += MAC_ADDR_LEN;

    /* 19 octets reserved here */
    bzero(pvalue, i5TlvBackhaulSTAMLDAffliatedBSTA_Res_Len);
    pvalue += i5TlvBackhaulSTAMLDAffliatedBSTA_Res_Len;

    ieee1905_glist_append(&bsta_mld_conf->affliated_bstas, (dll_t*)affliated_bsta);
    i5TraceInfo("Affliated bSTA Flag 0x%x RUID["I5_MAC_DELIM_FMT"] "
      "bSTA MAC["I5_MAC_DELIM_FMT"]\n",
      affliated_bsta->bsta_flag, I5_MAC_PRM(affliated_bsta->ruid),
      I5_MAC_PRM(affliated_bsta->bsta_mac));
  }

  return 0;

end:
  return -1;
}

/* TLV to Insert Associated STA MLD Configuration */
int i5TlvAssociatedSTAMLDConfigurationInsert(i5_message_type *pmsg, i5_dm_clients_type *pdmclient)
{
  unsigned char *pbuf, *pmem;
  i5_tlv_t *ptlv;
  int rc = 0;
  i5_dm_bss_type *pdmbss;
  i5_sta_mld_conf_t *sta_mld_conf;
  i5_sta_mld_affiliated_sta_conf_t *affiliated_sta;

  i5Trace("\n");

  pdmbss = (i5_dm_bss_type*)I5LL_PARENT(pdmclient);
  sta_mld_conf = pdmclient->mld_conf;

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  pbuf = pmem + sizeof(i5_tlv_t);	// Header filled at the end

  /* MAC Address of STA MLD */
  eacopy(pdmclient->sta_mld, pbuf);
  pbuf += MAC_ADDR_LEN;

  /* MAC Address of the AP MLD to which the STA MLD is associated */
  eacopy(pdmbss->mld_addr, pbuf);
  pbuf += MAC_ADDR_LEN;

  /* MLD Mode */
  *pbuf = sta_mld_conf->mld_mode;
  pbuf++;

  /* 18 octets are reserved here */
  bzero(pbuf, i5TlvAssociatedSTAMLDConfiguration_Res_Len);
  pbuf += i5TlvAssociatedSTAMLDConfiguration_Res_Len;

  /* Number of Affiliated STAs in the STA non-AP MLD */
  *pbuf = (unsigned char)sta_mld_conf->affiliated_stas.count;
  pbuf++;
  i5TraceInfo("STA MLD MAC["I5_MAC_DELIM_FMT"] AP MLD MAC["I5_MAC_DELIM_FMT"] MLD Mode 0x%x "
    "Number of Affiliated STAs %u\n",
    I5_MAC_PRM(pdmclient->sta_mld), I5_MAC_PRM(pdmbss->mld_addr),
    sta_mld_conf->mld_mode, sta_mld_conf->affiliated_stas.count);

  foreach_iglist_item(affiliated_sta, i5_sta_mld_affiliated_sta_conf_t,
    sta_mld_conf->affiliated_stas) {

    /* BSSID on which Affiliated STA has setup a link  */
    eacopy(affiliated_sta->bssid, pbuf);
    pbuf += MAC_ADDR_LEN;

    /* MAC Address of Affiliated STA */
    eacopy(affiliated_sta->sta_mac, pbuf);
    pbuf += MAC_ADDR_LEN;

    /* 19 octets are reserved here */
    bzero(pbuf, i5TlvAssociatedSTAMLDAffliatedSTA_Res_Len);
    pbuf += i5TlvAssociatedSTAMLDAffliatedSTA_Res_Len;

    i5TraceInfo("Affliated STA BSSID["I5_MAC_DELIM_FMT"] STA MAC["I5_MAC_DELIM_FMT"]\n",
      I5_MAC_PRM(affiliated_sta->bssid), I5_MAC_PRM(affiliated_sta->sta_mac));
  }

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvAssociatedSTAMLDConfigurationValueType;
  ptlv->length = htons(pbuf - pmem - sizeof(i5_tlv_t));
  rc = i5MessageInsertTlv(pmsg, pmem, pbuf - pmem);

  free(pmem);
  return rc;
}

/* Extract an associated STA MLD configuration tlv data */
int i5TlvAssociatedSTAMLDConfigurationExtract(unsigned char *pvalue, unsigned int length,
  i5_dm_clients_type *pdmclient)
{
  unsigned char num_affiliated_stas, j;
  unsigned int extracted_len = i5TlvAssociatedSTAMLDConfiguration_Min_Length;
  i5_sta_mld_conf_t *sta_mld_conf;
  i5_sta_mld_affiliated_sta_conf_t *affiliated_sta;
  unsigned char ap_mld_mac[MAC_ADDR_LEN];

  i5Trace("\n");

  if (length < i5TlvAssociatedSTAMLDConfiguration_Min_Length) {
    i5TraceError("Length %u is less than Minimum required length %u\n",
      length, i5TlvAssociatedSTAMLDConfiguration_Min_Length);
    goto end;
  }

  sta_mld_conf = pdmclient->mld_conf;

  /* STA MLD MAC Address. */
  pvalue += MAC_ADDR_LEN;

  /* AP MLD MAC Address. */
  eacopy(pvalue, ap_mld_mac);
  pvalue += MAC_ADDR_LEN;

  /* MLD Mode */
  sta_mld_conf->mld_mode = *pvalue;
  pvalue++;

  /* 18 octets reserved here */
  bzero(pvalue, i5TlvAssociatedSTAMLDConfiguration_Res_Len);
  pvalue += i5TlvAssociatedSTAMLDConfiguration_Res_Len;

  /* Number of Affiliated bSTAs in the bSTA non-AP MLD */
  num_affiliated_stas = *pvalue;
  pvalue++;
  i5TraceInfo("STA MLD MAC["I5_MAC_DELIM_FMT"] AP MLD MAC["I5_MAC_DELIM_FMT"] MLD Mode 0x%x "
    "Number of Affiliated STAs %u\n",
    I5_MAC_PRM(pdmclient->sta_mld), I5_MAC_PRM(ap_mld_mac),
    sta_mld_conf->mld_mode, num_affiliated_stas);

  if (length < (extracted_len + (num_affiliated_stas * i5TlvAssociatedSTAMLDAffliatedSTA_Length))) {
    i5TraceError("Length %u is less than Minimum required length %u. extracted_len %u\n",
      length, (extracted_len + (num_affiliated_stas * i5TlvAssociatedSTAMLDAffliatedSTA_Length)),
      extracted_len);
    goto end;
  }

  for (j = 0; j < num_affiliated_stas; j++) {
    affiliated_sta = (i5_sta_mld_affiliated_sta_conf_t*)calloc(1, sizeof(*affiliated_sta));
    if (!affiliated_sta) {
      i5TraceDirPrint("Malloc Failed for Affliated STA Configuration for associated STA MLD\n");
      i5DmGlistCleanup(&sta_mld_conf->affiliated_stas);
      ieee1905_glist_init(&sta_mld_conf->affiliated_stas);
      goto end;
    }

    /* BSSID on which Affiliated STA has setup the link */
    eacopy(pvalue, affiliated_sta->bssid);
    pvalue += MAC_ADDR_LEN;

    /* MAC Address of Affiliated STA of the associated STA MLD. */
    eacopy(pvalue, affiliated_sta->sta_mac);
    pvalue += MAC_ADDR_LEN;

    /* 19 octets reserved here */
    bzero(pvalue, i5TlvAssociatedSTAMLDAffliatedSTA_Res_Len);
    pvalue += i5TlvAssociatedSTAMLDAffliatedSTA_Res_Len;
    i5TraceInfo("Affliated STA BSSID["I5_MAC_DELIM_FMT"] STA MAC["I5_MAC_DELIM_FMT"]\n",
      I5_MAC_PRM(affiliated_sta->bssid), I5_MAC_PRM(affiliated_sta->sta_mac));

    ieee1905_glist_append(&sta_mld_conf->affiliated_stas, (dll_t*)affiliated_sta);
  }

  return 0;

end:
  return -1;
}

/* TLV to add Affiliated STA Traffic Stats TLV */
int i5TlvAffiliatedSTAMetricsTypeInsert(i5_message_type *pmsg, unsigned char *mac,
  ieee1905_affiliated_sta_metric *affiliated_metric, i5_dm_device_type *pDestDevice)
{
  unsigned char *pbuf, *pmem, byte_cntr_unit = pDestDevice->byte_cntr_unit;
  i5_tlv_t *ptlv;
  int rc = 0;
  unsigned int bytes_sent, bytes_recv;

  if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
    i5TraceDirPrint("malloc error\n");
    return -1;
  }

  bzero(pmem, I5_PACKET_BUF_LEN);
  pbuf = pmem + sizeof(i5_tlv_t); // Header filled at the end

  eacopy(mac, pbuf);
  pbuf += MAC_ADDR_LEN;

  if (I5_P2APCAP_COUNTER_UNITS_GET(byte_cntr_unit) == I5_P2APCAP_COUNTER_UNITS_KBYTES) {
    bytes_sent = (unsigned int)(affiliated_metric->bytes_sent / 1024);
    bytes_recv = (unsigned int) (affiliated_metric->bytes_recv / 1024);
  } else if (I5_P2APCAP_COUNTER_UNITS_GET(byte_cntr_unit) == I5_P2APCAP_COUNTER_UNITS_MBYTES) {
    bytes_sent = (unsigned int) (affiliated_metric->bytes_sent / (1024 * 1024));
    bytes_recv = (unsigned int) (affiliated_metric->bytes_recv/ (1024 * 1024));
  } else {
    bytes_sent = (unsigned int)affiliated_metric->bytes_sent;
    bytes_recv = (unsigned int)affiliated_metric->bytes_recv;
  }

  *((unsigned int *)pbuf) = htonl(bytes_sent);
  pbuf += 4;

  *((unsigned int *)pbuf) = htonl(bytes_recv);
  pbuf += 4;

  *((unsigned int *)pbuf) = htonl(affiliated_metric->packets_sent);
  pbuf += 4;

  *((unsigned int *)pbuf) = htonl(affiliated_metric->packets_recv);
  pbuf += 4;

  *((unsigned int *)pbuf) = htonl(affiliated_metric->tx_packet_err);
  pbuf += 4;

  i5Trace("STA["I5_MAC_DELIM_FMT"] bytes_sent %u bytes_recv %u packets_sent %u "
    "packets_recv %u tx_packet_err %u\n",
    I5_MAC_PRM(mac), bytes_sent, bytes_recv, affiliated_metric->packets_sent,
    affiliated_metric->packets_recv, affiliated_metric->tx_packet_err);

  /* 998 octets are reserved here */
  bzero(pbuf, i5TlvAffiliatedSTAMetrics_Res_Len);
  pbuf += i5TlvAffiliatedSTAMetrics_Res_Len;

  ptlv = (i5_tlv_t *)pmem;
  ptlv->type = i5TlvAffiliatedSTAMetricsType;
  ptlv->length = htons(pbuf-pmem-sizeof(i5_tlv_t));

  rc = i5MessageInsertTlv(pmsg, pmem, pbuf-pmem);

  free(pmem);
  return (rc);
}

/* Extract Associated STA Traffic Stats TLV */
int i5TlvAffiliatedSTAMetricsTypeExtract(i5_message_type *pmsg)
{
  i5_dm_device_type *pdevice = pmsg->pDevice;
  i5_sta_mld_affiliated_sta_conf_t *affiliated_sta;
  unsigned char *pvalue, mac[MAC_ADDR_LEN];
  unsigned int length, found = 0;
  int rc = 0;

  if (pdevice == NULL) {
    i5TraceError("Neighbour device does not exist\n");
    rc = -1;
    goto end;
  }

  i5MessageReset(pmsg);
  while ((rc = i5MessageTlvExtract(pmsg, i5TlvAffiliatedSTAMetricsType, &length, &pvalue,
    i5MessageTlvExtractWithoutReset)) == 0) {
    int pos = 0;
    found = 1;
    if (length < i5TlvAffiliatedSTAMetrics_Min_Len) {
      i5TraceError("Minimum expected length is %u but TLV length is %u\n",
        i5TlvAffiliatedSTAMetrics_Min_Len, length);
      rc = -1;
      goto end;
    }
    memcpy(mac, &pvalue[pos], MAC_ADDR_LEN);
    pos += MAC_ADDR_LEN;

    affiliated_sta = i5DmFindAffiliatedSTAInDevice(pdevice, mac);
    if (affiliated_sta == NULL) {
      i5TraceError("Affliated STA " I5_MAC_DELIM_FMT " does not exist in "
        "device " I5_MAC_DELIM_FMT "\n",
        I5_MAC_PRM(mac), I5_MAC_PRM(pdevice->DeviceId));
      continue;
    }

    affiliated_sta->affiliated_metric.bytes_sent = ntohl(*((unsigned int *)&pvalue[pos]));
    pos += 4;

    affiliated_sta->affiliated_metric.bytes_recv = ntohl(*((unsigned int *)&pvalue[pos]));
    pos += 4;

    affiliated_sta->affiliated_metric.packets_sent = ntohl(*((unsigned int *)&pvalue[pos]));
    pos += 4;

    affiliated_sta->affiliated_metric.packets_recv = ntohl(*((unsigned int *)&pvalue[pos]));
    pos += 4;

    affiliated_sta->affiliated_metric.tx_packet_err = ntohl(*((unsigned int *)&pvalue[pos]));
    pos += 4;

    i5Trace("STA["I5_MAC_DELIM_FMT"] bytes_sent %u bytes_recv %u packets_sent %u "
      "packets_recv %u tx_packet_err %u\n",
      I5_MAC_PRM(mac), affiliated_sta->affiliated_metric.bytes_sent,
      affiliated_sta->affiliated_metric.bytes_recv, affiliated_sta->affiliated_metric.packets_sent,
      affiliated_sta->affiliated_metric.packets_recv,
      affiliated_sta->affiliated_metric.tx_packet_err);
  }
  if (!found) {
    rc = -1;
  }
end:
  return rc;
}

#endif /* MULTIAP */
