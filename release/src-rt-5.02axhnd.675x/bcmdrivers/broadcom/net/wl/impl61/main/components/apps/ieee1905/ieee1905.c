/*
 * Broadcom IEEE1905 library definitions
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * $Id: ieee1905.c 776889 2019-07-12 08:07:32Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "ieee1905.h"
#include "ieee1905_glue.h"
#include "ieee1905_interface.h"
#include "ieee1905_socket.h"
#include "ieee1905_json.h"
#include "ieee1905_message.h"
#include "ieee1905_netlink.h"
#include "ieee1905_security.h"
#include "ieee1905_trace.h"
#include "ieee1905_plc.h"
#include "ieee1905_ethstat.h"
#include "ieee1905_brutil.h"
#include "ieee1905_udpsocket.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_flowmanager.h"
#include "ieee1905_control.h"
#include "ieee1905_cmsutil.h"
#include "i5ctl.h"

#define I5_TRACE_MODULE i5TraceMain

#define I5_MAP_FLAG_QUERY_ASSOC_STA_METRICS     3
#define I5_MAP_FLAG_QUERY_UNASSOC_STA_METRICS   2
#define I5_MAP_FLAG_QUERY_BEACON_METRICS        1
#define I5_MAP_FLAG_QUERY_POLICY_CONFIG         4

char g_map_process_name[MAP_MAX_PROCESS_NAME];

/* Initialize the IEEE1905 */
int ieee1905_init(void *usched_hdl, unsigned int supServiceFlag, int isRegistrar,
  ieee1905_msglevel_t *msglevel, ieee1905_config *config)
{
  int rc = -1, idx;
  i5_dm_device_type *pdmdev;

  memset(&i5_config, 0, sizeof(i5_config_type));

  if (config->flags & I5_INIT_FLAG_MCHAN) {
    i5_config.flags |= I5_CONFIG_FLAG_MCHAN;
  }

  if (config->flags & I5_INIT_FLAG_GUEST_ENABLED) {
    i5_config.flags |= I5_CONFIG_FLAG_GUEST_ENABLED;

    i5_config.prim_vlan_id = config->prim_vlan_id;
    i5_config.sec_vlan_id = config->sec_vlan_id;
    i5_config.vlan_ether_type = config->vlan_ether_type;
    i5Trace("PrimVLANID[%d] SecVLANID[%d] VLAN ethertype[0x%04x]\n",
      i5_config.prim_vlan_id, i5_config.sec_vlan_id, i5_config.vlan_ether_type);
  }

  /* Set Process Name */
  memset(g_map_process_name, 0, sizeof(g_map_process_name));
  if (I5_IS_MULTIAP_CONTROLLER(supServiceFlag)) {
    snprintf(g_map_process_name, sizeof(g_map_process_name), "Controller");
  } else if (I5_IS_MULTIAP_AGENT(supServiceFlag)) {
    snprintf(g_map_process_name, sizeof(g_map_process_name), "Agent");
  }

  /* Update the message level */
  if (msglevel && msglevel->module) {
    for (idx = 0; idx < msglevel->module_count; idx++) {
      i5TraceSet(msglevel->module[idx], msglevel->level, msglevel->ifindex, msglevel->ifmacaddr);
    }
  }

#if defined(USEBCMUSCHED)
  if (usched_hdl == NULL) {
    printf("Micro Scheduler Handle Cannot be NULL\n");
    return -1;
  }
  i5_config.usched_hdl = (bcm_usched_handle*)usched_hdl;
#endif /* USEBCMUSCHED */

  if ( i5GlueMainInit(supServiceFlag) < 0 ) {
    i5TraceError("Main initialization error\n");
    goto end;
  }

  i5SocketInit(supServiceFlag);

#if defined(BRCM_CMS_BUILD)
  if ( i5CmsutilInit() < 0 ) {
    i5TraceError("CMS intialization error\n");
    goto end;
  }
#endif /* BRCM_CMS_BUILD */

  i5GlueLoadConfig(supServiceFlag, isRegistrar);

  if ( i5DmInit(supServiceFlag, isRegistrar, i5_config.device_mode) < 0 ) {
    i5TraceError("DM initialization error\n");
    goto end;
  }

 if (I5_IS_MULTIAP_AGENT(supServiceFlag)) {
   i5JsonInit();
 }

  i5NetlinkInit();

#if defined(SUPPORT_HOMEPLUG)
  i5PlcInitialize();
#endif /* SUPPORT_HOMEPLUG */

#if defined(IEEE1905_KERNEL_MODULE_PB_SUPPORT)
  i5UdpSocketInit();
#endif /* IEEE1905_KERNEL_MODULE_PB_SUPPORT */

  i5SecurityInit();

#if defined(WIRELESS)
  i5WlCfgInit();
#endif /* WIRELESS */

  i5EthStatInit();

  i5BrUtilInit();

  i5InterfaceInit();

  /* enable state or other settings may have changed during init */
  i5_config.running = 1;
  i5GlueSaveConfig();

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    goto end;
  }
  pdmdev->BasicCaps = config->basic_caps;

  rc = 0;
end:
  return rc;
}

