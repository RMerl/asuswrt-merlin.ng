#ifdef CONFIG_BCM_KF_PHONET
/*
<:copyright-BRCM:2014:DUAL/GPL:standard

   Copyright (c) 2014 Broadcom 
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
 * File: ld_tmodem.c
 * Thin Modem device TTY line discipline
 */

#include <linux/debugfs.h>
#include <linux/if_arp.h>
#include <linux/if_phonet.h>
#include <linux/mhi_l2mux.h>
#include <linux/module.h>
#include <linux/phonet.h>
#include <linux/tty.h>
#include <net/af_mhi.h>
#include <net/phonet/phonet.h>
#include <net/phonet/pn_dev.h>
#include <net/sock.h>

#define LD_TMODEM_PLATFORM_DRIVER_NAME	"ld_tmodem"
#define PN_HARD_HEADER_LEN	1
#define NETDEV_HARD_HEADER_LEN	(PN_HARD_HEADER_LEN + L2MUX_HDR_SIZE)
#define TMODEM_SENDING_BIT	1 /* Bit 1 = 0x02 */
#define LD_TMODEM_BUFFER_LEN	SZ_1M
#define LD_TMODEM_INIT_LEN	0
#define LD_WAKEUP_DATA_INIT	0
#define MHI_MAX_MTU		65540
#define LD_RECEIVE_ROOM		SZ_64K

#define PN_DEV_AUX_HOST		0x44	/* Additional host */
#define PN_DEV_MODEM		0x60	/* Modem */
#define PN_DEV_MODEM_1		0x64	/* Modem 1 */
#define PN_DEV_HOST		0

#define LOW_PRIORITY	0
#define MEDIUM_PRIORITY	1
#define HIGH_PRIORITY	6

struct ldtmdm_dbg_stats {
	u64 ld_tmodem_tx_request_count;
	u64 ld_tmodem_rx_request_count;
	u64 ld_tmodem_tx_bytes;
	u64 ld_tmodem_rx_bytes;
	u64 ld_tmodem_hangup_events;
	u64 ld_tmodem_drop_events;
	u64 ld_tmodem_skb_tx_err;
};

struct ld_tty_wakeup_work_t {
	struct work_struct ld_work;
	/*This holds TTY info for TTY wakeup */
	struct tty_struct *ld_work_write_wakeup_tty;
};

struct ld_tmodem {
	struct tty_struct *tty;
	wait_queue_head_t wait;
	spinlock_t lock;
	unsigned long flags;
	struct sk_buff *skb;
	unsigned long len;
	unsigned long state;
	struct net_device *dev;
	struct list_head node;
	struct sk_buff_head head;
	char *tty_name;
	bool link_up;
	char *pending_buffer;
	unsigned int pending_length;
	struct dentry *root;
	struct workqueue_struct *ld_tmodem_wq;
	struct ld_tty_wakeup_work_t *ld_tty_wakeup_work;
	struct ldtmdm_dbg_stats ldtmdm_dbg;
	int ld_backlog_len;	/* LD Phonet Tx Backlog buffer Len */
};

struct ld_tmodem *gb_ld_tmodem;

struct prop_ctrl_msg {
	struct l2muxhdr l2hdr;
	u8 ch_id;
	u8 event;
	u8 rsvd[2];
};

/* L2MUX Queue mapping for protocols */
enum tmodem_l2mux_queue {
	LD_TMODEM_L2MUX_QUEUE_1_PHONET,
	LD_TMODEM_L2MUX_QUEUE_2_MHI,
};

enum tmodem_flow_ctrl_events {
	HOST_STOP_SENDING_MSG = 1,
	HOST_RESUME_SENDING_MSG,
};

static const char ld_tmodem_ifname[] = "smc0";

static void free_ld_tmodem_skb(struct sk_buff *skb)
{
	if (in_interrupt())
		dev_kfree_skb_irq(skb);
	else
		kfree_skb(skb);
}

