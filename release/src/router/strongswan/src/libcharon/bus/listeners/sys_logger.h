/*
 * Copyright (C) 2012-2020 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
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

/**
 * @defgroup sys_logger sys_logger
 * @{ @ingroup listeners
 */

#ifndef SYS_LOGGER_H_
#define SYS_LOGGER_H_

#include <bus/listeners/logger.h>

typedef struct sys_logger_t sys_logger_t;

/**
 * Logger for syslog which implements listener_t.
 */
struct sys_logger_t {

	/**
	 * Implements the logger_t interface.
	 */
	logger_t logger;

	/**
	 * Set the loglevel for a debug group.
	 *
	 * @param group		debug group to set
	 * @param level		max level to log (0..4)
	 */
	void (*set_level) (sys_logger_t *this, debug_t group, level_t level);

	/**
	 * Set options used by this logger.
	 *
	 * @param ike_name	TRUE to prefix the name of the IKE_SA
	 * @param log_level	TRUE to include the log level in the message
	 * @param map_level	map log level to syslog level, -1 to disable
	 */
	void (*set_options) (sys_logger_t *this, bool ike_name, bool log_level,
						 int map_level);

	/**
	 * Destroys a sys_logger_t object.
	 */
	void (*destroy) (sys_logger_t *this);
};

/**
 * Constructor to create a sys_logger_t object.
 *
 * @param facility	syslog facility to use
 * @return			sys_logger_t object
 */
sys_logger_t *sys_logger_create(int facility);

#endif /** SYS_LOGGER_H_ @}*/
