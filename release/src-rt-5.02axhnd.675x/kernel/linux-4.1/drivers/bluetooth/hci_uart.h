/*
 *
 *  Bluetooth HCI UART driver
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2004-2005  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef N_HCI
#define N_HCI	15
#endif

/* Ioctls */
#define HCIUARTSETPROTO		_IOW('U', 200, int)
#define HCIUARTGETPROTO		_IOR('U', 201, int)
#define HCIUARTGETDEVICE	_IOR('U', 202, int)
#define HCIUARTSETFLAGS		_IOW('U', 203, int)
#define HCIUARTGETFLAGS		_IOR('U', 204, int)

/* UART protocols */
#define HCI_UART_MAX_PROTO	8

#define HCI_UART_H4	0
#define HCI_UART_BCSP	1
#define HCI_UART_3WIRE	2
#define HCI_UART_H4DS	3
#define HCI_UART_LL	4
#define HCI_UART_ATH3K	5
#define HCI_UART_INTEL	6
#define HCI_UART_BCM	7

#define HCI_UART_RAW_DEVICE	0
#define HCI_UART_RESET_ON_INIT	1
#define HCI_UART_CREATE_AMP	2
#define HCI_UART_INIT_PENDING	3
#define HCI_UART_EXT_CONFIG	4
#define HCI_UART_VND_DETECT	5

struct hci_uart;

struct hci_uart_proto {
	unsigned int id;
	const char *name;
	int (*open)(struct hci_uart *hu);
	int (*close)(struct hci_uart *hu);
	int (*flush)(struct hci_uart *hu);
	int (*setup)(struct hci_uart *hu);
	int (*recv)(struct hci_uart *hu, const void *data, int len);
	int (*enqueue)(struct hci_uart *hu, struct sk_buff *skb);
	struct sk_buff *(*dequeue)(struct hci_uart *hu);
};

struct hci_uart {
	struct tty_struct	*tty;
	struct hci_dev		*hdev;
	unsigned long		flags;
	unsigned long		hdev_flags;

	struct work_struct	init_ready;
	struct work_struct	write_work;

	const struct hci_uart_proto *proto;
	void			*priv;

	struct sk_buff		*tx_skb;
	unsigned long		tx_state;
	spinlock_t		rx_lock;
};

/* HCI_UART proto flag bits */
#define HCI_UART_PROTO_SET	0
#define HCI_UART_REGISTERED	1

/* TX states  */
#define HCI_UART_SENDING	1
#define HCI_UART_TX_WAKEUP	2

int hci_uart_register_proto(const struct hci_uart_proto *p);
int hci_uart_unregister_proto(const struct hci_uart_proto *p);
int hci_uart_tx_wakeup(struct hci_uart *hu);
int hci_uart_init_ready(struct hci_uart *hu);

#ifdef CONFIG_BT_HCIUART_H4
int h4_init(void);
int h4_deinit(void);

struct h4_recv_pkt {
	u8  type;	/* Packet type */
	u8  hlen;	/* Header length */
	u8  loff;	/* Data length offset in header */
	u8  lsize;	/* Data length field size */
	u16 maxlen;	/* Max overall packet length */
	int (*recv)(struct hci_dev *hdev, struct sk_buff *skb);
};

#define H4_RECV_ACL \
	.type = HCI_ACLDATA_PKT, \
	.hlen = HCI_ACL_HDR_SIZE, \
	.loff = 2, \
	.lsize = 2, \
	.maxlen = HCI_MAX_FRAME_SIZE \

#define H4_RECV_SCO \
	.type = HCI_SCODATA_PKT, \
	.hlen = HCI_SCO_HDR_SIZE, \
	.loff = 2, \
	.lsize = 1, \
	.maxlen = HCI_MAX_SCO_SIZE

#define H4_RECV_EVENT \
	.type = HCI_EVENT_PKT, \
	.hlen = HCI_EVENT_HDR_SIZE, \
	.loff = 1, \
	.lsize = 1, \
	.maxlen = HCI_MAX_EVENT_SIZE

struct sk_buff *h4_recv_buf(struct hci_dev *hdev, struct sk_buff *skb,
			    const unsigned char *buffer, int count,
			    const struct h4_recv_pkt *pkts, int pkts_count);
#endif

#ifdef CONFIG_BT_HCIUART_BCSP
int bcsp_init(void);
int bcsp_deinit(void);
#endif

#ifdef CONFIG_BT_HCIUART_LL
int ll_init(void);
int ll_deinit(void);
#endif

#ifdef CONFIG_BT_HCIUART_ATH3K
int ath_init(void);
int ath_deinit(void);
#endif

#ifdef CONFIG_BT_HCIUART_3WIRE
int h5_init(void);
int h5_deinit(void);
#endif

#ifdef CONFIG_BT_HCIUART_BCM
int bcm_init(void);
int bcm_deinit(void);
#endif
