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
 * @defgroup eap_simaka_pseudonym eap_simaka_pseudonym
 * @ingroup cplugins
 *
 * @defgroup eap_simaka_pseudonym_plugin eap_simaka_pseudonym_plugin
 * @{ @ingroup eap_simaka_pseudonym
 */

#ifndef EAP_SIMAKA_PSEUDONYM_PLUGIN_H_
#define EAP_SIMAKA_PSEUDONYM_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_simaka_pseudonym_plugin_t eap_simaka_pseudonym_plugin_t;

/**
 * Plugin to provide in-memory storage of EAP-SIM/AKA pseudonyms.
 */
struct eap_simaka_pseudonym_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_SIMAKA_PSEUDONYM_PLUGIN_H_ @}*/
