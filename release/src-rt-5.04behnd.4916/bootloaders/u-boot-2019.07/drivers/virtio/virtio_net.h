/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * From Linux kernel include/uapi/linux/virtio_net.h
 */

#ifndef _LINUX_VIRTIO_NET_H
#define _LINUX_VIRTIO_NET_H

/* TODO: needs to be removed! */
#define ETH_ALEN				6

/* The feature bitmap for virtio net */

/* Host handles pkts w/ partial csum */
#define VIRTIO_NET_F_CSUM			0
/* Guest handles pkts w/ partial csum */
#define VIRTIO_NET_F_GUEST_CSUM			1
/* Dynamic offload configuration */
#define VIRTIO_NET_F_CTRL_GUEST_OFFLOADS	2
/* Initial MTU advice */
#define VIRTIO_NET_F_MTU			3
/* Host has given MAC address */
#define VIRTIO_NET_F_MAC			5
/* Guest can handle TSOv4 in */
#define VIRTIO_NET_F_GUEST_TSO4			7
/* Guest can handle TSOv6 in */
#define VIRTIO_NET_F_GUEST_TSO6			8
/* Guest can handle TSO[6] w/ ECN in */
#define VIRTIO_NET_F_GUEST_ECN			9
/* Guest can handle UFO in */
#define VIRTIO_NET_F_GUEST_UFO			10
/* Host can handle TSOv4 in */
#define VIRTIO_NET_F_HOST_TSO4			11
/* Host can handle TSOv6 in */
#define VIRTIO_NET_F_HOST_TSO6			12
/* Host can handle TSO[6] w/ ECN in */
#define VIRTIO_NET_F_HOST_ECN			13
/* Host can handle UFO in */
#define VIRTIO_NET_F_HOST_UFO			14
/* Host can merge receive buffers */
#define VIRTIO_NET_F_MRG_RXBUF			15
/* virtio_net_config.status available */
#define VIRTIO_NET_F_STATUS			16
/* Control channel available */
#define VIRTIO_NET_F_CTRL_VQ			17
/* Control channel RX mode support */
#define VIRTIO_NET_F_CTRL_RX			18
/* Control channel VLAN filtering */
#define VIRTIO_NET_F_CTRL_VLAN			19
/* Extra RX mode control support */
#define VIRTIO_NET_F_CTRL_RX_EXTRA		20
/* Guest can announce device on the network */
#define VIRTIO_NET_F_GUEST_ANNOUNCE		21
/* Device supports receive flow steering */
#define VIRTIO_NET_F_MQ				22
/* Set MAC address */
#define VIRTIO_NET_F_CTRL_MAC_ADDR		23
/* Device set linkspeed and duplex */
#define VIRTIO_NET_F_SPEED_DUPLEX		63

#ifndef VIRTIO_NET_NO_LEGACY
/* Host handles pkts w/ any GSO type */
#define VIRTIO_NET_F_GSO			6
#endif /* VIRTIO_NET_NO_LEGACY */

#define VIRTIO_NET_S_LINK_UP			1 /* Link is up */
#define VIRTIO_NET_S_ANNOUNCE			2 /* Announcement is needed */

struct __packed virtio_net_config {
	/* The config defining mac address (if VIRTIO_NET_F_MAC) */
	__u8 mac[ETH_ALEN];
	/* See VIRTIO_NET_F_STATUS and VIRTIO_NET_S_* above */
	__u16 status;
	/*
	 * Maximum number of each of transmit and receive queues;
	 * see VIRTIO_NET_F_MQ and VIRTIO_NET_CTRL_MQ.
	 * Legal values are between 1 and 0x8000
	 */
	__u16 max_virtqueue_pairs;
	/* Default maximum transmit unit advice */
	__u16 mtu;
	/*
	 * speed, in units of 1Mb. All values 0 to INT_MAX are legal.
	 * Any other value stands for unknown.
	 */
	__u32 speed;
	/*
	 * 0x00 - half duplex
	 * 0x01 - full duplex
	 * Any other value stands for unknown.
	 */
	__u8 duplex;
};

