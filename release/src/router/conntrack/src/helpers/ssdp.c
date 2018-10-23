/*
 * SSDP/UPnP connection tracking helper
 * (SSDP = Simple Service Discovery Protocol)
 * For documentation about SSDP see
 * http://en.wikipedia.org/wiki/Simple_Service_Discovery_Protocol
 *
 * Copyright (C) 2014 Ashley Hughes <ashley.hughes@blueyonder.co.uk>
 * Based on the SSDP conntrack helper (nf_conntrack_ssdp.c),
 * :http://marc.info/?t=132945775100001&r=1&w=2
 *  (C) 2012 Ian Pilcher <arequipeno@gmail.com>
 * Copyright (C) 2017 Google Inc.
 *
 * This requires Linux 3.12 or higher.  Basic usage:
 *
 *     nfct add helper ssdp inet udp
 *     nfct add helper ssdp inet tcp
 *     iptables -t raw -A OUTPUT -p udp --dport 1900 -j CT --helper ssdp
 *     iptables -t raw -A PREROUTING -p udp --dport 1900 -j CT --helper ssdp
 *
 * This helper supports SNAT when used in conjunction with a daemon that
 * forwards SSDP broadcasts/replies between interfaces, e.g.
 * https://chromium.googlesource.com/chromiumos/platform2/+/master/arc-networkd/multicast_forwarder.h
 *
 * If UPnP eventing is used, callbacks should be triggered at regular
 * intervals to prevent the expectation from expiring.  It will expire
 * after min(nf_conntrack_tcp_timeout_time_wait, ExpectTimeout) which
 * is min(120, 300) = 120 by default.  The latter option can be changed
 * in the conntrackd configuration file; the former option can be changed
 * via procfs or through policy:
 *
 *     nfct timeout add long-timewait inet tcp \
 *         established 1000 close 10 time_wait 10 last_ack 10
 *     nfct timeout add long-timewait inet tcp time_wait 3600
 *     iptables -t raw -A OUTPUT -p udp --dport 1900 -j CT --helper ssdp \
 *         --timeout long-timewait
 *     iptables -t raw -A PREROUTING -p udp --dport 1900 -j CT --helper ssdp \
 *         --timeout long-timewait
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "conntrackd.h"
#include "helper.h"
#include "myct.h"
#include "log.h"
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#define _GNU_SOURCE
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_tcp.h>
#include <libnetfilter_queue/pktbuff.h>
#include <linux/netfilter.h>

#define SSDP_MCAST_ADDR		"239.255.255.250"
#define UPNP_MCAST_LL_ADDR	"FF02::C" /* link-local */
#define UPNP_MCAST_SL_ADDR	"FF05::C" /* site-local */

#define SSDP_M_SEARCH		"M-SEARCH"
#define SSDP_M_SEARCH_SIZE	(sizeof SSDP_M_SEARCH - 1)

/* So, this packet has hit the connection tracking matching code.
   Mangle it, and change the expectation to match the new version. */
static unsigned int nf_nat_ssdp(struct pkt_buff *pkt,
				int ctinfo,
				unsigned int matchoff,
				unsigned int matchlen,
				struct nf_conntrack *ct,
				struct nf_expect *exp)
{
	union nfct_attr_grp_addr newip;
	uint16_t port;
	int dir = CTINFO2DIR(ctinfo);
	char buffer[sizeof("255.255.255.255:65535")];
	unsigned int buflen;
	const struct nf_conntrack *expected;
	struct nf_conntrack *nat_tuple;
	uint16_t initial_port;

	/* Connection will come from wherever this packet goes, hence !dir */
	cthelper_get_addr_dst(ct, !dir, &newip);

	expected = nfexp_get_attr(exp, ATTR_EXP_EXPECTED);

	nat_tuple = nfct_new();
	if (nat_tuple == NULL)
		return NF_ACCEPT;

	initial_port = nfct_get_attr_u16(expected, ATTR_PORT_DST);

	/* pkt is NULL for NOTIFY (renewal, same dir), non-NULL otherwise */
	nfexp_set_attr_u32(exp, ATTR_EXP_NAT_DIR, pkt ? !dir : dir);

