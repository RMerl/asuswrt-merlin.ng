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

#include "wapp_ctrl.h"
#include "wapp_cli.h"

struct wapp_ctrl *ctrl_conn;

int RTDebugLevel = RT_DEBUG_ERROR;

static int wapp_cli_open_connection(const char *ctrl_path)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ctrl_conn = wapp_ctrl_open(ctrl_path);

	if (!ctrl_conn) {
		DBGPRINT(RT_DEBUG_ERROR, "wapp_ctrl_open fail\n");
		return -1;
	}

	return 0;
}

static void wapp_cli_close_connection(void)
{
	wapp_ctrl_close(ctrl_conn);
	ctrl_conn = NULL;
}

int _wapp_ctrl_command(struct wapp_ctrl *ctrl, const char *cmd, 
													char *rsp, size_t *rsp_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (ctrl_conn == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "Connect to hotspot daemon first\n");
		return -1;
	}

	ret = wapp_ctrl_command(ctrl, cmd, os_strlen(cmd), rsp, rsp_len);

	if (ret == -2) {
		DBGPRINT(RT_DEBUG_ERROR, "Timeout\n");
		return -2;
	} else if (ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Command fail\n");
		return -1;
	}

	return 0;
}

static int wapp_cli_cmd_help(struct wapp_ctrl *ctrl, int argc, char *argv[]);

static int wapp_cli_cmd_version(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=hs_version\n");

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);

	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);
	
	return ret;
}

static int wapp_cli_cmd_on(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=on\n");

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int wapp_cli_cmd_off(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret = 0;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=off\n");

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);

	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);
	
	return ret;
}

static struct wapp_cli_get_param w_cli_get_params[] = {
	{"all", "All Parameters"},
};

