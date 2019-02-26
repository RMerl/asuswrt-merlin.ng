/*
 * Copyright (C) 2012-2015 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
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
 * @defgroup file_logger file_logger
 * @{ @ingroup listeners
 */

#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_

#include <bus/listeners/logger.h>

typedef struct file_logger_t file_logger_t;

/**
 * Logger to files which implements listener_t.
 */
struct file_logger_t {

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
	void (*set_level) (file_logger_t *this, debug_t group, level_t level);

	/**
	 * Set options used by this logger
	 *
	 * @param time_format	format of timestamp prefix, as in strftime(), cloned
	 * @param add_ms		TRUE to add the number of milliseconds within the
	 *						current second after the timestamp
	 * @param ike_name		TRUE to prefix the name of the IKE_SA
	 */
	void (*set_options) (file_logger_t *this, char *time_format, bool add_ms,
						 bool ike_name);

	/**
	 * Open (or reopen) the log file according to the given parameters
	 *
	 * @param flush_line	TRUE to flush buffers after every logged line
	 * @param append		FALSE to overwrite an existing file, TRUE to append
	 */
	void (*open) (file_logger_t *this, bool flush_line, bool append);

	/**
	 * Destroys a file_logger_t object.
	 */
	void (*destroy) (file_logger_t *this);
};

/**
 * Constructor to create a file_logger_t object.
 *
 * The logger has to be opened via file_logger_t.open() before anything is
 * logged.
 *
 * @param filename	name of the log file (stderr and stdout are handled
 *					specially), cloned
 * @return			file_logger_t object
 */
file_logger_t *file_logger_create(char *filename);

#endif /** FILE_LOGGER_H_ @}*/
