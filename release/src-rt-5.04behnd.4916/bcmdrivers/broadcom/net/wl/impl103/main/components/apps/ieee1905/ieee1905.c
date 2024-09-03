/*
 * Broadcom IEEE1905 library definitions
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
 * $Id: ieee1905.c 836368 2024-02-12 07:46:39Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <syslog.h>
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
#include "ieee1905_vendor.h"
#include <bcmnvram.h>
#include "shutils.h"
#include "wlif_utils.h"

#define I5_TRACE_MODULE i5TraceMain

#define I5_MAP_FLAG_QUERY_ASSOC_STA_METRICS     3
#define I5_MAP_FLAG_QUERY_UNASSOC_STA_METRICS   2
#define I5_MAP_FLAG_QUERY_BEACON_METRICS        1
#define I5_MAP_FLAG_QUERY_POLICY_CONFIG         4

/*  If HLE wants to receive HLD on different port, HLE has to register port number and protocol's
 *  {port number, protocol count,  {protocol's}}
 *  Each port number register with number of protocol ID
 *  Based on received protocol id which is register on each port will send data to HLE.
 */

char g_map_process_name[MAP_MAX_PROCESS_NAME];
char *g_ifnames_list = NULL;

extern void i5VendorInit();
extern void i5VendorDeinit();
extern void i5VendorInformMessageSend(const unsigned char *dst_al_mac, const void *pmsg,
  const i5_message_types_t message_type, const void *reserved);

/* Copy the traffic separation policy to i5_config.policyConfig.ts_policy_list */
static int copy_ts_policy_config(ieee1905_config *config)
{
  ieee1905_ts_policy_t *ts_policy, *iter_policy;
  ieee1905_ssid_list_type *iter_ssid;
  dll_t *item_p;
  dll_t *ssid_item_p;

   /* Clean up before filling the latest data. */
  i5DmTSPolicyCleanup(&i5_config.policyConfig.ts_policy_list);
  ieee1905_glist_init(&i5_config.policyConfig.ts_policy_list);

  /* For All VLANID's */
  for (item_p = dll_head_p(&config->ts_policy_list.head);
    !dll_end(&config->ts_policy_list.head, item_p);
    item_p = dll_next_p(item_p)) {
    iter_policy = (ieee1905_ts_policy_t *)item_p;

    i5Trace("For VLANID[%d]\n", iter_policy->vlan_id);
    ts_policy = i5DmAddVLANIDToList(&i5_config.policyConfig.ts_policy_list, iter_policy->vlan_id);
    if (ts_policy == NULL) {
      return -1;
    }

    /* For all the SSID's in the VLAN ID */
    for (ssid_item_p = dll_head_p(&iter_policy->ssid_list.head);
      !dll_end(&iter_policy->ssid_list.head, ssid_item_p);
      ssid_item_p = dll_next_p(ssid_item_p)) {
      iter_ssid = (ieee1905_ssid_list_type *)ssid_item_p;
      i5Trace("For SSID[%s]\n", iter_ssid->ssid.SSID);
      if (i5DmAddSSIDToList(&i5_config.policyConfig.ts_policy_list, &ts_policy->ssid_list,
        &iter_ssid->ssid) == NULL) {
        return -1;
      }
    }
  }

  return 0;
}

/* Initialize the global object i5_config & its glists by 1905 library before anything else */
static void ieee1905_startup(void) __attribute__((constructor));
static void ieee1905_startup(void)
{
  memset(&i5_config, 0, sizeof(i5_config_type));
  ieee1905_glist_init(&i5_config.vlan_ifr_list);
  ieee1905_glist_init(&(i5_config.policyConfig.no_steer_sta_list));
  ieee1905_glist_init(&i5_config.policyConfig.no_btm_steer_sta_list);
  ieee1905_glist_init(&i5_config.policyConfig.steercfg_bss_list);
  ieee1905_glist_init(&i5_config.policyConfig.metricrpt_config.ifr_list);
  ieee1905_glist_init(&i5_config.policyConfig.ts_policy_list);
  ieee1905_glist_init(&i5_config.policyConfig.no_bh_bss_list);
  ieee1905_glist_init(&i5_config.policyConfig.qosmgmt_config.mscs_disallowed_sta_list);
  ieee1905_glist_init(&i5_config.policyConfig.qosmgmt_config.scs_disallowed_sta_list);
  ieee1905_glist_init(&i5_config.err_code_list);
  ieee1905_glist_init(&i5_config.client_bssinfo_list);
  ieee1905_glist_init(&i5_config.dpp_config_resp_str_list);
  ieee1905_glist_init(&i5_config.dpp_config_objs_list);
  ieee1905_glist_init(&i5_config.hld_port_protocol_set);
  ieee1905_glist_init(&i5_dm_network_topology.agent_ap_mld_conf);
}

/* Load the traffic separation policy to i5_config.policyConfig.ts_policy_list */
void
ieee1905_load_ts_policy_config(ieee1905_config *config)
{
  /* Initialize flags and variables of traffic separation policy config. This function is called
   * at init time as well as to reload the configuration
   */
  i5DmTSPolicyCleanup(&i5_config.policyConfig.ts_policy_list);
  i5_config.flags &= ~I5_CONFIG_FLAG_TS_SUPPORTED;
  i5_config.flags &= ~I5_CONFIG_FLAG_TS_USING_IPTABLE;
  i5_config.vlan_ether_type = 0;
  i5_config.flags &= ~I5_CONFIG_FLAG_TS_ACTIVE;
  i5_config.policyConfig.prim_vlan_id = 0;
  i5_config.policyConfig.default_pcp = 0;

  ieee1905_glist_init(&i5_config.policyConfig.ts_policy_list);

  /* If Traffic separation is supported */
  if (config->flags & I5_INIT_FLAG_TS_SUPPORTED) {
    i5_config.flags |= I5_CONFIG_FLAG_TS_SUPPORTED;

    /* Check if the traffic separation is using iptable or normal vlanctl */
    if (config->flags & I5_INIT_FLAG_TS_USING_IPTABLE) {
      i5_config.flags |= I5_CONFIG_FLAG_TS_USING_IPTABLE;
    }

    /* This may be required if the primary VLAN ID is enabled at run time on repeater */
    i5_config.vlan_ether_type = config->vlan_ether_type;

    /* Check if the traffic separation policy is present or not */
    if (config->flags & I5_INIT_FLAG_TS_ACTIVE) {
      i5_config.flags |= I5_CONFIG_FLAG_TS_ACTIVE;
      i5_config.policyConfig.prim_vlan_id = config->prim_vlan_id;
      i5_config.policyConfig.default_pcp = config->default_pcp;

      if (copy_ts_policy_config(config) == -1) {
        i5DmTSPolicyCleanup(&i5_config.policyConfig.ts_policy_list);
        ieee1905_glist_init(&i5_config.policyConfig.ts_policy_list);
      }

      i5Trace("PrimVLANID[%d] PCP[%d] VLAN ethertype[0x%04x]\n",
        i5_config.policyConfig.prim_vlan_id, i5_config.policyConfig.default_pcp,
        i5_config.vlan_ether_type);
    }
  }
}

