/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

/* Windows 7, for some fwpmu.h functionality */
#define _WIN32_WINNT 0x0601

#include "kernel_wfp_compat.h"
#include "kernel_wfp_ipsec.h"

#include <daemon.h>
#include <hydra.h>
#include <threading/mutex.h>
#include <collections/array.h>
#include <collections/hashtable.h>
#include <processing/jobs/callback_job.h>


typedef struct private_kernel_wfp_ipsec_t private_kernel_wfp_ipsec_t;

struct private_kernel_wfp_ipsec_t {

	/**
	 * Public interface
	 */
	kernel_wfp_ipsec_t public;

	/**
	 * Next SPI to allocate
	 */
	refcount_t nextspi;

	/**
	 * Mix value to distribute SPI allocation randomly
	 */
	u_int32_t mixspi;

	/**
	 * IKE bypass filters, as UINT64 filter LUID
	 */
	array_t *bypass;

	/**
	 * Temporary SAD/SPD entries referenced reqid, as uintptr_t => entry_t
	 */
	hashtable_t *tsas;

	/**
	 * SAD/SPD entries referenced by inbound SA, as sa_entry_t => entry_t
	 */
	hashtable_t *isas;

	/**
	 * SAD/SPD entries referenced by outbound SA, as sa_entry_t => entry_t
	 */
	hashtable_t *osas;

	/**
	 * Installed routes, as route_t => route_t
	 */
	hashtable_t *routes;

	/**
	 * Installed traps, as trap_t => trap_t
	 */
	hashtable_t *traps;

	/**
	 * Mutex for accessing entries
	 */
	mutex_t *mutex;

	/**
	 * WFP session handle
	 */
	HANDLE handle;

	/**
	 * Provider charon registers as
	 */
	FWPM_PROVIDER0 provider;

	/**
	 * Event handle
	 */
	HANDLE event;
};

/**
 * Security association entry
 */
typedef struct {
	/** SPI for this SA */
	u_int32_t spi;
	/** protocol, IPPROTO_ESP/IPPROTO_AH */
	u_int8_t protocol;
	/** hard lifetime of SA */
	u_int32_t lifetime;
	/** destination host address for this SPI */
	host_t *dst;
	struct {
		/** algorithm */
		u_int16_t alg;
		/** key */
		chunk_t key;
	} integ, encr;
} sa_entry_t;

/**
 * Hash function for sas lookup table
 */
static u_int hash_sa(sa_entry_t *key)
{
	return chunk_hash_inc(chunk_from_thing(key->spi),
						  chunk_hash(key->dst->get_address(key->dst)));
}

/**
 * equals function for sas lookup table
 */
static bool equals_sa(sa_entry_t *a, sa_entry_t *b)
{
	return a->spi == b->spi && a->dst->ip_equals(a->dst, b->dst);
}

/**
 * Security policy entry
 */
typedef struct {
	/** policy source addresses */
	traffic_selector_t *src;
	/** policy destinaiton addresses */
	traffic_selector_t *dst;
	/** WFP allocated LUID for inbound filter ID */
	u_int64_t policy_in;
	/** WFP allocated LUID for outbound filter ID */
	u_int64_t policy_out;
	/** WFP allocated LUID for forward inbound filter ID, tunnel mode only */
	u_int64_t policy_fwd_in;
	/** WFP allocated LUID for forward outbound filter ID, tunnel mode only */
	u_int64_t policy_fwd_out;
	/** have installed a route for it? */
	bool route;
} sp_entry_t;

/**
 * Destroy an SP entry
 */
static void sp_entry_destroy(sp_entry_t *sp)
{
	sp->src->destroy(sp->src);
	sp->dst->destroy(sp->dst);
	free(sp);
}

/**
 * Collection of SA/SP database entries for a reqid
 */
typedef struct {
	/** reqid of entry */
	u_int32_t reqid;
	/** outer address on local host */
	host_t *local;
	/** outer address on remote host */
	host_t *remote;
	/** inbound SA entry */
	sa_entry_t isa;
	/** outbound SA entry */
	sa_entry_t osa;
	/** associated (outbound) policies, as sp_entry_t* */
	array_t *sps;
	/** IPsec mode, tunnel|transport */
	ipsec_mode_t mode;
	/** UDP encapsulation */
	bool encap;
	/** provider context, for tunnel mode only */
	u_int64_t provider;
	/** WFP allocated LUID for SA context */
	u_int64_t sa_id;
} entry_t;

/**
 * Installed route
 */
typedef struct {
	/** destination net of route */
	host_t *dst;
	/** prefix length of dst */
	u_int8_t mask;
	/** source address for route */
	host_t *src;
	/** gateway of route, NULL if directly attached */
	host_t *gtw;
	/** references for route */
	u_int refs;
} route_t;

/**
 * Destroy a route_t
 */
static void destroy_route(route_t *this)
{
	this->dst->destroy(this->dst);
	this->src->destroy(this->src);
	DESTROY_IF(this->gtw);
	free(this);
}

/**
 * Hashtable equals function for routes
 */
static bool equals_route(route_t *a, route_t *b)
{
	return a->mask == b->mask &&
		   a->dst->ip_equals(a->dst, b->dst) &&
		   a->src->ip_equals(a->src, b->src);
}

/**
 * Hashtable hash function for routes
 */
static u_int hash_route(route_t *route)
{
	return chunk_hash_inc(route->src->get_address(route->src),
			chunk_hash_inc(route->dst->get_address(route->dst), route->mask));
}

/** forward declaration */
static bool manage_routes(private_kernel_wfp_ipsec_t *this, entry_t *entry,
						  bool add);

/**
 * Remove policies associated to an entry from kernel
 */
