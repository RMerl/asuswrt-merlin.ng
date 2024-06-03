#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>
#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "includes.h"
#include "eloop.h"
#include "client_db.h"
#include "mapd_i.h"
#include "db.h"
#ifdef SUPPORT_MULTI_AP
#include "topologySrv.h"
#endif
#include "ctrl_iface.h"
#include "wapp_if.h"
#include "chan_mon.h"
#include "client_mon.h"
#ifdef SUPPORT_MULTI_AP
#include "1905_if.h"
#include "apSelection.h"
#include "1905_map_interface.h"
#endif
#include "steer_action.h"
#ifdef CORE_DUMP_ENABLED
#include <sys/resource.h>
#endif
#ifdef BACKTRACK_SUPPORT
#include <execinfo.h>
#endif
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "mapd_interface_ctrl.h"
#include "wapp_usr_intf_ctrl.h"
#ifdef SUPPORT_MULTI_AP
#include "ch_planning.h"
#include "network_optimization.h"
#endif
#include "interface.h"
#include "ap_roam_algo.h"
#ifdef SUPPORT_MULTI_AP
#include "tlv_parsor.h"
#include "mapfilter_if.h"
#ifdef CENT_STR
#include "ap_cent_str.h"
#include "ap_est.h"

#endif
#endif
#define MAPD_HOUSEKEEPING_INTERVAL 500000 /* in usecs */
#define MAPD_ONE_SEC_CNT (1000000 / MAPD_HOUSEKEEPING_INTERVAL)
#define MAPD_ONE_MIN_CNT (60000000 / MAPD_HOUSEKEEPING_INTERVAL)
#define MAPD_ONE_HOUR_CNT (3600000000UL / MAPD_HOUSEKEEPING_INTERVAL)

extern u8 ZERO_MAC_ADDR[ETH_ALEN];
u8 MTK_OUI[OUI_LEN] = {0x00, 0x0C, 0xE7};
//#define file_path_wts "/etc/map/wts_bss_info_config"
#ifdef SUPPORT_MULTI_AP
void mapd_steering_complete(struct mapd_global *global);
/* OK */
int SendApMetricRsp(struct mapd_radio_info *ra_info,struct own_1905_device *ctx)
{
	struct metric_policy_db *policy_db = NULL;
	struct metrics_policy *mpolicy = &ctx->map_policy.mpolicy;
	uint8_t CuThreshInPolicy = 0,fAPMetricRsp = 0, ch_util = 0;
//ra_info->ch_util is in % , so convert it into a scale of 255 , to check with policy
	ch_util = ((ra_info->ch_util) * 255) / 100;
	SLIST_FOREACH(policy_db, &(mpolicy->policy_head), policy_entry){
		if(os_memcmp(policy_db->identifier,ra_info->identifier,ETH_ALEN)){
			CuThreshInPolicy = policy_db->ch_util_thres;
			break;
		}
	}
	if(CuThreshInPolicy == 0){
		debug("CuThreshInPolicy is 0 return");
		return 0;
	}
	if((ch_util > CuThreshInPolicy) && (ra_info->CuThCrossSend == 0)){
		debug("CU thresh exceed");
		fAPMetricRsp = 1;
		ra_info->CuThCrossSend = 1;
	} else if((ch_util < CuThreshInPolicy) && (ra_info->CuThCrossSend == 1)){
		debug("CU thresh receed");
		fAPMetricRsp = 1;
		ra_info->CuThCrossSend = 0;
	}
	return fAPMetricRsp;
}
signed short append_ap_metrics_rsp_onlyAPmetricTLV_message(struct own_1905_device *ctx,
					       struct ap_metrics_info_lib **info, int *ap_metrics_info_cnt)
{
	struct bss_info_db *mrsp = NULL;
	unsigned char *tlv_temp_buf = NULL;
	struct bss_db *bss = NULL;
	unsigned char *ptr = NULL;

	SLIST_FOREACH(bss, &(ctx->metric_entry.metrics_query_head), bss_entry) {
		mrsp = topo_srv_get_bss_by_bssid(ctx, topo_srv_get_1905_device(ctx, NULL), bss->bssid);
		if (!mrsp) {
			err("failed to find AP with bssid %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(bss->bssid));
			continue;
		}
		*ap_metrics_info_cnt = *ap_metrics_info_cnt + 1;
	}
	debug("ap metircs info cnt=%d", *ap_metrics_info_cnt);

	if (*ap_metrics_info_cnt) {
		tlv_temp_buf = os_zalloc(sizeof(struct ap_metrics_info_lib) * (*ap_metrics_info_cnt));
		if(!tlv_temp_buf){
			err("alloc fail");
			return -1;
		}
	}

	ptr = tlv_temp_buf;
	SLIST_FOREACH(bss, &(ctx->metric_entry.metrics_query_head), bss_entry) {
		mrsp = topo_srv_get_bss_by_bssid(ctx, topo_srv_get_1905_device(ctx, NULL), bss->bssid);
		if (!mrsp) {
			err("failed to find AP with bssid %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(bss->bssid));
			continue;
		}
		append_ap_metrics_tlv(tlv_temp_buf, mrsp);
		tlv_temp_buf += sizeof(struct ap_metrics_info_lib);
	}
	if (*ap_metrics_info_cnt)
		*info = (struct ap_metrics_info_lib *)ptr;

	return 0;
}
int send_ap_metric_rsp_only_APmetricTLV(struct own_1905_device *own_dev)
{
	struct ap_metrics_info_lib *ap_metrics = NULL;
	int ap_metrics_info_cnt = 0;
	struct mapd_global *mapd_ctx = (struct mapd_global *)own_dev->back_ptr;
	append_ap_metrics_rsp_onlyAPmetricTLV_message(own_dev, &ap_metrics, &ap_metrics_info_cnt);
#ifdef MAP_R2  // TODO: Raghav : fix this code.
	map_1905_Set_Ap_Metric_Rsp_Info(mapd_ctx->_1905_ctrl, ap_metrics, ap_metrics_info_cnt,
					NULL, 0, NULL, 0,NULL, 0, NULL, 0, NULL,0, NULL, 0);
#else
	map_1905_Set_Ap_Metric_Rsp_Info(mapd_ctx->_1905_ctrl, ap_metrics, ap_metrics_info_cnt,
					NULL, 0, NULL, 0);
#endif
	if (ap_metrics)
		free(ap_metrics);
	return 0;
}
#endif
void mapd_get_all_ap_metrics_info(struct mapd_global *global)
{
	uint8_t i = 0;
	struct mapd_radio_info *ra_info = NULL;
	struct mapd_bss *bss = NULL;

	mapd_printf(MSG_MSGDUMP, "%s: ENTERED", __func__);
	/* Iterate over all BSS on this device */
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;
		bss = NULL;
		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
			if (wlanif_get_ap_metrics_info(global, bss->bssid) != 0) {
				mapd_printf(MSG_ERROR, "%s: FAILED for bss=" MACSTR,
								__func__, MAC2STR(bss->bssid));
			}
		}
#ifdef SUPPORT_MULTI_AP
		if(SendApMetricRsp(ra_info,&global->dev)){
			send_ap_metric_rsp_only_APmetricTLV(&global->dev);
		}

		if (global->dev.dual_bh_en && global->dev.load_balance_en) {
			unsigned char band;
			if (ra_info->channel > 0 && ra_info->channel <= 14)
				band = _24G;
			else if (ra_info->channel >= 36 && ra_info->channel < 100)
				band = _5GL;
			else
				band = _5GH;
			mapfilter_set_channel_utilization(ra_info->identifier, band, ra_info->ch_util);
		}
#endif
	}
}

/* Clear all leftover blacklists */
void mapd_flush_all_bl(struct mapd_global *global)
{
	uint8_t i = 0;
	struct mapd_radio_info *ra_info = NULL;
	struct mapd_bss *bss = NULL;

	mapd_printf(MSG_MSGDUMP, "%s: ENTERED", __func__);
	/* Iterate over all BSS on this device */
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;
		bss = NULL;
		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
			if (wlanif_flush_bl_for_bss(global, bss->bssid) != 0) {
				mapd_printf(MSG_ERROR, "%s: FAILED for bss=" MACSTR,
								__func__, MAC2STR(bss->bssid));
			}
		}
	}
}

void mapd_reset_first_seen_for_all_dev(
	struct own_1905_device *ctx, u8 value)
{
	struct _1905_map_device *dev = NULL;
	SLIST_FOREACH(dev, &(ctx->_1905_dev_head), next_1905_device) {
		if(value == 1) {
			os_get_time(&dev->first_seen);
		} else if (value == 0) {
			dev->first_seen.sec = 0;
			dev->first_seen.usec = 0;
		}
	}
}
/* Periodic cleanup tasks */
static void mapd_periodic(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = eloop_ctx;
#ifdef SUPPORT_MULTI_AP
	struct own_1905_device *ctx = &global->dev;
#endif
	static uint32_t periodic_cnt = 0;
	struct _1905_map_device *map_dev;
	periodic_cnt++;
	eloop_register_timeout(0, MAPD_HOUSEKEEPING_INTERVAL,
			mapd_periodic, global, NULL);

#ifdef SUPPORT_MULTI_AP
	if (ctx->config_status != DEVICE_CONFIGURED) {
		info("skip mapd periodic for unconf device");
		return;
	}
#endif
	debug("Do Periodic tasks here");

#ifdef CENT_STR
	if((global->dev.cent_str_en && global->dev.device_role == DEVICE_ROLE_CONTROLLER))
		client_mon_chk_post_assoc_cent_str(global);
#endif


	/* Every one second */
	if (((periodic_cnt % MAPD_ONE_SEC_CNT)) == 0) {
		/* Get Stats for Steering */
		client_mon_get_assoc_stats(global);
		/* Get AP Metrics Info for channel load information */
		mapd_get_all_ap_metrics_info(global);
		/* Do Periodic Client DB Mainenance */
		client_mon_cli_db_maintenance(global);
		/* Post Assoc Steering */
#ifdef CENT_STR
		if((!global->dev.cent_str_en)
			|| (global->dev.cent_str_en && global->dev.device_role == DEVICE_ROLE_AGENT))
#endif
		client_mon_chk_post_assoc_str(global);


#ifdef SUPPORT_MULTI_AP
		/* Steer Complete handling */
#ifdef CENT_STR
		if (global->dev.cent_str_en) {
			info("steer complete replaced");
		} else
#endif
		mapd_steering_complete(global);
#endif
	}
#ifdef SUPPORT_MULTI_AP
	if (!global->params.Certification) {
		if(!global->dev.cent_str_en)
		chan_mon_rr_trigger_handler(global);

		if (((periodic_cnt % MAPD_ONE_HOUR_CNT)) == 0) {
			mapd_client_db_flush(global, 0);
			periodic_cnt = 0;
		}
		if (periodic_cnt % MAPD_ONE_MIN_CNT == 0) {
			if (global->dev.enhanced_logging)
				system("logrotate /etc/logrotate.conf");
		}

		if(global->dev.cent_str_en)
			steer_handle_chan_plan_net_opt_trigger(global);

		if (global->dev.ch_planning.ch_planning_enabled)
			mapd_perform_channel_planning(&global->dev);

		if ((((periodic_cnt % (MAPD_ONE_SEC_CNT))) == 0) &&
			global->dev.network_optimization.network_optimization_enabled){
			if (global->dev.device_role == DEVICE_ROLE_CONTROLLER ) {
					network_optimization_state_handler(global);
				}
			}

		if (periodic_cnt % MAPD_ONE_SEC_CNT == 0) {
			if (global->dev.device_role == DEVICE_ROLE_AGENT) {
				send_cac_start(global);
			}
		}

		if((((periodic_cnt % (MAPD_ONE_SEC_CNT))) == 0) &&
			ctx->device_role == DEVICE_ROLE_CONTROLLER &&
			ctx->ch_planning.need_restart_ch_plan) {
#ifdef MAP_R2
			if ((ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_IDLE) &&
			 (ctx->ch_planning.ch_planning_state == CHANNEL_PLANNING_IDLE))
#else
			if (ctx->ch_planning.ch_planning_state == CHANNEL_PLANNING_IDLE)
#endif
			 {
			 	err("due to new dev connect restart ch planning");
				mapd_reset_first_seen_for_all_dev(ctx, 1);
				mapd_restart_channel_plannig(ctx->back_ptr);
				ctx->ch_planning.need_restart_ch_plan = 0;
			}
		}
		if ((((periodic_cnt % (5*MAPD_ONE_SEC_CNT))) == 0) &&
			global->dev.network_optimization.network_optimization_enabled &&
			global->dev.network_optimization.prefer_5G_bh) {

			if (global->dev.device_role == DEVICE_ROLE_CONTROLLER ) {
				struct radio_info_db *radio = NULL;
				SLIST_FOREACH(map_dev, &(ctx->_1905_dev_head), next_1905_device) {
				do {
					radio = topo_srv_get_next_radio(map_dev, radio);
					if (radio) {
						if ((radio->channel[0] <= 14) && (radio->uplink_bh_present == TRUE)){
							if ((global->dev.network_optimization.network_opt_state == NETOPT_STATE_IDLE)
								&& (ctx->nw_opt_triggered_5G == FALSE)
									&& (ctx->nw_opt_triggered_5G_in_process == FALSE)
									&& (global->dev.network_optimization.prefer_5G_bh_try_cnt_curr <
										global->dev.network_optimization.prefer_5G_bh_try_cnt_user)) {
								eloop_register_timeout(0, 0, retrigger_ch_planning, global, NULL);

								eloop_cancel_timeout(trigger_5G_net_opt, ctx, NULL);
//								network_opt_reset(ctx->back_ptr);
								eloop_register_timeout(120, 0, trigger_5G_net_opt, ctx, NULL);
								ctx->nw_opt_triggered_5G = TRUE;
								global->dev.network_optimization.prefer_5G_bh_try_cnt_curr++;
							}
							break;
						}

					}
				} while (radio);
				}
			}
		}
	}
#endif
}

int map_get_info_from_wapp(struct own_1905_device *ctx,
        unsigned short msgtype, unsigned short waitmsgtype, unsigned char *bssid,
        unsigned char *stamac, void *data, int datalen)
{
#ifdef SUPPORT_MULTI_AP
	mapd_printf(MSG_INFO, "%s: 1905/toposrv request", __func__);
#else
	mapd_printf(MSG_INFO, "%s:  request", __func__);
#endif
	if (waitmsgtype)
		return wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, msgtype,
					waitmsgtype, bssid, stamac, data, datalen, 0, 1, 0);
	else
		return wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, msgtype,
					waitmsgtype, bssid, stamac, data, datalen, 0, 0, 0);
}
#ifdef SUPPORT_MULTI_AP
void map_start_auto_role_detection(void *eloop_ctx, void *timeout_ctx);
void map_1905_poll_timeout(void *eloop_ctx, void *timeout_ctx);

void map_register_network_poll_timer(struct own_1905_device *dev, unsigned int sec)
{
	eloop_register_timeout(sec,0, map_start_auto_role_detection, dev->back_ptr, dev);
}

void map_register_poll_timeout(struct own_1905_device *dev)
{
	eloop_register_timeout(30, 0, map_1905_poll_timeout, dev->back_ptr, dev);
}

void map_start_auto_role_detection(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *dev = &global->dev;
#ifdef AUTOROLE_NEGO
	struct bh_link_entry *bh_entry;
	int status=0;
#endif
	if ((!dev) || (!global)) {
		err("%s dev|global ctx is null dev=%p global=%p", __func__, dev, global);
		return;
	}
	if (dev->device_role != DEVICE_ROLE_UNCONFIGURED) {
		mapfilter_set_drop_specific_dest_ip_status(0);
		eloop_cancel_timeout(map_1905_poll_timeout, global, NULL);
		return;
	}
#ifdef AUTOROLE_NEGO
	SLIST_FOREACH(bh_entry, &(dev->bh_link_head), next_bh_link) {
		status=_1905_poll_devices_in_network(dev, bh_entry);
		if (status < 0) {
			if(dev->ThirdPartyConnection == 1) {
				wlanif_issue_wapp_command(global, WAPP_NEGOTIATE_ROLE,
					0, NULL,NULL, &dev->device_role, sizeof(int),0, 0, 0);
			}
		}
	}
#else
	_1905_poll_devices_in_network(dev, NULL);
#endif
	/* Keep polling every 5 sec */
	map_register_network_poll_timer(dev, 5);
}

/*DHCP_CTL*/
void map_register_dhcp_timer(struct own_1905_device *dev)
{
	eloop_cancel_timeout(map_dhcp_poll_timeout, dev->back_ptr, dev);
	eloop_register_timeout(180, 0,
		map_dhcp_poll_timeout, dev->back_ptr, dev);
}

void map_dhcp_poll_timeout(void *eloop_ctx, void *timeout_ctx) {
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *dev = &global->dev;
	int ret = -1;
	if (dev->dhcp_req.dhcp_client_enable == 1) {
		u8 retry_cnt = 3;
		do {
			ret = wlanif_issue_wapp_command((struct mapd_global *)global->dev.back_ptr, WAPP_USER_GET_BRIDGE_IP_REQUEST,
                       WAPP_BRIDGE_IP, NULL, NULL, NULL, 0, 0, 1, 0);
			mapd_printf(MSG_OFF,"retry_cnt:%d, get ip : %s", retry_cnt,global->dev.ipbuf);
			retry_cnt --;
		} while (!(0 == ret || 0 == retry_cnt));
		if (ret != 0 || strlen(global->dev.ipbuf) <= 0) {
			mapd_printf(MSG_ERROR,"get ip fail!\n");
			if (!(1 == dev->dhcp_req.dhcp_server_enable && 0 == dev->dhcp_req.dhcp_client_enable)) {
				mapd_printf(MSG_INFO,"set br for default ip!\n");
				wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_USER_SET_BRIDGE_DEFAULT_IP_REQUEST,
					0, NULL, NULL, NULL, 0, 0, 0, 0);
			}
		}
	}
	mapd_printf(MSG_INFO,"cancel timer!\n");
	eloop_cancel_timeout(map_dhcp_poll_timeout, global, dev);
}


Boolean is_interface_up(char *iface) {
	struct ifreq ifr;
	int sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_IP);
	memset(&ifr, 0, sizeof(ifr));
	os_memcpy(ifr.ifr_name, iface, IFNAMSIZ);
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
		perror("SIOCGIFFLAGS");
	}
	close(sock);
	return !!(ifr.ifr_flags & IFF_UP);
}

void map_update_device_role_as_controller(struct own_1905_device *dev)
{
	struct _1905_map_device *_1905_device = topo_srv_get_next_1905_device(dev, NULL);
	dev->device_role = DEVICE_ROLE_CONTROLLER;
	_1905_device->device_role = DEVICE_ROLE_CONTROLLER;
	chan_mon_update_rr_ctrl_trigger_info(dev->back_ptr);
	_1905_device->root_distance = 0;
	wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_USER_MAP_CONTROLLER_FOUND,
					0, NULL, NULL, _1905_device->_1905_info.al_mac_addr, ETH_ALEN, 0, 0, 0);
#ifdef MAP_R2
	wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_UPDATE_MAP_DEVICE_ROLE,
		0, NULL, NULL,(void *)&_1905_device->device_role , sizeof(int), 0, 0, 0);
#else
	if(dev->ThirdPartyConnection)
		wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_UPDATE_MAP_DEVICE_ROLE,
			0, NULL, NULL,(void *)&_1905_device->device_role , sizeof(int), 0, 0, 0);
#endif
	topo_srv_start_combined_infra_metrics_srv(dev);
}

void mapd_start_wired_iface_monitor(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = eloop_ctx;
	struct own_1905_device *dev = &global->dev;
	Boolean is_iface_up = FALSE;
	Boolean connectivity = FALSE;

	if ((dev->device_role != DEVICE_ROLE_UNCONFIGURED)) {
		mapfilter_set_drop_specific_dest_ip_status(0);
		return;
	}
#ifdef AUTOROLE_NEGO
	if(dev->ThirdPartyConnection == 0) {
#endif // AUTOROLE_NEGO
		dev->auto_role_detect = 1;
		is_iface_up = is_interface_up((char *)dev->wan_iface);
		debug("wan inteface is %d", is_iface_up);

		if (is_iface_up ==  FALSE) {
			is_iface_up = is_interface_up((char *)dev->lan_iface);
			debug("lan inteface is %d", is_iface_up);
		}

		if (is_iface_up) {
			/*ping with special size 88, mapfilter will drop ping package with len = 88 + 28(header len) while auto role selection*/
			if (system("ping -c 1 -s 88 8.8.8.8  > /dev/null 2>&1") == 0)
				connectivity = 1;
			else if(system("ping -c 1 -s 88 208.67.222.222 > /dev/null 2>&1") == 0)
				connectivity = 1;
			else
				connectivity = 0;
		}

		err("connectivity is %d", connectivity);
		if (connectivity)
			map_start_auto_role_detection_srv(dev);
		else
			eloop_register_timeout(5, 0, mapd_start_wired_iface_monitor, global, NULL);
#ifdef AUTOROLE_NEGO
	}else {
			if(dev->ConnectThirdPartyVend) {
			struct bh_link_info info;
			info.type = 0;
			memcpy(info.ifname, dev->lan_iface, IFNAMSIZ);
			map_start_auto_role_detection_srv(dev);
		} else
			eloop_register_timeout(5, 0, mapd_start_wired_iface_monitor, global, NULL);
	}
#endif // AUTOROLE_NEGO
}

int map_start_auto_role_detection_srv(struct own_1905_device *dev)
{
	struct _1905_map_device *_1905_device = topo_srv_get_next_1905_device(dev, NULL);
	struct agent_list *a = NULL;

	if (_1905_device->supported_services == 0) {
		warn("device is confiured as controller, skipping auto role detection\n");
		map_update_device_role_as_controller(dev);
		return 0;
	}
	dev->auto_role_detect = 2;
	if (!SLIST_EMPTY(&dev->a_list)) {
		err("earch agent_list not null, clear");
		SLIST_FOREACH(a, &dev->a_list, next_agent) {
			SLIST_REMOVE(&dev->a_list, a, agent_list, next_agent);
			os_free(a);
			if (SLIST_EMPTY(&dev->a_list))
				break;
		}
		SLIST_INIT(&dev->a_list);
	}
	err("Starting auto role detection service\n");
#ifdef AUTOROLE_NEGO
	dev->own_new_DevRole = DEVICE_ROLE_UNCONFIGURED;
#endif // AUTOROLE_NEGO
	map_register_poll_timeout(dev);
	map_register_network_poll_timer(dev, 0);
	return 0;
}

void map_1905_poll_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *dev = &global->dev;
	struct _1905_map_device *_1905_device = topo_srv_get_next_1905_device(dev, NULL);
	struct agent_list *a = NULL;
	int i = 0, role = -1;
	int f_large_bit = -1, f_small_bit = -1;

	if (!_1905_device) {
		err("own 1905 device not found\n");
		return;
	}
#ifdef AUTOROLE_NEGO
	if(global->dev.ThirdPartyConnection) {
		if(global->dev.own_new_DevRole == DEVICE_ROLE_AGENT) {
			err("Agent made, disconnect\n");
			map_get_info_from_wapp(dev, WAPP_USER_MAP_CONTROLLER_FOUND, 0, NULL, NULL, NULL, 0);
			map_1905_controller_found(dev);
			wlanif_issue_wapp_command(global, WAPP_USER_ISSUE_APCLI_DISCONNECT,
					0, NULL, NULL, NULL, 0, 0, 0, 0);
		}else {
			err("controller made\n");
			topo_srv_update_bss_role_for_controller(dev);
			map_1905_Set_Role(global->_1905_ctrl, MAP_CONTROLLER);
			map_1905_Set_Read_Bss_Conf_and_Renew(global->_1905_ctrl, 1);
			map_update_device_role_as_controller(dev);
			mapd_update_controller_steer_policy(global);
		}
	}else {
#endif //AUTOROLE_NEGO
		if (dev->device_role == DEVICE_ROLE_UNCONFIGURED && dev->auto_role_detect == 2) {
			SLIST_FOREACH(a, &dev->a_list, next_agent) {
				f_large_bit = -1, f_small_bit = -1;
				for (i = 0; i < ETH_ALEN; i++) {
					err("almac[%d] %02x-%02x",i, dev->al_mac[i], a->almac[i])
					if (dev->al_mac[i] > a->almac[i]) {
						if (f_large_bit == -1) {
							f_large_bit = i;
						}
					} else if (dev->al_mac[i] < a->almac[i]) {
						if (f_small_bit == -1) {
							f_small_bit = i;
						}
					}
				}
				err("f_small_bit-%d f_large_bit-%d", f_small_bit, f_large_bit);
				if ((f_small_bit != -1 && f_large_bit != -1 && f_small_bit < f_large_bit) ||
					(f_small_bit != -1 && f_large_bit == -1)) {
					role = MAP_AGENT;
					break;
				}
			}
			SLIST_FOREACH(a, &dev->a_list, next_agent) {
				SLIST_REMOVE(&dev->a_list, a, agent_list, next_agent);
				os_free(a);
				if(SLIST_EMPTY(&dev->a_list))
					break;
			}
			SLIST_INIT(&dev->a_list);
			dev->auto_role_detect = 0;

			if (role == -1)
				role = MAP_CONTROLLER;

			if (role == MAP_CONTROLLER) {
				err("updated device role as controller\n");
				topo_srv_update_bss_role_for_controller(dev);
				map_1905_Set_Role(global->_1905_ctrl, MAP_CONTROLLER);
				map_1905_Set_Read_Bss_Conf_and_Renew(global->_1905_ctrl, 1);
				map_update_device_role_as_controller(dev);
				mapd_update_controller_steer_policy(global);
			} else {
				err("updated device role as agent\n");
				map_1905_controller_found(dev);
			}
			mapfilter_set_drop_specific_dest_ip_status(0);
		}
#ifdef AUTOROLE_NEGO
	}
#endif //AUTOROLE_NEGO
	eloop_cancel_timeout(map_1905_poll_timeout, global, dev);
	eloop_cancel_timeout(map_start_auto_role_detection, global, dev);
}


int map_1905_controller_found(struct own_1905_device *dev)
{
	struct _1905_map_device *_1905_device = topo_srv_get_next_1905_device(dev, NULL);

	dev->device_role = DEVICE_ROLE_AGENT;
	_1905_device->device_role = DEVICE_ROLE_AGENT;
	warn("updated device role as agent");
#ifdef MAP_R2
	wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_UPDATE_MAP_DEVICE_ROLE,
		0, NULL, NULL,(void *)&_1905_device->device_role , sizeof(int), 0, 0, 0);
#else
	if(dev->ThirdPartyConnection){
		wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_UPDATE_MAP_DEVICE_ROLE,
			0, NULL, NULL,(void *)&_1905_device->device_role , sizeof(int), 0, 0, 0);
	}
#endif
#ifdef ACL_CTRL
	eloop_register_timeout(5, 0, map_sync_acl_info, dev->back_ptr, dev);
#endif
	return 0;
}
#endif

void mapd_deinit(struct mapd_global *global)
{
    if (global == NULL)
        return;

	/* Currently, this is causing a hang at reboot, hence remove*/
	//mapd_client_db_flush(global, 0);
#ifdef SUPPORT_MULTI_AP
    _1905_close_connection(global);
#endif
    eloop_cancel_timeout(mapd_periodic, global, NULL);

    eloop_destroy();
#ifdef SUPPORT_MULTI_AP
    topo_srv_deinit_topo_srv(&global->dev);
#endif
    if (global->params.pid_file) {
        os_daemonize_terminate(global->params.pid_file);
        os_free(global->params.pid_file);
    }
    os_free(global->params.ctrl_interface);
    os_free(global->params.ctrl_interface_group);

    os_free(global);
    mapd_debug_close_syslog();
    mapd_debug_close_file();
    mapd_debug_close_linux_tracing();
    wapp_close_connection();
}

