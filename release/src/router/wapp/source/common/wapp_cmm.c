/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#ifdef OPENWRT_SUPPORT
#include <libdatconf.h>
#endif
#include "wdev.h"
#include "driver_wext.h"
#include "hotspot.h"
#include "wapp_cmm.h"
#include "wps.h"
#ifdef DPP_SUPPORT
#include "dpp/dpp_wdev.h"
#endif /*DPP_SUPPORT*/

extern struct wapp_drv_ops wapp_drv_wext_ops;
extern struct hotspot_event_ops hs_event_ops;
extern int wapp_iface_init(struct wifi_app *wapp);
struct wifi_app_event_ops wapp_event_ops = {
	.event_get_mbo_neighbor_report = wapp_event_get_neighbor_report_list,
	.event_handle = wapp_event_handle,
	.event_btm_rsp = wapp_event_btm_rsp,
	.event_btm_req = wapp_event_btm_req,
	.event_anqp_req = wapp_event_anqp_req,
	.event_btm_query = wapp_event_btm_query,
	.event_offch_info = wapp_event_offchannel_info,
#ifdef MAP_R2
	.event_wnm_notify = wapp_event_wnm_notify_req,
#endif
};

int wapp_cmd_show_help( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_wdev_query( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_wdev_query_by_req_id( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_wdev_ht_cap_query( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_wdev_vht_cap_query( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_wdev_list( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_ap_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_query_cli( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_set_bss_start( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_get_scan_result( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_set_bss_stop( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_set_txpwr_prctg( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_show_cli_list( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_chn_list_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_bss_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_ap_metric( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_set_sec( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_set_ch( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_set_ssid( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_show_radio_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_show_probe_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_show_blocked_list( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_set_load_thrd( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int wapp_cmd_set_log_level( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);

int wapp_cmm_init(
	struct wifi_app *wapp,
	int drv_mode,
	int opmode,
	int version,
	struct hotspot *hs,
	struct mbo_cfg *mbo,
	struct oce_cfg *oce,
	struct map_info *map,
	struct _RTMP_IAPP *IAPP_ctrl_block)
{
	int ret = 0;
	int i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp->hs = hs;
	wapp->mbo = mbo;
	wapp->oce = oce;
#ifdef MAP_SUPPORT
	wapp->map = map;
	wapp->wsc_save_bh_profile = FALSE;
#endif
	wapp->IAPP_Ctrl_Block = IAPP_ctrl_block;

	dl_list_init(&wapp->dev_list);

	/* kill old IAPP daemon if exists */
	IAPP_PID_Kill();

	// TODO: we should build wdev here for other modules' init processes

#ifdef MBO_SUPPORT
	ret += mbo_init(wapp->mbo);
#endif /* MBO_SUPPORT */
#ifdef OCE_SUPPORT
		ret += oce_init(wapp->oce);
#endif /* OCE_SUPPORT */
	ret += anqp_init(wapp, &hs_event_ops, version);
#ifdef MAP_SUPPORT
	ret += map_init(wapp->map);
#endif /* MAP_SUPPORT */

	/* init daemon neighbor report list */
	os_memset(&wapp->daemon_nr_list, 0, sizeof(DAEMON_NR_LIST));

	wapp->opmode = opmode;
	wapp->drv_mode = drv_mode;

	for (i = 0; i < PROBE_TABLE_SIZE; i++) {
		wapp->probe_entry[i].valid = 0;
	}
	os_memset(&wapp->probe_hash[0], 0, sizeof(wapp->probe_hash));

	return ret;
}

int wapp_socket_and_ctrl_inf_init(
	struct wifi_app *wapp,
	int drv_mode,
	int opmode
)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	/* Initialze event loop */
	if(eloop_init() != 0)
		return -1;

	/* Initialize control interface */
	wapp->w_ctrl_iface = wapp_ctrl_iface_init(wapp);

#ifdef MAP_SUPPORT
	wapp_iface_init(wapp);
#ifdef AUTOROLE_NEGO
	wapp_MapDevRoleNegotiation_init(wapp);
#endif // AUTOROLE_NEGO
#endif

	wapp->drv_ops = &wapp_drv_wext_ops;
	wapp->event_ops = &wapp_event_ops;
	wapp->drv_data = wapp->drv_ops->drv_inf_init(wapp, opmode, drv_mode);

	if (!wapp->drv_data) {
		/* deinit control interface */
		wapp_ctrl_iface_deinit(wapp);
		return -1;

	}

	eloop_register_timeout(1, 0, wapp_periodic_exec, NULL, wapp);

	return 0;
}

int wapp_get_mac_addr_by_ifname(
	char *ifname,
	u8 *mac_addr)
{
    int fd;
    struct ifreq ifr;
    u8 *mac = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , ifname , IFNAMSIZ-1);

    ioctl(fd, SIOCGIFHWADDR, &ifr);

    close(fd);

    mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	os_memcpy(mac_addr, mac, MAC_ADDR_LEN);

    return WAPP_SUCCESS;

}

int get_mac_addr_by_ifname(char *_iface, u8 *mac_addr) {
	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , _iface , IFNAMSIZ-1);

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
		DBGPRINT(RT_DEBUG_OFF, RED("failed to get the hw addr of %s\n"), _iface);
		close(fd);
		return WAPP_UNEXP;
	} else {
		os_memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
	}
	close(fd);
	return WAPP_SUCCESS;
}

int test_inf_status(const char *_iface, u32 if_flag) {
	struct ifreq ifr;
	int sockfd;
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Cannot open socket\n", __func__);
		return FALSE;
	}
	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name , _iface , IFNAMSIZ-1);

	ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	if (ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: ioctl failed\n", __func__);
		close(sockfd);
		return FALSE;
	}

	close(sockfd);
	if (ifr.ifr_flags & if_flag)
		return TRUE;
 	else
 		return FALSE;
}

int wapp_get_wireless_interfaces(struct wifi_app *wapp)
{
	struct iwreq wrq;
	int skfd;
	u8 addr[MAC_ADDR_LEN];
	char *token;
	char buff[1024], *pbuf;
	FILE * fh;
	char * end;
	char name[IFNAMSIZ + 1] = {0};
	char value[350] = {0};
	char *rem_strng = value;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	/* 1. get all local interfaces */
	get_map_parameters(wapp->map, "bss_config_priority", value, NON_DRIVER_PARAM, sizeof(value));

	if (strlen(value) > 0) {
		/* Static Interface list */
		token = strtok_r(rem_strng, ";", &rem_strng);
		/* Read each device line */
		while(token)
		{
			os_memset(name,0,IFNAMSIZ + 1);
			os_memcpy(name, token, os_strlen(token));
			/* Got it, print info about this interface */
			os_memset(&wrq, 0, sizeof(struct iwreq));
			/* check if the interface is a wireless interface */
			strncpy(wrq.ifr_name, name, IFNAMSIZ - 1);

			if ((ioctl(skfd, SIOCGIWNAME, &wrq)) >= 0) {
				/* Valid wireless interface, create a wdev for this interface */
				if (get_mac_addr_by_ifname(name, addr) != WAPP_SUCCESS) {
					DBGPRINT(RT_DEBUG_OFF, RED("failed to get hw addr for %s\n"), name);
				} else {
					wapp_dev_create(wapp, name, if_nametoindex(name), addr);
				}
			}
			token = strtok_r(rem_strng, ";", &rem_strng);
		}
	} else {
		/* Dynamic interface list */
		fh = fopen(PROC_NET_DEV, "r");
		if (fh != NULL) {
			/* Ignore two lines */
			fgets(buff, sizeof(buff), fh);
			fgets(buff, sizeof(buff), fh);
			/* Read each device line */
			while (fgets(buff, sizeof(buff), fh)) {
				char *s;

				if ((buff[0] == '\0') || (buff[1] == '\0'))
					continue;
				/* Extract interface name */
				pbuf = buff;
				while (isspace(*pbuf))
						pbuf++;
				end = strrchr(pbuf, ':');
				if((end == NULL) || (((end - pbuf) + 1) > sizeof(name)))
					s = NULL;
				else {
					os_memset(name, 0, IFNAMSIZ + 1);
					memcpy(name, pbuf, (end - pbuf));
					name[end - pbuf] = '\0';
					s = end;
				}
				if (s) {
					/* Got it, print info about this interface */
					os_memset(&wrq, 0, sizeof(struct iwreq));
					/* check if the interface is a wireless interface */
					strncpy(wrq.ifr_name, name, IFNAMSIZ-1);
					if ((ioctl(skfd, SIOCGIWNAME, &wrq)) >= 0) {
						/* Valid wireless interface, create a wdev for this interface */
						if (get_mac_addr_by_ifname(name, addr) != WAPP_SUCCESS) {
							DBGPRINT(RT_DEBUG_OFF, RED("failed to get hw addr for %s\n"), name);
						} else {
							wapp_dev_create(wapp, name, if_nametoindex(name), addr);
						}
					}
				}
			}
			fclose(fh);
		}
	}
	close(skfd);

	return WAPP_SUCCESS;
}

void wapp_periodic_exec(void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = (struct wifi_app*) user_ctx;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Error! wapp is NULL.\n",__FUNCTION__);
		return;
	}

#ifdef MAP_SUPPORT
	map_periodic_exec(wapp);
#endif /* MAP_SUPPORT */

	eloop_register_timeout(1, 0, wapp_periodic_exec, NULL, wapp);
}

/* send wapp request to driver */
int wapp_req_send(struct wifi_app *wapp,
										const char *iface,
										struct wapp_req *req)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
#if 1
	if(test_inf_status(iface, IFF_UP) == FALSE) {
		DBGPRINT(RT_DEBUG_TRACE, "inf name [%s] is not up\n",iface);
		return WAPP_NOT_INITIALIZED;
	}
#endif
	ret = wapp->drv_ops->drv_wapp_req(wapp->drv_data,
									iface,
								    req);
	return ret;
}


int wapp_query_wdev(struct wifi_app *wapp,
										const char *iface)
{
	int ret = 0;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!iface) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	req.req_id = WAPP_DEV_QUERY_REQ;
	req.data_len = 0;
	req.data.ifindex = if_nametoindex(iface);
	DBGPRINT(RT_DEBUG_TRACE, "%s - inf name [%s], ifindex = %u\n",__FUNCTION__,iface, req.data.ifindex);

	ret = wapp_req_send(wapp, iface, &req);

	return ret;
}

int wapp_query_wdev_by_req_id(struct wifi_app *wapp,
										const char *iface,
										u8 req_id)
{
	int ret;
	struct wapp_req req;
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!iface) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);

	if (!wdev)
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;

	req.req_id = req_id;
	/*req.data_len = sizeof(wapp_req_data);*/
	req.data.ifindex = wdev->ifindex;

	ret = wapp_req_send(wapp, iface, &req);
	return ret;
}

int wapp_query_wdev_ht_cap(struct wifi_app *wapp,
										const char *iface)
{
	int ret;
	struct wapp_req req;
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!iface) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);

	if (!wdev)
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;

	req.req_id = WAPP_HT_CAP_QUERY_REQ;
	req.data.ifindex = wdev->ifindex;

	ret = wapp_req_send(wapp, iface, &req);
	return ret;
}

int wapp_query_wdev_vht_cap(struct wifi_app *wapp,
										const char *iface)
{
	int ret;
	struct wapp_req req;
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!iface) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);

	if (!wdev)
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;

	req.req_id = WAPP_VHT_CAP_QUERY_REQ;
	/*req.data_len = sizeof(wapp_req_data);*/
	req.data.ifindex = wdev->ifindex;

	ret = wapp_req_send(wapp, iface, &req);
	return ret;
}

int wapp_query_cli(struct wifi_app *wapp,
							const char *iface,
							const u8 *mac_addr)
{
	int ret;
	struct wapp_req req;
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);

	if (!wdev)
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;

	req.req_id = WAPP_CLI_QUERY_REQ;
	req.data.ifindex = wdev->ifindex;
	COPY_MAC_ADDR(req.data.mac_addr, mac_addr);

	ret = wapp_req_send(wapp, iface, &req);
	return ret;
}


int wapp_set_bss_start(struct wifi_app *wapp,
	const char *iface)
{
	int ret;
	struct wapp_req req;
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);

	if (!wdev)
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;

	if (wdev->dev_type != WAPP_DEV_TYPE_AP) {
		printf("%s wdev type is not AP, do nothing.", __func__);
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;
	}

	req.req_id = WAPP_BSS_START_REQ;
	req.data.ifindex = wdev->ifindex;

	ret = wapp_req_send(wapp, iface, &req);
	return ret;
}


int wapp_set_bss_stop(struct wifi_app *wapp,
	const char *iface)
{
	int ret;
	struct wapp_req req;
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);

	if (!wdev)
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;

	if (wdev->dev_type != WAPP_DEV_TYPE_AP) {
		printf("%s wdev type is not AP, do nothing.", __func__);
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;
	}

	req.req_id = WAPP_BSS_STOP_REQ;
	req.data.ifindex = wdev->ifindex;

	ret = wapp_req_send(wapp, iface, &req);
	return ret;
}

int wapp_set_bssload_thrd(struct wifi_app *wapp,
	const char *iface,
	char *high_thrd,
	char *low_thrd)
{
	int ret;
	struct wapp_req req;
	struct wapp_dev *wdev = NULL;

	if(!iface || !high_thrd || !low_thrd) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);
	if (!wdev)
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;

	req.req_id = WAPP_BSS_LOAD_THRD_SET_REQ;
	req.data.ifindex = wdev->ifindex;
	req.data.bssload_thrd.high_bssload_thrd = (uint8_t)atoi(high_thrd);
	req.data.bssload_thrd.low_bssload_thrd = (uint8_t)atoi(low_thrd);
	ret = wapp_req_send(wapp, iface, &req);
	return ret;
}

int wapp_set_tx_power_percentage(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	u8 pwr_prctg)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"%s:\n"
		"\t ifname = %s\n"
		"\t pwr_prctg = %u(%%)\n",
		__func__,
		wdev->ifname, pwr_prctg);

	req.req_id = WAPP_TXPWR_PRCTG_REQ;
	req.data.ifindex = wdev->ifindex;
	req.data.value = pwr_prctg;

	ret = wapp_req_send(wapp, wdev->ifname, &req);

	return ret;
}

int wapp_set_steering_policy(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	wdev_steer_policy *policy)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	req.req_id = WAPP_STEERING_POLICY_SET_REQ;
	req.data.ifindex = wdev->ifindex;
	os_memcpy(&req.data.str_policy, policy, sizeof(wdev_steer_policy));

	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;
}


static void wapp_init_features(
	struct wifi_app *wapp,
	struct wapp_dev *wdev)
{
	char cmd[MAX_CMD_MSG_LEN];

	if(wdev) {
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		sprintf(cmd, "iwpriv %s set MapChannelEn=%d", wdev->ifname, wapp->map->quick_ch_change);
		system(cmd);
	}

#ifdef KV_API_SUPPORT
	/* enable wnm */
	driver_wnm_btm_onoff(wapp->drv_data, wdev->ifname, 1);
	wdev->wnm_enbale = 1;
	DBGPRINT(RT_DEBUG_TRACE, "%s: wdev->wnm_enbale = %d\n", __func__, wdev->wnm_enbale);

	/* enable rrm */
	driver_rrm_onoff(wapp->drv_data, wdev->ifname, 1);
	wdev->rrm_enable = 1;
	DBGPRINT(RT_DEBUG_TRACE, "%s: wdev->rrm_enable = %d\n", __func__, wdev->rrm_enable);
#endif /*KV_API_SUPPORT*/
}

int wapp_set_ap_config(
	struct wifi_app *wapp,
	struct wapp_dev *wdev)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	req.req_id = WAPP_AP_CONFIG_SET_REQ;
	req.data.ifindex = wdev->ifindex;
	req.data.ap_conf.sta_report_on_cop = wapp->map->sta_report_on_cop;
	req.data.ap_conf.sta_report_not_cop = wapp->map->sta_report_not_cop;
	req.data.ap_conf.rssi_steer = wapp->map->rssi_steer;

	ret = wapp_req_send(wapp, wdev->ifname, &req);

	wapp_init_features(wapp, wdev);
	return ret;
}

int wapp_query_bssload(
	struct wifi_app *wapp,
	struct wapp_dev *wdev)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	req.req_id = WAPP_BSSLOAD_QUERY_REQ;
	req.data.ifindex = wdev->ifindex;

	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;
}

int wapp_query_he_cap(
	struct wifi_app *wapp,
	struct wapp_dev *wdev)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	req.req_id = WAPP_HECAP_QUERY_REQ;
	req.data.ifindex = wdev->ifindex;

	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;
}

int wapp_query_sta_rssi(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	const u8 *mac_addr)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	req.req_id = WAPP_STA_RSSI_QUERY_REQ;
	req.data.ifindex = wdev->ifindex;
	COPY_MAC_ADDR(req.data.mac_addr, mac_addr);

	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;
}

int wapp_query_apcli_rssi(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	const u8 *mac_addr)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	req.req_id = WAPP_APCLI_RSSI_QUERY_REQ;
	req.data.ifindex = wdev->ifindex;
	COPY_MAC_ADDR(req.data.mac_addr, mac_addr);

	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;
}
/* Fucntion is required to fetch the scan results from the driver
*/
int wapp_query_scan_result(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	char more_data)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	os_memset(&req, 0, sizeof(req));
	req.req_id = WAPP_GET_SCAN_RESULTS;
	req.data.ifindex = wdev->ifindex;

	req.data.value = wdev->scan_cookie;

	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;
}

/*
	Function sends an OID to driver to perform single step in APCLI PBC state machine
*/
int wapp_trigger_wsc_pbc_exec(
	struct wifi_app *wapp,
	struct wapp_dev *wdev)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	req.req_id = WAPP_WSC_PBC_EXEC;
	req.data.ifindex = wdev->ifindex;

	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;

}

