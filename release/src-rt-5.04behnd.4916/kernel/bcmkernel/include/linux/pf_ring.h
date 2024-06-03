/*
<:copyright-BRCM:2021:GPL/GPL:standard

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 *
 * Definitions for packet ring
 *
 * 2004-2020 - ntop.org
 *
 */

#ifndef __RING_H
#define __RING_H

/**
 * @file pf_ring.h
 *
 * @brief      PF_RING kernel module header file.
 * @details    This header file should NOT be included in PF_RING-based applications directly.
 */

#ifdef __KERNEL__
#include <linux/in6.h>
#else
#include <netinet/in.h>
#endif /* __KERNEL__ */

#if defined(CONFIG_BCM_KF_SGS) || !defined(__KERNEL__)
#include <linux/pf_ring_sgs.h>
#endif

/* Versioning */
#define RING_VERSION                "7.5.0"
#define RING_VERSION_NUM           0x070500

/* Increment whenever we change slot or packet header layout (e.g. we add/move a field) */
#define RING_FLOWSLOT_VERSION          17

#define RING_MAGIC
#define RING_MAGIC_VALUE             0x88

#define MIN_NUM_SLOTS                 512
#define DEFAULT_NUM_SLOTS            4096
#define DEFAULT_BUCKET_LEN            128
#define MAX_NUM_DEVICES               256

#define MAX_NUM_RING_SOCKETS          256

/* Watermark */
#define DEFAULT_MIN_PKT_QUEUED          1 /* 128 */
#define DEFAULT_POLL_WATERMARK_TIMEOUT  0

#define FILTERING_SAMPLING_RATIO       10

/* Set */
#define SO_ADD_TO_CLUSTER                 99
#define SO_REMOVE_FROM_CLUSTER           100
#define SO_SET_STRING                    101
#define SO_ADD_FILTERING_RULE            102
#define SO_REMOVE_FILTERING_RULE         103
#define SO_TOGGLE_FILTER_POLICY          104
#define SO_SET_SAMPLING_RATE             105
#define SO_ACTIVATE_RING                 106
#define SO_RING_BUCKET_LEN               107
#define SO_SET_CHANNEL_ID                108
#define SO_PURGE_IDLE_HASH_RULES         109 /* inactivity (sec) */
#define SO_SET_APPL_NAME                 110
#define SO_SET_PACKET_DIRECTION          111
#define SO_SET_MASTER_RING               112
#define SO_ADD_HW_FILTERING_RULE         113
#define SO_DEL_HW_FILTERING_RULE         114
#define SO_DISCARD_INJECTED_PKTS         115 /* discard stack injected packets */
#define SO_DEACTIVATE_RING               116
#define SO_SET_POLL_WATERMARK            117
#define SO_SET_VIRTUAL_FILTERING_DEVICE  118
#define SO_REHASH_RSS_PACKET             119
#define SO_SET_FILTERING_SAMPLING_RATE   120
#define SO_SET_POLL_WATERMARK_TIMEOUT    121
#define SO_SHUTDOWN_RING                 124
#define SO_PURGE_IDLE_RULES              125 /* inactivity (sec) */
#define SO_SET_SOCKET_MODE               126
#define SO_USE_SHORT_PKT_HEADER          127
#define SO_ENABLE_RX_PACKET_BOUNCE       131
#define SO_SET_APPL_STATS                133
#define SO_SET_STACK_INJECTION_MODE      134 /* stack injection/interception from userspace */
#define SO_CREATE_CLUSTER_REFEREE        135
#define SO_PUBLISH_CLUSTER_OBJECT        136
#define SO_LOCK_CLUSTER_OBJECT           137
#define SO_UNLOCK_CLUSTER_OBJECT         138
#define SO_SET_CUSTOM_BOUND_DEV_NAME     139
#define SO_SET_IFF_PROMISC               140
#define SO_SET_VLAN_ID                   141

/* Get */
#define SO_GET_RING_VERSION              170
#define SO_GET_FILTERING_RULE_STATS      171
#define SO_GET_HASH_FILTERING_RULE_STATS 172
#define SO_GET_ZC_DEVICE_INFO            173
#define SO_GET_NUM_RX_CHANNELS           174
#define SO_GET_RING_ID                   175
#define SO_GET_BPF_EXTENSIONS            176
#define SO_GET_BOUND_DEVICE_ADDRESS      177
#define SO_GET_NUM_QUEUED_PKTS           178
#define SO_GET_PKT_HEADER_LEN            179
#define SO_GET_LOOPBACK_TEST             180
#define SO_GET_BUCKET_LEN                181
#define SO_GET_DEVICE_TYPE               182
#define SO_GET_EXTRA_DMA_MEMORY          183
#define SO_GET_BOUND_DEVICE_IFINDEX      184
#define SO_GET_DEVICE_IFINDEX            185
#define SO_GET_APPL_STATS_FILE_NAME      186
#define SO_GET_LINK_STATUS               187

/* Other *sockopt */
#define SO_SELECT_ZC_DEVICE              190

/* Error codes */
#define PF_RING_ERROR_GENERIC              -1
#define PF_RING_ERROR_INVALID_ARGUMENT     -2
#define PF_RING_ERROR_NO_PKT_AVAILABLE	   -3
#define PF_RING_ERROR_NO_TX_SLOT_AVAILABLE -4
#define PF_RING_ERROR_WRONG_CONFIGURATION  -5
#define PF_RING_ERROR_END_OF_DEMO_MODE     -6
#define PF_RING_ERROR_NOT_SUPPORTED        -7
#define PF_RING_ERROR_INVALID_LIB_VERSION  -8
#define PF_RING_ERROR_UNKNOWN_ADAPTER      -9
#define PF_RING_ERROR_NOT_ENOUGH_MEMORY   -10
#define PF_RING_ERROR_INVALID_STATUS      -11
#define PF_RING_ERROR_RING_NOT_ENABLED    -12

#define REFLECTOR_NAME_LEN                 8

#ifndef IN6ADDR_ANY_INIT
#define IN6ADDR_ANY_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }
#endif

/* *********************************** */

/*
  Note that as offsets *can* be negative,
  please do not change them to unsigned
*/
struct pkt_offset {
  /* This 'eth_offset' offset *must* be added to all offsets below 
   * ONLY if you are inside the kernel. Ignore it in user-space. */
  int16_t eth_offset; 

  int16_t vlan_offset;
  int16_t l3_offset;
  int16_t l4_offset;
  int16_t payload_offset;
} __attribute__((packed));

