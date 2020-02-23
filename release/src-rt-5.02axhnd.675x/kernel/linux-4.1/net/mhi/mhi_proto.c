#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

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
/*
 * File: mhi_proto.c
 *
 * Modem-Host Interface (MHI) Protocol Family
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/if_mhi.h>
#include <linux/mhi.h>
#include <linux/mhi_l2mux.h>

#include <net/af_mhi.h>
#include <net/mhi/sock.h>
#include <net/mhi/dgram.h>
#include <net/mhi/raw.h>

#ifdef CONFIG_MHI_DEBUG
# define DPRINTK(...)    pr_debug("AF_MHI: " __VA_ARGS__)
#else
# define DPRINTK(...)
#endif

/* Supported L2 protocols */
static __u8 mhi_protocols[MHI_L3_NPROTO] __read_mostly = { 0, };

/*** Functions ***/

int mhi_protocol_registered(int protocol)
{
	if (protocol >= 0 && protocol < MHI_L3_NPROTO)
		return mhi_protocols[protocol];
	if (protocol == MHI_L3_ANY)
		return 1;

	return 0;
}
EXPORT_SYMBOL(mhi_protocol_registered);

int mhi_register_protocol(int protocol)
{
	DPRINTK("mhi_register_protocol: %d\n", protocol);

	if (protocol < 0 || protocol >= MHI_L3_NPROTO)
		return -EINVAL;

	mhi_protocols[protocol] = 1;

	return 0;
}
EXPORT_SYMBOL(mhi_register_protocol);

int mhi_unregister_protocol(int protocol)
{
	DPRINTK("mhi_unregister_protocol: %d\n", protocol);

	if (protocol < 0 || protocol >= MHI_L3_NPROTO)
		return -EINVAL;

	mhi_protocols[protocol] = 0;

	return 0;
}
EXPORT_SYMBOL(mhi_unregister_protocol);

int mhi_skb_send(struct sk_buff *skb, struct net_device *dev, u8 proto)
{
	int err = 0;

	DPRINTK("mhi_skb_send: proto:%d skb_len:%d\n", proto, skb->len);

	skb->protocol = htons(ETH_P_MHI);
	skb->dev = dev;

	if (skb->pkt_type == PACKET_LOOPBACK) {
		skb_orphan(skb);
		netif_rx_ni(skb);
	} else {

		if ((proto == MHI_L3_XFILE) || (proto == MHI_L3_LOW_PRIO_TEST))
			skb->priority = 1;	/* Low prio */
		else if ((proto == MHI_L3_AUDIO)
			 || (proto == MHI_L3_TEST_PRIO)
			 || (proto == MHI_L3_HIGH_PRIO_TEST))
			skb->priority = 6;	/* high prio */
		else
			skb->priority = 0;	/* medium prio */
		err = dev_queue_xmit(skb);
	}

	return err;
}

int
mhi_skb_recv(struct sk_buff *skb,
	     struct net_device *dev,
	     struct packet_type *type, struct net_device *orig_dev)
{
	struct l2muxhdr *l2hdr;

	u8 l3pid;
	u32 l3len;
	int err;

	l2hdr = l2mux_hdr(skb);

	l3pid = l2mux_get_proto(l2hdr);
	l3len = l2mux_get_length(l2hdr);

	DPRINTK("mhi_skb_recv: skb_len:%d l3pid:%d l3len:%d\n",
		skb->len, l3pid, l3len);

	err = mhi_sock_rcv_multicast(skb, l3pid, l3len);

	return err;
}

static struct packet_type mhi_packet_type __read_mostly = {
	.type = cpu_to_be16(ETH_P_MHI),
	.func = mhi_skb_recv,
};

static int __init mhi_proto_init(void)
{
	int err;

	DPRINTK("mhi_proto_init\n");

	err = mhi_sock_init();
	if (err) {
		pr_alert("MHI socket layer registration failed\n");
		goto err0;
	}

	err = mhi_dgram_proto_init();
	if (err) {
		pr_alert("MHI DGRAM protocol layer registration failed\n");
		goto err1;
	}

	err = mhi_raw_proto_init();
	if (err) {
		pr_alert("MHI RAW protocol layer registration failed\n");
		goto err2;
	}

	dev_add_pack(&mhi_packet_type);

	return 0;

err2:
	mhi_dgram_proto_exit();
err1:
	mhi_sock_exit();
err0:
	return err;
}

static void __exit mhi_proto_exit(void)
{
	DPRINTK("mhi_proto_exit\n");

	dev_remove_pack(&mhi_packet_type);

	mhi_dgram_proto_exit();
	mhi_raw_proto_exit();
	mhi_sock_exit();
}

module_init(mhi_proto_init);
module_exit(mhi_proto_exit);

MODULE_ALIAS_NETPROTO(PF_MHI);

MODULE_DESCRIPTION("MHI Protocol Family for Linux");
#endif /* CONFIG_BCM_KF_MHI */