static void cleanup_policies(private_kernel_wfp_ipsec_t *this, entry_t *entry)
{
	enumerator_t *enumerator;
	sp_entry_t *sp;

	if (entry->mode == MODE_TUNNEL)
	{
		manage_routes(this, entry, FALSE);
	}

	enumerator = array_create_enumerator(entry->sps);
	while (enumerator->enumerate(enumerator, &sp))
	{
		if (sp->policy_in)
		{
			FwpmFilterDeleteById0(this->handle, sp->policy_in);
			sp->policy_in = 0;
		}
		if (sp->policy_out)
		{
			FwpmFilterDeleteById0(this->handle, sp->policy_out);
			sp->policy_out = 0;
		}
		if (sp->policy_fwd_in)
		{
			FwpmFilterDeleteById0(this->handle, sp->policy_fwd_in);
			sp->policy_fwd_in = 0;
		}
		if (sp->policy_fwd_out)
		{
			FwpmFilterDeleteById0(this->handle, sp->policy_fwd_out);
			sp->policy_fwd_out = 0;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Destroy a SA/SP entry set
 */
static void entry_destroy(private_kernel_wfp_ipsec_t *this, entry_t *entry)
{
	if (entry->sa_id)
	{
		IPsecSaContextDeleteById0(this->handle, entry->sa_id);
	}
	if (entry->provider)
	{
		FwpmProviderContextDeleteById0(this->handle, entry->provider);
	}
	cleanup_policies(this, entry);
	array_destroy_function(entry->sps, (void*)sp_entry_destroy, NULL);
	entry->local->destroy(entry->local);
	entry->remote->destroy(entry->remote);
	chunk_clear(&entry->isa.integ.key);
	chunk_clear(&entry->isa.encr.key);
	chunk_clear(&entry->osa.integ.key);
	chunk_clear(&entry->osa.encr.key);
	free(entry);
}

/**
 * Append/Realloc a filter condition to an existing condition set
 */
static FWPM_FILTER_CONDITION0 *append_condition(FWPM_FILTER_CONDITION0 *conds[],
												int *count)
{
	FWPM_FILTER_CONDITION0 *cond;

	(*count)++;
	*conds = realloc(*conds, *count * sizeof(*cond));
	cond = *conds + *count - 1;
	memset(cond, 0, sizeof(*cond));

	return cond;
}

/**
 * Convert an IPv4 prefix to a host order subnet mask
 */
static u_int32_t prefix2mask(u_int8_t prefix)
{
	u_int8_t netmask[4] = {};
	int i;

	for (i = 0; i < sizeof(netmask); i++)
	{
		if (prefix < 8)
		{
			netmask[i] = 0xFF << (8 - prefix);
			break;
		}
		netmask[i] = 0xFF;
		prefix -= 8;
	}
	return untoh32(netmask);
}

/**
 * Convert a 16-bit range to a WFP condition
 */
static void range2cond(FWPM_FILTER_CONDITION0 *cond,
					   u_int16_t from, u_int16_t to)
{
	if (from == to)
	{
		cond->matchType = FWP_MATCH_EQUAL;
		cond->conditionValue.type = FWP_UINT16;
		cond->conditionValue.uint16 = from;
	}
	else
	{
		cond->matchType = FWP_MATCH_RANGE;
		cond->conditionValue.type = FWP_RANGE_TYPE;
		cond->conditionValue.rangeValue = calloc(1, sizeof(FWP_RANGE0));
		cond->conditionValue.rangeValue->valueLow.type = FWP_UINT16;
		cond->conditionValue.rangeValue->valueLow.uint16 = from;
		cond->conditionValue.rangeValue->valueHigh.type = FWP_UINT16;
		cond->conditionValue.rangeValue->valueHigh.uint16 = to;
	}
}

/**
 * (Re-)allocate filter conditions for given local or remote traffic selector
 */
static bool ts2condition(traffic_selector_t *ts, const GUID *target,
						 FWPM_FILTER_CONDITION0 *conds[], int *count)
{
	FWPM_FILTER_CONDITION0 *cond;
	FWP_BYTE_ARRAY16 *addr;
	FWP_RANGE0 *range;
	u_int16_t from_port, to_port;
	void *from, *to;
	u_int8_t proto;
	host_t *net;
	u_int8_t prefix;

	from = ts->get_from_address(ts).ptr;
	to = ts->get_to_address(ts).ptr;
	from_port = ts->get_from_port(ts);
	to_port = ts->get_to_port(ts);

	cond = append_condition(conds, count);
	cond->fieldKey = *target;
	if (ts->is_host(ts, NULL))
	{
		cond->matchType = FWP_MATCH_EQUAL;
		switch (ts->get_type(ts))
		{
			case TS_IPV4_ADDR_RANGE:
				cond->conditionValue.type = FWP_UINT32;
				cond->conditionValue.uint32 = untoh32(from);
				break;
			case TS_IPV6_ADDR_RANGE:
				cond->conditionValue.type = FWP_BYTE_ARRAY16_TYPE;
				cond->conditionValue.byteArray16 = addr = malloc(sizeof(*addr));
				memcpy(addr, from, sizeof(*addr));
				break;
			default:
				return FALSE;
		}
	}
	else if (ts->to_subnet(ts, &net, &prefix))
	{
		FWP_V6_ADDR_AND_MASK *m6;
		FWP_V4_ADDR_AND_MASK *m4;

		cond->matchType = FWP_MATCH_EQUAL;
		switch (net->get_family(net))
		{
			case AF_INET:
				cond->conditionValue.type = FWP_V4_ADDR_MASK;
				cond->conditionValue.v4AddrMask = m4 = calloc(1, sizeof(*m4));
				m4->addr = untoh32(from);
				m4->mask = prefix2mask(prefix);
				break;
			case AF_INET6:
				cond->conditionValue.type = FWP_V6_ADDR_MASK;
				cond->conditionValue.v6AddrMask = m6 = calloc(1, sizeof(*m6));
				memcpy(m6->addr, from, sizeof(m6->addr));
				m6->prefixLength = prefix;
				break;
			default:
				net->destroy(net);
				return FALSE;
		}
		net->destroy(net);
	}
	else
	{
		cond->matchType = FWP_MATCH_RANGE;
		cond->conditionValue.type = FWP_RANGE_TYPE;
		cond->conditionValue.rangeValue = range = calloc(1, sizeof(*range));
		switch (ts->get_type(ts))
		{
			case TS_IPV4_ADDR_RANGE:
				range->valueLow.type = FWP_UINT32;
				range->valueLow.uint32 = untoh32(from);
				range->valueHigh.type = FWP_UINT32;
				range->valueHigh.uint32 = untoh32(to);
				break;
			case TS_IPV6_ADDR_RANGE:
				range->valueLow.type = FWP_BYTE_ARRAY16_TYPE;
				range->valueLow.byteArray16 = addr = malloc(sizeof(*addr));
				memcpy(addr, from, sizeof(*addr));
				range->valueHigh.type = FWP_BYTE_ARRAY16_TYPE;
				range->valueHigh.byteArray16 = addr = malloc(sizeof(*addr));
				memcpy(addr, to, sizeof(*addr));
				break;
			default:
				return FALSE;
		}
	}

	proto = ts->get_protocol(ts);
	if (proto && target == &FWPM_CONDITION_IP_LOCAL_ADDRESS)
	{
		cond = append_condition(conds, count);
		cond->fieldKey = FWPM_CONDITION_IP_PROTOCOL;
		cond->matchType = FWP_MATCH_EQUAL;
		cond->conditionValue.type = FWP_UINT8;
		cond->conditionValue.uint8 = proto;
	}

	if (proto == IPPROTO_ICMP)
	{
		if (target == &FWPM_CONDITION_IP_LOCAL_ADDRESS)
		{
			u_int8_t from_type, to_type, from_code, to_code;

			from_type = traffic_selector_icmp_type(from_port);
			to_type = traffic_selector_icmp_type(to_port);
			from_code = traffic_selector_icmp_code(from_port);
			to_code = traffic_selector_icmp_code(to_port);

			if (from_type != 0 || to_type != 0xFF)
			{
				cond = append_condition(conds, count);
				cond->fieldKey = FWPM_CONDITION_ICMP_TYPE;
				range2cond(cond, from_type, to_type);
			}
			if (from_code != 0 || to_code != 0xFF)
			{
				cond = append_condition(conds, count);
				cond->fieldKey = FWPM_CONDITION_ICMP_CODE;
				range2cond(cond, from_code, to_code);
			}
		}
	}
	else if (from_port != 0 || to_port != 0xFFFF)
	{
		if (target == &FWPM_CONDITION_IP_LOCAL_ADDRESS)
		{
			cond = append_condition(conds, count);
			cond->fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
			range2cond(cond, from_port, to_port);
		}
		if (target == &FWPM_CONDITION_IP_REMOTE_ADDRESS)
		{
			cond = append_condition(conds, count);
			cond->fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
			range2cond(cond, from_port, to_port);
		}
	}
	return TRUE;
}

/**
 * Free memory associated to a single condition
 */
static void free_condition(FWP_DATA_TYPE type, void *value)
{
	FWP_RANGE0 *range;

	switch (type)
	{
		case FWP_BYTE_ARRAY16_TYPE:
		case FWP_V4_ADDR_MASK:
		case FWP_V6_ADDR_MASK:
			free(value);
			break;
		case FWP_RANGE_TYPE:
			range = value;
			free_condition(range->valueLow.type, range->valueLow.sd);
			free_condition(range->valueHigh.type, range->valueHigh.sd);
			free(range);
			break;
		default:
			break;
	}
}

/**
 * Free memory used by a set of conditions
 */
static void free_conditions(FWPM_FILTER_CONDITION0 *conds, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		free_condition(conds[i].conditionValue.type, conds[i].conditionValue.sd);
	}
	free(conds);
}

/**
 * Find the callout GUID for given parameters
 */
static bool find_callout(bool tunnel, bool v6, bool inbound, bool forward,
						 GUID *layer, GUID *sublayer, GUID *callout)
{
	struct {
		bool tunnel;
		bool v6;
		bool inbound;
		bool forward;
		const GUID *layer;
		const GUID *sublayer;
		const GUID *callout;
	} map[] = {
		{ 0, 0, 0, 0, 	&FWPM_LAYER_OUTBOUND_TRANSPORT_V4, NULL,
						&FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V4			},
		{ 0, 0, 1, 0,	&FWPM_LAYER_INBOUND_TRANSPORT_V4, NULL,
						&FWPM_CALLOUT_IPSEC_INBOUND_TRANSPORT_V4			},
		{ 0, 1, 0, 0,	&FWPM_LAYER_OUTBOUND_TRANSPORT_V6, NULL,
						&FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V6			},
		{ 0, 1, 1, 0,	&FWPM_LAYER_INBOUND_TRANSPORT_V6, NULL,
						&FWPM_CALLOUT_IPSEC_INBOUND_TRANSPORT_V6			},
		{ 1, 0, 0, 0,	&FWPM_LAYER_OUTBOUND_TRANSPORT_V4,
						&FWPM_SUBLAYER_IPSEC_TUNNEL,
						&FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V4				},
		{ 1, 0, 0, 1,	&FWPM_LAYER_IPFORWARD_V4,
						&FWPM_SUBLAYER_IPSEC_FORWARD_OUTBOUND_TUNNEL,
						&FWPM_CALLOUT_IPSEC_FORWARD_OUTBOUND_TUNNEL_V4		},
		{ 1, 0, 1, 0,	&FWPM_LAYER_INBOUND_TRANSPORT_V4,
						&FWPM_SUBLAYER_IPSEC_TUNNEL,
						&FWPM_CALLOUT_IPSEC_INBOUND_TUNNEL_V4				},
		{ 1, 0, 1, 1,	&FWPM_LAYER_IPFORWARD_V4,
						&FWPM_SUBLAYER_IPSEC_TUNNEL,
						&FWPM_CALLOUT_IPSEC_FORWARD_INBOUND_TUNNEL_V4		},
		{ 1, 1, 0, 0,	&FWPM_LAYER_OUTBOUND_TRANSPORT_V6,
						&FWPM_SUBLAYER_IPSEC_TUNNEL,
						&FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V6				},
		{ 1, 1, 0, 1,	&FWPM_LAYER_IPFORWARD_V6,
						&FWPM_SUBLAYER_IPSEC_TUNNEL,
						&FWPM_CALLOUT_IPSEC_FORWARD_OUTBOUND_TUNNEL_V6		},
		{ 1, 1, 1, 0,	&FWPM_LAYER_INBOUND_TRANSPORT_V6,
						&FWPM_SUBLAYER_IPSEC_TUNNEL,
						&FWPM_CALLOUT_IPSEC_INBOUND_TUNNEL_V6				},
		{ 1, 1, 1, 1,	&FWPM_LAYER_IPFORWARD_V6,
						&FWPM_SUBLAYER_IPSEC_TUNNEL,
						&FWPM_CALLOUT_IPSEC_FORWARD_INBOUND_TUNNEL_V6		},
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (tunnel == map[i].tunnel &&
			v6 == map[i].v6 &&
			inbound == map[i].inbound &&
			forward == map[i].forward)
		{
			*callout = *map[i].callout;
			*layer = *map[i].layer;
			if (map[i].sublayer)
			{
				*sublayer = *map[i].sublayer;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Install a single policy in to the kernel
 */
static bool install_sp(private_kernel_wfp_ipsec_t *this, sp_entry_t *sp,
					   GUID *context, bool inbound, bool fwd, UINT64 *filter_id)
{
	FWPM_FILTER_CONDITION0 *conds = NULL;
	traffic_selector_t *local, *remote;
	const GUID *ltarget, *rtarget;
	int count = 0;
	bool v6;
	DWORD res;
	FWPM_FILTER0 filter = {
		.displayData = {
			.name = L"charon IPsec policy",
		},
		.action = {
			.type = FWP_ACTION_CALLOUT_TERMINATING,
		},
	};

	if (context)
	{
		filter.flags |= FWPM_FILTER_FLAG_HAS_PROVIDER_CONTEXT;
		filter.providerKey = (GUID*)&this->provider.providerKey;
		filter.providerContextKey = *context;
	}

	v6 = sp->src->get_type(sp->src) == TS_IPV6_ADDR_RANGE;
	if (!find_callout(context != NULL, v6, inbound, fwd,
					  &filter.layerKey, &filter.subLayerKey,
					  &filter.action.calloutKey))
	{
		return FALSE;
	}

	if (inbound && fwd)
	{
		local = sp->dst;
		remote = sp->src;
	}
	else
	{
		local = sp->src;
		remote = sp->dst;
	}
	if (fwd)
	{
		ltarget = &FWPM_CONDITION_IP_SOURCE_ADDRESS;
		rtarget = &FWPM_CONDITION_IP_DESTINATION_ADDRESS;
	}
	else
	{
		ltarget = &FWPM_CONDITION_IP_LOCAL_ADDRESS;
		rtarget = &FWPM_CONDITION_IP_REMOTE_ADDRESS;
	}
	if (!ts2condition(local, ltarget, &conds, &count) ||
		!ts2condition(remote, rtarget, &conds, &count))
	{
		free_conditions(conds, count);
		return FALSE;
	}

	filter.numFilterConditions = count;
	filter.filterCondition = conds;

	res = FwpmFilterAdd0(this->handle, &filter, NULL, filter_id);
	free_conditions(conds, count);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "installing %s%sbound WFP filter failed: 0x%08x",
			 fwd ? "forward " : "", inbound ? "in" : "out", res);
		return FALSE;
	}
	return TRUE;
}

/**
 * Install a set of policies in to the kernel
 */
static bool install_sps(private_kernel_wfp_ipsec_t *this,
						entry_t *entry, GUID *context)
{
	enumerator_t *enumerator;
	sp_entry_t *sp;

	enumerator = array_create_enumerator(entry->sps);
	while (enumerator->enumerate(enumerator, &sp))
	{
		/* inbound policy */
		if (!install_sp(this, sp, context, TRUE, FALSE, &sp->policy_in))
		{
			enumerator->destroy(enumerator);
			return FALSE;
		}
		/* outbound policy */
		if (!install_sp(this, sp, context, FALSE, FALSE, &sp->policy_out))
		{
			enumerator->destroy(enumerator);
			return FALSE;
		}
		if (context)
		{
			if (!sp->src->is_host(sp->src, entry->local) ||
				!sp->dst->is_host(sp->dst, entry->remote))
			{
				/* inbound forward policy, from decapsulation */
				if (!install_sp(this, sp, context,
								TRUE, TRUE, &sp->policy_fwd_in))
				{
					enumerator->destroy(enumerator);
					return FALSE;
				}
				/* outbound forward policy, to encapsulate */
				if (!install_sp(this, sp, context,
								FALSE, TRUE, &sp->policy_fwd_out))
				{
					enumerator->destroy(enumerator);
					return FALSE;
				}
			}
		}
	}
	enumerator->destroy(enumerator);

	return TRUE;
}

/**
 * Convert a chunk_t to a WFP FWP_BYTE_BLOB
 */
static inline FWP_BYTE_BLOB chunk2blob(chunk_t chunk)
{
	return (FWP_BYTE_BLOB){
		.size = chunk.len,
		.data = chunk.ptr,
	};
}

/**
 * Convert an integrity_algorithm_t to a WFP IPSEC_AUTH_TRANFORM_ID0
 */
static bool alg2auth(integrity_algorithm_t alg,
					 IPSEC_SA_AUTH_INFORMATION0 *info)
{
	struct {
		integrity_algorithm_t alg;
		IPSEC_AUTH_TRANSFORM_ID0 transform;
	} map[] = {
		{ AUTH_HMAC_MD5_96,			IPSEC_AUTH_TRANSFORM_ID_HMAC_MD5_96		},
		{ AUTH_HMAC_SHA1_96,		IPSEC_AUTH_TRANSFORM_ID_HMAC_SHA_1_96	},
		{ AUTH_HMAC_SHA2_256_128,	IPSEC_AUTH_TRANSFORM_ID_HMAC_SHA_256_128},
		{ AUTH_AES_128_GMAC,		IPSEC_AUTH_TRANSFORM_ID_GCM_AES_128		},
		{ AUTH_AES_192_GMAC,		IPSEC_AUTH_TRANSFORM_ID_GCM_AES_192		},
		{ AUTH_AES_256_GMAC,		IPSEC_AUTH_TRANSFORM_ID_GCM_AES_256		},
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (map[i].alg == alg)
		{
			info->authTransform.authTransformId = map[i].transform;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Convert an encryption_algorithm_t to a WFP IPSEC_CIPHER_TRANFORM_ID0
 */
static bool alg2cipher(encryption_algorithm_t alg, int keylen,
					   IPSEC_SA_CIPHER_INFORMATION0 *info)
{
	struct {
		encryption_algorithm_t alg;
		int keylen;
		IPSEC_CIPHER_TRANSFORM_ID0 transform;
	} map[] = {
		{ ENCR_DES,				 8, IPSEC_CIPHER_TRANSFORM_ID_CBC_DES		},
		{ ENCR_3DES,			24, IPSEC_CIPHER_TRANSFORM_ID_CBC_3DES		},
		{ ENCR_AES_CBC,			16, IPSEC_CIPHER_TRANSFORM_ID_AES_128		},
		{ ENCR_AES_CBC,			24, IPSEC_CIPHER_TRANSFORM_ID_AES_192		},
		{ ENCR_AES_CBC,			32, IPSEC_CIPHER_TRANSFORM_ID_AES_256		},
		{ ENCR_AES_GCM_ICV16,	20, IPSEC_CIPHER_TRANSFORM_ID_GCM_AES_128	},
		{ ENCR_AES_GCM_ICV16,	28, IPSEC_CIPHER_TRANSFORM_ID_GCM_AES_192	},
		{ ENCR_AES_GCM_ICV16,	36, IPSEC_CIPHER_TRANSFORM_ID_GCM_AES_256	},
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (map[i].alg == alg && map[i].keylen == keylen)
		{
			info->cipherTransform.cipherTransformId = map[i].transform;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Get the integrity algorithm used for an AEAD transform
 */
static integrity_algorithm_t encr2integ(encryption_algorithm_t encr, int keylen)
{
	struct {
		encryption_algorithm_t encr;
		int keylen;
		integrity_algorithm_t integ;
	} map[] = {
		{ ENCR_NULL_AUTH_AES_GMAC,		20, AUTH_AES_128_GMAC				},
		{ ENCR_NULL_AUTH_AES_GMAC,		28, AUTH_AES_192_GMAC				},
		{ ENCR_NULL_AUTH_AES_GMAC,		36, AUTH_AES_256_GMAC				},
		{ ENCR_AES_GCM_ICV16,			20, AUTH_AES_128_GMAC				},
		{ ENCR_AES_GCM_ICV16,			28, AUTH_AES_192_GMAC				},
		{ ENCR_AES_GCM_ICV16,			36, AUTH_AES_256_GMAC				},
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (map[i].encr == encr && map[i].keylen == keylen)
		{
			return map[i].integ;
		}
	}
	return AUTH_UNDEFINED;
}

/**
 * Install a single SA
 */
static bool install_sa(private_kernel_wfp_ipsec_t *this, entry_t *entry,
					   bool inbound, sa_entry_t *sa, FWP_IP_VERSION version)
{
	IPSEC_SA_AUTH_AND_CIPHER_INFORMATION0 info = {};
	IPSEC_SA0 ipsec = {
		.spi = ntohl(sa->spi),
	};
	IPSEC_SA_BUNDLE0 bundle = {
		.lifetime = {
			.lifetimeSeconds = inbound ? entry->isa.lifetime
									   : entry->osa.lifetime,
		},
		.saList = &ipsec,
		.numSAs = 1,
		.ipVersion = version,
	};
	struct {
		u_int16_t alg;
		chunk_t key;
	} integ = {}, encr = {};
	DWORD res;

	switch (sa->protocol)
	{
		case IPPROTO_AH:
			ipsec.saTransformType = IPSEC_TRANSFORM_AH;
			ipsec.ahInformation = &info.saAuthInformation;
			integ.key = sa->integ.key;
			integ.alg = sa->integ.alg;
			break;
		case IPPROTO_ESP:
			if (sa->encr.alg == ENCR_NULL ||
				sa->encr.alg == ENCR_NULL_AUTH_AES_GMAC)
			{
				ipsec.saTransformType = IPSEC_TRANSFORM_ESP_AUTH;
				ipsec.espAuthInformation = &info.saAuthInformation;
			}
			else
			{
				ipsec.saTransformType = IPSEC_TRANSFORM_ESP_AUTH_AND_CIPHER;
				ipsec.espAuthAndCipherInformation = &info;
				encr.key = sa->encr.key;
				encr.alg = sa->encr.alg;
			}
			if (encryption_algorithm_is_aead(sa->encr.alg))
			{
				integ.alg = encr2integ(sa->encr.alg, sa->encr.key.len);
				integ.key = sa->encr.key;
			}
			else
			{
				integ.alg = sa->integ.alg;
				integ.key = sa->integ.key;
			}
			break;
		default:
			return FALSE;
	}

	if (integ.alg)
	{
		info.saAuthInformation.authKey = chunk2blob(integ.key);
		if (!alg2auth(integ.alg, &info.saAuthInformation))
		{
			DBG1(DBG_KNL, "integrity algorithm %N not supported by WFP",
				 integrity_algorithm_names, integ.alg);
			return FALSE;
		}
	}
	if (encr.alg)
	{
		info.saCipherInformation.cipherKey = chunk2blob(encr.key);
		if (!alg2cipher(encr.alg, encr.key.len, &info.saCipherInformation))
		{
			DBG1(DBG_KNL, "encryption algorithm %N not supported by WFP",
				 encryption_algorithm_names, encr.alg);
			return FALSE;
		}
	}

	if (inbound)
	{
		res = IPsecSaContextAddInbound0(this->handle, entry->sa_id, &bundle);
	}
	else
	{
		bundle.flags |= IPSEC_SA_BUNDLE_FLAG_ASSUME_UDP_CONTEXT_OUTBOUND;
		res = IPsecSaContextAddOutbound0(this->handle, entry->sa_id, &bundle);
	}
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "adding %sbound WFP SA failed: 0x%08x",
			 inbound ? "in" : "out", res);
		return FALSE;
	}
	return TRUE;
}

/**
 * Convert an IPv6 host address to WFP representation
 */
static void host2address6(host_t *host, void *out)
{
	u_int32_t *src, *dst = out;

	src = (u_int32_t*)host->get_address(host).ptr;

	dst[0] = untoh32(&src[3]);
	dst[1] = untoh32(&src[2]);
	dst[2] = untoh32(&src[1]);
	dst[3] = untoh32(&src[0]);
}

/**
 * Fill in traffic structure from entry addresses
 */
static bool hosts2traffic(private_kernel_wfp_ipsec_t *this,
						  host_t *l, host_t *r, IPSEC_TRAFFIC1 *traffic)
{
	if (l->get_family(l) != r->get_family(r))
	{
		return FALSE;
	}
	switch (l->get_family(l))
	{
		case AF_INET:
			traffic->ipVersion = FWP_IP_VERSION_V4;
			traffic->localV4Address = untoh32(l->get_address(l).ptr);
			traffic->remoteV4Address = untoh32(r->get_address(r).ptr);
			return TRUE;
		case AF_INET6:
			traffic->ipVersion = FWP_IP_VERSION_V6;
			host2address6(l, &traffic->localV6Address);
			host2address6(r, &traffic->remoteV6Address);
			return TRUE;
		default:
			return FALSE;
	}
}

/**
 * Install SAs to the kernel
 */
static bool install_sas(private_kernel_wfp_ipsec_t *this, entry_t *entry,
						IPSEC_TRAFFIC_TYPE type)
{
	IPSEC_TRAFFIC1 traffic = {
		.trafficType = type,
	};
	IPSEC_GETSPI1 spi = {
		.inboundIpsecTraffic = {
			.trafficType = type,
		},
	};
	enumerator_t *enumerator;
	sp_entry_t *sp;
	DWORD res;

	if (type == IPSEC_TRAFFIC_TYPE_TRANSPORT)
	{
		enumerator = array_create_enumerator(entry->sps);
		if (enumerator->enumerate(enumerator, &sp))
		{
			traffic.ipsecFilterId = sp->policy_out;
			spi.inboundIpsecTraffic.ipsecFilterId = sp->policy_in;
		}
		else
		{
			enumerator->destroy(enumerator);
			return FALSE;
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		traffic.tunnelPolicyId = entry->provider;
		spi.inboundIpsecTraffic.tunnelPolicyId = entry->provider;
	}

	if (!hosts2traffic(this, entry->local, entry->remote, &traffic))
	{
		return FALSE;
	}

	res = IPsecSaContextCreate1(this->handle, &traffic, NULL, NULL,
								&entry->sa_id);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "creating WFP SA context failed: 0x%08x", res);
		return FALSE;
	}

	memcpy(spi.inboundIpsecTraffic.localV6Address, traffic.localV6Address,
		   sizeof(traffic.localV6Address));
	memcpy(spi.inboundIpsecTraffic.remoteV6Address, traffic.remoteV6Address,
		   sizeof(traffic.remoteV6Address));
	spi.ipVersion = traffic.ipVersion;

	res = IPsecSaContextSetSpi0(this->handle, entry->sa_id, &spi,
								ntohl(entry->isa.spi));
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "setting WFP SA SPI failed: 0x%08x", res);
		IPsecSaContextDeleteById0(this->handle, entry->sa_id);
		entry->sa_id = 0;
		return FALSE;
	}

	if (!install_sa(this, entry, TRUE, &entry->isa, spi.ipVersion) ||
		!install_sa(this, entry, FALSE, &entry->osa, spi.ipVersion))
	{
		IPsecSaContextDeleteById0(this->handle, entry->sa_id);
		entry->sa_id = 0;
		return FALSE;
	}

	if (entry->encap)
	{
		IPSEC_V4_UDP_ENCAPSULATION0 encap = {
			.localUdpEncapPort = entry->local->get_port(entry->local),
			.remoteUdpEncapPort = entry->remote->get_port(entry->remote),
		};
		IPSEC_SA_CONTEXT1 *ctx;

		res = IPsecSaContextGetById1(this->handle, entry->sa_id, &ctx);
		if (res != ERROR_SUCCESS)
		{
			DBG1(DBG_KNL, "getting WFP SA for UDP encap failed: 0x%08x", res);
			IPsecSaContextDeleteById0(this->handle, entry->sa_id);
			entry->sa_id = 0;
			return FALSE;
		}
		ctx->inboundSa->udpEncapsulation = &encap;
		ctx->outboundSa->udpEncapsulation = &encap;

		res = IPsecSaContextUpdate0(this->handle,
								IPSEC_SA_DETAILS_UPDATE_UDP_ENCAPSULATION, ctx);
		FwpmFreeMemory0((void**)&ctx);
		if (res != ERROR_SUCCESS)
		{
			DBG1(DBG_KNL, "enable WFP UDP encap failed: 0x%08x", res);
			IPsecSaContextDeleteById0(this->handle, entry->sa_id);
			entry->sa_id = 0;
			return FALSE;
		}
	}

	return TRUE;
}

/**
 * Install a transport mode SA/SP set to the kernel
 */
static bool install_transport(private_kernel_wfp_ipsec_t *this, entry_t *entry)
{
	if (install_sps(this, entry, NULL) &&
		install_sas(this, entry, IPSEC_TRAFFIC_TYPE_TRANSPORT))
	{
		return TRUE;
	}
	cleanup_policies(this, entry);
	return FALSE;
}

/**
 * Generate a new GUID, random
 */
static bool generate_guid(private_kernel_wfp_ipsec_t *this, GUID *guid)
{
	bool ok;
	rng_t *rng;

	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng)
	{
		return FALSE;
	}
	ok = rng->get_bytes(rng, sizeof(GUID), (u_int8_t*)guid);
	rng->destroy(rng);
	return ok;
}

/**
 * Register a dummy tunnel provider to associate tunnel filters to
 */
static bool add_tunnel_provider(private_kernel_wfp_ipsec_t *this,
								entry_t *entry, GUID *guid, UINT64 *luid)
{
	DWORD res;

	IPSEC_AUTH_TRANSFORM0 transform = {
		/* Create any valid proposal. This is actually not used, as we
		 * don't create an SA from this information. */
		.authTransformId = IPSEC_AUTH_TRANSFORM_ID_HMAC_SHA_1_96,
	};
	IPSEC_SA_TRANSFORM0 transforms = {
		.ipsecTransformType = IPSEC_TRANSFORM_ESP_AUTH,
		.espAuthTransform = &transform,
	};
	IPSEC_PROPOSAL0 proposal = {
		.lifetime = {
			/* We need a valid lifetime, even if we don't create any SA
			 * from these values. Pick some values accepted. */
			.lifetimeSeconds = 0xFFFF,
			.lifetimeKilobytes = 0xFFFFFFFF,
			.lifetimePackets = 0xFFFFFFFF,
		},
		.numSaTransforms = 1,
		.saTransforms = &transforms,
	};
	IPSEC_TUNNEL_POLICY0 policy = {
		.numIpsecProposals = 1,
		.ipsecProposals = &proposal,
		.saIdleTimeout = {
			/* not used, set to lifetime for maximum */
			.idleTimeoutSeconds = proposal.lifetime.lifetimeSeconds,
			.idleTimeoutSecondsFailOver = proposal.lifetime.lifetimeSeconds,
		},
	};
	FWPM_PROVIDER_CONTEXT0 qm = {
		.displayData = {
			.name = L"charon tunnel provider",
		},
		.providerKey = (GUID*)&this->provider.providerKey,
		.type = FWPM_IPSEC_IKE_QM_TUNNEL_CONTEXT,
		.ikeQmTunnelPolicy = &policy,
	};

	switch (entry->local->get_family(entry->local))
	{
		case AF_INET:
			policy.tunnelEndpoints.ipVersion = FWP_IP_VERSION_V4;
			policy.tunnelEndpoints.localV4Address =
						untoh32(entry->local->get_address(entry->local).ptr);
			policy.tunnelEndpoints.remoteV4Address =
						untoh32(entry->remote->get_address(entry->remote).ptr);
			break;
		case AF_INET6:
			policy.tunnelEndpoints.ipVersion = FWP_IP_VERSION_V6;
			host2address6(entry->local, &policy.tunnelEndpoints.localV6Address);
			host2address6(entry->remote, &policy.tunnelEndpoints.remoteV6Address);
			break;
		default:
			return FALSE;
	}

	if (!generate_guid(this, &qm.providerContextKey))
	{
		return FALSE;
	}

	res = FwpmProviderContextAdd0(this->handle, &qm, NULL, luid);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "adding provider context failed: 0x%08x", res);
		return FALSE;
	}
	*guid = qm.providerContextKey;
	return TRUE;
}

/**
 * Install tunnel mode SPs to the kernel
 */
static bool install_tunnel_sps(private_kernel_wfp_ipsec_t *this, entry_t *entry)
{
	GUID guid;

	if (!add_tunnel_provider(this, entry, &guid, &entry->provider))
	{
		return FALSE;
	}
	if (!install_sps(this, entry, &guid))
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * Reduce refcount, or uninstall a route if all refs gone
 */
static bool uninstall_route(private_kernel_wfp_ipsec_t *this,
							host_t *dst, u_int8_t mask, host_t *src, host_t *gtw)
{
	route_t *route, key = {
		.dst = dst,
		.mask = mask,
		.src = src,
	};
	char *name;
	bool res = FALSE;

	this->mutex->lock(this->mutex);
	route = this->routes->get(this->routes, &key);
	if (route)
	{
		if (--route->refs == 0)
		{
			if (hydra->kernel_interface->get_interface(hydra->kernel_interface,
													   src, &name))
			{
				res = hydra->kernel_interface->del_route(hydra->kernel_interface,
						dst->get_address(dst), mask, gtw, src, name) == SUCCESS;
				free(name);
			}
			route = this->routes->remove(this->routes, route);
			if (route)
			{
				destroy_route(route);
			}
		}
		else
		{
			res = TRUE;
		}
	}
	this->mutex->unlock(this->mutex);

	return res;
}

/**
 * Install a single route, or refcount if exists
 */
static bool install_route(private_kernel_wfp_ipsec_t *this,
						  host_t *dst, u_int8_t mask, host_t *src, host_t *gtw)
{
	route_t *route, key = {
		.dst = dst,
		.mask = mask,
		.src = src,
	};
	char *name;
	bool res = FALSE;

	this->mutex->lock(this->mutex);
	route = this->routes->get(this->routes, &key);
	if (route)
	{
		route->refs++;
		res = TRUE;
	}
	else
	{
		if (hydra->kernel_interface->get_interface(hydra->kernel_interface,
												   src, &name))
		{
			if (hydra->kernel_interface->add_route(hydra->kernel_interface,
						dst->get_address(dst), mask, gtw, src, name) == SUCCESS)
			{
				INIT(route,
					.dst = dst->clone(dst),
					.mask = mask,
					.src = src->clone(src),
					.gtw = gtw ? gtw->clone(gtw) : NULL,
					.refs = 1,
				);
				route = this->routes->put(this->routes, route, route);
				if (route)
				{
					destroy_route(route);
				}
				res = TRUE;
			}
			free(name);
		}
	}
	this->mutex->unlock(this->mutex);

	return res;
}

/**
 * (Un)-install a single route
 */
static bool manage_route(private_kernel_wfp_ipsec_t *this,
						 host_t *local, host_t *remote,
						 traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
						 bool add)
{
	host_t *src, *dst, *gtw;
	u_int8_t mask;
	bool done;

	if (!dst_ts->to_subnet(dst_ts, &dst, &mask))
	{
		return FALSE;
	}
	if (hydra->kernel_interface->get_address_by_ts(hydra->kernel_interface,
												src_ts, &src, NULL) != SUCCESS)
	{
		dst->destroy(dst);
		return FALSE;
	}
	gtw = hydra->kernel_interface->get_nexthop(hydra->kernel_interface,
											   remote, -1, local);
	if (add)
	{
		done = install_route(this, dst, mask, src, gtw);
	}
	else
	{
		done = uninstall_route(this, dst, mask, src, gtw);
	}
	dst->destroy(dst);
	src->destroy(src);
	DESTROY_IF(gtw);

	if (!done)
	{
		DBG1(DBG_KNL, "%sinstalling route for policy %R === %R failed",
			 add ? "" : "un", src_ts, dst_ts);
	}
	return done;
}

/**
 * (Un)-install routes for IPsec policies
 */
static bool manage_routes(private_kernel_wfp_ipsec_t *this, entry_t *entry,
						  bool add)
{
	enumerator_t *enumerator;
	sp_entry_t *sp;

	enumerator = array_create_enumerator(entry->sps);
	while (enumerator->enumerate(enumerator, &sp))
	{
		if (add && sp->route)
		{
			continue;
		}
		if (!add && !sp->route)
		{
			continue;
		}
		if (manage_route(this, entry->local, entry->remote,
						 sp->src, sp->dst, add))
		{
			sp->route = add;
		}
	}
	enumerator->destroy(enumerator);

	return TRUE;
}

/**
 * Install a tunnel mode SA/SP set to the kernel
 */
static bool install_tunnel(private_kernel_wfp_ipsec_t *this, entry_t *entry)
{
	if (install_tunnel_sps(this, entry) &&
		manage_routes(this, entry, TRUE) &&
		install_sas(this, entry, IPSEC_TRAFFIC_TYPE_TUNNEL))
	{
		return TRUE;
	}
	cleanup_policies(this, entry);
	return FALSE;
}

/**
 * Install a SA/SP set to the kernel
 */
static bool install(private_kernel_wfp_ipsec_t *this, entry_t *entry)
{
	switch (entry->mode)
	{
		case MODE_TRANSPORT:
			return install_transport(this, entry);
		case MODE_TUNNEL:
			return install_tunnel(this, entry);
		case MODE_BEET:
		default:
			return FALSE;
	}
}

/**
 * Installed trap entry
 */
typedef struct {
	/** reqid this trap is installed for */
	u_int32_t reqid;
	/** is this a forward policy trap for tunnel mode? */
	bool fwd;
	/** do we have installed a route for this trap policy? */
	bool route;
	/** local address of associated route */
	host_t *local;
	/** remote address of associated route */
	host_t *remote;
	/** src traffic selector */
	traffic_selector_t *src;
	/** dst traffic selector */
	traffic_selector_t *dst;
	/** LUID of installed tunnel policy filter */
	UINT64 filter_id;
} trap_t;

/**
 * Destroy a trap entry
 */
static void destroy_trap(trap_t *this)
{
	this->local->destroy(this->local);
	this->remote->destroy(this->remote);
	this->src->destroy(this->src);
	this->dst->destroy(this->dst);
	free(this);
}

/**
 * Hashtable equals function for traps
 */
static bool equals_trap(trap_t *a, trap_t *b)
{
	return a->filter_id == b->filter_id;
}

/**
 * Hashtable hash function for traps
 */
static u_int hash_trap(trap_t *trap)
{
	return chunk_hash(chunk_from_thing(trap->filter_id));
}

/**
 * Send an acquire for an installed trap filter
 */
static void acquire(private_kernel_wfp_ipsec_t *this, UINT64 filter_id,
					traffic_selector_t *src, traffic_selector_t *dst)
{
	u_int32_t reqid = 0;
	trap_t *trap, key = {
		.filter_id = filter_id,
	};

	this->mutex->lock(this->mutex);
	trap = this->traps->get(this->traps, &key);
	if (trap)
	{
		reqid = trap->reqid;
	}
	this->mutex->unlock(this->mutex);

	if (reqid)
	{
		src = src ? src->clone(src) : NULL;
		dst = dst ? dst->clone(dst) : NULL;
		hydra->kernel_interface->acquire(hydra->kernel_interface, reqid,
										 src, dst);
	}
}

/**
 * Create a single host traffic selector from an FWP address definition
 */
static traffic_selector_t *addr2ts(FWP_IP_VERSION version, void *data,
					u_int8_t protocol, u_int16_t from_port, u_int16_t to_port)
{
	ts_type_t type;
	UINT32 ints[4];
	chunk_t addr;

	switch (version)
	{
		case FWP_IP_VERSION_V4:
			ints[0] = untoh32(data);
			addr = chunk_from_thing(ints[0]);
			type = TS_IPV4_ADDR_RANGE;
			break;
		case FWP_IP_VERSION_V6:
			ints[3] = untoh32(data);
			ints[2] = untoh32(data + 4);
			ints[1] = untoh32(data + 8);
			ints[0] = untoh32(data + 12);
			addr = chunk_from_thing(ints);
			type = TS_IPV6_ADDR_RANGE;
			break;
		default:
			return NULL;
	}
	return traffic_selector_create_from_bytes(protocol, type, addr, from_port,
											  addr, to_port);
}

/**
 * FwpmNetEventSubscribe0() callback
 */
static void WINAPI event_callback(void *user, const FWPM_NET_EVENT1 *event)
{
	private_kernel_wfp_ipsec_t *this = user;
	traffic_selector_t *local = NULL, *remote = NULL;
	u_int8_t protocol = 0;
	u_int16_t from_local = 0, to_local = 65535;
	u_int16_t from_remote = 0, to_remote = 65535;

	if ((event->header.flags & FWPM_NET_EVENT_FLAG_LOCAL_ADDR_SET) &&
		(event->header.flags & FWPM_NET_EVENT_FLAG_REMOTE_ADDR_SET))
	{
		if (event->header.flags & FWPM_NET_EVENT_FLAG_LOCAL_PORT_SET)
		{
			from_local = to_local = event->header.localPort;
		}
		if (event->header.flags & FWPM_NET_EVENT_FLAG_LOCAL_PORT_SET)
		{
			from_remote = to_remote = event->header.remotePort;
		}
		if (event->header.flags & FWPM_NET_EVENT_FLAG_IP_PROTOCOL_SET)
		{
			protocol = event->header.ipProtocol;
		}

		local = addr2ts(event->header.ipVersion,
						(void*)&event->header.localAddrV6,
						protocol, from_local, to_local);
		remote = addr2ts(event->header.ipVersion,
						(void*)&event->header.remoteAddrV6,
						protocol, from_remote, to_remote);
	}

	switch (event->type)
	{
		case FWPM_NET_EVENT_TYPE_CLASSIFY_DROP:
			acquire(this, event->classifyDrop->filterId, local, remote);
			break;
		case FWPM_NET_EVENT_TYPE_IKEEXT_MM_FAILURE:
		case FWPM_NET_EVENT_TYPE_IKEEXT_QM_FAILURE:
		case FWPM_NET_EVENT_TYPE_IKEEXT_EM_FAILURE:
		case FWPM_NET_EVENT_TYPE_IPSEC_KERNEL_DROP:
			DBG1(DBG_KNL, "IPsec kernel drop: %R === %R, error 0x%08x, "
				 "SPI 0x%08x, %s filterId %llu", local, remote,
				 event->ipsecDrop->failureStatus, event->ipsecDrop->spi,
				 event->ipsecDrop->direction ? "in" : "out",
				 event->ipsecDrop->filterId);
			break;
		case FWPM_NET_EVENT_TYPE_IPSEC_DOSP_DROP:
		default:
			break;
	}

	DESTROY_IF(local);
	DESTROY_IF(remote);
}

/**
 * Register for net events
 */
static bool register_events(private_kernel_wfp_ipsec_t *this)
{
	FWPM_NET_EVENT_SUBSCRIPTION0 subscription = {};
	DWORD res;

	res = FwpmNetEventSubscribe0(this->handle, &subscription,
								 event_callback, this, &this->event);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "registering for WFP events failed: 0x%08x", res);
		return FALSE;
	}
	return TRUE;
}

/**
 * Install a trap policy to kernel
 */
static bool install_trap(private_kernel_wfp_ipsec_t *this, trap_t *trap)
{
	FWPM_FILTER_CONDITION0 *conds = NULL;
	int count = 0;
	DWORD res;
	const GUID *starget, *dtarget;
	UINT64 weight = 0x000000000000ff00;
	FWPM_FILTER0 filter = {
		.displayData = {
			.name = L"charon IPsec trap",
		},
		.action = {
			.type = FWP_ACTION_BLOCK,
		},
		.weight = {
			.type = FWP_UINT64,
			.uint64 = &weight,
		},
	};

	if (trap->fwd)
	{
		if (trap->src->get_type(trap->src) == TS_IPV4_ADDR_RANGE)
		{
			filter.layerKey = FWPM_LAYER_IPFORWARD_V4;
		}
		else
		{
			filter.layerKey = FWPM_LAYER_IPFORWARD_V6;
		}
		starget = &FWPM_CONDITION_IP_SOURCE_ADDRESS;
		dtarget = &FWPM_CONDITION_IP_DESTINATION_ADDRESS;
	}
	else
	{
		if (trap->src->get_type(trap->src) == TS_IPV4_ADDR_RANGE)
		{
			filter.layerKey = FWPM_LAYER_OUTBOUND_TRANSPORT_V4;
		}
		else
		{
			filter.layerKey = FWPM_LAYER_OUTBOUND_TRANSPORT_V6;
		}
		starget = &FWPM_CONDITION_IP_LOCAL_ADDRESS;
		dtarget = &FWPM_CONDITION_IP_REMOTE_ADDRESS;
	}

	if (!ts2condition(trap->src, starget, &conds, &count) ||
		!ts2condition(trap->dst, dtarget, &conds, &count))
	{
		free_conditions(conds, count);
		return FALSE;
	}

	filter.numFilterConditions = count;
	filter.filterCondition = conds;

	res = FwpmFilterAdd0(this->handle, &filter, NULL, &trap->filter_id);
	free_conditions(conds, count);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "installing WFP trap filter failed: 0x%08x", res);
		return FALSE;
	}
	return TRUE;
}