static int mhi_flow_ctrl_rx(struct sk_buff *skb, struct net_device *dev)
{
	struct prop_ctrl_msg *ctrl_ptr = (struct prop_ctrl_msg *)skb->data;
	int subqueue;

	BUG_ON(dev == NULL);

	netdev_dbg(dev, "channel id %d, event id %d\n",
				ctrl_ptr->ch_id, ctrl_ptr->event);

	if (skb == NULL) {
		netdev_err(dev, "skb received is NULL");
		return -ENOMEM;
	}

	switch (ctrl_ptr->ch_id) {
	case LD_TMODEM_L2MUX_QUEUE_1_PHONET:
		subqueue = LD_TMODEM_L2MUX_QUEUE_1_PHONET;
		break;
	case LD_TMODEM_L2MUX_QUEUE_2_MHI:
		subqueue = LD_TMODEM_L2MUX_QUEUE_2_MHI;
		break;
	default:
		netdev_err(dev, "ch id not supported %d",
			ctrl_ptr->ch_id);
		free_ld_tmodem_skb(skb);
		return 0;
	}

	switch (ctrl_ptr->event) {
	case HOST_STOP_SENDING_MSG:
		netif_stop_subqueue(dev, subqueue);
		break;
	case HOST_RESUME_SENDING_MSG:
		netif_wake_subqueue(dev, subqueue);
		break;
	default:
		netdev_err(dev, "event not supported %d",
					ctrl_ptr->event);
		free_ld_tmodem_skb(skb);
		return 0;
	}
	free_ld_tmodem_skb(skb);
	return 0;
}

static int ld_tmdm_net_open(struct net_device *dev)
{
	netdev_dbg(dev, "ld_tmdm_net_open: wakeup queues");
	netif_tx_wake_all_queues(dev);
	phonet_route_add(dev, PN_DEV_MODEM);
	phonet_route_add(dev, PN_DEV_MODEM_1);
	phonet_route_add(dev, PN_DEV_AUX_HOST);
	netif_carrier_on(dev);
	return 0;
}

static int ld_tmdm_net_close(struct net_device *dev)
{
	netdev_dbg(dev, "ld_tmdm_net_close: stop all queues");
	netif_tx_stop_all_queues(dev);
	phonet_route_del(dev, PN_DEV_MODEM);
	phonet_route_del(dev, PN_DEV_MODEM_1);
	phonet_route_del(dev, PN_DEV_AUX_HOST);
	netif_carrier_off(dev);
	return 0;
}

static void ld_tx_overflow(struct ld_tmodem *ld_tmdm)
{
	struct ldtmdm_dbg_stats *dbg;
	dbg = &ld_tmdm->ldtmdm_dbg;
	dbg->ld_tmodem_drop_events++;
	pr_debug(
		"##### ATTENTION : LD Phonet Transmit overflow events %llu : %u #####",
		dbg->ld_tmodem_drop_events, LD_TMODEM_BUFFER_LEN);
}

static int ld_tmdm_handle_tx(struct ld_tmodem *ld_tmdm)
{
	struct tty_struct *tty = ld_tmdm->tty;
	struct sk_buff *skb;
	unsigned int tty_wr, room;
	struct sk_buff *tmp;
	int ret;

	if (tty == NULL)
		return 0;
	/* Enter critical section */
	if (test_and_set_bit(TMODEM_SENDING_BIT, &ld_tmdm->state))
		return 0;

	/* skb_peek is safe because handle_tx is called after skb_queue_tail */
	while ((skb = skb_peek(&ld_tmdm->head)) != NULL) {
		room = tty_write_room(tty);

		if (!room && ld_tmdm->ld_backlog_len > LD_TMODEM_BUFFER_LEN) {
			if (ld_tmdm->link_up == true)
				ld_tx_overflow(ld_tmdm);
			ld_tmdm->link_up = false;
			/* Flush TX queue */
			while ((skb = skb_dequeue(&ld_tmdm->head)) != NULL) {
				skb->dev->stats.tx_dropped++;
				netdev_dbg(ld_tmdm->dev, "Flush TX queue tx_dropped = %ld",
				skb->dev->stats.tx_dropped);
				free_ld_tmodem_skb(skb);
			}
			ld_tmdm->ld_backlog_len = LD_TMODEM_INIT_LEN;
			goto out;
		} else if (!room) {
			netdev_dbg(ld_tmdm->dev,
				"ld_tmdm_handle_tx no room, waiting for previous to be sent..:\n");

			if (!test_bit(TTY_DO_WRITE_WAKEUP, &tty->flags)) {
				/* wakeup bit is not set, set it */
				netdev_dbg(ld_tmdm->dev,
					"ld_tmdm_handle_tx Setting TTY_DO_WRITE_WAKEUP bit\n");
				set_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
			} else {
				netdev_dbg(ld_tmdm->dev,
					"ld_tmdm_handle_tx TTY_DO_WRITE_WAKEUP bit already set!\n");
			}
			goto out;
		}

		if (skb->len > (room + NETDEV_HARD_HEADER_LEN)) {
			netdev_warn(ld_tmdm->dev, "%s :room not available buf len:%d, room:%d\n"
						, __func__, skb->len, room);
			goto out;
		}

		ret = l2mux_skb_tx(skb, ld_tmdm->dev);
		if (ret) {
			/*drop this packet...*/
			struct sk_buff *tmp = skb_dequeue(&ld_tmdm->head);
			BUG_ON(tmp != skb);
			free_ld_tmodem_skb(skb);
			ld_tmdm->ldtmdm_dbg.ld_tmodem_skb_tx_err++;
			goto out;
		}

		tty_wr = tty->ops->write(tty, skb->data, skb->len);

		print_hex_dump(KERN_DEBUG, "ld_tm_write: ", DUMP_PREFIX_NONE, 16, 1,
						skb->data, skb->len, 1);

		if (unlikely(tty_wr != skb->len)) {
			netdev_err(ld_tmdm->dev,
						"buffer split...req len :%d, act written:%d\n",
							skb->len, tty_wr);
		}
		ld_tmdm->ld_backlog_len -= skb->len;

			ld_tmdm->ldtmdm_dbg.ld_tmodem_tx_bytes += tty_wr;
		if (ld_tmdm->ld_backlog_len < LD_TMODEM_INIT_LEN)
			ld_tmdm->ld_backlog_len = LD_TMODEM_INIT_LEN;

		ld_tmdm->dev->stats.tx_packets++;
		ld_tmdm->dev->stats.tx_bytes += tty_wr;

		tmp = skb_dequeue(&ld_tmdm->head);

		BUG_ON(tmp != skb);
		free_ld_tmodem_skb(skb);

	}
out:
	clear_bit(TMODEM_SENDING_BIT, &ld_tmdm->state);
	return NETDEV_TX_OK;
}

