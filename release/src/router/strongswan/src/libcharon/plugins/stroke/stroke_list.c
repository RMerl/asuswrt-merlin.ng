/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2015 Andreas Steffen
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

#include "stroke_list.h"

#include <inttypes.h>
#include <time.h>
#include <sys/utsname.h>

#if defined(HAVE_MALLINFO2) || defined (HAVE_MALLINFO)
#include <malloc.h>
#endif

#include <daemon.h>
#include <collections/linked_list.h>
#include <plugins/plugin.h>
#include <credentials/certificates/x509.h>
#include <credentials/certificates/certificate_printer.h>
#include <config/peer_cfg.h>

typedef struct private_stroke_list_t private_stroke_list_t;

/**
 * private data of stroke_list
 */
struct private_stroke_list_t {

	/**
	 * public functions
	 */
	stroke_list_t public;

	/**
	 * Kind of *swan we run
	 */
	char *swan;

	/**
	 * timestamp of daemon start
	 */
	time_t uptime;

	/**
	 * strokes attribute provider
	 */
	stroke_attribute_t *attribute;
};

/**
 * Static certificate printer object
 */
static certificate_printer_t *cert_printer = NULL;

/**
 * Log tasks of a specific queue to out
 */
static void log_task_q(FILE *out, ike_sa_t *ike_sa, task_queue_t q, char *name)
{
	enumerator_t *enumerator;
	bool has = FALSE;
	task_t *task;

	enumerator = ike_sa->create_task_enumerator(ike_sa, q);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (!has)
		{
			fprintf(out, "%12s[%d]: Tasks %s: ", ike_sa->get_name(ike_sa),
					ike_sa->get_unique_id(ike_sa), name);
			has = TRUE;
		}
		fprintf(out, "%N ", task_type_names, task->get_type(task));
	}
	enumerator->destroy(enumerator);
	if (has)
	{
		fprintf(out, "\n");
	}
}

/**
 * log an IKE_SA to out
 */
static void log_ike_sa(FILE *out, ike_sa_t *ike_sa, bool all)
{
	ike_sa_id_t *id = ike_sa->get_id(ike_sa);
	time_t now = time_monotonic(NULL);

	fprintf(out, "%12s[%d]: %N",
			ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa),
			ike_sa_state_names, ike_sa->get_state(ike_sa));

	if (ike_sa->get_state(ike_sa) == IKE_ESTABLISHED)
	{
		time_t established;

		established = ike_sa->get_statistic(ike_sa, STAT_ESTABLISHED);
		fprintf(out, " %V ago", &now, &established);
	}

	fprintf(out, ", %H[%Y]...%H[%Y]\n",
			ike_sa->get_my_host(ike_sa), ike_sa->get_my_id(ike_sa),
			ike_sa->get_other_host(ike_sa), ike_sa->get_other_id(ike_sa));

	if (all)
	{
		proposal_t *ike_proposal;
		identification_t *eap_id;

		eap_id = ike_sa->get_other_eap_id(ike_sa);

		if (!eap_id->equals(eap_id, ike_sa->get_other_id(ike_sa)))
		{
			fprintf(out, "%12s[%d]: Remote %s identity: %Y\n",
					ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa),
					ike_sa->get_version(ike_sa) == IKEV1 ? "XAuth" : "EAP",
					eap_id);
		}

		ike_proposal = ike_sa->get_proposal(ike_sa);

		fprintf(out, "%12s[%d]: %N SPIs: %.16"PRIx64"_i%s %.16"PRIx64"_r%s",
				ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa),
				ike_version_names, ike_sa->get_version(ike_sa),
				be64toh(id->get_initiator_spi(id)),
				id->is_initiator(id) ? "*" : "",
				be64toh(id->get_responder_spi(id)),
				id->is_initiator(id) ? "" : "*");


		if (ike_sa->get_state(ike_sa) == IKE_ESTABLISHED)
		{
			time_t rekey, reauth;
			peer_cfg_t *peer_cfg;

			rekey = ike_sa->get_statistic(ike_sa, STAT_REKEY);
			reauth = ike_sa->get_statistic(ike_sa, STAT_REAUTH);
			peer_cfg = ike_sa->get_peer_cfg(ike_sa);

			if (rekey)
			{
				fprintf(out, ", rekeying in %V", &rekey, &now);
			}
			if (reauth)
			{
				bool first = TRUE;
				enumerator_t *enumerator;
				auth_cfg_t *auth;

				fprintf(out, ", ");
				enumerator = peer_cfg->create_auth_cfg_enumerator(peer_cfg, TRUE);
				while (enumerator->enumerate(enumerator, &auth))
				{
					if (!first)
					{
						fprintf(out, "+");
					}
					first = FALSE;
					fprintf(out, "%N", auth_class_names,
							auth->get(auth, AUTH_RULE_AUTH_CLASS));
				}
				enumerator->destroy(enumerator);
				fprintf(out, " reauthentication in %V", &reauth, &now);
			}
			if (!rekey && !reauth)
			{
				fprintf(out, ", rekeying disabled");
			}
		}
		fprintf(out, "\n");

		if (ike_proposal)
		{
			char buf[BUF_LEN];

			snprintf(buf, BUF_LEN, "%P", ike_proposal);
			fprintf(out, "%12s[%d]: IKE proposal: %s\n",
					ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa),
					buf+4);
		}

		log_task_q(out, ike_sa, TASK_QUEUE_QUEUED, "queued");
		log_task_q(out, ike_sa, TASK_QUEUE_ACTIVE, "active");
		log_task_q(out, ike_sa, TASK_QUEUE_PASSIVE, "passive");
	}
}