static struct wapp_cli_set_param w_cli_set_params[] = {
	{"interface", "Interface name", STRING_TYPE},
	{"interworking", "Interworing (Enabled(1) / Disabled(0))", INTEGER_TYPE},
	{"access_network_type", "Hotspot Access Network Type", STRING_TYPE},
	{"internet", "Hotspot Internet Support (Enabled(1)/ Disabled(0))", INTEGER_TYPE},
	{"venue_group", "Venue Group Information", STRING_TYPE},
	{"venue_type", "Venue Type Information", STRING_TYPE},
	{"anqp_query", "ANQP Query (Enabled(1) / Disabled(0))", INTEGER_TYPE}, 
	{"mih_support", "MIH Information Service Advertisement Protocol Enabled/Disabled", INTEGER_TYPE},
	{"venue_name", "Venue Name", STRING_TYPE},
	{"venue_name_id", "Venue Name ID", INTEGER_TYPE},
	{"hessid", "HESSID", STRING_TYPE},
	{"roaming_consortium_oi", "Roaming Consortium OI (Hex)", STRING_TYPE},
	{"advertisement_proto_id", "Advertisement Protocol ID", INTEGER_TYPE},
	{"domain_name", "Domain Name", STRING_TYPE},
	{"network_auth_type", "Network Auth Type", STRING_TYPE},
	{"net_auth_type_id", "Network Auth Type ID", INTEGER_TYPE},
	{"ipv4_type", "IPV4 Type", INTEGER_TYPE},
	{"ipv6_type", "IPV6 Type", INTEGER_TYPE},
	{"ip_type_id", "IP Address Type Availability ID", INTEGER_TYPE},
	{"nai_realm_data", "NAI Realm Data", STRING_TYPE},
	{"nai_realm_id", "NAI Realm List ID", INTEGER_TYPE},
	{"op_friendly_name", "Operator Friendly Name", STRING_TYPE},
	{"op_friendly_name_id", "Operator Friendly Name ID", INTEGER_TYPE},
	{"proto_port", "Protocol Port", STRING_TYPE},
	{"con_cap_id", "Connection Capability Element ID", INTEGER_TYPE},
	{"wan_metrics", "WAN Metrics", STRING_TYPE},
	{"wan_metrics_id", "WAN Metrics ID", INTEGER_TYPE},
	{"plmn", "3GPP cellular network information", STRING_TYPE},
	{"operating_class", "Operating Class information", STRING_TYPE},
	{"operating_class_id", "Operating Class ID", STRING_TYPE},
	{"preferred_candi_list_included", "Preferred Candidate List", INTEGER_TYPE},
	{"abridged", "Abridged", INTEGER_TYPE},
	{"disassociation_imminent", "Disassiciation Imminent", INTEGER_TYPE},
	{"bss_termination_included", "BSS Termination Included", INTEGER_TYPE},
	{"ess_disassociation_imminent", "ESS Disassociation Imminent", INTEGER_TYPE},
	{"disassociation_timer", "Disassociation Timer", INTEGER_TYPE},
	{"validity_interval", "Validity Interval", INTEGER_TYPE},
	{"bss_termination_duration", "BSS Termination Duration", INTEGER_TYPE},
	{"session_information_url", "Session Information URL", STRING_TYPE},
	{"bss_transisition_candi_list_preferences", "BSS Transition Candidates List Preferences", STRING_TYPE},
	{"timezone", "Time Zone", STRING_TYPE},
	{"dgaf_disabled", "Downstream Group addressed Forwarding Disabled Bit (0/1)", INTEGER_TYPE},
	{"proxy_arp", "Proxy ARP Support (Enabled(1) / Disabled(0)", INTEGER_TYPE},
	{"l2_filter", "L2 Traffic Inspection and Filtering", INTEGER_TYPE},
	{"icmpv4_deny", "Deny icmpv4 packet", INTEGER_TYPE},
	{"p2p_cross_connect_permitted", "P2P Cross Connect Permitted", INTEGER_TYPE},
	{"mmpdu_size", "MMPDU Size", INTEGER_TYPE},
	{"external_anqp_server_test", "External ANQP Server Test", INTEGER_TYPE},
	{"gas_cb_delay", "GAS Comeback Delay in TUs", INTEGER_TYPE},
	{"hs2_openmode_test", "Test HS2 Under Open Security Mode", INTEGER_TYPE},
	{"anonymous_nai", "Anonymous NAI", STRING_TYPE},
	{"osu_interface", "OSU Interface", STRING_TYPE},
	{"legacy_osu", "Legacy OSU Enable", INTEGER_TYPE},
	{"osu_providers_list", "OSU Providers List", STRING_TYPE},
	{"osu_providers_id", "OSU Providers List ID", INTEGER_TYPE},
	{"icon_path", "icon file path", STRING_TYPE},
	{"icon_tag", "icon id num", INTEGER_TYPE},
	{"qosmap", "Qosmap Enable", INTEGER_TYPE},
	{"dscp_range", "DSCP Range", STRING_TYPE},
	{"dscp_exception", "DSCP Exception", STRING_TYPE},
	{"qload_test", "QLoad IE test", INTEGER_TYPE},
	{"qload_cu", "QLoad CU", INTEGER_TYPE},
	{"qload_sta_cnt", "QLoad Station Count", INTEGER_TYPE},
	{"mbo_ap_cdcp", "MBO AP CDCP", INTEGER_TYPE},
	{"mbo_ap_assoc_disallow_reason", "MBO AP assoc disallow reason", INTEGER_TYPE},	
	{"mbo_default_assoc_retry_delay", "MBO AP Assoc Retry_Delay", INTEGER_TYPE},
	{"mbo_ap_transition_reason_code", "MBO AP Transition Reason Code", INTEGER_TYPE},
	{"mbo_ap_capability", "MBO AP Capability", INTEGER_TYPE},
	{"venue_url_id", "Venue Url ID", INTEGER_TYPE},
	{"advice_of_charge_id", "Advice of charge ID", INTEGER_TYPE},
	{"t_c_filename", "Termas and condition filename", STRING_TYPE},
	{"t_c_filename_id", "Terms and condition filename id", INTEGER_TYPE},
	{"t_c_timestamp", "Terms and condition timestamp", STRING_TYPE},
	{"osu_providers_nai_list", "osu providers nai list", STRING_TYPE},
	{"osu_providers_nai_id", "osu providers nai id", INTEGER_TYPE},
	{"oim_id", "operator icon metadata id", INTEGER_TYPE},
}; 

