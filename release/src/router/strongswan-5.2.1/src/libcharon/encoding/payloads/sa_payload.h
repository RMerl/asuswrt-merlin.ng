/*
 * Copyright (C) 2005-2006 Martin Willi
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

/**
 * @defgroup sa_payload sa_payload
 * @{ @ingroup payloads
 */

#ifndef SA_PAYLOAD_H_
#define SA_PAYLOAD_H_

typedef struct sa_payload_t sa_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/proposal_substructure.h>
#include <collections/linked_list.h>
#include <kernel/kernel_ipsec.h>
#include <sa/authenticator.h>

/**
 * Class representing an IKEv1 or IKEv2 SA Payload.
 *
 * The SA Payload format is described in RFC section 3.3.
 */
struct sa_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Gets the proposals in this payload as a list.
	 *
	 * @return				a list containing proposal_ts
	 */
	linked_list_t *(*get_proposals) (sa_payload_t *this);

	/**
	 * Gets the proposals from the first proposal in this payload with IPComp
	 * enabled (IKEv1 only).
	 *
	 * @param cpi			the CPI of the first IPComp (sub)proposal
	 * @return				a list containing proposal_ts
	 */
	linked_list_t *(*get_ipcomp_proposals) (sa_payload_t *this, u_int16_t *cpi);

	/**
	 * Get the (shortest) lifetime of a proposal (IKEv1 only).
	 *
	 * @return					lifetime, in seconds
	 */
	u_int32_t (*get_lifetime)(sa_payload_t *this);

	/**
	 * Get the (shortest) life duration of a proposal (IKEv1 only).
	 *
	 * @return					life duration, in bytes
	 */
	u_int64_t (*get_lifebytes)(sa_payload_t *this);

	/**
	 * Get the first authentication method from the proposal (IKEv1 only).
	 *
	 * @return					auth method, or AUTH_NONE
	 */
	auth_method_t (*get_auth_method)(sa_payload_t *this);

	/**
	 * Get the (first) encapsulation mode from a proposal (IKEv1 only).
	 *
	 * @param udp				set to TRUE if UDP encapsulation used
	 * @return					ipsec encapsulation mode
	 */
	ipsec_mode_t (*get_encap_mode)(sa_payload_t *this, bool *udp);

	/**
	 * Create an enumerator over all proposal substructures.
	 *
	 * @return					enumerator over proposal_substructure_t
	 */
	enumerator_t* (*create_substructure_enumerator)(sa_payload_t *this);

	/**
	 * Destroys an sa_payload_t object.
	 */
	void (*destroy) (sa_payload_t *this);
};

/**
 * Creates an empty sa_payload_t object
 *
 * @param type				PLV2_SECURITY_ASSOCIATION or PLV1_SECURITY_ASSOCIATION
 * @return					created sa_payload_t object
 */
sa_payload_t *sa_payload_create(payload_type_t type);

/**
 * Creates an IKEv2 sa_payload_t object from a list of proposals.
 *
 * @param proposals			list of proposals to build the payload from
 * @return					sa_payload_t object
 */
sa_payload_t *sa_payload_create_from_proposals_v2(linked_list_t *proposals);

/**
 * Creates an IKEv2 sa_payload_t object from a single proposal.
 *
 * @param proposal			proposal from which the payload should be built.
 * @return					sa_payload_t object
 */
sa_payload_t *sa_payload_create_from_proposal_v2(proposal_t *proposal);

/**
 * Creates an IKEv1 sa_payload_t object from a list of proposals.
 *
 * @param proposals			list of proposals to build the payload from
 * @param lifetime			lifetime in seconds
 * @param lifebytes			lifebytes, in bytes
 * @param auth				authentication method to use, or AUTH_NONE
 * @param mode				IPsec encapsulation mode, TRANSPORT or TUNNEL
 * @param udp				ENCAP_UDP to use UDP encapsulation
 * @param cpi				CPI in case IPComp should be used
 * @return					sa_payload_t object
 */
sa_payload_t *sa_payload_create_from_proposals_v1(linked_list_t *proposals,
							u_int32_t lifetime, u_int64_t lifebytes,
							auth_method_t auth, ipsec_mode_t mode, encap_t udp,
							u_int16_t cpi);

/**
 * Creates an IKEv1 sa_payload_t object from a single proposal.
 *
 * @param proposal			proposal from which the payload should be built.
 * @param lifetime			lifetime in seconds
 * @param lifebytes			lifebytes, in bytes
 * @param auth				authentication method to use, or AUTH_NONE
 * @param mode				IPsec encapsulation mode, TRANSPORT or TUNNEL
 * @param udp				ENCAP_UDP to use UDP encapsulation
 * @param cpi				CPI in case IPComp should be used
 * @return					sa_payload_t object
 */
sa_payload_t *sa_payload_create_from_proposal_v1(proposal_t *proposal,
							u_int32_t lifetime, u_int64_t lifebytes,
							auth_method_t auth, ipsec_mode_t mode, encap_t udp,
							u_int16_t cpi);

#endif /** SA_PAYLOAD_H_ @}*/