int l2mux_send(char *data, int len, int l3_id, int res)
{
	struct net_device *dev = NULL;
	struct l2muxhdr *l2hdr;
	struct sk_buff *skb;
	struct net *net;
	int ret;

	skb = alloc_skb(len + L2MUX_HDR_SIZE, GFP_ATOMIC);
	if (!skb) {
		pr_err("l2mux_send: skb_alloc failed\n");
		return -ENOMEM;
	}

	skb_reserve(skb, L2MUX_HDR_SIZE);
	skb_reset_transport_header(skb);

	memcpy(skb_put(skb, len), data, len);

	for_each_net(net) {
		dev = dev_get_by_name(net, ld_tmodem_ifname);
		if (dev)
			break;
	}

	if (!dev) {
		ret = -ENODEV;
		pr_err("l2mux_send: interface:%s not found\n",
							ld_tmodem_ifname);
		goto drop;
	}

	if (!(dev->flags & IFF_UP)) {
		netdev_err(dev, "l2mux_send: device %s not IFF_UP\n",
							ld_tmodem_ifname);
		ret = -ENETDOWN;
		goto drop;
	}

	if (len + L2MUX_HDR_SIZE > dev->mtu) {
		netdev_err(dev, "l2mux_send: device %s not IFF_UP\n",
						ld_tmodem_ifname);
		ret = -EMSGSIZE;
		goto drop;
	}

	skb_reset_network_header(skb);

	skb_push(skb, L2MUX_HDR_SIZE);
	skb_reset_mac_header(skb);

	l2hdr = l2mux_hdr(skb);
	l2mux_set_proto(l2hdr, l3_id);
	l2mux_set_length(l2hdr, len);
	netdev_dbg(dev, "l2mux_send: proto:%d skb_len:%d\n",
					l3_id, skb->len);
	skb->protocol = htons(ETH_P_MHI);
	skb->dev = dev;

	switch (l3_id) {
	case MHI_L3_XFILE:
	case MHI_L3_LOW_PRIO_TEST:
		skb->priority = MEDIUM_PRIORITY;
		break;
	case MHI_L3_AUDIO:
	case MHI_L3_TEST_PRIO:
	case MHI_L3_HIGH_PRIO_TEST:
		skb->priority = HIGH_PRIORITY;
		break;
	default:
		skb->priority = LOW_PRIORITY;
		break;
	}
	ret = dev_queue_xmit(skb); /*this will consume irrespective of status*/

	dev_put(dev);
	return ret;

drop:
	if (skb)
		kfree_skb(skb);
	if (dev)
		dev_put(dev);

	return ret;
}
EXPORT_SYMBOL(l2mux_send);

