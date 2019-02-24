/*
 * Copyright (C) 2006-2018 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2008-2016 Andreas Steffen
 * Copyright (C) 2006-2007 Fabian Hartmann, Noah Heusser
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
 * HSR Hochschule fuer Technik Rapperswil
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
 * Copyright (C) 2018 Mellanox Technologies.
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

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/ipsec.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/xfrm.h>
#include <linux/udp.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "kernel_netlink_ipsec.h"
#include "kernel_netlink_shared.h"

#include <daemon.h>
#include <utils/debug.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <collections/array.h>
#include <collections/hashtable.h>
#include <collections/linked_list.h>

/** Required for Linux 2.6.26 kernel and later */
#ifndef XFRM_STATE_AF_UNSPEC
#define XFRM_STATE_AF_UNSPEC 32
#endif

/** From linux/in.h */
#ifndef IP_XFRM_POLICY
#define IP_XFRM_POLICY 17
#endif

/** Missing on uclibc */
#ifndef IPV6_XFRM_POLICY
#define IPV6_XFRM_POLICY 34
#endif /*IPV6_XFRM_POLICY*/

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

/**
 * Map the limit for bytes and packets to XFRM_INF by default
 */
#define XFRM_LIMIT(x) ((x) == 0 ? XFRM_INF : (x))

/**
 * Create ORable bitfield of XFRM NL groups
 */
#define XFRMNLGRP(x) (1<<(XFRMNLGRP_##x-1))

/**
 * Returns a pointer to the first rtattr following the nlmsghdr *nlh and the
 * 'usual' netlink data x like 'struct xfrm_usersa_info'
 */
#define XFRM_RTA(nlh, x) ((struct rtattr*)(NLMSG_DATA(nlh) + \
										   NLMSG_ALIGN(sizeof(x))))
/**
 * Returns the total size of attached rta data
 * (after 'usual' netlink data x like 'struct xfrm_usersa_info')
 */
#define XFRM_PAYLOAD(nlh, x) NLMSG_PAYLOAD(nlh, sizeof(x))

typedef struct kernel_algorithm_t kernel_algorithm_t;

/**
 * Mapping of IKEv2 kernel identifier to linux crypto API names
 */
struct kernel_algorithm_t {
	/**
	 * Identifier specified in IKEv2
	 */
	int ikev2;

	/**
	 * Name of the algorithm in linux crypto API
	 */
	const char *name;
};

ENUM(xfrm_msg_names, XFRM_MSG_NEWSA, XFRM_MSG_MAPPING,
	"XFRM_MSG_NEWSA",
	"XFRM_MSG_DELSA",
	"XFRM_MSG_GETSA",
	"XFRM_MSG_NEWPOLICY",
	"XFRM_MSG_DELPOLICY",
	"XFRM_MSG_GETPOLICY",
	"XFRM_MSG_ALLOCSPI",
	"XFRM_MSG_ACQUIRE",
	"XFRM_MSG_EXPIRE",
	"XFRM_MSG_UPDPOLICY",
	"XFRM_MSG_UPDSA",
	"XFRM_MSG_POLEXPIRE",
	"XFRM_MSG_FLUSHSA",
	"XFRM_MSG_FLUSHPOLICY",
	"XFRM_MSG_NEWAE",
	"XFRM_MSG_GETAE",
	"XFRM_MSG_REPORT",
	"XFRM_MSG_MIGRATE",
	"XFRM_MSG_NEWSADINFO",
	"XFRM_MSG_GETSADINFO",
	"XFRM_MSG_NEWSPDINFO",
	"XFRM_MSG_GETSPDINFO",
	"XFRM_MSG_MAPPING"
);

ENUM(xfrm_attr_type_names, XFRMA_UNSPEC, XFRMA_OFFLOAD_DEV,
	"XFRMA_UNSPEC",
	"XFRMA_ALG_AUTH",
	"XFRMA_ALG_CRYPT",
	"XFRMA_ALG_COMP",
	"XFRMA_ENCAP",
	"XFRMA_TMPL",
	"XFRMA_SA",
	"XFRMA_POLICY",
	"XFRMA_SEC_CTX",
	"XFRMA_LTIME_VAL",
	"XFRMA_REPLAY_VAL",
	"XFRMA_REPLAY_THRESH",
	"XFRMA_ETIMER_THRESH",
	"XFRMA_SRCADDR",
	"XFRMA_COADDR",
	"XFRMA_LASTUSED",
	"XFRMA_POLICY_TYPE",
	"XFRMA_MIGRATE",
	"XFRMA_ALG_AEAD",
	"XFRMA_KMADDRESS",
	"XFRMA_ALG_AUTH_TRUNC",
	"XFRMA_MARK",
	"XFRMA_TFCPAD",
	"XFRMA_REPLAY_ESN_VAL",
	"XFRMA_SA_EXTRA_FLAGS",
	"XFRMA_PROTO",
	"XFRMA_ADDRESS_FILTER",
	"XFRMA_PAD",
	"XFRMA_OFFLOAD_DEV",
);

/**
 * Algorithms for encryption
 */
static kernel_algorithm_t encryption_algs[] = {
/*	{ENCR_DES_IV64,				"***"				}, */
	{ENCR_DES,					"des"				},
	{ENCR_3DES,					"des3_ede"			},
/*	{ENCR_RC5,					"***"				}, */
/*	{ENCR_IDEA,					"***"				}, */
	{ENCR_CAST,					"cast5"				},
	{ENCR_BLOWFISH,				"blowfish"			},
/*	{ENCR_3IDEA,				"***"				}, */
/*	{ENCR_DES_IV32,				"***"				}, */
	{ENCR_NULL,					"cipher_null"		},
	{ENCR_AES_CBC,				"aes"				},
	{ENCR_AES_CTR,				"rfc3686(ctr(aes))"	},
	{ENCR_AES_CCM_ICV8,			"rfc4309(ccm(aes))"	},
	{ENCR_AES_CCM_ICV12,		"rfc4309(ccm(aes))"	},
	{ENCR_AES_CCM_ICV16,		"rfc4309(ccm(aes))"	},
	{ENCR_AES_GCM_ICV8,			"rfc4106(gcm(aes))"	},
	{ENCR_AES_GCM_ICV12,		"rfc4106(gcm(aes))"	},
	{ENCR_AES_GCM_ICV16,		"rfc4106(gcm(aes))"	},
	{ENCR_NULL_AUTH_AES_GMAC,	"rfc4543(gcm(aes))"	},
	{ENCR_CAMELLIA_CBC,			"cbc(camellia)"		},
/*	{ENCR_CAMELLIA_CTR,			"***"				}, */
/*	{ENCR_CAMELLIA_CCM_ICV8,	"***"				}, */
/*	{ENCR_CAMELLIA_CCM_ICV12,	"***"				}, */
/*	{ENCR_CAMELLIA_CCM_ICV16,	"***"				}, */
	{ENCR_SERPENT_CBC,			"serpent"			},
	{ENCR_TWOFISH_CBC,			"twofish"			},
	{ENCR_CHACHA20_POLY1305,	"rfc7539esp(chacha20,poly1305)"},
};

/**
 * Algorithms for integrity protection
 */
static kernel_algorithm_t integrity_algs[] = {
	{AUTH_HMAC_MD5_96,			"md5"				},
	{AUTH_HMAC_MD5_128,			"hmac(md5)"			},
	{AUTH_HMAC_SHA1_96,			"sha1"				},
	{AUTH_HMAC_SHA1_160,		"hmac(sha1)"		},
	{AUTH_HMAC_SHA2_256_96,		"sha256"			},
	{AUTH_HMAC_SHA2_256_128,	"hmac(sha256)"		},
	{AUTH_HMAC_SHA2_384_192,	"hmac(sha384)"		},
	{AUTH_HMAC_SHA2_512_256,	"hmac(sha512)"		},
/*	{AUTH_DES_MAC,				"***"				}, */
/*	{AUTH_KPDK_MD5,				"***"				}, */
	{AUTH_AES_XCBC_96,			"xcbc(aes)"			},
	{AUTH_AES_CMAC_96,			"cmac(aes)"			},
};

/**
 * Algorithms for IPComp
 */
static kernel_algorithm_t compression_algs[] = {
/*	{IPCOMP_OUI,				"***"				}, */
	{IPCOMP_DEFLATE,			"deflate"			},
	{IPCOMP_LZS,				"lzs"				},
	{IPCOMP_LZJH,				"lzjh"				},
};

/**
 * IPsec HW offload state in kernel
 */
typedef enum {
	NL_OFFLOAD_UNKNOWN,
	NL_OFFLOAD_UNSUPPORTED,
	NL_OFFLOAD_SUPPORTED
} nl_offload_state_t;

/**
 * Global metadata used for IPsec HW offload
 */
static struct {
	/** bit in feature set */
	u_int bit;
	/** total number of device feature blocks */
	u_int total_blocks;
	/** determined HW offload state */
	nl_offload_state_t state;
} netlink_hw_offload;

/**
 * Look up a kernel algorithm name and its key size
 */
static const char* lookup_algorithm(transform_type_t type, int ikev2)
{
	kernel_algorithm_t *list;
	int i, count;
	char *name;

	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			list = encryption_algs;
			count = countof(encryption_algs);
			break;
		case INTEGRITY_ALGORITHM:
			list = integrity_algs;
			count = countof(integrity_algs);
			break;
		case COMPRESSION_ALGORITHM:
			list = compression_algs;
			count = countof(compression_algs);
			break;
		default:
			return NULL;
	}
	for (i = 0; i < count; i++)
	{
		if (list[i].ikev2 == ikev2)
		{
			return list[i].name;
		}
	}
	if (charon->kernel->lookup_algorithm(charon->kernel, ikev2, type, NULL,
										 &name))
	{
		return name;
	}
	return NULL;
}

typedef struct private_kernel_netlink_ipsec_t private_kernel_netlink_ipsec_t;

/**
 * Private variables and functions of kernel_netlink class.
 */
struct private_kernel_netlink_ipsec_t {
	/**
	 * Public part of the kernel_netlink_t object
	 */
	kernel_netlink_ipsec_t public;

	/**
	 * Mutex to lock access to installed policies
	 */
	mutex_t *mutex;

	/**
	 * Condvar to synchronize access to individual policies
	 */
	condvar_t *condvar;

	/**
	 * Hash table of installed policies (policy_entry_t)
	 */
	hashtable_t *policies;

	/**
	 * Hash table of IPsec SAs using policies (ipsec_sa_t)
	 */
	hashtable_t *sas;

	/**
	 * Netlink xfrm socket (IPsec)
	 */
	netlink_socket_t *socket_xfrm;

	/**
	 * Netlink xfrm socket to receive acquire and expire events
	 */
	int socket_xfrm_events;

	/**
	 * Whether to install routes along policies
	 */
	bool install_routes;

	/**
	 * Whether to set protocol and ports on selector installed with transport
	 * mode IPsec SAs
	 */
	bool proto_port_transport;

	/**
	 * Whether to always use UPDATE to install policies
	 */
	bool policy_update;

	/**
	 * Installed port based IKE bypass policies, as bypass_t
	 */
	array_t *bypass;

	/**
	 * Custom priority calculation function
	 */
	uint32_t (*get_priority)(kernel_ipsec_policy_id_t *id,
							 kernel_ipsec_manage_policy_t *data);
};

typedef struct route_entry_t route_entry_t;

/**
 * Installed routing entry
 */
struct route_entry_t {
	/** Name of the interface the route is bound to */
	char *if_name;

	/** Source ip of the route */
	host_t *src_ip;

	/** Gateway for this route */
	host_t *gateway;

	/** Destination net */
	chunk_t dst_net;

	/** Destination net prefixlen */
	uint8_t prefixlen;
};

/**
 * Destroy a route_entry_t object
 */
static void route_entry_destroy(route_entry_t *this)
{
	free(this->if_name);
	this->src_ip->destroy(this->src_ip);
	DESTROY_IF(this->gateway);
	chunk_free(&this->dst_net);
	free(this);
}

/**
 * Compare two route_entry_t objects
 */
static bool route_entry_equals(route_entry_t *a, route_entry_t *b)
{
	return a->if_name && b->if_name && streq(a->if_name, b->if_name) &&
		   a->src_ip->ip_equals(a->src_ip, b->src_ip) &&
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

	/** Optional mark */
	mark_t mark;

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
						  chunk_hash_inc(chunk_from_thing(sa->mark),
						  chunk_hash(chunk_from_thing(sa->cfg)))));
}

/**
 * Equality function for ipsec_sa_t objects
 */
