/*
 * (C) 2006-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include "conntrackd.h"
#include "log.h"
#include "fds.h"
#include "helper.h"

#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#define _GNU_SOURCE
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>

#ifndef __aligned_be64
#define __aligned_be64 unsigned long long __attribute__((aligned(8)))
#endif

#include <linux/netfilter/nfnetlink_queue.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/pktbuff.h>
#include <libnetfilter_cthelper/libnetfilter_cthelper.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/pktbuff.h>

void cthelper_kill(void)
{
	mnl_socket_close(STATE_CTH(nl));
	free(state.cthelper);
}

int cthelper_local(int fd, int type, void *data)
{
	/* No services to obtain information on helpers yet, sorry */
	return LOCAL_RET_OK;
}

static struct nlmsghdr *
nfq_hdr_put(char *buf, int type, uint32_t queue_num)
{
	struct nlmsghdr *nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_QUEUE << 8) | type;
	nlh->nlmsg_flags = NLM_F_REQUEST;

	struct nfgenmsg *nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
	nfg->nfgen_family = AF_UNSPEC;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = htons(queue_num);

	return nlh;
}

static int
pkt_get(void *pkt, uint32_t pktlen, uint16_t proto, uint32_t *protoff)
{
	uint8_t protocol;

	switch(proto) {
	case ETHERTYPE_IP: {
		struct iphdr *ip = (struct iphdr *) pkt;

		/* No room for IPv4 header. */
		if (pktlen < sizeof(struct iphdr)) {
			dlog(LOG_ERR, "no room for IPv4 header");
			return -1;
		}

		/* this is not IPv4, skip. */
		if (ip->version != 4) {
			dlog(LOG_ERR, "not IPv4, skipping");
			return -1;
		}

		*protoff = 4 * ip->ihl;
		protocol = ip->protocol;
		break;
	}
	case ETHERTYPE_IPV6: {
		struct iphdr *ip = (struct iphdr *) pkt;
		struct ip6_hdr *ip6 = (struct ip6_hdr *) pkt;

		/* No room for IPv6 header. */
		if (pktlen < sizeof(struct ip6_hdr)) {
			dlog(LOG_ERR, "no room for IPv6 header");
			return -1;
		}

		/* this is not IPv6, skip. */
		if (ip->version != 6) {
			dlog(LOG_ERR, "not IPv6, skipping");
			return -1;
		}

		*protoff = sizeof(struct ip6_hdr);
		protocol = ip6->ip6_nxt;
		break;
	}
	default:
		/* Unknown layer 3 protocol. */
		dlog(LOG_ERR, "unknown layer 3 protocol (%d), skipping", proto);
		return -1;
	}

	switch (protocol) {
	case IPPROTO_TCP: {
		struct tcphdr *tcph =
			(struct tcphdr *) ((char *)pkt + *protoff);

		/* No room for IPv4 header plus TCP header. */
		if (pktlen < *protoff + sizeof(struct tcphdr) ||
		    pktlen < *protoff + tcph->doff * 4) {
			dlog(LOG_ERR, "no room for IPv4 + TCP header, skip");
			return -1;
		}
		return 0;
	}
	case IPPROTO_UDP:
		/* No room for IPv4 header plus UDP header. */
		if (pktlen < *protoff + sizeof(struct udphdr)) {
			dlog(LOG_ERR, "no room for IPv4 + UDP header, skip");
			return -1;
		}
		return 0;
	default:
		dlog(LOG_ERR, "not TCP/UDP, skipping");
		return -1;
	}

	return 0;
}

static int
pkt_verdict_issue(struct ctd_helper_instance *cur, struct myct *myct,
		  uint16_t queue_num, uint32_t id, uint32_t verdict,
		  struct pkt_buff *pktb)
{
	struct nlmsghdr *nlh;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *nest;

	nlh = nfq_hdr_put(buf, NFQNL_MSG_VERDICT, queue_num);

	/* save private data and send it back to kernel-space, if any. */
	if (myct->priv_data) {
		nfct_set_attr_l(myct->ct, ATTR_HELPER_INFO, myct->priv_data,
				cur->helper->priv_data_len);
	}

	nfq_nlmsg_verdict_put(nlh, id, verdict);
	if (pktb_mangled(pktb))
		nfq_nlmsg_verdict_put_pkt(nlh, pktb_data(pktb), pktb_len(pktb));

	nest = mnl_attr_nest_start(nlh, NFQA_CT);
	if (nest == NULL)
		return -1;

