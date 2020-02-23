#if defined(CONFIG_BCM_KF_PROTO_IPSEC)
/*
<:copyright-BRCM:2012:GPL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/ip.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/udp.h>
#include <linux/netfilter/nf_conntrack_ipsec.h>

#ifdef CONFIG_NF_BL_EXT
#include <linux/netfilter.h>
#endif /* CONFIG_NF_BL_EXT */

MODULE_AUTHOR("Pavan Kumar <pavank@broadcom.com>");
MODULE_DESCRIPTION("Netfilter connection tracking module for ipsec");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_conntrack_ipsec");

static DEFINE_SPINLOCK(nf_ipsec_lock);

int
(*nf_nat_ipsec_hook_outbound)(struct sk_buff *skb,
      struct nf_conn *ct, enum ip_conntrack_info ctinfo) __read_mostly;
EXPORT_SYMBOL_GPL(nf_nat_ipsec_hook_outbound);

int
(*nf_nat_ipsec_hook_inbound)(struct sk_buff *skb,
      struct nf_conn *ct, enum ip_conntrack_info ctinfo, __be32 lan_ip) __read_mostly;
EXPORT_SYMBOL_GPL(nf_nat_ipsec_hook_inbound);

static void __exit nf_conntrack_ipsec_fini(void);

#define CT_REFRESH_TIMEOUT (60 * HZ)	 /* KT: Changed from 13 Sec to 1 Min */

/* Internal table for ISAKMP */
struct _ipsec_table 
{
   u_int32_t initcookie;
   __be32 lan_ip;
   struct nf_conn *ct;
   int pkt_rcvd;
   int inuse;
} ipsec_table[MAX_VPN_CONNECTION];

static struct _ipsec_table *ipsec_alloc_entry(int *index)
{
   int idx = 0;

   for( ; idx < MAX_VPN_CONNECTION; idx++ ) 
   {
      if( ipsec_table[idx].inuse )
         continue;
   
      *index = idx;
      memset(&ipsec_table[idx], 0, sizeof(struct _ipsec_table));

      pr_debug("([%d] alloc_entry()\n", idx);

      return (&ipsec_table[idx]);
   }
   
   return NULL;
}

/*
 * Search an IPsec table entry by ct.
 */
struct _ipsec_table *search_ipsec_entry_by_ct(struct nf_conn *ct)
{
   int idx = 0;

   for( ; idx < MAX_VPN_CONNECTION; idx++)
   {
	  if (!ipsec_table[idx].inuse)
		 continue;

      pr_debug("Searching entry->ct(%p) <--> ct(%p)\n",
         ipsec_table[idx].ct, ct);

      /* check ct */
      if (ipsec_table[idx].ct == ct)
      {
         pr_debug("Found entry with ct(%p)\n", ct);

         return &ipsec_table[idx];
      }
   }
   pr_debug("No Entry for ct(%p)\n", ct);
   return NULL;
}

/*
 * Search an IPSEC table entry by the initiator cookie.
 */
struct _ipsec_table *
search_ipsec_entry_by_cookie(struct isakmp_pkt_hdr *isakmph)
{
   int idx = 0;
   struct _ipsec_table *ipsec_entry = ipsec_table;

   for( ; idx < MAX_VPN_CONNECTION; idx++ ) 
   {
	   pr_debug("Searching initcookie %x <-> %x\n",
          ntohl(isakmph->initcookie), ntohl(ipsec_entry->initcookie));
      
      if( isakmph->initcookie == ipsec_entry->initcookie ) 
         return ipsec_entry;
      
      ipsec_entry++;
   }
   
   return NULL;
}

/*
 * Search an IPSEC table entry by the source IP address.
 */
struct _ipsec_table *
search_ipsec_entry_by_addr(const __be32 lan_ip, int *index)
{
   int idx = 0;
   struct _ipsec_table *ipsec_entry = ipsec_table;

   for( ; idx < MAX_VPN_CONNECTION; idx++ ) 
   {
	   pr_debug("Looking up lan_ip=%pI4 table entry %pI4\n",
              &lan_ip, &ipsec_entry->lan_ip);

      if( ntohl(ipsec_entry->lan_ip) == ntohl(lan_ip) ) 
      {
    	  pr_debug("Search by addr returning entry %p\n", ipsec_entry);

         *index = idx;
         return ipsec_entry;
      }
      ipsec_entry++;
   }
   
   return NULL;
}

