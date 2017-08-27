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

#include "qmi-message.h"

static const struct {
	const char *name;
	QmiWdaLinkLayerProtocol val;
} link_modes[] = {
	{ "802.3", QMI_WDA_LINK_LAYER_PROTOCOL_802_3 },
	{ "raw-ip", QMI_WDA_LINK_LAYER_PROTOCOL_RAW_IP },
};

#define cmd_wda_set_data_format_cb no_cb

static enum qmi_cmd_result
cmd_wda_set_data_format_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_wda_set_data_format_request data_req = {};
	int i;

	for (i = 0; i < ARRAY_SIZE(link_modes); i++) {
		if (strcasecmp(link_modes[i].name, arg) != 0)
			continue;

		qmi_set(&data_req, link_layer_protocol, link_modes[i].val);
		qmi_set_wda_set_data_format_request(msg, &data_req);
		return QMI_CMD_REQUEST;
	}

	uqmi_add_error("Invalid auth mode (valid: 802.3, raw-ip)");
	return QMI_CMD_EXIT;
}

static void
cmd_wda_get_data_format_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wda_get_data_format_response res;
	const char *name = "unknown";
	int i;

	qmi_parse_wda_get_data_format_response(msg, &res);
	for (i = 0; i < ARRAY_SIZE(link_modes); i++) {
		if (link_modes[i].val != res.data.link_layer_protocol)
			continue;

		name = link_modes[i].name;
		break;
	}

	blobmsg_add_string(&status, NULL, name);
}

static enum qmi_cmd_result
cmd_wda_get_data_format_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_wda_get_data_format_request(msg);
	return QMI_CMD_REQUEST;
}
