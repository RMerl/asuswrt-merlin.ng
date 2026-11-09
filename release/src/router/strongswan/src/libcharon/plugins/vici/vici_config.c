/*
 * Copyright (C) 2015-2024 Tobias Brunner
 * Copyright (C) 2015-2018 Andreas Steffen
 * Copyright (C) 2014 Martin Willi
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
#include <collections/hashtable.h>
#include <collections/linked_list.h>
#include <sa/ikev2/tasks/child_create.h>
#include <sa/ikev1/tasks/quick_mode.h>

#include <pubkey_cert.h>

#include <stdio.h>

/**
 *  Maximum proposal length
 */
#define MAX_PROPOSAL_LEN   2048

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
	 * Hashtable of loaded connections, as peer_cfg_t
	 */
	hashtable_t *conns;

	/**
	 * Lock for conns table
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

CALLBACK(peer_filter, bool,
	void *data, enumerator_t *orig, va_list args)
{
	peer_cfg_t **out;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, NULL, out))
	{
		return TRUE;
	}
	return FALSE;
}

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_vici_config_t *this, identification_t *me, identification_t *other)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(this->conns->create_enumerator(this->conns),
									peer_filter, this->lock,
									(void*)this->lock->unlock);
}

CALLBACK(ike_filter, bool,
	void *data, enumerator_t *orig, va_list args)
{
	peer_cfg_t *cfg;
	ike_cfg_t **out;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, NULL, &cfg))
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
	peer_cfg_t *found;

	this->lock->read_lock(this->lock);
	found = this->conns->get(this->conns, name);
	if (found)
	{
		found->get_ref(found);
	}
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
	array_t *pubkeys;
	uint32_t round;
} auth_data_t;

/**
 * Clean up auth config data
 */
static void free_auth_data(auth_data_t *data)
{
	array_destroy(data->pubkeys);
	DESTROY_IF(data->cfg);
	free(data);
}

/**
 * Data associated to a peer config
 */