/* De-Initialize the IEEE1905 */
void ieee1905_deinit()
{
  printf("Shutting down 1905\n");

  /* save configuration - specifically status */
  i5GlueSaveConfig();

#if defined(WIRELESS)
  i5WlCfgDeInit();
#endif /* WIRELESS */

#if defined(SUPPORT_IEEE1905_FM)
  i5FlowManagerDeinit();
#endif /* SUPPORT_IEEE1905_FM */

  i5BrUtilDeinit();

  i5MessageDeinit();

  i5DmDeinit();

  i5JsonDeinit();

#if defined(IEEE1905_KERNEL_MODULE_PB_SUPPORT)
  i5UdpSocketDeinit();
#endif /* IEEE1905_KERNEL_MODULE_PB_SUPPORT */

#if defined(BRCM_CMS_BUILD)
  i5CmsutilDeinit();
#endif /* BRCM_CMS_BUILD */

  i5SocketDeinit();

#if defined(MULTIAP)
  i5GlueMainDeinit();
#endif	/* MULTIAP */
}

/* Get the AL MAC address */
unsigned char *ieee1905_get_al_mac()
{
  return &i5_config.i5_mac_address[0];
}

/* Get the BSSID of the STA interface timeout callback */
static void i5GetBSSIDOfSTAInterfaceTimeout(void *arg)
{
  unsigned char *InterfaceId = (unsigned char*)arg;
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;
  ieee1905_ifr_info info;

  i5TraceInfo("Get BSSID of STA timeout\n");

  if (i5_config.ptmrGetBSSID) {
    i5TimerFree(i5_config.ptmrGetBSSID);
    i5_config.ptmrGetBSSID = NULL;
  }

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return;
  }

  if ((pdmif = i5DmInterfaceFind(pdmdev, InterfaceId)) == NULL) {
    i5TraceError("Interface " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(InterfaceId));
    return;
  }

  /* Get the interface info where we will get the BSSID of the bSTA interface */
  memset(&info, 0, sizeof(info));
  if (i5_config.cbs.get_interface_info) {
    if (i5_config.cbs.get_interface_info(pdmif->ifname, &info) != 0) {
      i5TraceError("Failed to get interface info["I5_MAC_DELIM_FMT"]\n",
        I5_MAC_PRM(pdmif->InterfaceId));
      goto end;
    }

    /* If it is STA interface check if the BSSID is NULL or not */
    if (info.mapFlags & IEEE1905_MAP_FLAG_STA) {
      if (i5DmIsMacNull(info.bssid)) {
        i5TraceInfo("MAP[%d] BSSID is NULL\n", info.mapFlags);
        goto end;
      }
    } else {
      /* If not STA interface just return */
      i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
      return;
    }

    /* Update the AP caps from any of the virtual BSS as primary interface is a STA */
    pdmbss = pdmif->bss_list.ll.next;
    if (pdmbss != NULL) {
      i5DmUpdateAPCaps(pdmbss->ifname, pdmif);
    } else {
      /* In case of dedicated backhaul. there wont be virtual BSS */
      i5DmUpdateAPCaps(pdmif->ifname, pdmif);
    }

    pdmif->band = (unsigned char)ieee1905_get_band_from_radiocaps(&pdmif->ApCaps.RadioCaps);
    memcpy(pdmif->bssid, info.bssid, sizeof(pdmif->bssid));
    i5WlCfgUpdateMediaInfo(pdmif->InterfaceId, info.chanspec, info.bssid, info.mapFlags);
    i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);

    return;
  }

end:
  i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
  /* Start the timer again to get the BSSID */
  i5_config.ptmrGetBSSID = i5TimerNew(I5_DM_GET_BSTA_BSSID_TIME_MSEC,
    i5GetBSSIDOfSTAInterfaceTimeout, (void*)pdmif->InterfaceId);
}

/* Start IEEE1905 Messaging */
void ieee1905_start()
{
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;
  ieee1905_ifr_info info;

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return;
  }

  /* Get the Wireless interface info and fill the info via callback */
  pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
  while (pdmif != NULL) {
    if (i5DmIsInterfaceWireless(pdmif->MediaType)) {
      memset(&info, 0, sizeof(info));
      if (i5_config.cbs.get_interface_info) {
        if (i5_config.cbs.get_interface_info(pdmif->ifname, &info) != 0) {
          i5TraceError("Failed to get interface info["I5_MAC_DELIM_FMT"]\n",
            I5_MAC_PRM(pdmif->InterfaceId));
          i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
          goto next;
        }
        pdmif->chanspec = info.chanspec;
        if (I5_IS_MULTIAP_AGENT(i5_config.flags)) {
          pdmif->mapFlags = info.mapFlags;
          i5DmCopyAPCaps(&pdmif->ApCaps, &info.ap_caps);

          /* If it is STA interface get BSSID where the bSTA is associated, to send it
           * as part of the device information TLV's media specific info
           */
          if (info.mapFlags & IEEE1905_MAP_FLAG_STA) {
            if (i5DmIsMacNull(info.bssid)) {
              i5TraceInfo("MAP[%d] BSSID is NULL\n", info.mapFlags);
              if (!i5_config.ptmrGetBSSID) {
                i5_config.ptmrGetBSSID = i5TimerNew(I5_DM_GET_BSTA_BSSID_TIME_MSEC,
                  i5GetBSSIDOfSTAInterfaceTimeout, (void*)pdmif->InterfaceId);
              }
            } else {
              memcpy(pdmif->bssid, info.bssid, sizeof(pdmif->bssid));

              /* Update the AP caps from any of the virtual BSS as primary interface is a STA */
              pdmbss = pdmif->bss_list.ll.next;
              if (pdmbss != NULL) {
                i5DmUpdateAPCaps(pdmbss->ifname, pdmif);
              }
            }
          }

          i5WlCfgUpdateMediaInfo(pdmif->InterfaceId, info.chanspec, info.bssid, info.mapFlags);
          pdmif->band = (unsigned char)ieee1905_get_band_from_radiocaps(&pdmif->ApCaps.RadioCaps);
        }

        /* Free radio caps memory */
        i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
      }
    }

next:
      pdmif = pdmif->ll.next;
  }

  i5_config.flags |= I5_CONFIG_FLAG_START_MESSAGE;
  if (i5_config.ptmrApSearch) {
    i5TimerFree(i5_config.ptmrApSearch);
  }

  i5_config.ptmrApSearch = i5TimerNew(I5_MESSAGE_AP_SEARCH_START_INTERVAL_MSEC,
    i5WlCfgMultiApControllerSearch, NULL);

