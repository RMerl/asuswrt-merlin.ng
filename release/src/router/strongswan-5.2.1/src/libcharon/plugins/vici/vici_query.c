/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include "vici_query.h"
#include "vici_builder.h"

#include <inttypes.h>
#include <time.h>
#ifndef WIN32
#include <sys/utsname.h>
#endif
#ifdef HAVE_MALLINFO
#include <malloc.h>
#endif

#include <daemon.h>

typedef struct private_vici_query_t private_vici_query_t;

/**
 * Private data of an vici_query_t object.
 */
struct private_vici_query_t {

	/**
	 * Public vici_query_t interface.
	 */
	vici_query_t public;

	/**
	 * Dispatcher
	 */
	vici_dispatcher_t *dispatcher;

	/**
	 * Daemon startup timestamp
	 */
	time_t uptime;
};

/**
 * List details of a CHILD_SA
 */
static void list_child(private_vici_query_t *this, vici_builder_t *b,
					   child_sa_t *child, time_t now)
{
	time_t t;
	u_int64_t bytes, packets;
	u_int16_t alg, ks;
	proposal_t *proposal;
	enumerator_t *enumerator;
	traffic_selector_t *ts;

	b->add_kv(b, "reqid", "%u", child->get_reqid(child));
	b->add_kv(b, "state", "%N", child_sa_state_names, child->get_state(child));
	b->add_kv(b, "mode", "%N", ipsec_mode_names, child->get_mode(child));
	if (child->get_state(child) == CHILD_INSTALLED ||
		child->get_state(child) == CHILD_REKEYING)
	{
		b->add_kv(b, "protocol", "%N", protocol_id_names,
				  child->get_protocol(child));
		if (child->has_encap(child))
		{
			b->add_kv(b, "encap", "yes");
		}
		b->add_kv(b, "spi-in", "%.8x", ntohl(child->get_spi(child, TRUE)));
		b->add_kv(b, "spi-out", "%.8x", ntohl(child->get_spi(child, FALSE)));

		if (child->get_ipcomp(child) != IPCOMP_NONE)
		{
			b->add_kv(b, "cpi-in", "%.4x", ntohs(child->get_cpi(child, TRUE)));
			b->add_kv(b, "cpi-out", "%.4x", ntohs(child->get_cpi(child, FALSE)));
		}
		proposal = child->get_proposal(child);
		if (proposal)
		{
			if (proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM,
										&alg, &ks) && alg != ENCR_UNDEFINED)
			{
				b->add_kv(b, "encr-alg", "%N", encryption_algorithm_names, alg);
				if (ks)
				{
					b->add_kv(b, "encr-keysize", "%u", ks);
				}
			}
			if (proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM,
										&alg, &ks) && alg != ENCR_UNDEFINED)
			{
				b->add_kv(b, "integ-alg", "%N", integrity_algorithm_names, alg);
				if (ks)
				{
					b->add_kv(b, "integ-keysize", "%u", ks);
				}
			}
			if (proposal->get_algorithm(proposal, PSEUDO_RANDOM_FUNCTION,
										&alg, NULL))
			{
				b->add_kv(b, "prf-alg", "%N", pseudo_random_function_names, alg);
			}
			if (proposal->get_algorithm(proposal, DIFFIE_HELLMAN_GROUP,
										&alg, NULL))
			{
				b->add_kv(b, "dh-group", "%N", diffie_hellman_group_names, alg);
			}
			if (proposal->get_algorithm(proposal, EXTENDED_SEQUENCE_NUMBERS,
										&alg, NULL) && alg == EXT_SEQ_NUMBERS)
			{
				b->add_kv(b, "esn", "1");
			}
		}

		child->get_usestats(child, TRUE,  &t, &bytes, &packets);
		b->add_kv(b, "bytes-in", "%" PRIu64, bytes);
		b->add_kv(b, "packets-in", "%" PRIu64, packets);
		if (t)
		{
			b->add_kv(b, "use-in", "%"PRIu64, (u_int64_t)(now - t));
		}

		child->get_usestats(child, FALSE, &t, &bytes, &packets);
		b->add_kv(b, "bytes-out", "%"PRIu64, bytes);
		b->add_kv(b, "packets-out", "%"PRIu64, packets);
		if (t)
		{
			b->add_kv(b, "use-out", "%"PRIu64, (u_int64_t)(now - t));
		}

		t = child->get_lifetime(child, FALSE);
		if (t)
		{
			b->add_kv(b, "rekey-time", "%"PRId64, (int64_t)(t - now));
		}
		t = child->get_lifetime(child, TRUE);
		if (t)
		{
			b->add_kv(b, "life-time", "%"PRId64, (int64_t)(t - now));
		}
		t = child->get_installtime(child);
		b->add_kv(b, "install-time", "%"PRId64, (int64_t)(now - t));
	}

	b->begin_list(b, "local-ts");
	enumerator = child->create_ts_enumerator(child, TRUE);
	while (enumerator->enumerate(enumerator, &ts))
	{
		b->add_li(b, "%R", ts);
	}
	enumerator->destroy(enumerator);
	b->end_list(b /* local-ts */);

	b->begin_list(b, "remote-ts");
	enumerator = child->create_ts_enumerator(child, FALSE);
	while (enumerator->enumerate(enumerator, &ts))
	{
		b->add_li(b, "%R", ts);
	}
	enumerator->destroy(enumerator);
	b->end_list(b /* remote-ts */);
}

