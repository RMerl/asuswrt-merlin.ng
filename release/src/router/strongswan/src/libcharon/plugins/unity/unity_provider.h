/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup unity_provider unity_provider
 * @{ @ingroup unity
 */

#ifndef UNITY_PROVIDER_H_
#define UNITY_PROVIDER_H_

typedef struct unity_provider_t unity_provider_t;

#include <attributes/attribute_provider.h>

/**
 * Cisco Unity extension attribute provider.
 */
struct unity_provider_t {

	/**
	 * Implements attribute_provier_t interface.
	 */
	attribute_provider_t provider;

	/**
	 * Destroy a unity_provider_t.
	 */
	void (*destroy)(unity_provider_t *this);
};

/**
 * Create a unity_provider instance.
 */
unity_provider_t *unity_provider_create();

#endif /** UNITY_PROVIDER_H_ @}*/