/**
 * Uninstall a trap policy from kernel
 */
static bool uninstall_trap(private_kernel_wfp_ipsec_t *this, trap_t *trap)
{
	DWORD res;

	res = FwpmFilterDeleteById0(this->handle, trap->filter_id);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "uninstalling WFP trap filter failed: 0x%08x", res);
		return FALSE;
	}
	return TRUE;
}

/**
 * Create and install a new trap entry
 */
static bool add_trap(private_kernel_wfp_ipsec_t *this,
					 u_int32_t reqid, bool fwd, host_t *local, host_t *remote,
					 traffic_selector_t *src, traffic_selector_t *dst)
{
	trap_t *trap;

	INIT(trap,
		.reqid = reqid,
		.fwd = fwd,
		.src = src->clone(src),
		.dst = dst->clone(dst),
		.local = local->clone(local),
		.remote = remote->clone(remote),
	);

	if (!install_trap(this, trap))
	{
		destroy_trap(trap);
		return FALSE;
	}

	trap->route = manage_route(this, local, remote, src, dst, TRUE);

	this->mutex->lock(this->mutex);
	this->traps->put(this->traps, trap, trap);
	this->mutex->unlock(this->mutex);
	return TRUE;
}

/**
 * Uninstall and remove a new trap entry
 */
