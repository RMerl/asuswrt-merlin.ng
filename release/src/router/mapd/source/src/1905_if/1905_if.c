/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  1905 interface
 *
 *  Abstract:
 *  1905 interface
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02     First implementation of the 1905 interface Module
 * */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>
#include "mapd_debug.h"

#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "client_db.h"
#include "mapd_i.h"
#include "topologySrv.h"
#include "eloop.h"
#include "wapp_if.h"
#include "tlv_parsor.h"
#include "apSelection.h"
#include "1905_map_interface.h"
#include "./../1905_local_lib/data_def.h"
#include "chan_mon.h"
#include "client_mon.h"
#include "ch_planning.h"
#include "network_optimization.h"
#include "mapd_user_iface.h"
#include "ctrl_iface.h"
#include "mapfilter_if.h"




unsigned char BROADCOM_OUI[]    = {0x00, 0x90, 0x4c};
unsigned char MARVELL_OUI[]     = {0x00, 0x50, 0x43};
unsigned char ATHEROS_OUI[] 	= {0x00, 0x03, 0x7F};
unsigned char INTEL_OUI[]	= {0x00, 0x17, 0x35};
unsigned char channel_list[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,144,149,153,157,161,165,169};

#ifndef SUPPORT_1905

int _1905_open_connection(const char *ctrl_path, struct mapd_global *global)
{
	return 0;
}

int _1905_close_connection(struct mapd_global *global)
{
	return 0;
}

int _1905_poll_devices_in_network(struct own_1905_device *own_dev, struct bh_link_entry *bh_entry)
{
	return 0;
}

int _1905_update_channel_pref_report(struct own_1905_device *ctx
	,struct cac_completion_report * report
#ifdef DFS_CAC_R2
	,struct cac_status_report *status_report
#endif
	)
{
	return 0;
}
int _1905_if_send_ap_metric_rsp(struct own_1905_device *own_dev
#if defined(MAP_R2) || defined(CENT_STR)
, unsigned char periodic
#endif
)
{
	return 0;
}
#else
int IsValidChannel(unsigned char ch_num)
{
	unsigned char i;
	err("chan num %d size %zd \n",ch_num, sizeof(channel_list));
	for (i = 0; i < sizeof(channel_list); i++)
	{
		if (ch_num == channel_list[i])
			return TRUE;
	}
	return FALSE;
}
/**
* @brief Fn to poll other 1905 devices in network
*
* @param own_dev own 1905 device ctx
* @param bh_entry backhaul info
*
* @return -1 if error else 0
*/
int _1905_poll_devices_in_network(struct own_1905_device *own_dev, struct bh_link_entry *bh_entry)
{
	struct mapd_global *mapd_ctx = (struct mapd_global *)own_dev->back_ptr;
	unsigned int band = 0;
#ifdef AUTOROLE_NEGO
	struct bss_info_db *bss = NULL;
#endif

#ifdef AUTOROLE_NEGO
	if (bh_entry) {
		topo_srv_get_bss_by_bssid(own_dev, NULL, bh_entry->bssid);
		if (!bss) {
			err("bssid not found");
			return -1;
		}
		if (bss->radio)
			band = bss->radio->band;
		else {
			err("radio not found for the bss");
			return -1;
		}
	}

	map_1905_Send_AP_Autoconfig_Search_Message(mapd_ctx->_1905_ctrl, band);
#else
	map_1905_Send_AP_Autoconfig_Search_Message(mapd_ctx->_1905_ctrl, band);
#endif

	return 0;
}