/**
 * log an CHILD_SA to out
 */
static void log_child_sa(FILE *out, child_sa_t *child_sa, bool all)
{
	time_t use_in, use_out, rekey, now;
	uint64_t bytes_in, bytes_out, packets_in, packets_out;
	proposal_t *proposal;
	linked_list_t *my_ts, *other_ts;
	child_cfg_t *config;

	config = child_sa->get_config(child_sa);
	now = time_monotonic(NULL);

	fprintf(out, "%12s{%d}:  %N, %N%s, reqid %u",
			child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa),
			child_sa_state_names, child_sa->get_state(child_sa),
			ipsec_mode_names, child_sa->get_mode(child_sa),
			config->has_option(config, OPT_PROXY_MODE) ? "_PROXY" : "",
			child_sa->get_reqid(child_sa));

	if (child_sa->get_state(child_sa) == CHILD_INSTALLED)
	{
		fprintf(out, ", %N%s SPIs: %.8x_i %.8x_o",
				protocol_id_names, child_sa->get_protocol(child_sa),
				child_sa->has_encap(child_sa) ? " in UDP" : "",
				ntohl(child_sa->get_spi(child_sa, TRUE)),
				ntohl(child_sa->get_spi(child_sa, FALSE)));

		if (child_sa->get_ipcomp(child_sa) != IPCOMP_NONE)
		{
			fprintf(out, ", IPCOMP CPIs: %.4x_i %.4x_o",
					ntohs(child_sa->get_cpi(child_sa, TRUE)),
					ntohs(child_sa->get_cpi(child_sa, FALSE)));
		}

		if (all)
		{
			fprintf(out, "\n%12s{%d}:  ", child_sa->get_name(child_sa),
					child_sa->get_unique_id(child_sa));

			proposal = child_sa->get_proposal(child_sa);
			if (proposal)
			{
				uint16_t alg, ks;
				bool first = TRUE;

				if (proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM,
											&alg, &ks) && alg != ENCR_UNDEFINED)
				{
					fprintf(out, "%N", encryption_algorithm_names, alg);
					first = FALSE;
					if (ks)
					{
						fprintf(out, "_%u", ks);
					}
				}
				if (proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM,
											&alg, &ks) && alg != AUTH_UNDEFINED)
				{
					fprintf(out, "%s%N", first ? "" : "/",
							integrity_algorithm_names, alg);
					if (ks)
					{
						fprintf(out, "_%u", ks);
					}
				}
				if (proposal->get_algorithm(proposal, KEY_EXCHANGE_METHOD,
											&alg, NULL))
				{
					fprintf(out, "/%N", key_exchange_method_names, alg);
				}
				if (proposal->get_algorithm(proposal, EXTENDED_SEQUENCE_NUMBERS,
											&alg, NULL) && alg == EXT_SEQ_NUMBERS)
				{
					fprintf(out, "/ESN");
				}
			}

			child_sa->get_usestats(child_sa, TRUE,
								   &use_in, &bytes_in, &packets_in);
			fprintf(out, ", %" PRIu64 " bytes_i", bytes_in);
			if (use_in)
			{
				fprintf(out, " (%" PRIu64 " pkt%s, %" PRIu64 "s ago)",
						packets_in, (packets_in == 1) ? "": "s",
						(uint64_t)(now - use_in));
			}

			child_sa->get_usestats(child_sa, FALSE,
								   &use_out, &bytes_out, &packets_out);
			fprintf(out, ", %" PRIu64 " bytes_o", bytes_out);
			if (use_out)
			{
				fprintf(out, " (%" PRIu64 " pkt%s, %" PRIu64 "s ago)",
						packets_out, (packets_out == 1) ? "": "s",
						(uint64_t)(now - use_out));
			}
			fprintf(out, ", rekeying ");

			rekey = child_sa->get_lifetime(child_sa, FALSE);
			if (rekey)
			{
				if (now > rekey)
				{
					fprintf(out, "active");
				}
				else
				{
					fprintf(out, "in %V", &now, &rekey);
				}
			}
			else
			{
				fprintf(out, "disabled");
			}

		}
	}
	else if (child_sa->get_state(child_sa) == CHILD_REKEYING ||
			 child_sa->get_state(child_sa) == CHILD_REKEYED)
	{
		rekey = child_sa->get_lifetime(child_sa, TRUE);
		fprintf(out, ", expires in %V", &now, &rekey);
	}

	my_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, TRUE));
	other_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, FALSE));
	fprintf(out, "\n%12s{%d}:   %#R === %#R\n",
			child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa),
			my_ts, other_ts);
	my_ts->destroy(my_ts);
	other_ts->destroy(other_ts);
}

