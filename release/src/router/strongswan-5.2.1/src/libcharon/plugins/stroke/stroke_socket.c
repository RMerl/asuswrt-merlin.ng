/*
 * Copyright (C) 2011-2013 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include "stroke_socket.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <hydra.h>
#include <daemon.h>

#include "stroke_config.h"
#include "stroke_control.h"
#include "stroke_cred.h"
#include "stroke_ca.h"
#include "stroke_attribute.h"
#include "stroke_handler.h"
#include "stroke_list.h"
#include "stroke_counter.h"

/**
 * To avoid clogging the thread pool with (blocking) jobs, we limit the number
 * of concurrently handled stroke commands.
 */
#define MAX_CONCURRENT_DEFAULT 4

typedef struct stroke_job_context_t stroke_job_context_t;
typedef struct private_stroke_socket_t private_stroke_socket_t;

/**
 * private data of stroke_socket
 */
struct private_stroke_socket_t {

	/**
	 * public functions
	 */
	stroke_socket_t public;

	/**
	 * Service accepting stroke connections
	 */
	stream_service_t *service;

	/**
	 * configuration backend
	 */
	stroke_config_t *config;

	/**
	 * attribute provider
	 */
	stroke_attribute_t *attribute;

	/**
	 * attribute handler (requests only)
	 */
	stroke_handler_t *handler;

	/**
	 * controller to control daemon
	 */
	stroke_control_t *control;

	/**
	 * credential set
	 */
	stroke_cred_t *cred;

	/**
	 * CA sections
	 */
	stroke_ca_t *ca;

	/**
	 * status information logging
	 */
	stroke_list_t *list;

	/**
	 * Counter values for IKE events
	 */
	stroke_counter_t *counter;

	/**
	 * TRUE if log level changes are not allowed
	 */
	bool prevent_loglevel_changes;
};

/**
 * Helper macro to log configuration options, but only if they are defined.
 */
#define DBG_OPT(...) VA_ARGS_DISPATCH(DBG_OPT, __VA_ARGS__)(__VA_ARGS__)
#define DBG_OPT2(fmt, val) ({ \
	typeof(val) _val = val; \
	if (_val) { DBG2(DBG_CFG, fmt, _val); } \
})
#define DBG_OPT3(fmt, label, val) ({ \
	typeof(val) _val = val; \
	if (_val) { DBG2(DBG_CFG, fmt, label, _val); } \
})

/**
 * Helper function which corrects the string pointers
 * in a stroke_msg_t. Strings in a stroke_msg sent over "wire"
 * contains RELATIVE addresses (relative to the beginning of the
 * stroke_msg). They must be corrected if they reach our address
 * space...
 */
static void pop_string(stroke_msg_t *msg, char **string)
{
	if (*string == NULL)
	{
		return;
	}

	/* check for sanity of string pointer and string */
	if (string < (char**)msg ||
		string > (char**)((char*)msg + sizeof(stroke_msg_t)) ||
		(unsigned long)*string < (unsigned long)((char*)msg->buffer - (char*)msg) ||
		(unsigned long)*string > msg->length)
	{
		*string = "(invalid pointer in stroke msg)";
	}
	else
	{
		*string = (char*)msg + (unsigned long)*string;
	}
}

/**
 * Pop the strings of a stroke_end_t struct and log them for debugging purposes
 */