	/* libnetfilter_conntrack needs this */
	nfct_set_attr_u8(nat_tuple, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(nat_tuple, ATTR_IPV4_SRC, 0);
	nfct_set_attr_u32(nat_tuple, ATTR_IPV4_DST, 0);
	nfct_set_attr_u8(nat_tuple, ATTR_L4PROTO,
			 nfct_get_attr_u8(ct, ATTR_L4PROTO));
	nfct_set_attr_u16(nat_tuple, ATTR_PORT_DST, 0);

	/* When you see the packet, we need to NAT it the same as the
	   this one. */
	nfexp_set_attr(exp, ATTR_EXP_FN, "nat-follow-master");

	/* Try to get same port: if not, try to change it. */
	for (port = ntohs(initial_port); port != 0; port++) {
		int ret;

		nfct_set_attr_u16(nat_tuple, ATTR_PORT_SRC, htons(port));
		nfexp_set_attr(exp, ATTR_EXP_NAT_TUPLE, nat_tuple);

		ret = cthelper_add_expect(exp);
		if (ret == 0)
			break;
		else if (ret != -EBUSY) {
			port = 0;
			break;
		}
	}

	if (port == 0)
		return NF_DROP;

	/* Only the SUBSCRIBE request contains an IP string that needs to be
	   mangled. */
	if (!matchoff)
		return NF_ACCEPT;

	buflen = snprintf(buffer, sizeof(buffer),
				"%u.%u.%u.%u:%u",
                                ((unsigned char *)&newip.ip)[0],
                                ((unsigned char *)&newip.ip)[1],
                                ((unsigned char *)&newip.ip)[2],
                                ((unsigned char *)&newip.ip)[3], port);
	if (!buflen)
		goto out;

	if (!nfq_tcp_mangle_ipv4(pkt, matchoff, matchlen, buffer, buflen))
		goto out;

	return NF_ACCEPT;

out:
	cthelper_del_expect(exp);
	return NF_DROP;
}

static int handle_ssdp_new(struct pkt_buff *pkt, uint32_t protoff,
			   struct myct *myct, uint32_t ctinfo)
{
	int ret = NF_ACCEPT;
	union nfct_attr_grp_addr daddr, saddr, taddr;
	struct iphdr *net_hdr = (struct iphdr *)pktb_network_header(pkt);
	int good_packet = 0;
	struct nf_expect *exp;
	uint16_t port;
	unsigned int dataoff;
	void *sb_ptr;

	cthelper_get_addr_dst(myct->ct, MYCT_DIR_ORIG, &daddr);
	switch (nfct_get_attr_u8(myct->ct, ATTR_L3PROTO)) {
	case AF_INET:
		inet_pton(AF_INET, SSDP_MCAST_ADDR, &(taddr.ip));
		if (daddr.ip == taddr.ip)
			good_packet = 1;
		break;
	case AF_INET6:
		inet_pton(AF_INET6, UPNP_MCAST_LL_ADDR, &(taddr.ip6));
		if (daddr.ip6[0] == taddr.ip6[0] &&
		    daddr.ip6[1] == taddr.ip6[1] &&
		    daddr.ip6[2] == taddr.ip6[2] &&
		    daddr.ip6[3] == taddr.ip6[3]) {
			good_packet = 1;
			break;
		}
		inet_pton(AF_INET6, UPNP_MCAST_SL_ADDR, &(taddr.ip6));
		if (daddr.ip6[0] == taddr.ip6[0] &&
		    daddr.ip6[1] == taddr.ip6[1] &&
		    daddr.ip6[2] == taddr.ip6[2] &&
		    daddr.ip6[3] == taddr.ip6[3]) {
			good_packet = 1;
			break;
		}
		break;
	default:
		break;
	}

	if (!good_packet) {
		pr_debug("ssdp_help: destination address not multicast; ignoring\n");
		return NF_ACCEPT;
	}

	/* No data? Ignore */
	dataoff = net_hdr->ihl*4 + sizeof(struct udphdr);
	if (dataoff >= pktb_len(pkt)) {
		pr_debug("ssdp_help: UDP payload too small for M-SEARCH; ignoring\n");
		return NF_ACCEPT;
	}

	sb_ptr = pktb_network_header(pkt) + dataoff;

	if (memcmp(sb_ptr, SSDP_M_SEARCH, SSDP_M_SEARCH_SIZE) != 0) {
		pr_debug("ssdp_help: UDP payload does not begin with 'M-SEARCH'; ignoring\n");
		return NF_ACCEPT;
	}