#ifdef MULTIAP
  {
    int interval_ms = 0;
    /* Wait at least two discovery interval for discovery before removing a neighbor */
    if (i5_config.discovery_timeout != 0) {
      interval_ms = i5_config.discovery_timeout * 2;
    } else {
      interval_ms =  I5_MESSAGE_TOPOLOGY_DISCOVERY_PERIOD_MSEC * 2;
    }
    if (i5_config.ptmrRemoveStaleNeighbors) {
      i5TimerFree(i5_config.ptmrRemoveStaleNeighbors);
    }
    i5_config.ptmrRemoveStaleNeighbors = i5TimerNew(interval_ms,
      i5DmDeviceRemoveStaleNeighborsTimer, NULL);
  }
#endif // endif
}

/* Get Data model */
i5_dm_network_topology_type *ieee1905_get_datamodel()
{
  return &i5_dm_network_topology;
}

#ifdef MULTIAP
/* Register Callbacks */
void ieee1905_register_callbacks(ieee1905_call_bks_t *incbs)
{
  i5_dm_device_type *pdevice;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pbss;
  i5_dm_clients_type *pclient;

  i5_config.cbs.device_init = incbs->device_init;

  i5_config.cbs.device_deinit = incbs->device_deinit;

  i5_config.cbs.interface_init = incbs->interface_init;

  i5_config.cbs.interface_deinit = incbs->interface_deinit;

  i5_config.cbs.neighbor_init = incbs->neighbor_init;

  i5_config.cbs.neighbor_deinit = incbs->neighbor_deinit;

  i5_config.cbs.bss_init = incbs->bss_init;

  i5_config.cbs.bss_deinit = incbs->bss_deinit;

  i5_config.cbs.client_init = incbs->client_init;

  i5_config.cbs.client_deinit = incbs->client_deinit;

  i5_config.cbs.assoc_disassoc = incbs->assoc_disassoc;

  i5_config.cbs.steer_req = incbs->steer_req;

  i5_config.cbs.block_unblock_sta_req = incbs->block_unblock_sta_req;

  i5_config.cbs.prepare_channel_pref = incbs->prepare_channel_pref;

  i5_config.cbs.recv_chan_selection_req = incbs->recv_chan_selection_req;

  i5_config.cbs.send_opchannel_rpt = incbs->send_opchannel_rpt;

  i5_config.cbs.set_tx_power_limit = incbs->set_tx_power_limit;

  i5_config.cbs.backhaul_link_metric = incbs->backhaul_link_metric;

  i5_config.cbs.interface_metric = incbs->interface_metric;

  i5_config.cbs.ap_metric = incbs->ap_metric;

  i5_config.cbs.assoc_sta_metric = incbs->assoc_sta_metric;

  i5_config.cbs.unassoc_sta_metric = incbs->unassoc_sta_metric;

  i5_config.cbs.create_bss_on_ifr = incbs->create_bss_on_ifr;

  i5_config.cbs.get_interface_info = incbs->get_interface_info;

  i5_config.cbs.backhaul_steer_req = incbs->backhaul_steer_req;

  i5_config.cbs.beacon_metrics_query = incbs->beacon_metrics_query;

  i5_config.cbs.configure_policy = incbs->configure_policy;

  i5_config.cbs.process_vendor_specific_msg = incbs->process_vendor_specific_msg;

  i5_config.cbs.assoc_sta_metric_resp = incbs->assoc_sta_metric_resp;

  i5_config.cbs.unassoc_sta_metric_resp = incbs->unassoc_sta_metric_resp;

  i5_config.cbs.beacon_metric_resp = incbs->beacon_metric_resp;

  i5_config.cbs.ap_configured = incbs->ap_configured;

  i5_config.cbs.operating_chan_report = incbs->operating_chan_report;

  i5_config.cbs.get_vendor_specific_tlv = incbs->get_vendor_specific_tlv;

  i5_config.cbs.steering_btm_rpt = incbs->steering_btm_rpt;

  i5_config.cbs.higher_layer_data = incbs->higher_layer_data;

  i5_config.cbs.interface_chan_change = incbs->interface_chan_change;

  i5_config.cbs.ap_auto_config_resp = incbs->ap_auto_config_resp;

  i5_config.cbs.ap_auto_config_search_sent = incbs->ap_auto_config_search_sent;

  i5_config.cbs.set_bh_sta_params = incbs->set_bh_sta_params;

  /* Call the callbacks of device, interface, bss and client initialization if the initialization
   * happened before registering the callback
   */
  pdevice = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  /* For all the devices */
  while (pdevice != NULL) {
    if (i5_config.cbs.device_init) {
      i5_config.cbs.device_init(pdevice);
    }

    pdmif = (i5_dm_interface_type *)pdevice->interface_list.ll.next;
    /* For all the Interfaces */
    while (pdmif != NULL) {
      if (i5_config.cbs.interface_init) {
        i5_config.cbs.interface_init(pdmif);
      }

      pbss = (i5_dm_bss_type *)pdmif->bss_list.ll.next;
      /* For all the BSS */
      while (pbss != NULL) {
        if (i5_config.cbs.bss_init) {
          i5_config.cbs.bss_init(pbss);
        }

        pclient = (i5_dm_clients_type *)pbss->client_list.ll.next;
        /* For all the Clients */
        while (pclient != NULL) {
          if (i5_config.cbs.client_init) {
            i5_config.cbs.client_init(pclient);
          }

          pclient = pclient->ll.next;
        }

        pbss = pbss->ll.next;
      }

      pdmif = pdmif->ll.next;
    }

    pdevice = pdevice->ll.next;
  }
}

