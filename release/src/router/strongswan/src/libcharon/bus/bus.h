/*
 * Copyright (C) 2012-2016 Tobias Brunner
 * Copyright (C) 2006-2009 Martin Willi
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
 * @defgroup bus bus
 * @{ @ingroup libcharon
 */

#ifndef BUS_H_
#define BUS_H_

typedef enum alert_t alert_t;
typedef enum narrow_hook_t narrow_hook_t;
typedef struct bus_t bus_t;

#include <stdarg.h>

#include <utils/debug.h>
#include <sa/ike_sa.h>
#include <sa/child_sa.h>
#include <processing/jobs/job.h>
#include <bus/listeners/logger.h>
#include <bus/listeners/listener.h>

/* undefine the definitions from libstrongswan */
#undef DBG0
#undef DBG1
#undef DBG2
#undef DBG3
#undef DBG4

#ifndef DEBUG_LEVEL
# define DEBUG_LEVEL 4
#endif /* DEBUG_LEVEL */

#if DEBUG_LEVEL >= 0
#define DBG0(group, format, ...) charon->bus->log(charon->bus, group, 0, format, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL >= 0 */
#if DEBUG_LEVEL >= 1
#define DBG1(group, format, ...) charon->bus->log(charon->bus, group, 1, format, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL >= 1 */
#if DEBUG_LEVEL >= 2
#define DBG2(group, format, ...) charon->bus->log(charon->bus, group, 2, format, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL >= 2 */
#if DEBUG_LEVEL >= 3
#define DBG3(group, format, ...) charon->bus->log(charon->bus, group, 3, format, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL >= 3 */
#if DEBUG_LEVEL >= 4
#define DBG4(group, format, ...) charon->bus->log(charon->bus, group, 4, format, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL >= 4 */

#ifndef DBG0
# define DBG0(...) {}
#endif /* DBG0 */
#ifndef DBG1
# define DBG1(...) {}
#endif /* DBG1 */
#ifndef DBG2
# define DBG2(...) {}
#endif /* DBG2 */
#ifndef DBG3
# define DBG3(...) {}
#endif /* DBG3 */
#ifndef DBG4
# define DBG4(...) {}
#endif /* DBG4 */

/**
 * Kind of alerts to raise.
 */
enum alert_t {
	/** a RADIUS server did not respond, no additional arguments */
	ALERT_RADIUS_NOT_RESPONDING,
	/** a shutdown signal has been received, argument is the signal (int) */
	ALERT_SHUTDOWN_SIGNAL,
	/** local peer authentication failed (by us or by peer), no arguments */
	ALERT_LOCAL_AUTH_FAILED,
	/** peer authentication failed, no arguments */
	ALERT_PEER_AUTH_FAILED,
	/** failed to resolve peer address, no arguments */
	ALERT_PEER_ADDR_FAILED,
	/** peer did not respond to initial message, current try (int, 0-based) */
	ALERT_PEER_INIT_UNREACHABLE,
	/** received IKE message with invalid SPI, argument is message_t* */
	ALERT_INVALID_IKE_SPI,
	/** received IKE message with invalid header, argument is message_t* */
	ALERT_PARSE_ERROR_HEADER,
	/** received IKE message with invalid body, argument is message_t*,
	 *  followed by a status_t result returned by message_t.parse_body(). */
	ALERT_PARSE_ERROR_BODY,
	/** sending a retransmit for a message, arguments are packet_t and number
	 * of the retransmit, if the message got fragmented only the first fragment
	 * is passed */
	ALERT_RETRANSMIT_SEND,
	/** received response for retransmitted request, argument is packet_t, if
	 * the message got fragmented only the first fragment is passed */
	ALERT_RETRANSMIT_SEND_CLEARED,
	/** sending retransmits timed out, argument is packet_t, if available and if
	 *  the message got fragmented only the first fragment is passed */
	ALERT_RETRANSMIT_SEND_TIMEOUT,
	/** received a retransmit for a message, argument is message_t */
	ALERT_RETRANSMIT_RECEIVE,
	/** received half-open timeout before IKE_SA established, no argument */
	ALERT_HALF_OPEN_TIMEOUT,
	/** IKE proposals do not match, argument is linked_list_t of proposal_t */
	ALERT_PROPOSAL_MISMATCH_IKE,
	/** CHILD proposals do not match, argument is linked_list_t of proposal_t */
	ALERT_PROPOSAL_MISMATCH_CHILD,
	/** traffic selectors do not match, arguments are two linked_list_t
	 *  containing traffic_selector_t for initiator and for responder */
	ALERT_TS_MISMATCH,
	/** traffic selectors have been narrowed by the peer, arguments are
	 *  an int (TRUE for local TS), a linked_list_t* (final TS list), and the
	 *  child_cfg_t*. */
	ALERT_TS_NARROWED,
	/** Installation of IPsec SAs failed, argument is child_sa_t */
	ALERT_INSTALL_CHILD_SA_FAILED,
	/** Installation of IPsec Policy failed, argument is child_sa_t */
	ALERT_INSTALL_CHILD_POLICY_FAILED,
	/** IKE_SA deleted because of "replace" unique policy, no argument */
	ALERT_UNIQUE_REPLACE,
	/** IKE_SA deleted because of "keep" unique policy, no argument */
	ALERT_UNIQUE_KEEP,
	/** IKE_SA kept on failed child SA establishment, argument is an int (!=0 if
	 * first child SA) */
	ALERT_KEEP_ON_CHILD_SA_FAILURE,
	/** allocating virtual IP failed, linked_list_t of host_t requested */
	ALERT_VIP_FAILURE,
	/** an authorize() hook failed, no argument */
	ALERT_AUTHORIZATION_FAILED,
	/** IKE_SA hit the hard lifetime limit before it could be rekeyed */
	ALERT_IKE_SA_EXPIRED,
	/** Certificate rejected; it has expired, certificate_t */
	ALERT_CERT_EXPIRED,
	/** Certificate rejected; it has been revoked, certificate_t */
	ALERT_CERT_REVOKED,
	/** Validating certificate status failed, certificate_t */
	ALERT_CERT_VALIDATION_FAILED,
	/** Certificate rejected; no trusted issuer found, certificate_t */
	ALERT_CERT_NO_ISSUER,
	/** Certificate rejected; root not trusted, certificate_t */
	ALERT_CERT_UNTRUSTED_ROOT,
	/** Certificate rejected; trustchain length exceeds limit, certificate_t */
	ALERT_CERT_EXCEEDED_PATH_LEN,
	/** Certificate rejected; other policy violation, certificate_t */
	ALERT_CERT_POLICY_VIOLATION,
};

