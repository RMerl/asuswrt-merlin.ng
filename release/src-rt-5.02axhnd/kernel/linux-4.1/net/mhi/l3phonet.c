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
 * File: l3phonet.c
 *
 * L2 PHONET channel to AF_PHONET binding.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/mhi.h>
#include <linux/mhi_l2mux.h>

/* Functions */

static int mhi_pn_netif_rx(struct sk_buff *skb, struct net_device *dev)
{
	/* Set Protocol Family */
	skb->protocol = htons(ETH_P_PHONET);

	/* Remove L2MUX header and Phonet media byte */
	skb_pull(skb, L2MUX_HDR_SIZE + 1);

	/* Pass upwards to the Procotol Family */
	return netif_rx(skb);
}

static int mhi_pn_netif_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct l2muxhdr *l2hdr;
	int l3len;
	u8 *ptr;

	/* Add media byte */
	ptr = skb_push(skb, 1);

	/* Set media byte */
	ptr[0] = dev->dev_addr[0];

	/* L3 length */
	l3len = skb->len;

	/* Add L2MUX header */
	skb_push(skb, L2MUX_HDR_SIZE);

	/* Mac header starts here */
	skb_reset_mac_header(skb);

	/* L2MUX header pointer */
	l2hdr = l2mux_hdr(skb);

	/* L3 Proto ID */
	l2mux_set_proto(l2hdr, MHI_L3_PHONET);

	/* L3 payload length */
	l2mux_set_length(l2hdr, l3len);

	return 0;
}

/* Module registration */

int __init mhi_pn_init(void)
{
	int err;

	err = l2mux_netif_rx_register(MHI_L3_PHONET, mhi_pn_netif_rx);
	if (err)
		goto err1;

	err = l2mux_netif_tx_register(ETH_P_PHONET, mhi_pn_netif_tx);
	if (err)
		goto err2;

	return 0;

err2:
	l2mux_netif_rx_unregister(MHI_L3_PHONET);
err1:
	return err;
}

void __exit mhi_pn_exit(void)
{
	l2mux_netif_rx_unregister(MHI_L3_PHONET);
	l2mux_netif_tx_unregister(ETH_P_PHONET);
}

module_init(mhi_pn_init);
module_exit(mhi_pn_exit);

MODULE_DESCRIPTION("MHI Phonet protocol family bridge");
#endif /* CONFIG_BCM_KF_MHI */
