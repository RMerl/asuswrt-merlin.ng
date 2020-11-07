#if defined(CONFIG_BCM_KF_PROTO_ESP)
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
/******************************************************************************
Filename:       nf_nat_proto_esp.c
Author:         Pavan Kumar
Creation Date:  05/27/04

Description:
    Implements the ESP ALG connectiontracking.
    Migrated to kernel 2.6.21.5 on April 16, 2008 by Dan-Han Tsai.
    Migrated to kernel 3.4.11 on Jan 21, 2013 by Kirill Tsym
*****************************************************************************/
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>

#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_l4proto.h>
#include <linux/netfilter/nf_conntrack_proto_esp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@gnumonks.org>");
MODULE_DESCRIPTION("Netfilter NAT protocol helper module for ESP");

/* is spi in given range between min and max */
static bool
esp_in_range(const struct nf_conntrack_tuple *tuple,
	     enum nf_nat_manip_type maniptype,
	     const union nf_conntrack_man_proto *min,
	     const union nf_conntrack_man_proto *max)
{
   return true;
}

/* generate unique tuple ... */
static void
esp_unique_tuple(const struct nf_nat_l3proto *l3proto,
		 struct nf_conntrack_tuple *tuple,
		 const struct nf_nat_range *range,
		 enum nf_nat_manip_type maniptype,
		 const struct nf_conn *ct)
{
   return;
}

/* manipulate a ESP packet according to maniptype */
static bool
esp_manip_pkt(struct sk_buff *skb,
	      const struct nf_nat_l3proto *l3proto,
	      unsigned int iphdroff, unsigned int hdroff,
	      const struct nf_conntrack_tuple *tuple,
	      enum nf_nat_manip_type maniptype)
{
   struct esphdr *esph;
   struct iphdr *iph = (struct iphdr *)(skb->data + iphdroff);
   __be32 oldip, newip;

   if (!skb_make_writable(skb, hdroff + sizeof(*esph)))
      return false;

   if (maniptype == NF_NAT_MANIP_SRC)
   {
      /* Get rid of src ip and src pt */
      oldip = iph->saddr;
      newip = tuple->src.u3.ip;
   } 
   else 
   {
      /* Get rid of dst ip and dst pt */
      oldip = iph->daddr;
      newip = tuple->dst.u3.ip;
   }

   return true;
}

const struct nf_nat_l4proto esp __read_mostly = {
   .l4proto = IPPROTO_ESP,
   .manip_pkt = esp_manip_pkt,
   .in_range = esp_in_range,
   .unique_tuple = esp_unique_tuple,
};

int __init nf_nat_proto_esp_init(void)
{
   return nf_nat_l4proto_register(NFPROTO_IPV4, &esp);
}

void __exit nf_nat_proto_esp_fini(void)
{
   nf_nat_l4proto_unregister(NFPROTO_IPV4, &esp);
}

module_init(nf_nat_proto_esp_init);
module_exit(nf_nat_proto_esp_fini);
#endif
