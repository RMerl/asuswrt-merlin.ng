/* RTSP helper for connection tracking. */

/* (C) 2008 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/ctype.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_seqadj.h>
#include <linux/netfilter/nf_conntrack_rtsp.h>
#if IS_ENABLED(CONFIG_BCM_INGQOS)
#include <linux/iqos.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("RTSP connection tracking helper");
MODULE_ALIAS("ip_conntrack_rtsp");

#define RTSP_PORT 554

/* This is slow, but it's simple. --RR */
static char *rtsp_buffer;

static DEFINE_SPINLOCK(nf_rtsp_lock);

#define MAX_PORTS 8
static u_int16_t ports[MAX_PORTS];
static unsigned int ports_c;
module_param_array(ports, ushort, &ports_c, 0400);
MODULE_PARM_DESC(ports, "port numbers of RTSP servers");

#define RTSP_CHANNEL_MAX 8
static int max_outstanding = RTSP_CHANNEL_MAX;
module_param(max_outstanding, int, 0600);
MODULE_PARM_DESC(max_outstanding,
		 "max number of outstanding SETUP requests per RTSP session");

/* Single data channel */
int (*nat_rtsp_channel_hook) (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			      enum ip_conntrack_info ctinfo,
			      unsigned int matchoff, unsigned int matchlen,
			      struct nf_conntrack_expect *exp, int *delta);
EXPORT_SYMBOL_GPL(nat_rtsp_channel_hook);

/* A pair of data channels (RTP/RTCP) */
int (*nat_rtsp_channel2_hook) (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			       enum ip_conntrack_info ctinfo,
			       unsigned int matchoff, unsigned int matchlen,
			       struct nf_conntrack_expect *rtp_exp,
			       struct nf_conntrack_expect *rtcp_exp,
			       char dash, int *delta);
EXPORT_SYMBOL_GPL(nat_rtsp_channel2_hook);

/* Modify parameters like client_port in Transport for single data channel */
int (*nat_rtsp_modify_port_hook) (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			      	  enum ip_conntrack_info ctinfo,
			      	  unsigned int matchoff, unsigned int matchlen,
			      	  __be16 rtpport, int *delta);
EXPORT_SYMBOL_GPL(nat_rtsp_modify_port_hook);

/* Modify parameters like client_port in Transport for multiple data channels*/
int (*nat_rtsp_modify_port2_hook) (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			       	   enum ip_conntrack_info ctinfo,
			       	   unsigned int matchoff, unsigned int matchlen,
			       	   __be16 rtpport, __be16 rtcpport,
				   char dash, int *delta);
EXPORT_SYMBOL_GPL(nat_rtsp_modify_port2_hook);

/* Modify parameters like destination in Transport */
int (*nat_rtsp_modify_addr_hook) (struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			 	  enum ip_conntrack_info ctinfo,
			 	  int matchoff, int matchlen, int *delta);
EXPORT_SYMBOL_GPL(nat_rtsp_modify_addr_hook);

static int memmem(const char *haystack, int haystacklen,
		  const char *needle, int needlelen)
{
	const char *p = haystack;
	int l = haystacklen - needlelen + 1;
	char c = *needle;

	while(l-- > 0) { /* "!=0" won't handle haystacklen less than needlelen, need ">" */
		if (*p++ == c) {
			if (memcmp(p, needle+1, needlelen-1) == 0)
				return p - haystack - 1;
		}
	}
	return -1;
}

static int memstr(const char *haystack, int haystacklen,
		  const char *needle, int needlelen)
{
	const char *p = haystack;
	int l = haystacklen - needlelen + 1;
	char c = *needle;

	if (isalpha(c)) {
		char lower = __tolower(c);
		char upper = __toupper(c);

		while(l-- > 0) {  /* "!=0" won't handle haystacklen less than needlelen, need ">" */
			if (*p == lower || *p == upper) {
				if (strncasecmp(p, needle, needlelen) == 0)
					return p - haystack;
			}
			p++;
		}
	} else {
		while(l-- > 0) {
			if (*p++ == c) {
				if (strncasecmp(p, needle+1, needlelen-1) == 0)
					return p - haystack - 1;
			}
		}
	}
	return -1;
}

