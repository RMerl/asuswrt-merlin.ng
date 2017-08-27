/*
 * Copyright (C) 2006-2013 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2008 Andreas Steffen
 * Copyright (C) 2006-2007 Fabian Hartmann, Noah Heusser
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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

#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <linux/ipsec.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/xfrm.h>
#include <linux/udp.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "kernel_netlink_ipsec.h"
#include "kernel_netlink_shared.h"

#include <hydra.h>
#include <utils/debug.h>
#include <threading/mutex.h>
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
#define PRIO_BASE 384

/** Default lifetime of an acquire XFRM state (in seconds) */
#define DEFAULT_ACQUIRE_LIFETIME 165

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
	char *name;
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

ENUM(xfrm_attr_type_names, XFRMA_UNSPEC, XFRMA_REPLAY_ESN_VAL,
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
 * Look up a kernel algorithm name and its key size
 */
static char* lookup_algorithm(transform_type_t type, int ikev2)
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
	if (hydra->kernel_interface->lookup_algorithm(hydra->kernel_interface,
												  ikev2, type, NULL, &name))
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
	 * Whether to track the history of a policy
	 */
	bool policy_history;
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
	u_int8_t prefixlen;
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
		   memeq(&sa->mark, &other_sa->mark, sizeof(mark_t)) &&
		   memeq(&sa->cfg, &other_sa->cfg, sizeof(ipsec_sa_cfg_t));
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
typedef struct policy_sa_fwd_t policy_sa_fwd_t;

/**
 * Mapping between a policy and an IPsec SA.
 */
struct policy_sa_t {
	/** Priority assigned to the policy when installed with this SA */
	u_int32_t priority;

	/** Type of the policy */
	policy_type_t type;

	/** Assigned SA */
	ipsec_sa_t *sa;
};

/**
 * For forward policies we also cache the traffic selectors in order to install
 * the route.
 */
struct policy_sa_fwd_t {
	/** Generic interface */
	policy_sa_t generic;

	/** Source traffic selector of this policy */
	traffic_selector_t *src_ts;

	/** Destination traffic selector of this policy */
	traffic_selector_t *dst_ts;
};

/**
 * Create a policy_sa(_fwd)_t object
 */
static policy_sa_t *policy_sa_create(private_kernel_netlink_ipsec_t *this,
	policy_dir_t dir, policy_type_t type, host_t *src, host_t *dst,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts, mark_t mark,
	ipsec_sa_cfg_t *cfg)
{
	policy_sa_t *policy;

	if (dir == POLICY_FWD)
	{
		policy_sa_fwd_t *fwd;
		INIT(fwd,
			.src_ts = src_ts->clone(src_ts),
			.dst_ts = dst_ts->clone(dst_ts),
		);
		policy = &fwd->generic;
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
 * Destroy a policy_sa(_fwd)_t object
 */
static void policy_sa_destroy(policy_sa_t *policy, policy_dir_t *dir,
							  private_kernel_netlink_ipsec_t *this)
{
	if (*dir == POLICY_FWD)
	{
		policy_sa_fwd_t *fwd = (policy_sa_fwd_t*)policy;
		fwd->src_ts->destroy(fwd->src_ts);
		fwd->dst_ts->destroy(fwd->dst_ts);
	}
	ipsec_sa_destroy(this, policy->sa);
	free(policy);
}

typedef struct policy_entry_t policy_entry_t;

/**
 * Installed kernel policy.
 */
struct policy_entry_t {

	/** Direction of this policy: in, out, forward */
	u_int8_t direction;

	/** Parameters of installed policy */
	struct xfrm_selector sel;

	/** Optional mark */
	u_int32_t mark;

	/** Associated route installed for this policy */
	route_entry_t *route;

	/** List of SAs this policy is used by, ordered by priority */
	linked_list_t *used_by;

	/** reqid for this policy */
	u_int32_t reqid;
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
		policy->used_by->invoke_function(policy->used_by,
										(linked_list_invoke_t)policy_sa_destroy,
										 &policy->direction, this);
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
 * Calculate the priority of a policy
 */
static inline u_int32_t get_priority(policy_entry_t *policy,
									 policy_priority_t prio)
{
	u_int32_t priority = PRIO_BASE;
	switch (prio)
	{
		case POLICY_PRIORITY_FALLBACK:
			priority <<= 1;
			/* fall-through */
		case POLICY_PRIORITY_ROUTED:
			priority <<= 1;
			/* fall-through */
		case POLICY_PRIORITY_DEFAULT:
			priority <<= 1;
			/* fall-through */
		case POLICY_PRIORITY_PASS:
			break;
	}
	/* calculate priority based on selector size, small size = high prio */
	priority -= policy->sel.prefixlen_s;
	priority -= policy->sel.prefixlen_d;
	priority <<= 2; /* make some room for the two flags */
	priority += policy->sel.sport_mask || policy->sel.dport_mask ? 0 : 2;
	priority += policy->sel.proto ? 0 : 1;
	return priority;
}

/**
 * Convert the general ipsec mode to the one defined in xfrm.h
 */
static u_int8_t mode2kernel(ipsec_mode_t mode)
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
static host_t* xfrm2host(int family, xfrm_address_t *xfrm, u_int16_t port)
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
					  xfrm_address_t *net, u_int8_t *mask)
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
					 u_int16_t *port, u_int16_t *mask)
{
	/* Linux does not seem to accept complex portmasks. Only
	 * any or a specific port is allowed. We set to any, if we have
	 * a port range, or to a specific, if we have one port only.
	 */
	u_int16_t from, to;

	from = ts->get_from_port(ts);
	to = ts->get_to_port(ts);

	if (from == to)
	{
		*port = htons(from);
		*mask = ~0;
	}
	else
	{
		*port = 0;
		*mask = 0;
	}
}

/**
 * Convert a pair of traffic_selectors to an xfrm_selector
 */
static struct xfrm_selector ts2selector(traffic_selector_t *src,
										traffic_selector_t *dst)
{
	struct xfrm_selector sel;

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
		/* the ICMP type is encoded in the most significant 8 bits and the ICMP
		 * code in the least significant 8 bits of the port.  via XFRM we have
		 * to pass the ICMP type and code in the source and destination port
		 * fields, respectively.  the port is in network byte order. */
		u_int16_t port = max(sel.dport, sel.sport);
		sel.sport = htons(port & 0xff);
		sel.dport = htons(port >> 8);
	}
	sel.ifindex = 0;
	sel.user = 0;

	return sel;
}

/**
 * Convert an xfrm_selector to a src|dst traffic_selector
 */
static traffic_selector_t* selector2ts(struct xfrm_selector *sel, bool src)
{
	u_char *addr;
	u_int8_t prefixlen;
	u_int16_t port = 0;
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
	u_int32_t reqid = 0;
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

	hydra->kernel_interface->acquire(hydra->kernel_interface, reqid, src_ts,
									 dst_ts);
}

/**
 * Process a XFRM_MSG_EXPIRE from kernel
 */
static void process_expire(private_kernel_netlink_ipsec_t *this,
						   struct nlmsghdr *hdr)
{
	struct xfrm_user_expire *expire;
	u_int32_t spi, reqid;
	u_int8_t protocol;

	expire = NLMSG_DATA(hdr);
	protocol = expire->state.id.proto;
	spi = expire->state.id.spi;
	reqid = expire->state.reqid;

	DBG2(DBG_KNL, "received a XFRM_MSG_EXPIRE");

	if (protocol != IPPROTO_ESP && protocol != IPPROTO_AH)
	{
		DBG2(DBG_KNL, "ignoring XFRM_MSG_EXPIRE for SA with SPI %.8x and "
					  "reqid {%u} which is not a CHILD_SA", ntohl(spi), reqid);
		return;
	}

	hydra->kernel_interface->expire(hydra->kernel_interface, reqid, protocol,
									spi, expire->hard != 0);
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
	u_int32_t reqid = 0;
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
		hydra->kernel_interface->migrate(hydra->kernel_interface, reqid,
										 src_ts, dst_ts, dir, local, remote);
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
	u_int32_t spi, reqid;

	mapping = NLMSG_DATA(hdr);
	spi = mapping->id.spi;
	reqid = mapping->reqid;

	DBG2(DBG_KNL, "received a XFRM_MSG_MAPPING");

	if (mapping->id.proto == IPPROTO_ESP)
	{
		host_t *host;
		host = xfrm2host(mapping->id.family, &mapping->new_saddr,
						 mapping->new_sport);
		if (host)
		{
			hydra->kernel_interface->mapping(hydra->kernel_interface, reqid,
											 spi, host);
		}
	}
}

/**
 * Receives events from kernel
 */
static bool receive_events(private_kernel_netlink_ipsec_t *this, int fd,
						   watcher_event_t event)
{
	char response[1024];
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
				DBG1(DBG_KNL, "unable to receive from xfrm event socket");
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
				DBG1(DBG_KNL, "received unknown event from xfrm event "
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
	return KERNEL_ESP_V3_TFC;
}

/**
 * Get an SPI for a specific protocol from the kernel.
 */
static status_t get_spi_internal(private_kernel_netlink_ipsec_t *this,
	host_t *src, host_t *dst, u_int8_t proto, u_int32_t min, u_int32_t max,
	u_int32_t reqid, u_int32_t *spi)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out;
	struct xfrm_userspi_info *userspi;
	u_int32_t received_spi = 0;
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
	userspi->info.reqid = reqid;
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
	u_int8_t protocol, u_int32_t reqid, u_int32_t *spi)
{
	DBG2(DBG_KNL, "getting SPI for reqid {%u}", reqid);

	if (get_spi_internal(this, src, dst, protocol,
						 0xc0000000, 0xcFFFFFFF, reqid, spi) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to get SPI for reqid {%u}", reqid);
		return FAILED;
	}

	DBG2(DBG_KNL, "got SPI %.8x for reqid {%u}", ntohl(*spi), reqid);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_kernel_netlink_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t reqid, u_int16_t *cpi)
{
	u_int32_t received_spi = 0;

	DBG2(DBG_KNL, "getting CPI for reqid {%u}", reqid);

	if (get_spi_internal(this, src, dst, IPPROTO_COMP,
						 0x100, 0xEFFF, reqid, &received_spi) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to get CPI for reqid {%u}", reqid);
		return FAILED;
	}

	*cpi = htons((u_int16_t)ntohl(received_spi));

	DBG2(DBG_KNL, "got CPI %.4x for reqid {%u}", ntohs(*cpi), reqid);
	return SUCCESS;
}

/**
 * Add a XFRM mark to message if required
 */
static bool add_mark(struct nlmsghdr *hdr, int buflen, mark_t mark)
{
	if (mark.value)
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

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_netlink_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t spi, u_int8_t protocol, u_int32_t reqid, mark_t mark,
	u_int32_t tfc, lifetime_cfg_t *lifetime, u_int16_t enc_alg, chunk_t enc_key,
	u_int16_t int_alg, chunk_t int_key, ipsec_mode_t mode,
	u_int16_t ipcomp, u_int16_t cpi, u_int32_t replay_window,
	bool initiator, bool encap, bool esn, bool inbound,
	traffic_selector_t* src_ts, traffic_selector_t* dst_ts)
{
	netlink_buf_t request;
	char *alg_name;
	struct nlmsghdr *hdr;
	struct xfrm_usersa_info *sa;
	u_int16_t icv_size = 64;
	ipsec_mode_t original_mode = mode;
	status_t status = FAILED;

	/* if IPComp is used, we install an additional IPComp SA. if the cpi is 0
	 * we are in the recursive call below */
	if (ipcomp != IPCOMP_NONE && cpi != 0)
	{
		lifetime_cfg_t lft = {{0,0,0},{0,0,0},{0,0,0}};
		add_sa(this, src, dst, htonl(ntohs(cpi)), IPPROTO_COMP, reqid, mark,
			   tfc, &lft, ENCR_UNDEFINED, chunk_empty, AUTH_UNDEFINED,
			   chunk_empty, mode, ipcomp, 0, 0, initiator, FALSE, FALSE,
			   inbound, src_ts, dst_ts);
		ipcomp = IPCOMP_NONE;
		/* use transport mode ESP SA, IPComp uses tunnel mode */
		mode = MODE_TRANSPORT;
	}

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "adding SAD entry with SPI %.8x and reqid {%u}  (mark "
				  "%u/0x%08x)", ntohl(spi), reqid, mark.value, mark.mask);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = inbound ? XFRM_MSG_UPDSA : XFRM_MSG_NEWSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_info));

	sa = NLMSG_DATA(hdr);
	host2xfrm(src, &sa->saddr);
	host2xfrm(dst, &sa->id.daddr);
	sa->id.spi = spi;
	sa->id.proto = protocol;
	sa->family = src->get_family(src);
	sa->mode = mode2kernel(mode);
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
			if (src_ts && dst_ts)
			{
				sa->sel = ts2selector(src_ts, dst_ts);
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

	sa->reqid = reqid;
	sa->lft.soft_byte_limit = XFRM_LIMIT(lifetime->bytes.rekey);
	sa->lft.hard_byte_limit = XFRM_LIMIT(lifetime->bytes.life);
	sa->lft.soft_packet_limit = XFRM_LIMIT(lifetime->packets.rekey);
	sa->lft.hard_packet_limit = XFRM_LIMIT(lifetime->packets.life);
	/* we use lifetimes since added, not since used */
	sa->lft.soft_add_expires_seconds = lifetime->time.rekey;
	sa->lft.hard_add_expires_seconds = lifetime->time.life;
	sa->lft.soft_use_expires_seconds = 0;
	sa->lft.hard_use_expires_seconds = 0;

	switch (enc_alg)
	{
		case ENCR_UNDEFINED:
			/* no encryption */
			break;
		case ENCR_AES_CCM_ICV16:
		case ENCR_AES_GCM_ICV16:
		case ENCR_NULL_AUTH_AES_GMAC:
		case ENCR_CAMELLIA_CCM_ICV16:
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

			alg_name = lookup_algorithm(ENCRYPTION_ALGORITHM, enc_alg);
			if (alg_name == NULL)
			{
				DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
						 encryption_algorithm_names, enc_alg);
					goto failed;
			}
			DBG2(DBG_KNL, "  using encryption algorithm %N with key size %d",
				 encryption_algorithm_names, enc_alg, enc_key.len * 8);

			algo = netlink_reserve(hdr, sizeof(request), XFRMA_ALG_AEAD,
								   sizeof(*algo) + enc_key.len);
			if (!algo)
			{
				goto failed;
			}
			algo->alg_key_len = enc_key.len * 8;
			algo->alg_icv_len = icv_size;
			strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
			algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
			memcpy(algo->alg_key, enc_key.ptr, enc_key.len);
			break;
		}
		default:
		{
			struct xfrm_algo *algo;

			alg_name = lookup_algorithm(ENCRYPTION_ALGORITHM, enc_alg);
			if (alg_name == NULL)
			{
				DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
					 encryption_algorithm_names, enc_alg);
				goto failed;
			}
			DBG2(DBG_KNL, "  using encryption algorithm %N with key size %d",
				 encryption_algorithm_names, enc_alg, enc_key.len * 8);

			algo = netlink_reserve(hdr, sizeof(request), XFRMA_ALG_CRYPT,
								   sizeof(*algo) + enc_key.len);
			if (!algo)
			{
				goto failed;
			}
			algo->alg_key_len = enc_key.len * 8;
			strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
			algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
			memcpy(algo->alg_key, enc_key.ptr, enc_key.len);
		}
	}

	if (int_alg != AUTH_UNDEFINED)
	{
		u_int trunc_len = 0;

		alg_name = lookup_algorithm(INTEGRITY_ALGORITHM, int_alg);
		if (alg_name == NULL)
		{
			DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
				 integrity_algorithm_names, int_alg);
			goto failed;
		}
		DBG2(DBG_KNL, "  using integrity algorithm %N with key size %d",
			 integrity_algorithm_names, int_alg, int_key.len * 8);

		switch (int_alg)
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
								   sizeof(*algo) + int_key.len);
			if (!algo)
			{
				goto failed;
			}
			algo->alg_key_len = int_key.len * 8;
			algo->alg_trunc_len = trunc_len;
			strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
			algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
			memcpy(algo->alg_key, int_key.ptr, int_key.len);
		}
		else
		{
			struct xfrm_algo* algo;

			algo = netlink_reserve(hdr, sizeof(request), XFRMA_ALG_AUTH,
								   sizeof(*algo) + int_key.len);
			if (!algo)
			{
				goto failed;
			}
			algo->alg_key_len = int_key.len * 8;
			strncpy(algo->alg_name, alg_name, sizeof(algo->alg_name));
			algo->alg_name[sizeof(algo->alg_name) - 1] = '\0';
			memcpy(algo->alg_key, int_key.ptr, int_key.len);
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

	if (encap)
	{
		struct xfrm_encap_tmpl *tmpl;

		tmpl = netlink_reserve(hdr, sizeof(request), XFRMA_ENCAP, sizeof(*tmpl));
		if (!tmpl)
		{
			goto failed;
		}
		tmpl->encap_type = UDP_ENCAP_ESPINUDP;
		tmpl->encap_sport = htons(src->get_port(src));
		tmpl->encap_dport = htons(dst->get_port(dst));
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

	if (!add_mark(hdr, sizeof(request), mark))
	{
		goto failed;
	}

	if (tfc && protocol == IPPROTO_ESP && mode == MODE_TUNNEL)
	{	/* the kernel supports TFC padding only for tunnel mode ESP SAs */
		u_int32_t *tfcpad;

		tfcpad = netlink_reserve(hdr, sizeof(request), XFRMA_TFCPAD,
								 sizeof(*tfcpad));
		if (!tfcpad)
		{
			goto failed;
		}
		*tfcpad = tfc;
	}

	if (protocol != IPPROTO_COMP)
	{
		if (replay_window != 0 && (esn || replay_window > 32))
		{
			/* for ESN or larger replay windows we need the new
			 * XFRMA_REPLAY_ESN_VAL attribute to configure a bitmap */
			struct xfrm_replay_state_esn *replay;
			u_int32_t bmp_size;

			bmp_size = round_up(replay_window, sizeof(u_int32_t) * 8) / 8;
			replay = netlink_reserve(hdr, sizeof(request), XFRMA_REPLAY_ESN_VAL,
									 sizeof(*replay) + bmp_size);
			if (!replay)
			{
				goto failed;
			}
			/* bmp_len contains number uf __u32's */
			replay->bmp_len = bmp_size / sizeof(u_int32_t);
			replay->replay_window = replay_window;
			DBG2(DBG_KNL, "  using replay window of %u packets", replay_window);

			if (esn)
			{
				DBG2(DBG_KNL, "  using extended sequence numbers (ESN)");
				sa->flags |= XFRM_STATE_ESN;
			}
		}
		else
		{
			DBG2(DBG_KNL, "  using replay window of %u packets", replay_window);
			sa->replay_window = replay_window;
		}
	}

	if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
	{
		if (mark.value)
		{
			DBG1(DBG_KNL, "unable to add SAD entry with SPI %.8x  "
						  "(mark %u/0x%08x)", ntohl(spi), mark.value, mark.mask);
		}
		else
		{
			DBG1(DBG_KNL, "unable to add SAD entry with SPI %.8x", ntohl(spi));
		}
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
							 u_int32_t spi, u_int8_t protocol,
							 host_t *dst, mark_t mark,
							 struct xfrm_replay_state_esn **replay_esn,
							 u_int32_t *replay_esn_len,
							 struct xfrm_replay_state **replay)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out = NULL;
	struct xfrm_aevent_id *out_aevent = NULL, *aevent_id;
	size_t len;
	struct rtattr *rta;
	size_t rtasize;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "querying replay state from SAD entry with SPI %.8x",
				   ntohl(spi));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETAE;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_aevent_id));

	aevent_id = NLMSG_DATA(hdr);
	aevent_id->flags = XFRM_AE_RVAL;

	host2xfrm(dst, &aevent_id->sa_id.daddr);
	aevent_id->sa_id.spi = spi;
	aevent_id->sa_id.proto = protocol;
	aevent_id->sa_id.family = dst->get_family(dst);

	if (!add_mark(hdr, sizeof(request), mark))
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
								  "failed: %s (%d)", strerror(-err->error),
								  -err->error);
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
			if (rta->rta_type == XFRMA_REPLAY_VAL &&
				RTA_PAYLOAD(rta) == sizeof(**replay))
			{
				*replay = malloc(RTA_PAYLOAD(rta));
				memcpy(*replay, RTA_DATA(rta), RTA_PAYLOAD(rta));
				break;
			}
			if (rta->rta_type == XFRMA_REPLAY_ESN_VAL &&
				RTA_PAYLOAD(rta) >= sizeof(**replay_esn))
			{
				*replay_esn = malloc(RTA_PAYLOAD(rta));
				*replay_esn_len = RTA_PAYLOAD(rta);
				memcpy(*replay_esn, RTA_DATA(rta), RTA_PAYLOAD(rta));
				break;
			}
			rta = RTA_NEXT(rta, rtasize);
		}
	}
	free(out);
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_netlink_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t spi, u_int8_t protocol, mark_t mark,
	u_int64_t *bytes, u_int64_t *packets, time_t *time)
{
	netlink_buf_t request;
	struct nlmsghdr *out = NULL, *hdr;
	struct xfrm_usersa_id *sa_id;
	struct xfrm_usersa_info *sa = NULL;
	status_t status = FAILED;
	size_t len;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "querying SAD entry with SPI %.8x  (mark %u/0x%08x)",
				   ntohl(spi), mark.value, mark.mask);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_id));

	sa_id = NLMSG_DATA(hdr);
	host2xfrm(dst, &sa_id->daddr);
	sa_id->spi = spi;
	sa_id->proto = protocol;
	sa_id->family = dst->get_family(dst);

	if (!add_mark(hdr, sizeof(request), mark))
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

					if (mark.value)
					{
						DBG1(DBG_KNL, "querying SAD entry with SPI %.8x  "
									  "(mark %u/0x%08x) failed: %s (%d)",
									   ntohl(spi), mark.value, mark.mask,
									   strerror(-err->error), -err->error);
					}
					else
					{
						DBG1(DBG_KNL, "querying SAD entry with SPI %.8x "
									  "failed: %s (%d)", ntohl(spi),
									   strerror(-err->error), -err->error);
					}
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
		DBG2(DBG_KNL, "unable to query SAD entry with SPI %.8x", ntohl(spi));
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
	private_kernel_netlink_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t spi, u_int8_t protocol, u_int16_t cpi, mark_t mark)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct xfrm_usersa_id *sa_id;

	/* if IPComp was used, we first delete the additional IPComp SA */
	if (cpi)
	{
		del_sa(this, src, dst, htonl(ntohs(cpi)), IPPROTO_COMP, 0, mark);
	}

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "deleting SAD entry with SPI %.8x  (mark %u/0x%08x)",
				   ntohl(spi), mark.value, mark.mask);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_DELSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_id));

	sa_id = NLMSG_DATA(hdr);
	host2xfrm(dst, &sa_id->daddr);
	sa_id->spi = spi;
	sa_id->proto = protocol;
	sa_id->family = dst->get_family(dst);

	if (!add_mark(hdr, sizeof(request), mark))
	{
		return FAILED;
	}

	switch (this->socket_xfrm->send_ack(this->socket_xfrm, hdr))
	{
		case SUCCESS:
			DBG2(DBG_KNL, "deleted SAD entry with SPI %.8x (mark %u/0x%08x)",
				 ntohl(spi), mark.value, mark.mask);
			return SUCCESS;
		case NOT_FOUND:
			return NOT_FOUND;
		default:
			if (mark.value)
			{
				DBG1(DBG_KNL, "unable to delete SAD entry with SPI %.8x "
					 "(mark %u/0x%08x)", ntohl(spi), mark.value, mark.mask);
			}
			else
			{
				DBG1(DBG_KNL, "unable to delete SAD entry with SPI %.8x",
					 ntohl(spi));
			}
			return FAILED;
	}
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_netlink_ipsec_t *this, u_int32_t spi, u_int8_t protocol,
	u_int16_t cpi, host_t *src, host_t *dst, host_t *new_src, host_t *new_dst,
	bool old_encap, bool new_encap, mark_t mark)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out = NULL;
	struct xfrm_usersa_id *sa_id;
	struct xfrm_usersa_info *out_sa = NULL, *sa;
	size_t len;
	struct rtattr *rta;
	size_t rtasize;
	struct xfrm_encap_tmpl* tmpl = NULL;
	struct xfrm_replay_state *replay = NULL;
	struct xfrm_replay_state_esn *replay_esn = NULL;
	u_int32_t replay_esn_len;
	status_t status = FAILED;

	/* if IPComp is used, we first update the IPComp SA */
	if (cpi)
	{
		update_sa(this, htonl(ntohs(cpi)), IPPROTO_COMP, 0,
				  src, dst, new_src, new_dst, FALSE, FALSE, mark);
	}

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "querying SAD entry with SPI %.8x for update", ntohl(spi));

	/* query the existing SA first */
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_id));

	sa_id = NLMSG_DATA(hdr);
	host2xfrm(dst, &sa_id->daddr);
	sa_id->spi = spi;
	sa_id->proto = protocol;
	sa_id->family = dst->get_family(dst);

	if (!add_mark(hdr, sizeof(request), mark))
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
					out_sa = NLMSG_DATA(hdr);
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
	if (out_sa == NULL)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x", ntohl(spi));
		goto failed;
	}

	get_replay_state(this, spi, protocol, dst, mark, &replay_esn, &replay_esn_len, &replay);

	/* delete the old SA (without affecting the IPComp SA) */
	if (del_sa(this, src, dst, spi, protocol, 0, mark) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to delete old SAD entry with SPI %.8x",
					   ntohl(spi));
		goto failed;
	}

	DBG2(DBG_KNL, "updating SAD entry with SPI %.8x from %#H..%#H to %#H..%#H",
				   ntohl(spi), src, dst, new_src, new_dst);
	/* copy over the SA from out to request */
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_NEWSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_info));
	sa = NLMSG_DATA(hdr);
	memcpy(sa, NLMSG_DATA(out), sizeof(struct xfrm_usersa_info));
	sa->family = new_dst->get_family(new_dst);

	if (!src->ip_equals(src, new_src))
	{
		host2xfrm(new_src, &sa->saddr);
	}
	if (!dst->ip_equals(dst, new_dst))
	{
		host2xfrm(new_dst, &sa->id.daddr);
	}

	rta = XFRM_RTA(out, struct xfrm_usersa_info);
	rtasize = XFRM_PAYLOAD(out, struct xfrm_usersa_info);
	while (RTA_OK(rta, rtasize))
	{
		/* copy all attributes, but not XFRMA_ENCAP if we are disabling it */
		if (rta->rta_type != XFRMA_ENCAP || new_encap)
		{
			if (rta->rta_type == XFRMA_ENCAP)
			{	/* update encap tmpl */
				tmpl = RTA_DATA(rta);
				tmpl->encap_sport = ntohs(new_src->get_port(new_src));
				tmpl->encap_dport = ntohs(new_dst->get_port(new_dst));
			}
			netlink_add_attribute(hdr, rta->rta_type,
								  chunk_create(RTA_DATA(rta), RTA_PAYLOAD(rta)),
								  sizeof(request));
		}
		rta = RTA_NEXT(rta, rtasize);
	}

	if (tmpl == NULL && new_encap)
	{	/* add tmpl if we are enabling it */
		tmpl = netlink_reserve(hdr, sizeof(request), XFRMA_ENCAP, sizeof(*tmpl));
		if (!tmpl)
		{
			goto failed;
		}
		tmpl->encap_type = UDP_ENCAP_ESPINUDP;
		tmpl->encap_sport = ntohs(new_src->get_port(new_src));
		tmpl->encap_dport = ntohs(new_dst->get_port(new_dst));
		memset(&tmpl->encap_oa, 0, sizeof (xfrm_address_t));
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
		DBG1(DBG_KNL, "unable to copy replay state from old SAD entry "
					  "with SPI %.8x", ntohl(spi));
	}

	if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x", ntohl(spi));
		goto failed;
	}

	status = SUCCESS;