struct _1905_map_device *topo_srv_create_1905_device(struct own_1905_device *ctx, unsigned char *almac);
/**
* @brief Generic handling of vendor specific messages. RFS/TSQ/STA_INFO
*
* @param pGlobal_dev pointer to global structure
* @param buf vendor specific message contents
* @param len length of the message.
*/
void _1905_handle_vendor_msg(struct mapd_global *pGlobal_dev, u8 *buf, size_t len)
{
	unsigned char *temp_buf = NULL;
	u8 al_mac_addr[ETH_ALEN];
	struct _1905_map_device *_1905_device = NULL;
	enum map_vendor vendor;

	struct tlv_head *tlv= NULL;

	temp_buf = buf;

	if(pGlobal_dev->params.Certification) {
		return;
	}
	os_memcpy(al_mac_addr, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	_1905_device = topo_srv_get_1905_device(&pGlobal_dev->dev, al_mac_addr);
	if(_1905_device == NULL) {
		_1905_device = topo_srv_create_1905_device(&pGlobal_dev->dev, al_mac_addr);
		topo_srv_send_vendor_oui_tlv(&pGlobal_dev->dev, al_mac_addr);
		topo_srv_send_vendor_chan_report_msg(&pGlobal_dev->dev, al_mac_addr);
	}

	tlv = (struct tlv_head *)temp_buf;

	// currently only 1 tlv to be present in one vendor specific message. No support for handling multiple tlvs in the same message.


	if(tlv->tlv_type != TLV_802_11_VENDOR_SPECIFIC) {
		err("wrong tlv type: %d", tlv->tlv_type);
		return;
	}

	if(!os_memcmp(tlv->oui, MTK_OUI, OUI_LEN))
		vendor = VENDOR_MEDIATEK;
	else if(!os_memcmp(tlv->oui, BROADCOM_OUI, OUI_LEN))
		vendor = VENDOR_BROADCOM;
	else if(!os_memcmp(tlv->oui, ATHEROS_OUI, OUI_LEN))
		vendor = VENDOR_QUALCOMM;
	else if(!os_memcmp(tlv->oui, MARVELL_OUI, OUI_LEN))
		vendor = VENDOR_MARVELL;
	else if(!os_memcmp(tlv->oui, INTEL_OUI, OUI_LEN))
		vendor = VENDOR_INTEL;
	else
		vendor = VENDOR_UNKNOWN;

	if ((tlv->func_type < FUNC_VENDOR_OUI) && (vendor == VENDOR_MEDIATEK))
		return steer_msg_handle_vendor_specific_msg(pGlobal_dev, _1905_device, tlv);

	switch(tlv->func_type)
	{
		case FUNC_VENDOR_OUI:
			topo_srv_update_1905_dev_vendor(&pGlobal_dev->dev, _1905_device, vendor);
			break;
		case FUNC_VENDOR_CHAN_REPORT:
			mapd_hexdump(MSG_DEBUG, "_1905_handle_vendor_msg", (char *)(temp_buf), len);
			temp_buf+=sizeof(struct tlv_head);
			break;
		case FUNC_VENDOR_TRIGER_WPS:
			/*
			field		length		value
			sub-type	1 octet		9
			sub-length	1 octet		6
			sub-value	6 octets	BSS mac
			*/
			{
				int ret = 0;
				unsigned char ret_buf[32] = {0};
				char cmd_buf[32] = {0};

				if (len != (sizeof(struct tlv_head) + 17)) {
					mapd_printf(MSG_ERROR, "FUNC_VENDOR_TRIGER_WPS length error, len=%d", (int)len);
					break;
				}

				temp_buf+=sizeof(struct tlv_head) + 1;
				os_snprintf(cmd_buf, sizeof(cmd_buf), "%02x:%02x:%02x:%02x:%02x:%02x PBC",
					PRINT_MAC(temp_buf));

				ret = mapd_trigger_wps(pGlobal_dev, cmd_buf, (char *)ret_buf, sizeof(ret_buf));
				if (ret != 2 || os_strncmp((const char *)ret_buf, "OK", 2) != 0) {
					mapd_printf(MSG_ERROR, "trigger wps fail");
				}
			}
			break;
				case FUNC_VENDOR_SET_TX_POWER_PERCENTAGE:
			{
				info("Call handler to set Tx Power percentage");
				ch_planning_handle_tx_power_percentage_msg(pGlobal_dev,(struct tx_power_percentage_tlv *) tlv);
			}
			break;
		case FUNC_VENDOR_OFF_CH_SCAN_REQ:
		case FUNC_VENDOR_NET_OPT_SCAN_REQ:
			{
				err("FUNC_VENDOR_OFF_CH_SCAN_REQ, handle_off_ch_scan_req \n");
				handle_off_ch_scan_req(pGlobal_dev,(unsigned char *) tlv,
					_1905_device, tlv->func_type);
			}
			break;
		case FUNC_VENDOR_OFF_CH_SCAN_RESP:
			{
				err("FUNC_VENDOR_OFF_CH_SCAN_RESP, handle_off_ch_scan_resp\n");
				handle_off_ch_scan_resp(pGlobal_dev,(struct off_ch_scan_resp_tlv *) tlv,
						_1905_device);
			}
			break;
		case FUNC_VENDOR_NET_OPT_SCAN_RESP:
			{
				err("FUNC_VENDOR_OFF_CH_SCAN_RESP, handle_off_ch_scan_resp\n");
				handle_net_opt_scan_resp(pGlobal_dev,(struct net_opt_scan_resp_tlv *) tlv,
						_1905_device);
			}
			break;
		case FUNC_CAC_START:
			{
				err("FUNC_CAC_START, handle cac start on one agents\n");
				handle_cac_start_from_agent(pGlobal_dev, (struct cac_start_tlv *) tlv, _1905_device);
			}
			break;
		case FUNC_BH_PRIORITY_INFO:
			{
				err("Received BH priority info");
				handle_bh_priority_info_from_agent(pGlobal_dev, (struct bh_priority_msg *)tlv, _1905_device);
			}
			break;
#ifdef ACL_CTRL
		case FUNC_VENDOR_ACL_CTRL:
			{
				err("rx acl ctrl msg from almac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(al_mac_addr));
				handle_acl_ctrl_msg(pGlobal_dev, (struct acl_ctrl_tlv *) tlv, _1905_device);
			}
			break;
#endif
		default:
			return;
	}
}

/**
* @brief Fn to update channel report to 1905 daemon
*
* @param ctx own 1905 device ctx
*
* @return -1 if error else 0
*/
int _1905_update_channel_pref_report(struct own_1905_device *ctx
	,struct cac_completion_report * report
#ifdef DFS_CAC_R2
	,struct cac_status_report *status_report
#endif
	)
{
	int ch_prefer_cnt = 0;
	struct ch_prefer_lib *ch_prefers = NULL;
	int restriction_cnt = 0;
	struct restriction_lib *restrictions = NULL;
	struct mapd_global *global;
	global = (struct mapd_global *)(ctx->back_ptr);

	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct radio_info_db *radio = topo_srv_get_next_radio(topo_srv_get_next_1905_device(ctx, NULL), NULL);
	struct prefer_info_db *prefer_db = NULL;
	struct restrict_db *res_db = NULL;
	struct prefer_info *pinfo = NULL;
	struct restrict_info *rinfo = NULL;
	int i = 0;
	unsigned char *temp_buf = NULL;
	unsigned char *temp_buf2 = NULL;

	temp_buf = os_zalloc(2*2048);
	if (temp_buf == NULL) {
		err("Allocation failed\n");
		return -1;
	}

	temp_buf2 = os_zalloc(2*2048);
	if (temp_buf2 == NULL) {
		err("Allocation failed\n");
		if (temp_buf)
			os_free(temp_buf);
		return -1;
	}

	restrictions = (struct restriction_lib *)&temp_buf2[0];
	ch_prefers = (struct ch_prefer_lib *)&temp_buf[0];
	while (radio) {
		ch_prefer_cnt++;
		restriction_cnt++;
		i = 0;
		memcpy(ch_prefers->identifier, radio->identifier, ETH_ALEN);
		ch_prefers->op_class_num = radio->chan_preferance.op_class_num;
		SLIST_FOREACH(prefer_db,
			&radio->chan_preferance.prefer_info_head, prefer_info_entry) {
			int channel_count = 0;
			int j;
			pinfo = &ch_prefers->opinfo[i];
			pinfo->op_class = prefer_db->op_class;
			for(j = 0; j < prefer_db->ch_num; j++)
			{
				pinfo->ch_list[channel_count] = prefer_db->ch_list[j];
				channel_count++;
			}
			pinfo->ch_num = channel_count;
			pinfo->perference = prefer_db->perference;
			pinfo->reason = prefer_db->reason;
			i++;
		}
		ch_prefers++;
		memcpy(restrictions->identifier, radio->identifier, ETH_ALEN);
		restrictions->op_class_num = radio->chan_restrict.op_class_num;
		i = 0;
		SLIST_FOREACH(res_db, &radio->chan_restrict.restrict_head, restrict_entry) {
			rinfo = &restrictions->opinfo[i];
			rinfo->op_class = res_db->op_class;
			rinfo->ch_num = res_db->ch_num;
			memcpy(rinfo->ch_list, res_db->ch_list, MAX_CH_NUM);
			memcpy(rinfo->fre_separation, res_db->min_fre_sep, MAX_CH_NUM);
			i++;
		}
		restrictions++;
		radio = topo_srv_get_next_radio(topo_srv_get_next_1905_device(ctx, NULL), radio);
	}
	err(" ");
	map_1905_Set_Channel_Preference_Report_Info(_1905_ctrl, ch_prefer_cnt,
		(struct ch_prefer_lib *)temp_buf, restriction_cnt,
		(struct restriction_lib *)temp_buf2
		,report
#ifdef DFS_CAC_R2
		, status_report
#endif
		); //Prakhar
	err(" ");
	os_free(temp_buf2);
	os_free(temp_buf);
	return 0;
}

#ifdef MAP_R2

/**
* @brief Fn to send ap metrics response to 1905 daemon
*
* @param own_dev 1905 own device ctx
*
* @return -1 if error else 0
*/
int _1905_if_send_sta_metric_rsp(struct own_1905_device *own_dev)
{
	struct link_metrics *sta_metrics = NULL;
	unsigned char sta_metrics_cnt = 0;
	struct mapd_global *mapd_ctx = (struct mapd_global *)own_dev->back_ptr;
	struct sta_extended_metrics_lib *ext_sta_metrics = NULL;
	unsigned char ext_sta_metric_cnt = 0;

	topo_srv_sta_metrics_rsp_message(own_dev, &sta_metrics, &sta_metrics_cnt
					, &ext_sta_metrics, &ext_sta_metric_cnt);

	map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(mapd_ctx->_1905_ctrl, sta_metrics_cnt, sta_metrics,
					ext_sta_metric_cnt, ext_sta_metrics, NULL, 0);
	if (sta_metrics)
		os_free(sta_metrics);
	if(ext_sta_metrics)
		os_free(ext_sta_metrics);

	return 0;
}

#endif

/**
* @brief Fn to send ap metrics response to 1905 daemon
*
* @param own_dev 1905 own device ctx
*
* @return -1 if error else 0
*/
int _1905_if_send_ap_metric_rsp(struct own_1905_device *own_dev
#if defined(MAP_R2) || defined(CENT_STR)
, unsigned char periodic
#endif
)
{
	struct ap_metrics_info_lib *ap_metrics = NULL;
	int ap_metrics_info_cnt = 0;
	struct stat_info *sta_states = NULL;
	int sta_states_cnt = 0;
	struct link_metrics *sta_metrics = NULL;
	int sta_metrics_cnt = 0;
	struct mapd_global *mapd_ctx = (struct mapd_global *)own_dev->back_ptr;
#ifdef MAP_R2
	struct radio_metrics_lib *radio_metrics = NULL;
	int radio_metrics_info_cnt = 0;
	struct ap_extended_metrics_lib *ext_ap_metrics = NULL;
	int ext_ap_metric_cnt = 0;
	struct sta_extended_metrics_lib *ext_sta_metrics = NULL;
	int ext_sta_metric_cnt = 0;
	struct ch_util_lib *ch_util = NULL;
	int ch_util_cnt = 0;
#endif

	topo_srv_ap_metrics_rsp_message(own_dev, &ap_metrics, &ap_metrics_info_cnt, &sta_states,
					&sta_states_cnt, &sta_metrics, &sta_metrics_cnt
#ifdef MAP_R2
					, &ext_ap_metrics, &ext_ap_metric_cnt, &ext_sta_metrics, &ext_sta_metric_cnt, &radio_metrics, &radio_metrics_info_cnt
					, &ch_util , &ch_util_cnt, periodic
#endif
			);

#ifdef MAP_R2
#if 0
	if(!periodic) {
		ext_ap_metric_cnt = 0; /*Extended AP metric TLV not to be included in periodic AP metric rsp.*/
		if (ext_ap_metrics) {
			os_free(ext_ap_metrics);
			ext_ap_metrics = NULL;
		}
	}
#endif
	u8 vs_len = sizeof(struct tlv_head) + ch_util_cnt * sizeof(struct ch_util_lib);
	if (own_dev->map_version == DEV_TYPE_R2) {
		map_1905_Set_Ap_Metric_Rsp_Info(mapd_ctx->_1905_ctrl, ap_metrics, ap_metrics_info_cnt,
						sta_states, sta_states_cnt, sta_metrics, sta_metrics_cnt, ext_ap_metrics, ext_ap_metric_cnt,
						ext_sta_metrics, ext_sta_metric_cnt, radio_metrics, radio_metrics_info_cnt,
						ch_util, vs_len);
	} else {
		map_1905_Set_Ap_Metric_Rsp_Info(mapd_ctx->_1905_ctrl, ap_metrics, ap_metrics_info_cnt,
						sta_states, sta_states_cnt, sta_metrics, sta_metrics_cnt, NULL, 0,
						NULL, 0, NULL, 0,
						NULL, 0);
	}
#else
	map_1905_Set_Ap_Metric_Rsp_Info(mapd_ctx->_1905_ctrl, ap_metrics, ap_metrics_info_cnt,
					sta_states, sta_states_cnt, sta_metrics, sta_metrics_cnt);
#endif

	if (sta_states)
		free(sta_states);
	if (ap_metrics)
		free(ap_metrics);
	if (sta_metrics)
		free(sta_metrics);
#ifdef MAP_R2
	if(radio_metrics)
		os_free(radio_metrics);
	if(ext_ap_metrics)
		os_free(ext_ap_metrics);
	if(ext_sta_metrics)
		os_free(ext_sta_metrics);
	if(ch_util)
		os_free(ch_util);
#endif
	return 0;
}

/**
* @brief Receive data from 1905 daemon
*
* @param ctrl 1905 control interface
* @param reply data buffer
* @param reply_len data length
*
* @return 0 if success else error code
*/
int _1905_interface_ctrl_recv(struct _1905_context *ctrl, unsigned char *reply, size_t * reply_len)
{
	int res;
	int sock = ctrl->s;

	res = recv(sock, reply, *reply_len, 0);
	if (res < 0)
		return res;
	*reply_len = res;
	return 0;
}

int _1905_interface_parse_event(struct mapd_global *mapd_ctx, unsigned char *buff, int len);

/**
* @brief Fn to receive pending data from 1905 daemon
*
* @param ctrl 1905 control interface
* @param global mapd global context
*/
#ifdef MAP_R2
#define P1905_EVT_SIZE 15360 // 15KB
#else
#define P1905_EVT_SIZE 3072 // 3KB
#endif

static void mapd_recv_pending_from_1905(struct _1905_context *ctrl, struct mapd_global *global)
{
	/* No need to wait..I already know there is something to be received */
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if (ctrl == NULL) {
		err("WHAT??");
		return;
	}
	while (map_1905_interface_ctrl_pending(ctrl, &tv) > 0) {
		char *buf = malloc(P1905_EVT_SIZE); //as 1905 is sending
		if (!buf) {
			err("failed to allocate memory");
			return;
		}
		size_t len = P1905_EVT_SIZE - 1;

		if (map_1905_Receive(ctrl, buf, &len) == 0) {
			debug("Rx event length=%zu", len);
			buf[len] = '\0';
			_1905_interface_parse_event(global, (unsigned char *)buf, len);
		} else {
			err("Could not read pending message.");
			free(buf);
			break;
		}
		free(buf);
	}

	if (map_1905_interface_ctrl_pending(ctrl, &tv) < 0) {
		err("Connection to WAPP lost");
	}
}

/**
* @brief Fn to receive data from 1905
*
* @param sock sock number
* @param eloop_ctx eloop ctx
* @param sock_ctx sock ctx
*/
static void mapd_receive_from_1905(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct _1905_context *ctrl = sock_ctx;
	struct mapd_global *global = eloop_ctx;
	mapd_recv_pending_from_1905(ctrl, global);
}

/**
* @brief Fn to open a 1905 connection and register an eloop sock for this
*
* @param ctrl_path ctrl path
* @param global mapd global ctx
*
* @return -1 if failed else 0
*/
int _1905_open_connection(const char *ctrl_path, struct mapd_global *global)
{
	/* For Syncronous commapd-response */
	global->_1905_ctrl = map_1905_Init(ctrl_path);	//Should do attach

	if (global->_1905_ctrl == NULL)
		return -1;

	eloop_register_read_sock(global->_1905_ctrl->s, mapd_receive_from_1905, global, global->_1905_ctrl);

	return 0;
}

/**
* @brief Fn to close socket with 1905
*
* @param global mapd global
*
* @return 0
*/
int _1905_close_connection(struct mapd_global *global)
{
	if (global->_1905_ctrl ) {
		err("free sock and deinit _1905_ctrl");
		eloop_unregister_read_sock(global->_1905_ctrl->s);
		map_1905_Deinit(global->_1905_ctrl);
		global->_1905_ctrl = NULL;
	} else
		err("_1905_ctrl is null, do nothing");

	return 0;
}

/**
* @brief Fn to handle backhaul steer
*
* @param ctx 1905 map device ctx
* @param buf msg buffer
* @param len msg len
*
* @return -1 if error else 0
*/
void send_bh_steering_req_to_wapp(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *) eloop_ctx;
	struct backhaul_steer_request_extended *bh = NULL;
	struct _1905_map_device *target_dev = NULL;
	struct bss_info_db *target_bss = NULL;

	bh=os_zalloc(sizeof (struct backhaul_steer_request_extended));
	if(bh==NULL) {
		err("Malloc Failed")
			return;
	}
	os_memcpy(&bh->request,&ctx->bsteer_req,sizeof(ctx->bsteer_req) );
	err("backhaul_mac(%02x:%02x:%02x:%02x:%02x:%02x)"
			"target_bssid(%02x:%02x:%02x:%02x:%02x:%02x)"
			"operating class=%d, channel=%d",
			PRINT_MAC(bh->request.backhaul_mac),
			PRINT_MAC(bh->request.target_bssid),
			bh->request.oper_class, bh->request.channel);
	target_dev = topo_srv_get_1905_by_bssid(ctx, bh->request.target_bssid);
	target_bss = topo_srv_get_bss_by_bssid(ctx, target_dev, bh->request.target_bssid);
	if (target_bss != NULL) {
		os_memset(bh->target_ssid,0,MAX_SSID_LEN);
		os_memcpy(bh->target_ssid, target_bss->ssid, target_bss->ssid_len);
		bh->ssid_len=target_bss->ssid_len;
		map_get_info_from_wapp(ctx, WAPP_USER_SET_BACKHAUL_STEER, WAPP_BACKHAUL_STEER_RSP, ctx->bsteer_req.backhaul_mac, NULL,
				(void *)bh, sizeof(struct backhaul_steer_request_extended));
	} else {
		struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
		err("BSS does not exist");
		memcpy(ctx->bsteer_rsp.backhaul_mac, ctx->bsteer_req.backhaul_mac, ETH_ALEN);
		memcpy(ctx->bsteer_rsp.target_bssid, ctx->bsteer_req.target_bssid, ETH_ALEN);
		ctx->bsteer_rsp.status = 0x01;
		map_1905_Set_Bh_Steer_Rsp_Info(mapd_ctx->_1905_ctrl, &ctx->bsteer_rsp);
	}
	os_free(bh);
}
void update_bh_priority(struct mapd_global *pGlobal, unsigned int priority_2g, unsigned int priority_5gl, unsigned int priority_5gh)
{
	struct bh_link_entry *bh_entry = NULL;
	SLIST_FOREACH(bh_entry, &(pGlobal->dev.bh_link_head), next_bh_link) {
		if (bh_entry->bh_channel <= 14) {
			bh_entry->priority_info.priority = priority_2g;
		} else if (bh_entry->bh_channel > 14 && bh_entry->bh_channel < 100) {
			bh_entry->priority_info.priority = priority_5gl;
		} else if (bh_entry->bh_channel >= 100) {
			bh_entry->priority_info.priority = priority_5gh;
		}
	}
}
static int backhaul_steer_req_handle(struct own_1905_device *ctx, unsigned char *buf, int len)
{
	unsigned char bh_steer_triggered = FALSE;
	struct bh_link_entry *bh_entry;
	struct mapd_global *global;
	char cmd[100] = {0};
	unsigned char invalid_ch_bss = DEFAULT_ERROR_TLV_REASON;
	err("got BACKHAUL_STEERING_REQUEST");
	//mapd_hexdump(MSG_ERROR,"BACKHAUL_STEERING_REQUEST", (unsigned char *)(buf - ETH_HLEN), len);

	global = (struct mapd_global *)(ctx->back_ptr);
	/*parse BACKHAUL_STEERING_REQUEST msg */
	if (0 > parse_backhaul_steering_request_message(ctx, buf)) {
		debug("error! no need to response this backhaul steering request message");
		return -1;
	}

	if((ctx->bh_cu_params.bh_switch_cu_en) && (eloop_is_timeout_registered(band_switch_by_cu_timeout,
							(void *)ctx, NULL))) {
		struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
		err("is still in cu overload switch");
		memcpy(ctx->bsteer_rsp.backhaul_mac, ctx->bsteer_req.backhaul_mac, ETH_ALEN);
		memcpy(ctx->bsteer_rsp.target_bssid, ctx->bsteer_req.target_bssid, ETH_ALEN);
		ctx->bsteer_rsp.status = 0x01;
		map_1905_Set_Bh_Steer_Rsp_Info(mapd_ctx->_1905_ctrl, &ctx->bsteer_rsp);
		return 0;
	}

	//! ToDo Need add capability to process 80Mhz Channels, controller should also send appropriate 80 Mhz freq
	if ((global->params.Certification && IsValidChannel(ctx->bsteer_req.channel)) ||
			IsValidChannel(ctx->bsteer_req.channel))

	{
		struct backhaul_steer_request_extended *bh = NULL;
		bh=os_zalloc(sizeof (struct backhaul_steer_request_extended));
		if(bh==NULL)
		return 0;
		//copy steer req inside the request extended
		os_memcpy(&bh->request,&ctx->bsteer_req,sizeof(ctx->bsteer_req) );

		debug("backhaul_mac(%02x:%02x:%02x:%02x:%02x:%02x)"
				"target_bssid(%02x:%02x:%02x:%02x:%02x:%02x)"
				"operating class=%d, channel=%d",
				PRINT_MAC(bh->request.backhaul_mac),
				PRINT_MAC(bh->request.target_bssid),
				bh->request.oper_class, bh->request.channel);
		//find ssid and copy to req extended
		struct _1905_map_device *target_dev = topo_srv_get_1905_by_bssid(ctx, bh->request.target_bssid);
		struct bss_info_db *target_bss = topo_srv_get_bss_by_bssid(ctx, target_dev, bh->request.target_bssid);
		if (target_bss != NULL) {
			os_memset(bh->target_ssid,0,MAX_SSID_LEN);
			os_memcpy(bh->target_ssid, target_bss->ssid, target_bss->ssid_len);
			bh->ssid_len=target_bss->ssid_len;
			debug("SSID of target BSS %s , ssid_len %d",bh->target_ssid, bh->ssid_len);
			if (!global->params.Certification){
				SLIST_FOREACH(bh_entry, &(ctx->bh_link_head), next_bh_link) {

					if (os_memcmp(bh_entry->mac_addr, bh->request.backhaul_mac,ETH_ALEN) == 0) {
						ctx->current_bh_state = BH_STATE_WIFI_BH_STEER;
						ctx->current_bh_substate = BH_SUBSTATE_IDLE;
						/* Abort MBH */
						ctx->bh_dup_entry = NULL;

						/*To Do Instead of all channel scanning only single channel should be done*/
						//bh_entry->bss.Channel = bh->request.channel;
						os_memcpy(ctx->bh_steer_bssid, bh->request.target_bssid, ETH_ALEN);
						os_memset(cmd, 0, 100);
						ctx->bh_steer_channel = bh->request.channel;
						ctx->bh_steer_bh_entry = bh_entry;
						eloop_register_timeout(global->dev.bh_steer_timeout,0,send_bh_steering_fail,ctx,NULL);
						ap_selection_issue_scan(ctx);
					}
				}
			}
			else {
				//send command to wappd
				if(global->params.Certification){
					struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
					memcpy(ctx->bsteer_rsp.backhaul_mac, ctx->bsteer_req.backhaul_mac, ETH_ALEN);
					memcpy(ctx->bsteer_rsp.target_bssid, ctx->bsteer_req.target_bssid, ETH_ALEN);
					ctx->bsteer_rsp.status = 0x00;
					map_1905_Set_Bh_Steer_Rsp_Info(mapd_ctx->_1905_ctrl, &ctx->bsteer_rsp);
					send_bh_steering_req_to_wapp(ctx, NULL);
				} else
					map_get_info_from_wapp(ctx, WAPP_USER_SET_BACKHAUL_STEER, WAPP_BACKHAUL_STEER_RSP, ctx->bsteer_req.backhaul_mac, NULL,
							(void *)bh, sizeof(struct backhaul_steer_request_extended));
			}
			bh_steer_triggered = TRUE;
		}
		invalid_ch_bss = FAIL_TARGET_BSS;
		os_free(bh);

	} else {
		invalid_ch_bss = FAIL_CH_OP_CLASS;
	}

	if (bh_steer_triggered == FALSE){
		struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
		err("invalid channel or opclass or BSS does not exist: %d", invalid_ch_bss);
		memcpy(ctx->bsteer_rsp.backhaul_mac, ctx->bsteer_req.backhaul_mac, ETH_ALEN);
		memcpy(ctx->bsteer_rsp.target_bssid, ctx->bsteer_req.target_bssid, ETH_ALEN);
		ctx->bsteer_rsp.status = 0x01;
		ctx->bsteer_rsp.error = invalid_ch_bss;
		map_1905_Set_Bh_Steer_Rsp_Info(mapd_ctx->_1905_ctrl, &ctx->bsteer_rsp);
	}

	return 0;
}

void mapd_ctrl_sendto_app_inf(struct mapd_global *global, unsigned char* buf, int buf_len)
{
	struct mapd_user_higher_layer_data_event *client_notif = NULL;
	struct mapd_user_event *user_event = NULL;
	struct ctrl_iface_global_priv *priv = global->ctrl_iface;

	user_event = (struct mapd_user_event *)os_zalloc(sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_higher_layer_data_event));
	if (!user_event) {
		err("mem alloc failed");
		return;
	}
	os_memset(user_event, 0, sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_higher_layer_data_event));

	user_event->event_id = HIGHER_LAYER_PAYLOAD;
	client_notif = (struct mapd_user_higher_layer_data_event *)user_event->event_body;

	client_notif->buf_len = buf_len;
	os_memcpy(client_notif->buf, buf, buf_len);

	if (!dl_list_empty(&priv->ctrl_dst)) {
		mapd_ctrl_iface_send(global,
					priv->sock,
					&priv->ctrl_dst,
					(const char *)user_event, sizeof(struct mapd_user_event) +
						sizeof(struct mapd_user_higher_layer_data_event),
					priv);
	}
	os_free(user_event);
}