static int ld_tmdm_net_xmit(struct sk_buff *skb, struct net_device *dev)
{

	struct ld_tmodem *ld_tmdm;

	BUG_ON(dev == NULL);

	print_hex_dump(KERN_DEBUG, "ld_tm_xmit: ", DUMP_PREFIX_NONE,
					16, 1, skb->data, skb->len, 1);

	ld_tmdm = netdev_priv(dev);
	if ((ld_tmdm == NULL)) {
		free_ld_tmodem_skb(skb);
		return NETDEV_TX_OK;
	}
	ld_tmdm->ldtmdm_dbg.ld_tmodem_tx_request_count++;
	netdev_dbg(dev, "ld_tmdm_net_xmit: send skb to %s", dev->name);
	if (ld_tmdm->link_up == true) {
		skb_queue_tail(&ld_tmdm->head, skb);
		ld_tmdm->ld_backlog_len += skb->len;
		return ld_tmdm_handle_tx(ld_tmdm);
	} else if (tty_write_room(ld_tmdm->tty)) {
		/* link is up again */
		ld_tmdm->link_up = true;
		skb_queue_tail(&ld_tmdm->head, skb);
		ld_tmdm->ld_backlog_len += skb->len;
		return ld_tmdm_handle_tx(ld_tmdm);
	} else {
		free_ld_tmodem_skb(skb);
		dev->stats.tx_dropped++;
		netdev_dbg(dev, "tx_dropped = %ld",
						dev->stats.tx_dropped);
		return NETDEV_TX_OK;
	}

}

static int ld_tmdm_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct if_phonet_req *req = (struct if_phonet_req *)ifr;

	netdev_dbg(dev, "IOCTL called\n");
	switch (cmd) {
	case SIOCPNGAUTOCONF:
		netdev_dbg(dev, "IOCTL SIOCPNGAUTOCON called, adding routes\n");
		req->ifr_phonet_autoconf.device = PN_DEV_HOST;
		//dev_open(dev);
		return 0;
	}
	/* Return NOIOCTLCMD so Phonet won't do it again */
	return -ENOIOCTLCMD;
}

static u16
ld_tmdm_net_select_queue(struct net_device *dev, struct sk_buff *skb,
		void *accel_priv, select_queue_fallback_t fallback)
{

	struct ld_tmodem *ld_tmdm;
	u16 subqueue = 0;/*default return value is phonet sub queue*/

	BUG_ON(dev == NULL);
	ld_tmdm = netdev_priv(dev);

	if (skb->protocol == htons(ETH_P_PHONET)) {
		netdev_dbg(dev, "ld_tmdm_net_select_queue: protocol ETH_P_PHONET");
		subqueue = LD_TMODEM_L2MUX_QUEUE_1_PHONET;
	} else if (skb->protocol == htons(ETH_P_MHI)) {
		netdev_dbg(dev, "ld_tmdm_net_select_queue: protocol ETH_P_MHI");
		subqueue = LD_TMODEM_L2MUX_QUEUE_2_MHI;
	} else
		netdev_err(dev, "unsupported protocol device %p, 0x%04X",
							dev, skb->protocol);

	return subqueue;
}

static int ld_tmdm_set_mtu(struct net_device *dev, int new_mtu)
{
	dev->mtu = new_mtu;
	return 0;
}

static const struct net_device_ops ld_tmdm_netdev_ops = {
	.ndo_open = ld_tmdm_net_open,
	.ndo_stop = ld_tmdm_net_close,
	.ndo_select_queue = ld_tmdm_net_select_queue,
	.ndo_start_xmit = ld_tmdm_net_xmit,
	.ndo_do_ioctl = ld_tmdm_net_ioctl,
	.ndo_change_mtu = ld_tmdm_set_mtu,
};

static void ld_tmdm_net_setup(struct net_device *dev)
{
	dev->features		= NETIF_F_SG;
	dev->type		= ARPHRD_MHI;
	dev->flags		= IFF_POINTOPOINT | IFF_NOARP;
	dev->mtu		= MHI_MAX_MTU;
	dev->hard_header_len	= NETDEV_HARD_HEADER_LEN;
	dev->dev_addr[0]	= PN_MEDIA_AUX_HOST_HOST_IF;
	dev->addr_len		= 1;
	dev->tx_queue_len	= 500;
	dev->netdev_ops		= &ld_tmdm_netdev_ops;
	dev->destructor		= free_netdev;
};

static int ld_tmodem_ldisc_open(struct tty_struct *tty)
{
	struct ld_tmodem *ld_tmdm = gb_ld_tmodem;
	ld_tmdm->tty = tty;
	tty->disc_data = ld_tmdm;

	gb_ld_tmodem->tty = tty;
	tty->receive_room = LD_RECEIVE_ROOM;
	tty->port->low_latency = 1;
	return 0;
}

