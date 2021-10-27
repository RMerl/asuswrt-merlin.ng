/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <time.h>
#include <sched.h>
#include <ethernet.h>
#include <bcmevent.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <bcmwifi_rspec.h>
#include <bcmendian.h>

#include "cms.h"
#include "cms_core.h"
#include "cms_qdm.h"
#include "cms_msg.h"
#include "wlDiagnostic.h"


void *msgHandle=NULL;
int event_fd = -1;
wl_bss_info_resv_t escan_bss_result[MAX_ESCAN_BSS];


/* function */
static int open_eventfd()
{
   struct sockaddr_ll sll;
   
   cmsLog_debug("enter");
   
   event_fd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE_BRCM));
   if (event_fd < 0) 
   {
      cmsLog_error("Cannot create socket %s", strerror(errno));
      return errno;
   }

   /* bind the socket first before starting escan so we won't miss any event */
   memset(&sll, 0, sizeof(sll));
   sll.sll_family = AF_PACKET;
   sll.sll_protocol = htons(ETHER_TYPE_BRCM);
   if (bind(event_fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) 
   {
      cmsLog_error("Cannot bind %s", strerror(errno));
      return errno;
   }

   return 0;
}


static char *wl_format_ssid(uint8* ssid, int ssid_len)
{
   static char ssid_buf[32];
   int i, c;
   char *p = ssid_buf;

   memset(ssid_buf, 0, sizeof(ssid_buf));
   
   for (i = 0; i < ssid_len; i++) 
   {
      c = (int)ssid[i];
      if (c == '\\') {
         if (p - ssid_buf + 2 >= sizeof(ssid_buf))
         {
            cmsLog_error("truncate ssid %s", ssid);
            return ssid_buf;
         }
         *p++ = '\\';
         *p++ = '\\';
      } 
      else if (isprint((uchar)c)) 
      {
         if (p - ssid_buf + 1 >= sizeof(ssid_buf))
         {
            cmsLog_error("truncate ssid %s", ssid);
            return ssid_buf;
         }
         *p++ = (char)c;
      } 
      else 
      {
         if (p - ssid_buf + 4 >= sizeof(ssid_buf))
         {
            cmsLog_error("truncate ssid %s", ssid);
            return ssid_buf;
         }
         p += sprintf(p, "\\x%02X", c);
      }
   }
   *p = '\0';

   return ssid_buf;
}


static char *wl_ether_etoa(const struct ether_addr *n)
{
   static char etoa_buf[ETHER_ADDR_LEN * 3];
   char *c = etoa_buf;
   int i;

   for (i = 0; i < ETHER_ADDR_LEN; i++) 
   {
      if (i) *c++ = ':';
      c += sprintf(c, "%02X", n->octet[i] & 0xff);
   }
   return etoa_buf;
}


static const char *capmode2str(uint16 capability)
{
   capability &= (DOT11_CAP_ESS | DOT11_CAP_IBSS);

   if (capability == DOT11_CAP_ESS)
      return "Infrastructure";
   else if (capability == DOT11_CAP_IBSS)
      return "AdHoc";
   else
      return "<unknown>";
}


static void wl_get_rate(uint8 *rates, uint count, char *basic_rate_buf, int bBufLen, char *supported_rate_buf, int sBufLen)
{
   uint isFirstBasicRate = 1, isFirstSupportedRate = 1;
   char rateStr[8];
   uint i, r;
   bool b;

   if (basic_rate_buf) memset(basic_rate_buf, 0, bBufLen);
   if (supported_rate_buf) memset(supported_rate_buf, 0, sBufLen);
   
   for (i = 0; i < count; i++) 
   {
      r = rates[i] & 0x7f;
      b = rates[i] & 0x80;
      
      /* Assuming any "rate" above 54 Mbps is a BSS Membership Selector value */
      if (r > WLC_MAXRATE) 
      {
         continue;
      }
      if (r == 0)
      {
         break;
      }

      snprintf(rateStr, sizeof(rateStr), "%d%s", (r / 2), (r % 2)?".5":"");
      
      if (b && basic_rate_buf)
      {
         if (isFirstBasicRate)
         {
            strcat(basic_rate_buf, rateStr);
            isFirstBasicRate = 0;
         }
         else
         {
            strcat(basic_rate_buf, ",");
            strcat(basic_rate_buf, rateStr);
         }
      }
      
      if (supported_rate_buf)
      {
         if (isFirstSupportedRate)
         {
            strcat(supported_rate_buf, rateStr);
            isFirstSupportedRate = 0;
         }
         else
         {
            strcat(supported_rate_buf, ",");
            strcat(supported_rate_buf, rateStr);
         }
      }
   }
}


