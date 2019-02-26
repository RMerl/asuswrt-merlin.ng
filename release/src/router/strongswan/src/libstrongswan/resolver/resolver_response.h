/*
 * Copyright (C) 2012 Reto Guadagnini
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

/**
 * @defgroup rsolver_response resolver_response
 * @{ @ingroup resolver
 */

#ifndef RESOLVER_RESPONSE_H_
#define RESOLVER_RESPONSE_H_

typedef struct resolver_response_t resolver_response_t;
typedef enum dnssec_status_t dnssec_status_t;

#include <library.h>
#include <resolver/rr_set.h>

/**
 * DNSSEC security state.
 *
 * DNSSEC security state, which a security aware resolver is able determine
 * according to RFC 4033.
 */
enum dnssec_status_t {
	/**
	 * The validating resolver has a trust anchor, has a chain of
	 * trust, and is able to verify all the signatures in the response.
	 * [RFC4033]
	 */
	SECURE,
	/**
	 * The validating resolver has a trust anchor, a chain of
	 * trust, and, at some delegation point, signed proof of the
	 * non-existence of a DS record.  This indicates that subsequent
	 * branches in the tree are provably insecure.  A validating resolver
	 * may have a local policy to mark parts of the domain space as
	 * insecure. [RFC4033]
	 */
	INSECURE,
	/**
	 * The validating resolver has a trust anchor and a secure
	 * delegation indicating that subsidiary data is signed, but the
	 * response fails to validate for some reason: missing signatures,
	 * expired signatures, signatures with unsupported algorithms, data
	 * missing that the relevant NSEC RR says should be present, and so
	 * forth. [RFC4033]
	 */
	BOGUS,
	/**
	 * There is no trust anchor that would indicate that a
	 * specific portion of the tree is secure.  This is the default
	 * operation mode. [RFC4033]
	 */
	INDETERMINATE,
};


/**
 * A response of the DNS resolver to a DNS query.
 *
 * A response represents the answer of the Domain Name System to a query.
 * It contains the RRset with the queried Resource Records and additional
 * information.
 */
struct resolver_response_t {

    /**
     * Get the original question string.
     *
     * The string to which the returned pointer points, is still owned
	 * by the resolver_response. Clone it if necessary.
     *
     * @return			the queried name
     */
	char *(*get_query_name)(resolver_response_t *this);

	/**
	 * Get the canonical name of the result.
	 *
	 * The string to which the returned pointer points, is still owned
	 * by the resolver_response. Clone it if necessary.
	 *
	 * @return			- canonical name of result
	 * 					- NULL, if result has no canonical name
	 */
	char *(*get_canon_name)(resolver_response_t *this);

	/**
	 * Does the RRset of this response contain some Resource Records?
	 *
	 * Returns TRUE if the RRset of this response contains some RRs
	 * (RRSIG Resource Records are ignored).
	 *
	 * @return
	 * 					- TRUE, if there are some RRs in the RRset
	 * 					- FALSE, otherwise
	 */
	bool (*has_data)(resolver_response_t *this);

	/**
	 * Does the queried name exist?
	 *
	 * @return
	 * 					- TRUE, if the queried name exists
	 * 					- FALSE, otherwise
	 */
	bool (*query_name_exist)(resolver_response_t *this);

	/**
	 * Get the DNSSEC security state of the response.
	 *
	 * @return			DNSSEC security state
	 */
	dnssec_status_t (*get_security_state)(resolver_response_t *this);

	/**
	 * Get the RRset with all Resource Records of this response.
	 *
	 * @return			- RRset
	 * 					- NULL if there is no data or the query name
	 * 					  does not exist
	 */
	rr_set_t *(*get_rr_set)(resolver_response_t *this);

	/**
	 * Destroy this response.
	 */
	void (*destroy) (resolver_response_t *this);
};

#endif /** RR_SET_H_ @}*/