failed:
	free(replay);
	free(replay_esn);
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

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "flushing all SAD entries");

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = XFRM_MSG_FLUSHSA;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_usersa_flush));

	flush = NLMSG_DATA(hdr);
	flush->proto = IPSEC_PROTO_ANY;

	if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to flush SAD entries");
		return FAILED;
	}
	return SUCCESS;
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

	if (mapping->type == POLICY_IPSEC)
	{
		struct xfrm_user_tmpl *tmpl;
		struct {
			u_int8_t proto;
			bool use;
		} protos[] = {
			{ IPPROTO_COMP, ipsec->cfg.ipcomp.transform != IPCOMP_NONE },
			{ IPPROTO_ESP, ipsec->cfg.esp.use },
			{ IPPROTO_AH, ipsec->cfg.ah.use },
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
			this->mutex->unlock(this->mutex);
			return FAILED;
		}

		for (i = 0; i < countof(protos); i++)
		{
			if (!protos[i].use)
			{
				continue;
			}
			tmpl->reqid = policy->reqid;
			tmpl->id.proto = protos[i].proto;
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
		this->mutex->unlock(this->mutex);
		return FAILED;
	}
	this->mutex->unlock(this->mutex);

	if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
	{
		return FAILED;
	}

	/* find the policy again */
	this->mutex->lock(this->mutex);
	policy = this->policies->get(this->policies, &clone);
	if (!policy ||
		 policy->used_by->find_first(policy->used_by,
									 NULL, (void**)&mapping) != SUCCESS)
	{	/* policy or mapping is already gone, ignore */
		this->mutex->unlock(this->mutex);
		return SUCCESS;
	}

	/* install a route, if:
	 * - this is a forward policy (to just get one for each child)
	 * - we are in tunnel/BEET mode or install a bypass policy
	 * - routing is not disabled via strongswan.conf
	 */
	if (policy->direction == POLICY_FWD && this->install_routes &&
		(mapping->type != POLICY_IPSEC || ipsec->cfg.mode != MODE_TRANSPORT))
	{
		policy_sa_fwd_t *fwd = (policy_sa_fwd_t*)mapping;
		route_entry_t *route;
		host_t *iface;

		INIT(route,
			.prefixlen = policy->sel.prefixlen_s,
		);

		if (hydra->kernel_interface->get_address_by_ts(hydra->kernel_interface,
				fwd->dst_ts, &route->src_ip, NULL) == SUCCESS)
		{
			/* get the nexthop to src (src as we are in POLICY_FWD) */
			if (!ipsec->src->is_anyaddr(ipsec->src))
			{
				route->gateway = hydra->kernel_interface->get_nexthop(
											hydra->kernel_interface, ipsec->src,
											-1, ipsec->dst);
			}
			else
			{	/* for shunt policies */
				iface = xfrm2host(policy->sel.family, &policy->sel.saddr, 0);
				route->gateway = hydra->kernel_interface->get_nexthop(
										hydra->kernel_interface, iface,
										policy->sel.prefixlen_s, route->src_ip);
				iface->destroy(iface);
			}
			route->dst_net = chunk_alloc(policy->sel.family == AF_INET ? 4 : 16);
			memcpy(route->dst_net.ptr, &policy->sel.saddr, route->dst_net.len);

			/* get the interface to install the route for. If we have a local
			 * address, use it. Otherwise (for shunt policies) use the
			 * routes source address. */
			iface = ipsec->dst;
			if (iface->is_anyaddr(iface))
			{
				iface = route->src_ip;
			}
			/* install route via outgoing interface */
			if (!hydra->kernel_interface->get_interface(hydra->kernel_interface,
														iface, &route->if_name))
			{
				this->mutex->unlock(this->mutex);
				route_entry_destroy(route);
				return SUCCESS;
			}

			if (policy->route)
			{
				route_entry_t *old = policy->route;
				if (route_entry_equals(old, route))
				{
					this->mutex->unlock(this->mutex);
					route_entry_destroy(route);
					return SUCCESS;
				}
				/* uninstall previously installed route */
				if (hydra->kernel_interface->del_route(hydra->kernel_interface,
						old->dst_net, old->prefixlen, old->gateway,
						old->src_ip, old->if_name) != SUCCESS)
				{
					DBG1(DBG_KNL, "error uninstalling route installed with "
								  "policy %R === %R %N", fwd->src_ts,
								   fwd->dst_ts, policy_dir_names,
								   policy->direction);
				}
				route_entry_destroy(old);
				policy->route = NULL;
			}

			DBG2(DBG_KNL, "installing route: %R via %H src %H dev %s",
				 fwd->src_ts, route->gateway, route->src_ip, route->if_name);
			switch (hydra->kernel_interface->add_route(
								hydra->kernel_interface, route->dst_net,
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
	this->mutex->unlock(this->mutex);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_netlink_ipsec_t *this, host_t *src, host_t *dst,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
	policy_dir_t direction, policy_type_t type, ipsec_sa_cfg_t *sa,
	mark_t mark, policy_priority_t priority)
{
	policy_entry_t *policy, *current;
	policy_sa_t *assigned_sa, *current_sa;
	enumerator_t *enumerator;
	bool found = FALSE, update = TRUE;

	/* create a policy */
	INIT(policy,
		.sel = ts2selector(src_ts, dst_ts),
		.mark = mark.value & mark.mask,
		.direction = direction,
		.reqid = sa->reqid,
	);

	/* find the policy, which matches EXACTLY */
	this->mutex->lock(this->mutex);
	current = this->policies->get(this->policies, policy);
	if (current)
	{
		if (current->reqid != sa->reqid)
		{
			DBG1(DBG_CFG, "unable to install policy %R === %R %N (mark "
				 "%u/0x%08x) for reqid %u, the same policy for reqid %u exists",
				 src_ts, dst_ts, policy_dir_names, direction,
				 mark.value, mark.mask, sa->reqid, current->reqid);
			policy_entry_destroy(this, policy);
			this->mutex->unlock(this->mutex);
			return INVALID_STATE;
		}
		/* use existing policy */
		DBG2(DBG_KNL, "policy %R === %R %N  (mark %u/0x%08x) "
					  "already exists, increasing refcount",
					   src_ts, dst_ts, policy_dir_names, direction,
					   mark.value, mark.mask);
		policy_entry_destroy(this, policy);
		policy = current;
		found = TRUE;
	}
	else
	{	/* use the new one, if we have no such policy */
		policy->used_by = linked_list_create();
		this->policies->put(this->policies, policy, policy);
	}

	/* cache the assigned IPsec SA */
	assigned_sa = policy_sa_create(this, direction, type, src, dst, src_ts,
								   dst_ts, mark, sa);
	assigned_sa->priority = get_priority(policy, priority);

	if (this->policy_history)
	{	/* insert the SA according to its priority */
		enumerator = policy->used_by->create_enumerator(policy->used_by);
		while (enumerator->enumerate(enumerator, (void**)&current_sa))
		{
			if (current_sa->priority >= assigned_sa->priority)
			{
				break;
			}
			update = FALSE;
		}
		policy->used_by->insert_before(policy->used_by, enumerator,
									   assigned_sa);
		enumerator->destroy(enumerator);
	}
	else
	{	/* simply insert it last and only update if it is not installed yet */
		policy->used_by->insert_last(policy->used_by, assigned_sa);
		update = !found;
	}

	if (!update)
	{	/* we don't update the policy if the priority is lower than that of
		 * the currently installed one */
		this->mutex->unlock(this->mutex);
		return SUCCESS;
	}

	DBG2(DBG_KNL, "%s policy %R === %R %N  (mark %u/0x%08x)",
				   found ? "updating" : "adding", src_ts, dst_ts,
				   policy_dir_names, direction, mark.value, mark.mask);

	if (add_policy_internal(this, policy, assigned_sa, found) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to %s policy %R === %R %N",
					   found ? "update" : "add", src_ts, dst_ts,
					   policy_dir_names, direction);
		return FAILED;
	}
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_netlink_ipsec_t *this, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_dir_t direction, mark_t mark,
	time_t *use_time)
{
	netlink_buf_t request;
	struct nlmsghdr *out = NULL, *hdr;
	struct xfrm_userpolicy_id *policy_id;
	struct xfrm_userpolicy_info *policy = NULL;
	size_t len;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "querying policy %R === %R %N  (mark %u/0x%08x)",
				   src_ts, dst_ts, policy_dir_names, direction,
				   mark.value, mark.mask);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdr->nlmsg_type = XFRM_MSG_GETPOLICY;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct xfrm_userpolicy_id));

	policy_id = NLMSG_DATA(hdr);
	policy_id->sel = ts2selector(src_ts, dst_ts);
	policy_id->dir = direction;

	if (!add_mark(hdr, sizeof(request), mark))
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
		DBG2(DBG_KNL, "unable to query policy %R === %R %N", src_ts, dst_ts,
					   policy_dir_names, direction);
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
	private_kernel_netlink_ipsec_t *this, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_dir_t direction, u_int32_t reqid,
	mark_t mark, policy_priority_t prio)
{
	policy_entry_t *current, policy;
	enumerator_t *enumerator;
	policy_sa_t *mapping;
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct xfrm_userpolicy_id *policy_id;
	bool is_installed = TRUE;
	u_int32_t priority;

	DBG2(DBG_KNL, "deleting policy %R === %R %N  (mark %u/0x%08x)",
				   src_ts, dst_ts, policy_dir_names, direction,
				   mark.value, mark.mask);

	/* create a policy */
	memset(&policy, 0, sizeof(policy_entry_t));
	policy.sel = ts2selector(src_ts, dst_ts);
	policy.mark = mark.value & mark.mask;
	policy.direction = direction;

	/* find the policy */
	this->mutex->lock(this->mutex);
	current = this->policies->get(this->policies, &policy);
	if (!current || current->reqid != reqid)
	{
		if (mark.value)
		{
			DBG1(DBG_KNL, "deleting policy %R === %R %N  (mark %u/0x%08x) "
						  "failed, not found", src_ts, dst_ts, policy_dir_names,
						   direction, mark.value, mark.mask);
		}
		else
		{
			DBG1(DBG_KNL, "deleting policy %R === %R %N failed, not found",
						   src_ts, dst_ts, policy_dir_names, direction);
		}
		this->mutex->unlock(this->mutex);
		return NOT_FOUND;
	}

	if (this->policy_history)
	{	/* remove mapping to SA by reqid and priority */
		priority = get_priority(current, prio);
		enumerator = current->used_by->create_enumerator(current->used_by);
		while (enumerator->enumerate(enumerator, (void**)&mapping))
		{
			if (priority == mapping->priority)
			{
				current->used_by->remove_at(current->used_by, enumerator);
				policy_sa_destroy(mapping, &direction, this);
				break;
			}
			is_installed = FALSE;
		}
		enumerator->destroy(enumerator);
	}
	else
	{	/* remove one of the SAs but don't update the policy */
		current->used_by->remove_last(current->used_by, (void**)&mapping);
		policy_sa_destroy(mapping, &direction, this);
		is_installed = FALSE;
	}

	if (current->used_by->get_count(current->used_by) > 0)
	{	/* policy is used by more SAs, keep in kernel */
		DBG2(DBG_KNL, "policy still used by another CHILD_SA, not removed");
		if (!is_installed)
		{	/* no need to update as the policy was not installed for this SA */
			this->mutex->unlock(this->mutex);
			return SUCCESS;
		}

		DBG2(DBG_KNL, "updating policy %R === %R %N  (mark %u/0x%08x)",
					   src_ts, dst_ts, policy_dir_names, direction,
					   mark.value, mark.mask);

		current->used_by->get_first(current->used_by, (void**)&mapping);
		if (add_policy_internal(this, current, mapping, TRUE) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to update policy %R === %R %N",
						   src_ts, dst_ts, policy_dir_names, direction);
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
	policy_id->dir = direction;

	if (!add_mark(hdr, sizeof(request), mark))
	{
		return FAILED;
	}

	if (current->route)
	{
		route_entry_t *route = current->route;
		if (hydra->kernel_interface->del_route(hydra->kernel_interface,
				route->dst_net, route->prefixlen, route->gateway,
				route->src_ip, route->if_name) != SUCCESS)
		{
			DBG1(DBG_KNL, "error uninstalling route installed with "
						  "policy %R === %R %N", src_ts, dst_ts,
						   policy_dir_names, direction);
		}
	}

	this->policies->remove(this->policies, current);
	policy_entry_destroy(this, current);
	this->mutex->unlock(this->mutex);

	if (this->socket_xfrm->send_ack(this->socket_xfrm, hdr) != SUCCESS)
	{
		if (mark.value)
		{
			DBG1(DBG_KNL, "unable to delete policy %R === %R %N  "
						  "(mark %u/0x%08x)", src_ts, dst_ts, policy_dir_names,
						   direction, mark.value, mark.mask);
		}
		else
		{
			DBG1(DBG_KNL, "unable to delete policy %R === %R %N",
						   src_ts, dst_ts, policy_dir_names, direction);
		}
		return FAILED;
	}
	return SUCCESS;
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


METHOD(kernel_ipsec_t, bypass_socket, bool,
	private_kernel_netlink_ipsec_t *this, int fd, int family)
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
		DBG1(DBG_KNL, "unable to set IPSEC_POLICY on socket: %s",
					   strerror(errno));
		return FALSE;
	}
	policy.dir = XFRM_POLICY_IN;
	if (setsockopt(fd, sol, ipsec_policy, &policy, sizeof(policy)) < 0)
	{
		DBG1(DBG_KNL, "unable to set IPSEC_POLICY on socket: %s",
					   strerror(errno));
		return FALSE;
	}
	return TRUE;
}