#ifndef ETH_ALEN
#define ETH_ALEN  6
#endif

#define REFLECT_PACKET_DEVICE_NONE     0

typedef union {
  struct in6_addr v6;  /* IPv6 src/dst IP addresses (Network byte order) */
  u_int32_t v4;        /* IPv4 src/dst IP addresses */
} ip_addr;

#define ipv4_tos     ip_tos
#define ipv6_tos     ip_tos
#define ipv4_src     ip_src.v4
#define ipv4_dst     ip_dst.v4
#define ipv6_src     ip_src.v6
#define ipv6_dst     ip_dst.v6
#define host4_low    host_low.v4
#define host4_high   host_high.v4
#define host6_low    host_low.v6
#define host6_high   host_high.v6
#define host4_peer_a host_peer_a.v4
#define host4_peer_b host_peer_b.v4
#define host6_peer_a host_peer_a.v6
#define host6_peer_b host_peer_b.v6

struct eth_vlan_hdr {
  u_int16_t h_vlan_id; /* Tag Control Information (QoS, VLAN ID) */
  u_int16_t h_proto;   /* packet type ID field */
} __attribute__((packed));

#define NEXTHDR_HOP     	  0
#define NEXTHDR_IPV6    	 41
#define NEXTHDR_ROUTING 	 43
#define NEXTHDR_FRAGMENT	 44
#define NEXTHDR_ESP     	 50
#define NEXTHDR_AUTH    	 51
#define NEXTHDR_NONE    	 59
#define NEXTHDR_DEST    	 60
#define NEXTHDR_MOBILITY	135

struct kcompact_ipv6_hdr {
  u_int32_t         flow_lbl:24,
		    priority:4,
                    version:4;
  u_int16_t         payload_len;
  u_int8_t          nexthdr;
  u_int8_t          hop_limit;
  struct in6_addr saddr;
  struct in6_addr daddr;
} __attribute__((packed));

struct kcompact_ipv6_opt_hdr {
  u_int8_t          nexthdr;
  u_int8_t          hdrlen;
  u_int8_t          padding[6];
} __attribute__((packed));

#define GRE_HEADER_CHECKSUM     0x8000
#define GRE_HEADER_ROUTING      0x4000
#define GRE_HEADER_KEY          0x2000
#define GRE_HEADER_SEQ_NUM      0x1000
#define GRE_HEADER_VERSION      0x0007

struct gre_header {
  u_int16_t flags_and_version;
  u_int16_t proto;
  /* Optional fields */
} __attribute__((packed));

#define GTP_SIGNALING_PORT      2123
#define GTP_U_DATA_PORT         2152

#define GTP_VERSION_1           0x1
#define GTP_VERSION_2           0x2
#define GTP_PROTOCOL_TYPE       0x1

struct gtp_v1_hdr {
#define GTP_FLAGS_VERSION       0xE0
#define GTP_FLAGS_VERSION_SHIFT 5
#define GTP_FLAGS_PROTOCOL_TYPE 0x10
#define GTP_FLAGS_RESERVED      0x08
#define GTP_FLAGS_EXTENSION     0x04
#define GTP_FLAGS_SEQ_NUM       0x02
#define GTP_FLAGS_NPDU_NUM      0x01
  u_int8_t  flags;
  u_int8_t  message_type;
  u_int16_t payload_len;
  u_int32_t teid;
} __attribute__((__packed__));

/* Optional: GTP_FLAGS_EXTENSION | GTP_FLAGS_SEQ_NUM | GTP_FLAGS_NPDU_NUM */
struct gtp_v1_opt_hdr { 
  u_int16_t seq_num;
  u_int8_t  npdu_num;
  u_int8_t  next_ext_hdr;
} __attribute__((__packed__));

/* Optional: GTP_FLAGS_EXTENSION && next_ext_hdr != 0 */
struct gtp_v1_ext_hdr {
#define GTP_EXT_HDR_LEN_UNIT_BYTES 4
  u_int8_t len; /* 4-byte unit */
  /*
   * u_char   contents[len*4-2];
   * u_int8_t next_ext_hdr;
   */
} __attribute__((__packed__));

#define NO_TUNNEL_ID     0xFFFFFFFF

/* GPRS Tunneling Protocol */
typedef struct {
  u_int32_t tunnel_id; /* GTP/GRE tunnelId or NO_TUNNEL_ID for no filtering */
  u_int8_t tunneled_ip_version; /* Layer 4 protocol */
  u_int8_t tunneled_proto; /* Layer 4 protocol */
  ip_addr tunneled_ip_src, tunneled_ip_dst;  
  u_int16_t tunneled_l4_src_port, tunneled_l4_dst_port;
} __attribute__((packed))
 tunnel_info;

#define MOBILE_IP_PORT           434

struct mobile_ip_hdr {
  u_int8_t message_type, next_header;
  u_int16_t reserved;
} __attribute__((packed));

typedef enum {
  long_pkt_header = 0, /* it includes PF_RING-extensions over the original pcap header */
  short_pkt_header     /* Short pcap-like header */
} pkt_header_len;

struct pkt_parsing_info {
  /* Core fields (also used by NetFlow) */
  u_int8_t  dmac[ETH_ALEN], smac[ETH_ALEN];  /* MAC src/dst addresses */
  u_int16_t eth_type;         /* Ethernet type */
  u_int16_t vlan_id;          /* VLAN Id or NO_VLAN */
  u_int16_t qinq_vlan_id;     /* VLAN Id or NO_VLAN */
  u_int8_t  ip_version;
  u_int8_t  l3_proto, ip_tos; /* Layer 3 protocol, TOS */
  ip_addr   ip_src, ip_dst;   /* IPv4/6 src/dst IP addresses */
  u_int16_t l4_src_port, l4_dst_port;/* Layer 4 src/dst ports */
  u_int8_t  icmp_type, icmp_code;    /* Variables for ICMP packets */
  struct {
    u_int8_t flags;   /* TCP flags (0 if not available) */
    u_int32_t seq_num, ack_num; /* TCP sequence number */
  } tcp;
  tunnel_info tunnel;
  int32_t last_matched_rule_id; /* If > 0 identifies a rule that matched the packet */
  struct pkt_offset offset; /* Offsets of L3/L4/payload elements */
} __attribute__((packed));