void mapd_init_radio_interface(struct mapd_radio_info *radio_info) {

	radio_info->radio_idx = (uint8_t)-1;
	radio_info->bss_bitmap = 0;

}
void mapd_init_steer_params(struct own_1905_device *dev, const char *confname)
{
	FILE *file = NULL;
	char buf[256], *pos, *token;
	char tmpbuf[256], tmp1buf[256], tmp2buf[256];
	int line = 0;
#ifdef SUPPORT_MULTI_AP
 	int i = 0;
#endif
	/* Defualt device configuration */
	dev->cli_steer_params.CUOverloadTh_2G = 80;
	dev->cli_steer_params.CUOverloadTh_5G_L = 70;
	dev->cli_steer_params.CUOverloadTh_5G_H = 70;
	dev->cli_steer_params.CUSafetyTh_2G = 60;
	dev->cli_steer_params.CUSafetyTh_5G_L = 60;
	dev->cli_steer_params.CUSafetyh_5G_H = 60;
	dev->cli_steer_params.MinRSSIOverload = 20;
	dev->cli_steer_params.RSSISteeringEdge_DG = 20;
	dev->cli_steer_params.RSSISteeringEdge_UG = 20;
	dev->cli_steer_params.MCSCrossingThreshold_DG = 6000;
	dev->cli_steer_params.MCSCrossingThreshold_UG = 50000;
	dev->cli_steer_params.RSSICrossingThreshold_DG = 15;
	dev->cli_steer_params.RSSICrossingThreshold_UG = 30;
	dev->cli_steer_params.phy_scal_factx100 = 70;
	dev->cli_steer_params.RSSIAgeLim = 5;
	dev->cli_steer_params.RSSIAgeLim_preAssoc = 10;
	dev->cli_steer_params.RSSIMeasureSamples = 5;
	dev->cli_steer_params.ForceStrBlockTime = 600;
	dev->cli_steer_params.BTMStrBlockTime = 300;
	dev->cli_steer_params.ForceStrForbidTime = 300;
	dev->cli_steer_params.BTMStrForbidTime = 30;
	dev->cli_steer_params.StrForbidTimeJoin = 10;
	dev->cli_steer_params.MaxClientOverloaded = 100;
	dev->cli_steer_params.ActivityThreshold = 3000;
	dev->cli_steer_params.StartInActive = 1;
	dev->cli_steer_params.prohibitTime11K = 30;
	dev->cli_steer_params.disable_pre_assoc_strng = 0;
	dev->cli_steer_params.disable_post_assoc_strng = 0;
	dev->cli_steer_params.disable_offloading = 0;
#ifdef SUPPORT_MULTI_AP
	dev->cli_steer_params.disable_nolmultiap = 0;
#ifdef CENT_STR
	dev->cent_str_en = 0;
	dev->cli_steer_params.cent_str_max_steer_cand = MAX_CENT_STEER_CAND;
	dev->cli_steer_params.cent_str_max_bs_fail = MAX_BS_FAIL;
	dev->cli_steer_params.cent_str_max_ol_steer_cand = MAX_OL_STEER_CAND;
	dev->cli_steer_params.cent_str_max_ug_steer_cand = MAX_UG_STEER_CAND;
	dev->cli_steer_params.cent_str_cu_mon_time = MAX_CU_MONITOR_TIME;
	dev->cli_steer_params.cent_str_cu_mon_prohibit_time = MAX_CU_MONITOR_PROHIBIT_TIME;
#endif

#endif
	dev->cli_steer_params.disable_active_ug = 0;
	dev->cli_steer_params.disable_active_dg = 0;
	dev->cli_steer_params.disable_idle_ug = 0;
	dev->cli_steer_params.disable_idle_dg = 0;
	dev->cli_steer_params.ForcedRssiUpdate = 0;
	dev->cli_steer_params.GlobalProhibitTime = 0;
	dev->cli_steer_params.idle_count_th = 1;
#ifdef SUPPORT_MULTI_AP
	// Multi-AP
	dev->cli_steer_params.LowRSSIAPSteerEdge_root = 20;
	dev->cli_steer_params.LowRSSIAPSteerEdge_RE = 40;
	dev->cli_steer_params.MinRssiIncTh_Root = 5;
	dev->cli_steer_params.MinRssiIncTh_RE = 10;
	dev->cli_steer_params.MinRssiIncTh_Peer = 10;
#endif
	dev->cli_steer_params.CUAvgPeriod = 60;
	dev->cli_steer_params.BTMStrTimeout = 6;
	dev->cli_steer_params.ForceStrTimeout = 15;
#ifdef SUPPORT_MULTI_AP
	dev->cli_steer_params.single_steer = 1;
#endif
	dev->cli_steer_params.PHYBasedSelection = 0;
	dev->cli_steer_params.MinSteerRetryTime= MIN_STEER_RETRY_TIME;
	dev->cli_steer_params.MaxSteerRetryTime= MAX_STEER_RETRY_TIME;
	dev->cli_steer_params.SteerRetryStep= STEER_RETRY_STEP_CNT;
#ifdef SUPPORT_MULTI_AP
	dev->cli_steer_params.force_roam_rssi_th = -87;
#endif
	dev->cli_steer_params.reset_btm_csbc_at_join = 0;
	dev->ap_metric_rep_intv = 60;
#ifdef SUPPORT_MULTI_AP
	for(i =0 ;i < BAND_5GH ;i++){
		dev->controller_context.ap_metric_policy.policy_params[i].RadioBand = (i+1);
		dev->controller_context.ap_metric_policy.policy_params[i].MetricPolicyChUtilThres = 0;
		dev->controller_context.ap_metric_policy.policy_params[i].MetricPolicyTrafficInclusion = 1;
		dev->controller_context.ap_metric_policy.policy_params[i].MetricPolicyMetricsInclusion = 1;
		dev->controller_context.ap_metric_policy.policy_params[i].MetricPolicyRcpi = 0;
		dev->controller_context.ap_metric_policy.policy_params[i].MetricPolicyHys = 0;
	}
#endif
	if (confname)
		file = fopen(confname, "r");

	if (!file) {
		mapd_printf(MSG_ERROR, ("open configuration fail,"
					"Using default configuration \n"));
	} else {
		mapd_printf(MSG_INFO, "Open Conf file Succeess\n");
		os_memset(buf, 0, 256);
		os_memset(tmpbuf, 0, 256);
		os_memset(tmp1buf, 0, 256);
		os_memset(tmp2buf, 0, 256);

		while (mapd_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
			os_snprintf(tmpbuf, sizeof(tmpbuf), "%s", (const char *)pos);
			token = strtok(pos, "=");
			if (token != NULL) {
				if (os_strcmp((const char *)token, "CUOverloadTh_2G") == 0) {
					token = strtok(NULL, "");
					if(token){
						dev->cli_steer_params.CUOverloadTh_2G = atoi((const char *)token);
						mapd_printf(MSG_INFO, "CUOverloadTh_2G=%d\n"
								,dev->cli_steer_params.CUOverloadTh_2G);
					}
				} else if(os_strcmp((const char *)token, "CUOverloadTh_5G_L") == 0){
					token = strtok(NULL, "");
					if(token){
						dev->cli_steer_params.CUOverloadTh_5G_L = atoi((const char *)token);
						mapd_printf(MSG_INFO, "CUOverloadTh_5G_L=%d\n",
								dev->cli_steer_params.CUOverloadTh_5G_L);
					}
				} else if(os_strcmp((const char *)token, "CUOverloadTh_5G_H") == 0){
					token = strtok(NULL, "");
					if(token){
						dev->cli_steer_params.CUOverloadTh_5G_H = atoi((const char *)token);
						mapd_printf(MSG_INFO, "CUOverloadTh_5G_H =%d\n",
								dev->cli_steer_params.CUOverloadTh_5G_H);
					}
				} else if(os_strcmp((const char *)token, "CUSafetyTh_2G") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.CUSafetyTh_2G = atoi((const char *)token);
					mapd_printf(MSG_INFO, "CUSafetyTh_2G=%d\n",
							dev->cli_steer_params.CUSafetyTh_2G);
				} else if(os_strcmp((const char *)token, "CUSafetyTh_5G_L") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.CUSafetyTh_5G_L = atoi((const char *)token);
					mapd_printf(MSG_INFO, " CUSafetyTh_5G_L=%d\n",
							dev->cli_steer_params.CUSafetyTh_5G_L);
				} else if(os_strcmp((const char *)token, "CUSafetyTh_5G_H") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.CUSafetyh_5G_H = atoi((const char *)token);
					mapd_printf(MSG_INFO, " CUSafetyh_5G_H=%d\n",
							dev->cli_steer_params.CUSafetyh_5G_H);
				} else if(os_strcmp((const char *)token, "MinRSSIOverload") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MinRSSIOverload = atoi((const char *)token);
					mapd_printf(MSG_INFO, " MinRSSIOverload=%d\n",
							dev->cli_steer_params.MinRSSIOverload);
				} else if(os_strcmp((const char *)token, "RSSISteeringEdge_DG") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.RSSISteeringEdge_DG = atoi((const char *)token);
					mapd_printf(MSG_INFO, " RSSISteeringEdge_DG=%d\n",
							dev->cli_steer_params.RSSISteeringEdge_DG);
				} else if(os_strcmp((const char *)token, "RSSISteeringEdge_UG") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.RSSISteeringEdge_UG = atoi((const char *)token);
					mapd_printf(MSG_INFO, " RSSISteeringEdge_UG=%d\n",
							dev->cli_steer_params.RSSISteeringEdge_UG);
#ifdef SUPPORT_MULTI_AP
				} else if(os_strcmp((const char *)token, "force_roam_rssi_th") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.force_roam_rssi_th = atoi((const char *)token);
					mapd_printf(MSG_INFO, " force_roam_rssi_th=%d\n",
							dev->cli_steer_params.force_roam_rssi_th);
#endif
				} else if(os_strcmp(token, "MCSCrossingThreshold_DG") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MCSCrossingThreshold_DG = atoi(token);
					mapd_printf(MSG_INFO, " MCSCrossingThreshold_DG=%d\n",
							dev->cli_steer_params.MCSCrossingThreshold_DG);
				} else if(os_strcmp(token, "MCSCrossingThreshold_UG") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MCSCrossingThreshold_UG = atoi(token);
					mapd_printf(MSG_INFO, " MCSCrossingThreshold_UG=%d\n",
							dev->cli_steer_params.MCSCrossingThreshold_UG);
				} else if(os_strcmp(token, "RSSICrossingThreshold_DG") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.RSSICrossingThreshold_DG = atoi(token);
					mapd_printf(MSG_INFO, " RSSICrossingThreshold_DG=%d\n",
							dev->cli_steer_params.RSSICrossingThreshold_DG);
				} else if(os_strcmp(token, "RSSICrossingThreshold_UG") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.RSSICrossingThreshold_UG = atoi(token);
					mapd_printf(MSG_INFO, " RSSICrossingThreshold_UG=%d\n",
							dev->cli_steer_params.RSSICrossingThreshold_UG);
				} else if(os_strcmp(token, "phy_scal_factx100") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.phy_scal_factx100 = atoi((const char *)token);
					mapd_printf(MSG_INFO, " phy_scal_factx100=%d\n",
							dev->cli_steer_params.phy_scal_factx100);
				} else if(os_strcmp(token, "RSSIAgeLim") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.RSSIAgeLim = atoi(token);
					mapd_printf(MSG_INFO, " RSSIAgeLim=%d\n",
							dev->cli_steer_params.RSSIAgeLim);
				} else if(os_strcmp(token, "RSSIAgeLim_preAssoc") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.RSSIAgeLim_preAssoc = atoi(token);
					mapd_printf(MSG_INFO, "RSSIAgeLim_preAssoc =%d\n",
							dev->cli_steer_params.RSSIAgeLim_preAssoc);
				} else if(os_strcmp(token, "RSSIMeasureSamples") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.RSSIMeasureSamples = atoi((const char *)token);
					mapd_printf(MSG_INFO, " RSSIMeasureSamples=%d\n",
							dev->cli_steer_params.RSSIMeasureSamples);
				} else if(os_strcmp(token, "ForceStrBlockTime") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.ForceStrBlockTime = atoi(token);
					mapd_printf(MSG_INFO, "ForceStrBlockTime =%d\n",
							dev->cli_steer_params.ForceStrBlockTime);
				} else if(os_strcmp(token, "BTMStrBlockTime") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.BTMStrBlockTime = atoi(token);
					mapd_printf(MSG_INFO, "BTMStrBlockTime =%d\n",
							dev->cli_steer_params.BTMStrBlockTime);
				} else if(os_strcmp(token, "ForceStrForbidTime") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.ForceStrForbidTime = atoi(token);
					mapd_printf(MSG_INFO, " ForceStrForbidTime=%d\n",
							dev->cli_steer_params.ForceStrForbidTime);
				} else if(os_strcmp(token, "BTMStrForbidTime") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.BTMStrForbidTime = atoi(token);
					mapd_printf(MSG_INFO, " BTMStrForbidTime=%d\n",
							dev->cli_steer_params.BTMStrForbidTime);
				} else if(os_strcmp(token, "StrForbidTimeJoin") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.StrForbidTimeJoin = atoi(token);
					mapd_printf(MSG_INFO, " StrForbidTimeJoin=%d\n",
							dev->cli_steer_params.StrForbidTimeJoin);
				} else if(os_strcmp(token, "MinSteerRetryTime") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MinSteerRetryTime= atoi((const char *)token);
					mapd_printf(MSG_INFO, " MinSteerRetryTime=%d\n",
							dev->cli_steer_params.MinSteerRetryTime);
				} else if(os_strcmp((const char *)token, "MaxSteerRetryTime") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MaxSteerRetryTime = atoi((const char *)token);
					mapd_printf(MSG_INFO, " MaxSteerRetryTime=%d\n",
							dev->cli_steer_params.MaxSteerRetryTime);
				} else if(os_strcmp((const char *)token, "St") == 0){
					token = strtok(NULL, "SteerRetryStep");
					dev->cli_steer_params.SteerRetryStep= atoi((const char *)token);
					mapd_printf(MSG_INFO, " SteerRetryStep=%d\n",
							dev->cli_steer_params.SteerRetryStep);
				} else if(os_strcmp((const char *)token, "MaxClientOverloaded") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MaxClientOverloaded = atoi((const char *)token);
					mapd_printf(MSG_INFO, "MaxClientOverloaded =%d\n",
							dev->cli_steer_params.MaxClientOverloaded);
				} else if(os_strcmp((const char *)token, "ActivityThreshold") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.ActivityThreshold = atoi((const char *)token);
					mapd_printf(MSG_INFO, " ActivityThreshold=%d\n",
							dev->cli_steer_params.ActivityThreshold);
				} else if(os_strcmp((const char *)token, "StartInActive") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.StartInActive = atoi((const char *)token);
					mapd_printf(MSG_INFO, " StartInActive=%d\n",
							dev->cli_steer_params.StartInActive);
#ifdef SUPPORT_MULTI_AP
				} else if(os_strcmp((const char *)token, "LowRSSIAPSteerEdge_root") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.LowRSSIAPSteerEdge_root = atoi((const char *)token);
					mapd_printf(MSG_INFO, "LowRSSIAPSteerEdge_root =%d\n",
							dev->cli_steer_params.LowRSSIAPSteerEdge_root);
				} else if(os_strcmp((const char *)token, "LowRSSIAPSteerEdge_RE") == 0){
					token = strtok(NULL, "");
					if(token){
					dev->cli_steer_params.LowRSSIAPSteerEdge_RE = atoi((const char *)token);
					mapd_printf(MSG_INFO, " LowRSSIAPSteerEdge_RE=%d\n",
							dev->cli_steer_params.LowRSSIAPSteerEdge_RE);
					}
				} else if(os_strcmp((const char *)token, "MinRssiIncTh_Root") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MinRssiIncTh_Root = atoi((const char *)token);
					mapd_printf(MSG_INFO, " MinRssiIncTh_Root=%d\n",
							dev->cli_steer_params.MinRssiIncTh_Root);
				} else if(os_strcmp((const char *)token, "MinRssiIncTh_RE") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MinRssiIncTh_RE = atoi((const char *)token);
					mapd_printf(MSG_INFO, "MinRssiIncTh_RE =%d\n",
							dev->cli_steer_params.MinRssiIncTh_RE);
				} else if(os_strcmp((const char *)token, "MinRssiIncTh_Peer") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.MinRssiIncTh_Peer = atoi((const char *)token);
					mapd_printf(MSG_INFO, " MinRssiIncTh_Peer=%d\n",
							dev->cli_steer_params.MinRssiIncTh_Peer);
#endif
				} else if(os_strcmp((const char *)token, "CUAvgPeriod") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.CUAvgPeriod = atoi((const char *)token);
					mapd_printf(MSG_INFO, " CUAvgPeriod=%d\n",
							dev->cli_steer_params.CUAvgPeriod);
				} else if(os_strcmp(token, "BTMStrTimeout") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.BTMStrTimeout = atoi(token);
					mapd_printf(MSG_INFO, " BTMStrTimeout=%d\n",
							dev->cli_steer_params.BTMStrTimeout);
				} else if(os_strcmp(token, "ForceStrTimeout") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.ForceStrTimeout = atoi(token);
					mapd_printf(MSG_INFO, " ForceStrTimeout=%d\n",
							dev->cli_steer_params.ForceStrTimeout);
#ifdef SUPPORT_MULTI_AP
				} else if(os_strcmp(token, "single_steer") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.single_steer = atoi((const char *)token);
					mapd_printf(MSG_INFO, "single_steer =%d\n",
							dev->cli_steer_params.single_steer);
#endif
				} else if(os_strcmp((const char *)token, "prohibitTime11K") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.prohibitTime11K = atoi((const char *)token);
					mapd_printf(MSG_INFO, "prohibitTime11K=%d\n",
							dev->cli_steer_params.prohibitTime11K);
				}  else if(os_strcmp((const char *)token, "PHYBasedSelection") == 0){
					token = strtok(NULL, "");
					dev->cli_steer_params.PHYBasedSelection = atoi((const char *)token);
					mapd_printf(MSG_INFO, "PHYBasedSelection=%d\n",
							dev->cli_steer_params.PHYBasedSelection);
				} else if(os_strcmp((const char *)token, "disable_pre_assoc_strng")==0) {
					token = strtok(NULL, "");
					dev->cli_steer_params.disable_pre_assoc_strng = atoi((const char *)token);
					mapd_printf(MSG_INFO, "disable_pre_assoc_strng=%d",
							dev->cli_steer_params.disable_pre_assoc_strng);
				} else if (os_strcmp((const char *)token, "disable_post_assoc_strng")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.disable_post_assoc_strng = atoi((const char *)token);
						mapd_printf(MSG_INFO, "disable_post_assoc_strng=%d",
							dev->cli_steer_params.disable_post_assoc_strng);
				} else if (os_strcmp((const char *)token, "disable_offloading")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.disable_offloading = atoi((const char *)token);
						mapd_printf(MSG_INFO, "disable_offloading=%d",
							dev->cli_steer_params.disable_offloading);
#ifdef SUPPORT_MULTI_AP
				} else if (os_strcmp((const char *)token, "disable_nolmultiap")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.disable_nolmultiap = atoi((const char *)token);
						mapd_printf(MSG_INFO, "disable_nolmultiap=%d",
							dev->cli_steer_params.disable_nolmultiap);
#endif
				} else if (os_strcmp((const char *)token, "disable_active_ug")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.disable_active_ug = atoi((const char *)token);
						mapd_printf(MSG_INFO, "disable_active_ug=%d",
							dev->cli_steer_params.disable_active_ug);
				} else if (os_strcmp((const char *)token, "disable_active_dg")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.disable_active_dg = atoi((const char *)token);
						mapd_printf(MSG_INFO, "disable_active_dg=%d",
							dev->cli_steer_params.disable_active_dg);
				} else if (os_strcmp((const char *)token, "disable_idle_dg")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.disable_idle_dg = atoi((const char *)token);
						mapd_printf(MSG_INFO, "disable_idle_dg=%d",
							dev->cli_steer_params.disable_idle_dg);
				} else if (os_strcmp((const char *)token, "disable_idle_ug")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.disable_idle_ug = atoi((const char *)token);
						mapd_printf(MSG_INFO, "disable_idle_ug=%d",
							dev->cli_steer_params.disable_idle_ug);
				} else if (os_strcmp((const char *)token, "GlobalProhibitTime")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.GlobalProhibitTime = atoi((const char *)token);
						mapd_printf(MSG_INFO, "GlobalProhibitTime=%d",
							dev->cli_steer_params.GlobalProhibitTime);
				} else if (os_strcmp((const char *)token, "idle_count_th")==0){
						token = strtok(NULL, "");
						dev->cli_steer_params.idle_count_th = atoi((const char *)token);
						mapd_printf(MSG_INFO, "idle_count_th=%d",
							dev->cli_steer_params.idle_count_th);
				} else if (os_strcmp((const char *)token, "reset_btm_csbc_at_join") == 0){
						token = strtok(NULL, "");
						dev->cli_steer_params.reset_btm_csbc_at_join = atoi((const char *)token);
						mapd_printf(MSG_INFO, "reset_btm_csbc_at_join=%d",
							dev->cli_steer_params.reset_btm_csbc_at_join);
						if (dev->cli_steer_params.reset_btm_csbc_at_join) {
							mapd_printf(MSG_OFF, "WARNING: ****** BTM CSBC would be reset at join************");
						}
				} else if(os_strcmp((const char *)token, "ForcedRssiUpdate")==0) {
						token = strtok(NULL, "");
						dev->cli_steer_params.ForcedRssiUpdate = atoi((const char *)token);
						mapd_printf(MSG_INFO, "ForcedRssiUpdate=%d",
							dev->cli_steer_params.ForcedRssiUpdate);
#ifdef SUPPORT_MULTI_AP
#ifdef CENT_STR
				} else if(os_strcmp((const char *)token, "CentSteerMaxBSFail")==0) {
							token = strtok(NULL, "");
							dev->cli_steer_params.cent_str_max_bs_fail = atoi((const char *)token);
							mapd_printf(MSG_INFO, "CentSteerMaxBSFail=%d",
								dev->cli_steer_params.cent_str_max_bs_fail);
				} else if(os_strcmp((const char *)token, "CentStrMaxOLSteerCand")==0) {
							token = strtok(NULL, "");
							dev->cli_steer_params.cent_str_max_ol_steer_cand = atoi((const char *)token);
							mapd_printf(MSG_INFO, "CentStrMaxOLSteerCand=%d",
								dev->cli_steer_params.cent_str_max_ol_steer_cand);
				} else if (os_strcmp((const char *)token, "CentStrMaxUGSteerCand")==0) {
							token = strtok(NULL, "");
							dev->cli_steer_params.cent_str_max_ug_steer_cand = atoi((const char *)token);
							mapd_printf(MSG_INFO, "CentStrMaxUGSteerCand=%d",
								dev->cli_steer_params.cent_str_max_ug_steer_cand);
				} else if(os_strcmp((const char *)token, "CentStrCuMonTime")==0) {
							token = strtok(NULL, "");
							dev->cli_steer_params.cent_str_cu_mon_time = atoi((const char *)token);
							mapd_printf(MSG_INFO, "CentStrCuMonTime=%d",
								dev->cli_steer_params.cent_str_cu_mon_time);
				} else if(os_strcmp((const char *)token, "CentStrCuMonProhibitTime")==0) {
							token = strtok(NULL, "");
							dev->cli_steer_params.cent_str_cu_mon_prohibit_time = atoi((const char *)token);
							mapd_printf(MSG_INFO, "CentStrCuMonProhibitTime=%d",
								dev->cli_steer_params.cent_str_cu_mon_prohibit_time);

#endif
				} else if(os_strcmp((const char *)token, "MetricPolicyRcpi_24G")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyRcpi = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyRcpi_24G=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyRcpi);
				} else if(os_strcmp((const char *)token, "MetricPolicyHys_24G")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyHys = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyHys_24G=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyHys);
				} else if(os_strcmp((const char *)token, "MetricPolicyMetricsInclusion_24G")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyMetricsInclusion = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyMetricsInclusion_24G=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyMetricsInclusion);
				}else if(os_strcmp((const char *)token, "MetricPolicyTrafficInclusion_24G")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyTrafficInclusion = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyTrafficInclusion_24G=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyTrafficInclusion);
				}else if(os_strcmp((const char *)token, "MetricPolicyChUtilThres_24G")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyChUtilThres = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyChUtilThres_24G=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_2G-1].MetricPolicyChUtilThres);
				}else if(os_strcmp((const char *)token, "MetricPolicyRcpi_5GL")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyRcpi = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyRcpi_24G=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyRcpi);
				} else if(os_strcmp(token, "MetricPolicyHys_5GL")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyHys = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyHys_24G=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyHys);
				} else if(os_strcmp((const char *)token, "MetricPolicyMetricsInclusion_5GL")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyMetricsInclusion = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyMetricsInclusion_5GL=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyMetricsInclusion);
				}else if(os_strcmp((const char *)token, "MetricPolicyTrafficInclusion_5GL")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyTrafficInclusion = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyTrafficInclusion_5GL=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyTrafficInclusion);
				}else if(os_strcmp((const char *)token, "MetricPolicyChUtilThres_5GL")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyChUtilThres = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyChUtilThres_5GL=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GL-1].MetricPolicyChUtilThres);
				}else if(os_strcmp((const char *)token, "MetricPolicyRcpi_5GH")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyRcpi = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyRcpi_5GH=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyRcpi);
				} else if(os_strcmp((const char *)token, "MetricPolicyHys_5GH")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyHys = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyHys_5GH=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyHys);
				} else if(os_strcmp((const char *)token, "MetricPolicyMetricsInclusion_5GH")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyMetricsInclusion = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyMetricsInclusion_5GH=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyMetricsInclusion);
				}else if(os_strcmp(token, "MetricPolicyTrafficInclusion_5GH")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyTrafficInclusion = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyTrafficInclusion_5GH=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyTrafficInclusion);
				}else if(os_strcmp((const char *)token, "MetricPolicyChUtilThres_5GH")==0) {
						token = strtok(NULL, "");
						dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyChUtilThres = atoi((const char *)token);
						mapd_printf(MSG_INFO, "MetricPolicyChUtilThres_5GH=%d",
							dev->controller_context.ap_metric_policy.policy_params[BAND_5GH-1].MetricPolicyChUtilThres);
#endif
				}
#ifdef MAP_R2
			else if(os_strcmp((const char *)token, "ChPlanningChUtilThresh_24G")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_plan_thres[BAND_2G-1].ch_util_threshold = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningChUtilThresh_24G=%d",
					dev->ch_planning_R2.ch_plan_thres[BAND_2G-1].ch_util_threshold );
			} else if(os_strcmp((const char *)token, "ChPlanningChUtilThresh_5GL")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_plan_thres[BAND_5GL-1].ch_util_threshold = atoi((const char *)token);
				dev->ch_planning_R2.ch_plan_thres[BAND_5GH-1].ch_util_threshold = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningChUtilThresh_5GL=%d",
					dev->ch_planning_R2.ch_plan_thres[BAND_5GL-1].ch_util_threshold );
			} else if(os_strcmp((const char *)token, "ChPlanningEDCCAThresh_24G")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_plan_thres[BAND_2G-1].edcca_threshold = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningEDCCAThresh_24G=%d",
					dev->ch_planning_R2.ch_plan_thres[BAND_2G-1].edcca_threshold );
			} else if(os_strcmp((const char *)token, "ChPlanningEDCCAThresh_5GL")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_plan_thres[BAND_5GL-1].edcca_threshold = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningEDCCAThresh_5GL=%d",
					dev->ch_planning_R2.ch_plan_thres[BAND_5GL-1].edcca_threshold );
			}else if(os_strcmp((const char *)token, "ChPlanningOBSSThresh_24G")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_plan_thres[BAND_2G-1].obss_load_threshold = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningOBSSThresh_24G=%d",
					dev->ch_planning_R2.ch_plan_thres[BAND_2G-1].obss_load_threshold );
			} else if(os_strcmp((const char *)token, "ChPlanningOBSSThresh_5GL")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_plan_thres[BAND_5GL-1].obss_load_threshold = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningOBSSThresh_5GL=%d",
					dev->ch_planning_R2.ch_plan_thres[BAND_5GL-1].obss_load_threshold );
			}else if(os_strcmp((const char *)token, "ChPlanningR2MonitorTimeoutSecs")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_monitor_timeout = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningR2MonitorTimeoutSecs=%d",
					dev->ch_planning_R2.ch_monitor_timeout);
			}else if(os_strcmp((const char *)token, "ChPlanningR2MonitorProhibitSecs")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_monitor_prohibit_wait_time = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningR2MonitorProhibitSecs=%d",
					dev->ch_planning_R2.ch_monitor_prohibit_wait_time);
			} else if(os_strcmp((const char *)token, "ChPlanningR2MetricReportingInterval")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.ch_plan_metric_policy_interval = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningR2MetricReportingInterval=%d",
					dev->ch_planning_R2.ch_plan_metric_policy_interval);
			} else if(os_strcmp((const char *)token, "ChPlanningR2MinScoreMargin")==0) {
				token = strtok(NULL, "");
				dev->ch_planning_R2.min_score_inc = atoi((const char *)token);
				mapd_printf(MSG_ERROR, "ChPlanningR2MinScoreMargin=%d",
					dev->ch_planning_R2.min_score_inc);
			}
#endif
				else if(os_strcmp((const char *)token, "MetricRepIntv")==0) {
				token = strtok(NULL, "");
				dev->ap_metric_rep_intv = atoi((const char *)token);
					mapd_printf(MSG_OFF, "MetricRepIntv=%d",
					dev->ap_metric_rep_intv);
			}

			}
		}
		fclose(file);
	}

}

void mapd_init_own_1905_dev(struct own_1905_device *dev)
{
	uint8_t idx = 0;
	/* Get steer_params cli_steer_params */
	/* Get map_1905_device info
	 * struct map_1905_device *node_pointers[MAX_NODES];
	 * */

	/* Array representing client database */
	client_db_init(&dev->client_db[0]);

	dl_list_init(&dev->sta_seen_list);

#ifdef CENT_STR
	dev->p_cent_str_curr_1905_rr = NULL;
#endif

	for (idx = 0; idx<MAX_NUM_OF_RADIO; ++idx) {
		mapd_init_radio_interface(&dev->dev_radio_info[idx]);
	}

#ifdef ACL_CTRL
	dl_list_init(&dev->acl_cli_list);
#endif
	/* List of clients sorted by last seen */
	//struct controller ctx;
	//struct agent agent_ctx;
	dev->wsc_save_bh_profile = FALSE;
}

#ifdef BACKTRACK_SUPPORT
void mapd_sigsegv_handler(int sig)
{
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

static void mapd_enable_backtrack()
{
	signal(SIGSEGV, mapd_sigsegv_handler);
}
#else
static void mapd_enable_backtrack() { }
#endif

#ifdef CORE_DUMP_ENABLED
static void mapd_enable_core_dump()
{
	struct rlimit limit;
	err("enabling core dump");
	limit.rlim_cur = 65535;
	limit.rlim_max = 65535;
	setrlimit(RLIMIT_CORE, &limit);
}
#else
static void mapd_enable_core_dump() { }
#endif
#ifdef SUPPORT_MULTI_AP
int mapfilter_init(struct own_1905_device *dev)
{
	u8 mac_addr[ETH_ALEN] = {0};
	u8 total_itfs = 0, band;
	struct local_itfs *itfs = NULL;
	int i = 0;
	struct local_interface *inf = NULL;
	struct mapd_radio_info *ra_info = NULL;

	mapfilter_netlink_init(getpid());
	/* Get ETH MAC ADDR */
	if(!lookup_iface_addr((char *)dev->lan_iface, mac_addr)) {
			dev->eth_itf = (struct local_interface *)os_malloc(sizeof(struct local_interface));
			if (dev->eth_itf) {
				os_memset(dev->eth_itf->name, 0, IFNAMSIZ);
				os_memcpy(dev->eth_itf->name, dev->lan_iface, IFNAMSIZ);
				os_memcpy(dev->eth_itf->mac, mac_addr, ETH_ALEN);
				dev->eth_itf->dev_type = ETH;
				total_itfs ++;
				mapd_printf(MSG_OFF, "ETH ifname:%s mac " MACSTR "type=%02x, band=%d\n",
								dev->eth_itf->name, MAC2STR(dev->eth_itf->mac),
								dev->eth_itf->dev_type, dev->eth_itf->band);
			}
			else
				return -1;
	}
	total_itfs += dev->num_wifi_itfs;
	itfs = (struct local_itfs *)os_malloc(sizeof(struct local_itfs) +
					total_itfs * sizeof(struct local_interface));
	if (!itfs)
		return -1;

	itfs->num = total_itfs;
	inf = &itfs->inf[0];

	os_memcpy(inf, dev->eth_itf, sizeof(struct local_interface));
	inf++;
	for (i = 0; i < dev->num_wifi_itfs; i++) {
		os_memcpy(inf, dev->wifi_itfs[i], sizeof(struct local_interface));
		mapd_printf(MSG_OFF," Name=%s MAC " MACSTR, inf->name, MAC2STR(inf->mac));
		inf ++;
	}

	mapfilter_set_all_interface(itfs);
	os_free(itfs);

	mapfilter_enable_dynamic_load_balance(dev->load_balance_en);
	if (dev->dual_bh_en && dev->load_balance_en) {
		for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
			ra_info = &dev->dev_radio_info[i];
			if (ra_info->radio_idx == (uint8_t)-1)
				continue;

			if (ra_info->channel > 0 && ra_info->channel <= 14)
				band = _24G;
			else if (ra_info->channel >= 36 && ra_info->channel < 100)
				band = _5GL;
			else
				band = _5GH;
			mapfilter_set_channel_utilization(ra_info->identifier, band, ra_info->ch_util);

		}
	}
	return 0;
}
#endif
struct mapd_global * mapd_init(struct mapd_params *params)
{
    struct mapd_global *global;
    int ret;
  //  uint8_t file_name[] = "/etc/mapd_strng.conf";

    if (params == NULL)
        return NULL;

    /* backtrace is not available for mip32 */
    if (params->core_dump)
        mapd_enable_core_dump();
    else
        mapd_enable_backtrack();

    if (params->mapd_debug_file_path)
        mapd_debug_open_file(params->mapd_debug_file_path);
    else
        mapd_debug_setup_stdout();
    if (params->mapd_debug_syslog)
        mapd_debug_open_syslog();
    if (params->mapd_debug_tracing) {
        ret = mapd_debug_open_linux_tracing();
        if (ret) {
            mapd_printf(MSG_ERROR,
                    "Failed to enable trace logging");
            return NULL;
        }
    }
    global = os_zalloc(sizeof(*global));
    if (global == NULL)
        return NULL;
#ifdef SUPPORT_MULTI_AP
	global->params.Certification = params->Certification;
#endif
    global->params.daemonize = params->daemonize;
    if (params->pid_file)
        global->params.pid_file = os_strdup(params->pid_file);
    if (params->ctrl_interface)
        global->params.ctrl_interface =
            os_strdup(params->ctrl_interface);
    else
        global->params.ctrl_interface = os_strdup("/tmp/mapd_ctrl");

#ifdef CONFIG_CLIENT_DB_FILE
	if(params->clientDBname)
		global->params.clientDBname = os_strdup(params->clientDBname);
	else
		global->params.clientDBname = os_strdup("/tmp/client_db.txt");
	mapd_printf(MSG_ERROR, "DB NAME=%s", global->params.clientDBname);
#endif
    mapd_debug_level = global->params.mapd_debug_level =
        params->mapd_debug_level;
    mapd_debug_timestamp = global->params.mapd_debug_timestamp =
        params->mapd_debug_timestamp;

