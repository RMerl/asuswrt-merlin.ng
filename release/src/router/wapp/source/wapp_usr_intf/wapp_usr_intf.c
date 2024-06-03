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

/*this file include some function to test 1905 deamon*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stddef.h>

#include "rt_config.h"
#include "wapp_usr_intf.h"
#include "interface.h"
#include "wapp_cmm.h"

unsigned char default_match_func(struct msg* event, struct cmd_to_wapp* stored_cmd);
extern int map_handler(struct wifi_app *wapp, char *buffer_recv, int len_recv, char* event_buf, int* len_event);
struct _map_cmd_event _mapping[] =
{
		{WAPP_USER_GET_CLI_CAPABILITY_REPORT, 		0,									SYNC,	NULL},
		{WAPP_USER_SET_WIRELESS_SETTING, 			WAPP_OPERBSS_REPORT,				SYNC,	NULL},
		{WAPP_USER_GET_RADIO_BASIC_CAP, 			WAPP_RADIO_BASIC_CAP,				SYNC,	NULL},
		{WAPP_USER_GET_AP_CAPABILITY, 				WAPP_AP_CAPABILITY,					SYNC,	NULL},
		{WAPP_USER_GET_AP_HT_CAPABILITY, 			WAPP_AP_HT_CAPABILITY,				SYNC,	NULL},
		{WAPP_USER_GET_AP_VHT_CAPABILITY, 			WAPP_AP_VHT_CAPABILITY,				SYNC,	NULL},
		{WAPP_USER_GET_SUPPORTED_SERVICE, 			0,									SYNC,	NULL},
		{WAPP_USER_GET_SEARCHED_SERVICE, 			0,									SYNC,	NULL},
		{WAPP_USER_GET_ASSOCIATED_CLIENT, 			0,									SYNC,	NULL},
		{WAPP_USER_GET_RA_OP_RESTRICTION, 			WAPP_RADIO_OPERATION_RESTRICTION,	SYNC,	NULL},
		{WAPP_USER_GET_CHANNEL_PREFERENCE, 			WAPP_CHANNLE_PREFERENCE,			SYNC,	NULL},
		{WAPP_USER_SET_CHANNEL_SETTING, 			0,									SYNC,	NULL},
		{WAPP_USER_GET_OPERATIONAL_BSS, 			WAPP_OPERBSS_REPORT,				SYNC,	NULL},
		{WAPP_USER_SET_STEERING_SETTING, 			WAPP_CLI_STEER_BTM_REPORT,			ASYNC,	default_match_func},
		{WAPP_USER_SET_STEERING_SETTING,			WAPP_STEERING_COMPLETED,			SYNC,	NULL},
		{WAPP_USER_SET_ASSOC_CNTRL_SETTING, 		0,									SYNC,	NULL},
		{WAPP_USER_SET_LOCAL_STEER_DISALLOW_STA, 	0,									SYNC,	NULL},
		{WAPP_USER_SET_BTM_STEER_DISALLOW_STA, 		0,									SYNC,	NULL},
		{WAPP_USER_SET_RADIO_CONTROL_POLICY, 		0,									SYNC,	NULL},
		{WAPP_USER_GET_AP_METRICS_INFO, 			WAPP_AP_METRICS_INFO,				ASYNC,	default_match_func},
		{WAPP_USER_MAP_CONTROLLER_FOUND, 			0,									SYNC,	NULL},
		{WAPP_USER_SET_BACKHAUL_STEER, 				WAPP_BACKHAUL_STEER_RSP,			ASYNC,	default_match_func},
		{WAPP_USER_SET_METIRCS_POLICY, 				0,									SYNC,	NULL},
		{WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS, 	WAPP_ALL_ASSOC_STA_TRAFFIC_STATS,	ASYNC,	default_match_func},
		{WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS, WAPP_ONE_ASSOC_STA_TRAFFIC_STATS,	ASYNC,	default_match_func},
		{WAPP_USER_GET_ASSOC_STA_LINK_METRICS, 		WAPP_ALL_ASSOC_STA_LINK_METRICS,	ASYNC,	default_match_func},
		{WAPP_USER_GET_ALL_ASSOC_TP_METRICS, 		WAPP_ALL_ASSOC_TP_METRICS,			ASYNC,	default_match_func},
		{WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS, 	WAPP_ONE_ASSOC_STA_LINK_METRICS,	ASYNC,	default_match_func},
		{WAPP_USER_GET_TX_LINK_STATISTICS, 			WAPP_TX_LINK_STATISTICS,			ASYNC,	default_match_func},
		{WAPP_USER_GET_RX_LINK_STATISTICS, 			WAPP_RX_LINK_STATISTICS,			ASYNC,	default_match_func},
		{WAPP_USER_GET_UNASSOC_STA_LINK_METRICS, 	WAPP_UNASSOC_STA_LINK_METRICS,		SYNC,	NULL},
		{WAPP_USER_SET_RADIO_TEARED_DOWN, 			0,									SYNC,	NULL},
		{WAPP_USER_SET_BEACON_METRICS_QRY, 			WAPP_BEACON_METRICS_REPORT,			ASYNC,	default_match_func},
		{WAPP_USER_GET_AP_HE_CAPABILITY, 			WAPP_AP_HE_CAPABILITY,				SYNC,	NULL},
		{WAPP_USER_FLUSH_ACL, 						0,									SYNC,	NULL},
		{WAPP_USER_GET_BSSLOAD, 					WAPP_STA_BSSLOAD,					ASYNC,	default_match_func},
		{WAPP_USER_GET_RSSI_REQ, 					WAPP_STA_RSSI,						ASYNC,	default_match_func},
		{WAPP_USER_SET_WHPROBE_REQ, 				0,									SYNC,	NULL},
		{WAPP_USER_SET_NAC_REQ, 					WAPP_NAC_INFO,						ASYNC,	default_match_func},
		{WAPP_USER_SET_VENDOR_IE, 					0,									SYNC,	NULL},
		{WAPP_USER_GET_APCLI_RSSI_REQ, 				WAPP_APCLI_UPLINK_RSSI,				ASYNC,	default_match_func},
		{WAPP_USER_GET_WIRELESS_INF_INFO, 			WAPP_WIRELESS_INF_INFO,				SYNC,	NULL},
		{WAPP_USER_SET_ADDITIONAL_BH_ASSOC,			0,									SYNC,	NULL},
		{WAPP_USER_SET_AIR_MONITOR_REQUEST, 		WAPP_AIR_MONITOR_REPORT,			ASYNC,	default_match_func},
#ifdef MAP_R2
		{WAPP_USER_GET_SCAN_CAP,					WAPP_CHANNEL_SCAN_CAPAB		,						SYNC,	default_match_func},
#ifdef DFS_CAC_R2
		{WAPP_USER_GET_CAC_CAP,						WAPP_CAC_CAPAB,						SYNC,	default_match_func},
#endif
#endif
};

int wapp_iface_open_sock(struct wifi_app *wapp, char* socket_patch);

struct _map_cmd_event* lookup_mapping_entry(unsigned short event, unsigned short cmd)
{
	int i = 0;

	for (i = 0; i < sizeof(_mapping)/sizeof(struct _map_cmd_event); i++) {
		if (event != 0) {
			if(_mapping[i].event == event)
				return &_mapping[i];
		} else if (cmd != 0) {
			if (_mapping[i].cmd == cmd)
				return &_mapping[i];
		}
	}
	return NULL;
}

unsigned char is_async(struct cmd_to_wapp* cmd)
{
	struct _map_cmd_event* map_entry = NULL;

	map_entry = lookup_mapping_entry(0, cmd->type);
	if (map_entry == NULL)
		return 0;
	else
		return (map_entry->type == ASYNC);
}

unsigned char default_match_func(struct msg* event, struct cmd_to_wapp* stored_cmd)
{
	struct _map_cmd_event* map_entry = NULL;

	map_entry = lookup_mapping_entry(event->type,0);
	if (map_entry == NULL)
		return 0;

	if (map_entry->cmd == stored_cmd->type) {
		DBGPRINT(RT_DEBUG_TRACE, "default match func event=(%02x) stored_cmd=(%02x)\n", event->type, stored_cmd->type);
		//printf("default match func event=(%02x) stored_cmd=(%02x)\n", event->type, stored_cmd->type);
		return 1;
	}

	return 0;
}


void printf_encode(char *txt, size_t maxlen, const unsigned char *data, size_t len)
{
	char *end = txt + maxlen;
	size_t i;

	for (i = 0; i < len; i++) {
		if (txt + 4 >= end)
			break;

		switch (data[i]) {
		case '\"':
			*txt++ = '\\';
			*txt++ = '\"';
			break;
		case '\\':
			*txt++ = '\\';
			*txt++ = '\\';
			break;
		case '\033':
			*txt++ = '\\';
			*txt++ = 'e';
			break;
		case '\n':
			*txt++ = '\\';
			*txt++ = 'n';
			break;
		case '\r':
			*txt++ = '\\';
			*txt++ = 'r';
			break;
		case '\t':
			*txt++ = '\\';
			*txt++ = 't';
			break;
		default:
			if (data[i] >= 32 && data[i] <= 127) {
				*txt++ = data[i];
			} else {
				txt += os_snprintf(txt, end - txt, "\\x%02x",
						   data[i]);
			}
			break;
		}
	}

	*txt = '\0';
}

int is_client_event_registered(unsigned short event, unsigned char registed_event_bitmap[], int size)
{
	unsigned short i = 0, j = 0;
	unsigned char offset = 0;

	i = event / (sizeof(unsigned char) * 8);
	j = event % (sizeof(unsigned char) * 8);
	offset = 0x01 << j;
	if (i >= size)
		return 0;

	if (registed_event_bitmap[i] & offset)
		return 1;
	else
		return 0;
}

int register_event(struct wapp_usr_intf_cli* cli, unsigned short event)
{
	int index = 0;
	int offset = 0;
#if 0
	struct _map_cmd_event* map_entry = NULL;

	map_entry = lookup_mapping_entry(event, 0);

	if(map_entry == NULL)
	{
		printf("event not find\n");
		return -1;
	}
#endif

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	index = event / (sizeof(cli->registerd_event_bitmap[0]) * 8);
	if (index >= (sizeof(cli->registerd_event_bitmap) / sizeof(cli->registerd_event_bitmap[0])))
		return -1;
	offset = event % (sizeof(cli->registerd_event_bitmap[0]) * 8);
	cli->registerd_event_bitmap[index] |= (0x01 << offset);

	return 0;
}

int wapp_iface_init(struct wifi_app *wapp)
{
	struct wapp_usr_intf_cli_ctrl* cli_ctrl = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	cli_ctrl = &wapp->infcli_ctrl;
	dl_list_init(&cli_ctrl->daemon_cli_list);

	if (wapp_iface_open_sock(wapp, "/tmp/wapp_server") < 0) {
		return -1;
	}

	return 0;
}


int wapp_iface_reinit(struct wifi_app *wapp)
{
	int res;
	struct wapp_usr_intf_cli_ctrl* cli_ctrl = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	cli_ctrl = &wapp->infcli_ctrl;
	if (cli_ctrl->sock <= 0)
		return -1;

	eloop_unregister_read_sock(cli_ctrl->sock);
	close(cli_ctrl->sock);
	cli_ctrl->sock = -1;

	res = wapp_iface_open_sock(wapp, "/tmp/wapp_server");
	if (res < 0)
		return -1;
	return cli_ctrl->sock;
}

void wapp_iface_deinit(struct wifi_app *wapp)
{
	struct wapp_usr_intf_cli_ctrl* cli_ctrl = NULL;
	struct wapp_usr_intf_cli *dst, *prev;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	cli_ctrl = &wapp->infcli_ctrl;

	if (cli_ctrl->sock > -1) {
		eloop_unregister_read_sock(cli_ctrl->sock);
		if (!dl_list_empty(&cli_ctrl->daemon_cli_list)) {
			/*
			 * Wait before closing the control socket if
			 * there are any attached monitors in order to allow
			 * them to receive any pending messages.
			 */
			printf("CTRL_IFACE wait for attached monitors to receive messages");
			os_sleep(0, 100000);
		}
		close(cli_ctrl->sock);
		cli_ctrl->sock = -1;
		unlink(cli_ctrl->addr.sun_path);
	}

	dl_list_for_each_safe(dst, prev, &cli_ctrl->daemon_cli_list, struct wapp_usr_intf_cli, list)
	os_free(dst);
}