/*
 * This header comes first in the scatter-gather list. If you don't
 * specify GSO or CSUM features, you can simply ignore the header.
 *
 * This is bitwise-equivalent to the legacy struct virtio_net_hdr_mrg_rxbuf,
 * only flattened.
 */
struct virtio_net_hdr_v1 {
#define VIRTIO_NET_HDR_F_NEEDS_CSUM	0x01 /* Use csum_start, csum_offset */
#define VIRTIO_NET_HDR_F_DATA_VALID	0x02 /* Csum is valid */
	__u8 flags;
#define VIRTIO_NET_HDR_GSO_NONE		0x00 /* Not a GSO frame */
#define VIRTIO_NET_HDR_GSO_TCPV4	0x01 /* GSO frame, IPv4 TCP (TSO) */
#define VIRTIO_NET_HDR_GSO_UDP		0x03 /* GSO frame, IPv4 UDP (UFO) */
#define VIRTIO_NET_HDR_GSO_TCPV6	0x04 /* GSO frame, IPv6 TCP */
#define VIRTIO_NET_HDR_GSO_ECN		0x80 /* TCP has ECN set */
	__u8 gso_type;
	__virtio16 hdr_len;	/* Ethernet + IP + tcp/udp hdrs */
	__virtio16 gso_size;	/* Bytes to append to hdr_len per frame */
	__virtio16 csum_start;	/* Position to start checksumming from */
	__virtio16 csum_offset;	/* Offset after that to place checksum */
	__virtio16 num_buffers;	/* Number of merged rx buffers */
};

#ifndef VIRTIO_NET_NO_LEGACY
/*
 * This header comes first in the scatter-gather list.
 *
 * For legacy virtio, if VIRTIO_F_ANY_LAYOUT is not negotiated, it must
 * be the first element of the scatter-gather list. If you don't
 * specify GSO or CSUM features, you can simply ignore the header.
 */
struct virtio_net_hdr {
	/* See VIRTIO_NET_HDR_F_* */
	__u8 flags;
	/* See VIRTIO_NET_HDR_GSO_* */
	__u8 gso_type;
	__virtio16 hdr_len;	/* Ethernet + IP + tcp/udp hdrs */
	__virtio16 gso_size;	/* Bytes to append to hdr_len per frame */
	__virtio16 csum_start;	/* Position to start checksumming from */
	__virtio16 csum_offset;	/* Offset after that to place checksum */
};

/*
 * This is the version of the header to use when the MRG_RXBUF
 * feature has been negotiated.
 */
struct virtio_net_hdr_mrg_rxbuf {
	struct virtio_net_hdr hdr;
	__virtio16 num_buffers;	/* Number of merged rx buffers */
};
#endif /* ...VIRTIO_NET_NO_LEGACY */

/*
 * Control virtqueue data structures
 *
 * The control virtqueue expects a header in the first sg entry
 * and an ack/status response in the last entry.  Data for the
 * command goes in between.
 */
struct __packed virtio_net_ctrl_hdr {
	__u8 class;
	__u8 cmd;
};

typedef __u8 virtio_net_ctrl_ack;

#define VIRTIO_NET_OK				0
#define VIRTIO_NET_ERR				1

/*
 * Control the RX mode, ie. promisucous, allmulti, etc...
 *
 * All commands require an "out" sg entry containing a 1 byte state value,
 * zero = disable, non-zero = enable.
 *
 * Commands 0 and 1 are supported with the VIRTIO_NET_F_CTRL_RX feature.
 * Commands 2-5 are added with VIRTIO_NET_F_CTRL_RX_EXTRA.
 */
#define VIRTIO_NET_CTRL_RX			0
#define VIRTIO_NET_CTRL_RX_PROMISC		0
#define VIRTIO_NET_CTRL_RX_ALLMULTI		1
#define VIRTIO_NET_CTRL_RX_ALLUNI		2
#define VIRTIO_NET_CTRL_RX_NOMULTI		3
#define VIRTIO_NET_CTRL_RX_NOUNI		4
#define VIRTIO_NET_CTRL_RX_NOBCAST		5