static int wapp_cli_cmd_get(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	struct wapp_cli_get_param *param = w_cli_get_params;
	struct wapp_cli_get_param *match = NULL;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);	
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	while (param->param) {
		if (os_strcmp(param->param, argv[2]) == 0) {
			match = param;
			break;
		}
		
		param++;
	}

	if (match) {
		sprintf(&cmd[i], "interface=%s\n", argv[0]);
		i += 11 + os_strlen(argv[0]);

		sprintf(&cmd[i], "cmd=get %s", argv[2]);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknown parameter\n");
		return -1;
	}

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int wapp_cli_cmd_set(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[2048]; //cmd[256];
	int i = 0, ret;
	struct wapp_cli_set_param *param = w_cli_set_params;
	struct wapp_cli_set_param *match = NULL;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	//os_memset(cmd, 0, 256);
	os_memset(cmd,0, 2048);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	while (param->param) {
		if (os_strcmp(param->param, argv[2]) == 0) {
			match = param;
			break;
		}
		
		param++;
	}

	if (match) {
		sprintf(&cmd[i], "interface=%s\n", argv[0]);
		i += 11 + os_strlen(argv[0]);

		sprintf(&cmd[i], "cmd=set %s %s", argv[2], argv[3]);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknown parameter\n");
		return -1;
	}

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);
	
	return ret;
}

static int wapp_cli_cmd_btmreq(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 6)
	{
		DBGPRINT(RT_DEBUG_ERROR, "The params are error, please follow format: btmreq [peer_mac] [ess_IMM] [disassoc_timer] [session_url]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=btmreq %s %s %s %s", argv[2], argv[3], argv[4], argv[5]);

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int wapp_cli_cmd_ext(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, j = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	//DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
#if 1	
	sprintf(&cmd[i], "cmd=");
	i += 4;

	for(j = 1; j < argc; j++) {
		sprintf(&cmd[i], "%s ", argv[j]);
		i += os_strlen(argv[j]) + 1;
	}
#else	
	sprintf(&cmd[i], "cmd=%s %s %s %s %s", argv[1], argv[2], argv[3], argv[4], argv[5]);
#endif
	//DBGPRINT(RT_DEBUG_OFF, "#(cmd(%s) len=%zu)#\n", cmd, os_strlen(cmd));

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	//DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int wapp_cli_cmd_drv_version(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);	
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=drv_version\n");

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_OFF, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int wapp_cli_cmd_wps_pbc_ctrl(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	//DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);	
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=wps_pbc_trigger\n");

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	//DBGPRINT(RT_DEBUG_OFF, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}
static int wapp_cli_cmd_get_version(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	//DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=version_info\n");

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);

	rsp[rsp_len] = '\0';

	return ret;
}

static int wapp_cli_cmd_wps_PbcOnCli_ctrl(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	os_memset(cmd, 0, 256);	
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	sprintf(&cmd[i], "cmd=wps_pbc_trigger_cli\n");
	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	rsp[rsp_len] = '\0';
	return ret;
}
static int wapp_cli_cmd_ipv4_proxy_arp_list(struct wapp_ctrl *ctrl, 
												int argc, char *argv[])
{
	char cmd[256];
	int i = 0, j = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	struct proxy_arp_ipv4_unit *proxy_arp_unit;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=ipv4_proxy_arp_list\n");

	ret =  _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';

	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\n", cmd);
	
	DBGPRINT_RAW(RT_DEBUG_OFF, "MAC\t IPv4 Address\n");
	
	proxy_arp_unit = (struct proxy_arp_ipv4_unit *)rsp;

	for (i = 0; i < (rsp_len / sizeof(*proxy_arp_unit)); i++) {
		for (j = 0; j < 6; j++)
			DBGPRINT_RAW(RT_DEBUG_OFF, "%02x ", proxy_arp_unit->target_mac_addr[j]);

		 DBGPRINT_RAW(RT_DEBUG_OFF, "\t");
	
		for (j = 0; j < 4; j++) {
			if (j == 3) {
				DBGPRINT_RAW(RT_DEBUG_OFF, "%d", proxy_arp_unit->target_ip_addr[j]);
			} else {
				DBGPRINT_RAW(RT_DEBUG_OFF, "%d.", proxy_arp_unit->target_ip_addr[j]);
			}
		}
		
	 	DBGPRINT_RAW(RT_DEBUG_OFF, "\n");
		proxy_arp_unit++;
	}
	
	return ret;
}

static int wapp_cli_cmd_ipv6_proxy_arp_list(struct wapp_ctrl *ctrl, 
												int argc, char *argv[])
{
	char cmd[256];
	int i = 0, j = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	struct proxy_arp_ipv6_unit *proxy_arp_unit;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=ipv6_proxy_arp_list\n");

	ret =  _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';

	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\n", cmd);

	DBGPRINT_RAW(RT_DEBUG_OFF, "IPv6 Type\t MAC\t IPv6 Address\n");
	
	proxy_arp_unit = (struct proxy_arp_ipv6_unit *)rsp;

	for (i = 0; i < (rsp_len / sizeof(*proxy_arp_unit)); i++) {

		if (proxy_arp_unit->target_ip_type == 0) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "Link Local\t");
		} else {
			DBGPRINT_RAW(RT_DEBUG_OFF, "Global\t");
		}

		for (j = 0; j < 6; j++)
			DBGPRINT_RAW(RT_DEBUG_OFF, "%02x ", proxy_arp_unit->target_mac_addr[j]);

		 DBGPRINT_RAW(RT_DEBUG_OFF, "\t");
	
		for (j = 0; j < 16; j++)
			DBGPRINT_RAW(RT_DEBUG_OFF, "%02x ", proxy_arp_unit->target_ip_addr[j]);
		
	 	DBGPRINT_RAW(RT_DEBUG_OFF, "\n");
		proxy_arp_unit++;
	}
	
	return ret;
}