int wapp_iface_attach(
	struct dl_list *ctrl_dst,
	struct sockaddr_un *from,
	socklen_t fromlen, char* daemon_name)
{
	struct wapp_usr_intf_cli* dst, *dst_n;
	struct client_cmd* cmd, * cmd_n;
	char addr_txt[200];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	/*firstly delete the previous same attached daemon*/
	dl_list_for_each_safe(dst, dst_n, ctrl_dst, struct wapp_usr_intf_cli, list) {
		if (!os_memcmp(dst->daemon_name, daemon_name, os_strlen(daemon_name))) {
			printf("delete the previous daemon(%s)\n", daemon_name);
			dl_list_for_each_safe(cmd, cmd_n, &dst->cmd_list, struct client_cmd, list) {
				if (cmd->cmd_buf != NULL) {
					os_free(cmd->cmd_buf);
					cmd->cmd_buf = NULL;
				}
				dl_list_del(&cmd->list);
				os_free(cmd);
			}
			dl_list_del(&dst->list);
			os_free(dst);
		}
	}

	/*alloc an new client structure and add to the linklist*/
	dst = (struct wapp_usr_intf_cli* )os_zalloc(sizeof(*dst));
	if (dst == NULL)
		return -1;
	os_memcpy(&dst->addr, from, sizeof(struct sockaddr_un));
	snprintf(dst->daemon_name, sizeof(dst->daemon_name), "%s", daemon_name);
	dst->addrlen = fromlen;
	dl_list_init(&dst->cmd_list);
	dl_list_add(ctrl_dst, &dst->list);
	printf_encode(addr_txt, sizeof(addr_txt),
				(u8 *) from->sun_path,
				fromlen - offsetof(struct sockaddr_un, sun_path));
	printf("daemon %s attached %s\n", daemon_name, addr_txt);
	return 0;
}

