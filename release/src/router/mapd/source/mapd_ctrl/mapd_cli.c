/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mapd_cli.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include<string.h>
#include "includes.h"
#include "mapd_interface_ctrl.h"
#include "mapd_cli.h"
#include "os.h"
#include "interface.h"
#include "common.h"

static struct mapd_interface_ctrl *ctrl_conn;

static int mapd_cli_open_connection(const char *ctrl_path)
{
	ctrl_conn = mapd_interface_ctrl_open(ctrl_path);

	if (!ctrl_conn) {
		printf("mapd_interface_ctrl_open failed\n");
		return -1;
	}

	return 0;
}

static void mapd_cli_close_connection(void)
{
	mapd_interface_ctrl_close(ctrl_conn);
	ctrl_conn = NULL;
}

static void mapd_cli_msg_cb(char *msg, size_t len)
{
    printf("%s\n", msg);
}

static int _mapd_ctrl_commapd(struct mapd_interface_ctrl *ctrl, const char *cmd, size_t len1)
{
    char buf[4096];
    size_t len;
    int ret;

    if (ctrl_conn == NULL) {
        printf("Not connected to Mand - commapd dropped.\n");
        return -1;
    }
    len = sizeof(buf) - 1;
    ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,
            mapd_cli_msg_cb);
    if (ret == -2) {
        printf("'%s' commapd timed out.\n", cmd);
        return -2;
    } else if (ret < 0) {
        printf("'%s' commapd failed.\n", cmd);
        return -1;
    }
    buf[len] = '\0';
    printf("%s", buf);
    return 0;
}

static int mapd_cli_cmd_get(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[256];
    int res;

    if (argc != 1) {
        printf("Invalid GET commapd: needs one argument (variable "
                "name)\n");
        return -1;
    }

    res = os_snprintf(cmd, sizeof(cmd), "GET %s", argv[0]);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long GET commapd.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_cmd_ping(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;

    res = os_snprintf(cmd, sizeof(cmd), "PING");
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Invalid\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));

}

static int mapd_cli_cmd_mib(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
	char fmt_str[1000]="";
	char *fmt_str_pos, *fmt_str_end;
    int res;
	int i;

	if(argc == 0)
	{
		res = os_snprintf(cmd, sizeof(cmd), "MIB");
    	if (os_snprintf_error(sizeof(cmd), res)) {
        	printf("Invalid\n");
        	return -1;
    	}
		return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
	}
	else
	{	fmt_str_pos = fmt_str;
		fmt_str_end = fmt_str + 1000;

		res = os_snprintf(fmt_str_pos, fmt_str_end - fmt_str_pos, "MIB");
		if (os_snprintf_error((fmt_str_end - fmt_str_pos), res)) {
			printf("Invalid\n");
			return -1;
		}
		fmt_str_pos += res;

		for(i=0;i<argc;++i)
		{
			res = os_snprintf(fmt_str_pos, sizeof(fmt_str), " %s", argv[i]);
			if (os_snprintf_error((fmt_str_end - fmt_str_pos), res)) {
				printf("Invalid\n");
				return -1;
			}
			fmt_str_pos += res;

		}
		return _mapd_ctrl_commapd(ctrl, fmt_str, strlen(fmt_str));
	}		

}
static int mapd_cli_cmd_set(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;

    if (argc != 2) {
        printf("Invalid SET commapd: needs two arguments (variable "
                "name and value, count=%d)\n", argc);
        return -1;
    }

    res = os_snprintf(cmd, sizeof(cmd), "SET %s %s", argv[0], argv[1]);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long SET commapd.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));

}

