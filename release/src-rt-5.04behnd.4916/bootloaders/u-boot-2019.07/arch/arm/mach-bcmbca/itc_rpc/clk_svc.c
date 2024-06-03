/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#include <stdio.h>
#include <string.h>
#include "itc_rpc.h"
#include "clk_svc.h"

#define RPC_SERVICE_VER_CLK_DOMAIN_ID		0
#define RPC_SERVICE_VER_CLK_DOMAIN_NAME		0
#define RPC_SERVICE_VER_CLK_GET_DOMAIN_STATE	0
#define RPC_SERVICE_VER_CLK_SET_DOMAIN_STATE	0

#define CLK_SVC_RPC_REQUEST_TIMEOUT	1 /* sec */

/* CLK service helpers */
static inline int clk_svc_request(rpc_msg *msg)
{
	int rc = 0;
	rc = rpc_send_request_timeout(RPC_TUNNEL_ARM_SMC_NS,
		msg, CLK_SVC_RPC_REQUEST_TIMEOUT);
	if (rc) {
		printf("clk_svc: rpc_send_request failure (%d)\n", rc);
		rpc_dump_msg(msg);
		goto done;
	}
	rc = clk_svc_msg_get_retcode(msg);
done:
	return rc;
}

/* CLK service calls */
int clk_svc_domain_id(const char *name, uint8_t *id)
{
	struct clk_msg clk_msg;
	rpc_msg *msg = (rpc_msg *)&clk_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_DOMAIN_ID,
			RPC_SERVICE_VER_CLK_DOMAIN_ID, 0, 0, 0);
	strncpy(clk_msg.name, name, sizeof(clk_msg.name));
	rc = clk_svc_request(msg);
	if (rc)
		goto done;
	*id = clk_msg.id;
done:
	return rc;
}

int clk_svc_domain_name(uint8_t id, char *name)
{
	struct clk_msg clk_msg;
	rpc_msg *msg = (rpc_msg *)&clk_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_DOMAIN_NAME,
			RPC_SERVICE_VER_CLK_DOMAIN_NAME, 0, 0, 0);
	clk_msg.id = id;
	rc = clk_svc_request(msg);
	if (rc)
		goto done;
	memcpy(name, &clk_msg.name[0], sizeof(clk_msg.name));
	name[8] = '\0';

done:
	return rc;
}

int clk_svc_get_domain_state(uint8_t id, bool *enabled, uint32_t *rate)
{
	struct clk_msg clk_msg;
	rpc_msg *msg = (rpc_msg *)&clk_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_GET_DOMAIN_STATE,
			RPC_SERVICE_VER_CLK_GET_DOMAIN_STATE, 0, 0, 0);
	clk_msg.id = id;
	rc = clk_svc_request(msg);
	if (rc)
		goto done;
	*enabled = clk_msg.enable == 1 ? true : false;
	*rate = clk_msg.rate;
done:
	return rc;
}

int clk_svc_set_domain_state(uint8_t id, bool enable, uint32_t rate)
{
	struct clk_msg clk_msg;
	rpc_msg *msg = (rpc_msg *)&clk_msg;

	rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_SET_DOMAIN_STATE,
			RPC_SERVICE_VER_CLK_SET_DOMAIN_STATE, 0, 0, 0);
	clk_msg.id = id;
	clk_msg.enable = enable ? 1 : 0;
	clk_msg.rate = rate;
	return clk_svc_request(msg);
}
