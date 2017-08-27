/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup eap_md5 eap_md5
 * @ingroup cplugins
 *
 * @defgroup eap_md5_plugin eap_md5_plugin
 * @{ @ingroup eap_md5
 */

#ifndef EAP_MD5_PLUGIN_H_
#define EAP_MD5_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_md5_plugin_t eap_md5_plugin_t;

/**
 * EAP-MD5 plugin
 */
struct eap_md5_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_MD5_PLUGIN_H_ @}*/
