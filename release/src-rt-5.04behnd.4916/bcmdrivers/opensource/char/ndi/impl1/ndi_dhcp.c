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
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/ndi.h>
#include "ndi_local.h"

/* --- types and consts --- */
#define DHCP_OPT_MAGIC		0x63825363

#define BOOTREQUEST		1
#define BOOTREPLY		2

/* DHCP operations */
#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8

/* DHCP option types */
#define DHCP_OPT_NETWORK_MASK	1
#define DHCP_OPT_ROUTERS	3
#define DHCP_OPT_SERVERS_DNS	6
#define DHCP_OPT_SERVERS_LOG	7
#define DHCP_OPT_HOSTNAME	12
#define DHCP_OPT_ADDR_REQUESTED	50
#define DHCP_OPT_ADDR_LEASETIME	51
#define DHCP_OPT_MESSAGE_TYPE	53
#define DHCP_OPT_SERVER_ID	54
#define DHCP_OPT_REQUEST_PARAMS	55
#define DHCP_OPT_MESSAGE	56
#define DHCP_OPT_MAX_MSG_SIZE	57
#define DHCP_OPT_T1_RENEW_TIME	58
#define DHCP_OPT_T2_REBIND_TIME	59
#define DHCP_OPT_VENDOR_CLASSID	60
#define DHCP_OPT_CLIENT_ID	61
#define DHCP_OPT_TFTP_SNAME	66
#define DHCP_OPT_TFTP_FILENAME	67
#define DHCP_OPT_RAPID_COMMIT	80
#define DHCP_OPT_RELAY_INFO	82
#define DHCP_OPT_SYSTEM_ARCH	93
#define DHCP_OPT_NET_DEV_IFACE	94
#define DHCP_OPT_SERVERS_TFTP	150
#define DHCP_OPT_END		255

struct dhcp_pkt {
	u8	op;
	u8	htype;
	u8	hlen;
	u8	hops;

	u32	xid;

	u16	secs;
	u16	flags;

	struct in_addr	ciaddr;
	struct in_addr	yiaddr;
	struct in_addr	siaddr;
	struct in_addr	giaddr;

	u8	chaddr[16];
	u8	sname[64];
	u8	file[128];
	u32	magic;		/* DHCP magic value */
	u8	options[0];	/* options */
} __packed;

/* --- functions --- */
static int
ignore_dhcp_packet(struct ndi_classify *p, struct dhcp_pkt *dp, int total_len)
{
	if (total_len < sizeof(*dp)) {
		pr_debug("DHCP packet too small (%d, must be >= %lu)\n",
			 total_len, (unsigned long)sizeof(*dp));
		return 1;
	}

	if (is_zero_ether_addr(dp->chaddr)) {
		pr_debug("DHCP packet contains empty MAC\n");
		return 1;
	}

	return 0;
}

static void
dhcp_device_check(struct ndi_classify *p, struct dhcp_pkt *dp)
{
	struct ndi_dev *old;

	if (dp->op != BOOTREQUEST) {
		pr_debug("don't reclassify non-request\n");
		return;
	}

	if (p->dev && ether_addr_equal(p->dev->mac, dp->chaddr))
		return;

	old = p->dev;
	/* if the source MAC is not the same as the requester, change the
	 * classification to use the chaddr in the DHCP packet. */
	p->dev = dev_find_or_new(dp->chaddr, p->skb);
	if (p->dev) {
		if (old)
			pr_debug("device changed from [%pM] to [%pM]\n",
				  old->mac, p->dev->mac);
		else
			pr_debug("new or renewed device [%pM]\n", p->dev->mac);
		set_bit(UPDATED_BIT, &p->flags);
	}
}

static void dhcp_ack(struct ndi_classify *p, struct dhcp_pkt *dp)
{
	u32 *ciaddr = (u32*)&dp->ciaddr;
	u32 *yiaddr = (u32*)&dp->yiaddr;

	if (ipv4_ignore(*yiaddr) || is_zero_ether_addr(dp->chaddr))
		return;

	/* Allocate IP->MAC mapping for new DHCP issuances */
	pr_debug("[%pM] DHCPACK %pI4\n", dp->chaddr, yiaddr);

	/* if renewal changed addresses, free the old address */
	if (*ciaddr != *yiaddr) {
		struct ndi_ip *old;
		old = ip_find(ciaddr, NFPROTO_IPV4);
		if (old)
			ip_free(old);
	}

	ip_find_or_new(yiaddr, NFPROTO_IPV4, dp->chaddr);
}

static int set_hostname(struct ndi_classify *p, char *str, size_t len)
{
	if (!p->dev)
		return 0;
	if (strncmp(p->dev->hostname, str, len) == 0)
		return 1;

	memset(p->dev->hostname, 0, sizeof(p->dev->hostname));
	memcpy(p->dev->hostname, str, len);
	pr_debug("%s updated hostname to %s\n", ndi_dev_name(p->dev),
		 p->dev->hostname);
	set_bit(UPDATED_BIT, &p->flags);
	return 0;
}

void ndi_parse_dhcp(struct ndi_classify *p)
{
	struct udphdr *uh	= udp_hdr(p->skb);
	struct dhcp_pkt *dp	= (void*)(uh + 1);
	int total_len		= ntohs(uh->len);
	u8 *data		= &dp->options[0];

	if (ignore_dhcp_packet(p, dp, total_len))
		return;

	dhcp_device_check(p, dp);

	/* copy the DHCP hostname if it exists */
	if (strnlen(dp->sname, sizeof(dp->sname))) {
		if (!set_hostname(p, dp->sname, sizeof(dp->sname)))
			pr_debug("DHCP sname '%s'\n", p->dev->hostname);
	}

	if (dp->magic != htonl(DHCP_OPT_MAGIC)) {
		pr_debug("skipping BOOTP message\n");
		return;
	}

	total_len -= offsetof(struct dhcp_pkt, options);
	while (total_len > 0) {
		u8 tag = data[0];
		u8 len = total_len > 1 ? data[1] : 0;

		pr_debug("tag %d, len %d\n", tag, len);

		/* validate */
		if (len > total_len || !len)
			break;

		/* copy hostname from DHCP options */
		if (tag == DHCP_OPT_HOSTNAME) {
			size_t strlen = len;
			strlen = min(strlen, sizeof(p->dev->hostname) - 1);
			if (set_hostname(p, &data[2], strlen))
				pr_debug("DHCP hostname '%s'\n",
					 p->dev->hostname);
			break;
		}
		if (tag == DHCP_OPT_MESSAGE_TYPE) {
			if (data[2] == DHCPACK)
				dhcp_ack(p, dp);
		}

		total_len	-= len + 2;
		data		+= len + 2;
	}
}
