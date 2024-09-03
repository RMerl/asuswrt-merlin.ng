/*
 * <:copyright-BRCM:2020:DUAL/GPL:standard
 * 
 *    Copyright (c) 2020 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <net/ax25.h>
#include <linux/ndi.h>
#include "ndi_local.h"

void ndi_parse_arp(struct ndi_classify *p)
{
	struct net_device *dev = p->skb->dev;
	struct in_device *in_dev = __in_dev_get_rcu(dev);
	struct arphdr *arp;
	unsigned char *arp_ptr;
	unsigned char *sha;
	struct in_addr *addr;
	u16 dev_type = dev->type;

	if (!in_dev)
		return;

	arp = arp_hdr(p->skb);

	switch (dev_type) {
	default:
		if (arp->ar_pro != htons(ETH_P_IP) ||
		    htons(dev_type) != arp->ar_hrd)
			return;
		break;
	case ARPHRD_ETHER:
	case ARPHRD_FDDI:
	case ARPHRD_IEEE802:
		/*
		 * ETHERNET, and Fibre Channel (which are IEEE 802
		 * devices, according to RFC 2625) devices will accept ARP
		 * hardware types of either 1 (Ethernet) or 6 (IEEE 802.2).
		 * This is the case also of FDDI, where the RFC 1390 says that
		 * FDDI devices should accept ARP hardware of (1) Ethernet,
		 * however, to be more robust, we'll accept both 1 (Ethernet)
		 * or 6 (IEEE 802.2)
		 */
		if ((arp->ar_hrd != htons(ARPHRD_ETHER) &&
		     arp->ar_hrd != htons(ARPHRD_IEEE802)) ||
		    arp->ar_pro != htons(ETH_P_IP))
			return;
		break;
	case ARPHRD_AX25:
		if (arp->ar_pro != htons(AX25_P_IP) ||
		    arp->ar_hrd != htons(ARPHRD_AX25))
			return;
		break;
	case ARPHRD_NETROM:
		if (arp->ar_pro != htons(AX25_P_IP) ||
		    arp->ar_hrd != htons(ARPHRD_NETROM))
			return;
		break;
	}

	/* Only process ARP replies, as only ARP replies are originated
	   from real devices with complete information */
	if (arp->ar_op != htons(ARPOP_REPLY))
		return;

	/*
	 *	Extract fields
	 */
	arp_ptr = (unsigned char *)(arp + 1);
	sha	= arp_ptr;
	addr = (struct in_addr *)(arp_ptr + dev->addr_len);

	p->dev = dev_find_or_new(sha, p->skb);
	if (!p->dev)
		return;
	if (!ipv4_addr_equal(addr, &p->dev->ip4))
		p->dev->ip4 = *addr;

	pr_debug("%s got arp response\n", ndi_dev_name(p->dev));
}