/**
 * Kind of narrow hook.
 *
 * There is a non-authenticated (IKE_AUTH) and a authenticated
 * (CREATE_CHILD_SA) narrowing hook for the initiator. Only one of these
 * hooks is invoked before the exchange.
 * To verify the traffic selectors negotiated, each PRE hook has a POST
 * counterpart that follows. POST hooks are invoked with an authenticated peer.
 * It is usually not a good idea to narrow in the POST hooks,
 * as the resulting traffic selector is not negotiated and results
 * in non-matching policies.
 */
enum narrow_hook_t {
	/** invoked as initiator before exchange, peer is not yet authenticated */
	NARROW_INITIATOR_PRE_NOAUTH,
	/** invoked as initiator before exchange, peer is authenticated */
	NARROW_INITIATOR_PRE_AUTH,
	/** invoked as responder during exchange, peer is authenticated */
	NARROW_RESPONDER,
	/** invoked as responder after exchange, peer is authenticated */
	NARROW_RESPONDER_POST,
	/** invoked as initiator after exchange, follows a INITIATOR_PRE_NOAUTH */
	NARROW_INITIATOR_POST_NOAUTH,
	/** invoked as initiator after exchange, follows a INITIATOR_PRE_AUTH */
	NARROW_INITIATOR_POST_AUTH,
};

/**
 * The bus receives events and sends them to all registered listeners.
 *
 * Loggers are handled separately.
 */
struct bus_t {

	/**
	 * Register a listener to the bus.
	 *
	 * A registered listener receives all events which are sent to the bus.
	 * The listener is passive; the thread which emitted the event
	 * processes the listener routine.
	 *
	 * @param listener	listener to register.
	 */
	void (*add_listener) (bus_t *this, listener_t *listener);

	/**
	 * Unregister a listener from the bus.
	 *
	 * @param listener	listener to unregister.
	 */
	void (*remove_listener) (bus_t *this, listener_t *listener);

	/**
	 * Register a logger with the bus.
	 *
	 * The logger is passive; the thread which emitted the event
	 * processes the logger routine.  This routine may be called concurrently
	 * by multiple threads.  Recursive calls are not prevented, so logger that
	 * may cause recursive calls are responsible to avoid infinite loops.
	 *
	 * During registration get_level() is called for all log groups and the
	 * logger is registered to receive log messages for groups for which
	 * the requested log level is > LEVEL_SILENT and whose level is lower
	 * or equal than the requested level.
	 *
	 * To update the registered log levels call add_logger again with the
	 * same logger and return the new levels from get_level().
	 *
	 * @param logger	logger to register.
	 */
	void (*add_logger) (bus_t *this, logger_t *logger);

	/**
	 * Unregister a logger from the bus.
	 *
	 * @param logger	logger to unregister.
	 */
	void (*remove_logger) (bus_t *this, logger_t *logger);

