/*
 * (C) 2012 by Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Sponsored by Vyatta Inc. <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "conntrackd.h"
#include "network.h"	/* for before and after */
#include "helper.h"
#include "myct.h"
#include "log.h"

#include <ctype.h>	/* for isdigit */
#include <errno.h>

#define _GNU_SOURCE
#include <netinet/tcp.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_tcp.h>
#include <libnetfilter_queue/pktbuff.h>
#include <linux/netfilter.h>

/* TNS SQL*Net Version 2 */
enum tns_types {
       TNS_TYPE_CONNECT        = 1,
       TNS_TYPE_ACCEPT	       = 2,
       TNS_TYPE_ACK	       = 3,
       TNS_TYPE_REFUSE	       = 4,
       TNS_TYPE_REDIRECT       = 5,
       TNS_TYPE_DATA	       = 6,
       TNS_TYPE_NULL	       = 7,
       TNS_TYPE_ABORT	       = 9,
       TNS_TYPE_RESEND	       = 11,
       TNS_TYPE_MARKER	       = 12,
       TNS_TYPE_ATTENTION      = 13,
       TNS_TYPE_CONTROL        = 14,
       TNS_TYPE_MAX	       = 19,
};

struct tns_header {
       uint16_t len;
       uint16_t csum;
       uint8_t type;
       uint8_t reserved;
       uint16_t header_csum;
};

struct tns_redirect {
       uint16_t data_len;
};

struct tns_info {
       /* Scan next DATA|REDIRECT packet */
       bool parse;
};

static int try_number(const char *data, size_t dlen, uint32_t array[],
		      int array_size, char sep, char term)
{
	uint32_t len;
	int i;

	memset(array, 0, sizeof(array[0])*array_size);

	/* Keep data pointing at next char. */
	for (i = 0, len = 0; len < dlen && i < array_size; len++, data++) {
		if (*data >= '0' && *data <= '9') {
			array[i] = array[i]*10 + *data - '0';
		}
		else if (*data == sep)
			i++;
		else {
			/* Skip spaces. */
			if (*data == ' ')
				continue;
			/* Unexpected character; true if it's the
			   terminator and we're finished. */
			if (*data == term && i == array_size - 1)
				return len;
			pr_debug("Char %u (got %u nums) `%c' unexpected\n",
				 len, i, *data);
			return 0;
		}
	}
	pr_debug("Failed to fill %u numbers separated by %c\n",
		 array_size, sep);
	return 0;
}

/* Grab port: number up to delimiter */
static int get_port(const char *data, size_t dlen, char delim,
		    struct myct_man *cmd)
{
	uint16_t tmp_port = 0;
	uint32_t i;

	for (i = 0; i < dlen; i++) {
		/* Finished? */
		if (data[i] == delim) {
			if (tmp_port == 0)
				break;
			cmd->u.port = htons(tmp_port);
			pr_debug("get_port: return %d\n", tmp_port);
			return i + 1;
		}
		else if (data[i] >= '0' && data[i] <= '9')
			tmp_port = tmp_port*10 + data[i] - '0';
		else if (data[i] == ' ') /* Skip spaces */
			continue;
		else { /* Some other crap */
			pr_debug("get_port: invalid char `%c'\n", data[i]);
			break;
		}
	}
	return 0;
}

/* (ADDRESS=(PROTOCOL=tcp)(DEV=x)(HOST=a.b.c.d)(PORT=a)) */
/* FIXME: handle hostnames */

/* Returns 0, or length of port number */
static unsigned int
find_pattern(struct pkt_buff *pkt, unsigned int dataoff, size_t dlen,
	     struct myct_man *cmd, unsigned int *numoff)
{
	const char *data = (const char *)pktb_network_header(pkt) + dataoff
						+ sizeof(struct tns_header);
	int length, offset, ret;
	uint32_t array[4];
	const char *p, *start;

	p = strstr(data, "(");
	if (!p)
		return 0;

