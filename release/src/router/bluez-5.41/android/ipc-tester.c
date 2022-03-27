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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <libgen.h>
#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"

#include "src/shared/tester.h"
#include "src/shared/mgmt.h"
#include "emulator/hciemu.h"

#include "hal-msg.h"
#include "ipc-common.h"

#include <cutils/properties.h>

#define WAIT_FOR_SIGNAL_TIME 2 /* in seconds */
#define EMULATOR_SIGNAL "emulator_started"

struct test_data {
	struct mgmt *mgmt;
	uint16_t mgmt_index;
	struct hciemu *hciemu;
	enum hciemu_type hciemu_type;
	pid_t bluetoothd_pid;
	bool setup_done;
};

struct ipc_data {
	void *buffer;
	size_t len;
};

struct generic_data {
	struct ipc_data ipc_data;

	unsigned int num_services;
	int init_services[];
};

struct regmod_msg {
	struct ipc_hdr header;
	struct hal_cmd_register_module cmd;
} __attribute__((packed));

#define CONNECT_TIMEOUT (5 * 1000)
#define SERVICE_NAME "bluetoothd"

static char exec_dir[PATH_MAX];

static int cmd_sk = -1;
static int notif_sk = -1;

static void read_info_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct mgmt_rp_read_info *rp = param;
	char addr[18];
	uint16_t manufacturer;
	uint32_t supported_settings, current_settings;

	tester_print("Read Info callback");
	tester_print("  Status: 0x%02x", status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	ba2str(&rp->bdaddr, addr);
	manufacturer = btohs(rp->manufacturer);
	supported_settings = btohl(rp->supported_settings);
	current_settings = btohl(rp->current_settings);

	tester_print("  Address: %s", addr);
	tester_print("  Version: 0x%02x", rp->version);
	tester_print("  Manufacturer: 0x%04x", manufacturer);
	tester_print("  Supported settings: 0x%08x", supported_settings);
	tester_print("  Current settings: 0x%08x", current_settings);
	tester_print("  Class: 0x%02x%02x%02x",
			rp->dev_class[2], rp->dev_class[1], rp->dev_class[0]);
	tester_print("  Name: %s", rp->name);
	tester_print("  Short name: %s", rp->short_name);

	if (strcmp(hciemu_get_address(data->hciemu), addr)) {
		tester_pre_setup_failed();
		return;
	}

	tester_pre_setup_complete();
}

static void index_added_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Index Added callback");
	tester_print("  Index: 0x%04x", index);

	data->mgmt_index = index;

	mgmt_send(data->mgmt, MGMT_OP_READ_INFO, data->mgmt_index, 0, NULL,
					read_info_callback, NULL, NULL);
}

static void index_removed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Index Removed callback");
	tester_print("  Index: 0x%04x", index);

	if (index != data->mgmt_index)
		return;

	mgmt_unregister_index(data->mgmt, data->mgmt_index);

	mgmt_unref(data->mgmt);
	data->mgmt = NULL;

	tester_post_teardown_complete();
}

static void read_index_list_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Read Index List callback");
	tester_print("  Status: 0x%02x", status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	mgmt_register(data->mgmt, MGMT_EV_INDEX_ADDED, MGMT_INDEX_NONE,
					index_added_callback, NULL, NULL);

	mgmt_register(data->mgmt, MGMT_EV_INDEX_REMOVED, MGMT_INDEX_NONE,
					index_removed_callback, NULL, NULL);

	data->hciemu = hciemu_new(data->hciemu_type);
	if (!data->hciemu) {
		tester_warn("Failed to setup HCI emulation");
		tester_pre_setup_failed();
		return;
	}

	tester_print("New hciemu instance created");
}

static void test_pre_setup(const void *data)
{
	struct test_data *test_data = tester_get_data();

	if (!tester_use_debug())
		fclose(stderr);

	test_data->mgmt = mgmt_new_default();
	if (!test_data->mgmt) {
		tester_warn("Failed to setup management interface");
		tester_pre_setup_failed();
		return;
	}

	mgmt_send(test_data->mgmt, MGMT_OP_READ_INDEX_LIST, MGMT_INDEX_NONE, 0,
				NULL, read_index_list_callback, NULL, NULL);
}

static void test_post_teardown(const void *data)
{
	struct test_data *test_data = tester_get_data();

	if (test_data->hciemu) {
		hciemu_unref(test_data->hciemu);
		test_data->hciemu = NULL;
	}
}

static void bluetoothd_start(int hci_index)
{
	char prg_name[PATH_MAX];
	char index[8];
	char *prg_argv[4];

	snprintf(prg_name, sizeof(prg_name), "%s/%s", exec_dir, "bluetoothd");
	snprintf(index, sizeof(index), "%d", hci_index);

	prg_argv[0] = prg_name;
	prg_argv[1] = "-i";
	prg_argv[2] = index;
	prg_argv[3] = NULL;

	if (!tester_use_debug())
		fclose(stderr);

	execve(prg_argv[0], prg_argv, NULL);
}

static void emulator(int pipe, int hci_index)
{
	static const char SYSTEM_SOCKET_PATH[] = "\0android_system";
	char buf[1024];
	struct sockaddr_un addr;
	struct timeval tv;
	int fd;
	ssize_t len;

	fd = socket(PF_LOCAL, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0)
		goto failed;

	tv.tv_sec = WAIT_FOR_SIGNAL_TIME;
	tv.tv_usec = 0;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, SYSTEM_SOCKET_PATH, sizeof(SYSTEM_SOCKET_PATH));

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind system socket");
		goto failed;
	}

	len = write(pipe, EMULATOR_SIGNAL, sizeof(EMULATOR_SIGNAL));

	if (len != sizeof(EMULATOR_SIGNAL))
		goto failed;

	memset(buf, 0, sizeof(buf));

	len = read(fd, buf, sizeof(buf));
	if (len <= 0 || strcmp(buf, "ctl.start=bluetoothd"))
		goto failed;

	close(pipe);
	close(fd);
	return bluetoothd_start(hci_index);