static int get_cseq(const char *str)
{
	unsigned long cseq = 0, i = 0;
	char c = *str;
	while(i++ < 10 && c && c != 0xd && c>='0' && c <= '9'){
		cseq = (cseq * 10) + (c - '0');
		c = *(str + i);
	}
	if(!cseq)
		cseq = -1;
	return (int) cseq;
}

/* Get next message in a packet */
static int get_next_message(const char *tcpdata, int tcpdatalen,
			    int *msgoff, int *msglen, int *msghdrlen)
{
	if (*msglen == 0) { /* The first message */
		*msgoff = 0;
	} else {
		*msgoff += *msglen;
		if ((*msgoff + 4) >= tcpdatalen) /* No more message */
			return 0;
	}

	/* Get message header length */
	*msghdrlen = memmem(tcpdata+*msgoff, tcpdatalen-*msgoff, "\r\n\r\n", 4);
	if (*msghdrlen < 0) {
		*msghdrlen = *msglen = tcpdatalen - *msgoff;
	} else {
		/* Get message length including SDP */
		int cloff = memstr(tcpdata+*msgoff, *msghdrlen, "Content-Length: ", 16);
		if (cloff < 0) {
			*msglen = *msghdrlen + 4;
		} else {
			unsigned long cl = simple_strtoul(tcpdata+*msgoff+cloff+16, NULL, 10);
			*msglen = *msghdrlen + 4 + cl;
		}
	}

	return 1;
}

/* Get next client_port parameter in a Transport header */
static int get_next_client_port(const char *tcpdata, int tpoff, int tplen,
				int *portoff, int *portlen,
				__be16 *rtpport, __be16 *rtcpport,
				char *dash)
{
	int off;
	char *p;

	if (*portlen == 0) { /* The first client_port */
		*portoff = tpoff;
	} else {
		*portoff += *portlen;
		if (*portoff >= tpoff + tplen) /* No more data */
			return 0;
	}

	off = memmem(tcpdata+*portoff, tplen-(*portoff-tpoff),
		     ";client_port=", 13);
	if (off < 0)
		return 0;
	*portoff += off + 13;

	*rtpport = htons((unsigned short)simple_strtoul(tcpdata+*portoff,
							&p, 10));
	if (*p != '-' && *p != '/') {
		*dash = 0;
	} else {
		*dash = *p++;
		*rtcpport = htons((unsigned short)simple_strtoul(p, &p, 10));
	}
	*portlen = p - tcpdata - *portoff;
	return 1;
}

/* Get next destination=<ip>:<port> parameter in a Transport header
 * This is not a standard parameter, so far, it's only seen in some customers'
 * products.
 */
static int get_next_dest_ipport(const char *tcpdata, int tpoff, int tplen,
				int *destoff, int *destlen, union nf_inet_addr *dest,
				int *portoff, int *portlen, __be16 *port, u_int16_t l3num)
{
	int off;
	char *p;
	int ret;
	
	if (*destlen == 0) { /* The first destination */
		*destoff = tpoff;
	} else {
		*destoff += *destlen + 1 + *portlen;
		if (*destoff >= tpoff + tplen) /* No more data */
			return 0;
	}

	off = memmem(tcpdata+*destoff, tplen-(*destoff-tpoff),
		     ";destination=", 13);
	if (off < 0)
		return 0;
	*destoff += off + 13;

	if (l3num == AF_INET6) {
		ret = in6_pton(tcpdata+*destoff, tplen-(*destoff-tpoff), (u8 *)&dest->ip6,
	                     -1, (const char **)&p);
	}
	else {
		ret = in4_pton(tcpdata+*destoff, tplen-(*destoff-tpoff), (u8 *)&dest->ip,
	                     -1, (const char **)&p);
	}
	
	if (!ret) {
		return 0;
	}
	*destlen = p - tcpdata - *destoff;

	if (*p != ':') {
		return 0;
	}
	*portoff = p - tcpdata + 1;

	*port = htons((unsigned short)simple_strtoul(tcpdata+*portoff, &p, 10));
	*portlen = p - tcpdata - *portoff;

	return 1;
}