static bool ipsec_sa_equals(ipsec_sa_t *sa, ipsec_sa_t *other_sa)
{
	return sa->src->ip_equals(sa->src, other_sa->src) &&
		   sa->dst->ip_equals(sa->dst, other_sa->dst) &&
		   sa->mark.value == other_sa->mark.value &&
		   sa->mark.mask == other_sa->mark.mask &&
		   ipsec_sa_cfg_equals(&sa->cfg, &other_sa->cfg);
}

/**
 * Allocate or reference an IPsec SA object
 */
static ipsec_sa_t *ipsec_sa_create(private_kernel_netlink_ipsec_t *this,
								   host_t *src, host_t *dst, mark_t mark,
								   ipsec_sa_cfg_t *cfg)
{
	ipsec_sa_t *sa, *found;
	INIT(sa,
		.src = src,
		.dst = dst,
		.mark = mark,
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
static void ipsec_sa_destroy(private_kernel_netlink_ipsec_t *this,
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

	/** Automatic priority assigned to the policy when installed with this SA */
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
static policy_sa_t *policy_sa_create(private_kernel_netlink_ipsec_t *this,
	policy_dir_t dir, policy_type_t type, host_t *src, host_t *dst,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts, mark_t mark,
	ipsec_sa_cfg_t *cfg)
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
	policy->sa = ipsec_sa_create(this, src, dst, mark, cfg);
	return policy;
}

/**
 * Destroy a policy_sa(_in)_t object
 */
static void policy_sa_destroy(policy_sa_t *policy, policy_dir_t dir,
							  private_kernel_netlink_ipsec_t *this)
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
	private_kernel_netlink_ipsec_t *this;
	policy_dir_t dir;

	VA_ARGS_VGET(args, dir, this);
	policy_sa_destroy(policy, dir, this);
}

typedef struct policy_entry_t policy_entry_t;

/**
 * Installed kernel policy.
 */
struct policy_entry_t {

	/** Direction of this policy: in, out, forward */
	uint8_t direction;

	/** Parameters of installed policy */
	struct xfrm_selector sel;

	/** Optional mark */
	uint32_t mark;

	/** Associated route installed for this policy */
	route_entry_t *route;

	/** List of SAs this policy is used by, ordered by priority */
	linked_list_t *used_by;

	/** reqid for this policy */
	uint32_t reqid;

	/** Number of threads waiting to work on this policy */
	int waiting;

	/** TRUE if a thread is working on this policy */
	bool working;
};

/**
 * Destroy a policy_entry_t object
 */
static void policy_entry_destroy(private_kernel_netlink_ipsec_t *this,
								 policy_entry_t *policy)
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
	free(policy);
}

/**
 * Hash function for policy_entry_t objects
 */
static u_int policy_hash(policy_entry_t *key)
{
	chunk_t chunk = chunk_from_thing(key->sel);
	return chunk_hash_inc(chunk, chunk_hash(chunk_from_thing(key->mark)));
}

/**
 * Equality function for policy_entry_t objects
 */
static bool policy_equals(policy_entry_t *key, policy_entry_t *other_key)
{
	return memeq(&key->sel, &other_key->sel, sizeof(struct xfrm_selector)) &&
		   key->mark == other_key->mark &&
		   key->direction == other_key->direction;
}

/**
 * Determine number of set bits in 16 bit port mask
 */
static inline uint32_t port_mask_bits(uint16_t port_mask)
{
	uint32_t bits;
	uint16_t bit_mask = 0x8000;

	port_mask = ntohs(port_mask);

	for (bits = 0; bits < 16; bits++)
	{
		if (!(port_mask & bit_mask))
		{
			break;
		}
		bit_mask >>= 1;
	}
	return bits;
}

/**
 * Calculate the priority of a policy
 *
 * bits 0-0:  separate trap and regular policies (0..1) 1 bit
 * bits 1-1:  restriction to network interface (0..1)   1 bit
 * bits 2-7:  src + dst port mask bits (2 * 0..16)      6 bits
 * bits 8-8:  restriction to protocol (0..1)            1 bit
 * bits 9-17: src + dst network mask bits (2 * 0..128)  9 bits
 *                                                     18 bits
 *
 * smallest value: 000000000 0 000000 0 0:       0, lowest priority = 200'000
 * largest value : 100000000 1 100000 1 1: 131'459, highst priority =  68'541
 */
static uint32_t get_priority(policy_entry_t *policy, policy_priority_t prio,
							 char *interface)
{
	uint32_t priority = PRIO_BASE, sport_mask_bits, dport_mask_bits;

	switch (prio)
	{
		case POLICY_PRIORITY_FALLBACK:
			priority += PRIO_BASE;
			/* fall-through to next case */
		case POLICY_PRIORITY_ROUTED:
		case POLICY_PRIORITY_DEFAULT:
			priority += PRIO_BASE;
			/* fall-through to next case */
		case POLICY_PRIORITY_PASS:
			break;
	}
	sport_mask_bits = port_mask_bits(policy->sel.sport_mask);
	dport_mask_bits = port_mask_bits(policy->sel.dport_mask);

	/* calculate priority */
	priority -= (policy->sel.prefixlen_s + policy->sel.prefixlen_d) * 512;
	priority -=  policy->sel.proto ? 256 : 0;
	priority -= (sport_mask_bits + dport_mask_bits) * 4;
	priority -= (interface != NULL) * 2;
	priority -= (prio != POLICY_PRIORITY_ROUTED);

	return priority;
}

/**
 * Convert the general ipsec mode to the one defined in xfrm.h
 */
static uint8_t mode2kernel(ipsec_mode_t mode)
{
	switch (mode)
	{
		case MODE_TRANSPORT:
			return XFRM_MODE_TRANSPORT;
		case MODE_TUNNEL:
			return XFRM_MODE_TUNNEL;
		case MODE_BEET:
			return XFRM_MODE_BEET;
		default:
			return mode;
	}
}

/**
 * Convert a host_t to a struct xfrm_address
 */
static void host2xfrm(host_t *host, xfrm_address_t *xfrm)
{
	chunk_t chunk = host->get_address(host);
	memcpy(xfrm, chunk.ptr, min(chunk.len, sizeof(xfrm_address_t)));
}

/**
 * Convert a struct xfrm_address to a host_t
 */
static host_t* xfrm2host(int family, xfrm_address_t *xfrm, uint16_t port)
{
	chunk_t chunk;

	switch (family)
	{
		case AF_INET:
			chunk = chunk_create((u_char*)&xfrm->a4, sizeof(xfrm->a4));
			break;
		case AF_INET6:
			chunk = chunk_create((u_char*)&xfrm->a6, sizeof(xfrm->a6));
			break;
		default:
			return NULL;
	}
	return host_create_from_chunk(family, chunk, ntohs(port));
}

/**
 * Convert a traffic selector address range to subnet and its mask.
 */
static void ts2subnet(traffic_selector_t* ts,
					  xfrm_address_t *net, uint8_t *mask)
{
	host_t *net_host;
	chunk_t net_chunk;

	ts->to_subnet(ts, &net_host, mask);
	net_chunk = net_host->get_address(net_host);
	memcpy(net, net_chunk.ptr, net_chunk.len);
	net_host->destroy(net_host);
}

/**
 * Convert a traffic selector port range to port/portmask
 */
static void ts2ports(traffic_selector_t* ts,
					 uint16_t *port, uint16_t *mask)
{
	uint16_t from, to, bitmask;
	int bit;

	from = ts->get_from_port(ts);
	to = ts->get_to_port(ts);

	/* Quick check for a single port */
	if (from == to)
	{
		*port = htons(from);
		*mask = ~0;
	}
	else
	{
		/* Compute the port mask for port ranges */
		*mask = 0;

		for (bit = 15; bit >= 0; bit--)
		{
			bitmask = 1 << bit;

			if ((bitmask & from) != (bitmask & to))
			{
				*port = htons(from & *mask);
				*mask = htons(*mask);
				return;
			}
			*mask |= bitmask;
		}
	}
	return;
}

/**
 * Convert a pair of traffic_selectors to an xfrm_selector
 */
static struct xfrm_selector ts2selector(traffic_selector_t *src,
										traffic_selector_t *dst,
										char *interface)
{
	struct xfrm_selector sel;
	uint16_t port;

	memset(&sel, 0, sizeof(sel));
	sel.family = (src->get_type(src) == TS_IPV4_ADDR_RANGE) ? AF_INET : AF_INET6;
	/* src or dest proto may be "any" (0), use more restrictive one */
	sel.proto = max(src->get_protocol(src), dst->get_protocol(dst));
	ts2subnet(dst, &sel.daddr, &sel.prefixlen_d);
	ts2subnet(src, &sel.saddr, &sel.prefixlen_s);
	ts2ports(dst, &sel.dport, &sel.dport_mask);
	ts2ports(src, &sel.sport, &sel.sport_mask);
	if ((sel.proto == IPPROTO_ICMP || sel.proto == IPPROTO_ICMPV6) &&
		(sel.dport || sel.sport))
	{
		/* the kernel expects the ICMP type and code in the source and
		 * destination port fields, respectively. */
		port = ntohs(max(sel.dport, sel.sport));
		sel.sport = htons(traffic_selector_icmp_type(port));
		sel.sport_mask = sel.sport ? ~0 : 0;
		sel.dport = htons(traffic_selector_icmp_code(port));
		sel.dport_mask = sel.dport ? ~0 : 0;
	}
	sel.ifindex = interface ? if_nametoindex(interface) : 0;
	sel.user = 0;

	return sel;
}

/**
 * Convert an xfrm_selector to a src|dst traffic_selector
 */
static traffic_selector_t* selector2ts(struct xfrm_selector *sel, bool src)
{
	u_char *addr;
	uint8_t prefixlen;
	uint16_t port = 0;
	host_t *host = NULL;

	if (src)
	{
		addr = (u_char*)&sel->saddr;
		prefixlen = sel->prefixlen_s;
		if (sel->sport_mask)
		{
			port = ntohs(sel->sport);
		}
	}
	else
	{
		addr = (u_char*)&sel->daddr;
		prefixlen = sel->prefixlen_d;
		if (sel->dport_mask)
		{
			port = ntohs(sel->dport);
		}
	}
	if (sel->proto == IPPROTO_ICMP || sel->proto == IPPROTO_ICMPV6)
	{	/* convert ICMP[v6] message type and code as supplied by the kernel in
		 * source and destination ports (both in network order) */
		port = (sel->sport >> 8) | (sel->dport & 0xff00);
		port = ntohs(port);
	}
	/* The Linux 2.6 kernel does not set the selector's family field,
	 * so as a kludge we additionally test the prefix length.
	 */
	if (sel->family == AF_INET || sel->prefixlen_s == 32)
	{
		host = host_create_from_chunk(AF_INET, chunk_create(addr, 4), 0);
	}
	else if (sel->family == AF_INET6 || sel->prefixlen_s == 128)
	{
		host = host_create_from_chunk(AF_INET6, chunk_create(addr, 16), 0);
	}

	if (host)
	{
		return traffic_selector_create_from_subnet(host, prefixlen,
											sel->proto, port, port ?: 65535);
	}
	return NULL;
}

/**
 * Process a XFRM_MSG_ACQUIRE from kernel
 */
static void process_acquire(private_kernel_netlink_ipsec_t *this,
							struct nlmsghdr *hdr)
{
	struct xfrm_user_acquire *acquire;
	struct rtattr *rta;
	size_t rtasize;
	traffic_selector_t *src_ts, *dst_ts;
	uint32_t reqid = 0;
	int proto = 0;

	acquire = NLMSG_DATA(hdr);
	rta = XFRM_RTA(hdr, struct xfrm_user_acquire);
	rtasize = XFRM_PAYLOAD(hdr, struct xfrm_user_acquire);

	DBG2(DBG_KNL, "received a XFRM_MSG_ACQUIRE");