/**
 * Log a configs local or remote authentication config to out
 */
static void log_auth_cfgs(FILE *out, peer_cfg_t *peer_cfg, bool local)
{
	enumerator_t *enumerator, *rules;
	auth_rule_t rule;
	auth_cfg_t *auth;
	auth_class_t auth_class;
	identification_t *id;
	certificate_t *cert;
	cert_validation_t valid;
	char *name;

	name = peer_cfg->get_name(peer_cfg);

	enumerator = peer_cfg->create_auth_cfg_enumerator(peer_cfg, local);
	while (enumerator->enumerate(enumerator, &auth))
	{
		fprintf(out, "%12s:   %s", name, local ? "local: " : "remote:");
		id = auth->get(auth, AUTH_RULE_IDENTITY);
		if (id)
		{
			fprintf(out, " [%Y]", id);
		}
		fprintf(out, " uses ");

		auth_class = (uintptr_t)auth->get(auth, AUTH_RULE_AUTH_CLASS);
		if (auth_class == AUTH_CLASS_EAP)
		{
			if ((uintptr_t)auth->get(auth, AUTH_RULE_EAP_TYPE) == EAP_NAK)
			{
				fprintf(out, "EAP authentication");
			}
			else
			{
				if ((uintptr_t)auth->get(auth, AUTH_RULE_EAP_VENDOR))
				{
					fprintf(out, "EAP_%" PRIuPTR "-%" PRIuPTR " authentication",
						(uintptr_t)auth->get(auth, AUTH_RULE_EAP_TYPE),
						(uintptr_t)auth->get(auth, AUTH_RULE_EAP_VENDOR));
				}
				else
				{
					fprintf(out, "%N authentication", eap_type_names,
						(uintptr_t)auth->get(auth, AUTH_RULE_EAP_TYPE));
				}
			}
			id = auth->get(auth, AUTH_RULE_EAP_IDENTITY);
			if (id)
			{
				fprintf(out, " with EAP identity '%Y'", id);
			}
			fprintf(out, "\n");
		}
		else if (auth_class == AUTH_CLASS_XAUTH)
		{
			fprintf(out, "%N authentication: %s", auth_class_names, auth_class,
					auth->get(auth, AUTH_RULE_XAUTH_BACKEND) ?: "any");
			id = auth->get(auth, AUTH_RULE_XAUTH_IDENTITY);
			if (id)
			{
				fprintf(out, " with XAuth identity '%Y'", id);
			}
			fprintf(out, "\n");
		}
		else
		{
			fprintf(out, "%N authentication\n", auth_class_names, auth_class);
		}

		cert = auth->get(auth, AUTH_RULE_CA_CERT);
		if (cert)
		{
			fprintf(out, "%12s:    ca:    \"%Y\"\n", name, cert->get_subject(cert));
		}

		cert = auth->get(auth, AUTH_RULE_IM_CERT);
		if (cert)
		{
			fprintf(out, "%12s:    im-ca: \"%Y\"\n", name, cert->get_subject(cert));
		}

		cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
		if (cert)
		{
			fprintf(out, "%12s:    cert:  \"%Y\"\n", name,
					cert->get_subject(cert));
		}

		valid = (uintptr_t)auth->get(auth, AUTH_RULE_OCSP_VALIDATION);
		if (valid != VALIDATION_FAILED)
		{
			fprintf(out, "%12s:    ocsp:  status must be GOOD%s\n", name,
					(valid == VALIDATION_SKIPPED) ? " or SKIPPED" : "");
		}

		valid = (uintptr_t)auth->get(auth, AUTH_RULE_CRL_VALIDATION);
		if (valid != VALIDATION_FAILED)
		{
			fprintf(out, "%12s:    crl:   status must be GOOD%s\n", name,
					(valid == VALIDATION_SKIPPED) ? " or SKIPPED" : "");
		}

		rules = auth->create_enumerator(auth);
		while (rules->enumerate(rules, &rule, &id))
		{
			if (rule == AUTH_RULE_GROUP)
			{
				fprintf(out, "%12s:    group: %Y\n", name, id);
			}
		}
		rules->destroy(rules);
	}
	enumerator->destroy(enumerator);
}