	cthelper_get_addr_src(myct->ct, MYCT_DIR_ORIG, &saddr);
	cthelper_get_port_src(myct->ct, MYCT_DIR_ORIG, &port);

	exp = nfexp_new();
	if (exp == NULL)
		return NF_DROP;

	if (cthelper_expect_init(exp, myct->ct, 0, NULL, &saddr,
				 IPPROTO_UDP, NULL, &port,
				 NF_CT_EXPECT_PERMANENT)) {
		nfexp_destroy(exp);
		return NF_DROP;
	}
	nfexp_set_attr(exp, ATTR_EXP_HELPER_NAME, "ssdp");
	if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_SRC_NAT)
		return nf_nat_ssdp(pkt, ctinfo, 0, 0, myct->ct, exp);

	myct->exp = exp;

	return ret;
}

static int find_hdr(const char *name, const uint8_t *data, int data_len,
		    char *val, int val_len, const uint8_t **pos)
{
	int name_len = strlen(name);
	int i;

	while (1) {
		if (data_len < name_len + 2)
			return -1;

		if (strncasecmp(name, (char *)data, name_len) == 0)
			break;

		for (i = 0; ; i++) {
			if (i >= data_len - 1)
				return -1;
			if (data[i] == '\r' && data[i+1] == '\n')
				break;
		}

		data_len -= i+2;
		data += i+2;
	}

	data_len -= name_len;
	data += name_len;
	if (pos)
		*pos = data;

	for (i = 0; ; i++, val_len--) {
		if (!val_len)
			return -1;
		if (*data == '\r') {
			*val = 0;
			return 0;
		}
		*(val++) = *(data++);
	}
}

static int parse_url(const char *url,
		     uint8_t l3proto,
		     union nfct_attr_grp_addr *addr,
		     uint16_t *port,
		     size_t *match_offset,
		     size_t *match_len)
{
	const char *start = url, *end;
	size_t ip_len;

	if (strncasecmp(url, "http://[", 8) == 0) {
		char buf[64] = {0};

		if (l3proto != AF_INET6) {
			pr_debug("conntrack_ssdp: IPv6 URL in IPv4 SSDP reply\n");
			return -1;
		}

		url += 8;

		end = strchr(url, ']');
		if (!end) {
			pr_debug("conntrack_ssdp: unterminated IPv6 address: '%s'\n", url);
			return -1;
		}

		ip_len = end - url;
		if (ip_len > sizeof(buf) - 1) {
			pr_debug("conntrack_ssdp: IPv6 address too long: '%s'\n", url);
			return -1;
		}
		strncpy(buf, url, ip_len);

		if (inet_pton(AF_INET6, buf, addr) != 1) {
			pr_debug("conntrack_ssdp: Error parsing IPv6 address: '%s'\n", buf);
			return -1;
		}
	} else if (strncasecmp(url, "http://", 7) == 0) {
		char buf[64] = {0};

		if (l3proto != AF_INET) {
			pr_debug("conntrack_ssdp: IPv4 URL in IPv6 SSDP reply\n");
			return -1;
		}

		url += 7;
		for (end = url; ; end++) {
			if (*end != '.' && *end != '\0' &&
			    (*end < '0' || *end > '9'))
				break;
		}

		ip_len = end - url;
		if (ip_len > sizeof(buf) - 1) {
			pr_debug("conntrack_ssdp: IPv4 address too long: '%s'\n", url);
			return -1;
		}
		strncpy(buf, url, ip_len);

		if (inet_pton(AF_INET, buf, addr) != 1) {
			pr_debug("conntrack_ssdp: Error parsing IPv4 address: '%s'\n", buf);
			return -1;
		}
	} else {
		pr_debug("conntrack_ssdp: header does not start with http://\n");
		return -1;
	}

	if (match_offset)
		*match_offset = url - start;

	if (*end != ':') {
		*port = htons(80);
		if (match_len)
			*match_len = ip_len;
	} else {
		char *endptr = NULL;
		*port = htons(strtol(end + 1, &endptr, 10));
		if (match_len)
			*match_len = ip_len + endptr - end;;
	}

	return 0;
}

static int handle_ssdp_reply(struct pkt_buff *pkt, uint32_t protoff,
			     struct myct *myct, uint32_t ctinfo)
{
	uint8_t *data = pktb_network_header(pkt);
	size_t bytes_left = pktb_len(pkt);
	char hdr_val[256];
	union nfct_attr_grp_addr addr;
	uint16_t port;
	struct nf_expect *exp = NULL;