int wapp_iface_detach(
	struct dl_list *ctrl_dst,
	struct sockaddr_un *from,
	socklen_t fromlen)
{
	struct wapp_usr_intf_cli* dst, *dst_n;
	struct client_cmd* cmd, * cmd_n;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dl_list_for_each_safe(dst, dst_n, ctrl_dst, struct wapp_usr_intf_cli, list) {
		if (fromlen == dst->addrlen &&
			os_memcmp(from->sun_path, dst->addr.sun_path, fromlen - offsetof(struct sockaddr_un, sun_path)) == 0) {
			char addr_txt[200];
			printf_encode(addr_txt, sizeof(addr_txt),
						(u8 *) from->sun_path,
						fromlen -
						offsetof(struct sockaddr_un, sun_path));
			printf("daemon %s monitor detached %s\n", dst->daemon_name, addr_txt);

			dl_list_for_each_safe(cmd, cmd_n, &dst->cmd_list, struct client_cmd, list) {
				if (cmd->cmd_buf != NULL) {
					os_free(cmd->cmd_buf);
					cmd->cmd_buf = NULL;
				}
				dl_list_del(&cmd->list);
				os_free(cmd);
			}
			dl_list_del(&dst->list);
			os_free(dst);
			return 0;
		}
	}
	return -1;
}