static void ld_tmodem_ldisc_close(struct tty_struct *tty)
{
	struct ld_tmodem *ld_tmdm = tty->disc_data;
	netdev_dbg(ld_tmdm->dev, "ld_tmodem_ldisc_close\n");
	tty->disc_data = NULL;
	ld_tmdm->tty = NULL;
	ld_tmdm->ld_tty_wakeup_work->ld_work_write_wakeup_tty = NULL;
}

static void send_buffer_to_l2mux(struct ld_tmodem *ld_tmdm,
				const unsigned char *buff_ptr,
							unsigned int count)
{
	struct sk_buff *skb;

	skb = netdev_alloc_skb(ld_tmdm->dev, count + NET_IP_ALIGN);
	if (NULL == skb)
		return;

	skb_reserve(skb, NET_IP_ALIGN);

	memcpy(skb->data, buff_ptr, count);
	skb_put(skb, count);

	skb_reset_mac_header(skb);
	skb->pkt_type = PACKET_HOST;

	/*no need to free skb: skb will be consumed by callee in all the cases*/
	if (l2mux_skb_rx(skb, ld_tmdm->dev) != 0)
		netdev_err(ld_tmdm->dev, "not able to send skb");

	ld_tmdm->ldtmdm_dbg.ld_tmodem_rx_bytes += count;

}

static void process_rx_buffer(struct ld_tmodem *ld_tmdm, int curr_len,
						const unsigned char *buff_ptr)
{
	unsigned int pending_len = ld_tmdm->pending_length;
	const unsigned char *pbuff_start;
	struct l2muxhdr *pl2msg;
	int l3len;

	if (pending_len + curr_len < L2MUX_HDR_SIZE) {
		memcpy(ld_tmdm->pending_buffer + pending_len,
		buff_ptr,
		curr_len);
		ld_tmdm->pending_length += curr_len;
		return;
	}

	netdev_dbg(ld_tmdm->dev, "pending_length %d", pending_len);
	while (1) {
		if (pending_len && pending_len < L2MUX_HDR_SIZE) {
			memcpy(ld_tmdm->pending_buffer + pending_len,
				buff_ptr,
				L2MUX_HDR_SIZE - pending_len);
			curr_len -= (L2MUX_HDR_SIZE - pending_len);
			buff_ptr += (L2MUX_HDR_SIZE - pending_len);

			ld_tmdm->pending_length = L2MUX_HDR_SIZE;
			pending_len = ld_tmdm->pending_length;
		}

		if (pending_len)
			pbuff_start = ld_tmdm->pending_buffer;
		else
			pbuff_start = buff_ptr;

		pl2msg = (struct l2muxhdr *)pbuff_start;
		l3len = l2mux_get_length(pl2msg);

		if (pending_len + curr_len < l3len + L2MUX_HDR_SIZE) {
			memcpy(ld_tmdm->pending_buffer + pending_len,
			buff_ptr,
			curr_len);

			ld_tmdm->pending_length += curr_len;
			break;
		}

		if (pending_len) {
			memcpy(ld_tmdm->pending_buffer + pending_len,
				buff_ptr,
				l3len + L2MUX_HDR_SIZE - pending_len);
		}

		print_hex_dump(KERN_DEBUG, "ld_tm_process_buff: ",
				DUMP_PREFIX_NONE,
				16, 1,
				pbuff_start, l3len + L2MUX_HDR_SIZE,
				1);

		netdev_dbg(ld_tmdm->dev,
			"total pending length %d",
			pending_len);

		send_buffer_to_l2mux(ld_tmdm, pbuff_start,
					l3len + L2MUX_HDR_SIZE);

		curr_len -= (l3len + L2MUX_HDR_SIZE - pending_len);
		buff_ptr += (l3len + L2MUX_HDR_SIZE - pending_len);

		ld_tmdm->pending_length = 0;
		pending_len = ld_tmdm->pending_length;
		if (curr_len > 0) {
			netdev_dbg(ld_tmdm->dev,
			"current length is more than zero %d",
			curr_len);
			continue;
		} else
			break;
	}

}

static void ld_tmodem_ldisc_receive
	(struct tty_struct *tty, const unsigned char *cp, char *fp, int count)
{
	struct ld_tmodem *ld_tmdm = tty->disc_data;
	ld_tmdm->ldtmdm_dbg.ld_tmodem_rx_request_count++;
	if (ld_tmdm->link_up == false) {
		/* data received from PC => can TX */
		ld_tmdm->link_up = true;
	}
	process_rx_buffer(ld_tmdm, count, cp);
}

