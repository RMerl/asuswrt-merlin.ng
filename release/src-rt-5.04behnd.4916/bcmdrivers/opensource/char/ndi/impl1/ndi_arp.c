/*
 * <:copyright-BRCM:2020:DUAL/GPL:standard
 *
 *    Copyright (c) 2020 Broadcom
 *    All Rights Reserved
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
	if (!ipv4_addr_equal(addr, &p->dev->ip4))
		p->dev->ip4 = *addr;

	pr_debug("%s got arp response\n", ndi_dev_name(p->dev));
}
