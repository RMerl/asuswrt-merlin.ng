/*
 * (C) 2010-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Based on: kernel-space FTP extension for connection tracking.
 *
 * This port has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 *
 * Original copyright notice:
 *
 * (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 * (C) 2003,2004 USAGI/WIDE Project <http://www.linux-ipv6.org>
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

static bool loose; /* XXX: export this as config option. */

#define NUM_SEQ_TO_REMEMBER 2

/* This structure exists only once per master */
struct ftp_info {
	/* Valid seq positions for cmd matching after newline */
	uint32_t seq_aft_nl[MYCT_DIR_MAX][NUM_SEQ_TO_REMEMBER];
	/* 0 means seq_match_aft_nl not set */
	int seq_aft_nl_num[MYCT_DIR_MAX];
};

enum nf_ct_ftp_type {
	/* PORT command from client */
	NF_CT_FTP_PORT,
	/* PASV response from server */
	NF_CT_FTP_PASV,
	/* EPRT command from client */
	NF_CT_FTP_EPRT,
	/* EPSV response from server */
	NF_CT_FTP_EPSV,
};

static int
get_ipv6_addr(const char *src, size_t dlen, struct in6_addr *dst, uint8_t term)
{
	const char *end;
	int ret = in6_pton(src, min_t(size_t, dlen, 0xffff),
				(uint8_t *)dst, term, &end);
	if (ret > 0)
		return (int)(end - src);
	return 0;
}

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
			/* Unexpected character; true if it's the
			   terminator and we're finished. */
			if (*data == term && i == array_size - 1)
				return len;
			pr_debug("Char %u (got %u nums) `%u' unexpected\n",
				 len, i, *data);
			return 0;
		}
	}
	pr_debug("Failed to fill %u numbers separated by %c\n",
		 array_size, sep);
	return 0;
}

/* Grab port: number up to delimiter */
static int get_port(const char *data, int start, size_t dlen, char delim,
		    struct myct_man *cmd)
{
	uint16_t tmp_port = 0;
	uint32_t i;

	for (i = start; i < dlen; i++) {
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
		else { /* Some other crap */
			pr_debug("get_port: invalid char.\n");
			break;
		}
	}
	return 0;
}

/* Returns 0, or length of numbers: 192,168,1,1,5,6 */
static int try_rfc959(const char *data, size_t dlen, struct myct_man *cmd,
		      uint16_t l3protonum, char term)
{
	int length;
	uint32_t array[6];

	length = try_number(data, dlen, array, 6, ',', term);
	if (length == 0)
		return 0;

	cmd->u3.ip =  htonl((array[0] << 24) | (array[1] << 16) |
			   (array[2] << 8) | array[3]);
	cmd->u.port = htons((array[4] << 8) | array[5]);
	return length;
}

/* Returns 0, or length of numbers: |1|132.235.1.2|6275| or |2|3ffe::1|6275| */
static int try_eprt(const char *data, size_t dlen,
		    struct myct_man *cmd, uint16_t l3protonum, char term)
{
	char delim;
	int length;

	/* First character is delimiter, then "1" for IPv4 or "2" for IPv6,
	   then delimiter again. */
	if (dlen <= 3) {
		pr_debug("EPRT: too short\n");
		return 0;
	}
	delim = data[0];
	if (isdigit(delim) || delim < 33 || delim > 126 || data[2] != delim) {
		pr_debug("try_eprt: invalid delimitter.\n");
		return 0;
	}

	if ((l3protonum == PF_INET && data[1] != '1') ||
	    (l3protonum == PF_INET6 && data[1] != '2')) {
		pr_debug("EPRT: invalid protocol number.\n");
		return 0;
	}

	pr_debug("EPRT: Got %c%c%c\n", delim, data[1], delim);
	if (data[1] == '1') {
		uint32_t array[4];

		/* Now we have IP address. */
		length = try_number(data + 3, dlen - 3, array, 4, '.', delim);
		if (length != 0)
			cmd->u3.ip = htonl((array[0] << 24) | (array[1] << 16)
					   | (array[2] << 8) | array[3]);
	} else {
		/* Now we have IPv6 address. */
		length = get_ipv6_addr(data + 3, dlen - 3,
				       (struct in6_addr *)cmd->u3.ip6, delim);
	}