failed:
	close(pipe);
	if (fd >= 0)
		close(fd);
}

static int accept_connection(int sk)
{
	int err;
	struct pollfd pfd;
	int new_sk;

	memset(&pfd, 0 , sizeof(pfd));
	pfd.fd = sk;
	pfd.events = POLLIN;

	err = poll(&pfd, 1, CONNECT_TIMEOUT);
	if (err < 0) {
		err = errno;
		tester_warn("Failed to poll: %d (%s)", err, strerror(err));
		return -errno;
	}

	if (err == 0) {
		tester_warn("bluetoothd connect timeout");
		return -errno;
	}

	new_sk = accept(sk, NULL, NULL);
	if (new_sk < 0) {
		err = errno;
		tester_warn("Failed to accept socket: %d (%s)",
							err, strerror(err));
		return -errno;
	}

	return new_sk;
}

static bool init_ipc(void)
{
	struct sockaddr_un addr;

	int sk;
	int err;

	sk = socket(AF_LOCAL, SOCK_SEQPACKET, 0);
	if (sk < 0) {
		err = errno;
		tester_warn("Failed to create socket: %d (%s)", err,
							strerror(err));
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	memcpy(addr.sun_path, BLUEZ_HAL_SK_PATH, sizeof(BLUEZ_HAL_SK_PATH));

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		err = errno;
		tester_warn("Failed to bind socket: %d (%s)", err,
								strerror(err));
		close(sk);
		return false;
	}

	if (listen(sk, 2) < 0) {
		err = errno;
		tester_warn("Failed to listen on socket: %d (%s)", err,
								strerror(err));
		close(sk);
		return false;
	}

	/* Start Android Bluetooth daemon service */
	if (property_set("ctl.start", SERVICE_NAME) < 0) {
		tester_warn("Failed to start service %s", SERVICE_NAME);
		close(sk);
		return false;
	}

	cmd_sk = accept_connection(sk);
	if (cmd_sk < 0) {
		close(sk);
		return false;
	}

	notif_sk = accept_connection(sk);
	if (notif_sk < 0) {
		close(sk);
		close(cmd_sk);
		cmd_sk = -1;
		return false;
	}

	tester_print("bluetoothd connected");

	close(sk);

	return true;
}

static void cleanup_ipc(void)
{
	if (cmd_sk < 0)
		return;

	close(cmd_sk);
	cmd_sk = -1;
}

static gboolean check_for_daemon(gpointer user_data)
{
	int status;
	struct test_data *data = user_data;

	if ((waitpid(data->bluetoothd_pid, &status, WNOHANG))
							!= data->bluetoothd_pid)
		return true;

	if (data->setup_done) {
		if (WIFEXITED(status) &&
				(WEXITSTATUS(status) == EXIT_SUCCESS)) {
			tester_test_passed();
			return false;
		}
		tester_test_failed();
	} else {
		tester_setup_failed();
		test_post_teardown(data);
	}

	tester_warn("Unexpected Daemon shutdown with status %d", status);
	return false;
}

static bool setup_module(int service_id)
{
	struct ipc_hdr response;
	struct ipc_hdr expected_response;

	struct regmod_msg btmodule_msg = {
		.header = {
			.service_id = HAL_SERVICE_ID_CORE,
			.opcode = HAL_OP_REGISTER_MODULE,
			.len = sizeof(struct hal_cmd_register_module),
			},
		.cmd = {
			.service_id = service_id,
			.mode = HAL_MODE_DEFAULT,
			.max_clients = 1,
			},
	};

	if (write(cmd_sk, &btmodule_msg, sizeof(btmodule_msg)) < 0)
		goto fail;

	if (read(cmd_sk, &response, sizeof(response)) < 0)
		goto fail;

	expected_response = btmodule_msg.header;
	expected_response.len = 0;

	if (memcmp(&response, &expected_response, sizeof(response)) == 0)
		return true;

fail:
	tester_warn("Module registration failed.");
	return false;
}

static void setup(const void *data)
{
	const struct generic_data *generic_data = data;
	struct test_data *test_data = tester_get_data();
	int signal_fd[2];
	char buf[1024];
	pid_t pid;
	int len;
	unsigned int i;

	if (pipe(signal_fd))
		goto failed;

	pid = fork();

	if (pid < 0) {
		close(signal_fd[0]);
		close(signal_fd[1]);
		goto failed;
	}

	if (pid == 0) {
		if (!tester_use_debug())
			fclose(stderr);

		close(signal_fd[0]);
		emulator(signal_fd[1], test_data->mgmt_index);
		exit(0);
	}

	close(signal_fd[1]);
	test_data->bluetoothd_pid = pid;

	len = read(signal_fd[0], buf, sizeof(buf));
	if (len <= 0 || (strcmp(buf, EMULATOR_SIGNAL))) {
		close(signal_fd[0]);
		goto failed;
	}

	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, check_for_daemon, test_data,
									NULL);

	if (!init_ipc()) {
		tester_warn("Cannot initialize IPC mechanism!");
		goto failed;
	}
	tester_print("Will init %d services.", generic_data->num_services);

	for (i = 0; i < generic_data->num_services; i++)
		if (!setup_module(generic_data->init_services[i])) {
			cleanup_ipc();
			goto failed;
		}

	test_data->setup_done = true;

	tester_setup_complete();
	return;

