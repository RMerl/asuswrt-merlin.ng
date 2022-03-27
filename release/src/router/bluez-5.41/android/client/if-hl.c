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
#include <unistd.h>
#include <string.h>

#include <hardware/bluetooth.h>
#include <hardware/bt_hl.h>

#include "if-main.h"
#include "pollhandler.h"
#include "../hal-utils.h"

SINTMAP(bthl_mdep_role_t, -1, "(unknown)")
	DELEMENT(BTHL_MDEP_ROLE_SOURCE),
	DELEMENT(BTHL_MDEP_ROLE_SINK),
ENDMAP

SINTMAP(bthl_channel_type_t, -1, "(unknown)")
	DELEMENT(BTHL_CHANNEL_TYPE_RELIABLE),
	DELEMENT(BTHL_CHANNEL_TYPE_STREAMING),
	DELEMENT(BTHL_CHANNEL_TYPE_ANY),
ENDMAP

SINTMAP(bthl_app_reg_state_t, -1, "(unknown)")
	DELEMENT(BTHL_APP_REG_STATE_REG_SUCCESS),
	DELEMENT(BTHL_APP_REG_STATE_REG_FAILED),
	DELEMENT(BTHL_APP_REG_STATE_DEREG_SUCCESS),
	DELEMENT(BTHL_APP_REG_STATE_DEREG_FAILED),
ENDMAP

SINTMAP(bthl_channel_state_t, -1, "(unknown)")
	DELEMENT(BTHL_CONN_STATE_CONNECTING),
	DELEMENT(BTHL_CONN_STATE_CONNECTED),
	DELEMENT(BTHL_CONN_STATE_DISCONNECTING),
	DELEMENT(BTHL_CONN_STATE_DISCONNECTED),
	DELEMENT(BTHL_CONN_STATE_DESTROYED),
ENDMAP

#define APP_ID_SIZE 20
#define MDEP_CFG_SIZE 10
#define CHANNEL_ID_SIZE 50

struct channel_info {
	int fd;
};

struct mdep_cfg {
	uint8_t role;
	struct channel_info channel[CHANNEL_ID_SIZE];
};

struct {
	struct mdep_cfg mdep[MDEP_CFG_SIZE];
} app[APP_ID_SIZE];

const bthl_interface_t *if_hl = NULL;

static void app_reg_state_cb(int app_id, bthl_app_reg_state_t state)
{
	haltest_info("%s: app_id=%d app_reg_state=%s\n", __func__,
				app_id, bthl_app_reg_state_t2str(state));
}

static void channel_state_cb(int app_id, bt_bdaddr_t *bd_addr,
					int index, int channel_id,
					bthl_channel_state_t state, int fd)
{
	char addr[MAX_ADDR_STR_LEN];

	haltest_info("%s: app_id=%d bd_addr=%s mdep_cfg_index=%d\n"
			"channel_id=%d channel_state=%s fd=%d\n", __func__,
			app_id, bt_bdaddr_t2str(bd_addr, addr), index,
			channel_id, bthl_channel_state_t2str(state), fd);

	if (app_id >= APP_ID_SIZE || index >= MDEP_CFG_SIZE
			|| channel_id >= CHANNEL_ID_SIZE) {
		haltest_error("exceeds maximum limit");
		return;
	}

	if (state == BTHL_CONN_STATE_CONNECTED) {
		app[app_id].mdep[index].channel[channel_id].fd = fd;

		/*
		 * PTS expects dummy data on fd when it
		 * connects in source role.
		 */
		if (app[app_id].mdep[index].role == BTHL_MDEP_ROLE_SOURCE)
			if (write(fd, "0", sizeof("0")) < 0)
				haltest_error("writing data on fd failed\n");

		return;
	}

	if (state == BTHL_CONN_STATE_DISCONNECTED ||
			state == BTHL_CONN_STATE_DESTROYED) {
		if (app[app_id].mdep[index].channel[channel_id].fd >= 0) {
			close(app[app_id].mdep[index].channel[channel_id].fd);
			app[app_id].mdep[index].channel[channel_id].fd = -1;
		}
	}
}

static bthl_callbacks_t hl_cbacks = {
	.size = sizeof(hl_cbacks),
	.app_reg_state_cb = app_reg_state_cb,
	.channel_state_cb = channel_state_cb,
};

/* init */

static void init_p(int argc, const char **argv)
{
	int i, j, k;

	for (i = 0; i < APP_ID_SIZE; i++) {
		for (j = 0; j < MDEP_CFG_SIZE; j++) {
			app[i].mdep[j].role = 0;
			for (k = 0; k < CHANNEL_ID_SIZE; k++)
				app[i].mdep[j].channel[k].fd = -1;
		}
	}


	RETURN_IF_NULL(if_hl);

	EXEC(if_hl->init, &hl_cbacks);
}

/* register_application */