	if (length == 0)
		return 0;
	pr_debug("EPRT: Got IP address!\n");
	/* Start offset includes initial "|1|", and trailing delimiter */
	return get_port(data, 3 + length + 1, dlen, delim, cmd);
}

/* Returns 0, or length of numbers: |||6446| */
static int try_epsv_response(const char *data, size_t dlen,
			     struct myct_man *cmd,
			     uint16_t l3protonum, char term)
{
	char delim;

	/* Three delimiters. */
	if (dlen <= 3) return 0;
	delim = data[0];
	if (isdigit(delim) || delim < 33 || delim > 126 ||
	    data[1] != delim || data[2] != delim)
		return 0;

	return get_port(data, 3, dlen, delim, cmd);
}

static struct ftp_search {
	const char *pattern;
	size_t plen;
	char skip;
	char term;
	enum nf_ct_ftp_type ftptype;
	int (*getnum)(const char *, size_t, struct myct_man *, uint16_t, char);
} search[MYCT_DIR_MAX][2] = {
	[MYCT_DIR_ORIG] = {
		{
			.pattern	= "PORT",
			.plen		= sizeof("PORT") - 1,
			.skip		= ' ',
			.term		= '\r',
			.ftptype	= NF_CT_FTP_PORT,
			.getnum		= try_rfc959,
		},
		{
			.pattern	= "EPRT",
			.plen		= sizeof("EPRT") - 1,
			.skip		= ' ',
			.term		= '\r',
			.ftptype	= NF_CT_FTP_EPRT,
			.getnum		= try_eprt,
		},
	},
	[MYCT_DIR_REPL] = {
		{
			.pattern	= "227 ",
			.plen		= sizeof("227 ") - 1,
			.skip		= '(',
			.term		= ')',
			.ftptype	= NF_CT_FTP_PASV,
			.getnum		= try_rfc959,
		},
		{
			.pattern	= "229 ",
			.plen		= sizeof("229 ") - 1,
			.skip		= '(',
			.term		= ')',
			.ftptype	= NF_CT_FTP_EPSV,
			.getnum		= try_epsv_response,
		},
	},
};

static int ftp_find_pattern(struct pkt_buff *pkt,
			    unsigned int dataoff, unsigned int dlen,
			    const char *pattern, size_t plen,
			    char skip, char term,
			    unsigned int *matchoff, unsigned int *matchlen,
			    struct myct_man *cmd,
			    int (*getnum)(const char *, size_t,
					  struct myct_man *cmd,
					  uint16_t, char),
			    int dir)
{
	char *data = (char *)pktb_network_header(pkt) + dataoff;
	int numlen;
	uint32_t i;

	if (dlen == 0)
		return 0;

	/* short packet, skip partial matching. */
	if (dlen <= plen)
		return 0;

	if (strncmp(data, pattern, plen) != 0)
		return 0;

	pr_debug("Pattern matches!\n");

	/* Now we've found the constant string, try to skip
	   to the 'skip' character */
	for (i = plen; data[i] != skip; i++)
		if (i == dlen - 1) return 0;

	/* Skip over the last character */
	i++;

	pr_debug("Skipped up to `%c'!\n", skip);

	numlen = getnum(data + i, dlen - i, cmd, PF_INET, term);
	if (!numlen)
		return 0;

	*matchoff = i;
	*matchlen = numlen;

	pr_debug("Match succeded!\n");
	return 1;
}

/* Look up to see if we're just after a \n. */
static int find_nl_seq(uint32_t seq, struct ftp_info *info, int dir)
{
	int i;

	for (i = 0; i < info->seq_aft_nl_num[dir]; i++)
		if (info->seq_aft_nl[dir][i] == seq)
			return 1;
	return 0;
}

/* We don't update if it's older than what we have. */
static void update_nl_seq(uint32_t nl_seq, struct ftp_info *info, int dir)
{
	int i, oldest;

	/* Look for oldest: if we find exact match, we're done. */
	for (i = 0; i < info->seq_aft_nl_num[dir]; i++) {
		if (info->seq_aft_nl[dir][i] == nl_seq)
			return;
	}

	if (info->seq_aft_nl_num[dir] < NUM_SEQ_TO_REMEMBER) {
		info->seq_aft_nl[dir][info->seq_aft_nl_num[dir]++] = nl_seq;
	} else {
		if (before(info->seq_aft_nl[dir][0], info->seq_aft_nl[dir][1]))
			oldest = 0;
		else
			oldest = 1;

		if (after(nl_seq, info->seq_aft_nl[dir][oldest]))
			info->seq_aft_nl[dir][oldest] = nl_seq;
	}
}

