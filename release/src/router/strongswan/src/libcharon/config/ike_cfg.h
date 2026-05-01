/*
 * Copyright (C) 2012-2019 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

/**
 * @defgroup ike_cfg ike_cfg
 * @{ @ingroup config
 */

#ifndef IKE_CFG_H_
#define IKE_CFG_H_

typedef enum ike_version_t ike_version_t;
typedef enum fragmentation_t fragmentation_t;
typedef enum childless_t childless_t;
typedef struct ike_cfg_t ike_cfg_t;
typedef struct ike_cfg_create_t ike_cfg_create_t;

#include <library.h>
#include <networking/host.h>
#include <collections/linked_list.h>
#include <utils/identification.h>
#include <crypto/proposal/proposal.h>

/**
 * IKE version.
 */
enum ike_version_t {
	/** any version */
	IKE_ANY = 0,
	/** IKE version 1 */
	IKEV1 = 1,
	/** IKE version 2 */
	IKEV2 = 2,
};

/**
 * Proprietary IKEv1 fragmentation and IKEv2 fragmentation
 */
enum fragmentation_t {
	/** disable fragmentation */
	FRAGMENTATION_NO,
	/** announce support, but don't send any fragments */
	FRAGMENTATION_ACCEPT,
	/** enable fragmentation, if supported by peer */
	FRAGMENTATION_YES,
	/** force use of fragmentation (even for the first message for IKEv1) */
	FRAGMENTATION_FORCE,
};

/**
 * Childless IKE_SAs (RFC 6023)
 */
enum childless_t {
	/** Allow childless IKE_SAs as responder, but initiate regular IKE_SAs */
	CHILDLESS_ALLOW,
	/** Initiate childless IKE_SAs if supported, allow them as responder */
	CHILDLESS_PREFER,
	/** Don't accept childless IKE_SAs as responder, don't initiate them */
	CHILDLESS_NEVER,
	/** Only accept the creation of childless IKE_SAs (also as responder) */
	CHILDLESS_FORCE,
};

/**
 * enum strings for ike_version_t
 */
extern enum_name_t *ike_version_names;

/**
 * An ike_cfg_t defines the rules to set up an IKE_SA.
 *
 * @see peer_cfg_t to get an overview over the configurations.
 */
struct ike_cfg_t {

	/**
	 * Get the IKE version to use with this configuration.
	 *
	 * @return				IKE major version
	 */
	ike_version_t (*get_version)(ike_cfg_t *this);

	/**
	 * Resolve the local address to use for initiation.
	 *
	 * @param family		address family to prefer, or AF_UNSPEC
	 * @return				resolved host, NULL on error
	 */
	host_t* (*resolve_me)(ike_cfg_t *this, int family);

	/**
	 * Resolve the remote address to use for initiation.
	 *
	 * @param family		address family to prefer, or AF_UNSPEC
	 * @return				resolved host, NULL on error
	 */
	host_t* (*resolve_other)(ike_cfg_t *this, int family);

	/**
	 * Check how good a host matches to the configured local address.
	 *
	 * @param host			host to check match quality
	 * @return				quality of the match, 0 if not matching at all
	 */
	u_int (*match_me)(ike_cfg_t *this, host_t *host);

	/**
	 * Check how good a host matches to the configured remote address.
	 *
	 * @param host			host to check match quality
	 * @return				quality of the match, 0 if not matching at all
	 */
	u_int (*match_other)(ike_cfg_t *this, host_t *host);

	/**
	 * Get own address.
	 *
	 * @return				string of address/DNS name
	 */
	char* (*get_my_addr) (ike_cfg_t *this);

	/**
	 * Get peer's address.
	 *
	 * @return				string of address/DNS name
	 */
	char* (*get_other_addr) (ike_cfg_t *this);

	/**
	 * Get the port to use as our source port.
	 *
	 * @return				source address port, host order
	 */
	uint16_t (*get_my_port)(ike_cfg_t *this);

	/**
	 * Get the port to use as destination port.
	 *
	 * @return				destination address, host order
	 */
	uint16_t (*get_other_port)(ike_cfg_t *this);

	/**
	 * Get the DSCP value to use for IKE packets send from connections.
	 *
	 * @return				DSCP value
	 */
	uint8_t (*get_dscp)(ike_cfg_t *this);

	/**
	 * Adds a proposal to the list.
	 *
	 * The first added proposal has the highest priority, the last
	 * added the lowest. It is safe to add NULL as proposal, which has no
	 * effect.
	 *
	 * @param proposal		proposal to add, or NULL
	 */
	void (*add_proposal) (ike_cfg_t *this, proposal_t *proposal);

	/**
	 * Returns a list of all supported proposals.
	 *
	 * Returned list and its proposals must be destroyed after use.
	 *
	 * @return				list containing all the proposals
	 */
	linked_list_t* (*get_proposals) (ike_cfg_t *this);