static char *wl_operating_standards(wl_bss_info_t *bi)
{
   static char supported_standards_buf[64];
   char supported_rate_buf[512], *p;

    memset(supported_standards_buf, 0, sizeof(supported_standards_buf));
   
   if (CHSPEC_IS2G(bi->chanspec))
   {
      //2.4G, could be b/g/n
      
      wl_get_rate(bi->rateset.rates, ntohl(bi->rateset.count), NULL, 0, supported_rate_buf, sizeof(supported_rate_buf));
      for (p = strtok(supported_rate_buf,","); p && atoi(p) <= 11; p = strtok(NULL, ","));
      
      strcat(supported_standards_buf,  p ? "b,g" : "b");
      if (bi->n_cap) strcat(supported_standards_buf, ",n");

   }
   else
   {
      //5G, could be a/n/ac

      strcpy(supported_standards_buf, "a");
      if (bi->n_cap) strcat(supported_standards_buf, ",n");
      if (bi->vht_cap) strcat(supported_standards_buf, ",ac");

   }

   return supported_standards_buf;
}


static bool wlu_is_wpa_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len)
{
   uint8 *ie = *wpaie;

   /* If the contents match the WPA_OUI and type=1 */
   if ((ie[1] >= 6) && !memcmp(&ie[2], WPA_OUI "\x01", 4)) 
   {
      return TRUE;
   }

   /* point to the next ie */
   ie += ie[1] + 2;
   /* calculate the length of the rest of the buffer */
   *tlvs_len -= (int)(ie - *tlvs);
   /* update the pointer to the start of the buffer */
   *tlvs = ie;

   return FALSE;
}

/*
 * Traverse a string of 1-byte tag/1-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag
 */
static uint8 *wlu_parse_tlvs(uint8 *tlv_buf, int buflen, uint key)
{
   uint8 *cp;
   int totlen;

   cp = tlv_buf;
   totlen = buflen;

   /* find tagged parameter */
   while (totlen >= 2) 
   {
      uint tag;
      int len;

      tag = *cp;
      len = *(cp +1);

      /* validate remaining totlen */
      if ((tag == key) && (totlen >= (len + 2)))
      {
         return (cp);
      }

      cp += (len + 2);
      totlen -= (len + 2);
   }

   return NULL;
}


static int wl_rsn_ie_parse_info(uint8* rsn_buf, uint len, rsn_parse_info_t *rsn)
{
   uint16 count;

   memset(rsn, 0, sizeof(rsn_parse_info_t));

   /* version */
   if (len < sizeof(uint16))
   {
      return 1;
   }

   rsn->version = ltoh16_ua(rsn_buf);
   len -= sizeof(uint16);
   rsn_buf += sizeof(uint16);

   /* Multicast Suite */
   if (len < sizeof(wpa_suite_mcast_t))
   {
      return 0;
   }

   rsn->mcast = (wpa_suite_mcast_t*)rsn_buf;
   len -= sizeof(wpa_suite_mcast_t);
   rsn_buf += sizeof(wpa_suite_mcast_t);

   /* Unicast Suite */
   if (len < sizeof(uint16))
   {
      return 0;
   }

   count = ltoh16_ua(rsn_buf);

   if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
   {
      return 1;
   }

   rsn->ucast = (wpa_suite_ucast_t*)rsn_buf;
   len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
   rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

   /* AKM Suite */
   if (len < sizeof(uint16))
   {
      return 0;
   }

   count = ltoh16_ua(rsn_buf);

   if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
   {
      return 1;
   }

   rsn->akm = (wpa_suite_auth_key_mgmt_t*)rsn_buf;
   len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
   rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

   /* Capabilites */
   if (len < sizeof(uint16))
   {
      return 0;
   }

   rsn->capabilities = rsn_buf;

   return 0;
}


