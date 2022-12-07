/*
 * Copyright (C) 2007-2019 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
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
 * @defgroup peer_cfg peer_cfg
 * @{ @ingroup config
 */

#ifndef PEER_CFG_H_
#define PEER_CFG_H_

typedef enum cert_policy_t cert_policy_t;
typedef enum unique_policy_t unique_policy_t;
typedef struct peer_cfg_t peer_cfg_t;
typedef struct peer_cfg_create_t peer_cfg_create_t;

#include <library.h>
#include <utils/identification.h>
#include <collections/enumerator.h>
#include <selectors/traffic_selector.h>
#include <crypto/proposal/proposal.h>
#include <config/ike_cfg.h>
#include <config/child_cfg.h>
#include <credentials/auth_cfg.h>

/**
 * Certificate sending policy. This is also used for certificate
 * requests when using this definition for the other peer. If
 * it is CERT_NEVER_SEND, a certreq is omitted, otherwise its
 * included.
 *
 * @warning These definitions must be the same as in pluto/starter,
 * as they are sent over the stroke socket.
 */
enum cert_policy_t {
	/** always send certificates, even when not requested */
	CERT_ALWAYS_SEND =		0,
	/** send certificate upon cert request */
	CERT_SEND_IF_ASKED =	1,
	/** never send a certificate, even when requested */
	CERT_NEVER_SEND =		2,
};

/**
 * enum strings for cert_policy_t
 */
extern enum_name_t *cert_policy_names;

/**
 * Uniqueness of an IKE_SA, used to drop multiple connections with one peer.
 */
enum unique_policy_t {
	/** never check for client uniqueness */
	UNIQUE_NEVER,
	/** only check for client uniqueness when receiving an INITIAL_CONTACT */
	UNIQUE_NO,
	/** replace existing IKE_SAs when new ones get established by a client */
	UNIQUE_REPLACE,
	/** keep existing IKE_SAs, close the new ones on connection attempt */
	UNIQUE_KEEP,
};

/**
 * enum strings for unique_policy_t
 */
extern enum_name_t *unique_policy_names;

/**
 * Configuration of a peer, specified by IDs.
 *
 * The peer config defines a connection between two given IDs. It contains
 * exactly one ike_cfg_t, which is used for initiation. Additionally, it
 * contains multiple child_cfg_t defining which CHILD_SAs are allowed for this
 * peer.
 * @verbatim
                          +-------------------+        +---------------+
   +---------------+      |     peer_cfg      |      +---------------+ |
   |    ike_cfg    |      +-------------------+      |   child_cfg   | |
   +---------------+      | - ids             |      +---------------+ |
   | - hosts       | 1  1 | - cas             | 1  n | - proposals   | |
   | - proposals   |<-----| - auth info       |----->| - traffic sel | |
   | - ...         |      | - dpd config      |      | - ...         |-+
   +---------------+      | - ...             |      +---------------+
                          +-------------------+
                             | 1         0 |
                             |             |
                             v n         n V
             +-------------------+     +-------------------+
           +-------------------+ |   +-------------------+ |
           |     auth_cfg      | |   |     auth_cfg      | |
           +-------------------+ |   +-------------------+ |
           | - local rules     |-+   | - remote constr.  |-+
           +-------------------+     +-------------------+
   @endverbatim
 *
 * Each peer_cfg has two lists of authentication config attached. Local
 * authentication configs define how to authenticate ourself against the remote
 * peer. Each config is enforced using the multiple authentication extension
 * (RFC4739).
 * The remote authentication configs are handled as constraints. The peer has
 * to fulfill each of these rules (using multiple authentication, in any order)
 * to gain access to the configuration.
 */
struct peer_cfg_t {

	/**
	 * Get the name of the peer_cfg.
	 *
	 * Returned object is not getting cloned.
	 *
	 * @return				peer_cfg's name
	 */
	char* (*get_name) (peer_cfg_t *this);

	/**
	 * Get the IKE version to use for initiating.
	 *
	 * @return				IKE major version
	 */
	ike_version_t (*get_ike_version)(peer_cfg_t *this);

	/**
	 * Get the IKE config to use for initiation.
	 *
	 * @return				the IKE config to use
	 */
	ike_cfg_t* (*get_ike_cfg) (peer_cfg_t *this);

	/**
	 * Attach a CHILD config.
	 *
	 * @param child_cfg		CHILD config to add
	 */
	void (*add_child_cfg) (peer_cfg_t *this, child_cfg_t *child_cfg);

	/**
	 * Detach a CHILD config, pointed to by an enumerator.
	 *
	 * @param enumerator	enumerator indicating element position
	 */
	void (*remove_child_cfg)(peer_cfg_t *this, enumerator_t *enumerator);

