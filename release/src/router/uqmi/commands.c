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

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>

#include "uqmi.h"
#include "commands.h"

static struct blob_buf status;
bool single_line = false;

static void no_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
}

static void cmd_version_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_ctl_get_version_info_response res;
	void *c;
	char name_buf[16];
	int i;

	qmi_parse_ctl_get_version_info_response(msg, &res);

	c = blobmsg_open_table(&status, NULL);
	for (i = 0; i < res.data.service_list_n; i++) {
		sprintf(name_buf, "service_%d", res.data.service_list[i].service);
		blobmsg_printf(&status, name_buf, "%d,%d",
			res.data.service_list[i].major_version,
			res.data.service_list[i].minor_version);
	}
	blobmsg_close_table(&status, c);
}

static enum qmi_cmd_result
cmd_version_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_ctl_get_version_info_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_sync_cb no_cb
static enum qmi_cmd_result
cmd_sync_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_ctl_sync_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_get_client_id_cb no_cb
static enum qmi_cmd_result
cmd_get_client_id_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	QmiService svc = qmi_service_get_by_name(arg);

	if (svc < 0) {
		fprintf(stderr, "Invalid service name '%s'\n", arg);
		return QMI_CMD_EXIT;
	}

	if (qmi_service_connect(qmi, svc, -1)) {
		fprintf(stderr, "Failed to connect to service\n");
		return QMI_CMD_EXIT;
	}

	printf("%d\n", qmi_service_get_client_id(qmi, svc));
	return QMI_CMD_DONE;
}

#define cmd_set_client_id_cb no_cb
static enum qmi_cmd_result
cmd_set_client_id_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	QmiService svc;
	int id;
	char *s;

	s = strchr(arg, ',');
	if (!s) {
		fprintf(stderr, "Invalid argument\n");
		return QMI_CMD_EXIT;
	}
	*s = 0;
	s++;

	id = strtoul(s, &s, 0);
	if (s && *s) {
		fprintf(stderr, "Invalid argument\n");
		return QMI_CMD_EXIT;
	}

	svc = qmi_service_get_by_name(arg);
	if (svc < 0) {
		fprintf(stderr, "Invalid service name '%s'\n", arg);
		return QMI_CMD_EXIT;
	}

	if (qmi_service_connect(qmi, svc, id)) {
		fprintf(stderr, "Failed to connect to service\n");
		return QMI_CMD_EXIT;
	}

	return QMI_CMD_DONE;
}

static int
qmi_get_array_idx(const char **array, int size, const char *str)
{
	int i;

	for (i = 0; i < size; i++) {
		if (!array[i])
			continue;

		if (!strcmp(array[i], str))
			return i;
	}

	return -1;
}

#define cmd_ctl_set_data_format_cb no_cb
static enum qmi_cmd_result
cmd_ctl_set_data_format_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_ctl_set_data_format_request sreq = {};
	const char *modes[] = {
		[QMI_CTL_DATA_LINK_PROTOCOL_802_3] = "802.3",
		[QMI_CTL_DATA_LINK_PROTOCOL_RAW_IP] = "raw-ip",
	};
	int mode = qmi_get_array_idx(modes, ARRAY_SIZE(modes), arg);

	if (mode < 0) {
		uqmi_add_error("Invalid mode (modes: 802.3, raw-ip)");
		return QMI_CMD_EXIT;
	}

	qmi_set_ctl_set_data_format_request(msg, &sreq);
	return QMI_CMD_DONE;
}

#include "commands-wds.c"
#include "commands-dms.c"
#include "commands-nas.c"
#include "commands-wms.c"
#include "commands-wda.c"
#include "commands-uim.c"

#define __uqmi_command(_name, _optname, _arg, _type) \
	[__UQMI_COMMAND_##_name] = { \
		.name = #_optname, \
		.type = _type, \
		.prepare = cmd_##_name##_prepare, \
		.cb = cmd_##_name##_cb, \
	}

const struct uqmi_cmd_handler uqmi_cmd_handler[__UQMI_COMMAND_LAST] = {
	__uqmi_commands
};
#undef __uqmi_command

static struct uqmi_cmd *cmds;
static int n_cmds;

void uqmi_add_command(char *arg, int cmd)
{
	int idx = n_cmds++;

	cmds = realloc(cmds, n_cmds * sizeof(*cmds));
	cmds[idx].handler = &uqmi_cmd_handler[cmd];
	cmds[idx].arg = optarg;
}

static void uqmi_print_result(struct blob_attr *data)
{
	char *str;

	if (!blob_len(data))
		return;

	str = blobmsg_format_json_indent(blob_data(data), false, single_line ? -1 : 0);
	if (!str)
		return;

	printf("%s\n", str);
	free(str);
}

static bool __uqmi_run_commands(struct qmi_dev *qmi, bool option)
{
	static struct qmi_request req;
	char *buf = qmi->buf;
	int i;

	for (i = 0; i < n_cmds; i++) {
		enum qmi_cmd_result res;
		bool cmd_option = cmds[i].handler->type == CMD_TYPE_OPTION;
		bool do_break = false;

		if (cmd_option != option)
			continue;

		blob_buf_init(&status, 0);
		if (cmds[i].handler->type > QMI_SERVICE_CTL &&
		    qmi_service_connect(qmi, cmds[i].handler->type, -1)) {
			uqmi_add_error("Failed to connect to service");
			res = QMI_CMD_EXIT;
		} else {
			res = cmds[i].handler->prepare(qmi, &req, (void *) buf, cmds[i].arg);
		}

		if (res == QMI_CMD_REQUEST) {
			qmi_request_start(qmi, &req, cmds[i].handler->cb);
			req.no_error_cb = true;
			if (qmi_request_wait(qmi, &req)) {
				uqmi_add_error(qmi_get_error_str(req.ret));
				do_break = true;
			}
		} else if (res == QMI_CMD_EXIT) {
			do_break = true;
		}

		uqmi_print_result(status.head);
		if (do_break)
			return false;
	}
	return true;
}

int uqmi_add_error(const char *msg)
{
	blobmsg_add_string(&status, NULL, msg);
	return QMI_CMD_EXIT;
}

bool uqmi_run_commands(struct qmi_dev *qmi)
{
	bool ret;

	ret = __uqmi_run_commands(qmi, true) &&
	      __uqmi_run_commands(qmi, false);

	free(cmds);
	cmds = NULL;
	n_cmds = 0;

	return ret;
}