METHOD(stroke_list_t, status, void,
	private_stroke_list_t *this, stroke_msg_t *msg, FILE *out,
	bool all, bool wait)
{
	enumerator_t *enumerator, *children;
	ike_cfg_t *ike_cfg;
	child_cfg_t *child_cfg;
	child_sa_t *child_sa;
	ike_sa_t *ike_sa;
	linked_list_t *my_ts, *other_ts;
	bool first, found = FALSE;
	char *name = msg->status.name;
	u_int half_open;

	if (all)
	{
		peer_cfg_t *peer_cfg;
		ike_version_t ike_version;
		char *pool;
		host_t *host;
		uint32_t dpd;
		time_t since, now;
		u_int size, online, offline, i;
		struct utsname utsname;

		now = time_monotonic(NULL);
		since = time(NULL) - (now - this->uptime);

		fprintf(out, "Status of IKE charon daemon (%sSwan "VERSION, this->swan);
		if (uname(&utsname) == 0)
		{
			fprintf(out, ", %s %s, %s",
					utsname.sysname, utsname.release, utsname.machine);
		}
		fprintf(out, "):\n  uptime: %V, since %T\n", &now, &this->uptime, &since,
				FALSE);
		{
#ifdef HAVE_MALLINFO2
			struct mallinfo2 mi = mallinfo2();

			fprintf(out, "  malloc: sbrk %zu, mmap %zu, used %zu, free %zu\n",
				    mi.arena, mi.hblkhd, mi.uordblks, mi.fordblks);
#elif defined (HAVE_MALLINFO)
			struct mallinfo mi = mallinfo();

			fprintf(out, "  malloc: sbrk %u, mmap %u, used %u, free %u\n",
				    mi.arena, mi.hblkhd, mi.uordblks, mi.fordblks);
#endif
		}
		fprintf(out, "  worker threads: %d of %d idle, ",
				lib->processor->get_idle_threads(lib->processor),
				lib->processor->get_total_threads(lib->processor));
		for (i = 0; i < JOB_PRIO_MAX; i++)
		{
			fprintf(out, "%s%d", i == 0 ? "" : "/",
					lib->processor->get_working_threads(lib->processor, i));
		}
		fprintf(out, " working, job queue: ");
		for (i = 0; i < JOB_PRIO_MAX; i++)
		{
			fprintf(out, "%s%d", i == 0 ? "" : "/",
					lib->processor->get_job_load(lib->processor, i));
		}
		fprintf(out, ", scheduled: %d\n",
				lib->scheduler->get_job_load(lib->scheduler));
		fprintf(out, "  loaded plugins: %s\n",
				lib->plugins->loaded_plugins(lib->plugins));

		first = TRUE;
		enumerator = this->attribute->create_pool_enumerator(this->attribute);
		while (enumerator->enumerate(enumerator, &pool, &size, &online, &offline))
		{
			if (name && !streq(name, pool))
			{
				continue;
			}
			if (first)
			{
				first = FALSE;
				fprintf(out, "Virtual IP pools (size/online/offline):\n");
			}
			fprintf(out, "  %s: %u/%u/%u\n", pool, size, online, offline);
		}
		enumerator->destroy(enumerator);

		enumerator = charon->kernel->create_address_enumerator(charon->kernel,
															ADDR_TYPE_REGULAR);
		fprintf(out, "Listening IP addresses:\n");
		while (enumerator->enumerate(enumerator, (void**)&host))
		{
			fprintf(out, "  %H\n", host);
		}
		enumerator->destroy(enumerator);

		fprintf(out, "Connections:\n");
		enumerator = charon->backends->create_peer_cfg_enumerator(
							charon->backends, NULL, NULL, NULL, NULL, IKE_ANY);
		while (enumerator->enumerate(enumerator, &peer_cfg))
		{
			char *my_addr, *other_addr;

			if (name && !streq(name, peer_cfg->get_name(peer_cfg)))
			{
				continue;
			}

			ike_cfg = peer_cfg->get_ike_cfg(peer_cfg);
			ike_version = peer_cfg->get_ike_version(peer_cfg);
			my_addr = ike_cfg->get_my_addr(ike_cfg);
			other_addr = ike_cfg->get_other_addr(ike_cfg);
			fprintf(out, "%12s:  %s...%s  %N", peer_cfg->get_name(peer_cfg),
					my_addr, other_addr, ike_version_names, ike_version);

			if (ike_version == IKEV1 && peer_cfg->use_aggressive(peer_cfg))
			{
				fprintf(out, " Aggressive");
			}

			dpd = peer_cfg->get_dpd(peer_cfg);
			if (dpd)
			{
				fprintf(out, ", dpddelay=%us", dpd);
			}
			fprintf(out, "\n");

			log_auth_cfgs(out, peer_cfg, TRUE);
			log_auth_cfgs(out, peer_cfg, FALSE);

			children = peer_cfg->create_child_cfg_enumerator(peer_cfg);
			while (children->enumerate(children, &child_cfg))
			{
				my_ts = child_cfg->get_traffic_selectors(child_cfg, TRUE,
														 NULL, NULL, FALSE);
				other_ts = child_cfg->get_traffic_selectors(child_cfg, FALSE,
															NULL, NULL, FALSE);
				fprintf(out, "%12s:   child:  %#R === %#R %N",
						child_cfg->get_name(child_cfg),	my_ts, other_ts,
						ipsec_mode_names, child_cfg->get_mode(child_cfg));
				my_ts->destroy_offset(my_ts, offsetof(traffic_selector_t, destroy));
				other_ts->destroy_offset(other_ts, offsetof(traffic_selector_t, destroy));

				if (dpd)
				{
					fprintf(out, ", dpdaction=%N", action_names,
							child_cfg->get_dpd_action(child_cfg));
				}
				fprintf(out, "\n");
			}
			children->destroy(children);
		}
		enumerator->destroy(enumerator);
	}

	/* Enumerate shunt policies */
	first = TRUE;
	enumerator = charon->shunts->create_enumerator(charon->shunts);
	while (enumerator->enumerate(enumerator, NULL, &child_cfg))
	{
		if (name && !streq(name, child_cfg->get_name(child_cfg)))
		{
			continue;
		}
		if (first)
		{
			fprintf(out, "Shunted Connections:\n");
			first = FALSE;
		}
		my_ts = child_cfg->get_traffic_selectors(child_cfg, TRUE, NULL,
												 NULL, FALSE);
		other_ts = child_cfg->get_traffic_selectors(child_cfg, FALSE, NULL,
													NULL, FALSE);
		fprintf(out, "%12s:  %#R === %#R %N\n",
				child_cfg->get_name(child_cfg),	my_ts, other_ts,
				ipsec_mode_names, child_cfg->get_mode(child_cfg));
		my_ts->destroy_offset(my_ts, offsetof(traffic_selector_t, destroy));
		other_ts->destroy_offset(other_ts, offsetof(traffic_selector_t, destroy));
	}
	enumerator->destroy(enumerator);

	/* Enumerate traps */
	first = TRUE;
	enumerator = charon->traps->create_enumerator(charon->traps);
	while (enumerator->enumerate(enumerator, NULL, &child_sa))
	{
		if (name && !streq(name, child_sa->get_name(child_sa)))
		{
			continue;
		}
		if (first)
		{
			fprintf(out, "Routed Connections:\n");
			first = FALSE;
		}
		log_child_sa(out, child_sa, all);
	}
	enumerator->destroy(enumerator);

	half_open = charon->ike_sa_manager->get_half_open_count(
										charon->ike_sa_manager, NULL, FALSE);
	fprintf(out, "Security Associations (%u up, %u connecting):\n",
		charon->ike_sa_manager->get_count(charon->ike_sa_manager) - half_open,
		half_open);
	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, wait);
	while (enumerator->enumerate(enumerator, &ike_sa) && ferror(out) == 0)
	{
		bool ike_printed = FALSE;
		enumerator_t *children = ike_sa->create_child_sa_enumerator(ike_sa);

		if (name == NULL || streq(name, ike_sa->get_name(ike_sa)))
		{
			log_ike_sa(out, ike_sa, all);
			found = TRUE;
			ike_printed = TRUE;
		}

		while (children->enumerate(children, (void**)&child_sa))
		{
			if (name == NULL || streq(name, child_sa->get_name(child_sa)))
			{
				if (!ike_printed)
				{
					log_ike_sa(out, ike_sa, all);
					found = TRUE;
					ike_printed = TRUE;
				}
				log_child_sa(out, child_sa, all);
			}
		}
		children->destroy(children);
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		if (name)
		{
			fprintf(out, "  no match\n");
		}
		else
		{
			fprintf(out, "  none\n");
		}
	}
}

