/*
 * Copyright (C) 2008-2018 Tobias Brunner
 * Copyright (C) 2008 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
/*
 * Copyright (C) 2014 Nanoteq Pty Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __FreeBSD__
#include <limits.h> /* for LONG_MAX */
#endif

#ifdef HAVE_NET_PFKEYV2_H
#include <net/pfkeyv2.h>
#else
#include <linux/pfkeyv2.h>
#endif

#ifdef SADB_X_EXT_NAT_T_TYPE
#define HAVE_NATT
#endif

#ifdef HAVE_NETIPSEC_IPSEC_H
#include <netipsec/ipsec.h>
#elif defined(HAVE_NETINET6_IPSEC_H)
#include <netinet6/ipsec.h>
#else
#include <linux/ipsec.h>
#endif

#ifdef HAVE_NATT
#ifdef HAVE_LINUX_UDP_H
#include <linux/udp.h>
#else
#include <netinet/udp.h>
#endif /*HAVE_LINUX_UDP_H*/
#endif /*HAVE_NATT*/

#include <unistd.h>
#include <time.h>
#include <errno.h>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#include "kernel_pfkey_ipsec.h"

#include <daemon.h>
#include <utils/debug.h>
#include <networking/host.h>
#include <collections/linked_list.h>
#include <collections/hashtable.h>
#include <threading/mutex.h>

/** non linux specific */
#ifndef IPPROTO_COMP
#ifdef IPPROTO_IPCOMP
#define IPPROTO_COMP IPPROTO_IPCOMP
#endif
#endif

#ifndef SADB_X_AALG_SHA2_256HMAC
#define SADB_X_AALG_SHA2_256HMAC SADB_X_AALG_SHA2_256
#define SADB_X_AALG_SHA2_384HMAC SADB_X_AALG_SHA2_384
#define SADB_X_AALG_SHA2_512HMAC SADB_X_AALG_SHA2_512
#endif

#ifndef SADB_X_EALG_AESCBC
#define SADB_X_EALG_AESCBC SADB_X_EALG_AES
#endif

#ifndef SADB_X_EALG_CASTCBC
#define SADB_X_EALG_CASTCBC SADB_X_EALG_CAST128CBC
#endif

#if !defined(SADB_X_EALG_AES_GCM_ICV8) && defined(SADB_X_EALG_AESGCM8)
#define SADB_X_EALG_AES_GCM_ICV8 SADB_X_EALG_AESGCM8
#define SADB_X_EALG_AES_GCM_ICV12 SADB_X_EALG_AESGCM12
#define SADB_X_EALG_AES_GCM_ICV16 SADB_X_EALG_AESGCM16
#endif

#ifndef SOL_IP
#define SOL_IP IPPROTO_IP
#define SOL_IPV6 IPPROTO_IPV6
#endif

/** from linux/in.h */
#ifndef IP_IPSEC_POLICY
#define IP_IPSEC_POLICY 16
#endif

/** missing on uclibc */
#ifndef IPV6_IPSEC_POLICY
#define IPV6_IPSEC_POLICY 34
#endif

/* from linux/udp.h */
#ifndef UDP_ENCAP
#define UDP_ENCAP 100
#endif

#ifndef UDP_ENCAP_ESPINUDP
#define UDP_ENCAP_ESPINUDP 2
#endif

/* this is not defined on some platforms */
#ifndef SOL_UDP
#define SOL_UDP IPPROTO_UDP
#endif

/** Base priority for installed policies */
#define PRIO_BASE 200000

#ifdef __APPLE__
/** from xnu/bsd/net/pfkeyv2.h */
#define SADB_X_EXT_NATT 0x002
	struct sadb_sa_2 {
		struct sadb_sa	sa;
		uint16_t		sadb_sa_natt_port;
		uint16_t		sadb_reserved0;
		uint32_t		sadb_reserved1;
	};
#endif

/** buffer size for PF_KEY messages */
#define PFKEY_BUFFER_SIZE 4096

/** PF_KEY messages are 64 bit aligned */
#define PFKEY_ALIGNMENT 8
/** aligns len to 64 bits */
#define PFKEY_ALIGN(len) (((len) + PFKEY_ALIGNMENT - 1) & ~(PFKEY_ALIGNMENT - 1))
/** calculates the properly padded length in 64 bit chunks */
#define PFKEY_LEN(len) ((PFKEY_ALIGN(len) / PFKEY_ALIGNMENT))
/** calculates user mode length i.e. in bytes */
#define PFKEY_USER_LEN(len) ((len) * PFKEY_ALIGNMENT)

/** given a PF_KEY message header and an extension this updates the length in the header */
#define PFKEY_EXT_ADD(msg, ext) ((msg)->sadb_msg_len += ((struct sadb_ext*)ext)->sadb_ext_len)
/** given a PF_KEY message header this returns a pointer to the next extension */
#define PFKEY_EXT_ADD_NEXT(msg) ((struct sadb_ext*)(((char*)(msg)) + PFKEY_USER_LEN((msg)->sadb_msg_len)))
/** copy an extension and append it to a PF_KEY message */
#define PFKEY_EXT_COPY(msg, ext) (PFKEY_EXT_ADD(msg, memcpy(PFKEY_EXT_ADD_NEXT(msg), ext, PFKEY_USER_LEN(((struct sadb_ext*)ext)->sadb_ext_len))))
/** given a PF_KEY extension this returns a pointer to the next extension */
#define PFKEY_EXT_NEXT(ext) ((struct sadb_ext*)(((char*)(ext)) + PFKEY_USER_LEN(((struct sadb_ext*)ext)->sadb_ext_len)))
/** given a PF_KEY extension this returns a pointer to the next extension also updates len (len in 64 bit words) */
#define PFKEY_EXT_NEXT_LEN(ext,len) ((len) -= (ext)->sadb_ext_len, PFKEY_EXT_NEXT(ext))
/** true if ext has a valid length and len is large enough to contain ext (assuming len in 64 bit words) */
#define PFKEY_EXT_OK(ext,len) ((len) >= PFKEY_LEN(sizeof(struct sadb_ext)) && \
				(ext)->sadb_ext_len >= PFKEY_LEN(sizeof(struct sadb_ext)) && \
				(ext)->sadb_ext_len <= (len))

typedef struct private_kernel_pfkey_ipsec_t private_kernel_pfkey_ipsec_t;

/**
 * Private variables and functions of kernel_pfkey class.
 */
struct private_kernel_pfkey_ipsec_t
{
	/**
	 * Public part of the kernel_pfkey_t object.
	 */
	kernel_pfkey_ipsec_t public;

	/**
	 * mutex to lock access to various lists
	 */
	mutex_t *mutex;

	/**
	 * List of installed policies (policy_entry_t)
	 */
	linked_list_t *policies;

	/**
	 * List of exclude routes (exclude_route_t)
	 */
	linked_list_t *excludes;

	/**
	 * Hash table of IPsec SAs using policies (ipsec_sa_t)
	 */
	hashtable_t *sas;

	/**
	 * whether to install routes along policies
	 */
	bool install_routes;

	/**
	 * whether to install the route via internal interface
	 */
	bool route_via_internal;

	/**
	 * mutex to lock access to the PF_KEY socket
	 */
	mutex_t *mutex_pfkey;

	/**
	 * PF_KEY socket to communicate with the kernel
	 */
	int socket;

	/**
	 * PF_KEY socket to receive acquire and expire events
	 */
	int socket_events;

	/**
	 * sequence number for messages sent to the kernel
	 */
	int seq;
};

typedef struct exclude_route_t exclude_route_t;

/**
 * Exclude route definition
 */
struct exclude_route_t {
	/** destination address of exclude */
	host_t *dst;
	/** source address for route */
	host_t *src;
	/** nexthop exclude has been installed */
	host_t *gtw;
	/** references to this route */
	int refs;
};

/**
 * clean up a route exclude entry
 */
static void exclude_route_destroy(exclude_route_t *this)
{
	this->dst->destroy(this->dst);
	this->src->destroy(this->src);
	this->gtw->destroy(this->gtw);
	free(this);
}

typedef struct route_entry_t route_entry_t;

/**
 * installed routing entry
 */
struct route_entry_t {
	/** name of the interface the route is bound to */
	char *if_name;

	/** source ip of the route */
	host_t *src_ip;

	/** gateway for this route */
	host_t *gateway;

	/** destination net */
	chunk_t dst_net;

	/** destination net prefixlen */
	uint8_t prefixlen;

	/** reference to exclude route, if any */
	exclude_route_t *exclude;
};

/**
 * destroy an route_entry_t object
 */
static void route_entry_destroy(route_entry_t *this)
{
	free(this->if_name);
	DESTROY_IF(this->src_ip);
	DESTROY_IF(this->gateway);
	chunk_free(&this->dst_net);
	free(this);
}

/**
 * compare two route_entry_t objects
 */
static bool route_entry_equals(route_entry_t *a, route_entry_t *b)
{
	return a->if_name && b->if_name && streq(a->if_name, b->if_name) &&
		   a->src_ip->ip_equals(a->src_ip, b->src_ip) &&
		   a->gateway && b->gateway &&
		   a->gateway->ip_equals(a->gateway, b->gateway) &&
		   chunk_equals(a->dst_net, b->dst_net) && a->prefixlen == b->prefixlen;
}

typedef struct ipsec_sa_t ipsec_sa_t;

/**
 * IPsec SA assigned to a policy.
 */
struct ipsec_sa_t {
	/** Source address of this SA */
	host_t *src;

	/** Destination address of this SA */
	host_t *dst;

	/** Description of this SA */
	ipsec_sa_cfg_t cfg;

	/** Reference count for this SA */
	refcount_t refcount;
};

/**
 * Hash function for ipsec_sa_t objects
 */
static u_int ipsec_sa_hash(ipsec_sa_t *sa)
{
	return chunk_hash_inc(sa->src->get_address(sa->src),
						  chunk_hash_inc(sa->dst->get_address(sa->dst),
						  chunk_hash(chunk_from_thing(sa->cfg))));
}

/**
 * Equality function for ipsec_sa_t objects
 */
static bool ipsec_sa_equals(ipsec_sa_t *sa, ipsec_sa_t *other_sa)
{
	return sa->src->ip_equals(sa->src, other_sa->src) &&
		   sa->dst->ip_equals(sa->dst, other_sa->dst) &&
		   ipsec_sa_cfg_equals(&sa->cfg, &other_sa->cfg);
}

/**
 * Allocate or reference an IPsec SA object
 */
static ipsec_sa_t *ipsec_sa_create(private_kernel_pfkey_ipsec_t *this,
								   host_t *src, host_t *dst,
								   ipsec_sa_cfg_t *cfg)
{
	ipsec_sa_t *sa, *found;
	INIT(sa,
		.src = src,
		.dst = dst,
		.cfg = *cfg,
	);
	found = this->sas->get(this->sas, sa);
	if (!found)
	{
		sa->src = src->clone(src);
		sa->dst = dst->clone(dst);
		this->sas->put(this->sas, sa, sa);
	}
	else
	{
		free(sa);
		sa = found;
	}
	ref_get(&sa->refcount);
	return sa;
}

/**
 * Release and destroy an IPsec SA object
 */
static void ipsec_sa_destroy(private_kernel_pfkey_ipsec_t *this,
							 ipsec_sa_t *sa)
{
	if (ref_put(&sa->refcount))
	{
		this->sas->remove(this->sas, sa);
		DESTROY_IF(sa->src);
		DESTROY_IF(sa->dst);
		free(sa);
	}
}

typedef struct policy_sa_t policy_sa_t;
typedef struct policy_sa_out_t policy_sa_out_t;

/**
 * Mapping between a policy and an IPsec SA.
 */
struct policy_sa_t {
	/** Priority assigned to the policy when installed with this SA */
	uint32_t priority;

	/** Base priority assigned to the policy when installed with this SA */
	uint32_t auto_priority;

	/** Type of the policy */
	policy_type_t type;

	/** Assigned SA */
	ipsec_sa_t *sa;
};

/**
 * For outbound policies we also cache the traffic selectors in order to install
 * the route.
 */
struct policy_sa_out_t {
	/** Generic interface */
	policy_sa_t generic;

	/** Source traffic selector of this policy */
	traffic_selector_t *src_ts;

	/** Destination traffic selector of this policy */
	traffic_selector_t *dst_ts;
};

/**
 * Create a policy_sa(_in)_t object
 */
static policy_sa_t *policy_sa_create(private_kernel_pfkey_ipsec_t *this,
	policy_dir_t dir, policy_type_t type, host_t *src, host_t *dst,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts, ipsec_sa_cfg_t *cfg)
{
	policy_sa_t *policy;

	if (dir == POLICY_OUT)
	{
		policy_sa_out_t *out;
		INIT(out,
			.src_ts = src_ts->clone(src_ts),
			.dst_ts = dst_ts->clone(dst_ts),
		);
		policy = &out->generic;
	}
	else
	{
		INIT(policy, .priority = 0);
	}
	policy->type = type;
	policy->sa = ipsec_sa_create(this, src, dst, cfg);
	return policy;
}

/**
 * Destroy a policy_sa(_in)_t object
 */
static void policy_sa_destroy(policy_sa_t *policy, policy_dir_t dir,
							  private_kernel_pfkey_ipsec_t *this)
{
	if (dir == POLICY_OUT)
	{
		policy_sa_out_t *out = (policy_sa_out_t*)policy;
		out->src_ts->destroy(out->src_ts);
		out->dst_ts->destroy(out->dst_ts);
	}
	ipsec_sa_destroy(this, policy->sa);
	free(policy);
}