	/**
	 * Select a proposal from a list of supplied proposals.
	 *
	 * Returned proposal must be destroyed after use.
	 *
	 * @param proposals		list of proposals to select from
	 * @param flags			flags to consider during proposal selection
	 * @return				selected proposal, or NULL if none matches.
	 */
	proposal_t *(*select_proposal) (ike_cfg_t *this, linked_list_t *proposals,
									proposal_selection_flag_t flags);

	/**
	 * Check if the config has a matching proposal.
	 *
	 * @param match			proposal to check
	 * @param private		accept algorithms from a private range
	 * @return				TRUE if a matching proposal is contained
	 */
	bool(*has_proposal)(ike_cfg_t *this, proposal_t *match, bool private);

	/**
	 * Should we send a certificate request in IKE_SA_INIT?
	 *
	 * @return				certificate request sending policy
	 */
	bool (*send_certreq) (ike_cfg_t *this);

	/**
	 * Should we send an OCSP status request in IKE_SA_INIT?
	 *
	 * @return				OCSP status request sending policy
	 */
	bool (*send_ocsp_certreq) (ike_cfg_t *this);

	/**
	 * Enforce UDP encapsulation by faking NATD notifies?
	 *
	 * @return				TRUE to enforce UDP encapsulation
	 */
	bool (*force_encap) (ike_cfg_t *this);

	/**
	 * Use IKE fragmentation
	 *
	 * @return				TRUE to use fragmentation
	 */
	fragmentation_t (*fragmentation) (ike_cfg_t *this);

	/**
	 * Whether to initiate/accept childless IKE_SAs
	 *
	 * @return				initiate/accept childless IKE_SAs
	 */
	childless_t (*childless)(ike_cfg_t *this);

	/**
	 * Get the first algorithm of a certain transform type that's contained in
	 * any of the configured proposals.
	 *
	 * For instance, use with KEY_EXCHANGE_METHOD to get the KE method to use
	 * for the IKE_SA initiation.
	 *
	 * @param type			transform type to look for
	 * @return				algorithm identifier (0 for none)
	 */
	uint16_t (*get_algorithm)(ike_cfg_t *this, transform_type_t type);

	/**
	 * Check if two IKE configs are equal.
	 *
	 * @param other			other to check for equality
	 * @return				TRUE if other equal to this
	 */
	bool (*equals)(ike_cfg_t *this, ike_cfg_t *other);

	/**
	 * Increase reference count.
	 *
	 * @return				reference to this
	 */
	ike_cfg_t* (*get_ref) (ike_cfg_t *this);

	/**
	 * Destroys a ike_cfg_t object.
	 *
	 * Decrements the internal reference counter and
	 * destroys the ike_cfg when it reaches zero.
	 */
	void (*destroy) (ike_cfg_t *this);
};

/**
 * Data passed to the constructor of an ike_cfg_t object.
 *
 * local and remote are comma separated lists of IP addresses, DNS names,
 * IP ranges or subnets. When initiating, the first non-range/subnet address is
 * used as address. When responding, a match is performed against all items in
 * the list.
 */
struct ike_cfg_create_t {
	/** IKE major version to use for this config */
	ike_version_t version;
	/** Address/DNS name of local peer (cloned) */
	char *local;
	/** IKE port to use as source, 500 uses IKEv2 port floating */
	uint16_t local_port;
	/** Address/DNS name of remote peer (cloned) */
	char *remote;
	/** IKE port to use as dest, 500 uses IKEv2 port floating */
	uint16_t remote_port;
	/** TRUE to not send any certificate requests */
	bool no_certreq;
	/** TRUE to send OCSP status requests */
	bool ocsp_certreq;
	/** Enforce UDP encapsulation by faking NATD notify */
	bool force_encap;
	/** Use IKE fragmentation */
	fragmentation_t fragmentation;
	/** Childless IKE_SA configuration */
	childless_t childless;
	/** DSCP value to send IKE packets with */
	uint8_t dscp;
};

/**
 * Creates an ike_cfg_t object.
 *
 * @param data				data for this ike_cfg
 * @return					ike_cfg_t object.
 */
ike_cfg_t *ike_cfg_create(ike_cfg_create_t *data);

/**
 * Determine the address family of the local or remote address(es).  If multiple
 * families are configured AF_UNSPEC is returned.  %any is ignored (%any4|6 are
 * not though).
 *
 * @param this				ike config to check
 * @param local				TRUE to check local addresses, FALSE for remote
 * @return					address family of address(es) if distinct
 */
int ike_cfg_get_family(ike_cfg_t *this, bool local);

/**
 * Determine if the given address was explicitly configured as local or remote
 * address.
 *
 * @param this				ike config to check
 * @param addr				address to check
 * @param local				TRUE to check local addresses, FALSE for remote
 * @return					TRUE if address was configured
 */
bool ike_cfg_has_address(ike_cfg_t *this, host_t *addr, bool local);

#endif /** IKE_CFG_H_ @}*/