/**
 * List tasks in a specific queue
 */
static void list_task_queue(private_vici_query_t *this, vici_builder_t *b,
							ike_sa_t *ike_sa, task_queue_t q, char *name)
{
	enumerator_t *enumerator;
	bool has = FALSE;
	task_t *task;

	enumerator = ike_sa->create_task_enumerator(ike_sa, q);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (!has)
		{
			b->begin_list(b, name);
			has = TRUE;
		}
		b->add_li(b, "%N", task_type_names, task->get_type(task));
	}
	enumerator->destroy(enumerator);
	if (has)
	{
		b->end_list(b);
	}
}

/**
 * List details of an IKE_SA
 */
static void list_ike(private_vici_query_t *this, vici_builder_t *b,
					 ike_sa_t *ike_sa, time_t now)
{
	time_t t;
	ike_sa_id_t *id;
	identification_t *eap;
	proposal_t *proposal;
	u_int16_t alg, ks;

	b->add_kv(b, "uniqueid", "%u", ike_sa->get_unique_id(ike_sa));
	b->add_kv(b, "version", "%u", ike_sa->get_version(ike_sa));
	b->add_kv(b, "state", "%N", ike_sa_state_names, ike_sa->get_state(ike_sa));

	b->add_kv(b, "local-host", "%H", ike_sa->get_my_host(ike_sa));
	b->add_kv(b, "local-id", "%Y", ike_sa->get_my_id(ike_sa));

	b->add_kv(b, "remote-host", "%H", ike_sa->get_other_host(ike_sa));
	b->add_kv(b, "remote-id", "%Y", ike_sa->get_other_id(ike_sa));

	eap = ike_sa->get_other_eap_id(ike_sa);

	if (!eap->equals(eap, ike_sa->get_other_id(ike_sa)))
	{
		if (ike_sa->get_version(ike_sa) == IKEV1)
		{
			b->add_kv(b, "remote-xauth-id", "%Y", eap);
		}
		else
		{
			b->add_kv(b, "remote-eap-id", "%Y", eap);
		}
	}

	id = ike_sa->get_id(ike_sa);
	if (id->is_initiator(id))
	{
		b->add_kv(b, "initiator", "yes");
	}
	b->add_kv(b, "initiator-spi", "%.16"PRIx64, id->get_initiator_spi(id));
	b->add_kv(b, "responder-spi", "%.16"PRIx64, id->get_responder_spi(id));

	proposal = ike_sa->get_proposal(ike_sa);
	if (proposal)
	{
		if (proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &alg, &ks))
		{
			b->add_kv(b, "encr-alg", "%N", encryption_algorithm_names, alg);
			if (ks)
			{
				b->add_kv(b, "encr-keysize", "%u", ks);
			}
		}
		if (proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM, &alg, &ks))
		{
			b->add_kv(b, "integ-alg", "%N", integrity_algorithm_names, alg);
			if (ks)
			{
				b->add_kv(b, "integ-keysize", "%u", ks);
			}
		}
		if (proposal->get_algorithm(proposal, PSEUDO_RANDOM_FUNCTION, &alg, NULL))
		{
			b->add_kv(b, "prf-alg", "%N", pseudo_random_function_names, alg);
		}
		if (proposal->get_algorithm(proposal, DIFFIE_HELLMAN_GROUP, &alg, NULL))
		{
			b->add_kv(b, "dh-group", "%N", diffie_hellman_group_names, alg);
		}
	}

	if (ike_sa->get_state(ike_sa) == IKE_ESTABLISHED)
	{
		t = ike_sa->get_statistic(ike_sa, STAT_ESTABLISHED);
		b->add_kv(b, "established", "%"PRId64, (int64_t)(now - t));
		t = ike_sa->get_statistic(ike_sa, STAT_REKEY);
		if (t)
		{
			b->add_kv(b, "rekey-time", "%"PRId64, (int64_t)(t - now));
		}
		t = ike_sa->get_statistic(ike_sa, STAT_REAUTH);
		if (t)
		{
			b->add_kv(b, "reauth-time", "%"PRId64, (int64_t)(t - now));
		}
	}

	list_task_queue(this, b, ike_sa, TASK_QUEUE_QUEUED, "tasks-queued");
	list_task_queue(this, b, ike_sa, TASK_QUEUE_ACTIVE, "tasks-active");
	list_task_queue(this, b, ike_sa, TASK_QUEUE_PASSIVE, "tasks-passive");
}