CALLBACK(policy_sa_destroy_cb, void,
	policy_sa_t *policy, va_list args)
{
	private_kernel_pfkey_ipsec_t *this;
	policy_dir_t dir;

	VA_ARGS_VGET(args, dir, this);
	policy_sa_destroy(policy, dir, this);
}

typedef struct policy_entry_t policy_entry_t;

/**
 * installed kernel policy.
 */
struct policy_entry_t {
	/** Index assigned by the kernel */
	uint32_t index;

	/** Direction of this policy: in, out, forward */
	uint8_t direction;

	/** Parameters of installed policy */
	struct {
		/** Subnet and port */
		host_t *net;
		/** Subnet mask */
		uint8_t mask;
		/** Protocol */
		uint8_t proto;
	} src, dst;

	/** Associated route installed for this policy */
	route_entry_t *route;

	/** List of SAs this policy is used by, ordered by priority */
	linked_list_t *used_by;
};

/**
 * Create a policy_entry_t object
 */
static policy_entry_t *create_policy_entry(traffic_selector_t *src_ts,
										   traffic_selector_t *dst_ts,
										   policy_dir_t dir)
{
	policy_entry_t *policy;
	INIT(policy,
		.direction = dir,
	);
	uint16_t port;
	uint8_t proto;

	src_ts->to_subnet(src_ts, &policy->src.net, &policy->src.mask);
	dst_ts->to_subnet(dst_ts, &policy->dst.net, &policy->dst.mask);

	/* src or dest proto may be "any" (0), use more restrictive one */
	proto = max(src_ts->get_protocol(src_ts), dst_ts->get_protocol(dst_ts));
	/* map the ports to ICMP type/code how the Linux kernel expects them, that
	 * is, type in src, code in dst */
	if (proto == IPPROTO_ICMP || proto == IPPROTO_ICMPV6)
	{
		port = max(policy->src.net->get_port(policy->src.net),
				   policy->dst.net->get_port(policy->dst.net));
		policy->src.net->set_port(policy->src.net,
								  traffic_selector_icmp_type(port));
		policy->dst.net->set_port(policy->dst.net,
								  traffic_selector_icmp_code(port));
	}
	else if (!proto)
	{
		proto = IPSEC_PROTO_ANY;
	}
	policy->src.proto = policy->dst.proto = proto;

	return policy;
}

/**
 * Destroy a policy_entry_t object
 */
static void policy_entry_destroy(policy_entry_t *policy,
								 private_kernel_pfkey_ipsec_t *this)
{
	if (policy->route)
	{
		route_entry_destroy(policy->route);
	}
	if (policy->used_by)
	{
		policy->used_by->invoke_function(policy->used_by, policy_sa_destroy_cb,
										 policy->direction, this);
		policy->used_by->destroy(policy->used_by);
	}
	DESTROY_IF(policy->src.net);
	DESTROY_IF(policy->dst.net);
	free(policy);
}

CALLBACK(policy_entry_destroy_cb, void,
	policy_entry_t *policy, va_list args)
{
	private_kernel_pfkey_ipsec_t *this;

	VA_ARGS_VGET(args, this);
	policy_entry_destroy(policy, this);
}

CALLBACK(policy_entry_equals, bool,
	policy_entry_t *current, va_list args)
{
	policy_entry_t *policy;

	VA_ARGS_VGET(args, policy);
	return current->direction == policy->direction &&
		   current->src.proto == policy->src.proto &&
		   current->dst.proto == policy->dst.proto &&
		   current->src.mask == policy->src.mask &&
		   current->dst.mask == policy->dst.mask &&
		   current->src.net->equals(current->src.net, policy->src.net) &&
		   current->dst.net->equals(current->dst.net, policy->dst.net);
}

CALLBACK(policy_entry_match_byindex, bool,
	policy_entry_t *current, va_list args)
{
	uint32_t index;

	VA_ARGS_VGET(args, index);
	return current->index == index;
}

/**
 * Calculate the priority of a policy
 *
 * This is the same formula we use in the kernel-netlink interface, but some
 * features are currently not or only partially supported by PF_KEY.
 *
 * bits 0-0:  separate trap and regular policies (0..1)     1 bit
 * bits 1-1:  reserved for interface restriction (0..1)     1 bit
 * bits 2-7:  src + dst port mask bits (2 * 0..16)          6 bits
 * bits 8-8:  restriction to protocol (0..1)                1 bit
 * bits 9-17: src + dst network mask bits (2 * 0..128)      9 bits
 *                                                         18 bits
 *
 * smallest value: 000000000 0 000000 0 0:       0, lowest priority = 100'000
 * largest value : 100000000 1 100000 0 1: 131'457, highst priority =  68'543
 */
static inline uint32_t get_priority(policy_entry_t *policy,
									policy_priority_t prio)
{
	uint32_t priority = PRIO_BASE;

	switch (prio)
	{
		case POLICY_PRIORITY_FALLBACK:
			priority += PRIO_BASE;
			/* fall-through */
		case POLICY_PRIORITY_ROUTED:
		case POLICY_PRIORITY_DEFAULT:
			priority += PRIO_BASE;
			/* fall-through */
		case POLICY_PRIORITY_PASS:
			break;
	}

	/* calculate priority */
	priority -= (policy->src.mask + policy->dst.mask) * 512;
	priority -=  policy->src.proto != IPSEC_PROTO_ANY ? 256 : 0;
	priority -= policy->src.net->get_port(policy->src.net) ? 64 : 0;
	priority -= policy->dst.net->get_port(policy->dst.net) ? 64 : 0;
	priority -= (prio != POLICY_PRIORITY_ROUTED);
	return priority;
}

typedef struct pfkey_msg_t pfkey_msg_t;

struct pfkey_msg_t
{
	/**
	 * PF_KEY message base
	 */
	struct sadb_msg *msg;

	/**
	 * PF_KEY message extensions
	 */
	union {
		struct sadb_ext *ext[SADB_EXT_MAX + 1];
		struct {
			struct sadb_ext *reserved;				/* SADB_EXT_RESERVED */
			struct sadb_sa *sa;						/* SADB_EXT_SA */
			struct sadb_lifetime *lft_current;		/* SADB_EXT_LIFETIME_CURRENT */
			struct sadb_lifetime *lft_hard;			/* SADB_EXT_LIFETIME_HARD */
			struct sadb_lifetime *lft_soft;			/* SADB_EXT_LIFETIME_SOFT */
			struct sadb_address *src;				/* SADB_EXT_ADDRESS_SRC */
			struct sadb_address *dst;				/* SADB_EXT_ADDRESS_DST */
			struct sadb_address *proxy;				/* SADB_EXT_ADDRESS_PROXY */
			struct sadb_key *key_auth;				/* SADB_EXT_KEY_AUTH */
			struct sadb_key *key_encr;				/* SADB_EXT_KEY_ENCRYPT */
			struct sadb_ident *id_src;				/* SADB_EXT_IDENTITY_SRC */
			struct sadb_ident *id_dst;				/* SADB_EXT_IDENTITY_DST */
			struct sadb_sens *sensitivity;			/* SADB_EXT_SENSITIVITY */
			struct sadb_prop *proposal;				/* SADB_EXT_PROPOSAL */
			struct sadb_supported *supported_auth;	/* SADB_EXT_SUPPORTED_AUTH */
			struct sadb_supported *supported_encr;	/* SADB_EXT_SUPPORTED_ENCRYPT */
			struct sadb_spirange *spirange;			/* SADB_EXT_SPIRANGE */
			struct sadb_x_kmprivate *x_kmprivate;	/* SADB_X_EXT_KMPRIVATE */
			struct sadb_x_policy *x_policy;			/* SADB_X_EXT_POLICY */
			struct sadb_x_sa2 *x_sa2;				/* SADB_X_EXT_SA2 */
#if defined(__linux__) || defined (__FreeBSD__)
			struct sadb_x_nat_t_type *x_natt_type;	/* SADB_X_EXT_NAT_T_TYPE */
			struct sadb_x_nat_t_port *x_natt_sport;	/* SADB_X_EXT_NAT_T_SPORT */
			struct sadb_x_nat_t_port *x_natt_dport;	/* SADB_X_EXT_NAT_T_DPORT */
#ifdef __linux__
			struct sadb_address *x_natt_oa;			/* SADB_X_EXT_NAT_T_OA */
			struct sadb_x_sec_ctx *x_sec_ctx;		/* SADB_X_EXT_SEC_CTX */
			struct sadb_x_kmaddress *x_kmaddress;	/* SADB_X_EXT_KMADDRESS */
#else
			struct sadb_address *x_natt_oai;		/* SADB_X_EXT_NAT_T_OAI */
			struct sadb_address *x_natt_oar;		/* SADB_X_EXT_NAT_T_OAR */
#ifdef SADB_X_EXT_NAT_T_FRAG
			struct sadb_x_nat_t_frag *x_natt_frag;	/* SADB_X_EXT_NAT_T_FRAG */
#ifdef SADB_X_EXT_SA_REPLAY
			struct sadb_x_sa_replay *x_replay;		/* SADB_X_EXT_SA_REPLAY */
			struct sadb_address *x_new_addr_src;	/* SADB_X_EXT_NEW_ADDRESS_SRC */
			struct sadb_address *x_new_addr_dst;	/* SADB_X_EXT_NEW_ADDRESS_DST */
#endif
#endif
#endif /* __linux__ */
#endif /* __linux__ || __FreeBSD__ */
		} __attribute__((__packed__));
	};
};

ENUM(sadb_ext_type_names, SADB_EXT_RESERVED, SADB_EXT_MAX,
	"SADB_EXT_RESERVED",
	"SADB_EXT_SA",
	"SADB_EXT_LIFETIME_CURRENT",
	"SADB_EXT_LIFETIME_HARD",
	"SADB_EXT_LIFETIME_SOFT",
	"SADB_EXT_ADDRESS_SRC",
	"SADB_EXT_ADDRESS_DST",
	"SADB_EXT_ADDRESS_PROXY",
	"SADB_EXT_KEY_AUTH",
	"SADB_EXT_KEY_ENCRYPT",
	"SADB_EXT_IDENTITY_SRC",
	"SADB_EXT_IDENTITY_DST",
	"SADB_EXT_SENSITIVITY",
	"SADB_EXT_PROPOSAL",
	"SADB_EXT_SUPPORTED_AUTH",
	"SADB_EXT_SUPPORTED_ENCRYPT",
	"SADB_EXT_SPIRANGE",
	"SADB_X_EXT_KMPRIVATE",
	"SADB_X_EXT_POLICY",
	"SADB_X_EXT_SA2",
#ifdef __APPLE__
	"SADB_EXT_SESSION_ID",
	"SADB_EXT_SASTAT",
	"SADB_X_EXT_IPSECIF",
	"SADB_X_EXT_ADDR_RANGE_SRC_START",
	"SADB_X_EXT_ADDR_RANGE_SRC_END",
	"SADB_X_EXT_ADDR_RANGE_DST_START",
	"SADB_X_EXT_ADDR_RANGE_DST_END",
	"SADB_EXT_MIGRATE_ADDRESS_SRC",
	"SADB_EXT_MIGRATE_ADDRESS_DST",
	"SADB_X_EXT_MIGRATE_IPSECIF",
#else
	"SADB_X_EXT_NAT_T_TYPE",
	"SADB_X_EXT_NAT_T_SPORT",
	"SADB_X_EXT_NAT_T_DPORT",
#ifdef __linux__
	"SADB_X_EXT_NAT_T_OA",
	"SADB_X_EXT_SEC_CTX",
	"SADB_X_EXT_KMADDRESS",
#else
	"SADB_X_EXT_NAT_T_OAI",
	"SADB_X_EXT_NAT_T_OAR",
	"SADB_X_EXT_NAT_T_FRAG",
	"SADB_X_EXT_SA_REPLAY",
	"SADB_X_EXT_NEW_ADDRESS_SRC",
	"SADB_X_EXT_NEW_ADDRESS_DST",
#endif /* __linux__ */
#endif /* __APPLE__ */
);

/**
 * convert a protocol identifier to the PF_KEY sa type
 */
static uint8_t proto2satype(uint8_t proto)
{
	switch (proto)
	{
		case IPPROTO_ESP:
			return SADB_SATYPE_ESP;
		case IPPROTO_AH:
			return SADB_SATYPE_AH;
		case IPPROTO_COMP:
			return SADB_X_SATYPE_IPCOMP;
		default:
			return proto;
	}
}

/**
 * convert a PF_KEY sa type to a protocol identifier
 */
static uint8_t satype2proto(uint8_t satype)
{
	switch (satype)
	{
		case SADB_SATYPE_ESP:
			return IPPROTO_ESP;
		case SADB_SATYPE_AH:
			return IPPROTO_AH;
		case SADB_X_SATYPE_IPCOMP:
			return IPPROTO_COMP;
		default:
			return satype;
	}
}

/**
 * convert the general ipsec mode to the one defined in ipsec.h
 */
static uint8_t mode2kernel(ipsec_mode_t mode)
{
	switch (mode)
	{
		case MODE_TRANSPORT:
			return IPSEC_MODE_TRANSPORT;
		case MODE_TUNNEL:
			return IPSEC_MODE_TUNNEL;
#ifdef HAVE_IPSEC_MODE_BEET
		case MODE_BEET:
			return IPSEC_MODE_BEET;
#endif
		default:
			return mode;
	}
}

