/*
 * Copyright (C) 2013 Tobias Brunner
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

/**
 * @defgroup xauth_noauth xauth_noauth
 * @ingroup cplugins
 *
 * @defgroup xauth_noauth_plugin xauth_noauth_plugin
 * @{ @ingroup xauth_noauth
 */

#ifndef XAUTH_NOAUTH_PLUGIN_H_
#define XAUTH_NOAUTH_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct xauth_noauth_plugin_t xauth_noauth_plugin_t;

/**
 * XAuth plugin that does not actually do any authentication but simply
 * concludes the XAuth exchange successfully.  This could be used to implement
 * basic RSA authentication in cases where the client does not offer an option
 * to disable XAuth.
 */
struct xauth_noauth_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** XAUTH_NOAUTH_PLUGIN_H_ @}*/
