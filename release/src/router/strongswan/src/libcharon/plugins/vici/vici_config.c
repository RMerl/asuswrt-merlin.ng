/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * Copyright (C) 2015-2018 Tobias Brunner
 * Copyright (C) 2015-2018 Andreas Steffen
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
 * Copyright (C) 2014 Timo Teräs <timo.teras@iki.fi>
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

#include "vici_config.h"
#include "vici_builder.h"

#include <daemon.h>
#include <threading/rwlock.h>
#include <threading/rwlock_condvar.h>
#include <collections/array.h>
#include <collections/linked_list.h>

#include <pubkey_cert.h>

#include <stdio.h>

/**
 * Magic value for an undefined lifetime
 */
#define LFT_UNDEFINED (~(uint64_t)0)

/**
 * Default IKE rekey time
 */
#define LFT_DEFAULT_IKE_REKEY_TIME		(4 * 60 * 60)

/**
 * Default CHILD rekey time
 */
#define LFT_DEFAULT_CHILD_REKEY_TIME	(1 * 60 * 60)

/**
 * Default CHILD rekey bytes
 */
#define LFT_DEFAULT_CHILD_REKEY_BYTES		0

/**
 * Default CHILD rekey packets
 */
#define LFT_DEFAULT_CHILD_REKEY_PACKETS		0

/**
 * Undefined replay window
 */
#define REPLAY_UNDEFINED (~(uint32_t)0)

typedef struct private_vici_config_t private_vici_config_t;

/**
 * Private data of an vici_config_t object.
 */
struct private_vici_config_t {

	/**
	 * Public vici_config_t interface.
	 */
	vici_config_t public;

	/**
	 * Dispatcher
	 */
	vici_dispatcher_t *dispatcher;

	/**
	 * List of loaded connections, as peer_cfg_t
	 */
	linked_list_t *conns;

	/**
	 * Lock for conns list
	 */
	rwlock_t *lock;

	/**
	 * Condvar used to sync running actions
	 */
	rwlock_condvar_t *condvar;

	/**
	 * True while we run or undo a start action
	 */
	bool handling_actions;

	/**
	 * Credential backend managed by VICI used for our certificates
	 */
	vici_cred_t *cred;

	/**
	 * Auxiliary certification authority information
	 */
	vici_authority_t *authority;

};

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_vici_config_t *this, identification_t *me, identification_t *other)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_cleaner(this->conns->create_enumerator(this->conns),
									 (void*)this->lock->unlock, this->lock);
}

CALLBACK(ike_filter, bool,
	void *data, enumerator_t *orig, va_list args)
{
	peer_cfg_t *cfg;
	ike_cfg_t **out;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, &cfg))
	{
		*out = cfg->get_ike_cfg(cfg);
		return TRUE;
	}
	return FALSE;
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	private_vici_config_t *this, host_t *me, host_t *other)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(this->conns->create_enumerator(this->conns),
									ike_filter, this->lock,
									(void*)this->lock->unlock);
}

METHOD(backend_t, get_peer_cfg_by_name, peer_cfg_t*,
	private_vici_config_t *this, char *name)
{
	peer_cfg_t *current, *found = NULL;
	enumerator_t *enumerator;

	this->lock->read_lock(this->lock);
	enumerator = this->conns->create_enumerator(this->conns);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (streq(current->get_name(current), name))
		{
			found = current;
			found->get_ref(found);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return found;
}

/**
 * Create a (error) reply message
 */
static vici_message_t* create_reply(char *fmt, ...)
{
	vici_builder_t *builder;
	va_list args;

	builder = vici_builder_create();
	builder->add_kv(builder, "success", fmt ? "no" : "yes");
	if (fmt)
	{
		va_start(args, fmt);
		builder->vadd_kv(builder, "errmsg", fmt, args);
		va_end(args);
	}
	return builder->finalize(builder);
}

/**
 * A rule to parse a key/value or list item
 */
typedef struct {
	/** name of the key/value or list */
	char *name;
	/** function to parse value */
	bool (*parse)(void *out, chunk_t value);
	/** result, passed to parse() */
	void *out;
} parse_rule_t;

/**
 * Parse key/values using a rule-set
 */
static bool parse_rules(parse_rule_t *rules, int count, char *name,
						chunk_t value, vici_message_t **reply)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (streq(name, rules[i].name))
		{
			if (rules[i].parse(rules[i].out, value))
			{
				return TRUE;
			}
			*reply = create_reply("invalid value for: %s, config discarded",
								  name);
			return FALSE;
		}
	}
	*reply = create_reply("unknown option: %s, config discarded", name);
	return FALSE;
}

/**
 * Parse callback data, passed to each callback
 */
typedef struct {
	private_vici_config_t *this;
	vici_message_t *reply;
} request_data_t;

/**
 * Certificate data
 */
typedef struct {
	request_data_t *request;
	char *handle;
	uint32_t slot;
	char *module;
	char *file;
} cert_data_t;

/**
 * Clean up certificate data
 */
static void free_cert_data(cert_data_t *data)
{
	free(data->handle);
	free(data->module);
	free(data->file);
	free(data);
}

/**
 * Auth config data
 */
typedef struct {
	request_data_t *request;
	auth_cfg_t *cfg;
	uint32_t round;
} auth_data_t;

/**
 * Clean up auth config data
 */
static void free_auth_data(auth_data_t *data)
{
	DESTROY_IF(data->cfg);
	free(data);
}

/**
 * Data associated to a peer config
 */
typedef struct {
	request_data_t *request;
	uint32_t version;
	bool aggressive;
	bool encap;
	bool mobike;
	bool send_certreq;
	bool pull;
	identification_t *ppk_id;
	bool ppk_required;
	cert_policy_t send_cert;
	uint64_t dpd_delay;
	uint64_t dpd_timeout;
	fragmentation_t fragmentation;
	unique_policy_t unique;
	uint32_t keyingtries;
	uint32_t local_port;
	uint32_t remote_port;
	char *local_addrs;
	char *remote_addrs;
	linked_list_t *local;
	linked_list_t *remote;
	linked_list_t *proposals;
	linked_list_t *children;
	linked_list_t *vips;
	char *pools;
	uint64_t reauth_time;
	uint64_t rekey_time;
	uint64_t over_time;
	uint64_t rand_time;
	uint8_t dscp;
#ifdef ME
	bool mediation;
	char *mediated_by;
	identification_t *peer_id;
#endif /* ME */
} peer_data_t;

/**
 * Log relevant auth config data
 */
