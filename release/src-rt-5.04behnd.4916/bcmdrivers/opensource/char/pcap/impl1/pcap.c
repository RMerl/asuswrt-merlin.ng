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
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/uaccess.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/xfrm.h>
#include <bcmpcap.h>

/* --- types and constants --- */
#define PCAP_MAGIC			0xA1B23C4D /* use nsec */
#define PCAP_MAJOR			2
#define PCAP_MINOR			4
#define PCAP_SNAPLEN			65535

#define PCAPNG_MAJOR			1
#define PCAPNG_MINOR			0
#define PCAPNG_BYTEORDER_MAGIC		0x1A2B3C4D
#define PCAPNG_TSRESOL			9 /* 10^9, ie nanoseconds */

#define LINUX_SLL_HDRLEN		16
#define LINUX_SLL2_HDRLEN		20

#define write_str			pcap_write_str

enum link_type {
	LINKTYPE_NULL		= 0,
	LINKTYPE_ETHERNET	= 1,
	LINKTYPE_PPP		= 9,
	LINKTYPE_PPP_ETHER	= 51,
	LINKTYPE_LOOP		= 108,
	LINKTYPE_LINUX_SLL	= 113,
	LINKTYPE_LINUX_SLL2	= 276,
};

enum pcap_packet_type {
	PACKET_TYPE_LOCAL_IN	= 0,
	PACKET_TYPE_BROADCAST	= 1,
	PACKET_TYPE_MULTICAST	= 2,
	PACKET_TYPE_FORWARD	= 3,
	PACKET_TYPE_LOCAL_OUT	= 4,
};

enum pcapng_block_type {
	PCAPNG_SECTION_HEADER_BLOCK		= 0x0A0D0D0A,
	PCAPNG_INTERFACE_DESCRIPTION_BLOCK	= 0x1,
	PCAPNG_ENHANCED_PACKET_BLOCK		= 0x6,
};

struct pcapng_block {
	u32 type;
	u32 offset;
};

enum pcapng_opt_type {
	OPT_ENDOFOPT			= 0,
	OPT_COMMENT			= 1,

	OPT_IF_NAME			= 2,
	OPT_IF_DESCRIPTION		= 3,
	OPT_IF_IPV4ADDR			= 4,
	OPT_IF_IPV6ADDR			= 5,
	OPT_IF_MACADDR			= 6,
	OPT_IF_EUIADDR			= 7,
	OPT_IF_SPEED			= 8,
	OPT_IF_TSRESOL			= 9,
	OPT_IF_TZONE			= 10,
	OPT_IF_FILTER			= 11,
	OPT_IF_OS			= 12,
	OPT_IF_FCSLEN			= 13,
	OPT_IF_TSOFFSET			= 14,
	OPT_IF_HARDWARE			= 15,
	OPT_IF_TXSPEED			= 16,
	OPT_IF_RXSPEED			= 17,

	OPT_EPB_FLAGS			= 2,
	OPT_HASH			= 3,
	OPT_DROPCOUNT			= 4,
	OPT_EPB_PACKETID		= 5,
	OPT_EPB_QUEUE			= 6,
	OPT_EPB_VERDICT			= 7,

	OPT_CUSTOM_STRING		= 2988,
	OPT_CUSTOM_BINARY		= 2989,
	OPT_CUSTOM_STRING_NOCOPY	= 19372,
	OPT_CUSTOM_BINARY_NOCOPY	= 19373,
};

enum pcapng_epb_flag {
	EPB_FLAG_INBOUND	= 0x00000001,
	EPB_FLAG_OUTBOUND	= 0x00000002,
	EPB_FLAG_UNICAST	= 0x00000004,
	EPB_FLAG_MULTICAST	= 0x00000010,
	EPB_FLAG_BROADCAST	= 0x00000014,
	EPB_FLAG_PROMISCUOUS	= 0x00000020,
};

struct cap_ops {
	int (*file_header)(struct cap_buf *cb);
	int (*packet)(struct cap_pkt_state *s);
	u32 snaplen;
	u32 linktype;
};

/* --- variables --- */
static LIST_HEAD(cap_list);
static DEFINE_SPINLOCK(cap_list_lock);
static struct dentry *debug_pcap_dir;
static const char *hook_names[] = {
	[NF_INET_PRE_ROUTING] = "PREROUTE",
	[NF_INET_LOCAL_IN] = "IN",
	[NF_INET_FORWARD] = "FORWARD",
	[NF_INET_LOCAL_OUT] = "OUT",
	[NF_INET_POST_ROUTING] = "POSTROUTE",
};

