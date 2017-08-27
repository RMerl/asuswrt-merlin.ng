/*
 * eBPF kernel space program part
 *
 * Toy eBPF program for demonstration purposes, some parts derived from
 * kernel tree's samples/bpf/sockex2_kern.c example.
 *
 * More background on eBPF, kernel tree: Documentation/networking/filter.txt
 *
 * Note, this file is rather large, and most classifier and actions are
 * likely smaller to accomplish one specific use-case and are tailored
 * for high performance. For performance reasons, you might also have the
 * classifier and action already merged inside the classifier.
 *
 * In order to show various features it serves as a bigger programming
 * example, which you should feel free to rip apart and experiment with.
 *
 * Compilation, configuration example:
 *
 *  Note: as long as the BPF backend in LLVM is still experimental,
 *  you need to build LLVM with LLVM with --enable-experimental-targets=BPF
 *  Also, make sure your 4.1+ kernel is compiled with CONFIG_BPF_SYSCALL=y,
 *  and you have libelf.h and gelf.h headers and can link tc against -lelf.
 *
 *  In case you need to sync kernel headers, go to your kernel source tree:
 *  # make headers_install INSTALL_HDR_PATH=/usr/
 *
 *  $ export PATH=/home/<...>/llvm/Debug+Asserts/bin/:$PATH
 *  $ clang -O2 -emit-llvm -c bpf_prog.c -o - | llc -march=bpf -filetype=obj -o bpf.o
 *  $ objdump -h bpf.o
 *  [...]
 *  3 classifier    000007f8  0000000000000000  0000000000000000  00000040  2**3
 *                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
 *  4 action-mark   00000088  0000000000000000  0000000000000000  00000838  2**3
 *                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
 *  5 action-rand   00000098  0000000000000000  0000000000000000  000008c0  2**3
 *                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
 *  6 maps          00000030  0000000000000000  0000000000000000  00000958  2**2
 *                  CONTENTS, ALLOC, LOAD, DATA
 *  7 license       00000004  0000000000000000  0000000000000000  00000988  2**0
 *                  CONTENTS, ALLOC, LOAD, DATA
 *  [...]
 *  # echo 1 > /proc/sys/net/core/bpf_jit_enable
 *  $ gcc bpf_agent.c -o bpf_agent -Wall -O2
 *  # ./bpf_agent /tmp/bpf-uds      (e.g. on a different terminal)
 *  # tc filter add dev em1 parent 1: bpf obj bpf.o exp /tmp/bpf-uds flowid 1:1 \
 *                             action bpf obj bpf.o sec action-mark            \
 *                             action bpf obj bpf.o sec action-rand ok
 *  # tc filter show dev em1
 *  filter parent 1: protocol all pref 49152 bpf
 *  filter parent 1: protocol all pref 49152 bpf handle 0x1 flowid 1:1 bpf.o:[classifier]
 *    action order 1: bpf bpf.o:[action-mark] default-action pipe
 *    index 52 ref 1 bind 1
 *
 *    action order 2: bpf bpf.o:[action-rand] default-action pipe
 *    index 53 ref 1 bind 1
 *
 *    action order 3: gact action pass
 *    random type none pass val 0
 *    index 38 ref 1 bind 1
 *
 * The same program can also be installed on ingress side (as opposed to above
 * egress configuration), e.g.:
 *
 * # tc qdisc add dev em1 handle ffff: ingress
 * # tc filter add dev em1 parent ffff: bpf obj ...
 *
 * Notes on BPF agent:
 *
 * In the above example, the bpf_agent creates the unix domain socket
 * natively. "tc exec" can also spawn a shell and hold the socktes there:
 *
 *  # tc exec bpf imp /tmp/bpf-uds
 *  # tc filter add dev em1 parent 1: bpf obj bpf.o exp /tmp/bpf-uds flowid 1:1 \
 *                             action bpf obj bpf.o sec action-mark            \
 *                             action bpf obj bpf.o sec action-rand ok
 *  sh-4.2# (shell spawned from tc exec)
 *  sh-4.2# bpf_agent
 *  [...]
 *
 * This will read out fds over environment and produce the same data dump
 * as below. This has the advantage that the spawned shell owns the fds
 * and thus if the agent is restarted, it can reattach to the same fds, also
 * various programs can easily read/modify the data simultaneously from user
 * space side.
 *
 * If the shell is unnecessary, the agent can also just be spawned directly
 * via tc exec:
 *
 *  # tc exec bpf imp /tmp/bpf-uds run bpf_agent
 *  # tc filter add dev em1 parent 1: bpf obj bpf.o exp /tmp/bpf-uds flowid 1:1 \
 *                             action bpf obj bpf.o sec action-mark            \
 *                             action bpf obj bpf.o sec action-rand ok
 *
 * BPF agent example output:
 *
 * ver: 1
 * obj: bpf.o
 * dev: 64770
 * ino: 6045133
 * maps: 3
 * map0:
 *  `- fd: 4
 *   | serial: 1
 *   | type: 1
 *   | max elem: 256
 *   | size key: 1
 *   ` size val: 16
 * map1:
 *  `- fd: 5
 *   | serial: 2
 *   | type: 1
 *   | max elem: 1024
 *   | size key: 4
 *   ` size val: 16
 * map2:
 *  `- fd: 6
 *   | serial: 3
 *   | type: 2
 *   | max elem: 64
 *   | size key: 4
 *   ` size val: 8
 * data, period: 5sec
 *  `- number of drops:	cpu0:     0	cpu1:     0	cpu2:     0	cpu3:     0
 *   | nic queues:	q0:[pkts: 0, mis: 0]	q1:[pkts: 0, mis: 0]	q2:[pkts: 0, mis: 0]	q3:[pkts: 0, mis: 0]
 *   ` protos:	tcp:[pkts: 0, bytes: 0]	udp:[pkts: 0, bytes: 0]	icmp:[pkts: 0, bytes: 0]
 * data, period: 5sec
 *  `- number of drops:	cpu0:     5	cpu1:     0	cpu2:     0	cpu3:     1
 *   | nic queues:	q0:[pkts: 0, mis: 0]	q1:[pkts: 0, mis: 0]	q2:[pkts: 24, mis: 14]	q3:[pkts: 0, mis: 0]
 *   ` protos:	tcp:[pkts: 13, bytes: 1989]	udp:[pkts: 10, bytes: 710]	icmp:[pkts: 0, bytes: 0]
 * data, period: 5sec
 *  `- number of drops:	cpu0:     5	cpu1:     0	cpu2:     3	cpu3:     3
 *   | nic queues:	q0:[pkts: 0, mis: 0]	q1:[pkts: 0, mis: 0]	q2:[pkts: 39, mis: 21]	q3:[pkts: 0, mis: 0]
 *   ` protos:	tcp:[pkts: 20, bytes: 3549]	udp:[pkts: 18, bytes: 1278]	icmp:[pkts: 0, bytes: 0]
 * [...]
 *
 * This now means, the below classifier and action pipeline has been loaded
 * as eBPF bytecode into the kernel, the kernel has verified that the
 * execution of the bytecode is "safe", and it has JITed the programs
 * afterwards, so that upon invocation they're running on native speed. tc
 * has transferred all map file descriptors to the bpf_agent via IPC and
 * even after tc exits, the agent can read out or modify all map data.
 *
 * Note that the export to the uds is done only once in the classifier and
 * not in the action. It's enough to export the (here) shared descriptors
 * once.
 *
 * If you need to disassemble the generated JIT image (echo with 2), the
 * kernel tree has under tools/net/ a small helper, you can invoke e.g.
 * `bpf_jit_disasm -o`.
 *
 * Please find in the code below further comments.
 *
 *   -- Happy eBPF hacking! ;)
 */
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_tunnel.h>
#include <linux/filter.h>
#include <linux/bpf.h>

