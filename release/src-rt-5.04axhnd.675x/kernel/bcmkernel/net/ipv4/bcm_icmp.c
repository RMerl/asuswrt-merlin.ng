/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include <net/ip.h>
#include <net/icmp.h>
#include <net/bcm_icmp.h>

/*
 * This function is only used by MAP-T (or MAP-E) feature.
 * If LAN IPv4 packet with DF flag set and translated IPv6 packet
 * exceeds mtu, this function is used to send fragment needed ICMP
 */
void send_icmp_frag(struct sk_buff *skb_in, int type, int code, __be32 info)
{
	struct iphdr *iph;
	int room;
	struct icmp_bxm *icmp_param;
	struct rtable *rt;
	struct ipcm_cookie ipc;
	struct flowi4 fl4;
	__be32 saddr;
	u8  tos;
	u32 mark;
	struct net *net;

	net = dev_net(skb_in->dev);
	iph = ip_hdr(skb_in);

	if ((u8 *)iph < skb_in->head ||
	    (skb_network_header(skb_in) + sizeof(*iph)) >
	    skb_tail_pointer(skb_in))
		goto out;

	if (skb_in->pkt_type != PACKET_HOST)
		goto out;

	/*
	 *	Only reply to fragment 0. We byte re-order the constant
	 *	mask for efficiency.
	 */
	if (iph->frag_off & htons(IP_OFFSET))
		goto out;

	icmp_param = kmalloc(sizeof(*icmp_param), GFP_ATOMIC);
	if (!icmp_param)
		return;

	saddr = 0;    

	tos = icmp_pointers[type].error ? ((iph->tos & IPTOS_TOS_MASK) |
					   IPTOS_PREC_INTERNETCONTROL) :
					  iph->tos;
	mark = skb_in->mark;

	if (ip_options_echo(net, &icmp_param->replyopts.opt.opt, skb_in))
		goto out_free;


	/*
	 *	Prepare data for ICMP header.
	 */
	icmp_param->data.icmph.type	 = type;
	icmp_param->data.icmph.code	 = code;
	icmp_param->data.icmph.un.gateway = info;
	icmp_param->data.icmph.checksum	 = 0;
	icmp_param->skb	  = skb_in;
	icmp_param->offset = skb_network_offset(skb_in);
	ipcm_init(&ipc);
	ipc.addr = iph->saddr;
	ipc.opt = &icmp_param->replyopts.opt;

	rt = icmp_route_lookup(net, &fl4, skb_in, iph, saddr, tos, mark,
			       type, code, icmp_param);
	if (IS_ERR(rt))
		goto out_free;

	room = dst_mtu(&rt->dst);
	if (room > 576)
		room = 576;
	room -= sizeof(struct iphdr) + icmp_param->replyopts.opt.opt.optlen;
	room -= sizeof(struct icmphdr);

	icmp_param->data_len = skb_in->len - icmp_param->offset;
	if (icmp_param->data_len > room)
		icmp_param->data_len = room;
	icmp_param->head_len = sizeof(struct icmphdr);

	icmp_push_reply(icmp_param, &fl4, &ipc, &rt);

out_free:
	kfree(icmp_param);
out:;  
}
EXPORT_SYMBOL(send_icmp_frag);