static void wl_rsn_ie_dump(bcm_tlv_t *ie, int *secMode, int *encryptMode)
{
   int i;
   int rsn;
   wpa_ie_fixed_t *wpa = NULL;
   rsn_parse_info_t rsn_info;
   wpa_suite_t *suite;
   uint8 std_oui[3];
   int unicast_count = 0;
   int akm_count = 0;
   int err;

   if (ie->id == DOT11_MNG_RSN_ID) 
   {
      rsn = TRUE;
      memcpy(std_oui, WPA2_OUI, WPA_OUI_LEN);
      err = wl_rsn_ie_parse_info(ie->data, ie->len, &rsn_info);
   } 
   else 
   {
      rsn = FALSE;
      memcpy(std_oui, WPA_OUI, WPA_OUI_LEN);
      wpa = (wpa_ie_fixed_t*)ie;
      err = wl_rsn_ie_parse_info((uint8*)&wpa->version, wpa->length - WPA_IE_OUITYPE_LEN,
                                 &rsn_info);
   }

   if (err || rsn_info.version != WPA_VERSION)
   {
      return;
   }

   /* Check for unicast suite(s) */
   if (rsn_info.ucast) 
   {
      unicast_count = ltoh16_ua(&rsn_info.ucast->count);
      for (i = 0; i < unicast_count; i++) 
      {
         suite = &rsn_info.ucast->list[i];
         if (!memcmp(suite->oui, std_oui, 3)) 
         {
            switch (suite->type) 
            {
            case WPA_CIPHER_NONE:
               break;
            case WPA_CIPHER_WEP_40:
            case WPA_CIPHER_WEP_104:
               *secMode |= SECURITY_MODE_WEP;
               break;
            case WPA_CIPHER_TKIP:
               *encryptMode |= ENCRYPT_MODE_TKIP;
               break;
            case WPA_CIPHER_AES_OCB:
            case WPA_CIPHER_AES_CCM:
            case WPA_CIPHER_AES_GCM:
            case WPA_CIPHER_AES_GCM256:
               *encryptMode |= ENCRYPT_MODE_AES;
               break;
            default:
               break;
            }
         }
      }
   }
   
   /* Authentication Key Management */
   if (rsn_info.akm) 
   {
      akm_count = ltoh16_ua(&rsn_info.akm->count);
      for (i = 0; i < akm_count; i++) {
         suite = &rsn_info.akm->list[i];
         if (!memcmp(suite->oui, std_oui, 3)) 
         {
            switch (suite->type) 
            {
            case RSN_AKM_NONE:
               break;
            case RSN_AKM_UNSPECIFIED:
               *secMode |= rsn ? SECURITY_MODE_WPA2 : SECURITY_MODE_WPA;
               break;
            case RSN_AKM_PSK:
               *secMode |= rsn ? SECURITY_MODE_WPA2_PSK : SECURITY_MODE_WPA_PSK;
               break;
            case RSN_AKM_FBT_1X:
            case RSN_AKM_FBT_PSK:
            default:
               break;
            }
         }
      }
   }
}