/* Get next destination parameter in a Transport header */
static int get_next_destination(const char *tcpdata, int tpoff, int tplen,
				int *destoff, int *destlen, union nf_inet_addr *dest, u_int16_t l3num)
{
	int off;
	char *p;
	int ret;
	
	if (*destlen == 0) { /* The first destination */
		*destoff = tpoff;
	} else {
		*destoff += *destlen;
		if (*destoff >= tpoff + tplen) /* No more data */
			return 0;
	}

	off = memmem(tcpdata+*destoff, tplen-(*destoff-tpoff),
		     ";destination=", 13);
	if (off < 0)
		return 0;
	*destoff += off + 13;

	if (l3num == AF_INET6) {
		ret = in6_pton(tcpdata+*destoff, tplen-(*destoff-tpoff), (u8 *)&dest->ip6,
	                     -1, (const char **)&p);
	}
	else {
		ret = in4_pton(tcpdata+*destoff, tplen-(*destoff-tpoff), (u8 *)&dest->ip,
	                     -1, (const char **)&p);
	}
						 
	if (ret) {
		*destlen = p - tcpdata - *destoff;
		return 1;
	} 
	else {
		return 0;
	}
}

static int expect_rtsp_channel(struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
			       enum ip_conntrack_info ctinfo,
			       int portoff, int portlen,
			       __be16 rtpport, int *delta)
{
	int ret = 0;
	int dir = CTINFO2DIR(ctinfo);
	struct nf_conntrack_expect *rtp_exp;
	typeof(nat_rtsp_channel_hook) nat_rtsp_channel;

	if (rtpport == 0)
		return -1;

	/* Create expect for RTP */
	if ((rtp_exp = nf_ct_expect_alloc(ct)) == NULL)
		return -1;

	nf_ct_expect_init(rtp_exp, NF_CT_EXPECT_CLASS_DEFAULT, nf_ct_l3num(ct),
			  NULL, &ct->tuplehash[!dir].tuple.dst.u3,
			  IPPROTO_UDP, NULL, &rtpport);

	if ((nat_rtsp_channel = rcu_dereference(nat_rtsp_channel_hook)) &&
	    ct->status & IPS_NAT_MASK) {
		/* NAT needed */
		ret = nat_rtsp_channel(skb, protoff, ct, ctinfo, portoff, portlen,
				       rtp_exp, delta);
	} else {		/* Conntrack only */
		if (NF_CT_EXPECT_RELATED(rtp_exp) == 0) {
			pr_debug("nf_ct_rtsp: expect RTP ");
			nf_ct_dump_tuple(&rtp_exp->tuple);
		} else
			ret = -1;
	}

	nf_ct_expect_put(rtp_exp);

	return ret;
}

static int expect_rtsp_channel2(struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct,
				enum ip_conntrack_info ctinfo,
				int portoff, int portlen,
				__be16 rtpport, __be16 rtcpport,
				char dash, int *delta)
{
	int ret = 0;
	int dir = CTINFO2DIR(ctinfo);
	struct nf_conntrack_expect *rtp_exp;
	struct nf_conntrack_expect *rtcp_exp;
	typeof(nat_rtsp_channel2_hook) nat_rtsp_channel2;

	if (rtpport == 0 || rtcpport == 0)
		return -1;

	/* Create expect for RTP */
	if ((rtp_exp = nf_ct_expect_alloc(ct)) == NULL)
		return -1;
	nf_ct_expect_init(rtp_exp, NF_CT_EXPECT_CLASS_DEFAULT ,nf_ct_l3num(ct),
			  NULL, &ct->tuplehash[!dir].tuple.dst.u3,
			  IPPROTO_UDP, NULL, &rtpport);

	/* Create expect for RTCP */
	if ((rtcp_exp = nf_ct_expect_alloc(ct)) == NULL) {
		nf_ct_expect_put(rtp_exp);
		return -1;
	}
	nf_ct_expect_init(rtcp_exp, NF_CT_EXPECT_CLASS_DEFAULT, nf_ct_l3num(ct),
			  NULL, &ct->tuplehash[!dir].tuple.dst.u3,
			  IPPROTO_UDP, NULL, &rtcpport);

	if ((nat_rtsp_channel2 = rcu_dereference(nat_rtsp_channel2_hook)) &&
	    ct->status & IPS_NAT_MASK) {
		/* NAT needed */
		ret = nat_rtsp_channel2(skb, protoff, ct, ctinfo, portoff, portlen,
				   	rtp_exp, rtcp_exp, dash, delta);
	} else {		/* Conntrack only */
		if (NF_CT_EXPECT_RELATED(rtp_exp) == 0) {
			if (NF_CT_EXPECT_RELATED(rtcp_exp) == 0) {
				pr_debug("nf_ct_rtsp: expect RTP ");
				nf_ct_dump_tuple(&rtp_exp->tuple);
				pr_debug("nf_ct_rtsp: expect RTCP ");
				nf_ct_dump_tuple(&rtcp_exp->tuple);
			} else {
				nf_ct_unexpect_related(rtp_exp);
				ret = -1;
			}
		} else
			ret = -1;
	}

	nf_ct_expect_put(rtp_exp);
	nf_ct_expect_put(rtcp_exp);

	return ret;
}