static void pop_end(stroke_msg_t *msg, const char* label, stroke_end_t *end)
{
	pop_string(msg, &end->address);
	pop_string(msg, &end->subnets);
	pop_string(msg, &end->sourceip);
	pop_string(msg, &end->dns);
	pop_string(msg, &end->auth);
	pop_string(msg, &end->auth2);
	pop_string(msg, &end->id);
	pop_string(msg, &end->id2);
	pop_string(msg, &end->rsakey);
	pop_string(msg, &end->cert);
	pop_string(msg, &end->cert2);
	pop_string(msg, &end->ca);
	pop_string(msg, &end->ca2);
	pop_string(msg, &end->groups);
	pop_string(msg, &end->groups2);
	pop_string(msg, &end->cert_policy);
	pop_string(msg, &end->updown);

	DBG_OPT("  %s=%s", label, end->address);
	DBG_OPT("  %ssubnet=%s", label, end->subnets);
	DBG_OPT("  %ssourceip=%s", label, end->sourceip);
	DBG_OPT("  %sdns=%s", label, end->dns);
	DBG_OPT("  %sauth=%s", label, end->auth);
	DBG_OPT("  %sauth2=%s", label, end->auth2);
	DBG_OPT("  %sid=%s", label, end->id);
	DBG_OPT("  %sid2=%s", label, end->id2);
	DBG_OPT("  %srsakey=%s", label, end->rsakey);
	DBG_OPT("  %scert=%s", label, end->cert);
	DBG_OPT("  %scert2=%s", label, end->cert2);
	DBG_OPT("  %sca=%s", label, end->ca);
	DBG_OPT("  %sca2=%s", label, end->ca2);
	DBG_OPT("  %sgroups=%s", label, end->groups);
	DBG_OPT("  %sgroups2=%s", label, end->groups2);
	DBG_OPT("  %supdown=%s", label, end->updown);
}

/**
 * Add a connection to the configuration list
 */
static void stroke_add_conn(private_stroke_socket_t *this, stroke_msg_t *msg)
{
	pop_string(msg, &msg->add_conn.name);
	DBG1(DBG_CFG, "received stroke: add connection '%s'", msg->add_conn.name);

	DBG2(DBG_CFG, "conn %s", msg->add_conn.name);
	pop_end(msg, "left", &msg->add_conn.me);
	pop_end(msg, "right", &msg->add_conn.other);
	pop_string(msg, &msg->add_conn.eap_identity);
	pop_string(msg, &msg->add_conn.aaa_identity);
	pop_string(msg, &msg->add_conn.xauth_identity);
	pop_string(msg, &msg->add_conn.algorithms.ike);
	pop_string(msg, &msg->add_conn.algorithms.esp);
	pop_string(msg, &msg->add_conn.algorithms.ah);
	pop_string(msg, &msg->add_conn.ikeme.mediated_by);
	pop_string(msg, &msg->add_conn.ikeme.peerid);
	DBG_OPT("  eap_identity=%s", msg->add_conn.eap_identity);
	DBG_OPT("  aaa_identity=%s", msg->add_conn.aaa_identity);
	DBG_OPT("  xauth_identity=%s", msg->add_conn.xauth_identity);
	DBG_OPT("  ike=%s", msg->add_conn.algorithms.ike);
	DBG_OPT("  esp=%s", msg->add_conn.algorithms.esp);
	DBG_OPT("  ah=%s", msg->add_conn.algorithms.ah);
	DBG_OPT("  dpddelay=%d", msg->add_conn.dpd.delay);
	DBG_OPT("  dpdtimeout=%d", msg->add_conn.dpd.timeout);
	DBG_OPT("  dpdaction=%d", msg->add_conn.dpd.action);
	DBG_OPT("  closeaction=%d", msg->add_conn.close_action);
	DBG_OPT("  mediation=%s", msg->add_conn.ikeme.mediation ? "yes" : "no");
	DBG_OPT("  mediated_by=%s", msg->add_conn.ikeme.mediated_by);
	DBG_OPT("  me_peerid=%s", msg->add_conn.ikeme.peerid);
	DBG_OPT("  keyexchange=ikev%u", msg->add_conn.version);

	this->config->add(this->config, msg);
	this->attribute->add_dns(this->attribute, msg);
	this->handler->add_attributes(this->handler, msg);
}

/**
 * Delete a connection from the list
 */
static void stroke_del_conn(private_stroke_socket_t *this, stroke_msg_t *msg)
{
	pop_string(msg, &msg->del_conn.name);
	DBG1(DBG_CFG, "received stroke: delete connection '%s'", msg->del_conn.name);

	this->config->del(this->config, msg);
	this->attribute->del_dns(this->attribute, msg);
	this->handler->del_attributes(this->handler, msg);
}

/**
 * initiate a connection by name
 */
static void stroke_initiate(private_stroke_socket_t *this, stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->initiate.name);
	DBG1(DBG_CFG, "received stroke: initiate '%s'", msg->initiate.name);

	this->control->initiate(this->control, msg, out);
}