int wapp_iface_process(struct wifi_app *wapp, struct sockaddr_un* from,
					socklen_t fromlen, char *buf, size_t buf_len, char** reply, size_t *resp_len)
{
	struct wapp_usr_intf_cli* dst;
	struct client_cmd *cmd = NULL;
	int reply_len = 0;
	char* reply_buf = NULL;
	int ret = MAP_SUCCESS;
	unsigned short cmd_type = ((struct cmd_to_wapp *)buf)->type;
	unsigned char valid_cli = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	reply_buf = (char*)os_malloc(3072);
	if (reply_buf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "%s alloc relay_buf fail!\n", __func__);
		goto error3;
	}
	os_memset(reply_buf, 0, 3072);

	dl_list_for_each(dst, &wapp->infcli_ctrl.daemon_cli_list, struct wapp_usr_intf_cli, list) {
		if (fromlen == dst->addrlen &&
			os_memcmp(from->sun_path, dst->addr.sun_path, fromlen - offsetof(struct sockaddr_un, sun_path)) == 0) {
			DBGPRINT(RT_DEBUG_TRACE, "process cmd(%d) from daemon(%s)\n", cmd_type, dst->daemon_name);
			valid_cli = 1;
			break;
		}
	}

	/*check whether is the dst valid client*/
	if (valid_cli == 0) {
		DBGPRINT(RT_DEBUG_ERROR, "invalid daemon\n");
		goto error2;
	}

	/*check whether is it async cmd<->event, if yes, store this cmd buf for futher use, or should not store this cmd*/
	if (is_async((struct cmd_to_wapp *)buf) && buf_len > 0) {
		cmd = (struct client_cmd*)os_malloc(sizeof(struct client_cmd));
		if (cmd == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, "%s alloc cmd fail\n", __func__);
			goto error2;
		}
		cmd->cmd_buf = (struct cmd_to_wapp*)os_malloc(buf_len);
		if (cmd->cmd_buf == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, "%s alloc cmd->cmd_buf fail\n", __func__);
			goto error1;
		}
		os_memcpy(cmd->cmd_buf, buf, buf_len);
		DBGPRINT(RT_DEBUG_TRACE, "111'store cmd-%p type=%d\n", cmd, cmd_type);
		dl_list_add(&dst->cmd_list, &cmd->list);
	}

	/*add command handler here, now just add map handler*/
	/*if the command handler want to reply event, please fill the reply_buf and reply_len*/

	/*handler modify*/
	if (cmd_type > WAPP_USER_MIN_CMD && cmd_type < WAPP_USER_MAX_CMD) {
		ret = map_handler(wapp, buf, buf_len, reply_buf, &reply_len);
	}
	else if (cmd_type == SET_REGITER_EVENT) {
		unsigned short *event = (unsigned short*)((struct cmd_to_wapp*)buf)->body;
		DBGPRINT(RT_DEBUG_TRACE, "%s recv set register event\n", __func__);
		if (register_event(dst, *event) >= 0) {
			reply_len = 3;
			os_memcpy(reply_buf, "OK\n", 3);
		}
	} else {
		/*other cmd handler*/
	}
	*reply = reply_buf;
	*resp_len = reply_len;
	return ret;