/**
 * convert the general policy direction to the one defined in ipsec.h
 */
static uint8_t dir2kernel(policy_dir_t dir)
{
	switch (dir)
	{
		case POLICY_IN:
			return IPSEC_DIR_INBOUND;
		case POLICY_OUT:
			return IPSEC_DIR_OUTBOUND;
#ifdef HAVE_IPSEC_DIR_FWD
		case POLICY_FWD:
			return IPSEC_DIR_FWD;
#endif
		default:
			return IPSEC_DIR_INVALID;
	}
}

/**
 * convert the policy type to the one defined in ipsec.h
 */
static inline uint16_t type2kernel(policy_type_t type)
{
	switch (type)
	{
		case POLICY_IPSEC:
			return IPSEC_POLICY_IPSEC;
		case POLICY_PASS:
			return IPSEC_POLICY_NONE;
		case POLICY_DROP:
			return IPSEC_POLICY_DISCARD;
	}
	return type;
}

#ifdef SADB_X_MIGRATE
/**
 * convert the policy direction in ipsec.h to the general one.
 */
static policy_dir_t kernel2dir(uint8_t  dir)
{
	switch (dir)
	{
		case IPSEC_DIR_INBOUND:
			return POLICY_IN;
		case IPSEC_DIR_OUTBOUND:
			return POLICY_OUT;
#ifdef HAVE_IPSEC_DIR_FWD
		case IPSEC_DIR_FWD:
			return POLICY_FWD;
#endif
		default:
			return dir;
	}
}
#endif /*SADB_X_MIGRATE*/

typedef struct kernel_algorithm_t kernel_algorithm_t;

/**
 * Mapping of IKEv2 algorithms to PF_KEY algorithms
 */
struct kernel_algorithm_t {
	/**
	 * Identifier specified in IKEv2
	 */
	int ikev2;

	/**
	 * Identifier as defined in pfkeyv2.h
	 */
	int kernel;
};

#define END_OF_LIST -1

/**
 * Algorithms for encryption
 */
static kernel_algorithm_t encryption_algs[] = {
/*	{ENCR_DES_IV64,				0							}, */
	{ENCR_DES,					SADB_EALG_DESCBC			},
	{ENCR_3DES,					SADB_EALG_3DESCBC			},
/*	{ENCR_RC5,					0							}, */
/*	{ENCR_IDEA,					0							}, */
	{ENCR_CAST,					SADB_X_EALG_CASTCBC			},
	{ENCR_BLOWFISH,				SADB_X_EALG_BLOWFISHCBC		},
/*	{ENCR_3IDEA,				0							}, */
/*	{ENCR_DES_IV32,				0							}, */
	{ENCR_NULL,					SADB_EALG_NULL				},
	{ENCR_AES_CBC,				SADB_X_EALG_AESCBC			},
#ifdef SADB_X_EALG_AESCTR
	{ENCR_AES_CTR,				SADB_X_EALG_AESCTR			},
#endif
/*  {ENCR_AES_CCM_ICV8,			SADB_X_EALG_AES_CCM_ICV8	}, */
/*	{ENCR_AES_CCM_ICV12,		SADB_X_EALG_AES_CCM_ICV12	}, */
/*	{ENCR_AES_CCM_ICV16,		SADB_X_EALG_AES_CCM_ICV16	}, */
#ifdef SADB_X_EALG_AES_GCM_ICV8 /* assume the others are defined too */
	{ENCR_AES_GCM_ICV8,			SADB_X_EALG_AES_GCM_ICV8	},
	{ENCR_AES_GCM_ICV12,		SADB_X_EALG_AES_GCM_ICV12	},
	{ENCR_AES_GCM_ICV16,		SADB_X_EALG_AES_GCM_ICV16	},
#elif defined(SADB_X_EALG_AES_GCM) /* macOS */
	{ENCR_AES_GCM_ICV16,		SADB_X_EALG_AES_GCM			},
#endif
#ifdef SADB_X_EALG_CAMELLIACBC
	{ENCR_CAMELLIA_CBC,			SADB_X_EALG_CAMELLIACBC		},
#endif
#ifdef SADB_X_EALG_CHACHA20POLY1305
	{ENCR_CHACHA20_POLY1305,	SADB_X_EALG_CHACHA20POLY1305},
#endif
	{END_OF_LIST,				0							},
};

/**
 * Algorithms for integrity protection
 */
static kernel_algorithm_t integrity_algs[] = {
	{AUTH_HMAC_MD5_96,			SADB_AALG_MD5HMAC			},
	{AUTH_HMAC_SHA1_96,			SADB_AALG_SHA1HMAC			},
	{AUTH_HMAC_SHA2_256_128,	SADB_X_AALG_SHA2_256HMAC	},
	{AUTH_HMAC_SHA2_384_192,	SADB_X_AALG_SHA2_384HMAC	},
	{AUTH_HMAC_SHA2_512_256,	SADB_X_AALG_SHA2_512HMAC	},
/*	{AUTH_DES_MAC,				0,							}, */
/*	{AUTH_KPDK_MD5,				0,							}, */
#ifdef SADB_X_AALG_AES_XCBC_MAC
	{AUTH_AES_XCBC_96,			SADB_X_AALG_AES_XCBC_MAC,	},
#endif
	{END_OF_LIST,				0,							},
};

/**
 * Algorithms for IPComp, unused yet
 */
static kernel_algorithm_t compression_algs[] = {
/*	{IPCOMP_OUI,				0							}, */
	{IPCOMP_DEFLATE,			SADB_X_CALG_DEFLATE			},
#ifdef SADB_X_CALG_LZS
	{IPCOMP_LZS,				SADB_X_CALG_LZS				},
#endif
#ifdef SADB_X_CALG_LZJH
	{IPCOMP_LZJH,				SADB_X_CALG_LZJH			},
#endif
	{END_OF_LIST,				0							},
};

/**
 * Look up a kernel algorithm ID and its key size
 */
static int lookup_algorithm(transform_type_t type, int ikev2)
{
	kernel_algorithm_t *list;
	uint16_t alg = 0;

	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			list = encryption_algs;
			break;
		case INTEGRITY_ALGORITHM:
			list = integrity_algs;
			break;
		case COMPRESSION_ALGORITHM:
			list = compression_algs;
			break;
		default:
			return 0;
	}
	while (list->ikev2 != END_OF_LIST)
	{
		if (ikev2 == list->ikev2)
		{
			return list->kernel;
		}
		list++;
	}
	charon->kernel->lookup_algorithm(charon->kernel, ikev2, type, &alg, NULL);
	return alg;
}

/**
 * Helper to set a port in a sockaddr_t, the port has to be in host order
 */
static void set_port(sockaddr_t *addr, uint16_t port)
{
	switch (addr->sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in*)addr;
			sin->sin_port = htons(port);
			break;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)addr;
			sin6->sin6_port = htons(port);
			break;
		}
	}
}

/**
 * Copy a host_t as sockaddr_t to the given memory location.
 * @return		the number of bytes copied
 */
static size_t hostcpy(void *dest, host_t *host, bool include_port)
{
	sockaddr_t *addr = host->get_sockaddr(host), *dest_addr = dest;
	socklen_t *len = host->get_sockaddr_len(host);

	memcpy(dest, addr, *len);
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
	dest_addr->sa_len = *len;
#endif
	if (!include_port)
	{
		set_port(dest_addr, 0);
	}
	return *len;
}

/**
 * add a host to the given sadb_msg
 */
static void add_addr_ext(struct sadb_msg *msg, host_t *host, uint16_t type,
						 uint8_t proto, uint8_t prefixlen, bool include_port)
{
	struct sadb_address *addr = (struct sadb_address*)PFKEY_EXT_ADD_NEXT(msg);
	size_t len;

	addr->sadb_address_exttype = type;
	addr->sadb_address_proto = proto;
	addr->sadb_address_prefixlen = prefixlen;
	len = hostcpy(addr + 1, host, include_port);
	addr->sadb_address_len = PFKEY_LEN(sizeof(*addr) + len);
	PFKEY_EXT_ADD(msg, addr);
}

#ifdef HAVE_NATT
/**
 * add udp encap extensions to a sadb_msg
 */
static void add_encap_ext(struct sadb_msg *msg, host_t *src, host_t *dst)
{
	struct sadb_x_nat_t_type* nat_type;
	struct sadb_x_nat_t_port* nat_port;

	nat_type = (struct sadb_x_nat_t_type*)PFKEY_EXT_ADD_NEXT(msg);
	nat_type->sadb_x_nat_t_type_exttype = SADB_X_EXT_NAT_T_TYPE;
	nat_type->sadb_x_nat_t_type_len = PFKEY_LEN(sizeof(*nat_type));
	nat_type->sadb_x_nat_t_type_type = UDP_ENCAP_ESPINUDP;
	PFKEY_EXT_ADD(msg, nat_type);

	nat_port = (struct sadb_x_nat_t_port*)PFKEY_EXT_ADD_NEXT(msg);
	nat_port->sadb_x_nat_t_port_exttype = SADB_X_EXT_NAT_T_SPORT;
	nat_port->sadb_x_nat_t_port_len = PFKEY_LEN(sizeof(*nat_port));
	nat_port->sadb_x_nat_t_port_port = htons(src->get_port(src));
	PFKEY_EXT_ADD(msg, nat_port);

	nat_port = (struct sadb_x_nat_t_port*)PFKEY_EXT_ADD_NEXT(msg);
	nat_port->sadb_x_nat_t_port_exttype = SADB_X_EXT_NAT_T_DPORT;
	nat_port->sadb_x_nat_t_port_len = PFKEY_LEN(sizeof(*nat_port));
	nat_port->sadb_x_nat_t_port_port = htons(dst->get_port(dst));
	PFKEY_EXT_ADD(msg, nat_port);
}
#endif /*HAVE_NATT*/

/**
 * Convert a sadb_address to a traffic_selector
 */
static traffic_selector_t* sadb_address2ts(struct sadb_address *address)
{
	traffic_selector_t *ts;
	host_t *host;
	uint8_t proto;

	proto = address->sadb_address_proto;
	proto = proto == IPSEC_PROTO_ANY ? 0 : proto;

	/* The Linux 2.6 kernel does not set the protocol and port information
	 * in the src and dst sadb_address extensions of the SADB_ACQUIRE message.
	 */
	host = host_create_from_sockaddr((sockaddr_t*)&address[1]);
	ts = traffic_selector_create_from_subnet(host,
											 address->sadb_address_prefixlen,
											 proto, host->get_port(host),
											 host->get_port(host) ?: 65535);
	return ts;
}

/**
 * Parses a pfkey message received from the kernel
 */
static status_t parse_pfkey_message(struct sadb_msg *msg, pfkey_msg_t *out)
{
	struct sadb_ext* ext;
	size_t len;

	memset(out, 0, sizeof(pfkey_msg_t));
	out->msg = msg;

	len = msg->sadb_msg_len;
	len -= PFKEY_LEN(sizeof(struct sadb_msg));

	ext = (struct sadb_ext*)(((char*)msg) + sizeof(struct sadb_msg));

	while (len >= PFKEY_LEN(sizeof(struct sadb_ext)))
	{
		DBG3(DBG_KNL, "  %N", sadb_ext_type_names, ext->sadb_ext_type);
		if (ext->sadb_ext_len < PFKEY_LEN(sizeof(struct sadb_ext)) ||
			ext->sadb_ext_len > len)
		{
			DBG1(DBG_KNL, "length of %N extension is invalid",
						   sadb_ext_type_names, ext->sadb_ext_type);
			break;
		}

		if ((ext->sadb_ext_type > SADB_EXT_MAX) || (!ext->sadb_ext_type))
		{
			DBG1(DBG_KNL, "type of PF_KEY extension (%d) is invalid",
						   ext->sadb_ext_type);
			break;
		}

		if (out->ext[ext->sadb_ext_type])
		{
			DBG1(DBG_KNL, "duplicate %N extension",
						   sadb_ext_type_names, ext->sadb_ext_type);
			break;
		}

		out->ext[ext->sadb_ext_type] = ext;
		ext = PFKEY_EXT_NEXT_LEN(ext, len);
	}

	if (len)
	{
		DBG1(DBG_KNL, "PF_KEY message length is invalid");
		return FAILED;
	}

	return SUCCESS;
}

/**
 * Send a message to a specific PF_KEY socket and handle the response.
 */
