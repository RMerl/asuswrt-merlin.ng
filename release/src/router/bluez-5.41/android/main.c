/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/signalfd.h>
#if defined(ANDROID)
#include <sys/prctl.h>
#include <sys/capability.h>
#endif

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "src/log.h"
#include "src/sdpd.h"
#include "src/shared/util.h"

#include "ipc-common.h"
#include "ipc.h"
#include "bluetooth.h"
#include "socket.h"
#include "hidhost.h"
#include "hal-msg.h"
#include "a2dp.h"
#include "pan.h"
#include "avrcp.h"
#include "handsfree.h"
#include "gatt.h"
#include "health.h"
#include "handsfree-client.h"
#include "map-client.h"
#include "utils.h"

#define DEFAULT_VENDOR "BlueZ"
#define DEFAULT_MODEL "BlueZ for Android"
#define DEFAULT_NAME "BlueZ for Android"

#define STARTUP_GRACE_SECONDS 5
#define SHUTDOWN_GRACE_SECONDS 5

static char *config_vendor = NULL;
static char *config_model = NULL;
static char *config_name = NULL;
static char *config_serial = NULL;
static char *config_fw_rev = NULL;
static char *config_hw_rev = NULL;
static uint64_t config_system_id = 0;
static uint16_t config_pnp_source = 0x0002;	/* USB */
static uint16_t config_pnp_vendor = 0x1d6b;	/* Linux Foundation */
static uint16_t config_pnp_product = 0x0247;	/* BlueZ for Android */
static uint16_t config_pnp_version = 0x0000;

static guint quit_timeout = 0;

static bdaddr_t adapter_bdaddr;

static GMainLoop *event_loop;

static struct ipc *hal_ipc = NULL;

static bool services[HAL_SERVICE_ID_MAX + 1] = { false };

const char *bt_config_get_vendor(void)
{
	if (config_vendor)
		return config_vendor;

	return DEFAULT_VENDOR;
}

const char *bt_config_get_name(void)
{
	if (config_name)
		return config_name;

	return DEFAULT_NAME;
}

const char *bt_config_get_model(void)
{
	if (config_model)
		return config_model;

	return DEFAULT_MODEL;
}

const char *bt_config_get_serial(void)
{
	return config_serial;
}

const char *bt_config_get_fw_rev(void)
{
	return config_fw_rev;
}

const char *bt_config_get_hw_rev(void)
{
	return config_hw_rev;
}

uint64_t bt_config_get_system_id(void)
{
	return config_system_id;
}

uint16_t bt_config_get_pnp_source(void)
{
	return config_pnp_source;
}

uint16_t bt_config_get_pnp_vendor(void)
{
	return config_pnp_vendor;
}

uint16_t bt_config_get_pnp_product(void)
{
	return config_pnp_product;
}

uint16_t bt_config_get_pnp_version(void)
{
	return config_pnp_version;
}