static void ld_tty_wakeup_workfunction(struct work_struct *work)
{
	struct tty_struct *tty;
	struct ld_tmodem *ld_tmdm;
	struct ld_tty_wakeup_work_t *ld_work_tty_wk =
				(struct ld_tty_wakeup_work_t *)work;

	if (ld_work_tty_wk == NULL) {
		pr_err("TTY work NULL\n");
		return;
	}

	tty = ld_work_tty_wk->ld_work_write_wakeup_tty;
	if (tty == NULL) {
		pr_err("LD Work Queue tty Data NULL\n");
		return;
	}

	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	ld_tmdm = tty->disc_data;
	if (ld_tmdm == NULL) {
		pr_err("LD PN Work Queue DATA NULL\n");
		return;
	}

	BUG_ON(ld_tmdm->tty != tty);
	ld_tmdm_handle_tx(ld_tmdm);
}

static void ld_tmodem_ldisc_write_wakeup(struct tty_struct *tty)
{
	struct ld_tmodem *ld_tmdm;
	ld_tmdm = tty->disc_data;
	if (!ld_tmdm) {
		pr_err("ld modem ldisc write wakeup: ld_tmdm is NULL\n");
		return;
	}
	ld_tmdm->ld_tty_wakeup_work->ld_work_write_wakeup_tty = tty;
	queue_work(ld_tmdm->ld_tmodem_wq,
			(struct work_struct *)ld_tmdm->ld_tty_wakeup_work);
}

static int ld_tmodem_hangup_wait(void *data)
{
	return NETDEV_TX_OK;
}

static int ld_tmodem_ldisc_hangup(struct tty_struct *tty)
{
	struct ld_tmodem *ld_tmdm;
	struct sk_buff *skb;

	ld_tmdm = tty->disc_data;
	ld_tmdm->ldtmdm_dbg.ld_tmodem_hangup_events++;
	wait_on_bit_lock(&ld_tmdm->state, TMODEM_SENDING_BIT,
			 ld_tmodem_hangup_wait, TASK_KILLABLE);

	while ((skb = skb_dequeue(&ld_tmdm->head)) != NULL) {
		skb->dev->stats.tx_dropped++;
		free_ld_tmodem_skb(skb);
	}
	ld_tmdm->ld_backlog_len = LD_TMODEM_INIT_LEN;
	clear_bit(TMODEM_SENDING_BIT, &ld_tmdm->state);
	return NETDEV_TX_OK;
}

static struct tty_ldisc_ops ld_tmodem_ldisc = {
	.owner = THIS_MODULE,
	.name = "tmodem",
	.open = ld_tmodem_ldisc_open,
	.close = ld_tmodem_ldisc_close,
	.receive_buf = ld_tmodem_ldisc_receive,
	.write_wakeup = ld_tmodem_ldisc_write_wakeup,
	.hangup = ld_tmodem_ldisc_hangup
};

static ssize_t tmdm_dbg_write(struct file *file, const char __user *buf,
		size_t size, loff_t *ppos)
{
	if (!gb_ld_tmodem) {
		pr_err("gb_ld_tmodem is NULL");
		return -EBUSY;
	}
	/*resetting received and transmitted stats; while keeping the
	*error stats intact
	*/
	gb_ld_tmodem->ldtmdm_dbg.ld_tmodem_tx_request_count =
	gb_ld_tmodem->ldtmdm_dbg.ld_tmodem_rx_request_count =
	gb_ld_tmodem->ldtmdm_dbg.ld_tmodem_tx_bytes =
	gb_ld_tmodem->ldtmdm_dbg.ld_tmodem_rx_bytes = 0;

	return size;
}