static int wapp_cli_cmd_reload(struct wapp_ctrl *ctrl,
								  int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=reload\n");

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int wapp_cli_cmd_wnmreq(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=wnmreq %s 0 %s", argv[2], argv[3]);

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int wapp_cli_cmd_wnmreq2(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	if (argc == 5)
		sprintf(&cmd[i], "cmd=wnmreq %s 1 %s %s", argv[2], argv[3], argv[4]);
	else
		sprintf(&cmd[i], "cmd=wnmreq %s 1 %s %s %s", argv[2], argv[3], argv[4], argv[5]);

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int wapp_cli_cmd_qosmap(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	if (argc == 5)
		sprintf(&cmd[i], "cmd=qosmap %s 0 %s %s", argv[2], argv[3], argv[4]);
	else
		sprintf(&cmd[i], "cmd=qosmap %s 1 %s", argv[2], argv[3]);

	ret = _wapp_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static struct wapp_cli_cmd w_cli_cmds[] = {
	{"help", wapp_cli_cmd_help, "command usage"},
	{"hs_version", wapp_cli_cmd_version, "show hotspot daemon version"},
	{"on", wapp_cli_cmd_on, "enable hotspot"},
	{"off", wapp_cli_cmd_off, "disable hotspot"},
	{"get [param]", wapp_cli_cmd_get, "get hotspot parameter"},
	{"set [param] [value]", wapp_cli_cmd_set, "set hotspot parameter"},
	{"btmreq [peer_mac] [ess_IMM] [disassoc_timer] [session_url]", wapp_cli_cmd_btmreq,
		"send bss transition request frame to peer address"},
	{"drv_version", wapp_cli_cmd_drv_version, "show wifi driver version"},
	{"ipv4_proxy_arp_list", wapp_cli_cmd_ipv4_proxy_arp_list, "show ipv4 proxy arp table"},
	{"ipv6_proxy_arp_list", wapp_cli_cmd_ipv6_proxy_arp_list, "show ipv6 proxy arp table"},
	{"reload", wapp_cli_cmd_reload, "reload all configuration"},
	{"wnmreq [peer_mac] [server url]", wapp_cli_cmd_wnmreq, "send wnm notify request for remediation"},
	{"wnmreq2 [peer_mac] [code] [delay] [reason url(option)]", wapp_cli_cmd_wnmreq2, "send wnm notify request for deauth imminent"},
	{"qosmap [peer_mac] [dscp_exception(option)] [dscp_range]", wapp_cli_cmd_qosmap, "send qos map configure"},
	{"mbo cmd [arg1] [arg2]", wapp_cli_cmd_ext, "mbo cmd"},
	{"wapp cmd [arg1] [arg2]", wapp_cli_cmd_ext, "wapp cmd"},
	{"map cmd [arg1] [arg2]", wapp_cli_cmd_ext, "map cmd"},
	{"cli_wps_pbc", wapp_cli_cmd_wps_PbcOnCli_ctrl, "PBC for Concurent WPS to be run always on cli interface"},
	{"wps_pbc", wapp_cli_cmd_wps_pbc_ctrl, "PBC for Concurent WPS"},
	{"version_info", wapp_cli_cmd_get_version, "To get version info wapp"},
};

static int wapp_cli_cmd_help(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	struct wapp_cli_cmd *cmd;
	int cmd_num = sizeof(w_cli_cmds) / sizeof(struct wapp_cli_cmd);
	int i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	cmd = w_cli_cmds;
	DBGPRINT_RAW(RT_DEBUG_OFF, "hsctrl [interface] [cmd] [args]\n");
	DBGPRINT_RAW(RT_DEBUG_OFF, "Command Usage:\n");
	
	for (i = 0; i < cmd_num; i++, cmd++) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "  %-60s %-50s\n", cmd->cmd, cmd->usage);
	}
#ifdef DPP_SUPPORT
	wapp_dpp_cmd_show_help(ctrl);
#endif /*DPP_SUPPORT*/

	return 0;
}

static int wapp_cli_request(struct wapp_ctrl *ctrl, int argc, char *argv[])
{
	struct wapp_cli_cmd *cmd, *match = NULL;
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	int i = 0;
	int cmd_num = sizeof(w_cli_cmds) / sizeof(struct wapp_cli_cmd);
	cmd = w_cli_cmds;

	while (i < cmd_num && cmd->cmd) {
		if (os_strncmp(cmd->cmd, argv[1], os_strlen(argv[1])) == 0) {
			match = cmd;
			break;
		}
		cmd++; i++;
	}

	if (match) {
		ret = match->cmd_handler(ctrl, argc, &argv[0]);
	} else {
#ifdef DPP_SUPPORT
		/* Check for dpp command */
		ret = wapp_dpp_cli_request(ctrl, argc, &argv[0]);
		if (ret < 0) {
			DBGPRINT(RT_DEBUG_ERROR, "Unknown command\n");
			wapp_cli_cmd_help(ctrl, 0, NULL);
			ret = -1;
		}
#else
		DBGPRINT(RT_DEBUG_ERROR, "Unknown command\n");
		ret = -1;
#endif /*DPP_SUPPORT*/
	}

	return ret;
}

int optind = 1;
int main(int argc, char *argv[])
{
	int ret = 0;
	char socket_path[64]={0};

	//os_snprintf(socket_path,sizeof(socket_path),"/tmp/hotspot%s",argv[1]);	
	os_snprintf(socket_path,sizeof(socket_path),"/tmp/wapp_ctrl");	

	if (argc < 3) {
		wapp_cli_cmd_help(NULL, 0, NULL);
		return 0;
	}
	ret = wapp_cli_open_connection(socket_path);

	if (ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Failed to open connection to wapp ");
		return ret;
	}
	ret = wapp_cli_request(ctrl_conn, argc - optind, &argv[optind]);

	wapp_cli_close_connection();

	return ret;
}