error1:
	os_free(cmd);
error2:
	os_free(reply_buf);
	reply_buf = NULL;
error3:
	*reply = reply_buf;
	*resp_len = reply_len;
	return MAP_ERROR;
}


int retry_socket_send(int sock){
	fd_set FdSet;
	struct timeval timeout;
	while (1) {
		FD_ZERO(&FdSet);
		FD_SET(sock, &FdSet);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0; //no us

		int sel_value = select(sock+1, NULL, &FdSet, NULL, &timeout);
		DBGPRINT(RT_DEBUG_TRACE, "%s sel value is %d \n", __func__, sel_value);
		if(sel_value == -1){
			perror("select");
			return 0;

		}else if(sel_value == 0){
			DBGPRINT(RT_DEBUG_TRACE, "%s Timeout can not send for  5 seconds \n", __func__);
			return 0;
		}
		else{
			if(FD_ISSET(sock, &FdSet)){
				return 1;
			}
		}
	}
	return 0;

}

void wapp_socket_send(struct wifi_app* wapp, char* buf, size_t buf_len, struct wapp_usr_intf_cli* dst)
{
	int sock = 0;
	int _errno = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (sendto(wapp->infcli_ctrl.sock, buf, buf_len, 0, (struct sockaddr *) &dst->addr, dst->addrlen) >= 0) {
		dst->errors = 0;
		return;
	}
	dst->errors++;
	_errno = errno;
	DBGPRINT(RT_DEBUG_ERROR, "wapp_iface sendto failed: %d - %s\n",_errno, strerror(_errno));

	if (dst->errors > 10 || _errno == ENOENT || _errno == EPERM) {
		/*
		*DBGPRINT(RT_DEBUG_ERROR, "CTRL_IFACE: Detach monitor %s that cannot receive messages\n", dst->daemon_name);
		*wapp_iface_detach(&wapp->infcli_ctrl.daemon_cli_list, &dst->addr, dst->addrlen);
		*/
		sock = wapp_iface_reinit(wapp);
		if (sock < 0) {
			DBGPRINT(RT_DEBUG_ERROR, "Failed to reinitialize wapp interface socket");
		}

	}

	if (_errno == ENOBUFS || _errno == EAGAIN || _errno == EWOULDBLOCK) {
		/*
		 * The socket send buffer could be full. This
		 * may happen if client programs are not
		 * receiving their pending messages. Close and
		 * reopen the socket as a workaround to avoid
		 * getting stuck being unable to send any new
		 * responses.
		 */
		/*sock = wapp_iface_reinit(wapp);
		*if (sock < 0) {
		*	DBGPRINT(RT_DEBUG_ERROR, "Failed to reinitialize wapp interface socket");
		}*/
		if(retry_socket_send(wapp->infcli_ctrl.sock)) {
			/* retry socket send*/
			if (sendto(wapp->infcli_ctrl.sock, buf, buf_len, 0, (struct sockaddr *) &dst->addr, dst->addrlen) >= 0) {
				dst->errors = 0;
				return;
			} else {
				DBGPRINT(RT_DEBUG_ERROR, "wapp_iface sendto re-try failed: %d - %s\n",_errno, strerror(_errno));				
				hex_dump_dbg("Socket send Fail ", (unsigned char *)buf, buf_len);
			}
		} else {
			hex_dump_dbg("Socket send Fail ", (unsigned char *)buf, buf_len);
		}
	}
}