/* Initialize the IEEE1905 */
int ieee1905_init(void *usched_hdl, unsigned int supServiceFlag, int isRegistrar,
  ieee1905_msglevel_t *msglevel, ieee1905_config *config)
{
  int rc = -1, idx;
  i5_dm_device_type *pdmdev;

  ieee1905_register_callbacks(config->cbs);

  i5_config.map_profile = config->map_profile;
  if (config->flags & I5_INIT_FLAG_DONT_UPDATE_MEDIA_INFO) {
    i5_config.flags |= I5_CONFIG_FLAG_DONT_UPDATE_MEDIA_INFO;
  }

  /* Dont split vendor message */
  if(!strtoul(nvram_safe_get("map_no_large_vendr_msg_split"), NULL, 0)) {
    i5_config.flags |= I5_CONFIG_FLAG_SPLIT_VNDR_MSG;
  }
  if (config->flags & I5_INIT_FLAG_DPP_ENABLED) {
    i5_config.flags |= I5_CONFIG_FLAG_DPP_ENABLED;
  }

  i5DmGlistCleanup(&i5_config.hld_port_protocol_set);
  ieee1905_glist_init(&i5_config.hld_port_protocol_set);
  ieee1905_add_hld_port_protocol(EAPD_WKSP_WBD_EVENT_PORT, i5TlvHigherLayerProtocol_TR181Transport);
  ieee1905_add_hld_port_protocol(EAPD_WKSP_HLD_QMD_PORT, i5TlvHigherLayerProtocol_QMD);

  ieee1905_load_ts_policy_config(config);

  /* Set Process Name */
  memset(g_map_process_name, 0, sizeof(g_map_process_name));
  if (I5_IS_MULTIAP_CONTROLLER(supServiceFlag)) {
    snprintf(g_map_process_name, sizeof(g_map_process_name), "Controller");
  } else if (I5_IS_MULTIAP_AGENT(supServiceFlag)) {
    snprintf(g_map_process_name, sizeof(g_map_process_name), "Agent");
  }

  /* IEEE1905 logs in syslogd enabled or not */
  if (config->flags & I5_INIT_FLAG_LOG_IN_SYSLOGD_INFO) {
	/* open syslog. explicit opening is optional; helps add ident,
	 * pid prefixes and set flags
	 */
	openlog((const char *)g_map_process_name, LOG_ODELAY, LOG_USER);

	i5_config.flags |= I5_CONFIG_FLAG_LOG_IN_SYSLOGD_INFO;
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

#ifndef MULTIAP
  i5JsonInit();
#endif

  i5NetlinkInit();

#if defined(SUPPORT_HOMEPLUG)
  i5PlcInitialize();
#endif /* SUPPORT_HOMEPLUG */

#if defined(IEEE1905_KERNEL_MODULE_PB_SUPPORT)
  i5UdpSocketInit();
#endif /* IEEE1905_KERNEL_MODULE_PB_SUPPORT */

  i5SecurityInit();

#if defined(WIRELESS)
  if (i5WlCfgInit() < 0 ) {
    i5TraceError("WlCFG initialization error\n");
    goto end;
  }
#endif /* WIRELESS */

  i5EthStatInit();

  i5BrUtilInit();

  i5InterfaceInit();

  i5VendorInit();

  /* enable state or other settings may have changed during init */
  i5_config.running = 1;
  i5GlueSaveConfig();

  if (strtoul(nvram_safe_get("config_ctr_chan_in_media_info"), NULL, 0)) {
    i5_config.flags |= I5_CONFIG_FLAG_CTR_CHAN_MEDIA_INFO;
  }

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

  /* First delete all the vlan interfaces before deiniting others */
  i5GlueDeleteAllVlanInterfaces();

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

  i5VendorDeinit();

  i5DmDeinit();

#ifndef MULTIAP
  i5JsonDeinit();
#endif

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

  /* close the current syslog connection */
  closelog();
}

/* Deinit when exiting due to signal handlers. Just close the sockets and VLAN interfaces */
void ieee1905_deinit_on_sig()
{
  /* Update the flag saying the signal handler has issued the deinit */
  i5_config.self_flags |= I5_CFG_SELF_FLAG_SIGNAL;

  /* First delete all the vlan interfaces before deiniting others */
  i5GlueDeleteAllVlanInterfaces();

  i5SocketDeinit();
}

/* Get the AL MAC address */
unsigned char *ieee1905_get_al_mac()
{
  return &i5_config.i5_mac_address[0];
}

/* Get the i5_config_type structure */
void *ieee1905_get_i5_config()
{
  return (i5_config_type*)&i5_config;
}

/* Remove all get BSSID timer except in pdmif */
void i5RemoveAllGetBSSIDTimer(i5_dm_interface_type *pdmif)
{
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pIterIf;

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return;
  }

  foreach_i5glist_item(pIterIf, i5_dm_interface_type, pdmdev->interface_list) {
    /* Unset the BSSID if the current interface is not the interface which is connected to upstream.
     */
    if (!I5_IS_IFR_WIRELESS(pIterIf->flags)) {
      continue;
    }

    if (pdmif && (pIterIf == pdmif || !I5_IS_BSS_STA(pIterIf->mapFlags))) {
      continue;
    }

    /* Close the get BSSID timer */
    if (pIterIf->ptmrGetBSSID) {
      i5TimerFree(pIterIf->ptmrGetBSSID);
      pIterIf->ptmrGetBSSID = NULL;
      i5TraceInfo("Interface["I5_MAC_FMT"]. Close Get BSSID Timer\n",
        I5_MAC_PRM(pIterIf->InterfaceId));
    }
  }
}

/* Add a backhaul STA to the backhaul STA MLD conf whenever any association happens */
static void i5AddBackhaulSTAMLDConf(unsigned char *ruid, unsigned char *bsta_mld_mac,
  unsigned char *ap_mld_mac)
{
  i5_bsta_mld_conf_t *bsta_mld_conf = i5_dm_network_topology.bsta_mld_conf;
  i5_bsta_mld_affliated_bsta_conf_t *affliated_bsta;

  /* No need to add it if it is not MLO */
  if (bsta_mld_mac == NULL) {
    return;
  }

  /* If it is the first item, allocate memory for the list */
  if (bsta_mld_conf == NULL) {
    bsta_mld_conf = (i5_bsta_mld_conf_t*)calloc(1, sizeof(*bsta_mld_conf));
    if (!bsta_mld_conf) {
      i5TraceDirPrint("Malloc Failed for Backhaul STA MLD Configuration\n");
      return;
    }
    ieee1905_glist_init(&bsta_mld_conf->affliated_bstas);
    bsta_mld_conf->mld_mode = I5_MLD_MODE_STR;
    i5_dm_network_topology.bsta_mld_conf = bsta_mld_conf;
  }

  bsta_mld_conf->bsta_mld_flag |= I5_BSTA_MLD_FLAG_BSTA_MAC_ADDR_VALID;
  eacopy(bsta_mld_mac, bsta_mld_conf->bsta_mld_mac);

  if (ap_mld_mac) {
    bsta_mld_conf->bsta_mld_flag |= I5_BSTA_MLD_FLAG_AP_MAC_ADDR_VALID;
    eacopy(ap_mld_mac, bsta_mld_conf->ap_mld_mac);
  }

  /* Check if the backhaul STA is already there in the list or not */
  if (i5DmFindRUIDInbSTAMLDConf(bsta_mld_conf, ruid) != NULL) {
    i5TraceInfo("bSTA RUID "I5_MAC_DELIM_FMT" Already Present\n", I5_MAC_PRM(ruid));
    return;
  }

  affliated_bsta = (i5_bsta_mld_affliated_bsta_conf_t*)calloc(1, sizeof(*affliated_bsta));
  if (!affliated_bsta) {
    i5TraceDirPrint("Malloc Failed for Affliated bSTA Configuration for a bSTA MLD\n");
    return;
  }

  affliated_bsta->bsta_flag |= I5_AFFILIATED_BSTA_FLAG_MAC_ADDR_VALID;
  eacopy(ruid, affliated_bsta->ruid);
  eacopy(ruid, affliated_bsta->bsta_mac);
  ieee1905_glist_append(&bsta_mld_conf->affliated_bstas, (dll_t*)affliated_bsta);
  i5TraceInfo("Added RUID "I5_MAC_DELIM_FMT" with MLD MAC "I5_MAC_DELIM_FMT"\n",
    I5_MAC_PRM(affliated_bsta->ruid), I5_MAC_PRM(bsta_mld_conf->bsta_mld_mac));
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

  if (pdmif->ptmrGetBSSID) {
    i5TimerFree(pdmif->ptmrGetBSSID);
    pdmif->ptmrGetBSSID = NULL;
  }

  /* Get the interface info where we will get the BSSID of the bSTA interface */
  memset(&info, 0, sizeof(info));
  if (i5_config.cbs.get_interface_info) {
    if (i5_config.cbs.get_interface_info(pdmif->ifname, &info) != 0) {
      i5TraceError("Failed to get interface info["I5_MAC_DELIM_FMT"]\n",
        I5_MAC_PRM(pdmif->InterfaceId));
      goto end;
    }

    /* All the interface supports Traffic Separation on combined fronthaul and Profile-1 backhaul
     * and Traffic Separation on combined Profile-1 backhaul and Profile-2 backhaul
     */
    pdmif->flags |= (I5_FLAG_IFR_TS_MIX_FH_P1BH_SUPPORTED | I5_FLAG_IFR_TS_MIX_P1BH_P2BH_SUPPORTED);

    /* If it is STA interface check if the BSSID is NULL or not */
    if (info.mapFlags & IEEE1905_MAP_FLAG_STA) {
      if (i5DmIsMacNull(info.bssid)) {
        i5TraceInfo("MAP[%d] BSSID is NULL\n", info.mapFlags);
        goto end;
      }
    } else {
      /* If not STA interface just return */
      i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
      i5DmFreeChScanCaps(&info.ap_caps.ChScanCaps);
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
    eacopy(info.bsta_peer_mld, pdmif->bsta_peer_mld);
    eacopy(info.self_mld_addr, pdmif->self_mld_addr);
    i5TraceInfo("For Backhaul STA " I5_MAC_FMT " band 0x%x BSSID "I5_MAC_FMT" self MLD "
      "address " I5_MAC_FMT " Peer MLD "I5_MAC_FMT"\n",
      I5_MAC_PRM(pdmif->InterfaceId), pdmif->band, I5_MAC_PRM(pdmif->bssid),
      I5_MAC_PRM(pdmif->self_mld_addr), I5_MAC_PRM(pdmif->bsta_peer_mld));
    if (I5_IS_IFR_MLO_ENAB(pdmif->flags)) {
      i5AddBackhaulSTAMLDConf(pdmif->InterfaceId,
        (i5DmIsMacNull(pdmif->self_mld_addr) ? NULL : pdmif->self_mld_addr),
        (i5DmIsMacNull(pdmif->bsta_peer_mld) ? NULL : pdmif->bsta_peer_mld));
    }
    i5WlCfgUpdateMediaInfo(pdmif->InterfaceId, info.chanspec, info.bssid, info.mapFlags);
    i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);

    i5DmFreeChScanCaps(&info.ap_caps.ChScanCaps);

    /* Create Empty Channel Scan Results Using Channel Scan Capabilities,
     * and store it to Self Device. To send Result for Requested Channel Scan - Stored
     */
    i5DmStoreEmptyChScanResultsFmCapabilities(i5_dm_network_topology.selfDevice);

    /* Remove all get BSSID timer except in pdmif */
    i5RemoveAllGetBSSIDTimer(pdmif);

    return;
  }

end:
  i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
  i5DmFreeChScanCaps(&info.ap_caps.ChScanCaps);

  /* Start the timer again to get the BSSID */
  if (pdmif) {
    pdmif->ptmrGetBSSID = i5TimerNew(I5_DM_GET_BSTA_BSSID_TIME_MSEC,
      i5GetBSSIDOfSTAInterfaceTimeout, (void*)pdmif->InterfaceId);
  }
}