static int mapd_cli_trigger_str_cand_selection(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;

    res = os_snprintf(cmd, sizeof(cmd), "TRIGGER_STR_CANDS");
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_save_db(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    printf("SAVE CONFIG\n");
    res = os_snprintf(cmd, sizeof(cmd), "SAVE_DB");
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
#ifdef SUPPORT_MULTI_AP
static int mapd_cli_dump_topo(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    printf("dump topology\n");
    res = os_snprintf(cmd, sizeof(cmd), "DUMP_TOPO");
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_dump_topo_v1(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char *reply = NULL;
	reply = os_malloc(15000);
	size_t len = 15000;
	if (reply == NULL)
    	{
        	printf("%s, alloc memory fail\n", __func__);
        	return -1;
    	}
	os_memset(reply, '\0', 14999);
	if(argc == 1) {
		if (mapd_interface_get_topology(ctrl, reply, &len, (char *)(argv[0])) < 0) {
			os_free(reply);
			return -1;
		}
	} else {
		if (mapd_interface_get_topology(ctrl, reply, &len, NULL) < 0) {
			os_free(reply);
			return -1;
		}
	}
	//always("%s", reply);
	os_free(reply);
	return 0;
}

static int mapd_cli_cmd_bh_info(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    printf("Backhaul Info\n");
    res = os_snprintf(cmd, sizeof(cmd), "BH_INFO");
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
#endif /* #ifdef SUPPORT_MULTI_AP */
static int mapd_cli_cmd_sta_info(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    printf("Connected STA Info\n");
    res = os_snprintf(cmd, sizeof(cmd), "STA_MED_INFO");
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
#ifdef SUPPORT_MULTI_AP
static int mapd_cli_cmd_get_conn_status(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	int fhbss_status = 0;
	int bhsta_status = 0;
	if(mapd_interface_get_conn_status(ctrl, &fhbss_status, &bhsta_status) < 0) {
		printf("Error: Getting connection status\n");
		return 0;
	}
	printf("conn_status fhbss: %d, bhsta: %d\n", fhbss_status, bhsta_status);
	return 0 ;
}

static int mapd_cli_cmd_set_rssi_threhold_2g(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
	int thresh;
    printf("Setting Scan RSSI threhold 2.4G\n");
	if (argc != 1)
		return -1;
	thresh = atoi(argv[0]);
    res = os_snprintf(cmd, sizeof(cmd), "scan_thresh_2g %d", thresh);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
static int mapd_cli_cmd_set_rssi_threhold_5g(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
	int thresh;
    printf("Setting Scan RSSI threhold 5G\n");
	if (argc != 1)
		return -1;
	thresh = atoi(argv[0]);
    res = os_snprintf(cmd, sizeof(cmd), "scan_thresh_5g %d", thresh);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_cmd_set_bh_priority(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	if (argc > 3 || argc < 2)
		return -1;
	if (argc == 3)
		mapd_interface_set_bh_priority(ctrl, (unsigned char *)argv[0], (unsigned char *)argv[1], (unsigned char *)argv[2]);
	if (argc == 2)
		mapd_interface_set_bh_priority(ctrl, (unsigned char *)argv[0], (unsigned char *)argv[1], NULL);
	return 0;
}
#endif
static int mapd_cli_cmd_get_version_info(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    printf("Getting Version Info...\n");
    res = os_snprintf(cmd, sizeof(cmd), "version_info");
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

#ifdef SUPPORT_MULTI_AP
static int mapd_cli_set_acl_block(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	unsigned char type = 0;
	char cmd[256];
	char recv_buf[16];
	int res = 0;
	unsigned char block_flag = 0;
	//dur = atoi((const char *)duration);
	unsigned char cli_mac[ETH_ALEN]={0};
	unsigned char bss_mac[ETH_ALEN]={0};
	unsigned char al_mac[ETH_ALEN]={0};
	if(argc < 3)
		return -1;
	type = atoi(argv[0]);
	block_flag = atoi(argv[1]); // if 0 then block, if 1 then unblock
	if(type <0 || type > 2)
		return -1;
	if(block_flag == 0) {
		if (type == 0 && argc != 4) {
			printf("type: %d, argc: %d\n", type, argc);
			return -1;
		} else if ((type == 1 || type == 2) && argc != 5) {
			printf("type: %d, argc: %d\n", type, argc);
			return -1;
		}
		hwaddr_aton(argv[3], cli_mac);
		if (type == 1)
			hwaddr_aton(argv[4], al_mac);
		else if (type == 2)
			hwaddr_aton(argv[4], bss_mac);
	} else if (block_flag == 1) {
		if (type == 0 && argc != 3) {
			printf("type: %d, argc: %d\n", type, argc);
			return -1;
		} else if ((type == 1 || type == 2) && argc != 4) {
			printf("type: %d, argc: %d\n", type, argc);
			return -1;
		}
		hwaddr_aton(argv[2], cli_mac);
		if (type == 1)
			hwaddr_aton(argv[3], al_mac);
		else if (type == 2)
			hwaddr_aton(argv[3], bss_mac);
	}
	os_memset(cmd, 0, sizeof(cmd));
	os_memset(recv_buf, 0, sizeof(recv_buf));
	if(block_flag == 1) {
		if(type == 2)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_block %d %d %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x", type, block_flag, PRINT_MAC(cli_mac), PRINT_MAC(bss_mac));
		else if (type == 1)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_block %d %d %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x", type, block_flag, PRINT_MAC(cli_mac), PRINT_MAC(al_mac));
		else if (type == 0)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_block %d %d %02x:%02x:%02x:%02x:%02x:%02x", type, block_flag, PRINT_MAC(cli_mac));
	} else if (block_flag == 0) {
		if(type == 2)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_block %d %d %s %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x", type, block_flag, argv[2], PRINT_MAC(cli_mac), PRINT_MAC(bss_mac));
		else if (type == 1)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_block %d %d %s %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x", type, block_flag, argv[2], PRINT_MAC(cli_mac), PRINT_MAC(al_mac));
		else if (type == 0)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_block %d %d %s %02x:%02x:%02x:%02x:%02x:%02x", type, block_flag, argv[2], PRINT_MAC(cli_mac));
	}
	printf("%s\n",cmd);
	
	if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_set_enrollee_bh_info(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
//	unsigned char mac_addr[ETH_ALEN] = {0};
	unsigned char type = 0;

//	hwaddr_aton(argv[0], mac_addr);

	type = atoi(argv[1]);

	printf("[%s],if mac %s, type=%d\n", __FUNCTION__, argv[0], type);
	
	if (mapd_interface_set_enrollee_bh_info(ctrl, argv[0], type) < -1)
		return -1;
	
	return 0;
}

static int mapd_cli_set_bss_role(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	unsigned char mac_addr[ETH_ALEN] = {0};
	unsigned char role = 0;

	hwaddr_aton(argv[0], mac_addr);

	hexstr2bin(argv[1], &role, 1);

	printf("[%s],bss mac %02x:%02x:%02x:%02x:%02x:%02x, role=%d\n", __FUNCTION__, PRINT_MAC(mac_addr), role);
	
	if (mapd_interface_set_bss_role(ctrl, mac_addr, role) < -1)
		return -1;
	
	return 0;
}

static int mapd_cli_trigger_wps(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
//	unsigned char mac_addr[ETH_ALEN] = {0};

//	hwaddr_aton(argv[0], mac_addr);

	printf("[%s],bss mac %s\n", __FUNCTION__, argv[0]);
	
	if (mapd_interface_trigger_map_wps(ctrl, argv[0]) < 0)
		return -1;
	
	return 0;
}

static int mapd_cli_test_bh_steer(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char ifname[IFNAMSIZ] = {0};
	char bssid[IFNAMSIZ] = {0};

	os_memcpy(ifname, argv[0], IFNAMSIZ);
	os_memcpy(bssid, argv[1], IFNAMSIZ);

	printf("%s: iface name is %s\n", __func__, ifname);
	printf("%s: bssid is %s\n", __func__, bssid);

	if (mapd_interface_trigger_bh_steer(ctrl, ifname, bssid) < 0)
		return -1;

	return 0;
}

#ifdef ACL_CTRL
static int mapd_cli_get_agent_info(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[2048];
	int res;
	printf("dump agent info\n");
	res = os_snprintf(cmd, sizeof(cmd), "dump_agent_info");
	if (os_snprintf_error(sizeof(cmd), res)) {
		printf("Too long command.\n");
		return -1;
	}
	return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_set_acl_ctrl(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	unsigned char type = 0;
	char cmd[256];
	char recv_buf[16];
	int res = 0;
	unsigned char acl_cmd = 0;
	unsigned char cli_mac[ETH_ALEN]={0};
	unsigned char bss_mac[ETH_ALEN]={0};
	unsigned char al_mac[ETH_ALEN]={0};

	if(argc < 2)
		return -1;

	type = atoi(argv[0]);
	acl_cmd = atoi(argv[1]);

	if(type <0 || type > 2)
		return -1;

	if (acl_cmd < 2) {
		if (type == 0 && argc != 3) {
			printf("type: %d, argc: %d\n", type, argc);
			return -1;
		} else if ((type == 1 || type == 2) && argc != 4) {
			printf("type: %d, argc: %d\n", type, argc);
			return -1;
		}
		hwaddr_aton(argv[2], cli_mac);
		if (type == 1)
			hwaddr_aton(argv[3], al_mac);
		else if (type == 2)
			hwaddr_aton(argv[3], bss_mac);
	} else {
		if (type == 0 && argc != 2) {
			printf("type: %d, argc: %d\n", type, argc);
			return -1;
		} else if ((type == 1 || type == 2) && argc != 3) {
			printf("type: %d, argc: %d\n", type, argc);
			return -1;
		}
		if (type == 1)
			hwaddr_aton(argv[2], al_mac);
		else if (type == 2)
			hwaddr_aton(argv[2], bss_mac);
	}

	os_memset(cmd, 0, sizeof(cmd));
	os_memset(recv_buf, 0, sizeof(recv_buf));

	if(acl_cmd < 2) {
		if(type == 2)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_ctrl %d %d %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x", type, acl_cmd, PRINT_MAC(cli_mac), PRINT_MAC(bss_mac));
		else if (type == 1)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_ctrl %d %d %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x", type, acl_cmd, PRINT_MAC(cli_mac), PRINT_MAC(al_mac));
		else if (type == 0)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_ctrl %d %d %02x:%02x:%02x:%02x:%02x:%02x", type, acl_cmd, PRINT_MAC(cli_mac));
	} else {
		if(type == 2)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_ctrl %d %d %02x:%02x:%02x:%02x:%02x:%02x", type, acl_cmd, PRINT_MAC(bss_mac));
		else if (type == 1)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_ctrl %d %d %02x:%02x:%02x:%02x:%02x:%02x", type, acl_cmd, PRINT_MAC(al_mac));
		else if (type == 0)
			res = os_snprintf(cmd, sizeof(cmd), "set_acl_ctrl %d %d", type, acl_cmd);
	}

	printf("%s\n",cmd);

	if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
#endif /* ACL_CTRL */
#endif /* #ifdef SUPPORT_MULTI_AP */
static int mapd_cli_get_client_db(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	struct client_db *dbs = NULL;
	char buf[2048];
	size_t len = sizeof(buf);
	int client_num = 0, i = 0;
	unsigned char j = 0, arr_idx;
	client_num = mapd_interface_get_client_db(ctrl, (struct client_db*)buf, &len);
	dbs = (struct client_db*)buf;
	
	if (client_num < 0)
		return -1;

	for (i = 0; i < client_num; i++) {
		printf("client[%d]:\n"
			   "\tmac:      %02x:%02x:%02x:%02x:%02x:%02x\n"
			   "\tbssid:    %02x:%02x:%02x:%02x:%02x:%02x\n"
			   "\tcapab:    %d\n"
			   "\tphy_mode: %02x\n"
			   "\tmax_bw_0: %02x\n"
			   "\tmax_bw_1: %02x\n"
			   "\tsp_stream:%d\n"
			   "\tknown_band:%02x\n"
			   "\tknown_channel:%02x %02x %02x %02x %02x\n"
			   "\treal_channels:",
			   i, PRINT_MAC(dbs[i].mac), PRINT_MAC(dbs[i].bssid),
			   dbs[i].capab, dbs[i].phy_mode, dbs[i].max_bw[0],
			   dbs[i].max_bw[1], dbs[i].spatial_stream, dbs[i].know_band,
			   dbs[i].know_channels[0], dbs[i].know_channels[1], dbs[i].know_channels[2],
			   dbs[i].know_channels[3], dbs[i].know_channels[4]);
		for (j = 0; j < 38; j++) {
			arr_idx = j / 8;
			if (dbs[i].know_channels[arr_idx] & BIT(j % 8)) {
				printf("%d ", idx_to_chan(j));
			}
		}
		printf("\n---------------------------------------------\n");
		
	}
	
	return 0;
}
#ifdef SUPPORT_MULTI_AP
static int mapd_set_radar(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[32] = {"radar"};
	char response_buf[16] = {0};;
	char response_len = sizeof(response_buf);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), response_buf, (size_t *)&response_len, NULL) < 0)
		return -1;
	return 0;
}
static int mapd_cli_restart_ch_planning(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[32] = {"restart_ch_planning"};
	char response_buf[16] = {0};
	char response_len = sizeof(response_buf);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), response_buf, (size_t *)&response_len, NULL) < 0)
		return -1;
	return 0;
}
static int mapd_cli_enable_channel_planning
	(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char *cmd;
	char response_buf[16] = {0};
	char response_len = sizeof(response_buf);

	if (argc && *argv[0] == '1')
		cmd = "ENABLE_CHANNEL_PLANNING";
	else
		cmd = "DISABLE_CHANNEL_PLANNING";
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), response_buf, (size_t *)&response_len, NULL) < 0)
		return -1;
	return 0;
}
static int mapd_cli_get_scan_results(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    printf("get scan results\n");
    res = os_snprintf(cmd, sizeof(cmd), "GET_SCAN_RESULTS");
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_link_metrics_stub (struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[2048] = {0};
	int buf_len, idx, idx2, num_sta;
	char *token;
	struct msg *tlv_msg;
	struct sta_link_metrics *sta_metrics;

	num_sta = atoi(argv[1]);

	buf_len = sizeof(struct sta_link_metrics) + num_sta * sizeof(struct link_metrics);
	tlv_msg = (struct msg *) os_zalloc(sizeof(struct msg) + buf_len);
	if(tlv_msg == NULL){
		printf("memory alloc fail\n");
		return -1;
	}
		tlv_msg->type = WAPP_ALL_ASSOC_STA_LINK_METRICS;
	tlv_msg->length = buf_len;
	sta_metrics = (struct sta_link_metrics *)tlv_msg->buffer;
	for (idx2 = 0; idx2<6; ++idx2)
	{
		token = strsep(&argv[0], ":");
		sta_metrics->identifier[idx2]=(unsigned char)strtoul(token, NULL, 16);
	}

	sta_metrics->sta_cnt = num_sta;

	for(idx = 0; idx < num_sta *5; idx +=5)
	{
		for (idx2 = 0; idx2<6; ++idx2)
		{
			token = strsep(&argv[idx+2], ":");
			sta_metrics->info[idx/3].mac[idx2]=(unsigned char)strtoul(token, NULL, 16);
		}
		for (idx2 = 0; idx2<6; ++idx2)
		{
			token = strsep(&argv[idx+3], ":");
			sta_metrics->info[idx/3].bssid[idx2]=(unsigned char)strtoul(token, NULL, 16);
		}
		sta_metrics->info[idx/3].erate_downlink = atoi(argv[idx + 4]);
		sta_metrics->info[idx/3].erate_uplink = atoi(argv[idx + 5]);
		sta_metrics->info[idx/3].rssi_uplink = atoi(argv[idx + 6]);
	}
	os_snprintf(cmd, sizeof(cmd), "STUB ");
	os_memcpy(cmd+5, (char *)tlv_msg, buf_len + sizeof(struct msg));
	os_free(tlv_msg);
	return _mapd_ctrl_commapd(ctrl, cmd, 5+buf_len+4);



}
#endif /* #ifdef SUPPORT_MULTI_AP */

static int mapd_cli_sta_traffic_stats_stub (struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[2048] = {0};
	int buf_len, idx, idx2, num_sta;
	char *token;
	struct msg *tlv_msg;
	struct sta_traffic_stats *sta_stats;

	num_sta = atoi(argv[1]);

	buf_len = sizeof(struct sta_traffic_stats) + num_sta * sizeof(struct stat_info);
	tlv_msg = (struct msg *) os_zalloc(sizeof(struct msg) + buf_len);
	if(tlv_msg == NULL){
		printf("memory alloc fail\n");
		return -1;
	}
		tlv_msg->type = WAPP_ALL_ASSOC_STA_TRAFFIC_STATS;
	tlv_msg->length = buf_len;
	sta_stats = (struct sta_traffic_stats *)tlv_msg->buffer;
	for (idx2 = 0; idx2<6; ++idx2)
	{
		token = strsep(&argv[0], ":");
		sta_stats->identifier[idx2]=(unsigned char)strtoul(token, NULL, 16);
	}

	sta_stats->sta_cnt = num_sta;
	printf("Reach here\n");

	for(idx = 0; idx < num_sta *3; idx +=3)
	{
		for (idx2 = 0; idx2<6; ++idx2)
		{
			token = strsep(&argv[idx+2], ":");
			sta_stats->stats[idx/3].mac[idx2]=(unsigned char)strtoul(token, NULL, 16);
		}
		sta_stats->stats[idx/3].bytes_sent = atoi(argv[idx + 3]);
		sta_stats->stats[idx/3].bytes_received = atoi(argv[idx + 4]);
	}
	os_snprintf(cmd, sizeof(cmd), "STUB ");
	os_memcpy(cmd+5, (char *)tlv_msg, buf_len + sizeof(struct msg));
	os_free(tlv_msg);
	return _mapd_ctrl_commapd(ctrl, cmd, 5+buf_len+4);



}


static int mapd_cli_client_assoc_stub(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{

	char cmd[2048] = {0};
	int buf_len, idx2;
	char *token;
	struct client_association_event_local *evt = NULL;
	struct map_client_association_event_local *pevt = NULL;
	struct msg *tlv_msg;
	
	buf_len = sizeof(struct client_association_event_local);
	tlv_msg = (struct msg *) os_zalloc(sizeof(struct msg) + buf_len);
	if(tlv_msg == NULL){
		printf("memory alloc fail\n");
		return -1;
	}
	tlv_msg->type = WAPP_CLIENT_NOTIFICATION;
	tlv_msg->length = buf_len;
	evt = (struct client_association_event_local *)tlv_msg->buffer;
	pevt = (struct map_client_association_event_local *)&evt;

	for (idx2 = 0; idx2<6; ++idx2)
	{
		token = strsep(&argv[0], ":");
		pevt->sta_mac[idx2]=(unsigned char)strtoul(token, NULL, 16);
	}

	for (idx2 = 0; idx2<6; ++idx2)
	{
		token = strsep(&argv[1], ":");
		pevt->bssid[idx2]=(unsigned char)strtoul(token, NULL, 16);
	}

	pevt->assoc_evt = atoi(argv[2]);
	os_snprintf(cmd, sizeof(cmd), "STUB ");
    os_memcpy(cmd+5, (char *)tlv_msg, buf_len + sizeof(struct msg));
	os_free(tlv_msg);
    return _mapd_ctrl_commapd(ctrl, cmd, 5+buf_len+4);



}


static int mapd_cli_oper_bss_stub(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[2048] = {0};
	int buf_len, idx, idx2, num_bss;
	char *token;
	struct oper_bss_cap *bss_cap;
	struct msg *tlv_msg;

	// add basic sanity here for checking cli command invokation syntax

	num_bss = atoi(argv[1]);

	buf_len = sizeof(struct oper_bss_cap) + num_bss * sizeof(struct op_bss_cap);
	tlv_msg = (struct msg *) os_zalloc(sizeof(struct msg) + buf_len);
	if(tlv_msg == NULL){
		printf("memory alloc fail\n");
		return -1;
	}
	tlv_msg->type = WAPP_OPERBSS_REPORT;
	tlv_msg->length = buf_len;
	bss_cap = (struct oper_bss_cap *)tlv_msg->buffer;
	for (idx2 = 0; idx2<6; ++idx2)
	{
		token = strsep(&argv[0], ":");
		bss_cap->identifier[idx2]=(unsigned char)strtoul(token, NULL, 16);
	}

	bss_cap->oper_bss_num = num_bss;
	bss_cap->band = atoi(argv[2]);

	for (idx = 3; idx <= num_bss*3; idx += 3)
	{
		for (idx2 = 0; idx2<6; ++idx2)
		{
			token = strsep(&argv[idx], ":");
			bss_cap->cap[(idx-3)/3].bssid[idx2]=(unsigned char)strtoul(token, NULL, 16);
		}

		bss_cap->cap[idx/3-1].ssid_len = atoi(argv[idx+1]);
		os_memcpy(bss_cap->cap[idx/3-1].ssid, argv[idx+2], atoi(argv[idx+1]));
	}
	os_snprintf(cmd, sizeof(cmd), "STUB ");
	os_memcpy(cmd+5, (char *)tlv_msg, buf_len + sizeof(struct msg));
	os_free(tlv_msg);
    return _mapd_ctrl_commapd(ctrl, cmd, 5+buf_len+4);

}

static int mapd_cli_op_chan_stub(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])

{
	char cmd[2048] = {0};
	int buf_len, idx, idx2, num_rad;
	char *token;
	struct channel_report *chan_rep;
	struct msg *tlv_msg;

	if(argc % 4 != 1)
	{
		printf("Incorrect argument format, check the input args\n");
		return -1;
	}

	num_rad = atoi(argv[0]);
	
	buf_len = sizeof(struct channel_report) + num_rad * sizeof(struct ch_rep_info);
	tlv_msg = (struct msg *)os_zalloc(sizeof(struct msg) + buf_len);
	if(tlv_msg == NULL){
		printf("memory alloc fail\n");
		return -1;
	}
	tlv_msg->type = WAPP_OPERATING_CHANNEL_INFO;
	tlv_msg->length = buf_len;
	chan_rep = (struct channel_report *)tlv_msg->buffer;
	chan_rep->ch_rep_num = num_rad;
	for (idx = 0; idx < num_rad*4; idx += 4)
	{
		for (idx2 = 0; idx2<6; ++idx2)
		{
			token = strsep(&argv[idx+1], ":");
			chan_rep->info[idx/4].identifier[idx2]=(unsigned char)strtoul(token, NULL, 16);
		}

		chan_rep->info[idx/4].op_class = atoi(argv[idx+2]);
		chan_rep->info[idx/4].channel = atoi(argv[idx+3]);
		chan_rep->info[idx/4].tx_power = atoi(argv[idx+4]);
	}

	os_snprintf(cmd, sizeof(cmd), "STUB ");
	os_memcpy(cmd+5, (char *)tlv_msg, buf_len + sizeof(struct msg));
	os_free(tlv_msg);
    return _mapd_ctrl_commapd(ctrl, cmd, 5+buf_len+4);

}
#ifdef SUPPORT_MULTI_AP
static int mapd_cli_cmd_get_role(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	 int dev_role=0;
	 mapd_interface_get_role(ctrl, &dev_role);
	 printf("dev_role %d\n", dev_role);
	 return 0 ;
}
static int mapd_cli_cmd_bestAP(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_select_best_ap(ctrl);
	return 0;
}

static int mapd_cli_cmd_set_rssith(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_set_rssi_thresh(ctrl,argv[0]);
	return 0 ;
}

static int mapd_cli_cmd_set_ChUtil_th(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_set_ChUtil_thresh(ctrl,argv[0],argv[1],argv[2]);
	return 0 ;
}

static int mapd_cli_cmd_mandate_steer(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_mandate_steer(ctrl, argv[0], argv[1]);
	return 0 ;
}
static int mapd_cli_cmd_bhsteer(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_bh_steer(ctrl, argv[0], argv[1]);
	return 0 ;
}
#ifdef MAP_R2
static int mapd_cli_cmd_send_cac_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[50];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "send_cac_req_ter %d %d %d %d %d %d", atoi(argv[0]), atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	if (os_snprintf_error(sizeof(cmd), res)) {
		printf("Too long command.\n");
		return -1;
	}
	return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
static int mapd_cli_cmd_send_ch_sel_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[50];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "ch_sel_req %d %d", atoi(argv[0]), atoi(argv[1]));
	if (os_snprintf_error(sizeof(cmd), res)) {
		printf("Too long command.\n");
		return -1;
	}
	return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_cmd_get_de_dump_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	//char cmd[50];
	//int res;
	char *reply = NULL;
	reply = os_malloc(5000);
	size_t len = 5000;
	if (reply == NULL) {
    	printf("%s, alloc memory fail\n", __func__);
    	return -1;
	}
	os_memset(reply, '\0', len);
	if (argc > 1) {
		os_free(reply);
		return -1;
	}
	if(argc == 1) {
		if (mapd_interface_get_de_dump(ctrl, reply, &len, (char *)(argv[0])) < 0) {
			os_free(reply);
			return -1;
		}
	} else {
		if (mapd_interface_get_de_dump(ctrl, reply, &len, NULL) < 0) {
			os_free(reply);
			return -1;
		}
	}
	//res = os_snprintf(cmd, sizeof(cmd), "get_de_stats %s", argv[0]);
	//printf("DE CMD: %s\n", cmd);
	//if (os_snprintf_error(sizeof(cmd), res)) {
		//printf("Too long command.\n");
		//return -1;
	//}
	os_free(reply);
	return 0;
	//return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
static int mapd_cli_cmd_get_de_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	if (argc != 1)
		return -1;
	if (mapd_interface_trigger_de_dump(ctrl, (char *)(argv[0])) < 0) {
		return -1;
	}
	return 0;
}

static int mapd_cli_cmd_trigger_ch_scan_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	if (argc != 1)
		return -1;

	if (mapd_interface_trigger_ch_scan(ctrl, (char *)(argv[0])) < 0) {
		return -1;
	}
	return 0;
}
static int mapd_cli_cmd_trigger_ch_plan_R2_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	if (argc != 1)
		return -1;

	if (mapd_interface_trigger_ch_plan_R2(ctrl, (char *)(argv[0])) < 0) {
		return -1;
	}
	return 0;
}