/* Add the BSS */
int ieee1905_add_bss(unsigned char *radio_mac, unsigned char *bssid, unsigned char *ssid,
  unsigned char ssid_len, unsigned short chanspec, char *ifname, unsigned char mapFlags)
{
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;

  /* If it is not agent dont add the BSS */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  /* Find the interface in the device */
  pdmif = i5DmInterfaceFind(pdmdev, radio_mac);
  if (pdmif == NULL) {
    i5TraceError("Interface " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(radio_mac));
    return IEEE1905_RADIO_NOT_FOUND;
  }
  pdmif->chanspec = chanspec;

  /* Find the BSS in the device */
  pdmbss = i5DmFindBSSFromInterface(pdmif, bssid);
  if (pdmbss != NULL) {
    i5TraceError("BSS " I5_MAC_FMT "Already Exists\n", I5_MAC_PRM(bssid));
    return IEEE1905_OK;
  }

  if ((pdmbss = i5DmBSSNew(pdmif, bssid, ssid, ssid_len, mapFlags)) == NULL) {
    i5TraceError("Failed to Add BSS " I5_MAC_FMT "In Interface " I5_MAC_FMT "\n", I5_MAC_PRM(bssid),
      I5_MAC_PRM(radio_mac));
    return IEEE1905_FAIL;
  }
  memcpy(pdmbss->ifname, ifname, sizeof(pdmbss->ifname));
  pdmbss->mapFlags = mapFlags;

  /* Dont update media info from virtual BSS */
  if (!i5WlCfgIsVirtualInterface(pdmbss->ifname)) {
    i5WlCfgUpdateMediaInfo(radio_mac, chanspec, bssid, mapFlags);
  }

  /* If the primary interface is STA. then the AP capablities should be from any of the virtual
   * BSS. Primary interface gives the capability of the backhaul BSS where the STA is associated
   */
  if (pdmif->mapFlags & IEEE1905_MAP_FLAG_STA) {
    i5DmUpdateAPCaps(pdmbss->ifname, pdmif);
  }

  return IEEE1905_OK;
}

/* Add the BSS to Controller table. only required in controller */
int ieee1905_add_bssto_controller_table(ieee1905_client_bssinfo_type *bss)
{
  ieee1905_client_bssinfo_type *clientbss = NULL;

  /* If it is not controller dont add the BSS */
  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Controller functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_CONTROLLER_NOT_FOUND;
  }

  clientbss = (ieee1905_client_bssinfo_type *)malloc(sizeof(*clientbss));
  if (!clientbss) {
    i5TraceDirPrint("Malloc Failed\n");
    return IEEE1905_FAIL;
  }

  memset(clientbss, 0, sizeof(*clientbss));
  memcpy(clientbss->ALID, bss->ALID, sizeof(clientbss->ALID));
  clientbss->band_flag = bss->band_flag;
  memcpy(&clientbss->ssid, &bss->ssid, sizeof(clientbss->ssid));
  clientbss->AuthType = bss->AuthType;
  clientbss->EncryptType = bss->EncryptType;
  memcpy(&clientbss->NetworkKey, &bss->NetworkKey, sizeof(clientbss->NetworkKey));
  clientbss->BackHaulBSS = bss->BackHaulBSS;
  clientbss->FrontHaulBSS = bss->FrontHaulBSS;
  clientbss->Guest = bss->Guest;
  ieee1905_glist_append(&i5_config.client_bssinfo_list, (dll_t*)clientbss);

  i5TraceInfo("ALID["I5_MAC_DELIM_FMT"] band_flag[0x%x] ssid_len[%d] ssid[%s] auth[0x%x] "
        "encr[0x%x] pwd_len[%d] Password[%s] bkhaul[%d] frnthaul[%d] Guest[%d]\n",
        I5_MAC_PRM(clientbss->ALID), clientbss->band_flag, clientbss->ssid.SSID_len,
        clientbss->ssid.SSID, clientbss->AuthType, clientbss->EncryptType,
        clientbss->NetworkKey.key_len, clientbss->NetworkKey.key, clientbss->BackHaulBSS,
        clientbss->FrontHaulBSS, clientbss->Guest);

  return IEEE1905_OK;
}

/* STA has Associated or Disassociated */
int ieee1905_sta_assoc_disassoc(unsigned char *bssid, unsigned char *mac, int isAssoc,
  unsigned short time_elapsed, unsigned char notify, unsigned char *assoc_frame,
  unsigned int assoc_frame_len)
{
  i5_dm_device_type *pdmdev;

  /* If it is not agent dont add the BSS */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (isAssoc) {
    return i5DmAssociateClient(NULL, pdmdev, bssid, mac, time_elapsed, notify, assoc_frame,
      assoc_frame_len);
  } else {
    return i5DmDisAssociateClient(pdmdev, bssid, mac, notify);
  }

  return IEEE1905_OK;
}

