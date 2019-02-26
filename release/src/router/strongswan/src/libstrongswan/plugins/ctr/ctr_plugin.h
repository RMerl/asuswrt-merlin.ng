/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup ctr ctr
 * @ingroup plugins
 *
 * @defgroup ctr_plugin ctr_plugin
 * @{ @ingroup ctr
 */

#ifndef CTR_PLUGIN_H_
#define CTR_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct ctr_plugin_t ctr_plugin_t;

/**
 * Plugin providing CTR mode operation of symmetric encryption algorithms.
 */
struct ctr_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** CTR_PLUGIN_H_ @}*/
