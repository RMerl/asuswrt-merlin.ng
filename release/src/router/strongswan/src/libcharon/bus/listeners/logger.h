/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup logger logger
 * @{ @ingroup listeners
 */

#ifndef LOGGER_H_
#define LOGGER_H_

typedef struct logger_t logger_t;

#include <bus/bus.h>

/**
 * Logger interface, listens for log events on the bus.
 *
 * Calls to bus_t.log() are handled separately from calls to other functions.
 * Logger functions may be called concurrently by multiple threads. Also
 * recursive calls are not prevented, loggers that may cause recursive log
 * messages are responsible to avoid infinite loops.
 *
 * Both the log() and the vlog() methods are optional to implement. With many
 * loggers, using log() may be faster as printf() format substitution is done
 * only once for all loggers.
 */
struct logger_t {

	/**
	 * Log a debugging message.
	 *
	 * @param group		kind of the signal (up, down, rekeyed, ...)
	 * @param level		verbosity level of the signal
	 * @param thread	ID of the thread raised this signal
	 * @param ike_sa	IKE_SA associated to the event
	 * @param message	log message
	 */
	void (*log)(logger_t *this, debug_t group, level_t level, int thread,
				ike_sa_t *ike_sa, const char *message);

	/**
	 * Log a debugging message with a format string.
	 *
	 * @note Calls to bus_t.log() are handled separately from calls to
	 * other functions.  This callback may be called concurrently by
	 * multiple threads.  Also recursive calls are not prevented, loggers that
	 * may cause recursive log messages are responsible to avoid infinite loops.
	 *
	 * @param group		kind of the signal (up, down, rekeyed, ...)
	 * @param level		verbosity level of the signal
	 * @param thread	ID of the thread raised this signal
	 * @param ike_sa	IKE_SA associated to the event
	 * @param fmt		log message format string
	 * @param args		variable arguments to format string
	 */
	void (*vlog)(logger_t *this, debug_t group, level_t level, int thread,
				 ike_sa_t *ike_sa, const char *fmt, va_list args);

	/**
	 * Get the desired log level for a debug group.  This is called during
	 * registration.
	 *
	 * If the desired log levels have changed, re-register the logger with
	 * the bus.
	 *
	 * @param group		debug group
	 * @return			max level to log (0..4) or -1 for none (see debug.h)
	 */
	level_t (*get_level)(logger_t *this, debug_t group);
};

#endif /** LOGGER_H_ @}*/