void send_event_dispatch(struct wifi_app* wapp, struct msg* event, size_t event_len, char* dst_daemon)
{
	struct wapp_usr_intf_cli* dst, *dst_n;
	struct client_cmd* cmd, *cmd_n;
	struct _map_cmd_event* map_entry = NULL;

	//DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dl_list_for_each_safe(dst, dst_n, &wapp->infcli_ctrl.daemon_cli_list, struct wapp_usr_intf_cli, list)
	{
		/*if dst_daemon is not NULL, send this event to this specific daemon*/
		if(dst_daemon != NULL)
		{
			if(!os_memcmp(dst_daemon, dst->daemon_name, os_strlen(dst_daemon)))
			{
				wapp_socket_send(wapp, (char *)event, event_len, dst);
				break;
			}
		}

		/*check whether does a daemon register this event*/
		if(is_client_event_registered(event->type, dst->registerd_event_bitmap,
			sizeof(dst->registerd_event_bitmap) / sizeof(dst->registerd_event_bitmap[0])))
		{
			wapp_socket_send(wapp, (char *)event, event_len, dst);
		}
		/*then check whether the content of cmd and event are matched */
		else
		{
			dl_list_for_each_safe(cmd, cmd_n, &dst->cmd_list, struct client_cmd, list)
			{
				map_entry = lookup_mapping_entry(0, cmd->cmd_buf->type);
				if(map_entry != NULL &&  map_entry->match_func != NULL && map_entry->match_func(event, cmd->cmd_buf))
				{
					wapp_socket_send(wapp, (char *)event, event_len, dst);
					DBGPRINT(RT_DEBUG_TRACE, "[%s]delete stored cmd-%p(%d)\n", __func__, cmd, cmd->cmd_buf->type);
					os_free(cmd->cmd_buf);
					dl_list_del(&cmd->list);
					os_free(cmd);
				}
			}
		}
	}
}