    mapd_printf(MSG_OFF, "Man Daemon XX version: %s", VERSION_MAPD);

    if (eloop_init()) {
        mapd_printf(MSG_ERROR, "Failed to initialize event loop");
        mapd_deinit(global);
        return NULL;
    }

    global->ctrl_iface = mapd_global_ctrl_iface_init(global);
    if (global->ctrl_iface == NULL) {
        mapd_deinit(global);
        return NULL;
    }
	/* Start comm with WAPP */
	ret = wapp_open_connection("/tmp/wapp_ctrl", global);
	while (ret != 0) {
		mapd_printf(MSG_ERROR, "Failed to connect to WAPP");
		/* Sleep for 1 sec */
		os_sleep(1, 0);
		/* Try again */
		ret = wapp_open_connection("/tmp/wapp_ctrl", global);
	}
	mapd_printf(MSG_OFF, "Succesfully connected to WAPPD");

	wlanif_register_wapp_events(global);
#ifdef SUPPORT_MULTI_AP
	/* Start comm with 1905D */
	while (_1905_open_connection("map_daemon", global) != 0) {
		mapd_printf(MSG_ERROR, "Failed to connect to 1905 daemon");
		os_sleep(1, 0);
	}
	mapd_printf(MSG_OFF, "Succesfully connected to 1905");
	ret = wapp_get_all_wifi_interface_status(global);
	while (ret != 1) {
		mapd_printf(MSG_ERROR, "wapp wifi interface not ready");
		os_sleep(1, 0);
		ret = wapp_get_all_wifi_interface_status(global);
	}
	mapd_printf(MSG_OFF, "Wapp wireless interface init success");
#endif
	/* Init own_1905_dev */
	mapd_init_own_1905_dev(&global->dev);
	global->dev.back_ptr = (void *) global;

	/* Steer Params */
	mapd_init_steer_params(&global->dev, g_mapd_strng_path);

#ifdef SUPPORT_MULTI_AP
	if( !global->params.Certification) {
#endif
			/* Read DB file */
			mapd_client_db_read(global);
#ifdef SUPPORT_MULTI_AP
	}
	global->dev.scan_triggered = FALSE;
	topo_srv_init_own_info(&global->dev);
	if (global->dev.device_role == DEVICE_ROLE_CONTROLLER) {
		mapd_printf(MSG_ERROR, "map_1905_Set_Read_Bss_Conf_and_Renew");
		map_1905_Set_Read_Bss_Conf_and_Renew(global->_1905_ctrl, 1);
	}
	mapd_read_config_file(&global->dev);

	if (mapfilter_init(&global->dev)) {
		mapd_printf(MSG_OFF, "MAP Filter init Failed");
	}
	/*check auto role drop flag*/
	if (global->dev.device_role == DEVICE_ROLE_UNCONFIGURED) {
		mapfilter_set_drop_specific_dest_ip_status(1);
		err("role(DEVICE_ROLE_UNCONFIGURED), set drop flag to mapfilter");
	} else {
		mapfilter_set_drop_specific_dest_ip_status(0);
		err("role(%d), cancel drop flag to mapfilter",
			global->dev.device_role);
	}

	global->dev.bl_timeout = MAX_BL_TIMEOUT;
	eloop_register_timeout(0, 0, mapd_start_wired_iface_monitor, global, NULL);
	eloop_register_timeout(5, 0, topo_srv_start_1905_timer, global, NULL);
	/* Flush all (leftover) BL */
	mapd_flush_all_bl(global);
	if (global->dev.dhcp_ctl_enable
		&& (global->dev.device_role == DEVICE_ROLE_AGENT)) {
		if (!(1 == global->dev.dhcp_req.dhcp_server_enable && 0 == global->dev.dhcp_req.dhcp_client_enable)) {
					mapd_printf(MSG_OFF,"Agent Init,enable_dhcp_server!\n");
					global->dev.dhcp_req.dhcp_server_enable = 1;
					global->dev.dhcp_req.dhcp_client_enable = 0;
					wlanif_issue_wapp_command((struct mapd_global *)global->dev.back_ptr, WAPP_USER_SET_DHCP_CTL_REQUEST,
						0, NULL, NULL, &global->dev.dhcp_req, sizeof(struct dhcp_ctl_req), 0, 0, 0);
		}
	}
	network_opt_init(global);

#ifdef MAP_R2
	if(global->dev.device_role == DEVICE_ROLE_CONTROLLER) {
		ch_planning_R2_init(&global->dev);
	}
#endif
#else
	{
	struct own_1905_device *ctx = &global->dev;
	int j;
	if(ctx) {
		mapd_printf(MSG_DEBUG, "sending wlanif_get_op_chan_info\n");
		SLIST_INIT(&ctx->steer_cands_head);
		wlanif_get_op_chan_info(ctx->back_ptr);
	}

	for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[j];
		if (radio_info->radio_idx == (uint8_t)-1) {
			continue;
		}
		mapd_printf(MSG_DEBUG, " sending WAPP_OPERBSS_REPORT for j=%d radio_info->radio_idx:%d\n", j, radio_info->radio_idx);

		map_get_info_from_wapp(ctx, WAPP_USER_GET_OPERATIONAL_BSS,
			WAPP_OPERBSS_REPORT, radio_info->identifier, NULL, NULL, 0);
		/*get cap for the specific radio */
		map_get_info_from_wapp(ctx, WAPP_USER_GET_RADIO_BASIC_CAP, WAPP_RADIO_BASIC_CAP, radio_info->identifier, NULL, NULL, 0);
		map_get_info_from_wapp(ctx, WAPP_USER_GET_AP_HT_CAPABILITY, WAPP_AP_HT_CAPABILITY, radio_info->identifier, NULL, NULL, 0);
		map_get_info_from_wapp(ctx, WAPP_USER_GET_AP_VHT_CAPABILITY, WAPP_AP_VHT_CAPABILITY, radio_info->identifier, NULL, NULL, 0);
		map_get_info_from_wapp(ctx, WAPP_USER_GET_AP_HE_CAPABILITY, WAPP_AP_HE_CAPABILITY, radio_info->identifier, NULL, NULL, 0);
	}
	/* Flush all (leftover) BL */
	mapd_flush_all_bl(global);
	}
#endif
	global->dev.cac_enable = FALSE;
	/* Start a periodic_exec function */
    eloop_register_timeout(0, MAPD_HOUSEKEEPING_INTERVAL,
            mapd_periodic, global, NULL);

#ifdef CENT_STR
	if(global->dev.cent_str_en && global->dev.device_role == DEVICE_ROLE_CONTROLLER)
		eloop_register_timeout(CENT_STR_1_MIN, 0,
			cent_str_rr_steer_cand_selection, global, NULL);
#endif

    return global;
}

static int mapd_daemon(const char *pid_file)
{
    mapd_printf(MSG_INFO, "Daemonize..");
    return os_daemonize(pid_file);
}

void mapd_terminate_proc(struct mapd_global *global)
{
    int pending = 0;
    /* Mark if anything is pending and termination needs to delayed */
    if (pending)
        return;
    eloop_terminate();
}

static void mapd_terminate(int sig, void *signal_ctx)
{
    struct mapd_global *global = signal_ctx;
    mapd_terminate_proc(global);
}

int mapd_run(struct mapd_global *global)
{
    if (global->params.daemonize &&
            (mapd_daemon(global->params.pid_file) ||
             eloop_sock_requeue()))
        return -1;

    eloop_register_signal_terminate(mapd_terminate, global);

    eloop_run();

    return 0;
}
/* Get the pointer to a particular bss provided the radio idx and mac address */
struct mapd_radio_info * get_radio_info_by_radio_id(struct mapd_global *global,
				unsigned char *radio_id)
{
	uint8_t op_channel_idx = 0;
	for (op_channel_idx = 0; op_channel_idx < MAX_NUM_OF_RADIO; op_channel_idx++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[op_channel_idx];
		if (radio_info->channel == 0)
			continue;
		if(!os_memcmp(radio_info->identifier, radio_id, ETH_ALEN)) {
				return radio_info;
		}
	}
	return NULL;
}

unsigned char *mapd_get_ssid_from_bssid(struct mapd_global *global, unsigned char *bssid)
{
	uint8_t op_channel_idx = 0;
	struct mapd_bss *bss;

	for (op_channel_idx = 0; op_channel_idx < MAX_NUM_OF_RADIO; op_channel_idx++)
   	{
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[op_channel_idx];
		//printf("Entering next iteration\n");
		if (radio_info->radio_idx == (uint8_t)-1)
		{
			err("Uinitialized radio, continuing");
			continue;
		}
		err("Iterating over bss list of radio #%d", op_channel_idx);
		dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry)
			if (!os_memcmp(bssid, bss->bssid, ETH_ALEN))
				return bss->ssid;
	}
	return NULL;
}



struct mapd_radio_info * mapd_get_radio_info_from_bss(struct mapd_global *global,
		struct mapd_bss *target_bss)
{
	uint8_t op_channel_idx;
    struct mapd_bss *bss = NULL;

	for (op_channel_idx = 0; op_channel_idx < MAX_NUM_OF_RADIO; op_channel_idx++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[op_channel_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry)
			if (!os_memcmp(target_bss->bssid, bss->bssid, ETH_ALEN))
				return radio_info;
	}
	return NULL;
}

struct mapd_bss * mapd_get_bss_from_mac(struct mapd_global *global, u8 *mac_addr)
{
    struct mapd_bss *bss = NULL;
	uint8_t op_channel_idx = 0;
	for (op_channel_idx = 0; op_channel_idx < MAX_NUM_OF_RADIO; op_channel_idx++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[op_channel_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry)
				if (bss && !os_memcmp(mac_addr, bss->bssid, ETH_ALEN))
					return bss;
	}
	return NULL;
}

struct mapd_bss * mapd_get_bss_from_bssid(struct mapd_global *global, unsigned char *bssid)
{
        uint8_t radio_idx = 0;
        struct mapd_bss *bss = NULL;

        for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++)
        {
                struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[radio_idx];
                if (radio_info->radio_idx == (uint8_t)-1)
                {
                        err("Uinitialized radio, continuing");
                        continue;
                }
                err("Iterating over bss list of radio #%d", radio_idx);
                dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry)
                        if (bss && !os_memcmp(bssid, bss->bssid, ETH_ALEN))
                                return bss;
        }
        return NULL;
}

uint8_t get_free_bss_idx_in_bitmap(struct mapd_radio_info *radio_info) {

	uint32_t bitmap = radio_info->bss_bitmap;
	uint8_t idx = 0;
	for (idx = 0; idx<MAX_NUM_BSS; ++idx)
		if (!((1 << idx) & bitmap)) {
			radio_info->bss_bitmap = (radio_info->bss_bitmap | (1<<idx));
			break;
		}
	return idx;
}

void reset_bss_idx_in_bitmap(struct mapd_radio_info *radio_info, uint8_t idx) {
	radio_info->bss_bitmap = (radio_info->bss_bitmap & ~(1<<idx));
}

void bss_init(struct mapd_radio_info *radio_info, unsigned char *bssid, unsigned char *ssid, u8 ssid_len, uint8_t bss_idx) {

	debug("Entering bss_init function\n");

	if (radio_info->channel == 0) {
		err("Attempting to add bss to nonexistent radio interface \n");
		return;
	}

	struct mapd_bss *new_bss = os_zalloc(sizeof(struct mapd_bss));
	if (new_bss == NULL) {
		err("Malloc failed");
		return;
	}

	os_memcpy(new_bss->ssid, ssid, 33);
	new_bss->ssid_len = ssid_len;
	new_bss->ssid[new_bss->ssid_len] = '\0';

	os_memcpy(new_bss->bssid, bssid, ETH_ALEN);
	dl_list_init(&new_bss->bl_sta_list);
	dl_list_init(&new_bss->assoc_sta_list);
	new_bss->bss_idx = bss_idx;
	new_bss->radio_idx = radio_info->radio_idx;
	new_bss->channel = radio_info->channel;
	dl_list_add(&radio_info->bss_list, &new_bss->bss_entry);

}


void bss_deinit(struct mapd_radio_info *radio_info, struct mapd_bss *bss) {

	struct client *tmp = NULL, *next = NULL;
	struct bl_client *bl_sta_curr = NULL, *bl_sta_next = NULL;

	dl_list_for_each_safe(tmp, next, &bss->assoc_sta_list, struct client, assoc_sta_entry)
	{
		dl_list_del(&tmp->assoc_sta_entry);
	}

	dl_list_for_each_safe(bl_sta_curr, bl_sta_next, &bss->bl_sta_list,
			struct bl_client, list_entry) {
		struct map_dev *map_dev_curr = NULL, *map_dev_next = NULL;
		dl_list_for_each_safe(map_dev_curr, map_dev_next, &bl_sta_curr->map_dev_list,
						struct map_dev, map_dev_entry) {
			dl_list_del(&map_dev_curr->map_dev_entry);
			os_free(map_dev_curr);
		}
		dl_list_del(&bl_sta_curr->list_entry);
		os_free(bl_sta_curr);
	}

	dl_list_del(&bss->bss_entry);
	os_memset(bss,0, sizeof(struct mapd_bss));
	os_free(bss);

}


uint8_t mapd_get_radio_idx_from_bssid(struct mapd_global *global, u8 *bssid)
{
	struct mapd_bss *bss = mapd_get_bss_from_mac(global, bssid);
	if(bss)
		return bss->radio_idx;
	else
		return -1;
}


uint8_t mapd_get_channel_from_bssid(struct mapd_global *global, u8 *bssid)
{
	struct mapd_bss *bss = mapd_get_bss_from_mac(global, bssid);
	if (bss != NULL)
		return bss->channel;
	return 0;
}

struct mapd_radio_info * mapd_get_radio_from_channel(
			struct mapd_global *global, uint8_t channel)
{
	uint8_t i = 0;

	if (channel == 0) {
		mapd_printf(MSG_ERROR, "Channel=0");
		return NULL;
	}
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		struct mapd_radio_info *ra_info = NULL;
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;
		if (ra_info->channel == channel)
			return ra_info;
	}
	mapd_printf(MSG_INFO, "%s: channel not found", __func__);
	return NULL;
}

void mapd_add_client_to_bss_assoc_list(struct mapd_global *global,
			u8 client_id, u8* bssid)
{
	struct mapd_bss *bss = mapd_get_bss_from_mac(global, bssid);
	struct dl_list *assoc_sta_entry = NULL;

	assoc_sta_entry = client_db_get_assoc_list_entry(global, client_id);
	if(bss == NULL || assoc_sta_entry->next != 0 || assoc_sta_entry->prev != 0)
	{
			mapd_printf(MSG_ERROR, "%s: Assoc for already associated", __func__);
			return;
	}
	dl_list_add(&bss->assoc_sta_list, assoc_sta_entry);
}

void mapd_handle_bss_channel_change(struct mapd_bss *bss, uint8_t new_channel)
{
	struct client *client = NULL;
	bss->channel = new_channel;
	dl_list_for_each(client, &bss->assoc_sta_list, struct client, assoc_sta_entry) {
			client->current_chan = bss->channel; //change it to client_db_handle_cli_channel_change
	}
	/* XXX:Do we need to do anything with the bss->bl_sta_list */
}
void mapd_handle_radio_channel_change(struct mapd_radio_info *radio_info, uint8_t new_channel)
{
	struct mapd_bss *bss = NULL;

	mapd_printf(MSG_INFO, "%s: Channel changed on " MACSTR " (%d)from %d to %d",
					__func__, MAC2STR(radio_info->identifier), radio_info->radio_idx,
					radio_info->channel, new_channel);

	radio_info->channel = new_channel;
	/* Change channel for all the BSSs on this radio, and it's associated clients */
	dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry) {
		mapd_handle_bss_channel_change(bss, new_channel);
	}
}

/* OK */
void mapd_handle_ap_metrics_info(struct mapd_global *global, u8 *bssid, u8 ch_util,
					 unsigned short assoc_sta_cnt)
{
	uint32_t temp_util;
	struct mapd_bss *bss = mapd_get_bss_from_mac(global, bssid);

	mapd_printf(MSG_DEBUG, "%s: ch_util(qload)=%d assoc_cnt=%d", __func__, ch_util,
					assoc_sta_cnt);
	if (bss == NULL) {
		mapd_printf(MSG_ERROR, "%s: No corrosponding BSS found", __func__);
		mapd_ASSERT(0);
		return;
	}
	bss->assoc_sta_cnt = assoc_sta_cnt;
	temp_util = ch_util*100;
	temp_util /= 255;
	mapd_printf(MSG_DEBUG, "%s: chan utilization : %d", __func__, temp_util);
	chan_mon_set_util(global, bss->radio_idx, (uint8_t)temp_util);

	if(global->dev.bh_cu_params.bh_switch_cu_en)
		bh_switch_check_by_cu(global, bss->radio_idx);
}

void mapd_radio_init(uint8_t radio_idx, struct mapd_radio_info *radio_info, uint8_t channel,
				uint8_t op_class, signed char tx_power, u8 *identifier)
{
	mapd_printf(MSG_INFO, "%s: RUID=" MACSTR " OpClass=%d Channel=%d TxPower=%d",
					__func__, MAC2STR(identifier), op_class, channel, tx_power);

	radio_info->radio_idx = radio_idx;
	radio_info->channel = channel;
	if(op_class != 0)
		radio_info->op_class = op_class;
	else
		radio_info->op_class = chan_mon_get_op_class_frm_channel(channel, BW_20);

	radio_info->tx_power = tx_power;
	os_memcpy(&radio_info->identifier, identifier, ETH_ALEN);
	dl_list_init(&radio_info->bss_list);
}

void mapd_radio_deinit(struct mapd_global *global, struct mapd_radio_info *radio_info)
{
	struct mapd_bss *bss_elem = NULL;
	/* Remove the BSS list */
	while ((bss_elem = dl_list_first(&radio_info->bss_list, struct mapd_bss,
									bss_entry))) {
			bss_deinit(radio_info, bss_elem);
	}
	os_memset((char *)radio_info, 0, sizeof(struct mapd_radio_info));
	radio_info->radio_idx = (uint8_t)-1;
}
#ifdef SUPPORT_MULTI_AP
/*
xx:xx:xx:xx:xx:xx
*/
int mapd_set_enrollee_bh(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	struct timeval tv;
	struct enrollee_bh bh;
	size_t len = buf_len;

	if (hwaddr_aton(cmd_buf, bh.mac_address) < 0)
		return -1;

	if (hexstr2bin(cmd_buf + 18, &bh.if_type, 1) < 0)
		return -1;

	if (wlanif_issue_wapp_command(global, WAPP_USER_SET_ENROLLEE_BH, 0, bh.mac_address,
			bh.mac_address, &bh, sizeof(struct enrollee_bh), 0, 0, 0) < 0)
		return -1;

	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (wapp_usr_intf_ctrl_pending(global->wapp_ctrl, &tv)) {
		if (wapp_usr_intf_ctrl_recv(global->wapp_ctrl, buf, &len) < 0) {
			return -1;
		}
	} else
		return -1;

	return len;
}

int mapd_set_bh_priority(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	int bands;
	int priority_2g, priority_5gl, priority_5gh;
	struct bh_link_entry *bh_entry = NULL;
	struct own_1905_device *ctx = &global->dev;
	unsigned char trigger_flag = 0;

	bands = atoi(cmd_buf);
	priority_2g = atoi(cmd_buf + 2);
	priority_5gl = atoi(cmd_buf + 4);
	if (bands == 3)
		priority_5gh = atoi(cmd_buf + 6);
	else
		priority_5gh = priority_5gl;
	SLIST_FOREACH(bh_entry, &(global->dev.bh_link_head), next_bh_link) {
		if (bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED ) {
			trigger_flag = 1;
		}
		if (bh_entry->bh_channel <= 14) {
			bh_entry->priority_info.priority = priority_2g;
		} else if (bh_entry->bh_channel > 14 && bh_entry->bh_channel < 100) {
			bh_entry->priority_info.priority = priority_5gl;
		} else if (bh_entry->bh_channel >= 100) {
			bh_entry->priority_info.priority = priority_5gh;
		}
	}
	send_vs_bh_priority(ctx);
	eloop_cancel_timeout(ap_selection_reconnection_timeout, ctx, NULL);
	if(trigger_flag) {
		#if 0
		if(IS_NTWRK_OPT_TRIGGERED(ctx)){
			err("Network Optimization Stop Due to Band switch");
			send_network_optimization_rsp(global, NETWORK_OPTIMIZATION_FAILED);
			reset_ntwrk_opt_states(ctx);
		}
		if(ctx->network_optimization_enabled) {
			trigger_network_optimization_stop_to_controller(global);
		}
		#endif
		ctx->link_fail_single_channel_scan_count = 3;
		ctx->current_bh_substate = BH_SUBSTATE_IDLE;
		ctx->current_bh_state = BH_STATE_WIFI_BAND_SWITCHED;
		//err("Block Network Optimization");
		ap_selection_issue_scan(ctx);
	}

	return 0;
}
int mapd_set_bss_role(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	struct timeval tv;
	struct bss_role brole;
	size_t len = buf_len;

	if (hwaddr_aton(cmd_buf, brole.bssid) < 0)
		return -1;

	if (hexstr2bin(cmd_buf + 18, &brole.role, 1) < 0)
		return -1;

	if (wlanif_issue_wapp_command(global, WAPP_USER_SET_BSS_ROLE, 0, brole.bssid,
			brole.bssid, &brole, sizeof(struct bss_role), 0, 0, 0) < 0)
		return -1;

	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (wapp_usr_intf_ctrl_pending(global->wapp_ctrl, &tv)) {
		if (wapp_usr_intf_ctrl_recv(global->wapp_ctrl, buf, &len) < 0) {
			return -1;
		}
	} else
		return -1;

	return len;
}

int mapd_set_scan_rssi_thresh(struct mapd_global *global, char *cmd_buf, int band)
{
	int j = 0;
	unsigned char rssi_thresh[4] = {0};
	while(cmd_buf[j] != ' ') {
		rssi_thresh[j] = cmd_buf[j];
		j++;
		if(j == 4)
			break;
	}
	if (band == 0)
		global->dev.rssi_threshold_2g = (signed short)atoi((const char *)rssi_thresh);
	else if (band == 1)
		global->dev.rssi_threshold_5g = (signed short)atoi((const char *)rssi_thresh);
	printf("2g threshold: %d, 5g threshold %d\n", global->dev.rssi_threshold_2g, global->dev.rssi_threshold_5g);
	return 0;
}