extern void mapd_start_wired_iface_monitor(void *eloop_ctx, void *timeout_ctx);

/**
* @brief Fn to parse different 1905 events
*
* @param mapd_ctx global mapd ctx
* @param buff msg buffer
* @param len msg len
*
* @return -1 if error else 0
*/
int _1905_interface_parse_event(struct mapd_global *mapd_ctx, unsigned char *buff, int len)
{
	struct msg *_1905_event = NULL;
	struct own_1905_device *global = &mapd_ctx->dev;
	unsigned char *buf;
	int status;
	int wapp_cmd_role=0;
	struct agent_list *a = NULL;


	_1905_event = (struct msg *)buff;
	hex_dump("1905 msg", buff, len);
	buf = _1905_event->buffer;

	debug("got event=%u len=%d", _1905_event->type, len);
	switch (_1905_event->type) {
	case _1905_SET_WIRELESS_SETTING_EVENT:
		/* we should update our data only after getting a success from here,
		   TODO move topo part to ack once that is available */
		info("got wireless settings %u", _1905_event->length);
		mapd_hexdump(MSG_DEBUG, "wsc packet", buf, _1905_event->length);
		/* This will be reset in WAPP_OPERBSS_REPORT as response
		 * to WAPP_USER_SET_WIRELESS_SETTING */
		global->config_status = DEVICE_CONFIG_ONGOING;
		if(global->device_role > DEVICE_ROLE_INVALID)
		{
			wapp_cmd_role=global->device_role;
		}
		status = wlanif_issue_wapp_command((struct mapd_global *)global->back_ptr,
				WAPP_USER_SET_WIRELESS_SETTING, 0,
				NULL, NULL, buf, len, 0, 0, wapp_cmd_role);
		if (status != 0) {
			/* lets try to send this msg after 2 sec */
			//TODO
		}
		topo_srv_update_wireless_setting(global, buf, len);

		break;
	case _1905_RECV_CH_SELECTION_RSP_EVENT:
		{
			unsigned char *almac = buf;
			struct _1905_map_device *peer_1905 = topo_srv_get_1905_device(global,almac);

			if (peer_1905 == global->ch_planning.current_ch_planning_dev) {
				err("received channel_selection resp %02x:%02x:%02x:%02x:%02x:%02x",
						PRINT_MAC(peer_1905->_1905_info.al_mac_addr));
				eloop_cancel_timeout(ch_planning_timeout_handler, global, peer_1905);
				eloop_register_timeout(25, 0, ch_planning_timeout_handler,
						global,peer_1905);
#ifdef MAPR2
				ch_planning_handle_ch_selection_rsp(global,peer_1905);
#endif
			}
			break;
		}
	case _1905_RECV_CHANNEL_SELECTION_REQ_EVENT:
		{
			info("received Channel selection req");
			_1905_2_wapp_cert_channel_setting_event(global, buf, len);
			break;
		}
	case _1905_RECV_CHANNEL_PREFERENCE_QUERY_EVENT:
		info("received _1905_RECV_CHANNEL_PREFERENCE_QUERY_EVENT");
#ifdef MAP_R2
		topo_srv_get_cac_statusinfo(&mapd_ctx->dev);
#else
		_1905_update_channel_pref_report(&mapd_ctx->dev, NULL);
#endif
		break;
	case _1905_RECV_STEERING_REQUEST_EVENT:
		//ap_roam_algo_select_steer_candidates(global, buf, len);
		chan_mon_handle_steering_req(mapd_ctx, buf + ETH_ALEN,len);
		break;
	case _1905_RECV_CLIENT_ASSOC_CNTRL_SETTING_EVENT:
		parse_cli_assoc_control_request_message(global, buf, len);
		break;
	case _1905_RECV_AP_METRICS_QUERY_EVENT:
		if( topo_srv_handle_metrics_query(global, buf + ETH_ALEN, len,0) != -1)
			_1905_if_send_ap_metric_rsp(global
#if defined(MAP_R2) || defined(CENT_STR)
                         ,0
#endif
                         );

		break;
#if defined(MAP_R2) || defined(CENT_STR)
	case _1905_RECV_AP_METRICS_QUERY_PERIODIC_EVENT:
		if( topo_srv_handle_metrics_query(global, buf + ETH_ALEN, len,1) != -1) {
			_1905_if_send_ap_metric_rsp(global,1);
		}
              break;
#endif

	case _1905_RECV_BACKHAUL_STEER_REQ_EVENT:
		backhaul_steer_req_handle(global, buf + ETH_ALEN, len);
		break;
	case _1905_RECV_ASSOC_STA_LINK_METRICS_QUERY_EVENT:
		if (topo_srv_handle_assoc_sta_metrics_query(global, buf+ETH_ALEN, len) == -1)
			break;
#ifdef MAP_R2
		_1905_if_send_sta_metric_rsp(global);
#endif
		break;
	case _1905_RECV_LINK_METRICS_QUERY:
		topo_srv_handle_link_metrics_query(global, buf+ETH_ALEN);
		break;
	case _1905_RECV_UNASSOC_STA_LINK_METRICS_QUERY_EVENT:
		topo_srv_handle_unassoc_sta_link_metrics_query(global, buf+ETH_ALEN);
		break;
	case _1905_SET_RADIO_TEARED_DOWN_EVENT:
		{
			struct radio_info_db *radio = topo_srv_get_radio(topo_srv_get_1905_device(global, NULL), buf);
			if (radio && radio->is_configured == FALSE) {
				radio->is_configured = TRUE;
				topo_srv_update_own_device_config_status(global);
			}
			wlanif_issue_wapp_command((struct mapd_global *)global->back_ptr, WAPP_USER_SET_RADIO_TEARED_DOWN,
				0, NULL, NULL, buf, ETH_ALEN, 0, 0, 0);
		}
		break;
	case _1905_AUTOCONFIG_RENEW_EVENT:
		wlanif_issue_wapp_command((struct mapd_global *)global->back_ptr, WAPP_USER_SET_RADIO_RENEW,
			0, NULL, NULL, buf + ETH_ALEN, 1, 0, 0, 0);
		break;
	case _1905_RECV_BEACON_METRICS_QUERY_EVENT:
		topo_srv_handle_beacon_metrics_query(global, buf + ETH_ALEN);
		break;
	case _1905_MAP_CONTROLLER_FOUND_EVENT:
		/* TODO do we need to update +6 in this?? */
		map_get_info_from_wapp(global, WAPP_USER_MAP_CONTROLLER_FOUND, 0, NULL, NULL, buf + 4, ETH_ALEN);
		/* check whether auto configuration was on, if yes, set it as agent */
		if (global->auto_role_detect) {
			map_1905_controller_found(global);
			eloop_cancel_timeout(mapd_start_wired_iface_monitor, mapd_ctx, NULL);
			eloop_cancel_timeout(map_1905_poll_timeout, mapd_ctx, global);
			eloop_cancel_timeout(map_start_auto_role_detection, mapd_ctx, global);
			global->auto_role_detect = 0;
			SLIST_FOREACH(a, &global->a_list, next_agent) {
				SLIST_REMOVE(&global->a_list, a, agent_list, next_agent);
				os_free(a);
			}
			SLIST_INIT(&global->a_list);
			mapfilter_set_drop_specific_dest_ip_status(0);
		}
		break;
	case _1905_RECV_POLICY_CONFIG_REQUEST_EVENT:
		parse_map_policy_config_request_message(global, buf + 6);
		wapp_set_metrics_policy_setting(global, &global->map_policy.mpolicy);
#ifdef MAP_R2
		wapp_set_unsuccessful_association_policy_setting(global, &global->map_policy.assoc_failed_policy);
#endif
		break;
	case _1905_RECV_COMBINED_INFRASTRUCTURE_METRICS_EVENT:
		topo_srv_parse_combined_infra_msg(global, buf + 6);
		break;
	case _1905_RECV_AP_CAPABILITY_REPORT_EVENT:
		topo_srv_prase_ap_cap_report(global, buf);
		break;
	case _1905_RECV_TOPOLOGY_RSP_EVENT:
		info("got topo rsp event len=%d", len);
		topo_srv_handle_topology_event(global, buf, _1905_event->length);
		break;
	case _1905_RECV_TOPOLOGY_NOTIFICATION_EVENT:
		{
			info("got topology notification");
			unsigned char *tmp_buf[200];
			struct topo_notification *evt = (struct topo_notification *)tmp_buf;
			parse_topology_notification_evt(global, buf + 6, evt);
			topo_srv_issue_disconnect_if_local(global, evt);
			duplicate_sta_check_for_notification_evt(global, evt);
			client_mon_handle_topo_notification(global->back_ptr,evt);
		}
		break;
	case _1905_RECV_CHANNEL_PREFERENCE_REPORT_EVENT:
		topo_srv_update_chan_preference(global, buf, len);
		break;
	case _1905_RECV_OPERATING_CH_REPORT_EVENT:
		ch_planning_handle_operating_channel_report(global, buf, len);
		break;
	case _1905_RECV_CLI_STEER_BTM_REPORT_EVENT:
		/* TOD handle btm report msg, discuss what needs to be done for this */
		topo_srv_handle_client_steer_btm_report(global, buf, len);
		break;
	case _1905_RECV_STEER_COMPLETE_EVENT:
		topo_srv_handle_steer_complete(global, buf, len);
		break;
	case _1905_RECV_LINK_METRICS_RSP_EVENT:
		debug("_1905_RECV_LINK_METRICS_RSP_EVENT from 1905");
		topo_srv_handle_link_metrics_rsp_event(global, buf, len);

		break;
	case _1905_RECV_ASSOC_STA_LINK_METRICS_RSP_EVENT:
		/* TODO add handling */
		topo_srv_handle_assoc_link_metrics_rsp(global, buf, len);
		break;
	case _1905_RECV_UNASSOC_STA_LINK_METRICS_RSP_EVENT:
		topo_srv_handle_unassoc_link_metrics_rsp(global, buf, len);
		break;
	case _1905_RECV_BCN_METRICS_RSP_EVENT:
		/* TODO add handling */
		topo_srv_handle_beacon_metrics_rsp_event(global, buf, len);
		break;
	case _1905_RECV_BACKHAUL_STEER_RSP_EVENT:
		topo_srv_handle_backhaul_steer_rsp(global, buf, len);
		break;
	case _1905_RECV_AP_METRICS_RSP_EVENT:
		topo_srv_handle_ap_metrics_rsp(global, buf, len);
		break;
	case _1905_RECV_VENDOR_SPECIFIC_MESSAGE_EVENT:
		_1905_handle_vendor_msg(mapd_ctx, buf, len);
		break;
	case _1905_RECV_COMBINED_INFRASTRUCTURE_METRICS_QUERY_EVENT:
		info("command(%04x) from 1905", _1905_event->type);
		eloop_register_timeout(0, 0, infra_metrics_srv_send_cb_infra_metrics, mapd_ctx, global);
		break;
	case _1905_AUTOCONFIG_SEARCH_EVENT:
		if (global->auto_role_detect == 2) {
			SLIST_FOREACH(a, &global->a_list, next_agent)
			{
				if (!os_memcmp(a->almac, buf, ETH_ALEN))
					break;
			}
			if (!a) {
				a = (struct agent_list *)os_zalloc(sizeof(struct agent_list));
				if (!a) {
					err("Mem alloc failed");
					return -1;
				}
				os_memcpy(a->almac, buf, ETH_ALEN);
				err("Added search agent almac(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(a->almac));
				SLIST_INSERT_HEAD(&global->a_list, a, next_agent);
			}
		}
		break;
	case _1905_AUTOCONFIG_RSP_EVENT:
		if (global->auto_role_detect == 2) {
			map_1905_controller_found(global);
			err("_1905_AUTOCONFIG_RSP_EVENT-updated device role as agent");
			eloop_cancel_timeout(map_1905_poll_timeout, global->back_ptr, global);
			eloop_cancel_timeout(map_start_auto_role_detection, global->back_ptr, global);
			global->auto_role_detect = 0;
			SLIST_FOREACH(a, &global->a_list, next_agent) {
				SLIST_REMOVE(&global->a_list, a, agent_list, next_agent);
				os_free(a);
			}
			SLIST_INIT(&global->a_list);
			mapfilter_set_drop_specific_dest_ip_status(0);
		}
		break;
#ifdef MAP_R2
	case _1905_RECV_CHANNEL_SCAN_REQ_EVENT:
		info("command(%04x) from 1905", _1905_event->type);
		err("_1905_RECV_CHANNEL_SCAN_REQ_EVENT: Send to WAPP");
		mapd_hexdump(MSG_OFF, "Ch_Scan_req", _1905_event->buffer, _1905_event->length);
		topo_srv_handle_ch_scan_req(global, _1905_event->buffer+6, _1905_event->length-6);
		break;
	case _1905_RECV_CHANNEL_SCAN_REP_EVENT:
		// TODO: controller functionality. Nothing for plugfest
		err("_1905_RECV_CHANNEL_SCAN_REP_EVENT: command(%04x) from 1905", _1905_event->type);
		//mapd_hexdump(MSG_OFF, "Ch_Scan_rep", _1905_event->buffer, _1905_event->length);
		topo_srv_handle_ch_scan_report((struct mapd_global *)global->back_ptr, _1905_event->buffer, _1905_event->length);
		break;
	case _1905_RECV_TUNNELED_MESSAGE_EVENT:
		// TODO: controller functionality. Nothing for plugfest
		err("_1905_RECV_TUNNELED_MESSAGE_EVENT: command(%04x) from 1905", _1905_event->type);
	//	mapd_hexdump(MSG_OFF, "Tunneled Message", _1905_event->buffer, _1905_event->length);
		topo_srv_handle_tunneled_msg((struct mapd_global *)global->back_ptr, _1905_event->buffer, _1905_event->length);
		break;
		//Prakhar
	case _1905_RECV_ASSOCIATION_STATUS_NOTIFICATION_EVENT:
		// TODO: controller functionality. Nothing for plugfest
		topo_srv_handle_assoc_status_notif_event((struct mapd_global *)global->back_ptr, _1905_event->buffer, _1905_event->length);
		err("_1905_RECV_ASSOC_NOTIFICATION_EVENT: command(%04x) from 1905", _1905_event->type);
	//	mapd_hexdump(MSG_OFF, "Assoc Status Notification Message", _1905_event->buffer, _1905_event->length);
		break;
#ifdef DFS_CAC_R2
	case _1905_RECV_CAC_REQUEST_EVENT:
		err("_1905_RECV_CHANNEL_CAC_REQUST _EVENT: command(%04x) from 1905", _1905_event->type);
		mapd_hexdump(MSG_OFF, "_1905_RECV_CHANNEL_CAC_REQUST _EVENT", _1905_event->buffer, _1905_event->length);
		topo_srv_handle_cac_req(global, _1905_event->buffer+6, _1905_event->length-6);
		break;
	case _1905_RECV_CAC_TERMINATE_EVENT:
		err("_1905_RECV_CHANNEL_CAC_TERMINATE _EVENT: command(%04x) from 1905", _1905_event->type);
		mapd_hexdump(MSG_OFF, "_1905_RECV_CHANNEL_CAC_TERMINATE _EVENT", _1905_event->buffer, _1905_event->length);
		topo_srv_handle_cac_terminate(global, _1905_event->buffer+6, _1905_event->length-6);
		break;
#endif
	case _1905_SET_TRAFFIC_SEPARATION_SETTING_EVENT:
		err("got ts settings  event%u", _1905_event->length);
		status = wlanif_issue_wapp_command((struct mapd_global *)global->back_ptr,
				WAPP_USER_SET_TRAFFIC_SEPARATION_SETTING, 0,
				NULL, NULL, buf, _1905_event->length, 0, 0, wapp_cmd_role);
		break;
	case _1905_SET_TRANSPARENT_VLAN_SETTING_EVENT:
		err("got transparent settings  event%u", _1905_event->length);
		status = wlanif_issue_wapp_command((struct mapd_global *)global->back_ptr,
				WAPP_USER_SET_TRANSPARENT_VLAN_SETTING, 0,
				NULL, NULL, buf, _1905_event->length, 0, 0, wapp_cmd_role);
		break;
	case _1905_RECV_CLIENT_DISASSOCIATION_STATS_EVENT:
		topo_srv_handle_disassoc_stats_event((struct mapd_global *)global->back_ptr, _1905_event->buffer, _1905_event->length);
		err("_1905_RECV_CLIENT_DISASSOCIATION_STATS_EVENT: command(%04x) from 1905", _1905_event->type);
		break;
	case _1905_RECV_BACKHAUL_STA_CAP_REPORT_EVENT:
		topo_srv_handle_bh_sta_report((struct mapd_global *)global->back_ptr, _1905_event->buffer, _1905_event->length);
	case _1905_RECV_ERROR_CODE_EVENT:
		mapd_hexdump(MSG_OFF, "_1905_RECV_ERROR_CODE_EVENT", _1905_event->buffer, _1905_event->length);
		topo_srv_handle_ack_msg((struct mapd_global *)global->back_ptr, _1905_event->buffer, _1905_event->length);
		break;
#endif
	case _1905_RECV_HIGHER_LAYER_DATA_EVENT:
		{
		int hl_payload_len, hl_protocol;

		always("1905 higher layer data recvd in mapd.Buff_len:%d \n", _1905_event->length);
		buf = buf + ETH_ALEN;
		if (*buf != 0xa0) {
			err("Invalid TLV for higher layer protocol data\n");
			return -1;
		}

		/*Higher Layer Data TLV format
		| Field		| Length octets	| Value	| Description
		------------------------------------------------------------
		| tlvType	| 1		| 0xA0	| Higher layer data TLV
		| tlvLength	| 2		| Var	| Number of octets in ensuing field
		| tlvValue	| 1		| Var	| Higher layer protocol
					| Var	| Var	| Higher layer protocol payload
		*/

		hl_payload_len = *(unsigned short *)(buf+1);
		hl_payload_len = SWAP16(hl_payload_len);
		/* len in payload includes protocol byte also */
		hl_protocol = *(buf+3);
		info("payload length: %d Protocol: %d\n", hl_payload_len, hl_protocol);

		/*Pass protocol and payload to application  */
		mapd_ctrl_sendto_app_inf(global->back_ptr, buf+3, hl_payload_len);
		break;
		}
#ifdef CENT_STR
	case _1905_RECV_CLIENT_CAPABILITY_REPORT_EVENT:
		if(mapd_ctx->dev.cent_str_en)
			topo_srv_handle_client_cap_report(mapd_ctx, buf, len);
		break;
#endif
	default:
		info("unknow command(%04x) from 1905", _1905_event->type);
		break;
	}
	return 0;
}
#endif				/*  */
