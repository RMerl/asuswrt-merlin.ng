/***********************************************************************
 * <:copyright-BRCM:2013:proprietary:standard
 *
 *    Copyright (c) 2013 Broadcom
 *    All Rights Reserved
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
 * IEEE1905 Glue
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#if defined(WANDEV_SUPPORT)
#include <bcm_local_kernel_include/linux/sockios.h>
#else
#include <linux/sockios.h>
#endif // endif
#include <linux/if.h>
#include "ieee1905_glue.h"
#include "ieee1905_plc.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_ethstat.h"
#include "ieee1905_trace.h"
#include "ieee1905_datamodel_priv.h"
#include "i5api.h"
#include "ieee1905_brutil.h"
#include "ieee1905_tlv.h"
#if defined(SUPPORT_ETHSWCTL)
#include "ethswctl_api.h"
#endif // endif
#if defined(BRCM_CMS_BUILD)
#include "ieee1905_cmsutil.h"
#include "ieee1905_cmsmdm.h"
#endif // endif
#ifdef MULTIAP
#include <bcmnvram.h>
#include "shutils.h"
#include "wlutils.h"
#include "wlif_utils.h"
#endif /* MULTIAP */

#define I5_TRACE_MODULE i5TraceGlue

typedef unsigned short i5GlueInterfaceGetMediaType(const char *ifname, unsigned char *pMediaInfo,
  int *pMediaLen, unsigned char *netTechOui, unsigned char *netTechVariant,
  unsigned char *netTechName, unsigned char *url, int sizeUrl);

typedef struct _i5_glue_interface_name_info {
  char             *nameString;
  unsigned short   mediaType;
  i5GlueInterfaceGetMediaType *getMediaType;
  int              flags;
} i5_glue_interface_name_info;

static void i5GlueDisableHWSTP(char *ifname);
static void i5GlueEnableHWSTP(char *ifname);

#define I5_GLUE_INTERFACE_EXACT_MATCH   (1 << 0)

/* I5_GLUE_INTERFACE_EXACT_MATCH can be set in the flags field to require exact name match
 * anything requiring an exact match should be at the start of the list.
 * For ethernet interface(eth) use lan_ifnames in case of MULTIAP. For WDS and vlan interafces
 * use this list. Moved eth interfaces to non MULTIAP because in CMS build, the virtual interfaces
 * getting created for each eth interface. So, it creates socket for both and starts recieveing
 * packet on both interfaces.
 */

#define HWSTPFILE		"/tmp/hwstpstate.txt"

i5_glue_interface_name_info i5GlueInterfaceList[] = {
#if !defined(MULTIAP)
  {"eth",  I5_MEDIA_TYPE_UNKNOWN,  i5EthStatFetchIfInfo,       0 },
#endif /* MULTIAP */
  {"vlan",  I5_MEDIA_TYPE_UNKNOWN, i5EthStatFetchIfInfo,       0 },
#if defined(SUPPORT_HOMEPLUG)
  {"plc",  I5_MEDIA_TYPE_UNKNOWN,  i5PlcFetchIfInfo,           0 },
#endif // endif
  {"moca", I5_MEDIA_TYPE_MOCA_V11, NULL,                       0 },
#if defined(WIRELESS)
  {"wl",   I5_MEDIA_TYPE_UNKNOWN,  i5WlCfgFetchWirelessIfInfo, 0 },
  {"wds",  I5_MEDIA_TYPE_UNKNOWN,  i5WlCfgFetchWirelessIfInfo, 0 },
#endif // endif
  {NULL,   0 }
};

static int i5GlueAssign1905AlMac(unsigned int multiapMode)
{
#if defined(CMS_BOARD_UTIL_SUPPORT)
#if defined(SUPPORT_HOMEPLUG)
  unsigned char tmpMacAddr[MAC_ADDR_LEN];
  /* TDB: 1905 daemon and homeplugd start at roughly the same time and
     both request a MAC address. MAC addresses are reassigned every time
     the board boots. This can result in 1905 and homeplugd
     swapping MAC addresses after a reboot. To avoid this,
     1905 will request the PLC MAC before requesting its own MAC */
  i5CmsutilGet1901MacAddress(tmpMacAddr);
#endif // endif
  if ( i5CmsutilGet1905MacAddress(i5_config.i5_mac_address) < 0 ) {
    return -1;
  }
#elif defined(WIRELESS)
  if ( i5WlCfgGet1905MacAddress(multiapMode, i5_config.i5_mac_address) < 0 ) {
    return -1;
  }
#else
  /* assign a random MAC */
  srand((unsigned)time(NULL));
  i5_config.i5_mac_address[0] = 0x02;
  i5_config.i5_mac_address[1] = 0x10;
  i5_config.i5_mac_address[2] = 0x18;
  i5_config.i5_mac_address[3] = rand() & 0xFF;
  i5_config.i5_mac_address[4] = rand() & 0xFF;
  i5_config.i5_mac_address[5] = rand() & 0xFF;
#endif // endif

  printf("1905 MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           i5_config.i5_mac_address[0], i5_config.i5_mac_address[1], i5_config.i5_mac_address[2],
           i5_config.i5_mac_address[3], i5_config.i5_mac_address[4], i5_config.i5_mac_address[5]);
  return 0;
}