/* Send the BTM report to the controller */
int ieee1905_send_btm_report(ieee1905_btm_report *btm_report)
{
  i5_dm_device_type *pDeviceController = i5DmDeviceFind(btm_report->neighbor_al_mac);

  if (pDeviceController == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Controller not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_CONTROLLER_NOT_FOUND;
  }

  /* If it is not agent dont send BTM Report */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  /* Send BTM report only for steer mandate steering request */
  if (!IEEE1905_IS_STEER_MANDATE(btm_report->request_flags)) {
    i5TraceError("BTM Report only for steer mandate request from controller\n");
    return IEEE1905_FAIL;
  }

  i5MessageClientSteeringBTMReportSend(pDeviceController->psock, pDeviceController->DeviceId,
    btm_report);

  return IEEE1905_OK;
}

/* Send steering completed message to the controller */
int ieee1905_send_steering_completed_message(ieee1905_steer_req *steer_req)
{
  i5_dm_device_type *pDeviceController = i5DmDeviceFind(steer_req->neighbor_al_mac);

  if (pDeviceController == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Controller not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_CONTROLLER_NOT_FOUND;
  }

  /* If it is not agent dont send steering completed message */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  /* Send steering completed message only for steer opportunity steering request */
  if (!IEEE1905_IS_STEER_OPPORTUNITY(steer_req->request_flags)) {
    i5TraceError("Steering completed message only for steer opportunity request from controller\n");
    return IEEE1905_FAIL;
  }

  i5TraceInfo("Sending Steering Completed message\n");
  i5MessageSteeringCompletedSend(pDeviceController->psock, pDeviceController->DeviceId);

  return IEEE1905_OK;
}

/* Send the Client assocaition control messages to all the BSS except the source and target */
int ieee1905_send_client_association_control(ieee1905_client_assoc_cntrl_info *assoc_cntrl)
{
  return i5MessagePrepareandSendClientAssociationControl(assoc_cntrl);
}

/* Check if the STA is in BTM Steering Disallowed STA list */
int ieee1905_is_sta_in_BTM_steering_disallowed_list(unsigned char *mac)
{
  return i5DmIsMACInList(&i5_config.policyConfig.no_btm_steer_sta_list, mac);
}

/* Check if the STA is in Local Steering Disallowed STA List */
int ieee1905_is_sta_in_local_steering_disallowed_list(unsigned char *mac)
{
  return i5DmIsMACInList(&i5_config.policyConfig.no_steer_sta_list, mac);
}

/* Send backhaul steering response */
int ieee1905_send_bh_steering_repsonse(ieee1905_backhaul_steer_msg *bh_steer_resp)
{
  i5_dm_device_type *pDeviceController = i5DmDeviceFind(bh_steer_resp->neighbor_al_mac);

  if (pDeviceController == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Controller not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_CONTROLLER_NOT_FOUND;
  }

  /* If it is not agent dont send BTM Report */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  i5MessageBackhaulSteeringResponseSend(pDeviceController->psock, pDeviceController->DeviceId,
    bh_steer_resp);

  return IEEE1905_OK;
}

