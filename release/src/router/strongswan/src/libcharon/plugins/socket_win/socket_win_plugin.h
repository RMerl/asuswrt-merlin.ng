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
 * @defgroup socket_win socket_win
 * @ingroup cplugins
 *
 * @defgroup socket_win_plugin socket_win_plugin
 * @{ @ingroup socket_win
 */

#ifndef SOCKET_WIN_PLUGIN_H_
#define SOCKET_WIN_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct socket_win_plugin_t socket_win_plugin_t;

/**
 * Winsock2 based socket implementation plugin.
 */
struct socket_win_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** SOCKET_WIN_PLUGIN_H_ @}*/