static status_t pfkey_send_socket(private_kernel_pfkey_ipsec_t *this, int socket,
					struct sadb_msg *in, struct sadb_msg **out, size_t *out_len)
{
	unsigned char buf[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg;
	int in_len, len;

	this->mutex_pfkey->lock(this->mutex_pfkey);

	/* the kernel may broadcast messages not related to our requests (e.g. when
	 * managing SAs and policies via an external tool), so let's clear the
	 * receive buffer so there is room for our request and its reply. */
	while (TRUE)
	{
		len = recv(socket, buf, sizeof(buf), MSG_DONTWAIT);

		if (len < 0)
		{
			if (errno == EINTR)
			{	/* interrupted, try again */
				continue;
			}
			break;
		}
	}

	/* FIXME: our usage of sequence numbers is probably wrong. check RFC 2367,
	 * in particular the behavior in response to an SADB_ACQUIRE. */
	in->sadb_msg_seq = ++this->seq;
	in->sadb_msg_pid = getpid();

	in_len = PFKEY_USER_LEN(in->sadb_msg_len);

	while (TRUE)
	{
		len = send(socket, in, in_len, 0);

		if (len != in_len)
		{
			if (errno == EINTR)
			{
				/* interrupted, try again */
				continue;
			}
			this->mutex_pfkey->unlock(this->mutex_pfkey);
			DBG1(DBG_KNL, "error sending to PF_KEY socket: %s",
						   strerror(errno));
			return FAILED;
		}
		break;
	}

	while (TRUE)
	{
		msg = (struct sadb_msg*)buf;

		len = recv(socket, buf, sizeof(buf), 0);

		if (len < 0)
		{
			if (errno == EINTR)
			{
				DBG1(DBG_KNL, "got interrupted");
				/* interrupted, try again */
				continue;
			}
			DBG1(DBG_KNL, "error reading from PF_KEY socket: %s",
						   strerror(errno));
			this->mutex_pfkey->unlock(this->mutex_pfkey);
			return FAILED;
		}
		if (len < sizeof(struct sadb_msg) ||
			msg->sadb_msg_len < PFKEY_LEN(sizeof(struct sadb_msg)))
		{
			DBG1(DBG_KNL, "received corrupted PF_KEY message");
			this->mutex_pfkey->unlock(this->mutex_pfkey);
			return FAILED;
		}
		if (msg->sadb_msg_len > len / PFKEY_ALIGNMENT)
		{
			DBG1(DBG_KNL, "buffer was too small to receive the complete PF_KEY "
					      "message");
			this->mutex_pfkey->unlock(this->mutex_pfkey);
			return FAILED;
		}
		if (msg->sadb_msg_pid != in->sadb_msg_pid)
		{
			DBG2(DBG_KNL, "received PF_KEY message is not intended for us");
			continue;
		}
		if (msg->sadb_msg_seq != this->seq)
		{
			DBG2(DBG_KNL, "received PF_KEY message with unexpected sequence "
						  "number, was %d expected %d", msg->sadb_msg_seq,
						  this->seq);
			if (msg->sadb_msg_seq == 0)
			{
				/* FreeBSD and Mac OS X do this for the response to
				 * SADB_X_SPDGET (but not for the response to SADB_GET).
				 * FreeBSD: 'key_spdget' in /usr/src/sys/netipsec/key.c. */
			}
			else if (msg->sadb_msg_seq < this->seq)
			{
				continue;
			}
			else
			{
				this->mutex_pfkey->unlock(this->mutex_pfkey);
				return FAILED;
			}
		}
		if (msg->sadb_msg_type != in->sadb_msg_type)
		{
			DBG2(DBG_KNL, "received PF_KEY message of wrong type, "
						  "was %d expected %d, ignoring", msg->sadb_msg_type,
						   in->sadb_msg_type);
		}
		break;
	}

	*out_len = len;
	*out = (struct sadb_msg*)malloc(len);
	memcpy(*out, buf, len);

	this->mutex_pfkey->unlock(this->mutex_pfkey);
	return SUCCESS;
}

/**
 * Send a message to the default PF_KEY socket and handle the response.
 */
static status_t pfkey_send(private_kernel_pfkey_ipsec_t *this,
						   struct sadb_msg *in, struct sadb_msg **out,
						   size_t *out_len)
{
	return pfkey_send_socket(this, this->socket, in, out, out_len);
}

/**
 * Process a SADB_ACQUIRE message from the kernel
 */
static void process_acquire(private_kernel_pfkey_ipsec_t *this,
							struct sadb_msg* msg)
{
	pfkey_msg_t response;
	kernel_acquire_data_t data = {};
	uint32_t index, reqid = 0;
	policy_entry_t *policy;
	policy_sa_t *sa;

	switch (msg->sadb_msg_satype)
	{
		case SADB_SATYPE_UNSPEC:
		case SADB_SATYPE_ESP:
		case SADB_SATYPE_AH:
			break;
		default:
			/* acquire for AH/ESP only */
			return;
	}
	DBG2(DBG_KNL, "received an SADB_ACQUIRE");

	if (parse_pfkey_message(msg, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "parsing SADB_ACQUIRE from kernel failed");
		return;
	}

	if (response.x_sa2)
	{
		reqid = response.x_sa2->sadb_x_sa2_reqid;
	}
	else
	{
		index = response.x_policy->sadb_x_policy_id;
		this->mutex->lock(this->mutex);
		if (this->policies->find_first(this->policies, policy_entry_match_byindex,
									   (void**)&policy, index) &&
			policy->used_by->get_first(policy->used_by, (void**)&sa) == SUCCESS)
		{
			reqid = sa->sa->cfg.reqid;
		}
		else
		{
			DBG1(DBG_KNL, "received an SADB_ACQUIRE with policy id %d but no "
				 "matching policy found", index);
		}
		this->mutex->unlock(this->mutex);
	}

	if (reqid)
	{
		data.src = sadb_address2ts(response.src);
		data.dst = sadb_address2ts(response.dst);

		charon->kernel->acquire(charon->kernel, reqid, &data);

		data.src->destroy(data.src);
		data.dst->destroy(data.dst);
	}
}

/**
 * Process a SADB_EXPIRE message from the kernel
 */
static void process_expire(private_kernel_pfkey_ipsec_t *this,
						   struct sadb_msg* msg)
{
	pfkey_msg_t response;
	uint8_t protocol;
	uint32_t spi;
	host_t *dst;
	bool hard;

	DBG2(DBG_KNL, "received an SADB_EXPIRE");

	if (parse_pfkey_message(msg, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "parsing SADB_EXPIRE from kernel failed");
		return;
	}

	protocol = satype2proto(msg->sadb_msg_satype);
	spi = response.sa->sadb_sa_spi;
	hard = response.lft_hard != NULL;

	if (protocol == IPPROTO_ESP || protocol == IPPROTO_AH)
	{
		dst = host_create_from_sockaddr((sockaddr_t*)(response.dst + 1));
		if (dst)
		{
			charon->kernel->expire(charon->kernel, protocol, spi, dst, hard);
			dst->destroy(dst);
		}
	}
}

#ifdef SADB_X_MIGRATE
/**
 * Process a SADB_X_MIGRATE message from the kernel
 */
static void process_migrate(private_kernel_pfkey_ipsec_t *this,
							struct sadb_msg* msg)
{
	pfkey_msg_t response;
	traffic_selector_t *src_ts, *dst_ts;
	policy_dir_t dir;
	uint32_t reqid = 0;
	host_t *local = NULL, *remote = NULL;

	DBG2(DBG_KNL, "received an SADB_X_MIGRATE");

	if (parse_pfkey_message(msg, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "parsing SADB_X_MIGRATE from kernel failed");
		return;
	}
	src_ts = sadb_address2ts(response.src);
	dst_ts = sadb_address2ts(response.dst);
	dir = kernel2dir(response.x_policy->sadb_x_policy_dir);
	DBG2(DBG_KNL, "  policy %R === %R %N, id %u", src_ts, dst_ts,
					 policy_dir_names, dir);

	/* SADB_X_EXT_KMADDRESS is not present in unpatched kernels < 2.6.28 */
	if (response.x_kmaddress)
	{
		sockaddr_t *local_addr, *remote_addr;
		uint32_t local_len;

		local_addr  = (sockaddr_t*)&response.x_kmaddress[1];
		local = host_create_from_sockaddr(local_addr);
		local_len = (local_addr->sa_family == AF_INET6)?
					sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
		remote_addr = (sockaddr_t*)((uint8_t*)local_addr + local_len);
		remote = host_create_from_sockaddr(remote_addr);
		DBG2(DBG_KNL, "  kmaddress: %H...%H", local, remote);
	}

	if (src_ts && dst_ts && local && remote)
	{
		charon->kernel->migrate(charon->kernel, reqid, src_ts, dst_ts, dir,
								local, remote);
	}
	else
	{
		DESTROY_IF(src_ts);
		DESTROY_IF(dst_ts);
		DESTROY_IF(local);
		DESTROY_IF(remote);
	}
}
#endif /*SADB_X_MIGRATE*/

#ifdef SADB_X_NAT_T_NEW_MAPPING
/**
 * Process a SADB_X_NAT_T_NEW_MAPPING message from the kernel
 */
static void process_mapping(private_kernel_pfkey_ipsec_t *this,
							struct sadb_msg* msg)
{
	pfkey_msg_t response;
	uint32_t spi;
	sockaddr_t *sa;
	host_t *dst, *new;

	DBG2(DBG_KNL, "received an SADB_X_NAT_T_NEW_MAPPING");

	if (parse_pfkey_message(msg, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "parsing SADB_X_NAT_T_NEW_MAPPING from kernel failed");
		return;
	}

	if (!response.x_sa2)
	{
		DBG1(DBG_KNL, "received SADB_X_NAT_T_NEW_MAPPING is missing required "
					  "information");
		return;
	}

	spi = response.sa->sadb_sa_spi;

	if (satype2proto(msg->sadb_msg_satype) != IPPROTO_ESP)
	{
		return;
	}

	sa = (sockaddr_t*)(response.dst + 1);
	dst = host_create_from_sockaddr(sa);
	switch (sa->sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in*)sa;
			sin->sin_port = htons(response.x_natt_dport->sadb_x_nat_t_port_port);
			break;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;
			sin6->sin6_port = htons(response.x_natt_dport->sadb_x_nat_t_port_port);
			break;
		}
		default:
			break;
	}
	if (dst)
	{
		new = host_create_from_sockaddr(sa);
		if (new)
		{
			charon->kernel->mapping(charon->kernel, IPPROTO_ESP, spi, dst, new);
			new->destroy(new);
		}
		dst->destroy(dst);
	}
}
#endif /*SADB_X_NAT_T_NEW_MAPPING*/

/**
 * Receives events from kernel
 */
static bool receive_events(private_kernel_pfkey_ipsec_t *this, int fd,
						   watcher_event_t event)
{
	unsigned char buf[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg = (struct sadb_msg*)buf;
	int len;

	len = recvfrom(this->socket_events, buf, sizeof(buf), MSG_DONTWAIT, NULL, 0);
	if (len < 0)
	{
		switch (errno)
		{
			case EINTR:
				/* interrupted, try again */
				return TRUE;
			case EAGAIN:
				/* no data ready, select again */
				return TRUE;
			default:
				DBG1(DBG_KNL, "unable to receive from PF_KEY event socket");
				sleep(1);
				return TRUE;
		}
	}

	if (len < sizeof(struct sadb_msg) ||
		msg->sadb_msg_len < PFKEY_LEN(sizeof(struct sadb_msg)))
	{
		DBG2(DBG_KNL, "received corrupted PF_KEY message");
		return TRUE;
	}
	if (msg->sadb_msg_pid != 0)
	{	/* not from kernel. not interested, try another one */
		return TRUE;
	}
	if (msg->sadb_msg_len > len / PFKEY_ALIGNMENT)
	{
		DBG1(DBG_KNL, "buffer was too small to receive the complete "
					  "PF_KEY message");
		return TRUE;
	}

	switch (msg->sadb_msg_type)
	{
		case SADB_ACQUIRE:
			process_acquire(this, msg);
			break;
		case SADB_EXPIRE:
			process_expire(this, msg);
			break;
#ifdef SADB_X_MIGRATE
		case SADB_X_MIGRATE:
			process_migrate(this, msg);
			break;
#endif /*SADB_X_MIGRATE*/
#ifdef SADB_X_NAT_T_NEW_MAPPING
		case SADB_X_NAT_T_NEW_MAPPING:
			process_mapping(this, msg);
			break;
#endif /*SADB_X_NAT_T_NEW_MAPPING*/
		default:
			break;
	}

	return TRUE;
}

/**
 * Get an SPI for a specific protocol from the kernel.
 */

static status_t get_spi_internal(private_kernel_pfkey_ipsec_t *this,
	host_t *src, host_t *dst, uint8_t proto, uint32_t min, uint32_t max,
	uint32_t *spi)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_spirange *range;
	pfkey_msg_t response;
	uint32_t received_spi = 0;
	size_t len;

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_GETSPI;
	msg->sadb_msg_satype = proto2satype(proto);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	add_addr_ext(msg, src, SADB_EXT_ADDRESS_SRC, 0, 0, FALSE);
	add_addr_ext(msg, dst, SADB_EXT_ADDRESS_DST, 0, 0, FALSE);

	range = (struct sadb_spirange*)PFKEY_EXT_ADD_NEXT(msg);
	range->sadb_spirange_exttype = SADB_EXT_SPIRANGE;
	range->sadb_spirange_len = PFKEY_LEN(sizeof(struct sadb_spirange));
	range->sadb_spirange_min = min;
	range->sadb_spirange_max = max;
	PFKEY_EXT_ADD(msg, range);

	if (pfkey_send(this, msg, &out, &len) == SUCCESS)
	{
		if (out->sadb_msg_errno)
		{
			DBG1(DBG_KNL, "allocating SPI failed: %s (%d)",
						   strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		}
		else if (parse_pfkey_message(out, &response) == SUCCESS)
		{
			received_spi = response.sa->sadb_sa_spi;
		}
		free(out);
	}

	if (received_spi == 0)
	{
		return FAILED;
	}

	*spi = received_spi;
	return SUCCESS;
}

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_kernel_pfkey_ipsec_t *this, host_t *src, host_t *dst,
	uint8_t protocol, uint32_t *spi)
{
	uint32_t spi_min, spi_max;

	spi_min = lib->settings->get_int(lib->settings, "%s.spi_min",
									 KERNEL_SPI_MIN, lib->ns);
	spi_max = lib->settings->get_int(lib->settings, "%s.spi_max",
									 KERNEL_SPI_MAX, lib->ns);

	if (get_spi_internal(this, src, dst, protocol, min(spi_min, spi_max),
						 max(spi_min, spi_max), spi) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to get SPI");
		return FAILED;
	}

	DBG2(DBG_KNL, "got SPI %.8x", ntohl(*spi));
	return SUCCESS;
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_kernel_pfkey_ipsec_t *this, host_t *src, host_t *dst,
	uint16_t *cpi)
{
	uint32_t received_spi = 0;

	DBG2(DBG_KNL, "getting CPI");

	if (get_spi_internal(this, src, dst, IPPROTO_COMP,
						 0x100, 0xEFFF, &received_spi) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to get CPI");
		return FAILED;
	}

	*cpi = htons((uint16_t)ntohl(received_spi));

	DBG2(DBG_KNL, "got CPI %.4x", ntohs(*cpi));
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_pfkey_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out = NULL;
	struct sadb_sa *sa;
	struct sadb_x_sa2 *sa2;
	struct sadb_lifetime *lft;
	struct sadb_key *key;
	size_t len;
	uint16_t ipcomp = data->ipcomp;
	ipsec_mode_t mode = data->mode;
	status_t status = FAILED;

	/* if IPComp is used, we install an additional IPComp SA. if the cpi is 0
	 * we are in the recursive call below */
	if (ipcomp != IPCOMP_NONE && data->cpi != 0)
	{
		lifetime_cfg_t lft = {{0,0,0},{0,0,0},{0,0,0}};
		kernel_ipsec_sa_id_t ipcomp_id = {
			.src = id->src,
			.dst = id->dst,
			.spi = htonl(ntohs(data->cpi)),
			.proto = IPPROTO_COMP,
			.mark = id->mark,
		};
		kernel_ipsec_add_sa_t ipcomp_sa = {
			.reqid = data->reqid,
			.mode = data->mode,
			.src_ts = data->src_ts,
			.dst_ts = data->dst_ts,
			.lifetime = &lft,
			.enc_alg = ENCR_UNDEFINED,
			.int_alg = AUTH_UNDEFINED,
			.tfc = data->tfc,
			.ipcomp = data->ipcomp,
			.initiator = data->initiator,
			.inbound = data->inbound,
			.update = data->update,
		};
		add_sa(this, &ipcomp_id, &ipcomp_sa);
		ipcomp = IPCOMP_NONE;
		/* use transport mode ESP SA, IPComp uses tunnel mode */
		mode = MODE_TRANSPORT;
	}

	if (data->update)
	{
		/* As we didn't know the reqid during SPI allocation, we used reqid
		 * zero. Unfortunately we can't SADB_UPDATE to the new reqid, hence we
		 * have to delete the SPI allocation state manually. The reqid
		 * selector does not count for that, therefore we have to delete
		 * that state before installing the new SA to avoid deleting the
		 * the new state after installing it. */
		kernel_ipsec_sa_id_t del_id = {
			.src = id->src,
			.dst = id->dst,
			.spi = id->spi,
			.proto = id->proto,
		};
		kernel_ipsec_del_sa_t del = { 0 };

		if (this->public.interface.del_sa(&this->public.interface, &del_id,
										  &del) != SUCCESS)
		{
			DBG1(DBG_KNL, "deleting SPI allocation SA failed");
		}
	}

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "adding SAD entry with SPI %.8x and reqid {%u}",
		 ntohl(id->spi), data->reqid);

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_ADD;
	msg->sadb_msg_satype = proto2satype(id->proto);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

