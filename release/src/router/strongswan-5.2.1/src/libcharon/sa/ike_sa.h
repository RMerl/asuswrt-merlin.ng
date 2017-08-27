/*
 * Copyright (C) 2006-2014 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005-2009 Martin Willi
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
 * @defgroup ike_sa ike_sa
 * @{ @ingroup sa
 */

#ifndef IKE_SA_H_
#define IKE_SA_H_

typedef enum ike_extension_t ike_extension_t;
typedef enum ike_condition_t ike_condition_t;
typedef enum ike_sa_state_t ike_sa_state_t;
typedef enum statistic_t statistic_t;
typedef struct ike_sa_t ike_sa_t;

#include <library.h>
#include <attributes/attribute_handler.h>
#include <encoding/message.h>
#include <encoding/payloads/proposal_substructure.h>
#include <encoding/payloads/configuration_attribute.h>
#include <sa/ike_sa_id.h>
#include <sa/child_sa.h>
#include <sa/task.h>
#include <sa/task_manager.h>
#include <sa/keymat.h>
#include <config/peer_cfg.h>
#include <config/ike_cfg.h>
#include <credentials/auth_cfg.h>
#include <networking/packet.h>

/**
 * Timeout in seconds after that a half open IKE_SA gets deleted.
 */
#define HALF_OPEN_IKE_SA_TIMEOUT 30

/**
 * Interval to send keepalives when NATed, in seconds.
 */
#define KEEPALIVE_INTERVAL 20

/**
 * After which time rekeying should be retried if it failed, in seconds.
 */
#define RETRY_INTERVAL 30

/**
 * Jitter to subtract from RETRY_INTERVAL to randomize rekey retry.
 */
#define RETRY_JITTER 20

/**
 * Extensions (or optional features) the peer supports
 */
enum ike_extension_t {

	/**
	 * peer supports NAT traversal as specified in RFC4306 or RFC3947
	 * including some RFC3947 drafts
	 */
	EXT_NATT = (1<<0),

	/**
	 * peer supports MOBIKE (RFC4555)
	 */
	EXT_MOBIKE = (1<<1),

	/**
	 * peer supports HTTP cert lookups as specified in RFC4306
	 */
	EXT_HASH_AND_URL = (1<<2),

	/**
	 * peer supports multiple authentication exchanges, RFC4739
	 */
	EXT_MULTIPLE_AUTH = (1<<3),

	/**
	 * peer uses strongSwan, accept private use extensions
	 */
	EXT_STRONGSWAN = (1<<4),

	/**
	 * peer supports EAP-only authentication, draft-eronen-ipsec-ikev2-eap-auth
	 */
	EXT_EAP_ONLY_AUTHENTICATION = (1<<5),

	/**
	 * peer is probably a Windows RAS client
	 */
	EXT_MS_WINDOWS = (1<<6),

	/**
	 * peer supports XAuth authentication, draft-ietf-ipsec-isakmp-xauth-06
	 */
	EXT_XAUTH = (1<<7),

	/**
	 * peer supports DPD detection, RFC 3706 (or IKEv2)
	 */
	EXT_DPD = (1<<8),

	/**
	 * peer supports Cisco Unity configuration attributes
	 */
	EXT_CISCO_UNITY = (1<<9),

	/**
	 * peer supports NAT traversal as specified in
	 * draft-ietf-ipsec-nat-t-ike-02 .. -03
	 */
	EXT_NATT_DRAFT_02_03 = (1<<10),

	/**
	 * peer supports proprietary IKEv1 or standardized IKEv2 fragmentation
	 */
	EXT_IKE_FRAGMENTATION = (1<<11),
};

/**
 * Conditions of an IKE_SA, change during its lifetime
 */
enum ike_condition_t {

	/**
	 * Connection is natted (or faked) somewhere
	 */
	COND_NAT_ANY = (1<<0),

	/**
	 * we are behind NAT
	 */
	COND_NAT_HERE = (1<<1),

	/**
	 * other is behind NAT
	 */
	COND_NAT_THERE = (1<<2),

	/**
	 * Faking NAT to enforce UDP encapsulation
	 */
	COND_NAT_FAKE = (1<<3),

