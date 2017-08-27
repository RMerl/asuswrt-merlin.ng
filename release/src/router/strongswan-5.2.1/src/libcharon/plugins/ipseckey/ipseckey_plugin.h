/*
 * Copyright (C) 2012 Reto Guadagnini
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
 * @defgroup ipseckey ipseckey
 * @ingroup cplugins
 *
 * @defgroup ipseckey_plugin ipseckey_plugin
 * @{ @ingroup ipseckey
 */

#ifndef IPSECKEY_PLUGIN_H_
#define IPSECKEY_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct ipseckey_plugin_t ipseckey_plugin_t;

/**
 * IPSECKEY plugin
 *
 * The IPSECKEY plugin registers a credential set for IPSECKEYs.
 *
 * With this credential set it is possible to authenticate tunnel endpoints
 * using IPSECKEY resource records which are retrieved from the DNS in a secure
 * way (DNSSEC).
 */
struct ipseckey_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** IPSECKEY_PLUGIN_H_ @}*/