#ifdef __APPLE__
	if (data->encap)
	{
		struct sadb_sa_2 *sa_2;
		sa_2 = (struct sadb_sa_2*)PFKEY_EXT_ADD_NEXT(msg);
		sa_2->sadb_sa_natt_port = id->dst->get_port(id->dst);
		sa = &sa_2->sa;
		sa->sadb_sa_flags |= SADB_X_EXT_NATT;
		len = sizeof(struct sadb_sa_2);
	}
	else
#endif
	{
		sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
		len = sizeof(struct sadb_sa);
	}
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(len);
	sa->sadb_sa_spi = id->spi;
	sa->sadb_sa_state = SADB_SASTATE_MATURE;
	if (id->proto == IPPROTO_COMP)
	{
		sa->sadb_sa_encrypt = lookup_algorithm(COMPRESSION_ALGORITHM,
											   ipcomp);
	}
	else
	{
		/* Linux interprets sadb_sa_replay as number of packets/bits in the
		 * replay window, whereas on BSD it's the size of the window in bytes.
		 * Only set for the inbound SA as it's not relevant for the outbound
		 * SA and might waste memory with large windows. */
		if (data->inbound)
		{
#ifdef __linux__
			sa->sadb_sa_replay = min(data->replay_window, 32);
#else
			sa->sadb_sa_replay = min((data->replay_window + 7) / 8, UINT8_MAX);
#endif
		}
		if (data->esn)
		{
#ifdef SADB_X_SAFLAGS_ESN
			DBG2(DBG_KNL, "  using extended sequence numbers (ESN)");
			sa->sadb_sa_flags |= SADB_X_SAFLAGS_ESN;
#else
			DBG1(DBG_KNL, "extended sequence numbers (ESN) not supported by "
				 "kernel!");
			goto failed;
#endif
		}
		sa->sadb_sa_auth = lookup_algorithm(INTEGRITY_ALGORITHM, data->int_alg);
		sa->sadb_sa_encrypt = lookup_algorithm(ENCRYPTION_ALGORITHM,
											   data->enc_alg);
	}
	PFKEY_EXT_ADD(msg, sa);

#ifdef SADB_X_EXT_SA_REPLAY
	if (data->inbound)
	{
		struct sadb_x_sa_replay *repl;

		repl = (struct sadb_x_sa_replay*)PFKEY_EXT_ADD_NEXT(msg);
		repl->sadb_x_sa_replay_exttype = SADB_X_EXT_SA_REPLAY;
		repl->sadb_x_sa_replay_len = PFKEY_LEN(sizeof(struct sadb_x_sa_replay));
		repl->sadb_x_sa_replay_replay = min(data->replay_window, UINT32_MAX-32);
		PFKEY_EXT_ADD(msg, repl);
	}
#endif

	sa2 = (struct sadb_x_sa2*)PFKEY_EXT_ADD_NEXT(msg);
	sa2->sadb_x_sa2_exttype = SADB_X_EXT_SA2;
	sa2->sadb_x_sa2_len = PFKEY_LEN(sizeof(struct sadb_spirange));
	sa2->sadb_x_sa2_mode = mode2kernel(mode);
	sa2->sadb_x_sa2_reqid = data->reqid;
	PFKEY_EXT_ADD(msg, sa2);

	add_addr_ext(msg, id->src, SADB_EXT_ADDRESS_SRC, 0, 0, FALSE);
	add_addr_ext(msg, id->dst, SADB_EXT_ADDRESS_DST, 0, 0, FALSE);

	lft = (struct sadb_lifetime*)PFKEY_EXT_ADD_NEXT(msg);
	lft->sadb_lifetime_exttype = SADB_EXT_LIFETIME_SOFT;
	lft->sadb_lifetime_len = PFKEY_LEN(sizeof(struct sadb_lifetime));
	lft->sadb_lifetime_allocations = data->lifetime->packets.rekey;
	lft->sadb_lifetime_bytes = data->lifetime->bytes.rekey;
	lft->sadb_lifetime_addtime = data->lifetime->time.rekey;
	lft->sadb_lifetime_usetime = 0; /* we only use addtime */
	PFKEY_EXT_ADD(msg, lft);

	lft = (struct sadb_lifetime*)PFKEY_EXT_ADD_NEXT(msg);
	lft->sadb_lifetime_exttype = SADB_EXT_LIFETIME_HARD;
	lft->sadb_lifetime_len = PFKEY_LEN(sizeof(struct sadb_lifetime));
	lft->sadb_lifetime_allocations = data->lifetime->packets.life;
	lft->sadb_lifetime_bytes = data->lifetime->bytes.life;
	lft->sadb_lifetime_addtime = data->lifetime->time.life;
	lft->sadb_lifetime_usetime = 0; /* we only use addtime */
	PFKEY_EXT_ADD(msg, lft);

	if (data->enc_alg != ENCR_UNDEFINED)
	{
		if (!sa->sadb_sa_encrypt)
		{
			DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
				 encryption_algorithm_names, data->enc_alg);
			goto failed;
		}
		DBG2(DBG_KNL, "  using encryption algorithm %N with key size %d",
			 encryption_algorithm_names, data->enc_alg, data->enc_key.len * 8);

		key = (struct sadb_key*)PFKEY_EXT_ADD_NEXT(msg);
		key->sadb_key_exttype = SADB_EXT_KEY_ENCRYPT;
		key->sadb_key_bits = data->enc_key.len * 8;
		key->sadb_key_len = PFKEY_LEN(sizeof(struct sadb_key) + data->enc_key.len);
		memcpy(key + 1, data->enc_key.ptr, data->enc_key.len);

		PFKEY_EXT_ADD(msg, key);
	}

	if (data->int_alg != AUTH_UNDEFINED)
	{
		if (!sa->sadb_sa_auth)
		{
			DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
				 integrity_algorithm_names, data->int_alg);
			goto failed;
		}
		DBG2(DBG_KNL, "  using integrity algorithm %N with key size %d",
			 integrity_algorithm_names, data->int_alg, data->int_key.len * 8);

		key = (struct sadb_key*)PFKEY_EXT_ADD_NEXT(msg);
		key->sadb_key_exttype = SADB_EXT_KEY_AUTH;
		key->sadb_key_bits = data->int_key.len * 8;
		key->sadb_key_len = PFKEY_LEN(sizeof(struct sadb_key) + data->int_key.len);
		memcpy(key + 1, data->int_key.ptr, data->int_key.len);

		PFKEY_EXT_ADD(msg, key);
	}

#ifdef HAVE_NATT
	if (data->encap)
	{
		add_encap_ext(msg, id->src, id->dst);
	}
#endif /*HAVE_NATT*/

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to add SAD entry with SPI %.8x",
			 ntohl(id->spi));
		goto failed;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to add SAD entry with SPI %.8x: %s (%d)",
			 ntohl(id->spi), strerror(out->sadb_msg_errno),
			 out->sadb_msg_errno);
		goto failed;
	}

	status = SUCCESS;

failed:
	memwipe(&request, sizeof(request));
	memwipe(out, len);
	free(out);
	return status;
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_pfkey_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out = NULL;
	struct sadb_sa *sa;
	pfkey_msg_t response;
	size_t len;
	status_t status = FAILED;

	if (data->new_reqid)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x: reqid "
			 "change is not supported", ntohl(id->spi));
		return NOT_SUPPORTED;
	}
#ifndef SADB_X_EXT_NEW_ADDRESS_SRC
	/* we can't update the SA if any of the ip addresses have changed.
	 * that's because we can't use SADB_UPDATE and by deleting and readding the
	 * SA the sequence numbers would get lost */
	if (!id->src->ip_equals(id->src, data->new_src) ||
		!id->dst->ip_equals(id->dst, data->new_dst))
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x: address "
			 "changes are not supported", ntohl(id->spi));
		return NOT_SUPPORTED;
	}
#endif /*SADB_X_EXT_NEW_ADDRESS_SRC*/

	/* if IPComp is used, we first update the IPComp SA */
	if (data->cpi)
	{
		kernel_ipsec_sa_id_t ipcomp_id = {
			.src = id->src,
			.dst = id->dst,
			.spi = htonl(ntohs(data->cpi)),
			.proto = IPPROTO_COMP,
			.mark = id->mark,
		};
		kernel_ipsec_update_sa_t ipcomp = {
			.new_src = data->new_src,
			.new_dst = data->new_dst,
		};
		update_sa(this, &ipcomp_id, &ipcomp);
	}

	memset(&request, 0, sizeof(request));

	DBG3(DBG_KNL, "querying SAD entry with SPI %.8x for update",
		 ntohl(id->spi));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_GET;
	msg->sadb_msg_satype = proto2satype(id->proto);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = id->spi;
	sa->sadb_sa_state = SADB_SASTATE_MATURE;
	PFKEY_EXT_ADD(msg, sa);

	add_addr_ext(msg, id->src, SADB_EXT_ADDRESS_SRC, 0, 0, FALSE);
	add_addr_ext(msg, id->dst, SADB_EXT_ADDRESS_DST, 0, 0, FALSE);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to query SAD entry with SPI %.8x",
			 ntohl(id->spi));
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to query SAD entry with SPI %.8x: %s (%d)",
			 ntohl(id->spi), strerror(out->sadb_msg_errno),
			 out->sadb_msg_errno);
		goto failed;
	}
	else if (parse_pfkey_message(out, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to query SAD entry with SPI %.8x: parsing "
			 "response from kernel failed", ntohl(id->spi));
		goto failed;
	}

	DBG2(DBG_KNL, "updating SAD entry with SPI %.8x from %#H..%#H to %#H..%#H",
		 ntohl(id->spi), id->src, id->dst, data->new_src, data->new_dst);

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_UPDATE;
	msg->sadb_msg_satype = proto2satype(id->proto);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

#ifdef __APPLE__
	{
		struct sadb_sa_2 *sa_2;
		sa_2 = (struct sadb_sa_2*)PFKEY_EXT_ADD_NEXT(msg);
		sa_2->sa.sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa_2));
		memcpy(&sa_2->sa, response.sa, sizeof(struct sadb_sa));
		if (data->new_encap)
		{
			sa_2->sadb_sa_natt_port = data->new_dst->get_port(data->new_dst);
			sa_2->sa.sadb_sa_flags |= SADB_X_EXT_NATT;
		}
	}