int mapd_set_acl_block(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	unsigned char type = 0;
	unsigned char block_flag = 0;
	unsigned char cli_mac[ETH_ALEN]={0};
	unsigned char bssid[ETH_ALEN]={0};
	unsigned char al_mac[ETH_ALEN]={0};
	struct cli_assoc_control *cli_assoc = (struct cli_assoc_control *)os_malloc(sizeof(struct cli_assoc_control) + ETH_ALEN);
	struct _1905_map_device *_1905_device = NULL;
	struct _1905_map_device *own_1905 = NULL;
	struct own_1905_device *global2 = &global->dev;
	int len=0;
	unsigned short duration=0;
	u8 already_seen = 0;
	char * ptmp = NULL;

	if(cli_assoc == NULL) {
		err("Alloc failed for cli assoc");
		return -1;
	}
	ptmp = strtok_r(cmd_buf, " ", &cmd_buf);
	type = strtoul(ptmp, &ptmp, 10);
	ptmp = strtok_r(cmd_buf, " ", &cmd_buf);
	block_flag = strtol(ptmp, &ptmp, 10);
	if (block_flag == 0) {
		ptmp = strtok_r(cmd_buf, " ", &cmd_buf);
		duration = strtol(ptmp, &ptmp, 10);
	}
	debug("type: %d block_flag: %d duration: %d", type, block_flag, duration);
	if (hwaddr_aton(cmd_buf + len, cli_mac) < 0) {
		os_free(cli_assoc);
		return -1;
	}
	len += 3*ETH_ALEN;
	err("cli mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(cli_mac));
	_1905_device = topo_srv_get_next_1905_device(&global->dev, NULL); /*Get own 1905 device*/
	if(_1905_device == NULL) {
		os_free(cli_assoc);
		return -1;
	}
	own_1905 = topo_srv_get_next_1905_device(&global->dev, NULL);
	if(own_1905 == NULL) {
		os_free(cli_assoc);
		return -1;
	}
	if(type == 0) {
		while(_1905_device) {
			struct bss_info_db *map_bss = topo_srv_get_next_bss(_1905_device, NULL);
			if (_1905_device->in_network == 1) {
				while (map_bss != NULL) {
					if(os_memcmp(_1905_device->_1905_info.al_mac_addr, own_1905->_1905_info.al_mac_addr, ETH_ALEN) != 0) {
						map_1905_Send_Client_Association_Control_Request_Message(global->_1905_ctrl,
							(char *)_1905_device->_1905_info.al_mac_addr, map_bss->bssid, block_flag,
							duration, 1, cli_mac);
					} else {
						struct mapd_bss *my_bss = NULL;
						u32 client_id = 0;
						struct client *cli = NULL;
						err("cli bssid mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(map_bss->bssid));
						err("cli sta mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(cli_mac));
						my_bss = mapd_get_bss_from_mac(global, map_bss->bssid);
						client_id = client_db_track_add(global, cli_mac, &already_seen);
						if (client_id == (uint32_t)-1) {
								mapd_printf(MSG_ERROR, "No more room to accomodate " MACSTR
												, MAC2STR(cli_mac));
								os_free(cli_assoc);
								return -1;
						}
						cli = &global->dev.client_db[client_id];
						if (!my_bss || !cli) {
							mapd_printf(MSG_ERROR, "my_bss/cli NULL; my_bss=%p cli=%p",
											my_bss, cli);
							os_free(cli_assoc);
							return -1;
						}
						cli->in_db = IN_DB;
						if (already_seen != 1) {
							mapd_printf(MSG_DEBUG, "New Client discovered"
											MACSTR, MAC2STR(cli_mac));
							cli->dirty = 1;
						}
						if(!block_flag) // block
							client_mon_block_cli_on_bss(global, my_bss, cli,
											BL_MAP_ASSOC_CONTROL, duration,
											global2->al_mac);
						else
							client_mon_unblock_cli_on_bss(global, my_bss, cli,
											BL_MAP_ASSOC_CONTROL, global2->al_mac);
					}
					map_bss = topo_srv_get_next_bss(_1905_device, map_bss);
				}
			}
			_1905_device = topo_srv_get_next_1905_device(&global->dev,_1905_device);
		}
	} else if (type == 1) {
		if (hwaddr_aton(cmd_buf + len, al_mac) < 0) {
			os_free(cli_assoc);
			return -1;
		}
		len += 3*ETH_ALEN;
		debug("al mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(al_mac));
		_1905_device = topo_srv_get_1905_device(&global->dev, al_mac);
		if(_1905_device != NULL && _1905_device->in_network == 1) {
			struct bss_info_db *map_bss = topo_srv_get_next_bss(_1905_device, NULL);
				if(os_memcmp(_1905_device->_1905_info.al_mac_addr, own_1905->_1905_info.al_mac_addr, ETH_ALEN) != 0) {
					while (map_bss != NULL) {
						map_1905_Send_Client_Association_Control_Request_Message(global->_1905_ctrl,
							(char *)_1905_device->_1905_info.al_mac_addr, map_bss->bssid, block_flag,
							duration, 1, cli_mac);
						map_bss = topo_srv_get_next_bss(_1905_device, map_bss);
					}
				} else {
					while (map_bss != NULL) {
							struct mapd_bss *my_bss = NULL;
							u32 client_id = 0;
							struct client *cli = NULL;
							err("cli bssid mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(map_bss->bssid));
							err("cli sta mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(cli_mac));
							my_bss = mapd_get_bss_from_mac(global, map_bss->bssid);
							client_id = client_db_track_add(global, cli_mac, &already_seen);
							if (client_id == (uint32_t)-1) {
									mapd_printf(MSG_ERROR, "No more room to accomodate " MACSTR
													, MAC2STR(cli_mac));
									os_free(cli_assoc);
									return -1;
							}
							cli = &global->dev.client_db[client_id];
							if (!my_bss || !cli) {
								mapd_printf(MSG_ERROR, "my_bss/cli NULL; my_bss=%p cli=%p",
											my_bss, cli);
								os_free(cli_assoc);
								return -1;
							}
							cli->in_db = IN_DB;
							if (already_seen != 1) {
								mapd_printf(MSG_DEBUG, "New Client discovered"
												MACSTR, MAC2STR(cli_mac));
								cli->dirty = 1;
							}
							if(!block_flag) // block
									client_mon_block_cli_on_bss(global, my_bss, cli, BL_MAP_ASSOC_CONTROL, duration, global2->al_mac);
							else
									client_mon_unblock_cli_on_bss(global, my_bss, cli, BL_MAP_ASSOC_CONTROL, global2->al_mac);
							map_bss = topo_srv_get_next_bss(_1905_device, map_bss);
					}
				}
		}
	} else {
		if (hwaddr_aton(cmd_buf + len, bssid) < 0) {
			os_free(cli_assoc);
			return -1;
		}
		len += 3*ETH_ALEN;
		debug("bssid: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(bssid));
		_1905_device = topo_srv_get_1905_by_bssid(&global->dev, bssid);
		if(_1905_device != NULL && _1905_device->in_network == 1 &&
			(os_memcmp(_1905_device->_1905_info.al_mac_addr, own_1905->_1905_info.al_mac_addr, ETH_ALEN) != 0)) {
			map_1905_Send_Client_Association_Control_Request_Message(global->_1905_ctrl,
				(char *)_1905_device->_1905_info.al_mac_addr, bssid, block_flag,
				duration, 1, cli_mac);
		} else {
			struct mapd_bss *my_bss = NULL;
			u32 client_id = 0;
			struct client *cli = NULL;
			err("cli bssid mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(bssid));
			err("cli sta mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(cli_mac));
			my_bss = mapd_get_bss_from_mac(global, bssid);
			client_id = client_db_track_add(global, cli_mac, &already_seen);
			if (client_id == (uint32_t)-1) {
				mapd_printf(MSG_ERROR, "No more room to accomodate " MACSTR
						, MAC2STR(cli_mac));
				if (cli_assoc)
					os_free(cli_assoc);
				return -1;
			}
			cli = &global->dev.client_db[client_id];
			if (!my_bss || !cli) {
				if (cli_assoc)
					os_free(cli_assoc);
				return -1;
			}
			cli->in_db = IN_DB;
			if (already_seen != 1) {
					mapd_printf(MSG_DEBUG, "New Client discovered"
									MACSTR, MAC2STR(cli_mac));
					cli->dirty = 1;
			}
			if(!block_flag) // block
				client_mon_block_cli_on_bss(global, my_bss, cli, BL_MAP_ASSOC_CONTROL, duration, global2->al_mac);
			else
				client_mon_unblock_cli_on_bss(global, my_bss, cli, BL_MAP_ASSOC_CONTROL, global2->al_mac);
			os_memset(cli_assoc, 0, sizeof(struct cli_assoc_control));
		}
	}
	if(cli_assoc)
		os_free(cli_assoc);
	return 0;
}

int mapd_trigger_ap_selection_bh(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	struct _1905_map_device *dev = NULL;
	struct bh_link_entry *bh_entry;
	char cmd[256] = {0};

	err("cmd buf is %s", cmd_buf);
	SLIST_FOREACH(bh_entry, &(global->dev.bh_link_head), next_bh_link)
		if (os_strncmp(cmd_buf, (const char *)bh_entry->ifname, os_strlen((const char *)bh_entry->ifname)) == 0)
			break;
		else
			err("ifname is %s", bh_entry->ifname);

	if (!bh_entry) {
		err("failed to get bh here");
		return -1;
	}

	err("iface name is %s", bh_entry->ifname);

	dev = topo_srv_get_1905_device(&global->dev, NULL);
	if (dev == NULL)
		return -1;

	/* TODO remove it later */
	global->dev.current_bh_state = BH_STATE_WIFI_BH_STEER;
        os_memset(cmd, 0, 256);
	err("traget bssid is %s", cmd_buf + strlen((const char *)bh_entry->ifname));
        sprintf(cmd, "iwpriv %s set ApCliBssid=%s;", bh_entry->ifname, cmd_buf + strlen((const char *)bh_entry->ifname) + 1);
	err("cmd buf is %s", cmd);
	system(cmd);
	return 0;
}

int mapd_trigger_wps(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	struct _1905_map_device *dev = NULL;
	struct bss_info_db *bss = NULL;
	struct iface_info *iface = NULL;
	u8 found = 0;
	struct trigger_wps_param wps;
	struct timeval tv;
	size_t len = buf_len;

	os_memset(&wps, 0 , sizeof(struct trigger_wps_param));
	if (hwaddr_aton(cmd_buf, wps.if_mac) < 0)
		return -1;

	if (os_strcmp(cmd_buf + 18, "PBC") == 0)
		wps.mode = 2;
	else
		return -1;

	dev = topo_srv_get_1905_device(&global->dev, NULL);
	if (dev == NULL)
		return -1;

	SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
		/*local wps trigger*/
		mapd_printf(MSG_ERROR, "bssid=%02x:%02x:%02x:%02x:%02x:%02x, target=%02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(bss->bssid), PRINT_MAC(wps.if_mac));
		if (os_memcmp(bss->bssid, wps.if_mac, ETH_ALEN) == 0) {
			found = 1;
			break;
		}
	}

	if (found == 0) {
		SLIST_FOREACH(iface, &dev->_1905_info.first_iface, next_iface) {
			mapd_printf(MSG_ERROR, "iface mac=%02x:%02x:%02x:%02x:%02x:%02x, target=%02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(iface->iface_addr), PRINT_MAC(wps.if_mac));
			if (os_memcmp(iface->iface_addr, wps.if_mac, ETH_ALEN) == 0) {
				found = 1;
				break;
			}
		}
	}
	if (found == 1) {
		if (wlanif_issue_wapp_command(global, WAPP_USER_TRIGGER_WPS, 0, wps.if_mac,
				wps.if_mac, &wps, sizeof(struct trigger_wps_param), 0, 0, 0) < 0)
			return -1;

		tv.tv_sec = 3;
		tv.tv_usec = 0;
		if (wapp_usr_intf_ctrl_pending(global->wapp_ctrl, &tv)) {
			if(wapp_usr_intf_ctrl_recv(global->wapp_ctrl, buf, &len) < 0) {
				return -1;
			}
		} else
			return -1;
	} else {
		/*remote wps trigger*/
		/*
		field		length		value
		sub-type	1 octet		9
		sub-length	1 octet		6
		sub-value	6 octets	BSS mac
		*/
		struct tlv_head *tlv = (struct tlv_head*)buf;
		unsigned char* p = NULL;

		dev = topo_srv_get_1905_by_iface_addr(&global->dev, wps.if_mac);
		if (dev == NULL) {
			mapd_printf(MSG_ERROR, "remote device not found, ifmac(%02x:%02x:%02x:%02x:%02x:%02x)",
				PRINT_MAC(wps.if_mac));
			return -1;
		}
		tlv->tlv_type = TLV_802_11_VENDOR_SPECIFIC;
		os_memcpy(tlv->oui, MTK_OUI, OUI_LEN);
		tlv->func_type = FUNC_VENDOR_TRIGER_WPS;
		p = (unsigned char *)buf + sizeof(struct tlv_head);
		*p++ = ETH_ALEN;
		os_memcpy(p, wps.if_mac, ETH_ALEN);

		tlv->tlv_len = OUI_LEN + REMOTE_WPS_VENDOR_LEN;
		tlv->tlv_len = host_to_be16(tlv->tlv_len);

		if (map_1905_Send_Vendor_Specific_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr,
			buf, (unsigned short)sizeof(struct tlv_head) + 7) < 0) {
			mapd_printf(MSG_ERROR, "remote device trigger fail, ifmac(%02x:%02x:%02x:%02x:%02x:%02x)",
				PRINT_MAC(wps.if_mac));
			return -1;
		}
		mapd_printf(MSG_DEBUG, "remote wps trigger");

		os_snprintf(buf, buf_len, "OK");
		len = 2;
	}
	return len;
}

#ifdef ACL_CTRL
void mapd_acl_record_network_cmd(struct mapd_global *global, u8 *sta_mac, u8 cmd)
{
	struct own_1905_device *dev = &global->dev;
	struct acl_cli *acl_sta = NULL;
	u8 match = FALSE;

	if (cmd == ACL_ADD || cmd == ACL_DEL) {
		if (is_zero_ether_addr(sta_mac))
		return;

		dl_list_for_each(acl_sta, &dev->acl_cli_list, struct acl_cli, list_entry) {
			if (!os_memcmp(acl_sta->mac_addr, sta_mac, ETH_ALEN)) {
				mapd_printf(MSG_DEBUG, "already added\n");
				match = TRUE;
				break;
			}
		}

		if ((match == TRUE && cmd == ACL_ADD) || (match == FALSE && cmd == ACL_DEL))
			return;

		if(cmd == ACL_ADD) {
			acl_sta = (struct acl_cli *)malloc(sizeof(struct acl_cli));
			if(acl_sta == NULL) {
				mapd_printf(MSG_ERROR,"memory alloc fail");
				return;
			}
			os_memset(acl_sta, 0, sizeof(struct acl_cli));
			os_memcpy(acl_sta->mac_addr, sta_mac, ETH_ALEN);
			dl_list_add(&dev->acl_cli_list, &acl_sta->list_entry);
		} else {
			if (acl_sta && !os_memcmp(acl_sta->mac_addr, sta_mac, ETH_ALEN)) {
				dl_list_del(&acl_sta->list_entry);
				os_free(acl_sta);
			}
		}
	} else if (cmd == ACL_FLUSH) {
		struct acl_cli *acl_sta_next = NULL;
		dl_list_for_each_safe(acl_sta, acl_sta_next, &dev->acl_cli_list, struct acl_cli, list_entry) {
			mapd_printf(MSG_DEBUG, "delete mac mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(acl_sta->mac_addr));
			dl_list_del(&acl_sta->list_entry);
			os_free(acl_sta);
		}
	} else if (cmd == ACL_SHOW) {
		mapd_printf(MSG_ERROR, "Acl List for all n/w acl_policy:%d\n", dev->acl_policy);
		if (dl_list_empty(&dev->acl_cli_list)) {
			err("List empty \n");
		} else {
			dl_list_for_each(acl_sta, &dev->acl_cli_list, struct acl_cli, list_entry) {
				mapd_printf(MSG_ERROR,"\t %02x:%02x:%02x:%02x:%02x:%02x \n", PRINT_MAC(acl_sta->mac_addr));
			}
		}
	} else if (cmd == ACL_POLICY_0 || cmd == ACL_POLICY_1 || cmd == ACL_POLICY_2){
		if (cmd == ACL_POLICY_2)
			dev->acl_policy = 2;
		else if(cmd == ACL_POLICY_1)
			dev->acl_policy = 1;
		else
			dev->acl_policy = 0;
	}
}

void mapd_acl_update_bss_topology(struct mapd_global *global, u8 *sta_mac,
		struct bss_info_db *bss, u8 cmd)
{
	struct acl_cli *acl_sta = NULL;
	u8 match = FALSE;
	if (!bss) {
		return;
	}

	if (cmd == ACL_ADD || cmd == ACL_DEL) {
		dl_list_for_each(acl_sta, &bss->acl_cli_list, struct acl_cli, list_entry) {
			if (!os_memcmp(acl_sta->mac_addr, sta_mac, ETH_ALEN)) {
				mapd_printf(MSG_DEBUG, "already added\n");
				match = TRUE;
				break;
			}
		}

		if ((match == TRUE && cmd == ACL_ADD) || (match == FALSE && cmd == ACL_DEL))
			return;

		if(cmd == ACL_ADD) {
			mapd_printf(MSG_DEBUG, "Add ("MACSTR") ACLList ON BSSID=" MACSTR, MAC2STR(sta_mac), MAC2STR(bss->bssid));
			acl_sta = (struct acl_cli *)malloc(sizeof(struct acl_cli));
			if(acl_sta == NULL) {
				mapd_printf(MSG_ERROR,"memory alloc fail");
				return;
			}
			os_memset(acl_sta, 0, sizeof(struct acl_cli));
			os_memcpy(acl_sta->mac_addr, sta_mac, ETH_ALEN);
			dl_list_add(&bss->acl_cli_list, &acl_sta->list_entry);
		} else {
			mapd_printf(MSG_DEBUG, "remove ("MACSTR") ACLList ON BSSID=" MACSTR, MAC2STR(sta_mac), MAC2STR(bss->bssid));
			if (acl_sta && !os_memcmp(acl_sta->mac_addr, sta_mac, ETH_ALEN)) {
				dl_list_del(&acl_sta->list_entry);
				os_free(acl_sta);
			}
		}
	} else if (cmd == ACL_FLUSH) {
		struct acl_cli *acl_sta_next = NULL;
		mapd_printf(MSG_DEBUG, "flush acl list for bssid mac: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(bss->bssid));
		dl_list_for_each_safe(acl_sta, acl_sta_next, &bss->acl_cli_list, struct acl_cli, list_entry) {
			mapd_printf(MSG_DEBUG, "delete mac mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(acl_sta->mac_addr));
			dl_list_del(&acl_sta->list_entry);
			os_free(acl_sta);
		}
	} else if (cmd == ACL_POLICY_0 || cmd == ACL_POLICY_1 || cmd == ACL_POLICY_2){
		if (cmd == ACL_POLICY_2)
			bss->acl_policy = 2;
		else if(cmd == ACL_POLICY_1)
			bss->acl_policy = 1;
		else
			bss->acl_policy = 0;
		mapd_printf(MSG_DEBUG, "success acl cmd %d for bssid mac: %02x:%02x:%02x:%02x:%02x:%02x\n", cmd, PRINT_MAC(bss->bssid));
	}
}

void map_1905_Send_Acl_Ctrl_Message(struct _1905_context *ctx,
		char *almac, unsigned char *bssid,
		unsigned char type,
		unsigned char cmd,
		unsigned char sta_cnt,
		unsigned char *sta_list)
{
	struct acl_ctrl_tlv *acl_ctrl_msg = NULL;

	acl_ctrl_msg = malloc(sizeof(struct acl_ctrl_tlv) + ETH_ALEN*sta_cnt);
	if(acl_ctrl_msg == NULL) {
		mapd_printf(MSG_ERROR,"%s: Cannot allocate memory\n", __func__);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_DEBUG, "Type:%d cmd:%d sta_cnt:%d \n", type, cmd, sta_cnt);
	mapd_printf(MSG_DEBUG, "send acl ctrl msg t0 bssid:%02x:%02x:%02x:%02x:%02x:%02x \n", PRINT_MAC(bssid));
	mapd_printf(MSG_DEBUG, "send acl ctrl msg t0 almac:%02x:%02x:%02x:%02x:%02x:%02x \n", PRINT_MAC(almac));

	acl_ctrl_msg->tlv.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	os_memcpy(acl_ctrl_msg->tlv.oui, MTK_OUI, OUI_LEN);
	acl_ctrl_msg->tlv.func_type = FUNC_VENDOR_ACL_CTRL;

	acl_ctrl_msg->acl_info.type = type;
	acl_ctrl_msg->acl_info.cmd = cmd;
	os_memcpy(acl_ctrl_msg->acl_info.bssid, bssid, ETH_ALEN);
	acl_ctrl_msg->acl_info.sta_list_count = sta_cnt;
	if (sta_cnt) {
		os_memcpy(acl_ctrl_msg->acl_info.sta_mac, sta_list, ETH_ALEN*sta_cnt);
	}

	acl_ctrl_msg->tlv.tlv_len = ACL_CTRL_TLV_LEN + ETH_ALEN*sta_cnt;
	acl_ctrl_msg->tlv.tlv_len = host_to_be16(acl_ctrl_msg->tlv.tlv_len);

	if (map_1905_Send_Vendor_Specific_Message(ctx, almac,
		(char *)acl_ctrl_msg, (ACL_CTRL_TLV_LEN + ETH_ALEN*sta_cnt)) < 0) {
		err("unable to send acl ctrl msg");
		return;
	}
	return;
}

void mapd_acl_ctrl_for_bss(struct mapd_global *global, u8 *sta_mac,
		struct bss_info_db *bss, u8 cmd)
{
	struct acl_cli *acl_sta = NULL;
	unsigned char *bssid = NULL;
	u8 match = FALSE;

	if (!bss) {
		mapd_printf(MSG_ERROR, "my_bss NULL\n");
		return ;
	}

	bssid = bss->bssid;
	mapd_printf(MSG_DEBUG, "cli bssid mac: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(bssid));

	if (cmd == ACL_ADD || cmd == ACL_DEL) {
		mapd_printf(MSG_DEBUG, "cli sta mac: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sta_mac));
		dl_list_for_each(acl_sta, &bss->acl_cli_list, struct acl_cli, list_entry) {
			if (!os_memcmp(acl_sta->mac_addr, sta_mac, ETH_ALEN)) {
				mapd_printf(MSG_DEBUG, "already added\n");
				match = TRUE;
				break;
			}
		}

		if ((match == TRUE && cmd == ACL_ADD) || (match == FALSE && cmd == ACL_DEL))
			return;

		if (wlanif_acl_ctrl_for_bss(global, sta_mac, bssid, cmd) == 0) {
			if(cmd == ACL_ADD) {
				mapd_printf(MSG_OFF, "Add ("MACSTR") ACLList ON BSSID=" MACSTR, MAC2STR(sta_mac), MAC2STR(bssid));
				acl_sta = (struct acl_cli *)malloc(sizeof(struct acl_cli));
				if(acl_sta == NULL) {
					mapd_printf(MSG_ERROR,"memory alloc fail");
					return;
				}
				os_memset(acl_sta, 0, sizeof(struct acl_cli));
				os_memcpy(acl_sta->mac_addr, sta_mac, ETH_ALEN);
				dl_list_add(&bss->acl_cli_list, &acl_sta->list_entry);
			} else {
				mapd_printf(MSG_OFF, "remove ("MACSTR") ACLList ON BSSID=" MACSTR, MAC2STR(sta_mac), MAC2STR(bssid));
				if (acl_sta && !os_memcmp(acl_sta->mac_addr, sta_mac, ETH_ALEN)) {
					dl_list_del(&acl_sta->list_entry);
					os_free(acl_sta);
				}
			}
		} else {
			mapd_printf(MSG_ERROR, "(" MACSTR ") ACL CTRL ON BSSID="MACSTR " FAILED\n",
				MAC2STR(sta_mac), MAC2STR(bss->bssid));
		}
	}
	else if (cmd == ACL_FLUSH) {
		mapd_printf(MSG_DEBUG, "flush acl list for bssid mac: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(bssid));
		if (wlanif_acl_ctrl_for_bss(global, sta_mac, bssid, cmd) == 0){
			struct acl_cli *acl_sta_next = NULL;
			dl_list_for_each_safe(acl_sta, acl_sta_next, &bss->acl_cli_list, struct acl_cli, list_entry) {
				mapd_printf(MSG_ERROR, "delete mac mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(acl_sta->mac_addr));
				dl_list_del(&acl_sta->list_entry);
				os_free(acl_sta);
			}
		} else {
			err("fail acl flush for bssid mac: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(bssid));
		}
	}
	else if (cmd == ACL_SHOW) {
		mapd_printf(MSG_ERROR, "Acl List on bssid mac: %02x:%02x:%02x:%02x:%02x:%02x  acl_policy:%d\n", PRINT_MAC(bssid), bss->acl_policy);
		if (dl_list_empty(&bss->acl_cli_list)) {
			err("List empty \n");
		} else {
			dl_list_for_each(acl_sta, &bss->acl_cli_list, struct acl_cli, list_entry) {
				mapd_printf(MSG_ERROR,"\t %02x:%02x:%02x:%02x:%02x:%02x \n", PRINT_MAC(acl_sta->mac_addr));
			}
		}
	}
	else if (cmd == ACL_POLICY_0 || cmd == ACL_POLICY_1 || cmd == ACL_POLICY_2){
#if 0
	/* add apcli addr of agent to controller's ap whitelist to avoid breaking of n/w */
		if (cmd == ACL_POLICY_1) {
			struct iface_info *iface;
			struct bss_info_db *map_bss = NULL;
			struct _1905_map_device *_1905_own_dev = topo_srv_get_next_1905_device(&global->dev, NULL);
			struct _1905_map_device *_1905_dev = topo_srv_get_next_1905_device(&global->dev, NULL);

			SLIST_FOREACH(iface, &(_1905_own_dev->_1905_info.first_iface), next_iface) {
				if (iface->is_map_if == 1
					&& (iface->media_type >= ieee_802_11_b && iface->media_type <= ieee_802_11_ax)
					&& iface->media_info.role == 0x00) {
					map_bss = topo_srv_get_bss_by_bssid(&global->dev, _1905_own_dev, iface->iface_addr);
					mapd_printf(MSG_OFF,"\t inf addr %02x:%02x:%02x:%02x:%02x:%02x \n", PRINT_MAC(iface->iface_addr));
				}
			}

			while (map_bss && _1905_dev) {
				if (_1905_dev != _1905_own_dev) {
					SLIST_FOREACH(iface, &(_1905_dev->_1905_info.first_iface), next_iface) {
						if (iface->is_map_if == 1
							&& (iface->media_type >= ieee_802_11_b && iface->media_type <= ieee_802_11_ax)
							&& iface->media_info.role == 0x4) {
							mapd_acl_ctrl_for_bss(global, iface->iface_addr, map_bss, ACL_ADD);
						}
					}
				}
				_1905_dev = topo_srv_get_next_1905_device(&global->dev, _1905_dev);
			}
		}
#endif
		if (wlanif_acl_ctrl_for_bss(global, sta_mac, bssid, cmd) == 0){
			if (cmd == ACL_POLICY_2)
				bss->acl_policy = 2;
			else if(cmd == ACL_POLICY_1)
				bss->acl_policy = 1;
			else
				bss->acl_policy = 0;
			mapd_printf(MSG_DEBUG, "success acl cmd %d for bssid mac: %02x:%02x:%02x:%02x:%02x:%02x\n", cmd, PRINT_MAC(bssid));
		} else {
			mapd_printf(MSG_ERROR, "fail acl cmd %d for bssid mac: %02x:%02x:%02x:%02x:%02x:%02x\n", cmd, PRINT_MAC(bssid));
		}
	} else {
		err("Invalid acl cmd : %d\n", cmd);
	}
}

int mapd_set_acl_ctrl(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	unsigned char type = 0;
	unsigned char cmd = 0;
	unsigned char cli_mac[ETH_ALEN]={0};
	unsigned char bssid[ETH_ALEN]={0};
	unsigned char al_mac[ETH_ALEN]={0};
	struct _1905_map_device *_1905_device = NULL;
	struct _1905_map_device *own_1905 = NULL;
	struct bss_info_db *map_bss = NULL;
	int len=0;
	char * ptmp = NULL;
	struct iface_info *iface;
	unsigned char sta_list[256] = {0};
	unsigned char idx, sta_cnt = 0;
	u8 dummy_cmd = 0;

	ptmp = strtok_r(cmd_buf, " ", &cmd_buf);
	type = strtoul(ptmp, &ptmp, 10);
	ptmp = strtok_r(cmd_buf, " ", &cmd_buf);
	cmd = strtol(ptmp, &ptmp, 10);

	mapd_printf(MSG_DEBUG, "type: %d cmd: %d", type, cmd);

	if (cmd == ACL_ADD || cmd == ACL_DEL) {
		if (hwaddr_aton(cmd_buf + len, cli_mac) < 0) {
			return -1;
		}
		len += 3*ETH_ALEN;
		mapd_printf(MSG_DEBUG, "cli mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(cli_mac));
	}

	_1905_device = topo_srv_get_next_1905_device(&global->dev, NULL);
	if(_1905_device == NULL) {
		return -1;
	}
	own_1905 = topo_srv_get_next_1905_device(&global->dev, NULL);/*Get own 1905 device*/
	if(own_1905 == NULL) {
		return -1;
	}

	if(type == ACL_FUNC_ALL_DEV) {
		sta_cnt = 0;
		/* record n/w ACL cmd */
		if (own_1905->device_role == DEVICE_ROLE_CONTROLLER) {
			mapd_acl_record_network_cmd(global, cli_mac, cmd);
			/* For whitelist ACL policy add all agent's BH MAC to all FHBSS of n/w*/
			/* Prepare list of all agents BH MAC */
			if (cmd == ACL_POLICY_1) {
				dummy_cmd = ACL_ADD;
				while(_1905_device) {
					if(_1905_device->device_role == DEVICE_ROLE_AGENT) {
						SLIST_FOREACH(iface, &(_1905_device->_1905_info.first_iface), next_iface) {
							if ((iface->media_type >= ieee_802_11_b && iface->media_type <= ieee_802_11_ax)
								&& iface->media_info.role == 0x4) {
								os_memcpy(sta_list + (ETH_ALEN*sta_cnt), iface->iface_addr, ETH_ALEN);
								sta_cnt++;
								mapd_acl_record_network_cmd(global, iface->iface_addr, dummy_cmd);
							}
						}
					}
					_1905_device = topo_srv_get_next_1905_device(&global->dev,_1905_device);
				}
			}
		}

		_1905_device = topo_srv_get_next_1905_device(&global->dev, NULL);
		while(_1905_device) {
			if(os_memcmp(_1905_device->_1905_info.al_mac_addr, own_1905->_1905_info.al_mac_addr, ETH_ALEN) != 0) {
				if (_1905_device->in_network == 1) {
					if (cmd == ACL_POLICY_1 && sta_cnt != 0)
						map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,
								bssid, type, dummy_cmd, sta_cnt, sta_list);
					map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,
							bssid, type, cmd, 1, cli_mac);
				}
			} else {
				map_bss = topo_srv_get_next_bss(own_1905, NULL);
				while (map_bss != NULL) {
					if (cmd == ACL_POLICY_1 && sta_cnt != 0) {
						for (idx = 0; idx < sta_cnt; idx++)
							mapd_acl_ctrl_for_bss(global, sta_list + (ETH_ALEN*idx), map_bss, dummy_cmd);
					}
					mapd_acl_ctrl_for_bss(global, cli_mac, map_bss, cmd);
					map_bss = topo_srv_get_next_bss(own_1905, map_bss);
				}
			}
			_1905_device = topo_srv_get_next_1905_device(&global->dev,_1905_device);
		}
	} else if (type == ACL_FUNC_DEV) {
		if (hwaddr_aton(cmd_buf + len, al_mac) < 0) {
			return -1;
		}
		len += 3*ETH_ALEN;
		mapd_printf(MSG_DEBUG, "al mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(al_mac));
		_1905_device = topo_srv_get_1905_device(&global->dev, al_mac);
		if(_1905_device) {
			if(os_memcmp(_1905_device->_1905_info.al_mac_addr, own_1905->_1905_info.al_mac_addr, ETH_ALEN) != 0) {
				if (_1905_device->in_network == 1)
					map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,
						bssid, type, cmd,  1, cli_mac);
				if (own_1905->device_role == DEVICE_ROLE_CONTROLLER) {
					map_bss = topo_srv_get_next_bss(_1905_device, NULL);
					while (map_bss != NULL) {
						mapd_acl_update_bss_topology(global, cli_mac, map_bss, cmd);
						map_bss = topo_srv_get_next_bss(_1905_device, map_bss);
					}
				}
			} else {
				map_bss = topo_srv_get_next_bss(own_1905, NULL);
				while (map_bss != NULL) {
					mapd_acl_ctrl_for_bss(global, cli_mac, map_bss, cmd);
					map_bss = topo_srv_get_next_bss(own_1905, map_bss);
				}
			}
		}
	} else if (type == ACL_FUNC_BSSID) {
		if (hwaddr_aton(cmd_buf + len, bssid) < 0) {
			return -1;
		}
		len += 3*ETH_ALEN;
		mapd_printf(MSG_DEBUG, "bssid: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(bssid));
		_1905_device = topo_srv_get_1905_by_bssid(&global->dev, bssid);
		map_bss = topo_srv_get_bss_by_bssid(&global->dev, _1905_device, bssid);
		if(_1905_device && (os_memcmp(_1905_device->_1905_info.al_mac_addr, own_1905->_1905_info.al_mac_addr, ETH_ALEN) != 0)) {
				if (_1905_device->in_network == 1)
					map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,
						bssid, type, cmd,  1, cli_mac);
				if (own_1905->device_role == DEVICE_ROLE_CONTROLLER)
					mapd_acl_update_bss_topology(global, cli_mac, map_bss, cmd);
		} else {
			mapd_acl_ctrl_for_bss(global, cli_mac, map_bss, cmd);
		}
	} else {
		err("unknow type:%d \n", type);
	}

	return 0;
}

int handle_acl_ctrl_msg(struct mapd_global *global, struct acl_ctrl_tlv *acl_ctrl_tlv, struct _1905_map_device *p1905_device)
{

	struct acl_ctrl *acl_info = &acl_ctrl_tlv->acl_info;
	struct _1905_map_device *own_1905 = NULL;
	struct bss_info_db *map_bss = NULL;
	struct acl_cli *acl_sta = NULL;
	unsigned char sta_mac[6] = {0};
	unsigned char *p_mac;
	u8 cnt = 0;

	own_1905 = topo_srv_get_next_1905_device(&global->dev, NULL);/*Get own 1905 device*/
	if (!own_1905) {
		err("own map dev not found\n");
		return -1;
	}

	if ((acl_info->type == ACL_FUNC_ALL_DEV) || (acl_info->type == ACL_FUNC_DEV)) {
		map_bss = topo_srv_get_next_bss(own_1905, NULL);
		while (map_bss != NULL) {
			//mapd_acl_ctrl_for_bss(global, acl_info->sta_mac, map_bss, acl_info->cmd);
			if (acl_info->sta_list_count > 0) {
				for (cnt = 0; cnt < acl_info->sta_list_count; cnt++) {
					p_mac = acl_info->sta_mac + cnt*ETH_ALEN;
					mapd_printf(MSG_DEBUG, "cmd:%d sta_mac:%02x:%02x:%02x:%02x:%02x:%02x\n", acl_info->cmd, PRINT_MAC(p_mac));
					mapd_acl_ctrl_for_bss(global, p_mac, map_bss, acl_info->cmd);
				}
			} else {
				mapd_acl_ctrl_for_bss(global, sta_mac, map_bss, acl_info->cmd);
			}
			map_bss = topo_srv_get_next_bss(own_1905, map_bss);
		}
	} else if (acl_info->type == ACL_FUNC_BSSID) {
		map_bss = topo_srv_get_bss_by_bssid(&global->dev, own_1905, acl_info->bssid);
		if (acl_info->sta_list_count > 0) {
			for (cnt = 0; cnt < acl_info->sta_list_count; cnt++) {
				p_mac = acl_info->sta_mac + cnt*ETH_ALEN;
				mapd_printf(MSG_DEBUG, "cmd:%d sta_mac:%02x:%02x:%02x:%02x:%02x:%02x\n", acl_info->cmd, PRINT_MAC(p_mac));
				mapd_acl_ctrl_for_bss(global, p_mac, map_bss, acl_info->cmd);
			}
		} else {
			mapd_acl_ctrl_for_bss(global, sta_mac, map_bss, acl_info->cmd);
		}
	} else if (acl_info->type == ACL_FUNC_REQ) {
		struct own_1905_device *dev = &global->dev;
		unsigned char sta_list[512] = {0};
		unsigned char sta_cnt = 0;
		u8 cmd = 0;

		mapd_printf(MSG_DEBUG,"Send RSP to ACK agent first\n");
		map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)p1905_device->_1905_info.al_mac_addr,
				(unsigned char *)p1905_device->_1905_info.al_mac_addr, ACL_FUNC_RSP, 0, 0, NULL);

		mapd_printf(MSG_DEBUG,"Sync agent specific ACL info only\n");
		map_bss = topo_srv_get_next_bss(p1905_device, NULL);
		while (map_bss != NULL) {
			sta_cnt = 0;
			mapd_printf(MSG_DEBUG, "BSSID %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(map_bss->bssid));
			if (!dl_list_empty(&map_bss->acl_cli_list)) {
				dl_list_for_each(acl_sta, &map_bss->acl_cli_list, struct acl_cli, list_entry) {
					os_memcpy(sta_list + (ETH_ALEN*sta_cnt), acl_sta->mac_addr, ETH_ALEN);
					mapd_printf(MSG_DEBUG, "STA %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(acl_sta->mac_addr));
					sta_cnt++;
					if (sta_cnt >= 64) {
						map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)p1905_device->_1905_info.al_mac_addr,
							map_bss->bssid, ACL_FUNC_BSSID, ACL_ADD, sta_cnt, sta_list);
						sta_cnt = 0;
					}
				}
				map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)p1905_device->_1905_info.al_mac_addr,
							map_bss->bssid, ACL_FUNC_BSSID, ACL_ADD, sta_cnt, sta_list);
			}
			/* update ACL policy */
			if (map_bss->acl_policy == 2)
				cmd = ACL_POLICY_2;
			else if (map_bss->acl_policy == 1)
				cmd = ACL_POLICY_1;
			else
				cmd = ACL_POLICY_0;
			map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)p1905_device->_1905_info.al_mac_addr,
						map_bss->bssid, ACL_FUNC_BSSID, cmd, 0, NULL);
			map_bss = topo_srv_get_next_bss(p1905_device, map_bss);
		}

		mapd_printf(MSG_DEBUG,"Sync n/w specific ACL info \n");
		sta_cnt = 0;
		/* update ACL policy */
		if (dev->acl_policy == 2)
			cmd = ACL_POLICY_2;
		else if (dev->acl_policy == 1)
			cmd = ACL_POLICY_1;
		else
			cmd = ACL_POLICY_0;

		if (!dl_list_empty(&dev->acl_cli_list)) {
			dl_list_for_each(acl_sta, &dev->acl_cli_list, struct acl_cli, list_entry) {
				os_memcpy(sta_list + (ETH_ALEN*sta_cnt), acl_sta->mac_addr, ETH_ALEN);
				mapd_printf(MSG_DEBUG, "STA %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(acl_sta->mac_addr));
				sta_cnt++;
				if (sta_cnt >= 64) {
					map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)p1905_device->_1905_info.al_mac_addr,
						(unsigned char *)p1905_device->_1905_info.al_mac_addr, ACL_FUNC_DEV, ACL_ADD, sta_cnt, sta_list);
					sta_cnt = 0;
				}
			}

			map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)p1905_device->_1905_info.al_mac_addr,
						(unsigned char *)p1905_device->_1905_info.al_mac_addr, ACL_FUNC_DEV, ACL_ADD, sta_cnt, sta_list);

			map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)p1905_device->_1905_info.al_mac_addr,
						(unsigned char *)p1905_device->_1905_info.al_mac_addr, ACL_FUNC_DEV, cmd, 0, NULL);
		}
		else if (cmd != ACL_POLICY_0) {
			map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)p1905_device->_1905_info.al_mac_addr,
						(unsigned char *)p1905_device->_1905_info.al_mac_addr, ACL_FUNC_DEV, cmd, 0, NULL);
		}
	}
	else if (acl_info->type == ACL_FUNC_RSP) {
		mapd_printf(MSG_DEBUG, "ACK to ACL inf REQ from %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(p1905_device->_1905_info.al_mac_addr));
		eloop_cancel_timeout(map_sync_acl_info, global, &global->dev);
	}
	else {
		err("unknow acl type:%d\n", acl_info->type);
	}
	return 0;
}

void map_sync_acl_info(void * eloop_ctx,void * timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *dev = &global->dev;
	struct _1905_map_device *_1905_dev = topo_srv_get_next_1905_device(dev, NULL);

	while (_1905_dev) {
		if (_1905_dev->device_role == DEVICE_ROLE_CONTROLLER) {
			/*Controller found sed acl info req*/
			map_1905_Send_Acl_Ctrl_Message(global->_1905_ctrl, (char *)_1905_dev->_1905_info.al_mac_addr, (unsigned char*)_1905_dev->_1905_info.al_mac_addr,
					ACL_FUNC_REQ, 0, 0, NULL);
			break;
		}
		_1905_dev = topo_srv_get_next_1905_device(dev, _1905_dev);
	}
	eloop_register_timeout(10, 0, map_sync_acl_info, global, dev);
}

/* On new onboarding agent, add BH mac to acl if whitelist enable*/
void mapd_acl_sync_new_agent_info(struct own_1905_device *ctx, struct _1905_map_device *p1905_device)
{
	//struct mapd_global *global = ctx->back_ptr;
	struct iface_info *iface;
	char cmd_buf[64] = {0};
	int res;

	err("ctx policy:%d dev role:%d\n", ctx->acl_policy, p1905_device->device_role);

	if ((ctx->acl_policy == 1) && (p1905_device->device_role == DEVICE_ROLE_AGENT)) {
		SLIST_FOREACH(iface, &(p1905_device->_1905_info.first_iface), next_iface) {
			if ((iface->media_type >= ieee_802_11_b && iface->media_type <= ieee_802_11_ax)
				&& iface->media_info.role == 0x4) {
				res = os_snprintf(cmd_buf, sizeof(cmd_buf), "0 0 %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(iface->iface_addr));
				err("cmd_buf:%s res:%d\n", cmd_buf, res);
				mapd_set_acl_ctrl(ctx->back_ptr, cmd_buf, NULL, 0);
			}
		}
	}
}
#endif /* ACL_CTRL */
#endif
int mapd_get_client_db(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	struct client_db *dbs = (struct client_db*)buf;
	struct client *cli = NULL;
	unsigned char zero_mac[ETH_ALEN] = {0};
	int count = 0, i = 0;
	unsigned char assoc = 0;

	assoc = *cmd_buf - '0';

	count = atoi(cmd_buf + 2);

	count = count > MAX_STA_SEEN ? MAX_STA_SEEN : count;

	count = count > (buf_len / sizeof(struct client_db)) ? (buf_len / sizeof(struct client_db)) : count;

	dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry) {
		if (assoc == 1 && os_memcmp(zero_mac, cli->bssid, ETH_ALEN) == 0) {
			continue;
		}
		os_memcpy(dbs[i].mac, cli->mac_addr, ETH_ALEN);
		os_memcpy(dbs[i].bssid, cli->bssid, ETH_ALEN);
		dbs[i].capab = cli->capab;
		dbs[i].phy_mode = cli->phy_capab.phy_mode[0];
		dbs[i].max_bw[0] = cli->phy_capab.max_bw[0];
		dbs[i].max_bw[1] = cli->phy_capab.max_bw[1];
		dbs[i].spatial_stream = cli->phy_capab.num_sp_streams;
		dbs[i].know_band = cli->known_bands;
		os_memcpy(dbs[i].know_channels, cli->known_channels, sizeof(cli->known_channels));
		i++;
		if (i >= count)
			break;
	}
	buf_len = i * sizeof(struct client_db);

	return buf_len;
}
#ifdef DFS_CAC_R2
void map_send_cac_completion (struct mapd_global *global,
			struct cac_completion_report * report)
{

	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *own_device = topo_srv_get_1905_device(ctx,NULL);
	struct ch_distribution_cb *ch_planning = &ctx->ch_planning.ch_ditribution_5g;
	struct radio_info_db *radio = NULL;
	struct prefered_ch_cb *prefered_ch = NULL;
	SLIST_FOREACH(radio, &own_device->first_radio, next_radio){
		if (radio->channel[0] > 14)
			break;
	}

	SLIST_FOREACH(prefered_ch, &ch_planning->first_prefered_ch, next_prefered_ch)
	{ // TODO: Raghav : check for primary or central.
		if (radio) {
			if (prefered_ch->ch_num == radio->channel[0]) {
				prefered_ch->preference = 0;
				prefered_ch->reason = 0x7;
				break;
			}
		}
	}
	if (ctx->map_version == DEV_TYPE_R2)
		_1905_update_channel_pref_report(ctx, report, NULL);
	else
		_1905_update_channel_pref_report(ctx, NULL, NULL);

}
#endif

#ifdef SUPPORT_MULTI_AP
void mapd_restart_channel_plannig(struct mapd_global *global)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *dev;
	SLIST_FOREACH(dev, &(ctx->_1905_dev_head), next_1905_device) {
		dev->channel_planning_completed = FALSE;
	}
}
#endif
void mapd_handle_stub(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	err("entered mapd_handle_stub");
	mapd_hexdump(MSG_ERROR, "From STUB", cmd_buf, os_strlen(cmd_buf));
	wlanif_process_wapp_events(global, cmd_buf+5, os_strlen(cmd_buf) - 5 +1);

	return;
}

static int mapd_get_sta_seen_list(struct mapd_global *global, char *buf, int  buf_len)
{
	char *end, *pos;
	struct client *cli = NULL;
	int ret = 0;

	mapd_printf(MSG_DEBUG, "*");

	pos = buf;
	end = buf + buf_len;

	ret = os_snprintf(pos, end - pos,
			"STA SEEN List:\n");
	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
		return pos - buf;
	}
	pos = pos + ret;
    /* Dump seen list */
	dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry) {
		ret = os_snprintf(pos, end - pos,
				"\t" MACSTR "\t id = %u \n", MAC2STR(cli->mac_addr),cli->client_id);
		if (os_snprintf_error(end - pos, ret)) {
			*pos = '\0';
			return pos - buf;
		}
		pos += ret;
	}
	return pos - buf;

}

static int mapd_get_bl_lists(struct mapd_global *global, char *buf, int  buf_len)
{
	char *end, *pos;
	struct client *cli = NULL;
	struct bl_client *bl_sta = NULL;
	struct mapd_bss *bss = NULL;
	struct mapd_radio_info *ra_info = NULL;

	int ret = 0;
	u32 i = 0;

	mapd_printf(MSG_DEBUG, "*");

	pos = buf;
	end = buf + buf_len;

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;
		bss = NULL;
		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
				ret = os_snprintf(pos, end - pos,
								"BSSID: " MACSTR "\n", MAC2STR(bss->bssid));
				if (os_snprintf_error(end - pos, ret)) {
						*pos = '\0';
						return pos - buf;
				}
				pos += ret;
				dl_list_for_each(bl_sta, &bss->bl_sta_list, struct bl_client,
								list_entry)
				{
						if(!bl_sta->cli)
							continue;
						cli = bl_sta->cli;
						ret = os_snprintf(pos, end - pos,
										"\t\t\tMAC=" MACSTR "\t\t client_id = %u \t\t Reason=0x%x\n",
										MAC2STR(cli->mac_addr),cli->client_id, bl_sta->bl_reason);
						if (os_snprintf_error(end - pos, ret)) {
								*pos = '\0';
								return pos - buf;
						}
						pos += ret;
				}
		}
	}
	return pos - buf;
}

