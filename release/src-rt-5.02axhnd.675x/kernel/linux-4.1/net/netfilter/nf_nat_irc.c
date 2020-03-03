/* IRC extension for TCP NAT alteration.
 *
 * (C) 2000-2001 by Harald Welte <laforge@gnumonks.org>
 * (C) 2004 Rusty Russell <rusty@rustcorp.com.au> IBM Corporation
 * based on a copy of RR's ip_nat_ftp.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/tcp.h>
#include <linux/kernel.h>

#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <linux/netfilter/nf_conntrack_irc.h>

MODULE_AUTHOR("Harald Welte <laforge@gnumonks.org>");
MODULE_DESCRIPTION("IRC (DCC) NAT helper");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_nat_irc");

static unsigned int help(struct sk_buff *skb,
			 enum ip_conntrack_info ctinfo,
			 unsigned int protoff,
			 unsigned int matchoff,
			 unsigned int matchlen,
			 struct nf_conntrack_expect *exp)
{
	char buffer[sizeof("4294967296 65635")];
	struct nf_conn *ct = exp->master;
	union nf_inet_addr newaddr;
	u_int16_t port;
	unsigned int ret;

	/* Reply comes from server. */
	newaddr = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3;

	exp->saved_proto.tcp.port = exp->tuple.dst.u.tcp.port;
	exp->dir = IP_CT_DIR_REPLY;
	exp->expectfn = nf_nat_follow_master;

	/* Try to get same port: if not, try to change it. */
	for (port = ntohs(exp->saved_proto.tcp.port); port != 0; port++) {
		int ret;

		exp->tuple.dst.u.tcp.port = htons(port);
		ret = nf_ct_expect_related(exp);
		if (ret == 0)
			break;
		else if (ret != -EBUSY) {
			port = 0;
			break;
		}
	}

	if (port == 0) {
		nf_ct_helper_log(skb, ct, "all ports in use");
		return NF_DROP;
	}

	/* strlen("\1DCC CHAT chat AAAAAAAA P\1\n")=27
	 * strlen("\1DCC SCHAT chat AAAAAAAA P\1\n")=28
	 * strlen("\1DCC SEND F AAAAAAAA P S\1\n")=26
	 * strlen("\1DCC MOVE F AAAAAAAA P S\1\n")=26
	 * strlen("\1DCC TSEND F AAAAAAAA P S\1\n")=27
	 *
	 * AAAAAAAAA: bound addr (1.0.0.0==16777216, min 8 digits,
	 *                        255.255.255.255==4294967296, 10 digits)
	 * P:         bound port (min 1 d, max 5d (65635))
	 * F:         filename   (min 1 d )
	 * S:         size       (min 1 d )
	 * 0x01, \n:  terminators
	 */
	/* AAA = "us", ie. where server normally talks to. */
	snprintf(buffer, sizeof(buffer), "%u %u", ntohl(newaddr.ip), port);
	pr_debug("nf_nat_irc: inserting '%s' == %pI4, port %u\n",
		 buffer, &newaddr.ip, port);

	ret = nf_nat_mangle_tcp_packet(skb, ct, ctinfo, protoff, matchoff,
				       matchlen, buffer, strlen(buffer));
	if (ret != NF_ACCEPT) {
		nf_ct_helper_log(skb, ct, "cannot mangle packet");
		nf_ct_unexpect_related(exp);
	}

	return ret;
}

static void __exit nf_nat_irc_fini(void)
{
	RCU_INIT_POINTER(nf_nat_irc_hook, NULL);
	synchronize_rcu();
}

static int __init nf_nat_irc_init(void)
{
	BUG_ON(nf_nat_irc_hook != NULL);
	RCU_INIT_POINTER(nf_nat_irc_hook, help);
	return 0;
}

/* Prior to 2.6.11, we had a ports param.  No longer, but don't break users. */
static int warn_set(const char *val, struct kernel_param *kp)
{
	printk(KERN_INFO KBUILD_MODNAME
	       ": kernel >= 2.6.10 only uses 'ports' for conntrack modules\n");
	return 0;
}
module_param_call(ports, warn_set, NULL, NULL, 0);

module_init(nf_nat_irc_init);
module_exit(nf_nat_irc_fini);
