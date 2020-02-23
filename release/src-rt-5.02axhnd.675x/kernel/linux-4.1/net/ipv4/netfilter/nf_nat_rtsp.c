#if defined(CONFIG_BCM_KF_NETFILTER)
/*
 * RTSP extension for NAT alteration.
 *
 * Copyright (c) 2008 Broadcom Corporation.
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license 
 * agreement governing use of this software, this software is licensed 
 * to you under the terms of the GNU General Public License version 2 
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php, 
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give 
 *    you permission to link this software with independent modules, and 
 *    to copy and distribute the resulting executable under terms of your 
 *    choice, provided that you also meet, for each linked independent 
 *    module, the terms and conditions of the license of that module. 
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications 
 *    of the software.  
 * 
 * Not withstanding the above, under no circumstances may you combine 
 * this software in any way with any other Broadcom software provided 
 * under a license other than the GPL, without Broadcom's express prior 
 * written consent. 
 * 
 * :>
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/tcp.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <linux/netfilter/nf_conntrack_rtsp.h>

static inline int rtsp_mangle_tcp_packet(struct sk_buff *skb,
					   struct nf_conn *ct,
					   enum ip_conntrack_info ctinfo,
					   unsigned int protoff,
					   unsigned int match_offset,
					   unsigned int match_len,
					   const char *rep_buffer,
					   unsigned int rep_len)
{
	return __nf_nat_mangle_tcp_packet(skb, ct, ctinfo, protoff,
					  match_offset, match_len,
					  rep_buffer, rep_len, false);
}

/****************************************************************************/
static int modify_ports(struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			enum ip_conntrack_info ctinfo,
			int matchoff, int matchlen,
			u_int16_t rtpport, u_int16_t rtcpport,
			char dash, int *delta)
{
	char buf[sizeof("65535-65535")];
	int len;

	if (dash)
		len = sprintf(buf, "%hu%c%hu", rtpport, dash, rtcpport);
	else
		len = sprintf(buf, "%hu", rtpport);
	if (!rtsp_mangle_tcp_packet(skb, ct, ctinfo, protoff, matchoff, matchlen,
				      buf, len)) {
		if (net_ratelimit())
			printk("nf_nat_rtsp: rtsp_mangle_tcp_packet error\n");
		return -1;
	}
	*delta = len - matchlen;
	return 0;
}

/* Setup NAT on this expected conntrack so it follows master but expect the src ip. */
/* If we fail to get a free NAT slot, we'll get dropped on confirm */
static void nf_nat_follow_master_nosrc(struct nf_conn *ct,
                          struct nf_conntrack_expect *exp)
{
	struct nf_nat_range range;

	/* This must be a fresh one. */
	BUG_ON(ct->status & IPS_NAT_DONE_MASK);

	/* For DST manip, map port here to where it's expected. */
	range.flags = (NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED);
	range.min_proto = range.max_proto = exp->saved_proto;
	range.min_addr = range.max_addr
		= ct->master->tuplehash[!exp->dir].tuple.src.u3;
	nf_nat_setup_info(ct, &range, NF_NAT_MANIP_DST);
}

/****************************************************************************/
/* One data channel */
static int nat_rtsp_channel (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			     enum ip_conntrack_info ctinfo,
			     unsigned int matchoff, unsigned int matchlen,
			     struct nf_conntrack_expect *rtp_exp, int *delta)
{
	struct nf_conn_help *help = nfct_help(ct);
	struct nf_conntrack_expect *exp;
	int dir = CTINFO2DIR(ctinfo);
	u_int16_t nated_port = 0;
	int exp_exist = 0;

	/* Set expectations for NAT */
	rtp_exp->saved_proto.udp.port = rtp_exp->tuple.dst.u.udp.port;
	rtp_exp->expectfn = nf_nat_follow_master_nosrc;
	rtp_exp->dir = !dir;

	/* Lookup existing expects */
	spin_lock_bh(&nf_conntrack_expect_lock);
	hlist_for_each_entry(exp, &help->expectations, lnode) {
		if (exp->saved_proto.udp.port == rtp_exp->saved_proto.udp.port){
			/* Expectation already exists */ 
			rtp_exp->tuple.dst.u.udp.port = 
				exp->tuple.dst.u.udp.port;
			nated_port = ntohs(exp->tuple.dst.u.udp.port);
			exp_exist = 1;
			break;
		}
	}
	spin_unlock_bh(&nf_conntrack_expect_lock);

	if (exp_exist) {
		nf_ct_expect_related(rtp_exp);
		goto modify_message;
	}

	/* Try to get a port. */
	for (nated_port = ntohs(rtp_exp->tuple.dst.u.udp.port);
	     nated_port != 0; nated_port++) {
		rtp_exp->tuple.dst.u.udp.port = htons(nated_port);
		if (nf_ct_expect_related(rtp_exp) == 0)
			break;
	}

