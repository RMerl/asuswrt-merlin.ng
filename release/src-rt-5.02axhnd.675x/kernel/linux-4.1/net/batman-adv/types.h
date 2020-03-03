/* Copyright (C) 2007-2014 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NET_BATMAN_ADV_TYPES_H_
#define _NET_BATMAN_ADV_TYPES_H_

#include "packet.h"
#include "bitarray.h"
#include <linux/kernel.h>

#ifdef CONFIG_BATMAN_ADV_DAT

/**
 * batadv_dat_addr_t - it is the type used for all DHT addresses. If it is
 *  changed, BATADV_DAT_ADDR_MAX is changed as well.
 *
 * *Please be careful: batadv_dat_addr_t must be UNSIGNED*
 */
#define batadv_dat_addr_t uint16_t

#endif /* CONFIG_BATMAN_ADV_DAT */

/**
 * enum batadv_dhcp_recipient - dhcp destination
 * @BATADV_DHCP_NO: packet is not a dhcp message
 * @BATADV_DHCP_TO_SERVER: dhcp message is directed to a server
 * @BATADV_DHCP_TO_CLIENT: dhcp message is directed to a client
 */
enum batadv_dhcp_recipient {
	BATADV_DHCP_NO = 0,
	BATADV_DHCP_TO_SERVER,
	BATADV_DHCP_TO_CLIENT,
};

/**
 * BATADV_TT_REMOTE_MASK - bitmask selecting the flags that are sent over the
 *  wire only
 */
#define BATADV_TT_REMOTE_MASK	0x00FF

/**
 * BATADV_TT_SYNC_MASK - bitmask of the flags that need to be kept in sync
 *  among the nodes. These flags are used to compute the global/local CRC
 */
#define BATADV_TT_SYNC_MASK	0x00F0

/**
 * struct batadv_hard_iface_bat_iv - per hard interface B.A.T.M.A.N. IV data
 * @ogm_buff: buffer holding the OGM packet
 * @ogm_buff_len: length of the OGM packet buffer
 * @ogm_seqno: OGM sequence number - used to identify each OGM
 */
struct batadv_hard_iface_bat_iv {
	unsigned char *ogm_buff;
	int ogm_buff_len;
	atomic_t ogm_seqno;
};

/**
 * struct batadv_hard_iface - network device known to batman-adv
 * @list: list node for batadv_hardif_list
 * @if_num: identificator of the interface
 * @if_status: status of the interface for batman-adv
 * @net_dev: pointer to the net_device
 * @num_bcasts: number of payload re-broadcasts on this interface (ARQ)
 * @hardif_obj: kobject of the per interface sysfs "mesh" directory
 * @refcount: number of contexts the object is used
 * @batman_adv_ptype: packet type describing packets that should be processed by
 *  batman-adv for this interface
 * @soft_iface: the batman-adv interface which uses this network interface
 * @rcu: struct used for freeing in an RCU-safe manner
 * @bat_iv: BATMAN IV specific per hard interface data
 * @cleanup_work: work queue callback item for hard interface deinit
 * @debug_dir: dentry for nc subdir in batman-adv directory in debugfs
 */
struct batadv_hard_iface {
	struct list_head list;
	int16_t if_num;
	char if_status;
	struct net_device *net_dev;
	uint8_t num_bcasts;
	struct kobject *hardif_obj;
	atomic_t refcount;
	struct packet_type batman_adv_ptype;
	struct net_device *soft_iface;
	struct rcu_head rcu;
	struct batadv_hard_iface_bat_iv bat_iv;
	struct work_struct cleanup_work;
	struct dentry *debug_dir;
};