/*
 * Control the MAC
 *
 * The MAC filter table is managed by the hypervisor, the guest should assume
 * the size is infinite. Filtering should be considered non-perfect, ie. based
 * on hypervisor resources, the guest may received packets from sources not
 * specified in the filter list.
 *
 * In addition to the class/cmd header, the TABLE_SET command requires two
 * out scatterlists. Each contains a 4 byte count of entries followed by a
 * concatenated byte stream of the ETH_ALEN MAC addresses.  The first sg list
 * contains unicast addresses, the second is for multicast. This functionality
 * is present if the VIRTIO_NET_F_CTRL_RX feature is available.
 *
 * The ADDR_SET command requests one out scatterlist, it contains a 6 bytes MAC
 * address. This functionality is present if the VIRTIO_NET_F_CTRL_MAC_ADDR
 * feature is available.
 */
struct __packed virtio_net_ctrl_mac {
	__virtio32 entries;
	__u8 macs[][ETH_ALEN];
};

#define VIRTIO_NET_CTRL_MAC			1
#define VIRTIO_NET_CTRL_MAC_TABLE_SET		0
#define VIRTIO_NET_CTRL_MAC_ADDR_SET		1

/*
 * Control VLAN filtering
 *
 * The VLAN filter table is controlled via a simple ADD/DEL interface. VLAN IDs
 * not added may be filterd by the hypervisor. Del is the opposite of add. Both
 * commands expect an out entry containing a 2 byte VLAN ID. VLAN filterting is
 * available with the VIRTIO_NET_F_CTRL_VLAN feature bit.
 */
#define VIRTIO_NET_CTRL_VLAN			2
#define VIRTIO_NET_CTRL_VLAN_ADD		0
#define VIRTIO_NET_CTRL_VLAN_DEL		1

/*
 * Control link announce acknowledgment
 *
 * The command VIRTIO_NET_CTRL_ANNOUNCE_ACK is used to indicate that driver has
 * recevied the notification; device would clear the VIRTIO_NET_S_ANNOUNCE bit
 * in the status field after it receives this command.
 */
#define VIRTIO_NET_CTRL_ANNOUNCE		3
#define VIRTIO_NET_CTRL_ANNOUNCE_ACK		0

/*
 * Control receive flow steering
 *
 * The command VIRTIO_NET_CTRL_MQ_VQ_PAIRS_SET enables receive flow steering,
 * specifying the number of the transmit and receive queues that will be used.
 * After the command is consumed and acked by the device, the device will not
 * steer new packets on receive virtqueues other than specified nor read from
 * transmit virtqueues other than specified. Accordingly, driver should not
 * transmit new packets  on virtqueues other than specified.
 */
struct virtio_net_ctrl_mq {
	__virtio16 virtqueue_pairs;
};

#define VIRTIO_NET_CTRL_MQ			4
#define VIRTIO_NET_CTRL_MQ_VQ_PAIRS_SET		0
#define VIRTIO_NET_CTRL_MQ_VQ_PAIRS_MIN		1
#define VIRTIO_NET_CTRL_MQ_VQ_PAIRS_MAX		0x8000

/*
 * Control network offloads
 *
 * Reconfigures the network offloads that guest can handle.
 *
 * Available with the VIRTIO_NET_F_CTRL_GUEST_OFFLOADS feature bit.
 *
 * Command data format matches the feature bit mask exactly.
 *
 * See VIRTIO_NET_F_GUEST_* for the list of offloads
 * that can be enabled/disabled.
 */
#define VIRTIO_NET_CTRL_GUEST_OFFLOADS		5
#define VIRTIO_NET_CTRL_GUEST_OFFLOADS_SET	0

#endif /* _LINUX_VIRTIO_NET_H */