/* Send the associated STA link metrics to the requested neighbor */
int ieee1905_send_assoc_sta_link_metric(unsigned char *neighbor_al_mac, unsigned char *sta_mac)
{
  i5_dm_device_type *pDeviceNeighbor = i5DmDeviceFind(neighbor_al_mac);
  i5_dm_device_type *pDevice = i5DmGetSelfDevice();
  i5_dm_clients_type *pclient;

  if (pDevice == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT " not found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  pclient = i5DmFindClientInDevice(pDevice, sta_mac);
  if (pclient == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Client["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(sta_mac));
    return IEEE1905_STA_NOT_FOUND;
  }

  clock_gettime(CLOCK_REALTIME, &pclient->link_metric.queried);

  i5_config.last_message_identifier++;
  i5MessageAssociatedSTALinkMetricsResponseSend(pDeviceNeighbor->psock,
    pDeviceNeighbor->DeviceId, i5_config.last_message_identifier, sta_mac, 1);

  return IEEE1905_OK;
}

/* Send the Vendor Specific Message to the requested neighbor from Application */
int ieee1905_send_vendor_specific_msg(ieee1905_vendor_data *msg_data)
{
  i5_dm_device_type *pDevice = i5DmDeviceFind(msg_data->neighbor_al_mac);

  if (pDevice == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(msg_data->neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  return i5MessageVendorSpecificMessageSend(pDevice->psock, pDevice->DeviceId, msg_data);
}

/* Send the Un associated STA link metrics to the requested neighbor */
int ieee1905_send_unassoc_sta_link_metric(ieee1905_unassoc_sta_link_metric *metrics)
{
  i5_dm_device_type *pDevice = i5DmDeviceFind(metrics->neighbor_al_mac);

  if (pDevice == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(metrics->neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  /* If it is not agent dont send Unassociated STA link metrics */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  i5MessageUnAssociatedSTALinkMetricsResponseSend(pDevice->psock, pDevice->DeviceId, metrics);

  return IEEE1905_OK;
}

/* Send the beacon report */
int ieee1905_send_beacon_report(ieee1905_beacon_report *report)
{
  i5_dm_device_type *pDevice = i5DmDeviceFind(report->neighbor_al_mac);

  if (pDevice == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(report->neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  /* If it is not agent dont send beacon report */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  i5MessageBeaconMetricsResponseSend(pDevice->psock, pDevice->DeviceId, report);

  return IEEE1905_OK;
}

/* Send Query message to the neighbor. Flags of type I5_MAP_FLAG_QUERY_XXX tells which
 * query to be send
 */
static int ieee1905_send_common_query(unsigned char *neighbor_al_mac, void *data,
  unsigned int flags)
{
  i5_dm_device_type *pDeviceNeighbor = i5DmDeviceFind(neighbor_al_mac);
  i5_dm_device_type *pDevice = i5DmGetSelfDevice();

  if (pDevice == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT " not found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (flags == I5_MAP_FLAG_QUERY_ASSOC_STA_METRICS) {
    i5MessageAssociatedSTALinkMetricsQuerySend(pDeviceNeighbor->psock, neighbor_al_mac,
      (unsigned char*)data);
  } else if (flags == I5_MAP_FLAG_QUERY_UNASSOC_STA_METRICS) {
    i5MessageUnAssociatedSTALinkMetricsQuerySend(pDeviceNeighbor->psock, neighbor_al_mac,
      (ieee1905_unassoc_sta_link_metric_query*)data);
  } else if (flags == I5_MAP_FLAG_QUERY_BEACON_METRICS) {
    i5MessageBeaconMetricsQuerySend(pDeviceNeighbor->psock, neighbor_al_mac,
      (ieee1905_beacon_request*)data);
  } else if (flags == I5_MAP_FLAG_QUERY_POLICY_CONFIG) {
    i5MessageMultiAPPolicyConfigRequestSend(pDeviceNeighbor->psock, neighbor_al_mac);
  }

  return IEEE1905_OK;
}

/* Send Associated STA Link Metrics Query message */
int ieee1905_send_assoc_sta_link_metric_query(unsigned char *neighbor_al_mac,
  unsigned char *sta_mac)
{
  return ieee1905_send_common_query(neighbor_al_mac, sta_mac, I5_MAP_FLAG_QUERY_ASSOC_STA_METRICS);
}

/* Send UnAssociated STA Link Metrics Query message */
int ieee1905_send_unassoc_sta_link_metric_query(unsigned char *neighbor_al_mac,
  ieee1905_unassoc_sta_link_metric_query *query)
{
  return ieee1905_send_common_query(neighbor_al_mac, query, I5_MAP_FLAG_QUERY_UNASSOC_STA_METRICS);
}

/* Send Beacon Metrics Query message */
int ieee1905_send_beacon_metrics_query(unsigned char *neighbor_al_mac,
  ieee1905_beacon_request *query)
{
  return ieee1905_send_common_query(neighbor_al_mac, query, I5_MAP_FLAG_QUERY_BEACON_METRICS);
}

/* Add the Metric Reporting Policy for a radio */
int ieee1905_add_metric_reporting_policy_for_radio(unsigned char ap_rpt_intvl,
  ieee1905_ifr_metricrpt *metricrptIn)
{
  ieee1905_ifr_metricrpt *metricrpt;

  /* AP metric reporting policy should be added only to the controller */
  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Controller functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_CONTROLLER;
  }

  i5_config.policyConfig.metricrpt_config.ap_rpt_intvl = ap_rpt_intvl;

  if (metricrptIn == NULL) {
    goto end;
  }

  /* If the policy already found, just update the values else add the new one */
  if ((metricrpt = i5DmFindMetricReportPolicy(metricrptIn->mac)) == NULL) {
    metricrpt = (ieee1905_ifr_metricrpt*)malloc(sizeof(*metricrpt));
    if (!metricrpt) {
      i5TraceError("Failed to allocate memory for metric report policy for " I5_MAC_FMT "\n",
        I5_MAC_PRM(metricrptIn->mac));
      return IEEE1905_FAIL;
    }
    memset(metricrpt, 0, sizeof(*metricrpt));
    memcpy(metricrpt->mac, metricrptIn->mac, sizeof(metricrpt->mac));
    ieee1905_glist_append(&i5_config.policyConfig.metricrpt_config.ifr_list, (dll_t*)metricrpt);
  }

  /* Update the values */
  metricrpt->sta_mtrc_rssi_thld =  metricrptIn->sta_mtrc_rssi_thld;
  metricrpt->sta_mtrc_rssi_hyst =  metricrptIn->sta_mtrc_rssi_hyst;
  metricrpt->ap_mtrc_chan_util =  metricrptIn->ap_mtrc_chan_util;
  metricrpt->sta_mtrc_policy_flag =  metricrptIn->sta_mtrc_policy_flag;

end:
  return IEEE1905_OK;
}

void ieee1905_send_channel_preference_query( unsigned char *neighbor_al_mac_address)
{
  i5_dm_device_type *pDeviceNeighbor = i5DmDeviceFind(neighbor_al_mac_address);
  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(neighbor_al_mac_address));
    return;
  }
  i5MessageChannelPreferenceQuerySend(pDeviceNeighbor->psock, neighbor_al_mac_address);
}

void ieee1905_send_message(void *pmsg)
{
  i5TlvEndOfMessageTypeInsert((i5_message_type *)pmsg);
  i5MessageSend((i5_message_type *)pmsg, 0);
  i5MessageFree((i5_message_type *)pmsg);
}

void *ieee1905_create_channel_selection_request( unsigned char *neighbor_al_mac_address)
{
  i5_dm_device_type *pDeviceNeighbor = i5DmDeviceFind(neighbor_al_mac_address);
  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(neighbor_al_mac_address));
    return NULL;
  }
  return (void *)i5MessageChannelSelectionRequestCreate(pDeviceNeighbor->psock, neighbor_al_mac_address);
}

void ieee1905_insert_channel_selection_request_tlv(void *pmsg,
  unsigned char *radio_mac, ieee1905_chan_pref_rc_map_array *chan_pref)
{
  if (pmsg && radio_mac && chan_pref && chan_pref->rc_map) {
       i5Trace("i5TlvChannelPreferenceTypeInsert_Stored\n");
       i5TlvChannelPreferenceTypeInsert_Stored((i5_message_type *)pmsg, radio_mac, chan_pref);
  }
  return;
}

void ieee1905_insert_vendor_message_tlv(void *pmsg, ieee1905_vendor_data *vndr_msg)
{

  if (!pmsg) {
    i5Trace(" Invalid msg while adding vendor message TLV");
    goto fail;
  }

  if (vndr_msg && (vndr_msg->vendorSpec_len > 0)) {
    i5Trace("Add Vendor Specific TLV in message");
    i5TlvVendorSpecificTypeInsert(pmsg, vndr_msg->vendorSpec_msg,
      vndr_msg->vendorSpec_len);
  }

fail:
  return;
}

/* Send Policy Configuration to neighbor */
int ieee1905_send_policy_config(unsigned char *neighbor_al_mac)
{
  return ieee1905_send_common_query(neighbor_al_mac, NULL, I5_MAP_FLAG_QUERY_POLICY_CONFIG);
}

void ieee1905_send_ap_autoconfig_renew(unsigned char *neighbor_al_mac_address)
{
  i5MessageApAutoconfigurationRenewSend(0);
}

/* Send operating channel report to controller */
int ieee1905_send_operating_chan_report(ieee1905_operating_chan_report *chan_rpt)
{
  i5_dm_device_type *pDeviceController = NULL;
  i5_dm_device_type *pdevice = NULL;

  pdevice = i5DmGetSelfDevice();

  if (!pdevice) {
    i5TraceError("Local device does not exist\n");
    return -1;
  }
  if (!chan_rpt) {
    i5TraceError("Invalid argument to prepare operating channel report \n");
    return -1;
  }
  pDeviceController = i5DmFindController();
  if (!pDeviceController) {
    i5TraceError("controller device does not exist\n");
    return -1;
  }

  i5MessageOperatingChanReportSend(pDeviceController->psock, pDeviceController->DeviceId, chan_rpt);
  return 0;
}

/* Send channel preference report to controller */
int ieee1905_send_chan_preference_report()
{
  i5_dm_device_type *pDeviceController = NULL;
  i5_dm_device_type *pdevice = NULL;

  pdevice = i5DmGetSelfDevice();

  if (!pdevice) {
    i5TraceError("Local device does not exist\n");
    return -1;
  }
  pDeviceController = i5DmFindController();
  if (!pDeviceController) {
    i5TraceError("controller device does not exist\n");
    return -1;
  }

  i5MessageChannelPreferenceReportSend(pDeviceController->psock, pDeviceController->DeviceId,
    i5_config.last_message_identifier, FALSE);
  return 0;
}

/* To inform ieee1905 about association of bSTA to the backhaul AP */
int ieee1905_bSTA_associated_to_backhaul_ap(unsigned char *InterfaceId)
{
  i5_dm_device_type *pdevice = i5DmGetSelfDevice();
  i5_dm_interface_type *pdmif;
  int do_renew = 0;

  /* Start the auto configuration process again */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  if (pdevice == NULL) {
    i5TraceError("NO device found\n");
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  /* Update the BSSID of the BSS where the STA is connected */
  i5GetBSSIDOfSTAInterfaceTimeout(InterfaceId);

  /* Dont do renew if there is 0 configured radio */
  pdmif = (i5_dm_interface_type *)pdevice->interface_list.ll.next;
  while (pdmif != NULL) {
    if (i5DmIsInterfaceWireless(pdmif->MediaType) && !i5WlCfgIsVirtualInterface(pdmif->ifname)) {
      if (pdmif->isConfigured == 1) {
        do_renew = 1;
        break;
      }
    }
    pdmif = pdmif->ll.next;
  }

  if (do_renew == 0) {
    return IEEE1905_OK;
  }

  /* make all the interface as not configured */
  i5WlcfgMarkAllInterfacesUnconfigured();

  i5WlCfgMultiApControllerSearch(NULL);

  return IEEE1905_OK;
}

/* Send neighbor link metrics query */
int ieee1905_send_neighbor_link_metric_query(unsigned char *neighbor_al_mac,
  unsigned char specify_neighbor, unsigned char *neighbor_of_recv_device)
{
  i5_dm_device_type *pDeviceNeighbor = i5DmDeviceFind(neighbor_al_mac);

  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  i5MessageLinkMetricQuerySend(pDeviceNeighbor->psock, neighbor_al_mac, specify_neighbor,
    neighbor_of_recv_device);

  return 0;
}

/* Send Ap Metrics Query. BSSIDs stored in a linear array. 6 octets for each BSSID */
int ieee1905_send_ap_metrics_query(unsigned char *neighbor_al_mac,
  unsigned char *bssids, unsigned char bssid_count)
{
  i5_dm_device_type *pDeviceNeighbor = i5DmDeviceFind(neighbor_al_mac);

  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  i5MessageAPMetricsQuerySend(pDeviceNeighbor->psock, neighbor_al_mac, bssids, bssid_count);

  return IEEE1905_OK;
}

/* Add STA MAC addresses to Local Steering Disallowed List */
int ieee1905_add_mac_to_local_steering_disallowed_list(unsigned char *macs,
  unsigned char mac_count)
{
  int i;

  /* Add MAC to the list one by one */
  for (i = 0; i < mac_count; i++) {
    ieee1905_sta_list *sta = (ieee1905_sta_list*)malloc(sizeof(*sta));
    if (!sta) {
      i5TraceDirPrint("malloc error while adding STA to steering disallowed list\n");
      return -1;
    }

    memcpy(sta->mac, &macs[i * MAC_ADDR_LEN], MAC_ADDR_LEN);
    ieee1905_glist_append(&i5_config.policyConfig.no_steer_sta_list, (dll_t*)sta);
  }

  return 0;
}

/* Function to get radio's RF bands supported in radio capbilities */
int
ieee1905_get_band_from_radiocaps(ieee1905_radio_caps_type *RadioCaps)
{
	int rc_count;
	int band_flag = BAND_INV;
	int i, j, rcidx = 0;
	uint8 rc = 0;
	int ch_count = 0;
	const i5_dm_rc_chan_map_type *rc_chan_map = i5DmGetRegClassChannelMap();

	if (!RadioCaps || !RadioCaps->List) {
		i5TraceError("Rabio capabilities NULL\n");
		goto end;
	}
	/* First find a regulatory class with valid channels and get its band */
	rc_count = RadioCaps->List ? RadioCaps->List[rcidx++]: 0;
	for (j = 0; j < rc_count && rcidx < RadioCaps->Len; j++) {
		const i5_dm_rc_chan_map_type *map_ptr = NULL;
		int chan_idx_start, chan_idx_end;
		uint8 count = 0;
		const uint8 *chan_list = NULL;
		bool invalid = FALSE;

		/* Get regulatory class from radio caps and check if it is valid */
		rc = RadioCaps->List[rcidx++];
    for (i = 0; i < REGCLASS_MAX_COUNT; i++) {
      if (rc_chan_map[i].regclass == rc) {
        break;
      }
    }
    if (i == REGCLASS_MAX_COUNT) {
      i5TraceError("Invalid regulatory class %d in RadioCaps\n", rc);
      band_flag = BAND_INV;
      goto end;
    }

    map_ptr = &rc_chan_map[i];
		rcidx++; /* Max Transmit power */
		ch_count = RadioCaps->List[rcidx++];
		if (ch_count == 0) {
			/* Allchannels are valid, check band of all channels */
			i5TraceInfo("All the channels are valid in Regclass: %d\n", rc);
			for (i = 0; i < map_ptr->count; i++) {
				band_flag |= i5WlCfgChannelToband(map_ptr->channel[i]);
			}
			continue;
		}

		count = map_ptr->count;
		chan_list = map_ptr->channel;
		/* Some channels in the current regulatory class is not valid.
		 * Get band of all valid channels
		 */
		chan_idx_start = rcidx;
		chan_idx_end = rcidx + ch_count;

		for (i = 0; i < count; i++) {
			invalid = FALSE;
			for (rcidx = chan_idx_start; rcidx < chan_idx_end; rcidx++) {
				if (chan_list[i] == RadioCaps->List[rcidx]) {
					i5TraceInfo("channel %d is invalid in rclass %d\n",
						chan_list[i], rc);
					invalid = TRUE;
					break;
				}
			}
			if (!invalid) {
				i5TraceInfo("channel %d is valid in rclass %d, find its band\n",
					chan_list[i], rc);
				band_flag |= i5WlCfgChannelToband(chan_list[i]);
			}
		}
		rcidx = chan_idx_end;

	}
end:
	return band_flag;
}

/* Function to get notified on channel change */
void
ieee1905_notify_channel_change(i5_dm_interface_type *pdmif)
{
	i5_dm_device_type *pdevice;

	if (!pdmif) {
		i5TraceError("Interface structure NULL\n");
		return;
	}

	i5WlCfgUpdateMediaInfo(pdmif->InterfaceId, pdmif->chanspec, pdmif->bssid, pdmif->mapFlags);

	/* Modify hasChanged to send topology notification */
	pdevice = I5LL_PARENT(pdmif);
	if ( i5DmDeviceIsSelf(pdevice->DeviceId) ) {
		pdevice->hasChanged++;
	}
}

/* Send Unsolicitated AP Metrics Response */
int
ieee1905_send_ap_metrics_response(unsigned char *neighbor_al_mac, unsigned char *ifr_mac)
{
  i5_dm_device_type *pDeviceNeighbor = i5DmDeviceFind(neighbor_al_mac);

  if (pDeviceNeighbor == NULL) {
    i5TraceError("Neighbor["I5_MAC_DELIM_FMT"] not found\n", I5_MAC_PRM(neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  i5_config.last_message_identifier++;
  i5MessageAPMetricsResponseSend(pDeviceNeighbor->psock, pDeviceNeighbor->DeviceId,
    i5_config.last_message_identifier, NULL, 0, ifr_mac, 1);

  return IEEE1905_OK;
}

/* Send Backhaul Steering Request */
int
ieee1905_send_backhaul_steering_request(unsigned char *neighbor_al_mac,
  ieee1905_backhaul_steer_msg *bh_steer_req)
{
  int ret;
  i5_dm_device_type *pDeviceNeighbor = i5DmDeviceFind(neighbor_al_mac);

  if (pDeviceNeighbor == NULL) {
    i5TraceError("Neighbor["I5_MAC_DELIM_FMT"] not found\n", I5_MAC_PRM(neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (pDeviceNeighbor->psock == NULL) {
    i5TraceError("Neighbor["I5_MAC_DELIM_FMT"] psock is NULL\n", I5_MAC_PRM(neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  ret = i5MessageBackhaulSteeringRequestSend(pDeviceNeighbor->psock, neighbor_al_mac, bh_steer_req);

  return ret;
}
#endif /* MULTIAP */