/* --- functions --- */
static inline u32 trunc_ptr(void *ptr)
{
	return (u32)((uintptr_t)ptr & 0xFFFFFFFF);
}

static inline int normalize_offset(struct cap_buf *cb, int offset)
{
	if (unlikely(!cb->size)) {
		pr_err("no cap_buf size!\n");
		return 0;
	}

	while (offset < 0)
		offset += cb->size;
	offset = offset % cb->size;

	return offset;
}

static int __write(struct cap_buf *cb, u8 *data, int len)
{
	int ret = 0;

	if (len >= (cb->size - cb->avail)) {
		pr_warn_ratelimited("pcap: userspace not keeping up\n");
		ret = -ENOMEM;
		goto out;
	}
	cb->avail += len;
	cb->total_bytes += len;

	if (cb->size - cb->in >= len) {
		memcpy(&cb->buf[cb->in], data, len);
		cb->in += len;
	} else {
		memcpy(&cb->buf[cb->in], data, cb->size - cb->in);
		len -= cb->size - cb->in;
		memcpy(&cb->buf[0], &data[len], len);
		cb->in = len;
	}

	cb->in = cb->in % cb->size;

out:
	return ret;
}

static inline int write_size(struct cap_buf *cb, void *data, int len)
{
	return __write(cb, data, len);
}

int pcap_write_str(struct cap_buf *cb, char *fmt, ...)
{
	va_list args;
	int ret = 0;

	va_start(args, fmt);

	ret = vsprintf(cb->tmp, fmt, args);
	if (ret < 0)
		goto out;
	ret = write_size(cb, cb->tmp, ret);

out:
	va_end(args);

	return ret;
}
EXPORT_SYMBOL(pcap_write_str);

static int __replace(struct cap_buf *cb, int offset, u8 *data, int len)
{
	offset = normalize_offset(cb, offset);

	if (cb->size - offset >= len) {
		memcpy(&cb->buf[offset], data, len);
	} else {
		memcpy(&cb->buf[offset], data, cb->size - offset);
		memcpy(&cb->buf[0], &data[cb->size - offset],
		       len - (cb->size - offset));
	}

	return 0;
}

static inline int
replace_size(struct cap_buf *cb, int offset, void *data, int len)
{
	return __replace(cb, offset, data, len);
}

#define DEFINE_TYPE_FNS(type) \
	static int __maybe_unused write_##type(struct cap_buf *cb, type data) \
	{ \
		return write_size(cb, &data, sizeof(data)); \
	} \
	static int __maybe_unused replace_##type(struct cap_buf *cb, \
						 int offset, type data) \
	{ \
		return replace_size(cb, offset, &data, sizeof(data)); \
	}
DEFINE_TYPE_FNS(u64);
DEFINE_TYPE_FNS(u32);
DEFINE_TYPE_FNS(u16);
DEFINE_TYPE_FNS(u8);

static inline int pad(struct cap_buf *cb, int pad_size)
{
	int len = 0;

	while (cb->in % pad_size) {
		write_u8(cb, 0);
		len++;
	}

	return len;
}

static inline int pad_to_octet(struct cap_buf *cb)
{
	return pad(cb, 4);
}

static struct cap_intf *cap_find_intf(struct cap_buf *cb, struct sk_buff *skb)
{
	struct cap_intf *intf;

	list_for_each_entry(intf, &cb->intf_list, node) {
		if (intf->dev == skb->dev)
			return intf;
	}

	return NULL;
}

#if CAP_SUPPORT_PCAP
static int get_packet_type(struct cap_pkt_state *s)
{
	struct ethhdr *ethhdr = eth_hdr(s->skb);

	if (s->hook == NF_INET_LOCAL_IN)
		return PACKET_TYPE_LOCAL_IN;
	else if (s->hook == NF_INET_LOCAL_OUT)
		return PACKET_TYPE_LOCAL_OUT;
	else if (is_multicast_ether_addr(ethhdr->h_dest))
		return PACKET_TYPE_MULTICAST;
	else if (is_broadcast_ether_addr(ethhdr->h_dest))
		return PACKET_TYPE_BROADCAST;
	else
		return PACKET_TYPE_FORWARD;
}

static int generic_packet_data(struct cap_pkt_state *s)
{
	struct ethhdr *ethhdr = eth_hdr(s->skb);

	return write_size(s->cb, &ethhdr[1], s->skb->len);
}

