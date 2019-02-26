/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup config_t config
 * @{ @ingroup conftest
 */

#ifndef CONFIG_H_
#define CONFIG_H_

typedef struct config_t config_t;

#include <config/backend.h>

/**
 * Conftest IKE and CHILD config backend
 */
struct config_t {

	/**
	 * Implements the backend_t interface.
	 */
	backend_t backend;

	/**
	 * Load configurations from a settings file.
	 *
	 * @param settings		settings file to load configs from
	 */
	void (*load)(config_t *this, settings_t *settings);

	/**
	 * Destroy a config_t.
	 */
	void (*destroy)(config_t *this);
};

/**
 * Create a config instance.
 */
config_t *config_create();

#endif /** CONFIG_H_ @}*/