static bool remove_trap(private_kernel_wfp_ipsec_t *this,
						u_int32_t reqid, bool fwd,
						traffic_selector_t *src, traffic_selector_t *dst)
{
	enumerator_t *enumerator;
	trap_t *trap, *found = NULL;

	this->mutex->lock(this->mutex);
	enumerator = this->traps->create_enumerator(this->traps);
	while (enumerator->enumerate(enumerator, NULL, &trap))
	{
		if (reqid == trap->reqid &&
			fwd == trap->fwd &&
			src->equals(src, trap->src) &&
			dst->equals(dst, trap->dst))
		{
			this->traps->remove_at(this->traps, enumerator);
			found = trap;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	if (found)
	{
		if (trap->route)
		{
			trap->route = !manage_route(this, trap->local, trap->remote,
										src, dst, FALSE);
		}
		uninstall_trap(this, found);
		destroy_trap(found);
		return TRUE;
	}
	return FALSE;
}

METHOD(kernel_ipsec_t, get_features, kernel_feature_t,
	private_kernel_wfp_ipsec_t *this)
{
	return KERNEL_ESP_V3_TFC | KERNEL_NO_POLICY_UPDATES;
}

/**
 * Initialize seeds for SPI generation
 */
static bool init_spi(private_kernel_wfp_ipsec_t *this)
{
	bool ok = TRUE;
	rng_t *rng;

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!rng)
	{
		return FALSE;
	}
	ok = rng->get_bytes(rng, sizeof(this->nextspi), (u_int8_t*)&this->nextspi);
	if (ok)
	{
		ok = rng->get_bytes(rng, sizeof(this->mixspi), (u_int8_t*)&this->mixspi);
	}
	rng->destroy(rng);
	return ok;
}

/**
 * Map an integer x with a one-to-one function using quadratic residues.
 */
static u_int permute(u_int x, u_int p)
{
	u_int qr;

	x = x % p;
	qr = ((u_int64_t)x * x) % p;
	if (x <= p / 2)
	{
		return qr;
	}
	return p - qr;
}

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_kernel_wfp_ipsec_t *this, host_t *src, host_t *dst,
	u_int8_t protocol, u_int32_t reqid, u_int32_t *spi)
{
	/* To avoid sequencial SPIs, we use a one-to-one permuation function on
	 * an incrementing counter, that is a full period PRNG for the range we
	 * allocate SPIs in. We add some randomness using a fixed XOR and start
	 * the counter at random position. This is not cryptographically safe,
	 * but that is actually not required.
	 * The selected prime should be smaller than the range we allocate SPIs
	 * in, and it must satisfy p % 4 == 3 to map x > p/2 using p - qr. */
	static const u_int p = 268435399, offset = 0xc0000000;

	*spi = htonl(offset + permute(ref_get(&this->nextspi) ^ this->mixspi, p));
	return SUCCESS;
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_kernel_wfp_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t reqid, u_int16_t *cpi)
{
	return NOT_SUPPORTED;
}