static int mapd_get_bss_info(struct mapd_global *global, char *buf, int  buf_len)
{
	char *end, *pos;
	struct mapd_bss *bss = NULL;
	struct mapd_radio_info *radio_info = NULL;

	int ret = 0;
	u32 i = 0;

	mapd_printf(MSG_DEBUG, "*");

	pos = buf;
	end = buf + buf_len;

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		u8 safety_th = 0, ol_th = 0;
		radio_info = &global->dev.dev_radio_info[i];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		bss = NULL;
		ap_roam_algo_get_ch_ol_safety_th(&global->dev, radio_info->channel,
						&ol_th, &safety_th);
		ret = os_snprintf(pos, end - pos,
						"RaNum=%d\n \tidx=%d Id=" MACSTR " BssBM=%x channel=%d Util=%d Th(OL/Safety)=%d/%d\n",
						i, radio_info->radio_idx,  MAC2STR(radio_info->identifier),
						radio_info->bss_bitmap, radio_info->channel, radio_info->ch_util,
						ol_th, safety_th);
		if (os_snprintf_error(end - pos, ret)) {
				*pos = '\0';
				return pos - buf;
		}
		pos += ret;
		bss = NULL;
		dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry) {
				ret = os_snprintf(pos, end - pos,
								"\t\t SSID=%s BSSID: " MACSTR " STA CNT=%d\n",
								bss->ssid, MAC2STR(bss->bssid),
								dl_list_len(&bss->assoc_sta_list));
				if (os_snprintf_error(end - pos, ret)) {
						*pos = '\0';
						return pos - buf;
				}
				pos += ret;
		}
	}
	return pos - buf;
}


int mapd_get_mib_options(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)

{
	char *token;
	char client_id;
	mapd_printf(MSG_DEBUG, "*");
	if(os_strncmp(cmd_buf+4,"sta ",4)==0)
	{
		mapd_printf(MSG_DEBUG, "sta command received");
		//parse client id somehow
		token = strtok(cmd_buf+8," ");
		client_id = atoi(token);
		if ((client_id < 0) || (client_id >= MAX_STA_SEEN)) {
			mapd_printf(MSG_ERROR, "Invalid client_id");
			return 0;
		}
		return(mapd_get_mib_sta(global, buf, buf_len, client_id));
	}
	else if(os_strncmp(cmd_buf+4,"bl_list",7)==0)
	{
		return (mapd_get_bl_lists(global, buf, buf_len));
	}
	else if(os_strncmp(cmd_buf+4, "sta_seen_list",13)==0)
	{
		return (mapd_get_sta_seen_list(global, buf, buf_len));
	}
	else if(os_strncmp(cmd_buf + 4, "bss_info", 8) == 0)
	{
		return (mapd_get_bss_info(global, buf, buf_len));
	}
	return 0;
}

const char *max_bw_str(enum max_bw max_bw)
{
	switch (max_bw) {
		case BW_20:
			return "20Mhz";
		case BW_40:
			return "40Mhz";
		case BW_80:
			return "80Mhz";
		case BW_160:
		case BW_8080:
			return "160Mhz";
		default:
			return "Invalid";
	}
}

const char *phy_mode_str(coarse_phy_mode phy_mode)
{
	switch (phy_mode) {
		case LEGACY_MODE:
			return "LEGACY";
		case HT_MODE:
			return "HT";
		case VHT_MODE:
			return "VHT";
		case HE_MODE:
			return "HE";
		default:
			return "Invalid";
	}
}
int mapd_get_mib_sta(struct mapd_global *global, char *buf, int  buf_len, int client_id)
{
    char *end, *pos;
    uint8_t i = 0, channel;
    int ret = 0;
	uint8_t arr_idx;
    struct client *cli = NULL;
	struct mapd_radio_info *ra_info = NULL;
	char phy_caps_2g[20] = "UNKNOWN";
	char phy_caps_5g[20] = "UNKNOWN";
#ifdef CENT_STR
	struct _1905_map_device * own_device = NULL;
	struct associated_clients * client_dev = NULL;
	struct radio_info_db * radio_tmp = NULL;
#endif

	cli = &global->dev.client_db[client_id];

    pos = buf;
    end = buf + buf_len;

	ret = os_snprintf(pos, end - pos,
			"STA ENTRY IN DB:\n");
	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
		return pos - buf;
	}
	pos += ret;

	ret = os_snprintf(pos, end - pos,
			"Client_id=%d \tMAC=" MACSTR "\tBSSID=" MACSTR "\n",
			cli->client_id, MAC2STR(cli->mac_addr), MAC2STR(cli->bssid));
	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
		return pos - buf;
	}
	pos += ret;
	ret = os_snprintf(pos, end - pos,
			"Steer State=%d\tSteer method =%d\n"
			"num_11k=%d\tnum_11k_succ=%d\n",
			cli->cli_steer_state, cli->cli_steer_method,
			cli->steer_stats.num_11k, cli->steer_stats.num_11k_succ);
	if (os_snprintf_error(end - pos, ret)) {
			*pos = '\0';
			return pos - buf;
	}
	pos += ret;

	ret = os_snprintf(pos, end - pos,
			"Local Steer Stats\n");
	if (os_snprintf_error(end - pos, ret)) {
			*pos = '\0';
			return pos - buf;
	}
	pos += ret;

	for (i = 0; i < MAX_NUM_STR_METHODS; i++) {
		ret = os_snprintf(pos, end - pos,
						"\t%-25s: BTM(Attempts/Succ/Fail) = %d/%d/%d"
						" Foced(Attempts/Succ/fail) = %d/%d/%d\n",
						str_method_str(i), cli->steer_stats.steer_attempts_btm[i],
						cli->steer_stats.steer_succ_cnt_btm[i],
						cli->steer_stats.steer_fail_cnt_btm[i],
						cli->steer_stats.steer_attempts_f[i],
						cli->steer_stats.steer_succ_cnt_f[i],
						cli->steer_stats.steer_fail_cnt_f[i]);

		if (os_snprintf_error(end - pos, ret)) {
				*pos = '\0';
				return pos - buf;
		}
		pos += ret;
	}
#ifdef SUPPORT_MULTI_AP
	ret = os_snprintf(pos, end - pos,
			"Remote Steer Stats BTM(Succ/Fail) = %d/%d "
			"Foced(Succ/fail) = %d/%d\n",
			cli->steer_stats.rem_steer_succ_cnt_btm,
			cli->steer_stats.rem_steer_fail_cnt_btm,
			cli->steer_stats.rem_steer_succ_cnt_f,
			cli->steer_stats.rem_steer_fail_cnt_f);

	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
		return pos - buf;
	}
	pos += ret;
#endif
	ret = steer_action_print_csbc_stats(global, cli, pos, end);
	pos += ret;

	if (cli->phy_cap_known[0]) {
		if (os_snprintf(phy_caps_2g, sizeof(phy_caps_2g), "%s/%s",
						phy_mode_str(cli->phy_capab.phy_mode[0]),
						max_bw_str(cli->phy_capab.max_bw[0])) < 0)
			return pos - buf;
	}

	if (cli->phy_cap_known[1]) {
		if (os_snprintf(phy_caps_5g, sizeof(phy_caps_5g), "%s/%s",
						phy_mode_str(cli->phy_capab.phy_mode[1]),
						max_bw_str(cli->phy_capab.max_bw[1])) < 0)
			return pos - buf;
	}

	ret = os_snprintf(pos, end - pos,
			"PHY CAPS: sp=%d 2G:PhyMode/BW=%s 5G:PhyMode/BW=%s\n",
			cli->phy_capab.num_sp_streams,
			phy_caps_2g, phy_caps_5g);

	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
		return pos - buf;
	}
	pos += ret;

	ret = os_snprintf(pos, end - pos,
			"Known channels list \n");
	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
		return pos - buf;
	}
	pos += ret;

	for(i=0; i<MAX_NUM_CHANNELS; ++i)
	{
		arr_idx = i / 8;
		if(cli->known_channels[arr_idx] & BIT(i%8)) {
			channel = idx_to_chan(i);
#ifdef CENT_STR
		if(!global->dev.cent_str_en || global->dev.device_role != DEVICE_ROLE_CONTROLLER)

#endif
		{
			ra_info = mapd_get_radio_from_channel(global, channel);
        	ret = os_snprintf(pos, end - pos,
                	"Channel=%d RSSI=%d(%s)\n", channel,
					ra_info ? cli->ul_rssi[ra_info->radio_idx] : -127,
					(cli->current_chan == channel) ? "current" :  "non-serving");

        	if (os_snprintf_error(end - pos, ret)) {
            	*pos = '\0';
            	return pos - buf;
        	}
        	pos += ret;
#ifdef CENT_STR
		} else {
			int8_t uplink_rssi_non_serving = 0;


			own_device = topo_srv_get_1905_by_bssid(&global->dev, cli->bssid);

			if(!own_device){
				ret = os_snprintf(pos, end - pos,
				"Channel=%d RSSI=%d(%s)\n", channel, -127,
						(cli->current_chan == channel) ? "current" :  "non-serving");
				if (os_snprintf_error(end - pos, ret)) {
					*pos = '\0';
					return pos - buf;
				}
				pos += ret;
					continue;
			}

			radio_tmp = topo_srv_get_radio_by_channel(own_device,channel);

			if(!radio_tmp){
				ret = os_snprintf(pos, end - pos,
				"Channel=%d RSSI=%d(%s)\n", channel, -127,
						(cli->current_chan == channel) ? "current" :  "non-serving");
				if (os_snprintf_error(end - pos, ret)) {
					*pos = '\0';
					return pos - buf;
				}
				pos += ret;
				continue;

			}

			client_dev = topo_srv_get_associate_client(&global->dev,own_device, cli->mac_addr);

			if(!client_dev) {
				ret = os_snprintf(pos, end - pos,
				"Channel=%d RSSI=%d(%s)\n", channel, -127,
						(cli->current_chan == channel) ? "current" :  "non-serving");
				if (os_snprintf_error(end - pos, ret)) {
					*pos = '\0';
					return pos - buf;
				}
				pos += ret;
				continue;

			}

			if(radio_tmp->channel[0] == cli->current_chan){
				ret = os_snprintf(pos, end - pos,
				"Channel=%d RSSI=%d(%s)\n", channel,(signed char)client_dev->rssi_uplink,
						(cli->current_chan == channel) ? "current" :  "non-serving");
				if (os_snprintf_error(end - pos, ret)) {
					*pos = '\0';
					return pos - buf;
				}
				pos += ret;

			} else {
				uplink_rssi_non_serving = ap_est_update_non_serving_rssi_cent_str(global,cli,client_dev->bss->radio->band,radio_tmp->band);
			ret = os_snprintf(pos, end - pos,
			"Channel=%d RSSI=%d(%s)\n", channel, uplink_rssi_non_serving,
					(cli->current_chan == channel) ? "current" :  "non-serving");
			if (os_snprintf_error(end - pos, ret)) {
				*pos = '\0';
				return pos - buf;
			}
			pos += ret;

			}

		}
#endif

		}
	}

	ret = os_snprintf(pos, end - pos,
			"Current chan = %d \tActivity state = %s \tRRM/BTM/MBO = %s/%s/%s\n",
			cli->current_chan, cli->activity_state ? "ACTIVE" : "IDLE",
			((cli->capab & CLI_CAP_11K) ? "YES": "NO"),
			((cli->capab & CAP_11V_SUPPORTED) ? "YES" : "NO"),
			((cli->capab & CAP_MBO_SUPPORTED) ? "YES" : "NO"));

	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
		return pos - buf;
	}
	pos += ret;


	ret = os_snprintf(pos, end - pos,
			"dl_rate=%d\tul_rate=%d\ttx_count=%d\trx_count=%d\tauth_deny_count=%d"
			"\tauth_deny_max=%d\tCurrAirTime(calculated)=%d\n",
			cli->dl_rate, cli->ul_rate, cli->tx_count, cli->rx_count,
			cli->auth_deny_count, cli->auth_deny_max, cli->curr_air_time);

	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
		return pos - buf;
	}
	pos += ret;

	ret = os_snprintf(pos, end - pos,
					"PhyRate=%d\n",cli->dl_phy_rate);
	if (os_snprintf_error(end - pos, ret)) {
			*pos = '\0';
			return pos - buf;
	}
	pos += ret;



    return pos - buf;


}


int mapd_get_mib(struct mapd_global *global, char *buf, int buf_len)
{

    char *end, *pos;
    uint8_t i = 0;
    int ret = 0;
    struct mapd_bss *bss = NULL;
    struct client *cli = NULL;

    pos = buf;
    end = buf + buf_len;

	/* Dump radio info */
    for(i= 0; i < MAX_NUM_OF_RADIO; i++) {
        struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[i];
        ret = os_snprintf(pos, end - pos,
                "RaNum=%d\n \tidx=%d Id=" MACSTR " BssBM=%x channel=%d Util=%d\n",
                i, radio_info->radio_idx,  MAC2STR(radio_info->identifier),
                radio_info->bss_bitmap, radio_info->channel, radio_info->ch_util);
        if (os_snprintf_error(end - pos, ret)) {
            *pos = '\0';
            return pos - buf;
        }
        pos += ret;


		if(((radio_info->bss_list.next) != NULL)&&((radio_info->bss_list.prev) != NULL)&&(dl_list_len(&radio_info->bss_list)!=0))
		{
        	ret = os_snprintf(pos, end - pos,
                	"\tBSS List:\n");
        	if (os_snprintf_error(end - pos, ret)) {
            	*pos = '\0';
            	return pos - buf;
        	}
        	pos += ret;
        	dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry) {
            	ret = os_snprintf(pos, end - pos,
                 	   "\t\tbssid=" MACSTR " ssid=%s channel=%d idx=%d\n",
                    	MAC2STR(bss->bssid), bss->ssid, bss->channel, bss->bss_idx);
            	if (os_snprintf_error(end - pos, ret)) {
                	*pos = '\0';
                	return pos - buf;
            	}
            	pos += ret;

				if(((bss->assoc_sta_list.next) != NULL)&&((bss->assoc_sta_list.prev) != NULL)&&(dl_list_len(&bss->assoc_sta_list)!=0))
				{
            		ret = os_snprintf(pos, end - pos,
                	    	"\t\tASSOC List:\n");
            		if (os_snprintf_error(end - pos, ret)) {
               	 	*pos = '\0';
                		return pos - buf;
           			}
            		pos += ret;
            		dl_list_for_each(cli, &bss->assoc_sta_list, struct client, assoc_sta_entry) {
                		ret = os_snprintf(pos, end - pos,
								"\t\t\tMAC=" MACSTR "\t\t client_id = %u \n", MAC2STR(cli->mac_addr),cli->client_id);
						if (os_snprintf_error(end - pos, ret)) {
                    		*pos = '\0';
                    		return pos - buf;
                		}
                		pos += ret;
					}
            	}
        	}
		}
		else
		{

			//Dont need to do anything here technically, just throw in a print that
		   	//the list is still not initialized

		}

	}
    return pos - buf;
}
#ifdef SUPPORT_MULTI_AP
Boolean mapd_is_mandate_on(struct mapd_global *global, u8 *cli_mac_addr)
{
	struct client *map_client = client_db_get_client_from_sta_mac(global,cli_mac_addr);
	struct mapd_bss *connected_bss = NULL;//= mapd_get_bss_from_mac(global, ->bssid);
		/* XXX: TODO: TO be filled by SS5 */
		//Check if steer timer is remaining and is > 'x' seconds

	if(map_client == NULL || is_zero_ether_addr(map_client->bssid))
		return FALSE;

	connected_bss = mapd_get_bss_from_mac(global, map_client->bssid);
	if(connected_bss == NULL || connected_bss->steer_req_len == 0)
		return FALSE;

	if(connected_bss->_1905_steer_req_msg->request_mode == 1) {
		int i;
		for (i=0; i< connected_bss->_1905_steer_req_msg->sta_count;i++) {
			if(!os_memcmp(connected_bss->_1905_steer_req_msg->info[i].sta_mac, cli_mac_addr, ETH_ALEN)) {
				if((i < 32) && connected_bss->mandate_steer_done_bitmap & BIT (i)) {
					err("Mandate SteerComplete for client : " MACSTR "\n", MAC2STR(cli_mac_addr));
					return FALSE;
				} else
					return TRUE;
			}
		}
	}
	return FALSE;
}

u8 * mapd_get_target_mandate_bssid(struct mapd_global *global, uint8_t client_id)
{
	struct client *map_client = client_db_get_client_from_client_id(global,client_id);
	struct mapd_bss *connected_bss = NULL;//= mapd_get_bss_from_mac(global, ->bssid);
		/* XXX: TODO: TO be filled by SS5 */
		//Check if steer timer is remaining and is > 'x' seconds

	if(map_client == NULL || is_zero_ether_addr(map_client->bssid))
		return ZERO_MAC_ADDR;

	connected_bss = mapd_get_bss_from_mac(global, map_client->bssid);
	if(connected_bss == NULL || connected_bss->steer_req_len == 0)
		return ZERO_MAC_ADDR;

	if(connected_bss->_1905_steer_req_msg->request_mode == 1) {
		int i;
		for (i=0; i< connected_bss->_1905_steer_req_msg->sta_count;i++) {
			if(!os_memcmp(connected_bss->_1905_steer_req_msg->info[i].sta_mac, map_client->mac_addr, ETH_ALEN))
				return connected_bss->_1905_steer_req_msg->info[i].target_bssid;
		}
	}
	return ZERO_MAC_ADDR;
}

/* Return STEER_OPP_VALID if a steering opportunity has been received and there are atleast
 * 'X' seconds remaining of the steering window */
enum steer_opp_allow mapd_is_steering_opp_recvd(struct mapd_global *global, u8 *bssid, u8 *cli_mac_addr)
{
	struct os_time now;
	struct mapd_bss *bss = mapd_get_bss_from_mac(global, bssid);
	if(bss == NULL)
		return STEER_OPP_INVALID;

	os_get_time(&now);

	if (bss->steer_req_len == 0
		|| bss->_1905_steer_req_msg->request_mode == 1)
		return STEER_OPP_INVALID;

	if((now.sec > (bss->steer_req_timestamp.sec + bss->_1905_steer_req_msg->steer_window)))
		return STEER_OPP_TIME_EXPIRE;

	if((now.sec > (bss->steer_req_timestamp.sec + bss->_1905_steer_req_msg->steer_window - STEER_OPP_MIN_TIME)))
		return STEER_OPP_TIME_INSUFF;


	if (bss->_1905_steer_req_msg->sta_count == 0)
		return STEER_OPP_VALID;
	else {
		int i;
		for (i=0; i< bss->_1905_steer_req_msg->sta_count;i++) {
			if(!os_memcmp(bss->_1905_steer_req_msg->info[i].sta_mac, cli_mac_addr, ETH_ALEN))
				return STEER_OPP_VALID;
		}
	}
	return STEER_OPP_INVALID;
}
#ifdef CENT_STR
void mapd_cent_str_send_steering_complete(struct mapd_global *global,struct mapd_bss *bss)
{
	struct _1905_map_device *own_1905_device = NULL;


	own_1905_device = topo_srv_get_next_1905_device(&global->dev,NULL);
	if(own_1905_device->device_role == DEVICE_ROLE_CONTROLLER){
		err("SteerComplete for bss : " MACSTR "\n", MAC2STR(bss->bssid));
	}
	else {
		map_1905_Set_Steering_Complete_Info(global->_1905_ctrl);

		if(bss->_1905_steer_req_msg != NULL) {
			os_free(bss->_1905_steer_req_msg);
			bss->steer_req_len = 0;
			bss->_1905_steer_req_msg = NULL;
			bss->steer_req_timestamp.sec = 0;
			bss->steer_req_timestamp.usec = 0;
		}
	}
	return;
}

#endif

void mapd_send_steering_complete(struct mapd_global *global, struct mapd_bss *bss)
{
	struct _1905_map_device *own_1905_device = NULL;
	enum steer_opp_allow steer_window_status;

	debug("SteerComplete for bss : " MACSTR "\n", MAC2STR(bss->bssid));

	steer_window_status = mapd_is_steering_opp_recvd(global, bss->bssid, NULL);
	if(global->params.Certification && steer_window_status == STEER_OPP_INVALID) {
		mapd_printf(MSG_ERROR, "Steering opportunity invalid for this BSS"
			MACSTR, MAC2STR(bss->bssid));
		if(bss->_1905_steer_req_msg != NULL) {
			os_free(bss->_1905_steer_req_msg);
			bss->steer_req_len = 0;
			bss->_1905_steer_req_msg = NULL;
			bss->steer_req_timestamp.sec = 0;
			bss->steer_req_timestamp.usec = 0;
		}
		return;
	}
	if (steer_window_status == STEER_OPP_INVALID) {
		mapd_printf(MSG_EXCESSIVE, "Steering opportunity invalid for this BSS"
			MACSTR, MAC2STR(bss->bssid));
		return;
	}

	own_1905_device = topo_srv_get_next_1905_device(&global->dev,NULL);
	if(own_1905_device->device_role == DEVICE_ROLE_CONTROLLER)
		chan_mon_handle_steer_complete(&global->dev,own_1905_device);
	else {
		map_1905_Set_Steering_Complete_Info(global->_1905_ctrl);
#if 1
		if(bss->_1905_steer_req_msg != NULL) {
			os_free(bss->_1905_steer_req_msg);
			bss->steer_req_len = 0;
			bss->_1905_steer_req_msg = NULL;
			bss->steer_req_timestamp.sec = 0;
			bss->steer_req_timestamp.usec = 0;
		}
#endif

	}
	return;
}

Boolean steer_window_required(STEERING_METHOD_TYPE steer_method)
{
	if((steer_method == MANDATE) ||
	   (steer_method == ACTIVE_STANDALONE_DG) ||
	   (steer_method == IDLE_STANDALONE_DG) ||
	   (steer_method == NOL_MULTIAP))
		return FALSE;

	return TRUE;
}

void mapd_steering_complete(struct mapd_global *global)
{

	struct mapd_bss *bss = NULL;
	uint8_t i = 0;
	enum steer_opp_allow steer_window_status = 0;
	struct own_1905_device *ctx = &global->dev;
	struct steer_cands *cand = NULL;
	int send_flag = 0;
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		struct mapd_radio_info *ra_info = NULL;
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;
		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
			struct client *cli = NULL;

			steer_window_status = mapd_is_steering_opp_recvd(global, bss->bssid, NULL);
			if (steer_window_status == STEER_OPP_INVALID) {
				mapd_printf(MSG_EXCESSIVE, "Steering opportunity invalid for this BSS"
					MACSTR, MAC2STR(bss->bssid));
				continue;
			}
			/* Steer Window is ON */
			send_flag = 0;
			SLIST_FOREACH(cand, &ctx->steer_cands_head, next_cand) {
				if ((!is_zero_ether_addr(cand->steer_cand_home_bssid)) &&
					(bss == mapd_get_bss_from_mac(global, cand->steer_cand_home_bssid))) {
					cli = cand->steer_cand;
					if (!cli) {
						mapd_printf(MSG_ERROR, "steer_cand is NULL");
						mapd_ASSERT(0);
					}
					if (steer_window_required(cli->cli_steer_method)) {
						if (steer_window_status == STEER_OPP_TIME_EXPIRE) {
							mapd_printf(MSG_ERROR, "WARNING(OOPS)Steer window elapsed, but"
											" steering(%d) is in progress for" MACSTR,
											cli->cli_steer_method, MAC2STR(bss->bssid));
						}
						send_flag = 1;
						break;
					}
				}
			}
			if (send_flag == 1)
				continue;
			/* No steering window dependent Steering ongoing on
			 * this BSS - send steer complete */
			mapd_send_steering_complete(global, bss);
			}
		}
}
#endif

