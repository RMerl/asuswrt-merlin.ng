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

#ifndef __UQMI_COMMANDS_H
#define __UQMI_COMMANDS_H

#include <stdbool.h>
#include "commands-wds.h"
#include "commands-dms.h"
#include "commands-nas.h"
#include "commands-wms.h"
#include "commands-wda.h"
#include "commands-uim.h"

enum qmi_cmd_result {
	QMI_CMD_DONE,
	QMI_CMD_REQUEST,
	QMI_CMD_EXIT,
};

enum {
	CMD_TYPE_OPTION = -1,
};

struct uqmi_cmd_handler {
	const char *name;
	int type;

	enum qmi_cmd_result (*prepare)(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg);
	void (*cb)(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg);
};

struct uqmi_cmd {
	const struct uqmi_cmd_handler *handler;
	char *arg;
};

#define __uqmi_commands \
	__uqmi_command(version, get-versions, no, QMI_SERVICE_CTL), \
	__uqmi_command(sync, sync, no, QMI_SERVICE_CTL), \
	__uqmi_command(set_client_id, set-client-id, required, CMD_TYPE_OPTION), \
	__uqmi_command(get_client_id, get-client-id, required, QMI_SERVICE_CTL), \
	__uqmi_command(ctl_set_data_format, set-data-format, required, QMI_SERVICE_CTL), \
	__uqmi_wds_commands, \
	__uqmi_dms_commands, \
	__uqmi_nas_commands, \
	__uqmi_wms_commands, \
	__uqmi_wda_commands, \
	__uqmi_uim_commands

#define __uqmi_command(_name, _optname, _arg, _option) __UQMI_COMMAND_##_name
enum uqmi_command {
	__uqmi_commands,
	__UQMI_COMMAND_LAST
};
#undef __uqmi_command

extern bool single_line;
extern const struct uqmi_cmd_handler uqmi_cmd_handler[];
void uqmi_add_command(char *arg, int longidx);
bool uqmi_run_commands(struct qmi_dev *qmi);
int uqmi_add_error(const char *msg);

#endif