	/**
	 * peer has been authenticated using EAP at least once
	 */
	COND_EAP_AUTHENTICATED = (1<<4),

	/**
	 * received a certificate request from the peer
	 */
	COND_CERTREQ_SEEN = (1<<5),

	/**
	 * Local peer is the "original" IKE initiator. Unaffected from rekeying.
	 */
	COND_ORIGINAL_INITIATOR = (1<<6),

	/**
	 * IKE_SA is stale, the peer is currently unreachable (MOBIKE)
	 */
	COND_STALE = (1<<7),

	/**
	 * Initial contact received
	 */
	COND_INIT_CONTACT_SEEN = (1<<8),

	/**
	 * Peer has been authenticated using XAuth
	 */
	COND_XAUTH_AUTHENTICATED = (1<<9),

	/**
	 * This IKE_SA is currently being reauthenticated
	 */
	COND_REAUTHENTICATING = (1<<10),
};

/**
 * Timing information and statistics to query from an SA
 */
enum statistic_t {
	/** Timestamp of SA establishement */
	STAT_ESTABLISHED = 0,
	/** Timestamp of scheduled rekeying */
	STAT_REKEY,
	/** Timestamp of scheduled reauthentication */
	STAT_REAUTH,
	/** Timestamp of scheduled delete */
	STAT_DELETE,
	/** Timestamp of last inbound IKE packet */
	STAT_INBOUND,
	/** Timestamp of last outbound IKE packet */
	STAT_OUTBOUND,

	STAT_MAX
};

/**
 * State of an IKE_SA.
 *
 * An IKE_SA passes various states in its lifetime. A newly created
 * SA is in the state CREATED.
 * @verbatim
                 +----------------+
                 ¦   SA_CREATED   ¦
                 +----------------+
                         ¦
    on initiate()--->    ¦   <----- on IKE_SA_INIT received
                         V
                 +----------------+
                 ¦ SA_CONNECTING  ¦
                 +----------------+
                         ¦
                         ¦   <----- on IKE_AUTH successfully completed
                         V
                 +----------------+
                 ¦ SA_ESTABLISHED ¦-------------------------+ <-- on rekeying
                 +----------------+                         ¦
                         ¦                                  V
    on delete()--->      ¦   <----- on IKE_SA        +-------------+
                         ¦          delete request   ¦ SA_REKEYING ¦
                         ¦          received         +-------------+
                         V                                  ¦
                 +----------------+                         ¦
                 ¦  SA_DELETING   ¦<------------------------+ <-- after rekeying
                 +----------------+
                         ¦
                         ¦   <----- after delete() acknowledged
                         ¦
                        \V/
                         X
                        / \
   @endverbatim
 */
enum ike_sa_state_t {

	/**
	 * IKE_SA just got created, but is not initiating nor responding yet.
	 */
	IKE_CREATED,

	/**
	 * IKE_SA gets initiated actively or passively
	 */
	IKE_CONNECTING,

	/**
	 * IKE_SA is fully established
	 */
	IKE_ESTABLISHED,

	/**
	 * IKE_SA is managed externally and does not process messages
	 */
	IKE_PASSIVE,

	/**
	 * IKE_SA rekeying in progress
	 */
	IKE_REKEYING,

	/**
	 * IKE_SA is in progress of deletion
	 */
	IKE_DELETING,

	/**
	 * IKE_SA object gets destroyed
	 */
	IKE_DESTROYING,
};

/**
 * enum names for ike_sa_state_t.
 */
extern enum_name_t *ike_sa_state_names;

/**
 * Class ike_sa_t representing an IKE_SA.
 *
 * An IKE_SA contains crypto information related to a connection
 * with a peer. It contains multiple IPsec CHILD_SA, for which
 * it is responsible. All traffic is handled by an IKE_SA, using
 * the task manager and its tasks.
 */
struct ike_sa_t {

	/**
	 * Get the id of the SA.
	 *
	 * Returned ike_sa_id_t object is not getting cloned!
	 *
	 * @return				ike_sa's ike_sa_id_t
	 */
	ike_sa_id_t* (*get_id) (ike_sa_t *this);

	/**
	 * Gets the IKE version of the SA
	 */
	ike_version_t (*get_version)(ike_sa_t *this);