	nfct_nlmsg_build(nlh, myct->ct);
	mnl_attr_nest_end(nlh, nest);

	if (myct->exp) {
		nest = mnl_attr_nest_start(nlh, NFQA_EXP);
		if (nest == NULL)
			return -1;

		nfexp_nlmsg_build(nlh, myct->exp);
		mnl_attr_nest_end(nlh, nest);
	}

	if (mnl_socket_sendto(STATE_CTH(nl), nlh, nlh->nlmsg_len) < 0) {
		dlog(LOG_ERR, "failed to send verdict: %s", strerror(errno));
		return -1;
	}

	return 0;
}

static int
pkt_verdict_error(uint16_t queue_num, uint32_t id)
{
	struct nlmsghdr *nlh;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	nlh = nfq_hdr_put(buf, NFQNL_MSG_VERDICT, queue_num);
	nfq_nlmsg_verdict_put(nlh, id, NF_ACCEPT);

	if (mnl_socket_sendto(STATE_CTH(nl), nlh, nlh->nlmsg_len) < 0) {
		dlog(LOG_ERR, "failed to send verdict: %s", strerror(errno));
		return -1;
	}
	return 0;
}

static struct ctd_helper_instance *
helper_run(struct pkt_buff *pktb, uint32_t protoff, struct myct *myct,
	   uint32_t ctinfo, uint32_t queue_num, int *verdict)
{
	struct ctd_helper_instance *cur, *helper = NULL;

	list_for_each_entry(cur, &CONFIG(cthelper).list, head) {
		if (cur->queue_num == queue_num) {
			const void *priv_data;

			/* retrieve helper private data. */
			priv_data = nfct_get_attr(myct->ct, ATTR_HELPER_INFO);
			if (priv_data != NULL) {
				myct->priv_data =
					calloc(1, cur->helper->priv_data_len);

				if (myct->priv_data == NULL)
					continue;

				memcpy(myct->priv_data, priv_data,
					cur->helper->priv_data_len);
			}

			*verdict = cur->helper->cb(pktb, protoff, myct, ctinfo);
			helper = cur;
			break;
		}
	}
	return helper;
}

static int nfq_queue_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nfqnl_msg_packet_hdr *ph = NULL;
	struct nlattr *attr[NFQA_MAX+1] = {};
	struct nfgenmsg *nfg;
	uint8_t *pkt;
	uint16_t l3num;
	uint32_t id, ctinfo, queue_num = 0, protoff = 0, pktlen;
	struct nf_conntrack *ct = NULL;
	struct myct *myct;
	struct ctd_helper_instance *helper;
	struct pkt_buff *pktb;
	int verdict = NF_ACCEPT;

	if (nfq_nlmsg_parse(nlh, attr) < 0) {
		dlog(LOG_ERR, "problems parsing message from kernel");
		return MNL_CB_ERROR;
	}

	ph = (struct nfqnl_msg_packet_hdr *)
		mnl_attr_get_payload(attr[NFQA_PACKET_HDR]);
	if (ph == NULL) {
		dlog(LOG_ERR, "problems retrieving metaheader");
		return MNL_CB_ERROR;
	}

	id = ntohl(ph->packet_id);

	if (!attr[NFQA_PAYLOAD]) {
		dlog(LOG_ERR, "packet with no payload");
		goto err1;
	}
	if (!attr[NFQA_CT] || !attr[NFQA_CT_INFO]) {
		dlog(LOG_ERR, "no CT attached to this packet");
		goto err1;
	}

	pkt = mnl_attr_get_payload(attr[NFQA_PAYLOAD]);
	pktlen = mnl_attr_get_payload_len(attr[NFQA_PAYLOAD]);

	nfg = mnl_nlmsg_get_payload(nlh);
	l3num = nfg->nfgen_family;
	queue_num = ntohs(nfg->res_id);

	if (pkt_get(pkt, pktlen, ntohs(ph->hw_protocol), &protoff))
		goto err1;

	ct = nfct_new();
	if (ct == NULL)
		goto err1;

	if (nfct_payload_parse(mnl_attr_get_payload(attr[NFQA_CT]),
			       mnl_attr_get_payload_len(attr[NFQA_CT]),
			       l3num, ct) < 0) {
		dlog(LOG_ERR, "cannot convert message to CT");
		goto err2;
	}

	myct = calloc(1, sizeof(struct myct));
	if (myct == NULL)
		goto err2;

	myct->ct = ct;
	ctinfo = ntohl(mnl_attr_get_u32(attr[NFQA_CT_INFO]));

	/* XXX: 256 bytes enough for possible NAT mangling in helpers? */
	pktb = pktb_alloc(AF_INET, pkt, pktlen, 256);
	if (pktb == NULL)
		goto err3;

	/* Misconfiguration: if no helper found, accept the packet. */
	helper = helper_run(pktb, protoff, myct, ctinfo, queue_num, &verdict);
	if (!helper)
		goto err4;

	if (pkt_verdict_issue(helper, myct, queue_num, id, verdict, pktb) < 0)
		goto err4;

	pktb_free(pktb);
	nfct_destroy(ct);
	if (myct->exp != NULL)
		nfexp_destroy(myct->exp);
	if (myct->priv_data != NULL)
		free(myct->priv_data);
	free(myct);

	return MNL_CB_OK;