static int mapd_cli_cmd_get_ch_plan_score_dump_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char *reply = NULL;
	reply = os_malloc(5000);
	size_t len = 5000;
	if (reply == NULL) {
		printf("%s, alloc memory fail\n", __func__);
		return -1;
	}
	os_memset(reply, '\0', len);
	if (argc > 1) {
		os_free(reply);
		return -1;
	}
	if(argc == 1) {
		if (mapd_interface_get_ch_score_dump(ctrl, reply, &len, (char *)(argv[0])) < 0) {
			os_free(reply);
			return -1;
		}
	} else {
		if (mapd_interface_get_ch_score_dump(ctrl, reply, &len, NULL) < 0) {
			os_free(reply);
			return -1;
		}
	}
	
	os_free(reply);
	return 0;
	//return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}


static int mapd_cli_cmd_get_ch_scan_dump_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	//char cmd[50];
	//int res;
	char *reply = NULL;
	reply = os_malloc(5000);
	size_t len = 5000;
	if (reply == NULL) {
    	printf("%s, alloc memory fail\n", __func__);
    	return -1;
	}
	os_memset(reply, '\0', len);
	if (argc > 1) {
		os_free(reply);
		return -1;
	}
	if(argc == 1) {
		if (mapd_interface_get_ch_scan_dump(ctrl, reply, &len, (char *)(argv[0])) < 0) {
			os_free(reply);
			return -1;
		}
	} else {
		if (mapd_interface_get_ch_scan_dump(ctrl, reply, &len, NULL) < 0) {
			os_free(reply);
			return -1;
		}
	}
	//res = os_snprintf(cmd, sizeof(cmd), "get_de_stats %s", argv[0]);
	//printf("DE CMD: %s\n", cmd);
	//if (os_snprintf_error(sizeof(cmd), res)) {
		//printf("Too long command.\n");
		//return -1;
	//}
	os_free(reply);
	return 0;
	//return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}


