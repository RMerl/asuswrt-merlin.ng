/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup acert acert
 * @ingroup plugins
 *
 * @defgroup acert_plugin acert_plugin
 * @{ @ingroup acert
 */

#ifndef ACERT_PLUGIN_H_
#define ACERT_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct acert_plugin_t acert_plugin_t;

/**
 * X.509 attribute certificate group membership checking.
 */
struct acert_plugin_t {

	/**
	 * Implements plugin_t. interface.
	 */
	plugin_t plugin;
};

#endif /** ACERT_PLUGIN_H_ @}*/
