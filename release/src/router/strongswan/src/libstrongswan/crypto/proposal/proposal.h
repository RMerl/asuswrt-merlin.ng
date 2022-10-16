/*
 * Copyright (C) 2009-2020 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
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
 * @defgroup proposal proposal
 * @{ @ingroup crypto
 */

#ifndef PROPOSAL_H_
#define PROPOSAL_H_

typedef enum protocol_id_t protocol_id_t;
typedef enum proposal_selection_flag_t proposal_selection_flag_t;
typedef enum extended_sequence_numbers_t extended_sequence_numbers_t;
typedef struct proposal_t proposal_t;

#include <library.h>
#include <utils/identification.h>
#include <collections/linked_list.h>
#include <networking/host.h>
#include <crypto/transform.h>
#include <crypto/crypters/crypter.h>
#include <crypto/signers/signer.h>
#include <crypto/diffie_hellman.h>

/**
 * Protocol ID of a proposal.
 */
enum protocol_id_t {
	PROTO_NONE = 0,
	PROTO_IKE = 1,
	PROTO_AH = 2,
	PROTO_ESP = 3,
	PROTO_IPCOMP = 4, /* IKEv1 only */
};

/**
 * enum names for protocol_id_t
 */
extern enum_name_t *protocol_id_names;

/**
 * Flags for selecting proposals.
 */
enum proposal_selection_flag_t {
	/** Whether to prefer configured (default) or supplied proposals. */
	PROPOSAL_PREFER_SUPPLIED = (1<<0),
	/** Whether to skip and ignore algorithms from a private range. */
	PROPOSAL_SKIP_PRIVATE = (1<<1),
	/** Whether to skip and ignore diffie hellman groups. */
	PROPOSAL_SKIP_DH = (1<<2),
};

/**
 * Stores a set of algorithms used for an SA.
 *
 * A proposal stores algorithms for a specific protocol.
 * Proposals with multiple protocols are not supported, as that's not specified
 * in RFC 7296 anymore.
 */
struct proposal_t {

	/**
	 * Add an algorithm to the proposal.
	 *
	 * The algorithms are stored by priority, first added is the most preferred.
	 * Key size is only needed for encryption algorithms with variable key
	 * size (such as AES). Must be set to zero if the key size is not specified.
	 *
	 * @param type			kind of algorithm
	 * @param alg			identifier for algorithm
	 * @param key_size		key size to use
	 */
	void (*add_algorithm) (proposal_t *this, transform_type_t type,
						   uint16_t alg, uint16_t key_size);

	/**
	 * Get an enumerator over algorithms for a specific transform type.
	 *
	 * @param type			kind of algorithm
	 * @return				enumerator over uint16_t alg, uint16_t key_size
	 */
	enumerator_t *(*create_enumerator) (proposal_t *this, transform_type_t type);

	/**
	 * Get the algorithm for a type to use.
	 *
	 * If there are multiple algorithms, only the first is returned.
	 *
	 * @param type			kind of algorithm
	 * @param alg			pointer which receives algorithm
	 * @param key_size		pointer which receives the key size
	 * @return				TRUE if algorithm of this kind available
	 */
	bool (*get_algorithm) (proposal_t *this, transform_type_t type,
						   uint16_t *alg, uint16_t *key_size);

	/**
	 * Check if the proposal has a specific DH group.
	 *
	 * @param group			group to check for
	 * @return				TRUE if algorithm included
	 */
	bool (*has_dh_group)(proposal_t *this, diffie_hellman_group_t group);

	/**
	 * Move the given DH group to the front of the list if it was contained in
	 * the proposal.
	 *
	 * @param group			group to promote
	 * @return				TRUE if algorithm included
	 */
	bool (*promote_dh_group)(proposal_t *this, diffie_hellman_group_t group);

	/**
	 * Compare two proposals and select a matching subset.
	 *
	 * If the proposals are for the same protocols (AH/ESP), they are
	 * compared. If they have at least one algorithm of each type
	 * in common, a resulting proposal of this kind is created.
	 *
	 * Unless the flag PROPOSAL_PREFER_SUPPLIED is set, other is expected to be
	 * the remote proposal from which to copy SPI and proposal number to the
	 * result, otherwise copy from this proposal.
	 *
	 * @param other			proposal to compare against
	 * @param flags			flags to consider during proposal selection
	 * @return				selected proposal, NULL if proposals don't match
	 */
	proposal_t *(*select)(proposal_t *this, proposal_t *other,
						  proposal_selection_flag_t flags);