TRIGGER_TYPE mapd_get_trigger_from_steer_method(struct mapd_global *global,
				STEERING_METHOD_TYPE steer_method)
{
	switch(steer_method)
	{
#ifdef SUPPORT_MULTI_AP
		case MANDATE:
			return MANDATE_TRIGGER;
#endif
		case ACTIVE_STANDALONE_DG:
			return ACTIVE_STANDALONE_DG_TRIGGER;
		case IDLE_STANDALONE_DG:
			return IDLE_STANDALONE_DG_TRIGGER;
#ifdef SUPPORT_MULTI_AP
		case NOL_MULTIAP:
			return NOL_MULTIAP_TRIGGER;
#endif
		case OFFLOADING:
			return OFFLOADING_TRIGGER;
		case ACTIVE_STANDALONE_UG:
			return ACTIVE_STANDALONE_UG_TRIGGER;
		case IDLE_STANDALONE_UG:
			return IDLE_STANDALONE_UG_TRIGGER;
		case IDLE_5GL_TO_5GH:
			return IDLE_STANDALONE_5GL_TO_5GH_TRIGGER;
		case IDLE_5GH_TO_5GL:
			return IDLE_STANDALONE_5GH_TO_5GL_TRIGGER;
		case ACTIVE_5GL_TO_5GH:
			return ACTIVE_STANDALONE_5GL_TO_5GH_TRIGGER;
		case ACTIVE_5GH_TO_5GL:
			return ACTIVE_STANDALONE_5GH_TO_5GL_TRIGGER;
		default:
				assert(0);
	}
}

void mapd_handle_traffic_stats(struct mapd_global *global,
				struct sta_traffic_stats *stats_arr)
{
	struct mapd_radio_info *radio_info = NULL;
	unsigned long byte_count = 0;
	int idx = 0;

	radio_info = get_radio_info_by_radio_id(global, stats_arr->identifier);
	if (!radio_info) {
		mapd_printf(MSG_ERROR, "Invalid radio id " MACSTR,
						MAC2STR(stats_arr->identifier));
		return;
	}
	debug("channel %d", radio_info->channel);

	for(idx = 0; idx < stats_arr->sta_cnt; ++idx) {
			byte_count += stats_arr->stats[idx].bytes_sent
				+ stats_arr->stats[idx].bytes_received;
			debug("sta mac "MACSTR " incremental byte_count %ld ",
				MAC2STR(stats_arr->stats[idx].mac),byte_count);
			if (!stats_arr->stats[idx].is_APCLI) {
				client_mon_handle_traffic_stats(global, stats_arr->stats[idx].mac,
					stats_arr->stats[idx].bytes_sent,
					stats_arr->stats[idx].bytes_received,
					stats_arr->stats[idx].packets_sent,
					stats_arr->stats[idx].packets_received,
					stats_arr->stats[idx].tx_packets_errors,
					stats_arr->stats[idx].rx_packets_errors);
			}
	}
	if(!stats_arr->sta_cnt) {
		debug("sta cnt 0 , return");	
		return;
	}
	debug(" total byte_count %ld last byte count %ld",
		byte_count, 
		radio_info->last_byte_count);
#ifdef SUPPORT_MULTI_AP
	if ((byte_count > radio_info->last_byte_count) &&
	((byte_count - radio_info->last_byte_count) >
		global->dev.ch_planning.ChPlanningIdleByteCount))
	{
		debug("last high byte get");
		os_get_time(&global->dev.ch_planning.last_high_byte_count_ts);
	}
	radio_info->last_byte_count = byte_count;
#endif
}


int newline_terminated(const char *buf, size_t buflen)
{
	size_t len = os_strlen(buf);
	if (len == 0)
		return 0;
	if (len == buflen - 1 && buf[buflen - 1] != '\r' &&
	    buf[len - 1] != '\n')
		return 0;
	return 1;
}

void skip_line_end(FILE *stream)
{
	char buf[100];
	while (fgets(buf, sizeof(buf), stream)) {
		buf[sizeof(buf) - 1] = '\0';
		if (newline_terminated(buf, sizeof(buf)))
			return;
	}
}

/**
 * mapd_config_get_line - Read the next configuration file line
 * @s: Buffer for the line
 * @size: The buffer length
 * @stream: File stream to read from
 * @line: Pointer to a variable storing the file line number
 * @_pos: Buffer for the pointer to the beginning of data on the text line or
 * %NULL if not needed (returned value used instead)
 * Returns: Pointer to the beginning of data on the text line or %NULL if no
 * more text lines are available.
 *
 * This function reads the next non-empty line from the configuration file and
 * removes comments. The returned string is guaranteed to be null-terminated.
 */
char * mapd_config_get_line(char *s, int size, FILE *stream, int *line,
				  char **_pos)
{
	char *pos, *end, *sstart;

	while (fgets(s, size, stream)) {
		(*line)++;
		s[size - 1] = '\0';
		if (!newline_terminated(s, size)) {
			/*
			 * The line was truncated - skip rest of it to avoid
			 * confusing error messages.
			 */
			mapd_printf(MSG_INFO, "Long line in configuration file "
				   "truncated");
			skip_line_end(stream);
		}
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
#ifdef SUPPORT_MULTI_AP
extern const int8_t NOISE_OFFSET_BY_CH_WIDTH[];
int mapd_Set_RssiTh(struct mapd_global *global, char *cmd_buf)
{
	struct _1905_map_device *tmp_dev, *own_dev;
	struct steer_params *cli_steer ;
	int rssi_thresh_value;
    rssi_thresh_value=atoi(cmd_buf);
	mapd_printf(MSG_ERROR,"rssi_thresh_value  %d\n",rssi_thresh_value );
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);

	tmp_dev = topo_srv_get_next_1905_device(&global->dev,own_dev); /*Get the next 1905 device*/

	cli_steer = &global->dev.cli_steer_params;
	cli_steer->LowRSSIAPSteerEdge_RE = rssi_thresh_value - NOISE_OFFSET_BY_CH_WIDTH[0];
	//printf("lrse %d, rssi_thresh_value %d, noise offset %d\n", cli_steer->LowRSSIAPSteerEdge_RE ,rssi_thresh_value, NOISE_OFFSET_BY_CH_WIDTH[0] );
	//run loop to update policy at all the 1905 dev
	while(tmp_dev)
	{
	//	printf("(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(tmp_dev->_1905_info.al_mac_addr));
		steer_msg_update_policy_config(global, tmp_dev);
		tmp_dev = topo_srv_get_next_1905_device(&global->dev,tmp_dev); /*Get the next 1905 device*/
	}
	return 0 ;
}

int mapd_Set_ChUtilTh(struct mapd_global *global, char *cmd_buf)
{
	struct _1905_map_device *tmp_dev, *own_dev;
	struct steer_params *cli_steer ;
	int ch_util_thresh_value[3];
	sscanf(cmd_buf, "%d %d %d",
		(unsigned int*)ch_util_thresh_value ,
		(unsigned int*)(ch_util_thresh_value+1),
		(unsigned int*)(ch_util_thresh_value+2));
	mapd_printf(MSG_ERROR,"CU_thresh_value:  %d ,%d ,%d\n",ch_util_thresh_value[0],ch_util_thresh_value[1],ch_util_thresh_value[2] );
	cli_steer = &global->dev.cli_steer_params;
	cli_steer->CUOverloadTh_2G = ch_util_thresh_value[0];
	cli_steer->CUOverloadTh_5G_L = ch_util_thresh_value[1];
	cli_steer->CUOverloadTh_5G_H = ch_util_thresh_value[2];
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);
	tmp_dev = topo_srv_get_next_1905_device(&global->dev,own_dev); /*Get the next 1905 device*/
	//run loop to update policy at all the 1905 dev
	while(tmp_dev)
	{
		mapd_printf(MSG_ERROR,"(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(tmp_dev->_1905_info.al_mac_addr));
		steer_msg_update_policy_config(global, tmp_dev);
		tmp_dev = topo_srv_get_next_1905_device(&global->dev,tmp_dev); /*Get the next 1905 device*/
	}

	return 0 ;
}

int mapd_mandate_steer(struct mapd_global *global, char *cmd_buf)
{

	unsigned char mac_sta[6] = {0}, mac_bssid[6] = {0};
	struct client *cli=NULL;
	struct _1905_map_device *dev=NULL, *own_dev=NULL;
	struct _1905_map_device *target_1905=NULL;
	struct bss_info_db *target_bss =NULL;
	struct lib_steer_request *steer_req_msg = NULL;
	struct map_lib_target_bssid_info *map_bss_info = NULL;
	struct rr_steer_controller *rr_control =NULL;
	struct os_time now;
	struct mapd_bss *curr_own_bss = NULL;
#ifdef MAP_R2
	struct lib_steer_request_R2 *steer_req_msg_r2 = NULL;
	struct map_lib_target_bssid_info_R2 *map_bss_info_r2 = NULL;

#endif
	if (os_get_time(&now)) {
		mapd_printf(MSG_ERROR,"Can't get os time.\n");
		return -1 ;
	}
	if (hwaddr_aton(cmd_buf, mac_sta) < 0) {
		return -1;
	}
	cmd_buf = cmd_buf+18;
	if (hwaddr_aton(cmd_buf, mac_bssid) < 0) {
		return -1;
	}
	mapd_printf(MSG_ERROR, "Trigger mandate Steering on agent with MAC %02x:%02x:%02x:%02x:%02x:%02x \n", PRINT_MAC(mac_sta));
	mapd_printf(MSG_ERROR, "Target BSSID %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(mac_bssid));

	if(global->dev.SetSteer == STEER_DISABLE){
		mapd_printf(MSG_ERROR,"Steering is disabled\n");
		return -1;
	}
//Find client
	cli = client_db_get_client_from_sta_mac(global, mac_sta);
	if (cli == NULL) {
		mapd_printf(MSG_ERROR, MACSTR " not in DB", MAC2STR(mac_sta));
		return -1;
	}
//Find device to which client is connected
	dev = topo_srv_get_1905_by_bssid(&global->dev,(unsigned char *)cli->bssid);//Agent's dev
	if(dev == NULL) {
		mapd_printf(MSG_ERROR,"Can't find device to which client is connected\n");
		return -1 ;
	}
	mapd_printf(MSG_ERROR,"client is connected to 1905 device MAC %x:%x:%x:%x:%x:%x,  almac %x:%x:%x:%x:%x:%x\n",PRINT_MAC(cli->bssid),PRINT_MAC(dev->_1905_info.al_mac_addr));
//Find the target device to which we want client to connect
	target_1905 = topo_srv_get_1905_by_bssid(&global->dev,(unsigned char *)mac_bssid);
	if(target_1905 == NULL) {
			mapd_printf(MSG_ERROR,"Can't find target device to steer client\n");
			return -1 ;
		}
//Find the target device BSS for channel and operating class
	  target_bss = topo_srv_get_bss_by_bssid(&global->dev, target_1905, mac_bssid);
	  if(target_bss == NULL) {
		  mapd_printf(MSG_ERROR,"target bss not found\n");
		  return -1 ;
	  }
	  mapd_printf(MSG_ERROR,"class %d ,channel %d\n",
	  target_bss->radio->operating_class,target_bss->radio->channel[0]);

//Create Mandate Steer request message
#ifdef MAP_R2
	if((dev->map_version == DEV_TYPE_R2) && (cli->capab & CAP_MBO_SUPPORTED))
		chan_mon_create_steer_req_mandate(1 , (char *)mac_sta , 1 , (char *)cli->bssid, target_bss,NULL,NULL, &steer_req_msg_r2, &map_bss_info_r2,STEER_REASON_UNSPECIFIED);
	else	
		chan_mon_create_steer_req_mandate(1 , (char *)mac_sta , 1 , (char *)cli->bssid, target_bss, &steer_req_msg, &map_bss_info,NULL,NULL,0);
#else
		chan_mon_create_steer_req_mandate(1 , (char *)mac_sta , 1 , (char *)cli->bssid, target_bss, &steer_req_msg, &map_bss_info);
#endif

#ifdef CENT_STR
	if(!global->dev.cent_str_en)
#endif
	{
//To avoid race condition of steering
	rr_control = &global->dev.controller_context.rr_control;
	//Update the p_current_1905_rr to the device which we are sending the mandate steer request
	rr_control->p_current_1905_rr =dev;
	//Update round robin states of own device, to stop RR steering till Mandate steering is complete
	rr_control->can_trigger_steer_req = FALSE;
	rr_control->rr_state = STEER_REQ_TRIGGERED;
	rr_control->rr_steer_req_timestamp.sec = now.sec;
	rr_control->p_current_1905_rr->p_current_bss_rr = target_bss;
	rr_control->p_current_1905_rr->p_current_bss_rr->b_steer_triggered = FALSE;
	}
//Find own device
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);

//Decide whether to send Steer request to other device or use for owndevice
		if (dev == own_dev)
		{	//if client is connected to myself then i don't need to send command via 1905
			mapd_printf(MSG_ERROR,"this client is connected to myself\n");
			curr_own_bss = mapd_get_bss_from_mac(global,cli->bssid );
			if(curr_own_bss == NULL) {
				mapd_printf(MSG_ERROR,"crr_own_bss is NULL\n");
				goto RET;
			}
			if(curr_own_bss)
			{	mapd_printf(MSG_ERROR,"ssid %s\n",curr_own_bss->ssid);
				curr_own_bss->_1905_steer_req_msg = NULL;
				curr_own_bss->steer_req_len = 0;
				mapd_printf(MSG_ERROR,"fill steer req data in bss\n");
#ifdef MAP_R2
			if((dev->map_version == DEV_TYPE_R2) && (cli->capab & CAP_MBO_SUPPORTED))
				chan_mon_fill_steer_req_data(curr_own_bss, NULL, NULL, steer_req_msg_r2, map_bss_info_r2);
			else
				chan_mon_fill_steer_req_data(curr_own_bss, steer_req_msg, map_bss_info, NULL, NULL);
#else
				chan_mon_fill_steer_req_data(curr_own_bss, steer_req_msg, map_bss_info);
#endif

#ifdef CENT_STR
				if(global->dev.cent_str_en) {
					if (cli && cli->cli_steer_state > CLI_STATE_IDLE) {
						mapd_printf(MSG_ERROR, "Steering in progress for " MACSTR,
										MAC2STR(cli->mac_addr));
						return -1;
					}
					if(mapd_is_mandate_on(global, cli->mac_addr)){
						struct cent_steer_cands *cand = NULL;
						cli->cli_steer_method = MANDATE;
						cand = os_zalloc(sizeof(*cand));
						cand->steer_cand = cli;
						err("Mandate Steer Enqueued");
						os_memcpy(cand->steer_cand_home_bssid, cli->bssid, ETH_ALEN);
						STAILQ_INSERT_TAIL(&global->dev.cent_steer_cands_head, cand, next_cand);
					}

				}
#endif
			}
		}
		else
		{
			mapd_printf(MSG_ERROR,"this client is connected to some other device send 1905 msg\n");
#ifdef MAP_R2 // TODO: Fix for turnkey for MBO STA
			if((dev->map_version == DEV_TYPE_R2) && (cli->capab & CAP_MBO_SUPPORTED))
				map_1905_Send_Client_Steering_Request_Message(global->_1905_ctrl,
														(char *)dev->_1905_info.al_mac_addr,
														NULL,0, NULL, steer_req_msg_r2, 1, map_bss_info_r2->bss_info);
			else
			map_1905_Send_Client_Steering_Request_Message(global->_1905_ctrl,
														(char *)dev->_1905_info.al_mac_addr,
														steer_req_msg, 1, map_bss_info->bss_info, NULL,0, NULL);
#else
			map_1905_Send_Client_Steering_Request_Message(global->_1905_ctrl,
														(char *)dev->_1905_info.al_mac_addr,
														steer_req_msg, 1, map_bss_info->bss_info);
#endif
		}
RET:
	//free memory
	if(steer_req_msg)
		os_free(steer_req_msg);

	if(map_bss_info)
		os_free(map_bss_info);
#ifdef MAP_R2
	if(steer_req_msg_r2)
		os_free(steer_req_msg_r2);

	if(map_bss_info_r2)
		os_free(map_bss_info_r2);

#endif
	return 0;
}


/**
 * @brief Fn to check whether an AP is in downstream device
 *
 * @param ctx own 1905 device ctx
 * @param bssid bssid of the AP
*
* @return True/False
*/
int is_1905_device_downstream(struct _1905_map_device *target_dev, struct _1905_map_device *bh_dev)
{
	while(target_dev && target_dev->in_network) {
		if (target_dev == bh_dev)
			return TRUE;
		target_dev = topo_srv_get_upstream_device(target_dev);
	}

	return FALSE;
}

int  mapd_bh_steer(struct mapd_global *global, char *cmd_buf)
{

	unsigned char mac_bh[ETH_ALEN] = {0};
	unsigned char mac_bssid[ETH_ALEN] = {0};
	struct _1905_map_device *dev_1905 , *target_1905;
	unsigned char downstream_dev = 0;

	if (hwaddr_aton(cmd_buf, mac_bh) < 0) {
		return -1;
	}

	cmd_buf=cmd_buf+18;

	if (hwaddr_aton(cmd_buf, mac_bssid) < 0) {
		return -1;
	}

	mapd_printf(MSG_ERROR,"Trigger backhaul Steering on agent with MAC %02x:%02x:%02x:%02x:%02x:%02x \n",PRINT_MAC(mac_bh));
	mapd_printf(MSG_ERROR,"Target BSSID %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(mac_bssid));

	dev_1905 = topo_srv_get_1905_by_iface_addr(&global->dev,(u8 *)mac_bh);
	if(dev_1905 == NULL) {
		mapd_printf(MSG_ERROR,"1905 Device not found\n");
		return -1 ;
	}
	target_1905=topo_srv_get_1905_by_iface_addr(&global->dev,(u8 *)mac_bssid);
	if(target_1905 == NULL || !target_1905->in_network) {
		mapd_printf(MSG_ERROR,"target 1905 Device not found\n");
		return -1 ;
	}
    struct bss_info_db *target_bss = topo_srv_get_bss_by_bssid(&global->dev, target_1905, mac_bssid);
	//printf("class %d ,channel %d\n", target_bss->radio->operating_class,target_bss->radio->channel);
	if(target_bss == NULL) {
		mapd_printf(MSG_ERROR,"target bss not found\n");
		return -1 ;
	}
	/*if target bss is a downstream bss, return*/
	downstream_dev = is_1905_device_downstream(target_1905, dev_1905);
	if (downstream_dev) {
		err("invalid bhsteer!!!!! target bssid operates on downstream dev");
		err("target_bssid " MACSTR " target_dev " MACSTR, MAC2STR(mac_bssid), MAC2STR(target_1905->_1905_info.al_mac_addr));
		err("bh_mac " MACSTR " bh_dev " MACSTR, MAC2STR(mac_bh), MAC2STR(dev_1905->_1905_info.al_mac_addr));
		return -1;
	}

	err("Disable Network Otimization");
	global->dev.network_optimization.network_optimization_enabled = 0;
	update_ntwrk_opt_in_dat_file(0);

	map_1905_Send_Backhaul_Steering_Request_Message(global->_1905_ctrl, (char *)dev_1905->_1905_info.al_mac_addr, mac_bh,
	mac_bssid,target_bss->radio->operating_class,target_bss->radio->channel[0]);

	return 0;
}


int mapd_Get_Bh_ConnectionStatus(struct own_1905_device *ctx, char *buf, size_t buf_Len)
{
	struct bh_link_entry *bh_entry;
	struct iface_info *iface = NULL;
	struct _1905_map_device *own_dev = topo_srv_get_next_1905_device(ctx, NULL);
	struct backhaul_link_info *link = NULL;
	struct map_neighbor_info *neighbor = NULL;
	char term = 0;
	char hopcnt = 0;
	mapd_printf(MSG_ERROR,"%s\n",__FUNCTION__);
	buf[0]=WAPP_APCLI_DISASSOCIATED;

	SLIST_FOREACH(bh_entry, &(ctx->bh_link_head), next_bh_link) {
		if (bh_entry->bh_assoc_state==WAPP_APCLI_ASSOCIATED)
			{
				mapd_printf(MSG_ERROR,"WIFI BH CONNECT");
				buf[0]=WAPP_APCLI_ASSOCIATED;
				break;
			}
		mapd_printf(MSG_ERROR,"WIFI BH NOT CONNECT");
		buf[0]=WAPP_APCLI_DISASSOCIATED;
	}

	SLIST_FOREACH(neighbor, &(own_dev->neighbors_entry), next_neighbor) {
		topo_serv_clear_visit_node(ctx);
		own_dev->visited = 1;
		hopcnt_to_controller(neighbor->neighbor, &hopcnt, &term);

		SLIST_FOREACH(link, &neighbor->bh_head, next_bh) {
			iface = topo_srv_get_iface(own_dev , link->connected_iface_addr);
			if (!iface)
				continue;
			if (iface->media_type < IEEE802_11_GROUP) {
				if (term) {
					mapd_printf(MSG_ERROR,"ETH BH CONNECT");
					buf[0]=WAPP_APCLI_ASSOCIATED;
					break;
				}
				mapd_printf(MSG_ERROR,"ETH BH NOT CONNECT");
				buf[0]=WAPP_APCLI_DISASSOCIATED;
			}
		}
	}

	buf_Len=1;

	return buf_Len;

}

int mapd_Get_Bh_ConnectionType(struct own_1905_device *ctx, char *buf, size_t buf_Len)
{
	struct bh_link_entry *bh_entry;
	struct iface_info *iface = NULL;
	struct _1905_map_device *own_dev = topo_srv_get_next_1905_device(ctx, NULL);
	struct backhaul_link_info *link = NULL;
	struct map_neighbor_info *neighbor = NULL;
	char term = 0;
	char hopcnt = 0;
	mapd_printf(MSG_ERROR,"%s\n",__FUNCTION__);
	buf[0]=WAPP_APCLI_DISASSOCIATED;

	SLIST_FOREACH(bh_entry, &(ctx->bh_link_head), next_bh_link) {
		if (bh_entry->bh_assoc_state==WAPP_APCLI_ASSOCIATED)
		{
			    mapd_printf(MSG_ERROR,"WIFI BH CONNECT");
				if(bh_entry->bh_channel <= 14)
				{
					buf[0]=1;
				}
				else if(bh_entry->bh_channel > 14)
				{
					buf[0]=2;
				}
				break;
		}
		mapd_printf(MSG_ERROR,"WIFI BH NOT CONNECT");
		buf[0]=WAPP_APCLI_DISASSOCIATED;
	}

	SLIST_FOREACH(neighbor, &(own_dev->neighbors_entry), next_neighbor) {
		topo_serv_clear_visit_node(ctx);
		own_dev->visited = 1;
		hopcnt_to_controller(neighbor->neighbor, &hopcnt, &term);

		SLIST_FOREACH(link, &neighbor->bh_head, next_bh) {
			iface = topo_srv_get_iface(own_dev , link->connected_iface_addr);
			if (!iface)
				continue;
			if (iface->media_type < IEEE802_11_GROUP) {
				if (term) {
					mapd_printf(MSG_ERROR,"ETH BH CONNECT");
					buf[0]= 3;
					break;
				}
				mapd_printf(MSG_ERROR,"ETH BH NOT CONNECT");
				buf[0]=WAPP_APCLI_DISASSOCIATED;
			}
		}
	}

	buf_Len=1;

	return buf_Len;

}
#endif /* #ifdef SUPPORT_MULTI_AP */
void Write_Steer_Status(char *status)
{
	FILE *fptr;
	fptr = fopen("/tmp/sta_steer_progress","w");
	if(fptr == NULL)
	{
	  err("Error!");
	  //exit(1);
	  return;
	}
	fprintf(fptr,"%s",status);
	fclose(fptr);
}
#ifdef SUPPORT_MULTI_AP
int  mapd_send_config_renew(struct mapd_global *global)
{
	struct radio_info_db *radio;
	struct _1905_map_device  *own_dev;
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);
	if (!own_dev) {
		err("own 1905 dev is missing\n");
		return -1;
	}
	//err("update the configuration renew message \n");
	map_1905_Set_Read_Bss_Conf_Request(global->_1905_ctrl);
	SLIST_FOREACH(radio, &own_dev->first_radio, next_radio) {
		if(radio->channel[0] <= 14) {
			err("24G %d\n", radio->channel[0]);
			map_1905_Send_AP_autoconfig_Renew_Message(global->_1905_ctrl,0);//BAND_24G is 0 in wapp
		}
		else if(isChan5GL(radio->channel[0])||isChan5GH(radio->channel[0])){
			err("5GL %d\n", radio->channel[0]);
			map_1905_Send_AP_autoconfig_Renew_Message(global->_1905_ctrl,1);//BAND_5G is 1 in wapp
		}
		else {
				err("Invalid Channel\n");
				return -1;
		}
	}
	return 0 ;
}

void mapd_renew(struct mapd_global *global)
{
#ifndef CONFIG_SUPPORT_OPENWRT
	char cmd[200];
	memset(cmd,0,sizeof(cmd));
	os_snprintf(cmd, sizeof(cmd), "RenewProfile=$(cat /etc/wts_bss_info_config);nvram_set cert WTS_BSS_INFO_CONFIG \"$RenewProfile\"");
	system(cmd);

//	memset(cmd,0,sizeof(cmd));
//	os_snprintf(cmd, sizeof(cmd), "nvram_set 2860 WTS_BSS_INFO_CONFIG_CERT_OK 1);
//	system(cmd);

#endif
	unsigned int val = 3;
	u8 *buf = (u8*)&val;
	global->dev.need_to_update_wts = 1;
	wlanif_issue_wapp_command(global, WAPP_USER_SET_RADIO_RENEW,
		0, NULL, NULL,buf, 1, 0, 0, 0);
	map_1905_Set_Read_Bss_Conf_and_Renew_v2(global->_1905_ctrl, 0);
}

int mapd_Get_BH_interfaceAP(struct mapd_global *global,char *buf, size_t buf_Len)
{
	struct _1905_map_device  *own_dev;
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);
	struct bss_info_db *bss;
	unsigned char temp_buf[300];
	int i=0, num_mac=0;
	if (!own_dev) {
		err("own 1905 dev is missing\n");
		return -1;
	}

	buf_Len=0;
	SLIST_FOREACH(bss, &own_dev->first_bss, next_bss) {
		if (!bss->radio) {
			err("radio for bss not found\n");
		}
		if(bss->map_vendor_extn & BH_BSS) {
			memcpy(&temp_buf[i], bss->bssid, ETH_ALEN);
			i=i+6;
			buf_Len+=6;
		}
	}

	num_mac=buf_Len/6;
	buf_Len=0;
	for (i=0;i<num_mac;i++)
		{	os_snprintf(buf+(i*18),19,MACSTR";",MAC2STR(temp_buf+(i*6)));
			buf_Len+=18;
		}
	buf_Len+=1;//For '\0'
	mapd_printf(MSG_ERROR,"mapd.c %s , reply=%zd\n", buf, buf_Len);
	return buf_Len;
}

int mapd_Get_FH_interfaceAP(struct mapd_global *global,char *buf, size_t buf_Len)
{
	struct _1905_map_device  *own_dev;
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);
	struct bss_info_db *bss;
	unsigned char temp_buf[300];
	int i=0,num_mac=0;
	if (!own_dev) {
		err("own 1905 dev is missing\n");
		return -1;
	}

	buf_Len=0;
	SLIST_FOREACH(bss, &own_dev->first_bss, next_bss) {
		if (!bss->radio) {
			err("radio for bss not found\n");
		}
		if(bss->map_vendor_extn & FH_BSS) {
			memcpy(&temp_buf[i], bss->bssid, ETH_ALEN);
			i=i+6;
			buf_Len+=6;
		}
	}

	num_mac=buf_Len/6;
	buf_Len=0;
	for (i=0;i<num_mac;i++)
		{	os_snprintf(buf+(i*18),19,MACSTR";",MAC2STR(temp_buf+(i*6)));
			buf_Len+=18;
		}
	buf_Len+=1;//For '\0'
	mapd_printf(MSG_ERROR,"mapd.c %s , reply=%zd\n", buf, buf_Len);
	return buf_Len;
}

#ifdef MAP_R2
void Send_Failed_assoc_message(struct _1905_context *ctx, struct own_1905_device *own_dev, u8 *sta_mac_address,
		u16 assoc_sts_cd, u16 assoc_reason_code)
{
		struct _1905_map_device *dev = topo_srv_get_controller_device(own_dev);

		map_1905_Send_Failed_Assoc_message(ctx, (char *)dev->_1905_info.al_mac_addr, sta_mac_address, assoc_sts_cd, assoc_reason_code);
}
#endif


