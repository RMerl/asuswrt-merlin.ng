#ifdef CONFIG_BCM_KF_PHONET
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
 * File: ld_pnonet.c
 *
 * Phonet device TTY line discipline
 */

#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/tty.h>

#include <asm/unaligned.h>
#include <net/sock.h>
#include <linux/errno.h>

#include <linux/if_arp.h>
#include <linux/if_phonet.h>
#include <linux/phonet.h>
#include <net/phonet/phonet.h>
#include <net/phonet/pn_dev.h>
//#include <linux/switch.h>	/* AT-ISI Separation */	// TODO: do we need this header? it is not in 3.4.11
#include <linux/interrupt.h>
MODULE_AUTHOR("david RMC");
MODULE_DESCRIPTION("Phonet TTY line discipline");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS_LDISC(N_PHONET);

/* Comment - 01 */
/*AT+ATSTART will be entered by closing ISI application. By the methods under
 implementation for disconnecting an application in NTrace)the same is
 expected to be available for TnT), it is expected that, congestion condition
 will be present when executing AT+ATSTART allowing a few bytes of room from
 underlying layer. Hence, keeping simplicity later write_back functionality
 is not used here as it is done for normal transfer.*/

/* Comment - 02 */
/*If control is transferred to AT Parser, activateld can close the tty
 interfering tty->write. Hence, tty->write is done first. Only
 programming error can fail AT switch . practically, no other reasons apply.
 Tty->write will synchronously write to the lower driver which can later
 transfer the data in tty independent way. In testing no synchronization
 issue is seen.*/

#define SEND_QUEUE_LOW		10
#define SEND_QUEUE_HIGH		100
#define PHONET_SENDING		1	/* Bit 1 = 0x02 */
#define PHONET_FLOW_OFF_SENT	4	/* Bit 4 = 0x10 */
#define MAX_WRITE_CHUNK		8192
#define ISI_MSG_HEADER_SIZE	6
/*#define MAX_BUFF_SIZE		20000*/
#define MAX_BUFF_SIZE		65535

#define LD_PHONET_SWITCH		4
#define LD_PHONET_NEW_ISI_MSG		0
#define LD_PHONET_ISI_MSG_LEN		1
#define LD_PHONET_ISI_MSG_NO_LEN	2

#define LD_PHONET_BUFFER_LEN	1048576
#define LD_PHONET_INIT_LEN	0

#define LD_ATCMD_BUFFER_LEN	1024

#define LD_WAKEUP_DATA_INIT	0
#define ATPLIB_AT_CMD_MAX	1024

#define LD_SWITCH_ATSTART_RESP		1
#define LD_SWITCH_MODECHAN02_RESP	2

#define GUID_HEADER_BYTE1	0xdd
#define GUID_HEADER_BYTE2	0x7f
#define GUID_HEADER_BYTE3	0x21
#define GUID_HEADER_BYTE4	0x9a


struct ld_phonet {
	struct tty_struct *tty;
	wait_queue_head_t wait;
	spinlock_t lock;
	unsigned long flags;
	struct sk_buff *skb;
	unsigned long len;
	unsigned long lentorcv;
	unsigned long datarcv;
	unsigned long state;
	struct net_device *dev;
	struct list_head node;
	struct sk_buff_head head;
	char *tty_name;
	int ld_phonet_state;
	int n_data_processed;
	int n_data_sent;
	int n_remaining_data;
	bool link_up;
	int nb_try_to_tx;
	unsigned char *ld_atcmd_buffer;
};

static int ld_buff_len;		/* LD Phonet Tx Backlog buffer Len */
static struct workqueue_struct *ld_phonet_wq;

/* Work to hanlde TTY wake up */
struct ld_tty_wakeup_work_t {
	struct work_struct ld_work;
	/*This holds TTY info for TTY wakeup */
	struct tty_struct *ld_work_write_wakeup_tty;
};
static struct ld_tty_wakeup_work_t *ld_tty_wakeup_work;

/* Wotk to Handle AT+ATSTART Switch */
struct ld_uart_switch_work_t {
	struct work_struct ld_work;
	unsigned long at_modechan02_mode;
};
static struct ld_uart_switch_work_t *ld_uart_switch_work;