/* Check if the interface is WDS or not */
static int i5GlueIsIfnameWDS(const char *ifname)
{
  return (!strncmp(ifname, I5_GLUE_WLCFG_WDS_NAME_STRING, strlen(I5_GLUE_WLCFG_WDS_NAME_STRING)));
}

/* Check if the operational status UP or Down */
int i5GlueIsOperStateUP(const char *ifname)
{
  char path[MAX_PATH_NAME];
  char operStatus[32];
  FILE *f;

  snprintf(path, MAX_PATH_NAME, "/sys/class/net/%s/operstate", ifname);
  f = fopen(path, "r");
  if ( f ) {
    if ((fscanf(f, "%31s", &operStatus[0]) == 1) &&
        ((0 == strcmp("up", &operStatus[0])) || (0 == strcmp("unknown", &operStatus[0])) )
       ) {
      fclose(f);
      return 1;
    }
    fclose(f);
  }
  else {
    i5TraceError("cannot read operstate for interface %s\n", ifname);
  }

  return 0;
}

/* Once a data model entry is created for an interface 1905 will create a socket
 * to send and receive packets. This interface can be used to prevent a socket
 * from being created. Right now,
 * 1. If the interface is not oeprational it is excluded
 * 2. If the interface is only fronthaul in case of WiFi interface, it is excluded
 * 3. If the multiap_backhaultype NVRAM is set to WiFi, exclude ethernet interfaces
 */
int i5GlueInterfaceIsSocketRequired(char const *ifname, unsigned short media_type,
  char *real_ifname)
{
#ifdef MULTIAP
  char *nvval;
  unsigned int backhaul_type, mapFlags;
  char prefix[I5_MAX_IFNAME], tmpifname[I5_MAX_IFNAME];

  /* Special case for MultiAP certification on 47189 ACDBMR board where there is only one ethernet
   * port. We should not create socket on ethernet interface if the onboarding type is WiFi
   */
  if (i5DmIsInterfaceEthernet(media_type)) {
    nvval = nvram_get(I5_WLCFG_NVRAM_BACKHAUL_TYPE);
    /* Process only if the backhaul type is set. This is set from Sigma agent */
    if (nvval) {
      backhaul_type = strtoul(nvval, NULL, 0);
      /* If the backhaul type is WiFi, dont create socket for ethernet interface */
      if (backhaul_type == I5_WLCFG_BACKHAUL_WIFI) {
        i5TraceInfo("ifname[%s] MediaType[%d] BackhaulType[%d]. Its WiFi. So socket not "
          "required on ethernet interface\n", ifname, media_type, backhaul_type);
        return 0;
      }
    }
  } else if (i5DmIsInterfaceWireless(media_type)) {
    /* In case of wireless interfaces, no need to open sockets on non backhaul interfaces. In case
     * of backhaul AP, create only on WDS interface.
     * So, check the wlxy_map NVRAM before creating the sockets. If this interface is Virtual VLAN
     * interface then use the real ifname
     */
    if (real_ifname && strlen(real_ifname) > 0) {
      I5STRNCPY(tmpifname, real_ifname, sizeof(tmpifname));
    } else {
      I5STRNCPY(tmpifname, ifname, sizeof(tmpifname));
    }

    if (i5WlCfgGetPrefix(tmpifname, prefix, sizeof(prefix)) == 0) {
      mapFlags = strtoul(i5WlCfgGetNvVal(prefix, "map"), NULL, 0);
      i5TraceInfo("ifname[%s] tmpifname[%s] Prefix[%s] MediaType[%d] mapFlags[0x%x]\n",
        ifname, tmpifname, prefix, media_type, mapFlags);
      if (!(mapFlags & (IEEE1905_MAP_FLAG_STA | IEEE1905_MAP_FLAG_BACKHAUL))) {
        i5TraceInfo("ifname[%s] tmpifname[%s] Prefix[%s] MediaType[%d] mapFlags[0x%x]. Its not "
          "backhaul or STA interface\n", ifname, tmpifname, prefix, media_type, mapFlags);
        return 0;
      }
      if ((mapFlags & IEEE1905_MAP_FLAG_BACKHAUL) && !i5GlueIsIfnameWDS(tmpifname)) {
        i5TraceInfo("ifname[%s] tmpifname[%s] Prefix[%s] MediaType[%d] mapFlags[0x%x]. Its non "
          "WDS backhaul\n", ifname, tmpifname, prefix, media_type, mapFlags);
        return 0;
      }
    }
  }
#endif /* MULTIAP */

  /* don't create socket if interface is not operational */
  if (i5GlueIsOperStateUP(ifname)) {
    return 1;
  }

  return 0;
}

/* This function is used to exclude interfaces prior to creating a data model entry.
 *  - excludes interfaces flagged as WAN
 *  - if a bridge exists, exclude interfaces that are not members of a bridge
 *
 * NOTE: This function assumes the bridge interfaces are learned before all other interfaces
 */