failed:
	g_idle_remove_by_data(test_data);
	tester_setup_failed();
	test_post_teardown(data);
}

static void teardown(const void *data)
{
	struct test_data *test_data = tester_get_data();

	g_idle_remove_by_data(test_data);
	cleanup_ipc();

	if (test_data->bluetoothd_pid)
		waitpid(test_data->bluetoothd_pid, NULL, 0);

	tester_teardown_complete();
}

static void ipc_send_tc(const void *data)
{
	const struct generic_data *generic_data = data;
	const struct ipc_data *ipc_data = &generic_data->ipc_data;

	if (ipc_data->len) {
		if (write(cmd_sk, ipc_data->buffer, ipc_data->len) < 0)
			tester_test_failed();
	}
}

#define service_data(args...) { args }

#define gen_data(writelen, writebuf, servicelist...) \
	{								\
		.ipc_data = {						\
			.buffer = writebuf,				\
			.len = writelen,				\
		},							\
		.init_services = service_data(servicelist),		\
		.num_services = sizeof((const int[])			\
					service_data(servicelist)) /	\
					sizeof(int),			\
	}

#define test_generic(name, test, setup, teardown, buffer, writelen, \
							services...) \
	do {								\
		struct test_data *user;					\
		static const struct generic_data data =			\
				gen_data(writelen, buffer, services);	\
		user = g_malloc0(sizeof(struct test_data));		\
		if (!user)						\
			break;						\
		user->hciemu_type = HCIEMU_TYPE_BREDRLE;		\
		tester_add_full(name, &data, test_pre_setup, setup,	\
				test, teardown, test_post_teardown,	\
				3, user, g_free);			\
	} while (0)

#define test_opcode_valid(_name, _service, _opcode, _len, _servicelist...) \
	do {								\
		static struct ipc_hdr hdr = {				\
			.service_id = _service,				\
			.opcode = _opcode,				\
			.len = _len,					\
		};							\
									\
		test_generic("Opcode out of range: "_name,		\
				ipc_send_tc, setup, teardown,		\
				&hdr,					\
				sizeof(hdr),				\
				_servicelist);				\
	} while (0)

struct vardata {
	struct ipc_hdr hdr;
	uint8_t buf[IPC_MTU];
} __attribute__((packed));

#define test_datasize_valid(_name, _service, _opcode, _hlen, _addatasize, \
							_servicelist...) \
	do {								\
		static struct vardata vdata = {				\
			.hdr.service_id = _service,			\
			.hdr.opcode = _opcode,				\
			.hdr.len = (_hlen) + (_addatasize),		\
			.buf = {},					\
		};							\
		test_generic("Data size "_name,				\
				ipc_send_tc, setup, teardown,		\
				&vdata,					\
				sizeof(vdata.hdr) + (_hlen) + (_addatasize),\
				_servicelist);				\
	} while (0)

static struct regmod_msg register_bt_msg = {
	.header = {
		.service_id = HAL_SERVICE_ID_CORE,
		.opcode = HAL_OP_REGISTER_MODULE,
		.len = sizeof(struct hal_cmd_register_module),
		},
	.cmd = {
		.service_id = HAL_SERVICE_ID_BLUETOOTH,
		},
};

static struct regmod_msg register_bt_malformed_size_msg = {
	.header = {
		.service_id = HAL_SERVICE_ID_CORE,
		.opcode = HAL_OP_REGISTER_MODULE,
		/* wrong payload size declared */
		.len = sizeof(struct hal_cmd_register_module) - 1,
		},
	.cmd = {
		.service_id = HAL_SERVICE_ID_CORE,
		},
};

struct malformed_data3_struct {
	struct regmod_msg valid_msg;
	int redundant_data;
}  __attribute__((packed));

static struct malformed_data3_struct malformed_data3_msg = {
	/* valid register service message */
	.valid_msg = {
		.header = {
			.service_id = HAL_SERVICE_ID_CORE,
			.opcode = HAL_OP_REGISTER_MODULE,
			.len = sizeof(struct hal_cmd_register_module),
			},
		.cmd = {
			.service_id = HAL_SERVICE_ID_CORE,
			},
	},
	/* plus redundant data */
	. redundant_data = 666,
};

static struct ipc_hdr enable_unknown_service_hdr = {
	.service_id = HAL_SERVICE_ID_MAX + 1,
	.opcode = HAL_OP_REGISTER_MODULE,
	.len = 0,
};

static struct ipc_hdr enable_bt_service_hdr = {
	.service_id = HAL_SERVICE_ID_BLUETOOTH,
	.opcode = HAL_OP_ENABLE,
	.len = 0,
};

struct bt_set_adapter_prop_data {
	struct ipc_hdr hdr;
	struct hal_cmd_set_adapter_prop prop;

	/* data placeholder for hal_cmd_set_adapter_prop.val[0] */
	uint8_t buf[IPC_MTU - sizeof(struct ipc_hdr) -
				sizeof(struct hal_cmd_set_adapter_prop)];
} __attribute__((packed));

#define set_name "new name"

static struct bt_set_adapter_prop_data bt_set_adapter_prop_data_overs = {
	.hdr.service_id = HAL_SERVICE_ID_BLUETOOTH,
	.hdr.opcode = HAL_OP_SET_ADAPTER_PROP,
	.hdr.len = sizeof(struct hal_cmd_set_adapter_prop) + sizeof(set_name),

	.prop.type = HAL_PROP_ADAPTER_NAME,
	/* declare wrong descriptor length */
	.prop.len = sizeof(set_name) + 1,
	/* init prop.val[0] */
	.buf = set_name,
};