#ifdef HOSTAPD_MAP_SUPPORT
int	wapp_get_hapd_wifi_profile(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct wireless_setting *wifi_profile,
	unsigned int bh_idx,
	BOOLEAN is_backhaul_bss)
{
	char param_prefix[6] = {0};
	char file_name[64] = {0};
	char value[128] = {0};
	char param[32] = {0};

	os_memset(param_prefix, 0, sizeof(param_prefix));
	if(is_backhaul_bss == TRUE) {
		if(bh_idx < RADIO_5G)
			os_snprintf(param_prefix, sizeof(param_prefix), "bh%d", bh_idx);
		else {
			printf("Invalid bh profile index %d\n", bh_idx);
			return 0;
		}
	} else
		os_strcpy(param_prefix, "");

	os_snprintf(file_name, sizeof(file_name), "/etc/hostapd_%s_map.conf", wdev->ifname);

	/*read ssid*/
	os_snprintf(param, sizeof(param), "%sssid", param_prefix);
	get_parameters(file_name, param, value, NON_DRIVER_PARAM, sizeof(wifi_profile->Ssid));
	os_memcpy(wifi_profile->Ssid, value, os_strlen(value));

	/*read wpa*/
	os_snprintf(param, sizeof(param), "%swpa", param_prefix);
	get_parameters(file_name, param, value, NON_DRIVER_PARAM, sizeof(value));
	if (value[0] == '2') {
		os_snprintf(param, sizeof(param), "%swpa_key_mgmt", param_prefix);
		get_parameters(file_name, param, value, NON_DRIVER_PARAM, sizeof(value));
		if(os_memcmp(value, "WPA-PSK", 7) == 0)
			wifi_profile->AuthMode = 0x20;		/*WPA2PSK*/
	} else {
		wifi_profile->AuthMode = 0x01;		/*OPEN*/
	}

	/*read wpa_passphrase*/
	os_snprintf(param, sizeof(param), "%swpa_passphrase", param_prefix);
	get_parameters(file_name, param, value, NON_DRIVER_PARAM, sizeof(wifi_profile->WPAKey));
	os_memcpy(wifi_profile->WPAKey, value, os_strlen(value));

	/*read rsn_pairwise*/
	os_snprintf(param, sizeof(param), "%srsn_pairwise", param_prefix);
	get_parameters(file_name, param, value, NON_DRIVER_PARAM, sizeof(value));
	if(os_memcmp(value, "CCMP", 4) == 0)
		wifi_profile->EncrypType = 0x08;			/*CCMP*/

	/*read map_vendor_extension*/
	os_snprintf(param, sizeof(param), "%smap_vendor_extension", param_prefix);
	get_parameters(file_name, param, value, NON_DRIVER_PARAM, sizeof(value));
	wifi_profile->map_vendor_extension = atoi(value);			/*map_vendor_extension*/

	/*read bh mac address*/
	if(is_backhaul_bss == TRUE) {
		os_snprintf(param, sizeof(param), "%smacaddr", param_prefix);
		get_parameters(file_name, param, value, NON_DRIVER_PARAM, sizeof(value));
		hwaddr_aton(value, wifi_profile->mac_addr);
	}

	return 0;
}

int wapp_set_hapd_wifi_profile(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct wireless_setting *wifi_profile,
	unsigned int bh_idx,
	BOOLEAN is_backhaul_profile)
{
	char param_prefix[6];
	char file_name[64] = {0};
	char value[128] = {0};
	char param[32] = {0};

	os_memset(param_prefix, 0, sizeof(param_prefix));
	if (is_backhaul_profile == TRUE){
		struct wapp_dev *wdev_wifi_profile = NULL;

		if(!(wifi_profile->map_vendor_extension & BIT_BH_BSS)) {
			DBGPRINT(RT_DEBUG_OFF, "Dont set fronthaul profile %s as bh for %s\n",wifi_profile->Ssid, wdev->ifname);
			return 0;
		}
		wdev_wifi_profile = wapp_dev_list_lookup_by_mac_and_type(wapp, wifi_profile->mac_addr, WAPP_DEV_TYPE_AP);

		if (wdev_wifi_profile) {
			if (bh_idx < RADIO_5GH)
				os_snprintf(param_prefix, sizeof(param_prefix), "bh%d", bh_idx);
			else {
				DBGPRINT(RT_DEBUG_OFF, "LINE %d:Invalid bh profile index %d\n", __LINE__, bh_idx);
				return 0;
			}
		} else {
			DBGPRINT(RT_DEBUG_OFF, "Error!! bh wdev not found\n");
			return 0;
		}
	} else
		os_strcpy(param_prefix, "");

	os_snprintf(file_name, sizeof(file_name), "/etc/hostapd_%s_map.conf", wdev->ifname);

	/*set ssid*/

	os_snprintf(param, sizeof(param), "%sssid", param_prefix);
	set_parameters(file_name, param, (char *)wifi_profile->Ssid, NON_DRIVER_PARAM);

	/*set wpa2psk*/
	if (wifi_profile->AuthMode == 0x20) {
		/*set wpa*/
		os_snprintf(param, sizeof(param), "%swpa", param_prefix);
		set_parameters(file_name, param, "2", NON_DRIVER_PARAM);

		/*set wpa_key_mgmt*/
		os_snprintf(param, sizeof(param), "%swpa_key_mgmt", param_prefix);
		set_parameters(file_name, param, "WPA-PSK", NON_DRIVER_PARAM);

		/*set rsn_pairwise*/
		if (wifi_profile->EncrypType == 0x08) {
			os_snprintf(param, sizeof(param), "%srsn_pairwise", param_prefix);
			set_parameters(file_name, param, "CCMP", NON_DRIVER_PARAM);
		}

		/*set wpa_passphrase*/
		os_snprintf(param, sizeof(param), "%swpa_passphrase", param_prefix);
		set_parameters(file_name, param, (char *)wifi_profile->WPAKey, NON_DRIVER_PARAM);
	} else if(wifi_profile->AuthMode == 0x01) { /* OPEN */
		/*set wpa*/
		os_snprintf(param, sizeof(param), "%swpa", param_prefix);
		set_parameters(file_name, param, "0", NON_DRIVER_PARAM);


		/*set wpa_key_mgmt*/
		os_snprintf(param, sizeof(param), "%swpa_key_mgmt", param_prefix);
		set_parameters(file_name, param, "", NON_DRIVER_PARAM);

		/*set rsn_pairwise*/
		if (wifi_profile->EncrypType == 0x08) {
			os_snprintf(param, sizeof(param), "%srsn_pairwise", param_prefix);
			set_parameters(file_name, param, "", NON_DRIVER_PARAM);
		}

		/*set wpa_passphrase*/
		os_snprintf(param, sizeof(param), "%swpa_passphrase", param_prefix);
		set_parameters(file_name, param, "", NON_DRIVER_PARAM);
	}
		else
			DBGPRINT(RT_DEBUG_OFF, "wrong bh_profile config\n");

	/*set map_vendor_extension*/
	os_snprintf(param, sizeof(param), "%smap_vendor_extension", param_prefix);
	os_snprintf(value, sizeof(value), "%c", wifi_profile->map_vendor_extension);
	set_parameters(file_name, param, value, NON_DRIVER_PARAM);

	if (is_backhaul_profile == TRUE) {
		os_snprintf(param, sizeof(param), "%smacaddr", param_prefix);
		os_snprintf(value, sizeof(value), "%02x:%02x:%02x:%02x:%02x:%02x",
			PRINT_MAC(wifi_profile->mac_addr));
		set_parameters(file_name, param, value, NON_DRIVER_PARAM);
	}

	return 0;
}
#endif /* HOSTAPD_MAP_SUPPORT */

/*
*	Function is called to send an OID to fronthaul BSS to inform about
*	profile parameters of one of the backhaul BSS present on the same device.
*/
int wapp_set_bh_wsc_profile(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct wireless_setting *bh_wsc_profile)
{
	int ret;
	struct wapp_req req;
	BOOLEAN auth_reset_required = FALSE;
	short auth_mode;

#ifdef HOSTAPD_MAP_SUPPORT
	int bh_idx = 0;
	BOOLEAN bh_profile_update = FALSE;
	struct wireless_setting hapd_bh_wsc_profile = {0};
#endif /* HOSTAPD_MAP_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	os_memset(&req, 0, sizeof(struct wapp_req));
	req.req_id = WAPP_WSC_SET_BH_PROFILE;
	req.data.ifindex = wdev->ifindex;

	req.data.bh_wsc_profile.SSID.SsidLength = os_strlen((char *)bh_wsc_profile->Ssid);
	os_memcpy(req.data.bh_wsc_profile.SSID.Ssid,
		bh_wsc_profile->Ssid, os_strlen((char *)bh_wsc_profile->Ssid));
	COPY_MAC_ADDR(req.data.bh_wsc_profile.MacAddr, bh_wsc_profile->mac_addr);
#ifdef MAP_SUPPORT		/*Add WAP3 support with MAP_R1*/
	if (bh_wsc_profile->AuthMode & WSC_AUTHTYPE_SAE) {
		auth_reset_required = TRUE;
		auth_mode = bh_wsc_profile->AuthMode;
		DBGPRINT(RT_DEBUG_ERROR," wpa3 config, set bh config as wpa2");
		bh_wsc_profile->AuthMode = WSC_AUTHTYPE_WPA2PSK;
	}
#endif
	req.data.bh_wsc_profile.AuthType = bh_wsc_profile->AuthMode;
	req.data.bh_wsc_profile.EncrType = bh_wsc_profile->EncrypType;
	req.data.bh_wsc_profile.bss_role = bh_wsc_profile->map_vendor_extension & BIT_BH_BSS;
	os_memcpy(req.data.bh_wsc_profile.Key,
		bh_wsc_profile->WPAKey, os_strlen((char *)bh_wsc_profile->WPAKey));
	DBGPRINT(RT_DEBUG_ERROR,"wapd sends WPAPSK = %s\n", req.data.bh_wsc_profile.Key);
	req.data.bh_wsc_profile.KeyLength = os_strlen((char *)bh_wsc_profile->WPAKey);
	ret = wapp_req_send(wapp, wdev->ifname, &req);
	if (auth_reset_required)
		bh_wsc_profile->AuthMode = auth_mode;

#ifdef HOSTAPD_MAP_SUPPORT
	for (bh_idx = RADIO_24G; bh_idx < RADIO_5GH; bh_idx++) {
		wapp_get_hapd_wifi_profile(wapp, wdev, &hapd_bh_wsc_profile, bh_idx, TRUE);
		if(!(os_memcmp(bh_wsc_profile->mac_addr, hapd_bh_wsc_profile.mac_addr, ETH_ALEN))) {
			if(	(bh_wsc_profile->AuthMode == hapd_bh_wsc_profile.AuthMode) &&
				(bh_wsc_profile->EncrypType == hapd_bh_wsc_profile.EncrypType) &&
				(bh_wsc_profile->map_vendor_extension ==  hapd_bh_wsc_profile.map_vendor_extension)&&
				(hapd_bh_wsc_profile.map_vendor_extension & BIT_BH_BSS) &&
				!(os_memcmp(bh_wsc_profile->Ssid, hapd_bh_wsc_profile.Ssid, os_strlen((char *)bh_wsc_profile->Ssid))) &&
				!(os_memcmp(bh_wsc_profile->WPAKey, hapd_bh_wsc_profile.WPAKey, os_strlen((char *)bh_wsc_profile->WPAKey)))
				) {
				DBGPRINT(RT_DEBUG_ERROR,"new bh profile is already updated to hostapd conf\n");
				bh_profile_update = FALSE;
			} else {
				DBGPRINT(RT_DEBUG_ERROR,"Update bh profile %d\n", bh_idx);
				bh_profile_update = TRUE;
			}
			break;
		} else if(bh_idx == RADIO_5GL) { /* BH MAC address not found, update as a new profile (first time update)*/
			bh_idx = wdev->bh_profile_id;
			if(wdev->bh_profile_id < RADIO_5GL)
				wdev->bh_profile_id++;
			else
				wdev->bh_profile_id = 0;
			bh_profile_update = TRUE;
			break;
		}
	}

	if (bh_profile_update == TRUE) {
		DBGPRINT(RT_DEBUG_ERROR,"Update bh profile %d\n", bh_idx);
		wapp_set_hapd_wifi_profile(wapp, wdev, bh_wsc_profile, bh_idx, TRUE);
		wdev->i_need_hostapd_reload = TRUE;
	}
#endif /* HOSTAPD_MAP_SUPPORT */
	return ret;

}

int wapp_send_null_frames(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	unsigned char *sta_addr,
	unsigned int count)
{
	int ret;
	struct wapp_req req;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	req.req_id = WAPP_SEND_NULL_FRAMES;
	req.data.ifindex = wdev->ifindex;
	req.data.value = count;
	os_memcpy(req.data.mac_addr, sta_addr, ETH_ALEN);

	ret = wapp_req_send(wapp, wdev->ifname, &req);

	return ret;
}


void wapp_csa_time_out_handler(void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = (struct wifi_app *)eloop_data;
	struct apcli_association_info *apcli_stat_info = &wapp->cli_assoc_info;

	map_operating_channel_info(wapp);
	wapp->csa_notif_received = FALSE;

	if (wapp->link_change_notif_pending == TRUE)
	{
		apcli_stat_info->current_channel = wapp->csa_new_channel;
		wapp_send_1905_msg(wapp, WAPP_APCLI_ASSOC_STAT_CHANGE, sizeof(struct apcli_association_info),
			(char *)&wapp->cli_assoc_info);
		wapp->link_change_notif_pending = FALSE;
	}
}

void wapp_event_handle(struct wifi_app *wapp, struct wapp_event *event)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s event_id %d\n", __func__,event->event_id);
	switch(event->event_id)
	{
		case WAPP_DEV_QUERY_RSP:
			wdev_query_rsp_handle(wapp, &event->data);
			break;
		case WAPP_HT_CAP_QUERY_RSP:
			wdev_ht_cap_query_rsp_handle(wapp, event->ifindex, &event->data.ht_cap);
			break;
		case WAPP_VHT_CAP_QUERY_RSP:
			wdev_vht_cap_query_rsp_handle(wapp, event->ifindex, &event->data.vht_cap);
			break;
		case WAPP_MISC_CAP_QUERY_RSP:
			wdev_misc_cap_query_rsp_handle(wapp, event->ifindex, &event->data.misc_cap);
			break;
		case WAPP_CLI_QUERY_RSP:
			wdev_cli_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CLI_LIST_QUERY_RSP:
			wdev_cli_list_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CLI_JOIN_EVENT:
			wdev_cli_join_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CLI_LEAVE_EVENT:
			wdev_cli_leave_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CLI_PROBE_EVENT:
			wdev_cli_probe_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CHN_LIST_RSP:
			wdev_chn_list_query_rsp_handle(wapp, event->ifindex, &event->data.chn_list);
			break;
		case WAPP_OP_CLASS_RSP:
			wdev_op_class_query_rsp_handle(wapp, event->ifindex, &event->data.op_class);
			break;
		case WAPP_BSS_INFO_RSP:
			wdev_bss_info_query_rsp_handle(wapp, event->ifindex, &event->data.bss_info);
			break;
		case WAPP_AP_METRIC_RSP:
			wdev_ap_metric_query_rsp_handle(wapp, event->ifindex, &event->data.ap_metrics);
			break;
#ifdef MAP_R2
		case WAPP_RADIO_METRIC_RSP:
			wdev_radio_metric_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
#endif
		case WAPP_CH_UTIL_QUERY_RSP:
			wdev_ch_util_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_AP_CONFIG_RSP:
			wdev_ap_config_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_APCLI_QUERY_RSP:
			wdev_apcli_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
#ifdef MAP_SUPPORT // TODO: move to MAP
		case MAP_BH_STA_WPS_DONE:
			map_event_bh_sta_wap_done(wapp, event->ifindex, &event->data);
			break;
		case MAP_TRIGGER_RSSI_STEER:
			map_event_str_sta_rsp_handle(wapp, event->ifindex, &event->data);
			break;
#endif
		case WAPP_RCEV_BCN_REPORT:
			wdev_bcn_report_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_RCEV_BCN_REPORT_COMPLETE:
			wdev_bcn_report_complete_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_BSSLOAD_RSP:
			wdev_bssload_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_RCEV_MONITOR_INFO:
			wdev_mnt_info_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_STA_RSSI_RSP:
			wdev_sta_rssi_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CLI_ACTIVE_CHANGE:
			wdev_cli_active_change_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_BSS_STATE_CHANGE:
			wdev_bss_stat_change_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CH_CHANGE:
			wdev_chn_change_rsp_handle(wapp, event->ifindex, &event->data);
			map_operating_channel_info(wapp);
			break;
		case WAPP_TX_POWER_CHANGE:
			/* TBD*/
			break;
		case WAPP_APCLI_ASSOC_STATE_CHANGE:
			wdev_apcli_assoc_stat_change_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_APCLI_ASSOC_STATE_CHANGE_VENDOR10:
			 wdev_apcli_assoc_stat_change_handle_vendor10(wapp, event->ifindex, &event->data);
			break;
		case WAPP_BSSLOAD_CROSSING:
			wdev_bssload_crossing_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CSA_EVENT:
			wdev_csa_event_rsp_handle(wapp, event->ifindex, &event->data);
			if(!wapp->map->TurnKeyEnable)
				break;
			wapp->csa_notif_received = TRUE;

			if(wapp->map->quick_ch_change ==  FALSE) {
				struct wapp_dev *temp_wdev = NULL;
				struct dl_list *dev_list;
				char local_command[128];

				dev_list = &wapp->dev_list;
				dl_list_for_each(temp_wdev, dev_list, struct wapp_dev, list){
					if (temp_wdev && temp_wdev->dev_type == WAPP_DEV_TYPE_STA
						&& temp_wdev->ifindex == event->ifindex) {
						os_memset(local_command, 0, sizeof(local_command));
						os_snprintf(local_command, sizeof(local_command), "iwpriv %s set ApCliEnable=0",
							temp_wdev->ifname);
						system(local_command);
						break;
					}
				}
			eloop_register_timeout(10, 0, wapp_csa_time_out_handler, wapp, NULL);
			}
			break;
		case WAPP_STA_CNNCT_REJ:
			wdev_sta_cnnct_rej_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_APCLI_RSSI_RSP:
			wdev_apcli_rssi_query_rsp_handle(wapp, event->ifindex, &event->data);
			break;
		case WAPP_WSC_SCAN_COMP_NOTIF:
			wdev_process_wsc_scan_comp(wapp,
				 event->ifindex, &event->data);
			break;
		case WAPP_SCAN_RESULT_RSP:
			if (wapp->map->off_ch_scan_state.ch_scan_state == CH_SCAN_IDLE) {
				wdev_handle_scan_results(wapp, event->ifindex, &event->data);
			} else {
				wdev_handle_off_ch_scan_results(wapp, event->ifindex, &event->data);
			}
			break;
		case WAPP_MAP_VENDOR_IE:
			wdev_handle_map_vend_ie_evt(wapp, event->ifindex, &event->data);
			break;
#ifdef DPP_SUPPORT
		case WAPP_DPP_ACTION_FRAME_RECEIVED:
			DBGPRINT(RT_DEBUG_TRACE,"action frame event \n");
			wdev_get_dpp_action_frame(wapp, event->ifindex, &event->data);
			break;
		case WAPP_DPP_ACTION_FRAME_STATUS:
			wdev_handle_dpp_frm_tx_status(wapp, event->ifindex, &event->data);
			break;
#endif /*DPP_SUPPORT*/
		case WAPP_MAP_WSC_CONFIG:
			wdev_handle_wsc_config_write(wapp, event->ifindex, &event->data);
			break;
		case WAPP_WSC_EAPOL_START_NOTIF:
			wdev_handle_wsc_eapol_notif(wapp, event->ifindex, &event->data);
			break;
		case WAPP_WSC_EAPOL_COMPLETE_NOTIF:
			wdev_handle_wsc_eapol_end_notif(wapp, event->ifindex, &event->data);
			break;
		case WAPP_SCAN_COMPLETE_NOTIF:
			if (wapp->map->off_ch_scan_state.ch_scan_state == CH_SCAN_IDLE) {
				wdev_handle_scan_complete_notif(wapp, event->ifindex, &event->data);
			}
			break;
		case WAPP_A4_ENTRY_MISSING_NOTIF:
			wdev_handle_a4_entry_missing_notif(wapp, event->ifindex, &event->data);
			break;
		case WAPP_RADAR_DETECT_NOTIF:
			wdev_handle_radar(wapp, event->ifindex, &event->data);
			break;
		case WAPP_CAC_STOP:
			DBGPRINT(RT_DEBUG_ERROR,"\n CAC stop received");
			wdev_handle_cac_stop(wapp, event->ifindex, &event->data.cac_info.channel, event->data.cac_info.ret, FALSE);
			break;
#ifdef MAP_R2
		case WAPP_STA_DISASSOC_EVENT:
			DBGPRINT(RT_DEBUG_ERROR,"WAPP_STA_DISASSOC_EVENT\n");
			wdev_sta_disassoc_stats_handle(wapp, event->ifindex, &event->data);
			break;
#endif
		case WAPP_CAC_PERIOD_EVENT:
			DBGPRINT(RT_DEBUG_ERROR,"WAPP_CAC_PERIOD_EVENT\n");
			wdev_handle_cac_period(wapp, event->ifindex, &event->data);
			break;
#ifdef WIFI_MD_COEX_SUPPORT
		case WAPP_UNSAFE_CHANNEL_EVENT:
			DBGPRINT(RT_DEBUG_ERROR,"WAPP_UNSAFE_CHANNEL_EVENT\n");
			wdev_handle_unsafe_ch_event(wapp, &event->data);
			break;
		case WAPP_BAND_STATUS_CHANGE_EVENT:
			DBGPRINT(RT_DEBUG_ERROR,"WAPP_BAND_STATUS_CHANGE_EVENT\n");
			wdev_handle_band_status_change(wapp, event->ifindex, &event->data);
			break;
#endif
		default:
			DBGPRINT(RT_DEBUG_TRACE, "unknown WAPP_EVENT_ID %d\n", event->event_id);
			break;
	}
}

