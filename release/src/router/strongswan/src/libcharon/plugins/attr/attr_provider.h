/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup attr_provider attr_provider
 * @{ @ingroup attr
 */

#ifndef ATTR_PROVIDER_H_
#define ATTR_PROVIDER_H_

#include <attributes/attribute_provider.h>

typedef struct attr_provider_t attr_provider_t;

/**
 * Provide configuration attributes through static strongswan.conf definition.
 */
struct attr_provider_t {

	/**
	 * Implements attribute provider interface
	 */
	attribute_provider_t provider;

	/**
	 * Reload configuration from strongswan.conf.
	 */
	void (*reload)(attr_provider_t *this);

	/**
	 * Destroy a attr_provider instance.
	 */
	void (*destroy)(attr_provider_t *this);
};

/**
 * Create a attr_provider instance.
 */
attr_provider_t *attr_provider_create();

#endif /** ATTR_PROVIDER @}*/
