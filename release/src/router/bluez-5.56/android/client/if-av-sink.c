// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2014 Intel Corporation
 *
 */

#define _GNU_SOURCE
#include "if-main.h"
#include "../hal-utils.h"

const btav_interface_t *if_av_sink = NULL;

SINTMAP(btav_connection_state_t, -1, "(unknown)")
	DELEMENT(BTAV_CONNECTION_STATE_DISCONNECTED),
	DELEMENT(BTAV_CONNECTION_STATE_CONNECTING),
	DELEMENT(BTAV_CONNECTION_STATE_CONNECTED),
	DELEMENT(BTAV_CONNECTION_STATE_DISCONNECTING),
ENDMAP

SINTMAP(btav_audio_state_t, -1, "(unknown)")
	DELEMENT(BTAV_AUDIO_STATE_REMOTE_SUSPEND),
	DELEMENT(BTAV_AUDIO_STATE_STOPPED),
	DELEMENT(BTAV_AUDIO_STATE_STARTED),
ENDMAP

static char last_addr[MAX_ADDR_STR_LEN];

static void connection_state(btav_connection_state_t state,
							bt_bdaddr_t *bd_addr)
{
	haltest_info("(sink) %s: connection_state=%s remote_bd_addr=%s\n",
				__func__, btav_connection_state_t2str(state),
				bt_bdaddr_t2str(bd_addr, last_addr));
}

static void audio_state(btav_audio_state_t state, bt_bdaddr_t *bd_addr)
{
	haltest_info("(sink) %s: audio_state=%s remote_bd_addr=%s\n", __func__,
					btav_audio_state_t2str(state),
					bt_bdaddr_t2str(bd_addr, last_addr));
}

static void audio_config(bt_bdaddr_t *bd_addr, uint32_t sample_rate,
							uint8_t channel_count) {
	haltest_info("(sink) %s: addr=%s\n sample_rate=%d\n channel_count=%d\n",
				__func__, bt_bdaddr_t2str(bd_addr, last_addr),
				sample_rate, channel_count);
}

static btav_callbacks_t av_cbacks = {
	.size = sizeof(av_cbacks),
	.connection_state_cb = connection_state,
	.audio_state_cb = audio_state,
	.audio_config_cb = audio_config,
};

/* init */

static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_av_sink);

	EXEC(if_av_sink->init, &av_cbacks);
}

/* connect */

static void connect_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = NULL;
		*enum_func = enum_devices;
	}
}

static void connect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_av_sink);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_av_sink->connect, &addr);
}

/* disconnect */

static void disconnect_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = last_addr;
		*enum_func = enum_one_string;
	}
}

static void disconnect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_av_sink);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_av_sink->disconnect, &addr);
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_av_sink);

	EXECV(if_av_sink->cleanup);
	if_av_sink = NULL;
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODCH(connect, "<addr>"),
	STD_METHODCH(disconnect, "<addr>"),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface av_sink_if = {
	.name = "av-sink",
	.methods = methods
};