#define UNKNOWN_INTERFACE          -1
#define FAKE_PACKET                -2 /* It indicates that the returned packet
					 is faked, and that the info is basically
					 a message from PF_RING
				      */

struct pfring_extended_pkthdr {
  u_int64_t timestamp_ns;  /* Packet timestamp at ns precision. Note that if your NIC supports
			      hardware timestamp, this is the place to read timestamp from */
#define PKT_FLAGS_CHECKSUM_OFFLOAD    1 << 0 /* IP/TCP checksum offload enabled */
#define PKT_FLAGS_CHECKSUM_OK         1 << 1 /* Valid checksum (with IP/TCP checksum offload enabled) */
#define PKT_FLAGS_IP_MORE_FRAG        1 << 2 /* IP More fragments flag set */
#define PKT_FLAGS_IP_FRAG_OFFSET      1 << 3 /* IP fragment offset set (not 0) */
#define PKT_FLAGS_VLAN_HWACCEL        1 << 4 /* VLAN stripped by hw */
#define PKT_FLAGS_FLOW_OFFLOAD_UPDATE 1 << 6 /* Flow update metadata, see generic_flow_update struct (keep flag compatible with ZC) */
#define PKT_FLAGS_FLOW_OFFLOAD_PACKET 1 << 7 /* Flow raw packet, pkt_hash contains the flow_id (keep flag compatible with ZC) */
#define PKT_FLAGS_FLOW_OFFLOAD_MARKER 1 << 8 /* Flow raw packet belongs to a flow that has been marked (keep flag compatible with ZC) */
  u_int32_t flags;

  u_int8_t rx_direction;   /* 1=RX: packet received by the NIC, 0=TX: packet transmitted by the NIC */
  int32_t  if_index;       /* index of the interface on which the packet has been received.
			      It can be also used to report other information */
  u_int32_t pkt_hash;      /* Hash based on the packet header */

  /* --- short header ends here --- */

  struct {
    int32_t bounce_interface; /* Interface Id where this packet will bounce after processing
			     if its values is other than UNKNOWN_INTERFACE */
    struct sk_buff *reserved; /* Kernel only pointer */
#if (defined(CONFIG_BCM_KF_SGS) && defined(CONFIG_ARM)) || defined(USER_ARCH_ARM32)
    u_int32_t reserved1;
#endif
  } tx;
#if (defined(CONFIG_BCM_KF_SGS) && defined(CONFIG_ARM)) || defined(USER_ARCH_ARM32)
    u_int8_t align[4];
#endif

  /* NOTE: leave it as last field of the memset on parse_pkt() will fail */
  struct pkt_parsing_info parsed_pkt; /* packet parsing info */
} __attribute__((packed));

/* NOTE: Keep 'struct pfring_pkthdr' in sync with 'struct pcap_pkthdr' */

struct pfring_pkthdr {
  /* pcap header */
  struct timeval ts;    /* time stamp */
#if (defined(CONFIG_BCM_KF_SGS) && defined(CONFIG_ARM)) || defined(USER_ARCH_ARM32)
  u_int32_t padding[2];
#endif
  u_int32_t caplen;     /* length of portion present */
  u_int32_t len;        /* length of whole packet (off wire) */
#if defined(CONFIG_BCM_KF_SGS) || !defined(__KERNEL__)
  struct sgs_pkthdr sgs;
#endif
  struct pfring_extended_pkthdr extended_hdr; /* PF_RING extended header */
} __attribute__((packed));

/* *********************************** */

#define MAX_NUM_LIST_ELEMENTS MAX_NUM_RING_SOCKETS /* sizeof(bits_set) [see below] */

#ifdef __KERNEL__
typedef struct {
  u_int32_t num_elements, top_element_id;
  rwlock_t list_lock;
  void *list_elements[MAX_NUM_LIST_ELEMENTS];
} lockless_list;

void init_lockless_list(lockless_list *l);
int lockless_list_add(lockless_list *l, void *elem);
int lockless_list_remove(lockless_list *l, void *elem);
void* lockless_list_get_next(lockless_list *l, u_int32_t *last_list_idx);
void* lockless_list_get_first(lockless_list *l, u_int32_t *last_list_idx);
void lockless_list_empty(lockless_list *l, u_int8_t free_memory);
void term_lockless_list(lockless_list *l, u_int8_t free_memory);
#endif

/* ************************************************* */

typedef struct {
  int32_t if_index;                    /* Index of the interface on which the packet has been received */
  u_int8_t smac[ETH_ALEN], dmac[ETH_ALEN]; /* Use '0' (zero-ed MAC address) for any MAC address.
					      This is applied to both source and destination. */
  u_int16_t vlan_id;                   /* Use 0 for any vlan */
  u_int16_t eth_type;		       /* Use 0 for any ethernet type */
  u_int8_t  proto;                     /* Use 0 for any l3 protocol */
  ip_addr   shost, dhost;              /* User '0' for any host. This is applied to both source and destination. */
  ip_addr   shost_mask, dhost_mask;    /* IPv4/6 network mask */
  u_int16_t sport_low, sport_high;     /* All ports between port_low...port_high means 'any' port */
  u_int16_t dport_low, dport_high;     /* All ports between port_low...port_high means 'any' port */
  struct {
    u_int8_t flags;             /* TCP flags (0 if not available) */
  } tcp;
} __attribute__((packed))
filtering_rule_core_fields;

/* ************************************************* */

typedef struct {

#define FILTER_TUNNEL_ID_FLAG 1 << 0
  u_int16_t optional_fields;          /* Use this mask to activate optional fields */

  struct {
    u_int32_t tunnel_id;              /* GTP/GRE tunnelId or NO_TUNNEL_ID for no filtering */
    ip_addr   shost, dhost;           /* Filter on tunneled IPs */
    ip_addr   shost_mask, dhost_mask; /* IPv4/6 network mask */
  } tunnel;

  char payload_pattern[32];         /* If strlen(payload_pattern) > 0, the packet payload
				       must match the specified pattern */
} __attribute__((packed))
filtering_rule_extended_fields;

/* ************************************************* */

typedef enum {
  forward_packet_and_stop_rule_evaluation = 0,
  dont_forward_packet_and_stop_rule_evaluation,
  execute_action_and_continue_rule_evaluation,
  execute_action_and_stop_rule_evaluation,
  forward_packet_add_rule_and_stop_rule_evaluation, /* auto-filled hash rule */
  reflect_packet_and_stop_rule_evaluation,
  reflect_packet_and_continue_rule_evaluation,
  bounce_packet_and_stop_rule_evaluation,
  bounce_packet_and_continue_rule_evaluation
} rule_action_behaviour;