/* Ld phonet statistics */
static unsigned long ld_phonet_tx_request_count;
static unsigned long ld_phonet_rx_request_count;
static unsigned long ld_phonet_tx_bytes;
static unsigned long ld_phonet_rx_bytes;
static unsigned long ld_phonet_hangup_events;
static unsigned long ld_phonet_drop_events;

/* AT-ISI Separation ends */
#define LD_PHONET_DEBUG 0
#if LD_PHONET_DEBUG
#define dbg(fmt, ...) printk(fmt,  ## __VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

static int ld_pn_net_open(struct net_device *dev)
{
	netif_wake_queue(dev);
	return 0;
}

static int ld_pn_net_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static void ld_tx_overflow(void)
{
	ld_phonet_drop_events++;
	pr_crit(
	       "##### ATTENTION : LD Phonet Transmit overflow events %lu : 1 MB #####",
	       ld_phonet_drop_events);
}

static int ld_pn_handle_tx(struct ld_phonet *ld_pn)
{
	struct tty_struct *tty = ld_pn->tty;
	struct sk_buff *skb;
	int tty_wr, len, room, i;

	if (tty == NULL)
		return 0;
	/* Enter critical section */
	if (test_and_set_bit(PHONET_SENDING, &ld_pn->state))
		return 0;

	/* skb_peek is safe because handle_tx is called after skb_queue_tail */
	while ((skb = skb_peek(&ld_pn->head)) != NULL) {

		/* Make sure you don't write too much */
		len = skb->len;
		room = tty_write_room(tty);

		if (!room) {
			if (ld_buff_len > LD_PHONET_BUFFER_LEN) {
				if (ld_pn->link_up == true)
					ld_tx_overflow();
				ld_pn->link_up = false;
				/* Flush TX queue */
				while ((skb =
					skb_dequeue(&ld_pn->head)) != NULL) {
					skb->dev->stats.tx_dropped++;
					dbg("Flush TX queue tx_dropped = %d",
					    skb->dev->stats.tx_dropped);
					if (in_interrupt())
						dev_kfree_skb_irq(skb);
					else
						kfree_skb(skb);
				}
				ld_buff_len = LD_PHONET_INIT_LEN;
				goto error;
			} else {	/* FALLBACK TRIAL */
				dbg(
					"ld_pn_handle_tx no room, waiting for previous to be sent..:\n");

				if (!test_bit(TTY_DO_WRITE_WAKEUP,
					&tty->flags)) {
					/* wakeup bit is not set, set it */
					dbg(
						"ld_pn_handle_tx Setting TTY_DO_WRITE_WAKEUP bit...\n");
					set_bit(TTY_DO_WRITE_WAKEUP,
						&tty->flags);
				} else {
					dbg(
						"ld_pn_handle_tx TTY_DO_WRITE_WAKEUP bit already set!...\n");
				}
			}
			break;
		}

		/* Get room => reset nb_try_to_tx counter */
		ld_pn->nb_try_to_tx = 0;

		if (len > room)
			len = room;

		tty_wr = tty->ops->write(tty, skb->data, len);
		ld_buff_len -= tty_wr;
		if (tty_wr > 0)
			ld_phonet_tx_bytes += tty_wr;
		if (ld_buff_len < LD_PHONET_INIT_LEN)
			ld_buff_len = LD_PHONET_INIT_LEN;
		ld_pn->dev->stats.tx_packets++;
		ld_pn->dev->stats.tx_bytes += tty_wr;
		dbg(" Response start\n");
		for (i = 1; i <= len; i++) {
			dbg(" %02x", skb->data[i - 1]);
			if ((i % 8) == 0)
				dbg("\n");
		}
		dbg("\n");
		dbg(" Response stop\n");
		/* Error on TTY ?! */
		if (tty_wr < 0)
			goto error;
		/* Reduce buffer written, and discard if empty */
		skb_pull(skb, tty_wr);
		if (skb->len == 0) {
			struct sk_buff *tmp = skb_dequeue(&ld_pn->head);
			BUG_ON(tmp != skb);
			if (in_interrupt())
				dev_kfree_skb_irq(skb);
			else
				kfree_skb(skb);
		}
	}
	/* Send flow off if queue is empty */
	clear_bit(PHONET_SENDING, &ld_pn->state);
	return NETDEV_TX_OK;
error:
	clear_bit(PHONET_SENDING, &ld_pn->state);
	return NETDEV_TX_OK;
}