	while (RTA_OK(rta, rtasize))
	{
		DBG2(DBG_KNL, "  %N", xfrm_attr_type_names, rta->rta_type);

		if (rta->rta_type == XFRMA_TMPL)
		{
			struct xfrm_user_tmpl* tmpl;
			tmpl = (struct xfrm_user_tmpl*)RTA_DATA(rta);
			reqid = tmpl->reqid;
			proto = tmpl->id.proto;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
	switch (proto)
	{
		case 0:
		case IPPROTO_ESP:
		case IPPROTO_AH:
			break;
		default:
			/* acquire for AH/ESP only, not for IPCOMP */
			return;
	}
	src_ts = selector2ts(&acquire->sel, TRUE);
	dst_ts = selector2ts(&acquire->sel, FALSE);

	charon->kernel->acquire(charon->kernel, reqid, src_ts, dst_ts);
}

/**
 * Process a XFRM_MSG_EXPIRE from kernel
 */
static void process_expire(private_kernel_netlink_ipsec_t *this,
						   struct nlmsghdr *hdr)
{
	struct xfrm_user_expire *expire;
	uint32_t spi;
	uint8_t protocol;
	host_t *dst;

	expire = NLMSG_DATA(hdr);
	protocol = expire->state.id.proto;
	spi = expire->state.id.spi;

	DBG2(DBG_KNL, "received a XFRM_MSG_EXPIRE");

	if (protocol == IPPROTO_ESP || protocol == IPPROTO_AH)
	{
		dst = xfrm2host(expire->state.family, &expire->state.id.daddr, 0);
		if (dst)
		{
			charon->kernel->expire(charon->kernel, protocol, spi, dst,
								   expire->hard != 0);
			dst->destroy(dst);
		}
	}
}

/**
 * Process a XFRM_MSG_MIGRATE from kernel
 */
static void process_migrate(private_kernel_netlink_ipsec_t *this,
							struct nlmsghdr *hdr)
{
	struct xfrm_userpolicy_id *policy_id;
	struct rtattr *rta;
	size_t rtasize;
	traffic_selector_t *src_ts, *dst_ts;
	host_t *local = NULL, *remote = NULL;
	host_t *old_src = NULL, *old_dst = NULL;
	host_t *new_src = NULL, *new_dst = NULL;
	uint32_t reqid = 0;
	policy_dir_t dir;

	policy_id = NLMSG_DATA(hdr);
	rta     = XFRM_RTA(hdr, struct xfrm_userpolicy_id);
	rtasize = XFRM_PAYLOAD(hdr, struct xfrm_userpolicy_id);

	DBG2(DBG_KNL, "received a XFRM_MSG_MIGRATE");

	src_ts = selector2ts(&policy_id->sel, TRUE);
	dst_ts = selector2ts(&policy_id->sel, FALSE);
	dir = (policy_dir_t)policy_id->dir;

	DBG2(DBG_KNL, "  policy: %R === %R %N", src_ts, dst_ts, policy_dir_names);

	while (RTA_OK(rta, rtasize))
	{
		DBG2(DBG_KNL, "  %N", xfrm_attr_type_names, rta->rta_type);
		if (rta->rta_type == XFRMA_KMADDRESS)
		{
			struct xfrm_user_kmaddress *kmaddress;

			kmaddress = (struct xfrm_user_kmaddress*)RTA_DATA(rta);
			local  = xfrm2host(kmaddress->family, &kmaddress->local, 0);
			remote = xfrm2host(kmaddress->family, &kmaddress->remote, 0);
			DBG2(DBG_KNL, "  kmaddress: %H...%H", local, remote);
		}
		else if (rta->rta_type == XFRMA_MIGRATE)
		{
			struct xfrm_user_migrate *migrate;

			migrate = (struct xfrm_user_migrate*)RTA_DATA(rta);
			old_src = xfrm2host(migrate->old_family, &migrate->old_saddr, 0);
			old_dst = xfrm2host(migrate->old_family, &migrate->old_daddr, 0);
			new_src = xfrm2host(migrate->new_family, &migrate->new_saddr, 0);
			new_dst = xfrm2host(migrate->new_family, &migrate->new_daddr, 0);
			reqid = migrate->reqid;
			DBG2(DBG_KNL, "  migrate %H...%H to %H...%H, reqid {%u}",
						   old_src, old_dst, new_src, new_dst, reqid);
			DESTROY_IF(old_src);
			DESTROY_IF(old_dst);
			DESTROY_IF(new_src);
			DESTROY_IF(new_dst);
		}
		rta = RTA_NEXT(rta, rtasize);
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

/**
 * Process a XFRM_MSG_MAPPING from kernel
 */
static void process_mapping(private_kernel_netlink_ipsec_t *this,
							struct nlmsghdr *hdr)
{
	struct xfrm_user_mapping *mapping;
	uint32_t spi;

	mapping = NLMSG_DATA(hdr);
	spi = mapping->id.spi;

	DBG2(DBG_KNL, "received a XFRM_MSG_MAPPING");

	if (mapping->id.proto == IPPROTO_ESP)
	{
		host_t *dst, *new;

		dst = xfrm2host(mapping->id.family, &mapping->id.daddr, 0);
		if (dst)
		{
			new = xfrm2host(mapping->id.family, &mapping->new_saddr,
							mapping->new_sport);
			if (new)
			{
				charon->kernel->mapping(charon->kernel, IPPROTO_ESP, spi, dst,
										new);
				new->destroy(new);
			}
			dst->destroy(dst);
		}
	}
}

/**
 * Receives events from kernel
 */
static bool receive_events(private_kernel_netlink_ipsec_t *this, int fd,
						   watcher_event_t event)
{
	char response[netlink_get_buflen()];
	struct nlmsghdr *hdr = (struct nlmsghdr*)response;
	struct sockaddr_nl addr;
	socklen_t addr_len = sizeof(addr);
	int len;

	len = recvfrom(this->socket_xfrm_events, response, sizeof(response),
				   MSG_DONTWAIT, (struct sockaddr*)&addr, &addr_len);
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
				DBG1(DBG_KNL, "unable to receive from XFRM event socket: %s "
					 "(%d)", strerror(errno), errno);
				sleep(1);
				return TRUE;
		}
	}

	if (addr.nl_pid != 0)
	{	/* not from kernel. not interested, try another one */
		return TRUE;
	}

	while (NLMSG_OK(hdr, len))
	{
		switch (hdr->nlmsg_type)
		{
			case XFRM_MSG_ACQUIRE:
				process_acquire(this, hdr);
				break;
			case XFRM_MSG_EXPIRE:
				process_expire(this, hdr);
				break;
			case XFRM_MSG_MIGRATE:
				process_migrate(this, hdr);
				break;
			case XFRM_MSG_MAPPING:
				process_mapping(this, hdr);
				break;
			default:
				DBG1(DBG_KNL, "received unknown event from XFRM event "
					 "socket: %d", hdr->nlmsg_type);
				break;
		}
		hdr = NLMSG_NEXT(hdr, len);
	}
	return TRUE;
}

METHOD(kernel_ipsec_t, get_features, kernel_feature_t,
	private_kernel_netlink_ipsec_t *this)
{
	return KERNEL_ESP_V3_TFC | KERNEL_POLICY_SPI;
}

/**
 * Get an SPI for a specific protocol from the kernel.
 */
static status_t get_spi_internal(private_kernel_netlink_ipsec_t *this,
	host_t *src, host_t *dst, uint8_t proto, uint32_t min, uint32_t max,
	uint32_t *spi)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out;
	struct xfrm_userspi_info *userspi;
	uint32_t received_spi = 0;
	size_t len;

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_ALLOCSPI;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_userspi_info));

	userspi = NLMSG_DATA(hdr);
	host2xfrm(src, &userspi->info.saddr);
	host2xfrm(dst, &userspi->info.id.daddr);
	userspi->info.id.proto = proto;
	userspi->info.mode = XFRM_MODE_TUNNEL;
	userspi->info.family = src->get_family(src);
	userspi->min = min;
	userspi->max = max;

	if (this->socket_xfrm->send(this->socket_xfrm, hdr, &out, &len) == SUCCESS)
	{
		hdr = out;
		while (NLMSG_OK(hdr, len))
		{
			switch (hdr->nlmsg_type)
			{
				case XFRM_MSG_NEWSA:
				{
					struct xfrm_usersa_info* usersa = NLMSG_DATA(hdr);
					received_spi = usersa->id.spi;
					break;
				}
				case NLMSG_ERROR:
				{
					struct nlmsgerr *err = NLMSG_DATA(hdr);
					DBG1(DBG_KNL, "allocating SPI failed: %s (%d)",
						 strerror(-err->error), -err->error);
					break;
				}
				default:
					hdr = NLMSG_NEXT(hdr, len);
					continue;
				case NLMSG_DONE:
					break;
			}
			break;
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
	private_kernel_netlink_ipsec_t *this, host_t *src, host_t *dst,
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
	private_kernel_netlink_ipsec_t *this, host_t *src, host_t *dst,
	uint16_t *cpi)
{
	uint32_t received_spi = 0;

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

/**
 * Format the mark for debug messages
 */
static void format_mark(char *buf, int buflen, mark_t mark)
{
	if (mark.value | mark.mask)
	{
		snprintf(buf, buflen, " (mark %u/0x%08x)", mark.value, mark.mask);
	}
}

/**
 * Add a XFRM mark to message if required
 */
static bool add_mark(struct nlmsghdr *hdr, int buflen, mark_t mark)
{
	if (mark.value | mark.mask)
	{
		struct xfrm_mark *xmrk;

		xmrk = netlink_reserve(hdr, buflen, XFRMA_MARK, sizeof(*xmrk));
		if (!xmrk)
		{
			return FALSE;
		}
		xmrk->v = mark.value;
		xmrk->m = mark.mask;
	}
	return TRUE;
}

/**
 * Add a uint32 attribute to message
 */
static bool add_uint32(struct nlmsghdr *hdr, int buflen,
					   enum xfrm_attr_type_t type, uint32_t value)
{
	uint32_t *xvalue;

	xvalue = netlink_reserve(hdr, buflen, type, sizeof(*xvalue));
	if (!xvalue)
	{
		return FALSE;
	}
	*xvalue = value;
	return TRUE;
}

/**
 * Check if kernel supports HW offload
 */
static void netlink_find_offload_feature(const char *ifname, int query_socket)
{
	struct ethtool_sset_info *sset_info;
	struct ethtool_gstrings *cmd = NULL;
	struct ifreq ifr;
	uint32_t sset_len, i;
	char *str;
	int err;

	netlink_hw_offload.state = NL_OFFLOAD_UNSUPPORTED;

	/* determine number of device features */
	INIT_EXTRA(sset_info, sizeof(uint32_t),
		.cmd = ETHTOOL_GSSET_INFO,
		.sset_mask = 1ULL << ETH_SS_FEATURES,
	);
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';
	ifr.ifr_data = (void*)sset_info;

	err = ioctl(query_socket, SIOCETHTOOL, &ifr);
	if (err || sset_info->sset_mask != 1ULL << ETH_SS_FEATURES)
	{
		goto out;
	}
	sset_len = sset_info->data[0];

	/* retrieve names of device features */
	INIT_EXTRA(cmd, ETH_GSTRING_LEN * sset_len,
		.cmd = ETHTOOL_GSTRINGS,
		.string_set = ETH_SS_FEATURES,
	);
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';
	ifr.ifr_data = (void*)cmd;

	err = ioctl(query_socket, SIOCETHTOOL, &ifr);
	if (err)
	{
		goto out;
	}

	/* look for the ESP_HW feature bit */
	str = (char*)cmd->data;
	for (i = 0; i < cmd->len; i++)
	{
		if (strneq(str, "esp-hw-offload", ETH_GSTRING_LEN))
		{
			netlink_hw_offload.bit = i;
			netlink_hw_offload.total_blocks = (sset_len + 31) / 32;
			netlink_hw_offload.state = NL_OFFLOAD_SUPPORTED;
			break;
		}
		str += ETH_GSTRING_LEN;
	}

out:
	free(sset_info);
	free(cmd);
}

/**
 * Check if interface supported HW offload
 */
static bool netlink_detect_offload(const char *ifname)
{
	struct ethtool_gfeatures *cmd;
	uint32_t feature_bit;
	struct ifreq ifr;
	int query_socket;
	int block;
	bool ret = FALSE;

	query_socket = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_XFRM);
	if (query_socket < 0)
	{
		return FALSE;
	}

	/* kernel requires a real interface in order to query the kernel-wide
	 * capability, so we do it here on first invocation.
	 */
	if (netlink_hw_offload.state == NL_OFFLOAD_UNKNOWN)
	{
		netlink_find_offload_feature(ifname, query_socket);
	}
	if (netlink_hw_offload.state == NL_OFFLOAD_UNSUPPORTED)
	{
		DBG1(DBG_KNL, "HW offload is not supported by kernel");
		goto out;
	}

	/* feature is supported by kernel, query device features */
	INIT_EXTRA(cmd, sizeof(cmd->features[0]) * netlink_hw_offload.total_blocks,
		.cmd = ETHTOOL_GFEATURES,
		.size = netlink_hw_offload.total_blocks,
	);
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';
	ifr.ifr_data = (void*)cmd;

	if (ioctl(query_socket, SIOCETHTOOL, &ifr))
	{
		goto out_free;
	}

	block = netlink_hw_offload.bit / 32;
	feature_bit = 1U << (netlink_hw_offload.bit % 32);
	if (cmd->features[block].active & feature_bit)
	{
		ret = TRUE;
	}

out_free:
	free(cmd);
	if (!ret)
	{
		DBG1(DBG_KNL, "HW offload is not supported by device");
	}
out:
	close(query_socket);
	return ret;
}

/**
 * There are 3 HW offload configuration values:
 * 1. HW_OFFLOAD_NO   : Do not configure HW offload.
 * 2. HW_OFFLOAD_YES  : Configure HW offload.
 *                      Fail SA addition if offload is not supported.
 * 3. HW_OFFLOAD_AUTO : Configure HW offload if supported by the kernel
 *                      and device.
 *                      Do not fail SA addition otherwise.
 */
static bool config_hw_offload(kernel_ipsec_sa_id_t *id,
							  kernel_ipsec_add_sa_t *data, struct nlmsghdr *hdr,
							  int buflen)
{
	host_t *local = data->inbound ? id->dst : id->src;
	struct xfrm_user_offload *offload;
	bool hw_offload_yes, ret = FALSE;
	char *ifname;

	/* do Ipsec configuration without offload */
	if (data->hw_offload == HW_OFFLOAD_NO)
	{
		return TRUE;
	}

	hw_offload_yes = (data->hw_offload == HW_OFFLOAD_YES);

	if (!charon->kernel->get_interface(charon->kernel, local, &ifname))
	{
		return !hw_offload_yes;
	}

	/* check if interface supports hw_offload */
	if (!netlink_detect_offload(ifname))
	{
		ret = !hw_offload_yes;
		goto out;
	}

	/* activate HW offload */
	offload = netlink_reserve(hdr, buflen,
							  XFRMA_OFFLOAD_DEV, sizeof(*offload));
	if (!offload)
	{
		ret = !hw_offload_yes;
		goto out;
	}
	offload->ifindex = if_nametoindex(ifname);
	if (local->get_family(local) == AF_INET6)
	{
		offload->flags |= XFRM_OFFLOAD_IPV6;
	}
	offload->flags |= data->inbound ? XFRM_OFFLOAD_INBOUND : 0;

	ret = TRUE;

out:
	free(ifname);
	return ret;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_netlink_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	netlink_buf_t request;
	const char *alg_name;
	char markstr[32] = "";
	struct nlmsghdr *hdr;
	struct xfrm_usersa_info *sa;
	uint16_t icv_size = 64, ipcomp = data->ipcomp;
	ipsec_mode_t mode = data->mode, original_mode = data->mode;
	traffic_selector_t *first_src_ts, *first_dst_ts;
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

	memset(&request, 0, sizeof(request));
	format_mark(markstr, sizeof(markstr), id->mark);

	DBG2(DBG_KNL, "adding SAD entry with SPI %.8x and reqid {%u}%s",
		 ntohl(id->spi), data->reqid, markstr);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = data->update ? XFRM_MSG_UPDSA : XFRM_MSG_NEWSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_info));

	sa = NLMSG_DATA(hdr);
	host2xfrm(id->src, &sa->saddr);
	host2xfrm(id->dst, &sa->id.daddr);
	sa->id.spi = id->spi;
	sa->id.proto = id->proto;
	sa->family = id->src->get_family(id->src);
	sa->mode = mode2kernel(mode);

	if (!data->copy_df)
	{
		sa->flags |= XFRM_STATE_NOPMTUDISC;
	}

	if (!data->copy_ecn)
	{
		sa->flags |= XFRM_STATE_NOECN;
	}

	if (data->inbound)
	{
		switch (data->copy_dscp)
		{
			case DSCP_COPY_YES:
			case DSCP_COPY_IN_ONLY:
				sa->flags |= XFRM_STATE_DECAP_DSCP;
				break;
			default:
				break;
		}
	}
	else
	{
		switch (data->copy_dscp)
		{
			case DSCP_COPY_IN_ONLY:
			case DSCP_COPY_NO:
			{
				/* currently the only extra flag */
				if (!add_uint32(hdr, sizeof(request), XFRMA_SA_EXTRA_FLAGS,
								XFRM_SA_XFLAG_DONT_ENCAP_DSCP))
				{
					goto failed;
				}
				break;
			}
			default:
				break;
		}
	}

	switch (mode)
	{
		case MODE_TUNNEL:
			sa->flags |= XFRM_STATE_AF_UNSPEC;
			break;
		case MODE_BEET:
		case MODE_TRANSPORT:
			if (original_mode == MODE_TUNNEL)
			{	/* don't install selectors for switched SAs.  because only one
				 * selector can be installed other traffic would get dropped */
				break;
			}
			if (data->src_ts->get_first(data->src_ts,
										(void**)&first_src_ts) == SUCCESS &&
				data->dst_ts->get_first(data->dst_ts,
										(void**)&first_dst_ts) == SUCCESS)
			{
				sa->sel = ts2selector(first_src_ts, first_dst_ts,
									  data->interface);
				if (!this->proto_port_transport)
				{
					/* don't install proto/port on SA. This would break
					 * potential secondary SAs for the same address using a
					 * different prot/port. */
					sa->sel.proto = 0;
					sa->sel.dport = sa->sel.dport_mask = 0;
					sa->sel.sport = sa->sel.sport_mask = 0;
				}
			}
			break;
		default:
			break;
	}
	if (id->proto == IPPROTO_AH && sa->family == AF_INET)
	{	/* use alignment to 4 bytes for IPv4 instead of the incorrect 8 byte
		 * alignment that's used by default but is only valid for IPv6 */
		sa->flags |= XFRM_STATE_ALIGN4;
	}

	sa->reqid = data->reqid;
	sa->lft.soft_byte_limit = XFRM_LIMIT(data->lifetime->bytes.rekey);
	sa->lft.hard_byte_limit = XFRM_LIMIT(data->lifetime->bytes.life);
	sa->lft.soft_packet_limit = XFRM_LIMIT(data->lifetime->packets.rekey);
	sa->lft.hard_packet_limit = XFRM_LIMIT(data->lifetime->packets.life);
	/* we use lifetimes since added, not since used */
	sa->lft.soft_add_expires_seconds = data->lifetime->time.rekey;
	sa->lft.hard_add_expires_seconds = data->lifetime->time.life;
	sa->lft.soft_use_expires_seconds = 0;
	sa->lft.hard_use_expires_seconds = 0;

	switch (data->enc_alg)
	{
		case ENCR_UNDEFINED:
			/* no encryption */
			break;
		case ENCR_AES_CCM_ICV16:
		case ENCR_AES_GCM_ICV16:
		case ENCR_NULL_AUTH_AES_GMAC:
		case ENCR_CAMELLIA_CCM_ICV16:
		case ENCR_CHACHA20_POLY1305:
			icv_size += 32;
			/* FALL */
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_GCM_ICV12:
		case ENCR_CAMELLIA_CCM_ICV12:
			icv_size += 32;
			/* FALL */
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_GCM_ICV8:
		case ENCR_CAMELLIA_CCM_ICV8:
		{
			struct xfrm_algo_aead *algo;

			alg_name = lookup_algorithm(ENCRYPTION_ALGORITHM, data->enc_alg);
			if (alg_name == NULL)
			{
				DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
						 encryption_algorithm_names, data->enc_alg);
					goto failed;
			}
			DBG2(DBG_KNL, "  using encryption algorithm %N with key size %d",
				 encryption_algorithm_names, data->enc_alg,
				 data->enc_key.len * 8);

			algo = netlink_reserve(hdr, sizeof(request), XFRMA_ALG_AEAD,
								   sizeof(*algo) + data->enc_key.len);
			if (!algo)
			{
				goto failed;
			}
			algo->alg_key_len = data->enc_key.len * 8;
			algo->alg_icv_len = icv_size;
			strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
			algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
			memcpy(algo->alg_key, data->enc_key.ptr, data->enc_key.len);
			break;
		}
		default:
		{
			struct xfrm_algo *algo;

			alg_name = lookup_algorithm(ENCRYPTION_ALGORITHM, data->enc_alg);
			if (alg_name == NULL)
			{
				DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
					 encryption_algorithm_names, data->enc_alg);
				goto failed;
			}
			DBG2(DBG_KNL, "  using encryption algorithm %N with key size %d",
				 encryption_algorithm_names, data->enc_alg,
				 data->enc_key.len * 8);

			algo = netlink_reserve(hdr, sizeof(request), XFRMA_ALG_CRYPT,
								   sizeof(*algo) + data->enc_key.len);
			if (!algo)
			{
				goto failed;
			}
			algo->alg_key_len = data->enc_key.len * 8;
			strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
			algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
			memcpy(algo->alg_key, data->enc_key.ptr, data->enc_key.len);
		}
	}

	if (data->int_alg != AUTH_UNDEFINED)
	{
		u_int trunc_len = 0;

		alg_name = lookup_algorithm(INTEGRITY_ALGORITHM, data->int_alg);
		if (alg_name == NULL)
		{
			DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
				 integrity_algorithm_names, data->int_alg);
			goto failed;
		}
		DBG2(DBG_KNL, "  using integrity algorithm %N with key size %d",
			 integrity_algorithm_names, data->int_alg, data->int_key.len * 8);

		switch (data->int_alg)
		{
			case AUTH_HMAC_MD5_128:
			case AUTH_HMAC_SHA2_256_128:
				trunc_len = 128;
				break;
			case AUTH_HMAC_SHA1_160:
				trunc_len = 160;
				break;
			default:
				break;
		}

		if (trunc_len)
		{
			struct xfrm_algo_auth* algo;

			/* the kernel uses SHA256 with 96 bit truncation by default,
			 * use specified truncation size supported by newer kernels.
			 * also use this for untruncated MD5 and SHA1. */
			algo = netlink_reserve(hdr, sizeof(request), XFRMA_ALG_AUTH_TRUNC,
								   sizeof(*algo) + data->int_key.len);
			if (!algo)
			{
				goto failed;
			}
			algo->alg_key_len = data->int_key.len * 8;
			algo->alg_trunc_len = trunc_len;
			strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
			algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
			memcpy(algo->alg_key, data->int_key.ptr, data->int_key.len);
		}
		else
		{
			struct xfrm_algo* algo;

			algo = netlink_reserve(hdr, sizeof(request), XFRMA_ALG_AUTH,
								   sizeof(*algo) + data->int_key.len);
			if (!algo)
			{
				goto failed;
			}
			algo->alg_key_len = data->int_key.len * 8;
			strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
			algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
			memcpy(algo->alg_key, data->int_key.ptr, data->int_key.len);
		}
	}

	if (ipcomp != IPCOMP_NONE)
	{
		struct xfrm_algo* algo;

		alg_name = lookup_algorithm(COMPRESSION_ALGORITHM, ipcomp);
		if (alg_name == NULL)
		{
			DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
				 ipcomp_transform_names, ipcomp);
			goto failed;
		}
		DBG2(DBG_KNL, "  using compression algorithm %N",
			 ipcomp_transform_names, ipcomp);

		algo = netlink_reserve(hdr, sizeof(request), XFRMA_ALG_COMP,
							   sizeof(*algo));
		if (!algo)
		{
			goto failed;
		}
		algo->alg_key_len = 0;
		strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
		algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
	}

	if (data->encap)
	{
		struct xfrm_encap_tmpl *tmpl;

		tmpl = netlink_reserve(hdr, sizeof(request), XFRMA_ENCAP, sizeof(*tmpl));
		if (!tmpl)
		{
			goto failed;
		}
		tmpl->encap_type = UDP_ENCAP_ESPINUDP;
		tmpl->encap_sport = htons(id->src->get_port(id->src));
		tmpl->encap_dport = htons(id->dst->get_port(id->dst));
		memset(&tmpl->encap_oa, 0, sizeof (xfrm_address_t));
		/* encap_oa could probably be derived from the
		 * traffic selectors [rfc4306, p39]. In the netlink kernel
		 * implementation pluto does the same as we do here but it uses
		 * encap_oa in the pfkey implementation.
		 * BUT as /usr/src/linux/net/key/af_key.c indicates the kernel ignores
		 * it anyway
		 *   -> does that mean that NAT-T encap doesn't work in transport mode?
		 * No. The reason the kernel ignores NAT-OA is that it recomputes
		 * (or, rather, just ignores) the checksum. If packets pass the IPsec
		 * checks it marks them "checksum ok" so OA isn't needed. */
	}

	if (!add_mark(hdr, sizeof(request), id->mark))
	{
		goto failed;
	}

	if (ipcomp == IPCOMP_NONE && (data->mark.value | data->mark.mask))
	{
		if (!add_uint32(hdr, sizeof(request), XFRMA_SET_MARK,
						data->mark.value) ||
			!add_uint32(hdr, sizeof(request), XFRMA_SET_MARK_MASK,
						data->mark.mask))
		{
			goto failed;
		}
	}

	if (data->tfc && id->proto == IPPROTO_ESP && mode == MODE_TUNNEL)
	{	/* the kernel supports TFC padding only for tunnel mode ESP SAs */
		if (!add_uint32(hdr, sizeof(request), XFRMA_TFCPAD, data->tfc))
		{
			goto failed;
		}
	}

	if (id->proto != IPPROTO_COMP)
	{
		/* generally, we don't need a replay window for outbound SAs, however,
		 * when using ESN the kernel rejects the attribute if it is 0 */
		if (!data->inbound && data->replay_window)
		{
			data->replay_window = data->esn ? 1 : 0;
		}
		if (data->replay_window != 0 && (data->esn || data->replay_window > 32))
		{
			/* for ESN or larger replay windows we need the new
			 * XFRMA_REPLAY_ESN_VAL attribute to configure a bitmap */
			struct xfrm_replay_state_esn *replay;
			uint32_t bmp_size;

			bmp_size = round_up(data->replay_window, sizeof(uint32_t) * 8) / 8;
			replay = netlink_reserve(hdr, sizeof(request), XFRMA_REPLAY_ESN_VAL,
									 sizeof(*replay) + bmp_size);
			if (!replay)
			{
				goto failed;
			}
			/* bmp_len contains number uf __u32's */
			replay->bmp_len = bmp_size / sizeof(uint32_t);
			replay->replay_window = data->replay_window;
			DBG2(DBG_KNL, "  using replay window of %u packets",
				 data->replay_window);

			if (data->esn)
			{
				DBG2(DBG_KNL, "  using extended sequence numbers (ESN)");
				sa->flags |= XFRM_STATE_ESN;
			}
		}
		else
		{
			DBG2(DBG_KNL, "  using replay window of %u packets",
				 data->replay_window);
			sa->replay_window = data->replay_window;
		}

		DBG2(DBG_KNL, "  HW offload: %N", hw_offload_names, data->hw_offload);
		if (!config_hw_offload(id, data, hdr, sizeof(request)))
		{
			DBG1(DBG_KNL, "failed to configure HW offload");
			goto failed;
		}
	}

	status = this->socket_xfrm->send_ack(this->socket_xfrm, hdr);
	if (status == NOT_FOUND && data->update)
	{
		DBG1(DBG_KNL, "allocated SPI not found anymore, try to add SAD entry");
		hdr->nlmsg_type = XFRM_MSG_NEWSA;
		status = this->socket_xfrm->send_ack(this->socket_xfrm, hdr);
	}

	if (status != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to add SAD entry with SPI %.8x%s (%N)", ntohl(id->spi),
			 markstr, status_names, status);
		status = FAILED;
		goto failed;
	}

	status = SUCCESS;