/**
 * terminate a connection by name
 */
static void stroke_terminate(private_stroke_socket_t *this, stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->terminate.name);
	DBG1(DBG_CFG, "received stroke: terminate '%s'", msg->terminate.name);

	this->control->terminate(this->control, msg, out);
}

/**
 * terminate a connection by peers virtual IP
 */
static void stroke_terminate_srcip(private_stroke_socket_t *this,
								   stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->terminate_srcip.start);
	pop_string(msg, &msg->terminate_srcip.end);
	DBG1(DBG_CFG, "received stroke: terminate-srcip %s-%s",
		 msg->terminate_srcip.start, msg->terminate_srcip.end);

	this->control->terminate_srcip(this->control, msg, out);
}

/**
 * rekey a connection by name/id
 */
static void stroke_rekey(private_stroke_socket_t *this, stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->terminate.name);
	DBG1(DBG_CFG, "received stroke: rekey '%s'", msg->rekey.name);

	this->control->rekey(this->control, msg, out);
}

/**
 * route a policy (install SPD entries)
 */
static void stroke_route(private_stroke_socket_t *this, stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->route.name);
	DBG1(DBG_CFG, "received stroke: route '%s'", msg->route.name);

	this->control->route(this->control, msg, out);
}

/**
 * unroute a policy
 */
static void stroke_unroute(private_stroke_socket_t *this, stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->terminate.name);
	DBG1(DBG_CFG, "received stroke: unroute '%s'", msg->route.name);

	this->control->unroute(this->control, msg, out);
}

/**
 * Add a ca information record to the cainfo list
 */
static void stroke_add_ca(private_stroke_socket_t *this,
						  stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->add_ca.name);
	DBG1(DBG_CFG, "received stroke: add ca '%s'", msg->add_ca.name);

	pop_string(msg, &msg->add_ca.cacert);
	pop_string(msg, &msg->add_ca.crluri);
	pop_string(msg, &msg->add_ca.crluri2);
	pop_string(msg, &msg->add_ca.ocspuri);
	pop_string(msg, &msg->add_ca.ocspuri2);
	pop_string(msg, &msg->add_ca.certuribase);
	DBG2(DBG_CFG, "ca %s", msg->add_ca.name);
	DBG_OPT("  cacert=%s", msg->add_ca.cacert);
	DBG_OPT("  crluri=%s", msg->add_ca.crluri);
	DBG_OPT("  crluri2=%s", msg->add_ca.crluri2);
	DBG_OPT("  ocspuri=%s", msg->add_ca.ocspuri);
	DBG_OPT("  ocspuri2=%s", msg->add_ca.ocspuri2);
	DBG_OPT("  certuribase=%s", msg->add_ca.certuribase);

	this->ca->add(this->ca, msg);
}

/**
 * Delete a ca information record from the cainfo list
 */
static void stroke_del_ca(private_stroke_socket_t *this,
						  stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->del_ca.name);
	DBG1(DBG_CFG, "received stroke: delete ca '%s'", msg->del_ca.name);

	this->ca->del(this->ca, msg);
}


/**
 * show status of daemon
 */
static void stroke_status(private_stroke_socket_t *this,
						  stroke_msg_t *msg, FILE *out, bool all, bool wait)
{
	pop_string(msg, &(msg->status.name));

	this->list->status(this->list, msg, out, all, wait);
}

/**
 * list various information
 */
static void stroke_list(private_stroke_socket_t *this, stroke_msg_t *msg,
						FILE *out)
{
	if (msg->list.flags & LIST_CAINFOS)
	{
		this->ca->list(this->ca, msg, out);
	}
	this->list->list(this->list, msg, out);
}

/**
 * reread various information
 */
static void stroke_reread(private_stroke_socket_t *this,
						  stroke_msg_t *msg, FILE *out)
{
	this->cred->reread(this->cred, msg, out);
}

/**
 * purge various information
 */
