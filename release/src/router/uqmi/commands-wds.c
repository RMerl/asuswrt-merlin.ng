/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <arpa/inet.h>

#include "qmi-message.h"

static struct qmi_wds_start_network_request wds_sn_req = {
	QMI_INIT(authentication_preference,
	         QMI_WDS_AUTHENTICATION_PAP | QMI_WDS_AUTHENTICATION_CHAP),
};
static struct qmi_wds_stop_network_request wds_stn_req;

#define cmd_wds_set_apn_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_apn_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_ptr(&wds_sn_req, apn, arg);
	return QMI_CMD_DONE;
}

#define cmd_wds_set_auth_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_auth_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static const struct {
		const char *name;
		QmiWdsAuthentication auth;
	} modes[] = {
		{ "pap", QMI_WDS_AUTHENTICATION_PAP },
		{ "chap", QMI_WDS_AUTHENTICATION_CHAP },
		{ "both", QMI_WDS_AUTHENTICATION_PAP | QMI_WDS_AUTHENTICATION_CHAP },
		{ "none", QMI_WDS_AUTHENTICATION_NONE },
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		if (strcasecmp(modes[i].name, arg) != 0)
			continue;

		qmi_set(&wds_sn_req, authentication_preference, modes[i].auth);
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid auth mode (valid: pap, chap, both, none)");
	return QMI_CMD_EXIT;
}

#define cmd_wds_set_username_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_username_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_ptr(&wds_sn_req, username, arg);
	return QMI_CMD_DONE;
}

#define cmd_wds_set_password_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_password_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_ptr(&wds_sn_req, password, arg);
	return QMI_CMD_DONE;
}

#define cmd_wds_set_autoconnect_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_autoconnect_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set(&wds_sn_req, enable_autoconnect, true);
	qmi_set(&wds_stn_req, disable_autoconnect, true);
	return QMI_CMD_DONE;
}

#define cmd_wds_set_ip_family_pref_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_ip_family_pref_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static const struct {
		const char *name;
		const QmiWdsIpFamily mode;
	} modes[] = {
		{ "ipv4", QMI_WDS_IP_FAMILY_IPV4 },
		{ "ipv6", QMI_WDS_IP_FAMILY_IPV6 },
		{ "unspecified", QMI_WDS_IP_FAMILY_UNSPECIFIED },
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		if (strcasecmp(modes[i].name, arg) != 0)
			continue;

		qmi_set(&wds_sn_req, ip_family_preference, modes[i].mode);
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid value (valid: ipv4, ipv6, unspecified)");
	return QMI_CMD_EXIT;
}

#define cmd_wds_set_profile_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_profile_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	uint32_t idx = strtoul(arg, NULL, 10);

	qmi_set(&wds_sn_req, profile_index_3gpp, idx);
	return QMI_CMD_DONE;
}

static void
cmd_wds_start_network_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_start_network_response res;

	qmi_parse_wds_start_network_response(msg, &res);
	if (res.set.packet_data_handle)
		blobmsg_add_u32(&status, NULL, res.data.packet_data_handle);
}

static enum qmi_cmd_result
cmd_wds_start_network_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_wds_start_network_request(msg, &wds_sn_req);
	return QMI_CMD_REQUEST;
}

#define cmd_wds_stop_network_cb no_cb
static enum qmi_cmd_result
cmd_wds_stop_network_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	uint32_t pdh = strtoul(arg, NULL, 0);

	qmi_set(&wds_stn_req, packet_data_handle, pdh);
	qmi_set_wds_stop_network_request(msg, &wds_stn_req);
	return QMI_CMD_REQUEST;
}

static void
cmd_wds_get_packet_service_status_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_get_packet_service_status_response res;
	const char *data_status[] = {
		[QMI_WDS_CONNECTION_STATUS_UNKNOWN] = "unknown",
		[QMI_WDS_CONNECTION_STATUS_DISCONNECTED] = "disconnected",
		[QMI_WDS_CONNECTION_STATUS_CONNECTED] = "connected",
		[QMI_WDS_CONNECTION_STATUS_SUSPENDED] = "suspended",
		[QMI_WDS_CONNECTION_STATUS_AUTHENTICATING] = "authenticating",
	};
	int s = 0;

	qmi_parse_wds_get_packet_service_status_response(msg, &res);
	if (res.set.connection_status &&
	    res.data.connection_status < ARRAY_SIZE(data_status))
		s = res.data.connection_status;

	blobmsg_add_string(&status, NULL, data_status[s]);
}

static enum qmi_cmd_result
cmd_wds_get_packet_service_status_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_wds_get_packet_service_status_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_wds_set_autoconnect_setting_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_autoconnect_setting_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_wds_set_autoconnect_setting_request ac_req;
	const char *modes[] = {
		[QMI_WDS_AUTOCONNECT_DISABLED] = "disabled",
		[QMI_WDS_AUTOCONNECT_ENABLED] = "enabled",
		[QMI_WDS_AUTOCONNECT_PAUSED] = "paused",
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		if (strcasecmp(modes[i], arg) != 0)
			continue;

		qmi_set(&ac_req, setting, i);
		qmi_set_wds_set_autoconnect_setting_request(msg, &ac_req);
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid value (valid: disabled, enabled, paused)");
	return QMI_CMD_EXIT;
}