static void register_application_p(int argc, const char **argv)
{
	bthl_reg_param_t reg;
	uint16_t mdep_argc_init, mdep_argc_off;
	int app_id = -1;
	int i;

	RETURN_IF_NULL(if_hl);

	if (argc <= 2) {
		haltest_error("No app name is specified\n");
		return;
	}

	if (argc <= 3) {
		haltest_error("No provider is specified\n");
		return;
	}

	if (argc <= 4) {
		haltest_error("No service name is specified\n");
		return;
	}

	if (argc <= 5) {
		haltest_error("No service description is specified\n");
		return;
	}

	if (argc <= 6) {
		haltest_error("No num of mdeps is specified\n");
		return;
	}

	memset(&reg, 0, sizeof(reg));

	if (argc != ((atoi(argv[6]) * 4) + 7)) {
		haltest_error("mdep cfg argumetns are not proper\n");
		return;
	}

	reg.application_name = argv[2];

	if (strcmp("-", argv[3]))
		reg.provider_name = argv[3];

	if (strcmp("-", argv[4]))
		reg.srv_name = argv[4];

	if (strcmp("-", argv[5]))
		reg.srv_desp = argv[5];

	reg.number_of_mdeps = atoi(argv[6]);

	reg.mdep_cfg = malloc(reg.number_of_mdeps * sizeof(bthl_mdep_cfg_t));
	if (!reg.mdep_cfg) {
		haltest_error("malloc failed\n");
		return;
	}
	mdep_argc_init = 7;

	for (i = 0; i < reg.number_of_mdeps; i++) {
		mdep_argc_off = mdep_argc_init + (4 * i);
		reg.mdep_cfg[i].mdep_role =
				str2bthl_mdep_role_t(argv[mdep_argc_off]);
		reg.mdep_cfg[i].data_type = atoi(argv[mdep_argc_off + 1]);
		reg.mdep_cfg[i].channel_type =
			str2bthl_channel_type_t(argv[mdep_argc_off + 2]);

		if (!strcmp("-", argv[mdep_argc_off + 3])) {
			reg.mdep_cfg[i].mdep_description = NULL;
			continue;
		}

		reg.mdep_cfg[i].mdep_description = argv[mdep_argc_off + 3];
	}

	EXEC(if_hl->register_application, &reg, &app_id);

	for (i = 0; i < reg.number_of_mdeps; i++)
		app[app_id].mdep[i].role = reg.mdep_cfg[i].mdep_role;

	free(reg.mdep_cfg);
}

/* unregister_application */

static void unregister_application_p(int argc, const char **argv)
{
	uint32_t app_id;

	RETURN_IF_NULL(if_hl);

	if (argc <= 2) {
		haltest_error("No app id is specified");
		return;
	}

	app_id = (uint32_t) atoi(argv[2]);

	EXEC(if_hl->unregister_application, app_id);
}

/* connect_channel */

static void connect_channel_p(int argc, const char **argv)
{
	uint32_t app_id, mdep_cfg_index;
	int channel_id = -1;
	bt_bdaddr_t bd_addr;

	RETURN_IF_NULL(if_hl);

	if (argc <= 2) {
		haltest_error("No app id is specified");
		return;
	}

	VERIFY_ADDR_ARG(3, &bd_addr);

	if (argc <= 4) {
		haltest_error("No mdep cfg index is specified");
		return;
	}

	app_id = (uint32_t) atoi(argv[2]);
	mdep_cfg_index = (uint32_t) atoi(argv[4]);

	EXEC(if_hl->connect_channel, app_id, &bd_addr, mdep_cfg_index,
								&channel_id);
}

/* destroy_channel */

static void destroy_channel_p(int argc, const char **argv)
{
	uint32_t channel_id;

	RETURN_IF_NULL(if_hl);

	if (argc <= 2) {
		haltest_error("No channel id is specified");
		return;
	}

	channel_id = (uint32_t) atoi(argv[2]);

	EXEC(if_hl->destroy_channel, channel_id);
}

/* close_channel */

static void close_channel_p(int argc, const char **argv)
{
	uint32_t app_id;
	uint8_t index;
	int channel_id;

	RETURN_IF_NULL(if_hl);

	if (argc <= 2) {
		haltest_error("No app id is specified");
		return;
	}

	if (argc <= 3) {
		haltest_error("No mdep_cfg_index is specified");
		return;
	}

	if (argc <= 4) {
		haltest_error("No channel_id is specified");
		return;
	}

	app_id = (uint32_t) atoi(argv[2]);
	if (app_id >= APP_ID_SIZE) {
		haltest_error("Wrong app_id specified: %u\n", app_id);
		return;
	}

	index = (uint8_t) atoi(argv[3]);
	if (index >= MDEP_CFG_SIZE) {
		haltest_error("Wrong mdep cfg index: %u\n", index);
		return;
	}

	channel_id = atoi(argv[4]);
	if (channel_id >= CHANNEL_ID_SIZE) {
		haltest_error("Wrong channel id: %u\n", channel_id);
		return;
	}

	if (app[app_id].mdep[index].channel[channel_id].fd >= 0) {
		shutdown(app[app_id].mdep[index].channel[channel_id].fd,
								SHUT_RDWR);
		app[app_id].mdep[index].channel[channel_id].fd = -1;
	}
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hl);

	EXECV(if_hl->cleanup);
	if_hl = NULL;
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODH(register_application,
		"<app_name> <provider_name> <srv_name> <srv_descr>\n"
		"<num_of_mdeps>\n"
		"[[<mdep_role>] [<data_type>] [<channel_type>] [<mdep_descr>]]"
		"..."),
	STD_METHODH(unregister_application, "<app_id>"),
	STD_METHODH(connect_channel, "<app_id> <bd_addr> <mdep_cfg_index>"),
	STD_METHODH(destroy_channel, "<channel_id>"),
	STD_METHODH(close_channel, "<app_id> <mdep_cfg_index> <channel_id>"),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface hl_if = {
	.name = "hl",
	.methods = methods
};