int mapd_get_bh_config(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len) {

	wlanif_issue_wapp_command(global, WAPP_USER_GET_BH_WIRELESS_SETTING,
						WAPP_MAP_BH_CONFIG, NULL, NULL, NULL, 0, 0, 0, 0);

	return 0;
}
#endif /* #ifdef SUPPORT_MULTI_AP */
int mapd_reset_csbc(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len)
{
	char *pos = buf;
	char *end = buf + buf_len;
	int ret = 0;
	uint32_t client_id;

	client_id = atoi(cmd_buf);

	if ((client_id < 0) || (client_id >= MAX_STA_SEEN)) {
		ret = os_snprintf(pos, end - pos, "FAILED: Invalid client ID");
	} else {
		if (steer_action_reset_csbc(global, client_id, 0)) {
			ret = os_snprintf(pos, end - pos, "FAILED: Steering in Progress/Invalid client_id");
		} else {
			ret = os_snprintf(pos, end - pos, "OK");
		}
	}
	if (os_snprintf_error(end - pos, ret)) {
		*pos = '\0';
	} else
		pos += ret;
	return pos - buf;
}
#ifdef SUPPORT_MULTI_AP
int mapd_set_metric_policy_param (struct mapd_global *global, char *cmd_buf)
{
	char *token = NULL;
	unsigned char param[3], i = 0;
	unsigned char Radio_band,param_id,value;
	struct _1905_map_device *tmp_dev, *own_dev;
#define PARAM_ID_RCPI_THRESH 0
#define PARAM_ID_HYS 1
#define PARAM_ID_METRIC_INCLUSION 2
#define PARAM_ID_TRAFFIC_INCLUSION 3
#define PARAM_ID_CH_UTIL_THRESH 4
#define MAX_RCPI_THRESH 220
#define MAX_HYS 5
#define MAX_CU_UTIL_THRESH 255
	token = strtok(cmd_buf, " ");
	while (token != NULL) {
		param[i] = atoi(token);
		i = i + 1;
		token = strtok(NULL, " ");
   }
	Radio_band = param[0];
	param_id = param[1];
	value = param[2];
	always("Radio_band %d\n", Radio_band);
	always("paramid %d\n", param_id);
	always("value %d\n", value);
	if((Radio_band < BAND_2G) || (Radio_band > BAND_5GH)){
		err("valid value for radio band is 2G(1) , 5GL(2) , 5GH(3)");
		return -1;
	}
	if(param_id > PARAM_ID_CH_UTIL_THRESH){
		err("invalid param ID(0 to 4)");
		return -1;
	}
	if(param_id == PARAM_ID_RCPI_THRESH){
		if((value < 0) || (value > MAX_RCPI_THRESH)){
			err("RCPI Threshold out of range(0-220)");
			return -1;
		}
		global->dev.controller_context.ap_metric_policy.policy_params[Radio_band-1].MetricPolicyRcpi = value;
	}else if(param_id == PARAM_ID_HYS){
		if((value < 0) || (value > MAX_HYS)){
			err("HYSTERESIS out of range");
			return -1;
		}
		global->dev.controller_context.ap_metric_policy.policy_params[Radio_band-1].MetricPolicyHys = value;
	}else if(param_id == PARAM_ID_METRIC_INCLUSION){
		if((value != 0) && (value != 1)){
			err("Invalid value");
			return -1;
		}
		global->dev.controller_context.ap_metric_policy.policy_params[Radio_band-1].MetricPolicyMetricsInclusion = value;
	}else if(param_id == PARAM_ID_TRAFFIC_INCLUSION){
		if((value != 0) && (value != 1)){
			err("Invalid value");
			return -1;
		}
		global->dev.controller_context.ap_metric_policy.policy_params[Radio_band-1].MetricPolicyTrafficInclusion = value;
	}else if(param_id == PARAM_ID_CH_UTIL_THRESH){
		if((value < 0) || (value > MAX_CU_UTIL_THRESH)){
			err("CU Util value out of range(0-255)");
			return -1;
		}
		global->dev.controller_context.ap_metric_policy.policy_params[Radio_band-1].MetricPolicyChUtilThres = value;
	}
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);
	tmp_dev = topo_srv_get_next_1905_device(&global->dev,own_dev); /*Get the next 1905 device*/
	//run loop to update policy at all the 1905 dev
	while(tmp_dev)
	{
		mapd_printf(MSG_ERROR,"(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(tmp_dev->_1905_info.al_mac_addr));
		steer_msg_update_policy_config(global, tmp_dev);
		tmp_dev = topo_srv_get_next_1905_device(&global->dev,tmp_dev); /*Get the next 1905 device*/
	}
	return 0;
}
#ifdef MAP_R2
int trigger_metric_msg (struct mapd_global *global, char *cmd_buf)
{
	char *token = NULL;
	unsigned char param[3], i = 0;
	unsigned char rd_cnt, bss_cnt, value, bss_cnt_tmp, rd_cnt_tmp;
	struct _1905_map_device *tmp_dev, *own_dev;
	struct own_1905_device *ctx = &global->dev;
	unsigned char radio_list[MAX_NUM_OF_RADIO][ETH_ALEN];
	unsigned char bssid_list[MAX_NUM_OF_BSS_PER_RADIO][ETH_ALEN];
	unsigned char *tmp = NULL;
	struct bss_info_db *bss = NULL;
	struct radio_info_db *radio = NULL;
	struct associated_clients *client_info = NULL;
	token = strtok(cmd_buf, " ");
	while (token != NULL) {
		param[i] = atoi(token);
		i = i + 1;
		token = strtok(NULL, " ");
   }
	rd_cnt = param[0];
	bss_cnt = param[1];
	value = param[2];
	always("rd_cnt %d\n", rd_cnt);
	always("bss_cnt %d\n", bss_cnt);
	always("value %d\n", value);
	rd_cnt_tmp = rd_cnt;
	bss_cnt_tmp = bss_cnt;
	if(rd_cnt > 3 || rd_cnt < 1){
		err("valid value for radio cnt");
		return -1;
	}
	own_dev = topo_srv_get_1905_device(ctx, NULL);
	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		if (own_dev == tmp_dev)
			continue;
		tmp = &bssid_list[0][0];
		SLIST_FOREACH(bss, &tmp_dev->first_bss, next_bss) {
			if (bss_cnt_tmp == 0)
				break;
			os_memcpy(tmp, bss->bssid, ETH_ALEN);
			err("bssid: "MACSTR, PRINT_MAC(bss->bssid));
			tmp += ETH_ALEN;
			bss_cnt_tmp--;
		}
		tmp = &radio_list[0][0];
		SLIST_FOREACH(radio, &tmp_dev->first_radio, next_radio) {
			if (rd_cnt_tmp == 0)
				break;
			os_memcpy(tmp, radio->identifier, ETH_ALEN);
			err("raid: "MACSTR, PRINT_MAC(radio->identifier));
			tmp += ETH_ALEN;
			rd_cnt_tmp--;
		}
		if (value == 0) {
			if (tmp_dev->map_version == DEV_TYPE_R2)
				map_1905_Send_AP_Metrics_Query_Message(global->_1905_ctrl, (char *)(tmp_dev->_1905_info.al_mac_addr),
					(s8)bss_cnt, (u8 *)bssid_list, (s8)rd_cnt, (u8 *)radio_list);
			else
				map_1905_Send_AP_Metrics_Query_Message(global->_1905_ctrl, (char *)(tmp_dev->_1905_info.al_mac_addr),
					(s8)bss_cnt, (u8 *)bssid_list, 0, NULL);
		} else {
		SLIST_FOREACH(client_info, &tmp_dev->assoc_clients, next_client) {
			map_1905_Send_Associated_STA_Link_Metrics_Query_Message(global->_1905_ctrl,
				(char *)tmp_dev->_1905_info.al_mac_addr, client_info->client_addr);
		}
		}
	}

	return 0;
}

int trigger_cac_msg (struct mapd_global *global, char *cmd_buf)
{
	char *token = NULL;
	unsigned char param[6] = {0}, i = 0;
	unsigned char rd_cnt, value, rd_cnt_tmp, ch, op, cac_method, cac_action;
	struct _1905_map_device *tmp_dev, *own_dev;
	struct own_1905_device *ctx = &global->dev;
	struct radio_info_db *radio = NULL;
	struct cac_request * req = NULL;
	struct cac_terminate *term = NULL;
	token = strtok(cmd_buf, " ");
	while (token != NULL) {
		param[i] = atoi(token);
		i = i + 1;
		token = strtok(NULL, " ");
	}
	rd_cnt = param[0];
	value = param[1];
	ch = param[2];
	op = param[3];
	cac_method = param[4];
	cac_action = param[5];
	always("rd_cnt %d\n", rd_cnt);
	always("req/terminate %d\n", value);
	always("channel: %d", ch);
	always("op class: %d", op);
	req = os_zalloc(sizeof(struct cac_request) + rd_cnt * sizeof(struct cac_tlv));
	if (req == NULL) {
		err("Malloc failed");
		return -1;
	}
	term = os_zalloc(sizeof(struct cac_terminate) + rd_cnt * sizeof(struct cac_term_tlv));
	if (term == NULL) {
		err("Malloc failed");
		os_free(req);
		return -1;
	}
	req->num_radio = rd_cnt;
	term->num_radio = rd_cnt;
	rd_cnt_tmp = rd_cnt;
	if(rd_cnt > 3 || rd_cnt < 1){
		err("valid value for radio cnt");
		os_free(term);
		os_free(req);
		return -1;
	}
	own_dev = topo_srv_get_1905_device(ctx, NULL);
	ctx->user_triggered_cac = 1;
	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		if (own_dev == tmp_dev)
			continue;
		SLIST_FOREACH(radio, &tmp_dev->first_radio, next_radio) {
			if (rd_cnt_tmp == 0)
				break;
			os_memcpy(req->tlv[rd_cnt-rd_cnt_tmp].identifier, radio->identifier, ETH_ALEN);
			req->tlv[rd_cnt-rd_cnt_tmp].cac_action = cac_action;
			req->tlv[rd_cnt-rd_cnt_tmp].cac_method = cac_method;
			req->tlv[rd_cnt-rd_cnt_tmp].ch_num = ch;
			req->tlv[rd_cnt-rd_cnt_tmp].op_class_num = op;
			err("operating class: %d", req->tlv[rd_cnt-rd_cnt_tmp].op_class_num);
			os_memcpy(term->tlv[rd_cnt-rd_cnt_tmp].identifier, radio->identifier, ETH_ALEN);
			term->tlv[rd_cnt-rd_cnt_tmp].ch_num = ch;
			term->tlv[rd_cnt-rd_cnt_tmp].op_class_num = op;
			err("raid: "MACSTR, PRINT_MAC(radio->identifier));
			rd_cnt_tmp--;
		}
		if (value == 0) {
			map_1905_Send_CAC_Request(global->_1905_ctrl, (char *)tmp_dev->_1905_info.al_mac_addr, req);
		} else
			map_1905_Send_CAC_Terminate(global->_1905_ctrl, (char *)tmp_dev->_1905_info.al_mac_addr, term);
	}
	os_free(req);
	os_free(term);
	return 0;
}
void ch_planning_ch_selection_prefer_data_trigger(
	unsigned char channel,
	struct radio_info_db *radio,
	struct ch_prefer_lib *ch_prefer)
{
	struct prefer_info_db *prefer_info = NULL;
	unsigned char selection_count = 0;
	err("%s\n", __FUNCTION__);
	SLIST_FOREACH(prefer_info,
		&(radio->chan_preferance.prefer_info_head),
		prefer_info_entry) {
		if (prefer_info->op_class <= 130)
		{
			int i =0;
			for (i = 0; i < prefer_info->ch_num; i++) {
				if (channel
					== prefer_info->ch_list[i])
				{
					os_memcpy(ch_prefer->identifier, radio->identifier, ETH_ALEN);
					ch_prefer->opinfo[selection_count].op_class =
						prefer_info->op_class;
					ch_prefer->opinfo[selection_count].ch_num = 1;
					ch_prefer->opinfo[selection_count].ch_list[0] =
						channel;
					ch_prefer->opinfo[selection_count].perference = 14;
					selection_count++;
				}
			}
		}

	}
	ch_prefer->op_class_num = selection_count;
}

int trigger_ch_sel_msg (struct mapd_global *global, char *cmd_buf)
{
	char *token = NULL;
	unsigned char param[2] = {0}, i = 0;
	unsigned char cac_req = 1, ch = 100;
	struct _1905_map_device *tmp_dev, *own_dev;
	struct own_1905_device *ctx = &global->dev;
	struct radio_info_db *radio = NULL;
	unsigned char ch_prefer_count = 0;
	struct ch_prefer_lib *ch_prefer;

	ch_prefer = os_zalloc(sizeof(struct ch_prefer_lib) * 3);
	if (!ch_prefer)
		return 0;
	token = strtok(cmd_buf, " ");
	while (token != NULL) {
		param[i] = atoi(token);
		i = i + 1;
		token = strtok(NULL, " ");
	}
	cac_req = param[0];
	ch = param[1];
	always("cac req %d\n", cac_req);
	own_dev = topo_srv_get_1905_device(ctx, NULL);
	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		if (own_dev == tmp_dev)
			continue;
		SLIST_FOREACH(radio, &(tmp_dev->first_radio), next_radio) {
			ch_planning_ch_selection_prefer_data_trigger(ch,
						radio, &ch_prefer[ch_prefer_count]);
			for (i = 0;i < ch_prefer[ch_prefer_count].op_class_num; i++) {
				if (cac_req == 0)
					ch_prefer[ch_prefer_count].opinfo[i].reason = ch_prefer[ch_prefer_count].opinfo[i].reason | DFS_CH_CLEAR_INDICATION;
			}
			ch_prefer_count++;
		}
		map_1905_Send_Channel_Selection_Request_Message(
				global->_1905_ctrl,
				(char *)tmp_dev->_1905_info.al_mac_addr,
				ch_prefer_count,
				ch_prefer,
				0,
				NULL);
	}
	os_free(ch_prefer);
	return 0;
}

struct metric_report_policy_params * get_metric_policy_param_from_band(
	struct mapd_global *global,u8 band)
{
	struct metric_report_policy_params *policy_params;
	policy_params =
		&global->dev.controller_context.ap_metric_policy.policy_params[band-1];
	return policy_params;
}

int update_policy_config(struct mapd_global *global, struct _1905_map_device *dev, int metric_inclusion)
{
	struct lib_steer_radio_policy *radio_policy;
	struct radio_info_db *radio;
	struct metric_report_policy_params *policy_params;
	u8 radio_cnt = 0,band = 0;
	u8 i=0;
	struct lib_metrics_radio_policy *metrics_policy = NULL;
	struct lib_unsuccess_assoc_policy *assoc_policy = os_zalloc(sizeof(struct lib_unsuccess_assoc_policy));

	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		radio_cnt++;
	}
	radio_policy = os_zalloc(sizeof(struct lib_steer_radio_policy) * radio_cnt);
	metrics_policy = os_zalloc(sizeof(struct lib_metrics_radio_policy) * radio_cnt);
	if(metrics_policy == NULL || radio_policy == NULL) {
		mapd_ASSERT(0);
		if(metrics_policy) {
			os_free(metrics_policy);
		}
		if(radio_policy){
			os_free(radio_policy);
		}

		if(assoc_policy)
			os_free(assoc_policy);

		return -1;
	}
	if(assoc_policy == NULL) {
		os_free(metrics_policy);
		os_free(radio_policy);
		mapd_ASSERT(0);
		return -1;
	}
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		os_memcpy(radio_policy[i].identifier, radio->identifier,ETH_ALEN);
		radio_policy[i].steer_policy = AGENT_STEER_RSSI_ALLOWD;
		if(isChan5GH(radio->channel[0])) {
			band = BAND_5GH;
			radio_policy[i].ch_util_thres = global->dev.cli_steer_params.CUOverloadTh_5G_H;
		} else if (isChan5GL(radio->channel[0])) {
			band = BAND_5GL;
			radio_policy[i].ch_util_thres = global->dev.cli_steer_params.CUOverloadTh_5G_L;
		} else if(radio->channel[0] <= 14) {
			band = BAND_2G;
			radio_policy[i].ch_util_thres = global->dev.cli_steer_params.CUOverloadTh_2G;
		} else {
			err("Invalid Channel\n");
			os_free(radio_policy);
			os_free(metrics_policy);
			os_free(assoc_policy);
			return -1;
		}
		radio_policy[i].rssi_thres =rssi_to_rcpi( global->dev.cli_steer_params.LowRSSIAPSteerEdge_RE + (NOISE_OFFSET_BY_CH_WIDTH[0]));
		os_memcpy(metrics_policy[i].identifier, radio->identifier, ETH_ALEN);
		policy_params = get_metric_policy_param_from_band(global, band);
		metrics_policy[i].metrics_inclusion = policy_params->MetricPolicyMetricsInclusion;
		if (metric_inclusion == 0)
			metrics_policy[i].traffic_inclusion = policy_params->MetricPolicyTrafficInclusion;
		else
			metrics_policy[i].traffic_inclusion = metric_inclusion;
		metrics_policy[i].rssi_thres = policy_params->MetricPolicyRcpi;
		metrics_policy[i].rssi_margin = policy_params->MetricPolicyHys;
		metrics_policy[i].ch_util_thres = policy_params->MetricPolicyChUtilThres;
		i++;
	}
	assoc_policy->report_switch = 1;
	assoc_policy->report_rate = 10;
	err(" due to DE ");
	if(global->dev.ch_planning_R2.ch_plan_enable && (global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE))
		if (dev->map_version == DEV_TYPE_R2)
			map_1905_Send_MAP_Policy_Request_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr, 0,NULL,
				0, NULL, radio_cnt, radio_policy,global->dev.ch_planning_R2.ch_plan_metric_policy_interval, radio_cnt, metrics_policy, 1, 0, 1, assoc_policy);
		else
			map_1905_Send_MAP_Policy_Request_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr, 0,NULL,
				0, NULL, radio_cnt, radio_policy,global->dev.ch_planning_R2.ch_plan_metric_policy_interval, radio_cnt, metrics_policy,0, 0, 0, NULL);
	else
		if (dev->map_version == DEV_TYPE_R2)
			map_1905_Send_MAP_Policy_Request_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr, 0,NULL,
				0, NULL, radio_cnt, radio_policy,global->dev.cli_steer_params.CUAvgPeriod, radio_cnt, metrics_policy, 1, 0, 1, assoc_policy);
		else
			map_1905_Send_MAP_Policy_Request_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr, 0,NULL,
				0, NULL, radio_cnt, radio_policy,global->dev.cli_steer_params.CUAvgPeriod, radio_cnt, metrics_policy, 0, 0, 0, NULL);
	os_free(radio_policy);
	os_free(metrics_policy);
	os_free(assoc_policy);
	return 0;
}

int get_own_de_info(struct mapd_global *global, struct _1905_map_device *dev)
{
	struct radio_info_db *radio = NULL;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		wlanif_issue_wapp_command(global, WAPP_USER_GET_RADIO_METRICS_INFO,
					WAPP_RADIO_METRICS_INFO, NULL, NULL, radio->identifier, ETH_ALEN, 1, 1, 0);
		wlanif_get_all_assoc_sta_ext_link_metrics(global, radio->identifier);
	}
	topo_srv_get_own_metrics_info(&global->dev);
	dev->de_done = 0;
	return 0;
}

int get_de_stats_from_device(struct mapd_global *global, struct _1905_map_device *dev)
{
	unsigned char radio_list[MAX_NUM_OF_RADIO][ETH_ALEN];
	unsigned char bssid_list[MAX_NUM_OF_BSS_PER_RADIO][ETH_ALEN];	// Send config policy request set traffic stats inclusion
	unsigned char *tmp = NULL;
	struct bss_info_db *bss = NULL;
	struct radio_info_db *radio = NULL;
	int rd_cnt = 0, bss_cnt = 0;

	update_policy_config(global, dev, 1);
	// Trigger AP metric query
	tmp = &bssid_list[0][0];
	SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
		os_memcpy(tmp, bss->bssid, ETH_ALEN);
		err("bssid: "MACSTR, PRINT_MAC(bss->bssid));
		tmp += ETH_ALEN;
		bss_cnt++;
	}
	tmp = &radio_list[0][0];
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		os_memcpy(tmp, radio->identifier, ETH_ALEN);
		err("raid: "MACSTR, PRINT_MAC(radio->identifier));
		tmp += ETH_ALEN;
		rd_cnt++;
	}
	map_1905_Send_AP_Metrics_Query_Message(global->_1905_ctrl, (char *)(dev->_1905_info.al_mac_addr),
			(s8)bss_cnt, (u8 *)bssid_list, (s8)rd_cnt, (u8 *)radio_list);
	// When response comes, Reset config policy request traffic stats inclusion
	// Trigger channel scan
	//send_channel_scan_req(global, dev);
	// Start a 5 mins timer
	err("5 secs timer for DE registered");
	eloop_register_timeout(5, 0, de_stats_timeout, global, dev);
	return 0;
}
int mapd_get_de_stats(struct mapd_global *global, char *cmd_buf)
{

	unsigned char al_mac[ETH_ALEN] = {0};
	struct _1905_map_device *dev, *own_dev = NULL;
	struct own_1905_device *ctx = &global->dev;
	u8 bcast_mac[ETH_ALEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	if (hwaddr_aton(cmd_buf, al_mac) < 0) {
		return -1;
	}

	mapd_printf(MSG_ERROR,"Get DE STATS from device with MAC %02x:%02x:%02x:%02x:%02x:%02x \n",PRINT_MAC(al_mac));
	if (global->dev.ch_planning.ch_planning_state != CHANNEL_PLANNING_IDLE) {
		err("ERR:Channel Planning Ongoing");
		return 1;
	}
	if (global->dev.network_optimization.network_opt_state != NETOPT_STATE_IDLE) {
		err("ERR:Network Optimization Ongoing");
		return 2;
	}
	if (global->dev.de_state != OFF) {
		err("ERR:Data Elements Wrong State");
		return 3;
	}
	//global->dev.de_cnt = 0;
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);
	if (!own_dev) {
		err("no dev");
		return -1;
	}
	if (own_dev->device_role == DEVICE_ROLE_CONTROLLER) {
		if (os_memcmp(al_mac, bcast_mac, ETH_ALEN) == 0) {
			// Sending for all agents
			global->dev.de_state = ALL_AGENTS;
			SLIST_FOREACH(dev, &ctx->_1905_dev_head, next_1905_device) {
				dev->de_done = 1;
				if (os_memcmp(own_dev->_1905_info.al_mac_addr, dev->_1905_info.al_mac_addr, ETH_ALEN) == 0) {
					err("get ow de info");
					get_own_de_info(global, dev);
				} else {
					//global->dev.de_cnt++;
					err("get de stats from dev");
					get_de_stats_from_device(global, dev);
				}
			}
		} else {
			// For single agent
			global->dev.de_state = SINGLE_AGENT;
			err("single agent");
			dev = topo_srv_get_1905_device(&global->dev, al_mac);
			dev->de_done = 1;
			if (os_memcmp(own_dev->_1905_info.al_mac_addr, dev->_1905_info.al_mac_addr, ETH_ALEN) == 0){
				err("own de info");
				get_own_de_info(global, dev);
			}else {
			err("get agent info");
				get_de_stats_from_device(global, dev);
			}
		}
		reset_de_if_needed(&global->dev);
	}

	return 0;
}

int get_default_radio_policy(
	struct own_1905_device *own_dev,
	struct radio_info_db *radio,
	struct lib_steer_radio_policy *radio_policy)
{
	os_memcpy(radio_policy->identifier, radio->identifier, ETH_ALEN);
	radio_policy->steer_policy = AGENT_STEER_RSSI_ALLOWD;
	if(isChan5GH(radio->channel[0])) {
		radio_policy->ch_util_thres =
			own_dev->cli_steer_params.CUOverloadTh_5G_H;
	} else if (isChan5GL(radio->channel[0])) {
		radio_policy->ch_util_thres =
			own_dev->cli_steer_params.CUOverloadTh_5G_L;
	} else if (radio->channel[0] <= 14) {
		radio_policy->ch_util_thres =
			own_dev->cli_steer_params.CUOverloadTh_2G;
	} else {
		err("Invalid Channel\n");
		os_free(radio_policy);
		return -1;
	}
	radio_policy->rssi_thres =
		rssi_to_rcpi(own_dev->cli_steer_params.LowRSSIAPSteerEdge_RE +
		(NOISE_OFFSET_BY_CH_WIDTH[0]));
	return 0;
}

void own_dev_get_metric_info(
	struct mapd_global *global,
	struct radio_info_db *radio)
{
	u8 i = 0;
	struct mapd_radio_info *ra_info = NULL;
	struct mapd_bss *bss = NULL;
	/* Iterate over all BSS on this device */
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (u8)-1)
			continue;
		bss = NULL;
		if(ra_info->channel != radio->channel[0])
			continue;
		debug("ra_identifier "MACSTR"ra_info->channel %d ",MAC2STR(radio->identifier), ra_info->channel);
		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {

			if (wlanif_get_ap_metrics_info(global, bss->bssid) != 0) {
				mapd_printf(MSG_ERROR, "%s: FAILED for bss=" MACSTR,
								__func__, MAC2STR(bss->bssid));
			} else
				break;
		}
		wlanif_issue_wapp_command(global, WAPP_USER_GET_RADIO_METRICS_INFO,
			WAPP_RADIO_METRICS_INFO, NULL, NULL, radio->identifier,
				ETH_ALEN, 1, 1, 0);
	}
}
u8 check_if_task_exist(struct own_1905_device *own_dev,
	u8 task_type,
	struct monitor_ch_info *ch_info)
{
	struct task_info *task = NULL;
	SLIST_FOREACH(task, &(own_dev->task_list_head), next_task) {
		if(task->task_type != task_type) {
			err(" ");
			continue;
		}
		if((task_type == TASK_USER_TRIGGERED_SCAN) ||
			(task_type == TASK_NETWORK_OPT_TRIGGER)) {
			err("user triggered scan req/netopt is already pending return");
			return 0;
		} else {
			if(ch_info->channel_num == task->ch_info->channel_num) {
				err("this ch is already in monitor list ");
				return 0;
			}
		}
	}
	err("return 1");
	return 1;
}

void insert_into_task_list(struct own_1905_device *own_dev,
	u8 task_type,
	struct monitor_ch_info *ch_info,
	struct channel_scan_req *scan_req,
	u8 *almac)
{
	struct task_info *task = NULL, *prev_task = NULL, *task_list_entry = NULL;
	u8 insert_new = 0;
	insert_new = check_if_task_exist(own_dev,task_type,ch_info);
	if(insert_new == 0)
		return;
	task= os_zalloc(sizeof(struct task_info));
	if (task == NULL) {
		err("alloc memory fail");
		assert(0);
		return;
	}
	task->task_type = task_type;
//add more if required
	if(ch_info)
		task->ch_info = ch_info;
	if(scan_req)
		task->scan_req = scan_req;
	if(almac)
		os_memcpy(task->almac,almac,ETH_ALEN);

	/*find the last task in the list */
	SLIST_FOREACH(task_list_entry, &(own_dev->task_list_head), next_task) {
		prev_task = task_list_entry;
	}
	if (prev_task != NULL) {
		err("Add after task type  = %d\n",
			prev_task->task_type);
		SLIST_INSERT_AFTER(prev_task, task, next_task);
	} else {
		err("Insert into head\n");
		SLIST_INSERT_HEAD(
			&(own_dev->task_list_head),
			task,
			next_task);
	}
}

void find_and_remove_pending_task(
	struct own_1905_device *own_dev,
	u8 pending_task_type)
{
	err(" ");
	struct task_info *new_task;
	if(SLIST_EMPTY (&(own_dev->task_list_head)))
	{
		err("no pending task");
		return;
	}
	err("Remove all pending tasks for task type %d", pending_task_type);
	SLIST_FOREACH(new_task, &(own_dev->task_list_head), next_task) {
		if(new_task->task_type == pending_task_type)
		{
			SLIST_REMOVE(&(own_dev->task_list_head),
				new_task,
				task_info,
				next_task);
			os_free(new_task);
			if(SLIST_EMPTY (&(own_dev->task_list_head)))
				break;
		}
	}
}

void handle_task_completion(
	struct own_1905_device *own_dev)
{

	struct task_info *new_task;
	struct mapd_global *global = own_dev->back_ptr;
	if(SLIST_EMPTY (&(own_dev->task_list_head)))
	{
		err("no pending task");
		return;
	}

	new_task = SLIST_FIRST(&(own_dev->task_list_head));
	err("Handle pending task %d", new_task->task_type);
	if(new_task->task_type == TASK_USER_TRIGGERED_SCAN)
	{
		struct _1905_map_device *scan_dev = topo_srv_get_1905_device(own_dev,new_task->almac);
		if(!scan_dev) {
			err("scan_dev for user triggered scan not found");
			SLIST_REMOVE_HEAD(&(own_dev->task_list_head), next_task);
			os_free(new_task);
			return;
		}
		if(scan_dev->device_role == DEVICE_ROLE_CONTROLLER) {
			u16 length;
			err("scan command is meant for owndev  itself");
			length = sizeof(struct channel_scan_req) + new_task->scan_req->radio_num*sizeof(struct scan_body);
			new_task->scan_req->neighbour_only = 2;//NB_ALL;
			map_get_info_from_wapp(&global->dev, WAPP_USER_SET_CHANNEL_SCAN_REQ, 0, NULL, NULL, (void *)new_task->scan_req, length);
		} else if(scan_dev->device_role == DEVICE_ROLE_AGENT){
			mapd_hexdump(MSG_OFF, "agent's scan req", (u8*)new_task->scan_req, 100);
			err(" scan_req->radio_num %d", new_task->scan_req->radio_num);
			map_1905_Send_Channel_Scan_Request_Message(
			global->_1905_ctrl,
			(char *)new_task->almac,
			new_task->scan_req->fresh_scan,
			new_task->scan_req->radio_num,
			(unsigned char *)new_task->scan_req->body);
		}
		global->dev.user_triggered_scan = 1;
		// Start a 5 mins timer
		err("start ch scan 5min req timer");
		eloop_register_timeout(CH_SCAN_TIMEOUT, 0, ch_scan_req_timeout, global, scan_dev);
		os_free(new_task->scan_req);
	} else if (new_task->task_type == TASK_NETWORK_OPT_TRIGGER) {
		own_dev->network_optimization.network_optimization_enabled = 1;
	} else if (new_task->task_type == TASK_CHANNEL_PLANNING_TRIGGER) {
		eloop_register_timeout(0,0,channel_monitor_timeout,own_dev,new_task->ch_info);
	}
	SLIST_REMOVE_HEAD(&(own_dev->task_list_head), next_task);
	os_free(new_task);
}
int map_cmd_ch_scan_req_demo( struct mapd_global *global, char *cmd_buf)
{
	u8 almac[ETH_ALEN] = {0};
	struct _1905_map_device *dev_1905 = NULL, *own_dev = NULL;
	struct channel_scan_req *scan_req = NULL;
	struct radio_info_db *radio = NULL;
	struct _1905_map_device *_1905_device = NULL;
	u8 buf[8000] = {0};
	u8 boot_only_scan = 0;
	u32 len;
	u8 val2 = 1;
	u8 scan_own = 0;
	u8 is_triband = 0;

	if (hwaddr_aton(cmd_buf, almac) < 0) {
		return -1;
	}
	if (global->dev.ch_planning_R2.ch_plan_enable == FALSE ||
		global->dev.ch_planning.ch_planning_state != CHANNEL_PLANNING_IDLE ||
		global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE ||
		global->dev.network_optimization.network_opt_state != NETOPT_STATE_IDLE ||
		global->dev.user_triggered_scan == TRUE) {
		err("ERR:Channel Planning Ongoing return 3");
		return 3;
	}

	mapd_printf(MSG_ERROR,"Trigger scan on dev with ALMAC %02x:%02x:%02x:%02x:%02x:%02x \n",PRINT_MAC(almac));

	dev_1905 = topo_srv_get_1905_by_iface_addr(&global->dev,(u8 *)almac);
	if(dev_1905 == NULL) {
		mapd_printf(MSG_ERROR,"1905 Device not found\n");
		return -1 ;
	}



//add valid check if the ALMAC in the scan request is for a profile2 type agent
	_1905_device = topo_srv_get_1905_device(&global->dev,almac);
	if (!_1905_device) {
		err("This 1905 device almac is not valid");
		return -1;
	} else if (0){
		err("this ALMAC does not belong to a profile2 MAP dev");
		return -1;
	}
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);

#if 1
	scan_req = (struct channel_scan_req *)buf;

	scan_req->fresh_scan= 0x80;//(val == 1?0x80:0);//hardcode fresh scan
	if (os_memcmp(own_dev->_1905_info.al_mac_addr,_1905_device->_1905_info.al_mac_addr, ETH_ALEN) == 0)
		scan_own = 1;
	val2 = 1;//for fresh scan
//the on boot only capability for this ALMAC is 0 only then AP can send Fresh scan =1 command
	if ((scan_req->fresh_scan) && (boot_only_scan == 1)) {
		err("controller can't send fresh scan req to agent with boot only scan cap");
		return -1;
	}