/* --- PCAP definitions --- */
static int pcap_file_header(struct cap_buf *cb)
{
	int ret = 0;

	/*                          1                   2                   3
	 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  0 |                          Magic Number                         |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  4 |          Major Version        |         Minor Version         |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  8 |                           Reserved1                           |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 12 |                           Reserved2                           |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 16 |                            SnapLen                            |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 20 | FCS |f|                   LinkType                            |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 */
	ret |= write_u32(cb, PCAP_MAGIC);
	ret |= write_u32(cb, (PCAP_MAJOR << 16) | PCAP_MINOR);
	ret |= write_u32(cb, 0);
	ret |= write_u32(cb, 0);
	ret |= write_u32(cb, cb->ops->snaplen);
	ret |= write_u32(cb, cb->ops->linktype);

	return ret;
}

static int __pcap_header(struct cap_pkt_state *s, int mac_hdr_len)
{
	struct timespec ts;
	int ret = 0;

	if (s->skb->tstamp)
		skb_get_timestampns(s->skb, &ts);
	else
		ktime_get_ts(&ts);

	/*
	 * PCAP packet record
	 *
	 *                        1                   2                   3
	 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  0 |                      Timestamp (Seconds)                      |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  4 |            Timestamp (Microseconds or nanoseconds)            |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  8 |                    Captured Packet Length                     |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 12 |                    Original Packet Length                     |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 16 /                                                               /
	 *    /                          Packet Data                          /
	 *    /                        variable length                        /
	 *    /                                                               /
	 *    +---------------------------------------------------------------+
	 */
	ret |= write_u32(s->cb, (u32)ts.tv_sec);
	ret |= write_u32(s->cb, (u32)ts.tv_nsec);
	ret |= write_u32(s->cb, mac_hdr_len + s->skb->len);
	ret |= write_u32(s->cb, mac_hdr_len + s->skb->len);

	return ret;
}

#if CAP_SUPPORT_SSL
static int linux_sll_hdr(struct cap_pkt_state *s)
{
	struct cap_buf *cb = s->cb;
	struct sk_buff *skb = s->skb;
	struct ethhdr *ethhdr = eth_hdr(skb);
	u16 packet_type = get_packet_type(s);
	int ret = 0;

	/*
	 * +---------------------------+
	 * |         Packet type       |
	 * |         (2 Octets)        |
	 * +---------------------------+
	 * |        ARPHRD_ type       |
	 * |         (2 Octets)        |
	 * +---------------------------+
	 * | Link-layer address length |
	 * |         (2 Octets)        |
	 * +---------------------------+
	 * |    Link-layer address     |
	 * |         (8 Octets)        |
	 * +---------------------------+
	 * |        Protocol type      |
	 * |         (2 Octets)        |
	 * +---------------------------+
	 */
	ret |= write_u16(cb, htons(packet_type));
	ret |= write_u16(cb, htons(skb->dev ? skb->dev->type : ARPHRD_ETHER));
	ret |= write_u16(cb, htons(ETH_ALEN));
	ret |= write_size(cb, ethhdr->h_source, ETH_ALEN);
	ret |= write_u16(cb, 0); /* padding (8 octets - ETH_ALEN == 2) */
	ret |= write_u16(cb, ethhdr->h_proto);

	return ret;
}

static int pcap_sll_packet(struct cap_pkt_state *s)
{
	int ret = 0;

	spin_lock_bh(&s->cb->lock);
	ret |= __pcap_header(s, LINUX_SLL_HDRLEN);
	ret |= linux_sll_hdr(s);
	ret |= generic_packet_data(s);
	spin_unlock_bh(&s->cb->lock);

	return ret;
}

static struct cap_ops pcap_sll_ops = {
	.file_header	= pcap_file_header,
	.packet		= pcap_sll_packet,
	.snaplen	= PCAP_SNAPLEN,
	.linktype	= LINKTYPE_LINUX_SLL,
};
#endif /* CAP_SUPPORT_SLL */