int wapp_cmd_disconnect_sta( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char cmd[256];
	u8 mac_addr[MAC_ADDR_LEN];
	char *token;
	int i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	i = 0;
	token = strtok(argv[1], ":");
	while (token != NULL) {
		AtoH(token, (char *) &mac_addr[i], 1);
		i++;
		if (i >= MAC_ADDR_LEN)
			break;
		token = strtok(NULL, ":");
	}

    	DBGPRINT_RAW(RT_DEBUG_OFF,
		"disconnect sta:\n"
		"\t sta = %02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(mac_addr));

	// TODO: 1. search the sta from all ap_dev
	// TODO: 2. if the sta support btm, use btm (DISASSOC_STA case). else just send disconnect cmd
	sprintf(cmd, "iwpriv %s set DisConnectSta=%02x:%02x:%02x:%02x:%02x:%02x;", iface, PRINT_MAC(mac_addr));
printf("\033[1;33m %s, %u cmd = %s\033[0m\n", __FUNCTION__, __LINE__, cmd);  /* Haipin Debug Print (Y)*/
	system(cmd);

	return WAPP_SUCCESS;
}

struct wapp_ctrl_cmd wapp_cmd[] = {
	{"wdev_query",        wapp_cmd_wdev_query,                     "[arg1 interface]: query all wdev of the interface"},
	{"wdev_list",         wapp_cmd_wdev_list,                      "show wdev list"},
	{"query",             wapp_cmd_wdev_query_by_req_id,           "[arg1]:interface [arg2]:req_id. Use req_id to send query to the specified interface"},
	{"ht_cap_query",      wapp_cmd_wdev_ht_cap_query,              "[arg1]:interface. query ht_cap of the interface"},
	{"vht_cap_query",     wapp_cmd_wdev_vht_cap_query,             "[arg1]:interface. query vht_cap of the interface"},
	{"cli",               wapp_cmd_query_cli,                      "[arg1]:interface. [arg2]: mac addr of the cli to be queried"},
	{"cli_list",          wapp_cmd_show_cli_list,                  "[arg1]:interface. show the cli_list of an ap_dev"},
	{"ap_info",           wapp_cmd_ap_info,                        "[arg1]:interface. show ap info of an ap_dev"},
	{"chn_list",          wapp_cmd_chn_list_info,                  "[arg1]:interface. show chn list info of an ap_dev"},
	{"bss_info",          wapp_cmd_bss_info,                       "[arg1]:interface. show bss info of an ap_dev"},
	{"ap_metric",         wapp_cmd_ap_metric,                      "[arg1]:interface. show ap metric of an ap_dev"},
	{"set_sec",           wapp_cmd_set_sec,                        "[arg1]:interface. [argv2] auth. [arg3] encryp. [arg4] passphrase. set security and passphrases of a wdev"},
	{"set_ch",            wapp_cmd_set_ch,                         "[arg1]:interface. [argv2] channel. set op channel of the interface"},
	{"disconnect_sta",    wapp_cmd_disconnect_sta,                 "[arg1]:interface. [argv2] mac addr of the sta"},
	{"set_ssid",          wapp_cmd_set_ssid,                       "[arg1]:interface. [argv2] ssid. set ssid of the interface"},
	{"ra_info",           wapp_cmd_show_radio_info,                "show radio info"},
	{"probe_info",        wapp_cmd_show_probe_info,                "show probe info list"},
	{"block_list",        wapp_cmd_show_blocked_list,              "[arg1]:interface. show blocked list"},
	{"bcn_start",		  wapp_cmd_set_bss_start,                  "[arg1]:interface. start bss of this interface"},
	{"bcn_stop",		  wapp_cmd_set_bss_stop,                   "[arg1]:interface. stop bss of this interface"},
	{"txpwr_prctg",       wapp_cmd_set_txpwr_prctg,                "[arg1]:interface. [arg2] tx power percentage"},
	{"scan_request",	wapp_cmd_get_scan_result,	  	"send scan request"},
	{"set_load_thrd",	  wapp_cmd_set_load_thrd,					  "[arg1]:interface. [arg2] High bss load threshold. [arg3] Low bss load threshold."},
	{"log_level",	  	wapp_cmd_set_log_level,					  "set log level"},

#if 0 /* TODO */
	{"set_op_class",	  wapp_cmd_wdev_ht_cap_query,			   "[arg1]:interface. query ht_cap of the interface"},
	{"set_ssid",	  wapp_cmd_wdev_ht_cap_query,			   "[arg1]:interface. query ht_cap of the interface"},
#ifdef MAP_SUPPORT
	{"set_devrole",	  wapp_cmd_wdev_ht_cap_query,			   "[arg1]:interface. query ht_cap of the interface"},
	{"set_bh_type",   wapp_cmd_wdev_ht_cap_query,			   "[arg1]:interface. query ht_cap of the interface"},
	{"set_bh_if",   wapp_cmd_wdev_ht_cap_query,			   "[arg1]:interface. query ht_cap of the interface"},
#endif
#endif /* TODO*/
	{"help",              wapp_cmd_show_help,                      "show this help"},
	{NULL,                wapp_cmd_show_help}
};


int wapp_ctrl_interface_cmd_handle(
	struct wifi_app *wapp,
	const char *iface,
	u8 argc,
	char **argv)
{
	int i, ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[0]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	for (i = 0; wapp_cmd[i].cmd != NULL; i++)
	{
		if (os_strncmp(wapp_cmd[i].cmd, argv[0], os_strlen(argv[0])) == 0) {
			ret = wapp_cmd[i].cmd_proc(wapp, iface, argc, argv);
			if (ret != WAPP_SUCCESS)
				DBGPRINT(RT_DEBUG_ERROR, "cmd [%s] failed. ret = %d\n",	wapp_cmd[i].cmd, ret);
			break;
		}
	}

	return WAPP_SUCCESS;
}

int wapp_cmd_show_help( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	printf("\033[1;36m available cmds: \033[0m\n");
	for(i=0;(wapp_cmd[i].cmd != NULL);i++){
		printf("\033[1;36m %20s  \t -  %s\033[0m\n",wapp_cmd[i].cmd,wapp_cmd[i].help);
	}

	return WAPP_SUCCESS;
}

int wapp_cmd_wdev_query( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	//DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	printf("\033[1;36m inf=%s \033[0m\n", argv[1]);
	wapp_query_wdev(wapp, argv[1]);
	return WAPP_SUCCESS;
}

int wapp_cmd_wdev_ht_cap_query( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	//DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);
	printf("\033[1;36m %s \033[0m\n", __FUNCTION__);
	wapp_query_wdev_ht_cap(wapp, argv[1]);
	return WAPP_SUCCESS;
}

int wapp_cmd_wdev_vht_cap_query( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	//DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);
	printf("\033[1;36m %s \033[0m\n", __FUNCTION__);
	wapp_query_wdev_vht_cap(wapp, argv[1]);
	return WAPP_SUCCESS;
}

int wapp_cmd_wdev_query_by_req_id( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 reg_id = atoi(argv[2]);
	//DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);
	printf("\033[1;36m %s req_id = %u\033[0m\n", __FUNCTION__, reg_id);
	wapp_query_wdev_by_req_id(wapp, argv[1], reg_id);
	return WAPP_SUCCESS;
}


int wapp_cmd_query_cli( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 i;
	u8 mac_addr[MAC_ADDR_LEN];
	char str[3];
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if(argc != 3)
		return WAPP_INVALID_ARG;

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
		for (i = 0; i < MAC_ADDR_LEN; i++) {
			str[0] = argv[2][i*2];
			str[1] = argv[2][i*2+1];
			str[2] = 0;
	 		mac_addr[i] = strtol(str, NULL, 16);
		}
		//DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);
		printf("\033[1;36m %s mac_addr = %s\033[0m\n", __FUNCTION__, argv[2]);
		wapp_query_cli(wapp, argv[1], mac_addr);
	}
	return WAPP_SUCCESS;
}

int wapp_cmd_set_bss_start( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
		printf(BLUE("%s inf = %s\n"), __FUNCTION__, argv[1]);
		wapp_set_bss_start(wapp, argv[1]);
	}
	return WAPP_SUCCESS;
}

int wapp_cmd_get_scan_result( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
		printf(BLUE("%s inf = %s\n"), __FUNCTION__, argv[1]);
		wapp_query_scan_result(wapp, wdev, 0);
	} else
		printf(RED("%s inf not found\n"), __FUNCTION__);

	return WAPP_SUCCESS;
}

int wapp_cmd_set_bss_stop( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
		printf(BLUE("%s inf = %s\n"), __FUNCTION__, argv[1]);
		wapp_set_bss_stop(wapp, argv[1]);
	}
	return WAPP_SUCCESS;
}



int wapp_cmd_set_txpwr_prctg( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;
	u8 pwr_prctg = atoi(argv[2]);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
		printf(BLUE("%s inf = %s, pwr_prctg = %u%%\n"), __FUNCTION__, argv[1], pwr_prctg);
		wapp_set_tx_power_percentage(wapp, wdev, pwr_prctg);
	}
	return WAPP_SUCCESS;
}


int wapp_cmd_show_blocked_list( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;
	//DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP)
		wdev_ap_show_block_list(wapp, (struct ap_dev *) wdev->p_dev);

	return WAPP_SUCCESS;
}

int wapp_cmd_show_cli_list( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;
	//DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
			DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
			return WAPP_INVALID_ARG;
		}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP)
		wdev_ap_show_cli_list(wapp, (struct ap_dev *) wdev->p_dev);

	return WAPP_SUCCESS;
}

int wapp_cmd_chn_list_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
    struct wapp_dev *wdev = NULL;
    //DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

    wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
    if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP)
    {
        wdev_ap_show_chn_list(wapp, (struct ap_dev *) wdev->p_dev);
    }

    return WAPP_SUCCESS;
}

int wapp_cmd_bss_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
    struct wapp_dev *wdev = NULL;
    //DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
			DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
			return WAPP_INVALID_ARG;
		}

    wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
    if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP)
    {
        wdev_ap_show_bss_info(wapp, (struct ap_dev *) wdev->p_dev);
    }

    return WAPP_SUCCESS;
}

int wapp_cmd_ap_metric( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
    struct wapp_dev *wdev = NULL;
    //DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: %s\033[0m\n", argv[1]);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

    wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
    if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP)
    {
		DBGPRINT_RAW(RT_DEBUG_OFF, "bss_info:\n");
        wdev_ap_show_ap_metric(wapp, (struct ap_dev *) wdev->p_dev);
    }

    return WAPP_SUCCESS;
}

int wapp_cmd_set_sec( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;
	struct sec_info sec;

	if(argc != 5 || !argv[1] || !argv[2] || !argv[3] || !argv[4]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	DBGPRINT(RT_DEBUG_OFF, "\033[1;36m:"
		"inf = %s \n"
		"auth = %s \n"
		"encryp = %s \n"
		"psphr = %s \033[0m\n",
		argv[1], argv[2], argv[3], argv[4]);

	os_strncpy(sec.auth, argv[2], 32);
	os_strncpy(sec.encryp, argv[3], 32);
	if (argv[4])
		os_strncpy(sec.psphr, argv[4], 256);

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev)
		wdev_set_sec_and_ssid(wapp, wdev, &sec, NULL);

	return WAPP_SUCCESS;
}

int wapp_cmd_set_ch( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;

	if(argc != 3 || !argv[1] || !argv[2]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	DBGPRINT(RT_DEBUG_OFF, "\033[1;36m:"
		"inf = %s \n"
		"ch = %s \033[0m\n",
		argv[1], argv[2]);

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev)
		wdev_set_ch(wapp, wdev, atoi(argv[2]), 0);

	return WAPP_SUCCESS;
}

int wapp_cmd_set_ssid( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;

	if(argc != 3 || !argv[1] || !argv[2]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	DBGPRINT(RT_DEBUG_OFF, "\033[1;36m:"
		"inf = %s \n"
		"ssid = %s \033[0m\n",
		argv[1], argv[2]);

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev)
		wdev_set_ssid(wapp, wdev, argv[2]);

	return WAPP_SUCCESS;
}


int wapp_cmd_show_radio_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	int i;
	struct wapp_radio *ra = NULL;
#ifdef MAP_SUPPORT
	u8 idfr[MAC_ADDR_LEN];
	char *RADIO_BAND[] = { "24G", "5GL", "5GH", "5G"};
#endif

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra = &wapp->radio[i];
		if (ra->adpt_id) {
			DBGPRINT(RT_DEBUG_OFF,
				"\n===============\n"
				"ra index = %u\n"
				"adpt_id  = %u\n"
				"card_id  = %u\n"
				"ra_id  =   %u\n"
				"op_ch  =   %u\n",
				ra->index,
				ra->adpt_id,
				ra->card_id,
				ra->radio_id,
				ra->op_ch);
#ifdef MAP_SUPPORT
			MAP_GET_RADIO_IDNFER(ra, idfr);
			DBGPRINT(RT_DEBUG_OFF,
				"idfer  =   %02x%02x%02x%02x%02x%02x\n"
				"conf_stat = %u\n"
				"metric policy sta_rssi_thres = %d\n"
				"metric policy sta_hysteresis_margin = %d\n"
				"metric policy ch_util_thres = %d\n"
				"radio_band = %s (%p)\n",
				PRINT_RA_IDENTIFIER(idfr),
				ra->conf_state.state,
				ra->metric_policy.sta_rssi_thres,
				ra->metric_policy.sta_hysteresis_margin,
				ra->metric_policy.ch_util_thres,
				ra->radio_band ? RADIO_BAND[*ra->radio_band] : "NULL", ra->radio_band
				);
#endif /* MAP_SUPPORT */
			DBGPRINT(RT_DEBUG_OFF,
				"\n===============\n");
		}
	}

	return WAPP_SUCCESS;
}

int wapp_cmd_wdev_list( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	//DBGPRINT(RT_DEBUG_TRACE, "\033[1;36m: \033[0m\n");
	printf("\033[1;36m  \033[0m\n");
	wapp_show_dev_list(wapp);
	return WAPP_SUCCESS;
}

int wapp_cmd_ap_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, argv[1]);
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP)
	{
		wdev_ap_show_ap_info(wapp, wdev, (struct ap_dev *) wdev->p_dev);
	}
	return WAPP_SUCCESS;
}