typedef enum {
  pkt_detail_flow,
  pkt_detail_aggregation
} pkt_detail_mode;

typedef enum {
  rx_and_tx_direction = 0,
  rx_only_direction,
  tx_only_direction
} packet_direction;

typedef enum {
  send_and_recv_mode = 0,
  send_only_mode,
  recv_only_mode
} socket_mode;

typedef struct {
  unsigned long jiffies_last_match;  /* Jiffies of the last rule match (updated by pf_ring) */
  struct net_device *reflector_dev;  /* Reflector device */
} __attribute__((packed))
filtering_internals;

typedef struct {
#define FILTERING_RULE_AUTO_RULE_ID 0xFFFF
  u_int16_t rule_id;                 /* Rules are processed in order from lowest to higest id */

  rule_action_behaviour rule_action; /* What to do in case of match */
  u_int8_t balance_id, balance_pool; /* If balance_pool > 0, then pass the packet above only if the
					(hash(proto, sip, sport, dip, dport) % balance_pool) = balance_id */
  u_int8_t locked;		     /* Do not purge with pfring_purge_idle_rules() */
  u_int8_t bidirectional;	     /* Swap peers when checking if they match the rule. Default: monodir */
  filtering_rule_core_fields     core_fields;
  filtering_rule_extended_fields extended_fields;
  char reflector_device_name[REFLECTOR_NAME_LEN];

  filtering_internals internals;   /* PF_RING internal fields */
} __attribute__((packed))
filtering_rule;

/* *********************************** */

/* 82599 packet steering filters */

typedef struct {
  u_int8_t  proto;
  u_int32_t s_addr, d_addr;
  u_int16_t s_port, d_port;
  u_int16_t queue_id;
} __attribute__((packed))
intel_82599_five_tuple_filter_hw_rule;

typedef struct {
  u_int16_t vlan_id;
  u_int8_t  proto;
  u_int32_t s_addr, d_addr;
  u_int16_t s_port, d_port;
  u_int16_t queue_id;
} __attribute__((packed))
intel_82599_perfect_filter_hw_rule;

/*
  Rules are defined per port. Each redirector device
  has 4 ports (numbeder 0..3):

         0   +--------------+   2   +--------------+
  LAN  <===> |              | <===> |   1/10G      |
             |  Redirector  |       |   Ethernet   |
  LAN  <===> |    Switch    | <===> |   Adapter    |
         1   +--------------+   3   +--------------+

  Drop Rule
  Discard incoming packets matching the filter
  on 'rule_port'

  Redirect Rule
  Divert incoming packets matching the filter
  on 'rule_port' to 'rule_target_port'.

  Mirror Rule
  Copy incoming packets matching the filter
  on 'rule_port' to 'rule_target_port'. The original
  packet will continue its journey (i.e. packet are
  actually duplicated)
*/

typedef enum {
  drop_rule,
  redirect_rule,
  mirror_rule
} silicom_redirector_rule_type;

typedef struct {
  silicom_redirector_rule_type rule_type;
  u_int8_t rule_port; /* Port on which the rule is defined */
  u_int8_t rule_target_port; /* Target port (ignored for drop rules) */
  u_int16_t vlan_id_low, vlan_id_high;
  u_int8_t l3_proto;
  ip_addr src_addr, dst_addr;
  u_int32_t src_mask, dst_mask;
  u_int16_t src_port_low, src_port_high;
  u_int16_t dst_port_low, dst_port_high;
} __attribute__((packed))
silicom_redirector_hw_rule;

typedef enum {
  accolade_drop,
  accolade_pass
} accolade_rule_action_type;

/* Accolade supports mode 1 filtering on almost all cards (up to 32 rules),
 * and mode 2 filtering on selected adapters (up to 1K rules).
 * PF_RING automatically select mode 2 when available, and mode 1 as fallback.
 * Mode 1 and 2 support different fields, please refer to the fields description. */
typedef struct {
  accolade_rule_action_type action; /* in mode 2 this should be always the opposite of the default action */
  u_int32_t port_mask; /* ports on which the rule is defined (default 0xf) - mode 1 only */
  u_int8_t ip_version;
  u_int8_t protocol; /* l4 */
  u_int16_t vlan_id; /* mode 2 only (if vlan_id is set, mpls_label is ignored due to hw limitations) */
  u_int32_t mpls_label; /* mode 2 only */
  ip_addr src_addr, dst_addr;
  u_int32_t src_addr_bits, dst_addr_bits;
  u_int16_t src_port_low;
  u_int16_t src_port_high; /* mode 1 only */
  u_int16_t dst_port_low;
  u_int16_t dst_port_high; /* mode 1 only */
  u_int8_t l4_port_not; /* rule match if src_port_low/dst_port_low are defined and they do not match - mode 2 only */
} __attribute__((packed))
accolade_hw_rule;

typedef enum {
  flow_drop_rule,
  flow_mark_rule
} generic_flow_rule_action_type;

typedef struct { 
  generic_flow_rule_action_type action;
  u_int32_t flow_id; /* flow id from flow metadata */
  u_int32_t thread; /* id of the thread setting the rule */
} __attribute__((packed))
generic_flow_id_hw_rule;

typedef struct { 
  generic_flow_rule_action_type action;
  ip_addr src_ip;
  ip_addr dst_ip;
  u_int16_t src_port;
  u_int16_t dst_port;
  u_int8_t ip_version;
  u_int8_t protocol;
  u_int8_t interface; /* from extended_hdr.if_index */
} __attribute__((packed))
generic_flow_tuple_hw_rule;

typedef enum {
  intel_82599_five_tuple_rule,
  intel_82599_perfect_filter_rule,
  silicom_redirector_rule,
  generic_flow_id_rule,
  generic_flow_tuple_rule,
  accolade_rule,
  accolade_default
} hw_filtering_rule_type;

typedef struct {
  hw_filtering_rule_type rule_family_type;
  u_int16_t rule_id;

  union {
    intel_82599_five_tuple_filter_hw_rule five_tuple_rule;
    intel_82599_perfect_filter_hw_rule perfect_rule;
    silicom_redirector_hw_rule redirector_rule;
    generic_flow_id_hw_rule flow_id_rule;
    generic_flow_tuple_hw_rule flow_tuple_rule;
    accolade_hw_rule accolade_rule;
  } rule_family;
} __attribute__((packed))
hw_filtering_rule;