/**
 * create a unique certificate list without duplicates
 * certificates having the same issuer are grouped together.
 */
static linked_list_t* create_unique_cert_list(certificate_type_t type)
{
	linked_list_t *list = linked_list_create();
	enumerator_t *enumerator = lib->credmgr->create_cert_enumerator(
									lib->credmgr, type, KEY_ANY, NULL, FALSE);
	certificate_t *cert;

	while (enumerator->enumerate(enumerator, (void**)&cert))
	{
		enumerator_t *added = list->create_enumerator(list);
		identification_t *issuer = cert->get_issuer(cert);
		bool previous_same, same = FALSE, found = FALSE;
		certificate_t *list_cert;

		while (added->enumerate(added, (void**)&list_cert))
		{
			if (list_cert->equals(list_cert, cert))
			{	/* stop if we found a duplicate*/
				found = TRUE;
				break;
			}
			previous_same = same;
			same = list_cert->has_issuer(list_cert, issuer);
			if (previous_same && !same)
			{	/* group certificates with same issuer */
				break;
			}
		}
		if (!found)
		{
			list->insert_before(list, added, cert->get_ref(cert));
		}
		added->destroy(added);
	}
	enumerator->destroy(enumerator);
	return list;
}

/**
 * Is there a matching private key?
 */