#define cmd_wds_reset_cb no_cb
static enum qmi_cmd_result
cmd_wds_reset_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_wds_reset_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_wds_set_ip_family_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_ip_family_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_wds_set_ip_family_request ipf_req;
	const struct ip_modes {
		const char *name;
		const QmiWdsIpFamily mode;
	} modes[] = {
		{ "ipv4", QMI_WDS_IP_FAMILY_IPV4 },
		{ "ipv6", QMI_WDS_IP_FAMILY_IPV6 },
		{ "unspecified", QMI_WDS_IP_FAMILY_UNSPECIFIED },
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		if (strcasecmp(modes[i].name, arg) != 0)
			continue;

		qmi_set(&ipf_req, preference, modes[i].mode);
		qmi_set_wds_set_ip_family_request(msg, &ipf_req);
		return QMI_CMD_REQUEST;
	}

	uqmi_add_error("Invalid value (valid: ipv4, ipv6, unspecified)");
	return QMI_CMD_EXIT;
}

static void wds_to_ipv4(const char *name, const uint32_t addr)
{
	struct in_addr ip_addr;
	char buf[INET_ADDRSTRLEN];

	ip_addr.s_addr = htonl(addr);
	blobmsg_add_string(&status, name, inet_ntop(AF_INET, &ip_addr, buf, sizeof(buf)));
}

static void wds_to_ipv6(const char *name, const uint16_t *addr)
{
	char buf[INET6_ADDRSTRLEN];
	uint16_t ip_addr[8];
	int i;

	for (i = 0; i < ARRAY_SIZE(ip_addr); i++)
		ip_addr[i] = htons(addr[i]);

	blobmsg_add_string(&status, name, inet_ntop(AF_INET6, &ip_addr, buf, sizeof(buf)));
}

static void
cmd_wds_get_current_settings_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	void *v4, *v6, *d, *t;
	struct qmi_wds_get_current_settings_response res;
	const char *pdptypes[] = {
		[QMI_WDS_PDP_TYPE_IPV4] = "ipv4",
		[QMI_WDS_PDP_TYPE_PPP] = "ppp",
		[QMI_WDS_PDP_TYPE_IPV6] = "ipv6",
		[QMI_WDS_PDP_TYPE_IPV4_OR_IPV6] = "ipv4-or-ipv6",
	};
	const struct ip_modes {
		const char *name;
		const QmiWdsIpFamily mode;
	} modes[] = {
		{ "ipv4", QMI_WDS_IP_FAMILY_IPV4 },
		{ "ipv6", QMI_WDS_IP_FAMILY_IPV6 },
		{ "unspecified", QMI_WDS_IP_FAMILY_UNSPECIFIED },
	};
	int i;

	qmi_parse_wds_get_current_settings_response(msg, &res);

	t = blobmsg_open_table(&status, NULL);

	if (res.set.pdp_type && (int) res.data.pdp_type < ARRAY_SIZE(pdptypes))
		blobmsg_add_string(&status, "pdp-type", pdptypes[res.data.pdp_type]);

	if (res.set.ip_family) {
		for (i = 0; i < ARRAY_SIZE(modes); i++) {
			if (modes[i].mode != res.data.ip_family)
				continue;
			blobmsg_add_string(&status, "ip-family", modes[i].name);
			break;
		}
	}

	if (res.set.mtu)
		blobmsg_add_u32(&status, "mtu", res.data.mtu);

	/* IPV4 */
	v4 = blobmsg_open_table(&status, "ipv4");

	if (res.set.ipv4_address)
		wds_to_ipv4("ip", res.data.ipv4_address);
	if (res.set.primary_ipv4_dns_address)
		wds_to_ipv4("dns1", res.data.primary_ipv4_dns_address);
	if (res.set.secondary_ipv4_dns_address)
		wds_to_ipv4("dns2", res.data.secondary_ipv4_dns_address);
	if (res.set.ipv4_gateway_address)
		wds_to_ipv4("gateway", res.data.ipv4_gateway_address);
	if (res.set.ipv4_gateway_subnet_mask)
		wds_to_ipv4("subnet", res.data.ipv4_gateway_subnet_mask);
	blobmsg_close_table(&status, v4);

	/* IPV6 */
	v6 = blobmsg_open_table(&status, "ipv6");

	if (res.set.ipv6_address) {
		wds_to_ipv6("ip", res.data.ipv6_address.address);
		blobmsg_add_u32(&status, "ip-prefix-length", res.data.ipv6_address.prefix_length);
	}
	if (res.set.ipv6_gateway_address) {
		wds_to_ipv6("gateway", res.data.ipv6_gateway_address.address);
		blobmsg_add_u32(&status, "gw-prefix-length", res.data.ipv6_gateway_address.prefix_length);
	}
	if (res.set.ipv6_primary_dns_address)
		wds_to_ipv6("dns1", res.data.ipv6_primary_dns_address);
	if (res.set.ipv6_secondary_dns_address)
		wds_to_ipv6("dns2", res.data.ipv6_secondary_dns_address);

	blobmsg_close_table(&status, v6);

	d = blobmsg_open_table(&status, "domain-names");
	for (i = 0; i < res.data.domain_name_list_n; i++) {
		blobmsg_add_string(&status, NULL, res.data.domain_name_list[i]);
	}
	blobmsg_close_table(&status, d);

	blobmsg_close_table(&status, t);
}

static enum qmi_cmd_result
cmd_wds_get_current_settings_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_wds_get_current_settings_request gcs_req;
	memset(&gcs_req, '\0', sizeof(struct qmi_wds_get_current_settings_request));
	qmi_set(&gcs_req, requested_settings,
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_PDP_TYPE |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_DNS_ADDRESS |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_GRANTED_QOS |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_IP_ADDRESS |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_GATEWAY_INFO |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_MTU |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_DOMAIN_NAME_LIST |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_IP_FAMILY);
	qmi_set_wds_get_current_settings_request(msg, &gcs_req);
	return QMI_CMD_REQUEST;
}