int i5GlueInterfaceFilter(char const *ifname)
{
#if defined(WANDEV_SUPPORT)
  struct ifreq ifr;
  int sockfd, err;

  sockfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    printf("unable to create socket\n");
    return 0;
  }

  strncpy(ifr.ifr_name, ifname, I5_MAX_IFNAME);
  err = ioctl(sockfd, SIOCDEVISWANDEV, (void*)&ifr);
  close(sockfd);
  if((err != -1) && (ifr.ifr_flags != 0))
  {
    return 0;
  }
#endif // endif

  /* If MULTIAP is defined, interfaces are filtered based on lan_ifnames.
   * So no need to check if the device is on bridge or not
   */
#ifndef MULTIAP
  if ( !i5BrUtilDevIsBridge (ifname) ) {
    i5_dm_device_type *selfDevice = i5DmGetSelfDevice();

    if (NULL == selfDevice) {
      i5TraceInfo("No device yet\n");
      return 0;
    }
    else if ( selfDevice->bridging_tuple_list.ll.next != NULL ) {
      char path[MAX_PATH_NAME];
      struct stat s;
      snprintf(path, MAX_PATH_NAME, "/sys/class/net/%s/brport", ifname);

      i5TraceInfo("Bridge Detected, checking for %s\n",path);

      if ( (stat(path, &s) != 0) || (!S_ISDIR(s.st_mode)) ) {
#if defined(WIRELESS)
        /* For boards 4709 DHDAP-ATLAS some interfaces will be
	 * on forwarder. Check if it is in forwarder before ignoring it
         */
        if (i5WlCfgIsInterfaceInFwder(ifname) == 1) {
          return 1;
        }
#endif // endif
        i5TraceInfo("Not in bridge: %s\n",ifname);
        return 0;
      }
    }
  }
#endif /* !MULTIAP */

  return 1;
}

unsigned short i5GlueInterfaceGetMediaInfoFromName( char const *ifname, unsigned char *pMediaInfo,
                                                    int *pMediaLen, unsigned char *pNetTechOui,
                                                    unsigned char *pNetTechVariant,
                                                    unsigned char *pNetTechName, unsigned char *url,
                                                    int sizeUrl, char *real_ifname)
{
  i5GlueInterfaceGetMediaType *getMediaType = NULL;
  i5_glue_interface_name_info * pNameInfo = &i5GlueInterfaceList[0];
  int ifNameLen;
  int mediaType = I5_MEDIA_TYPE_UNKNOWN;
  char tmpifname[I5_MAX_IFNAME];
#ifdef MULTIAP
  int unit = -1, found = 0, isSecondary = 0;
  char os_name[I5_MAX_IFNAME];
  char *ifnames;
#endif // endif

  i5TraceInfo("\n");

  I5STRNCPY(tmpifname, ifname, sizeof(tmpifname));

  if ( 0 == i5GlueInterfaceFilter(tmpifname) ) {
    return I5_MEDIA_TYPE_UNKNOWN;
  }

  if ( i5BrUtilDevIsBridge(tmpifname) ) {
    return I5_MEDIA_TYPE_BRIDGE;
  }

#ifdef MULTIAP
  ifnames = nvram_safe_get("lan_ifnames");
  /* If the ifname is not in lan_ifnames, it can be a VLAN interface. So use VLAN's real_ifname to
   * check in the lan_ifnames
   */
  if (!find_in_list(ifnames, ifname)) {
    /* The primary interface can be in secondary LAN. Still we need to create interface for it */
    if (!i5WlCfgIsVirtualInterface(ifname)) {
      if (find_in_list(nvram_safe_get("lan1_ifnames"), ifname)) {
        i5Trace("ifname[%s] found in lan1_ifnames\n", ifname);
        found = 1;
      }
    } else {
      /* If the ifname is virtual VLAN ifname, then start using the real ifname */
      if (I5_IS_GUEST_ENABLED(i5_config.flags) &&
        i5GlueIsIfnameVLAN(ifname, tmpifname, &isSecondary)) {
        found = 1;
	if (real_ifname) {
          I5STRNCPY(real_ifname, tmpifname, IFNAMSIZ);
	}
        if (isSecondary) {
          i5Trace("ifname[%s] tmpifname[%s] is Secondary, no need to use it\n", ifname, tmpifname);
          return I5_MEDIA_TYPE_UNKNOWN;
        }
        i5Trace("ifname[%s] is VLAN tmpifname[%s]\n", ifname, tmpifname);
      }
    }
  } else {
    found = 1;
  }

  if (found) {
    /* call nvifname_osifname() to guarantee that the interface name is in the  os native format */
    if (nvifname_to_osifname(tmpifname, os_name, sizeof(os_name)) < 0) {
      i5Trace("Failed nvifname_to_osifname ifname %s name %s\n", ifname, tmpifname);
    } else {
      if (wl_probe(os_name) || wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit))) {
        /* Its a Ethernet interface */
        getMediaType = i5EthStatFetchIfInfo;
        i5Trace("ifname %s tmpifname %s os_name %s Ethernet Interface\n", ifname, tmpifname,
          os_name);
      } else {
        /* Its a wireless interface */
        getMediaType = i5WlCfgFetchWirelessIfInfo;
        i5Trace("ifname %s tmpifname %s os_name %s Wireless Interface\n", ifname, tmpifname,
          os_name);
      }
    }
  }

  if (getMediaType) {
    mediaType = getMediaType(tmpifname, pMediaInfo, pMediaLen, pNetTechOui, pNetTechVariant,
      pNetTechName, url, sizeUrl);
    i5Trace("ifname %s tmpifname %s mediaType %x\n", ifname, tmpifname, mediaType);
    return mediaType;
  }