#else
	PFKEY_EXT_COPY(msg, response.sa);
#endif
	PFKEY_EXT_COPY(msg, response.x_sa2);

	PFKEY_EXT_COPY(msg, response.src);
	PFKEY_EXT_COPY(msg, response.dst);

	PFKEY_EXT_COPY(msg, response.lft_soft);
	PFKEY_EXT_COPY(msg, response.lft_hard);

#ifndef __FreeBSD__
	/* FreeBSD 11.1 does not allow key updates via SADB_UPDATE for mature SAs */
	if (response.key_encr)
	{
		PFKEY_EXT_COPY(msg, response.key_encr);
	}

	if (response.key_auth)
	{
		PFKEY_EXT_COPY(msg, response.key_auth);
	}
#endif

#ifdef HAVE_NATT
	if (data->new_encap)
	{
		add_encap_ext(msg, data->new_src, data->new_dst);
	}
#endif /*HAVE_NATT*/

#ifdef SADB_X_EXT_NEW_ADDRESS_SRC
	if (!id->src->ip_equals(id->src, data->new_src))
	{
		add_addr_ext(msg, data->new_src, SADB_X_EXT_NEW_ADDRESS_SRC, 0, 0,
					 FALSE);
	}
	if (!id->dst->ip_equals(id->dst, data->new_dst))
	{
		add_addr_ext(msg, data->new_dst, SADB_X_EXT_NEW_ADDRESS_DST, 0, 0,
					 FALSE);
	}
#endif /*SADB_X_EXT_NEW_ADDRESS_SRC*/

	memwipe(out, len);
	free(out);
	out = NULL;

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x",
			 ntohl(id->spi));
		goto failed;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x: %s (%d)",
			 ntohl(id->spi), strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		goto failed;
	}

	status = SUCCESS;
failed:
	memwipe(&request, sizeof(request));
	memwipe(out, len);
	free(out);
	return status;
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_pfkey_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_sa *sa;
	pfkey_msg_t response;
	size_t len;
	status_t status = FAILED;

	memset(&request, 0, sizeof(request));

	DBG3(DBG_KNL, "querying SAD entry with SPI %.8x", ntohl(id->spi));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_GET;
	msg->sadb_msg_satype = proto2satype(id->proto);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = id->spi;
	PFKEY_EXT_ADD(msg, sa);

	/* the Linux Kernel doesn't care for the src address, but other systems do
	 * (e.g. FreeBSD)
	 */
	add_addr_ext(msg, id->src, SADB_EXT_ADDRESS_SRC, 0, 0, FALSE);
	add_addr_ext(msg, id->dst, SADB_EXT_ADDRESS_DST, 0, 0, FALSE);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to query SAD entry with SPI %.8x",
			 ntohl(id->spi));
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to query SAD entry with SPI %.8x: %s (%d)",
			 ntohl(id->spi), strerror(out->sadb_msg_errno),
			 out->sadb_msg_errno);
		goto failed;
	}
	else if (parse_pfkey_message(out, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to query SAD entry with SPI %.8x",
			 ntohl(id->spi));
		goto failed;
	}
	if (bytes)
	{
		*bytes = response.lft_current->sadb_lifetime_bytes;
	}
	if (packets)
	{
		/* at least on Linux and FreeBSD this contains the number of packets */
		*packets = response.lft_current->sadb_lifetime_allocations;
	}
	if (time)
	{
#ifdef __APPLE__
		/* OS X uses the "last" time of use in usetime */
		*time = response.lft_current->sadb_lifetime_usetime;
#else /* !__APPLE__ */
		/* on Linux, sadb_lifetime_usetime is set to the "first" time of use,
		 * which is actually correct according to PF_KEY. We have to query
		 * policies for the last usetime. */
		*time = 0;
#endif /* !__APPLE__ */
	}

	status = SUCCESS;
failed:
	memwipe(out, len);
	free(out);
	return status;
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_kernel_pfkey_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_sa *sa;
	size_t len;

	/* if IPComp was used, we first delete the additional IPComp SA */
	if (data->cpi)
	{
		kernel_ipsec_sa_id_t ipcomp_id = {
			.src = id->src,
			.dst = id->dst,
			.spi = htonl(ntohs(data->cpi)),
			.proto = IPPROTO_COMP,
			.mark = id->mark,
		};
		kernel_ipsec_del_sa_t ipcomp = { 0 };
		del_sa(this, &ipcomp_id, &ipcomp);
	}

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "deleting SAD entry with SPI %.8x", ntohl(id->spi));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_DELETE;
	msg->sadb_msg_satype = proto2satype(id->proto);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = id->spi;
	PFKEY_EXT_ADD(msg, sa);

	/* the Linux Kernel doesn't care for the src address, but other systems do
	 * (e.g. FreeBSD)
	 */
	add_addr_ext(msg, id->src, SADB_EXT_ADDRESS_SRC, 0, 0, FALSE);
	add_addr_ext(msg, id->dst, SADB_EXT_ADDRESS_DST, 0, 0, FALSE);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to delete SAD entry with SPI %.8x",
			 ntohl(id->spi));
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to delete SAD entry with SPI %.8x: %s (%d)",
			 ntohl(id->spi), strerror(out->sadb_msg_errno),
			 out->sadb_msg_errno);
		free(out);
		return FAILED;
	}

	DBG2(DBG_KNL, "deleted SAD entry with SPI %.8x", ntohl(id->spi));
	free(out);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, flush_sas, status_t,
	private_kernel_pfkey_ipsec_t *this)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct {
		uint8_t proto;
		char *name;
	} protos[] = {
		{ SADB_SATYPE_AH, "AH" },
		{ SADB_SATYPE_ESP, "ESP" },
		{ SADB_X_SATYPE_IPCOMP, "IPComp" },
	};
	size_t len;
	int i;

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_FLUSH;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	for (i = 0; i < countof(protos); i++)
	{
		DBG2(DBG_KNL, "flushing all %s SAD entries", protos[i].name);

		msg->sadb_msg_satype = protos[i].proto;
		if (pfkey_send(this, msg, &out, &len) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to flush %s SAD entries", protos[i].name);
			return FAILED;
		}
		else if (out->sadb_msg_errno)
		{
			DBG1(DBG_KNL, "unable to flush %s SAD entries: %s (%d)",
				 protos[i].name, strerror(out->sadb_msg_errno),
				 out->sadb_msg_errno);
			free(out);
			return FAILED;
		}
		free(out);
	}
	return SUCCESS;
}

/**
 * Add an explicit exclude route to a routing entry
 */
static void add_exclude_route(private_kernel_pfkey_ipsec_t *this,
							  route_entry_t *route, host_t *src, host_t *dst)
{
	enumerator_t *enumerator;
	exclude_route_t *exclude;
	host_t *gtw;

	enumerator = this->excludes->create_enumerator(this->excludes);
	while (enumerator->enumerate(enumerator, &exclude))
	{
		if (dst->ip_equals(dst, exclude->dst))
		{
			route->exclude = exclude;
			exclude->refs++;
		}
	}
	enumerator->destroy(enumerator);

	if (!route->exclude)
	{
		DBG2(DBG_KNL, "installing new exclude route for %H src %H", dst, src);
		gtw = charon->kernel->get_nexthop(charon->kernel, dst, -1, NULL, NULL);
		if (gtw)
		{
			char *if_name = NULL;

			if (gtw->ip_equals(gtw, dst))
			{
				DBG1(DBG_KNL, "not installing exclude route for directly "
					 "connected peer %H", dst);
			}
			else if (charon->kernel->get_interface(charon->kernel, src, &if_name) &&
					 charon->kernel->add_route(charon->kernel,
									dst->get_address(dst),
									dst->get_family(dst) == AF_INET ? 32 : 128,
									gtw, src, if_name, FALSE) == SUCCESS)
			{
				INIT(exclude,
					.dst = dst->clone(dst),
					.src = src->clone(src),
					.gtw = gtw->clone(gtw),
					.refs = 1,
				);
				route->exclude = exclude;
				this->excludes->insert_last(this->excludes, exclude);
			}
			else
			{
				DBG1(DBG_KNL, "installing exclude route for %H failed", dst);
			}
			gtw->destroy(gtw);
			free(if_name);
		}
		else
		{
			DBG1(DBG_KNL, "gateway lookup for for %H failed", dst);
		}
	}
}

/**
 * Remove an exclude route attached to a routing entry
 */
static void remove_exclude_route(private_kernel_pfkey_ipsec_t *this,
								 route_entry_t *route)
{
	if (route->exclude)
	{
		enumerator_t *enumerator;
		exclude_route_t *exclude;
		bool removed = FALSE;
		host_t *dst;

		enumerator = this->excludes->create_enumerator(this->excludes);
		while (enumerator->enumerate(enumerator, &exclude))
		{
			if (route->exclude == exclude)
			{
				if (--exclude->refs == 0)
				{
					this->excludes->remove_at(this->excludes, enumerator);
					removed = TRUE;
					break;
				}
			}
		}
		enumerator->destroy(enumerator);

		if (removed)
		{
			char *if_name = NULL;

			dst = route->exclude->dst;
			DBG2(DBG_KNL, "uninstalling exclude route for %H src %H",
				 dst, route->exclude->src);
			if (charon->kernel->get_interface(
									charon->kernel,
									route->exclude->src, &if_name) &&
				charon->kernel->del_route(charon->kernel,
									dst->get_address(dst),
									dst->get_family(dst) == AF_INET ? 32 : 128,
									route->exclude->gtw, route->exclude->src,
									if_name, FALSE) != SUCCESS)
			{
				DBG1(DBG_KNL, "uninstalling exclude route for %H failed", dst);
			}
			exclude_route_destroy(route->exclude);
			free(if_name);
		}
		route->exclude = NULL;
	}
}

/**
 * Try to install a route to the given outbound policy
 */
static bool install_route(private_kernel_pfkey_ipsec_t *this,
						  policy_entry_t *policy, policy_sa_out_t *out)
{
	route_entry_t *route, *old;
	host_t *host, *src, *dst;
	char *out_interface = NULL;
	bool is_virtual;

	if (charon->kernel->get_address_by_ts(charon->kernel, out->src_ts, &host,
										  &is_virtual) != SUCCESS)
	{
		return FALSE;
	}

	INIT(route,
		.prefixlen = policy->dst.mask,
		.src_ip = host,
		.dst_net = chunk_clone(policy->dst.net->get_address(policy->dst.net)),
	);

	src = out->generic.sa->src;
	dst = out->generic.sa->dst;

	if (!dst->is_anyaddr(dst))
	{
		route->gateway = charon->kernel->get_nexthop(charon->kernel, dst, -1,
													 src, &route->if_name);

		/* if the IP is virtual, we install the route over the interface it has
		 * been installed on. Otherwise we use the interface we use for IKE, as
		 * this is required for example on Linux. */
		if (is_virtual || this->route_via_internal)
		{
			out_interface = route->if_name;
			route->if_name = NULL;
			src = route->src_ip;
		}
	}
	else
	{	/* for shunt policies */
		route->gateway = charon->kernel->get_nexthop(charon->kernel,
											policy->dst.net, policy->dst.mask,
											route->src_ip, &route->if_name);

		/* we don't have a source address, use the address we found */
		src = route->src_ip;
	}

	/* get interface for route, using source address */
	if (!route->if_name &&
		!charon->kernel->get_interface(charon->kernel, src, &route->if_name))
	{
		route_entry_destroy(route);
		free(out_interface);
		return FALSE;
	}

	if (policy->route)
	{
		old = policy->route;

		if (route_entry_equals(old, route))
		{	/* such a route already exists */
			route_entry_destroy(route);
			free(out_interface);
			return TRUE;
		}
		/* uninstall previously installed route */
		if (charon->kernel->del_route(charon->kernel, old->dst_net,
								old->prefixlen, old->gateway,
								old->src_ip, old->if_name, FALSE) != SUCCESS)
		{
			DBG1(DBG_KNL, "error uninstalling route installed with policy "
				 "%R === %R %N", out->src_ts, out->dst_ts,
				policy_dir_names, policy->direction);
		}
		route_entry_destroy(old);
		policy->route = NULL;
	}

	/* if we don't route via outbound interface and the remote traffic selector
	 * covers the IKE peer, add an exclude route */
	if (!streq(route->if_name, out_interface) &&
		charon->kernel->get_features(charon->kernel) & KERNEL_REQUIRE_EXCLUDE_ROUTE)
	{
		if (out->dst_ts->is_host(out->dst_ts, dst))
		{
			DBG1(DBG_KNL, "can't install route for %R === %R %N, conflicts "
				 "with IKE traffic", out->src_ts, out->dst_ts, policy_dir_names,
				 policy->direction);
			route_entry_destroy(route);
			free(out_interface);
			return FALSE;
		}
		if (out->dst_ts->includes(out->dst_ts, dst))
		{
			add_exclude_route(this, route, out->generic.sa->src, dst);
		}
	}
	free(out_interface);

	DBG2(DBG_KNL, "installing route: %R via %H src %H dev %s",
		 out->dst_ts, route->gateway, route->src_ip, route->if_name);

	switch (charon->kernel->add_route(charon->kernel, route->dst_net,
									  route->prefixlen, route->gateway,
									  route->src_ip, route->if_name, FALSE))
	{
		case ALREADY_DONE:
			/* route exists, do not uninstall */
			remove_exclude_route(this, route);
			route_entry_destroy(route);
			return TRUE;
		case SUCCESS:
			/* cache the installed route */
			policy->route = route;
			return TRUE;
		default:
			DBG1(DBG_KNL, "installing route failed: %R via %H src %H dev %s",
				 out->dst_ts, route->gateway, route->src_ip, route->if_name);
			remove_exclude_route(this, route);
			route_entry_destroy(route);
			return FALSE;
	}
}