CALLBACK(list_sas, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	vici_builder_t *b;
	enumerator_t *isas, *csas;
	ike_sa_t *ike_sa;
	child_sa_t *child_sa;
	time_t now;
	char *ike;
	u_int ike_id;
	bool bl;

	bl = request->get_str(request, NULL, "noblock") == NULL;
	ike = request->get_str(request, NULL, "ike");
	ike_id = request->get_int(request, 0, "ike-id");

	isas = charon->controller->create_ike_sa_enumerator(charon->controller, bl);
	while (isas->enumerate(isas, &ike_sa))
	{
		if (ike && !streq(ike, ike_sa->get_name(ike_sa)))
		{
			continue;
		}
		if (ike_id && ike_id != ike_sa->get_unique_id(ike_sa))
		{
			continue;
		}

		now = time_monotonic(NULL);

		b = vici_builder_create();
		b->begin_section(b, ike_sa->get_name(ike_sa));

		list_ike(this, b, ike_sa, now);

		b->begin_section(b, "child-sas");
		csas = ike_sa->create_child_sa_enumerator(ike_sa);
		while (csas->enumerate(csas, &child_sa))
		{
			b->begin_section(b, child_sa->get_name(child_sa));
			list_child(this, b, child_sa, now);
			b->end_section(b);
		}
		csas->destroy(csas);
		b->end_section(b /* child-sas */ );

		b->end_section(b);

		this->dispatcher->raise_event(this->dispatcher, "list-sa", id,
									  b->finalize(b));
	}
	isas->destroy(isas);

	b = vici_builder_create();
	return b->finalize(b);
}

/**
 * Raise a list-policy event for given CHILD_SA
 */