#if CAP_SUPPORT_SLL2
static int linux_sll2_hdr(struct cap_pkt_state *s)
{
	struct cap_buf *cb = s->cb;
	struct sk_buff *skb = s->skb;
	struct ethhdr *ethhdr = eth_hdr(skb);
	u16 packet_type = get_packet_type(s);
	int ret = 0;

	/*
	 * +---------------------------+
	 * |        Protocol type      |
	 * |         (2 Octets)        |
	 * +---------------------------+
	 * |       Reserved (MBZ)      |
	 * |         (2 Octets)        |
	 * +---------------------------+
	 * |       Interface index     |
	 * |         (4 Octets)        |
	 * +---------------------------+
	 * |        ARPHRD_ type       |
	 * |         (2 Octets)        |
	 * +---------------------------+
	 * |         Packet type       |
	 * |         (1 Octet)         |
	 * +---------------------------+
	 * | Link-layer address length |
	 * |         (1 Octets)        |
	 * +---------------------------+
	 * |    Link-layer address     |
	 * |         (8 Octets)        |
	 * +---------------------------+
	 */
	ret |= write_u16(cb, ethhdr->h_proto);
	ret |= write_u16(cb, 0);
	ret |= write_u32(cb, htons(skb->dev ? skb->dev->ifindex : 0));
	ret |= write_u16(cb, htons(skb->dev ? skb->dev->type : ARPHRD_ETHER));
	ret |= write_u8(cb, packet_type);
	ret |= write_u8(cb, ETH_ALEN);
	ret |= write_size(cb, ethhdr->h_source, ETH_ALEN);
	ret |= write_u16(cb, 0); /* padding (8 octets - ETH_ALEN == 2) */

	return ret;
}

static int pcap_sll2_packet(struct cap_pkt_state *s)
{
	int ret = 0;

	spin_lock_bh(&s->cb->lock);
	ret |= __pcap_header(s, LINUX_SLL2_HDRLEN);
	ret |= linux_sll2_hdr(s);
	ret |= generic_packet_data(s);
	spin_unlock_bh(&s->cb->lock);

	return ret;
}
static struct cap_ops pcap_sll2_ops = {
	.file_header	= pcap_file_header,
	.packet		= pcap_sll2_packet,
	.snaplen	= PCAP_SNAPLEN,
	.linktype	= LINKTYPE_LINUX_SLL2,
};
#endif /* CAP_SUPPORT_SLL2 */

static int ethernet_hdr(struct cap_pkt_state *s)
{
	struct ethhdr *ethhdr = eth_hdr(s->skb);

	return write_size(s->cb, ethhdr, sizeof(*ethhdr));
}

static int pcap_eth_packet(struct cap_pkt_state *s)
{
	int ret = 0;

	spin_lock_bh(&s->cb->lock);
	ret |= __pcap_header(s, ETH_ALEN);
	ret |= ethernet_hdr(s);
	ret |= generic_packet_data(s);
	spin_unlock_bh(&s->cb->lock);

	return ret;
}
static struct cap_ops pcap_eth_ops = {
	.file_header	= pcap_file_header,
	.packet		= pcap_eth_packet,
	.snaplen	= PCAP_SNAPLEN,
	.linktype	= LINKTYPE_ETHERNET,
};
#endif /* CAP_SUPPORT_PCAP */

/* --- pcapng functions --- */
#if CAP_SUPPORT_PCAPNG
static int pcapng_start_block(struct cap_buf *cb, struct pcapng_block *b,
			      u32 type)
{
	int ret = 0;

	b->offset = cb->in;

	ret |= write_u32(cb, type);
	ret |= write_u32(cb, 0); /* block size (we will fill it in later) */

	return ret;
}

static int pcapng_end_block(struct cap_buf *cb, struct pcapng_block *b)
{
	int ret = 0;
	int size;

	/* pad block size to next octet */
	size = normalize_offset(cb, cb->in + 4 - b->offset);
	size += pad_to_octet(cb);

	/* write out tail size and replace header size */
	ret |= write_u32(cb, size);
	ret |= replace_u32(cb, b->offset + 4, size);

	return ret;
}

static int pcapng_start_option(struct cap_buf *cb, struct pcapng_block *b,
			       u16 option)
{
	int ret = 0;

	/*
	 *                      1                   2                   3
	 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |      Option Code              |         Option Length         |
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * /                       Option Value                            /
	 * /              variable length, padded to 32 bits               /
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * /                                                               /
	 * /                 . . . other options . . .                     /
	 * /                                                               /
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * |   Option Code == opt_endofopt |   Option Length == 0          |
	 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 */
	ret |= write_u16(cb, option);
	ret |= write_u16(cb, 0); /* option length (calculate later) */

	b->offset = cb->in;

	return ret;
}

static int pcapng_end_option(struct cap_buf *cb, struct pcapng_block *b)
{
	int ret = 0;
	int len;

	len = normalize_offset(cb, cb->in - b->offset);
	ret |= replace_u16(cb, b->offset - 2, (u16)len);
	len += pad_to_octet(cb);

	return ret;
}