	p = strstr(p+1, "HOST=");
	if (!p) {
		pr_debug("HOST= not found\n");
		return 0;
	}

	start = p + strlen("HOST=");
	offset = (int)(p - data) + strlen("HOST=");
	*numoff = offset + sizeof(struct tns_header);
	data += offset;

	length = try_number(data, dlen - offset, array, 4, '.', ')');
	if (length == 0)
		return 0;

	cmd->u3.ip = htonl((array[0] << 24) | (array[1] << 16) |
			   (array[2] << 8) | array[3]);

	p = strstr(data+length, "(");
	if (!p)
		return 0;

	p = strstr(p, "PORT=");
	if (!p) {
		pr_debug("PORT= not found\n");
		return 0;
	}

	p += strlen("PORT=");
	ret = get_port(p, dlen - offset - length, ')', cmd);
	if (ret == 0)
		return 0;

	p += ret;
	return (int)(p - start);
}

static inline uint16_t
nton(uint16_t len, unsigned int matchoff, unsigned int matchlen)
{
	uint32_t l = (uint32_t)ntohs(len) + matchoff - matchlen;

	return htons(l);
}

/* So, this packet has hit the connection tracking matching code.
   Mangle it, and change the expectation to match the new version. */
static unsigned int
nf_nat_tns(struct pkt_buff *pkt, struct tns_header *tns, struct nf_expect *exp,
	   struct nf_conntrack *ct, int dir,
	   unsigned int matchoff, unsigned int matchlen)
{
	union nfct_attr_grp_addr newip;
	char buffer[sizeof("255.255.255.255)(PORT=65535)")];
	unsigned int buflen;
	const struct nf_conntrack *expected;
	struct nf_conntrack *nat_tuple;
	uint16_t initial_port, port;

	/* Connection will come from wherever this packet goes, hence !dir */
	cthelper_get_addr_dst(ct, !dir, &newip);

	expected = nfexp_get_attr(exp, ATTR_EXP_EXPECTED);

	nat_tuple = nfct_new();
	if (nat_tuple == NULL)
		return NF_ACCEPT;

	initial_port = nfct_get_attr_u16(expected, ATTR_PORT_DST);

	nfexp_set_attr_u32(exp, ATTR_EXP_NAT_DIR, !dir);

	/* libnetfilter_conntrack needs this */
	nfct_set_attr_u8(nat_tuple, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(nat_tuple, ATTR_IPV4_SRC, 0);
	nfct_set_attr_u32(nat_tuple, ATTR_IPV4_DST, 0);
	nfct_set_attr_u8(nat_tuple, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(nat_tuple, ATTR_PORT_DST, 0);

	/* When you see the packet, we need to NAT it the same as the
	 * this one. */
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
	nfct_destroy(nat_tuple);

	if (port == 0)
		return NF_DROP;

	buflen = snprintf(buffer, sizeof(buffer),
				"%u.%u.%u.%u)(PORT=%u)",
                                ((unsigned char *)&newip.ip)[0],
                                ((unsigned char *)&newip.ip)[1],
                                ((unsigned char *)&newip.ip)[2],
                                ((unsigned char *)&newip.ip)[3], port);
	if (!buflen)
		goto out;

	if (!nfq_tcp_mangle_ipv4(pkt, matchoff, matchlen, buffer, buflen))
		goto out;

	if (buflen != matchlen) {
		/* FIXME: recalculate checksum */
		tns->csum = 0;
		tns->header_csum = 0;

		tns->len = nton(tns->len,  matchlen, buflen);
		if (tns->type == TNS_TYPE_REDIRECT) {
			struct tns_redirect *r;

			r = (struct tns_redirect *)((char *)tns + sizeof(struct tns_header));

			r->data_len = nton(r->data_len, matchlen, buflen);
		}
	}

	return NF_ACCEPT;

out:
	cthelper_del_expect(exp);
	return NF_DROP;
}

static int
tns_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
	      struct myct *myct, uint32_t ctinfo)
{
	struct tcphdr *th;
	struct tns_header *tns;
	int dir = CTINFO2DIR(ctinfo);
	unsigned int dataoff, datalen, numoff = 0, numlen;
	struct tns_info *tns_info = myct->priv_data;
	union nfct_attr_grp_addr addr;
	struct nf_expect *exp = NULL;
	struct myct_man cmd;
	int ret = NF_ACCEPT;

	memset(&cmd, 0, sizeof(struct myct_man));
	memset(&addr, 0, sizeof(union nfct_attr_grp_addr));

	/* Until there's been traffic both ways, don't look into TCP packets. */
	if (ctinfo != IP_CT_ESTABLISHED
	    && ctinfo != IP_CT_ESTABLISHED_REPLY) {
		pr_debug("TNS: Conntrackinfo = %u\n", ctinfo);
		goto out;
	}
	/* Parse server direction only */
	if (dir != MYCT_DIR_REPL) {
		pr_debug("TNS: skip client direction\n");
		goto out;
	}

	th = (struct tcphdr *) (pktb_network_header(pkt) + protoff);

	dataoff = protoff + th->doff * 4;
	datalen = pktb_len(pkt);

	if (datalen < sizeof(struct tns_header)) {
		pr_debug("TNS: skip packet with short header\n");
		goto out;
	}

	tns = (struct tns_header *)(pktb_network_header(pkt) + dataoff);

	if (tns->type == TNS_TYPE_REDIRECT) {
		struct tns_redirect *redirect;

		dataoff += sizeof(struct tns_header);
		datalen -= sizeof(struct tns_header);
		redirect = (struct tns_redirect *)(pktb_network_header(pkt) + dataoff);
		tns_info->parse = false;

		if (ntohs(redirect->data_len) == 0) {
			tns_info->parse = true;
			goto out;
		}
		goto parse;
	}

	/* Parse only the very next DATA packet */
	if (!(tns_info->parse && tns->type == TNS_TYPE_DATA)) {
		tns_info->parse = false;
		goto out;
	}
parse:
	numlen = find_pattern(pkt, dataoff, datalen, &cmd, &numoff);
	tns_info->parse = false;
	if (!numlen)
		goto out;

	/* We refer to the reverse direction ("!dir") tuples here,
	 * because we're expecting something in the other direction.
	 * Doesn't matter unless NAT is happening.  */
	cthelper_get_addr_src(myct->ct, !dir, &addr);

	exp = nfexp_new();
	if (exp == NULL)
		goto out;

	if (cthelper_expect_init(exp, myct->ct, 0,
				 &addr, &cmd.u3,
				 IPPROTO_TCP,
				 NULL, &cmd.u.port, 0)) {
		pr_debug("TNS: failed to init expectation\n");
		goto out_exp;
	}

	/* Now, NAT might want to mangle the packet, and register the
	 * (possibly changed) expectation itself.
	 */
	if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_NAT_MASK) {
		ret = nf_nat_tns(pkt, tns, exp, myct->ct, dir,
				numoff + sizeof(struct tns_header), numlen);
		goto out_exp;
	}

	/* Can't expect this?  Best to drop packet now. */
	if (cthelper_add_expect(exp) < 0) {
		pr_debug("TNS: cannot add expectation: %s\n",
			 strerror(errno));
		ret = NF_DROP;
		goto out_exp;
	}
	goto out;

out_exp:
	nfexp_destroy(exp);
out:
	return ret;
}

static struct ctd_helper tns_helper = {
	.name		= "tns",
	.l4proto	= IPPROTO_TCP,
	.cb		= tns_helper_cb,
	.priv_data_len	= sizeof(struct tns_info),
	.policy		= {
		[0] = {
			.name		= "tns",
			.expect_max	= 1,
			.expect_timeout	= 300,
		},
	},
};

void __attribute__ ((constructor)) tns_init(void);

void tns_init(void)
{
	helper_register(&tns_helper);
}