static int ld_pn_net_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ld_phonet *ld_pn;
	u8 *ptr;

	BUG_ON(dev == NULL);
	ld_pn = netdev_priv(dev);
	if ((ld_pn == NULL) || (ld_pn->tty == NULL)) {
		if (in_interrupt())
			dev_kfree_skb_irq(skb);
		else
			kfree_skb(skb);
		return NETDEV_TX_OK;
	}
	ld_phonet_tx_request_count++;
	ptr = skb_push(skb, 6);
	ptr[0] = GUID_HEADER_BYTE1;
	ptr[1] = GUID_HEADER_BYTE2;
	ptr[2] = GUID_HEADER_BYTE3;
	ptr[3] = GUID_HEADER_BYTE4;
	ptr[4] = skb->data[10];
	ptr[5] = skb->data[11];
	PN_PRINTK("ld_pn_net_xmit: send skb to %s", dev->name);
	if (ld_pn->link_up == true) {
		skb_queue_tail(&ld_pn->head, skb);
		ld_buff_len += skb->len;
		return ld_pn_handle_tx(ld_pn);
	} else {
		if (tty_write_room(ld_pn->tty)) {
			/* link is up again */
			ld_pn->link_up = true;
			ld_pn->nb_try_to_tx = 0;
			skb_queue_tail(&ld_pn->head, skb);
			ld_buff_len += skb->len;
			return ld_pn_handle_tx(ld_pn);
		} else {
			if (in_interrupt())
				dev_kfree_skb_irq(skb);
			else
				kfree_skb(skb);
			dev->stats.tx_dropped++;
			dbg("tx_dropped = %d", dev->stats.tx_dropped);
			return NETDEV_TX_OK;
		}
	}
}

static int ld_pn_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	switch (cmd) {
	case SIOCPNGAUTOCONF:
		phonet_route_add(dev, PN_DEV_PC);
		dev_open(dev);
		netif_carrier_on(dev);
		/* Return NOIOCTLCMD so Phonet won't do it again */
		return -ENOIOCTLCMD;
	}
	return -ENOIOCTLCMD;
}

