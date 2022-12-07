/*
 * Copyright (C) 2014 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#ifndef LOAD_CONNS_H_
#define LOAD_CONNS_H_

#include "command.h"

/**
 * Load all connections from configuration file
 *
 * @param conn		vici connection to load to
 * @param format	output format
 * @param cfg		configuration to load from
 */
int load_conns_cfg(vici_conn_t *conn, command_format_options_t format,
				   settings_t *cfg);

#endif /** LOAD_CONNS_H_ */