/* Common, shared definitions with ebpf_agent.c. */
#include "bpf_shared.h"
/* Selection of BPF helper functions for our example. */
#include "bpf_funcs.h"

/* Could be defined here as well, or included from the header. */
#define TC_ACT_UNSPEC		(-1)
#define TC_ACT_OK		0
#define TC_ACT_RECLASSIFY	1
#define TC_ACT_SHOT		2
#define TC_ACT_PIPE		3
#define TC_ACT_STOLEN		4
#define TC_ACT_QUEUED		5
#define TC_ACT_REPEAT		6

/* Other, misc stuff. */
#define IP_MF			0x2000
#define IP_OFFSET		0x1FFF

/* eBPF map definitions, all placed in section "maps". */
struct bpf_elf_map __section("maps") map_proto = {
	.type		=	BPF_MAP_TYPE_HASH,
	.id		=	BPF_MAP_ID_PROTO,
	.size_key	=	sizeof(uint8_t),
	.size_value	=	sizeof(struct count_tuple),
	.max_elem	=	256,
};

struct bpf_elf_map __section("maps") map_queue = {
	.type		=	BPF_MAP_TYPE_HASH,
	.id		=	BPF_MAP_ID_QUEUE,
	.size_key	=	sizeof(uint32_t),
	.size_value	=	sizeof(struct count_queue),
	.max_elem	=	1024,
};

struct bpf_elf_map __section("maps") map_drops = {
	.type		=	BPF_MAP_TYPE_ARRAY,
	.id		=	BPF_MAP_ID_DROPS,
	.size_key	=	sizeof(uint32_t),
	.size_value	=	sizeof(long),
	.max_elem	=	64,
};

