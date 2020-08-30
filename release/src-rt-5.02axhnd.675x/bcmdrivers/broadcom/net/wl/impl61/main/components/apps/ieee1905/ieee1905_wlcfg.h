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

#ifndef _IEEE1905_WLCFG_H_
#define _IEEE1905_WLCFG_H_

#if defined(WIRELESS)
#include "i5ctl.h"
#include "ieee1905_socket.h"
#include "ieee1905_message.h"
#include "ieee1905_tlv.h"

#define I5_WLCFG_NVRAM_AL_MAC "multiap_almac"
#define I5_WLCFG_NVRAM_CTL_AL_MAC "multiap_ctl_almac"
#define I5_WLCFG_NVRAM_LAN_HWADDR "lan_hwaddr"
#define I5_WLCFG_NVRAM_WPS_BH_BSS "wps_backhaul_ifnames"
#define I5_WLCFG_NVRAM_WPS_FH_IFNAME "wps_custom_ifnames"
#define I5_WLCFG_NVRAM_BACKHAUL_TYPE  "multiap_backhaultype"
#define I5_WLCFG_NVRAM_AGENT_CONFIGURED "map_agent_configured"
#define I5_WLCFG_NVRAM_LOOPBACK       "map_lo_listen"

#define I5_WLCFG_BACKHAUL_ETH   0x01
#define I5_WLCFG_BACKHAUL_WIFI  0x02

int i5ctlWlcfgHandler(i5_socket_type *psock,t_I5_API_WLCFG_MSG *pMsg);
unsigned int i5WlCfgAreMediaTypesCompatible(unsigned short mediaType1, unsigned short mediaType2);
unsigned short i5WlCfgFetchWirelessIfInfo(char const *ifname, unsigned char *pMediaInfo, int *pMediaLen,
                                          unsigned char *netTechOui, unsigned char *netTechVariant, unsigned char *netTechName, unsigned char *url, int sizeUrl);
char *i5WlcfgGetWlParentInterface(char const *ifname, char *wlParentIf);
char *i5WlcfgGetIfnameFromWlParent(const char *wlParent);
int i5WlCfgGetWdsMacFromName(char const * ifname, char *macString, int maxLen);
void i5WlcfgApAutoconfigurationStart(const char *ifname);
void i5WlcfgApAutoconfigurationRenew(const char *ifname);
void i5WlcfgApAutoconfigurationStop(const char *ifname);
int i5WlcfgApAutoConfigProcessMessage( i5_message_type *pmsg, unsigned int freqband,
unsigned char *pWscData, int wscDataLen, unsigned char *mac);
void i5WlcfgMarkAllInterfacesUnconfigured();
int i5WlcfgApAutoconfigurationRenewProcess(i5_message_type * pmsg, unsigned int freqband,
  unsigned char *neighbor_al_mac_address);
void i5WlCfgInit(void);
void i5WlCfgDeInit(void);
int i5WlCfgGet1905MacAddress(unsigned int multiapMode, unsigned char *MACAddress);
char *i5WlCfgGetNvVal(const char *ifname, const char *name);
int wl_ioctl(char *name, int cmd, void *buf, int len);
#ifdef MULTIAP

typedef struct wsc_data
{
  unsigned char mac[MAC_ADDR_LEN];
  unsigned char rf_band;
  unsigned char msg_type;
} wsc_data_t;

/* Creates and updates the media type and media specific info from chanspec */
void i5WlCfgUpdateMediaInfo(unsigned char *InterfaceId, unsigned short chanspec,
  unsigned char *bssid, unsigned char mapflags);
/* Get chanspec from Mediaspecific info */
void i5WlCfgGetChanspecFromMediaInfo(unsigned char *MediaSpecificInfo,
  unsigned short *outChanspec);
/* Get Regulatory class of Interface */
int i5WlCfgGetRclass(char* ifname, unsigned short chspec, unsigned short *out_rclass);
/* Get chanspec of the interface */
int i5WlCfgGetChanspec(char *ifname, chanspec_t *out_chanspec);
int i5WlCfgIsInterfaceEnabled(char *ifname);
void i5WlCfgHandleWPSPBC(char* ifname, int mode);
int Wlcfg_proto_create_m1(unsigned char band, unsigned char **m1, int *m1_size, i5_wps_wscKey **m1_keys);
int Wlcfg_proto_create_m2(unsigned char band, unsigned char *m1, int m1_len,
  ieee1905_client_bssinfo_type *bssinfo, unsigned char **m2, int *m2_size);
int Wlcfg_proto_process_m2(i5_wps_wscKey *key, unsigned char *m1, int m1_len, unsigned char *m2,
  int m2_len, ieee1905_client_bssinfo_type *bssinfo);
void wlcfg_wsc_free(unsigned char *m1, i5_wps_wscKey *keys);
void wlcfg_wsc_get_data(unsigned char *msg, int len, wsc_data_t *data);
/* Process the Ap autoconfiguration search and response */
int i5WlCfgProcessAPAutoConfigSearch(i5_message_type *pmsg, unsigned int freqband,
  unsigned char *searcher_al_mac_address);
/* Process the Ap autoconfiguration WSC M1 message */
int i5WlCfgProcessAPAutoConfigWSCM1(i5_message_type *pmsg, i5_dm_device_type *pdevice,
  unsigned char *pWscData, int wscDataLen, unsigned char *radioMac,
  ieee1905_radio_caps_type *RadioCaps);
/* Process the Ap autoconfiguration WSC M2 message */
int i5WlCfgProcessAPAutoConfigWSCM2(i5_message_type *pmsg, unsigned char *radioMac,
  ieee1905_vendor_data *msg_data);
/* Create mediaspecific info */
void i5WlCfgCreateMediaInfo(unsigned char *InterfaceId, unsigned char *bssid,
  unsigned short chanspec, unsigned char mapflags, unsigned char *MediaSpecificInfo);
int i5WlCfgGetPrefix(const char *ifname, char *prefix, int prefix_len);
int i5WlCfgIsVirtualInterface(const char *ifname);
void i5WlCfgMultiApControllerSearch(void *arg);
/* Checks whether the interface is backhaul or not */
int i5WlCfgIsBackHaulInterface(const char *ifname);
/* Timeout after sending M1 */
void i5WlCfgMultiApWSCTimeout(void *arg);
/* Get band based on channel */
int i5WlCfgChannelToband(unsigned char channel);
/* Check if the AL MAC address present in the BSS info table which the controller has */
int i5WlCfgIsALMACPresentInControllerTable(unsigned char *al_mac);
/* Check is the wireless interface is in bridge or not */
int i5WlCfgIsInterfaceInFwder(const char *ifname);
#if defined(__CONFIG_DHDAP__) && defined(__CONFIG_GMAC3__)
/* Set IEEE1905 AL MAC or Multicast MAC, so that the driver will send the packet to socket created
 * on the interface directly instead of sending it to bridge socket
 */
int i5WlCfgSetIEEE1905MACInDHD(char *ifname, unsigned char *mac, int isUcast);
#endif /* __CONFIG_DHDAP__ &&  __CONFIG_GMAC3__ */
/* Get Device type */
int i5WlCfgGetDevType(char *name, void *buf, int len);
#endif /* MULTIAP */
#endif // endif

#endif // endif