	/**
	 * Get the numerical ID uniquely defining this IKE_SA.
	 *
	 * @return				unique ID
	 */
	u_int32_t (*get_unique_id) (ike_sa_t *this);

	/**
	 * Get the state of the IKE_SA.
	 *
	 * @return				state of the IKE_SA
	 */
	ike_sa_state_t (*get_state) (ike_sa_t *this);

	/**
	 * Set the state of the IKE_SA.
	 *
	 * @param state			state to set for the IKE_SA
	 */
	void (*set_state) (ike_sa_t *this, ike_sa_state_t state);

	/**
	 * Get the name of the connection this IKE_SA uses.
	 *
	 * @return				name
	 */
	char* (*get_name) (ike_sa_t *this);

	/**
	 * Get statistic values from the IKE_SA.
	 *
	 * @param kind			kind of requested value
	 * @return				value as integer
	 */
	u_int32_t (*get_statistic)(ike_sa_t *this, statistic_t kind);

	/**
	 * Set statistic value of the IKE_SA.
	 *
	 * @param kind			kind of value to update
	 * @param value			value as integer
	 */
	void (*set_statistic)(ike_sa_t *this, statistic_t kind, u_int32_t value);

	/**
	 * Get the own host address.
	 *
	 * @return				host address
	 */
	host_t* (*get_my_host) (ike_sa_t *this);

	/**
	 * Set the own host address.
	 *
	 * @param me			host address
	 */
	void (*set_my_host) (ike_sa_t *this, host_t *me);

	/**
	 * Get the other peers host address.
	 *
	 * @return				host address
	 */
	host_t* (*get_other_host) (ike_sa_t *this);

	/**
	 * Set the others host address.
	 *
	 * @param other			host address
	 */
	void (*set_other_host) (ike_sa_t *this, host_t *other);

	/**
	 * Float to port 4500 (e.g. if a NAT is detected).
	 *
	 * The port of either endpoint is changed only if it is currently
	 * set to the default value of 500.
	 */
	void (*float_ports)(ike_sa_t *this);

	/**
	 * Update the IKE_SAs host.
	 *
	 * Hosts may be NULL to use current host.
	 *
	 * @param me			new local host address, or NULL
	 * @param other			new remote host address, or NULL
	 * @param force			force update
	 */
	void (*update_hosts)(ike_sa_t *this, host_t *me, host_t *other, bool force);

	/**
	 * Get the own identification.
	 *
	 * @return				identification
	 */
	identification_t* (*get_my_id) (ike_sa_t *this);

	/**
	 * Set the own identification.
	 *
	 * @param me			identification
	 */
	void (*set_my_id) (ike_sa_t *this, identification_t *me);

	/**
	 * Get the other peer's identification.
	 *
	 * @return				identification
	 */
	identification_t* (*get_other_id) (ike_sa_t *this);

	/**
	 * Get the others peer identity, but prefer an EAP-Identity.
	 *
	 * @return				EAP or IKEv2 identity
	 */
	identification_t* (*get_other_eap_id)(ike_sa_t *this);

	/**
	 * Set the other peer's identification.
	 *
	 * @param other			identification
	 */
	void (*set_other_id) (ike_sa_t *this, identification_t *other);

	/**
	 * Get the config used to setup this IKE_SA.
	 *
	 * @return				ike_config
	 */
	ike_cfg_t* (*get_ike_cfg) (ike_sa_t *this);

	/**
	 * Set the config to setup this IKE_SA.
	 *
	 * @param config		ike_config to use
	 */
	void (*set_ike_cfg) (ike_sa_t *this, ike_cfg_t* config);

	/**
	 * Get the peer config used by this IKE_SA.
	 *
	 * @return				peer_config
	 */
	peer_cfg_t* (*get_peer_cfg) (ike_sa_t *this);

	/**
	 * Set the peer config to use with this IKE_SA.
	 *
	 * @param config		peer_config to use
	 */
	void (*set_peer_cfg) (ike_sa_t *this, peer_cfg_t *config);

	/**
	 * Get the authentication config with rules of the current auth round.
	 *
	 * @param local			TRUE for local rules, FALSE for remote constraints
	 * @return				current cfg
	 */
	auth_cfg_t* (*get_auth_cfg)(ike_sa_t *this, bool local);

