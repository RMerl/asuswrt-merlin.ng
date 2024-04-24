/*
 * Copyright (C) 2023 Andreas Steffen, strongSec GmbH
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
 * @defgroup openxpki_p openxpki
 * @ingroup plugins
 *
 * @defgroup openxpki_plugin openxpki_plugin
 * @{ @ingroup openxpki_p
 */

#ifndef OPENXPKI_PLUGIN_H_
#define OPENXPKI_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct openxpki_plugin_t openxpki_plugin_t;

/**
 * Plugin implementing an OCSP responder based on OpenXPKI.
 */
struct openxpki_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** OPENXPKI_PLUGIN_H_ @}*/