static struct bt_set_adapter_prop_data bt_set_adapter_prop_data_unders = {
	.hdr.service_id = HAL_SERVICE_ID_BLUETOOTH,
	.hdr.opcode = HAL_OP_SET_ADAPTER_PROP,
	.hdr.len = sizeof(struct hal_cmd_set_adapter_prop) + sizeof(set_name),

	.prop.type = HAL_PROP_ADAPTER_NAME,
	/* declare wrong descriptor length */
	.prop.len = sizeof(set_name) - 1,
	/* init prop.val[0] */
	.buf = set_name,
};

struct bt_set_remote_prop_data {
	struct ipc_hdr hdr;
	struct hal_cmd_set_remote_device_prop prop;

	/* data placeholder for hal_cmd_set_remote_device_prop.val[0] */
	uint8_t buf[IPC_MTU - sizeof(struct ipc_hdr) -
				sizeof(struct hal_cmd_set_remote_device_prop)];
} __attribute__((packed));

static struct bt_set_remote_prop_data bt_set_remote_prop_data_overs = {
	.hdr.service_id = HAL_SERVICE_ID_BLUETOOTH,
	.hdr.opcode = HAL_OP_SET_REMOTE_DEVICE_PROP,
	.hdr.len = sizeof(struct hal_cmd_set_remote_device_prop) +
							sizeof(set_name),

	.prop.bdaddr = {},
	.prop.type = HAL_PROP_DEVICE_NAME,
	/* declare wrong descriptor length */
	.prop.len = sizeof(set_name) + 1,
	.buf = set_name,
};

static struct bt_set_remote_prop_data bt_set_remote_prop_data_unders = {
	.hdr.service_id = HAL_SERVICE_ID_BLUETOOTH,
	.hdr.opcode = HAL_OP_SET_REMOTE_DEVICE_PROP,
	.hdr.len = sizeof(struct hal_cmd_set_remote_device_prop) +
						sizeof(set_name),

	.prop.bdaddr = {},
	.prop.type = HAL_PROP_DEVICE_NAME,
	/* declare wrong descriptor length */
	.prop.len = sizeof(set_name) - 1,
	.buf = set_name,
};

struct hidhost_set_info_data {
	struct ipc_hdr hdr;
	struct hal_cmd_hidhost_set_info info;

	/* data placeholder for hal_cmd_hidhost_set_info.descr[0] field */
	uint8_t buf[IPC_MTU - sizeof(struct ipc_hdr) -
				sizeof(struct hal_cmd_hidhost_set_info)];
} __attribute__((packed));

#define set_info_data "some descriptor"

static struct hidhost_set_info_data hidhost_set_info_data_overs = {
	.hdr.service_id = HAL_SERVICE_ID_HIDHOST,
	.hdr.opcode = HAL_OP_HIDHOST_SET_INFO,
	.hdr.len = sizeof(struct hal_cmd_hidhost_set_info) +
							sizeof(set_info_data),

	/* declare wrong descriptor length */
	.info.descr_len = sizeof(set_info_data) + 1,
	/* init .info.descr[0] */
	.buf = set_info_data,
};

static struct hidhost_set_info_data hidhost_set_info_data_unders = {
	.hdr.service_id = HAL_SERVICE_ID_HIDHOST,
	.hdr.opcode = HAL_OP_HIDHOST_SET_INFO,
	.hdr.len = sizeof(struct hal_cmd_hidhost_set_info) +
							sizeof(set_info_data),

	/* declare wrong descriptor length */
	.info.descr_len = sizeof(set_info_data) - 1,
	/* init .info.descr[0] */
	.buf = set_info_data,
};

struct hidhost_set_report_data {
	struct ipc_hdr hdr;
	struct hal_cmd_hidhost_set_report report;

	/* data placeholder for hal_cmd_hidhost_set_report.data[0] field */
	uint8_t buf[IPC_MTU - sizeof(struct ipc_hdr) -
				sizeof(struct hal_cmd_hidhost_set_report)];
} __attribute__((packed));

#define set_rep_data "1234567890"

static struct hidhost_set_report_data hidhost_set_report_data_overs = {
	.hdr.service_id = HAL_SERVICE_ID_HIDHOST,
	.hdr.opcode = HAL_OP_HIDHOST_SET_REPORT,
	.hdr.len = sizeof(struct hal_cmd_hidhost_set_report) +
							sizeof(set_rep_data),

	/* declare wrong descriptor length */
	.report.len = sizeof(set_rep_data) + 1,
	/* init report.data[0] */
	.buf = set_rep_data,
};

static struct hidhost_set_report_data hidhost_set_report_data_unders = {
	.hdr.service_id = HAL_SERVICE_ID_HIDHOST,
	.hdr.opcode = HAL_OP_HIDHOST_SET_REPORT,
	.hdr.len = sizeof(struct hal_cmd_hidhost_set_report) +
							sizeof(set_rep_data),

	/* declare wrong descriptor length */
	.report.len = sizeof(set_rep_data) - 1,
	/* init report.data[0] */
	.buf = set_rep_data,
};

struct hidhost_send_data_data {
	struct ipc_hdr hdr;
	struct hal_cmd_hidhost_send_data hiddata;

	/* data placeholder for hal_cmd_hidhost_send_data.data[0] field */
	uint8_t buf[IPC_MTU - sizeof(struct ipc_hdr) -
				sizeof(struct hal_cmd_hidhost_send_data)];
} __attribute__((packed));

#define send_data_data "1234567890"

static struct hidhost_send_data_data hidhost_send_data_overs = {
	.hdr.service_id = HAL_SERVICE_ID_HIDHOST,
	.hdr.opcode = HAL_OP_HIDHOST_SEND_DATA,
	.hdr.len = sizeof(struct hal_cmd_hidhost_send_data) +
							sizeof(send_data_data),