static void raise_policy(private_vici_query_t *this, u_int id, child_sa_t *child)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	vici_builder_t *b;

	b = vici_builder_create();
	b->begin_section(b, child->get_name(child));

	b->add_kv(b, "mode", "%N", ipsec_mode_names, child->get_mode(child));

	b->begin_list(b, "local-ts");
	enumerator = child->create_ts_enumerator(child, TRUE);
	while (enumerator->enumerate(enumerator, &ts))
	{
		b->add_li(b, "%R", ts);
	}
	enumerator->destroy(enumerator);
	b->end_list(b /* local-ts */);

	b->begin_list(b, "remote-ts");
	enumerator = child->create_ts_enumerator(child, FALSE);
	while (enumerator->enumerate(enumerator, &ts))
	{
		b->add_li(b, "%R", ts);
	}
	enumerator->destroy(enumerator);
	b->end_list(b /* remote-ts */);

	b->end_section(b);

	this->dispatcher->raise_event(this->dispatcher, "list-policy", id,
								  b->finalize(b));
}

/**
 * Raise a list-policy event for given CHILD_SA config
 */
static void raise_policy_cfg(private_vici_query_t *this, u_int id,
							 child_cfg_t *cfg)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	traffic_selector_t *ts;
	vici_builder_t *b;

	b = vici_builder_create();
	b->begin_section(b, cfg->get_name(cfg));

	b->add_kv(b, "mode", "%N", ipsec_mode_names, cfg->get_mode(cfg));

	b->begin_list(b, "local-ts");
	list = cfg->get_traffic_selectors(cfg, TRUE, NULL, NULL);
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &ts))
	{
		b->add_li(b, "%R", ts);
	}
	enumerator->destroy(enumerator);
	list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
	b->end_list(b /* local-ts */);

	b->begin_list(b, "remote-ts");
	list = cfg->get_traffic_selectors(cfg, FALSE, NULL, NULL);
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &ts))
	{
		b->add_li(b, "%R", ts);
	}
	enumerator->destroy(enumerator);
	list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
	b->end_list(b /* remote-ts */);

	b->end_section(b);

	this->dispatcher->raise_event(this->dispatcher, "list-policy", id,
								  b->finalize(b));
}

CALLBACK(list_policies, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	enumerator_t *enumerator;
	vici_builder_t *b;
	child_sa_t *child_sa;
	child_cfg_t *child_cfg;
	bool drop, pass, trap;
	char *child;

	drop = request->get_str(request, NULL, "drop") != NULL;
	pass = request->get_str(request, NULL, "pass") != NULL;
	trap = request->get_str(request, NULL, "trap") != NULL;
	child = request->get_str(request, NULL, "child");

	if (trap)
	{
		enumerator = charon->traps->create_enumerator(charon->traps);
		while (enumerator->enumerate(enumerator, NULL, &child_sa))
		{
			if (child && !streq(child, child_sa->get_name(child_sa)))
			{
				continue;
			}
			raise_policy(this, id, child_sa);
		}
		enumerator->destroy(enumerator);
	}

	if (drop || pass)
	{
		enumerator = charon->shunts->create_enumerator(charon->shunts);
		while (enumerator->enumerate(enumerator, &child_cfg))
		{
			if (child && !streq(child, child_cfg->get_name(child_cfg)))
			{
				continue;
			}
			switch (child_cfg->get_mode(child_cfg))
			{
				case MODE_DROP:
					if (drop)
					{
						raise_policy_cfg(this, id, child_cfg);
					}
					break;
				case MODE_PASS:
					if (pass)
					{
						raise_policy_cfg(this, id, child_cfg);
					}
					break;
				default:
					break;
			}
		}
		enumerator->destroy(enumerator);
	}

	b = vici_builder_create();
	return b->finalize(b);
}

/**
 * Build sections for auth configs, local or remote
 */
