/*
 * Copyright (C) 2008-2009 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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

#ifndef DUMM_H
#define DUMM_H

#include <signal.h>

#include <library.h>
#include <collections/enumerator.h>

#include "guest.h"
#include "bridge.h"

typedef struct dumm_t dumm_t;

/**
 * dumm - Dynamic Uml Mesh Modeler
 *
 * Controls a group of UML guests and their networks.
 */
struct dumm_t {

	/**
	 * Starts a new UML guest
	 *
	 * @param name		name of the guest
	 * @param kernel	UML kernel to use for guest
	 * @param master	mounted read only master filesystem
	 * @param args		additional args to pass to kernel
	 * @return			guest if started, NULL if failed
	 */
	guest_t* (*create_guest) (dumm_t *this, char *name, char *kernel,
							  char *master, char *args);

	/**
	 * Create an enumerator over all guests.
	 *
	 * @return			enumerator over guest_t's
	 */
	enumerator_t* (*create_guest_enumerator) (dumm_t *this);

	/**
	 * Delete a guest from disk.
	 *
	 * @param guest		guest to destroy
	 */
	void (*delete_guest) (dumm_t *this, guest_t *guest);

	/**
	 * Create a new bridge.
	 *
	 * @param name		name of the bridge to create
	 * @return			created bridge
	 */
	bridge_t* (*create_bridge)(dumm_t *this, char *name);

	/**
	 * Create an enumerator over all bridges.
	 *
	 * @return			enumerator over bridge_t's
	 */
	enumerator_t* (*create_bridge_enumerator)(dumm_t *this);

	/**
	 * Delete a bridge.
	 *
	 * @param bridge	bridge to destroy
	 */
	void (*delete_bridge) (dumm_t *this, bridge_t *bridge);

	/**
	 * Add an overlay to all guests.
	 *
	 * Directories named after the guests are created, if they do not exist
	 * in the given overlay directory.
	 *
	 * If adding the overlay on at lest one guest fails, FALSE is returned and
	 * the overlay is again removed from all guests.
	 *
	 * @param dir		dir to the overlay
	 * @return			FALSE, on failure
	 */
	bool (*add_overlay)(dumm_t *this, char *dir);

	/**
	 * Removes an overlay from all guests.
	 *
	 * @param dir		dir to the overlay
	 * @return			FALSE, if the overlay was not found on any guest
	 */
	bool (*del_overlay)(dumm_t *this, char *dir);

	/**
	 * Remove the latest overlay from all guests.
	 *
	 * @return			FALSE, if no overlay was found on any guest
	 */
	bool (*pop_overlay)(dumm_t *this);

	/**
	 * Loads a template, create a new one if it does not exist.
	 *
	 * This is basically a wrapper around add/del_overlay to simplify working
	 * with overlays. Templates are located in a predefined directory, so that
	 * only a name for the template has to be specified here. Only one template
	 * can be loaded at any one time (but other overlays can be added on top or
	 * below a template).
	 *
	 * @param name		name of the template to load, NULL to unload
	 * @return			FALSE if load/create failed
	 */
	bool (*load_template)(dumm_t *this, char *name);

	/**
	 * Create an enumerator over all available templates.
	 *
	 * @return			enumerator over char*
	 */
	enumerator_t* (*create_template_enumerator)(dumm_t *this);

	/**
	 * stop all guests and destroy the modeler
	 */
	void (*destroy) (dumm_t *this);
};

/**
 * Create a group of UML hosts and networks.
 *
 * @param dir			directory to create guests/load from, NULL for cwd
 * @return				created UML group, or NULL if failed.
 */
dumm_t *dumm_create(char *dir);

#endif /* DUMM_H */

