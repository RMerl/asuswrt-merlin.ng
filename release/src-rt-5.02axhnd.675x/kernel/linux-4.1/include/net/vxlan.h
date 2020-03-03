#ifndef __NET_VXLAN_H
#define __NET_VXLAN_H 1

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_vlan.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/udp.h>

#define VNI_HASH_BITS	10
#define VNI_HASH_SIZE	(1<<VNI_HASH_BITS)

/*
 * VXLAN Group Based Policy Extension:
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |1|-|-|-|1|-|-|-|R|D|R|R|A|R|R|R|        Group Policy ID        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                VXLAN Network Identifier (VNI) |   Reserved    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * D = Don't Learn bit. When set, this bit indicates that the egress
 *     VTEP MUST NOT learn the source address of the encapsulated frame.
 *
 * A = Indicates that the group policy has already been applied to
 *     this packet. Policies MUST NOT be applied by devices when the
 *     A bit is set.
 *
 * [0] https://tools.ietf.org/html/draft-smith-vxlan-group-policy
 */
struct vxlanhdr_gbp {
	__u8	vx_flags;
#ifdef __LITTLE_ENDIAN_BITFIELD
	__u8	reserved_flags1:3,
		policy_applied:1,
		reserved_flags2:2,
		dont_learn:1,
		reserved_flags3:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	reserved_flags1:1,
		dont_learn:1,
		reserved_flags2:2,
		policy_applied:1,
		reserved_flags3:3;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__be16	policy_id;
	__be32	vx_vni;
};

#define VXLAN_GBP_USED_BITS (VXLAN_HF_GBP | 0xFFFFFF)

/* skb->mark mapping
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |R|R|R|R|R|R|R|R|R|D|R|R|A|R|R|R|        Group Policy ID        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define VXLAN_GBP_DONT_LEARN		(BIT(6) << 16)
#define VXLAN_GBP_POLICY_APPLIED	(BIT(3) << 16)
#define VXLAN_GBP_ID_MASK		(0xFFFF)

/* VXLAN protocol header:
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |G|R|R|R|I|R|R|C|               Reserved                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                VXLAN Network Identifier (VNI) |   Reserved    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * G = 1	Group Policy (VXLAN-GBP)
 * I = 1	VXLAN Network Identifier (VNI) present
 * C = 1	Remote checksum offload (RCO)
 */
struct vxlanhdr {
	__be32 vx_flags;
	__be32 vx_vni;
};

/* VXLAN header flags. */
#define VXLAN_HF_RCO BIT(21)
#define VXLAN_HF_VNI BIT(27)
#define VXLAN_HF_GBP BIT(31)

/* Remote checksum offload header option */
#define VXLAN_RCO_MASK  0x7f    /* Last byte of vni field */
#define VXLAN_RCO_UDP   0x80    /* Indicate UDP RCO (TCP when not set *) */
#define VXLAN_RCO_SHIFT 1       /* Left shift of start */
#define VXLAN_RCO_SHIFT_MASK ((1 << VXLAN_RCO_SHIFT) - 1)
#define VXLAN_MAX_REMCSUM_START (VXLAN_RCO_MASK << VXLAN_RCO_SHIFT)

#define VXLAN_N_VID     (1u << 24)
#define VXLAN_VID_MASK  (VXLAN_N_VID - 1)
#define VXLAN_VNI_MASK  (VXLAN_VID_MASK << 8)
#define VXLAN_HLEN (sizeof(struct udphdr) + sizeof(struct vxlanhdr))

struct vxlan_metadata {
	__be32		vni;
	u32		gbp;
};

struct vxlan_sock;
typedef void (vxlan_rcv_t)(struct vxlan_sock *vh, struct sk_buff *skb,
			   struct vxlan_metadata *md);

/* per UDP socket information */
struct vxlan_sock {
	struct hlist_node hlist;
	vxlan_rcv_t	 *rcv;
	void		 *data;
	struct work_struct del_work;
	struct socket	 *sock;
	struct rcu_head	  rcu;
	struct hlist_head vni_list[VNI_HASH_SIZE];
	atomic_t	  refcnt;
	struct udp_offload udp_offloads;
	u32		  flags;
};

#define VXLAN_F_LEARN			0x01
#define VXLAN_F_PROXY			0x02
#define VXLAN_F_RSC			0x04
#define VXLAN_F_L2MISS			0x08
#define VXLAN_F_L3MISS			0x10
#define VXLAN_F_IPV6			0x20
#define VXLAN_F_UDP_CSUM		0x40
#define VXLAN_F_UDP_ZERO_CSUM6_TX	0x80
#define VXLAN_F_UDP_ZERO_CSUM6_RX	0x100
#define VXLAN_F_REMCSUM_TX		0x200
#define VXLAN_F_REMCSUM_RX		0x400
#define VXLAN_F_GBP			0x800
#define VXLAN_F_REMCSUM_NOPARTIAL	0x1000

/* Flags that are used in the receive path. These flags must match in
 * order for a socket to be shareable
 */
#define VXLAN_F_RCV_FLAGS		(VXLAN_F_GBP |			\
					 VXLAN_F_UDP_ZERO_CSUM6_RX |	\
					 VXLAN_F_REMCSUM_RX |		\
					 VXLAN_F_REMCSUM_NOPARTIAL)

struct vxlan_sock *vxlan_sock_add(struct net *net, __be16 port,
				  vxlan_rcv_t *rcv, void *data,
				  bool no_share, u32 flags);

void vxlan_sock_release(struct vxlan_sock *vs);

int vxlan_xmit_skb(struct rtable *rt, struct sock *sk, struct sk_buff *skb,
		   __be32 src, __be32 dst, __u8 tos, __u8 ttl, __be16 df,
		   __be16 src_port, __be16 dst_port, struct vxlan_metadata *md,
		   bool xnet, u32 vxflags);

static inline netdev_features_t vxlan_features_check(struct sk_buff *skb,
						     netdev_features_t features)
{
	u8 l4_hdr = 0;

	if (!skb->encapsulation)
		return features;

	switch (vlan_get_protocol(skb)) {
	case htons(ETH_P_IP):
		l4_hdr = ip_hdr(skb)->protocol;
		break;
	case htons(ETH_P_IPV6):
		l4_hdr = ipv6_hdr(skb)->nexthdr;
		break;
	default:
		return features;;
	}

	if ((l4_hdr == IPPROTO_UDP) &&
	    (skb->inner_protocol_type != ENCAP_TYPE_ETHER ||
	     skb->inner_protocol != htons(ETH_P_TEB) ||
	     (skb_inner_mac_header(skb) - skb_transport_header(skb) !=
	      sizeof(struct udphdr) + sizeof(struct vxlanhdr))))
		return features & ~(NETIF_F_ALL_CSUM | NETIF_F_GSO_MASK);

	return features;
}

/* IP header + UDP + VXLAN + Ethernet header */
#define VXLAN_HEADROOM (20 + 8 + 8 + 14)
/* IPv6 header + UDP + VXLAN + Ethernet header */
#define VXLAN6_HEADROOM (40 + 8 + 8 + 14)

#if IS_ENABLED(CONFIG_VXLAN)
void vxlan_get_rx_port(struct net_device *netdev);
#else
static inline void vxlan_get_rx_port(struct net_device *netdev)
{
}
#endif
#endif