static void build_auth_cfgs(peer_cfg_t *peer_cfg, bool local, vici_builder_t *b)
{
	enumerator_t *enumerator, *rules;
	auth_rule_t rule;
	auth_cfg_t *auth;
	union {
		uintptr_t u;
		identification_t *id;
		certificate_t *cert;
		char *str;
	} v;

	enumerator = peer_cfg->create_auth_cfg_enumerator(peer_cfg, local);
	while (enumerator->enumerate(enumerator, &auth))
	{
		b->begin_section(b, local ? "local" : "remote");

		rules = auth->create_enumerator(auth);
		while (rules->enumerate(rules, &rule, &v))
		{
			switch (rule)
			{
				case AUTH_RULE_AUTH_CLASS:
					b->add_kv(b, "class", "%N", auth_class_names, v.u);
					break;
				case AUTH_RULE_EAP_TYPE:
					b->add_kv(b, "eap-type", "%N", eap_type_names, v.u);
					break;
				case AUTH_RULE_EAP_VENDOR:
					b->add_kv(b, "eap-vendor", "%u", v.u);
					break;
				case AUTH_RULE_XAUTH_BACKEND:
					b->add_kv(b, "xauth", "%s", v.str);
					break;
				case AUTH_RULE_CRL_VALIDATION:
					b->add_kv(b, "revocation", "%N", cert_validation_names, v.u);
					break;
				case AUTH_RULE_IDENTITY:
					b->add_kv(b, "id", "%Y", v.id);
					break;
				case AUTH_RULE_AAA_IDENTITY:
					b->add_kv(b, "aaa_id", "%Y", v.id);
					break;
				case AUTH_RULE_EAP_IDENTITY:
					b->add_kv(b, "eap_id", "%Y", v.id);
					break;
				case AUTH_RULE_XAUTH_IDENTITY:
					b->add_kv(b, "xauth_id", "%Y", v.id);
					break;
				default:
					break;
			}
		}
		rules->destroy(rules);

		b->begin_list(b, "groups");
		rules = auth->create_enumerator(auth);
		while (rules->enumerate(rules, &rule, &v))
		{
			if (rule == AUTH_RULE_GROUP)
			{
				b->add_li(b, "%Y", v.id);
			}
		}
		rules->destroy(rules);
		b->end_list(b);

		b->begin_list(b, "certs");
		rules = auth->create_enumerator(auth);
		while (rules->enumerate(rules, &rule, &v))
		{
			if (rule == AUTH_RULE_SUBJECT_CERT)
			{
				b->add_li(b, "%Y", v.cert->get_subject(v.cert));
			}
		}
		rules->destroy(rules);
		b->end_list(b);

		b->begin_list(b, "cacerts");
		rules = auth->create_enumerator(auth);
		while (rules->enumerate(rules, &rule, &v))
		{
			if (rule == AUTH_RULE_CA_CERT)
			{
				b->add_li(b, "%Y", v.cert->get_subject(v.cert));
			}
		}
		rules->destroy(rules);
		b->end_list(b);

		b->end_section(b);
	}
	enumerator->destroy(enumerator);
}