#endif /* MULTIAP */

  ifNameLen = strlen(tmpifname);

  while( pNameInfo->nameString != NULL ) {
    int nameLen = strlen(pNameInfo->nameString);
    if ( ifNameLen >= strlen(pNameInfo->nameString) ) {
      if ( pNameInfo->flags & I5_GLUE_INTERFACE_EXACT_MATCH ) {
        if (0 == strcmp(tmpifname, pNameInfo->nameString)) {
          break;
        }
      }
      else {
        if (0 == strncmp(tmpifname, pNameInfo->nameString, nameLen) ) {
          break;
        }
      }
    }
    pNameInfo++;
  }

  if ( pNameInfo->nameString != NULL ) {
    if ( pNameInfo->getMediaType != NULL ) {
      mediaType = (pNameInfo->getMediaType)(tmpifname, pMediaInfo, pMediaLen, pNetTechOui,
        pNetTechVariant, pNetTechName, url, sizeUrl);
    }
    else {
      mediaType = pNameInfo->mediaType;
    }
  }

  return mediaType;
}

int i5GlueMainInit(unsigned int multiapMode)
{
  if ( i5GlueAssign1905AlMac(multiapMode) < 0 ) {
    return -1;
  }

#if defined(SUPPORT_ETHSWCTL)
  /* Program the integrated switch to not flood the 1905 MAC address on all ports */
  bcm_multiport_set(0, I5_MULTICAST_MAC);
#endif // endif

  ieee1905_glist_init(&i5_config.vlan_ifr_list);

  return 0;
}

void i5GlueSaveConfig()
{
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  t_I5_API_CONFIG_BASE cfg;
  t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY cfgNT;

  i5TraceInfo("\n");

  cfg.isEnabled = i5_config.running;
  strncpy(cfg.deviceFriendlyName, i5_config.friendlyName, I5_DEVICE_FRIENDLY_NAME_LEN-1);
  if (I5_IS_REGISTRAR(i5_config.flags)) {
    cfg.isRegistrar = 1;
  } else {
    cfg.isRegistrar = 0;
  }
  cfg.apFreqBand24En = i5_config.freqBandEnable[i5MessageFreqBand_802_11_2_4Ghz];
  cfg.apFreqBand5En = i5_config.freqBandEnable[i5MessageFreqBand_802_11_5Ghz];
  i5CmsMdmSaveConfig(&cfg);

  cfgNT.isEnabled = i5_config.networkTopEnabled;
  i5CmsMdmSaveNetTopConfig(&cfgNT);
#endif // endif
}

#ifdef MULTIAP
/* Remove the socket created for interface */
static void i5GlueRemoveInterfaceSocket(char *ifname)
{
  i5_socket_type *pif;

  pif = i5SocketFindDevSocketByName(ifname);
  if (pif) {
    if (pif->u.sll.notify) {
      pif->u.sll.notify(pif, IF_OPER_DOWN);
    }
    i5MessageCancel(pif);
    i5SocketClose(pif);
    pif = NULL;
    i5TraceInfo("ifname[%s] closing\n", ifname);
  } else {
    i5TraceInfo("ifname[%s] not found\n", ifname);
  }
}

/* Delete the VLAN interface created and add the primary interface to the bridge again */
static void i5GlueDeleteVLANInterface(char *ifname)
{
  char vprim[I5_MAX_IFNAME], vsec[I5_MAX_IFNAME];
  char prim_bridge[I5_MAX_IFNAME], sec_bridge[I5_MAX_IFNAME];

  /* Get the primary bridge */
  I5STRNCPY(prim_bridge, nvram_safe_get("lan_ifname"), sizeof(prim_bridge));
  if (strlen(prim_bridge) <= 0) {
    i5TraceError("Primary Bridge Name Not Known\n");
    return;
  }

  /* Get the Secondary bridge */
  I5STRNCPY(sec_bridge, nvram_safe_get("lan1_ifname"), sizeof(sec_bridge));
  if (strlen(sec_bridge) <= 0) {
    i5TraceError("Secondary Bridge Name Not Known\n");
    return;
  }

  snprintf(vprim, sizeof(vprim), "%s.%d", ifname, i5_config.prim_vlan_id);
  snprintf(vsec, sizeof(vsec), "%s.%d", ifname, i5_config.sec_vlan_id);

  /* Delete primary VLAN ifr from primary bridge */
  eval("brctl", "delif", prim_bridge, vprim);
  /* Add back the main interface to primary bridge */
  eval("brctl", "addif", prim_bridge, ifname);
  eval("ip", "link", "set", ifname, "up");

  /* Remove seconbdary VLAN ifr from secondary bridge */
  eval("brctl", "delif", sec_bridge, vsec);

  /* Remove VLAN interface created */
  eval("vlanctl", "--if-delete", vprim);
  eval("vlanctl", "--if-delete", vsec);

  i5TraceInfo("Deleted VLAN interface. Prim[%s] Sec[%s]\n", vprim, vsec);
}