/**
 * Data for an expire callback job
 */
typedef struct {
	/* backref to kernel backend */
	private_kernel_wfp_ipsec_t *this;
	/* SPI of expiring SA */
	u_int32_t spi;
	/* destination address of expiring SA */
	host_t *dst;
	/* is this a hard expire, or a rekey request? */
	bool hard;
} expire_data_t;

/**
 * Clean up expire data
 */
static void expire_data_destroy(expire_data_t *data)
{
	data->dst->destroy(data->dst);
	free(data);
}

/**
 * Callback job for SA expiration
 */
static job_requeue_t expire_job(expire_data_t *data)
{
	private_kernel_wfp_ipsec_t *this = data->this;
	u_int32_t reqid = 0;
	u_int8_t protocol;
	entry_t *entry;
	sa_entry_t key = {
		.spi = data->spi,
		.dst = data->dst,
	};

	if (data->hard)
	{
		this->mutex->lock(this->mutex);
		entry = this->isas->remove(this->isas, &key);
		this->mutex->unlock(this->mutex);
		if (entry)
		{
			protocol = entry->isa.protocol;
			reqid = entry->reqid;
			if (entry->osa.dst)
			{
				key.dst = entry->osa.dst;
				key.spi = entry->osa.spi;
				this->osas->remove(this->osas, &key);
			}
			entry_destroy(this, entry);
		}
	}
	else
	{
		this->mutex->lock(this->mutex);
		entry = this->isas->get(this->isas, &key);
		if (entry)
		{
			protocol = entry->isa.protocol;
			reqid = entry->reqid;
		}
		this->mutex->unlock(this->mutex);
	}

	if (reqid)
	{
		hydra->kernel_interface->expire(hydra->kernel_interface,
										reqid, protocol, data->spi, data->hard);
	}

	return JOB_REQUEUE_NONE;
}

