/*
 * Copyright (C) 2015-2017 Tobias Brunner
 * Copyright (C) 2015-2018 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
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

#include "vici_query.h"
#include "vici_builder.h"
#include "vici_cert_info.h"

#include <inttypes.h>
#include <time.h>
#ifndef WIN32
#include <sys/utsname.h>
#endif
#ifdef HAVE_MALLINFO
#include <malloc.h>
#endif

#include <daemon.h>
#include <asn1/asn1.h>
#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>
#include <counters_query.h>

ENUM(vici_counter_type_names,
	COUNTER_INIT_IKE_SA_REKEY, COUNTER_OUT_INFORMATIONAL_RSP,
	"ike-rekey-init",
	"ike-rekey-resp",
	"child-rekey",
	"invalid",
	"invalid-spi",
	"ike-init-in-req",
	"ike-init-in-resp",
	"ike-init-out-req",
	"ike-init-out-resp",
	"ike-auth-in-req",
	"ike-auth-in-resp",
	"ike-auth-out-req",
	"ike-auth-out-resp",
	"create-child-in-req",
	"create-child-in-resp",
	"create-child-out-req",
	"create-child-out-resp",
	"info-in-req",
	"info-in-resp",
	"info-out-req",
	"info-out-resp",
);

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
	 * Query interface for counters
	 */
	counters_query_t *counters;

	/**
	 * Daemon startup timestamp
	 */
	time_t uptime;
};

static void add_mark(vici_builder_t *b, mark_t mark,
					 char *label, char *mask_label)
{
	if (mark.value | mark.mask)
	{
		b->add_kv(b, label, "%.8x", mark.value);
		if (~mark.mask)
		{
			b->add_kv(b, mask_label, "%.8x", mark.mask);
		}
	}
}

/**
 * List the mode of a CHILD_SA or config
 */
static void list_mode(vici_builder_t *b, child_sa_t *child, child_cfg_t *cfg)
{
	ipsec_mode_t mode;
	char *sub_mode = "";

	if (child || cfg)
	{
		if (!cfg)
		{
			cfg = child->get_config(child);
		}
		mode = child ? child->get_mode(child) : cfg->get_mode(cfg);
		if (mode == MODE_TRANSPORT && cfg->has_option(cfg, OPT_PROXY_MODE))
		{	/* only report this if the negotiated mode is actually TRANSPORT */
			sub_mode = "_PROXY";
		}
		b->add_kv(b, "mode", "%N%s", ipsec_mode_names, mode, sub_mode);
	}
}

/**
 * List details of a CHILD_SA
 */
