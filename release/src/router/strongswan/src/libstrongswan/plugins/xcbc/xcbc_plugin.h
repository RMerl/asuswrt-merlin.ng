/*
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
 * @defgroup xcbc_p xcbc
 * @ingroup plugins
 *
 * @defgroup xcbc_plugin xcbc_plugin
 * @{ @ingroup xcbc_p
 */

#ifndef XCBC_PLUGIN_H_
#define XCBC_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct xcbc_plugin_t xcbc_plugin_t;

/**
 * Plugin implementing xcbc algorithm to provide crypter based PRF and signers.
 */
struct xcbc_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** XCBC_PLUGIN_H_ @}*/