/* Find Real VLAN ifname in the VLAN interface list */
static i5_vlan_ifr_node *i5GlueFindRealVlanIfr(char *ifname)
{
  i5_vlan_ifr_node *ifr_node;
  dll_t *item_p;

  for (item_p = dll_head_p(&i5_config.vlan_ifr_list.head);
    !dll_end(&i5_config.vlan_ifr_list.head, item_p);
    item_p = dll_next_p(item_p)) {

    ifr_node = (i5_vlan_ifr_node*)item_p;

    if (strcmp(ifr_node->ifname, ifname) == 0) {
      i5TraceInfo("ifname[%s] found in list\n", ifname);
      return ifr_node;
    }
  }

  return NULL;
}

/* Add VLAN interface to the list */
i5_vlan_ifr_node *i5GlueAddVlanIfr(char *ifname)
{
  i5_vlan_ifr_node *ifr_node;

  if ((ifr_node = i5GlueFindRealVlanIfr(ifname)) != NULL) {
    return ifr_node;
  }

  ifr_node = (i5_vlan_ifr_node *)malloc(sizeof(*ifr_node));
  if (!ifr_node) {
    i5TraceDirPrint("Malloc Failed\n");
    return NULL;
  }
  memset(ifr_node, 0, sizeof(*ifr_node));

  snprintf(ifr_node->ifname, sizeof(ifr_node->ifname), "%s", ifname);
  ieee1905_glist_append(&i5_config.vlan_ifr_list, (dll_t*)ifr_node);
  i5TraceInfo("ifname[%s] Added to list\n", ifname);

  return ifr_node;
}

/* Delete VLAN interface from the list */
void i5GlueDeleteVlanIfr(char *ifname)
{
  dll_t *item_p, *next_p;
  i5_vlan_ifr_node *ifr_node;

  /* Travese List */
  for (item_p = dll_head_p(&i5_config.vlan_ifr_list.head);
    !dll_end(&i5_config.vlan_ifr_list.head, item_p);
    item_p = next_p) {

    /* need to keep next item incase we remove node in between */
    next_p = dll_next_p(item_p);
    ifr_node = (i5_vlan_ifr_node*)item_p;

    if (strcmp(ifr_node->ifname, ifname) == 0) {

      i5GlueDeleteVLANInterface(ifname);
      if (ifr_node->hwStpDisabled) {
        i5GlueEnableHWSTP(ifname);
      }

      /* Remove item itself from list */
      ieee1905_glist_delete(&i5_config.vlan_ifr_list, item_p);
      free(item_p);
      i5TraceInfo("ifname[%s] Deleted from list\n", ifname);
      break;
    }
  }
}

/* Delete all the VLANs created */
void i5GlueDeleteAllVlanInterfaces()
{
  dll_t *item_p, *next_p;
  i5_vlan_ifr_node *ifr_node;

  /* Unset guest enabled flag, or else after deleting the VLAN interface, the socket creation
   * might again create the VLAN interfaces
   */
  i5_config.flags &= ~I5_CONFIG_FLAG_GUEST_ENABLED;

  /* Travese List */
  for (item_p = dll_head_p(&i5_config.vlan_ifr_list.head);
    !dll_end(&i5_config.vlan_ifr_list.head, item_p);
    item_p = next_p) {

    /* need to keep next item incase we remove node in between */
    next_p = dll_next_p(item_p);
    ifr_node = (i5_vlan_ifr_node*)item_p;

    i5GlueDeleteVLANInterface(ifr_node->ifname);
    if (ifr_node->hwStpDisabled) {
      i5GlueEnableHWSTP(ifr_node->ifname);
    }

    i5TraceInfo("ifname[%s] Deleted from list\n", ifr_node->ifname);
    /* Remove item itself from list */
    ieee1905_glist_delete(&i5_config.vlan_ifr_list, item_p);
    free(item_p);
  }
}

/* Create Primary VLAN interface for an interface */
static void i5CreatePrimaryVLAN(char *ifr, unsigned short vlanid, char *bridge)
{
  char vprim[I5_MAX_IFNAME];

  snprintf(vprim, sizeof(vprim), "%s.%d", ifr, vlanid);
  i5TraceInfo("For ifname[%s] vlanid[%d] bridge[%s] virtual[%s]\n", ifr, vlanid, bridge, vprim);

  /* Create VLAN interface */
  eval("vlanctl", "--mcast", "--if-create-name", ifr, vprim, "--if", ifr, "--set-if-mode-rg");

  /* Accept Data with 0 tags */
  eval("vlanctl", "--if", ifr, "--rx", "--tags", "0", "--set-rxif", vprim, "--rule-append");

  /* Send only the 0 tagged packets */
  eval("vlanctl", "--if", ifr, "--tx", "--tags", "0", "--filter-txif", vprim, "--rule-append");

  /* Do brige and interface operation */
  eval("brctl", "delif", bridge, ifr);
  eval("brctl", "addif", bridge, vprim);
  eval("ip", "link", "set", vprim, "up");
}