static void ieee1905_fill_dev_akm_suites(i5_dm_akm_suite_caps_t *dev_akm_suites)
{
  i5_dm_bss_akm_suite_caps_t *bss_akm_suite;
  i5_dm_akm_suite_t bh_akm_suites[] = {
    {{0x00, 0x0f, 0xac}, 2},	/* PSK */
    {{0x00, 0x0f, 0xac}, 6},	/* PSK with SHA256 */
    {{0x00, 0x0f, 0xac}, 8},	/* SAE */
    {{0x50, 0x6f, 0x9a}, 2},	/* DPP */
  };
  i5_dm_akm_suite_t fh_akm_suites[] = {
    {{0x00, 0x0f, 0xac}, 2},	/* PSK */
    {{0x00, 0x0f, 0xac}, 4},	/* FT using PSK */
    {{0x00, 0x0f, 0xac}, 6},	/* PSK with SHA256 */
    {{0x00, 0x0f, 0xac}, 8},	/* SAE with SHA256 */
    {{0x00, 0x0f, 0xac}, 9},	/* FT over SAE */
    {{0x50, 0x6f, 0x9a}, 2},	/* DPP */
  };
  int idx, count;

  ieee1905_glist_init(&dev_akm_suites->bh_akm_suites);
  ieee1905_glist_init(&dev_akm_suites->fh_akm_suites);

  count = sizeof(bh_akm_suites)/sizeof(bh_akm_suites[0]);
  for (idx = 0; idx < count; idx++) {
    bss_akm_suite = (i5_dm_bss_akm_suite_caps_t*)malloc(sizeof(*bss_akm_suite));
    if (!bss_akm_suite) {
      i5TraceDirPrint("malloc error \n");
      return;
    }
    memcpy(&bss_akm_suite->akm_suite, &bh_akm_suites[idx], sizeof(bss_akm_suite->akm_suite));
    ieee1905_glist_append(&dev_akm_suites->bh_akm_suites, (dll_t*)bss_akm_suite);
  }

  count = sizeof(fh_akm_suites)/sizeof(fh_akm_suites[0]);
  for (idx = 0; idx < count; idx++) {
    bss_akm_suite = (i5_dm_bss_akm_suite_caps_t*)malloc(sizeof(*bss_akm_suite));
    if (!bss_akm_suite) {
      i5TraceDirPrint("malloc error \n");
      return;
    }
    memcpy(&bss_akm_suite->akm_suite, &fh_akm_suites[idx], sizeof(bss_akm_suite->akm_suite));
    ieee1905_glist_append(&dev_akm_suites->fh_akm_suites, (dll_t*)bss_akm_suite);
  }
}

/* Start IEEE1905 Messaging */
void ieee1905_start()
{
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif, *pdmifSTA = NULL;
  i5_dm_bss_type *pdmbss;
  ieee1905_ifr_info info;

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return;
  }

  /* Fill out the akm suites capability for device's fronthaul and backhaul */
  if (I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    ieee1905_fill_dev_akm_suites(&pdmdev->dev_akm_suites);
  }
  /* Get the Wireless interface info and fill the info via callback */
  pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
  while (pdmif != NULL) {
    if (I5_IS_IFR_WIRELESS(pdmif->flags)) {
      memset(&info, 0, sizeof(info));
      if (i5_config.cbs.get_interface_info) {
        if (i5_config.cbs.get_interface_info(pdmif->ifname, &info) != 0) {
          i5TraceError("Failed to get interface info["I5_MAC_DELIM_FMT"]\n",
            I5_MAC_PRM(pdmif->InterfaceId));
          i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
          i5DmFreeChScanCaps(&info.ap_caps.ChScanCaps);
          goto next;
        }
        pdmif->flags |= (I5_FLAG_IFR_TS_MIX_FH_P1BH_SUPPORTED | I5_FLAG_IFR_TS_MIX_P1BH_P2BH_SUPPORTED);
        pdmif->chanspec = info.chanspec;
        if (I5_IS_MULTIAP_AGENT(i5_config.flags)) {
          pdmif->mapFlags |= info.mapFlags;
          i5DmCopyAPCaps(&pdmif->ApCaps, &info.ap_caps);
          eacopy(info.self_mld_addr, pdmif->self_mld_addr);
          i5TraceInfo("Interface " I5_MAC_FMT " self MLD address " I5_MAC_FMT "\n",
            I5_MAC_PRM(pdmif->InterfaceId), I5_MAC_PRM(pdmif->self_mld_addr));

          /* If it is STA interface get BSSID where the bSTA is associated, to send it
           * as part of the device information TLV's media specific info
           */
          if (info.mapFlags & IEEE1905_MAP_FLAG_STA) {
            if (i5DmIsMacNull(info.bssid)) {
              i5TraceInfo("MAP[%d] BSSID is NULL\n", info.mapFlags);
              if (!pdmif->ptmrGetBSSID) {
                pdmif->ptmrGetBSSID = i5TimerNew(I5_DM_GET_BSTA_BSSID_TIME_MSEC,
                  i5GetBSSIDOfSTAInterfaceTimeout, (void*)pdmif->InterfaceId);
              }
            } else {
              memcpy(pdmif->bssid, info.bssid, sizeof(pdmif->bssid));
              eacopy(info.bsta_peer_mld, pdmif->bsta_peer_mld);
              i5TraceInfo("For Backhaul STA " I5_MAC_FMT " BSSID "I5_MAC_FMT" self MLD "
                "address " I5_MAC_FMT " Peer MLD "I5_MAC_FMT"\n",
                I5_MAC_PRM(pdmif->InterfaceId), I5_MAC_PRM(pdmif->bssid),
                I5_MAC_PRM(pdmif->self_mld_addr), I5_MAC_PRM(pdmif->bsta_peer_mld));
              if (I5_IS_IFR_MLO_ENAB(pdmif->flags)) {
                i5AddBackhaulSTAMLDConf(pdmif->InterfaceId,
                  (i5DmIsMacNull(pdmif->self_mld_addr) ? NULL : pdmif->self_mld_addr),
                  (i5DmIsMacNull(pdmif->bsta_peer_mld) ? NULL : pdmif->bsta_peer_mld));
              }

              /* Update the AP caps from any of the virtual BSS as primary interface is a STA */
              pdmbss = pdmif->bss_list.ll.next;
              if (pdmbss != NULL) {
                i5DmUpdateAPCaps(pdmbss->ifname, pdmif);
              }
              /* This STA interface associated to upstream AP */
              pdmifSTA = pdmif;
            }
          }

          i5WlCfgUpdateMediaInfo(pdmif->InterfaceId, info.chanspec, info.bssid, info.mapFlags);
          pdmif->band = (unsigned char)ieee1905_get_band_from_radiocaps(&pdmif->ApCaps.RadioCaps);
        } else if ((!I5_IS_MAP_CERT(i5_config.flags)) &&
          (!I5_DONT_ADD_MEDIA_INFO(i5_config.flags))) {
          i5WlCfgUpdateMediaInfo(pdmif->InterfaceId, info.chanspec, info.bssid, info.mapFlags);
        }

        if (I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
          pdmif->band = (unsigned char)ieee1905_get_band_from_radiocaps(&info.ap_caps.RadioCaps);
        }

        /* Free radio & Channel Scan caps memory */
        i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
        i5DmFreeChScanCaps(&info.ap_caps.ChScanCaps);
      }
    }

next:
      pdmif = pdmif->ll.next;
  }

  /* Create Empty Channel Scan Results Using Channel Scan Capabilities,
   * and store it to Self Device. To send Result for Requested Channel Scan - Stored
   */
  i5DmStoreEmptyChScanResultsFmCapabilities(i5_dm_network_topology.selfDevice);

  /* If any of the STA is associated to upstream AP, remove bssid timer of all other interfaces */
  if (pdmifSTA) {
    i5RemoveAllGetBSSIDTimer(pdmifSTA);
  }

  /* First time get it immediately */
  i5BrUtilUpdate(NULL);

  i5_config.flags |= I5_CONFIG_FLAG_START_MESSAGE;
  if (i5_config.ptmrApSearch) {
    i5TimerFree(i5_config.ptmrApSearch);
    i5_config.ptmrApSearch = NULL;
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
#endif
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
  i5_config.cbs.device_init = incbs->device_init;

  i5_config.cbs.device_update = incbs->device_update;

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

  i5_config.cbs.recv_chan_selection_resp = incbs->recv_chan_selection_resp;

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

  i5_config.cbs.assoc_sta_metric_resp = incbs->assoc_sta_metric_resp;

  i5_config.cbs.unassoc_sta_metric_resp = incbs->unassoc_sta_metric_resp;

  i5_config.cbs.beacon_metric_resp = incbs->beacon_metric_resp;

  i5_config.cbs.ap_configured = incbs->ap_configured;

  i5_config.cbs.operating_chan_report = incbs->operating_chan_report;

  i5_config.cbs.steering_btm_rpt = incbs->steering_btm_rpt;

  i5_config.cbs.higher_layer_data = incbs->higher_layer_data;

  i5_config.cbs.interface_chan_change = incbs->interface_chan_change;

  i5_config.cbs.ap_auto_config_resp = incbs->ap_auto_config_resp;

  i5_config.cbs.process_ap_auto_config_search_chirp = incbs->process_ap_auto_config_search_chirp;

  i5_config.cbs.ap_auto_config_search_sent = incbs->ap_auto_config_search_sent;

  i5_config.cbs.set_bh_sta_params = incbs->set_bh_sta_params;

  i5_config.cbs.operating_channel_dfs_update = incbs->operating_channel_dfs_update;

  i5_config.cbs.nonoperable_channel_update = incbs->nonoperable_channel_update;

  i5_config.cbs.remove_and_deauth_sta_entry = incbs->remove_and_deauth_sta_entry;

  i5_config.cbs.process_tunneled_msg = incbs->process_tunneled_msg;

  i5_config.cbs.channel_scan_req = incbs->channel_scan_req;

  i5_config.cbs.channel_scan_rpt = incbs->channel_scan_rpt;

  i5_config.cbs.process_cac_msg = incbs->process_cac_msg;

  i5_config.cbs.prepare_cac_complete = incbs->prepare_cac_complete;

  i5_config.cbs.process_cac_complete = incbs->process_cac_complete;

  i5_config.cbs.process_cac_status = incbs->process_cac_status;

  i5_config.cbs.prepare_cac_capabilities = incbs->prepare_cac_capabilities;

  i5_config.cbs.get_bh_sta_profile = incbs->get_bh_sta_profile;

  i5_config.cbs.mbo_assoc_disallowed = incbs->mbo_assoc_disallowed;

  i5_config.cbs.prepare_cac_status = incbs->prepare_cac_status;

  i5_config.cbs.set_dfs_chan_clear = incbs->set_dfs_chan_clear;

  i5_config.cbs.get_primary_vlan_id = incbs->get_primary_vlan_id;

  i5_config.cbs.dpp_cce_indication = incbs->dpp_cce_indication;

  i5_config.cbs.dpp_chirp_notification = incbs->dpp_chirp_notification;

  i5_config.cbs.encap_1905_eapol = incbs->encap_1905_eapol;

  i5_config.cbs.dpp_proxied_encap = incbs->dpp_proxied_encap;

  i5_config.cbs.dpp_direct_encap = incbs->dpp_direct_encap;

  i5_config.cbs.get_dpp_config_req_obj = incbs->get_dpp_config_req_obj;

  i5_config.cbs.parse_dpp_config_req_obj = incbs->parse_dpp_config_req_obj;

  i5_config.cbs.get_dpp_config_resp_obj = incbs->get_dpp_config_resp_obj;

  i5_config.cbs.parse_dpp_config_resp_obj = incbs->parse_dpp_config_resp_obj;

  i5_config.cbs.process_agentlist = incbs->process_agentlist;

  i5_config.cbs.process_dpp_bootstrap_uri_obj = incbs->process_dpp_bootstrap_uri_obj;

  i5_config.cbs.notify_command = incbs->notify_command;

  i5_config.cbs.process_err_resp_msg = incbs->process_err_resp_msg;

  i5_config.cbs.process_serv_prio_req_msg = incbs->process_serv_prio_req_msg;

  i5_config.cbs.process_qosmgmt_notif_msg = incbs->process_qosmgmt_notif_msg;
}