static bool has_privkey(certificate_t *cert)
{
	public_key_t *public;
	private_key_t *private = NULL;
	chunk_t keyid;
	identification_t *id;

	public = cert->get_public_key(cert);
	if (!public)
	{
		return FALSE;
	}
	if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &keyid))
	{
		id = identification_create_from_encoding(ID_KEY_ID, keyid);
		private = lib->credmgr->get_private(lib->credmgr,
									public->get_type(public), id, NULL);
		id->destroy(id);
	}
	public->destroy(public);
	DESTROY_IF(private);

	return (private != NULL);
}

/**
 * list all X.509 certificates matching the flags
 */
static void stroke_list_x509_certs(linked_list_t *list, x509_flag_t flag)
{
	enumerator_t *enumerator;
	certificate_t *cert;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, (void**)&cert))
	{
		x509_t *x509 = (x509_t*)cert;
		x509_flag_t flags = x509->get_flags(x509) & X509_ANY;

		/* list only if flag is set or flag == 0 */
		if ((flags & flag) || flags == flag)
		{
			cert_printer->print_caption(cert_printer, CERT_X509, flag);
			cert_printer->print(cert_printer, cert, has_privkey(cert));
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * list all other certificates types
 */
static void stroke_list_other_certs(certificate_type_t type)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	linked_list_t *list;

	list = create_unique_cert_list(type);

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &cert))
	{
		cert_printer->print_caption(cert_printer, cert->get_type(cert), X509_NONE);
		cert_printer->print(cert_printer, cert, has_privkey(cert));
	}
	enumerator->destroy(enumerator);

	list->destroy_offset(list, offsetof(certificate_t, destroy));
}

/**
 * Print the name of an algorithm plus the name of the plugin that registered it
 */
static void print_alg(FILE *out, int *len, enum_name_t *alg_names, int alg_type,
					  const char *plugin_name)
{
	char alg_name[BUF_LEN];
	int alg_name_len;

	if (alg_names)
	{
		alg_name_len = sprintf(alg_name, " %N[%s]", alg_names, alg_type,
							   plugin_name);
	}
	else
	{
		alg_name_len = sprintf(alg_name, " [%s]", plugin_name);
	}
	if (*len + alg_name_len > CRYPTO_MAX_ALG_LINE)
	{
		fprintf(out, "\n             ");
		*len = 13;
	}
	fprintf(out, "%s", alg_name);
	*len += alg_name_len;
}