/* Create Secondary VLAN interface for an interface */
static void i5CreateSecondaryVLAN(char *ifr, unsigned short vlanid, char *bridge, char *prim_bridge)
{
  char vprim[I5_MAX_IFNAME], strvlanid[I5_MAX_VLANID_LEN], strether_type[I5_MAX_VLANID_LEN];

  snprintf(strvlanid, sizeof(strvlanid), "%d", vlanid);
  snprintf(vprim, sizeof(vprim), "%s.%s", ifr, strvlanid);
  snprintf(strether_type, sizeof(strether_type), "%d", i5_config.vlan_ether_type);
  i5TraceInfo("For ifname[%s] vlanid[%s] bridge[%s] virtual[%s] primary bridge[%s]\n",
    ifr, strvlanid, bridge, vprim, prim_bridge);

  /* Create VLAN interface */
  eval("vlanctl", "--mcast", "--if-create-name", ifr, vprim, "--if", ifr, "--set-if-mode-rg");

  /* Accept secondary tagged frame */
  eval("vlanctl", "--if", ifr, "--rx", "--tags", "1", "--filter-vid", strvlanid, "0",
    "--filter-ethertype", strether_type, "--pop-tag", "--set-rxif", vprim, "--rule-append");

  /* Send secondary tagged frame */
  eval("vlanctl", "--if", ifr, "--tx", "--tags", "0", "--filter-txif", vprim, "--push-tag",
    "--set-vid", strvlanid, "0", "--set-ethertype", strether_type, "--set-pbits", "0", "0",
    "--rule-append");

   /* Do brige and interface operation */
  eval("brctl", "delif", bridge, ifr);
  eval("brctl", "delif", prim_bridge, ifr);
  eval("brctl", "delif", prim_bridge, vprim);

  eval("brctl", "addif", bridge, vprim);
  eval("ip", "link", "set", vprim, "up");

  return;
}

/* For an interface create VLAN interfaces */
void i5GlueCreateVLAN(char *ifname, int isWireless)
{
  char prim_bridge[I5_MAX_IFNAME], sec_bridge[I5_MAX_IFNAME];
  char *hwstp = NULL;

  /* Get the primary bridge */
  I5STRNCPY(prim_bridge, nvram_safe_get("lan_ifname"), sizeof(prim_bridge));
  if (strlen(prim_bridge) <= 0) {
    i5TraceError("Primary Bridge Name Not Known\n");
    return;
  }

  /* Get the Secondary bridge */
  I5STRNCPY(sec_bridge, nvram_safe_get("lan1_ifname"), sizeof(sec_bridge));
  if (strlen(sec_bridge) <= 0) {
    i5TraceError("Secondary Bridge Name Not Known\n");
    return;
  }

  /* Create primary VLAN interface */
  i5CreatePrimaryVLAN(ifname, i5_config.prim_vlan_id, prim_bridge);

  /* Create secondary vlan interface for guest network */
  i5CreateSecondaryVLAN(ifname, i5_config.sec_vlan_id, sec_bridge, prim_bridge);

  i5GlueAddVlanIfr(ifname);
  /* Disable HW STP for ehternet interface */
  if (!isWireless) {
    hwstp = nvram_safe_get("map_disable_hwstp");
    if (hwstp && (hwstp[0] != '\0') && (0 == strtoul(hwstp, NULL, 0))) {
      /* If map_disable_hwstp is set to 0 then dont disable HW STP */
      return;
    }

    i5GlueDisableHWSTP(ifname);
  }

  return;
}

/* Checks whether VLANs can be created or not */
int i5GlueIsCreateVLANS()
{
  if (!I5_IS_GUEST_ENABLED(i5_config.flags)) {
    i5TraceInfo("Guest Network is not enabled. Flags[%x]\n", i5_config.flags);
    return 0;
  }

  /* Create VLANs only from one process(i.e from agent) if both controller and agent is running
   * on same device
   */
  if ((I5_IS_MULTIAP_CONTROLLER(i5_config.device_mode) &&
      I5_IS_MULTIAP_AGENT(i5_config.device_mode))) {
      if (I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
        return 0;
      }
  }

  i5TraceInfo("Allowed to Create VLANS. Flags[%x]\n", i5_config.flags);
  return 1;
}

/* Checks whether the ifname is Virtual VLAN interface or not. If yes, then it stores the
 * real_ifname argument with the primary ifname from which the VLAN interface is created
 */