void wapp_iface_send(struct wifi_app* wapp, char* buf, size_t buf_len, char *dst_dameon)
{
	send_event_dispatch(wapp, (struct msg*)buf, buf_len, dst_dameon);
}

void wapp_iface_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct wapp_usr_intf_cli_ctrl* cli_ctrl = NULL;
	char buf[3072];
	char* daemon_name = NULL;
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	char *reply = NULL, *reply_buf = NULL;
	size_t reply_len = 0;
	int new_attached = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	cli_ctrl = &wapp->infcli_ctrl;
	if (!cli_ctrl) {
		DBGPRINT(RT_DEBUG_ERROR, "cli_ctrl == NULL");
		return;
	}
	
	memset(buf, 0, sizeof(buf));
	res = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "recvfrom(ctrl_iface): %s", strerror(errno));
		return;
	}
	/*
	hex_dump_dbg("wapp recv cmd:", (unsigned char*)buf, res);
	*/
	if (os_strncmp(buf, "ATTACH:", strlen("ATTACH:")) == 0) {
		daemon_name = strchr(buf, ':');
		if (daemon_name != NULL) {
			daemon_name++;

			if (wapp_iface_attach(&cli_ctrl->daemon_cli_list, &from, fromlen, daemon_name)) {
				reply = "FAIL\n";
				reply_len = 5;
			} else {
				new_attached = 1;
				reply = "OK\n";
				reply_len = 3;
				if (os_strncmp(daemon_name, "bs20", strlen("bs20")) == 0)
					wapp->is_bs20_attached = TRUE;
			}
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "%s: daemon_name is NULL!\n", __func__);
		}
	} else if (os_strncmp(buf, "DETACH", strlen("DETACH")) == 0) {
		if (wapp_iface_detach(&cli_ctrl->daemon_cli_list, &from, fromlen)) {
			reply = "FAIL\n";
			reply_len = 5;
		} else {
			reply = "OK\n";
			reply_len = 3;
			if (wapp->is_bs20_attached)
				wapp->is_bs20_attached = FALSE;
		}
	} else {
		wapp_iface_process(wapp, &from, fromlen, buf, res, &reply_buf, &reply_len);
		reply = reply_buf;
	}

	if (reply_len) {
		if (sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from, fromlen) < 0) {
			int _errno = errno;
			DBGPRINT(RT_DEBUG_ERROR, "ctrl_iface sendto failed: %d - %s\n", _errno, strerror(_errno));

			if (_errno == ENOBUFS || _errno == EAGAIN) {
				/*
				 * The socket send buffer could be full. This
				 * may happen if client programs are not
				 * receiving their pending messages. Close and
				 * reopen the socket as a workaround to avoid
				 * getting stuck being unable to send any new
				 * responses.
				 */
				sock = wapp_iface_reinit(wapp);
				if (sock < 0) {
					DBGPRINT(RT_DEBUG_ERROR, "Failed to reinitialize ctrl_iface socket");
				}
			}
			if (new_attached) {
				DBGPRINT(RT_DEBUG_ERROR, "Failed to send response to ATTACH - detaching");
				new_attached = 0;
				wapp_iface_detach(&cli_ctrl->daemon_cli_list, &from, fromlen);
			}
		}
	}

	if (reply_buf != NULL)
		os_free(reply_buf);
}

