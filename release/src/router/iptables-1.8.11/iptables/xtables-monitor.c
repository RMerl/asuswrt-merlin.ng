/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#define _GNU_SOURCE
#include "config.h"
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <net/if_arp.h>
#include <getopt.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/table.h>
#include <libnftnl/trace.h>
#include <libnftnl/chain.h>
#include <libnftnl/rule.h>

#include <include/xtables.h>
#include "iptables.h" /* for xtables_globals */
#include "xtables-multi.h"
#include "nft.h"

struct cb_arg {
	uint32_t nfproto;
	bool is_event;
	struct nft_handle *h;
};

static int table_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t type = nlh->nlmsg_type & 0xFF;
	const struct cb_arg *arg = data;
	struct nftnl_table *t;
	char buf[4096];

	t = nftnl_table_alloc();
	if (t == NULL)
		goto err;

	if (nftnl_table_nlmsg_parse(nlh, t) < 0)
		goto err_free;

	if (arg->nfproto && arg->nfproto != nftnl_table_get_u32(t, NFTNL_TABLE_FAMILY))
		goto err_free;
	nftnl_table_snprintf(buf, sizeof(buf), t, NFTNL_OUTPUT_DEFAULT, 0);
	printf(" EVENT: ");
	printf("nft: %s table: %s\n", type == NFT_MSG_NEWTABLE ? "NEW" : "DEL", buf);

err_free:
	nftnl_table_free(t);
err:
	return MNL_CB_OK;
}

static const char *family_cmd(int family)
{
	switch (family) {
	case NFPROTO_IPV4:
		return "iptables";
	case NFPROTO_IPV6:
		return "ip6tables";
	case NFPROTO_ARP:
		return "arptables";
	case NFPROTO_BRIDGE:
		return "ebtables";
	default:
		return NULL;
	}
}

static bool counters;
static bool trace;
static bool events;

static int rule_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t type = nlh->nlmsg_type & 0xFF;
	const struct cb_arg *arg = data;
	struct nftnl_rule *r;
	uint8_t family;

	r = nftnl_rule_alloc();
	if (r == NULL)
		goto err;

	if (nftnl_rule_nlmsg_parse(nlh, r) < 0)
		goto err_free;

	family = nftnl_rule_get_u32(r, NFTNL_RULE_FAMILY);
	if (arg->nfproto && arg->nfproto != family)
		goto err_free;

	xtables_set_nfproto(family);
	arg->h->ops = nft_family_ops_lookup(family);
	arg->h->family = family;

	/* ignore policy rules unless tracing,
	 * they are reported when deleting user-defined chains */
	if (family == NFPROTO_BRIDGE &&
	    arg->is_event &&
	    nft_rule_is_policy_rule(r))
		goto err_free;

	if (!family_cmd(family))
		goto err_free;

	printf("%s%s -t %s ",
	       arg->is_event ? " EVENT: " : "",
	       family_cmd(family),
	       nftnl_rule_get_str(r, NFTNL_RULE_TABLE));
	nft_rule_print_save(arg->h, r,
			    type == NFT_MSG_NEWRULE ? NFT_RULE_APPEND
						    : NFT_RULE_DEL,
			    counters ? 0 : FMT_NOCOUNTS);
err_free:
	nftnl_rule_free(r);
err:
	return MNL_CB_OK;
}

static int chain_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t type = nlh->nlmsg_type & 0xFF;
	const struct cb_arg *arg = data;
	struct nftnl_chain *c;
	char buf[4096];
	int family;

	c = nftnl_chain_alloc();
	if (c == NULL)
		goto err;

	if (nftnl_chain_nlmsg_parse(nlh, c) < 0)
		goto err_free;

	family = nftnl_chain_get_u32(c, NFTNL_CHAIN_FAMILY);
	if (arg->nfproto && arg->nfproto != family)
		goto err_free;

	printf(" EVENT: ");

	if (nftnl_chain_is_set(c, NFTNL_CHAIN_PRIO) || !family_cmd(family)) {
		nftnl_chain_snprintf(buf, sizeof(buf),
				     c, NFTNL_OUTPUT_DEFAULT, 0);
		printf("nft: %s chain: %s\n",
		       type == NFT_MSG_NEWCHAIN ? "NEW" : "DEL", buf);
		goto err_free;
	}

	printf("%s -t %s -%c %s\n",
			family_cmd(family),
			nftnl_chain_get_str(c, NFTNL_CHAIN_TABLE),
			type == NFT_MSG_NEWCHAIN ? 'N' : 'X',
			nftnl_chain_get_str(c, NFTNL_CHAIN_NAME));
