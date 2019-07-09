/*
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
 * @defgroup android_attr android_attr
 * @{ @ingroup android_backend
 */

#ifndef ANDROID_ATTR_H_
#define ANDROID_ATTR_H_

#include <library.h>
#include <attributes/attribute_handler.h>

typedef struct android_attr_t android_attr_t;

/**
 * Handler for DNS configuration
 */
struct android_attr_t {

	/**
	 * implements the attribute_handler_t interface
	 */
	attribute_handler_t handler;

	/**
	 * Destroy a android_attr_t
	 */
	void (*destroy)(android_attr_t *this);
};

/**
 * Create a android_attr_t instance.
 */
android_attr_t *android_attr_create(void);

#endif /** ANDROID_ATTR_H_ @}*/