static void stroke_purge(private_stroke_socket_t *this,
						 stroke_msg_t *msg, FILE *out)
{
	if (msg->purge.flags & PURGE_OCSP)
	{
		lib->credmgr->flush_cache(lib->credmgr, CERT_X509_OCSP_RESPONSE);
	}
	if (msg->purge.flags & PURGE_CRLS)
	{
		lib->credmgr->flush_cache(lib->credmgr, CERT_X509_CRL);
	}
	if (msg->purge.flags & PURGE_CERTS)
	{
		lib->credmgr->flush_cache(lib->credmgr, CERT_X509);
	}
	if (msg->purge.flags & PURGE_IKE)
	{
		this->control->purge_ike(this->control, msg, out);
	}
}

/**
 * Print a certificate in PEM to out
 */
static void print_pem_cert(FILE *out, certificate_t *cert)
{
	chunk_t encoded;

	if (cert->get_encoding(cert, CERT_PEM, &encoded))
	{
		fprintf(out, "%.*s", (int)encoded.len, encoded.ptr);
		free(encoded.ptr);
	}
}

/**
 * Export in-memory credentials
 */
static void stroke_export(private_stroke_socket_t *this,
						  stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->export.selector);

	if (msg->export.flags & EXPORT_X509)
	{
		enumerator_t *enumerator;
		identification_t *id;
		certificate_t *cert;

		id = identification_create_from_string(msg->export.selector);
		enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
												CERT_X509, KEY_ANY, id, FALSE);
		while (enumerator->enumerate(enumerator, &cert))
		{
			print_pem_cert(out, cert);
		}
		enumerator->destroy(enumerator);
		id->destroy(id);
	}

	if (msg->export.flags & (EXPORT_CONN_CERT | EXPORT_CONN_CHAIN))
	{
		enumerator_t *sas, *auths, *certs;
		ike_sa_t *ike_sa;
		auth_cfg_t *auth;
		certificate_t *cert;
		auth_rule_t rule;

		sas = charon->ike_sa_manager->create_enumerator(
												charon->ike_sa_manager, TRUE);
		while (sas->enumerate(sas, &ike_sa))
		{
			if (streq(msg->export.selector, ike_sa->get_name(ike_sa)))
			{
				auths = ike_sa->create_auth_cfg_enumerator(ike_sa, FALSE);
				while (auths->enumerate(auths, &auth))
				{
					bool got_subject = FALSE;

					certs = auth->create_enumerator(auth);
					while (certs->enumerate(certs, &rule, &cert))
					{
						switch (rule)
						{
							case AUTH_RULE_CA_CERT:
							case AUTH_RULE_IM_CERT:
								if (msg->export.flags & EXPORT_CONN_CHAIN)
								{
									print_pem_cert(out, cert);
								}
								break;
							case AUTH_RULE_SUBJECT_CERT:
								if (!got_subject)
								{
									print_pem_cert(out, cert);
									got_subject = TRUE;
								}
								break;
							default:
								break;
						}
					}
					certs->destroy(certs);
				}
				auths->destroy(auths);
			}
		}
		sas->destroy(sas);
	}
}

/**
 * list pool leases
 */
static void stroke_leases(private_stroke_socket_t *this,
						  stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->leases.pool);
	pop_string(msg, &msg->leases.address);

	this->list->leases(this->list, msg, out);
}

/**
 * Callback function for usage report
 */
static void report_usage(FILE *out, int count, size_t bytes,
						 backtrace_t *bt, bool detailed)
{
	fprintf(out, "%zu bytes total, %d allocations, %zu bytes average:\n",
			bytes, count, bytes / count);
	bt->log(bt, out, detailed);
}

/**
 * Callback function for memusage summary
 */
static void sum_usage(FILE *out, int count, size_t bytes, int whitelisted)
{
	fprintf(out, "Total memory usage: %zu\n", bytes);
}

/**
 * Show memory usage
 */
static void stroke_memusage(private_stroke_socket_t *this,
							stroke_msg_t *msg, FILE *out)
{
	if (lib->leak_detective)
	{
		lib->leak_detective->usage(lib->leak_detective,
								   (leak_detective_report_cb_t)report_usage,
								   (leak_detective_summary_cb_t)sum_usage, out);
	}
}

