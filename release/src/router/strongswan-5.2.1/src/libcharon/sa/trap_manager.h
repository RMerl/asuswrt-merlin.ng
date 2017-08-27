/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup trap_manager trap_manager
 * @{ @ingroup sa
 */

#ifndef TRAP_MANAGER_H_
#define TRAP_MANAGER_H_

#include <library.h>
#include <collections/enumerator.h>
#include <config/peer_cfg.h>

typedef struct trap_manager_t trap_manager_t;

/**
 * Manage policies to create SAs from traffic.
 */
struct trap_manager_t {

	/**
	 * Install a policy as a trap.
	 *
	 * @param peer		peer configuration to initiate on trap
	 * @param child 	child configuration to install as a trap
	 * @param reqid		optional reqid to use
	 * @return			reqid of installed CHILD_SA, 0 if failed
	 */
	u_int32_t (*install)(trap_manager_t *this, peer_cfg_t *peer,
						 child_cfg_t *child, u_int32_t reqid);

	/**
	 * Uninstall a trap policy.
	 *
	 * @param id		reqid of CHILD_SA to uninstall, returned by install()
	 * @return			TRUE if uninstalled successfully
	 */
	bool (*uninstall)(trap_manager_t *this, u_int32_t reqid);

	/**
	 * Create an enumerator over all installed traps.
	 *
	 * @return			enumerator over (peer_cfg_t, child_sa_t)
	 */
	enumerator_t* (*create_enumerator)(trap_manager_t *this);

	/**
	 * Find the reqid of a child config installed as a trap.
	 *
	 * @param child		CHILD_SA config to get the reqid for
	 * @return			reqid of trap, 0 if not found
	 */
	u_int32_t (*find_reqid)(trap_manager_t *this, child_cfg_t *child);

	/**
	 * Acquire an SA triggered by an installed trap.
	 *
	 * @param reqid		requid of the triggering CHILD_SA
	 * @param src		source of the triggering packet
	 * @param dst		destination of the triggering packet
	 */
	void (*acquire)(trap_manager_t *this, u_int32_t reqid,
					traffic_selector_t *src, traffic_selector_t *dst);

	/**
	 * Clear any installed trap.
	 */
	void (*flush)(trap_manager_t *this);

	/**
	 * Destroy a trap_manager_t.
	 */
	void (*destroy)(trap_manager_t *this);
};

/**
 * Create a trap_manager instance.
 */
trap_manager_t *trap_manager_create();

#endif /** TRAP_MANAGER_H_ @}*/