int wapp_cmd_set_load_thrd( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	map_config_bssload_thrd_setting_msg(wapp, argv[1], argv[2], argv[3]); /*For now, this func is called by wappctrl*/
	return WAPP_SUCCESS;
}


int wapp_cmd_set_log_level( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	int log_level = 0;

	log_level = atoi(argv[1]);

	printf("%s set log level %d\n",__func__, log_level);
	if (log_level >= RT_DEBUG_OFF && log_level <= RT_DEBUG_INFO)
		RTDebugLevel = log_level;
	else
		printf("invalid log level %d\n", log_level);

	return WAPP_SUCCESS;
}


char * wapp_config_get_line(char *s, int size, FILE *stream, int *line,
									 char **_pos)
{
	char *pos, *end, *sstart;

    while (fgets(s, size, stream)) {
        (*line)++;
        s[size - 1] = '\0';
        pos = s;

        /* Skip white space from the beginning of line. */
        while (*pos == ' ' || *pos == '\t' || *pos == '\r')
            pos++;

        /* Skip comment lines and empty lines */
        if (*pos == '#' || *pos == '\n' || *pos == '\0')
            continue;

        /*
         * Remove # comments unless they are within a double quoted
         * string.
		 */
        sstart = os_strchr(pos, '"');
        if (sstart)
            sstart = os_strrchr(sstart + 1, '"');
        if (!sstart)
            sstart = pos;
        end = os_strchr(sstart, '#');
        if (end)
            *end-- = '\0';
        else
            end = pos + os_strlen(pos) - 1;

        /* Remove trailing white space. */
        while (end > pos &&
               (*end == '\n' || *end == ' ' || *end == '\t' ||
            *end == '\r'))
            *end-- = '\0';

        if (*pos == '\0')
            continue;

		if (_pos)
            *_pos = pos;
        return pos;
    }

    if (_pos)
        *_pos = NULL;
    return NULL;
}


/* wapp parameter setting */
inline static int wapp_cmm_param_setting(struct wifi_app *wapp,
										struct wapp_conf *conf,
										u32 param, u32 value)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ret = wapp->drv_ops->drv_wapp_param_setting(wapp->drv_data, conf->iface,
								    param, value);
	return ret;
}

//inline static int hotspot_param_setting(struct wifi_app *wapp,
//										struct wapp_conf *conf,
//										u32 param, u32 value)

static int wapp_init_param_setting(struct wifi_app *wapp, struct wapp_conf *conf)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	/* MMPDU Size */
	ret = wapp_cmm_param_setting(wapp, conf, PARAM_MMPDU_SIZE, conf->mmpdu_size);

	/* GAS come back delay */
	ret += wapp_cmm_param_setting(wapp, conf, PARAM_GAS_COME_BACK_DELAY, conf->gas_cb_delay);

	/* set WNM BSS transition management */
	ret += wapp_cmm_param_setting(wapp, conf, PARAM_WNM_BSS_TRANSITION_MANAGEMENT, 1);

	/* set WNM Notification */
	ret += wapp_cmm_param_setting(wapp, conf, PARAM_WNM_NOTIFICATION, 1);

	/* Qos map */
	ret += wapp_cmm_param_setting(wapp, conf, PARAM_QOSMAP, conf->qosmap_enable);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "WAPP_cmm_param_setting FAILED, please check if driver compiled WAPP\n");
		ret = 0;
	}

#ifdef MBO_SUPPORT
	/* mbo assoc disallow */
	ret = mbo_param_setting(wapp, conf->iface, PARAM_MBO_AP_ASSOC_DISALLOW, wapp->mbo->assoc_disallow_reason);

	/* mbo_ap_capability */
	ret += mbo_param_setting(wapp, conf->iface, PARAM_MBO_AP_CAP, wapp->mbo->ap_capability);

	/* mbo_ap_cdcp */
	ret += mbo_param_setting(wapp, conf->iface, PARAM_MBO_AP_CDCP, wapp->mbo->cdcp);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "mbo_param_setting FAILED, please check if driver compiled MBO\n");
		ret = 0;
	}
#endif /* MBO_SUPPORT */

#ifdef PASSPOINT_SUPPORT
	ret = hotspot_init_param(wapp,conf);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "hotspot_param_setting FAILED, please check if driver compiled hotspot\n");
		ret = 0;
	}
#endif /* PASSPOINT SUPPORT */

	return 0;
}


