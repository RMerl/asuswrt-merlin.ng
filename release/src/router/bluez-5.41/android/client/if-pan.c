/*
 * Copyright (C) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "if-main.h"
#include "../hal-utils.h"

const btpan_interface_t *if_pan = NULL;

typedef int btpan_role_t;

SINTMAP(btpan_role_t, -1, "(unknown)")
	DELEMENT(BTPAN_ROLE_NONE),
	DELEMENT(BTPAN_ROLE_PANNAP),
	DELEMENT(BTPAN_ROLE_PANU),
ENDMAP

SINTMAP(btpan_connection_state_t, -1, "(unknown)")
	DELEMENT(BTPAN_STATE_CONNECTED),
	DELEMENT(BTPAN_STATE_CONNECTING),
	DELEMENT(BTPAN_STATE_DISCONNECTED),
	DELEMENT(BTPAN_STATE_DISCONNECTING),
ENDMAP

SINTMAP(btpan_control_state_t, -1, "(unknown)")
	DELEMENT(BTPAN_STATE_ENABLED),
	DELEMENT(BTPAN_STATE_DISABLED),
ENDMAP

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void control_state_cb(btpan_control_state_t state, int local_role,
					bt_status_t error, const char *ifname)
#else
static void control_state_cb(btpan_control_state_t state, bt_status_t error,
					int local_role, const char *ifname)
#endif
{
	haltest_info("%s: state=%s error=%s local_role=%s ifname=%s\n",
			__func__, btpan_control_state_t2str(state),
			bt_status_t2str(error), btpan_role_t2str(local_role),
			ifname);
}

static char last_used_addr[MAX_ADDR_STR_LEN];

static void connection_state_cb(btpan_connection_state_t state,
				bt_status_t error, const bt_bdaddr_t *bd_addr,
				int local_role, int remote_role)
{
	haltest_info("%s: state=%s error=%s bd_addr=%s local_role=%s remote_role=%s\n",
			__func__, btpan_connection_state_t2str(state),
			bt_status_t2str(error),
			bt_bdaddr_t2str(bd_addr, last_used_addr),
			btpan_role_t2str(local_role),
			btpan_role_t2str(remote_role));
}

static btpan_callbacks_t pan_cbacks = {
	.size = sizeof(pan_cbacks),
	.control_state_cb = control_state_cb,
	.connection_state_cb = connection_state_cb
};

static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_pan);

	EXEC(if_pan->init, &pan_cbacks);
}

/* enable */

static void enable_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(btpan_role_t);
		*enum_func = enum_defines;
	}
}

static void enable_p(int argc, const char **argv)
{
	int local_role;

	RETURN_IF_NULL(if_pan);

	/* local role */
	if (argc < 3) {
		haltest_error("No local mode specified\n");
		return;
	}
	local_role = str2btpan_role_t(argv[2]);
	if (local_role == -1)
		local_role = atoi(argv[2]);

	EXEC(if_pan->enable, local_role);
}

/* get_local_role */

static void get_local_role_p(int argc, const char **argv)
{
	int local_role;

	RETURN_IF_NULL(if_pan);

	local_role = if_pan->get_local_role();
	haltest_info("local_role: %s\n", btpan_role_t2str(local_role));
}

/* connect */

static void connect_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = NULL;
		*enum_func = enum_devices;
	} else if (argc == 4 || argc == 5) {
		*user = TYPE_ENUM(btpan_role_t);
		*enum_func = enum_defines;
	}
}

static void connect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;
	int local_role;
	int remote_role;

	RETURN_IF_NULL(if_pan);
	VERIFY_ADDR_ARG(2, &addr);

	/* local role */
	if (argc < 4) {
		haltest_error("No local mode specified\n");
		return;
	}
	local_role = str2btpan_role_t(argv[3]);
	if (local_role == -1)
		local_role = atoi(argv[3]);

	/* remote role */
	if (argc < 5) {
		haltest_error("No remote mode specified\n");
		return;
	}
	remote_role = str2btpan_role_t(argv[4]);
	if (remote_role == -1)
		remote_role = atoi(argv[4]);

	EXEC(if_pan->connect, &addr, local_role, remote_role);
}

/* disconnect */

static void disconnect_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = last_used_addr;
		*enum_func = enum_one_string;
	}
}

static void disconnect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_pan);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_pan->disconnect, &addr);
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_pan);

	EXECV(if_pan->cleanup);
	if_pan = NULL;
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODCH(connect, "<addr> <local_role> <remote_role>"),
	STD_METHODCH(enable, "<local_role>"),
	STD_METHOD(get_local_role),
	STD_METHODCH(disconnect, "<addr>"),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface pan_if = {
	.name = "pan",
	.methods = methods
};
