/*
 * Copyright (C) 2010-2011 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
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
 * @defgroup android_dns_handler android_dns_handler
 * @{ @ingroup android_dns
 */

#ifndef ANDROID_DNS_HANDLER_H_
#define ANDROID_DNS_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct android_dns_handler_t android_dns_handler_t;

/**
 * Android specific DNS attribute handler.
 */
struct android_dns_handler_t {

	/**
	 * Implements attribute_handler_t.
	 */
	attribute_handler_t handler;

	/**
	 * Destroy a android_dns_handler_t.
	 */
	void (*destroy)(android_dns_handler_t *this);
};

/**
 * Create an android_dns_handler_t instance.
 */
android_dns_handler_t *android_dns_handler_create();

#endif /** ANDROID_DNS_HANDLER_H_ @}*/
