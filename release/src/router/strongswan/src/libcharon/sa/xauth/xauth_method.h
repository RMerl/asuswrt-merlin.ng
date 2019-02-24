/*
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
 * @defgroup xauth_method xauth_method
 * @{ @ingroup xauth
 */

#ifndef XAUTH_METHOD_H_
#define XAUTH_METHOD_H_

typedef struct xauth_method_t xauth_method_t;
typedef enum xauth_role_t xauth_role_t;

#include <library.h>
#include <plugins/plugin.h>
#include <utils/identification.h>
#include <encoding/payloads/cp_payload.h>

/**
 * Role of an xauth_method, SERVER or PEER (client)
 */
enum xauth_role_t {
	XAUTH_SERVER,
	XAUTH_PEER,
};

/**
 * enum names for xauth_role_t.
 */
extern enum_name_t *xauth_role_names;

/**
 * Interface of an XAuth method for server and client side.
 *
 * An XAuth method initiates an XAuth exchange and processes requests and
 * responses. An XAuth method may need multiple exchanges before succeeding.
 * Sending of XAUTH(STATUS) message is done by the framework, not a method.
 */
struct xauth_method_t {

	/**
	 * Initiate the XAuth exchange.
	 *
	 * initiate() is only usable for server implementations, as clients only
	 * reply to server requests.
	 * A cp_payload is created in "out" if result is NEED_MORE.
	 *
	 * @param out		cp_payload to send to the client
	 * @return
	 * 					- NEED_MORE, if an other exchange is required
	 * 					- FAILED, if unable to create XAuth request payload
	 */
	status_t (*initiate) (xauth_method_t *this, cp_payload_t **out);

	/**
	 * Process a received XAuth message.
	 *
	 * A cp_payload is created in "out" if result is NEED_MORE.
	 *
	 * @param in		cp_payload response received
	 * @param out		created cp_payload to send
	 * @return
	 * 					- NEED_MORE, if an other exchange is required
	 * 					- FAILED, if XAuth method failed
	 * 					- SUCCESS, if XAuth method succeeded
	 */
	status_t (*process) (xauth_method_t *this, cp_payload_t *in,
						 cp_payload_t **out);

	/**
	 * Get the XAuth username received as XAuth initiator.
	 *
	 * @return			used XAuth username, pointer to internal data
	 */
	identification_t* (*get_identity)(xauth_method_t *this);

	/**
	 * Destroys a eap_method_t object.
	 */
	void (*destroy) (xauth_method_t *this);
};

/**
 * Constructor definition for a pluggable XAuth method.
 *
 * Each XAuth module must define a constructor function which will return
 * an initialized object with the methods defined in xauth_method_t.
 * Constructors for server and peers are identical, to support both roles
 * of a XAuth method, a plugin needs register two constructors in the
 * xauth_manager_t.
 *
 * @param server		ID of the server to use for credential lookup
 * @param peer			ID of the peer to use for credential lookup
 * @param profile		configuration string to pass to XAuth method, or NULL
 * @return				implementation of the eap_method_t interface
 */
typedef xauth_method_t *(*xauth_constructor_t)(identification_t *server,
											   identification_t *peer,
											   char *profile);

/**
 * Helper function to (un-)register XAuth methods from plugin features.
 *
 * This function is a plugin_feature_callback_t and can be used with the
 * PLUGIN_CALLBACK macro to register a XAuth method constructor.
 *
 * @param plugin		plugin registering the XAuth method constructor
 * @param feature		associated plugin feature
 * @param reg			TRUE to register, FALSE to unregister.
 * @param data			data passed to callback, an xauth_constructor_t
 */
bool xauth_method_register(plugin_t *plugin, plugin_feature_t *feature,
						   bool reg, void *data);

#endif /** XAUTH_METHOD_H_ @}*/
