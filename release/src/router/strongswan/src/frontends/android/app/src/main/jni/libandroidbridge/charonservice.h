/*
 * Copyright (C) 2012-2013 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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
 * @defgroup libandroidbridge libandroidbridge
 *
 * @defgroup android_backend backend
 * @ingroup libandroidbridge
 *
 * @defgroup android_byod byod
 * @ingroup libandroidbridge
 *
 * @defgroup android_kernel kernel
 * @ingroup libandroidbridge
 *
 * @defgroup charonservice charonservice
 * @{ @ingroup libandroidbridge
 */

#ifndef CHARONSERVICE_H_
#define CHARONSERVICE_H_

#include "vpnservice_builder.h"
#include "kernel/network_manager.h"

#include <library.h>
#include <collections/linked_list.h>

typedef enum android_vpn_state_t android_vpn_state_t;
typedef enum android_imc_state_t android_imc_state_t;
typedef struct charonservice_t charonservice_t;

/**
 * Default value for the MTU of TUN device and the size of IKE fragments
 */
#define ANDROID_DEFAULT_MTU 1400

/**
 * VPN status codes. As defined in CharonVpnService.java
 */
enum android_vpn_state_t {
	CHARONSERVICE_CHILD_STATE_UP = 1,
	CHARONSERVICE_CHILD_STATE_DOWN,
	CHARONSERVICE_AUTH_ERROR,
	CHARONSERVICE_PEER_AUTH_ERROR,
	CHARONSERVICE_LOOKUP_ERROR,
	CHARONSERVICE_UNREACHABLE_ERROR,
	CHARONSERVICE_CERTIFICATE_UNAVAILABLE,
	CHARONSERVICE_GENERIC_ERROR,
};

/**
 * Final IMC state as defined in ImcState.java
 */
enum android_imc_state_t {
	ANDROID_IMC_STATE_UNKNOWN = 0,
	ANDROID_IMC_STATE_ALLOW = 1,
	ANDROID_IMC_STATE_BLOCK = 2,
	ANDROID_IMC_STATE_ISOLATE = 3,
};

/**
 * Public interface of charonservice.
 *
 * Used to communicate with CharonVpnService via JNI
 */
struct charonservice_t {

	/**
	 * Update the status in the Java domain (UI)
	 *
	 * @param code			status code
	 * @return				TRUE on success
	 */
	bool (*update_status)(charonservice_t *this, android_vpn_state_t code);

	/**
	 * Update final IMC state in the Java domain (UI)
	 *
	 * @param state			IMC state
	 * @return				TRUE on success
	 */
	bool (*update_imc_state)(charonservice_t *this, android_imc_state_t state);

	/**
	 * Add a remediation instruction via JNI
	 *
	 * @param instr			remediation instruction
	 * @return				TRUE on success
	 */
	bool (*add_remediation_instr)(charonservice_t *this, char *instr);

	/**
	 * Install a bypass policy for the given socket using the protect() Method
	 * of the Android VpnService interface.
	 *
	 * Use -1 as fd to re-bypass previously bypassed sockets.
	 *
	 * @param fd			socket file descriptor
	 * @param family		socket protocol family
	 * @return				TRUE if operation successful
	 */
	bool (*bypass_socket)(charonservice_t *this, int fd, int family);

	/**
	 * Get a list of trusted certificates via JNI
	 *
	 * @return				list of DER encoded certificates (as chunk_t*),
	 *						NULL on failure
	 */
	linked_list_t *(*get_trusted_certificates)(charonservice_t *this);

	/**
	 * Get the configured user certificate chain via JNI
	 *
	 * The first item in the returned list is the  user certificate followed
	 * by any remaining elements of the certificate chain.
	 *
	 * @return				list of DER encoded certificates (as chunk_t*),
	 *						NULL on failure
	 */
	linked_list_t *(*get_user_certificate)(charonservice_t *this);

	/**
	 * Get the configured private key via JNI
	 *
	 * @param pubkey		the public key as extracted from the certificate
	 * @return				PrivateKey object, NULL on failure
	 */
	private_key_t *(*get_user_key)(charonservice_t *this, public_key_t *pubkey);

	/**
	 * Get the current vpnservice_builder_t object
	 *
	 * @return				VpnService.Builder instance
	 */
	vpnservice_builder_t *(*get_vpnservice_builder)(charonservice_t *this);

	/**
	 * Get the current network_manager_t object
	 *
	 * @return				NetworkManager instance
	 */
	network_manager_t *(*get_network_manager)(charonservice_t *this);
};

/**
 * The single instance of charonservice_t.
 *
 * Set between JNI calls to initializeCharon() and deinitializeCharon().
 */
extern charonservice_t *charonservice;

#endif /** CHARONSERVICE_H_ @}*/