err_free:
	nftnl_chain_free(c);
err:
	return MNL_CB_OK;
}

static int newgen_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t genid = 0, pid = 0;
	const struct nlattr *attr;
	const char *name = NULL;

	mnl_attr_for_each(attr, nlh, sizeof(struct nfgenmsg)) {
		switch (mnl_attr_get_type(attr)) {
		case NFTA_GEN_ID:
			if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
				break;
		        genid = ntohl(mnl_attr_get_u32(attr));
			break;
		case NFTA_GEN_PROC_NAME:
			if (mnl_attr_validate(attr, MNL_TYPE_NUL_STRING) < 0)
				break;
			name = mnl_attr_get_str(attr);
			break;
		case NFTA_GEN_PROC_PID:
			if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
				break;
			pid = ntohl(mnl_attr_get_u32(attr));
			break;
		}
	}

	if (name)
		printf("NEWGEN: GENID=%u PID=%u NAME=%s\n", genid, pid, name);

	return MNL_CB_OK;
}

static void trace_print_return(const struct nftnl_trace *nlt)
{
	const char *chain = NULL;

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_JUMP_TARGET)) {
		chain = nftnl_trace_get_str(nlt, NFTNL_TRACE_JUMP_TARGET);
		printf("%s", chain);
	}
}

static void trace_print_rule(const struct nftnl_trace *nlt, struct cb_arg *args)
{
	uint64_t handle = nftnl_trace_get_u64(nlt, NFTNL_TRACE_RULE_HANDLE);
	uint32_t family = nftnl_trace_get_u32(nlt, NFTNL_TRACE_FAMILY);
	const char *table = nftnl_trace_get_str(nlt, NFTNL_TRACE_TABLE);
	const char *chain = nftnl_trace_get_str(nlt, NFTNL_TRACE_CHAIN);
        struct nftnl_rule *r;
	struct mnl_socket *nl;
	struct nlmsghdr *nlh;
	uint32_t portid;
	char buf[16536];
	int ret;

        r = nftnl_rule_alloc();
	if (r == NULL) {
		perror("OOM");
		exit(EXIT_FAILURE);
	}

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETRULE, family, 0, 0);

        nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);
	nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, chain);
	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, table);
	nftnl_rule_set_u64(r, NFTNL_RULE_HANDLE, handle);
	nftnl_rule_nlmsg_build_payload(nlh, r);
	nftnl_rule_free(r);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	portid = mnl_socket_get_portid(nl);
	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret > 0) {
		args->is_event = false;
		ret = mnl_cb_run(buf, ret, 0, portid, rule_cb, args);
	}
	if (ret == -1) {
		perror("error");
		exit(EXIT_FAILURE);
	}
	mnl_socket_close(nl);
}

