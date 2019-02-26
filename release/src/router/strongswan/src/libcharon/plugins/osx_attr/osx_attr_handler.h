/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup osx_attr_handler osx_attr_handler
 * @{ @ingroup osx_attr
 */

#ifndef OSX_ATTR_HANDLER_H_
#define OSX_ATTR_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct osx_attr_handler_t osx_attr_handler_t;

/**
 * OS X specific attribute handler, using SystemConfiguration framework.
 */
struct osx_attr_handler_t {

	/**
	 * Implements attribute_handler_t.
	 */
	attribute_handler_t handler;

	/**
	 * Destroy a osx_attr_handler_t.
	 */
	void (*destroy)(osx_attr_handler_t *this);
};

/**
 * Create an osx_attr_handler_t instance.
 */
osx_attr_handler_t *osx_attr_handler_create();

#endif /** OSX_ATTR_HANDLER_H_ @}*/
