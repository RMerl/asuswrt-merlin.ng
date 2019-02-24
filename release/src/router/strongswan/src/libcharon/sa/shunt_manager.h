/*
 * Copyright (C) 2015-2017 Tobias Brunner
 * Copyright (C) 2011 Andreas Steffen
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
 * @defgroup shunt_manager shunt_manager
 * @{ @ingroup sa
 */

#ifndef SHUNT_MANAGER_H_
#define SHUNT_MANAGER_H_

#include <library.h>
#include <collections/enumerator.h>
#include <config/child_cfg.h>

typedef struct shunt_manager_t shunt_manager_t;

/**
 * Manage PASS and DROP shunt policy excepting traffic from IPsec SAs.
 */
struct shunt_manager_t {

	/**
	 * Install a policy as a shunt.
	 *
	 * @param ns		namespace (e.g. name of a connection or plugin), cloned
	 * @param child		child configuration to install as a shunt
	 * @return			TRUE if installed successfully
	 */
	bool (*install)(shunt_manager_t *this, char *ns, child_cfg_t *child);

	/**
	 * Uninstall a shunt policy.
	 *
	 * If no namespace is given the first matching child configuration is
	 * removed.
	 *
	 * @param ns		namespace (same as given during installation) or NULL
	 * @param name	 	name of child configuration to uninstall as a shunt
	 * @return			TRUE if uninstalled successfully
	 */
	bool (*uninstall)(shunt_manager_t *this, char *ns, char *name);

	/**
	 * Create an enumerator over all installed shunts.
	 *
	 * @return			enumerator over (char*, child_cfg_t*)
	 */
	enumerator_t* (*create_enumerator)(shunt_manager_t *this);

	/**
	 * Clear any installed shunt.
	 */
	void (*flush)(shunt_manager_t *this);

	/**
	 * Destroy a shunt_manager_t.
	 */
	void (*destroy)(shunt_manager_t *this);
};

/**
 * Create a shunt_manager instance.
 */
shunt_manager_t *shunt_manager_create();

#endif /** SHUNT_MANAGER_H_ @}*/
