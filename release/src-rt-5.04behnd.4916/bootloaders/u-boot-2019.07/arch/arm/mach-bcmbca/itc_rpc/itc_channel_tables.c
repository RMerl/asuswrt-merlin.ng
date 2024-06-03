/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#include "itc_rpc.h"
#include "itc_channel_structs.h"

/* This file should only be included in itc_rpc_core.c
 * The tables below are intended to be modified
 * for specific implementation requirements
 */

/*
 * services table: must match service indexes
 */
enum init_svc_idx {
	INIT_SVC_HANDSHAKE = 0,
	INIT_SVC_ERR,
	INIT_SVC_MAX
};

enum init_err_rc {
	INIT_SVC_ERR_RC_HANDSHAKE = 1,
	INIT_SVC_ERR_RC_RPC_VER_MISMATCH,
	INIT_SVC_ERR_RC_MSG_VER_MISMATCH
};

char *init_err_rc_str[] = {
	"tunnel init failure",
	"RPC version mismatch",
	"message version mismatch"
};

#define RPC_SERVICE_VER_INIT_HANDSHAKE	0
#define RPC_SERVICE_VER_INIT_ERR	0

int init_service_handshake(enum rpc_tunnel_idx tunnel, rpc_msg *msg);
int init_service_err(enum rpc_tunnel_idx tunnel, rpc_msg *msg);
rpc_function init_svc_table[] =
{
	{ init_service_handshake,	RPC_SERVICE_VER_INIT_HANDSHAKE },
	{ init_service_err,		RPC_SERVICE_VER_INIT_ERR },
};

rpc_service itc_rpc_services[RPC_MAX_SERVICES] =
{
	{
		.name = "rpc_init",
		.func_tab = init_svc_table,
		.func_tab_sz = INIT_SVC_MAX,
		.active = true
	},
	{ .name = "rpc_misc",		.active = false },
	{ .name = "rpc_flash",		.active = false },
	{ .name = "rpc_nonvol",		.active = false },
	{ .name = "rpc_erouter",	.active = false },
	{ .name = "rpc_erouter_pm",	.active = false },
	{ .name = "rpc_wpsctl",		.active = false },
	{ .name = "rpc_bash",		.active = false },
	{ .name = "rpc_testu",		.active = false },
	{ .name = "rpc_gpio",		.active = false },
	{ .name = "rpc_pinctrl",	.active = false },
	{ .name = "rpc_led",		.active = false },
	{ .name = "rpc_vfbio",		.active = false },
	{ .name = "rpc_pwr",		.active = false },
	{ .name = "rpc_clk",		.active = false },
	{ .name = "rpc_sys",		.active = false },
	{ .name = "rpc_bbs",		.active = false },
	{ .name = "rpc_ba",		.active = true  },
	{ .name = "rpc_avs",		.active = false },
	{ .name = "rpc_ubcap",		.active = false },
	{ .name = "rpc_fpm",		.active = false },
	{ .name = "rpc_rtc",		.active = false },
};

/* FIFO tunnels */
fifo_tunnel tunnels[] =
{
	{
		.name = "rpcrgsmc",
		.tx_fifo = "dqm0.q0",
		.rx_fifo = "dqm0.q1",
		.fifo_dev_idx = DQM_DEV_SMC,
		.tx_fifo_idx = 0,
		.rx_fifo_idx = 1,
	},
	{
		.name = "rpcvflashsmc",
		.tx_fifo = "dqm1.q0",
		.rx_fifo = "dqm1.q1",
		.fifo_dev_idx = DQM_DEV_SMC,
		.tx_fifo_idx = 2,
		.rx_fifo_idx = 3,
	},
	{
		.name = "rpcavssmc",
		.tx_fifo = "dqm2.q0",
		.rx_fifo = "dqm2.q1",
		.fifo_dev_idx = DQM_DEV_SMC,
		.tx_fifo_idx = 4,
		.rx_fifo_idx = 5,
	}
};

#define RPC_MAX_TUNNELS	((int)(sizeof(tunnels)/sizeof(fifo_tunnel)))
