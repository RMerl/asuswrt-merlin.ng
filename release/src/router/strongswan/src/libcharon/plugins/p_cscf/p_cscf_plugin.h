/*
 * Copyright (C) 2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup p_cscf p_cscf
 * @ingroup cplugins
 *
 * @defgroup p_cscf_plugin p_cscf_plugin
 * @{ @ingroup p_cscf
 */

#ifndef P_CSCF_PLUGIN_H_
#define P_CSCF_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct p_cscf_plugin_t p_cscf_plugin_t;

/**
 * Plugin that requests P-CSCF server addresses from an ePDG as specified
 * in RFC 7651.
 */
struct p_cscf_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** P_CSCF_PLUGIN_H_ @}*/