METHOD(kernel_ipsec_t, enable_udp_decap, bool,
	private_kernel_netlink_ipsec_t *this, int fd, int family, u_int16_t port)
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
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * Described in header.
 */
kernel_netlink_ipsec_t *kernel_netlink_ipsec_create()
{
	private_kernel_netlink_ipsec_t *this;
	bool register_for_events = TRUE;
	FILE *f;

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
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.policy_history = TRUE,
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

	f = fopen("/proc/sys/net/core/xfrm_acq_expires", "w");
	if (f)
	{
		fprintf(f, "%u", lib->settings->get_int(lib->settings,
								"%s.plugins.kernel-netlink.xfrm_acq_expires",
								DEFAULT_ACQUIRE_LIFETIME, lib->ns));
		fclose(f);
	}

	this->socket_xfrm = netlink_socket_create(NETLINK_XFRM, xfrm_msg_names);
	if (!this->socket_xfrm)
	{
		destroy(this);
		return NULL;
	}

	if (register_for_events)
	{
		struct sockaddr_nl addr;

		memset(&addr, 0, sizeof(addr));
		addr.nl_family = AF_NETLINK;

		/* create and bind XFRM socket for ACQUIRE, EXPIRE, MIGRATE & MAPPING */
		this->socket_xfrm_events = socket(AF_NETLINK, SOCK_RAW, NETLINK_XFRM);
		if (this->socket_xfrm_events <= 0)
		{
			DBG1(DBG_KNL, "unable to create XFRM event socket");
			destroy(this);
			return NULL;
		}
		addr.nl_groups = XFRMNLGRP(ACQUIRE) | XFRMNLGRP(EXPIRE) |
						 XFRMNLGRP(MIGRATE) | XFRMNLGRP(MAPPING);
		if (bind(this->socket_xfrm_events, (struct sockaddr*)&addr, sizeof(addr)))
		{
			DBG1(DBG_KNL, "unable to bind XFRM event socket");
			destroy(this);
			return NULL;
		}
		lib->watcher->add(lib->watcher, this->socket_xfrm_events, WATCHER_READ,
						  (watcher_cb_t)receive_events, this);
	}

	return &this->public;
}