//check each radio in comand if it is supported or not
	scan_req->radio_num = 0;

	is_triband = check_is_triband(_1905_device);

	 SLIST_FOREACH(radio, &_1905_device->first_radio, next_radio){
		os_memcpy(scan_req->body[scan_req->radio_num].radio_id, radio->identifier, ETH_ALEN);
		if(val2 == 1) {
		  if(radio->band == BAND_2G) {
			 scan_req->body[scan_req->radio_num].oper_class_num = 2;
			 scan_req->body[scan_req->radio_num].ch_body[0].oper_class = 81;
			 scan_req->body[scan_req->radio_num].ch_body[0].ch_list_num = 0;
			 scan_req->body[scan_req->radio_num].ch_body[1].oper_class = 82;
			 scan_req->body[scan_req->radio_num].ch_body[1].ch_list_num = 0;

		  }else {
			 if(is_triband) {
			   if(radio->band == BAND_5GL) {
				 scan_req->body[scan_req->radio_num].oper_class_num = 2;
				 scan_req->body[scan_req->radio_num].ch_body[0].oper_class = 118;
				 scan_req->body[scan_req->radio_num].ch_body[0].ch_list_num = 0;
				 scan_req->body[scan_req->radio_num].ch_body[1].oper_class = 115;
				 scan_req->body[scan_req->radio_num].ch_body[1].ch_list_num = 0;
			   } else if (radio->band == BAND_5GH) {
				 scan_req->body[scan_req->radio_num].oper_class_num = 2;
				 scan_req->body[scan_req->radio_num].ch_body[0].oper_class = 125;
				 scan_req->body[scan_req->radio_num].ch_body[0].ch_list_num = 0;
				 scan_req->body[scan_req->radio_num].ch_body[1].oper_class = 121;
				 scan_req->body[scan_req->radio_num].ch_body[1].ch_list_num = 0;
			   }
			 } else {
			   scan_req->body[scan_req->radio_num].oper_class_num = 4;
			   scan_req->body[scan_req->radio_num].ch_body[0].oper_class = 125;
			   scan_req->body[scan_req->radio_num].ch_body[0].ch_list_num = 0;
			   scan_req->body[scan_req->radio_num].ch_body[1].oper_class = 121;
			   scan_req->body[scan_req->radio_num].ch_body[1].ch_list_num = 0;
			   scan_req->body[scan_req->radio_num].ch_body[2].oper_class = 118;
			   scan_req->body[scan_req->radio_num].ch_body[2].ch_list_num = 0;
			   scan_req->body[scan_req->radio_num].ch_body[3].oper_class = 115;
			   scan_req->body[scan_req->radio_num].ch_body[3].ch_list_num = 0;
			 }
		  }
		}
		scan_req->radio_num++;
	 }
	len = sizeof(struct channel_scan_req) + 3*sizeof(struct scan_body);
	if(((global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE) ||
		(global->dev.network_optimization.network_opt_state != NETOPT_STATE_IDLE))&&
		(global->dev.device_role == DEVICE_ROLE_CONTROLLER))
	{
		err("need to add to pending task list, defer this scan");
		struct channel_scan_req *scan_req_dup = os_zalloc(len);
		os_memcpy(scan_req_dup,scan_req,len);
		insert_into_task_list(&global->dev,TASK_USER_TRIGGERED_SCAN,NULL,scan_req_dup,almac);
		os_free(scan_req_dup);
	}
	else
	{
		global->dev.user_triggered_scan = 1;
		if(scan_own == 1) {
			u16 length;
			err("scan command is meant for owndev  itself");
			length = sizeof(struct channel_scan_req) + scan_req->radio_num*sizeof(struct scan_body);
			debug("Len = %d\n", length);
			scan_req->neighbour_only = 2;//NB_ALL;
			mapd_hexdump(MSG_OFF, "MAPD SCAN_REQ", buf, length);
			map_get_info_from_wapp(&global->dev, WAPP_USER_SET_CHANNEL_SCAN_REQ, 0, NULL, NULL, (void *)buf, length);
		}else {
			// Start a 5 mins timer
			eloop_register_timeout(CH_SCAN_TIMEOUT, 0, ch_scan_req_timeout, global, _1905_device);
			mapd_hexdump(MSG_OFF, "agent's scan req", (u8*)scan_req, 100);
			debug(" scan_req->radio_num %d",  scan_req->radio_num);
			map_1905_Send_Channel_Scan_Request_Message(global->_1905_ctrl,(char *)almac,scan_req->fresh_scan, scan_req->radio_num, (unsigned char *)scan_req->body);
		}
	}
#endif
	return 0;
	// fill send channel scan req
}

int map_cmd_ch_plan_R2_demo( struct mapd_global *global, char *cmd_buf)
{
	u8 band = BAND_2G;// 1 for 2G , 2 for 5G or 5GL , 3 for 5GH
	band = atoi(cmd_buf);
	err("BAND in command is %d", band);
	struct radio_info_db * radio = NULL;
	struct mapd_radio_info *radio_info = NULL;
	u8 is_triband = 0;
	struct _1905_map_device *_1905_dev = topo_srv_get_1905_device(&global->dev, NULL);

	if (global->dev.ch_planning_R2.ch_plan_enable == FALSE ||
		global->dev.ch_planning.ch_planning_state != CHANNEL_PLANNING_IDLE ||
		global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE ||
		global->dev.network_optimization.network_opt_state != NETOPT_STATE_IDLE ||
		global->dev.user_triggered_scan == TRUE) {
		err("ERR:Channel Planning Ongoing return 3");
		return 3;
	}

	is_triband = check_is_triband(_1905_dev);
	radio = topo_srv_get_next_radio(_1905_dev, radio);
	while(radio) {
		if(is_triband) {
			err("radio->band %d", radio->band);
			if(radio->band == band){
				err("radio found %d", radio->channel[0]);
				break;
			}
		} else {
			if(radio->band == BAND_2G && band == BAND_2G){
				err("radio found %d", radio->channel[0]);
				break;
			}else if(band == BAND_5GL && (radio->band == BAND_5GL || radio->band == BAND_5GH)){
				err("radio found %d", radio->channel[0]);
				break;
			}
		}
		radio = topo_srv_get_next_radio(_1905_dev, radio);
	};
	ch_planning_reset_user_preff_ch(global);

	if(radio) {
		radio_info = get_radio_info_by_radio_id(global,radio->identifier);
		if(radio_info){
			err("bootup run force done for ch %d", radio_info->channel);
			radio_info->bootup_run = 1;//since force trigger command has come
			ch_planning_R2_force_trigger(global, radio_info->channel);
		}
	}
	return 0;
}
int trigger_bh_sta_query (struct mapd_global *global, char *cmd_buf)
{
	u8 mac[ETH_ALEN] = {0};

	if (hwaddr_aton(cmd_buf, mac) < 0) {
		return -1;
	}
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, mac);
	if (!dev) {
		err("No 1905 dev");
		return -1;
	}
	if (global->dev.map_version == DEV_TYPE_R2 && dev->map_version == DEV_TYPE_R2) {
		err("Sending BH query to"MACSTR, MAC2STR(dev->_1905_info.al_mac_addr));
		map_1905_send_bh_sta_cap_query(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr);
	}
	return 0;
}

#endif
int mapd_set_bh_switch_cu_en(struct mapd_global *global, char *cmd_buf)
{
	int j = 0;
	unsigned char cu_en[4];
	while(cmd_buf[j] != ' ') {
		cu_en[j] = cmd_buf[j];
		j++;
		if(j == 4)
			break;
	}

	global->dev.bh_cu_params.bh_switch_cu_en = (unsigned char)atoi((const char *)cu_en);

	mapd_printf(MSG_ERROR, "bh switch by cu ol enable flag: %d\n", global->dev.bh_cu_params.bh_switch_cu_en);
	return 0;
}

int mapd_set_cu_maxcount_thresh(struct mapd_global *global, char *cmd_buf)
{
	int j = 0;
	unsigned char cu_thresh[10];
	while(cmd_buf[j] != ' ') {
		cu_thresh[j] = cmd_buf[j];
		j++;
		if(j == 10)
			break;
	}

	global->dev.bh_cu_params.BHOLSteerCountTh = (uint32_t)atoi((const char *)cu_thresh);

	mapd_printf(MSG_ERROR, "cu ol max count threshold: %d\n", global->dev.bh_cu_params.BHOLSteerCountTh);
	return 0;
}

int mapd_set_bh_cu_forbidtime_thresh(struct mapd_global *global, char *cmd_buf)
{
	int j = 0;
	unsigned char cu_thresh[10];
	while(cmd_buf[j] != ' ') {
		cu_thresh[j] = cmd_buf[j];
		j++;
		if(j == 10)
			break;
	}

	global->dev.bh_cu_params.BHOLForbidTime = (uint32_t)atoi((const char *)cu_thresh);

	mapd_printf(MSG_ERROR, "cu ol forbid time threshold: %d\n", global->dev.bh_cu_params.BHOLForbidTime);
	return 0;
}

//TODO merge this with map_cfg.txt
//#define MAPD_CFG_FILE "/etc/map/mapd_cfg"
int mapd_read_config_file(struct own_1905_device *dev)
{
	FILE *file;
	char buf[256], *pos, *token;
	char tmpbuf[256] = {0};
	signed int device_role = DEVICE_ROLE_INVALID;
	char avoid_scan_cac = 0;
	int line = 0;
	struct bh_link_entry *bh = NULL;
	unsigned char bh_priority_2g = 255;
	unsigned char bh_priority_5gh = 255;
	unsigned char bh_priority_5gl = 255;
	unsigned char user_prefered_channel = 0;
#ifdef MAP_R2
	FILE *file_1905;
#endif

	if (!dev) {
		err("own dev not found");
		return -1;
	}

	file = fopen(g_map_cfg_path, "r");

	if (!file) {
		err("open MAP cfg file (%s) fail\n", g_map_cfg_path);
		if (dev->device_role == DEVICE_ROLE_UNCONFIGURED)
			map_1905_controller_found(dev);
		return -1;
	}
#ifdef MAP_R2
	file_1905 = fopen(g_map_1905_cfg_path, "r");

	if (!file_1905) {
		err("open MAP cfg file (%s) fail\n", g_map_1905_cfg_path);
	}
#endif

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);
	dev->bh_cu_params.BHOLForbidTime = MAX_BH_OL_FORBID_TIME;
	dev->bh_cu_params.BHOLSteerCountTh = MAX_BH_OL_STEER_COUNT;
	dev->bh_steer_timeout = DEFAULT_BH_STEER_TIMEOUT;
	dev->max_allowed_scan = MAX_ALLOWED_SEC_LINK_SCAN;
	while (mapd_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		os_snprintf(tmpbuf, sizeof(tmpbuf), "%s", pos);
		token = os_strtok(pos, "=");

		if (token != NULL) {
			if (os_strcmp(token, "lan_interface") == 0) {
				token = os_strtok(NULL, "");
				if(token){
					os_memcpy((char *)dev->lan_iface, token, sizeof(dev->lan_iface));
					err("lan interface is %s\n", dev->lan_iface);
				}
			} else if (os_strcmp(token, "wan_interface") == 0) {
				token = os_strtok(NULL, "");
				if(token){
					os_memcpy((char *)dev->wan_iface, token, sizeof(dev->wan_iface));
					err("wan interface is %s\n", dev->wan_iface);
				}
			} else if (os_strcmp(token, "DeviceRole") == 0) {
				token = os_strtok(NULL, "");
				if(token){
					err("device role is %s\n", token);
					if (os_strcmp(token, "0") == 0)
						device_role = DEVICE_ROLE_UNCONFIGURED;
					else if(os_strcmp(token, "1") == 0)
						device_role = DEVICE_ROLE_CONTROLLER;
					else if(os_strcmp(token, "2") == 0)
						device_role = DEVICE_ROLE_AGENT;
			}} else if (os_strcmp(token, "ChPlanningEnable") == 0) {
				token = os_strtok(NULL, "");
				dev->ch_planning.ch_planning_enabled = FALSE;
				if (token && token[0] == '1') {
					dev->ch_planning.ch_planning_enabled = TRUE;
					err("Channel Planning Enabled =%s\n", token);
				}
#ifdef MAP_R2
			} else if (os_strcmp(token, "ChPlanningEnableR2") == 0) {
				token = os_strtok(NULL, "");
				if (token && token[0] == '0') {
					dev->ch_planning_R2.ch_plan_enable = FALSE;
				} else {
				/* by default this feature is ON if MAP R2 define is ON, later is map_ver is R1 then make it false again*/
					dev->ch_planning_R2.ch_plan_enable = TRUE;
				}
				err("Channel Planning R2 Enabled =%d , R1 ch_planning_enabled %d\n", dev->ch_planning_R2.ch_plan_enable, dev->ch_planning.ch_planning_enabled);
			} else if (os_strcmp(token, "ChPlanningEnableR2withBW") == 0) {
				token = os_strtok(NULL, "");
				if (token && token[0] == '0') {
					dev->ch_planning_R2.ch_plan_enable_bw = FALSE;
				} else {
				/* by default this feature is ON if MAP R2 define is ON*/
					dev->ch_planning_R2.ch_plan_enable_bw = TRUE;
				}
				err("Channel Planning R2 Enabled_bw =%d\n", dev->ch_planning_R2.ch_plan_enable_bw);
#endif
			} else if (os_strcmp(token, "ChPlanningIdleByteCount") == 0) {
				token = os_strtok(NULL, "");
				dev->ch_planning.ChPlanningIdleByteCount = 65536;
				if (token) {
					dev->ch_planning.ChPlanningIdleByteCount = atol(token);
					err("ChPlanningIdleByteCount = %lu\n", dev->ch_planning.ChPlanningIdleByteCount);
				}
			} else if (os_strcmp(token, "ChPlanningIdleTime") == 0) {
				token = os_strtok(NULL, "");
				dev->ch_planning.ChPlanningIdleTime = 30*60;
				if (token) {
					dev->ch_planning.ChPlanningIdleTime = atol(token);
					err("ChPlanningIdleTime = %lu\n", dev->ch_planning.ChPlanningIdleTime);
				}
			} else if (os_strcmp(token, "BhPriority2G") == 0) {
				token = os_strtok(NULL, "");
				if (token && token[0] == '1')
					bh_priority_2g = 1;
				else if (token && token[0] == '2')
					bh_priority_2g = 2;
				else if (token && token[0] == '3')
					bh_priority_2g = 3;
				else if (token && token[0] == '0')
					bh_priority_2g = 0;
			} else if (os_strcmp(token, "BhPriority5GL") == 0) {
				token = os_strtok(NULL, "");
				if (token && token[0] == '1')
					bh_priority_5gl = 1;
				else if (token && token[0] == '2')
					bh_priority_5gl = 2;
				else if (token && token[0] == '3')
					bh_priority_5gl = 3;
				else if (token && token[0] == '0')
					bh_priority_5gl = 0;
			} else if (os_strcmp(token, "BhPriority5GH") == 0) {
				token = os_strtok(NULL, "");
				if (token && token[0] == '1')
					bh_priority_5gh = 1;
				else if (token && token[0] == '2')
					bh_priority_5gh = 2;
				else if (token && token[0] == '3')
					bh_priority_5gh = 3;
				else if (token && token[0] == '0')
					bh_priority_5gh = 0;
			} else if (os_strcmp(token, "ChPlanningUserPreferredChannel5G") == 0) {
				int it = 0;
				token = os_strtok(NULL, "");
				if (token) {
					user_prefered_channel = atoi(token);
					if (user_prefered_channel > 14)
						ch_planning_set_user_preff_ch(dev->back_ptr,
						user_prefered_channel);
				} else {
					for (it = 0; it < MAX_NUM_OF_RADIO; it++) {
						if (dev->dev_radio_info[it].channel > 14) {
							user_prefered_channel = 0;
							ch_planning_set_user_preff_ch(dev->back_ptr,
								user_prefered_channel);
							break;
						}
					}
				}
			}else if (os_strcmp(token, "ChPlanningUserPreferredChannel5GH") == 0) {
				int it = 0;
				token = strtok(NULL, "");
				if (token) {
					user_prefered_channel = atoi(token);
					if (isChan5GH(user_prefered_channel)) {
						ch_planning_set_user_preff_ch(dev->back_ptr,
						user_prefered_channel);
						}
				} else {
					for (it = 0; it < MAX_NUM_OF_RADIO; it++) {
						if (isChan5GH(dev->dev_radio_info[it].channel)) {
							user_prefered_channel = 0;
							ch_planning_set_user_preff_ch(dev->back_ptr,
								user_prefered_channel);
							break;
						}
					}
				}
			} else if (os_strcmp(token, "ChPlanningUserPreferredChannel2G") == 0) {
				int it = 0;
				token = os_strtok(NULL, "");
				if (token) {
					user_prefered_channel = atoi(token);
					if (user_prefered_channel <= 14)
						ch_planning_set_user_preff_ch(dev->back_ptr,
							user_prefered_channel);
				} else {
					for (it = 0; it < MAX_NUM_OF_RADIO; it++) {
						if (dev->dev_radio_info[it].channel <= 14) {
							user_prefered_channel = 0;
							ch_planning_set_user_preff_ch(dev->back_ptr,
								user_prefered_channel);
							break;
						}
					}
				}
			}
			else if (os_strcmp(token, "SteerEnable") == 0){
				token = os_strtok(NULL, "");
				dev->SetSteer=STEER_ENABLE;
				if(token){
					if (token[0] == '0')
						dev->SetSteer=STEER_DISABLE;
				}
			} else if (os_strcmp(token, "ScanThreshold2g") == 0) {
				token = os_strtok(NULL, "");
				if(token){
					dev->rssi_threshold_2g = atoi(token);
					always("2G ScanThreshold=%d", dev->rssi_threshold_2g);
				}
			} else if (os_strcmp(token, "ScanThreshold5g") == 0) {
				token = os_strtok(NULL, "");
				if(token){
					dev->rssi_threshold_5g = atoi(token);
					always("5G ScanThreshold=%d", dev->rssi_threshold_5g);
				}
			} else if (os_strcmp(token, "ChPlanningInitTimeout") == 0) {
				token = os_strtok(NULL, "");
				if(token){
					dev->channel_planning_initial_timeout = atoi(token);
					always("ChPlanningInitTimeout=%d", atoi(token));
			} }else if (os_strcmp(token, "DhcpCtl") == 0) {
				/*DHCP_CTL*/
				token = os_strtok(NULL, "");
				always("DhcpCtl=%s\n", token);
				if (os_strcmp(token, "1") == 0) {
					mapd_printf(MSG_OFF,"enable");
					dev->dhcp_ctl_enable = 1;
				} else {
					mapd_printf(MSG_OFF,"disable");
					dev->dhcp_ctl_enable = 0;
				}
				mapd_printf(MSG_OFF," dhcp_ctl in config file is : %s !\n",
					dev->dhcp_ctl_enable == 1 ? "enable":"disable");
			} else if (os_strcmp(token, "NetworkOptimizationEnabled") == 0) {
				token = os_strtok(NULL, "");
				if(token) {
					dev->network_optimization.network_optimization_enabled= atoi(token);
					always("NetworkOptimizationEnabled=%d", atoi(token));
				}
			} else if (os_strcmp(token, "NetworkOptPrefer5Gover2G") == 0) {
				token = os_strtok(NULL, "");
				if(token) {
					dev->network_optimization.prefer_5G_bh= atoi(token);
					always("NetworkOptPrefer5Gover2G=%d", atoi(token));
				}
			} else if (os_strcmp(token, "NetworkOptPrefer5Gover2GRetryCnt") == 0) {
				token = os_strtok(NULL, "");
				if(token) {
					dev->network_optimization.prefer_5G_bh_try_cnt_user = atoi(token);
					always("NetworkOptPrefer5Gover2GRetryCnt=%d", atoi(token));
				}
			}
#if 0
			else if (os_strcmp(token, "NtwrkOptBootupWaitTime") == 0){
				token = os_strtok(NULL, "");
				if(token) {
					always("NtwrkOptBootupWaitTime=%d", atoi(token));
					dev->network_optimization.bootup_wait_time = atoi(token);
				}
			}
			else if (os_strcmp(token, "NetworkOptimizationScoreMargin") == 0){
				token = os_strtok(NULL, "");
				if(token) {
					dev->ntwrk_opt.network_optimization.ntwrk_opt_score_margin = atoi(token);
					always("NetworkOptimizationScoreMargin=%d", atoi(token));
				}
			}
#endif
			else if (os_strcmp(token, "NtwrkOptConnectWaitTime") == 0){
				token = os_strtok(NULL, "");
				if(token) {
					dev->network_optimization.connect_wait_time = atoi(token);
					always("NtwrkOptConnectWaitTime=%d", atoi(token));
				}
			}else if (os_strcmp(token, "NtwrkOptDisconnectWaitTime") == 0){
				token = os_strtok(NULL, "");
				if(token) {
					dev->network_optimization.disconnect_wait_time = atoi(token);
					always("NtwrkOptDisconnectWaitTime=%d", atoi(token));
				}
			}
			else if (os_strcmp(token, "NtwrkOptPostCACTriggerTime") == 0){
				token = os_strtok(NULL, "");
				if(token) {
					dev->network_optimization.post_cac_trigger_time= atoi(token);
					always("NtwrkOptPostCACTriggerTime=%d", atoi(token));
				}
			}
			else if (os_strcmp(token, "NtwrkOptDataCollectionTime") == 0) {
				token = os_strtok(NULL, "");
				if(token) {
					dev->network_optimization.data_collection_wait_time = atoi(token);
					always("NtwrkOptDataCollectionTime=%d", atoi(token));
				}
			}

#if 0
			else if (os_strcmp(token, "NtwrkOptPeriodicity") == 0){
				token = os_strtok(NULL, "");
				if(token) {
					dev->network_optimization_periodicity = atoi(token);
					always("network_optimization_periodicity=%d", atoi(token));
				}
			}
#endif
			else if (os_strcmp(token, "AutoBHSwitching") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					dev->auto_bh_switch = atol(token);
					err("AutoBHSwitching = %d\n", dev->auto_bh_switch);
				}
			}	else if (os_strcmp(token, "DualBH") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					dev->dual_bh_en = atol(token);
					err("DualBH = %d\n", dev->dual_bh_en);
				}
			}	else if (os_strcmp(token, "DynamicLoadBalance") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					dev->load_balance_en = atol(token);
					err("DynamicLoadBalance = %d\n", dev->load_balance_en);
				}
			}	else if (os_strcmp(token, "BandSwitchTime") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					dev->band_switch_time = atol(token);
					err("BandSwitchTime = %d\n", dev->band_switch_time);
				} else {
					dev->band_switch_time = 120;
					err("BandSwitchTime = %d\n", dev->band_switch_time);
				}
			} else if (os_strcmp(token, "ThirdPartyConnection") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					dev->ThirdPartyConnection= atol(token);
					err("ThirdPartyConnection = %d\n", dev->ThirdPartyConnection);
				} else {
					dev->ThirdPartyConnection = 0;
					err("default ThirdPartyConnection = %d\n", dev->ThirdPartyConnection);
				}
			}else if (os_strcmp(token, "EnhancedLogging") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					dev->enhanced_logging= atol(token);
					err("ThirdPartyConnection = %d\n", dev->enhanced_logging);
				} else {
					dev->enhanced_logging = 0;
					err("default ThirdPartyConnection = %d\n", dev->enhanced_logging);
				}
			}else if (os_strcmp(token, "AvoidScanDuringCac") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					avoid_scan_cac = atol(token);
					err("AvoidScanDuringCac = %d",avoid_scan_cac);
				} else {
					err("not in file AvoidScanDuringCac = 0\n");
				}
				err("send command to wapp");
				wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_USER_SET_AVOID_SCAN_CAC,
						0, NULL, NULL,(void *)&avoid_scan_cac, sizeof(char), 0, 0, 0);
			}else if (os_strcmp(token, "BHSwitchbyCUEnable") == 0) {
				token = strtok(NULL, "");
				dev->bh_cu_params.bh_switch_cu_en = FALSE;
				if (token && token[0] == '1') {
					dev->bh_cu_params.bh_switch_cu_en = TRUE;
					err("band switch by cu Enabled =%s\n", token);
				}
			} else if (os_strcmp(token, "BHOLCountThreshold") == 0) {
				token = strtok(NULL, "");
				if (token) {
					dev->bh_cu_params.BHOLSteerCountTh = atol(token);
					err("BHOLSteerCountTh = %d\n", dev->bh_cu_params.BHOLSteerCountTh);
				} else {
					dev->bh_cu_params.BHOLSteerCountTh = MAX_BH_OL_STEER_COUNT;
					err("BHOLSteerCountTh = %d\n", dev->bh_cu_params.BHOLSteerCountTh);
				}
			}else if (os_strcmp(token, "BHOLForbidTime") == 0) {
				token = strtok(NULL, "");
				if (token) {
					dev->bh_cu_params.BHOLForbidTime = atol(token);
					err("BHOLForbidTime = %d\n", dev->bh_cu_params.BHOLForbidTime);
				} else {
					dev->bh_cu_params.BHOLForbidTime = MAX_BH_OL_FORBID_TIME;
					err("BHOLForbidTime = %d\n", dev->bh_cu_params.BHOLForbidTime);
				}
			}else if (os_strcmp(token, "MaxAllowedScan") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					dev->max_allowed_scan = atol(token);
					err("max_allowed_scan = %d",dev->max_allowed_scan);
				} else {
					dev->max_allowed_scan = MAX_ALLOWED_SEC_LINK_SCAN;
					err("Default max_allowed_scan = %d",dev->max_allowed_scan);
				}
			}else if (os_strcmp(token, "BHSteerTimeout") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					dev->bh_steer_timeout = atol(token);
					if (dev->bh_steer_timeout == 0)
						dev->bh_steer_timeout = DEFAULT_BH_STEER_TIMEOUT;
					err("bh_steer_timeout = %d",dev->bh_steer_timeout);
				} else {
					dev->bh_steer_timeout = DEFAULT_BH_STEER_TIMEOUT;
					err("Default bh_steer_timeout = %d",dev->bh_steer_timeout);
				}
			}else if (os_strcmp(token, "NonMAPAPEnable") == 0){
				token = strtok(NULL, "");
				dev->non_map_ap_enable=NON_MAP_ENABLE;
				if(token){
					if (token[0] == '0')
						dev->non_map_ap_enable=NON_MAP_DISABLE;
				}
			} else if(os_strcmp((const char *)token, "CentralizedSteering")==0) {
					token = strtok(NULL, "");
					dev->cent_str_en = atoi((const char *)token);
					mapd_printf(MSG_INFO, "CentralizedSteering=%d",
						dev->cent_str_en);
			}
#ifdef MAP_R2
			else if(os_strcmp((const char *)token, "DivergentChPlanning")==0) {
					token = strtok(NULL, "");
					dev->div_ch_planning = atoi((const char *)token);
					mapd_printf(MSG_INFO, "DivergentChPlanning=%d",
						dev->div_ch_planning);
					if (dev->div_ch_planning == 1) {
						dev->ch_planning_R2.ch_plan_enable = FALSE;
						dev->ch_planning.ch_planning_enabled = FALSE;
					}
			}
#endif
		}
	}
#ifdef MAP_R2
	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);

	err("going to check 1905 map version");
	struct _1905_map_device *tmp_dev;
	tmp_dev = topo_srv_get_1905_device(dev, NULL);
	while (mapd_config_get_line(buf, sizeof(buf), file_1905, &line, &pos)) {
		os_snprintf(tmpbuf, sizeof(tmpbuf), "%s", pos);
		token = os_strtok(pos, "=");

		if (token != NULL) {
			if (os_strcmp(token, "map_ver") == 0) {
				token = os_strtok(NULL, "");
				if (token) {
					err("for 1905 cfg ");
					if (os_strcmp(token, "R1") == 0) {
						err("it is R1");
						dev->map_version = DEV_TYPE_R1;
						if(tmp_dev) {
							err("1905dev own is R1");
							tmp_dev->map_version = DEV_TYPE_R1;
						}
					}else {
						err("it is R2");
						dev->map_version = DEV_TYPE_R2;
						if(tmp_dev){
							err("1905dev own is R2");
							tmp_dev->map_version = DEV_TYPE_R2;
						}
					}
				} else {
					dev->map_version = DEV_TYPE_R1;
					err("Default map version = %d", dev->map_version);
					if(tmp_dev){
						err("1905dev own is R2");
						tmp_dev->map_version = DEV_TYPE_R1;
					}
				}
			}
		}
	}
	if(dev->map_version == DEV_TYPE_R1) {
		err("since ver is MAP R1, so disable R2 turnkey");
		dev->ch_planning_R2.ch_plan_enable = FALSE;
		dev->ch_planning_R2.ch_plan_enable_bw = FALSE;
	}
#endif
	err("Network Optimization Enable %d",dev->network_optimization.network_optimization_enabled);

	err("conn %d dis %d",dev->network_optimization.connect_wait_time,dev->network_optimization.disconnect_wait_time);

	os_get_time(&dev->network_optimization.ntwrk_change_ts);
	err("Init %ld ", dev->network_optimization.ntwrk_change_ts.sec);


	if (device_role == DEVICE_ROLE_INVALID)
		device_role = DEVICE_ROLE_AGENT;

	dev->device_role = device_role;
	if (device_role == DEVICE_ROLE_UNCONFIGURED) {
		topo_srv_update_device_role(dev, NULL, device_role);
	}
#ifdef MAP_R2
	wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_UPDATE_MAP_DEVICE_ROLE,
		0, NULL, NULL,(void *)&device_role, sizeof(int), 0, 0, 0);
#else
	if(dev->ThirdPartyConnection){
		wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_UPDATE_MAP_DEVICE_ROLE,
			0, NULL, NULL,(void *)&device_role, sizeof(int), 0, 0, 0);
	}

#endif
	if (!SLIST_EMPTY(&dev->bh_link_head)) {
		SLIST_FOREACH(bh, &dev->bh_link_head, next_bh_link)
		{
			if (bh->bh_channel >= 100)
			{
				bh->priority_info.priority = bh_priority_5gh == 255?1:bh_priority_5gh;
				err("%s BH priority = %d\n",
					bh->ifname, bh->priority_info.priority);
			} else if (bh->bh_channel > 14)
			{
				bh->priority_info.priority = bh_priority_5gl == 255? 1:bh_priority_5gl;
				err("%s BH priority = %d\n",
					bh->ifname, bh->priority_info.priority);
			} else {
				bh->priority_info.priority = bh_priority_2g == 255? 1:bh_priority_2g;
				err("%s BH priority = %d\n",
					bh->ifname, bh->priority_info.priority);
			}
			struct bh_priority bh_priority_msg;
			bh_priority_msg.priority = bh->priority_info.priority;
			os_memcpy(bh_priority_msg.bh_mac, bh->mac_addr, ETH_ALEN);
			wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_USER_SET_BH_PRIORITY,
				0, NULL, NULL, &bh_priority_msg, sizeof(struct bh_priority), 0, 0, 0);
		}
	}
	fclose(file);
	return 0;
}

Boolean is_chan_supported(u8 *known_channels, u8 channel)
{
	uint8_t ch_grp[MAX_CHANNEL_BLOCKS] = {0};
	uint8_t idx;
	uint8_t arr_idx;
	uint8_t i = 0;
	ch_grp[0] = channel;
	if (channel > 14) {
		mapd_fill_secondary_channels(ch_grp, 0, 2);
	}

	for (i = 0; (i < MAX_CHANNEL_BLOCKS) && (ch_grp[i] != 0);
		i++) {
		idx = chan_to_idx(ch_grp[i]);
		arr_idx = idx / 8;
		if (known_channels[arr_idx] & BIT(idx % 8)) {
			return TRUE;
		}
	}
	return FALSE;
}
#else
Boolean is_chan_supported(u8 *known_channels, u8 channel)
{
        uint8_t idx;
        uint8_t arr_idx;

        if (channel) {
        idx = chan_to_idx(channel);
                arr_idx = idx / 8;
                if (known_channels[arr_idx] & BIT(idx % 8)) {
                        return TRUE;
                }
        }
        return FALSE;
}
#endif /* #ifdef SUPPORT_MULTI_AP */
