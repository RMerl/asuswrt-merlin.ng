/*
 * Copyright (C) 2014 Intel Corporation
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

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <hardware/bluetooth.h>
#include <hardware/bt_rc.h>

#include "if-main.h"
#include "pollhandler.h"
#include "../hal-utils.h"

const btrc_ctrl_interface_t *if_rc_ctrl = NULL;
static char last_addr[MAX_ADDR_STR_LEN];

static void passthrough_rsp_cb(int id, int key_state)
{
	haltest_info("%s: id=%d key_state=%d\n", __func__, id, key_state);
}

static void connection_state_cb(bool state, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: state=%s bd_addr=%s\n", __func__,
					state ? "true" : "false",
					bt_bdaddr_t2str(bd_addr, last_addr));
}

static btrc_ctrl_callbacks_t rc_ctrl_cbacks = {
	.size = sizeof(rc_ctrl_cbacks),
	.passthrough_rsp_cb = passthrough_rsp_cb,
	.connection_state_cb = connection_state_cb,
};

/* init */

static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_rc_ctrl);

	EXEC(if_rc_ctrl->init, &rc_ctrl_cbacks);
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_rc_ctrl);

	EXECV(if_rc_ctrl->cleanup);
	if_rc_ctrl = NULL;
}

/* send_pass_through_cmd */
static void send_pass_through_cmd_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
	if (argc == 3) {
		*user = NULL;
		*enum_func = enum_devices;
	}
}

static void send_pass_through_cmd_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;
	uint8_t key_code, key_state;

	RETURN_IF_NULL(if_rc);
	VERIFY_ADDR_ARG(2, &addr);

	if (argc < 4) {
		haltest_error("No key code specified\n");
		return;
	}

	key_code = (uint8_t) atoi(argv[3]);

	if (argc < 5) {
		haltest_error("No key state specified\n");
		return;
	}

	key_state = (uint8_t) atoi(argv[4]);

	EXEC(if_rc_ctrl->send_pass_through_cmd, &addr, key_code, key_state);
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODCH(send_pass_through_cmd, "<bd_addr> <key_code> <key_state>"),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface ctrl_rc_if = {
	.name = "rc-ctrl",
	.methods = methods
};