err4:
	pktb_free(pktb);
err3:
	free(myct);
err2:
	nfct_destroy(ct);
err1:
	/* In case of error, we don't want to disrupt traffic. We accept all.
	 * This is connection tracking after all. The policy is not to drop
	 * packet unless we enter some inconsistent state.
	 */
	pkt_verdict_error(queue_num, id);

	return MNL_CB_OK;
}

static void nfq_cb(void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret;

	ret = mnl_socket_recvfrom(STATE_CTH(nl), buf, sizeof(buf));
	if (ret == -1) {
		dlog(LOG_ERR, "failed to receive message: %s", strerror(errno));
		return;
	}

	ret = mnl_cb_run(buf, ret, 0, STATE_CTH(portid), nfq_queue_cb, NULL);
	if (ret < 0){
		dlog(LOG_ERR, "failed to process message");
		return;
	}
}

static int cthelper_setup(struct ctd_helper_instance *cur)
{
	struct nfct_helper *t;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t seq;
	int j, ret;

	t = nfct_helper_alloc();
	if (t == NULL) {
		dlog(LOG_ERR, "cannot allocate object for helper");
		return -1;
	}

	nfct_helper_attr_set(t, NFCTH_ATTR_NAME, cur->helper->name);
	nfct_helper_attr_set_u32(t, NFCTH_ATTR_QUEUE_NUM, cur->queue_num);
	nfct_helper_attr_set_u16(t, NFCTH_ATTR_PROTO_L3NUM, cur->l3proto);
	nfct_helper_attr_set_u8(t, NFCTH_ATTR_PROTO_L4NUM, cur->l4proto);
	nfct_helper_attr_set_u32(t, NFCTH_ATTR_STATUS,
					NFCT_HELPER_STATUS_ENABLED);

	dlog(LOG_NOTICE, "configuring helper `%s' with queuenum=%d and "
			 "queuelen=%d", cur->helper->name, cur->queue_num,
			 cur->queue_len);

	for (j=0; j<CTD_HELPER_POLICY_MAX; j++) {
		struct nfct_helper_policy *p;

		if (!cur->helper->policy[j].name[0])
			break;

		p = nfct_helper_policy_alloc();
		if (p == NULL) {
			dlog(LOG_ERR, "cannot allocate object for helper");
			return -1;
		}
		/* FIXME: get existing policy values from the kernel first. */
		nfct_helper_policy_attr_set(p, NFCTH_ATTR_POLICY_NAME,
					cur->helper->policy[j].name);
		nfct_helper_policy_attr_set_u32(p, NFCTH_ATTR_POLICY_TIMEOUT,
					cur->helper->policy[j].expect_timeout);
		nfct_helper_policy_attr_set_u32(p, NFCTH_ATTR_POLICY_MAX,
					cur->helper->policy[j].expect_max);

		dlog(LOG_NOTICE, "policy name=%s expect_timeout=%d expect_max=%d",
			cur->helper->policy[j].name,
			cur->helper->policy[j].expect_timeout,
			cur->helper->policy[j].expect_max);

		nfct_helper_attr_set(t, NFCTH_ATTR_POLICY+j, p);
	}

	if (j == 0) {
		dlog(LOG_ERR, "you have to define one policy for helper");
		return -1;
	}

	seq = time(NULL);
	nlh = nfct_helper_nlmsg_build_hdr(buf, NFNL_MSG_CTHELPER_NEW,
					  NLM_F_CREATE | NLM_F_ACK, seq);
	nfct_helper_nlmsg_build_payload(nlh, t);

	nfct_helper_free(t);

	if (mnl_socket_sendto(STATE_CTH(nl), nlh, nlh->nlmsg_len) < 0) {
		dlog(LOG_ERR, "sending cthelper configuration");
		return -1;
	}

	ret = mnl_socket_recvfrom(STATE_CTH(nl), buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, STATE_CTH(portid), NULL, NULL);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(STATE_CTH(nl), buf, sizeof(buf));
	}
	if (ret == -1) {
		dlog(LOG_ERR, "trying to configure cthelper `%s': %s",
			cur->helper->name, strerror(errno));
		return -1;
	}

	return 0;
}