	/**
	 * Insert a completed authentication round.
	 *
	 * @param local			TRUE for own rules, FALSE for others constraints
	 * @param cfg			auth config to append
	 */
	void (*add_auth_cfg)(ike_sa_t *this, bool local, auth_cfg_t *cfg);

	/**
	 * Create an enumerator over added authentication rounds.
	 *
	 * @param local			TRUE for own rules, FALSE for others constraints
	 * @return				enumerator over auth_cfg_t
	 */
	enumerator_t* (*create_auth_cfg_enumerator)(ike_sa_t *this, bool local);

	/**
	 * Get the selected proposal of this IKE_SA.
	 *
	 * @return				selected proposal
	 */
	proposal_t* (*get_proposal)(ike_sa_t *this);

	/**
	 * Set the proposal selected for this IKE_SA.
	 *
	 * @param				selected proposal
	 */
	void (*set_proposal)(ike_sa_t *this, proposal_t *proposal);

	/**
	 * Set the message id of the IKE_SA.
	 *
	 * The IKE_SA stores two message IDs, one for initiating exchanges (send)
	 * and one to respond to exchanges (expect).
	 *
	 * @param initiate		TRUE to set message ID for initiating
	 * @param mid			message id to set
	 */
	void (*set_message_id)(ike_sa_t *this, bool initiate, u_int32_t mid);

	/**
	 * Add an additional address for the peer.
	 *
	 * In MOBIKE, a peer may transmit additional addresses where it is
	 * reachable. These are stored in the IKE_SA.
	 * The own list of addresses is not stored, they are queried from
	 * the kernel when required.
	 *
	 * @param host			host to add to list
	 */
	void (*add_peer_address)(ike_sa_t *this, host_t *host);

	/**
	 * Create an enumerator over all known addresses of the peer.
	 *
	 * @return				enumerator over addresses
	 */
	enumerator_t* (*create_peer_address_enumerator)(ike_sa_t *this);

	/**
	 * Remove all known addresses of the peer.
	 */
	void (*clear_peer_addresses)(ike_sa_t *this);

	/**
	 * Check if mappings have changed on a NAT for our source address.
	 *
	 * @param hash			received DESTINATION_IP hash
	 * @return				TRUE if mappings have changed
	 */
	bool (*has_mapping_changed)(ike_sa_t *this, chunk_t hash);

	/**
	 * Enable an extension the peer supports.
	 *
	 * If support for an IKE extension is detected, this method is called
	 * to enable that extension and behave accordingly.
	 *
	 * @param extension		extension to enable
	 */
	void (*enable_extension)(ike_sa_t *this, ike_extension_t extension);

	/**
	 * Check if the peer supports an extension.
	 *
	 * @param extension		extension to check for support
	 * @return				TRUE if peer supports it, FALSE otherwise
	 */
	bool (*supports_extension)(ike_sa_t *this, ike_extension_t extension);

	/**
	 * Enable/disable a condition flag for this IKE_SA.
	 *
	 * @param condition		condition to enable/disable
	 * @param enable		TRUE to enable condition, FALSE to disable
	 */
	void (*set_condition) (ike_sa_t *this, ike_condition_t condition, bool enable);

	/**
	 * Check if a condition flag is set.
	 *
	 * @param condition		condition to check
	 * @return				TRUE if condition flag set, FALSE otherwise
	 */
	bool (*has_condition) (ike_sa_t *this, ike_condition_t condition);

	/**
	 * Get the number of queued MOBIKE address updates.
	 *
	 * @return				number of pending updates
	 */
	u_int32_t (*get_pending_updates)(ike_sa_t *this);

	/**
	 * Set the number of queued MOBIKE address updates.
	 *
	 * @param updates		number of pending updates
	 */
	void (*set_pending_updates)(ike_sa_t *this, u_int32_t updates);

#ifdef ME
	/**
	 * Activate mediation server functionality for this IKE_SA.
	 */
	void (*act_as_mediation_server) (ike_sa_t *this);

	/**
	 * Get the server reflexive host.
	 *
	 * @return				server reflexive host
	 */
	host_t* (*get_server_reflexive_host) (ike_sa_t *this);