int wapp_init_ap_config(struct wifi_app *wapp, const char *confname)
{
	int ret = 0;
	FILE *file;
	char buf[256], *pos, *token, *token1;
	char tmpbuf[256], tmp1buf[256], tmp2buf[256];
	int line = 0, i = 0;
	struct wapp_conf *conf;
	int varlen;
	struct anqp_capability *capability_info;
	struct nai_realm_data *realm_data = NULL, *realm_data_new = NULL;
	struct eap_method *eapmethod = NULL;
	struct auth_param *authparam, *authparam_new;
	struct anqp_hs_capability *hs_capability_subtype;
	struct plmn *plmn_unit = NULL;
	struct proto_port_tuple *proto_port_unit = NULL;
	struct advice_of_charge_data *charge_data = NULL;
	struct aoc_plan_tuple_data *plan_tuple_data = NULL;
	u8 IsNAIRealmData = 0, IsPLMN = 0, IsProtoPort = 0, IsWanMetrics = 0, IsAOCData = 0;

	struct osu_providers *providers_list = NULL;
	u8 IsProviderList = 0;
	u8 qos_cnt = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s(%s)\n", __FUNCTION__, confname);

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);
	os_memset(tmp1buf, 0, 256);
	os_memset(tmp2buf, 0, 256);

	conf = os_zalloc(sizeof(struct wapp_conf));

	if (!conf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available\n"));
		return -1;
	}

	os_strcpy(conf->confname, confname);

	dl_list_init(&conf->anqp_capability_list);
	dl_list_init(&conf->venue_name_list);
	dl_list_init(&conf->emergency_call_number_list);
	dl_list_init(&conf->network_auth_type_list);
	dl_list_init(&conf->oi_duple_list);
	dl_list_init(&conf->nai_realm_list);
	dl_list_init(&conf->plmn_list);
	dl_list_init(&conf->domain_name_list);

	/* Following are HS2.0 elemets list */
	dl_list_init(&conf->hs_capability_list);
	dl_list_init(&conf->operator_friendly_duple_list);
	dl_list_init(&conf->connection_capability_list);
	dl_list_init(&conf->nai_home_realm_name_query_list);
	dl_list_init(&conf->operating_class_list);
	dl_list_init(&conf->bss_transition_candi_list);
	dl_list_init(&conf->osu_providers_list);
	dl_list_init(&conf->icon_file_list);
	dl_list_init(&conf->advice_of_charge_list);
	dl_list_init(&conf->venue_url_list);
	dl_list_init(&conf->osu_providers_nai_duple_list);
	dl_list_init(&conf->icon_metadata_list);

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, ("open configuration fail\n"));
		goto error;
	}

	while (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		os_strcpy(tmpbuf, pos);
		varlen = 0;
		token = strtok(pos, "=");
		if (token != NULL) {
			if (os_strcmp(token, "interface") == 0) {
				token = strtok(NULL, "");
				os_strcpy(conf->iface, token);
				DBGPRINT(RT_DEBUG_TRACE, "Interface = %s\n", conf->iface);
			} else if (os_strcmp(token, "interworking") == 0) {
				token = strtok(NULL, "");
				conf->interworking = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "interworking = %d\n", conf->interworking);
			}else if (os_strcmp(token, "access_network_type") == 0) {
				token = strtok(NULL, "");
				conf->access_network_type = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "access_network_type = %d\n", conf->access_network_type);
			} else if (os_strcmp(token, "internet") == 0) {
				token = strtok(NULL, "");
				conf->internet = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "internet = %d\n", conf->internet);
			} else if (os_strcmp(token, "venue_group") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->venue_group = atoi(token);
					conf->is_venue_group = 1;
					DBGPRINT(RT_DEBUG_TRACE, "venue_group = %d\n", conf->venue_group);
				}
			} else if (os_strcmp(token, "venue_type") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->venue_type = atoi(token);
					conf->is_venue_type = 1;
					DBGPRINT(RT_DEBUG_TRACE, "venue_type = %d\n", conf->venue_type);
				}
			} else if (os_strcmp(token, "anqp_query") == 0) {
				token = strtok(NULL, "");
				conf->anqp_query = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "anqp_query = %d\n", conf->anqp_query);
			} else if (os_strcmp(token, "anqp_domain_id") == 0) {
				token = strtok(NULL, "");
				conf->anqp_domain_id = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "anqp_domain_id = %d\n", conf->anqp_domain_id);
			} else if (os_strcmp(token, "mih_support") == 0) {
				token = strtok(NULL, "");
				conf->mih_support = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "mih_support = %d\n", conf->mih_support);
			} else if (os_strcmp(token, "hessid") == 0) {
				token = strtok(NULL, ",");
				if (os_strcmp(token, "bssid") == 0) {
					hotspot_get_bssid(wapp, conf);
					conf->is_hessid = 1;
				} else if (os_strcmp(token, "n/a") != 0) {
					os_strcpy(tmp1buf, token);
					token1 = strtok(tmp1buf, ":");
					i = 0;
					while (token1 != NULL) {
						//i = 0;
						AtoH(token1, &conf->hessid[i], 1);
						DBGPRINT(RT_DEBUG_TRACE, "hessid[%d] = 0x%02x\n", i, conf->hessid[i]);
						i++;
						token1 = strtok(NULL, ":");
					}
					if (i == 6)
						conf->is_hessid = 1;
				}
			} else if (os_strcmp(token, "roaming_consortium_oi") == 0) {
				struct oi_duple *oiduple;
				token = strtok(NULL, ",");
				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						i = 0;
						varlen = 0;
						os_strcpy(tmp1buf, token);
						os_strcpy(tmp2buf, token);
						token1 = strtok(tmp1buf, "-");
						while (token1 != NULL) {
							varlen += 1;
							token1 = strtok(NULL, "-");
						}
						oiduple = os_zalloc(sizeof(struct oi_duple) + varlen);
						oiduple->length = varlen;

						token1 = strtok(tmp2buf, "-");
						while(token1 != NULL) {
							AtoH(token1, &oiduple->oi[i], 1);
							DBGPRINT(RT_DEBUG_TRACE, "roaming consortium_oi[%d] = 0x%02x\n", i, oiduple->oi[i]);
							token1 = strtok(NULL, "-");
							i++;
						}

						dl_list_add_tail(&conf->oi_duple_list, &oiduple->list);

						token = strtok(token + (varlen * 3), ",");
					}

					if (!dl_list_empty(&conf->oi_duple_list))
						conf->have_roaming_consortium_list = 1;
				}

			} else if (os_strcmp(token, "advertisement_proto_id") == 0) {
				token = strtok(NULL, ":");
				while (token != NULL) {
					conf->advertisement_proto_num++;
					token = strtok(NULL, ":");
				}
				if (conf->advertisement_proto_num > 0)
				{
					conf->advertisement_proto = os_zalloc(conf->advertisement_proto_num);
					if (!conf->advertisement_proto) {
						DBGPRINT(RT_DEBUG_ERROR, "Not available memory\n");
						goto error1;
					}
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					i = 0;

					while (token != NULL && i < conf->advertisement_proto_num) {
						conf->advertisement_proto[i] = atoi(token);
						DBGPRINT(RT_DEBUG_TRACE, "advertisement proto[%d] = %x\n", i, atoi(token));
						i++;
						token = strtok(NULL, ":");
					}
				}
			} else if (os_strcmp(token, "domain_name") == 0) {
				struct domain_name_field *dname_field;
				token = strtok(NULL, ";");

				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						dname_field = os_zalloc(sizeof(struct domain_name_field) + os_strlen(token));
						dname_field->length = os_strlen(token);
						DBGPRINT(RT_DEBUG_TRACE, "length of domain name = %d\n", dname_field->length);
						DBGPRINT(RT_DEBUG_TRACE, "domain name:%s\n", token);
						os_strcpy(dname_field->domain_name, token);
						dl_list_add_tail(&conf->domain_name_list, &dname_field->list);
						token = strtok(NULL, ";");
					}

					if (!dl_list_empty(&conf->domain_name_list))
						conf->have_domain_name_list = 1;
				}

			} else if (os_strncmp(token, "venue_name", 10) == 0) {
				struct venue_name_duple *vname_duple;
				token = strtok(NULL, "%");
				if (os_strcmp(token, "n/a") != 0) {
					int max_venue_len = 255, copy_len = 0;
					char *venue_ptr;
					token = strtok(NULL, "%");
					vname_duple = os_zalloc(sizeof(struct venue_name_duple) + 256);

					token = strtok(tmpbuf, "=");
					token = strtok(NULL, "%");
					vname_duple->length += 3;
					os_strncpy(vname_duple->language, token, 3);
					DBGPRINT(RT_DEBUG_TRACE, "Language of venue name = %s\n", token);
					token = strtok(NULL, "%");
					if (token[0] == '{')
					{
						token++;
						venue_ptr = vname_duple->venue_name;

						while(1)
						{
							if (token[os_strlen(token)-1] == '}')
							{
								if ((vname_duple->length)+(os_strlen(token)-1) > max_venue_len)
									copy_len = max_venue_len-vname_duple->length;
								else
									copy_len = os_strlen(token)-1;

								vname_duple->length += copy_len;
								os_strncpy(venue_ptr, token, copy_len);
								venue_ptr += copy_len;
								break;
							}
							else
							{
								if ((vname_duple->length)+(os_strlen(token)+1) > max_venue_len)
								{
									copy_len = max_venue_len-vname_duple->length;
									vname_duple->length += copy_len;
									os_strncpy(venue_ptr, token, copy_len);
									venue_ptr += copy_len;
									break;
								}
								else
								{
									copy_len = os_strlen(token)+1;
									vname_duple->length += copy_len;
									os_strncpy(venue_ptr, token, copy_len-1);
									venue_ptr += copy_len-1;
									*venue_ptr = 0x0a;
									venue_ptr += 1;
								}
							}
							if (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
								os_strcpy(tmpbuf, pos);
								varlen = 0;
								token = strtok(pos, "=");
							}
						}
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE, "venue name format error!! no { start\n");
					}

					DBGPRINT(RT_DEBUG_TRACE, "venue name:%s\n", token);
					dl_list_add_tail(&conf->venue_name_list, &vname_duple->list);
					conf->venue_name_nums++;

					if (!dl_list_empty(&conf->venue_name_list))
						conf->have_venue_name = 1;
				}
			} else if (os_strncmp(token, "network_auth_type", 17) == 0) {
				struct net_auth_type_unit *auth_type_unit;
				token = strtok(NULL, ",");
				if (os_strcmp(token, "n/a") != 0) {
					token = strtok(NULL, ",");

					if (token)
						auth_type_unit = os_zalloc(sizeof(struct net_auth_type_unit) + os_strlen(token));
					else
						auth_type_unit = os_zalloc(sizeof(struct net_auth_type_unit));

					token = strtok(tmpbuf, "=");
					DBGPRINT(RT_DEBUG_TRACE, "%s\n", token);
					token = strtok(NULL, ",");
					auth_type_unit->net_auth_type_indicator = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "Network auth type indicator = %d\n", atoi(token));
					token = strtok(NULL, ",");

					if (token)
						auth_type_unit->re_direct_URL_len = os_strlen(token);
					else
						auth_type_unit->re_direct_URL_len = 0;

					if (token) {
						os_strcpy(auth_type_unit->re_direct_URL, token);
						DBGPRINT(RT_DEBUG_TRACE, "re direct URL = %s\n", token);
					}

					dl_list_add_tail(&conf->network_auth_type_list, &auth_type_unit->list);
					conf->network_auth_type_nums++;

					if (!dl_list_empty(&conf->network_auth_type_list))
						conf->have_network_auth_type = 1;
				}
			} else if (os_strcmp(token, "ipv4_type") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->ipv4_address_type = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "ipv4_type = %d\n", conf->ipv4_address_type);
					conf->have_ip_address_type = 1;
				}
			} else if (os_strcmp(token, "ipv6_type") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->ipv6_address_type = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "ipv6_type = %d\n", conf->ipv6_address_type);
					conf->have_ip_address_type = 1;
				}
			} else if (os_strncmp(token, "osu_providers_list", 18) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					if (os_strcmp(token, "{") == 0) {
						IsProviderList = 1;
						providers_list = os_zalloc(sizeof(*providers_list));
						/* 5:osu_server_uri_len + osu_method_list_len + osu_nai_len + icon_avail_len */
						/* 4:osu_friendly_name_len + osu_service_len */
						providers_list->osu_providers_list_field_len = 5 + 4;
						dl_list_init(&providers_list->osu_friendly_name_list);
						dl_list_init(&providers_list->osu_method_list);
						dl_list_init(&providers_list->icon_list);
						dl_list_init(&providers_list->osu_nai_list);
						dl_list_init(&providers_list->osu_service_desc_list);
						dl_list_add_tail(&conf->osu_providers_list, &providers_list->list);
					}
				} else
					conf->osu_providers_list_nums = 0;
			} else if (os_strcmp(token, "osu_friendly_name") == 0) {
				struct osu_friendly_name *friendly_name;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					friendly_name = os_zalloc(sizeof(struct osu_friendly_name) + os_strlen(token));
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					friendly_name->len += 3;
					os_strncpy(friendly_name->language, token, 3);
					DBGPRINT(RT_DEBUG_TRACE, "Language of osu_friendly_name = %s\n", token);
					token = strtok(NULL, ":");
					friendly_name->len += os_strlen(token);
					os_strncpy(friendly_name->osu_friendly_name_value, token, os_strlen(token));
					DBGPRINT(RT_DEBUG_TRACE, "osu_friendly_name:%s\n", token);
					providers_list->osu_providers_list_field_len += 4+os_strlen(token);
					dl_list_add_tail(&providers_list->osu_friendly_name_list, &friendly_name->list);
					providers_list->osu_friendly_name_len += 4+os_strlen(token);
				}
			} else if (os_strcmp(token, "osu_server_uri") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					providers_list->osu_server_uri = os_zalloc(os_strlen(token));
					DBGPRINT(RT_DEBUG_TRACE, "osu_server_uri:%s\n", token);
					os_memcpy(providers_list->osu_server_uri, token, os_strlen(token));
					providers_list->osu_server_uri_len = os_strlen(token);
					providers_list->osu_providers_list_field_len += os_strlen(token);
				}
			} else if (os_strcmp(token, "osu_method") == 0) {
				struct osu_method *method;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					method = os_zalloc(sizeof(struct osu_method));
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, " ");
					method->osu_method_value = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "osu_method = %d\n", atoi(token));
					dl_list_add_tail(&providers_list->osu_method_list, &method->list);
					providers_list->osu_method_len++;
					providers_list->osu_providers_list_field_len += 1;
				}
			} else if (os_strcmp(token, "icon") == 0) {
				struct icon_available *icon;
				char *type, *name;
				char *lang;
				int weight, height;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					weight = atoi(token);
					token = strtok(NULL, ":");
					height = atoi(token);
					lang = strtok(NULL, ":");
					type = strtok(NULL, ":");
					name = strtok(NULL, ":");
					DBGPRINT(RT_DEBUG_TRACE, "typelen=%zu, filelen=%zu\n", os_strlen(type), os_strlen(name));
					icon = os_zalloc(sizeof(*icon)+os_strlen(type)+os_strlen(name));

					icon->weight = weight;
					icon->height = height;
					os_memcpy(icon->language, lang, 3);
					os_memcpy(icon->icon_buf, type, os_strlen(type));
					icon->type_len = os_strlen(type);

					os_memcpy(&icon->icon_buf[icon->type_len], name, os_strlen(name));
					icon->filename_len = os_strlen(name);
					dl_list_add_tail(&providers_list->icon_list, &icon->list);
					providers_list->icon_len += 9+os_strlen(type)+os_strlen(name);
					providers_list->osu_providers_list_field_len += 9+os_strlen(type)+os_strlen(name); //providers_list->icon_len;
				}
			} else if (os_strncmp(token,"advice_of_charge_data", 21) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					if (os_strcmp(token, "{") == 0) {
						IsAOCData = 1;
						charge_data = os_zalloc(sizeof(*charge_data));
						dl_list_init(&charge_data->aoc_plan_tuples_list);
					}
				 } else {
					conf->advice_of_charge_data_nums = 0;
					conf->have_advice_of_charge = 0;
				}
			} else if ((os_strcmp(token, "}") == 0) && IsAOCData) {
				if(charge_data)
					dl_list_add_tail(&conf->advice_of_charge_list, &charge_data->list);
				else
					DBGPRINT(RT_DEBUG_ERROR, "[ERR] AdviceOfChargeData unexpected NULL, error in file parsing \n");

				conf->advice_of_charge_data_nums ++;
				IsAOCData = 0;
				charge_data = NULL;
			} else if (os_strcmp(token, "advice_of_charge_type") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "advice_of_charge_type:%s\n", token);

				switch (atoi(token)) {
				case TIME_BASED:
					charge_data->advice_of_charge_type = TIME_BASED;
					DBGPRINT(RT_DEBUG_TRACE, "Type = TIME_BASED\n");
					break;
				case DATA_VOLUME_BASED:
					charge_data->advice_of_charge_type = DATA_VOLUME_BASED;
					DBGPRINT(RT_DEBUG_TRACE, "Type = DATA_VOLUME_BASED\n");
					break;
				case TIME_AND_DATA_VOLUME_BASED:
					charge_data->advice_of_charge_type = TIME_AND_DATA_VOLUME_BASED;
					DBGPRINT(RT_DEBUG_TRACE, "Type = DATA_VOLUME_BASED\n");
					break;
				case UNLIMITED:
					charge_data->advice_of_charge_type = UNLIMITED;
					DBGPRINT(RT_DEBUG_TRACE, "Type = UNLIMITED\n");
					break;
				default:
					DBGPRINT(RT_DEBUG_ERROR, "UNKNOWN ADVICE of CHARGE TYPE.\n");
					break;
				}
			} else if (os_strcmp(token, "aoc_realm_encoding") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "aoc_realm_encoding:[%s]\n", token);
				if ((charge_data != NULL) && (token != NULL))
					charge_data->aoc_realm_encoding = atoi(token);
				else
					DBGPRINT(RT_DEBUG_ERROR, "[ERR]ADVICE OF CHARGE Element incomplete, charge_data: [%p], aoc_realm_encoding: [%s]\n",
                                                                                        charge_data, token);
			} else if (os_strcmp(token, "aoc_realm") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "aoc_realm:[%s]\n", token);
				if ((charge_data != NULL) && (token != NULL) && os_strcmp(token, "n/a") != 0) {
					charge_data->aoc_realm_len = os_strlen(token);
					charge_data->aoc_realm = os_zalloc(charge_data->aoc_realm_len);
					os_strncpy(charge_data->aoc_realm, token, charge_data->aoc_realm_len);
				} else if ((charge_data != NULL)) {
					charge_data->aoc_realm_len = 0;
					DBGPRINT(RT_DEBUG_TRACE, "aoc_realm is NULL\n");
				} else
					DBGPRINT(RT_DEBUG_ERROR,"ADVICE OF CHARGE Element incomplete, charge_data: [%p], aoc_realm: [%s]\n",
                                                          charge_data, token);
			} else if (os_strcmp(token, "aoc_language") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "aoc_language:[%s]\n", token);
				plan_tuple_data = os_zalloc(sizeof(*plan_tuple_data));
				if ((plan_tuple_data != NULL) && (token != NULL))
					os_strncpy(plan_tuple_data->language, token, 3);
				else
					DBGPRINT(RT_DEBUG_ERROR, "[ERR]ADVICE OF CHARGE Element incomplete, plan_tuple_data: [%p], aoc_language: [%s]\n",
                                                          plan_tuple_data, token);
			} else if (os_strcmp(token, "aoc_currency_code") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "aoc_currency_code:[%s]\n", token);
				if ((plan_tuple_data != NULL) && (token != NULL))
					os_strncpy(plan_tuple_data->currency_code, token, 3);
				else
					DBGPRINT(RT_DEBUG_ERROR, "[ERR]ADVICE OF CHARGE Element incomplete, plan_tuple_data: [%p], aoc_currency_code: [%s]\n",
                                                          plan_tuple_data, token);
			} else if (os_strcmp(token, "aoc_plan_info") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "aoc_plan_info:[%s]\n", token);
				if ((plan_tuple_data != NULL) && (token != NULL)) {
					plan_tuple_data->plan_information_len = os_strlen(token);
					plan_tuple_data->plan_information = os_zalloc(plan_tuple_data->plan_information_len);
					os_strncpy(plan_tuple_data->plan_information, token, plan_tuple_data->plan_information_len);
					conf->have_advice_of_charge = 1;
					dl_list_add_tail(&charge_data->aoc_plan_tuples_list, &plan_tuple_data->list);
					DBGPRINT(RT_DEBUG_TRACE, "parsed advice_of_charge_data and insert into advice_of_charge_list\n");
				} else {
					if (plan_tuple_data != NULL)
						plan_tuple_data->plan_information_len = 0;
					DBGPRINT(RT_DEBUG_ERROR, "[ERR]ADVICE OF CHARGE Element incomplete, plan_tuple_data: [%p], aoc_plan_info: [%s]\n",
                                                          plan_tuple_data, token);
				}
				plan_tuple_data = NULL;
			} else if (os_strcmp(token, "venue_url") == 0) {
				struct venue_url_duple *vurl_duple;
				int ven_num;
				token = strtok(NULL, ",");
				if ((os_strcmp(token, "n/a") != 0) && (token != NULL)) {
					ven_num = atoi(token);
					token = strtok(NULL, ",");
					DBGPRINT(RT_DEBUG_TRACE, "venue_url_len=%zu\n", os_strlen(token));
					vurl_duple = os_zalloc(sizeof(*vurl_duple)+os_strlen(token));

					vurl_duple->venue_number = ven_num;
					vurl_duple->url_length = os_strlen(token);
					os_strncpy(vurl_duple->venue_url, token, vurl_duple->url_length);
					dl_list_add_tail(&conf->venue_url_list, &vurl_duple->list);
					conf->venue_url_nums++;

					if (!dl_list_empty(&conf->venue_url_list))
						conf->have_venue_url = 1;
					else
						conf->have_venue_url = 0;
				}
			} else if (os_strcmp(token, "t_c_filename") == 0) {
				token = strtok(NULL, "");
				conf->have_t_c_filename = 1;
				conf->t_c_filename = os_zalloc(os_strlen(token));
				os_memcpy(conf->t_c_filename, token, os_strlen(token));
				DBGPRINT(RT_DEBUG_TRACE, "t_c_filename=%s\n", conf->t_c_filename);
			} else if (os_strcmp(token, "t_c_server_url") == 0) {
				token = strtok(NULL, "");
				conf->have_t_c_server_url = 1;
				conf->t_c_server_url = os_zalloc(os_strlen(token));
				os_memcpy(conf->t_c_server_url, token, os_strlen(token));
				DBGPRINT(RT_DEBUG_TRACE, "t_c_server_url=%s\n", conf->t_c_server_url);
			} else if (os_strcmp(token, "t_c_timestamp") == 0) {
				token = strtok(NULL, "");
				conf->have_t_c_timestamp = 1;
				conf->t_c_timestamp = os_zalloc(os_strlen(token));
				os_memcpy(conf->t_c_timestamp, token, os_strlen(token));
				DBGPRINT(RT_DEBUG_TRACE, "t_c_timestamp=%s\n", conf->t_c_timestamp);
			} else if (os_strcmp(token, "osu_providers_nai_list") == 0) {
				struct osu_providers_nai_duple *osu_prov_nai_duple;
				int len;
				token = strtok(NULL, ",");
				if ((os_strcmp(token, "n/a") != 0) && (token != NULL)) {
					len = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "Osu providers nai list len = %d\n", len);
					token = strtok(NULL, ",");

					osu_prov_nai_duple = os_zalloc(sizeof(*osu_prov_nai_duple)+len);
					osu_prov_nai_duple->length = len;
					os_strncpy(osu_prov_nai_duple->osu_providers_nai_list, token, len);
					DBGPRINT(RT_DEBUG_TRACE, "Osu providers nai list: %s\n", token);
					dl_list_add_tail(&conf->osu_providers_nai_duple_list, &osu_prov_nai_duple->list);
					conf->osu_providers_nai_nums++;

					if (!dl_list_empty(&conf->osu_providers_nai_duple_list))
						conf->have_osu_providers_nai_list = 1;
					else
						conf->have_osu_providers_nai_list = 0;
				}
			} else if (os_strcmp(token,"icon_metadata") == 0) {
				struct operator_icon_metadata *oim_data;
				char *type, *name;
				char *lang;
				int weight, height;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					weight = atoi(token);
					token = strtok(NULL, ":");
					height = atoi(token);
					lang = strtok(NULL, ":");
					type = strtok(NULL, ":");
					name = strtok(NULL, ":");
					DBGPRINT(RT_DEBUG_OFF, "typelen=%zu, filelen=%zu\n", os_strlen(type), os_strlen(name));
					oim_data = os_zalloc(sizeof(*oim_data)+os_strlen(type)+os_strlen(name));

					oim_data->weight = weight;
					oim_data->height = height;
					os_memcpy(oim_data->language, lang, 3);
					os_memcpy(oim_data->icon_buf, type, os_strlen(type));
					oim_data->type_len = os_strlen(type);

					os_memcpy(&oim_data->icon_buf[oim_data->type_len], name, os_strlen(name));
					oim_data->filename_len = os_strlen(name);
					dl_list_add_tail(&conf->icon_metadata_list, &oim_data->list);

					if (!dl_list_empty(&conf->icon_metadata_list))
						conf->have_icon_metadata = 1;
					else
						conf->have_icon_metadata = 0;
				}
			} else if (os_strcmp(token, "osu_nai") == 0) {
				struct osu_nai *nai;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					nai = os_zalloc(sizeof(*nai) + os_strlen(token));
					nai->len = os_strlen(token);
					os_memcpy(nai->osu_nai_value, token, os_strlen(token));

					DBGPRINT(RT_DEBUG_TRACE, "osu_nai = %s\n", token);
					providers_list->osu_providers_list_field_len += nai->len;
					dl_list_add_tail(&providers_list->osu_nai_list, &nai->list);
					providers_list->osu_nai_len += nai->len;
				}
			} else if (os_strcmp(token, "osu_service_desc") == 0) {
				struct osu_service_desc *service_desc;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					service_desc = os_zalloc(sizeof(*service_desc) + os_strlen(token));
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					service_desc->len += 3;
					os_strncpy(service_desc->language, token, 3);
					DBGPRINT(RT_DEBUG_TRACE, "Language of osu_service_desc = %s\n", token);
					token = strtok(NULL, ":");
					service_desc->len += os_strlen(token);
					os_strncpy(service_desc->osu_service_desc_value, token, os_strlen(token));
					DBGPRINT(RT_DEBUG_TRACE, "osu_service_desc:%s\n", token);
					providers_list->osu_providers_list_field_len += 4+os_strlen(token);
					dl_list_add_tail(&providers_list->osu_service_desc_list, &service_desc->list);
					providers_list->osu_service_len += 4+os_strlen(token);
				}
			} else if ((os_strcmp(token, "}") == 0) && IsProviderList) {
				conf->osu_providers_list_nums++;
				conf->have_osu_providers_list = 1;
				IsProviderList = 0;
				DBGPRINT(RT_DEBUG_TRACE, "total len=%d\n", providers_list->osu_providers_list_field_len);
				DBGPRINT(RT_DEBUG_TRACE, "list num:%d\n", conf->osu_providers_list_nums);
			} else if (os_strncmp(token, "nai_realm_data", 14) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					if (os_strcmp(token, "{") == 0) {
						IsNAIRealmData = 1;
						realm_data = os_zalloc(sizeof(*realm_data));
						realm_data->nai_realm_data_field_len = 3;
						realm_data->nai_realm_encoding = 0;
						dl_list_init(&realm_data->eap_method_list);
					}
				} else
					conf->nai_realm_data_nums = 0;
			} else if ((os_strcmp(token, "}") == 0) && IsNAIRealmData) {
				dl_list_for_each(eapmethod, &realm_data_new->eap_method_list,
										struct eap_method, list) {
					realm_data_new->nai_realm_data_field_len += 1;
					realm_data_new->nai_realm_data_field_len += eapmethod->len;
				}
				conf->nai_realm_data_nums++;
				IsNAIRealmData = 0;
			} else if (os_strcmp(token, "nai_realm") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "nai_realm:%s\n", token);
				varlen = os_strlen(token);
				realm_data_new = os_realloc(realm_data, sizeof(*realm_data) + varlen);
				dl_list_init(&realm_data_new->eap_method_list);
				os_memcpy(realm_data_new->nai_realm, token, varlen);
				realm_data_new->nai_realm_len = varlen;
				realm_data_new->nai_realm_data_field_len += varlen;
				conf->have_nai_realm_list = 1;
				dl_list_add_tail(&conf->nai_realm_list, &realm_data_new->list);
			} else if (os_strncmp(token, "eap_method", 10) == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "eap_method:%s\n", token);
				if (os_strcmp(token, "eap-ttls") == 0) {
					realm_data_new->eap_method_count++;
					eapmethod = os_zalloc(sizeof(*eapmethod));
					eapmethod->len = 2;
					eapmethod->eap_method = EAP_TTLS;
					dl_list_init(&eapmethod->auth_param_list);
					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);
				} else if (os_strcmp(token, "eap-tls") == 0) {
					realm_data_new->eap_method_count++;
					eapmethod = os_zalloc(sizeof(*eapmethod));
					eapmethod->len = 2;
					eapmethod->eap_method = EAP_TLS;
					dl_list_init(&eapmethod->auth_param_list);
					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);
				} else if (os_strcmp(token, "eap-sim") == 0) {
					realm_data_new->eap_method_count++;
					eapmethod = os_zalloc(sizeof(*eapmethod));
					eapmethod->len = 2;
					eapmethod->eap_method = EAP_SIM;
					dl_list_init(&eapmethod->auth_param_list);
					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);
				} else if (os_strcmp(token, "eap-aka") == 0) {
					realm_data_new->eap_method_count++;
					eapmethod = os_zalloc(sizeof(*eapmethod));
					eapmethod->len = 2;
					eapmethod->eap_method = EAP_AKA;
					dl_list_init(&eapmethod->auth_param_list);
					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);
				}
			} else if (os_strncmp(token, "auth_param", 9) == 0) {
				DBGPRINT(RT_DEBUG_TRACE, "auth_param:\n");
				token = strtok(NULL, ":");
				authparam = os_zalloc(sizeof(*authparam));
				eapmethod->auth_param_count++;
				switch (atoi(token)) {
				case EXPANDED_EAP_METHOD:
					authparam->id = EXPANDED_EAP_METHOD;
					authparam->len = 7;
					varlen = 7;
					eapmethod->len += 9;
					DBGPRINT(RT_DEBUG_TRACE, "ID = EXPANDED_EAP_METHOD\n")
					break;
				case NON_EAP_INNER_AUTH_TYPE:
					authparam->id = NON_EAP_INNER_AUTH_TYPE;
					authparam->len = 1;
					varlen = 1;
					eapmethod->len += 3;
					DBGPRINT(RT_DEBUG_TRACE, "ID = NON_EAP_INNER_AUTH_TYPE\n")
					break;
				case INNER_AUTH_EAP_METHOD_TYPE:
					authparam->id = INNER_AUTH_EAP_METHOD_TYPE;
					authparam->len = 1;
					varlen = 1;
					eapmethod->len += 3;
					DBGPRINT(RT_DEBUG_TRACE, "ID = INNER_AUTH_EAP_METHOD_TYPE\n")
					break;
				case EXPANDED_INNER_EAP_METHOD:
					authparam->id = EXPANDED_INNER_EAP_METHOD;
					authparam->len = 7;
					varlen = 7;
					eapmethod->len += 9;
					DBGPRINT(RT_DEBUG_TRACE, "ID = EXPANDED_INNER_EAP_METHOD\n")
					break;
				case CREDENTIAL_TYPE:
					authparam->id = CREDENTIAL_TYPE;
					authparam->len = 1;
					varlen = 1;
					eapmethod->len += 3;
					DBGPRINT(RT_DEBUG_TRACE, "ID = CREDENTIAL_TYPE\n")
					break;
				case TUNNELED_EAP_METHOD_CREDENTIAL_TYPE:
					authparam->id = TUNNELED_EAP_METHOD_CREDENTIAL_TYPE;
					authparam->len = 1;
					varlen = 1;
					eapmethod->len += 3;
					DBGPRINT(RT_DEBUG_TRACE, "ID = TUNNELED_EAP_METHOD_CREDENTIAL_TYPE\n")
					break;
				case VENDOR_SPECIFIC:
					DBGPRINT(RT_DEBUG_TRACE, "ID = VENDOR_SPECIFIC\n")
					/* TODO: varlen is variable */
				default:
					DBGPRINT(RT_DEBUG_ERROR, "Unknown authentication parameter types\n");
					break;
				}

				authparam_new = os_realloc(authparam, sizeof(*authparam) + varlen);
				token = strtok(NULL, ":");
				DBGPRINT(RT_DEBUG_TRACE, "Value = %d\n", atoi(token));
				while (token) {
					if (varlen == 1) {
						*authparam_new->auth_param_value = atoi(token);
					} else if (varlen == 7) {
						/* TODO */
					}
					token = strtok(NULL, ":");
				}

				dl_list_add_tail(&eapmethod->auth_param_list, &authparam_new->list);

			} else if (os_strncmp(token, "op_friendly_name", 16) == 0) {
				struct operator_name_duple *op_name_duple;
				token = strtok(NULL, ",");
				if (os_strcmp(token, "n/a") != 0) {
					token = strtok(NULL, ",");
					op_name_duple = os_zalloc(sizeof(struct operator_name_duple) + os_strlen(token));

					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ",");
					op_name_duple->length += 3;
					os_strncpy(op_name_duple->language, token, 3);
					DBGPRINT(RT_DEBUG_TRACE, "Language of operator friendly name = %s\n", token);
					token = strtok(NULL, ",");
					op_name_duple->length += os_strlen(token);
					os_strncpy(op_name_duple->operator_name, token, os_strlen(token));
					DBGPRINT(RT_DEBUG_TRACE, "operator friendly name:%s , strlen:%zu\n", token,os_strlen(token));
					dl_list_add_tail(&conf->operator_friendly_duple_list, &op_name_duple->list);
					conf->op_friendly_name_nums++;

					if (!dl_list_empty(&conf->operator_friendly_duple_list))
						conf->have_operator_friendly_name = 1;
				}
			} else if (os_strncmp(token, "plmn", 4) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					if (os_strcmp(token, "{") == 0) {
						IsPLMN = 1;
						plmn_unit = os_zalloc(sizeof(*plmn_unit));
					}
				} else
					conf->plmn_nums = 0;
			} else if ((os_strcmp(token, "}") == 0) && IsPLMN) {
				dl_list_add_tail(&conf->plmn_list, &plmn_unit->list);
				conf->plmn_nums++;
				if (!dl_list_empty(&conf->plmn_list)) {
					conf->have_3gpp_network_info = 1;
				}
				IsPLMN = 0;
			} else if (os_strcmp(token, "mcc") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "mcc = %s\n", token);
				for (i = 0; i < 3; i++) {
					plmn_unit->mcc[i] = token[i] - '0';
					DBGPRINT(RT_DEBUG_TRACE, "mcc[%d] = %d\n", i, plmn_unit->mcc[i]);
				}
			} else if (os_strcmp(token, "mnc") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "mnc = %s\n", token);
				for (i = 0; i < 3; i++) {
					if (i == 2 && os_strlen(token) == 2)
						plmn_unit->mnc[i] = 0x0f;
					else
						plmn_unit->mnc[i] = token[i] - '0';
					DBGPRINT(RT_DEBUG_TRACE, "mnc[%d] = %d\n", i, plmn_unit->mnc[i]);
				}

			} else if (os_strncmp(token, "proto_port", 10) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "{") == 0) {
					IsProtoPort = 1;
					proto_port_unit = os_zalloc(sizeof(*proto_port_unit));
				}
			} else if ((os_strcmp(token, "}") == 0) && IsProtoPort) {
				dl_list_add_tail(&conf->connection_capability_list, &proto_port_unit->list);
				conf->proto_port_nums++;
				if (!dl_list_empty(&conf->connection_capability_list))
					conf->have_connection_capability_list = 1;
				IsProtoPort = 0;
			} else if (os_strcmp(token, "operating_class") == 0) {
				struct operating_class_unit *operating_class;
				token = strtok(NULL, ",");

				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						operating_class = os_zalloc(sizeof(struct operating_class_unit));
						DBGPRINT(RT_DEBUG_TRACE, "operating class:%s\n", token);
						operating_class->op_class = atoi(token);
						dl_list_add_tail(&conf->operating_class_list, &operating_class->list);
						token = strtok(NULL, ",");
					}

					if (!dl_list_empty(&conf->operating_class_list))
						conf->have_operating_class = 1;
				}
			} else if (os_strcmp(token, "ip_protocol") == 0) {
				token = strtok(NULL, "");
				proto_port_unit->ip_protocol = atoi(token);
			} else if (os_strcmp(token, "port") == 0) {
				token = strtok(NULL, "");
				proto_port_unit->port = atoi(token);
			} else if (os_strcmp(token, "status") == 0) {
				token = strtok(NULL, "");
				proto_port_unit->status = atoi(token);
			} else if (os_strcmp(token, "wan_metrics") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "{") == 0)
					IsWanMetrics = 1;
			} else if ((os_strcmp(token, "}") == 0) && IsWanMetrics) {
				conf->wan_metrics_nums++;
				conf->have_wan_metrics = 1;
				IsWanMetrics = 0;
			} else if (os_strcmp(token, "link_status") == 0) {
				token = strtok(NULL, "");
				conf->metrics.link_status = atoi(token);
			} else if (os_strcmp(token, "at_capacity") == 0) {
				token = strtok(NULL, "");
				conf->metrics.at_capacity = atoi(token);
			} else if (os_strcmp(token, "dl_speed") == 0) {
				token = strtok(NULL, "");
				conf->metrics.dl_speed = atoi(token);
			} else if (os_strcmp(token, "ul_speed") == 0) {
				token = strtok(NULL, "");
				conf->metrics.ul_speed = atoi(token);
			} else if (os_strcmp(token, "dl_load") == 0) {
				token = strtok(NULL, "");
				conf->metrics.dl_load = atoi(token);
			} else if (os_strcmp(token, "up_load") == 0) {
				token = strtok(NULL, "");
				conf->metrics.ul_load = atoi(token);
			} else if (os_strcmp(token, "lmd") == 0) {
				token = strtok(NULL, "");
				conf->metrics.lmd = atoi(token);
			} else if (os_strcmp(token, "preferred_candi_list_included") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "preferred candi list included = %s\n", token);
				conf->preferred_candi_list_included = atoi(token);
			} else if (os_strcmp(token, "abridged") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "abridged = %s\n", token);
				conf->abridged = atoi(token);
			} else if (os_strcmp(token, "disassociation_imminent") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "disassociationimminent = %s\n", token);
				conf->disassociation_imminent = atoi(token);
			} else if (os_strcmp(token, "bss_termination_included") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "bss termination included = %s\n", token);
				conf->bss_termination_included = atoi(token);
			} else if (os_strcmp(token, "ess_disassociation_imminent") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "ess disassociation imminent = %s\n", token);
				conf->ess_disassociation_imminent = atoi(token);
			} else if (os_strcmp(token, "disassociation_timer") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "disassociation timer = %s\n", token);
				conf->disassociation_timer = atoi(token);
			} else if (os_strcmp(token, "validity_interval") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "validity interval = %s\n", token);
				conf->validity_interval = atoi(token);
			} else if (os_strcmp(token, "bss_termination_duration") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->have_bss_termination_duration = 1;
					token = strtok(NULL, ",");
					DBGPRINT(RT_DEBUG_TRACE, "bss termination tsf = %s\n", token);
					conf->bss_termination_tsf = atoi(token);
					token = strtok(NULL, "");
					DBGPRINT(RT_DEBUG_TRACE, "bss termination duration = %s\n", token);
					conf->bss_termination_duration = atoi(token);
				}
			} else if (os_strcmp(token, "session_information_url") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "session information url = %s\n", token);
				conf->have_session_info_url = 1;
				conf->session_info_url_len = os_strlen(token);
				conf->session_info_url = os_zalloc(conf->session_info_url_len);
				os_memcpy(conf->session_info_url, token, conf->session_info_url_len);
			} else if (os_strcmp(token, "bss_transisition_candi_list_preferences") == 0) {
				token = strtok(NULL, ",");
				if (os_strcmp(token, "n/a") != 0) {
					token = strtok(NULL, ",");

					while (token != NULL) {
						struct bss_transition_candi_preference_unit *preference_unit;
						preference_unit = os_zalloc(sizeof(*preference_unit));
						preference_unit->preference = atoi(token);
						dl_list_add_tail(&conf->bss_transition_candi_list, &preference_unit->list);
						conf->have_bss_transition_candi_list = 1;
						token = strtok(NULL, ",");
					}
				}
			} else if (os_strcmp(token, "timezone") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "time zone = %s\n", token);
				conf->time_zone_len = os_strlen(token);
				if (conf->time_zone_len > 0) {
					char time_zone[50];
					conf->time_zone = os_zalloc(conf->time_zone_len);
					conf->have_time_zone = 1;
					os_memcpy(conf->time_zone, token, conf->time_zone_len);
					conf->time_zone[conf->time_zone_len] = '\0';
					/* Set time zone in TZ Environement */
					sprintf(time_zone, "TZ=%s", token);
					putenv(time_zone);
				}
			} else if (os_strcmp(token, "dgaf_disabled") == 0) {
				token = strtok(NULL, "");
				conf->DGAF_disabled = atoi(token);
			} else if (os_strcmp(token, "proxy_arp") == 0) {
				token = strtok(NULL, "");
				conf->proxy_arp = atoi(token);
			} else if (os_strcmp(token, "l2_filter") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "l2_filter = %s\n", token);
				conf->l2_filter = atoi(token);
			} else if (os_strcmp(token, "icmpv4_deny") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "icmpv4_deny = %s\n", token);
				conf->icmpv4_deny = atoi(token);
			} else if (os_strcmp(token, "p2p_cross_connect_permitted") == 0) {
				token = strtok(NULL, "");
				conf->p2p_cross_connect_permitted = atoi(token);
			} else if (os_strcmp(token, "mmpdu_size") == 0) {
				token = strtok(NULL, "");
				conf->mmpdu_size = atoi(token);
			} else if (os_strcmp(token, "external_anqp_server_test") == 0) {
				token = strtok(NULL, "");
				conf->external_anqp_server_test = atoi(token);
			} else if (os_strcmp(token, "gas_cb_delay") == 0) {
				token = strtok(NULL, "");
				conf->gas_cb_delay = atoi(token);
			} else if (os_strcmp(token, "hs2_openmode_test") == 0) {
				token = strtok(NULL, "");
				conf->hs2_openmode_test = atoi(token);
			} else if (os_strcmp(token, "anonymous_nai") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					DBGPRINT(RT_DEBUG_TRACE, "anonymous_nai = %s\n", token);
					conf->have_anonymous_nai = 1;
					conf->anonymous_nai_len = os_strlen(token);
					conf->anonymous_nai = os_zalloc(conf->anonymous_nai_len);
					os_memcpy(conf->anonymous_nai, token, conf->anonymous_nai_len);
				}
			}
			else if (os_strcmp(token, "osu_interface") == 0) {
				token = strtok(NULL, "");
				os_strcpy(conf->osu_iface, token);
				DBGPRINT(RT_DEBUG_TRACE, "OSU Interface = %s\n", conf->osu_iface);
			}
			else if (os_strcmp(token, "legacy_osu") == 0) {
				token = strtok(NULL, "");
				conf->legacy_osu_exist = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "Legacy_osu = %d\n", conf->legacy_osu_exist);
			}
			else if (os_strcmp(token, "icon_path") == 0) {
				token = strtok(NULL, "");
				conf->have_iconfile_path = 1;
				conf->iconfile_path_len = os_strlen(token);
				conf->iconfile_path = os_zalloc(conf->iconfile_path_len);
				os_memcpy(conf->iconfile_path, token, conf->iconfile_path_len);
				DBGPRINT(RT_DEBUG_TRACE, "icon path = %s, len = %d\n", token, conf->iconfile_path_len);
			}
			else if (os_strcmp(token, "qosmap") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->qosmap_enable = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "qosmap_enable = %d\n", conf->qosmap_enable);
				}
			} else if (os_strcmp(token, "dscp_range") == 0) {
				qos_cnt = 0;
				token = strtok(NULL, ":");
				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						conf->dscp_range[qos_cnt] = atoi(token);
						token = strtok(NULL, ":");
						conf->dscp_range[qos_cnt++] |= (atoi(token) << 8);
						token = strtok(NULL, ":");
					}
				}
			} else if (os_strcmp(token, "dscp_exception") == 0) {
				qos_cnt = 0;
				token = strtok(NULL, ":");
				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						conf->dscp_exception[qos_cnt] = atoi(token);
						token = strtok(NULL, ":");
						conf->dscp_exception[qos_cnt++] |= (atoi(token) << 8);
						token = strtok(NULL, ":");
						conf->dscp_field++;
					}
				}
			} else if (os_strcmp(token, "qload_test") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->qload_mode = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "qload mode = %d\n", conf->qload_mode);
				}
			} else if (os_strcmp(token, "qload_cu") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->qload_cu = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "qload_cu = %d\n", conf->qload_cu);
				}
			} else if (os_strcmp(token, "qload_sta_cnt") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->qload_sta_cnt = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "qload_sta_cnt = %d\n", conf->qload_sta_cnt);
				}
			}else if (os_strcmp(token, "mbo_cdcp") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					wapp->mbo->cdcp = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "mbo_cdcp = %d\n", wapp->mbo->cdcp);
				}
			}else if (os_strcmp(token, "mbo_ap_assoc_disallow_reason") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					wapp->mbo->assoc_disallow_reason = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "mbo_ap_assoc_disallow_reason = %d\n", wapp->mbo->assoc_disallow_reason);
				}
			}else if (os_strcmp(token, "mbo_default_assoc_retry_delay") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					wapp->mbo->assoc_retry_delay = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "mbo_default_assoc_retry_delay = %d\n", wapp->mbo->assoc_retry_delay);
				}
			}else if (os_strcmp(token, "mbo_default_trans_reason") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					wapp->mbo->dft_trans_reason = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "mbo_default_trans_reason = %d\n", wapp->mbo->dft_trans_reason);
				}
			}else if (os_strcmp(token, "mbo_ap_capability") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					wapp->mbo->ap_capability = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "mbo_ap_capability = %d\n", wapp->mbo->ap_capability);
				}
			}
		}
	}

	conf->have_anqp_capability_list = 1;
	conf->have_hs_capability_list = 1;

	capability_info = os_zalloc(sizeof(*capability_info));
	capability_info->info_id = ANQP_CAPABILITY;
	dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);

	if (conf->have_venue_name) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = VENUE_NAME_INFO;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_network_auth_type) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = NETWORK_AUTH_TYPE_INFO;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_roaming_consortium_list) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = ROAMING_CONSORTIUM_LIST;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_ip_address_type) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = IP_ADDRESS_TYPE_AVAILABILITY_INFO;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_nai_realm_list) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = NAI_REALM_LIST;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
		conf->have_nai_home_realm_query = 1;
	}

	if (conf->have_3gpp_network_info) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = ThirdGPP_CELLULAR_NETWORK_INFO;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_domain_name_list) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = DOMAIN_NAME_LIST;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_advice_of_charge) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = ADVICE_OF_CHARGE;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	/* Following are HS2.0 capability list */
	hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
	hs_capability_subtype->subtype = HS_CAPABILITY;
	dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);

	if (conf->have_operator_friendly_name) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = OPERATOR_FRIENDLY_NAME;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_wan_metrics) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = WAN_METRICS;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_connection_capability_list) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = CONNECTION_CAPABILITY;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_nai_realm_list) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = NAI_HOME_REALM_QUERY;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_operating_class) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = OPERATING_CLASS;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_osu_providers_list) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = OSU_PROVIDE_LIST;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_anonymous_nai) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = ANONYMOUS_NAI;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_osu_providers_list) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
        hs_capability_subtype->subtype = ICON_REQUEST;
        dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);

        hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
        hs_capability_subtype->subtype = ICON_BINARY_FILE;
        dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
    }

	dl_list_add_tail(&wapp->conf_list, &conf->list);

	fclose(file);

	if(wapp->wapp_default_config == NULL)
		wapp->wapp_default_config = conf;


	/* Set interworking capability to driver */
	wapp_set_interworking_enable(wapp, conf->iface, (char *)&conf->interworking);

	/* Set parameter to driver */
	if (conf->interworking)
		ret = wapp_init_param_setting(wapp, conf);


	return ret;