/**
 * Schedule an expire event for an SA
 */
static void schedule_expire(private_kernel_wfp_ipsec_t *this, u_int32_t spi,
							host_t *dst, u_int32_t lifetime, bool hard)
{
	expire_data_t *data;

	INIT(data,
		.this = this,
		.spi = spi,
		.dst = dst->clone(dst),
		.hard = hard,
	);

	lib->scheduler->schedule_job(lib->scheduler, (job_t*)
						callback_job_create((void*)expire_job, data,
											(void*)expire_data_destroy, NULL),
						lifetime);
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_wfp_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t spi, u_int8_t protocol, u_int32_t reqid, mark_t mark,
	u_int32_t tfc, lifetime_cfg_t *lifetime, u_int16_t enc_alg, chunk_t enc_key,
	u_int16_t int_alg, chunk_t int_key, ipsec_mode_t mode,
	u_int16_t ipcomp, u_int16_t cpi, u_int32_t replay_window,
	bool initiator, bool encap, bool esn, bool inbound,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts)
{
	host_t *local, *remote;
	entry_t *entry;

	if (inbound)
	{
		/* comes first, create new entry */
		local = dst->clone(dst);
		remote = src->clone(src);

		INIT(entry,
			.reqid = reqid,
			.isa = {
				.spi = spi,
				.dst = local,
				.protocol = protocol,
				.lifetime = lifetime->time.life,
				.encr = {
					.alg = enc_alg,
					.key = chunk_clone(enc_key),
				},
				.integ = {
					.alg = int_alg,
					.key = chunk_clone(int_key),
				},
			},
			.sps = array_create(0, 0),
			.local = local,
			.remote = remote,
			.mode = mode,
			.encap = encap,
		);

		if (lifetime->time.life)
		{
			schedule_expire(this, spi, local, lifetime->time.life, TRUE);
		}
		if (lifetime->time.rekey && lifetime->time.rekey != lifetime->time.life)
		{
			schedule_expire(this, spi, local, lifetime->time.rekey, FALSE);
		}

		this->mutex->lock(this->mutex);
		this->tsas->put(this->tsas, (void*)(uintptr_t)reqid, entry);
		this->isas->put(this->isas, &entry->isa, entry);
		this->mutex->unlock(this->mutex);
	}
	else
	{
		/* comes after inbound, update entry */
		this->mutex->lock(this->mutex);
		entry = this->tsas->remove(this->tsas, (void*)(uintptr_t)reqid);
		this->mutex->unlock(this->mutex);

		if (!entry)
		{
			DBG1(DBG_KNL, "adding outbound SA failed, no inbound SA found "
				 "for reqid %u ", reqid);
			return NOT_FOUND;
		}
		/* TODO: should we check for local/remote, mode etc.? */

		entry->osa = (sa_entry_t){
			.spi = spi,
			.dst = entry->remote,
			.protocol = protocol,
			.lifetime = lifetime->time.life,
			.encr = {
				.alg = enc_alg,
				.key = chunk_clone(enc_key),
			},
			.integ = {
				.alg = int_alg,
				.key = chunk_clone(int_key),
			},
		};

		this->mutex->lock(this->mutex);
		this->osas->put(this->osas, &entry->osa, entry);
		this->mutex->unlock(this->mutex);
	}

	return SUCCESS;
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_wfp_ipsec_t *this, u_int32_t spi, u_int8_t protocol,
	u_int16_t cpi, host_t *src, host_t *dst, host_t *new_src, host_t *new_dst,
	bool encap, bool new_encap, mark_t mark)
{
	entry_t *entry;
	sa_entry_t key = {
		.dst = dst,
		.spi = spi,
	};
	UINT64 sa_id = 0;
	IPSEC_SA_CONTEXT1 *ctx;
	IPSEC_V4_UDP_ENCAPSULATION0 ports;
	UINT32 flags = IPSEC_SA_DETAILS_UPDATE_TRAFFIC;
	DWORD res;

	this->mutex->lock(this->mutex);
	entry = this->osas->get(this->osas, &key);
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		/* outbound entry, nothing to do */
		return SUCCESS;
	}

	this->mutex->lock(this->mutex);
	entry = this->isas->get(this->isas, &key);
	if (entry)
	{
		/* inbound entry, do update */
		sa_id = entry->sa_id;
		ports.localUdpEncapPort = entry->local->get_port(entry->local);
		ports.remoteUdpEncapPort = entry->remote->get_port(entry->remote);
	}
	this->mutex->unlock(this->mutex);

	if (!sa_id)
	{
		return NOT_FOUND;
	}

	res = IPsecSaContextGetById1(this->handle, sa_id, &ctx);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "getting WFP SA context for updated failed: 0x%08x", res);
		return FAILED;
	}
	if (!hosts2traffic(this, new_dst, new_src, &ctx->inboundSa->traffic) ||
		!hosts2traffic(this, new_dst, new_src, &ctx->outboundSa->traffic))
	{
		FwpmFreeMemory0((void**)&ctx);
		return FAILED;
	}

	if (new_encap != encap)
	{
		if (new_encap)
		{
			ctx->inboundSa->udpEncapsulation = &ports;
			ctx->outboundSa->udpEncapsulation = &ports;
		}
		else
		{
			ctx->inboundSa->udpEncapsulation = NULL;
			ctx->outboundSa->udpEncapsulation = NULL;
		}
		flags |= IPSEC_SA_DETAILS_UPDATE_UDP_ENCAPSULATION;
	}

	res = IPsecSaContextUpdate0(this->handle, flags, ctx);
	FwpmFreeMemory0((void**)&ctx);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "updating WFP SA context failed: 0x%08x", res);
		return FAILED;
	}

	this->mutex->lock(this->mutex);
	entry = this->isas->remove(this->isas, &key);
	if (entry)
	{
		key.spi = entry->osa.spi;
		key.dst = entry->osa.dst;
		this->osas->remove(this->osas, &key);

		entry->local->destroy(entry->local);
		entry->remote->destroy(entry->remote);
		entry->local = new_dst->clone(new_dst);
		entry->remote = new_src->clone(new_src);
		entry->isa.dst = entry->local;
		entry->osa.dst = entry->remote;

		this->isas->put(this->isas, &entry->isa, entry);
		this->osas->put(this->osas, &entry->osa, entry);

		manage_routes(this, entry, FALSE);
		manage_routes(this, entry, TRUE);
	}
	this->mutex->unlock(this->mutex);

	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_wfp_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t spi, u_int8_t protocol, mark_t mark, u_int64_t *bytes,
	u_int64_t *packets, time_t *time)
{
	/* It does not seem that WFP provides any means of getting per-SA traffic
	 * statistics. IPsecGetStatistics0/1() provides global stats, and
	 * IPsecSaContextEnum0/1() and IPsecSaEnum0/1() return the configured
	 * values only. */
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_kernel_wfp_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t spi, u_int8_t protocol, u_int16_t cpi, mark_t mark)
{
	entry_t *entry;
	sa_entry_t key = {
		.dst = dst,
		.spi = spi,
	};

	this->mutex->lock(this->mutex);
	entry = this->isas->remove(this->isas, &key);
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		/* keep entry until removal of outbound */
		return SUCCESS;
	}

	this->mutex->lock(this->mutex);
	entry = this->osas->remove(this->osas, &key);
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		entry_destroy(this, entry);
		return SUCCESS;
	}

	return NOT_FOUND;
}

