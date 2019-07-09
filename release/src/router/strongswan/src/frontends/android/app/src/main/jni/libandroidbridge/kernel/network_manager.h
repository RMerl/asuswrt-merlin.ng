/*
 * Copyright (C) 2012-2015 Tobias Brunner
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
 * @defgroup network_manager network_manager
 * @{ @ingroup android_kernel
 */

#ifndef NETWORK_MANAGER_H_
#define NETWORK_MANAGER_H_

#include <jni.h>

#include <library.h>
#include <networking/host.h>

typedef struct network_manager_t network_manager_t;

/**
 * Callback called if connectivity changes somehow.
 *
 * Implementation should be quick as the call is made by the Java apps main
 * thread.
 *
 * @param data					data supplied during registration
 * @param disconnected			TRUE if currently disconnected
 */
typedef void (*connectivity_cb_t)(void *data, bool disconnected);

/**
 * NetworkManager, used to listen for network changes.
 *
 * Communicates with NetworkManager via JNI
 */
struct network_manager_t {

	/**
	 * Register a callback that is called if connectivity changes
	 *
	 * @note Only the first registered callback is currently used
	 *
	 * @param cb				callback to register
	 * @param data				data provided to callback
	 */
	void (*add_connectivity_cb)(network_manager_t *this, connectivity_cb_t cb,
								void *data);

	/**
	 * Unregister a previously registered callback for connectivity changes
	 *
	 * @param cb				previously registered callback
	 */
	void (*remove_connectivity_cb)(network_manager_t *this,
								   connectivity_cb_t cb);

	/**
	 * Check whether we currently have connectivity
	 *
	 * @return					TRUE if currently connected
	 */
	bool (*is_connected)(network_manager_t *this);

	/**
	 * Destroy a network_manager_t instance
	 */
	void (*destroy)(network_manager_t *this);
};

/**
 * Create a network_manager_t instance
 *
 * @param context				Context object
 * @return						network_manager_t instance
 */
network_manager_t *network_manager_create(jobject context);

#endif /** NETWORK_MANAGER_H_ @}*/