CALLBACK(list_conns, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	enumerator_t *enumerator, *tokens, *selectors, *children;
	peer_cfg_t *peer_cfg;
	ike_cfg_t *ike_cfg;
	child_cfg_t *child_cfg;
	char *ike, *str;
	linked_list_t *list;
	traffic_selector_t *ts;
	vici_builder_t *b;

	ike = request->get_str(request, NULL, "ike");

	enumerator = charon->backends->create_peer_cfg_enumerator(charon->backends,
											NULL, NULL, NULL, NULL, IKE_ANY);
	while (enumerator->enumerate(enumerator, &peer_cfg))
	{
		if (ike && !streq(ike, peer_cfg->get_name(peer_cfg)))
		{
			continue;
		}

		b = vici_builder_create();
		b->begin_section(b, peer_cfg->get_name(peer_cfg));

		ike_cfg = peer_cfg->get_ike_cfg(peer_cfg);

		b->begin_list(b, "local_addrs");
		str = ike_cfg->get_my_addr(ike_cfg);
		tokens = enumerator_create_token(str, ",", " ");
		while (tokens->enumerate(tokens, &str))
		{
			b->add_li(b, "%s", str);
		}
		tokens->destroy(tokens);
		b->end_list(b);

		b->begin_list(b, "remote_addrs");
		str = ike_cfg->get_other_addr(ike_cfg);
		tokens = enumerator_create_token(str, ",", " ");
		while (tokens->enumerate(tokens, &str))
		{
			b->add_li(b, "%s", str);
		}
		tokens->destroy(tokens);
		b->end_list(b);

		b->add_kv(b, "version", "%N", ike_version_names,
			peer_cfg->get_ike_version(peer_cfg));

		build_auth_cfgs(peer_cfg, TRUE, b);
		build_auth_cfgs(peer_cfg, FALSE, b);

		b->begin_section(b, "children");

		children = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		while (children->enumerate(children, &child_cfg))
		{
			b->begin_section(b, child_cfg->get_name(child_cfg));

			b->add_kv(b, "mode", "%N", ipsec_mode_names,
				child_cfg->get_mode(child_cfg));

			b->begin_list(b, "local-ts");
			list = child_cfg->get_traffic_selectors(child_cfg, TRUE, NULL, NULL);
			selectors = list->create_enumerator(list);
			while (selectors->enumerate(selectors, &ts))
			{
				b->add_li(b, "%R", ts);
			}
			selectors->destroy(selectors);
			list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
			b->end_list(b /* local-ts */);

			b->begin_list(b, "remote-ts");
			list = child_cfg->get_traffic_selectors(child_cfg, FALSE, NULL, NULL);
			selectors = list->create_enumerator(list);
			while (selectors->enumerate(selectors, &ts))
			{
				b->add_li(b, "%R", ts);
			}
			selectors->destroy(selectors);
			list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
			b->end_list(b /* remote-ts */);

			b->end_section(b);
		}
		children->destroy(children);

		b->end_section(b); /* children */

		b->end_section(b); /* name */

		this->dispatcher->raise_event(this->dispatcher, "list-conn", id,
									  b->finalize(b));
	}
	enumerator->destroy(enumerator);

	b = vici_builder_create();
	return b->finalize(b);
}

/**
 * Do we have a private key for given certificate
 */
static bool has_privkey(private_vici_query_t *this, certificate_t *cert)
{
	private_key_t *private;
	public_key_t *public;
	identification_t *keyid;
	chunk_t chunk;
	bool found = FALSE;

	public = cert->get_public_key(cert);
	if (public)
	{
		if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &chunk))
		{
			keyid = identification_create_from_encoding(ID_KEY_ID, chunk);
			private = lib->credmgr->get_private(lib->credmgr,
								public->get_type(public), keyid, NULL);
			if (private)
			{
				found = TRUE;
				private->destroy(private);
			}
			keyid->destroy(keyid);
		}
		public->destroy(public);
	}
	return found;
}

CALLBACK(list_certs, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	enumerator_t *enumerator, *added;
	linked_list_t *list;
	certificate_t *cert, *current;
	chunk_t encoding;
	identification_t *subject = NULL;
	int type;
	vici_builder_t *b;
	bool found;
	char *str;

	str = request->get_str(request, "ANY", "type");
	if (!enum_from_name(certificate_type_names, str, &type))
	{
		b = vici_builder_create();
		return b->finalize(b);
	}
	str = request->get_str(request, NULL, "subject");
	if (str)
	{
		subject = identification_create_from_string(str);
	}

	list = linked_list_create();
	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
												type, KEY_ANY, subject, FALSE);
	while (enumerator->enumerate(enumerator, &cert))
	{
		found = FALSE;
		added = list->create_enumerator(list);
		while (added->enumerate(added, &current))
		{
			if (current->equals(current, cert))
			{
				found = TRUE;
				break;
			}
		}
		added->destroy(added);

		if (!found && cert->get_encoding(cert, CERT_ASN1_DER, &encoding))
		{
			b = vici_builder_create();
			b->add_kv(b, "type", "%N",
					  certificate_type_names, cert->get_type(cert));
			if (has_privkey(this, cert))
			{
				b->add_kv(b, "has_privkey", "yes");
			}
			b->add(b, VICI_KEY_VALUE, "data", encoding);
			free(encoding.ptr);

			this->dispatcher->raise_event(this->dispatcher, "list-cert", id,
										  b->finalize(b));
			list->insert_last(list, cert->get_ref(cert));
		}
	}
	enumerator->destroy(enumerator);

	list->destroy_offset(list, offsetof(certificate_t, destroy));
	DESTROY_IF(subject);

	b = vici_builder_create();
	return b->finalize(b);
}

