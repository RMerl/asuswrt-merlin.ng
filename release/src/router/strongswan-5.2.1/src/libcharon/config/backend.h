/*
 * Copyright (C) 2007-2008 Martin Willi
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
 * @defgroup backend backend
 * @{ @ingroup config
 */

#ifndef BACKEND_H_
#define BACKEND_H_

typedef struct backend_t backend_t;

#include <library.h>
#include <config/ike_cfg.h>
#include <config/peer_cfg.h>
#include <collections/linked_list.h>

/**
 * The interface for a configuration backend.
 *
 * A configuration backend is loaded into the backend_manager. It does the actual
 * configuration lookup for the method it implements. See backend_manager_t for
 * more information.
 */
struct backend_t {

	/**
	 * Create an enumerator over all IKE configs matching two hosts.
	 *
	 * Hosts may be NULL to get all.
	 *
	 * There is no requirement for the backend to filter the configurations
	 * using the supplied hosts; but it may do so if it increases lookup times
	 * (e.g. include hosts in SQL query).
	 *
	 * @param me		address of local host
	 * @param other		address of remote host
	 * @return			enumerator over ike_cfg_t's
	 */
	enumerator_t* (*create_ike_cfg_enumerator)(backend_t *this,
											   host_t *me, host_t *other);
	/**
	 * Create an enumerator over all peer configs matching two identities.
	 *
	 * IDs may be NULL to get all.
	 *
	 * As configurations are looked up in the first authentication round (when
	 * multiple authentication), the backend implementation should compare
	 * the identities to the first auth_cfgs only.
	 * There is no requirement for the backend to filter the configurations
	 * using the supplied identities; but it may do so if it increases lookup
	 * times (e.g. include hosts in SQL query).
	 *
	 * @param me		identity of ourself
	 * @param other		identity of remote host
	 * @return			enumerator over peer_cfg_t
	 */
	enumerator_t* (*create_peer_cfg_enumerator)(backend_t *this,
												identification_t *me,
												identification_t *other);
	/**
	 * Get a peer_cfg identified by it's name, or a name of its children.
	 *
	 * @param name				name of peer/child cfg
	 * @return					matching peer_config, or NULL if none found
	 */
	peer_cfg_t *(*get_peer_cfg_by_name)(backend_t *this, char *name);
};

#endif /** BACKEND_H_ @}*/
