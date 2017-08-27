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
 * @defgroup android_dns android_dns
 * @ingroup cplugins
 *
 * @defgroup android_dns_plugin android_dns_plugin
 * @{ @ingroup android_dns
 */

#ifndef ANDROID_DNS_PLUGIN_H_
#define ANDROID_DNS_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct android_dns_plugin_t android_dns_plugin_t;

/**
 * Plugin providing an Android-specific handler for DNS servers.
 */
struct android_dns_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** ANDROID_DNS_PLUGIN_H_ @}*/