#define MAGIC_HW_FILTERING_RULE_REQUEST  0x29010020 /* deprecated? */

#ifdef __KERNEL__

#define ETHTOOL_PFRING_SRXFTCHECK 0x10000000
#define ETHTOOL_PFRING_SRXFTRLDEL 0x10000031
#define ETHTOOL_PFRING_SRXFTRLINS 0x10000032

#if defined(I82599_HW_FILTERING_SUPPORT) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,40))
#define	FLOW_EXT 0x80000000
union _kcompat_ethtool_flow_union {
	struct ethtool_tcpip4_spec		tcp_ip4_spec;
	struct ethtool_usrip4_spec		usr_ip4_spec;
	__u8					hdata[60];
};
struct _kcompat_ethtool_flow_ext {
	__be16	vlan_etype;
	__be16	vlan_tci;
	__be32	data[2];
};
struct _kcompat_ethtool_rx_flow_spec {
	__u32		flow_type;
	union _kcompat_ethtool_flow_union h_u;
	struct _kcompat_ethtool_flow_ext h_ext;
	union _kcompat_ethtool_flow_union m_u;
	struct _kcompat_ethtool_flow_ext m_ext;
	__u64		ring_cookie;
	__u32		location;
};
#define ethtool_rx_flow_spec _kcompat_ethtool_rx_flow_spec
#endif /* defined(I82599_HW_FILTERING_SUPPORT) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,40)) */

#endif /* __KERNEL__ */

typedef enum {
  add_hw_rule,
  remove_hw_rule
} hw_filtering_rule_command;

/* *********************************** */

struct pfring_timespec {
  u_int32_t tv_sec;
  u_int32_t tv_nsec;
} __attribute__((packed));

typedef struct { 
  u_int32_t flow_id;

  u_int8_t ip_version;
  u_int8_t l4_protocol;

  u_int8_t tos;
  u_int8_t tcp_flags;

  ip_addr src_ip;
  ip_addr dst_ip;

  u_int16_t vlan_id;
  u_int16_t reserved; /* padding */

  u_int16_t src_port;
  u_int16_t dst_port;

  u_int32_t fwd_packets;
  u_int32_t fwd_bytes;
  u_int32_t rev_packets;
  u_int32_t rev_bytes;
  
  struct pfring_timespec fwd_ts_first;
  struct pfring_timespec fwd_ts_last;
  struct pfring_timespec rev_ts_first;
  struct pfring_timespec rev_ts_last;
} __attribute__((packed))
generic_flow_update;

typedef struct {
  generic_flow_rule_action_type action;
  u_int32_t flow_id;
} __attribute__((packed))
generic_flow_feedback;

/* *********************************** */

extern struct pf_ring_socket *pfr; /* Forward */

/* *********************************** */

typedef int (*five_tuple_rule_handler)(struct pf_ring_socket *pfr,
				       hw_filtering_rule *rule,
				       hw_filtering_rule_command request);
typedef int (*perfect_filter_hw_rule_handler)(struct pf_ring_socket *pfr,
					      hw_filtering_rule *rule,
					      hw_filtering_rule_command request);

typedef struct {
  five_tuple_rule_handler five_tuple_handler;
  perfect_filter_hw_rule_handler perfect_filter_handler;
} __attribute__((packed))
hw_filtering_device_handler;

/* *********************************** */

/* Hash size used for precise packet matching */
#define DEFAULT_RING_HASH_SIZE     4096

/*
 * The hashtable contains only perfect matches: no
 * wildacards or so are accepted. (bidirectional)
 */
typedef struct {
  u_int16_t rule_id; /* Future use */
  u_int16_t vlan_id;
  u_int8_t ip_version;
  u_int8_t proto; /* Layer 3 protocol */
  ip_addr host_peer_a, host_peer_b;
  u_int16_t port_peer_a, port_peer_b;

  rule_action_behaviour rule_action; /* What to do in case of match */
  char reflector_device_name[REFLECTOR_NAME_LEN];

  filtering_internals internals;   /* PF_RING internal fields */
} __attribute__((packed))
hash_filtering_rule;

typedef struct {
  u_int64_t match;
  u_int64_t filtered;
  u_int64_t match_forward;
  u_int32_t inactivity; /* sec */
} __attribute__((packed))
hash_filtering_rule_stats;

/* ************************************************* */

typedef struct _sw_filtering_hash_bucket {
  hash_filtering_rule           rule;
  u_int64_t                     match;         /* number of packets matching the rule */
  u_int64_t                     filtered;      /* number of packets filtered by the rule */
  u_int64_t                     match_forward; /* number of packets sampled by the rule (equivalent to match minus filtered) */
  struct _sw_filtering_hash_bucket *next;
} __attribute__((packed))
sw_filtering_hash_bucket;

/* *********************************** */

#define RING_MIN_SLOT_SIZE    (60+sizeof(struct pfring_pkthdr))
#define RING_MAX_SLOT_SIZE    (2000+sizeof(struct pfring_pkthdr))

#if !defined(__cplusplus)

#define min_val(a,b) ((a < b) ? a : b)
#define max_val(a,b) ((a > b) ? a : b)

#endif

/* *********************************** */
#if defined(CONFIG_BCM_KF_SGS) || !defined(__KERNEL__)
struct response {
	union {
		unsigned long	status;
		u_int64_t	status_u64;
	};
	union {
		struct sk_buff	*skb;
		u_int64_t	skb_u64;
	};
};

struct response_queue {
	u_int64_t	read;  /* managed by kernel */
	u_int64_t	size;  /* managed by kernel */
	u_int64_t	write; /* managed by userspace */
	struct response	data[(65536 - 48) / sizeof(struct response)];
} __attribute__((packed));
#endif

/* False sharing reference: http://en.wikipedia.org/wiki/False_sharing */