/* Agent Update Service Prioritization IFR flags in Self Device */
static void
ieee1905_update_ifr_serv_prio_flags(i5_dm_bss_type *pdmbss, i5_dm_interface_type *pdmif)
{
  char prefix[I5_MAX_IFNAME], final_nvram[256];

  i5WlCfgMakeWlPrefix(pdmbss->ifname, prefix, sizeof(prefix));

  snprintf(final_nvram, sizeof(final_nvram), "%s%s", prefix,
    I5_WLCFG_NVRAM_QOSMGMT_ENABLE);

  pdmbss->qosmgmt_enable = wlcfg_get_nvram_val_uint(final_nvram);

  /* Extract QoSMgmt Capabilities, Set corrosponding Service Prioritization IFR flags */
  if (I5_IS_BSS_QOSMGMT_MSCS(pdmbss->qosmgmt_enable)) {
    pdmif->flags |= I5_FLAG_IFR_SP_MSCS_SUPPORTED;
  }
  if (I5_IS_BSS_QOSMGMT_SCS(pdmbss->qosmgmt_enable)) {
    pdmif->flags |= I5_FLAG_IFR_SP_SCS_SUPPORTED;
  }
  if (I5_IS_BSS_QOSMGMT_QOSMAP(pdmbss->qosmgmt_enable)) {
    pdmif->flags |= I5_FLAG_IFR_SP_QOS_MAP_SUPPORTED;
  }
  if (I5_IS_BSS_QOSMGMT_DSCP_POLICY(pdmbss->qosmgmt_enable)) {
    pdmif->flags |= I5_FLAG_IFR_SP_DSCP_POL_SUPPORTED;
  }
  if (I5_IS_BSS_QOSMGMT_SCS_TRF_DESC(pdmbss->qosmgmt_enable)) {
    pdmif->flags |= I5_FLAG_IFR_SP_SCSTD_SUPPORTED;
  }
}

/* Add the BSS */
int ieee1905_add_bss(unsigned char *radio_mac, unsigned char *bssid, unsigned char *ssid,
  unsigned char ssid_len, unsigned short chanspec, char *ifname, unsigned char mapFlags,
  unsigned char *mld_addr, int8 mld_unit)
{
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;
  i5_agent_ap_mld_conf_t *agent_ap_mld;

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

  if ((pdmbss = i5DmBSSNew(pdmif, ifname, bssid, ssid, ssid_len, mapFlags, mld_addr)) == NULL) {
    i5TraceError("Failed to Add BSS " I5_MAC_FMT "In Interface " I5_MAC_FMT "\n", I5_MAC_PRM(bssid),
      I5_MAC_PRM(radio_mac));
    return IEEE1905_FAIL;
  }
  pdmbss->mapFlags = mapFlags;
  pdmif->mapFlags |= mapFlags;
  pdmbss->mld_unit = mld_unit;

  if (mld_addr && !i5DmIsMacNull(mld_addr)) {
    pdmbss->flags |= I5_BSS_FLAG_MLO_ENAB;

    /* Add SSID to Agent AP MLD Configuration */
    agent_ap_mld = i5DmFindSSIDInApMLDConf(&i5_dm_network_topology.agent_ap_mld_conf,
      &pdmbss->ssid);
    if (agent_ap_mld == NULL) {
      i5TraceInfo("Add SSID %s with MLD Address "I5_MAC_DELIM_FMT" to Agent AP MLD Configuration\n",
        (char*)pdmbss->ssid.SSID, I5_MAC_PRM(mld_addr));
      agent_ap_mld = i5DmNewAgentAPMLD(&i5_dm_network_topology.agent_ap_mld_conf, &pdmbss->ssid,
        mld_addr, TRUE);
      if (agent_ap_mld == NULL) {
        return IEEE1905_FAIL;
      }
    }
    /* Add Affliated AP */
    (void)i5DmNewAffliatedAP(agent_ap_mld, radio_mac, bssid, pdmif->link_id);
  }

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

  /* For fronthaul BSS, in plugfest we should block VLAN tagged packets from stations. We can remove
   * this code in production branch. TODO
   */
  if (I5_IS_TS_ADD_FH_RULE(i5_config.flags) && i5GlueIsCreateVLANS() &&
    !I5_IS_BSS_BACKHAUL(pdmbss->mapFlags)) {
    i5GlueCreateTSBSSRules(pdmbss->ifname);
    pdmbss->flags |= I5_BSS_FLAG_EBTABLE_RULE;
  }

  /* Agent Update Service Prioritization IFR flags in Self Device */
  ieee1905_update_ifr_serv_prio_flags(pdmbss, pdmif);

  return IEEE1905_OK;
}

/* Remove the BSS */
int ieee1905_remove_bss(char *ifname)
{
  int ret = -1;
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;

  if ((pdmdev = i5DmGetSelfDevice()) == NULL) {
    return -1;
  }

  /* Find the BSS item to be removed from this device for a given ifname */
  for (pdmif = pdmdev->interface_list.ll.next; pdmif; pdmif = pdmif->ll.next) {

    for (pdmbss = pdmif->bss_list.ll.next; pdmbss; pdmbss = pdmbss->ll.next) {

      if (strcmp(pdmbss->ifname, ifname) != 0) {
        continue;
      }

      /* Remove the BSS */
      ret = i5DmBSSFree(pdmif, pdmbss);
      return ret;
    }
  }

  return ret;
}

