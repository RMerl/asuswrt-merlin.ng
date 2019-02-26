/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup error_notify error_notify
 * @ingroup cplugins
 *
 * @defgroup error_notify_plugin error_notify_plugin
 * @{ @ingroup error_notify
 */

#ifndef ERROR_NOTIFY_PLUGIN_H_
#define ERROR_NOTIFY_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct error_notify_plugin_t error_notify_plugin_t;

/**
 * Plugin sending error notifications over a UNIX socket.
 */
struct error_notify_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** ERROR_NOTIFY_PLUGIN_H_ @}*/