	/**
	 * Set the IKE_SA the calling thread is using.
	 *
	 * To associate a received log message with an IKE_SA without passing it as
	 * parameter each time, the thread registers the currently used IKE_SA
	 * during check-out. Before check-in, the thread unregisters the IKE_SA.
	 * This IKE_SA is stored per-thread, so each thread has its own IKE_SA
	 * registered.
	 *
	 * @param ike_sa	ike_sa to register, or NULL to unregister
	 */
	void (*set_sa) (bus_t *this, ike_sa_t *ike_sa);

	/**
	 * Get the IKE_SA the calling thread is currently using.
	 *
	 * If a thread currently does not know what IKE_SA it is processing,
	 * it can call get_sa() to look up the SA set during checkout via set_sa().
	 *
	 * @return			registered ike_sa, NULL if none registered
	 */
	ike_sa_t* (*get_sa)(bus_t *this);

	/**
	 * Send a log message to the bus.
	 *
	 * The format string specifies an additional informational or error
	 * message with a printf() like variable argument list.
	 * Use the DBG() macros.
	 *
	 * @param group		debugging group
	 * @param level		verbosity level of the signal
	 * @param format	printf() style format string
	 * @param ...		printf() style argument list
	 */
	void (*log)(bus_t *this, debug_t group, level_t level, char* format, ...);

	/**
	 * Send a log message to the bus using va_list arguments.
	 *
	 * Same as bus_t.log(), but uses va_list argument list.
	 *
	 * @param group		kind of the signal (up, down, rekeyed, ...)
	 * @param level		verbosity level of the signal
	 * @param format	printf() style format string
	 * @param args		va_list arguments
	 */
	void (*vlog)(bus_t *this, debug_t group, level_t level,
				 char* format, va_list args);

	/**
	 * Raise an alert over the bus.
	 *
	 * @param alert		kind of alert
	 * @param ...		alert specific arguments
	 */
	void (*alert)(bus_t *this, alert_t alert, ...);

	/**
	 * Send a IKE_SA state change event to the bus.
	 *
	 * @param ike_sa	IKE_SA which changes its state
	 * @param state		new state IKE_SA changes to
	 */
	void (*ike_state_change)(bus_t *this, ike_sa_t *ike_sa,
							 ike_sa_state_t state);
	/**
	 * Send a CHILD_SA state change event to the bus.
	 *
	 * @param child_sa	CHILD_SA which changes its state
	 * @param state		new state CHILD_SA changes to
	 */
	void (*child_state_change)(bus_t *this, child_sa_t *child_sa,
							   child_sa_state_t state);
	/**
	 * Message send/receive hook.
	 *
	 * The hook is invoked twice for each message: Once with plain, parsed data
	 * and once encoded and encrypted.
	 *
	 * @param message	message to send/receive
	 * @param incoming	TRUE for incoming messages, FALSE for outgoing
	 * @param plain		TRUE if message is parsed and decrypted, FALSE it not
	 */
	void (*message)(bus_t *this, message_t *message, bool incoming, bool plain);

	/**
	 * IKE_SA authorization hook.
	 *
	 * @param final		TRUE if this is the final invocation
	 * @return			TRUE to establish IKE_SA, FALSE to send AUTH_FAILED
	 */
	bool (*authorize)(bus_t *this, bool final);

	/**
	 * CHILD_SA traffic selector narrowing hook.
	 *
	 * @param child_sa	CHILD_SA set up with these traffic selectors
	 * @param type		type of hook getting invoked
	 * @param local		list of local traffic selectors to narrow
	 * @param remote	list of remote traffic selectors to narrow
	 */
	void (*narrow)(bus_t *this, child_sa_t *child_sa, narrow_hook_t type,
				   linked_list_t *local, linked_list_t *remote);

	/**
	 * IKE_SA keymat hook.
	 *
	 * @param ike_sa	IKE_SA this keymat belongs to
	 * @param dh		diffie hellman shared secret
	 * @param dh_other	others DH public value (IKEv1 only)
	 * @param nonce_i	initiator's nonce
	 * @param nonce_r	responder's nonce
	 * @param rekey		IKE_SA we are rekeying, if any (IKEv2 only)
	 * @param shared	shared key used for key derivation (IKEv1-PSK only)
	 * @param method	auth method for key derivation (IKEv1-non-PSK only)
	 */
	void (*ike_keys)(bus_t *this, ike_sa_t *ike_sa, diffie_hellman_t *dh,
					 chunk_t dh_other, chunk_t nonce_i, chunk_t nonce_r,
					 ike_sa_t *rekey, shared_key_t *shared,
					 auth_method_t method);

	/**
	 * IKE_SA derived keys hook.
	 *
	 * @param sk_ei		SK_ei, or Ka for IKEv1
	 * @param sk_er		SK_er
	 * @param sk_ai		SK_ai, or SKEYID_a for IKEv1
	 * @param sk_ar		SK_ar
	 */
	void (*ike_derived_keys)(bus_t *this, chunk_t sk_ei, chunk_t sk_er,
							 chunk_t sk_ai, chunk_t sk_ar);