static int nf_nat_ftp_fmt_cmd(enum nf_ct_ftp_type type,
			      char *buffer, size_t buflen,
			      uint32_t addr, uint16_t port)
{
	switch (type) {
	case NF_CT_FTP_PORT:
	case NF_CT_FTP_PASV:
		return snprintf(buffer, buflen, "%u,%u,%u,%u,%u,%u",
				((unsigned char *)&addr)[0],
				((unsigned char *)&addr)[1],
				((unsigned char *)&addr)[2],
				((unsigned char *)&addr)[3],
				port >> 8,
				port & 0xFF);
	case NF_CT_FTP_EPRT:
		return snprintf(buffer, buflen, "|1|%u.%u.%u.%u|%u|",
				((unsigned char *)&addr)[0],
				((unsigned char *)&addr)[1],
				((unsigned char *)&addr)[2],
				((unsigned char *)&addr)[3],
				port);
	case NF_CT_FTP_EPSV:
		return snprintf(buffer, buflen, "|||%u|", port);
	}

	return 0;
}

/* So, this packet has hit the connection tracking matching code.
   Mangle it, and change the expectation to match the new version. */
static unsigned int nf_nat_ftp(struct pkt_buff *pkt,
			       int dir,
			       int ctinfo,
			       enum nf_ct_ftp_type type,
			       unsigned int matchoff,
			       unsigned int matchlen,
			       struct nf_conntrack *ct,
			       struct nf_expect *exp)
{
	union nfct_attr_grp_addr newip;
	uint16_t port;
	char buffer[sizeof("|1|255.255.255.255|65535|")];
	unsigned int buflen;
	const struct nf_conntrack *expected;
	struct nf_conntrack *nat_tuple;
	uint16_t initial_port;

	pr_debug("FTP_NAT: type %i, off %u len %u\n", type, matchoff, matchlen);

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

	buflen = nf_nat_ftp_fmt_cmd(type, buffer, sizeof(buffer),
					newip.ip, port);
	if (!buflen)
		goto out;

	if (!nfq_tcp_mangle_ipv4(pkt, matchoff, matchlen, buffer, buflen))
		goto out;

	return NF_ACCEPT;

out:
	cthelper_del_expect(exp);
	return NF_DROP;
}

static int
ftp_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
	      struct myct *myct, uint32_t ctinfo)
{
	struct tcphdr *th;
	unsigned int dataoff;
	unsigned int matchoff = 0, matchlen = 0; /* makes gcc happy. */
	unsigned int datalen;
	unsigned int i;
	int found = 0, ends_in_nl;
	uint32_t seq;
	int ret = NF_ACCEPT;
	struct myct_man cmd;
	union nfct_attr_grp_addr addr;
	union nfct_attr_grp_addr daddr;
	int dir = CTINFO2DIR(ctinfo);
	struct ftp_info *ftp_info = myct->priv_data;
	struct nf_expect *exp = NULL;