METHOD(kernel_ipsec_t, flush_sas, status_t,
	private_kernel_wfp_ipsec_t *this)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_wfp_ipsec_t *this, host_t *src, host_t *dst,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
	policy_dir_t direction, policy_type_t type, ipsec_sa_cfg_t *sa, mark_t mark,
	policy_priority_t priority)
{
	status_t status = SUCCESS;
	entry_t *entry;
	sp_entry_t *sp;
	sa_entry_t key = {
		.spi = sa->esp.use ? sa->esp.spi : sa->ah.spi,
		.dst = dst,
	};

	if (sa->esp.use && sa->ah.use)
	{
		return NOT_SUPPORTED;
	}

	switch (type)
	{
		case POLICY_IPSEC:
			break;
		case POLICY_PASS:
		case POLICY_DROP:
			return NOT_SUPPORTED;
	}

	switch (direction)
	{
		case POLICY_OUT:
			break;
		case POLICY_IN:
		case POLICY_FWD:
			/* not required */
			return SUCCESS;
		default:
			return NOT_SUPPORTED;
	}

	switch (priority)
	{
		case POLICY_PRIORITY_DEFAULT:
			break;
		case POLICY_PRIORITY_ROUTED:
			if (!add_trap(this, sa->reqid, FALSE, src, dst, src_ts, dst_ts))
			{
				return FAILED;
			}
			if (sa->mode == MODE_TUNNEL)
			{
				if (!add_trap(this, sa->reqid, TRUE, src, dst, src_ts, dst_ts))
				{
					return FAILED;
				}
			}
			return SUCCESS;
		case POLICY_PRIORITY_FALLBACK:
		default:
			return NOT_SUPPORTED;
	}

	this->mutex->lock(this->mutex);
	entry = this->osas->get(this->osas, &key);
	if (entry)
	{
		if (sa->mode == MODE_TUNNEL || array_count(entry->sps) == 0)
		{
			INIT(sp,
				.src = src_ts->clone(src_ts),
				.dst = dst_ts->clone(dst_ts),
			);
			array_insert(entry->sps, -1, sp);
			if (array_count(entry->sps) == sa->policy_count)
			{
				if (!install(this, entry))
				{
					status = FAILED;
				}
			}
		}
		else
		{
			/* TODO: reinstall with a filter using multiple TS?
			 * Filters are ANDed for a match, but we could install a filter
			 * with the inverse TS set using NOT-matches... */
			DBG1(DBG_KNL, "multiple transport mode traffic selectors not "
				 "supported by WFP");
			status = NOT_SUPPORTED;
		}
	}
	else
	{
		DBG1(DBG_KNL, "adding SP failed, no SA found for SPI 0x%08x", key.spi);
		status = FAILED;
	}
	this->mutex->unlock(this->mutex);

	return status;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_wfp_ipsec_t *this, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_dir_t direction, mark_t mark,
	time_t *use_time)
{
	/* see query_sa() for some notes */
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_kernel_wfp_ipsec_t *this, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_dir_t direction, u_int32_t reqid,
	mark_t mark, policy_priority_t priority)
{
	if (direction == POLICY_OUT && priority == POLICY_PRIORITY_ROUTED)
	{
		if (remove_trap(this, reqid, FALSE, src_ts, dst_ts))
		{
			remove_trap(this, reqid, TRUE, src_ts, dst_ts);
			return SUCCESS;
		}
		return NOT_FOUND;
	}
	/* not required, as we delete the whole SA/SP set during del_sa() */
	return SUCCESS;
}