error1:
	fclose(file);
error:
	os_free(conf);
	return -1;
}


static int wapp_init_sta_config(struct wifi_app *wapp, const char *confname)
{
	int ret = 0;
	FILE *file;
	char buf[256], *pos, *token;
	char tmpbuf[256], tmp1buf[256], tmp2buf[256];
	int line = 0, i;
	struct wapp_conf *conf;
	int query_id;

	DBGPRINT(RT_DEBUG_TRACE, "%s(%s)\n", __FUNCTION__, confname);

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);
	os_memset(tmp1buf, 0, 256);
	os_memset(tmp2buf, 0, 256);

	conf = os_zalloc(sizeof(struct wapp_conf));

	if (!conf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available\n"));
		return -1;
	}

	/* Following are 802.11u element list */
	dl_list_init(&conf->anqp_capability_list);
	dl_list_init(&conf->venue_name_list);
	dl_list_init(&conf->emergency_call_number_list);
	dl_list_init(&conf->network_auth_type_list);
	dl_list_init(&conf->oi_duple_list);
	dl_list_init(&conf->nai_realm_list);
	dl_list_init(&conf->plmn_list);
	dl_list_init(&conf->domain_name_list);

	/* Following are HS2.0 elemets list */
	dl_list_init(&conf->hs_capability_list);
	dl_list_init(&conf->operator_friendly_duple_list);
	dl_list_init(&conf->connection_capability_list);
	dl_list_init(&conf->nai_home_realm_name_query_list);

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
		goto error;
	}

	while (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		os_strcpy(tmpbuf, pos);
		token = strtok(pos, "=");
		if (token != NULL) {
			if (os_strcmp(token, "interface") == 0) {
				token = strtok(NULL, "");
				os_strcpy(conf->iface, token);
				DBGPRINT(RT_DEBUG_TRACE, "interface = %s\n", conf->iface);
			} else if (os_strcmp(token, "hs_peer_mac") == 0) {
				i = 0;
				token = strtok(NULL, ":");
				while (token != NULL) {
					AtoH(token, &conf->hs_peer_mac[i], 1);
					DBGPRINT(RT_DEBUG_TRACE, "hs_peer_mac[%d] = 0x%02x\n", i, conf->hs_peer_mac[i]);
					i++;
					token = strtok(NULL, ":");
				}
			} else if (os_strcmp(token, "ANQPQueryID") == 0) {
				token = strtok(NULL, ";");
				while (token != NULL) {
					query_id = atoi(token);
					switch (query_id) {
					case ANQP_CAPABILITY:
						conf->query_anqp_capability_list = 1;
						break;
					case VENUE_NAME_INFO:
						conf->query_venue_name = 1;
						break;
					case EMERGENCY_CALL_NUMBER_INFO:
						conf->query_emergency_call_number = 1;
						break;
					case NETWORK_AUTH_TYPE_INFO:
						conf->query_network_auth_type = 1;
						break;
					case ROAMING_CONSORTIUM_LIST:
						conf->query_roaming_consortium_list = 1;
						break;
					case IP_ADDRESS_TYPE_AVAILABILITY_INFO:
						conf->query_ip_address_type = 1;
						break;
					case NAI_REALM_LIST:
						conf->query_nai_realm_list = 1;
						break;
					case ThirdGPP_CELLULAR_NETWORK_INFO:
						conf->query_3gpp_network_info = 1;
						break;
					case AP_GEOSPATIAL_LOCATION:
						conf->query_ap_geospatial_location = 1;
						break;
					case AP_CIVIC_LOCATION:
						conf->query_ap_civic_location = 1;
						break;
					case AP_LOCATION_PUBLIC_IDENTIFIER_URI:
						conf->query_ap_location_public_uri = 1;
						break;
					case DOMAIN_NAME_LIST:
						conf->query_domain_name_list = 1;
						break;
					case EMERGENCY_ALERT_IDENTIFIER_URI:
						conf->query_emergency_alert_uri = 1;
						break;
					case EMERGENCY_NAI:
						conf->query_emergency_nai = 1;
						break;
					default:
						DBGPRINT(RT_DEBUG_ERROR, "Unknown QueryID\n");
						break;
					}
					token = strtok(NULL, ";");
				}
			} else if (os_strcmp(token, "ANQPQueryType") == 0) {
				token = strtok(NULL, ";");
				conf->anqp_req_type = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "ANQPQueryType = %d\n", conf->anqp_req_type);
				token = strtok(NULL, "");
			} else if (os_strcmp(token, "HSANQPQueryID") == 0) {
				token = strtok(NULL, ";");
				while (token != NULL) {
					query_id = atoi(token);
					switch (query_id) {
					case HS_CAPABILITY:
						conf->query_hs_capability_list = 1;
						break;
					case OPERATOR_FRIENDLY_NAME:
						conf->query_operator_friendly_name = 1;
						break;
					case WAN_METRICS:
						conf->query_wan_metrics = 1;
						break;
					case CONNECTION_CAPABILITY:
						conf->query_connection_capability_list = 1;
						break;
					case NAI_HOME_REALM_QUERY:
						conf->query_nai_home_realm = 1;
						break;
					default:
						DBGPRINT(RT_DEBUG_ERROR, "Unknown HS2.0 QueryID\n");
						break;

					}
					token = strtok(NULL, ";");
				}
			}
		}
	}

	dl_list_add_tail(&wapp->conf_list, &conf->list);

	fclose(file);

	return ret;