	memset(&cmd, 0, sizeof(struct myct_man));
	memset(&addr, 0, sizeof(union nfct_attr_grp_addr));

	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED &&
	    ctinfo != IP_CT_ESTABLISHED_REPLY) {
		pr_debug("ftp: Conntrackinfo = %u\n", ctinfo);
		goto out;
	}

	th = (struct tcphdr *) (pktb_network_header(pkt) + protoff);

	dataoff = protoff + th->doff * 4;
	datalen = pktb_len(pkt) - dataoff;

	ends_in_nl = (pktb_network_header(pkt)[pktb_len(pkt) - 1] == '\n');
	seq = ntohl(th->seq) + datalen;

	/* Look up to see if we're just after a \n. */
	if (!find_nl_seq(ntohl(th->seq), ftp_info, dir)) {
		/* Now if this ends in \n, update ftp info. */
		pr_debug("nf_conntrack_ftp: wrong seq pos %s(%u) or %s(%u)\n",
		ftp_info->seq_aft_nl_num[dir] > 0 ? "" : "(UNSET)",
		ftp_info->seq_aft_nl[dir][0],
		ftp_info->seq_aft_nl_num[dir] > 1 ? "" : "(UNSET)",
		ftp_info->seq_aft_nl[dir][1]);
		goto out_update_nl;
	}

	/* Initialize IP/IPv6 addr to expected address (it's not mentioned
	   in EPSV responses) */
	cmd.l3num = nfct_get_attr_u16(myct->ct, ATTR_L3PROTO);
	nfct_get_attr_grp(myct->ct, ATTR_GRP_ORIG_ADDR_SRC, &cmd.u3);

	for (i = 0; i < ARRAY_SIZE(search[dir]); i++) {
		found = ftp_find_pattern(pkt, dataoff, datalen,
					 search[dir][i].pattern,
					 search[dir][i].plen,
					 search[dir][i].skip,
					 search[dir][i].term,
					 &matchoff, &matchlen,
					 &cmd,
					 search[dir][i].getnum,
					 dir);
		if (found) break;
	}
	if (found == 0) /* No match */
		goto out_update_nl;

	pr_debug("conntrack_ftp: match `%.*s' (%u bytes at %u)\n",
		 matchlen, pktb_network_header(pkt) + dataoff + matchoff,
		 matchlen, ntohl(th->seq) + matchoff);

	/* We refer to the reverse direction ("!dir") tuples here,
	 * because we're expecting something in the other direction.
	 * Doesn't matter unless NAT is happening.  */
	cthelper_get_addr_dst(myct->ct, !dir, &daddr);

	cthelper_get_addr_src(myct->ct, dir, &addr);

	/* Update the ftp info */
	if ((cmd.l3num == nfct_get_attr_u16(myct->ct, ATTR_L3PROTO)) &&
	    memcmp(&cmd.u3, &addr, sizeof(addr)) != 0) {
		/* Enrico Scholz's passive FTP to partially RNAT'd ftp
		   server: it really wants us to connect to a
		   different IP address.  Simply don't record it for
		   NAT. */
		if (cmd.l3num == PF_INET) {
			pr_debug("conntrack_ftp: NOT RECORDING: %pI4 != %pI4\n",
				 &cmd.u3.ip, &addr);
		} else {
			pr_debug("conntrack_ftp: NOT RECORDING: %pI6 != %pI6\n",
				 cmd.u3.ip6, &addr);
		}
		/* Thanks to Cristiano Lincoln Mattos
		   <lincoln@cesar.org.br> for reporting this potential
		   problem (DMZ machines opening holes to internal
		   networks, or the packet filter itself). */
		if (!loose) {
			ret = NF_ACCEPT;
			goto out;
		}
		memcpy(&daddr, &cmd.u3, sizeof(cmd.u3));
	}

	exp = nfexp_new();
	if (exp == NULL)
		goto out_update_nl;

	cthelper_get_addr_src(myct->ct, !dir, &addr);

	if (cthelper_expect_init(exp, myct->ct, 0, &addr, &daddr, IPPROTO_TCP,
				 NULL, &cmd.u.port, 0)) {
		pr_debug("conntrack_ftp: failed to init expectation\n");
		goto out_update_nl;
	}

	/* Now, NAT might want to mangle the packet, and register the
	 * (possibly changed) expectation itself. */
	if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_NAT_MASK) {
		ret = nf_nat_ftp(pkt, dir, ctinfo, search[dir][i].ftptype,
				 matchoff, matchlen, myct->ct, exp);
		goto out_update_nl;
	}

	/* Can't expect this?  Best to drop packet now. */
	if (cthelper_add_expect(exp) < 0) {
		pr_debug("conntrack_ftp: cannot add expectation: %s\n",
			strerror(errno));
		ret = NF_DROP;
		goto out_update_nl;
	}

out_update_nl:
	if (exp != NULL)
		nfexp_destroy(exp);

	/* Now if this ends in \n, update ftp info.  Seq may have been
	 * adjusted by NAT code. */
	if (ends_in_nl)
		update_nl_seq(seq, ftp_info, dir);
out:
	return ret;
}

static struct ctd_helper ftp_helper = {
	.name		= "ftp",
	.l4proto	= IPPROTO_TCP,
	.cb		= ftp_helper_cb,
	.priv_data_len	= sizeof(struct ftp_info),
	.policy		= {
		[0] = {
			.name			= "ftp",
			.expect_max		= 1,
			.expect_timeout		= 300,
		},
	},
};

void __attribute__ ((constructor)) ftp_init(void);

void ftp_init(void)
{
	helper_register(&ftp_helper);
}