#if defined(CONFIG_BCM_NF_DERIVED_CONN)
static void set_normal_timeout(struct nf_conn *ct, struct sk_buff *skb)
{
	struct nf_conn *child;

	/* make sure to preserve the lock order. derived_lock-->blog_lock */
 	BCM_DERIVED_CONN_LOCK_BH();
	list_for_each_entry(child, &ct->bcm_ext.derived_connections, bcm_ext.derived_list) {
#if defined(CONFIG_BLOG)
		blog_lock();
		if ((child->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID)
			|| (child->bcm_ext.blog_key[IP_CT_DIR_REPLY] != BLOG_KEY_FC_INVALID)) {
			/* remove flow from flow cache */
			blog_notify(DESTROY_FLOWTRACK, (void*)child,
					(uint32_t)child->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
								(uint32_t)child->bcm_ext.blog_key[IP_CT_DIR_REPLY]);

			clear_bit(IPS_BLOG_BIT, &ct->status);
		}
		blog_unlock();
#endif
		child->bcm_ext.derived_timeout = 5*HZ;
		nf_ct_refresh(child, skb, 5*HZ);

		set_bit(IPS_FIXED_TIMEOUT_BIT, &child->status);  /* dont refresh */
	}
 	BCM_DERIVED_CONN_UNLOCK_BH();
}

static void set_long_timeout(struct nf_conn *ct, struct sk_buff *skb)
{
	struct nf_conn *child;

 	BCM_DERIVED_CONN_LOCK_BH();
	list_for_each_entry(child, &ct->bcm_ext.derived_connections, bcm_ext.derived_list) {

		/*here no need to evict from blog, as we are using 3600 secs */
		nf_ct_refresh(child, skb, 3600*HZ);
	}
 	BCM_DERIVED_CONN_UNLOCK_BH();
}
#endif


