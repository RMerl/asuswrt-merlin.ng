/*
 * Copyright (C) 2009 Tobias Brunner
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
 * @defgroup eap_mschapv2 eap_mschapv2
 * @ingroup cplugins
 *
 * @defgroup eap_mschapv2_plugin eap_mschapv2_plugin
 * @{ @ingroup eap_mschapv2
 */

#ifndef EAP_MSCHAPV2_PLUGIN_H_
#define EAP_MSCHAPV2_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_mschapv2_plugin_t eap_mschapv2_plugin_t;

/**
 * EAP-MS-CHAPv2 plugin
 */
struct eap_mschapv2_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_MSCHAPV2_PLUGIN_H_ @}*/