static void wl_dump_wpa_rsn_ies(uint8* cp, uint len, char *secMode, int secModeLen,
                                char *encryptMode, int encryptModeLen)
{
   uint8 *parse = cp;
   uint parse_len = len;
   uint8 *wpaie;
   uint8 *rsnie;
   int securityMode = 0, encryptionMode = 0;
   uint8 isFirstEncrypt = 1;
   
   while ((wpaie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
   {
      if (wlu_is_wpa_ie(&wpaie, &parse, &parse_len)) break;
   }

   if (wpaie)
   {
      wl_rsn_ie_dump((bcm_tlv_t*)wpaie, &securityMode, &encryptionMode);
   }

   rsnie = wlu_parse_tlvs(cp, len, DOT11_MNG_RSN_ID);
   if (rsnie)
   {
      wl_rsn_ie_dump((bcm_tlv_t*)rsnie, &securityMode, &encryptionMode);
   }

   memset(secMode, 0, secModeLen);
   memset(encryptMode, 0, encryptModeLen);

   if (securityMode & SECURITY_MODE_WPA_PSK)
   {
      strncpy(secMode, (securityMode & SECURITY_MODE_WPA2_PSK) ? SECURITY_MODE_S_WPA_WPA2 : SECURITY_MODE_S_WPA, secModeLen);
   }
   else if (securityMode & SECURITY_MODE_WPA2_PSK)
   {
      strncpy(secMode, SECURITY_MODE_S_WPA2, secModeLen);
   }
   else if (securityMode & SECURITY_MODE_WPA)
   {
      strncpy(secMode, (securityMode & SECURITY_MODE_WPA2) ? SECURITY_MODE_S_WPA_WPA2_ENTERPRISE : SECURITY_MODE_S_WPA_ENTERPRISE, secModeLen);
   }
   else if (securityMode & SECURITY_MODE_WPA2)
   {
      strncpy(secMode, SECURITY_MODE_S_WPA2_ENTERPRISE, secModeLen); 
   }
   else if (securityMode & SECURITY_MODE_WEP)
   {
      strncpy(secMode, SECURITY_MODE_S_WEP, secModeLen);
   }
   else 
   {
      strncpy(secMode, SECURITY_MODE_S_NONE, secModeLen);
   }

   if (encryptionMode & ENCRYPT_MODE_TKIP)
   {
      strncpy(encryptMode, ENCRYPT_MODE_S_TKIP, encryptModeLen);
      isFirstEncrypt = 0;      
   }
   
   if (encryptionMode & ENCRYPT_MODE_AES)
   {
      if (!isFirstEncrypt)
      {
         strncat(encryptMode, ",", encryptModeLen);
      }
      
      strncat(encryptMode, ENCRYPT_MODE_S_AES, encryptModeLen);
   }
}


static void updateBssInfo(wl_bss_info_t *bi)
{
   int i;
   wl_bss_info_t *bss = NULL;
   
   for (i = 0; i < MAX_ESCAN_BSS && (bss = (wl_bss_info_t *)&escan_bss_result[i])->SSID_len; i++) 
   {
      if (memcmp(bi->BSSID.octet, bss->BSSID.octet, ETHER_ADDR_LEN) ||
          CHSPEC_BAND(bi->chanspec) != CHSPEC_BAND(bss->chanspec) ||
          bi->SSID_len != bss->SSID_len ||
          memcmp(bi->SSID, bss->SSID, bi->SSID_len))
      {
          continue;
      }

      /* We've already got this BSS. Update RSSI if necessary */
      /* Prefer valid RSSI */
      if (bi->RSSI == WLC_RSSI_INVALID)
      {
         break;
      }
      else if (bss->RSSI == WLC_RSSI_INVALID)
      {
         goto escan_update;
      }

      /* Prefer on-channel RSSI */
      if (!(bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) &&
           (bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL))
      {
         break;
      }
      else if ((bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) &&
         !(bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL))
      {
         goto escan_update;
      }

      /* Prefer probe response RSSI */
      if ((bi->flags & WL_BSS_FLAGS_FROM_BEACON) &&
          !(bss->flags & WL_BSS_FLAGS_FROM_BEACON))
      {
         break;
      }
      else if (!(bi->flags & WL_BSS_FLAGS_FROM_BEACON) &&
                (bss->flags & WL_BSS_FLAGS_FROM_BEACON))
      {
         goto escan_update;
      }

      /* Prefer better RSSI */
      if (bi->RSSI <= bss->RSSI)
      {
         break;
      }

escan_update:            /* Update known entry */
      bss->RSSI = bi->RSSI;
      bss->SNR = bi->SNR;
      bss->phy_noise = bi->phy_noise;
      bss->flags = bi->flags;
      break;
   }

   if (bss->SSID_len == 0)
   {
      cmsLog_debug("new bss(%d) %d\n", i);
      if (bi->length > sizeof(escan_bss_result[0]))
      {
         cmsLog_error("escan_bss_result size is too small(%d < %d)", 
                      sizeof(escan_bss_result[0]), bi->length);
      }
      else
      {
         memcpy(&escan_bss_result[i], bi, bi->length);
      }
   }
}


static void   dumpBssInfo(wl_bss_info_t *bi)
{
   char BasicDataTransferRates[256];
   char SupportedDataTransferRates[256];
   char SecurityMode[32];
   char EncryptionMode[32];
   
   printf("---------------------------\n");
   
   printf("SSID: %s\n", wl_format_ssid(bi->SSID, bi->SSID_len));

   printf("BSSID: %s\n", wl_ether_etoa(&bi->BSSID));
   
   printf("Mode: %s\n", capmode2str(bi->capability));

   printf("Channel: %d\n", CHSPEC_CHANNEL(bi->chanspec));

   printf("SignalStrength: %d dBm\n", bi->RSSI);

   wl_dump_wpa_rsn_ies((uint8 *)(((uint8 *)bi) + bi->ie_offset), bi->ie_length, SecurityMode, 
                  sizeof(SecurityMode), EncryptionMode, sizeof(EncryptionMode));

   printf("SecurityMode: %s\n", SecurityMode);
   printf("EncryptionMode: %s\n", EncryptionMode);
   
   printf("OperatingFrequencyBand: %sGHz\n", CHSPEC_IS2G(bi->chanspec)?"2.4":"5");

    //use same standard as operating standard
   printf("SupportedStandards: %s\n", wl_operating_standards(bi));
   
   printf("OperatingStandards: %s\n", wl_operating_standards(bi));

   printf("OperatingChannelBandwidth: %sMHz\n", 
         (CHSPEC_IS160(bi->chanspec) ?
         "160":(CHSPEC_IS80(bi->chanspec) ?
         "80" : (CHSPEC_IS40(bi->chanspec) ?
         "40" : (CHSPEC_IS20(bi->chanspec) ? "20" : "10")))));

   printf("BeaconPeriod: %d\n", bi->beacon_period);

   printf("Noise: %d dBm\n", bi->phy_noise);

    wl_get_rate(bi->rateset.rates, bi->rateset.count, BasicDataTransferRates, 
                sizeof(BasicDataTransferRates), SupportedDataTransferRates, sizeof(SupportedDataTransferRates));
   printf("BasicDataTransferRates: %s\n", BasicDataTransferRates);
   printf("SupportedDataTransferRates: %s\n", SupportedDataTransferRates);

   
   printf("DTIMPeriod: %d dBm\n", bi->dtim_period);
   
   printf("---------------------------\n");
}


static int runEscan()
{
   fd_set fdset;
   char buf[4096];
   int status = 0, bytes, escanComplete = 0;
   bcm_event_t *pvt_data;
   uint32 evt_type, escan_event_status;
   wl_escan_result_t *escan_data = NULL;
   CmsRet ret = CMSRET_SUCCESS;


   //open socket to receive escan result
   open_eventfd();

   //use wl0 to run escan
   system("wl -i wl0 escan -a 500");

   memset(escan_bss_result, 0, sizeof(escan_bss_result));
   
   while (!escanComplete)
   {
      /* init file descriptor set */
      FD_ZERO(&fdset);
      FD_SET(event_fd, &fdset);

      /* listen to data availible on all sockets */
      status = select(event_fd+1, &fdset, NULL, NULL, NULL);

      if (status <= 0) 
      {
         cmsLog_error("err from select: %s", strerror(errno));
         continue;
      }

      if (!FD_ISSET(event_fd, &fdset)) 
      {
         continue;
      }

      if ((bytes = recv(event_fd, buf, sizeof(buf), 0)) <= 0)
      {
         cmsLog_error("err recv: %s", strerror(errno));
         continue;
      }

      pvt_data = (bcm_event_t *)(buf);
      evt_type = ntohl(pvt_data->event.event_type);
      cmsLog_debug("recved brcm event(%d), event_type: %d\n", bytes, evt_type);

      switch (evt_type) 
      {
         case WLC_E_SCAN_COMPLETE:
         {
            cmsLog_debug("recved brcm event: WLC_E_SCAN_COMPLETE\n");
            escanComplete = 1;
            break;
         }
         case WLC_E_ESCAN_RESULT:
         {            
            escan_event_status = ntohl(pvt_data->event.status);
            escan_data = (wl_escan_result_t*)(pvt_data + 1);

            if (escan_event_status == WLC_E_STATUS_PARTIAL) 
            {
               cmsLog_debug("recved brcm event: WLC_E_ESCAN_RESULT: WLC_E_STATUS_PARTIAL");
               
               wl_bss_info_t *bi = &escan_data->bss_info[0];
               updateBssInfo(bi);
            } 
            else if (escan_event_status == WLC_E_STATUS_SUCCESS) 
            {
               cmsLog_debug("recved brcm event: WLC_E_ESCAN_RESULT: WLC_E_STATUS_SUCCESS");
               escanComplete = 1;
            } 
            else 
            {
               cmsLog_error("sync_id: %d, status:%d, misc. error/abort", escan_data->sync_id, escan_event_status);
               ret = CMSRET_INTERNAL_ERROR;
               escanComplete = 1;
            }
            break;
         }
         default: break;
      }      
   }
   
   close(event_fd);
   return ret;
}


static void update_escan_result_to_mdm(CmsRet retState)
{
   char BasicDataTransferRates[256];
   char SupportedDataTransferRates[256];
   char SecurityMode[32];
   char EncryptionMode[32];
   wl_bss_info_t *bi;
   Dev2WifiNeighboringwifidiagnosticResultObject *wifiDiagRes=NULL;
   Dev2WifiNeighboringwifidiagnosticObject *wifiDiag=NULL;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   int i;
   CmsRet ret = CMSRET_SUCCESS;

   //cleanup old ecan results
   while (cmsObj_getNext(MDMOID_DEV2_WIFI_NEIGHBORINGWIFIDIAGNOSTIC_RESULT,
          &iidStack, (void **) &wifiDiagRes) == CMSRET_SUCCESS)
   {
      cmsObj_deleteInstance(MDMOID_DEV2_WIFI_NEIGHBORINGWIFIDIAGNOSTIC_RESULT, &iidStack);
      cmsObj_free((void **)&wifiDiagRes);
      INIT_INSTANCE_ID_STACK(&iidStack);
   }

   //add escan results
   for (i = 0; i < MAX_ESCAN_BSS && (bi = (wl_bss_info_t *) &escan_bss_result[i])->SSID_len; i++) 
   {
      if (cmsLog_getLevel() >= LOG_LEVEL_NOTICE)
      {
         dumpBssInfo(bi);
      }

      INIT_INSTANCE_ID_STACK(&iidStack);
      
      if ((ret = cmsObj_addInstance(MDMOID_DEV2_WIFI_NEIGHBORINGWIFIDIAGNOSTIC_RESULT, &iidStack)) != CMSRET_SUCCESS)
      {
         cmsLog_error("add instance of WIFI_NEIGHBORINGWIFIDIAGNOSTIC returned %d", ret);
         return;
      }
      else if((ret = cmsObj_get(MDMOID_DEV2_WIFI_NEIGHBORINGWIFIDIAGNOSTIC_RESULT, &iidStack, 
                                OGF_NO_VALUE_UPDATE, (void **) &wifiDiagRes))!=CMSRET_SUCCESS) 
      {
         cmsLog_error("could not get instance of WIFI_NEIGHBORINGWIFIDIAGNOSTIC %d", ret);
         return;
      }

      //hardcode to radio 1
      CMSMEM_REPLACE_STRING(wifiDiagRes->radio, "Device.WiFi.Radio.1");
      
      CMSMEM_REPLACE_STRING(wifiDiagRes->SSID, wl_format_ssid(bi->SSID, bi->SSID_len));

      CMSMEM_REPLACE_STRING(wifiDiagRes->BSSID, wl_ether_etoa(&bi->BSSID));
      
      CMSMEM_REPLACE_STRING(wifiDiagRes->mode, capmode2str(bi->capability));
      
      wifiDiagRes->channel = CHSPEC_CHANNEL(bi->chanspec);
      
      wifiDiagRes->signalStrength = bi->RSSI;

      wl_dump_wpa_rsn_ies((uint8 *)(((uint8 *)bi) + bi->ie_offset), bi->ie_length, SecurityMode, 
                          sizeof(SecurityMode), EncryptionMode, sizeof(EncryptionMode));
      CMSMEM_REPLACE_STRING(wifiDiagRes->securityModeEnabled, SecurityMode);
      CMSMEM_REPLACE_STRING(wifiDiagRes->encryptionMode, EncryptionMode);
      
      CMSMEM_REPLACE_STRING(wifiDiagRes->operatingFrequencyBand, CHSPEC_IS2G(bi->chanspec)?"2.4GHz":"5GHz");
      
      CMSMEM_REPLACE_STRING(wifiDiagRes->supportedStandards, wl_operating_standards(bi));
      
      //use same standard as operating standard
      CMSMEM_REPLACE_STRING(wifiDiagRes->operatingStandards, wifiDiagRes->supportedStandards);
      
      CMSMEM_REPLACE_STRING(wifiDiagRes->operatingChannelBandwidth, 
                            (CHSPEC_IS160(bi->chanspec) ?
                            "160MHz":(CHSPEC_IS80(bi->chanspec) ?
                            "80MHz" : (CHSPEC_IS40(bi->chanspec) ?
                            "40MHz" : (CHSPEC_IS20(bi->chanspec) ? "20MHz" : "10MHz")))));
      
      wifiDiagRes->beaconPeriod = bi->beacon_period;
      
      wifiDiagRes->noise = bi->phy_noise;

      wl_get_rate(bi->rateset.rates, bi->rateset.count, BasicDataTransferRates, 
                  sizeof(BasicDataTransferRates), SupportedDataTransferRates, sizeof(SupportedDataTransferRates));
      CMSMEM_REPLACE_STRING(wifiDiagRes->basicDataTransferRates, BasicDataTransferRates);
      CMSMEM_REPLACE_STRING(wifiDiagRes->supportedDataTransferRates, SupportedDataTransferRates);

      wifiDiagRes->DTIMPeriod = bi->dtim_period;

      if ((ret = cmsObj_set(wifiDiagRes, &iidStack)) != CMSRET_SUCCESS)
      {
         cmsLog_error("set of WIFI_NEIGHBORINGWIFIDIAGNOSTIC_RESULT failed, ret=%d", ret);
      }
      else
      {
         cmsLog_debug("wifiDiagRes entry added");
      }
      
      cmsObj_free((void **) &wifiDiagRes);
   }


   INIT_INSTANCE_ID_STACK(&iidStack);
   if((ret = cmsObj_get(MDMOID_DEV2_WIFI_NEIGHBORINGWIFIDIAGNOSTIC, &iidStack, OGF_NO_VALUE_UPDATE, (void **) &wifiDiag))!=CMSRET_SUCCESS) 
   {
      cmsLog_error("could not get instance of WIFI_NEIGHBORINGWIFIDIAGNOSTIC %d", ret);
      return;
   }   

   CMSMEM_REPLACE_STRING(wifiDiag->diagnosticsState, (retState == CMSRET_SUCCESS) ? MDMVS_COMPLETE : MDMVS_ERROR);
   wifiDiag->resultNumberOfEntries = i;
   
   if ((ret = cmsObj_set(wifiDiag, &iidStack)) != CMSRET_SUCCESS)
   {
      cmsLog_error("set of WIFI_NEIGHBORINGWIFIDIAGNOSTIC failed, ret=%d", ret);
   }
   
   cmsObj_free((void **) &wifiDiag);
}


static void send_notification()
{
   CmsMsgHeader msg = EMPTY_MSG_HEADER;
   msg.type = CMS_MSG_DIAG;
   msg.src =  EID_WLDIAG;
   msg.dst = EID_SMD;
 
   msg.flags_event = 1;
   if (cmsMsg_send(msgHandle, &msg) != CMSRET_SUCCESS)
   {
      cmsLog_error("could not send out CMS_MSG_DIAG event msg");
   }
   else
   {
      cmsLog_debug("Send out CMS_MSG_DIAG event msg.");
   }
}

int main (int argc, char *argv[])
{
   SINT32 shmId=UNINITIALIZED_SHM_ID;
   int sendNotify = 0;
   CmsRet ret = CMSRET_SUCCESS;
   int ch;
   
   /* initialize cms */
   cmsLog_init(EID_WLDIAG);
   cmsLog_setLevel(LOG_LEVEL_ERR);
   //cmsLog_setLevel(LOG_LEVEL_DEBUG);

   cmsLog_debug("wlDiagnostic enter\n");
   
   if ((ret = cmsMsg_init(EID_WLDIAG, &msgHandle)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsMsg_init failed, ret=%d", ret);
      cmsLog_cleanup();
      return -1;
   }

   while ((ch = getopt(argc, argv, "nm:")) != -1)
   {
      switch (ch) 
      {
      case 'n':
         sendNotify = 1;
         break;
      case 'm':
         shmId = atoi(optarg);
         break;
      default:
         cmsLog_error("unrecognized option %c, ignore", ch);
      }
   }

   cmsLog_debug("sendNotify:%d, shmId:%d\n", sendNotify, shmId);

   if ((ret = cmsMdm_init(EID_WLDIAG, msgHandle, &shmId)) != CMSRET_SUCCESS)
   {
      cmsLog_error("cmsMdm_init error ret=%d", ret);
      cmsMsg_cleanup(&msgHandle);
      cmsLog_cleanup();
      return -1;
   }


   /* run escan and collect result */
   ret = runEscan();

   /* update datamodel */
   update_escan_result_to_mdm(ret);

   /* send out notification */
   if (sendNotify)
   {
      send_notification();
   }

   
   /* cleanup cms */
   cmsMdm_cleanup();
   cmsMsg_cleanup(&msgHandle);
   cmsLog_cleanup();

   return 0;
}



