/*
<:copyright-BRCM:2021:GPL/GPL:standard

   Copyright (c) 2021 Broadcom 
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

#if IS_ENABLED(CONFIG_NF_DEFRAG_IPV6)

#include <net/ipv6_frag.h>
#include <net/ip6_route.h>
#include <linux/version.h>

void nf_ct_frag6_ident_reuse(struct frag_queue *fq, struct sk_buff *skb,
			     struct net_device *dev)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	struct net *net;
	struct rt6_info *rt;

	net = container_of(fq->q.net, struct net, nf_frag.frags);
	rt = rt6_lookup(net, &ipv6_hdr(skb)->daddr, NULL, 0,
#else
	struct rt6_info *rt;

	rt = rt6_lookup(fq->q.fqdir->net, &ipv6_hdr(skb)->daddr, NULL, 0,
#endif
			NULL, 0);

	if (rt && rt->dst.dev) {
		u32 sig = *((u32 *)netdev_priv(rt->dst.dev));

		/* Translator gets a chance to copy the low-order 16 bits
		 * in the identification field.
		 * RFC7915, Section 5.1.1 */
		if (sig == NAT46_DEVICE_SIGNATURE && dev != rt->dst.dev) {
			struct frag_hdr *fhdr;

			fhdr = (struct frag_hdr *)skb_transport_header(skb);
			skbuff_bcm_ext_map_get(skb, map_id) = fhdr->identification;
		}
	}
	ip6_rt_put(rt);
}
EXPORT_SYMBOL_GPL(nf_ct_frag6_ident_reuse);

#endif /* IS_ENABLED(CONFIG_NF_DEFRAG_IPV6) */
