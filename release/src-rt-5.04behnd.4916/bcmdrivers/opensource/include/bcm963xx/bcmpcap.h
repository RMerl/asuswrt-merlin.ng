/*
 * <:copyright-BRCM:2022:DUAL/GPL:standard
 * 
 *    Copyright (c) 2022 Broadcom 
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
#ifndef _BCMPCAP_H
#define _BCMPCAP_H

/* --- types and constants --- */
#define CAP_SUPPORT_PCAP		0
#define CAP_SUPPORT_SLL			1
#define CAP_SUPPORT_SLL2		0
#define CAP_SUPPORT_PCAPNG		1

#define CAP_BUF_BASE_SIZE		(1000 * 4096) /* 4MB */

struct cap_intf {
	int			index;
	struct net_device	*dev;
	struct list_head	node;
};

struct cap_pkt_state {
	struct cap_buf		*cb;
	int			hook;
	struct sk_buff		*skb;
	struct nf_conn		*ct;
	int			(*dump_cb)(struct cap_pkt_state *s);
	void			*data;
	struct cap_intf		*intf;
};

struct cap_buf {
	u8			*buf;
	int			size;
	int			in, out;
	int			avail;
	struct cap_ops		*ops;
	unsigned long		total_pkts;
	unsigned long long	total_bytes;
	struct list_head	intf_list;
	char			tmp[1024];
	wait_queue_head_t	wait;
	struct dentry		*dentry;
	spinlock_t		lock;
	struct list_head	node;
};

/* --- functions --- */
struct dentry *pcap_register(const char *name);
void pcap_unregister(struct dentry *de);
void pcap_packet(struct dentry *de, struct sk_buff *skb, int hook, void *data,
		 int (*dump_cb)(struct cap_pkt_state *s));
int pcap_write_str(struct cap_buf *cb, char *fmt, ...);

#endif /* _BCMPCAP_H */