static int help(struct sk_buff *skb, unsigned int protoff,
		struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
#if defined(CONFIG_BLOG)
	struct nf_conn_help *hlp = nfct_help(ct);
#endif
	struct nf_ct_rtsp_master *info = nfct_help_data(ct);
	struct tcphdr _tcph, *th;
	unsigned int tcpdataoff, tcpdatalen;
	char *tcpdata;
	int msgoff, msglen, msghdrlen;
	int tpoff, tplen;
	int portlen = 0;
	int portoff = 0;
	__be16 rtpport = 0;
	__be16 rtcpport = 0;
	char dash = 0;
	int destlen = 0;
	int destoff = 0;
	int totaldelta = 0;
	union nf_inet_addr dest;
	typeof(nat_rtsp_modify_addr_hook) nat_rtsp_modify_addr;
	typeof(nat_rtsp_modify_port_hook) nat_rtsp_modify_port;
	typeof(nat_rtsp_modify_port2_hook) nat_rtsp_modify_port2;

	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED
	    && ctinfo != IP_CT_ESTABLISHED+IP_CT_IS_REPLY) {
		return NF_ACCEPT;
	}
	pr_debug("nf_ct_rtsp: skblen = %u\n", skb->len);

	/* Get TCP header */
	th = skb_header_pointer(skb, protoff, sizeof(_tcph), &_tcph);
	if (th == NULL) {
		return NF_ACCEPT;
	}

	/* Get TCP payload offset */
	tcpdataoff = protoff + th->doff * 4;
	if (tcpdataoff >= skb->len) { /* No data? */
		return NF_ACCEPT;
	}

	/* Get TCP payload length */
	tcpdatalen = skb->len - tcpdataoff;

	spin_lock_bh(&nf_rtsp_lock);

	/* Get TCP payload pointer */
	tcpdata = skb_header_pointer(skb, tcpdataoff, tcpdatalen, rtsp_buffer);
	BUG_ON(tcpdata == NULL);

	/* There may be more than one message in a packet, check them
	 * one by one */
	msgoff = msglen = msghdrlen = 0;
	while(get_next_message(tcpdata, tcpdatalen, &msgoff, &msglen,
			       &msghdrlen)) {
		/* Messages from LAN side through MASQUERADED connections */
		if (memcmp(&ct->tuplehash[dir].tuple.src.u3,
			   &ct->tuplehash[!dir].tuple.dst.u3,
			   sizeof(ct->tuplehash[dir].tuple.src.u3)) != 0) {
			if(memcmp(tcpdata+msgoff, "PAUSE ", 6) == 0) {
				int cseq = memmem(tcpdata+msgoff, msglen, "CSeq: ", 6);
				if(cseq == -1) {
				        /* Fix the IOP issue with DSS on Drawin system */
				        cseq = memmem(tcpdata+msgoff, msglen, "Cseq: ", 6);
				        if(cseq == -1) {
					   pr_debug("nf_ct_rtsp: wrong PAUSE msg\n");
				        } else {
					   cseq = get_cseq(tcpdata+msgoff+cseq+6);
				        }
				} else {
					cseq = get_cseq(tcpdata+msgoff+cseq+6);
				}
				
				pr_debug("nf_ct_rtsp: PAUSE, CSeq=%d\n", cseq);
				info->paused = cseq;
				continue;
			} else {
				info->paused = 0;
			}
			if(memcmp(tcpdata+msgoff, "TEARDOWN ", 9) == 0) {
				pr_debug("nf_ct_rtsp: TEARDOWN\n");
#if defined(CONFIG_BCM_NF_DERIVED_CONN)
				set_normal_timeout(ct, skb);
#endif
				continue;
			} else if(memcmp(tcpdata+msgoff, "SETUP ", 6) != 0) {
				continue;
			}
			
			/* Now begin to process SETUP message */
			pr_debug("nf_ct_rtsp: SETUP\n");
		/* Reply message that's from WAN side. */
		} else {
			/* We only check replies */
			if(memcmp(tcpdata+msgoff, "RTSP/", 5) != 0)
				continue;
			
			pr_debug("nf_ct_rtsp: Reply message\n");

		 	/* Response to a previous PAUSE message */
			if (info->paused) {
				int cseq = memmem(tcpdata+msgoff, msglen, "CSeq: ", 6);
				if(cseq == -1) {
				        /* Fix the IOP issue with DSS on Drawin system */
				        cseq = memmem(tcpdata+msgoff, msglen, "Cseq: ", 6);
				        if(cseq == -1) {
					   pr_debug("nf_ct_rtsp: wrong reply msg\n");
				        } else {
					   cseq = get_cseq(tcpdata+msgoff+cseq+6);
				        }
				} else {
					cseq = get_cseq(tcpdata+msgoff+cseq+6);
				}
				if(cseq == info->paused) {
				pr_debug("nf_ct_rtsp: Reply to PAUSE\n");
#if defined(CONFIG_BCM_NF_DERIVED_CONN)
				set_long_timeout(ct, skb);
#endif
				info->paused = 0;
				goto end;
			}
			
			}
			
			/* Now begin to process other reply message */
		}

		/* Get Transport header offset */
		tpoff = memmem(tcpdata+msgoff+6, msghdrlen-6,
			       "\r\nTransport: ", 13);
		if (tpoff < 0)
			continue;
		tpoff += msgoff + 6 + 13;

		/* Get Transport header length */
		tplen = memmem(tcpdata+tpoff, msghdrlen-(tpoff - msgoff),
			       "\r\n", 2);
		if (tplen < 0)
			tplen = msghdrlen - (tpoff - msgoff);

#if defined(CONFIG_BLOG)
        if (!test_bit(IPS_SEQ_ADJUST_BIT, &ct->status)) {
            /* Check for a server reply containing interleaved transport */
            if (dir && memmem(tcpdata+tpoff, msghdrlen-(tpoff - msgoff),
                    "interleaved=", 12) >= 0) {
                    pr_debug("Found RTSP interleaved transport: make it accelerated\n");
                    RCU_INIT_POINTER(hlp->helper, NULL);
                    set_bit(IPS_BLOG_BIT, &ct->status);
                    goto end;
            }
        }
#endif

		/* There maybe more than one client_port parameter in this
		 * field, we'll process each of them. I know not all of them
		 * are unicast UDP ports, but that is the only situation we
		 * care about so far. So just KISS. */
		portoff = portlen = 0;
		while(get_next_client_port(tcpdata, tpoff, tplen,
					   &portoff, &portlen,
					   &rtpport, &rtcpport, &dash)) {
			int ret=0, delta=0;

			if (memcmp(&ct->tuplehash[dir].tuple.src.u3,
			   	   &ct->tuplehash[!dir].tuple.dst.u3,
			   	   sizeof(ct->tuplehash[dir].tuple.src.u3))
			    != 0) {
				/* LAN to WAN */
				if (dash == 0) {
					/* Single data channel */
					ret = expect_rtsp_channel(skb, protoff, ct,
								  ctinfo,
			 					  portoff,
								  portlen,
								  rtpport,
								  &delta);
				} else {
					/* A pair of data channels (RTP/RTCP)*/
					ret = expect_rtsp_channel2(skb, protoff, ct,
								   ctinfo,
								   portoff,
								   portlen,
								   rtpport,
								   rtcpport,
								   dash,
								   &delta);
				}
			} else {
				nat_rtsp_modify_port = rcu_dereference(
					nat_rtsp_modify_port_hook);
				nat_rtsp_modify_port2 = rcu_dereference(
					nat_rtsp_modify_port2_hook);
				/* WAN to LAN */
				if (dash == 0 ) {
					/* Single data channel */
					if (nat_rtsp_modify_port) {
					ret = nat_rtsp_modify_port(skb, protoff, ct,
								   ctinfo,
								   portoff,
								   portlen,
								   rtpport,
								   &delta);
					}
				} else {
					/* A pair of data channels (RTP/RTCP)*/
					if (nat_rtsp_modify_port2) {
					ret = nat_rtsp_modify_port2(skb, protoff, ct,
								    ctinfo,
								    portoff,
								    portlen,
								    rtpport,
								    rtcpport,
								    dash,
								    &delta);
					}
				}
			}

            /* register the RTP ports with ingress QoS classifier */
            pr_debug("\n RTP Port = %d, RTCP Port = %d\n", rtpport, rtcpport);
#if IS_ENABLED(CONFIG_BCM_INGQOS)
            iqos_add_L4port(IPPROTO_UDP, rtpport, IQOS_ENT_DYN, IQOS_PRIO_HIGH);
            iqos_add_L4port(IPPROTO_UDP, rtcpport, IQOS_ENT_DYN, IQOS_PRIO_HIGH);
#endif

			if (ret < 0)
				goto end;

			if (delta) {
				/* Packet length has changed, we need to adjust
				 * everthing */
				tcpdatalen += delta;
				msglen += delta;
				msghdrlen += delta;
				tplen += delta;
				portlen += delta;

				/* Relocate TCP payload pointer */
				tcpdata = skb_header_pointer(skb,
							     tcpdataoff,
							     tcpdatalen,
							     rtsp_buffer);
				BUG_ON(tcpdata == NULL);
                totaldelta += delta; 
			}
		}

		/* Process special destination=<ip>:<port> parameter in 
		 * Transport header. This is not a standard parameter,
		 * so far, it's only seen in some customers' products.
 		 */
		while(get_next_dest_ipport(tcpdata, tpoff, tplen,
					   &destoff, &destlen, &dest,
					   &portoff, &portlen, &rtpport, nf_ct_l3num(ct))) {
			int ret = 0, delta = 0;

			/* Process the port part */
			if (memcmp(&ct->tuplehash[dir].tuple.src.u3,
			   	   &ct->tuplehash[!dir].tuple.dst.u3,
			   	   sizeof(ct->tuplehash[dir].tuple.src.u3))
			    != 0) {
				/* LAN to WAN */
				ret = expect_rtsp_channel(skb, protoff, ct, ctinfo,
							  portoff, portlen,
							  rtpport, &delta);
			} else {
				/* WAN to LAN */
				if ((nat_rtsp_modify_port = rcu_dereference(
				    nat_rtsp_modify_port_hook))) {
					ret = nat_rtsp_modify_port(skb, protoff, ct,
								   ctinfo,
								   portoff,
								   portlen,
								   rtpport,
								   &delta);
				}
			}
            
            /* register the RTP ports with ingress QoS classifier */
            pr_debug("\n RTP Port = %d\n", rtpport);
#if IS_ENABLED(CONFIG_BCM_INGQOS)
            iqos_add_L4port(IPPROTO_UDP, rtpport, IQOS_ENT_DYN, IQOS_PRIO_HIGH);
#endif

			if (ret < 0)
				goto end;

			if (delta) {
				/* Packet length has changed, we need to adjust
				 * everthing */
				tcpdatalen += delta;
				msglen += delta;
				msghdrlen += delta;
				tplen += delta;
				portlen += delta;

				/* Relocate TCP payload pointer */
				tcpdata = skb_header_pointer(skb,
							     tcpdataoff,
							     tcpdatalen,
							     rtsp_buffer);
				BUG_ON(tcpdata == NULL);
                totaldelta += delta;
			}

			/* Then the IP part */
			if (nf_ct_l3num(ct) == AF_INET6) {
			    if (memcmp(dest.ip6, ct->tuplehash[dir].tuple.src.u3.ip6,sizeof(dest.ip6)))
					continue;
			}
			else {		
				if (dest.ip != ct->tuplehash[dir].tuple.src.u3.ip)
					continue;
			}

			if ((nat_rtsp_modify_addr =
			     rcu_dereference(nat_rtsp_modify_addr_hook)) &&
			    ct->status & IPS_NAT_MASK) {
			}
			/* NAT needed */
			ret = nat_rtsp_modify_addr(skb, protoff, ct, ctinfo,
						   destoff, destlen, &delta);
			if (ret < 0)
				goto end;

			if (delta) {
				/* Packet length has changed, we need
				 * to adjust everthing */
				tcpdatalen += delta;
				msglen += delta;
				msghdrlen += delta;
				tplen += delta;
				portlen += delta;

				/* Relocate TCP payload pointer */
				tcpdata = skb_header_pointer(skb, tcpdataoff,
							     tcpdatalen,
							     rtsp_buffer);
				BUG_ON(tcpdata == NULL);
                totaldelta += delta;
			}
		}

		if ((nat_rtsp_modify_addr =
		     rcu_dereference(nat_rtsp_modify_addr_hook)) &&
		    ct->status & IPS_NAT_MASK) {
			destoff = destlen = 0;
			while(get_next_destination(tcpdata, tpoff, tplen,
					   	   &destoff, &destlen, &dest, nf_ct_l3num(ct))) {
				int ret, delta;
				
				if (nf_ct_l3num(ct) == AF_INET6) {
					if (memcmp(dest.ip6, ct->tuplehash[dir].tuple.src.u3.ip6,sizeof(dest.ip6)))
						continue;
				}
				else {
					if (dest.ip != ct->tuplehash[dir].tuple.src.u3.ip)
						continue;
				}
				/* NAT needed */
				ret = nat_rtsp_modify_addr(skb, protoff, ct, ctinfo,
							   destoff, destlen,
				   			   &delta);
				if (ret < 0)
					goto end;

				if (delta) {
					/* Packet length has changed, we need
					 * to adjust everthing */
					tcpdatalen += delta;
					msglen += delta;
					msghdrlen += delta;
					tplen += delta;
					portlen += delta;

					/* Relocate TCP payload pointer */
					tcpdata = skb_header_pointer(skb,
							     tcpdataoff,
							     tcpdatalen,
							     rtsp_buffer);
					BUG_ON(tcpdata == NULL);
                    totaldelta += delta;
				}

			}
		}
	}
	nf_ct_seqadj_set(ct, ctinfo, th->seq, totaldelta);
	