	/**
	 * CHILD_SA keymat hook.
	 *
	 * @param child_sa	CHILD_SA this keymat is used for
	 * @param initiator	initiator of the CREATE_CHILD_SA exchange
	 * @param dh		diffie hellman shared secret
	 * @param nonce_i	initiator's nonce
	 * @param nonce_r	responder's nonce
	 */
	void (*child_keys)(bus_t *this, child_sa_t *child_sa, bool initiator,
					   diffie_hellman_t *dh, chunk_t nonce_i, chunk_t nonce_r);

	/**
	 * CHILD_SA derived keys hook.
	 *
	 * @param child_sa	CHILD_SA these keys are used for
	 * @param initiator	initiator of the CREATE_CHILD_SA exchange
	 * @param encr_i	initiator's encryption key
	 * @param encr_o	responder's encryption key
	 * @param integ_i	initiator's integrity key
	 * @param integ_r	responder's integrity key
	 */
	void (*child_derived_keys)(bus_t *this, child_sa_t *child_sa,
							   bool initiator, chunk_t encr_i, chunk_t encr_r,
							   chunk_t integ_i, chunk_t integ_r);

	/**
	 * IKE_SA up/down hook.
	 *
	 * @param ike_sa	IKE_SA coming up/going down
	 * @param up		TRUE for an up event, FALSE for a down event
	 */
	void (*ike_updown)(bus_t *this, ike_sa_t *ike_sa, bool up);

	/**
	 * IKE_SA rekeying hook.
	 *
	 * @param old		rekeyed and obsolete IKE_SA
	 * @param new		new IKE_SA replacing old
	 */
	void (*ike_rekey)(bus_t *this, ike_sa_t *old, ike_sa_t *new);

	/**
	 * IKE_SA peer endpoint update hook.
	 *
	 * @param ike_sa	updated IKE_SA, having old endpoints set
	 * @param local		TRUE if local endpoint gets updated, FALSE for remote
	 * @param new		new endpoint address and port
	 */
	void (*ike_update)(bus_t *this, ike_sa_t *ike_sa, bool local, host_t *new);

	/**
	 * IKE_SA reestablishing hook (before resolving hosts).
	 *
	 * @param old		reestablished and obsolete IKE_SA
	 * @param new		new IKE_SA replacing old
	 */
	void (*ike_reestablish_pre)(bus_t *this, ike_sa_t *old, ike_sa_t *new);

	/**
	 * IKE_SA reestablishing hook (after configuring and initiating the new
	 * IKE_SA).
	 *
	 * @param old		reestablished and obsolete IKE_SA
	 * @param new		new IKE_SA replacing old
	 * @param initiated	TRUE if initiated successfully, FALSE otherwise
	 */
	void (*ike_reestablish_post)(bus_t *this, ike_sa_t *old, ike_sa_t *new,
								 bool initiated);

	/**
	 * CHILD_SA up/down hook.
	 *
	 * @param child_sa	CHILD_SA coming up/going down
	 * @param up		TRUE for an up event, FALSE for a down event
	 */
	void (*child_updown)(bus_t *this, child_sa_t *child_sa, bool up);

	/**
	 * CHILD_SA rekeying hook.
	 *
	 * @param old		rekeyed and obsolete CHILD_SA
	 * @param new		new CHILD_SA replacing old
	 */
	void (*child_rekey)(bus_t *this, child_sa_t *old, child_sa_t *new);

	/**
	 * CHILD_SA migration hook.
	 *
	 * @param new		ID of new SA when called for the old, NULL otherwise
	 * @param uniue		unique ID of new SA when called for the old, 0 otherwise
	 */
	void (*children_migrate)(bus_t *this, ike_sa_id_t *new, uint32_t unique);

	/**
	 * Virtual IP assignment hook.
	 *
	 * @param ike_sa	IKE_SA the VIPs are assigned to
	 * @param assign	TRUE if assigned to IKE_SA, FALSE if released
	 */
	void (*assign_vips)(bus_t *this, ike_sa_t *ike_sa, bool assign);

	/**
	 * Virtual IP handler hook.
	 *
	 * @param ike_sa	IKE_SA the VIPs/attributes got handled on
	 * @param assign	TRUE after installing attributes, FALSE on release
	 */
	void (*handle_vips)(bus_t *this, ike_sa_t *ike_sa, bool handle);

	/**
	 * Destroy the event bus.
	 */
	void (*destroy) (bus_t *this);
};

/**
 * Create the event bus which forwards events to its listeners.
 *
 * @return		event bus instance
 */
bus_t *bus_create();

#endif /** BUS_H_ @}*/