failed:
	memwipe(&request, sizeof(request));
	return status;
}

/**
 * Get the ESN replay state (i.e. sequence numbers) of an SA.
 *
 * Allocates into one the replay state structure we get from the kernel.
 */
static void get_replay_state(private_kernel_netlink_ipsec_t *this,
							 kernel_ipsec_sa_id_t *sa,
							 struct xfrm_replay_state_esn **replay_esn,
							 uint32_t *replay_esn_len,
							 struct xfrm_replay_state **replay,
							 struct xfrm_lifetime_cur **lifetime)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out = NULL;
	struct xfrm_aevent_id *out_aevent = NULL, *aevent_id;
	size_t len;
	struct rtattr *rta;
	size_t rtasize;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "querying replay state from SAD entry with SPI %.8x",
		 ntohl(sa->spi));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETAE;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_aevent_id));

	aevent_id = NLMSG_DATA(hdr);
	aevent_id->flags = XFRM_AE_RVAL;

	host2xfrm(sa->dst, &aevent_id->sa_id.daddr);
	aevent_id->sa_id.spi = sa->spi;
	aevent_id->sa_id.proto = sa->proto;
	aevent_id->sa_id.family = sa->dst->get_family(sa->dst);

	if (!add_mark(hdr, sizeof(request), sa->mark))
	{
		return;
	}

	if (this->socket_xfrm->send(this->socket_xfrm, hdr, &out, &len) == SUCCESS)
	{
		hdr = out;
		while (NLMSG_OK(hdr, len))
		{
			switch (hdr->nlmsg_type)
			{
				case XFRM_MSG_NEWAE:
				{
					out_aevent = NLMSG_DATA(hdr);
					break;
				}
				case NLMSG_ERROR:
				{
					struct nlmsgerr *err = NLMSG_DATA(hdr);
					DBG1(DBG_KNL, "querying replay state from SAD entry "
						 "failed: %s (%d)", strerror(-err->error), -err->error);
					break;
				}
				default:
					hdr = NLMSG_NEXT(hdr, len);
					continue;
				case NLMSG_DONE:
					break;
			}
			break;
		}
	}

	if (out_aevent)
	{
		rta = XFRM_RTA(out, struct xfrm_aevent_id);
		rtasize = XFRM_PAYLOAD(out, struct xfrm_aevent_id);
		while (RTA_OK(rta, rtasize))
		{
			if (rta->rta_type == XFRMA_LTIME_VAL &&
				RTA_PAYLOAD(rta) == sizeof(**lifetime))
			{
				free(*lifetime);
				*lifetime = malloc(RTA_PAYLOAD(rta));
				memcpy(*lifetime, RTA_DATA(rta), RTA_PAYLOAD(rta));
			}
			if (rta->rta_type == XFRMA_REPLAY_VAL &&
				RTA_PAYLOAD(rta) == sizeof(**replay))
			{
				free(*replay);
				*replay = malloc(RTA_PAYLOAD(rta));
				memcpy(*replay, RTA_DATA(rta), RTA_PAYLOAD(rta));
			}
			if (rta->rta_type == XFRMA_REPLAY_ESN_VAL &&
				RTA_PAYLOAD(rta) >= sizeof(**replay_esn))
			{
				free(*replay_esn);
				*replay_esn = malloc(RTA_PAYLOAD(rta));
				*replay_esn_len = RTA_PAYLOAD(rta);
				memcpy(*replay_esn, RTA_DATA(rta), RTA_PAYLOAD(rta));
			}
			rta = RTA_NEXT(rta, rtasize);
		}
	}
	free(out);
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_netlink_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	netlink_buf_t request;
	struct nlmsghdr *out = NULL, *hdr;
	struct xfrm_usersa_id *sa_id;
	struct xfrm_usersa_info *sa = NULL;
	status_t status = FAILED;
	size_t len;
	char markstr[32] = "";

	memset(&request, 0, sizeof(request));
	format_mark(markstr, sizeof(markstr), id->mark);

	DBG2(DBG_KNL, "querying SAD entry with SPI %.8x%s", ntohl(id->spi),
		 markstr);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_id));

	sa_id = NLMSG_DATA(hdr);
	host2xfrm(id->dst, &sa_id->daddr);
	sa_id->spi = id->spi;
	sa_id->proto = id->proto;
	sa_id->family = id->dst->get_family(id->dst);

	if (!add_mark(hdr, sizeof(request), id->mark))
	{
		return FAILED;
	}

	if (this->socket_xfrm->send(this->socket_xfrm, hdr, &out, &len) == SUCCESS)
	{
		hdr = out;
		while (NLMSG_OK(hdr, len))
		{
			switch (hdr->nlmsg_type)
			{
				case XFRM_MSG_NEWSA:
				{
					sa = NLMSG_DATA(hdr);
					break;
				}
				case NLMSG_ERROR:
				{
					struct nlmsgerr *err = NLMSG_DATA(hdr);

					DBG1(DBG_KNL, "querying SAD entry with SPI %.8x%s failed: "
						 "%s (%d)", ntohl(id->spi), markstr,
						 strerror(-err->error), -err->error);
					break;
				}
				default:
					hdr = NLMSG_NEXT(hdr, len);
					continue;
				case NLMSG_DONE:
					break;
			}
			break;
		}
	}

	if (sa == NULL)
	{
		DBG2(DBG_KNL, "unable to query SAD entry with SPI %.8x%s",
			 ntohl(id->spi), markstr);
	}
	else
	{
		if (bytes)
		{
			*bytes = sa->curlft.bytes;
		}
		if (packets)
		{
			*packets = sa->curlft.packets;
		}
		if (time)
		{	/* curlft contains an "use" time, but that contains a timestamp
			 * of the first use, not the last. Last use time must be queried
			 * on the policy on Linux */
			*time = 0;
		}
		status = SUCCESS;
	}
	memwipe(out, len);
	free(out);
	return status;
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_kernel_netlink_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct xfrm_usersa_id *sa_id;
	char markstr[32] = "";

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
		kernel_ipsec_del_sa_t ipcomp = {};
		del_sa(this, &ipcomp_id, &ipcomp);
	}

	memset(&request, 0, sizeof(request));
	format_mark(markstr, sizeof(markstr), id->mark);

	DBG2(DBG_KNL, "deleting SAD entry with SPI %.8x%s", ntohl(id->spi),
		 markstr);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_DELSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_id));

	sa_id = NLMSG_DATA(hdr);
	host2xfrm(id->dst, &sa_id->daddr);
	sa_id->spi = id->spi;
	sa_id->proto = id->proto;
	sa_id->family = id->dst->get_family(id->dst);

	if (!add_mark(hdr, sizeof(request), id->mark))
	{
		return FAILED;
	}

	switch (this->socket_xfrm->send_ack(this->socket_xfrm, hdr))
	{
		case SUCCESS:
			DBG2(DBG_KNL, "deleted SAD entry with SPI %.8x%s",
				 ntohl(id->spi), markstr);
			return SUCCESS;
		case NOT_FOUND:
			return NOT_FOUND;
		default:
			DBG1(DBG_KNL, "unable to delete SAD entry with SPI %.8x%s",
				 ntohl(id->spi), markstr);
			return FAILED;
	}
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_netlink_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out_hdr = NULL, *out = NULL;
	struct xfrm_usersa_id *sa_id;
	struct xfrm_usersa_info *sa;
	size_t len;
	struct rtattr *rta;
	size_t rtasize;
	struct xfrm_encap_tmpl* encap = NULL;
	struct xfrm_replay_state *replay = NULL;
	struct xfrm_replay_state_esn *replay_esn = NULL;
	struct xfrm_lifetime_cur *lifetime = NULL;
	uint32_t replay_esn_len = 0;
	kernel_ipsec_del_sa_t del = { 0 };
	status_t status = FAILED;
	traffic_selector_t *ts;
	char markstr[32] = "";

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
	format_mark(markstr, sizeof(markstr), id->mark);

	DBG2(DBG_KNL, "querying SAD entry with SPI %.8x%s for update",
		 ntohl(id->spi), markstr);

	/* query the existing SA first */
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_id));

	sa_id = NLMSG_DATA(hdr);
	host2xfrm(id->dst, &sa_id->daddr);
	sa_id->spi = id->spi;
	sa_id->proto = id->proto;
	sa_id->family = id->dst->get_family(id->dst);

	if (!add_mark(hdr, sizeof(request), id->mark))
	{
		return FAILED;
	}

	if (this->socket_xfrm->send(this->socket_xfrm, hdr, &out, &len) == SUCCESS)
	{
		hdr = out;
		while (NLMSG_OK(hdr, len))
		{
			switch (hdr->nlmsg_type)
			{
				case XFRM_MSG_NEWSA:
				{
					out_hdr = hdr;
					break;
				}
				case NLMSG_ERROR:
				{
					struct nlmsgerr *err = NLMSG_DATA(hdr);
					DBG1(DBG_KNL, "querying SAD entry failed: %s (%d)",
						 strerror(-err->error), -err->error);
					break;
				}
				default:
					hdr = NLMSG_NEXT(hdr, len);
					continue;
				case NLMSG_DONE:
					break;
			}
			break;
		}
	}
	if (!out_hdr)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x%s",
			 ntohl(id->spi), markstr);
		goto failed;
	}

	get_replay_state(this, id, &replay_esn, &replay_esn_len, &replay,
					 &lifetime);

	/* delete the old SA (without affecting the IPComp SA) */
	if (del_sa(this, id, &del) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to delete old SAD entry with SPI %.8x%s",
			 ntohl(id->spi), markstr);
		goto failed;
	}

	DBG2(DBG_KNL, "updating SAD entry with SPI %.8x%s from %#H..%#H to "
		 "%#H..%#H", ntohl(id->spi), markstr, id->src, id->dst, data->new_src,
		 data->new_dst);
	/* copy over the SA from out to request */
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_NEWSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_info));
	sa = NLMSG_DATA(hdr);
	memcpy(sa, NLMSG_DATA(out_hdr), sizeof(struct xfrm_usersa_info));
	sa->family = data->new_dst->get_family(data->new_dst);

	if (!id->src->ip_equals(id->src, data->new_src))
	{
		host2xfrm(data->new_src, &sa->saddr);

		ts = selector2ts(&sa->sel, TRUE);
		if (ts && ts->is_host(ts, id->src))
		{
			ts->set_address(ts, data->new_src);
			ts2subnet(ts, &sa->sel.saddr, &sa->sel.prefixlen_s);
		}
		DESTROY_IF(ts);
	}
	if (!id->dst->ip_equals(id->dst, data->new_dst))
	{
		host2xfrm(data->new_dst, &sa->id.daddr);

		ts = selector2ts(&sa->sel, FALSE);
		if (ts && ts->is_host(ts, id->dst))
		{
			ts->set_address(ts, data->new_dst);
			ts2subnet(ts, &sa->sel.daddr, &sa->sel.prefixlen_d);
		}
		DESTROY_IF(ts);
	}

	rta = XFRM_RTA(out_hdr, struct xfrm_usersa_info);
	rtasize = XFRM_PAYLOAD(out_hdr, struct xfrm_usersa_info);
	while (RTA_OK(rta, rtasize))
	{
		/* copy all attributes, but not XFRMA_ENCAP if we are disabling it */
		if (rta->rta_type != XFRMA_ENCAP || data->new_encap)
		{
			if (rta->rta_type == XFRMA_ENCAP)
			{	/* update encap tmpl */
				encap = RTA_DATA(rta);
				encap->encap_sport = ntohs(data->new_src->get_port(data->new_src));
				encap->encap_dport = ntohs(data->new_dst->get_port(data->new_dst));
			}
			if (rta->rta_type == XFRMA_OFFLOAD_DEV)
			{	/* update offload device */
				struct xfrm_user_offload *offload;
				host_t *local;
				char *ifname;

				offload = RTA_DATA(rta);
				local = offload->flags & XFRM_OFFLOAD_INBOUND ? data->new_dst
															  : data->new_src;

				if (charon->kernel->get_interface(charon->kernel, local,
												  &ifname))
				{
					offload->ifindex = if_nametoindex(ifname);
					if (local->get_family(local) == AF_INET6)
					{
						offload->flags |= XFRM_OFFLOAD_IPV6;
					}
					else
					{
						offload->flags &= ~XFRM_OFFLOAD_IPV6;
					}
					free(ifname);
				}
			}
			netlink_add_attribute(hdr, rta->rta_type,
								  chunk_create(RTA_DATA(rta), RTA_PAYLOAD(rta)),
								  sizeof(request));
		}
		rta = RTA_NEXT(rta, rtasize);
	}

	if (encap == NULL && data->new_encap)
	{	/* add tmpl if we are enabling it */
		encap = netlink_reserve(hdr, sizeof(request), XFRMA_ENCAP,
								sizeof(*encap));
		if (!encap)
		{
			goto failed;
		}
		encap->encap_type = UDP_ENCAP_ESPINUDP;
		encap->encap_sport = ntohs(data->new_src->get_port(data->new_src));
		encap->encap_dport = ntohs(data->new_dst->get_port(data->new_dst));
		memset(&encap->encap_oa, 0, sizeof (xfrm_address_t));
	}

	if (replay_esn)
	{
		struct xfrm_replay_state_esn *state;

		state = netlink_reserve(hdr, sizeof(request), XFRMA_REPLAY_ESN_VAL,
								replay_esn_len);
		if (!state)
		{
			goto failed;
		}
		memcpy(state, replay_esn, replay_esn_len);
	}
	else if (replay)
	{
		struct xfrm_replay_state *state;

		state = netlink_reserve(hdr, sizeof(request), XFRMA_REPLAY_VAL,
								sizeof(*state));
		if (!state)
		{
			goto failed;
		}
		memcpy(state, replay, sizeof(*state));
	}
	else
	{
		DBG1(DBG_KNL, "unable to copy replay state from old SAD entry with "
			 "SPI %.8x%s", ntohl(id->spi), markstr);
	}
	if (lifetime)
	{
		struct xfrm_lifetime_cur *state;

		state = netlink_reserve(hdr, sizeof(request), XFRMA_LTIME_VAL,
								sizeof(*state));
		if (!state)
		{
			goto failed;
		}
		memcpy(state, lifetime, sizeof(*state));
	}
	else
	{
		DBG1(DBG_KNL, "unable to copy usage stats from old SAD entry with "
			 "SPI %.8x%s", ntohl(id->spi), markstr);
	}

	if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x%s",
			 ntohl(id->spi), markstr);
		goto failed;
	}

	status = SUCCESS;