METHOD(kernel_ipsec_t, flush_policies, status_t,
	private_kernel_wfp_ipsec_t *this)
{
	return NOT_SUPPORTED;
}

/**
 * Add a bypass policy for a specific UDP port
 */
static bool add_bypass(private_kernel_wfp_ipsec_t *this,
					   int family, u_int16_t port, bool inbound, UINT64 *luid)
{
	FWPM_FILTER_CONDITION0 *cond, *conds = NULL;
	int count = 0;
	DWORD res;
	UINT64 weight = 0xff00000000000000;
	FWPM_FILTER0 filter = {
		.displayData = {
			.name = L"charon IKE bypass",
		},
		.action = {
			.type = FWP_ACTION_PERMIT,
		},
		.weight = {
			.type = FWP_UINT64,
			.uint64 = &weight,
		},
	};

	switch (family)
	{
		case AF_INET:
			filter.layerKey = inbound ? FWPM_LAYER_INBOUND_TRANSPORT_V4
									  : FWPM_LAYER_OUTBOUND_TRANSPORT_V4;
			break;
		case AF_INET6:
			filter.layerKey = inbound ? FWPM_LAYER_INBOUND_TRANSPORT_V6
									  : FWPM_LAYER_OUTBOUND_TRANSPORT_V6;
			break;
		default:
			return FALSE;
	}

	cond = append_condition(&conds, &count);
	cond->fieldKey = FWPM_CONDITION_IP_PROTOCOL;
	cond->matchType = FWP_MATCH_EQUAL;
	cond->conditionValue.type = FWP_UINT8;
	cond->conditionValue.uint8 = IPPROTO_UDP;

	cond = append_condition(&conds, &count);
	cond->fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
	cond->matchType = FWP_MATCH_EQUAL;
	cond->conditionValue.type = FWP_UINT16;
	cond->conditionValue.uint16 = port;

	filter.numFilterConditions = count;
	filter.filterCondition = conds;

	res = FwpmFilterAdd0(this->handle, &filter, NULL, luid);
	free_conditions(conds, count);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "installing WFP bypass filter failed: 0x%08x", res);
		return FALSE;
	}
	return TRUE;
}

METHOD(kernel_ipsec_t, bypass_socket, bool,
	private_kernel_wfp_ipsec_t *this, int fd, int family)
{
	union {
		struct sockaddr sa;
		SOCKADDR_IN in;
		SOCKADDR_IN6 in6;
	} saddr;
	int addrlen = sizeof(saddr);
	UINT64 filter_out, filter_in = 0;
	u_int16_t port;

	if (getsockname(fd, &saddr.sa, &addrlen) == SOCKET_ERROR)
	{
		return FALSE;
	}
	switch (family)
	{
		case AF_INET:
			port = ntohs(saddr.in.sin_port);
			break;
		case AF_INET6:
			port = ntohs(saddr.in6.sin6_port);
			break;
		default:
			return FALSE;
	}

	if (!add_bypass(this, family, port, TRUE, &filter_in) ||
		!add_bypass(this, family, port, FALSE, &filter_out))
	{
		if (filter_in)
		{
			FwpmFilterDeleteById0(this->handle, filter_in);
		}
		return FALSE;
	}

	this->mutex->lock(this->mutex);
	array_insert(this->bypass, ARRAY_TAIL, &filter_in);
	array_insert(this->bypass, ARRAY_TAIL, &filter_out);
	this->mutex->unlock(this->mutex);

	return TRUE;
}

METHOD(kernel_ipsec_t, enable_udp_decap, bool,
	private_kernel_wfp_ipsec_t *this, int fd, int family, u_int16_t port)
{
	return FALSE;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_kernel_wfp_ipsec_t *this)
{
	UINT64 filter;

	while (array_remove(this->bypass, ARRAY_TAIL, &filter))
	{
		FwpmFilterDeleteById0(this->handle, filter);
	}
	if (this->handle)
	{
		if (this->event)
		{
			FwpmNetEventUnsubscribe0(this->handle, this->event);
		}
		FwpmProviderDeleteByKey0(this->handle, &this->provider.providerKey);
		FwpmEngineClose0(this->handle);
	}
	array_destroy(this->bypass);
	this->tsas->destroy(this->tsas);
	this->isas->destroy(this->isas);
	this->osas->destroy(this->osas);
	this->routes->destroy(this->routes);
	this->traps->destroy(this->traps);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * Described in header.
 */
kernel_wfp_ipsec_t *kernel_wfp_ipsec_create()
{
	private_kernel_wfp_ipsec_t *this;
	DWORD res;
	FWPM_SESSION0 session = {
		.displayData = {
			.name = L"charon",
			.description = L"strongSwan IKE kernel-wfp backend",
		},
	};

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
		.provider = {
			.displayData = {
				.name = L"charon",
				.description = L"strongSwan IKE kernel-wfp backend",
			},
			.providerKey = { 0x59cdae2e, 0xf6bb, 0x4c09,
							{ 0xa9,0x59,0x9d,0x91,0xac,0xaf,0xf9,0x19 }},
		},
		.mutex = mutex_create(MUTEX_TYPE_RECURSIVE),
		.bypass = array_create(sizeof(UINT64), 2),
		.tsas = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 4),
		.isas = hashtable_create((void*)hash_sa, (void*)equals_sa, 4),
		.osas = hashtable_create((void*)hash_sa, (void*)equals_sa, 4),
		.routes = hashtable_create((void*)hash_route, (void*)equals_route, 4),
		.traps = hashtable_create((void*)hash_trap, (void*)equals_trap, 4),
	);

	if (!init_spi(this))
	{
		destroy(this);
		return NULL;
	}

	res = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session,
						  &this->handle);
	if (res != ERROR_SUCCESS)
	{
		DBG1(DBG_KNL, "opening WFP engine failed: 0x%08x", res);
		destroy(this);
		return NULL;
	}

	res = FwpmProviderAdd0(this->handle, &this->provider, NULL);
	if (res != ERROR_SUCCESS && res != FWP_E_ALREADY_EXISTS)
	{
		DBG1(DBG_KNL, "registering WFP provider failed: 0x%08x", res);
		destroy(this);
		return NULL;
	}

	if (!register_events(this))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