end:
	spin_unlock_bh(&nf_rtsp_lock);
	return NF_ACCEPT;
}

static struct nf_conntrack_helper rtsp[MAX_PORTS][2];
static char rtsp_names[MAX_PORTS][sizeof("rtsp-65535")];
static struct nf_conntrack_expect_policy rtsp_exp_policy;

/* don't make this __exit, since it's called from __init ! */
static void nf_conntrack_rtsp_fini(void)
{
	int i,j = -1;

	for (i = 0; i < ports_c; i++) {
		for (j = 0; j < 2; j++) {
			if (rtsp[i][j].me == NULL)
				continue;

		    pr_debug("nf_ct_rtsp: unregistering helper for port %d\n",
		       	 ports[i]);
			nf_conntrack_helper_unregister(&rtsp[i][j]);
		}
		
#if IS_ENABLED(CONFIG_BCM_INGQOS)
        /* unregister the RTSP ports with ingress QoS classifier */
		iqos_rem_L4port( IPPROTO_TCP, ports[i], IQOS_ENT_STAT );
#endif
	}

	kfree(rtsp_buffer);
}

static int __init nf_conntrack_rtsp_init(void)
{
	int i, j = -1, ret = 0;
	char *tmpname;

	rtsp_buffer = kmalloc(4000, GFP_KERNEL);
	if (!rtsp_buffer)
		return -ENOMEM;

	if (ports_c == 0)
		ports[ports_c++] = RTSP_PORT;

	rtsp_exp_policy.max_expected = max_outstanding;
	rtsp_exp_policy.timeout = 5 * 60;
	for (i = 0; i < ports_c; i++) {
		rtsp[i][0].tuple.src.l3num = PF_INET;
		rtsp[i][1].tuple.src.l3num = PF_INET6;
		for (j = 0; j < 2; j++) {
			rtsp[i][j].tuple.src.u.tcp.port = htons(ports[i]);
			rtsp[i][j].tuple.dst.protonum = IPPROTO_TCP;
			rtsp[i][j].data_len = sizeof(struct nf_ct_rtsp_master);
			rtsp[i][j].expect_policy = &rtsp_exp_policy;
			rtsp[i][j].expect_class_max = 1;
			rtsp[i][j].me = THIS_MODULE;
			rtsp[i][j].help = help;
			tmpname = &rtsp_names[i][0];
			if (ports[i] == RTSP_PORT)
				sprintf(tmpname, "rtsp");
			else
				sprintf(tmpname, "rtsp-%d", ports[i]);
			strncpy(rtsp[i][j].name, tmpname, NF_CT_HELPER_NAME_LEN);
			rtsp[i][j].name[NF_CT_HELPER_NAME_LEN-1]='\0'; 

			pr_debug("nf_ct_rtsp: registering helper for port %d\n",
			       	 ports[i]);
			ret = nf_conntrack_helper_register(&rtsp[i][j]);
			if (ret) {
				printk("nf_ct_rtsp: failed to register helper "
				       "for port %d\n", ports[i]);
				nf_conntrack_rtsp_fini();
				return ret;
			}
		}
#if IS_ENABLED(CONFIG_BCM_INGQOS)
    /* register the RTSP ports with ingress QoS classifier */
    iqos_add_L4port( IPPROTO_TCP, ports[i], IQOS_ENT_STAT, IQOS_PRIO_HIGH );
#endif
	}

	return 0;
}

module_init(nf_conntrack_rtsp_init);
module_exit(nf_conntrack_rtsp_fini);