failed:
	free(replay);
	free(replay_esn);
	free(lifetime);
	memwipe(out, len);
	memwipe(&request, sizeof(request));
	free(out);

	return status;
}

METHOD(kernel_ipsec_t, flush_sas, status_t,
	private_kernel_netlink_ipsec_t *this)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct xfrm_usersa_flush *flush;
	struct {
		uint8_t proto;
		char *name;
	} protos[] = {
		{ IPPROTO_AH, "AH" },
		{ IPPROTO_ESP, "ESP" },
		{ IPPROTO_COMP, "IPComp" },
	};
	int i;

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_FLUSHSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_flush));

	flush = NLMSG_DATA(hdr);

	for (i = 0; i < countof(protos); i++)
	{
		DBG2(DBG_KNL, "flushing all %s SAD entries", protos[i].name);

		flush->proto = protos[i].proto;

		if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to flush %s SAD entries", protos[i].name);
			return FAILED;
		}
	}
	return SUCCESS;
}

/**
 * Unlock the mutex and signal waiting threads
 */
static void policy_change_done(private_kernel_netlink_ipsec_t *this,
							   policy_entry_t *policy)
{
	policy->working = FALSE;
	if (policy->waiting)
	{	/* don't need to wake threads waiting for other policies */
		this->condvar->broadcast(this->condvar);
	}
	this->mutex->unlock(this->mutex);
}

