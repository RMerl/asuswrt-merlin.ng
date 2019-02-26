/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "socket.h"

#include <daemon.h>

/**
 * See header
 */
bool socket_register(plugin_t *plugin, plugin_feature_t *feature,
					 bool reg, void *data)
{
	if (reg)
	{
		charon->socket->add_socket(charon->socket, (socket_constructor_t)data);
	}
	else
	{
		charon->socket->remove_socket(charon->socket,
									  (socket_constructor_t)data);
	}
	return TRUE;
}
