/*
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#include <ctf_cfg.h>

#include "flow_linux.h"
#include "flow_api.h"

typedef enum {
	FLOW_REQ_NONE,
	FLOW_REQ_SUSPEND,
	FLOW_REQ_RESUME,
	FLOW_REQ_DELETE,
	FLOW_REQ_TUPLE_VALID,
	FLOW_LAST
} flow_req_t;

/*
 * Given a 5 tuple, perform the flow operation.
 */
static int _configure_flow(int family,
                           ip_address_t *src_addr, uint16_t src_port,
                           ip_address_t *dst_addr, uint16_t dst_port,
                           uint8_t protocol,
                           flow_req_t request)
{
	ctf_cfg_request_t req;
	ctf_tuple_t tuple, *tp = NULL;
	int ret;

	if (src_addr == NULL) {
		fprintf(stderr, "src_addr is NULL\n");
		return FAILURE;
	}

	if (dst_addr == NULL) {
		fprintf(stderr, "dst_addr is NULL\n");
		return FAILURE;
	}

	tuple.family = family;
	tuple.src_addr = *src_addr;
	tuple.src_port = src_port;
	tuple.dst_addr = *dst_addr;
	tuple.dst_port = dst_port;
	tuple.protocol = protocol;

	switch (request) {
	case FLOW_REQ_SUSPEND:
	case FLOW_REQ_RESUME:
	case FLOW_REQ_TUPLE_VALID:
		/* Send a netlink message to CTF driver */
		memset(&req, '\0', sizeof(req));
		if (request == FLOW_REQ_SUSPEND) {
			req.command_id = CTFCFG_CMD_SUSPEND;
		} else if (request == FLOW_REQ_RESUME) {
			req.command_id = CTFCFG_CMD_RESUME;
		} else if (request == FLOW_REQ_TUPLE_VALID) {
			req.command_id = CTFCFG_CMD_TUPLE_VALID;
		} else {
			fprintf(stderr, "Invalid request %d\n", request);
			return FAILURE;
		}
		req.size = sizeof(ctf_tuple_t);
		tp = (ctf_tuple_t *) req.arg;
		*tp = tuple;

		ret = flow_cfg_request_send(&req, sizeof(req));
		if (ret < 0) {
			fprintf(stderr, "Unable to send request to CTF\n");
			return FAILURE;
		}

		if (req.status != CTFCFG_STATUS_SUCCESS) {
			fprintf(stderr, "%s\n", req.arg);
			return FAILURE;
		}
		break;
	case FLOW_REQ_DELETE:
		/*
		 * Deleting a flow requires sending a netlink message to
		 * netfilter.
		 */
		ret = flow_netfilter_delete_flow(&tuple);
		break;
	default:
		return FAILURE;
	}

	return SUCCESS;
}

int flow_suspend(int family,
                 ip_address_t *src_addr, uint16_t src_port,
                 ip_address_t *dst_addr, uint16_t dst_port,
                 uint8_t protocol)
{
	return _configure_flow(family, src_addr, src_port,
	                       dst_addr, dst_port, protocol,
	                       FLOW_REQ_SUSPEND);
}

int flow_resume(int family,
                ip_address_t *src_addr, uint16_t src_port,
                ip_address_t *dst_addr, uint16_t dst_port,
                uint8_t protocol)
{
	return _configure_flow(family, src_addr, src_port,
	                       dst_addr, dst_port, protocol,
	                       FLOW_REQ_RESUME);
}

int flow_delete(int family,
                ip_address_t *src_addr, uint16_t src_port,
                ip_address_t *dst_addr, uint16_t dst_port,
                uint8_t protocol)
{
	return _configure_flow(family, src_addr, src_port,
	                       dst_addr, dst_port, protocol,
	                       FLOW_REQ_DELETE);
}

int flow_valid(int family,
               ip_address_t *src_addr, uint16_t src_port,
               ip_address_t *dst_addr, uint16_t dst_port,
               uint8_t protocol)
{
	return _configure_flow(family, src_addr, src_port,
	                       dst_addr, dst_port, protocol,
	                       FLOW_REQ_TUPLE_VALID);
}

/* For backward compatibility */
int flow_default_forwarding_get(ctf_fwd_t *fwd)
{
	return flow_default_forwarding_get_for_proto(fwd, IPPROTO_TCP);
}

/* For backward compatibility */
int flow_default_forwarding_set(ctf_fwd_t fwd)
{
	return flow_default_forwarding_set_for_proto(fwd, IPPROTO_TCP);
}

int flow_default_forwarding_get_for_proto(ctf_fwd_t *fwd, uint8_t proto)
{
	ctf_cfg_request_t req;
	ctf_fwd_t *f;
	uint8_t *p;
	int ret;

	memset(&req, '\0', sizeof(req));
	req.command_id = CTFCFG_CMD_DEFAULT_FWD_GET;
	req.size = sizeof(uint8_t);
	p = req.arg;
	*p = proto;

	ret = flow_cfg_request_send(&req, sizeof(req));

	if (ret < 0) {
		fprintf(stderr, "Unable to send request to CTF\n");
		return FAILURE;
	}

	if (req.status != CTFCFG_STATUS_SUCCESS) {
		fprintf(stderr, "%s\n", req.arg);
		return FAILURE;
	}

	f = (ctf_fwd_t *) req.arg;
	*fwd = *f;

	return SUCCESS;
}

int flow_default_forwarding_set_for_proto(ctf_fwd_t fwd, uint8_t proto)
{
	ctf_cfg_request_t req;
	ctf_fwd_t *f;
	uint8_t *p;
	int ret;

	memset(&req, '\0', sizeof(req));
	req.command_id = CTFCFG_CMD_DEFAULT_FWD_SET;
	req.size = sizeof(ctf_fwd_t) + sizeof(uint8_t);
	f = (ctf_fwd_t *) req.arg;
	*f = fwd;
	p = (req.arg + sizeof(ctf_fwd_t));
	*p = proto;

	ret = flow_cfg_request_send(&req, sizeof(req));

	if (ret < 0) {
		fprintf(stderr, "Unable to send request to CTF\n");
		return FAILURE;
	}

	if (req.status != CTFCFG_STATUS_SUCCESS) {
		fprintf(stderr, "%s\n", req.arg);
		return FAILURE;
	}

	return SUCCESS;
}