/**
 * Install a route for the given policy if enabled and required
 */
static void install_route(private_kernel_netlink_ipsec_t *this,
	policy_entry_t *policy, policy_sa_t *mapping, ipsec_sa_t *ipsec)
{
	policy_sa_out_t *out = (policy_sa_out_t*)mapping;
	route_entry_t *route;
	host_t *iface;

	INIT(route,
		.prefixlen = policy->sel.prefixlen_d,
	);

	if (charon->kernel->get_address_by_ts(charon->kernel, out->src_ts,
										  &route->src_ip, NULL) == SUCCESS)
	{
		if (!ipsec->dst->is_anyaddr(ipsec->dst))
		{
			route->gateway = charon->kernel->get_nexthop(charon->kernel,
												ipsec->dst, -1, ipsec->src,
												&route->if_name);
		}
		else
		{	/* for shunt policies */
			iface = xfrm2host(policy->sel.family, &policy->sel.daddr, 0);
			route->gateway = charon->kernel->get_nexthop(charon->kernel,
												iface, policy->sel.prefixlen_d,
												route->src_ip, &route->if_name);
			iface->destroy(iface);
		}
		route->dst_net = chunk_alloc(policy->sel.family == AF_INET ? 4 : 16);
		memcpy(route->dst_net.ptr, &policy->sel.daddr, route->dst_net.len);

		/* get the interface to install the route for, if we haven't one yet.
		 * If we have a local address, use it. Otherwise (for shunt policies)
		 * use the route's source address. */
		if (!route->if_name)
		{
			iface = ipsec->src;
			if (iface->is_anyaddr(iface))
			{
				iface = route->src_ip;
			}
			if (!charon->kernel->get_interface(charon->kernel, iface,
											   &route->if_name))
			{
				route_entry_destroy(route);
				return;
			}
		}
		if (policy->route)
		{
			route_entry_t *old = policy->route;
			if (route_entry_equals(old, route))
			{
				route_entry_destroy(route);
				return;
			}
			/* uninstall previously installed route */
			if (charon->kernel->del_route(charon->kernel, old->dst_net,
										  old->prefixlen, old->gateway,
										  old->src_ip, old->if_name) != SUCCESS)
			{
				DBG1(DBG_KNL, "error uninstalling route installed with policy "
					 "%R === %R %N", out->src_ts, out->dst_ts, policy_dir_names,
					 policy->direction);
			}
			route_entry_destroy(old);
			policy->route = NULL;
		}

		DBG2(DBG_KNL, "installing route: %R via %H src %H dev %s", out->dst_ts,
			 route->gateway, route->src_ip, route->if_name);
		switch (charon->kernel->add_route(charon->kernel, route->dst_net,
										  route->prefixlen, route->gateway,
										  route->src_ip, route->if_name))
		{
			default:
				DBG1(DBG_KNL, "unable to install source route for %H",
					 route->src_ip);
				/* FALL */
			case ALREADY_DONE:
				/* route exists, do not uninstall */
				route_entry_destroy(route);
				break;
			case SUCCESS:
				/* cache the installed route */
				policy->route = route;
				break;
		}
	}
	else
	{
		free(route);
	}
}

/**
 * Add or update a policy in the kernel.
 *
 * Note: The mutex has to be locked when entering this function
 * and is unlocked here in any case.
 */
