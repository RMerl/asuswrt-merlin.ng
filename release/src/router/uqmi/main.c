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

#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>

#include "uqmi.h"
#include "commands.h"

static const char *device;

#define CMD_OPT(_arg) (-2 - _arg)

#define __uqmi_command(_name, _optname, _arg, _option) { #_optname, _arg##_argument, NULL, CMD_OPT(__UQMI_COMMAND_##_name) }
static const struct option uqmi_getopt[] = {
	__uqmi_commands,
	{ "single", no_argument, NULL, 's' },
	{ "device", required_argument, NULL, 'd' },
	{ "keep-client-id", required_argument, NULL, 'k' },
	{ "release-client-id", required_argument, NULL, 'r' },
	{ "mbim",  no_argument, NULL, 'm' },
	{ NULL, 0, NULL, 0 }
};
#undef __uqmi_command

static int usage(const char *progname)
{
	fprintf(stderr, "Usage: %s <options|actions>\n"
		"Options:\n"
		"  --single, -s:                     Print output as a single line (for scripts)\n"
		"  --device=NAME, -d NAME:           Set device name to NAME (required)\n"
		"  --keep-client-id <name>:          Keep Client ID for service <name>\n"
		"  --release-client-id <name>:       Release Client ID after exiting\n"
		"  --mbim, -m                        NAME is an MBIM device with EXT_QMUX support\n"
		"\n"
		"Services:                           dms, nas, pds, wds, wms\n"
		"\n"
		"Actions:\n"
		"  --get-versions:                   Get service versions\n"
		"  --set-client-id <name>,<id>:      Set Client ID for service <name> to <id>\n"
		"                                    (implies --keep-client-id)\n"
		"  --get-client-id <name>:           Connect and get Client ID for service <name>\n"
		"                                    (implies --keep-client-id)\n"
		"  --sync:                           Release all Client IDs\n"
		wds_helptext
		dms_helptext
		uim_helptext
		nas_helptext
		wms_helptext
		wda_helptext
		"\n", progname);
	return 1;
}

static void keep_client_id(struct qmi_dev *qmi, const char *optarg)
{
	QmiService svc = qmi_service_get_by_name(optarg);
	if (svc < 0) {
		fprintf(stderr, "Invalid service %s\n", optarg);
		exit(1);
	}
	qmi_service_get_client_id(qmi, svc);
}

static void release_client_id(struct qmi_dev *qmi, const char *optarg)
{
	QmiService svc = qmi_service_get_by_name(optarg);
	if (svc < 0) {
		fprintf(stderr, "Invalid service %s\n", optarg);
		exit(1);
	}
	qmi_service_release_client_id(qmi, svc);
}

static void handle_exit_signal(int signal)
{
	cancel_all_requests = true;
	uloop_end();
}

int main(int argc, char **argv)
{
	static struct qmi_dev dev;
	int ch, ret;

	uloop_init();
	signal(SIGINT, handle_exit_signal);
	signal(SIGTERM, handle_exit_signal);

	while ((ch = getopt_long(argc, argv, "d:k:sm", uqmi_getopt, NULL)) != -1) {
		int cmd_opt = CMD_OPT(ch);

		if (ch < 0 && cmd_opt >= 0 && cmd_opt < __UQMI_COMMAND_LAST) {
			uqmi_add_command(optarg, cmd_opt);
			continue;
		}

		switch(ch) {
		case 'r':
			release_client_id(&dev, optarg);
			break;
		case 'k':
			keep_client_id(&dev, optarg);
			break;
		case 'd':
			device = optarg;
			break;
		case 's':
			single_line = true;
			break;
		case 'm':
			dev.is_mbim = true;
			break;
		default:
			return usage(argv[0]);
		}
	}

	if (!device) {
		fprintf(stderr, "No device given\n");
		return usage(argv[0]);
	}

	if (qmi_device_open(&dev, device)) {
		fprintf(stderr, "Failed to open device\n");
		return 2;
	}

	ret = uqmi_run_commands(&dev) ? 0 : -1;

	qmi_device_close(&dev);

	return ret;
}
