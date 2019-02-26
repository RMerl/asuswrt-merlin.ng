/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup systime_fix systime_fix
 * @ingroup cplugins
 *
 * @defgroup systime_fix_plugin systime_fix_plugin
 * @{ @ingroup systime_fix
 */

#ifndef SYSTIME_FIX_PLUGIN_H_
#define SYSTIME_FIX_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct systime_fix_plugin_t systime_fix_plugin_t;

/**
 * Plugin handling cert lifetimes gracefully if system time is out of sync.
 */
struct systime_fix_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** SYSTIME_FIX_PLUGIN_H_ @}*/