typedef struct flowSlotInfo {
  /* first page, managed by kernel */
  u_int16_t version, sample_rate;
  u_int32_t min_num_slots, slot_len, data_len;
  u_int64_t tot_mem;
  volatile u_int64_t insert_off;
  u_int64_t kernel_remove_off;
  u_int64_t tot_pkts, tot_lost;
  volatile u_int64_t tot_insert;
  u_int64_t kernel_tot_read;
  u_int64_t tot_fwd_ok, tot_fwd_notok;
  u_int64_t good_pkt_sent, pkt_send_error;
  /* <-- 64 bytes here, should be enough to avoid some L1 VIVT coherence issues (32 ~ 64bytes lines) */
  char padding[128-104];
  /* <-- 128 bytes here, should be enough to avoid false sharing in most L2 (64 ~ 128bytes lines) */
  char k_padding[4096-128];
  /* <-- 4096 bytes here, to get a page aligned block writable by kernel side only */

  /* second page, managed by userland */
  volatile u_int64_t tot_read;
  volatile u_int64_t remove_off /* managed by userland */;
#if defined(CONFIG_BCM_KF_SGS) || !defined(__KERNEL__)
  struct response_queue rq;
  /* <-- 69632 (68k) bytes here, to get a page aligned block writable by userland only */
#endif
} __attribute__((packed))
FlowSlotInfo;

/* **************************************** */

#ifdef __KERNEL__
FlowSlotInfo *getRingPtr(void);
int allocateRing(char *deviceName, u_int numSlots, u_int bucketLen, u_int sampleRate);
unsigned int pollRing(struct file *fp, struct poll_table_struct * wait);
void deallocateRing(void);
#endif /* __KERNEL__ */

/* *********************************** */

#define PF_RING          27      /* (0x1b) Packet Ring */
#define SOCK_RING        PF_RING

/* ioctl() */
#define SIORINGPOLL      0x8888

/* ************************************************* */

#ifdef __KERNEL__
struct ring_sock {
  struct sock           sk; /* It MUST be the first element */
  struct pf_ring_socket *pf_ring_sk;
  /* FIXX Do we really need the following items? */
  //struct packet_type    prot_hook;
  //spinlock_t		bind_lock;
};
#endif

/* *********************************** */

typedef int (*zc_dev_wait_packet)(void *adapter, int mode);
typedef int (*zc_dev_notify)(void *rx_adapter_ptr, void *tx_adapter_ptr, u_int8_t device_in_use);

typedef enum {
  add_device_mapping = 0, remove_device_mapping
} zc_dev_operation;

/* IMPORTANT NOTE
 * add new family types ALWAYS at the end
 * (i.e. append) of this datatype */
typedef enum {
  intel_e1000e = 0,
  intel_igb,
  intel_ixgbe,
  intel_ixgbe_82598,
  intel_ixgbe_82599,
  intel_igb_82580,
  intel_e1000,
  intel_ixgbe_82599_ts,
  intel_i40e,
  intel_fm10k,
  intel_ixgbe_vf,
  intel_ixgbe_x550
} zc_dev_model;

typedef struct {
  u_int32_t packet_memory_num_slots;
  u_int32_t packet_memory_slot_len;
  u_int32_t descr_packet_memory_tot_len;
  u_int16_t registers_index;
  u_int16_t stats_index;
  u_int32_t vector;
  u_int32_t num_queues;
} __attribute__((packed))
mem_ring_info;

typedef struct {
  mem_ring_info rx;
  mem_ring_info tx;
  u_int32_t phys_card_memory_len;
  zc_dev_model device_model;
} __attribute__((packed))
zc_memory_info;

typedef struct {
  zc_memory_info mem_info;
  u_int16_t channel_id;
  void *rx_descr_packet_memory; /* Invalid in userland */
  void *tx_descr_packet_memory; /* Invalid in userland */
  char *phys_card_memory;       /* Invalid in userland */
  struct net_device *dev;       /* Invalid in userland */
  struct device *hwdev;         /* Invalid in userland */
  u_char device_address[6];
#ifdef __KERNEL__
  wait_queue_head_t *packet_waitqueue;
#else
  void *packet_waitqueue;
#endif
  u_int8_t *interrupt_received, in_use;
  void *rx_adapter_ptr, *tx_adapter_ptr;
  zc_dev_wait_packet wait_packet_function_ptr;
  zc_dev_notify usage_notification;
} __attribute__((packed))
zc_dev_info;

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

typedef struct {
  zc_dev_operation operation;
  char device_name[IFNAMSIZ];
  int32_t channel_id;
  zc_dev_model device_model; /* out */
} __attribute__((packed))
zc_dev_mapping;

/* ************************************************* */

#define RING_ANY_CHANNEL          ((u_int64_t)-1)
#define MAX_NUM_RX_CHANNELS       64 /* channel_id_mask is a 64 bit mask */
#define UNKNOWN_NUM_RX_CHANNELS   1

#define RING_ANY_VLAN             ((u_int16_t)0xFFFF)
#define RING_NO_VLAN              ((u_int16_t)0)

/* ************************************************* */

typedef enum {
  cluster_per_flow = 0,              /* 6-tuple: <src ip, src port, dst ip, dst port, proto, vlan>  */
  cluster_round_robin,
  cluster_per_flow_2_tuple,          /* 2-tuple: <src ip,           dst ip                       >  */
  cluster_per_flow_4_tuple,          /* 4-tuple: <src ip, src port, dst ip, dst port             >  */
  cluster_per_flow_5_tuple,          /* 5-tuple: <src ip, src port, dst ip, dst port, proto      >  */
  cluster_per_flow_tcp_5_tuple,      /* 5-tuple only with TCP, 2 tuple with all other protos        */
  /* same as above, computing on tunnel content when present */
  cluster_per_inner_flow,            /* 6-tuple: <src ip, src port, dst ip, dst port, proto, vlan>  */
  cluster_per_inner_flow_2_tuple,    /* 2-tuple: <src ip,           dst ip                       >  */
  cluster_per_inner_flow_4_tuple,    /* 4-tuple: <src ip, src port, dst ip, dst port             >  */
  cluster_per_inner_flow_5_tuple,    /* 5-tuple: <src ip, src port, dst ip, dst port, proto      >  */
  cluster_per_inner_flow_tcp_5_tuple,/* 5-tuple only with TCP, 2 tuple with all other protos        */
  /* new types, for L2-only protocols */
  cluster_per_flow_ip_5_tuple,       /* 5-tuple only with IP, 2 tuple with non-IP <src mac, dst mac> */
  cluster_per_inner_flow_ip_5_tuple, /* 5-tuple only with IP, 2 tuple with non-IP <src mac, dst mac> */
  cluster_per_flow_ip_with_dup_tuple /* 1-tuple: <src ip> and <dst ip> with duplication              */
} cluster_type;

#define MAX_CLUSTER_TYPE_ID cluster_per_flow_ip_with_dup_tuple

struct add_to_cluster {
  u_int clusterId;
  cluster_type the_type;
} __attribute__((packed));