	if (bytes_left < protoff + sizeof(struct udphdr)) {
		pr_debug("conntrack_ssdp: Short packet\n");
		return NF_ACCEPT;
	}
	bytes_left -= protoff + sizeof(struct udphdr);
	data += protoff + sizeof(struct udphdr);

	if (find_hdr("LOCATION: ", data, bytes_left,
		     hdr_val, sizeof(hdr_val), NULL) < 0) {
		pr_debug("conntrack_ssdp: No LOCATION header found\n");
		return NF_ACCEPT;
	}
	pr_debug("conntrack_ssdp: found location URL `%s'\n", hdr_val);

	if (parse_url(hdr_val, nfct_get_attr_u8(myct->ct, ATTR_L3PROTO),
		      &addr, &port, NULL, NULL) < 0) {
		pr_debug("conntrack_ssdp: Error parsing URL\n");
		return NF_ACCEPT;
	}

	exp = nfexp_new();
	if (cthelper_expect_init(exp,
				 myct->ct,
				 0 /* class */,
				 NULL /* saddr */,
				 &addr /* daddr */,
				 IPPROTO_TCP,
				 NULL /* sport */,
				 &port /* dport */,
				 NF_CT_EXPECT_PERMANENT /* flags */) < 0) {
		pr_debug("conntrack_ssdp: Failed to init expectation\n");
		nfexp_destroy(exp);
		return NF_ACCEPT;
	}

	nfexp_set_attr(exp, ATTR_EXP_HELPER_NAME, "ssdp");
	if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_SRC_NAT)
		return nf_nat_ssdp(pkt, ctinfo, 0, 0, myct->ct, exp);

	myct->exp = exp;
	return NF_ACCEPT;
}

static int renew_exp(struct myct *myct, uint32_t ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
	union nfct_attr_grp_addr saddr = {0}, daddr = {0};
	uint16_t sport, dport;
	struct nf_expect *exp = nfexp_new();

	pr_debug("conntrack_ssdp: Renewing NOTIFY expectation\n");

	cthelper_get_addr_src(myct->ct, dir, &saddr);
	cthelper_get_addr_dst(myct->ct, dir, &daddr);
	cthelper_get_port_src(myct->ct, dir, &sport);
	cthelper_get_port_dst(myct->ct, dir, &dport);

	if (cthelper_expect_init(exp,
				 myct->ct,
				 0 /* class */,
				 &saddr /* saddr */,
				 &daddr /* daddr */,
				 IPPROTO_TCP,
				 NULL /* sport */,
				 &dport /* dport */,
				 0 /* flags */) < 0) {
		pr_debug("conntrack_ssdp: Failed to init expectation\n");
		nfexp_destroy(exp);
		return NF_ACCEPT;
	}

	nfexp_set_attr(exp, ATTR_EXP_HELPER_NAME, "ssdp");
	if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_DST_NAT)
		return nf_nat_ssdp(NULL, ctinfo, 0, 0, myct->ct, exp);

	myct->exp = exp;
	return NF_ACCEPT;
}

static int handle_http_request(struct pkt_buff *pkt, uint32_t protoff,
			       struct myct *myct, uint32_t ctinfo)
{
	struct tcphdr *th;
	unsigned int dataoff, datalen;
	const uint8_t *data;
	char hdr_val[256];
	union nfct_attr_grp_addr cbaddr = {0}, daddr = {0}, saddr = {0};
	uint16_t cbport;
	struct nf_expect *exp = NULL;
	const uint8_t *hdr_pos;
	size_t ip_offset, ip_len;
	int dir = CTINFO2DIR(ctinfo);

	th = (struct tcphdr *) (pktb_network_header(pkt) + protoff);
	dataoff = protoff + th->doff * 4;
	datalen = pktb_len(pkt) - dataoff;
	data = pktb_network_header(pkt) + dataoff;

	if (datalen >= 7 && strncmp((char *)data, "NOTIFY ", 7) == 0)
		return renew_exp(myct, ctinfo);

	if (datalen < 10 || strncmp((char *)data, "SUBSCRIBE ", 10) != 0)
		return NF_ACCEPT;