int i5GlueIsIfnameVLAN(const char *ifname, char *real_ifname, int *isSecondary)
{
  char dev_type[50], tmpifname[I5_MAX_IFNAME];
  char *dotLocation, *lan_ifnames;
  int vlan_id, tmpiSSecondary = 0, searchLanIfnames = 1;

  I5STRNCPY(tmpifname, ifname, sizeof(tmpifname));

  /* If the ifname is WDS, then it will not be there in lan_ifnames. So do not search there */
  if (i5GlueIsIfnameWDS(ifname)) {
    searchLanIfnames = 0;
  }

  /* Virtual VLAN interfaces gives device type as "Broadcom VLAN Interface" */
  if (i5WlCfgGetDevType(tmpifname, dev_type, sizeof(dev_type)) < 0) {
    i5TraceError("Ifname[%s] Get Dev Type Failed\n", ifname);
    return 0;
  }

  if (!strstr(dev_type, "Broadcom VLAN Interface")) {
    i5TraceInfo("Ifname[%s] DevType[%s] is non VLAN\n", ifname, dev_type);
    return 0;
  }

  if (searchLanIfnames) {
    lan_ifnames = nvram_safe_get("lan_ifnames");

    /* If the ifname is not in lan_ifnames, it can be a VLAN interface. So use VLAN's
     * real_ifname to check in the lan_ifnames
     */
    if (find_in_list(lan_ifnames, ifname)) {
      return 0;
    }
  }

  /* Get the real ifname that is primary ifname by removing the VLAN ID from the ifname that is
   * all characters after the last dot
   */
  dotLocation = strrchr(tmpifname, '.');
  if (dotLocation == NULL) {
    i5TraceError("Ifname[%s] DevType[%s] No string after dot\n", ifname, dev_type);
    return 0;
  }

  if (isSecondary) {
    vlan_id = atoi(dotLocation+1);
    i5TraceInfo("Ifname[%s] VLANID[%d] SecondaryVLANID[%d]\n", ifname, vlan_id,
      i5_config.sec_vlan_id);
    if (vlan_id == i5_config.sec_vlan_id) {
      tmpiSSecondary = 1;
    }
  }
  /* Remove all characters from dot location to get real_ifname */
  dotLocation[0] = '\0';

  /* If the real_ifname which is primary is in lan_ifnames, then the ifname is Virtual VLAN */
  if (searchLanIfnames && !find_in_list(lan_ifnames, tmpifname)) {
    i5TraceInfo("Ifname[%s] DevType[%s] RealIfname[%s] not found in lan_ifnames[%s]\n",
      ifname, dev_type, tmpifname, lan_ifnames);
    return 0;
  }

  if (real_ifname) {
    I5STRNCPY(real_ifname, tmpifname, IFNAMSIZ);
  }
  if (isSecondary) {
    *isSecondary = tmpiSSecondary;
  }

  i5TraceInfo("Ifname[%s] DevType[%s] is VLAN RealIfname[%s] iSSecondary[%d] "
    "searchLanIfnames[%d]\n", ifname, dev_type, tmpifname, tmpiSSecondary, searchLanIfnames);
  return 1;
}

/* Remove the socket created for primary interface if the virtual VLAN interface is created. */
void i5GlueRemovePrimarySocketOnVirtualSocketCreate(const char *ifname)
{
  char real_ifname[I5_MAX_IFNAME];

  /* If the ifname is virtual VLAN ifname, then delete the socket created for real ifname */
  if (i5GlueIsIfnameVLAN(ifname, real_ifname, NULL)) {
    i5TraceInfo("Remove socket for Primary[%s] of Virtual[%s]\n", real_ifname, ifname);
    i5GlueRemoveInterfaceSocket(real_ifname);
  }
}
#endif /* MULTIAP */

void i5GlueLoadConfig(unsigned int supServiceFlag, int isRegistrar)
{
  i5_dm_device_type *pdevice = i5DmGetSelfDevice();
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  t_I5_API_CONFIG_BASE cfg;
  t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY cfgNT;
#endif // endif

  i5TraceInfo("\n");

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  i5CmsMdmLoadConfig(&cfg);
  i5_config.running = cfg.isEnabled;
  /* cfg.deviceFriendlyName is NULL terminated */
  strncpy(i5_config.friendlyName, cfg.deviceFriendlyName, I5_DEVICE_FRIENDLY_NAME_LEN-1);
  if (cfg.isRegistrar) {
    i5_config.flags |= I5_CONFIG_FLAG_REGISTRAR;
  }
  i5_config.freqBandEnable[i5MessageFreqBand_802_11_2_4Ghz] = cfg.apFreqBand24En;
  i5_config.freqBandEnable[i5MessageFreqBand_802_11_5Ghz] = cfg.apFreqBand5En;

  i5CmsMdmLoadNetTopConfig(&cfgNT);
  i5_config.networkTopEnabled = cfgNT.isEnabled;
#else
  i5_config.running = 1;
  snprintf(i5_config.friendlyName, sizeof(i5_config.friendlyName), "%02X%02X%02X%02X%02X%02X",
           i5_config.i5_mac_address[0], i5_config.i5_mac_address[1], i5_config.i5_mac_address[2],
           i5_config.i5_mac_address[3], i5_config.i5_mac_address[4], i5_config.i5_mac_address[5]);
  if (isRegistrar) {
    i5_config.flags |= I5_CONFIG_FLAG_REGISTRAR;
  }

#ifdef MULTIAP
  /* Supported Service Flag */
  if (I5_IS_MULTIAP_CONTROLLER(supServiceFlag)) {
    i5_config.flags |= I5_CONFIG_FLAG_CONTROLLER;
  }
  if (I5_IS_MULTIAP_AGENT(supServiceFlag)) {
    i5_config.flags |= I5_CONFIG_FLAG_AGENT;
  }
  ieee1905_glist_init(&(i5_config.policyConfig.no_steer_sta_list));
  ieee1905_glist_init(&i5_config.policyConfig.no_btm_steer_sta_list);
  ieee1905_glist_init(&i5_config.policyConfig.steercfg_bss_list);
  ieee1905_glist_init(&i5_config.policyConfig.metricrpt_config.ifr_list);

  /* In controller this will have all the details filled. In Agent it will have only SSID,
   * fronthaul and backhaul details
   */
  ieee1905_glist_init(&i5_config.client_bssinfo_list);
  i5_config.discovery_timeout = strtoul(nvram_safe_get("map_discovery_timeout"), NULL, 0);
  i5_config.device_mode = (unsigned char)strtoul(nvram_safe_get("multiap_mode"), NULL, 0);
#endif /* MULTIAP */
  i5_config.freqBandEnable[i5MessageFreqBand_802_11_2_4Ghz] = 1;
  i5_config.freqBandEnable[i5MessageFreqBand_802_11_5Ghz] = 1;
  i5_config.networkTopEnabled = 0;
#endif // endif
  if ( pdevice )
  {
     /* cfg.deviceFriendlyName is NULL terminated */
     strncpy(pdevice->friendlyName, i5_config.friendlyName, I5_DEVICE_FRIENDLY_NAME_LEN);
  }
}