error:
	os_free(conf);
	return -1;
}


int wapp_init_all_config(struct wifi_app *wapp, const char *confname)
{

	FILE *file;
	char buf[256], *pos, *token, *tokentmp;
	int line = 0, ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s  confname[%s]\n", __FUNCTION__,confname);

	os_memset(buf, 0, 256);
	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, ("reading configuration fail, maybe do not have this file\n"));
		goto error;
	}

	wapp->wapp_default_config = NULL;

	dl_list_init(&wapp->conf_list);
	while (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		token = strtok(pos, "=");

		if (token != NULL) {
			if (os_strcmp(token, "conf_list") == 0) {
				token = strtok(NULL, ";");
				while (token != NULL) {
					tokentmp = token + os_strlen(token) + 1;
					if (wapp->opmode == OPMODE_STA)
						ret = wapp_init_sta_config(wapp, token);
					else
						ret = wapp_init_ap_config(wapp, token);

					token = strtok(tokentmp, ";");
				}
			}
		}
	}
	fclose(file);

	/* init location IE */
	os_memset(wapp->hs->civic_IE, 0, LOCATION_IE_LEN);
	wapp->hs->civic_IE_len = 0;
	os_memset(wapp->hs->lci_IE, 0, LOCATION_IE_LEN);
	wapp->hs->lci_IE_len = 0;
	os_memset(wapp->hs->public_id_uri, 0, LOCATION_IE_LEN);
	wapp->hs->public_id_uri_len = 0;

error:
	return ret;
}

int wapp_deinit_config(struct wifi_app *wapp, struct wapp_conf *conf)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (conf->have_anqp_capability_list) {
		struct anqp_capability *capability_info, *capability_info_tmp;
		dl_list_for_each_safe(capability_info, capability_info_tmp, &conf->anqp_capability_list,
										struct anqp_capability, list) {
			dl_list_del(&capability_info->list);
			os_free(capability_info);
		}
	}

	if (conf->have_venue_name) {
		struct venue_name_duple *vname_duple, *vname_duple_tmp;
		dl_list_for_each_safe(vname_duple, vname_duple_tmp, &conf->venue_name_list,
										struct venue_name_duple, list) {
			dl_list_del(&vname_duple->list);
			os_free(vname_duple);
		}
	}

	if (conf->have_roaming_consortium_list) {
		struct oi_duple *oiduple, *oiduple_tmp;
		dl_list_for_each_safe(oiduple, oiduple_tmp, &conf->oi_duple_list,
										struct oi_duple, list) {
			dl_list_del(&oiduple->list);
			os_free(oiduple);
		}
	}


	if (conf->have_nai_realm_list) {
		struct nai_realm_data *nai_realm, *nai_realm_tmp;
		struct eap_method *eapmethod, *eapmethod_tmp;
		struct auth_param *authparam, *authparam_tmp;
		dl_list_for_each_safe(nai_realm, nai_realm_tmp, &conf->nai_realm_list,
										struct nai_realm_data, list) {
			dl_list_del(&nai_realm->list);
			dl_list_for_each_safe(eapmethod, eapmethod_tmp, &nai_realm->eap_method_list,
										struct eap_method, list) {

				dl_list_del(&eapmethod->list);
				dl_list_for_each_safe(authparam, authparam_tmp, &eapmethod->auth_param_list,
										struct auth_param, list) {
					dl_list_del(&authparam->list);
					os_free(authparam);
				}

				os_free(eapmethod);
			}
			os_free(nai_realm);

		}
	}

	if (conf->have_3gpp_network_info) {
		struct plmn *plmn_unit, *plmn_unit_tmp;
		dl_list_for_each_safe(plmn_unit, plmn_unit_tmp, &conf->plmn_list,
								struct plmn, list) {
			dl_list_del(&plmn_unit->list);
			os_free(plmn_unit);
		}
	}

	if (conf->have_domain_name_list) {
		struct domain_name_field *dname_field, *dname_field_tmp;
		dl_list_for_each_safe(dname_field, dname_field_tmp, &conf->domain_name_list,
										struct domain_name_field, list) {
			dl_list_del(&dname_field->list);
			os_free(dname_field);
		}
	}

	if (conf->have_network_auth_type) {
		struct net_auth_type_unit *auth_type_unit, *auth_type_unit_tmp;
		dl_list_for_each_safe(auth_type_unit, auth_type_unit_tmp, &conf->network_auth_type_list,
										struct net_auth_type_unit, list) {
			dl_list_del(&auth_type_unit->list);
			os_free(auth_type_unit);
		}
	}

	/* Following are hotspot2.0 elements */
	if (conf->have_hs_capability_list) {
		struct anqp_hs_capability *hs_capability_subtype, *hs_capability_subtype_tmp;
		dl_list_for_each_safe(hs_capability_subtype, hs_capability_subtype_tmp,
							  &conf->hs_capability_list, struct anqp_hs_capability, list) {
			dl_list_del(&hs_capability_subtype->list);
			os_free(hs_capability_subtype);
		}
	}

	if (conf->have_operator_friendly_name) {
		struct operator_name_duple *op_name_duple, *op_name_duple_tmp;
		dl_list_for_each_safe(op_name_duple, op_name_duple_tmp, &conf->operator_friendly_duple_list,
										struct operator_name_duple, list) {
			dl_list_del(&op_name_duple->list);
			os_free(op_name_duple);
		}
	}

	if (conf->have_connection_capability_list) {
		struct proto_port_tuple *proto_port, *proto_port_tmp;
		dl_list_for_each_safe(proto_port, proto_port_tmp, &conf->connection_capability_list,
										struct proto_port_tuple, list) {
			dl_list_del(&proto_port->list);
			os_free(proto_port);
		}
	}

	if (conf->have_operating_class) {
		struct operating_class_unit *operating_class, *operating_class_tmp;
		dl_list_for_each_safe(operating_class, operating_class_tmp,
						&conf->operating_class_list, struct operating_class_unit, list) {

			dl_list_del(&operating_class->list);
			os_free(operating_class);
		}
	}

	if (conf->have_osu_providers_list) {
		struct osu_providers *providers_list, *providers_list_tmp;
		struct osu_friendly_name *friendly_name_list, *friendly_name_list_tmp;
		struct osu_method *method_list, *method_list_tmp;
		struct icon_available *icon_list, *icon_list_tmp;
		struct osu_nai *nai_list, *nai_list_tmp;
		struct osu_service_desc *service_desc_list, *service_desc_list_tmp;

		dl_list_for_each_safe(providers_list, providers_list_tmp,
						&conf->osu_providers_list, struct osu_providers, list) {
			dl_list_del(&providers_list->list);

			dl_list_for_each_safe(friendly_name_list, friendly_name_list_tmp, &providers_list->osu_friendly_name_list,
										struct osu_friendly_name, list) {
				dl_list_del(&friendly_name_list->list);
				os_free(friendly_name_list);
			}

			dl_list_for_each_safe(method_list, method_list_tmp, &providers_list->osu_method_list,
										struct osu_method, list) {
				dl_list_del(&method_list->list);
				os_free(method_list);
			}

			dl_list_for_each_safe(icon_list, icon_list_tmp, &providers_list->icon_list,
										struct icon_available, list) {
				dl_list_del(&icon_list->list);
				os_free(icon_list);
			}

			dl_list_for_each_safe(nai_list, nai_list_tmp, &providers_list->osu_nai_list,
										struct osu_nai, list) {
				dl_list_del(&nai_list->list);
				os_free(nai_list);
			}

			dl_list_for_each_safe(service_desc_list, service_desc_list_tmp, &providers_list->osu_service_desc_list,
										struct osu_service_desc, list) {
				dl_list_del(&service_desc_list->list);
				os_free(service_desc_list);
			}

			if (providers_list->osu_server_uri_len != 0)
				os_free(providers_list->osu_server_uri);

			os_free(providers_list);
		}
	}

	if (conf->have_osu_providers_nai_list) {
		struct osu_providers_nai_duple *osu_prov_nai_duple, *osu_prov_nai_duple_tmp;
		dl_list_for_each_safe(osu_prov_nai_duple, osu_prov_nai_duple_tmp, &conf->osu_providers_nai_duple_list,
											struct osu_providers_nai_duple, list) {
			dl_list_del(&osu_prov_nai_duple->list);
			os_free(osu_prov_nai_duple);
		}
	}

	if (conf->have_icon_metadata) {
		struct operator_icon_metadata *oim_data, *oim_data_tmp;
		dl_list_for_each_safe(oim_data, oim_data_tmp, &conf->icon_metadata_list,
											struct operator_icon_metadata, list) {
			dl_list_del(&oim_data->list);
			os_free(oim_data);
		}
	}

	if (conf->have_anonymous_nai)
		os_free(conf->anonymous_nai);

	if (conf->have_iconfile_path)
		os_free(conf->iconfile_path);

	if (conf->have_session_info_url)
		os_free(conf->session_info_url);

	if (conf->have_t_c_filename)
		os_free(conf->t_c_filename);

	if (conf->have_t_c_server_url)
		os_free(conf->t_c_server_url);

	if (conf->have_t_c_timestamp)
		os_free(conf->t_c_timestamp);

	if (conf->have_bss_transition_candi_list) {
		struct bss_transition_candi_preference_unit *preference_unit, *preference_unit_tmp;
		dl_list_for_each_safe(preference_unit, preference_unit_tmp, &conf->bss_transition_candi_list,
				struct bss_transition_candi_preference_unit, list) {
			dl_list_del(&preference_unit->list);
			os_free(preference_unit);
		}
	}

	if(conf->have_venue_url) {
		struct venue_url_duple *vurl_duple, *vurl_duple_tmp;
		dl_list_for_each_safe(vurl_duple, vurl_duple_tmp, &conf->venue_url_list,
				struct venue_url_duple, list) {
			dl_list_del(&vurl_duple->list);
			os_free(vurl_duple);
		}
	}

	if (conf->have_advice_of_charge) {
		struct advice_of_charge_data *charge_data, *charge_data_tmp;
		struct aoc_plan_tuple_data *plan_tuple_data, *plan_tuple_data_tmp;
		dl_list_for_each_safe(charge_data, charge_data_tmp, &conf->advice_of_charge_list,
												struct advice_of_charge_data, list) {
			dl_list_del(&charge_data->list);
			dl_list_for_each_safe(plan_tuple_data, plan_tuple_data_tmp, &charge_data->aoc_plan_tuples_list,
												struct aoc_plan_tuple_data, list) {
				dl_list_del(&plan_tuple_data->list);
				os_free(plan_tuple_data);
			}
			os_free(charge_data);
		}
	}

	if (conf->have_time_zone)
		os_free(conf->time_zone);

	return ret;
}

int wapp_deinit_all_config(struct wifi_app *wapp)
{
	struct wapp_conf *conf, *conf_tmp;
#ifdef MAP_SUPPORT
	struct air_monitor_query_rsp *mntr, *mntr_tmp;
	struct sta_mnt_stat *sta_stat,*sta_stat_tmp;
	struct bh_link *bh_links = NULL, *bh_links_tmp = NULL;
#endif
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	dl_list_for_each_safe(conf, conf_tmp, &wapp->conf_list, struct wapp_conf, list) {
		wapp_deinit_config(wapp, conf);
		dl_list_del(&conf->list);
		os_free(conf);
	}
#ifdef MAP_SUPPORT
	dl_list_for_each_safe(mntr, mntr_tmp, &wapp->air_monitor_query_list, struct air_monitor_query_rsp, list) {
		dl_list_del(&mntr->list);
		os_free(mntr);
	}

	dl_list_for_each_safe(sta_stat, sta_stat_tmp, &wapp->sta_mntr_list, struct sta_mnt_stat, list) {
		dl_list_del(&sta_stat->list);
		os_free(sta_stat);
	}

	if (!dl_list_empty(&(wapp->map->bh_link_list))){
		dl_list_for_each_safe(bh_links, bh_links_tmp, &wapp->map->bh_link_list, struct bh_link, list) {
			dl_list_del(&bh_links->list);
			os_free(bh_links);
		}
	}
	wapp->map->bh_link_num = 0;
#endif
	return 0;
}

int wapp_set_ie(struct wifi_app *wapp, const char *iface,
				          char *ie, size_t ie_len)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_set_ie(wapp->drv_data, iface, ie, ie_len);

	return ret;
}

u8 wapp_aquire_card_id(
	struct wifi_app *wapp,
	u32 adpt_id)
{
	u8 i = 0, max_card_id = 0, new_card_id = 0;
#if 0
	bool no_valid_adpt = TRUE;
#endif
	struct wapp_radio *ra = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	/* base on the fact that each card have its own adpt_id */
	for ( i = 0; i < MAX_NUM_OF_RADIO; i++)
	{
		/* check if adpt exsits */
		ra = &wapp->radio[i];
		if (ra->adpt_id != 0) {
#if 0
			no_valid_adpt = FALSE;
#endif
			if (ra->adpt_id == adpt_id) {
				/* adpt exist, return the card id */
				return ra->card_id;
			} else {
				/* record the max card_id */
				max_card_id = (ra->card_id > max_card_id) ? \
							  ra->card_id : max_card_id;
			}
		}
	}

#if 0 /* card_id starts from 0 */
	/* if there is no any existing card, assign 0 as the first card id */
	new_card_id = (no_valid_adpt == TRUE) ? \
				  0 : (max_card_id+1)
#else /* card_id starts from 1 */
	new_card_id = (max_card_id+1);
#endif

	/* no existing adpt_id, so this is a new card */
	DBGPRINT_RAW(RT_DEBUG_OFF,
			BLUE("(%u) aquired a new card id %u\n"), adpt_id, new_card_id);
	return new_card_id;
}