static inline int
ipsec_inbound_pkt(struct sk_buff *skb, struct nf_conn *ct,
		  enum ip_conntrack_info ctinfo, __be32 lan_ip)
{
//   struct nf_ct_ipsec_master *info = &nfct_help(ct)->help.ct_ipsec_info;
   typeof(nf_nat_ipsec_hook_inbound) nf_nat_ipsec_inbound;

   pr_debug("inbound ISAKMP packet for LAN %pI4\n", &lan_ip);

   nf_nat_ipsec_inbound = rcu_dereference(nf_nat_ipsec_hook_inbound);
   if (nf_nat_ipsec_inbound && ct->status & IPS_NAT_MASK)
      return nf_nat_ipsec_inbound(skb, ct, ctinfo, lan_ip);
   
   return NF_ACCEPT;
}

/*
 * For outgoing ISAKMP packets, we need to make sure UDP ports=500
 */
static inline int
ipsec_outbound_pkt(struct sk_buff *skb,
                   struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
   typeof(nf_nat_ipsec_hook_outbound) nf_nat_ipsec_outbound;

   pr_debug("outbound ISAKMP packet skb(%p)\n", skb);

   nf_nat_ipsec_outbound = rcu_dereference(nf_nat_ipsec_hook_outbound);
   if( nf_nat_ipsec_outbound && ct->status & IPS_NAT_MASK )
      return nf_nat_ipsec_outbound(skb, ct, ctinfo);
   
   return NF_ACCEPT;
}