	if (find_hdr("CALLBACK: <", data, datalen,
		     hdr_val, sizeof(hdr_val), &hdr_pos) < 0) {
		pr_debug("conntrack_ssdp: No CALLBACK header found\n");
		return NF_ACCEPT;
	}
	pr_debug("conntrack_ssdp: found callback URL `%s'\n", hdr_val);

	if (parse_url(hdr_val, nfct_get_attr_u8(myct->ct, ATTR_L3PROTO),
		      &cbaddr, &cbport, &ip_offset, &ip_len) < 0) {
		pr_debug("conntrack_ssdp: Error parsing URL\n");
		return NF_ACCEPT;
	}

	cthelper_get_addr_dst(myct->ct, !dir, &daddr);
	cthelper_get_addr_src(myct->ct, dir, &saddr);

	if (memcmp(&saddr, &cbaddr, sizeof(cbaddr)) != 0) {
		pr_debug("conntrack_ssdp: Callback address belongs to another host\n");
		return NF_ACCEPT;
	}

	cthelper_get_addr_src(myct->ct, !dir, &saddr);

	exp = nfexp_new();
	if (cthelper_expect_init(exp,
				 myct->ct,
				 0 /* class */,
				 &saddr /* saddr */,
				 &daddr /* daddr */,
				 IPPROTO_TCP,
				 NULL /* sport */,
				 &cbport /* dport */,
				 0 /* flags */) < 0) {
		pr_debug("conntrack_ssdp: Failed to init expectation\n");
		nfexp_destroy(exp);
		return NF_ACCEPT;
	}

	nfexp_set_attr(exp, ATTR_EXP_HELPER_NAME, "ssdp");
	if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_SRC_NAT) {
		return nf_nat_ssdp(pkt, ctinfo,
				   (hdr_pos - data) + ip_offset,
				   ip_len, myct->ct, exp);
	}

	myct->exp = exp;
	return NF_ACCEPT;
}

static int ssdp_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
			  struct myct *myct, uint32_t ctinfo)
{
	uint8_t proto;

	/* All new UDP conntracks are M-SEARCH queries. */
	if (ctinfo == IP_CT_NEW)
		return handle_ssdp_new(pkt, protoff, myct, ctinfo);

	proto = nfct_get_attr_u16(myct->ct, ATTR_ORIG_L4PROTO);

	/* All existing UDP conntracks are replies to an M-SEARCH query.
	   M-SEARCH queries often generate replies from multiple devices
	   on the LAN. */
	if (proto == IPPROTO_UDP)
		return handle_ssdp_reply(pkt, protoff, myct, ctinfo);
	else {
		/* TCP conntracks can represent:
		 *
		 *  - SUBSCRIBE requests (control point -> device) containing a
		 *    callback URL.  These create an expectation that allows
		 *    the NOTIFY callbacks to pass.
		 *  - NOTIFY callbacks (device -> control point), which
		 *    "auto-renew" the expectation
		 *  - Some other HTTP request (don't care)
		 *
		 * Currently all TCP conntracks are scanned for SUBSCRIBE
		 * and NOTIFY requests.  This is not ideal, because we do
		 * not want callbacks to be able to create new expectations
		 * on a different port.  Fixing this will require convincing
		 * the kernel to pass private state data for related
		 * conntracks. */
		if (ctinfo == IP_CT_ESTABLISHED)
			return handle_http_request(pkt, protoff, myct, ctinfo);
		else
			return NF_ACCEPT;
	}

	/* Not reached. */
	return NF_DROP;
}

static struct ctd_helper ssdp_helper_udp = {
	.name		= "ssdp",
	.l4proto	= IPPROTO_UDP,
	.priv_data_len	= 0,
	.cb		= ssdp_helper_cb,
	.policy		= {
		[0] = {
			.name		= "ssdp",
			.expect_max	= 8,
			.expect_timeout	= 5 * 60,
		},
	},
};

static struct ctd_helper ssdp_helper_tcp = {
	.name		= "ssdp",
	.l4proto	= IPPROTO_TCP,
	.priv_data_len	= 0,
	.cb		= ssdp_helper_cb,
	.policy		= {
		[0] = {
			.name		= "ssdp",
			.expect_max	= 8,
			.expect_timeout	= 5 * 60,
		},
	},
};

static void __attribute__ ((constructor)) ssdp_init(void)
{
	helper_register(&ssdp_helper_udp);
	helper_register(&ssdp_helper_tcp);
}