	/**
	 * Replace the CHILD configs with those in the given PEER config.
	 *
	 * The enumerator enumerates the removed and added CHILD configs
	 * (child_cfg_t*, bool), where the flag is FALSE for removed configs and
	 * TRUE for added configs. Configs that are equal are not enumerated.
	 *
	 * @param other			other config to get CHILD configs from
	 * @return				an enumerator over removed/added CHILD configs
	 */
	enumerator_t* (*replace_child_cfgs)(peer_cfg_t *this, peer_cfg_t *other);

	/**
	 * Create an enumerator for all attached CHILD configs.
	 *
	 * @return				an enumerator over all CHILD configs.
	 */
	enumerator_t* (*create_child_cfg_enumerator) (peer_cfg_t *this);

	/**
	 * Select a CHILD config from received traffic selectors.
	 *
	 * @param my_ts			TS for local side
	 * @param other_ts		TS for remote side
	 * @param my_hosts		hosts to narrow down dynamic TS for local side
	 * @param other_hosts	hosts to narrow down dynamic TS for remote side
	 * @param my_labels		optional local security labels
	 * @param other_labels	optional remove security labels
	 * @return					selected CHILD config, or NULL if no match found
	 */
	child_cfg_t* (*select_child_cfg)(peer_cfg_t *this,
							linked_list_t *my_ts, linked_list_t *other_ts,
							linked_list_t *my_hosts, linked_list_t *other_hosts,
							linked_list_t *my_labels, linked_list_t *other_labels);

	/**
	 * Add an authentication config to the peer configuration.
	 *
	 * @param cfg			config to add
	 * @param local			TRUE for local rules, FALSE for remote constraints
	 */
	void (*add_auth_cfg)(peer_cfg_t *this, auth_cfg_t *cfg, bool local);

	/**
	 * Create an enumerator over registered authentication configs.
	 *
	 * @param local			TRUE for local rules, FALSE for remote constraints
	 * @return				enumerator over auth_cfg_t*
	 */
	enumerator_t* (*create_auth_cfg_enumerator)(peer_cfg_t *this, bool local);

	/**
	 * Should a certificate be sent for this connection?
	 *
	 * @return			certificate sending policy
	 */
	cert_policy_t (*get_cert_policy) (peer_cfg_t *this);

	/**
	 * How to handle uniqueness of IKE_SAs?
	 *
	 * @return			unique policy
	 */
	unique_policy_t (*get_unique_policy) (peer_cfg_t *this);

	/**
	 * Get the max number of retries after timeout.
	 *
	 * @return			max number retries
	 */
	uint32_t (*get_keyingtries) (peer_cfg_t *this);

	/**
	 * Get a time to start rekeying.
	 *
	 * @param jitter	subtract a jitter value to randomize time
	 * @return			time in s when to start rekeying, 0 disables rekeying
	 */
	uint32_t (*get_rekey_time)(peer_cfg_t *this, bool jitter);

	/**
	 * Get a time to start reauthentication.
	 *
	 * @param jitter	subtract a jitter value to randomize time
	 * @return			time in s when to start reauthentication, 0 disables it
	 */
	uint32_t (*get_reauth_time)(peer_cfg_t *this, bool jitter);

	/**
	 * Get the timeout of a rekeying/reauthenticating SA.
	 *
	 * @return			timeout in s
	 */
	uint32_t (*get_over_time)(peer_cfg_t *this);

	/**
	 * Use MOBIKE (RFC4555) if peer supports it?
	 *
	 * @return			TRUE to enable MOBIKE support
	 */
	bool (*use_mobike) (peer_cfg_t *this);

	/**
	 * Use/Accept aggressive mode with IKEv1?.
	 *
	 * @return			TRUE to use aggressive mode
	 */
	bool (*use_aggressive)(peer_cfg_t *this);

	/**
	 * Use pull or push mode for mode config?
	 *
	 * @return			TRUE to use pull, FALSE to use push mode
	 */
	bool (*use_pull_mode)(peer_cfg_t *this);

	/**
	 * Get the DPD check interval.
	 *
	 * @return			dpd_delay in seconds
	 */
	uint32_t (*get_dpd) (peer_cfg_t *this);

	/**
	 * Get the DPD timeout interval (IKEv1 only)
	 *
	 * @return			dpd_timeout in seconds
	 */
	uint32_t (*get_dpd_timeout) (peer_cfg_t *this);

	/**
	 * Add a virtual IP to request as initiator.
	 *
	 * @param vip			virtual IP to request, may be %any or %any6
	 */
	void (*add_virtual_ip)(peer_cfg_t *this, host_t *vip);

	/**
	 * Create an enumerator over virtual IPs to request.
	 *
	 * The returned enumerator enumerates over IPs added with add_virtual_ip().
	 *
	 * @return				enumerator over host_t*
	 */
	enumerator_t* (*create_virtual_ip_enumerator)(peer_cfg_t *this);

