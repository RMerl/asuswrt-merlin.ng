/*
 * Copyright (C) 2010 Martin Willi
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
 * @defgroup eap_tls eap_tls
 * @ingroup cplugins
 *
 * @defgroup eap_tls_plugin eap_tls_plugin
 * @{ @ingroup eap_tls
 */

#ifndef EAP_TLS_PLUGIN_H_
#define EAP_TLS_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_tls_plugin_t eap_tls_plugin_t;

/**
 * EAP-TLS plugin
 */
struct eap_tls_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_TLS_PLUGIN_H_ @}*/