static void list_child(private_vici_query_t *this, vici_builder_t *b,
					   child_sa_t *child, time_t now)
{
	time_t t;
	uint64_t bytes, packets;
	uint16_t alg, ks;
	proposal_t *proposal;
	enumerator_t *enumerator;
	traffic_selector_t *ts;

	b->add_kv(b, "name", "%s", child->get_name(child));
	b->add_kv(b, "uniqueid", "%u", child->get_unique_id(child));
	b->add_kv(b, "reqid", "%u", child->get_reqid(child));
	b->add_kv(b, "state", "%N", child_sa_state_names, child->get_state(child));
	list_mode(b, child, NULL);
	if (child->get_state(child) == CHILD_INSTALLED ||
		child->get_state(child) == CHILD_REKEYING ||
		child->get_state(child) == CHILD_REKEYED)
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
		add_mark(b, child->get_mark(child, TRUE), "mark-in", "mark-mask-in");
		add_mark(b, child->get_mark(child, FALSE), "mark-out", "mark-mask-out");
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
										&alg, &ks) && alg != AUTH_UNDEFINED)
			{
				b->add_kv(b, "integ-alg", "%N", integrity_algorithm_names, alg);
				if (ks)
				{
					b->add_kv(b, "integ-keysize", "%u", ks);
				}
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
			b->add_kv(b, "use-in", "%"PRIu64, (uint64_t)(now - t));
		}

		child->get_usestats(child, FALSE, &t, &bytes, &packets);
		b->add_kv(b, "bytes-out", "%"PRIu64, bytes);
		b->add_kv(b, "packets-out", "%"PRIu64, packets);
		if (t)
		{
			b->add_kv(b, "use-out", "%"PRIu64, (uint64_t)(now - t));
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
 * Add an IKE_SA condition to the given builder
 */
static void add_condition(vici_builder_t *b, ike_sa_t *ike_sa,
						  char *key, ike_condition_t cond)
{
	if (ike_sa->has_condition(ike_sa, cond))
	{
		b->add_kv(b, key, "yes");
	}
}

/**
 * List virtual IPs
 */
static void list_vips(private_vici_query_t *this, vici_builder_t *b,
					  ike_sa_t *ike_sa, bool local, char *name)
{
	enumerator_t *enumerator;
	bool has = FALSE;
	host_t *vip;

	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, local);
	while (enumerator->enumerate(enumerator, &vip))
	{
		if (!has)
		{
			b->begin_list(b, name);
			has = TRUE;
		}
		b->add_li(b, "%H", vip);
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
	uint16_t alg, ks;
	host_t *host;

	b->add_kv(b, "uniqueid", "%u", ike_sa->get_unique_id(ike_sa));
	b->add_kv(b, "version", "%u", ike_sa->get_version(ike_sa));
	b->add_kv(b, "state", "%N", ike_sa_state_names, ike_sa->get_state(ike_sa));

	host = ike_sa->get_my_host(ike_sa);
	b->add_kv(b, "local-host", "%H", host);
	b->add_kv(b, "local-port", "%d", host->get_port(host));
	b->add_kv(b, "local-id", "%Y", ike_sa->get_my_id(ike_sa));

	host = ike_sa->get_other_host(ike_sa);
	b->add_kv(b, "remote-host", "%H", host);
	b->add_kv(b, "remote-port", "%d", host->get_port(host));
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
	b->add_kv(b, "initiator-spi", "%.16"PRIx64,
			  be64toh(id->get_initiator_spi(id)));
	b->add_kv(b, "responder-spi", "%.16"PRIx64,
			  be64toh(id->get_responder_spi(id)));

	add_condition(b, ike_sa, "nat-local", COND_NAT_HERE);
	add_condition(b, ike_sa, "nat-remote", COND_NAT_THERE);
	add_condition(b, ike_sa, "nat-fake", COND_NAT_FAKE);
	add_condition(b, ike_sa, "nat-any", COND_NAT_ANY);

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
	add_condition(b, ike_sa, "ppk", COND_PPK);

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

	list_vips(this, b, ike_sa, TRUE, "local-vips");
	list_vips(this, b, ike_sa, FALSE, "remote-vips");

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
	char buf[BUF_LEN];

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
			snprintf(buf, sizeof(buf), "%s-%u", child_sa->get_name(child_sa),
					 child_sa->get_unique_id(child_sa));
			b->begin_section(b, buf);
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
static void raise_policy(private_vici_query_t *this, u_int id, char *ike,
						 child_sa_t *child)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	vici_builder_t *b;
	char buf[BUF_LEN];

	b = vici_builder_create();
	snprintf(buf, sizeof(buf), "%s/%s", ike, child->get_name(child));
	b->begin_section(b, buf);
	b->add_kv(b, "child", "%s", child->get_name(child));
	b->add_kv(b, "ike", "%s", ike);

	list_mode(b, child, NULL);

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
static void raise_policy_cfg(private_vici_query_t *this, u_int id, char *ike,
							 child_cfg_t *cfg)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	traffic_selector_t *ts;
	vici_builder_t *b;
	char buf[BUF_LEN];

	b = vici_builder_create();
	snprintf(buf, sizeof(buf), "%s%s%s", ike ? ike : "", ike ? "/" : "",
			 cfg->get_name(cfg));
	b->begin_section(b, buf);
	b->add_kv(b, "child", "%s", cfg->get_name(cfg));
	if (ike)
	{
		b->add_kv(b, "ike", "%s", ike);
	}

	list_mode(b, NULL, cfg);

	b->begin_list(b, "local-ts");
	list = cfg->get_traffic_selectors(cfg, TRUE, NULL, NULL, FALSE);
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &ts))
	{
		b->add_li(b, "%R", ts);
	}
	enumerator->destroy(enumerator);
	list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
	b->end_list(b /* local-ts */);

	b->begin_list(b, "remote-ts");
	list = cfg->get_traffic_selectors(cfg, FALSE, NULL, NULL, FALSE);
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
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	bool drop, pass, trap;
	char *child, *ike, *ns;

	drop = request->get_str(request, NULL, "drop") != NULL;
	pass = request->get_str(request, NULL, "pass") != NULL;
	trap = request->get_str(request, NULL, "trap") != NULL;
	child = request->get_str(request, NULL, "child");
	ike = request->get_str(request, NULL, "ike");

	if (trap)
	{
		enumerator = charon->traps->create_enumerator(charon->traps);
		while (enumerator->enumerate(enumerator, &peer_cfg, &child_sa))
		{
			if ((ike && !streq(ike, peer_cfg->get_name(peer_cfg))) ||
				(child && !streq(child, child_sa->get_name(child_sa))))
			{
				continue;
			}
			raise_policy(this, id, peer_cfg->get_name(peer_cfg), child_sa);
		}
		enumerator->destroy(enumerator);
	}

	if (drop || pass)
	{
		enumerator = charon->shunts->create_enumerator(charon->shunts);
		while (enumerator->enumerate(enumerator, &ns, &child_cfg))
		{
			if ((ike && !streq(ike, ns)) ||
				(child && !streq(child, child_cfg->get_name(child_cfg))))
			{
				continue;
			}
			switch (child_cfg->get_mode(child_cfg))
			{
				case MODE_DROP:
					if (drop)
					{
						raise_policy_cfg(this, id, ns, child_cfg);
					}
					break;
				case MODE_PASS:
					if (pass)
					{
						raise_policy_cfg(this, id, ns, child_cfg);
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
	char buf[32];
	int i = 0;

	enumerator = peer_cfg->create_auth_cfg_enumerator(peer_cfg, local);
	while (enumerator->enumerate(enumerator, &auth))
	{
		snprintf(buf, sizeof(buf), "%s-%d", local ? "local" : "remote", ++i);
		b->begin_section(b, buf);

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

		b->begin_list(b, "cert_policy");
		rules = auth->create_enumerator(auth);
		while (rules->enumerate(rules, &rule, &v))
		{
			if (rule == AUTH_RULE_CERT_POLICY)
			{
				b->add_li(b, "%s", v.str);
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
	char *ike, *str, *interface;
	uint32_t manual_prio, dpd_delay, dpd_timeout;
	identification_t *ppk_id;
	linked_list_t *list;
	traffic_selector_t *ts;
	lifetime_cfg_t *lft;
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
		b->add_kv(b, "reauth_time", "%u",
			peer_cfg->get_reauth_time(peer_cfg, FALSE));
		b->add_kv(b, "rekey_time", "%u",
			peer_cfg->get_rekey_time(peer_cfg, FALSE));
		b->add_kv(b, "unique", "%N", unique_policy_names,
			peer_cfg->get_unique_policy(peer_cfg));

		dpd_delay = peer_cfg->get_dpd(peer_cfg);
		if (dpd_delay)
		{
			b->add_kv(b, "dpd_delay", "%u", dpd_delay);
		}

		dpd_timeout = peer_cfg->get_dpd_timeout(peer_cfg);
		if (dpd_timeout)
		{
			b->add_kv(b, "dpd_timeout", "%u", dpd_timeout);
		}

		ppk_id = peer_cfg->get_ppk_id(peer_cfg);
		if (ppk_id)
		{
			b->add_kv(b, "ppk_id", "%Y", ppk_id);
		}
		if (peer_cfg->ppk_required(peer_cfg))
		{
			b->add_kv(b, "ppk_required", "yes");
		}

		build_auth_cfgs(peer_cfg, TRUE, b);
		build_auth_cfgs(peer_cfg, FALSE, b);

		b->begin_section(b, "children");

		children = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		while (children->enumerate(children, &child_cfg))
		{
			b->begin_section(b, child_cfg->get_name(child_cfg));

			list_mode(b, NULL, child_cfg);

			lft = child_cfg->get_lifetime(child_cfg, FALSE);
			b->add_kv(b, "rekey_time",    "%"PRIu64, lft->time.rekey);
			b->add_kv(b, "rekey_bytes",   "%"PRIu64, lft->bytes.rekey);
			b->add_kv(b, "rekey_packets", "%"PRIu64, lft->packets.rekey);
			free(lft);

			b->add_kv(b, "dpd_action", "%N", action_names,
					  child_cfg->get_dpd_action(child_cfg));
			b->add_kv(b, "close_action", "%N", action_names,
					  child_cfg->get_close_action(child_cfg));

			b->begin_list(b, "local-ts");
			list = child_cfg->get_traffic_selectors(child_cfg, TRUE, NULL,
													NULL, FALSE);
			selectors = list->create_enumerator(list);
			while (selectors->enumerate(selectors, &ts))
			{
				b->add_li(b, "%R", ts);
			}
			selectors->destroy(selectors);
			list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
			b->end_list(b /* local-ts */);

			b->begin_list(b, "remote-ts");
			list = child_cfg->get_traffic_selectors(child_cfg, FALSE, NULL,
													NULL, FALSE);
			selectors = list->create_enumerator(list);
			while (selectors->enumerate(selectors, &ts))
			{
				b->add_li(b, "%R", ts);
			}
			selectors->destroy(selectors);
			list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
			b->end_list(b /* remote-ts */);

			interface = child_cfg->get_interface(child_cfg);
			if (interface)
			{
				b->add_kv(b, "interface", "%s", interface);
			}

			manual_prio = child_cfg->get_manual_prio(child_cfg);
			if (manual_prio)
			{
				b->add_kv(b, "priority", "%u", manual_prio);
			}

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
static bool has_privkey(certificate_t *cert)
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

/**
 * Store cert filter data
 */
typedef struct {
	certificate_type_t type;
	x509_flag_t flag;
	identification_t *subject;
} cert_filter_t;

/**
 * Enumerate all X.509 certificates with a given flag
 */
static void enum_x509(private_vici_query_t *this, u_int id,
					  linked_list_t *certs, cert_filter_t *filter,
					  x509_flag_t flag)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	vici_builder_t *b;
	chunk_t encoding;
	x509_t *x509;

	if (filter->type != CERT_ANY && filter->flag != X509_ANY &&
		filter->flag != flag)
	{
		return;
	}

	enumerator = certs->create_enumerator(certs);
	while (enumerator->enumerate(enumerator, &cert))
	{
		x509 = (x509_t*)cert;
		if ((x509->get_flags(x509) & X509_ANY) != flag)
		{
			continue;
		}

		if (cert->get_encoding(cert, CERT_ASN1_DER, &encoding))
		{
			b = vici_builder_create();
			b->add_kv(b, "type", "%N", certificate_type_names, CERT_X509);
			b->add_kv(b, "flag", "%N", x509_flag_names, flag);
			if (has_privkey(cert))
			{
				b->add_kv(b, "has_privkey", "yes");
			}
			b->add(b, VICI_KEY_VALUE, "data", encoding);
			free(encoding.ptr);

			this->dispatcher->raise_event(this->dispatcher, "list-cert", id,
										  b->finalize(b));
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Enumerate all non-X.509 certificate types
 */
static void enum_others(private_vici_query_t *this, u_int id,
						linked_list_t *certs, certificate_type_t type)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	vici_builder_t *b;
	chunk_t encoding, t_ch;
	cred_encoding_type_t encoding_type;
	identification_t *subject;
	time_t not_before, not_after;

	encoding_type = (type == CERT_TRUSTED_PUBKEY) ? PUBKEY_SPKI_ASN1_DER :
													CERT_ASN1_DER;

	enumerator = certs->create_enumerator(certs);
	while (enumerator->enumerate(enumerator, &cert))
	{
		if (cert->get_encoding(cert, encoding_type, &encoding))
		{
			b = vici_builder_create();
			b->add_kv(b, "type", "%N", certificate_type_names, type);
			if (has_privkey(cert))
			{
				b->add_kv(b, "has_privkey", "yes");
			}
			b->add(b, VICI_KEY_VALUE, "data", encoding);
			free(encoding.ptr);

			if (type == CERT_TRUSTED_PUBKEY)
			{
				subject = cert->get_subject(cert);
				if (subject->get_type(subject) != ID_KEY_ID)
				{
					b->add_kv(b, "subject", "%Y", cert->get_subject(cert));
				}
				cert->get_validity(cert, NULL, &not_before, &not_after);
				if (not_before != UNDEFINED_TIME)
				{
					t_ch = asn1_from_time(&not_before, ASN1_GENERALIZEDTIME);
					b->add(b, VICI_KEY_VALUE, "not-before", chunk_skip(t_ch, 2));
					chunk_free(&t_ch);
				}
				if (not_after != UNDEFINED_TIME)
				{
					t_ch = asn1_from_time(&not_after, ASN1_GENERALIZEDTIME);
					b->add(b, VICI_KEY_VALUE, "not-after", chunk_skip(t_ch, 2));
					chunk_free(&t_ch);
				}
			}
			this->dispatcher->raise_event(this->dispatcher, "list-cert", id,
										  b->finalize(b));
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Enumerate all certificates of a given type
 */
static void enum_certs(private_vici_query_t *this,	u_int id,
					   cert_filter_t *filter, certificate_type_t type)
{
	enumerator_t *e1, *e2;
	certificate_t *cert, *current;
	linked_list_t *certs;
	bool found;

	if (filter->type != CERT_ANY && filter->type != type)
	{
		return;
	}
	certs = linked_list_create();

	e1 = lib->credmgr->create_cert_enumerator(lib->credmgr, type, KEY_ANY,
											  filter->subject, FALSE);
	while (e1->enumerate(e1, &cert))
	{
		found = FALSE;

		e2 = certs->create_enumerator(certs);
		while (e2->enumerate(e2, &current))
		{
			if (current->equals(current, cert))
			{
				found = TRUE;
				break;
			}
		}
		e2->destroy(e2);

		if (!found)
		{
			certs->insert_last(certs, cert->get_ref(cert));
		}
	}
	e1->destroy(e1);

	if (type == CERT_X509)
	{
		enum_x509(this, id, certs, filter, X509_NONE);
		enum_x509(this, id, certs, filter, X509_CA);
		enum_x509(this, id, certs, filter, X509_AA);
		enum_x509(this, id, certs, filter, X509_OCSP_SIGNER);
	}
	else
	{
		enum_others(this, id, certs, type);
	}
	certs->destroy_offset(certs, offsetof(certificate_t, destroy));
}

CALLBACK(list_certs, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	cert_filter_t filter = {
		.type = CERT_ANY,
		.flag = X509_ANY,
		.subject = NULL
	};
	vici_builder_t *b;
	char *str;

	str = request->get_str(request, "ANY", "type");
	if (enum_from_name(certificate_type_names, str, &filter.type))
	{
		if (filter.type == CERT_X509)
		{
			str = request->get_str(request, "ANY", "flag");
			if (!enum_from_name(x509_flag_names, str, &filter.flag))
			{
				DBG1(DBG_CFG, "invalid certificate flag '%s'", str);
				goto finalize;
			}
		}
	}
	else if	(!vici_cert_info_from_str(str, &filter.type, &filter.flag))
	{
		DBG1(DBG_CFG, "invalid certificate type '%s'", str);
		goto finalize;
	}

	str = request->get_str(request, NULL, "subject");
	if (str)
	{
		filter.subject = identification_create_from_string(str);
	}

	enum_certs(this, id, &filter, CERT_TRUSTED_PUBKEY);
	enum_certs(this, id, &filter, CERT_X509);
	enum_certs(this, id, &filter, CERT_X509_AC);
	enum_certs(this, id, &filter, CERT_X509_CRL);
	enum_certs(this, id, &filter, CERT_X509_OCSP_RESPONSE);
	DESTROY_IF(filter.subject);

finalize:
	b = vici_builder_create();
	return b->finalize(b);
}

/**
 * Add a key/value pair of ALG => plugin
 */
static void add_algorithm(vici_builder_t *b, enum_name_t *alg_names,
						  int alg_type, const char *plugin_name)
{
	char alg_name[BUF_LEN];

	sprintf(alg_name, "%N", alg_names, alg_type);
	b->add_kv(b, alg_name, (char*)plugin_name);
}

CALLBACK(get_algorithms, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	vici_builder_t *b;
	enumerator_t *enumerator;
	encryption_algorithm_t encryption;
	integrity_algorithm_t integrity;
	hash_algorithm_t hash;
	pseudo_random_function_t prf;
	ext_out_function_t xof;
	diffie_hellman_group_t group;
	rng_quality_t quality;
	const char *plugin_name;

	b = vici_builder_create();

	b->begin_section(b, "encryption");
	enumerator = lib->crypto->create_crypter_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
	{
		add_algorithm(b, encryption_algorithm_names, encryption, plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	b->begin_section(b, "integrity");
	enumerator = lib->crypto->create_signer_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &integrity, &plugin_name))
	{
		add_algorithm(b, integrity_algorithm_names, integrity, plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	b->begin_section(b, "aead");
	enumerator = lib->crypto->create_aead_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
	{
		add_algorithm(b, encryption_algorithm_names, encryption, plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	b->begin_section(b, "hasher");
	enumerator = lib->crypto->create_hasher_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &hash, &plugin_name))
	{
		add_algorithm(b, hash_algorithm_names, hash, plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	b->begin_section(b, "prf");
	enumerator = lib->crypto->create_prf_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &prf, &plugin_name))
	{
		add_algorithm(b, pseudo_random_function_names, prf, plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	b->begin_section(b, "xof");
	enumerator = lib->crypto->create_xof_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &xof, &plugin_name))
	{
		add_algorithm(b, ext_out_function_names, xof, plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	b->begin_section(b, "dh");
	enumerator = lib->crypto->create_dh_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &group, &plugin_name))
	{
		add_algorithm(b, diffie_hellman_group_names, group, plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	b->begin_section(b, "rng");
	enumerator = lib->crypto->create_rng_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &quality, &plugin_name))
	{
		add_algorithm(b, rng_quality_names, quality, plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	b->begin_section(b, "nonce-gen");
	enumerator = lib->crypto->create_nonce_gen_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &plugin_name))
	{
		b->add_kv(b, "NONCE_GEN", (char*)plugin_name);
	}
	enumerator->destroy(enumerator);
	b->end_section(b);

	return b->finalize(b);
}

/**
 * Make sure we have the counters query interface
 */
static inline bool ensure_counters(private_vici_query_t *this)
{
	if (this->counters)
	{
		return TRUE;
	}
	return (this->counters = lib->get(lib, "counters")) != NULL;
}

/**
 * Add a single set of counters to the message
 *
 * Frees the array of counter values
 */
static void add_counters(vici_builder_t *b, char *name, uint64_t *counters)
{
	char buf[BUF_LEN];
	counter_type_t i;

	b->begin_section(b, name ?: "");
	for (i = 0; i < COUNTER_MAX; i++)
	{
		snprintf(buf, sizeof(buf), "%N", vici_counter_type_names, i);
		b->add_kv(b, buf, "%"PRIu64, counters[i]);
	}
	b->end_section(b);
	free(counters);
}

CALLBACK(get_counters, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	vici_builder_t *b;
	enumerator_t *enumerator;
	uint64_t *counters;
	char *conn, *errmsg = NULL;
	bool all;

	b = vici_builder_create();

	if (ensure_counters(this))
	{
		conn = request->get_str(request, NULL, "name");
		all = request->get_bool(request, FALSE, "all");

		b->begin_section(b, "counters");
		if (all)
		{
			enumerator = this->counters->get_names(this->counters);
			while (enumerator->enumerate(enumerator, &conn))
			{
				counters = this->counters->get_all(this->counters, conn);
				if (counters)
				{
					add_counters(b, conn, counters);
				}
			}
			enumerator->destroy(enumerator);
		}
		else
		{
			counters = this->counters->get_all(this->counters, conn);
			if (counters)
			{
				add_counters(b, conn, counters);
			}
			else
			{
				errmsg = "no counters found for this connection";
			}
		}
		b->end_section(b);
	}
	else
	{
		errmsg = "no counters available (plugin missing?)";
	}

	b->add_kv(b, "success", errmsg ? "no" : "yes");
	if (errmsg)
	{
		b->add_kv(b, "errmsg", "%s", errmsg);
	}
	return b->finalize(b);
}

CALLBACK(reset_counters, vici_message_t*,
	private_vici_query_t *this, char *name, u_int id, vici_message_t *request)
{
	vici_builder_t *b;
	char *conn, *errmsg = NULL;
	bool all;

	b = vici_builder_create();

	if (ensure_counters(this))
	{
		conn = request->get_str(request, NULL, "name");
		all = request->get_bool(request, FALSE, "all");

		if (all)
		{
			this->counters->reset_all(this->counters);
		}
		else
		{
			this->counters->reset(this->counters, conn);
		}
	}
	else
	{
		errmsg = "no counters available (plugin missing?)";
	}

	b->add_kv(b, "success", errmsg ? "no" : "yes");
	if (errmsg)
	{
		b->add_kv(b, "errmsg", "%s", errmsg);
	}
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
													NULL, FALSE));
	b->end_section(b);

	b->begin_list(b, "plugins");
	enumerator = lib->plugins->create_plugin_enumerator(lib->plugins);
	while (enumerator->enumerate(enumerator, &plugin, NULL))
	{
		b->add_li(b, "%s", plugin->get_name(plugin));
	}
	enumerator->destroy(enumerator);
	b->end_list(b);

#ifdef WIN32
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
		b->add_kv(b, "sbrk", "%u", mi.arena);
		b->add_kv(b, "mmap", "%u", mi.hblkhd);
		b->add_kv(b, "used", "%u", mi.uordblks);
		b->add_kv(b, "free", "%u", mi.fordblks);
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
	this->dispatcher->manage_event(this->dispatcher, "ike-updown", reg);
	this->dispatcher->manage_event(this->dispatcher, "ike-rekey", reg);
	this->dispatcher->manage_event(this->dispatcher, "child-updown", reg);
	this->dispatcher->manage_event(this->dispatcher, "child-rekey", reg);
	manage_command(this, "list-sas", list_sas, reg);
	manage_command(this, "list-policies", list_policies, reg);
	manage_command(this, "list-conns", list_conns, reg);
	manage_command(this, "list-certs", list_certs, reg);
	manage_command(this, "get-algorithms", get_algorithms, reg);
	manage_command(this, "get-counters", get_counters, reg);
	manage_command(this, "reset-counters", reset_counters, reg);
	manage_command(this, "version", version, reg);
	manage_command(this, "stats", stats, reg);
}

METHOD(listener_t, ike_updown, bool,
	private_vici_query_t *this, ike_sa_t *ike_sa, bool up)
{
	vici_builder_t *b;
	time_t now;

	if (!this->dispatcher->has_event_listeners(this->dispatcher, "ike-updown"))
	{
		return TRUE;
	}

	now = time_monotonic(NULL);

	b = vici_builder_create();

	if (up)
	{
		b->add_kv(b, "up", "yes");
	}

	b->begin_section(b, ike_sa->get_name(ike_sa));
	list_ike(this, b, ike_sa, now);
	b->end_section(b);

	this->dispatcher->raise_event(this->dispatcher,
								  "ike-updown", 0, b->finalize(b));

	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_vici_query_t *this, ike_sa_t *old, ike_sa_t *new)
{
	vici_builder_t *b;
	time_t now;

	if (!this->dispatcher->has_event_listeners(this->dispatcher, "ike-rekey"))
	{
		return TRUE;
	}

	now = time_monotonic(NULL);

	b = vici_builder_create();
	b->begin_section(b, old->get_name(old));
	b->begin_section(b, "old");
	list_ike(this, b, old, now);
	b->end_section(b);
	b->begin_section(b, "new");
	list_ike(this, b, new, now);
	b->end_section(b);
	b->end_section(b);

	this->dispatcher->raise_event(this->dispatcher,
								  "ike-rekey", 0, b->finalize(b));

	return TRUE;
}

METHOD(listener_t, child_updown, bool,
	private_vici_query_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa, bool up)
{
	vici_builder_t *b;
	time_t now;

	if (!this->dispatcher->has_event_listeners(this->dispatcher, "child-updown"))
	{
		return TRUE;
	}

	now = time_monotonic(NULL);
	b = vici_builder_create();

	if (up)
	{
		b->add_kv(b, "up", "yes");
	}

	b->begin_section(b, ike_sa->get_name(ike_sa));
	list_ike(this, b, ike_sa, now);
	b->begin_section(b, "child-sas");

	b->begin_section(b, child_sa->get_name(child_sa));
	list_child(this, b, child_sa, now);
	b->end_section(b);

	b->end_section(b);
	b->end_section(b);

	this->dispatcher->raise_event(this->dispatcher,
								  "child-updown", 0, b->finalize(b));

	return TRUE;
}

METHOD(listener_t, child_rekey, bool,
	private_vici_query_t *this, ike_sa_t *ike_sa, child_sa_t *old,
	child_sa_t *new)
{
	vici_builder_t *b;
	time_t now;

	if (!this->dispatcher->has_event_listeners(this->dispatcher, "child-rekey"))
	{
		return TRUE;
	}

	now = time_monotonic(NULL);
	b = vici_builder_create();

	b->begin_section(b, ike_sa->get_name(ike_sa));
	list_ike(this, b, ike_sa, now);
	b->begin_section(b, "child-sas");

	b->begin_section(b, old->get_name(old));

	b->begin_section(b, "old");
	list_child(this, b, old, now);
	b->end_section(b);
	b->begin_section(b, "new");
	list_child(this, b, new, now);
	b->end_section(b);

	b->end_section(b);

	b->end_section(b);
	b->end_section(b);

	this->dispatcher->raise_event(this->dispatcher,
								  "child-rekey", 0, b->finalize(b));

	return TRUE;
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
			.listener = {
				.ike_updown = _ike_updown,
				.ike_rekey = _ike_rekey,
				.child_updown = _child_updown,
				.child_rekey = _child_rekey,
			},
			.destroy = _destroy,
		},
		.dispatcher = dispatcher,
		.uptime = time_monotonic(NULL),
	);

	manage_commands(this, TRUE);

	return &this->public;
}