/**
 * Set username and password for a connection
 */
static void stroke_user_creds(private_stroke_socket_t *this,
							  stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->user_creds.name);
	pop_string(msg, &msg->user_creds.username);
	pop_string(msg, &msg->user_creds.password);

	DBG1(DBG_CFG, "received stroke: user-creds '%s'", msg->user_creds.name);

	this->config->set_user_credentials(this->config, msg, out);
}

/**
 * Print stroke counter values
 */
static void stroke_counters(private_stroke_socket_t *this,
							  stroke_msg_t *msg, FILE *out)
{
	pop_string(msg, &msg->counters.name);

	if (msg->counters.reset)
	{
		this->counter->reset(this->counter, msg->counters.name);
	}
	else
	{
		this->counter->print(this->counter, out, msg->counters.name);
	}
}

/**
 * set the verbosity debug output
 */
static void stroke_loglevel(private_stroke_socket_t *this,
							stroke_msg_t *msg, FILE *out)
{
	debug_t group;

	pop_string(msg, &(msg->loglevel.type));
	DBG1(DBG_CFG, "received stroke: loglevel %d for %s",
		 msg->loglevel.level, msg->loglevel.type);

	if (this->prevent_loglevel_changes)
	{
		DBG1(DBG_CFG, "prevented log level change");
		fprintf(out, "command not allowed!\n");
		return;
	}
	if (strcaseeq(msg->loglevel.type, "any"))
	{
		group = DBG_ANY;
	}
	else
	{
		if (!enum_from_name(debug_names, msg->loglevel.type, &group))
		{
			fprintf(out, "unknown type '%s'!\n", msg->loglevel.type);
			return;
		}
	}
	charon->set_level(charon, group, msg->loglevel.level);
}

/**
 * set various config options
 */
static void stroke_config(private_stroke_socket_t *this,
						  stroke_msg_t *msg, FILE *out)
{
	this->cred->cachecrl(this->cred, msg->config.cachecrl);
}

/**
 * process a stroke request
 */
static bool on_accept(private_stroke_socket_t *this, stream_t *stream)
{
	stroke_msg_t *msg;
	u_int16_t len;
	FILE *out;

	/* read length */
	if (!stream->read_all(stream, &len, sizeof(len)))
	{
		if (errno != EWOULDBLOCK)
		{
			DBG1(DBG_CFG, "reading length of stroke message failed: %s",
				 strerror(errno));
		}
		return FALSE;
	}

	/* read message (we need an additional byte to terminate the buffer) */
	msg = malloc(len + 1);
	msg->length = len;
	if (!stream->read_all(stream, (char*)msg + sizeof(len), len - sizeof(len)))
	{
		if (errno != EWOULDBLOCK)
		{
			DBG1(DBG_CFG, "reading stroke message failed: %s", strerror(errno));
		}
		free(msg);
		return FALSE;
	}
	/* make sure even incorrectly unterminated strings don't extend over the
	 * message boundaries */
	((char*)msg)[len] = '\0';

	DBG3(DBG_CFG, "stroke message %b", (void*)msg, len);

	out = stream->get_file(stream);
	if (!out)
	{
		DBG1(DBG_CFG, "creating stroke output stream failed");
		free(msg);
		return FALSE;
	}
	switch (msg->type)
	{
		case STR_INITIATE:
			stroke_initiate(this, msg, out);
			break;
		case STR_ROUTE:
			stroke_route(this, msg, out);
			break;
		case STR_UNROUTE:
			stroke_unroute(this, msg, out);
			break;
		case STR_TERMINATE:
			stroke_terminate(this, msg, out);
			break;
		case STR_TERMINATE_SRCIP:
			stroke_terminate_srcip(this, msg, out);
			break;
		case STR_REKEY:
			stroke_rekey(this, msg, out);
			break;
		case STR_STATUS:
			stroke_status(this, msg, out, FALSE, TRUE);
			break;
		case STR_STATUS_ALL:
			stroke_status(this, msg, out, TRUE, TRUE);
			break;
		case STR_STATUS_ALL_NOBLK:
			stroke_status(this, msg, out, TRUE, FALSE);
			break;
		case STR_ADD_CONN:
			stroke_add_conn(this, msg);
			break;
		case STR_DEL_CONN:
			stroke_del_conn(this, msg);
			break;
		case STR_ADD_CA:
			stroke_add_ca(this, msg, out);
			break;
		case STR_DEL_CA:
			stroke_del_ca(this, msg, out);
			break;
		case STR_LOGLEVEL:
			stroke_loglevel(this, msg, out);
			break;
		case STR_CONFIG:
			stroke_config(this, msg, out);
			break;
		case STR_LIST:
			stroke_list(this, msg, out);
			break;
		case STR_REREAD:
			stroke_reread(this, msg, out);
			break;
		case STR_PURGE:
			stroke_purge(this, msg, out);
			break;
		case STR_EXPORT:
			stroke_export(this, msg, out);
			break;
		case STR_LEASES:
			stroke_leases(this, msg, out);
			break;
		case STR_MEMUSAGE:
			stroke_memusage(this, msg, out);
			break;
		case STR_USER_CREDS:
			stroke_user_creds(this, msg, out);
			break;
		case STR_COUNTERS:
			stroke_counters(this, msg, out);
			break;
		default:
			DBG1(DBG_CFG, "received unknown stroke");
			break;
	}
	free(msg);
	fclose(out);
	return FALSE;
}