/**
 * List of registered cryptographical algorithms
 */
static void list_algs(FILE *out)
{
	enumerator_t *enumerator;
	encryption_algorithm_t encryption;
	integrity_algorithm_t integrity;
	hash_algorithm_t hash;
	pseudo_random_function_t prf;
	ext_out_function_t xof;
	key_derivation_function_t kdf;
	drbg_type_t drbg;
	key_exchange_method_t group;
	rng_quality_t quality;
	const char *plugin_name;
	int len;

	fprintf(out, "\n");
	fprintf(out, "List of registered IKE algorithms:\n");
	fprintf(out, "\n  encryption:");
	len = 13;
	enumerator = lib->crypto->create_crypter_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
	{
		print_alg(out, &len, encryption_algorithm_names, encryption, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  integrity: ");
	len = 13;
	enumerator = lib->crypto->create_signer_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &integrity, &plugin_name))
	{
		print_alg(out, &len, integrity_algorithm_names, integrity, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  aead:      ");
	len = 13;
	enumerator = lib->crypto->create_aead_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &encryption, &plugin_name))
	{
		print_alg(out, &len, encryption_algorithm_names, encryption, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  hasher:    ");
	len = 13;
	enumerator = lib->crypto->create_hasher_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &hash, &plugin_name))
	{
		print_alg(out, &len, hash_algorithm_names, hash, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  prf:       ");
	len = 13;
	enumerator = lib->crypto->create_prf_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &prf, &plugin_name))
	{
		print_alg(out, &len, pseudo_random_function_names, prf, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  xof:       ");
	len = 13;
	enumerator = lib->crypto->create_xof_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &xof, &plugin_name))
	{
		print_alg(out, &len, ext_out_function_names, xof, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  kdf:       ");
	len = 13;
	enumerator = lib->crypto->create_kdf_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &kdf, &plugin_name))
	{
		print_alg(out, &len, key_derivation_function_names, kdf, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  drbg:      ");
	len = 13;
	enumerator = lib->crypto->create_drbg_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &drbg, &plugin_name))
	{
		print_alg(out, &len, drbg_type_names, drbg, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  dh-group:  ");
	len = 13;
	enumerator = lib->crypto->create_ke_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &group, &plugin_name))
	{
		print_alg(out, &len, key_exchange_method_names, group, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  random-gen:");
	len = 13;
	enumerator = lib->crypto->create_rng_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &quality, &plugin_name))
	{
		print_alg(out, &len, rng_quality_names, quality, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n  nonce-gen: ");
	len = 13;
	enumerator = lib->crypto->create_nonce_gen_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &plugin_name))
	{
		print_alg(out, &len, NULL, 0, plugin_name);
	}
	enumerator->destroy(enumerator);
	fprintf(out, "\n");
}

/**
 * List loaded plugin information
 */
static void list_plugins(FILE *out)
{
	plugin_feature_t *features, *fp;
	enumerator_t *enumerator;
	linked_list_t *list;
	plugin_t *plugin;
	int count, i;
	bool loaded;
	char *str;

	fprintf(out, "\n");
	fprintf(out, "List of loaded Plugins:\n");
	fprintf(out, "\n");

	enumerator = lib->plugins->create_plugin_enumerator(lib->plugins);
	while (enumerator->enumerate(enumerator, &plugin, &list))
	{
		fprintf(out, "%s:\n", plugin->get_name(plugin));
		if (plugin->get_features)
		{
			count = plugin->get_features(plugin, &features);
			for (i = 0; i < count; i++)
			{
				str = plugin_feature_get_string(&features[i]);
				switch (features[i].kind)
				{
					case FEATURE_PROVIDE:
						fp = &features[i];
						loaded = list->find_first(list, NULL, (void**)&fp);
						fprintf(out, "    %s%s\n",
								str, loaded ? "" : " (not loaded)");
						break;
					case FEATURE_DEPENDS:
						fprintf(out, "        %s\n", str);
						break;
					case FEATURE_SDEPEND:
						fprintf(out, "        %s (soft)\n", str);
						break;
					default:
						break;
				}
				free(str);
			}
		}
		list->destroy(list);
	}
	enumerator->destroy(enumerator);
}