static ssize_t tmdm_dbg_read(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char *buf;
	int used = 0;
	int ret;
	struct ldtmdm_dbg_stats *dbg;
	if (!gb_ld_tmodem) {
		pr_err("gb_ld_tmodem is NULL");
		return -EBUSY;
	}

	dbg = &gb_ld_tmodem->ldtmdm_dbg;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = snprintf(buf + used, PAGE_SIZE - used,
				"LD Thin Modem Tx request: %llu\n",
				dbg->ld_tmodem_tx_request_count);
	if (ret < 0)
		goto out;

	used = used + ret;

	if (used >= PAGE_SIZE - 1)
		goto out;

	ret = snprintf(buf + used,
					PAGE_SIZE - used,
					"LD Thin Modem Rx request: %llu\n",
					dbg->ld_tmodem_rx_request_count);
	if (ret < 0)
		goto out;

	used = used + ret;

	if (used >= PAGE_SIZE - 1)
		goto out;

	ret = snprintf(buf + used,
					PAGE_SIZE - used,
					"LD Thin Modem Tx Bytes: %llu\n",
					dbg->ld_tmodem_tx_bytes);
	if (ret < 0)
		goto out;

	used = used + ret;

	if (used >= PAGE_SIZE - 1)
		goto out;

	ret = snprintf(buf + used,
					PAGE_SIZE - used,
					"LD Thin Modem Tx Bytes: %llu\n",
					dbg->ld_tmodem_rx_bytes);
	if (ret < 0)
		goto out;

	used = used + ret;

	if (used >= PAGE_SIZE - 1)
		goto out;

	ret = snprintf(buf + used,
				PAGE_SIZE - used,
				"LD Thin Modem hangup events: %llu\n",
				dbg->ld_tmodem_hangup_events);

	if (ret < 0)
		goto out;

	used = used + ret;

	if (used >= PAGE_SIZE - 1)
		goto out;

	ret = snprintf(buf + used,
					PAGE_SIZE - used,
					"LD Thin Modem drop events: %llu\n",
					dbg->ld_tmodem_drop_events);
	if (ret < 0)
		goto out;

	used = used + ret;

	if (used >= PAGE_SIZE - 1)
		goto out;

	ret = snprintf(buf + used,
					PAGE_SIZE - used,
					"LD Thin Modem Tx error: %llu\n",
					dbg->ld_tmodem_skb_tx_err);
	if (ret < 0)
		goto out;

	used = used + ret;

	if (used >= PAGE_SIZE - 1)
		goto out;

	ret = snprintf(buf + used,
				PAGE_SIZE - used,
				"LD Thin Modem buff len: %d\n",
				gb_ld_tmodem->ld_backlog_len);

out:
	ret = simple_read_from_buffer(user_buf, count, ppos,
			buf,
			strlen(buf));

	kfree(buf);

	return ret;

}

static const struct file_operations tmdm_dbg_fops = {
	.read	= tmdm_dbg_read,
	.write	= tmdm_dbg_write,
	.open	= simple_open,
	.llseek	= default_llseek,
};

static int ld_tmodem_dbgfs_init(struct net_device *ndev)
{
	int ret = -ENOMEM;
	struct ld_tmodem *ld_tmdm;
	struct dentry *root;
	struct dentry *node;

	ld_tmdm = netdev_priv(ndev);
	root = debugfs_create_dir("ld_tmodem", NULL);
	if (IS_ERR(root))
		return PTR_ERR(root);

	if (!root)
		goto err_root;

	node = debugfs_create_file("ld_dbg", S_IRUGO | S_IWUSR, root,
					NULL, &tmdm_dbg_fops);
	if (!node)
		goto err_node;

	ld_tmdm->root = root;
	return 0;

err_node:
	debugfs_remove_recursive(root);
err_root:
	pr_err("ld_tmodem_dbgfs_init: Failed to initialize debugfs\n");
	return ret;
}

static void
remove_nw_interface(struct net_device *ndev)
{
	struct ld_tmodem *ld_tmdm;
	ld_tmdm = netdev_priv(ndev);
	kfree(ld_tmdm->pending_buffer);
	unregister_netdev(ld_tmdm->dev);
}

static void
ld_tmodem_l2_mux_deinit(void)
{
	int err;
	err = l2mux_netif_rx_unregister(MHI_L3_CTRL_TMODEM);
	if (err) {
		pr_err("l2mux_netif_rx_unregister fails l3 id %d, error %d\n",
						MHI_L3_CTRL_TMODEM, err);
	}
	err = mhi_unregister_protocol(MHI_L3_CTRL_TMODEM);
	if (err) {
		pr_err("mhi_unregister_protocol fails l3 id %d, error %d\n",
						MHI_L3_CTRL_TMODEM, err);
	}
}

static int ld_tmodem_l2_mux_init(void)
{
	int err;
	err = l2mux_netif_rx_register(MHI_L3_CTRL_TMODEM, mhi_flow_ctrl_rx);
	if (err) {
		pr_err("l2mux_netif_rx_register fails l3 id %d, error %d\n",
						MHI_L3_CTRL_TMODEM, err);
		return err;
	}
	err = mhi_register_protocol(MHI_L3_CTRL_TMODEM);
	if (err) {
		l2mux_netif_rx_unregister(MHI_L3_CTRL_TMODEM);
		pr_err("mhi_register_protocol fails l3 id %d, error %d\n",
						MHI_L3_CTRL_TMODEM, err);
		return err;
	}
	return 0;
}