static int cthelper_nfqueue_setup(struct ctd_helper_instance *cur)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;

	nlh = nfq_hdr_put(buf, NFQNL_MSG_CONFIG, cur->queue_num);
	nfq_nlmsg_cfg_put_cmd(nlh, AF_INET, NFQNL_CFG_CMD_BIND);

	if (mnl_socket_sendto(STATE_CTH(nl), nlh, nlh->nlmsg_len) < 0) {
		dlog(LOG_ERR, "failed to send bind command");
		return -1;
	}

	nlh = nfq_hdr_put(buf, NFQNL_MSG_CONFIG, cur->queue_num);
	nfq_nlmsg_cfg_put_params(nlh, NFQNL_COPY_PACKET, 0xffff);
	mnl_attr_put_u32(nlh, NFQA_CFG_FLAGS, htonl(NFQA_CFG_F_CONNTRACK));
	mnl_attr_put_u32(nlh, NFQA_CFG_MASK, htonl(0xffffffff));
	if (cur->queue_len > 0) {
		mnl_attr_put_u32(nlh, NFQA_CFG_QUEUE_MAXLEN,
				 htonl(cur->queue_len));
	}

	if (mnl_socket_sendto(STATE_CTH(nl), nlh, nlh->nlmsg_len) < 0) {
		dlog(LOG_ERR, "failed to send configuration");
		return -1;
	}

	return 0;
}

static int cthelper_configure(struct ctd_helper_instance *cur)
{
	/* First, configure cthelper. */
	if (cthelper_setup(cur) < 0)
		return -1;

	/* Now, we are ready to configure nfqueue attached to this helper. */
	if (cthelper_nfqueue_setup(cur) < 0)
		return -1;

	dlog(LOG_NOTICE, "helper `%s' configured successfully",
		cur->helper->name);

	return 0;
}

static int nfq_configure(void)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;

	nlh = nfq_hdr_put(buf, NFQNL_MSG_CONFIG, 0);
	nfq_nlmsg_cfg_put_cmd(nlh, AF_INET, NFQNL_CFG_CMD_PF_UNBIND);

	if (mnl_socket_sendto(STATE_CTH(nl), nlh, nlh->nlmsg_len) < 0) {
		dlog(LOG_ERR, "failed to send pf unbind command");
		return -1;
	}

	nlh = nfq_hdr_put(buf, NFQNL_MSG_CONFIG, 0);
	nfq_nlmsg_cfg_put_cmd(nlh, AF_INET, NFQNL_CFG_CMD_PF_BIND);

	if (mnl_socket_sendto(STATE_CTH(nl), nlh, nlh->nlmsg_len) < 0) {
		dlog(LOG_ERR, "failed to send pf bind command");
		return -1;
	}

	return 0;
}

int cthelper_init(void)
{
	struct ctd_helper_instance *cur;
	int ret;

	state.cthelper = calloc(1, sizeof(struct ct_helper_state));
	if (state.cthelper == NULL) {
		dlog(LOG_ERR, "can't allocate memory for cthelper struct");
		return -1;
	}

	STATE_CTH(nl) = mnl_socket_open(NETLINK_NETFILTER);
	if (STATE_CTH(nl) == NULL) {
		dlog(LOG_ERR, "cannot open nfq socket");
		return -1;
	}
	fcntl(mnl_socket_get_fd(STATE_CTH(nl)), F_SETFL, O_NONBLOCK);

	if (mnl_socket_bind(STATE_CTH(nl), 0, MNL_SOCKET_AUTOPID) < 0) {
		dlog(LOG_ERR, "cannot bind nfq socket");
		return -1;
	}
	STATE_CTH(portid) = mnl_socket_get_portid(STATE_CTH(nl));

	if (nfq_configure())
		return -1;

	list_for_each_entry(cur, &CONFIG(cthelper).list, head) {
		ret = cthelper_configure(cur);
		if (ret < 0)
			return ret;
	}

	register_fd(mnl_socket_get_fd(STATE_CTH(nl)), nfq_cb, NULL, STATE(fds));

	return 0;
}