int wapp_iface_open_sock(struct wifi_app *wapp, char* socket_patch)
{
	int sock_len = 0;
	char *buf = NULL;
	int flags;
	struct wapp_usr_intf_cli_ctrl* cli_ctrl = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	buf = os_strdup(socket_patch);
	if (buf == NULL)
		goto fail;

	cli_ctrl = &wapp->infcli_ctrl;
	if (!cli_ctrl) {
		DBGPRINT(RT_DEBUG_ERROR, "cli_ctrl == NULL");
		goto fail;
	}
	cli_ctrl->sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (cli_ctrl->sock < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "socket(PF_UNIX): %s", strerror(errno));
		goto fail;
	}

	os_memset(&cli_ctrl->addr, 0, sizeof(cli_ctrl->addr));
	cli_ctrl->addr.sun_family = AF_UNIX;
	snprintf(cli_ctrl->addr.sun_path, sizeof(cli_ctrl->addr.sun_path), "%s", buf);
	sock_len = strlen(buf) + offsetof(struct sockaddr_un, sun_path);
	if (bind(cli_ctrl->sock, (struct sockaddr *) &cli_ctrl->addr, sock_len) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "wapp_ctrl_iface_init (%s) (will try fixup): bind(PF_UNIX): %s",
				cli_ctrl->addr.sun_path, strerror(errno));
		if (connect(cli_ctrl->sock, (struct sockaddr *) &cli_ctrl->addr,
					sock_len) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, "ctrl_iface exists, but does not"
					" allow connections - assuming it was left"
					"over from forced program termination");
			if (unlink(cli_ctrl->addr.sun_path) < 0) {
				DBGPRINT(RT_DEBUG_ERROR,
						"Could not unlink existing ctrl_iface socket '%s': %s",
						cli_ctrl->addr.sun_path, strerror(errno));
				goto fail;
			}
			if (bind(cli_ctrl->sock, (struct sockaddr *) &cli_ctrl->addr,
						sock_len) < 0) {
				DBGPRINT(RT_DEBUG_ERROR, "wapp_ctrl_iface_init: bind(PF_UNIX;%s): %s",
						cli_ctrl->addr.sun_path, strerror(errno));
				goto fail;
			}
			DBGPRINT(RT_DEBUG_ERROR, "Successfully replaced leftover "
					"ctrl_iface socket '%s'",
					cli_ctrl->addr.sun_path);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "ctrl_iface exists and seems to "
					"be in use - cannot override it");
			DBGPRINT(RT_DEBUG_ERROR, "Delete '%s' manually if it is "
					"not used anymore",
					cli_ctrl->addr.sun_path);
			goto fail;
		}
	}

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(cli_ctrl->sock, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(cli_ctrl->sock, F_SETFL, flags) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, "fcntl(ctrl, O_NONBLOCK): %s", strerror(errno));
			/* Not fatal, continue on.*/
		}
	}

	eloop_register_read_sock(cli_ctrl->sock, wapp_iface_receive, wapp, NULL);

	os_free(buf);
	return 0;

fail:
	if (cli_ctrl && cli_ctrl->sock >= 0) {
		close(cli_ctrl->sock);
		cli_ctrl->sock = -1;
	}
	if (buf) {
		os_free(buf);
	}
	return -1;
}