static int pcapng_terminate_options(struct cap_buf *cb)
{
	int ret = 0;

	ret |= write_u16(cb, OPT_ENDOFOPT);
	ret |= write_u16(cb, 0);

	return ret;
}

static int pcapng_section_header_block(struct cap_buf *cb)
{
	struct pcapng_block b = {};
	int ret = 0;

	/*
	 * SECTION HEADER BLOCK
	 *
	 *                         1                   2                   3
	 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  0 |                   Block Type = 0x0A0D0D0A                     |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  4 |                      Block Total Length                       |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  8 |                      Byte-Order Magic                         |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 12 |          Major Version        |         Minor Version         |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 16 |                                                               |
	 *    |                          Section Length                       |
	 *    |                                                               |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 24 /                                                               /
	 *    /                      Options (variable)                       /
	 *    /                                                               /
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *    |                      Block Total Length                       |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 */
	ret |= pcapng_start_block(cb, &b, PCAPNG_SECTION_HEADER_BLOCK);

	ret |= write_u32(cb, PCAPNG_BYTEORDER_MAGIC);
	ret |= write_u16(cb, PCAPNG_MAJOR);
	ret |= write_u16(cb, PCAPNG_MINOR);
	ret |= write_u64(cb, 0); /* section length */
	/* no options */

	ret |= pcapng_end_block(cb, &b);

	return ret;
}

static int
pcapng_interface_description_block(struct cap_buf *cb, struct net_device *dev)
{
	struct pcapng_block b, o;
	struct netdev_hw_addr *ha;
	struct in_device *in_dev;
	struct in_ifaddr *ifa;
	struct inet6_dev *in6_dev;
	struct inet6_ifaddr *ifa6;
	u16 linktype;
	int ret = 0;

	switch (dev->type) {
	case ARPHRD_ETHER:	linktype = LINKTYPE_ETHERNET; break;
	case ARPHRD_LOOPBACK:	linktype = LINKTYPE_LOOP; break;
	default:		linktype = LINKTYPE_NULL; break;
	}

	/*                        1                   2                   3
	 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 0 |                    Block Type = 0x00000001                    |
	 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 4 |                      Block Total Length                       |
	 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 8 |           LinkType            |           Reserved            |
	 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *12 |                            SnapLen                            |
	 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *16 /                                                               /
	 *   /                      Options (variable)                       /
	 *   /                                                               /
	 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *   |                      Block Total Length                       |
	 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 */
	ret |= pcapng_start_block(cb, &b, PCAPNG_INTERFACE_DESCRIPTION_BLOCK);

	ret |= write_u16(cb, linktype);
	ret |= write_u16(cb, 0); /* reserved */
	ret |= write_u32(cb, 0); /* snaplen */

	/* start of options */
	/* interface name */
	ret |= pcapng_start_option(cb, &o, OPT_IF_NAME);
	ret |= write_size(cb, dev->name, strlen(dev->name) + 1);
	ret |= pcapng_end_option(cb, &o);
	/* MAC */
	ha = list_first_entry_or_null(&dev->dev_addrs.list,
				      struct netdev_hw_addr, list);
	if (ha) {
		ret |= pcapng_start_option(cb, &o, OPT_IF_MACADDR);
		ret |= write_size(cb, ha->addr, ETH_ALEN);
		ret |= pcapng_end_option(cb, &o);
	}
	/* ipv4 */
	in_dev = in_dev_get(dev);
	if (in_dev) {
		rcu_read_lock();
		for (ifa = in_dev->ifa_list;
		     ifa && !(ifa->ifa_flags & IFA_F_SECONDARY);
		     ifa = ifa->ifa_next) {
			ret |= pcapng_start_option(cb, &o, OPT_IF_IPV4ADDR);
			ret |= write_u32(cb, ifa->ifa_address);
			ret |= write_u32(cb, ifa->ifa_mask);
			ret |= pcapng_end_option(cb, &o);
		}
		rcu_read_unlock();

		in_dev_put(in_dev);
	}
	/* ipv6 */
	in6_dev = in6_dev_get(dev);
	if (in6_dev) {
		read_lock_bh(&in6_dev->lock);
		list_for_each_entry(ifa6, &in6_dev->addr_list, if_list) {
			spin_lock(&ifa6->lock);
			ret |= pcapng_start_option(cb, &o, OPT_IF_IPV6ADDR);
			ret |= write_size(cb, &ifa6->addr, 16);
			ret |= write_u8(cb, (u8)ifa6->prefix_len);
			ret |= pcapng_end_option(cb, &o);
			spin_unlock(&ifa6->lock);
		}
		read_unlock_bh(&in6_dev->lock);

		in6_dev_put(in6_dev);
	}
	/* timestamp resolution */
	ret |= pcapng_start_option(cb, &o, OPT_IF_TSRESOL);
	ret |= write_u8(cb, PCAPNG_TSRESOL);
	ret |= pcapng_end_option(cb, &o);
	/* end of options */
	ret |= pcapng_terminate_options(cb);

	ret |= pcapng_end_block(cb, &b);

	return ret;
}

