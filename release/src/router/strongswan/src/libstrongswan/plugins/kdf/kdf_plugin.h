/*
 * Copyright (C) 2022 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup kdf_p kdf
 * @ingroup plugins
 *
 * @defgroup kdf_plugin kdf_plugin
 * @{ @ingroup kdf_p
 */

#ifndef KDF_PLUGIN_H_
#define KDF_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct kdf_plugin_t kdf_plugin_t;

/**
 * Plugin implementing the key derivation functions (KDF) in software.
 */
struct kdf_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** KDF_PLUGIN_H_ @}*/