	/* declare wrong descriptor length */
	.hiddata.len = sizeof(send_data_data) + 1,
	/* init .hiddata.data[0] */
	.buf = send_data_data,
};

static struct hidhost_send_data_data hidhost_send_data_unders = {
	.hdr.service_id = HAL_SERVICE_ID_HIDHOST,
	.hdr.opcode = HAL_OP_HIDHOST_SEND_DATA,
	.hdr.len = sizeof(struct hal_cmd_hidhost_send_data) +
							sizeof(send_data_data),

	/* declare wrong descriptor length */
	.hiddata.len = sizeof(send_data_data) - 1,
	/* init .hiddata.data[0] */
	.buf = send_data_data,
};

#define hfp_number "#1234567890"

struct hfp_dial_data {
	struct ipc_hdr hdr;
	struct hal_cmd_hf_client_dial data;

	uint8_t buf[IPC_MTU - sizeof(struct ipc_hdr) -
				sizeof(struct hal_cmd_hf_client_dial)];
} __attribute__((packed));

static struct hfp_dial_data hfp_dial_overs = {
	.hdr.service_id = HAL_SERVICE_ID_HANDSFREE_CLIENT,
	.hdr.opcode = HAL_OP_HF_CLIENT_DIAL,
	.hdr.len = sizeof(struct hal_cmd_hf_client_dial) + sizeof(hfp_number),

	.data.number_len = sizeof(hfp_number) + 1,
	.buf = hfp_number,
};

static struct hfp_dial_data hfp_dial_unders = {
	.hdr.service_id = HAL_SERVICE_ID_HANDSFREE_CLIENT,
	.hdr.opcode = HAL_OP_HF_CLIENT_DIAL,
	.hdr.len = sizeof(struct hal_cmd_hf_client_dial) + sizeof(hfp_number),

	.data.number_len = sizeof(hfp_number) - 1,
	.buf = hfp_number,
};