/* track cookies inside ISAKMP, call expect_related */
static int conntrack_ipsec_help(struct sk_buff *skb, unsigned int protoff,
                             struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
   int dir = CTINFO2DIR(ctinfo);
   struct nf_ct_ipsec_master *info = nfct_help_data(ct);
   struct isakmp_pkt_hdr _isakmph, *isakmph = NULL;
   struct _ipsec_table *ipsec_entry = ipsec_table;
   int ret, index=0;

   pr_debug("skb(%p) skb->data(%p) ct(%p) protoff(%d) offset(%d)\n", skb, skb->data, ct, protoff, (int) (protoff + sizeof(struct udphdr)));

   isakmph = skb_header_pointer(skb, protoff + sizeof(struct udphdr), sizeof(_isakmph), &_isakmph);
   if (isakmph == NULL)
   {
      pr_debug("ERR: no full ISAKMP header, can't track. isakmph=[%p]\n", isakmph);
      return NF_ACCEPT;
   }

   if ( 0 == isakmph->initcookie )
   {
      pr_debug("ERR: all zero ISAKMP initcookie.\n");
      return NF_ACCEPT;
   }

   spin_lock_bh(&nf_ipsec_lock);

   if( dir == IP_CT_DIR_ORIGINAL )
   {
      int lan_ip = ct->tuplehash[dir].tuple.src.u3.ip;
      
      /* create one entry in the internal table if a new connection is found */
      if( (ipsec_entry = search_ipsec_entry_by_cookie(isakmph)) == NULL ) 
      {
         /* NOTE: cookies may be updated in the connection */
         if( (ipsec_entry = 
              search_ipsec_entry_by_addr(lan_ip, &index)) == NULL ) 
         {
            ipsec_entry = ipsec_alloc_entry(&index);
            if( ipsec_entry == NULL ) 
            {
               /* All entries are currently in use */
               pr_debug("ERR: Too many sessions. ct(%p)\n", ct);
               spin_unlock_bh(&nf_ipsec_lock);
               return NF_DROP;
            }
            
            ipsec_entry->ct = ct; /* KT: Guess it should be here */
            ipsec_entry->initcookie = isakmph->initcookie; /* KT: Update our cookie information - moved to here */
            ipsec_entry->lan_ip = ct->tuplehash[dir].tuple.src.u3.ip;
            ipsec_entry->inuse = 1;

            pr_debug("NEW ipsec_entry[%d] with ct=%p, lan_ip=%pI4, initcookie=%x\n",
				index, ipsec_entry->ct, &ipsec_entry->lan_ip,
				ntohl(ipsec_entry->initcookie) );
         } else {
             pr_debug("EXISTING ipsec_entry[%d] with ct=%p, lan_ip=%pI4, initcookie=%x\n",
 				index, ipsec_entry->ct, &ipsec_entry->lan_ip,
 				ntohl(ipsec_entry->initcookie) );
         }
      }
      ipsec_entry->pkt_rcvd++;

      info->initcookie = isakmph->initcookie;
      info->lan_ip = ct->tuplehash[dir].tuple.src.u3.ip;

      pr_debug("L->W: initcookie=%x, lan_ip=%pI4, dir[%d] src.u3.ip=%pI4, dst.u3.ip=%pI4\n",
              info->initcookie, &info->lan_ip,
              dir,
              &ct->tuplehash[dir].tuple.src.u3.ip,
              &ct->tuplehash[dir].tuple.dst.u3.ip);
      
      nf_ct_refresh_acct(ipsec_entry->ct, 0, skb, CT_REFRESH_TIMEOUT);

      ret = ipsec_outbound_pkt(skb, ct, ctinfo);
   }
   else
   {
	  pr_debug("WAN->LAN ct=%p\n", ct);
      
      if( (ipsec_entry = search_ipsec_entry_by_cookie(isakmph)) != NULL )
      {
    	 nf_ct_refresh_acct(ipsec_entry->ct, 0, skb, CT_REFRESH_TIMEOUT);
         ipsec_entry->pkt_rcvd++;

         pr_debug("W->L: initcookie=%x, lan_ip=%pI4, dir[%d] src.u3.ip=%pI4, dst.u3.ip=%pI4\n",
              info->initcookie, &info->lan_ip,
              dir,
              &ct->tuplehash[dir].tuple.src.u3.ip,
              &ct->tuplehash[dir].tuple.dst.u3.ip);

         ret = ipsec_inbound_pkt(skb, ct, ctinfo, ipsec_entry->lan_ip);
      }
      else
      {
    	 pr_debug("WARNNING: client from WAN tries to connect to VPN server in the LAN. ipsec_entry=[%p]\n", ipsec_entry);
         ret = NF_ACCEPT;
      }
   }

   spin_unlock_bh(&nf_ipsec_lock);

   return ret;
}

/* Called when the connection is deleted. */
static void ipsec_destroy(struct nf_conn *ct)
{
	struct _ipsec_table *ipsec_entry = NULL;

	spin_lock_bh(&nf_ipsec_lock);
	pr_debug("DEL IPsec entry ct(%p)\n", ct);
	if ((ipsec_entry = search_ipsec_entry_by_ct(ct))) {
		memset(ipsec_entry, 0, sizeof(struct _ipsec_table));
	} else {
		pr_debug("DEL IPsec entry failed: ct(%p)\n", ct);
	}
	spin_unlock_bh(&nf_ipsec_lock);
}

static const struct nf_conntrack_expect_policy ipsec_exp_policy = {
	.max_expected	= 3,
	.timeout		= 300,
};

/* ISAKMP protocol helper */
static struct nf_conntrack_helper ipsec __read_mostly = {
   .name = "ipsec",
   .me = THIS_MODULE,
   .tuple.src.l3num = AF_INET,
   .tuple.dst.protonum = IPPROTO_UDP,
   .tuple.src.u.udp.port = __constant_htons(IPSEC_PORT),
   .data_len = sizeof(struct nf_ct_ipsec_master),
   .help = conntrack_ipsec_help,
   .destroy = ipsec_destroy,
   .expect_policy		= &ipsec_exp_policy,
};

static int __init nf_conntrack_ipsec_init(void)
{
   return nf_conntrack_helper_register(&ipsec);
}

static void __exit nf_conntrack_ipsec_fini(void)
{
   nf_conntrack_helper_unregister(&ipsec);
}

module_init(nf_conntrack_ipsec_init);
module_exit(nf_conntrack_ipsec_fini);
#endif
