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
 * @defgroup sshkey_p sshkey
 * @ingroup plugins
 *
 * @defgroup sshkey_plugin sshkey_plugin
 * @{ @ingroup sshkey_p
 */

#ifndef SSHKEY_PLUGIN_H_
#define SSHKEY_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct sshkey_plugin_t sshkey_plugin_t;

/**
 * Plugin providing RFC 4253 public key decoding functions.
 */
struct sshkey_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** SSHKEY_PLUGIN_H_ @}*/
