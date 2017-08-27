/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup vici_config vici_config
 * @{ @ingroup vici
 */

#ifndef VICI_CONFIG_H_
#define VICI_CONFIG_H_

#include "vici_dispatcher.h"

#include <config/backend.h>

typedef struct vici_config_t vici_config_t;

/**
 * In-memory configuration backend, managed by VICI.
 */
struct vici_config_t {

	/**
	 * Implements a configuraiton backend.
	 */
	backend_t backend;

	/**
	 * Destroy a vici_config_t.
	 */
	void (*destroy)(vici_config_t *this);
};
/**
 * Create a vici_config instance.
 *
 * @param dispatcher		dispatcher to receive requests from
 * @return					config backend
 */
vici_config_t *vici_config_create(vici_dispatcher_t *dispatcher);

#endif /** VICI_CONFIG_H_ @}*/
