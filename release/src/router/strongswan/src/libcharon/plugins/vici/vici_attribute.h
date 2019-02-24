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
 * @defgroup vici_attribute vici_attribute
 * @{ @ingroup vici
 */

#ifndef VICI_ATTRIBUTE_H_
#define VICI_ATTRIBUTE_H_

#include "vici_dispatcher.h"

#include <attributes/attribute_provider.h>

typedef struct vici_attribute_t vici_attribute_t;

/**
 * IKE configuration attribute backend for vici.
 */
struct vici_attribute_t {

	/**
	 * Implements attribute provider interface
	 */
	attribute_provider_t provider;

	/**
	 * Destroy a vici_attribute_t.
	 */
	void (*destroy)(vici_attribute_t *this);
};

/**
 * Create a vici_attribute instance.
 *
 * @param dispatcher		vici dispatcher context
 * @return					vici attribute handler
 */
vici_attribute_t *vici_attribute_create(vici_dispatcher_t *dispatcher);

#endif /** VICI_ATTRIBUTE_H_ @}*/