	/**
	 * Add a pool name this configuration uses to select virtual IPs.
	 *
	 * @param name			pool name to use for virtual IP lookup
	 */
	void (*add_pool)(peer_cfg_t *this, char *name);

	/**
	 * Create an enumerator over pool names of this config.
	 *
	 * @return				enumerator over char*
	 */
	enumerator_t* (*create_pool_enumerator)(peer_cfg_t *this);

	/**
	 * Optional interface ID to set on policies/SAs.
	 *
	 * @param inbound		TRUE for inbound, FALSE for outbound
	 * @return				interface ID
	 */
	uint32_t (*get_if_id)(peer_cfg_t *this, bool inbound);

	/**
	 * Get the PPK ID to use with this peer.
	 *
	 * @return				PPK id
	 */
	identification_t *(*get_ppk_id)(peer_cfg_t *this);

	/**
	 * Whether a PPK is required with this peer.
	 *
	 * @return				TRUE, if a PPK is required
	 */
	bool (*ppk_required)(peer_cfg_t *this);

#ifdef ME
	/**
	 * Is this a mediation connection?
	 *
	 * @return				TRUE, if this is a mediation connection
	 */
	bool (*is_mediation)(peer_cfg_t *this);

	/**
	 * Get name of the connection this one is mediated through.
	 *
	 * @return				the name of the mediation connection
	 */
	char* (*get_mediated_by)(peer_cfg_t *this);

	/**
	 * Get the id of the other peer at the mediation server.
	 *
	 * This is the leftid of the peer's connection with the mediation server.
	 *
	 * If it is not configured, it is assumed to be the same as the right id
	 * of this connection.
	 *
	 * @return				the id of the other peer
	 */
	identification_t* (*get_peer_id)(peer_cfg_t *this);
#endif /* ME */

	/**
	 * Check if two peer configurations are equal.
	 *
	 * This method does not compare associated ike/child_cfg.
	 *
	 * @param other			candidate to check for equality against this
	 * @return				TRUE if peer_cfg and ike_cfg are equal
	 */
	bool (*equals)(peer_cfg_t *this, peer_cfg_t *other);

	/**
	 * Increase reference count.
	 *
	 * @return				reference to this
	 */
	peer_cfg_t* (*get_ref) (peer_cfg_t *this);

	/**
	 * Destroys the peer_cfg object.
	 *
	 * Decrements the internal reference counter and
	 * destroys the peer_cfg when it reaches zero.
	 */
	void (*destroy) (peer_cfg_t *this);
};

/**
 * Data passed to the constructor of a peer_cfg_t object.
 */
struct peer_cfg_create_t {
	/** Whether to send a certificate payload */
	cert_policy_t cert_policy;
	/** Uniqueness of an IKE_SA */
	unique_policy_t unique;
	/** How many keying tries should be done before giving up */
	uint32_t keyingtries;
	/** Timeout in seconds before starting rekeying */
	uint32_t rekey_time;
	/** Timeout in seconds before starting reauthentication */
	uint32_t reauth_time;
	/** Time range in seconds to randomly subtract from rekey/reauth time */
	uint32_t jitter_time;
	/** Maximum overtime in seconds before closing a rekeying/reauth SA */
	uint32_t over_time;
	/** Disable MOBIKE (RFC4555) */
	bool no_mobike;
	/** Use/accept aggressive mode with IKEv1 */
	bool aggressive;
	/** TRUE to use modeconfig push, FALSE for pull */
	bool push_mode;
	/** DPD check interval, 0 to disable */
	uint32_t dpd;
	/** DPD timeout interval (IKEv1 only), if 0 default applies */
	uint32_t dpd_timeout;
	/** Optional inbound interface ID */
	uint32_t if_id_in;
	/** Optional outbound interface ID */
	uint32_t if_id_out;
	/** Postquantum Preshared Key ID (adopted) */
	identification_t *ppk_id;
	/** TRUE if a PPK is required, FALSE if it's optional */
	bool ppk_required;
#ifdef ME
	/** TRUE if this is a mediation connection */
	bool mediation;
	/** peer_cfg_t of the mediation connection to mediate through (cloned) */
	char *mediated_by;
	/** ID that identifies our peer at the mediation server (adopted) */
	identification_t *peer_id;
#endif /* ME */
};

/**
 * Create a configuration object for IKE_AUTH and later.
 *
 * @param name				name of the peer_cfg (cloned)
 * @param ike_cfg			IKE config to use when acting as initiator (adopted)
 * @param data				data for this peer_cfg
 * @return					peer_cfg_t object
 */
peer_cfg_t *peer_cfg_create(char *name, ike_cfg_t *ike_cfg,
							peer_cfg_create_t *data);

#endif /** PEER_CFG_H_ @}*/