static void trace_print_packet(const struct nftnl_trace *nlt, struct cb_arg *args)
{
	struct list_head stmts = LIST_HEAD_INIT(stmts);
	uint32_t nfproto, family;
	uint16_t l4proto = 0;
	uint32_t mark;
	char name[IFNAMSIZ];

	family = nftnl_trace_get_u32(nlt, NFTNL_TRACE_FAMILY);
	printf("PACKET: %d %08x ", family, nftnl_trace_get_u32(nlt, NFTNL_TRACE_ID));

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_IIF))
		printf("IN=%s ", if_indextoname(nftnl_trace_get_u32(nlt, NFTNL_TRACE_IIF), name));
	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_OIF))
		printf("OUT=%s ", if_indextoname(nftnl_trace_get_u32(nlt, NFTNL_TRACE_OIF), name));

	nfproto = family;
	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_NFPROTO)) {
		nfproto = nftnl_trace_get_u32(nlt, NFTNL_TRACE_NFPROTO);

		if (family != nfproto)
			printf("NFPROTO=%d ", nfproto);
	}

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_LL_HEADER)) {
		const struct ethhdr *eh;
		const char *linklayer;
		uint32_t i, len;
		uint16_t type = nftnl_trace_get_u16(nlt, NFTNL_TRACE_IIFTYPE);

		linklayer = nftnl_trace_get_data(nlt, NFTNL_TRACE_LL_HEADER, &len);
		switch (type) {
		case ARPHRD_ETHER:
			if (len < sizeof(*eh))
			       break;
			eh = (const void *)linklayer;
			printf("MACSRC=%s ", ether_ntoa((const void *)eh->h_source));
			printf("MACDST=%s ", ether_ntoa((const void *)eh->h_dest));
			printf("MACPROTO=%04x ", ntohs(eh->h_proto));
			break;
		case ARPHRD_LOOPBACK:
			printf("LOOPBACK ");
			break;
		default:
			printf("LL=0x%x ", type);
			for (i = 0 ; i < len; i++)
				printf("%02x", linklayer[i]);
			printf(" ");
			break;
		}
	}

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_NETWORK_HEADER)) {
		const struct ip6_hdr *ip6h;
		const struct iphdr *iph;
		uint32_t i, len;
		const char *nh;

		ip6h = nftnl_trace_get_data(nlt, NFTNL_TRACE_NETWORK_HEADER, &len);

		switch (nfproto) {
		case NFPROTO_IPV4: {
			char addrbuf[INET_ADDRSTRLEN];

			if (len < sizeof(*iph))
				break;
			iph = (const void *)ip6h;


			inet_ntop(AF_INET, &iph->saddr, addrbuf, sizeof(addrbuf));
			printf("SRC=%s ", addrbuf);
			inet_ntop(AF_INET, &iph->daddr, addrbuf, sizeof(addrbuf));
			printf("DST=%s ", addrbuf);

			printf("LEN=%d TOS=0x%x TTL=%d ID=%d ", ntohs(iph->tot_len), iph->tos, iph->ttl, ntohs(iph->id));
			if (iph->frag_off & htons(0x8000))
				printf("CE ");
			if (iph->frag_off & htons(IP_DF))
				printf("DF ");
			if (iph->frag_off & htons(IP_MF))
				printf("MF ");

			if (ntohs(iph->frag_off) & 0x1fff)
				printf("FRAG:%u ", ntohs(iph->frag_off) & 0x1fff);

			l4proto = iph->protocol;
			if (iph->ihl * 4 > sizeof(*iph)) {
				unsigned int optsize;
				const char *op;

				optsize = iph->ihl * 4 - sizeof(*iph);
				op = (const char *)iph;
				op += sizeof(*iph);

				printf("OPT (");
				for (i = 0; i < optsize; i++)
					printf("%02X", op[i]);
				printf(") ");
			}
			break;
		}
		case NFPROTO_IPV6: {
			uint32_t flowlabel = ntohl(*(uint32_t *)ip6h);
			char addrbuf[INET6_ADDRSTRLEN];

			if (len < sizeof(*ip6h))
				break;

			inet_ntop(AF_INET6, &ip6h->ip6_src, addrbuf, sizeof(addrbuf));
			printf("SRC=%s ", addrbuf);
			inet_ntop(AF_INET6, &ip6h->ip6_dst, addrbuf, sizeof(addrbuf));
			printf("DST=%s ", addrbuf);

			printf("LEN=%zu TC=%u HOPLIMIT=%u FLOWLBL=%u ",
				ntohs(ip6h->ip6_plen) + sizeof(*iph),
				(flowlabel & 0x0ff00000) >> 20,
				ip6h->ip6_hops,
				flowlabel & 0x000fffff);

			l4proto = ip6h->ip6_nxt;
			break;
		}
		default:
			nh = (const char *)ip6h;
			printf("NH=");
			for (i = 0 ; i < len; i++)
				printf("%02x", nh[i]);
			printf(" ");
		}
	}

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_TRANSPORT_HEADER)) {
		const struct tcphdr *tcph;
		uint32_t len;

		tcph = nftnl_trace_get_data(nlt, NFTNL_TRACE_TRANSPORT_HEADER, &len);

		switch (l4proto) {
		case IPPROTO_DCCP:
		case IPPROTO_SCTP:
		case IPPROTO_UDPLITE:
		case IPPROTO_UDP:
			if (len < 4)
				break;
			printf("SPORT=%d DPORT=%d ", ntohs(tcph->source), ntohs(tcph->dest));
			break;
		case IPPROTO_TCP:
			if (len < sizeof(*tcph))
				break;
			printf("SPORT=%d DPORT=%d ", ntohs(tcph->source), ntohs(tcph->dest));
			if (tcph->syn)
				printf("SYN ");
			if (tcph->ack)
				printf("ACK ");
			if (tcph->fin)
				printf("FIN ");
			if (tcph->rst)
				printf("RST ");
			if (tcph->psh)
				printf("PSH ");
			if (tcph->urg)
				printf("URG ");
			break;
		default:
			break;
		}
	}

	mark = nftnl_trace_get_u32(nlt, NFTNL_TRACE_MARK);
	if (mark)
		printf("MARK=0x%x ", mark);
	puts("");
}