METHOD(stroke_list_t, list, void,
	private_stroke_list_t *this, stroke_msg_t *msg, FILE *out)
{
	linked_list_t *cert_list = NULL;

	cert_printer = certificate_printer_create(out, TRUE, msg->list.utc);

	if (msg->list.flags & LIST_PUBKEYS)
	{
		stroke_list_other_certs(CERT_TRUSTED_PUBKEY);
	}
	if (msg->list.flags & LIST_CERTS)
	{
		stroke_list_other_certs(CERT_GPG);
	}
	if (msg->list.flags & (LIST_CERTS | LIST_CACERTS | LIST_OCSPCERTS | LIST_AACERTS))
	{
		cert_list = create_unique_cert_list(CERT_X509);
	}
	if (msg->list.flags & LIST_CERTS)
	{
		stroke_list_x509_certs(cert_list, X509_NONE);
	}
	if (msg->list.flags & LIST_CACERTS)
	{
		stroke_list_x509_certs(cert_list, X509_CA);
	}
	if (msg->list.flags & LIST_OCSPCERTS)
	{
		stroke_list_x509_certs(cert_list, X509_OCSP_SIGNER);
	}
	if (msg->list.flags & LIST_AACERTS)
	{
		stroke_list_x509_certs(cert_list, X509_AA);
	}
	DESTROY_OFFSET_IF(cert_list, offsetof(certificate_t, destroy));

	if (msg->list.flags & LIST_ACERTS)
	{
		stroke_list_other_certs(CERT_X509_AC);
	}
	if (msg->list.flags & LIST_CRLS)
	{
		stroke_list_other_certs(CERT_X509_CRL);
	}
	if (msg->list.flags & LIST_OCSP)
	{
		stroke_list_other_certs(CERT_X509_OCSP_RESPONSE);
	}
	if (msg->list.flags & LIST_ALGS)
	{
		list_algs(out);
	}
	if (msg->list.flags & LIST_PLUGINS)
	{
		list_plugins(out);
	}
	cert_printer->destroy(cert_printer);
	cert_printer = NULL;
}

/**
 * Print leases of a single pool
 */
static void pool_leases(private_stroke_list_t *this, FILE *out, char *pool,
						host_t *address, u_int size, u_int online, u_int offline)
{
	enumerator_t *enumerator;
	identification_t *id;
	host_t *lease;
	bool on;
	int found = 0;

	fprintf(out, "Leases in pool '%s', usage: %u/%u, %u online\n",
			pool, online + offline, size, online);
	enumerator = this->attribute->create_lease_enumerator(this->attribute, pool);
	while (enumerator->enumerate(enumerator, &id, &lease, &on))
	{
		if (!address || address->ip_equals(address, lease))
		{
			fprintf(out, "  %15H   %s   '%Y'\n",
					lease, on ? "online" : "offline", id);
			found++;
		}
	}
	enumerator->destroy(enumerator);
	if (!found)
	{
		fprintf(out, "  no matching leases found\n");
	}
}

METHOD(stroke_list_t, leases, void,
	private_stroke_list_t *this, stroke_msg_t *msg, FILE *out)
{
	enumerator_t *enumerator;
	u_int size, offline, online;
	host_t *address = NULL;
	char *pool;
	int found = 0;

	if (msg->leases.address)
	{
		address = host_create_from_string(msg->leases.address, 0);
	}

	enumerator = this->attribute->create_pool_enumerator(this->attribute);
	while (enumerator->enumerate(enumerator, &pool, &size, &online, &offline))
	{
		if (!msg->leases.pool || streq(msg->leases.pool, pool))
		{
			pool_leases(this, out, pool, address, size, online, offline);
			found++;
		}
	}
	enumerator->destroy(enumerator);
	if (!found)
	{
		if (msg->leases.pool)
		{
			fprintf(out, "pool '%s' not found\n", msg->leases.pool);
		}
		else
		{
			fprintf(out, "no pools found\n");
		}
	}
	DESTROY_IF(address);
}

METHOD(stroke_list_t, destroy, void,
	private_stroke_list_t *this)
{
	free(this);
}

/*
 * see header file
 */
stroke_list_t *stroke_list_create(stroke_attribute_t *attribute)
{
	private_stroke_list_t *this;

	INIT(this,
		.public = {
			.list = _list,
			.status = _status,
			.leases = _leases,
			.destroy = _destroy,
		},
		.uptime = time_monotonic(NULL),
		.swan = "strong",
		.attribute = attribute,
	);

	if (lib->settings->get_bool(lib->settings,
		"charon.i_dont_care_about_security_and_use_aggressive_mode_psk", FALSE))
	{
		this->swan = "weak";
	}

	return &this->public;
}
