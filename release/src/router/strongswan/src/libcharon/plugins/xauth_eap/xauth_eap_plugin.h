/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup xauth_eap xauth_eap
 * @ingroup cplugins
 *
 * @defgroup xauth_eap_plugin xauth_eap_plugin
 * @{ @ingroup xauth_eap
 */

#ifndef XAUTH_EAP_PLUGIN_H_
#define XAUTH_EAP_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct xauth_eap_plugin_t xauth_eap_plugin_t;

/**
 * XAuth plugin using EAP to verify credentials.
 */
struct xauth_eap_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** XAUTH_EAP_PLUGIN_H_ @}*/