/**
 * struct batadv_orig_ifinfo - originator info per outgoing interface
 * @list: list node for orig_node::ifinfo_list
 * @if_outgoing: pointer to outgoing hard interface
 * @router: router that should be used to reach this originator
 * @last_real_seqno: last and best known sequence number
 * @last_ttl: ttl of last received packet
 * @batman_seqno_reset: time when the batman seqno window was reset
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 */
struct batadv_orig_ifinfo {
	struct hlist_node list;
	struct batadv_hard_iface *if_outgoing;
	struct batadv_neigh_node __rcu *router; /* rcu protected pointer */
	uint32_t last_real_seqno;
	uint8_t last_ttl;
	unsigned long batman_seqno_reset;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_frag_table_entry - head in the fragment buffer table
 * @head: head of list with fragments
 * @lock: lock to protect the list of fragments
 * @timestamp: time (jiffie) of last received fragment
 * @seqno: sequence number of the fragments in the list
 * @size: accumulated size of packets in list
 */
struct batadv_frag_table_entry {
	struct hlist_head head;
	spinlock_t lock; /* protects head */
	unsigned long timestamp;
	uint16_t seqno;
	uint16_t size;
};

/**
 * struct batadv_frag_list_entry - entry in a list of fragments
 * @list: list node information
 * @skb: fragment
 * @no: fragment number in the set
 */
struct batadv_frag_list_entry {
	struct hlist_node list;
	struct sk_buff *skb;
	uint8_t no;
};

/**
 * struct batadv_vlan_tt - VLAN specific TT attributes
 * @crc: CRC32 checksum of the entries belonging to this vlan
 * @num_entries: number of TT entries for this VLAN
 */
struct batadv_vlan_tt {
	uint32_t crc;
	atomic_t num_entries;
};

/**
 * struct batadv_orig_node_vlan - VLAN specific data per orig_node
 * @vid: the VLAN identifier
 * @tt: VLAN specific TT attributes
 * @list: list node for orig_node::vlan_list
 * @refcount: number of context where this object is currently in use
 * @rcu: struct used for freeing in a RCU-safe manner
 */
struct batadv_orig_node_vlan {
	unsigned short vid;
	struct batadv_vlan_tt tt;
	struct list_head list;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_orig_bat_iv - B.A.T.M.A.N. IV private orig_node members
 * @bcast_own: bitfield containing the number of our OGMs this orig_node
 *  rebroadcasted "back" to us (relative to last_real_seqno)
 * @bcast_own_sum: counted result of bcast_own
 * @ogm_cnt_lock: lock protecting bcast_own, bcast_own_sum,
 *  neigh_node->bat_iv.real_bits & neigh_node->bat_iv.real_packet_count
 */
struct batadv_orig_bat_iv {
	unsigned long *bcast_own;
	uint8_t *bcast_own_sum;
	/* ogm_cnt_lock protects: bcast_own, bcast_own_sum,
	 * neigh_node->bat_iv.real_bits & neigh_node->bat_iv.real_packet_count
	 */
	spinlock_t ogm_cnt_lock;
};

/**
 * struct batadv_orig_node - structure for orig_list maintaining nodes of mesh
 * @orig: originator ethernet address
 * @ifinfo_list: list for routers per outgoing interface
 * @last_bonding_candidate: pointer to last ifinfo of last used router
 * @batadv_dat_addr_t:  address of the orig node in the distributed hash
 * @last_seen: time when last packet from this node was received
 * @bcast_seqno_reset: time when the broadcast seqno window was reset
 * @mcast_handler_lock: synchronizes mcast-capability and -flag changes
 * @mcast_flags: multicast flags announced by the orig node
 * @mcast_want_all_unsnoop_node: a list node for the
 *  mcast.want_all_unsnoopables list
 * @mcast_want_all_ipv4_node: a list node for the mcast.want_all_ipv4 list
 * @mcast_want_all_ipv6_node: a list node for the mcast.want_all_ipv6 list
 * @capabilities: announced capabilities of this originator
 * @capa_initialized: bitfield to remember whether a capability was initialized
 * @last_ttvn: last seen translation table version number
 * @tt_buff: last tt changeset this node received from the orig node
 * @tt_buff_len: length of the last tt changeset this node received from the
 *  orig node
 * @tt_buff_lock: lock that protects tt_buff and tt_buff_len
 * @tt_lock: prevents from updating the table while reading it. Table update is
 *  made up by two operations (data structure update and metdata -CRC/TTVN-
 *  recalculation) and they have to be executed atomically in order to avoid
 *  another thread to read the table/metadata between those.
 * @bcast_bits: bitfield containing the info which payload broadcast originated
 *  from this orig node this host already has seen (relative to
 *  last_bcast_seqno)
 * @last_bcast_seqno: last broadcast sequence number received by this host
 * @neigh_list: list of potential next hop neighbor towards this orig node
 * @neigh_list_lock: lock protecting neigh_list and router
 * @hash_entry: hlist node for batadv_priv::orig_hash
 * @bat_priv: pointer to soft_iface this orig node belongs to
 * @bcast_seqno_lock: lock protecting bcast_bits & last_bcast_seqno
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 * @in_coding_list: list of nodes this orig can hear
 * @out_coding_list: list of nodes that can hear this orig
 * @in_coding_list_lock: protects in_coding_list
 * @out_coding_list_lock: protects out_coding_list
 * @fragments: array with heads for fragment chains
 * @vlan_list: a list of orig_node_vlan structs, one per VLAN served by the
 *  originator represented by this object
 * @vlan_list_lock: lock protecting vlan_list
 * @bat_iv: B.A.T.M.A.N. IV private structure
 */
struct batadv_orig_node {
	uint8_t orig[ETH_ALEN];
	struct hlist_head ifinfo_list;
	struct batadv_orig_ifinfo *last_bonding_candidate;
#ifdef CONFIG_BATMAN_ADV_DAT
	batadv_dat_addr_t dat_addr;
#endif
	unsigned long last_seen;
	unsigned long bcast_seqno_reset;
#ifdef CONFIG_BATMAN_ADV_MCAST
	/* synchronizes mcast tvlv specific orig changes */
	spinlock_t mcast_handler_lock;
	uint8_t mcast_flags;
	struct hlist_node mcast_want_all_unsnoopables_node;
	struct hlist_node mcast_want_all_ipv4_node;
	struct hlist_node mcast_want_all_ipv6_node;
#endif
	unsigned long capabilities;
	unsigned long capa_initialized;
	atomic_t last_ttvn;
	unsigned char *tt_buff;
	int16_t tt_buff_len;
	spinlock_t tt_buff_lock; /* protects tt_buff & tt_buff_len */
	/* prevents from changing the table while reading it */
	spinlock_t tt_lock;
	DECLARE_BITMAP(bcast_bits, BATADV_TQ_LOCAL_WINDOW_SIZE);
	uint32_t last_bcast_seqno;
	struct hlist_head neigh_list;
	/* neigh_list_lock protects: neigh_list and router */
	spinlock_t neigh_list_lock;
	struct hlist_node hash_entry;
	struct batadv_priv *bat_priv;
	/* bcast_seqno_lock protects: bcast_bits & last_bcast_seqno */
	spinlock_t bcast_seqno_lock;
	atomic_t refcount;
	struct rcu_head rcu;
#ifdef CONFIG_BATMAN_ADV_NC
	struct list_head in_coding_list;
	struct list_head out_coding_list;
	spinlock_t in_coding_list_lock; /* Protects in_coding_list */
	spinlock_t out_coding_list_lock; /* Protects out_coding_list */
#endif
	struct batadv_frag_table_entry fragments[BATADV_FRAG_BUFFER_COUNT];
	struct list_head vlan_list;
	spinlock_t vlan_list_lock; /* protects vlan_list */
	struct batadv_orig_bat_iv bat_iv;
};

/**
 * enum batadv_orig_capabilities - orig node capabilities
 * @BATADV_ORIG_CAPA_HAS_DAT: orig node has distributed arp table enabled
 * @BATADV_ORIG_CAPA_HAS_NC: orig node has network coding enabled
 * @BATADV_ORIG_CAPA_HAS_TT: orig node has tt capability
 * @BATADV_ORIG_CAPA_HAS_MCAST: orig node has some multicast capability
 *  (= orig node announces a tvlv of type BATADV_TVLV_MCAST)
 */
enum batadv_orig_capabilities {
	BATADV_ORIG_CAPA_HAS_DAT,
	BATADV_ORIG_CAPA_HAS_NC,
	BATADV_ORIG_CAPA_HAS_TT,
	BATADV_ORIG_CAPA_HAS_MCAST,
};

/**
 * struct batadv_gw_node - structure for orig nodes announcing gw capabilities
 * @list: list node for batadv_priv_gw::list
 * @orig_node: pointer to corresponding orig node
 * @bandwidth_down: advertised uplink download bandwidth
 * @bandwidth_up: advertised uplink upload bandwidth
 * @deleted: this struct is scheduled for deletion
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 */
struct batadv_gw_node {
	struct hlist_node list;
	struct batadv_orig_node *orig_node;
	uint32_t bandwidth_down;
	uint32_t bandwidth_up;
	unsigned long deleted;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_neigh_node - structure for single hops neighbors
 * @list: list node for batadv_orig_node::neigh_list
 * @orig_node: pointer to corresponding orig_node
 * @addr: the MAC address of the neighboring interface
 * @ifinfo_list: list for routing metrics per outgoing interface
 * @ifinfo_lock: lock protecting private ifinfo members and list
 * @if_incoming: pointer to incoming hard interface
 * @last_seen: when last packet via this neighbor was received
 * @last_ttl: last received ttl from this neigh node
 * @rcu: struct used for freeing in an RCU-safe manner
 * @bat_iv: B.A.T.M.A.N. IV private structure
 */
struct batadv_neigh_node {
	struct hlist_node list;
	struct batadv_orig_node *orig_node;
	uint8_t addr[ETH_ALEN];
	struct hlist_head ifinfo_list;
	spinlock_t ifinfo_lock;	/* protects ifinfo_list and its members */
	struct batadv_hard_iface *if_incoming;
	unsigned long last_seen;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_neigh_ifinfo_bat_iv - neighbor information per outgoing
 *  interface for BATMAN IV
 * @tq_recv: ring buffer of received TQ values from this neigh node
 * @tq_index: ring buffer index
 * @tq_avg: averaged tq of all tq values in the ring buffer (tq_recv)
 * @real_bits: bitfield containing the number of OGMs received from this neigh
 *  node (relative to orig_node->last_real_seqno)
 * @real_packet_count: counted result of real_bits
 */
struct batadv_neigh_ifinfo_bat_iv {
	uint8_t tq_recv[BATADV_TQ_GLOBAL_WINDOW_SIZE];
	uint8_t tq_index;
	uint8_t tq_avg;
	DECLARE_BITMAP(real_bits, BATADV_TQ_LOCAL_WINDOW_SIZE);
	uint8_t real_packet_count;
};

/**
 * struct batadv_neigh_ifinfo - neighbor information per outgoing interface
 * @list: list node for batadv_neigh_node::ifinfo_list
 * @if_outgoing: pointer to outgoing hard interface
 * @bat_iv: B.A.T.M.A.N. IV private structure
 * @last_ttl: last received ttl from this neigh node
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in a RCU-safe manner
 */
struct batadv_neigh_ifinfo {
	struct hlist_node list;
	struct batadv_hard_iface *if_outgoing;
	struct batadv_neigh_ifinfo_bat_iv bat_iv;
	uint8_t last_ttl;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_bcast_duplist_entry - structure for LAN broadcast suppression
 * @orig[ETH_ALEN]: mac address of orig node orginating the broadcast
 * @crc: crc32 checksum of broadcast payload
 * @entrytime: time when the broadcast packet was received
 */
#ifdef CONFIG_BATMAN_ADV_BLA
struct batadv_bcast_duplist_entry {
	uint8_t orig[ETH_ALEN];
	__be32 crc;
	unsigned long entrytime;
};
#endif

/**
 * enum batadv_counters - indices for traffic counters
 * @BATADV_CNT_TX: transmitted payload traffic packet counter
 * @BATADV_CNT_TX_BYTES: transmitted payload traffic bytes counter
 * @BATADV_CNT_TX_DROPPED: dropped transmission payload traffic packet counter
 * @BATADV_CNT_RX: received payload traffic packet counter
 * @BATADV_CNT_RX_BYTES: received payload traffic bytes counter
 * @BATADV_CNT_FORWARD: forwarded payload traffic packet counter
 * @BATADV_CNT_FORWARD_BYTES: forwarded payload traffic bytes counter
 * @BATADV_CNT_MGMT_TX: transmitted routing protocol traffic packet counter
 * @BATADV_CNT_MGMT_TX_BYTES: transmitted routing protocol traffic bytes counter
 * @BATADV_CNT_MGMT_RX: received routing protocol traffic packet counter
 * @BATADV_CNT_MGMT_RX_BYTES: received routing protocol traffic bytes counter
 * @BATADV_CNT_FRAG_TX: transmitted fragment traffic packet counter
 * @BATADV_CNT_FRAG_TX_BYTES: transmitted fragment traffic bytes counter
 * @BATADV_CNT_FRAG_RX: received fragment traffic packet counter
 * @BATADV_CNT_FRAG_RX_BYTES: received fragment traffic bytes counter
 * @BATADV_CNT_FRAG_FWD: forwarded fragment traffic packet counter
 * @BATADV_CNT_FRAG_FWD_BYTES: forwarded fragment traffic bytes counter
 * @BATADV_CNT_TT_REQUEST_TX: transmitted tt req traffic packet counter
 * @BATADV_CNT_TT_REQUEST_RX: received tt req traffic packet counter
 * @BATADV_CNT_TT_RESPONSE_TX: transmitted tt resp traffic packet counter
 * @BATADV_CNT_TT_RESPONSE_RX: received tt resp traffic packet counter
 * @BATADV_CNT_TT_ROAM_ADV_TX: transmitted tt roam traffic packet counter
 * @BATADV_CNT_TT_ROAM_ADV_RX: received tt roam traffic packet counter
 * @BATADV_CNT_DAT_GET_TX: transmitted dht GET traffic packet counter
 * @BATADV_CNT_DAT_GET_RX: received dht GET traffic packet counter
 * @BATADV_CNT_DAT_PUT_TX: transmitted dht PUT traffic packet counter
 * @BATADV_CNT_DAT_PUT_RX: received dht PUT traffic packet counter
 * @BATADV_CNT_DAT_CACHED_REPLY_TX: transmitted dat cache reply traffic packet
 *  counter
 * @BATADV_CNT_NC_CODE: transmitted nc-combined traffic packet counter
 * @BATADV_CNT_NC_CODE_BYTES: transmitted nc-combined traffic bytes counter
 * @BATADV_CNT_NC_RECODE: transmitted nc-recombined traffic packet counter
 * @BATADV_CNT_NC_RECODE_BYTES: transmitted nc-recombined traffic bytes counter
 * @BATADV_CNT_NC_BUFFER: counter for packets buffered for later nc decoding
 * @BATADV_CNT_NC_DECODE: received and nc-decoded traffic packet counter
 * @BATADV_CNT_NC_DECODE_BYTES: received and nc-decoded traffic bytes counter
 * @BATADV_CNT_NC_DECODE_FAILED: received and decode-failed traffic packet
 *  counter
 * @BATADV_CNT_NC_SNIFFED: counter for nc-decoded packets received in promisc
 *  mode.
 * @BATADV_CNT_NUM: number of traffic counters
 */
enum batadv_counters {
	BATADV_CNT_TX,
	BATADV_CNT_TX_BYTES,
	BATADV_CNT_TX_DROPPED,
	BATADV_CNT_RX,
	BATADV_CNT_RX_BYTES,
	BATADV_CNT_FORWARD,
	BATADV_CNT_FORWARD_BYTES,
	BATADV_CNT_MGMT_TX,
	BATADV_CNT_MGMT_TX_BYTES,
	BATADV_CNT_MGMT_RX,
	BATADV_CNT_MGMT_RX_BYTES,
	BATADV_CNT_FRAG_TX,
	BATADV_CNT_FRAG_TX_BYTES,
	BATADV_CNT_FRAG_RX,
	BATADV_CNT_FRAG_RX_BYTES,
	BATADV_CNT_FRAG_FWD,
	BATADV_CNT_FRAG_FWD_BYTES,
	BATADV_CNT_TT_REQUEST_TX,
	BATADV_CNT_TT_REQUEST_RX,
	BATADV_CNT_TT_RESPONSE_TX,
	BATADV_CNT_TT_RESPONSE_RX,
	BATADV_CNT_TT_ROAM_ADV_TX,
	BATADV_CNT_TT_ROAM_ADV_RX,
#ifdef CONFIG_BATMAN_ADV_DAT
	BATADV_CNT_DAT_GET_TX,
	BATADV_CNT_DAT_GET_RX,
	BATADV_CNT_DAT_PUT_TX,
	BATADV_CNT_DAT_PUT_RX,
	BATADV_CNT_DAT_CACHED_REPLY_TX,
#endif
#ifdef CONFIG_BATMAN_ADV_NC
	BATADV_CNT_NC_CODE,
	BATADV_CNT_NC_CODE_BYTES,
	BATADV_CNT_NC_RECODE,
	BATADV_CNT_NC_RECODE_BYTES,
	BATADV_CNT_NC_BUFFER,
	BATADV_CNT_NC_DECODE,
	BATADV_CNT_NC_DECODE_BYTES,
	BATADV_CNT_NC_DECODE_FAILED,
	BATADV_CNT_NC_SNIFFED,
#endif
	BATADV_CNT_NUM,
};

/**
 * struct batadv_priv_tt - per mesh interface translation table data
 * @vn: translation table version number
 * @ogm_append_cnt: counter of number of OGMs containing the local tt diff
 * @local_changes: changes registered in an originator interval
 * @changes_list: tracks tt local changes within an originator interval
 * @local_hash: local translation table hash table
 * @global_hash: global translation table hash table
 * @req_list: list of pending & unanswered tt_requests
 * @roam_list: list of the last roaming events of each client limiting the
 *  number of roaming events to avoid route flapping
 * @changes_list_lock: lock protecting changes_list
 * @req_list_lock: lock protecting req_list
 * @roam_list_lock: lock protecting roam_list
 * @last_changeset: last tt changeset this host has generated
 * @last_changeset_len: length of last tt changeset this host has generated
 * @last_changeset_lock: lock protecting last_changeset & last_changeset_len
 * @commit_lock: prevents from executing a local TT commit while reading the
 *  local table. The local TT commit is made up by two operations (data
 *  structure update and metdata -CRC/TTVN- recalculation) and they have to be
 *  executed atomically in order to avoid another thread to read the
 *  table/metadata between those.
 * @work: work queue callback item for translation table purging
 */
struct batadv_priv_tt {
	atomic_t vn;
	atomic_t ogm_append_cnt;
	atomic_t local_changes;
	struct list_head changes_list;
	struct batadv_hashtable *local_hash;
	struct batadv_hashtable *global_hash;
	struct list_head req_list;
	struct list_head roam_list;
	spinlock_t changes_list_lock; /* protects changes */
	spinlock_t req_list_lock; /* protects req_list */
	spinlock_t roam_list_lock; /* protects roam_list */
	unsigned char *last_changeset;
	int16_t last_changeset_len;
	/* protects last_changeset & last_changeset_len */
	spinlock_t last_changeset_lock;
	/* prevents from executing a commit while reading the table */
	spinlock_t commit_lock;
	struct delayed_work work;
};

/**
 * struct batadv_priv_bla - per mesh interface bridge loope avoidance data
 * @num_requests; number of bla requests in flight
 * @claim_hash: hash table containing mesh nodes this host has claimed
 * @backbone_hash: hash table containing all detected backbone gateways
 * @bcast_duplist: recently received broadcast packets array (for broadcast
 *  duplicate suppression)
 * @bcast_duplist_curr: index of last broadcast packet added to bcast_duplist
 * @bcast_duplist_lock: lock protecting bcast_duplist & bcast_duplist_curr
 * @claim_dest: local claim data (e.g. claim group)
 * @work: work queue callback item for cleanups & bla announcements
 */
#ifdef CONFIG_BATMAN_ADV_BLA
struct batadv_priv_bla {
	atomic_t num_requests;
	struct batadv_hashtable *claim_hash;
	struct batadv_hashtable *backbone_hash;
	struct batadv_bcast_duplist_entry bcast_duplist[BATADV_DUPLIST_SIZE];
	int bcast_duplist_curr;
	/* protects bcast_duplist & bcast_duplist_curr */
	spinlock_t bcast_duplist_lock;
	struct batadv_bla_claim_dst claim_dest;
	struct delayed_work work;
};
#endif

/**
 * struct batadv_priv_debug_log - debug logging data
 * @log_buff: buffer holding the logs (ring bufer)
 * @log_start: index of next character to read
 * @log_end: index of next character to write
 * @lock: lock protecting log_buff, log_start & log_end
 * @queue_wait: log reader's wait queue
 */
#ifdef CONFIG_BATMAN_ADV_DEBUG
struct batadv_priv_debug_log {
	char log_buff[BATADV_LOG_BUF_LEN];
	unsigned long log_start;
	unsigned long log_end;
	spinlock_t lock; /* protects log_buff, log_start and log_end */
	wait_queue_head_t queue_wait;
};
#endif

/**
 * struct batadv_priv_gw - per mesh interface gateway data
 * @list: list of available gateway nodes
 * @list_lock: lock protecting gw_list & curr_gw
 * @curr_gw: pointer to currently selected gateway node
 * @bandwidth_down: advertised uplink download bandwidth (if gw_mode server)
 * @bandwidth_up: advertised uplink upload bandwidth (if gw_mode server)
 * @reselect: bool indicating a gateway re-selection is in progress
 */
struct batadv_priv_gw {
	struct hlist_head list;
	spinlock_t list_lock; /* protects gw_list & curr_gw */
	struct batadv_gw_node __rcu *curr_gw;  /* rcu protected pointer */
	atomic_t bandwidth_down;
	atomic_t bandwidth_up;
	atomic_t reselect;
};

/**
 * struct batadv_priv_tvlv - per mesh interface tvlv data
 * @container_list: list of registered tvlv containers to be sent with each OGM
 * @handler_list: list of the various tvlv content handlers
 * @container_list_lock: protects tvlv container list access
 * @handler_list_lock: protects handler list access
 */
struct batadv_priv_tvlv {
	struct hlist_head container_list;
	struct hlist_head handler_list;
	spinlock_t container_list_lock; /* protects container_list */
	spinlock_t handler_list_lock; /* protects handler_list */
};

/**
 * struct batadv_priv_dat - per mesh interface DAT private data
 * @addr: node DAT address
 * @hash: hashtable representing the local ARP cache
 * @work: work queue callback item for cache purging
 */
#ifdef CONFIG_BATMAN_ADV_DAT
struct batadv_priv_dat {
	batadv_dat_addr_t addr;
	struct batadv_hashtable *hash;
	struct delayed_work work;
};
#endif

#ifdef CONFIG_BATMAN_ADV_MCAST
/**
 * struct batadv_priv_mcast - per mesh interface mcast data
 * @mla_list: list of multicast addresses we are currently announcing via TT
 * @want_all_unsnoopables_list: a list of orig_nodes wanting all unsnoopable
 *  multicast traffic
 * @want_all_ipv4_list: a list of orig_nodes wanting all IPv4 multicast traffic
 * @want_all_ipv6_list: a list of orig_nodes wanting all IPv6 multicast traffic
 * @flags: the flags we have last sent in our mcast tvlv
 * @enabled: whether the multicast tvlv is currently enabled
 * @num_disabled: number of nodes that have no mcast tvlv
 * @num_want_all_unsnoopables: number of nodes wanting unsnoopable IP traffic
 * @num_want_all_ipv4: counter for items in want_all_ipv4_list
 * @num_want_all_ipv6: counter for items in want_all_ipv6_list
 * @want_lists_lock: lock for protecting modifications to mcast want lists
 *  (traversals are rcu-locked)
 */
struct batadv_priv_mcast {
	struct hlist_head mla_list;
	struct hlist_head want_all_unsnoopables_list;
	struct hlist_head want_all_ipv4_list;
	struct hlist_head want_all_ipv6_list;
	uint8_t flags;
	bool enabled;
	atomic_t num_disabled;
	atomic_t num_want_all_unsnoopables;
	atomic_t num_want_all_ipv4;
	atomic_t num_want_all_ipv6;
	/* protects want_all_{unsnoopables,ipv4,ipv6}_list */
	spinlock_t want_lists_lock;
};
#endif

/**
 * struct batadv_priv_nc - per mesh interface network coding private data
 * @work: work queue callback item for cleanup
 * @debug_dir: dentry for nc subdir in batman-adv directory in debugfs
 * @min_tq: only consider neighbors for encoding if neigh_tq > min_tq
 * @max_fwd_delay: maximum packet forward delay to allow coding of packets
 * @max_buffer_time: buffer time for sniffed packets used to decoding
 * @timestamp_fwd_flush: timestamp of last forward packet queue flush
 * @timestamp_sniffed_purge: timestamp of last sniffed packet queue purge
 * @coding_hash: Hash table used to buffer skbs while waiting for another
 *  incoming skb to code it with. Skbs are added to the buffer just before being
 *  forwarded in routing.c
 * @decoding_hash: Hash table used to buffer skbs that might be needed to decode
 *  a received coded skb. The buffer is used for 1) skbs arriving on the
 *  soft-interface; 2) skbs overheard on the hard-interface; and 3) skbs
 *  forwarded by batman-adv.
 */
struct batadv_priv_nc {
	struct delayed_work work;
	struct dentry *debug_dir;
	u8 min_tq;
	u32 max_fwd_delay;
	u32 max_buffer_time;
	unsigned long timestamp_fwd_flush;
	unsigned long timestamp_sniffed_purge;
	struct batadv_hashtable *coding_hash;
	struct batadv_hashtable *decoding_hash;
};

/**
 * struct batadv_softif_vlan - per VLAN attributes set
 * @bat_priv: pointer to the mesh object
 * @vid: VLAN identifier
 * @kobj: kobject for sysfs vlan subdirectory
 * @ap_isolation: AP isolation state
 * @tt: TT private attributes (VLAN specific)
 * @list: list node for bat_priv::softif_vlan_list
 * @refcount: number of context where this object is currently in use
 * @rcu: struct used for freeing in a RCU-safe manner
 */
struct batadv_softif_vlan {
	struct batadv_priv *bat_priv;
	unsigned short vid;
	struct kobject *kobj;
	atomic_t ap_isolation;		/* boolean */
	struct batadv_vlan_tt tt;
	struct hlist_node list;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_priv - per mesh interface data
 * @mesh_state: current status of the mesh (inactive/active/deactivating)
 * @soft_iface: net device which holds this struct as private data
 * @stats: structure holding the data for the ndo_get_stats() call
 * @bat_counters: mesh internal traffic statistic counters (see batadv_counters)
 * @aggregated_ogms: bool indicating whether OGM aggregation is enabled
 * @bonding: bool indicating whether traffic bonding is enabled
 * @fragmentation: bool indicating whether traffic fragmentation is enabled
 * @packet_size_max: max packet size that can be transmitted via
 *  multiple fragmented skbs or a single frame if fragmentation is disabled
 * @frag_seqno: incremental counter to identify chains of egress fragments
 * @bridge_loop_avoidance: bool indicating whether bridge loop avoidance is
 *  enabled
 * @distributed_arp_table: bool indicating whether distributed ARP table is
 *  enabled
 * @multicast_mode: Enable or disable multicast optimizations on this node's
 *  sender/originating side
 * @gw_mode: gateway operation: off, client or server (see batadv_gw_modes)
 * @gw_sel_class: gateway selection class (applies if gw_mode client)
 * @orig_interval: OGM broadcast interval in milliseconds
 * @hop_penalty: penalty which will be applied to an OGM's tq-field on every hop
 * @log_level: configured log level (see batadv_dbg_level)
 * @bcast_seqno: last sent broadcast packet sequence number
 * @bcast_queue_left: number of remaining buffered broadcast packet slots
 * @batman_queue_left: number of remaining OGM packet slots
 * @num_ifaces: number of interfaces assigned to this mesh interface
 * @mesh_obj: kobject for sysfs mesh subdirectory
 * @debug_dir: dentry for debugfs batman-adv subdirectory
 * @forw_bat_list: list of aggregated OGMs that will be forwarded
 * @forw_bcast_list: list of broadcast packets that will be rebroadcasted
 * @orig_hash: hash table containing mesh participants (orig nodes)
 * @forw_bat_list_lock: lock protecting forw_bat_list
 * @forw_bcast_list_lock: lock protecting forw_bcast_list
 * @orig_work: work queue callback item for orig node purging
 * @cleanup_work: work queue callback item for soft interface deinit
 * @primary_if: one of the hard interfaces assigned to this mesh interface
 *  becomes the primary interface
 * @bat_algo_ops: routing algorithm used by this mesh interface
 * @softif_vlan_list: a list of softif_vlan structs, one per VLAN created on top
 *  of the mesh interface represented by this object
 * @softif_vlan_list_lock: lock protecting softif_vlan_list
 * @bla: bridge loope avoidance data
 * @debug_log: holding debug logging relevant data
 * @gw: gateway data
 * @tt: translation table data
 * @tvlv: type-version-length-value data
 * @dat: distributed arp table data
 * @mcast: multicast data
 * @network_coding: bool indicating whether network coding is enabled
 * @batadv_priv_nc: network coding data
 */
struct batadv_priv {
	atomic_t mesh_state;
	struct net_device *soft_iface;
	struct net_device_stats stats;
	uint64_t __percpu *bat_counters; /* Per cpu counters */
	atomic_t aggregated_ogms;
	atomic_t bonding;
	atomic_t fragmentation;
	atomic_t packet_size_max;
	atomic_t frag_seqno;
#ifdef CONFIG_BATMAN_ADV_BLA
	atomic_t bridge_loop_avoidance;
#endif
#ifdef CONFIG_BATMAN_ADV_DAT
	atomic_t distributed_arp_table;
#endif
#ifdef CONFIG_BATMAN_ADV_MCAST
	atomic_t multicast_mode;
#endif
	atomic_t gw_mode;
	atomic_t gw_sel_class;
	atomic_t orig_interval;
	atomic_t hop_penalty;
#ifdef CONFIG_BATMAN_ADV_DEBUG
	atomic_t log_level;
#endif
	uint32_t isolation_mark;
	uint32_t isolation_mark_mask;
	atomic_t bcast_seqno;
	atomic_t bcast_queue_left;
	atomic_t batman_queue_left;
	char num_ifaces;
	struct kobject *mesh_obj;
	struct dentry *debug_dir;
	struct hlist_head forw_bat_list;
	struct hlist_head forw_bcast_list;
	struct batadv_hashtable *orig_hash;
	spinlock_t forw_bat_list_lock; /* protects forw_bat_list */
	spinlock_t forw_bcast_list_lock; /* protects forw_bcast_list */
	struct delayed_work orig_work;
	struct work_struct cleanup_work;
	struct batadv_hard_iface __rcu *primary_if;  /* rcu protected pointer */
	struct batadv_algo_ops *bat_algo_ops;
	struct hlist_head softif_vlan_list;
	spinlock_t softif_vlan_list_lock; /* protects softif_vlan_list */
#ifdef CONFIG_BATMAN_ADV_BLA
	struct batadv_priv_bla bla;
#endif
#ifdef CONFIG_BATMAN_ADV_DEBUG
	struct batadv_priv_debug_log *debug_log;
#endif
	struct batadv_priv_gw gw;
	struct batadv_priv_tt tt;
	struct batadv_priv_tvlv tvlv;
#ifdef CONFIG_BATMAN_ADV_DAT
	struct batadv_priv_dat dat;
#endif
#ifdef CONFIG_BATMAN_ADV_MCAST
	struct batadv_priv_mcast mcast;
#endif
#ifdef CONFIG_BATMAN_ADV_NC
	atomic_t network_coding;
	struct batadv_priv_nc nc;
#endif /* CONFIG_BATMAN_ADV_NC */
};

/**
 * struct batadv_socket_client - layer2 icmp socket client data
 * @queue_list: packet queue for packets destined for this socket client
 * @queue_len: number of packets in the packet queue (queue_list)
 * @index: socket client's index in the batadv_socket_client_hash
 * @lock: lock protecting queue_list, queue_len & index
 * @queue_wait: socket client's wait queue
 * @bat_priv: pointer to soft_iface this client belongs to
 */
struct batadv_socket_client {
	struct list_head queue_list;
	unsigned int queue_len;
	unsigned char index;
	spinlock_t lock; /* protects queue_list, queue_len & index */
	wait_queue_head_t queue_wait;
	struct batadv_priv *bat_priv;
};

/**
 * struct batadv_socket_packet - layer2 icmp packet for socket client
 * @list: list node for batadv_socket_client::queue_list
 * @icmp_len: size of the layer2 icmp packet
 * @icmp_packet: layer2 icmp packet
 */
struct batadv_socket_packet {
	struct list_head list;
	size_t icmp_len;
	uint8_t icmp_packet[BATADV_ICMP_MAX_PACKET_SIZE];
};

/**
 * struct batadv_bla_backbone_gw - batman-adv gateway bridged into the LAN
 * @orig: originator address of backbone node (mac address of primary iface)
 * @vid: vlan id this gateway was detected on
 * @hash_entry: hlist node for batadv_priv_bla::backbone_hash
 * @bat_priv: pointer to soft_iface this backbone gateway belongs to
 * @lasttime: last time we heard of this backbone gw
 * @wait_periods: grace time for bridge forward delays and bla group forming at
 *  bootup phase - no bcast traffic is formwared until it has elapsed
 * @request_sent: if this bool is set to true we are out of sync with this
 *  backbone gateway - no bcast traffic is formwared until the situation was
 *  resolved
 * @crc: crc16 checksum over all claims
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 */
#ifdef CONFIG_BATMAN_ADV_BLA
struct batadv_bla_backbone_gw {
	uint8_t orig[ETH_ALEN];
	unsigned short vid;
	struct hlist_node hash_entry;
	struct batadv_priv *bat_priv;
	unsigned long lasttime;
	atomic_t wait_periods;
	atomic_t request_sent;
	uint16_t crc;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_bla_claim - claimed non-mesh client structure
 * @addr: mac address of claimed non-mesh client
 * @vid: vlan id this client was detected on
 * @batadv_bla_backbone_gw: pointer to backbone gw claiming this client
 * @lasttime: last time we heard of claim (locals only)
 * @hash_entry: hlist node for batadv_priv_bla::claim_hash
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 */
struct batadv_bla_claim {
	uint8_t addr[ETH_ALEN];
	unsigned short vid;
	struct batadv_bla_backbone_gw *backbone_gw;
	unsigned long lasttime;
	struct hlist_node hash_entry;
	struct rcu_head rcu;
	atomic_t refcount;
};
#endif

/**
 * struct batadv_tt_common_entry - tt local & tt global common data
 * @addr: mac address of non-mesh client
 * @vid: VLAN identifier
 * @hash_entry: hlist node for batadv_priv_tt::local_hash or for
 *  batadv_priv_tt::global_hash
 * @flags: various state handling flags (see batadv_tt_client_flags)
 * @added_at: timestamp used for purging stale tt common entries
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 */
struct batadv_tt_common_entry {
	uint8_t addr[ETH_ALEN];
	unsigned short vid;
	struct hlist_node hash_entry;
	uint16_t flags;
	unsigned long added_at;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_tt_local_entry - translation table local entry data
 * @common: general translation table data
 * @last_seen: timestamp used for purging stale tt local entries
 */
struct batadv_tt_local_entry {
	struct batadv_tt_common_entry common;
	unsigned long last_seen;
};

/**
 * struct batadv_tt_global_entry - translation table global entry data
 * @common: general translation table data
 * @orig_list: list of orig nodes announcing this non-mesh client
 * @orig_list_count: number of items in the orig_list
 * @list_lock: lock protecting orig_list
 * @roam_at: time at which TT_GLOBAL_ROAM was set
 */
struct batadv_tt_global_entry {
	struct batadv_tt_common_entry common;
	struct hlist_head orig_list;
	atomic_t orig_list_count;
	spinlock_t list_lock;	/* protects orig_list */
	unsigned long roam_at;
};

/**
 * struct batadv_tt_orig_list_entry - orig node announcing a non-mesh client
 * @orig_node: pointer to orig node announcing this non-mesh client
 * @ttvn: translation table version number which added the non-mesh client
 * @list: list node for batadv_tt_global_entry::orig_list
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 */
struct batadv_tt_orig_list_entry {
	struct batadv_orig_node *orig_node;
	uint8_t ttvn;
	struct hlist_node list;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_tt_change_node - structure for tt changes occurred
 * @list: list node for batadv_priv_tt::changes_list
 * @change: holds the actual translation table diff data
 */
struct batadv_tt_change_node {
	struct list_head list;
	struct batadv_tvlv_tt_change change;
};

/**
 * struct batadv_tt_req_node - data to keep track of the tt requests in flight
 * @addr: mac address address of the originator this request was sent to
 * @issued_at: timestamp used for purging stale tt requests
 * @list: list node for batadv_priv_tt::req_list
 */
struct batadv_tt_req_node {
	uint8_t addr[ETH_ALEN];
	unsigned long issued_at;
	struct list_head list;
};

/**
 * struct batadv_tt_roam_node - roaming client data
 * @addr: mac address of the client in the roaming phase
 * @counter: number of allowed roaming events per client within a single
 *  OGM interval (changes are committed with each OGM)
 * @first_time: timestamp used for purging stale roaming node entries
 * @list: list node for batadv_priv_tt::roam_list
 */
struct batadv_tt_roam_node {
	uint8_t addr[ETH_ALEN];
	atomic_t counter;
	unsigned long first_time;
	struct list_head list;
};

/**
 * struct batadv_nc_node - network coding node
 * @list: next and prev pointer for the list handling
 * @addr: the node's mac address
 * @refcount: number of contexts the object is used by
 * @rcu: struct used for freeing in an RCU-safe manner
 * @orig_node: pointer to corresponding orig node struct
 * @last_seen: timestamp of last ogm received from this node
 */
struct batadv_nc_node {
	struct list_head list;
	uint8_t addr[ETH_ALEN];
	atomic_t refcount;
	struct rcu_head rcu;
	struct batadv_orig_node *orig_node;
	unsigned long last_seen;
};

/**
 * struct batadv_nc_path - network coding path
 * @hash_entry: next and prev pointer for the list handling
 * @rcu: struct used for freeing in an RCU-safe manner
 * @refcount: number of contexts the object is used by
 * @packet_list: list of buffered packets for this path
 * @packet_list_lock: access lock for packet list
 * @next_hop: next hop (destination) of path
 * @prev_hop: previous hop (source) of path
 * @last_valid: timestamp for last validation of path
 */
struct batadv_nc_path {
	struct hlist_node hash_entry;
	struct rcu_head rcu;
	atomic_t refcount;
	struct list_head packet_list;
	spinlock_t packet_list_lock; /* Protects packet_list */
	uint8_t next_hop[ETH_ALEN];
	uint8_t prev_hop[ETH_ALEN];
	unsigned long last_valid;
};

/**
 * struct batadv_nc_packet - network coding packet used when coding and
 *  decoding packets
 * @list: next and prev pointer for the list handling
 * @packet_id: crc32 checksum of skb data
 * @timestamp: field containing the info when the packet was added to path
 * @neigh_node: pointer to original next hop neighbor of skb
 * @skb: skb which can be encoded or used for decoding
 * @nc_path: pointer to path this nc packet is attached to
 */
struct batadv_nc_packet {
	struct list_head list;
	__be32 packet_id;
	unsigned long timestamp;
	struct batadv_neigh_node *neigh_node;
	struct sk_buff *skb;
	struct batadv_nc_path *nc_path;
};

/**
 * struct batadv_skb_cb - control buffer structure used to store private data
 *  relevant to batman-adv in the skb->cb buffer in skbs.
 * @decoded: Marks a skb as decoded, which is checked when searching for coding
 *  opportunities in network-coding.c
 */
struct batadv_skb_cb {
	bool decoded;
};

/**
 * struct batadv_forw_packet - structure for bcast packets to be sent/forwarded
 * @list: list node for batadv_socket_client::queue_list
 * @send_time: execution time for delayed_work (packet sending)
 * @own: bool for locally generated packets (local OGMs are re-scheduled after
 *  sending)
 * @skb: bcast packet's skb buffer
 * @packet_len: size of aggregated OGM packet inside the skb buffer
 * @direct_link_flags: direct link flags for aggregated OGM packets
 * @num_packets: counter for bcast packet retransmission
 * @delayed_work: work queue callback item for packet sending
 * @if_incoming: pointer to incoming hard-iface or primary iface if
 *  locally generated packet
 * @if_outgoing: packet where the packet should be sent to, or NULL if
 *  unspecified
 */
struct batadv_forw_packet {
	struct hlist_node list;
	unsigned long send_time;
	uint8_t own;
	struct sk_buff *skb;
	uint16_t packet_len;
	uint32_t direct_link_flags;
	uint8_t num_packets;
	struct delayed_work delayed_work;
	struct batadv_hard_iface *if_incoming;
	struct batadv_hard_iface *if_outgoing;
};

/**
 * struct batadv_algo_ops - mesh algorithm callbacks
 * @list: list node for the batadv_algo_list
 * @name: name of the algorithm
 * @bat_iface_enable: init routing info when hard-interface is enabled
 * @bat_iface_disable: de-init routing info when hard-interface is disabled
 * @bat_iface_update_mac: (re-)init mac addresses of the protocol information
 *  belonging to this hard-interface
 * @bat_primary_iface_set: called when primary interface is selected / changed
 * @bat_ogm_schedule: prepare a new outgoing OGM for the send queue
 * @bat_ogm_emit: send scheduled OGM
 * @bat_neigh_cmp: compare the metrics of two neighbors for their respective
 *  outgoing interfaces
 * @bat_neigh_is_equiv_or_better: check if neigh1 is equally good or better
 *  than neigh2 for their respective outgoing interface from the metric
 *  prospective
 * @bat_orig_print: print the originator table (optional)
 * @bat_orig_free: free the resources allocated by the routing algorithm for an
 *  orig_node object
 * @bat_orig_add_if: ask the routing algorithm to apply the needed changes to
 *  the orig_node due to a new hard-interface being added into the mesh
 * @bat_orig_del_if: ask the routing algorithm to apply the needed changes to
 *  the orig_node due to an hard-interface being removed from the mesh
 */
struct batadv_algo_ops {
	struct hlist_node list;
	char *name;
	int (*bat_iface_enable)(struct batadv_hard_iface *hard_iface);
	void (*bat_iface_disable)(struct batadv_hard_iface *hard_iface);
	void (*bat_iface_update_mac)(struct batadv_hard_iface *hard_iface);
	void (*bat_primary_iface_set)(struct batadv_hard_iface *hard_iface);
	void (*bat_ogm_schedule)(struct batadv_hard_iface *hard_iface);
	void (*bat_ogm_emit)(struct batadv_forw_packet *forw_packet);
	int (*bat_neigh_cmp)(struct batadv_neigh_node *neigh1,
			     struct batadv_hard_iface *if_outgoing1,
			     struct batadv_neigh_node *neigh2,
			     struct batadv_hard_iface *if_outgoing2);
	bool (*bat_neigh_is_equiv_or_better)
		(struct batadv_neigh_node *neigh1,
		 struct batadv_hard_iface *if_outgoing1,
		 struct batadv_neigh_node *neigh2,
		 struct batadv_hard_iface *if_outgoing2);
	/* orig_node handling API */
	void (*bat_orig_print)(struct batadv_priv *priv, struct seq_file *seq,
			       struct batadv_hard_iface *hard_iface);
	void (*bat_orig_free)(struct batadv_orig_node *orig_node);
	int (*bat_orig_add_if)(struct batadv_orig_node *orig_node,
			       int max_if_num);
	int (*bat_orig_del_if)(struct batadv_orig_node *orig_node,
			       int max_if_num, int del_if_num);
};

/**
 * struct batadv_dat_entry - it is a single entry of batman-adv ARP backend. It
 * is used to stored ARP entries needed for the global DAT cache
 * @ip: the IPv4 corresponding to this DAT/ARP entry
 * @mac_addr: the MAC address associated to the stored IPv4
 * @vid: the vlan ID associated to this entry
 * @last_update: time in jiffies when this entry was refreshed last time
 * @hash_entry: hlist node for batadv_priv_dat::hash
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 */
struct batadv_dat_entry {
	__be32 ip;
	uint8_t mac_addr[ETH_ALEN];
	unsigned short vid;
	unsigned long last_update;
	struct hlist_node hash_entry;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * struct batadv_hw_addr - a list entry for a MAC address
 * @list: list node for the linking of entries
 * @addr: the MAC address of this list entry
 */
struct batadv_hw_addr {
	struct hlist_node list;
	unsigned char addr[ETH_ALEN];
};

/**
 * struct batadv_dat_candidate - candidate destination for DAT operations
 * @type: the type of the selected candidate. It can one of the following:
 *	  - BATADV_DAT_CANDIDATE_NOT_FOUND
 *	  - BATADV_DAT_CANDIDATE_ORIG
 * @orig_node: if type is BATADV_DAT_CANDIDATE_ORIG this field points to the
 *	       corresponding originator node structure
 */
struct batadv_dat_candidate {
	int type;
	struct batadv_orig_node *orig_node;
};

/**
 * struct batadv_tvlv_container - container for tvlv appended to OGMs
 * @list: hlist node for batadv_priv_tvlv::container_list
 * @tvlv_hdr: tvlv header information needed to construct the tvlv
 * @value_len: length of the buffer following this struct which contains
 *  the actual tvlv payload
 * @refcount: number of contexts the object is used
 */
struct batadv_tvlv_container {
	struct hlist_node list;
	struct batadv_tvlv_hdr tvlv_hdr;
	atomic_t refcount;
};

/**
 * struct batadv_tvlv_handler - handler for specific tvlv type and version
 * @list: hlist node for batadv_priv_tvlv::handler_list
 * @ogm_handler: handler callback which is given the tvlv payload to process on
 *  incoming OGM packets
 * @unicast_handler: handler callback which is given the tvlv payload to process
 *  on incoming unicast tvlv packets
 * @type: tvlv type this handler feels responsible for
 * @version: tvlv version this handler feels responsible for
 * @flags: tvlv handler flags
 * @refcount: number of contexts the object is used
 * @rcu: struct used for freeing in an RCU-safe manner
 */
struct batadv_tvlv_handler {
	struct hlist_node list;
	void (*ogm_handler)(struct batadv_priv *bat_priv,
			    struct batadv_orig_node *orig,
			    uint8_t flags,
			    void *tvlv_value, uint16_t tvlv_value_len);
	int (*unicast_handler)(struct batadv_priv *bat_priv,
			       uint8_t *src, uint8_t *dst,
			       void *tvlv_value, uint16_t tvlv_value_len);
	uint8_t type;
	uint8_t version;
	uint8_t flags;
	atomic_t refcount;
	struct rcu_head rcu;
};

/**
 * enum batadv_tvlv_handler_flags - tvlv handler flags definitions
 * @BATADV_TVLV_HANDLER_OGM_CIFNOTFND: tvlv ogm processing function will call
 *  this handler even if its type was not found (with no data)
 * @BATADV_TVLV_HANDLER_OGM_CALLED: interval tvlv handling flag - the API marks
 *  a handler as being called, so it won't be called if the
 *  BATADV_TVLV_HANDLER_OGM_CIFNOTFND flag was set
 */
enum batadv_tvlv_handler_flags {
	BATADV_TVLV_HANDLER_OGM_CIFNOTFND = BIT(1),
	BATADV_TVLV_HANDLER_OGM_CALLED = BIT(2),
};

#endif /* _NET_BATMAN_ADV_TYPES_H_ */