struct wapp_radio* wapp_radio_create(
	struct wifi_app *wapp,
	u32 adpt_id,
	u8 ra_id)
{
	u8 i;
	struct wapp_radio *ra = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	for ( i = 0; i < MAX_NUM_OF_RADIO; i++)
	{
		ra = &wapp->radio[i];
#if 0
		printf("\033[1;36m adpt_id = %u, ra_id = %u \033[0m\n",
			ra->adpt_id,
			ra->radio_id);
#endif

		if (ra->adpt_id == 0) {
			ra->index = i;
			ra->card_id = wapp_aquire_card_id(wapp, adpt_id);
			ra->adpt_id = adpt_id;
			ra->radio_id = ra_id;
#if 1 //Haipin: TODO: get radio onoff from driver
			ra->onoff = RADIO_ON;
#endif
#if 0
			printf("\033[1;36m adpt_id = %u, ra_id = %u \033[0m\n",
					ra->adpt_id,
					ra->radio_id);
#endif
#ifdef WIFI_MD_COEX_SUPPORT
			ra->operatable = 1;
#endif
			break;
		}
	}

	return ra;
}

struct wapp_radio* wapp_radio_lookup(
	struct wifi_app *wapp,
	u32	adpt_id,
	u8	ra_id)
{
	u8 i;
	struct wapp_radio *ra = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	for ( i = 0; i < MAX_NUM_OF_RADIO; i++)
	{
		if (adpt_id == wapp->radio[i].adpt_id &&
			ra_id == wapp->radio[i].radio_id) {
			ra = &wapp->radio[i];
			break;
		}
	}

	return ra;
}

struct wapp_radio* wapp_radio_update_or_create(
	struct wifi_app *wapp,
	u32 adpt_id,
	u8 radio_id)

{
	struct wapp_radio *ra = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	ra = wapp_radio_lookup(wapp, adpt_id, radio_id);

	if (ra == NULL) {
		ra = wapp_radio_create(
				wapp,
				adpt_id,
				radio_id);
#if 1
		if (ra == NULL) {
			DBGPRINT_RAW(RT_DEBUG_OFF,
			"\033[1;31m ra table full \033[0m\n");
		}
#endif

#if 0
		if (wdev->radio == NULL)
			wdev->radio = ra;
#if 1
		else if (wdev->radio != ra) {
			DBGPRINT_RAW(RT_DEBUG_OFF,
			"\033[1;31m ra change? \033[0m\n");
		}
#endif
#endif

	} else {
		ra->adpt_id = adpt_id;
		ra->radio_id= radio_id;
	}

#ifdef DPP_SUPPORT
	/* TODO initialize radio DPP variables here, sepcifically for MAP-R3 */
	ra->ongoing_dpp_cnt = 0;
#endif
	return ra;
}

int wapp_radio_update_ch(
	struct wifi_app *wapp,
	struct wapp_radio *ra,
	u8 ch)

{
	if (!wapp || !ra)
		return WAPP_INVALID_ARG;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ra->op_ch = ch;
#ifdef MAP_SUPPORT
	map_update_radio_band(wapp, ra, ch);
#endif /* MAP_SUPPORT */

	return WAPP_SUCCESS;
}

u8 is_all_zero_mac(u8 *mac_addr)
{
	char all_zero_mac[6]={0};
	if (NdisCompareMemory(mac_addr, all_zero_mac, ETH_ALEN) == 0)
		return TRUE;
	else
		return FALSE;
}
struct probe_info * wapp_probe_lookup(struct wifi_app *wapp, u8 *mac_addr)
{
	int i;
	struct probe_info *probe = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	for (i = 0; i < PROBE_TABLE_SIZE; i++) {
		probe = &wapp->probe_entry[i];
		if (NdisCompareMemory(probe->mac_addr, mac_addr, ETH_ALEN) == 0)
			break; //found
	}

	if (i == PROBE_TABLE_SIZE)
		return NULL;

	return probe;
}

struct probe_info * wapp_probe_create(struct wifi_app *wapp, u8 *mac_addr)
{
	struct probe_info *info;
	int i;
	static u8 oldest_idx=0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	for (i = 0; i < PROBE_TABLE_SIZE; i++) {
		if (wapp->probe_entry[i].valid == 0)
			break; //found
	}

	if (i == PROBE_TABLE_SIZE) {
		info = &wapp->probe_entry[oldest_idx++];
		oldest_idx = oldest_idx % PROBE_TABLE_SIZE;
	}
	else {
		info = &wapp->probe_entry[i];
	}

	info->valid = 1;
	COPY_MAC_ADDR(info->mac_addr, mac_addr);
	return info;
}


int wapp_cmd_show_probe_info( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	int i, num = 0;
	struct probe_info *info = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	for (i = 0; i < PROBE_TABLE_SIZE; i++) {
		info = &wapp->probe_entry[i];
		if (info->valid == 1) {
			DBGPRINT(RT_DEBUG_OFF,
				"\n [%d]"
				"\t mac_addr = %02x:%02x:%02x:%02x:%02x:%02x\n"
				"\t channel = %u\n"
				"\t rssi = %u\n"
				"\t last_update_time = %lu Sec\n",
				num,
				PRINT_MAC(info->mac_addr),
				info->channel,
				info->rssi,
				info->last_update_time.sec);
			num++;
		}
	}
	return WAPP_SUCCESS;
}

int wapp_driver_version(struct wifi_app *wapp, const char *iface, char *ver, size_t *len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_wifi_version(wapp->drv_data, iface, ver, len);

	return ret;
}

void wapp_reset_map_params(struct wifi_app *wapp, struct wapp_dev *wdev)
{

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);
	char local_command[64];
	os_memset(local_command, 0, sizeof(local_command));
	os_snprintf(local_command, sizeof(local_command), "iwpriv %s set mapEnable=0",
		wdev->ifname);
	system(local_command);
	os_memset(local_command, 0, sizeof(local_command));
		os_snprintf(local_command, sizeof(local_command), "iwpriv %s set mapEnable=%d",
			wdev->ifname, wapp->map->MapMode);
	system(local_command);
}


void wapp_soft_reset_scan_states(struct wifi_app *wapp)
{
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);
	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		char local_command[64];
		if (wdev && (wdev->dev_type == WAPP_DEV_TYPE_STA)) {
			DBGPRINT(RT_DEBUG_OFF, "attempt disconnection for: %s\n", wdev->ifname);
			DBGPRINT(RT_DEBUG_OFF, "trigger disconnection %s\n", wdev->ifname);
			wdev->wps_triggered = FALSE;
			wdev->scan_cookie = 0;
			os_memset(local_command, 0, sizeof(local_command));
			os_snprintf(local_command, sizeof(local_command), "iwpriv %s set ApCliEnable=0",
			       wdev->ifname);
			system(local_command);
		}
	}
}

void wapp_reset_scan_states(struct wifi_app *wapp)
{
	wsc_apcli_config_msg apcli_config_msg;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;
	int i = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);
	dev_list = &wapp->dev_list;

	save_map_parameters(wapp,"BhProfile0Valid", "0", NON_DRIVER_PARAM);
	save_map_parameters(wapp,"BhProfile1Valid", "0", NON_DRIVER_PARAM);
	save_map_parameters(wapp,"BhProfile2Valid", "0", NON_DRIVER_PARAM);
	save_map_parameters(wapp,"BhProfile3Valid", "0", NON_DRIVER_PARAM);
	save_map_parameters(wapp,"BhProfile4Valid", "0", NON_DRIVER_PARAM);
	save_map_parameters(wapp,"BhProfile5Valid", "0", NON_DRIVER_PARAM);
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		wapp->map->apcli_configs[i].config_valid = FALSE;
	}

	if(wapp->wsc_save_bh_profile == TRUE)
		apcli_config_msg.profile_count = WSC_APCLI_CONFIG_MSG_SAVE_PROFILE;
	else
		apcli_config_msg.profile_count = 0;

	wapp_send_1905_msg(wapp, WAPP_MAP_BH_CONFIG, sizeof(apcli_config_msg), (void *)&apcli_config_msg);

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		char local_command[64];
		if (wdev && (wdev->dev_type == WAPP_DEV_TYPE_STA)) {
			wdev->wps_triggered = FALSE;
			wdev->scan_cookie = 0;
			os_memset(local_command, 0, sizeof(local_command));
			os_snprintf(local_command, sizeof(local_command), "iwpriv %s set ApCliEnable=0",
				wdev->ifname);
			system(local_command);
			eloop_cancel_timeout(map_get_scan_result, wapp, wdev);
		}
	}

	if(wapp->wsc_save_bh_profile)
	    wapp->map->bh_link_ready = 0;
}
int wapp_wps_pbc_trigger(struct wifi_app *wapp, const char *iface, char *ver, size_t *len)
{
#ifdef MAP_SUPPORT
	int ret = TRUE;
	BOOLEAN is_map_device_configured = FALSE;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);
	wapp_device_status *device_status = &wapp->map->device_status;
	os_memset(device_status, 0, sizeof(wapp_device_status));
	if(!wapp->wps_on_controller_cli ) {
		if(wapp->map->bh_link_ready && wapp->map->bh_link_num) {
			if (wapp->map->ctrler_found) {
				is_map_device_configured = TRUE;
			} else {
				return FALSE;
			}
		}
		if (wapp->map->ctrler_found && !wapp->map->bh_link_ready ) {  //controller
			is_map_device_configured = TRUE;
		}
	}
	if ((!is_map_device_configured)||(wapp->wps_on_controller_cli)) {
		device_status->status_bhsta = STATUS_BHSTA_WPS_TRIGGERED;
		device_status->status_fhbss = STATUS_FHBSS_UNCONFIGURED;
		if (wapp->map->bh_link_ready)
			wapp->wsc_save_bh_profile = TRUE;
		wapp_reset_scan_states(wapp);
		wapp->wsc_trigger_wdev
			= wps_ctrl_run_cli_wps(wapp, NULL);
		wapp->map->conf = MAP_CONN_STATUS_UNCONF;
	} else {
		device_status->status_bhsta = STATUS_BHSTA_CONFIGURED;
		device_status->status_fhbss = STATUS_FHBSS_WPS_TRIGGERED;
		if (wapp->map->off_ch_scan_state.ch_scan_state == CH_SCAN_IDLE)
			wps_ctrl_run_ap_wps(wapp);
		else {
			wapp->map->wps_after_scan = 1;
			return ret;
		}
	}
	eloop_cancel_timeout(map_wps_timeout, wapp, device_status);
	eloop_register_timeout(WPS_TIMEOUT, 0, map_wps_timeout, wapp, device_status);
	wapp_send_1905_msg(
		wapp,
		WAPP_DEVICE_STATUS,
		sizeof(wapp_device_status),
		(char *)device_status);
	return ret;
#else
	return FALSE;
#endif
}
int wapp_drv_support_version_check(struct wifi_app *wapp, const char *iface)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_wapp_version_check(wapp->drv_data, iface);

	return ret;
}

int wapp_get_misc_cap(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_misc_cap(wapp->drv_data, iface, buf, &buf_len);

	return ret;
}
int wapp_get_ht_cap(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_ht_cap(wapp->drv_data, iface, buf, &buf_len);

	return ret;
}

int wapp_get_vht_cap(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_vht_cap(wapp->drv_data, iface, buf, &buf_len);

	return ret;
}

int wapp_get_chan_list(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_chan_list(wapp->drv_data, iface, buf, &buf_len);

	return ret;
}

int wapp_get_nop_channels(struct wifi_app *wapp, const char *iface, char *buf,
		size_t buf_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_nop_channels(wapp->drv_data, iface, buf, &buf_len);

	return ret;
}

int wapp_get_op_class(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_op_class(wapp->drv_data, iface, buf, &buf_len);

	return ret;
}

int wapp_get_bss_info(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_bss_info(wapp->drv_data, iface, buf, &buf_len);

	return ret;
}
int wapp_get_ap_metrics(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
	{
		int ret;

		DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

		ret = wapp->drv_ops->drv_get_ap_metrics(wapp->drv_data, iface, buf, &buf_len);

		return ret;
	}

int wapp_get_chip_id(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
{
		int ret;

		DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

		ret = wapp->drv_ops->drv_get_chip_id(wapp->drv_data, iface, buf, &buf_len);

		return ret;
}

int wapp_set_scan_BH_ssids(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct scan_BH_ssids *scan_ssids)
{
	int ret;
	struct wapp_req req;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	os_memset(&req, 0, sizeof(struct scan_BH_ssids));
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}
	req.req_id = WAPP_SET_SCAN_BH_SSIDS;
	req.data.ifindex = wdev->ifindex;
	os_memcpy(&req.data.scan_bh_ssids, scan_ssids,
		sizeof(struct scan_BH_ssids));
	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;
}

int wapp_create_arp_socket(struct wifi_app *wapp)
{
	struct br_dev *pbr = NULL;
	struct wapp_dev *wdev = NULL;
	int ret = -1, err = 0;

	/*create arp socket for bridge*/
	pbr = &wapp->map->br;
	err = get_if_info(wapp->map->br_iface, &pbr->ip, pbr->mac_addr, &pbr->ifindex);
	if (err < 0) {
		if (err == -2) {
			DBGPRINT(RT_DEBUG_ERROR, "get %s ip fail, continue\n", wapp->map->br_iface);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "get %s info fail\n", wapp->map->br_iface);
			goto out;
		}
	}
	pbr->arp_sock = 0;
	if (bind_arp(pbr->ifindex, &pbr->arp_sock)) {
		DBGPRINT(RT_DEBUG_ERROR, "create/bind arp socket for %s fail\n",
			wapp->map->br_iface);
        goto out;
    }

	/*create arp socket for wifi interface*/
	dl_list_for_each(wdev, &wapp->dev_list, struct wapp_dev, list){
		wdev->arp_sock = 0;
		if (bind_arp(wdev->ifindex, &wdev->arp_sock)) {
			DBGPRINT(RT_DEBUG_ERROR, "create/bind arp socket for %s fail\n",
				wdev->ifname);
			goto out;
		}
	}

	ret = 0;
out:
	if (ret) {
		if (pbr->arp_sock > 0)
        	close(pbr->arp_sock);
		dl_list_for_each(wdev, &wapp->dev_list, struct wapp_dev, list){
			if (wdev->arp_sock > 0)
				close(wdev->arp_sock);
		}
    }
    return ret;
}

int wapp_set_AvoidScanDuringCAC(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	unsigned char enable)
{
	int ret;
	struct wapp_req req;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;
	req.req_id = WAPP_SET_AVOID_SCAN_CAC;
	req.data.ifindex = wdev->ifindex;
	req.data.value = enable;
	ret = wapp_req_send(wapp, wdev->ifname, &req);
	return ret;
}
int wapp_get_he_cap(struct wifi_app *wapp, const char *iface, char *buf,
				       		  size_t buf_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_he_cap(wapp->drv_data, iface, buf, &buf_len);

	return ret;
}

int wapp_get_valid_radio_count (struct wifi_app *wapp)
{
        int i = 0;
        int radio_count = 0;
        for (i = 0; i < MAX_NUM_OF_RADIO; i++)
        {
                if (wapp->radio[i].adpt_id)
                        radio_count++;
        }
        return radio_count;
}

int get_parameters(char *name, char *param, char *value, param_type type, size_t val_len)
{
#ifdef OPENWRT_SUPPORT
	const char *tmp_value = NULL;
	unsigned int len = 0;
	struct kvc_context *dat_ctx = NULL;
	const char *file = NULL;
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s param(%s) from file(%s)\n",__func__, param, name);

	os_memset(value, 0, val_len);

	if (type == NON_DRIVER_PARAM)
		file = name;
	else
		file = get_dat_path_by_ord(0);

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "invalid file!!! type(%d)\n", type);
		return -1;
	}

	dat_ctx = dat_load(file);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", file);
		ret = -1;
		goto out;
	}

	tmp_value = kvc_get(dat_ctx, (const char *)param);
	if (!tmp_value) {
		DBGPRINT(RT_DEBUG_ERROR, "get param(%s) fail\n", param);
		ret = -1;
		goto out;
	}
	len = os_min(os_strlen(tmp_value), val_len - 1);
	os_memcpy(value, tmp_value, len);

	DBGPRINT(RT_DEBUG_TRACE, "%s value(%s)\n",__func__, value);
out:
	if (file && (type == DRIVER_PARAM))
		free_dat_path(file);
	if (dat_ctx)
		kvc_unload(dat_ctx);

	return ret;
#else
	return 0;
#endif
}

int set_parameters(char *name, char *param, char *value, param_type type)
{
#ifdef OPENWRT_SUPPORT
	struct kvc_context *dat_ctx = NULL;
	int ret = 0;
	const char *file = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s param(%s) value(%s) to file(%s)\n",__func__,
		param, value, name);

	if (type == NON_DRIVER_PARAM)
		file = name;
	else
		file = get_dat_path_by_ord(0);

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "invalid file!!! type(%d)\n", type);
		return -1;
	}

	dat_ctx = dat_load((const char *)file);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", file);
		ret = -1;
		goto out;
	}

	ret = kvc_set(dat_ctx, (const char *)param, (const char *)value);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "set param(%s) fail\n", param);
		goto out;
	}
	ret = kvc_commit(dat_ctx);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "write param(%s) fail\n", param);
		goto out;
	}
out:
	if (file && (type == DRIVER_PARAM))
		free_dat_path(file);
	if (dat_ctx)
		kvc_unload(dat_ctx);
	if (ret)
		return -1;
	else
		return 0;
#else
	return 0;
#endif
}
#ifdef MAP_R2
#ifdef DFS_CAC_R2
int wapp_set_channel_monitor_assign(struct wifi_app *wapp, const char *iface,
				          char *msg, size_t msg_len)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_set_monitor_ch_assign(wapp->drv_data, iface, msg, msg_len);

	return ret;
}
int wapp_get_channel_avail_ch_list(struct wifi_app *wapp, const char *iface,
				          char *msg, size_t msg_len)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = wapp->drv_ops->drv_get_ch_avail_list(wapp->drv_data, iface, msg, msg_len);

	return ret;
}

#endif
#endif