/* Helper functions and definitions for the flow dissector used by the
 * example classifier. This resembles the kernel's flow dissector to
 * some extend and is just used as an example to show what's possible
 * with eBPF.
 */
struct sockaddr;

struct vlan_hdr {
	__be16 h_vlan_TCI;
	__be16 h_vlan_encapsulated_proto;
};

struct flow_keys {
	__u32 src;
	__u32 dst;
	union {
		__u32 ports;
		__u16 port16[2];
	};
	__s32 th_off;
	__u8 ip_proto;
};

static inline int flow_ports_offset(__u8 ip_proto)
{
	switch (ip_proto) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
	case IPPROTO_DCCP:
	case IPPROTO_ESP:
	case IPPROTO_SCTP:
	case IPPROTO_UDPLITE:
	default:
		return 0;
	case IPPROTO_AH:
		return 4;
	}
}

static inline bool flow_is_frag(struct __sk_buff *skb, int nh_off)
{
	return !!(load_half(skb, nh_off + offsetof(struct iphdr, frag_off)) &
		  (IP_MF | IP_OFFSET));
}

static inline int flow_parse_ipv4(struct __sk_buff *skb, int nh_off,
				  __u8 *ip_proto, struct flow_keys *flow)
{
	__u8 ip_ver_len;

	if (unlikely(flow_is_frag(skb, nh_off)))
		*ip_proto = 0;
	else
		*ip_proto = load_byte(skb, nh_off + offsetof(struct iphdr,
							     protocol));
	if (*ip_proto != IPPROTO_GRE) {
		flow->src = load_word(skb, nh_off + offsetof(struct iphdr, saddr));
		flow->dst = load_word(skb, nh_off + offsetof(struct iphdr, daddr));
	}

	ip_ver_len = load_byte(skb, nh_off + 0 /* offsetof(struct iphdr, ihl) */);
	if (likely(ip_ver_len == 0x45))
		nh_off += 20;
	else
		nh_off += (ip_ver_len & 0xF) << 2;

	return nh_off;
}

static inline __u32 flow_addr_hash_ipv6(struct __sk_buff *skb, int off)
{
	__u32 w0 = load_word(skb, off);
	__u32 w1 = load_word(skb, off + sizeof(w0));
	__u32 w2 = load_word(skb, off + sizeof(w0) * 2);
	__u32 w3 = load_word(skb, off + sizeof(w0) * 3);

	return w0 ^ w1 ^ w2 ^ w3;
}

static inline int flow_parse_ipv6(struct __sk_buff *skb, int nh_off,
				  __u8 *ip_proto, struct flow_keys *flow)
{
	*ip_proto = load_byte(skb, nh_off + offsetof(struct ipv6hdr, nexthdr));

	flow->src = flow_addr_hash_ipv6(skb, nh_off + offsetof(struct ipv6hdr, saddr));
	flow->dst = flow_addr_hash_ipv6(skb, nh_off + offsetof(struct ipv6hdr, daddr));

	return nh_off + sizeof(struct ipv6hdr);
}

static inline bool flow_dissector(struct __sk_buff *skb,
				  struct flow_keys *flow)
{
	int poff, nh_off = BPF_LL_OFF + ETH_HLEN;
	__be16 proto = skb->protocol;
	__u8 ip_proto;

	/* TODO: check for skb->vlan_tci, skb->vlan_proto first */
	if (proto == htons(ETH_P_8021AD)) {
		proto = load_half(skb, nh_off +
				  offsetof(struct vlan_hdr, h_vlan_encapsulated_proto));
		nh_off += sizeof(struct vlan_hdr);
	}
	if (proto == htons(ETH_P_8021Q)) {
		proto = load_half(skb, nh_off +
				  offsetof(struct vlan_hdr, h_vlan_encapsulated_proto));
		nh_off += sizeof(struct vlan_hdr);
	}

	if (likely(proto == htons(ETH_P_IP)))
		nh_off = flow_parse_ipv4(skb, nh_off, &ip_proto, flow);
	else if (proto == htons(ETH_P_IPV6))
		nh_off = flow_parse_ipv6(skb, nh_off, &ip_proto, flow);
	else
		return false;