static void trace_print_hdr(const struct nftnl_trace *nlt)
{
	printf(" TRACE: %d %08x %s:%s", nftnl_trace_get_u32(nlt, NFTNL_TABLE_FAMILY),
					nftnl_trace_get_u32(nlt, NFTNL_TRACE_ID),
					nftnl_trace_get_str(nlt, NFTNL_TRACE_TABLE),
					nftnl_trace_get_str(nlt, NFTNL_TRACE_CHAIN));
}

static void print_verdict(const struct nftnl_trace *nlt, uint32_t verdict)
{
	const char *chain;

	switch (verdict) {
	case NF_ACCEPT:
		printf("ACCEPT");
		break;
	case NF_DROP:
		printf("DROP");
		break;
	case NF_QUEUE:
		printf("QUEUE");
		break;
	case NF_STOLEN:
		printf("STOLEN");
		break;
	case NFT_BREAK:
		printf("BREAK");
		break;
	case NFT_CONTINUE:
		printf("CONTINUE");
		break;
	case NFT_GOTO:
		printf("GOTO");
		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_JUMP_TARGET)) {
			chain = nftnl_trace_get_str(nlt, NFTNL_TRACE_JUMP_TARGET);
			printf(":%s", chain);
		}
		break;
	case NFT_JUMP:
		printf("JUMP");
		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_JUMP_TARGET)) {
			chain = nftnl_trace_get_str(nlt, NFTNL_TRACE_JUMP_TARGET);
			printf(":%s", chain);
		}
		break;
	default:
		printf("0x%x", verdict);
		break;
	}

	printf(" ");
}

static int trace_cb(const struct nlmsghdr *nlh, struct cb_arg *arg)
{
	struct nftnl_trace *nlt;
	uint32_t verdict;

	nlt = nftnl_trace_alloc();
	if (nlt == NULL)
		goto err;

	if (nftnl_trace_nlmsg_parse(nlh, nlt) < 0)
		goto err_free;

	if (arg->nfproto &&
	    arg->nfproto != nftnl_trace_get_u32(nlt, NFTNL_TABLE_FAMILY))
		goto err_free;

	switch (nftnl_trace_get_u32(nlt, NFTNL_TRACE_TYPE)) {
	case NFT_TRACETYPE_RULE:
		verdict = nftnl_trace_get_u32(nlt, NFTNL_TRACE_VERDICT);

		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_LL_HEADER) ||
		    nftnl_trace_is_set(nlt, NFTNL_TRACE_NETWORK_HEADER))
			trace_print_packet(nlt, arg);

		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_RULE_HANDLE)) {
			trace_print_hdr(nlt);
			printf(":rule:0x%" PRIx64":", nftnl_trace_get_u64(nlt, NFTNL_TRACE_RULE_HANDLE));
			print_verdict(nlt, verdict);
			printf(" ");
			trace_print_rule(nlt, arg);
		}
		break;
	case NFT_TRACETYPE_POLICY:
		trace_print_hdr(nlt);
		printf(":policy:");
		verdict = nftnl_trace_get_u32(nlt, NFTNL_TRACE_POLICY);

		print_verdict(nlt, verdict);
		puts("");
		break;
	case NFT_TRACETYPE_RETURN:
		trace_print_hdr(nlt);
		printf(":return:");
		trace_print_return(nlt);
		puts("");
		break;
	}
err_free:
	nftnl_trace_free(nlt);
err:
	return MNL_CB_OK;
}