static void log_auth(auth_cfg_t *auth)
{
	enumerator_t *enumerator;
	auth_rule_t rule;
	union {
		uintptr_t u;
		identification_t *id;
		char *str;
	} v;

	enumerator = auth->create_enumerator(auth);
	while (enumerator->enumerate(enumerator, &rule, &v))
	{
		switch (rule)
		{
			case AUTH_RULE_AUTH_CLASS:
				DBG2(DBG_CFG, "   class = %N", auth_class_names, v.u);
				break;
			case AUTH_RULE_EAP_TYPE:
				DBG2(DBG_CFG, "   eap-type = %N", eap_type_names, v.u);
				break;
			case AUTH_RULE_EAP_VENDOR:
				DBG2(DBG_CFG, "   eap-vendor = %u", v.u);
				break;
			case AUTH_RULE_XAUTH_BACKEND:
				DBG2(DBG_CFG, "   xauth = %s", v.str);
				break;
			case AUTH_RULE_CRL_VALIDATION:
				DBG2(DBG_CFG, "   revocation = %N", cert_validation_names, v.u);
				break;
			case AUTH_RULE_IDENTITY:
				DBG2(DBG_CFG, "   id = %Y", v.id);
				break;
			case AUTH_RULE_AAA_IDENTITY:
				DBG2(DBG_CFG, "   aaa_id = %Y", v.id);
				break;
			case AUTH_RULE_EAP_IDENTITY:
				DBG2(DBG_CFG, "   eap_id = %Y", v.id);
				break;
			case AUTH_RULE_XAUTH_IDENTITY:
				DBG2(DBG_CFG, "   xauth_id = %Y", v.id);
				break;
			case AUTH_RULE_GROUP:
				DBG2(DBG_CFG, "   group = %Y", v.id);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Log parsed peer data
 */
static void log_peer_data(peer_data_t *data)
{
	enumerator_t *enumerator;
	auth_data_t *auth;
	host_t *host;

	DBG2(DBG_CFG, "  version = %u", data->version);
	DBG2(DBG_CFG, "  local_addrs = %s", data->local_addrs);
	DBG2(DBG_CFG, "  remote_addrs = %s", data->remote_addrs);
	DBG2(DBG_CFG, "  local_port = %u", data->local_port);
	DBG2(DBG_CFG, "  remote_port = %u", data->remote_port);
	DBG2(DBG_CFG, "  send_certreq = %u", data->send_certreq);
	DBG2(DBG_CFG, "  send_cert = %N", cert_policy_names, data->send_cert);
	DBG2(DBG_CFG, "  ppk_id = %Y",  data->ppk_id);
	DBG2(DBG_CFG, "  ppk_required = %u",  data->ppk_required);
	DBG2(DBG_CFG, "  mobike = %u", data->mobike);
	DBG2(DBG_CFG, "  aggressive = %u", data->aggressive);
	DBG2(DBG_CFG, "  dscp = 0x%.2x", data->dscp);
	DBG2(DBG_CFG, "  encap = %u", data->encap);
	DBG2(DBG_CFG, "  dpd_delay = %llu", data->dpd_delay);
	DBG2(DBG_CFG, "  dpd_timeout = %llu", data->dpd_timeout);
	DBG2(DBG_CFG, "  fragmentation = %u",  data->fragmentation);
	DBG2(DBG_CFG, "  unique = %N", unique_policy_names, data->unique);
	DBG2(DBG_CFG, "  keyingtries = %u", data->keyingtries);
	DBG2(DBG_CFG, "  reauth_time = %llu", data->reauth_time);
	DBG2(DBG_CFG, "  rekey_time = %llu", data->rekey_time);
	DBG2(DBG_CFG, "  over_time = %llu", data->over_time);
	DBG2(DBG_CFG, "  rand_time = %llu", data->rand_time);
	DBG2(DBG_CFG, "  proposals = %#P", data->proposals);
#ifdef ME
	DBG2(DBG_CFG, "  mediation = %u", data->mediation);
	if (data->mediated_by)
	{
		DBG2(DBG_CFG, "  mediated_by = %s", data->mediated_by);
		DBG2(DBG_CFG, "  mediation_peer = %Y", data->peer_id);
	}
#endif /* ME */

	if (data->vips->get_count(data->vips))
	{
		DBG2(DBG_CFG, "  vips:");
	}
	enumerator = data->vips->create_enumerator(data->vips);
	while (enumerator->enumerate(enumerator, &host))
	{
		DBG2(DBG_CFG, "   %H", host);
	}
	enumerator->destroy(enumerator);

	enumerator = data->local->create_enumerator(data->local);
	while (enumerator->enumerate(enumerator, &auth))
	{
		DBG2(DBG_CFG, "  local:");
		log_auth(auth->cfg);
	}
	enumerator->destroy(enumerator);

	enumerator = data->remote->create_enumerator(data->remote);
	while (enumerator->enumerate(enumerator, &auth))
	{
		DBG2(DBG_CFG, "  remote:");
		log_auth(auth->cfg);
	}
	enumerator->destroy(enumerator);
}

/**
 * Clean up peer config data
 */
static void free_peer_data(peer_data_t *data)
{
	data->local->destroy_function(data->local, (void*)free_auth_data);
	data->remote->destroy_function(data->remote, (void*)free_auth_data);
	data->children->destroy_offset(data->children,
									offsetof(child_cfg_t, destroy));
	data->proposals->destroy_offset(data->proposals,
									offsetof(proposal_t, destroy));
	data->vips->destroy_offset(data->vips, offsetof(host_t, destroy));
	free(data->pools);
	free(data->local_addrs);
	free(data->remote_addrs);
	DESTROY_IF(data->ppk_id);
#ifdef ME
	free(data->mediated_by);
	DESTROY_IF(data->peer_id);
#endif /* ME */
}

/**
 * CHILD config data
 */
typedef struct {
	request_data_t *request;
	linked_list_t *proposals;
	linked_list_t *local_ts;
	linked_list_t *remote_ts;
	uint32_t replay_window;
	child_cfg_create_t cfg;
} child_data_t;

/**
 * Log parsed CHILD config data
 */
static void log_child_data(child_data_t *data, char *name)
{
	child_cfg_create_t *cfg = &data->cfg;

#define has_opt(opt) ({ (cfg->options & (opt)) == (opt); })
	DBG2(DBG_CFG, "  child %s:", name);
	DBG2(DBG_CFG, "   rekey_time = %llu", cfg->lifetime.time.rekey);
	DBG2(DBG_CFG, "   life_time = %llu", cfg->lifetime.time.life);
	DBG2(DBG_CFG, "   rand_time = %llu", cfg->lifetime.time.jitter);
	DBG2(DBG_CFG, "   rekey_bytes = %llu", cfg->lifetime.bytes.rekey);
	DBG2(DBG_CFG, "   life_bytes = %llu", cfg->lifetime.bytes.life);
	DBG2(DBG_CFG, "   rand_bytes = %llu", cfg->lifetime.bytes.jitter);
	DBG2(DBG_CFG, "   rekey_packets = %llu", cfg->lifetime.packets.rekey);
	DBG2(DBG_CFG, "   life_packets = %llu", cfg->lifetime.packets.life);
	DBG2(DBG_CFG, "   rand_packets = %llu", cfg->lifetime.packets.jitter);
	DBG2(DBG_CFG, "   updown = %s", cfg->updown);
	DBG2(DBG_CFG, "   hostaccess = %u", has_opt(OPT_HOSTACCESS));
	DBG2(DBG_CFG, "   ipcomp = %u", has_opt(OPT_IPCOMP));
	DBG2(DBG_CFG, "   mode = %N%s", ipsec_mode_names, cfg->mode,
		 has_opt(OPT_PROXY_MODE) ? "_PROXY" : "");
	DBG2(DBG_CFG, "   policies = %u", !has_opt(OPT_NO_POLICIES));
	DBG2(DBG_CFG, "   policies_fwd_out = %u", has_opt(OPT_FWD_OUT_POLICIES));
	if (data->replay_window != REPLAY_UNDEFINED)
	{
		DBG2(DBG_CFG, "   replay_window = %u", data->replay_window);
	}
	DBG2(DBG_CFG, "   dpd_action = %N", action_names, cfg->dpd_action);
	DBG2(DBG_CFG, "   start_action = %N", action_names, cfg->start_action);
	DBG2(DBG_CFG, "   close_action = %N", action_names, cfg->close_action);
	DBG2(DBG_CFG, "   reqid = %u", cfg->reqid);
	DBG2(DBG_CFG, "   tfc = %d", cfg->tfc);
	DBG2(DBG_CFG, "   priority = %d", cfg->priority);
	DBG2(DBG_CFG, "   interface = %s", cfg->interface);
	DBG2(DBG_CFG, "   mark_in = %u/%u",
		 cfg->mark_in.value, cfg->mark_in.mask);
	DBG2(DBG_CFG, "   mark_in_sa = %u", has_opt(OPT_MARK_IN_SA));
	DBG2(DBG_CFG, "   mark_out = %u/%u",
		 cfg->mark_out.value, cfg->mark_out.mask);
	DBG2(DBG_CFG, "   set_mark_in = %u/%u",
		 cfg->set_mark_in.value, cfg->set_mark_in.mask);
	DBG2(DBG_CFG, "   set_mark_out = %u/%u",
		 cfg->set_mark_out.value, cfg->set_mark_out.mask);
	DBG2(DBG_CFG, "   inactivity = %llu", cfg->inactivity);
	DBG2(DBG_CFG, "   proposals = %#P", data->proposals);
	DBG2(DBG_CFG, "   local_ts = %#R", data->local_ts);
	DBG2(DBG_CFG, "   remote_ts = %#R", data->remote_ts);
	DBG2(DBG_CFG, "   hw_offload = %N", hw_offload_names, cfg->hw_offload);
	DBG2(DBG_CFG, "   sha256_96 = %u", has_opt(OPT_SHA256_96));
	DBG2(DBG_CFG, "   copy_df = %u", !has_opt(OPT_NO_COPY_DF));
	DBG2(DBG_CFG, "   copy_ecn = %u", !has_opt(OPT_NO_COPY_ECN));
	DBG2(DBG_CFG, "   copy_dscp = %N", dscp_copy_names, cfg->copy_dscp);
}

/**
 * Clean up CHILD config data
 */
static void free_child_data(child_data_t *data)
{
	data->proposals->destroy_offset(data->proposals,
									offsetof(proposal_t, destroy));
	data->local_ts->destroy_offset(data->local_ts,
									offsetof(traffic_selector_t, destroy));
	data->remote_ts->destroy_offset(data->remote_ts,
									offsetof(traffic_selector_t, destroy));
	free(data->cfg.updown);
	free(data->cfg.interface);
}

/**
 * Common proposal parsing
 */
static bool parse_proposal(linked_list_t *list, protocol_id_t proto, chunk_t v)
{
	char buf[BUF_LEN];
	proposal_t *proposal;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	if (strcaseeq("default", buf))
	{
		proposal = proposal_create_default(proto);
		if (proposal)
		{
			list->insert_last(list, proposal);
		}
		proposal = proposal_create_default_aead(proto);
		if (proposal)
		{
			list->insert_last(list, proposal);
		}
		return TRUE;
	}
	proposal = proposal_create_from_string(proto, buf);
	if (proposal)
	{
		list->insert_last(list, proposal);
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse IKE proposal
 */
CALLBACK(parse_ike_proposal, bool,
	linked_list_t *out, chunk_t v)
{
	return parse_proposal(out, PROTO_IKE, v);
}

/**
 * Parse ESP proposal
 */
CALLBACK(parse_esp_proposal, bool,
	linked_list_t *out, chunk_t v)
{
	return parse_proposal(out, PROTO_ESP, v);
}

/**
 * Parse AH proposal
 */
CALLBACK(parse_ah_proposal, bool,
	linked_list_t *out, chunk_t v)
{
	return parse_proposal(out, PROTO_AH, v);
}

/**
 * Parse a traffic selector
 */
CALLBACK(parse_ts, bool,
	linked_list_t *out, chunk_t v)
{
	char buf[BUF_LEN], *protoport, *sep, *port = "", *end;
	traffic_selector_t *ts = NULL;
	struct protoent *protoent;
	struct servent *svc;
	long int p;
	uint16_t from = 0, to = 0xffff;
	uint8_t proto = 0;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}

	protoport = strchr(buf, '[');
	if (protoport)
	{
		*(protoport++) = '\0';

		sep = strrchr(protoport, ']');
		if (!sep)
		{
			return FALSE;
		}
		*sep = '\0';

		sep = strchr(protoport, '/');
		if (sep)
		{	/* protocol/port */
			*sep = '\0';
			port = sep + 1;
		}

		if (streq(protoport, "any"))
		{
			proto = 0;
		}
		else
		{
			protoent = getprotobyname(protoport);
			if (protoent)
			{
				proto = protoent->p_proto;
			}
			else
			{
				p = strtol(protoport, &end, 0);
				if ((*protoport && *end) || p < 0 || p > 0xff)
				{
					return FALSE;
				}
				proto = (uint8_t)p;
			}
		}
		if (streq(port, "opaque"))
		{
			from = 0xffff;
			to = 0;
		}
		else if (*port && !streq(port, "any"))
		{
			svc = getservbyname(port, NULL);
			if (svc)
			{
				from = to = ntohs(svc->s_port);
			}
			else
			{
				p = strtol(port, &end, 0);
				if (p < 0 || p > 0xffff)
				{
					return FALSE;
				}
				from = p;
				if (*end == '-')
				{
					port = end + 1;
					p = strtol(port, &end, 0);
					if (p < 0 || p > 0xffff)
					{
						return FALSE;
					}
				}
				to = p;
				if (*end)
				{
					return FALSE;
				}
			}
		}
	}
	if (streq(buf, "dynamic"))
	{
		ts = traffic_selector_create_dynamic(proto, from, to);
	}
	else if (strchr(buf, '-'))
	{
		host_t *lower, *upper;
		ts_type_t type;

		if (host_create_from_range(buf, &lower, &upper))
		{
			type = (lower->get_family(lower) == AF_INET) ?
								TS_IPV4_ADDR_RANGE : TS_IPV6_ADDR_RANGE;
			ts = traffic_selector_create_from_bytes(proto, type,
								lower->get_address(lower), from,
								upper->get_address(upper), to);
			lower->destroy(lower);
			upper->destroy(upper);
		}
	}
	else
	{
		ts = traffic_selector_create_from_cidr(buf, proto, from, to);
	}
	if (!ts)
	{
		return FALSE;
	}
	out->insert_last(out, ts);
	return TRUE;
}

/**
 * Parse a string
 */
CALLBACK(parse_string, bool,
	char **out, chunk_t v)
{
	if (!chunk_printable(v, NULL, ' '))
	{
		return FALSE;
	}
	free(*out);
	*out = NULL;
	if (asprintf(out, "%.*s", (int)v.len, v.ptr) == -1)
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * Map a string to an integer
 */
typedef struct {
	char *str;
	int d;
} enum_map_t;

/**
 * Parse a string to an integer mapping
 */
static bool parse_map(enum_map_t *map, int count, int *out, chunk_t v)
{
	char buf[BUF_LEN];
	int i;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	for (i = 0; i < count; i++)
	{
		if (strcaseeq(map[i].str, buf))
		{
			*out = map[i].d;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Parse a boolean
 */
CALLBACK(parse_bool, bool,
	bool *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "yes",		TRUE	},
		{ "true",		TRUE	},
		{ "enabled",	TRUE	},
		{ "1",			TRUE	},
		{ "no",			FALSE	},
		{ "false",		FALSE	},
		{ "disabled",	FALSE	},
		{ "0",			FALSE	},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out = d;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse a ipsec_mode_t
 */
CALLBACK(parse_mode, bool,
	child_cfg_create_t *cfg, chunk_t v)
{
	enum_map_t map[] = {
		{ "tunnel",				MODE_TUNNEL		},
		{ "transport",			MODE_TRANSPORT	},
		{ "transport_proxy",	MODE_TRANSPORT	},
		{ "beet",				MODE_BEET		},
		{ "drop",				MODE_DROP		},
		{ "pass",				MODE_PASS		},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		cfg->mode = d;
		if ((d == MODE_TRANSPORT) && (v.len > 9))
		{
			cfg->options |= OPT_PROXY_MODE;
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Enable a child_cfg_option_t, the flag controls whether the option is enabled
 * if the parsed value is TRUE or FALSE.
 */
static bool parse_option(child_cfg_option_t *out, child_cfg_option_t opt,
						 chunk_t v, bool add_if_true)
{
	bool val;

	if (parse_bool(&val, v))
	{
		if (val == add_if_true)
		{
			*out |= opt;
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse OPT_HOSTACCESS option
 */
CALLBACK(parse_opt_haccess, bool,
	child_cfg_option_t *out, chunk_t v)
{
	return parse_option(out, OPT_HOSTACCESS, v, TRUE);
}

/**
 * Parse OPT_NO_POLICIES option
 */
CALLBACK(parse_opt_policies, bool,
	child_cfg_option_t *out, chunk_t v)
{
	return parse_option(out, OPT_NO_POLICIES, v, FALSE);
}

/**
 * Parse OPT_FWD_OUT_POLICIES option
 */
CALLBACK(parse_opt_fwd_out, bool,
	child_cfg_option_t *out, chunk_t v)
{
	return parse_option(out, OPT_FWD_OUT_POLICIES, v, TRUE);
}

/**
 * Parse OPT_IPCOMP option
 */
CALLBACK(parse_opt_ipcomp, bool,
	child_cfg_option_t *out, chunk_t v)
{
	return parse_option(out, OPT_IPCOMP, v, TRUE);
}

/**
 * Parse OPT_SHA256_96 option
 */
CALLBACK(parse_opt_sha256_96, bool,
	child_cfg_option_t *out, chunk_t v)
{
	return parse_option(out, OPT_SHA256_96, v, TRUE);
}

/**
 * Parse OPT_MARK_IN_SA option
 */
CALLBACK(parse_opt_mark_in, bool,
	child_cfg_option_t *out, chunk_t v)
{
	return parse_option(out, OPT_MARK_IN_SA, v, TRUE);
}

/**
 * Parse OPT_NO_COPY_DF option
 */
CALLBACK(parse_opt_copy_df, bool,
	child_cfg_option_t *out, chunk_t v)
{
	return parse_option(out, OPT_NO_COPY_DF, v, FALSE);
}

/**
 * Parse OPT_NO_COPY_ECN option
 */
CALLBACK(parse_opt_copy_ecn, bool,
	child_cfg_option_t *out, chunk_t v)
{
	return parse_option(out, OPT_NO_COPY_ECN, v, FALSE);
}

/**
 * Parse a dscp_copy_t
 */
CALLBACK(parse_copy_dscp, bool,
	dscp_copy_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "no",		DSCP_COPY_NO		},
		{ "in",		DSCP_COPY_IN_ONLY	},
		{ "out",	DSCP_COPY_OUT_ONLY	},
		{ "yes",	DSCP_COPY_YES		},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out = d;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse an action_t
 */
CALLBACK(parse_action, bool,
	action_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "start",		ACTION_RESTART	},
		{ "restart",	ACTION_RESTART	},
		{ "route",		ACTION_ROUTE	},
		{ "trap",		ACTION_ROUTE	},
		{ "none",		ACTION_NONE		},
		{ "clear",		ACTION_NONE		},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out = d;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse an hw_offload_t
 */
CALLBACK(parse_hw_offload, bool,
	action_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "no",		HW_OFFLOAD_NO	},
		{ "yes",	HW_OFFLOAD_YES	},
		{ "auto",	HW_OFFLOAD_AUTO	},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out = d;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse a uint32_t with the given base
 */
static bool parse_uint32_base(uint32_t *out, chunk_t v, int base)
{
	char buf[16], *end;
	u_long l;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	l = strtoul(buf, &end, base);
	if (*end == 0)
	{
		*out = l;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse a uint32_t
 */
CALLBACK(parse_uint32, bool,
	uint32_t *out, chunk_t v)
{
	return parse_uint32_base(out, v, 0);
}

/**
 * Parse a uint32_t in binary encoding
 */
CALLBACK(parse_uint32_bin, bool,
	uint32_t *out, chunk_t v)
{
	return parse_uint32_base(out, v, 2);
}

/**
 * Parse a uint64_t
 */
CALLBACK(parse_uint64, bool,
	uint64_t *out, chunk_t v)
{
	char buf[16], *end;
	unsigned long long l;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	l = strtoull(buf, &end, 0);
	if (*end == 0)
	{
		*out = l;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse a relative time
 */
CALLBACK(parse_time, bool,
	uint64_t *out, chunk_t v)
{
	char buf[16], *end;
	u_long l;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}

	l = strtoul(buf, &end, 0);
	while (*end == ' ')
	{
		end++;
	}
	switch (*end)
	{
		case 'd':
		case 'D':
			l *= 24;
			/* fall */
		case 'h':
		case 'H':
			l *= 60;
			/* fall */
		case 'm':
		case 'M':
			l *= 60;
			/* fall */
		case 's':
		case 'S':
			end++;
			break;
		case '\0':
			break;
		default:
			return FALSE;
	}
	if (*end)
	{
		return FALSE;
	}
	*out = l;
	return TRUE;
}

/**
 * Parse byte volume
 */
CALLBACK(parse_bytes, bool,
	uint64_t *out, chunk_t v)
{
	char buf[16], *end;
	unsigned long long l;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}

	l = strtoull(buf, &end, 0);
	while (*end == ' ')
	{
		end++;
	}
	switch (*end)
	{
		case 'g':
		case 'G':
			l *= 1024;
			/* fall */
		case 'm':
		case 'M':
			l *= 1024;
			/* fall */
		case 'k':
		case 'K':
			l *= 1024;
			end++;
			break;
		case '\0':
			break;
		default:
			return FALSE;
	}
	if (*end)
	{
		return FALSE;
	}
	*out = l;
	return TRUE;
}

/**
 * Parse a mark_t
 */
CALLBACK(parse_mark, bool,
	mark_t *out, chunk_t v)
{
	char buf[32];

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	return mark_from_string(buf, MARK_OP_UNIQUE, out);
}

/**
 * Parse a mark_t when using it as set_mark.
 */
CALLBACK(parse_set_mark, bool,
	mark_t *out, chunk_t v)
{
	char buf[32];

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	return mark_from_string(buf, MARK_OP_SAME, out);
}

/**
 * Parse TFC padding option
 */
CALLBACK(parse_tfc, bool,
	uint32_t *out, chunk_t v)
{
	if (chunk_equals(v, chunk_from_str("mtu")))
	{
		*out = -1;
		return TRUE;
	}
	return parse_uint32(out, v);
}

/**
 * Parse 6-bit DSCP value
 */
CALLBACK(parse_dscp, bool,
	uint8_t *out, chunk_t v)
{
	if (parse_uint32_bin(out, v))
	{
		*out = *out & 0x3f;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse authentication config
 */
CALLBACK(parse_auth, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	char buf[64], *pos;
	eap_vendor_type_t *type;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	if (strpfx(buf, "ike:") ||
		strpfx(buf, "pubkey") ||
		strpfx(buf, "rsa") ||
		strpfx(buf, "ecdsa") ||
		strpfx(buf, "bliss"))
	{
		cfg->add(cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
		cfg->add_pubkey_constraints(cfg, buf, TRUE);
		return TRUE;
	}
	if (strcaseeq(buf, "psk"))
	{
		cfg->add(cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PSK);
		return TRUE;
	}
	if (strcasepfx(buf, "xauth"))
	{
		pos = strchr(buf, '-');
		if (pos)
		{
			cfg->add(cfg, AUTH_RULE_XAUTH_BACKEND, strdup(++pos));
		}
		cfg->add(cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_XAUTH);
		return TRUE;
	}
	if (strcasepfx(buf, "eap"))
	{
		char *pos;

		cfg->add(cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_EAP);

		pos = strchr(buf, ':');
		if (pos)
		{
			*pos = 0;
			cfg->add_pubkey_constraints(cfg, pos + 1, FALSE);
		}
		type = eap_vendor_type_from_string(buf);
		if (type)
		{
			cfg->add(cfg, AUTH_RULE_EAP_TYPE, type->type);
			if (type->vendor)
			{
				cfg->add(cfg, AUTH_RULE_EAP_VENDOR, type->vendor);
			}
			free(type);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse identity; add as auth rule to config
 */
static bool parse_id(auth_cfg_t *cfg, auth_rule_t rule, chunk_t v)
{
	char buf[BUF_LEN];

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	cfg->add(cfg, rule, identification_create_from_string(buf));
	return TRUE;
}

/**
 * Parse IKE identity
 */
CALLBACK(parse_ike_id, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	return parse_id(cfg, AUTH_RULE_IDENTITY, v);
}

/**
 * Parse AAA identity
 */
CALLBACK(parse_aaa_id, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	return parse_id(cfg, AUTH_RULE_AAA_IDENTITY, v);
}

/**
 * Parse EAP identity
 */
CALLBACK(parse_eap_id, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	return parse_id(cfg, AUTH_RULE_EAP_IDENTITY, v);
}

/**
 * Parse XAuth identity
 */
CALLBACK(parse_xauth_id, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	return parse_id(cfg, AUTH_RULE_XAUTH_IDENTITY, v);
}

/**
 * Parse group membership
 */
CALLBACK(parse_group, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	return parse_id(cfg, AUTH_RULE_GROUP, v);
}

/**
 * Parse certificate policy
 */
CALLBACK(parse_cert_policy, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	char buf[BUF_LEN];

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	cfg->add(cfg, AUTH_RULE_CERT_POLICY, strdup(buf));
	return TRUE;
}

/**
 * Add a certificate as auth rule to config
 */
static bool add_cert(auth_data_t *auth, auth_rule_t rule, certificate_t *cert)
{
	vici_authority_t *authority;
	vici_cred_t *cred;

	if (rule == AUTH_RULE_SUBJECT_CERT)
	{
		authority = auth->request->this->authority;
		authority->check_for_hash_and_url(authority, cert);
	}
	cred = auth->request->this->cred;
	cert = cred->add_cert(cred, cert);
	auth->cfg->add(auth->cfg, rule, cert);
	return TRUE;
}

/**
 * Parse a certificate; add as auth rule to config
 */
static bool parse_cert(auth_data_t *auth, auth_rule_t rule, chunk_t v)
{
	certificate_t *cert;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_BLOB_PEM, v, BUILD_END);
	if (cert)
	{
		return add_cert(auth, rule, cert);
	}
	return FALSE;
}

/**
 * Parse subject certificates
 */
CALLBACK(parse_certs, bool,
	auth_data_t *auth, chunk_t v)
{
	return parse_cert(auth, AUTH_RULE_SUBJECT_CERT, v);
}

/**
 * Parse CA certificates
 */
CALLBACK(parse_cacerts, bool,
	auth_data_t *auth, chunk_t v)
{
	return parse_cert(auth, AUTH_RULE_CA_CERT, v);
}

/**
 * Parse raw public keys
 */
CALLBACK(parse_pubkeys, bool,
	auth_data_t *auth, chunk_t v)
{
	vici_cred_t *cred;
	certificate_t *cert;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_TRUSTED_PUBKEY,
							  BUILD_BLOB_PEM, v, BUILD_END);
	if (cert)
	{
		cred = auth->request->this->cred;
		cert = cred->add_cert(cred, cert);
		auth->cfg->add(auth->cfg, AUTH_RULE_SUBJECT_CERT, cert);
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse revocation status
 */
CALLBACK(parse_revocation, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	enum_map_t map[] = {
		{ "strict",		VALIDATION_GOOD		},
		{ "ifuri",		VALIDATION_SKIPPED	},
		{ "relaxed",	VALIDATION_FAILED	},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		if (d != VALIDATION_FAILED)
		{
			cfg->add(cfg, AUTH_RULE_CRL_VALIDATION, d);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse list items to comma separated strings
 */
CALLBACK(parse_stringlist, bool,
	char **out, chunk_t v)
{
	char *current;

	if (!chunk_printable(v, NULL, ' '))
	{
		return FALSE;
	}
	current = *out;
	if (current)
	{
		if (asprintf(out, "%s, %.*s", current, (int)v.len, v.ptr) == -1)
		{
			return FALSE;
		}
		free(current);
	}
	else
	{
		if (asprintf(out, "%.*s", (int)v.len, v.ptr) == -1)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Parse an fragmentation_t
 */
CALLBACK(parse_frag, bool,
	fragmentation_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "yes",		FRAGMENTATION_YES		},
		{ "accept",		FRAGMENTATION_ACCEPT	},
		{ "no",			FRAGMENTATION_NO		},
		{ "force",		FRAGMENTATION_FORCE		},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out = d;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse a cert_policy_t
 */
CALLBACK(parse_send_cert, bool,
	cert_policy_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "ifasked",	CERT_SEND_IF_ASKED	},
		{ "always",		CERT_ALWAYS_SEND	},
		{ "never",		CERT_NEVER_SEND		},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out = d;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse a unique_policy_t
 */
CALLBACK(parse_unique, bool,
	unique_policy_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "never",		UNIQUE_NEVER		},
		{ "no",			UNIQUE_NO			},
		{ "replace",	UNIQUE_REPLACE		},
		{ "keep",		UNIQUE_KEEP			},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out = d;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse host_t into a list
 */
CALLBACK(parse_hosts, bool,
	linked_list_t *list, chunk_t v)
{
	char buf[64];
	host_t *host;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	host = host_create_from_string(buf, 0);
	if (!host)
	{
		return FALSE;
	}
	list->insert_last(list, host);
	return TRUE;
}

/**
 * Parse peer/ppk ID
 */
CALLBACK(parse_peer_id, bool,
	identification_t **out, chunk_t v)
{
	char buf[BUF_LEN];

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	*out = identification_create_from_string(buf);
	return TRUE;
}


CALLBACK(cert_kv, bool,
	cert_data_t *cert, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "handle",			parse_string,		&cert->handle				},
		{ "slot",			parse_uint32,		&cert->slot					},
		{ "module",			parse_string,		&cert->module				},
		{ "file",			parse_string,		&cert->file					},
	};

	return parse_rules(rules, countof(rules), name, value,
					   &cert->request->reply);
}

CALLBACK(child_li, bool,
	child_data_t *child, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "ah_proposals",	parse_ah_proposal,	child->proposals			},
		{ "esp_proposals",	parse_esp_proposal,	child->proposals			},
		{ "local_ts",		parse_ts,			child->local_ts				},
		{ "remote_ts",		parse_ts,			child->remote_ts			},
	};

	return parse_rules(rules, countof(rules), name, value,
					   &child->request->reply);
}

CALLBACK(child_kv, bool,
	child_data_t *child, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "updown",				parse_string,		&child->cfg.updown					},
		{ "hostaccess",			parse_opt_haccess,	&child->cfg.options					},
		{ "mode",				parse_mode,			&child->cfg							},
		{ "policies",			parse_opt_policies,	&child->cfg.options					},
		{ "policies_fwd_out",	parse_opt_fwd_out,	&child->cfg.options					},
		{ "replay_window",		parse_uint32,		&child->replay_window				},
		{ "rekey_time",			parse_time,			&child->cfg.lifetime.time.rekey		},
		{ "life_time",			parse_time,			&child->cfg.lifetime.time.life		},
		{ "rand_time",			parse_time,			&child->cfg.lifetime.time.jitter	},
		{ "rekey_bytes",		parse_bytes,		&child->cfg.lifetime.bytes.rekey	},
		{ "life_bytes",			parse_bytes,		&child->cfg.lifetime.bytes.life		},
		{ "rand_bytes",			parse_bytes,		&child->cfg.lifetime.bytes.jitter	},
		{ "rekey_packets",		parse_uint64,		&child->cfg.lifetime.packets.rekey	},
		{ "life_packets",		parse_uint64,		&child->cfg.lifetime.packets.life	},
		{ "rand_packets",		parse_uint64,		&child->cfg.lifetime.packets.jitter	},
		{ "dpd_action",			parse_action,		&child->cfg.dpd_action				},
		{ "start_action",		parse_action,		&child->cfg.start_action			},
		{ "close_action",		parse_action,		&child->cfg.close_action			},
		{ "ipcomp",				parse_opt_ipcomp,	&child->cfg.options					},
		{ "inactivity",			parse_time,			&child->cfg.inactivity				},
		{ "reqid",				parse_uint32,		&child->cfg.reqid					},
		{ "mark_in",			parse_mark,			&child->cfg.mark_in					},
		{ "mark_in_sa",			parse_opt_mark_in,	&child->cfg.options					},
		{ "mark_out",			parse_mark,			&child->cfg.mark_out				},
		{ "set_mark_in",		parse_set_mark,		&child->cfg.set_mark_in				},
		{ "set_mark_out",		parse_set_mark,		&child->cfg.set_mark_out			},
		{ "tfc_padding",		parse_tfc,			&child->cfg.tfc						},
		{ "priority",			parse_uint32,		&child->cfg.priority				},
		{ "interface",			parse_string,		&child->cfg.interface				},
		{ "hw_offload",			parse_hw_offload,	&child->cfg.hw_offload				},
		{ "sha256_96",			parse_opt_sha256_96,&child->cfg.options					},
		{ "copy_df",			parse_opt_copy_df,	&child->cfg.options					},
		{ "copy_ecn",			parse_opt_copy_ecn,	&child->cfg.options					},
		{ "copy_dscp",			parse_copy_dscp,	&child->cfg.copy_dscp				},
	};

	return parse_rules(rules, countof(rules), name, value,
					   &child->request->reply);
}

CALLBACK(auth_li, bool,
	auth_data_t *auth, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "groups",			parse_group,		auth->cfg					},
		{ "cert_policy",	parse_cert_policy,	auth->cfg					},
		{ "certs",			parse_certs,		auth						},
		{ "cacerts",		parse_cacerts,		auth						},
		{ "pubkeys",		parse_pubkeys,		auth						},
	};

	return parse_rules(rules, countof(rules), name, value,
					   &auth->request->reply);
}

CALLBACK(auth_kv, bool,
	auth_data_t *auth, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "auth",			parse_auth,			auth->cfg					},
		{ "id",				parse_ike_id,		auth->cfg					},
		{ "aaa_id",			parse_aaa_id,		auth->cfg					},
		{ "eap_id",			parse_eap_id,		auth->cfg					},
		{ "xauth_id",		parse_xauth_id,		auth->cfg					},
		{ "revocation",		parse_revocation,	auth->cfg					},
		{ "round",			parse_uint32,		&auth->round				},
	};

	return parse_rules(rules, countof(rules), name, value,
					   &auth->request->reply);
}

CALLBACK(peer_li, bool,
	peer_data_t *peer, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "local_addrs",	parse_stringlist,	&peer->local_addrs			},
		{ "remote_addrs",	parse_stringlist,	&peer->remote_addrs			},
		{ "proposals",		parse_ike_proposal,	peer->proposals				},
		{ "vips",			parse_hosts,		peer->vips					},
		{ "pools",			parse_stringlist,	&peer->pools				},
	};

	return parse_rules(rules, countof(rules), name, value,
					   &peer->request->reply);
}

CALLBACK(peer_kv, bool,
	peer_data_t *peer, vici_message_t *message, char *name, chunk_t value)
{
	parse_rule_t rules[] = {
		{ "version",		parse_uint32,		&peer->version				},
		{ "aggressive",		parse_bool,			&peer->aggressive			},
		{ "pull",			parse_bool,			&peer->pull					},
		{ "dscp",			parse_dscp,			&peer->dscp					},
		{ "encap",			parse_bool,			&peer->encap				},
		{ "mobike",			parse_bool,			&peer->mobike				},
		{ "dpd_delay",		parse_time,			&peer->dpd_delay			},
		{ "dpd_timeout",	parse_time,			&peer->dpd_timeout			},
		{ "fragmentation",	parse_frag,			&peer->fragmentation		},
		{ "send_certreq",	parse_bool,			&peer->send_certreq			},
		{ "send_cert",		parse_send_cert,	&peer->send_cert			},
		{ "keyingtries",	parse_uint32,		&peer->keyingtries			},
		{ "unique",			parse_unique,		&peer->unique				},
		{ "local_port",		parse_uint32,		&peer->local_port			},
		{ "remote_port",	parse_uint32,		&peer->remote_port			},
		{ "reauth_time",	parse_time,			&peer->reauth_time			},
		{ "rekey_time",		parse_time,			&peer->rekey_time			},
		{ "over_time",		parse_time,			&peer->over_time			},
		{ "rand_time",		parse_time,			&peer->rand_time			},
		{ "ppk_id",			parse_peer_id,		&peer->ppk_id				},
		{ "ppk_required",	parse_bool,			&peer->ppk_required			},
#ifdef ME
		{ "mediation",		parse_bool,			&peer->mediation			},
		{ "mediated_by",	parse_string,		&peer->mediated_by			},
		{ "mediation_peer",	parse_peer_id,		&peer->peer_id				},
#endif /* ME */
	};

	return parse_rules(rules, countof(rules), name, value,
					   &peer->request->reply);
}

CALLBACK(auth_sn, bool,
	auth_data_t *auth, vici_message_t *message, vici_parse_context_t *ctx,
	char *name)
{
	if (strcasepfx(name, "cert") ||
		strcasepfx(name, "cacert"))
	{
		cert_data_t *data;
		auth_rule_t rule;
		certificate_t *cert;
		chunk_t handle;

		INIT(data,
			.request = auth->request,
			.slot = -1,
		);

		if (!message->parse(message, ctx, NULL, cert_kv, NULL, data))
		{
			free_cert_data(data);
			return FALSE;
		}
		if  (!data->handle && !data->file)
		{
			auth->request->reply = create_reply("handle or file path missing: "
												"%s", name);
			free_cert_data(data);
			return FALSE;
		}
		else if (data->handle && data->file)
		{
			auth->request->reply = create_reply("handle and file path given: "
												"%s", name);
			free_cert_data(data);
			return FALSE;
		}

		if (data->file)
		{
			cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								BUILD_FROM_FILE, data->file, BUILD_END);
		}
		else
		{
			handle = chunk_from_hex(chunk_from_str(data->handle), NULL);
			if (data->slot != -1)
			{
				cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
								CERT_X509, BUILD_PKCS11_KEYID, handle,
								BUILD_PKCS11_SLOT, data->slot,
								data->module ? BUILD_PKCS11_MODULE : BUILD_END,
								data->module, BUILD_END);
			}
			else
			{
				cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
								CERT_X509, BUILD_PKCS11_KEYID, handle,
								data->module ? BUILD_PKCS11_MODULE : BUILD_END,
								data->module, BUILD_END);
			}
			chunk_free(&handle);
		}
		free_cert_data(data);
		if (!cert)
		{
			auth->request->reply = create_reply("unable to load certificate: "
												"%s", name);
			return FALSE;
		}
		rule = strcasepfx(name, "cert") ? AUTH_RULE_SUBJECT_CERT
										: AUTH_RULE_CA_CERT;
		return add_cert(auth, rule, cert);
	}
	auth->request->reply = create_reply("invalid section: %s", name);
	return FALSE;
}

/**
 * Check and update lifetimes
 */
static void check_lifetimes(lifetime_cfg_t *lft)
{
	/* if no hard lifetime specified, add one at soft lifetime + 10% */
	if (lft->time.life == LFT_UNDEFINED)
	{
		lft->time.life = lft->time.rekey * 110 / 100;
	}
	if (lft->bytes.life == LFT_UNDEFINED)
	{
		lft->bytes.life = lft->bytes.rekey * 110 / 100;
	}
	if (lft->packets.life == LFT_UNDEFINED)
	{
		lft->packets.life = lft->packets.rekey * 110 / 100;
	}
	/* if no rand time defined, use difference of hard and soft */
	if (lft->time.jitter == LFT_UNDEFINED)
	{
		lft->time.jitter = lft->time.life -
									min(lft->time.life, lft->time.rekey);
	}
	if (lft->bytes.jitter == LFT_UNDEFINED)
	{
		lft->bytes.jitter = lft->bytes.life -
									min(lft->bytes.life, lft->bytes.rekey);
	}
	if (lft->packets.jitter == LFT_UNDEFINED)
	{
		lft->packets.jitter = lft->packets.life -
									min(lft->packets.life, lft->packets.rekey);
	}
}

CALLBACK(children_sn, bool,
	peer_data_t *peer, vici_message_t *message, vici_parse_context_t *ctx,
	char *name)
{
	child_data_t child = {
		.request = peer->request,
		.proposals = linked_list_create(),
		.local_ts = linked_list_create(),
		.remote_ts = linked_list_create(),
		.replay_window = REPLAY_UNDEFINED,
		.cfg = {
			.mode = MODE_TUNNEL,
			.lifetime = {
				.time = {
					.rekey = LFT_DEFAULT_CHILD_REKEY_TIME,
					.life = LFT_UNDEFINED,
					.jitter = LFT_UNDEFINED,
				},
				.bytes = {
					.rekey = LFT_DEFAULT_CHILD_REKEY_BYTES,
					.life = LFT_UNDEFINED,
					.jitter = LFT_UNDEFINED,
				},
				.packets = {
					.rekey = LFT_DEFAULT_CHILD_REKEY_PACKETS,
					.life = LFT_UNDEFINED,
					.jitter = LFT_UNDEFINED,
				},
			},
		},
	};
	child_cfg_t *cfg;
	proposal_t *proposal;
	traffic_selector_t *ts;

	if (!message->parse(message, ctx, NULL, child_kv, child_li, &child))
	{
		free_child_data(&child);
		return FALSE;
	}

	if (child.local_ts->get_count(child.local_ts) == 0)
	{
		child.local_ts->insert_last(child.local_ts,
							traffic_selector_create_dynamic(0, 0, 65535));
	}
	if (child.remote_ts->get_count(child.remote_ts) == 0)
	{
		child.remote_ts->insert_last(child.remote_ts,
							traffic_selector_create_dynamic(0, 0, 65535));
	}
	if (child.proposals->get_count(child.proposals) == 0)
	{
		proposal = proposal_create_default(PROTO_ESP);
		if (proposal)
		{
			child.proposals->insert_last(child.proposals, proposal);
		}
		proposal = proposal_create_default_aead(PROTO_ESP);
		if (proposal)
		{
			child.proposals->insert_last(child.proposals, proposal);
		}
	}

	check_lifetimes(&child.cfg.lifetime);

	log_child_data(&child, name);

	cfg = child_cfg_create(name, &child.cfg);

	if (child.replay_window != REPLAY_UNDEFINED)
	{
		cfg->set_replay_window(cfg, child.replay_window);
	}
	while (child.local_ts->remove_first(child.local_ts,
										(void**)&ts) == SUCCESS)
	{
		cfg->add_traffic_selector(cfg, TRUE, ts);
	}
	while (child.remote_ts->remove_first(child.remote_ts,
										 (void**)&ts) == SUCCESS)
	{
		cfg->add_traffic_selector(cfg, FALSE, ts);
	}
	while (child.proposals->remove_first(child.proposals,
										 (void**)&proposal) == SUCCESS)
	{
		cfg->add_proposal(cfg, proposal);
	}

	peer->children->insert_last(peer->children, cfg);

	free_child_data(&child);

	return TRUE;
}

CALLBACK(peer_sn, bool,
	peer_data_t *peer, vici_message_t *message, vici_parse_context_t *ctx,
	char *name)
{
	if (strcaseeq(name, "children"))
	{
		return message->parse(message, ctx, children_sn, NULL, NULL, peer);
	}
	if (strcasepfx(name, "local") ||
		strcasepfx(name, "remote"))
	{
		enumerator_t *enumerator;
		linked_list_t *auths;
		auth_data_t *auth, *current;
		auth_rule_t rule;
		certificate_t *cert;
		pubkey_cert_t *pubkey_cert;
		identification_t *id;
		bool default_id = FALSE;

		INIT(auth,
			.request = peer->request,
			.cfg = auth_cfg_create(),
		);

		if (!message->parse(message, ctx, auth_sn, auth_kv, auth_li, auth))
		{
			free_auth_data(auth);
			return FALSE;
		}
		id = auth->cfg->get(auth->cfg, AUTH_RULE_IDENTITY);

		enumerator = auth->cfg->create_enumerator(auth->cfg);
		while (enumerator->enumerate(enumerator, &rule, &cert))
		{
			if (rule == AUTH_RULE_SUBJECT_CERT && !default_id)
			{
				if (id == NULL)
				{
					id = cert->get_subject(cert);
					DBG1(DBG_CFG, "  id not specified, defaulting to"
								  " cert subject '%Y'", id);
					auth->cfg->add(auth->cfg, AUTH_RULE_IDENTITY, id->clone(id));
					default_id = TRUE;
				}
				else if (cert->get_type(cert) == CERT_TRUSTED_PUBKEY &&
						 id->get_type != ID_ANY)
				{
					/* set the subject of all raw public keys to the id */
					pubkey_cert = (pubkey_cert_t*)cert;
					pubkey_cert->set_subject(pubkey_cert, id);
				}
			}
		}
		enumerator->destroy(enumerator);

		auths = strcasepfx(name, "local") ? peer->local : peer->remote;
		enumerator = auths->create_enumerator(auths);
		while (enumerator->enumerate(enumerator, &current))
		{
			if (auth->round < current->round)
			{
				break;
			}
		}
		auths->insert_before(auths, enumerator, auth);
		enumerator->destroy(enumerator);
		return TRUE;
	}
	peer->request->reply = create_reply("invalid section: %s", name);
	return FALSE;
}

/**
 * Perform start actions associated with a child config
 */
static void run_start_action(private_vici_config_t *this, peer_cfg_t *peer_cfg,
							 child_cfg_t *child_cfg)
{
	switch (child_cfg->get_start_action(child_cfg))
	{
		case ACTION_RESTART:
			DBG1(DBG_CFG, "initiating '%s'", child_cfg->get_name(child_cfg));
			charon->controller->initiate(charon->controller,
					peer_cfg->get_ref(peer_cfg), child_cfg->get_ref(child_cfg),
					NULL, NULL, 0, FALSE);
			break;
		case ACTION_ROUTE:
			DBG1(DBG_CFG, "installing '%s'", child_cfg->get_name(child_cfg));
			switch (child_cfg->get_mode(child_cfg))
			{
				case MODE_PASS:
				case MODE_DROP:
					charon->shunts->install(charon->shunts,
									peer_cfg->get_name(peer_cfg), child_cfg);
					break;
				default:
					charon->traps->install(charon->traps, peer_cfg, child_cfg);
					break;
			}
			break;
		default:
			break;
	}
}

/**
 * Undo start actions associated with a child config
 */
static void clear_start_action(private_vici_config_t *this, char *peer_name,
							   child_cfg_t *child_cfg)
{
	enumerator_t *enumerator, *children;
	child_sa_t *child_sa;
	ike_sa_t *ike_sa;
	uint32_t id = 0, others;
	array_t *ids = NULL, *ikeids = NULL;
	char *name;

	name = child_cfg->get_name(child_cfg);

	switch (child_cfg->get_start_action(child_cfg))
	{
		case ACTION_RESTART:
			enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
			while (enumerator->enumerate(enumerator, &ike_sa))
			{
				if (!streq(ike_sa->get_name(ike_sa), peer_name))
				{
					continue;
				}
				others = id = 0;
				children = ike_sa->create_child_sa_enumerator(ike_sa);
				while (children->enumerate(children, &child_sa))
				{
					if (child_sa->get_state(child_sa) != CHILD_DELETING &&
						child_sa->get_state(child_sa) != CHILD_DELETED)
					{
						if (streq(name, child_sa->get_name(child_sa)))
						{
							id = child_sa->get_unique_id(child_sa);
						}
						else
						{
							others++;
						}
					}
				}
				children->destroy(children);

				if (id && !others)
				{
					/* found matching children only, delete full IKE_SA */
					id = ike_sa->get_unique_id(ike_sa);
					array_insert_create_value(&ikeids, sizeof(id),
											  ARRAY_TAIL, &id);
				}
				else
				{
					children = ike_sa->create_child_sa_enumerator(ike_sa);
					while (children->enumerate(children, &child_sa))
					{
						if (streq(name, child_sa->get_name(child_sa)))
						{
							id = child_sa->get_unique_id(child_sa);
							array_insert_create_value(&ids, sizeof(id),
													  ARRAY_TAIL, &id);
						}
					}
					children->destroy(children);
				}
			}
			enumerator->destroy(enumerator);

			if (array_count(ids))
			{
				while (array_remove(ids, ARRAY_HEAD, &id))
				{
					DBG1(DBG_CFG, "closing '%s' #%u", name, id);
					charon->controller->terminate_child(charon->controller,
														id, NULL, NULL, 0);
				}
				array_destroy(ids);
			}
			if (array_count(ikeids))
			{
				while (array_remove(ikeids, ARRAY_HEAD, &id))
				{
					DBG1(DBG_CFG, "closing IKE_SA #%u", id);
					charon->controller->terminate_ike(charon->controller, FALSE,
													  id, NULL, NULL, 0);
				}
				array_destroy(ikeids);
			}
			break;
		case ACTION_ROUTE:
			DBG1(DBG_CFG, "uninstalling '%s'", name);
			switch (child_cfg->get_mode(child_cfg))
			{
				case MODE_PASS:
				case MODE_DROP:
					charon->shunts->uninstall(charon->shunts, peer_name, name);
					break;
				default:
					charon->traps->uninstall(charon->traps, peer_name, name);
					break;
			}
			break;
		default:
			break;
	}
}

/**
 * Run or undo a start actions associated with a child config
 */
static void handle_start_action(private_vici_config_t *this,
								peer_cfg_t *peer_cfg, child_cfg_t *child_cfg,
								bool undo)
{
	this->handling_actions = TRUE;
	this->lock->unlock(this->lock);

	if (undo)
	{
		clear_start_action(this, peer_cfg->get_name(peer_cfg), child_cfg);
	}
	else
	{
		run_start_action(this, peer_cfg, child_cfg);
	}

	this->lock->write_lock(this->lock);
	this->handling_actions = FALSE;
}

/**
 * Run or undo start actions associated with all child configs of a peer config
 */
static void handle_start_actions(private_vici_config_t *this,
								 peer_cfg_t *peer_cfg, bool undo)
{
	enumerator_t *enumerator;
	child_cfg_t *child_cfg;

	this->handling_actions = TRUE;
	this->lock->unlock(this->lock);

	enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
	while (enumerator->enumerate(enumerator, &child_cfg))
	{
		if (undo)
		{
			clear_start_action(this, peer_cfg->get_name(peer_cfg), child_cfg);
		}
		else
		{
			run_start_action(this, peer_cfg, child_cfg);
		}
	}
	enumerator->destroy(enumerator);

	this->lock->write_lock(this->lock);
	this->handling_actions = FALSE;
}

/**
 * Replace children of a peer config by a new config
 */
static void replace_children(private_vici_config_t *this,
							 peer_cfg_t *from, peer_cfg_t *to)
{
	enumerator_t *enumerator;
	child_cfg_t *child;
	bool added;

	enumerator = to->replace_child_cfgs(to, from);
	while (enumerator->enumerate(enumerator, &child, &added))
	{
		handle_start_action(this, to, child, !added);
	}
	enumerator->destroy(enumerator);
}

/**
 * Merge/replace a peer config with existing configs
 */
static void merge_config(private_vici_config_t *this, peer_cfg_t *peer_cfg)
{
	enumerator_t *enumerator;
	peer_cfg_t *current;
	ike_cfg_t *ike_cfg;
	bool merged = FALSE;

	this->lock->write_lock(this->lock);
	while (this->handling_actions)
	{
		this->condvar->wait(this->condvar, this->lock);
	}

	enumerator = this->conns->create_enumerator(this->conns);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (streq(peer_cfg->get_name(peer_cfg), current->get_name(current)))
		{
			ike_cfg = current->get_ike_cfg(current);
			if (peer_cfg->equals(peer_cfg, current) &&
				ike_cfg->equals(ike_cfg, peer_cfg->get_ike_cfg(peer_cfg)))
			{
				DBG1(DBG_CFG, "updated vici connection: %s",
					 peer_cfg->get_name(peer_cfg));
				replace_children(this, peer_cfg, current);
				peer_cfg->destroy(peer_cfg);
			}
			else
			{
				DBG1(DBG_CFG, "replaced vici connection: %s",
					 peer_cfg->get_name(peer_cfg));
				this->conns->insert_before(this->conns, enumerator, peer_cfg);
				this->conns->remove_at(this->conns, enumerator);
				handle_start_actions(this, current, TRUE);
				handle_start_actions(this, peer_cfg, FALSE);
				current->destroy(current);
			}
			merged = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!merged)
	{
		DBG1(DBG_CFG, "added vici connection: %s", peer_cfg->get_name(peer_cfg));
		this->conns->insert_last(this->conns, peer_cfg);
		handle_start_actions(this, peer_cfg, FALSE);
	}
	this->condvar->signal(this->condvar);
	this->lock->unlock(this->lock);
}

CALLBACK(config_sn, bool,
	request_data_t *request, vici_message_t *message,
	vici_parse_context_t *ctx, char *name)
{
	peer_data_t peer = {
		.request = request,
		.local = linked_list_create(),
		.remote = linked_list_create(),
		.vips = linked_list_create(),
		.children = linked_list_create(),
		.proposals = linked_list_create(),
		.mobike = TRUE,
		.send_certreq = TRUE,
		.pull = TRUE,
		.send_cert = CERT_SEND_IF_ASKED,
		.version = IKE_ANY,
		.remote_port = IKEV2_UDP_PORT,
		.fragmentation = FRAGMENTATION_YES,
		.unique = UNIQUE_NO,
		.keyingtries = 1,
		.rekey_time = LFT_UNDEFINED,
		.reauth_time = LFT_UNDEFINED,
		.over_time = LFT_UNDEFINED,
		.rand_time = LFT_UNDEFINED,
	};
	enumerator_t *enumerator;
	peer_cfg_create_t cfg;
	peer_cfg_t *peer_cfg;
	ike_cfg_t *ike_cfg;
	child_cfg_t *child_cfg;
	auth_data_t *auth;
	proposal_t *proposal;
	host_t *host;
	char *str;

	DBG2(DBG_CFG, " conn %s:", name);

	if (!message->parse(message, ctx, peer_sn, peer_kv, peer_li, &peer))
	{
		free_peer_data(&peer);
		return FALSE;
	}

	if (peer.local->get_count(peer.local) == 0)
	{
		INIT(auth,
			.cfg = auth_cfg_create(),
		);
		peer.local->insert_last(peer.local, auth);
	}
	if (peer.remote->get_count(peer.remote) == 0)
	{
		INIT(auth,
			.cfg = auth_cfg_create(),
		);
		peer.remote->insert_last(peer.remote, auth);
	}
	if (peer.proposals->get_count(peer.proposals) == 0)
	{
		proposal = proposal_create_default(PROTO_IKE);
		if (proposal)
		{
			peer.proposals->insert_last(peer.proposals, proposal);
		}
		proposal = proposal_create_default_aead(PROTO_IKE);
		if (proposal)
		{
			peer.proposals->insert_last(peer.proposals, proposal);
		}
	}
	if (!peer.local_addrs)
	{
		peer.local_addrs = strdup("%any");
	}
	if (!peer.remote_addrs)
	{
		peer.remote_addrs = strdup("%any");
	}
	if (!peer.local_port)
	{
		peer.local_port = charon->socket->get_port(charon->socket, FALSE);
	}

	if (peer.rekey_time == LFT_UNDEFINED && peer.reauth_time == LFT_UNDEFINED)
	{
		/* apply a default rekey time if no rekey/reauth time set */
		peer.rekey_time = LFT_DEFAULT_IKE_REKEY_TIME;
		peer.reauth_time = 0;
	}
	if (peer.rekey_time == LFT_UNDEFINED)
	{
		peer.rekey_time = 0;
	}
	if (peer.reauth_time == LFT_UNDEFINED)
	{
		peer.reauth_time = 0;
	}
	if (peer.over_time == LFT_UNDEFINED)
	{
		/* default over_time to 10% of rekey/reauth time if not given */
		peer.over_time = max(peer.rekey_time, peer.reauth_time) / 10;
	}
	if (peer.rand_time == LFT_UNDEFINED)
	{
		/* default rand_time to over_time if not given, but don't make it
		 * longer than half of rekey/rauth time */
		if (peer.rekey_time && peer.reauth_time)
		{
			peer.rand_time = min(peer.rekey_time, peer.reauth_time);
		}
		else
		{
			peer.rand_time = max(peer.rekey_time, peer.reauth_time);
		}
		peer.rand_time = min(peer.over_time, peer.rand_time / 2);
	}

#ifdef ME
	if (peer.mediation && peer.mediated_by)
	{
		DBG1(DBG_CFG, "a mediation connection cannot be a mediated connection "
			 "at the same time, config discarded");
		free_peer_data(&peer);
		return FALSE;
	}
	if (peer.mediation)
	{	/* force unique connections for mediation connections */
		peer.unique = UNIQUE_REPLACE;
	}
	else if (peer.mediated_by)
	{	/* fallback to remote identity of first auth round if peer_id is not
		 * given explicitly */
		auth_cfg_t *cfg;

		if (!peer.peer_id &&
			peer.remote->get_first(peer.remote, (void**)&cfg) == SUCCESS)
		{
			peer.peer_id = cfg->get(cfg, AUTH_RULE_IDENTITY);
			if (peer.peer_id)
			{
				peer.peer_id = peer.peer_id->clone(peer.peer_id);
			}
			else
			{
				DBG1(DBG_CFG, "mediation peer missing for mediated connection, "
					 "config discarded");
				free_peer_data(&peer);
				return FALSE;
			}
		}
	}
#endif /* ME */

	log_peer_data(&peer);

	ike_cfg = ike_cfg_create(peer.version, peer.send_certreq, peer.encap,
						peer.local_addrs, peer.local_port,
						peer.remote_addrs, peer.remote_port,
						peer.fragmentation, peer.dscp);

	cfg = (peer_cfg_create_t){
		.cert_policy = peer.send_cert,
		.unique = peer.unique,
		.keyingtries = peer.keyingtries,
		.rekey_time = peer.rekey_time,
		.reauth_time = peer.reauth_time,
		.jitter_time = peer.rand_time,
		.over_time = peer.over_time,
		.no_mobike = !peer.mobike,
		.aggressive = peer.aggressive,
		.push_mode = !peer.pull,
		.dpd = peer.dpd_delay,
		.dpd_timeout = peer.dpd_timeout,
		.ppk_id = peer.ppk_id ? peer.ppk_id->clone(peer.ppk_id) : NULL,
		.ppk_required = peer.ppk_required,
	};
#ifdef ME
	cfg.mediation = peer.mediation;
	if (peer.mediated_by)
	{
		cfg.mediated_by = peer.mediated_by;
		if (peer.peer_id)
		{
			cfg.peer_id = peer.peer_id->clone(peer.peer_id);
		}
	}
#endif /* ME */
	peer_cfg = peer_cfg_create(name, ike_cfg, &cfg);

	while (peer.local->remove_first(peer.local,
									(void**)&auth) == SUCCESS)
	{
		peer_cfg->add_auth_cfg(peer_cfg, auth->cfg, TRUE);
		auth->cfg = NULL;
		free_auth_data(auth);
	}
	while (peer.remote->remove_first(peer.remote,
									 (void**)&auth) == SUCCESS)
	{
		peer_cfg->add_auth_cfg(peer_cfg, auth->cfg, FALSE);
		auth->cfg = NULL;
		free_auth_data(auth);
	}
	while (peer.children->remove_first(peer.children,
									   (void**)&child_cfg) == SUCCESS)
	{
		peer_cfg->add_child_cfg(peer_cfg, child_cfg);
	}
	while (peer.proposals->remove_first(peer.proposals,
										(void**)&proposal) == SUCCESS)
	{
		ike_cfg->add_proposal(ike_cfg, proposal);
	}
	while (peer.vips->remove_first(peer.vips, (void**)&host) == SUCCESS)
	{
		peer_cfg->add_virtual_ip(peer_cfg, host);
	}
	if (peer.pools)
	{
		enumerator = enumerator_create_token(peer.pools, ",", " ");
		while (enumerator->enumerate(enumerator, &str))
		{
			peer_cfg->add_pool(peer_cfg, str);
		}
		enumerator->destroy(enumerator);
	}

	free_peer_data(&peer);

	merge_config(request->this, peer_cfg);

	return TRUE;
}

CALLBACK(load_conn, vici_message_t*,
	private_vici_config_t *this, char *name, u_int id, vici_message_t *message)
{
	request_data_t request = {
		.this = this,
	};

	if (!message->parse(message, NULL, config_sn, NULL, NULL, &request))
	{
		if (request.reply)
		{
			return request.reply;
		}
		return create_reply("parsing request failed");
	}
	return create_reply(NULL);
}

CALLBACK(unload_conn, vici_message_t*,
	private_vici_config_t *this, char *name, u_int id, vici_message_t *message)
{
	enumerator_t *enumerator;
	peer_cfg_t *cfg;
	char *conn_name;
	bool found = FALSE;

	conn_name = message->get_str(message, NULL, "name");
	if (!conn_name)
	{
		return create_reply("unload: missing connection name");
	}

	this->lock->write_lock(this->lock);
	while (this->handling_actions)
	{
		this->condvar->wait(this->condvar, this->lock);
	}
	enumerator = this->conns->create_enumerator(this->conns);
	while (enumerator->enumerate(enumerator, &cfg))
	{
		if (streq(cfg->get_name(cfg), conn_name))
		{
			this->conns->remove_at(this->conns, enumerator);
			handle_start_actions(this, cfg, TRUE);
			cfg->destroy(cfg);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->condvar->signal(this->condvar);
	this->lock->unlock(this->lock);

	if (!found)
	{
		return create_reply("unload: connection '%s' not found", conn_name);
	}
	return create_reply(NULL);
}

CALLBACK(get_conns, vici_message_t*,
	private_vici_config_t *this, char *name, u_int id, vici_message_t *message)
{
	vici_builder_t *builder;
	enumerator_t *enumerator;
	peer_cfg_t *cfg;

	builder = vici_builder_create();
	builder->begin_list(builder, "conns");

	this->lock->read_lock(this->lock);
	enumerator = this->conns->create_enumerator(this->conns);
	while (enumerator->enumerate(enumerator, &cfg))
	{
		builder->add_li(builder, "%s", cfg->get_name(cfg));
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	builder->end_list(builder);

	return builder->finalize(builder);
}

static void manage_command(private_vici_config_t *this,
						   char *name, vici_command_cb_t cb, bool reg)
{
	this->dispatcher->manage_command(this->dispatcher, name,
									 reg ? cb : NULL, this);
}

/**
 * (Un-)register dispatcher functions
 */
static void manage_commands(private_vici_config_t *this, bool reg)
{
	manage_command(this, "load-conn", load_conn, reg);
	manage_command(this, "unload-conn", unload_conn, reg);
	manage_command(this, "get-conns", get_conns, reg);
}

METHOD(vici_config_t, destroy, void,
	private_vici_config_t *this)
{
	manage_commands(this, FALSE);
	this->conns->destroy_offset(this->conns, offsetof(peer_cfg_t, destroy));
	this->condvar->destroy(this->condvar);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
vici_config_t *vici_config_create(vici_dispatcher_t *dispatcher,
								  vici_authority_t *authority,
								  vici_cred_t *cred)
{
	private_vici_config_t *this;

	INIT(this,
		.public = {
			.backend = {
				.create_peer_cfg_enumerator = _create_peer_cfg_enumerator,
				.create_ike_cfg_enumerator = _create_ike_cfg_enumerator,
				.get_peer_cfg_by_name = _get_peer_cfg_by_name,
			},
			.destroy = _destroy,
		},
		.dispatcher = dispatcher,
		.conns = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.condvar = rwlock_condvar_create(),
		.authority = authority,
		.cred = cred,
	);

	manage_commands(this, TRUE);

	return &this->public;
}