	/**
	 * Set the server reflexive host.
	 *
	 * @param host			server reflexive host
	 */
	void (*set_server_reflexive_host) (ike_sa_t *this, host_t *host);

	/**
	 * Get the connect ID.
	 *
	 * @return				connect ID
	 */
	chunk_t (*get_connect_id) (ike_sa_t *this);

	/**
	 * Initiate the mediation of a mediated connection (i.e. initiate a
	 * ME_CONNECT exchange to a mediation server).
	 *
	 * @param mediated_cfg	peer_cfg of the mediated connection
	 * @return
	 *						- SUCCESS if initialization started
	 *						- DESTROY_ME if initialization failed
	 */
	status_t (*initiate_mediation) (ike_sa_t *this, peer_cfg_t *mediated_cfg);

	/**
	 * Initiate the mediated connection
	 *
	 * @param me			local endpoint (gets cloned)
	 * @param other			remote endpoint (gets cloned)
	 * @param connect_id	connect ID (gets cloned)
	 * @return
	 *						- SUCCESS if initialization started
	 *						- DESTROY_ME if initialization failed
	 */
	status_t (*initiate_mediated) (ike_sa_t *this, host_t *me, host_t *other,
								   chunk_t connect_id);

	/**
	 * Relay data from one peer to another (i.e. initiate a ME_CONNECT exchange
	 * to a peer).
	 *
	 * Data is cloned.
	 *
	 * @param requester		ID of the requesting peer
	 * @param connect_id	data of the ME_CONNECTID payload
	 * @param connect_key	data of the ME_CONNECTKEY payload
	 * @param endpoints		endpoints
	 * @param response		TRUE if this is a response
	 * @return
	 *						- SUCCESS if relay started
	 *						- DESTROY_ME if relay failed
	 */
	status_t (*relay) (ike_sa_t *this, identification_t *requester,
					   chunk_t connect_id, chunk_t connect_key,
					   linked_list_t *endpoints, bool response);

	/**
	 * Send a callback to a peer.
	 *
	 * Data is cloned.
	 *
	 * @param peer_id		ID of the other peer
	 * @return
	 *						- SUCCESS if response started
	 *						- DESTROY_ME if response failed
	 */
	status_t (*callback) (ike_sa_t *this, identification_t *peer_id);

	/**
	 * Respond to a ME_CONNECT request.
	 *
	 * Data is cloned.
	 *
	 * @param peer_id		ID of the other peer
	 * @param connect_id	the connect ID supplied by the initiator
	 * @return
	 *						- SUCCESS if response started
	 *						- DESTROY_ME if response failed
	 */
	status_t (*respond) (ike_sa_t *this, identification_t *peer_id,
						 chunk_t connect_id);
#endif /* ME */

	/**
	 * Initiate a new connection.
	 *
	 * The configs are owned by the IKE_SA after the call. If the initiate
	 * is triggered by a packet, traffic selectors of the packet can be added
	 * to the CHILD_SA.
	 *
	 * @param child_cfg		child config to create CHILD from
	 * @param reqid			reqid to use for CHILD_SA, 0 assigne uniquely
	 * @param tsi			source of triggering packet
	 * @param tsr			destination of triggering packet.
	 * @return
	 *						- SUCCESS if initialization started
	 *						- DESTROY_ME if initialization failed
	 */
	status_t (*initiate) (ike_sa_t *this, child_cfg_t *child_cfg,
						  u_int32_t reqid, traffic_selector_t *tsi,
						  traffic_selector_t *tsr);

	/**
	 * Retry initiation of this IKE_SA after it got deferred previously.
	 *
	 * @return
	 *						- SUCCESS if initiation deferred or started
	 *						- DESTROY_ME if initiation failed
	 */
	status_t (*retry_initiate) (ike_sa_t *this);

	/**
	 * Initiates the deletion of an IKE_SA.
	 *
	 * Sends a delete message to the remote peer and waits for
	 * its response. If the response comes in, or a timeout occurs,
	 * the IKE SA gets deleted.
	 *
	 * @return
	 *						- SUCCESS if deletion is initialized
	 *						- DESTROY_ME, if the IKE_SA is not in
	 *						  an established state and can not be
	 *						  deleted (but destroyed).
	 */
	status_t (*delete) (ike_sa_t *this);