typedef enum {
  standard_nic_family = 0, /* No Hw Filtering */
  intel_82599_family
} pfring_device_type;

typedef struct {
  char device_name[IFNAMSIZ];
  pfring_device_type device_type;

  /* Entry in the /proc filesystem */
  struct proc_dir_entry *proc_entry;
} __attribute__((packed))
virtual_filtering_device_info;

/* ************************************************* */

struct create_cluster_referee_info {
  u_int32_t cluster_id;
  u_int32_t recovered; /* fresh or recovered */
} __attribute__((packed));

struct public_cluster_object_info {
  u_int32_t cluster_id;
  u_int32_t object_type;
  u_int32_t object_id;
} __attribute__((packed));

struct lock_cluster_object_info {
  u_int32_t cluster_id;
  u_int32_t object_type;
  u_int32_t object_id;
  u_int32_t lock_mask;
  u_int32_t reserved;
} __attribute__((packed));

/* ************************************************* */

typedef enum {
  cluster_slave  = 0,
  cluster_master = 1
} cluster_client_type;

/* ************************************************* */

#ifdef __KERNEL__

#if(LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0))
#ifndef netdev_notifier_info_to_dev
#define netdev_notifier_info_to_dev(a) ((struct net_device*)a)
#endif
#endif

#define CLUSTER_LEN       64

/*
 * A ring cluster is used group together rings used by various applications
 * so that they look, from the PF_RING point of view, as a single ring.
 * This means that developers can use clusters for sharing packets across
 * applications using various policies as specified in the hashing_mode
 * parameter.
 */
struct ring_cluster {
  u_int32_t      cluster_id; /* 0 = no cluster */
  u_int32_t      num_cluster_elements;
  cluster_type   hashing_mode;
  u_short        hashing_id;
  struct sock    *sk[CLUSTER_LEN];
};

/*
 * Linked-list of ring clusters
 */
typedef struct {
  struct ring_cluster cluster;
  struct list_head list;
} ring_cluster_element;

#define MAX_NUM_ZC_BOUND_SOCKETS MAX_NUM_RING_SOCKETS

typedef struct {
  u8 num_bound_sockets;
  zc_dev_info zc_dev;
  struct list_head list;
  /*
    In the ZC world only one application can open and enable the 
    device@channel per direction. The array below is used to keep
    pointers to the sockets bound to device@channel.
    No more than one socket can be enabled for RX and one for TX.
  */
  struct pf_ring_socket *bound_sockets[MAX_NUM_ZC_BOUND_SOCKETS];
  rwlock_t lock;
} zc_dev_list;

#define MAX_NUM_IFINDEX 0x7FFFFFFF
#define MAX_NUM_DEV_IDX 1024

/*
 * Linked-list of virtual filtering devices
 */
typedef struct {
  virtual_filtering_device_info info;
  struct list_head list;
} virtual_filtering_device_element;

/* ************************************************* */

typedef struct {
  u_int8_t set;
  u_int8_t direct_mapping;
  int32_t ifindex;
} ifindex_map_item;

/* ************************************************* */

typedef struct {
  struct net_device *dev;

  /* Note: we keep device_name here for a couple of reasons:
   * 1. some device types might NOT have a net_device handler
   * 2. when a device name changes we need to remember the old name */
  char device_name[IFNAMSIZ];

  pfring_device_type device_type; /* Device Type */
  int32_t dev_index;

  u_int8_t do_not_remove_promisc; /* promisc was set before any socket */
  atomic_t promisc_users; /* number of rings with promisc set bound to this device */

  /* Entry in the /proc filesystem */
  struct proc_dir_entry *proc_entry;
  struct proc_dir_entry *proc_info_entry;

  /* ZC */
  u_int8_t is_zc_device;
  zc_dev_model zc_dev_model;
  u_int num_zc_dev_rx_queues; /* 0 for non ZC devices */
  u_int32_t num_zc_rx_slots;
  u_int32_t num_zc_tx_slots;

  /* Hardware Filters */
  struct {
    u_int16_t num_filters;
    hw_filtering_device_handler filter_handlers;
  } hw_filters;

  struct list_head device_list;
} pf_ring_device;

/* ************************************************* */

struct dma_memory_info {
  u_int32_t num_chunks, chunk_len;
  u_int32_t num_slots,  slot_len;
  unsigned long *virtual_addr;  /* chunks pointers */
  u_int64_t     *dma_addr;      /* per-slot DMA adresses */
  struct device *hwdev;         /* dev for DMA mapping */
};

/* ************************************************* */

typedef struct {
  u_int32_t object_type;
  u_int32_t object_id;
  u_int32_t lock_bitmap;

  struct list_head list;
} cluster_object;

struct cluster_referee {
  u_int32_t id;
  u_int32_t users;
  u_int8_t  master_running;
  struct list_head objects_list;

  struct list_head list;
};

/* ************************************************* */

typedef int (*do_handle_sw_filtering_hash_bucket)(struct pf_ring_socket *pfr,
					       sw_filtering_hash_bucket* rule,
					       u_char add_rule);

typedef int (*do_add_packet_to_ring)(struct pf_ring_socket *pfr,
				     u_int8_t real_skb,
				     struct pfring_pkthdr *hdr, struct sk_buff *skb,
				     int displ, u_int8_t parse_pkt_first);

typedef int (*do_add_raw_packet_to_ring)(struct pf_ring_socket *pfr,
					 struct pfring_pkthdr *hdr,
					 u_char *data, u_int data_len,
					 u_int8_t parse_pkt_first);

typedef u_int32_t (*do_rehash_rss)(struct sk_buff *skb, struct pfring_pkthdr *hdr);

/* ************************************************* */

#define NUM_FRAGMENTS_HASH_SLOTS                          4096
#define MAX_CLUSTER_FRAGMENTS_LEN   8*NUM_FRAGMENTS_HASH_SLOTS

struct hash_fragment_node {
  /* Key */
  u_int32_t ipv4_src_host, ipv4_dst_host;
  u_int16_t ip_fragment_id;

  /* Value */
  u_int8_t cluster_app_id; /* Identifier of the app where the main fragment has been placed */

  /* Expire */
  unsigned long expire_jiffies; /* Time at which this entry will be expired */

  /* collision list */
  struct list_head frag_list;
};

/* ************************************************* */

/*
 * Ring options
 */
struct pf_ring_socket {
  rwlock_t ring_config_lock;