static int create_nw_interface(void)
{
	struct net_device *ndev;
	struct ld_tmodem *ld_tmdm;
	int err;
	ndev = alloc_netdev_mq(sizeof(*ld_tmdm), ld_tmodem_ifname,
						ld_tmdm_net_setup, 2);
	if (!ndev)
		return -ENOMEM;

	netdev_dbg(ndev, "ld_tmodem_ldisc_open starts\n");

	ld_tmdm = netdev_priv(ndev);
	spin_lock_init(&ld_tmdm->lock);
	netif_carrier_off(ndev);
	skb_queue_head_init(&ld_tmdm->head);
	ld_tmdm->dev = ndev;
	ld_tmdm->skb = NULL;
	ld_tmdm->len = 0;

	ld_tmdm->pending_length = 0;

	netif_tx_stop_all_queues(ndev);

	ld_tmdm->link_up = true;

	ld_tmdm->pending_buffer = kmalloc(MHI_MAX_MTU, GFP_KERNEL);
	if (!ld_tmdm->pending_buffer) {
		err = -ENOMEM;
		goto out;
	}

	err = register_netdev(ndev);
out:
	if (err) {
		kfree(ld_tmdm->pending_buffer);
		free_netdev(ndev);
	} else {
		gb_ld_tmodem = ld_tmdm;
	}
	return err;
}

static int __init tmodem_net_init(void)
{
	int retval;
	struct workqueue_struct *ld_tmodem_wq;
	struct ld_tty_wakeup_work_t *ld_tty_wakeup_work;
	retval = tty_register_ldisc(N_LDTMODEM, &ld_tmodem_ldisc);

	printk(KERN_INFO "%s called\n", __func__);

	ld_tmodem_wq = create_workqueue("ld_queue");
	if (NULL == ld_tmodem_wq) {
		pr_err("Create Workqueue failed\n");
		retval = -ENOMEM;
		goto error;
	}
	ld_tty_wakeup_work = kmalloc(sizeof(struct ld_tty_wakeup_work_t),
					GFP_KERNEL);
	if (ld_tty_wakeup_work) {
		INIT_WORK((struct work_struct *)ld_tty_wakeup_work,
			  ld_tty_wakeup_workfunction);
		ld_tty_wakeup_work->ld_work_write_wakeup_tty = NULL;
	} else {
		pr_err("TTY Wake up work Error\n");
		goto error1;
	}

	retval = ld_tmodem_l2_mux_init();
	if (retval) {
		pr_err("ld_tmodem_l2_mux_init fails\n");
		goto error1;
	}

	retval = create_nw_interface();
	if (retval) {
		pr_err("create_nw_interface fails\n");
		goto error2;
	}

	gb_ld_tmodem->ld_tmodem_wq = ld_tmodem_wq;
	gb_ld_tmodem->ld_tty_wakeup_work = ld_tty_wakeup_work;

	retval = ld_tmodem_dbgfs_init(gb_ld_tmodem->dev);
	if (retval) {
		pr_err("ld_tmodem_dbgfs_init fails\n");
		goto error3;
	}

	return retval;

error3:
	remove_nw_interface(gb_ld_tmodem->dev);

error2:
	ld_tmodem_l2_mux_deinit();
error1:
	flush_workqueue(ld_tmodem_wq);
	destroy_workqueue(ld_tmodem_wq);
	kfree(ld_tty_wakeup_work);
error:
	tty_unregister_ldisc(N_LDTMODEM);
	return retval;

}

static void __exit tmodem_net_remove(void)
{

	struct ld_tmodem *ld_tmdm = gb_ld_tmodem;
	flush_workqueue(ld_tmdm->ld_tmodem_wq);
	destroy_workqueue(ld_tmdm->ld_tmodem_wq);
	kfree(ld_tmdm->ld_tty_wakeup_work);
	debugfs_remove_recursive(ld_tmdm->root);
	remove_nw_interface(ld_tmdm->dev);
	ld_tmodem_l2_mux_deinit();
	tty_unregister_ldisc(N_LDTMODEM);
}

module_init(tmodem_net_init);
module_exit(tmodem_net_remove);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("pranav@broadcom.com");
MODULE_DESCRIPTION("TMODEM TTY line discipline");
MODULE_ALIAS_LDISC(N_LDTMODEM);
#endif /* CONFIG_BCM_KF_PHONET */