	switch (ip_proto) {
	case IPPROTO_GRE: {
		struct gre_hdr {
			__be16 flags;
			__be16 proto;
		};

		__u16 gre_flags = load_half(skb, nh_off +
					    offsetof(struct gre_hdr, flags));
		__u16 gre_proto = load_half(skb, nh_off +
					    offsetof(struct gre_hdr, proto));

		if (gre_flags & (GRE_VERSION | GRE_ROUTING))
			break;

		nh_off += 4;
		if (gre_flags & GRE_CSUM)
			nh_off += 4;
		if (gre_flags & GRE_KEY)
			nh_off += 4;
		if (gre_flags & GRE_SEQ)
			nh_off += 4;

		if (gre_proto == ETH_P_8021Q) {
			gre_proto = load_half(skb, nh_off +
					      offsetof(struct vlan_hdr,
						       h_vlan_encapsulated_proto));
			nh_off += sizeof(struct vlan_hdr);
		}
		if (gre_proto == ETH_P_IP)
			nh_off = flow_parse_ipv4(skb, nh_off, &ip_proto, flow);
		else if (gre_proto == ETH_P_IPV6)
			nh_off = flow_parse_ipv6(skb, nh_off, &ip_proto, flow);
		else
			return false;
		break;
	}
	case IPPROTO_IPIP:
		nh_off = flow_parse_ipv4(skb, nh_off, &ip_proto, flow);
		break;
	case IPPROTO_IPV6:
		nh_off = flow_parse_ipv6(skb, nh_off, &ip_proto, flow);
	default:
		break;
	}

	nh_off += flow_ports_offset(ip_proto);

	flow->ports = load_word(skb, nh_off);
	flow->th_off = nh_off;
	flow->ip_proto = ip_proto;

	return true;
}

static inline void cls_update_proto_map(const struct __sk_buff *skb,
					const struct flow_keys *flow)
{
	uint8_t proto = flow->ip_proto;
	struct count_tuple *ct, _ct;

	ct = bpf_map_lookup_elem(&map_proto, &proto);
	if (likely(ct)) {
		__sync_fetch_and_add(&ct->packets, 1);
		__sync_fetch_and_add(&ct->bytes, skb->len);
		return;
	}

	/* No hit yet, we need to create a new entry. */
	_ct.packets = 1;
	_ct.bytes = skb->len;

	bpf_map_update_elem(&map_proto, &proto, &_ct, BPF_ANY);
}

static inline void cls_update_queue_map(const struct __sk_buff *skb)
{
	uint32_t queue = skb->queue_mapping;
	struct count_queue *cq, _cq;
	bool mismatch;

	mismatch = skb->queue_mapping != get_smp_processor_id();

	cq = bpf_map_lookup_elem(&map_queue, &queue);
	if (likely(cq)) {
		__sync_fetch_and_add(&cq->total, 1);
		if (mismatch)
			__sync_fetch_and_add(&cq->mismatch, 1);
		return;
	}

	/* No hit yet, we need to create a new entry. */
	_cq.total = 1;
	_cq.mismatch = mismatch ? 1 : 0;

	bpf_map_update_elem(&map_queue, &queue, &_cq, BPF_ANY);
}

/* eBPF program definitions, placed in various sections, which can
 * have custom section names. If custom names are in use, it's
 * required to point tc to the correct section, e.g.
 *
 *     tc filter add [...] bpf obj cls.o sec cls-tos [...]
 *
 * in case the program resides in __section("cls-tos").
 *
 * Default section for cls_bpf is: "classifier", for act_bpf is:
 * "action". Naturally, if for example multiple actions are present
 * in the same file, they need to have distinct section names.
 *
 * It is however not required to have multiple programs sharing
 * a file.
 */
__section("classifier") int cls_main(struct __sk_buff *skb)
{
	struct flow_keys flow;

	if (!flow_dissector(skb, &flow))
		return 0; /* No match in cls_bpf. */

	cls_update_proto_map(skb, &flow);
	cls_update_queue_map(skb);

	return flow.ip_proto;
}

static inline void act_update_drop_map(void)
{
	uint32_t *count, cpu = get_smp_processor_id();

	count = bpf_map_lookup_elem(&map_drops, &cpu);
	if (count)
		/* Only this cpu is accessing this element. */
		(*count)++;
}

__section("action-mark") int act_mark_main(struct __sk_buff *skb)
{
	/* You could also mangle skb data here with the helper function
	 * BPF_FUNC_skb_store_bytes, etc. Or, alternatively you could
	 * do that already in the classifier itself as a merged combination
	 * of classifier'n'action model.
	 */

	if (skb->mark == 0xcafe) {
		act_update_drop_map();
		return TC_ACT_SHOT;
	}

	/* Default configured tc opcode. */
	return TC_ACT_UNSPEC;
}

__section("action-rand") int act_rand_main(struct __sk_buff *skb)
{
	/* Sorry, we're near event horizon ... */
	if ((get_prandom_u32() & 3) == 0) {
		act_update_drop_map();
		return TC_ACT_SHOT;
	}

	return TC_ACT_UNSPEC;
}

/* Last but not least, the file contains a license. Some future helper
 * functions may only be available with a GPL license.
 */
char __license[] __section("license") = "GPL";