typedef struct {
	request_data_t *request;
	peer_cfg_option_t options;
	uint32_t version;
	bool encap;
	bool send_certreq;
	identification_t *ppk_id;
	cert_policy_t send_cert;
	ocsp_policy_t ocsp;
	uint64_t dpd_delay;
	uint64_t dpd_timeout;
	fragmentation_t fragmentation;
	childless_t childless;
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
	uint32_t if_id_in;
	uint32_t if_id_out;
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
		certificate_t *cert;
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
				DBG2(DBG_CFG, "   eap-vendor = %N", pen_names, v.u);
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
			case AUTH_RULE_CA_IDENTITY:
				DBG2(DBG_CFG, "   ca_id = %Y", v.id);
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
			case AUTH_RULE_SUBJECT_CERT:
				DBG2(DBG_CFG, "   cert = %Y", v.cert->get_subject(v.cert));
				break;
			case AUTH_RULE_CA_CERT:
				DBG2(DBG_CFG, "   cacert = %Y", v.cert->get_subject(v.cert));
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Helper macro to check if an option flag is set
 */
#define has_opt(cfg, opt) ({ ((cfg)->options & (opt)) == (opt); })

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
	DBG2(DBG_CFG, "  ocsp = %N", ocsp_policy_names, data->ocsp);
	DBG2(DBG_CFG, "  ppk_id = %Y",  data->ppk_id);
	DBG2(DBG_CFG, "  ppk_required = %u", has_opt(data, OPT_PPK_REQUIRED));
	DBG2(DBG_CFG, "  mobike = %u", !has_opt(data, OPT_NO_MOBIKE));
	DBG2(DBG_CFG, "  aggressive = %u", has_opt(data, OPT_IKEV1_AGGRESSIVE));
	DBG2(DBG_CFG, "  pull = %u", !has_opt(data, OPT_IKEV1_PUSH_MODE));
	DBG2(DBG_CFG, "  dscp = 0x%.2x", data->dscp);
	DBG2(DBG_CFG, "  encap = %u", data->encap);
	DBG2(DBG_CFG, "  dpd_delay = %llu", data->dpd_delay);
	DBG2(DBG_CFG, "  dpd_timeout = %llu", data->dpd_timeout);
	DBG2(DBG_CFG, "  fragmentation = %u",  data->fragmentation);
	DBG2(DBG_CFG, "  childless = %u",  data->childless);
	DBG2(DBG_CFG, "  unique = %N", unique_policy_names, data->unique);
	DBG2(DBG_CFG, "  keyingtries = %u", data->keyingtries);
	DBG2(DBG_CFG, "  reauth_time = %llu", data->reauth_time);
	DBG2(DBG_CFG, "  rekey_time = %llu", data->rekey_time);
	DBG2(DBG_CFG, "  over_time = %llu", data->over_time);
	DBG2(DBG_CFG, "  rand_time = %llu", data->rand_time);
	DBG2(DBG_CFG, "  proposals = %#P", data->proposals);
	DBG2(DBG_CFG, "  if_id_in = %u", data->if_id_in);
	DBG2(DBG_CFG, "  if_id_out = %u", data->if_id_out);
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
	child_cfg_create_t *cfg DBG_UNUSED = &data->cfg;

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
	DBG2(DBG_CFG, "   hostaccess = %u", has_opt(cfg, OPT_HOSTACCESS));
	DBG2(DBG_CFG, "   ipcomp = %u", has_opt(cfg, OPT_IPCOMP));
	DBG2(DBG_CFG, "   mode = %N%s", ipsec_mode_names, cfg->mode,
		 has_opt(cfg, OPT_PROXY_MODE) ? "_PROXY" : "");
	DBG2(DBG_CFG, "   policies = %u", !has_opt(cfg, OPT_NO_POLICIES));
	DBG2(DBG_CFG, "   policies_fwd_out = %u",
		 has_opt(cfg, OPT_FWD_OUT_POLICIES));
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
	DBG2(DBG_CFG, "   if_id_in = %u", cfg->if_id_in);
	DBG2(DBG_CFG, "   if_id_out = %u", cfg->if_id_out);
	DBG2(DBG_CFG, "   mark_in = %u/%u",
		 cfg->mark_in.value, cfg->mark_in.mask);
	DBG2(DBG_CFG, "   mark_in_sa = %u", has_opt(cfg, OPT_MARK_IN_SA));
	DBG2(DBG_CFG, "   mark_out = %u/%u",
		 cfg->mark_out.value, cfg->mark_out.mask);
	DBG2(DBG_CFG, "   set_mark_in = %u/%u",
		 cfg->set_mark_in.value, cfg->set_mark_in.mask);
	DBG2(DBG_CFG, "   set_mark_out = %u/%u",
		 cfg->set_mark_out.value, cfg->set_mark_out.mask);
	DBG2(DBG_CFG, "   label = %s",
		 cfg->label ? cfg->label->get_string(cfg->label) : NULL);
	DBG2(DBG_CFG, "   label_mode = %N", sec_label_mode_names, cfg->label_mode);
	DBG2(DBG_CFG, "   inactivity = %llu", cfg->inactivity);
	DBG2(DBG_CFG, "   proposals = %#P", data->proposals);
	DBG2(DBG_CFG, "   local_ts = %#R", data->local_ts);
	DBG2(DBG_CFG, "   remote_ts = %#R", data->remote_ts);
	DBG2(DBG_CFG, "   per_cpu_sas = %s",
		 has_opt(cfg, OPT_PER_CPU_SAS_ENCAP) ? "encap" :
		 has_opt(cfg, OPT_PER_CPU_SAS) ? "1" : "0");
	DBG2(DBG_CFG, "   hw_offload = %N", hw_offload_names, cfg->hw_offload);
	DBG2(DBG_CFG, "   sha256_96 = %u", has_opt(cfg, OPT_SHA256_96));
	DBG2(DBG_CFG, "   copy_df = %u", !has_opt(cfg, OPT_NO_COPY_DF));
	DBG2(DBG_CFG, "   copy_ecn = %u", !has_opt(cfg, OPT_NO_COPY_ECN));
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
	DESTROY_IF(data->cfg.label);
	free(data->cfg.updown);
	free(data->cfg.interface);
}

/**
 * Add the default proposals for the given protocol.  We currently prefer AEAD
 * for ESP but not for IKE.
 */
static void add_default_proposals(linked_list_t *list, protocol_id_t proto)
{
	proposal_t *first, *second;

	if (proto == PROTO_IKE)
	{
		first = proposal_create_default(proto);
		second = proposal_create_default_aead(proto);
	}
	else
	{
		first = proposal_create_default_aead(proto);
		second = proposal_create_default(proto);
	}
	if (first)
	{
		list->insert_last(list, first);
	}
	if (second)
	{
		list->insert_last(list, second);
	}
}

/**
 * Common proposal parsing
 */
