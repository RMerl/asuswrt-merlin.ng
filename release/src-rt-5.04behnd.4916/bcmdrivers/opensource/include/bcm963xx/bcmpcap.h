/*
 * <:copyright-BRCM:2022:DUAL/GPL:standard
 * 
 *    Copyright (c) 2022 Broadcom 
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