  u_int8_t ring_active, ring_shutdown, num_rx_channels, num_bound_devices;
  pf_ring_device *ring_dev;

  /* last device set with bind, needed to heck channels when multiple
   * devices are used with quick-mode */
  pf_ring_device *last_bind_dev; 

  DECLARE_BITMAP(pf_dev_mask, MAX_NUM_DEV_IDX /* bits */);
  int ring_pid;
  u_int32_t ring_id;
  char *appl_name; /* String that identifies the application bound to the socket */
  packet_direction direction; /* Specify the capture direction for packets */
  socket_mode mode; /* Specify the link direction to enable (RX, TX, both) */
  pkt_header_len header_len;
  u_int8_t stack_injection_mode;
  u_int8_t discard_injected_pkts;
  u_int8_t promisc_enabled;
  u_int8_t __padding;

  struct sock *sk;

  /* /proc */
  char sock_proc_name[64];       /* /proc/net/pf_ring/<sock_proc_name>             */
  char sock_proc_stats_name[64]; /* /proc/net/pf_ring/stats/<sock_proc_stats_name> */
  char statsString[1024];
  char custom_bound_device_name[32];

  /* Poll Watermark */
  u_int32_t num_poll_calls;
  u_int16_t poll_num_pkts_watermark;
  u_int16_t poll_watermark_timeout;
  u_long    queue_nonempty_timestamp;

  /* Master Ring */
  struct pf_ring_socket *master_ring;

  /* Used to transmit packets after they have been received
     from user space */
  struct {
    u_int8_t enable_tx_with_bounce;
    rwlock_t consume_tx_packets_lock;
    int32_t last_tx_dev_idx;
    struct net_device *last_tx_dev;
  } tx;

  /* ZC (Direct NIC Access) */
  zc_dev_mapping zc_mapping;
  zc_dev_info *zc_dev;
  zc_dev_list *zc_device_entry;

  /* Extra DMA memory */
  struct dma_memory_info *extra_dma_memory;

  /* Cluster */
  u_int32_t cluster_id /* 0 = no cluster */;

  /* Channel */
  int64_t channel_id_mask;  /* -1 = any channel */
  u_int16_t num_channels_per_ring;

  /* rehash rss function pointer */
  do_rehash_rss rehash_rss;

  /* Ring Slots */
  u_char *ring_memory;
  u_int16_t slot_header_len;
  u_int32_t bucket_len, slot_tot_mem;
  FlowSlotInfo *slots_info; /* Points to ring_memory */
  u_char *ring_slots;       /* Points to ring_memory+sizeof(FlowSlotInfo) */

  /* Packet Sampling */
  u_int32_t pktToSample, sample_rate;

  /* Virtual Filtering Device */
  virtual_filtering_device_element *v_filtering_dev;

  /* VLAN ID */
  u_int16_t vlan_id; /* 0 = all VLANs are accepted */

  int32_t bpfFilter; /* bool */

  /* Sw Filtering Rules - default policy */
  u_int8_t sw_filtering_rules_default_accept_policy; /* 1=default policy is accept, drop otherwise */

  /* Sw Filtering Rules - hash */
  sw_filtering_hash_bucket **sw_filtering_hash;
  u_int64_t sw_filtering_hash_match;
  u_int64_t sw_filtering_hash_miss;
  u_int64_t sw_filtering_hash_filtered;
  u_int32_t num_sw_filtering_hash;

  /* Sw Filtering Rules - wildcard */
  u_int32_t num_sw_filtering_rules;
  struct list_head sw_filtering_rules;

  /* Hw Filtering Rules */
  u_int16_t num_hw_filtering_rules;
  struct list_head hw_filtering_rules;

  /* Filtering sampling */
  u_int32_t filtering_sample_rate;
  u_int32_t filtering_sampling_size;

  /* Locks */
  atomic_t num_ring_users;
  wait_queue_head_t ring_slots_waitqueue;
  rwlock_t ring_index_lock, ring_rules_lock;

  /* Indexes (Internal) */
  u_int32_t insert_page_id, insert_slot_id;

  /* Function pointer */
  do_add_packet_to_ring add_packet_to_ring;
  do_add_raw_packet_to_ring add_raw_packet_to_ring;

  /* Kernel consumer */
  char *kernel_consumer_options, *kernel_consumer_private;

  /* Userspace cluster (ZC) */
  struct cluster_referee *cluster_referee;
  cluster_client_type cluster_role;

#if defined(CONFIG_BCM_KF_SGS)
  struct sgs_pfr_socket sgs;
#endif
};

/* **************************************** */

typedef struct {
  struct net *net;

  /* /proc entry for ring module */
  struct proc_dir_entry *proc;
  struct proc_dir_entry *proc_dir;
  struct proc_dir_entry *proc_dev_dir; 
  struct proc_dir_entry *proc_stats_dir;

  struct list_head list;
} pf_ring_net;

/* **************************************** */

#define MAX_NUM_PATTERN   32

typedef struct {
  filtering_rule rule;

#ifdef CONFIG_TEXTSEARCH
  struct ts_config *pattern[MAX_NUM_PATTERN];
#endif
  struct list_head list;
} sw_filtering_rule_element;

typedef struct {
  hw_filtering_rule rule;
  struct list_head list;
} hw_filtering_rule_element;

/* **************************************** */

/* Exported functions - used by drivers */

int pf_ring_skb_ring_handler(struct sk_buff *skb,
			     u_int8_t recv_packet,
			     u_int8_t real_skb /* 1=real skb, 0=faked skb */,
			     int32_t channel_id,
			     u_int32_t num_rx_channels);

void pf_ring_zc_dev_handler(zc_dev_operation operation,
			    mem_ring_info *rx_info,
			    mem_ring_info *tx_info,
			    void          *rx_descr_packet_memory,
			    void          *tx_descr_packet_memory,
			    void          *phys_card_memory,
			    u_int          phys_card_memory_len,
			    u_int channel_id,
			    struct net_device *dev,
			    struct device *hwdev,
			    zc_dev_model device_model,
			    u_char *device_address,
			    wait_queue_head_t *packet_waitqueue,
			    u_int8_t *interrupt_received,
			    void *rx_adapter_ptr, void *tx_adapter_ptr,
			    zc_dev_wait_packet wait_packet_function_ptr,
			    zc_dev_notify dev_notify_function_ptr);

#endif /* __KERNEL__  */

/* *********************************** */

#endif /* __RING_H */