/* get count of disabled bss in the bssinfo list */
int
ieee1905_ctlr_table_get_dis_bss_cnt(ieee1905_glist_t *bssinfo_list)
{
	int disabled_bssinfo_count = 0;
	ieee1905_client_bssinfo_type *iter_bss = NULL;

	foreach_iglist_item(iter_bss, ieee1905_client_bssinfo_type, *bssinfo_list) {
		if (iter_bss->disabled) {
			disabled_bssinfo_count++;
		}
	}
	return disabled_bssinfo_count;
}

/* Add the BSS to Controller table. only required in controller */
int ieee1905_add_bssto_controller_table(ieee1905_client_bssinfo_type *bss)
{
  ieee1905_client_bssinfo_type *clientbss = NULL;
  ieee1905_ssid_list_type *ssid_type;
  unsigned short vlan_id;

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
  snprintf(clientbss->bss_name, sizeof(clientbss->bss_name), "%s", bss->bss_name);
  memcpy(clientbss->ALID, bss->ALID, sizeof(clientbss->ALID));
  clientbss->band_flag = bss->band_flag;
  memcpy(&clientbss->ssid, &bss->ssid, sizeof(clientbss->ssid));
  clientbss->AuthType = bss->AuthType;
  clientbss->EncryptType = bss->EncryptType;
  memcpy(&clientbss->NetworkKey, &bss->NetworkKey, sizeof(clientbss->NetworkKey));
  clientbss->Closed = bss->Closed;
  clientbss->disabled = bss->disabled;
  clientbss->map_flag = bss->map_flag;
  clientbss->isolate = bss->isolate;
  /* If the traffic separation policy has SSID whose VLAN ID is not primary label it as Guest */
  ssid_type = i5DmTSPolicyFindSSID(&i5_config.policyConfig.ts_policy_list, bss->ssid.SSID,
    &vlan_id);
  if (ssid_type && vlan_id != i5_config.policyConfig.prim_vlan_id) {
    clientbss->map_flag |= IEEE1905_MAP_FLAG_GUEST;
  }
  clientbss->fbt_info.mdid = bss->fbt_info.mdid;
  clientbss->fbt_info.ft_cap_policy = bss->fbt_info.ft_cap_policy;
  clientbss->fbt_info.tie_reassoc_interval = bss->fbt_info.tie_reassoc_interval;
  memcpy(&clientbss->dpp_csign, &bss->dpp_csign, sizeof(clientbss->dpp_csign));

  /* Check MAP BSS info flags to config steering disable */
  if (IS_MAP_BSSINFO_CONFIG_STEER_DISABLED(bss->flags)) {
    clientbss->flags |= MAP_BSSINFO_CONFIG_STEER_DISABLED;
  }

  /* Is MLO disabled for this MAP BSS Name */
  if (IS_MAP_BSSINFO_MLO_DISABLED(bss->flags)) {
    clientbss->flags |= MAP_BSSINFO_MLO_DISABLED;
  }

  ieee1905_glist_append(&i5_config.client_bssinfo_list, (dll_t*)clientbss);

  i5TraceInfo("ALID["I5_MAC_DELIM_FMT"] band_flag[0x%x] ssid_len[%d] ssid[%s] auth[0x%x] "
        "encr[0x%x] pwd_len[%d] Password[%s] map_flag [0x%x] Closed[%d] "
        "FBT_MDID[%d] fbt_caps[%d] fbt_reassoc_interval[%d] disabled[%d] isolate[%d] "
        "csign[%s] flag[0x%x]\n",
        I5_MAC_PRM(clientbss->ALID), clientbss->band_flag, clientbss->ssid.SSID_len,
        clientbss->ssid.SSID, clientbss->AuthType, clientbss->EncryptType,
        clientbss->NetworkKey.key_len, clientbss->NetworkKey.key, clientbss->map_flag,
        clientbss->Closed, clientbss->fbt_info.mdid, clientbss->fbt_info.ft_cap_policy,
        clientbss->fbt_info.tie_reassoc_interval, clientbss->disabled, clientbss->isolate,
        clientbss->dpp_csign, clientbss->flags);

  return IEEE1905_OK;
}

/* Cleanup controller's BSS info table */
int
ieee1905_cleanup_controller_bss_info_table()
{
  i5DmGlistCleanup(&i5_config.client_bssinfo_list);
  ieee1905_glist_init(&i5_config.client_bssinfo_list);

  return IEEE1905_OK;
}

/* STA has Associated or Disassociated */
int ieee1905_sta_assoc_disassoc(unsigned char *bssid, unsigned char *mac, int isAssoc,
  unsigned short time_elapsed, unsigned char notify, unsigned char *assoc_frame,
  unsigned int assoc_frame_len, uint16 reason, unsigned char *sta_mld)
{
  i5_dm_device_type *pdmdev;
  int ret = IEEE1905_OK;

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
    ret = i5DmAssociateClient(NULL, pdmdev, bssid, mac, time_elapsed, notify, assoc_frame,
      assoc_frame_len, sta_mld);
  } else {
    ret = i5DmDisAssociateClient(pdmdev, bssid, mac, notify, reason);
  }

  return ret;
}

/* Send the BTM report to the controller */
int ieee1905_send_btm_report(ieee1905_btm_report *btm_report)
{
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

  i5MessageClientSteeringBTMReportSend(btm_report);

  return IEEE1905_OK;
}

/* Send steering completed message to the controller */
int ieee1905_send_steering_completed_message(ieee1905_steer_req *steer_req)
{
  i5_dm_device_type *pDeviceController = i5DmFindController();

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
  i5_dm_device_type *pDeviceController = i5DmFindController();

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

  i5MessageBackhaulSteeringResponseSend(pDeviceController, bh_steer_resp);

  return IEEE1905_OK;
}

/* Send the associated STA link metrics to the requested neighbor */
int ieee1905_send_assoc_sta_link_metric(i5_dm_device_type *pDeviceNeighbor,
  i5_dm_clients_type *pclient)
{
  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (pclient == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Client not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_STA_NOT_FOUND;
  }

  clock_gettime(CLOCK_REALTIME, &pclient->link_metric.queried);

  i5_config.last_message_identifier++;
  i5MessageAssociatedSTALinkMetricsResponseSend(pDeviceNeighbor->psock,
    pDeviceNeighbor->DeviceId, i5_config.last_message_identifier, pclient->mac, 1);

  return IEEE1905_OK;
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
static int ieee1905_send_common_query(i5_dm_device_type *pDeviceNeighbor, void *data,
  unsigned int flags)
{
  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor does not exist\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (flags == I5_MAP_FLAG_QUERY_ASSOC_STA_METRICS) {
    i5MessageAssociatedSTALinkMetricsQuerySend(pDeviceNeighbor->psock, pDeviceNeighbor->DeviceId,
      (unsigned char*)data);
  } else if (flags == I5_MAP_FLAG_QUERY_UNASSOC_STA_METRICS) {
    i5MessageUnAssociatedSTALinkMetricsQuerySend(pDeviceNeighbor,
      (ieee1905_unassoc_sta_link_metric_query*)data);
  } else if (flags == I5_MAP_FLAG_QUERY_BEACON_METRICS) {
    i5MessageBeaconMetricsQuerySend(pDeviceNeighbor->psock, pDeviceNeighbor->DeviceId,
      (ieee1905_beacon_request*)data);
  } else if (flags == I5_MAP_FLAG_QUERY_POLICY_CONFIG) {
    i5MessageMultiAPPolicyConfigRequestSend(pDeviceNeighbor);
  }

  return IEEE1905_OK;
}

/* Send Associated STA Link Metrics Query message */
int ieee1905_send_assoc_sta_link_metric_query(i5_dm_device_type *pDeviceNeighbor,
  unsigned char *sta_mac)
{
  return ieee1905_send_common_query(pDeviceNeighbor, sta_mac, I5_MAP_FLAG_QUERY_ASSOC_STA_METRICS);
}

/* Send UnAssociated STA Link Metrics Query message */
int ieee1905_send_unassoc_sta_link_metric_query(i5_dm_device_type *pDeviceNeighbor,
  ieee1905_unassoc_sta_link_metric_query *query)
{
  return ieee1905_send_common_query(pDeviceNeighbor, query, I5_MAP_FLAG_QUERY_UNASSOC_STA_METRICS);
}

/* Send Beacon Metrics Query message */
int ieee1905_send_beacon_metrics_query(i5_dm_device_type *pDeviceNeighbor,
  ieee1905_beacon_request *query)
{
  return ieee1905_send_common_query(pDeviceNeighbor, query, I5_MAP_FLAG_QUERY_BEACON_METRICS);
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

void ieee1905_send_channel_preference_query(i5_dm_device_type *pDeviceNeighbor)
{
  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor device does not exist\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return;
  }
  i5MessageChannelPreferenceQuerySend(pDeviceNeighbor->psock, pDeviceNeighbor->DeviceId);
}

void ieee1905_send_message(void *pmsg)
{
  i5VendorInformMessageSend(i5MessageDstMacAddressGet(pmsg), pmsg,
    i5MessageChannelSelectionRequestValue, NULL);

  i5TlvEndOfMessageTypeInsert((i5_message_type *)pmsg);
  i5MessageSend((i5_message_type *)pmsg, 0);
  i5MessageCreateRetryTimer((i5_message_type *)pmsg);
}

void *ieee1905_create_channel_selection_request(i5_dm_device_type *pDeviceNeighbor)
{
  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor device does not exist\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return NULL;
  }
  return (void *)i5MessageChannelSelectionRequestCreate(pDeviceNeighbor);
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
      vndr_msg->vendorSpec_len, I5_BRCM_VNDR_OUI);
  }