static int pcapng_enhanced_packet_block(struct cap_pkt_state *s)
{
	struct cap_buf *cb = s->cb;
	struct ethhdr *ethhdr = eth_hdr(s->skb);
	struct pcapng_block b, o;
	u32 flags = 0;
	int ret = 0;
	int len;
	u64 ts;

	if (s->skb->tstamp)
		ts = ktime_to_ns(skb_get_ktime(s->skb));
	else
		ts = ktime_get_ns();

	/*                         1                   2                   3
	 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  0 |                    Block Type = 0x00000006                    |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  4 |                      Block Total Length                       |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *  8 |                         Interface ID                          |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 12 |                        Timestamp (High)                       |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 16 |                        Timestamp (Low)                        |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 20 |                    Captured Packet Length                     |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 24 |                    Original Packet Length                     |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 * 28 /                                                               /
	 *    /                          Packet Data                          /
	 *    /              variable length, padded to 32 bits               /
	 *    /                                                               /
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *    /                                                               /
	 *    /                      Options (variable)                       /
	 *    /                                                               /
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 *    |                      Block Total Length                       |
	 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 */

	ret |= pcapng_start_block(cb, &b, PCAPNG_ENHANCED_PACKET_BLOCK);

	ret |= write_u32(cb, s->intf ? s->intf->index : 0);
	ret |= write_u32(cb, (u32)(ts >> 32));
	ret |= write_u32(cb, (u32)(ts & 0xFFFFFFFF));

	len = s->skb->len + s->skb->mac_len;
	ret |= write_u32(cb, len);
	ret |= write_u32(cb, len);
	ret |= write_size(cb, ethhdr, len);
	pad_to_octet(cb);

	/* options */
	ret |= pcapng_start_option(cb, &o, OPT_EPB_FLAGS);
	if (s->hook == NF_INET_LOCAL_OUT)
		flags |= EPB_FLAG_OUTBOUND;
	else
		flags |= EPB_FLAG_INBOUND;
	if (is_multicast_ether_addr(ethhdr->h_dest))
		flags |= EPB_FLAG_MULTICAST;
	else if (is_broadcast_ether_addr(ethhdr->h_dest))
		flags |= EPB_FLAG_BROADCAST;
	else
		flags |= EPB_FLAG_UNICAST;
	ret |= write_u32(cb, flags);
	ret |= pcapng_end_option(cb, &o);

	/* packet and lookup info */
	ret |= pcapng_start_option(cb, &o, OPT_COMMENT);
	ret |= write_str(cb, "\n%s", hook_names[s->hook]);
	ret |= write_str(cb, "\nct %08x", trunc_ptr(s->ct));
	if (s->ct && s->ct->master)
		ret |= write_str(cb, "\nmaster_ct %08x",
				 trunc_ptr(s->ct->master));
	ret |= write_str(cb, "\nskb %08x", trunc_ptr(s->skb->dev));
	ret |= write_str(cb, "\ndev %s", s->skb->dev ? s->skb->dev->name : NULL);
	ret |= write_str(cb, "\nsecpath_exists %d", secpath_exists(s->skb));
	/* classification results and saved classification */
	if (s->dump_cb)
		ret |= s->dump_cb(s);
	ret |= pcapng_end_option(cb, &o);

	ret |= pcapng_terminate_options(cb);
	ret |= pcapng_end_block(cb, &b);

	return ret;
}

static int pcapng_file_header(struct cap_buf *cb)
{
	struct cap_intf *intf;
	struct net_device *dev;
	int ret = 0, i = 0;

	spin_lock_bh(&cb->lock);

	ret |= pcapng_section_header_block(cb);

	rcu_read_lock();
	for_each_netdev_rcu(&init_net, dev) {
		intf = kzalloc(sizeof(*intf), GFP_KERNEL);
		if (!intf) {
			ret = -ENOMEM;
			break;
		}

		INIT_LIST_HEAD(&intf->node);
		intf->dev = dev;
		intf->index = i;
		list_add_tail(&intf->node, &cb->intf_list);
		i++;

		ret |= pcapng_interface_description_block(cb, dev);
	}
	rcu_read_unlock();

	spin_unlock_bh(&cb->lock);

	return ret;
}