static int monitor_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t type = nlh->nlmsg_type & 0xFF;
	struct cb_arg *arg = data;
	int ret = MNL_CB_OK;

	switch(type) {
	case NFT_MSG_NEWTABLE:
	case NFT_MSG_DELTABLE:
		ret = table_cb(nlh, data);
		break;
	case NFT_MSG_NEWCHAIN:
	case NFT_MSG_DELCHAIN:
		ret = chain_cb(nlh, data);
		break;
	case NFT_MSG_NEWRULE:
	case NFT_MSG_DELRULE:
		arg->is_event = true;
		ret = rule_cb(nlh, data);
		break;
	case NFT_MSG_NEWGEN:
		ret = newgen_cb(nlh, data);
		break;
	case NFT_MSG_TRACE:
		ret = trace_cb(nlh, data);
		break;
	}

	fflush(stdout);
	return ret;
}

static const struct option options[] = {
	{.name = "counters", .has_arg = false, .val = 'c'},
	{.name = "trace", .has_arg = false, .val = 't'},
	{.name = "event", .has_arg = false, .val = 'e'},
	{.name = "ipv4", .has_arg = false, .val = '4'},
	{.name = "ipv6", .has_arg = false, .val = '6'},
	{.name = "version", .has_arg = false, .val = 'V'},
	{.name = "help", .has_arg = false, .val = 'h'},
	{NULL},
};

static void print_usage(void)
{
	printf("%s %s\n", xtables_globals.program_name,
			  xtables_globals.program_version);
	printf("Usage: %s [ -t | -e ]\n"
	       "        --trace    -t    trace ruleset traversal of packets tagged via -j TRACE rule\n"
	       "        --event    -e    show events that modify the ruleset\n"
	       "Optional arguments:\n"
	       "        --ipv4     -4    only monitor IPv4\n"
	       "        --ipv6     -6    only monitor IPv6\n"
	       "	--counters -c    show counters in rules\n"

	       , xtables_globals.program_name);
	exit(EXIT_FAILURE);
}

int xtables_monitor_main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	uint32_t nfgroup = 0;
	struct nft_handle h = {};
	struct cb_arg cb_arg = {
		.h = &h,
	};
	int ret, c;

	xtables_globals.program_name = "xtables-monitor";
	/* XXX xtables_init_all does several things we don't want */
	c = xtables_init_all(&xtables_globals, NFPROTO_IPV4);
	if (c < 0) {
		fprintf(stderr, "%s/%s Failed to initialize xtables\n",
				xtables_globals.program_name,
				xtables_globals.program_version);
		exit(1);
	}
	init_extensions();
	init_extensions4();
	init_extensions6();
	init_extensionsa();
	init_extensionsb();

	if (nft_init(&h, AF_INET)) {
		fprintf(stderr, "%s/%s Failed to initialize nft: %s\n",
			xtables_globals.program_name,
			xtables_globals.program_version,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	opterr = 0;
	while ((c = getopt_long(argc, argv, "ceht46V", options, NULL)) != -1) {
		switch (c) {
	        case 'c':
			counters = true;
			break;
	        case 't':
			trace = true;
			break;
	        case 'e':
			events = true;
			break;
	        case 'h':
			print_usage();
			exit(0);
		case '4':
			cb_arg.nfproto = NFPROTO_IPV4;
			break;
		case '6':
			cb_arg.nfproto = NFPROTO_IPV6;
			break;
		case 'V':
			printf("xtables-monitor %s\n", PACKAGE_VERSION);
			exit(0);
		default:
			fprintf(stderr, "xtables-monitor %s: Bad argument.\n", PACKAGE_VERSION);
			fprintf(stderr, "Try `xtables-monitor -h' for more information.\n");
			exit(PARAMETER_PROBLEM);
		}
	}

	if (trace)
		nfgroup |= 1 << (NFNLGRP_NFTRACE - 1);
	if (events)
		nfgroup |= 1 << (NFNLGRP_NFTABLES - 1);

	if (nfgroup == 0) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("cannot open nfnetlink socket");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, nfgroup, MNL_SOCKET_AUTOPID) < 0) {
		perror("cannot bind to nfnetlink socket");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, 0, 0, monitor_cb, &cb_arg);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("cannot receive from nfnetlink socket");
		exit(EXIT_FAILURE);
	}
	mnl_socket_close(nl);

	xtables_fini();

	return EXIT_SUCCESS;
}