/**
 * Check if any significant data has changed to warrant sending an update to
 * the kernel.
 */
static bool policy_update_required(policy_sa_t *current, policy_sa_t *updated)
{
	if (current->type != updated->type
#ifdef HAVE_STRUCT_SADB_X_POLICY_SADB_X_POLICY_PRIORITY
		|| current->priority != updated->priority
#endif
		)
	{
		return TRUE;
	}
	if (current->type == POLICY_IPSEC)
	{
		ipsec_sa_cfg_t *cur = &current->sa->cfg, *upd = &updated->sa->cfg;

		/* we don't use ipsec_sa_cfg_equals() here as e.g. SPIs are not
		 * relevant for this kernel interface, so we don't have to update the
		 * policy during a rekeying */
		if (cur->mode != upd->mode ||
			cur->reqid != upd->reqid ||
			cur->esp.use != upd->esp.use ||
			cur->ah.use != upd->ah.use ||
			cur->ipcomp.transform != upd->ipcomp.transform)
		{
			return TRUE;
		}
		if (cur->mode == MODE_TUNNEL &&
			(!current->sa->src->ip_equals(current->sa->src, updated->sa->src) ||
			 !current->sa->dst->ip_equals(current->sa->dst, updated->sa->dst)))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Add or update a policy in the kernel.
 *
 * Note: The mutex has to be locked when entering this function.
 */
static status_t add_policy_internal(private_kernel_pfkey_ipsec_t *this,
	policy_entry_t *policy, policy_sa_t *mapping, bool update)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_x_policy *pol;
	struct sadb_x_ipsecrequest *req;
	ipsec_sa_t *ipsec = mapping->sa;
	pfkey_msg_t response;
	size_t len;
	ipsec_mode_t proto_mode;
	status_t status;

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = update ? SADB_X_SPDUPDATE : SADB_X_SPDADD;
	msg->sadb_msg_satype = 0;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	pol = (struct sadb_x_policy*)PFKEY_EXT_ADD_NEXT(msg);
	pol->sadb_x_policy_exttype = SADB_X_EXT_POLICY;
	pol->sadb_x_policy_len = PFKEY_LEN(sizeof(struct sadb_x_policy));
	pol->sadb_x_policy_id = 0;
	pol->sadb_x_policy_dir = dir2kernel(policy->direction);
	pol->sadb_x_policy_type = type2kernel(mapping->type);
#ifdef HAVE_STRUCT_SADB_X_POLICY_SADB_X_POLICY_PRIORITY
	pol->sadb_x_policy_priority = mapping->priority;
#endif

	if (mapping->type == POLICY_IPSEC && ipsec->cfg.reqid)
	{
		/* one or more sadb_x_ipsecrequest extensions are added to the
		 * sadb_x_policy extension */
		proto_mode = ipsec->cfg.mode;

		req = (struct sadb_x_ipsecrequest*)(pol + 1);

		if (ipsec->cfg.ipcomp.transform != IPCOMP_NONE)
		{
			req->sadb_x_ipsecrequest_proto = IPPROTO_COMP;

			/* !!! the length here MUST be in octets instead of 64 bit words */
			req->sadb_x_ipsecrequest_len = sizeof(struct sadb_x_ipsecrequest);
			req->sadb_x_ipsecrequest_mode = mode2kernel(ipsec->cfg.mode);
			req->sadb_x_ipsecrequest_reqid = ipsec->cfg.reqid;
			req->sadb_x_ipsecrequest_level = (policy->direction == POLICY_OUT) ?
											IPSEC_LEVEL_UNIQUE : IPSEC_LEVEL_USE;
			if (ipsec->cfg.mode == MODE_TUNNEL)
			{
				len = hostcpy(req + 1, ipsec->src, FALSE);
				req->sadb_x_ipsecrequest_len += len;
				len = hostcpy((char*)(req + 1) + len, ipsec->dst, FALSE);
				req->sadb_x_ipsecrequest_len += len;
				/* use transport mode for other SAs */
				proto_mode = MODE_TRANSPORT;
			}

			pol->sadb_x_policy_len += PFKEY_LEN(req->sadb_x_ipsecrequest_len);
			req = (struct sadb_x_ipsecrequest*)((char*)(req) +
												req->sadb_x_ipsecrequest_len);
		}

		req->sadb_x_ipsecrequest_proto = ipsec->cfg.esp.use ? IPPROTO_ESP
															: IPPROTO_AH;
		/* !!! the length here MUST be in octets instead of 64 bit words */
		req->sadb_x_ipsecrequest_len = sizeof(struct sadb_x_ipsecrequest);
		req->sadb_x_ipsecrequest_mode = mode2kernel(proto_mode);
		req->sadb_x_ipsecrequest_reqid = ipsec->cfg.reqid;
		req->sadb_x_ipsecrequest_level = IPSEC_LEVEL_UNIQUE;
		if (proto_mode == MODE_TUNNEL)
		{
			len = hostcpy(req + 1, ipsec->src, FALSE);
			req->sadb_x_ipsecrequest_len += len;
			len = hostcpy((char*)(req + 1) + len, ipsec->dst, FALSE);
			req->sadb_x_ipsecrequest_len += len;
		}

		pol->sadb_x_policy_len += PFKEY_LEN(req->sadb_x_ipsecrequest_len);
	}
	PFKEY_EXT_ADD(msg, pol);

	add_addr_ext(msg, policy->src.net, SADB_EXT_ADDRESS_SRC, policy->src.proto,
				 policy->src.mask, TRUE);
	add_addr_ext(msg, policy->dst.net, SADB_EXT_ADDRESS_DST, policy->dst.proto,
				 policy->dst.mask, TRUE);

#ifdef __FreeBSD__
	{	/* on FreeBSD a lifetime has to be defined to be able to later query
		 * the current use time. */
		struct sadb_lifetime *lft;
		lft = (struct sadb_lifetime*)PFKEY_EXT_ADD_NEXT(msg);
		lft->sadb_lifetime_exttype = SADB_EXT_LIFETIME_HARD;
		lft->sadb_lifetime_len = PFKEY_LEN(sizeof(struct sadb_lifetime));
		lft->sadb_lifetime_addtime = LONG_MAX;
		PFKEY_EXT_ADD(msg, lft);
	}
#endif

	this->mutex->unlock(this->mutex);

	status = pfkey_send(this, msg, &out, &len);
	if (status == SUCCESS && !update && out->sadb_msg_errno == EEXIST)
	{
		DBG1(DBG_KNL, "policy already exists, try to update it");
		free(out);
		msg->sadb_msg_type = SADB_X_SPDUPDATE;
		status = pfkey_send(this, msg, &out, &len);
	}
	if (status != SUCCESS)
	{
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to %s policy: %s (%d)",
					   update ? "update" : "add", strerror(out->sadb_msg_errno),
					   out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	else if (parse_pfkey_message(out, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to %s policy: parsing response from kernel "
					  "failed", update ? "update" : "add");
		free(out);
		return FAILED;
	}

	/* we try to find the policy again and update the kernel index */
	this->mutex->lock(this->mutex);
	if (!this->policies->find_first(this->policies, NULL, (void**)&policy))
	{
		DBG2(DBG_KNL, "unable to update index, the policy is already gone, "
					  "ignoring");
		this->mutex->unlock(this->mutex);
		free(out);
		return SUCCESS;
	}
	policy->index = response.x_policy->sadb_x_policy_id;
	free(out);

	/* install a route, if:
	 * - this is an outbound policy (to just get one for each child)
	 * - routing is not disabled via strongswan.conf
	 * - the selector is not for a specific protocol/port
	 * - we are in tunnel mode or install a bypass policy
	 */
	if (policy->direction == POLICY_OUT && this->install_routes &&
		policy->src.proto == IPSEC_PROTO_ANY &&
		!policy->src.net->get_port(policy->src.net) &&
		!policy->dst.net->get_port(policy->dst.net))
	{
		if (mapping->type == POLICY_PASS ||
		   (mapping->type == POLICY_IPSEC && ipsec->cfg.mode != MODE_TRANSPORT))
		{
			install_route(this, policy, (policy_sa_out_t*)mapping);
		}
	}
	this->mutex->unlock(this->mutex);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_pfkey_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	policy_entry_t *policy, *found = NULL;
	policy_sa_t *assigned_sa, *current_sa = NULL;
	enumerator_t *enumerator;
	bool update = TRUE;

	if (dir2kernel(id->dir) == IPSEC_DIR_INVALID)
	{	/* FWD policies are not supported on all platforms */
		return SUCCESS;
	}

	/* create a policy */
	policy = create_policy_entry(id->src_ts, id->dst_ts, id->dir);

	/* find a matching policy */
	this->mutex->lock(this->mutex);
	if (this->policies->find_first(this->policies, policy_entry_equals,
								   (void**)&found, policy))
	{	/* use existing policy */
		DBG2(DBG_KNL, "policy %R === %R %N already exists, increasing "
			 "refcount", id->src_ts, id->dst_ts, policy_dir_names, id->dir);
		policy_entry_destroy(policy, this);
		policy = found;
	}
	else
	{	/* use the new one, if we have no such policy */
		this->policies->insert_first(this->policies, policy);
		policy->used_by = linked_list_create();
	}

	/* cache the assigned IPsec SA */
	assigned_sa = policy_sa_create(this, id->dir, data->type, data->src,
								   data->dst, id->src_ts, id->dst_ts, data->sa);
	assigned_sa->auto_priority = get_priority(policy, data->prio);
	assigned_sa->priority = data->manual_prio ? data->manual_prio :
												assigned_sa->auto_priority;


	/* insert the SA according to its priority */
	enumerator = policy->used_by->create_enumerator(policy->used_by);
	while (enumerator->enumerate(enumerator, (void**)&current_sa))
	{
		if (current_sa->priority > assigned_sa->priority)
		{
			break;
		}
		if (current_sa->priority == assigned_sa->priority)
		{
			/* in case of equal manual prios order SAs by automatic priority */
			if (current_sa->auto_priority > assigned_sa->auto_priority)
			{
				break;
			}
			/* prefer SAs with a reqid over those without */
			if (current_sa->auto_priority == assigned_sa->auto_priority &&
				(!current_sa->sa->cfg.reqid || assigned_sa->sa->cfg.reqid))
			{
				break;
			}
		}
		update = FALSE;
	}
	policy->used_by->insert_before(policy->used_by, enumerator, assigned_sa);
	enumerator->destroy(enumerator);

	if (update && current_sa)
	{	/* check if there are actually any relevant changes, if not, we don't
		 * send an update to the kernel as e.g. FreeBSD doesn't do that
		 * atomically, causing unnecessary traffic loss during rekeyings */
		update = policy_update_required(current_sa, assigned_sa);
	}

	if (!update)
	{	/* we don't update the policy if the priority is lower than that of the
		 * currently installed one */
		this->mutex->unlock(this->mutex);
		return SUCCESS;
	}

	DBG2(DBG_KNL, "%s policy %R === %R %N",
		 found ? "updating" : "adding", id->src_ts, id->dst_ts,
		 policy_dir_names, id->dir);

	if (add_policy_internal(this, policy, assigned_sa, found) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to %s policy %R === %R %N",
			 found ? "update" : "add", id->src_ts, id->dst_ts,
			 policy_dir_names, id->dir);
		return FAILED;
	}
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_pfkey_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_x_policy *pol;
	policy_entry_t *policy, *found = NULL;
	pfkey_msg_t response;
	size_t len;

	if (dir2kernel(id->dir) == IPSEC_DIR_INVALID)
	{	/* FWD policies are not supported on all platforms */
		return NOT_FOUND;
	}

	DBG3(DBG_KNL, "querying policy %R === %R %N", id->src_ts, id->dst_ts,
		 policy_dir_names, id->dir);

	/* create a policy */
	policy = create_policy_entry(id->src_ts, id->dst_ts, id->dir);

	/* find a matching policy */
	this->mutex->lock(this->mutex);
	if (!this->policies->find_first(this->policies, policy_entry_equals,
									(void**)&found, policy))
	{
		DBG1(DBG_KNL, "querying policy %R === %R %N failed, not found",
			 id->src_ts, id->dst_ts, policy_dir_names, id->dir);
		policy_entry_destroy(policy, this);
		this->mutex->unlock(this->mutex);
		return NOT_FOUND;
	}
	policy_entry_destroy(policy, this);
	policy = found;

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_X_SPDGET;
	msg->sadb_msg_satype = 0;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	pol = (struct sadb_x_policy*)PFKEY_EXT_ADD_NEXT(msg);
	pol->sadb_x_policy_exttype = SADB_X_EXT_POLICY;
	pol->sadb_x_policy_id = policy->index;
	pol->sadb_x_policy_len = PFKEY_LEN(sizeof(struct sadb_x_policy));
	pol->sadb_x_policy_dir = dir2kernel(id->dir);
	pol->sadb_x_policy_type = IPSEC_POLICY_IPSEC;
	PFKEY_EXT_ADD(msg, pol);

	add_addr_ext(msg, policy->src.net, SADB_EXT_ADDRESS_SRC, policy->src.proto,
				 policy->src.mask, TRUE);
	add_addr_ext(msg, policy->dst.net, SADB_EXT_ADDRESS_DST, policy->dst.proto,
				 policy->dst.mask, TRUE);

	this->mutex->unlock(this->mutex);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to query policy %R === %R %N", id->src_ts,
			 id->dst_ts, policy_dir_names, id->dir);
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to query policy %R === %R %N: %s (%d)",
			 id->src_ts, id->dst_ts, policy_dir_names, id->dir,
			 strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	else if (parse_pfkey_message(out, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to query policy %R === %R %N: parsing response "
			 "from kernel failed", id->src_ts, id->dst_ts, policy_dir_names,
			 id->dir);
		free(out);
		return FAILED;
	}
	else if (response.lft_current == NULL)
	{
		DBG2(DBG_KNL, "unable to query policy %R === %R %N: kernel reports no "
			 "use time", id->src_ts, id->dst_ts, policy_dir_names,
			 id->dir);
		free(out);
		return FAILED;
	}

	/* we need the monotonic time, but the kernel returns system time. */
	if (response.lft_current->sadb_lifetime_usetime)
	{
		*use_time = time_monotonic(NULL) -
					(time(NULL) - response.lft_current->sadb_lifetime_usetime);
	}
	else
	{
		*use_time = 0;
	}
	free(out);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_kernel_pfkey_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_x_policy *pol;
	policy_entry_t *policy, *found = NULL;
	policy_sa_t *mapping, *to_remove = NULL;
	enumerator_t *enumerator;
	bool first = TRUE, is_installed = TRUE;
	uint32_t priority, auto_priority;
	size_t len;
	ipsec_sa_t assigned_sa = {
		.src = data->src,
		.dst = data->dst,
		.cfg = *data->sa,
	};

	if (dir2kernel(id->dir) == IPSEC_DIR_INVALID)
	{	/* FWD policies are not supported on all platforms */
		return SUCCESS;
	}

	DBG2(DBG_KNL, "deleting policy %R === %R %N", id->src_ts, id->dst_ts,
		 policy_dir_names, id->dir);

	/* create a policy */
	policy = create_policy_entry(id->src_ts, id->dst_ts, id->dir);

	/* find a matching policy */
	this->mutex->lock(this->mutex);
	if (!this->policies->find_first(this->policies, policy_entry_equals,
									(void**)&found, policy))
	{
		DBG1(DBG_KNL, "deleting policy %R === %R %N failed, not found",
			 id->src_ts, id->dst_ts, policy_dir_names, id->dir);
		policy_entry_destroy(policy, this);
		this->mutex->unlock(this->mutex);
		return NOT_FOUND;
	}
	policy_entry_destroy(policy, this);
	policy = found;

	/* remove mapping to SA by reqid and priority, if multiple match, which
	 * could happen when rekeying due to an address change, remove the oldest */
	auto_priority = get_priority(policy, data->prio);
	priority = data->manual_prio ? data->manual_prio : auto_priority;
	enumerator = policy->used_by->create_enumerator(policy->used_by);
	while (enumerator->enumerate(enumerator, (void**)&mapping))
	{
		if (priority == mapping->priority &&
			auto_priority == mapping->auto_priority &&
			data->type == mapping->type &&
			ipsec_sa_equals(mapping->sa, &assigned_sa))
		{
			to_remove = mapping;
			is_installed = first;
		}
		else if (priority < mapping->priority)
		{
			break;
		}
		first = FALSE;
	}
	enumerator->destroy(enumerator);
	if (!to_remove)
	{	/* sanity check */
		this->mutex->unlock(this->mutex);
		return SUCCESS;
	}
	policy->used_by->remove(policy->used_by, to_remove, NULL);

	if (policy->used_by->get_count(policy->used_by) > 0)
	{	/* policy is used by more SAs, keep in kernel */
		DBG2(DBG_KNL, "policy still used by another CHILD_SA, not removed");

		if (is_installed)
		{	/* check if there are actually any relevant changes, if not, we do
			 * not send an update to the kernel as e.g. FreeBSD doesn't do that
			 * atomically, causing unnecessary traffic loss during rekeyings */
			policy->used_by->get_first(policy->used_by, (void**)&mapping);
			is_installed = policy_update_required(mapping, to_remove);
		}
		policy_sa_destroy(to_remove, id->dir, this);

		if (!is_installed)
		{	/* no need to update as the policy */
			this->mutex->unlock(this->mutex);
			return SUCCESS;
		}

		DBG2(DBG_KNL, "updating policy %R === %R %N", id->src_ts, id->dst_ts,
			 policy_dir_names, id->dir);
		if (add_policy_internal(this, policy, mapping, TRUE) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to update policy %R === %R %N",
				 id->src_ts, id->dst_ts, policy_dir_names, id->dir);
			return FAILED;
		}
		return SUCCESS;
	}

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_X_SPDDELETE;
	msg->sadb_msg_satype = 0;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	pol = (struct sadb_x_policy*)PFKEY_EXT_ADD_NEXT(msg);
	pol->sadb_x_policy_exttype = SADB_X_EXT_POLICY;
	pol->sadb_x_policy_len = PFKEY_LEN(sizeof(struct sadb_x_policy));
	pol->sadb_x_policy_dir = dir2kernel(id->dir);
	pol->sadb_x_policy_type = type2kernel(to_remove->type);
	PFKEY_EXT_ADD(msg, pol);

	add_addr_ext(msg, policy->src.net, SADB_EXT_ADDRESS_SRC, policy->src.proto,
				 policy->src.mask, TRUE);
	add_addr_ext(msg, policy->dst.net, SADB_EXT_ADDRESS_DST, policy->dst.proto,
				 policy->dst.mask, TRUE);

	if (policy->route)
	{
		route_entry_t *route = policy->route;
		if (charon->kernel->del_route(charon->kernel, route->dst_net,
							route->prefixlen, route->gateway,
							route->src_ip, route->if_name, FALSE) != SUCCESS)
		{
			DBG1(DBG_KNL, "error uninstalling route installed with "
				 "policy %R === %R %N", id->src_ts, id->dst_ts,
				 policy_dir_names, id->dir);
		}
		remove_exclude_route(this, route);
	}

	this->policies->remove(this->policies, found, NULL);
	policy_sa_destroy(to_remove, id->dir, this);
	policy_entry_destroy(policy, this);
	this->mutex->unlock(this->mutex);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to delete policy %R === %R %N", id->src_ts,
			 id->dst_ts, policy_dir_names, id->dir);
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to delete policy %R === %R %N: %s (%d)",
			 id->src_ts, id->dst_ts, policy_dir_names, id->dir,
			 strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, flush_policies, status_t,
	private_kernel_pfkey_ipsec_t *this)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	size_t len;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "flushing all policies from SPD");

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_X_SPDFLUSH;
	msg->sadb_msg_satype = SADB_SATYPE_UNSPEC;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to flush SPD entries");
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to flush SPD entries: %s (%d)",
					   strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);
	return SUCCESS;
}