static int pcapng_packet(struct cap_pkt_state *s)
{
	struct cap_intf *intf;
	int ret = 0;

	spin_lock_bh(&s->cb->lock);
	intf = cap_find_intf(s->cb, s->skb);
	if (!intf) {
		ret = 1;
		goto out;
	}

	s->intf = intf;
	ret |= pcapng_enhanced_packet_block(s);

out:
	spin_unlock_bh(&s->cb->lock);

	return ret;
}

static struct cap_ops pcapng_ops = {
	.file_header	= pcapng_file_header,
	.packet		= pcapng_packet,
};
#endif /* CAP_SUPPORT_PCAPNG */

/* --- debugfs interface functions --- */ 
static int cap_wait_event(struct file *file, struct cap_buf *cb)
{
	DECLARE_WAITQUEUE(waita, current);

	add_wait_queue(&cb->wait, &waita);
	set_current_state(TASK_INTERRUPTIBLE);

	spin_lock_bh(&cb->lock);
	while (!cb->avail) {
		spin_unlock_bh(&cb->lock);

		if (file->f_flags & O_NONBLOCK) {
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&cb->wait, &waita);
			return -EWOULDBLOCK;
		}
		schedule();
		if (signal_pending(current)) {
			remove_wait_queue(&cb->wait, &waita);
			return -EINTR;
		}
		set_current_state(TASK_INTERRUPTIBLE);

		spin_lock_bh(&cb->lock);
	}
	spin_unlock_bh(&cb->lock);

	set_current_state(TASK_RUNNING);
	remove_wait_queue(&cb->wait, &waita);

	return 0;
}

static int cap_open(struct inode *inode, struct file *file)
{
	struct cap_buf *cb;
	int err = -ENOMEM;

	cb = kzalloc(sizeof(*cb), GFP_KERNEL);
	if (!cb)
		goto err;

	cb->size = CAP_BUF_BASE_SIZE;
	cb->buf = kzalloc(cb->size, GFP_KERNEL);
	if (!cb->buf)
		goto err_free_pb;
	cb->ops = inode->i_private;
	if (file && file->f_path.dentry)
		cb->dentry = file->f_path.dentry->d_parent;

	init_waitqueue_head(&cb->wait);
	INIT_LIST_HEAD(&cb->intf_list);
	INIT_LIST_HEAD(&cb->node);

	err = cb->ops->file_header(cb);
	if (err)
		goto err_free_buf;

	spin_lock_bh(&cap_list_lock);
	list_add_tail(&cb->node, &cap_list);
	spin_unlock_bh(&cap_list_lock);

	file->private_data = cb;
	return 0;

err_free_buf:
	kfree(cb->buf);
err_free_pb:
	kfree(cb);
err:
	return err;
};

static ssize_t
cap_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	struct cap_buf *cb = file->private_data;
	int ret = -EFAULT;
	int read = 0;

	if (!cb || !buf)
		return -EINVAL;

	ret = cap_wait_event(file, cb);
	if (ret < 0)
		return ret;

	spin_lock_bh(&cb->lock);
	if (nbytes > cb->avail)
		nbytes = cb->avail;

	pr_debug_ratelimited("%s: nbytes to read %zu\n", __func__, nbytes);
	if (cb->size - cb->out >= nbytes) {
		ret = copy_to_user(buf, &cb->buf[cb->out], nbytes);
		if (ret)
			goto out;
		read = nbytes;
		*ppos += read;
		cb->avail -= read;
		cb->out += read;
	} else {
		ret = copy_to_user(buf, &cb->buf[cb->out], cb->size - cb->out);
		if (ret)
			goto out;
		read = cb->size - cb->out;
		*ppos += read;
		cb->avail -= read;
		cb->out = 0;
		nbytes -= read;

		ret = copy_to_user(&buf[read], &cb->buf[0], nbytes);
		if (ret)
			goto out;
		read += nbytes;
		*ppos += nbytes;
		cb->avail -= nbytes;
		cb->out = nbytes;
	}

	cb->out = cb->out % cb->size;
	ret = read;

out:
	spin_unlock_bh(&cb->lock);

	pr_debug_ratelimited("%s: avail %d in %d out %d\n", __func__,
			     cb->avail, cb->in, cb->out);
	return ret;
}