static void service_register(const void *buf, uint16_t len)
{
	const struct hal_cmd_register_module *m = buf;
	uint8_t status;

	if (m->service_id > HAL_SERVICE_ID_MAX || services[m->service_id]) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	switch (m->service_id) {
	case HAL_SERVICE_ID_BLUETOOTH:
		if (!bt_bluetooth_register(hal_ipc, m->mode)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_SOCKET:
		bt_socket_register(hal_ipc, &adapter_bdaddr, m->mode);

		break;
	case HAL_SERVICE_ID_HIDHOST:
		if (!bt_hid_register(hal_ipc, &adapter_bdaddr, m->mode)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_A2DP:
		if (!bt_a2dp_register(hal_ipc, &adapter_bdaddr, m->mode)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_PAN:
		if (!bt_pan_register(hal_ipc, &adapter_bdaddr, m->mode)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_AVRCP:
		if (!bt_avrcp_register(hal_ipc, &adapter_bdaddr, m->mode)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_HANDSFREE:
		if (!bt_handsfree_register(hal_ipc, &adapter_bdaddr, m->mode,
							m->max_clients)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_GATT:
		if (!bt_gatt_register(hal_ipc, &adapter_bdaddr)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_HEALTH:
		if (!bt_health_register(hal_ipc, &adapter_bdaddr, m->mode)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_HANDSFREE_CLIENT:
		if (!bt_hf_client_register(hal_ipc, &adapter_bdaddr)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	case HAL_SERVICE_ID_MAP_CLIENT:
		if (!bt_map_client_register(hal_ipc, &adapter_bdaddr,
								m->mode)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		break;
	default:
		DBG("service %u not supported", m->service_id);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	services[m->service_id] = true;

	status = HAL_STATUS_SUCCESS;

	info("Service ID=%u registered", m->service_id);

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
								status);
}

static bool unregister_service(uint8_t id)
{
	if (id > HAL_SERVICE_ID_MAX || !services[id])
		return false;

	switch (id) {
	case HAL_SERVICE_ID_BLUETOOTH:
		bt_bluetooth_unregister();
		break;
	case HAL_SERVICE_ID_SOCKET:
		bt_socket_unregister();
		break;
	case HAL_SERVICE_ID_HIDHOST:
		bt_hid_unregister();
		break;
	case HAL_SERVICE_ID_A2DP:
		bt_a2dp_unregister();
		break;
	case HAL_SERVICE_ID_PAN:
		bt_pan_unregister();
		break;
	case HAL_SERVICE_ID_AVRCP:
		bt_avrcp_unregister();
		break;
	case HAL_SERVICE_ID_HANDSFREE:
		bt_handsfree_unregister();
		break;
	case HAL_SERVICE_ID_GATT:
		bt_gatt_unregister();
		break;
	case HAL_SERVICE_ID_HEALTH:
		bt_health_unregister();
		break;
	case HAL_SERVICE_ID_HANDSFREE_CLIENT:
		bt_hf_client_unregister();
		break;
	case HAL_SERVICE_ID_MAP_CLIENT:
		bt_map_client_unregister();
		break;
	default:
		DBG("service %u not supported", id);
		return false;
	}

	services[id] = false;

	return true;
}

static void service_unregister(const void *buf, uint16_t len)
{
	const struct hal_cmd_unregister_module *m = buf;
	uint8_t status;

	if (!unregister_service(m->service_id)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

	info("Service ID=%u unregistered", m->service_id);

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
								status);
}

static char *get_prop(char *prop, uint16_t len, const uint8_t *val)
{
	/* TODO should fail if set more than once ? */
	free(prop);

	prop = malloc0(len);
	if (!prop)
		return NULL;

	memcpy(prop, val, len);
	prop[len - 1] = '\0';

	return prop;
}

static void parse_pnp_id(uint16_t len, const uint8_t *val)
{
	int result;
	uint16_t vendor, product, version , source;
	char *pnp;

	/* version is optional */
	version = config_pnp_version;

	pnp = get_prop(NULL, len, val);
	if (!pnp)
		return;

	DBG("pnp_id %s", pnp);

	result = sscanf(pnp, "bluetooth:%4hx:%4hx:%4hx",
						&vendor, &product, &version);
	if (result != EOF && result >= 2) {
		source = 0x0001;
		goto done;
	}

	result = sscanf(pnp, "usb:%4hx:%4hx:%4hx", &vendor, &product, &version);
	if (result != EOF && result >= 2) {
		source = 0x0002;
		goto done;
	}

	free(pnp);
	return;
done:
	free(pnp);

	config_pnp_source = source;
	config_pnp_vendor = vendor;
	config_pnp_product = product;
	config_pnp_version = version;
}

static void parse_system_id(uint16_t len, const uint8_t *val)
{
	uint64_t res;
	char *id;

	id = get_prop(NULL, len, val);
	if (!id)
		return;

	res = strtoull(id, NULL, 16);
	if (res == ULLONG_MAX && errno == ERANGE)
		goto done;

	config_system_id = res;
done:
	free(id);
}

static void configuration(const void *buf, uint16_t len)
{
	const struct hal_cmd_configuration *cmd = buf;
	const struct hal_config_prop *prop;
	unsigned int i;

	buf += sizeof(*cmd);
	len -= sizeof(*cmd);

	for (i = 0; i < cmd->num; i++) {
		prop = buf;

		if (len < sizeof(*prop) || len < sizeof(*prop) + prop->len) {
			error("Invalid configuration command, terminating");
			raise(SIGTERM);
			return;
		}

		switch (prop->type) {
		case HAL_CONFIG_VENDOR:
			config_vendor = get_prop(config_vendor, prop->len,
								prop->val);
			DBG("vendor %s", config_vendor);
			break;
		case HAL_CONFIG_NAME:
			config_name = get_prop(config_name, prop->len,
								prop->val);
			DBG("name %s", config_name);
			break;
		case HAL_CONFIG_MODEL:
			config_model = get_prop(config_model, prop->len,
								prop->val);
			DBG("model %s", config_model);
			break;
		case HAL_CONFIG_SERIAL_NUMBER:
			config_serial = get_prop(config_serial, prop->len,
								prop->val);
			DBG("serial %s", config_serial);
			break;
		case HAL_CONFIG_SYSTEM_ID:
			parse_system_id(prop->len, prop->val);
			break;
		case HAL_CONFIG_PNP_ID:
			parse_pnp_id(prop->len, prop->val);
			break;
		case HAL_CONFIG_FW_REV:
			config_fw_rev = get_prop(config_fw_rev, prop->len,
								prop->val);
			DBG("fw_rev %s", config_fw_rev);
			break;
		case HAL_CONFIG_HW_REV:
			config_hw_rev = get_prop(config_hw_rev, prop->len,
								prop->val);
			DBG("hw_rev %s", config_hw_rev);
			break;
		default:
			error("Invalid configuration option (%u), terminating",
								prop->type);
			raise(SIGTERM);
			return;
		}

		buf += sizeof(*prop) + prop->len;
		len -= sizeof(*prop) + prop->len;
	}

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_CORE, HAL_OP_CONFIGURATION,
							HAL_STATUS_SUCCESS);
}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_REGISTER_MODULE */
	{ service_register, false, sizeof(struct hal_cmd_register_module) },
	/* HAL_OP_UNREGISTER_MODULE */
	{ service_unregister, false, sizeof(struct hal_cmd_unregister_module) },
	/* HAL_OP_CONFIGURATION */
	{ configuration, true, sizeof(struct hal_cmd_configuration) },
};

static void bluetooth_stopped(void)
{
	g_main_loop_quit(event_loop);
}

static gboolean quit_eventloop(gpointer user_data)
{
	g_main_loop_quit(event_loop);

	quit_timeout = 0;

	return FALSE;
}

static void stop_bluetooth(void)
{
	static bool __stop = false;

	if (__stop)
		return;

	__stop = true;

	if (!bt_bluetooth_stop(bluetooth_stopped)) {
		g_main_loop_quit(event_loop);
		return;
	}

	quit_timeout = g_timeout_add_seconds(SHUTDOWN_GRACE_SECONDS,
							quit_eventloop, NULL);
}

static void ipc_disconnected(void *data)
{
	stop_bluetooth();
}

static void adapter_ready(int err, const bdaddr_t *addr)
{
	if (err < 0) {
		error("Adapter initialization failed: %s", strerror(-err));
		exit(EXIT_FAILURE);
	}

	bacpy(&adapter_bdaddr, addr);

	if (quit_timeout > 0) {
		g_source_remove(quit_timeout);
		quit_timeout = 0;
	}

	info("Adapter initialized");

	hal_ipc = ipc_init(BLUEZ_HAL_SK_PATH, sizeof(BLUEZ_HAL_SK_PATH),
						HAL_SERVICE_ID_MAX, true,
						ipc_disconnected, NULL);
	if (!hal_ipc) {
		error("Failed to initialize IPC");
		exit(EXIT_FAILURE);
	}

	ipc_register(hal_ipc, HAL_SERVICE_ID_CORE, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));
}

static gboolean signal_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	static bool __terminated = false;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGINT:
	case SIGTERM:
		if (!__terminated) {
			info("Terminating");
			stop_bluetooth();
		}

		__terminated = true;
		break;
	}

	return TRUE;
}

static guint setup_signalfd(void)
{
	GIOChannel *channel;
	guint source;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("Failed to set signal mask");
		return 0;
	}

	fd = signalfd(-1, &mask, 0);
	if (fd < 0) {
		perror("Failed to create signal descriptor");
		return 0;
	}

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				signal_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

static gboolean option_version = FALSE;
static gint option_index = -1;
static gboolean option_dbg = FALSE;
static gboolean option_mgmt_dbg = FALSE;

static GOptionEntry options[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
				"Show version information and exit", NULL },
	{ "index", 'i', 0, G_OPTION_ARG_INT, &option_index,
				"Use specified controller", "INDEX"},
	{ "debug", 'd', 0, G_OPTION_ARG_NONE, &option_dbg,
				"Enable debug logs", NULL},
	{ "mgmt-debug", 0, 0, G_OPTION_ARG_NONE, &option_mgmt_dbg,
				"Enable mgmt debug logs", NULL},

	{ NULL }
};

static void cleanup_services(void)
{
	int i;

	DBG("");

	for (i = HAL_SERVICE_ID_MAX; i > HAL_SERVICE_ID_CORE; i--)
		unregister_service(i);
}

static bool set_capabilities(void)
{
#if defined(ANDROID)
	struct __user_cap_header_struct header;
	struct __user_cap_data_struct cap;

	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = 0;

	/*
	 * CAP_NET_ADMIN: Allow use of MGMT interface
	 * CAP_NET_BIND_SERVICE: Allow use of privileged PSM
	 * CAP_NET_RAW: Allow use of bnep ioctl calls
	 */
	cap.effective = cap.permitted =
		CAP_TO_MASK(CAP_NET_RAW) |
		CAP_TO_MASK(CAP_NET_ADMIN) |
		CAP_TO_MASK(CAP_NET_BIND_SERVICE);
	cap.inheritable = 0;

	/* don't clear capabilities when dropping root */
	if (prctl(PR_SET_KEEPCAPS, 1) < 0) {
		error("%s: prctl(): %s", __func__, strerror(errno));
		return false;
	}

	/* Android bluetooth user UID=1002 */
	if (setuid(1002) < 0) {
		error("%s: setuid(): %s", __func__, strerror(errno));
		return false;
	}

	/* TODO: Move to cap_set_proc once bionic support it */
	if (capset(&header, &cap) < 0) {
		error("%s: capset(): %s", __func__, strerror(errno));
		return false;
	}

	/* TODO: Move to cap_get_proc once bionic support it */
	if (capget(&header, &cap) < 0) {
		error("%s: capget(): %s", __func__, strerror(errno));
		return false;
	}

	DBG("Caps: eff: 0x%x, perm: 0x%x, inh: 0x%x", cap.effective,
					cap.permitted, cap.inheritable);

#endif
	return true;
}

static void set_version(void)
{
	uint8_t major, minor;

	if (sscanf(VERSION, "%hhu.%hhu", &major, &minor) != 2)
		return;

	config_pnp_version = major << 8 | minor;
}

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *err = NULL;
	guint signal;

	set_version();

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (g_option_context_parse(context, &argc, &argv, &err) == FALSE) {
		if (err != NULL) {
			g_printerr("%s\n", err->message);
			g_error_free(err);
		} else
			g_printerr("An unknown error occurred\n");

		exit(EXIT_FAILURE);
	}

	g_option_context_free(context);

	if (option_version == TRUE) {
		printf("%s\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	signal = setup_signalfd();
	if (!signal)
		return EXIT_FAILURE;

	if (option_dbg || option_mgmt_dbg)
		__btd_log_init("*", 0);
	else
		__btd_log_init(NULL, 0);

	if (!set_capabilities()) {
		__btd_log_cleanup();
		g_source_remove(signal);
		return EXIT_FAILURE;
	}

	quit_timeout = g_timeout_add_seconds(STARTUP_GRACE_SECONDS,
							quit_eventloop, NULL);
	if (quit_timeout == 0) {
		error("Failed to init startup timeout");
		__btd_log_cleanup();
		g_source_remove(signal);
		return EXIT_FAILURE;
	}

	if (!bt_bluetooth_start(option_index, option_mgmt_dbg, adapter_ready)) {
		__btd_log_cleanup();
		g_source_remove(quit_timeout);
		g_source_remove(signal);
		return EXIT_FAILURE;
	}

	/* Use params: mtu = 0, flags = 0 */
	start_sdp_server(0, 0);

	DBG("Entering main loop");

	event_loop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(event_loop);

	g_source_remove(signal);

	if (quit_timeout > 0)
		g_source_remove(quit_timeout);

	cleanup_services();

	stop_sdp_server();
	bt_bluetooth_cleanup();
	g_main_loop_unref(event_loop);

	/* If no adapter was initialized, hal_ipc is NULL */
	if (hal_ipc) {
		ipc_unregister(hal_ipc, HAL_SERVICE_ID_CORE);
		ipc_cleanup(hal_ipc);
	}

	info("Exit");

	__btd_log_cleanup();

	free(config_vendor);
	free(config_model);
	free(config_name);
	free(config_serial);
	free(config_fw_rev);
	free(config_hw_rev);

	return EXIT_SUCCESS;
}