	/**
	 * Update IKE_SAs after network interfaces have changed.
	 *
	 * Whenever the network interface configuration changes, the kernel
	 * interface calls roam() on each IKE_SA. The IKE_SA then checks if
	 * the new network config requires changes, and handles appropriate.
	 * If MOBIKE is supported, addresses are updated; If not, the tunnel is
	 * restarted.
	 *
	 * @param address		TRUE if address list changed, FALSE otherwise
	 * @return				SUCCESS, FAILED, DESTROY_ME
	 */
	status_t (*roam)(ike_sa_t *this, bool address);

	/**
	 * Processes an incoming IKE message.
	 *
	 * Message processing may fail. If a critical failure occurs,
	 * process_message() return DESTROY_ME. Then the caller must
	 * destroy the IKE_SA immediately, as it is unusable.
	 *
	 * @param message		message to process
	 * @return
	 *						- SUCCESS
	 *						- FAILED
	 *						- DESTROY_ME if this IKE_SA MUST be deleted
	 */
	status_t (*process_message)(ike_sa_t *this, message_t *message);

	/**
	 * Generate an IKE message to send it to the peer.
	 *
	 * This method generates all payloads in the message and encrypts/signs
	 * the packet.
	 *
	 * @param message		message to generate
	 * @param packet		generated output packet
	 * @return
	 *						- SUCCESS
	 *						- FAILED
	 *						- DESTROY_ME if this IKE_SA MUST be deleted
	 */
	status_t (*generate_message)(ike_sa_t *this, message_t *message,
								 packet_t **packet);

	/**
	 * Generate an IKE message to send it to the peer. If enabled and supported
	 * it will be fragmented.
	 *
	 * This method generates all payloads in the message and encrypts/signs
	 * the packet/fragments.
	 *
	 * @param message		message to generate
	 * @param packets		enumerator of generated packet_t* (are not destroyed
	 *						with the enumerator)
	 * @return
	 *						- SUCCESS
	 *						- FAILED
	 *						- DESTROY_ME if this IKE_SA MUST be deleted
	 */
	status_t (*generate_message_fragmented)(ike_sa_t *this, message_t *message,
											enumerator_t **packets);

	/**
	 * Retransmits a request.
	 *
	 * @param message_id	ID of the request to retransmit
	 * @return
	 *						- SUCCESS
	 *						- NOT_FOUND if request doesn't have to be retransmited
	 */
	status_t (*retransmit) (ike_sa_t *this, u_int32_t message_id);

	/**
	 * Sends a DPD request to the peer.
	 *
	 * To check if a peer is still alive, periodic
	 * empty INFORMATIONAL messages are sent if no
	 * other traffic was received.
	 *
	 * @return
	 *						- SUCCESS
	 *						- DESTROY_ME, if peer did not respond
	 */
	status_t (*send_dpd) (ike_sa_t *this);

	/**
	 * Sends a keep alive packet.
	 *
	 * To refresh NAT tables in a NAT router between the peers, periodic empty
	 * UDP packets are sent if no other traffic was sent.
	 */
	void (*send_keepalive) (ike_sa_t *this);

	/**
	 * Get the keying material of this IKE_SA.
	 *
	 * @return				per IKE_SA keymat instance
	 */
	keymat_t* (*get_keymat)(ike_sa_t *this);

	/**
	 * Associates a child SA to this IKE SA
	 *
	 * @param child_sa		child_sa to add
	 */
	void (*add_child_sa) (ike_sa_t *this, child_sa_t *child_sa);

	/**
	 * Get a CHILD_SA identified by protocol and SPI.
	 *
	 * @param protocol		protocol of the SA
	 * @param spi			SPI of the CHILD_SA
	 * @param inbound		TRUE if SPI is inbound, FALSE if outbound
	 * @return				child_sa, or NULL if none found
	 */
	child_sa_t* (*get_child_sa) (ike_sa_t *this, protocol_id_t protocol,
								 u_int32_t spi, bool inbound);