static status_t add_policy_internal(private_kernel_netlink_ipsec_t *this,
	policy_entry_t *policy, policy_sa_t *mapping, bool update)
{
	netlink_buf_t request;
	policy_entry_t clone;
	ipsec_sa_t *ipsec = mapping->sa;
	struct xfrm_userpolicy_info *policy_info;
	struct nlmsghdr *hdr;
	status_t status;
	int i;

	/* clone the policy so we are able to check it out again later */
	memcpy(&clone, policy, sizeof(policy_entry_t));

	memset(&request, 0, sizeof(request));
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = update ? XFRM_MSG_UPDPOLICY : XFRM_MSG_NEWPOLICY;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_userpolicy_info));

	policy_info = NLMSG_DATA(hdr);
	policy_info->sel = policy->sel;
	policy_info->dir = policy->direction;

	/* calculate priority based on selector size, small size = high prio */
	policy_info->priority = mapping->priority;
	policy_info->action = mapping->type != POLICY_DROP ? XFRM_POLICY_ALLOW
													   : XFRM_POLICY_BLOCK;
	policy_info->share = XFRM_SHARE_ANY;

	/* policies don't expire */
	policy_info->lft.soft_byte_limit = XFRM_INF;
	policy_info->lft.soft_packet_limit = XFRM_INF;
	policy_info->lft.hard_byte_limit = XFRM_INF;
	policy_info->lft.hard_packet_limit = XFRM_INF;
	policy_info->lft.soft_add_expires_seconds = 0;
	policy_info->lft.hard_add_expires_seconds = 0;
	policy_info->lft.soft_use_expires_seconds = 0;
	policy_info->lft.hard_use_expires_seconds = 0;

	if (mapping->type == POLICY_IPSEC && ipsec->cfg.reqid)
	{
		struct xfrm_user_tmpl *tmpl;
		struct {
			uint8_t proto;
			uint32_t spi;
			bool use;
		} protos[] = {
			{ IPPROTO_COMP, htonl(ntohs(ipsec->cfg.ipcomp.cpi)),
			  ipsec->cfg.ipcomp.transform != IPCOMP_NONE },
			{ IPPROTO_ESP, ipsec->cfg.esp.spi, ipsec->cfg.esp.use },
			{ IPPROTO_AH, ipsec->cfg.ah.spi, ipsec->cfg.ah.use },
		};
		ipsec_mode_t proto_mode = ipsec->cfg.mode;
		int count = 0;

		for (i = 0; i < countof(protos); i++)
		{
			if (protos[i].use)
			{
				count++;
			}
		}
		tmpl = netlink_reserve(hdr, sizeof(request), XFRMA_TMPL,
							   count * sizeof(*tmpl));
		if (!tmpl)
		{
			policy_change_done(this, policy);
			return FAILED;
		}

		for (i = 0; i < countof(protos); i++)
		{
			if (!protos[i].use)
			{
				continue;
			}
			tmpl->reqid = ipsec->cfg.reqid;
			tmpl->id.proto = protos[i].proto;
			if (policy->direction == POLICY_OUT)
			{
				tmpl->id.spi = protos[i].spi;
			}
			tmpl->aalgos = tmpl->ealgos = tmpl->calgos = ~0;
			tmpl->mode = mode2kernel(proto_mode);
			tmpl->optional = protos[i].proto == IPPROTO_COMP &&
							 policy->direction != POLICY_OUT;
			tmpl->family = ipsec->src->get_family(ipsec->src);

			if (proto_mode == MODE_TUNNEL || proto_mode == MODE_BEET)
			{	/* only for tunnel mode */
				host2xfrm(ipsec->src, &tmpl->saddr);
				host2xfrm(ipsec->dst, &tmpl->id.daddr);
			}

			tmpl++;

			/* use transport mode for other SAs */
			proto_mode = MODE_TRANSPORT;
		}
	}

	if (!add_mark(hdr, sizeof(request), ipsec->mark))
	{
		policy_change_done(this, policy);
		return FAILED;
	}
	this->mutex->unlock(this->mutex);

	status = this->socket_xfrm->send_ack(this->socket_xfrm, hdr);
	if (status == ALREADY_DONE && !update)
	{
		DBG1(DBG_KNL, "policy already exists, try to update it");
		hdr->nlmsg_type = XFRM_MSG_UPDPOLICY;
		status = this->socket_xfrm->send_ack(this->socket_xfrm, hdr);
	}

	this->mutex->lock(this->mutex);
	if (status != SUCCESS)
	{
		policy_change_done(this, policy);
		return FAILED;
	}
	/* install a route, if:
	 * - this is an outbound policy (to just get one for each child)
	 * - routing is not disabled via strongswan.conf
	 * - the selector is not for a specific protocol/port
	 * - we are in tunnel/BEET mode or install a bypass policy
	 */
	if (policy->direction == POLICY_OUT && this->install_routes &&
		!policy->sel.proto && !policy->sel.dport && !policy->sel.sport)
	{
		if (mapping->type == POLICY_PASS ||
		   (mapping->type == POLICY_IPSEC && ipsec->cfg.mode != MODE_TRANSPORT))
		{
			install_route(this, policy, mapping, ipsec);
		}
	}
	policy_change_done(this, policy);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_netlink_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	policy_entry_t *policy, *current;
	policy_sa_t *assigned_sa, *current_sa;
	enumerator_t *enumerator;
	bool found = FALSE, update = TRUE;
	char markstr[32] = "";
	uint32_t cur_priority = 0;
	int use_count;

	/* create a policy */
	INIT(policy,
		.sel = ts2selector(id->src_ts, id->dst_ts, id->interface),
		.mark = id->mark.value & id->mark.mask,
		.direction = id->dir,
		.reqid = data->sa->reqid,
	);
	format_mark(markstr, sizeof(markstr), id->mark);

	/* find the policy, which matches EXACTLY */
	this->mutex->lock(this->mutex);
	current = this->policies->get(this->policies, policy);
	if (current)
	{
		if (current->reqid && data->sa->reqid &&
			current->reqid != data->sa->reqid)
		{
			DBG1(DBG_CFG, "unable to install policy %R === %R %N%s for reqid "
				 "%u, the same policy for reqid %u exists",
				 id->src_ts, id->dst_ts, policy_dir_names, id->dir, markstr,
				 data->sa->reqid, current->reqid);
			policy_entry_destroy(this, policy);
			this->mutex->unlock(this->mutex);
			return INVALID_STATE;
		}
		/* use existing policy */
		DBG2(DBG_KNL, "policy %R === %R %N%s already exists, increasing "
			 "refcount", id->src_ts, id->dst_ts, policy_dir_names, id->dir,
			 markstr);
		policy_entry_destroy(this, policy);
		policy = current;
		found = TRUE;

		policy->waiting++;
		while (policy->working)
		{
			this->condvar->wait(this->condvar, this->mutex);
		}
		policy->waiting--;
		policy->working = TRUE;
	}
	else
	{	/* use the new one, if we have no such policy */
		policy->used_by = linked_list_create();
		this->policies->put(this->policies, policy, policy);
	}

	/* cache the assigned IPsec SA */
	assigned_sa = policy_sa_create(this, id->dir, data->type, data->src,
						data->dst, id->src_ts, id->dst_ts, id->mark, data->sa);
	assigned_sa->auto_priority = get_priority(policy, data->prio, id->interface);
	assigned_sa->priority = this->get_priority ? this->get_priority(id, data)
											   : data->manual_prio;
	assigned_sa->priority = assigned_sa->priority ?: assigned_sa->auto_priority;

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
		if (update)
		{
			cur_priority = current_sa->priority;
			update = FALSE;
		}
	}
	policy->used_by->insert_before(policy->used_by, enumerator, assigned_sa);
	enumerator->destroy(enumerator);

	use_count = policy->used_by->get_count(policy->used_by);
	if (!update)
	{	/* we don't update the policy if the priority is lower than that of
		 * the currently installed one */
		policy_change_done(this, policy);
		DBG2(DBG_KNL, "not updating policy %R === %R %N%s [priority %u, "
			 "refcount %d]", id->src_ts, id->dst_ts, policy_dir_names,
			 id->dir, markstr, cur_priority, use_count);
		return SUCCESS;
	}
	policy->reqid = assigned_sa->sa->cfg.reqid;

	if (this->policy_update)
	{
		found = TRUE;
	}

	DBG2(DBG_KNL, "%s policy %R === %R %N%s [priority %u, refcount %d]",
		 found ? "updating" : "adding", id->src_ts, id->dst_ts,
		 policy_dir_names, id->dir, markstr, assigned_sa->priority, use_count);

	if (add_policy_internal(this, policy, assigned_sa, found) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to %s policy %R === %R %N%s",
			 found ? "update" : "add", id->src_ts, id->dst_ts,
			 policy_dir_names, id->dir, markstr);
		return FAILED;
	}
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_netlink_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	netlink_buf_t request;
	struct nlmsghdr *out = NULL, *hdr;
	struct xfrm_userpolicy_id *policy_id;
	struct xfrm_userpolicy_info *policy = NULL;
	size_t len;
	char markstr[32] = "";

	memset(&request, 0, sizeof(request));
	format_mark(markstr, sizeof(markstr), id->mark);

	DBG2(DBG_KNL, "querying policy %R === %R %N%s", id->src_ts, id->dst_ts,
		 policy_dir_names, id->dir, markstr);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETPOLICY;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_userpolicy_id));

	policy_id = NLMSG_DATA(hdr);
	policy_id->sel = ts2selector(id->src_ts, id->dst_ts, id->interface);
	policy_id->dir = id->dir;

	if (!add_mark(hdr, sizeof(request), id->mark))
	{
		return FAILED;
	}

	if (this->socket_xfrm->send(this->socket_xfrm, hdr, &out, &len) == SUCCESS)
	{
		hdr = out;
		while (NLMSG_OK(hdr, len))
		{
			switch (hdr->nlmsg_type)
			{
				case XFRM_MSG_NEWPOLICY:
				{
					policy = NLMSG_DATA(hdr);
					break;
				}
				case NLMSG_ERROR:
				{
					struct nlmsgerr *err = NLMSG_DATA(hdr);
					DBG1(DBG_KNL, "querying policy failed: %s (%d)",
						 strerror(-err->error), -err->error);
					break;
				}
				default:
					hdr = NLMSG_NEXT(hdr, len);
					continue;
				case NLMSG_DONE:
					break;
			}
			break;
		}
	}

	if (policy == NULL)
	{
		DBG2(DBG_KNL, "unable to query policy %R === %R %N%s", id->src_ts,
			 id->dst_ts, policy_dir_names, id->dir, markstr);
		free(out);
		return FAILED;
	}

	if (policy->curlft.use_time)
	{
		/* we need the monotonic time, but the kernel returns system time. */
		*use_time = time_monotonic(NULL) - (time(NULL) - policy->curlft.use_time);
	}
	else
	{
		*use_time = 0;
	}

	free(out);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_kernel_netlink_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	policy_entry_t *current, policy;
	enumerator_t *enumerator;
	policy_sa_t *mapping;
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct xfrm_userpolicy_id *policy_id;
	bool is_installed = TRUE;
	uint32_t priority, auto_priority, cur_priority;
	ipsec_sa_t assigned_sa = {
		.src = data->src,
		.dst = data->dst,
		.mark = id->mark,
		.cfg = *data->sa,
	};
	char markstr[32] = "";
	int use_count;
	status_t status = SUCCESS;

	format_mark(markstr, sizeof(markstr), id->mark);

	DBG2(DBG_KNL, "deleting policy %R === %R %N%s", id->src_ts, id->dst_ts,
		 policy_dir_names, id->dir, markstr);

	/* create a policy */
	memset(&policy, 0, sizeof(policy_entry_t));
	policy.sel = ts2selector(id->src_ts, id->dst_ts, id->interface);
	policy.mark = id->mark.value & id->mark.mask;
	policy.direction = id->dir;

	/* find the policy */
	this->mutex->lock(this->mutex);
	current = this->policies->get(this->policies, &policy);
	if (!current)
	{
		DBG1(DBG_KNL, "deleting policy %R === %R %N%s failed, not found",
			 id->src_ts, id->dst_ts, policy_dir_names, id->dir, markstr);
		this->mutex->unlock(this->mutex);
		return NOT_FOUND;
	}
	current->waiting++;
	while (current->working)
	{
		this->condvar->wait(this->condvar, this->mutex);
	}
	current->working = TRUE;
	current->waiting--;

	/* remove mapping to SA by reqid and priority */
	auto_priority = get_priority(current, data->prio,id->interface);
	priority = this->get_priority ? this->get_priority(id, data)
								  : data->manual_prio;
	priority = priority ?: auto_priority;

	enumerator = current->used_by->create_enumerator(current->used_by);
	while (enumerator->enumerate(enumerator, (void**)&mapping))
	{
		if (priority == mapping->priority &&
			auto_priority == mapping->auto_priority &&
			data->type == mapping->type &&
			ipsec_sa_equals(mapping->sa, &assigned_sa))
		{
			current->used_by->remove_at(current->used_by, enumerator);
			policy_sa_destroy(mapping, id->dir, this);
			break;
		}
		if (is_installed)
		{
			cur_priority = mapping->priority;
			is_installed = FALSE;
		}
	}
	enumerator->destroy(enumerator);

	use_count = current->used_by->get_count(current->used_by);
	if (use_count > 0)
	{	/* policy is used by more SAs, keep in kernel */
		DBG2(DBG_KNL, "policy still used by another CHILD_SA, not removed");
		if (!is_installed)
		{	/* no need to update as the policy was not installed for this SA */
			policy_change_done(this, current);
			DBG2(DBG_KNL, "not updating policy %R === %R %N%s [priority %u, "
				 "refcount %d]", id->src_ts, id->dst_ts, policy_dir_names,
				 id->dir, markstr, cur_priority, use_count);
			return SUCCESS;
		}
		current->used_by->get_first(current->used_by, (void**)&mapping);
		current->reqid = mapping->sa->cfg.reqid;

		DBG2(DBG_KNL, "updating policy %R === %R %N%s [priority %u, "
			 "refcount %d]", id->src_ts, id->dst_ts, policy_dir_names, id->dir,
			 markstr, mapping->priority, use_count);

		if (add_policy_internal(this, current, mapping, TRUE) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to update policy %R === %R %N%s",
				 id->src_ts, id->dst_ts, policy_dir_names, id->dir, markstr);
			return FAILED;
		}
		return SUCCESS;
	}

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_DELPOLICY;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_userpolicy_id));

	policy_id = NLMSG_DATA(hdr);
	policy_id->sel = current->sel;
	policy_id->dir = id->dir;

	if (!add_mark(hdr, sizeof(request), id->mark))
	{
		policy_change_done(this, current);
		return FAILED;
	}

	if (current->route)
	{
		route_entry_t *route = current->route;
		if (charon->kernel->del_route(charon->kernel, route->dst_net,
									  route->prefixlen, route->gateway,
									  route->src_ip, route->if_name) != SUCCESS)
		{
			DBG1(DBG_KNL, "error uninstalling route installed with policy "
				 "%R === %R %N%s", id->src_ts, id->dst_ts, policy_dir_names,
				 id->dir, markstr);
		}
	}
	this->mutex->unlock(this->mutex);

	if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to delete policy %R === %R %N%s", id->src_ts,
			 id->dst_ts, policy_dir_names, id->dir, markstr);
		status = FAILED;
	}

	this->mutex->lock(this->mutex);
	if (!current->waiting)
	{	/* only if no other thread still needs the policy */
		this->policies->remove(this->policies, current);
		policy_entry_destroy(this, current);
		this->mutex->unlock(this->mutex);
	}
	else
	{
		policy_change_done(this, current);
	}
	return status;
}

METHOD(kernel_ipsec_t, flush_policies, status_t,
	private_kernel_netlink_ipsec_t *this)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "flushing all policies from SPD");

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_FLUSHPOLICY;
	hdr->nlmsg_len = NLMSG_LENGTH(0); /* no data associated */

	/* by adding an rtattr of type  XFRMA_POLICY_TYPE we could restrict this
	 * to main or sub policies (default is main) */

	if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to flush SPD entries");
		return FAILED;
	}
	return SUCCESS;
}

/**
 * Bypass socket using a per-socket policy
 */