fail:
  return;
}

/* Send Policy Configuration to neighbor */
int ieee1905_send_policy_config(i5_dm_device_type *pDeviceNeighbor)
{
  return ieee1905_send_common_query(pDeviceNeighbor, NULL, I5_MAP_FLAG_QUERY_POLICY_CONFIG);
}

void ieee1905_send_ap_autoconfig_renew(i5_dm_device_type *pDeviceNeighbor)
{
  i5MessageApAutoconfigurationRenewSend(pDeviceNeighbor, 0);
}

/* Send operating channel report to controller */
int ieee1905_send_operating_chan_report(ieee1905_operating_chan_report *chan_rpt)
{
  i5_dm_device_type *pDeviceController = NULL;

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
int ieee1905_bSTA_associated_to_backhaul_ap(unsigned char *InterfaceId, char *ifname,
  unsigned short prim_vlan_id, int is_8021q_present)
{
  i5_dm_device_type *pdevice = i5DmGetSelfDevice();
  i5_dm_interface_type *pdmif;
  i5_socket_type *pif;
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

  /* Remove VLAN if 8021Q gets disabled or primary VLAN ID changes.
   * Otherwise, BH STA keeps using old VLAN ID and no layer 2 communication with upstream
   * AP possible.
   */
  if ((!I5_IS_MAP_CERT(i5_config.flags)) && i5_config.flags & I5_CONFIG_FLAG_TS_ACTIVE) {
    if (!is_8021q_present || i5_config.policyConfig.prim_vlan_id != prim_vlan_id) {
      i5Trace("Primary VLAN changed 8021_present[%d] old VLAN ID[%d] new VLAN ID[%d]\n",
        is_8021q_present, i5_config.policyConfig.prim_vlan_id, prim_vlan_id);
      /* Unset guest enabled flag, or else after deleting the VLAN interface, the socket creation
       * might again create the VLAN interfaces
       */
      i5_config.flags &= ~I5_CONFIG_FLAG_TS_ACTIVE;
      /* Remove VLAN interface for only that interface, if we remove all the VLAN interfaces,
       * downstream connections will be lost
       */
      i5GlueDeleteVlanIfr(ifname);
    }
  }

  /* If the primary VLAN ID is not configured, then create VLAN interface with primary VLAN ID */
  if (is_8021q_present && I5_IS_TS_SUPPORTED(i5_config.flags)) {
    i5_config.flags |= I5_CONFIG_FLAG_TS_ACTIVE;
    i5_config.self_flags |= I5_SELF_FLAG_TS_POLICY_IN_M2_OR_ASSOC;
    i5_config.policyConfig.prim_vlan_id = prim_vlan_id;

    /* Create VLANs only if the ifname is not VLAN */
    if (i5GlueIsCreateVLANS() && !i5GlueIsIfnameVLAN(ifname, NULL, NULL)) {
      i5Trace("For Ifname[%s] Create VLANs with the primary VLAN ID[%d] given by AP\n",
        ifname, prim_vlan_id);
      i5InterfaceSearchAdd(I5_MATCH_MEDIA_TYPE_ETH);
      i5GlueCreateVLAN(ifname, 1);
    }
  } else {
    /* Send topology discovery on this interface as soon as its connected to the upstream AP as
     * it will help in neighbor to identify this device as its neighbor.
     * If we create VLAN then its not required because, after the VLAN interface is created,
     * it will send the topology discovery
     */
    pif = i5SocketFindDevSocketByAddr(InterfaceId, NULL);
    if (pif)  {
      i5MessageRestartTopologyDiscovery(pif);
    }
  }

  /* Start ControllerSearch immediately to make fast MAP DPP onboarding time */
  i5WlCfgMultiApControllerSearch(NULL);

  /* Update the BSSID of the BSS where the STA is connected */
  i5GetBSSIDOfSTAInterfaceTimeout(InterfaceId);

  /* Dont do renew if there is 0 configured radio */
  pdmif = (i5_dm_interface_type *)pdevice->interface_list.ll.next;
  while (pdmif != NULL) {
    if (I5_IS_IFR_WIRELESS(pdmif->flags) && !i5WlCfgIsVirtualInterface(pdmif->ifname)) {
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

  return IEEE1905_OK;
}

/* Send neighbor link metrics query */
int ieee1905_send_neighbor_link_metric_query(i5_dm_device_type *pDeviceNeighbor,
  unsigned char specify_neighbor, unsigned char *neighbor_of_recv_device)
{
  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor device does not exist\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  i5MessageLinkMetricQuerySend(pDeviceNeighbor->psock, pDeviceNeighbor->DeviceId, specify_neighbor,
    neighbor_of_recv_device);

  return 0;
}

/* Send Ap Metrics Query. BSSIDs stored in a linear array. 6 octets for each BSSID */
int ieee1905_send_ap_metrics_query(i5_dm_device_type *pDeviceNeighbor,
  unsigned char *bssids, unsigned char bssid_count)
{
  if (pDeviceNeighbor == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor device does not exist\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  i5MessageAPMetricsQuerySend(pDeviceNeighbor->psock, pDeviceNeighbor->DeviceId, bssids,
    bssid_count);

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
	const i5_dm_rc_chan_map_type *rc_chan_map;
	unsigned int reg_class_count = 0;

	rc_chan_map = i5DmGetRCChannelMap(&reg_class_count);

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
		for (i = 0; i < reg_class_count; i++) {
			if (rc_chan_map[i].regclass == rc) {
				break;
			}
		}
		if (i == reg_class_count) {
			i5TraceError("Unknown regulatory class %d in RadioCaps. skip it\n", rc);
			rcidx++; /* Max Transmit power */
			ch_count = RadioCaps->List[rcidx++];
			rcidx += ch_count; /* channels */
			continue;
		}
		/* get band from rc if possible */
		band_flag |= i5WlCfgChannelToband(rc, 0);

		if ((band_flag == BAND_2G) || (band_flag == BAND_6G) ||
			(band_flag == (BAND_5GL | BAND_5GH))) {
			goto end;
		}

		map_ptr = &rc_chan_map[i];
		rcidx++; /* Max Transmit power */
		ch_count = RadioCaps->List[rcidx++];
		if (ch_count == 0) {
			/* Allchannels are valid, check band of all channels */
			i5TraceInfo("All the channels are valid in Regclass: %d\n", rc);
			for (i = 0; i < map_ptr->count; i++) {
				band_flag |= i5WlCfgChannelToband(rc, map_ptr->channel[i]);
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
				band_flag |= i5WlCfgChannelToband(rc, chan_list[i]);
			}
		}
		rcidx = chan_idx_end;

	}
end:
	i5TraceInfo("band=%d\n", band_flag);
	return band_flag;
}

/* Function to get band from the given channel */
int
ieee1905_get_band_from_channel(unsigned char opclass, unsigned char channel)
{
	return i5WlCfgChannelToband(opclass, channel);
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
ieee1905_send_ap_metrics_response(i5_dm_device_type *pDeviceNeighbor, unsigned char *ifr_mac)
{
  if (pDeviceNeighbor == NULL) {
    i5TraceError("Neighbor device does not exist\n");
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  /* Send the AP Metrics response to the controller. This is called from application may be due
   * to channel utilization threshold
   */
  i5MessageAPMetricsUnsolicitatedResponseSend(pDeviceNeighbor, ifr_mac);

  return IEEE1905_OK;
}

/* Send Backhaul Steering Request */
int
ieee1905_send_backhaul_steering_request(i5_dm_device_type *pDeviceNeighbor,
  ieee1905_backhaul_steer_msg *bh_steer_req)
{
  int ret;

  if (pDeviceNeighbor == NULL) {
    i5TraceError("Neighbor device does not exist\n");
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (pDeviceNeighbor->psock == NULL) {
    i5TraceError("Neighbor["I5_MAC_DELIM_FMT"] psock is NULL\n",
      I5_MAC_PRM(pDeviceNeighbor->DeviceId));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  ret = i5MessageBackhaulSteeringRequestSend(pDeviceNeighbor, bh_steer_req);

  return ret;
}

/* To inform ieee1905 about disassociation of bSTA from the backhaul AP */
int ieee1905_bSTA_disassociated_from_backhaul_ap(unsigned char *InterfaceId)
{
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif, *pIterIf;

  i5TraceInfo("STA Interface["I5_MAC_FMT"] Disassociated from AP\n", I5_MAC_PRM(InterfaceId));

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Self Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return -1;
  }

  if ((pdmif = i5DmInterfaceFind(pdmdev, InterfaceId)) == NULL) {
    i5TraceError("Interface " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(InterfaceId));
    return -1;
  }

  /* If it is MLO, unset the bsta_peer_mld of all the links */
  if (I5_IS_IFR_MLO_ENAB(pdmif->flags)) {
    foreach_i5glist_item(pIterIf, i5_dm_interface_type, pdmdev->interface_list) {
      if (!I5_IS_IFR_WIRELESS(pIterIf->flags)) {
        continue;
      }

      /* If the bsta_peer_mld is not matching, no need to unset */
      if (eacmp(pIterIf->bsta_peer_mld, pdmif->bsta_peer_mld) != 0) {
        continue;
      }

      bzero(pIterIf->MediaSpecificInfo, MAC_ADDR_LEN);
      bzero(pIterIf->bssid, sizeof(pIterIf->bssid));
      bzero(pIterIf->bsta_peer_mld, sizeof(pIterIf->bsta_peer_mld));
    }
  } else {
    /* Set the Media specific info to NULL as the STA got disconnected and BSSID will not be
     * there
     */
    bzero(pdmif->MediaSpecificInfo, MAC_ADDR_LEN);
    bzero(pdmif->bssid, sizeof(pdmif->bssid));
    bzero(pdmif->bsta_peer_mld, sizeof(pdmif->bsta_peer_mld));
  }

  /* Remove all the neighbor entry which is connected via this STA interface */
  i5DmFreeNeibhorsFromInterfaceId(pdmdev, InterfaceId);
  /* Remove the VLAN interface created */
  i5GlueDeleteVlanIfr(pdmif->ifname);

  return 1;
}

/* Get policy config */
ieee1905_policy_config* ieee1905_get_policy_config()
{
  return &i5_config.policyConfig;
}

/* Send M1 for unconfigured radio from wbd */
void ieee1905_start_m1(void)
{
  i5_dm_device_type *pdeviceController;

  pdeviceController = i5DmFindController();
  if (pdeviceController == NULL || pdeviceController->psock == NULL) {
    /* Controller not found search again */
    i5Trace("Controller not found search again\n");
    i5WlCfgMultiApControllerSearch(NULL);
    return;
  }

  if (I5_IS_EARLY_AP_CAP_PREF(i5_config.flags)) {
    i5MessageAllAPCapabilityReportSend(NULL, pdeviceController, 1);
  }

  /* If agent process the AP-autoconfiguration response set M1 Active now it for allowing renew process */
  i5_config.flags |= I5_CONFIG_FLAG_ACCEPT_RENEW;
  i5Trace("Starting M1 to Controller ["I5_MAC_DELIM_FMT"] \n",
    I5_MAC_PRM(pdeviceController->DeviceId));
  /* Controller found send M1 */
  i5WlCfgSendM1ForUnConfiguredRadio(pdeviceController);
}

/* STA assoc Failed connection */
int ieee1905_sta_assoc_failed_connection(unsigned char *bssid, unsigned char *mac, uint16 status, uint16 reason)
{
  i5_dm_device_type *pDeviceController = i5DmFindController();

  /* If it is not agent dont send report */
  if (!I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Agent functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_AGENT;
  }

  if (pDeviceController == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Controller not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return -1;
  }
  i5MessageFailedConnectionMessageSend(pDeviceController, bssid, mac, status, reason);

  return IEEE1905_OK;
}

/* Add the Unsuccessful association Policy for a device */
int ieee1905_add_unsuccessful_association_policy(ieee1905_unsuccessful_assoc_config_t *unsuccessful_assoc_config)
{
  /* Unsuccessful association policy should be added only to the controller */
  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Controller functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_CONTROLLER;
  }
  if (unsuccessful_assoc_config == NULL) {
    goto end;
  }

  memcpy(&(i5_config.policyConfig.unsuccessful_assoc_config), unsuccessful_assoc_config,
    sizeof(i5_config.policyConfig.unsuccessful_assoc_config));
end:
  return IEEE1905_OK;
}

/* Add the Channel Scan Reporting Policy for a radio */
int ieee1905_add_chscan_reporting_policy_for_radio(ieee1905_chscanrpt_config *chscanrptIn)
{
  /* Channel Scan Reporting Policy should be added only to the controller */
  if (!I5_IS_MULTIAP_CONTROLLER(i5_config.flags)) {
    i5TraceError("Device " I5_MAC_FMT " does not support Multi AP Controller functionality\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return IEEE1905_NOT_CONTROLLER;
  }

  if (chscanrptIn == NULL) {
    goto end;
  }

  /* Update the values */
  i5_config.policyConfig.chscanrpt_config.chscan_rpt_policy_flag =  chscanrptIn->chscan_rpt_policy_flag;

end:
  return IEEE1905_OK;
}

/* Send Channel Scan Result to Controller from Stored Scan data, for given Channel Scan Request */
int ieee1905_send_requested_stored_channel_scan(ieee1905_chscan_req_msg *chscan_req,
  unsigned char in_status_code)
{
  i5_dm_device_type *pDeviceController = NULL;
  ieee1905_chscan_report_msg chscan_results;

  pDeviceController = i5DmFindController();
  if (!pDeviceController) {
    i5TraceError("controller device does not exist\n");
    return -1;
  }

  /* Initialize Channel Scan Result list to be Sent */
  memset(&chscan_results, 0, sizeof(chscan_results));
  chscan_results.num_of_results = 0;
  ieee1905_glist_init(&chscan_results.chscan_result_list);

  i5TraceInfo("Sending Requested {STORED} Channel Scan Report Message to [" I5_MAC_FMT "]\n",
    I5_MAC_PRM(pDeviceController->DeviceId));

  /* Create Empty Channel Scan Results from Channel Scan Request */
  i5DmCreateEmptyChScanResultsFmRequest(chscan_req, &chscan_results, in_status_code);

  /* Copy Channel Scan Results from Results stored in Self Device, including neighbors */
  i5DmCopyChScanResults(&(i5_dm_network_topology.selfDevice->stored_chscan_results),
    &chscan_results, in_status_code, TRUE);

  /* Send Channel Scan Request Message to a Multi AP Device */
  i5MessageChannelScanReportSend(pDeviceController, &chscan_results);

  /* Free the memory allocated for Channel Scan Result Msg structure */
  i5DmChannelScanResultInfoFree(&chscan_results);

  return 0;
}

/* Send tunnel message to controller */
int ieee1905_send_tunneled_msg(ieee1905_tunnel_msg_t *tunnel_msg)
{
  i5_dm_device_type *pDeviceController = NULL;

  pDeviceController = i5DmFindController();
  if (!pDeviceController) {
    i5TraceError("controller device does not exist\n");
    return -1;
  }
  i5MessageTunneledMessageSend(pDeviceController, tunnel_msg);
  return 0;
}

/* Send association status notification to controller */
int ieee1905_send_association_status_notification(
	ieee1905_association_status_notification *assoc_notif)
{
  i5_dm_device_type *pDeviceController = NULL;

  if (!assoc_notif) {
    i5TraceError("Invalid argument to prepare association status notification \n");
    return -1;
  }
  pDeviceController = i5DmFindController();
  if (!pDeviceController) {
    i5TraceError("controller device does not exist\n");
    return -1;
  }
  i5MessageAssociationStatusNotificationSend(pDeviceController->psock, pDeviceController->DeviceId,
    assoc_notif);
  return 0;
}

/* Send CAC request message */
int
ieee1905_send_cac_request(i5_dm_device_type *pDevice, ieee1905_cac_rqst_list_t *cac_list)
{
  int ret;

  if (pDevice == NULL) {
    i5TraceError("Neighbor device does not exist\n");
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (pDevice->psock == NULL) {
    i5TraceError("Device["I5_MAC_DELIM_FMT"] psock is NULL\n", I5_MAC_PRM(pDevice->DeviceId));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  ret = i5MessageCACRequestSend(pDevice, cac_list);

  return ret;
}

/* Send CAC termination message */
int
ieee1905_send_cac_termination(i5_dm_device_type *pDevice,
  ieee1905_cac_termination_list_t *cac_term_list)
{
  int ret;

  if (pDevice == NULL) {
    i5TraceError("Neighbor device does not exist\n");
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  if (pDevice->psock == NULL) {
    i5TraceError("Device["I5_MAC_DELIM_FMT"] psock is NULL\n", I5_MAC_PRM(pDevice->DeviceId));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  ret = i5MessageCACTerminationSend(pDevice, cac_term_list);

  return ret;
}

/* Send Backhaul STA Capability Query message from controller to agent */
int
ieee1905_send_backhaul_sta_capability_query(i5_dm_device_type *pDevice)
{
  if (pDevice == NULL) {
    i5TraceError("Neighbor Device does not exists\n");
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  i5MessageBackhaulSTACapabilityQuerySend(pDevice->psock, pDevice->DeviceId);

  return IEEE1905_OK;
}

int
ieee1905_calc_lanifnames_list_bufsize(void)
{
	return NVRAM_MAX_VALUE_LEN * WLIFU_MAX_NO_BRIDGE;
}

char *
ieee1905_get_all_lanifnames_list(void)
{
	int ifnames_listsz = ieee1905_calc_lanifnames_list_bufsize();

	if (g_ifnames_list) {
		goto end;
	}
	g_ifnames_list = ( char *)calloc(1, ifnames_listsz);
	if (!g_ifnames_list) {
		i5TraceDirPrint("malloc error while getting all lan ifnames size: %d\n",
			ifnames_listsz);
		goto end;
	}
	get_all_lanifnames_list(g_ifnames_list, ifnames_listsz);
	i5Trace("lanX_ifnames[%s] size [%d]\n", g_ifnames_list, ifnames_listsz);
end:
	return g_ifnames_list;
}

int
ieee1905_find_in_all_lanifnames_list(char const *ifname)
{
	char *ifnames_list = ieee1905_get_all_lanifnames_list();
	int ret = 0;

	if (find_in_list(ifnames_list, ifname)) {
		ret = 1;
	}
	i5Trace("%s%s found in lan ifnames list\n", ifname, ret ? "" : " NOT");

	return ret;
}

void
ieee1905_free_lanifnames_list(void)
{
	if (g_ifnames_list) {
		free(g_ifnames_list);
		g_ifnames_list = NULL;
	}
}

/* Send DPP Chirp TLV in 1905 AutoConfiguration Search */
int
ieee1905_send_controller_search(bool force_send_chirp)
{
  int ret = IEEE1905_FAIL;

  /* Before starting the search, kill the controller search timer */
  if (i5_config.ptmrApSearch != NULL ) {
    i5TraceInfo("Main Controller Search is In Progres. Avoid sending another "
      "Controller Search %s DPP Chirp\n", force_send_chirp ? "WITH" : "WITHOUT");
  } else {
    i5Trace("Controller Search Sent %s DPP Chirp\n", force_send_chirp ? "WITH" : "WITHOUT");
    i5MessageApAutoconfigurationSearchSend(I5_FREQ_BAND_2G, force_send_chirp);
    ret = IEEE1905_OK;
  }
  return ret;
}

/* To add port and protocol for sending the Higher Layer Data with a particular protocol
 * to Higher Layer Application listening on a port
 */
int
ieee1905_add_hld_port_protocol(uint16 port, uint8 protocol)
{
  int ret = 0;
  ieee1905_hld_port_list_t *portlist = NULL;

  /* Add Port to list of type ieee1905_hld_port_list_t */
  portlist = i5DmAddHLDportToList(&i5_config.hld_port_protocol_set, port);
  if (portlist == NULL) {
    ret = -1;
    goto end;
  }
  /* To find whether protocol is already registered or not */
  i5DmAddHLDprotocolToList(portlist, protocol);

end:
  return ret;
}

/* Send Higher layer Data to all the registered HLE's for a protocol */
int
ieee1905SendHLDtoHLE(int protocol, unsigned char *hld_data, unsigned int hld_data_len)
{
  int ret = 0;
  int sockfd = -1;
  ieee1905_hld_port_list_t *portlist = NULL;
  ieee1905_hld_protocol_list_t *protocol_list = NULL;

  i5TraceInfo("Send protocol %d with HLD to HLE's\n", protocol);

  /* Travese List */
  foreach_iglist_item(portlist, ieee1905_hld_port_list_t, i5_config.hld_port_protocol_set) {

    foreach_iglist_item(protocol_list, ieee1905_hld_protocol_list_t, portlist->protocol_list) {
      if (protocol_list->protocol != protocol) {
        continue;
      }
      i5TraceInfo("port = %d , protocol[%d] found\n",portlist->port,  protocol_list->protocol);

      sockfd = i5SocketConnectToServer(I5_SOCKET_LOOPBACK_IP, portlist->port);
      if (sockfd == -1) {
        i5TraceError("Failed to connect to HLE server ipaddr[%s] portno[%d]\n",
          I5_SOCKET_LOOPBACK_IP, portlist->port);
        continue;
      }

      /* Send the payload */
      if (i5SocketSendData(sockfd, (char*)hld_data, hld_data_len) <= 0) {
        i5TraceError("Failed to send Higher layer Data to HLE on port %d\n",
         portlist->port);
        ret = -1;
      }
      i5SocketCloseSockFD(&sockfd);
      break;
    }
  }
  return ret;
}

/* Add Service Prioritization Rule Item to list of type i5_serv_prio_rule_t */
i5_serv_prio_rule_t *ieee1905_add_servprio_rule_to_list(ieee1905_glist_t *serv_prio_rule_list,
  unsigned int rule_id, unsigned char rule_operation_flag,
  unsigned char rule_precedence, unsigned char rule_output, unsigned char rule_flag)
{
  i5_serv_prio_rule_t *serv_prio_rule = NULL;

  serv_prio_rule = (i5_serv_prio_rule_t*)malloc(sizeof(*serv_prio_rule));
  if (!serv_prio_rule) {
    i5TraceDirPrint("malloc error\n");
    goto end;
  }

  bzero(serv_prio_rule, sizeof(*serv_prio_rule));
  serv_prio_rule->rule_id = rule_id;
  serv_prio_rule->rule_operation_flag = rule_operation_flag;
  serv_prio_rule->rule_precedence = rule_precedence;
  serv_prio_rule->rule_output = rule_output;
  serv_prio_rule->rule_flag = rule_flag;
  ieee1905_glist_append(serv_prio_rule_list, (dll_t*)serv_prio_rule);
  i5Trace("Added Service Prioritization Rule Item for rule_id[%d]\n", rule_id);

end:
  return serv_prio_rule;
}

/* Add QoS Management Descriptor Item to list of type i5_qosmgmt_desc_t */
i5_qosmgmt_desc_t *ieee1905_add_qosmgmt_desc_to_list(ieee1905_glist_t *qosmgmt_desc_list,
  unsigned short qm_id, unsigned char *bssid, unsigned char *client_mac,
  unsigned char *descriptor, unsigned int descriptor_len)
{
  i5_qosmgmt_desc_t *qosmgmt_desc = NULL;

  if (!bssid || !client_mac || !descriptor || !descriptor_len) {
    i5TraceDirPrint("Invalid Arguments\n");
    goto end;
  }

  qosmgmt_desc = (i5_qosmgmt_desc_t*)malloc(sizeof(*qosmgmt_desc));
  if (!qosmgmt_desc) {
    i5TraceDirPrint("malloc error\n");
    goto end;
  }

  bzero(qosmgmt_desc, sizeof(*qosmgmt_desc));
  qosmgmt_desc->qm_id = qm_id;
  eacopy(bssid, qosmgmt_desc->bssid);
  eacopy(client_mac, qosmgmt_desc->client_mac);
  qosmgmt_desc->descriptor_len = descriptor_len;

  qosmgmt_desc->descriptor = (unsigned char *)malloc(qosmgmt_desc->descriptor_len);
  if (!qosmgmt_desc->descriptor) {
    i5TraceDirPrint("malloc error\n");
    free(qosmgmt_desc);
    qosmgmt_desc = NULL;
    goto end;
  }
  bzero(qosmgmt_desc->descriptor, qosmgmt_desc->descriptor_len);
  memcpy(qosmgmt_desc->descriptor, descriptor, qosmgmt_desc->descriptor_len);

  ieee1905_glist_append(qosmgmt_desc_list, (dll_t*)qosmgmt_desc);
  i5Trace("Added QoS Management Descriptor Item for qm_id[%d]\n", qm_id);

end:
  return qosmgmt_desc;
}

/* Cleanup Service Prioritization Request Message */
int
ieee1905_cleanup_serv_prio_req_msg(i5_serv_prio_req_msg_t *in_sp_req)
{
  i5_qosmgmt_desc_t *qosmgmt_desc = NULL;

  /* Cleanup Each QoS Management Descriptor List Item's Descriptor */
  foreach_iglist_item(qosmgmt_desc, i5_qosmgmt_desc_t, in_sp_req->qosmgmt_desc_list) {
    if (qosmgmt_desc->descriptor) {
      free(qosmgmt_desc->descriptor);
      qosmgmt_desc->descriptor = NULL;
    }
  }
  /* Cleanup QoS Management Descriptor List */
  i5DmGlistCleanup(&in_sp_req->qosmgmt_desc_list);
  /* Cleanup Service Prioritization Rule List */
  i5DmGlistCleanup(&in_sp_req->serv_prio_rule_list);

  /* Init Service Prioritization Request Message */
  bzero(in_sp_req, sizeof(*in_sp_req));
  ieee1905_glist_init(&in_sp_req->serv_prio_rule_list);
  ieee1905_glist_init(&in_sp_req->qosmgmt_desc_list);

  return IEEE1905_OK;
}

/* Cleanup QoS Management Notification Message */
int
ieee1905_cleanup_qosmgmt_notif_msg(i5_qosmgmt_notif_msg_t *in_qosmgmt_notif)
{
  i5_qosmgmt_desc_t *qosmgmt_desc = NULL;

  /* Cleanup Each QoS Management Descriptor List Item's Descriptor */
  foreach_iglist_item(qosmgmt_desc, i5_qosmgmt_desc_t, in_qosmgmt_notif->qosmgmt_desc_list) {
    if (qosmgmt_desc->descriptor) {
      free(qosmgmt_desc->descriptor);
      qosmgmt_desc->descriptor = NULL;
    }
  }
  /* Cleanup QoS Management Descriptor List */
  i5DmGlistCleanup(&in_qosmgmt_notif->qosmgmt_desc_list);

  /* Init Service Prioritization Request Message */
  bzero(in_qosmgmt_notif, sizeof(*in_qosmgmt_notif));
  ieee1905_glist_init(&in_qosmgmt_notif->qosmgmt_desc_list);

  return IEEE1905_OK;
}
#endif /* MULTIAP */