static __poll_t cap_poll(struct file *file, struct poll_table_struct *wait)
{
	struct cap_buf *cb = file->private_data;
	__poll_t mask = 0;

	if (!cb)
		return 0;

	if (file->f_mode & FMODE_READ)
		poll_wait(file, &cb->wait, wait);

	spin_lock_bh(&cb->lock);
	if (cb->avail)
		mask |= EPOLLIN | EPOLLRDNORM;
	spin_unlock_bh(&cb->lock);

	return mask;
}

static int cap_release(struct inode *inode, struct file *file)
{
	struct cap_buf *cb = file->private_data;
	struct cap_intf *intf, *tmp;

	if (!cb)
		return 0;

	spin_lock_bh(&cap_list_lock);
	list_del(&cb->node);
	spin_unlock_bh(&cap_list_lock);

	pr_notice("\rCaptured %lu pkts (%llu bytes).\n", cb->total_pkts,
		  cb->total_bytes);

	list_for_each_entry_safe(intf, tmp, &cb->intf_list, node) {
		list_del(&intf->node);
		kfree(intf);
	}

	kfree(cb->buf);
	kfree(cb);

	return 0;
}

static const struct file_operations cap_fops = {
	.open		= cap_open,
	.llseek		= no_llseek,
	.read		= cap_read,
	.poll		= cap_poll,
	.release	= cap_release
};

void pcap_packet(struct dentry *de, struct sk_buff *skb, int hook, void *data,
		 int (*dump_cb)(struct cap_pkt_state *s))
{
	enum ip_conntrack_info ctinfo;
	struct cap_pkt_state s = {
		.skb = skb,
		.ct = nf_ct_get(skb, &ctinfo),
		.hook = hook,
		.data = data,
		.dump_cb = dump_cb,
	};
	struct cap_buf *cb;
	int ret;

	if (!skb_mac_header_was_set(skb))
		return;

	spin_lock_bh(&cap_list_lock);
	list_for_each_entry(cb, &cap_list, node) {
		if (de != cb->dentry)
			continue;

		s.cb = cb;

		ret = cb->ops->packet(&s);
		if (!ret)
			cb->total_pkts++;

		pr_debug_ratelimited("%s: ret %d avail %d in %d out %d\n",
				     __func__, ret, cb->avail, cb->in,
				     cb->out);

		wake_up(&cb->wait);
	}
	spin_unlock_bh(&cap_list_lock);
}
EXPORT_SYMBOL(pcap_packet);

struct dentry *pcap_register(const char *name)
{
	struct dentry *de, *dir = NULL;

	dir = debugfs_create_dir(name, debug_pcap_dir);
	if (!dir)
		goto err;

#if CAP_SUPPORT_PCAP
	de = debugfs_create_file("pcap", S_IRUGO, dir, &pcap_eth_ops,
				 &cap_fops);
	if (!de)
		goto err_close_dir;

#if CAP_SUPPORT_SLL
	de = debugfs_create_file("pcap_sll", S_IRUGO, dir, &pcap_sll_ops,
				 &cap_fops);
	if (!de)
		goto err_close_dir;
#endif /* CAP_SUPPORT_SLL */

#if CAP_SUPPORT_SLL2
	de = debugfs_create_file("pcap_sll2", S_IRUGO, dir, &pcap_sll2_ops,
				 &cap_fops);
	if (!de)
		goto err_close_dir;
#endif /* CAP_SUPPORT_SLL2 */
#endif /* CAP_SUPPORT_PCAP */

#if CAP_SUPPORT_PCAPNG
	de = debugfs_create_file("pcapng", S_IRUGO, dir, &pcapng_ops,
				 &cap_fops);
	if (!de)
		goto err_close_dir;
#endif
	return dir;

err_close_dir:
	debugfs_remove(dir);
err:
	return NULL;
}
EXPORT_SYMBOL(pcap_register);

void pcap_unregister(struct dentry *de)
{
	if (de)
		debugfs_remove_recursive(de);
}
EXPORT_SYMBOL(pcap_unregister);

int __init pcap_init(void)
{
	debug_pcap_dir = debugfs_create_dir("pcap", NULL);
	if (!debug_pcap_dir)
		return -ENOMEM;

	return 0;
}

void __exit pcap_exit(void)
{
	if (debug_pcap_dir)
		debugfs_remove_recursive(debug_pcap_dir);
}

module_init(pcap_init);
module_exit(pcap_exit);
MODULE_LICENSE("GPL");