int i5GlueAssignFriendlyName(unsigned char *deviceId, char *pFriendlyName, int maxLen)
{
  if ( (NULL == deviceId) || (NULL == pFriendlyName) ) {
    i5TraceError("Invalid input\n");
    return -1;
  }

#if defined(BRCM_CMS_BUILD)
  if ( i5DmDeviceIsSelf(deviceId) )
  {
     i5CmsUtilGetFriendlyName(deviceId, pFriendlyName, maxLen);
     return 0;
  }
#endif // endif
  snprintf(pFriendlyName, I5_DEVICE_FRIENDLY_NAME_LEN, "%02X%02X%02X%02X%02X%02X", deviceId[0], deviceId[1], deviceId[2], deviceId[3], deviceId[4], deviceId[5]);
  return 0;
}

void i5GlueMainDeinit()
{
#if defined(MULTIAP)
  i5GlueDeleteAllVlanInterfaces();
  i5DmConfigListFree(&i5_config.policyConfig);
  i5DmGlistCleanup(&i5_config.client_bssinfo_list);
#endif	/* MULTIAP */
}

#if defined(MULTIAP) && defined(CONFIG_HOSTAPD)
int i5GlueWpsPbc(char *fhIfname, char *bhIfname)
{
   return wl_wlif_wps_pbc_hdlr(fhIfname, bhIfname);
}

int i5GlueIsHapdEnabled()
{
   return nvram_match("hapd_enable", "1");
}
#endif	/* MULTIAP && CONFIG_HOSTAPD */

static bool i5GlueisHWSTPDisabled(char *ifname)
{
  char cmd[512];
  char *pbuf = NULL;
  FILE *fp = NULL;

  /* Get HW STP state for <ifname> in file */
  snprintf(cmd, sizeof(cmd), "ethswctl -c ifstp -i %s > %s", ifname, HWSTPFILE);
  system(cmd);

  /* Open the HW STP state file */
  fp = fopen(HWSTPFILE, "r");
  if (!fp) {  /* validate file open for reading */
    i5TraceError("Failed to open file %s Error: %s\n", HWSTPFILE, strerror(errno));
    return FALSE;
  }

  /* Format of the output of ethswctl -c ifstp -i <ifname> is as below
   * ===================================
   * <ifname> STP state: <State>
   * Success.
   * ===================================
  */

  /* Read the first line of the file only and last string is stp state */
  fgets(cmd, sizeof(cmd), fp);

  pbuf = strstr(cmd, "Disabled");

  fclose (fp);
  unlink(HWSTPFILE);

  if (pbuf) {
    return TRUE;
  }

  return FALSE;
}

static void i5GlueDisableHWSTP(char *ifname)
{
  char cmd[512];
  i5_vlan_ifr_node *ifr_node;

  if (i5GlueisHWSTPDisabled(ifname)) {
    i5TraceInfo("HW STP already disabled for %s\n", ifname);
    return;
  }

  i5Trace("Disabling HW STP for %s\n", ifname);

  /* Disable HW STP state for the interface */
  snprintf(cmd, sizeof(cmd), "ethswctl -c hwstp -i %s -o disable", ifname);
  system(cmd);

  /* Enable Softswitch for the interface */
  snprintf(cmd, sizeof(cmd), "ethswctl -c softswitch -i %s -o enable", ifname);
  system(cmd);

  ifr_node = i5GlueFindRealVlanIfr(ifname);
  if (ifr_node) {
    ifr_node->hwStpDisabled = TRUE;
  }
}

static void i5GlueEnableHWSTP(char *ifname)
{
  char cmd[512];

  i5TraceInfo("Enabling HW STP for %s\n", ifname);

  /* Enable HW STP state for the interface */
  snprintf(cmd, sizeof(cmd), "ethswctl -c hwstp -i %s -o enable", ifname);
  system(cmd);

  /* Disable Softswitch for the interface */
  snprintf(cmd, sizeof(cmd), "ethswctl -c softswitch -i %s -o disable", ifname);
  system(cmd);
}
