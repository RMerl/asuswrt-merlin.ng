/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup dnskey_p dnskey
 * @ingroup plugins
 *
 * @defgroup dnskey_plugin dnskey_plugin
 * @{ @ingroup dnskey_p
 */

#ifndef DNSKEY_PLUGIN_H_
#define DNSKEY_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct dnskey_plugin_t dnskey_plugin_t;

/**
 * Plugin providing RFC4034 public key decoding functions.
 */
struct dnskey_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** DNSKEY_PLUGIN_H_ @}*/