METHOD(stroke_socket_t, destroy, void,
	private_stroke_socket_t *this)
{
	DESTROY_IF(this->service);
	lib->credmgr->remove_set(lib->credmgr, &this->ca->set);
	lib->credmgr->remove_set(lib->credmgr, &this->cred->set);
	charon->backends->remove_backend(charon->backends, &this->config->backend);
	hydra->attributes->remove_provider(hydra->attributes, &this->attribute->provider);
	hydra->attributes->remove_handler(hydra->attributes, &this->handler->handler);
	charon->bus->remove_listener(charon->bus, &this->counter->listener);
	this->cred->destroy(this->cred);
	this->ca->destroy(this->ca);
	this->config->destroy(this->config);
	this->attribute->destroy(this->attribute);
	this->handler->destroy(this->handler);
	this->control->destroy(this->control);
	this->list->destroy(this->list);
	this->counter->destroy(this->counter);
	free(this);
}

/*
 * see header file
 */
stroke_socket_t *stroke_socket_create()
{
	private_stroke_socket_t *this;
	int max_concurrent;
	char *uri;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.prevent_loglevel_changes = lib->settings->get_bool(lib->settings,
				"%s.plugins.stroke.prevent_loglevel_changes", FALSE, lib->ns),
	);

	this->cred = stroke_cred_create();
	this->attribute = stroke_attribute_create();
	this->handler = stroke_handler_create();
	this->ca = stroke_ca_create(this->cred);
	this->config = stroke_config_create(this->ca, this->cred, this->attribute);
	this->control = stroke_control_create();
	this->list = stroke_list_create(this->attribute);
	this->counter = stroke_counter_create();

	lib->credmgr->add_set(lib->credmgr, &this->ca->set);
	lib->credmgr->add_set(lib->credmgr, &this->cred->set);
	charon->backends->add_backend(charon->backends, &this->config->backend);
	hydra->attributes->add_provider(hydra->attributes, &this->attribute->provider);
	hydra->attributes->add_handler(hydra->attributes, &this->handler->handler);
	charon->bus->add_listener(charon->bus, &this->counter->listener);

	max_concurrent = lib->settings->get_int(lib->settings,
				"%s.plugins.stroke.max_concurrent", MAX_CONCURRENT_DEFAULT,
				lib->ns);
	uri = lib->settings->get_str(lib->settings,
				"%s.plugins.stroke.socket", "unix://" STROKE_SOCKET, lib->ns);
	this->service = lib->streams->create_service(lib->streams, uri, 10);
	if (!this->service)
	{
		DBG1(DBG_CFG, "creating stroke socket failed");
		destroy(this);
		return NULL;
	}
	this->service->on_accept(this->service, (stream_service_cb_t)on_accept,
							 this, JOB_PRIO_CRITICAL, max_concurrent);

	return &this->public;
}