	if (nated_port == 0) {	/* No port available */
		if (net_ratelimit())
			printk("nf_nat_rtsp: out of UDP ports\n");
		return 0;
	}

modify_message:
	/* Modify message */
	if (modify_ports(skb, protoff, ct, ctinfo, matchoff, matchlen,
			 nated_port, 0, 0, delta) < 0) {
		nf_ct_unexpect_related(rtp_exp);
		return -1;
	}

	/* Success */
	pr_debug("nf_nat_rtsp: expect RTP ");
	nf_ct_dump_tuple(&rtp_exp->tuple);

	return 0;
}

/****************************************************************************/
/* A pair of data channels (RTP/RTCP) */
static int nat_rtsp_channel2 (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			      enum ip_conntrack_info ctinfo,
			      unsigned int matchoff, unsigned int matchlen,
			      struct nf_conntrack_expect *rtp_exp,
			      struct nf_conntrack_expect *rtcp_exp,
			      char dash, int *delta)
{
	struct nf_conn_help *help = nfct_help(ct);
	struct nf_conntrack_expect *exp;
	int dir = CTINFO2DIR(ctinfo);
	u_int16_t nated_port = 0;
	int exp_exist = 0;

	/* Set expectations for NAT */
	rtp_exp->saved_proto.udp.port = rtp_exp->tuple.dst.u.udp.port;
	rtp_exp->expectfn = nf_nat_follow_master_nosrc;
	rtp_exp->dir = !dir;
	rtcp_exp->saved_proto.udp.port = rtcp_exp->tuple.dst.u.udp.port;
	rtcp_exp->expectfn = nf_nat_follow_master_nosrc;
	rtcp_exp->dir = !dir;

	/* Lookup existing expects */
	spin_lock_bh(&nf_conntrack_expect_lock);
	hlist_for_each_entry(exp, &help->expectations, lnode) {
		if (exp->saved_proto.udp.port == rtp_exp->saved_proto.udp.port){
			/* Expectation already exists */ 
			rtp_exp->tuple.dst.u.udp.port = 
				exp->tuple.dst.u.udp.port;
			rtcp_exp->tuple.dst.u.udp.port = 
				htons(ntohs(exp->tuple.dst.u.udp.port) + 1);
			nated_port = ntohs(exp->tuple.dst.u.udp.port);
			exp_exist = 1;
			break;
		}
	}
	spin_unlock_bh(&nf_conntrack_expect_lock);

	if (exp_exist) {
		nf_ct_expect_related(rtp_exp);
		nf_ct_expect_related(rtcp_exp);
		goto modify_message;
	}

	/* Try to get a pair of ports. */
	for (nated_port = ntohs(rtp_exp->tuple.dst.u.udp.port) & (~1);
	     nated_port != 0; nated_port += 2) {
		rtp_exp->tuple.dst.u.udp.port = htons(nated_port);
		if (nf_ct_expect_related(rtp_exp) == 0) {
			rtcp_exp->tuple.dst.u.udp.port =
			    htons(nated_port + 1);
			if (nf_ct_expect_related(rtcp_exp) == 0)
				break;
			nf_ct_unexpect_related(rtp_exp);
		}
	}

	if (nated_port == 0) {	/* No port available */
		if (net_ratelimit())
			printk("nf_nat_rtsp: out of RTP/RTCP ports\n");
		return 0;
	}

modify_message:
	/* Modify message */
	if (modify_ports(skb, protoff, ct, ctinfo, matchoff, matchlen,
			 nated_port, nated_port + 1, dash, delta) < 0) {
		nf_ct_unexpect_related(rtp_exp);
		nf_ct_unexpect_related(rtcp_exp);
		return -1;
	}

	/* Success */
	pr_debug("nf_nat_rtsp: expect RTP ");
	nf_ct_dump_tuple(&rtp_exp->tuple);
	pr_debug("nf_nat_rtsp: expect RTCP ");
	nf_ct_dump_tuple(&rtcp_exp->tuple);

	return 0;
}

/****************************************************************************/
static __be16 lookup_mapping_port(struct nf_conn *ct,
				  enum ip_conntrack_info ctinfo,
				  __be16 port)
{
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	struct nf_conn_help *help = nfct_help(ct);
	struct nf_conntrack_expect *exp;
	struct nf_conn *child;
	__be16 proto_all;

	spin_lock_bh(&nf_conntrack_expect_lock);
	/* Lookup existing expects */
	pr_debug("nf_nat_rtsp: looking up existing expectations...\n");
	hlist_for_each_entry(exp, &help->expectations, lnode) {
		if (exp->tuple.dst.u.udp.port == port) {
			pr_debug("nf_nat_rtsp: found port %hu mapped from "
					"%hu\n",
					ntohs(exp->tuple.dst.u.udp.port),
					ntohs(exp->saved_proto.all));

			proto_all = exp->saved_proto.all;
			spin_unlock_bh(&nf_conntrack_expect_lock);
			return proto_all;
		}
	}
	spin_unlock_bh(&nf_conntrack_expect_lock);

	/* Lookup existing connections */
	pr_debug("nf_nat_rtsp: looking up existing connections...\n");
	list_for_each_entry(child, &ct->derived_connections, derived_list) {
		if (child->tuplehash[dir].tuple.dst.u.udp.port == port) {
			pr_debug("nf_nat_rtsp: found port %hu mapped from "
				 "%hu\n",
			       	 ntohs(child->tuplehash[dir].
			       	 tuple.dst.u.udp.port),
			       	 ntohs(child->tuplehash[!dir].
			       	 tuple.src.u.udp.port));
			return child->tuplehash[!dir].tuple.src.u.udp.port;
		}
	}

	return htons(0);
}