	/**
	 * Get the number of CHILD_SAs.
	 *
	 * @return				number of CHILD_SAs
	 */
	int (*get_child_count) (ike_sa_t *this);

	/**
	 * Create an enumerator over all CHILD_SAs.
	 *
	 * @return				enumerator
	 */
	enumerator_t* (*create_child_sa_enumerator) (ike_sa_t *this);

	/**
	 * Remove the CHILD_SA the given enumerator points to from this IKE_SA.
	 *
	 * @param enumerator	enumerator pointing to CHILD_SA
	 */
	void (*remove_child_sa) (ike_sa_t *this, enumerator_t *enumerator);

	/**
	 * Rekey the CHILD SA with the specified reqid.
	 *
	 * Looks for a CHILD SA owned by this IKE_SA, and start the rekeing.
	 *
	 * @param protocol		protocol of the SA
	 * @param spi			inbound SPI of the CHILD_SA
	 * @return
	 *						- NOT_FOUND, if IKE_SA has no such CHILD_SA
	 *						- SUCCESS, if rekeying initiated
	 */
	status_t (*rekey_child_sa) (ike_sa_t *this, protocol_id_t protocol, u_int32_t spi);

	/**
	 * Close the CHILD SA with the specified protocol/SPI.
	 *
	 * Looks for a CHILD SA owned by this IKE_SA, deletes it and
	 * notify's the remote peer about the delete. The associated
	 * states and policies in the kernel get deleted, if they exist.
	 *
	 * @param protocol		protocol of the SA
	 * @param spi			inbound SPI of the CHILD_SA
	 * @param expired		TRUE if CHILD_SA is expired
	 * @return
	 *						- NOT_FOUND, if IKE_SA has no such CHILD_SA
	 *						- SUCCESS, if delete message sent
	 */
	status_t (*delete_child_sa)(ike_sa_t *this, protocol_id_t protocol,
								u_int32_t spi, bool expired);

	/**
	 * Destroy a CHILD SA with the specified protocol/SPI.
	 *
	 * Looks for a CHILD SA owned by this IKE_SA and destroys it.
	 *
	 * @param protocol		protocol of the SA
	 * @param spi			inbound SPI of the CHILD_SA
	 * @return
	 *						- NOT_FOUND, if IKE_SA has no such CHILD_SA
	 *						- SUCCESS
	 */
	status_t (*destroy_child_sa) (ike_sa_t *this, protocol_id_t protocol, u_int32_t spi);

	/**
	 * Rekey the IKE_SA.
	 *
	 * Sets up a new IKE_SA, moves all CHILDs to it and deletes this IKE_SA.
	 *
	 * @return				- SUCCESS, if IKE_SA rekeying initiated
	 */
	status_t (*rekey) (ike_sa_t *this);

	/**
	 * Reauthenticate the IKE_SA.
	 *
	 * Create a completely new IKE_SA with authentication, recreates all children
	 * within the IKE_SA, closes this IKE_SA.
	 *
	 * @return				DESTROY_ME to destroy the IKE_SA
	 */
	status_t (*reauth) (ike_sa_t *this);

	/**
	 * Restablish the IKE_SA.
	 *
	 * Reestablish an IKE_SA after it has been closed.
	 *
	 * @return				DESTROY_ME to destroy the IKE_SA
	 */
	status_t (*reestablish) (ike_sa_t *this);

	/**
	 * Set the lifetime limit received/to send in a AUTH_LIFETIME notify.
	 *
	 * If the IKE_SA is already ESTABLISHED, an INFORMATIONAL is sent with
	 * an AUTH_LIFETIME notify. The call never fails on unestablished SAs.
	 *
	 * @param lifetime		lifetime in seconds
	 * @return				DESTROY_ME to destroy the IKE_SA
	 */
	status_t (*set_auth_lifetime)(ike_sa_t *this, u_int32_t lifetime);

	/**
	 * Add a virtual IP to use for this IKE_SA and its children.
	 *
	 * The virtual IP is assigned per IKE_SA, not per CHILD_SA. It has the same
	 * lifetime as the IKE_SA.
	 *
	 * @param local			TRUE to set local address, FALSE for remote
	 * @param ip			IP to set as virtual IP
	 */
	void (*add_virtual_ip) (ike_sa_t *this, bool local, host_t *ip);