	/**
	 * Check if the given proposal matches this proposal.
	 *
	 * This is similar to select, but no resulting proposal is selected.
	 *
	 * @param other			proposal to compare against
	 * @param flags			flags to consider during proposal selection
	 * @return				TRUE if the proposals match
	 */
	bool (*matches)(proposal_t *this, proposal_t *other,
					proposal_selection_flag_t flags);

	/**
	 * Get the protocol ID of the proposal.
	 *
	 * @return				protocol of the proposal
	 */
	protocol_id_t (*get_protocol) (proposal_t *this);

	/**
	 * Get the SPI of the proposal.
	 *
	 * @return				SPI of the proposal
	 */
	uint64_t (*get_spi) (proposal_t *this);

	/**
	 * Set the SPI of the proposal.
	 *
	 * @param spi			SPI to set for proposal
	 */
	void (*set_spi) (proposal_t *this, uint64_t spi);

	/**
	 * Get the proposal number, as encoded in the SA payload.
	 *
	 * @return				proposal number
	 */
	uint8_t (*get_number)(proposal_t *this);

	/**
	 * Get number of the transform on which this proposal is based (IKEv1 only)
	 *
	 * @return				transform number (or 0)
	 */
	uint8_t (*get_transform_number)(proposal_t *this);

	/**
	 * Check for the equality of two proposals.
	 *
	 * @param other			other proposal to check for equality
	 * @return				TRUE if proposals are equal
	 */
	bool (*equals)(proposal_t *this, proposal_t *other);

	/**
	 * Clone a proposal.
	 *
	 * @param flags			flags to consider during cloning
	 * @return				clone of proposal
	 */
	proposal_t *(*clone)(proposal_t *this, proposal_selection_flag_t flags);

	/**
	 * Destroys the proposal object.
	 */
	void (*destroy) (proposal_t *this);
};

/**
 * Create a proposal for IKE, ESP or AH.
 *
 * @param protocol			protocol, such as PROTO_ESP
 * @param number			proposal number, as encoded in SA payload
 * @return					proposal_t object
 */
proposal_t *proposal_create(protocol_id_t protocol, uint8_t number);

/**
 * Create a proposal for IKE, ESP or AH that includes a transform number.
 *
 * @param protocol			protocol, such as PROTO_ESP
 * @param number			proposal number, as encoded in SA payload
 * @param transform			transform number, as encoded in payload
 * @return					proposal_t object
 */
proposal_t *proposal_create_v1(protocol_id_t protocol, uint8_t number,
							   uint8_t transform);

/**
 * Create a default proposal.
 *
 * @param protocol			protocol, such as PROTO_ESP
 * @return					proposal_t object
 */
proposal_t *proposal_create_default(protocol_id_t protocol);

/**
 * Create a default proposal for supported AEAD algorithms.
 *
 * @param protocol			protocol, such as PROTO_ESP
 * @return					proposal_t object, NULL if none supported
 */
proposal_t *proposal_create_default_aead(protocol_id_t protocol);

/**
 * Create a proposal from a string identifying the algorithms.
 *
 * Each algorithm identifier is separated with a '-' character e.g.
 * aes256-sha2-curve25519. Multiple algorithms of the same transform type can be
 * given (they don't have to be grouped together), the order is preserved e.g.
 * curve25519-sha2-aes256-sha1-modp3072-aes128 is the same as
 * aes256-aes128-sha2-sha1-curve25519-modp3072.
 *
 * The proposal is validated (e.g. PROTO_IKE proposals must contain a key
 * exchange method, AEAD algorithms can't be combined with classic encryption
 * algorithms etc.) and in some cases modified (e.g. by adding missing PRFs for
 * PROTO_IKE, or by adding noesn in PROTO_ESP/AH proposals if neither esn, nor
 * noesn is contained in the string etc.).
 *
 * @param protocol			protocol, such as PROTO_ESP
 * @param algs				algorithms as string
 * @return					proposal_t object, NULL if invalid
 */
proposal_t *proposal_create_from_string(protocol_id_t protocol,
										const char *algs);

/**
 * Select a common proposal from the given lists of proposals.
 *
 * @param configured		list of configured/local proposals
 * @param supplied			list of supplied/remote proposals
 * @param flags				flags to consider during proposal selection
 * @return					selected proposal, or NULL (allocated)
 */
proposal_t *proposal_select(linked_list_t *configured, linked_list_t *supplied,
							proposal_selection_flag_t flags);

/**
 * printf hook function for proposal_t.
 *
 * Arguments are:
 *	proposal_t*
 * With the #-specifier, arguments are:
 *	linked_list_t* (containing proposal_t*)
 */
int proposal_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
						 const void *const *args);

#endif /** PROPOSAL_H_ @}*/