/**
 * Register a socket for ACQUIRE/EXPIRE messages
 */
static status_t register_pfkey_socket(private_kernel_pfkey_ipsec_t *this,
									  uint8_t satype)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	size_t len;

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_REGISTER;
	msg->sadb_msg_satype = satype;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	if (pfkey_send_socket(this, this->socket_events, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to register PF_KEY socket");
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to register PF_KEY socket: %s (%d)",
					   strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, bypass_socket, bool,
	private_kernel_pfkey_ipsec_t *this, int fd, int family)
{
	struct sadb_x_policy policy;
	u_int sol, ipsec_policy;

	switch (family)
	{
		case AF_INET:
		{
			sol = SOL_IP;
			ipsec_policy = IP_IPSEC_POLICY;
			break;
		}
		case AF_INET6:
		{
			sol = SOL_IPV6;
			ipsec_policy = IPV6_IPSEC_POLICY;
			break;
		}
		default:
			return FALSE;
	}

	memset(&policy, 0, sizeof(policy));
	policy.sadb_x_policy_len = sizeof(policy) / sizeof(uint64_t);
	policy.sadb_x_policy_exttype = SADB_X_EXT_POLICY;
	policy.sadb_x_policy_type = IPSEC_POLICY_BYPASS;

	policy.sadb_x_policy_dir = IPSEC_DIR_OUTBOUND;
	if (setsockopt(fd, sol, ipsec_policy, &policy, sizeof(policy)) < 0)
	{
		DBG1(DBG_KNL, "unable to set IPSEC_POLICY on socket: %s",
					   strerror(errno));
		return FALSE;
	}
	policy.sadb_x_policy_dir = IPSEC_DIR_INBOUND;
	if (setsockopt(fd, sol, ipsec_policy, &policy, sizeof(policy)) < 0)
	{
		DBG1(DBG_KNL, "unable to set IPSEC_POLICY on socket: %s",
					   strerror(errno));
		return FALSE;
	}
	return TRUE;
}

METHOD(kernel_ipsec_t, enable_udp_decap, bool,
	private_kernel_pfkey_ipsec_t *this, int fd, int family, uint16_t port)
{
#ifndef __APPLE__
	int type = UDP_ENCAP_ESPINUDP;

	if (setsockopt(fd, SOL_UDP, UDP_ENCAP, &type, sizeof(type)) < 0)
	{
		DBG1(DBG_KNL, "unable to set UDP_ENCAP: %s", strerror(errno));
		return FALSE;
	}
#else /* __APPLE__ */
	int intport = port;

	if (sysctlbyname("net.inet.ipsec.esp_port", NULL, NULL, &intport,
					 sizeof(intport)) != 0)
	{
		DBG1(DBG_KNL, "could not set net.inet.ipsec.esp_port to %d: %s",
			 port, strerror(errno));
		return FALSE;
	}
#endif /* __APPLE__ */

	return TRUE;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_kernel_pfkey_ipsec_t *this)
{
	if (this->socket > 0)
	{
		close(this->socket);
	}
	if (this->socket_events > 0)
	{
		lib->watcher->remove(lib->watcher, this->socket_events);
		close(this->socket_events);
	}
	this->policies->invoke_function(this->policies, policy_entry_destroy_cb,
									this);
	this->policies->destroy(this->policies);
	this->excludes->destroy(this->excludes);
	this->sas->destroy(this->sas);
	this->mutex->destroy(this->mutex);
	this->mutex_pfkey->destroy(this->mutex_pfkey);
	free(this);
}

/*
 * Described in header.
 */
kernel_pfkey_ipsec_t *kernel_pfkey_ipsec_create()
{
	private_kernel_pfkey_ipsec_t *this;
	bool register_for_events = TRUE;
	int rcv_buffer;

	INIT(this,
		.public = {
			.interface = {
				.get_spi = _get_spi,
				.get_cpi = _get_cpi,
				.add_sa  = _add_sa,
				.update_sa = _update_sa,
				.query_sa = _query_sa,
				.del_sa = _del_sa,
				.flush_sas = _flush_sas,
				.add_policy = _add_policy,
				.query_policy = _query_policy,
				.del_policy = _del_policy,
				.flush_policies = _flush_policies,
				.bypass_socket = _bypass_socket,
				.enable_udp_decap = _enable_udp_decap,
				.destroy = _destroy,
			},
		},
		.policies = linked_list_create(),
		.excludes = linked_list_create(),
		.sas = hashtable_create((hashtable_hash_t)ipsec_sa_hash,
								(hashtable_equals_t)ipsec_sa_equals, 32),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.mutex_pfkey = mutex_create(MUTEX_TYPE_DEFAULT),
		.install_routes = lib->settings->get_bool(lib->settings,
												  "%s.install_routes", TRUE,
												  lib->ns),
		.route_via_internal = lib->settings->get_bool(lib->settings,
								"%s.plugins.kernel-pfkey.route_via_internal",
								FALSE, lib->ns),
	);

	if (streq(lib->ns, "starter"))
	{	/* starter has no threads, so we do not register for kernel events */
		register_for_events = FALSE;
	}

	/* create a PF_KEY socket to communicate with the kernel */
	this->socket = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
	if (this->socket <= 0)
	{
		DBG1(DBG_KNL, "unable to create PF_KEY socket");
		destroy(this);
		return NULL;
	}

	if (register_for_events)
	{
		/* create a PF_KEY socket for ACQUIRE & EXPIRE */
		this->socket_events = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
		if (this->socket_events <= 0)
		{
			DBG1(DBG_KNL, "unable to create PF_KEY event socket");
			destroy(this);
			return NULL;
		}

		rcv_buffer = lib->settings->get_int(lib->settings,
					"%s.plugins.kernel-pfkey.events_buffer_size", 0, lib->ns);
		if (rcv_buffer > 0)
		{
			if (setsockopt(this->socket_events, SOL_SOCKET, SO_RCVBUF,
						   &rcv_buffer, sizeof(rcv_buffer)) == -1)
			{
				DBG1(DBG_KNL, "unable to set receive buffer size on PF_KEY "
					 "event socket: %s", strerror(errno));
			}
		}

		/* register the event socket */
		if (register_pfkey_socket(this, SADB_SATYPE_ESP) != SUCCESS ||
			register_pfkey_socket(this, SADB_SATYPE_AH) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to register PF_KEY event socket");
			destroy(this);
			return NULL;
		}

		lib->watcher->add(lib->watcher, this->socket_events, WATCHER_READ,
						  (watcher_cb_t)receive_events, this);
	}

	return &this->public;
}