static bool parse_proposal(linked_list_t *list, protocol_id_t proto, chunk_t v)
{
	char buf[MAX_PROPOSAL_LEN];
	proposal_t *proposal;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	if (strcaseeq("default", buf))
	{
		add_default_proposals(list, proto);
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
		{ "iptfs",				MODE_IPTFS		},
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
 * Macro to parse an option flag, add it if parsed value is either TRUE or FALSE.
 */
#define PARSE_OPTION(out, opt, v, add_if_true) \
	bool val; \
	if (parse_bool(&val, v)) { \
		if (val == add_if_true)	{ \
			*out |= opt; \
		} \
		return TRUE; \
	} \
	return FALSE;

/**
 * Enable a peer_cfg_option_t, the flag controls whether the option is enabled
 * if the parsed value is TRUE or FALSE.
 */
static bool parse_peer_option(peer_cfg_option_t *out, peer_cfg_option_t opt,
							  chunk_t v, bool add_if_true)
{
	PARSE_OPTION(out, opt, v, add_if_true)
}

/**
 * Parse OPT_NO_MOBIKE option
 */
CALLBACK(parse_opt_mobike, bool,
	peer_cfg_option_t *out, chunk_t v)
{
	return parse_peer_option(out, OPT_NO_MOBIKE, v, FALSE);
}

/**
 * Parse OPT_IKEV1_AGGRESSIVE option
 */
CALLBACK(parse_opt_aggr, bool,
	peer_cfg_option_t *out, chunk_t v)
{
	return parse_peer_option(out, OPT_IKEV1_AGGRESSIVE, v, TRUE);
}

/**
 * Parse OPT_IKEV1_PUSH_MODE option
 */
CALLBACK(parse_opt_pull, bool,
	peer_cfg_option_t *out, chunk_t v)
{
	return parse_peer_option(out, OPT_IKEV1_PUSH_MODE, v, FALSE);
}

/**
 * Parse OPT_PPK_REQUIRED option
 */
CALLBACK(parse_opt_ppk_req, bool,
	peer_cfg_option_t *out, chunk_t v)
{
	return parse_peer_option(out, OPT_PPK_REQUIRED, v, TRUE);
}

/**
 * Enable a child_cfg_option_t, the flag controls whether the option is enabled
 * if the parsed value is TRUE or FALSE.
 */
static bool parse_option(child_cfg_option_t *out, child_cfg_option_t opt,
						 chunk_t v, bool add_if_true)
{
	PARSE_OPTION(out, opt, v, add_if_true)
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
 * Parse OPT_PER_CPU_SAS option
 */
CALLBACK(parse_opt_cpus, bool,
	child_cfg_option_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "encap",	OPT_PER_CPU_SAS|OPT_PER_CPU_SAS_ENCAP	},
	};
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out |= d;
		return TRUE;
	}
	return parse_option(out, OPT_PER_CPU_SAS, v, TRUE);
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
		{ "start",		ACTION_START	},
		{ "restart",	ACTION_START	},
		{ "route",		ACTION_TRAP		},
		{ "trap",		ACTION_TRAP		},
		{ "none",		ACTION_NONE		},
		{ "clear",		ACTION_NONE		},
	};
	char buf[BUF_LEN];
	int d;

	if (parse_map(map, countof(map), &d, v))
	{
		*out = d;
		return TRUE;
	}
	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	if (enum_flags_from_string(action_names, buf, out))
	{
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
		{ "no",		HW_OFFLOAD_NO		},
		{ "yes",	HW_OFFLOAD_CRYPTO	},
		{ "crypto",	HW_OFFLOAD_CRYPTO	},
		{ "packet",	HW_OFFLOAD_PACKET	},
		{ "auto",	HW_OFFLOAD_AUTO		},
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
	char buf[32], *end;
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
 * Parse a relative time (32-bit)
 */
CALLBACK(parse_time32, bool,
	uint32_t *out, chunk_t v)
{
	uint64_t time;

	if (parse_time(&time, v))
	{
		*out = time;
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse byte volume
 */
CALLBACK(parse_bytes, bool,
	uint64_t *out, chunk_t v)
{
	char buf[32], *end;
	unsigned long long l, ll;

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}

	l = ll = strtoull(buf, &end, 0);
	while (*end == ' ')
	{
		end++;
	}
	switch (*end)
	{
		case 'g':
		case 'G':
			ll *= 1024;
			/* fall */
		case 'm':
		case 'M':
			ll *= 1024;
			/* fall */
		case 'k':
		case 'K':
			ll *= 1024;
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
	*out = (ll < l) ? UINT64_MAX : ll;
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
 * Parse interface ID
 */
CALLBACK(parse_if_id, bool,
	uint32_t *out, chunk_t v)
{
	char buf[32];

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	return if_id_from_string(buf, out);
}

/**
 * Parse security label
 */
CALLBACK(parse_label, bool,
	sec_label_t **out, chunk_t v)
{
	char buf[BUF_LEN];

	if (!vici_stringify(v, buf, sizeof(buf)))
	{
		return FALSE;
	}
	*out = sec_label_from_string(buf);
	return *out != NULL;
}

/**
 * Parse security label mode
 */
CALLBACK(parse_label_mode, bool,
	sec_label_mode_t *out, chunk_t v)
{
	char buf[BUF_LEN];

	if (!vici_stringify(v, buf, sizeof(buf)) ||
		!sec_label_mode_from_string(buf, out))
	{
		return FALSE;
	}
	return TRUE;
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
		strpfx(buf, "ecdsa"))
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
	identification_t *id;
	char buf[BUF_LEN];

	if (!vici_stringify(v, buf, sizeof(buf)) ||
		!(id = identification_create_from_string_with_regex(buf)))
	{
		return FALSE;
	}
	cfg->add(cfg, rule, id);
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
 * Parse CA identity constraint
 */
CALLBACK(parse_ca_id, bool,
	auth_cfg_t *cfg, chunk_t v)
{
	return parse_id(cfg, AUTH_RULE_CA_IDENTITY, v);
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

	if (rule == AUTH_RULE_CA_CERT)
	{
		authority = auth->request->this->authority;
		cert = authority->add_ca_cert(authority, cert);
	}
	else
	{
		cred = auth->request->this->cred;
		cert = cred->add_cert(cred, cert);
	}
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
							  BUILD_BLOB, v, BUILD_END);
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
	/* because we don't have an identity yet, just store the blob to parse/wrap
	 * the key later */
	array_insert_create_value(&auth->pubkeys, sizeof(chunk_t),
							  ARRAY_TAIL, &v);
	return TRUE;
}

/**
 * Create raw public key certificates associated with the given identity and
 * add them to the config.
 */
static bool parse_and_add_pubkeys(auth_data_t *auth, identification_t *id)
{
	certificate_t *cert;
	chunk_t pubkey;
	bool id_usable = id && id->get_type(id) != ID_ANY;

	while (array_remove(auth->pubkeys, ARRAY_HEAD, &pubkey))
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
								  CERT_TRUSTED_PUBKEY, BUILD_BLOB, pubkey,
								  id_usable ? BUILD_SUBJECT : BUILD_END,
								  id, BUILD_END);
		if (!cert || !add_cert(auth, AUTH_RULE_SUBJECT_CERT, cert))
		{
			return FALSE;
		}
	}
	return TRUE;
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
 * Parse a childless_t
 */
CALLBACK(parse_childless, bool,
	childless_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "allow",		CHILDLESS_ALLOW		},
		{ "prefer",		CHILDLESS_PREFER	},
		{ "never",		CHILDLESS_NEVER		},
		{ "force",		CHILDLESS_FORCE		},
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
 * Parse an ocsp_policy_t
 */
CALLBACK(parse_ocsp, bool,
	ocsp_policy_t *out, chunk_t v)
{
	enum_map_t map[] = {
		{ "both",		OCSP_SEND_BOTH		},
		{ "reply",		OCSP_SEND_REPLY		},
		{ "request",	OCSP_SEND_REQUEST	},
		{ "never",		OCSP_SEND_NEVER		},
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
		{ "inactivity",			parse_time32,		&child->cfg.inactivity				},
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
		{ "if_id_in",			parse_if_id,		&child->cfg.if_id_in				},
		{ "if_id_out",			parse_if_id,		&child->cfg.if_id_out				},
		{ "label",				parse_label,		&child->cfg.label					},
		{ "label_mode",			parse_label_mode,	&child->cfg.label_mode				},
		{ "per_cpu_sas",		parse_opt_cpus,		&child->cfg.options					},
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
		{ "ca_id",			parse_ca_id,		auth->cfg					},
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
		{ "aggressive",		parse_opt_aggr,		&peer->options				},
		{ "pull",			parse_opt_pull,		&peer->options				},
		{ "dscp",			parse_dscp,			&peer->dscp					},
		{ "encap",			parse_bool,			&peer->encap				},
		{ "mobike",			parse_opt_mobike,	&peer->options				},
		{ "dpd_delay",		parse_time,			&peer->dpd_delay			},
		{ "dpd_timeout",	parse_time,			&peer->dpd_timeout			},
		{ "fragmentation",	parse_frag,			&peer->fragmentation		},
		{ "childless",		parse_childless,	&peer->childless			},
		{ "send_certreq",	parse_bool,			&peer->send_certreq			},
		{ "send_cert",		parse_send_cert,	&peer->send_cert			},
		{ "ocsp",			parse_ocsp,			&peer->ocsp					},
		{ "keyingtries",	parse_uint32,		&peer->keyingtries			},
		{ "unique",			parse_unique,		&peer->unique				},
		{ "local_port",		parse_uint32,		&peer->local_port			},
		{ "remote_port",	parse_uint32,		&peer->remote_port			},
		{ "reauth_time",	parse_time,			&peer->reauth_time			},
		{ "rekey_time",		parse_time,			&peer->rekey_time			},
		{ "over_time",		parse_time,			&peer->over_time			},
		{ "rand_time",		parse_time,			&peer->rand_time			},
		{ "ppk_id",			parse_peer_id,		&peer->ppk_id				},
		{ "ppk_required",	parse_opt_ppk_req,	&peer->options				},
		{ "if_id_in",		parse_if_id,		&peer->if_id_in				},
		{ "if_id_out",		parse_if_id,		&peer->if_id_out			},
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
	/* if no soft lifetime specified, set a default or base it on the hard lifetime */
	if (lft->time.rekey == LFT_UNDEFINED)
	{
		if (lft->time.life != LFT_UNDEFINED)
		{
			lft->time.rekey = lft->time.life / 1.1;
		}
		else
		{
			lft->time.rekey = LFT_DEFAULT_CHILD_REKEY_TIME;
		}
	}
	if (lft->bytes.rekey == LFT_UNDEFINED)
	{
		if (lft->bytes.life != LFT_UNDEFINED)
		{
			lft->bytes.rekey = lft->bytes.life / 1.1;
		}
		else
		{
			lft->bytes.rekey = LFT_DEFAULT_CHILD_REKEY_BYTES;
		}
	}
	if (lft->packets.rekey == LFT_UNDEFINED)
	{
		if (lft->packets.life != LFT_UNDEFINED)
		{
			lft->packets.rekey = lft->packets.life / 1.1;
		}
		else
		{
			lft->packets.rekey = LFT_DEFAULT_CHILD_REKEY_PACKETS;
		}
	}
	/* if no hard lifetime specified, add one at soft lifetime + 10% */
	if (lft->time.life == LFT_UNDEFINED)
	{
		lft->time.life = lft->time.rekey * 1.1;
	}
	if (lft->bytes.life == LFT_UNDEFINED)
	{
		lft->bytes.life = lft->bytes.rekey * 1.1;
	}
	if (lft->packets.life == LFT_UNDEFINED)
	{
		lft->packets.life = lft->packets.rekey * 1.1;
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
					.rekey = LFT_UNDEFINED,
					.life = LFT_UNDEFINED,
					.jitter = LFT_UNDEFINED,
				},
				.bytes = {
					.rekey = LFT_UNDEFINED,
					.life = LFT_UNDEFINED,
					.jitter = LFT_UNDEFINED,
				},
				.packets = {
					.rekey = LFT_UNDEFINED,
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
		add_default_proposals(child.proposals, PROTO_ESP);
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
		certificate_t *cert;
		identification_t *id;

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

		if (!parse_and_add_pubkeys(auth, id))
		{
			free_auth_data(auth);
			return FALSE;
		}

		if (!id)
		{
			cert = auth->cfg->get(auth->cfg, AUTH_RULE_SUBJECT_CERT);
			if (cert)
			{
				id = cert->get_subject(cert);
				DBG1(DBG_CFG, "  id not specified, defaulting to"
							  " cert subject '%Y'", id);
				auth->cfg->add(auth->cfg, AUTH_RULE_IDENTITY, id->clone(id));
			}
		}

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
	action_t action;

	action = child_cfg->get_start_action(child_cfg);

	if (action & ACTION_TRAP)
	{
		DBG1(DBG_CFG, "vici installing '%s'", child_cfg->get_name(child_cfg));
		switch (child_cfg->get_mode(child_cfg))
		{
			case MODE_PASS:
			case MODE_DROP:
				charon->shunts->install(charon->shunts,
										peer_cfg->get_name(peer_cfg), child_cfg);
				/* no need to check for ACTION_START */
				return;
			default:
				charon->traps->install(charon->traps, peer_cfg, child_cfg);
				break;
		}
	}

	if (action & ACTION_START)
	{
		DBG1(DBG_CFG, "vici initiating '%s'", child_cfg->get_name(child_cfg));
		charon->controller->initiate(charon->controller,
					peer_cfg->get_ref(peer_cfg), child_cfg->get_ref(child_cfg),
					NULL, NULL, 0, 0, FALSE);
	}
}

/**
 * Type to keep track of unique IDs and names of CHILD_SAs to terminate.
 */
typedef struct {
	uint32_t id;
	char *name;
} child_name_id_t;

/**
 * Get the child config of the given task depending on its type.
 */
static child_cfg_t *get_task_config(task_t *task, task_type_t type)
{
	child_create_t *child_create = (child_create_t*)task;

	if (type == TASK_QUICK_MODE)
	{
		quick_mode_t *quick_mode = (quick_mode_t*)task;
		return quick_mode->get_config(quick_mode);
	}
	return child_create->get_config(child_create);
}

/**
 * Abort the given child-creating task depending on its type.
 */
static void abort_child_task(task_t *task, task_type_t type)
{
	child_create_t *child_create = (child_create_t*)task;

	if (type == TASK_QUICK_MODE)
	{
		quick_mode_t *quick_mode = (quick_mode_t*)task;
		return quick_mode->abort(quick_mode);
	}
	return child_create->abort(child_create);
}

/**
 * Check if there are child-creating tasks queued that should be aborted, as
 * well as if there are any others that should prevent the deletion of the
 * IKE_SA.
 */
static void check_queued_tasks(ike_sa_t *ike_sa, hashtable_t *to_terminate,
							   bool *others)
{
	enumerator_t *enumerator;
	child_cfg_t *cfg;
	task_t *task;
	task_type_t type = TASK_CHILD_CREATE;

	if (ike_sa->get_version(ike_sa) == IKEV1)
	{
		type = TASK_QUICK_MODE;
	}

	/* if we find an active task, we can't remove it as there is no way to
	 * abort an exchange (i.e. the peer will have created the SA even if we
	 * don't process the response). so we instruct the task to immediately
	 * delete the CHILD_SA once it's created */
	enumerator = ike_sa->create_task_enumerator(ike_sa, TASK_QUEUE_ACTIVE);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == type)
		{
			cfg = get_task_config(task, type);
			if (to_terminate->get(to_terminate, cfg->get_name(cfg)))
			{
				DBG1(DBG_CFG, "vici aborting %N task for CHILD_SA '%s'",
					 task_type_names, type, cfg->get_name(cfg));
				abort_child_task(task, type);
			}
			else
			{
				*others = TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);

	/* for the queued tasks, we just remove any that use a config that is to
	 * be terminated, but also note if there are others */
	enumerator = ike_sa->create_task_enumerator(ike_sa, TASK_QUEUE_QUEUED);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == type)
		{
			cfg = get_task_config(task, type);
			if (to_terminate->get(to_terminate, cfg->get_name(cfg)))
			{
				DBG1(DBG_CFG, "vici removing %N task for CHILD_SA '%s'",
					 task_type_names, type, cfg->get_name(cfg));
				ike_sa->remove_task(ike_sa, enumerator);
				task->destroy(task);
			}
			else
			{
				*others = TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Terminate given CHILD_SAs and optionally terminate any IKE_SA without other
 * children.
 */
static void terminate_for_action(private_vici_config_t *this, char *peer_name,
								 hashtable_t *to_terminate, bool delete_ike)
{
	enumerator_t *enumerator, *children;
	child_sa_t *child_sa;
	ike_sa_t *ike_sa;
	child_name_id_t child_id;
	uint32_t id;
	array_t *ids = NULL, *ikeids = NULL;
	bool others;

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (!streq(ike_sa->get_name(ike_sa), peer_name))
		{
			continue;
		}

		others = FALSE;
		children = ike_sa->create_child_sa_enumerator(ike_sa);
		while (children->enumerate(children, &child_sa))
		{
			if (child_sa->get_state(child_sa) != CHILD_DELETING &&
				child_sa->get_state(child_sa) != CHILD_DELETED &&
				!to_terminate->get(to_terminate, child_sa->get_name(child_sa)))
			{
				others = TRUE;
				break;
			}
		}
		children->destroy(children);

		check_queued_tasks(ike_sa, to_terminate, &others);

		if (delete_ike && !others)
		{
			/* found no children/tasks or only matching, delete IKE_SA */
			id = ike_sa->get_unique_id(ike_sa);
			array_insert_create_value(&ikeids, sizeof(id),
									  ARRAY_TAIL, &id);
			continue;
		}

		/* otherwise, delete only the matching CHILD_SAs */
		children = ike_sa->create_child_sa_enumerator(ike_sa);
		while (children->enumerate(children, &child_sa))
		{
			child_id.name = child_sa->get_name(child_sa);

			if (child_sa->get_state(child_sa) != CHILD_DELETING &&
				child_sa->get_state(child_sa) != CHILD_DELETED &&
				to_terminate->get(to_terminate, child_id.name))
			{
				child_id.id = child_sa->get_unique_id(child_sa);
				child_id.name = strdup(child_id.name);
				array_insert_create_value(&ids, sizeof(child_id),
										  ARRAY_TAIL, &child_id);
			}
		}
		children->destroy(children);
	}
	enumerator->destroy(enumerator);

	while (array_remove(ids, ARRAY_HEAD, &child_id))
	{
		DBG1(DBG_CFG, "vici closing CHILD_SA '%s' #%u", child_id.name,
			 child_id.id);
		charon->controller->terminate_child(charon->controller,
											child_id.id, NULL, NULL, 0, 0);
		free(child_id.name);
	}
	array_destroy(ids);

	while (array_remove(ikeids, ARRAY_HEAD, &id))
	{
		DBG1(DBG_CFG, "vici closing IKE_SA '%s' #%u", peer_name, id);
		charon->controller->terminate_ike(charon->controller, id,
										  FALSE, NULL, NULL, 0, 0);
	}
	array_destroy(ikeids);
}

/**
 * Clear the start action associated with the given child config. To reduce the
 * overhead when terminating active SAs, only collect the name.
 *
 * Note: The lock must be unlocked when calling this.
 */
static void clear_start_action(private_vici_config_t *this, char *peer_name,
							   child_cfg_t *child_cfg, hashtable_t *to_terminate)
{
	action_t action;
	char *name;

	name = child_cfg->get_name(child_cfg);
	action = child_cfg->get_start_action(child_cfg);
	if (action & ACTION_TRAP)
	{
		DBG1(DBG_CFG, "vici uninstalling '%s'", name);
		switch (child_cfg->get_mode(child_cfg))
		{
			case MODE_PASS:
			case MODE_DROP:
				charon->shunts->uninstall(charon->shunts, peer_name, name);
				/* no need to check for ACTION_START */
				return;
			default:
				charon->traps->uninstall(charon->traps, peer_name, name);
				break;
		}
	}
	if (action & ACTION_START)
	{
		to_terminate->put(to_terminate, name, name);
	}
}

/**
 * Clear start actions associated with a list of child configs, optionally
 * deletes empty IKE_SAs.
 */
static void clear_start_actions(private_vici_config_t *this,
								peer_cfg_t *peer_cfg, array_t *child_cfgs,
								bool delete_ike)
{
	enumerator_t *enumerator;
	hashtable_t *to_terminate;
	child_cfg_t *child_cfg;
	char *peer_name;

	this->handling_actions = TRUE;
	this->lock->unlock(this->lock);

	to_terminate = hashtable_create(hashtable_hash_str,
									hashtable_equals_str, 8);
	peer_name = peer_cfg->get_name(peer_cfg);

	enumerator = array_create_enumerator(child_cfgs);
	while (enumerator->enumerate(enumerator, &child_cfg))
	{
		clear_start_action(this, peer_name, child_cfg, to_terminate);
	}
	enumerator->destroy(enumerator);

	terminate_for_action(this, peer_name, to_terminate, delete_ike);
	to_terminate->destroy(to_terminate);

	this->lock->write_lock(this->lock);
	this->handling_actions = FALSE;
}

/**
 * Run start actions associated with a list of child configs.
 */
static void run_start_actions(private_vici_config_t *this,
							  peer_cfg_t *peer_cfg, array_t *child_cfgs)
{
	enumerator_t *enumerator;
	child_cfg_t *child_cfg;

	this->handling_actions = TRUE;
	this->lock->unlock(this->lock);

	enumerator = array_create_enumerator(child_cfgs);
	while (enumerator->enumerate(enumerator, &child_cfg))
	{
		run_start_action(this, peer_cfg, child_cfg);
	}
	enumerator->destroy(enumerator);

	this->lock->write_lock(this->lock);
	this->handling_actions = FALSE;
}

/**
 * Run or undo start actions for all child configs of the given peer config
 * after it has changed.
 */
static void handle_start_actions(private_vici_config_t *this,
								 peer_cfg_t *peer_cfg, bool undo)
{
	array_t *child_cfgs;

	child_cfgs = array_create(0, 0);
	array_insert_enumerator(child_cfgs, ARRAY_TAIL,
							peer_cfg->create_child_cfg_enumerator(peer_cfg));
	if (undo)
	{
		/* the peer config has changed, so allow IKE_SAs to get terminated */
		clear_start_actions(this, peer_cfg, child_cfgs, TRUE);
	}
	else
	{
		run_start_actions(this, peer_cfg, child_cfgs);
	}
	array_destroy(child_cfgs);
}

/**
 * Replace children of a peer config by a new config
 */
static void replace_children(private_vici_config_t *this,
							 peer_cfg_t *from, peer_cfg_t *to)
{
	enumerator_t *enumerator;
	child_cfg_t *child;
	array_t *to_run = NULL, *to_clear = NULL;
	bool added, any_to_initiate = FALSE;

	enumerator = to->replace_child_cfgs(to, from);
	while (enumerator->enumerate(enumerator, &child, &added))
	{
		if (added)
		{
			array_insert_create(&to_run, ARRAY_TAIL, child);

			if (child->get_start_action(child) & ACTION_START)
			{
				any_to_initiate = TRUE;
			}
		}
		else
		{
			array_insert_create(&to_clear, ARRAY_TAIL, child);
		}
	}
	/* keep empty IKE_SAs only if we are to initiate any CHILD_SAs */
	clear_start_actions(this, to, to_clear, !any_to_initiate);
	run_start_actions(this, to, to_run);
	array_destroy(to_clear);
	array_destroy(to_run);
	enumerator->destroy(enumerator);
}

/**
 * Merge/replace a peer config with existing configs
 */
static void merge_config(private_vici_config_t *this, peer_cfg_t *peer_cfg)
{
	peer_cfg_t *found;
	ike_cfg_t *ike_cfg;

	this->lock->write_lock(this->lock);
	while (this->handling_actions)
	{
		this->condvar->wait(this->condvar, this->lock);
	}

	found = this->conns->get(this->conns, peer_cfg->get_name(peer_cfg));
	if (found)
	{
		ike_cfg = found->get_ike_cfg(found);
		if (peer_cfg->equals(peer_cfg, found) &&
			ike_cfg->equals(ike_cfg, peer_cfg->get_ike_cfg(peer_cfg)))
		{
			DBG1(DBG_CFG, "updated vici connection: %s",
				 peer_cfg->get_name(peer_cfg));
			replace_children(this, peer_cfg, found);
			peer_cfg->destroy(peer_cfg);
		}
		else
		{
			DBG1(DBG_CFG, "replaced vici connection: %s",
				 peer_cfg->get_name(peer_cfg));
			this->conns->put(this->conns, peer_cfg->get_name(peer_cfg),
							 peer_cfg);
			handle_start_actions(this, found, TRUE);
			handle_start_actions(this, peer_cfg, FALSE);
			found->destroy(found);
		}
	}
	else
	{
		DBG1(DBG_CFG, "added vici connection: %s", peer_cfg->get_name(peer_cfg));
		this->conns->put(this->conns, peer_cfg->get_name(peer_cfg), peer_cfg);
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
		.send_certreq = TRUE,
		.send_cert = CERT_SEND_IF_ASKED,
		.ocsp = OCSP_SEND_REPLY,
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
	ike_cfg_create_t ike;
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
		add_default_proposals(peer.proposals, PROTO_IKE);
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
		request->reply = create_reply("a mediation connection cannot be a "
									  "mediated connection at the same time");
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
		auth_data_t *auth;

		if (!peer.peer_id &&
			peer.remote->get_first(peer.remote, (void**)&auth) == SUCCESS)
		{
			peer.peer_id = auth->cfg->get(auth->cfg, AUTH_RULE_IDENTITY);
			if (peer.peer_id)
			{
				peer.peer_id = peer.peer_id->clone(peer.peer_id);
			}
		}
		if (!peer.peer_id)
		{
			request->reply = create_reply("mediation peer or remote identity "
										  "missing for mediated connection");
			free_peer_data(&peer);
			return FALSE;
		}
	}
#endif /* ME */

	log_peer_data(&peer);

	ike = (ike_cfg_create_t){
		.version = peer.version,
		.local = peer.local_addrs,
		.local_port = peer.local_port,
		.remote = peer.remote_addrs,
		.remote_port = peer.remote_port,
		.no_certreq = !peer.send_certreq,
		.ocsp_certreq = peer.ocsp == OCSP_SEND_BOTH ||
						peer.ocsp == OCSP_SEND_REQUEST,
		.force_encap = peer.encap,
		.fragmentation = peer.fragmentation,
		.childless = peer.childless,
		.dscp = peer.dscp,
	};
	ike_cfg = ike_cfg_create(&ike);

	cfg = (peer_cfg_create_t){
		.options = peer.options,
		.cert_policy = peer.send_cert,
		.ocsp_policy = peer.ocsp,
		.unique = peer.unique,
		.keyingtries = peer.keyingtries,
		.rekey_time = peer.rekey_time,
		.reauth_time = peer.reauth_time,
		.jitter_time = peer.rand_time,
		.over_time = peer.over_time,
		.dpd = peer.dpd_delay,
		.dpd_timeout = peer.dpd_timeout,
		.ppk_id = peer.ppk_id ? peer.ppk_id->clone(peer.ppk_id) : NULL,
		.if_id_in = peer.if_id_in,
		.if_id_out = peer.if_id_out,
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
	peer_cfg_t *cfg;
	char *conn_name;

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
	cfg = this->conns->remove(this->conns, conn_name);
	if (cfg)
	{
		DBG1(DBG_CFG, "removed vici connection: %s", cfg->get_name(cfg));
		handle_start_actions(this, cfg, TRUE);
		cfg->destroy(cfg);
	}
	this->condvar->signal(this->condvar);
	this->lock->unlock(this->lock);

	if (!cfg)
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
	while (enumerator->enumerate(enumerator, NULL, &cfg))
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

CALLBACK(destroy_conn, void,
	peer_cfg_t *cfg, const void *key)
{
	cfg->destroy(cfg);
}

METHOD(vici_config_t, destroy, void,
	private_vici_config_t *this)
{
	manage_commands(this, FALSE);
	this->conns->destroy_function(this->conns, destroy_conn);
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
		.conns = hashtable_create(hashtable_hash_str, hashtable_equals_str, 32),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.condvar = rwlock_condvar_create(),
		.authority = authority,
		.cred = cred,
	);

	manage_commands(this, TRUE);

	return &this->public;
}