CALLBACK(version, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	vici_builder_t *b;

	b = vici_builder_create();

	b->add_kv(b, "daemon", "%s", lib->ns);
	b->add_kv(b, "version", "%s", VERSION);

#ifdef WIN32
	{
		OSVERSIONINFOEX osvie;

		memset(&osvie, 0, sizeof(osvie));
		osvie.dwOSVersionInfoSize = sizeof(osvie);

		if (GetVersionEx((LPOSVERSIONINFO)&osvie))
		{
			b->add_kv(b, "sysname", "Windows %s",
				osvie.wProductType == VER_NT_WORKSTATION ? "Client" : "Server");
			b->add_kv(b, "release", "%d.%d.%d (SP %d.%d)",
				osvie.dwMajorVersion, osvie.dwMinorVersion, osvie.dwBuildNumber,
				osvie.wServicePackMajor, osvie.wServicePackMinor);
			b->add_kv(b, "machine", "%s",
#ifdef WIN64
				"x86_64");
#else
				"x86");
#endif /* !WIN64 */
		}
	}
#else /* !WIN32 */
	{
		struct utsname utsname;

		if (uname(&utsname) == 0)
		{
			b->add_kv(b, "sysname", "%s", utsname.sysname);
			b->add_kv(b, "release", "%s", utsname.release);
			b->add_kv(b, "machine", "%s", utsname.machine);
		}
	}
#endif /* !WIN32 */
	return b->finalize(b);
}

/**
 * Callback function for memusage summary
 */
CALLBACK(sum_usage, void,
	vici_builder_t *b, int count, size_t bytes, int whitelisted)
{
	b->begin_section(b, "mem");
	b->add_kv(b, "total", "%zu", bytes);
	b->add_kv(b, "allocs", "%d", count);
	b->end_section(b);
}

