/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup stroke_config stroke_config
 * @{ @ingroup stroke
 */

#ifndef STROKE_CONFIG_H_
#define STROKE_CONFIG_H_

#include <config/backend.h>
#include <stroke_msg.h>
#include "stroke_ca.h"
#include "stroke_cred.h"
#include "stroke_attribute.h"

typedef struct stroke_config_t stroke_config_t;

/**
 * Stroke in-memory configuration backend
 */
struct stroke_config_t {

	/**
	 * Implements the backend_t interface
	 */
	backend_t backend;

	/**
	 * Add a configuration to the backend.
	 *
	 * @param msg		received stroke message containing config
	 */
	void (*add)(stroke_config_t *this, stroke_msg_t *msg);

	/**
	 * Remove a configuration from the backend.
	 *
	 * @param msg		received stroke message containing config name
	 */
	void (*del)(stroke_config_t *this, stroke_msg_t *msg);

	/**
	 * Set the username and password for a connection in this backend.
	 *
	 * @param msg		received stroke message
	 * @param prompt	I/O channel to prompt for the password
	 */
	void (*set_user_credentials)(stroke_config_t *this, stroke_msg_t *msg,
								 FILE *prompt);

	/**
	 * Destroy a stroke_config instance.
	 */
	void (*destroy)(stroke_config_t *this);
};

/**
 * Create a stroke_config instance.
 */
stroke_config_t *stroke_config_create(stroke_ca_t *ca, stroke_cred_t *cred,
									  stroke_attribute_t *attributes);

#endif /** STROKE_CONFIG_H_ @}*/
