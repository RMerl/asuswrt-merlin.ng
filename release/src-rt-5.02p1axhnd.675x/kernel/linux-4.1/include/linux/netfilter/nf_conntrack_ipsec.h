#if defined(CONFIG_BCM_KF_PROTO_IPSEC)
/* IPSEC constants and structs */
#ifndef _NF_CONNTRACK_IPSEC_H
#define _NF_CONNTRACK_IPSEC_H

#include <linux/netfilter/nf_conntrack_common.h>

/* conntrack private data */
struct nf_ct_ipsec_master 
{
   __be32 initcookie;  /* initcookie of ISAKMP */
   __be32 lan_ip;        /* LAN IP */
};

struct nf_nat_ipsec 
{
   __be32 lan_ip;   /* LAN IP */
};

#ifdef __KERNEL__

#define IPSEC_PORT   500
#define MAX_VPN_CONNECTION 8  

struct isakmp_pkt_hdr 
{
   __be32 initcookie;
};


/* crap needed for nf_conntrack_compat.h */
struct nf_conn;
struct nf_conntrack_expect;

extern int
(*nf_nat_ipsec_hook_outbound)(struct sk_buff *skb,
                           struct nf_conn *ct, enum ip_conntrack_info ctinfo);

extern int
(*nf_nat_ipsec_hook_inbound)(struct sk_buff *skb, struct nf_conn *ct,
                             enum ip_conntrack_info ctinfo, __be32 lan_ip);

#endif /* __KERNEL__ */
#endif /* _NF_CONNTRACK_IPSEC_H */
#endif