	/**
	 * Clear all virtual IPs stored on this IKE_SA.
	 *
	 * @param local			TRUE to clear local addresses, FALSE for remote
	 */
	void (*clear_virtual_ips) (ike_sa_t *this, bool local);

	/**
	 * Create an enumerator over virtual IPs.
	 *
	 * @param local			TRUE to get local virtual IP, FALSE for remote
	 * @return				enumerator over host_t*
	 */
	enumerator_t* (*create_virtual_ip_enumerator) (ike_sa_t *this, bool local);

	/**
	 * Register a configuration attribute to the IKE_SA.
	 *
	 * If an IRAS sends a configuration attribute it is installed and
	 * registered at the IKE_SA. Attributes are inherit()ed and get released
	 * when the IKE_SA is closed.
	 *
	 * Unhandled attributes are passed as well, but with a NULL handler. They
	 * do not get released.
	 *
	 * @param handler		handler installed the attribute, use for release()
	 * @param type			configuration attribute type
	 * @param data			associated attribute data
	 */
	void (*add_configuration_attribute)(ike_sa_t *this,
							attribute_handler_t *handler,
							configuration_attribute_type_t type, chunk_t data);

	/**
	 * Create an enumerator over received configuration attributes.
	 *
	 * The resulting enumerator is over the configuration_attribute_type_t type,
	 * a value chunk_t followed by a bool flag. The boolean flag indicates if
	 * the attribute has been handled by an attribute handler.
	 *
	 * @return				enumerator over type, value and the "handled" flag.
	 */
	enumerator_t* (*create_attribute_enumerator)(ike_sa_t *this);

	/**
	 * Set local and remote host addresses to be used for IKE.
	 *
	 * These addresses are communicated via the KMADDRESS field of a MIGRATE
	 * message sent via the NETLINK or PF _KEY kernel socket interface.
	 *
	 * @param local			local kmaddress
	 * @param remote		remote kmaddress
	 */
	void (*set_kmaddress) (ike_sa_t *this, host_t *local, host_t *remote);

	/**
	 * Create enumerator over a task queue of this IKE_SA.
	 *
	 * @param				queue type to enumerate
	 * @return				enumerator over task_t
	 */
	enumerator_t* (*create_task_enumerator)(ike_sa_t *this, task_queue_t queue);

	/**
	 * Flush a task queue, cancelling all tasks in it.
	 *
	 * @param queue			queue type to flush
	 */
	void (*flush_queue)(ike_sa_t *this, task_queue_t queue);

	/**
	 * Queue a task for initiaton to the task manager.
	 *
	 * @param task			task to queue
	 */
	void (*queue_task)(ike_sa_t *this, task_t *task);

	/**
	 * Inherit required attributes to new SA before rekeying.
	 *
	 * Some properties of the SA must be applied before starting IKE_SA
	 * rekeying, such as the configuration or support extensions.
	 *
	 * @param other			other IKE_SA to inherit from
	 */
	void (*inherit_pre)(ike_sa_t *this, ike_sa_t *other);

	/**
	 * Inherit all attributes of other to this after rekeying.
	 *
	 * When rekeying is completed, all CHILD_SAs, the virtual IP and all
	 * outstanding tasks are moved from other to this.
	 *
	 * @param other			other IKE SA to inherit from
	 */
	void (*inherit_post) (ike_sa_t *this, ike_sa_t *other);

	/**
	 * Reset the IKE_SA, useable when initiating fails
	 */
	void (*reset) (ike_sa_t *this);

	/**
	 * Destroys a ike_sa_t object.
	 */
	void (*destroy) (ike_sa_t *this);
};

/**
 * Creates an ike_sa_t object with a specific ID and IKE version.
 *
 * @param ike_sa_id		ike_sa_id_t to associate with new IKE_SA/ISAKMP_SA
 * @param initiator		TRUE to create this IKE_SA as initiator
 * @param version		IKE version of this SA
 * @return				ike_sa_t object
 */
ike_sa_t *ike_sa_create(ike_sa_id_t *ike_sa_id, bool initiator,
						ike_version_t version);

#endif /** IKE_SA_H_ @}*/