int main(int argc, char *argv[])
{
	snprintf(exec_dir, sizeof(exec_dir), "%s", dirname(argv[0]));

	tester_init(&argc, &argv);

	/* check general IPC errors */
	test_generic("Too small data",
				ipc_send_tc, setup, teardown,
				&register_bt_msg, 1);

	test_generic("Malformed data (wrong payload declared)",
				ipc_send_tc, setup, teardown,
				&register_bt_malformed_size_msg,
				sizeof(register_bt_malformed_size_msg),
				HAL_SERVICE_ID_BLUETOOTH);

	test_generic("Malformed data2 (undersized msg)",
				ipc_send_tc, setup, teardown,
				&register_bt_msg,
				sizeof(register_bt_msg) - 1,
				HAL_SERVICE_ID_BLUETOOTH);

	test_generic("Malformed data3 (oversized msg)",
				ipc_send_tc, setup, teardown,
				&malformed_data3_msg,
				sizeof(malformed_data3_msg),
				HAL_SERVICE_ID_BLUETOOTH);

	test_generic("Invalid service",
				ipc_send_tc, setup, teardown,
				&enable_unknown_service_hdr,
				sizeof(enable_unknown_service_hdr),
				HAL_SERVICE_ID_BLUETOOTH);

	test_generic("Enable unregistered service",
				ipc_send_tc, setup, teardown,
				&enable_bt_service_hdr,
				sizeof(enable_bt_service_hdr));

	/* check service handler's max opcode value */
	test_opcode_valid("CORE", HAL_SERVICE_ID_CORE, 0x03, 0);

	test_opcode_valid("BLUETOOTH", HAL_SERVICE_ID_BLUETOOTH, 0x15, 0,
			HAL_SERVICE_ID_BLUETOOTH);

	test_opcode_valid("SOCK", HAL_SERVICE_ID_SOCKET, 0x03, 0,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_SOCKET);

	test_opcode_valid("HIDHOST", HAL_SERVICE_ID_HIDHOST, 0x10, 0,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);

	test_opcode_valid("PAN", HAL_SERVICE_ID_PAN, 0x05, 0,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_PAN);

	test_opcode_valid("HANDSFREE", HAL_SERVICE_ID_HANDSFREE, 0x10, 0,
						HAL_SERVICE_ID_BLUETOOTH,
						HAL_SERVICE_ID_HANDSFREE);

	test_opcode_valid("A2DP", HAL_SERVICE_ID_A2DP, 0x03, 0,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_A2DP);

	test_opcode_valid("HEALTH", HAL_SERVICE_ID_HEALTH, 0x06, 0,
						HAL_SERVICE_ID_BLUETOOTH,
						HAL_SERVICE_ID_HEALTH);

	test_opcode_valid("AVRCP", HAL_SERVICE_ID_AVRCP, 0x0b, 0,
				HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_AVRCP);

	test_opcode_valid("GATT", HAL_SERVICE_ID_GATT, 0x24, 0,
				HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_GATT);

	test_opcode_valid("HF_CLIENT", HAL_SERVICE_ID_HANDSFREE_CLIENT, 0x10, 0,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);

	test_opcode_valid("MAP_CLIENT", HAL_SERVICE_ID_MAP_CLIENT, 0x01, 0,
						HAL_SERVICE_ID_BLUETOOTH,
						HAL_SERVICE_ID_MAP_CLIENT);

	/* check for valid data size */
	test_datasize_valid("CORE Register+", HAL_SERVICE_ID_CORE,
			HAL_OP_REGISTER_MODULE,
			sizeof(struct hal_cmd_register_module), 1);
	test_datasize_valid("CORE Register-", HAL_SERVICE_ID_CORE,
			HAL_OP_REGISTER_MODULE,
			sizeof(struct hal_cmd_register_module), -1);
	test_datasize_valid("CORE Unregister+", HAL_SERVICE_ID_CORE,
			HAL_OP_UNREGISTER_MODULE,
			sizeof(struct hal_cmd_unregister_module), 1);
	test_datasize_valid("CORE Unregister-", HAL_SERVICE_ID_CORE,
			HAL_OP_UNREGISTER_MODULE,
			sizeof(struct hal_cmd_unregister_module), -1);

	/* check for valid data size for BLUETOOTH */
	test_datasize_valid("BT Enable+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_ENABLE,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Disable+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_DISABLE,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Adapter Props+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_ADAPTER_PROPS,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Adapter Prop+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_ADAPTER_PROP,
			sizeof(struct hal_cmd_get_adapter_prop), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Adapter Prop-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_ADAPTER_PROP,
			sizeof(struct hal_cmd_get_adapter_prop), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Set Adapter Prop+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_SET_ADAPTER_PROP,
			sizeof(struct hal_cmd_set_adapter_prop), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Set Adapter Prop-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_SET_ADAPTER_PROP,
			sizeof(struct hal_cmd_set_adapter_prop), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_generic("Data size BT Set Adapter Prop Vardata+",
			ipc_send_tc, setup, teardown,
			&bt_set_adapter_prop_data_overs,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_set_adapter_prop) +
				sizeof(set_name)),
			HAL_SERVICE_ID_BLUETOOTH);
	test_generic("Data size BT Set Adapter Prop Vardata+",
			ipc_send_tc, setup, teardown,
			&bt_set_adapter_prop_data_unders,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_set_adapter_prop) +
				sizeof(set_name)),
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Remote Props+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_REMOTE_DEVICE_PROPS,
			sizeof(struct hal_cmd_get_remote_device_props), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Remote Props-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_REMOTE_DEVICE_PROPS,
			sizeof(struct hal_cmd_get_remote_device_props), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Remote Prop+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_REMOTE_DEVICE_PROP,
			sizeof(struct hal_cmd_get_remote_device_prop), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Remote Prop-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_REMOTE_DEVICE_PROP,
			sizeof(struct hal_cmd_get_remote_device_prop), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Set Remote Prop+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_SET_REMOTE_DEVICE_PROP,
			sizeof(struct hal_cmd_set_remote_device_prop), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Set Remote Prop-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_SET_REMOTE_DEVICE_PROP,
			sizeof(struct hal_cmd_set_remote_device_prop), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_generic("Data size BT Set Remote Prop Vardata+",
			ipc_send_tc, setup, teardown,
			&bt_set_remote_prop_data_overs,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_set_remote_device_prop) +
				sizeof(set_name)),
			HAL_SERVICE_ID_BLUETOOTH);
	test_generic("Data size BT Set Remote Prop Vardata-",
			ipc_send_tc, setup, teardown,
			&bt_set_remote_prop_data_unders,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_set_remote_device_prop) +
				sizeof(set_name)),
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Remote SV Rec+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_REMOTE_SERVICE_REC,
			sizeof(struct hal_cmd_get_remote_service_rec), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Remote SV Rec-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_REMOTE_SERVICE_REC,
			sizeof(struct hal_cmd_get_remote_service_rec), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Remote Services+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_REMOTE_SERVICES,
			sizeof(struct hal_cmd_get_remote_services), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Get Remote Services-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_GET_REMOTE_SERVICES,
			sizeof(struct hal_cmd_get_remote_services), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Start Discovery+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_START_DISCOVERY,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Cancel Discovery+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_CANCEL_DISCOVERY,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Create Bond+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_CREATE_BOND,
			sizeof(struct hal_cmd_create_bond), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Create Bond-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_CREATE_BOND,
			sizeof(struct hal_cmd_create_bond), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Remove Bond+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_REMOVE_BOND,
			sizeof(struct hal_cmd_remove_bond), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Remove Bond-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_REMOVE_BOND,
			sizeof(struct hal_cmd_remove_bond), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Cancel Bond+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_CANCEL_BOND,
			sizeof(struct hal_cmd_cancel_bond), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Cancel Bond-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_CANCEL_BOND,
			sizeof(struct hal_cmd_cancel_bond), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Pin Reply+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_PIN_REPLY,
			sizeof(struct hal_cmd_pin_reply), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT Pin Reply-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_PIN_REPLY,
			sizeof(struct hal_cmd_pin_reply), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT SSP Reply+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_SSP_REPLY,
			sizeof(struct hal_cmd_ssp_reply), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT SSP Reply-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_SSP_REPLY,
			sizeof(struct hal_cmd_ssp_reply), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT DUT Mode Conf+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_DUT_MODE_CONF,
			sizeof(struct hal_cmd_dut_mode_conf), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT DUT Mode Conf-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_DUT_MODE_CONF,
			sizeof(struct hal_cmd_dut_mode_conf), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT DUT Mode Send+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_DUT_MODE_SEND,
			sizeof(struct hal_cmd_dut_mode_send), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT DUT Mode Send-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_DUT_MODE_SEND,
			sizeof(struct hal_cmd_dut_mode_send), -1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT LE Test+", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_LE_TEST_MODE,
			sizeof(struct hal_cmd_le_test_mode), 1,
			HAL_SERVICE_ID_BLUETOOTH);
	test_datasize_valid("BT LE Test-", HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_LE_TEST_MODE,
			sizeof(struct hal_cmd_le_test_mode), -1,
			HAL_SERVICE_ID_BLUETOOTH);

	/* check for valid data size for SOCK */
	test_datasize_valid("SOCKET Listen+", HAL_SERVICE_ID_SOCKET,
			HAL_OP_SOCKET_LISTEN,
			sizeof(struct hal_cmd_socket_listen), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_SOCKET);
	test_datasize_valid("SOCKET Listen-", HAL_SERVICE_ID_SOCKET,
			HAL_OP_SOCKET_LISTEN,
			sizeof(struct hal_cmd_socket_listen), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_SOCKET);
	test_datasize_valid("SOCKET Connect+", HAL_SERVICE_ID_SOCKET,
			HAL_OP_SOCKET_CONNECT,
			sizeof(struct hal_cmd_socket_connect), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_SOCKET);
	test_datasize_valid("SOCKET Connect-", HAL_SERVICE_ID_SOCKET,
			HAL_OP_SOCKET_CONNECT,
			sizeof(struct hal_cmd_socket_connect), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_SOCKET);

	/* check for valid data size for HID Host */
	test_datasize_valid("HIDHOST Connect+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_CONNECT,
			sizeof(struct hal_cmd_hidhost_connect), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Connect-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_CONNECT,
			sizeof(struct hal_cmd_hidhost_connect), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Disconnect+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_DISCONNECT,
			sizeof(struct hal_cmd_hidhost_disconnect), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Disconnect-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_DISCONNECT,
			sizeof(struct hal_cmd_hidhost_disconnect), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Virt. Unplug+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_VIRTUAL_UNPLUG,
			sizeof(struct hal_cmd_hidhost_virtual_unplug), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Virt. Unplug-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_VIRTUAL_UNPLUG,
			sizeof(struct hal_cmd_hidhost_virtual_unplug), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Set Info+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_SET_INFO,
			sizeof(struct hal_cmd_hidhost_set_info), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Set Info-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_SET_INFO,
			sizeof(struct hal_cmd_hidhost_set_info), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_generic("Data size HIDHOST Set Info Vardata+",
			ipc_send_tc, setup, teardown,
			&hidhost_set_info_data_overs,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_hidhost_set_info) +
				sizeof(set_info_data)),
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_generic("Data size HIDHOST Set Info Vardata-",
			ipc_send_tc, setup, teardown,
			&hidhost_set_info_data_unders,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_hidhost_set_info) +
				sizeof(set_info_data)),
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Get Protocol+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_GET_PROTOCOL,
			sizeof(struct hal_cmd_hidhost_get_protocol), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Get Protocol-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_GET_PROTOCOL,
			sizeof(struct hal_cmd_hidhost_get_protocol), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Set Protocol+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_SET_PROTOCOL,
			sizeof(struct hal_cmd_hidhost_set_protocol), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Set Protocol-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_SET_PROTOCOL,
			sizeof(struct hal_cmd_hidhost_set_protocol), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Get Report+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_GET_REPORT,
			sizeof(struct hal_cmd_hidhost_get_report), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Get Report-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_GET_REPORT,
			sizeof(struct hal_cmd_hidhost_get_report), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Set Report+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_SET_REPORT,
			sizeof(struct hal_cmd_hidhost_set_report), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Set Report-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_SET_REPORT,
			sizeof(struct hal_cmd_hidhost_set_report), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_generic("Data size HIDHOST Set Report Vardata+",
			ipc_send_tc, setup, teardown,
			&hidhost_set_report_data_overs,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_hidhost_set_report) +
				sizeof(set_rep_data)),
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_generic("Data size HIDHOST Set Report Vardata-",
			ipc_send_tc, setup, teardown,
			&hidhost_set_report_data_unders,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_hidhost_set_report) +
				sizeof(set_rep_data)),
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Send Data+", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_SEND_DATA,
			sizeof(struct hal_cmd_hidhost_send_data), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_datasize_valid("HIDHOST Send Data-", HAL_SERVICE_ID_HIDHOST,
			HAL_OP_HIDHOST_SEND_DATA,
			sizeof(struct hal_cmd_hidhost_send_data), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_generic("Data size HIDHOST Send Vardata+",
			ipc_send_tc, setup, teardown,
			&hidhost_send_data_overs,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_hidhost_send_data) +
				sizeof(send_data_data)),
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);
	test_generic("Data size HIDHOST Send Vardata-",
			ipc_send_tc, setup, teardown,
			&hidhost_send_data_unders,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_hidhost_send_data) +
				sizeof(send_data_data)),
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_HIDHOST);

	/* check for valid data size for PAN */
	test_datasize_valid("PAN Enable+", HAL_SERVICE_ID_PAN,
			HAL_OP_PAN_ENABLE,
			sizeof(struct hal_cmd_pan_enable), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_PAN);
	test_datasize_valid("PAN Enable-", HAL_SERVICE_ID_PAN,
			HAL_OP_PAN_ENABLE,
			sizeof(struct hal_cmd_pan_enable), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_PAN);
	test_datasize_valid("PAN Get Role+", HAL_SERVICE_ID_PAN,
			HAL_OP_PAN_GET_ROLE,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_PAN);
	test_datasize_valid("PAN Connect+", HAL_SERVICE_ID_PAN,
			HAL_OP_PAN_CONNECT,
			sizeof(struct hal_cmd_pan_connect), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_PAN);
	test_datasize_valid("PAN Connect-", HAL_SERVICE_ID_PAN,
			HAL_OP_PAN_CONNECT,
			sizeof(struct hal_cmd_pan_connect), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_PAN);
	test_datasize_valid("PAN Disconnect+", HAL_SERVICE_ID_PAN,
			HAL_OP_PAN_DISCONNECT,
			sizeof(struct hal_cmd_pan_disconnect), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_PAN);
	test_datasize_valid("PAN Disconnect-", HAL_SERVICE_ID_PAN,
			HAL_OP_PAN_DISCONNECT,
			sizeof(struct hal_cmd_pan_disconnect), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_PAN);

	/* check for valid data size for A2DP */
	test_datasize_valid("A2DP Connect+", HAL_SERVICE_ID_A2DP,
			HAL_OP_A2DP_CONNECT,
			sizeof(struct hal_cmd_a2dp_connect), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_A2DP);
	test_datasize_valid("A2DP Connect-", HAL_SERVICE_ID_A2DP,
			HAL_OP_A2DP_CONNECT,
			sizeof(struct hal_cmd_a2dp_connect), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_A2DP);
	test_datasize_valid("A2DP Disconnect+", HAL_SERVICE_ID_A2DP,
			HAL_OP_A2DP_DISCONNECT,
			sizeof(struct hal_cmd_a2dp_disconnect), 1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_A2DP);
	test_datasize_valid("A2DP Disconnect-", HAL_SERVICE_ID_A2DP,
			HAL_OP_A2DP_DISCONNECT,
			sizeof(struct hal_cmd_a2dp_disconnect), -1,
			HAL_SERVICE_ID_BLUETOOTH, HAL_SERVICE_ID_A2DP);

	/* Check for valid data size for Handsfree Client */
	test_datasize_valid("HF_CLIENT Connect+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_CONNECT,
			sizeof(struct hal_cmd_hf_client_connect), 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Connect-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_CONNECT,
			sizeof(struct hal_cmd_hf_client_connect), -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Disconnect+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_DISCONNECT,
			sizeof(struct hal_cmd_hf_client_disconnect), 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Disconnect-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_DISCONNECT,
			sizeof(struct hal_cmd_hf_client_disconnect), -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Connect Audio+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_CONNECT_AUDIO,
			sizeof(struct hal_cmd_hf_client_connect_audio), 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Connect Audio-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_CONNECT_AUDIO,
			sizeof(struct hal_cmd_hf_client_connect_audio), -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Disconnect Audio+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_DISCONNECT_AUDIO,
			sizeof(struct hal_cmd_hf_client_disconnect_audio), 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Disconnect Audio-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_DISCONNECT_AUDIO,
			sizeof(struct hal_cmd_hf_client_disconnect_audio), -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Start VR+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_START_VR,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Start VR-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_START_VR,
			0, -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Stop VR+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_STOP_VR,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Stop VR-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_STOP_VR,
			0, -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Vol Contr.+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_VOLUME_CONTROL,
			sizeof(struct hal_cmd_hf_client_volume_control), 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Vol Contr.-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_VOLUME_CONTROL,
			sizeof(struct hal_cmd_hf_client_volume_control), -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_generic("Data size HF_CLIENT Dial Vardata+",
			ipc_send_tc, setup, teardown,
			&hfp_dial_overs,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_hf_client_dial) +
				sizeof(hfp_number)),
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_generic("Data size HF_CLIENT Dial Vardata-",
			ipc_send_tc, setup, teardown,
			&hfp_dial_unders,
			(sizeof(struct ipc_hdr) +
				sizeof(struct hal_cmd_hf_client_dial) +
				sizeof(hfp_number)),
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Dial Memory+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_DIAL_MEMORY,
			sizeof(struct hal_cmd_hf_client_dial_memory), 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Dial Memory-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_DIAL_MEMORY,
			sizeof(struct hal_cmd_hf_client_dial_memory), -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Call Action+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_CALL_ACTION,
			sizeof(struct hal_cmd_hf_client_call_action), 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Call Action-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_CALL_ACTION,
			sizeof(struct hal_cmd_hf_client_call_action), -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Query Current Calls+",
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_OP_HF_CLIENT_QUERY_CURRENT_CALLS,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Query Current Calls-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_QUERY_CURRENT_CALLS,
			0, -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Query Operator Name+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_QUERY_OPERATOR_NAME,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Query Operator Name-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_QUERY_OPERATOR_NAME,
			0, -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Retrieve Subscrb. Info+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_RETRIEVE_SUBSCR_INFO,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Retrieve Subscrb. Info-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_RETRIEVE_SUBSCR_INFO,
			0, -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Send DTMF+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_SEND_DTMF,
			sizeof(struct hal_cmd_hf_client_send_dtmf), 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Send DTMF-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_SEND_DTMF,
			sizeof(struct hal_cmd_hf_client_send_dtmf), -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Get Last Voice Tag+",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_GET_LAST_VOICE_TAG_NUM,
			0, 1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);
	test_datasize_valid("HF_CLIENT Get Last Voice Tag-",
			HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_GET_LAST_VOICE_TAG_NUM,
			0, -1,
			HAL_SERVICE_ID_BLUETOOTH,
			HAL_SERVICE_ID_HANDSFREE_CLIENT);

	/* check for valid data size for MAP CLIENT */
	test_datasize_valid("MAP CLIENT Get instances+",
				HAL_SERVICE_ID_MAP_CLIENT,
				HAL_OP_MAP_CLIENT_GET_INSTANCES,
				sizeof(struct hal_cmd_map_client_get_instances),
				1, HAL_SERVICE_ID_BLUETOOTH,
				HAL_SERVICE_ID_MAP_CLIENT);
	test_datasize_valid("MAP CLIENT Get instances-",
				HAL_SERVICE_ID_MAP_CLIENT,
				HAL_OP_MAP_CLIENT_GET_INSTANCES,
				sizeof(struct hal_cmd_map_client_get_instances),
				-1, HAL_SERVICE_ID_BLUETOOTH,
				HAL_SERVICE_ID_MAP_CLIENT);

	return tester_run();
}