CALLBACK(stats, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	vici_builder_t *b;
	enumerator_t *enumerator;
	plugin_t *plugin;
	time_t since, now;
	int i;

	b = vici_builder_create();

	now = time_monotonic(NULL);
	since = time(NULL) - (now - this->uptime);

	b->begin_section(b, "uptime");
	b->add_kv(b, "running", "%V", &now, &this->uptime);
	b->add_kv(b, "since", "%T", &since, FALSE);
	b->end_section(b);

	b->begin_section(b, "workers");
	b->add_kv(b, "total", "%d",
		lib->processor->get_total_threads(lib->processor));
	b->add_kv(b, "idle", "%d",
		lib->processor->get_idle_threads(lib->processor));
	b->begin_section(b, "active");
	for (i = 0; i < JOB_PRIO_MAX; i++)
	{
		b->add_kv(b, enum_to_name(job_priority_names, i), "%d",
			lib->processor->get_working_threads(lib->processor, i));
	}
	b->end_section(b);
	b->end_section(b);

	b->begin_section(b, "queues");
	for (i = 0; i < JOB_PRIO_MAX; i++)
	{
		b->add_kv(b, enum_to_name(job_priority_names, i), "%d",
			lib->processor->get_job_load(lib->processor, i));
	}
	b->end_section(b);

	b->add_kv(b, "scheduled", "%d",
		lib->scheduler->get_job_load(lib->scheduler));

	b->begin_section(b, "ikesas");
	b->add_kv(b, "total", "%u",
		charon->ike_sa_manager->get_count(charon->ike_sa_manager));
	b->add_kv(b, "half-open", "%u",
		charon->ike_sa_manager->get_half_open_count(charon->ike_sa_manager,
													NULL));
	b->end_section(b);

	b->begin_list(b, "plugins");
	enumerator = lib->plugins->create_plugin_enumerator(lib->plugins);
	while (enumerator->enumerate(enumerator, &plugin, NULL))
	{
		b->add_li(b, "%s", plugin->get_name(plugin));
	}
	enumerator->destroy(enumerator);
	b->end_list(b);

	if (lib->leak_detective)
	{
		lib->leak_detective->usage(lib->leak_detective, NULL, sum_usage, b);
	}
#ifdef WIN32
	else
	{
		DWORD lasterr = ERROR_INVALID_HANDLE;
		HANDLE heaps[32];
		int i, count;
		char buf[16];
		size_t total = 0;
		int allocs = 0;

		b->begin_section(b, "mem");
		count = GetProcessHeaps(countof(heaps), heaps);
		for (i = 0; i < count; i++)
		{
			PROCESS_HEAP_ENTRY entry = {};
			size_t heap_total = 0;
			int heap_allocs = 0;

			if (HeapLock(heaps[i]))
			{
				while (HeapWalk(heaps[i], &entry))
				{
					if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
					{
						heap_total += entry.cbData;
						heap_allocs++;
					}
				}
				lasterr = GetLastError();
				HeapUnlock(heaps[i]);
			}
			if (lasterr != ERROR_NO_MORE_ITEMS)
			{
				break;
			}
			snprintf(buf, sizeof(buf), "heap-%d", i);
			b->begin_section(b, buf);
			b->add_kv(b, "total", "%zu", heap_total);
			b->add_kv(b, "allocs", "%d", heap_allocs);
			b->end_section(b);

			total += heap_total;
			allocs += heap_allocs;
		}
		if (lasterr == ERROR_NO_MORE_ITEMS)
		{
			b->add_kv(b, "total", "%zu", total);
			b->add_kv(b, "allocs", "%d", allocs);
		}
		b->end_section(b);
	}
#endif

#ifdef HAVE_MALLINFO
	{
		struct mallinfo mi = mallinfo();

		b->begin_section(b, "mallinfo");
		b->add_kv(b, "sbrk", "%d", mi.arena);
		b->add_kv(b, "mmap", "%d", mi.hblkhd);
		b->add_kv(b, "used", "%d", mi.uordblks);
		b->add_kv(b, "free", "%d", mi.fordblks);
		b->end_section(b);
	}
#endif /* HAVE_MALLINFO */

	return b->finalize(b);
}

static void manage_command(private_vici_query_t *this,
						   char *name, vici_command_cb_t cb, bool reg)
{
	this->dispatcher->manage_command(this->dispatcher, name,
									 reg ? cb : NULL, this);
}

/**
 * (Un-)register dispatcher functions
 */
static void manage_commands(private_vici_query_t *this, bool reg)
{
	this->dispatcher->manage_event(this->dispatcher, "list-sa", reg);
	this->dispatcher->manage_event(this->dispatcher, "list-policy", reg);
	this->dispatcher->manage_event(this->dispatcher, "list-conn", reg);
	this->dispatcher->manage_event(this->dispatcher, "list-cert", reg);
	manage_command(this, "list-sas", list_sas, reg);
	manage_command(this, "list-policies", list_policies, reg);
	manage_command(this, "list-conns", list_conns, reg);
	manage_command(this, "list-certs", list_certs, reg);
	manage_command(this, "version", version, reg);
	manage_command(this, "stats", stats, reg);
}

METHOD(vici_query_t, destroy, void,
	private_vici_query_t *this)
{
	manage_commands(this, FALSE);
	free(this);
}

/**
 * See header
 */
vici_query_t *vici_query_create(vici_dispatcher_t *dispatcher)
{
	private_vici_query_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.dispatcher = dispatcher,
		.uptime = time_monotonic(NULL),
	);

	manage_commands(this, TRUE);

	return &this->public;
}
