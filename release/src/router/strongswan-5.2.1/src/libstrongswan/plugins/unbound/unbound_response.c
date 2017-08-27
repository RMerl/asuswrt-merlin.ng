/*
 * Copyright (C) 2012 Reto Guadagnini
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

#include <resolver/resolver_response.h>
#include <resolver/rr.h>
#include "unbound_rr.h"
#include "unbound_response.h"

#include <library.h>
#include <utils/debug.h>

#include <unbound.h>
#include <ldns/ldns.h>

typedef struct private_unbound_response_t private_unbound_response_t;

/**
 * private data of an unbound_response_t object.
 */
struct private_unbound_response_t {

	/**
	 * Public data
	 */
	unbound_response_t public;

	/**
	 * Original question string
	 */
	char* query_name;

	/**
	 * Canonical name of the response
	 */
	char* canon_name;

	/**
	 * Are the some RRs in the RRset of this response?
	 */
	bool has_data;

	/*
	 * Does the queried name exist?
	 */
	bool query_name_exist;

	/**
	 * DNSSEC security state
	 */
	dnssec_status_t security_state;

	/**
	 * RRset
	 */
	rr_set_t *rr_set;
};

METHOD(resolver_response_t, get_query_name, char*,
	private_unbound_response_t *this)
{
	return this->query_name;
}

METHOD(resolver_response_t, get_canon_name, char*,
	private_unbound_response_t *this)
{
	return this->canon_name;
}

METHOD(resolver_response_t, has_data, bool,
	private_unbound_response_t *this)
{
	return this->has_data;
}

METHOD(resolver_response_t, query_name_exist, bool,
	private_unbound_response_t *this)
{
	return this->query_name_exist;
}

METHOD(resolver_response_t, get_security_state, dnssec_status_t,
	private_unbound_response_t *this)
{
	return this->security_state;
}

METHOD(resolver_response_t, get_rr_set, rr_set_t*,
	private_unbound_response_t *this)
{
	return this->rr_set;
}

METHOD(resolver_response_t, destroy, void,
	private_unbound_response_t *this)
{
	free(this->query_name);
	free(this->canon_name);
	DESTROY_IF(this->rr_set);
	free(this);
}

/*
 * Described in header.
 */
unbound_response_t *unbound_response_create_frm_libub_response(
											struct ub_result *libub_response)
{
	private_unbound_response_t *this = NULL;

	INIT(this,
		.public = {
			.interface = {
				.get_query_name = _get_query_name,
				.get_canon_name = _get_canon_name,
				.has_data = _has_data,
				.query_name_exist = _query_name_exist,
				.get_security_state = _get_security_state,
				.get_rr_set = _get_rr_set,
				.destroy = _destroy,
			},
		},
	);

	this->query_name = strdup(libub_response->qname);

	if (libub_response->canonname)
	{
		this->canon_name = strdup(libub_response->canonname);
	}

	this->has_data = libub_response->havedata;

	this->query_name_exist = !(libub_response->nxdomain);

	if (libub_response->secure)
	{
		this->security_state = SECURE;
	}
	else if (libub_response->bogus)
	{
		this->security_state = BOGUS;
	}
	else
	{
		this->security_state = INDETERMINATE;
	}

	/**
	* Create RRset
	*/
	if (this->query_name_exist && this->has_data)
	{
		ldns_pkt *dns_pkt = NULL;
		ldns_rr_list *orig_rr_list = NULL;
		size_t orig_rr_count;
		ldns_rr *orig_rr = NULL;
		ldns_rdf *orig_rdf = NULL;
		ldns_status status;
		linked_list_t *rr_list = NULL, *rrsig_list = NULL;
		unbound_rr_t *rr = NULL;
		int i;

		/**Parse the received DNS packet using the ldns library */
		status = ldns_wire2pkt(&dns_pkt, libub_response->answer_packet,
							   libub_response->answer_len);

		if (status != LDNS_STATUS_OK)
		{
			DBG1(DBG_LIB, "failed to parse DNS packet");
			destroy(this);
			return NULL;
		}

		/* Create a list with the queried RRs. If there are corresponding RRSIGs
		 * create also a list with these.
		 */
		rr_list = linked_list_create();

		orig_rr_list = ldns_pkt_get_section_clone(dns_pkt, LDNS_SECTION_ANSWER);
		orig_rr_count = ldns_rr_list_rr_count(orig_rr_list);

		for (i = 0; i < orig_rr_count; i++)
		{
			orig_rr = ldns_rr_list_rr(orig_rr_list, i);

			if (ldns_rr_get_type(orig_rr) == libub_response->qtype &&
				ldns_rr_get_class(orig_rr) == libub_response->qclass)
			{
				/* RR is part of the queried RRset.
				 * => add it to the list of Resource Records.
				 */
				rr = unbound_rr_create_frm_ldns_rr(orig_rr);
				if (rr)
				{
					rr_list->insert_last(rr_list, rr);
				}
				else
				{
					DBG1(DBG_LIB, "failed to create RR");
				}
			}

			if (ldns_rr_get_type(orig_rr) == LDNS_RR_TYPE_RRSIG)
			{
				orig_rdf = ldns_rr_rrsig_typecovered(orig_rr);
				if (!orig_rdf)
				{
					DBG1(DBG_LIB, "failed to get the type covered by an RRSIG");
				}
				else if (ldns_rdf2native_int16(orig_rdf) == libub_response->qtype)
				{
					/* The current RR represent a signature (RRSIG)
					 * which belongs to the queried RRset.
					 * => add it to the list of signatures.
					 */
					rr = unbound_rr_create_frm_ldns_rr(orig_rr);
					if (rr)
					{
						if (!rrsig_list)
						{
							rrsig_list = linked_list_create();
						}
						rrsig_list->insert_last(rrsig_list, rr);
					}
					else
					{
						DBG1(DBG_LIB, "failed to create RRSIG");
					}
				}
				else
				{
					DBG1(DBG_LIB, "failed to determine the RR type "
								  "covered by RRSIG RR");
				}
			}
		}
		/**
		 * Create the RRset for which the query was performed.
		 */
		this->rr_set = rr_set_create(rr_list, rrsig_list);

		ldns_pkt_free(dns_pkt);
		ldns_rr_list_free(orig_rr_list);
	}
	return &this->public;
}