static bool add_socket_bypass(private_kernel_netlink_ipsec_t *this,
							  int fd, int family)
{
	struct xfrm_userpolicy_info policy;
	u_int sol, ipsec_policy;

	switch (family)
	{
		case AF_INET:
			sol = SOL_IP;
			ipsec_policy = IP_XFRM_POLICY;
			break;
		case AF_INET6:
			sol = SOL_IPV6;
			ipsec_policy = IPV6_XFRM_POLICY;
			break;
		default:
			return FALSE;
	}

	memset(&policy, 0, sizeof(policy));
	policy.action = XFRM_POLICY_ALLOW;
	policy.sel.family = family;

	policy.dir = XFRM_POLICY_OUT;
	if (setsockopt(fd, sol, ipsec_policy, &policy, sizeof(policy)) < 0)
	{
		DBG1(DBG_KNL, "unable to set IPSEC_POLICY on socket: %s (%d)",
			 strerror(errno), errno);
		return FALSE;
	}
	policy.dir = XFRM_POLICY_IN;
	if (setsockopt(fd, sol, ipsec_policy, &policy, sizeof(policy)) < 0)
	{
		DBG1(DBG_KNL, "unable to set IPSEC_POLICY on socket: %s (%d)",
			 strerror(errno), errno);
		return FALSE;
	}
	return TRUE;
}

/**
 * Port based IKE bypass policy
 */
typedef struct {
	/** address family */
	int family;
	/** layer 4 protocol */
	int proto;
	/** port number, network order */
	uint16_t port;
} bypass_t;

/**
 * Add or remove a bypass policy from/to kernel
 */
static bool manage_bypass(private_kernel_netlink_ipsec_t *this,
						  int type, policy_dir_t dir, bypass_t *bypass)
{
	netlink_buf_t request;
	struct xfrm_selector *sel;
	struct nlmsghdr *hdr;

	memset(&request, 0, sizeof(request));
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = type;

	if (type == XFRM_MSG_NEWPOLICY)
	{
		struct xfrm_userpolicy_info *policy;

		hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_userpolicy_info));

		policy = NLMSG_DATA(hdr);
		policy->dir = dir;
		policy->priority = 32;
		policy->action = XFRM_POLICY_ALLOW;
		policy->share = XFRM_SHARE_ANY;

		policy->lft.soft_byte_limit = XFRM_INF;
		policy->lft.soft_packet_limit = XFRM_INF;
		policy->lft.hard_byte_limit = XFRM_INF;
		policy->lft.hard_packet_limit = XFRM_INF;

		sel = &policy->sel;
	}
	else /* XFRM_MSG_DELPOLICY */
	{
		struct xfrm_userpolicy_id *policy;

		hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_userpolicy_id));

		policy = NLMSG_DATA(hdr);
		policy->dir = dir;

		sel = &policy->sel;
	}

	sel->family = bypass->family;
	sel->proto = bypass->proto;
	if (dir == POLICY_IN)
	{
		sel->dport = bypass->port;
		sel->dport_mask = 0xffff;
	}
	else
	{
		sel->sport = bypass->port;
		sel->sport_mask = 0xffff;
	}
	return this->socket_xfrm->send_ack(this->socket_xfrm, hdr) == SUCCESS;
}

/**
 * Bypass socket using a port-based bypass policy
 */
static bool add_port_bypass(private_kernel_netlink_ipsec_t *this,
							int fd, int family)
{
	union {
		struct sockaddr sa;
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
	} saddr;
	socklen_t len;
	bypass_t bypass = {
		.family = family,
	};

	len = sizeof(saddr);
	if (getsockname(fd, &saddr.sa, &len) != 0)
	{
		return FALSE;
	}
#ifdef SO_PROTOCOL /* since 2.6.32 */
	len = sizeof(bypass.proto);
	if (getsockopt(fd, SOL_SOCKET, SO_PROTOCOL, &bypass.proto, &len) != 0)
#endif
	{	/* assume UDP if SO_PROTOCOL not supported */
		bypass.proto = IPPROTO_UDP;
	}
	switch (family)
	{
		case AF_INET:
			bypass.port = saddr.in.sin_port;
			break;
		case AF_INET6:
			bypass.port = saddr.in6.sin6_port;
			break;
		default:
			return FALSE;
	}

	if (!manage_bypass(this, XFRM_MSG_NEWPOLICY, POLICY_IN, &bypass))
	{
		return FALSE;
	}
	if (!manage_bypass(this, XFRM_MSG_NEWPOLICY, POLICY_OUT, &bypass))
	{
		manage_bypass(this, XFRM_MSG_DELPOLICY, POLICY_IN, &bypass);
		return FALSE;
	}
	array_insert(this->bypass, ARRAY_TAIL, &bypass);

	return TRUE;
}

/**
 * Remove installed port based bypass policy
 */
static void remove_port_bypass(bypass_t *bypass, int idx,
							   private_kernel_netlink_ipsec_t *this)
{
	manage_bypass(this, XFRM_MSG_DELPOLICY, POLICY_OUT, bypass);
	manage_bypass(this, XFRM_MSG_DELPOLICY, POLICY_IN, bypass);
}

METHOD(kernel_ipsec_t, bypass_socket, bool,
	private_kernel_netlink_ipsec_t *this, int fd, int family)
{
	if (lib->settings->get_bool(lib->settings,
					"%s.plugins.kernel-netlink.port_bypass", FALSE, lib->ns))
	{
		return add_port_bypass(this, fd, family);
	}
	return add_socket_bypass(this, fd, family);
}

METHOD(kernel_ipsec_t, enable_udp_decap, bool,
	private_kernel_netlink_ipsec_t *this, int fd, int family, uint16_t port)
{
	int type = UDP_ENCAP_ESPINUDP;

	if (setsockopt(fd, SOL_UDP, UDP_ENCAP, &type, sizeof(type)) < 0)
	{
		DBG1(DBG_KNL, "unable to set UDP_ENCAP: %s", strerror(errno));
		return FALSE;
	}
	return TRUE;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_kernel_netlink_ipsec_t *this)
{
	enumerator_t *enumerator;
	policy_entry_t *policy;

	array_destroy_function(this->bypass,
						   (array_callback_t)remove_port_bypass, this);
	if (this->socket_xfrm_events > 0)
	{
		lib->watcher->remove(lib->watcher, this->socket_xfrm_events);
		close(this->socket_xfrm_events);
	}
	DESTROY_IF(this->socket_xfrm);
	enumerator = this->policies->create_enumerator(this->policies);
	while (enumerator->enumerate(enumerator, &policy, &policy))
	{
		policy_entry_destroy(this, policy);
	}
	enumerator->destroy(enumerator);
	this->policies->destroy(this->policies);
	this->sas->destroy(this->sas);
	this->condvar->destroy(this->condvar);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * Get the currently configured SPD hashing thresholds for an address family
 */
static bool get_spd_hash_thresh(private_kernel_netlink_ipsec_t *this,
								int type, uint8_t *lbits, uint8_t *rbits)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out;
	struct xfrmu_spdhthresh *thresh;
	struct rtattr *rta;
	size_t len, rtasize;
	bool success = FALSE;

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETSPDINFO;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(uint32_t));

	if (this->socket_xfrm->send(this->socket_xfrm, hdr, &out, &len) == SUCCESS)
	{
		hdr = out;
		while (NLMSG_OK(hdr, len))
		{
			switch (hdr->nlmsg_type)
			{
				case XFRM_MSG_NEWSPDINFO:
				{
					rta = XFRM_RTA(hdr, uint32_t);
					rtasize = XFRM_PAYLOAD(hdr, uint32_t);
					while (RTA_OK(rta, rtasize))
					{
						if (rta->rta_type == type &&
							RTA_PAYLOAD(rta) == sizeof(*thresh))
						{
							thresh = RTA_DATA(rta);
							*lbits = thresh->lbits;
							*rbits = thresh->rbits;
							success = TRUE;
							break;
						}
						rta = RTA_NEXT(rta, rtasize);
					}
					break;
				}
				case NLMSG_ERROR:
				{
					struct nlmsgerr *err = NLMSG_DATA(hdr);
					DBG1(DBG_KNL, "getting SPD hash threshold failed: %s (%d)",
						 strerror(-err->error), -err->error);
					break;
				}
				default:
					hdr = NLMSG_NEXT(hdr, len);
					continue;
				case NLMSG_DONE:
					break;
			}
			break;
		}
		free(out);
	}
	return success;
}

/**
 * Configure SPD hashing threshold for an address family
 */
static void setup_spd_hash_thresh(private_kernel_netlink_ipsec_t *this,
								  char *key, int type, uint8_t def)
{
	struct xfrmu_spdhthresh *thresh;
	struct nlmsghdr *hdr;
	netlink_buf_t request;
	uint8_t lbits, rbits;

	if (!get_spd_hash_thresh(this, type, &lbits, &rbits))
	{
		return;
	}
	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_NEWSPDINFO;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(uint32_t));

	thresh = netlink_reserve(hdr, sizeof(request), type, sizeof(*thresh));
	thresh->lbits = lib->settings->get_int(lib->settings,
							"%s.plugins.kernel-netlink.spdh_thresh.%s.lbits",
							def, lib->ns, key);
	thresh->rbits = lib->settings->get_int(lib->settings,
							"%s.plugins.kernel-netlink.spdh_thresh.%s.rbits",
							def, lib->ns, key);
	if (thresh->lbits != lbits || thresh->rbits != rbits)
	{
		if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
		{
			DBG1(DBG_KNL, "setting SPD hash threshold failed");
		}
	}
}

/*
 * Described in header.
 */
kernel_netlink_ipsec_t *kernel_netlink_ipsec_create()
{
	private_kernel_netlink_ipsec_t *this;
	bool register_for_events = TRUE;

	INIT(this,
		.public = {
			.interface = {
				.get_features = _get_features,
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
		.policies = hashtable_create((hashtable_hash_t)policy_hash,
									 (hashtable_equals_t)policy_equals, 32),
		.sas = hashtable_create((hashtable_hash_t)ipsec_sa_hash,
								(hashtable_equals_t)ipsec_sa_equals, 32),
		.bypass = array_create(sizeof(bypass_t), 0),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		.get_priority = dlsym(RTLD_DEFAULT,
							  "kernel_netlink_get_priority_custom"),
		.policy_update = lib->settings->get_bool(lib->settings,
					"%s.plugins.kernel-netlink.policy_update", FALSE, lib->ns),
		.install_routes = lib->settings->get_bool(lib->settings,
							"%s.install_routes", TRUE, lib->ns),
		.proto_port_transport = lib->settings->get_bool(lib->settings,
						"%s.plugins.kernel-netlink.set_proto_port_transport_sa",
						FALSE, lib->ns),
	);

	if (streq(lib->ns, "starter"))
	{	/* starter has no threads, so we do not register for kernel events */
		register_for_events = FALSE;
	}

	this->socket_xfrm = netlink_socket_create(NETLINK_XFRM, xfrm_msg_names,
				lib->settings->get_bool(lib->settings,
					"%s.plugins.kernel-netlink.parallel_xfrm", FALSE, lib->ns));
	if (!this->socket_xfrm)
	{
		destroy(this);
		return NULL;
	}

	setup_spd_hash_thresh(this, "ipv4", XFRMA_SPD_IPV4_HTHRESH, 32);
	setup_spd_hash_thresh(this, "ipv6", XFRMA_SPD_IPV6_HTHRESH, 128);

	if (register_for_events)
	{
		struct sockaddr_nl addr;

		memset(&addr, 0, sizeof(addr));
		addr.nl_family = AF_NETLINK;

		/* create and bind XFRM socket for ACQUIRE, EXPIRE, MIGRATE & MAPPING */
		this->socket_xfrm_events = socket(AF_NETLINK, SOCK_RAW, NETLINK_XFRM);
		if (this->socket_xfrm_events <= 0)
		{
			DBG1(DBG_KNL, "unable to create XFRM event socket: %s (%d)",
				 strerror(errno), errno);
			destroy(this);
			return NULL;
		}
		addr.nl_groups = XFRMNLGRP(ACQUIRE) | XFRMNLGRP(EXPIRE) |
						 XFRMNLGRP(MIGRATE) | XFRMNLGRP(MAPPING);
		if (bind(this->socket_xfrm_events, (struct sockaddr*)&addr, sizeof(addr)))
		{
			DBG1(DBG_KNL, "unable to bind XFRM event socket: %s (%d)",
				 strerror(errno), errno);
			destroy(this);
			return NULL;
		}
		lib->watcher->add(lib->watcher, this->socket_xfrm_events, WATCHER_READ,
						  (watcher_cb_t)receive_events, this);
	}

	return &this->public;
}