/****************************************************************************/
static int nat_rtsp_modify_port (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			      	 enum ip_conntrack_info ctinfo,
				 unsigned int matchoff, unsigned int matchlen,
			      	 __be16 rtpport, int *delta)
{
	__be16 orig_port;

	orig_port = lookup_mapping_port(ct, ctinfo, rtpport);
	if (orig_port == htons(0)) {
		*delta = 0;
		return 0;
	}
	if (modify_ports(skb, protoff, ct, ctinfo, matchoff, matchlen,
			 ntohs(orig_port), 0, 0, delta) < 0)
		return -1;
	pr_debug("nf_nat_rtsp: Modified client_port from %hu to %hu\n",
	       	 ntohs(rtpport), ntohs(orig_port));
	return 0;
}

/****************************************************************************/
static int nat_rtsp_modify_port2 (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			       	  enum ip_conntrack_info ctinfo,
				  unsigned int matchoff, unsigned int matchlen,
			       	  __be16 rtpport, __be16 rtcpport,
				  char dash, int *delta)
{
	__be16 orig_port;

	orig_port = lookup_mapping_port(ct, ctinfo, rtpport);
	if (orig_port == htons(0)) {
		*delta = 0;
		return 0;
	}
	if (modify_ports(skb, protoff, ct, ctinfo, matchoff, matchlen,
			 ntohs(orig_port), ntohs(orig_port)+1, dash, delta) < 0)
		return -1;
	pr_debug("nf_nat_rtsp: Modified client_port from %hu to %hu\n",
	       	 ntohs(rtpport), ntohs(orig_port));
	return 0;
}

/****************************************************************************/
static int nat_rtsp_modify_addr(struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
				enum ip_conntrack_info ctinfo,
				int matchoff, int matchlen, int *delta)
{
	char buf[sizeof("255.255.255.255")];
	int dir = CTINFO2DIR(ctinfo);
	int len;

	/* Change the destination address to FW's WAN IP address */

	len = sprintf(buf, "%pI4",
		       &ct->tuplehash[!dir].tuple.dst.u3.ip);
	if (!rtsp_mangle_tcp_packet(skb, ct, ctinfo, protoff, matchoff, matchlen,
				      buf, len)) {
		if (net_ratelimit())
			printk("nf_nat_rtsp: rtsp_mangle_tcp_packet error\n");
		return -1;
	}
	*delta = len - matchlen;
	return 0;
}

/****************************************************************************/
static int __init init(void)
{
	BUG_ON(rcu_dereference(nat_rtsp_channel_hook) != NULL);
	BUG_ON(rcu_dereference(nat_rtsp_channel2_hook) != NULL);
	BUG_ON(rcu_dereference(nat_rtsp_modify_port_hook) != NULL);
	BUG_ON(rcu_dereference(nat_rtsp_modify_port2_hook) != NULL);
	BUG_ON(rcu_dereference(nat_rtsp_modify_addr_hook) != NULL);
	rcu_assign_pointer(nat_rtsp_channel_hook, nat_rtsp_channel);
	rcu_assign_pointer(nat_rtsp_channel2_hook, nat_rtsp_channel2);
	rcu_assign_pointer(nat_rtsp_modify_port_hook, nat_rtsp_modify_port);
	rcu_assign_pointer(nat_rtsp_modify_port2_hook, nat_rtsp_modify_port2);
	rcu_assign_pointer(nat_rtsp_modify_addr_hook, nat_rtsp_modify_addr);

	pr_debug("nf_nat_rtsp: init success\n");
	return 0;
}

/****************************************************************************/
static void __exit fini(void)
{
	rcu_assign_pointer(nat_rtsp_channel_hook, NULL);
	rcu_assign_pointer(nat_rtsp_channel2_hook, NULL);
	rcu_assign_pointer(nat_rtsp_modify_port_hook, NULL);
	rcu_assign_pointer(nat_rtsp_modify_port2_hook, NULL);
	rcu_assign_pointer(nat_rtsp_modify_addr_hook, NULL);
	synchronize_rcu();
}

/****************************************************************************/
module_init(init);
module_exit(fini);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("RTSP NAT helper");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_nat_rtsp");

#endif // defined(CONFIG_BCM_KF_NETFILTER)
