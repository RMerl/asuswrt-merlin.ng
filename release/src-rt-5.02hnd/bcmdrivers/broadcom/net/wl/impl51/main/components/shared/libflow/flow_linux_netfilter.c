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
#include <netinet/in.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <linux/netfilter/nf_conntrack_tcp.h>

#include <ctf_cfg.h>

#include "flow_linux.h"

/* This is based on netfilter's conntrack delete example */
int flow_netfilter_delete_flow(ctf_tuple_t *req)
{
	struct mnl_socket *nl;
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	unsigned int seq, portid;
	struct nf_conntrack *ct = NULL;
	int ret;
	int status = SUCCESS;

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		return FAILURE;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		status = FAILURE;
		goto out;
	}
	portid = mnl_socket_get_portid(nl);

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_CTNETLINK << 8) | IPCTNL_MSG_CT_DELETE;
	nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	if (req->family == AF_INET6) {
		nfh->nfgen_family = AF_INET6;
	} else {
		nfh->nfgen_family = AF_INET;
	}
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;

	ct = nfct_new();
	if (ct == NULL) {
		perror("nfct_new");
		status = FAILURE;
		goto out;
	}

	if (req->family == AF_INET6) {
		nfct_set_attr_u8(ct, ATTR_L3PROTO, AF_INET6);
		nfct_set_attr_l(ct, ATTR_IPV6_SRC, &req->src_addr.ip_v6,
		                sizeof(req->src_addr.ip_v6));
		nfct_set_attr_l(ct, ATTR_IPV6_DST, &req->dst_addr.ip_v6,
		                sizeof(req->dst_addr.ip_v6));
	} else {
		nfct_set_attr_u8(ct, ATTR_L3PROTO, AF_INET);
		nfct_set_attr_u32(ct, ATTR_IPV4_SRC, req->src_addr.ip_v4.s_addr);
		nfct_set_attr_u32(ct, ATTR_IPV4_DST, req->dst_addr.ip_v4.s_addr);
	}

	nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(ct, ATTR_PORT_SRC, req->src_port);
	nfct_set_attr_u16(ct, ATTR_PORT_DST, req->dst_port);

	nfct_nlmsg_build(nlh, ct);

	ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
	if (ret == -1) {
		perror("mnl_socket_sendto");
		status = FAILURE;
		goto out;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));

	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
		if (ret <= MNL_CB_STOP)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("mnl_socket_recvfrom");
		status = FAILURE;
		goto out;
	}

out:
	if (nl) {
		mnl_socket_close(nl);
	}

	if (ct) {
		nfct_destroy(ct);
	}

	return status;
}
