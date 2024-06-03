/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#include <stdio.h>
#include <string.h>
#include "itc_rpc.h"
#include "pwr_svc.h"

#define RPC_SERVICE_VER_PWR_DOMAIN_ID			0
#define RPC_SERVICE_VER_PWR_DOMAIN_NAME			0
#define RPC_SERVICE_VER_PWR_GET_DOMAIN_STATE	0
#define RPC_SERVICE_VER_PWR_SET_DOMAIN_STATE	0
#define PWR_SVC_RPC_REQUEST_TIMEOUT				1 /* sec */

/* BA service helpers */
static inline int pwr_svc_request(rpc_msg *msg)
{
	int rc = 0;
	rc = rpc_send_request_timeout(RPC_TUNNEL_ARM_SMC_NS,
		msg, PWR_SVC_RPC_REQUEST_TIMEOUT);
	if (rc) {
		printf("pwr_svc: rpc_send_request failure (%d)\n", rc);
		rpc_dump_msg(msg);
		goto done;
	}
	rc = pwr_svc_msg_get_retcode(msg);
done:
	return rc;
}

static inline int pwr_svc_message(rpc_msg *msg)
{
	int rc = 0;
	rc = rpc_send_message(RPC_TUNNEL_ARM_SMC_NS, msg);
	if (rc) {
		printf("pwr_svc: rpc_send_message failure (%d)\n", rc);
		rpc_dump_msg(msg);
	}
	return rc;
}

/* PWR service calls */
int pwr_svc_domain_id(char *name, uint8_t *id)
{
	struct pwr_msg pwr_msg;
	rpc_msg *msg = (rpc_msg *)&pwr_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_PWR, PWR_SVC_DOMAIN_ID,
			RPC_SERVICE_VER_PWR_DOMAIN_ID, 0, 0, 0);
	strncpy(pwr_msg.name, name, sizeof(pwr_msg.name));
	rc = pwr_svc_request(msg);
	if (rc)
		goto done;
	*id = pwr_msg.id;

done:
	return rc;
}

int pwr_svc_domain_name(uint8_t id, char *name)
{
	struct pwr_msg pwr_msg;
	rpc_msg *msg = (rpc_msg *)&pwr_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_PWR, PWR_SVC_DOMAIN_NAME,
			RPC_SERVICE_VER_PWR_DOMAIN_NAME, 0, 0, 0);
	pwr_msg.id = id;
	rc = pwr_svc_request(msg);
	if (rc)
		goto done;
	memcpy(name, &pwr_msg.name[0], sizeof(pwr_msg.name));
	name[8] = '\0';

done:
	return rc;
}

int pwr_svc_get_domain_state(uint8_t id, enum pwr_dom_state *state,
			     enum pwr_dom_reset *reset)
{
	struct pwr_msg pwr_msg;
	rpc_msg *msg = (rpc_msg *)&pwr_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_PWR, PWR_SVC_GET_DOMAIN_STATE,
			RPC_SERVICE_VER_PWR_GET_DOMAIN_STATE, 0, 0, 0);
	pwr_msg.id = id;
	rc = pwr_svc_request(msg);
	if (rc)
		goto done;
	*state = pwr_msg.state;
	*reset = pwr_msg.reset;
done:
	return rc;
}

int pwr_svc_set_domain_state(uint8_t id, enum pwr_dom_state state,
			     enum pwr_dom_reset reset)
{
	struct pwr_msg pwr_msg;
	rpc_msg *msg = (rpc_msg *)&pwr_msg;

	rpc_msg_init(msg, RPC_SERVICE_PWR, PWR_SVC_SET_DOMAIN_STATE,
			RPC_SERVICE_VER_PWR_SET_DOMAIN_STATE, 0, 0, 0);
	pwr_msg.id = id;
	pwr_msg.state = state;
	pwr_msg.reset = reset;
	return pwr_svc_request(msg);
}