static int mapd_cli_cmd_send_metric_msg(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[50];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "METRIC_MSG %d %d %d", atoi(argv[0]), atoi(argv[1]), atoi(argv[2]));
	if (os_snprintf_error(sizeof(cmd), res)) {
		printf("Too long command.\n");
		return -1;
	}
	return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
static int mapd_cli_cmd_send_bh_sta_query(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[50];
	int res;
	if (argc != 1)
		return -1;
	res = os_snprintf(cmd, sizeof(cmd), "send_bh_sta_query %s", (char *)argv[0]);
	if (os_snprintf_error(sizeof(cmd), res)) {
		printf("Too long command.\n");
		return -1;
	}
	return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
#endif
static int mapd_cli_cmd_bhconnstatus(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char bh_conn_status=0;
	mapd_interface_bh_ConnectionStatus(ctrl, &bh_conn_status);
	printf("backhaul connection status %d\n", bh_conn_status);
	return 0 ;
}

static int mapd_cli_cmd_bhconntype(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{

	int fhbss_status = 0;
	int bhsta_status = 0;
	char bh_conn_type=0;

	if(mapd_interface_get_conn_status(ctrl, &fhbss_status, &bhsta_status) < 0)
	{
		printf("Error: Getting connection status\n");
		return 0;
	}
	printf("conn_status fhbss: %d, bhsta: %d\n", fhbss_status, bhsta_status);

	if (bhsta_status != STATUS_BHSTA_WPS_NORMAL_CONFIGURED)
		mapd_interface_bh_ConnectionType(ctrl, &bh_conn_type);
	printf("backhaul connection Type %d\n", bh_conn_type);
	return 0 ;
}

#endif
static int mapd_cli_cmd_SetSteer(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_Set_Steer(ctrl, argv[0]);
	return 0 ;
}
#ifdef SUPPORT_MULTI_AP
static int mapd_cli_cmd_onboarding(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_trigger_onboarding(ctrl, argv[0]);
	return 0 ;
}
static int mapd_cli_cmd_config_renew(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	 mapd_interface_config_renew(ctrl);
	 printf("return out of mapdcli config renew \n");
	 return 0 ;
}

static int mapd_cli_cmd_get_bh_ap(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char if_mac[900]= {0};
	mapd_interface_get_bh_ap(ctrl,if_mac);
	printf("mapdcli bh ap list  %s \n", if_mac);
	return 0;
}
static int mapd_cli_cmd_get_fh_ap(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	 char if_mac[900]= {0};
	 mapd_interface_get_fh_ap(ctrl, if_mac);
	 printf("mapdcli fh ap list %s \n", if_mac);
	 return 0 ;
}

static int mapd_cli_renew(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	int ret = 0;
	ret = mapd_interface_set_renew(ctrl);
	return ret;
}

static int mapd_cli_cmd_forceChSwitch(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char channel[]="0";
	if (argc <2)
		return -1;
	else if (argc==2)
		mapd_interface_forceChSwitch(ctrl, argv[0], argv[1], channel, channel);
	else if (argc==3)
		mapd_interface_forceChSwitch(ctrl, argv[0], argv[1], argv[2], channel);
	else if (argc==4)
		mapd_interface_forceChSwitch(ctrl, argv[0], argv[1], argv[2], argv[3]);
	return 0 ;
}

static int mapd_cli_cmd_set_txpower_percentage(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_set_txpower_percentage(ctrl, argv[0], argv[1], argv[2]);
	return 0 ;
}

static int mapd_cli_cmd_off_ch_scan_req(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	if (argc < 2) {
		printf("Min 2 args required\n");
		return -1;
	}
	mapd_interface_off_ch_scan_req(ctrl, argc, argv);
	return 0 ;
}

static int mapd_cli_cmd_off_ch_scan_req_noise(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	if (argc < 2) {
		printf("Min 2 args required\n");
		return -1;
	}
	mapd_interface_off_ch_scan_req_noise(ctrl, argc, argv);
	return 0 ;
}

static int mapd_cli_cmd_off_ch_scan_result(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_off_ch_scan_result(ctrl, argv[0]);
	return 0 ;
}
#endif
static int mapd_cli_cmd_set_bl_timeout(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[2048];
    int res;
	if (argc != 1)
		return -1;
	res = os_snprintf(cmd, sizeof(cmd), "SET_BL_TIMEOUT %s", argv[0]);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long mapd_cli_cmd_set_bl_timeout cmd.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_cmd_reset_csbc(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;

    if (argc != 1) {
        printf("Invalid reset_csbc command: needs one argument");
        return -1;
    }

    res = os_snprintf(cmd, sizeof(cmd), "RESET_CSBC %s", argv[0]);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long RESET_CSBC cmd.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));

	return 0 ;
}
#ifdef SUPPORT_MULTI_AP
static int mapd_cli_metric_policy_param_set(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[50];
	int res;
	if (argc != 3) {
	    printf("Invalid SET commapd: needs three arguments <radio_band> "
	            "<parameter ID> <value> count=%d)\n", argc);
	    return -1;
	}
	res = os_snprintf(cmd, sizeof(cmd), "metric_policy_set %s %s %s", argv[0], argv[1], argv[2]);
	if (os_snprintf_error(sizeof(cmd), res)) {
	    printf("Too long SET commapd.\n");
	    return -1;
	}
	return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}
#endif

#ifdef SUPPORT_MULTI_AP
static int mapd_cli_cmd_tx_higher_layer_data(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	mapd_interface_tx_higher_layer_data(ctrl, argv[0], argv[1], argv[2], argv[3]);
	return 0;
}
#endif

static int mapd_cli_cmd_set_bh_switch_cu_en(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    int thresh;
    printf("Setting bh switch by cu ol enable\n");
    if (argc != 1)
        return -1;
    thresh = atoi(argv[0]);
    res = os_snprintf(cmd, sizeof(cmd), "bh_switch_cu_en %d", thresh);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long enable command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_cmd_set_cu_ol_count_threhold(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    int thresh;
    printf("Setting cu ol count threhold\n");
    if (argc != 1)
        return -1;
    thresh = atoi(argv[0]);
    res = os_snprintf(cmd, sizeof(cmd), "cu_ol_count_thresh %d", thresh);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long set command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static int mapd_cli_cmd_set_bh_ol_time(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
    char cmd[2048];
    int res;
    int thresh;
    printf("Setting bh switch by cu ol forbid time\n");
    if (argc != 1)
        return -1;
    thresh = atoi(argv[0]);
    res = os_snprintf(cmd, sizeof(cmd), "bh_ol_forbid_time %d", thresh);
    if (os_snprintf_error(sizeof(cmd), res)) {
        printf("Too long set command.\n");
        return -1;
    }
    return _mapd_ctrl_commapd(ctrl, cmd, strlen(cmd));
}

static struct mapd_cli_cmd mapd_cli_cmds[] = {
	{"get [param]", mapd_cli_cmd_get, "get mapd parameter"},
	{"set [param] [value]", mapd_cli_cmd_set, "set mapd parameter"},
	{"ping", mapd_cli_cmd_ping, "ping mapd"},
	{"mib", mapd_cli_cmd_mib, "mib"},
	{"op_chan_info_stub", mapd_cli_op_chan_stub, "send a operating channel info event to mapd"},
	{"oper_bss_stub", mapd_cli_oper_bss_stub, "send an operational bss report event to mapd"},
	{"cli_assoc_evt_stub", mapd_cli_client_assoc_stub, "send an association/dissociation event to mapd"},
	{"sta_traffic_stats_stub", mapd_cli_sta_traffic_stats_stub, "send an all assoc sta traffic stats event to mapd"},
#ifdef SUPPORT_MULTI_AP
	{"sta_link_metrics_stub", mapd_cli_link_metrics_stub, "send an all assoc sta link event to mapd"},
#endif
	{"trigger_str_cand_selection", mapd_cli_trigger_str_cand_selection, "Trigger steering candidate selection"},
	{"save_db", mapd_cli_save_db, "Save clientDB to Persistent file"},
#ifdef SUPPORT_MULTI_AP
	{"dump_topology", mapd_cli_dump_topo, "dump all topology info"},
	{"scan", mapd_cli_get_scan_results, "issue a scan"},
	{"dump_topology_v1", mapd_cli_dump_topo_v1, "dump all topology info v1"},
	{"set_enrollee_bh_info", mapd_cli_set_enrollee_bh_info, "set_enrollee_bh_info"},
	{"set_bss_role", mapd_cli_set_bss_role, "mapd_cli_set_bss_role"},
	{"trigger_map_wps", mapd_cli_trigger_wps, "mapd_cli_trigger_wps"},
#endif
	{"get_client_db", mapd_cli_get_client_db, "mapd_cli_get_client_db"},
#ifdef SUPPORT_MULTI_AP
	{"getrole", mapd_cli_cmd_get_role, "get device role"},
	{"select best AP", mapd_cli_cmd_bestAP, "trigger best AP selection"},
	{"set_rssithreshold", mapd_cli_cmd_set_rssith, "set rssi threshold"},
	{"steermand", mapd_cli_cmd_mandate_steer, "Trigger Mandate steering on agent STA_MAC TARGET BSSID"},
	{"steerbh", mapd_cli_cmd_bhsteer, "Trigger Backhaul steering on agent BH_MAC TARGET BSSID"},
	{"test_bh_steer", mapd_cli_test_bh_steer, "testing for ap selection code"},
	{"bh_conn_status",mapd_cli_cmd_bhconnstatus, "GET Backhaul Connection status"},
	{"bh_conn_type",mapd_cli_cmd_bhconntype, "GET Backhaul Connection type"},
#endif
	{"setsteer",mapd_cli_cmd_SetSteer, "Set Steering Enable 1 /Disable 0"},
#ifdef SUPPORT_MULTI_AP
	{"set_ch_util_threshold",mapd_cli_cmd_set_ChUtil_th, "Set channel utilization threshold for 2G, 5GL, 5GH in percentage(0 to 100)"},
	{"onboarding",mapd_cli_cmd_onboarding, "Do ethernet(0) or wifi(1) onboarding"},
	{"restart_ch_planning", mapd_cli_restart_ch_planning, "mapd_restart_ch_planning"},
	{"enable_ch_planning", mapd_cli_enable_channel_planning, "mapd_cli_enable_channel_planning"},
	{"radar", mapd_set_radar, "radar"},
	{"config_renew", mapd_cli_cmd_config_renew, "Issue Config Renew command from controller on all bands "},
	{"renew", mapd_cli_renew, "renew"},
	{"get_bh_ap", mapd_cli_cmd_get_bh_ap, "get MAC addresses of all Backhaul AP interfaces "},
	{"get_fh_ap", mapd_cli_cmd_get_fh_ap, "get MAC addresses of all Fronthaul AP interfaces "},
	{"bh_info", mapd_cli_cmd_bh_info, "Backhaul connection info"},
	{"block_acl", mapd_cli_set_acl_block, "Blocking client"},
#ifdef ACL_CTRL
	{"acl_ctrl", mapd_cli_set_acl_ctrl, "ACL list ctrl"},
	{"agent_list", mapd_cli_get_agent_info, "All dev info"},
#endif /*ACL_CTRL*/
#endif
	{"sta_med_info", mapd_cli_cmd_sta_info, "Connected STA info"},
#ifdef SUPPORT_MULTI_AP
	{"conn_status", mapd_cli_cmd_get_conn_status, "Connection Status"},
	{"scan_thresh_2g", mapd_cli_cmd_set_rssi_threhold_2g, "Rssi Threshold 2g"},
	{"scan_thresh_5g", mapd_cli_cmd_set_rssi_threhold_5g, "Rssi Threshold 5g"},
	{"forceChSwitch", mapd_cli_cmd_forceChSwitch, "Ignore channel planning forceChSwitch <ALMAC> <channel>"},
	{"txpower_percentage", mapd_cli_cmd_set_txpower_percentage, "txpower_percentage <ALMAC> <BandIdx> <TxPower percent>"},
#endif
	{"version_info", mapd_cli_cmd_get_version_info, "Version Information"},
#ifdef SUPPORT_MULTI_AP
	{"bh_priority", mapd_cli_cmd_set_bh_priority, "Backhaul Priority"},
	{"off_ch_scan_req", mapd_cli_cmd_off_ch_scan_req, "Off Channel Scan Request"},
	{"off_ch_scan_result", mapd_cli_cmd_off_ch_scan_result, "Off Channel Scan Result"},
#endif
	{"reset_csbc", mapd_cli_cmd_reset_csbc, "reset_csbc <client_id>"},
	{"bh_switch_cu_en", mapd_cli_cmd_set_bh_switch_cu_en, "set bh switch by cu ol enable"},
	{"cu_ol_count_thresh", mapd_cli_cmd_set_cu_ol_count_threhold, "ch util ol count Threshold"},
	{"bh_ol_forbid_time", mapd_cli_cmd_set_bh_ol_time, "bh switch by cu ol forbid time"},
#ifdef SUPPORT_MULTI_AP
	{"bl_timeout", mapd_cli_cmd_set_bl_timeout, "cmd to set block listing timeout for APs"},
	{"metric_policy_set", mapd_cli_metric_policy_param_set, "Set metric policy parameter format: metric_policy_set <radio band 0/1/2> <parameter ID 0 to 4> <value>"},
	{"noise_floor_cal", mapd_cli_cmd_off_ch_scan_req_noise, "Noise floor calculation"},
#ifdef MAP_R2	
	{"send_metric_msg", mapd_cli_cmd_send_metric_msg, "send_metric_msg"},
	{"send_cac_req_ter", mapd_cli_cmd_send_cac_msg, "send_metric_msg"},
	{"send_ch_sel_req", mapd_cli_cmd_send_ch_sel_msg, "send_metric_msg"},
	{"get_de_stats", mapd_cli_cmd_get_de_msg, "Retrieving DE stats"},
	{"get_de_stats_dump", mapd_cli_cmd_get_de_dump_msg, "Retrieving DE stats"},
	{"ch_scan_trigger", mapd_cli_cmd_trigger_ch_scan_msg, "Triggering Channel Scan"},
	{"ch_scan_res_dump", mapd_cli_cmd_get_ch_scan_dump_msg, "Retrieving Channel Scan Results"},
	{"ch_plan_R2", mapd_cli_cmd_trigger_ch_plan_R2_msg, "Triggering Channel plan R2"},
	{"ch_plan_score_dump", mapd_cli_cmd_get_ch_plan_score_dump_msg, "Retrieving Channel Planning Results"},
	{"send_bh_sta_query", mapd_cli_cmd_send_bh_sta_query, "Retrieving BH STA infos"},
#endif
	{"tx_higher_layer_data", mapd_cli_cmd_tx_higher_layer_data, "TX Higher Layer Data"}
#else
	{"bl_timeout", mapd_cli_cmd_set_bl_timeout, "cmd to set block listing timeout for APs"}
#endif
};

static int mapd_cli_cmd_help(int argc, char *argv[])
{
	struct mapd_cli_cmd *cmd;
	int cmd_num = sizeof(mapd_cli_cmds) / sizeof(struct mapd_cli_cmd);
	int i;

	cmd = mapd_cli_cmds;
	printf("mapd_cli [Path to mapd sockfd] [cmd] [args]\n");
	printf("Commapd Usage:\n");

	for (i = 0; i < cmd_num; i++, cmd++) {
			printf("  %-60s %-50s\n", cmd->cmd, cmd->usage);
	}

	return 0;
}

static int mapd_cli_request(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	struct mapd_cli_cmd *cmd, *match = NULL;
	int ret = 0;

	cmd = mapd_cli_cmds;
	//assert(argc >=1);

	while (cmd && cmd->cmd) {
		if (os_strncmp(cmd->cmd, argv[0], os_strlen(argv[0])) == 0) {
			match = cmd;
			break;
		}
		cmd++;
	}

	if (match) {
        if(argc > 1)
            ret = match->cmd_handler(ctrl, argc - 1, &argv[1]);
        else
            ret = match->cmd_handler(ctrl, 0, NULL);
	} else {
		printf("Unknown command\n");
		ret = -1;
	}

	return ret;
}

int optind = 1;
int main(int argc, char *argv[])
{
	int ret = 0;
	char socket_path[64]={0};

    if (argc < 2) {
        mapd_cli_cmd_help(argc, &argv[0]);
        return -1;
    }

    printf("count =%d 1=%s 2=%s\n", argc, argv[0], argv[1]);
	os_snprintf(socket_path,sizeof(socket_path),"%s",argv[1]);

	if(mapd_cli_open_connection(socket_path) < 0) {
        printf("Can't connect to %s\n", socket_path);
	    return -1;
    }
    printf("Succesfully opened connection to mapd\n");

	ret = mapd_cli_request(ctrl_conn, argc - 2, &argv[2]);

	if (ret < 0)
		printf("command FAIL!!!\n");
	
	mapd_cli_close_connection();

	return ret;
}