static int ld_pn_net_mtu(struct net_device *dev, int new_mtu)
{
	if ((new_mtu < PHONET_MIN_MTU) || (new_mtu > PHONET_MAX_MTU))
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

static const struct net_device_ops ld_pn_netdev_ops = {
	.ndo_open = ld_pn_net_open,
	.ndo_stop = ld_pn_net_close,
	.ndo_start_xmit = ld_pn_net_xmit,
	.ndo_do_ioctl = ld_pn_net_ioctl,
	.ndo_change_mtu = ld_pn_net_mtu,
};

#define PN_ADDR_LEN 1
#define PN_QUEUE_LEN 5
#define PN_HARD_HEADER_LEN 1
static void ld_pn_net_setup(struct net_device *dev)
{
	dev->features = 0;
	dev->type = ARPHRD_PHONET;
	dev->flags = IFF_POINTOPOINT | IFF_NOARP;
	dev->mtu = PHONET_DEV_MTU;
	dev->hard_header_len = PN_HARD_HEADER_LEN;
	dev->dev_addr[0] = PN_MEDIA_USB;
	dev->addr_len = PN_ADDR_LEN;
	dev->tx_queue_len = PN_QUEUE_LEN;

	dev->netdev_ops = &ld_pn_netdev_ops;
	dev->destructor = free_netdev;
	dev->header_ops = &phonet_header_ops;
};

/*****************************************
*** TTY
******************************************/
#define LD_RECEIVE_ROOM 65536
static int ld_phonet_ldisc_open(struct tty_struct *tty)
{

	struct ld_phonet *ld_pn;
	struct net_device *dev;
	int err = 0;
	dbg("ld_phonet_ldisc_open starts\n");

	/* Create net device */
	dev = alloc_netdev(sizeof(*ld_pn), "upnlink%d", ld_pn_net_setup);
	if (!dev)
		return -ENOMEM;

	ld_pn = netdev_priv(dev);
	spin_lock_init(&ld_pn->lock);
	netif_carrier_off(dev);
	skb_queue_head_init(&ld_pn->head);
	ld_pn->tty = tty;
	tty->disc_data = ld_pn;
	tty->receive_room = LD_RECEIVE_ROOM;
	ld_pn->dev = dev;
	ld_pn->skb = NULL;
	ld_pn->len = 0;
	ld_pn->lentorcv = 0;
	ld_pn->datarcv = 0;
	ld_pn->ld_phonet_state = LD_PHONET_NEW_ISI_MSG;
	ld_pn->n_data_processed = 0;
	ld_pn->n_data_sent = 0;
	ld_pn->n_remaining_data = 0;
	ld_pn->link_up = true;
	ld_pn->nb_try_to_tx = 0;
	ld_pn->ld_atcmd_buffer = kmalloc(LD_ATCMD_BUFFER_LEN, GFP_KERNEL);
	if (ld_pn->ld_atcmd_buffer == NULL)
		goto LDISC_ERROR;
	err = register_netdev(dev);
	if (err)
LDISC_ERROR:
		free_netdev(dev);
	else
		ld_tty_wakeup_work->ld_work_write_wakeup_tty = tty;

	dbg("ld_phonet_ldisc_open exits err = %d\n", err);
	return err;

}

static void ld_phonet_ldisc_close(struct tty_struct *tty)
{
	struct ld_phonet *ld_pn = tty->disc_data;

	tty->disc_data = NULL;
	kfree(ld_pn->ld_atcmd_buffer);
	ld_pn->tty = NULL;
	ld_tty_wakeup_work->ld_work_write_wakeup_tty = NULL;
	unregister_netdev(ld_pn->dev);
}

static void ld_phonet_ldisc_init_transfer
	(struct ld_phonet *ld_pn, const unsigned char *cp, int count)
{
	struct sk_buff *skb = NULL;
	unsigned int msglen = 0;

	struct phonethdr *ph = NULL;
	int i;

	dbg("ld_phonet: initiate transfer Data Sent = %d ", ld_pn->n_data_sent);
	dbg("Data Processed = %d ", ld_pn->n_data_processed);
	dbg("Data Remaining = %d\n", ld_pn->n_remaining_data);

	/* Check if there is still data in cp */
	while (ld_pn->n_data_processed < count) {
		/* Check if extract length is possible */
		if ((count - ld_pn->n_data_processed) > ISI_MSG_HEADER_SIZE) {
			/* Extract length */
			/* Move 1 byte since media parameter is not there in */
			/* phonethdr structure */
			ph = (struct phonethdr *)
			    (cp + ld_pn->n_data_processed + sizeof(char));
			msglen = get_unaligned_be16(&ph->pn_length);
			ld_pn->len = msglen + ISI_MSG_HEADER_SIZE;

			if (ld_pn->len == ISI_MSG_HEADER_SIZE) {
				printk(
					"ld_phonet: Extracted ISI msg len = ISI_MSG_HEADER_SIZE, dumping rest of buffer");
				goto out;
			}

			/* Alloc SKBuff */
			skb = netdev_alloc_skb(ld_pn->dev, ld_pn->len);
			if (NULL == skb) {
				/* TBD handle error */
				return;
			}

			skb->dev = ld_pn->dev;
			skb->protocol = htons(ETH_P_PHONET);
			skb_reset_mac_header(skb);
			ld_pn->skb = skb;

			/* check if we receive complete data in this */
			/* usb frame */
			if (ld_pn->len <= (count - ld_pn->n_data_processed)) {
				/* We received complete data in this usb */
				/* frame */
				/* copy the ISI buffer */
				memcpy(skb_put(skb, ld_pn->len),
				       cp + ld_pn->n_data_processed,
				       ld_pn->len);
				ld_pn->n_data_processed += ld_pn->len;

				/* Send to Phonet */
				ld_pn->dev->stats.rx_packets++;
				ld_pn->dev->stats.rx_bytes += skb->len;
				__skb_pull(skb, 1);
				dbg("Request buffer start\n");
				for (i = 1; i <= skb->len; i++) {
					dbg("%02x", skb->data[i - 1]);
					if (i % 8 == 0)
						dbg("\n");
				}

				dbg("Request buffer end\n");
				dbg(
					"calling netif_rx inside initiate_transfer ld_pn->len=%d\n",
					ld_pn->len);
				ld_pn->n_data_sent += ld_pn->len;
				ld_phonet_rx_bytes += skb->len;
				netif_rx(skb);

				/* TBD : Reset pointers */
				ld_pn->len = LD_PHONET_INIT_LEN;
			} else {
				/* We receive only partial ISI message */
				/* Copy the partial ISI message */
				memcpy(skb_put(skb, count -
					       ld_pn->n_data_processed), cp +
				       ld_pn->n_data_processed, count -
				       ld_pn->n_data_processed);
				ld_pn->ld_phonet_state = LD_PHONET_ISI_MSG_LEN;
				ld_pn->n_remaining_data = ld_pn->len -
				    (count - ld_pn->n_data_processed);
				ld_pn->n_data_processed += count -
				    ld_pn->n_data_processed;

				return;
			}
		} else {
			/* Not able to extract length since received */
			/* usb frame length is */
			/* less than ISI message header size */

			/* Alloc SKBuff with max size */
			skb = netdev_alloc_skb(ld_pn->dev, MAX_BUFF_SIZE);
			if (NULL == skb) {
				/* TBD handle error */
				return;
			}

			skb->dev = ld_pn->dev;
			skb->protocol = htons(ETH_P_PHONET);
			skb_reset_mac_header(skb);
			ld_pn->skb = skb;

			/* Copy available data */
			memcpy(skb_put(skb, count - ld_pn->n_data_processed),
			       cp + ld_pn->n_data_processed, count -
			       ld_pn->n_data_processed);
			ld_pn->ld_phonet_state = LD_PHONET_ISI_MSG_NO_LEN;

			ld_pn->len += count - ld_pn->n_data_processed;
			ld_pn->n_data_processed +=
			    count - ld_pn->n_data_processed;

			return;
		}
	}

out:
	/* No more data in cp */
	ld_pn->ld_phonet_state = LD_PHONET_NEW_ISI_MSG;
	ld_pn->len = 0;
	ld_pn->n_data_processed = 0;
	ld_pn->n_data_sent = 0;
	ld_pn->n_remaining_data = 0;

	return;
}

/* AT-ISI Message Separation Starts */

ssize_t ld_set_manualsw(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count);

int stop_isi;

/* AT-ISI Message Separation Ends */
static void ld_phonet_ldisc_receive
	(struct tty_struct *tty, const unsigned char *cp, char *fp, int count)
{
	struct ld_phonet *ld_pn = tty->disc_data;
	struct sk_buff *skb = ld_pn->skb;
	unsigned long flags = 0;
	unsigned int msglen = 0, i;
	int check_at = 27;
	struct phonethdr *ph = NULL;

	ld_phonet_rx_request_count++;
	if (ld_pn->link_up == false) {
		/* data received from PC => can TX */
		ld_pn->link_up = true;

		ld_pn->nb_try_to_tx = 0;
	}
	PN_PRINTK("ld_phonet_ldisc_receive: receive  %d data", count);
	for (i = 1; i <= count; i++) {
		PN_DATA_PRINTK(" %02x", cp[i - 1]);
		if ((i % 8) == 0)
			PN_DATA_PRINTK("\n");
	}

	spin_lock_irqsave(&ld_pn->lock, flags);

	/* Whenever you receive a new USB frame Data Processed should be reset */
	ld_pn->n_data_processed = 0;

	while (1) {
		switch (ld_pn->ld_phonet_state) {
		case LD_PHONET_NEW_ISI_MSG:
		{
			int first_byte = 0;
			if (count >= 1) {
				if (*cp) {
					first_byte = *cp;
					dbg(
						"case LD_PHONET_NEW_ISI_MSG: %d\n",
						*cp);
				}
			} else
				dbg("case LD_PHONET_NEW_ISI_MSG\n");

			if ((count >= 1) && (first_byte != check_at)) {
				dbg("MATCH FOR change mode %c\n", *cp);
				ld_pn->ld_phonet_state =
				    LD_PHONET_SWITCH;
				continue;
			}

			/* AT-ISI Message Separation Ends */
			ld_phonet_ldisc_init_transfer(ld_pn, cp,
							  count);
			break;
		}
		case LD_PHONET_ISI_MSG_LEN:
		/* check if Remaining Data is complete */
		if (ld_pn->n_remaining_data > count) {
			/* We dont receive complete data */
			/* Copy the available data */
			memcpy(skb_put(skb, count), cp +
			       ld_pn->n_data_processed, count);
			ld_pn->n_data_processed += count;
			ld_pn->ld_phonet_state = LD_PHONET_ISI_MSG_LEN;
			ld_pn->n_remaining_data -= count;
		} else {
			/* We have complete data available */
			/* Copy remaining data */
			memcpy(skb_put(skb, ld_pn->n_remaining_data),
			       cp + ld_pn->n_data_processed,
			       ld_pn->n_remaining_data);
			/* Send to Phonet */
			ld_pn->dev->stats.rx_packets++;
			ld_pn->dev->stats.rx_bytes += skb->len;
			__skb_pull(skb, sizeof(char));
			dbg("Request buffer start\n");
			for (i = 1; i <= skb->len; i++) {
				dbg("%02x", skb->data[i - 1]);
				if (i % 8 == 0)
					dbg("\n");
			}
			dbg("Request buffer end\n");
			dbg(
				"calling netif_rx inside ldisc_receive first ld_pn->len=%d\n",
				ld_pn->len);
			ld_pn->n_data_sent += ld_pn->len;
			ld_phonet_rx_bytes += skb->len;
			netif_rx(skb);

			/* TBD : Update pointers */
			ld_pn->n_data_sent += ld_pn->n_remaining_data;
			ld_pn->n_data_processed +=
			    ld_pn->n_remaining_data;
			ld_pn->len = LD_PHONET_INIT_LEN;

			/* Initiate a new ISI transfer */
			ld_phonet_ldisc_init_transfer
			    (ld_pn, cp, count);
		}
		break;

		case LD_PHONET_ISI_MSG_NO_LEN:
		/*Check if we can extact length */
		if ((ld_pn->len + count) >= ISI_MSG_HEADER_SIZE) {

			/* Copy remaining header to SKBuff to extract */
			/* length */
			memcpy(skb_put(skb, ISI_MSG_HEADER_SIZE -
				       ld_pn->len),
			       cp + ld_pn->n_data_processed,
			       ISI_MSG_HEADER_SIZE - ld_pn->len);
			ph = (struct phonethdr *)
			    (skb->data + sizeof(char));
			msglen = get_unaligned_be16(&ph->pn_length);

			ld_pn->n_data_processed +=
			    ISI_MSG_HEADER_SIZE - ld_pn->len;

			/* Check if we receive complete data */
			if ((count + ld_pn->len) <
			    (msglen + ISI_MSG_HEADER_SIZE)) {
				/* We have not received complete data */
				/* Copy available data */
				memcpy(skb_put(skb, count -
					       (ISI_MSG_HEADER_SIZE -
						ld_pn->len)),
				       cp + ld_pn->n_data_processed,
				       count - (ISI_MSG_HEADER_SIZE -
						ld_pn->len));
				ld_pn->ld_phonet_state =
				    LD_PHONET_ISI_MSG_LEN;
				ld_pn->n_remaining_data =
				    (msglen + ISI_MSG_HEADER_SIZE) -
				    (count + ld_pn->len);
				ld_pn->n_data_processed +=
				    count - (ISI_MSG_HEADER_SIZE -
					     ld_pn->len);

				/* Reset pointers */
				ld_pn->len = msglen +
				    ISI_MSG_HEADER_SIZE;

				/* return; */
				break;
			} else {
				/* We receive complete data */
				/* Copy remaining data */
				memcpy(skb_put(skb,
					(msglen + ISI_MSG_HEADER_SIZE)
					- (ld_pn->len +
					ld_pn->n_data_processed)),
				       cp + ld_pn->n_data_processed,
				       (msglen + ISI_MSG_HEADER_SIZE) -
				       (ld_pn->len +
					ld_pn->n_data_processed));

				/* Send to Phonet */
				ld_pn->dev->stats.rx_packets++;
				ld_pn->dev->stats.rx_bytes += skb->len;
				__skb_pull(skb, sizeof(char));
				dbg("Request buffer start\n");
				for (i = 1; i <= skb->len; i++) {
					dbg("%02x", skb->data[i - 1]);
					if (i % 8 == 0)
						dbg("\n");
				}

				dbg("Request buffer end\n");
				dbg(
					"calling netif_rx inside ldisc_receive second ld_pn->len= %d\n",
					ld_pn->len);
				ld_phonet_rx_bytes += skb->len;
				netif_rx(skb);

				ld_pn->n_data_sent += (msglen +
					ISI_MSG_HEADER_SIZE)
					- (ld_pn->len +
					ld_pn->n_data_processed);

				ld_pn->n_data_processed += (msglen +
					ISI_MSG_HEADER_SIZE)
					- (ld_pn->len +
					ld_pn->n_data_processed);

				/* Reset len as skb buffer */
				/* is sent to phonet */
				ld_pn->len = LD_PHONET_INIT_LEN;

				/* Check if we still have data in cp */
				if (count > ld_pn->n_data_processed) {
					/* We still have data in cp */
					/* Initiate new ISI transfer */
					ld_phonet_ldisc_init_transfer(
					    ld_pn, cp, count);
				} else {
					/* No more data in cp */
					ld_pn->ld_phonet_state =
					    LD_PHONET_NEW_ISI_MSG;

					/* Reset pointers */
					ld_pn->len = 0;
					ld_pn->n_data_processed = 0;
					ld_pn->n_data_sent = 0;
					ld_pn->n_remaining_data = 0;
				}
			}
		} else {
			/* Cannot extract length */
			/* Copy available data */
			memcpy(skb_put(skb, count), cp +
			       ld_pn->n_data_processed, count);
			ld_pn->len += count;
			ld_pn->ld_phonet_state =
			    LD_PHONET_ISI_MSG_NO_LEN;
			ld_pn->n_data_processed += count;
		}
		break;

		default:
			break;
		}
		break;
	}

	spin_unlock_irqrestore(&ld_pn->lock, flags);
}

#define AT_START_LEN 10
#define AT_MODECHAN_LEN 15
static void ld_uart_switch_function(struct work_struct *work)
{
	struct ld_uart_switch_work_t *at_mode_work;
	at_mode_work = (struct ld_uart_switch_work_t *)work;
	set_current_state(TASK_INTERRUPTIBLE);
	ld_uart_switch_work->at_modechan02_mode = 0;
	return;
}

static void ld_tty_wakeup_workfunction(struct work_struct *work)
{
	struct tty_struct *tty;
	struct ld_phonet *ld_pn;
	struct ld_tty_wakeup_work_t *ld_work_tty_wk =
	    (struct ld_tty_wakeup_work_t *)work;

	if (ld_work_tty_wk == NULL) {
		dbg("TTY work NULL\n");
		return;
	}

	tty = ld_work_tty_wk->ld_work_write_wakeup_tty;
	if (tty == NULL) {
		dbg("LD Work Queue tty Data NULL\n");
		return;
	}

	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	ld_pn = tty->disc_data;
	if (ld_pn == NULL) {
		dbg("LD PN Work Queue DATA NULL\n");
		return;
	}

	BUG_ON(ld_pn->tty != tty);
	ld_pn_handle_tx(ld_pn);
	return;
}

static void ld_phonet_ldisc_write_wakeup(struct tty_struct *tty)
{
	ld_tty_wakeup_work->ld_work_write_wakeup_tty = tty;
	queue_work(ld_phonet_wq, (struct work_struct *)ld_tty_wakeup_work);
}

int ld_phonet_hangup_wait(void *data)
{
	return NETDEV_TX_OK;
}

static int ld_phonet_ldisc_hangup(struct tty_struct *tty)
{
	struct ld_phonet *ld_pn;
	struct sk_buff *skb;

	/* Flush TX queue */
	ld_pn = tty->disc_data;
	ld_phonet_hangup_events++;
	wait_on_bit_lock(&ld_pn->state, PHONET_SENDING,
			 ld_phonet_hangup_wait, TASK_KILLABLE);

	while ((skb = skb_dequeue(&ld_pn->head)) != NULL) {
		skb->dev->stats.tx_dropped++;
		if (in_interrupt())
			dev_kfree_skb_irq(skb);
		else
			kfree_skb(skb);
	}
	ld_buff_len = LD_PHONET_INIT_LEN;
	clear_bit(PHONET_SENDING, &ld_pn->state);
	return NETDEV_TX_OK;
}

static struct tty_ldisc_ops ld_phonet_ldisc = {
	.owner = THIS_MODULE,
	.name = "phonet",
	.open = ld_phonet_ldisc_open,
	.close = ld_phonet_ldisc_close,
	.receive_buf = ld_phonet_ldisc_receive,
	.write_wakeup = ld_phonet_ldisc_write_wakeup,
	.hangup = ld_phonet_ldisc_hangup
};

static ssize_t ld_phonet_show_stats(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	pr_crit("LD Phonet Tx request %lu\n",
	       ld_phonet_tx_request_count);
	pr_crit("LD Phonet Rx request %lu\n",
	       ld_phonet_rx_request_count);
	pr_crit("LD Phonet Tx Bytes %lu\n", ld_phonet_tx_bytes);
	pr_crit("LD Phonet Rx Bytes %lu\n", ld_phonet_rx_bytes);
	pr_crit("LD Phonet TTY hangup events %lu\n",
	       ld_phonet_hangup_events);
	pr_crit("LD Phonet Tx overflow events %lu\n",
	       ld_phonet_drop_events);
	pr_crit("LD Phonet TX buffer len %d\n", ld_buff_len);
	return 0;
}

static ssize_t ld_phonet_reset_stats(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	ld_phonet_tx_request_count = ld_phonet_rx_request_count
	    = ld_phonet_tx_bytes
	    = ld_phonet_rx_bytes = ld_phonet_hangup_events = 0;
	return 0;
}

static DEVICE_ATTR(ld_phonet_stats, S_IRUGO | S_IWUSR,
		   ld_phonet_show_stats, ld_phonet_reset_stats);

static struct attribute *ld_phonet_attributes[] = {
	&dev_attr_ld_phonet_stats.attr,
	NULL
};

static struct kobject *ld_phonet_kobj;
#define LD_PHONET_FS "ld_phonet_isi"

static const struct attribute_group ld_phonet_group = {
	.attrs = ld_phonet_attributes,
};

static int ld_phonet_sysfs_init(void)
{
	int ret = 1;
	ld_phonet_kobj = kobject_create_and_add(LD_PHONET_FS, kernel_kobj);
	if (!ld_phonet_kobj) {
		pr_err("LD Sysfs Kojb failed");
		return -ENOMEM;
	}
	ret = sysfs_create_group(ld_phonet_kobj, &ld_phonet_group);
	if (ret)
		kobject_put(kernel_kobj);
	return ret;
}

static int __init ld_phonet_init(void)
{
	int retval;
	retval = tty_register_ldisc(N_PHONET, &ld_phonet_ldisc);
	ld_buff_len = LD_PHONET_INIT_LEN;

	ld_phonet_wq = create_workqueue("ld_queue");
	if (NULL == ld_phonet_wq) {
		pr_err("Create Workqueue failed\n");
		tty_unregister_ldisc(N_PHONET);
		return -ENOMEM;
	}

	/* Work for handling TTY wakr up */
	ld_tty_wakeup_work = kmalloc(sizeof(struct ld_tty_wakeup_work_t),
				     GFP_KERNEL);
	if (ld_tty_wakeup_work) {
		INIT_WORK((struct work_struct *)ld_tty_wakeup_work,
			  ld_tty_wakeup_workfunction);
		ld_tty_wakeup_work->ld_work_write_wakeup_tty = NULL;
	} else {
		pr_err("TTY Wake up work Error\n");
		tty_unregister_ldisc(N_PHONET);
		return false;
	}
	/* Work for handling AT+ATSTART switch */
	ld_uart_switch_work = kmalloc(sizeof(struct ld_uart_switch_work_t),
				      GFP_KERNEL);
	if (ld_uart_switch_work) {
		INIT_WORK((struct work_struct *)ld_uart_switch_work,
			  ld_uart_switch_function);
	} else {
		pr_crit("UART Switch Work Failed");
	}

	retval = ld_phonet_sysfs_init();
	return retval;
}

static void __exit ld_phonet_exit(void)
{
	flush_workqueue(ld_phonet_wq);
	destroy_workqueue(ld_phonet_wq);
	kfree(ld_tty_wakeup_work);
	kfree(ld_uart_switch_work);
	tty_unregister_ldisc(N_PHONET);
	sysfs_remove_group(ld_phonet_kobj, &ld_phonet_group);
	kobject_put(ld_phonet_kobj);
}

module_init(ld_phonet_init);
module_exit(ld_phonet_exit);
#endif /* CONFIG_BCM_KF_PHONET */
